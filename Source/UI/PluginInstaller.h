#ifndef PLUGININSTALLER_H_INCLUDED
#define PLUGININSTALLER_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"
#include "LookAndFeel/MaterialButtonLookAndFeel.h"

#define WINDOW_TITLE "Plugin Installer" 

class MainWindow;
class PluginInstallerComponent;

class PluginInstaller : public DocumentWindow
{
public:
    /** Initializes the MainWindow, creates the AudioComponent, ProcessorGraph,
        and UIComponent, and sets the window boundaries. */
    PluginInstaller(MainWindow* mainWindow);

    /** Destroys the AudioComponent, ProcessorGraph, and UIComponent, and saves the window boundaries. */
    ~PluginInstaller();

    /** Called when the user hits the close button of the MainWindow. This destroys
        the MainWindow and closes the application. */
    void closeButtonPressed();

    /** A JUCE class that allows the PluginManager to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

private:

    /* Pointer to the main window so we can keep in bounds */
    DocumentWindow* parent;

    WeakReference<PluginInstaller>::Master masterReference;
    friend class WeakReference<PluginInstaller>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstaller);

};


/*
*   Struct for storing information of a plugin
*/

struct SelectedPluginInfo
{
    String pluginName;
    String packageName;
    String owner;
    String latestVersion;
    StringArray versions;
    String selectedVersion;
    String lastUpdated;
    String description;
    String dependencies;
    String docURL;
};

/**
 *  Create Info Panel for the selected plugin from the table
*/
class PluginInfoComponent : public Component,
                            public Button::Listener,
                            public ComboBox::Listener
{
public:
    PluginInfoComponent();

    void paint (Graphics&) override;
    void resized() override;

    String getSelectedPlugin() { return pInfo.pluginName; }

    void buttonClicked(Button* button) override;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    void setPluginInfo(const SelectedPluginInfo& p);

    void updateStatusMessage(const String& str, bool isVisible);

    void makeInfoVisible(bool isEnabled);

    /** Called when the user hits the 'Download' button for a selected plugin **/
    bool downloadPlugin(const String& plugin, const String& package, 
                        const String& version);

private:

    int selectedPlugin;

    Label pluginNameLabel, pluginNameText;
    Label ownerLabel, ownerText;
    Label versionLabel;
    Label lastUpdatedLabel, lastUpdatedText;
    Label descriptionLabel;
    Label descriptionText;
    Label dependencyLabel, dependencyText;
    Label statusLabel;

    TextButton downloadButton;
    TextButton documentationButton;

    ComboBox versionMenu;

    SelectedPluginInfo pInfo;

    MaterialButtonLookAndFeel mb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInfoComponent);

};

/**
 *  Create a Table of all the plugins hosted on bintray
*/
class PluginListBoxComponent : public Component,
                               public ListBoxModel
{
public:

    PluginListBoxComponent();

    /* Raw list of plugins available for download */
    StringArray pluginArray;

    int getNumRows() override;

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintListBoxItem (int rowNumber, Graphics &g, 
                           int width, int height, 
                           bool rowIsSelected) override;

    void resized() override;

private:

    ListBox pluginList;
    Font listFont;
    int numRows;

    var pluginData; 
    Array<String> pluginVersion;

    SelectedPluginInfo selectedPluginInfo;

    PluginInfoComponent pluginInfoPanel;

    void loadPluginNames();

    bool loadPluginInfo(const String& pluginName);

    void listBoxItemClicked (int row, const MouseEvent &) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListBoxComponent);

};


/**
 *  Create a Component for handling the plugins table and info panel
*/

class PluginInstallerComponent : public Component,
                                 public ComboBox::Listener
{
public:

    PluginInstallerComponent();

    void paint (Graphics&) override;
    void resized() override;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
 
private:

    PluginListBoxComponent pluginListAndInfo;

    Label sortingLabel;
    ComboBox sortByMenu;

    Font font;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallerComponent);

};

#endif