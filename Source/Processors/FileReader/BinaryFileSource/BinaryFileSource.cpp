/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2018 Open Ephys

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

#include "BinaryFileSource.h"

using namespace BinarySource;

BinaryFileSource::BinaryFileSource() 
	: m_samplePos(0), 
	  hasEventData(false), 
	  loopCount(0)
{

}

bool BinaryFileSource::open(File file)
{
	m_jsonData = JSON::parse(file);
	if (m_jsonData.isVoid())
	{
		std::cout << "Invalid JSON." << std::endl;
		return false;
	}
		

	if (m_jsonData["GUI version"].isVoid())
	{
		std::cout << "No GUI version." << std::endl;
		return false;
	}
		
	
	var cont = m_jsonData["continuous"];
	if (cont.isVoid() || cont.size() <= 0)
	{
		std::cout << "No continuous data found." << std::endl;
		return false;
	}

	var event = m_jsonData["events"];
	if (!(event.isVoid() || event.size() <= 0))
	{
		hasEventData = true;
	}

	m_rootPath = file.getParentDirectory();

	return true;
}

void BinaryFileSource::fillRecordInfo()
{

	Identifier idGUIVersion("GUI version");
	String guiVersion = m_jsonData[idGUIVersion];

	String sampleNumbersFilename;
	String channelStatesFilename;

	int minorVersion = guiVersion.substring(2,3).getIntValue();

	if (minorVersion < 6) {
		sampleNumbersFilename = "timestamps.npy";
		channelStatesFilename = "channel_states.npy";
	}
	else
	{
		sampleNumbersFilename = "sample_numbers.npy";
		channelStatesFilename = "states.npy";
	}


	const int maxSensibleFileSize = 2 * 1024 * 1024; 

	var continuousData = m_jsonData["continuous"];

	//create identifiers to speed up stuff
	Identifier idFolder("folder_name");
	Identifier idSampleRate("sample_rate");
	Identifier idNumChannels("num_channels");
	Identifier idChannels("channels");
	Identifier idChannelName("channel_name");
	Identifier idBitVolts("bit_volts");

	int numProcessors = continuousData.size();

	std::map<String, int64> startSampleNumbers;

	for (int i = 0; i < numProcessors; i++)
	{
		var record = continuousData[i];
		if (record.isVoid()) continue;

		var channels = record[idChannels];
		if (channels.isVoid() || channels.size() <= 0) continue;

		RecordInfo info;

		String streamName = record[idFolder];
		streamName = streamName.trimCharactersAtEnd("/");

		File dataFile = m_rootPath.getChildFile("continuous").getChildFile(streamName).getChildFile("continuous.dat");
		if (!dataFile.existsAsFile()) continue;

		int numChannels = record[idNumChannels];
		int64 numSamples = (dataFile.getSize() / numChannels) / sizeof(int16);

		info.name = streamName;
		info.sampleRate = record[idSampleRate];
		info.numSamples = numSamples;
		
		File tsFile = m_rootPath.getChildFile("continuous").getChildFile(streamName).getChildFile(sampleNumbersFilename);
		if (tsFile.exists())
		{
			std::unique_ptr<FileInputStream> tsDataStream = tsFile.createInputStream();
			MemoryBlock tsData;
			if (!(tsDataStream->readIntoMemoryBlock(tsData, maxSensibleFileSize))) continue;
			int64* startTimestamp = (int64*)tsData.getData() + EVENT_HEADER_SIZE_IN_BYTES / 8;
			info.startTimestamp = *startTimestamp;
			startSampleNumbers[streamName] = *startTimestamp;
		}
		else 
		{
			info.startTimestamp = 0;
		}

		for (int c = 0; c < numChannels; c++)
		{
			var chan = channels[c];
			RecordedChannelInfo cInfo;

			cInfo.name = chan[idChannelName];
			cInfo.bitVolts = chan[idBitVolts];
			
			info.channels.add(cInfo);
		}
		
		infoArray.add(info);
		numRecords++;	

		m_dataFileArray.add(dataFile);
		
	}

	if (hasEventData)
	{

		var eventData = m_jsonData["events"];
		
		/* Create identifiers for efficiency */
		Identifier idFolder("folder_name");
		Identifier idChannelName("channel_name");
		Identifier idDescription("description");
		Identifier idIdentifier("identifier");
		Identifier idSampleRate("sample_rate");
		Identifier idType("type");
		Identifier idNumChannels("num_channels");
		Identifier idChannels("channels");
		Identifier idSourceProcessor("source_processor");

		int numEventProcessors = eventData.size();

		for (int i = 0; i < numEventProcessors; i++) 
		{

			var events = eventData[i];

			String streamName = events[idFolder];
			streamName = streamName.trimCharactersAtEnd("/");

			File sampleNumbersFile = m_rootPath.getChildFile("events").getChildFile(streamName).getChildFile(sampleNumbersFilename);
			std::unique_ptr<MemoryMappedFile> sampleNumbersMap(new MemoryMappedFile(sampleNumbersFile, MemoryMappedFile::readOnly));

			if (sampleNumbersFile.getSize() == EVENT_HEADER_SIZE_IN_BYTES)
				continue;

			int nEvents = (sampleNumbersFile.getSize() - EVENT_HEADER_SIZE_IN_BYTES) / 8;

			if (streamName.contains("TTL"))
			{

				File channelStatesFile = m_rootPath.getChildFile("events").getChildFile(streamName).getChildFile(channelStatesFilename);
				std::unique_ptr<MemoryMappedFile> channelStatesFileMap(new MemoryMappedFile(channelStatesFile, MemoryMappedFile::readOnly));


				streamName = streamName.substring(0,streamName.lastIndexOf("/TTL"));

				EventInfo eventInfo;

				for (int j = 0; j < nEvents; j++)
				{
					int16* data = static_cast<int16*>(channelStatesFileMap->getData()) + (EVENT_HEADER_SIZE_IN_BYTES / 2) + j * sizeof(int16) / 2;
					eventInfo.channels.push_back(abs(*data));
					eventInfo.channelStates.push_back(*data > 0);
					int64* snData = static_cast<int64*>(sampleNumbersMap->getData()) + (EVENT_HEADER_SIZE_IN_BYTES / 8) + j * sizeof(int64) / 8;
					eventInfo.timestamps.push_back(*snData - startSampleNumbers[streamName]);
					eventInfo.text.push_back("");
				}
				eventInfoMap[streamName] = eventInfo;
			}
			else if (streamName.equalsIgnoreCase("MessageCenter"))
			{

				File textFile = m_rootPath.getChildFile("events").getChildFile(streamName).getChildFile("text.npy");

				juce::FileInputStream inputStream(textFile);
				inputStream.skipNextBytes(10); // \x93NUMPY \x01 \x00
				String line = inputStream.readNextLine();

				uint64 itemSize = std::stoi(line.fromFirstOccurrenceOf("'|S", 0, 0).upToFirstOccurrenceOf("'", 0, 0).toStdString());

				EventInfo eventInfo;
				int k_word;

				juce::MemoryBlock buffer(itemSize);
				auto *data = static_cast<char *>(buffer.getData());

				for (int j = 0; j < nEvents; j++)
				{

					for (int k = 0; k < itemSize; k++)
					{
						data[k] = inputStream.readByte();
						if ((data[k] == 0) || (k == itemSize - 1))
							k_word = k;
					} 

					String outString = juce::String::fromUTF8(data, (int)k_word);
					eventInfo.channels.push_back(0);
					eventInfo.channelStates.push_back(0);
					int64 *snData = static_cast<int64 *>(sampleNumbersMap->getData()) + (EVENT_HEADER_SIZE_IN_BYTES / 8) + j * sizeof(int64) / 8;
					eventInfo.timestamps.push_back(*snData - startSampleNumbers[streamName]);
					eventInfo.text.push_back(outString);
				}
				eventInfoMap[streamName] = eventInfo;
			}
		}
	}
}

