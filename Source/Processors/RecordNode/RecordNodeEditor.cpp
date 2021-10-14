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

RecordNodeEditor::RecordNodeEditor(RecordNode* parentNode, bool useDefaultParameterEditors = true)
	: GenericEditor(parentNode, useDefaultParameterEditors),
	numSubprocessors(0), subprocessorsVisible(false)
{

	recordNode = parentNode;

	fifoDrawerButton = new FifoDrawerButton("FifoDrawer");
	fifoDrawerButton->setBounds(4, 40, 10, 78);
	fifoDrawerButton->addListener(this);
	addAndMakeVisible(fifoDrawerButton);

	masterLabel = new Label("masterLabel", "Avail:");
	masterLabel->setBounds(7, 21, 40, 20);
	masterLabel->setFont(Font("Small Text", 8.0f, Font::plain));
	//addAndMakeVisible(masterLabel);

	masterMonitor = new FifoMonitor(recordNode, -1);
	masterMonitor->setBounds(18, 33, 15, 92);
	addAndMakeVisible(masterMonitor);

	masterRecord = new RecordToggleButton(recordNode, "MasterRecord");
	masterRecord->setBounds(18, 110, 15, 15);
	masterRecord->addListener(this);
	//addAndMakeVisible(masterRecord);

	/*
	engineSelectLabel = new Label("engineSelect", "ENGINE");
	engineSelectLabel->setBounds(36, 33, 80, 20);
	engineSelectLabel->setFont(Font("Small Text", 9.0f, Font::plain));
	addAndMakeVisible(engineSelectLabel);
	*/

	dataPathLabel = new Label(CoreServices::getDefaultRecordingDirectory().getFullPathName());
	dataPathLabel->setText(CoreServices::getDefaultRecordingDirectory().getFullPathName(), juce::NotificationType::dontSendNotification);
	dataPathLabel->setTooltip(dataPathLabel->getText());
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
	for (int i = 0; i < engines.size(); i++)
	{
		engineSelectCombo->addItem(engines[i]->getName(), i + 1);
	}
	engineSelectCombo->setSelectedId(CoreServices::getDefaultRecordEngineIdx());
	engineSelectCombo->addListener(this);
	addAndMakeVisible(engineSelectCombo);

	recordEventsLabel = new Label("recordEvents", "RECORD EVENTS");
	recordEventsLabel->setBounds(40, 91, 80, 20);
	recordEventsLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordEventsLabel);

	eventRecord = new RecordToggleButton(recordNode, "EventRecord");
	eventRecord->setBounds(120, 93, 15, 15);
	eventRecord->addListener(this);
	eventRecord->setToggleState(1, sendNotification); //enable event recortding by default
	addAndMakeVisible(eventRecord);

	recordSpikesLabel = new Label("recordSpikes", "RECORD SPIKES");
	recordSpikesLabel->setBounds(40, 108, 76, 20);
	recordSpikesLabel->setFont(Font("Small Text", 10.0f, Font::plain));
	addAndMakeVisible(recordSpikesLabel);

	spikeRecord = new RecordToggleButton(recordNode, "SpikeRecord");
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

	startTimer(500);

}

RecordNodeEditor::~RecordNodeEditor() {}

