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

#include "SpikeDisplayNode.h"
#include <stdio.h>

#include "Channel.h"

SpikeDisplayNode::SpikeDisplayNode()
    : GenericProcessor("Spike Viewer"),
      bufferSize(0)

{
    //	displayBuffer = new AudioSampleBuffer(8, 100);
    eventBuffer = new MidiBuffer();
}

SpikeDisplayNode::~SpikeDisplayNode()
{
    //deleteAndZero(displayBuffer);
    //deleteAndZero(eventBuffer);
}

AudioProcessorEditor* SpikeDisplayNode::createEditor()
{
    std::cout<<"Creating SpikeDisplayCanvas."<<std::endl;

    editor = new SpikeDisplayEditor(this);
    return editor;

}

// void SpikeDisplayNode::updateSettings()
// {
// 	//std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;

// }

// void SpikeDisplayNode::updateVisualizer()
// {

// }

bool SpikeDisplayNode::enable()
{
    std::cout << "SpikeDisplayNode::enable()" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->enable();
    return true;

}

bool SpikeDisplayNode::disable()
{
    std::cout << "SpikeDisplayNode disabled!" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();
    return true;
}

int SpikeDisplayNode::getNumberOfChannelsForElectrode(int elec)
{
    std::cout<<"SpikeDisplayNode::getNumberOfChannelsForInput(" << elec << ")"<<std::endl;

    int electrodeIndex = -1;

    for (int i = 0; i < eventChannels.size(); i++)
    {
		if ((eventChannels[i]->eventType < 999) && (eventChannels[i]->eventType > SPIKE_BASE_CODE))
        {
            electrodeIndex++;

            if (electrodeIndex == elec)
            {
                std::cout << "Electrode " << elec << " has " << eventChannels[i]->eventType << " channels" << std::endl;
                return (eventChannels[i]->eventType - SPIKE_BASE_CODE);
            }
        }
    }

    return 0;
}

String SpikeDisplayNode::getNameForElectrode(int elec)
{

    int electrodeIndex = -1;

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if ((eventChannels[i]->eventType < 999) && (eventChannels[i]->eventType > SPIKE_BASE_CODE))
        {
            electrodeIndex++;

            if (electrodeIndex == elec)
            {
                std::cout << "Electrode " << elec << " has " << eventChannels[i]->eventType << " channels" << std::endl;
                return eventChannels[i]->name;
            }
        }
    }

    return " ";
}

int SpikeDisplayNode::getNumElectrodes()
{
    int nElectrodes = 0;

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if ((eventChannels[i]->eventType < 999) && (eventChannels[i]->eventType > SPIKE_BASE_CODE))
        {
            nElectrodes++;
        }
    }

    return nElectrodes;

}

void SpikeDisplayNode::startRecording()
{
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->startRecording();
    
}

void SpikeDisplayNode::stopRecording()
{
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->stopRecording();
}


void SpikeDisplayNode::setParameter(int param, float val)
{
    std::cout<<"Got Param:"<< param<< " with value:"<<val<<std::endl;
}



void SpikeDisplayNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{

    checkForEvents(events); // automatically calls 'handleEvent

}

void SpikeDisplayNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{

    //std::cout << "Received event of type " << eventType << std::endl;

    if (eventType == SPIKE)
    {
       // const MessageManagerLock mmLock; // get the lock to prevent the midi buffer from being read
        
      eventBuffer->addEvent(event, 0);
    }

}
