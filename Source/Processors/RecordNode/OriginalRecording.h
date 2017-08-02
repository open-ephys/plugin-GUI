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
#ifndef ORIGINALRECORDING_H_INCLUDED
#define ORIGINALRECORDING_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "RecordEngine.h"
#include <stdio.h>
#include <map>

#define HEADER_SIZE 1024
#define BLOCK_LENGTH 1024

#define VERSION 0.4

#define VSTR(s) #s
#define VSTR2(s) VSTR(s)
#define VERSION_STRING VSTR2(VERSION)

class OriginalRecording : public RecordEngine
{
public:
    OriginalRecording();
    ~OriginalRecording();

    void setParameter(EngineParameter& parameter);
    String getEngineID() const override;
    void openFiles(File rootFolder, int experimentNumber, int recordingNumber) override;
	void closeFiles() override;
	void writeData(int writeChannel, int realChannel, const float* buffer, int size) override;
	void writeEvent(int eventType, const MidiMessage& event, int64 timestamp) override;
	void resetChannels() override;
	void addSpikeElectrode(int index, const SpikeRecordInfo* elec) override;
	void writeSpike(int electrodeIndex, const SpikeObject& spike, int64 timestamp) override;

    static RecordEngineManager* getEngineManager();

private:
    String getFileName(Channel* ch);
    void openFile(File rootFolder, Channel* ch);
    String generateHeader(Channel* ch);
    void writeContinuousBuffer(const float* data, int nSamples, int channel);
    void writeTimestampAndSampleCount(FILE* file, int channel);
    void writeRecordMarker(FILE* file);

    void openSpikeFile(File rootFolder, SpikeRecordInfo* elec);
    String generateSpikeHeader(SpikeRecordInfo* elec);

    void openMessageFile(File rootFolder);
    void writeTTLEvent(const MidiMessage& event, int64 timestamp);
    void writeMessage(const MidiMessage& event, int64 timestamp);

    void writeXml();

    bool separateFiles;
    Array<int> blockIndex;
    Array<int> samplesSinceLastTimestamp;
    int recordingNumber;
    int experimentNumber;

    bool renameFiles;
    String renamedPrefix;

    /** Holds data that has been converted from float to int16 before
        saving.
    */
	HeapBlock<int16> continuousDataIntegerBuffer;
    //int16* continuousDataIntegerBuffer;

    /** Holds data that has been converted from float to int16 before
        saving.
    */
	HeapBlock<float> continuousDataFloatBuffer;
    //float* continuousDataFloatBuffer;

    /** Used to indicate the end of each record */
	HeapBlock<char> recordMarker;
    //char* recordMarker;

    AudioSampleBuffer zeroBuffer;

    FILE* eventFile;
    FILE* messageFile;
    Array<FILE*> fileArray;
    Array<FILE*> spikeFileArray;

    CriticalSection diskWriteLock;

    struct ChannelInfo
    {
        String name;
        String filename;
        float bitVolts;
        long int startPos;
    };
    struct ProcInfo
    {
        int id;
        float sampleRate;
        OwnedArray<ChannelInfo> channels;
    };

    OwnedArray<ProcInfo> processorArray;
    int lastProcId;
    String recordPath;
    int64 startTimestamp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OriginalRecording);
};

#endif  // ORIGINALRECORDING_H_INCLUDED
