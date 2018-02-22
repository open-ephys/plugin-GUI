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

#ifndef KWIKFORMAT_H_INCLUDED
#define KWIKFORMAT_H_INCLUDED
 
#include <OpenEphysHDF5Lib/HDF5FileFormat.h>
using namespace OpenEphysHDF5;

struct KWIKRecordingInfo
{
	String name;
	int64 start_time;
	uint32 start_sample;
	float sample_rate;
	uint32 bit_depth;
	Array<float> bitVolts;
	Array<float> channelSampleRates;
	bool multiSample;
};

class KWDFile : public HDF5FileBase
{
public:
    KWDFile(int processorNumber, String basename);
    KWDFile();
    virtual ~KWDFile();
    void initFile(int processorNumber, String basename);
    void startNewRecording(int recordingNumber, int nChannels, KWIKRecordingInfo* info);
    void stopRecording();
    void writeBlockData(int16* data, int nSamples);
    void writeRowData(int16* data, int nSamples);
	void writeRowData(int16* data, int nSamples, int channel);
	void writeTimestamps(int64* ts, int nTs, int channel);
    String getFileName();

protected:
    int createFileStructure();

private:
    int recordingNumber;
    int nChannels;
    int curChan;
    String filename;
    bool multiSample;
    ScopedPointer<HDF5RecordingData> recdata;
	ScopedPointer<HDF5RecordingData> tsData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWDFile);
};

class KWEFile : public HDF5FileBase
{
public:
    KWEFile(String basename);
    KWEFile();
    virtual ~KWEFile();
    void initFile(String basename);
    void startNewRecording(int recordingNumber, KWIKRecordingInfo* info);
    void stopRecording();
    void writeEvent(int type, uint8 id, uint8 processor, void* data, int64 timestamp);
  //  void addKwdFile(String filename);
	void addEventType(String name, BaseDataType type, String dataName);
    String getFileName();

protected:
    int createFileStructure();

private:
    int recordingNumber;
    String filename;
    OwnedArray<HDF5RecordingData> timeStamps;
    OwnedArray<HDF5RecordingData> recordings;
    OwnedArray<HDF5RecordingData> eventID;
    OwnedArray<HDF5RecordingData> nodeID;
    OwnedArray<HDF5RecordingData> eventData;
    Array<String> eventNames;
	Array<BaseDataType> eventTypes;
    Array<String> eventDataNames;
    int kwdIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWEFile);
};

class KWXFile : public HDF5FileBase
{
public:
    KWXFile(String basename);
    KWXFile();
    virtual ~KWXFile();
    void initFile(String basename);
    void startNewRecording(int recordingNumber);
    void stopRecording();
    void addChannelGroup(int nChannels);
    void resetChannels();
    void writeSpike(int groupIndex, int nSamples, const float* data, Array<float>& bitVolts, int64 timestamp);
    String getFileName();

protected:
    int createFileStructure();

private:
    int createChannelGroup(int index);
    int recordingNumber;
    String filename;
    OwnedArray<HDF5RecordingData> spikeArray;
    OwnedArray<HDF5RecordingData> recordingArray;
    OwnedArray<HDF5RecordingData> timeStamps;
    Array<int> channelArray;
    int numElectrodes;
	HeapBlock<int16> transformVector;
    //int16* transformVector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWXFile);
};

#endif
