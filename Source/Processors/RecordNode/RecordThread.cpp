/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

RecordThread::RecordThread (RecordNode* parentNode, RecordEngine* engine) : Thread ("Record Thread"),
                                                                            m_engine (engine),
                                                                            recordNode (parentNode),
                                                                            m_receivedFirstBlock (false),
                                                                            m_cleanExit (true),
                                                                            m_minWriteSamples (BLOCK_DEFAULT_MIN_WRITE_SAMPLES),
                                                                            m_maxWriteSamples (BLOCK_DEFAULT_MAX_WRITE_SAMPLES)
{
    // Ensure max >= min to avoid deadlock
    if (m_maxWriteSamples < m_minWriteSamples)
    {
        LOGC ("WARNING: MAX_WRITE_SAMPLES (", m_maxWriteSamples, ") < MIN_WRITE_SAMPLES (", m_minWriteSamples, "), setting MAX = MIN");
        m_maxWriteSamples = m_minWriteSamples;
    }

    if (m_engine == nullptr || ! m_engine->getEngineId().equalsIgnoreCase ("BINARY"))
    {
        useBatchWrites = false;
        LOGD ("[RecordThread] Batch writes disabled - using single-sample writes for compatibility with engines other than BINARY");
    }

    LOGD ("RecordThread initialized with MIN_WRITE_SAMPLES=", m_minWriteSamples, " MAX_WRITE_SAMPLES=", m_maxWriteSamples);
}

RecordThread::~RecordThread()
{
}

void RecordThread::setEngine (RecordEngine* engine)
{
    m_engine = engine;

    if (m_engine == nullptr || ! m_engine->getEngineId().equalsIgnoreCase ("BINARY"))
    {
        useBatchWrites = false;
        LOGD ("[RecordThread] Batch writes disabled - using single-sample writes for compatibility with engines other than BINARY");
    }
    else
    {
        useBatchWrites = true;
        LOGD ("[RecordThread] Batch writes enabled for engine: ", m_engine->getEngineId());
    }
}

void RecordThread::setFileComponents (File rootFolder, int experimentNumber, int recordingNumber)
{
    if (isThreadRunning())
    {
        LOGD (__FUNCTION__, " Tried to set file components while thread was running!");
        return;
    }

    m_rootFolder = rootFolder;
    LOGD ("RecordThread::setFileComponents - Experiment number: ", experimentNumber, " Recording number: ", recordingNumber);
    m_experimentNumber = experimentNumber;
    m_recordingNumber = recordingNumber;
}

void RecordThread::setTimestampChannelMap (const Array<int>& channels)
{
    if (isThreadRunning())
        return;
    m_timestampBufferChannelArray = channels;
}

void RecordThread::setChannelMap (const Array<int>& channels)
{
    if (isThreadRunning())
        return;
    m_channelArray = channels;
    m_numChannels = channels.size();
}

void RecordThread::setQueuePointers (OwnedArray<DataQueue>* dataQueues, EventMsgQueue* events, SpikeMsgQueue* spikes)
{
    m_dataQueues = dataQueues;
    m_eventQueue = events;
    m_spikeQueue = spikes;
}

void RecordThread::setFirstBlockFlag (bool state)
{
    m_receivedFirstBlock = state;
    this->notify();
}

