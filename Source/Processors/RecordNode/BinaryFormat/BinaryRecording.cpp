/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#include "../../Settings/InfoObject.h"
#include "../../Settings/DataStream.h"

#include "../../Events/Spike.h"

#define TIC std::chrono::high_resolution_clock::now()

#define MAX_BUFFER_SIZE 40960

BinaryRecording::BinaryRecording()
{
    m_bufferSize = MAX_BUFFER_SIZE;
	m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
	m_intBuffer.malloc(MAX_BUFFER_SIZE);
    m_sampleNumberBuffer.malloc(MAX_BUFFER_SIZE);
}

BinaryRecording::~BinaryRecording() {}

String BinaryRecording::getEngineId() const
{
	return "BINARY";
}

String BinaryRecording::getProcessorString(const InfoObject* channelInfo)
{
    /* Format: Neuropixels-PXI-100.ProbeA-LFP */
    /* Convert spaces or @ symbols in source node name to underscore */
    String fName = channelInfo->getSourceNodeName().replaceCharacters(" @", "__") + "-";
    fName += String(((ChannelInfoObject*)channelInfo)->getSourceNodeId());
    fName += "." + String(((ChannelInfoObject*)channelInfo)->getStreamName());
	fName += File::getSeparatorString();
	return fName;
}

void BinaryRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{

    m_recordingNum = recordingNumber;
    m_experimentNum = experimentNumber;

	String basepath = rootFolder.getFullPathName() + rootFolder.getSeparatorString() + "experiment" + String(experimentNumber)
        + File::getSeparatorString() + "recording" + String(recordingNumber + 1) + File::getSeparatorString();

    String contPath = basepath + "continuous" + File::getSeparatorString();

    m_channelIndexes.insertMultiple(0, 0, getNumRecordedContinuousChannels());
    m_fileIndexes.insertMultiple(0, 0, getNumRecordedContinuousChannels());
    m_samplesWritten.insertMultiple(0, 0, getNumRecordedContinuousChannels());

    Array<var> continuousChannelJSON;
    Array<var> singleStreamJSON;
    Array<var> multiStreamJSON;

    Array<const ContinuousChannel*> firstChannels;
    Array<int> channelCounts;

    int streamIndex = -1;
    uint16 lastStreamId = 0;
    int indexWithinStream = 0;

    for (int ch = 0; ch < getNumRecordedContinuousChannels(); ch++)
    {

        int globalIndex = getGlobalIndex(ch); // the global channel index
        int localIndex = getLocalIndex(ch); // the local channel index (within a stream)

        const ContinuousChannel* channelInfo = getContinuousChannel(globalIndex); // channel info object

        int streamId = channelInfo->getStreamId();

        if (streamId != lastStreamId)
        {
            firstChannels.add(channelInfo);
            streamIndex++;

            if (streamIndex > 0)
            {
                multiStreamJSON.add(var(singleStreamJSON));
                channelCounts.add(indexWithinStream);
            }

            indexWithinStream = 0;
            singleStreamJSON.clear();
        }

        m_fileIndexes.set(ch, streamIndex);
        m_channelIndexes.set(ch, indexWithinStream++);

        DynamicObject::Ptr singleChannelJSON = new DynamicObject();

        singleChannelJSON->setProperty("channel_name", channelInfo->getName());
        singleChannelJSON->setProperty("description", channelInfo->getDescription());
        singleChannelJSON->setProperty("identifier", channelInfo->getIdentifier());
        singleChannelJSON->setProperty("history", channelInfo->getHistoryString());
        singleChannelJSON->setProperty("bit_volts", channelInfo->getBitVolts());
        singleChannelJSON->setProperty("units", channelInfo->getUnits());
        createChannelMetadata(channelInfo, singleChannelJSON);

        singleStreamJSON.add(var(singleChannelJSON));

        lastStreamId = streamId;

    }

    multiStreamJSON.add(var(singleStreamJSON));
    channelCounts.add(indexWithinStream);

    streamIndex = -1;

    for (auto ch : firstChannels)
    {
        streamIndex++;

        String datPath = getProcessorString(ch);
        String filename = contPath + datPath + "continuous.dat";

        LOGD("Creating file: ", contPath, datPath, "sample_numbers.npy");
        ScopedPointer<NpyFile> tFile = new NpyFile(contPath + datPath + "sample_numbers.npy", NpyType(BaseType::INT64,1));
        m_dataTimestampFiles.add(tFile.release());

        ScopedPointer<NpyFile> syncTimestampFile = new NpyFile(contPath + datPath + "timestamps.npy", NpyType(BaseType::DOUBLE,1));
        m_dataSyncTimestampFiles.add(syncTimestampFile.release());

        DynamicObject::Ptr fileJSON = new DynamicObject();
        fileJSON->setProperty("folder_name", datPath.replace(File::getSeparatorString(), "/")); //to make it more system agnostic, replace separator with only one slash
        fileJSON->setProperty("sample_rate", ch->getSampleRate());
        fileJSON->setProperty("source_processor_name", ch->getSourceNodeName());
        fileJSON->setProperty("source_processor_id", ch->getSourceNodeId());
        fileJSON->setProperty("stream_name", ch->getStreamName());
        fileJSON->setProperty("recorded_processor", ch->getNodeName());
        fileJSON->setProperty("recorded_processor_id", ch->getNodeId());
        fileJSON->setProperty("num_channels", channelCounts[streamIndex]);

        ScopedPointer<SequentialBlockFile> bFile = new SequentialBlockFile(channelCounts[streamIndex], samplesPerBlock);

        if (bFile->openFile(filename))
            m_continuousFiles.add(bFile.release());
        else
            m_continuousFiles.add(nullptr);

        fileJSON->setProperty("channels", multiStreamJSON.getReference(streamIndex));

        continuousChannelJSON.add(var(fileJSON));
    }

    //Event data files
    String eventPath(basepath + "events" + File::getSeparatorString());
    Array<var> eventChannelJSON;

    std::map<String, int> ttlMap;

    for (int ev = 0; ev < getNumRecordedEventChannels(); ev++)
    {

        const EventChannel* chan = getEventChannel(ev);
        String eventName;
        NpyType type;
        String dataFileName;

        switch (chan->getType())
        {
        case EventChannel::TEXT:
            LOGD("Got text channel");
            eventName = "MessageCenter" + File::getSeparatorString();
            type = NpyType(BaseType::CHAR, chan->getLength());
            dataFileName = "text";
            break;
        case EventChannel::TTL:
            LOGD("Got TTL channel");
            eventName = getProcessorString(chan);
            if (ttlMap.count(eventName))
                ttlMap[eventName]++;
            else
                ttlMap[eventName] = 0;
            eventName += "TTL" + (ttlMap[eventName] ? "_" + String(ttlMap[eventName]) : "") + File::getSeparatorString();
            type = NpyType(BaseType::INT16, 1);
            dataFileName = "states";
            break;
        default:
            LOGD("Got BINARY group");
            eventName = getProcessorString(chan);
            eventName += "BINARY_group";
            type = NpyType(chan->getEquivalentMetadataType(), chan->getLength());
            dataFileName = "data_array";
            break;
        }

        ScopedPointer<EventRecording> rec = new EventRecording();

        rec->data = std::make_unique<NpyFile>(eventPath + eventName + dataFileName + ".npy", type);
        rec->samples = std::make_unique<NpyFile>(eventPath + eventName + "sample_numbers.npy", NpyType(BaseType::INT64, 1));
        rec->timestamps = std::make_unique<NpyFile>(eventPath + eventName + "timestamps.npy", NpyType(BaseType::DOUBLE, 1));
        if (chan->getType() == EventChannel::TTL && m_saveTTLWords)
        {
            rec->extraFile = std::make_unique<NpyFile>(eventPath + eventName + "full_words.npy", NpyType(BaseType::UINT64, 1));
        }

        DynamicObject::Ptr jsonChannel = new DynamicObject();
        jsonChannel->setProperty("folder_name", eventName.replace(File::getSeparatorString(), "/"));
        jsonChannel->setProperty("channel_name", chan->getName());
        jsonChannel->setProperty("description", chan->getDescription());

        jsonChannel->setProperty("identifier", chan->getIdentifier());
        jsonChannel->setProperty("sample_rate", chan->getSampleRate());
        jsonChannel->setProperty("type", jsonTypeValue(type.getType()));
        jsonChannel->setProperty("source_processor", chan->getSourceNodeName());
        jsonChannel->setProperty("stream_name", chan->getStreamName());

        if (chan->getType() == EventChannel::TTL)
        {
            jsonChannel->setProperty("initial_state", int(chan->getTTLWord()));
        }

        createChannelMetadata(chan, jsonChannel);

        //rec->metaDataFile = createEventMetadataFile(chan, eventPath + eventName + "metadata.npy", jsonChannel);
        m_eventFiles.add(rec.release());
        eventChannelJSON.add(var(jsonChannel));
    }

    int nSpikeChannels = getNumRecordedSpikeChannels();

    Array<const SpikeChannel*> indexedSpikes;
    Array<uint16> indexedChannels;
    m_spikeFileIndexes.insertMultiple(0, 0, nSpikeChannels);
    m_spikeChannelIndexes.insertMultiple(0, 0, nSpikeChannels);
    String spikePath(basepath + "spikes" + File::getSeparatorString());
    Array<var> spikeChannelJSON;
    Array<var> jsonSpikeChannels;
    std::map<uint32, int> groupMap;

    for (int spikeChannelIndex = 0; spikeChannelIndex < nSpikeChannels; spikeChannelIndex++)
    {

        const SpikeChannel* ch = getSpikeChannel(spikeChannelIndex);

        DynamicObject::Ptr electrodeJSON = new DynamicObject();

        unsigned int numSpikeChannels = ch->getNumChannels();
        electrodeJSON->setProperty("name", ch->getName());
        electrodeJSON->setProperty("description", ch->getDescription());
        electrodeJSON->setProperty("identifier", ch->getIdentifier());
        electrodeJSON->setProperty("source_processor_id", ch->getSourceNodeId());
        electrodeJSON->setProperty("stream_name", ch->getStreamName());
        electrodeJSON->setProperty("sample_rate", ch->getSampleRate());
        electrodeJSON->setProperty("num_channels", (int)ch->getNumChannels());
        electrodeJSON->setProperty("pre_peak_samples", (int)ch->getPrePeakSamples());
        electrodeJSON->setProperty("post_peak_samples", (int)ch->getPostPeakSamples());

        Array<var> channelJSON;

        for (int i = 0; i < numSpikeChannels; i++)
        {
            DynamicObject::Ptr subChannelInfo = new DynamicObject();

            const ContinuousChannel* subCh = ch->getSourceChannels()[i];

            subChannelInfo->setProperty("name", subCh->getName());
            subChannelInfo->setProperty("local_index", subCh->getLocalIndex());
            subChannelInfo->setProperty("bit_volts", subCh->getBitVolts());
            channelJSON.add(var(subChannelInfo));
        }

        createChannelMetadata(ch, electrodeJSON);

        int fileIndex = m_spikeFiles.size();

        m_spikeFileIndexes.set(spikeChannelIndex, fileIndex);
        m_spikeChannelIndexes.set(spikeChannelIndex, spikeChannelIndex);

        ScopedPointer<EventRecording> rec = new EventRecording();

        uint32 procID = 0; // FIXME GenericProcessor::getProcessorFullId(ch->getSourceNodeID(), ch->getSubProcessorIdx());
        int groupIndex = ++groupMap[procID];

        String directoryName = getProcessorString(ch) + ch->getName() + File::getSeparatorString();

        rec->data = std::make_unique<NpyFile>(spikePath + directoryName + "waveforms.npy", NpyType(BaseType::INT16, ch->getTotalSamples()), ch->getNumChannels());
        rec->samples = std::make_unique<NpyFile>(spikePath + directoryName + "sample_numbers.npy", NpyType(BaseType::INT64, 1));
        rec->timestamps = std::make_unique<NpyFile>(spikePath + directoryName + "timestamps.npy", NpyType(BaseType::DOUBLE, 1));
        rec->channels = std::make_unique<NpyFile>(spikePath + directoryName + "electrode_indices.npy", NpyType(BaseType::UINT16, 1));
        rec->extraFile = std::make_unique<NpyFile>(spikePath + directoryName + "clusters.npy", NpyType(BaseType::UINT16, 1));

        electrodeJSON->setProperty("folder", directoryName.replace(File::getSeparatorString(),"/"));
        electrodeJSON->setProperty("source_channels", channelJSON);

        //rec->metaDataFile = createEventMetadataFile(ch, spikePath + spikeName + "metadata.npy", jsonFile);
        m_spikeFiles.add(rec.release());
        spikeChannelJSON.add(var(electrodeJSON));
    }

    File syncFile = File(basepath + "sync_messages.txt");
    Result res = syncFile.create();
    if (res.failed())
    {
        std::cerr << "Error creating sync text file:" << res.getErrorMessage() << std::endl;
    }
    else
    {
        m_syncTextFile = syncFile.createOutputStream();
    }

    DynamicObject::Ptr settingsJSON = new DynamicObject();

    settingsJSON->setProperty("GUI version", CoreServices::getGUIVersion());
    settingsJSON->setProperty("continuous", continuousChannelJSON);
    settingsJSON->setProperty("events", eventChannelJSON);
    settingsJSON->setProperty("spikes", spikeChannelJSON);

    FileOutputStream settingsFileStream(File(basepath + "structure.oebin"));

    settingsJSON->writeAsJSON(settingsFileStream, 2, false, 3);

}