void BinaryFileSource::processEventData(EventInfo &eventInfo, int64 start, int64 stop)
{

	int local_start = start % getActiveNumSamples();
	int local_stop = stop % getActiveNumSamples();
	int loop_count = start / getActiveNumSamples();

	std::vector<String> includeStreams = {currentStream, "MessageCenter"};

	for (int s = 0; s < includeStreams.size(); s++)
	{
		EventInfo info = eventInfoMap[includeStreams[s]];

		int i = 0;
			
		while (i < info.timestamps.size())
		{
			if (info.timestamps[i] >= local_start && info.timestamps[i] <= local_stop)
			{
				eventInfo.channels.push_back(info.channels[i] - 1);
				eventInfo.channelStates.push_back((info.channelStates[i]));
				eventInfo.timestamps.push_back(info.timestamps[i] + loop_count * getActiveNumSamples());
				eventInfo.text.push_back(info.text[i]);
			}
			i++;
		}
	}
}

void BinaryFileSource::updateActiveRecord(int index)
{
    m_dataFile.reset();
	m_dataFile = std::make_unique<MemoryMappedFile>(m_dataFileArray[index], MemoryMappedFile::readOnly);
	m_samplePos = 0;
	numActiveChannels = getActiveNumChannels();

	bitVolts.clear();

	for (int i = 0; i < numActiveChannels; i++)
		bitVolts.add(getChannelInfo(index, i).bitVolts);

	currentStream = m_dataFileArray[index].getParentDirectory().getFileName();

}

void BinaryFileSource::seekTo(int64 sample)
{
	m_samplePos = sample % getActiveNumSamples();
}

int BinaryFileSource::readData(int16* buffer, int nSamples)
{
	int64 samplesToRead;

	if (m_samplePos + nSamples > getActiveNumSamples())
	{
		samplesToRead = getActiveNumSamples() - m_samplePos;
	}
	else
	{
		samplesToRead = nSamples;
	}

	int16* data = static_cast<int16*>(m_dataFile->getData()) + (m_samplePos * numActiveChannels);

	//FIXME: Can crash here (heap overflow?), secondary either to wrong index or scrubbing too fast? Not sure yet. 
	memcpy(buffer, data, samplesToRead * numActiveChannels * sizeof(int16));
    m_samplePos += samplesToRead;
	return samplesToRead;
}

void BinaryFileSource::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{

	if (!inBuffer) return;

	for (int i = 0; i < numSamples; i++)
	{
		*(outBuffer + i) = *(inBuffer + (numActiveChannels * i) + channel) * bitVolts[channel];
	}
}
