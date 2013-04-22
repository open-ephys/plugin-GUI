/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "PulsePalOutput.h"


PulsePalOutput::PulsePalOutput()
    : GenericProcessor("Pulse Pal")
{

    pulsePal.initialize();

    // changes the display, but adds a trailing zero
    pulsePal.updateDisplay("GUI Connected"," Click for menu");

    // doesn't seem to do anything:
    pulsePal.setPhase1Duration(2, 20000);

    // check to make sure it's running
    // doesn't seem to do anything yet
    pulsePal.triggerChannel(1);



}

PulsePalOutput::~PulsePalOutput()
{
    pulsePal.updateDisplay("Pulse Pal v0.3","Click for menu");
}

AudioProcessorEditor* PulsePalOutput::createEditor()
{
    editor = new PulsePalOutputEditor(this, true);
    return editor;
}

void PulsePalOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
        std::cout << "Received an event!" << std::endl;
       // do something cool
    }

}

void PulsePalOutput::setParameter(int parameterIndex, float newValue)
{

}

void PulsePalOutput::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events,
                         int& nSamples)
{


    checkForEvents(events);


}
