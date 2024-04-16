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

#include <stdio.h>
#include "RecordNodeEditor.h"
#include "RecordNode.h"
#include "../../CoreServices.h"

StreamMonitor::StreamMonitor(RecordNode* rn, uint64 id)
	: LevelMonitor(rn),
	streamId(id)
{
	startTimerHz(10);
}

StreamMonitor::~StreamMonitor() {}

void StreamMonitor::timerCallback()
{
	if (((RecordNode*)processor)->recordThread->isThreadRunning())
		setFillPercentage(((RecordNode*)processor)->fifoUsage[streamId]);
	else
		setFillPercentage(0.0);
}

DiskSpaceMonitor::DiskSpaceMonitor(RecordNode* rn)
    : LevelMonitor(rn),
      lastFreeSpace(0.0),
      recordingTimeLeftInSeconds(0),
	  dataRate(0.0)
{
	startTimerHz(1);
}

DiskSpaceMonitor::~DiskSpaceMonitor() {}

void DiskSpaceMonitor::reset()
{
	lastUpdateTime = Time::getMillisecondCounterHiRes();
	lastFreeSpace = ((RecordNode*) processor)->getDataDirectory().getBytesFreeOnVolume();
}

void DiskSpaceMonitor::timerCallback()
{
	RecordNode* recordNode = (RecordNode*) processor;

	int64 bytesFree = recordNode->getDataDirectory().getBytesFreeOnVolume();
	int64 volumeSize = recordNode->getDataDirectory().getVolumeTotalSize();

	float ratio = float(bytesFree) / float(volumeSize);
	if (ratio > 0)
		setFillPercentage(1.0f - ratio);

	float currentTime = Time::getMillisecondCounterHiRes();

	if (recordNode->getRecordingStatus())
	{
		// Update data rate and recording time left every 30 seconds
		if (currentTime - lastUpdateTime > 5000.0f)
		{
			dataRate = (lastFreeSpace - bytesFree) / (currentTime - lastUpdateTime); //bytes/ms
			lastUpdateTime = currentTime;
			lastFreeSpace = bytesFree;

			recordingTimeLeftInSeconds = bytesFree / dataRate / 1000.0f;

			// Stop recording and show warning when less than 5 minutes of disk space left
			if (dataRate > 0.0f && recordingTimeLeftInSeconds < (60.0f * 5.0f))
			{
				CoreServices::setRecordingStatus(false);
				String msg = "Recording stopped. Less than 5 minutes of disk space remaining.";
				AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "WARNING", msg);
			}

			if (dataRate > 0.0f)
			{
				String msg = String(bytesFree / pow(2, 30)) + " GB available\n";
				msg += String(int(recordingTimeLeftInSeconds / 60.0f)) + " minutes remaining\n";
				msg += "Data rate: " + String(dataRate * 1000 / pow(2, 20), 2) + " MB/s";
				setTooltip(msg);

				LOGD("Data rate: ", dataRate, " bytes/ms");
			}
		}
	}
	else
	{
		setTooltip(String(bytesFree / pow(2, 30)) + " GB available");
	}

	repaint();
}

RecordChannelsParameterEditor::RecordChannelsParameterEditor(RecordNode* rn, Parameter* param, int rowHeightPixels, int rowWidthPixels) 
	: ParameterEditor(param), recordNode(rn)
{
	int numChannels = ((MaskChannelsParameter*)param)->getChannelStates().size();
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*)param)->getChannelStates())
		if (chan) selected++;

	uint64 streamId = ((DataStream*)param->getOwner())->getStreamId();
	String streamName  = ((DataStream*)param->getOwner())->getName();
	String sourceNodeId = String(((DataStream*)param->getOwner())->getSourceNodeId());

    monitor = std::make_unique<StreamMonitor>(recordNode, streamId);
    monitor->setTooltip(sourceNodeId + " | " + streamName);
	monitor->setComponentID(sourceNodeId + " | " + streamName);
	monitor->addListener(this);
	monitor->setBounds(0, 0, 15, 73);
	addAndMakeVisible(monitor.get());

	setBounds(0, 0, 15, 73);

	editor = monitor.get();
};

void RecordChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);

    param->setNextValue(newArray);

    updateView();
}

