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
	: GenericProcessor("Record Node"), isRecording(false), isProcessing(false)
{

	
//	newDataFolder = true; // defaults to creating a new data folder on startup
	continuousDataBuffer = new int16[10000];
	signalFilesShouldClose = false;

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

void RecordNode::setChannelStatus(int chan, bool status)
{

	std::cout << "Setting channel status!" << std::endl;
	setCurrentChannel(chan);

	if (status)
		setParameter(2, 1.0f);
	else
		setParameter(2, 0.0f);

}

// void RecordNode::enableCurrentChannel(bool state)
// {
// 	continuousChannels[nextAvailableChannel].isRecording = state;
// }

void RecordNode::resetConnections()
{
	//std::cout << "Resetting connections" << std::endl;
	nextAvailableChannel = 0;
	wasConnected = false;

	continuousChannels.clear();
	eventChannels.clear();
}

void RecordNode::filenameComponentChanged(FilenameComponent* fnc)
{

	std::cout << "Got a new file" << std::endl;
	dataDirectory = fnc->getCurrentFile();
	std::cout << "File name: " << dataDirectory.getFullPathName();
	if (dataDirectory.isDirectory())
		std::cout << " is a directory." << std::endl;
	else
		std::cout << " is NOT a directory." << std::endl;

	createNewDirectory();

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

		String filename = rootFolder.getFullPathName();
		filename += "/";
		filename += newChannel.nodeId;
		filename += "_";
		filename += newChannel.name;
		filename += ".continuous";

		newChannel.filename = filename;
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

void RecordNode::createNewDirectory()
{
	std::cout << "Creating new directory." << std::endl;

	Time calendar = Time::getCurrentTime();

	Array<int> t;
	t.add(calendar.getYear()-2000);
	t.add(calendar.getMonth()+1); // January = 0
	t.add(calendar.getDayOfMonth());
	t.add(calendar.getHours());
	t.add(calendar.getMinutes());
	t.add(calendar.getSeconds());

	String filename = "";
	
	for (int n = 0; n < t.size(); n++)
	{
		if (t[n] < 10)
			filename += "0";

		filename += t[n];

		if (n == 2)
			filename += "_";
		else if (n < 5)
			filename += "-";
	}

	rootFolder = File(dataDirectory.getFullPathName() + File::separator + filename);

}


void RecordNode::setParameter (int parameterIndex, float newValue)
{

	// 0 = stop recording
	// 1 = start recording
	// 2 = toggle individual channel (0.0f = OFF, anything else = ON)

 	if (parameterIndex == 1) {

		isRecording = true;
		std::cout << "START RECORDING." << std::endl;

 		if (!rootFolder.exists())
 			rootFolder.createDirectory();

		// create / open necessary files
		for (int i = 0; i < continuousChannels.size(); i++)
		{
			if (continuousChannels[i].isRecording)
			{
				std::cout << "OPENING FILE: " << continuousChannels[i].filename << std::endl;
				continuousChannels[i].file = fopen(continuousChannels[i].filename.toUTF8(), "a");
			}
		}
 		

 	} else if (parameterIndex == 0) {

		
		std::cout << "STOP RECORDING." << std::endl;

		if (isRecording) {
			// close necessary files

			signalFilesShouldClose = true;

			
		}

		isRecording = false;

 		
 	} else if (parameterIndex == 2) {

		if (isProcessing) {

			std::cout << "Toggling channel " << currentChannel << std::endl;

	 		if (newValue == 0.0f) {
	 			continuousChannels[currentChannel].isRecording = false;

	 			if (isRecording) {
	 				std::cout << "CLOSING FILE: " << continuousChannels[currentChannel].filename << std::endl;
	 				fclose(continuousChannels[currentChannel].file);
	 			}

	 		}
	 		else {
	 			continuousChannels[currentChannel].isRecording = true;

	 			if (isRecording) {
	 				std::cout << "OPENING FILE: " << continuousChannels[currentChannel].filename << std::endl;
	 				continuousChannels[currentChannel].file = 
	 					fopen(continuousChannels[currentChannel].filename.toUTF8(), "a");
	 			}
	 		}
		}
 	}
}

void RecordNode::closeAllFiles()
{

	for (int i = 0; i < continuousChannels.size(); i++)
	{
		if (continuousChannels[i].isRecording)
		{
			std::cout << "CLOSING FILE: " << continuousChannels[i].filename << std::endl;
			fclose(continuousChannels[i].file);
		}
	}
}

bool RecordNode::enable()
{
	isProcessing = true;
	return true;
}


bool RecordNode::disable() 
{	
	// close files if necessary
	setParameter(0, 10.0f);

	isProcessing = false;

	return true;
}

float RecordNode::getFreeSpace()
{
	return 1.0f - float(dataDirectory.getBytesFreeOnVolume())/float(dataDirectory.getVolumeTotalSize());
}

void RecordNode::writeContinuousBuffer(float* data, int nSamples, int channel)
{

	// find file and write samples to disk

	AudioDataConverters::convertFloatToInt16BE(data, continuousDataBuffer, nSamples);

	//int16 samps = nSamples;

	fwrite(&timestamp,							// ptr
			   8,   							// size of each element
			   1, 		  						// count 
			   continuousChannels[channel].file);   // ptr to FILE object

	fwrite(&nSamples,								// ptr
			   sizeof(nSamples),   				// size of each element
			   1, 		  						// count 
			   continuousChannels[channel].file);   // ptr to FILE object

	int n = fwrite(continuousDataBuffer,			// ptr
			   2,			     					// size of each element
			   nSamples, 		  					// count 
			   continuousChannels[channel].file);   // ptr to FILE object
	// n must equal "count", otherwise there was an error
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

	timestamp = timer.getHighResolutionTicks();

	if (isRecording) {

		// WHY IS THIS AFFECTING THE LFP DISPLAY?
		//buffer.applyGain(0, nSamples, 0.5);

		// cycle through events -- extract the samples per channel
		// NOT YET IMPLEMENTED

		// cycle through buffer channels
		for (int i = 0; i < buffer.getNumChannels(); i++)
		{

			if (continuousChannels[i].isRecording)
			{
				// write buffer to disk!
				writeContinuousBuffer(buffer.getSampleData(i),
									  nSamples,
									  i);
				
				//std::cout << "Record channel " << i << std::endl;
			}
				

		}

		return;

	}

	if (signalFilesShouldClose)
	{
		// prevent parameter changes from closing files
		// before recording stops
		closeAllFiles();
		signalFilesShouldClose = false;
	}

}
