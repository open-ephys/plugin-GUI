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


#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "okFrontPanelDLL.h"
#include "DataThread.h"

/**

  --UNDER CONSTRUCTION--

  Communicates with the custom acquisition board via an Opal Kelly FPGA.

  @see DataThread, SourceNode

*/

class SourceNode;

class FPGAThread : public DataThread

{
public:
	FPGAThread(SourceNode* sn);
	~FPGAThread();

	bool foundInputSource() {return true;}
	bool startAcquisition();
	bool stopAcquisition();
	int getNumChannels() {return 32;}
	float getSampleRate() {return 25000.0;}
	
private:

	okCFrontPanel* dev;
	char bitfile[128];
	char dll_date[32], dll_time[32];
	UINT32 i;

	int m_u32SegmentSize;
	
	unsigned char pBuffer[50000];  // request a 1MB block of data

	bool isRunning;

	float thisSample[32];

	int numchannels;
	int Ndatabytes;

	bool updateBuffer();
	bool initializeFPGA(okCFrontPanel*, char*);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FPGAThread);
};



#endif  // __FPGATHREAD_H_FBB22A45__
