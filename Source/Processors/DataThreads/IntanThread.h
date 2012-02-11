/*
  ==============================================================================

    IntanThread.h
    Created: 9 Jun 2011 1:34:16pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __INTANTHREAD_H_D9135C03__
#define __INTANTHREAD_H_D9135C03__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThread.h"

class IntanThread : public DataThread

{
public:
	IntanThread();
	~IntanThread();

	bool threadStarted() {return isTransmitting;}
	
private:

	struct ftdi_context ftdic;
	int vendorID, productID;
	int baudrate;
	bool isTransmitting;

	bool initializeUSB();
	
	unsigned char startCode, stopCode;
	unsigned char buffer[240]; // should be 5 samples per channel

	float thisSample[16];
	//float thisSample[64];

	int ch;

	void updateBuffer();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntanThread);
};


#endif  // __INTANTHREAD_H_D9135C03__
