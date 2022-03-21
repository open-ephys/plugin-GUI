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
	LOGDD("Experiment number: ", experimentNumber, " Recording number: ", recordingNumber);
	m_experimentNumber = experimentNumber;
	m_recordingNumber = recordingNumber;
}

void RecordThread::setFTSChannelMap(const Array<int>& channels)
{
	if (isThreadRunning())
		return;
	m_ftsChannelArray = channels;
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
	const SynchronizedTimestampBuffer& ftsBuffer = m_dataQueue->getFTSBufferReference();

	spikesReceived = 0;
	spikesWritten = 0;

	bool closeEarly = true;

	//1-Wait until the first block has arrived, so we can align the timestamps
	bool isWaiting = false;
	while (!m_receivedFirstBlock && !threadShouldExit())
	{
		if (!isWaiting)
		{
			isWaiting = true;
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

		m_engine->updateTimestamps(timestamps);

		m_engine->openFiles(m_rootFolder, m_experimentNumber, m_recordingNumber);
	}
	//3-Normal loop
	while (!threadShouldExit())
		writeData(dataBuffer, ftsBuffer, BLOCK_MAX_WRITE_SAMPLES, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES);

	
	//LOGD(__FUNCTION__, " Exiting record thread");
	//4-Before closing the thread, try to write the remaining samples

	LOGD("Closing all files");
	
	if (!closeEarly)
	{
		// flush the buffers 
		writeData(dataBuffer, ftsBuffer, BLOCK_MAX_WRITE_SAMPLES, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES, true);

		//5-Close files
		m_engine->closeFiles();
	}
	m_cleanExit = true;
	m_receivedFirstBlock = false;

	//LOGC("RecordThread received ", spikesReceived, " spikes and wrote ", spikesWritten, ".");

}

void RecordThread::writeData(const AudioSampleBuffer& dataBuffer, 
										 const SynchronizedTimestampBuffer& ftsBuffer, 
										 int maxSamples, 
										 int maxEvents, 
									     int maxSpikes, 
									     bool lastBlock)
{

	Array<int64> timestamps;
	Array<CircularBufferIndexes> dataBufferIdxs;
	Array<CircularBufferIndexes> ftsBufferIdxs;
	m_dataQueue->startSynchronizedRead(dataBufferIdxs, ftsBufferIdxs, timestamps, maxSamples);
	m_engine->updateTimestamps(timestamps);

	/* Copy data to record engine */
	for (int chan = 0; chan < m_numChannels; ++chan)
	{

		if (dataBufferIdxs[chan].size1 > 0)
		{
			m_engine->writeContinuousData(chan, chan, 
				dataBuffer.getReadPointer(chan, dataBufferIdxs[chan].index1),
				ftsBuffer.getReadPointer(m_ftsChannelArray[chan], dataBufferIdxs[chan].index1), dataBufferIdxs[chan].size1);

			samplesWritten+=dataBufferIdxs[chan].size1;

			if (dataBufferIdxs[chan].size2 > 0)
			{
				timestamps.set(chan, timestamps[chan] + dataBufferIdxs[chan].size1);
				m_engine->updateTimestamps(timestamps, chan);
				m_engine->writeContinuousData(chan, chan,
					dataBuffer.getReadPointer(chan, dataBufferIdxs[chan].index2),
					ftsBuffer.getReadPointer(m_ftsChannelArray[chan], ftsBufferIdxs[m_ftsChannelArray[chan]].index2), dataBufferIdxs[chan].size2);
				samplesWritten += dataBufferIdxs[chan].size2;
			}
		}
	}

	m_dataQueue->stopSynchronizedRead();

	std::vector<EventMessagePtr> events;
	int nEvents = m_eventQueue->getEvents(events, maxEvents);

	for (int ev = 0; ev < nEvents; ++ev)
	{
		
		const MidiMessage& event = events[ev]->getData();
		
		if (SystemEvent::getBaseType(event) == EventBase::Type::SYSTEM_EVENT)
		{
			String syncText = SystemEvent::getSyncText(event);
			std::cout << "Writing sync text: " << syncText << std::endl;
			m_engine->writeTimestampSyncText(SystemEvent::getStreamId(event), SystemEvent::getTimestamp(event), 0.0f, SystemEvent::getSyncText(event));
		}
		else
		{
			int processorId = EventBase::getProcessorId(event);
			int streamId = EventBase::getStreamId(event);
			int channelIdx = EventBase::getChannelIndex(event);

			const EventChannel* chan = recordNode->getEventChannel(processorId, streamId, channelIdx);
			int eventIndex = recordNode->getIndexOfMatchingChannel(chan);

			m_engine->writeEvent(eventIndex, event);
		}
	}

	std::vector<SpikeMessagePtr> spikes;
	int nSpikes = m_spikeQueue->getEvents(spikes, BLOCK_MAX_WRITE_SPIKES);

	for (int sp = 0; sp < nSpikes; ++sp)
	{

		spikesReceived++;

		if (spikes[sp] != nullptr)
		{

			const Spike& spike = spikes[sp]->getData();
			const SpikeChannel* chan = spike.getChannelInfo();
			int spikeIndex = recordNode->getIndexOfMatchingChannel(chan);
			spikesWritten++;

			m_engine->writeSpike(spikeIndex, &spikes[sp]->getData());
		}
	}
}


void RecordThread::forceCloseFiles()
{
	if (isThreadRunning() || m_cleanExit)
		return;

	m_engine->closeFiles();
	m_cleanExit = true;
}