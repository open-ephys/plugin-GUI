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

#include "BinaryFormat/BinaryRecording.h"
#include "EventQueue.h"
#include "DataQueue.h"
#include "../../Utils/Utils.h"
#include <atomic>

#define BLOCK_MAX_WRITE_SAMPLES 4096
#define BLOCK_MAX_WRITE_EVENTS 512
#define BLOCK_MAX_WRITE_SPIKES 512

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
	RecordThread(RecordNode* parentNode, const ScopedPointer<RecordEngine>& engine);

	/** Destructor */
	~RecordThread();

	/** Sets the recording directory, experiment number, and recording number*/
	void setFileComponents(File rootFolder, int experimentNumber, int recordingNumber);

	/** Sets the channel index map */
	void setChannelMap(const Array<int>& channels);

	/** Sets the float timestamp channel map */
	void setFTSChannelMap(const Array<int>& channels);

	/** Sets the pointers to the 3 data queues*/
	void setQueuePointers(DataQueue* data, EventMsgQueue* events, SpikeMsgQueue* spikes);

	/** Runs the thread */
	void run() override;

	/** Sets whether the first block is being written */
	void setFirstBlockFlag(bool state);

	/** Force all open files to close */
	void forceCloseFiles();

	RecordNode *recordNode;
	int64 samplesWritten;

private:

	/** Writes continuous data from an audio buffer */
	void writeData(const AudioSampleBuffer& buffer, 
		int maxSamples, 
		int maxEvents, 
		int maxSpikes, 
		bool lastBlock = false);

	/** Writes continuous data with an array of synchronized timestamps */
	void writeSynchronizedData(const AudioSampleBuffer& dataBuffer, 
		const SynchronizedTimestampBuffer& ftsBuffer, 
		int maxSamples, 
		int maxEvents, 
		int maxSpikes, 
		bool lastBlock = false);

	const ScopedPointer<RecordEngine>& m_engine;
	Array<int> m_channelArray;
	Array<int> m_ftsChannelArray;

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
