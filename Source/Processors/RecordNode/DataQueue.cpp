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

#include "DataQueue.h"
#include <climits>

DataQueue::DataQueue (int blockSize, int nBlocks) : m_buffer (0, blockSize * nBlocks),
                                                    m_numChans (0),
                                                    m_blockSize (blockSize),
                                                    m_readInProgress (false),
                                                    m_numBlocks (nBlocks),
                                                    m_maxSize (blockSize * nBlocks)
{
}

DataQueue::~DataQueue()
{
}

int DataQueue::getBlockSize()
{
    return m_blockSize;
}

void DataQueue::setTimestampStreamCount (int nStreams)
{
    if (m_readInProgress)
        return;

    m_FTSFifos.clear();
    m_readFTSSamples.clear();
    m_numFTSChans = nStreams;

    for (int i = 0; i < nStreams; ++i)
    {
        m_FTSFifos.add (new AbstractFifo (m_maxSize));
        m_readFTSSamples.push_back (0);
    }
    m_FTSBuffer.setSize (nStreams, m_maxSize);
}

void DataQueue::setChannelCount (int nChans)
{
    if (m_readInProgress)
        return;

    m_fifos.clear();
    m_readSamples.clear();
    m_numChans = nChans;
    m_sampleNumbers.clear();
    m_lastReadSampleNumber = 0;

    for (int i = 0; i < nChans; ++i)
    {
        m_fifos.add (new AbstractFifo (m_maxSize));
        m_readSamples.push_back (0);
    }

    // Initialize per-stream sample numbers (one per block)
    for (int j = 0; j < m_numBlocks; j++)
    {
        m_sampleNumbers.push_back (0);
    }

    m_buffer.setSize (nChans, m_maxSize);
}

void DataQueue::resize (int nBlocks)
{
    if (m_readInProgress)
        return;

    int size = m_blockSize * nBlocks;
    m_maxSize = size;
    m_numBlocks = nBlocks;

    for (int i = 0; i < m_numChans; ++i)
    {
        m_fifos[i]->setTotalSize (size);
        m_fifos[i]->reset();
        m_readSamples[i] = 0;
    }

    // Resize per-stream sample numbers
    m_sampleNumbers.resize (nBlocks);
    m_lastReadSampleNumber = 0;

    m_readFTSSamples.clear();

    for (int i = 0; i < m_numFTSChans; ++i)
    {
        m_readFTSSamples.push_back (0);
        m_FTSFifos[i]->setTotalSize (size);
        m_FTSFifos[i]->reset();
    }
    m_buffer.setSize (m_numChans, size);
    m_FTSBuffer.setSize (m_numFTSChans, size);
}

void DataQueue::fillSampleNumbers (int index, int size, int64 sampleNumber)
{
    //Search for the next block start.
    int blockMod = index % m_blockSize;
    int blockIdx = index / m_blockSize;
    int64 startSampleNumber;
    int blockStartPos;

    if (blockMod == 0) //block starts here
    {
        startSampleNumber = sampleNumber;
        blockStartPos = index;
    }
    else //we're in the middle of a block, correct to jump to the start of the next-
    {
        startSampleNumber = sampleNumber + (m_blockSize - blockMod);
        blockStartPos = index + (m_blockSize - blockMod);
        blockIdx++;
    }

    //check that the block is in range
    int64 latestSampleNumber;

    for (int i = 0; i < size; i += m_blockSize)
    {
        if ((blockStartPos + i) < (index + size))
        {
            latestSampleNumber = startSampleNumber + (i * m_blockSize);
            m_sampleNumbers[blockIdx] = latestSampleNumber;
        }
    }
}

float DataQueue::writeSynchronizedTimestamps (double start, double step, int destChannel, int nSamples)
{
    int index1, size1, index2, size2;

    m_FTSFifos[destChannel]->prepareToWrite (nSamples, index1, size1, index2, size2);

    if ((size1 + size2) < nSamples)
    {
        LOGE (__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);
    }

    // Get direct pointer access for faster writes
    double* writePtr = m_FTSBuffer.getWritePointer (destChannel);

    // Fill first segment
    for (int i = 0; i < size1; i++)
    {
        writePtr[index1 + i] = start + (double) i * step;
    }

    // Fill second segment (wrap-around) if present
    if (size2 > 0)
    {
        double offset = start + (double) size1 * step;
        for (int i = 0; i < size2; i++)
        {
            writePtr[index2 + i] = offset + (double) i * step;
        }
    }

    m_FTSFifos[destChannel]->finishedWrite (size1 + size2);

    return 1.0f - (float) m_FTSFifos[destChannel]->getFreeSpace() / (float) m_FTSFifos[destChannel]->getTotalSize();
}

