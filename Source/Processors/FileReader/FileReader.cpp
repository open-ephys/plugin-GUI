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

#include "../Settings/DeviceInfo.h"
#include "../Settings/DataStream.h"

#include "../Events/Event.h"

FileReader::FileReader() : GenericProcessor ("File Reader")
    , Thread ("filereader_Async_Reader")
    , totalSamplesAcquired      (0)
    , currentSampleRate         (0)
    , currentNumChannels        (0)
    , currentSample             (0)
    , currentNumTotalSamples    (0)
    , currentNumScrubbedSamples (0)
    , startSample               (0)
    , stopSample                (0)
    , loopCount                 (0)
    , bufferCacheWindow         (0)
    , m_shouldFillBackBuffer    (false)
	, m_bufferSize              (1024)
	, m_sysSampleRate           (44100)
    , playbackActive            (true)
    , gotNewFile                (true)
    , loopPlayback              (true)
{

	/* Load any plugin file sources */
    const int numFileSources = AccessClass::getPluginManager()->getNumFileSources();

    LOGD("Found ", numFileSources, " File Source plugins.");

    for (int i = 0; i < numFileSources; ++i)
    {
        Plugin::FileSourceInfo info = AccessClass::getPluginManager()->getFileSourceInfo (i);

        LOGD("Plugin ", i + 1, ": ", info.name, " (", info.extensions, ")");

        StringArray extensions;
        extensions.addTokens (info.extensions, ";", "\"");

        const int numExtensions = extensions.size();
        
        for (int j = 0; j < numExtensions; ++j)
        {
            supportedExtensions.set (extensions[j].toLowerCase(), i + 1);
        }
    }

	/* Load built-in file sources */
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

    /* Create a File Reader device */
    DeviceInfo::Settings settings {
        "File Reader",
        "description",
        "identifier",
        "00000x003",
        "Open Ephys"
    };
    devices.add(new DeviceInfo(settings));

    isEnabled = false;

}

FileReader::~FileReader()
{
    signalThreadShouldExit();
    notify();
}

AudioProcessorEditor* FileReader::createEditor()
{
    editor = std::make_unique<FileReaderEditor>(this);

    return editor.get();
}

void FileReader::initialize(bool signalChainIsLoading)
{

    LOGD("INITIALIZING FILE READER");

    if (signalChainIsLoading)
        return;

    if (isEnabled)
        return;

    LOGD("SETTING FILE");

    File executable = File::getSpecialLocation(File::currentApplicationFile);
#ifdef __APPLE__
    File defaultFile = executable.getChildFile("Contents/Resources/resources").getChildFile("structure.oebin");
#else
    File defaultFile = executable.getParentDirectory().getChildFile("resources").getChildFile("structure.oebin");
#endif

    if (defaultFile.exists())
    {
        FileReaderEditor* ed = (FileReaderEditor*)editor.get();
        ed->setFile("default", false);
    }
}

void FileReader::togglePlayback()
{
    playbackActive = !playbackActive;
}

bool FileReader::playbackIsActive()
{
    return playbackActive;
}

int64 FileReader::getCurrentNumTotalSamples()
{
    return currentNumTotalSamples;
}

float FileReader::getCurrentSampleRate() const
{
    return input->getActiveSampleRate();
}

float FileReader::getDefaultSampleRate() const
{
    if (input)
        return currentSampleRate;
    else
        return 44100.0;
}

bool FileReader::startAcquisition()
{

    if (!isEnabled)
        return false;

    static_cast<FileReaderEditor*> (getEditor())->startTimer(100);

    /* Start asynchronous file reading thread */
	startThread(); 

	return true;
}

bool FileReader::stopAcquisition()
{

	stopThread(500);
    static_cast<FileReaderEditor*> (getEditor())->stopTimer();
	return true;
}

bool FileReader::isFileSupported (const String& fileName) const
{
    const File file (fileName);
    String ext = file.getFileExtension().toLowerCase().substring (1);

    return supportedExtensions[ext] - 1 >= 0;
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
            LOGD("Found input.");
		}
		else
		{
			input = createBuiltInFileSource(index - numPluginFileSources);
            LOGD("Found input.");
		}
		if (!input)
		{
			LOGE("Error creating file source for extension ", ext);
			return false;
		}

    }
    else
    {
        CoreServices::sendStatusMessage ("File type not supported");
        return false;
    }

    if (! input->openFile (file))
    {
        input = nullptr;
        CoreServices::sendStatusMessage ("Invalid file");

        return false;
    }

    const bool isEmptyFile = input->getNumRecords() <= 0;
    if (isEmptyFile)
    {
        input = nullptr;
        CoreServices::sendStatusMessage ("Empty file. Ignoring open operation");

        return false;
    }

    static_cast<FileReaderEditor*> (getEditor())->populateRecordings (input);
    setActiveRecording (0);
    
    gotNewFile = true;

    return true;
}

