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

#ifndef __UICOMPONENT_H_D97C73CF__
#define __UICOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "PluginInstaller.h"
#include "MessageCenterButton.h"
#include "../Processors/MessageCenter/MessageCenterEditor.h"
#include "DefaultConfig.h"
#include "LookAndFeel/CustomLookAndFeel.h"

class MainWindow;
class ProcessorList;
class ControlPanel;
class EditorViewportButton;
class ProcessorGraph;
class AudioComponent;
class GraphViewer;
class InfoLabel;
class DataViewport;
class EditorViewport;
class SignalChainTabComponent;
class DefaultConfigWindow;
class PopupManager;

/**

  Creates objects for user interaction.

  The UIComponent is responsible for the layout of the user interface and
  for creating the application's menu bar.

  @see ControlPanel, ProcessorList, EditorViewport, DataViewport,
       MessageCenter

*/

class UIComponent : public Component,
    public ActionBroadcaster,
    public MenuBarModel,
    public ApplicationCommandTarget,
    public Button::Listener,
    public DragAndDropContainer // required for
// drag-and-drop
// internal components


{
public:

    /** Constructor */
    UIComponent(MainWindow* mainWindow_,
                ProcessorGraph* pgraph,
                AudioComponent* audio,
                ControlPanel* controlPanel,
                CustomLookAndFeel* customLookAndFeel);

    /** Destructor */
    ~UIComponent();

    /** Returns a pointer to the EditorViewport. */
	EditorViewport* getEditorViewport();

    /** Returns a pointer to the ProcessorList. */
	ProcessorList* getProcessorList();

    /** Returns a pointer to the DataViewport. */
	DataViewport* getDataViewport();

    /** Returns a pointer to the ProcessorGraph. */
	ProcessorGraph* getProcessorGraph();

    /** Returns a pointer to the GraphViewer. */
	GraphViewer* getGraphViewer();

    /** Returns a pointer to the ControlPanel. */
	ControlPanel* getControlPanel();

    /** Returns a pointer to the UIComponent. */
	UIComponent* getUIComponent();

    /** Returns a pointer to the AudioComponent. */
	AudioComponent* getAudioComponent();

    /** Returns a pointer to the Plugin Installer (UI) */
    PluginInstaller* getPluginInstaller();

    /** Returns a pointer to the Popup Manager */
    PopupManager* getPopupManager();
    
    /** Called by the MessageCenterButton */
    void buttonClicked(Button* button);

    /** Stops the callbacks to the ProcessorGraph which drive data acquisition. */
    void disableCallbacks();

    /** Disables the connection between the DataViewport and the EditorViewport. */
    void disableDataViewport();

    /**
    Called whenever a major change takes place within a child component, in order
    to make sure the UIComponent's other children get resized appropriately. */
    void childComponentChanged();

    /** Returns the names of all the requested menubar drop-down lists (e.g., "File", "Edit", "Help", etc.). */
    StringArray getMenuBarNames();

    /** Adds the commands contained within a given drop-down menu from the menubar. */
    PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName);

    /** Called when a particular menu item is selected. Doesn't do anything yet. */
    void menuItemSelected(int menuItemID, int topLevelMenuIndex);

    /** Doesn't do anything yet. */
    ApplicationCommandTarget* getNextCommandTarget();

    /** Returns a list of commands the application can perform. */
    void getAllCommands(Array <CommandID>& commands);

    /** Returns the info, default keypress, and activation state of all the application's commands. */
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result);

    /** Determines what takes place when a given command is executed by the user. */
    bool perform(const InvocationInfo& info);

    /** Save settings. */
    void saveStateToXml(XmlElement*);

    /** Load settings. */
    void loadStateFromXml(XmlElement*);
    
    /** Get current color theme*/
    ColorTheme getTheme();
    
    /** Set current color theme*/
    void setTheme(ColorTheme);

    /** Get list of recently used recording directories*/
    Array<String> getRecentlyUsedFilenames();

    /** Set list of recently used recording directories */
    void setRecentlyUsedFilenames(const Array<String>& filenames);

    // Adds the info tab to the DataViewport if it is not already open
    void addInfoTab();

    // Adds the graph tab to the DataViewport if it is not already open
    void addGraphTab();
    
    /** Notifies the UI component when the graph viewer is closed */
    void closeGraphViewer() { graphViewerIsOpen = false; }
    
    /** Notifies the UI component when the info tab is closed */
    void closeInfoTab() { infoTabIsOpen = false; }
    
    /** Finds a child component based on a unique component ID */
    Component* findComponentByIDRecursive(Component* parent, const String& id);
	
