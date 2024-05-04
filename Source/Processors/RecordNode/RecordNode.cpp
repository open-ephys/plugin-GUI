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

	//checkDiskSpace();

}

RecordNode::~RecordNode()
{
}

void RecordNode::registerParameters()
{
	String defaultRecordDirectory = CoreServices::getRecordingParentDirectory().getFullPathName();
	addPathParameter(Parameter::PROCESSOR_SCOPE, "directory", "Directory", "Path to write data to", defaultRecordDirectory, {}, true);
	addCategoricalParameter(Parameter::PROCESSOR_SCOPE, "engine", "Engine", "Recording data format", {}, 0, true);
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "events", "Record Events", "Toggle saving events coming into this node", true);
	addBooleanParameter(Parameter::PROCESSOR_SCOPE, "spikes", "Record Spikes", "Toggle saving spikes coming into this node", true);

	Array<String> recordEngines;
	std::vector<RecordEngineManager*> engines = getAvailableRecordEngines();
	for (int i = 0; i < engines.size(); i++)
		recordEngines.add(engines[i]->getName());
	CategoricalParameter* engineParam = (CategoricalParameter*)getParameter("engine");
	engineParam->setCategories(recordEngines);

	addMaskChannelsParameter(Parameter::STREAM_SCOPE, "channels", "Channels", "Channels to record from", true);
	addTtlLineParameter(Parameter::STREAM_SCOPE, "sync_line", "Sync Line", "Event line to use for sync signal", 8, true, false, true);
	addSelectedStreamParameter(Parameter::PROCESSOR_SCOPE, "main_sync", "Main Sync Stream", "Use this stream as main sync", {}, 0, true);
}

void RecordNode::initialize(bool signalChainIsLoading)
{
	if (!signalChainIsLoading)
		checkDiskSpace();
}

void RecordNode::parameterValueChanged(Parameter* p)
{
	if (p->getName() == "directory")
	{
		String newPath = static_cast<PathParameter*>(p)->getValue();
		if (newPath == "default")
			newPath = CoreServices::getRecordingParentDirectory().getFullPathName();
		setDataDirectory(File(newPath));
	}
	else if (p->getName() == "engine")
	{
		LOGD("Parameter changed: engine");
		setEngine(((CategoricalParameter*)p)->getSelectedString());
	}
	else if (p->getName() == "events")
	{
		setRecordEvents(((BooleanParameter*)p)->getBoolValue());
	}
	else if (p->getName() == "spikes")
	{
		setRecordSpikes(((BooleanParameter*)p)->getBoolValue());
	}
	else if (p->getName() == "channels")
	{
		LOGD("Parameter changed: channels");
	}
	else if (p->getName() == "sync_line")
	{
		LOGD("Parameter changed: sync_line");
		synchronizer.setSyncLine(getDataStream(p->getStreamId())->getKey(), ((TtlLineParameter*)p)->getSelectedLine());
	}
	else if (p->getName() == "main_sync")
	{
		LOGD("Parameter changed: main_sync");
		Array<String> streamNames = ((SelectedStreamParameter*)p)->getStreamNames();
		for (auto stream : dataStreams)
		{
			String key = stream->getKey();
			if (key == streamNames[((SelectedStreamParameter*)p)->getSelectedIndex()])
			{
				synchronizer.setMainDataStream(stream->getKey());
				break;
			}
		}
	}
	else
	{
		LOGD("RecordNode: unknown parameter changed: ", p->getName());
	}

}

void RecordNode::checkDiskSpace()
{
	float diskSpaceWarningThreshold = 5; //GB

	File dataDir(dataDirectory);
	int64 freeSpace = dataDir.getBytesFreeOnVolume();

	float availableBytes = freeSpace / pow(2, 30); //1 GB == 2^30 bytes

	if (availableBytes < diskSpaceWarningThreshold && !isRecording)
	{
		String msg = "Record Node " + String(getNodeId());
		msg += "\n\n";
		msg += "Less than " + String(int(diskSpaceWarningThreshold)) + " GB of disk space available in:\n";
		msg += "\n";
		msg += "\t" + String(dataDirectory.getFullPathName());
		msg += "\n\n";
		msg += "Recording may fail. Please free up space or change the recording directory.";

		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "WARNING", msg);
	}
}

