#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "../../Processors/MessageCenter/MessageCenterEditor.h"
#include "BinaryFormat/BinaryRecording.h"
#include "../../Audio/AudioComponent.h"
#include "../../AccessClass.h"

#include "../Events/Spike.h"
#include "../Settings/DataStream.h"
#include "../Settings/DeviceInfo.h"

using namespace std::chrono;

#define CONTINUOUS_CHANNELS_ON_BY_DEFAULT true
#define RECEIVED_SOFTWARE_TIME (event.getVelocity() == 136)

EventMonitor::EventMonitor()
	: receivedEvents(0),
	  receivedSpikes(0),
      bufferedEvents(0),
      bufferedSpikes(0)
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
	LOGD("Record Node received ", receivedEvents, " total EVENTS and sent ", bufferedEvents, " to the RecordThread");

	LOGD("Record Node received ", receivedSpikes, " total SPIKES and sent ", bufferedSpikes, " to the RecordThread");
}

RecordNode::RecordNode()
	: GenericProcessor("Record Node"),
	newDirectoryNeeded(true),
	setFirstBlock(false),
	samplesWritten(0),
	experimentNumber(1), // 1-based indexing
	recordingNumber(0), // 0-based indexing
	isRecording(false),
	hasRecorded(false),
	settingsNeeded(false),
	numDataStreams(0)
{

	//Get the current audio device's buffer size and use as data queue block size
	AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
	AudioDeviceManager::AudioDeviceSetup ads;
	adm.getAudioDeviceSetup(ads);
	int bufferSize = ads.bufferSize;

	dataQueue = std::make_unique<DataQueue>(bufferSize, DATA_BUFFER_NBLOCKS);
	eventQueue = std::make_unique<EventMsgQueue>(EVENT_BUFFER_NEVENTS);
	spikeQueue = std::make_unique<SpikeMsgQueue>(SPIKE_BUFFER_NSPIKES);

	isSyncReady = true;

	/* New record nodes default to the record engine currently selected in the Control Panel */
	setEngine(CoreServices::getDefaultRecordEngineId());

	dataDirectory = CoreServices::getRecordingParentDirectory();

	recordThread = std::make_unique<RecordThread>(this, recordEngine.get());

	eventMonitor = new EventMonitor();

	checkDiskSpace();

}

RecordNode::~RecordNode()
{
}

void RecordNode::checkDiskSpace()
{
	int diskSpaceWarningThreshold = 5; //GB

	File dataDir(dataDirectory);
	int64 freeSpace = dataDir.getBytesFreeOnVolume();

	float availableBytes = freeSpace / pow(2, 30); //1 GB == 2^30 bytes

	if (availableBytes < diskSpaceWarningThreshold && !isRecording)
	{
		String msg = "Less than " + String(diskSpaceWarningThreshold) + " GB of disk space available in " + String(dataDirectory.getFullPathName());
		msg += ". Recording may fail. Please free up space or change the recording directory.";
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "WARNING", msg);
	}
}


String RecordNode::handleConfigMessage(String msg)
{

	const MessageManagerLock mml;

    StringArray tokens;
    tokens.addTokens (msg, "=", "\"");

    if (tokens.size() != 2) return "Invalid msg";

	if (tokens[0] == "engine")
		static_cast<RecordNodeEditor*> (getEditor())->engineSelectCombo->setSelectedItemIndex(std::stoi(tokens[1].toStdString()), sendNotification);
	else
		LOGD("Record Node: invalid engine key");

    return "Record Node received config: " + msg;
}

void RecordNode::handleBroadcastMessage(String msg)
{

    if (recordEvents && isRecording)
    {

        int64 messageSampleNumber = getFirstSampleNumberForBlock(synchronizer.mainStreamId);

        TextEventPtr event = TextEvent::createTextEvent(getMessageChannel(), messageSampleNumber, msg);

        double ts = synchronizer.convertSampleNumberToTimestamp(synchronizer.mainStreamId, messageSampleNumber);

        event->setTimestampInSeconds(ts);

        size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

        HeapBlock<char> buffer(size);

        event->serialize(buffer, size);

        eventQueue->addEvent(EventPacket(buffer, size), messageSampleNumber, -1);

    }


}

