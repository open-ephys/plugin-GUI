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
	: GenericProcessor("FPGA Output"), isEnabled(true), TTLchannel(3), continuousStim(false)
{
    
    Array<var> channelNumbers;
    channelNumbers.add(0);
    channelNumbers.add(3);
    channelNumbers.add(5);
    //
   // channelNumbers.add(6);

    parameters.add(Parameter("TTL channel",channelNumbers, 1, 0));

    Array<var> stimType;
    stimType.add(0);
    stimType.add(1);
    stimType.add(2);

    parameters.add(Parameter("Stim Type",stimType, 0, 1));

}

FPGAOutput::~FPGAOutput()
{

}

AudioProcessorEditor* FPGAOutput::createEditor()
{
	editor = new FPGAOutputEditor(this);
	return editor;
}

void FPGAOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL && isEnabled)
    {

        uint8* dataptr = event.getRawData();

        int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

       // std::cout << "FPGA output received event: " << eventNodeId << " " << eventId << " " << eventChannel << std::endl;

        if (eventId == 1 && eventChannel == TTLchannel) // channel 3 only at the moment
        {
            sendActionMessage("HI");
            isEnabled = false;

            if (!continuousStim)
                startTimer(5); // pulse width
           // else
            //    startTimer(25); // pulse width

        } else if (eventId == 0 && eventChannel == TTLchannel)// && eventChannel == TTLchannel)
        {
            if (continuousStim)
            { /// this isn't working
                sendActionMessage("LO");
                isEnabled = true;
             //   stopTimer();
            }
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
    
    if (lastSrc != 0)
    {
        SourceNode* s = (SourceNode*) lastSrc;
        addActionListener(s);
        std::cout << "FPGA Output node communicating with " << lastSrc->getName() << std::endl;
    } else {
        std::cout << "FPGA Output couldn't find a source" << std::endl;
    }

    
    //dataThread = (FPGAThread*) s->getThread();
}

void FPGAOutput::setParameter (int parameterIndex, float newValue)
{

    //std::cout << "FPGAOutput received parameter change notification." << std::endl;

    if (parameterIndex == 0)
    {
        TTLchannel = int(newValue);
    } else if (parameterIndex == 1)
    {
        if (newValue == 0.0f)
        {
            continuousStim = false;
        } else {
            continuousStim = true;
        }
   }

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

    if (!continuousStim)
    {
        sendActionMessage("LO");
    
        isEnabled = true;
        stopTimer();

    } else {

        if (isEnabled)
        {
            sendActionMessage("HI");
            isEnabled = false;
        } else {
            sendActionMessage("LO");
            isEnabled = true;
        }
            

    }
    
}