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

#ifndef SYNCHRONIZER_H_INCLUDED
#define SYNCHRONIZER_H_INCLUDED

#include <algorithm>
#include <chrono>
#include <map>
#include <math.h>
#include <memory>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"

/** 

	Represents a single sync pulse

*/
struct SyncPulse
{
    /** The time (in seconds) since the start of acquisition
        for the pulse's stream
        */
    double localTimestamp;

    /** The sample number at which this event occurred */
    int64 localSampleNumber;

    /** The computer clock time at which this event was received 
        by the synchronizer */
    int64 computerTimeMillis;

    /** Whether the whole pulse has completed (on/off sequence) */
    bool complete = false;

    /** Duration of the event in seconds */
    double duration = -1.0;

    /** Time between the start of this event and the start of the last event */
    double interval = -1.0;

    /** Index of matching pulse in main stream */
    int matchingPulseIndex = -1;

    /** Global timestamp of pulse (if known) */
    double globalTimestamp = -1.0;
};

/**
 *
 * Represents an incoming data stream
 *
 * */
class SyncStream
{
public:
    /** Constructor */
    SyncStream (String streamKey, float expectedSampleRate);

    /** Resets stream parameters before acquisition */
    void reset (String mainStreamKey);

    /** True if this is the main stream */
    bool isMainStream;

    /** Adds a sync event with a particular sample number and state*/
    void addEvent (int64 sampleNumber, bool state);

    /** Global start time of this stream */
    double globalStartTime;

    /** Returns time of latest sync pulse */
    double getLatestSyncTime();

    /** Returns difference between actual and expected sync times */
    double getSyncAccuracy();

    /** Synchronize this stream with another one */
    void syncWith (const SyncStream* mainStream);

    /** Compares pulses; returns true if a match is found */
    bool comparePulses (const SyncPulse& pulse1, const SyncPulse& pulse2);

    /** Stated sample rate for this stream */
    float expectedSampleRate;

    /** Computed sample rate for this stream */
    float actualSampleRate;

    /** TTL line to use for synchronization */
    int syncLine;

    /** true if this stream is successfully synchronized */
    bool isSynchronized;

    /** Holds the unique key for this stream */
    String streamKey;

    /** true if the stream is in active use */
    bool isActive;

    /** The sync pulses for this stream 
    
    The latest pulse is added to the beginning of the vector
    Expired pulses are removed from the end
    */
	std::vector<SyncPulse> pulses;

    /** First pulse matching the global stream */
    SyncPulse firstMatchingPulse;

    /** Determines the maximum size of the sync pulse buffer */
    const int MAX_PULSES_IN_BUFFER = 10;

    /** Threshold for calling pulses synchronous */
    const int MAX_TIME_DIFFERENCE_MS = 50;

    /** Threshold of calling durations equal */
    const double MAX_DURATION_DIFFERENCE_MS = 2;

    /** Threshold of calling intervals equal */
    const double MAX_INTERVAL_DIFFERENCE_MS = 2;

private:

    int64 latestSyncSampleNumber = 0;
    double latestGlobalSyncTime = 0.0;
    int64 latestSyncMillis = -1;
};

enum PLUGIN_API SyncStatus
{
    OFF, //Synchronizer is not running
    SYNCING, //Synchronizer is attempting to sync
    SYNCED //Signal has been synchronized
};

/**

    Uses events across multiple streams
    to synchronize data to a common clock.

    One incoming stream is set as the "main" stream,
    which is used to determine the clock start time.
    The main stream's sample rate is treated as ground
    truth, and all other streams clocks are scaled
    to align with this one.

    The synchronizer works best when the sync line
    has a TTL pulse with a relatively slow inter-event
    interval (e.g. 1 Hz). This interval does not have
    to be regular, however.

*/
class PLUGIN_API Synchronizer : public HighResolutionTimer
{
public:
    /** Constructor*/
    Synchronizer();

    /** Destructor */
    ~Synchronizer() {}

    /** Converts an int64 sample number to a double timestamp */
    double convertSampleNumberToTimestamp (String streamKey, int64 sampleNumber);

    /** Converts a double timestamp to an int64 sample number */
    int64 convertTimestampToSampleNumber (String streamKey, double timestamp);

    /** Returns offset (relative start time) for stream in ms */
    double getStartTime (String streamKey);

    /** Get latest sync time */
    double getLastSyncEvent (String streamKey);

    /** Get the accuracy of synchronization (difference between expected and actual event time) */
    double getAccuracy (String streamKey);

    /** Resets all values when acquisition is re-started */
    void reset();

    /** Sets main stream ID to 0 and stream count to 0*/
    void prepareForUpdate();

    /** Adds a new data stream with an expected sample rate
     *  If the stream already exists, */
    void addDataStream (String streamKey, float expectedSampleRate);

    /** Checks if there is only one stream */
    void finishedUpdate();

    /** Sets the ID of the main data stream */
    void setMainDataStream (String streamKey);

    /** Sets the TTL line to use for synchronization (0-based indexing)*/
    void setSyncLine (String streamKey, int line);

    /** Returns the TTL line to use for synchronization (0-based indexing)*/
    int getSyncLine (String streamKey);

    /** Returns true if a stream is synchronized */
    bool isStreamSynced (String streamKey);

    /** Returns the status (OFF / SYNCING / SYNCED) of a given stream*/
    SyncStatus getStatus (String streamKey);

    /** Adds an event for a stream ID / line combination */
    void addEvent (String streamKey, int ttlLine, int64 sampleNumber, bool state);

    /** Signals start of acquisition */
    void startAcquisition();

    /** Signals start of acquisition */
    void stopAcquisition();

    String mainStreamKey = String();
    String previousMainStreamKey = String();

    /** Total number of streams*/
    int streamCount = 0;

private:
    int eventCount = 0;
    bool acquisitionIsActive = false;

    void hiResTimerCallback();

    CriticalSection synchronizerLock;

    std::map<String, SyncStream*> streams;
    OwnedArray<SyncStream> dataStreamObjects;

};

/**
 
    Abstract base class for Record Node and Event Translator
 
 */
class PLUGIN_API SynchronizingProcessor
{
public:
    /** Sets the main data stream to use for synchronization */
    void setMainDataStream (String streamKey);

    /** Returns true if a stream ID matches the one to use for synchronization*/
    bool isMainDataStream (String streamKey);

    /** Sets the TTL line to use for synchronization*/
    void setSyncLine (String streamKey, int line);

    /** Returns the TTL line to use for synchronization*/
    int getSyncLine (String streamKey);

    /** The synchronizer for this processor */
    Synchronizer synchronizer;
};

#endif //SYNCHRONIZER_H_INCLUDED
