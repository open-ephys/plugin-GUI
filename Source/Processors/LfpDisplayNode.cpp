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

#include "LfpDisplayNode.h"
#include "Visualization/LfpDisplayCanvas.h"
#include <stdio.h>

LfpDisplayNode::LfpDisplayNode()
	: GenericProcessor("LFP Viewer"),
	  bufferLength(5.0f), displayBufferIndex(0), displayGain(1),
	  abstractFifo(100)

{
	displayBuffer = new AudioSampleBuffer(8, 100);
	eventBuffer = new MidiBuffer();

	Array<var> timeBaseValues;
	timeBaseValues.add(1);
	timeBaseValues.add(2);
	timeBaseValues.add(5);
	timeBaseValues.add(10);

	parameters.add(Parameter("timebase",timeBaseValues, 0, 0));//true);//a,0);

	Array<var> displayGainValues;
	displayGainValues.add(1);
	displayGainValues.add(2);
	displayGainValues.add(4);
	displayGainValues.add(8);

	parameters.add(Parameter("display gain",displayGainValues, 0, 1));//true);//a,0);

}

LfpDisplayNode::~LfpDisplayNode()
{
	//deleteAndZero(displayBuffer);
	//deleteAndZero(eventBuffer);
}

AudioProcessorEditor* LfpDisplayNode::createEditor()
{

	editor = new LfpDisplayEditor(this);	
	return editor;

}

void LfpDisplayNode::updateSettings()
{
	std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;
}

bool LfpDisplayNode::resizeBuffer()
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

bool LfpDisplayNode::enable()
{

	if (resizeBuffer())
	{
		LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
		editor->enable();
		return true;
	} else {
		return false;
	}

}

bool LfpDisplayNode::disable()
{
	LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
	editor->disable();
	return true;
}

void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{
	LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
	if (ed->canvas != 0)
		ed->canvas->setParameter(parameterIndex, newValue);
}

void LfpDisplayNode::process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples)
{
	// 1. place any new samples into the displayBuffer
	//std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;


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


	///// failed attempt to use abstractFifo:

	// int start1, size1, start2, size2;

	// abstractFifo.prepareToWrite(nSamples, start1, size1, start2, size2);

	// if (size1 > 0)
	// {
	// 	for (int chan = 0; chan < buffer.getNumChannels(); chan++)
	// 	{	
	// 		displayBuffer->copyFrom(chan,  			// destChannel
	// 						    start1, 			// destStartSample
	// 						    buffer, 			// source
	// 						    chan, 				// source channel
	// 						    0,					// source start sample
	// 						    size1); 			// numSamples

	// 	}

	// 	displayBufferIndex += size1;
	// }

	// if (size2 > 0)
	// {
	// 	for (int chan = 0; chan < buffer.getNumChannels(); chan++)
	// 	{	
	// 		displayBuffer->copyFrom(chan,  			// destChannel
	// 						    start2, 			// destStartSample
	// 						    buffer, 			// source
	// 						    chan, 				// source channel
	// 						    size1,				// source start sample
	// 						    size2); 			// numSamples

	// 	}

	// 	displayBufferIndex = size2;
	// }

	// std::cout << displayBufferIndex << std::endl;

	// abstractFifo.finishedWrite(size1 + size2);


}

