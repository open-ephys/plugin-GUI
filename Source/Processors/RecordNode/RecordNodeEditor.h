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

#include "SyncControlButton.h"

class RecordThread;
class RecordNode;

/**
    
    Vertical button that opens/closes the FIFO drawer
 
 */
class FifoDrawerButton : public DrawerButton
{
public:
    
    /** Constructor */
	FifoDrawerButton(const String& name);
    
    /** Destructor */
	~FifoDrawerButton();
private:
    
    /** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

/**
    
    Component that displays the FIFO filling state for each stream
 
 */
class FifoMonitor : public Component, 
					public SettableTooltipClient,
					public Timer, 
				    public PopupChannelSelector::Listener,
                    public ComponentListener
{
public:
    
    /** Constructor */
	FifoMonitor(RecordNode* recordNode, uint16 streamId, String streamName);

    /** Sets fill amount */
	void setFillPercentage(float percentage);

    /** Updates the display */
	void timerCallback();

    /** Called when the selected channels are changed */
	void channelStateChanged(Array<int> selectedChannels);

    /** Listens for mouse clicks and opens the popup channel selection */
	void mouseDown(const MouseEvent &event);

    /** Called when channel selection popup is closed */
	//void componentBeingDeleted(Component &component);

	std::vector<bool> channelStates;

private :

	OwnedArray<ChannelButton> channelButtons;
	void paint(Graphics &g);

	float fillPercentage;
	RecordNode *recordNode;
	uint16 streamId;
	String streamName;
	Random random;
    
    bool stateChangeSinceLastUpdate;

	float dataRate;
	float lastFreeSpace;
	float lastUpdateTime;
	int recordingTimeLeftInSeconds;
	
};

/**
    
    Toggles event or spike recording on and off
 
 */
class RecordToggleButton :
    public Button,
    public Timer
{
public:
    
    /** Constructor */
	RecordToggleButton(RecordNode* node, const String& name);
    
    /** Destructor */
	~RecordToggleButton();

private:
	RecordNode* node;
    
    /** Repaints the button */
    void timerCallback();
    
    /** Renders the button*/
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

/**
    Custom editor for the RecordNode
 */
class RecordNodeEditor : 
	public GenericEditor,
    public Timer,
    public ComboBox::Listener,
    public Label::Listener,
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

	/** Updates the local directory for this Record Node*/
	void setDataDirectory(String dir);

	/** Sets the Record Engine for this Record Node*/
	void setEngine(String id);
    
    /** Updates settings based on Record Node state*/
    void updateSettings() override;

	/** Used to change active Record Engine */
	void comboBoxChanged(ComboBox*); 

	/** Respond to changes in data directory */
	void labelTextChanged(Label*);

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
	OwnedArray<FifoMonitor> streamMonitors;
	OwnedArray<SyncControlButton> streamRecords;
	ScopedPointer<Label> diskSpaceLabel;
	ScopedPointer<FifoMonitor> diskSpaceMonitor;
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
