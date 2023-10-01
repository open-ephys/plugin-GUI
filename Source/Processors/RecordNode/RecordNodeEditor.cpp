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

#include "SyncChannelSelector.h"

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
	  dataRate(0.0) {}

DiskSpaceMonitor::~DiskSpaceMonitor() {}

void DiskSpaceMonitor::timerCallback()
{

	RecordNode* recordNode = (RecordNode*) processor;

	lastFreeSpace = recordNode->getDataDirectory().getBytesFreeOnVolume();
	float bytesFree = (float) recordNode->getDataDirectory().getBytesFreeOnVolume();
	float volumeSize = (float) recordNode->getDataDirectory().getVolumeTotalSize();

	float ratio = bytesFree / volumeSize;

	if (ratio > 0)
		setFillPercentage(1.0f - ratio);

	if (!recordingTimeLeftInSeconds)
		setTooltip(String(bytesFree / pow(2, 30)) + " GB available");

	float currentTime = Time::getMillisecondCounterHiRes();

	// Update data rate and recording time left every 30 seconds
	if (recordNode->getRecordingStatus() && currentTime - lastUpdateTime > 30000) {

		if (lastUpdateTime == 0.0) {
			lastUpdateTime = Time::getMillisecondCounterHiRes();
			lastFreeSpace = bytesFree;
		}
		else
		{
			dataRate = (lastFreeSpace - bytesFree) / (currentTime - lastUpdateTime); //bytes/ms
			lastUpdateTime = currentTime;
			lastFreeSpace = bytesFree;

			recordingTimeLeftInSeconds = (int) (bytesFree / dataRate / 1000.0f);

			LOGD("Data rate: ", dataRate, " bytes/ms");

			// Stop recording and show warning when less than 5 minutes of disk space left
			if (dataRate > 0 && recordingTimeLeftInSeconds < 60*5) {
				CoreServices::setRecordingStatus(false);
				String msg = "Recording stopped. Less than 5 minutes of disk space remaining.";
				AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "WARNING", msg);
			}

			String msg = String(bytesFree / pow(2, 30)) + " GB available\n";
			msg += String(recordingTimeLeftInSeconds / 60) + " minutes remaining\n";
			msg += "Data rate: " + String(dataRate * 1000 / pow(2, 20), 2) + " MB/s";
			setTooltip(msg);
		}
	}

	repaint();
}

RecordChannelsParameterEditor::RecordChannelsParameterEditor(RecordNode* rn, Parameter* param, int rowHeightPixels, int rowWidthPixels) 
	: ParameterEditor(param)
{

	recordNode = rn;

	int numChannels = ((MaskChannelsParameter*)param)->getChannelStates().size();
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*)param)->getChannelStates())
		if (chan) selected++;

	/* No place for label w/ current layout -- maybe in a future layout
	label = std::make_unique<Label>("Parameter name", param->getDisplayName());
	label->setFont(Font("Small Text", 12.0f, Font::plain));
	label->setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(label.get());
	*/

	uint64 streamId = ((DataStream*)param->getOwner())->getStreamId();
	String streamName  = ((DataStream*)param->getOwner())->getName();
	String sourceNodeId = String(((DataStream*)param->getOwner())->getSourceNodeId());

    monitor = std::make_unique<StreamMonitor>(recordNode, streamId);
    monitor->setTooltip(sourceNodeId + " | " + streamName);
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

void RecordChannelsParameterEditor::buttonClicked(Button* label)
{
    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*)param;

    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector(this, channelStates);

    channelSelector->setChannelButtonColour(Colour(255, 0, 0));

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            monitor->getScreenBounds(),
            nullptr);
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

	/*Draw static black circle in center on top */
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

