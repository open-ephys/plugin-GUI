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

#include "../CoreServices.h"
#include "../AccessClass.h"
#include "../Processors/PluginManager/PluginManager.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "ProcessorList.h"
#include "ControlPanel.h"
#ifdef WIN32
#include <Windows.h>
#endif


//-----------------------------------------------------------------------

static inline File getPluginsLocationDirectory() {
#if defined(__APPLE__)
    File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
#elif _WIN32
    File dir = File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Open Ephys");
#else
	File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".open-ephys");
#endif
    return std::move(dir);
}

static String osType;

PluginInstaller::PluginInstaller(MainWindow* mainWindow)
: DocumentWindow(WINDOW_TITLE,
		Colour(Colours::black),
		DocumentWindow::closeButton)
{

	MouseCursor::showWaitCursor();
	parent = (DocumentWindow*)mainWindow;

	setResizable(
        true,  // isResizable
		false); // useBottomCornerRisizer -- doesn't work very well

   // Identify the OS on which the GUI is running
	SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();

	if ((os & SystemStats::OperatingSystemType::Windows) != 0)
	 	osType = "windows";
	else if ((os & SystemStats::OperatingSystemType::MacOSX) != 0)
	 	osType = "mac";
	else if ((os & SystemStats::OperatingSystemType::Linux) != 0)
	 	osType = "linux";

	//Initialize Plugin Installer Components

	int x = parent->getX();
	int y = parent->getY();
	int w = parent->getWidth();
	int h = parent->getHeight();

	setBounds(x + (0.5*w) - 427, y + 0.5*h - 240, 854, 480);

	setUsingNativeTitleBar(true);
	setContentOwned(new PluginInstallerComponent(), false);
	setVisible(true);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(854, 480, 8192, 5120);

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
	File pluginsDir = getPluginsLocationDirectory().getChildFile("plugins");
    if (!pluginsDir.isDirectory())
        pluginsDir.createDirectory();
    
    String xmlFile = "plugins" + File::separatorString + "installedPlugins.xml";
	File file = getPluginsLocationDirectory().getChildFile(xmlFile);

	XmlDocument doc(file);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		std::unique_ptr<XmlElement> baseTag(new XmlElement("PluginInstaller"));
		baseTag->setAttribute("gui_version", JUCEApplication::getInstance()->getApplicationVersion());

		std::unique_ptr<XmlElement> plugins(new XmlElement("InstalledPlugins"));

		baseTag->addChildElement(plugins.release());

		if (! baseTag->writeToFile(file, String::empty))
			std::cout << "Error! Couldn't write to installedPlugins.xml" << std::endl;
	}
	else
	{
		String baseStr = "plugins" + File::separatorString;

		auto child = xml->getFirstChildElement();
		Array<XmlElement*> elementsToRemove;

		forEachXmlChildElement(*child, e)
		{
			File pluginPath = getPluginsLocationDirectory().getChildFile(baseStr + e->getAttributeValue(1));
			if (!pluginPath.exists())
				elementsToRemove.add(e);	
		}

		for (auto element : elementsToRemove)
		{
			child->removeChildElement(element, true);
		}

		if (! xml->writeToFile(file, String::empty))
		{
			std::cout << "Error! Couldn't write to installedPlugins.xml" << std::endl;
		}

		elementsToRemove.clear();

	}
}

int versionCompare(const String& v1, const String& v2)
{ 
    String nv1 = v1.substring(0, v1.indexOf("-"));
	String nv2 = v2.substring(0, v2.indexOf("-"));

	//  vnum stores each numeric part of version 
    int vnum1 = 0, vnum2 = 0; 
  
    //  loop untill both numeric versions are processed 
    for (int i=0, j=0; (i<nv1.length() || j<nv2.length()); ) 
    { 
        //  storing numeric part of version 1 in vnum1 
        while (i < nv1.length() && nv1[i] != '.') 
        { 
            vnum1 = vnum1 * 10 + (nv1[i] - '0'); 
            i++; 
        } 
  
        //  storing numeric part of version 2 in vnum2 
        while (j < nv2.length() && nv2[j] != '.') 
        { 
            vnum2 = vnum2 * 10 + (nv2[j] - '0'); 
            j++; 
        } 
  
        if (vnum1 > vnum2) 
            return 1; 
        if (vnum2 > vnum1) 
            return -1; 
  
        //  if equal, reset variables and go for next numeric part 
        vnum1 = vnum2 = 0; 
        i++; 
        j++; 
    } 

	// Numeric versions match, check API versions
	int av1 = v1.substring(v1.indexOf("I") + 1).getIntValue();
	int av2 = v2.substring(v2.indexOf("I") + 1).getIntValue();

	if (av1 > av2)
		return 1;
	if (av1 < av2)
		return -1;

    return 0; 
}


