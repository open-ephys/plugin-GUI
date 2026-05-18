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

#include <vector>

namespace fs = std::filesystem;

static inline File getPluginsDirectory();
static inline File getSharedDirectory();

namespace
{
constexpr auto pluginGatewayUrl = "https://open-ephys-plugin-gateway.herokuapp.com/";

struct InstalledPluginState
{
    HashMap<String, String> versions;
    HashMap<String, String> dllNames;
};

struct PluginCatalog
{
    String downloadUrl;
    var pluginData;
    HashMap<String, String> dependencyVersions;
};

StringArray getCompatibleVersions (const var& versions)
{
    StringArray compatibleVersions;

    if (auto* allVersions = versions.getArray())
    {
        for (const auto& value : *allVersions)
        {
            const auto version = value.toString();
            const auto apiVer = version.fromLastOccurrenceOf ("API", false, false);

            if (apiVer.equalsIgnoreCase (String (PLUGIN_API_VER)))
                compatibleVersions.add (version);
        }
    }

    return compatibleVersions;
}

String getLatestCompatibleVersion (const var& versions)
{
    auto compatibleVersions = getCompatibleVersions (versions);

    if (compatibleVersions.isEmpty())
        return {};

    compatibleVersions.sort (true);
    return compatibleVersions[compatibleVersions.size() - 1];
}

String getLatestCompatibleVersionOrDefault (const var& versions)
{
    if (const auto latest = getLatestCompatibleVersion (versions); latest.isNotEmpty())
        return latest;

    return "0.0.0-API" + String (PLUGIN_API_VER);
}

bool parseGatewayCatalog (PluginCatalog& catalog, String& errorMessage)
{
    const auto response = URL (pluginGatewayUrl).readEntireTextStream();

    if (response.isEmpty())
    {
        errorMessage = "Unable to fetch plugins! Please check your internet connection and try again.";
        return false;
    }

    var gatewayData;
    if (const auto result = JSON::parse (response, gatewayData); result.failed())
    {
        errorMessage = result.getErrorMessage();
        return false;
    }

    catalog.downloadUrl = gatewayData.getProperty ("download_url", {}).toString();
    catalog.pluginData = gatewayData.getProperty ("plugins", {});
    catalog.dependencyVersions.clear();

    if (const auto* plugins = catalog.pluginData.getArray())
    {
        for (const auto& entry : *plugins)
        {
            const auto pluginName = entry.getProperty ("name", {}).toString();
            const auto pluginType = entry.getProperty ("type", {}).toString();

            if (! pluginType.equalsIgnoreCase ("CommonLib"))
                continue;

            if (const auto latestVersion = getLatestCompatibleVersion (entry.getProperty ("versions", {})); latestVersion.isNotEmpty())
                catalog.dependencyVersions.set (pluginName, latestVersion);
        }
    }

    return true;
}

bool readInstalledPluginState (InstalledPluginState& installedState)
{
    const auto xmlFile = getPluginsDirectory().getChildFile ("installedPlugins.xml");
    XmlDocument doc (xmlFile);
    std::unique_ptr<XmlElement> xml (doc.getDocumentElement());

    if (xml == nullptr || ! xml->hasTagName ("PluginInstaller"))
        return false;

    installedState.versions.clear();
    installedState.dllNames.clear();

    if (auto* child = xml->getFirstChildElement())
    {
        for (auto* pluginElement : child->getChildIterator())
        {
            const auto pluginName = pluginElement->getTagName();
            installedState.versions.set (pluginName, pluginElement->getStringAttribute ("version"));
            installedState.dllNames.set (pluginName, pluginElement->getStringAttribute ("dllName"));
        }
    }

    return true;
}

int findPluginIndexByProperty (const var& pluginData, const Identifier& property, const String& value)
{
    if (const auto* plugins = pluginData.getArray())
    {
        for (int index = 0; index < plugins->size(); ++index)
        {
            if ((*plugins)[index].getProperty (property, {}).toString().equalsIgnoreCase (value))
                return index;
        }
    }

    return -1;
}

SelectedPluginInfo createSelectedPluginInfo (const var& entry,
                                             const InstalledPluginState& installedState,
                                             const HashMap<String, String>& dependencyVersions)
{
    SelectedPluginInfo pluginInfo;
    pluginInfo.pluginName = entry.getProperty ("name", {}).toString();
    pluginInfo.displayName = entry.getProperty ("display_name", pluginInfo.pluginName).toString();
    pluginInfo.type = entry.getProperty ("type", {}).toString();
    pluginInfo.developers = entry.getProperty ("developers", {}).toString();

    const auto updated = entry.getProperty ("updated", {}).toString();
    pluginInfo.lastUpdated = updated.upToFirstOccurrenceOf ("T", false, false);
    pluginInfo.description = entry.getProperty ("desc", {}).toString();
    pluginInfo.docURL = entry.getProperty ("docs", {}).toString();
    pluginInfo.versions = getCompatibleVersions (entry.getProperty ("versions", {}));
    pluginInfo.versions.sort (true);
    pluginInfo.selectedVersion = {};
    pluginInfo.latestVersion = {};

    if (! pluginInfo.versions.isEmpty())
        pluginInfo.latestVersion = pluginInfo.versions[pluginInfo.versions.size() - 1];
    pluginInfo.installedVersion = installedState.versions[pluginInfo.pluginName];
    pluginInfo.dependencies.clear();
    pluginInfo.dependencyVersions.clear();

    if (auto* dependencies = entry.getProperty ("dependencies", {}).getArray())
    {
        for (const auto& dependencyValue : *dependencies)
        {
            const auto dependency = dependencyValue.toString();

            if (dependency.equalsIgnoreCase ("None"))
                continue;

            pluginInfo.dependencies.add (dependency);
            pluginInfo.dependencyVersions.add (dependencyVersions[dependency]);
        }
    }

    if (pluginInfo.selectedVersion.isEmpty())
        pluginInfo.selectedVersion = pluginInfo.latestVersion;

    if (pluginInfo.selectedVersion.isEmpty() && ! pluginInfo.versions.isEmpty())
        pluginInfo.selectedVersion = pluginInfo.versions[pluginInfo.versions.size() - 1];

    if (pluginInfo.selectedVersion.isEmpty())
        pluginInfo.selectedVersion = pluginInfo.installedVersion;

    return pluginInfo;
}
} // namespace

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
        setSize (1200, 640);

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
        setResizeLimits (1200, 640, 8192, 5120);

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

