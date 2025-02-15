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

#ifndef __STREAMSELECTOR_H_BDCEE716__
#define __STREAMSELECTOR_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../UI/PopupComponent.h"
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Used to apply plugin settings to different streams.

  @see GenericEditor

*/

class DataStream;
class GenericEditor;
class TTLMonitor;
class SyncStartTimeMonitor;
class LastSyncEventMonitor;
class SyncAccuracyMonitor;
class DelayMonitor;
class UtilityButton;

class ExpanderButton;
class ExpandedTableComponent;

class StreamSelectorTable;

/**
*   TableListBoxModel for selecting streams
*/
class StreamTableModel : public TableListBoxModel
{
public:
    /** Constructor */
    StreamTableModel (StreamSelectorTable* owner);

    /** Destructor */
    ~StreamTableModel() {}

    /** Column types*/
    enum Columns
    {
        SELECTED = 1,
        PROCESSOR_ID,
        NAME,
        NUM_CHANNELS,
        SAMPLE_RATE,
        DELAY,
        TTL_LINE_STATES,
        ENABLED,
        START_TIME,
        LATEST_SYNC,
        SYNC_ACCURACY
    };

    /** Callback when a cell is clicked (not a sub-component) */
    void cellClicked (int rowNumber, int columnId, const MouseEvent& event) override;

    /** Called whenever a cell needs to be updated; creates custom components inside each cell*/
    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    /** Returns the number of rows in the table */
    int getNumRows() override;

    /** Updates the underlying StreamInfoView objects */
    void update (Array<const DataStream*> dataStreams);

    /** Determines row colours */
    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    /** Paints the columns*/
    void paintCell (Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    /** Returns the tooltip for a particular cell*/
    String getCellTooltip (int rowNumber, int columnId) override;

    /** Called whenever the list is scrolled; tells the editor to update the monitors*/
    void listWasScrolled() override;

    TableListBox* table;

private:
    Array<const DataStream*> streams;

    StreamSelectorTable* owner;

    bool isShuttingDown = false;

    bool acquisitionIsActive;
};

/**

Allows the user to browse through the available DataStreams within a given plugin

Lives inside the GenericEditor

*/
class PLUGIN_API StreamSelectorTable : public Component,
                                       public Button::Listener,
                                       public Timer,
                                       public ComponentListener
{
public:
    /** Constructor*/
    StreamSelectorTable (GenericEditor* editor);

    /** Destructor*/
    virtual ~StreamSelectorTable();

    /** Responds to button clicks */
    void buttonClicked (Button* button) override;

    /** Adds a new DataStream*/
    void add (const DataStream*);

    /** Removes a DataStream*/
    void remove (const DataStream*);

    /** Informs the GenericEditor about the component width*/
    int getDesiredWidth();

    /** Sets the location of the viewport and its sub-components*/
    void resized() override;

    /** Returns a pointer to the currently viewed stream*/
    const DataStream* getCurrentStream();

    /** Returns true if a given stream is enabled*/
    bool checkStream (const DataStream* stream);

    /** Renders the component*/
    void paint (Graphics& g) override;

    /** Returns a pointer to the SyncStartTimeMonitor for a given DataStream*/
    SyncStartTimeMonitor* getSyncStartTimeMonitor (const DataStream* stream);

    /** Returns a pointer to the LastSyncEventMonitor for a given DataStream*/
    LastSyncEventMonitor* getlastSyncEventMonitor (const DataStream* stream);

    /** Returns a pointer to the SyncAccuracyMonitor for a given DataStream*/
    SyncAccuracyMonitor* getSyncAccuracyMonitor (const DataStream* stream);

    /** Returns a pointer to the TTLMonitor for a given DataStream*/
    TTLMonitor* getTTLMonitor (const DataStream* stream);

    /** Returns a pointer to the DelayMonitor for a given DataStream*/
    DelayMonitor* getDelayMonitor (const DataStream* stream);

    /** Starts TTLMonitor and DelayMonitor animations*/
    void startAcquisition();

    /** Stops TTLMonitor and DelayMonitor animations*/
    void stopAcquisition();

    /** Used for scrolling animations*/
    void beginUpdate();

    /** Signals that all streams have been copied, and returns ID of selected stream*/
    uint16 finishedUpdate();

    /** Returns the total number of streams for this plugin*/
    int getNumStreams() { return streams.size(); }

    /** Returns the index of the stream that's currently in view*/
    int getViewedIndex();

    /** Sets the stream that's currently in view*/
    void setViewedIndex (int i);

    /** Used to enable and disable a given stream*/
    void setStreamEnabledState (uint16 streamId, bool isEnabled);

    /** Called when popup window is deleted */
    void componentBeingDeleted (Component& component) override;

    /** Pointer to editor */
    GenericEditor* editor;

    /** Index of the currently viewed stream */
    int viewedStreamIndex;

    /** True if this table belongs to a Record Node */
    bool isRecordNode = false;

private:
    /** Renders delay & TTL monitors */
    void timerCallback() override;

    /** Creates a new table view */
    TableListBox* createTableView (bool expanded = false);

    std::unique_ptr<StreamTableModel> tableModel;
    std::unique_ptr<TableListBox> streamTable;
    std::unique_ptr<ExpanderButton> expanderButton;

    ExpandedTableComponent* expandedTableComponent = nullptr;

    Array<const DataStream*> streams;

    std::map<uint16, bool> streamStates;

    int streamInfoViewWidth;
    int streamInfoViewHeight;

    int counter = 0;
};

/**
  Button to show expanded view of stream info table
*/
class ExpanderButton : public Button
{
public:
    /** Constructor*/
    ExpanderButton();

    /** Destructor*/
    ~ExpanderButton() {}

private:
    /** Renders the button*/
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    Path iconPath;
};

/**
  Popup Table component for expanded view of stream info table.
  Supports undo/redo actions.
*/
class ExpandedTableComponent : public PopupComponent
{
public:
    /** Constructor*/
    ExpandedTableComponent (TableListBox* table, Component* parent);

    /** Destructor*/
    ~ExpandedTableComponent() {}

    void updatePopup() override;

private:
    std::unique_ptr<TableListBox> expandedTable;
};

#endif // __STREAMSELECTOR_H_BDCEE716__
