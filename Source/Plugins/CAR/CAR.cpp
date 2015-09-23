
/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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




#include <stdio.h>
#include "CAR.h"
    
CAR::CAR()
    : GenericProcessor("Common Avg Ref") //, threshold(200.0), state(true)

{

    parameters.add(Parameter("Gain (%)", 0.0, 100.0, 100.0, 0));

    avgBuffer = AudioSampleBuffer(1,10000); // 1-dimensional buffer to hold the avg

}

CAR::~CAR()
{

}



void CAR::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    // std::cout << "Setting CAR Gain" << std::endl;

    if (currentChannel >= 0)
    {
        Parameter& p =  parameters.getReference(parameterIndex);
        p.setValue(newValue, currentChannel);
    }
}

void CAR::process(AudioSampleBuffer& buffer,
                  MidiBuffer& events)
{
	int nChannels = buffer.getNumChannels();

    float gain = -1.0f * float(getParameterVar(0, 0)) / 100.0f; // just use channel 0, since we can't have individual channel settings at the moment

    avgBuffer.clear();

    for (int j = 0; j < nChannels; j++)
	{
		avgBuffer.addFrom(0,           // destChannel 
                          0,           // destStartSample
                          buffer,      // source
                          j,           // sourceChannel
                          0,           // sourceStartSample
                          buffer.getNumSamples(), // numSamples
                          1.0f); // gain to apply       
	}

    avgBuffer.applyGain(1.0f/float(nChannels));

    for (int j = 0; j < nChannels; j++)
    {
        buffer.addFrom(j,           // destChannel 
                       0,           // destStartSample
                       avgBuffer,   // source
                       0,           // sourceChannel
                       0,           // sourceStartSample
                       buffer.getNumSamples(), // numSamples
                       gain); // gain to apply            
    }

}
