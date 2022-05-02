/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#include "Synchronizer.h"

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

Synchronizer::Synchronizer()
	: syncWindowLengthMs(50),
	  syncWindowIsOpen(false),
	  firstMainSyncEvent(false),
	  mainStreamId(0),
	  previousMainStreamId(0),
	  streamCount(0),
      acquisitionIsActive(false)
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
    
    std::cout << "Synchronizer adding " << streamId << std::endl;
	// if this is the first stream, make it the main one
	if (mainStreamId == 0)
		mainStreamId = streamId;
    
    std::cout << "Main stream ID: " << mainStreamId << std::endl;

	// if there's a stored value, and it appears again,
	// re-instantiate this as the main stream
	if (streamId == previousMainStreamId)
		mainStreamId = previousMainStreamId;

	// if there's no Stream object yet, create a new one
	if (streams.count(streamId) == 0)
	{
        std::cout << "Creating new Stream object" << std::endl;
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
	
    if (streamId == mainStreamId)
        reset();
    else
        streams[streamId]->reset();
}

int Synchronizer::getSyncLine(uint16 streamId)
{
	return streams[streamId]->syncLine;
}

void Synchronizer::startAcquisition()
{
    acquisitionIsActive = true;
}

void Synchronizer::stopAcquisition()
{
    acquisitionIsActive = false;
}

void Synchronizer::addEvent(uint16 streamId, int ttlLine, int64 sampleNumber)
{

	if (streamCount == 1)
		return;

	//LOGC("Synchronizer received sync event for stream ", streamId);

	if (streams[streamId]->syncLine == ttlLine)
	{

		//LOGC("Correct line!");

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

			//LOGC("[M] Main time: ", mainTimeSec);

			eventCount++;
		}

		//LOGC("[T] Estimated time: ", convertSampleNumberToTimestamp(streamId, sampleNumber));
		//LOGC("[S] Is synchronized: ", streams[streamId]->isSynchronized);
		//LOGC(" ");

	}
}

double Synchronizer::convertSampleNumberToTimestamp(uint16 streamId, int64 sampleNumber)
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

int64 Synchronizer::convertTimestampToSampleNumber(uint16 streamId, double timestamp)
{

    if (streams[streamId]->isSynchronized)
    {
        double t = (timestamp - streams[streamId]->startSampleMainTime) * streams[streamId]->actualSampleRate           + streams[streamId]->startSample;
        
        return (int64) t;
    }
    else {
        return -1;
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

	if (streamId <= 0 || !acquisitionIsActive)
		return SyncStatus::OFF;

	if (isStreamSynced(streamId))
		return SyncStatus::SYNCED;
	else
		return SyncStatus::SYNCING;

}

void Synchronizer::hiResTimerCallback()
{
	stopTimer();

	syncWindowIsOpen = false;

	for (auto [id, stream] : streams)
		stream->closeSyncWindow();

	//LOGD(" ");
}


// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setMainDataStream(uint16 streamId)
{
    LOGD("Setting ", streamId, " as the main stream");
    synchronizer.setMainDataStream(streamId);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setSyncLine(uint16 streamId, int line)
{
    synchronizer.setSyncLine(streamId, line);
}

// called by SyncControlButton
int SynchronizingProcessor::getSyncLine(uint16 streamId)
{
    return synchronizer.getSyncLine(streamId);
}

// called by SyncControlButton
bool SynchronizingProcessor::isMainDataStream(uint16 streamId)
{
    return (streamId == synchronizer.mainStreamId);
}
