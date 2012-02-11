/*
  ==============================================================================

    FileReaderThread.h
    Created: 6 Sep 2011 11:05:57am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __FILEREADERTHREAD_H_82594504__
#define __FILEREADERTHREAD_H_82594504__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThread.h"

class FileReaderThread : public DataThread

{
public:
	FileReaderThread();
	~FileReaderThread();
	
private:

	float sampleRate;
	int numChannels;
	int samplesPerBlock;

	FileInputStream* input;

	float thisSample[16];

	void updateBuffer();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileReaderThread);
};



#endif  // __FILEREADERTHREAD_H_82594504__
