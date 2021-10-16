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

	LOGD("-----------Event Monitor---------");
	LOGD("Received ", receivedEvents, " totalEvents.");
	LOGD("---------------------------------");

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
	recordingNumber(0),
	isRecording(false),
	hasRecorded(false),
	settingsNeeded(false),
	receivedSoftwareTime(false),
	numDataStreams(0)
{
	setProcessorType(PROCESSOR_TYPE_RECORD_NODE);

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
	setEngine(CoreServices::getDefaultRecordEngineIdx() - 1);

	dataDirectory = CoreServices::getDefaultRecordingDirectory();

	recordThread = new RecordThread(this, recordEngine);

	lastDataChannelArraySize = 0;

	eventMonitor = new EventMonitor();

}

RecordNode::~RecordNode()
{
}

void RecordNode::updateBlockSize(int newBlockSize)
{
	if (dataQueue->getBlockSize() != newBlockSize)
		dataQueue = std::make_unique<DataQueue>(newBlockSize, DATA_BUFFER_NBLOCKS);
}

void RecordNode::setEngine(int index)
{
	availableEngines = getAvailableRecordEngines();
	recordEngine = availableEngines[index]->instantiateEngine();
}

void RecordNode::setEngine(String id)
{
	availableEngines = getAvailableRecordEngines();

	for (auto engine : availableEngines)
	{
		if (engine->getID().compare(id) == 0)
			recordEngine = engine->instantiateEngine();
	}
}

std::vector<RecordEngineManager*> RecordNode::getAvailableRecordEngines()
{
	return CoreServices::getAvailableRecordEngines();
}

String RecordNode::generateDirectoryName()
{
	Time calendar = Time::getCurrentTime();

	Array<int> t;
	t.add(calendar.getYear());
	t.add(calendar.getMonth() + 1); // January = 0 
	t.add(calendar.getDayOfMonth());
	t.add(calendar.getHours());
	t.add(calendar.getMinutes());
	t.add(calendar.getSeconds());

	//Any custom text to prepend;
	String filename = AccessClass::getControlPanel()->getTextToPrepend();

	String datestring = "";

	for (int n = 0; n < t.size(); n++)
	{
		if (t[n] < 10)
			datestring += "0";

		datestring += t[n];

		if (n == 2)
			datestring += "_";
		else if (n < 5)
			datestring += "-";
	}

	AccessClass::getControlPanel()->setDateText(datestring);

	if (filename.length() < 24) // this is a hack, to prevent ov
		filename += datestring;
	else
		filename = filename.substring(0, filename.length() - 1);

	filename += AccessClass::getControlPanel()->getTextToAppend();

	return filename;

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
	rootFolder = File(dataDirectory.getFullPathName() + File::getSeparatorString() + generateDirectoryName() + File::getSeparatorString() + getName() + " " + String(getNodeId()));
	newDirectoryNeeded = false;
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
	LOGD(__FUNCTION__, " = ", experimentNumber);
	return experimentNumber;
}

// called by CoreServices
int RecordNode::getRecordingNumber() const
{
	LOGD(__FUNCTION__, " = ", recordingNumber);
	return recordingNumber;
}

// called by ProcessorGraph::createNewProcessor
AudioProcessorEditor* RecordNode::createEditor()
{
	editor = std::make_unique<RecordNodeEditor>(this, true);
	return editor.get();
}

// called by RecordEngine when saving the signal chain
const String &RecordNode::getLastSettingsXml() const
{
	return lastSettingsText;
}

/* Use this function to change parameters while recording...*/
// Called by:
// - EngineConfigComponent (3, 0.0f) -- used to disable record thread (deprecated?)
void RecordNode::setParameter(int parameterIndex, float newValue)
{
	//editor->updateParameterButtons(parameterIndex);

	//if (currentChannel >= 0)
	//{
	//	Parameter* p = parameters[parameterIndex];
	//	p->setValue(newValue, currentChannel);
	//}

}

// Called when deleting FifoMonitor
void RecordNode::updateChannelStates(uint64 streamId, std::vector<bool> channelStates)
{
	this->dataChannelStates[streamId] = channelStates;
}


