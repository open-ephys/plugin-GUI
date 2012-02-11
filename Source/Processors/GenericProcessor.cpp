/*
  ==============================================================================

    GenericProcessor.cpp
    Created: 7 May 2011 2:26:54pm
    Author:  jsiegle

  ==============================================================================
*/

#include "GenericProcessor.h"
#include "../UI/UIComponent.h"

GenericProcessor::GenericProcessor(const String& name_) : name(name_),
	sourceNode(0), destNode(0), editor(0)
	
{

	//name = "Generic Processor";
	//setSourceNode(source_);
	//setDestNode(dest_);

	//setNumInputs();
	//setNumOutputs();

	//setPlayConfigDetails(getNumInputs(),getNumOutputs(),1024);

}

GenericProcessor::~GenericProcessor()
{
	//deleteAllChildren();
	//std::cout << name << " deleting editor." << std::endl;

	if (editor != 0)
	{
		delete(editor);
		editor = 0;
	}
}

AudioProcessorEditor* GenericProcessor::createEditor()
{
	editor = new GenericEditor (this, viewport); 
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

// void GenericProcessor::setViewport(FilterViewport* vp) {
	
// 	viewport = vp;
// }

void GenericProcessor::setDataViewport(DataViewport* dv)

{	
	std::cout << "Processor data viewport: " << dv << std::endl;
	dataViewport = dv;
}

void GenericProcessor::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	//std::cout << "Preparing to play." << std::endl;

}

void GenericProcessor::releaseResources() 
{	
}


// void GenericProcessor::sendMessage(const String& msg)
// {
// 	std::cout << "Message: ";
// 	std::cout << msg << "...." << std::endl;
// 	UI->transmitMessage(msg);
// }


void GenericProcessor::setNumSamples(MidiBuffer& midiMessages, int numberToAdd) {
	//lock.enter();
	//*numSamplesInThisBuffer = n;
	//lock.exit();

	uint8 data[2];

	data[0] = numberToAdd >> 8; 	// most-significant byte
    data[1] = numberToAdd & 0xFF; 	// least-significant byte

    midiMessages.addEvent(data, 		// spike data
                          sizeof(data), // total bytes
                          0);           // sample index


}

int GenericProcessor::getNumSamples(MidiBuffer& midiMessages) {
	//lock.enter();
	//int numRead = *numSamplesInThisBuffer;
	//lock.exit();
	int numRead = 0;

	if (midiMessages.getNumEvents() > 0) 
	{
			
		int m = midiMessages.getNumEvents();

		// if (m == 1)
		// 	std::cout << m << " event received by node " << getNodeId() << std::endl;
		// else
		// 	std::cout << m << " events received by node " << getNodeId() << std::endl;

		MidiBuffer::Iterator i (midiMessages);
		MidiMessage message(0xf4);

		int samplePosition = 0;
		//i.setNextSamplePosition(samplePosition);

		while (i.getNextEvent (message, samplePosition)) {
			
				int numbytes = message.getRawDataSize();
				uint8* dataptr = message.getRawData();

				//std::cout << " Bytes received: " << numbytes << std::endl;
				//std::cout << " Message timestamp = " << message.getTimeStamp() << std::endl;

				numRead = (*dataptr<<8) + *(dataptr+1);
				//std::cout << "   " << numRead << std::endl;
		}

	}

	return numRead;
}

void GenericProcessor::setSourceNode(GenericProcessor* sn)
{
	if (!isSource())
	{
		if (sn != 0)
		{
			if (!sn->isSink())
			{
				if (sourceNode != sn) {
					sourceNode = sn;
					sn->setDestNode(this);
					setNumInputs(sn->getNumOutputs());
					setSampleRate(sn->getSampleRate());
				}
			} else {
				sourceNode = 0;
			}
		} else {
			sourceNode = 0;
		}
	} else {
		if (sn != 0)
			sn->setDestNode(this);
	}
}


void GenericProcessor::setDestNode(GenericProcessor* dn)
{
	if (!isSink())
	{
		if (dn != 0)
		{
			if (!dn->isSource())
			{
				if (destNode != dn) 
				{
					destNode = dn;
					dn->setSourceNode(this);
				}
			} else {
				destNode = 0;
			}
		} else {
			destNode = 0;
		}
	} else {
		if (dn != 0)
			dn->setSourceNode(this);
	}
}

// void GenericProcessor::setSourceNode(GenericProcessor* sn)
// {
// 	if (!isSource())
// 		sourceNode = sn;
// 	else
// 		sourceNode = 0;
// }

// void GenericProcessor::setDestNode(GenericProcessor* dn)
// {
// 	if (!isSink())
// 		destNode = dn
// 	else
// 		destNode = 0;
// }
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

void GenericProcessor::checkForMidiEvents(MidiBuffer& midiMessages)
{

	if (midiMessages.getNumEvents() > 0) 
	{
			
		int m = midiMessages.getNumEvents();
		std::cout << m << "events received by node " << getNodeId() << std::endl;

		MidiBuffer::Iterator i (midiMessages);
		MidiMessage message(0xf4);

		int samplePosition;
		i.setNextSamplePosition(samplePosition);

		while (i.getNextEvent (message, samplePosition)) {
			
				int numbytes = message.getRawDataSize();
				uint8* dataptr = message.getRawData();

				std::cout << " Bytes received: " << numbytes << std::endl;
				std::cout << " Message timestamp = " << message.getTimeStamp() << std::endl;

				int value = (*dataptr<<8) + *(dataptr+1);
				std::cout << "   " << value << std::endl;
		}

	}
}

void GenericProcessor::addMidiEvent(MidiBuffer& midiMessages, int numberToAdd)
{
	uint8 data[2];

	data[0] = numberToAdd >> 8; 	// most-significant byte
    data[1] = numberToAdd & 0xFF; 	// least-significant byte

    midiMessages.addEvent(data, 		// spike data
                          sizeof(data), // total bytes
                          0);           // sample index
}

void GenericProcessor::processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages)
{
	
	int nSamples = getNumSamples(midiMessages); // removes first value from midimessages

	process(buffer, midiMessages, nSamples);

	setNumSamples(midiMessages, nSamples); // adds it back,
										   // even if it's unchanged

}
