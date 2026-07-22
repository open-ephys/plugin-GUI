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

#ifndef PLUGININSTALLER_H_INCLUDED
#define PLUGININSTALLER_H_INCLUDED

#include <functional>
#include <vector>

#include "../../JuceLibraryCode/JuceHeader.h"

#define WINDOW_TITLE "Plugin Installer"

class MainWindow;
class PluginInstallerComponent;

class PluginInstaller : public DocumentWindow
{
public:
    /** Creates and launches Plugin Installer window with its components */
    PluginInstaller (bool loadComponents = true);

    /** Destructor*/
    ~PluginInstaller();

    /** Called when the user hits the close button of the PluginInstaller. */
    void closeButtonPressed();

    /** A JUCE class that allows the PluginInstaller to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

    /** Access method to install a plugin directly without interacting with the Plugin Installer interface*/
    void installPluginAndDependency (const String& plugin, String version);

    /** Checks for plugin updates in the background and populates updatablePlugins array.
     *  Returns the number of plugins that have updates available. */
    static int checkForPluginUpdates();

private:
    WeakReference<PluginInstaller>::Master masterReference;
    friend class WeakReference<PluginInstaller>;

    void createXmlFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstaller);
};

/*
*   Struct for storing information about a plugin
*/

struct SelectedPluginInfo
{
    String pluginName;
    String displayName;
    String type;
    String developers;
    String latestVersion;
    String installedVersion;
    StringArray versions;
    String selectedVersion;
    String lastUpdated;
    String description;
    StringArray dependencies;
    StringArray dependencyVersions;
    String docURL;
    bool hasUpdate = false;
};

extern StringArray updatablePlugins;

/**
 *  Runs install and uninstall actions for the selected plugin.
*/
class PluginInstallActionRunner : public ThreadWithProgressWindow
{
public:
    using OperationCompleteHandler = std::function<void(const SelectedPluginInfo&, bool)>;

    enum ReturnCode
    {
        ZIP_NOTFOUND,
        SUCCESS,
        UNCMP_ERR,
        XML_MISSING,
        VER_EXISTS_ERR,
        XML_WRITE_ERR,
        LOAD_ERR,
        HTTP_ERR,
        RESTART_REQUIRED
    };

    PluginInstallActionRunner();

    /** Sets selected plugin info before running an action. */
    void setPluginInfo (const SelectedPluginInfo& p);

    /** Called when the user hits the 'Download' button for a selected plugin **/
    int downloadPlugin (const String& plugin, const String& version, bool isDependency);

    bool uninstallPlugin (const String& plugin);

    void setDownloadURL (const String& url);

    void setOperationCompleteHandler (OperationCompleteHandler handler);

    void installSelectedPlugin();

    bool uninstallSelectedPlugin();

private:
    String downloadURL;

    SelectedPluginInfo pInfo;

    int httpStatusCode = 0;
    bool restartRequired = false;

    void run() override;

    /** Shows the alert message on the message thread **/
    void showAlertOnMessageThread (MessageBoxIconType iconType, const String& title, const String& message);

    /** Updates the UI on the message thread **/
    void updateUIOnMessageThread();

    void notifyOperationComplete (bool isInstalled);

    OperationCompleteHandler operationCompleteHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallActionRunner);
};

/**
 *  Create a Table of all the plugins hosted on JFrong Artifactory
*/
class PluginListBoxComponent : public Component,
                               public TableListBoxModel
{
public:
    enum Columns
    {
        displayNameColumn = 1,
        typeColumn,
        developersColumn,
        updatedColumn,
        descriptionColumn,
        dependenciesColumn,
        installedVersionColumn,
        versionSelectorColumn,
        installColumn,
        uninstallColumn
    };

    PluginListBoxComponent();

    int getNumRows() override;

    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void cellClicked (int rowNumber, int columnId, const MouseEvent& event) override;

    Component* refreshComponentForCell (int rowNumber,
                                        int columnId,
                                        bool isRowSelected,
                                        Component* existingComponentToUpdate) override;

    String getCellTooltip (int rowNumber, int columnId) override;

    void sortOrderChanged (int newSortColumnId, bool isForwards) override;

    int getColumnAutoSizeWidth (int columnId) override;

    void resized() override;

    void setSearchText (const String& text);

    void setShowInstalledOnly (bool shouldShowInstalledOnly);

    void setTypeFilters (bool shouldShowSources,
                         bool shouldShowFilters,
                         bool shouldShowSinks,
                         bool shouldShowOther);

    bool refreshCatalog();

private:
    void applyFilters();

    bool matchesCurrentFilters (const SelectedPluginInfo& pluginInfo) const;

    bool isOtherType (const String& type) const;

    SelectedPluginInfo* getPluginForVisibleRow (int rowNumber);

    const SelectedPluginInfo* getPluginForVisibleRow (int rowNumber) const;

    void setSelectedVersion (int rowNumber, const String& version);

    void installPluginForRow (int rowNumber);

    void uninstallPluginForRow (int rowNumber);

    void updatePluginState (const SelectedPluginInfo& pluginInfo, bool isInstalled);

    std::unique_ptr<TableListBox> pluginTable;
    std::unique_ptr<PluginInstallActionRunner> actionRunner;
    std::vector<SelectedPluginInfo> allPlugins;
    std::vector<int> visibleRows;
    String searchText;
    FontOptions tableFont, nameFont;
    bool showInstalledOnly = false;
    bool showSources = true;
    bool showFilters = true;
    bool showSinks = true;
    bool showOther = true;

    std::unique_ptr<DropShadower> tableDropShadower;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListBoxComponent);
};

/**
 *  Create a Component for handling the plugins table and info panel
*/

class PluginInstallerComponent : public Component,
                                 public Button::Listener
{
public:
    PluginInstallerComponent();

    void paint (Graphics&) override;
    void resized() override;

    void buttonClicked (Button* button) override;

    void colourChanged() override;

private:
    std::unique_ptr<PluginListBoxComponent> pluginListAndInfo;

    std::unique_ptr<Label> searchLabel;
    std::unique_ptr<TextEditor> searchEditor;

    std::unique_ptr<Label> viewLabel;
    std::unique_ptr<ToggleButton> allButton, installedButton;
    std::unique_ptr<ShapeButton> updatesButton;

    std::unique_ptr<Label> typeLabel;
    std::unique_ptr<ToggleButton> filterType, sourceType, sinkType, otherType;

    FontOptions font;

    void applyTableFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallerComponent);
};

#endif
