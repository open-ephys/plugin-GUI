/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include <filesystem>
#include <stdio.h>

#include "../AccessClass.h"
#include "../CoreServices.h"
#include "../Processors/PluginManager/PluginManager.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "ControlPanel.h"
#include "ProcessorList.h"
#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = std::filesystem;

//-----------------------------------------------------------------------
static inline File getPluginsDirectory()
{
    File dir = CoreServices::getSavedStateDirectory();
    if (! dir.getFullPathName().contains ("plugin-GUI" + File::getSeparatorString() + "Build"))
        dir = dir.getChildFile ("plugins-api" + String (PLUGIN_API_VER));
    else
        dir = dir.getChildFile ("plugins");

    return std::move (dir);
}

static inline File getSharedDirectory()
{
    File dir = CoreServices::getSavedStateDirectory();
    if (! dir.getFullPathName().contains ("plugin-GUI" + File::getSeparatorString() + "Build"))
        dir = dir.getChildFile ("shared-api" + String (PLUGIN_API_VER));
    else
        dir = dir.getChildFile ("shared");

    return std::move (dir);
}

static String osType = String ("");
StringArray updatablePlugins;

PluginInstaller::PluginInstaller (bool loadComponents)
    : DocumentWindow (WINDOW_TITLE,
                      Colour (Colours::black),
                      DocumentWindow::closeButton)
{
    MouseCursor::showWaitCursor();
    // Identify the OS on which the GUI is running
    SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();

    if ((os & SystemStats::OperatingSystemType::Windows) != 0)
        osType = "windows";
    else if ((os & SystemStats::OperatingSystemType::MacOSX) != 0)
        osType = "mac";
    else if ((os & SystemStats::OperatingSystemType::Linux) != 0)
        osType = "linux";

    createXmlFile();

    //Initialize Plugin Installer Components

    if (loadComponents)
    {
        setSize (910, 480);

        if (auto window = getActiveTopLevelWindow())
            setCentrePosition (window->getScreenBounds().getCentre());

#ifdef JUCE_WINDOWS
        setUsingNativeTitleBar (false);
#else
        setUsingNativeTitleBar (true); // Use native title bar on Mac and Linux
#endif
        setContentOwned (new PluginInstallerComponent(), false);
        setVisible (true);
        setResizable (true, false); // useBottomCornerRisizer -- doesn't work very well
        setResizeLimits (910, 480, 8192, 5120);

#ifdef __APPLE__
        File iconDir = File::getSpecialLocation (File::currentApplicationFile).getChildFile ("Contents/Resources");
#else
        File iconDir = File::getSpecialLocation (File::currentApplicationFile).getParentDirectory();
#endif
        Image titleBarIcon = ImageCache::getFromFile (iconDir.getChildFile ("icon-small.png"));
        setIcon (titleBarIcon);
    }

    MouseCursor::hideWaitCursor();
    CoreServices::sendStatusMessage ("Plugin Installer is ready!");
}

PluginInstaller::~PluginInstaller()
{
    masterReference.clear();
}

void PluginInstaller::closeButtonPressed()
{
    setVisible (false);
    delete this;
}

void PluginInstaller::createXmlFile()
{
    File file = getPluginsDirectory().getChildFile ("installedPlugins.xml");

    XmlDocument doc (file);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    if (xml == 0 || ! xml->hasTagName ("PluginInstaller"))
    {
        std::unique_ptr<XmlElement> baseTag (new XmlElement ("PluginInstaller"));
        baseTag->setAttribute ("gui_version", JUCEApplication::getInstance()->getApplicationVersion());

        std::unique_ptr<XmlElement> plugins (new XmlElement ("InstalledPlugins"));

        baseTag->addChildElement (plugins.release());

        if (! baseTag->writeTo (file))
            LOGE ("Error! Couldn't write to installedPlugins.xml");
    }
    else
    {
        xml->setAttribute ("gui_version", JUCEApplication::getInstance()->getApplicationVersion());

        auto child = xml->getFirstChildElement();
        Array<XmlElement*> elementsToRemove;

        for (auto* e : child->getChildIterator())
        {
            File pluginPath = getPluginsDirectory().getChildFile (e->getAttributeValue (1));
            if (! pluginPath.exists())
                elementsToRemove.add (e);
        }

        for (auto element : elementsToRemove)
        {
            child->removeChildElement (element, true);
        }

        if (! xml->writeTo (file))
        {
            LOGE ("Error! Couldn't write to installedPlugins.xml");
        }

        elementsToRemove.clear();
    }
}

void PluginInstaller::installPluginAndDependency (const String& plugin, String version)
{
    PluginInfoComponent tempInfoComponent;

    /** Get list of plugins uploaded to Artifactory */
    String baseUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";
    String response = URL (baseUrl).readEntireTextStream();

    if (response.isEmpty())
        LOGE ("Unable to fetch plugins! Please check your internet connection and try again.")

    var gatewayData;
    Result result = JSON::parse (response, gatewayData);

    String url = gatewayData.getProperty ("download_url", var()).toString();
    tempInfoComponent.setDownloadURL (url);

    var pluginData = gatewayData.getProperty ("plugins", var());

    int pIndex;
    bool pluginFound = false;

    for (int i = 0; i < pluginData.size(); i++)
    {
        if (pluginData[i].getProperty ("display_name", "NULL").toString().equalsIgnoreCase (plugin))
        {
            pIndex = i;
            pluginFound = true;
            break;
        }
    }

    if (! pluginFound)
    {
        LOGE ("Automated Plugin Installation Failed! Plugin not found!")
        return;
    }

    auto platforms = pluginData[pIndex].getProperty ("platforms", "none").getArray();

    if (! platforms->contains (osType))
    {
        LOGD ("No platform specific package found for ", plugin);
        return;
    }

    LOGC (plugin, " plugin found! Installing it now...")

    SelectedPluginInfo requiredPluginInfo;

    requiredPluginInfo.pluginName = pluginData[pIndex].getProperty ("name", "NULL").toString();
    requiredPluginInfo.displayName = plugin;
    requiredPluginInfo.type = pluginData[pIndex].getProperty ("type", "NULL").toString();

    auto allVersions = pluginData[pIndex].getProperty ("versions", "NULL").getArray();

    requiredPluginInfo.versions.clear();

    for (String v : *allVersions)
    {
        String apiVer = v.substring (v.indexOf ("I") + 1);

        if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
            requiredPluginInfo.versions.add (v);
    }

    requiredPluginInfo.dependencies.clear();
    auto dependencies = pluginData[pIndex].getProperty ("dependencies", "NULL").getArray();
    for (String dependency : *dependencies)
    {
        if (! dependency.equalsIgnoreCase ("None"))
        {
            requiredPluginInfo.dependencies.add (dependency);
            for (int i = 0; i < pluginData.size(); i++)
            {
                if (pluginData[i].getProperty ("name", "NULL").toString().equalsIgnoreCase (dependency))
                {
                    // Get the latest compatible version of the dependency
                    auto allDepVersions = pluginData[i].getProperty ("versions", "NULL").getArray();
                    StringArray compatibleVersions;
                    for (String depVersion : *allDepVersions)
                    {
                        String apiVer = depVersion.substring (depVersion.indexOf ("I") + 1);

                        if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
                            compatibleVersions.add (depVersion);
                    }

                    if (! compatibleVersions.isEmpty())
                    {
                        compatibleVersions.sort (false);
                        requiredPluginInfo.dependencyVersions.add (compatibleVersions[compatibleVersions.size() - 1]);
                    }
                    else
                    {
                        LOGE ("Automated Plugin Installation Failed! Compatible plugin version not found!")
                        return;
                    }

                    break;
                }
            }
        }
    }

    tempInfoComponent.setPluginInfo (requiredPluginInfo, false);

    for (int i = 0; i < requiredPluginInfo.dependencies.size(); i++)
    {
        tempInfoComponent.downloadPlugin (requiredPluginInfo.dependencies[i],
                                          requiredPluginInfo.dependencyVersions[i],
                                          true);
    }

    // download the plugin
    if (version.isEmpty() || ! requiredPluginInfo.versions.contains (version))
    {
        if (! requiredPluginInfo.versions.isEmpty())
        {
            LOGC (plugin, " version ", version, " not found! Installing the latest version");
            requiredPluginInfo.versions.sort (false);
            version = requiredPluginInfo.versions[requiredPluginInfo.versions.size() - 1];
        }
        else
        {
            LOGE ("Automated Plugin Installation Failed! Compatible plugin version not found!")
            return;
        }
    }

    int code = tempInfoComponent.downloadPlugin (requiredPluginInfo.pluginName, version, false);

    if (code == 1)
        LOGC ("Install successful!!")
    else
        LOGC ("Install failed!!");
}

