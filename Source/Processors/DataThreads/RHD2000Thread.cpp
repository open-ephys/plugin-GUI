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
    evalBoard->open();

    string bitfilename;
    bitfilename = "rhd2000.bit"; 
    evalBoard->uploadFpgaBitfile(bitfilename);

     // Initialize board.
    evalBoard->initialize();
    evalBoard->setDataSource(0, Rhd2000EvalBoard::PortA1);

    numChannels = 32;

    // Select per-channel amplifier sampling rate.
    evalBoard->setSampleRate(Rhd2000EvalBoard::SampleRate20000Hz);

    // Now that we have set our sampling rate, we can set the MISO sampling delay
    // which is dependent on the sample rate.  We assume a 3-foot cable.
    evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortA, 3.0);

    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

}


RHD2000Thread::~RHD2000Thread() {
	
	std::cout << "RHD2000 interface destroyed." << std::endl;

	int ledArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

	deleteAndZero(evalBoard);

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
	return 28344.67;
}

float RHD2000Thread::getBitVolts()
{
	return 0.1907;
}

bool RHD2000Thread::foundInputSource()
{

	return true;

}

bool RHD2000Thread::startAcquisition()
{

	//memset(filter_states,0,256*sizeof(double)); 

   startThread();

  // isTransmitting = true;
  // accumulator = 0;

   return true;
}

bool RHD2000Thread::stopAcquisition()
{

//	isTransmitting = false;

	if (isThreadRunning()) {
        signalThreadShouldExit();
    }

    return true;
}

bool RHD2000Thread::updateBuffer()
{

	// data transfer and sorting code goes here

}