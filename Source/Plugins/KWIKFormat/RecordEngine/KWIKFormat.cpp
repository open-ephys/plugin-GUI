/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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
 
 #include "KWIKFormat.h"

#define CHUNK_XSIZE 2048
#define EVENT_CHUNK_SIZE 8
#define SPIKE_CHUNK_XSIZE 8
#define SPIKE_CHUNK_YSIZE 40
#define TIMESTAMP_CHUNK_SIZE 16
#define MAX_TRANSFORM_SIZE 512
#define MAX_STR_SIZE 256
 
 //KWD File

KWDFile::KWDFile(int processorNumber, String basename) : HDF5FileBase()
{
    initFile(processorNumber, basename);
}

KWDFile::KWDFile() : HDF5FileBase()
{
}

KWDFile::~KWDFile() {}

String KWDFile::getFileName()
{
    return filename;
}

void KWDFile::initFile(int processorNumber, String basename)
{
    if (isOpen()) return;
    filename = basename + "_" + String(processorNumber) + ".raw.kwd";
    readyToOpen=true;
}

void KWDFile::startNewRecording(int recordingNumber, int nChannels, KWIKRecordingInfo* info)
{
    this->recordingNumber = recordingNumber;
    this->nChannels = nChannels;
    this->multiSample = info->multiSample;
    uint8 mSample = info->multiSample ? 1 : 0;

	ScopedPointer<HDF5RecordingData> bitVoltsSet;
	ScopedPointer<HDF5RecordingData> sampleRateSet;

    String recordPath = String("/recordings/")+String(recordingNumber);
    CHECK_ERROR(createGroup(recordPath));
    CHECK_ERROR(setAttributeStr(info->name,recordPath,String("name")));
	CHECK_ERROR(setAttribute(BaseDataType::U64, &(info->start_time), recordPath, String("start_time")));
	CHECK_ERROR(setAttribute(BaseDataType::U32, &(info->start_sample), recordPath, String("start_sample")));
	CHECK_ERROR(setAttribute(BaseDataType::F32, &(info->sample_rate), recordPath, String("sample_rate")));
	CHECK_ERROR(setAttribute(BaseDataType::U32, &(info->bit_depth), recordPath, String("bit_depth")));
    CHECK_ERROR(createGroup(recordPath+"/application_data"));
   // CHECK_ERROR(setAttributeArray(F32,info->bitVolts.getRawDataPointer(),info->bitVolts.size(),recordPath+"/application_data",String("channel_bit_volts")));
	bitVoltsSet = createDataSet(BaseDataType::F32, info->bitVolts.size(), 0, recordPath + "/application_data/channel_bit_volts");
	if (bitVoltsSet.get())
		bitVoltsSet->writeDataBlock(info->bitVolts.size(), BaseDataType::F32, info->bitVolts.getRawDataPointer());
	else
		std::cerr << "Error creating bitvolts data set" << std::endl;
	
	CHECK_ERROR(setAttribute(BaseDataType::U8, &mSample, recordPath + "/application_data", String("is_multiSampleRate_data")));
    //CHECK_ERROR(setAttributeArray(F32,info->channelSampleRates.getRawDataPointer(),info->channelSampleRates.size(),recordPath+"/application_data",String("channel_sample_rates")));
	sampleRateSet = createDataSet(BaseDataType::F32, info->channelSampleRates.size(), 0, recordPath + "/application_data/channel_sample_rates");
	if (sampleRateSet.get())
		sampleRateSet->writeDataBlock(info->channelSampleRates.size(), BaseDataType::F32, info->channelSampleRates.getRawDataPointer());
	else
		std::cerr << "Error creating sample rates data set" << std::endl;

	recdata = createDataSet(BaseDataType::I16, 0, nChannels, CHUNK_XSIZE, recordPath + "/data");
    if (!recdata.get())
        std::cerr << "Error creating data set" << std::endl;

	tsData = createDataSet(BaseDataType::I64, 0, nChannels, TIMESTAMP_CHUNK_SIZE, recordPath + "/application_data/timestamps");
	if (!tsData.get())
		std::cerr << "Error creating timestamps data set" << std::endl;

    curChan = nChannels;
}

