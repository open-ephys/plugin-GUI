/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Open Ephys

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

DataQueue::DataQueue(int blockSize, int nBlocks) :
	m_buffer(0, blockSize*nBlocks),
	m_numChans(0),
	m_blockSize(blockSize),
	m_readInProgress(false),
	m_numBlocks(nBlocks),
	m_maxSize(blockSize*nBlocks)
{}

DataQueue::~DataQueue()
{}

void DataQueue::setFTSChannels(int nChans)
{
	if (m_readInProgress)
		return;

	m_FTSFifos.clear();
	m_readFTSSamples.clear();
	m_numFTSChans = nChans;

	for (int i = 0; i < nChans; ++i)
	{
		m_FTSFifos.add(new AbstractFifo(m_maxSize));
	}
	m_FTSBuffer.setSize(nChans, m_maxSize);
}

void DataQueue::setChannels(int nChans)
{
	if (m_readInProgress)
		return;

	m_fifos.clear();
	m_readSamples.clear();
	m_numChans = nChans;
	m_timestamps.clear();
	m_lastReadTimestamps.clear();

	for (int i = 0; i < nChans; ++i)
	{
		m_fifos.add(new AbstractFifo(m_maxSize));
		m_readSamples.add(0);
		m_timestamps.add(new Array<int64>());
		m_timestamps.getLast()->resize(m_numBlocks);
		m_lastReadTimestamps.add(0);
	}
	m_buffer.setSize(nChans, m_maxSize);

}

void DataQueue::resize(int nBlocks)
{
	if (m_readInProgress)
		return;

	int size = m_blockSize*nBlocks;
	m_maxSize = size;
	m_numBlocks = nBlocks;

	for (int i = 0; i < m_numChans; ++i)
	{
		m_fifos[i]->setTotalSize(size);
		m_fifos[i]->reset();
		m_readSamples.set(i, 0);
		m_timestamps[i]->resize(nBlocks);
		m_lastReadTimestamps.set(i, 0);
	}

	for (int i = 0; i < m_numFTSChans; ++i)
	{
		m_readFTSSamples.set(i,0);
		m_FTSFifos[i]->setTotalSize(size);
		m_FTSFifos[i]->reset();
	}
	m_buffer.setSize(m_numChans, size);
	m_FTSBuffer.setSize(m_numFTSChans, size);
}

void DataQueue::fillTimestamps(int channel, int index, int size, int64 timestamp)
{
	//Search for the next block start.
	int blockMod = index % m_blockSize;
	int blockIdx = index / m_blockSize;
	int64 startTimestamp;
	int blockStartPos;

	if (blockMod == 0) //block starts here
	{
		startTimestamp = timestamp;
		blockStartPos = index;
	}
	else //we're in the middle of a block, correct to jump to the start of the next-
	{
		startTimestamp = timestamp + (m_blockSize - blockMod);
		blockStartPos = index + (m_blockSize - blockMod);
		blockIdx++;
	}

	//check that the block is in range
	for (int i = 0; i < size; i += m_blockSize)
	{
		if ((blockStartPos + i) < (index + size))
		{
			int64 ts = startTimestamp + (i*m_blockSize);
			m_timestamps[channel]->set(blockIdx, ts);
		}

	}
}