int PluginInstaller::checkForPluginUpdates()
{
    LOGD ("Checking for plugin updates...");

    InstalledPluginState installedState;
    if (! readInstalledPluginState (installedState))
    {
        LOGD ("[PluginInstaller] installedPlugins.xml not found.");
        return 0;
    }

    PluginCatalog catalog;
    String errorMessage;
    if (! parseGatewayCatalog (catalog, errorMessage))
    {
        LOGE ("Unable to fetch plugin updates! Please check your internet connection.");
        return 0;
    }

    updatablePlugins.clear();

    if (const auto* plugins = catalog.pluginData.getArray())
    {
        for (const auto& plugin : *plugins)
        {
            const auto pluginName = plugin.getProperty ("name", {}).toString();
            const auto installedVersion = installedState.versions[pluginName];

            if (installedVersion.isEmpty())
                continue;

            const auto latestVersion = getLatestCompatibleVersionOrDefault (plugin.getProperty ("versions", {}));

            if (latestVersion.compareNatural (installedVersion) > 0)
            {
                updatablePlugins.add (pluginName);
                LOGD ("Plugin update available: ", pluginName);
            }
        }
    }

    LOGD ("Found ", updatablePlugins.size(), " plugin(s) with updates available.");
    return updatablePlugins.size();
}

