/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "FileReaderActions.h"

#include "../../AccessClass.h"
#include "../../Audio/AudioComponent.h"
#include "../PluginManager/PluginManager.h"
#include "BinaryFileSource/BinaryFileSource.h"
#include <stdio.h>

#include "../Settings/DataStream.h"
#include "../Settings/DeviceInfo.h"

#include "../Events/Event.h"

FileReader::FileReader() : GenericProcessor ("File Reader"),
                           Thread ("filereader_Async_Reader"),
                           playbackSamplePos (0),
                           currentSampleRate (0),
                           currentNumChannels (0),
                           currentSample (0),
                           currentNumTotalSamples (0),
                           startSample (0),
                           stopSample (0),
                           bufferCacheWindow (0),
                           m_shouldFillBackBuffer (false),
                           m_bufferSize (1024),
                           m_sysSampleRate (44100),
                           playbackActive (true),
                           gotNewFile (true),
                           loopPlayback (true),
                           sampleRateWarningShown (false),
                           firstProcess (false)
{
    /* Define a default file location based on OS */
#ifdef __APPLE__
    defaultFile = File::getSpecialLocation (File::currentApplicationFile).getChildFile ("Contents/Resources/resources").getChildFile ("structure.oebin");
#else
    defaultFile = File::getSpecialLocation (File::currentApplicationFile).getParentDirectory().getChildFile ("resources").getChildFile ("structure.oebin");
#endif

    /* Create a File Reader device */
    DeviceInfo::Settings settings {
        "File Reader",
        "description",
        "identifier",
        "00000x003",
        "Open Ephys"
    };
    devices.add (new DeviceInfo (settings));

    isEnabled = false;
}

FileReader::~FileReader()
{
    signalThreadShouldExit();
    notify();
}

void FileReader::registerParameters()
{
    /* Add parameters */
    addPathParameter (Parameter::PROCESSOR_SCOPE, "selected_file", "Selected File", "Select a file to load data from", defaultFile, getSupportedExtensions(), false, true);
    addSelectedStreamParameter (Parameter::PROCESSOR_SCOPE, "active_stream", "Active Stream", "Currently active stream", {"example_data"}, 0);
    addTimeParameter (Parameter::PROCESSOR_SCOPE, "start_time", "Start Time", "Time to start playback", "00:00:00.000");
    addTimeParameter (Parameter::PROCESSOR_SCOPE, "end_time", "Stop Time", "Time to end playback", "00:00:04.999");

    /* Link parameters */
    PathParameter* fileParam = static_cast<PathParameter*>(getParameter("selected_file"));
    CategoricalParameter* streamParam = static_cast<CategoricalParameter*>(getParameter("active_stream"));
    TimeParameter* startTimeParam = static_cast<TimeParameter*>(getParameter("start_time"));
    TimeParameter* endTimeParam = static_cast<TimeParameter*>(getParameter("end_time"));

    Array<Parameter*> linkedParams = {streamParam, startTimeParam, endTimeParam};
    fileParam->linkParameters(linkedParams);
}

void FileReader::parameterValueChanged (Parameter* p)
{
    if (p->getName() == "selected_file")
    {
        setFile (p->getValue(), false);
    }
    else if (p->getName() == "active_stream")
    {
        bool resetPlayback = false;
        setActiveStream (p->getValue(), resetPlayback);
    }
    else if (p->getName() == "start_time")
    {
        if (! input)
            return;

        TimeParameter* tp = static_cast<TimeParameter*> (p);
        startSample = millisecondsToSamples (tp->getTimeValue()->getTimeInMilliseconds());

        TimeParameter* endTime = static_cast<TimeParameter*> (getParameter ("end_time"));
        endTime->getTimeValue()->setMinTimeInMilliseconds (samplesToMilliseconds (startSample + 1));
        
        setPlaybackStart(startSample);
    }
    else if (p->getName() == "end_time")
    {
        if (! input)
            return;

        TimeParameter* tp = static_cast<TimeParameter*> (p);
        stopSample = millisecondsToSamples (tp->getTimeValue()->getTimeInMilliseconds());
        
        if (input != nullptr && stopSample == startSample)
        {
            stopSample = input->getActiveNumSamples();
            String newTime = TimeParameter::TimeValue (1000 * stopSample / input->getActiveSampleRate()).toString();
            p->setNextValue (newTime, false);
        }
        
        TimeParameter* startTime = static_cast<TimeParameter*> (getParameter ("start_time"));
        startTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (stopSample - 1));
        
        setPlaybackStop(stopSample);
    }

    currentNumTotalSamples = stopSample - startSample;

    if (! headlessMode)
    {
        static_cast<FileReaderEditor*> (getEditor())->enableScrubDrawer (true);

        if (input != nullptr)
        {
            getScrubberInterface()->updatePlaybackTimes();
            getScrubberInterface()->update();
        }
    }
}

