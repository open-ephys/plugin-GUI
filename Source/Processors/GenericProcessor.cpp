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

#include "GenericProcessor.h"
#include "../UI/UIComponent.h"

GenericProcessor::GenericProcessor(const String& name_) : 
    name(name_),
	sourceNode(0), destNode(0),
	isEnabled(true), 
	saveOrder(-1), loadOrder(-1),
	nextAvailableChannel(0), currentChannel(-1),
	wasConnected(false), nullParam("VOID", false, -1),
	audioAndRecordNodeStartChannel(0)
{
}

GenericProcessor::~GenericProcessor()
{
}

AudioProcessorEditor* GenericProcessor::createEditor()
{
	editor = new GenericEditor (this); 
	return editor;
}

Parameter& GenericProcessor::getParameterByName(String name_)
{
	// doesn't work
	for (int i = 0; i < getNumParameters(); i++)
	{

		Parameter& p =  parameters.getReference(i);
		const String parameterName = p.getName();

		if (parameterName.compare(name_) == 0) // fails at this point
			return p;//parameters.getReference(i);
	} 


	return nullParam;

}

Parameter& GenericProcessor::getParameterReference(int parameterIndex)
{

	return parameters.getReference(parameterIndex);

}

void GenericProcessor::setParameter (int parameterIndex, float newValue)
{
    std::cout << "Setting parameter" << std::endl;
    
	if (currentChannel >= 0)
	{
		Parameter& p =  parameters.getReference(parameterIndex);
		p.setValue(newValue, currentChannel);
	}

}

const String GenericProcessor::getParameterName (int parameterIndex)
{
	//Parameter& p = parameters[parameterIndex];
	//return parameters[parameterIndex].getName();
}

const String GenericProcessor::getParameterText (int parameterIndex)
{
	//Parameter& p = parameters[parameterIndex];
	//return parameters[parameterIndex].getDescription();
}
	

void GenericProcessor::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	// use the enable() function instead
	// prepareToPlay() is called by Juce as soon as a processor is created
	// enable() is only called by the ProcessorGraph just before the start of acquisition
}

void GenericProcessor::releaseResources() 
{	
	// use the disable() function instead
	// releaseResources() is called by Juce at unpredictable times
	// disable() is only called by the ProcessorGraph at the end of acquisition
}

int GenericProcessor::getNextChannel(bool increment)
{
	int chan = nextAvailableChannel;

	std::cout << "Next channel: " << chan << ", num inputs: " << getNumInputs() << std::endl;
 
	if (increment)
		nextAvailableChannel++;
	
	if (chan < getNumInputs())
		return chan;
	else
		return -1;

}

void GenericProcessor::resetConnections()
{
	//std::cout << "Resetting connections" << std::endl;
	// if (isAudioOrRecordNode())
	// 	nextAvailableChannel = 2;
	// else
	nextAvailableChannel = 0;

	wasConnected = false;
}

void GenericProcessor::setNumSamples(MidiBuffer& events, int sampleIndex) {

	uint8 data[2];

	data[0] = BUFFER_SIZE; 	// most-significant byte
    data[1] = nodeId; 		// least-significant byte

    events.addEvent(data, 		// spike data
                    2, 			// total bytes
                    sampleIndex); // sample index

}

int GenericProcessor::getNumSamples(MidiBuffer& events) {

	int numRead = 0;

	if (events.getNumEvents() > 0) 
	{
			
		int m = events.getNumEvents();

		//std::cout << getName() << " received " << m << " events." << std::endl;

		MidiBuffer::Iterator i (events);
		MidiMessage message(0xf4);

		int samplePosition = -5;

		while (i.getNextEvent (message, samplePosition)) {
			
			uint8* dataptr = message.getRawData();

			if (*dataptr == BUFFER_SIZE)
			{
				numRead = message.getTimeStamp();
			}
		}
	}

	return numRead;
}

void GenericProcessor::setSourceNode(GenericProcessor* sn)
{
	//std::cout << "My name is " << getName() << ". Setting source node." << std::endl;

	if (!isSource())
	{
	//	std::cout << " I am not a source." << std::endl;

		if (sn != 0)
		{

		//	std::cout << " The source is not blank." << std::endl;

			if (!sn->isSink())
			{
		//		std::cout << " The source is not a sink." << std::endl;

				if (sourceNode != sn) {

		//			std::cout << " The source is new and named " << sn->getName() << std::endl;
					
					if (this->isMerger())
						setMergerSourceNode(sn);
					else
						sourceNode = sn;

					sn->setDestNode(this);
					//setNumInputs(sn->getNumOutputs());
					//setSampleRate(sn->getSampleRate());
				} else {
		//			std::cout << "  The source node is not new." << std::endl;
				}
			} else {
		//		std::cout << " The source is a sink." << std::endl;
				sourceNode = 0;
			}

		} else {
	//		std::cout << " The source is blank." << std::endl;
			sourceNode = 0;
		}
	} else {
	//	std::cout << " I am a source. I can't have a source node." << std::endl;

		if (sn != 0)
			sn->setDestNode(this);
	}
}


