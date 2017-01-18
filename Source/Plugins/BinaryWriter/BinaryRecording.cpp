/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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

#include "BinaryRecording.h"

#define MAX_BUFFER_SIZE 40960

using namespace BinaryRecordingEngine;

BinaryRecording::BinaryRecording()
{
	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
}

BinaryRecording::~BinaryRecording()
{

}

String BinaryRecording::getEngineID() const
{
	return "RAWBINARY";
}

void BinaryRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
	String basepath = rootFolder.getFullPathName() + rootFolder.separatorString + "experiment" + String(experimentNumber);
	//Open channel files
	int nProcessors = getNumRecordedProcessors();

	for (int i = 0; i < nProcessors; i++)
	{
		const RecordProcessorInfo& pInfo = getProcessorInfo(i);
		File datFile(basepath + "_" + String(pInfo.processorId) + "_" + String(recordingNumber) + ".dat");
		ScopedPointer<SequentialBlockFile> bFile = new SequentialBlockFile(pInfo.recordedChannels.size(), samplesPerBlock);
		if (bFile->openFile(datFile))
			m_DataFiles.add(bFile.release());
	}
	int nChans = getNumRecordedChannels();
	//Origin Timestamp
	for (int i = 0; i < nChans; i++)
	{
		m_startTS.add(getTimestamp(i));
	}

	//Other files, using OriginalRecording code
	openEventFile(basepath, recordingNumber);
	openMessageFile(basepath, recordingNumber);
	for (int i = 0; i < spikeFileArray.size(); i++)
	{
		openSpikeFile(basepath, i, recordingNumber);
	}
	m_recordingNum = recordingNumber;
}

void BinaryRecording::closeFiles()
{
	m_DataFiles.clear();
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
	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
	m_bufferSize = MAX_BUFFER_SIZE;
	m_startTS.clear();
}

void BinaryRecording::resetChannels()
{
	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
	m_bufferSize = MAX_BUFFER_SIZE;
	m_DataFiles.clear();
	spikeFileArray.clear();
	m_startTS.clear();
}

void BinaryRecording::writeData(int writeChannel, int realChannel, const float* buffer, int size)
{
	if (size > m_bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	{
		std::cerr << "Write buffer overrun, resizing to" << size << std::endl;
		m_bufferSize = size;
		m_scaledBuffer.malloc(size);
		m_intBuffer.malloc(size);
	}
	double multFactor = 1 / (float(0x7fff) * getDataChannel(realChannel)->getBitVolts());
	FloatVectorOperations::copyWithMultiply(m_scaledBuffer.getData(), buffer, multFactor, size);
	AudioDataConverters::convertFloatToInt16LE(m_scaledBuffer.getData(), m_intBuffer.getData(), size);

	m_DataFiles[getProcessorFromChannel(writeChannel)]->writeChannel(getTimestamp(writeChannel)-m_startTS[writeChannel],getChannelNumInProc(writeChannel),m_intBuffer.getData(),size);
}

//Code below is copied from OriginalRecording, so it's not as clean as newer one

void BinaryRecording::addSpikeElectrode(int index, const SpikeChannel* elec)
{
	spikeFileArray.add(nullptr);
}

void BinaryRecording::openEventFile(String basepath, int recordingNumber)
{
	FILE* chFile;
	String fullPath = basepath + "_all_channels_" + String(recordingNumber) + ".events";


	
	std::cout << "OPENING FILE: " << fullPath << std::endl;

	File f = File(fullPath);

	bool fileExists = f.exists();

	diskWriteLock.enter();

	chFile = fopen(fullPath.toUTF8(), "ab");

	if (!fileExists)
	{
		// create and write header
		std::cout << "Writing header." << std::endl;
		String header = generateEventHeader();
		//std::cout << header << std::endl;
		std::cout << "File ID: " << chFile << ", number of bytes: " << header.getNumBytesAsUTF8() << std::endl;


		fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), chFile);

		std::cout << "Wrote header." << std::endl;

		// std::cout << "Block index: " << blockIndex << std::endl;

	}
	else
	{
		std::cout << "File already exists, just opening." << std::endl;
		fseek(chFile, 0, SEEK_END);
	}

	eventFile = chFile;
	
	diskWriteLock.exit();

}

void BinaryRecording::openSpikeFile(String basePath, int spikeIndex, int recordingNumber)
{
	const SpikeChannel* elec = getSpikeChannel(spikeIndex);
	FILE* spFile;
	String fullPath = basePath + "_" + elec->getName().removeCharacters(" ") + "_" + String(recordingNumber) + ".spikes";
	
	std::cout << "OPENING FILE: " << fullPath << std::endl;

	File f = File(fullPath);

	bool fileExists = f.exists();

	diskWriteLock.enter();

	spFile = fopen(fullPath.toUTF8(), "ab");

	if (!fileExists)
	{
		String header = generateSpikeHeader(elec);
		fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), spFile);
	}
	diskWriteLock.exit();
	spikeFileArray.set(spikeIndex, spFile);

}

