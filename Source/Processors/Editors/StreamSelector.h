/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Used to apply plugin settings to different streams.

  @see GenericEditor

*/

class DataStream;
class GenericEditor;
class TTLMonitor;
class DelayMonitor;
class UtilityButton;

class ExpanderButton;
class ShadowGradient;

class StreamScrollButton;
class StreamNameButton;
class StreamEnableButton;

class StreamSelectorTable;

/**
 
 Displays information about a particular stream.
 
 Lives inside the StreamSelector component.
 
 */

class PLUGIN_API StreamInfoView : public Component,
    public Button::Listener
{
public:
    
    /** Constructor */
    StreamInfoView(const DataStream* stream, GenericEditor* editor, bool isEnabled);
    
    /** Destructor*/
    ~StreamInfoView() { }

    /** Returns the streamId associated with this view*/
    uint16 getStreamId() const;

    /** Returns the stream name associated with this view*/
    String getStreamName() const;

    /** Returns the stream source node ID associated with this view*/
    int getStreamSourceNodeId() const;

    /** Returns a pointer to the original DataStream*/
    const DataStream* getStream() { return stream; }

    /** Returns true if this stream is enabled*/
    bool getEnabledState() const;

    /** Enables or disables this stream*/
    void setEnabled(bool state);

    /** Returns a pointer to the TTLMonitor for this view*/
    TTLMonitor* getTTLMonitor() { return ttlMonitor; }
    
    /** Returns a pointer to the DelayMonitor for this view*/
    DelayMonitor* getDelayMonitor() { return delayMonitor; }

    /** Sets the DelayMonitor for this view*/
    void setDelayMonitor(DelayMonitor* delayMonitor);

    /** Returns a pointer to the TTL Monitor for this view*/
    void setTTLMonitor(TTLMonitor* ttlMonitor);

    /** Renders the StreamInfoView*/
    void paint(Graphics& g);
    
    /** Sets the location of the sub-components*/
    void resized();

    /** Called before new streams are added*/
    void beginUpdate();
    
    /** Updates the DataStream object referenced by this view */
    void update(const DataStream* stream);
    
    /** Updates the info displayed for the underlying stream*/
    void updateInfoString();

    /** Starts animations on TTLMonitor and DelayMonitor */
    void startAcquisition();

    /** Stops animations on TTLMonitor and DelayMonitor */
    void stopAcquisition();

    /** Called when enable button is clicked */
    void buttonClicked(Button* button);

    /** Set to 'true' after update, if stream still exists*/
    bool streamIsStillNeeded;

    uint16 streamId;

private:

    const DataStream* stream;

    bool isEnabled;

    String infoString;
    String enabledString;

    GenericEditor* editor;

    String streamName;
    int sourceNodeId;

    bool acquisitionIsActive;

    std::unique_ptr<StreamEnableButton> enableButton;
    TTLMonitor* ttlMonitor;
    DelayMonitor* delayMonitor;
};

/**

Allows the user to browse through the available DataStreams within a given plugin

Lives inside the GenericEditor

*/
class PLUGIN_API StreamSelector : public Component,
    public Timer,
    public Button::Listener
{
public:
    
    /** Constructor*/
    StreamSelector(GenericEditor* editor);
    
    /** Destructor*/
    ~StreamSelector() { }

    /** Adds a new DataStream*/
    void add(const DataStream*);
    
    /** Removes a DataStream*/
    void remove(StreamInfoView*);

    /** Informs the GenericEditor about the component width*/
    int getDesiredWidth() { return 140; }

    /** Sets the location of the viewport and its sub-components*/
    void resized();
    
    /** Returns a pointer to the currently viewed stream*/
    const DataStream* getCurrentStream();

    /** Returns true if a given stream is enabled*/
    bool checkStream(const DataStream* stream);

    /** Renders the component*/
    void paint(Graphics& g);

    /** Called when stream browser buttons are clicked*/
    void buttonClicked(Button* button);

    /** Returns a pointer to the TTLMonitor for a given DataStream*/
    TTLMonitor* getTTLMonitor(const DataStream* stream);

    /** Returns a pointer to the DelayMonitor for a given DataStream*/
    DelayMonitor* getDelayMonitor(const DataStream* stream);

    /** Returns a pointer to the StreamInfoView for a given DataStream*/
    StreamInfoView* getStreamInfoView(const DataStream* stream);

    /** Starts TTLMonitor and DelayMonitor animations*/
    void startAcquisition();

    /** Stops TTLMonitor and DelayMonitor animations*/
    void stopAcquisition();

    /** Begins TTLMonitor and DelayMonitor animations*/
    void timerCallback();

    /** Used for scrolling animations*/
    void beginUpdate();
    
    /** Signals that all streams have been copied, and returns ID of selected stream*/
    uint16 finishedUpdate();

    /** Returns the total number of streams for this plugin*/
    int getNumStreams() { return streams.size(); }
    
    /** Returns the index of the stream that's currently in view*/
    int getViewedIndex();
    
    /** Sets the stream that's currently in view*/
    void setViewedIndex(int i);

    /** Used to enable and disable a given stream*/
    void setStreamEnabledState(uint16 streamId, bool isEnabled);

private:

    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<Component> viewedComponent;

    std::unique_ptr<StreamScrollButton> leftScrollButton;
    std::unique_ptr<StreamScrollButton> rightScrollButton;
    std::unique_ptr<StreamNameButton> streamSelectorButton;

    OwnedArray<StreamInfoView> streams;

    std::map<uint16, bool> streamStates;

    int streamInfoViewWidth;
    int streamInfoViewHeight;

    SmoothedValue<float, ValueSmoothingTypes::Linear> scrollOffset;

    int viewedStreamIndex;

    GenericEditor* editor;

};


