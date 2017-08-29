/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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

#include "BinaryRecording.h"

#define MAX_BUFFER_SIZE 40960

using namespace BinaryRecordingEngine;

BinaryRecording::BinaryRecording()
{
	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
}

BinaryRecording::~BinaryRecording()
{

}

String BinaryRecording::getEngineID() const
{
	return "RAWBINARY";
}

void BinaryRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
	String basepath = rootFolder.getFullPathName() + rootFolder.separatorString + "experiment" + String(experimentNumber)
		+ File::separatorString + "recording" + String(recordingNumber) + File::separatorString;
	String contPath = basepath + "continuous" + File::separatorString;
	//Open channel files
	int nProcessors = getNumRecordedProcessors();

	m_channelIndexes.insertMultiple(0, 0, getNumRecordedChannels());
	m_fileIndexes.insertMultiple(0, 0, getNumRecordedChannels());

	Array<const DataChannel*> indexedDataChannels;
	Array<unsigned int> indexedChannelCount;

	int lastId = 0;
	for (int proc = 0; proc < nProcessors; proc++)
	{
		const RecordProcessorInfo& pInfo = getProcessorInfo(proc);
		int recChans = pInfo.recordedChannels.size();

		for (int chan = 0; chan < recChans; chan++)
		{
			int recordedChan = pInfo.recordedChannels[chan];
			int realChan = getRealChannel(recordedChan);
			const DataChannel* channelInfo = getDataChannel(realChan);
			int sourceId = channelInfo->getSourceNodeID();
			int sourceSubIdx = channelInfo->getSubProcessorIdx();
			int nInfoArrays = indexedDataChannels.size();
			bool found = false;
			for (int i = lastId; i < nInfoArrays; i++)
			{
				if (sourceId == indexedDataChannels[i]->getSourceNodeID() && sourceSubIdx == indexedDataChannels[i]->getSubProcessorIdx())
				{
					unsigned int count = indexedChannelCount[i];
					m_channelIndexes.set(recordedChan, count);
					m_fileIndexes.set(recordedChan, i);
					indexedChannelCount.set(i, count + 1);
					found = true;
					break;
				}
			}
			if (!found)
			{
				String datPath(contPath + channelInfo->getCurrentNodeName() + "_(" + String(channelInfo->getCurrentNodeID()) + ")" + File::separatorString);
				String datFileName(datPath + channelInfo->getSourceName() + "_(" + String(sourceId) + "." + String(sourceSubIdx) + ")");
				ScopedPointer<SequentialBlockFile> bFile = new SequentialBlockFile(pInfo.recordedChannels.size(), samplesPerBlock);
				if (bFile->openFile(datFileName + ".dat"))
					m_DataFiles.add(bFile.release());
				else
					m_DataFiles.add(nullptr);
				Array<NpyType> tstypes;
				tstypes.add(NpyType("Timestamp", BaseType::INT64, 1));
				
				ScopedPointer<NpyFile> tFile = new NpyFile(datFileName + "_timestamps.npy", tstypes);
				m_dataTimestampFiles.add(tFile.release());

				m_fileIndexes.set(recordedChan, nInfoArrays);
				m_channelIndexes.set(recordedChan, 0);
				indexedChannelCount.add(1);
				
			}
		}
	}
	int nChans = getNumRecordedChannels();
	//Timestamps
	Array<uint32> procIDs;
	for (int i = 0; i < nChans; i++)
	{
		m_startTS.add(getTimestamp(i));
	}

	int nEvents = getNumRecordedEvents();
	String eventPath(basepath + "events" + File::separatorString);
	int binCount = 0, ttlCount = 0, textCount = 0;
	for (int ev = 0; ev < nEvents; ev++)
	{
		const EventChannel* chan = getEventChannel(ev);
		String fName = eventPath;
		Array<NpyType> types;
		switch (chan->getChannelType())
		{
		case EventChannel::TEXT:
			textCount++;
			fName += "TEXT" + String(textCount);
			types.add(NpyType("message", BaseType::CHAR, chan->getLength()));
			break;
		case EventChannel::TTL:
			ttlCount++;
			fName += "TTL" + String(ttlCount);
			types.add(NpyType("TTL_Channel", BaseType::INT16, 1));
			break;
		default:
			binCount++;
			fName += "BIN" + String(ttlCount);
			types.add(NpyType("Data", chan->getEquivalentMetaDataType(), chan->getLength()));
			break;
		}
		fName += "_" + chan->getSourceName() + "(" + String(chan->getSourceNodeID()) + "." + String(chan->getSubProcessorIdx()) + ")";
		ScopedPointer<EventRecording> rec = new EventRecording();
		Array<NpyType> tsType;
		tsType.add(NpyType("Timestamp", BaseType::INT64, 1));
		//TTL channels behave a bit different
		if (chan->getChannelType() == EventChannel::TTL)
		{
			if (m_TTLMode == TTLMode::JointWord)
			{
				types.add(NpyType("TTL_Word", BaseType::UINT8, chan->getDataSize()));
			}
			else if (m_TTLMode == TTLMode::SeparateWord)
			{
				Array<NpyType> wordType;
				wordType.add(NpyType("TTL_Word", BaseType::UINT8, chan->getDataSize()));
				rec->extraFile = new NpyFile(fName + "_TTLWord.npy", wordType);
			}
			//since the main TTL file already contins channel numbers, it would be redundant to store them on the timestamp file
		}
		else
		{
			if (m_eventMode == EventMode::SeparateChannel)
			{
				Array<NpyType> chanType;
				chanType.add(NpyType("Channel", BaseType::UINT16, 1));
				rec->channelFile = new NpyFile(fName + "_channel.npy", chanType);
			}
			else
				tsType.add(NpyType("Channel", BaseType::UINT16, 1));
		}
		rec->mainFile = new NpyFile(fName + ".npy", types);
		rec->timestampFile = new NpyFile(fName + "_timestamps.npy", tsType);
		rec->metaDataFile = createEventMetadataFile(chan, fName + "_metadata.npy");
		m_eventFiles.add(rec.release());
	}

	int nSpikes = getNumRecordedSpikes();
	Array<const SpikeChannel*> indexedSpikes;
	Array<uint16> indexedChannels;
	m_spikeFileIndexes.insertMultiple(0, 0, nSpikes);
	m_spikeChannelIndexes.insertMultiple(0, 0, nSpikes);
	String spikePath(basepath + "spikes" + File::separatorString);
	for (int sp = 0; sp < nSpikes; sp++)
	{
		const SpikeChannel* ch = getSpikeChannel(sp);
		int nIndexed = indexedSpikes.size();
		bool found = false;
		for (int i = 0; i < nIndexed; i++)
		{
			const SpikeChannel* ich = indexedSpikes[i];
			//identical channels (same data and metadata) from the same processor go to the same file
			if (ch->getCurrentNodeID() == ich->getCurrentNodeID() && ch == ich)
			{
				found = true;
				m_spikeFileIndexes.set(sp, i);
				unsigned int numChans = indexedChannels[i];
				indexedChannels.set(i, numChans);
				m_spikeChannelIndexes.set(sp, numChans + 1);
				break;
			}
		}
		
		if (!found)
		{
			int fileIndex = m_spikeFiles.size();
			m_spikeFileIndexes.set(sp, fileIndex);
			indexedSpikes.add(ch);
			m_spikeChannelIndexes.set(sp, 0);
			indexedChannels.add(1);
			ScopedPointer<EventRecording> rec = new EventRecording();
			Array<NpyType> spTypes;
			for (int c = 0; c < ch->getNumChannels(); c++)
			{
				spTypes.add(NpyType("channel" + String(c + 1), BaseType::INT16, ch->getTotalSamples()));
			}
			String fName(spikePath + "spike_group_" + String(fileIndex + 1));
			rec->mainFile = new NpyFile(fName + ".npy", spTypes);
			Array<NpyType> tsTypes;
			tsTypes.add(NpyType("timestamp", BaseType::INT64, 1));
			if (m_spikeMode == SpikeMode::AllInOne)
			{
				tsTypes.add(NpyType("electrode_index", BaseType::UINT16, 1));
				tsTypes.add(NpyType("sorted_id", BaseType::UINT16, 1));
			}
			else
			{
				Array<NpyType> indexType;
				indexType.add(NpyType("electrode_index", BaseType::UINT16, 1));
				if (m_spikeMode == SpikeMode::AllSeparated)
				{
					Array<NpyType> sortedType;
					sortedType.add(NpyType("sorted_id", BaseType::UINT16, 1));
					rec->extraFile = new NpyFile(fName + "_sortedID.npy", sortedType);
				}
				else
				{
					indexType.add(NpyType("sorted_id", BaseType::UINT16, 1));
				}
				rec->channelFile = new NpyFile(fName + "indexes.npy", indexType);
			}
			rec->timestampFile = new NpyFile(fName + "_timestamps.npy", tsTypes);
			rec->metaDataFile = createEventMetadataFile(ch, fName + "_metadata.npy");
			m_spikeFiles.add(rec.release());
		}
	}

	Array<NpyType> msgType;
	msgType.add(NpyType("sync_text", BaseType::CHAR, 256));
	m_syncTextFile = new NpyFile(basepath + "sync_text.npy", msgType);
	m_recordingNum = recordingNumber;
}

