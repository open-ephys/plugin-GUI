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
#include "../../Audio/AudioComponent.h"
#include "../PluginManager/PluginManager.h"
#include "BinaryFileSource/BinaryFileSource.h"


FileReader::FileReader()
    : GenericProcessor ("File Reader")
    , Thread ("filereader_Async_Reader")
    , timestamp             (0)
    , currentSampleRate     (0)
    , currentNumChannels    (0)
    , currentSample         (0)
    , currentNumSamples     (0)
    , startSample           (0)
    , stopSample            (0)
    , counter               (0)
    , bufferCacheWindow     (0)
    , m_shouldFillBackBuffer(false)
	, m_bufferSize(1024)
	, m_sysSampleRate(44100)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);

    setEnabledState (false);

	//Load pluIn file Sources
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

	//Load Built-in file Sources
	const int numBuiltInFileSources = getNumBuiltInFileSources();
	for (int i = 0; i < numBuiltInFileSources; ++i)
	{
		StringArray extensions;
		extensions.addTokens(getBuiltInFileSourceExtensions(i), ";", "\"");

		const int numExtensions = extensions.size();
		for (int j = 0; j < numExtensions; ++j)
		{
			supportedExtensions.set(extensions[j].toLowerCase(), i + numFileSources + 1);
		}

	}
}


FileReader::~FileReader()
{
    signalThreadShouldExit();
    notify();
}


AudioProcessorEditor* FileReader::createEditor()
{
    editor = new FileReaderEditor (this, true);

    return editor;
}

