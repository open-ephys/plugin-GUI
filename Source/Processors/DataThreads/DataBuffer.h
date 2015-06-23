/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

/**

	Manages reading and writing data to a circular buffer.

    See @DataThread

*/

class DataBuffer
{

public:
    DataBuffer(int chans, int size);
    ~DataBuffer();

    /** Clears the buffer.*/
    void clear();

    /** Add an array of floats to the buffer.*/
    void addToBuffer(float* data, int64* ts, uint64* eventCodes, int numItems);

    /** Returns the number of samples currently available in the buffer.*/
    int getNumSamples();

    /** Copies as many samples as possible from the DataBuffer to an AudioSampleBuffer.*/
    int readAllFromBuffer(AudioSampleBuffer& data, uint64* ts, uint64* eventCodes, int maxSize);

    /** Resizes the data buffer */
    void resize(int chans, int size);

private:
    AbstractFifo abstractFifo;
    AudioSampleBuffer buffer;

    HeapBlock<int64> timestampBuffer;
    HeapBlock<uint64> eventCodeBuffer;

    int numChans;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataBuffer);

};


#endif  // __DATABUFFER_H_11C6C591__