void PluginInstaller::installPluginAndDependency (const String& plugin, String version)
{
    PluginInstallActionRunner actionRunner;

    PluginCatalog catalog;
    String errorMessage;
    if (! parseGatewayCatalog (catalog, errorMessage))
    {
        LOGE (errorMessage)
        return;
    }

    actionRunner.setDownloadURL (catalog.downloadUrl);

    const auto pluginIndex = findPluginIndexByProperty (catalog.pluginData, "display_name", plugin);

    if (pluginIndex < 0)
    {
        LOGE ("Automated Plugin Installation Failed! Plugin not found!")
        return;
    }

    const auto selectedEntry = catalog.pluginData[pluginIndex];
    auto* platforms = selectedEntry.getProperty ("platforms", {}).getArray();

    if (platforms == nullptr || ! platforms->contains (osType))
    {
        LOGD ("No platform specific package found for ", plugin);
        return;
    }

    LOGC (plugin, " plugin found! Installing it now...")

    SelectedPluginInfo requiredPluginInfo;

    requiredPluginInfo.pluginName = selectedEntry.getProperty ("name", {}).toString();
    requiredPluginInfo.displayName = plugin;
    requiredPluginInfo.type = selectedEntry.getProperty ("type", {}).toString();
    requiredPluginInfo.versions = getCompatibleVersions (selectedEntry.getProperty ("versions", {}));

    requiredPluginInfo.dependencies.clear();
    requiredPluginInfo.dependencyVersions.clear();

    if (auto* dependencies = selectedEntry.getProperty ("dependencies", {}).getArray())
    {
        for (const auto& dependencyValue : *dependencies)
        {
            const auto dependency = dependencyValue.toString();

            if (dependency.equalsIgnoreCase ("None"))
                continue;

            requiredPluginInfo.dependencies.add (dependency);

            if (const auto dependencyVersion = catalog.dependencyVersions[dependency]; dependencyVersion.isNotEmpty())
            {
                requiredPluginInfo.dependencyVersions.add (dependencyVersion);
                continue;
            }

            LOGE ("Automated Plugin Installation Failed! Compatible plugin version not found!")
            return;
        }
    }

    actionRunner.setPluginInfo (requiredPluginInfo);

    for (int i = 0; i < requiredPluginInfo.dependencies.size(); i++)
    {
        actionRunner.downloadPlugin (requiredPluginInfo.dependencies[i],
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

    int code = actionRunner.downloadPlugin (requiredPluginInfo.pluginName, version, false);

    if (code == 1)
        LOGC ("Install successful!!")
    else
        LOGC ("Install failed!!");
}

namespace
{
String getDependenciesText (const SelectedPluginInfo& pluginInfo)
{
    return pluginInfo.dependencies.isEmpty() ? "None" : pluginInfo.dependencies.joinIntoString (", ");
}

String getSelectedVersionOrFallback (const SelectedPluginInfo& pluginInfo)
{
    if (pluginInfo.selectedVersion.isNotEmpty())
        return pluginInfo.selectedVersion;

    if (pluginInfo.latestVersion.isNotEmpty())
        return pluginInfo.latestVersion;

    if (! pluginInfo.versions.isEmpty())
        return pluginInfo.versions[pluginInfo.versions.size() - 1];

    return {};
}

String getInstallActionLabel (const SelectedPluginInfo& pluginInfo)
{
    if (pluginInfo.versions.isEmpty())
        return "Unavailable";

    const auto selectedVersion = getSelectedVersionOrFallback (pluginInfo);

    if (selectedVersion.isEmpty())
        return "Unavailable";

    if (pluginInfo.installedVersion.isEmpty())
        return "Install";

    const auto result = selectedVersion.compareNatural (pluginInfo.installedVersion);

    if (result == 0)
        return "Installed";

    return result > 0 ? "Upgrade" : "Downgrade";
}

bool canInstallPlugin (const SelectedPluginInfo& pluginInfo)
{
    const auto label = getInstallActionLabel (pluginInfo);
    return label != "Installed" && label != "Unavailable";
}

Path createSvgPath (std::initializer_list<const char*> svgPathSegments)
{
    Path path;

    for (const auto* segment : svgPathSegments)
        path.addPath (Drawable::parseSVGPath (segment));

    return path;
}

const Path& getInstallIconPath()
{
    static const auto path = createSvgPath ({ "M4 17v2a2 2 0 0 0 2 2h12a2 2 0 0 0 2 -2v-2",
                                              "M7 11l5 5l5 -5",
                                              "M12 4l0 12" });

    return path;
}

const Path& getRemoveIconPath()
{
    static const auto path = createSvgPath ({ "M4 7h16",
                                              "M5 7l1 12a2 2 0 0 0 2 2h8a2 2 0 0 0 2 -2l1 -12",
                                              "M9 7v-3a1 1 0 0 1 1 -1h4a1 1 0 0 1 1 1v3",
                                              "M10 12l4 4m0 -4l-4 4" });

    return path;
}

const Path& getDocsIconPath()
{
    static const auto path = createSvgPath ({ "M14 3v4a1 1 0 0 0 1 1h4",
                                              "M17 21h-10a2 2 0 0 1 -2 -2v-14a2 2 0 0 1 2 -2h7l5 5v11a2 2 0 0 1 -2 2",
                                              "M11 14h1v4h1",
                                              "M12 11h.01" });

    return path;
}

class PluginIconButton : public Button
{
public:
    enum class IconType
    {
        install,
        remove,
        docs
    };

    explicit PluginIconButton (IconType iconType)
        : Button (iconType == IconType::install ? "Install"
                                                : (iconType == IconType::remove ? "Remove" : "Docs")),
          icon (iconType)
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        const auto enabled = isEnabled();

        auto background = findColour (ThemeColours::widgetBackground);
        auto outline = hasKeyboardFocus (false) ? findColour (ThemeColours::highlightedFill) : findColour (ThemeColours::outline);
        outline = outline.withAlpha (enabled ? 0.9f : 0.4f);
        auto iconColour = Colours::dodgerblue;

        if (icon == IconType::install)
            iconColour = Colours::green;
        else if (icon == IconType::remove)
            iconColour = Colours::red;

        iconColour = iconColour.withAlpha (enabled ? 0.95f : 0.28f);

        if (enabled && isMouseOverButton)
            background = background.brighter (isButtonDown ? 0.08f : 0.16f);

        if (enabled && isButtonDown)
            outline = outline.withAlpha (0.42f);

        g.setColour (background);
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (outline);
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        auto iconBounds = bounds.reduced (7.0f);
        const auto& iconPath = icon == IconType::install ? getInstallIconPath()
                                                         : (icon == IconType::remove ? getRemoveIconPath() : getDocsIconPath());
        auto transform = iconPath.getTransformToScaleToFit (iconBounds, true, Justification::centred);

        g.setColour (iconColour);
        g.strokePath (iconPath, PathStrokeType (1.5f, PathStrokeType::curved, PathStrokeType::rounded), transform);
    }

private:
    IconType icon;
};

class PluginVersionCell : public Component
{
public:
    PluginVersionCell()
    {
        addAndMakeVisible (versionMenu);
        versionMenu.setJustificationType (Justification::centred);
        versionMenu.setTextWhenNoChoicesAvailable ("- N/A -");
        versionMenu.onChange = [this]
        {
            if (onVersionChanged != nullptr)
                onVersionChanged (versionMenu.getText());
        };
    }

    void update (const SelectedPluginInfo& pluginInfo,
                 std::function<void (const String&)> callback)
    {
        onVersionChanged = std::move (callback);

        versionMenu.clear (dontSendNotification);

        for (int index = 0; index < pluginInfo.versions.size(); ++index)
            versionMenu.addItem (pluginInfo.versions[index], index + 1);

        const auto selectedVersion = getSelectedVersionOrFallback (pluginInfo);
        const auto selectedIndex = pluginInfo.versions.indexOf (selectedVersion);

        if (selectedIndex >= 0)
            versionMenu.setSelectedId (selectedIndex + 1, dontSendNotification);

        versionMenu.setEnabled (! pluginInfo.versions.isEmpty());
        versionMenu.setTooltip (selectedVersion);
    }

    void resized() override
    {
        versionMenu.setBounds (getLocalBounds().reduced (2, 6));
    }

private:
    ComboBox versionMenu;
    std::function<void (const String&)> onVersionChanged;
};

class PluginIconButtonCell : public Component
{
public:
    explicit PluginIconButtonCell (PluginIconButton::IconType iconType)
        : button (iconType)
    {
        addAndMakeVisible (button);
        button.onClick = [this]
        {
            if (onClick != nullptr)
                onClick();
        };
    }

    void update (const String& label,
                 bool isEnabled,
                 String tooltip,
                 std::function<void()> callback)
    {
        onClick = std::move (callback);
        button.setButtonText (label);
        button.setEnabled (isEnabled);
        button.setTooltip (std::move (tooltip));
    }

    void resized() override
    {
        button.setBounds (getLocalBounds().reduced (2, 4));
    }

private:
    PluginIconButton button;
    std::function<void()> onClick;
};
} // namespace

/* ================================== Plugin Installer Component ================================== */

PluginInstallerComponent::PluginInstallerComponent()
{
    font = FontOptions ("Inter", "Regular", 17.0f);
    setSize (getWidth() - 10, getHeight() - 10);

    addAndMakeVisible (searchLabel);
    searchLabel.setFont (font);
    searchLabel.setText ("Search:", dontSendNotification);

    addAndMakeVisible (searchEditor);
    searchEditor.setJustification (Justification::centredLeft);
    searchEditor.setTextToShowWhenEmpty ("Search by display name...", Colours::grey);
    searchEditor.setFont (FontOptions ("Inter", "Regular", 15.0f));
    searchEditor.setPopupMenuEnabled (false);
    searchEditor.onTextChange = [this]
    { applyTableFilters(); };
    searchEditor.onEscapeKey = [this]
    {
        searchEditor.clear();
        applyTableFilters();
        searchEditor.giveAwayKeyboardFocus();
    };

    addAndMakeVisible (viewLabel);
    viewLabel.setFont (font);
    viewLabel.setText ("View:", dontSendNotification);

    addAndMakeVisible (allButton);
    allButton.setButtonText ("All");
    allButton.setRadioGroupId (101, dontSendNotification);
    allButton.setToggleState (true, dontSendNotification);
    allButton.addListener (this);

    addAndMakeVisible (installedButton);
    installedButton.setButtonText ("Installed");
    installedButton.setRadioGroupId (101, dontSendNotification);
    installedButton.addListener (this);

    updatesButton = std::make_unique<ShapeButton> ("Refresh Plugins",
                                                    Colours::transparentBlack,
                                                    Colours::transparentBlack,
                                                    Colours::transparentBlack);
    String reloadIconPath = "M19.933 13.041a8 8 0 1 1 -9.925 -8.788c3.899 -1 7.935 1.007 9.425 4.747 M20 4v5h-5";
    updatesButton->setShape (Drawable::parseSVGPath (reloadIconPath).createPathWithRoundedCorners(2.0f), true, true, false);
    updatesButton->setOutline (findColour (ThemeColours::defaultText), 2.0f);
    updatesButton->setMouseCursor (MouseCursor::PointingHandCursor);
    updatesButton->setTooltip ("Refresh Plugins");
    updatesButton->addListener (this);
    addAndMakeVisible (updatesButton.get());

    addAndMakeVisible (typeLabel);
    typeLabel.setFont (font);
    typeLabel.setText ("Type:", dontSendNotification);

    addAndMakeVisible (sourceType);
    sourceType.setButtonText ("Source");
    sourceType.setToggleState (true, dontSendNotification);
    sourceType.addListener (this);

    addAndMakeVisible (filterType);
    filterType.setButtonText ("Filter");
    filterType.setToggleState (true, dontSendNotification);
    filterType.addListener (this);

    addAndMakeVisible (sinkType);
    sinkType.setButtonText ("Sink");
    sinkType.setToggleState (true, dontSendNotification);
    sinkType.addListener (this);

    addAndMakeVisible (otherType);
    otherType.setButtonText ("Other");
    otherType.setToggleState (true, dontSendNotification);
    otherType.addListener (this);

    addAndMakeVisible (pluginListAndInfo);
    applyTableFilters();
}

void PluginInstallerComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::componentBackground).darker());
    g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.2f));
    g.fillRect (10, 50, getWidth() - 20, 1);
}