/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent() : ThreadWithProgressWindow ("Plugin Installer", false, false),
                                                       checkForUpdates (false)
{
    font = FontOptions ("Inter", "Regular", 18.0f);
    setSize (getWidth() - 10, getHeight() - 10);

    addAndMakeVisible (pluginListAndInfo);

    //Auto check for updates on startup
    checkForUpdates = true;
    this->run();

    addAndMakeVisible (sortingLabel);
    sortingLabel.setFont (font);
    sortingLabel.setText ("Sort By:", dontSendNotification);

    addAndMakeVisible (sortByMenu);
    sortByMenu.setJustificationType (Justification::centred);
    sortByMenu.addItem ("A - Z", 1);
    sortByMenu.addItem ("Z - A", 2);
    sortByMenu.setTextWhenNothingSelected ("-----");
    sortByMenu.addListener (this);

    addAndMakeVisible (viewLabel);
    viewLabel.setFont (font);
    viewLabel.setText ("View:", dontSendNotification);

    addAndMakeVisible (allButton);
    allButton.setButtonText ("All");
    allButton.setRadioGroupId (101, dontSendNotification);
    allButton.addListener (this);
    allButton.setToggleState (true, dontSendNotification);

    addAndMakeVisible (installedButton);
    installedButton.setButtonText ("Installed");
    installedButton.setClickingTogglesState (true);
    installedButton.setRadioGroupId (101, dontSendNotification);
    installedButton.addListener (this);

    addAndMakeVisible (updatesButton);
    updatesButton.setButtonText ("Fetch Updates");
    updatesButton.changeWidthToFitText();
    updatesButton.addListener (this);

    addAndMakeVisible (typeLabel);
    typeLabel.setFont (font);
    typeLabel.setText ("Type:", dontSendNotification);

    addAndMakeVisible (filterType);
    filterType.setButtonText ("Filter");
    filterType.addListener (this);
    filterType.setToggleState (true, dontSendNotification);

    addAndMakeVisible (sourceType);
    sourceType.setButtonText ("Source");
    sourceType.addListener (this);
    sourceType.setToggleState (true, dontSendNotification);

    addAndMakeVisible (sinkType);
    sinkType.setButtonText ("Sink");
    sinkType.addListener (this);
    sinkType.setToggleState (true, dontSendNotification);

    addAndMakeVisible (otherType);
    otherType.setButtonText ("Other");
    otherType.addListener (this);
    otherType.setToggleState (true, dontSendNotification);
}

void PluginInstallerComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentBackground).darker());
    g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
    g.fillRect (195, 5, 2, 38);
    g.fillRect (405, 5, 2, 38);
}

void PluginInstallerComponent::resized()
{
    sortingLabel.setBounds (20, 10, 70, 30);
    sortByMenu.setBounds (90, 10, 90, 30);

    viewLabel.setBounds (200, 10, 50, 30);
    allButton.setBounds (250, 11, 55, 28);
    installedButton.setBounds (305, 11, 105, 28);

    typeLabel.setBounds (410, 11, 50, 28);
    sourceType.setBounds (460, 11, 80, 28);
    filterType.setBounds (540, 11, 70, 28);
    sinkType.setBounds (610, 11, 65, 28);
    otherType.setBounds (675, 11, 75, 28);

    updatesButton.setBounds (getWidth() - 140, 11, 120, 28);

    pluginListAndInfo.setBounds (10, 40, getWidth() - 10, getHeight() - 40);
}

void PluginInstallerComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged->getSelectedId() == 1)
    {
        pluginListAndInfo.pluginArray.sort (true);
        pluginListAndInfo.repaint();
    }
    else if (comboBoxThatHasChanged->getSelectedId() == 2)
    {
        pluginListAndInfo.pluginArray.sort (true);
        int size = pluginListAndInfo.pluginArray.size();
        for (int i = 0; i < size / 2; i++)
        {
            pluginListAndInfo.pluginArray.getReference (i).swapWith (pluginListAndInfo.pluginArray.getReference (size - i - 1));
        }

        pluginListAndInfo.repaint();
    }
}

