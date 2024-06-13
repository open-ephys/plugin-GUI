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

#ifndef DISKSPACECHECKER_H_INCLUDED
#define DISKSPACECHECKER_H_INCLUDED

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "DiskSpaceListener.h"
#include <mutex>
#include <vector>

class RecordNode;

class DiskSpaceChecker : public Timer
{
public:
    /* Constructor */
    DiskSpaceChecker (RecordNode* rn);

    /* Destructor */
    ~DiskSpaceChecker();

    /* Reset */
    void reset();

    /* Timer callback */
    void timerCallback() override;

    /* Adds a listener */
    void addListener (DiskSpaceListener* listener);

    /* Removes a listener */
    void removeListener (DiskSpaceListener* listener);

    void checkDiskSpace();

protected:
    void checkDirectoryAndDiskSpace();
    void update (float dataRate, int64 bytesFree, float timeLeft);
    void notifyDiskSpaceRemaining (float percentage);
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