void PluginInstallerComponent::resized()
{
    searchLabel.setBounds (20, 10, 60, 28);
    searchEditor.setBounds (80, 10, 250, 28);

    viewLabel.setBounds (350, 10, 50, 28);
    allButton.setBounds (400, 10, 55, 28);
    installedButton.setBounds (460, 10, 95, 28);

    typeLabel.setBounds (570, 10, 50, 28);
    sourceType.setBounds (625, 10, 80, 28);
    filterType.setBounds (710, 10, 70, 28);
    sinkType.setBounds (785, 10, 65, 28);
    otherType.setBounds (855, 10, 75, 28);

    updatesButton->setBounds (getWidth() - 44, 10, 20, 20);

    pluginListAndInfo.setBounds (10, 64, getWidth() - 20, getHeight() - 94);
}

void PluginInstallerComponent::buttonClicked (Button* button)
{
    if (button == updatesButton.get())
    {
        MouseCursor::showWaitCursor();
        pluginListAndInfo.refreshCatalog();
        MouseCursor::hideWaitCursor();
    }

    applyTableFilters();
}

void PluginInstallerComponent::colourChanged()
{
    updatesButton->setOutline (findColour (ThemeColours::defaultText), 2.0f);
}

void PluginInstallerComponent::applyTableFilters()
{
    pluginListAndInfo.setSearchText (searchEditor.getText());
    pluginListAndInfo.setShowInstalledOnly (installedButton.getToggleState());
    pluginListAndInfo.setTypeFilters (sourceType.getToggleState(),
                                      filterType.getToggleState(),
                                      sinkType.getToggleState(),
                                      otherType.getToggleState());
}

