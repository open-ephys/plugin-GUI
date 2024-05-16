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

#ifndef __DISPLAYBUFFER_H__
#define __DISPLAYBUFFER_H__

#include <ProcessorHeaders.h>

#include <map>

namespace LfpViewer
{
#pragma mark - LfpDisplay -
//==============================================================================
/**

        Temporary buffer for incoming continous and event data.

        Data is transferred from the displayBuffer to the screenBuffer, 
        after which it is drawn.

    */
class TESTABLE DisplayBuffer : public AudioBuffer<float>
{
public:
    /** Constructor */
    DisplayBuffer (int id, String name, float sampleRate);

    /** Destructor */
    ~DisplayBuffer();

    /** Resets buffer*/
    void prepareToUpdate();

    /** Updates buffer settings*/
    void update();

    /** Adds a continuous channel to the buffer*/
    void addChannel (String name,
                     int channelNum,
                     ContinuousChannel::Type channelType,
                     bool isRecorded,
                     int group = 0,
                     float ypos = 0,
                     String description = "",
                     String structure = "None");

    /** Initializes the event channel at the start of each buffer */
    void initializeEventChannel (int nSamples);

    /** Cleans up the event channel at the end of each buffer*/
    void finalizeEventChannel (int nSamples);

    /** Sets buffer indices to zero */
    void resetIndices();

    /** Adds an event for a particular time and channel (line) */
    void addEvent (int eventTime, int eventChannel, int eventId, int numSourceSamples);

    /** Adds continuous data*/
    void addData (AudioBuffer<float>& buffer, int chan, int nSamples);

    CriticalSection* getMutex() { return &displayMutex; }

    struct ChannelMetadata
    {
        String name = "";
        int group = 0;
        float ypos = 0;
        String structure = "None";
        ContinuousChannel::Type type;
        bool isRecorded = false;
        String description = "";
    };

    Array<ChannelMetadata> channelMetadata;

    String name;
    int id;
    String streamKey;

    int64 bufferIndex;
    std::map<int, int> channelMap;

    Array<int> displayBufferIndices;

    int numChannels;

    float* arrayOfOnes;

    int previousSize;

    float sampleRate;

    uint64 ttlState;

    int triggerChannel;

    int latestTriggerTime;
    int latestCurrentTriggerTime;

    CriticalSection displayMutex;

    bool isNeeded;

    void addDisplay (int splitID);

    void removeDisplay (int splitID);

    Array<int> displays;
};
}; // namespace LfpViewer

#endif //__DISPLAYBUFFER_H__