void BinaryRecording::openMessageFile(String basepath, int recordNumber)
{
	FILE* mFile;
	String fullPath = basepath + "_messages_" + String(recordNumber) + ".events";

	fullPath += "messages";



	std::cout << "OPENING FILE: " << fullPath << std::endl;

	File f = File(fullPath);

	//bool fileExists = f.exists();

	diskWriteLock.enter();

	mFile = fopen(fullPath.toUTF8(), "ab");

	//If this file needs a header, it goes here

	diskWriteLock.exit();
	messageFile = mFile;

}

String BinaryRecording::generateEventHeader()
{

	String header = "header.format = 'Open Ephys Data Format'; \n";

	header += "header.version = " + String(VERSION_STRING) + "; \n";
	header += "header.header_bytes = ";
	header += String(HEADER_SIZE);
	header += ";\n";

	header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample position, one uint8 event type, one uint8 processor ID, one uint8 event ID, one uint8 event channel, and one uint16 recordingNumber'; \n";


	header += "header.date_created = '";
	header += generateDateString();
	header += "';\n";

	header += "header.channel = '";
	header += "Events";
	header += "';\n";

    header += "header.channelType = 'Event';\n";


	header += "header.sampleRate = ";
	// all channels need to have the same sample rate under the current scheme
	header += String(getEventChannel(0)->getSampleRate());
	header += ";\n";
	header += "header.blockLength = ";
	header += BLOCK_LENGTH;
	header += ";\n";
	header += "header.bufferSize = ";
	header += "1024";
	header += ";\n";
	header += "header.bitVolts = ";
	header += "1";
	header += ";\n";

	header = header.paddedRight(' ', HEADER_SIZE);

	//std::cout << header << std::endl;

	return header;

}

String BinaryRecording::generateSpikeHeader(const SpikeChannel* elec)
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
	header += elec->getNumChannels();
	header += ";\n";

	header += "header.sampleRate = ";
	header += String(elec->getSampleRate());
	header += ";\n";

	header = header.paddedRight(' ', HEADER_SIZE);

	//std::cout << header << std::endl;

	return header;
}

void BinaryRecording::writeEvent(int eventIndex, const MidiMessage& event)
{
	writeTTLEvent(eventIndex, event);
	if (Event::getEventType(event) == EventChannel::TEXT)
	{
		TextEventPtr ev = TextEvent::deserializeFromMessage(event, getEventChannel(eventIndex));
		if (ev == nullptr) return;
		writeMessage(ev->getText(), ev->getSourceID(), ev->getChannel(), ev->getTimestamp());
	}
}

void BinaryRecording::writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float, String text)
{
	writeMessage(text, sourceID, 255, timestamp);
}

void BinaryRecording::writeMessage(String message, uint16 processorID, uint16 channel, int64 timestamp)
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

void BinaryRecording::writeTTLEvent(int eventIndex, const MidiMessage& event)
{
	// find file and write samples to disk
	// std::cout << "Received event!" << std::endl;

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
	*reinterpret_cast<uint16*>(data + 14) = static_cast<uint16>(m_recordingNum);


	diskWriteLock.enter();

	fwrite(&data,					// ptr
		sizeof(uint8),   							// size of each element
		16, 		  						// count
		eventFile);   			// ptr to FILE object

	diskWriteLock.exit();
}


void BinaryRecording::writeSpike(int electrodeIndex, const SpikeEvent* spike)
{
	if (spikeFileArray[electrodeIndex] == nullptr)
		return;

	HeapBlock<char> spikeBuffer;
	const SpikeChannel* channel = getSpikeChannel(electrodeIndex);

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

	int ptrIdx = 0;
	for (int i = 0; i < numChannels; i++)
	{
		float scaleFactor = 1 / (float(0x7fff) * channel->getChannelBitVolts(i));
		FloatVectorOperations::copyWithMultiply(m_scaledBuffer + ptrIdx, spike->getDataPointer(i), scaleFactor, chanSamples);
		ptrIdx += chanSamples;
	}
	AudioDataConverters::convertFloatToInt16LE(m_scaledBuffer, (spikeBuffer.getData() + 42), totalSamples);
	ptrIdx = totalSamples * 2 + 42;
	for (int i = 0; i < numChannels; i++)
	{
		*reinterpret_cast<float*>(spikeBuffer.getData() + ptrIdx) = channel->getGain();
		ptrIdx += sizeof(float);
	}
	for (int i = 0; i < numChannels; i++)
	{
		*reinterpret_cast<int16*>(spikeBuffer.getData() + ptrIdx) = spike->getThreshold(i);
		ptrIdx += sizeof(int16);
	}

	diskWriteLock.enter();

	fwrite(spikeBuffer, 1, totalBytes, spikeFileArray[electrodeIndex]);

	fwrite(&m_recordingNum,                         // ptr
		2,                               // size of each element
		1,                               // count
		spikeFileArray[electrodeIndex]); // ptr to FILE object

	diskWriteLock.exit();
}

RecordEngineManager* BinaryRecording::getEngineManager()
{
	RecordEngineManager* man = new RecordEngineManager("RAWBINARY", "Binary", &(engineFactory<BinaryRecording>));
	return man;
}