float DataQueue::writeSynchronizedTimestamps (const double* timestamps, int destChannel, int nSamples)
{
    int index1, size1, index2, size2;

    m_FTSFifos[destChannel]->prepareToWrite (nSamples, index1, size1, index2, size2);

    if ((size1 + size2) < nSamples)
    {
        LOGE (__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);
    }

    double* writePtr = m_FTSBuffer.getWritePointer (destChannel);

    if (size1 > 0)
        memcpy (writePtr + index1, timestamps, (size_t) size1 * sizeof (double));

    if (size2 > 0)
        memcpy (writePtr + index2, timestamps + size1, (size_t) size2 * sizeof (double));

    m_FTSFifos[destChannel]->finishedWrite (size1 + size2);

    return 1.0f - (float) m_FTSFifos[destChannel]->getFreeSpace() / (float) m_FTSFifos[destChannel]->getTotalSize();
}

float DataQueue::writeChannel (const AudioBuffer<float>& buffer,
                               int srcChannel,
                               int destChannel,
                               int nSamples,
                               int64 sampleNumber)
{
    int index1, size1, index2, size2;
    m_fifos[destChannel]->prepareToWrite (nSamples, index1, size1, index2, size2);

    if ((size1 + size2) < nSamples)
    {
        LOGE (__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);
    }
    m_buffer.copyFrom (destChannel,
                       index1,
                       buffer,
                       srcChannel,
                       0,
                       size1);

    // Only fill sample numbers once per stream (using first channel write)
    if (destChannel == 0)
    {
        fillSampleNumbers (index1, size1, sampleNumber);
    }

    if (size2 > 0)
    {
        m_buffer.copyFrom (destChannel,
                           index2,
                           buffer,
                           srcChannel,
                           size1,
                           size2);

        if (destChannel == 0)
        {
            fillSampleNumbers (index2, size2, sampleNumber + size1);
        }
    }
    m_fifos[destChannel]->finishedWrite (size1 + size2);

    return 1.0f - (float) m_fifos[destChannel]->getFreeSpace() / (float) m_fifos[destChannel]->getTotalSize();
}

float DataQueue::writeAllChannels (const AudioBuffer<float>& buffer,
                                   const Array<int>& srcChannels,
                                   int nSamples,
                                   int64 sampleNumber)
{
    if (m_numChans == 0 || nSamples == 0)
        return 0.0f;
    int index1 = 0;
    int size1 = 0;
    int index2 = 0;
    int size2 = 0;

    {
        // Get FIFO indices from first channel - all channels should be in sync
        auto fifoScope = m_fifos[0]->write (nSamples);
        index1 = fifoScope.startIndex1;
        size1 = fifoScope.blockSize1;
        index2 = fifoScope.startIndex2;
        size2 = fifoScope.blockSize2;

        if ((size1 + size2) < nSamples)
            LOGE (__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);

        // Fill sample numbers once for the stream (not per-channel)
        fillSampleNumbers (index1, size1, sampleNumber);
        if (size2 > 0)
        {
            fillSampleNumbers (index2, size2, sampleNumber + size1);
        }

        const int* srcChannelPtr = srcChannels.getRawDataPointer();

        // Batch copy and update all channels
        for (int destChannel = 0; destChannel < m_numChans; ++destChannel)
        {
            const int srcChannel = srcChannelPtr[destChannel];
            const float* src = buffer.getReadPointer (srcChannel);
            float* dest = m_buffer.getWritePointer (destChannel);

            // Copy first segment
            FloatVectorOperations::copy (dest + index1, src, size1);

            // Copy second segment (wrap-around) if present
            if (size2 > 0)
            {
                FloatVectorOperations::copy (dest + index2, src + size1, size2);
            }

            // Update FIFO state for channels > 0 (indices are aligned with channel 0)
            if (destChannel > 0)
            {
                jassert (m_fifos[destChannel]->getFreeSpace() >= (size1 + size2));
                m_fifos[destChannel]->finishedWrite (size1 + size2);
            }
        }
    }

    // Return usage from last channel (all should be the same)
    return getFifoUsage();
}

float DataQueue::getFifoUsage() const
{
    const float freeSpace = (float) m_fifos[m_numChans - 1]->getFreeSpace();
    const float totalSize = (float) m_fifos[m_numChans - 1]->getTotalSize();

    const float usage = 1.0f - (freeSpace / totalSize);

    //LOGC ("DataQueue::getFifoUsage: usage = ", usage, " (free: ", freeSpace, ", total: ", totalSize, ")");

    return usage;
}

/*
We could copy the internal circular buffer to an external one, as DataBuffer does.
is, however, intended for disk writing, which is one of the most CPU-critical systems. Just
allowing the record subsystem to access the internal buffer is way faster, although it has to be
done with special care and manually finish the read process.
*/

const AudioBuffer<float>& DataQueue::getContinuousDataBufferReference() const
{
    return m_buffer;
}

const SynchronizedTimestampBuffer& DataQueue::getTimestampBufferReference() const
{
    return m_FTSBuffer;
}

int DataQueue::getNumSamplesReady() const
{
    if (m_numChans == 0)
        return 0;

    int minReady = INT_MAX;
    for (int chan = 0; chan < m_numChans; ++chan)
    {
        int ready = m_fifos.getUnchecked (chan)->getNumReady();
        if (ready < minReady)
            minReady = ready;
    }
    return (minReady == INT_MAX) ? 0 : minReady;
}

