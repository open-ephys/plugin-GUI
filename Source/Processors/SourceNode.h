/*
  ==============================================================================

    SourceNode.h
    Created: 7 May 2011 5:07:14pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SOURCENODE_H_DCE798F1__
#define __SOURCENODE_H_DCE798F1__

#include "../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThreads/DataBuffer.h"
#include "DataThreads/IntanThread.h"
#include "DataThreads/FPGAThread.h"
#include "DataThreads/FileReaderThread.h"
#include "GenericProcessor.h"

class SourceNode : public GenericProcessor

{
public:
	
	// real member functions:
	SourceNode(const String& name);
	~SourceNode();

	//void setName(const String name_);
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	void setConfiguration(Configuration* cf);

	float getSampleRate();

	// void setSourceNode(GenericProcessor* sn);
	// void setDestNode(GenericProcessor* dn);

	AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}

	bool enable();
	bool disable();

	bool isSource() {return true;}
	
private:

	//const String name;

	DataThread* dataThread;
	DataBuffer* inputBuffer;

	int* numSamplesInThisBuffer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);

};


#endif  // __SOURCENODE_H_DCE798F1__

