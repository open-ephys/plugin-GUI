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

Stream::Stream(uint16 streamId_, float expectedSampleRate_)
	: streamId(streamId_),
	  expectedSampleRate(expectedSampleRate_),
	  actualSampleRate(-1.0f),
	  sampleRateTolerance(0.01f),
	  isActive(true)
{

	reset();
}

void Stream::reset()
{

	startSampleMainTime = -1.0f;
	lastSampleMainTime = -1.0f;

	actualSampleRate = -1.0f;
	startSample = -1;
	lastSample = -1;

	receivedEventInWindow = false;
	receivedMainTimeInWindow = false;
	isSynchronized = false;

}

void Stream::setMainTime(float time)
{
	if (!receivedMainTimeInWindow)
	{
		tempMainTime = time;
		receivedMainTimeInWindow = true;
		//LOGD("Stream ", streamId, " received main time: ", time);

	}
	else { // multiple events, something could be wrong
		receivedMainTimeInWindow = false;
	}

}

void Stream::addEvent(int64 sampleNumber)
{
	//LOGD("[+] Adding event for stream ", streamId, " (", sampleNumber, ")");
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
	if (receivedEventInWindow && receivedMainTimeInWindow)
	{
		if (startSample < 0)
		{
			startSample = tempSampleNum;
			startSampleMainTime = tempMainTime;
		}
		else {

			lastSample = tempSampleNum;
			lastSampleMainTime = tempMainTime;

			double tempSampleRate = (lastSample - startSample) / (lastSampleMainTime - startSampleMainTime);

			if (actualSampleRate < 0.0f)
			{
				actualSampleRate = tempSampleRate;
				isSynchronized = true;
				//LOGD("Stream ", streamId, " new sample rate: ", actualSampleRate);
			}
			else {
				// check whether the sample rate has changed
				if (abs((tempSampleRate - actualSampleRate) / actualSampleRate) < sampleRateTolerance)
				{
					actualSampleRate = tempSampleRate;
					isSynchronized = true;
					//LOGD("Stream ", streamId, " UPDATED sample rate: ", actualSampleRate);

				}
				else { // reset the clock
					startSample = tempSampleNum;
					startSampleMainTime = tempMainTime;
					isSynchronized = false;
					//LOGD("Stream ", streamId, " NO LONGER SYNCHRONIZED.");

				}
			}
		}
	}

	//LOGD("[x] Stream ", streamId, " closed sync window.");

	receivedEventInWindow = false;
	receivedMainTimeInWindow = false;
}

// =======================================================

Synchronizer::Synchronizer(RecordNode* parentNode)
	: syncWindowLengthMs(50),
	  syncWindowIsOpen(false),
	  firstMainSyncEvent(false),
	  mainStreamId(0),
	  previousMainStreamId(0),
	  node(parentNode),
	  streamCount(0)
{
}

void Synchronizer::reset()
{

    syncWindowIsOpen = false;
    firstMainSyncEvent = true;
	eventCount = 0;

	if (streamCount == 1)
	{
		streams[mainStreamId]->actualSampleRate = streams[mainStreamId]->expectedSampleRate;
		streams[mainStreamId]->isSynchronized = true;
		streams[mainStreamId]->startSampleMainTime = 0.0;
		streams[mainStreamId]->startSample = 0;
		LOGD("Only one stream, setting as synchronized.");
	} else {
		for (auto [id, stream] : streams)
			stream->reset();
	}



}

void Synchronizer::prepareForUpdate()
{
	previousMainStreamId = mainStreamId;
	mainStreamId = 0;
	streamCount = 0;

	for (auto [id, stream] : streams)
		stream->isActive = false;
}

void Synchronizer::finishedUpdate()
{

}


void Synchronizer::addDataStream(uint16 streamId, float expectedSampleRate)
{
	// if this is the first stream, make it the main one
	if (mainStreamId == 0)
		mainStreamId = streamId;

	// if there's a stored value, and it appears again,
	// re-instantiate this as the main stream
	if (streamId == previousMainStreamId)
		mainStreamId = previousMainStreamId;

	// if there's no Stream object yet, create a new one
	if (streams.count(streamId) == 0)
	{
		dataStreamObjects.add(new Stream(streamId, expectedSampleRate));
		streams[streamId] = dataStreamObjects.getLast();
		setSyncLine(streamId, 0);
	} else {
		// otherwise, indicate that the stream is currently active
		streams[streamId]->isActive = true;
	}

	streamCount++;

}

void Synchronizer::setMainDataStream(uint16 streamId)
{
	mainStreamId = streamId;
	reset();
}

void Synchronizer::setSyncLine(uint16 streamId, int ttlLine)
{
	streams[streamId]->syncLine = ttlLine;
	reset();
}

int Synchronizer::getSyncLine(uint16 streamId)
{
	return streams[streamId]->syncLine;
}

void Synchronizer::addEvent(uint16 streamId, int ttlLine, int64 sampleNumber)
{

	if (streamCount == 1)
		return;

	if (streams[streamId]->syncLine == ttlLine)
	{

		//LOGD("Synchronizer received sync event for stream ", streamId);

		if (!syncWindowIsOpen)
		{
			openSyncWindow();
		}

		streams[streamId]->addEvent(sampleNumber);

		if (streamId == mainStreamId)
		{

			float mainTimeSec;

			if (!firstMainSyncEvent)
			{
				mainTimeSec = (sampleNumber - streams[streamId]->startSample) / streams[streamId]->expectedSampleRate;
			}
			else
			{
				mainTimeSec = 0.0f;
				firstMainSyncEvent = false;
			}

			for (auto [id, stream] : streams)
			{
				if (stream->isActive)
					stream->setMainTime(mainTimeSec);
			}


			///if (eventCount % 10 == 0)
			//LOGD("[M] Main time: ", mainTimeSec);
			//

			eventCount++;
		}

		//LOGD("[T] Estimated time: ", convertTimestamp(streamId, sampleNumber));
		//LOGD("[S] Is synchronized: ", streams[streamId]->isSynchronized);

	}
}

double Synchronizer::convertTimestamp(uint16 streamId, int64 sampleNumber)
{

	if (streams[streamId]->isSynchronized)
	{
		return (double)(sampleNumber - streams[streamId]->startSample) /
			streams[streamId]->actualSampleRate +
			streams[streamId]->startSampleMainTime;
	}
	else {
		return (double) -1.0f;
	}
}

void Synchronizer::openSyncWindow()
{
	startTimer(syncWindowLengthMs);

	syncWindowIsOpen = true;
}

bool Synchronizer::isStreamSynced(uint16 streamId)
{
	return streams[streamId]->isSynchronized;
}

SyncStatus Synchronizer::getStatus(uint16 streamId)
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

	for (auto [id, stream] : streams)
		stream->closeSyncWindow();

	//LOGD(" ");
}