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
#include "EventNode.h"

#include "../Channel/Channel.h"

EventNode::EventNode()
    : GenericProcessor("Event Generator"), accumulator(0), Hz(1)
{

    Array<var> hzValues;
    hzValues.add(0.25f);
    hzValues.add(0.5f);
    hzValues.add(1.0f);
    hzValues.add(2.0f);

    parameters.add(Parameter("Frequency",hzValues, 0, 0));

}

EventNode::~EventNode()
{

}

AudioProcessorEditor* EventNode::createEditor()
{
    editor = new EventNodeEditor(this, true);
    return editor;
}

void EventNode::updateSettings()
{
    // add event channels

    Channel* ch = new Channel(this, 1, EVENT_CHANNEL);
    ch->setName("Trigger");

    eventChannels.add(ch);

}


// void EventNode::setParameter (int parameterIndex, float newValue)
// {
// 	std::cout << "Setting frequency to " << newValue << " Hz." << std::endl;
// 	Hz = newValue;
// }


void EventNode::process(AudioSampleBuffer& buffer,
                        MidiBuffer& events)
{
    events.clear();

    //std::cout << "Adding message." << std::endl;

    Parameter& p1 =  parameters.getReference(0);

    //std::cout << (float) p1[0] << std::endl;

    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        accumulator += 1.0f;

        if (accumulator > getSampleRate() / (float) p1[0])
        {
            std::cout << "Adding message." << std::endl;
            addEvent(events, // MidiBuffer
                     TTL,    // eventType
                     i,      // sampleNum
                     1,	     // eventID
                     1		 // eventChannel
                    );

            accumulator = 0;
        }

    }

}
