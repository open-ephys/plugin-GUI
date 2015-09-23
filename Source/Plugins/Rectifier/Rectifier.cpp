
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
#include "Rectifier.h"

Rectifier::Rectifier()
: GenericProcessor("Rectifier")

{
    // It would be nice to have the option to do -abs for negative events (e.g. sharp waves)
    //parameters.add(Parameter("Sign", -1.0, 1.0, 1.0, -1.0));
    
}

Rectifier::~Rectifier()
{
    
}



void Rectifier::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    // std::cout << "Setting Rectifier" << std::endl;
    
    if (currentChannel >= 0)
    {
        Parameter& p =  parameters.getReference(parameterIndex);
        p.setValue(newValue, currentChannel);
    }
}

void Rectifier::process(AudioSampleBuffer& buffer,
                        MidiBuffer& events)
{
    int nChannels = buffer.getNumChannels();
    
    for (int ch = 0; ch < nChannels ; ch++)
    {
        int nSamples = buffer.getNumSamples();
        float* bufPtr = buffer.getWritePointer(ch);
        for (int n = 0; n < nSamples; n++)
        {
            *(bufPtr + n) = fabsf(*(bufPtr + n));
        }
    }
    
}
