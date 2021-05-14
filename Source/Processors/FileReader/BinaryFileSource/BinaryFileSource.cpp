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

BinaryFileSource::BinaryFileSource() : m_samplePos(0), hasEventData(false)
{}

BinaryFileSource::~BinaryFileSource()
{}

bool BinaryFileSource::Open(File file)
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
	var continuousData = m_jsonData["continuous"];

	//create identifiers to speed up stuff
	Identifier idFolder("folder_name");
	Identifier idSampleRate("sample_rate");
	Identifier idNumChannels("num_channels");
	Identifier idChannels("channels");
	Identifier idChannelName("channel_name");
	Identifier idBitVolts("bit_volts");

	int numProcessors = continuousData.size();

	for (int i = 0; i < numProcessors; i++)
	{
		var record = continuousData[i];
		if (record.isVoid()) continue;

		var channels = record[idChannels];
		if (channels.isVoid() || channels.size() <= 0) continue;

		RecordInfo info;

		String folderName = record[idFolder];
		folderName = folderName.trimCharactersAtEnd("/");

		File dataFile = m_rootPath.getChildFile("continuous").getChildFile(folderName).getChildFile("continuous.dat");
		if (!dataFile.existsAsFile()) continue;

		int numChannels = record[idNumChannels];
		int64 numSamples = (dataFile.getSize() / numChannels) / sizeof(int16);

		info.name = folderName;
		info.sampleRate = record[idSampleRate];
		info.numSamples = numSamples;

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
		
		//create identifiers to speed up stuff
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

			String folderName = events[idFolder];
			folderName = folderName.trimCharactersAtEnd("/");

			File channelFile = m_rootPath.getChildFile("events").getChildFile(folderName).getChildFile("channels.npy");
			if (!channelFile.existsAsFile()) continue;

			File channelStatesFile = m_rootPath.getChildFile("events").getChildFile(folderName).getChildFile("channel_states.npy");
			if (!channelStatesFile.existsAsFile()) continue;

			File timestampsFile = m_rootPath.getChildFile("events").getChildFile(folderName).getChildFile("timestamps.npy");
			if (!timestampsFile.existsAsFile()) continue;

			int channelFileSize = channelFile.getSize(); 

			int nEvents = (channelFileSize - EVENT_HEADER_SIZE_IN_BYTES) / BYTES_PER_EVENT; 

			//Convert to std::unique_ptr<FileInputStream> when upgrade to JUCE6
			ScopedPointer<FileInputStream> channelDataStream = channelFile.createInputStream();
			MemoryBlock channelData;
			ScopedPointer<FileInputStream> channelStateDataStream = channelStatesFile.createInputStream();
			MemoryBlock channelStateData;
			ScopedPointer<FileInputStream> timestampsDataStream = timestampsFile.createInputStream();
			MemoryBlock timestampData;

			const int maxSensibleFileSize = 2 * 1024 * 1024;

			// (put a sanity-check on the file size, as channel files are relatively small)
			if (!(channelDataStream->readIntoMemoryBlock (channelData, maxSensibleFileSize))) continue;
			if (!(channelStateDataStream->readIntoMemoryBlock (channelStateData, maxSensibleFileSize))) continue;
			if (!(timestampsDataStream->readIntoMemoryBlock (timestampData, maxSensibleFileSize))) continue;
		
			std::vector<int16> channels;
			std::vector<int16> channelStates;
			std::vector<int64> timestamps;

			for (int i = 0; i < nEvents; i++)
			{
				int16* data = static_cast<int16*>(channelFile.getData() + EVENT_HEADER_SIZE_IN_BYTES + i*sizeof(int16));	
				channels.push_back(*data);

				data = static_cast<int16*>(channelStateData.getData() + EVENT_HEADER_SIZE_IN_BYTES + i*sizeof(int16));
				channelStates.push_back(*data);

				int64* tsData = static_cast<int64*>(timestampData.getData() + EVENT_HEADER_SIZE_IN_BYTES + i*sizeof(int64));
				timestamps.push_back(*tsData);

			}

			for (int i = 0; i < channels.size(); i++)
				LOGD("CH: ", channels[i], " State: ", channelStates[i], " ts: ", timestamps[i]);


		}
	}

}

void BinaryFileSource::updateActiveRecord()
{
	m_dataFile = new MemoryMappedFile(m_dataFileArray[activeRecord.get()], MemoryMappedFile::readOnly);
	m_samplePos = 0;
}

void BinaryFileSource::seekTo(int64 sample)
{
	m_samplePos = sample % getActiveNumSamples();
}

int BinaryFileSource::readData(int16* buffer, int nSamples)
{
	int nChans = getActiveNumChannels();
	int64 samplesToRead;

	if (m_samplePos + nSamples > getActiveNumSamples())
	{
		samplesToRead = getActiveNumSamples() - m_samplePos;
	}
	else
	{
		samplesToRead = nSamples;
	}

	int16* data = static_cast<int16*>(m_dataFile->getData()) + (m_samplePos * nChans);

	memcpy(buffer, data, samplesToRead*nChans*sizeof(int16));
    m_samplePos += samplesToRead;
	return samplesToRead;
}

void BinaryFileSource::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{
	int n = getActiveNumChannels();
	float bitVolts = getChannelInfo(channel).bitVolts;

	for (int i = 0; i < numSamples; i++)
	{
		*(outBuffer + i) = *(inBuffer + (n*i) + channel) * bitVolts;
	}
}

bool BinaryFileSource::isReady()
{
	return true;
}