void PluginInstallerComponent::run()
{
    File xmlFile = getPluginsDirectory().getChildFile ("installedPlugins.xml");

    XmlDocument doc (xmlFile);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    if (xml == 0 || ! xml->hasTagName ("PluginInstaller"))
    {
        LOGE ("[PluginInstaller] File not found.");
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
            setStatusMessage ("Fetching plugin updates...");
            updatablePlugins.clear();

            String response = URL (baseUrl).readEntireTextStream();

            if (response.isEmpty())
            {
                LOGE ("Unable to fetch updates! Please check you internet connection and try again.")
                return;
            }

            Result result = JSON::parse (response, gatewayData);
            gatewayData = gatewayData.getProperty ("plugins", var());
        }

        for (auto* e : child->getChildIterator())
        {
            String pName = e->getTagName();
            installedPlugins.add (pName);

            if (checkForUpdates)
            {
                String latestVer;

                //Get latest version
                for (int i = 0; i < gatewayData.size(); i++)
                {
                    if (gatewayData[i].getProperty ("name", "NULL").toString().equalsIgnoreCase (pName))
                    {
                        // Get the latest compatible version
                        auto allVersions = gatewayData[i].getProperty ("versions", "NULL").getArray();
                        StringArray compatibleVersions;
                        for (String depVersion : *allVersions)
                        {
                            String apiVer = depVersion.substring (depVersion.indexOf ("I") + 1);

                            if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
                                compatibleVersions.add (depVersion);
                        }

                        if (! compatibleVersions.isEmpty())
                        {
                            compatibleVersions.sort (false);
                            latestVer = compatibleVersions[compatibleVersions.size() - 1];
                        }
                        else
                        {
                            latestVer = "0.0.0-API" + String (PLUGIN_API_VER);
                        }

                        break;
                    }
                }

                if (latestVer.compareNatural (e->getAttributeValue (0)) > 0)
                    updatablePlugins.add (pName);
            }
        }

        checkForUpdates = false;
    }

    /*if (updatablePlugins.size() > 0)
	{
		const String updatemsg = "Some of your plugins have updates available! "
								 "Please update them to get the latest features and bug-fixes.";

		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon,
										 "Updates Available",
										 updatemsg, "OK", this);
	}*/
}

void PluginInstallerComponent::buttonClicked (Button* button)
{
    // Filter plugins on the basis of checkbox selected
    if (allPlugins.isEmpty())
    {
        allPlugins.addArray (pluginListAndInfo.pluginArray);
    }

    if (button == &installedButton)
    {
        this->run();

        pluginListAndInfo.pluginArray.clear();
        pluginListAndInfo.pluginArray.addArray (installedPlugins);
        pluginListAndInfo.setNumRows (installedPlugins.size());
    }
    else if (button == &allButton)
    {
        pluginListAndInfo.pluginArray.clear();
        pluginListAndInfo.pluginArray.addArray (allPlugins);
        pluginListAndInfo.setNumRows (allPlugins.size());
    }
    else if (button == &updatesButton)
    {
        checkForUpdates = true;
        this->runThread();
        pluginListAndInfo.resized();
    }

    if (button == &sourceType || button == &filterType || button == &sinkType || button == &otherType)
    {
        bool sourceState = sourceType.getToggleState();
        bool filterState = filterType.getToggleState();
        bool sinkState = sinkType.getToggleState();
        bool otherState = otherType.getToggleState();

        if (sourceState || filterState || sinkState || otherState)
        {
            StringArray tempArray;

            pluginListAndInfo.pluginArray.clear();

            if (installedButton.getToggleState())
                tempArray.addArray (installedPlugins);
            else
                tempArray.addArray (allPlugins);

            for (int i = 0; i < tempArray.size(); i++)
            {
                String label;

                label = pluginListAndInfo.pluginLabels[tempArray[i]];

                int containsType = 0;

                bool isSource = label.containsWholeWordIgnoreCase ("source");
                bool isFilter = label.containsWholeWordIgnoreCase ("filter");
                bool isSink = label.containsWholeWordIgnoreCase ("sink");
                bool isOther = isSource ? false : (isFilter ? false : (isSink ? false : true));

                if (sourceState && isSource)
                    containsType++;

                if (filterState && isFilter)
                    containsType++;

                if (sinkState && isSink)
                    containsType++;

                if (otherState && isOther)
                    containsType++;

                if (containsType > 0)
                {
                    pluginListAndInfo.pluginArray.add (tempArray[i]);
                }
                pluginListAndInfo.setNumRows (pluginListAndInfo.pluginArray.size());
            }
        }
        else
        {
            pluginListAndInfo.pluginArray.clear();
            pluginListAndInfo.setNumRows (0);
        }
    }

    sortByMenu.setSelectedId (-1, dontSendNotification);
}

/* ================================== Plugin Table Component ================================== */

PluginListBoxComponent::PluginListBoxComponent() : Thread ("Plugin List"), maxTextWidth (0)
{
    listFont = FontOptions ("Inter", "Semi Bold", 22.0f);

    // Set progress window text and background colours
    //auto window = this->getAlertWindow();
    //window->setColour(AlertWindow::textColourId, Colours::white);
    //window->setColour(AlertWindow::backgroundColourId, Colour::fromRGB(50, 50, 50));
    //setStatusMessage("Fetching plugins ...");

    this->run(); //Load all plugin names and labels from bintray

    addAndMakeVisible (pluginList);
    pluginList.setModel (this);
    pluginList.setRowHeight (35);
    pluginList.setMouseMoveSelectsRows (true);
    pluginList.getViewport()->setScrollBarThickness (10.0f);

    listBoxDropShadower.setOwner (&pluginList);

    addAndMakeVisible (pluginInfoPanel);
}

int PluginListBoxComponent::getNumRows()
{
    return numRows;
}

void PluginListBoxComponent::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
    {
        g.fillAll (findColour (ThemeColours::defaultFill).withAlpha (0.5f));
        g.setColour (findColour (ThemeColours::defaultText));
    }
    else
    {
        g.fillAll (findColour (ThemeColours::componentBackground));
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.8f));
    }

    if (rowNumber == pluginArray.indexOf (lastPluginSelected, true, 0))
    {
        g.fillAll (findColour (ThemeColours::menuHighlightBackground));
        g.setColour (findColour (ThemeColours::menuHighlightText));
    }

    g.setFont (listFont);

    String text = displayNames[pluginArray[rowNumber]];

    g.drawText (text, 20, 0, maxTextWidth + 5, height, Justification::centredLeft, true);

    // Draw update indicator next to plugin name, if any
    if (updatablePlugins.contains (pluginArray[rowNumber]))
    {
        g.setColour (Colours::green);
        g.fillEllipse (maxTextWidth + 25.0f, 6.0f, 23.0f, height - 12.0f);
        g.setColour (Colours::white);
        g.drawArrow (Line (maxTextWidth + 37.0f, height - 11.0f, maxTextWidth + 37.0f, 11.0f), 3.0f, 9.0f, 9.0f);
    }
}

