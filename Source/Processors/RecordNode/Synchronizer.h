#include <chrono>
#include <math.h>
#include <algorithm>
#include <memory>
#include <map>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"

class FloatTimestampBuffer
{
public:
    FloatTimestampBuffer(int size);
    ~FloatTimestampBuffer();

    void clear();

    int addToBuffer(float* data, int64* timestamps, int numItems, int chunkSize=1);
    int getNumSamples() const;
    int readAllFromBuffer(AudioSampleBuffer& data, uint64* timestamp, int maxSize, int dstStartChannel, int numChannels);
    void resize(int size);

private:


    int64 lastTimestamp;
    AbstractFifo abstractFifo;
    AudioSampleBuffer buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatTimestampBuffer);

};

/**
 *
 * Represents an incoming data stream
 *
 * */
class Stream
{
public:

    /** Constructor */
    Stream(uint16 streamId, float expectedSampleRate);

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

    /** Holds the ID for this stream */
    uint16 streamId;

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
    Synchronizer(RecordNode* parent);

    /** Destructor */
    ~Synchronizer() { }

    /** Converts an integer (index) timestamp to a float (synchronized) timestamp */
    double convertTimestamp(uint16 streamId, int64 sampleNumber);

    /** Resets all values when acquisition is re-started */
    void reset();

    /** Sets main stream ID to 0*/
    void prepareForUpdate();

    /** Adds a new data stream with an expected sample rate
     *  If the stream already exists, */
    void addDataStream(uint16 streamId, float expectedSampleRate);

    /** Sets the ID of the main data stream */
    void setMainDataStream(uint16 streamId);

    /** Sets the TTL line to use for synchronization (0-based indexing)*/
    void setSyncLine(uint16 streamId, int line);

    /** Returns the TTL line to use for synchronization (0-based indexing)*/
    int getSyncLine(uint16 streamId);

    /** Returns true if a stream is synchronized */
    bool isStreamSynced(uint16 streamId);

    /** Returns the status (OFF / SYNCING / SYNCED) of a given stream*/
    SyncStatus getStatus(uint16 streamId);

    /** Checks an event for a stream ID / line combination */
    void addEvent(uint16 streamId, int ttlLine, int64 sampleNumber);

    RecordNode* node;

    uint16 mainStreamId = 0;
    uint16 previousMainStreamId = 0;

    bool isAvailable() { return mainStreamId > 0; };

private:

    int eventCount = 0;

    float syncWindowLengthMs;
    bool syncWindowIsOpen;

    void hiResTimerCallback();

    bool firstMainSyncEvent;

    std::map<uint16, Stream*> streams;
    OwnedArray<Stream> dataStreamObjects;

    OwnedArray<FloatTimestampBuffer> ftsBuffer;

    void openSyncWindow();
};