void KWDFile::stopRecording()
{
    Array<uint32> samples;
    String path = String("/recordings/")+String(recordingNumber)+String("/data");
    recdata->getRowXPositions(samples);

	CHECK_ERROR(setAttributeArray(BaseDataType::U32, samples.getRawDataPointer(), samples.size(), path, "valid_samples"));
    //ScopedPointer does the deletion and destructors the closings
    recdata = nullptr;
	tsData = nullptr;
}

int KWDFile::createFileStructure()
{
    const uint16 ver = 2;
    if (createGroup("/recordings")) return -1;
	if (setAttribute(BaseDataType::U16, (void*)&ver, "/", "kwik_version")) return -1;
    return 0;
}

void KWDFile::writeBlockData(int16* data, int nSamples)
{
	CHECK_ERROR(recdata->writeDataBlock(nSamples, BaseDataType::I16, data));
}

void KWDFile::writeRowData(int16* data, int nSamples)
{
    if (curChan >= nChannels)
    {
        curChan=0;
    }
	CHECK_ERROR(recdata->writeDataRow(curChan, nSamples, BaseDataType::I16, data));
    curChan++;
}

void KWDFile::writeRowData(int16* data, int nSamples, int channel)
{
	if (channel >= 0 && channel < nChannels)
	{
		CHECK_ERROR(recdata->writeDataRow(channel, nSamples, BaseDataType::I16, data));
		curChan = channel;
	}
}

void KWDFile::writeTimestamps(int64* ts, int nTs, int channel)
{
	if (channel >= 0 && channel < nChannels)
	{
		CHECK_ERROR(tsData->writeDataRow(channel, nTs, BaseDataType::I64, ts));
	}
}

//KWE File

KWEFile::KWEFile(String basename) : HDF5FileBase()
{
    initFile(basename);
}

KWEFile::KWEFile() : HDF5FileBase()
{

}

KWEFile::~KWEFile() {}

String KWEFile::getFileName()
{
    return filename;
}

void KWEFile::initFile(String basename)
{
    if (isOpen()) return;
    filename = basename + ".kwe";
    readyToOpen=true;
}

int KWEFile::createFileStructure()
{
    const uint16 ver = 2;
    if (createGroup("/recordings")) return -1;
    if (createGroup("/event_types")) return -1;
    for (int i=0; i < eventNames.size(); i++)
    {
        ScopedPointer<HDF5RecordingData> dSet;
        String path = "/event_types/" + eventNames[i];
        if (createGroup(path)) return -1;
        path += "/events";
        if (createGroup(path)) return -1;
		dSet = createDataSet(BaseDataType::U64, 0, EVENT_CHUNK_SIZE, path + "/time_samples");
        if (!dSet) return -1;
		dSet = createDataSet(BaseDataType::U16, 0, EVENT_CHUNK_SIZE, path + "/recording");
        if (!dSet) return -1;
        path += "/user_data";
        if (createGroup(path)) return -1;
		dSet = createDataSet(BaseDataType::U8, 0, EVENT_CHUNK_SIZE, path + "/eventID");
        if (!dSet) return -1;
		dSet = createDataSet(BaseDataType::U8, 0, EVENT_CHUNK_SIZE, path + "/nodeID");
        if (!dSet) return -1;
        dSet = createDataSet(eventTypes[i],0,EVENT_CHUNK_SIZE,path + "/" + eventDataNames[i]);
        if (!dSet) return -1;
    }
	if (setAttribute(BaseDataType::U16, (void*)&ver, "/", "kwik_version")) return -1;

    return 0;
}

