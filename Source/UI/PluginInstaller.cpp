/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2019 Open Ephys

	 ------------------------------------------------------------------

	 This program is free software: you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "PluginInstaller.h"
#include <stdio.h>
#include <filesystem>

#include "../CoreServices.h"
#include "../AccessClass.h"
#include "../Processors/PluginManager/PluginManager.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "ProcessorList.h"
#include "ControlPanel.h"
#ifdef _WIN32
#include <Windows.h>
#endif


//-----------------------------------------------------------------------
static inline File getPluginsDirectory() {

	File dir = CoreServices::getSavedStateDirectory();
	if(!dir.getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
		dir = dir.getChildFile("plugins-api" + String(PLUGIN_API_VER));
	else
		dir = dir.getChildFile("plugins");

    return std::move(dir);
}

static inline File getSharedDirectory() {

	File dir = CoreServices::getSavedStateDirectory();
	if(!dir.getFullPathName().contains("plugin-GUI" + File::getSeparatorString() + "Build"))
		dir = dir.getChildFile("shared-api" + String(PLUGIN_API_VER));
	else
		dir = dir.getChildFile("shared");

    return std::move(dir);
}

static String osType;
StringArray updatablePlugins;

PluginInstaller::PluginInstaller(MainWindow* mainWindow, bool loadComponents)
: DocumentWindow(WINDOW_TITLE,
		Colour(Colours::black),
		DocumentWindow::closeButton)
{

	MouseCursor::showWaitCursor();
	parent = (DocumentWindow*)mainWindow;

   // Identify the OS on which the GUI is running
	SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();

	if ((os & SystemStats::OperatingSystemType::Windows) != 0)
	 	osType = "windows";
	else if ((os & SystemStats::OperatingSystemType::MacOSX) != 0)
	 	osType = "mac";
	else if ((os & SystemStats::OperatingSystemType::Linux) != 0)
	 	osType = "linux";

	//Initialize Plugin Installer Components

	if(loadComponents)
	{
		int x = parent->getX();
		int y = parent->getY();
		int w = parent->getWidth();
		int h = parent->getHeight();

		setBounds(x + (0.5*w) - 450, y + 0.5*h - 240, 900, 480);
		setUsingNativeTitleBar(true);
		setContentOwned(new PluginInstallerComponent(), false);
		setVisible(true);
		setResizable(true, false); // useBottomCornerRisizer -- doesn't work very well
		setResizeLimits(910, 480, 8192, 5120);
	}

	createXmlFile();

	MouseCursor::hideWaitCursor();
	CoreServices::sendStatusMessage("Plugin Installer is ready!");
}

PluginInstaller::~PluginInstaller()
{
	masterReference.clear();
}

void PluginInstaller::closeButtonPressed()
{
	setVisible(false);
	delete this;
}

void PluginInstaller::createXmlFile()
{
    
	File file = getPluginsDirectory().getChildFile("installedPlugins.xml");

	XmlDocument doc(file);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		std::unique_ptr<XmlElement> baseTag(new XmlElement("PluginInstaller"));
		baseTag->setAttribute("gui_version", JUCEApplication::getInstance()->getApplicationVersion());

		std::unique_ptr<XmlElement> plugins(new XmlElement("InstalledPlugins"));

		baseTag->addChildElement(plugins.release());

		if (! baseTag->writeTo(file))
			LOGE("Error! Couldn't write to installedPlugins.xml");
	}
	else
	{
		auto child = xml->getFirstChildElement();
		Array<XmlElement*> elementsToRemove;

		for (auto* e : child->getChildIterator())
		{
			File pluginPath = getPluginsDirectory().getChildFile(e->getAttributeValue(1));
			if (!pluginPath.exists())
				elementsToRemove.add(e);	
		}

		for (auto element : elementsToRemove)
		{
			child->removeChildElement(element, true);
		}

		if (! xml->writeTo(file))
		{
			LOGE("Error! Couldn't write to installedPlugins.xml");
		}

		elementsToRemove.clear();

	}
}


void PluginInstaller::installPluginAndDependency(const String& plugin, String version)
{
	PluginInfoComponent tempInfoComponent;

	/** Get list of plugins uploaded to Artifactory */
	String baseUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";
	String response = URL(baseUrl).readEntireTextStream();

	if(response.isEmpty())
		LOGD("Unable to fetch plugins! Please check your internet connection and try again.")

	var gatewayData;
	Result result = JSON::parse(response, gatewayData);
	
	String url = gatewayData.getProperty("download_url", var()).toString();
	tempInfoComponent.setDownloadURL(url);

	var pluginData = gatewayData.getProperty("plugins", var());

	int pIndex;
	bool pluginFound = false;

	for (int i = 0; i < pluginData.size(); i++)
	{
		if(pluginData[i].getProperty("display_name", "NULL").toString().equalsIgnoreCase(plugin))
		{
			pIndex = i;
			pluginFound = true;
			break;
		}
	}

	if(!pluginFound)
	{
		LOGC("Automated Plugin Installation Failed! Plugin not found!")
		return;
	}
    
	auto platforms = pluginData[pIndex].getProperty("platforms", "none").getArray();

	if (!platforms->contains(osType))
	{
		LOGD("*********** No platform specific package found for ", plugin);
		return;
	}

	LOGC(plugin, " plugin found! Installing it now...")

	SelectedPluginInfo requiredPluginInfo;

	requiredPluginInfo.pluginName = pluginData[pIndex].getProperty("name", "NULL").toString();
	requiredPluginInfo.displayName = plugin;
	requiredPluginInfo.type = pluginData[pIndex].getProperty("type", "NULL").toString();
	requiredPluginInfo.latestVersion = pluginData[pIndex].getProperty("latest_version", "NULL");

	auto allVersions = pluginData[pIndex].getProperty("versions", "NULL").getArray();

	requiredPluginInfo.versions.clear();

	for (String v : *allVersions)
	{
		String apiVer = v.substring(v.indexOf("I") + 1);
		
		if (apiVer.equalsIgnoreCase(String(PLUGIN_API_VER)))
			requiredPluginInfo.versions.add(v);
	}

	requiredPluginInfo.dependencies.clear();
	auto dependencies = pluginData[pIndex].getProperty("dependencies", "NULL").getArray();
	for (String dependency : *dependencies)
	{
		if(!dependency.equalsIgnoreCase("None"))
		{
			requiredPluginInfo.dependencies.add(dependency);
			for (int i = 0; i < pluginData.size(); i++)
			{
				if(pluginData[i].getProperty("name", "NULL").toString().equalsIgnoreCase(dependency))
				{
					requiredPluginInfo.dependencyVersions.add(pluginData[i].getProperty("latest_version", "NULL").toString());
					break;
				}
			}
		}
	}

	tempInfoComponent.setPluginInfo(requiredPluginInfo, false);

	for (int i = 0; i < requiredPluginInfo.dependencies.size(); i++)
	{
		tempInfoComponent.downloadPlugin(requiredPluginInfo.dependencies[i], 
										 requiredPluginInfo.dependencyVersions[i], 
										 true);
	}
	
	// download the plugin
	if(version.isEmpty() || !requiredPluginInfo.versions.contains(version))
	{
		if(!requiredPluginInfo.versions.isEmpty())
		{
			LOGC(plugin, " version ", version, " not found! Installing the latest version");
			requiredPluginInfo.versions.sort(false);
			version = requiredPluginInfo.versions[requiredPluginInfo.versions.size() - 1];
		}
		else
		{
			LOGC("Automated Plugin Installation Failed! Compatible plugin version not found!")
			return;
		}
	}
	
	int code = tempInfoComponent.downloadPlugin(requiredPluginInfo.pluginName, version, false);

	if(code == 1)
		LOGC("Install sucessfull!!")
	else
		LOGC("Install failed!!");

}


