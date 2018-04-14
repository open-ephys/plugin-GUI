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

#include "DataBuffer.h"


DataBuffer::DataBuffer (int chans, int size)
    : abstractFifo  (size)
    , buffer        (chans, size)
    , numChans      (chans)
{
    timestampBuffer.malloc (size);
    eventCodeBuffer.malloc (size);

	lastTimestamp = 0;
}


DataBuffer::~DataBuffer() {}


void DataBuffer::clear()
{
    buffer.clear();
    abstractFifo.reset();
	lastTimestamp = 0;
}


void DataBuffer::resize (int chans, int size)
{
    buffer.setSize (chans, size);

    timestampBuffer.malloc (size);
    eventCodeBuffer.malloc (size);

	lastTimestamp = 0;

    numChans = chans;
}

int DataBuffer::addToBuffer (float* data, int64* timestamps, uint64* eventCodes, int numItems, int chunkSize)
{
    int startIndex1, blockSize1, startIndex2, blockSize2;

    abstractFifo.prepareToWrite (numItems, startIndex1, blockSize1, startIndex2, blockSize2);

    int bs[3] = { blockSize1, blockSize2, 0 };
    int si[2] = { startIndex1, startIndex2 };
    int cSize = 0;
    int idx = 0;
    int blkIdx;

	if (numItems > 0)
		lastTimestamp = timestamps[numItems-1]; 

    for (int i = 0; bs[i] != 0; ++i)
    {                                // for each of the dest blocks we can write to...
        blkIdx = 0;
        for (int j = 0; j < bs[i]; j+= chunkSize) 
        {                     // for each chunk...
            cSize = chunkSize <= bs[i] - j ? chunkSize : bs[i] - j;     // figure our how much you can write
            for (int chan = 0; chan < numChans; ++chan)         // write that much, per channel
            {
                buffer.copyFrom (chan,                           // (int destChannel)
                                 si[i] + j,                      // (int destStartSample)
                                 data + (idx * numChans) + chan, // (const float* source)
                                 cSize);                         // (int num samples)
            }

            for (int k = 0; k < cSize; ++k)
            {
                timestampBuffer[si[i] + blkIdx + k] = timestamps[idx + k];
                eventCodeBuffer[si[i] + blkIdx + k] = eventCodes[idx + k];
            }
            idx     += cSize;
            blkIdx  += cSize;
        }
    }

	

    // finish write
    abstractFifo.finishedWrite (idx);

    return idx;
}


int DataBuffer::getNumSamples() const { return abstractFifo.getNumReady(); }


int DataBuffer::readAllFromBuffer (AudioSampleBuffer& data, uint64* timestamp, uint64* eventCodes, int maxSize, int dstStartChannel, int numChannels)
{
    // check to see if the maximum size is smaller than the total number of available ints

    // Better version (1/27/14)?
    int numReady = abstractFifo.getNumReady();
    int numItems = (maxSize < numReady) ? maxSize : numReady;

    // Original version:
    //int numItems = (maxSize < abstractFifo.getNumReady()) ?
    //               maxSize : abstractFifo.getNumReady();

    int startIndex1, blockSize1, startIndex2, blockSize2;
    abstractFifo.prepareToRead (numItems, startIndex1, blockSize1, startIndex2, blockSize2);

	int channelsToCopy = numChannels < 0 ? data.getNumChannels() : numChannels;

    if (blockSize1 > 0)
    {
        for (int chan = 0; chan < channelsToCopy; ++chan)
        {
            data.copyFrom (dstStartChannel+chan,            // destChan
                           0,               // destStartSample
                           buffer,          // source
                           chan,            // sourceChannel
                           startIndex1,     // sourceStartSample
                           blockSize1);     // numSamples
        }

        memcpy (timestamp, timestampBuffer + startIndex1, 8);
        memcpy (eventCodes, eventCodeBuffer + startIndex1, blockSize1 * 8);
    }
    else
    {
		memcpy(timestamp, &lastTimestamp, 8);
    }

    if (blockSize2 > 0)
    {
        for (int chan = 0; chan < channelsToCopy; ++chan)
        {
            data.copyFrom (dstStartChannel+chan,            // destChan
                           blockSize1,      // destStartSample
                           buffer,          // source
                           chan,            // sourceChannel
                           startIndex2,     // sourceStartSample
                           blockSize2);     // numSamples
        }
        memcpy (eventCodes + blockSize1, eventCodeBuffer + startIndex2, blockSize2 * 8);
    }

    abstractFifo.finishedRead (numItems);

    return numItems;
}
