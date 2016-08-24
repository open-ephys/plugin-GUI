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
 
 NWBRecordEngine::NWBRecordEngine() : bufferSize(MAX_BUFFER_SIZE)
 {
	 scaledBuffer.malloc(MAX_BUFFER_SIZE);
	 intBuffer.malloc(MAX_BUFFER_SIZE);
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
		 const RecordProcessorInfo procInfo = getProcessorInfo(proc);
		 int recChans = procInfo.recordedChannels.size();
		 for (int chan = 0; chan < recChans; chan++)
		 {
			 int recordedChan = procInfo.recordedChannels[chan];
			 int realChan = getRealChannel(recordedChan);
			 Channel* channelInfo = getChannel(realChan);
			 int sourceId = channelInfo->sourceNodeId;
			 int nInfoArrays = continuousInfo.size();
			 bool found = false;
			 for (int i = lastId; i < nInfoArrays; i++)
			 {
				 if (sourceId == continuousInfo[i].sourceId)
				 {
					 //A dataset for the current processor from the current source is already present
					 writeChannelIndexes.set(recordedChan, continuousInfo[i].nChannels);
					 continuousInfo.getReference(i).nChannels += 1;
					 datasetIndexes.set(recordedChan, i);
					 found = true;
					 break;
				 }
			 }
			 if (!found) //a new dataset must be created
			 {
				 NWBRecordingInfo recInfo;
				 recInfo.bitVolts = channelInfo->bitVolts;
				 recInfo.nChannels = 1;
				 recInfo.processorId = procInfo.processorId;
				 recInfo.sampleRate = channelInfo->sampleRate;
				 recInfo.sourceName = "processor_(" + String(sourceId) + ")"; //TODO: add a way to get the actual processor name
				 recInfo.sourceId = sourceId;
				 recInfo.spikeElectrodeName = " ";
				 recInfo.nSamplesPerSpike = 0;
				 continuousInfo.add(recInfo);
				 datasetIndexes.set(recordedChan, nInfoArrays);
				 writeChannelIndexes.set(recordedChan, 0);
			 }

		 }
		 lastId = continuousInfo.size();
	 }

	 //open the file
	 recordFile->open(getNumRecordedChannels() + continuousInfo.size()); //total channels + timestamp arrays, to create a big enough buffer

	 //create the recording
	 recordFile->startNewRecording(recordingNumber, continuousInfo, spikeInfo);
	 
 }

 
 void NWBRecordEngine::closeFiles()
 {
	 //Called when acquisition stops. Should close the files and leave the processor in a reset status
	 recordFile->stopRecording();
	 recordFile->close();
	 recordFile = nullptr;
	 resetChannels(false);
 }

 void NWBRecordEngine::resetChannels()
 {
	 resetChannels(true);
 }
 
 void NWBRecordEngine::resetChannels(bool resetSpikes)
 {
	 //Called at various points, should reset everything.

	 if (resetSpikes) //only clear this at actual reset, not when closing files.
		spikeInfo.clear();
	 continuousInfo.clear();
	 datasetIndexes.clear();
	 writeChannelIndexes.clear();
	 scaledBuffer.malloc(MAX_BUFFER_SIZE);
	 intBuffer.malloc(MAX_BUFFER_SIZE);
	 tsBuffer.malloc(MAX_BUFFER_SIZE);
	 bufferSize = MAX_BUFFER_SIZE;
 }
 
 void NWBRecordEngine::writeData(int writeChannel, int realChannel, const float* buffer, int size)
 {
	 if (size > bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	 {
		 std::cerr << "Write buffer overrun, resizing to" << size << std::endl;
		 bufferSize = size;
		 scaledBuffer.malloc(size);
		 intBuffer.malloc(size);
		 tsBuffer.malloc(size);
	 }

	 double multFactor = 1 / (float(0x7fff) * getChannel(realChannel)->bitVolts);
	 FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), buffer, multFactor, size);
	 AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), intBuffer.getData(), size);

	 recordFile->writeData(datasetIndexes[writeChannel], writeChannelIndexes[writeChannel], size, intBuffer.getData());

	 /* All channels in a dataset have the same number of samples and share timestamps. But since this method is called 
		asynchronously, the timestamps might not be in sync during acquisition, so we chose a channel and write the
		timestamps when writing that channel's data */
	 if (writeChannelIndexes[writeChannel] == 0)
	 {
		 int64 baseTS = getTimestamp(writeChannel);
		 double fs = getChannel(realChannel)->sampleRate;
		 //Let's hope that the compiler is smart enough to vectorize this. 
		 for (int i = 0; i < size; i++)
		 {
			 tsBuffer[i] = (baseTS + i) / fs;
		 }
		 recordFile->writeTimestamps(datasetIndexes[writeChannel], size, tsBuffer);
	 }
		 
 }
 
void NWBRecordEngine::writeEvent(int eventType, const MidiMessage& event, int64 timestamp) 
{
	const uint8* dataptr = event.getRawData();
	if (eventType == GenericProcessor::TTL)
	{
		recordFile->writeTTLEvent(*(dataptr + 3), *(dataptr + 2), *(dataptr + 1), timestamp);
	}
	else if (eventType == GenericProcessor::MESSAGE)
	{
		recordFile->writeMessage((const char*)(dataptr + 6), timestamp);
	}
}

void NWBRecordEngine::registerSpikeSource(GenericProcessor* proc)
{
	currentSpikeProc = proc->nodeId;
	currentSpikeProcName = proc->getName();
}

void NWBRecordEngine::addSpikeElectrode(int index,const  SpikeRecordInfo* elec) 
{
	//Called during chain update by a processor that records spikes. Allows the RecordEngine to gather data about the electrode, which will usually
	//be used in openfiles to be sent to startNewRecording so the electrode info is stored into the file.
	NWBRecordingInfo info;
	info.bitVolts = elec->bitVolts;
	info.nChannels = elec->numChannels;
	info.nSamplesPerSpike = MAX_NUMBER_OF_SPIKE_CHANNEL_SAMPLES; //TODO: forward this information properly through the chain at configuration.
	info.processorId = currentSpikeProc;
	info.sourceId = currentSpikeProc;
	info.sampleRate = elec->sampleRate;
	info.sourceName = currentSpikeProcName + "_(" + String(currentSpikeProc) + ")";
	info.spikeElectrodeName = elec->name;
	spikeInfo.add(info);
}
void NWBRecordEngine::writeSpike(int electrodeIndex, const SpikeObject& spike, int64 timestamp) 
{
	recordFile->writeSpike(electrodeIndex, spike.data, timestamp);
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