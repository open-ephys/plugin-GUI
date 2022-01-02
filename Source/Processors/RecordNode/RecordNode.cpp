#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "../../Processors/MessageCenter/MessageCenterEditor.h"
#include "BinaryFormat/BinaryRecording.h"
#include "OpenEphysFormat/OriginalRecording.h"
#include "../../Audio/AudioComponent.h"
#include "../../AccessClass.h"

#include "../Events/Spike.h"
#include "../Settings/DataStream.h"

using namespace std::chrono;

#define CONTINUOUS_CHANNELS_ON_BY_DEFAULT true
#define RECEIVED_SOFTWARE_TIME (event.getVelocity() == 136)

EventMonitor::EventMonitor()
	: receivedEvents(0) {}

EventMonitor::~EventMonitor() {}

void EventMonitor::displayStatus()
{
	LOGD("Record Node received ", receivedEvents, " total events");
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
	receivedSoftwareTime(false),
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

	lastPrimaryStreamTimestamp = 0;
	
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
        std::cout << "Invalid key" << std::endl;

    return "Record Node received config: " + msg;
}

void RecordNode::handleBroadcastMessage(String msg)
{

	TextEventPtr event = TextEvent::createTextEvent(getMessageChannel(), lastPrimaryStreamTimestamp, msg);

	size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

	HeapBlock<char> buffer(size);

	event->serialize(buffer, size);

	handleEvent(getMessageChannel(), EventPacket(buffer, size), lastPrimaryStreamTimestamp);
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

	recordEngine->directoryChanged();

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
	this->dataChannelStates[streamId] = channelStates;
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setPrimaryDataStream(uint16 streamId)
{
	LOGD("Setting ", streamId, " as primary");
	synchronizer->setPrimaryDataStream(streamId);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setSyncBit(uint16 streamId, int bit)
{
	synchronizer->setSyncBit(streamId, bit);
}

// called by SyncControlButton
int RecordNode::getSyncBit(uint16 streamId)
{
	return synchronizer->getSyncBit(streamId);
}

// called by SyncControlButton
bool RecordNode::isPrimaryDataStream(uint16 streamId)
{
	return (streamId == synchronizer->primaryStreamId);
}

// called by GenericProcessor::update()
void RecordNode::updateSettings()
{

	activeStreamIds.clear();
	synchronizer->prepareForUpdate();

	int count = 0;

	for (auto stream : dataStreams)
	{
		LOGD("Record Node found stream: (", stream->getStreamId(), ") ", stream->getName());
		activeStreamIds.add(stream->getStreamId());

		//Check for new streams coming into record node
		if (dataChannelStates[stream->getStreamId()].empty())
		{

			//Register the new data stream 
			synchronizer->addDataStream(stream->getStreamId(), stream->getSampleRate());
			fifoUsage[stream->getStreamId()] = 0.0f;

			for (auto channel : stream->getContinuousChannels())
			{
				dataChannelStates[stream->getStreamId()].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				dataChannelOrder[count] = channel->getLocalIndex();
				count++;
			}

		}
		else
		{
			//An existing data stream has changed its number of input channels
			if (dataChannelStates[stream->getStreamId()].size() != stream->getChannelCount())
			{
				dataChannelStates[stream->getStreamId()].clear();
				for (auto channel : stream->getContinuousChannels())
				{
					dataChannelStates[stream->getStreamId()].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
					dataChannelOrder[count] = channel->getLocalIndex();
					count++;
				}
			}
			else //Found existing data stream, only need to increment channel counter
			{
				count += stream->getContinuousChannels().size();
			}

		}

	}

	for (auto it = dataChannelStates.begin(); it != dataChannelStates.end(); ) {
		if (!activeStreamIds.contains(it->first))
			it = dataChannelStates.erase(it);
		else
			++it;
	}

	if (recordEngine->getEngineId().equalsIgnoreCase("OPENEPHYS") && getNumInputs() > 300)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
			"WARNING", "Open Ephys format does not support > 300 channels. Resetting to Binary format.");
		setEngine("BINARY");
	}

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
    eventChannels.getLast()->setDataStream(getDataStream(synchronizer->primaryStreamId), false);

	if (hasRecorded)
	{
		hasRecorded = false;
		experimentNumber++;
		settingsNeeded = true;
	}

	recordingNumber = -1;
	recordEngine->configureEngine();
	recordEngine->startAcquisition();

    synchronizer->reset();
    return true;

}

