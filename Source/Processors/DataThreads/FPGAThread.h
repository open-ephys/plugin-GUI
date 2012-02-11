/*
  ==============================================================================

    FPGAThread.h
    Created: 9 Jun 2011 2:08:11pm
    Author:  jsiegle

  ==============================================================================
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

class FPGAThread : public DataThread

{
public:
	FPGAThread();
	~FPGAThread();
	
private:

	okCFrontPanel* dev;
	char bitfile[128];
	char dll_date[32], dll_time[32];
	UINT32 i;

	int m_u32SegmentSize;
	
	unsigned char pBuffer[500000];  // request a 1MB block of data

	bool isRunning;

	DataBuffer* dataBuffer;

	float thisSample[32];

	int numchannels;
	int Ndatabytes;

	void updateBuffer();
	bool initializeFPGA(okCFrontPanel*, char*);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FPGAThread);
};



#endif  // __FPGATHREAD_H_FBB22A45__
