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

#ifndef __UICOMPONENT_H_D97C73CF__
#define __UICOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../Processors/MessageCenter/MessageCenterEditor.h"
#include "../Processors/Visualization/DataWindow.h"
#include "CustomArrowButton.h"
#include "DefaultConfig.h"
#include "LookAndFeel/CustomLookAndFeel.h"
#include "MessageCenterButton.h"
#include "MessageWindow.h"
#include "PluginInstaller.h"

class MainWindow;
class ProcessorList;
class ControlPanel;
class ShowHideEditorViewportButton;
class ProcessorGraph;
class AudioComponent;
class GraphViewer;
class InfoLabel;
class ConsoleViewer;
class DataViewport;
class EditorViewport;
class SignalChainTabComponent;
class DefaultConfigWindow;
class PopupManager;
class DataWindow;

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
                    public DragAndDropContainer,
                    public DataWindow::Listener

{
public:
    /** Constructor */
    UIComponent (MainWindow* mainWindow_,
                 ProcessorGraph* pgraph,
                 AudioComponent* audio,
                 ControlPanel* controlPanel,
                 ConsoleViewer* consoleViewer,
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
    void buttonClicked (Button* button);

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
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);

    /** Called when a particular menu item is selected. Doesn't do anything yet. */
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);

    /** Doesn't do anything yet. */
    ApplicationCommandTarget* getNextCommandTarget();

    /** Returns a list of commands the application can perform. */
    void getAllCommands (Array<CommandID>& commands);

    /** Returns the info, default keypress, and activation state of all the application's commands. */
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);

    /** Determines what takes place when a given command is executed by the user. */
    bool perform (const InvocationInfo& info);

    /** Save settings. */
    void saveStateToXml (XmlElement*);

    /** Load settings. */
    void loadStateFromXml (XmlElement*);

    /** Get current colour theme*/
    ColourTheme getTheme();

    /** Set current colour theme*/
    void setTheme (ColourTheme);

    /** Get list of recently used recording directories*/
    Array<String> getRecentlyUsedFilenames();

    /** Set list of recently used recording directories */
    void setRecentlyUsedFilenames (const Array<String>& filenames);

    // Adds the info tab to the DataViewport if it is not already open
    void addInfoTab();

    // Adds the graph tab to the DataViewport if it is not already open
    void addGraphTab();

    // Adds the console viewer to the DataViewport if it is not already open
    void addConsoleTab();

    // Opens the console viewer in a separate window
    void openConsoleWindow();

    /** Notifies the UI component when the graph viewer is closed */
    void closeGraphViewer() { graphViewerIsOpen = false; }

    /** Notifies the UI component when the info tab is closed */
    void closeInfoTab() { infoTabIsOpen = false; }

    /** Notifies the UI component when the console viewer tab is closed */
    void closeConsoleViewer() { consoleOpenInTab = false; }

    /** Returns true if the console viewer is open */
    bool isConsoleOpen () { return consoleOpenInTab || consoleOpenInWindow; }

    /** Notifies the UI component when the console viewer window is closed */
    void windowClosed (const String& windowName) override;

    /** Shows a message bubble above/below the specified child component */
    void showBubbleMessage (Component *component, const String& message);

    /** Finds a child component based on a unique component ID */
    Component* findComponentByIDRecursive (Component* parent, const String& id);

    /** Resizes all of components inside the UIComponent to fit the new boundaries
    of the MainWindow, or to account for opening/closing events.*/
    void resized();

    /** Paints the an overlay on the UIComponent to show busy state */
    void paintOverChildren (Graphics& g) override;

    /** Sets the busy state of the UIComponent */
    void setUIBusy (bool busy);

private:
    ScopedPointer<DataViewport> dataViewport;
    ScopedPointer<SignalChainTabComponent> signalChainTabComponent;
    ScopedPointer<ShowHideEditorViewportButton> showHideEditorViewportButton;
    ScopedPointer<ProcessorList> processorList;
    ScopedPointer<InfoLabel> infoLabel;
    ScopedPointer<GraphViewer> graphViewer;
    std::unique_ptr<ConsoleViewer> consoleViewer;
    std::unique_ptr<DataWindow> consoleWindow;

    bool infoTabIsOpen = false;
    bool graphViewerIsOpen = false;
    bool consoleOpenInTab = false;
    bool consoleOpenInWindow = false;

    EditorViewport* editorViewport;

    MessageCenterButton messageCenterButton;

    WeakReference<PluginInstaller> pluginInstaller;

    std::unique_ptr<DefaultConfigWindow> defaultConfigWindow;
    std::unique_ptr<MessageWindow> messageWindow;

    std::unique_ptr<PopupManager> popupManager;

    std::unique_ptr<BubbleMessageComponent> bubbleMsgComponent;

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

    /** Contains codes for common user commands to which the application must react.*/
    enum CommandIDs
    {
        openSignalChain = 0x2001,
        saveSignalChain = 0x2002,
        undo = 0x2003,
        redo = 0x2004,
        copySignalChain = 0x2005,
        pasteSignalChain = 0x2006,
        clearSignalChain = 0x2007,
        toggleProcessorList = 0x2008,
        toggleSignalChain = 0x2009,
        toggleFileInfo = 0x2010,
        toggleInfoTab = 0x2011,
        toggleGraphViewer = 0x2012,
        toggleConsoleViewer = 0x2013,
        showMessageWindow = 0x2014,
        setClockModeDefault = 0x2111,
        setClockModeHHMMSS = 0x2112,
        setClockReferenceTimeCumulative = 0x2113,
        setClockReferenceTimeAcqStart = 0x2114,
        toggleHttpServer = 0x4001,
        showHelp = 0x2211,
        checkForUpdates = 0x2222,
        resizeWindow = 0x2212,
        reloadOnStartup = 0x2213,
        saveSignalChainAs = 0x2214,
        openPluginInstaller = 0x2216,
        openDefaultConfigWindow = 0x2217,
        loadPluginSettings = 0x3001,
        savePluginSettings = 0x3002,
        lockSignalChain = 0x5001,
        setColourThemeLight = 0x6111,
        setColourThemeMedium = 0x6112,
        setColourThemeDark = 0x6113,
        setSoftwareRenderer = 0x7001,
        setDirect2DRenderer = 0x7002

    };

    File currentConfigFile;

    bool messageCenterIsCollapsed = true;

    CustomLookAndFeel* customLookAndFeel;

    bool isBusy = false;

    /** Set the rendering engine to use on Windows - Software (CPU) or Direct2D (GPU))*/
    void setRenderingEngine (int engineIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent);
};

/**

  A button used to show/hide the EditorViewport.

  @see UIComponent, EditorViewport

*/

class ShowHideEditorViewportButton : public ToggleButton,
                                     public Button::Listener
{
public:
    /** Constructor */
    ShowHideEditorViewportButton();

    /** Destructor */
    ~ShowHideEditorViewportButton() {}

    /** Draws the button. */
    void paint (Graphics& g) override;

    /** Listens for clicks on sub-component */
    void buttonClicked (Button* button);

private:
    std::unique_ptr<CustomArrowButton> arrow;

    FontOptions buttonFont;
};

#endif // __UICOMPONENT_H_D97C73CF__