void FileReader::setActiveRecording (int index)
{
    if (!input) { return; }

    input->setActiveRecord (index);

    currentNumChannels       = input->getActiveNumChannels();
    currentNumTotalSamples   = input->getActiveNumSamples();
    currentSampleRate        = input->getActiveSampleRate();

    currentSample   = 0;
    startSample     = 0;
    stopSample      = currentNumTotalSamples;
    bufferCacheWindow = 0;
    loopCount = 0;

    channelInfo.clear();

    for (int i = 0; i < currentNumChannels; ++i)
    {
           channelInfo.add (input->getChannelInfo (index, i));
    }

    static_cast<FileReaderEditor*> (getEditor())->setTotalTime (samplesToMilliseconds (currentNumTotalSamples));
	input->seekTo(startSample);
    
    gotNewFile = true;

   
}

int64 FileReader::getCurrentSample()
{
    return currentSample;
}

void FileReader::setPlaybackStart(int64 startSample)
{
    this->startSample = startSample;
    this->totalSamplesAcquired = startSample;

    input->seekTo(startSample);
    currentSample = startSample;

    switchBuffer();

    if (CoreServices::getAcquisitionStatus() && !isThreadRunning())
    {
        m_shouldFillBackBuffer.set(true);
        startThread();
    }
    
}

void FileReader::setPlaybackStop(int64 stopSample)
{
    this->stopSample = stopSample;
    currentNumScrubbedSamples = stopSample - startSample;
}

String FileReader::getFile() const
{
    if (input)
        return input->getFileName();
    else
        return String();
}

void FileReader::updateSettings()
{

    LOGD("File Reader updating custom settings.");

    if (!input)
    {
        LOGD("No input, returning.");
        isEnabled = false;
        return;
    }

    if (gotNewFile)
    {

        LOGD("File Reader got new file.");

        dataStreams.clear();
        continuousChannels.clear();
        eventChannels.clear();

        String streamName = input->getRecordName(input->getActiveRecord());

         /* Only use the original stream name (FileReader-100.example_data -> example_data) */
        StringArray tokens;
        tokens.addTokens (input->getRecordName(input->getActiveRecord()), ".");
        if ( tokens.size() )
            streamName = tokens[tokens.size()-1];

        DataStream::Settings streamSettings{

            streamName,
            "A description of the File Reader Stream",
            "identifier",
            getDefaultSampleRate()

        };

        LOGD("File Reader adding data stream.");

        dataStreams.add(new DataStream(streamSettings));
        dataStreams.getLast()->addProcessor(processorInfo.get());

        for (int i = 0; i < currentNumChannels; i++)
        {
            ContinuousChannel::Settings channelSettings
            {
                ContinuousChannel::Type::ELECTRODE,
                channelInfo[i].name,
                "description",
                "filereader.stream",
                channelInfo[i].bitVolts, // BITVOLTS VALUE
                dataStreams.getLast()
            };

            continuousChannels.add(new ContinuousChannel(channelSettings));
            continuousChannels.getLast()->addProcessor(processorInfo.get());
        }

        EventChannel* events;

        EventChannel::Settings eventSettings{
            EventChannel::Type::TTL,
            "All TTL events",
            "All TTL events loaded for the current input data source",
            "filereader.events",
            dataStreams.getLast()
        };

        //FIXME: Should add an event channel for each event channel detected in the current file source
        events = new EventChannel(eventSettings);
        String id = "sourceevent";
        events->setIdentifier(id);
        events->addProcessor(processorInfo.get());
        eventChannels.add(events);

        gotNewFile = false;

    }
    else {
        LOGD("File Reader has no new file...not updating.");
    }

    isEnabled = true;

    /* Set the timestamp to start of playback and reset loop counter */
    totalSamplesAcquired = startSample;
    loopCount = 0;

    /* Setup internal buffer based on audio device settings */
    AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
    AudioDeviceManager::AudioDeviceSetup ads;
    adm.getAudioDeviceSetup(ads);
    m_sysSampleRate = ads.sampleRate;
    m_bufferSize = ads.bufferSize;
    if (m_bufferSize == 0) m_bufferSize = 1024;
    m_samplesPerBuffer.set(m_bufferSize * (getDefaultSampleRate() / m_sysSampleRate));

    bufferA.malloc(currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);
    bufferB.malloc(currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);

    /* Reset stream to start of playback */
    input->seekTo(startSample);
    currentSample = startSample;

    /* Pre-fills the front buffer with a blocking read */
    readAndFillBufferCache(bufferA);

    readBuffer = &bufferB;
    bufferCacheWindow = 0;
    m_shouldFillBackBuffer.set(false);

}

