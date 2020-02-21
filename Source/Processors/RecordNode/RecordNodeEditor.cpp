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

RecordNodeEditor::RecordNodeEditor(RecordNode* parentNode, bool useDefaultParameterEditors = true)
	: GenericEditor(parentNode, useDefaultParameterEditors),
	numSubprocessors(0)
{

	recordNode = parentNode;

	desiredWidth = 150;

	fifoDrawerButton = new FifoDrawerButton("FifoDrawer");
	fifoDrawerButton->setBounds(4, 40, 10, 78);
	fifoDrawerButton->addListener(this);
	addAndMakeVisible(fifoDrawerButton);

	masterLabel = new Label("masterLabel", "MASTER");
	masterLabel->setBounds(7, 21, 40, 20);
	masterLabel->setFont(Font("Small Text", 8.0f, Font::plain));
	addAndMakeVisible(masterLabel);

	masterMonitor = new FifoMonitor(recordNode, -1, -1);
	masterMonitor->setBounds(18, 43, 15, 62);
	addAndMakeVisible(masterMonitor);

	masterRecord = new RecordToggleButton("MasterRecord",-1,-1);
	masterRecord->setBounds(18, 110, 15, 15);
	masterRecord->addListener(this);
	addAndMakeVisible(masterRecord);

	/*
	engineSelectLabel = new Label("engineSelect", "ENGINE");
	engineSelectLabel->setBounds(36, 33, 80, 20);
	engineSelectLabel->setFont(Font("Small Text", 9.0f, Font::plain));
	addAndMakeVisible(engineSelectLabel);
	*/

	engineSelectCombo = new ComboBox("engineSelectCombo");
	engineSelectCombo->setBounds(42, 46, 93, 20);
	engineSelectCombo->addItem("Binary", 1);
	engineSelectCombo->addItem("OpenEphys", 2);
	engineSelectCombo->setSelectedId(1);
	addAndMakeVisible(engineSelectCombo);

	recordEventsLabel = new Label("recordEvents", "RECORD EVENTS");
	recordEventsLabel->setBounds(40, 71, 80, 20);
	recordEventsLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordEventsLabel);

	eventRecord = new RecordToggleButton("EventRecord",-1,-1);
	eventRecord->setBounds(120, 73, 15, 15);
	eventRecord->addListener(this);
	recordNode->setRecordEvents(false); //TODO: This needs to be loaded from save file
	addAndMakeVisible(eventRecord);

	recordSpikesLabel = new Label("recordSpikes", "RECORD SPIKES");
	recordSpikesLabel->setBounds(40, 88, 76, 20);
	recordSpikesLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordSpikesLabel);

	spikeRecord = new RecordToggleButton("SpikeRecord",-1,-1);
	spikeRecord->setBounds(120, 90, 15, 15);
	spikeRecord->addListener(this);
	addAndMakeVisible(spikeRecord);

	/*
	writeSpeedLabel = new Label("writeSpeedLabel", "WRITE SPEED");
	writeSpeedLabel->setBounds(40, 102, 76, 20);
	writeSpeedLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(writeSpeedLabel);
	*/

}

RecordNodeEditor::~RecordNodeEditor() {}

void RecordNodeEditor::timerCallback()
{}

void RecordNodeEditor::updateSubprocessorFifos()
{
	if (recordNode->getNumSubProcessors() != subProcMonitors.size())
	{

		subProcLabels.clear();
		subProcMonitors.clear();
		subProcRecords.clear();

		std::map<int, std::map<int, std::vector<bool>>>::iterator it;
		std::map<int, std::vector<bool>>::iterator ptr;

		int i = 0;
		for (it = recordNode->m.begin(); it != recordNode->m.end(); it++) {

			for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) {

				LOGD("First key is ", it->first, " And second key is ", ptr->first, " And value is ", ptr->second.size());

				String name = "(" + String(it->first) + "," + String(ptr->first) + ")";
				subProcLabels.add(new Label(name, name));
				subProcLabels.getLast()->setBounds(13 + i * 20, 24, 40, 20);
				subProcLabels.getLast()->setFont(Font("Small Text", 7.0f, Font::plain));
				addAndMakeVisible(subProcLabels.getLast());
				subProcLabels.getLast()->setVisible(false);

				subProcMonitors.add(new FifoMonitor(recordNode, it->first, ptr->first));
				subProcMonitors.getLast()->setBounds(18 + i * 20, 43, 15, 62);
				addAndMakeVisible(subProcMonitors.getLast());
				subProcMonitors.getLast()->setVisible(false);

				subProcRecords.add(new RecordToggleButton("RecSP" + String(i), it->first, ptr->first));
				subProcRecords.getLast()->setBounds(18 + i * 20, 110, 15, 15);
				subProcRecords.getLast()->addListener(this);
				addAndMakeVisible(subProcRecords.getLast());
				subProcRecords.getLast()->setVisible(false);

				i++;

			}
		}


	} 
}