SyncChannelsParameterEditor::SyncChannelsParameterEditor(RecordNode* rn, Parameter* param, int rowHeightPixels, int rowWidthPixels) 
	: ParameterEditor(param)
{

	recordNode = rn;

	uint64 streamId = ((DataStream*)param->getOwner())->getStreamId();
	String streamName  = ((DataStream*)param->getOwner())->getName();
	String sourceNodeId = String(((DataStream*)param->getOwner())->getSourceNodeId());

	int nEvents = 8; //TODO: Better way to determine how many TTL lines are available

	syncControlButton = std::make_unique<SyncControlButton>(recordNode,
                                                recordNode->getDataStream(streamId)->getName(),
                                                streamId, 8);
	syncControlButton->setTooltip("Configure synchronization settings for this stream");
	syncControlButton->addListener(this);
	syncControlButton->setBounds(0, 0, 15, 15);
	addAndMakeVisible(syncControlButton.get());

	setBounds(0, 0, 15, 15);

	editor = syncControlButton.get();
}

void SyncChannelsParameterEditor::buttonClicked(Button*)
{
    if (param == nullptr)
        return;

    SelectedChannelsParameter* p = (SelectedChannelsParameter*)param;

    auto* channelSelector = new PopupChannelSelector(this, p->getChannelStates());

    channelSelector->setChannelButtonColour(param->getColor());
    
    channelSelector->setMaximumSelectableChannels(p->getMaxSelectableChannels());

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
            editor->getScreenBounds(),
            nullptr);
}

void SyncChannelsParameterEditor::channelStateChanged(Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add(newChannels[i]);
    
    param->setNextValue(newArray);

    updateView();

}

void SyncChannelsParameterEditor::updateView()
{
	//if (param != nullptr)
	//	toggleButton->setToggleState(param->getValue(), dontSendNotification);
}

void SyncChannelsParameterEditor::resized()
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

	/*
	diskSpaceLabel = new Label("diskSpaceLabel", "Avail:");
	diskSpaceLabel->setBounds(7, 21, 40, 20);
	diskSpaceLabel->setFont(Font("Small Text", 8.0f, Font::plain));
	*/

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

	//addCustomParameterEditor(new RecordChannelsParameterEditor(parentNode, parentNode->getParameter("channels")), 20, 150);

	/*
	dataPathLabel = new Label(CoreServices::getRecordingParentDirectory().getFullPathName());
	dataPathLabel->setText(CoreServices::getRecordingParentDirectory().getFullPathName(), juce::NotificationType::dontSendNotification);
	dataPathLabel->setTooltip(dataPathLabel->getText());
    dataPathLabel->setMinimumHorizontalScale(0.7f);
	dataPathLabel->setBounds(42,35,72,20);
	dataPathLabel->setColour(Label::backgroundColourId, Colours::grey);
	dataPathLabel->setColour(Label::backgroundWhenEditingColourId, Colours::white);
	dataPathLabel->setJustificationType(Justification::centredLeft);
	dataPathLabel->addListener(this);
	addAndMakeVisible(dataPathLabel);
	dataPathLabel->setVisible(false);

	dataPathButton = new UtilityButton("...", Font(12));
	dataPathButton->setBounds(117, 35, 18, 20);
	dataPathButton->addListener(this);
	addAndMakeVisible(dataPathButton);
	dataPathButton->setVisible(false);

	engineSelectCombo = new ComboBox("engineSelectCombo");
	engineSelectCombo->setBounds(42, 66, 93, 20);

	std::vector<RecordEngineManager*> engines = recordNode->getAvailableRecordEngines();

	int defaultEngine = 0;

	for (int i = 0; i < engines.size(); i++)
	{
		engineSelectCombo->addItem(engines[i]->getName(), i + 1);

		if (CoreServices::getDefaultRecordEngineId().equalsIgnoreCase(engines[i]->getID()))
			defaultEngine = i + 1;

	}
	engineSelectCombo->setSelectedId(defaultEngine);
	engineSelectCombo->addListener(this);
	//addAndMakeVisible(engineSelectCombo);

	recordEventsLabel = new Label("recordEvents", "RECORD EVENTS");
	recordEventsLabel->setBounds(85, 92, 80, 20);
	//recordEventsLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	recordEventsLabel->setFont(Font("Arial", "Regular", 10.0f));
	//addAndMakeVisible(recordEventsLabel);

	eventRecord = new RecordToggleButton(getNameAndId() + " Event Recording Toggle Button");
	eventRecord->setBounds(155, 94, 15, 15);
	eventRecord->addListener(this);
	eventRecord->setToggleState(1, sendNotification); //enable event recortding by default
	//addAndMakeVisible(eventRecord);

	recordSpikesLabel = new Label("recordSpikes", "RECORD SPIKES");
	recordSpikesLabel->setBounds(85, 110, 76, 20);
	//recordSpikesLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	recordSpikesLabel->setFont(Font("Arial", "Regular", 10.0f));
	//addAndMakeVisible(recordSpikesLabel);

	spikeRecord = new RecordToggleButton(getNameAndId() + " Spike Recording Toggle Button");
	spikeRecord->setBounds(155, 112, 15, 15);
	spikeRecord->addListener(this);
	spikeRecord->setToggleState(1, sendNotification); //enable spike recording by default
	//addAndMakeVisible(spikeRecord);

	writeSpeedLabel = new Label("writeSpeedLabel", "WRITE SPEED");
	writeSpeedLabel->setBounds(40, 102, 76, 20);
	writeSpeedLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(writeSpeedLabel);
	*/

    fifoDrawerButton->triggerClick();

}