void PluginListBoxComponent::run()
{
    /* Get list of plugins uploaded to bintray */
    String baseUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";
    String response = URL (baseUrl).readEntireTextStream();

    if (response.isEmpty())
    {
        String errorMsg = "Unable to fetch plugins! Please check your internet connection and try again.";
        LOGE (errorMsg);
        MessageManager::callAsync ([this, errorMsg]
                                   { pluginInfoPanel.updateStatusMessage (errorMsg, true); });

        return;
    }

    var gatewayData;
    Result result = JSON::parse (response, gatewayData);

    String url = gatewayData.getProperty ("download_url", var()).toString();
    pluginInfoPanel.setDownloadURL (url);

    pluginData = gatewayData.getProperty ("plugins", var());

    numRows = pluginData.size();

    String pluginName, label, dispName;

    int pluginTextWidth;
    StringArray compatibleVersions;

    // Get each plugin's labels and add them to the list
    for (int i = 0; i < numRows; i++)
    {
        auto allVersions = pluginData[i].getProperty ("versions", "NULL").getArray();
        compatibleVersions.clear();

        for (String version : *allVersions)
        {
            String apiVer = version.substring (version.indexOf ("I") + 1);

            if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
            {
                compatibleVersions.add (version);
            }
        }

        auto platforms = pluginData[i].getProperty ("platforms", "none").getArray();

        if (! compatibleVersions.isEmpty() && platforms->contains (osType))
        {
            pluginName = pluginData[i].getProperty ("name", var()).toString();
            label = pluginData[i].getProperty ("type", "NULL").toString();
            dispName = pluginData[i].getProperty ("display_name", "NULL").toString();

            pluginTextWidth = GlyphArrangement::getStringWidthInt (Font (listFont), dispName);
            if (pluginTextWidth > maxTextWidth)
                maxTextWidth = pluginTextWidth;

            if (! label.equalsIgnoreCase ("CommonLib"))
            {
                pluginArray.add (pluginName);
                displayNames.set (pluginName, dispName);
                pluginLabels.set (pluginName, label);
            }
            else
            {
                compatibleVersions.sort (false);
                dependencyVersion.set (pluginName, compatibleVersions[compatibleVersions.size() - 1]);
            }
        }

        //setProgress ((i + 1) / (double) numRows);
    }

    MessageManager::callAsync ([this]
                               { setNumRows (pluginArray.size()); });
}

bool PluginListBoxComponent::loadPluginInfo (const String& pluginName)
{
    int pIndex;
    for (int i = 0; i < pluginData.size(); i++)
    {
        if (pluginData[i].getProperty ("name", "NULL").toString().equalsIgnoreCase (pluginName))
        {
            pIndex = i;
            break;
        }
    }

    auto platforms = pluginData[pIndex].getProperty ("platforms", "none").getArray();

    if (! platforms->contains (osType))
    {
        LOGE ("No platform specific package found for ", pluginName);
        pluginInfoPanel.makeInfoVisible (false);
        return false;
    }

    selectedPluginInfo.pluginName = pluginName;
    selectedPluginInfo.displayName = displayNames[pluginName];
    selectedPluginInfo.type = pluginLabels[pluginName];
    selectedPluginInfo.developers = pluginData[pIndex].getProperty ("developers", "NULL");
    String updated = pluginData[pIndex].getProperty ("updated", "NULL");
    selectedPluginInfo.lastUpdated = updated.substring (0, updated.indexOf ("T"));
    selectedPluginInfo.description = pluginData[pIndex].getProperty ("desc", "NULL");
    selectedPluginInfo.docURL = pluginData[pIndex].getProperty ("docs", "NULL").toString();
    selectedPluginInfo.selectedVersion = String();

    auto allVersions = pluginData[pIndex].getProperty ("versions", "NULL").getArray();

    selectedPluginInfo.versions.clear();

    for (String version : *allVersions)
    {
        String apiVer = version.substring (version.indexOf ("I") + 1);

        if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
            selectedPluginInfo.versions.add (version);
    }

    // Set the latest version from the list of compatible versions
    auto sortedVersions = selectedPluginInfo.versions;
    sortedVersions.sort (false);
    selectedPluginInfo.latestVersion = sortedVersions[sortedVersions.size() - 1];

    selectedPluginInfo.dependencies.clear();
    auto dependencies = pluginData[pIndex].getProperty ("dependencies", "NULL").getArray();
    for (String dependency : *dependencies)
    {
        if (! dependency.equalsIgnoreCase ("None"))
        {
            selectedPluginInfo.dependencies.add (dependency);
            selectedPluginInfo.dependencyVersions.add (dependencyVersion[dependency]);
        }
    }

    // If the plugin is already installed, get installed version number
    File xmlFile = getPluginsDirectory().getChildFile ("installedPlugins.xml");
    ;

    XmlDocument doc (xmlFile);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    if (xml == 0 || ! xml->hasTagName ("PluginInstaller"))
    {
        LOGE ("File not found.");
        return false;
    }
    else
    {
        auto child = xml->getFirstChildElement();

        auto pluginEntry = child->getChildByName (pluginName);

        if (pluginEntry != nullptr)
            selectedPluginInfo.installedVersion = pluginEntry->getAttributeValue (0);
        else
            selectedPluginInfo.installedVersion = String();
    }

    pluginInfoPanel.setPluginInfo (selectedPluginInfo);
    pluginInfoPanel.makeInfoVisible (true);

    return true;
}

void PluginListBoxComponent::listBoxItemClicked (int row, const MouseEvent&)
{
    this->returnKeyPressed (row);
}

void PluginListBoxComponent::resized()
{
    // position our table with a gap around its edge
    if (updatablePlugins.isEmpty())
    {
        pluginList.setBounds (10, 10, maxTextWidth + 60, getHeight() - 30);
        pluginInfoPanel.setBounds (maxTextWidth + 80, 10, getWidth() - maxTextWidth - 100, getHeight() - 30);
    }
    else
    {
        pluginList.setBounds (10, 10, maxTextWidth + 70, getHeight() - 30);
        pluginInfoPanel.setBounds (maxTextWidth + 90, 10, getWidth() - maxTextWidth - 110, getHeight() - 30);
    }
}

void PluginListBoxComponent::returnKeyPressed (int lastRowSelected)
{
    if (! lastPluginSelected.equalsIgnoreCase (pluginArray[lastRowSelected]))
    {
        lastPluginSelected = pluginArray[lastRowSelected];

        pluginInfoPanel.makeInfoVisible (false);
        pluginInfoPanel.updateStatusMessage ("Loading Plugin Info...", true);

        if (loadPluginInfo (lastPluginSelected))
            pluginInfoPanel.updateStatusMessage ("", false);
        else
            pluginInfoPanel.updateStatusMessage ("No platform specific package found for " + lastPluginSelected, true);

        this->repaint();
    }
}

/* ================================== Plugin Information Component ================================== */

