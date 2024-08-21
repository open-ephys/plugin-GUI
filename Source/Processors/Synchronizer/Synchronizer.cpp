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

#include "Synchronizer.h"

SyncStream::SyncStream (String streamKey_, float expectedSampleRate_)
    : streamKey (streamKey_),
      expectedSampleRate (expectedSampleRate_),
      actualSampleRate (-1.0f),
      isActive (true)
{
}

void SyncStream::reset (String mainStreamKey)
{
    isMainStream = (streamKey == mainStreamKey);

    pulses.clear();
    firstMatchingPulse = SyncPulse();

    latestSyncSampleNumber = 0;
    latestGlobalSyncTime = 0.0;
    latestSyncMillis = -1;

    if (isMainStream)
    {
        actualSampleRate = expectedSampleRate;
        globalStartTime = 0.0;
        isSynchronized = true;
    }
    else
    {
        actualSampleRate = -1.0;
        globalStartTime = -1.0;
        isSynchronized = false;
    }
}

void SyncStream::addEvent (int64 sampleNumber, bool state)
{
    //LOGD ("[+] Adding event for stream ", streamKey, " (", sampleNumber, ")");

    if (state) // on event received, pulse initiated
    {
        SyncPulse latestPulse;
        latestPulse.localSampleNumber = sampleNumber;
        latestPulse.localTimestamp = sampleNumber / expectedSampleRate;
        latestPulse.computerTimeMillis = Time::currentTimeMillis();

        pulses.insert (pulses.begin(), latestPulse);
    }
    else // off event received, pulse terminated
    {
        if (pulses.size() > 0)
        {
            SyncPulse& latestPulse = pulses.front();

            latestPulse.complete = true;
            latestPulse.duration =
                (sampleNumber / expectedSampleRate) - latestPulse.localTimestamp;

            if (pulses.size() > 1)
            {
                latestPulse.interval = latestPulse.localTimestamp - pulses[1].localTimestamp;
            }
        }

        if (pulses.size() > MAX_PULSES_IN_BUFFER)
        {
            pulses.pop_back();
        }
    }
}

double SyncStream::getLatestSyncTime()
{
    //LOGD ("Getting latest sync time for stream ", streamKey, "...");
    //LOGD ("Time::currentTimeMillis(): ", Time::currentTimeMillis());
    //LOGD ("latestSyncMillis: ", latestSyncMillis);
   

    if (latestSyncMillis != -1)
    {
       // LOGD ("Returning: ", double (Time::currentTimeMillis() - latestSyncMillis) / 1000.0f);
        return double (Time::currentTimeMillis() - latestSyncMillis) / 1000.0f;
	}
    else
    {
       // LOGD ("Returning: ", -1);
		return -1.0;
	}
}

double SyncStream::getSyncAccuracy()
{
    if (pulses.size() > 0)
    {

        //LOGD ("Sync accuracy for stream ", streamKey);

        //LOGD ("latestSyncSampleNumber: ", latestSyncSampleNumber);
        //LOGD ("latestGlobalSyncTime: ", latestGlobalSyncTime);
        //LOGD ("globalStartTime: ", globalStartTime);
        //LOGD ("actualSampleRate: ", actualSampleRate);
        double estimatedGlobalTime = latestSyncSampleNumber / actualSampleRate + globalStartTime;
        //LOGD ("estimatedGlobalTime: ", estimatedGlobalTime);
        //LOGD ("difference: ", latestGlobalSyncTime - estimatedGlobalTime);

        return (estimatedGlobalTime - latestGlobalSyncTime) * 1000;
    }
    else
    {
        return 0.0;
    }
}