void RecordNodeEditor::timerCallback()
{
	fifoDrawerButton->triggerClick();
	stopTimer();
	//fifoDrawerButton->setEnabled(false);
}

void RecordNodeEditor::startRecording()
{
	/*
    dataPathButton->setEnabled(false);
    engineSelectCombo->setEnabled(false);
    eventRecord->setEnabled(false);
    spikeRecord->setEnabled(false);
	*/
}
    
void RecordNodeEditor::stopRecording()
{
	/*
    dataPathButton->setEnabled(true);
    engineSelectCombo->setEnabled(true);
    eventRecord->setEnabled(true);
    spikeRecord->setEnabled(true);
	*/
}

void RecordNodeEditor::comboBoxChanged(ComboBox* box)
{

	/*

	if (!recordNode->recordThread->isThreadRunning())
	{
		int selectedEngineIndex = box->getSelectedId() - 1;

		std::vector<RecordEngineManager*> engines = CoreServices::getAvailableRecordEngines();

#ifdef _WIN32
		if (engines[selectedEngineIndex]->getID().equalsIgnoreCase("OPENEPHYS") &&
			recordNode->getNumInputs() > 300)
		{

			int new_max = 0;
			int calculated_max = recordNode->getNumInputs()
								 + recordNode->getTotalEventChannels()
								 + recordNode->getTotalSpikeChannels()
								 + recordNode->getNumDataStreams() // 1 timestamp file per stream
								 + 5; // 1 each for STDIN + STDOUT + STDERROR, 1 for message channel & 1 for structure.openephys file 

			if (calculated_max < 8192)
				new_max = _setmaxstdio(calculated_max);

			if (new_max != calculated_max)
			{
				AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
					"WARNING", "Open Ephys format does not support this many simultaneously recorded channels. Resetting to Binary format.");
				box->setSelectedItemIndex(0);
				recordNode->setEngine("BINARY");

				return;
			}
		}
#endif

		recordNode->setEngine(engines[selectedEngineIndex]->getID());

		CoreServices::saveRecoveryConfig();
		CoreServices::createNewRecordingDirectory();
	}

	*/

}