void RecordThread::run()
{
    // Initialize counters
    spikesReceived = 0;
    spikesWritten = 0;

    // Initialize per-channel sample numbers
    sampleNumbers.clear();
    for (int chan = 0; chan < m_numChannels; ++chan)
    {
        sampleNumbers.add (0);
    }

    // Pre-allocate per-stream buffer index arrays
    int numStreams = recordNode->getNumDataStreams();
    m_perStreamDataIdxs.resize (numStreams);
    m_perStreamTimestampIdxs.resize (numStreams);
    m_perStreamSampleNumbers.resize (numStreams, 0);

    // 1 - Open files
    bool closeEarly = false;
    m_cleanExit = false;
    Array<int64> initSampleNumbers;

    m_engine->openFiles (m_rootFolder, m_experimentNumber, m_recordingNumber);

    if (recordNode != nullptr)
        recordNode->notifyRecordThreadFilesOpened();

    //2-Wait until the first block has arrived, so we can align the timestamps
    bool isWaiting = false;
    while (! m_receivedFirstBlock && ! threadShouldExit())
    {
        if (! isWaiting)
        {
            isWaiting = true;
        }
        wait (1);
    }

    LOGD ("RecordThread received first block, starting main loop...");

    // 3 - Get initial sample numbers from each stream's queue
    int globalChan = 0;
    for (int streamIdx = 0; streamIdx < numStreams; streamIdx++)
    {
        DataQueue* queue = (*m_dataQueues)[streamIdx];
        if (queue != nullptr)
        {
            int64 streamSampleNum = queue->getSampleNumberForBlock (0);
            m_perStreamSampleNumbers[streamIdx] = streamSampleNum;

            // Count channels in this stream and set sample numbers for all of them
            int numChannelsInStream = 0;
            for (int ch = 0; ch < m_numChannels; ch++)
            {
                if (m_timestampBufferChannelArray[ch] == streamIdx)
                    numChannelsInStream++;
            }

            for (int i = 0; i < numChannelsInStream; i++)
            {
                if (globalChan < m_numChannels)
                    initSampleNumbers.add (streamSampleNum);
                globalChan++;
            }
        }
    }
    m_engine->updateLatestSampleNumbers (initSampleNumbers);

    //4 - Normal loop
    while (! threadShouldExit())
        writeData (m_minWriteSamples, m_maxWriteSamples, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES);

    //LOGD(__FUNCTION__, " Exiting record thread");
    //5 - Before closing the thread, try to write the remaining samples

    LOGD ("Closing all files");

    if (! closeEarly)
    {
        // flush the buffers - use 0 for minSamples to write all remaining data
        writeData (0, m_maxWriteSamples, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES, true);

        //5-Close files
        m_engine->closeFiles();
    }
    m_cleanExit = true;
    m_receivedFirstBlock = false;

    //LOGC("RecordThread received ", spikesReceived, " spikes and wrote ", spikesWritten, ".");
}