void KWEFile::startNewRecording(int recordingNumber, KWIKRecordingInfo* info)
{
    this->recordingNumber = recordingNumber;
    kwdIndex=0;
    String recordPath = String("/recordings/")+String(recordingNumber);
    CHECK_ERROR(createGroup(recordPath));
    CHECK_ERROR(setAttributeStr(info->name,recordPath,String("name")));
	CHECK_ERROR(setAttribute(BaseDataType::U64, &(info->start_time), recordPath, String("start_time")));
    //	CHECK_ERROR(setAttribute(U32,&(info->start_sample),recordPath,String("start_sample")));
	CHECK_ERROR(setAttribute(BaseDataType::F32, &(info->sample_rate), recordPath, String("sample_rate")));
	CHECK_ERROR(setAttribute(BaseDataType::U32, &(info->bit_depth), recordPath, String("bit_depth")));
   // CHECK_ERROR(createGroup(recordPath + "/raw"));
  //  CHECK_ERROR(createGroup(recordPath + "/raw/hdf5_paths"));

    for (int i = 0; i < eventNames.size(); i++)
    {
        HDF5RecordingData* dSet;
        String path = "/event_types/" + eventNames[i] + "/events";
        dSet = getDataSet(path + "/time_samples");
        if (!dSet)
            std::cerr << "Error loading event timestamps dataset for type " << i << std::endl;
        timeStamps.add(dSet);
        dSet = getDataSet(path + "/recording");
        if (!dSet)
            std::cerr << "Error loading event recordings dataset for type " << i << std::endl;
        recordings.add(dSet);
        dSet = getDataSet(path + "/user_data/eventID");
        if (!dSet)
            std::cerr << "Error loading event ID dataset for type " << i << std::endl;
        eventID.add(dSet);
        dSet = getDataSet(path + "/user_data/nodeID");
        if (!dSet)
            std::cerr << "Error loading event node ID dataset for type " << i << std::endl;
        nodeID.add(dSet);
        dSet = getDataSet(path + "/user_data/" + eventDataNames[i]);
        if (!dSet)
            std::cerr << "Error loading event channel dataset for type " << i << std::endl;
        eventData.add(dSet);
    }
}

void KWEFile::stopRecording()
{
    timeStamps.clear();
    recordings.clear();
    eventID.clear();
    nodeID.clear();
    eventData.clear();
}

void KWEFile::writeEvent(int type, uint8 id, uint8 processor, void* data, int64 timestamp)
{
    if (type > eventNames.size() || type < 0)
    {
        std::cerr << "HDF5::writeEvent Invalid event type " << type << std::endl;
        return;
    }
	CHECK_ERROR(timeStamps[type]->writeDataBlock(1, BaseDataType::U64, &timestamp));
	CHECK_ERROR(recordings[type]->writeDataBlock(1, BaseDataType::I32, &recordingNumber));
	CHECK_ERROR(eventID[type]->writeDataBlock(1, BaseDataType::U8, &id));
	CHECK_ERROR(nodeID[type]->writeDataBlock(1, BaseDataType::U8, &processor));
    CHECK_ERROR(eventData[type]->writeDataBlock(1,eventTypes[type],data));
}

/*void KWEFile::addKwdFile(String filename)
{
	if (kwdIndex == 0)
	{
		CHECK_ERROR(setAttributeStr(filename + "/recordings/" + String(recordingNumber), "/recordings/" + String(recordingNumber) +
			"/raw", "hdf5_path"));
	}
    CHECK_ERROR(setAttributeStr(filename + "/recordings/" + String(recordingNumber),"/recordings/" + String(recordingNumber) +
                                "/raw/hdf5_paths",String(kwdIndex)));
    kwdIndex++;
}*/

void KWEFile::addEventType(String name, BaseDataType type, String dataName)
{
    eventNames.add(name);
    eventTypes.add(type);
    eventDataNames.add(dataName);
}

//KWX File

KWXFile::KWXFile(String basename) : HDF5FileBase()
{
    initFile(basename);
    numElectrodes=0;
    transformVector.malloc(MAX_TRANSFORM_SIZE);
}

