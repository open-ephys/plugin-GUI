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
#include "ProcessorList.h"
//-----------------------------------------------------------------------

static inline File getPluginsLocationDirectory() {
#if defined(__APPLE__)
    File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
    if (!dir.isDirectory()) {
        dir.createDirectory();
    }
    return std::move(dir);
#else
    return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
#endif
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

    //TODO: Add command manager for hot-key functionality later...

    /*
	commandManager.registerAllCommandsForTarget(ui);
	commandManager.registerAllCommandsForTarget(JUCEApplication::getInstance());
	ui->setApplicationCommandManagerToWatch(&commandManager);
	addKeyListener(commandManager.getKeyMappings());
    */

   // Identify the OS on which the GUI is running
	SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();

	if ((os & SystemStats::OperatingSystemType::Windows) != 0)
	 	osType = "windows";
	else if ((os & SystemStats::OperatingSystemType::MacOSX) != 0)
	 	osType = "mac";
	else if ((os & SystemStats::OperatingSystemType::Linux) != 0)
	 	osType = "linux";

	//Initialize Plugin Installer Components
	setUsingNativeTitleBar(true);
	setContentOwned(new PluginInstallerComponent(), false);
	setVisible(true);
	
	int x = parent->getX();
	int y = parent->getY();
	int w = parent->getWidth();
	int h = parent->getHeight();

	setBounds(x + (0.5*w) - 427, y + 0.5*h - 240, 854, 480);

	// Constraining the window's size doesn't seem to work:
	setResizeLimits(640, 480, 8192, 5120);

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
}

/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent()
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
	allButton.setRadioGroupId(101, dontSendNotification);
	allButton.addListener(this);	
	allButton.setToggleState(true, dontSendNotification);

	addAndMakeVisible(installedButton);
	installedButton.setButtonText("Installed");
	installedButton.setColour(ToggleButton::textColourId, Colours::white);
	installedButton.setRadioGroupId(101, dontSendNotification);
	installedButton.addListener(this);

	addAndMakeVisible(updatesButton);
	updatesButton.setButtonText("Updates");
	updatesButton.setColour(ToggleButton::textColourId, Colours::white);
	updatesButton.setRadioGroupId(101, dontSendNotification);
	updatesButton.addListener(this);

	addAndMakeVisible(typeLabel);
	typeLabel.setColour(Label::textColourId, Colours::white);
	typeLabel.setFont(font);
	typeLabel.setText("Type:", dontSendNotification);

	addAndMakeVisible(filterType);
	filterType.setButtonText("Filter");
	filterType.setColour(ToggleButton::textColourId, Colours::white);
	filterType.addListener(this);
	filterType.setToggleState(true, dontSendNotification);	

	addAndMakeVisible(sourceType);
	sourceType.setButtonText("Source");
	sourceType.setColour(ToggleButton::textColourId, Colours::white);
	sourceType.addListener(this);
	sourceType.setToggleState(true, dontSendNotification);

	addAndMakeVisible(sinkType);
	sinkType.setButtonText("Sink");
	sinkType.setColour(ToggleButton::textColourId, Colours::white);
	sinkType.addListener(this);
	sinkType.setToggleState(true, dontSendNotification);
}

void PluginInstallerComponent::paint(Graphics& g)
{
	g.fillAll (Colours::darkgrey);
	g.setColour(Colours::lightgrey);
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

	typeLabel.setBounds(500, 10, 50, 30);
	sourceType.setBounds(550, 10, 80, 30);
	filterType.setBounds(630, 10, 70, 30);
	sinkType.setBounds(700, 10, 60, 30);

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

void PluginInstallerComponent::loadInstalledPluginNames()
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
				//Get latest version
				String versionUrl = baseUrl + pName + "/" + pName + "-" + osType + "/versions/_latest";

				String vResponse = URL(versionUrl).readEntireTextStream();
				var vReply = JSON::parse(vResponse);

				String latest_ver = vReply.getProperty("name", "NULL").toString();

				if (!latest_ver.equalsIgnoreCase(e->getAttributeValue(0)))
					updatablePlugins.add(pName);
			}
		}
	}
}