PluginInfoComponent::PluginInfoComponent() : ThreadWithProgressWindow ("Plugin Installer", true, false)
{
    infoCompDropShadower.setOwner (this);

    infoFont = FontOptions ("Inter", "Regular", 20.0f);
    infoFontBold = FontOptions ("Inter", "Semi Bold", 20.0f);

    addChildComponent (pluginNameLabel);
    pluginNameLabel.setFont (infoFontBold);
    pluginNameLabel.setText ("Name: ", dontSendNotification);

    addChildComponent (pluginNameText);
    pluginNameText.setFont (infoFont);

    addChildComponent (developersLabel);
    developersLabel.setFont (infoFontBold);
    developersLabel.setText ("Developers: ", dontSendNotification);

    addChildComponent (developersText);
    developersText.setFont (infoFont);
    developersText.setMinimumHorizontalScale (1.0f);

    addChildComponent (versionLabel);
    versionLabel.setFont (infoFontBold);
    versionLabel.setText ("Version: ", dontSendNotification);

    addChildComponent (versionMenu);
    versionMenu.setJustificationType (Justification::centred);
    versionMenu.setTextWhenNoChoicesAvailable ("- N/A -");
    versionMenu.addListener (this);

    addChildComponent (installedVerLabel);
    installedVerLabel.setFont (infoFontBold);
    installedVerLabel.setText ("Installed: ", dontSendNotification);

    addChildComponent (installedVerText);
    installedVerText.setFont (infoFont);
    installedVerText.setMinimumHorizontalScale (1.0f);

    addChildComponent (lastUpdatedLabel);
    lastUpdatedLabel.setFont (infoFontBold);
    lastUpdatedLabel.setText ("Last Updated: ", dontSendNotification);

    addChildComponent (lastUpdatedText);
    lastUpdatedText.setFont (infoFont);

    addChildComponent (descriptionLabel);
    descriptionLabel.setFont (infoFontBold);
    descriptionLabel.setText ("Description: ", dontSendNotification);

    addChildComponent (descriptionText);
    descriptionText.setFont (infoFont);
    descriptionText.setJustificationType (Justification::topLeft);
    descriptionText.setMinimumHorizontalScale (1.0f);

    addChildComponent (dependencyLabel);
    dependencyLabel.setFont (infoFontBold);
    dependencyLabel.setText ("Dependencies: ", dontSendNotification);

    addChildComponent (dependencyText);
    dependencyText.setFont (infoFont);

    addChildComponent (downloadButton);
    downloadButton.setButtonText ("Install");
    downloadButton.addListener (this);

    addChildComponent (uninstallButton);
    uninstallButton.setButtonText ("Uninstall");
    uninstallButton.addListener (this);

    addChildComponent (documentationButton);
    documentationButton.setButtonText ("Documentation");
    documentationButton.addListener (this);

    addAndMakeVisible (statusLabel);
    statusLabel.setFont (infoFont);
    statusLabel.setText ("Please select a plugin from the list on the left...", dontSendNotification);
}

void PluginInfoComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentBackground));
}

void PluginInfoComponent::resized()
{
    pluginNameLabel.setBounds (10, 30, 140, 30);
    pluginNameText.setBounds (145, 30, getWidth() - 150, 30);

    developersLabel.setBounds (10, 60, 140, 30);
    developersText.setBounds (145, 60, getWidth() - 150, 30);

    versionLabel.setBounds (10, 90, 140, 30);
    versionMenu.setBounds (150, 90, 110, 26);

    installedVerLabel.setBounds (10, versionLabel.getBottom(), 140, 30);
    installedVerText.setBounds (145, versionLabel.getBottom(), 110, 30);

    lastUpdatedLabel.setBounds (10, installedVerLabel.getBottom(), 140, 30);
    lastUpdatedText.setBounds (145, installedVerLabel.getBottom(), getWidth() - 150, 30);

    descriptionLabel.setBounds (10, lastUpdatedLabel.getBottom(), 140, 30);
    descriptionText.setBounds (145, lastUpdatedLabel.getBottom() + 5, getWidth() - 150, 75);

    dependencyLabel.setBounds (10, descriptionText.getBottom() + 5, 140, 30);
    dependencyText.setBounds (145, dependencyLabel.getY(), getWidth() - 150, 30);

    downloadButton.setBounds (getWidth() - (getWidth() * 0.25) - 20, getHeight() - 60, getWidth() * 0.25, 30);
    uninstallButton.setBounds (getWidth() - (2 * (getWidth() * 0.25)) - 30, getHeight() - 60, getWidth() * 0.25, 30);
    documentationButton.setBounds (20, getHeight() - 60, getWidth() * 0.25, 30);

    statusLabel.setBounds (10, (getHeight() / 2) - 15, getWidth() - 10, 30);
}

void PluginInfoComponent::buttonClicked (Button* button)
{
    if (button == &downloadButton)
    {
        if (auto* alertWindow = this->getAlertWindow())
        {
            if (auto parent = button->getTopLevelComponent())
                alertWindow->setCentrePosition (parent->getScreenBounds().getCentre());
        }

        this->runThread();
    }
    else if (button == &uninstallButton)
    {
        if (! uninstallPlugin (pInfo.pluginName))
        {
            LOGE ("Failed to uninstall ", pInfo.displayName);
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "[Plugin Installer] " + pInfo.displayName,
                                              "Failed to uninstall " + pInfo.displayName);
        }
        else
        {
            LOGC (pInfo.displayName, " uninstalled successfully!");
            AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                              "[Plugin Installer] " + pInfo.displayName,
                                              pInfo.displayName + " uninstalled successfully");
        }
    }
    else if (button == &documentationButton)
    {
        URL url = URL (pInfo.docURL);
        url.launchInDefaultBrowser();
    }
}

void PluginInfoComponent::setDownloadURL (const String& url)
{
    downloadURL = url;
}

void PluginInfoComponent::showAlertOnMessageThread (MessageBoxIconType iconType, const String& title, const String& message)
{
    MessageManager::callAsync ([=]()
                               { AlertWindow::showMessageBoxAsync (iconType, title, message); });
}

void PluginInfoComponent::updateUIOnMessageThread()
{
    MessageManager::callAsync ([this]()
    {
        pInfo.installedVersion = pInfo.selectedVersion;
        installedVerText.setText (pInfo.installedVersion, dontSendNotification);
        downloadButton.setEnabled (false);
        downloadButton.setButtonText ("Installed");
        uninstallButton.setVisible (true);

        if (pInfo.installedVersion.equalsIgnoreCase (pInfo.latestVersion))
        {
            updatablePlugins.removeString (pInfo.pluginName);
            this->getParentComponent()->resized();
        } 
    });
}

