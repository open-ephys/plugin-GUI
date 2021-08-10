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


class Stream
{
public:
    Stream(float expectedSampleRate);

    void reset();

    float expectedSampleRate;
    float actualSampleRate;

    int startSample;
    int lastSample;

    int syncChannel;

    bool isSynchronized;

    bool receivedEventInWindow;
    bool receivedPrimaryTimeInWindow;

    float primaryIntervalSec;

    int tempSampleNum;
    float tempPrimaryTime;

    float startSamplePrimaryTime = -1.0f;
    float lastSamplePrimaryTime = -1.0f;

    float sampleRateTolerance;

    void addEvent(int sampleNumber);

    void setPrimaryTime(float time);
    void openSyncWindow();
    void closeSyncWindow();
};

class RecordNode;

enum SyncStatus { 
    OFF,        //Synchronizer is not running
    SYNCING,    //Synchronizer is attempting to sync
    SYNCED      //Signal has been synchronized
};

class Synchronizer : public HighResolutionTimer
{
public:

    Synchronizer(RecordNode* parent);
    ~Synchronizer();
    
    void reset();

    void addDataStream(uint16 streamId, float expectedSampleRate);
    void setPrimaryDataStream(uint16 streamId);

    void setSyncBit(uint16 streamId, int bit);
    int getSyncBit(uint16 streamId);

    bool isStreamSynced(uint64 streamId);
    SyncStatus getStatus(uint64 streamId);

    void addEvent(uint64 streamId, int ttlChannel, int sampleNumber);

    double convertTimestamp(uint64 streamId, int sampleNumber);

    RecordNode* node;

    int primaryStreamId = -1;

    bool isAvailable() { return primaryStreamId > 0; };

private:

    int eventCount = 0;

    float syncWindowLengthMs;
    bool syncWindowIsOpen;

    void hiResTimerCallback();

    bool firstMasterSync;

    std::map<int,Stream*> streams;

    OwnedArray<FloatTimestampBuffer> ftsBuffer;

    void openSyncWindow();
};