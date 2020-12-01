/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "OriginalRecording.h"
//#include "../../AccessClass.h"
//#include "../../Audio/AudioComponent.h"

OriginalRecording::OriginalRecording() : separateFiles(false),
recordingNumber(0), experimentNumber(0), zeroBuffer(1, 50000),
eventFile(nullptr), messageFile(nullptr), lastProcId(0), procIndex(0)
{
	/*continuousDataIntegerBuffer = new int16[10000];
	continuousDataFloatBuffer = new float[10000];

	recordMarker = new char[10];*/
	continuousDataIntegerBuffer.malloc(10000);
	continuousDataFloatBuffer.malloc(10000);
	recordMarker.malloc(10);

	for (int i = 0; i < 9; i++)
	{
		recordMarker[i] = i;
	}
	recordMarker[9] = 255;

	zeroBuffer.clear();
}

OriginalRecording::~OriginalRecording()
{
	//Cleanup just in case
	for (int i = 0; i < fileArray.size(); i++)
	{
		if (fileArray[i] != nullptr) fclose(fileArray[i]);
	}
	for (int i = 0; i < spikeFileArray.size(); i++)
	{
		if (spikeFileArray[i] != nullptr) fclose(spikeFileArray[i]);
	}
	/*   delete continuousDataFloatBuffer;
	delete continuousDataIntegerBuffer;
	delete recordMarker;*/
}

String OriginalRecording::getEngineID() const
{
	return "OPENEPHYS";
}

void OriginalRecording::addSpikeElectrode(int index, const SpikeChannel* elec)
{
	//spikeFileArray.add(nullptr); // deprecated
}

void OriginalRecording::resetChannels()
{
	fileArray.clear();
	spikeFileArray.clear();
	blockIndex.clear();
	processorArray.clear();
	samplesSinceLastTimestamp.clear();
	originalChannelIndexes.clear();
	procIndex = 0;
}

void OriginalRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
	this->recordingNumber = recordingNumber;
	this->experimentNumber = experimentNumber;

	processorArray.clear();
	lastProcId = 0;

	openFile(rootFolder, getEventChannel(0), 0);

	openMessageFile(rootFolder);

	int nChannels = getNumRecordedChannels();

	for (int i = 0; i < nChannels; i++)
	{
		const DataChannel* ch = getDataChannel(getRealChannel(i));
		openFile(rootFolder, ch, getRealChannel(i));
		blockIndex.add(0);
		samplesSinceLastTimestamp.add(0);
	}

	int nSpikes = getNumRecordedSpikes();

	for (int i = 0; i < nSpikes; i++)
	{
		spikeFileArray.add(nullptr);
		openSpikeFile(rootFolder, getSpikeChannel(i), i);
	}

}

void OriginalRecording::openFile(File rootFolder, const InfoObjectCommon* ch, int channelIndex)
{
	FILE* chFile;
	bool isEvent;
	String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);
	String fileName;

	recordPath = fullPath;

	isEvent = (ch->getInfoObjectType() == InfoObjectCommon::EVENT_CHANNEL) ? true : false;
	if (isEvent)
	{
		if (experimentNumber > 1)
			fileName += "all_channels_" + String(experimentNumber) + ".events";
		else
			fileName += "all_channels.events";
	}
	else
	{
		fileName += getFileName(channelIndex);
	}

	fullPath += fileName;
	LOGD("OPENING FILE: ", fullPath);

	File f = File(fullPath);

	bool fileExists = f.exists();

	diskWriteLock.enter();

	chFile = fopen(fullPath.toUTF8(), "ab");

	if (!fileExists)
	{
		// create and write header
		LOGD("Writing header.");
		String header = generateHeader(ch);
		LOGDD(header);
		LOGD("File ID: ", chFile, ", number of bytes: ", header.getNumBytesAsUTF8());


		fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), chFile);

		LOGD("Wrote header.");

		LOGDD("Block index: ", blockIndex);

	}
	else
	{
		LOGD("File already exists, just opening.");
		fseek(chFile, 0, SEEK_END);
	}

	if (isEvent)
		eventFile = chFile;
	else
	{
		fileArray.add(chFile);
		if (ch->getCurrentNodeID() != lastProcId)
		{
			lastProcId = ch->getCurrentNodeID();
			ProcInfo* p = new ProcInfo();
			p->id = ch->getCurrentNodeID();
			p->sampleRate = ch->getSampleRate();
			processorArray.add(p);
		}
		ChannelInfo* c = new ChannelInfo();
		c->filename = fileName;
		c->name = ch->getName();
		c->startPos = ftell(chFile);
		c->bitVolts = dynamic_cast<const DataChannel*>(ch)->getBitVolts();
		processorArray.getLast()->channels.add(c);
	}
	diskWriteLock.exit();

}

