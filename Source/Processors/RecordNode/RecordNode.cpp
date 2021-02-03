#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "../../Processors/MessageCenter/MessageCenterEditor.h"
#include "BinaryFormat/BinaryRecording.h"
#include "OpenEphysFormat/OriginalRecording.h"

#include "../../AccessClass.h"

using namespace std::chrono;

#define CONTINUOUS_CHANNELS_ON_BY_DEFAULT true
#define RECEIVED_SOFTWARE_TIME (event.getVelocity() == 136)

EventMonitor::EventMonitor()
	: receivedEvents(0) {}

EventMonitor::~EventMonitor() {}

void EventMonitor::displayStatus()
{

	LOGD("-----------Event Monitor---------");
	LOGD("Received events: ", receivedEvents);
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
    numSubprocessors(0),
	isConnectedToMessageCenter(false)
{
	setProcessorType(PROCESSOR_TYPE_RECORD_NODE);

	dataQueue = new DataQueue(WRITE_BLOCK_LENGTH, DATA_BUFFER_NBLOCKS);
	eventQueue = new EventMsgQueue(EVENT_BUFFER_NEVENTS);
	spikeQueue = new SpikeMsgQueue(SPIKE_BUFFER_NSPIKES);

	synchronizer = new Synchronizer(this);

	isSyncReady = true;

	/* New record nodes default to the record engine currently selected in the Control Panel */
	setEngine(CoreServices::getSelectedRecordEngineIdx() - 1);

	dataDirectory = CoreServices::getRecordingDirectory();

	recordThread = new RecordThread(this, recordEngine);

	lastDataChannelArraySize = 0;

	eventMonitor = new EventMonitor();

}

RecordNode::~RecordNode()
{
}

void RecordNode::connectToMessageCenter()
{

	const EventChannel* messageChannel = AccessClass::getMessageCenter()->messageCenter->getMessageChannel();

	if (!isConnectedToMessageCenter)
	{
		eventChannelArray.add(new EventChannel(*messageChannel));
	
		isConnectedToMessageCenter = true;

		LOGD("Record node ", getNodeId(), " connected to Message Center");
	}

}


void RecordNode::disconnectMessageCenter()
{

	const EventChannel* origin = AccessClass::getMessageCenter()->messageCenter->getMessageChannel();

	for (auto eventChannel : eventChannelArray)
	{

		if (eventChannel->getSourceNodeID() > 900)
		{
			eventChannelArray.removeObject(eventChannel);
		}

		isConnectedToMessageCenter = false;

		LOGD("Record node ", getNodeId(), " disconnected from Message Center");
	}

}

void RecordNode::addInputChannel(const GenericProcessor* sourceNode, int chan)
{
	// not getting called

	if (chan != AccessClass::getProcessorGraph()->midiChannelIndex)
	{
		int channelIndex = getNextChannel(false);

		const DataChannel* orig = sourceNode->getDataChannel(chan);
		DataChannel* newChannel = new DataChannel(*orig);
		newChannel->setRecordState(orig->getRecordState());
		dataChannelArray.add(newChannel);
		recordEngine->addDataChannel(channelIndex, dataChannelArray[channelIndex]);

	}
	else
	{
		for (int n = 0; n < sourceNode->getTotalEventChannels(); n++)
		{
			const EventChannel* orig = sourceNode->getEventChannel(n);
			//only add to the record node the events originating from this processor, to avoid duplicates
			if (orig->getSourceNodeID() == sourceNode->getNodeId())
				eventChannelArray.add(new EventChannel(*orig));

		}

	}

}

void RecordNode::addSpecialProcessorChannels(Array<EventChannel*>& channels)
{

	eventChannelArray.addArray(channels);
	settings.numInputs = dataChannelArray.size();
	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 1024);

}

void RecordNode::setEngine(int index)
{
	availableEngines = getAvailableRecordEngines();
	recordEngine = availableEngines[index]->instantiateEngine();
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

	if (filename.length() < 24) // this is a hack
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

// called by RecordNode::startRecording and RHD2000Editor (should be removed)
void RecordNode::createNewDirectory()
{
	rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName() + File::separator + getName() + " " + String(getNodeId()));
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
	editor = new RecordNodeEditor(this, true);
	return editor;
}

// Juce method, not used
void RecordNode::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	/* This gets called right when the play button is pressed*/
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
	editor->updateParameterButtons(parameterIndex);

	if (currentChannel >= 0)
	{
		Parameter* p = parameters[parameterIndex];
		p->setValue(newValue, currentChannel);
	}

}

// Called by ProcessorGraph::enableProcessors()
void RecordNode::updateRecordChannelIndexes()
{
	//Keep the nodeIDs of the original processor from each channel comes from
	updateChannelIndexes(false);
}

