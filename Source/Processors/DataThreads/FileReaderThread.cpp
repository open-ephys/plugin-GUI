/*
  ==============================================================================

    FileReaderThread.cpp
    Created: 6 Sep 2011 11:05:57am
    Author:  jsiegle

  ==============================================================================
*/


#include "FileReaderThread.h"

FileReaderThread::FileReaderThread() : DataThread(),
			sampleRate(40000.0),
			numChannels(16),
			samplesPerBlock(1024)

{
	File file = File("./data_stream_16ch");
	input = file.createInputStream();

	dataBuffer = new DataBuffer(16, 4096);

	std::cout << "File Reader Thread initialized." << std::endl;

	startThread();
}

FileReaderThread::~FileReaderThread() {

	stopThread(500);

	std::cout << "File reader received disable signal." << std::endl;

	deleteAndZero(input);

	delete dataBuffer;
	dataBuffer = 0;
}


void FileReaderThread::updateBuffer()
{

	while (dataBuffer->getNumSamples() < 4096)
	{

    	for (int ch = 0; ch < numChannels; ch++) {
    		
    		if (input->isExhausted())
    			input->setPosition(0);   			
    	
    		thisSample[ch%numChannels] = float(input->readShort());

    	}

    	dataBuffer->addToBuffer(thisSample,1);

    }

}
