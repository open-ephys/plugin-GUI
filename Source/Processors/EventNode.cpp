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
#include "EventNode.h"

EventNode::EventNode()
	: GenericProcessor("Event Generator"), Hz(1), accumulator(0)
{
	setNumOutputs(0);
	setNumInputs(0);
//	setSampleRate(44100.0);

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

}

EventNode::~EventNode()
{

}

AudioProcessorEditor* EventNode::createEditor()
{
	EventNodeEditor* editor = new EventNodeEditor(this, viewport);
	setEditor(editor);

	std::cout << "Creating editor." << std::endl;
	//editor->setUIComponent(getUIComponent());
	editor->setConfiguration(config);

	//filterEditor = new FilterEditor(this);
	return editor;

	//return 0;
}

//AudioProcessorEditor* FilterNode::createEditor(AudioProcessorEditor* const editor)
//{
	
//	return editor;
//}
void EventNode::setParameter (int parameterIndex, float newValue)
{
	std::cout << "Setting frequency to " << newValue << " Hz." << std::endl;
	Hz = newValue;
}


void EventNode::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	//std::cout << "Filter node preparing." << std::endl;
}

//void EventNode::enable() 
//{
	//prepareToPlay();
//}


//void EventNode::disable()
//{
	//releaseResources();
//}

void EventNode::releaseResources() 
{	
}

void EventNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{
	midiMessages.clear();

	for (int i = 0; i < buffer.getNumSamples(); i++)
	{
		accumulator += 1.0f;

		if (accumulator > getSampleRate()/Hz)
		{
			//std::cout << "Adding message." << std::endl;
			addMidiEvent(midiMessages, 10, i);
			accumulator = 0;
		}

	}	
	

}
