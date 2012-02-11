/*
  ==============================================================================

    AudioNode.h
    Created: 14 Jul 2011 6:04:39pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __AUDIONODE_H_AF61F3C5__
#define __AUDIONODE_H_AF61F3C5__


#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "GenericProcessor.h"
#include "Editors/AudioEditor.h"

class AudioNode : public GenericProcessor
{
public:
	
	// real member functions:
	AudioNode();
	~AudioNode();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	AudioProcessorEditor* createEditor();
	
private:

	Array<int> leftChan;
	Array<int> rightChan;
	float volume;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioNode);

};





#endif  // __AUDIONODE_H_AF61F3C5__
