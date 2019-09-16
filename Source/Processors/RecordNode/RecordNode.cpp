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

#include "RecordNode.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "../../AccessClass.h"
#include "RecordEngine.h"
#include "RecordThread.h"
#include "DataQueue.h"

#define EVERY_ENGINE for(int eng = 0; eng < engineArray.size(); eng++) engineArray[eng]


RecordNode::RecordNode()
    : GenericProcessor("Record Node"),
      newDirectoryNeeded(true),  timestamp(0)
{

    isProcessing = false;
    isRecording = false;
	shouldRecord = true;
	setFirstBlock = false;

    settings.numInputs = 0;
    settings.numOutputs = 0;
    settings.originalSource = nullptr;

    recordingNumber = -1;

    spikeElectrodeIndex = 0;

    experimentNumber = 0;
    hasRecorded = false;
    settingsNeeded = false;

	m_recordThread = new RecordThread(engineArray);
	m_dataQueue = new DataQueue(WRITE_BLOCK_LENGTH, DATA_BUFFER_NBLOCKS);
	m_eventQueue = new EventMsgQueue(EVENT_BUFFER_NEVENTS);
	m_spikeQueue = new SpikeMsgQueue(SPIKE_BUFFER_NSPIKES);
	m_recordThread->setQueuePointers(m_dataQueue, m_eventQueue, m_spikeQueue);
}


RecordNode::~RecordNode()
{
}

void RecordNode::setChannel(const DataChannel* ch)
{
	int channelNum = getDataChannelIndex(ch->getSourceIndex(), ch->getSourceNodeID(), ch->getSubProcessorIdx());

    std::cout << "Record node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);

}

void RecordNode::resetConnections()
{
    nextAvailableChannel = 0;
    wasConnected = false;
    spikeElectrodeIndex = 0;
    settings.numInputs = 0;

    dataChannelArray.clear();
    eventChannelArray.clear();
    spikeChannelArray.clear();

    EVERY_ENGINE->resetChannels();

}

void RecordNode::filenameComponentChanged(FilenameComponent* fnc)
{

    dataDirectory = fnc->getCurrentFile();
	newDirectoryNeeded = true;

}


void RecordNode::getChannelNamesAndRecordingStatus(StringArray& names, Array<bool>& recording)
{
    names.clear();
    recording.clear();

    for (int k = 0; k < dataChannelArray.size(); k++)
    {
        if (dataChannelArray[k] == nullptr)
        {
            names.add("not allocated");
            recording.add(false);

        }
        else
        {
            DataChannel* ch = dataChannelArray[k];
            String n = ch->getName();
            names.add(n);
            recording.add(dataChannelArray[k]->getRecordState());
        }
    }
}

void RecordNode::addInputChannel(const GenericProcessor* sourceNode, int chan)
{

    if (chan != AccessClass::getProcessorGraph()->midiChannelIndex)
    {
        int channelIndex = getNextChannel(false);

        const DataChannel* orig = sourceNode->getDataChannel(chan);
        DataChannel* newChannel = new DataChannel(*orig);
        newChannel->setRecordState(orig->getRecordState());
        dataChannelArray.add(newChannel);


        EVERY_ENGINE->addDataChannel(channelIndex,dataChannelArray[channelIndex]);

    }
    else
    {

        for (int n = 0; n < sourceNode->getTotalEventChannels(); n++)
        {
			const EventChannel* orig = sourceNode->getEventChannel(n);
			//only add to the record node the events originating from this processor, to avoid duplicates
			if (orig->getSourceNodeID() == sourceNode->getNodeId())
				eventChannelArray.add(new EventChannel(*orig));

        }

    }

}

void RecordNode::createNewDirectory()
{
    std::cout << "Creating new directory." << std::endl;

    rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName());
    newDirectoryNeeded = false;

}

