#include "RecordNode.h"
#include "BinaryFormat/BinaryRecording.h"

using namespace std::chrono;

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
	setProcessorType(PROCESSOR_TYPE_FILTER);

	dataQueue = new DataQueue(WRITE_BLOCK_LENGTH, DATA_BUFFER_NBLOCKS);
	eventQueue = new EventMsgQueue(EVENT_BUFFER_NEVENTS);
	spikeQueue = new SpikeMsgQueue(SPIKE_BUFFER_NSPIKES);
	
	recordEngine = new BinaryRecording();

	recordThread = new RecordThread(this, recordEngine);

}

RecordNode::~RecordNode()
{
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
	//String filename = AccessClass::getControlPanel()->getTextToPrepend();
	String filename = "";

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

	//AccessClass::getControlPanel()->setDateText(datestring);

	if (filename.length() < 24)
		filename += datestring;
	else
		filename = filename.substring(0, filename.length() - 1);

	//filename += AccessClass::getControlPanel()->getTextToAppend();

	return filename;

}

void RecordNode::createNewDirectory()
{

	dataDirectory = CoreServices::getRecordingDirectory();

	LOGD(__FUNCTION__, dataDirectory.getFullPathName());

	rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName());

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
	this->m[srcIndex][subProcIdx] = channelStates;
}

void RecordNode::updateSubprocessorMap()
{

	std::vector<int> procIds;

	for (int ch = 0; ch < dataChannelArray.size(); ch++)
	{

		DataChannel* chan = dataChannelArray[ch];
		int sourceID = chan->getSourceNodeID();
		int subProcID = chan->getSubProcessorIdx();

		if (!std::count(procIds.begin(), procIds.end(), sourceID))
			procIds.push_back(sourceID);

		if (!m[sourceID][subProcID].size())
		{
			int orderInSubprocessor = 0;
			while (ch < dataChannelArray.size() && dataChannelArray[ch]->getSubProcessorIdx() == subProcID && dataChannelArray[ch]->getSourceNodeID() == sourceID)
			{
				m[sourceID][subProcID].push_back(false);
				n[ch] = orderInSubprocessor++;
				ch++;
			}
			ch--;
		}

	}

	std::map<int, std::map<int, std::vector<bool>>>::iterator it;
	std::map<int, std::vector<bool>>::iterator ptr;

	numSubprocessors = 0;
	for (it = m.begin(); it != m.end(); it++) {

		for (ptr = it->second.begin(); ptr != it->second.end(); ptr++) {
			LOGD("Source ", it->first, " subprocessor ", ptr->first, " has ", ptr->second.size(), "channels.");
			if (!std::count(procIds.begin(), procIds.end(), it->first))
				m.erase(it->first);
			else
				numSubprocessors++;
		}
	}

	LOGD("Generated channel map:");
	std::map<int, int>::iterator itr;
	for (itr = n.begin(); itr != n.end(); itr++) {

		LOGD("(",itr->first, ",", itr->second,")");
	}

}

int RecordNode::getNumSubProcessors() const
{
	return numSubprocessors;
}

void RecordNode::updateSettings()
{
	updateSubprocessorMap();
}

void RecordNode::startRecording()
{

	updateSettings(); 

	channelMap.clear();
	int totChans = dataChannelArray.size();
	OwnedArray<RecordProcessorInfo> procInfo;
	Array<int> chanProcessorMap;
	Array<int> chanOrderinProc;
	int lastProcessor = -1;
	int lastSubProcessor = -1;
	int procIndex = -1;
	int chanProcOrder = 0;

	int chanSubIdx = 0;

	for (int ch = 0; ch < totChans; ++ch)
	{
		DataChannel* chan = dataChannelArray[ch];
		int srcIndex = chan->getSourceNodeID();
		int subIndex = chan->getSubProcessorIdx();

		if (m[srcIndex][subIndex][n[ch]])
		{
			channelMap.add(ch);
			if (chan->getSubProcessorIdx() != lastSubProcessor || chan->getSourceNodeID() != lastProcessor)
			{
				lastSubProcessor = chan->getSubProcessorIdx();
				startRecChannels.push_back(ch);
			}

			if (chan->getCurrentNodeID() != lastProcessor)
			{
				lastProcessor = chan->getCurrentNodeID();
				RecordProcessorInfo* pi = new RecordProcessorInfo();
				pi->processorId = chan->getCurrentNodeID();
				procInfo.add(pi);
				procIndex++;
				chanProcOrder = 0;
			}
			procInfo.getLast()->recordedChannels.add(channelMap.size() - 1);
			chanProcessorMap.add(procIndex);
			chanOrderinProc.add(chanProcOrder);
			chanProcOrder++;
		}
	}

	for (int i = 0; i < channelMap.size(); i++)
	{
		LOGD("channelMap: ", channelMap[i], " chanProcMap: ", chanProcessorMap[i], " chanOrderInProc: ", chanOrderinProc[i]);
	}

	int numRecordedChannels = channelMap.size();
	
	validBlocks.clear();
	validBlocks.insertMultiple(0, false, getNumInputs());

	//WARNING: If at some point we record at more that one recordEngine at once, we should change this, as using OwnedArrays only works for the first

	recordEngine->registerRecordNode(this);
	recordEngine->resetChannels();
	recordEngine->setChannelMapping(channelMap, chanProcessorMap, chanOrderinProc, procInfo);
	recordThread->setChannelMap(channelMap);

	dataQueue->setChannels(numRecordedChannels);
	eventQueue->reset();
	spikeQueue->reset();
	recordThread->setQueuePointers(dataQueue, eventQueue, spikeQueue);
	recordThread->setFirstBlockFlag(false);

	hasRecorded = true;

	/* Set write properties */
	setFirstBlock = false;

	//Only start recording thread if at least one continous OR event channel is enabled
	if (channelMap.size() || recordEvents)
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

void RecordNode::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{

	if (recordEvents) 
	{
		int64 timestamp = Event::getTimestamp(event);
		int eventIndex;
		if (eventInfo)
			eventIndex = getEventChannelIndex(Event::getSourceIndex(event), Event::getSourceID(event), Event::getSubProcessorIdx(event));
		else
			eventIndex = -1;
		if (isRecording)
			eventQueue->addEvent(event, timestamp, eventIndex);
	}

}

void RecordNode::handleTimestampSyncTexts(const MidiMessage& event)
{
	handleEvent(nullptr, event, 0);
}
	
void RecordNode::process(AudioSampleBuffer& buffer)
{

	if (isRecording)
	{

		checkForEvents();

		for (int ch = 0; ch < channelMap.size(); ch++)
		{

			if (isFirstChannelInRecordedSubprocessor(channelMap[ch]))
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
			
			if (shouldWrite)
			{
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
	if (isRecording && shouldRecord)
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