float DataQueue::writeSynchronizedTimestampChannel(double start, double step, int destChannel, int64 nSamples)
{

	int index1, size1, index2, size2;

	m_FTSFifos[destChannel]->prepareToWrite(nSamples, index1, size1, index2, size2);

	//LOGD("nFTSChannels=", m_FTSFifos.size(), " destChannel=", destChannel, " index1: ", index1, " size1=", size1, " size2=", size2);

	if ((size1 + size2) < nSamples)
	{
		LOGD(__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);
	}

	//LOGD("DataQueue::writeSynchronizedTimestampChannel: ", start);

	for (int i = 0; i < size1; i++)
	{
		//LOGD("setSample: ", start + (float)i*step);
		m_FTSBuffer.setSample(destChannel, index1+i, start+(double)i*step);
		//LOGD("Set sample ", index1+i, " to ", start);
		//m_FTSBuffer.setSample(destChannel, index1+i, start + (float)i*step);
	}

	if (size2 > 0)
	{
		for (int i = 0; i < size2; i++)
		{
			//m_FTSBuffer.setSample(destChannel, index2+i, start + (float)i*step);
			m_FTSBuffer.setSample(destChannel, index2 + i, start+(double)size1*step + double(i*step));
		}
	}

	m_FTSFifos[destChannel]->finishedWrite(size1 + size2);

	return 1.0f - (float)m_FTSFifos[destChannel]->getFreeSpace() / (float)m_FTSFifos[destChannel]->getTotalSize();

}


float DataQueue::writeChannel(const AudioSampleBuffer& buffer, int srcChannel, int destChannel, int nSamples, int64 timestamp)
{
	int index1, size1, index2, size2;
	m_fifos[destChannel]->prepareToWrite(nSamples, index1, size1, index2, size2);

	if ((size1 + size2) < nSamples)
	{ //TODO: turn this into a proper notification. Probably returning a bool.
		//std::cerr << "Recording Data Queue Overflow" << std::endl;
		LOGD(__FUNCTION__, " Recording Data Queue Overflow: sz1: ", size1, " sz2: ", size2, " nSamples: ", nSamples);
	}
	m_buffer.copyFrom(destChannel,
		index1,
		buffer,
		srcChannel,
		0,
		size1);

	fillTimestamps(destChannel, index1, size1, timestamp);

	if (size2 > 0)
	{
		m_buffer.copyFrom(destChannel,
			index2,
			buffer,
			srcChannel,
			size1,
			size2);

		fillTimestamps(destChannel, index2, size2, timestamp + size1);
	}
	m_fifos[destChannel]->finishedWrite(size1 + size2);

	return 1.0f - (float)m_fifos[destChannel]->getFreeSpace() / (float)m_fifos[destChannel]->getTotalSize();
}

/*
We could copy the internal circular buffer to an external one, as DataBuffer does. This class
is, however, intended for disk writing, which is one of the most CPU-critical systems. Just
allowing the record subsytem to access the internal buffer is way faster, altough it has to be
done with special care and manually finish the read process.
*/

const AudioSampleBuffer& DataQueue::getAudioBufferReference() const
{
	return m_buffer;
}

const SynchronizedTimestampBuffer& DataQueue::getFTSBufferReference() const
{
	return m_FTSBuffer;
}

bool DataQueue::startSynchronizedRead(Array<CircularBufferIndexes>& dataIndexes, Array<CircularBufferIndexes>& ftsIndexes, Array<int64>& timestamps, int nMax)
{

	//This should never happen, but it never hurts to be on the safe side.
	if (m_readInProgress)
		return false;

	m_readInProgress = true;
	dataIndexes.clear();
	ftsIndexes.clear();
	timestamps.clear();

	for (int chan = 0; chan < m_numChans; ++chan)
	{
		CircularBufferIndexes idx;
		int readyToRead = m_fifos[chan]->getNumReady();
		int samplesToRead = ((readyToRead > nMax) && (nMax > 0)) ? nMax : readyToRead;

		m_fifos[chan]->prepareToRead(samplesToRead, idx.index1, idx.size1, idx.index2, idx.size2);
		dataIndexes.add(idx);
		m_readSamples.set(chan, idx.size1 + idx.size2);

		int blockMod = idx.index1 % m_blockSize;
		int blockDiff = (blockMod == 0) ? 0 : (m_blockSize - blockMod);

		//If the next timestamp block is within the data we're reading, include the translated timestamp in the output
		int64 ts;
		if (blockDiff < (idx.size1 + idx.size2))
		{
			int blockIdx = ((idx.index1 + blockDiff) / m_blockSize) % m_numBlocks;
			ts = m_timestamps[chan]->getUnchecked(blockIdx) - blockDiff;
		}
		//If not, copy the last sent again 
		else
		{
			ts = m_lastReadTimestamps[chan];
		}
		timestamps.add(ts);
		m_lastReadTimestamps.set(chan, ts + idx.size1 + idx.size2);
	}

	for (int chan = 0; chan < m_numFTSChans; ++chan)
	{
		CircularBufferIndexes idx;
		int readyToRead = m_FTSFifos[chan]->getNumReady();
		int samplesToRead = ((readyToRead > nMax) && (nMax > 0)) ? nMax : readyToRead;

		m_FTSFifos[chan]->prepareToRead(samplesToRead, idx.index1, idx.size1, idx.index2, idx.size2);
		//LOGD("idx1: ", idx.index1, " | s1: ", idx.size1, " | idx2: ", idx.index2, " | s2: ", idx.size2);
		ftsIndexes.add(idx);
		m_readFTSSamples.set(chan, idx.size1 + idx.size2);
	}

	return true;
}