bool RecordNode::stopAcquisition()
{

	// Remove message channel
	eventChannels.removeLast();

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

	for (auto stream : dataStreams)
	{

		RecordProcessorInfo* pi = new RecordProcessorInfo();
		ContinuousChannel* firstChannel = stream->getContinuousChannels()[0];
		pi->processorId = stream->getSourceNodeId();

		if (stream->getSourceNodeId() != lastSourceNodeId)
		{
			channelIndexInSourceProcessor = 0;
			lastSourceNodeId = stream->getSourceNodeId();
		}

		for (auto recordState : dataChannelStates[stream->getStreamId()])
		{

			if (recordState)
			{
				channelMap.add(channelIndexInRecordNode);
				pi->recordedChannels.add(channelMap.size() - 1);
				chanProcessorMap.add(stream->getSourceNodeId());
				chanOrderinProc.add(channelIndexInSourceProcessor);
			}

			channelIndexInRecordNode++;
			channelIndexInSourceProcessor++;
		}

		procInfo.add(pi);

	}

	int numRecordedChannels = channelMap.size();
	
	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	recordEngine->registerRecordNode(this);
	recordEngine->resetChannels();
	recordEngine->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
	LOGD("channelMap size: ", channelMap.size());
	LOGD("chanProcessorMap size: ", chanProcessorMap.size());
	LOGD("chanOrderinProc size: ", chanOrderinProc.size());
	LOGD("procInfo size: ", procInfo.size());
	
	recordThread->setChannelMap(channelMap);
	recordThread->setFTSChannelMap(ftsChannelMap);

	dataQueue->setChannels(numRecordedChannels);
	dataQueue->setFTSChannels(dataStreams.size());

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

	useSynchronizer = recordEngine->getEngineId().equalsIgnoreCase("BINARY");

	recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);
	recordThread->startThread();
	isRecording = true;

	if (settingsNeeded)
	{
		String settingsFileName = rootFolder.getFullPathName() + File::getSeparatorString() + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String()) + ".xml";
		AccessClass::getEditorViewport()->saveState(File(settingsFileName), lastSettingsText);
		settingsNeeded = false;
	}

	//getEditor()->setBackgroundColor(Colour(255, 0, 0)); // ensure that it's red

}

