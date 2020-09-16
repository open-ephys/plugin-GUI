#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
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
    numSubprocessors(0)
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

void RecordNode::addInputChannel(const GenericProcessor* sourceNode, int chan)
{

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

	if (filename.length() < 24)
		filename += datestring;
	else
		filename = filename.substring(0, filename.length() - 1);

	filename += AccessClass::getControlPanel()->getTextToAppend();

	return filename;

}

File RecordNode::getDataDirectory()
{
	return dataDirectory;
}

void RecordNode::setDataDirectory(File directory)
{
	dataDirectory = directory;
	newDirectoryNeeded = true;
}

void RecordNode::createNewDirectory()
{
	rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName() + File::separator + getName() + " " + String(getNodeId()));
	newDirectoryNeeded = false;
}

String RecordNode::generateDateString() const
{

	Time calendar = Time::getCurrentTime();

	String datestring;

	datestring += String(calendar.getDayOfMonth());
	datestring += "-";
	datestring += calendar.getMonthName(true);
	datestring += "-";
	datestring += String(calendar.getYear());
	datestring += " ";

	int hrs, mins, secs;
	hrs = calendar.getHours();
	mins = calendar.getMinutes();
	secs = calendar.getSeconds();

	datestring += hrs;

	if (mins < 10)
		datestring += 0;

	datestring += mins;

	if (secs < 0)
		datestring += 0;

	datestring += secs;

	return datestring;

}

int RecordNode::getExperimentNumber() const
{
	LOGD(__FUNCTION__, " = ", experimentNumber);
	return experimentNumber;
}

int RecordNode::getRecordingNumber() const
{
	LOGD(__FUNCTION__, " = ", recordingNumber);
	return recordingNumber;
}

AudioProcessorEditor* RecordNode::createEditor()
{
	editor = new RecordNodeEditor(this, true);
	return editor;
}

void RecordNode::prepareToPlay(double sampleRate, int estimatedSamplesPerBlock)
{
	/* This gets called right when the play button is pressed*/
}

const String &RecordNode::getLastSettingsXml() const
{
	return lastSettingsText;
}

/* Use this function to change paramaters while recording...*/
void RecordNode::setParameter(int parameterIndex, float newValue)
{
	editor->updateParameterButtons(parameterIndex);

	if (currentChannel >= 0)
	{
		Parameter* p = parameters[parameterIndex];
		p->setValue(newValue, currentChannel);
	}

}

void RecordNode::updateRecordChannelIndexes()
{
	//Keep the nodeIDs of the original processor from each channel comes from
	updateChannelIndexes(false);
}

void RecordNode::updateChannelStates(int srcIndex, int subProcIdx, std::vector<bool> channelStates)
{
	this->dataChannelStates[srcIndex][subProcIdx] = channelStates;
}

