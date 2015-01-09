
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
                               MidiBuffer& events,
                               int& nSamples)
{
	int nChannels=buffer.getNumChannels();
    for (int i = 0; i < nSamples; i++)
    {
    	float average=0;
    	for (int j = 0; j < (nChannels); j++)
    	{
    		average=average+buffer.getSample(j, i);
    	}
    	average=average/nChannels;

    	for (int j = 0; j < nChannels; j++)
    	{
    		float gain=getParameterVar(0, j);
    		// Subtract from sample
    		float subtracted=(buffer.getSample(j,i)*(1+(1/nChannels*gain/100))-average*gain/100);
			buffer.setSample(j,i, subtracted);
    	}    	
    }
}