String RecordNode::handleConfigMessage(String msg)
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

	LOGD(tokens[0]);

	if (tokens[0] == "engine")
	{
		if (tokens.size() == 2)
		{
			RecordNodeEditor* ed = static_cast<RecordNodeEditor*> (getEditor());
			
			int engineIndex = tokens[1].getIntValue();

			int numEngines = ((CategoricalParameter*)getParameter("engine"))->getCategories().size();

			if (engineIndex >= 0 && engineIndex < numEngines)
			{
				//ed->engineSelectCombo->setSelectedItemIndex(engineIndex, sendNotification);
				getParameter("engine")->setNextValue(engineIndex);
				return "Record Node: updated record engine to " + ((CategoricalParameter*)getParameter("engine"))->getCategories()[engineIndex];
			}
			else {
				return "Record Node: invalid engine index (max = " + String(numEngines - 1) + ")";
			}
			
		}
		else
		{
			return "Record Node: invalid engine key";
		}
	}

	tokens.clear();
	tokens.addTokens(msg, " ", "");

	LOGD(tokens[0]);

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
			else {
				return "Record Node: Invalid stream index; max = " + String(dataStreams.size() - 1);
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
					channelStates.add(i);
				}
			}
			else
			{
				
				Array<int> channels;

				for (int i = 2; i < tokens.size(); i++)
				{
					int ch = tokens[i].getIntValue() - 1;
					channels.add(ch);
				}

				//select some channels
				for (int i = 0; i < channelCount; i++)
				{
					if (channels.contains(i))
						channelStates.add(i);
				}
			}

			//updateChannelStates(streamId, channelStates);
			DataStream* stream = getDataStream(streamId);
			MaskChannelsParameter* maskChannels = (MaskChannelsParameter*)stream->getParameter("channels");
			maskChannels->setNextValue(channelStates);
		}
		else
		{
			LOGD("Record Node: invalid config message");
		}
	}

    return "Record Node received config: " + msg;
}

