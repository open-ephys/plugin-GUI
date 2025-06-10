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
    Base class to display a metric related to synchronization
    in the StreamSelector

    Sub-classes override paint() to display the metric

*/
class SyncMonitor : public Component
{
public:
    /** Constructor */
    SyncMonitor();

    /** Destructor */
    ~SyncMonitor();

    /** Sets the most recent sync metric */
    void setSyncMetric (bool isSynchronized, float syncMetric);

    /** Enable or disable this component*/
    void setEnabled (bool isEnabled);

protected:
    bool isEnabled = true;
    bool isSynchronized = false;
    float metric = 0.0f;
};

/** 
* 
    Displays the discrepancy between actual and expected
    event times, given the current sync parameters

*/
class SyncAccuracyMonitor : public SyncMonitor
{
public:
    /** Constructor */
    SyncAccuracyMonitor() {}

    /** Destructor */
    ~SyncAccuracyMonitor() {}

    /** Paints the metric */
    void paint (Graphics& g);
};

/** 
* 
    Displays the approximate time since a last sync event 
    was received

*/
class LastSyncEventMonitor : public SyncMonitor
{
public:
    /** Constructor */
    LastSyncEventMonitor() {}

    /** Destructor */
    ~LastSyncEventMonitor() {}

    /** Paints the metric */
    void paint (Graphics& g);
};

/** 
* 
    Displays the offset between stream start times

*/
class SyncStartTimeMonitor : public SyncMonitor
{
public:
    /** Constructor */
    SyncStartTimeMonitor() {}

    /** Destructor */
    ~SyncStartTimeMonitor() {}

    /** Paints the metric */
    void paint (Graphics& g);
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
    void directoryInvalid(bool recordingStopped) override;

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

    /** Get the number of channels */
    int getChannelCount() override;

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
 
 Custom button to revert recording directory to the control panel default
 
 */
class ClearButton : public juce::Button
{
public:
    /** Constructor */
    ClearButton() : Button ("Revert Dir") {}

    /** Renders the button */
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
};

class RecordPathParameterEditor : public ParameterEditor,
                                  public Button::Listener
{
public:
    /** Constructor */
    RecordPathParameterEditor (Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 170);

    /** Destructor */
    virtual ~RecordPathParameterEditor() {}

    /** Displays the File Chooser*/
    void buttonClicked (Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    void updateView() override;

    /** Sets sub-component locations */
    void resized() override;

    /** Shows a clear button if non-default path is set*/
    void mouseEnter (const MouseEvent& event) override;

    /** Hides the clear button*/
    void mouseExit (const MouseEvent& event) override;

private:
    std::unique_ptr<TextButton> button;
    std::unique_ptr<ClearButton> clearButton;
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
    void setStreamStartTime (uint16 streamId, bool isSynchronized, float offsetMs);

    /** Set the time of latest sync pulse */
    void setLastSyncEvent (uint16 streamId, bool isSynchronized, float syncTimeSeconds);

    /** Set the synchronization accuracy metric for a particular stream */
    void setSyncAccuracy (uint16 streamId, bool isSynchronized, float expectedMinusActual);

    std::unique_ptr<FifoDrawerButton> fifoDrawerButton;

private:
    RecordNode* recordNode;

    OwnedArray<Label> streamLabels;
    std::vector<ParameterEditor*> streamMonitors;
    std::vector<ParameterEditor*> syncMonitors;
    std::unique_ptr<DiskMonitor> diskMonitor;

    std::map<uint16, SyncStartTimeMonitor*> syncStartTimeMonitors;
    std::map<uint16, SyncAccuracyMonitor*> syncAccuracyMonitors;
    std::map<uint16, LastSyncEventMonitor*> lastSyncEventMonitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNodeEditor);
};

#endif // __RECORDNODEEDITOR_H__