void PluginInfoComponent::run()
{
    setProgress (-1.0);

    // Check if plugin already present in signal chain
    bool pluginInUse = false;
    String nameInUse;
    if (pInfo.type == "RecordEngine" && AccessClass::getProcessorGraph()->hasRecordNode())
    {
        pluginInUse = true;
        nameInUse = "Record Node";
    }
    else
    {
        auto processors = AccessClass::getProcessorGraph()->getListOfProcessors();
        for (auto* p : processors)
        {
            if (p->getLibName().equalsIgnoreCase (pInfo.displayName))
            {
                pluginInUse = true;
                nameInUse = pInfo.displayName;
                break;
            }
        }
    }

    if (pluginInUse)
    {
        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  nameInUse + " is already in use. Please remove all instances of it from the signal chain and try again.");

        LOGE ("Error.. Plugin already in use. Please remove it from the signal chain and try again.");
        return;
    }

    // Remove older version of the plugin if present
    if (! AccessClass::getPluginManager()->removePlugin (pInfo.displayName))
    {
        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] ERROR",
                                  "Unable to remove current installed version of " + pInfo.displayName + "... Plugin update failed.");

        LOGE ("Unable to remove current installed version of " + pInfo.displayName + "... Plugin update failed.");
        return;
    }

    // If a plugin has dependencies outside its zip, download them
    for (int i = 0; i < pInfo.dependencies.size(); i++)
    {
        setStatusMessage ("Downloading dependency: " + pInfo.dependencies[i]);
        LOGD ("Downloading dependency: ", pInfo.dependencies[i], "...  ");

        int retCode = downloadPlugin (pInfo.dependencies[i], pInfo.dependencyVersions[i], true);

        if (retCode == 1)
        {
            continue;
        }
        else if (retCode == 2)
        {
            showAlertOnMessageThread (AlertWindow::WarningIcon,
                                      "[Plugin Installer] " + pInfo.dependencies[i],
                                      "Could not install dependency: " + pInfo.dependencies[i]
                                          + ". Please contact the developers.");

            LOGE ("Download Failed!!");
            return;
        }
        else if (retCode == ZIP_NOTFOUND)
        {
            showAlertOnMessageThread (AlertWindow::WarningIcon,
                                      "[Plugin Installer] " + pInfo.dependencies[i],
                                      "Could not find the ZIP file for " + pInfo.dependencies[i]
                                          + ". Please contact the developers.");

            LOGE ("Download Failed!!");
            return;
        }
        else if (retCode == HTTP_ERR)
        {
            String httpErr = "Please check your internet connection...";

            if (httpStatusCode != 0)
                httpErr = "Status Code: " + String (httpStatusCode);

            showAlertOnMessageThread (AlertWindow::WarningIcon,
                                      "[Plugin Installer] " + pInfo.dependencies[i],
                                      "HTTP request failed!!\n" + httpErr);

            LOGE ("HTTP request failed. ", httpErr);
            return;
        }
        else
        {
            showAlertOnMessageThread (AlertWindow::WarningIcon,
                                      "[Plugin Installer] " + pInfo.dependencies[i],
                                      "An unknown error occurred while installing dependencies for " + pInfo.displayName
                                          + ". Please contact the developers.");

            LOGE ("Download Failed!!");
            return;
        }

        httpStatusCode = 0;
    }

    setStatusMessage ("Downloading " + pInfo.displayName + " ...");
    LOGC ("Downloading Plugin: ", pInfo.displayName, " | Version: ", pInfo.selectedVersion);

    // download the plugin
    int dlReturnCode = downloadPlugin (pInfo.pluginName, pInfo.selectedVersion, false);

    if (dlReturnCode == SUCCESS)
    {
        LOGC ("Download Successful!");

        String pluginVer = pInfo.selectedVersion.substring (0, pInfo.selectedVersion.indexOf ("-API"));

        showAlertOnMessageThread (AlertWindow::InfoIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  pInfo.displayName + " v" + pluginVer + " Installed Successfully!");

        updateUIOnMessageThread();
    }
    else if (dlReturnCode == ZIP_NOTFOUND)
    {
        String errMsg = "Download Failed! ZIP file not found.";

        if (httpStatusCode != 0)
            errMsg += " HTTP Status Code: " + String (httpStatusCode);

        LOGE (errMsg);

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "Could not find the ZIP file for " + pInfo.displayName
                                      + ". Please contact the developers.");
    }
    else if (dlReturnCode == UNCMP_ERR)
    {
        LOGE ("Download Failed! Uncompressing ZIP failed.");

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "Could not uncompress the ZIP file. Please try again.");
    }
    else if (dlReturnCode == XML_MISSING)
    {
        LOGE ("XML File Missing! Please relaunch Plugin Installer.");

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "Unable to locate installedPlugins.xml \n Please relaunch Plugin Installer and try again.");
    }
    else if (dlReturnCode == VER_EXISTS_ERR)
    {
        LOGE ("Download Failed! Version already exists.");

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  pInfo.displayName + " v" + pInfo.selectedVersion
                                      + " already exists. Please download another version.");
    }
    else if (dlReturnCode == XML_WRITE_ERR)
    {
        LOGE ("Writing to XML Failed! Please try again.");

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "Unable to write to installedPlugins.xml \n Please try again.");
    }
    else if (dlReturnCode == LOAD_ERR)
    {
        LOGE ("Loading Plugin Failed!");

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "Unable to load " + pInfo.displayName
                                      + " in the Processor List.\nLook at console output for more details.");

        updateUIOnMessageThread();
    }
    else if (dlReturnCode == HTTP_ERR)
    {
        String httpErr = "Please check your internet connection...";

        if (httpStatusCode != 0)
            httpErr = "Status Code: " + String (httpStatusCode);

        LOGE ("HTTP request failed. ", httpErr);

        showAlertOnMessageThread (AlertWindow::WarningIcon,
                                  "[Plugin Installer] " + pInfo.displayName,
                                  "HTTP request failed!!\n" + httpErr);
    }

    httpStatusCode = 0;
}

void PluginInfoComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &versionMenu)
    {
        pInfo.selectedVersion = comboBoxThatHasChanged->getText();

        // Change install button name depending on the selected version of a plugin
        if (pInfo.installedVersion.isEmpty())
        {
            downloadButton.setEnabled (true);
            downloadButton.setButtonText ("Install");
        }
        else
        {
            int result = pInfo.selectedVersion.compareNatural (pInfo.installedVersion);

            if (result == 0)
            {
                downloadButton.setEnabled (false);
                downloadButton.setButtonText ("Installed");
            }
            else if (result > 0)
            {
                downloadButton.setEnabled (true);
                downloadButton.setButtonText ("Upgrade");
            }
            else
            {
                downloadButton.setEnabled (true);
                downloadButton.setButtonText ("Downgrade");
            }
        }
    }
}

