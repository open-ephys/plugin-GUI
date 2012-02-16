/*
  ==============================================================================

    EventNode.cpp
    Created: 13 Jun 2011 10:42:26am
    Author:  jsiegle

  ==============================================================================
*/


#include <stdio.h>
#include "EventNode.h"
//#include "FilterEditor.h"

EventNode::EventNode()
	: GenericProcessor("Event Generator"), Hz(1), accumulator(0)
{
	setNumOutputs(0);
	setNumInputs(0);
	setSampleRate(44100.0);

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

}

EventNode::~EventNode()
{

}

// AudioProcessorEditor* EventNode::createEditor()
// {
// 	FilterEditor* filterEditor = new FilterEditor(this, viewport);
	
// 	std::cout << "Creating editor." << std::endl;
// 	//filterEditor = new FilterEditor(this);
// 	return filterEditor;

// 	//return 0;
// }

//AudioProcessorEditor* FilterNode::createEditor(AudioProcessorEditor* const editor)
//{
	
//	return editor;
//}
void EventNode::setParameter (int parameterIndex, float newValue)
{
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