void FileReader::handleLinkedParameterChange (Parameter* param, var newValue)
{
    if (param->getName() == "selected_file")
    {
        // Parent parameter 
        PathParameter* pathParam = static_cast<PathParameter*> (param);

        // Linked parameters
        TimeParameter* startTime = static_cast<TimeParameter*>(getParameter("start_time"));
        TimeParameter* endTime = static_cast<TimeParameter*>(getParameter("end_time"));
        CategoricalParameter* activeStream = static_cast<CategoricalParameter*>(getParameter("active_stream"));

        // Linked action
        SelectFile* action = new SelectFile (this, pathParam, newValue, activeStream, startTime, endTime);

        // Check if selected file is a valid file by checking if it exists and size > 0
        if (! File(newValue).exists() || File(newValue).getSize() == 0)
        {
            CoreServices::sendStatusMessage ("Invalid file: File doesn't exist or is empty");
            return;
        }

        CoreServices::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        CoreServices::getUndoManager()->perform ((UndoableAction*) action);
    }
}

AudioProcessorEditor* FileReader::createEditor()
{
    editor = std::make_unique<FileReaderEditor> (this);

    return editor.get();
}

void FileReader::initialize (bool signalChainIsLoading)
{
    if (signalChainIsLoading)
        return;

    if (isEnabled)
        return;

    if (! defaultFile.exists())
    {
        CoreServices::sendStatusMessage ("File Reader: default file not found");
        return;
    }

    //setFile (defaultFile.getFullPathName(), false);
    input.reset (createBuiltInFileSource (0));
    input->openFile (defaultFile.getFullPathName());
    setActiveStream (0, true);
    setPlaybackStart (0);
    setPlaybackStop (input->getActiveNumSamples());
    setCurrentSample (0);

    TimeParameter* startTime = static_cast<TimeParameter*> (getParameter ("start_time"));
    startTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (input->getActiveNumSamples() - 1));

    TimeParameter* endTime = static_cast<TimeParameter*> (getParameter ("end_time"));
    endTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (input->getActiveNumSamples()));
}

bool FileReader::setFile (String fullpath, bool shouldUpdateSignalChain)
{
    if (fullpath.equalsIgnoreCase ("default") || fullpath.equalsIgnoreCase ("None"))
    {
        File executable = File::getSpecialLocation (File::currentApplicationFile);

#ifdef __APPLE__
        File defaultFile = executable.getChildFile ("Contents/Resources/resources").getChildFile ("structure.oebin");
#else
        File defaultFile = executable.getParentDirectory().getChildFile ("resources").getChildFile ("structure.oebin");
#endif

        if (defaultFile.exists())
        {
            fullpath = defaultFile.getFullPathName();
        }
    }

    //Open file
    File file (fullpath);

    String ext = file.getFileExtension().toLowerCase().substring (1);
    const int index = supportedExtensions[ext];
    const bool isExtensionSupported = index >= 0;

    if (isExtensionSupported)
    {
        const int numPluginFileSources = AccessClass::getPluginManager()->getNumFileSources();

        if (index > 1)
        {
            Plugin::FileSourceInfo sourceInfo = AccessClass::getPluginManager()->getFileSourceInfo (index - 2);
            input.reset (sourceInfo.creator());
        }
        else
        {
            input.reset (createBuiltInFileSource (0));
        }
        if (! input)
        {
            LOGE ("Error creating file source for extension ", ext);
            return false;
        }
    }
    else
    {
        input = nullptr;
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

    gotNewFile = true;

    //Set available streams
    Array<String> streamNames;
    for (int i = 0; i < input->getNumRecords(); ++i)
        streamNames.add (input->getRecordName (i));

    SelectedStreamParameter* activeStreamParam = (SelectedStreamParameter*) getParameter ("active_stream");
    activeStreamParam->setStreamNames (streamNames);
    activeStreamParam->setNextValue (0, false);

    //Set active stream
    setActiveStream (0, true);

    //Set max time on time parameters
    TimeParameter* startTime = static_cast<TimeParameter*> (getParameter ("start_time"));
    startTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (currentNumTotalSamples - 1));

    TimeParameter* endTime = static_cast<TimeParameter*> (getParameter ("end_time"));
    endTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (currentNumTotalSamples));

    //Set initial values on time parameters
    endTime->setNextValue (TimeParameter::TimeValue (1000 * stopSample / input->getActiveSampleRate()).toString(), false);

    firstProcess = false;

    return true;
}

