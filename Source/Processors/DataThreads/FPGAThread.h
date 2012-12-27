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

#ifndef __FPGATHREAD_H_FBB22A45__
#define __FPGATHREAD_H_FBB22A45__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"

#include <stdio.h>
#include <string.h>

#include "okFrontPanelDLL.h"
#include "DataThread.h"

class SourceNode;

/**
 
  Communicates with the Open Ephys acquisition board via an Opal Kelly FPGA.

  @see DataThread, SourceNode

*/

class FPGAThread : public DataThread

{
public:
	FPGAThread(SourceNode* sn);
	~FPGAThread();

	bool foundInputSource();
	int getNumChannels();
	float getSampleRate();
	float getBitVolts();
    
    int getNumEventChannels();

    void setOutputHigh();
    void setOutputLow();

private:

	okCFrontPanel* dev;
	char bitfile[128];
	char dll_date[32], dll_time[32];
	bool isTransmitting;
	bool deviceFound;

	bool initializeFPGA(bool);
	bool closeFPGA();

	bool startAcquisition();
	bool stopAcquisition();

    int alignBuffer(int nBytes);
    
    void checkTTLState();

	unsigned char pBuffer[500000];  // size of the data requested in each buffer
    int bytesToRead;
    unsigned char overflowBuffer[20000];
    
    int overflowSize;
    
    int ttl_out;
    
    int ttlState;
    
    int ttlOutputVal;
    int accumulator;

    bool bufferWasAligned;

	float thisSample[256];

	int numchannels;
	int Ndatabytes;

	bool updateBuffer();
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FPGAThread);
};



#endif  // __FPGATHREAD_H_FBB22A45__