void PluginInfoComponent::setPluginInfo (const SelectedPluginInfo& p, bool shouldUpdateUI)
{
    pInfo = p;

    if (shouldUpdateUI)
    {
        pluginNameText.setText (pInfo.displayName, dontSendNotification);
        developersText.setText (pInfo.developers, dontSendNotification);
        lastUpdatedText.setText (pInfo.lastUpdated, dontSendNotification);
        descriptionText.setText (pInfo.description, dontSendNotification);
        if (pInfo.dependencies.isEmpty())
            dependencyText.setText ("None", dontSendNotification);
        else
            dependencyText.setText (pInfo.dependencies.joinIntoString (", "), dontSendNotification);

        versionMenu.clear (dontSendNotification);

        if (pInfo.installedVersion.isEmpty())
            installedVerText.setText ("No", dontSendNotification);
        else
            installedVerText.setText (pInfo.installedVersion, dontSendNotification);

        if (pInfo.versions.isEmpty())
        {
            downloadButton.setEnabled (false);
            downloadButton.setButtonText ("Unavailable");
        }
        else
        {
            for (int i = 0; i < pInfo.versions.size(); i++)
                versionMenu.addItem (pInfo.versions[i], i + 1);

            //set default selected version to the first entry in combo box
            versionMenu.setSelectedId (1, sendNotification);
            pInfo.selectedVersion = pInfo.versions[0];
        }
    }
}

void PluginInfoComponent::updateStatusMessage (const String& str, bool isVisible)
{
    statusLabel.setText (str, dontSendNotification);
    statusLabel.setVisible (isVisible);
}

void PluginInfoComponent::makeInfoVisible (bool isEnabled)
{
    pluginNameLabel.setVisible (isEnabled);
    pluginNameText.setVisible (isEnabled);

    developersLabel.setVisible (isEnabled);
    developersText.setVisible (isEnabled);

    versionLabel.setVisible (isEnabled);
    versionMenu.setVisible (isEnabled);

    installedVerLabel.setVisible (isEnabled);
    installedVerText.setVisible (isEnabled);

    lastUpdatedLabel.setVisible (isEnabled);
    lastUpdatedText.setVisible (isEnabled);

    descriptionLabel.setVisible (isEnabled);
    descriptionText.setVisible (isEnabled);

    dependencyLabel.setVisible (isEnabled);
    dependencyText.setVisible (isEnabled);

    downloadButton.setVisible (isEnabled);
    documentationButton.setVisible (isEnabled);

    if (pInfo.installedVersion.isNotEmpty())
        uninstallButton.setVisible (isEnabled);
}

bool PluginInfoComponent::uninstallPlugin (const String& plugin)
{
    LOGC ("Uninstalling plugin: ", pInfo.displayName);

    // Check whether the plugin is loaded in a signal chain
    auto processors = AccessClass::getProcessorGraph()->getListOfProcessors();
    for (auto* p : processors)
    {
        if (p->getLibName().equalsIgnoreCase (pInfo.displayName))
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "[Plugin Installer] " + pInfo.displayName,
                                              pInfo.displayName + " is already in use. Please remove all instances of it from the signal chain and try again.");

            LOGD ("Plugin present in signal chain! Please remove it before uninstalling the plugin.");
            return false;
        }
    }

    // Open installedPluings.xml file
    File xmlFile = getPluginsDirectory().getChildFile ("installedPlugins.xml");

    XmlDocument doc (xmlFile);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    String dllName;
    XmlElement* pluginElement;

    if (xml == 0 || ! xml->hasTagName ("PluginInstaller"))
    {
        LOGD ("[PluginInstaller] InstalledPlugins.xml file not found.");
        return false;
    }
    else
    {
        // Fetch plugin DLL name
        pluginElement = xml->getFirstChildElement()->getChildByName (plugin);
        dllName = pluginElement->getAttributeValue (1);
    }

    // Remove and unload plugin via PluginManager
    if (! AccessClass::getPluginManager()->removePlugin (pInfo.displayName))
        return false;

    // Remove plugin XML entry
    xml->getFirstChildElement()->removeChildElement (pluginElement, true);
    if (! xml->writeTo (xmlFile))
    {
        LOGD ("Error! Couldn't write to installedPlugins.xml");
    }

    AccessClass::getProcessorList()->fillItemList();
    AccessClass::getProcessorList()->repaint();

    if (pInfo.type == "RecordEngine")
        AccessClass::getControlPanel()->updateRecordEngineList();

    uninstallButton.setVisible (false);
    downloadButton.setEnabled (true);
    downloadButton.setButtonText ("Install");
    installedVerText.setText ("No", dontSendNotification);

    //delete plugin file
    File pluginFile = getPluginsDirectory().getChildFile (dllName);
    if (! pluginFile.deleteRecursively())
    {
        LOGE ("Unable to delete plugin file ", pluginFile.getFullPathName(), " ... Please remove it manually!!");
        return false;
    }

    return true;
}

int PluginInfoComponent::downloadPlugin (const String& plugin, const String& version, bool isDependency)
{
    String fileDownloadURL = downloadURL;
    fileDownloadURL = fileDownloadURL.replace ("<plugin-name>", plugin);
    fileDownloadURL = fileDownloadURL.replace ("<platform>", osType);
    fileDownloadURL = fileDownloadURL.replace ("<version>", version);

    String filename = plugin + "-" + osType + "_" + version + ".zip";

    URL fileUrl (fileDownloadURL);

    //Create input stream from the plugin's zip file URL
    std::unique_ptr<InputStream> fileStream = fileUrl.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                                             .withConnectionTimeoutMs (0)
                                                                             .withNumRedirectsToFollow (5));

    // Could not retrieve data
    if (! fileStream)
        return HTTP_ERR;

    if (auto webStream = dynamic_cast<WebInputStream*> (fileStream.get()))
    {
        httpStatusCode = webStream->getStatusCode();
        if (httpStatusCode >= 400)
        {
            if (httpStatusCode == 404 || fileStream->getTotalLength() < 1)
                return ZIP_NOTFOUND;
            else
                return HTTP_ERR;
        }
    }

    //Construct path for downloaded zip file
    String pluginFilePath = CoreServices::getSavedStateDirectory().getFullPathName();
    pluginFilePath += File::getSeparatorString();
    pluginFilePath += filename;

    //Create local file
    File pluginFile (pluginFilePath);
    pluginFile.deleteFile();

    //Use the Url's input stream and write it to a file using output stream
    std::unique_ptr<FileOutputStream> out = pluginFile.createOutputStream();
    int64 total = 0;
    int64 fileSize = fileStream->getTotalLength();

    for (;;)
    {
        if (threadShouldExit())
            return HTTP_ERR;

        auto written = out->writeFromInputStream (*fileStream, 8192);

        if (written == 0)
            break;

        total += written;

        setProgress ((double) total / (double) fileSize);
    }

    out->flush();
    out.reset();

    //Uncompress zip file contents
    ZipFile pluginZip (pluginFile);

    if (! pluginFile.exists())
        return ZIP_NOTFOUND;

    //Get *.dll/*.so name of plugin