bool DataQueue::startRead(Array<CircularBufferIndexes>& indexes, Array<int64>& timestamps, int nMax)
{
	//This should never happen, but it never hurts to be on the safe side.
	if (m_readInProgress)
		return false;

	m_readInProgress = true;
	indexes.clear(); //Just in case it's not empty already
	timestamps.clear();

	for (int chan = 0; chan < m_numChans; ++chan)
	{
		CircularBufferIndexes idx;
		int readyToRead = m_fifos[chan]->getNumReady();
		int samplesToRead = ((readyToRead > nMax) && (nMax > 0)) ? nMax : readyToRead;

		m_fifos[chan]->prepareToRead(samplesToRead, idx.index1, idx.size1, idx.index2, idx.size2);
		indexes.add(idx);
		m_readSamples.set(chan, idx.size1 + idx.size2);

		int blockMod = idx.index1 % m_blockSize;
		int blockDiff = (blockMod == 0) ? 0 : (m_blockSize - blockMod);

		//If the next timestamp block is within the data we're reading, include the translated timestamp in the output
		if (blockDiff < (idx.size1 + idx.size2))
		{
			int blockIdx = ((idx.index1 + blockDiff) / m_blockSize) % m_numBlocks;
			int64 ts = m_timestamps[chan]->getUnchecked(blockIdx) - blockDiff;
			timestamps.add(ts);
			//update to the end of the block
			m_lastReadTimestamps.set(chan, ts + idx.size1 + idx.size2);
		}
		//If not, copy the last sent again 
		else
		{
			int64 ts = m_lastReadTimestamps[chan];
			timestamps.add(ts);
			m_lastReadTimestamps.set(chan, ts + idx.size1 + idx.size2);
		}
	}

	return true;
}

void DataQueue::stopSynchronizedRead()
{
	if (!m_readInProgress)
		return;

	for (int i = 0; i < m_numChans; ++i)
	{
		m_fifos[i]->finishedRead(m_readSamples[i]);
		m_readSamples.set(i, 0);
	}

	for (int i = 0; i < m_numFTSChans; ++i)
	{
		m_FTSFifos[i]->finishedRead(m_readFTSSamples[i]);
		m_readFTSSamples.set(i, 0);
	}

	m_readInProgress = false;
}

void DataQueue::stopRead()
{
	if (!m_readInProgress)
		return;

	for (int i = 0; i < m_numChans; ++i)
	{
		m_fifos[i]->finishedRead(m_readSamples[i]);
		m_readSamples.set(i, 0);
	}

	m_readInProgress = false;
}

void DataQueue::getTimestampsForBlock(int idx, Array<int64>& timestamps) const
{
	timestamps.clear();
	for (int chan = 0; chan < m_numChans; ++chan)
	{
		timestamps.add((*m_timestamps[chan])[idx]);
	}
}