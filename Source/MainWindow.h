/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __MAINWINDOW_H_BA75E17__
#define __MAINWINDOW_H_BA75E17__

#include "../JuceLibraryCode/JuceHeader.h"
#include "UI/UIComponent.h"
#include "UI/ControlPanel.h"
#include "Audio/AudioComponent.h"
#include "Processors/ProcessorGraph/ProcessorGraph.h"
#include "UI/DefaultConfig.h"
#include "UI/ControlPanel.h"
#include "Utils/OpenEphysHttpServer.h"

class OpenEphysHttpServer;

/**
    Custom DocumentWindow class
 */
class MainDocumentWindow : public DocumentWindow
{
public:
    
    /** Constructor */
    MainDocumentWindow();
    
    /** Destructor */
    virtual ~MainDocumentWindow() { }
    
    /** Called when the user hits the close button of the MainWindow. This destroys
        the MainWindow and closes the application. */
    void closeButtonPressed();
};

/**
  The main window for the GUI application.

  This object creates and destroys the AudioComponent, the ProcessorGraph,
  and the UIComponent (which exists as the ContentComponent of this window).

  @see AudioComponent, ProcessorGraph, UIComponent

*/

class MainWindow
{
public:

    /** Initializes the MainWindow, creates the AudioComponent, ProcessorGraph,
        and UIComponent, and sets the window boundaries. */
    MainWindow(const File& fileToLoad = File(), bool isConsoleApp = false);

    /** Destroys the AudioComponent, ProcessorGraph, and UIComponent, and saves the window boundaries. */
    ~MainWindow();

    /** A JUCE class that allows the MainWindow to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

    /** Determines whether the last used configuration reloads upon startup. */
    bool shouldReloadOnStartup;

    /** Determines whether the ProcessorGraph http server is enabled. */
    bool shouldEnableHttpServer;

    /** Determines whether the default config selection window needs to open on startup. */
    bool openDefaultConfigWindow;

    /** Determines whether the Auto Updater needs to run on startup. */
    bool automaticVersionChecking;

    /** Ends the process() callbacks and disables all processors.*/
	void shutDownGUI();

    /** Called when the GUI crashes unexpectedly.*/
    static void handleCrash(void *);

    /** Start thread which listens to remote commands to control the GUI */
    void enableHttpServer();

    /** Stop thread which listens to remote commands to control the GUI */
    void disableHttpServer();
    
    /** Sets the size of the Main Window */
    void centreWithSize(int, int);

private:
    
    /** Saves the processor graph to a file*/
    void saveProcessorGraph(const File& file);

    /** Loads  the processor graph from a file*/
    void loadProcessorGraph(const File& file);

    /** Saves the MainWindow's boundaries into the file "windowState.xml", located in the directory
        from which the GUI is run. */
    void saveWindowBounds();

    /** Loads the MainWindow's boundaries into the file "windowState.xml", located in the directory
        from which the GUI is run. */
    void loadWindowBounds();

    /** Checks whether the signal chains of both the config files (lastConfig.xml & recoveryConfig.xml) 
     *  match or not. */
    bool compareConfigFiles(File file1, File file2);

    /** API respective configs directory */
    File configsDir;

    /** A pointer to the DocumentWindow (only instantiated if running in GUI mode). */
    std::unique_ptr<MainDocumentWindow> documentWindow;
    
    /** A pointer to the application's AudioComponent (owned by the MainWindow). */
    std::unique_ptr<AudioComponent> audioComponent;

    /** A pointer to the application's ProcessorGraph (owned by the MainWindow). */
    std::unique_ptr<ProcessorGraph> processorGraph;
    
    /** A pointer to the application's ControlPanel (owned by the MainWindow). */
    std::unique_ptr<ControlPanel> controlPanel;
    
    /** A weak reference to default config window. */
    std::unique_ptr<DefaultConfigWindow> defaultConfigWindow;

    /** A pointer to the application's HttpServer (owned by the MainWindow). */
    std::unique_ptr<OpenEphysHttpServer> http_server_thread;
    
    /** Set to true if the application is running in console mode */
    bool isConsoleApp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)

};


#endif  // __MAINWINDOW_H_BA75E17__
