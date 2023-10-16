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

#ifndef SYNCHRONIZER_H_INCLUDED
#define SYNCHRONIZER_H_INCLUDED

#include <chrono>
#include <math.h>
#include <algorithm>
#include <memory>
#include <map>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"



/**
 *
 * Represents an incoming data stream
 *
 * */
class Stream
{
public:

    /** Constructor */
    Stream(String streamKey, float expectedSampleRate);

    /** Resets stream parameters before acquisition */
    void reset();

    /** Adds a sync event with a particular sample number*/
    void addEvent(int64 sampleNumber);

    /** Sets the main clock time for the last event */
    void setMainTime(float time);

    /** Opens sync window (when event is received on any sync line) */
    void openSyncWindow();

    /** Closes sync window (after a delay) */
    void closeSyncWindow();

    /** Stated sample rate for this stream */
    float expectedSampleRate;

    /** Computed sample rate for this stream */
    float actualSampleRate;

    /** Sample index to which all future events are relative to*/
    int64 startSample;

    /** Sample index of most recent event */
    int64 lastSample;

    /** TTL line to use for synchronization */
    int syncLine;

    /** true if this stream is succesfully synchronized */
    bool isSynchronized;

    /** true if a sync event was received in the latest window*/
    bool receivedEventInWindow;

    /** true if a main stream sync event was received in the latest window*/
    bool receivedMainTimeInWindow;

    /** Stores the latest sample number until the sync window is closed*/
    int64 tempSampleNum;

    /** Stores the latest main time until the sync window is closed */
    float tempMainTime;

    /** Holds the main time of the start sample */
    float startSampleMainTime = -1.0f;

    /** Holds the main time of the last sample*/
    float lastSampleMainTime = -1.0f;

    /** If the sample rate changes by more than this amount,
     * consider the stream desynchronized */
    float sampleRateTolerance;

    /** Holds the unique key for this stream */
    String streamKey;

    /** true if the stream is in active use */
    bool isActive;

};

class RecordNode;

enum SyncStatus {
    OFF,        //Synchronizer is not running
    SYNCING,    //Synchronizer is attempting to sync
    SYNCED      //Signal has been synchronized
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
class Synchronizer : public HighResolutionTimer
{
public:

    /** Constructor*/
    Synchronizer();

    /** Destructor */
    ~Synchronizer() { }

    /** Converts an int64 sample number to a double timestamp */
    double convertSampleNumberToTimestamp(String streamKey, int64 sampleNumber);
    
    /** Converts a double timestamp to an int64 sample number */
    int64 convertTimestampToSampleNumber(String streamKey, double timestamp);

    /** Resets all values when acquisition is re-started */
    void reset();

    /** Sets main stream ID to 0 and stream count to 0*/
    void prepareForUpdate();

    /** Adds a new data stream with an expected sample rate
     *  If the stream already exists, */
    void addDataStream(String streamKey, float expectedSampleRate);

    /** Checks if there is only one stream */
    void finishedUpdate();

    /** Sets the ID of the main data stream */
    void setMainDataStream(String streamKey);

    /** Sets the TTL line to use for synchronization (0-based indexing)*/
    void setSyncLine(String streamKey, int line);

    /** Returns the TTL line to use for synchronization (0-based indexing)*/
    int getSyncLine(String streamKey);

    /** Returns true if a stream is synchronized */
    bool isStreamSynced(String streamKey);

    /** Returns the status (OFF / SYNCING / SYNCED) of a given stream*/
    SyncStatus getStatus(String streamKey);

    /** Checks an event for a stream ID / line combination */
    void addEvent(String streamKey, int ttlLine, int64 sampleNumber);
    
    /** Signals start of acquisition */
    void startAcquisition();
    
    /** Signals start of acquisition */
    void stopAcquisition();

    String mainStreamKey = "";
    String previousMainStreamKey = "";

    /** Total number of streams*/
    int streamCount;

    // Not used
    //bool isAvailable() { return mainStreamKey.length() > 0; };

private:

    int eventCount = 0;

    float syncWindowLengthMs;
    bool syncWindowIsOpen;
    bool acquisitionIsActive;

    void hiResTimerCallback();

    bool firstMainSyncEvent;

    std::map<String, Stream*> streams;
    OwnedArray<Stream> dataStreamObjects;

    void openSyncWindow();
};

/**
 
    Abstract base class for Record Node and Event Translator
 
 */
class SynchronizingProcessor
{
public:
    /** Sets the main data stream to use for synchronization */
    void setMainDataStream(String streamKey);

    /** Returns true if a stream ID matches the one to use for sychronization*/
    bool isMainDataStream(String streamKey);

    /** Sets the TTL line to use for synchronization*/
    void setSyncLine(String streamKey, int line);

    /** Returns the TTL line to use for synchronization*/
    int getSyncLine(String streamKey);
    
    /** The synchronizer for this processor */
    Synchronizer synchronizer;
};

#endif //SYNCHRONIZER_H_INCLUDED