void OriginalRecording::openSpikeFile(File rootFolder, const SpikeChannel* elec, int channelIndex)
{

	FILE* spFile;
	String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);
	fullPath += elec->getName().removeCharacters(" ");

	if (experimentNumber > 1)
	{
		fullPath += "_" + String(experimentNumber);
	}

	fullPath += ".spikes";

	LOGD("OPENING FILE: ", fullPath);

	File f = File(fullPath);

	bool fileExists = f.exists();

	diskWriteLock.enter();

	spFile = fopen(fullPath.toUTF8(), "ab");

	if (!fileExists)
	{

		String header = generateSpikeHeader(elec);
		fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), spFile);
		LOGD("Wrote header.");
	}
	diskWriteLock.exit();
	spikeFileArray.set(channelIndex, spFile);
	LOGD("Added file.");

}

void OriginalRecording::openMessageFile(File rootFolder)
{
	FILE* mFile;
	String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);

	fullPath += "messages";

	if (experimentNumber > 1)
	{
		fullPath += "_" + String(experimentNumber);
	}

	fullPath += ".events";

	LOGD("OPENING FILE: ", fullPath);

	File f = File(fullPath);

	//bool fileExists = f.exists();

	diskWriteLock.enter();

	mFile = fopen(fullPath.toUTF8(), "ab");

	//If this file needs a header, it goes here

	diskWriteLock.exit();
	messageFile = mFile;

}

String OriginalRecording::getFileName(int channelIndex)
{
	String filename;
	const DataChannel* ch = getDataChannel(channelIndex);
    
	filename += String(static_cast<int>(ch->getSourceNodeID()));
	filename += "_";
	if (renameFiles)
		filename += renamedPrefix + String(getDataChannel(channelIndex)->getCurrentNodeChannelIdx() + 1);
	else
		filename += ch->getName();

	if (experimentNumber > 1)
	{
		filename += "_" + String(experimentNumber);
	}

	if (separateFiles)
	{
		filename += "_";
		filename += recordingNumber;
	}
	filename += ".continuous";

	return filename;
}

String OriginalRecording::generateHeader(const InfoObjectCommon* ch)
{

	String header = "header.format = 'Open Ephys Data Format'; \n";

	header += "header.version = " + String(VERSION_STRING) + "; \n";
	header += "header.header_bytes = ";
	header += String(HEADER_SIZE);
	header += ";\n";

	if (ch->getInfoObjectType() == InfoObjectCommon::EVENT_CHANNEL)
	{
		header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample position, one uint8 event type, one uint8 processor ID, one uint8 event ID, one uint8 event channel, and one uint16 recordingNumber'; \n";

	}
	else
	{
		header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample count (N), 1 uint16 recordingNumber, N 16-bit samples, and one 10-byte record marker (0 1 2 3 4 5 6 7 8 255)'; \n";
	}


	header += "header.date_created = '";
	header += generateDateString();
	header += "';\n";

	header += "header.channel = '";
	header += (ch != nullptr) ? ch->getName() : "Events";
	header += "';\n";

	if (ch == nullptr)
	{

		header += "header.channelType = 'Event';\n";
	}
	else
	{
		header += "header.channelType = 'Continuous';\n";
	}

	header += "header.sampleRate = ";
	if (ch == nullptr)
		header += String(getDataChannel(0)->getSampleRate());
	else
		header += String(ch->getSampleRate());
	header += ";\n";
	header += "header.blockLength = ";
	header += BLOCK_LENGTH;
	header += ";\n";
	//header += "header.bufferSize = ";
	//header += AccessClass::getAudioComponent()->getBufferSize();
	header += ";\n";
	header += "header.bitVolts = ";
	header += (ch->getInfoObjectType() == InfoObjectCommon::DATA_CHANNEL) ? String(dynamic_cast<const DataChannel*>(ch)->getBitVolts()) : "1";
	header += ";\n";

	header = header.paddedRight(' ', HEADER_SIZE);

LOGDD(header);

	return header;

}