void RecordNodeEditor::buttonEvent(Button *button)
{

	if (button == masterRecord) 
	{

	}
	else if (button == eventRecord)
	{
		recordNode->setRecordEvents(button->getToggleState());

	}
	else if (button == fifoDrawerButton)
	{

		updateSubprocessorFifos();
		if (button->getToggleState())
			showSubprocessorFifos(true);
		else
			showSubprocessorFifos(false);
	} 
	else if (subProcRecords.contains((RecordToggleButton*)button))
	{

		int subProcIdx = subProcRecords.indexOf((RecordToggleButton *)button);
		FifoMonitor* fifo = subProcMonitors[subProcIdx];
		bool enabled = button->getToggleState();
		fifo->channelStates.clear();
		int srcIndex = ((RecordToggleButton*)button)->srcIndex;
		int subIndex = ((RecordToggleButton*)button)->subProcIdx;
		for (int i = 0; i < recordNode->m[srcIndex][subIndex].size(); i++)
			fifo->channelStates.push_back(enabled);
		recordNode->updateChannelStates(((RecordToggleButton*)button)->srcIndex, ((RecordToggleButton*)button)->subProcIdx, fifo->channelStates);
	}
	
}

void RecordNodeEditor::collapsedStateChanged()
{

	if (getCollapsedState())
	{
		for (auto spl : subProcLabels)
			spl->setVisible(false);
		for (auto spm : subProcMonitors)
			spm->setVisible(false);
		for (auto spr : subProcRecords)
			spr->setVisible(false);
	} 
	else
	{
		for (auto spl : subProcLabels)
			spl->setVisible(subprocessorsVisible);
		for (auto spm : subProcMonitors)
			spm->setVisible(subprocessorsVisible);
		for (auto spr : subProcRecords)
			spr->setVisible(subprocessorsVisible);
	}
	
	
}

void RecordNodeEditor::showSubprocessorFifos(bool show)
{

	subprocessorsVisible = show;

	if (show)
		numSubprocessors = recordNode->getNumSubProcessors();

	int dX = 20 * (numSubprocessors + 1);
	dX = show ? dX : -dX;

	fifoDrawerButton->setBounds(
		fifoDrawerButton->getX() + dX, fifoDrawerButton->getY(),
		fifoDrawerButton->getWidth(), fifoDrawerButton->getHeight());

	masterLabel->setBounds(
		masterLabel->getX() + dX, masterLabel->getY(),
		masterLabel->getWidth(), masterLabel->getHeight());

	masterMonitor->setBounds(
		masterMonitor->getX() + dX, masterMonitor->getY(),
		masterMonitor->getWidth(), masterMonitor->getHeight());

	masterRecord->setBounds(
		masterRecord->getX() + dX, masterRecord->getY(),
		masterRecord->getWidth(), masterRecord->getHeight());

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

	for (auto spl : subProcLabels)
		spl->setVisible(show);
	for (auto spm : subProcMonitors)
		spm->setVisible(show);
	for (auto spr : subProcRecords)
		spr->setVisible(show);

	desiredWidth += dX;

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

	g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	g.drawVerticalLine(7, 0.0f, getHeight());
}

RecordToggleButton::RecordToggleButton(const String& name, int srcID, int subProcID) : Button(name) {
	setClickingTogglesState(true);
	srcIndex = srcID;
	subProcIdx = subProcID;
}

RecordToggleButton::~RecordToggleButton() {}

void RecordToggleButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

	g.setColour(Colour(0,0,0));
	g.fillRoundedRectangle(0,0,getWidth(),getHeight(),0.2*getWidth());

	if (isMouseOver)
	{
		g.setColour(Colour(210, 210, 210));
		if (getToggleState())
			g.setColour(Colour(255, 65, 65));
	}
	else
	{
		g.setColour(Colour(110, 110, 110));
		if (getToggleState())
			g.setColour(Colour(255, 0, 0));
	}
	g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

	/*Draw static black circle in center on top */
	g.setColour(Colour(0,0,0));
	g.fillEllipse(0.35*getWidth(), 0.35*getHeight(), 0.3*getWidth(), 0.3*getHeight());

}

FifoMonitor::FifoMonitor(RecordNode* node, int srcID, int subID) : recordNode(node), srcID(srcID), subID(subID), fillPercentage(0.0)
{

	startTimer(500);
}

/* RECORD CHANNEL SELECTOR LISTENER */
void FifoMonitor::mouseDoubleClick(const MouseEvent &event)
{

	if (srcID < 0) //TODO: Master box was selected; show read-only channel selector
		return;

	channelStates = recordNode->m[srcID][subID];
	
    auto* channelSelector = new RecordChannelSelector(channelStates);
 
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously (channelSelector, getScreenBounds(), nullptr);

	myBox.addComponentListener(this);

}
void FifoMonitor::componentBeingDeleted(Component &component)
{
	/*Capture button channel states and send back to record node. */

	auto* channelSelector = (RecordChannelSelector*)component.getChildComponent(0);

	channelStates.clear();
	for (auto* btn : channelSelector->channelButtons)
		channelStates.push_back(btn->getToggleState());

	recordNode->updateChannelStates(srcID, subID, channelStates);

	component.removeComponentListener(this);
}

void FifoMonitor::timerCallback()
{
	
	if (recordNode->recordThread->isThreadRunning())
	{
		//TODO: Get metric from recordThread
		setFillPercentage(0.0f);
	}
	else 
	{
		setFillPercentage(0.0f);
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
