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

#include "RecordThread.h"
//#include "RecordEngine.h"
//#include "../ProcessorGraph/ProcessorGraph.h"
#include "RecordNode.h"

//#define EVERY_ENGINE for(int eng = 0; eng < m_engineArray.size(); eng++) m_engineArray[eng]
#define EVERY_ENGINE m_engine;

RecordThread::RecordThread(RecordNode* parentNode, const ScopedPointer<RecordEngine>& engine) :
Thread("Record Thread"),
m_engine(engine),
recordNode(parentNode),
m_receivedFirstBlock(false),
m_cleanExit(true),
samplesWritten(0)
{
}

RecordThread::~RecordThread()
{
}

void RecordThread::setFileComponents(File rootFolder, int experimentNumber, int recordingNumber)
{
	if (isThreadRunning())
	{
		LOGD(__FUNCTION__, " Tried to set file components while thread was running!");
		return;
	}

	m_rootFolder = rootFolder;
	LOGD(__FUNCTION__, " Experiment number: ", experimentNumber, " Recording number: ", recordingNumber);
	m_experimentNumber = experimentNumber;
	m_recordingNumber = recordingNumber;
}

void RecordThread::setChannelMap(const Array<int>& channels)
{
	if (isThreadRunning())
		return;
	m_channelArray = channels;
	m_numChannels = channels.size();
	
}

void RecordThread::setQueuePointers(DataQueue* data, EventMsgQueue* events, SpikeMsgQueue* spikes)
{
	m_dataQueue = data;
	m_eventQueue = events;
	m_spikeQueue = spikes;
}

void RecordThread::setFirstBlockFlag(bool state)
{
	m_receivedFirstBlock = state;
	this->notify();
}

void RecordThread::run()
{
	const AudioSampleBuffer& dataBuffer = m_dataQueue->getAudioBufferReference();
	bool closeEarly = true;
	//1-Wait until the first block has arrived, so we can align the timestamps
	bool isWaiting = false;
	while (!m_receivedFirstBlock && !threadShouldExit())
	{
		if (!isWaiting)
		{
			isWaiting = true;
			LOGD(__FUNCTION__, " Waiting for first block...");
		}
		wait(1);
	}

	//2-Open Files 
	if (!threadShouldExit())
	{
		m_cleanExit = false;
		closeEarly = false;
		Array<int64> timestamps;
		m_dataQueue->getTimestampsForBlock(0, timestamps);

		//EVERY_ENGINE->updateTimestamps(timestamps);
		m_engine->updateTimestamps(timestamps);
		//EVERY_ENGINE->openFiles(m_rootFolder, m_experimentNumber, m_recordingNumber);
		LOGD(__FUNCTION__, " Opening files w/ experiment number: ", m_experimentNumber);
		m_engine->openFiles(m_rootFolder, m_experimentNumber, m_recordingNumber);
	}
	//3-Normal loop
	while (!threadShouldExit())
	{
		writeData(dataBuffer, BLOCK_MAX_WRITE_SAMPLES, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES);
	}
	LOGD(__FUNCTION__, " Exiting record thread");
	//4-Before closing the thread, try to write the remaining samples
	if (!closeEarly)
	{
		writeData(dataBuffer, -1, -1, -1, true);
		LOGD(__FUNCTION__, " Closing files");
		//5-Close files
		//EVERY_ENGINE->closeFiles();
		m_engine->closeFiles();
	}
	m_cleanExit = true;
	m_receivedFirstBlock = false;
}

void RecordThread::writeData(const AudioSampleBuffer& dataBuffer, int maxSamples, int maxEvents, int maxSpikes, bool lastBlock)
{
	Array<int64> timestamps;
	Array<CircularBufferIndexes> idx;
	m_dataQueue->startRead(idx, timestamps, maxSamples);
	//EVERY_ENGINE->updateTimestamps(timestamps);
	m_engine->updateTimestamps(timestamps);
	//EVERY_ENGINE->startChannelBlock(lastBlock);
	m_engine->startChannelBlock(lastBlock);
	for (int chan = 0; chan < m_numChannels; ++chan)
	{
		if (idx[chan].size1 > 0)
		{
			//EVERY_ENGINE->writeData(chan, m_channelArray[chan], dataBuffer.getReadPointer(chan, idx[chan].index1), idx[chan].size1);
			m_engine->writeData(chan, chan, dataBuffer.getReadPointer(chan, idx[chan].index1), idx[chan].size1);
			samplesWritten+=idx[chan].size1;
			if (idx[chan].size2 > 0)
			{
				timestamps.set(chan, timestamps[chan] + idx[chan].size1);
				//EVERY_ENGINE->updateTimestamps(timestamps, chan);
				m_engine->updateTimestamps(timestamps, chan);
				//EVERY_ENGINE->writeData(chan, m_channelArray[chan], dataBuffer.getReadPointer(chan, idx[chan].index2), idx[chan].size2);
				m_engine->writeData(chan, chan, dataBuffer.getReadPointer(chan, idx[chan].index2), idx[chan].size2);
				samplesWritten += idx[chan].size2;
			}
		}
	}
	m_dataQueue->stopRead();
	//EVERY_ENGINE->endChannelBlock(lastBlock);
	m_engine->endChannelBlock(lastBlock);

	std::vector<EventMessagePtr> events;
	int nEvents = m_eventQueue->getEvents(events, maxEvents);
	if (nEvents > 0)
	{
		std::cout << "[RN] nEvents = " << nEvents << std::endl;
	}
	for (int ev = 0; ev < nEvents; ++ev)
	{
		const MidiMessage& event = events[ev]->getData();
		if (SystemEvent::getBaseType(event) == SYSTEM_EVENT)
		{
			uint16 sourceID = SystemEvent::getSourceID(event);
			uint16 subProcIdx = SystemEvent::getSubProcessorIdx(event);
			int64 timestamp = SystemEvent::getTimestamp(event);
			m_engine->writeTimestampSyncText(sourceID, subProcIdx, timestamp,
				recordNode->getSourceTimestamp(sourceID, subProcIdx),
				SystemEvent::getSyncText(event));
		}
		else
			m_engine->writeEvent(events[ev]->getExtra(), events[ev]->getData());
	}

	std::vector<SpikeMessagePtr> spikes;
	int nSpikes = m_spikeQueue->getEvents(spikes, maxSpikes);
	for (int sp = 0; sp < nSpikes; ++sp)
	{
		m_engine->writeSpike(spikes[sp]->getExtra(), &spikes[sp]->getData());
	}
	
}

void RecordThread::forceCloseFiles()
{
	if (isThreadRunning() || m_cleanExit)
		return;

	//EVERY_ENGINE->closeFiles();
	m_engine->closeFiles();
	m_cleanExit = true;
}