String OriginalRecording::generateSpikeHeader(const SpikeChannel* elec)
{
	String header = "header.format = 'Open Ephys Data Format'; \n";
	header += "header.version = " + String(VERSION_STRING) + "; \n";
	header += "header.header_bytes = ";
	header += String(HEADER_SIZE);
	header += ";\n";

	header += "header.description = 'Each record contains 1 uint8 eventType, 1 int64 timestamp, 1 int64 software timestamp, "
		"1 uint16 sourceID, 1 uint16 numChannels (n), 1 uint16 numSamples (m), 1 uint16 sortedID, 1 uint16 electrodeID, "
		"1 uint16 channel, 3 uint8 color codes, 2 float32 component projections, n*m uint16 samples, n float32 channelGains, n uint16 thresholds, and 1 uint16 recordingNumber'; \n";

	header += "header.date_created = '";
	header += generateDateString();
	header += "';\n";

	header += "header.electrode = '";
	header += elec->getName();
	header += "';\n";

	header += "header.num_channels = ";
	header += String(elec->getNumChannels());
	header += ";\n";

	header += "header.sampleRate = ";
	header += String(elec->getSampleRate());
	header += ";\n";

	header = header.paddedRight(' ', HEADER_SIZE);

LOGDD(header);

	return header;
}

void OriginalRecording::writeEvent(int eventIndex, const MidiMessage& event)
{
	writeTTLEvent(eventIndex, event);
	if (Event::getEventType(event) == EventChannel::TEXT)
	{
		TextEventPtr ev = TextEvent::deserializeFromMessage(event, getEventChannel(eventIndex));
		if (ev == nullptr) return;
		writeMessage(ev->getText(), ev->getSourceID(), ev->getChannel(), ev->getTimestamp());
	}
}

void OriginalRecording::writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float, String text)
{
	writeMessage(text, sourceID, 255, timestamp);
}

void OriginalRecording::writeMessage(String message, uint16 processorID, uint16 channel, int64 timestamp)
{
	if (messageFile == nullptr)
		return;

	int msgLength = message.getNumBytesAsUTF8();

	String timestampText(timestamp);

	diskWriteLock.enter();
	fwrite(timestampText.toUTF8(), 1, timestampText.length(), messageFile);
	fwrite(" ", 1, 1, messageFile);
	fwrite(message.toUTF8(), 1, msgLength, messageFile);
	fwrite("\n", 1, 1, messageFile);
	diskWriteLock.exit();

}

void OriginalRecording::writeTTLEvent(int eventIndex, const MidiMessage& event)
{
	// find file and write samples to disk
	LOGDD("Received event!");

	if (eventFile == nullptr)
		return;

	uint8 data[16];
	//With the new external recording thread, this field has no sense.
	int16 samplePos = 0;

	EventPtr ev = Event::deserializeFromMessage(event, getEventChannel(eventIndex));
	if (!ev) return;
	*reinterpret_cast<int64*>(data) = ev->getTimestamp();
	*reinterpret_cast<int16*>(data + 8) = samplePos;
	*(data + 10) = static_cast<uint8>(ev->getEventType());
	*(data + 11) = static_cast<uint8>(ev->getSourceID());
	*(data + 12) = (ev->getEventType() == EventChannel::TTL) ? (dynamic_cast<TTLEvent*>(ev.get())->getState() ? 1 : 0) : 0;
	*(data + 13) = static_cast<uint8>(ev->getChannel());
	*reinterpret_cast<uint16*>(data + 14) = recordingNumber;


	diskWriteLock.enter();

	fwrite(&data,					// ptr
		sizeof(uint8),   							// size of each element
		16, 		  						// count
		eventFile);   			// ptr to FILE object

	diskWriteLock.exit();
}