int FileReader::getPlaybackStart() 
{
    return startSample;
}

int FileReader::getPlaybackStop()
{
    return stopSample;
}

Array<EventInfo> FileReader::getActiveEventInfo()
{
    return input->getEventInfo();
}

String FileReader::handleConfigMessage(String msg)
{

    const MessageManagerLock mml;

    StringArray tokens;
    tokens.addTokens (msg, "=", "\"");

    if (tokens.size() != 2) return "Invalid msg";

    if (tokens[0] == "file")
        static_cast<FileReaderEditor*> (getEditor())->setFile(tokens[1]);
    else if (tokens[0] == "index")
        static_cast<FileReaderEditor*> (getEditor())->setRecording(std::stoi(tokens[1].toStdString()));
    else if (tokens[0] == "start")
        static_cast<FileReaderEditor*> (getEditor())->setPlaybackStartTime(std::stoi(tokens[1].toStdString()));
    else if (tokens[0] == "stop")
        static_cast<FileReaderEditor*> (getEditor())->setPlaybackStartTime(std::stoi(tokens[1].toStdString()));
    else
        LOGD("Invalid key");

    return "File Reader received config: " + msg;
}

void FileReader::process(AudioBuffer<float>& buffer)
{

    bool switchNeeded = false;

    int samplesNeededPerBuffer = int (float (buffer.getNumSamples()) * (getDefaultSampleRate() / m_sysSampleRate));

    if (!playbackActive && totalSamplesAcquired + samplesNeededPerBuffer > stopSample)
    {
        samplesNeededPerBuffer = stopSample - totalSamplesAcquired;
        switchNeeded = true;
    }
    else
        m_samplesPerBuffer.set(samplesNeededPerBuffer);
    // FIXME: needs to account for the fact that the ratio might not be an exact
    //        integer value
    
    // if cache window id == 0, we need to read and cache BUFFER_WINDOW_CACHE_SIZE more buffer windows
    if (bufferCacheWindow == 0)
    {
        switchBuffer();
    }

    //std::cout << "Reading " << samplesNeededPerBuffer << " samples. " << std::endl;
    
    for (int i = 0; i < currentNumChannels; ++i)
    {
        // offset readBuffer index by current cache window count * buffer window size * num channels
        input->processChannelData (*readBuffer + (samplesNeededPerBuffer * currentNumChannels * bufferCacheWindow),
                                buffer.getWritePointer (i, 0),
                                i,
                                samplesNeededPerBuffer);
    }

    setTimestampAndSamples(totalSamplesAcquired, -1.0, samplesNeededPerBuffer, dataStreams[0]->getStreamId()); //TODO: Look at this

    int64 start = totalSamplesAcquired;

    totalSamplesAcquired += samplesNeededPerBuffer;

    //LOGD("Total samples acquired: ", totalSamplesAcquired);

    int64 stop = totalSamplesAcquired;

    addEventsInRange(start, stop);

    bufferCacheWindow += 1;
    bufferCacheWindow %= BUFFER_WINDOW_CACHE_SIZE;

    if (switchNeeded)
    {
        bufferCacheWindow = 0;
        this->stopThread(100);
    }

}

void FileReader::addEventsInRange(int64 start, int64 stop)
{

    EventInfo events;
    input->processEventData(events, start, stop);

    for (int i = 0; i < events.channels.size(); i++) 
    { 

        juce::int64 absoluteCurrentTimestamp = events.timestamps[i] + loopCount * (stopSample - startSample);
        String msg = events.text[i];
        if (!msg.isEmpty())
        {
            LOGD("Broadcasting message: ", msg, " at timestamp: ", absoluteCurrentTimestamp, " channel: ", events.channels[i]);
            broadcastMessage(msg);
        }
        else
        {
            uint8 ttlBit = events.channels[i];
            bool state = events.channelStates[i] > 0;
            TTLEventPtr event = TTLEvent::createTTLEvent(eventChannels[0], events.timestamps[i], ttlBit, state);
            addEvent(event, absoluteCurrentTimestamp); 
        }
    }
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
        
        if (samplesRead < 0)
            return;

        int samplesToRead = samplesNeeded - samplesRead;
        
        // if reached end of file stream
        if ( (currentSample + samplesToRead) > stopSample)
        {
            samplesToRead = stopSample - currentSample;
            if (samplesToRead > 0)
                input->readData (cacheBuffer + samplesRead * currentNumChannels, samplesToRead);

            if (startSample != 0)
            {
                startSample = 0;
            }
            
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
