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

BinaryFileSource::BinaryFileSource() : m_samplePos(0)
{}

BinaryFileSource::~BinaryFileSource()
{}

bool BinaryFileSource::Open(File file)
{
	m_jsonData = JSON::parse(file);
	if (m_jsonData.isVoid())
		return false;

	if (m_jsonData["GUI version"].isVoid())
		return false;
	
	var cont = m_jsonData["continuous"];
	if (cont.isVoid() || cont.size() <= 0)
		return false;

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