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
