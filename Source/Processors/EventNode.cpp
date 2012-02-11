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
	: GenericProcessor("Event Node")
{
	
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
	accumulator++;

	if (!isSource) {
		
		if (midiMessages.getNumEvents() > 0) {
			
			std::cout << "Events received by node " << getNodeId() << std::endl;

			 MidiBuffer::Iterator i (midiMessages);
			 MidiMessage message(0xf4);

			 int samplePosition;
			 i.setNextSamplePosition(samplePosition);

			 while (i.getNextEvent (message, samplePosition)) {
				
					//message.getChannel();

					//MidiMessage msgCopy = MidiMessage(message);
					int numbytes = message.getRawDataSize();
					uint8* dataptr = message.getRawData();

					

					std::cout << " Bytes received: " << numbytes << std::endl;
					std::cout << " Message timestamp = " << message.getTimeStamp() << std::endl;

					//std::cout << sizeof(int) << " " << sizeof(uint16) << std::endl;
 
 					std::cout << "   ";
					for (int n = 0; n < numbytes; n++) {
						std::cout << String(*dataptr++) << " ";
					}
					
					std::cout << std::endl << std::endl;
				 	//std::cout << "  Event on channel " << message.getRawData() << std::endl; //<< message.getRawDataSize() << std::endl;

			}


			// accumulator = 0;
		}//MidiBuffer::Iterator = midiMessages.

		//midiMessages.clear();

	} else {

		if (accumulator > 20) {

			uint8 data[95];

			for (int n = 0; n < sizeof(data); n++) {
				data[n] = 1;
			}

			//MidiMessage event = MidiMessage::noteOn(2,1,10.0f);
			MidiMessage event = MidiMessage(data, 	// spike data (float)
											sizeof(data), 	// number of bytes to use
											1000.0 	// timestamp (64-bit)
											);
			
			//event.setChannel(1);

			midiMessages.addEvent(data, sizeof(data), 5);
			//midiMessages.addEvent(event, 1);

			for (int n = 0; n < sizeof(data); n++) {
				data[n] = 2;
			}

			midiMessages.addEvent(data, sizeof(data), 10);

			for (int n = 0; n < sizeof(data); n++) {
				data[n] = 3;
			}

			midiMessages.addEvent(data, sizeof(data), 15);

			//midiMessages.addEvent(event, 5);

			//std::cout << "Midi buffer contains " << midiMessages.getNumEvents() << " events." << std::endl;

			accumulator = 0;
		}
		

	}

	
	

}
