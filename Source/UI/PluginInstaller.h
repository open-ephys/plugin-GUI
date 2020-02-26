#ifndef PLUGININSTALLER_H_INCLUDED
#define PLUGININSTALLER_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"
#include "RestRequest.h"
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

    /** Called when the user hits the 'Download' button for a selected plugin **/
    static bool pluginSelected(const String& plugin, const String& package, const String& version);

    /** Called when the user hits the close button of the MainWindow. This destroys
        the MainWindow and closes the application. */
    void closeButtonPressed();

    /** A JUCE class that allows the PluginManager to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

    /* Raw list of plugins available for download */
    static StringArray plugins;

private:

    /* Pointer to the main window so we can keep in bounds */
    DocumentWindow* parent;

    /*Add UI Elements here */

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

    void buttonClicked (Button* button) override
    {
        if(button == &downloadButton)
        {
            PluginInstaller::pluginSelected(pInfo.pluginName, pInfo.packageName, pInfo.selectedVersion);
        }
        else if (button == &documentationButton)
        {
            URL url = URL(pInfo.docURL);
            url.launchInDefaultBrowser();
        }
    }

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &versionMenu)
        {
            pInfo.selectedVersion = comboBoxThatHasChanged->getText();
            downloadButton.setEnabled(true);
        }
    }

    void setPluginInfo(const SelectedPluginInfo& p) 
    {
        pInfo = p;
        pluginNameLabel.setText("Name: " + pInfo.pluginName, dontSendNotification);
        ownerLabel.setText("Author: " + pInfo.owner, dontSendNotification);
        versionLabel.setText("Version: ", dontSendNotification);
        lastUpdatedLabel.setText("Last Updated: " + pInfo.lastUpdated, dontSendNotification);
        descriptionLabel.setText("Description: " + pInfo.description, dontSendNotification);
        dependencyLabel.setText(pInfo.dependencies, dontSendNotification);

        versionMenu.clear(dontSendNotification);

        for (int i = 0; i < pInfo.versions.size(); i++)
            versionMenu.addItem(pInfo.versions[i], i + 1);
        
        downloadButton.setEnabled(false);
    }

    void updateStatusMessage(const String& str, bool isVisible)
    {
        statusLabel.setText(str, dontSendNotification);
        statusLabel.setVisible(isVisible);
    }

    void makeInfoVisible(bool isEnabled) 
    {
        pluginNameLabel.setVisible(isEnabled);
        ownerLabel.setVisible(isEnabled);
        versionLabel.setVisible(isEnabled);
        versionMenu.setVisible(isEnabled);
        lastUpdatedLabel.setVisible(isEnabled);
        descriptionLabel.setVisible(isEnabled);
        dependencyLabel.setVisible(isEnabled);
        downloadButton.setVisible(isEnabled);
        documentationButton.setVisible(isEnabled);
    }

private:

    int selectedPlugin;
    Font infoFont;

    Label pluginNameLabel;
    Label ownerLabel;
    Label versionLabel;
    Label lastUpdatedLabel;
    Label descriptionLabel;
    Label dependencyLabel;
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

    RestRequest request;
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

    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged->getSelectedId() == 1)
        {
            PluginInstaller::plugins.sort(true);
            pluginListAndInfo.repaint();
        }
        else if (comboBoxThatHasChanged->getSelectedId() == 2)
        {
            PluginInstaller::plugins.sort(true);
            int size = PluginInstaller::plugins.size();
            for (int i = 0; i < size / 2; i++)
            {
                PluginInstaller::plugins.getReference(i).swapWith
                (PluginInstaller::plugins.getReference(size - i - 1));
            }

            pluginListAndInfo.repaint();
        }
    }
 
private:

    PluginListBoxComponent pluginListAndInfo;

    Label sortingLabel;
    ComboBox sortByMenu;

    Font font;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInstallerComponent);

};

#endif