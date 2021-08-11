#include "Synchronizer.h"

FloatTimestampBuffer::FloatTimestampBuffer(int size)
	: abstractFifo(size)
	, buffer (1, size)
{

	lastTimestamp = 0;

}

FloatTimestampBuffer::~FloatTimestampBuffer() {};

void FloatTimestampBuffer::clear()
{
	buffer.clear();
	abstractFifo.reset();
	lastTimestamp = 0;
}

void FloatTimestampBuffer::resize(int size)
{
	buffer.setSize(1,size);
	lastTimestamp = 0;
}

int FloatTimestampBuffer::addToBuffer(float* data, int64* timestamps, int numItems, int chunkSize)
{

	int startIndex1, blockSize1, startIndex2, blockSize2;

	abstractFifo.prepareToWrite(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

	int bs[3] = { blockSize1, blockSize2, 0 };
	int si[2] = { startIndex1, startIndex2 };
	int cSize = 0;
	int idx = 0;
	int blkIdx;

	if (numItems > 0)
		lastTimestamp = timestamps[numItems-1];

	for (int i = 0; bs[i] != 0; ++i)
	{
		blkIdx = 0;
		for (int j = 0; j < bs[i]; j+= chunkSize)
		{
			cSize = chunkSize <= bs[i] - j ? chunkSize : bs[i] - j;
			buffer.copyFrom(0,
							si[i] + j,
							data + idx,
							cSize);
		}
		idx += cSize;
		blkIdx += cSize;
	}

	abstractFifo.finishedWrite(idx);

	return idx;

}

int FloatTimestampBuffer::getNumSamples() const { return abstractFifo.getNumReady(); }

int FloatTimestampBuffer::readAllFromBuffer(AudioSampleBuffer& data, uint64* timestamp, int maxSize, int dstStartChannel, int numChannels)
{

	int numReady = abstractFifo.getNumReady();
	int numItems = (maxSize < numReady) ? maxSize : numReady;

	int startIndex1, blockSize1, startIndex2, blockSize2;
	abstractFifo.prepareToRead(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

	int channelsToCopy = 1; //TODO: Check this...

	if (blockSize1 > 0)
		data.copyFrom(dstStartChannel, 0, buffer, 0, startIndex1, blockSize1);

	if (blockSize2 > 0)
		data.copyFrom(dstStartChannel, blockSize1, buffer, 0, startIndex2, blockSize2);

	abstractFifo.finishedRead(numItems);

	return numItems;
	
}

Stream::Stream(float expectedSampleRate_)
{
	expectedSampleRate = expectedSampleRate_;
	actualSampleRate = -1.0f;

	sampleRateTolerance = 0.01;

	reset();
}

void Stream::reset()
{

	startSamplePrimaryTime = -1.0f;
	lastSamplePrimaryTime = -1.0f;

	actualSampleRate = -1.0f;
	startSample = -1;
	lastSample = -1;

	receivedEventInWindow = false;
	receivedPrimaryTimeInWindow = false;
	isSynchronized = false;

}

void Stream::setPrimaryTime(float masterTimeSec_)
{
	if (!receivedPrimaryTimeInWindow)
	{
		tempPrimaryTime = masterTimeSec_;
		receivedPrimaryTimeInWindow = true;
	}
	else { // multiple events, something could be wrong
		receivedPrimaryTimeInWindow = false;
	}

}

void Stream::addEvent(int sampleNumber)
{
	if (!receivedEventInWindow)
	{
		tempSampleNum = sampleNumber;
		receivedEventInWindow = true;
	}
	else { // multiple events, something could be wrong
		receivedEventInWindow = false;
	}

}

void Stream::closeSyncWindow()
{
	if (receivedEventInWindow && receivedPrimaryTimeInWindow)
	{
		if (startSample < 0)
		{
			startSample = tempSampleNum;
			startSamplePrimaryTime = tempPrimaryTime;
		}
		else {

			lastSample = tempSampleNum;
			lastSamplePrimaryTime = tempPrimaryTime;

			double tempSampleRate = (lastSample - startSample) / (lastSamplePrimaryTime - startSamplePrimaryTime);

			if (actualSampleRate < 0.0f)
			{
				actualSampleRate = tempSampleRate;
				isSynchronized = true;
				//LOGD("New sample rate: ", actualSampleRate);
			}
			else {
				// check whether the sample rate has changed
				if (abs((tempSampleRate - actualSampleRate) / actualSampleRate) < sampleRateTolerance)
				{
					actualSampleRate = tempSampleRate;
					isSynchronized = true;
					//LOGD("Updated sample rate: ", actualSampleRate);
				}
				else { // reset the clock
					startSample = tempSampleNum;
					startSamplePrimaryTime = tempPrimaryTime;
					isSynchronized = false;
				}
			}
		}
	}

	LOGDD("Subprocessor closed sync window.");

	receivedEventInWindow = false;
	receivedPrimaryTimeInWindow = false;
}

// =======================================================

Synchronizer::Synchronizer(RecordNode* parentNode)
{
	syncWindowLengthMs = 50;
	syncWindowIsOpen = false;
	firstMasterSync = true;
	primaryStreamId = -1;
	node = parentNode;
}

Synchronizer::~Synchronizer()
{

}

void Synchronizer::reset()
{
	
    syncWindowIsOpen = false;
    firstMasterSync = true;
	eventCount = 0;
    
    for (auto const& [id, stream] : streams)
		stream->reset();

}

/*
Adds a new data stream to the synchronizer
If this is the first stream, set it as the primary stream
Default the sync bit for this stream to the first ttlChannel in the stream 
*/
void Synchronizer::addDataStream(uint16 streamId, float expectedSampleRate)
{
	if (primaryStreamId < 0)
		primaryStreamId = streamId;

	streams[streamId] = (new Stream(expectedSampleRate));
	setSyncBit(streamId, 0);
}

void Synchronizer::setPrimaryDataStream(uint16 streamId)
{
	primaryStreamId = streamId;
	reset();
}

void Synchronizer::setSyncBit(uint16 streamId, int ttlChannel)
{
	//LOGD("Set sync channel: {", sourceID, ",", subProcIdx, "}->", ttlChannel);
	streams[streamId]->syncChannel = ttlChannel;
	reset();
}

int Synchronizer::getSyncBit(uint16 streamId)
{
	//LOGD("Get sync channel: {", sourceID, ",", subProcIdx, "}->", subprocessors[sourceID][subProcIdx]->syncChannel);
	return streams[streamId]->syncChannel;
}

void Synchronizer::addEvent(uint64 streamId, int ttlChannel, int sampleNumber)
{

	if (streams[streamId]->syncChannel == ttlChannel)
	{
		
		if (!syncWindowIsOpen)
		{
			openSyncWindow();
		}

		streams[streamId]->addEvent(sampleNumber);

		if (streamId == primaryStreamId)
		{

			//LOGD("Got event on master!");

			float masterTimeSec;

			if (!firstMasterSync)
			{
				masterTimeSec = (sampleNumber - streams[streamId]->startSample) / streams[streamId]->expectedSampleRate;
			}
			else
			{
				masterTimeSec = 0.0f;
				firstMasterSync = false;
			}

			for (auto const& [id, stream] : streams)
				stream->setPrimaryTime(masterTimeSec);

			/*
			if (eventCount % 10 == 0)
				LOGD("Master time: ", masterTimeSec);
			*/
		
			eventCount++;
		}
	}
}

double Synchronizer::convertTimestamp(uint64 streamId, int sampleNumber)
{

	if (streams[streamId]->isSynchronized)
	{
		return (double)(sampleNumber - streams[streamId]->startSample) /
			streams[streamId]->actualSampleRate +
			streams[streamId]->startSamplePrimaryTime;
	}
	else {
		return (double)-1.0;
	}
}

void Synchronizer::openSyncWindow()
{
	startTimer(syncWindowLengthMs);

	syncWindowIsOpen = true;
}

bool Synchronizer::isStreamSynced(uint64 streamId)
{
	return streams[streamId]->isSynchronized;
}

SyncStatus Synchronizer::getStatus(uint64 streamId)
{

	//Deal with synchronization of spikes and events later...
	if (streamId < 0)
		return SyncStatus::OFF;

	if (isStreamSynced(streamId))
		return SyncStatus::SYNCED;
	else if (true)
		return SyncStatus::SYNCING;
	else
		return SyncStatus::OFF;

}

void Synchronizer::hiResTimerCallback()
{
	stopTimer();

	syncWindowIsOpen = false;

	for (auto const& [id, stream] : streams)
		stream->closeSyncWindow();
}