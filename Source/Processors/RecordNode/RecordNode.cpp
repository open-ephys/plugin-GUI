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

#include "RecordNode.h"

#include "../../AccessClass.h"
#include "../../Audio/AudioComponent.h"
#include "../../Processors/MessageCenter/MessageCenterEditor.h"
#include "../../UI/ControlPanel.h"
#include "../../UI/EditorViewport.h"
#include "BinaryFormat/BinaryRecording.h"

#include "../Events/Spike.h"
#include "../Settings/DataStream.h"
#include "../Settings/DeviceInfo.h"

using namespace std::chrono;

#define CONTINUOUS_CHANNELS_ON_BY_DEFAULT true
#define RECEIVED_SOFTWARE_TIME (event.getVelocity() == 136)

bool RecordNode::overrideTimestampWarningShown = false;

EventMonitor::EventMonitor()
    : receivedEvents (0),
      receivedSpikes (0),
      bufferedEvents (0),
      bufferedSpikes (0)
{
}

EventMonitor::~EventMonitor() {}

void EventMonitor::reset()
{
    receivedEvents = 0;
    receivedSpikes = 0;
    bufferedEvents = 0;
    bufferedSpikes = 0;
}

void EventMonitor::displayStatus()
{
    LOGD ("Record Node received ", receivedEvents, " total EVENTS and sent ", bufferedEvents, " to the RecordThread");

    LOGD ("Record Node received ", receivedSpikes, " total SPIKES and sent ", bufferedSpikes, " to the RecordThread");
}

RecordNode::RecordNode()
    : GenericProcessor ("Record Node"),
      newDirectoryNeeded (true),
      setFirstBlock (false),
      samplesWritten (0),
      experimentNumber (1), // 1-based indexing
      recordingNumber (0), // 0-based indexing
      isRecording (false),
      hasRecorded (false),
      settingsNeeded (false)
{
    // Note: DataQueues are created per-stream in startRecording()
    eventQueue = std::make_unique<EventMsgQueue> (EVENT_BUFFER_NEVENTS);
    spikeQueue = std::make_unique<SpikeMsgQueue> (SPIKE_BUFFER_NSPIKES);

    isSyncReady = true;

    /* New record nodes default to the record engine currently selected in the Control Panel */
    setEngine (CoreServices::getDefaultRecordEngineId());

    dataDirectory = CoreServices::getRecordingParentDirectory();

    recordThread = std::make_unique<RecordThread> (this, recordEngine.get());

    eventMonitor = new EventMonitor();

    diskSpaceChecker = std::make_unique<DiskSpaceChecker> (this);
}

RecordNode::~RecordNode()
{
}

void RecordNode::registerParameters()
{
    defaultRecordDirectory = CoreServices::getRecordingParentDirectory().getFullPathName();
    addPathParameter (Parameter::PROCESSOR_SCOPE, "directory", "Directory", "Select a directory to write data to", defaultRecordDirectory, {}, true, false);

    Array<String> recordEngines;
    std::vector<RecordEngineManager*> engines = getAvailableRecordEngines();
    for (int i = 0; i < engines.size(); i++)
        recordEngines.add (engines[i]->getName());
    addCategoricalParameter (Parameter::PROCESSOR_SCOPE, "engine", "Engine", "Recording data format", recordEngines, engineIndex, true);

    addBooleanParameter (Parameter::PROCESSOR_SCOPE, "events", "Record Events", "Toggle saving events coming into this node", true, true);
    addBooleanParameter (Parameter::PROCESSOR_SCOPE, "spikes", "Record Spikes", "Toggle saving spikes coming into this node", true, true);

    addMaskChannelsParameter (Parameter::STREAM_SCOPE, "channels", "Channels", "Channels to record from", true);
    addTtlLineParameter (Parameter::STREAM_SCOPE, "sync_line", "Sync Line", "Event line to use for sync signal", 8, true, false, true);
    addSelectedStreamParameter (Parameter::PROCESSOR_SCOPE, "main_sync", "Main Sync Stream", "Use this stream as main sync", {}, 0, false, true);
}

void RecordNode::initialize (bool signalChainIsLoading)
{
    if (! signalChainIsLoading)
        checkDiskSpace();

    CoreServices::createNewRecordingDirectory();
}

