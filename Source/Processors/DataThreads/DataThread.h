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

class DataThread : public Thread
{

public:

	DataThread();
	~DataThread();

	void run();

	DataBuffer* getBufferAddress();

	virtual void updateBuffer() = 0;

	DataBuffer* dataBuffer;

	virtual bool threadStarted() {return true;}

};


#endif  // __DATATHREAD_H_C454F4DB__