/* ================================== Plugin Table Component ================================== */

PluginListBoxComponent::PluginListBoxComponent()
{
    tableFont = FontOptions ("Inter", "Regular", 14.0f);
    headerFont = FontOptions ("Inter", "Semi Bold", 15.0f);

    addAndMakeVisible (pluginTable);
    pluginTable.setModel (this);
    pluginTable.setRowHeight (38);
    pluginTable.setHeaderHeight (30);
    pluginTable.getViewport()->setScrollBarThickness (12);
    pluginTable.grabKeyboardFocus();

    auto& header = pluginTable.getHeader();
    constexpr int sortableColumnFlags = TableHeaderComponent::visible | TableHeaderComponent::resizable | TableHeaderComponent::sortable;
    constexpr int regularColumnFlags = TableHeaderComponent::visible | TableHeaderComponent::resizable;

    header.addColumn ("Plugin", displayNameColumn, 180, 120, -1, sortableColumnFlags);
    header.addColumn ("Type", typeColumn, 90, 90, 90, TableHeaderComponent::visible);
    header.addColumn ("Developers", developersColumn, 150, 100, 280, regularColumnFlags);
    header.addColumn ("Installed", installedVersionColumn, 80, 80, 80, TableHeaderComponent::visible);
    header.addColumn ("Updated", updatedColumn, 90, 90, 90, TableHeaderComponent::visible);
    header.addColumn ("Description", descriptionColumn, 250, 180, 420, regularColumnFlags);
    header.addColumn ("Dependencies", dependenciesColumn, 120, 120, 120, TableHeaderComponent::appearsOnColumnMenu);
    header.addColumn ("Version", versionSelectorColumn, 120, 120, 220, regularColumnFlags);
    header.addColumn ("Docs", documentationColumn, 60, 60, 60, TableHeaderComponent::visible);
    header.addColumn ("Install", installColumn, 60, 60, 60, TableHeaderComponent::visible);
    header.addColumn ("Remove", uninstallColumn, 60, 60, 60, TableHeaderComponent::visible);
    header.setSortColumnId (displayNameColumn, true);

    tableDropShadower.setOwner (&pluginTable);

    actionRunner.setOperationCompleteHandler ([this] (const SelectedPluginInfo& pluginInfo, bool isInstalled)
                                              { updatePluginState (pluginInfo, isInstalled); });

    refreshCatalog();
}

int PluginListBoxComponent::getNumRows()
{
    return static_cast<int> (visibleRows.size());
}

void PluginListBoxComponent::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (const auto* pluginInfo = getPluginForVisibleRow (rowNumber))
    {
        auto background = rowNumber % 2 == 0
                              ? findColour (ThemeColours::componentBackground)
                              : findColour (ThemeColours::componentBackground).darker (0.12f);

        g.fillAll (background);

        if (pluginInfo->hasUpdate)
        {
            g.setColour (Colours::green.withAlpha (0.8f));
            g.fillRect (0, 0, 4, height);
        }

        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.08f));
        g.fillRect (0, height - 1, width, 1);
    }
}

