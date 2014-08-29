/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen
 
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
 
#include "HDF5Recording.h"
#define MAX_BUFFER_SIZE 10000

HDF5Recording::HDF5Recording() : processorIndex(-1)
{
	scaledBuffer = new float[MAX_BUFFER_SIZE];
	intBuffer = new int16[MAX_BUFFER_SIZE];
}

HDF5Recording::~HDF5Recording()
{
	delete scaledBuffer;
	delete intBuffer;
}

void HDF5Recording::registerProcessor(GenericProcessor *proc)
{
	fileArray.add(new KWDFile());
	activeChannelCount.add(0);
	processorIndex++;
}

void HDF5Recording::resetChannels()
{
	processorIndex = -1;
	fileArray.clear();
	activeChannelCount.clear();
	processorMap.clear();
}

void HDF5Recording::addChannel(int index, Channel* chan)
{
	processorMap.add(processorIndex);
}

void HDF5Recording::openFiles(File rootFolder, int recordingNumber)
{
	for (int i = 0; i < processorMap.size(); i++)
	{
		int index = processorMap[i];
		if (getChannel(i)->getRecordState())
		{
			if (!fileArray[index]->isOpen())
			{
				fileArray[index]->initFile(getChannel(i)->nodeId,rootFolder.getFullPathName() + rootFolder.separatorString);
				fileArray[index]->open();
			}
			activeChannelCount.set(index,activeChannelCount[index]+1);
		}
	}
	for (int i = 0; i < fileArray.size(); i++)
	{
		if (fileArray[i]->isOpen())
		{
			fileArray[i]->startNewRecording(recordingNumber,activeChannelCount[i]);
		}
	}
}

void HDF5Recording::closeFiles()
{
	for (int i = 0; i < fileArray.size(); i++)
	{
		fileArray[i]->close();
		activeChannelCount.set(i,0);
	}
}

void HDF5Recording::writeData(AudioSampleBuffer& buffer, int nSamples)
{
	int index;
	for (int i = 0; i < buffer.getNumChannels(); i++)
	{
		if (getChannel(i)->getRecordState())
		{
			double multFactor = 1/(float(0x7fff) * getChannel(i)->bitVolts);
			int index = processorMap[getChannel(i)->recordIndex];
			FloatVectorOperations::copyWithMultiply(scaledBuffer,buffer.getReadPointer(i,0),multFactor,nSamples);
			AudioDataConverters::convertFloatToInt16LE(scaledBuffer,intBuffer,nSamples);
			fileArray[index]->writeRowData(intBuffer,nSamples);
		}
	}
}

void HDF5Recording::writeEvent(MidiMessage& event, int samplePosition)
{
	//TODO
}

void HDF5Recording::addSpikeElectrode(int index, SpikeRecordInfo* elec)
{
	//TODO
}
void HDF5Recording::writeSpike(const SpikeObject& spike, int electrodeIndex)
{
	//TODO
}