void PluginInstallerComponent::buttonClicked(Button* button)
{
	if(allPlugins.isEmpty())
	{
		allPlugins.addArray(pluginListAndInfo.pluginArray);
	}

	if(button == &installedButton)
	{
		//if(installedPlugins.isEmpty())
		loadInstalledPluginNames();
		
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
		//if(installedPlugins.isEmpty())
		loadInstalledPluginNames();
		
		pluginListAndInfo.pluginArray.clear();
		pluginListAndInfo.pluginArray.addArray(updatablePlugins);
		pluginListAndInfo.setNumRows(updatablePlugins.size());
	}

	if ( button == &sourceType || button == &filterType || button == &sinkType )
	{
		bool sourceState = sourceType.getToggleState();
		bool filterState = filterType.getToggleState();
		bool sinkState = sinkType.getToggleState();

		if( sourceState || filterState || sinkState)
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

				if(sourceState && labels.contains("source", true))
					containsType++;
				
				if(filterState && labels.contains("filter", true))
					containsType++;

				if(sinkState && labels.contains("sink", true))
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

PluginListBoxComponent::PluginListBoxComponent()
{
	listFont = Font("FiraSans Bold", 22, Font::plain);

	loadAllPluginNames();

	addAndMakeVisible(pluginList);
	pluginList.setModel(this);
	pluginList.setColour(ListBox::backgroundColourId , Colours::grey);
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
		g.fillAll(Colour::fromRGBA(238, 238, 238, 100));
		g.setColour (Colours::darkgrey);
	}
	else
	{
		g.fillAll(Colours::grey);
		g.setColour (Colours::white);
	}

	if ( rowNumber == pluginArray.indexOf(lastPluginSelected, true, 0) )
	{
		g.setColour (Colours::yellow);
	}

	g.setFont(listFont);

	String text = pluginArray[rowNumber];

	g.drawText (text, 20, 0, width - 10, height, Justification::centredLeft, true);
}

void PluginListBoxComponent::loadAllPluginNames()
{
	/* Get list of plugins uploaded to bintray */
	String baseUrl = "https://api.bintray.com/repos/open-ephys-gui-plugins";
	String response = URL(baseUrl).readEntireTextStream();

	var pluginData = JSON::parse(response);

	numRows = pluginData.size();
	
	String pluginName;

	int pluginTextWidth;

	for (int i = 0; i < numRows; i++)
	{		
		pluginName = pluginData[i].getProperty("name", var()).toString();

		pluginTextWidth = listFont.getStringWidth(pluginName);
		if (pluginTextWidth > maxTextWidth)
			maxTextWidth = pluginTextWidth;
		
		String pluginUrl = baseUrl + "/" + pluginName;
		response = URL(pluginUrl).readEntireTextStream();
		var labelData = JSON::parse(response);

		StringArray labels;
		for(String label : *labelData.getProperty("labels", "NULL").getArray())
			labels.add(label);
		
		if(!labels.contains("Dependency", true))
		{
			pluginArray.add(pluginName);
			pluginLabels.set(pluginName, labels);
		}
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
	String description = version_reply.getProperty("desc", "NULL");

	auto allVersions = version_reply.getProperty("versions", "NULL").getArray();

	selectedPluginInfo.versions.clear();

	for (String version : *allVersions)
		selectedPluginInfo.versions.add(version);

	selectedPluginInfo.docURL = version_reply.getProperty("vcs_url", "NULL").toString();
	selectedPluginInfo.selectedVersion = String();

	selectedPluginInfo.pluginName = pluginName;
	selectedPluginInfo.packageName = selectedPackage;
	selectedPluginInfo.owner = owner;
	selectedPluginInfo.latestVersion = latest_version;
	selectedPluginInfo.lastUpdated = updated;
	selectedPluginInfo.description = description;
	selectedPluginInfo.dependencies = " ";

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

PluginInfoComponent::PluginInfoComponent()
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
	downloadButton.setButtonText("Download");
	downloadButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	downloadButton.addListener(this);

	addChildComponent(documentationButton);
	documentationButton.setButtonText("Open Documentation");
	documentationButton.setColour(TextButton::buttonColourId, Colours::lightgrey);
	documentationButton.addListener(this);

	addAndMakeVisible(statusLabel);
	statusLabel.setFont(infoFont);
	statusLabel.setColour(Label::textColourId, Colours::white);
	statusLabel.setText("Please select a plugin from the list on the left...", dontSendNotification);
}

void PluginInfoComponent::paint(Graphics& g)
{
	g.fillAll (Colours::grey);
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

	downloadButton.setBounds(getWidth() - (getWidth() * 0.4) - 20, getHeight() - 60, getWidth() * 0.4, 30);
	documentationButton.setBounds(20, getHeight() - 60, getWidth() * 0.4, 30);
	
	statusLabel.setBounds(10, (getHeight() / 2) - 15, getWidth() - 10, 30);
}

void PluginInfoComponent::buttonClicked(Button* button)
{
	if (button == &downloadButton)
	{
		std::cout << "Downloading Plugin: " << pInfo.pluginName << "...  ";
		
		int dlReturnCode = downloadPlugin(pInfo.pluginName, pInfo.packageName, pInfo.selectedVersion);

		if (dlReturnCode == SUCCESS)
		{	
			AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   pInfo.pluginName + " Installed Successfully");

			std::cout << "Download Successfull!!" << std::endl;

			pInfo.installedVersion = pInfo.selectedVersion;
			downloadButton.setEnabled(false);
			downloadButton.setButtonText("Installed");
		}
		else if (dlReturnCode == ZIP_NOTFOUND)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Could not find the ZIP file for " + pInfo.pluginName + ". Please contact the developers.");

			std::cout << "Download Failed!!" << std::endl;
		}
		else if (dlReturnCode == UNCMP_ERR)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Could not uncompress the ZIP file. Please try again.");

			std::cout << "Download Failed!!" << std::endl;
		}
		else if (dlReturnCode == XML_MISSING)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Unable to locate installedPlugins.xml. Please restart Plugin Installer and try again.");

			std::cout << "Download Failed!!" << std::endl;
		}
		else if (dlReturnCode == VER_EXISTS_ERR)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   pInfo.pluginName + " v" + pInfo.selectedVersion + " already exists. Please download another version.");

			std::cout << "Download Failed!!" << std::endl;
		}
		else if (dlReturnCode == XML_WRITE_ERR)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Unable to write to installedPlugins.xml \n Please try again.");
			
			std::cout << "Download Failed!!" << std::endl;
		}
		else if (dlReturnCode == LOAD_ERR)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, 
												   "Plugin Installer " + pInfo.pluginName, 
												   "Unable to load " + pInfo.pluginName + " in the Processor List.\nLook at console output for more details.");

			std::cout << "Download Failed!!" << std::endl;
		}

	}
	else if (button == &documentationButton)
	{
		URL url = URL(pInfo.docURL);
		url.launchInDefaultBrowser();
	}
}

