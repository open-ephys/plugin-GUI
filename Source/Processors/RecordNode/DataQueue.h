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

#ifndef DATAQUEUE_H_INCLUDED
#define DATAQUEUE_H_INCLUDED

#include "../../Utils/Utils.h"
#include <JuceHeader.h>

class Synchronizer;

struct CircularBufferIndexes
{
    int index1;
    int size1;
    int index2;
    int size2;
};

/**
 *
 * Buffers data from the Record Node prior to disk writing
 *
 * */
class DataQueue
{
public:
    /** Constructor */
    DataQueue (int blockSize, int nBlocks);

    /** Destructor */
    ~DataQueue();

    /// -----------  NOT THREAD SAFE  -------------- //
    /** Sets the number of continuous channel buffers needed (all channels belong to one stream) */
    void setChannelCount (int nChans);

    /** Sets the number of timestamp buffers needed */
    void setTimestampStreamCount (int nStreams);

    /** Changes the number of blocks in the queue */
    void resize (int nBlocks);

    /** Returns the sample number for a given block (same for all channels in stream) */
    int64 getSampleNumberForBlock (int idx) const;

    /// -----------  THREAD SAFE  -------------- //

    /** Writes an array of data for one channel */
    float writeChannel (const AudioBuffer<float>& buffer, int srcChannel, int destChannel, int nSamples, int64 sampleNumber);

    /** 
     * Batch write: writes data for all channels in a single operation.
     * Much faster than calling writeChannel() repeatedly because it uses
     * a single FIFO operation for all channels.
     * 
     * @param buffer      Source audio buffer
     * @param srcChannels Array mapping dest channel index to source channel index
     * @param nSamples    Number of samples to write per channel
     * @param sampleNumber Starting sample number for this block
     * @return Maximum FIFO usage across all channels (0.0 to 1.0)
     */
    float writeAllChannels (const AudioBuffer<float>& buffer,
                            const Array<int>& srcChannels,
                            int nSamples,
                            int64 sampleNumber);

    /** Writes an array of timestamps for one stream */
    float writeSynchronizedTimestamps (double start, double step, int destChannel, int nSamples);

    /** Writes explicit per-sample timestamps for one stream */
    float writeSynchronizedTimestamps (const double* timestamps, int destChannel, int nSamples);

    /** Returns the number of samples available to read (minimum across all channels) */
    int getNumSamplesReady() const;

    /** Start reading data for all channels in this stream 
     *  @param dataBufferIdxs Output: indices for reading continuous data
     *  @param timestampBufferIdxs Output: indices for reading timestamps
     *  @param sampleNumber Output: sample number for the start of the read
     *  @param nMin Minimum samples required - returns false if fewer available
     *  @param nMax Maximum samples to read
     *  @return true if read was started successfully, false otherwise
     */
    bool startRead (std::vector<CircularBufferIndexes>& dataBufferIdxs,
                    std::vector<CircularBufferIndexes>& timestampBufferIdxs,
                    int64& sampleNumber,
                    int nMin,
                    int nMax);

    /** Called when data read is finished */
    void stopRead();

    /** Returns a reference to the continuous data buffer */
    const AudioBuffer<float>& getContinuousDataBufferReference() const;

    /** Returns a reference to the timestamp buffer */
    const SynchronizedTimestampBuffer& getTimestampBufferReference() const;

    /** Returns the current block size*/
    int getBlockSize();

private:
    /** Fills the sample number buffer for the stream */
    void fillSampleNumbers (int index, int size, int64 sampleNumber);

    /** Returns the fraction of the FIFO currently in use */
    float getFifoUsage() const;

    int lastIdx;

    OwnedArray<AbstractFifo> m_fifos;
    OwnedArray<AbstractFifo> m_FTSFifos;

    AudioSampleBuffer m_buffer;
    SynchronizedTimestampBuffer m_FTSBuffer;

    std::vector<int> m_readSamples;
    std::vector<int> m_readFTSSamples;
    std::vector<int64> m_sampleNumbers; // Per-stream sample numbers (one per block)
    int64 m_lastReadSampleNumber; // Last sample number read for the stream

    int m_numChans;
    int m_numFTSChans;
    int m_blockSize;
    bool m_readInProgress;
    int m_numBlocks;
    int m_maxSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataQueue);
};

#endif // DATAQUEUE_H_INCLUDED