std::unique_ptr<NpyFile> BinaryRecording::createEventMetadataFile(const MetadataEventObject* channel, String filename, DynamicObject* jsonFile)
{
    int nMetadata = channel->getEventMetadataCount();
    if (nMetadata < 1) return nullptr;

    Array<NpyType> types;
    Array<var> jsonMetadata;
    for (int i = 0; i < nMetadata; i++)
    {
        const MetadataDescriptor* md = channel->getEventMetadataDescriptor(i);
        types.add(NpyType(md->getName(), md->getType(), md->getLength()));
        DynamicObject::Ptr jsonValues = new DynamicObject();
        jsonValues->setProperty("name", md->getName());
        jsonValues->setProperty("description", md->getDescription());
        jsonValues->setProperty("identifier", md->getIdentifier());
        jsonValues->setProperty("type", jsonTypeValue(md->getType()));
        jsonValues->setProperty("length", (int)md->getLength());
        jsonMetadata.add(var(jsonValues));
    }
    if (jsonFile)
        jsonFile->setProperty("event_metadata", jsonMetadata);
    return std::make_unique<NpyFile>(filename, types);
}

template <typename TO, typename FROM>
void dataToVar(var& dataTo, const void* dataFrom, int length)
{
	const FROM* buffer = reinterpret_cast<const FROM*>(dataFrom);
	for (int i = 0; i < length; i++)
	{
		dataTo.append(static_cast<TO>(*(buffer + i)));
	}
}