/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent() : ThreadWithProgressWindow("Plugin Installer", false, false)
{
	font = Font("FiraSans", 18, Font::plain);
	setSize(getWidth() - 10, getHeight() - 10);

	addAndMakeVisible(pluginListAndInfo);

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
	installedButton.setColour(ToggleButton::textColourId, Colours::white);
	installedButton.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	installedButton.setRadioGroupId(101, dontSendNotification);
	installedButton.addListener(this);

	addAndMakeVisible(updatesButton);
	updatesButton.setButtonText("Updates");
	updatesButton.setColour(ToggleButton::textColourId, Colours::white);
	updatesButton.setColour(ToggleButton::tickDisabledColourId, Colours::lightgrey);
	updatesButton.setRadioGroupId(101, dontSendNotification);
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
	g.fillRect(192, 5, 3, 38);
	g.fillRect(490, 5, 3, 38);
}

void PluginInstallerComponent::resized()
{
	sortingLabel.setBounds(20, 10, 70, 30);
	sortByMenu.setBounds(90, 10, 90, 30);

	viewLabel.setBounds(200, 10, 50, 30);
	allButton.setBounds(250, 10, 50, 30);
	installedButton.setBounds(300, 10, 90, 30);
	updatesButton.setBounds(390, 10, 90, 30);

	typeLabel.setBounds(500, 10, 45, 30);
	sourceType.setBounds(545, 10, 80, 30);
	filterType.setBounds(625, 10, 70, 30);
	sinkType.setBounds(695, 10, 60, 30);
	otherType.setBounds(755, 10, 65, 30);

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
	String fileStr = "plugins" + File::separatorString + "installedPlugins.xml";
	File xmlFile = getPluginsLocationDirectory().getChildFile(fileStr);

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		std::cout << "[PluginInstaller] File not found." << std::endl;
		return;
	}
	else
	{	
		installedPlugins.clear();
		updatablePlugins.clear();
		auto child = xml->getFirstChildElement();

		String baseUrl = "https://api.bintray.com/packages/open-ephys-gui-plugins/";

		forEachXmlChildElement(*child, e)
		{
			String pName = e->getTagName();
			installedPlugins.add(pName);

			if (updatesButton.getToggleState())
			{
				setStatusMessage("Fetching plugin updates...");
				//Get latest version
				String versionUrl = baseUrl + pName + "/" + pName + "-" + osType + "/versions/_latest";

				String vResponse = URL(versionUrl).readEntireTextStream();
				var vReply = JSON::parse(vResponse);

				String latest_ver = vReply.getProperty("name", "NULL").toString();

				if (versionCompare(latest_ver, e->getAttributeValue(0)) > 0)
					updatablePlugins.add(pName);
			}
		}
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
		this->runThread();
		
		pluginListAndInfo.pluginArray.clear();
		pluginListAndInfo.pluginArray.addArray(updatablePlugins);
		pluginListAndInfo.setNumRows(updatablePlugins.size());
	}

	if ( button == &sourceType || button == &filterType || button == &sinkType || button == &otherType)
	{
		bool sourceState = sourceType.getToggleState();
		bool filterState = filterType.getToggleState();
		bool sinkState = sinkType.getToggleState();
		bool otherState = otherType.getToggleState();

		if( sourceState || filterState || sinkState || otherState)
		{
			String baseUrl = "https://api.bintray.com/repos/open-ephys-gui-plugins/";

			StringArray tempArray;

			pluginListAndInfo.pluginArray.clear();

			if(installedButton.getToggleState())
				tempArray.addArray(installedPlugins);
			else if(updatesButton.getToggleState())
				tempArray.addArray(updatablePlugins);
			else
				tempArray.addArray(allPlugins);

			for (int i = 0; i < tempArray.size(); i++)
			{
				StringArray labels;

				labels = pluginListAndInfo.pluginLabels[tempArray[i]];
				
				int containsType = 0;

				bool isSource = labels.contains("source", true);
				bool isFilter = labels.contains("filter", true);
				bool isSink = labels.contains("sink", true);
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

PluginListBoxComponent::PluginListBoxComponent() : ThreadWithProgressWindow("Loading Plugin Installer", true, false)
{
	listFont = Font("FiraSans Bold", 22, Font::plain);

	// Set progress window text and background colors
	auto window = this->getAlertWindow();
	window->setColour(AlertWindow::textColourId, Colours::white);
	window->setColour(AlertWindow::backgroundColourId, Colour::fromRGB(50, 50, 50));
	setStatusMessage("Fetching plugins ...");

	this->runThread(); //Load all plugin names and labels from bintray

	addAndMakeVisible(pluginList);
	pluginList.setModel(this);
	pluginList.setColour(ListBox::backgroundColourId , Colour::fromRGB(50, 50, 50));
	pluginList.setRowHeight(35);
	pluginList.setMouseMoveSelectsRows(true);

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

	StringArray pLabels = pluginLabels[pluginArray[rowNumber]];
	String text = pLabels[0];

	g.drawText (text, 20, 0, width - 10, height, Justification::centredLeft, true);
}

void PluginListBoxComponent::run()
{
	/* Get list of plugins uploaded to bintray */
	String baseUrl = "https://api.bintray.com/repos/open-ephys-gui-plugins";
	String response = URL(baseUrl).readEntireTextStream();

	var pluginData = JSON::parse(response);

	numRows = pluginData.size();
	
	String pluginName;

	int pluginTextWidth;

	// Get each plugin's labels and add them to the list
	for (int i = 0; i < numRows; i++)
	{		
		pluginName = pluginData[i].getProperty("name", var()).toString();

		setStatusMessage("Fetching " + pluginName + " ...");

		pluginTextWidth = listFont.getStringWidth(pluginName);
		if (pluginTextWidth > maxTextWidth)
			maxTextWidth = pluginTextWidth;
		
		String pluginUrl = baseUrl + "/" + pluginName;
		response = URL(pluginUrl).readEntireTextStream();
		var labelData = JSON::parse(response);

		StringArray labels;
		auto allLabels = labelData.getProperty("labels", "NULL").getArray();

		for(String label : *allLabels)
			labels.add(label);
		
		if(!labels.contains("Dependency", true))
		{
			pluginArray.add(pluginName);
			pluginLabels.set(pluginName, labels);
		}

		setProgress ((i + 1) / (double) numRows);
	}
	setNumRows(pluginArray.size());
}

bool PluginListBoxComponent::loadPluginInfo(const String& pluginName)
{
	// Find out all available packages for the plugin
	String url, version_url;
	
	url = "https://api.bintray.com/repos/open-ephys-gui-plugins/";
	url+=pluginName;
	url+="/packages";

	String packageResponse = URL(url).readEntireTextStream();

	var packageReply = JSON::parse(packageResponse);

	Array<String> packages;

	for (int i = 0; i < packageReply.size(); i++)
	{
	 	packages.add(packageReply[i].getProperty("name", var()).toString());
	}
		
	// Select platform specific package for the plugin
	String selectedPackage;
	for (int i = 0; i < packages.size(); i++)
	{
	 	if(packages[i].contains(osType))
	 	{
	 		selectedPackage = packages[i];
	 		break;
	 	}
	}

	if(selectedPackage.isEmpty())
	{
	 	std::cout << "*********** No platform specific package found for " << pluginName << std::endl;
		pluginInfoPanel.makeInfoVisible(false);
		return false;
	}

	//Get latest version
	version_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	version_url+=pluginName;
	version_url+="/";
	version_url+=selectedPackage;

	String version_response = URL(version_url).readEntireTextStream();;

	var version_reply = JSON::parse(version_response);

	String owner= version_reply.getProperty("owner", "NULL");
	String latest_version = version_reply.getProperty("latest_version", "NULL");
	String updated = version_reply.getProperty("updated", "NULL");
	updated = updated.substring(0, updated.indexOf("T")) + " at " 
			  + updated.substring(updated.indexOf("T") + 1, updated.indexOf("."));
	String description = version_reply.getProperty("desc", "NULL");

	auto allVersions = version_reply.getProperty("versions", "NULL").getArray();

	selectedPluginInfo.versions.clear();

	for (String version : *allVersions)
	{
		String apiVer = version.substring(version.indexOf("I") + 1);
		
		if (apiVer.equalsIgnoreCase(String(PLUGIN_API_VER)))
			selectedPluginInfo.versions.add(version);
	}

	auto dependencies = version_reply.getProperty("attribute_names", "NULL").getArray();
	selectedPluginInfo.dependencies.clear();
	for (String dependency : *dependencies)
		selectedPluginInfo.dependencies.add(dependency);

	selectedPluginInfo.docURL = version_reply.getProperty("website_url", "NULL").toString();
	selectedPluginInfo.selectedVersion = String();

	selectedPluginInfo.pluginName = pluginName;
	selectedPluginInfo.displayName = pluginLabels[pluginName][0];
	selectedPluginInfo.type = pluginLabels[pluginName][1];
	selectedPluginInfo.owner = owner;
	selectedPluginInfo.latestVersion = latest_version;
	selectedPluginInfo.lastUpdated = updated;
	selectedPluginInfo.description = description;

	// If the plugin is already installed, get installed version number
	String fileStr = "plugins" + File::separatorString + "installedPlugins.xml";
	File xmlFile = getPluginsLocationDirectory().getChildFile(fileStr);

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		std::cout << "[PluginInstaller] File not found." << std::endl;
		return false;
	}
	else
	{	
		auto child = xml->getFirstChildElement();

		auto pluginEntry = child->getChildByName(pluginName);

		if (pluginEntry != nullptr)
			selectedPluginInfo.installedVersion = pluginEntry->getAttributeValue(0);
		else
			selectedPluginInfo.installedVersion = String::empty;
		
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
    pluginList.setBounds(10, 10, maxTextWidth + 60, getHeight() - 30);
	pluginInfoPanel.setBounds(maxTextWidth + 80, 10, 
							  getWidth() - maxTextWidth - 100, getHeight() - 30);
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
	infoFont = Font("FiraSans", 20, Font::plain);
	infoFontBold = Font("FiraSans Bold", 20, Font::plain);
	
	addChildComponent(pluginNameLabel);
	pluginNameLabel.setFont(infoFontBold);
	pluginNameLabel.setColour(Label::textColourId, Colours::white);
	pluginNameLabel.setText("Name: ", dontSendNotification);

	addChildComponent(pluginNameText);
	pluginNameText.setFont(infoFont);
	pluginNameText.setColour(Label::textColourId, Colours::white);

	addChildComponent(ownerLabel);
	ownerLabel.setFont(infoFontBold);
	ownerLabel.setColour(Label::textColourId, Colours::white);
	ownerLabel.setText("Owner: ", dontSendNotification);

	addChildComponent(ownerText);
	ownerText.setFont(infoFont);
	ownerText.setColour(Label::textColourId, Colours::white);

	addChildComponent(versionLabel);
	versionLabel.setFont(infoFontBold);
	versionLabel.setColour(Label::textColourId, Colours::white);
	versionLabel.setText("Version: ", dontSendNotification);

	addChildComponent(versionMenu);
	versionMenu.setJustificationType(Justification::centred);
	versionMenu.setTextWhenNoChoicesAvailable("- N/A -");
	versionMenu.addListener(this);

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
	descriptionText.setJustificationType(Justification::top);
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
	pluginNameLabel.setBounds(10, 30, 60, 30);
	pluginNameText.setBounds(125, 30, getWidth() - 10, 30);

	ownerLabel.setBounds(10, 60, 60, 30);
	ownerText.setBounds(125, 60, getWidth() - 10, 30);

	versionLabel.setBounds(10, 90, 80, 30);
	versionMenu.setBounds(130, 90, 110, 30);

	lastUpdatedLabel.setBounds(10, 120, 120, 30);
	lastUpdatedText.setBounds(125, 120, getWidth() - 10, 30);

	descriptionLabel.setBounds(10, 150, 110, 30);
	descriptionText.setBounds(125, 155, getWidth() - 130, 75);

	dependencyLabel.setBounds(10, 160 + descriptionText.getHeight(), 120, 30);
	dependencyText.setBounds(125, dependencyLabel.getY(), getWidth() - 10, 30);

	downloadButton.setBounds(getWidth() - (getWidth() * 0.25) - 20, getHeight() - 60, getWidth() * 0.25, 30);
	documentationButton.setBounds(20, getHeight() - 60, getWidth() * 0.25, 30);
	
	statusLabel.setBounds(10, (getHeight() / 2) - 15, getWidth() - 10, 30);
}

void PluginInfoComponent::buttonClicked(Button* button)
{
	if (button == &downloadButton)
	{
		this->runThread();
	}
	else if (button == &documentationButton)
	{
		URL url = URL(pInfo.docURL);
		url.launchInDefaultBrowser();
	}
}

void PluginInfoComponent::run()
{
	std::cout << "\nDownloading Plugin: " << pInfo.pluginName << "...  " << std::endl;
		
	// If a plugin has depencies outside its zip, download them
	for (int i = 0; i < pInfo.dependencies.size(); i++)
	{
		String depUrl = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
		depUrl += pInfo.dependencies[i] + "/" + pInfo.dependencies[i] + "-" ;
		depUrl += osType + "/versions/_latest";

		setStatusMessage("Downloading dependency: " + pInfo.dependencies[i]);

		String depResponse = URL(depUrl).readEntireTextStream();
		var depReply = JSON::parse(depResponse);
		String ver = depReply.getProperty("name", "NULL");

		int retCode = downloadPlugin(pInfo.dependencies[i], ver, true);

		if (retCode == 2)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
											"[Plugin Installer] " + pInfo.dependencies[i], 
											"Could not install dependency: " + pInfo.dependencies[i] 
											+ ". Please contact the developers.");
			
			std::cout << "Download Failed!!" << std::endl;
			return;
		}
	}
	
	setStatusMessage("Downloading " + pInfo.displayName + " ...");

	// download the plugin
	int dlReturnCode = downloadPlugin(pInfo.pluginName, pInfo.selectedVersion, false);

	if (dlReturnCode == SUCCESS)
	{	
		AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										pInfo.displayName + " Installed Successfully");

		std::cout << "Download Successfull!!" << std::endl;

		pInfo.installedVersion = pInfo.selectedVersion;
		downloadButton.setEnabled(false);
		downloadButton.setButtonText("Installed");
	}
	else if (dlReturnCode == ZIP_NOTFOUND)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Could not find the ZIP file for " + pInfo.displayName 
										+ ". Please contact the developers.");

		std::cout << "Download Failed!!" << std::endl;
	}
	else if (dlReturnCode == UNCMP_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Could not uncompress the ZIP file. Please try again.");

		std::cout << "Download Failed!!" << std::endl;
	}
	else if (dlReturnCode == XML_MISSING)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to locate installedPlugins.xml \n Please restart Plugin Installer and try again.");

		std::cout << "XML File Missing!!" << std::endl;
	}
	else if (dlReturnCode == VER_EXISTS_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										pInfo.displayName + " v" + pInfo.selectedVersion 
										+ " already exists. Please download another version.");

		std::cout << "Download Failed!!" << std::endl;
	}
	else if (dlReturnCode == XML_WRITE_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to write to installedPlugins.xml \n Please try again.");
		
		std::cout << "Writing to XML Failed!!" << std::endl;
	}
	else if (dlReturnCode == LOAD_ERR)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Unable to load " + pInfo.displayName 
										+ " in the Processor List.\nLook at console output for more details.");

		std::cout << "Loading Plugin Failed!!" << std::endl;

		pInfo.installedVersion = pInfo.selectedVersion;
		downloadButton.setEnabled(false);
		downloadButton.setButtonText("Installed");
	}
	else if (dlReturnCode == PROC_IN_USE)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
										"[Plugin Installer] " + pInfo.displayName, 
										"Plugin already in use. Please remove it from the signal chain and try again.");

		std::cout << "Error.. Plugin already in use. Please remove it from the signal chain and try again." << std::endl;
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
			int result = versionCompare(pInfo.selectedVersion, pInfo.installedVersion);

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