void PluginListBoxComponent::paintCell (Graphics& g,
                                        int rowNumber,
                                        int columnId,
                                        int width,
                                        int height,
                                        bool /*rowIsSelected*/)
{
    const auto* pluginInfo = getPluginForVisibleRow (rowNumber);

    if (pluginInfo == nullptr)
        return;

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (tableFont);

    juce::Rectangle<int> textBounds (5, 0, width - 10, height);
    String text;

    switch (columnId)
    {
        case displayNameColumn:
            g.setFont (headerFont);
            text = pluginInfo->displayName;
            break;

        case typeColumn:
            text = pluginInfo->type;
            break;

        case developersColumn:
            text = pluginInfo->developers;
            break;

        case installedVersionColumn:
            text = pluginInfo->installedVersion.isEmpty() ? "No" : pluginInfo->installedVersion;
            break;

        case updatedColumn:
            text = pluginInfo->lastUpdated;
            break;

        case descriptionColumn:
            text = pluginInfo->description;
            break;

        case dependenciesColumn:
            text = getDependenciesText (*pluginInfo);
            break;

        default:
            return;
    }

    g.drawText (text.isNotEmpty() ? text : String ("-"), textBounds, Justification::centredLeft, true);
}

Component* PluginListBoxComponent::refreshComponentForCell (int rowNumber,
                                                            int columnId,
                                                            bool /*isRowSelected*/,
                                                            Component* existingComponentToUpdate)
{
    const auto* pluginInfo = getPluginForVisibleRow (rowNumber);

    if (pluginInfo == nullptr)
        return nullptr;

    if (columnId == versionSelectorColumn)
    {
        auto* versionCell = dynamic_cast<PluginVersionCell*> (existingComponentToUpdate);

        if (versionCell == nullptr)
            versionCell = new PluginVersionCell();

        versionCell->update (*pluginInfo,
                             [this, rowNumber] (const String& version)
                             {
                                 setSelectedVersion (rowNumber, version);
                             });

        return versionCell;
    }

    if (columnId == documentationColumn)
    {
        auto* docsCell = dynamic_cast<PluginIconButtonCell*> (existingComponentToUpdate);

        if (docsCell == nullptr)
            docsCell = new PluginIconButtonCell (PluginIconButton::IconType::docs);

        const auto docsUrl = pluginInfo->docURL;
        docsCell->update ("Docs",
                          docsUrl.isNotEmpty(),
                          docsUrl,
                          [docsUrl]
                          {
                              if (docsUrl.isNotEmpty())
                                  URL (docsUrl).launchInDefaultBrowser();
                          });
        return docsCell;
    }

    if (columnId == installColumn || columnId == uninstallColumn)
    {
        auto* actionCell = dynamic_cast<PluginIconButtonCell*> (existingComponentToUpdate);

        if (actionCell == nullptr)
        {
            actionCell = new PluginIconButtonCell (columnId == installColumn
                                                       ? PluginIconButton::IconType::install
                                                       : PluginIconButton::IconType::remove);
        }

        if (columnId == installColumn)
        {
            actionCell->update (getInstallActionLabel (*pluginInfo),
                                canInstallPlugin (*pluginInfo),
                                "Install the selected plugin version",
                                [this, rowNumber]
                                {
                                    installPluginForRow (rowNumber);
                                });
        }
        else
        {
            actionCell->update ("Remove",
                                pluginInfo->installedVersion.isNotEmpty(),
                                "Uninstall the currently installed version",
                                [this, rowNumber]
                                {
                                    uninstallPluginForRow (rowNumber);
                                });
        }

        return actionCell;
    }

    jassert (existingComponentToUpdate == nullptr);
    return nullptr;
}

String PluginListBoxComponent::getCellTooltip (int rowNumber, int columnId)
{
    const auto* pluginInfo = getPluginForVisibleRow (rowNumber);

    if (pluginInfo == nullptr)
        return {};

    switch (columnId)
    {
        case displayNameColumn:
            return pluginInfo->hasUpdate ? pluginInfo->displayName + " has an update available." : pluginInfo->displayName;

        case developersColumn:
            return pluginInfo->developers;

        case descriptionColumn:
            return pluginInfo->description;

        case dependenciesColumn:
            return getDependenciesText (*pluginInfo);

        default:
            return {};
    }
}

void PluginListBoxComponent::sortOrderChanged (int newSortColumnId, bool /*isForwards*/)
{
    if (newSortColumnId == displayNameColumn)
        applyFilters();
}

int PluginListBoxComponent::getColumnAutoSizeWidth (int columnId)
{
    if (columnId != displayNameColumn)
        return 0;

    auto maxWidth = GlyphArrangement::getStringWidthInt (Font (headerFont), pluginTable.getHeader().getColumnName (displayNameColumn));

    for (const auto& pluginInfo : allPlugins)
        maxWidth = jmax (maxWidth, GlyphArrangement::getStringWidthInt (Font (headerFont), pluginInfo.displayName));

    return maxWidth + 28;
}

void PluginListBoxComponent::resized()
{
    pluginTable.setBounds (getLocalBounds().reduced (6));
}

void PluginListBoxComponent::setSearchText (const String& text)
{
    searchText = text.trim();
    applyFilters();
}

void PluginListBoxComponent::setShowInstalledOnly (bool shouldShowInstalledOnly)
{
    showInstalledOnly = shouldShowInstalledOnly;
    applyFilters();
}

