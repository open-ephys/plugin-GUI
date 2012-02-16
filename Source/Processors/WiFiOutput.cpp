/*
  ==============================================================================

    WiFiOutput.cpp
    Created: 15 Feb 2012 9:09:19pm
    Author:  jsiegle

  ==============================================================================
*/

#include <stdio.h>
#include "WiFiOutput.h"

WiFiOutput::WiFiOutput()
	: GenericProcessor("WiFi Output")
{
	setNumOutputs(0);
	setNumInputs(0);

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

}

WiFiOutput::~WiFiOutput()
{

}

// AudioProcessorEditor* EventNode::createEditor()
// {
// 	FilterEditor* filterEditor = new FilterEditor(this, viewport);
	
// 	std::cout << "Creating editor." << std::endl;
// 	//filterEditor = new FilterEditor(this);
// 	return filterEditor;

// 	//return 0;
// }


void WiFiOutput::setParameter (int parameterIndex, float newValue)
{

}


void WiFiOutput::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	//std::cout << "Filter node preparing." << std::endl;
}



void WiFiOutput::releaseResources() 
{	
}

void WiFiOutput::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{
	

	int sampleNum = checkForMidiEvents(midiMessages);

	if (sampleNum >= 0)
	{
		startTimer((int) float(sampleNum)/getSampleRate()*1000.0);
	}
	

}

void WiFiOutput::timerCallback()
{
	//std::cout << "FIRE!" << std::endl;
	socket.sendTo("hi",2,"169.254.187.27",6000);
	stopTimer();
}
