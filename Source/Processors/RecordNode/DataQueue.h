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

#ifndef DATAQUEUE_H_INCLUDED
#define DATAQUEUE_H_INCLUDED

#include <JuceHeader.h>
#include "../../Utils/Utils.h"

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
	DataQueue(int blockSize, int nBlocks);

	/** Destructor */
	~DataQueue();

	/// -----------  NOT THREAD SAFE  -------------- //
	/** Sets the number of continuous channel buffers needed */
	void setChannelCount(int nChans);

	/** Sets the number of timestamp buffers needed */
	void setTimestampStreamCount(int nStreams);

	/** Changes the number of blocks in the queue */
	void resize(int nBlocks);

	/** Returns an array of sample numbers for a given block*/
	void getSampleNumbersForBlock(int idx, Array<int64>& sampleNumbers) const;

	/// -----------  THREAD SAFE  -------------- //

	/** Writes an array of data for one channel */
	float writeChannel(const AudioBuffer<float>& buffer, int srcChannel, int destChannel, int nSamples, int64 sampleNumbers);

	/** Writes an array of timestamps for one stream */
	float writeSynchronizedTimestamps(double start, double step, int destChannel, int64 nSamples);

	/** Start reading data for one channel */
	bool startRead(Array<CircularBufferIndexes>& dataIndexes, Array<CircularBufferIndexes>& ftsIndexes, Array<int64>& sampleNumbers, int nMax);

	/** Called when data read is finished */
	void stopRead();

	/** Returns a reference to the continuous data buffer */
	const AudioBuffer<float>& getContinuousDataBufferReference() const;

	/** Returns a reference to the timestamp buffer */
	const SynchronizedTimestampBuffer& getTimestampBufferReference() const;

	/** Returns the current block size*/
	int getBlockSize();

private:

	/** Fills the sample number buffer for a given channel */
	void fillSampleNumbers(int channel, int index, int size, int64 sampleNumbers);

	int lastIdx;

	OwnedArray<AbstractFifo> m_fifos;
	OwnedArray<AbstractFifo> m_FTSFifos;

	AudioSampleBuffer m_buffer;
	SynchronizedTimestampBuffer m_FTSBuffer;

	Array<int> m_readSamples;
	Array<int> m_readFTSSamples;
	OwnedArray<Array<int64>> m_sampleNumbers;
	Array<int64> m_lastReadSampleNumbers;

	int m_numChans;
	int m_numFTSChans;
	int m_blockSize;
	bool m_readInProgress;
	int m_numBlocks;
	int m_maxSize;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataQueue);
};


#endif  // DATAQUEUE_H_INCLUDED