void PluginListBoxComponent::setTypeFilters (bool shouldShowSources,
                                             bool shouldShowFilters,
                                             bool shouldShowSinks,
                                             bool shouldShowOther)
{
    showSources = shouldShowSources;
    showFilters = shouldShowFilters;
    showSinks = shouldShowSinks;
    showOther = shouldShowOther;
    applyFilters();
}

bool PluginListBoxComponent::refreshCatalog()
{
    HashMap<String, String> selectedVersions;
    for (const auto& pluginInfo : allPlugins)
        selectedVersions.set (pluginInfo.pluginName, getSelectedVersionOrFallback (pluginInfo));

    PluginCatalog catalog;
    String errorMessage;
    if (! parseGatewayCatalog (catalog, errorMessage))
    {
        LOGE (errorMessage);
        return false;
    }

    InstalledPluginState installedState;
    readInstalledPluginState (installedState);

    std::vector<SelectedPluginInfo> loadedPlugins;

    if (const auto* plugins = catalog.pluginData.getArray())
    {
        loadedPlugins.reserve (plugins->size());

        for (const auto& entry : *plugins)
        {
            const auto type = entry.getProperty ("type", {}).toString();
            auto* platforms = entry.getProperty ("platforms", {}).getArray();

            if (type.equalsIgnoreCase ("CommonLib") || platforms == nullptr || ! platforms->contains (osType))
                continue;

            auto pluginInfo = createSelectedPluginInfo (entry, installedState, catalog.dependencyVersions);

            if (pluginInfo.versions.isEmpty())
                continue;

            if (const auto selectedVersion = selectedVersions[pluginInfo.pluginName]; pluginInfo.versions.contains (selectedVersion))
                pluginInfo.selectedVersion = selectedVersion;

            loadedPlugins.push_back (std::move (pluginInfo));
        }
    }

    allPlugins = std::move (loadedPlugins);
    actionRunner.setDownloadURL (catalog.downloadUrl);
    pluginTable.autoSizeColumn (displayNameColumn);

    updatablePlugins.clear();

    for (auto& pluginInfo : allPlugins)
    {
        pluginInfo.hasUpdate = pluginInfo.installedVersion.isNotEmpty()
                               && pluginInfo.latestVersion.isNotEmpty()
                               && pluginInfo.latestVersion.compareNatural (pluginInfo.installedVersion) > 0;

        if (pluginInfo.hasUpdate)
            updatablePlugins.add (pluginInfo.pluginName);
    }

    applyFilters();
    return true;
}

void PluginListBoxComponent::applyFilters()
{
    visibleRows.clear();

    for (int index = 0; index < static_cast<int> (allPlugins.size()); ++index)
    {
        if (matchesCurrentFilters (allPlugins[static_cast<size_t> (index)]))
            visibleRows.push_back (index);
    }

    std::sort (visibleRows.begin(), visibleRows.end(), [this] (int lhs, int rhs)
               {
        const auto result = allPlugins[static_cast<size_t> (lhs)].displayName.compareNatural (
            allPlugins[static_cast<size_t> (rhs)].displayName);

        if (result == 0)
            return allPlugins[static_cast<size_t> (lhs)].pluginName.compareNatural (allPlugins[static_cast<size_t> (rhs)].pluginName) < 0;

        return pluginTable.getHeader().isSortedForwards() ? result < 0 : result > 0; });

    pluginTable.updateContent();
    pluginTable.repaint();
}

bool PluginListBoxComponent::matchesCurrentFilters (const SelectedPluginInfo& pluginInfo) const
{
    if (showInstalledOnly && pluginInfo.installedVersion.isEmpty())
        return false;

    if (! searchText.isEmpty() && ! pluginInfo.displayName.containsIgnoreCase (searchText))
        return false;

    const auto type = pluginInfo.type;
    const auto isSource = type.containsWholeWordIgnoreCase ("source");
    const auto isFilter = type.containsWholeWordIgnoreCase ("filter");
    const auto isSink = type.containsWholeWordIgnoreCase ("sink");
    const auto matchesType = (showSources && isSource)
                             || (showFilters && isFilter)
                             || (showSinks && isSink)
                             || (showOther && isOtherType (type));

    return matchesType;
}

bool PluginListBoxComponent::isOtherType (const String& type) const
{
    return ! type.containsWholeWordIgnoreCase ("source")
           && ! type.containsWholeWordIgnoreCase ("filter")
           && ! type.containsWholeWordIgnoreCase ("sink");
}

SelectedPluginInfo* PluginListBoxComponent::getPluginForVisibleRow (int rowNumber)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int> (visibleRows.size()))
        return nullptr;

    return &allPlugins[static_cast<size_t> (visibleRows[static_cast<size_t> (rowNumber)])];
}

const SelectedPluginInfo* PluginListBoxComponent::getPluginForVisibleRow (int rowNumber) const
{
    if (rowNumber < 0 || rowNumber >= static_cast<int> (visibleRows.size()))
        return nullptr;

    return &allPlugins[static_cast<size_t> (visibleRows[static_cast<size_t> (rowNumber)])];
}

