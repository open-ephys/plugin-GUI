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
};

extern StringArray updatablePlugins;

/**
 *  Create Info Panel for the selected plugin from the table
*/
class PluginInfoComponent : public Component,
                            public Button::Listener,
                            public ComboBox::Listener,
                            public ThreadWithProgressWindow
{
public:
    PluginInfoComponent();

    void paint (Graphics&) override;
    void resized() override;

    /** Called when any of the buttons inside this component is clicked that has a listener **/
    void buttonClicked (Button* button) override;

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    /** Sets selected plugin's info obtained from bintray**/
    void setPluginInfo (const SelectedPluginInfo& p, bool shouldUpdateUI = true);

    /** Sets the status message with custom string and makes is visible/hidden **/
    void updateStatusMessage (const String& str, bool isVisible);

    /** Make the selected plugin's info visible **/
    void makeInfoVisible (bool isEnabled);

    /** Called when the user hits the 'Download' button for a selected plugin **/
    int downloadPlugin (const String& plugin, const String& version, bool isDependency);

    bool uninstallPlugin (const String& plugin);

    void setDownloadURL (const String& url);

private:
    int selectedPlugin;
    String downloadURL;
    FontOptions infoFont, infoFontBold;

    Label pluginNameLabel;
    Label pluginNameText;

    Label developersLabel;
    Label developersText;

    Label versionLabel;
    Label installedVerLabel;
    Label installedVerText;

    Label lastUpdatedLabel;
    Label lastUpdatedText;

    Label descriptionLabel;
    Label descriptionText;

    Label dependencyLabel;
    Label dependencyText;

    Label statusLabel;

    TextButton downloadButton;
    TextButton documentationButton;
    TextButton uninstallButton;

    ComboBox versionMenu;

    SelectedPluginInfo pInfo;

    enum RetunCode
    {
        ZIP_NOTFOUND,
        SUCCESS,
        UNCMP_ERR,
        XML_MISSING,
        VER_EXISTS_ERR,
        XML_WRITE_ERR,
        LOAD_ERR,
        HTTP_ERR
    };

    int httpStatusCode = 0;

    void run() override;

    /** Shows the alert message on the message thread **/
    void showAlertOnMessageThread (MessageBoxIconType iconType, const String& title, const String& message);

    /** Updates the UI on the message thread **/
    void updateUIOnMessageThread();

    DropShadower infoCompDropShadower { DropShadow (Colours::black.withAlpha (0.5f), 6, { 2, 2 }) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInfoComponent);
};

/**
 *  Create a Table of all the plugins hosted on JFrong Artifactory
*/
class PluginListBoxComponent : public Component,
                               public ListBoxModel,
                               public Thread
{
public:
    PluginListBoxComponent();

    /* Raw list of plugins available for download */
    StringArray pluginArray;
    HashMap<String, String> pluginLabels;
    HashMap<String, String> displayNames;
    HashMap<String, String> dependencyVersion;

    int getNumRows() override;

    void setNumRows (int num)
    {
        numRows = num;
        pluginList.updateContent();
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

    void resized() override;

    void returnKeyPressed (int lastRowSelected) override;

private:
    ListBox pluginList;
    FontOptions listFont;
    int numRows;
    int maxTextWidth = 0;

    var pluginData;

    String lastPluginSelected;
    Array<String> pluginVersion;

    SelectedPluginInfo selectedPluginInfo;

    PluginInfoComponent pluginInfoPanel;

    void run() override;

    // Loads selected plugin's info from bintray
    bool loadPluginInfo (const String& pluginName);

    void listBoxItemClicked (int row, const MouseEvent&) override;

    DropShadower listBoxDropShadower { DropShadow (Colours::black.withAlpha (0.5f), 6, { 2, 2 }) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListBoxComponent);
};

/**
 *  Create a Component for handling the plugins table and info panel
*/

class PluginInstallerComponent : public Component,
                                 public ComboBox::Listener,
                                 public Button::Listener,
                                 public ThreadWithProgressWindow
{
public:
    PluginInstallerComponent();

    void paint (Graphics&) override;
    void resized() override;

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    void buttonClicked (Button* button) override;

private:
    PluginListBoxComponent pluginListAndInfo;

    StringArray allPlugins;
    StringArray installedPlugins;

    bool checkForUpdates;

    Label sortingLabel;
    ComboBox sortByMenu;

    Label viewLabel;
    ToggleButton allButton, installedButton;
    TextButton updatesButton;

    Label typeLabel;
    ToggleButton filterType, sourceType, sinkType, otherType;

    FontOptions font;

    void run() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallerComponent);
};

#endif