String RecordNode::generateDirectoryName()
{
    Time calendar = Time::getCurrentTime();

    Array<int> t;
    t.add(calendar.getYear());
    t.add(calendar.getMonth()+1); // January = 0
    t.add(calendar.getDayOfMonth());
    t.add(calendar.getHours());
    t.add(calendar.getMinutes());
    t.add(calendar.getSeconds());

    String filename = AccessClass::getControlPanel()->getTextToPrepend();

    String datestring = "";

    for (int n = 0; n < t.size(); n++)
    {
        if (t[n] < 10)
            datestring += "0";

        datestring += t[n];

        if (n == 2)
            datestring += "_";
        else if (n < 5)
            datestring += "-";
    }

    AccessClass::getControlPanel()->setDateText(datestring);

    filename += datestring;
    filename += AccessClass::getControlPanel()->getTextToAppend();

    return filename;

}

String RecordNode::generateDateString() const
{
    Time calendar = Time::getCurrentTime();

    String datestring;

    datestring += String(calendar.getDayOfMonth());
    datestring += "-";
    datestring += calendar.getMonthName(true);
    datestring += "-";
    datestring += String(calendar.getYear());
    datestring += " ";

    int hrs, mins, secs;
    hrs = calendar.getHours();
    mins = calendar.getMinutes();
    secs = calendar.getSeconds();

    datestring += hrs;

    if (mins < 10)
        datestring += 0;

    datestring += mins;

    if (secs < 0)
        datestring += 0;

    datestring += secs;

    return datestring;

}

int RecordNode::getExperimentNumber() const
{
	return experimentNumber;
}

int RecordNode::getRecordingNumber() const
{
	return recordingNumber;
}