void FileReader::setActiveStream (int index, bool reset)
{
    //Resets the stream to the beginning if reset flag is true
    //If reset true, this means sample rate / num channels / num samples could all have changed

    if (! input)
    {
        return;
    }

    input->setActiveRecord (index);

    currentNumChannels = input->getActiveNumChannels();
    currentNumTotalSamples = input->getActiveNumSamples();
    currentSampleRate = input->getActiveSampleRate();

    /*
    TimeParameter* startTime = static_cast<TimeParameter*> (getParameter ("start_time"));
    startTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (currentNumTotalSamples));

    TimeParameter* endTime = static_cast<TimeParameter*> (getParameter ("end_time"));
    endTime->getTimeValue()->setMaxTimeInMilliseconds (samplesToMilliseconds (currentNumTotalSamples));
    */

    //Check if currentSample is within bounds of new stream
    if (currentSample > currentNumTotalSamples)
        reset = true;

    stopSample = currentNumTotalSamples;

    if (reset)
    {
        startSample = 0;
        bufferCacheWindow = 0;

        /*
        startTime->getTimeValue()->setTimeFromMilliseconds (0);
        startTime->setNextValue (startTime->getTimeValue()->toString(), false);

        endTime->getTimeValue()->setTimeFromMilliseconds (samplesToMilliseconds (currentNumTotalSamples));
        endTime->setNextValue (endTime->getTimeValue()->toString(), false);
        */
    }

    channelInfo.clear();
    for (int i = 0; i < currentNumChannels; ++i)
        channelInfo.add (input->getChannelInfo (index, i));

    input->seekTo (startSample);

    updateSettings();
    CoreServices::updateSignalChain (this);
}

void FileReader::togglePlayback()
{
    playbackActive = ! playbackActive;
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
    if (! isEnabled)
        return false;

    checkAudioDevice();

    /* Start asynchronous file reading thread */
    startThread();

    return true;
}

bool FileReader::stopAcquisition()
{
    stopThread (500);

    return true;
}

bool FileReader::isFileSupported (const String& fileName) const
{
    const File file (fileName);
    String ext = file.getFileExtension().toLowerCase().substring (1);

    return supportedExtensions[ext] - 1 >= 0;
}

int64 FileReader::getCurrentSample()
{
    return currentSample;
}

void FileReader::setCurrentSample(int64 sampleNumber)
{
    // Stop background thread before modifying shared state
    stopThread(100);

    const ScopedLock sl(bufferLock);

    currentSample = sampleNumber;
    playbackSamplePos.set(sampleNumber);

    // Reset file position
    input->seekTo(sampleNumber);

    // Get the current back buffer without switching
    HeapBlock<float>* backBuffer = getBackBuffer();

    // Fill only the back buffer first
    readAndFillBufferCache(*backBuffer);

    // Signal that we want to switch buffers on next process() call
    bufferCacheWindow.set(BUFFER_WINDOW_CACHE_SIZE - 1); // Force buffer switch on next process
    needsBufferReset.set(true);

    // The process() thread will handle the buffer switch and trigger
    // the background thread to fill the new back buffer
    m_shouldFillBackBuffer.set(false);

    // Restart background thread
    startThread();
}