// called by GenericProcessor::setRecording() and CoreServices::setRecordingStatus()
void RecordNode::stopRecording()
{

	isRecording = false;

	if (recordThread->isThreadRunning())
	{
		recordThread->signalThreadShouldExit();
		recordThread->waitForThreadToExit(2000);
	}

	eventMonitor->displayStatus();

	if (CoreServices::getRecordingStatus())
		getEditor()->setBackgroundColor(Colour(128, 41, 41)); // turn it brown if it's not recording while recording is active
	else
		getEditor()->setBackgroundColor(Colour(255, 0, 0));

	receivedSoftwareTime = false;

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

void RecordNode::handleEvent(const EventChannel* eventInfo, const EventPacket& packet, int samplePosition)
{

	// Only handles TTL events (TEXT events are handled separately)

	eventMonitor->receivedEvents++;

	if (recordEvents) 
	{

		//TODO: Test these two should be the same?
		const int eventTime = samplePosition;
		int64 timestamp = Event::getTimestamp(packet);

		if (eventInfo != nullptr && eventInfo->getType() == EventChannel::TTL)
		{
			TTLEventPtr ttl = TTLEvent::deserialize(packet, eventInfo);

			if (samplePosition >= 0)
				synchronizer->addEvent(Event::getStreamId(packet), ttl->getBit(), timestamp);
		
		}

		if (isRecording)
		{
			int eventIndex;
			if (SystemEvent::getBaseType(packet) == EventBase::Type::SYSTEM_EVENT)
				eventIndex = -1;
			else
				eventIndex = getIndexOfMatchingChannel(eventInfo);
			
			eventQueue->addEvent(packet, timestamp, eventIndex);
		}

	}

}

// only called if recordSpikes is true
void RecordNode::handleSpike(const SpikeChannel* spikeInfo, const EventPacket& packet, int samplePosition)
{

	SpikePtr newSpike = Spike::deserialize(packet, spikeInfo);

	if (!newSpike) 
	{
		LOGE("Unable to deserialize spike event!");
		return;
	}

	if (recordSpikes)
		writeSpike(newSpike, spikeInfo);
	

}

void RecordNode::handleTimestampSyncTexts(const EventPacket& packet)
{
	std::cout << "Record Node " << getNodeId() << " writing sync timestamp " << std::endl;
	handleEvent(nullptr, packet, 0);
}
	
void RecordNode::process(AudioBuffer<float>& buffer)

{

	isProcessing = true;

	checkForEvents(recordSpikes);

	if (isRecording)
	{

		uint64 currentStreamId = -1;

		//For each channel that is to be recorded 
		for (int ch = 0; ch < channelMap.size(); ch++)
		{

			if (!receivedSoftwareTime)
			{
				MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
				HeapBlock<char> data;

				size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, this, 0, CoreServices::getSoftwareTimestamp(), true);

				handleTimestampSyncTexts(EventPacket(data, dataSize));

				receivedSoftwareTime = true;

			}

			ContinuousChannel* chan = continuousChannels[channelMap[ch]];

			uint64 streamId = ((ChannelInfoObject*)chan)->getStreamId();

			//Check if the source stream has changed
			if (streamId != currentStreamId)
			{
				numSamples = getNumSamples(channelMap[ch]);
				timestamp = getTimestamp(channelMap[ch]);

				if (!setFirstBlock)
				{

					MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
					HeapBlock<char> data;

					GenericProcessor* src = AccessClass::getProcessorGraph()->getProcessorWithNodeId(getDataStream(streamId)->getSourceNodeId());

					size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, src, streamId, timestamp, false);

					handleTimestampSyncTexts(EventPacket(data, dataSize));

				}
			}

			bool shouldWrite = validBlocks[ch];

			if (!shouldWrite && numSamples > 0)
			{
				shouldWrite = true;
				validBlocks.set(ch, true);
			}

			if (shouldWrite && numSamples > 0)
			{

				if (streamId != currentStreamId)
				{

					if (useSynchronizer)
					{
						double first = synchronizer->convertTimestamp(streamId, timestamp);
						double second = synchronizer->convertTimestamp(streamId, timestamp + 1);
						fifoUsage[streamId] = dataQueue->writeSynchronizedTimestampChannel(first, second - first, ftsChannelMap[ch], numSamples);

						if (streamId == synchronizer->primaryStreamId)
							lastPrimaryStreamTimestamp = timestamp;

						currentStreamId = streamId;

					}
					else
					{
						fifoUsage[streamId] = dataQueue->writeChannel(buffer, channelMap[ch], ch, numSamples, timestamp);
						samplesWritten+=numSamples;

						lastPrimaryStreamTimestamp = timestamp;
						
						currentStreamId = streamId;
						
						continue;
					}

				}

				dataQueue->writeChannel(buffer, channelMap[ch], ch, numSamples, timestamp);
				samplesWritten+=numSamples;

			}

		}

		if (!setFirstBlock)
		{
			bool shouldSetFlag = true;
			for (int chan = 0; chan < channelMap.size(); ++chan)
			{
				if (!validBlocks[chan])
				{
					shouldSetFlag = false;
					break;
				}
			}
			if (shouldSetFlag)
			{
				recordThread->setFirstBlockFlag(true);
				setFirstBlock = true;
			}
		}

	}

}

// called by process method
bool RecordNode::isFirstChannelInRecordedSubprocessor(int ch)
{
	return std::find(startRecChannels.begin(), startRecChannels.end(), ch) != startRecChannels.end();
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
