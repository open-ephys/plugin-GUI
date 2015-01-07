
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
#include "ExampleProcessor.h"

ExampleProcessor::ExampleProcessor()
    : GenericProcessor("Example Processor") //, threshold(200.0), state(true)

{

    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

}

ExampleProcessor::~ExampleProcessor()
{

}



void ExampleProcessor::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);

    //threshold = newValue;

    //std::cout << float(p[0]) << std::endl;
    editor->updateParameterButtons(parameterIndex);
}

void ExampleProcessor::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{

    // for (int i = 0; i < getNumSamples(channels[0]->sourceNodeId); i++)
    // {
    //
    //     if ((*buffer.getReadPointer(0, i) < -threshold) && !state)
    //     {
    //
    //         // generate midi event
    //         addEvent(events, TTL, i);
    //
    //         state = true;
    //
    //     } else if ((*buffer.getReadPointer(0, i) > -threshold + bufferZone)  && state)
    //     {
    //         state = false;
    //     }
    //
    //
    // }


}
