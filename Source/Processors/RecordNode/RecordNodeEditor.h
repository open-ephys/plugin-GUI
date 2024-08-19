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

#ifndef __RECORDNODEEDITOR_H__
#define __RECORDNODEEDITOR_H__

#include "../../Utils/Utils.h"
#include "../Editors/GenericEditor.h"
#include "../Editors/PopupChannelSelector.h"

#include "DiskMonitor/DiskSpaceListener.h"

class RecordThread;
class RecordNode;


/** 
* 
    Displays the time since the last sync pulse was received

*/
class SyncTimeMonitor : public Component,
                                     public Timer
{
public:
    /** Constructor */
    SyncTimeMonitor();

    /** Destructor */
    ~SyncTimeMonitor();

    /** Sets the most recent offset (in ms)*/
    void setSyncTime (bool isSynchronized, float syncTimeSeconds);

    /** Enable or disable this component*/
    void setEnabled (bool isEnabled);

    /** Render the offset*/
    void paint (Graphics& g);

    /** Calls 'repaint' to display the latest delay*/
    void timerCallback();

    /** Starts the 500 ms painting timer */
    void startAcquisition();

    /** Stops the timer*/
    void stopAcquisition();

    /** Sends repaint command asynchronously */
    void handleCommandMessage (int commandId);

private:
    bool isEnabled;
    bool isSynchronized;
    Colour colour;
    float lastSyncTime;
    bool canRepaint = true;
};


/** 
* 
    Displays the offset between the start of each stream
    and the main stream

*/
class SyncOffsetMonitor : public Component,
                                public Timer
{
public:
    /** Constructor */
    SyncOffsetMonitor();

    /** Destructor */
    ~SyncOffsetMonitor();

    /** Sets the most recent offset (in ms)*/
    void setOffset (bool isSynchronized, float offsetMs);

    /** Enable or disable this component*/
    void setEnabled (bool isEnabled);

    /** Render the offset*/
    void paint (Graphics& g);

    /** Calls 'repaint' to display the latest delay*/
    void timerCallback();

    /** Starts the 500 ms painting timer */
    void startAcquisition();

    /** Stops the timer*/
    void stopAcquisition();

    /** Sends repaint command asynchronously */
    void handleCommandMessage (int commandId);

private:
    bool isEnabled;
    bool isSynchronized;
    Colour colour;
    float offset;
    bool canRepaint = true;
};

class StreamMonitor : public LevelMonitor
{
public:
    /** Constructor */
    StreamMonitor (RecordNode* rn, uint64 streamId);

    /** Destructor */
    ~StreamMonitor();

    /** Updates the display */
    void timerCallback() override;

    /** Draws the monitor with custom text */
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Updates the number of channels to be recorded */
    void updateChannelCount (int selectedChans);

private:
    uint64 streamId;
    int selectedChannels;
    int totalChannels;
};

class DiskMonitor : public LevelMonitor, public DiskSpaceListener
{
public:
    /** Constructor */
    DiskMonitor (RecordNode* rn);

    /** Destructor */
    ~DiskMonitor();

    /** Updates the display */ //TODO: Potentially unused
    void timerCallback() override;

    /** Update data rate */
    void update (float dataRate, int64 bytesFree, float timeLeft) override;

    /** Updates disk remaining disk space */
    void updateDiskSpace (float percentage) override;

    /** Responds to invalid directory */
    void directoryInvalid() override;

    /** Responds to low disk space */
    void lowDiskSpace() override;

private:
    int64 lastFreeSpace;
    float recordingTimeLeftInSeconds;
    float dataRate;
};

class RecordChannelsParameterEditor : public ParameterEditor,
                                      public Button::Listener,
                                      public PopupChannelSelector::Listener
{
public:
    /** Constructor */
    RecordChannelsParameterEditor (RecordNode* rn, Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~RecordChannelsParameterEditor() {}

    /** Displays the PopupChannelSelector*/
    void buttonClicked (Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    Array<int> getSelectedChannels() override;

    /** Responds to changes in the PopupChannelSelector*/
    void channelStateChanged (Array<int> selectedChannels) override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<StreamMonitor> monitor;

    RecordNode* recordNode;
};

/**
    
    Toggles event or spike recording on and off
 
 */
class RecordToggleButton : public CustomToggleButton
{
public:
    /** Constructor */
    RecordToggleButton (const String& name);

    /** Destructor */
    ~RecordToggleButton();

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class RecordToggleParameterEditor : public ParameterEditor,
                                    public Button::Listener
{
public:
    /** Constructor */
    RecordToggleParameterEditor (Parameter* param);

    /** Destructor*/
    ~RecordToggleParameterEditor() {}

    /** Respond to mute button clicks*/
    void buttonClicked (Button* label);

    /** Ensures button state aligns with underlying parameter*/
    virtual void updateView() override;

    /** Sets component layout*/
    virtual void resized();

private:
    std::unique_ptr<Label> label;
    std::unique_ptr<CustomToggleButton> toggleButton;
};

/**
    
    Vertical button that opens/closes the FIFO drawer
 
 */
class FifoDrawerButton : public DrawerButton
{
public:
    /** Constructor */
    FifoDrawerButton (const String& name) : DrawerButton (name) {};

    /** Destructor */
    ~FifoDrawerButton() {};

private:
    /** Renders the button*/
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

/**
    Custom editor for the RecordNode
 */
class RecordNodeEditor : public GenericEditor,
                         public Timer,
                         public Button::Listener
{
public:
    /** Constructor */
    RecordNodeEditor (RecordNode* parentNode);

    /** Destructor*/
    virtual ~RecordNodeEditor() {}

    /** Hides FIFO monitors when the editor is collapsed*/
    void collapsedStateChanged() override;

    /** Propagates new settings to sync monitors */
    void updateSyncMonitors();

    /** Propagates new settings to FIFO Monitors */
    void updateFifoMonitors();

    /** Shows/hides FIFO Monitors*/
    void showFifoMonitors (bool);

    /** Automatically opens the drawer to reveal FIFO Monitors */
    void timerCallback();

    /** Updates settings based on Record Node state*/
    void updateSettings() override;

    /** Respond to button clicks*/
    void buttonClicked (Button* button);

    /** Disables parameter changes */
    void startRecording() override {};

    /** Enables parameter changes */
    void stopRecording() override {};

    /** Set start time for each stream */
    void setStreamOffset (uint16 streamId, bool isSynchronized, float offsetMs);

    /** Set the time of latest sync pulse */
    void setLatestSyncTime (uint16 streamId, bool isSynchronized, float syncTimeSeconds);

    std::unique_ptr<FifoDrawerButton> fifoDrawerButton;

private:
    RecordNode* recordNode;

    OwnedArray<Label> streamLabels;
    std::vector<ParameterEditor*> streamMonitors;
    std::vector<ParameterEditor*> syncMonitors;
    std::unique_ptr<DiskMonitor> diskMonitor;

    std::map<uint16, SyncOffsetMonitor*> syncOffsetMonitors;
    std::map<uint16, SyncTimeMonitor*> syncTimeMonitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNodeEditor);
};

#endif // __RECORDNODEEDITOR_H__
