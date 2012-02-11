/*
  ==============================================================================

    DataThread.cpp
    Created: 9 Jun 2011 1:31:49pm
    Author:  jsiegle

  ==============================================================================
*/

#include "DataThread.h"

DataThread::DataThread() : Thread ("Data Thread"), dataBuffer(0) {}

DataThread::~DataThread() {}

void DataThread::run() {

	while (! threadShouldExit())
	{
		const MessageManagerLock mml (Thread::getCurrentThread());
		if (! mml.lockWasGained())
			return;
		updateBuffer();
	}
}

DataBuffer* DataThread::getBufferAddress() {
	return dataBuffer;
}