void RecordNodeEditor::setEngine(String id)
{

	/*
	int selectedIndex = 0;

	for (auto engine : CoreServices::getAvailableRecordEngines())
	{
		selectedIndex++;

		if (engine->getID().compare(id) == 0)
			engineSelectCombo->setSelectedId(selectedIndex, dontSendNotification);
	}
	*/
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
	for (auto const& [streamId, channelStates] : recordNode->recordContinuousChannels)
	{
		Parameter* channels = recordNode->getDataStream(streamId)->getParameter("channels");
		recordNode->getDataStream(streamId)->setColor("channels", getLookAndFeel().findColour(ProcessorColor::IDs::RECORD_COLOR));
		addCustomParameterEditor(new RecordChannelsParameterEditor(recordNode, channels), 18 + streamCount * 20, 32);
		streamMonitors.push_back(parameterEditors.getLast());

		Parameter* sync = recordNode->getDataStream(streamId)->getParameter("sync_line");
		recordNode->getDataStream(streamId)->setColor("sync_line", getLookAndFeel().findColour(ProcessorColor::IDs::SYNC_COLOR));
		addCustomParameterEditor(new SyncChannelsParameterEditor(recordNode, sync), 18 + streamCount * 20, 110);
		syncMonitors.push_back(parameterEditors.getLast());
				
		streamCount++;
	}

	/*
	LOGD("Update FIFO monitors 2.");
	streamLabels.clear();
	streamMonitors.clear();
	streamRecords.clear();

	int streamCount = 0;

	if (recordNode->getNumDataStreams() == 0)
		return;

	LOGD("Update FIFO monitors 3.");

	for (auto const& [streamId, channelStates] : recordNode->recordContinuousChannels)
	{

		LOGD("Update FIFO monitors 4.");
		Parameter* selectedChannels = recordNode->getParameter("channels");
		DataStream* stream = recordNode->getDataStream(streamId);
		streamMonitors.add(new BufferMonitor(recordNode, selectedChannels, stream));
		streamMonitors.getLast()->LevelMonitor::setBounds(18 + streamCount * 20, 32, 15, 73);
		addAndMakeVisible(static_cast<LevelMonitor*>(streamMonitors.getLast())->setBounds(18 + streamCount * 20, 32, 15, 73););
		streamMonitors.getLast()->setVisible(false);

		LOGD("Adding sync control button for stream id: ", streamId);
        
        const Array<EventChannel*> eventChannels = recordNode->getDataStream(streamId)->getEventChannels();

        int nEvents;

        if (eventChannels.size() > 0)
            nEvents = eventChannels[0]->getMaxTTLBits();
        else
            nEvents = 1;
        
		streamRecords.add(new SyncControlButton(recordNode,
                                                recordNode->getDataStream(streamId)->getName(),
                                                streamId, nEvents));
		streamRecords.getLast()->setBounds(18 + streamCount * 20, 110, 15, 15);
		addAndMakeVisible(streamRecords.getLast());
		streamRecords.getLast()->setVisible(false);

		streamCount++;

	}
	*/

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
	LOGD("Calling RecordNodeEditor::updateSettings()");

	updateFifoMonitors();
    /*
    eventRecord->setToggleState(recordNode->recordEvents, dontSendNotification);
    spikeRecord->setToggleState(recordNode->recordSpikes, dontSendNotification);
    
    dataPathLabel->setText(recordNode->getDataDirectory().getFullPathName(), dontSendNotification);
     */

}

void RecordNodeEditor::buttonClicked(Button *button)
{
	/*
	if (button == recordToggleButton)
	{
		//TODO: Clicking on the master record monitor should do something useful in the future...
	}
	else if (button == eventRecord && !recordNode->recordThread->isThreadRunning())
	{
		recordNode->setRecordEvents(button->getToggleState());
	}
	else if (button == spikeRecord && !recordNode->recordThread->isThreadRunning())
	{
		recordNode->setRecordSpikes(button->getToggleState());
	}
	*/

	//TODO: Could probably turn this into a CustomToggleButton
	if (button == fifoDrawerButton)
	{
		updateFifoMonitors();
		showFifoMonitors(button->getToggleState());
	}
	/*
	else if (button == dataPathButton)
	{
		LOGD("Change data write directory!");

		FileChooser chooseWriteDirectory ("Please select the location to write data to...",
											File(dataPathLabel->getText()), "");

		if (chooseWriteDirectory.browseForDirectory())
		{
			recordNode->setDataDirectory(chooseWriteDirectory.getResult());
			dataPathLabel->setText(chooseWriteDirectory.getResult().getFullPathName(), juce::NotificationType::dontSendNotification);
			dataPathLabel->setTooltip(dataPathLabel->getText());
		}

	}
	*/

}