void FileReader::createEventChannels()
{
    //editor = new FileReaderEditor (this, true);

    //return editor;
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


int FileReader::getDefaultNumDataOutputs(DataChannel::DataChannelTypes type, int subproc) const
{
    if (subproc != 0) return 0;
    if (type != DataChannel::HEADSTAGE_CHANNEL) return 0;
    if (input)
        return currentNumChannels;
    else
        return 16;
}


float FileReader::getBitVolts (const DataChannel* chan) const
{
    if (input)
        return chan->getBitVolts();
    else
        return 0.05f;
}


void FileReader::setEnabledState (bool t)
{
    isEnabled = t;
}

bool FileReader::enable()
{
	timestamp = 0;

	AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
	AudioDeviceManager::AudioDeviceSetup ads;
	adm.getAudioDeviceSetup(ads);
	m_sysSampleRate = ads.sampleRate;
	m_bufferSize = ads.bufferSize;
	if (m_bufferSize == 0) m_bufferSize = 1024;

	m_samplesPerBuffer.set(m_bufferSize * (getDefaultSampleRate() / m_sysSampleRate));

	bufferA.malloc(currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);
	bufferB.malloc(currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);

        // reset stream to beginning
        input->seekTo (startSample);
        currentSample = startSample;
        readAndFillBufferCache(bufferA); // pre-fill the front buffer with a blocking read

	// set the backbuffer so that on the next call to process() we start with bufferA and buffer
	// cache window id = 0
	readBuffer = &bufferB;
	bufferCacheWindow = 0;
	m_shouldFillBackBuffer.set(false);

	startThread(); // start async file reader thread

	return isEnabled;
}

bool FileReader::disable()
{
	stopThread(100);
	return true;
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
        const int index = supportedExtensions[ext] -1 ;
		const int numPluginFileSources = AccessClass::getPluginManager()->getNumFileSources();

		if (index < numPluginFileSources)
		{
			Plugin::FileSourceInfo sourceInfo = AccessClass::getPluginManager()->getFileSourceInfo(index);
			input = sourceInfo.creator();
		}
		else
		{
			input = createBuiltInFileSource(index - numPluginFileSources);
		}
		if (!input)
		{
			std::cerr << "Error creating file source for extension " << ext << std::endl;
			return false;
		}

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
    if (!input) { return; }

    input->setActiveRecord (index);

    currentNumChannels  = input->getActiveNumChannels();
    currentNumSamples   = input->getActiveNumSamples();
    currentSampleRate   = input->getActiveSampleRate();

    currentSample   = 0;
    startSample     = 0;
    stopSample      = currentNumSamples;
    bufferCacheWindow = 0;

    for (int i = 0; i < currentNumChannels; ++i)
    {
        channelInfo.add (input->getChannelInfo (i));
    }

    static_cast<FileReaderEditor*> (getEditor())->setTotalTime (samplesToMilliseconds (currentNumSamples));
	input->seekTo(startSample);

   
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
     if (!input) return;

     for (int i=0; i < currentNumChannels; i++)
     {
         dataChannelArray[i]->setBitVolts(channelInfo[i].bitVolts);
         dataChannelArray[i]->setName(channelInfo[i].name);
     }
}

void FileReader::process (AudioSampleBuffer& buffer)
{
    const int samplesNeededPerBuffer = int (float (buffer.getNumSamples()) * (getDefaultSampleRate() / m_sysSampleRate));
    m_samplesPerBuffer.set(samplesNeededPerBuffer);
    // FIXME: needs to account for the fact that the ratio might not be an exact
    //        integer value
    
    // if cache window id == 0, we need to read and cache BUFFER_WINDOW_CACHE_SIZE more buffer windows
    if (bufferCacheWindow == 0)
    {
        switchBuffer();
    }
    
    for (int i = 0; i < currentNumChannels; ++i)
    {
        // offset readBuffer index by current cache window count * buffer window size * num channels
        input->processChannelData (*readBuffer + (samplesNeededPerBuffer * currentNumChannels * bufferCacheWindow),
                                   buffer.getWritePointer (i, 0),
                                   i,
                                   samplesNeededPerBuffer);
    }
    
    setTimestampAndSamples(timestamp, samplesNeededPerBuffer);
	timestamp += samplesNeededPerBuffer;

	static_cast<FileReaderEditor*> (getEditor())->setCurrentTime(samplesToMilliseconds(startSample + timestamp % (stopSample - startSample)));
    
    bufferCacheWindow += 1;
    bufferCacheWindow %= BUFFER_WINDOW_CACHE_SIZE;
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

void FileReader::switchBuffer()
{
    if (readBuffer == &bufferA)
        readBuffer = &bufferB;
    else
        readBuffer = &bufferA;
    
    m_shouldFillBackBuffer.set(true);
    notify();
}

HeapBlock<int16>* FileReader::getFrontBuffer()
{
    return readBuffer;
}

HeapBlock<int16>* FileReader::getBackBuffer()
{
    if (readBuffer == &bufferA) return &bufferB;
    
    return &bufferA;
}

void FileReader::run()
{
    while (!threadShouldExit())
    {
        if (m_shouldFillBackBuffer.compareAndSetBool(false, true))
        {
            readAndFillBufferCache(*getBackBuffer());
        }
        
        wait(30);
    }
}

void FileReader::readAndFillBufferCache(HeapBlock<int16> &cacheBuffer)
{
    const int samplesNeededPerBuffer = m_samplesPerBuffer.get();
    const int samplesNeeded = samplesNeededPerBuffer * BUFFER_WINDOW_CACHE_SIZE;
    
    int samplesRead = 0;
    
    // should only loop if reached end of file and resuming from start
    while (samplesRead < samplesNeeded)
    {
        int samplesToRead = samplesNeeded - samplesRead;
        
        // if reached end of file stream
        if ( (currentSample + samplesToRead) > stopSample)
        {
            samplesToRead = stopSample - currentSample;
            if (samplesToRead > 0)
                input->readData (cacheBuffer + samplesRead * currentNumChannels, samplesToRead);
            
            // reset stream to beginning
            input->seekTo (startSample);
            currentSample = startSample;
        }
        else // else read the block needed
        {
            input->readData (cacheBuffer + samplesRead * currentNumChannels, samplesToRead);
            
            currentSample += samplesToRead;
        }
        
        samplesRead += samplesToRead;
    }
}

StringArray FileReader::getSupportedExtensions() const
{
	StringArray extensions;
	HashMap<String, int>::Iterator i(supportedExtensions);
	while (i.next())
	{
		extensions.add(i.getKey());
	}
	return extensions;
}

//Built-In

int FileReader::getNumBuiltInFileSources() const
{
	return 1;
}

String FileReader::getBuiltInFileSourceExtensions(int index) const
{
	switch (index)
	{
	case 0: //Binary
		return "oebin";
	default:
		return "";
	}
}

FileSource* FileReader::createBuiltInFileSource(int index) const
{
	switch (index)
	{
	case 0:
		return new BinarySource::BinaryFileSource();
	default:
		return nullptr;
	}
}