void RecordNodeEditor::saveCustomParameters(XmlElement* xml) 
{  
	
	xml->setAttribute ("Type", "RecordNode");

    XmlElement* xmlNode = xml->createNewChildElement ("SETTINGS");
    xmlNode->setAttribute ("path", dataPathLabel->getText());
	//TODO: This should be the actual engine name instead of index in case engines added/removed between launches
    xmlNode->setAttribute ("engine", engineSelectCombo->getSelectedId());
	xmlNode->setAttribute ("recordEvents", eventRecord->getToggleState());
	xmlNode->setAttribute ("recordSpikes", spikeRecord->getToggleState());

	//Save channel states:
	for (auto streamId : extract_keys(recordNode->dataChannelStates))
	{

		if (recordNode->dataChannelStates[streamId].size() > 0)
		{
			XmlElement* stream = xmlNode->createNewChildElement("DATASTREAM");

			stream->setAttribute("stream_id", streamId);
			stream->setAttribute("isPrimary", 
				recordNode->synchronizer->primaryStreamId == streamId);
			stream->setAttribute("sync_bit", recordNode->syncChannelMap[streamId]);

			XmlElement* recStateNode = stream->createNewChildElement("RECORDSTATE");

			for (int ch = 0; ch < recordNode->dataChannelStates[streamId].size(); ch++)
			{
				recStateNode->setAttribute(String("CH")+String(ch), recordNode->dataChannelStates[streamId][ch]);
			}

		}


	}
	
}

void RecordNodeEditor::loadCustomParameters(XmlElement* xml) 
{

	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("SETTINGS"))
		{
			
			//Get saved record path
		    dataPathLabel->setText(xmlNode->getStringAttribute("path"), juce::NotificationType::sendNotification);
			engineSelectCombo->setSelectedId(xmlNode->getStringAttribute("engine").getIntValue());
			eventRecord->setToggleState((bool)(xmlNode->getStringAttribute("recordEvents").getIntValue()), juce::NotificationType::sendNotification);
			spikeRecord->setToggleState((bool)(xmlNode->getStringAttribute("recordSpikes").getIntValue()), juce::NotificationType::sendNotification);

			//std::cout << "Loading RecordNode settings" << std::endl;

			forEachXmlChildElement(*xmlNode, subNode)
			{
				if (subNode->hasTagName("DATASTREAM"))
				{

					int streamId = subNode->getIntAttribute("stream_id");

					if (recordNode->dataChannelStates[streamId].size())
					{

						if (subNode->getBoolAttribute("isPrimary"))
						{
							recordNode->setPrimaryDataStream(streamId);
						}

						recordNode->setSyncBit(streamId, subNode->getIntAttribute("sync_bit"));

						XmlElement* recordStates = subNode->getChildByName("RECORDSTATE");

						for (int ch = 0; ch < recordNode->dataChannelStates[streamId].size(); ch++)
						{
							//std::cout << "Setting channel " << ch << " : " << srcID << " : " << subIdx << " to " << recordStates->getIntAttribute("CH" + String(ch)) << std::endl;
							recordNode->dataChannelStates[streamId][ch] = recordStates->getIntAttribute("CH" + String(ch));
						}

					}

				}
			}

		}
	} 

}

int RecordNodeEditor::getSelectedEngineIdx()
{
	return engineSelectCombo->getSelectedId()-1;
}

void RecordNodeEditor::timerCallback()
{

	/* Disable buttons while recording */
	if (dataPathButton->isEnabled() && recordNode->recordThread->isThreadRunning())
	{
		dataPathButton->setEnabled(false);
		engineSelectCombo->setEnabled(false);
		eventRecord->setEnabled(false);
		spikeRecord->setEnabled(false);
	}
	else if (!dataPathButton->isEnabled() && !recordNode->recordThread->isThreadRunning())
	{
		dataPathButton->setEnabled(true);
		engineSelectCombo->setEnabled(true);
		eventRecord->setEnabled(true);
		spikeRecord->setEnabled(true);
	}

}

void RecordNodeEditor::comboBoxChanged(ComboBox* box)
{

	if (!recordNode->recordThread->isThreadRunning())
	{
		uint8 selectedEngineIndex = box->getSelectedId();

		//Prevent using OpenEphys format if > 300 channels coming into Record Node
		if (recordNode->getNumInputs() > 300 && selectedEngineIndex == 2)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
				"WARNING!", "Open Ephys format does not support > 300 channels. Resetting to Binary format");
			box->setSelectedItemIndex(0);
			recordNode->setEngine(0);
			return;
		}
		recordNode->setEngine(selectedEngineIndex-1);
	}

}

