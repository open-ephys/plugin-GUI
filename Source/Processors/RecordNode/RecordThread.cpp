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

RecordThread::RecordThread(RecordNode* parentNode, RecordEngine* engine) :
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

void RecordThread::setEngine(RecordEngine* engine)
{
    m_engine = engine;
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

void RecordThread::setTimestampChannelMap(const Array<int>& channels)
{
	if (isThreadRunning())
		return;
	m_timestampBufferChannelArray = channels;
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
	const AudioBuffer<float>& dataBuffer = m_dataQueue->getContinuousDataBufferReference();
	const SynchronizedTimestampBuffer& ftsBuffer = m_dataQueue->getTimestampBufferReference();

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

void RecordThread::writeData(const AudioBuffer<float>& dataBuffer,
										 const SynchronizedTimestampBuffer& timestampBuffer,
										 int maxSamples,
										 int maxEvents,
									     int maxSpikes,
									     bool lastBlock)
{

	Array<int64> timestamps;
	Array<CircularBufferIndexes> dataBufferIdxs;
	Array<CircularBufferIndexes> timestampBufferIdxs;
	m_dataQueue->startRead(dataBufferIdxs, timestampBufferIdxs, timestamps, maxSamples);
	m_engine->updateTimestamps(timestamps);

	/* Copy data to record engine */
	for (int chan = 0; chan < m_numChannels; ++chan)
	{

		if (dataBufferIdxs[chan].size1 > 0)
		{
			const double* r = timestampBuffer.getReadPointer(m_timestampBufferChannelArray[chan],
				dataBufferIdxs[chan].index1);

			m_engine->writeContinuousData(
				chan,					 // write channel (index among all recorded channels)
				m_channelArray[chan],	 // real channel (index within processor)
				dataBuffer.getReadPointer(chan, dataBufferIdxs[chan].index1), // pointer to float
				r, // pointer to float
				dataBufferIdxs[chan].size1); // integer

			/*if (chan == 0)
			{

				std::cout << chan << " " << m_channelArray[chan] <<
					" " << dataBufferIdxs[chan].index1 <<
					" " << dataBufferIdxs[chan].size1 <<
					" " << dataBufferIdxs[chan].index2 <<
					" " << dataBufferIdxs[chan].size2
					<< " "
					<< *r << std::endl;

				std::cout << chan << " " << m_timestampBufferChannelArray[chan] <<
					" " << timestampBufferIdxs[m_timestampBufferChannelArray[chan]].index1 <<
					" " << timestampBufferIdxs[m_timestampBufferChannelArray[chan]].size1 <<
					" " << timestampBufferIdxs[m_timestampBufferChannelArray[chan]].index2 <<
					" " << timestampBufferIdxs[m_timestampBufferChannelArray[chan]].size2
					<< " "
					<< *r << std::endl;
			}*/


			samplesWritten += dataBufferIdxs[chan].size1;

			if (dataBufferIdxs[chan].size2 > 0)
			{
				timestamps.set(chan, timestamps[chan] + dataBufferIdxs[chan].size1);

				m_engine->updateTimestamps(timestamps, chan);

				m_engine->writeContinuousData(
					chan, 					// write channel (index among all recorded channels)
					m_channelArray[chan],	// real channel (index within processor)
					dataBuffer.getReadPointer(chan, dataBufferIdxs[chan].index2), // pointer to float
					timestampBuffer.getReadPointer(m_timestampBufferChannelArray[chan],
						dataBufferIdxs[chan].index2), // pointer to float
					dataBufferIdxs[chan].size2); // integer

				samplesWritten += dataBufferIdxs[chan].size2;
			}
		}
	}

	m_dataQueue->stopRead();

	std::vector<EventMessagePtr> events;
	int nEvents = m_eventQueue->getEvents(events, maxEvents);

	for (int ev = 0; ev < nEvents; ++ev)
	{

		const MidiMessage& event = events[ev]->getData();

		if (SystemEvent::getBaseType(event) == EventBase::Type::SYSTEM_EVENT)
		{
			String syncText = SystemEvent::getSyncText(event);
			std::cout << "Writing sync text: " << syncText << std::endl;
			m_engine->writeTimestampSyncText(SystemEvent::getStreamId(event), SystemEvent::getSampleNumber(event), 0.0f, SystemEvent::getSyncText(event));
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
