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
    String getEngineID();
    void openFiles(File rootFolder, int experimentNumber, int recordingNumber);
    void closeFiles();
    void writeData(AudioSampleBuffer& buffer);
    void writeEvent(int eventType, MidiMessage& event, int samplePosition);
    void addChannel(int index, Channel* chan);
    void resetChannels();
    //void updateTimeStamp(int64 timestamp);
    void addSpikeElectrode(int index, SpikeRecordInfo* elec);
    void writeSpike(const SpikeObject& spike, int electrodeIndex);

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
    void writeTTLEvent(MidiMessage& event, int samplePosition);
    void writeMessage(MidiMessage& event, int samplePosition);

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
    int16* continuousDataIntegerBuffer;

    /** Holds data that has been converted from float to int16 before
        saving.
    */
    float* continuousDataFloatBuffer;

    /** Used to indicate the end of each record */
    char* recordMarker;

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