KWXFile::KWXFile() : HDF5FileBase()
{
    numElectrodes=0;
    transformVector.malloc(MAX_TRANSFORM_SIZE);
}

KWXFile::~KWXFile()
{
}

String KWXFile::getFileName()
{
    return filename;
}

void KWXFile::initFile(String basename)
{
    if (isOpen()) return;
    filename = basename + ".kwx";
    readyToOpen=true;
}

int KWXFile::createFileStructure()
{
    const uint16 ver = 2;
    if (createGroup("/channel_groups")) return -1;
	if (setAttribute(BaseDataType::U16, (void*)&ver, "/", "kwik_version")) return -1;
    for (int i=0; i < channelArray.size(); i++)
    {
        int res = createChannelGroup(i);
        if (res) return -1;
    }
    return 0;
}

void KWXFile::addChannelGroup(int nChannels)
{
    channelArray.add(nChannels);
    numElectrodes++;
}

int KWXFile::createChannelGroup(int index)
{
    ScopedPointer<HDF5RecordingData> dSet;
    int nChannels = channelArray[index];
    String path("/channel_groups/"+String(index));
    CHECK_ERROR(createGroup(path));
	dSet = createDataSet(BaseDataType::I16, 0, 0, nChannels, SPIKE_CHUNK_XSIZE, SPIKE_CHUNK_YSIZE, path + "/waveforms_filtered");
    if (!dSet) return -1;
	dSet = createDataSet(BaseDataType::U64, 0, SPIKE_CHUNK_XSIZE, path + "/time_samples");
    if (!dSet) return -1;
	dSet = createDataSet(BaseDataType::U16, 0, SPIKE_CHUNK_XSIZE, path + "/recordings");
    if (!dSet) return -1;
    return 0;
}

void KWXFile::startNewRecording(int recordingNumber)
{
    HDF5RecordingData* dSet;
    String path;
    this->recordingNumber = recordingNumber;

    for (int i=0; i < channelArray.size(); i++)
    {
        path = "/channel_groups/"+String(i);
        dSet=getDataSet(path+"/waveforms_filtered");
        if (!dSet)
            std::cerr << "Error loading spikes dataset for group " << i << std::endl;
        spikeArray.add(dSet);
        dSet=getDataSet(path+"/time_samples");
        if (!dSet)
            std::cerr << "Error loading spike timestamp dataset for group " << i << std::endl;
        timeStamps.add(dSet);
        dSet=getDataSet(path+"/recordings");
        if (!dSet)
            std::cerr << "Error loading spike recordings dataset for group " << i << std::endl;
        recordingArray.add(dSet);
    }
}

void KWXFile::stopRecording()
{
    spikeArray.clear();
    timeStamps.clear();
    recordingArray.clear();
}

void KWXFile::resetChannels()
{
    stopRecording(); //Just in case
    channelArray.clear();
}

void KWXFile::writeSpike(int groupIndex, int nSamples, const float* data, Array<float>& bitVolts, int64 timestamp)
{
    if ((groupIndex < 0) || (groupIndex >= numElectrodes))
    {
        std::cerr << "HDF5::writeSpike Electrode index out of bounds " << groupIndex << std::endl;
        return;
    }
    int nChans= channelArray[groupIndex];
    int16* dst=transformVector;

    //Given the way we store spike data, we need to transpose it to store in
    //N x NSAMPLES x NCHANNELS as well as convert from float to i16
    for (int i = 0; i < nSamples; i++)
    {
        for (int j = 0; j < nChans; j++)
		{
			*(dst++) = static_cast<int16>((*(data+j*nSamples+i))/bitVolts[j]);
        }
    }

	CHECK_ERROR(spikeArray[groupIndex]->writeDataBlock(1, nSamples, BaseDataType::I16, transformVector));
	CHECK_ERROR(recordingArray[groupIndex]->writeDataBlock(1, BaseDataType::I32, &recordingNumber));
	CHECK_ERROR(timeStamps[groupIndex]->writeDataBlock(1, BaseDataType::U64, &timestamp));
}
