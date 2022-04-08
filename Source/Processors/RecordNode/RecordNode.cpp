#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "../../Processors/MessageCenter/MessageCenterEditor.h"
#include "BinaryFormat/BinaryRecording.h"
#include "../../Audio/AudioComponent.h"
#include "../../AccessClass.h"

#include "../Events/Spike.h"
#include "../Settings/DataStream.h"

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
	timestamp(0),
	setFirstBlock(false),
	numChannels(0),
	numSamples(0),
	samplesWritten(0),
	experimentNumber(0),
	recordingNumber(-1),
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

	synchronizer = new Synchronizer(this);

	isSyncReady = true;

	/* New record nodes default to the record engine currently selected in the Control Panel */
	setEngine(CoreServices::getDefaultRecordEngineId());

	dataDirectory = CoreServices::getRecordingParentDirectory();

	recordThread = new RecordThread(this, recordEngine);

	lastMainStreamTimestamp = 0;

	lastDataChannelArraySize = 0;

	eventMonitor = new EventMonitor();

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

	TextEventPtr event = TextEvent::createTextEvent(getMessageChannel(), lastMainStreamTimestamp, msg);

	size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

	HeapBlock<char> buffer(size);

	event->serialize(buffer, size);

	handleEvent(getMessageChannel(), EventPacket(buffer, size));
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
			recordEngine = engine->instantiateEngine();
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
}