void OriginalRecording::writeSynchronizedData(int writeChannel, int realChannel, const float* dataBuffer, const double* ftsBuffer, int size) {};

void OriginalRecording::writeData(int writeChannel, int realChannel, const float* buffer, int size)
{
	int samplesWritten = 0;

	samplesSinceLastTimestamp.set(writeChannel, 0);

	int nSamples = size;

	while (samplesWritten < nSamples) // there are still unwritten samples in this buffer
	{
		int numSamplesToWrite = nSamples - samplesWritten;

		if (blockIndex[writeChannel] + numSamplesToWrite < BLOCK_LENGTH) // we still have space in this block
		{

			// write buffer to disk!
			writeContinuousBuffer(buffer + samplesWritten,
				numSamplesToWrite,
				writeChannel);

			//timestamp += numSamplesToWrite;
			samplesSinceLastTimestamp.set(writeChannel, samplesSinceLastTimestamp[writeChannel] + numSamplesToWrite);
			blockIndex.set(writeChannel, blockIndex[writeChannel] + numSamplesToWrite);
			samplesWritten += numSamplesToWrite;

		}
		else   // there's not enough space left in this block for all remaining samples
		{

			numSamplesToWrite = BLOCK_LENGTH - blockIndex[writeChannel];

			// write buffer to disk!
			writeContinuousBuffer(buffer + samplesWritten,
				numSamplesToWrite,
				writeChannel);

			// update our variables
			samplesWritten += numSamplesToWrite;
			//timestamp += numSamplesToWrite;
			samplesSinceLastTimestamp.set(writeChannel, samplesSinceLastTimestamp[writeChannel] + numSamplesToWrite);
			blockIndex.set(writeChannel, 0); // back to the beginning of the block
		}
	}


}

void OriginalRecording::writeContinuousBuffer(const float* data, int nSamples, int writeChannel)
{
	// check to see if the file exists
	if (fileArray[writeChannel] == nullptr)
		return;

	// scale the data back into the range of int16
	float scaleFactor = float(0x7fff) * getDataChannel(getRealChannel(writeChannel))->getBitVolts();

	for (int n = 0; n < nSamples; n++)
	{
		*(continuousDataFloatBuffer + n) = *(data + n) / scaleFactor;
	}
	AudioDataConverters::convertFloatToInt16BE(continuousDataFloatBuffer, continuousDataIntegerBuffer, nSamples);

	if (blockIndex[writeChannel] == 0)
	{
		writeTimestampAndSampleCount(fileArray[writeChannel], writeChannel);
	}

	diskWriteLock.enter();

	size_t count = fwrite(continuousDataIntegerBuffer,      // ptr
		2,                                // size of each element
		nSamples,                         // count
		fileArray[writeChannel]); // ptr to FILE object

	LOGDD(writeChannel, " : ", nSamples, " : ", count);

	jassert(count == nSamples); // make sure all the data was written
	(void)count;  // Suppress unused variable warning in release builds

	diskWriteLock.exit();

	if (blockIndex[writeChannel] + nSamples == BLOCK_LENGTH)
	{
		writeRecordMarker(fileArray[writeChannel]);
	}
}