bool DataQueue::startRead (std::vector<CircularBufferIndexes>& dataBufferIdxs,
                           std::vector<CircularBufferIndexes>& timestampBufferIdxs,
                           int64& sampleNumber,
                           int nMin,
                           int nMax)
{
    //This should never happen, but it never hurts to be on the safe side.
    if (m_readInProgress)
        return false;

    m_readInProgress = true;

    // First pass: find the minimum samples available across ALL channels
    // This ensures we don't read from some channels while others are still being written
    int minSamplesAvailable = INT_MAX;
    for (int chan = 0; chan < m_numChans; ++chan)
    {
        int readyToRead = m_fifos.getUnchecked (chan)->getNumReady();
        if (readyToRead < minSamplesAvailable)
            minSamplesAvailable = readyToRead;
    }

    // Apply nMax limit to the minimum
    int samplesToRead = ((minSamplesAvailable > nMax) && (nMax > 0)) ? nMax : minSamplesAvailable;

    // If not enough samples available (below nMin threshold), skip this read
    if (samplesToRead < nMin)
    {
        // Initialize all indexes to zero
        for (int chan = 0; chan < m_numChans; ++chan)
        {
            CircularBufferIndexes& idx = dataBufferIdxs[chan];
            idx.index1 = 0;
            idx.size1 = 0;
            idx.index2 = 0;
            idx.size2 = 0;
            m_readSamples[chan] = 0;
        }
        for (int chan = 0; chan < m_numFTSChans; ++chan)
        {
            CircularBufferIndexes& idx = timestampBufferIdxs[chan];
            idx.index1 = 0;
            idx.size1 = 0;
            idx.index2 = 0;
            idx.size2 = 0;
            m_readFTSSamples[chan] = 0;
        }
        sampleNumber = m_lastReadSampleNumber;
        m_readInProgress = false;
        return false;
    }

    // Get sample number for the stream (using first channel's indices)
    CircularBufferIndexes& firstIdx = dataBufferIdxs[0];
    m_fifos.getUnchecked (0)->prepareToRead (samplesToRead, firstIdx.index1, firstIdx.size1, firstIdx.index2, firstIdx.size2);
    m_readSamples[0] = firstIdx.size1 + firstIdx.size2;

    int blockMod = firstIdx.index1 % m_blockSize;
    int blockDiff = (blockMod == 0) ? 0 : (m_blockSize - blockMod);

    // If the next sample number block is within the data we're reading, include the translated sample number
    if (blockDiff < (firstIdx.size1 + firstIdx.size2))
    {
        int blockIdx = ((firstIdx.index1 + blockDiff) / m_blockSize) % m_numBlocks;
        sampleNumber = m_sampleNumbers[blockIdx] - blockDiff;
    }
    else
    {
        // If not, use the last sent sample number
        sampleNumber = m_lastReadSampleNumber;
    }

    m_lastReadSampleNumber = sampleNumber + firstIdx.size1 + firstIdx.size2;

    // Read remaining channels with same parameters
    for (int chan = 1; chan < m_numChans; ++chan)
    {
        CircularBufferIndexes& idx = dataBufferIdxs[chan];
        m_fifos.getUnchecked (chan)->prepareToRead (samplesToRead, idx.index1, idx.size1, idx.index2, idx.size2);
        m_readSamples[chan] = idx.size1 + idx.size2;
    }

    // Also find minimum for timestamp streams and read consistently
    int minFTSSamples = INT_MAX;
    for (int chan = 0; chan < m_numFTSChans; ++chan)
    {
        int readyToRead = m_FTSFifos.getUnchecked (chan)->getNumReady();
        if (readyToRead < minFTSSamples)
            minFTSSamples = readyToRead;
    }
    int ftsToRead = ((minFTSSamples > nMax) && (nMax > 0)) ? nMax : minFTSSamples;

    for (int chan = 0; chan < m_numFTSChans; ++chan)
    {
        CircularBufferIndexes& idx = timestampBufferIdxs[chan];

        m_FTSFifos.getUnchecked (chan)->prepareToRead (ftsToRead, idx.index1, idx.size1, idx.index2, idx.size2);
        m_readFTSSamples[chan] = idx.size1 + idx.size2;
    }

    return true;
}

void DataQueue::stopRead()
{
    if (! m_readInProgress)
        return;

    for (int i = 0; i < m_numChans; ++i)
    {
        m_fifos[i]->finishedRead (m_readSamples[i]);
        m_readSamples[i] = 0;
    }

    for (int i = 0; i < m_numFTSChans; ++i)
    {
        m_FTSFifos[i]->finishedRead (m_readFTSSamples[i]);
        m_readFTSSamples[i] = 0;
    }

    m_readInProgress = false;
}

int64 DataQueue::getSampleNumberForBlock (int idx) const
{
    return m_sampleNumbers[idx];
}