void RecordNodeEditor::labelTextChanged(Label* label)
{
	/*
	if(label == dataPathLabel)
	{
		recordNode->setDataDirectory(label->getText());
		label->setTooltip(label->getText());
	}
	*/
}

void RecordNodeEditor::setDataDirectory(String dir)
{
	/*
	dataPathLabel->setText(dir, sendNotificationSync);
	*/
}

void RecordNodeEditor::showFifoMonitors(bool show)
{

	monitorsVisible = show;

	int offset;

	if (show)
		numDataStreams = recordNode->getNumDataStreams();

	int dX = 20 * (numDataStreams + 1);
	dX = show ? dX : -dX;

	fifoDrawerButton->setBounds(
		fifoDrawerButton->getX() + dX, fifoDrawerButton->getY(),
		fifoDrawerButton->getWidth(), fifoDrawerButton->getHeight());
	/*
	diskSpaceLabel->setBounds(
		diskSpaceLabel->getX() + dX, diskSpaceLabel->getY(),
		diskSpaceLabel->getWidth(), diskSpaceLabel->getHeight());
	*/

	diskSpaceMonitor->setBounds(
		diskSpaceMonitor->getX() + dX, diskSpaceMonitor->getY(),
		diskSpaceMonitor->getWidth(), diskSpaceMonitor->getHeight());

	for (auto& p : {"directory", "engine", "events", "spikes"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(ed->getX() + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

    /*
	recordToggleButton->setBounds(
		recordToggleButton->getX() + dX, recordToggleButton->getY(),
		recordToggleButton->getWidth(), recordToggleButton->getHeight());

	dataPathLabel->setBounds(
		dataPathLabel->getX() + dX, dataPathLabel->getY(),
		dataPathLabel->getWidth(), dataPathLabel->getHeight());

	dataPathButton->setBounds(
		dataPathButton->getX() + dX, dataPathButton->getY(),
		dataPathButton->getWidth(), dataPathButton->getHeight());

	engineSelectCombo->setBounds(
		engineSelectCombo->getX() + dX, engineSelectCombo->getY(),
		engineSelectCombo->getWidth(), engineSelectCombo->getHeight());

	recordEventsLabel->setBounds(
		recordEventsLabel->getX() + dX, recordEventsLabel->getY(),
		recordEventsLabel->getWidth(), recordEventsLabel->getHeight());

	eventRecord->setBounds(
		eventRecord->getX() + dX, eventRecord->getY(),
		eventRecord->getWidth(), eventRecord->getHeight());

	recordSpikesLabel->setBounds(
		recordSpikesLabel->getX() + dX, recordSpikesLabel->getY(),
		recordSpikesLabel->getWidth(), recordSpikesLabel->getHeight());

	spikeRecord->setBounds(
		spikeRecord->getX() + dX, spikeRecord->getY(),
		spikeRecord->getWidth(), spikeRecord->getHeight());
    
	streamSelector->setBounds(
		streamSelector->getX() + dX, streamSelector->getY(),
		streamSelector->getWidth(), streamSelector->getHeight());
	*/

	desiredWidth += dX;

	if (getCollapsedState())
		return;

	/*
	for (auto const& [streamId, channelStates] : recordNode->recordContinuousChannels)
	{
		addCustomParameterEditor(new RecordChannelsParameterEditor(recordNode, recordNode->getParameter("channels")), 18 + streamCount * 20, 32);
		streamCount++;
	}
	*/

	for (auto& monitor : streamMonitors)
		monitor->setVisible(show);
	for (auto monitor : syncMonitors)
		monitor->setVisible(show);

	CoreServices::highlightEditor(this);
	deselect();

}

FifoDrawerButton::FifoDrawerButton(const String &name) : DrawerButton(name)
{
}

FifoDrawerButton::~FifoDrawerButton()
{
}

void FifoDrawerButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(Colour(110, 110, 110));
	if (isMouseOver)
		g.setColour(Colour(210, 210, 210));

	//g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	//g.drawVerticalLine(7, 0.0f, getHeight());
}
