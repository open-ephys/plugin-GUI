#include "DiskSpaceChecker.h"
#include "../RecordNode.h"

DiskSpaceChecker::DiskSpaceChecker(RecordNode* rn)
    : recordNode(rn),
      lastUpdateTime (0),
      lastFreeSpace (0),
      dataRate (0),
      recordingTimeLeftInSeconds (0)
{
    startTimerHz (1);
}

DiskSpaceChecker::~DiskSpaceChecker() {}

void DiskSpaceChecker::reset()
{
    lastUpdateTime = Time::getMillisecondCounterHiRes();
    lastFreeSpace = recordNode->getDataDirectory().getBytesFreeOnVolume();
}

void DiskSpaceChecker::addListener(DiskSpaceListener* listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    listeners.push_back(listener);
}

void DiskSpaceChecker::removeListener(DiskSpaceListener* listener)
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
}

void DiskSpaceChecker::timerCallback()
{
    checkDirectoryAndDiskSpace();
}

void DiskSpaceChecker::checkDirectoryAndDiskSpace() {

    if (!recordNode->getParameter("directory")->isValid())
    {
        notifyDirectoryInvalid();
        return;
    }

    recordNode->getParameter ("directory")->valueChanged();

    int64 bytesFree = recordNode->getDataDirectory().getBytesFreeOnVolume();
    int64 volumeSize = recordNode->getDataDirectory().getVolumeTotalSize();

    float ratio = float (bytesFree) / float (volumeSize);
    if (ratio > 0)
        notifyDiskSpaceRemaining (1.0f - ratio);

    float currentTime = Time::getMillisecondCounterHiRes();

    if (recordNode->getRecordingStatus())
    {
        // Update data rate and recording time left every 5 seconds
        if (currentTime - lastUpdateTime > 5000.0f)
        {
            dataRate = (lastFreeSpace - bytesFree) / (currentTime - lastUpdateTime); //bytes/ms
            lastUpdateTime = currentTime;
            lastFreeSpace = bytesFree;

            recordingTimeLeftInSeconds = bytesFree / dataRate / 1000.0f;

            // Stop recording and show warning when less than 5 minutes of disk space left
            if (dataRate > 0.0f && recordingTimeLeftInSeconds < (60.0f * 5.0f))
            {
                CoreServices::setRecordingStatus (false);
                notifyLowDiskSpace();
            }

            if (dataRate > 0.0f)
            {
                update(dataRate, bytesFree, recordingTimeLeftInSeconds);
            }
        }
    }
    else 
    {
        lastUpdateTime = currentTime;
        lastFreeSpace = bytesFree;
        update(0, bytesFree, 0);
    }

}

void DiskSpaceChecker::update(float dataRate, int64 bytesFree, float timeLeft)
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync([listener, dataRate, bytesFree, timeLeft]() { listener->update(dataRate, bytesFree, timeLeft); });
        }
    }
}

void DiskSpaceChecker::notifyDiskSpaceRemaining(float percentage)
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync([listener, percentage]() { listener->updateDiskSpace(percentage); });
        }
    }
}

void DiskSpaceChecker::notifyDirectoryInvalid()
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync([listener]() { listener->directoryInvalid(); });
        }
    }
}

void DiskSpaceChecker::notifyLowDiskSpace()
{
    std::lock_guard<std::mutex> lock(listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync([listener]() { listener->lowDiskSpace(); });
        }
    }
}
