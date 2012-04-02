/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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


#include "FileReaderThread.h"

FileReaderThread::FileReaderThread(SourceNode* sn) : DataThread(sn),
			sampleRate(40000.0),
			numChannels(16),
			samplesPerBlock(1024)

{
	File file = File("./data_stream_16ch");
	input = file.createInputStream();

   // lengthOfInputFile = file

	dataBuffer = new DataBuffer(16, 4096);

	std::cout << "File Reader Thread initialized." << std::endl;

}

FileReaderThread::~FileReaderThread() {

	deleteAndZero(input);

	deleteAndZero(dataBuffer);

}

bool FileReaderThread::startAcquisition()
{
	startThread();

}

bool FileReaderThread::stopAcquisition()
{
	stopThread(500);
	std::cout << "File reader received disable signal." << std::endl;
}

bool FileReaderThread::updateBuffer()
{

	while (dataBuffer->getNumSamples() < 4096)
	{

       // if (input->getTotalLength())

        //input->read(thisSample, 4*)

    	for (int ch = 0; ch < numChannels; ch++) {
    		
    		if (input->isExhausted())
    			input->setPosition(0);   			
    	
    		thisSample[ch%numChannels] = float(input->readShort())/80000.0f;

    	}

    	dataBuffer->addToBuffer(thisSample,1);

    }

    return true;
}
