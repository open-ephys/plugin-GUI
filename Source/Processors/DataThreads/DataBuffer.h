/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __DATABUFFER_H_11C6C591__
#define __DATABUFFER_H_11C6C591__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"


/**
    Manages reading and writing data to a circular buffer.

    See @DataThread
*/
class PLUGIN_API DataBuffer
{
public:
    DataBuffer (int chans, int size);
    ~DataBuffer();

    /** Clears the buffer.*/
    void clear();

    /** Add an array of floats to the buffer.

        @param data The data.
        @param timestamps Array of timestamps. Same length as numItems.
        @param eventCodes Array of event codes. Same length as numItems.
        @param numItems Total number of samples per channel.
        @param chunkSize Number of consecutive samples per channel per chunk.
        1 by default. Typically 1 or numItems.

        @return The number of items actually written. May be less than numItems if
        the buffer doesn't have space.
    */
    int addToBuffer (float* data, int64* timestamps, uint64* eventCodes, int numItems, int chunkSize=1);

    /** Returns the number of samples currently available in the buffer.*/
    int getNumSamples() const;

    /** Copies as many samples as possible from the DataBuffer to an AudioSampleBuffer.*/
    int readAllFromBuffer (AudioSampleBuffer& data, uint64* ts, uint64* eventCodes, int maxSize, int dstStartChannel = 0, int numChannels = -1);

    /** Resizes the data buffer */
    void resize (int chans, int size);


private:
    AbstractFifo abstractFifo;
    AudioSampleBuffer buffer;

    HeapBlock<int64> timestampBuffer;
    HeapBlock<uint64> eventCodeBuffer;

	int64 lastTimestamp;

    int numChans;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBuffer);
};


#endif  // __DATABUFFER_H_11C6C591__
