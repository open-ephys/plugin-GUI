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
#include "TimestampSourceSelection.h"
#include "PluginInstaller.h"


class MainWindow;
class ProcessorList;
class ControlPanel;
class EditorViewportButton;
class PluginManager;
class ProcessorGraph;
class AudioComponent;
class GraphViewer;
class MessageCenterEditor;
class InfoLabel;
class DataViewport;
class EditorViewport;
class TimestampSourceSelectionWindow;

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
    public DragAndDropContainer // required for
// drag-and-drop
// internal components


{
public:
    UIComponent(MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio);
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

    /** Returns a pointer to the MessageCenterEditor. */
	MessageCenterEditor* getMessageCenter();

    /** Returns a pointer to the UIComponent. */
	UIComponent* getUIComponent();

    /** Returns a pointer to the AudioComponent. */
	AudioComponent* getAudioComponent();

	PluginManager* getPluginManager();

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

    StringArray getRecentlyUsedFilenames();

    void setRecentlyUsedFilenames(const StringArray& filenames);
	
private:

    ScopedPointer<DataViewport> dataViewport;
    ScopedPointer<EditorViewport> editorViewport;
    ScopedPointer<EditorViewportButton> editorViewportButton;
    ScopedPointer<ProcessorList> processorList;
    ScopedPointer<ControlPanel> controlPanel;
    MessageCenterEditor* messageCenterEditor; // owned by ProcessorGraph
    ScopedPointer<InfoLabel> infoLabel;
    ScopedPointer<GraphViewer> graphViewer;
	ScopedPointer<PluginManager> pluginManager;

	WeakReference<TimestampSourceSelectionWindow> timestampWindow;

    WeakReference<PluginInstaller> pluginInstaller;

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

    /** Resizes all of components inside the UIComponent to fit the new boundaries
    of the MainWindow, or to account for opening/closing events.*/
    void resized();

    /** Contains codes for common user commands to which the application must react.*/
    enum CommandIDs
    {
        openConfiguration 		= 0x2001,
        saveConfiguration		= 0x2002,
        undo					= 0x2003,
        redo 					= 0x2004,
        copySignalChain			= 0x2005,
        pasteSignalChain		= 0x2006,
        clearSignalChain		= 0x2007,
        toggleProcessorList 	= 0x2008,
        toggleSignalChain	    = 0x2009,
        toggleFileInfo			= 0x2010,
        showHelp				= 0x2011,
        resizeWindow            = 0x2012,
        reloadOnStartup         = 0x2013,
        saveConfigurationAs     = 0x2014,
		openTimestampSelectionWindow = 0x2015,
        openPluginInstaller     = 0x2016
    };

    File currentConfigFile;

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
