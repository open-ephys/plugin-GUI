/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Open Ephys

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
 
 #include "NWBRecording.h"
#define MAX_BUFFER_SIZE 40960
 
 using namespace NWBRecording;
 
 NWBRecordEngine::NWBRecordEngine() 
 {
	 
	 tsBuffer.malloc(MAX_BUFFER_SIZE);
 }
 
 NWBRecordEngine::~NWBRecordEngine()
 {
 }
 
 String NWBRecordEngine::getEngineID() const
 {
	 return "NWB"; //a text identifier
 }
 
 void NWBRecordEngine::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
 {
	 
	 //Called when acquisition starts, to open the files
	 String basepath = rootFolder.getFullPathName() + rootFolder.separatorString + "experiment_" + String(experimentNumber) + ".nwb";
	 
	 recordFile = new NWBFile(basepath, CoreServices::getGUIVersion(), identifierText);
	 recordFile->setXmlText(getLatestSettingsXml());

	 int recProcs = getNumRecordedProcessors();

	 datasetIndexes.insertMultiple(0, 0, getNumRecordedChannels());
	 writeChannelIndexes.insertMultiple(0, 0, getNumRecordedChannels());

	 //Generate the continuous datasets info array, seeking for different combinations of recorded processor and source processor
	 int lastId = 0;
	 for (int proc = 0; proc < recProcs; proc++)
	 {
		 const RecordProcessorInfo& procInfo = getProcessorInfo(proc);
		 int recChans = procInfo.recordedChannels.size();
		 for (int chan = 0; chan < recChans; chan++)
		 {
			 int recordedChan = procInfo.recordedChannels[chan];
			 int realChan = getRealChannel(recordedChan);
			 const DataChannel* channelInfo = getDataChannel(realChan);
			 int sourceId = channelInfo->getSourceNodeID();
			 int sourceSubIdx = channelInfo->getSubProcessorIdx();
			 int nInfoArrays = continuousChannels.size();
			 bool found = false;
			 for (int i = lastId; i < nInfoArrays; i++)
			 {
				 if (sourceId == continuousChannels.getReference(i)[0]->getSourceNodeID() && sourceSubIdx == continuousChannels.getReference(i)[0]->getSubProcessorIdx())
				 {
					 //A dataset for the current processor from the current source is already present
					 writeChannelIndexes.set(recordedChan, continuousChannels.getReference(i).size());
					 continuousChannels.getReference(i).add(getDataChannel(realChan));
					 datasetIndexes.set(recordedChan, i);
					 found = true;
					 break;
				 }
			 }
			 if (!found) //a new dataset must be created
			 {
				 ContinuousGroup newGroup;
				 newGroup.add(getDataChannel(realChan));
				 continuousChannels.add(newGroup);
				 datasetIndexes.set(recordedChan, nInfoArrays);
				 writeChannelIndexes.set(recordedChan, 0);
			 }

		 }
		 lastId = continuousChannels.size();
	 }
	 int nEvents = getNumRecordedEvents();
	 for (int i = 0; i < nEvents; i++)
		 eventChannels.add(getEventChannel(i));

	 int nSpikes = getNumRecordedSpikes();
	 for (int i = 0; i < nSpikes; i++)
		 spikeChannels.add(getSpikeChannel(i));

	 //open the file
	 recordFile->open(getNumRecordedChannels() + continuousChannels.size() + eventChannels.size() + spikeChannels.size()); //total channels + timestamp arrays, to create a big enough buffer

	 //create the recording
	 recordFile->startNewRecording(recordingNumber, continuousChannels, eventChannels, spikeChannels);
	
 }

 
 void NWBRecordEngine::closeFiles()
 {
	 //Called when acquisition stops. Should close the files and leave the processor in a reset status
	 recordFile->stopRecording();
	 recordFile->close();
	 recordFile = nullptr;
	 resetChannels();
 }


 
 void NWBRecordEngine::resetChannels()
 {
	 spikeChannels.clear();
	 eventChannels.clear();
	 continuousChannels.clear();
	 datasetIndexes.clear();
	 writeChannelIndexes.clear();
	 tsBuffer.malloc(MAX_BUFFER_SIZE);
	 bufferSize = MAX_BUFFER_SIZE;
 }
 
 void NWBRecordEngine::writeData(int writeChannel, int realChannel, const float* buffer, int size)
 {

	 recordFile->writeData(datasetIndexes[writeChannel], writeChannelIndexes[writeChannel], size, buffer, getDataChannel(realChannel)->getBitVolts());

	 /* All channels in a dataset have the same number of samples and share timestamps. But since this method is called 
		asynchronously, the timestamps might not be in sync during acquisition, so we chose a channel and write the
		timestamps when writing that channel's data */
	 if (writeChannelIndexes[writeChannel] == 0)
	 {
		 int64 baseTS = getTimestamp(writeChannel);
		 double fs = getDataChannel(realChannel)->getSampleRate();
		 //Let's hope that the compiler is smart enough to vectorize this. 
		 for (int i = 0; i < size; i++)
		 {
			 tsBuffer[i] = (baseTS + i) / fs;
		 }
		 recordFile->writeTimestamps(datasetIndexes[writeChannel], size, tsBuffer);
	 }
		 
 }
 
void NWBRecordEngine::writeEvent(int eventIndex, const MidiMessage& event) 
{
	const EventChannel* channel = getEventChannel(eventIndex);
	EventPtr eventStruct = Event::deserializeFromMessage(event, channel);

	recordFile->writeEvent(eventIndex, channel, eventStruct);
}

void NWBRecordEngine::writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float sourceSampleRate, String text)
{
	recordFile->writeTimestampSyncText(sourceID, timestamp, sourceSampleRate, text);
}

void NWBRecordEngine::addSpikeElectrode(int index,const  SpikeChannel* elec) 
{
}

void NWBRecordEngine::writeSpike(int electrodeIndex, const SpikeEvent* spike) 
{
	const SpikeChannel* channel = getSpikeChannel(electrodeIndex);

	recordFile->writeSpike(electrodeIndex, channel, spike);
}

RecordEngineManager* NWBRecordEngine::getEngineManager()
{
	//static factory that instantiates the engine manager, which allows to configure recording options among other things. See OriginalRecording to see how to create options for a record engine
	RecordEngineManager* man = new RecordEngineManager("NWB", "NWB", &(engineFactory<NWBRecordEngine>));
	EngineParameter* param;
	param = new EngineParameter(EngineParameter::STR, 0, "Identifier Text", String::empty);
	man->addParameter(param);
	return man;
	
}

void NWBRecordEngine::setParameter(EngineParameter& parameter)
{
	strParameter(0, identifierText);
}