void RecordThread::writeData (int minSamples,
                              int maxSamples,
                              int maxEvents,
                              int maxSpikes,
                              bool lastBlock)
{
    int numStreams = m_dataQueues->size();
    int globalChannelOffset = 0;

    // Use sample-based thresholds directly - they naturally scale with channel count:
    // High channels = large byte writes (efficient), Low channels = small byte writes (fine)
    int effectiveMinSamples = lastBlock ? 0 : minSamples;
    int effectiveMaxSamples = maxSamples;

    // Process each stream independently - this avoids race conditions
    // between streams with different sample rates
    for (int streamIdx = 0; streamIdx < numStreams; streamIdx++)
    {
        DataQueue* queue = (*m_dataQueues)[streamIdx];
        if (queue == nullptr)
            continue;

        const AudioBuffer<float>& dataBuffer = queue->getContinuousDataBufferReference();
        const SynchronizedTimestampBuffer& timestampBuffer = queue->getTimestampBufferReference();

        // Ensure per-stream index arrays are properly sized
        int numChannelsInStream = 0;
        for (int ch = 0; ch < m_numChannels; ch++)
        {
            if (m_timestampBufferChannelArray[ch] == streamIdx)
                numChannelsInStream++;
        }

        m_perStreamDataIdxs[streamIdx].resize (numChannelsInStream);
        m_perStreamTimestampIdxs[streamIdx].resize (1); // One timestamp stream per queue

        // Get single sample number for this stream
        int64 streamSampleNumber = 0;

        if (queue->startRead (m_perStreamDataIdxs[streamIdx], m_perStreamTimestampIdxs[streamIdx], streamSampleNumber, effectiveMinSamples, effectiveMaxSamples))
        {
            // Update global sample numbers using the stream's sample number
            m_perStreamSampleNumbers[streamIdx] = streamSampleNumber;
            for (int globalChan = 0; globalChan < m_numChannels; globalChan++)
            {
                if (m_timestampBufferChannelArray[globalChan] == streamIdx)
                {
                    sampleNumbers.set (globalChan, streamSampleNumber);
                }
            }
            m_engine->updateLatestSampleNumbers (sampleNumbers);

            // Check if we have data to write
            if (numChannelsInStream > 0 && m_perStreamDataIdxs[streamIdx][0].size1 > 0)
            {
                int numSamples = m_perStreamDataIdxs[streamIdx][0].size1;
                int bufferIndex = m_perStreamDataIdxs[streamIdx][0].index1;

                // Build batch for all channels in this stream
                m_batchWriteChannels.clear();
                m_batchRealChannels.clear();
                m_batchDataPtrs.clear();

                int localChan = 0;
                for (int globalChan = 0; globalChan < m_numChannels; globalChan++)
                {
                    if (m_timestampBufferChannelArray[globalChan] == streamIdx)
                    {
                        m_batchWriteChannels.push_back (globalChan);
                        m_batchRealChannels.push_back (m_channelArray[globalChan]);
                        m_batchDataPtrs.push_back (dataBuffer.getReadPointer (localChan, bufferIndex));
                        localChan++;
                    }
                }

                int batchSize = static_cast<int> (m_batchWriteChannels.size());

                // Get timestamp buffer (index 0 since each queue has one timestamp stream)
                const double* timestamps = timestampBuffer.getReadPointer (0, bufferIndex);

                // Use batch write if we have multiple channels, otherwise use single-channel write
                if (batchSize > 1 && useBatchWrites)
                {
                    m_engine->writeContinuousDataBatch (
                        m_batchWriteChannels.data(),
                        m_batchRealChannels.data(),
                        m_batchDataPtrs.data(),
                        timestamps,
                        batchSize,
                        numSamples,
                        streamIdx);
                }
                else
                {
                    for (int i = 0; i < batchSize; ++i)
                    {
                        m_engine->writeContinuousData (
                            m_batchWriteChannels[i],
                            m_batchRealChannels[i],
                            m_batchDataPtrs[i],
                            timestamps,
                            numSamples);
                    }
                }

                // Handle wrap-around (size2) if present
                if (m_perStreamDataIdxs[streamIdx][0].size2 > 0)
                {
                    int numSamples2 = m_perStreamDataIdxs[streamIdx][0].size2;
                    int bufferIndex2 = m_perStreamDataIdxs[streamIdx][0].index2;

                    // Update sample numbers using per-stream value
                    int64 updatedStreamSampleNumber = m_perStreamSampleNumbers[streamIdx] + numSamples;
                    m_perStreamSampleNumbers[streamIdx] = updatedStreamSampleNumber;
                    for (int globalChan = 0; globalChan < m_numChannels; globalChan++)
                    {
                        if (m_timestampBufferChannelArray[globalChan] == streamIdx)
                        {
                            sampleNumbers.set (globalChan, updatedStreamSampleNumber);
                        }
                    }
                    m_engine->updateLatestSampleNumbers (sampleNumbers, m_batchWriteChannels[0]);

                    // Update data pointers for the second segment
                    localChan = 0;
                    for (int i = 0; i < batchSize; i++)
                    {
                        m_batchDataPtrs[i] = dataBuffer.getReadPointer (localChan, bufferIndex2);
                        localChan++;
                    }

                    const double* timestamps2 = timestampBuffer.getReadPointer (0, bufferIndex2);

                    if (batchSize > 1 && useBatchWrites)
                    {
                        m_engine->writeContinuousDataBatch (
                            m_batchWriteChannels.data(),
                            m_batchRealChannels.data(),
                            m_batchDataPtrs.data(),
                            timestamps2,
                            batchSize,
                            numSamples2,
                            streamIdx);
                    }
                    else
                    {
                        for (int i = 0; i < batchSize; ++i)
                        {
                            m_engine->writeContinuousData (
                                m_batchWriteChannels[i],
                                m_batchRealChannels[i],
                                m_batchDataPtrs[i],
                                timestamps2,
                                numSamples2);
                        }
                    }
                }
            }

            queue->stopRead();
        }
    }

    std::vector<EventMessagePtr> events;
    int nEvents = m_eventQueue->getEvents (events, maxEvents);

    for (int ev = 0; ev < nEvents; ++ev)
    {
        const MidiMessage& event = events[ev]->getData();

        if (SystemEvent::getBaseType (event) == EventBase::Type::SYSTEM_EVENT)
        {
            String syncText = SystemEvent::getSyncText (event);
            m_engine->writeTimestampSyncText (SystemEvent::getStreamId (event),
                                              SystemEvent::getSampleNumber (event),
                                              0.0f,
                                              SystemEvent::getSyncText (event));
        }
        else
        {
            int processorId = EventBase::getProcessorId (event);
            int streamId = EventBase::getStreamId (event);
            int channelIdx = EventBase::getChannelIndex (event);

            const EventChannel* chan = recordNode->getEventChannel (processorId, streamId, channelIdx);
            int eventIndex = recordNode->getIndexOfMatchingChannel (chan);

            m_engine->writeEvent (eventIndex, event);
        }
    }

    std::vector<SpikeMessagePtr> spikes;
    int nSpikes = m_spikeQueue->getEvents (spikes, BLOCK_MAX_WRITE_SPIKES);

    for (int sp = 0; sp < nSpikes; ++sp)
    {
        spikesReceived++;

        if (spikes[sp] != nullptr)
        {
            const Spike& spike = spikes[sp]->getData();
            const SpikeChannel* chan = spike.getChannelInfo();
            int spikeIndex = recordNode->getIndexOfMatchingChannel (chan);
            spikesWritten++;

            m_engine->writeSpike (spikeIndex, &spikes[sp]->getData());
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
