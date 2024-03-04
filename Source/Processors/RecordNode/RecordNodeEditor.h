/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Open Ephys

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

#include "../Editors/PopupChannelSelector.h"
#include "../../Utils/Utils.h"

#include "SyncChannelSelector.h"
#include "SyncControlButton.h"

class RecordThread;
class RecordNode;

class StreamMonitor : public LevelMonitor
{
public:

	/** Constructor */
	StreamMonitor(RecordNode* rn, uint64 streamId);

	/** Destructor */
	~StreamMonitor();

	/** Updates the display */
	void timerCallback() override;

private:
	uint64 streamId;
};

class DiskSpaceMonitor : public LevelMonitor
{
public:

	/** Constructor */
	DiskSpaceMonitor(RecordNode* rn);

	/** Destructor */
	~DiskSpaceMonitor();

	/** Updates the display */
	void timerCallback() override;

	/** Resets timer */
	void reset();

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
    RecordChannelsParameterEditor(RecordNode* rn, Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~RecordChannelsParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

	Array<int> getSelectedChannels() override;

    /** Responds to changes in the PopupChannelSelector*/
    void channelStateChanged(Array<int> selectedChannels) override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<StreamMonitor> monitor;

	RecordNode* recordNode;
};

class SyncChannelsParameterEditor : public ParameterEditor,
	public Button::Listener,
	public SyncChannelSelector::Listener,
	public ComponentListener
{
public:

	/** Constructor */
	SyncChannelsParameterEditor(RecordNode* rn, Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

	/** Destructor */
	virtual ~SyncChannelsParameterEditor() { }

	/** Displays the PopupChannelSelector*/
	void buttonClicked(Button* label) override;

	/** Must ensure that editor state matches underlying parameter */
	virtual void updateView() override;

	/** Responds to changes in the PopupChannelSelector*/
	void channelStateChanged(Array<int> selectedChannels) override;

	/** Sets sub-component locations */
	virtual void resized() override;

	void componentBeingDeleted(Component &component) override;

private:
	std::unique_ptr<SyncControlButton> syncControlButton;

	RecordNode* recordNode;
};

/**
    
    Toggles event or spike recording on and off
 
 */
class RecordToggleButton : public CustomToggleButton
{
public:
    
    /** Constructor */
	RecordToggleButton(const String& name);
    
    /** Destructor */
	~RecordToggleButton();

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class RecordToggleParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:

    /** Constructor */
    RecordToggleParameterEditor(Parameter* param);

    /** Destructor*/
    ~RecordToggleParameterEditor() { }

    /** Respond to mute button clicks*/
    void buttonClicked(Button* label);

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
	FifoDrawerButton(const String& name) : DrawerButton(name) { };
    
    /** Destructor */
	~FifoDrawerButton() {};
private:
    
    /** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

/**
    Custom editor for the RecordNode
 */
class RecordNodeEditor : 
	public GenericEditor,
    public Timer,
    public Button::Listener
{
public:

	/** Constructor */
	RecordNodeEditor(RecordNode* parentNode);

	/** Destructor*/
    virtual ~RecordNodeEditor() { }

	/** Hides FIFO monitors when the editor is collapsed*/
	void collapsedStateChanged() override;

	/** Propagates new settings to FIFO Monitors */
	void updateFifoMonitors();

	/** Shows/hides FIFO Monitors*/
	void showFifoMonitors(bool);

	/** Automatically opens the drawer to reveal FIFO Monitors */
	void timerCallback();
    
    /** Updates settings based on Record Node state*/
    void updateSettings() override;

	/** Respond to button clicks*/
	void buttonClicked(Button* button);
    
    /** Disables parameter changes */
    void startRecording() override;
    
    /** Enables parameter changes */
    void stopRecording() override;

	ScopedPointer<FifoDrawerButton> fifoDrawerButton;

	ScopedPointer<ComboBox> engineSelectCombo;

	bool monitorsVisible;
	int numDataStreams;
    
private:
	RecordNode* recordNode;

	OwnedArray<Label> streamLabels;
	std::vector<ParameterEditor*> streamMonitors;
	std::vector<ParameterEditor*> syncMonitors;
	ScopedPointer<Label> diskSpaceLabel;
	ScopedPointer<DiskSpaceMonitor> diskSpaceMonitor;
	ScopedPointer<RecordToggleButton> recordToggleButton;
	ScopedPointer<Label> engineSelectLabel;
	ScopedPointer<Label> dataPathLabel;
	ScopedPointer<Button> dataPathButton;
	ScopedPointer<Label> recordEventsLabel;
	ScopedPointer<RecordToggleButton> eventRecord;
	ScopedPointer<Label> recordSpikesLabel;
	ScopedPointer<RecordToggleButton> spikeRecord;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordNodeEditor);

};

#endif  // __RECORDNODEEDITOR_H__