void FileReader::setPlaybackStart (int64 startSample)
{
    this->startSample = startSample;
    setCurrentSample (startSample);
}

void FileReader::setPlaybackStop (int64 stopSample)
{
    this->stopSample = stopSample;
}

int64 FileReader::getPlayheadPosition()
{
    return playbackSamplePos.get();
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
    LOGD ("File Reader updating custom settings.");

    if (! input)
    {
        LOGD ("No input, returning.");
        isEnabled = false;
        return;
    }

    if (true)
    {
        LOGD ("File Reader got new file.");

        dataStreams.clear();
        continuousChannels.clear();
        eventChannels.clear();

        String streamName = input->getRecordName (input->getActiveRecord());

        /* Only use the original stream name (FileReader-100.example_data -> example_data) */
        StringArray tokens;
        tokens.addTokens (input->getRecordName (input->getActiveRecord()), ".");
        if (tokens.size())
            streamName = tokens[tokens.size() - 1];

        DataStream::Settings streamSettings {

            streamName,
            "A description of the File Reader Stream",
            "identifier",
            getDefaultSampleRate()

        };

        LOGD ("File Reader adding data stream.");

        dataStreams.add (new DataStream (streamSettings));
        dataStreams.getLast()->addProcessor (this);

        for (int i = 0; i < currentNumChannels; i++)
        {
            ContinuousChannel::Settings channelSettings {
                static_cast<ContinuousChannel::Type>(channelInfo[i].type),
                channelInfo[i].name,
                "description",
                "filereader.stream",
                channelInfo[i].bitVolts, // BITVOLTS VALUE

                dataStreams.getLast()
            };

            continuousChannels.add (new ContinuousChannel (channelSettings));
            continuousChannels.getLast()->addProcessor (this);
        }

        EventChannel* events;

        EventChannel::Settings eventSettings {
            EventChannel::Type::TTL,
            "All TTL events",
            "All TTL events loaded for the current input data source",
            "filereader.events",
            dataStreams.getLast()
        };

        //FIXME: Should add an event channel for each event channel detected in the current file source
        events = new EventChannel (eventSettings);
        String id = "sourceevent";
        events->setIdentifier (id);
        events->addProcessor (this);
        eventChannels.add (events);

        gotNewFile = false;
    }
    else
    {
        LOGD ("File Reader has no new file...not updating.");
    }

    isEnabled = true;

    /* Setup internal buffer based on audio device settings */
    AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
    AudioDeviceManager::AudioDeviceSetup ads;
    adm.getAudioDeviceSetup (ads);
    m_sysSampleRate = ads.sampleRate;
    m_bufferSize = ads.bufferSize;

    if (m_sysSampleRate < 44100.0)
    {
        if (! sampleRateWarningShown)
        {
            LOGE ("File Reader: Sample rate is less than 44100 Hz. Disabling processor.");

            if (! headlessMode)
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             "[File Reader] Invalid Sample Rate",
                                             "The sample rate of the audio device is less than 44.1 kHz. File Reader is disabled."
                                             "\n\nTry adjusting the sample rate in the audio settings by clicking the 'Latency' button in the Control Panel "
                                             "and setting it to 44.1 kHz or higher. After making this change, please remove and re-add the File Reader to the signal chain.");
            }

            sampleRateWarningShown = true;
        }

        isEnabled = false;
        return;
    }

    if (m_bufferSize == 0)
        m_bufferSize = 1024;

    m_samplesPerBuffer.set (m_bufferSize * (getDefaultSampleRate() / m_sysSampleRate));

    bufferA.malloc (currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);
    bufferB.malloc (currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);

    /* Reset stream to start of playback */
    input->seekTo (startSample);
    currentSample = startSample;

    /* Pre-fills the front buffer with a blocking read */
    readAndFillBufferCache (bufferA);

    readBuffer = &bufferB;
    bufferCacheWindow = 0;
    m_shouldFillBackBuffer.set (false);
    if (firstProcess)
        switchBuffer();

    LOGD ("File Reader finished updating custom settings.");
}