// called by GenericProcessor, RecordNodeEditor, SourceProcessorInfo,
// TimestampSourceSelectionComponent
//int RecordNode::getNumSubProcessors() const
//{
//	return numSubprocessors;
//}

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
		//return syncChannelMap[srcIndex][subProcIdx];
		//return synchronizer->getSyncChannel(srcIndex, subProcIdx);
		return 0; //FIXME
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

	int count = 0;

	for (auto stream : dataStreams)
	{
		LOGD("Found stream: (", stream->getStreamId(), ") ", stream->getName());
		activeStreamIds.push_back(stream->getStreamId());

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
				count+=stream->getContinuousChannels().size();
			}

		}

	}

	std::cout << "Record Node found " << eventChannels.size() << " event channels " << std::endl;

	// Remove any previous data streams that are no longer coming into the Record Node
	/* Temporarily remove for now */
	/*
	if (activeStreamIds.size() < dataChannelStates.size())
		for (auto const& [id, states] : dataChannelStates)
			if (!std::count(activeStreamIds.begin(), activeStreamIds.end(), id)) 
				dataChannelStates.erase(id);
	*/

	
	//Refresh editor as needed
	if (static_cast<RecordNodeEditor*> (getEditor())->subprocessorsVisible)
	{
		static_cast<RecordNodeEditor*> (getEditor())->showSubprocessorFifos(false);
		static_cast<RecordNodeEditor*> (getEditor())->buttonEvent(static_cast<RecordNodeEditor*> (getEditor())->fifoDrawerButton);
	}

	for (auto const& [ch, order] : dataChannelOrder)
		std::cout << "(" << ch << "," << order << ")" << std::endl;


	/*OLD CODE STARTS HERE

	bool refreshEditor = false;

	std::map<int, std::vector<int>> inputs;

	int updatedNumSubprocessors = 0;
	int originalChannelCount = numChannels;
	int ch = 0;

	while (ch < continuousChannels.size())
	{

		ContinuousChannel* chan = continuousChannels[ch];
		int sourceID = chan->getSourceNodeId();
		int streamId = chan->getStreamId();

		if (inputs.empty() || inputs[sourceID].empty() || inputs[sourceID].back() != streamId)
		{

			//Check if this (src,sub) combo has already been seen and show warning
			if (!inputs.empty() && !inputs[sourceID].empty() 
				&& std::find(inputs[sourceID].begin(), inputs[sourceID].end(), streamId) != inputs[sourceID].end())
			{
				AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
					"WARNING!", "Detected input channels re-mapped from different subprocessors. Please correct the channel mapping or else the RecordNode will crash!");
				return;
			}

			//Found a new subprocessor
			inputs[sourceID].push_back(streamId);
			fifoUsage[sourceID][streamId] = 0.0f;
			updatedNumSubprocessors++;
		}

		//Add any new sources
		if (!dataChannelStates.count(sourceID) || !dataChannelStates[sourceID].count(streamId))
		{
			int orderInSubprocessor = 0;
			synchronizer->addDataStream(chan->getStreamId(), chan->getSampleRate());

			if (synchronizer->primaryProcessorId < 0)
			{
				synchronizer->addDataStream(chan->getStreamId(), chan->getSampleRate());
			}
			while (ch < continuousChannels.size() && continuousChannels[ch]->getStreamId() == streamId
				& continuousChannels[ch]->getSourceNodeId() == sourceID)
			{
				dataChannelStates[sourceID][continuousChannels[ch]->getStreamId()].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				dataChannelOrder[ch] = orderInSubprocessor++;

				//std::cout << " Channel " << ch << ", " << " order: " << dataChannelOrder[ch] << ", record state: " << dataChannelStates[sourceID][subProcIdx].back() << std::endl;


				ch++;


			}
			refreshEditor = true;

			//LOGDD("RecordNode found ", orderInSubprocessor, " channels in ", sourceID, ".", subProcIdx);
		}
		else
		{
			// check if channel count has changed for existing source
			int count = 0;
			int originalSize = dataChannelStates[sourceID][streamId].size();

			for (int i = 0; i < continuousChannels.size(); i++)
			{

				//std::cout << "Channel " << i << ": " << dataChannelArray[i]->getSourceNodeID() << "." << dataChannelArray[i]->getSubProcessorIdx() << std::endl;
				if (continuousChannels[i]->getSourceNodeId() == sourceID && continuousChannels[i]->getStreamId() == streamId)
				{
					dataChannelOrder[ch + count] = count;
					count++;
				}

			}
			//If channel count is greater, add new channels to dataChannelStates
			if (count > originalSize)
			{
				for (int i = 0; i < count - originalSize; i++)
				{
					dataChannelStates[sourceID][streamId].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);

				}
			} //else if less, remove n channels from dataChannelStates
			else if (count < dataChannelStates[sourceID][streamId].size())
			{
				for (int i = 0; i < originalSize - count; i++)
				{
					dataChannelStates[sourceID][streamId].pop_back();
				}
			}
			else
			{
				//else do nothing
			}

			//LOGDD("RecordNode found ", count, " channels in ", sourceID, ".", subProcIdx);

			ch += count;

			//std::cout << " Channel " << ch << " record state: " << dataChannelStates[sourceID][subProcIdx].back() << std::endl;
		}

	}

	//Remove any stale processors
	std::vector<int> sources;
	for (auto const& sourceID : inputs)
		sources.push_back(sourceID.first);
	std::vector<int> toErase;

	std::map<int, std::map<int, std::vector<bool>>>::iterator it;
	for (it = dataChannelStates.begin(); it != dataChannelStates.end(); it++)
	{	
		if (std::find(sources.begin(), sources.end(), it->first) == sources.end())
			toErase.push_back(it->first);
	}

	for (int i = 0; i < toErase.size(); i++)
		dataChannelStates.erase(toErase[i]);

	if (refreshEditor && static_cast<RecordNodeEditor*> (getEditor())->subprocessorsVisible)
	{
		numDataStreams = updatedNumSubprocessors;
		static_cast<RecordNodeEditor*> (getEditor())->showSubprocessorFifos(false);
		static_cast<RecordNodeEditor*> (getEditor())->buttonEvent(static_cast<RecordNodeEditor*> (getEditor())->fifoDrawerButton);
	}

	numDataStreams = updatedNumSubprocessors;

	eventMap.clear();
	syncChannelMap.clear();
	syncOrderMap.clear();

	LOGDD("Record Node: ", getNodeId(), " has ", eventChannels.size(), " event channels");
	for (int ch = 0; ch < eventChannels.size(); ch++)
	{

		EventChannel* chan = eventChannels[ch];
		int sourceID = chan->getSourceNodeId();
		int streamId = chan->getStreamId();

		// WHAT IS THIS USED FOR?
		eventMap[sourceID][streamId] = 256; // chan->getNumChannels();

		if (dataChannelStates[sourceID][streamId].size() && !syncChannelMap[sourceID][streamId])
		{
			syncOrderMap[sourceID][streamId] = ch;
			syncChannelMap[sourceID][streamId] = 0;
			synchronizer->setSyncBit(chan->getStreamId(), ch);
		}

	}
	*/

}