// called by RecordNode::startRecording
void RecordNode::createNewDirectory()
{
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

	recordingNumber = -1;
	experimentNumber = 1;
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
	LOGD("Current experiment = ", experimentNumber);
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
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setMainDataStream(uint16 streamId)
{
	LOGD("Setting ", streamId, " as the main stream");
	synchronizer->setMainDataStream(streamId);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setSyncLine(uint16 streamId, int line)
{
	synchronizer->setSyncLine(streamId, line);
}

// called by SyncControlButton
int RecordNode::getSyncLine(uint16 streamId)
{
	return synchronizer->getSyncLine(streamId);
}

// called by SyncControlButton
bool RecordNode::isMainDataStream(uint16 streamId)
{
	return (streamId == synchronizer->mainStreamId);
}

// called by GenericProcessor::update()
void RecordNode::updateSettings()
{

	activeStreamIds.clear();
	synchronizer->prepareForUpdate();

	for (auto stream : dataStreams)
	{
		const uint16 streamId = stream->getStreamId();
		activeStreamIds.add(streamId);

		LOGD("Record Node found stream: (", streamId, ") ", stream->getName());
		//activeStreamIds.add(stream->getStreamId());
		synchronizer->addDataStream(streamId,
									stream->getSampleRate());

		fifoUsage[streamId] = 0.0f;

		if (recordContinuousChannels[streamId].empty()) // this ID has not been seen yet
		{
			for (auto channel : stream->getContinuousChannels())
			{
				recordContinuousChannels[streamId].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				channel->isRecorded = CONTINUOUS_CHANNELS_ON_BY_DEFAULT;
				LOGD("Channel ", channel->getName(), ": ", channel->isRecorded);
				LOGD(recordContinuousChannels[streamId].size())
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

				LOGD("Channel ", channel->getName(), ": ", channel->isRecorded);

				localIndex++;
				//dataChannelOrder[count] = channel->getLocalIndex();
				//count++;
			}

		}

	}

	synchronizer->finishedUpdate();


	// get rid of unused IDs
	for (auto it = recordContinuousChannels.begin(); it != recordContinuousChannels.end(); ) {
		if (!activeStreamIds.contains(it->first))
			it = recordContinuousChannels.erase(it);
		else
			++it;
	}

#ifdef WIN32
	// check Open Ephys format on windows
	if (recordEngine->getEngineId().equalsIgnoreCase("OPENEPHYS") && getNumInputs() > 300)
	{
		int new_max = 0;

		if (getNumInputs() < 8192) // actual upper bound of 8192
			new_max = _setmaxstdio(getNumInputs());

		if (new_max != getNumInputs())
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

// called by GenericProcessor::enableProcessor
bool RecordNode::startAcquisition()
{

    eventChannels.add(new EventChannel(*messageChannel));
    eventChannels.getLast()->addProcessor(processorInfo.get());
    eventChannels.getLast()->setDataStream(getDataStream(synchronizer->mainStreamId), false);

	if (hasRecorded)
	{
		hasRecorded = false;
		experimentNumber++;
		settingsNeeded = true;
	}

	recordingNumber = -1;
	recordEngine->configureEngine();
	synchronizer->reset();
	eventMonitor->reset();

    return true;

}

bool RecordNode::stopAcquisition()
{

	// Remove message channel
	eventChannels.removeLast();

	eventMonitor->displayStatus();

	return true;
}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::startRecording()
{

	Array<int> chanProcessorMap;
	Array<int> chanOrderinProc;
	OwnedArray<RecordProcessorInfo> procInfo;

	int lastSourceNodeId = -1;

	int channelIndexInRecordNode = 0;
	int channelIndexInSourceProcessor = 0;

	channelMap.clear();
	timestampChannelMap.clear();

	int streamIndex = 0;

	for (auto stream : dataStreams)
	{

		RecordProcessorInfo* pi = new RecordProcessorInfo();
		pi->processorId = stream->getSourceNodeId();

		if (stream->getSourceNodeId() != lastSourceNodeId)
		{
			channelIndexInSourceProcessor = 0;
			lastSourceNodeId = stream->getSourceNodeId();
		}

		for (auto channel : stream->getContinuousChannels())
		{
			if (channel->isRecorded)
			{
				LOGD("Channel map: ", channelIndexInRecordNode);
				LOGD("timestampChannelMap: ", streamIndex);

				channelMap.add(channelIndexInRecordNode);
				timestampChannelMap.add(streamIndex);
				pi->recordedChannels.add(channelMap.size() - 1);
				chanProcessorMap.add(stream->getSourceNodeId());
				chanOrderinProc.add(channelIndexInSourceProcessor);
			}

			channelIndexInRecordNode++;
			channelIndexInSourceProcessor++;
		}

		procInfo.add(pi);
		streamIndex++;

	}

	int numRecordedChannels = channelMap.size();

	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	recordEngine->registerRecordNode(this);
	recordEngine->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
	LOGD("channelMap size: ", channelMap.size());
	LOGD("chanProcessorMap size: ", chanProcessorMap.size());
	LOGD("chanOrderinProc size: ", chanOrderinProc.size());
	LOGD("procInfo size: ", procInfo.size());

	recordThread->setChannelMap(channelMap);
	recordThread->setTimestampChannelMap(timestampChannelMap);

	dataQueue->setChannelCount(numRecordedChannels);
	dataQueue->setTimestampStreamCount(dataStreams.size());

	eventQueue->reset();
	spikeQueue->reset();

	recordThread->setQueuePointers(dataQueue.get(), eventQueue.get(), spikeQueue.get());
	recordThread->setFirstBlockFlag(false);

	hasRecorded = true;

	/* Set write properties */
	setFirstBlock = false;

	recordingNumber++; // increment recording number within this directory

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

	int64 timestamp = event->getTimestamp();

	synchronizer->addEvent(event->getStreamId(), event->getLine(), timestamp);

	if (recordEvents && isRecording)
	{

		size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

		HeapBlock<char> buffer(size);
		event->serialize(buffer, size);

		eventQueue->addEvent(EventPacket(buffer, size), timestamp);

		eventMonitor->bufferedEvents++;

	}

}

void RecordNode::handleEvent(const EventChannel* eventInfo, const EventPacket& packet)
{

	if (recordEvents && isRecording)
	{

	    int64 timestamp = Event::getTimestamp(packet);

		int eventIndex;
		if (SystemEvent::getBaseType(packet) == EventBase::Type::SYSTEM_EVENT)
			eventIndex = -1;
		else
			eventIndex = getIndexOfMatchingChannel(eventInfo);

		eventQueue->addEvent(packet, timestamp, eventIndex);

	}

}

// only called if recordSpikes is true
void RecordNode::handleSpike(SpikePtr spike)
{

	eventMonitor->receivedSpikes++;

	if (recordSpikes)
	{
		writeSpike(spike, spike->getChannelInfo());
		eventMonitor->bufferedSpikes++;
	}


}

void RecordNode::handleTimestampSyncTexts(const EventPacket& packet)
{
	//std::cout << "Record Node " << getNodeId() << " writing sync timestamp " << std::endl;
	handleEvent(nullptr, packet);
}

void RecordNode::writeInitialEventStates()
{

	for (auto& channel : eventChannels) {

		if (channel->getType() == EventChannel::TTL) {

			for (int i = 0; i < channel->getMaxTTLBits(); i++)
			{
				TTLEventPtr event = TTLEvent::createTTLEvent(channel, 0, i, false, 0);

				size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

				HeapBlock<char> buffer(size);

				event->serialize(buffer, size);

				handleEvent(channel, EventPacket(buffer, size));
			}

		}

	}

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

			size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, this, 0, CoreServices::getSoftwareTimestamp(), true);

			handleTimestampSyncTexts(EventPacket(data, dataSize));

			writeInitialEventStates();

			for (auto stream : getDataStreams())
			{

				const uint16 streamId = stream->getStreamId();

				numSamples = getNumSourceSamples(streamId);
				timestamp = getSourceTimestamp(streamId);

				MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
				HeapBlock<char> data;

				GenericProcessor* src = AccessClass::getProcessorGraph()->getProcessorWithNodeId(getDataStream(streamId)->getSourceNodeId());

				size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, src, streamId, timestamp, false);

				handleTimestampSyncTexts(EventPacket(data, dataSize));
			}
		}

		bool fifoAlmostFull = false;

		int streamIndex = -1;
		int channelIndex = -1;

		for (auto stream : getDataStreams())
		{

			streamIndex++;

			const uint16 streamId = stream->getStreamId();

			numSamples = getNumSourceSamples(streamId);
			timestamp = getSourceTimestamp(streamId);

			if (numSamples == 0)
				continue;

			double first = synchronizer->convertTimestamp(streamId, timestamp);
			double second = synchronizer->convertTimestamp(streamId, timestamp + 1);

			fifoUsage[streamId] = dataQueue->writeSynchronizedTimestamps(
					first,
					second - first,
					streamIndex,
					numSamples);

			for (auto channel : stream->getContinuousChannels())
			{

				if (channel->isRecorded)
				{

					channelIndex++;

					dataQueue->writeChannel(buffer,
											channelMap[channelIndex],
											channelIndex,
											numSamples,
											timestamp);

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

	if (true)
	{
		int processorId = spike->getProcessorId();
		int streamId = spike->getStreamId();
		int idx = spike->getChannelIndex();

		int electrodeIndex = getIndexOfMatchingChannel(spikeElectrode);

		if (electrodeIndex >= 0)
			spikeQueue->addEvent(*spike, spike->getTimestamp(), electrodeIndex);
	}
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
