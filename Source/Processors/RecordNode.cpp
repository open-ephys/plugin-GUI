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

bool RecordNode::enable()
{
	// open files, creating them if necessary

	return true;
}


bool RecordNode::disable() 
{	
	
	// close files

	return true;
}

float RecordNode::getFreeSpace()
{
	// this needs to be updated:

	return 0.5;
	//return (1.0f-float(outputFile.getBytesFreeOnVolume())/float(outputFile.getVolumeTotalSize()));
}

void RecordNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Record node processing block." << std::endl;
	//std::cout << "Num channels: " << buffer.getNumChannels() << std::endl;

	if (isRecording) {

		// cycle through buffer channels, saving them to the appropriate places


	}

}