NpyFile* BinaryRecording::createEventMetadataFile(const MetaDataEventObject* channel, String filename)
{
	int nMetaData = channel->getEventMetaDataCount();
	if (nMetaData < 1) return nullptr;

	Array<NpyType> types;
	for (int i = 0; i < nMetaData; i++)
	{
		const MetaDataDescriptor* md = channel->getEventMetaDataDescriptor(i);
		types.add(NpyType(md->getName(), md->getType(), md->getLength()));
	}
	return new NpyFile(filename, types);
}

void BinaryRecording::closeFiles()
{
	resetChannels();
}

void BinaryRecording::resetChannels()
{
	m_DataFiles.clear();
	m_channelIndexes.clear();
	m_fileIndexes.clear();
	m_eventFiles.clear();
	m_spikeChannelIndexes.clear();
	m_spikeFileIndexes.clear();
	m_spikeFiles.clear();
	m_syncTextFile = nullptr;

	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
	m_bufferSize = MAX_BUFFER_SIZE;
	m_startTS.clear();
}

void BinaryRecording::writeData(int writeChannel, int realChannel, const float* buffer, int size)
{
	if (size > m_bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	{
		std::cerr << "Write buffer overrun, resizing to" << size << std::endl;
		m_bufferSize = size;
		m_scaledBuffer.malloc(size);
		m_intBuffer.malloc(size);
	}
	double multFactor = 1 / (float(0x7fff) * getDataChannel(realChannel)->getBitVolts());
	FloatVectorOperations::copyWithMultiply(m_scaledBuffer.getData(), buffer, multFactor, size);
	AudioDataConverters::convertFloatToInt16LE(m_scaledBuffer.getData(), m_intBuffer.getData(), size);

	m_DataFiles[m_fileIndexes[writeChannel]]->writeChannel(getTimestamp(writeChannel)-m_startTS[writeChannel],m_channelIndexes[writeChannel],m_intBuffer.getData(),size);
}


void BinaryRecording::addSpikeElectrode(int index, const SpikeChannel* elec)
{
}

void BinaryRecording::writeEventMetaData(const MetaDataEvent* event, NpyFile* file)
{
	if (!file || !event) return;
	int nMetaData = event->getMetadataValueCount();
	for (int i = 0; i < nMetaData; i++)
	{
		const MetaDataValue* val = event->getMetaDataValue(i);
		file->writeData(val->getRawValuePointer(), val->getDataSize());
	}
}

void BinaryRecording::writeEvent(int eventIndex, const MidiMessage& event)
{
	EventPtr ev = Event::deserializeFromMessage(event, getEventChannel(eventIndex));
	EventRecording* rec = m_eventFiles[eventIndex];
	if (!rec) return;
	const EventChannel* info = getEventChannel(eventIndex);
	int64 ts = ev->getTimestamp();
	rec->timestampFile->writeData(&ts, sizeof(int64));
	if (ev->getEventType() == EventChannel::TTL)
	{
		TTLEvent* ttl = static_cast<TTLEvent*>(ev.get());
		int16 data = ttl->getChannel() * (ttl->getState() ? 1 : -1);
		rec->mainFile->writeData(&data, sizeof(int16));
		NpyFile* wordFile = nullptr;
		if (m_TTLMode == TTLMode::JointWord)
		{
			wordFile = rec->mainFile;
		}
		else if (m_TTLMode == TTLMode::SeparateWord)
		{
			wordFile = rec->extraFile;
		}
		if (wordFile)
			wordFile->writeData(ttl->getTTLWordPointer(), info->getDataSize());
	}
	else
	{
		rec->mainFile->writeData(ev->getRawDataPointer(), info->getDataSize());
		NpyFile* chanFile = nullptr;
		if (m_eventMode == EventMode::SeparateChannel)
		{
			chanFile = rec->channelFile;
		}
		else
		{
			chanFile = rec->timestampFile;
		}
		uint16 chan = ev->getChannel();
		chanFile->writeData(&chan, sizeof(uint16));
	}
	writeEventMetaData(ev.get(), rec->metaDataFile);
}

void BinaryRecording::writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float, String text)
{
	text.paddedRight(' ', 256);
	m_syncTextFile->writeData(text.toUTF8(), 256);
}