void FileReader::checkAudioDevice()
{
    /* Setup internal buffer based on audio device settings */
    AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
    AudioDeviceManager::AudioDeviceSetup ads;
    adm.getAudioDeviceSetup (ads);
    if (ads.sampleRate != m_sysSampleRate || ads.bufferSize != m_bufferSize)
    {
        m_sysSampleRate = ads.sampleRate;

        m_bufferSize = ads.bufferSize;
        if (m_bufferSize == 0)
            m_bufferSize = 1024;
        m_samplesPerBuffer.set (m_bufferSize * (getDefaultSampleRate() / m_sysSampleRate));

        bufferA.malloc (currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);
        bufferB.malloc (currentNumChannels * m_bufferSize * BUFFER_WINDOW_CACHE_SIZE);

        /* Reset stream to start of playback */
        input->seekTo (startSample);
        currentSample = startSample;

        /* Pre-fills the front buffer with a blocking read */
        readAndFillBufferCache (bufferA);

        readBuffer = &bufferB;
        bufferCacheWindow = 0;
        m_shouldFillBackBuffer.set (false);
    }
}

int64 FileReader::getPlaybackStart()
{
    return startSample;
}

int64 FileReader::getPlaybackStop()
{
    return stopSample;
}

Array<EventInfo> FileReader::getActiveEventInfo()
{
    return input->getEventInfo();
}

String FileReader::handleConfigMessage (const String& msg)
{
    //TODO: Needs update to use new parameters

    /*
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
    */

    return "File Reader received config: " + msg;
}

void FileReader::process (AudioBuffer<float>& buffer)
{
    const ScopedLock sl(bufferLock);

    if (needsBufferReset.compareAndSetBool(false, true))
    {
        // Switch to the newly prepared back buffer
        switchBuffer();
        bufferCacheWindow.set(0);
    }

    int samplesNeededPerBuffer = int(float(buffer.getNumSamples()) * (getDefaultSampleRate() / m_sysSampleRate));
    m_samplesPerBuffer.set(samplesNeededPerBuffer);

    // Handle buffer switching
    if (firstProcess && bufferCacheWindow.get() == 0)
    {
        switchBuffer();
    }

    // Get current buffer position
    const float* tempReadBuffer = readBuffer->getData() + 
        (samplesNeededPerBuffer * currentNumChannels * bufferCacheWindow.get());

    // Copy data to output buffer
    for (int ch = 0; ch < currentNumChannels; ++ch)
    {
        float* writeBuffer = buffer.getWritePointer(ch);
        for (int sample = 0; sample < samplesNeededPerBuffer; sample++)
        {
            *(writeBuffer + sample) = *(tempReadBuffer + (currentNumChannels * sample) + ch);
        }
    }

    // Update timestamps and sample positions atomically
    int64 start = playbackSamplePos.get();
    playbackSamplePos.set(start + samplesNeededPerBuffer);
    int64 stop = playbackSamplePos.get();

    setTimestampAndSamples(start, -1.0, samplesNeededPerBuffer, dataStreams[0]->getStreamId());

    // Handle looping
    if (playbackSamplePos.get() >= stopSample)
    {
        playbackSamplePos.set(startSample + (playbackSamplePos.get() - stopSample));
    }

    // Process events for this buffer
    addEventsInRange(start, stop);

    // Update buffer window counter
    int newWindow = (bufferCacheWindow.get() + 1) % BUFFER_WINDOW_CACHE_SIZE;
    bufferCacheWindow.set(newWindow);

    if (!firstProcess)
        firstProcess = true;
}

