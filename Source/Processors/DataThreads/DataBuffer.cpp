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

#include "DataBuffer.h"

DataBuffer::DataBuffer(int chans, int size)
    : abstractFifo(size), buffer(chans, size), numChans(chans)
{
    timestampBuffer.malloc(size);
    eventCodeBuffer.malloc(size);

}


DataBuffer::~DataBuffer() {}

void DataBuffer::clear()
{
    buffer.clear();
    abstractFifo.reset();
}

void DataBuffer::resize(int chans, int size)
{
    buffer.setSize(chans, size);
    timestampBuffer.malloc(size);
    eventCodeBuffer.malloc(size);

    numChans = chans;
}

void DataBuffer::addToBuffer(float* data, int64* timestamps, uint64* eventCodes, int numItems)
{
    // writes one sample for all channels
    int startIndex1, blockSize1, startIndex2, blockSize2;
    abstractFifo.prepareToWrite(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

    for (int chan = 0; chan < numChans; chan++)
    {

        buffer.copyFrom(chan, // int destChannel
                        startIndex1, // int destStartSample
                        data + chan,  // const float* source
                        1); // int num samples
    }

    *(timestampBuffer + startIndex1) = *timestamps;
    *(eventCodeBuffer + startIndex1) = *eventCodes;

    abstractFifo.finishedWrite(numItems);
}

int DataBuffer::getNumSamples()
{
    return abstractFifo.getNumReady();
}


int DataBuffer::readAllFromBuffer(AudioSampleBuffer& data, uint64* timestamp, uint64* eventCodes, int maxSize)
{
    // check to see if the maximum size is smaller than the total number of available ints

    // Better version (1/27/14)?
    int numReady = abstractFifo.getNumReady();
    int numItems = (maxSize < numReady) ? maxSize : numReady;

    // Original version:
    //int numItems = (maxSize < abstractFifo.getNumReady()) ?
    //               maxSize : abstractFifo.getNumReady();

    int startIndex1, blockSize1, startIndex2, blockSize2;
    abstractFifo.prepareToRead(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

    if (blockSize1 > 0)
    {

        for (int chan = 0; chan < data.getNumChannels(); chan++)
        {
            data.copyFrom(chan, // destChan
                          0,    // destStartSample
                          buffer, // source
                          chan,  // sourceChannel
                          startIndex1,     // sourceStartSample
                          blockSize1); // numSamples
        }

        memcpy(timestamp, timestampBuffer+startIndex1, 8);
        memcpy(eventCodes, eventCodeBuffer+startIndex1, blockSize1*8);
    }
    else
    {
        memcpy(timestamp, timestampBuffer+startIndex2, 8);
    }

    if (blockSize2 > 0)
    {

        for (int chan = 0; chan < data.getNumChannels(); chan++)
        {
            data.copyFrom(chan, // destChan
                          blockSize1,    // destStartSample
                          buffer, // source
                          chan,  // sourceChannel
                          startIndex2,     // sourceStartSample
                          blockSize2); // numSamples
        }
        memcpy(eventCodes + blockSize1, eventCodeBuffer+startIndex2, blockSize2*8);
    }

    abstractFifo.finishedRead(numItems);

    return numItems;

}