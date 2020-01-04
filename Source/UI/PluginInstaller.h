
#include "../../JuceLibraryCode/JuceHeader.h"
#include "RestRequest.h"

#define WINDOW_TITLE "Plugin Installer" 

class MainWindow;

class PluginInstaller : public DocumentWindow
{
public:
    /** Initializes the MainWindow, creates the AudioComponent, ProcessorGraph,
        and UIComponent, and sets the window boundaries. */
    PluginInstaller(MainWindow* mainWindow);

    /** Destroys the AudioComponent, ProcessorGraph, and UIComponent, and saves the window boundaries. */
    ~PluginInstaller();

    /** Called when the user hits the 'Download' button for a selected plugin **/
    bool pluginSelected();

    /** Called when the user hits the close button of the MainWindow. This destroys
        the MainWindow and closes the application. */
    void closeButtonPressed();

    /** A JUCE class that allows the PluginManager to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

    RestRequest request;

private:

    /* Pointer to the main window so we can keep in bounds */
    DocumentWindow* parent;

    /* Raw list of plugins available for download */
    Array<String> plugins;

    /*Add UI Elements here */

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginInstaller)

};