void RecordNode::parameterValueChanged (Parameter* p)
{
    if (p->getName() == "directory")
    {
        String newPath = static_cast<PathParameter*> (p)->getValue();
        if (newPath == "None")
            setDataDirectory (defaultRecordDirectory);
        else
            setDataDirectory (newPath);
    }
    else if (p->getName() == "engine")
    {
        int selectedIdx = ((CategoricalParameter*) p)->getSelectedIndex();
        setEngine (getAvailableRecordEngines()[selectedIdx]->getID());
        // NOTE: Any record engine change will create a new directory for all record nodes to prevent data loss in edge cases
        CoreServices::createNewRecordingDirectory();
    }
    else if (p->getName() == "events")
    {
        setRecordEvents (((BooleanParameter*) p)->getBoolValue());
    }
    else if (p->getName() == "spikes")
    {
        setRecordSpikes (((BooleanParameter*) p)->getBoolValue());
    }
    else if (p->getName() == "channels")
    {
        LOGD ("Parameter changed: channels");
    }
    else if (p->getName() == "sync_line")
    {
        LOGD ("Parameter changed: sync_line");
        int selectedLine = ((TtlLineParameter*) p)->getSelectedLine();
        const String streamKey = getDataStream (p->getStreamId())->getKey();

        synchronizer.setSyncLine (streamKey, selectedLine);

        // Assume overrideTimestampWarningShown is already true if a sync line is set for any hardware-synced stream
        if (getDataStream (streamKey)->generatesTimestamps()
            && selectedLine >= 0
            && ! overrideTimestampWarningShown)
        {
            overrideTimestampWarningShown = true;
        }

        // If sync line is set to none and this is the main stream
        if (selectedLine == -1 && synchronizer.mainStreamKey == streamKey)
        {
            // If stream does not generate timestamps, set ttl line to 0
            if (! getDataStream (streamKey)->generatesTimestamps())
            {
                p->setNextValue (0, false);
                return;
            }

            // Find a new main stream that is not hardware-synced
            int streamIndex = 0;
            for (auto stream : dataStreams)
            {
                if (stream->getStreamId() != p->getStreamId()
                    && ! synchronizer.streamGeneratesTimestamps (stream->getKey()))
                {
                    getParameter ("main_sync")->setNextValue (streamIndex, false);
                    return;
                }
                streamIndex++;
            }

            getParameter ("main_sync")->setNextValue (-1, false);
        }
        else if (selectedLine >= 0 && synchronizer.mainStreamKey.isEmpty())
        {
            // If the sync line is set, but no main stream is set, we need to set the main stream to the one with the sync line
            int streamIndex = 0;
            for (auto stream : dataStreams)
            {
                if (stream->getKey() == streamKey)
                {
                    getParameter ("main_sync")->setNextValue (streamIndex, false);
                    return;
                }
                streamIndex++;
            }
        }
    }
    else if (p->getName() == "main_sync")
    {
        LOGD ("Parameter changed: main_sync");
        int streamIndex = ((SelectedStreamParameter*) p)->getSelectedIndex();

        if (streamIndex == -1)
        {
            synchronizer.setMainDataStream ("");
            return;
        }
        else
        {
            Array<String> streamNames = ((SelectedStreamParameter*) p)->getStreamNames();
            for (auto stream : dataStreams)
            {
                String key = stream->getKey();
                if (key == streamNames[streamIndex]
                    && ! synchronizer.streamGeneratesTimestamps (key))
                {
                    synchronizer.setMainDataStream (stream->getKey());
                    break;
                }
            }
        }
    }
    else
    {
        LOGD ("RecordNode: unknown parameter changed: ", p->getName());
    }
}

void RecordNode::checkDiskSpace()
{
    float diskSpaceWarningThreshold = 5; //GB

    File dataDir (dataDirectory);
    int64 freeSpace = dataDir.getBytesFreeOnVolume();

    float availableBytes = freeSpace / pow (2, 30); //1 GB == 2^30 bytes

    if (availableBytes < diskSpaceWarningThreshold && ! isRecording)
    {
        String msg = "Record Node " + String (getNodeId());
        msg += "\n\n";
        msg += "Less than " + String (int (diskSpaceWarningThreshold)) + " GB of disk space available in:\n";
        msg += "\n";
        msg += String (dataDirectory.getFullPathName());
        msg += "\n\n";
        msg += "Recording may fail. Please free up space or change the recording directory.";

        if (headlessMode) // headless mode
        {
            LOGC (msg);
        }
        else
        {
            MessageManager::callAsync ([msg]
                                       { AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "WARNING", msg); });
        }
    }
}