String BinaryRecording::jsonTypeValue(BaseType type)
{
	switch (type)
	{
	case BaseType::CHAR:
		return "string";
	case BaseType::INT8:
		return "int8";
	case BaseType::UINT8:
		return "uint8";
	case BaseType::INT16:
		return "int16";
	case BaseType::UINT16:
		return "uint16";
	case BaseType::INT32:
		return "int32";
	case BaseType::UINT32:
		return "uint32";
	case BaseType::INT64:
		return "int64";
	case BaseType::UINT64:
		return "uint64";
	case BaseType::FLOAT:
		return "float";
	case BaseType::DOUBLE:
		return "double";
	default:
		return String();
	}
}

void BinaryRecording::createChannelMetadata(const MetadataObject* channel, DynamicObject* jsonFile)
{
	int nMetadata = channel->getMetadataCount();
	if (nMetadata < 1) return;

	Array<var> jsonMetadata;
	for (int i = 0; i < nMetadata; i++)
	{
		const MetadataDescriptor* md = channel->getMetadataDescriptor(i);
		const MetadataValue* mv = channel->getMetadataValue(i);
		DynamicObject::Ptr jsonValues = new DynamicObject();
		MetadataDescriptor::MetadataType type = md->getType();
		unsigned int length = md->getLength();
		jsonValues->setProperty("name", md->getName());
		jsonValues->setProperty("description", md->getDescription());
		jsonValues->setProperty("identifier", md->getIdentifier());
		jsonValues->setProperty("type", jsonTypeValue(type));
		jsonValues->setProperty("length", (int)length);
		var val;

        if (type == MetadataDescriptor::CHAR)
		{
			String tmp;
			mv->getValue(tmp);
			val = tmp;
		}
		else
		{
			const void* buf = mv->getRawValuePointer();
			switch (type)
			{
			case MetadataDescriptor::INT8:
				dataToVar<int, int8>(val, buf, length);
				break;
			case MetadataDescriptor::UINT8:
				dataToVar<int, uint8>(val, buf, length);
				break;
			case MetadataDescriptor::INT16:
				dataToVar<int, int16>(val, buf, length);
				break;
			case MetadataDescriptor::UINT16:
				dataToVar<int, uint16>(val, buf, length);
				break;
			case MetadataDescriptor::INT32:
				dataToVar<int, int32>(val, buf, length);
				break;
				//A full uint32 doesn't fit in a regular int, so we increase size
			case MetadataDescriptor::UINT32:
				dataToVar<int64, uint8>(val, buf, length);
				break;
			case MetadataDescriptor::INT64:
				dataToVar<int64, int64>(val, buf, length);
				break;
				//This might overrun and end negative if the uint64 is really big, but there is no way to store a full uint64 in a var
			case MetadataDescriptor::UINT64:
				dataToVar<int64, uint64>(val, buf, length);
				break;
			case MetadataDescriptor::FLOAT:
				dataToVar<float, float>(val, buf, length);
				break;
			case MetadataDescriptor::DOUBLE:
				dataToVar<double, double>(val, buf, length);
				break;
			default:
				val = "invalid";
			}
		}
		jsonValues->setProperty("value", val);
		jsonMetadata.add(var(jsonValues));
	}
	jsonFile->setProperty("channel_metadata", jsonMetadata);
}