void BinaryRecording::writeSpike(int electrodeIndex, const SpikeEvent* spike)
{
	const SpikeChannel* channel = getSpikeChannel(electrodeIndex);
	EventRecording* rec = m_spikeFiles[m_spikeFileIndexes[electrodeIndex]];
	uint16 spikeChannel = m_spikeChannelIndexes[electrodeIndex];

	int totalSamples = channel->getTotalSamples() * channel->getNumChannels();
	

	if (totalSamples > m_bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	{
		std::cerr << "(spike) Write buffer overrun, resizing to" << totalSamples << std::endl;
		m_bufferSize = totalSamples;
		m_scaledBuffer.malloc(totalSamples);
		m_intBuffer.malloc(totalSamples);
	}
	double multFactor = 1 / (float(0x7fff) * channel->getChannelBitVolts(0));
	FloatVectorOperations::copyWithMultiply(m_scaledBuffer.getData(), spike->getDataPointer(), multFactor, totalSamples);
	AudioDataConverters::convertFloatToInt16LE(m_scaledBuffer.getData(), m_intBuffer.getData(), totalSamples);
	rec->mainFile->writeData(m_intBuffer.getData(), totalSamples*sizeof(int16));
	int64 ts = spike->getTimestamp();
	rec->timestampFile->writeData(&ts, sizeof(int64));
	NpyFile* indexFile;
	NpyFile* sortedFile;
	if (m_spikeMode == SpikeMode::AllInOne)
	{
		indexFile = rec->timestampFile;
		sortedFile = rec->timestampFile;
	}
	else if (m_spikeMode == SpikeMode::SeparateTimestamps)
	{
		indexFile = rec->channelFile;
		sortedFile = rec->channelFile;
	}
	else
	{
		indexFile = rec->channelFile;
		sortedFile = rec->extraFile;
	}
	indexFile->writeData(&spikeChannel, sizeof(uint16));

	uint16 sortedID = spike->getSortedID();
	sortedFile->writeData(&sortedID, sizeof(uint16));

	writeEventMetaData(spike, rec->metaDataFile);
}

RecordEngineManager* BinaryRecording::getEngineManager()
{
	RecordEngineManager* man = new RecordEngineManager("RAWBINARY", "Binary", &(engineFactory<BinaryRecording>));
	EngineParameter* param;
	param = new EngineParameter(EngineParameter::MULTI, 0, "Spike TS/chan/sortedID File Mode|All in one|Separate timestamps|All Separated", 0);
	man->addParameter(param);
	param = new EngineParameter(EngineParameter::MULTI, 1, "TTL Event word file|In main file|Separated|Do not save ttl word", 0);
	man->addParameter(param);
	param = new EngineParameter(EngineParameter::MULTI, 2, "Other event channel file|With timestamp|Separate", 0);
	man->addParameter(param);
	return man;
}

void BinaryRecording::setParameter(EngineParameter& parameter)
{
	multiParameter(0, reinterpret_cast<int&>(m_spikeMode));
	multiParameter(1, reinterpret_cast<int&>(m_TTLMode));
	multiParameter(2, reinterpret_cast<int&>(m_eventMode));
}