// Called when deleting FifoMonitor
void RecordNode::updateChannelStates(int srcIndex, int subProcIdx, std::vector<bool> channelStates)
{
	this->dataChannelStates[srcIndex][subProcIdx] = channelStates;
}

// called by updateSettings (could be refactored)
void RecordNode::updateSubprocessorMap()
{

	bool refreshEditor = false;
    
    std::map<int, std::vector<int>> inputs;

    int updatedNumSubprocessors = 0;
	int originalChannelCount = numChannels;
    int ch = 0;

    while (ch < dataChannelArray.size())
    {
        
        DataChannel* chan = dataChannelArray[ch];
        int sourceID = chan->getSourceNodeID();
        int subProcIdx = chan->getSubProcessorIdx();

        if (inputs.empty() || inputs[sourceID].empty() || inputs[sourceID].back() != subProcIdx)
        {

			//Check if this (src,sub) combo has already been seen and show warning
			if (!inputs.empty() && !inputs[sourceID].empty() && std::find(inputs[sourceID].begin(), inputs[sourceID].end(), subProcIdx) != inputs[sourceID].end())
			{
				AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
					"WARNING!", "Detected input channels re-mapped from different subprocessors. Please correct the channel mapping or else the RecordNode will crash!");
				return;
			}

            //Found a new subprocessor
            inputs[sourceID].push_back(subProcIdx);
            fifoUsage[sourceID][subProcIdx] = 0.0f;
            updatedNumSubprocessors++;
        }
        
        //Add any new sources
        if (!dataChannelStates.count(sourceID) || !dataChannelStates[sourceID].count(subProcIdx))
        {
            int orderInSubprocessor = 0;
            synchronizer->addSubprocessor(chan->getSourceNodeID(), chan->getSubProcessorIdx(), chan->getSampleRate());

            if (synchronizer->masterProcessor < 0)
            {
                synchronizer->setMasterSubprocessor(chan->getSourceNodeID(), chan->getSubProcessorIdx());
            }
            while (ch < dataChannelArray.size() && dataChannelArray[ch]->getSubProcessorIdx() == subProcIdx
            	& dataChannelArray[ch]->getSourceNodeID() == sourceID)
            {
                dataChannelStates[sourceID][dataChannelArray[ch]->getSubProcessorIdx()].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
                dataChannelOrder[ch] = orderInSubprocessor++;
                ch++;
            }
			refreshEditor = true;
        }
        else
		{
			// check if channel count has changed for existing source
			int count = 0;

			for (int i = 0; i < dataChannelArray.size(); i++)
			{
				if (dataChannelArray[i]->getSourceNodeID() == sourceID && dataChannelArray[i]->getSubProcessorIdx() == subProcIdx)
					count++;
			}
			//If channel count is greater, add new channels to dataChannelStates
			if (count > dataChannelStates[sourceID][subProcIdx].size())
			{
				count = count - dataChannelStates[sourceID][subProcIdx].size();
				for (int i=0; i<count; i++)
				{
					dataChannelStates[sourceID][subProcIdx].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				}
			} //else if less, remove n channels from dataChannelStates
			else if (count < dataChannelStates[sourceID][subProcIdx].size())
			{
				count = dataChannelStates[sourceID][subProcIdx].size() - count;
				for (int i=0; i<count; i++)
				{
					dataChannelStates[sourceID][subProcIdx].pop_back();
				}
			}
			else
			{
				//else do nothing
			}
			ch += count;
		}
        
    }

    //Remove any stale processors
    std::vector<int> sources;
    for(auto const& sourceID : inputs)
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
        numSubprocessors = updatedNumSubprocessors;
        static_cast<RecordNodeEditor*> (getEditor())->showSubprocessorFifos(false);
        static_cast<RecordNodeEditor*> (getEditor())->buttonEvent(static_cast<RecordNodeEditor*> (getEditor())->fifoDrawerButton);
    }
    
    numSubprocessors = updatedNumSubprocessors;
    
    eventMap.clear();
    syncChannelMap.clear();
    syncOrderMap.clear();

	LOGDD("Record Node: ", getNodeId(), " has ", eventChannelArray.size(), " event channels");
    for (int ch = 0; ch < eventChannelArray.size(); ch++)
    {

        EventChannel* chan = eventChannelArray[ch];
        int sourceID = chan->getSourceNodeID();
        int subProcID = chan->getSubProcessorIdx();

        eventMap[sourceID][subProcID] = chan->getNumChannels();

        if (dataChannelStates[sourceID][subProcID].size() && !syncChannelMap[sourceID][subProcID])
        {
            syncOrderMap[sourceID][subProcID] = ch;
            syncChannelMap[sourceID][subProcID] = 0;
            synchronizer->setSyncChannel(chan->getSourceNodeID(), chan->getSubProcessorIdx(), ch);
        }

    }
}