String RecordNode::handleConfigMessage (const String& msg)
{
    /*
	Available messages:
	- engine=<engine_name> -- changes the record engine
	- SELECT <stream_index> NONE / ALL / <channels> -- selects which channels to record, e.g.:
		"SELECT 0 NONE" -- deselect all channels for stream 0
		"SELECT 1 1 2 3 4 5 6 7 8" -- select channels 1-8 for stream 1
	*/

    if (CoreServices::getAcquisitionStatus())
    {
        return "Cannot configure Record Node while acquisition is active.";
    }

    const MessageManagerLock mml;

    StringArray tokens;
    tokens.addTokens (msg, "=", "\"");

    LOGD ("Record Node received: ", tokens[0]);

    if (tokens[0] == "engine")
    {
        if (tokens.size() == 2)
        {
            RecordNodeEditor* ed = static_cast<RecordNodeEditor*> (getEditor());

            String engineValue = tokens[1];
            int engineIndex = -1;

            // Try to parse as an integer first
            if (engineValue.containsOnly ("0123456789"))
            {
                engineIndex = engineValue.getIntValue();
            }
            else // If not an integer, try to match by name
            {
                auto engines = getAvailableRecordEngines();
                for (int i = 0; i < engines.size(); ++i)
                {
                    if (engines[i]->getID().toLowerCase() == engineValue.toLowerCase())
                    {
                        engineIndex = i;
                        break;
                    }
                }

                if (engineIndex == -1)
                {
                    return "Record Node: unknown engine name \"" + engineValue + "\"";
                }
            }

            int numEngines = ((CategoricalParameter*) getParameter ("engine"))->getCategories().size();

            if (engineIndex >= 0 && engineIndex < numEngines)
            {
                getParameter ("engine")->setNextValue (engineIndex);
                return "Record Node: updated record engine to " + ((CategoricalParameter*) getParameter ("engine"))->getCategories()[engineIndex];
            }
            else
            {
                return "Record Node: invalid engine index (max = " + String (numEngines - 1) + ")";
            }
        }
        else
        {
            return "Record Node: invalid engine key";
        }
    }

    tokens.clear();
    tokens.addTokens (msg, " ", "");

    LOGD (tokens[0]);

    if (tokens[0] == "SELECT")
    {
        if (tokens.size() >= 3)
        {
            int streamIndex = tokens[1].getIntValue();
            uint16 streamId;
            //std::vector<bool> channelStates;
            Array<var> channelStates;

            int channelCount;

            if (streamIndex >= 0 && streamIndex < dataStreams.size())
            {
                streamId = dataStreams[streamIndex]->getStreamId();
                channelCount = dataStreams[streamIndex]->getChannelCount();
            }
            else
            {
                return "Record Node: Invalid stream index; max = " + String (dataStreams.size() - 1);
            }

            if (tokens[2] == "NONE")
            {
                //select no channels
                channelStates.clear();
            }
            else if (tokens[2] == "ALL")
            {
                //select all channels
                for (int i = 0; i < channelCount; i++)
                {
                    channelStates.add (i);
                }
            }
            else
            {
                Array<int> channels;

                for (int i = 2; i < tokens.size(); i++)
                {
                    int ch = tokens[i].getIntValue() - 1;
                    channels.add (ch);
                }

                //select some channels
                for (int i = 0; i < channelCount; i++)
                {
                    if (channels.contains (i))
                        channelStates.add (i);
                }
            }

            DataStream* stream = getDataStream (streamId);
            MaskChannelsParameter* maskChannels = (MaskChannelsParameter*) stream->getParameter ("channels");
            maskChannels->setNextValue (channelStates);
        }
        else
        {
            LOGD ("Record Node: invalid config message");
        }
    }

    return "Record Node received config: " + msg;
}

void RecordNode::handleBroadcastMessage (const String& msg, const int64 messageSystemTime)
{
    if (recordEvents && isRecording)
    {
        String streamKey = synchronizer.mainStreamKey;

        const DataStream* syncStream = getDataStream (streamKey);

        if (syncStream == nullptr)
        {
            syncStream = getDataStreams().getFirst();
        }

        int64 offsetMilliseconds = Time::currentTimeMillis() - messageSystemTime;

        int64 messageSampleNumber = getFirstSampleNumberForBlock (syncStream->getStreamId())
                                    - int64 (offsetMilliseconds * syncStream->getSampleRate() / 1000.0f);

        TextEventPtr event = TextEvent::createTextEvent (getMessageChannel(), messageSampleNumber, msg);

        double ts = -1.0;

        if (synchronizer.mainStreamKey.isEmpty())
        {
            ts = getFirstTimestampForBlock (syncStream->getStreamId())
                 + (messageSampleNumber - getFirstSampleNumberForBlock (syncStream->getStreamId())) / syncStream->getSampleRate();
        }
        else
        {
            ts = synchronizer.convertSampleNumberToTimestamp (synchronizer.mainStreamKey, messageSampleNumber);
        }

        event->setTimestampInSeconds (ts);

        size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

        HeapBlock<char> buffer (size);

        event->serialize (buffer, size);

        eventQueue->addEvent (EventPacket (buffer, int (size)), messageSampleNumber, -1);
    }
}

String RecordNode::getEngineId()
{
    return recordEngine->getEngineId();
}

void RecordNode::setEngine (String id)
{
    availableEngines = getAvailableRecordEngines();

    int foundIndex = 0;

    for (auto engine : availableEngines)
    {
        if (engine->getID().equalsIgnoreCase (id))
        {
            if (recordEngine.get() != nullptr)
            {
                if (recordEngine->getEngineId() != id)
                {
                    recordEngine.reset (engine->instantiateEngine());

                    if (recordThread != nullptr)
                        recordThread->setEngine (recordEngine.get());
                }
            }
            else
            {
                recordEngine.reset (engine->instantiateEngine());
            }
            engineIndex = foundIndex;
        }
        foundIndex += 1;
    }
}