int PluginInfoComponent::versionCompare(const String& v1, const String& v2)
{ 
    //  vnum stores each numeric part of version 
    int vnum1 = 0, vnum2 = 0; 
  
    //  loop untill both string are processed 
    for (int i=0, j=0; (i<v1.length() || j<v2.length()); ) 
    { 
        //  storing numeric part of version 1 in vnum1 
        while (i < v1.length() && v1[i] != '.') 
        { 
            vnum1 = vnum1 * 10 + (v1[i] - '0'); 
            i++; 
        } 
  
        //  storing numeric part of version 2 in vnum2 
        while (j < v2.length() && v2[j] != '.') 
        { 
            vnum2 = vnum2 * 10 + (v2[j] - '0'); 
            j++; 
        } 
  
        if (vnum1 > vnum2) 
            return 1; 
        if (vnum2 > vnum1) 
            return -1; 
  
        //  if equal, reset variables and go for next numeric 
        // part 
        vnum1 = vnum2 = 0; 
        i++; 
        j++; 
    } 
    return 0; 
} 

void PluginInfoComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == &versionMenu)
	{
		pInfo.selectedVersion = comboBoxThatHasChanged->getText();

		if (pInfo.installedVersion.isEmpty())
		{
			downloadButton.setEnabled(true);
			downloadButton.setButtonText("Download");
		}
		else
		{
			String selected = pInfo.selectedVersion.substring(0, pInfo.selectedVersion.indexOf("-"));
			String installed = pInfo.installedVersion.substring(0, pInfo.installedVersion.indexOf("-"));
			int result = versionCompare(selected, installed);

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
	pluginNameText.setText(pInfo.pluginName, dontSendNotification);
	ownerText.setText(pInfo.owner, dontSendNotification);
	lastUpdatedText.setText(pInfo.lastUpdated, dontSendNotification);
	descriptionText.setText(pInfo.description, dontSendNotification);
	dependencyText.setText(pInfo.dependencies, dontSendNotification);

	versionMenu.clear(dontSendNotification);

	for (int i = 0; i < pInfo.versions.size(); i++)
		versionMenu.addItem(pInfo.versions[i], i + 1);

	//set default selected version to the first entry in combo box
	versionMenu.setSelectedId(1, sendNotification);
	pInfo.selectedVersion = pInfo.versions[0];
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

int PluginInfoComponent::downloadPlugin(const String& plugin, const String& package, const String& version) 
{

	String fileStr = "plugins" + File::separatorString + "installedPlugins.xml";
	File xmlFile = getPluginsLocationDirectory().getChildFile(fileStr);

	XmlDocument doc(xmlFile);
	std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

	std::unique_ptr<XmlElement> pluginEntry(new XmlElement(plugin));
	pluginEntry->setAttribute("version", version);

	if (xml == 0 || ! xml->hasTagName("PluginInstaller"))
	{
		std::cout << "[PluginInstaller] File not found." << std::endl;
		return 3;
	}
	else
	{	
		auto child = xml->getFirstChildElement();
		bool hasTag = false; 

		forEachXmlChildElement(*child, e)
		{
			if (e->hasTagName(pluginEntry->getTagName()))
			{
				if (e->getAttributeValue(0).equalsIgnoreCase(pluginEntry->getAttributeValue(0)))
				{
					std::cout << plugin << " v" << version << " already exists!!" << std::endl;
					return 4;
				}
				else 
				{
					e->setAttribute("version", version);
				}
				hasTag = true;
			}
		}

		if (!hasTag)
			child->addChildElement(pluginEntry.release());

		if (! xml->writeToFile(xmlFile, String::empty))
		{
			std::cout << "Error! Couldn't write to installedPlugins.xml" << std::endl;
			return 5;
		}
	}

	
	String files_url = "https://api.bintray.com/packages/open-ephys-gui-plugins/";
	files_url += plugin;
	files_url += "/";
	files_url += package;
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

		//Create input stream from the plugin's zip file URL
		ScopedPointer<InputStream> fileStream = fileUrl.createInputStream(false);

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

		ZipFile pluginZip(pluginFile);
		Result rs = pluginZip.uncompressTo(pluginsPath);

		pluginFile.deleteFile();		

		if (rs.failed())
		{
			std::cout << "Uncompressing plugin zip file failed!!" << std::endl;
			return 3;
		}

#if JUCE_WINDOWS
		auto entry = pluginZip.getEntry(0);
#else
		auto entry = pluginZip.getEntry(1);
#endif
		
		String libName = pluginsPath.getFullPathName() + File::separatorString + entry->filename;

		int loadPlugin = AccessClass::getPluginManager()->loadPlugin(libName);

		if (loadPlugin == -1)
			return 6;

		AccessClass::getProcessorList()->fillItemList();
		AccessClass::getProcessorList()->repaint();
		
		return 1;

	}
	//TODO: Prompt user to restart to see plugin in ProcessorList

	return 0;
}