void GenericProcessor::setDestNode(GenericProcessor* dn)
{


//	std::cout << "My name is " << getName() << ". Setting dest node." << std::endl;

	if (!isSink())
	{
	//	std::cout << "  I am not a sink." << std::endl;

		if (dn != 0)
		{
	//		std::cout << "  The dest node is not blank." << std::endl;

			if (!dn->isSource())
			{

		//		std::cout << "  The dest node is not a source." << std::endl;

				if (destNode != dn) 
				{
			//		std::cout << "  The dest node is new and named " << dn->getName() << std::endl;
//
					if (this->isSplitter())
						setSplitterDestNode(dn);
					else
						destNode = dn;

					dn->setSourceNode(this);
					//dn->setNumInputs(getNumOutputs());
					//dn->setSampleRate(getSampleRate());
				} else {
			//		std::cout << "  The dest node is not new." << std::endl;
				}
			} else {

			//	std::cout << "  The dest node is a source." << std::endl;

				destNode = 0;
			}
		} else {
		//	std::cout << "  The dest node is blank." << std::endl;

			destNode = 0;
		}
	} else {

		//std::cout << "  I am a sink, I can't have a dest node." << std::endl;
		//if (dn != 0)
		//	dn->setSourceNode(this);
	}
}

void GenericProcessor::clearSettings()
{
	originalSource = 0;
	numInputs = 0;
	numOutputs = 0;

	channels.clear();

}

void GenericProcessor::update()
{

	std::cout << getName() << " updating settings." << std::endl;

	clearSettings();
	
	if (sourceNode != 0)
	{
		// everything is inherited except numOutputs
		settings = sourceNode->settings;
		settings.numInputs = settings.numOutputs;
		settings.numOutputs = settings.numInputs;

	} else {

		settings.sampleRate = getDefaultSampleRate();
		settings.numOutputs = getDefaultNumOutputs();

		for (int i = 0; i < getNumOutputs(); i++)
			settings.bitVolts.add(getDefaultBitVolts());

		generateDefaultChannelNames(settings.outputChannelNames);

	}

	if (this->isSink())
	{
		settings.numOutputs = 0;
		settings.outputChannelNames.clear();
	}

	updateSettings(); // custom settings code

	// required for the ProcessorGraph to know the
	// details of this processor:
	setPlayConfigDetails(getNumInputs(),  // numIns
		                 getNumOutputs(), // numOuts
		                 44100.0,         // sampleRate
		                 128);            // blockSize

	editor->update(); // update editor settings

}

bool GenericProcessor::recordStatus(int chan)
{

	return getEditor()->getRecordStatus(chan);//recordChannels[chan];


}

bool GenericProcessor::audioStatus(int chan)
{

	return getEditor()->getAudioStatus(chan);//recordChannels[chan];


}

// void GenericProcessor::generateDefaultChannelNames(StringArray& names)
// {
// 	names.clear();

// 	for (int i = 0; i < settings.numOutputs; i++)
// 	{
// 		String channelName = "CH";
// 		channelName += (i+1);
// 		names.add(channelName);
// 	}

// }


int GenericProcessor::checkForEvents(MidiBuffer& midiMessages)
{

	if (midiMessages.getNumEvents() > 0) 
	{
			
		int m = midiMessages.getNumEvents();
		//std::cout << m << " events received by node " << getNodeId() << std::endl;

		MidiBuffer::Iterator i (midiMessages);
		MidiMessage message(0xf4);

		int samplePosition;
		i.setNextSamplePosition(samplePosition);

		while (i.getNextEvent (message, samplePosition)) {
			
			uint8* dataptr = message.getRawData();

			handleEvent(*dataptr, message);

		}

	}

	return -1;

}

void GenericProcessor::addEvent(MidiBuffer& eventBuffer,
							    uint8 type,
							    int sampleNum,
							    uint8 eventId,
							    uint8 eventChannel,
							    uint8 numBytes,
							    uint8* eventData)
{
	uint8 *data = new uint8[4+numBytes];

	data[0] = type;    // event type
    data[1] = nodeId;  // processor ID automatically added
    data[2] = eventId; // event ID
    data[3] = eventChannel; // event channel
    memcpy(&data[4], eventData, numBytes);

    eventBuffer.addEvent(data, 		// spike data
                          sizeof(data), // total bytes
                          sampleNum);     // sample index
	delete data;
}

// void GenericProcessor::unpackEvent(int type,
// 								   MidiMessage& event)
// {


// }

void GenericProcessor::processBlock (AudioSampleBuffer &buffer, MidiBuffer &eventBuffer)
{
	
	int nSamples = getNumSamples(eventBuffer); // removes first value from midimessages

	process(buffer, eventBuffer, nSamples);

	setNumSamples(eventBuffer, nSamples); // adds it back,
										  // even if it's unchanged

}