void RecordNode::updateBlockSize(int newBlockSize)
{
	if (dataQueue->getBlockSize() != newBlockSize)
		dataQueue = std::make_unique<DataQueue>(newBlockSize, DATA_BUFFER_NBLOCKS);
}

String RecordNode::getEngineId()
{

	return recordEngine->getEngineId();

}

void RecordNode::setEngine(String id)
{
	availableEngines = getAvailableRecordEngines();

	for (auto engine : availableEngines)
	{
		if (engine->getID().compare(id) == 0)
        {
            if (recordEngine.get() != nullptr)
            {
                if (recordEngine->getEngineId() != id)
                {
                    recordEngine.reset(engine->instantiateEngine());

                    if (recordThread != nullptr)
                        recordThread->setEngine(recordEngine.get());
                }
            } else {
                recordEngine.reset(engine->instantiateEngine());
            }

        }
	}

	if (getEditor() != nullptr)
	{
		RecordNodeEditor* ed = (RecordNodeEditor*)getEditor();
		ed->setEngine(id);
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
void RecordNode::setDataDirectory(File directory)
{
	dataDirectory = directory;
	newDirectoryNeeded = true;

	checkDiskSpace();
}

void RecordNode::createNewDirectory()
{

	LOGD("CREATE NEW DIRECTORY");

	rootFolder = File(dataDirectory.getFullPathName()
		+ File::getSeparatorString()
		+ generateDirectoryName());

	File recordingDirectory = rootFolder;
	int index = 0;

	while (recordingDirectory.exists())
	{
		index += 1;
		recordingDirectory = File(rootFolder.getFullPathName() + " (" + String(index) + ")");
	}

	rootFolder = File(recordingDirectory.getFullPathName()
			   + File::getSeparatorString()
			   + getName()
			   + " " + String(getNodeId()));

	newDirectoryNeeded = false;

	recordingNumber = 0;
	experimentNumber = 1;
	LOGD("RecordNode::createNewDirectory(): experimentNumber = 1");
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

	datestring += String(day);
	datestring += "-";
	datestring += calendar.getMonthName(true);
	datestring += "-";
	datestring += String(calendar.getYear());
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
	LOGD("getExperimentNumber(): Current experiment = ", experimentNumber);
	return experimentNumber;
}

// called by CoreServices
int RecordNode::getRecordingNumber() const
{
	LOGD("Current recording = ", recordingNumber);
	return recordingNumber;
}

// called by ProcessorGraph::createNewProcessor
AudioProcessorEditor* RecordNode::createEditor()
{
	editor = std::make_unique<RecordNodeEditor>(this);
	return editor.get();
}

// called by RecordEngine when saving the signal chain
const String &RecordNode::getLastSettingsXml() const
{
	return lastSettingsText;
}

// Called when deleting FifoMonitor
void RecordNode::updateChannelStates(uint16 streamId, std::vector<bool> channelStates)
{
	//std::cout << "Updating channel states." << std::endl;
	//for (int i = 0; i < channelStates.size(); i++)
	//	std::cout << channelStates[i] << std::endl;
	recordContinuousChannels[streamId] = channelStates;
	for (auto stream : dataStreams)
	{
		if (stream->getStreamId() == streamId)
		{
			int ch = 0;
			for (auto channel : stream->getContinuousChannels())
			{
				channel->isRecorded = channelStates[ch++];
			}
			break;
		}
	}

    //CoreServices::updateSignalChain(getEditor());
}


// called by GenericProcessor::update()
void RecordNode::updateSettings()
{

	activeStreamIds.clear();
	synchronizer.prepareForUpdate();

	for (auto stream : dataStreams)
	{
		const uint16 streamId = stream->getStreamId();
		activeStreamIds.add(streamId);

		LOGD("Record Node found stream: (", streamId, ") ", stream->getName());
		//activeStreamIds.add(stream->getStreamId());
		synchronizer.addDataStream(streamId,
									stream->getSampleRate());

		fifoUsage[streamId] = 0.0f;

		if (recordContinuousChannels[streamId].empty()) // this ID has not been seen yet
		{
			for (auto channel : stream->getContinuousChannels())
			{
				recordContinuousChannels[streamId].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				channel->isRecorded = CONTINUOUS_CHANNELS_ON_BY_DEFAULT;
				//LOGD("Channel ", channel->getName(), ": ", channel->isRecorded);
				//LOGD(recordContinuousChannels[streamId].size())
				//dataChannelOrder[count] = channel->getLocalIndex();
				//count++;
			}

		}
		else // we already have this ID, just apply the existing settings
		{
			int localIndex = 0;

			for (auto channel : stream->getContinuousChannels())
			{
				if (localIndex < recordContinuousChannels[streamId].size())
				{
					channel->isRecorded = recordContinuousChannels[streamId][localIndex];
				} else {
					recordContinuousChannels[streamId].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
					channel->isRecorded = CONTINUOUS_CHANNELS_ON_BY_DEFAULT;
				}

				//LOGD("Channel ", channel->getName(), ": ", channel->isRecorded);

				localIndex++;
				//dataChannelOrder[count] = channel->getLocalIndex();
				//count++;
			}

		}

	}

	synchronizer.finishedUpdate();


	// get rid of unused IDs
	for (auto it = recordContinuousChannels.begin(); it != recordContinuousChannels.end(); ) {
		if (!activeStreamIds.contains(it->first))
			it = recordContinuousChannels.erase(it);
		else
			++it;
	}

#ifdef _WIN32
	// check Open Ephys format on windows
	if (recordEngine->getEngineId().equalsIgnoreCase("OPENEPHYS") && getNumInputs() > 300)
	{
		int new_max = 0;
		int calculated_max = getNumInputs() + getTotalEventChannels() + getTotalSpikeChannels() + getNumDataStreams() + 5;

		if (calculated_max < 8192) // actual upper bound of 8192
			new_max = _setmaxstdio(calculated_max);

		if (new_max != calculated_max)
		{
			AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
				"WARNING", "Open Ephys format does not support this many simultaneously recorded channels. Resetting to Binary format.");
			setEngine("BINARY");
		}

	}
#endif

	//Refresh editor as needed
	if (static_cast<RecordNodeEditor*> (getEditor())->monitorsVisible)
	{
		static_cast<RecordNodeEditor*> (getEditor())->showFifoMonitors(false);
		static_cast<RecordNodeEditor*> (getEditor())->buttonClicked(static_cast<RecordNodeEditor*> (getEditor())->fifoDrawerButton);
	}

}

bool RecordNode::isSynchronized()
{

    if (dataStreams.size() == 1) // no need to sync only one DataStream
        return true;

    for (auto stream : dataStreams)
    {

        SyncStatus status = synchronizer.getStatus(stream->getStreamId());

        if (status != SYNCED)
            return false;
    }

    return true;
}

// called by GenericProcessor::enableProcessor
bool RecordNode::startAcquisition()
{

    synchronizer.startAcquisition();

    if (eventChannels.getLast()->getSourceNodeName() != "Message Center")
    {
        eventChannels.add(new EventChannel(*messageChannel));
        eventChannels.getLast()->addProcessor(processorInfo.get());
        eventChannels.getLast()->setDataStream(getDataStream(synchronizer.mainStreamId), false);
    }
    
    return true;

}

bool RecordNode::stopAcquisition()
{

    synchronizer.stopAcquisition();

	// Remove message channel
    if (eventChannels.getLast()->getSourceNodeName() == "Message Center")
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

	return true;
}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::startRecording()
{
	Array<int> chanProcessorMap;
	Array<int> chanOrderinProc;
	OwnedArray<RecordProcessorInfo> procInfo;
    
    // in case recording starts before acquisition:
    if (eventChannels.getLast()->getSourceNodeName() != "Message Center")
    {
        eventChannels.add(new EventChannel(*messageChannel));
        eventChannels.getLast()->addProcessor(processorInfo.get());
        eventChannels.getLast()->setDataStream(getDataStream(synchronizer.mainStreamId), false);
    }

	int lastSourceNodeId = -1;

	int channelIndexInRecordNode = 0;
    int channelIndexInStream = 0;

	channelMap.clear();
    localChannelMap.clear();

	timestampChannelMap.clear();

	int streamIndex = 0;

	for (auto stream : dataStreams)
	{

		RecordProcessorInfo* pi = new RecordProcessorInfo();
		pi->processorId = stream->getSourceNodeId();

		if (stream->getSourceNodeId() != lastSourceNodeId)
		{
            channelIndexInStream = 0;
			lastSourceNodeId = stream->getSourceNodeId();
		}

		for (auto channel : stream->getContinuousChannels())
		{
			if (channel->isRecorded)
			{
				channelMap.add(channelIndexInRecordNode);
                localChannelMap.add(channelIndexInStream++);
				timestampChannelMap.add(streamIndex);
			}

			channelIndexInRecordNode++;
		}

		procInfo.add(pi);
		streamIndex++;

	}

	int numRecordedChannels = channelMap.size();

	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	recordEngine->registerRecordNode(this);
	recordEngine->setChannelMap(channelMap, localChannelMap);

	recordThread->setChannelMap(channelMap);
	recordThread->setTimestampChannelMap(timestampChannelMap);

	dataQueue->setChannelCount(numRecordedChannels);
	dataQueue->setTimestampStreamCount(dataStreams.size());

	recordThread->setQueuePointers(dataQueue.get(), eventQueue.get(), spikeQueue.get());
	recordThread->setFirstBlockFlag(false);

	/* Set write properties */
	setFirstBlock = false;


	if (!rootFolder.exists())
	{
		rootFolder.createDirectory();
	}

	recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);
	recordThread->startThread();
	isRecording = true;

	if (settingsNeeded)
	{
		String settingsFileName = rootFolder.getFullPathName() + File::getSeparatorString() + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String()) + ".xml";
		AccessClass::getEditorViewport()->saveState(File(settingsFileName), lastSettingsText);
		settingsNeeded = false;
	}
}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::stopRecording()
{

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

void RecordNode::setRecordEvents(bool recordEvents)
{
	this->recordEvents = recordEvents;
}

void RecordNode::setRecordSpikes(bool recordSpikes)
{
	this->recordSpikes = recordSpikes;
}

void RecordNode::handleTTLEvent(TTLEventPtr event)
{

	eventMonitor->receivedEvents++;

	int64 sampleNumber = event->getSampleNumber();

	synchronizer.addEvent(event->getStreamId(), event->getLine(), sampleNumber);

	if (recordEvents && isRecording)
	{

		size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

		HeapBlock<char> buffer(size);
        event->setTimestampInSeconds(synchronizer.convertSampleNumberToTimestamp(event->getStreamId(), sampleNumber));
		event->serialize(buffer, size);

		eventQueue->addEvent(EventPacket(buffer, size), sampleNumber);

		eventMonitor->bufferedEvents++;

	}

}

void RecordNode::handleEvent(const EventChannel* eventInfo, const EventPacket& packet)
{

	if (recordEvents && isRecording)
	{

	    int64 sampleNumber = Event::getSampleNumber(packet);

		int eventIndex = getIndexOfMatchingChannel(eventInfo);

        Event::setTimestampInSeconds(packet, synchronizer.convertSampleNumberToTimestamp(eventInfo->getStreamId(), sampleNumber));

		eventQueue->addEvent(packet, sampleNumber, eventIndex);

	}

}

// only called if recordSpikes is true
void RecordNode::handleSpike(SpikePtr spike)
{

	eventMonitor->receivedSpikes++;

	if (recordSpikes)
	{
        spike->setTimestampInSeconds(synchronizer.convertSampleNumberToTimestamp(spike->getStreamId(),
                                                                    spike->getSampleNumber()));
		writeSpike(spike, spike->getChannelInfo());
		eventMonitor->bufferedSpikes++;
	}


}

void RecordNode::handleTimestampSyncTexts(const EventPacket& packet)
{
    int64 sampleNumber = Event::getSampleNumber(packet);

    eventQueue->addEvent(packet, sampleNumber, -1);
}

void RecordNode::process(AudioBuffer<float>& buffer)

{

	isProcessing = true;

	checkForEvents(recordSpikes);

	if (isRecording)
	{

		if (!setFirstBlock)
		{

			MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
			HeapBlock<char> data;

			size_t dataSize =
                SystemEvent::fillTimestampSyncTextData(
                     data,
                     this,
                     0,
                     CoreServices::getSoftwareTimestamp(),
                     -1.0,
                     true);

			handleTimestampSyncTexts(EventPacket(data, dataSize));

			for (auto stream : getDataStreams())
			{

				const uint16 streamId = stream->getStreamId();

				int64 firstSampleNumberInBlock = getFirstSampleNumberForBlock(streamId);

				MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
				HeapBlock<char> data;

				GenericProcessor* src = AccessClass::getProcessorGraph()->getProcessorWithNodeId(getDataStream(streamId)->getSourceNodeId());

				size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, src, streamId, firstSampleNumberInBlock, -1.0, false);

				handleTimestampSyncTexts(EventPacket(data, dataSize));
			}
		}

		bool fifoAlmostFull = false;

		int streamIndex = -1;
		int channelIndex = -1;

		for (auto stream : dataStreams)
		{

			streamIndex++;

			const uint16 streamId = stream->getStreamId();

            uint32 numSamples = getNumSamplesInBlock(streamId);

			int64 sampleNumber = getFirstSampleNumberForBlock(streamId);

			if (numSamples > 0)
			{
				double first = synchronizer.convertSampleNumberToTimestamp(streamId, sampleNumber);
				double second = synchronizer.convertSampleNumberToTimestamp(streamId, sampleNumber + 1);

				fifoUsage[streamId] = dataQueue->writeSynchronizedTimestamps(
					first,
					second - first,
					streamIndex,
					numSamples);

			}

			for (auto channel : stream->getContinuousChannels())
			{

				if (channel->isRecorded)
				{

					channelIndex++;

					if (numSamples > 0)
					{
						dataQueue->writeChannel(buffer,
							channelMap[channelIndex],
							channelIndex,
							numSamples,
							sampleNumber);
					}


				}
			}

			if (fifoUsage[streamId] > 0.9)
				fifoAlmostFull = true;

			samplesWritten += numSamples;

		}

		if (fifoAlmostFull)
		{

			AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
				"Record Buffer Warning",
				"The recording buffer has reached capacity. Stopping recording to prevent data corruption. \n\n"
				"To address the issue, you can try reducing the number of simultaneously recorded channels or "
				"using multiple Record Nodes to distribute data writing across more than one drive.",
				"OK");

			CoreServices::setRecordingStatus(false);
		}

		if (!setFirstBlock)
		{
			recordThread->setFirstBlockFlag(true);
			setFirstBlock = true;
		}

	}

}

