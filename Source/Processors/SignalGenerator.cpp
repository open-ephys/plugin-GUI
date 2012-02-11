/*
  ==============================================================================

    SignalGenerator.cpp
    Created: 9 May 2011 8:11:41pm
    Author:  jsiegle

  ==============================================================================
*/


#include "SignalGenerator.h"
//#include "SourceNodeEditor.h"
#include <stdio.h>

SignalGenerator::SignalGenerator()
	: GenericProcessor("Signal Generator"),

	  frequency(10.0),
	  sampleRate (44100.0),
	  currentPhase (0.0),
	  phasePerSample (0.0),
	  amplitude (0.02f)
	
{

	setNumOutputs(16);
	setNumInputs(0);


	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);


}

SignalGenerator::~SignalGenerator()
{
	config->removeDataSource(this);	
}


void SignalGenerator::setConfiguration(Configuration* cf)
{
	config = cf;

     DataSource* d = new DataSource(this, config);

	 // add a new data source to this configuration
     config->addDataSource(d);

}

AudioProcessorEditor* SignalGenerator::createEditor( )
{
	SignalGeneratorEditor* ed = new SignalGeneratorEditor(this, viewport);
	setEditor(ed);
	
	std::cout << "Creating editor." << std::endl;
	//filterEditor = new FilterEditor(this);
	return ed;
}


void SignalGenerator::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;

	if (parameterIndex == 0)
		amplitude = newValue;
	else
		frequency = newValue;

	phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);

}

void SignalGenerator::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	sampleRate = sampleRate_;
	phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);
	//std::cout << "Prepare to play: " << std::endl;
}

bool SignalGenerator::enable () {
	
	std::cout << "Signal generator received enable signal." << std::endl;
	return true;
}

bool SignalGenerator::disable() {
	
	std::cout << "Signal generator received disable signal." << std::endl;
	return true;
}

void SignalGenerator::releaseResources() 
{	
}

void SignalGenerator::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamps)
{

	//std::cout << buffer.getNumChannels() << std::endl;
	nSamps = buffer.getNumSamples();
	
    for (int i = 0; i < nSamps; ++i)
    {
        const float sample = amplitude * (float) std::sin (currentPhase);
        currentPhase += phasePerSample;

        for (int j = buffer.getNumChannels(); --j >= 0;)
        	// dereference pointer to one of the buffer's samples
            *buffer.getSampleData (j, i) = sample;
    }
}
