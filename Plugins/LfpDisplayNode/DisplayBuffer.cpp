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
#define BUFFER_LENGTH_S 1.0f

    DisplayBuffer::DisplayBuffer(int id_, String name_, float sampleRate_) : 
        id(id_), name(name_), sampleRate(sampleRate_), isNeeded(true)
    {
        previousSize = 0;
        numChannels = 0;

        const int heapSize = 5000;
        arrayOfOnes = new float[heapSize];
        for (int n = 0; n < heapSize; ++n)
        {
            arrayOfOnes[n] = 1;
        }

        ttlState = 0;

       // std::cout << "Subprocessor " << id << " has " << displays.size() << " displays." << std::endl;
        //std::cout << "Display buffer sample rate: " << sampleRate << std::endl;
    }

    DisplayBuffer::~DisplayBuffer()
    {
        delete[] arrayOfOnes;
    }

    void DisplayBuffer::prepareToUpdate()
    {
        previousSize = numChannels;
        channelMetadata.clear();
        channelMap.clear();
        numChannels = 0;

        isNeeded = false;
    }

    void DisplayBuffer::addChannel(String name, int channelNum, int group, float ypos, String structure)
    {
        ChannelMetadata metadata = ChannelMetadata();
        metadata.name = name;
        metadata.group = group;
        metadata.ypos = ypos;
        metadata.structure = structure;

        channelMetadata.add(metadata);
        channelMap[channelNum] = numChannels;
        numChannels++;

        isNeeded = true;

       // std::cout << "Adding channel " << name << " with index " << numChannels << "; ";
    }

    void DisplayBuffer::update()
    {
            
        if (numChannels != previousSize)
            setSize(numChannels + 1, int(sampleRate * BUFFER_LENGTH_S));

        clear();

        displayBufferIndices.clear();

        for (int i = 0; i <= numChannels; i++)
            displayBufferIndices.set(i, 0);
    }

    void DisplayBuffer::resetIndices()
    {
        for (int i = 0; i <= numChannels; i++)
            displayBufferIndices.set(i, 0);
    }

    void DisplayBuffer::addDisplay(int splitID)
    {
        if (!displays.contains(splitID))
             displays.add(splitID);

        //std::cout << "Subprocessor " << id << " has " << displays.size() << " displays." << std::endl;
    }

    void DisplayBuffer::removeDisplay(int splitID)
    {
        if (displays.contains(splitID))
            displays.remove(displays.indexOf(splitID));

       // std::cout << "Subprocessor " << id << " has " << displays.size() << " displays." << std::endl;

    }

    void DisplayBuffer::initializeEventChannel(int nSamples)
    {
        if (displays.size() == 0)
            return;

        const int samplesLeft = getNumSamples() - displayBufferIndices[numChannels];

        if (nSamples < samplesLeft)
        {

            copyFrom(numChannels,                                      // destChannel
                displayBufferIndices[numChannels],             // destStartSample
                arrayOfOnes,                               // source
                nSamples,                                  // numSamples
                float(ttlState));     // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            copyFrom(numChannels,                               // destChannel
                displayBufferIndices[numChannels],             // destStartSample
                arrayOfOnes,                               // source
                samplesLeft,                               // numSamples
                float(ttlState));                        // gain

            copyFrom(numChannels,                                      // destChannel
                0,                                         // destStartSample
                arrayOfOnes,                               // source
                extraSamples,                              // numSamples
                float(ttlState));     // gain
        }
    }

    void DisplayBuffer::finalizeEventChannel(int nSamples)
    {

        if (displays.size() == 0)
            return;

        const int index = displayBufferIndices[numChannels];
        const int samplesLeft = getNumSamples() - index;
   
        int newIdx = 0;

        if (nSamples < samplesLeft)
        {
            newIdx = displayBufferIndices[numChannels] + nSamples;
        }
        else
        {
            newIdx = nSamples - samplesLeft;
        }
        
        displayBufferIndices.set(numChannels, newIdx);
    }

    void DisplayBuffer::addEvent(int eventTime, int eventChannel, int eventId, int numSourceSamples)
    {

        if (displays.size() == 0)
            return;

        const int index = (displayBufferIndices[numChannels] + eventTime) % getNumSamples();
        const int samplesLeft = getNumSamples() - index;
        const int nSamples = numSourceSamples - eventTime;

        if (eventId == 1)
        {
            ttlState |= (1LL << eventChannel);
        }
        else {
            ttlState &= ~(1LL << eventChannel);
        }

       // std::cout << "Display buffer received event on " << eventChannel << " at " << eventTime << " with " << numSourceSamples << std::endl;

        if (nSamples < samplesLeft)
        {
            copyFrom(numChannels,                     // destChannel
                index,                                // destStartSample
                arrayOfOnes,                          // source
                nSamples,                             // numSamples
                float(ttlState));                     // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            copyFrom(numChannels,                     // destChannel
                index,                                // destStartSample
                arrayOfOnes,                          // source
                samplesLeft,                          // numSamples
                float(ttlState));                     // gain

            copyFrom(numChannels,                     // destChannel
                0,                                    // destStartSample
                arrayOfOnes,                          // source
                extraSamples,                         // numSamples
                float(ttlState));                     // gain
        }
    }

    void DisplayBuffer::addData(AudioSampleBuffer& buffer, int chan, int nSamples)
    {
        if (displays.size() == 0)
            return;

        int previousIndex = displayBufferIndices[channelMap[chan]];
        int channelIndex = channelMap[chan];

        const int samplesLeft = getNumSamples() - displayBufferIndices[channelMap[chan]];

        int newIndex;
        
        if (nSamples < samplesLeft)
        {
            copyFrom(channelMap[chan],                      // destChannel
                displayBufferIndices[channelMap[chan]],  // destStartSample
                buffer,                    // source
                chan,                      // source channel
                0,                         // source start sample
                nSamples);                 // numSamples

            int lastIndex = displayBufferIndices[channelMap[chan]];

            newIndex = lastIndex + nSamples;
            
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

            newIndex = extraSamples;
        }

        ScopedLock displayLock(displayMutex);

        displayBufferIndices.set(channelMap[chan], newIndex);

    }

};
