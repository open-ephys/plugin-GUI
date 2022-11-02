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

RecordNodeEditor::RecordNodeEditor(RecordNode* parentNode)
	: GenericEditor(parentNode),
	monitorsVisible(false), numDataStreams(0)
{

	recordNode = parentNode;

	fifoDrawerButton = new FifoDrawerButton(getNameAndId() + " Fifo Drawer Button");
	fifoDrawerButton->setBounds(4, 40, 10, 78);
	fifoDrawerButton->addListener(this);
	addAndMakeVisible(fifoDrawerButton);

	diskSpaceLabel = new Label("diskSpaceLabel", "Avail:");
	diskSpaceLabel->setBounds(7, 21, 40, 20);
	diskSpaceLabel->setFont(Font("Small Text", 8.0f, Font::plain));

	diskSpaceMonitor = new FifoMonitor(recordNode, 0, "Available Disk Space");
	diskSpaceMonitor->setBounds(18, 33, 15, 92);
	addAndMakeVisible(diskSpaceMonitor);

	recordToggleButton = new RecordToggleButton(recordNode, getNameAndId() + " Recording Toggle Button");
	recordToggleButton->setBounds(18, 110, 15, 15);
	recordToggleButton->addListener(this);
	//addAndMakeVisible(recordToggleButton); // functionality not implemented yet

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

	dataPathButton = new UtilityButton("...", Font(12));
	dataPathButton->setBounds(117, 35, 18, 20);
	dataPathButton->addListener(this);
	addAndMakeVisible(dataPathButton);

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
	addAndMakeVisible(engineSelectCombo);

	recordEventsLabel = new Label("recordEvents", "RECORD EVENTS");
	recordEventsLabel->setBounds(40, 91, 80, 20);
	recordEventsLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordEventsLabel);

	eventRecord = new RecordToggleButton(recordNode, getNameAndId() + " Event Recording Toggle Button");
	eventRecord->setBounds(120, 93, 15, 15);
	eventRecord->addListener(this);
	eventRecord->setToggleState(1, sendNotification); //enable event recortding by default
	addAndMakeVisible(eventRecord);

	recordSpikesLabel = new Label("recordSpikes", "RECORD SPIKES");
	recordSpikesLabel->setBounds(40, 108, 76, 20);
	recordSpikesLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordSpikesLabel);

	spikeRecord = new RecordToggleButton(recordNode, getNameAndId() + " Spike Recording Toggle Button");
	spikeRecord->setBounds(120, 110, 15, 15);
	spikeRecord->addListener(this);
	spikeRecord->setToggleState(1, sendNotification); //enable spike recording by default
	addAndMakeVisible(spikeRecord);

	/*
	writeSpeedLabel = new Label("writeSpeedLabel", "WRITE SPEED");
	writeSpeedLabel->setBounds(40, 102, 76, 20);
	writeSpeedLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(writeSpeedLabel);
	*/

	desiredWidth = 150;

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

    dataPathButton->setEnabled(false);
    engineSelectCombo->setEnabled(false);
    eventRecord->setEnabled(false);
    spikeRecord->setEnabled(false);
}
    
void RecordNodeEditor::stopRecording()
{
    dataPathButton->setEnabled(true);
    engineSelectCombo->setEnabled(true);
    eventRecord->setEnabled(true);
    spikeRecord->setEnabled(true);
}


void RecordNodeEditor::comboBoxChanged(ComboBox* box)
{

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

}

void RecordNodeEditor::setEngine(String id)
{

	int selectedIndex = 0;

	for (auto engine : CoreServices::getAvailableRecordEngines())
	{
		selectedIndex++;

		if (engine->getID().compare(id) == 0)
			engineSelectCombo->setSelectedId(selectedIndex, dontSendNotification);
	}
}

void RecordNodeEditor::updateFifoMonitors()
{

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
		streamMonitors.add(new FifoMonitor(recordNode, streamId, recordNode->getDataStream(streamId)->getName()));
		streamMonitors.getLast()->setBounds(18 + streamCount * 20, 32, 15, 73);
		addAndMakeVisible(streamMonitors.getLast());
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

}

void RecordNodeEditor::updateSettings()
{
    eventRecord->setToggleState(recordNode->recordEvents, dontSendNotification);
    spikeRecord->setToggleState(recordNode->recordSpikes, dontSendNotification);
    
    dataPathLabel->setText(recordNode->getDataDirectory().getFullPathName(), dontSendNotification);

}

void RecordNodeEditor::buttonClicked(Button *button)
{

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
	else if (button == fifoDrawerButton)
	{
		updateFifoMonitors();

		showFifoMonitors(button->getToggleState());

	}
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

}

void RecordNodeEditor::labelTextChanged(Label* label)
{
	if(label == dataPathLabel)
	{
		recordNode->setDataDirectory(label->getText());
		label->setTooltip(label->getText());
	}
}

