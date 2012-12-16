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

#include "Channel.h"

RecordNode::RecordNode()
	: GenericProcessor("Record Node"), isRecording(false), isProcessing(false),
		timestamp(0), signalFilesShouldClose(false)
{

	
	continuousDataIntegerBuffer = new int16[10000];
	continuousDataFloatBuffer = new float[10000];
	signalFilesShouldClose = false;

	settings.numInputs = 128;
	settings.numOutputs = 0;

	eventChannel = new Channel(this, 0);
	eventChannel->isEventChannel = true;

	recordMarker = new char[10];
	for (int i = 0; i < 9; i++)
	{
		recordMarker[i] = 0;
	}
	recordMarker[9] = 255;

	// 128 inputs, 0 outputs
	setPlayConfigDetails(getNumInputs(),getNumOutputs(),44100.0,128);

}


RecordNode::~RecordNode() {

}

void RecordNode::setChannel(Channel* ch)
{

	int channelNum = channelPointers.indexOf(ch);

	std::cout << "Record node setting channel to " << channelNum << std::endl;

	setCurrentChannel(channelNum);

	// for (int i = 0; i < con.size(); i++)
	// {

	// 	if (continuousChannels[i].nodeId == id &&
	// 		continuousChannels[i].chan == chan)
	// 	{
	// 		std::cout << "Found channel " << i << std::endl;
	// 		setCurrentChannel(i);
	// 		break;
	// 	}

	// }
}

void RecordNode::setChannelStatus(Channel* ch, bool status)
{

	//std::cout << "Setting channel status!" << std::endl;
	setChannel(ch);

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

	channelPointers.clear();
	eventChannelPointers.clear();

	

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
        
		int channelIndex = getNextChannel(false);

        setPlayConfigDetails(channelIndex+1,0,44100.0,128);

        channelPointers.add(sourceNode->channels[chan]);

     //   std::cout << channelIndex << std::endl;

        updateFileName(channelPointers[channelIndex]);

        

		//if (channelPointers[channelIndex]->isRecording)
		//	std::cout << "  This channel will be recorded." << std::endl;
		//else 
		//	std::cout << "  This channel will NOT be recorded." << std::endl;
	
		//std::cout << "adding channel " << getNextChannel(false) << std::endl;

		//std::pair<int, Channel> newPair (getNextChannel(false), newChannel);

		//std::cout << "adding channel " << getNextChannel(false) << std::endl;

		//continuouschannelPointers.insert(newPair);

		
	} else {

		for (int n = 0; n < sourceNode->eventChannels.size(); n++)
		{

			eventChannelPointers.add(sourceNode->eventChannels[n]);

		}

	}

}

void RecordNode::updateFileName(Channel* ch)
{
	String filename = rootFolder.getFullPathName();
	filename += rootFolder.separatorString;

	if (!ch->isEventChannel)
	{
		filename += ch->nodeId;
		filename += "_";
		filename += ch->name;
		filename += ".continuous";
	} else {
		filename += "all_channels.events";
	}
	
    ch->filename = filename;
    ch->file = 0;

    //std::cout << "Updating " << filename << std::endl;

}

void RecordNode::createNewDirectory()
{
	std::cout << "Creating new directory." << std::endl;

	rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName());

	updateFileName(eventChannel);

	for (int i = 0; i < channelPointers.size(); i++)
	{
		updateFileName(channelPointers[i]);
	}

}

String RecordNode::generateDirectoryName()
{
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

	return filename;

}

String RecordNode::generateDateString()
{
	Time calendar = Time::getCurrentTime();

	String datestring;

	datestring += String(calendar.getDayOfMonth());
	datestring += "-";
	datestring += calendar.getMonthName(true);
	datestring += "-";
	datestring += String(calendar.getYear());
	datestring += " ";
	datestring += calendar.getHours();
	datestring += ":";
	datestring += calendar.getMinutes();
	datestring += ":";
	datestring += calendar.getSeconds();
	
	return datestring;

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

 		openFile(eventChannel);

		// create / open necessary files
		for (int i = 0; i < channelPointers.size(); i++)
		{
			if (channelPointers[i]->isRecording)
			{
				openFile(channelPointers[i]);
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
	 			channelPointers[currentChannel]->isRecording = false;

	 			if (isRecording) {
	 				closeFile(channelPointers[currentChannel]);
	 			}

	 		}
	 		else {
	 			channelPointers[currentChannel]->isRecording = true;

	 			if (isRecording) {

	 				openFile(channelPointers[currentChannel]);
	 			
	 			}
	 		}
		}
 	}
}

void RecordNode::openFile(Channel* ch)
{
	std::cout << "OPENING FILE: " << ch->filename << std::endl;

	File f = File(ch->filename);

	bool fileExists = f.exists();

	ch->file = fopen(ch->filename.toUTF8(), "ab");

	if (!fileExists)
	{
		// create and write header
		std::cout << "Writing header." << std::endl;
		String header = generateHeader(ch);
		fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), ch->file);
	} else {
		std::cout << "File already exists, just opening." << std::endl;
	}
}