void RecordNodeEditor::updateSubprocessorFifos()
{
	if (recordNode->getNumDataStreams() != subProcMonitors.size())
	{

		subProcLabels.clear();
		subProcMonitors.clear();
		subProcRecords.clear();

		int streamCount = 0;

		for (auto const& [streamId, channelStates] : recordNode->dataChannelStates)
		{

			String name = "SP" + String(streamCount);
			subProcLabels.add(new Label(name, name));
			subProcLabels.getLast()->setBounds(13 + streamCount * 20, 24, 40, 20);
			subProcLabels.getLast()->setFont(Font("Small Text", 7.0f, Font::plain));
			addAndMakeVisible(subProcLabels.getLast());
			subProcLabels.getLast()->setVisible(false);

			subProcMonitors.add(new FifoMonitor(recordNode, streamId));
			subProcMonitors.getLast()->setBounds(18 + streamCount * 20, 43, 15, 62);
			addAndMakeVisible(subProcMonitors.getLast());
			subProcMonitors.getLast()->setVisible(false);

			LOGD("Adding sync control button for stream id: ", streamId);
			subProcRecords.add(new SyncControlButton(recordNode, "SP" + String(streamCount), streamId));
			subProcRecords.getLast()->setBounds(18 + streamCount * 20, 110, 15, 15);
			subProcRecords.getLast()->addListener(this);
			addAndMakeVisible(subProcRecords.getLast());
			subProcRecords.getLast()->setVisible(false);

			streamCount++;

		}
	} 
}

void RecordNodeEditor::buttonEvent(Button *button)
{

	if (button == masterRecord) 
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
		updateSubprocessorFifos();
		if (button->getToggleState())
			showSubprocessorFifos(true);
		else
			showSubprocessorFifos(false);
	} 
	else if (subProcRecords.contains((SyncControlButton*)button))
	{
		//Should be handled by SyncControlButton class
		/*
		int subProcIdx = subProcRecords.indexOf((SyncControlButton *)button);
		FifoMonitor* fifo = subProcMonitors[subProcIdx];
		bool enabled = button->getToggleState();
		fifo->channelStates.clear();
		int srcIndex = ((SyncControlButton*)button)->srcIndex;
		int subIndex = ((SyncControlButton*)button)->subProcIdx;
		for (int i = 0; i < recordNode->m[srcIndex][subIndex].size(); i++)
			fifo->channelStates.push_back(enabled);
		recordNode->updateChannelStates(((SyncControlButton*)button)->srcIndex, ((SyncControlButton*)button)->subProcIdx, fifo->channelStates);
		*/
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

void RecordNodeEditor::setDataDirectory(String dir)
{
	dataPathLabel->setText(dir, sendNotificationSync);
}

void RecordNodeEditor::showSubprocessorFifos(bool show)
{

	subprocessorsVisible = show;

	if (show)
		numSubprocessors = recordNode->getNumDataStreams();

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

SyncControlButton::SyncControlButton(RecordNode* _node, const String& name, uint64 streamId) 
	: Button(name),
	  streamId(streamId),
	  node(_node)
{

	isPrimary = node->isPrimaryDataStream(streamId);
	LOGD("Stream: ", streamId, " isPrimary: ", isPrimary);
    startTimer(100);
}

SyncControlButton::~SyncControlButton() {}

void SyncControlButton::timerCallback()
{
    repaint();
}

void SyncControlButton::componentBeingDeleted(Component &component)
{
	/*Capture button channel states and send back to record node. */

	auto* syncChannelSelector = (SyncChannelSelector*)component.getChildComponent(0);
	if (syncChannelSelector->isMaster)
	{
		LOGD("Set primary: {", streamId, "}");
		node->setPrimaryDataStream(streamId);
		isPrimary = true;
	}

	for (int i = 0; i < syncChannelSelector->buttons.size(); i++)
	{
		if (syncChannelSelector->buttons[i]->getToggleState())
		{
			node->setSyncBit(streamId, i);
			break;
		}

	}

	repaint();
}

void SyncControlButton::mouseUp(const MouseEvent &event)
{

	if (!node->recordThread->isThreadRunning() && event.mods.isLeftButtonDown())
	{

		const Array<EventChannel*> eventChannels = node->getDataStream(streamId)->getEventChannels();
		int nEvents = eventChannels[0]->getMaxTTLBits();
		int syncBit = node->getSyncBit(streamId);
		
		SyncChannelSelector* channelSelector = new SyncChannelSelector (nEvents, syncBit, node->isPrimaryDataStream(streamId));

		CallOutBox& myBox
			= CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);

		myBox.addComponentListener(this);

		return;

	}
}

void SyncControlButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),0.2*getWidth());

	g.setColour(Colour(110,110,110));
    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

	if (streamId > 0 && CoreServices::getAcquisitionStatus())
	{

		switch(node->synchronizer->getStatus(streamId)) {
                
            case SyncStatus::OFF :
            {
  
                if (isMouseOver)
                {
                    //LIGHT GREY
                    g.setColour(Colour(210, 210, 210));
                }
                else
                {
                    //DARK GREY
                    g.setColour(Colour(110, 110, 110));
                }
                break;
            }
            case SyncStatus::SYNCING :
            {

                if (isMouseOver)
                {
                    //LIGHT ORANGE
                   g.setColour(Colour(255,216,177));
                }
                else
                {
                    //DARK ORAN
                   g.setColour(Colour(255,165,0));
                }
                break;
            }
            case SyncStatus::SYNCED :
            {

                if (isMouseOver)
                {
                    //LIGHT GREEN
                    g.setColour(Colour(0, 255, 0));
                }
                else
                {
                    //DARK GREEN
                    g.setColour(Colour(20, 255, 20));
                }
                break;
            
            }
        }

	}
    
    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

	if (node->isPrimaryDataStream(streamId))
	{
		g.setColour(Colour(255,255,255));
		g.setFont(Font(10));
		g.drawText("M", 0, 0, getWidth(), getHeight(), juce::Justification::centred);
	}

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