void PluginListBoxComponent::setSelectedVersion (int rowNumber, const String& version)
{
    if (auto* pluginInfo = getPluginForVisibleRow (rowNumber))
    {
        pluginInfo->selectedVersion = version;
        pluginTable.repaintRow (rowNumber);
    }
}

void PluginListBoxComponent::installPluginForRow (int rowNumber)
{
    if (auto* pluginInfo = getPluginForVisibleRow (rowNumber))
    {
        actionRunner.setPluginInfo (*pluginInfo);
        actionRunner.installSelectedPlugin();
    }
}

void PluginListBoxComponent::uninstallPluginForRow (int rowNumber)
{
    if (auto* pluginInfo = getPluginForVisibleRow (rowNumber))
    {
        actionRunner.setPluginInfo (*pluginInfo);
        actionRunner.uninstallSelectedPlugin();
    }
}

void PluginListBoxComponent::updatePluginState (const SelectedPluginInfo& pluginInfo, bool isInstalled)
{
    for (auto& currentPlugin : allPlugins)
    {
        if (! currentPlugin.pluginName.equalsIgnoreCase (pluginInfo.pluginName))
            continue;

        currentPlugin.selectedVersion = pluginInfo.selectedVersion;
        currentPlugin.installedVersion = isInstalled ? pluginInfo.installedVersion : String();
        currentPlugin.hasUpdate = currentPlugin.installedVersion.isNotEmpty()
                                  && currentPlugin.latestVersion.isNotEmpty()
                                  && currentPlugin.latestVersion.compareNatural (currentPlugin.installedVersion) > 0;
        break;
    }

    updatablePlugins.clear();

    for (const auto& currentPlugin : allPlugins)
    {
        if (currentPlugin.hasUpdate)
            updatablePlugins.add (currentPlugin.pluginName);
    }

    applyFilters();
}

/* ================================== Plugin Install Action Runner ================================== */

PluginInstallActionRunner::PluginInstallActionRunner() : ThreadWithProgressWindow ("Plugin Installer", true, false)
{
}

void PluginInstallActionRunner::setDownloadURL (const String& url)
{
    downloadURL = url;
}

void PluginInstallActionRunner::setOperationCompleteHandler (OperationCompleteHandler handler)
{
    operationCompleteHandler = std::move (handler);
}

void PluginInstallActionRunner::setPluginInfo (const SelectedPluginInfo& p)
{
    pInfo = p;

    if (pInfo.selectedVersion.isEmpty())
    {
        if (pInfo.latestVersion.isNotEmpty())
            pInfo.selectedVersion = pInfo.latestVersion;
        else if (! pInfo.versions.isEmpty())
            pInfo.selectedVersion = pInfo.versions[pInfo.versions.size() - 1];
    }
}

void PluginInstallActionRunner::installSelectedPlugin()
{
    runThread();
}

bool PluginInstallActionRunner::uninstallSelectedPlugin()
{
    if (! uninstallPlugin (pInfo.pluginName))
    {
        LOGE ("Failed to uninstall ", pInfo.displayName);
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "[Plugin Installer] " + pInfo.displayName,
                                          "Failed to uninstall " + pInfo.displayName);
        return false;
    }

    LOGC (pInfo.displayName, " uninstalled successfully!");
    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                      "[Plugin Installer] " + pInfo.displayName,
                                      pInfo.displayName + " uninstalled successfully");
    return true;
}

void PluginInstallActionRunner::showAlertOnMessageThread (MessageBoxIconType iconType, const String& title, const String& message)
{
    MessageManager::callAsync ([=]()
                               { AlertWindow::showMessageBoxAsync (iconType, title, message); });
}

void PluginInstallActionRunner::updateUIOnMessageThread()
{
    MessageManager::callAsync ([this]()
                               {
        pInfo.installedVersion = pInfo.selectedVersion;
        notifyOperationComplete (true); });
}

void PluginInstallActionRunner::notifyOperationComplete (bool isInstalled)
{
    if (operationCompleteHandler != nullptr)
        operationCompleteHandler (pInfo, isInstalled);
}

void PluginInstallActionRunner::run()
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
        if (i >= pInfo.dependencyVersions.size() || pInfo.dependencyVersions[i].isEmpty())
        {
            showAlertOnMessageThread (AlertWindow::WarningIcon,
                                      "[Plugin Installer] " + pInfo.dependencies[i],
                                      "No compatible dependency version is available for " + pInfo.dependencies[i] + ".");
            LOGE ("Compatible dependency version not found for ", pInfo.dependencies[i]);
            return;
        }

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

bool PluginInstallActionRunner::uninstallPlugin (const String& plugin)
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

        if (pluginElement == nullptr)
            return false;

        dllName = pluginElement->getStringAttribute ("dllName");
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

    pInfo.installedVersion = {};
    notifyOperationComplete (false);

    //delete plugin file
    File pluginFile = getPluginsDirectory().getChildFile (dllName);
    if (! pluginFile.deleteRecursively())
    {
        LOGE ("Unable to delete plugin file ", pluginFile.getFullPathName(), " ... Please remove it manually!!");
        return false;
    }

    return true;
}

int PluginInstallActionRunner::downloadPlugin (const String& plugin, const String& version, bool isDependency)
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