std::vector<RecordEngineManager*> RecordNode::getAvailableRecordEngines()
{
    return CoreServices::getAvailableRecordEngines();
}

String RecordNode::generateDirectoryName()
{
    return AccessClass::getControlPanel()->getRecordingDirectoryName();
}

// called by FifoMonitor
File RecordNode::getDataDirectory()
{
    return dataDirectory;
}

// called by RecordNodeEditor
void RecordNode::setDataDirectory (File directory)
{
    dataDirectory = directory;
    newDirectoryNeeded = true;

    createNewDirectory();

    checkDiskSpace();
}

void RecordNode::setDefaultRecordingDirectory (File directory)
{
    defaultRecordDirectory = directory;

    Parameter* p = getParameter ("directory");

    if (p->getValueAsString() == "None")
    {
        setDataDirectory (directory);
    }
}

void RecordNode::createNewDirectory (bool resetCounters)
{
    LOGD ("CREATE NEW DIRECTORY");

    rootFolder = File (dataDirectory.getFullPathName()
                       + File::getSeparatorString()
                       + generateDirectoryName());

    File recordingDirectory = rootFolder;
    int index = 0;

    while (resetCounters && recordingDirectory.exists())
    {
        index += 1;
        recordingDirectory = File (rootFolder.getFullPathName() + " (" + String (index) + ")");
    }

    rootFolder = File (recordingDirectory.getFullPathName()
                       + File::getSeparatorString()
                       + getName()
                       + " " + String (getNodeId()));

    newDirectoryNeeded = false;

    if (resetCounters)
    {
        recordingNumber = 0;
        experimentNumber = 1;
        LOGD ("RecordNode::createNewDirectory(): experimentNumber = 1");
    }

    settingsNeeded = true;
}

// called by RecordEngine
String RecordNode::generateDateString() const
{
    Time calendar = Time::getCurrentTime();

    String datestring;

    int day = calendar.getDayOfMonth();

    if (day < 10)
        datestring += "0";

    datestring += String (day);
    datestring += "-";
    datestring += calendar.getMonthName (true);
    datestring += "-";
    datestring += String (calendar.getYear());
    datestring += " ";

    int hrs, mins, secs;
    hrs = calendar.getHours();
    mins = calendar.getMinutes();
    secs = calendar.getSeconds();

    if (hrs < 10)
        datestring += "0";

    datestring += hrs;
    datestring += ":";

    if (mins < 10)
        datestring += "0";

    datestring += mins;
    datestring += ":";

    if (secs < 0)
        datestring += "0";

    datestring += secs;

    return datestring;
}

// called by CoreServices
int RecordNode::getExperimentNumber() const
{
    LOGD ("getExperimentNumber(): Current experiment = ", experimentNumber);
    return experimentNumber;
}

// called by CoreServices
int RecordNode::getRecordingNumber() const
{
    LOGD ("Current recording = ", recordingNumber);
    return recordingNumber;
}

// called by ProcessorGraph::createNewProcessor
AudioProcessorEditor* RecordNode::createEditor()
{
    editor = std::make_unique<RecordNodeEditor> (this);
    return editor.get();
}

// called by GenericProcessor::update()
void RecordNode::updateSettings()
{
    activeStreamIds.clear();
    synchronizer.prepareForUpdate();

    for (auto stream : dataStreams)
    {
        TtlLineParameter* syncLineParam = static_cast<TtlLineParameter*> (stream->getParameter ("sync_line"));
        const uint16 streamId = stream->getStreamId();
        activeStreamIds.add (streamId);

        LOGD ("Record Node found stream: (", streamId, ") ", stream->getName(), " with sample rate ", stream->getSampleRate());
        int syncLine = syncLineParam->getSelectedLine();
        synchronizer.addDataStream (stream->getKey(), stream->getSampleRate(), syncLine, stream->generatesTimestamps());

        fifoUsage[streamId] = 0.0f;

        // Get the stream's event channels and set the number of available lines in the sync line parameter
        const Array<EventChannel*> eventChannels = stream->getEventChannels();

        int numLines;
        if (eventChannels.size() > 0)
            numLines = eventChannels[0]->getMaxTTLBits();
        else
            numLines = 1;

        // Update the number of available lines in the sync line parameter
        syncLineParam->setMaxAvailableLines (numLines);
    }

    synchronizer.finishedUpdate();

    // get rid of unused IDs
    for (auto it = recordContinuousChannels.begin(); it != recordContinuousChannels.end();)
    {
        if (! activeStreamIds.contains (it->first))
            it = recordContinuousChannels.erase (it);
        else
            ++it;
    }

    // Set the main sync stream from the synchronizer
    auto param = getParameter ("main_sync");
    Array<String> streamNames = ((SelectedStreamParameter*) param)->getStreamNames();
    String mainStreamKey = synchronizer.mainStreamKey;
    if (mainStreamKey.isEmpty())
    {
        param->setNextValue (-1, false); // no main stream selected
    }
    else
    {
        int mainStreamIndex = -1;
        for (int i = 0; i < streamNames.size(); i++)
        {
            if (streamNames[i] == mainStreamKey)
            {
                mainStreamIndex = i;
                break;
            }
        }
        param->setNextValue (mainStreamIndex, false); // set main stream index
    }
}

