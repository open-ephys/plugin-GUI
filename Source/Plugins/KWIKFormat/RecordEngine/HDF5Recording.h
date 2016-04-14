/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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

#include <RecordingLib.h>
#include "HDF5FileFormat.h"

class HDF5Recording : public RecordEngine
{
public:
    HDF5Recording();
    ~HDF5Recording();
    String getEngineID() const override;
    void openFiles(File rootFolder, int experimentNumber, int recordingNumber) override;
	void closeFiles() override;
	void writeData(int writeChannel, int realChannel, const float* buffer, int size) override;
	void writeEvent(int eventType, const MidiMessage& event, int64 timestamp) override;
	void addChannel(int index, const Channel* chan) override;
	void addSpikeElectrode(int index,const  SpikeRecordInfo* elec) override;
	void writeSpike(int electrodeIndex, const SpikeObject& spike, int64 timestamp) override;
	void registerProcessor(const GenericProcessor* processor) override;
	void resetChannels() override;
	void startAcquisition() override;
	void endChannelBlock(bool lastBlock) override;

    static RecordEngineManager* getEngineManager();
private:

    int processorIndex;

    Array<int> processorMap;
	Array<int> channelsPerProcessor;
	Array<int> recordedChanToKWDChan;
    OwnedArray<Array<float>> bitVoltsArray;
    OwnedArray<Array<float>> sampleRatesArray;
	OwnedArray<Array<int64>> channelTimestampArray;
	Array<int> channelLeftOverSamples;
    OwnedArray<KWDFile> fileArray;
    OwnedArray<HDF5RecordingInfo> infoArray;
    ScopedPointer<KWEFile> eventFile;
    ScopedPointer<KWXFile> spikesFile;
	HeapBlock<float> scaledBuffer;
	HeapBlock<int16> intBuffer;
	int bufferSize;
    //float* scaledBuffer;
    //int16* intBuffer;

    bool hasAcquired;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5Recording);
};


#endif  // HDF5RECORDING_H_INCLUDED
