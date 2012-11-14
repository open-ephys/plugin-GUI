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
#include "SourceNode.h"

FPGAOutput::FPGAOutput()
	: GenericProcessor("FPGA Output"), isEnabled(true)
{
    

}

FPGAOutput::~FPGAOutput()
{

}

AudioProcessorEditor* FPGAOutput::createEditor()
{
	editor = new FPGAOutputEditor(this);
	return editor;
}

void FPGAOutput::handleEvent(int eventType, MidiMessage& event)
{
    if (eventType == TTL && isEnabled)
    {

        uint8* dataptr = event.getRawData();

        int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

        if (eventId == 1 && eventChannel == 3) // channel 3 only at the moment
        {
            sendActionMessage("HI");
            isEnabled = false;
            startTimer(2); // 2 ms pulses
        }

        
    }
    
}

void FPGAOutput::updateSettings()
{
    removeAllActionListeners();
    
    GenericProcessor* src;
    GenericProcessor* lastSrc;
    
    lastSrc = getSourceNode();
    src = getSourceNode();
    
    while (src != 0)
    {
        lastSrc = src;
        src = lastSrc->getSourceNode();
    }
    
    //SourceNode* s = (SourceNode*) settings.originalSource;
    std::cout << "FPGA Output node communicating with " << lastSrc->getName() << std::endl;
    
    SourceNode* s = (SourceNode*) lastSrc;
    
    addActionListener(s);
    //dataThread = (FPGAThread*) s->getThread();
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
	//dataThread->setOutputLow();
    sendActionMessage("LO");
    
    isEnabled = true;
	stopTimer();
}