private:

    ScopedPointer<DataViewport> dataViewport;
    ScopedPointer<SignalChainTabComponent> signalChainTabComponent;
    ScopedPointer<EditorViewportButton> editorViewportButton;
    ScopedPointer<ProcessorList> processorList;
    ScopedPointer<InfoLabel> infoLabel;
    ScopedPointer<GraphViewer> graphViewer;
    
    bool infoTabIsOpen = false;
    bool graphViewerIsOpen = false;
    
    EditorViewport* editorViewport;
    
    MessageCenterButton messageCenterButton;
    
    WeakReference<PluginInstaller> pluginInstaller;

    std::unique_ptr<DefaultConfigWindow> defaultConfigWindow;

    std::unique_ptr<PopupManager> popupManager;

    Viewport processorListViewport;

    /** Pointer to the GUI's MainWindow, which owns the UIComponent. */
    MainWindow* mainWindow;

    /** Allows the application to use tooltips, which are messages
    that appear when the mouse hovers over particular components. */
    TooltipWindow tooltipWindow;

    /** Pointer to the GUI's ProcessorGraph. Owned by the MainWindow. */
    ProcessorGraph* processorGraph;

    /** Pointer to the GUI's AudioComponent. Owned by the MainWindow. */
    AudioComponent* audio;
    
    /** Pointer to the GUI's ControlPanel. Owned by the MainWindow. */
    ControlPanel* controlPanel;

    /** Pointer to the GUI's MessageCenterEditor. Owned by the MessageCenter. */
    MessageCenterEditor* messageCenterEditor;
    
    /** Resizes all of components inside the UIComponent to fit the new boundaries
    of the MainWindow, or to account for opening/closing events.*/
    void resized();

    /** Contains codes for common user commands to which the application must react.*/
    enum CommandIDs
    {
        openSignalChain 		= 0x2001,
        saveSignalChain 		= 0x2002,
        undo					= 0x2003,
        redo 					= 0x2004,
        copySignalChain			= 0x2005,
        pasteSignalChain		= 0x2006,
        clearSignalChain		= 0x2007,
        toggleProcessorList 	= 0x2008,
        toggleSignalChain	    = 0x2009,
        toggleFileInfo			= 0x2010,
        toggleInfoTab           = 0x2011,
        toggleGraphViewer       = 0x2012,
        setClockModeDefault     = 0x2111,
		setClockModeHHMMSS      = 0x2112,
        toggleHttpServer        = 0x4001,
        showHelp				= 0x2211,
        checkForUpdates         = 0x2222,
        resizeWindow            = 0x2212,
        reloadOnStartup         = 0x2213,
        saveSignalChainAs       = 0x2214,
        openPluginInstaller     = 0x2216,
        openDefaultConfigWindow = 0x2217,
        loadPluginSettings      = 0x3001,
        savePluginSettings      = 0x3002,
        lockSignalChain         = 0x5001,
        setColorThemeLight      = 0x6111,
        setColorThemeMedium    = 0x6112,
        setColorThemeDark       = 0x6113,
        setSoftwareRenderer     = 0x7001,
        setDirect2DRenderer     = 0x7002
        
    };

    File currentConfigFile;
    
    bool messageCenterIsCollapsed = true;
    
    ColorTheme theme = MEDIUM;
    
    CustomLookAndFeel* customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIComponent);

};

/**

  A button used to show/hide the EditorViewport.

  @see UIComponent, EditorViewport

*/

class EditorViewportButton : public Component
{
public:
    EditorViewportButton(UIComponent* ui);
    ~EditorViewportButton();

    /** Returns the open/closed state of the button. */
    bool isOpen()
    {
        return open;
    }

    /** Draws the button. */
    void paint(Graphics& g);

    /** Switches the open/closed state of the button. */
    void toggleState();

    /** Called when a mouse click begins inside the boundaries of the button. Used
    to toggle the button's open/closed state. */
    void mouseDown(const MouseEvent& e);

private:

    UIComponent* UI;
    bool open;

    Font buttonFont;

};

#endif  // __UICOMPONENT_H_D97C73CF__
