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



#include "../Channel/Channel.h"

RecordNode::RecordNode()
    : GenericProcessor("Record Node"),
      newDirectoryNeeded(true),  timestamp(0)
{

    isProcessing = false;
    isRecording = false;
	setFirstBlock = false;

    settings.numInputs = 2048;
    settings.numOutputs = 0;

    recordingNumber = -1;

    spikeElectrodeIndex = 0;

    experimentNumber = 0;
    hasRecorded = false;
    settingsNeeded = false;

    // 128 inputs, 0 outputs
    setPlayConfigDetails(getNumInputs(),getNumOutputs(),44100.0,128);
	m_recordThread = new RecordThread(engineArray);
	m_dataQueue = new DataQueue(WRITE_BLOCK_LENGTH, DATA_BUFFER_NBLOCKS);
	m_eventQueue = new EventMsgQueue(EVENT_BUFFER_NEVENTS);
	m_spikeQueue = new SpikeMsgQueue(SPIKE_BUFFER_NSPIKES);
	m_recordThread->setQueuePointers(m_dataQueue, m_eventQueue, m_spikeQueue);
}


RecordNode::~RecordNode()
{
}

void RecordNode::setChannel(Channel* ch)
{

    int channelNum = channelPointers.indexOf(ch);

    std::cout << "Record node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);

}

void RecordNode::setChannelStatus(Channel* ch, bool status)
{

    //std::cout << "Setting channel status!" << std::endl;
    setChannel(ch);

    if (status)
        setParameter(2, 1.0f);
    else
        setParameter(2, 0.0f);

}


void RecordNode::resetConnections()
{
    //std::cout << "Resetting connections" << std::endl;
    nextAvailableChannel = 0;
    wasConnected = false;
    spikeElectrodeIndex = 0;

    channelPointers.clear();
    eventChannelPointers.clear();
    spikeElectrodePointers.clear();

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

    for (int k = 0; k < channelPointers.size(); k++)
    {
        if (channelPointers[k] == nullptr)
        {
            names.add("not allocated");
            recording.add(false);

        }
        else
        {
            Channel* ch = channelPointers[k];
            String n = ch->name;
            names.add(n);
            recording.add(channelPointers[k]->getRecordState());
        }
    }
}

void RecordNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{

    if (chan != AccessClass::getProcessorGraph()->midiChannelIndex)
    {

        int channelIndex = getNextChannel(false);

        channelPointers.add(sourceNode->channels[chan]);
        setPlayConfigDetails(channelIndex+1,0,44100.0,128);

        //   std::cout << channelIndex << std::endl;

        channelPointers[channelIndex]->recordIndex = channelIndex;

        EVERY_ENGINE->addChannel(channelIndex,channelPointers[channelIndex]);

    }
    else
    {

        for (int n = 0; n < sourceNode->eventChannels.size(); n++)
        {

            eventChannelPointers.add(sourceNode->eventChannels[n]);

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

String RecordNode::generateDateString()
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

int RecordNode::getExperimentNumber()
{
	return experimentNumber;
}

int RecordNode::getRecordingNumber()
{
	return recordingNumber;
}

void RecordNode::setParameter(int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);

    // 0 = stop recording
    // 1 = start recording
    // 2 = toggle individual channel (0.0f = OFF, anything else = ON)

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
            AccessClass::getEditorViewport()->saveState(File(settingsFileName));
            settingsNeeded = false;
        }

		m_recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);

		channelMap.clear();
		int totChans = channelPointers.size();
		OwnedArray<RecordProcessorInfo> procInfo;
		Array<int> chanProcessorMap;
		Array<int> chanOrderinProc;
		int lastProcessor = -1;
		int procIndex = -1;
		int chanProcOrder = 0;
		for (int ch = 0; ch < totChans; ++ch)
		{
			Channel* chan = channelPointers[ch];
			if (chan->getRecordState())
			{
				channelMap.add(ch);
				//This is bassed on the assumption that all channels from the same processor are added contiguously
				//If this behaviour changes, this check should be most thorough
				if (chan->nodeId != lastProcessor)
				{
					lastProcessor = chan->nodeId;
					RecordProcessorInfo* pi = new RecordProcessorInfo();
					pi->processorId = chan->nodeId;
					procInfo.add(pi);
					procIndex++;
					chanProcOrder = 0;
				}
				procInfo.getLast()->recordedChannels.add(channelMap.size());
				chanProcessorMap.add(procIndex);
				chanOrderinProc.add(chanProcOrder);
				chanProcOrder++;
			}
		}
		std::cout << "Num Recording Processors: " << procInfo.size() << std::endl;
		int numRecordedChannels = channelMap.size();

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
                channelPointers[currentChannel]->setRecordState(false);
            }
            else
            {
                channelPointers[currentChannel]->setRecordState(true);
            }
        }
    }
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

float RecordNode::getFreeSpace()
{
    return 1.0f - float(dataDirectory.getBytesFreeOnVolume())/float(dataDirectory.getVolumeTotalSize());
}


void RecordNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
    if (isRecording)
    {
        if (isWritableEvent(eventType))
        {
            if (*(event.getRawData()+4) > 0) // saving flag > 0 (i.e., event has not already been processed)
            {
				uint8 sourceNodeId = event.getNoteNumber();
				int64 timestamp = timestamps[sourceNodeId] + samplePosition;
				m_eventQueue->addEvent(event, timestamp, eventType);
            }
        }
    }
}

void RecordNode::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events)
{
	
	// FIRST: cycle through events -- extract the TTLs and the timestamps
    checkForEvents(events);

    if (isRecording)
    {
        // SECOND: write channel data
		int recordChans = channelMap.size();
		for (int chan = 0; chan < recordChans; ++chan)
		{
			int realChan = channelMap[chan];
			int sourceNodeId = channelPointers[realChan]->sourceNodeId;
			int nSamples = numSamples.at(sourceNodeId);
			int timestamp = timestamps.at(sourceNodeId);
			m_dataQueue->writeChannel(buffer, chan, realChan, nSamples, timestamp);
		}

        //  std::cout << nSamples << " " << samplesWritten << " " << blockIndex << std::endl;
		if (!setFirstBlock)
		{
			m_recordThread->setFirstBlockFlag(true);
			setFirstBlock = true;
		}
        
    }


}

void RecordNode::registerProcessor(GenericProcessor* sourceNode)
{
    EVERY_ENGINE->registerProcessor(sourceNode);
}

Channel* RecordNode::getDataChannel(int index)
{
    return channelPointers[index];
}

void RecordNode::registerRecordEngine(RecordEngine* engine)
{
    engineArray.add(engine);
}

void RecordNode::registerSpikeSource(GenericProcessor* processor) 
{
    EVERY_ENGINE->registerSpikeSource(processor);
}

int RecordNode::addSpikeElectrode(SpikeRecordInfo* elec)
{
    elec->recordIndex=spikeElectrodeIndex;
    spikeElectrodePointers.add(elec);
    EVERY_ENGINE->addSpikeElectrode(spikeElectrodeIndex,elec);
    return spikeElectrodeIndex++;
}

void RecordNode::writeSpike(SpikeObject& spike, int electrodeIndex)
{
	if (isRecording)
	{
		m_spikeQueue->addEvent(spike, spike.timestamp, electrodeIndex);
	}
}

SpikeRecordInfo* RecordNode::getSpikeElectrode(int index)
{
    return spikeElectrodePointers[index];
}

void RecordNode::clearRecordEngines()
{
    engineArray.clear();
}
