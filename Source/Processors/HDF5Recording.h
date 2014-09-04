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

#ifndef HDF5RECORDING_H_INCLUDED
#define HDF5RECORDING_H_INCLUDED

#include "RecordEngine.h"
#include "HDF5FileFormat.h"

class HDF5Recording : public RecordEngine
{
public:
	HDF5Recording();
	~HDF5Recording();
	void openFiles(File rootFolder, int recordingNumber);
	void closeFiles();
	void writeData(AudioSampleBuffer& buffer, int nSamples);
	void writeEvent(MidiMessage& event, int samplePosition);
	void addChannel(int index, Channel* chan);
	void addSpikeElectrode(int index, SpikeRecordInfo* elec);
	void writeSpike(const SpikeObject& spike, int electrodeIndex);
	void registerProcessor(GenericProcessor* processor);
	void resetChannels();
	void updateTimeStamp(int64 timestamp);

private:

	int processorIndex;
	
	Array<int> processorMap;
	Array<int> activeChannelCount;
	OwnedArray<KWDFile> fileArray;
	OwnedArray<HDF5RecordingInfo> infoArray;
	ScopedPointer<KWIKFile> mainFile;
	float* scaledBuffer;
	int16* intBuffer;
	int64 timestamp;

};


#endif  // HDF5RECORDING_H_INCLUDED