void RecordNode::setParameter(int parameterIndex, float newValue)
{
	//editor->updateParameterButtons(parameterIndex);

	// 0 = stop recording
	// 1 = start recording
	// 2 = toggle individual channel (0.0f = OFF, anything else = ON)
	// 3 = toggle record thread ON/OFF (use carefully!)

	if (parameterIndex == 1)
	{


		// std::cout << "START RECORDING." << std::endl;

		if (newDirectoryNeeded)
		{
			createNewDirectory();
			recordingNumber = 0;
			experimentNumber = 1;
			settingsNeeded = true;
			EVERY_ENGINE->directoryChanged();
		}
		else
		{
			recordingNumber++; // increment recording number within this directory
		}

		if (!rootFolder.exists())
		{
			rootFolder.createDirectory();
		}
		if (settingsNeeded)
		{
			String settingsFileName = rootFolder.getFullPathName() + File::separator + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String::empty) + ".xml";
			AccessClass::getEditorViewport()->saveState(File(settingsFileName), m_lastSettingsText);
			settingsNeeded = false;
		}

		m_recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);

		channelMap.clear();
		int totChans = dataChannelArray.size();
		OwnedArray<RecordProcessorInfo> procInfo;
		Array<int> chanProcessorMap;
		Array<int> chanOrderinProc;
		int lastProcessor = -1;
		int procIndex = -1;
		int chanProcOrder = 0;
		for (int ch = 0; ch < totChans; ++ch)
		{
			DataChannel* chan = dataChannelArray[ch];
			if (chan->getRecordState())
			{
				channelMap.add(ch);
				//This is bassed on the assumption that all channels from the same processor are added contiguously
				//If this behaviour changes, this check should be most thorough
				if (chan->getCurrentNodeID() != lastProcessor)
				{
					lastProcessor = chan->getCurrentNodeID();
					RecordProcessorInfo* pi = new RecordProcessorInfo();
					pi->processorId = chan->getCurrentNodeID();
					procInfo.add(pi);
					procIndex++;
					chanProcOrder = 0;
				}
				procInfo.getLast()->recordedChannels.add(channelMap.size() - 1);
				chanProcessorMap.add(procIndex);
				chanOrderinProc.add(chanProcOrder);
				chanProcOrder++;
			}
		}
		std::cout << "Num Recording Processors: " << procInfo.size() << std::endl;
		int numRecordedChannels = channelMap.size();

		m_validBlocks.clear();
		m_validBlocks.insertMultiple(0, false, numRecordedChannels);

		//WARNING: If at some point we record at more that one recordEngine at once, we should change this, as using OwnedArrays only works for the first
		EVERY_ENGINE->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
		m_recordThread->setChannelMap(channelMap);
		m_dataQueue->setChannels(numRecordedChannels);
		m_eventQueue->reset();
		m_spikeQueue->reset();
		m_recordThread->setFirstBlockFlag(false);

		setFirstBlock = false;
		m_recordThread->startThread();

		isRecording = true;
		hasRecorded = true;

	}
	else if (parameterIndex == 0)
	{


		std::cout << "STOP RECORDING." << std::endl;

		if (isRecording)
		{
			isRecording = false;

			// close the writing thread.
			m_recordThread->signalThreadShouldExit();
			m_recordThread->waitForThreadToExit(2000);
			while (m_recordThread->isThreadRunning())
			{
				std::cerr << "RecordEngine timeout" << std::endl;
				if (AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Record Thread timeout",
					"The recording thread is taking too long to close.\nThis could mean there is still data waiting to be written in the buffer, but it normally "
					"shouldn't take this long.\nYou can either wait a bit more or forcefully close the thread. Note that data might be lost or corrupted"
					"if forcibly closing the thread.", "Stop the thread", "Wait a bit more"))
				{
					m_recordThread->stopThread(100);
					m_recordThread->forceCloseFiles();
				}
				else
				{
					m_recordThread->waitForThreadToExit(2000);
				}
			}

		}
	}
	else if (parameterIndex == 2)
	{

		if (isProcessing)
		{

			std::cout << "Toggling channel " << currentChannel << std::endl;

			if (isRecording)
			{
				//Toggling channels while recording isn't allowed. Code shouldn't reach here.
				//In case it does, display an error and exit.
				CoreServices::sendStatusMessage("Toggling record channels while recording is not allowed");
				std::cout << "ERROR: Wrong code section reached\n Toggling record channels while recording is not allowed." << std::endl;
				return;
			}

			if (newValue == 0.0f)
			{
				dataChannelArray[currentChannel]->setRecordState(false);
			}
			else
			{
				dataChannelArray[currentChannel]->setRecordState(true);
			}
		}
	}
	else if (parameterIndex == 3)
	{

		if (isRecording)
		{
			//Toggling thread status while recording isn't allowed.
			//In case it does, display an error and exit.
			CoreServices::sendStatusMessage("Changing record thread status while recording is not allowed");
			return;
		}

		if (newValue == 0.0f)
		{
			CoreServices::sendStatusMessage("Turning record thread off.");
			shouldRecord = false;
			
		}
		else
		{
			CoreServices::sendStatusMessage("Turning record thread on.");
			shouldRecord = true;
		}
	}
}

bool RecordNode::getRecordThreadStatus()
{
	return shouldRecord;
}

bool RecordNode::enable()
{
    if (hasRecorded)
    {
        hasRecorded = false;
        experimentNumber++;
        settingsNeeded = true;
    }

    //When starting a recording, if a new directory is needed it gets rewritten. Else is incremented by one.
    recordingNumber = -1;
    EVERY_ENGINE->configureEngine();
    EVERY_ENGINE->startAcquisition();
    isProcessing = true;
    return true;
}


bool RecordNode::disable()
{
    // close files if necessary
    setParameter(0, 10.0f);

    isProcessing = false;

    return true;
}

float RecordNode::getFreeSpace() const
{
    return 1.0f - float(dataDirectory.getBytesFreeOnVolume())/float(dataDirectory.getVolumeTotalSize());
}


