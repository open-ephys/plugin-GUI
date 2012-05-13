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

FileReaderThread::FileReaderThread(SourceNode* sn) : DataThread(sn)

{
	//File file = File("./data_stream_16ch");
	//input = file.createInputStream();
    bufferSize = 1600;

    input = fopen("./data_stream_16ch_2", "r");

    fseek(input, 0, SEEK_END);
    lengthOfInputFile = ftell(input);
    rewind(input);

	dataBuffer = new DataBuffer(16, bufferSize*3);

   

	std::cout << "File Reader Thread initialized." << std::endl;

}

FileReaderThread::~FileReaderThread() {

	deleteAndZero(input);

}

bool FileReaderThread::foundInputSource()
{
    return true;
}

int FileReaderThread::getNumChannels()
{
    return 16;
}

float FileReaderThread::getSampleRate()
{
    return 40000.0f;
}

float FileReaderThread::getBitVolts()
{
    return 0.0305f;
}

bool FileReaderThread::startAcquisition()
{
	startThread();

    return true;

}

bool FileReaderThread::stopAcquisition()
{
    std::cout << "File reader received disable signal." << std::endl;
	if (isThreadRunning()) {
        signalThreadShouldExit();
    }
	

    return true;

}

bool FileReaderThread::updateBuffer()
{

	if (dataBuffer->getNumSamples() < bufferSize)
	 {
 //       // std::cout << dataBuffer->getNumSamples() << std::endl;

       if (ftell(input) >= lengthOfInputFile - bufferSize)
       {
           rewind(input);
       }

         fread(readBuffer, 2, bufferSize, input);

        int chan = 0;

        for (int n = 0; n < bufferSize; n++)
        {
            thisSample[chan] = float(-readBuffer[n])*0.0305f;

            if (chan == 15)
            {
                dataBuffer->addToBuffer(thisSample,1);
                chan = 0;
            } else {
                chan++;
            }


         }
    	

     } else {
        wait(5); // pause for 5 ms
     }

    return true;
}
