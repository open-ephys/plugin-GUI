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
    sampleNumberBuffer.malloc (size);
    timestampBuffer.malloc (size);
    eventCodeBuffer.malloc (size);

	lastSampleNumber = 0;
    lastTimestamp = -1.0;
}


void DataBuffer::clear()
{
    buffer.clear();
    abstractFifo.reset();
    
    lastSampleNumber = 0;
    lastTimestamp = -1.0;
}


void DataBuffer::resize (int chans, int size)
{
    buffer.setSize (chans, size);

    sampleNumberBuffer.malloc (size);
    timestampBuffer.malloc (size);
    eventCodeBuffer.malloc (size);

    lastSampleNumber = 0;
    lastTimestamp = -1.0;

    numChans = chans;
}

int DataBuffer::addToBuffer (float* data,
                             int64* sampleNumbers,
                             double* timestamps,
                             uint64* eventCodes,
                             int numItems,
                             int chunkSize)
{
    int bs[] = { -1, -1, 0 };    // zero-termintated as check in outer loop
    int si[] = { -1, -1};
    int cSize = chunkSize;
    int offset = 0;
    
    abstractFifo.prepareToWrite (numItems, si[0], bs[0], si[1], bs[1]);

    // The contiguous case is separated from the interleaved version to reduce the
    // overhead of the for loops and small copies.
    if (numItems == chunkSize)
    {
        for (int chan = 0; chan < numChans; ++chan)
        {
            // copy first half
            buffer.copyFrom (chan,                       // destination channel
                             si[0],                // offset into buffer
                             data + chunkSize * chan,    // source data location
                             bs[0]);                // number of samples

            // copy remaining (if any)
            if (bs[1] != 0)
            {
                buffer.copyFrom (chan,
                                 si[1],
                                 data + chunkSize * chan + bs[0],
                                 bs[1]);
            }
        }
    }
    else
    {
      for (int i = 0; bs[i] != 0; ++i)
      {                                // for each of the dest blocks we can write to...
          for (int j = 0; j < bs[i]; )
          {                     // for each chunk...
              // If there wasn't enough room at the end of the circular buffer
              // to copy the full chunk, the rest of that chunk needs to be
              // copied to the start of the circular buffer.
              // The bounds check is not necessary since the data is being
              // copied to the beginning of the circular buffer
              // (si[1] should always be 0).
              if (cSize != chunkSize)
              {
                  offset = cSize;              // previous amount copied
                  cSize = chunkSize - cSize;   // remaining to copy
              }
              else
              {
                  offset = 0;
                  cSize = chunkSize <= bs[i] - j ? chunkSize : bs[i] - j;     // prevent out-of-bounds on buffer
              }

              for (int chan = 0; chan < numChans; ++chan)
              {
                  buffer.copyFrom (chan,
                                   si[i] + j,
                                   data + chunkSize * chan + offset,
                                   cSize);
              }

              j += cSize;   // advance the loop counter based on copied amount

              //  advance source data pointer, continue with full sized chunks.
              if ((cSize == chunkSize) || (i != 0))
              {
                data += chunkSize * numChans;
                cSize = chunkSize;
              }
              // else keep data and cSize to get 'remainder' of current group to
              // paste into the start of the circular buffer on next iteration.
              // At this point, internal loop should complete and the external
              // loop should go to next start/size set.
          }
      }
    }

    // Copy the other items; all of this information is contiguous,
    // so up to two copies are needed for wrap-around on the circular buffers.

    memcpy(sampleNumberBuffer + si[0], sampleNumbers, bs[0] * sizeof(sampleNumbers[0]));
    if (bs[1] != 0) {
      memcpy(sampleNumberBuffer, sampleNumbers + bs[0], bs[1] * sizeof(sampleNumbers[0]));
    }

    memcpy(timestampBuffer + si[0], timestamps, bs[0] * sizeof(timestamps[0]));
    if (bs[1] != 0) {
      memcpy(timestampBuffer, timestamps + bs[0], bs[1] * sizeof(timestamps[0]));
    }

    memcpy(eventCodeBuffer + si[0], eventCodes, bs[0] * sizeof(eventCodes[0]));
    if (bs[1] != 0) {
      memcpy(eventCodeBuffer, eventCodes + bs[0], bs[1] * sizeof(eventCodes[0]));
    }

    // maintain state of the circular buffer by updating the last written position.
    abstractFifo.finishedWrite (bs[0] + bs[1]);

    return bs[0] + bs[1];
}


int DataBuffer::getNumSamples() const { return abstractFifo.getNumReady(); }


int DataBuffer::readAllFromBuffer (AudioBuffer<float>& data,
                                   int64* blockSampleNumber,
                                   double* blockTimestamp,
                                   uint64* eventCodes,
                                   int maxSize,
                                   int dstStartChannel,
                                   int numChannels)
{
    // check to see if the maximum size is smaller than the total number of available ints
    int numReady = abstractFifo.getNumReady();
    int numItems = (maxSize < numReady) ? maxSize : numReady;

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

        memcpy (blockSampleNumber, sampleNumberBuffer + startIndex1, 8);
        memcpy (blockTimestamp, timestampBuffer + startIndex1, 8);
        memcpy (eventCodes, eventCodeBuffer + startIndex1, blockSize1 * 8);
    }
    else
    {
       // std::cout << "NO SAMPLES" << std::endl;
		memcpy(blockSampleNumber, &lastSampleNumber, 8);
        memcpy(blockTimestamp, &lastTimestamp, 8);
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

   // std::cout << "START SAMPLE FOR READ: " << *blockSampleNumber << std::endl;
    
    if (numItems > 0)
    {

        lastSampleNumber = *blockSampleNumber;
        lastTimestamp = *blockTimestamp;

       // std::cout << "Updating last sample number: " << lastSampleNumber << std::endl;
    }

    abstractFifo.finishedRead (numItems);

    return numItems;
}