// called in RecordNode::handleSpike
void RecordNode::writeSpike(const Spike *spike, const SpikeChannel *spikeElectrode)
{

    int electrodeIndex = getIndexOfMatchingChannel(spikeElectrode);

    if (electrodeIndex >= 0)
        spikeQueue->addEvent(*spike, spike->getSampleNumber(), electrodeIndex);

}

// FileNameComponent listener
void RecordNode::filenameComponentChanged(FilenameComponent *fnc)
{

	dataDirectory = fnc->getCurrentFile();
	newDirectoryNeeded = true;
}

// not called?
float RecordNode::getFreeSpace() const
{
	return 1.0f - float(dataDirectory.getBytesFreeOnVolume()) / float(dataDirectory.getVolumeTotalSize());
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

// not called?
void RecordNode::registerRecordEngine(RecordEngine *engine)
{
	engineArray.add(engine);
}

// not called?
void RecordNode::clearRecordEngines()
{
	engineArray.clear();
}

void RecordNode::saveCustomParametersToXml(XmlElement* xml)
{

    RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*) getEditor();

    xml->setAttribute ("path", dataDirectory.getFullPathName());
    xml->setAttribute("engine", recordEngine->getEngineId());
    xml->setAttribute ("recordEvents", recordEvents);
    xml->setAttribute ("recordSpikes", recordSpikes);
    xml->setAttribute("fifoMonitorsVisible", recordNodeEditor->fifoDrawerButton->getToggleState());

    //Save channel states:
    for (auto stream : getDataStreams())
    {

        const uint16 streamId = stream->getStreamId();

        if (recordContinuousChannels[streamId].size() > 0)
        {
            XmlElement* streamXml = xml->createNewChildElement("STREAM");

            streamXml->setAttribute("isMainStream", synchronizer.mainStreamId == streamId);
            streamXml->setAttribute("sync_line", getSyncLine(streamId));
            streamXml->setAttribute("name", stream->getName());
            streamXml->setAttribute("source_node_id", stream->getSourceNodeId());
            streamXml->setAttribute("sample_rate", stream->getSampleRate());
            streamXml->setAttribute("channel_count", stream->getChannelCount());

            if (stream->hasDevice())
                streamXml->setAttribute("device_name", stream->device->getName());

            String stateString;
            bool allOn = true;
            bool allOff = true;

            for (int ch = 0; ch < recordContinuousChannels[streamId].size(); ch++)
            {
                bool state = recordContinuousChannels[streamId][ch];

                if (!state)
                    allOn = false;

                if (state)
                    allOff = false;

                stateString += state ? "1" : "0";
            }

            if (allOn)
                stateString = "ALL";

            if (allOff)
                stateString = "NONE";

            streamXml->setAttribute("recording_state", stateString);

        }
    }
}