/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent() : ThreadWithProgressWindow("Plugin Installer", false, false),
													   checkForUpdates(false)
{
	font = Font("Fira Sans", "Regular", 18.0f);
	setSize(getWidth() - 10, getHeight() - 10);

	addAndMakeVisible(pluginListAndInfo);
	
	//Auto check for updates on startup
	checkForUpdates = true;
	this->run();

	addAndMakeVisible(sortingLabel);
	sortingLabel.setColour(Label::textColourId, Colours::white);
	sortingLabel.setFont(font);
	sortingLabel.setText("Sort By:", dontSendNotification);

	addAndMakeVisible(sortByMenu);
	sortByMenu.setJustificationType(Justification::centred);
	sortByMenu.addItem("A - Z", 1);
	sortByMenu.addItem("Z - A", 2);
	sortByMenu.setTextWhenNothingSelected("-----");
	sortByMenu.addListener(this);

	addAndMakeVisible(viewLabel);
	viewLabel.setColour(Label::textColourId, Colours::white);
	viewLabel.setFont(font);
	viewLabel.setText("View:", dontSendNotification);

	addAndMakeVisible(allButton);
	allButton.setButtonText("All");
	allButton.setColour(ToggleButton::textColourId, Colours::white);
	allButton.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	allButton.setRadioGroupId(101, dontSendNotification);
	allButton.addListener(this);	
	allButton.setToggleState(true, dontSendNotification);

	addAndMakeVisible(installedButton);
	installedButton.setButtonText("Installed");
	installedButton.setClickingTogglesState(true);
	installedButton.setColour(ToggleButton::textColourId, Colours::white);
	installedButton.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	installedButton.setRadioGroupId(101, dontSendNotification);
	installedButton.addListener(this);

	addAndMakeVisible(updatesButton);
	updatesButton.setButtonText("Fetch Updates");
	updatesButton.changeWidthToFitText();
	updatesButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	updatesButton.addListener(this);

	addAndMakeVisible(typeLabel);
	typeLabel.setColour(Label::textColourId, Colours::white);
	typeLabel.setFont(font);
	typeLabel.setText("Type:", dontSendNotification);

	addAndMakeVisible(filterType);
	filterType.setButtonText("Filter");
	filterType.setColour(ToggleButton::textColourId, Colours::white);
	filterType.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	filterType.addListener(this);
	filterType.setToggleState(true, dontSendNotification);	

	addAndMakeVisible(sourceType);
	sourceType.setButtonText("Source");
	sourceType.setColour(ToggleButton::textColourId, Colours::white);
	sourceType.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	sourceType.addListener(this);
	sourceType.setToggleState(true, dontSendNotification);

	addAndMakeVisible(sinkType);
	sinkType.setButtonText("Sink");
	sinkType.setColour(ToggleButton::textColourId, Colours::white);
	sinkType.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	sinkType.addListener(this);
	sinkType.setToggleState(true, dontSendNotification);

	addAndMakeVisible(otherType);
	otherType.setButtonText("Other");
	otherType.setColour(ToggleButton::textColourId, Colours::white);
	otherType.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	otherType.addListener(this);
	otherType.setToggleState(true, dontSendNotification);
}

void PluginInstallerComponent::paint(Graphics& g)
{
	g.fillAll (Colours::darkgrey);
	g.setColour(Colour::fromRGB(110, 110, 110));
	g.fillRect(190, 5, 3, 38);
	g.fillRect(410, 5, 3, 38);
}

void PluginInstallerComponent::resized()
{
	sortingLabel.setBounds(20, 10, 70, 30);
	sortByMenu.setBounds(90, 10, 90, 30);

	viewLabel.setBounds(200, 10, 45, 30);
	allButton.setBounds(245, 11, 55, 28);
	installedButton.setBounds(300, 11, 105, 28);

	typeLabel.setBounds(415, 11, 45, 28);
	sourceType.setBounds(460, 11, 80, 28);
	filterType.setBounds(540, 11, 80, 28);
	sinkType.setBounds(620, 11, 70, 28);
	otherType.setBounds(690, 11, 75, 28);

	updatesButton.setBounds(getWidth() - 125, 11, 105, 28);

	pluginListAndInfo.setBounds(10, 40, getWidth() - 10, getHeight() - 40);
}

void PluginInstallerComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged->getSelectedId() == 1)
	{
		pluginListAndInfo.pluginArray.sort(true);
		pluginListAndInfo.repaint();
	}
	else if (comboBoxThatHasChanged->getSelectedId() == 2)
	{
		pluginListAndInfo.pluginArray.sort(true);
		int size = pluginListAndInfo.pluginArray.size();
		for (int i = 0; i < size / 2; i++)
		{
			pluginListAndInfo.pluginArray.getReference(i).swapWith
			(pluginListAndInfo.pluginArray.getReference(size - i - 1));
		}

		pluginListAndInfo.repaint();
	}
}

void PluginInstallerComponent::run()
{
	File xmlFile = getPluginsDirectory().getChildFile("installedPlugins.xml");

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		LOGE("[PluginInstaller] File not found.");
		return;
	}
	else
	{	
		installedPlugins.clear();
		auto child = xml->getFirstChildElement();

		String baseUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";
		var gatewayData;
		if (checkForUpdates)
		{
			setStatusMessage("Fetching plugin updates...");
			updatablePlugins.clear();

			String response = URL(baseUrl).readEntireTextStream();

			if(response.isEmpty())
			{
				LOGE("Unable to fetch updates! Please check you internet connection and try again.")
				return;
			}

			Result result = JSON::parse(response, gatewayData);
			gatewayData = gatewayData.getProperty("plugins", var());

		}

		for (auto* e : child->getChildIterator())
		{
			String pName = e->getTagName();
			installedPlugins.add(pName);

			if (checkForUpdates)
			{	
				String latestVer;

				//Get latest version
				for (int i = 0; i < gatewayData.size(); i++)
				{
					if(gatewayData[i].getProperty("name", "NULL").toString().equalsIgnoreCase(pName))
					{
						latestVer = gatewayData[i].getProperty("latest_version", "NULL");
						break;
					}
				}

				if (latestVer.compareNatural(e->getAttributeValue(0)) > 0)
					updatablePlugins.add(pName);
			}
		}

		checkForUpdates = false;
	}

	if(updatablePlugins.size() > 0)
	{
		const String updatemsg = "Some of your plugins have updates available! "
								 "Please update them to get the latest features and bug-fixes.";

		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon,
										 "Updates Available",
										 updatemsg, "OK", this);
	}
}

void PluginInstallerComponent::buttonClicked(Button* button)
{
	// Filter plugins on the basis of checkbox selected
	if(allPlugins.isEmpty())
	{
		allPlugins.addArray(pluginListAndInfo.pluginArray);
	}

	if(button == &installedButton)
	{
		this->run();
		
		pluginListAndInfo.pluginArray.clear();
		pluginListAndInfo.pluginArray.addArray(installedPlugins);
		pluginListAndInfo.setNumRows(installedPlugins.size());
	}
	else if(button == &allButton)
	{	
		pluginListAndInfo.pluginArray.clear();
		pluginListAndInfo.pluginArray.addArray(allPlugins);
		pluginListAndInfo.setNumRows(allPlugins.size());
	}
	else if(button == &updatesButton)
	{
		checkForUpdates = true;
		this->runThread();
		pluginListAndInfo.resized();
	}

	if ( button == &sourceType || button == &filterType || button == &sinkType || button == &otherType)
	{
		bool sourceState = sourceType.getToggleState();
		bool filterState = filterType.getToggleState();
		bool sinkState = sinkType.getToggleState();
		bool otherState = otherType.getToggleState();

		if( sourceState || filterState || sinkState || otherState)
		{

			StringArray tempArray;

			pluginListAndInfo.pluginArray.clear();

			if(installedButton.getToggleState())
				tempArray.addArray(installedPlugins);
			else
				tempArray.addArray(allPlugins);

			for (int i = 0; i < tempArray.size(); i++)
			{
				String label;

				label = pluginListAndInfo.pluginLabels[tempArray[i]];
				
				int containsType = 0;

				bool isSource = label.containsWholeWordIgnoreCase("source");
				bool isFilter = label.containsWholeWordIgnoreCase("filter");
				bool isSink = label.containsWholeWordIgnoreCase("sink");
				bool isOther = isSource ? false : (isFilter ? false : (isSink ? false : true));

				if(sourceState && isSource)
					containsType++;
				
				if(filterState && isFilter)
					containsType++;

				if(sinkState && isSink)
					containsType++;
				
				if(otherState && isOther)
					containsType++;
				
				if(containsType > 0)
				{
					pluginListAndInfo.pluginArray.add(tempArray[i]);
				}
				pluginListAndInfo.setNumRows(pluginListAndInfo.pluginArray.size());
			}
			
		}
		else
		{
			pluginListAndInfo.pluginArray.clear();
			pluginListAndInfo.setNumRows(0);
		}
		
	}

	sortByMenu.setSelectedId(-1, dontSendNotification);
}


/* ================================== Plugin Table Component ================================== */

PluginListBoxComponent::PluginListBoxComponent() : Thread("Plugin List"), maxTextWidth(0)
{
	listFont = Font("Fira Code", "SemiBold", 22.0f);

	// Set progress window text and background colors
	//auto window = this->getAlertWindow();
	//window->setColour(AlertWindow::textColourId, Colours::white);
	//window->setColour(AlertWindow::backgroundColourId, Colour::fromRGB(50, 50, 50));
	//setStatusMessage("Fetching plugins ...");

	this->run(); //Load all plugin names and labels from bintray

	addAndMakeVisible(pluginList);
	pluginList.setModel(this);
	pluginList.setColour(ListBox::backgroundColourId , Colour::fromRGB(50, 50, 50));
	pluginList.setRowHeight(35);
	pluginList.setMouseMoveSelectsRows(true);

	listBoxDropShadower.setOwner(&pluginList);

	addAndMakeVisible(pluginInfoPanel);
}

int PluginListBoxComponent::getNumRows()
{
	return numRows;
}