/**
*   TableListBoxModel for selecting streams
*/
class StreamTableModel : public TableListBoxModel
{

public:

    /** Constructor */
    StreamTableModel(StreamSelectorTable* owner);

    /** Destructor */
    ~StreamTableModel() { }

    /** Column types*/
    enum Columns {
        SELECTED = 1,
        PROCESSOR_ID,
        NAME,
        NUM_CHANNELS,
        SAMPLE_RATE,
        DELAY,
        TTL_LINE_STATES,
        ENABLED
    };

    /** Callback when a cell is clicked (not a sub-component) */
    void cellClicked(int rowNumber, int columnId, const MouseEvent& event) override;

    /** Called whenever a cell needs to be updated; creates custom components inside each cell*/
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
        Component* existingComponentToUpdate) override;

    /** Returns the number of rows in the table */
    int getNumRows() override;

    /** Updates the underlying StreamInfoView objects */
    void update(Array<StreamInfoView*> dataStreams, int viewedStreamIndex);

    /** Determines row colors */
    void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;

    /** Paints the columns*/
    void paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    
    TableListBox* table;
private:

    Array<StreamInfoView*> streams;

    StreamSelectorTable* owner;

    int viewedStreamIndex = 0;
    bool isShuttingDown = false;

    bool acquisitionIsActive;

};


class CustomTableLookAndFeel : public LookAndFeel_V4
{
public:
    CustomTableLookAndFeel();
    
    void drawCallOutBoxBackground(CallOutBox& box, Graphics& g,
        const Path& path, Image& cachedImage) override { }
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
    StreamSelectorTable(GenericEditor* editor);

    /** Destructor*/
    virtual ~StreamSelectorTable();

    /** Responds to button clicks */
	void buttonClicked(Button* button) override;

    /** Adds a new DataStream*/
    void add(const DataStream*);

    /** Removes a DataStream*/
    void remove(StreamInfoView*);

    /** Informs the GenericEditor about the component width*/
    int getDesiredWidth();

    /** Sets the location of the viewport and its sub-components*/
    void resized();

    /** Returns a pointer to the currently viewed stream*/
    const DataStream* getCurrentStream();

    /** Returns true if a given stream is enabled*/
    bool checkStream(const DataStream* stream);

    /** Renders the component*/
    void paint(Graphics& g);

    /** Returns a pointer to the StreamInfoView for a given DataStream*/
    StreamInfoView* getStreamInfoView(const DataStream* stream);

    /** Returns a pointer to the TTLMonitor for a given DataStream*/
    TTLMonitor* getTTLMonitor(const DataStream* stream);

    /** Returns a pointer to the DelayMonitor for a given DataStream*/
    DelayMonitor* getDelayMonitor(const DataStream* stream);

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
    void setViewedIndex(int i);

    /** Used to enable and disable a given stream*/
    void setStreamEnabledState(uint16 streamId, bool isEnabled);

    /** Called when popup window is deleted */
    void componentBeingDeleted(Component& component) override;

    /** Pointer to editor */
    GenericEditor* editor;

    /** Index of the currently viewed stream */
    int viewedStreamIndex;

private:

    /** Renders delay & TTL monitors */
    void timerCallback();

    /** Creates a new table view */
    TableListBox* createTableView(bool expanded = false);

    std::unique_ptr<StreamTableModel> tableModel;
    std::unique_ptr<TableListBox> streamTable;
	std::unique_ptr<ExpanderButton> expanderButton;
    std::unique_ptr<ShadowGradient> shadowGradient;
    std::unique_ptr<CustomTableLookAndFeel> customTableLookAndFeel;

    OwnedArray<StreamInfoView> streams;

    std::map<uint16, bool> streamStates;

    int streamInfoViewWidth;
    int streamInfoViewHeight;

    int counter = 0;

};

/**
  Draws a gradient shadow over the table
*/
class ShadowGradient : public Component
{
public:

    /** Constructor*/
    ShadowGradient() { }

private:

    /** Renders the shadow*/
    void paint(Graphics& g);
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
    ~ExpanderButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:

    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;

    Path iconPath;
};

/**
  Arrow buttons used to select different streams
*/
class StreamScrollButton : public Button
{
public:
    
    /** Constructor*/
    StreamScrollButton(const String& name);
    
    /** Destructor*/
    ~StreamScrollButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};


/**
  Displays the name of the current stream
*/
class StreamNameButton : public Button
{
public:
    
    /** Constructor*/
    StreamNameButton(const String& name);
    
    /** Destructor*/
    ~StreamNameButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_;  }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

/**
  Displays the name of the current stream
*/
class StreamEnableButton : public Button
{
public:
    
    /** Constructor*/
    StreamEnableButton(const String& name);
    
    /** Destructor*/
    ~StreamEnableButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

#endif  // __STREAMSELECTOR_H_BDCEE716__
