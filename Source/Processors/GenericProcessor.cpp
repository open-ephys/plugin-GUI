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
	wasConnected(false)
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


void GenericProcessor::setParameter (int parameterIndex, float newValue)
{


}

GenericProcessor* GenericProcessor::getOriginalSourceNode()
{
	if (isSource())
	{
		return this;
	} else {
		
		GenericProcessor* source = getSourceNode();

		if (source != 0)
		{
			while (!source->isSource() && source != 0)
			{
				source = source->getSourceNode();
			}

			return source;

		} else {
			return 0;
		}
	}
}

int GenericProcessor::getDefaultNumOutputs()
{
	if (!isSink())
	{
		return 10;
	} else {
		return 0;
	}
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


// void GenericProcessor::sendMessage(const String& msg)
// {
// 	std::cout << "Message: ";
// 	std::cout << msg << "...." << std::endl;
// 	UI->transmitMessage(msg);
// }


int GenericProcessor::getNextChannel(bool increment)
{
	int chan = nextAvailableChannel;

	//std::cout << chan << std::endl;

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
	nextAvailableChannel = 0;
	wasConnected = false;
}

void GenericProcessor::setNumSamples(MidiBuffer& midiMessages, int numberToAdd) {

	uint8 data[2];

	data[0] = numberToAdd >> 8; 	// most-significant byte
    data[1] = numberToAdd & 0xFF; 	// least-significant byte

    midiMessages.addEvent(data, 		// spike data
                          sizeof(data), // total bytes
                          -1);           // sample index


}

int GenericProcessor::getNumSamples(MidiBuffer& midiMessages) {

	int numRead = 0;

	if (midiMessages.getNumEvents() > 0) 
	{
			
		int m = midiMessages.getNumEvents();

		MidiBuffer::Iterator i (midiMessages);
		MidiMessage message(0xf4);

		int samplePosition = -5;

		while (i.getNextEvent (message, samplePosition)) {
			
				int numbytes = message.getRawDataSize();
				uint8* dataptr = message.getRawData();

				if (message.getTimeStamp() < 0)
					numRead = (*dataptr<<8) + *(dataptr+1);
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

void GenericProcessor::updateSettings()
{
	
	if (sourceNode != 0)
	{
		setSampleRate(sourceNode->getSampleRate());
		setNumInputs(sourceNode->getNumOutputs());
	} else {
		setSampleRate(getDefaultSampleRate());
		setNumInputs(0);
		setNumOutputs(getDefaultNumOutputs());
	}

	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

	updateParameters();

	GenericEditor* editor = (GenericEditor*) getEditor();

	editor->update();

}

void GenericProcessor::updateParameters()
{
	
}


int GenericProcessor::getNumInputs()
{
	return numInputs;
}

int GenericProcessor::getNumOutputs()
{
	return numOutputs;
}

void GenericProcessor::setNumInputs(int n) {
	numInputs = n;
	// if (destNode != 0)
	// {
	// 	destNode->setNumInputs();
	// }
	//setPlayConfigDetails(numInputs,numOutputs,44100.0,1024);
}

void GenericProcessor::setNumInputs() {
	
	int n = getSourceNode()->getNumOutputs();
	setNumInputs(n);
}

void GenericProcessor::setNumOutputs()
{
	setNumOutputs(getNumInputs());
}

void GenericProcessor::setNumOutputs(int n) {
	numOutputs = n;
	//setPlayConfigDetails(numInputs,numOutputs,44100.0,1024);
}

float GenericProcessor::getSampleRate()
{
	return sampleRate;
}
void GenericProcessor::setSampleRate(float sr)
{
	sampleRate = sr;
}

int GenericProcessor::checkForMidiEvents(MidiBuffer& midiMessages)
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
			
				int numbytes = message.getRawDataSize();
				uint8* dataptr = message.getRawData();

				//std::cout << " Bytes received: " << numbytes << std::endl;
				//std::cout << " Message timestamp = " << message.getTimeStamp() << std::endl;

				if (message.getTimeStamp() >= 0)
				{
					int value = (*dataptr<<8) + *(dataptr+1);
					//std::cout << "   " << value << std::endl;
					return value;
				}
		}

	}

	return -1;

}

void GenericProcessor::addMidiEvent(MidiBuffer& midiMessages, int numberToAdd, int sampleNum)
{
	uint8 data[2];

	data[0] = numberToAdd >> 8; 	// most-significant byte
    data[1] = numberToAdd & 0xFF; 	// least-significant byte

    midiMessages.addEvent(data, 		// spike data
                          sizeof(data), // total bytes
                          sampleNum);     // sample index
}

void GenericProcessor::processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages)
{
	
	int nSamples = getNumSamples(midiMessages); // removes first value from midimessages

	process(buffer, midiMessages, nSamples);

	setNumSamples(midiMessages, nSamples); // adds it back,
										   // even if it's unchanged

}