void PluginListBoxComponent::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
	if (rowIsSelected)
	{
		g.fillAll(Colour::fromRGB(100, 100, 100));
		g.setColour(Colour::fromRGB(50, 50, 50));
	}
	else
	{
		g.fillAll(Colour::fromRGB(50, 50, 50));
		g.setColour(Colours::white);
	}

	if ( rowNumber == pluginArray.indexOf(lastPluginSelected, true, 0) )
	{
		g.setColour (Colours::yellow);
	}

	g.setFont(listFont);

	String text = displayNames[pluginArray[rowNumber]];

	g.drawText (text, 20, 0, maxTextWidth + 5, height, Justification::centredLeft, true);

	// Draw update indicator next to plugin name, if any
	if(updatablePlugins.contains(pluginArray[rowNumber]))
	{
		g.setColour(Colours::green);
		g.fillEllipse(maxTextWidth + 25.0f, 6.0f, 23.0f, height - 12.0f);
		g.setColour(Colours::white);
		g.drawArrow(Line(maxTextWidth + 37.0f, height - 11.0f, maxTextWidth + 37.0f, 11.0f ), 3.0f, 9.0f, 9.0f);
	}
}

void PluginListBoxComponent::run()
{
	/* Get list of plugins uploaded to bintray */
	String baseUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";
	String response = URL(baseUrl).readEntireTextStream();

	if(response.isEmpty())
		LOGE("Unable to fetch plugins! Please check your internet connection and try again.")

	var gatewayData;
	Result result = JSON::parse(response, gatewayData);
	
	String url = gatewayData.getProperty("download_url", var()).toString();
	pluginInfoPanel.setDownloadURL(url);

	pluginData = gatewayData.getProperty("plugins", var());

	numRows = pluginData.size();
	
	String pluginName, label, dispName;

	int pluginTextWidth;
	StringArray compatibleVersions;

	// Get each plugin's labels and add them to the list
	for (int i = 0; i < numRows; i++)
	{
        auto allVersions = pluginData[i].getProperty("versions", "NULL").getArray();
		compatibleVersions.clear();

		for (String version : *allVersions)
		{
			String apiVer = version.substring(version.indexOf("I") + 1);
			
			if (apiVer.equalsIgnoreCase(String(PLUGIN_API_VER)))
			{
				compatibleVersions.add(version);
				break;
			}
		}
		
		auto platforms = pluginData[i].getProperty("platforms", "none").getArray();

		if(!compatibleVersions.isEmpty() && platforms->contains(osType))
		{
			pluginName = pluginData[i].getProperty("name", var()).toString();
			label = pluginData[i].getProperty("type", "NULL").toString();
			dispName = pluginData[i].getProperty("display_name", "NULL").toString();

			pluginTextWidth = listFont.getStringWidth(dispName);
			if (pluginTextWidth > maxTextWidth)
				maxTextWidth = pluginTextWidth;
			
			if(!label.equalsIgnoreCase("CommonLib"))
			{
				pluginArray.add(pluginName);
				displayNames.set(pluginName, dispName);
				pluginLabels.set(pluginName, label);
			}
			else
			{	
				String ver = pluginData[i].getProperty("latest_version", "NULL").toString();
				dependencyVersion.set(pluginName, ver);
			}
		}

		//setProgress ((i + 1) / (double) numRows);
	}
	
    const MessageManagerLock mmLock;
    setNumRows(pluginArray.size());
}

bool PluginListBoxComponent::loadPluginInfo(const String& pluginName)
{

    int pIndex;
	for (int i = 0; i < pluginData.size(); i++)
	{
		if(pluginData[i].getProperty("name", "NULL").toString().equalsIgnoreCase(pluginName))
		{
			pIndex = i;
			break;
		}
	}
    
	auto platforms = pluginData[pIndex].getProperty("platforms", "none").getArray();

	if (!platforms->contains(osType))
	{
		LOGE("No platform specific package found for ", pluginName);
		pluginInfoPanel.makeInfoVisible(false);
		return false;
	}

	selectedPluginInfo.pluginName = pluginName;
	selectedPluginInfo.displayName = displayNames[pluginName];
	selectedPluginInfo.type = pluginLabels[pluginName];
	selectedPluginInfo.developers= pluginData[pIndex].getProperty("developers", "NULL");
	selectedPluginInfo.latestVersion = pluginData[pIndex].getProperty("latest_version", "NULL");
	String updated = pluginData[pIndex].getProperty("updated", "NULL");
	selectedPluginInfo.lastUpdated = updated.substring(0, updated.indexOf("T"));
	selectedPluginInfo.description = pluginData[pIndex].getProperty("desc", "NULL");
	selectedPluginInfo.docURL = pluginData[pIndex].getProperty("docs", "NULL").toString();
	selectedPluginInfo.selectedVersion = String();

	auto allVersions = pluginData[pIndex].getProperty("versions", "NULL").getArray();

	selectedPluginInfo.versions.clear();

	for (String version : *allVersions)
	{
		String apiVer = version.substring(version.indexOf("I") + 1);
		
		if (apiVer.equalsIgnoreCase(String(PLUGIN_API_VER)))
			selectedPluginInfo.versions.add(version);
	}

	selectedPluginInfo.dependencies.clear();
	auto dependencies = pluginData[pIndex].getProperty("dependencies", "NULL").getArray();
	for (String dependency : *dependencies)
	{
		if(!dependency.equalsIgnoreCase("None"))
		{
			selectedPluginInfo.dependencies.add(dependency);
			selectedPluginInfo.dependencyVersions.add(dependencyVersion[dependency]);
		}
	}

	// If the plugin is already installed, get installed version number
	File xmlFile = getPluginsDirectory().getChildFile("installedPlugins.xml");;

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		LOGE("File not found.");
		return false;
	}
	else
	{	
		auto child = xml->getFirstChildElement();

		auto pluginEntry = child->getChildByName(pluginName);

		if (pluginEntry != nullptr)
			selectedPluginInfo.installedVersion = pluginEntry->getAttributeValue(0);
		else
			selectedPluginInfo.installedVersion = String();
		
	}

	pluginInfoPanel.setPluginInfo(selectedPluginInfo);
	pluginInfoPanel.makeInfoVisible(true);

	return true;
}

void PluginListBoxComponent::listBoxItemClicked (int row, const MouseEvent &)
{
	this->returnKeyPressed(row);
}

