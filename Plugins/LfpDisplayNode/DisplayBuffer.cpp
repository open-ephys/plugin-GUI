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

#include "DisplayBuffer.h"

namespace LfpViewer {
    //==============================================================================
    /**



    */
#define BUFFER_LENGTH 3000

    DisplayBuffer::DisplayBuffer(int id_, String name_, float sampleRate_) : 
        id(id_), name(name_), sampleRate(sampleRate_)
    {
        previousSize = 0;

        const int heapSize = 5000;
        arrayOfOnes = new float[heapSize];
        for (int n = 0; n < heapSize; ++n)
        {
            arrayOfOnes[n] = 1;
        }

        ttlState = 0;
    }

    DisplayBuffer::~DisplayBuffer()
    {
        delete[] arrayOfOnes;
    }

    void DisplayBuffer::prepareToUpdate()
    {
        previousSize = channelNames.size();
        channelNames.clear();
        channelMap.clear();
        numChannels = 0;
    }

    void DisplayBuffer::addChannel(String name, int channelNum)
    {
        channelNames.add(name);
        channelMap.set(channelNum, numChannels);
        numChannels++;
    }

    void DisplayBuffer::update()
    {
        if (numChannels != previousSize)
            setSize(numChannels + 1, BUFFER_LENGTH);

        clear();

        displayBufferIndices.clear();
        displayBufferIndices.insert(channelNames.size() + 1, 0);
    }

    void DisplayBuffer::initializeEventChannel(int nSamples)
    {
        const int chan = numChannels;
        const int samplesLeft = BUFFER_LENGTH - displayBufferIndices[chan];

        if (nSamples < samplesLeft)
        {

            copyFrom(chan,                                      // destChannel
                displayBufferIndices[chan],             // destStartSample
                arrayOfOnes,                               // source
                nSamples,                                  // numSamples
                float(ttlState));     // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            copyFrom(chan,                               // destChannel
                displayBufferIndices[chan],             // destStartSample
                arrayOfOnes,                               // source
                samplesLeft,                               // numSamples
                float(ttlState));                        // gain

            copyFrom(chan,                                      // destChannel
                0,                                         // destStartSample
                arrayOfOnes,                               // source
                extraSamples,                              // numSamples
                float(ttlState));     // gain
        }
    }

    void DisplayBuffer::finalizeEventChannel(int nSamples)
    {
        const int chan = numChannels;
        const int index = displayBufferIndices[chan];
        const int samplesLeft = BUFFER_LENGTH - index;
   
        int newIdx = 0;

        if (nSamples < samplesLeft)
        {
            newIdx = displayBufferIndices[chan] + nSamples;
        }
        else
        {
            newIdx = nSamples - samplesLeft;
        }

        displayBufferIndices.set(chan, newIdx);
    }

    void DisplayBuffer::addEvent(int eventTime, int eventChannel, int eventId, int numSourceSamples)
    {
        const int chan = numChannels;
        const int index = (displayBufferIndices[chan] + eventTime) % BUFFER_LENGTH;
        const int samplesLeft = BUFFER_LENGTH - index;
        const int nSamples = numSourceSamples - eventTime;

        if (eventId == 1)
        {
            ttlState |= (1LL << eventChannel);
        }
        else {
            ttlState &= ~(1LL << eventChannel);
        }

        if (nSamples < samplesLeft)
        {
            copyFrom(chan,                            // destChannel
                index,                                // destStartSample
                arrayOfOnes,                          // source
                nSamples,                             // numSamples
                float(ttlState));                              // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            copyFrom(chan,                                 // destChannel
                index,                                // destStartSample
                arrayOfOnes,                          // source
                samplesLeft,                          // numSamples
                float(ttlState));  // gain

            copyFrom(chan,                                 // destChannel
                0,                                    // destStartSample
                arrayOfOnes,                          // source
                extraSamples,                         // numSamples
                float(ttlState));  // gain
        }
    }

    void DisplayBuffer::addData(AudioSampleBuffer& buffer, int chan, int nSamples)
    {
        ScopedLock displayLock(displayMutex);

        const int samplesLeft = BUFFER_LENGTH - displayBufferIndices[channelMap[chan]];
        
        if (nSamples < samplesLeft)
        {
            copyFrom(channelMap[chan],                      // destChannel
                displayBufferIndices[channelMap[chan]],  // destStartSample
                buffer,                    // source
                chan,                      // source channel
                0,                         // source start sample
                nSamples);                 // numSamples

            displayBufferIndices.set(channelMap[chan], displayBufferIndices[channelMap[chan]] + nSamples);
        }
        else
        {
            const int extraSamples = nSamples - samplesLeft;

            copyFrom(channelMap[chan],                      // destChannel
                displayBufferIndices[channelMap[chan]],  // destStartSample
                buffer,                    // source
                chan,                      // source channel
                0,                         // source start sample
                samplesLeft);              // numSamples

            copyFrom(channelMap[chan],                      // destChannel
                0,                         // destStartSample
                buffer,                    // source
                chan,                      // source channel
                samplesLeft,               // source start sample
                extraSamples);             // numSamples

            displayBufferIndices.set(channelMap[chan], extraSamples);
        }
    }

};
