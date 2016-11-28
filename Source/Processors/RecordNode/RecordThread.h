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

#ifndef RECORDTHREAD_H_INCLUDED
#define RECORDTHREAD_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "EventQueue.h"
#include "DataQueue.h"
#include <atomic>

#define BLOCK_MAX_WRITE_SAMPLES 4096
#define BLOCK_MAX_WRITE_EVENTS 32
#define BLOCK_MAX_WRITE_SPIKES 32

class Channel;
class RecordEngine;


class RecordThread : public Thread
{
public:
	RecordThread(const OwnedArray<RecordEngine>& engines);
	~RecordThread();
	void setFileComponents(File rootFolder, int experimentNumber, int recordingNumber);
	void setChannelMap(const Array<int>& channels);
	void setQueuePointers(DataQueue* data, EventMsgQueue* events, SpikeMsgQueue* spikes);

	void run() override;

	void setFirstBlockFlag(bool state);
	void forceCloseFiles();

private:
	void writeData(const AudioSampleBuffer& buffer, int maxSamples, int maxEvents, int maxSpikes, bool lastBlock = false);

	const OwnedArray<RecordEngine>& m_engineArray;
	Array<int> m_channelArray;
	
	DataQueue* m_dataQueue;
	EventMsgQueue* m_eventQueue;
	SpikeMsgQueue *m_spikeQueue;

	std::atomic<bool> m_receivedFirstBlock;
	std::atomic<bool> m_cleanExit;

	File m_rootFolder;
	int m_experimentNumber;
	int m_recordingNumber;
	int m_numChannels;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordThread);
};

#endif  // RECORDTHREAD_H_INCLUDED
