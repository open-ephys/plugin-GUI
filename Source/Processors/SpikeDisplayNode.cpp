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
	: GenericProcessor("SpikeDisplay Viewer"),
	  bufferLength(200), displayGain(1),
	  displayBufferIndex(0), abstractFifo(100)

{
	displayBuffer = new AudioSampleBuffer(8, 100);
	eventBuffer = new MidiBuffer();
}

SpikeDisplayNode::~SpikeDisplayNode()
{
	//deleteAndZero(displayBuffer);
	//deleteAndZero(eventBuffer);
}

AudioProcessorEditor* SpikeDisplayNode::createEditor()
{
	std::cout<<"SpikeDisplayNode Created!"<<std::endl;

	editor = new SpikeDisplayEditor(this);	
	return editor;

}

void SpikeDisplayNode::updateSettings()
{
	std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;
}

bool SpikeDisplayNode::resizeBuffer()
{
	
	int nSamples = (int) getSampleRate()*bufferLength;
	int nInputs = getNumInputs();

	std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

	if (nSamples > 0 && nInputs > 0)
	{
		abstractFifo.setTotalSize(nSamples);
		displayBuffer->setSize(nInputs, nSamples);
		return true;
	} else {
		return false;
	}

}

bool SpikeDisplayNode::enable()
{
	std::cout<<"SpikeDisplayNode enabled!"<<std::endl;
	if (resizeBuffer())
	{
		SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
		editor->enable();
		return true;
	} else {
		return false;
	}

}

bool SpikeDisplayNode::disable()
{
	std::cout<<"SpikeDisplayNode disabled!"<<std::endl;
	SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
	editor->disable();
	return true;
}

void SpikeDisplayNode::setParameter (int parameterIndex, float newValue)
{
	std::cout<<"SpikeDisplayNode setParameter!"<<std::endl;
}

void SpikeDisplayNode::process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples)
{
	std::cout<<"SpikeDisplayNode process!"<<std::endl;
	// 1. place any new samples into the displayBuffer

	int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex;

	if (nSamples < samplesLeft)
	{

		for (int chan = 0; chan < buffer.getNumChannels(); chan++)
		{	
			displayBuffer->copyFrom(chan,  				// destChannel
							    displayBufferIndex, // destStartSample
							    buffer, 			// source
							    chan, 				// source channel
							    0,					// source start sample
							    nSamples); 			// numSamples

		}
		displayBufferIndex += (nSamples);

	} else {

		int extraSamples = nSamples - samplesLeft;

		for (int chan = 0; chan < buffer.getNumChannels(); chan++)
		{	
			displayBuffer->copyFrom(chan,  				// destChannel
							    displayBufferIndex, // destStartSample
							    buffer, 			// source
							    chan, 				// source channel
							    0,					// source start sample
							    samplesLeft); 		// numSamples

			displayBuffer->copyFrom(chan,
								0,
								buffer,
								chan,
								samplesLeft,
								extraSamples);
		}

		displayBufferIndex = extraSamples;
	}


	
}

