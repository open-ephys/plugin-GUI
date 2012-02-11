/*
  ==============================================================================

    DataBuffer.cpp
    Created: 27 May 2011 2:13:42pm
    Author:  jsiegle

  ==============================================================================
*/

#include "DataBuffer.h"

DataBuffer::DataBuffer(int chans, int size)
	 : abstractFifo (size), buffer(chans, size), numChans(chans) {}


DataBuffer::~DataBuffer() {}

void DataBuffer::clear() {
	buffer.clear();
}

void DataBuffer::addToBuffer(float* data, int numItems) {
	// writes one sample for all channels
	int startIndex1, blockSize1, startIndex2, blockSize2;
	abstractFifo.prepareToWrite(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

	 for (int chan = 0; chan < numChans; chan++) {

		buffer.copyFrom(chan, // int destChannel
						startIndex1, // int destStartSample
						data + chan,  // const float* source
						1); // int num samples
	 }
	abstractFifo.finishedWrite(numItems);
}

int DataBuffer::getNumSamples() {
	return abstractFifo.getNumReady();
}


int DataBuffer::readAllFromBuffer (AudioSampleBuffer& data, int maxSize)
{
	// check to see if the maximum size is smaller than the total number of available ints
	int numItems = (maxSize < abstractFifo.getNumReady()) ? 
			maxSize : abstractFifo.getNumReady();
	
	int startIndex1, blockSize1, startIndex2, blockSize2;
	abstractFifo.prepareToRead(numItems, startIndex1, blockSize1, startIndex2, blockSize2);

	if (blockSize1 > 0) {
		for (int chan = 0; chan < data.getNumChannels(); chan++) {
			data.copyFrom(chan, // destChan
						   0,    // destStartSample
						   buffer, // source
						   chan,  // sourceChannel
						   startIndex1,     // sourceStartSample
						   blockSize1); // numSamples
		}
	}

		if (blockSize2 > 0) {

    for (int chan = 0; chan < data.getNumChannels(); chan++) {
			data.copyFrom(chan, // destChan
						   blockSize1,    // destStartSample
						   buffer, // source
						   chan,  // sourceChannel
						   startIndex2,     // sourceStartSample
						   blockSize2); // numSamples
		}
	}

	abstractFifo.finishedRead(numItems);

	return numItems;

}