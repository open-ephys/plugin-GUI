/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "FileReader.h"
#include "FileReaderEditor.h"
#include <stdio.h>

#include "KwikFileSource.h"

FileReader::FileReader()
    : GenericProcessor("File Reader")
{

    timestamp = 0;

    enabledState(false);


}

FileReader::~FileReader()
{
}

AudioProcessorEditor* FileReader::createEditor()
{
    editor = new FileReaderEditor(this, true);

    return editor;

}

bool FileReader::isReady()
{
    if (input == nullptr)
    {
        sendActionMessage("No file selected in File Reader.");
        return false;
    }
    else
    {
        return true;
    }
}


float FileReader::getDefaultSampleRate()
{
    if (input)
        return currentSampleRate;
    else
        return 44100.0;
}

int FileReader::getDefaultNumOutputs()
{
    if (input)
        return currentNumChannels;
    else
        return 16;
}

float FileReader::getBitVolts(int chan)
{
    if (input)
        return channelInfo[chan].bitVolts;
    else
        return 0.05f;
}

void FileReader::enabledState(bool t)
{

    isEnabled = t;

}


bool FileReader::setFile(String fullpath)
{
    File file(fullpath);

    String ext = file.getFileExtension();

    if (!ext.compareIgnoreCase(".kwd"))
    {
        input = new KWIKFileSource();
    }
    else
    {
        sendActionMessage("File type not supported");
        return false;
    }

    if (!input->OpenFile(file))
    {
        input = nullptr;
        sendActionMessage("Invalid file");
        return false;
    }

    if (input->getNumRecords() <= 0)
    {
        input = nullptr;
        sendActionMessage("Empty file. Inoring open operation");
        return false;
    }
    static_cast<FileReaderEditor*>(getEditor())->populateRecordings(input);
    setActiveRecording(0);
}

void FileReader::setActiveRecording(int index)
{
    input->setActiveRecord(index);
    currentNumChannels = input->getActiveNumChannels();
    currentNumSamples = input->getActiveNumSamples();
    currentSampleRate = input->getActiveSampleRate();
    startSample = 0;
    stopSample = currentNumSamples;
    currentSample = 0;
    for (int i=0; i < currentNumChannels; i++)
    {
        channelInfo.add(input->getChannelInfo(i));
    }
    static_cast<FileReaderEditor*>(getEditor())->setTotalTime(samplesToMilliseconds(currentNumSamples));
    readBuffer.malloc(currentNumChannels*BUFFER_SIZE);
}

String FileReader::getFile()
{
    if (input)
        return input->getFileName();
    else
        return String::empty;
}

void FileReader::updateSettings()
{
    if (!input) return;

    for (int i=0; i < currentNumChannels; i++)
    {
        channels[i]->bitVolts = channelInfo[i].bitVolts;
        channels[i]->name = channelInfo[i].name;
    }
}



void FileReader::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{

    uint8 data[8];
    memcpy(data, &timestamp, 8);

    // generate timestamp
    addEvent(events,    // MidiBuffer
             TIMESTAMP, // eventType
             0,         // sampleNum
             nodeId,    // eventID
             0,		 // eventChannel
             8,         // numBytes
             data   // data
            );

    // FIXME: needs to account for the fact that the ratio might not be an exact
    //        integer value

    // code for testing events:
    // if (counter > 100)
    // {
    //     addEvent(events,    // MidiBuffer
    //          TTL, // eventType
    //          0,         // sampleNum
    //          1,    // eventID
    //          0      // eventChannel
    //         );
    //     counter = 0;
    // } else {
    //     counter++;

    // }



    int samplesNeeded = (int) float(buffer.getNumSamples()) * (getDefaultSampleRate()/44100.0f);

    int samplesReaded = 0;

    while (samplesReaded < samplesNeeded)
    {
        int samplesToRead = samplesNeeded - samplesReaded;
        if ((currentSample + samplesToRead) > stopSample)
        {
            samplesToRead = stopSample - currentSample;
            if (samplesToRead > 0)
                input->readData(readBuffer+samplesReaded,samplesToRead);
            input->seekTo(startSample);
            currentSample = startSample;
        }
        else
        {
            input->readData(readBuffer+samplesReaded,samplesToRead);
            currentSample += samplesToRead;
        }
        samplesReaded += samplesToRead;
    }
    for (int i=0; i < currentNumChannels; i++)
    {
        input->processChannelData(readBuffer,buffer.getWritePointer(i,0),i,samplesNeeded);
    }

    timestamp += samplesNeeded;
    static_cast<FileReaderEditor*>(getEditor())->setCurrentTime(samplesToMilliseconds(currentSample));
    nSamples = samplesNeeded;

}


void FileReader::setParameter(int parameterIndex, float newValue)
{
    switch (parameterIndex)
    {
        case 0: //Change selected recording
            setActiveRecording(newValue);
            break;
        case 1: //set startTime
            startSample = millisecondsToSamples(newValue);
            currentSample = startSample;
            static_cast<FileReaderEditor*>(getEditor())->setCurrentTime(samplesToMilliseconds(currentSample));
            break;
        case 2: //set stop time
            stopSample = millisecondsToSamples(newValue);
            currentSample = startSample;
            static_cast<FileReaderEditor*>(getEditor())->setCurrentTime(samplesToMilliseconds(currentSample));
            break;
    }
}

unsigned int FileReader::samplesToMilliseconds(int64 samples)
{
    return (unsigned int)(1000.f*float(samples)/currentSampleRate);
}

int64 FileReader::millisecondsToSamples(unsigned int ms)
{
    return (int64)(currentSampleRate*float(ms)/1000.f);
}
