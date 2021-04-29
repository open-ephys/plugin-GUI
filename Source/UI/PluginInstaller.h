#ifndef PLUGININSTALLER_H_INCLUDED
#define PLUGININSTALLER_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"

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

    void createXmlFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstaller);

};


/*
*   Struct for storing information of a plugin
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
    void buttonClicked(Button* button) override;

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

    /** Sets selected plugin's info obtained from bintray**/
    void setPluginInfo(const SelectedPluginInfo& p);

    /** Sets the status message with custom string and makes is visible/hidden **/
    void updateStatusMessage(const String& str, bool isVisible);

    /** Make the selected plugin's info visible **/
    void makeInfoVisible(bool isEnabled);

    /** Called when the user hits the 'Download' button for a selected plugin **/
    int downloadPlugin(const String& plugin, const String& version,
                       bool isDependency);
    
    void setDownloadURL(const String& url);

private:

    int selectedPlugin;
    String downloadURL;
    Font infoFont, infoFontBold;

    Label pluginNameLabel, pluginNameText;
    Label developersLabel, developersText;
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

    enum RetunCode {ZIP_NOTFOUND, SUCCESS, UNCMP_ERR, XML_MISSING, VER_EXISTS_ERR, XML_WRITE_ERR, LOAD_ERR, PLUGIN_IN_USE, RECNODE_IN_USE};

    void run() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInfoComponent);

};

/**
 *  Create a Table of all the plugins hosted on bintray
*/
class PluginListBoxComponent : public Component,
                               public ListBoxModel,
                               public Thread
                             //  public ThreadWithProgressWindow
{
public:

    PluginListBoxComponent();

    /* Raw list of plugins available for download */
    StringArray pluginArray;
    HashMap<String, String> pluginLabels;
    HashMap<String, String> displayNames;
    HashMap<String, String> dependencyVersion;

    int getNumRows() override;

    void setNumRows(int num) { numRows = num; pluginList.updateContent(); }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintListBoxItem (int rowNumber, Graphics &g, 
                           int width, int height, 
                           bool rowIsSelected) override;

    void resized() override;

    void returnKeyPressed (int lastRowSelected) override;
    
private:

    ListBox pluginList;
    Font listFont;
    int numRows;
    int maxTextWidth = 0;
    
    var pluginData;

    String lastPluginSelected; 
    Array<String> pluginVersion;

    SelectedPluginInfo selectedPluginInfo;

    PluginInfoComponent pluginInfoPanel;

    void run() override;

    // Loads selected plugin's info from bintray
    bool loadPluginInfo(const String& pluginName);

    void listBoxItemClicked (int row, const MouseEvent &) override;

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

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
    
    void buttonClicked(Button* button) override;
 
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

    Font font;

    void run() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallerComponent);

};

#endif
