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

#include "RHD2000Thread.h"
#include "../SourceNode.h"

RHD2000Thread::RHD2000Thread(SourceNode* sn) : DataThread(sn)
{
	evalBoard = new Rhd2000EvalBoard;

	 // Open Opal Kelly XEM6010 board.
    int return_code = evalBoard->open();

    if (return_code == 1)
    {
    	deviceFound = true;
    } else {
    	deviceFound = false;
    }

    if (deviceFound)
    {
	    string bitfilename;
	    bitfilename = "rhd2000.bit"; 
	    evalBoard->uploadFpgaBitfile(bitfilename);

	     // Initialize board.
	    evalBoard->initialize();
	    evalBoard->setDataSource(0, Rhd2000EvalBoard::PortA1);
	    evalBoard->setContinuousRunMode(false);

	    numChannels = 32;

	    // Select per-channel amplifier sampling rate.
	    evalBoard->setSampleRate(Rhd2000EvalBoard::SampleRate10000Hz);

	    // Now that we have set our sampling rate, we can set the MISO sampling delay
	    // which is dependent on the sample rate.  We assume a 3-foot cable.
	    evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortA, 3.0);

	    // Let's turn one LED on to indicate that the program is running.
	    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
	    evalBoard->setLedDisplay(ledArray);

	    // Set up an RHD2000 register object using this sample rate to optimize MUX-related
    	// register settings.
    	chipRegisters = new Rhd2000Registers(evalBoard->getSampleRate());

    	// Before generating register configuration command sequences, set amplifier
    	// bandwidth paramters.
    	double dspCutoffFreq;
    	dspCutoffFreq = chipRegisters->setDspCutoffFreq(10.0);
    	cout << "Actual DSP cutoff frequency: " << dspCutoffFreq << " Hz" << endl;

    	chipRegisters->setLowerBandwidth(1.0);
    	chipRegisters->setUpperBandwidth(7500.0);

  		dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());

  		dataBuffer = new DataBuffer(numChannels, 10000);


	}

}

RHD2000Thread::~RHD2000Thread() {
	
	std::cout << "RHD2000 interface destroyed." << std::endl;

	if (deviceFound)
	{
		int ledArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    	evalBoard->setLedDisplay(ledArray);
	}

}


int RHD2000Thread::getNumChannels()
{
	return 32;
}

int RHD2000Thread::getNumEventChannels()
{
    return 16; // 8 inputs, 8 outputs
}

float RHD2000Thread::getSampleRate()
{
	return 10000.00;
}

float RHD2000Thread::getBitVolts()
{
	return 0.1907;
}

bool RHD2000Thread::foundInputSource()
{

	return deviceFound;

}

bool RHD2000Thread::startAcquisition()
{

	//memset(filter_states,0,256*sizeof(double)); 

	 int ledArray[8] = {1, 1, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

    cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
    cout << "Is eval board running: " << evalBoard->isRunning() << endl;


    // If this happens too soon after acquisition is stopped, problems ensue
    std::cout << "Flushing FIFO." << std::endl;
    evalBoard->flush();


    std::cout << "Setting max timestep." << std::endl;
    evalBoard->setMaxTimeStep(100);
	evalBoard->setContinuousRunMode(true);

	std::cout << "Starting acquisition." << std::endl;
	evalBoard->run();

	   blockSize = dataBlock->calculateDataBlockSizeInWords(evalBoard->getNumEnabledDataStreams());


   startThread();


  // isTransmitting = true;
  // accumulator = 0;

   return true;
}

bool RHD2000Thread::stopAcquisition()
{

//	isTransmitting = false;
	std::cout << "RHD2000 data thread stopping acquisition." << std::endl;

	if (isThreadRunning()) {
        signalThreadShouldExit();
    }

	evalBoard->setContinuousRunMode(false);
	evalBoard->setMaxTimeStep(0);

	cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;

   	std::cout << "Stopped eval board." << std::endl;


    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

    return true;
}

bool RHD2000Thread::updateBuffer()
{

	//cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
	//cout << "Block size: " << blockSize << endl;

	bool return_code;
	int lastBlock;

	for (int n = 0; n < 1; n++)
	{
		if (evalBoard->numWordsInFifo() >= blockSize)
		{

			return_code = evalBoard->readDataBlock(dataBlock);

			for (int samp = 0; samp < dataBlock->getSamplesPerDataBlock(); samp++)
			{

				for (int chan = 0; chan < numChannels; chan++)
				{

					int value = dataBlock->amplifierData[0][chan][samp];

					thisSample[chan] = double(value)*0.01;
				}

				timestamp = dataBlock->timeStamp[samp];
				eventCode = dataBlock->ttlIn[samp];

				dataBuffer->addToBuffer(thisSample, &timestamp, &eventCode, 1);
			}

		} 
	}
	

	return true;

}