void RecordNode::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{
    if (true)
    {

            if ((*(event.getRawData()+0) & 0x80) == 0) // saving flag > 0 (i.e., event has not already been processed)
            {
				int64 timestamp = Event::getTimestamp(event);
				int eventIndex;
				if (eventInfo)
					eventIndex = getEventChannelIndex(Event::getSourceIndex(event), Event::getSourceID(event), Event::getSubProcessorIdx(event));
				else
					eventIndex = -1;
				if (isRecording && shouldRecord)
					m_eventQueue->addEvent(event, timestamp, eventIndex);
            }
    }
}

void RecordNode::handleTimestampSyncTexts(const MidiMessage& event)
{
	handleEvent(nullptr, event, 0);
}

void RecordNode::process(AudioSampleBuffer& buffer)
{
	
	// FIRST: cycle through events -- extract the TTLs and the timestamps
    checkForEvents();

    if (isRecording && shouldRecord)
    {
        // SECOND: write channel data
		int recordChans = channelMap.size();
		for (int chan = 0; chan < recordChans; ++chan)
		{
			int realChan = channelMap[chan];
			int nSamples = getNumSamples(realChan);
			int timestamp = getTimestamp(realChan);
			bool shouldWrite = m_validBlocks[chan];
			if (!shouldWrite && nSamples > 0)
			{
				shouldWrite = true;
				m_validBlocks.set(chan, true);
			}

			if (shouldWrite)
				m_dataQueue->writeChannel(buffer, chan, realChan, nSamples, timestamp);
		}

        //  std::cout << nSamples << " " << samplesWritten << " " << blockIndex << std::endl;
		if (!setFirstBlock)
		{
			bool shouldSetFlag = true;
			for (int chan = 0; chan < recordChans; ++chan)
			{
				if (!m_validBlocks[chan])
				{
					shouldSetFlag = false;
					break;
				}
			}
			if (shouldSetFlag)
			{
				m_recordThread->setFirstBlockFlag(true);
				setFirstBlock = true;
			}
		}
        
    }


}

void RecordNode::registerProcessor(const GenericProcessor* sourceNode)
{
    settings.numInputs += sourceNode->getNumOutputs();
    setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

    EVERY_ENGINE->registerProcessor(sourceNode);
}


void RecordNode::registerRecordEngine(RecordEngine* engine)
{
    engineArray.add(engine);
}

void RecordNode::registerSpikeSource(const GenericProcessor* processor) 
{
    EVERY_ENGINE->registerSpikeSource(processor);
}

int RecordNode::addSpikeElectrode(const SpikeChannel* elec)
{
    spikeChannelArray.add(new SpikeChannel(*elec));
    EVERY_ENGINE->addSpikeElectrode(spikeElectrodeIndex,elec);
    return spikeElectrodeIndex++;
}

void RecordNode::writeSpike(const SpikeEvent* spike, const SpikeChannel* spikeElectrode)
{
	if (isRecording && shouldRecord)
	{
		int electrodeIndex = getSpikeChannelIndex(spikeElectrode->getSourceIndex(), spikeElectrode->getSourceNodeID(), spikeElectrode->getSubProcessorIdx());
		if (electrodeIndex >= 0)
			m_spikeQueue->addEvent(*spike, spike->getTimestamp(), electrodeIndex);
	}
}

void RecordNode::clearRecordEngines()
{
    engineArray.clear();
}

const String& RecordNode::getLastSettingsXml() const
{
	return m_lastSettingsText;
}

File RecordNode::getDataDirectory() const
{
	return rootFolder;
}

void RecordNode::updateRecordChannelIndexes()
{
	//Keep the nodeIDs of the original processor from each channel comes from
	updateChannelIndexes(false);
}

void RecordNode::addSpecialProcessorChannels(Array<EventChannel*>& channels)
{
	eventChannelArray.addArray(channels);
	settings.numInputs = dataChannelArray.size();
	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 1024);
}