void FileReader::addEventsInRange (int64 start, int64 stop)
{
    EventInfo events;
    input->processEventData (events, start, stop);

    for (int i = 0; i < events.channels.size(); i++)
    {
        int64 absoluteCurrentSampleNumber = events.sampleNumbers[i];
        if (events.text.size() && ! events.text[i].isEmpty())
        {
            String msg = events.text[i];
            LOGD ("File read broadcasting message: ", msg, " at sample number: ", absoluteCurrentSampleNumber, " channel: ", events.channels[i]);
            broadcastMessage (msg);
        }
        else
        {
            uint8 ttlBit = events.channels[i];
            bool state = events.channelStates[i] > 0;
            TTLEventPtr event = TTLEvent::createTTLEvent (eventChannels[0], events.sampleNumbers[i], ttlBit, state);
            addEvent (event, int (absoluteCurrentSampleNumber));
        }
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
    const ScopedLock sl(bufferLock);

    if (readBuffer == &bufferA)
        readBuffer = &bufferB;
    else
        readBuffer = &bufferA;

    m_shouldFillBackBuffer.set(true);
    notify();
}

HeapBlock<float>* FileReader::getFrontBuffer()
{
    return readBuffer;
}

HeapBlock<float>* FileReader::getBackBuffer()
{
    if (readBuffer == &bufferA)
        return &bufferB;

    return &bufferA;
}

void FileReader::run()
{
    while (! threadShouldExit())
    {
        if (m_shouldFillBackBuffer.compareAndSetBool (false, true))
        {
            readAndFillBufferCache (*getBackBuffer());
        }

        wait (30);
    }
}

void FileReader::readAndFillBufferCache (HeapBlock<float>& cacheBuffer)
{
    const int samplesNeededPerBuffer = m_samplesPerBuffer.get();
    const int samplesNeeded = samplesNeededPerBuffer * BUFFER_WINDOW_CACHE_SIZE;

    int samplesRead = 0;

    // should only loop if reached end of file and resuming from start
    while (samplesRead < samplesNeeded)
    {
        int samplesToRead = samplesNeeded - samplesRead;

        // if reached end of file stream
        if ((currentSample + samplesToRead) > stopSample)
        {
            samplesToRead = int (stopSample - currentSample);
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

        //LOGD("CURRENT SAMPLE: ", currentSample, " samplesRead: ", samplesRead, " samplesNeeded: ", samplesNeeded);

        if (samplesRead < 0)
            return;
    }
}

StringArray FileReader::getSupportedExtensions()
{
    if (supportedExtensions.size() == 0)
    {
        /* Add built-in file source (Binary Format) */
        supportedExtensions.set ("oebin", 1);

        /* Load any plugin file sources */
        const int numFileSources = AccessClass::getPluginManager()->getNumFileSources();

        LOGD ("Found ", numFileSources, " File Source plugins.");

        for (int i = 0; i < numFileSources; ++i)
        {
            Plugin::FileSourceInfo info = AccessClass::getPluginManager()->getFileSourceInfo (i);

            LOGD ("Plugin ", i + 1, ": ", info.name, " (", info.extensions, ")");

            StringArray extensions;
            extensions.addTokens (info.extensions, ";", "\"");

            const int numExtensions = extensions.size();

            for (int j = 0; j < numExtensions; ++j)
            {
                supportedExtensions.set (extensions[j].toLowerCase(), i + 2);
            }
        }
    }
    StringArray extensions;
    HashMap<String, int>::Iterator i (supportedExtensions);
    while (i.next())
    {
        extensions.add (i.getKey());
    }
    return extensions;
}

String FileReader::getBuiltInFileSourceExtensions (int index) const
{
    switch (index)
    {
        case 0: //Binary
            return "oebin";
        default:
            return "";
    }
}

FileSource* FileReader::createBuiltInFileSource (int index) const
{
    switch (index)
    {
        case 0:
            return new BinarySource::BinaryFileSource();
        default:
            return nullptr;
    }
}

ScrubberInterface* FileReader::getScrubberInterface()
{
    return ((FileReaderEditor*) getEditor())->getScrubberInterface();
}

void FileReader::saveCustomParametersToXml (XmlElement* xml)
{
    XmlElement* childNode = xml->createNewChildElement ("SCRUBBERINTERFACE");

    if (headlessMode)
        childNode->setAttribute ("show", "false");
    else
        childNode->setAttribute ("show", getScrubberInterface()->isVisible() ? "true" : "false");
}

void FileReader::loadCustomParametersFromXml (XmlElement* xml)
{
    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName ("SCRUBBERINTERFACE") && ! headlessMode)
        {
            static_cast<FileReaderEditor*> (getEditor())->showScrubInterface (element->getBoolAttribute ("show"));
        }
    }
}
