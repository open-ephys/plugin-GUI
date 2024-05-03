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

SyncStream::SyncStream(String streamKey_, float expectedSampleRate_)
	: streamKey(streamKey_),
	  expectedSampleRate(expectedSampleRate_),
	  actualSampleRate(-1.0f),
	  sampleRateTolerance(0.01f),
	  isActive(true)
{

	reset();
}

void SyncStream::reset()
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

void SyncStream::setMainTime(float time)
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

void SyncStream::addEvent(int64 sampleNumber)
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

void SyncStream::closeSyncWindow()
{

	//LOGC("Stream ", streamId, " Closing Sync Window...receivedEvent: ", receivedEventInWindow, ", receivedMainTime: ", receivedMainTimeInWindow);

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
				//LOGC("Stream ", streamId, " new sample rate: ", actualSampleRate);
			}
			else {
				// check whether the sample rate has changed
				if (abs((tempSampleRate - actualSampleRate) / actualSampleRate) < sampleRateTolerance)
				{
					actualSampleRate = tempSampleRate;
					isSynchronized = true;
					//LOGC("Stream ", streamId, " UPDATED sample rate: ", actualSampleRate);

				}
				else 
				{   // reset the clock
					actualSampleRate = -1.0f;
					startSample = tempSampleNum;
					startSampleMainTime = tempMainTime;
					isSynchronized = false;
					//LOGC("Stream ", streamId, " NO LONGER SYNCHRONIZED.");

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
	  mainStreamKey(""),
	  previousMainStreamKey(""),
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
		// TOFIX: Return early if main data stream hasn't been added while loading
		// Ideally shouldn't need this return statement
		if (streams.count(mainStreamKey) == 0) return;
		streams[mainStreamKey]->actualSampleRate = streams[mainStreamKey]->expectedSampleRate;
		streams[mainStreamKey]->isSynchronized = true;
		streams[mainStreamKey]->startSampleMainTime = 0.0;
		streams[mainStreamKey]->startSample = 0;
		LOGD("Only one stream, setting as synchronized.");
	} else {
		for (auto [id, stream] : streams)
			stream->reset();
	}

}

void Synchronizer::prepareForUpdate()
{
	previousMainStreamKey = mainStreamKey;
	//mainStreamKey = "";
	streamCount = 0;

	for (auto [id, stream] : streams)
		stream->isActive = false;
}

void Synchronizer::finishedUpdate()
{

}


void Synchronizer::addDataStream(String streamKey, float expectedSampleRate)
{
    
    //std::cout << "Synchronizer adding " << streamId << std::endl;
	// if this is the first stream, make it the main one
	if (mainStreamKey == "")
		mainStreamKey = streamKey;
    
    //std::cout << "Main stream ID: " << mainStreamId << std::endl;

	// if there's a stored value, and it appears again,
	// re-instantiate this as the main stream
	if (mainStreamKey == previousMainStreamKey)
		mainStreamKey = previousMainStreamKey;

	// if there's no Stream object yet, create a new one
	if (streams.count(streamKey) == 0)
	{
        //std::cout << "Creating new Stream object" << std::endl;
		dataStreamObjects.add(new SyncStream(streamKey, expectedSampleRate));
		streams[streamKey] = dataStreamObjects.getLast();
		setSyncLine(streamKey, 0);
	} else {
		// otherwise, indicate that the stream is currently active
		streams[streamKey]->isActive = true;
	}

	streamCount++;

}

void Synchronizer::setMainDataStream(String streamKey)
{
	mainStreamKey = streamKey;
	reset();
}

void Synchronizer::setSyncLine(String streamKey, int ttlLine)
{
	streams[streamKey]->syncLine = ttlLine;
	
    if (streamKey == mainStreamKey)
        reset();
    else
        streams[streamKey]->reset();
}

int Synchronizer::getSyncLine(String streamKey)
{
	return streams[streamKey]->syncLine;
}

void Synchronizer::startAcquisition()
{
    acquisitionIsActive = true;
    
    reset();
}

void Synchronizer::stopAcquisition()
{
    acquisitionIsActive = false;
}

void Synchronizer::addEvent(String streamKey, int ttlLine, int64 sampleNumber)
{

	if (streamCount == 1 || sampleNumber < 1000)
		return;

	//LOGC("Synchronizer received sync event for stream ", streamId, ", sampleNumber: ", sampleNumber);

	if (streams[streamKey]->syncLine == ttlLine)
	{

		//LOGC("Correct line!");

		if (!syncWindowIsOpen)
		{
			openSyncWindow();
		}

		streams[streamKey]->addEvent(sampleNumber);

		if (streamKey == mainStreamKey)
		{

			float mainTimeSec;

			if (!firstMainSyncEvent)
			{
				mainTimeSec = (sampleNumber - streams[streamKey]->startSample) / streams[streamKey]->expectedSampleRate;
			}
			else
			{
				mainTimeSec = 0.0f;
				firstMainSyncEvent = false;
			}

			for (auto [key, stream] : streams)
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

double Synchronizer::convertSampleNumberToTimestamp(String streamKey, int64 sampleNumber)
{

	if (streams[streamKey]->isSynchronized)
	{
		return (double)(sampleNumber - streams[streamKey]->startSample) /
			streams[streamKey]->actualSampleRate +
			streams[streamKey]->startSampleMainTime;
	}
	else {
		return (double) -1.0f;
	}
}

int64 Synchronizer::convertTimestampToSampleNumber(String streamKey, double timestamp)
{

    if (streams[streamKey]->isSynchronized)
    {
        double t = (timestamp - streams[streamKey]->startSampleMainTime) * streams[streamKey]->actualSampleRate + streams[streamKey]->startSample;
        
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

bool Synchronizer::isStreamSynced(String streamKey)
{
	return streams[streamKey]->isSynchronized;
}

SyncStatus Synchronizer::getStatus(String streamKey)
{

	if (!streamKey.length() || !acquisitionIsActive)
		return SyncStatus::OFF;

	if (isStreamSynced(streamKey))
		return SyncStatus::SYNCED;
	else
		return SyncStatus::SYNCING;

}

void Synchronizer::hiResTimerCallback()
{
	stopTimer();

	syncWindowIsOpen = false;

	for (auto [key, stream] : streams)
		stream->closeSyncWindow();

	//LOGD(" ");
}


// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setMainDataStream(String streamKey)
{
    //LOGD("Setting ", streamId, " as the main stream");
    synchronizer.setMainDataStream(streamKey);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setSyncLine(String streamKey, int line)
{
    synchronizer.setSyncLine(streamKey, line);
}

// called by SyncControlButton
int SynchronizingProcessor::getSyncLine(String streamKey)
{
    return synchronizer.getSyncLine(streamKey);
}

// called by SyncControlButton
bool SynchronizingProcessor::isMainDataStream(String streamKey)
{
    return (streamKey == synchronizer.mainStreamKey);
}
