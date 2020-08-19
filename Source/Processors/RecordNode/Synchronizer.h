#include <chrono>
#include <math.h>
#include <algorithm>
#include <memory>
#include <map>

#include "../../../JuceLibraryCode/JuceHeader.h"

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


class Subprocessor
{
public:
    Subprocessor(float expectedSampleRate);

    void reset();

    float expectedSampleRate;
    float actualSampleRate;

    int startSample;
    int lastSample;

    int syncChannel;

    bool isSynchronized;

    bool receivedEventInWindow;
    bool receivedMasterTimeInWindow;

    float masterIntervalSec;

    int tempSampleNum;
    float tempMasterTime;

    float startSampleMasterTime = -1.0f;
    float lastSampleMasterTime = -1.0f;

    float sampleRateTolerance;

    void addEvent(int sampleNumber);

    void setMasterTime(float time);
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

    void addSubprocessor(int sourceID, int subProcIdx, float expectedSampleRate);
    void setMasterSubprocessor(int sourceID, int subProcIdx);
    void setSyncChannel(int sourceID, int subProcIdx, int ttlChannel);
    int getSyncChannel(int sourceID, int subProcIdx);
    bool isSubprocessorSynced(int sourceID, int subProcIdx);
    SyncStatus getStatus(int sourceID, int subProcIdx);

    void addEvent(int sourceID, int subProcessorID, int ttlChannel, int sampleNumber);

    double convertTimestamp(int sourceID, int subProcID, int sampleNumber);

    std::map<int, std::map<int, Subprocessor*>> subprocessors;

    RecordNode* node;

    int masterProcessor = -1;
    int masterSubprocessor = -1;

    bool isAvailable() { return masterProcessor > 0; };

private:

    int eventCount = 0;

    float syncWindowLengthMs;
    bool syncWindowIsOpen;

    void hiResTimerCallback();

    bool firstMasterSync;

    OwnedArray<Subprocessor> subprocessorArray;
    OwnedArray<FloatTimestampBuffer> ftsBuffer;

    void openSyncWindow();
};