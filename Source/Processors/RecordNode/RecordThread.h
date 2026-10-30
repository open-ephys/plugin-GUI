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

#ifndef RECORDTHREAD_H_INCLUDED
#define RECORDTHREAD_H_INCLUDED

#include "../../Utils/Utils.h"
#include "BinaryFormat/BinaryRecording.h"
#include "DataQueue.h"
#include "EventQueue.h"
#include <atomic>
#include <chrono>
#include <cstdlib>

// Block write size limits (in samples) - tune these for performance
// Sample-based thresholds naturally scale with channel count:
//   - High channel count = large writes (efficient for high data rates)
//   - Low channel count = small writes (fine for low data rates, keeps latency low)
//

#define BLOCK_DEFAULT_MIN_WRITE_SAMPLES 512
#define BLOCK_DEFAULT_MAX_WRITE_SAMPLES 4096
#define BLOCK_MAX_WRITE_EVENTS 50000
#define BLOCK_MAX_WRITE_SPIKES 50000

class RecordNode;

/**
*
*	A thread inside the RecordNode that allows continuous data, spikes,
*   and events to be written outside of the process() method.
*
*/
class RecordThread : public Thread
{
public:
    /** Constructor */
    RecordThread (RecordNode* parentNode, RecordEngine* engine);

    /** Destructor */
    ~RecordThread();

    /** Sets the recording directory, experiment number, and recording number*/
    void setFileComponents (File rootFolder, int experimentNumber, int recordingNumber);

    /** Sets the indices of recorded channels */
    void setChannelMap (const Array<int>& channels);

    /** Sets the float timestamp channel map */
    void setTimestampChannelMap (const Array<int>& channels);

    /** Sets the pointers to the data queues (one per stream), event queue, and spike queue */
    void setQueuePointers (OwnedArray<DataQueue>* dataQueues, EventMsgQueue* events, SpikeMsgQueue* spikes);

    /** Runs the thread */
    void run() override;

    /** Sets whether the first block is being written */
    void setFirstBlockFlag (bool state);

    /** Force all open files to close */
    void forceCloseFiles();

    /** Updates the Record Engine for this thread*/
    void setEngine (RecordEngine* engine);

    /** Returns shutdown progress; 1.0 means all queues are drained and files are closed. */
    double getDrainProgress() const { return m_drainProgress.load(); }

    /** Pointer to the RecordNode */
    RecordNode* recordNode;

private:
    /** Writes one bounded batch of continuous data, events, and spikes
     *  @param minSamples Minimum samples required before writing (0 = write any available)
     *  @param maxSamples Maximum samples per write batch
     *  @param maxEvents Maximum events to write
     *  @param maxSpikes Maximum spikes to write
     *  @param lastBlock If true, ignore minSamples for this batch
     */
    void writeData (int minSamples,
                    int maxSamples,
                    int maxEvents,
                    int maxSpikes,
                    bool lastBlock = false);

    int64 getPendingItemCount() const;

    RecordEngine* m_engine;
    Array<int> m_channelArray;
    Array<int> m_timestampBufferChannelArray;

    OwnedArray<DataQueue>* m_dataQueues; // Array of per-stream queues
    EventMsgQueue* m_eventQueue;
    SpikeMsgQueue* m_spikeQueue;

    std::atomic<bool> m_receivedFirstBlock;
    std::atomic<bool> m_cleanExit;
    std::atomic<double> m_drainProgress { 0.0 };

    Array<int64> sampleNumbers; // Global sample numbers (indexed by global channel)
    std::vector<int64> m_perStreamSampleNumbers; // Per-stream sample numbers

    // Per-stream buffer index arrays for independent queue reading
    std::vector<std::vector<CircularBufferIndexes>> m_perStreamDataIdxs;
    std::vector<std::vector<CircularBufferIndexes>> m_perStreamTimestampIdxs;

    int spikesReceived;
    int spikesWritten;

    // Batch write support - pre-allocated buffers for grouping channels by stream
    std::vector<int> m_batchWriteChannels; // Write channel indices for current batch
    std::vector<int> m_batchRealChannels; // Real channel indices for current batch
    std::vector<const float*> m_batchDataPtrs; // Data buffer pointers for current batch

    File m_rootFolder;
    int m_experimentNumber;
    int m_recordingNumber;
    int m_numChannels;

    // Block size limits (in samples)
    int m_minWriteSamples; // Minimum samples before writing
    int m_maxWriteSamples; // Maximum samples per write batch

    bool useBatchWrites { true }; // Whether to use batch writes for continuous data

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordThread);
};

#endif // RECORDTHREAD_H_INCLUDED
