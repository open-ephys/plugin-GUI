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
#include "ProcessorGraph.h"

RecordNode::RecordNode()
	: GenericProcessor("Record Node"), isRecording(false)
{

	dataFolder = "./Data";

	// need to update this:
	setPlayConfigDetails(2,0,44100.0,128);
	


}


RecordNode::~RecordNode() {

}

void RecordNode::setChannel(int id, int chan)
{

	std::cout << "Record node setting channel." << std::endl;

	for (int i = 0; i < continuousChannels.size(); i++)
	{

		if (continuousChannels[i].nodeId == id &&
			continuousChannels[i].chan == chan)
		{
			std::cout << "Found channel " << i << std::endl;
			setCurrentChannel(i);
			break;
		}

	}
}

void RecordNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{

	if (chan != getProcessorGraph()->midiChannelIndex)
	{
		Channel newChannel;

		std::cout << "Record node adding channel." << std::endl;

		newChannel.nodeId = sourceNode->getNodeId();
		newChannel.chan = chan;
		newChannel.name = sourceNode->getOutputChannelName(chan);
		newChannel.isRecording = sourceNode->recordStatus(chan);
		newChannel.file = 0;

		if (newChannel.isRecording)
			std::cout << "  This channel will be recorded." << std::endl;
		else 
			std::cout << "  This channel will NOT be recorded." << std::endl;
	
		std::cout << "adding channel " << getNextChannel(false) << std::endl;

		std::pair<int, Channel> newPair (getNextChannel(false), newChannel);

		continuousChannels.insert(newPair);

		setPlayConfigDetails(getNextChannel(false)+1,0,44100.0,128);

	} else {


		std::map<int, Channel> eventChans;

		int ID = sourceNode->getNodeId();

		for (int n = 0; n < sourceNode->settings.eventChannelIds.size(); n++)
		{

			Channel newChannel;

			newChannel.nodeId = ID;
			newChannel.chan = sourceNode->settings.eventChannelIds[n];
			newChannel.name = sourceNode->settings.eventChannelNames[n];
			newChannel.isRecording = true;
			newChannel.file = 0;

			std::pair<int, Channel> newPair (newChannel.chan, newChannel);

			eventChans.insert(newPair);

		}

		std::pair<int, std::map<int, Channel> > newPair (ID, eventChans);

		eventChannels.insert(newPair);

	}

}


void RecordNode::setParameter (int parameterIndex, float newValue)
{
 	if (parameterIndex == 1) {
 		isRecording = true;
 		// create necessary files

 	} else if (parameterIndex == 0) {
 		isRecording = false;
 		// close necessary files
 	} else if (parameterIndex == 2) {

 		if (isRecording) {

 			std::cout << "Toggling channel " << currentChannel << std::endl;

	 		if (newValue == 0.0f)
	 			continuousChannels[currentChannel].isRecording = false;
	 		else
	 			continuousChannels[currentChannel].isRecording = true;
 		}
 	}
}

bool RecordNode::enable()
{
	// figure out the folder structure

	// File dir = File(dataFolder);

	// if (!dir.exists())
	// 	dir.createDirectory();

	//FILE* pFile;
   // pFile = fopen("Test.bin", "wb");

	return true;
}


bool RecordNode::disable() 
{	
	
	// close files if necessary

	return true;
}

float RecordNode::getFreeSpace()
{
	// this needs to be updated:

	return 0.5;
	//return (1.0f-float(outputFile.getBytesFreeOnVolume())/float(outputFile.getVolumeTotalSize()));
}

void RecordNode::writeContinuousBuffer(float* data, int nSamples, int channel)
{

	// find file and write samples to disk
}
 
void RecordNode::writeEventBuffer(MidiMessage& event, int node, int channel)
{
	// find file and write samples to disk

}

void RecordNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	//std::cout << "Record node processing block." << std::endl;
	//std::cout << "Num channels: " << buffer.getNumChannels() << std::endl;

	if (isRecording) {

		// cycle through events -- extract the samples per channel


		// cycle through buffer channels
		for (int i = 0; i < buffer.getNumChannels(); i++)
		{

			//std::cout << "CH" << i << " " << continuousChannels[i].isRecording << std::endl;

			if (continuousChannels[i].isRecording)
			{
				writeContinuousBuffer(buffer.getSampleData(i),
									  nSamples,
									  i);
				// write buffer to disk!
				//std::cout << "Record channel " << i << std::endl;
			}
				

		}

		//int n = fwrite(vector, // ptr
		//	   1,      		   // size of each element
		//	   sizeof(vector), // count 
		//	   pFile);         /// ptr to FILE object
		// n must equal "count", otherwise there was an error

		// cycle through buffer channels, saving them to the appropriate places


	}

}