void RecordNode::updateSubprocessorMap()
{
    
    std::map<int, std::vector<int>> inputs;

    int updatedNumSubprocessors = 0;
    int ch = 0;
    
    while (ch < dataChannelArray.size())
    {
        
        DataChannel* chan = dataChannelArray[ch];
        int sourceID = chan->getSourceNodeID();
        int subProcIdx = chan->getSubProcessorIdx();
        
        if (inputs.empty() || inputs[sourceID].empty() || inputs[sourceID].back() != subProcIdx)
        {
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
            while (ch < dataChannelArray.size() && dataChannelArray[ch]->getSubProcessorIdx() == subProcIdx)
            {
                dataChannelStates[sourceID][dataChannelArray[ch]->getSubProcessorIdx()].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
                dataChannelOrder[ch] = orderInSubprocessor++;
                ch++;
            }
        }
        else
        {
            ch++;
            //Don't do anything
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
    
    if (numSubprocessors != updatedNumSubprocessors && static_cast<RecordNodeEditor*> (getEditor())->subprocessorsVisible)
    {
        numSubprocessors = updatedNumSubprocessors;
        static_cast<RecordNodeEditor*> (getEditor())->showSubprocessorFifos(false);
        static_cast<RecordNodeEditor*> (getEditor())->buttonEvent(static_cast<RecordNodeEditor*> (getEditor())->fifoDrawerButton);
    }
    
    numSubprocessors = updatedNumSubprocessors;
    
    eventMap.clear();
    syncChannelMap.clear();
    syncOrderMap.clear();
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

int RecordNode::getNumSubProcessors() const
{
	return numSubprocessors;
}

void RecordNode::setMasterSubprocessor(int srcIndex, int subProcIdx)
{
	synchronizer->setMasterSubprocessor(srcIndex, subProcIdx);
}

void RecordNode::setSyncChannel(int srcIndex, int subProcIdx, int channel)
{
	syncChannelMap[srcIndex][subProcIdx] = channel;
	synchronizer->setSyncChannel(srcIndex, subProcIdx, syncOrderMap[srcIndex][subProcIdx]+channel);
}

int RecordNode::getSyncChannel(int srcIndex, int subProcIdx)
{
	return syncChannelMap[srcIndex][subProcIdx];
	//return synchronizer->getSyncChannel(srcIndex, subProcIdx);
}

bool RecordNode::isMasterSubprocessor(int srcIndex, int subProcIdx)
{
	return (srcIndex == synchronizer->masterProcessor && subProcIdx == synchronizer->masterSubprocessor);
}

void RecordNode::updateSettings()
{

	updateSubprocessorMap();

}

bool RecordNode::enable()
{

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

		if (dataChannelStates[srcIndex][subIndex][dataChannelOrder[ch]])
		{

			int chanOrderInProcessor = subIndex * dataChannelStates[srcIndex][subIndex].size() + dataChannelOrder[ch];
			channelMap.add(ch);

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
		
		//TODO:
		/*
		if (settingsNeeded)
		{
			String settingsFileName = rootFolder.getFullPathName() + File::separator + "settings" + ((experimentNumber > 1) ? "_" + String(experimentNumber) : String::empty) + ".xml";
			AccessClass::getEditorViewport()->saveState(File(settingsFileName), m_lastSettingsText);
			settingsNeeded = false;
		}
		*/

		useSynchronizer = static_cast<RecordNodeEditor*> (getEditor())->getSelectedEngineIdx() == 0;

		recordThread->setFileComponents(rootFolder, experimentNumber, recordingNumber);
		recordThread->startThread();
		isRecording = true;
	}
	else
		isRecording = false;

}

void RecordNode::stopRecording()
{

	isRecording = false;
	if (recordThread->isThreadRunning())
	{
		recordThread->signalThreadShouldExit();
		recordThread->waitForThreadToExit(200); //2000
	}

	eventMonitor->displayStatus();

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

	eventMonitor->receivedEvents++;

	if (recordEvents) 
	{
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


void RecordNode::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{


	SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
	if (!newSpike) 
	{
		std::cout << "Unable to deserialize spike event!" << std::endl; 
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

bool RecordNode::isFirstChannelInRecordedSubprocessor(int ch)
{
	return std::find(startRecChannels.begin(), startRecChannels.end(), ch) != startRecChannels.end();
}

void RecordNode::registerProcessor(const GenericProcessor* sourceNode)
{
	settings.numInputs += sourceNode->getNumOutputs();
	setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);
	recordEngine->registerProcessor(sourceNode);
}

void RecordNode::registerSpikeSource(const GenericProcessor *processor)
{
	recordEngine->registerSpikeSource(processor);
}

int RecordNode::addSpikeElectrode(const SpikeChannel *elec)
{
	spikeChannelArray.add(new SpikeChannel(*elec));
	recordEngine->addSpikeElectrode(spikeElectrodeIndex, elec);
	return spikeElectrodeIndex++;
}

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

void RecordNode::filenameComponentChanged(FilenameComponent *fnc)
{

	dataDirectory = fnc->getCurrentFile();
	newDirectoryNeeded = true;
}

float RecordNode::getFreeSpace() const
{
	return 1.0f - float(dataDirectory.getBytesFreeOnVolume()) / float(dataDirectory.getVolumeTotalSize());
}

void RecordNode::registerRecordEngine(RecordEngine *engine)
{
	engineArray.add(engine);
}

void RecordNode::clearRecordEngines()
{
	engineArray.clear();
}
