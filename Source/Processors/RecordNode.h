/*
  ==============================================================================

    RecordNode.h
    Created: 10 May 2011 7:17:09pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __RECORDNODE_H_FB9B1CA7__
#define __RECORDNODE_H_FB9B1CA7__


#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "GenericProcessor.h"

class RecordNode : public GenericProcessor
{
public:
	
	// real member functions:
	RecordNode();
	~RecordNode();
	
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	float getFreeSpace();
	
private:

	File outputFile;
	FileOutputStream* outputStream;

	bool isRecording;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNode);

};



#endif  // __RECORDNODE_H_FB9B1CA7__