bool RecordNode::isSynchronized()
{
    if (dataStreams.size() == 1) // no need to sync only one DataStream
        return true;

    for (auto stream : dataStreams)
    {
        SyncStatus status = synchronizer.getStatus (stream->getKey());

        LOGD (stream->getKey(), " sync status: ", status);

        if (status != SYNCED && status != HARDWARE_SYNCED && status != HARP_CLOCK)
            return false;
    }

    return true;
}

// called by GenericProcessor::enableProcessor
bool RecordNode::startAcquisition()
{
    recordEvents = ((BooleanParameter*) getParameter ("events"))->getBoolValue();
    recordSpikes = ((BooleanParameter*) getParameter ("spikes"))->getBoolValue();

    synchronizer.startAcquisition();

    if (eventChannels.size() == 0 || eventChannels.getLast()->getSourceNodeName() != "Message Center")
    {
        eventChannels.add (new EventChannel (*messageChannel));
        eventChannels.getLast()->addProcessor (this);
        eventChannels.getLast()->setDataStream (getDataStream (synchronizer.mainStreamKey), false);
    }

    if (! headlessMode)
        startTimer (1000);

    return true;
}

bool RecordNode::stopAcquisition()
{
    synchronizer.stopAcquisition();

    if (hasRecorded)
    {
        // stopRecording() signals the thread to exit, but we should wait here until the thread actually gracefully
        // exits before we reset some of its needed data (e.g. eventQueue, spikeQueue, etc.)
        if (recordThread)
        {
            recordThread->waitForThreadToExit (1000);
        }
    }

    // Remove message channel
    if (eventChannels.size() > 0 && eventChannels.getLast()->getSourceNodeName() == "Message Center")
    {
        eventChannels.removeLast();
    }

    eventMonitor->displayStatus();

    if (hasRecorded)
    {
        hasRecorded = false;
        experimentNumber++;
        settingsNeeded = true;
    }

    recordingNumber = 0;
    recordEngine->configureEngine();
    synchronizer.reset();
    eventMonitor->reset();

    eventQueue->reset();
    spikeQueue->reset();

    if (! headlessMode)
        stopTimer();

    return true;
}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::startRecording()
{
    Array<int> chanProcessorMap;
    Array<int> chanOrderinProc;
    OwnedArray<RecordProcessorInfo> procInfo;

    // in case recording starts before acquisition:
    if (eventChannels.size() == 0 || eventChannels.getLast()->getSourceNodeName() != "Message Center")
    {
        eventChannels.add (new EventChannel (*messageChannel));
        eventChannels.getLast()->addProcessor (this);
        eventChannels.getLast()->setDataStream (getDataStream (synchronizer.mainStreamKey), false);
    }

    int lastSourceNodeId = -1;

    int channelIndexInRecordNode = 0;
    int channelIndexInStream = 0;

    channelMap.clear();
    localChannelMap.clear();

    timestampChannelMap.clear();

    int streamIndex = 0;

    // Count recorded channels per stream for creating per-stream DataQueues
    Array<int> recordedChannelsPerStream;

    for (auto stream : dataStreams)
    {
        RecordProcessorInfo* pi = new RecordProcessorInfo();
        pi->processorId = stream->getSourceNodeId();

        if (stream->getSourceNodeId() != lastSourceNodeId)
        {
            channelIndexInStream = 0;
            lastSourceNodeId = stream->getSourceNodeId();
        }

        int recordedChannelsInThisStream = 0;
        for (auto channelRecordState : ((MaskChannelsParameter*) stream->getParameter ("channels"))->getChannelStates())
        {
            if (channelRecordState)
            {
                channelMap.add (channelIndexInRecordNode);
                localChannelMap.add (channelIndexInStream++);
                timestampChannelMap.add (streamIndex);
                recordedChannelsInThisStream++;
            }

            channelIndexInRecordNode++;
        }

        recordedChannelsPerStream.add (recordedChannelsInThisStream);
        procInfo.add (pi);
        streamIndex++;
    }

    int numRecordedChannels = channelMap.size();

    validBlocks.clear();
    validBlocks.insertMultiple (0, false, getNumInputs());

    recordEngine->registerRecordNode (this);
    recordEngine->setChannelMap (channelMap, localChannelMap);

    recordThread->setChannelMap (channelMap);
    recordThread->setTimestampChannelMap (timestampChannelMap);

    // Create per-stream DataQueues for race-condition-free reading
    // Get current audio buffer size for queue block size
    AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
    AudioDeviceManager::AudioDeviceSetup ads;
    adm.getAudioDeviceSetup (ads);
    int bufferSize = ads.bufferSize;

    dataQueues.clear();
    streamSourceChannels.clear();
    int globalChannelIdx = 0;

    for (int i = 0; i < dataStreams.size(); i++)
    {
        // Only create queue if this stream has recorded channels
        if (recordedChannelsPerStream[i] > 0)
        {
            DataQueue* queue = new DataQueue (bufferSize, DATA_BUFFER_NBLOCKS);
            queue->setChannelCount (recordedChannelsPerStream[i]);
            queue->setTimestampStreamCount (1); // Each queue has one timestamp stream
            dataQueues.add (queue);

            // Pre-compute source channel array for this stream
            Array<int>* srcChannels = new Array<int>();
            srcChannels->ensureStorageAllocated (recordedChannelsPerStream[i]);
            for (int j = 0; j < recordedChannelsPerStream[i]; j++)
            {
                srcChannels->add (channelMap[globalChannelIdx + j]);
            }
            streamSourceChannels.add (srcChannels);
        }
        else
        {
            // Add nullptr placeholder to maintain stream index alignment
            dataQueues.add (nullptr);
            streamSourceChannels.add (nullptr);
        }

        globalChannelIdx += recordedChannelsPerStream[i];
    }

    recordThread->setQueuePointers (&dataQueues, eventQueue.get(), spikeQueue.get());
    recordThread->setFirstBlockFlag (false);

    /* Set write properties */
    setFirstBlock = false;

    if (! rootFolder.exists())
    {
        Result res = rootFolder.createDirectory();
        if (res.failed())
        {
            LOGE ("Record Node " + String (getNodeId()) + ": Could not create directory: " + rootFolder.getFullPathName(), " -- ", res.getErrorMessage());

            CoreServices::setRecordingStatus (false);

            if (! headlessMode)
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Recording Error",
                                                  "Record Node " + String (getNodeId()) + " - Could not create recording directory:\n\n"
                                                      + rootFolder.getFullPathName() + "\n\n"
                                                      + res.getErrorMessage());
            }

            return;
        }
    }

    recordThread->setFileComponents (rootFolder, experimentNumber, recordingNumber);
    recordThread->startThread (Thread::Priority::highest);
    diskSpaceChecker->reset();

    if (settingsNeeded)
    {
        String settingsFileName = rootFolder.getFullPathName() + File::getSeparatorString() + "settings" + ((experimentNumber > 1) ? "_" + String (experimentNumber) : String()) + ".xml";

        std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement> ("SETTINGS");

        AccessClass::getProcessorGraph()->saveToXml (xml.get());

        xml->writeTo (settingsFileName);

        settingsNeeded = false;
    }
}

