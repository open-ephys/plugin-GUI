#include "RecordNode.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/ControlPanel.h"
#include "BinaryFormat/BinaryRecording.h"
#include "OpenEphysFormat/OriginalRecording.h"

#include "../../AccessClass.h"

using namespace std::chrono;

#define CONTINUOUS_CHANNELS_ON_BY_DEFAULT true

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
	settingsNeeded(false)
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

}

RecordNode::~RecordNode()
{
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

void RecordNode::updateChannelStates(int srcIndex, int subProcIdx, std::vector<bool> channelStates)
{
	this->dataChannelStates[srcIndex][subProcIdx] = channelStates;
}

void RecordNode::updateSubprocessorMap()
{

	std::vector<int> procIds;

	int masterSubprocessor = -1;
	int eventIndex = 0;

	for (int ch = 0; ch < dataChannelArray.size(); ch++)
	{

		DataChannel* chan = dataChannelArray[ch];
		int sourceID = chan->getSourceNodeID();
		int subProcID = chan->getSubProcessorIdx();

		if (!std::count(procIds.begin(), procIds.end(), sourceID))
			procIds.push_back(sourceID);

		if (!dataChannelStates[sourceID][subProcID].size())
		{
			synchronizer->addSubprocessor(chan->getSourceNodeID(), chan->getSubProcessorIdx(), chan->getSampleRate());
			eventIndex++;

			if (masterSubprocessor < 0)
			{
				masterSubprocessor = chan->getSourceNodeID();
				synchronizer->setMasterSubprocessor(chan->getSourceNodeID(), chan->getSubProcessorIdx());
			}

			int orderInSubprocessor = 0;
			while (ch < dataChannelArray.size() && dataChannelArray[ch]->getSubProcessorIdx() == subProcID && dataChannelArray[ch]->getSourceNodeID() == sourceID)
			{
				dataChannelStates[sourceID][subProcID].push_back(CONTINUOUS_CHANNELS_ON_BY_DEFAULT);
				dataChannelOrder[ch] = orderInSubprocessor++;
				ch++;
			}
			ch--;
		}

	}

	std::map<int, std::map<int, std::vector<bool>>>::iterator it;
	std::map<int, std::vector<bool>>::iterator ptr;

	numSubprocessors = 0;
	for (it = dataChannelStates.begin(); it != dataChannelStates.end(); it++) 
	{

		for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) {
			if (!std::count(procIds.begin(), procIds.end(), it->first))
				dataChannelStates.erase(it->first);
			else
				numSubprocessors++;
		}
	}

	eventChannelMap.clear();
	syncChannelMap.clear();
	syncOrderMap.clear();
	for (int ch = 0; ch < eventChannelArray.size(); ch++)
	{

		EventChannel* chan = eventChannelArray[ch];
		int sourceID = chan->getSourceNodeID();
		int subProcID = chan->getSubProcessorIdx();

		int chCount = 0;

		if (!syncChannelMap[sourceID][subProcID])
		{
			EventChannel* chan = eventChannelArray[ch];
			eventChannelMap[sourceID][subProcID] = chan->getNumChannels();
			syncOrderMap[sourceID][subProcID] = ch;
			syncChannelMap[sourceID][subProcID] = 0;
			//LOGD("Setting {", chan->getSourceNodeID(), ",", chan->getSubProcessorIdx(), "}->", ch);
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
	//eventChannelMap[srcIndex][subProcIdx]; 
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
	recordEngine->resetChannels();
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
			experimentNumber = 0;
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

	if (recordEvents) 
	{
		int64 timestamp = Event::getTimestamp(event);
		uint64 eventChan = event.getChannel();
		int eventIndex;
		if (eventInfo && samplePosition > 0)
		{
			eventIndex = getEventChannelIndex(Event::getSourceIndex(event), Event::getSourceID(event), Event::getSubProcessorIdx(event));
			synchronizer->addEvent(Event::getSourceID(event), Event::getSubProcessorIdx(event), eventIndex, timestamp);
		}
		else
			eventIndex = -1;

		if (isRecording && eventIndex >= 0)
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
	handleEvent(nullptr, event, 0);
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
					float first = synchronizer->convertTimestamp(sourceID, subProcIdx, timestamp);
					float second = synchronizer->convertTimestamp(sourceID, subProcIdx, timestamp + 1);
					dataQueue->writeSynchronizedTimestampChannel(first, second - first, ftsChannelMap[ch], numSamples);
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

//TODO: Need to validate these methods

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