#if JUCE_WINDOWS
    auto entry = pluginZip.getEntry (0);
#else
    auto entry = pluginZip.getEntry (1);
#endif

    String dllName = entry->filename;
    dllName = dllName.substring (dllName.indexOf (File::getSeparatorString()) + 1);

    // Open installedPluings.xml file
    File xmlFile = getPluginsDirectory().getChildFile ("installedPlugins.xml");

    XmlDocument doc (xmlFile);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    if (! isDependency)
    {
        // Create a new entry in xml for the downloaded plugin
        std::unique_ptr<XmlElement> pluginEntry (new XmlElement (plugin));

        // set version and dllName attributes of the plugins
        pluginEntry->setAttribute ("version", version);
        pluginEntry->setAttribute ("dllName", dllName);

        if (xml == 0 || ! xml->hasTagName ("PluginInstaller"))
        {
            LOGE ("[PluginInstaller] File not found.");
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
                if (e->hasTagName (pluginEntry->getTagName()))
                {
                    if (e->getAttributeValue (0).equalsIgnoreCase (pluginEntry->getAttributeValue (0)))
                    {
                        LOGE (plugin, " v", version, " already exists!!");
                        pluginFile.deleteFile();
                        return 4;
                    }
                    else
                    {
                        e->setAttribute ("version", version);
                    }
                    hasTag = true;
                }
            }

            // if no such plugin is installed, add its info to the xml file
            if (! hasTag)
                child->addChildElement (pluginEntry.release());
        }
    }

    // Create temp directory to uncompress the plugin
    File tempDir = File::getSpecialLocation (File::tempDirectory).getChildFile ("open-ephys");
    tempDir.createDirectory();

    // Delete any existing files in temp directory
    if (tempDir.getChildFile ("plugins").exists())
        tempDir.getChildFile ("plugins").deleteRecursively();

    if (tempDir.getChildFile ("shared").exists())
        tempDir.getChildFile ("shared").deleteRecursively();

    setProgress (-1.0);
    setStatusMessage ("Uncompressing ZIP file...");

    // Uncompress the plugin zip file to temp directory
    juce::Result res = pluginZip.uncompressTo (tempDir);

    if (res.failed())
    {
        LOGE ("Failed to uncompress plugin zip file: ", res.getErrorMessage());
        tempDir.deleteRecursively();
        pluginFile.deleteFile();
        return 2;
    }

    String pluginDllPath;

    // copy plugin DLL from temp directory to actual location
    if (! isDependency)
    {
        fs::path tempPluginPath = tempDir.getChildFile ("plugins").getFullPathName().toStdString();
        fs::path destPluginPath = getPluginsDirectory().getFullPathName().toStdString();

        // Copy only if plugin file exists in temp directory
        if (fs::exists (tempPluginPath))
        {
            // Delete existing plugin DLL file if it exists
            File dllFile = getPluginsDirectory().getChildFile (dllName);
            if (dllFile.exists())
            {
                dllFile.deleteRecursively();
            }

            const auto copyOptions = fs::copy_options::overwrite_existing
                                     | fs::copy_options::recursive;
            try
            {
                fs::copy (tempPluginPath, destPluginPath, copyOptions);
            }
            catch (fs::filesystem_error& e)
            {
                LOGE ("Could not copy plugin files: \"", e.what(), "\"");
                tempDir.deleteRecursively();
                pluginFile.deleteFile();
                return 2;
            }
        }
        else
        {
            LOGE ("Plugin file not found in temp directory!!");
            tempDir.deleteRecursively();
            pluginFile.deleteFile();
            return 2;
        }

        pluginDllPath = getPluginsDirectory().getChildFile (dllName).getFullPathName();
    }

    /* Copy shared files 
	*  Uses C++17's filesystem::copy functionality to allow copying symlinks
	*/
    fs::path tempSharedPath = tempDir.getChildFile ("shared").getFullPathName().toStdString();
    fs::path destSharedPath = getSharedDirectory().getFullPathName().toStdString();

    // Copy only if shared files exist
    if (fs::exists (tempSharedPath))
    {
#if JUCE_WINDOWS || JUCE_MAC
        const auto copyOptions = fs::copy_options::overwrite_existing
                                 | fs::copy_options::recursive
                                 | fs::copy_options::copy_symlinks;
#else
        const auto copyOptions = fs::copy_options::update_existing
                                 | fs::copy_options::recursive
                                 | fs::copy_options::copy_symlinks;
#endif
        try
        {
            fs::copy (tempSharedPath, destSharedPath, copyOptions);
        }
        catch (fs::filesystem_error& e)
        {
            LOGD ("Could not copy shared files: \"", e.what(), "\"");
        }
    }

    tempDir.deleteRecursively();
    pluginFile.deleteFile(); // delete zip after uncompressing

    // if the plugin is not a dependency, load the plugin and show it in processor list
    if (! isDependency)
    {
        // Write installed plugin's info to XML file
        if (! xml->writeTo (xmlFile))
        {
            LOGE ("Error! Couldn't write to installedPlugins.xml");
            return 5;
        }

#if JUCE_LINUX
        // Add shared library directory to LD_LIBRARY_PATH before loading plugin
        setStatusMessage ("Setting up library path...");

        File sharedDir = getSharedDirectory();
        String currentPath = SystemStats::getEnvironmentVariable ("LD_LIBRARY_PATH", "");
        String newPath = sharedDir.getFullPathName();

        if (! currentPath.contains (newPath))
        {
            if (! currentPath.isEmpty())
                newPath += ":" + currentPath;

            setenv ("LD_LIBRARY_PATH", newPath.toRawUTF8(), 1);
        }
#endif

        LOGD ("Loading plugin: ", pInfo.displayName, "from ", pluginDllPath);

        int retCode = -1;

        MessageManager::callSync ([&]()
                                  {
                                    retCode = AccessClass::getPluginManager()->loadPlugin (pluginDllPath);

                                    AccessClass::getProcessorList()->fillItemList();
                                    AccessClass::getProcessorList()->repaint();

                                    if (pInfo.type == "RecordEngine")
                                        AccessClass::getControlPanel()->updateRecordEngineList(); });

        if (retCode == -1)
            return 6;
    }

    return 1;
}