void RecordNode::closeFile(Channel* ch)
{
	std::cout << "CLOSING FILE: " << ch->filename << std::endl;
	fclose(ch->file);
}

String RecordNode::generateHeader(Channel* ch)
{

	String header = "header.format = 'OPEN EPHYS DATA FORMAT v0.0'; \n";

	header += "header.header_bytes = ";
	header += String(HEADER_SIZE);
	header += ";\n";

	if (ch->isEventChannel)
	{
		header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample position, one uint8 event type, one uint8 processor ID, one uint8 event ID, and one uint8 event channel'; \n";

	} else {
		header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample count (N), N 16-bit samples, and one 10-byte record marker (0 0 0 0 0 0 0 0 0 255)'; \n";
	}


	header += "header.date_created = '";
	header += generateDateString();
	header += "';\n";

	header += "header.channel = '";
	header += ch->name;
	header += "';\n";

	if (ch->isEventChannel)
	{

		header += "header.channelType = 'Event';\n";

	} else {

		header += "header.channelType = 'Continuous';\n";

		header += "header.sampleRate = ";
		header += String(ch->sampleRate);
		header += ";\n";
	}

	header += "header.bitVolts = ";
	header += String(ch->bitVolts);
	header += ";\n";

	header = header.paddedRight(' ', HEADER_SIZE);

	//std::cout << header << std::endl;

	return header;

}

void RecordNode::closeAllFiles()
{

	for (int i = 0; i < channelPointers.size(); i++)
	{
		if (channelPointers[i]->isRecording)
		{
			closeFile(channelPointers[i]);
		}
	}

	closeFile(eventChannel);
}

bool RecordNode::enable()
{

	//updateFileName(eventChannel);

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

	// scale the data appropriately -- currently just getting it into the right
	// range; actually need to take into account the gain of each channel
	for (int n = 0; n < nSamples; n++)
	{
		*(continuousDataFloatBuffer+n) = *(data+n) / 10000.0f; 
	}

	// find file and write samples to disk

	//if (nSamples < 1000) // this is temporary, but there seems to be an error reading in the data if too many samples are written
					     // in the first few blocks
	//{

		AudioDataConverters::convertFloatToInt16BE(continuousDataFloatBuffer, continuousDataIntegerBuffer, nSamples);

		int16 samps = (int16) nSamples;

		//std::cout << samps << std::endl;

		fwrite(&timestamp,							// ptr
				   8,   							// size of each element
				   1, 		  						// count 
				   channelPointers[channel]->file);   // ptr to FILE object

		fwrite(&samps,								// ptr
				   2,   							// size of each element
				   1, 		  						// count 
				   channelPointers[channel]->file);   // ptr to FILE object

		int n = fwrite(continuousDataIntegerBuffer,		// ptr
				   2,			     					// size of each element
				   nSamples, 		  					// count 
				   channelPointers[channel]->file);   // ptr to FILE object
		// n must equal "count", otherwise there was an error

		// write a 10-byte marker indicating the end of a record
		fwrite(recordMarker,		// ptr
				1,			     					// size of each element
				10, 		  					// count 
				channelPointers[channel]->file);   // ptr to FILE object



	//}
}
 
void RecordNode::writeEventBuffer(MidiMessage& event, int samplePosition) //, int node, int channel)
{
	// find file and write samples to disk
	//std::cout << "Received event!" << std::endl;

	uint8* dataptr = event.getRawData();
	int16 samplePos = (int16) samplePosition;

	// write timestamp (for buffer only, not the actual event timestamp!!!!!)
	fwrite(&timestamp,							// ptr
			   8,   							// size of each element
			   1, 		  						// count 
			   eventChannel->file);   			// ptr to FILE object

	fwrite(&samplePos,							// ptr
			   2,   							// size of each element
			   1, 		  						// count 
			   eventChannel->file);   			// ptr to FILE object

	// write 1st four bytes of event (type, nodeId, eventId, eventChannel)
	fwrite(dataptr, 1, 4, eventChannel->file);

}

void RecordNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
	if (eventType == TTL)
	{
		writeEventBuffer(event, samplePosition);
	}

}

void RecordNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{

	//std::cout << "Record node processing block." << std::endl;
	//std::cout << "Num channels: " << buffer.getNumChannels() << std::endl;


	if (isRecording) {

		timestamp = timer.getHighResolutionTicks();

		// WHY IS THIS AFFECTING THE LFP DISPLAY?
		//buffer.applyGain(0, nSamples, 5.2438f);

		// cycle through events -- extract the samples per channel
		// NOT YET IMPLEMENTED

		// cycle through buffer channels
		for (int i = 0; i < buffer.getNumChannels(); i++)
		{

			if (channelPointers[i]->isRecording)
			{
				// write buffer to disk!
				writeContinuousBuffer(buffer.getSampleData(i),
									  nSamples,
									  i);
				
				//std::cout << "Record channel " << i << std::endl;
			}
				

		}

		// cycle through events
		checkForEvents(events);

		return;

	}

	// this is intended to prevent parameter changes from closing files
	// before recording stops
	if (signalFilesShouldClose)
	{
		closeAllFiles();
		signalFilesShouldClose = false;
	}

}