void RecordNode::notifyRecordThreadFilesOpened()
{
    MessageManager::callAsync ([this]()
                               {
                                   if (recordThread->isThreadRunning())
                                       isRecording = true; });
}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::stopRecording()
{
    if (! isRecording && !recordThread->isThreadRunning())
        return;

    isRecording = false;
    hasRecorded = true;
    recordingNumber++; // increment recording number within this directory; should be zero for first recording

    if (recordThread->isThreadRunning())
    {
        recordThread->signalThreadShouldExit();
    }
}

bool RecordNode::getRecordingStatus() const
{
    return isRecording;
}

void RecordNode::setRecordEvents (bool recordEvents)
{
    this->recordEvents = recordEvents;
}

void RecordNode::setRecordSpikes (bool recordSpikes)
{
    this->recordSpikes = recordSpikes;
}

void RecordNode::handleTTLEvent (TTLEventPtr event)
{
    eventMonitor->receivedEvents++;

    int64 sampleNumber = event->getSampleNumber();

    String streamKey = getDataStream (event->getStreamId())->getKey();

    synchronizer.addEvent (streamKey, event->getLine(), sampleNumber, event->getState());

    if (recordEvents && isRecording)
    {
        size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
        uint16 streamId = event->getStreamId();

        double ts = -1.0;
        if (synchronizer.streamGeneratesTimestamps (streamKey))
        {
            ts = getFirstTimestampForBlock (streamId) + (sampleNumber - getFirstSampleNumberForBlock (streamId)) / getDataStream (streamId)->getSampleRate();
        }
        else
        {
            ts = synchronizer.convertSampleNumberToTimestamp (streamKey, sampleNumber);
        }

        HeapBlock<char> buffer (size);
        event->setTimestampInSeconds (ts);
        event->serialize (buffer, size);

        eventQueue->addEvent (EventPacket (buffer, int (size)), sampleNumber);

        eventMonitor->bufferedEvents++;
    }
}