void SyncStream::syncWith (const SyncStream* mainStream)
{
    //LOGD ("Synchronizing ", streamKey, " with ", mainStream->streamKey, "...")

    if (mainStream->pulses.size() < 2 || pulses.size() < 2)
    {
        //LOGD ("Not enough pulses to synchronize.");
        return;
    }

    int localIndex = 0;
    bool foundMatchingPulse = false;

    for (auto& pulse : pulses) // loop through pulses in this stream
    {
        if (pulse.complete)
        {
            int index = 0;

            for (auto& mainPulse : mainStream->pulses) // loop through pulses in main stream
            {
                if (mainPulse.complete)
                {
                    if (comparePulses (pulse, mainPulse)) // putative match
                    {
                        if (pulses.size() > localIndex + 3 && mainStream->pulses.size() > index + 3)
                        {
                            if (comparePulses (pulses[localIndex + 1], mainStream->pulses[index + 1])
                                && comparePulses (pulses[localIndex + 2], mainStream->pulses[index + 2])
                                && comparePulses (pulses[localIndex + 3], mainStream->pulses[index + 3]))
                            {
                                pulse.matchingPulseIndex = index;
                                pulse.globalTimestamp = mainPulse.localTimestamp;
                                latestSyncSampleNumber = pulse.localSampleNumber;
                                latestGlobalSyncTime = pulse.globalTimestamp;
                                latestSyncMillis = pulse.computerTimeMillis;
                                //LOGD ("Pulse at ", pulse.localTimestamp, " matches with 4 main pulses at ", index);
                                //LOGD ("latestSyncSampleNumber: ", latestSyncSampleNumber, ", latestGlobalSyncTime: ", latestGlobalSyncTime);


                                if (firstMatchingPulse.complete == false)
                                {
                                    firstMatchingPulse.localTimestamp = pulse.localTimestamp;
                                    firstMatchingPulse.globalTimestamp = mainPulse.localTimestamp;
                                    firstMatchingPulse.localSampleNumber = pulse.localSampleNumber;
                                    firstMatchingPulse.complete = true;
                                    //LOGD ("Time of first matching pulse: ", firstMatchingPulse.localTimestamp, " (local), ", firstMatchingPulse.globalTimestamp, " (global)");
                                }
                            }
                        }

                        break;
                    }
                }

                index++;
            }
        }

        if (pulse.matchingPulseIndex != -1)
        {
            foundMatchingPulse = true;
            break;
        }

        localIndex++;
    }

    if (foundMatchingPulse)
    {
        if (firstMatchingPulse.complete && (pulses[localIndex].localTimestamp - firstMatchingPulse.localTimestamp) > 1.0)
        {
            //LOGD ("pulses[localIndex].localSampleNumber: ", pulses[localIndex].localSampleNumber, ", firstMatchingPulse.localSampleNumber: ", firstMatchingPulse.localSampleNumber);
            //LOGD ("pulses[localIndex].localTimestamp: ", pulses[localIndex].localTimestamp, ", firstMatchingPulse.localTimestamp: ", firstMatchingPulse.localTimestamp);
            //LOGD ("pulses[localIndex].globalTimestamp: ", pulses[localIndex].globalTimestamp, ", firstMatchingPulse.globalTimestamp: ", firstMatchingPulse.globalTimestamp);

            float estimatedActualSampleRate = (pulses[localIndex].localSampleNumber - firstMatchingPulse.localSampleNumber) / (pulses[localIndex].globalTimestamp - firstMatchingPulse.globalTimestamp);

            double estimatedGlobalStartTime = pulses[localIndex].globalTimestamp - pulses[localIndex].localSampleNumber / actualSampleRate;

            if (std::abs (estimatedActualSampleRate - expectedSampleRate) / expectedSampleRate < 0.001)
            {
                actualSampleRate = estimatedActualSampleRate;

                if (std::abs (estimatedGlobalStartTime) < 0.1)
                {
                    if (! isSynchronized)

                    {
                        globalStartTime = estimatedGlobalStartTime;
                        isSynchronized = true;
                    }
                }
                else
                {
                    //LOGD ("Estimated global start time of ", estimatedGlobalStartTime, " is out of bounds. Ignoring.")
                }
            }
            else
            {
                //LOGD ("Estimated sample rate of ", estimatedActualSampleRate, " is out of bounds. Ignoring.");
                return;
            }

            //LOGD ("Stream ", streamKey, " synchronized with main stream. Sample rate: ", actualSampleRate, ", start time: ", globalStartTime);
        }
        else
        {
            //LOGD ("At least 1 second must elapse before synchronization can be attempted.");
        }
    }

    
}

bool SyncStream::comparePulses (const SyncPulse& pulse1, const SyncPulse& pulse2)
{
    if (std::abs (pulse1.computerTimeMillis - pulse2.computerTimeMillis) < MAX_TIME_DIFFERENCE_MS)
    {
        if (std::abs (pulse1.duration - pulse2.duration) < MAX_DURATION_DIFFERENCE_MS)
        {
            if (std::abs (pulse1.interval - pulse2.interval) < MAX_INTERVAL_DIFFERENCE_MS)
            {
                return true;
            }
        }
    }

    return false;
}

// =======================================================

Synchronizer::Synchronizer()
{
}