Array<int> RecordChannelsParameterEditor::getSelectedChannels()
{
	MaskChannelsParameter* p = (MaskChannelsParameter*)param;
	std::vector<bool> channelStates = p->getChannelStates();

	Array<int> selectedChannels;

	for (int i = 0; i < channelStates.size(); i++)
		if (channelStates[i])
			selectedChannels.add(i);

	return selectedChannels;
}

void RecordChannelsParameterEditor::buttonClicked(Button* label)
{
    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*)param;

    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector(label, this, channelStates);

	channelSelector->setChannelButtonColour(param->getColor());

	CoreServices::getPopupManager()->showPopup(std::unique_ptr<PopupComponent>(channelSelector), monitor.get());

	/*
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            editor->getScreenBounds(),
            nullptr);
	*/
}

void RecordChannelsParameterEditor::updateView()
{
	/* Do nothing, handled by StreamMonitor timerCallback */
}

void RecordChannelsParameterEditor::resized()
{
	updateBounds();
}


RecordToggleButton::RecordToggleButton(const String& name) : CustomToggleButton() {}

RecordToggleButton::~RecordToggleButton() {}

void RecordToggleButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),0.2*getWidth());

	if (!getToggleState())
		g.setColour(Colour(110,110,110));
	else
		g.setColour(Colour(255,0,0));

    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

	g.setColour(Colour(0,0,0));
	g.fillEllipse(0.35*getWidth(), 0.35*getHeight(), 0.3*getWidth(), 0.3*getHeight());
}

RecordToggleParameterEditor::RecordToggleParameterEditor(Parameter* param) : ParameterEditor(param)
{

	label = std::make_unique<Label>("Parameter name", param->getDisplayName());
    label->setFont(Font("Small Text", 12.0f, Font::plain));
    label->setColour(Label::textColourId, Colours::black);
    addAndMakeVisible(label.get());

    toggleButton = std::make_unique<RecordToggleButton>(param->getDisplayName());//param->getDisplayName());
    toggleButton->setClickingTogglesState (true);
    toggleButton->setTooltip ("Toggle recording state on/off");
    toggleButton->addListener(this);
    toggleButton->setToggleState(true, dontSendNotification);
	toggleButton->setBounds(50, 0, 15, 15);
    addAndMakeVisible(toggleButton.get());

    label->setBounds(0, 0, 100, 15);
	toggleButton->setBounds(95, 0, 15, 15);

	setBounds(0, 0, 150, 20);

	editor = toggleButton.get();
}

void RecordToggleParameterEditor::buttonClicked(Button*)
{
    param->setNextValue(toggleButton->getToggleState());
}

void RecordToggleParameterEditor::updateView()
{
    if (param != nullptr)
        toggleButton->setToggleState(param->getValue(), dontSendNotification);
}

void RecordToggleParameterEditor::resized()
{
    //toggleButton->setBounds(0, 0, 20, 20);
}

RecordNodeEditor::RecordNodeEditor(RecordNode* parentNode)
	: GenericEditor(parentNode),
	monitorsVisible(false), numDataStreams(0)
{
	desiredWidth = 165;

	recordNode = parentNode;

	fifoDrawerButton = new FifoDrawerButton(getNameAndId() + " Fifo Drawer Button");
	fifoDrawerButton->setBounds(4, 40, 10, 78);
	fifoDrawerButton->addListener(this);
	addAndMakeVisible(fifoDrawerButton);

	diskSpaceMonitor = new DiskSpaceMonitor(recordNode);
	diskSpaceMonitor->setBounds(18, 33, 15, 92);
	addAndMakeVisible(diskSpaceMonitor);

	addPathParameterEditor(Parameter::PROCESSOR_SCOPE, "directory", 42, 32);
	addComboBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "engine", 42, 57);

	for (auto& p : {"directory", "engine"})
    {
        auto* ed = getParameterEditor(p);
		ed->setLayout(ParameterEditor::Layout::nameHidden);
        ed->setBounds(ed->getX(), ed->getY(), 110, ed->getHeight());
    }

	addCustomParameterEditor(new RecordToggleParameterEditor(parentNode->getParameter("events")), 40, 85);
	addCustomParameterEditor(new RecordToggleParameterEditor(parentNode->getParameter("spikes")), 40, 107);

	// Show the fifo monitors by default
    fifoDrawerButton->triggerClick();

}