void RecordNode::handleEvent (const EventChannel* eventInfo, const EventPacket& packet)
{
    if (recordEvents && isRecording)
    {
        int64 sampleNumber = Event::getSampleNumber (packet);

        int eventIndex = getIndexOfMatchingChannel (eventInfo);

        String streamKey = getDataStream (eventInfo->getStreamId())->getKey();

        Event::setTimestampInSeconds (packet, synchronizer.convertSampleNumberToTimestamp (streamKey, sampleNumber));

        eventQueue->addEvent (packet, sampleNumber, eventIndex);
    }
}

// only called if recordSpikes is true
void RecordNode::handleSpike (SpikePtr spike)
{
    eventMonitor->receivedSpikes++;

    if (recordSpikes && isRecording)
    {
        String streamKey = getDataStream (spike->getStreamId())->getKey();
        uint16 streamId = spike->getStreamId();
        int64 sampleNumber = spike->getSampleNumber();

        double ts = -1.0;
        if (synchronizer.streamGeneratesTimestamps (streamKey))
        {
            ts = getFirstTimestampForBlock (streamId) + (sampleNumber - getFirstSampleNumberForBlock (streamId)) / getDataStream (streamId)->getSampleRate();
        }
        else
        {
            ts = synchronizer.convertSampleNumberToTimestamp (streamKey, sampleNumber);
        }

        spike->setTimestampInSeconds (ts);
        writeSpike (spike, spike->getChannelInfo());
        eventMonitor->bufferedSpikes++;
    }
}

void RecordNode::handleTimestampSyncTexts (const EventPacket& packet)
{
    int64 sampleNumber = Event::getSampleNumber (packet);

    String syncText = SystemEvent::getSyncText (packet);

    eventQueue->addEvent (packet, sampleNumber, -1);
}

void RecordNode::process (AudioBuffer<float>& buffer)

{
    isProcessing = true;

    checkForEvents (recordSpikes);

    if (isRecording)
    {
        if (! setFirstBlock)
        {
            MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer (this);
            HeapBlock<char> data;

            size_t dataSize =
                SystemEvent::fillTimestampSyncTextData (
                    data,
                    this,
                    0,
                    CoreServices::getSystemTime(),
                    -1.0,
                    true);

            handleTimestampSyncTexts (EventPacket (data, int (dataSize)));

            for (auto stream : getDataStreams())
            {
                const uint16 streamId = stream->getStreamId();

                int64 firstSampleNumberInBlock = getFirstSampleNumberForBlock (streamId);

                MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer (this);
                HeapBlock<char> data;

                GenericProcessor* src = AccessClass::getProcessorGraph()->getProcessorWithNodeId (getDataStream (streamId)->getSourceNodeId());

                size_t dataSize = SystemEvent::fillTimestampSyncTextData (data, src, streamId, firstSampleNumberInBlock, -1.0, false);

                handleTimestampSyncTexts (EventPacket (data, int (dataSize)));
            }
        }

        bool fifoAlmostFull = false;

        int streamIndex = -1;
        int channelIndex = -1;

        for (auto stream : dataStreams)
        {
            streamIndex++;

            // Use pre-computed array size to avoid JSON lookup on every call
            int recordChanCount = (streamSourceChannels[streamIndex] != nullptr)
                                      ? streamSourceChannels[streamIndex]->size()
                                      : 0;

            if (recordChanCount == 0)
                continue;

            const uint16 streamId = stream->getStreamId();
            const String streamKey = stream->getKey();

            uint32 numSamples = getNumSamplesInBlock (streamId);

            int64 sampleNumber = getFirstSampleNumberForBlock (streamId);

            double first, second;

            // Skip if no queue for this stream (no recorded channels)
            if (dataQueues[streamIndex] == nullptr)
            {
                channelIndex += recordChanCount;
                continue;
            }

            if (numSamples > 0)
            {
                if (! synchronizer.streamGeneratesTimestamps (streamKey))
                {
                    first = synchronizer.convertSampleNumberToTimestamp (streamKey, sampleNumber);
                    second = synchronizer.convertSampleNumberToTimestamp (streamKey, sampleNumber + 1);
                }
                else
                {
                    first = getFirstTimestampForBlock (streamId);
                    second = first + 1 / stream->getSampleRate();
                    synchronizer.setHardwareTimestamp (sampleNumber, first, streamKey);
                }
                // Each per-stream queue has only 1 timestamp stream (index 0)
                dataQueues[streamIndex]->writeSynchronizedTimestamps (
                    first,
                    second - first,
                    0, // timestamp stream index within this queue
                    numSamples);
            }

            if (numSamples > 0 && recordChanCount > 0 && streamSourceChannels[streamIndex] != nullptr)
            {
                // Batch write all channels at once using pre-computed source channels
                fifoUsage[streamId] = dataQueues[streamIndex]->writeAllChannels (
                    buffer,
                    *streamSourceChannels[streamIndex],
                    numSamples,
                    sampleNumber);
            }
            else
            {
                fifoUsage[streamId] = 0.0f;
            }

            channelIndex += recordChanCount;

            if (fifoUsage[streamId] > 0.9)
                fifoAlmostFull = true;

            samplesWritten += numSamples;
        }

        if (fifoAlmostFull)
        {
            CoreServices::setRecordingStatus (false);

            if (headlessMode)
            {
                LOGC ("Record Buffer Warning: The recording buffer has reached capacity. Stopping recording to prevent data corruption.\n\n \
                        To address the issue, you can try reducing the number of simultaneously recorded channels or \
                        using multiple Record Nodes to distribute data writing across more than one drive.");
            }
            else
            {
                MessageManager::callAsync ([this]
                                           { AlertWindow::showMessageBoxAsync (AlertWindow::AlertIconType::WarningIcon,
                                                                               "Record Buffer Warning",
                                                                               "The recording buffer has reached capacity. Stopping recording to prevent data corruption. \n\n"
                                                                               "To address the issue, you can try reducing the number of simultaneously recorded channels or "
                                                                               "using multiple Record Nodes to distribute data writing across more than one drive.",
                                                                               "OK"); });
            }
        }

        if (! setFirstBlock)
        {
            recordThread->setFirstBlockFlag (true);
            setFirstBlock = true;
        }
    }
}