void Synchronizer::reset()
{
    for (auto [id, stream] : streams)
        stream->reset (mainStreamKey);
}

void Synchronizer::prepareForUpdate()
{
    previousMainStreamKey = mainStreamKey;

    streamCount = 0;

    for (auto [id, stream] : streams)
        stream->isActive = false;
}

void Synchronizer::finishedUpdate()
{
}

void Synchronizer::addDataStream (String streamKey, float expectedSampleRate)
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
    if (streams.count (streamKey) == 0)
    {
        //std::cout << "Creating new Stream object" << std::endl;
        dataStreamObjects.add (new SyncStream (streamKey, expectedSampleRate));
        streams[streamKey] = dataStreamObjects.getLast();
        setSyncLine (streamKey, 0);
    }
    else
    {
        // otherwise, indicate that the stream is currently active
        streams[streamKey]->isActive = true;
    }

    streamCount++;
}

void Synchronizer::setMainDataStream (String streamKey)
{
    mainStreamKey = streamKey;
    reset();
}

void Synchronizer::setSyncLine (String streamKey, int ttlLine)
{
    streams[streamKey]->syncLine = ttlLine;

    if (streamKey == mainStreamKey)
        reset();
    else
        streams[streamKey]->reset (mainStreamKey);
}

int Synchronizer::getSyncLine (String streamKey)
{
    return streams[streamKey]->syncLine;
}

void Synchronizer::startAcquisition()
{
    reset();

    acquisitionIsActive = true;

    startTimer (1000);
}

void Synchronizer::stopAcquisition()
{
    acquisitionIsActive = false;

    stopTimer();
}

void Synchronizer::addEvent (String streamKey,
                             int ttlLine,
                             int64 sampleNumber,
                             bool state)
{
    const ScopedLock sl (synchronizerLock);

    if (streamCount == 1 || sampleNumber < 1000)
        return;

    if (streams[streamKey]->syncLine == ttlLine)
    {
        streams[streamKey]->addEvent (sampleNumber, state);
    }
}

double Synchronizer::convertSampleNumberToTimestamp (String streamKey, int64 sampleNumber)
{
    if (streams[streamKey]->isSynchronized)
    {
        return (double) sampleNumber / streams[streamKey]->actualSampleRate + streams[streamKey]->globalStartTime;
    }
    else
    {
        return (double) -1.0f;
    }
}

int64 Synchronizer::convertTimestampToSampleNumber (String streamKey, double timestamp)
{
    if (streams[streamKey]->isSynchronized)
    {
        double t = (timestamp - streams[streamKey]->globalStartTime) * streams[streamKey]->actualSampleRate;

        return (int64) t;
    }
    else
    {
        return -1;
    }
}

double Synchronizer::getStartTime (String streamKey)
{
	return streams[streamKey]->globalStartTime * 1000;
}

double Synchronizer::getLastSyncEvent (String streamKey)
{
    return streams[streamKey]->getLatestSyncTime();
}

double Synchronizer::getAccuracy (String streamKey)
{

    if(! streams[streamKey]->isSynchronized)
		return 0.0;
    else
    {
        if(streamKey == mainStreamKey)
			return 0.0;
        else
        {
            return streams[streamKey]->getSyncAccuracy();
        }
        
    }

}

bool Synchronizer::isStreamSynced (String streamKey)
{
    return streams[streamKey]->isSynchronized;
}

SyncStatus Synchronizer::getStatus (String streamKey)
{
    if (! streamKey.length() || ! acquisitionIsActive)
        return SyncStatus::OFF;

    if (isStreamSynced (streamKey))
        return SyncStatus::SYNCED;
    else
        return SyncStatus::SYNCING;
}

void Synchronizer::hiResTimerCallback()
{

    const ScopedLock sl (synchronizerLock);

    for (auto [key, stream] : streams)
    {
        if (key != mainStreamKey)
        {
            stream->syncWith (streams[mainStreamKey]);
        }
    }
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setMainDataStream (String streamKey)
{
    //LOGD("Setting ", streamId, " as the main stream");
    synchronizer.setMainDataStream (streamKey);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setSyncLine (String streamKey, int line)
{
    synchronizer.setSyncLine (streamKey, line);
}

// called by SyncControlButton
int SynchronizingProcessor::getSyncLine (String streamKey)
{
    return synchronizer.getSyncLine (streamKey);
}

// called by SyncControlButton
bool SynchronizingProcessor::isMainDataStream (String streamKey)
{
    return (streamKey == synchronizer.mainStreamKey);
}