void PluginInfoComponent::setPluginInfo(const SelectedPluginInfo& p)
{
	pInfo = p;
	pluginNameText.setText(pInfo.displayName, dontSendNotification);
	ownerText.setText(pInfo.owner, dontSendNotification);
	lastUpdatedText.setText(pInfo.lastUpdated, dontSendNotification);
	descriptionText.setText(pInfo.description, dontSendNotification);
	if (pInfo.dependencies.isEmpty())
		dependencyText.setText("None", dontSendNotification);
	else
		dependencyText.setText(pInfo.dependencies.joinIntoString(", "), dontSendNotification);

	versionMenu.clear(dontSendNotification);

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

void PluginInfoComponent::updateStatusMessage(const String& str, bool isVisible)
{
	statusLabel.setText(str, dontSendNotification);
	statusLabel.setVisible(isVisible);
}

void PluginInfoComponent::makeInfoVisible(bool isEnabled)
{
	pluginNameLabel.setVisible(isEnabled);
	pluginNameText.setVisible(isEnabled);

	ownerLabel.setVisible(isEnabled);
	ownerText.setVisible(isEnabled);

	versionLabel.setVisible(isEnabled);
	versionMenu.setVisible(isEnabled);

	lastUpdatedLabel.setVisible(isEnabled);
	lastUpdatedText.setVisible(isEnabled);

	descriptionLabel.setVisible(isEnabled);
	descriptionText.setVisible(isEnabled);

	dependencyLabel.setVisible(isEnabled);
	dependencyText.setVisible(isEnabled);

	downloadButton.setVisible(isEnabled);
	documentationButton.setVisible(isEnabled);
}

int PluginInfoComponent::downloadPlugin(const String& plugin, const String& version, bool isDependency) 
{

	String files_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	files_url += plugin;
	files_url += "/";
	files_url += plugin + "-" + osType;
	files_url += "/versions/";
	files_url += version;
	files_url += "/files";

	String files_response = URL(files_url).readEntireTextStream();;

	var files_reply = JSON::parse(files_response);

	String filename;

	if (files_reply.size())
	{
		for (int i = 0; i < files_reply.size(); i++)
		{
			filename = files_reply[i].getProperty("name", "NULL").toString();
		}

		//Unzip plugin and install in plugins directory
		//curl -L https://dl.bintray.com/$bintrayUser/$repo/$filename
		String fileDownloadURL = "https://dl.bintray.com/open-ephys-gui-plugins/";
		fileDownloadURL += plugin;
		fileDownloadURL += "/";
		fileDownloadURL += filename;

		URL fileUrl(fileDownloadURL);

		int statusC;
		StringPairArray responseHeaders;

		//Create input stream from the plugin's zip file URL
		ScopedPointer<InputStream> fileStream = fileUrl.createInputStream(false, nullptr, nullptr, String(), 1000, &responseHeaders, &statusC, 5, String());		
		String newLocation = responseHeaders.getValue("Location", "NULL");

		// ZIP URL Location changed, use the new location
		if(newLocation != "NULL")
		{
			fileUrl = newLocation;
			fileStream = fileUrl.createInputStream(false);
		}

		// ZIP file empty, return.
		if(fileStream->getTotalLength() == 0)
			return 0;

		//Get path to plugins directory
		File pluginsPath = getPluginsLocationDirectory();

		//Construct path for downloaded zip file
		String pluginFilePath = pluginsPath.getFullPathName();
		pluginFilePath += File::separatorString;
		pluginFilePath += filename;

		//Create local file
		File pluginFile(pluginFilePath);
		pluginFile.deleteFile();

		//Use the Url's input stream and write it to a file using output stream
		FileOutputStream* out = pluginFile.createOutputStream();
		out->writeFromInputStream(*fileStream, -1);
		out->flush();
		delete out;

		//Uncompress zip file contents
		ZipFile pluginZip(pluginFile);

		//Get *.dll/*.so name of plugin
#if JUCE_WINDOWS
		auto entry = pluginZip.getEntry(0);
#else
		auto entry = pluginZip.getEntry(1);
#endif

		// Open installedPluings.xml file
		String fileStr = "plugins" + File::separatorString + "installedPlugins.xml";
		File xmlFile = pluginsPath.getChildFile(fileStr);

		XmlDocument doc(xmlFile);
		std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

		if (!isDependency)
		{	
			// Create a new entry in xml for the downloaded plugin
			std::unique_ptr<XmlElement> pluginEntry(new XmlElement(plugin));

			// set version and dllName attributes of the plugins
			pluginEntry->setAttribute("version", version);
			String dllName = entry->filename;
			dllName = dllName.substring(dllName.indexOf(File::separatorString) + 1);
			pluginEntry->setAttribute("dllName", dllName);

			if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
			{
				std::cout << "[PluginInstaller] File not found." << std::endl;
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
				forEachXmlChildElement(*child, e)
				{
					if (e->hasTagName(pluginEntry->getTagName()))
					{
						if (e->getAttributeValue(0).equalsIgnoreCase(pluginEntry->getAttributeValue(0)))
						{
							std::cout << plugin << " v" << version << " already exists!!" << std::endl;
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
			bool procInSignalChain;
			if(pInfo.type == "Record Engine")
				procInSignalChain = AccessClass::getProcessorGraph()->hasRecordNode();
			else
				procInSignalChain = AccessClass::getProcessorGraph()->processorWithSameNameExists(pInfo.displayName);
				
			if(procInSignalChain)
			{
				pluginFile.deleteFile();
				return 7;
			}
		}

		// Uncompress the downloaded plugin's zip file
		Result rs = pluginZip.uncompressTo(pluginsPath, true);

		if (rs.failed())
		{
			String errorMsg = rs.getErrorMessage();

			if(errorMsg.containsIgnoreCase("Failed to write to target file"))
			{
				if(!isDependency)
				{
#ifdef WIN32
					String dllToUnload = errorMsg.substring(errorMsg.lastIndexOf("\\") + 1);
					const char* processorLocCString = static_cast<const char*>(dllToUnload.toUTF8());
					HMODULE md = GetModuleHandleA(processorLocCString);

					if(FreeLibrary(md))
						std::cout << "Unloaded old " << dllToUnload << std::endl;
#endif
					rs = pluginZip.uncompressTo(pluginsPath, true);

					if(rs.failed())
					{
						std::cout <<  rs.getErrorMessage() << std::endl;
						pluginFile.deleteFile();
						return 2;
					}
				}
				else
				{
					std::cout << "Dependency already exists" << std::endl;
				}
				

			}
			else
			{
				std::cout << rs.getErrorMessage() << std::endl;
				pluginFile.deleteFile(); // delete zip after uncompressing
				return 2;
			}
		}

		pluginFile.deleteFile(); // delete zip after uncompressing

		// if the plugin is not a dependency, load the plugin and show it in processor list	
		if (!isDependency)
		{
			// Write installed plugin's info to XML file
			if (! xml->writeToFile(xmlFile, String::empty))
			{
				std::cout << "Error! Couldn't write to installedPlugins.xml" << std::endl;
				return 5;
			}
			
			String libName = pluginsPath.getFullPathName() + File::separatorString + entry->filename;

			int loadPlugin = AccessClass::getPluginManager()->loadPlugin(libName);

			if (loadPlugin == -1)
				return 6;

			AccessClass::getProcessorList()->fillItemList();
			AccessClass::getProcessorList()->repaint();

			if(pInfo.type == "Record Engine")
				AccessClass::getControlPanel()->updateRecordEngineList();
			
			return 1;
		}

	}

	return 0;
}