// called by GenericProcessor::enableProcessor
bool RecordNode::startAcquisition()
{

	bool openEphysFormatSelected = static_cast<RecordNodeEditor*> (getEditor())->getSelectedEngineIdx() == 1;

	if (openEphysFormatSelected && getNumInputs() > 300)
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
			"WARNING!", "Open Ephys format does not support > 300 channels. Resetting to Binary format");
		static_cast<RecordNodeEditor*> (getEditor())->engineSelectCombo->setSelectedItemIndex(0);
		setEngine(0);
		return false;
	}

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
				//ftsChannelMap.add(streamId); //Shouldn't be necessary anymore
			}
			channelIndexInRecordNode++;
			channelIndexInSourceProcessor++;
		}

		procInfo.add(pi);

	}


	/* OLD CODE 

	channelMap.clear();
	ftsChannelMap.clear();
	int totChans = continuousChannels.size();
	OwnedArray<RecordProcessorInfo> procInfo;
	Array<int> chanProcessorMap;
	Array<int> chanOrderinProc;
	int lastProcessor = -1;
	int lastStreamId = -1;
	int procIndex = -1;
	int chanSubIdx = 0;

	int streamIdx = -1;

	int recordedProcessorIdx = -1;

	for (int ch = 0; ch < totChans; ++ch)
	{

		ContinuousChannel* chan = continuousChannels[ch];
		int srcIndex = chan->getSourceNodeId();
		int streamId = chan->getStreamId();

		if (dataChannelStates[streamId][dataChannelOrder[ch]])
		{

			//TODO: This logic will not work after a channel mapper with channels mapped from different subprocessors!
			if (chan->getStreamId() != lastStreamId)
			{
				recordedProcessorIdx++;
				startRecChannels.push_back(ch);
				lastStreamId = chan->getStreamId();

				RecordProcessorInfo* pi = new RecordProcessorInfo();
				pi->processorId = chan->getSourceNodeId();
				procInfo.add(pi);

				streamIdx++;
			}

			//FIX: This assumes each data stream within a processor has the same number of channels
			int channelIndexInSourceProcessor = streamIdx * dataChannelStates[streamId].size() + dataChannelOrder[ch];
			channelMap.add(ch);

			procInfo.getLast()->recordedChannels.add(channelMap.size() - 1);
			chanProcessorMap.add(srcIndex);
			chanOrderinProc.add(channelIndexInSourceProcessor);
			ftsChannelMap.add(recordedProcessorIdx);
		}
	}

	*/

	int numRecordedChannels = channelMap.size();
	
	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	recordEngine->registerRecordNode(this);
	recordEngine->resetChannels();
	std::cout << "channelMap size: " << channelMap.size() << std::endl;
	std::cout << "chanProcessorMap size: " << chanProcessorMap.size() << std::endl;
	std::cout << "chanOrderinProc size: " << chanOrderinProc.size() << std::endl;
	std::cout << "procInfo size: " << procInfo.size() << std::endl;
	recordEngine->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
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

	//Only start recording thread if at least one continous OR event channel is enabled
	if (true) //(channelMap.size() || recordEvents)
	{

		/* Got signal from plugin-GUI to start recording */
		if (newDirectoryNeeded)
		{
			createNewDirectory();
			recordingNumber = 0;
			experimentNumber = 1;
			settingsNeeded = true;
			recordEngine->directoryChanged();
		}
		else
		{
			recordingNumber++; // increment recording number within this directory
		}

		if (!rootFolder.exists())
		{
			rootFolder.createDirectory();
		}

		useSynchronizer = static_cast<RecordNodeEditor*> (getEditor())->getSelectedEngineIdx() == 0;

		recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);

		//LOGD("Num event channels: ", eventChannelArray.size());

		recordThread->startThread();
		isRecording = true;

		if (settingsNeeded)
		{
			String settingsFileName = rootFolder.getFullPathName() + File::getSeparatorString() + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String()) + ".xml";
			AccessClass::getEditorViewport()->saveState(File(settingsFileName), lastSettingsText);
			settingsNeeded = false;
		}
	}
	else
		isRecording = false;

	getEditor()->setBackgroundColor(Colour(255, 0, 0)); // ensure that it's red

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

	//Does not receive message events.:q
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

	LOGD("Record node got spike!");
    
	SpikePtr newSpike = Spike::deserialize(packet, spikeInfo);

	if (!newSpike) 
	{
		LOGD("Unable to deserialize spike event!");
		return;
	}

	if (recordSpikes)
		writeSpike(newSpike, spikeInfo);
	

}

void RecordNode::handleTimestampSyncTexts(const EventPacket& packet)
{
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

			ContinuousChannel* chan = continuousChannels[channelMap[ch]];

			uint64 streamId = ((ChannelInfoObject*)chan)->getStreamId();

			//Check if the source stream has changed
			if (streamId != currentStreamId)
			{
				numSamples = getNumSamples(channelMap[ch]);
				timestamp = getTimestamp(channelMap[ch]);
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
					}
					else
					{
						fifoUsage[streamId] = dataQueue->writeChannel(buffer, channelMap[ch], ch, numSamples, timestamp);
						samplesWritten+=numSamples;
						continue;
					}

					currentStreamId = streamId;
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

// called by ProcessorGraph::connectProcessors
void RecordNode::registerProcessor(const GenericProcessor* sourceNode)
{
	//settings.numInputs += sourceNode->getNumOutputs();
	//setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);
	recordEngine->registerProcessor(sourceNode);
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
