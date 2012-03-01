/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
	//config->removeDataSource(this);	
}


// void SignalGenerator::setConfiguration(Configuration* cf)
// {
// 	config = cf;

//      DataSource* d = new DataSource(this, config);

// 	 // add a new data source to this configuration
//      config->addDataSource(d);

// }

AudioProcessorEditor* SignalGenerator::createEditor( )
{
	SignalGeneratorEditor* ed = new SignalGeneratorEditor(this);
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


bool SignalGenerator::enable () {

	phasePerSample = double_Pi * 2.0 / (getSampleRate() / frequency);

	std::cout << "Signal generator received enable signal." << std::endl;
	return true;
}

bool SignalGenerator::disable() {
	
	std::cout << "Signal generator received disable signal." << std::endl;
	return true;
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
