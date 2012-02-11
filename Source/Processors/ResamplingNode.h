/*
  ==============================================================================

    ResamplingNode.h
    Created: 7 May 2011 5:07:58pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __RESAMPLINGNODE_H_79663B0__
#define __RESAMPLINGNODE_H_79663B0__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Dsp/Dsp.h"
#include "GenericProcessor.h"

//class ResamplingNode;

class ResamplingNode : public GenericProcessor

{
public:
	
	// real member functions:
	ResamplingNode(bool destBufferIsTempBuffer);
	~ResamplingNode();
	
	AudioSampleBuffer* getBufferAddress() { return destBuffer; }
	void updateFilter();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	AudioSampleBuffer* getContinuousBuffer() {return destBuffer;}


private:

	// sample rate, timebase, and ratio info:
	double sourceBufferSampleRate, destBufferSampleRate;
	double ratio, lastRatio;
	double destBufferTimebaseSecs;
	int destBufferWidth;
	
	// major objects:
	Dsp::Filter* filter;
	AudioSampleBuffer* destBuffer;
	AudioSampleBuffer* tempBuffer;

	// is the destBuffer a temp buffer or not?
	bool destBufferIsTempBuffer;
	bool isTransmitting;

	// indexing objects that persist between rounds:
	int destBufferPos;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResamplingNode);

};




#endif  // __RESAMPLINGNODE_H_79663B0__