void RecordNode::handleBroadcastMessage(String msg)
{

    if (recordEvents && isRecording)
    {

		String streamKey = synchronizer.mainStreamKey;

		DataStream* mainStream = getDataStream(streamKey);

        int64 messageSampleNumber = getFirstSampleNumberForBlock(mainStream->getStreamId());

        TextEventPtr event = TextEvent::createTextEvent(getMessageChannel(), messageSampleNumber, msg);

        double ts = synchronizer.convertSampleNumberToTimestamp(synchronizer.mainStreamKey, messageSampleNumber);

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
		if (engine->getName().equalsIgnoreCase(id))
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

		//parameterValueChanged(stream->getParameter("sync_line"));

		const uint16 streamId = stream->getStreamId();
		activeStreamIds.add(streamId);

		LOGD("Record Node found stream: (", streamId, ") ", stream->getName());
		//activeStreamIds.add(stream->getStreamId());
		synchronizer.addDataStream(stream->getKey(), stream->getSampleRate());

		fifoUsage[streamId] = 0.0f;
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

}

bool RecordNode::isSynchronized()
{

    if (dataStreams.size() == 1) // no need to sync only one DataStream
        return true;

    for (auto stream : dataStreams)
    {

        SyncStatus status = synchronizer.getStatus(stream->getKey());

        if (status != SYNCED)
            return false;
    }

    return true;
}

// called by GenericProcessor::enableProcessor
bool RecordNode::startAcquisition()
{

	recordEvents = ((BooleanParameter*)getParameter("events"))->getBoolValue();
	recordSpikes = ((BooleanParameter*)getParameter("spikes"))->getBoolValue();

    synchronizer.startAcquisition();

    if (eventChannels.size() == 0 || eventChannels.getLast()->getSourceNodeName() != "Message Center")
    {
        eventChannels.add(new EventChannel(*messageChannel));
        eventChannels.getLast()->addProcessor(this);
        eventChannels.getLast()->setDataStream(getDataStream(synchronizer.mainStreamKey), false);
    }
    
    return true;

}

bool RecordNode::stopAcquisition()
{

    synchronizer.stopAcquisition();

    if (hasRecorded) {
        // stopRecording() signals the thread to exit, but we should wait here until the thread actually gracefully
        // exits before we reset some of its needed data (e.g. eventQueue, spikeQueue, etc.)
        if (recordThread) {
            recordThread->waitForThreadToExit(1000);
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
        eventChannels.add(new EventChannel(*messageChannel));
        eventChannels.getLast()->addProcessor(this);
        eventChannels.getLast()->setDataStream(getDataStream(synchronizer.mainStreamKey), false);
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

		for (auto channelRecordState : ((MaskChannelsParameter*)stream->getParameter("channels"))->getChannelStates())
		{
			if (channelRecordState)
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
        
        //std::unique_ptr<XmlElement> xml = std::make_unique<XmlElement>("SETTINGS");
        
		//AccessClass::getProcessorGraph()->saveToXml(File(settingsFileName), lastSettingsText);
		
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

	String streamKey = getDataStream(event->getStreamId())->getKey();

	synchronizer.addEvent(streamKey, event->getLine(), sampleNumber);

	if (recordEvents && isRecording)
	{

		size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

		HeapBlock<char> buffer(size);
        event->setTimestampInSeconds(synchronizer.convertSampleNumberToTimestamp(streamKey, sampleNumber));
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

		String streamKey = getDataStream(eventInfo->getStreamId())->getKey();

        Event::setTimestampInSeconds(packet, synchronizer.convertSampleNumberToTimestamp(streamKey, sampleNumber));

		eventQueue->addEvent(packet, sampleNumber, eventIndex);

	}

}

// only called if recordSpikes is true
void RecordNode::handleSpike(SpikePtr spike)
{

	eventMonitor->receivedSpikes++;

	if (recordSpikes)
	{
		String streamKey = getDataStream(spike->getStreamId())->getKey();
        spike->setTimestampInSeconds(synchronizer.convertSampleNumberToTimestamp(streamKey,
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
			const String streamKey = stream->getKey();

            uint32 numSamples = getNumSamplesInBlock(streamId);

			int64 sampleNumber = getFirstSampleNumberForBlock(streamId);

			if (numSamples > 0)
			{
				double first = synchronizer.convertSampleNumberToTimestamp(streamKey, sampleNumber);
				double second = synchronizer.convertSampleNumberToTimestamp(streamKey, sampleNumber + 1);

				fifoUsage[streamId] = dataQueue->writeSynchronizedTimestamps(
					first,
					second - first,
					streamIndex,
					numSamples);

			}

			for (auto channelRecordState : ((MaskChannelsParameter*)stream->getParameter("channels"))->getChannelStates())
			{

				if (channelRecordState)
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

void RecordNode::saveCustomParametersToXml(XmlElement* xml)
{
	if (!headlessMode)
    {
        RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*) getEditor();
        xml->setAttribute("fifoMonitorsVisible", recordNodeEditor->fifoDrawerButton->getToggleState());
    }
}

void RecordNode::loadCustomParametersFromXml(XmlElement* xml)
{
	if (xml->hasAttribute("fifoMonitorsVisible"))
	{
		if (!headlessMode)
		{
			RecordNodeEditor* recordNodeEditor = (RecordNodeEditor*)getEditor();
			if (!xml->getBoolAttribute("fifoMonitorsVisible"))
				recordNodeEditor->fifoDrawerButton->triggerClick();
		}
	}
}
