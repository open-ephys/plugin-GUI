/*
  ==============================================================================

    RecordNode.cpp
    Created: 10 May 2011 7:17:09pm
    Author:  jsiegle

  ==============================================================================
*/

#include "RecordNode.h"

RecordNode::RecordNode()
	: GenericProcessor("Record Node"), isRecording(false)
{

	// need to update this:
	setPlayConfigDetails(64,0,44100.0,128);

	outputFile = File("./data"); // create output file
	outputStream = 0;
	
}


RecordNode::~RecordNode() {

}


void RecordNode::setParameter (int parameterIndex, float newValue)
{
 	if (parameterIndex == 1) {
 		isRecording = true;
 	} else {
 		isRecording = false;
 	}
}


void RecordNode::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	
	outputStream = outputFile.createOutputStream();
}

void RecordNode::releaseResources() 
{	
	
	if (outputStream != 0) {
		outputStream->flush();

		delete(outputStream);
		outputStream = 0;
	}
}

float RecordNode::getFreeSpace()
{
	return (1.0f-float(outputFile.getBytesFreeOnVolume())/float(outputFile.getVolumeTotalSize()));
}

void RecordNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Record node processing block." << std::endl;
	//std::cout << "Num channels: " << buffer.getNumChannels() << std::endl;

	if (isRecording) {

		//int nSamps = getNumSamples(midiMessages);

		for (int n = 0; n < nSamples; n++) {
		
			float* sample = buffer.getSampleData(1,n);
			outputStream->writeFloat(*sample);
			//AudioDataConverters::convertFloatToInt16BE(&sample)
			//);
		}
	}

}
