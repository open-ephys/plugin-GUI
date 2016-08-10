/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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
#include "../../AccessClass.h"
#include "../PluginManager/PluginManager.h"


FileReader::FileReader()
    : GenericProcessor ("File Reader")
    , timestamp             (0)
    , currentSampleRate     (0)
    , currentNumChannels    (0)
    , currentSample         (0)
    , currentNumSamples     (0)
    , startSample           (0)
    , stopSample            (0)
    , counter               (0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    setEnabledState (false);

    const int numFileSources = AccessClass::getPluginManager()->getNumFileSources();
    for (int i = 0; i < numFileSources; ++i)
    {
        Plugin::FileSourceInfo info = AccessClass::getPluginManager()->getFileSourceInfo (i);

        StringArray extensions;
        extensions.addTokens (info.extensions, ";", "\"");

        const int numExtensions = extensions.size();
        for (int j = 0; j < numExtensions; ++j)
        {
            supportedExtensions.set (extensions[j].toLowerCase(), i + 1);
        }
    }
}


FileReader::~FileReader()
{
}


AudioProcessorEditor* FileReader::createEditor()
{
    editor = new FileReaderEditor (this, true);

    return editor;
}


bool FileReader::isReady()
{
    if (! input)
    {
        CoreServices::sendStatusMessage ("No file selected in File Reader.");
        return false;
    }
    else
    {
        return input->isReady();
    }
}


float FileReader::getDefaultSampleRate() const
{
    if (input)
        return currentSampleRate;
    else
        return 44100.0;
}


int FileReader::getNumHeadstageOutputs() const
{
    if (input)
        return currentNumChannels;
    else
        return 16;
}


int FileReader::getNumEventChannels() const
{
    return 8;
}


float FileReader::getBitVolts (Channel* chan) const
{
    if (input)
        return chan->bitVolts;
    else
        return 0.05f;
}


void FileReader::setEnabledState (bool t)
{
    isEnabled = t;
}


bool FileReader::isFileSupported (const String& fileName) const
{
    const File file (fileName);
    String ext = file.getFileExtension().toLowerCase().substring (1);

    return isFileExtensionSupported (ext);
}


bool FileReader::isFileExtensionSupported (const String& ext) const
{
    const int index = supportedExtensions[ext] - 1;
    const bool isExtensionSupported = index >= 0;

    return isExtensionSupported;
}


bool FileReader::setFile (String fullpath)
{
    File file (fullpath);

    String ext = file.getFileExtension().toLowerCase().substring (1);
    const int index = supportedExtensions[ext] - 1;
    const bool isExtensionSupported = index >= 0;

    if (isExtensionSupported)
    {
        const int index = supportedExtensions[ext] - 1;
        Plugin::FileSourceInfo sourceInfo = AccessClass::getPluginManager()->getFileSourceInfo (index);
        input = sourceInfo.creator();
    }
    else
    {
        CoreServices::sendStatusMessage ("File type not supported");
        return false;
    }

    if (! input->OpenFile (file))
    {
        input = nullptr;
        CoreServices::sendStatusMessage ("Invalid file");

        return false;
    }

    const bool isEmptyFile = input->getNumRecords() <= 0;
    if (isEmptyFile)
    {
        input = nullptr;
        CoreServices::sendStatusMessage ("Empty file. Inoring open operation");

        return false;
    }

    static_cast<FileReaderEditor*> (getEditor())->populateRecordings (input);
    setActiveRecording (0);

    return true;
}


void FileReader::setActiveRecording (int index)
{
    input->setActiveRecord (index);

    currentNumChannels  = input->getActiveNumChannels();
    currentNumSamples   = input->getActiveNumSamples();
    currentSampleRate   = input->getActiveSampleRate();

    currentSample   = 0;
    startSample     = 0;
    stopSample      = currentNumSamples;

    for (int i = 0; i < currentNumChannels; ++i)
    {
        channelInfo.add (input->getChannelInfo (i));
    }

    static_cast<FileReaderEditor*> (getEditor())->setTotalTime (samplesToMilliseconds (currentNumSamples));

    readBuffer.malloc (currentNumChannels * BUFFER_SIZE);
}


String FileReader::getFile() const
{
    if (input)
        return input->getFileName();
    else
        return String::empty;
}


void FileReader::updateSettings()
{
    // if (!input) return;

    // for (int i=0; i < currentNumChannels; i++)
    // {
    //     channels[i]->bitVolts = channelInfo[i].bitVolts;
    //     channels[i]->name = channelInfo[i].name;
    // }
}


void FileReader::process (AudioSampleBuffer& buffer, MidiBuffer& events)
{
    setTimestamp (events, timestamp);

    const int samplesNeeded = int (float (buffer.getNumSamples()) * (getDefaultSampleRate() / 44100.0f));
    // FIXME: needs to account for the fact that the ratio might not be an exact
    //        integer value

    int samplesRead = 0;

    while (samplesRead < samplesNeeded)
    {
        int samplesToRead = samplesNeeded - samplesRead;
        if ( (currentSample + samplesToRead) > stopSample)
        {
            samplesToRead = stopSample - currentSample;
            if (samplesToRead > 0)
                input->readData (readBuffer + samplesRead, samplesToRead);

            input->seekTo (startSample);
            currentSample = startSample;
        }
        else
        {
            input->readData (readBuffer + samplesRead, samplesToRead);

            currentSample += samplesToRead;
        }

        samplesRead += samplesToRead;
    }

    for (int i = 0; i < currentNumChannels; ++i)
    {
        input->processChannelData (readBuffer, buffer.getWritePointer (i, 0), i, samplesNeeded);
    }

    timestamp += samplesNeeded;
    setNumSamples (events, samplesNeeded);

    // code for testing events:
    // // // ===========================================================================

    // if (counter == 100)
    // {
    //     //std::cout << "Adding on event for node id: " << nodeId << std::endl;
    //     addEvent (events,    // MidiBuffer
    //               TTL,       // eventType
    //               0,         // sampleNum
    //               1,         // eventID
    //               0);        // eventChannel
    //     ++counter;
    // } 
    // else if (counter > 102)
    // {
    //     //std::cout << "Adding off event!" << std::endl;
    //     addEvent (events,    // MidiBuffer
    //               TTL,       // eventType
    //               0,         // sampleNum
    //               0,         // eventID
    //               0);        // eventChannel
    //     counter = 0;
    // }
    // else 
    // {
    //     ++counter;
    // }
    // // // ===========================================================================
}


void FileReader::setParameter (int parameterIndex, float newValue)
{
    switch (parameterIndex)
    {
        //Change selected recording
        case 0:
            setActiveRecording (newValue);
            break;

        //set startTime
        case 1: 
            startSample = millisecondsToSamples (newValue);
            currentSample = startSample;

            static_cast<FileReaderEditor*> (getEditor())->setCurrentTime (samplesToMilliseconds (currentSample));
            break;

        //set stop time
        case 2:
            stopSample = millisecondsToSamples(newValue);
            currentSample = startSample;

            static_cast<FileReaderEditor*> (getEditor())->setCurrentTime (samplesToMilliseconds (currentSample));
            break;
    }
}


unsigned int FileReader::samplesToMilliseconds (int64 samples) const
{
    return (unsigned int) (1000.f * float (samples) / currentSampleRate);
}


int64 FileReader::millisecondsToSamples (unsigned int ms) const
{
    return (int64) (currentSampleRate * float (ms) / 1000.f);
}
