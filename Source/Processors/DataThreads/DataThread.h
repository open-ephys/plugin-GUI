/*
  ==============================================================================

    DataThread.h
    Created: 9 Jun 2011 1:31:49pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __DATATHREAD_H_C454F4DB__
#define __DATATHREAD_H_C454F4DB__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "DataBuffer.h"

class SourceNode;

class DataThread : public Thread
{

public:

	DataThread(SourceNode* sn);
	~DataThread();

	void run();

	DataBuffer* getBufferAddress();

	virtual bool updateBuffer() = 0;

	DataBuffer* dataBuffer;

	virtual bool foundInputSource() = 0;
	virtual bool startAcquisition() = 0;
	virtual bool stopAcquisition() = 0;
	virtual int getNumChannels() = 0;
	virtual float getSampleRate() = 0;

	SourceNode* sn;

};


#endif  // __DATATHREAD_H_C454F4DB__