// called by GenericProcessor, RecordNodeEditor, SourceProcessorInfo,
// TimestampSourceSelectionComponent
int RecordNode::getNumSubProcessors() const
{
	return numSubprocessors;
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setMasterSubprocessor(int srcIndex, int subProcIdx)
{
	synchronizer->setMasterSubprocessor(srcIndex, subProcIdx);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void RecordNode::setSyncChannel(int srcIndex, int subProcIdx, int channel)
{
	syncChannelMap[srcIndex][subProcIdx] = channel;
	synchronizer->setSyncChannel(srcIndex, subProcIdx, syncOrderMap[srcIndex][subProcIdx]+channel);
}

// called by SyncControlButton
int RecordNode::getSyncChannel(int srcIndex, int subProcIdx)
{
	return syncChannelMap[srcIndex][subProcIdx];
	//return synchronizer->getSyncChannel(srcIndex, subProcIdx);
}

// called by SyncControlButton
bool RecordNode::isMasterSubprocessor(int srcIndex, int subProcIdx)
{
	return (srcIndex == synchronizer->masterProcessor && subProcIdx == synchronizer->masterSubprocessor);
}

// called by GenericProcessor::update()
void RecordNode::updateSettings()
{

	updateSubprocessorMap();

}

// called by GenericProcessor::enableProcessor
bool RecordNode::enable()
{

	connectToMessageCenter();

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

// called by GenericProcessor::setRecording()
void RecordNode::startRecording()
{

	channelMap.clear();
	ftsChannelMap.clear();
	int totChans = dataChannelArray.size();
	OwnedArray<RecordProcessorInfo> procInfo;
	Array<int> chanProcessorMap;
	Array<int> chanOrderinProc;
	int lastProcessor = -1;
	int lastSubProcessor = -1;
	int procIndex = -1;
	int chanSubIdx = 0;

	int recordedProcessorIdx = -1;	

	//LOGD("Record Node ", getNodeId(), ": Total channels: ", totChans, " Total event channels: ", getTotalEventChannels());

	for (int ch = 0; ch < totChans; ++ch)
	{

		DataChannel* chan = dataChannelArray[ch];
		int srcIndex = chan->getSourceNodeID();
		int subIndex = chan->getSubProcessorIdx();

		LOGDD("Channel: ", ch, " Source Node: ", srcIndex, " Sub Index: ", subIndex);

		if (dataChannelStates[srcIndex][subIndex][dataChannelOrder[ch]])
		{

			int chanOrderInProcessor = subIndex * dataChannelStates[srcIndex][subIndex].size() + dataChannelOrder[ch];
			channelMap.add(ch);

			//TODO: This logic will not work after a channel mapper with channels mapped from different subprocessors!
			if (chan->getSourceNodeID() != lastProcessor || chan->getSubProcessorIdx() != lastSubProcessor)
			{
				recordedProcessorIdx++;
				startRecChannels.push_back(ch);
				lastProcessor = chan->getSourceNodeID();
				lastSubProcessor = chan->getSubProcessorIdx();

				RecordProcessorInfo* pi = new RecordProcessorInfo();
				pi->processorId = chan->getSourceNodeID();
				procInfo.add(pi);
			}

			procInfo.getLast()->recordedChannels.add(channelMap.size() - 1);
			chanProcessorMap.add(srcIndex);
			chanOrderinProc.add(chanOrderInProcessor);
			ftsChannelMap.add(recordedProcessorIdx);
		}
	}

	int numRecordedChannels = channelMap.size();
	
	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	recordEngine->registerRecordNode(this);
	//recordEngine->resetChannels();
	recordEngine->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
	recordThread->setChannelMap(channelMap);
	recordThread->setFTSChannelMap(ftsChannelMap);

	dataQueue->setChannels(numRecordedChannels);
	dataQueue->setFTSChannels(recordedProcessorIdx+1);

	eventQueue->reset();
	spikeQueue->reset();
	recordThread->setQueuePointers(dataQueue, eventQueue, spikeQueue);
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
		
		if (settingsNeeded)
		{
			String settingsFileName = rootFolder.getFullPathName() + File::separator + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String::empty) + ".xml";
			AccessClass::getEditorViewport()->saveState(File(settingsFileName), lastSettingsText);
			settingsNeeded = false;
		}

		useSynchronizer = static_cast<RecordNodeEditor*> (getEditor())->getSelectedEngineIdx() == 0;

		recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);

		LOGD("Num event channels: ", eventChannelArray.size());
		recordThread->startThread();
		isRecording = true;
	}
	else
		isRecording = false;

}

// called by GenericProcessor::setRecording()
void RecordNode::stopRecording()
{

	isRecording = false;
	if (recordThread->isThreadRunning())
	{
		recordThread->signalThreadShouldExit();
		recordThread->waitForThreadToExit(2000); //2000
	}

	eventMonitor->displayStatus();

	disconnectMessageCenter();

}