void OriginalRecording::writeTimestampAndSampleCount(FILE* file, int channel)
{
	diskWriteLock.enter();

	uint16 samps = BLOCK_LENGTH;

	// int sourceNodeId = getChannel(channel)->sourceNodeId;

	int64 ts = getTimestamp(channel) + samplesSinceLastTimestamp[channel];

	fwrite(&ts,                       // ptr
		8,                               // size of each element
		1,                               // count
		file); // ptr to FILE object

	fwrite(&samps,                           // ptr
		2,                               // size of each element
		1,                               // count
		file); // ptr to FILE object

	fwrite(&recordingNumber,                         // ptr
		2,                               // size of each element
		1,                               // count
		file); // ptr to FILE object

	diskWriteLock.exit();
}

void OriginalRecording::writeRecordMarker(FILE* file)
{
	// write a 10-byte marker indicating the end of a record

	diskWriteLock.enter();
	fwrite(recordMarker,        // ptr
		1,                   // size of each element
		10,                  // count
		file);               // ptr to FILE object

	diskWriteLock.exit();
}

void OriginalRecording::closeFiles()
{
	for (int i = 0; i < fileArray.size(); i++)
	{
		if (fileArray[i] != nullptr)
		{
			if (blockIndex[i] < BLOCK_LENGTH)
			{
				// fill out the rest of the current buffer
				writeContinuousBuffer(zeroBuffer.getReadPointer(0), BLOCK_LENGTH - blockIndex[i], i);
				diskWriteLock.enter();
				fclose(fileArray[i]);
				diskWriteLock.exit();
			}
		}
	}
	fileArray.clear();

	blockIndex.clear();
	samplesSinceLastTimestamp.clear();
	for (int i = 0; i < spikeFileArray.size(); i++)
	{
		if (spikeFileArray[i] != nullptr)
		{
			diskWriteLock.enter();
			fclose(spikeFileArray[i]);
			spikeFileArray.set(i, nullptr);
			diskWriteLock.exit();
		}
	}
	if (eventFile != nullptr)
	{
		diskWriteLock.enter();
		fclose(eventFile);
		eventFile = nullptr;
		diskWriteLock.exit();
	}
	if (messageFile != nullptr)
	{
		diskWriteLock.enter();
		fclose(messageFile);
		messageFile = nullptr;
		diskWriteLock.exit();
	}

	writeXml();

}

// void OriginalRecording::updateTimeStamp(int64 timestamp)
// {
//     this->timestamp = timestamp;
// }

void OriginalRecording::writeSpike(int electrodeIndex, const SpikeEvent* spike)
{
	LOGDD("Electrode index: ", electrodeIndex);

	if (spikeFileArray[electrodeIndex] == nullptr)
		return;

	LOGDD("Got spike");

	HeapBlock<char> spikeBuffer;
	const SpikeChannel* channel = getSpikeChannel(electrodeIndex);

	LOGDD("Got spike channel");

	int totalSamples = channel->getTotalSamples() * channel->getNumChannels();
	int numChannels = channel->getNumChannels();
	int chanSamples = channel->getTotalSamples();

	int totalBytes = totalSamples * 2 + // account for samples
		numChannels * 4 +            // acount for gain
		numChannels * 2 +            // account for thresholds
		42;             // 42, from SpikeObject.h
	spikeBuffer.malloc(totalBytes);
	*(spikeBuffer.getData()) = static_cast<char>(channel->getChannelType());
	*reinterpret_cast<int64*>(spikeBuffer.getData() + 1) = spike->getTimestamp();
	*reinterpret_cast<int64*>(spikeBuffer.getData() + 9) = 0; //Legacy unused value
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 17) = spike->getSourceID();
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 19) = numChannels;
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 21) = chanSamples;
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 23) = spike->getSortedID();
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 25) = electrodeIndex; //Legacy value
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 27) = 0; //Legacy unused value
	zeromem(spikeBuffer.getData() + 29, 3 * sizeof(uint8));
	zeromem(spikeBuffer.getData() + 32, 2 * sizeof(float));
	*reinterpret_cast<uint16*>(spikeBuffer.getData() + 40) = channel->getSampleRate();

	LOGDD("Allocated memory");

	int ptrIdx = 0;
	uint16* dataIntPtr = reinterpret_cast<uint16*>(spikeBuffer.getData() + 42);
	const float* spikeDataPtr = spike->getDataPointer();
	for (int i = 0; i < numChannels; i++)
	{
		const float bitVolts = channel->getChannelBitVolts(i);
		for (int j = 0; j < chanSamples; j++)
		{
			*(dataIntPtr + ptrIdx) = uint16(*(spikeDataPtr + ptrIdx) / bitVolts + 32768);
			ptrIdx++;
		}
	}
	ptrIdx = totalSamples * 2 + 42;
	for (int i = 0; i < numChannels; i++)
	{
		//To get the same value as the original version
		*reinterpret_cast<float*>(spikeBuffer.getData() + ptrIdx) = (int)(1.0f / channel->getChannelBitVolts(i)) * 1000;
		ptrIdx += sizeof(float);
	}
	for (int i = 0; i < numChannels; i++)
	{
		*reinterpret_cast<int16*>(spikeBuffer.getData() + ptrIdx) = spike->getThreshold(i);
		ptrIdx += sizeof(int16);
	}

	LOGDD("Starting disk write");

	diskWriteLock.enter();

	fwrite(spikeBuffer, 1, totalBytes, spikeFileArray[electrodeIndex]);

	fwrite(&recordingNumber,                         // ptr
		2,                               // size of each element
		1,                               // count
		spikeFileArray[electrodeIndex]); // ptr to FILE object

	diskWriteLock.exit();

	LOGDD("Wrote to file");
}