void BinaryRecording::closeFiles()
{

    m_continuousFiles.clear();
    m_eventFiles.clear();
    m_spikeFiles.clear();

    m_syncTextFile.reset(nullptr);

    m_channelIndexes.clear();
    m_fileIndexes.clear();
    m_samplesWritten.clear();
    
    m_dataTimestampFiles.clear();
    m_dataSyncTimestampFiles.clear();

    m_spikeChannelIndexes.clear();
    m_spikeFileIndexes.clear();

    m_scaledBuffer.malloc(MAX_BUFFER_SIZE);
    m_intBuffer.malloc(MAX_BUFFER_SIZE);
    m_sampleNumberBuffer.malloc(MAX_BUFFER_SIZE);
    m_bufferSize = MAX_BUFFER_SIZE;

}

void BinaryRecording::writeEventMetadata(const MetadataEvent* event, NpyFile* file)
{
    if (!file || !event) return;
    int nMetadata = event->getMetadataValueCount();
    for (int i = 0; i < nMetadata; i++)
    {
        const MetadataValue* val = event->getMetadataValue(i);
        file->writeData(val->getRawValuePointer(), val->getDataSize());
    }
}

void BinaryRecording::increaseEventCounts(EventRecording* rec)
{
    rec->data->increaseRecordCount();
    rec->samples->increaseRecordCount();
    rec->timestamps->increaseRecordCount();
    if (rec->channels) rec->channels->increaseRecordCount();
    if (rec->extraFile) rec->extraFile->increaseRecordCount();
}

void BinaryRecording::writeContinuousData(int writeChannel,
    int realChannel,
    const float* dataBuffer,
    const double* timestampBuffer,
    int size)
{

    if (!size)
        return;

    /* If our internal buffer is too small to hold the data... */
	if (size > m_bufferSize) //shouldn't happen, but if does, this prevents crash...
	{
		std::cerr << "[RN] Write buffer overrun, resizing from: " << m_bufferSize << " to: " << size << std::endl;
		m_scaledBuffer.malloc(size);
		m_intBuffer.malloc(size);
        m_sampleNumberBuffer.malloc(size);
		m_bufferSize = size;
	}

    /* Convert signal from float to int w/ bitVolts scaling */
	double multFactor = 1 / (float(0x7fff) * getContinuousChannel(realChannel)->getBitVolts());
	FloatVectorOperations::copyWithMultiply(m_scaledBuffer.getData(), dataBuffer, multFactor, size);
	AudioDataConverters::convertFloatToInt16LE(m_scaledBuffer.getData(), m_intBuffer.getData(), size);

    /* Get the file index that belongs to the current recording channel */
	int fileIndex = m_fileIndexes[writeChannel];

    /* Write the data to that file */
	m_continuousFiles[fileIndex]->writeChannel(
		m_samplesWritten[writeChannel],
		m_channelIndexes[writeChannel],
		m_intBuffer.getData(),
        size);
    
    m_samplesWritten.set(writeChannel, m_samplesWritten[writeChannel] + size);

    /* If is first channel in subprocessor */
	if (m_channelIndexes[writeChannel] == 0)
    {

		int64 baseSampleNumber = getLatestSampleNumber(writeChannel);

		for (int i = 0; i < size; i++)
            /* Generate int sample number */
            m_sampleNumberBuffer[i] = baseSampleNumber + i;

        /* Write int timestamps to disc */
		m_dataTimestampFiles[fileIndex]->writeData(m_sampleNumberBuffer, size*sizeof(int64));
		m_dataTimestampFiles[fileIndex]->increaseRecordCount(size);

        //LOGD("BinaryRecording::writeSynchronizedData: ", *timestampBuffer);
        //std::cout << timestampBuffer

        m_dataSyncTimestampFiles[fileIndex]->writeData(timestampBuffer, size*sizeof(double));
        m_dataSyncTimestampFiles[fileIndex]->increaseRecordCount(size);

	}
}