void RecordNode::loadCustomParametersFromXml(XmlElement* xml)
{

    RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*) getEditor();

    //Get saved record path
    String savedPath = xml->getStringAttribute("path");

	if (savedPath == "default" || !File(savedPath).exists())
        savedPath = CoreServices::getRecordingParentDirectory().getFullPathName();

    setDataDirectory(File(savedPath));

    setEngine(xml->getStringAttribute("engine", "BINARY"));

    recordEvents = xml->getBoolAttribute("recordEvents", true);
    recordSpikes = xml->getBoolAttribute("recordSpikes", true);


    Array<int> matchingIndexes;
    savedDataStreamParameters.clear();

    for (auto* subNode : xml->getChildIterator())
    {
        if (subNode->hasTagName("STREAM"))
        {

            ParameterCollection* parameterCollection = new ParameterCollection();

            parameterCollection->owner.channel_count = subNode->getIntAttribute("channel_count");
            parameterCollection->owner.name = subNode->getStringAttribute("name");
            parameterCollection->owner.sample_rate = subNode->getDoubleAttribute("sample_rate");
            parameterCollection->owner.channel_count = subNode->getIntAttribute("channel_count");
            parameterCollection->owner.sourceNodeId = subNode->getIntAttribute("source_node_id");

            if (subNode->hasAttribute("device_name"))
                parameterCollection->owner.deviceName = subNode->getStringAttribute("device_name");

            savedDataStreamParameters.add(parameterCollection);
        }
    }

    for (auto stream : dataStreams)
    {

        if (findMatchingStreamParameters(stream) > -1)
        {

			const uint16 streamId = stream->getStreamId();

            for (auto* subNode : xml->getChildIterator())
            {
                if (subNode->hasTagName("STREAM")
					&& subNode->getStringAttribute("name") == stream->getName()
					&& subNode->getIntAttribute("source_node_id") == stream->getSourceNodeId())
                {

					if (subNode->getBoolAttribute("isMainStream", false))
					{
						setMainDataStream(streamId);
					}

					setSyncLine(streamId, subNode->getIntAttribute("sync_line", 0));

					String recordState = subNode->getStringAttribute("recording_state", "ALL");

					for (int ch = 0; ch < recordContinuousChannels[streamId].size(); ch++)
					{
						bool channelState;

						if (recordState.equalsIgnoreCase("ALL"))
						{
							channelState = true;
						}
						else if (recordState.equalsIgnoreCase("NONE"))
						{
							channelState = false;
						}
						else {
							if (recordState.length() > ch)
							{
								channelState = recordState.substring(ch, ch + 1) == "1" ? true : false;
							}
							else {
								channelState = true;
							}
						}

						recordContinuousChannels[streamId][ch] = channelState;
					}

                }
            }


        }
    }

}