void RecordNodeEditor::collapsedStateChanged()
{

	if (getCollapsedState())
	{
		for (auto streamLabel : streamLabels)
			streamLabel->setVisible(false);
		for (auto streamMonitor : streamMonitors)
			streamMonitor->setVisible(false);
		for (auto streamRecord : streamRecords)
			streamRecord->setVisible(false);
	}
	else
	{
		for (auto spl : streamLabels)
			spl->setVisible(monitorsVisible);
		for (auto spm : streamMonitors)
			spm->setVisible(monitorsVisible);
		for (auto spr : streamRecords)
			spr->setVisible(monitorsVisible);
	}


}

void RecordNodeEditor::setDataDirectory(String dir)
{
	dataPathLabel->setText(dir, sendNotificationSync);
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

	diskSpaceLabel->setBounds(
		diskSpaceLabel->getX() + dX, diskSpaceLabel->getY(),
		diskSpaceLabel->getWidth(), diskSpaceLabel->getHeight());

	diskSpaceMonitor->setBounds(
		diskSpaceMonitor->getX() + dX, diskSpaceMonitor->getY(),
		diskSpaceMonitor->getWidth(), diskSpaceMonitor->getHeight());

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


	desiredWidth += dX;

	if (getCollapsedState())
		return;

	for (auto spl : streamLabels)
		spl->setVisible(show);
	for (auto spm : streamMonitors)
		spm->setVisible(show);
	for (auto spr : streamRecords)
		spr->setVisible(show);

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

RecordToggleButton::RecordToggleButton(RecordNode* _node, const String& name) : Button(name) {
	setClickingTogglesState(true);
	node = _node;
    startTimer(200);
}

RecordToggleButton::~RecordToggleButton() {}

void RecordToggleButton::timerCallback()
{
    repaint();
}

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

FifoMonitor::FifoMonitor(RecordNode* node, uint16 streamId_, String streamName_) :
	recordNode(node),
	streamId(streamId_),
	streamName(streamName_),
	fillPercentage(0.0),
    stateChangeSinceLastUpdate(false),
	dataRate(0.0),
	lastUpdateTime(0.0),
	lastFreeSpace(0.0),
	recordingTimeLeftInSeconds(0)
{
	startTimer(500);
}

/* RECORD CHANNEL SELECTOR LISTENER */
void FifoMonitor::mouseDown(const MouseEvent &event)
{

	// Ignore right-clicks...add functionality for right-clicks later...
	if (event.mods.isRightButtonDown())
		return;

	if (streamId == 0) // Disk space box was selected
		return;

	LOGA("Show Record Node channel selector for stream ", streamName);

	channelStates = recordNode->recordContinuousChannels[streamId];

	bool editable = !recordNode->recordThread->isThreadRunning();
    auto* channelSelector = new PopupChannelSelector(this, channelStates);
	channelSelector->setChannelButtonColour(Colours::red);

    CallOutBox& myBox
        = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);
    
    //myBox.addComponentListener(this);

}
/* No longer called -- all updates happen before the callout box is closed
void FifoMonitor::componentBeingDeleted(Component &component)
{
    // called when popup window closes
    
    LOGD("Record Node channel selector closed");
    
    if (stateChangeSinceLastUpdate)
    {
        CoreServices::updateSignalChain(recordNode->getEditor());
        stateChangeSinceLastUpdate = false;
    }

}
*/

void FifoMonitor::channelStateChanged(Array<int> selectedChannels)
{
	for (int i = 0; i < channelStates.size(); i++)
	{
		if (selectedChannels.contains(i))
			channelStates[i] = true;
		else
			channelStates[i] = false;
	}

	recordNode->updateChannelStates(streamId, channelStates);
    
    stateChangeSinceLastUpdate = true;

}


void FifoMonitor::timerCallback()
{

	if (streamId == 0) /* Disk space monitor */
	{
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
	}
	else /* Stream monitor */
	{
		setTooltip(String(recordNode->getDataStream(streamId)->getSourceNodeId())+" | "+streamName);
		setFillPercentage(recordNode->fifoUsage[streamId]);
	}

}

void FifoMonitor::setFillPercentage(float fill_)
{
	fillPercentage = fill_;

	repaint();
}

void FifoMonitor::paint(Graphics &g)
{
	g.setColour(Colours::grey);
	g.fillRoundedRectangle(0, 0, this->getWidth(), this->getHeight(), 4);
	g.setColour(Colours::lightslategrey);
	g.fillRoundedRectangle(2, 2, this->getWidth() - 4, this->getHeight() - 4, 2);

	if (fillPercentage < 0.7)
		g.setColour(Colours::yellow);
	else if (fillPercentage < 0.9)
		g.setColour(Colours::orange);
	else
		g.setColour(Colours::red);

	float barHeight = (this->getHeight() - 4) * fillPercentage;
	g.fillRoundedRectangle(2, this->getHeight() - 2 - barHeight, this->getWidth() - 4, barHeight, 2);
}
