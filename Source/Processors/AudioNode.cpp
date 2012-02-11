/*
  ==============================================================================

    AudioNode.cpp
    Created: 14 Jul 2011 6:04:39pm
    Author:  jsiegle

  ==============================================================================
*/


#include "AudioNode.h"

AudioNode::AudioNode()
	: GenericProcessor("Audio Node")
{

	// 64 inputs, 2 outputs (left and right channel)
	setPlayConfigDetails(64,2,44100.0,128);

	leftChan.add(0);
	rightChan.add(1);
}


AudioNode::~AudioNode() {

}

AudioProcessorEditor* AudioNode::createEditor()
{
	
	AudioEditor* editor = new AudioEditor(this);

	setEditor(editor);
	
	return editor; 

}


void AudioNode::setParameter (int parameterIndex, float newValue)
{
	// change left channel, right channel, or volume
	if (parameterIndex == 1) // volume level
		volume = newValue;

}


void AudioNode::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{

}

void AudioNode::releaseResources() 
{	

}


void AudioNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	buffer.clear(0,0,buffer.getNumSamples());
	buffer.clear(1,0,buffer.getNumSamples());

	for (int n = 0; n < leftChan.size(); n++) {
		buffer.addFrom(0,  // destination channel
					   0,  // destination start sample
					   buffer,      // source
					   leftChan[n]+2, // source channel
					   0,           // source start sample
					   buffer.getNumSamples(), //  number of samples
					   volume       // gain to apply
					   );
	}
	
	for (int n = 0; n < rightChan.size(); n++) {
		buffer.addFrom(1,  // destination channel
					   0,  // destination start sample
					   buffer,      // source
					   rightChan[n]+2, // source channel
					   0,           // source start sample
					   buffer.getNumSamples(), //  number of samples
					   volume       // gain to apply
					   );
	}
}