// called in RecordNode::handleSpike
void RecordNode::writeSpike (const Spike* spike, const SpikeChannel* spikeElectrode)
{
    int electrodeIndex = getIndexOfMatchingChannel (spikeElectrode);

    if (electrodeIndex >= 0)
        spikeQueue->addEvent (*spike, spike->getSampleNumber(), electrodeIndex);
}

void RecordNode::timerCallback()
{
    updateSyncMonitors();
}

void RecordNode::updateSyncMonitors()
{
    for (auto stream : dataStreams)
    {
        const uint16 streamId = stream->getStreamId();
        const String streamKey = stream->getKey();

        RecordNodeEditor* editor = (RecordNodeEditor*) getEditor();

        editor->setStreamStartTime (streamId, synchronizer.isStreamSynced (streamKey), synchronizer.getStartTime (streamKey));
        editor->setLastSyncEvent (streamId, synchronizer.isStreamSynced (streamKey), synchronizer.getLastSyncEvent (streamKey));
        editor->setSyncAccuracy (streamId, synchronizer.isStreamSynced (streamKey), synchronizer.getAccuracy (streamKey));
    }
}

// not called?
float RecordNode::getFreeSpace() const
{
    return 1.0f - float (dataDirectory.getBytesFreeOnVolume()) / float (dataDirectory.getVolumeTotalSize());
}

float RecordNode::getFreeSpaceKilobytes() const
{
    return dataDirectory.getBytesFreeOnVolume() / 1024.0f;
}

int RecordNode::getTotalRecordedStreams()
{
    int numStreams = 0;

    for (auto stream : dataStreams)
    {
        for (auto ch : stream->getContinuousChannels())
        {
            if (ch->isRecorded)
            {
                numStreams++;
                break;
            }
        }
    }

    return numStreams;
}

void RecordNode::saveCustomParametersToXml (XmlElement* xml)
{
    if (! headlessMode)
    {
        RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*) getEditor();
        xml->setAttribute ("fifoMonitorsVisible", recordNodeEditor->fifoDrawerButton->getToggleState());
    }

    for (auto stream : dataStreams)
    {
        auto* streamSyncStatusNode = xml->createNewChildElement ("SYNC_STATUS");
        const String streamKey = stream->getKey();
        streamSyncStatusNode->setAttribute ("streamKey", streamKey);

        String statusString = "OFF";
        switch (synchronizer.getStatus (streamKey))
        {
            case SyncStatus::OFF:
                statusString = "OFF";
                break;
            case SyncStatus::SYNCING:
                statusString = "NOT_SYNCED";
                break;
            case SyncStatus::SYNCED:
                statusString = "SYNCED_TO_MAIN";
                break;
            case SyncStatus::HARDWARE_SYNCED:
                statusString = "HARDWARE_SYNCED";
                break;
            case SyncStatus::HARP_DETECTING:
                statusString = "HARP_DETECTING";
                break;
            case SyncStatus::HARP_CLOCK:
                statusString = "HARP_SYNCED";
                break;
            default:
                statusString = "NONE";
                break;
        }

        streamSyncStatusNode->setAttribute ("status", statusString);
    }
}

void RecordNode::loadCustomParametersFromXml (XmlElement* xml)
{
    if (xml->hasAttribute ("fifoMonitorsVisible"))
    {
        if (! headlessMode)
        {
            RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*) getEditor();
            if (! xml->getBoolAttribute ("fifoMonitorsVisible"))
                recordNodeEditor->fifoDrawerButton->triggerClick();
        }
    }
}