void RecordNode::setRecordEvents(bool recordEvents)
{
	this->recordEvents = recordEvents;
}

void RecordNode::setRecordSpikes(bool recordSpikes)
{
	this->recordSpikes = recordSpikes;
}

void RecordNode::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{

	//Ignore any duplicate messages from MessageCenter
	if (Event::getSourceID(event) > 900)
	{
		if (!msgCenterMessages.contains(Event::getTimestamp(event)))
		{
			msgCenterMessages.add(Event::getTimestamp(event));
			LOGD("Received message.");
		}
		else
			return;
	}

	eventMonitor->receivedEvents++;

	if (recordEvents) 
	{
		// flags whether or not to write events

		int64 timestamp = Event::getTimestamp(event);
		uint64 eventChan = event.getChannel();
		int eventIndex;
		if (eventInfo)
			eventIndex = getEventChannelIndex(Event::getSourceIndex(event), Event::getSourceID(event), Event::getSubProcessorIdx(event));
		else
			eventIndex = -1;

		if (samplePosition > 0 && dataChannelStates[Event::getSourceID(event)][Event::getSubProcessorIdx(event)].size())
			synchronizer->addEvent(Event::getSourceID(event), Event::getSubProcessorIdx(event), eventIndex, timestamp);

		if (isRecording)
			eventQueue->addEvent(event, timestamp, eventIndex);

	}

}

// only called if recordSpikes is true
void RecordNode::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{
    
	SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
	if (!newSpike) 
	{
		LOGD("Unable to deserialize spike event!");
		return;
	}

	if (recordSpikes)
		writeSpike(newSpike, spikeInfo);
	

}

void RecordNode::handleTimestampSyncTexts(const MidiMessage& event)
{

	if (event.getVelocity() == 136)
	{
		if (!receivedSoftwareTime)
		{
			handleEvent(nullptr, event, 0);
			receivedSoftwareTime = true;
		}
	}
	else
	{
		handleEvent(nullptr, event, 0);
	}

}
	
void RecordNode::process(AudioSampleBuffer& buffer)

{

	isProcessing = true;

	checkForEvents(recordSpikes);

	if (isRecording)
	{

		DataChannel* chan; 
		int sourceID;
		int subProcIdx;

		for (int ch = 0; ch < channelMap.size(); ch++)
		{

			if (isFirstChannelInRecordedSubprocessor(channelMap[ch]))
			{

				chan = dataChannelArray[ch];

				numSamples = getNumSamples(channelMap[ch]);
				timestamp = getTimestamp(channelMap[ch]);

				sourceID = chan->getSourceNodeID();
				subProcIdx = chan->getSubProcessorIdx();

			}

			bool shouldWrite = validBlocks[ch];
			if (!shouldWrite && numSamples > 0)
			{
				shouldWrite = true;
				validBlocks.set(ch, true);
			}
			
			if (shouldWrite && numSamples > 0)
			{

				if (isFirstChannelInRecordedSubprocessor(channelMap[ch]))
				{
					
					if (useSynchronizer)
					{
						double first = synchronizer->convertTimestamp(sourceID, subProcIdx, timestamp);
						double second = synchronizer->convertTimestamp(sourceID, subProcIdx, timestamp + 1);
						fifoUsage[sourceID][subProcIdx] = dataQueue->writeSynchronizedTimestampChannel(first, second - first, ftsChannelMap[ch], numSamples);
					}
					else
					{
						fifoUsage[sourceID][subProcIdx] = dataQueue->writeChannel(buffer, channelMap[ch], ch, numSamples, timestamp);
						samplesWritten+=numSamples;
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

// called by ProcessorGraph::connectProcessors
void RecordNode::registerProcessor(const GenericProcessor* sourceNode)
{
	settings.numInputs += sourceNode->getNumOutputs();
	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);
	recordEngine->registerProcessor(sourceNode);
}

// not called
void RecordNode::registerSpikeSource(const GenericProcessor *processor)
{
	recordEngine->registerSpikeSource(processor);
}

// not called
int RecordNode::addSpikeElectrode(const SpikeChannel *elec)
{
	spikeChannelArray.add(new SpikeChannel(*elec));
	recordEngine->addSpikeElectrode(spikeElectrodeIndex, elec);
	return spikeElectrodeIndex++;
}

// called in RecordNode::handleSpike
void RecordNode::writeSpike(const SpikeEvent *spike, const SpikeChannel *spikeElectrode)
{
	//if (isRecording && shouldRecord)
	if (true)
	{
		int electrodeIndex = getSpikeChannelIndex(spikeElectrode->getSourceIndex(), spikeElectrode->getSourceNodeID(), spikeElectrode->getSubProcessorIdx());
		
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
