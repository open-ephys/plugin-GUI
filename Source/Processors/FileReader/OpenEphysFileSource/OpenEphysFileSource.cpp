/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Open Ephys

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

#include "OpenEphysFileSource.h"

using namespace OpenEphysSource;



OpenEphysFileSource::OpenEphysFileSource() : m_samplePos(0)
{}

OpenEphysFileSource::~OpenEphysFileSource()
{}

bool OpenEphysFileSource::Open(File file)
{

    XmlDocument doc(file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || !xml->hasTagName("EXPERIMENT"))
    {
        LOGD("File not found!");
        delete xml;
        return false;
    }

    forEachXmlChildElement(*xml, element)
    {
        if (element->hasTagName("RECORDING"))
        {

			Recording recording;

            recording.id = element->getIntAttribute("number");
            int sampleRate = element->getIntAttribute("samplerate");

            forEachXmlChildElement(*element, processor)
            {
				//Currently the ID of the Record Node and not used
                int id = processor->getIntAttribute("id"); 

                forEachXmlChildElement(*processor, channel)
                {

                    ChannelInfo info;

                    info.filename = channel->getStringAttribute("filename");
                    info.name = channel->getStringAttribute("name");
                    info.bitVolts = channel->getStringAttribute("bitVolts").getDoubleValue();
                    info.startPos = channel->getIntAttribute("position");

					//Get processor id
					juce::StringArray tokens;
					tokens.addTokens (info.filename, "_.", "\"");
					int processorID = std::stoi(tokens[0].toStdString());

					if (!recording.processors.count(processorID))
					{
						ProcInfo procInfo;

						procInfo.id = processorID;
						procInfo.sampleRate = sampleRate;
						recording.processors[processorID] = procInfo;
					}
					recording.processors[processorID].channels.push_back(info);

                }

            }
			recordings[recording.id] = recording;
        }
    }

	m_rootPath = file.getParentDirectory();

	return true;
}

void OpenEphysFileSource::fillRecordInfo()
{

	for (auto rec : extract_keys(recordings))
	{

		for (auto procID : extract_keys(recordings[rec].processors))
		{

			RecordInfo info;

			info.name = String(procID);
			info.sampleRate = recordings[rec].processors[procID].sampleRate;
			juce::File dataFile = m_rootPath.getChildFile(recordings[rec].processors[procID].channels[0].filename);
			info.numSamples = (dataFile.getSize() - 1024) / 2070 * 1024;

			for (int i = 0; i < recordings[rec].processors[procID].channels.size(); i++)
			{

				RecordedChannelInfo cInfo;

				cInfo.name = recordings[rec].processors[procID].channels[i].name;
				cInfo.bitVolts = recordings[rec].processors[procID].channels[i].bitVolts;
				
				info.channels.add(cInfo);

			}

			infoArray.add(info);
			numRecords++;
			
		}
	}

}

void OpenEphysFileSource::updateActiveRecord()
{

	dataFiles.clear();

	//TODO: This needs to be two indeces instead of just one, assume first recording until data buffering is working...
	int selectedRecording = 0;
	int selectedProcessorID = extract_keys(recordings[selectedRecording].processors)[activeRecord.get()];

	for (int i = 0; i < infoArray[selectedRecording].channels.size(); i++)
	{
		juce::File dataFile = m_rootPath.getChildFile(recordings[selectedRecording].processors[selectedProcessorID].channels[i].filename);
		dataFiles.add(new MemoryMappedFile(dataFile, juce::MemoryMappedFile::readOnly));
	}

	m_samplePos = 0;

	blockIdx = 0;
	samplesLeftInBlock = 0;
	
}

void OpenEphysFileSource::seekTo(int64 sample)
{
	m_samplePos = sample % getActiveNumSamples();
}

int OpenEphysFileSource::readData(int16* buffer, int nSamples)
{

	int nChans = getActiveNumChannels();
	int64 samplesToRead;

	if (m_samplePos + nSamples > getActiveNumSamples())
	{
		// TODO: The last block of data is likley to contain trailing zeros
		// Compute the number of trailing zeros and subtract from samplesToRead here
		samplesToRead = getActiveNumSamples() - m_samplePos;
	}
	else
	{
		samplesToRead = nSamples;
	}

	readSamples(buffer, samplesToRead);

	m_samplePos += samplesToRead;

	return samplesToRead;

}

void OpenEphysFileSource::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{
	int n = getActiveNumChannels();
	float bitVolts = getChannelInfo(channel).bitVolts;

	for (int i = 0; i < numSamples; i++)
	{
		//TODO: Make sure this works on all platforms (tested on Linux only)
		int16 hibyte = (*(inBuffer + (n*i) + channel) & 0x00ff) << 8;
		int16 lobyte = (*(inBuffer + (n*i) + channel) & 0xff00) >> 8;
		*(outBuffer + i) = ( lobyte | hibyte ) * bitVolts;
	}
}

void OpenEphysFileSource::processEventData(EventInfo &info, int64 startTimestamp, int64 stopTimestamp) { /* TODO */ };

bool OpenEphysFileSource::isReady()
{
	return true;
}

void OpenEphysFileSource::readSamples(int16* buffer, int64 samplesToRead)
{

	int nChans = getActiveNumChannels();

	/* Organize samples into a vector that mimics BinaryFormat */
	std::vector<int16> samples;

	/* Read rest of previous block */
	if (samplesLeftInBlock > 0) 
	{
		for (int i = 0; i < samplesLeftInBlock; i++) 
		{
			for (int j = 0; j < nChans; j++)
			{
				int16* data = static_cast<int16*>(dataFiles[j]->getData()) + 518 + blockIdx*1035 + (1024 - samplesLeftInBlock) + i;
				samples.push_back(data[0]);
			}
		}

		samplesToRead -= samplesLeftInBlock;
		samplesLeftInBlock = 0;
		blockIdx = (blockIdx + 1) % (infoArray[getActiveRecord()].numSamples / 1024);

	} 

	/* Read full blocks */
	while (samplesToRead >= 1024)
	{
		for (int i = 0; i < 1024; i++)
		{
			for (int j = 0; j < nChans; j++)
			{
				int16* data = static_cast<int16*>(dataFiles[j]->getData()) + 518 + blockIdx*1035 + i;
				samples.push_back(data[0]);
			}
		}

		blockIdx = (blockIdx + 1) % (infoArray[getActiveRecord()].numSamples / 1024);
		samplesToRead -= 1024;

	}

	/* Read only some of next block */
	if (samplesToRead > 0)
	{
		for (int i = 0; i < samplesToRead; i++) 
		{
			for (int j = 0; j < nChans; j++)
			{
				int16* data = static_cast<int16*>(dataFiles[j]->getData()) + 518 + blockIdx*1035 + i;
				samples.push_back(data[0]);
			}
		}

		samplesLeftInBlock = 1024 - samplesToRead;

	}

	memcpy(buffer, &samples[0], samples.size()*sizeof(int16));

}