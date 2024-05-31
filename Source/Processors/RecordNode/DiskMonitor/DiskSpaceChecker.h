#ifndef DISKSPACECHECKER_H_INCLUDED
#define DISKSPACECHECKER_H_INCLUDED

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "DiskSpaceListener.h"
#include <vector>
#include <mutex>

class RecordNode;

class DiskSpaceChecker : public Timer
{
public:
    /* Constructor */
    DiskSpaceChecker(RecordNode* rn);

    /* Destructor */
    ~DiskSpaceChecker();

    /* Reset */
    void reset();

    /* Timer callback */
    void timerCallback() override;

    /* Adds a listener */
    void addListener(DiskSpaceListener* listener);

    /* Removes a listener */
    void removeListener(DiskSpaceListener* listener);

    void checkDiskSpace();

protected:

    void checkDirectoryAndDiskSpace();
    void update(float dataRate, int64 bytesFree, float timeLeft);
    void notifyDiskSpaceRemaining(float percentage);
    void notifyDirectoryInvalid();
    void notifyLowDiskSpace();

private:

    RecordNode* recordNode;

    float lastUpdateTime;
    int64 lastFreeSpace;
    float dataRate;
    float recordingTimeLeftInSeconds;

    std::vector<DiskSpaceListener*> listeners;
    std::mutex listenerMutex;
};

#endif // DISKSPACECHECKER_H_INCLUDED