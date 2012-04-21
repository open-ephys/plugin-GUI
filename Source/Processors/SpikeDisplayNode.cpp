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

#include "SpikeDisplayNode.h"
#include <stdio.h>



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

void SpikeDisplayNode::updateSettings()
{
	std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;
}


bool SpikeDisplayNode::enable()
{
	std::cout<<"SpikeDisplayNode::enable()"<<std::endl;
	SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
	editor->enable();
	return true;
		
}

bool SpikeDisplayNode::disable()
{
	std::cout<<"SpikeDisplayNode disabled!"<<std::endl;
	SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
	editor->disable();
	return true;
}

int SpikeDisplayNode::getNumberOfChannelsForInput(int i){
	std::cout<<"SpikeDisplayNode::getNumberOfChannelsForInput()"<<std::endl;
	return 1;
}


void SpikeDisplayNode::setParameter (int param, float val)
{
	std::cout<<"Got Param:"<< param<< " with value:"<<val<<std::endl;
}



void SpikeDisplayNode::process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples)
{
	//std::cout<<"SpikeDisplayNode::process"<<std::endl;
	/*
	uint64_t ts =  00000; 
	int noise = 10;
	SpikeObject newSpike;

	generateSimulatedSpike(&newSpike, ts, noise);
	
	spikebuffer.push(newSpike);
	bufferSize++;
	*/
}

bool SpikeDisplayNode::getNextSpike(SpikeObject *spike){
	std::cout<<"SpikeDisplayNode::getNextSpike()"<<std::endl;
	/*
	if (bufferSize<1 || spikebuffer.empty())
		return false;
	else{
		SpikeObject s = spikebuffer.front();
		spikebuffer.pop();
		bufferSize--;
		*spike = s;
		return true;
	}
	*/
	return false;

}