/*
  ==============================================================================

    DataBuffer.h
    Created: 27 May 2011 2:13:42pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __DATABUFFER_H_11C6C591__
#define __DATABUFFER_H_11C6C591__

#include "../../../JuceLibraryCode/JuceHeader.h"

class DataBuffer
{
	
public:
	DataBuffer(int chans, int size);
	~DataBuffer();
	void clear();
	void addToBuffer(float* data, int numItems);
	int getNumSamples();
	int readAllFromBuffer(AudioSampleBuffer& data, int maxSize);

private:
	AbstractFifo abstractFifo;
	AudioSampleBuffer buffer;
	int numChans;
};


#endif  // __DATABUFFER_H_11C6C591__
