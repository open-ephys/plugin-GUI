/*
  ==============================================================================

    DataThread.cpp
    Created: 9 Jun 2011 1:31:49pm
    Author:  jsiegle

  ==============================================================================
*/

#include "DataThread.h"
#include "../SourceNode.h"


DataThread::DataThread(SourceNode* s) : Thread ("Data Thread"), dataBuffer(0) 
{
	sn = s;
	setPriority(10);
}

DataThread::~DataThread() {}

void DataThread::run() {

	while (! threadShouldExit())
	{
		const MessageManagerLock mml (Thread::getCurrentThread());
		if (! mml.lockWasGained())
			return;
		if (!updateBuffer()) {
			std::cout << "Aquisition error...stopping thread." << std::endl;
			signalThreadShouldExit();
			//stopAcquisition();
			std::cout << "Notifying source node to stop acqusition." << std::endl;
			sn->acquisitionStopped();
		}
			//

	}
}

DataBuffer* DataThread::getBufferAddress() {
	return dataBuffer;
}