FifoMonitor::FifoMonitor(RecordNode* node, uint64 streamId) : recordNode(node), streamId(streamId), fillPercentage(0.5)
{

	startTimer(500);
}

/* RECORD CHANNEL SELECTOR LISTENER */
void FifoMonitor::mouseDoubleClick(const MouseEvent &event)
{

	// Ignore right-clicks...add functionality for right-clicks later...
	if (event.mods.isRightButtonDown())
		return;

	if (streamId < 0) //TODO: Master box was selected; show read-only channel selector
		return;

	channelStates = recordNode->dataChannelStates[streamId];
	
	bool editable = !recordNode->recordThread->isThreadRunning();
    auto* channelSelector = new PopupChannelSelector(channelStates, Colour(255, 0, 0), editable);
 
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);

	myBox.addComponentListener(this);

}
void FifoMonitor::componentBeingDeleted(Component &component)
{
	/*Capture button channel states and send back to record node. */

	auto* channelSelector = (PopupChannelSelector*)component.getChildComponent(0);

	channelStates.clear();
	for (auto* btn : channelSelector->channelButtons)
		channelStates.push_back(btn->getToggleState());

	recordNode->updateChannelStates(streamId, channelStates);

	component.removeComponentListener(this);
}

void FifoMonitor::timerCallback()
{

	if (streamId < 0) /* Disk space monitor */
	{
		float diskSpace = (float)recordNode->getDataDirectory().getBytesFreeOnVolume() / recordNode->getDataDirectory().getVolumeTotalSize();

		if (diskSpace > 0)
			setFillPercentage(1.0f - diskSpace);
	}
	else /* Subprocessor monitor */
	{
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