void BinaryRecording::writeEvent(int eventIndex, const EventPacket& event)
{

    const EventChannel* info = getEventChannel(eventIndex);

	EventPtr ev = Event::deserialize(event, info);
	EventRecording* rec = m_eventFiles[eventIndex];

    if (!rec) return;

	if (ev->getEventType() == EventChannel::TTL)
	{

        TTLEvent* ttl = static_cast<TTLEvent*>(ev.get());

        int16 state = (ttl->getLine() + 1) * (ttl->getState() ? 1 : -1);
		rec->data->writeData(&state, sizeof(int16));

        int64 sampleIdx = ev->getSampleNumber();
        rec->samples->writeData(&sampleIdx, sizeof(int64));

        double ts = ev->getTimestampInSeconds();
        rec->timestamps->writeData(&ts, sizeof(double));

        if (rec->extraFile)
        {
            uint64 fullWord = ttl->getWord();
            rec->extraFile->writeData(&fullWord, sizeof(uint64));
        }

	}
	else if (ev->getEventType() == EventChannel::TEXT)
	{

        TextEvent* text = static_cast<TextEvent*>(ev.get());

        int64 sampleIdx = text->getSampleNumber();
        rec->samples->writeData(&sampleIdx, sizeof(int64));

        double ts = text->getTimestampInSeconds();
        rec->timestamps->writeData(&ts, sizeof(double));

		rec->data->writeData(ev->getRawDataPointer(), info->getDataSize());
	}

    // NOT IMPLEMENTED
	//writeEventMetadata(ev.get(), rec->metaDataFile.get());

	increaseEventCounts(rec);

}

void BinaryRecording::writeSpike(int electrodeIndex, const Spike* spike)
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
	rec->data->writeData(m_intBuffer.getData(), totalSamples*sizeof(int16));

	int64 sampleIdx = spike->getSampleNumber();
	rec->samples->writeData(&sampleIdx, sizeof(int64));

    double ts = spike->getTimestampInSeconds();
    rec->timestamps->writeData(&ts, sizeof(double));

	rec->channels->writeData(&spikeChannel, sizeof(uint16));

	uint16 sortedId = spike->getSortedId();
	rec->extraFile->writeData(&sortedId, sizeof(uint16));

    // NOT IMPLEMENTED
	//writeEventMetadata(spike, rec->metaDataFile.get());

	increaseEventCounts(rec);

}

void BinaryRecording::writeTimestampSyncText(uint64 streamId, int64 sampleNumber, float sourceSampleRate, String text)
{
	if (!m_syncTextFile)
		return;
	m_syncTextFile->writeText(text + ": " + String(sampleNumber) + "\r\n", false, false, nullptr);
    m_syncTextFile->flush();



}

RecordEngineManager* BinaryRecording::getEngineManager()
{
    RecordEngineManager* man = new RecordEngineManager("BINARY", "Binary",
                                                       &(engineFactory<BinaryRecording>));
    EngineParameter* param;
    param = new EngineParameter(EngineParameter::BOOL, 0, "Record TTL full words", true);
    man->addParameter(param);
    return man;
}

void BinaryRecording::setParameter(EngineParameter& parameter)
{
	boolParameter(0, m_saveTTLWords);
}