void PluginListBoxComponent::resized()
{
	// position our table with a gap around its edge
	if(updatablePlugins.isEmpty())
	{
		pluginList.setBounds(10, 10, maxTextWidth + 60, getHeight() - 30);
		pluginInfoPanel.setBounds(maxTextWidth + 80, 10, 
								  getWidth() - maxTextWidth - 100, 
								  getHeight() - 30);
	}
	else
	{
    	pluginList.setBounds(10, 10, maxTextWidth + 70, getHeight() - 30);
		pluginInfoPanel.setBounds(maxTextWidth + 90, 10, 
								  getWidth() - maxTextWidth - 110, 
								  getHeight() - 30);
	}
}

void PluginListBoxComponent::returnKeyPressed (int lastRowSelected)
{
	if (!lastPluginSelected.equalsIgnoreCase(pluginArray[lastRowSelected]))
	{
		lastPluginSelected = pluginArray[lastRowSelected];

		pluginInfoPanel.makeInfoVisible(false);
		pluginInfoPanel.updateStatusMessage("Loading Plugin Info...", true);
		
		if(loadPluginInfo(lastPluginSelected))
			pluginInfoPanel.updateStatusMessage("", false);
		else
			pluginInfoPanel.updateStatusMessage("No platform specific package found for " + lastPluginSelected, true);
		
		this->repaint();
	}
}

/* ================================== Plugin Information Component ================================== */

PluginInfoComponent::PluginInfoComponent() : ThreadWithProgressWindow("Plugin Installer", false, false)
{
	infoCompDropShadower.setOwner(this);
	
	infoFont = Font("Fira Code", "Regular", 20.0f);
	infoFontBold = Font("Fira Code", "SemiBold", 20.0f);
	
	addChildComponent(pluginNameLabel);
	pluginNameLabel.setFont(infoFontBold);
	pluginNameLabel.setColour(Label::textColourId, Colours::white);
	pluginNameLabel.setText("Name: ", dontSendNotification);

	addChildComponent(pluginNameText);
	pluginNameText.setFont(infoFont);
	pluginNameText.setColour(Label::textColourId, Colours::white);

	addChildComponent(developersLabel);
	developersLabel.setFont(infoFontBold);
	developersLabel.setColour(Label::textColourId, Colours::white);
	developersLabel.setText("Developers: ", dontSendNotification);

	addChildComponent(developersText);
	developersText.setFont(infoFont);
	developersText.setColour(Label::textColourId, Colours::white);
	developersText.setMinimumHorizontalScale(1.0f);

	addChildComponent(versionLabel);
	versionLabel.setFont(infoFontBold);
	versionLabel.setColour(Label::textColourId, Colours::white);
	versionLabel.setText("Version: ", dontSendNotification);

	addChildComponent(versionMenu);
	versionMenu.setJustificationType(Justification::centred);
	versionMenu.setTextWhenNoChoicesAvailable("- N/A -");
	versionMenu.addListener(this);

	addChildComponent(installedVerLabel);
	installedVerLabel.setFont(infoFontBold);
	installedVerLabel.setColour(Label::textColourId, Colours::white);
	installedVerLabel.setText("Installed: ", dontSendNotification);

	addChildComponent(installedVerText);
	installedVerText.setFont(infoFont);
	installedVerText.setColour(Label::textColourId, Colours::white);
	installedVerText.setMinimumHorizontalScale(1.0f);

	addChildComponent(lastUpdatedLabel);
	lastUpdatedLabel.setFont(infoFontBold);
	lastUpdatedLabel.setColour(Label::textColourId, Colours::white);
	lastUpdatedLabel.setText("Last Updated: ", dontSendNotification);

	addChildComponent(lastUpdatedText);
	lastUpdatedText.setFont(infoFont);
	lastUpdatedText.setColour(Label::textColourId, Colours::white);

	addChildComponent(descriptionLabel);
	descriptionLabel.setFont(infoFontBold);
	descriptionLabel.setColour(Label::textColourId, Colours::white);
	descriptionLabel.setText("Description: ", dontSendNotification);

	addChildComponent(descriptionText);
	descriptionText.setFont(infoFont);
	descriptionText.setColour(Label::textColourId, Colours::white);
	descriptionText.setJustificationType(Justification::topLeft);
	descriptionText.setMinimumHorizontalScale(1.0f);

	addChildComponent(dependencyLabel);
	dependencyLabel.setFont(infoFontBold);
	dependencyLabel.setColour(Label::textColourId, Colours::white);
	dependencyLabel.setText("Dependencies: ", dontSendNotification);

	addChildComponent(dependencyText);
	dependencyText.setFont(infoFont);
	dependencyText.setColour(Label::textColourId, Colours::white);

	addChildComponent(downloadButton);
	downloadButton.setButtonText("Install");
	downloadButton.setColour(TextButton::buttonColourId, Colours::skyblue);
	downloadButton.addListener(this);

	addChildComponent(uninstallButton);
	uninstallButton.setButtonText("Uninstall");
	uninstallButton.setColour(TextButton::buttonColourId, Colours::crimson);
	uninstallButton.addListener(this);

	addChildComponent(documentationButton);
	documentationButton.setButtonText("Documentation");
	documentationButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	documentationButton.addListener(this);

	addAndMakeVisible(statusLabel);
	statusLabel.setFont(infoFont);
	statusLabel.setColour(Label::textColourId, Colours::white);
	statusLabel.setText("Please select a plugin from the list on the left...", dontSendNotification);
}

void PluginInfoComponent::paint(Graphics& g)
{
	g.fillAll (Colour::fromRGB(50, 50, 50));
}

