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

#include "DiskSpaceChecker.h"
#include "../RecordNode.h"

DiskSpaceChecker::DiskSpaceChecker (RecordNode* rn)
    : recordNode (rn),
      lastUpdateTime (0),
      lastFreeSpace (0),
      dataRate (0),
      smoothedDataRate (0),
      recordingTimeLeftInSeconds (0)
{
    startTimerHz (1);
}

DiskSpaceChecker::~DiskSpaceChecker()
{
    listeners.clear();
}

void DiskSpaceChecker::reset()
{
    lastUpdateTime = Time::getMillisecondCounterHiRes();
    lastFreeSpace = recordNode->getDataDirectory().getBytesFreeOnVolume();
    smoothedDataRate = 0.0f;
}

void DiskSpaceChecker::addListener (DiskSpaceListener* listener)
{
    std::lock_guard<std::mutex> lock (listenerMutex);
    listeners.push_back (listener);
}

void DiskSpaceChecker::removeListener (DiskSpaceListener* listener)
{
    if (listeners.empty())
        return;

    std::lock_guard<std::mutex> lock (listenerMutex);
    listeners.erase (std::remove (listeners.begin(), listeners.end(), listener), listeners.end());
}

void DiskSpaceChecker::timerCallback()
{
    checkDirectoryAndDiskSpace();
}

void DiskSpaceChecker::checkDirectoryAndDiskSpace()
{
    if (! recordNode->getDataDirectory().exists())
    {
        recordNode->getParameter ("directory")->valueChanged();
        notifyDirectoryInvalid();
        return;
    }

    recordNode->getParameter ("directory")->valueChanged();

    int64 bytesFree = recordNode->getDataDirectory().getBytesFreeOnVolume();
    int64 volumeSize = recordNode->getDataDirectory().getVolumeTotalSize();

    float ratio = float (bytesFree) / float (volumeSize);
    if (ratio > 0)
        notifyDiskSpaceRemaining (ratio);

    float currentTime = Time::getMillisecondCounterHiRes();

    if (recordNode->getRecordingStatus())
    {
        // Update data rate and recording time left every 5 seconds
        if (currentTime - lastUpdateTime > 5000.0f)
        {
            dataRate = (lastFreeSpace - bytesFree) / (currentTime - lastUpdateTime); //bytes/ms
            lastUpdateTime = currentTime;
            lastFreeSpace = bytesFree;

            // Smooth with an exponential moving average to prevent a single OS write-back
            // cache flush from transiently inflating dataRate and causing a false positive.
            // Seed the EMA with the first positive measurement so it is useful immediately.
            static constexpr float alpha = 0.25f;
            if (smoothedDataRate <= 0.0f)
                smoothedDataRate = (dataRate > 0.0f) ? dataRate : 0.0f;
            else if (dataRate > 0.0f)
                smoothedDataRate = alpha * dataRate + (1.0f - alpha) * smoothedDataRate;

            if (smoothedDataRate > 0.0f)
            {
                recordingTimeLeftInSeconds = bytesFree / smoothedDataRate / 1000.0f;

                // Stop recording and show warning when less than 5 minutes of disk space left
                if (recordingTimeLeftInSeconds < (60.0f * 5.0f))
                {
                    CoreServices::setRecordingStatus (false);
                    notifyLowDiskSpace();
                }

                update (smoothedDataRate, bytesFree, recordingTimeLeftInSeconds);
            }
        }
    }
    else
    {
        lastUpdateTime = currentTime;
        lastFreeSpace = bytesFree;
        update (0, bytesFree, 0);
    }
}

void DiskSpaceChecker::update (float dataRate, int64 bytesFree, float timeLeft)
{
    std::lock_guard<std::mutex> lock (listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync ([listener, dataRate, bytesFree, timeLeft]()
            {
                try {
                    listener->update (dataRate, bytesFree, timeLeft);
                } catch (const std::exception& e) {
                    LOGE("Error updating disk space listener: ", e.what());
                }
            });
        }
    }
}

void DiskSpaceChecker::notifyDiskSpaceRemaining (float percentage)
{
    std::lock_guard<std::mutex> lock (listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync ([listener, percentage]()
            {
                try {
                    listener->updateDiskSpace (percentage);
                } catch (const std::exception& e) {
                    LOGE("Error updating disk space percentage: ", e.what());
                }
            });
        }
    }
}

void DiskSpaceChecker::notifyDirectoryInvalid()
{
    bool recordingStopped = false;
    if (recordNode->getRecordingStatus())
    {
        CoreServices::setRecordingStatus (false);
        CoreServices::sendStatusMessage ("Recording stopped due to invalid directory.");
        recordingStopped = true;
    }
    std::lock_guard<std::mutex> lock (listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync ([listener, recordingStopped]()
            {
                try {
                    listener->directoryInvalid (recordingStopped);
                } catch (const std::exception& e) {
                    LOGE("Error notifying directory invalid: ", e.what());
                }
            });
        }
    }
}

void DiskSpaceChecker::notifyLowDiskSpace()
{
    std::lock_guard<std::mutex> lock (listenerMutex);
    for (auto listener : listeners)
    {
        if (listener != nullptr)
        {
            juce::MessageManager::callAsync ([listener]()
            {
                try {
                    listener->lowDiskSpace();
                } catch (const std::exception& e) {
                    LOGE("Error notifying low disk space: ", e.what());
                }
            });
        }
    }
}
