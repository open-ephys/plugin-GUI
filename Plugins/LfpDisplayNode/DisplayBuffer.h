/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

namespace LfpViewer {
#pragma  mark - LfpDisplay -
    //==============================================================================
    /**



    */
    class DisplayBuffer : public AudioSampleBuffer
    {
    public:
        DisplayBuffer(int id, String name, float sampleRate);
        ~DisplayBuffer();

        void prepareToUpdate();
        void update();

        void addChannel(String name, int channelNum, int group = 0, float ypos = 0, String structure = "None");

        void initializeEventChannel(int nSamples);
        void finalizeEventChannel(int nSamples);

        void resetIndices();

        void addEvent(int eventTime, int eventChannel, int eventId, int numSourceSamples);
        void addData(AudioSampleBuffer& buffer, int chan, int nSamples);

        CriticalSection* getMutex() { return &displayMutex; }

        struct ChannelMetadata {
            String name = "";
            int group = 0;
            float ypos = 0;
            String structure = "None";
        };

        Array<ChannelMetadata> channelMetadata;

        String name;
        int id;

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

        void addDisplay(int splitID);

        void removeDisplay(int splitID);

        Array<int> displays;

    };
};


#endif //__DISPLAYBUFFER_H__