void PluginInfoComponent::resized()
{
	pluginNameLabel.setBounds(10, 30, 140, 30);
	pluginNameText.setBounds(145, 30, getWidth() - 10, 30);

	developersLabel.setBounds(10, 60, 140, 30);
	developersText.setBounds(145, 60, getWidth() - 130, 30);

	versionLabel.setBounds(10, 90, 140, 30);
	versionMenu.setBounds(150, 90, 110, 26);

	installedVerLabel.setBounds(10, versionLabel.getBottom(), 140, 30);
	installedVerText.setBounds(145, versionLabel.getBottom(), 110, 30);
	
	lastUpdatedLabel.setBounds(10, installedVerLabel.getBottom(), 140, 30);
	lastUpdatedText.setBounds(145, installedVerLabel.getBottom(), getWidth() - 10, 30);

	descriptionLabel.setBounds(10, lastUpdatedLabel.getBottom(), 140, 30);
	descriptionText.setBounds(145, lastUpdatedLabel.getBottom() + 5, getWidth() - 150, 75);

	dependencyLabel.setBounds(10, descriptionText.getBottom() + 5, 140, 30);
	dependencyText.setBounds(145, dependencyLabel.getY(), getWidth() - 10, 30);

	downloadButton.setBounds(getWidth() - (getWidth() * 0.25) - 20, getHeight() - 60, getWidth() * 0.25, 30);
	uninstallButton.setBounds(getWidth() - (2 * (getWidth() * 0.25)) - 30, getHeight() - 60, getWidth() * 0.25, 30);
	documentationButton.setBounds(20, getHeight() - 60, getWidth() * 0.25, 30);
	
	statusLabel.setBounds(10, (getHeight() / 2) - 15, getWidth() - 10, 30);
}

void PluginInfoComponent::buttonClicked(Button* button)
{
	if (button == &downloadButton)
	{
		this->runThread();
	}
	else if(button == &uninstallButton)
	{
		if(!uninstallPlugin(pInfo.pluginName))
		{
			LOGE("Failed to uninstall ", pInfo.displayName);
		}
		else
			LOGC(pInfo.displayName, " uninstalled successfully!");
	}
	else if (button == &documentationButton)
	{
		URL url = URL(pInfo.docURL);
		url.launchInDefaultBrowser();
	}
}

void PluginInfoComponent::setDownloadURL(const String& url)
{
	downloadURL = url;
}

void PluginInfoComponent::run()
{

	// If a plugin has depencies outside its zip, download them
	for (int i = 0; i < pInfo.dependencies.size(); i++)
	{
		setStatusMessage("Downloading dependency: " + pInfo.dependencies[i]);
		LOGD("Downloading dependency: ", pInfo.dependencies[i], "...  ");

		int retCode = downloadPlugin(pInfo.dependencies[i], pInfo.dependencyVersions[i], true);

		if (retCode == 2)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
											"[Plugin Installer] " + pInfo.dependencies[i], 
											"Could not install dependency: " + pInfo.dependencies[i] 
											+ ". Please contact the developers.");
			
			LOGE("Download Failed!!");
			return;
		}
		else if (retCode == ZIP_NOTFOUND)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
											"[Plugin Installer] " + pInfo.dependencies[i], 
											"Could not find the ZIP file for " + pInfo.dependencies[i] 
											+ ". Please contact the developers.");

			LOGE("Download Failed!!");
			return;
		}
		else if(retCode == HTTP_ERR)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
											"[Plugin Installer] " + pInfo.dependencies[i], 
											"HTTP request failed!!\nPlease check your internet connection...");
			
			LOGE("HTTP request failed!! Please check your internet connection...");
			return;
		}
		
	}
	
	setStatusMessage("Downloading " + pInfo.displayName + " ...");
	LOGD("Downloading Plugin: ", pInfo.pluginName, "...  ");

	// download the plugin
	int dlReturnCode = downloadPlugin(pInfo.pluginName, pInfo.selectedVersion, false);

	if (dlReturnCode == SUCCESS)
	{	
		AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										pInfo.displayName + " Installed Successfully");

		LOGD("Download Successfull!!");

		pInfo.installedVersion = pInfo.selectedVersion;
		installedVerText.setText(pInfo.installedVersion, dontSendNotification);
		downloadButton.setEnabled(false);
		downloadButton.setButtonText("Installed");
		uninstallButton.setVisible(true);

		if(pInfo.latestVersion.equalsIgnoreCase(pInfo.latestVersion))
		{
			updatablePlugins.removeString(pInfo.pluginName);
			this->getParentComponent()->resized();
		}
		
	}
	else if (dlReturnCode == ZIP_NOTFOUND)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Could not find the ZIP file for " + pInfo.displayName 
										+ ". Please contact the developers.");

		LOGE("Download Failed!!");
	}
	else if (dlReturnCode == UNCMP_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Could not uncompress the ZIP file. Please try again.");

		LOGE("Download Failed!!");
	}
	else if (dlReturnCode == XML_MISSING)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to locate installedPlugins.xml \n Please restart Plugin Installer and try again.");

		LOGE("XML File Missing!!");
	}
	else if (dlReturnCode == VER_EXISTS_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										pInfo.displayName + " v" + pInfo.selectedVersion 
										+ " already exists. Please download another version.");

		LOGE("Download Failed!!");
	}
	else if (dlReturnCode == XML_WRITE_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to write to installedPlugins.xml \n Please try again.");
		
		LOGE("Writing to XML Failed!!");
	}
	else if (dlReturnCode == LOAD_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to load " + pInfo.displayName 
										+ " in the Processor List.\nLook at console output for more details.");

		LOGE("Loading Plugin Failed!!");

		pInfo.installedVersion = pInfo.selectedVersion;
        
        const MessageManagerLock mmLock;
		downloadButton.setEnabled(false);
		downloadButton.setButtonText("Installed");

		if(pInfo.latestVersion.equalsIgnoreCase(pInfo.latestVersion))
		{
			updatablePlugins.removeString(pInfo.pluginName);
			this->getParentComponent()->resized();
		}
	}
	else if (dlReturnCode == PLUGIN_IN_USE || dlReturnCode == RECNODE_IN_USE)
	{
		String name = (dlReturnCode == PLUGIN_IN_USE) ? pInfo.displayName : "Record Node";
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										name + " is already in use. Please remove it from the signal chain and try again.");

		LOGE("Error.. Plugin already in use. Please remove it from the signal chain and try again.");
	}
	else if(dlReturnCode == HTTP_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"HTTP request failed!!\nPlease check your internet connection...");
		
		LOGE("HTTP request failed!! Please check your internet connection...");
	}

}
 

void PluginInfoComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == &versionMenu)
	{
		pInfo.selectedVersion = comboBoxThatHasChanged->getText();

		// Change install button name depending on the selected version of a plugin
		if (pInfo.installedVersion.isEmpty())
		{
			downloadButton.setEnabled(true);
			downloadButton.setButtonText("Install");
		}
		else
		{
			int result = pInfo.selectedVersion.compareNatural(pInfo.installedVersion);

			if (result == 0)
			{
				downloadButton.setEnabled(false);
				downloadButton.setButtonText("Installed");
			}
			else if (result > 0)
			{
				downloadButton.setEnabled(true);
				downloadButton.setButtonText("Upgrade");
			}
			else
			{
				downloadButton.setEnabled(true);
				downloadButton.setButtonText("Downgrade");
			}
		}	
	}
}

void PluginInfoComponent::setPluginInfo(const SelectedPluginInfo& p, bool shouldUpdateUI)
{
	pInfo = p;

	if(shouldUpdateUI)
	{
		pluginNameText.setText(pInfo.displayName, dontSendNotification);
		developersText.setText(pInfo.developers, dontSendNotification);
		lastUpdatedText.setText(pInfo.lastUpdated, dontSendNotification);
		descriptionText.setText(pInfo.description, dontSendNotification);
		if (pInfo.dependencies.isEmpty())
			dependencyText.setText("None", dontSendNotification);
		else
			dependencyText.setText(pInfo.dependencies.joinIntoString(", "), dontSendNotification);

		versionMenu.clear(dontSendNotification);

		if(pInfo.installedVersion.isEmpty())
			installedVerText.setText("No", dontSendNotification);
		else
			installedVerText.setText(pInfo.installedVersion, dontSendNotification);

		if (pInfo.versions.isEmpty())
		{
			downloadButton.setEnabled(false);
			downloadButton.setButtonText("Unavailable");
		}
		else
		{
			for (int i = 0; i < pInfo.versions.size(); i++)
				versionMenu.addItem(pInfo.versions[i], i + 1);

			//set default selected version to the first entry in combo box
			versionMenu.setSelectedId(1, sendNotification);
			pInfo.selectedVersion = pInfo.versions[0];
		}
	}
}

void PluginInfoComponent::updateStatusMessage(const String& str, bool isVisible)
{
	statusLabel.setText(str, dontSendNotification);
	statusLabel.setVisible(isVisible);
}

void PluginInfoComponent::makeInfoVisible(bool isEnabled)
{
	pluginNameLabel.setVisible(isEnabled);
	pluginNameText.setVisible(isEnabled);

	developersLabel.setVisible(isEnabled);
	developersText.setVisible(isEnabled);

	versionLabel.setVisible(isEnabled);
	versionMenu.setVisible(isEnabled);

	installedVerLabel.setVisible(isEnabled);
	installedVerText.setVisible(isEnabled);

	lastUpdatedLabel.setVisible(isEnabled);
	lastUpdatedText.setVisible(isEnabled);

	descriptionLabel.setVisible(isEnabled);
	descriptionText.setVisible(isEnabled);

	dependencyLabel.setVisible(isEnabled);
	dependencyText.setVisible(isEnabled);

	downloadButton.setVisible(isEnabled);
	documentationButton.setVisible(isEnabled);

	if(pInfo.installedVersion.isNotEmpty())
		uninstallButton.setVisible(isEnabled);
}


bool PluginInfoComponent::uninstallPlugin(const String& plugin)
{
	LOGC("Uninstalling plugin: ", pInfo.displayName);

	// Check whether the plugin is loaded in a signal chain
	if(AccessClass::getProcessorGraph()->processorWithSameNameExists(pInfo.displayName))
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										pInfo.displayName + " is already in use. Please remove it from the signal chain and try again.");
		
		LOGD("Plugin present in signal chain! Please remove it before uninstalling the plugin.");
		return false;	
	}
	
	// Open installedPluings.xml file
	File xmlFile = getPluginsDirectory().getChildFile("installedPlugins.xml");

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	String dllName;
	XmlElement* pluginElement;

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		LOGD("[PluginInstaller] InstalledPlugins.xml file not found.");
		return false;
	}
	else
	{	
		// Fetch plugin DLL name
		pluginElement = xml->getFirstChildElement()->getChildByName(plugin);
		dllName = pluginElement->getAttributeValue(1);
	}

	// Remove plugin from PluginManager
	if(!AccessClass::getPluginManager()->removePlugin(pInfo.displayName))
		return false;

	//delete plugin file
	File pluginFile = getPluginsDirectory().getChildFile(dllName);
	if(!pluginFile.deleteRecursively())
	{
		LOGD("Unable to delete ", pluginFile.getFullPathName(), " ...Trying again!");

#ifdef _WIN32
		const char* processorLocCString = static_cast<const char*>(pluginFile.getFullPathName().toUTF8());
		HMODULE md = GetModuleHandleA(processorLocCString);

		if(FreeLibrary(md))
			LOGD("Unloaded ", dllName);

		if(!pluginFile.deleteFile())
		{
			return false;
		}
#else
		return false;
#endif
	}

	// Remove plugin XML entry
	xml->getFirstChildElement()->removeChildElement(pluginElement, true);
	if (! xml->writeTo(xmlFile))
	{
		LOGD("Error! Couldn't write to installedPlugins.xml");
		return false;
	}

	AccessClass::getProcessorList()->fillItemList();
	AccessClass::getProcessorList()->repaint();

	if(pInfo.type == "RecordEngine")
		AccessClass::getControlPanel()->updateRecordEngineList();

	uninstallButton.setVisible(false);
	downloadButton.setEnabled(true);
	downloadButton.setButtonText("Install");
	installedVerText.setText("No", dontSendNotification);

	return true;
}

