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



#include <stdio.h>
#include "FPGAOutput.h"

FPGAOutput::FPGAOutput()
	: GenericProcessor("FPGA Output")
{
}

FPGAOutput::~FPGAOutput()
{

}

// AudioProcessorEditor* FPGAOutput::createEditor()
// {
// 	editor = new FPGAOutputEditor(this);
// 	return editor;
// }

void FPGAOutput::handleEvent(int eventType, MidiMessage& event)
{
    if (eventType == TTL)
    {
        //startTimer((int) float(event.getTimeStamp())/getSampleRate()*1000.0);
    }
    
}

void FPGAOutput::setParameter (int parameterIndex, float newValue)
{

}

void FPGAOutput::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{
	

	checkForEvents(events);
	

}

void FPGAOutput::timerCallback()
{
	std::cout << "FIRE!" << std::endl;
    
	stopTimer();
}