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
#include "SyncChannelSelector.h"
#include "../../Utils/Utils.h"

class RecordThread;
class RecordNode;

class FifoDrawerButton : public DrawerButton
{
public:
	FifoDrawerButton(const String& name);
	~FifoDrawerButton();
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class FifoMonitor : public Component, 
					public SettableTooltipClient,
					public Timer, 
				    public PopupChannelSelector::Listener
{
public:
	FifoMonitor(RecordNode* recordNode, uint16 streamId, String streamName);

	void setFillPercentage(float percentage);

	void timerCallback();

	void channelStateChanged(Array<int> selectedChannels);

	void mouseDown(const MouseEvent &event);

	void componentBeingDeleted(Component &component);

	std::vector<bool> channelStates;

private :

	OwnedArray<ChannelButton> channelButtons;
	void paint(Graphics &g);

	float fillPercentage;
	RecordNode *recordNode;
	uint16 streamId;
	String streamName;
	Random random;
	
};

class SyncControlButton : public Button, public Timer, public ComponentListener
{
public:
	SyncControlButton(RecordNode* node, const String& name, uint64 streamId);
	~SyncControlButton();

	int streamId;
	bool isPrimary;

	void mouseUp(const MouseEvent &event) override;

private:
	RecordNode* node;
    void timerCallback();
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
	
	void componentBeingDeleted(Component &component);

};

class RecordToggleButton : public Button, public Timer
{
public: 
	RecordToggleButton(RecordNode* node, const String& name);
	~RecordToggleButton();

private:
	RecordNode* node;
    void timerCallback();
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class RecordNodeEditor : 
	public GenericEditor, public Timer,
		   ComboBox::Listener, 
	       Label::Listener,
		   Button::Listener
{
public:

	/** Constructor */
	RecordNodeEditor(RecordNode* parentNode);

	/** Destructor*/
    virtual ~RecordNodeEditor() { }

	/** Hides FIFO monitors when the editor is collapsed*/
	void collapsedStateChanged() override;

    /** Used to automatically show FIFO monitors */
    void timerCallback();

	/** Propagates new settings to FIFO Monitors */
	void updateFifoMonitors();

	/** Shows/hides FIFO Monitors*/
	void showFifoMonitors(bool);

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