int PluginInfoComponent::downloadPlugin(const String& plugin, const String& version, bool isDependency) 
{

	String fileDownloadURL = downloadURL;
	fileDownloadURL = fileDownloadURL.replace("<plugin-name>", plugin);
	fileDownloadURL = fileDownloadURL.replace("<platform>", osType);
	fileDownloadURL = fileDownloadURL.replace("<version>", version);

	String filename = plugin + "-" + osType + "_" + version + ".zip";

	URL fileUrl(fileDownloadURL);

	//Create input stream from the plugin's zip file URL
	std::unique_ptr<InputStream> fileStream = fileUrl.createInputStream(URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                   .withConnectionTimeoutMs (0)
                                                   .withNumRedirectsToFollow (5));		

	// Could not retrieve data
	if(!fileStream)
		return 9;

	// ZIP file empty, return.
	if(fileStream->getTotalLength() == 0)
		return 0;

	//Construct path for downloaded zip file
	String pluginFilePath = CoreServices::getSavedStateDirectory().getFullPathName();
	pluginFilePath += File::getSeparatorString();
	pluginFilePath += filename;

	//Create local file
	File pluginFile(pluginFilePath);
	pluginFile.deleteFile();

	//Use the Url's input stream and write it to a file using output stream
	std::unique_ptr<FileOutputStream> out = pluginFile.createOutputStream();
	out->writeFromInputStream(*fileStream, -1);
	out->flush();
	out.reset();

	//Uncompress zip file contents
	ZipFile pluginZip(pluginFile);

	//Get *.dll/*.so name of plugin
#if JUCE_WINDOWS
	auto entry = pluginZip.getEntry(0);
#else
	auto entry = pluginZip.getEntry(1);
#endif

	String dllName = entry->filename;
	dllName = dllName.substring(dllName.indexOf(File::getSeparatorString()) + 1);

	// Open installedPluings.xml file
	File xmlFile = getPluginsDirectory().getChildFile("installedPlugins.xml");

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (!isDependency)
	{	
		// Create a new entry in xml for the downloaded plugin
		std::unique_ptr<XmlElement> pluginEntry(new XmlElement(plugin));

		// set version and dllName attributes of the plugins
		pluginEntry->setAttribute("version", version);
		pluginEntry->setAttribute("dllName", dllName);

		if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
		{
			LOGE("[PluginInstaller] File not found.");
			pluginFile.deleteFile();
			return 3;
		}
		else
		{	
			auto child = xml->getFirstChildElement();
			bool hasTag = false; 

			/** Check for whether the plugin is installed and if it has the same version
			 * 	as the one being downloaded
			 **/
			for (auto* e : child->getChildIterator())
			{
				if (e->hasTagName(pluginEntry->getTagName()))
				{
					if (e->getAttributeValue(0).equalsIgnoreCase(pluginEntry->getAttributeValue(0)))
					{
						LOGE(plugin, " v", version, " already exists!!");
						pluginFile.deleteFile();
						return 4;
					}
					else 
					{
						e->setAttribute("version", version);
					}
					hasTag = true;
				}
			}

			// if no such plugin is installed, add its info to the xml file
			if (!hasTag)
				child->addChildElement(pluginEntry.release());

		}

		// Check if plugin already present in signal chain
		if(pInfo.type == "RecordEngine" && AccessClass::getProcessorGraph()->hasRecordNode())
		{
			pluginFile.deleteFile();
			return 8;
		}
		else if(AccessClass::getProcessorGraph()->processorWithSameNameExists(pInfo.displayName))
		{
			pluginFile.deleteFile();
			return 7;	
		}
	}

	// Uncompress plugin zip file in temp directory
	String pluginDllPath;

	File tempDir = File::getSpecialLocation(File::tempDirectory).getChildFile("open-ephys");
	tempDir.createDirectory();

	pluginZip.uncompressTo(tempDir);

	if(!isDependency)
	{
		// copy plugin DLL from temp directory to actual location
		bool copySuccess = tempDir.getChildFile("plugins").getChildFile(dllName)
							.copyFileTo(getPluginsDirectory().getChildFile(dllName));

		File dllFile = getPluginsDirectory().getChildFile(dllName);
		pluginDllPath = dllFile.getFullPathName();

		if(!copySuccess && dllFile.exists())
		{
#ifdef _WIN32
			const char* processorLocCString = static_cast<const char*>(pluginDllPath.toUTF8());
			HMODULE md = GetModuleHandleA(processorLocCString);

			if(FreeLibrary(md))
				LOGD("Unloaded old ", dllName);
			
			// try copying again after unloading old DLL
			copySuccess = tempDir.getChildFile("plugins").getChildFile(dllName)
							.copyFileTo(getPluginsDirectory().getChildFile(dllName));

			if(!copySuccess)
			{
				LOGC("Unable to replace/update exisiting plugin file!");
				pluginFile.deleteFile();
				return 2;
			}
#endif		
		}
	}

	/* Copy shared files 
	*  Uses C++17's filesystem::copy functionality to allow copying symlinks
	*/
	std::filesystem::path tempSharedPath = tempDir.getChildFile("shared").getFullPathName().toStdString();
	std::filesystem::path destSharedPath = getSharedDirectory().getFullPathName().toStdString();

	// Copy only if shared files exist
	if(std::filesystem::exists(tempSharedPath))
	{
		const auto copyOptions = std::filesystem::copy_options::overwrite_existing
								| std::filesystem::copy_options::recursive
								| std::filesystem::copy_options::copy_symlinks
								;
		try {
			std::filesystem::copy(tempSharedPath, destSharedPath, copyOptions);
		} catch(std::filesystem::filesystem_error& e) {
			LOGE("Could not copy shared files: \"", e.what(), "\"");
		}
	}

	tempDir.deleteRecursively();
	pluginFile.deleteFile(); // delete zip after uncompressing

	// if the plugin is not a dependency, load the plugin and show it in processor list	
	if (!isDependency)
	{
		// Write installed plugin's info to XML file
		if (! xml->writeTo(xmlFile))
		{
			LOGE("Error! Couldn't write to installedPlugins.xml");
			return 5;
		}
		
		int loadPlugin = AccessClass::getPluginManager()->loadPlugin(pluginDllPath);

		if (loadPlugin == -1)
			return 6;

		AccessClass::getProcessorList()->fillItemList();
		AccessClass::getProcessorList()->repaint();

		if(pInfo.type == "RecordEngine")
			AccessClass::getControlPanel()->updateRecordEngineList();
		
	}

	return 1;

}
