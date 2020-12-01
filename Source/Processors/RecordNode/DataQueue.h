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

class DataQueue
{
public:
	DataQueue(int blockSize, int nBlocks);
	~DataQueue();
	void setChannels(int nChans);
	void setFTSChannels(int nChans);
	void resize(int nBlocks);
	void getTimestampsForBlock(int idx, Array<int64>& timestamps) const;

	//Only the methods after this comment are considered thread-safe.
	//Caution must be had to avoid calling more than one of the methods above simulatenously
	float writeChannel(const AudioSampleBuffer& buffer, int srcChannel, int destChannel, int nSamples, int64 timestamp);
	float writeSynchronizedTimestampChannel(double start, double step, int destChannel, int64 nSamples);
	bool startRead(Array<CircularBufferIndexes>& indexes, Array<int64>& timestamps, int nMax);
	bool startSynchronizedRead(Array<CircularBufferIndexes>& dataIndexes, Array<CircularBufferIndexes>& ftsIndexes, Array<int64>& timestamps, int nMax);
	const AudioSampleBuffer& getAudioBufferReference() const;
	const SynchronizedTimestampBuffer& getFTSBufferReference() const;
	void stopRead();
	void stopSynchronizedRead();

private:
	void fillTimestamps(int channel, int index, int size, int64 timestamp);

	int lastIdx;

	OwnedArray<AbstractFifo> m_fifos;
	OwnedArray<AbstractFifo> m_FTSFifos;

	AudioSampleBuffer m_buffer;
	SynchronizedTimestampBuffer m_FTSBuffer;

	Array<int> m_readSamples;
	Array<int> m_readFTSSamples;
	OwnedArray<Array<int64>> m_timestamps;
	Array<int64> m_lastReadTimestamps;

	int m_numChans;
	int m_numFTSChans;
	const int m_blockSize;
	bool m_readInProgress;
	int m_numBlocks;
	int m_maxSize;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataQueue);
};


#endif  // DATAQUEUE_H_INCLUDED