void RecordNodeEditor::timerCallback()
{
	fifoDrawerButton->triggerClick();
	stopTimer();
}

void RecordNodeEditor::startRecording()
{
	diskSpaceMonitor->reset();
}
    
void RecordNodeEditor::stopRecording()
{
}

void RecordNodeEditor::updateFifoMonitors()
{	
	auto removeExistingMonitors = [this](std::vector<ParameterEditor*>& monitors)
	{
		for (auto& monitor : monitors)
		{
			for (auto& ed : this->parameterEditors)
			{
				if (ed == monitor)
				{
					removeChildComponent(monitor);
					parameterEditors.removeObject(monitor);
				}
			}
		}
		monitors.clear();
	};

	removeExistingMonitors(streamMonitors);
	removeExistingMonitors(syncMonitors);

	int streamCount = 0;
	for (auto& stream : recordNode->getDataStreams())
	{
		uint64 streamId = stream->getStreamId();

		// Add a recording channel selector for each stream
		Parameter* channels = recordNode->getDataStream(streamId)->getParameter("channels");
		recordNode->getDataStream(streamId)->setColor("channels", getLookAndFeel().findColour(ProcessorColor::IDs::RECORD_COLOR));
		addCustomParameterEditor(new RecordChannelsParameterEditor(recordNode, channels), 18 + streamCount * 20, 32);
		parameterEditors.getLast()->setVisible(!getCollapsedState());
		parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
		streamMonitors.push_back(parameterEditors.getLast());

		// Add a sync line selector for each stream
		Parameter* syncLineParam = recordNode->getDataStream(streamId)->getParameter("sync_line");
		Parameter* mainSyncParam = recordNode->getParameter("main_sync");
		addSyncLineParameterEditor((TtlLineParameter*)syncLineParam, (SelectedStreamParameter*)mainSyncParam, 18 + streamCount * 20, 110);
		parameterEditors.getLast()->setVisible(!getCollapsedState());
		parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
		syncMonitors.push_back(parameterEditors.getLast());

		streamCount++;
	}
}

void RecordNodeEditor::collapsedStateChanged()
{
	if (getCollapsedState())
	{
		for (auto monitor : streamMonitors)
			monitor->setVisible(false);
		for (auto monitor : syncMonitors)
			monitor->setVisible(false);
	}
	else
	{
		for (auto monitor : streamMonitors)
			monitor->setVisible(monitorsVisible);
		for (auto monitor : syncMonitors)
			monitor->setVisible(monitorsVisible);
	}
}

void RecordNodeEditor::updateSettings()
{
	updateFifoMonitors();
	showFifoMonitors(fifoDrawerButton->getToggleState());
}

void RecordNodeEditor::buttonClicked(Button *button)
{
	if (button == fifoDrawerButton)
	{
		showFifoMonitors(button->getToggleState());
	}
}

void RecordNodeEditor::showFifoMonitors(bool show)
{
	numDataStreams = recordNode->getNumDataStreams();

	int dX = 20 * (numDataStreams + 1);
	dX = show ? dX : 0;

	fifoDrawerButton->setBounds(
		4 + dX, fifoDrawerButton->getY(),
		fifoDrawerButton->getWidth(), fifoDrawerButton->getHeight());

	diskSpaceMonitor->setBounds(
		18 + dX, diskSpaceMonitor->getY(),
		diskSpaceMonitor->getWidth(), diskSpaceMonitor->getHeight());

	for (auto& p : {"directory", "engine", "events", "spikes"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(42 + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

	desiredWidth = 165 + dX;

	if (getCollapsedState())
		return;

	for (auto& monitor : streamMonitors)
		monitor->setVisible(show);
	for (auto monitor : syncMonitors)
		monitor->setVisible(show);

	CoreServices::highlightEditor(this);
	deselect();
}

void FifoDrawerButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(Colour(110, 110, 110));
	if (isMouseOver)
		g.setColour(Colour(210, 210, 210));

	g.drawVerticalLine(5, 0.0f, getHeight());
}