void OriginalRecording::writeXml()
{
	String name = recordPath + "Continuous_Data";
	if (experimentNumber > 1)
	{
		name += "_";
		name += String(experimentNumber);
	}
	name += ".openephys";

	File file(name);
	XmlDocument doc(file);
	ScopedPointer<XmlElement> xml = doc.getDocumentElement();
	if (!xml || !xml->hasTagName("EXPERIMENT"))
	{
		xml = new XmlElement("EXPERIMENT");
		xml->setAttribute("version", VERSION);
		xml->setAttribute("number", experimentNumber);
		xml->setAttribute("separatefiles", separateFiles);
	}
	XmlElement* rec = new XmlElement("RECORDING");
	rec->setAttribute("number", recordingNumber);
	//rec->setAttribute("length",(double)(timestamp-startTimestamp));
	for (int i = 0; i < processorArray.size(); i++)
	{
		XmlElement* proc = new XmlElement("PROCESSOR");
		proc->setAttribute("id", processorArray[i]->id);
		rec->setAttribute("samplerate", processorArray[i]->sampleRate);
		for (int j = 0; j < processorArray[i]->channels.size(); j++)
		{
			ChannelInfo* c = processorArray[i]->channels[j];
			XmlElement* chan = new XmlElement("CHANNEL");
			chan->setAttribute("name", c->name);
			chan->setAttribute("bitVolts", c->bitVolts);
			chan->setAttribute("filename", c->filename);
			chan->setAttribute("position", (double)(c->startPos)); //As long as the file doesnt exceed 2^53 bytes, this will have integer precission. Better than limiting to 32bits.
			proc->addChildElement(chan);
		}
		rec->addChildElement(proc);
	}
	xml->addChildElement(rec);
	xml->writeToFile(file, String::empty);
}

void OriginalRecording::setParameter(EngineParameter& parameter)
{
	boolParameter(0, separateFiles);
	boolParameter(1, renameFiles);
	strParameter(2, renamedPrefix);
}

RecordEngineManager* OriginalRecording::getEngineManager()
{
	RecordEngineManager* man = new RecordEngineManager("OPENEPHYS", "Open Ephys", nullptr);
	EngineParameter* param;
	param = new EngineParameter(EngineParameter::BOOL, 0, "Separate Files", false);
	man->addParameter(param);
	param = new EngineParameter(EngineParameter::BOOL, 1, "Rename files based on channel order", false);
	man->addParameter(param);
	param = new EngineParameter(EngineParameter::STR, 2, "Renamed files prefix", "CH");
	man->addParameter(param);
	return man;
}