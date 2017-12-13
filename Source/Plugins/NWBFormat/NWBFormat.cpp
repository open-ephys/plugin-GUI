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
 
 #include "NWBFormat.h"
 using namespace NWBRecording;

#ifndef EVENT_CHUNK_SIZE
#define EVENT_CHUNK_SIZE 8
#endif

#ifndef SPIKE_CHUNK_XSIZE
#define SPIKE_CHUNK_XSIZE 8
#endif

#ifndef SPIKE_CHUNK_YSIZE
#define SPIKE_CHUNK_YSIZE 40
#endif

 #define MAX_BUFFER_SIZE 40960

 NWBFile::NWBFile(String fName, String ver, String idText) : HDF5FileBase(), filename(fName), identifierText(idText), GUIVersion(ver)
 {
	 //Init stuff
	 readyToOpen=true; //In KWIK this is in initFile, but the new recordEngine methods make it safe for it to be here

	 scaledBuffer.malloc(MAX_BUFFER_SIZE);
	 intBuffer.malloc(MAX_BUFFER_SIZE);
	 bufferSize = MAX_BUFFER_SIZE;
 }
 
 NWBFile::~NWBFile()
 {
 }


 void NWBFile::setXmlText(const String& xmlText)
 {
	 this->xmlText = &xmlText;
 }

//All int return values are 0 if succesful, other number otherwise
int NWBFile::createFileStructure()
{
	//This is called when the file is first created (not if it's opened but it already exists)
	//Creates all the common structures, like file version and such
	if (createGroup("/acquisition")) return -1;
	if (createGroup("/analysis")) return -1;
	if (createGroup("/epochs")) return -1;
	if (createGroup("/general")) return -1;
	if (createGroup("/processing")) return -1;
	if (createGroup("/stimulus")) return -1;

	if (createGroup("/acquisition/timeseries")) return -1;


	if (createGroup("/general/data_collection")) return -1;

	CHECK_ERROR(setAttributeStr(String("OpenEphys GUI v") + GUIVersion, "/general/data_collection", "software"));
	CHECK_ERROR(setAttributeStr(*xmlText, "/general/data_collection", "configuration"));
	
	//TODO: Add default datasets
	//Modify this one once we have JUCE4 to allow UTC time 
	String time = Time::getCurrentTime().formatted("%Y-%m-%dT%H:%M:%S");
	createTextDataSet("", "file_create_date", time);
	createTextDataSet("", "identifier", identifierText);
	createTextDataSet("", "nwb_version", "NWB-1.0.6");
	createTextDataSet("", "session_description", " ");
	createTextDataSet("", "session_start_time", time);
	
	return 0;
}
 
bool NWBFile::startNewRecording(int recordingNumber, const Array<ContinuousGroup>& continuousArray,
	const Array<const EventChannel*>& eventArray, const Array<const SpikeChannel*>& electrodeArray)
 {
	 //Created each time a new recording is started. Creates the specific file structures and attributes
	 //for that specific recording
	 String basePath;
	 StringArray ancestry;
	 String rootPath = "/acquisition/timeseries/recording" + String(recordingNumber + 1);
	 if (createGroup(rootPath)) return false;
	 if (createGroupIfDoesNotExist(rootPath + "/continuous")) return false;
	 if (createGroupIfDoesNotExist(rootPath + "/spikes")) return false;
	 if (createGroupIfDoesNotExist(rootPath + "/events")) return false;

	 //just in case
	 continuousDataSets.clearQuick(true);
	 spikeDataSets.clearQuick(true);
	 eventDataSets.clearQuick(true);

	 ScopedPointer<TimeSeries> tsStruct;
	 ScopedPointer<HDF5RecordingData> dSet;

	 int nCont;
	 nCont = continuousArray.size();
	 for (int i = 0; i < nCont; i++)
	 {
		 //All channels in a group will share the same source information (any caller to this method MUST assure this happen
		 //so we just pick the first channel.
		 const DataChannel* info = continuousArray.getReference(i)[0];
		 basePath = rootPath + "/continuous/processor" + String(info->getCurrentNodeID()) + "_" + String(info->getSourceNodeID());
		 if (info->getSourceSubprocessorCount() > 1) basePath += "." + String(info->getSubProcessorIdx());
		 String name = info->getCurrentNodeName() + " (" + String(info->getCurrentNodeID()) + ") From " + info->getSourceName() + " (" + String(info->getSourceNodeID());
		 if (info->getSourceSubprocessorCount() > 1) name += "." + String(info->getSubProcessorIdx());
		 name += ")";
		 ancestry.clearQuick();
		 ancestry.add("Timeseries");
		 ancestry.add("ElectricalSeries");
		 if (!createTimeSeriesBase(basePath, name, "Stores acquired voltage data from extracellular recordings", "", ancestry)) return false;
		 tsStruct = new TimeSeries();
		 tsStruct->basePath = basePath;
		 dSet = createDataSet(BaseDataType::I16, 0, continuousArray.getReference(i).size(), CHUNK_XSIZE, basePath + "/data");
		 if (dSet == nullptr)
		 {
			 std::cerr << "Error creating dataset for " << name << std::endl;
			 return false;
		 }
		 else
		 {
			 createDataAttributes(basePath, info->getBitVolts(), info->getBitVolts() / 65536, info->getDataUnits());
		 }
		 tsStruct->baseDataSet = dSet;

		 dSet = createTimestampDataSet(basePath, CHUNK_XSIZE);
		 if (dSet == nullptr) return false;
		 tsStruct->timestampDataSet = dSet;

		 basePath = basePath + "/oe_extra_info";
		 if (createGroup(basePath)) return false;
		 int nChans = continuousArray.getReference(i).size();
		 for (int j = 0; j < nChans; j++)
		 {
			 String channelPath = basePath + "/channel" + String(j + 1);
			 const DataChannel* chan = continuousArray.getReference(i)[j];
			 createExtraInfo(channelPath, chan->getName(), chan->getDescription(), chan->getIdentifier(), chan->getSourceIndex(), chan->getSourceTypeIndex());
			 createChannelMetaDataSets(channelPath + "/channel_metadata", chan);
		 }
		 continuousDataSets.add(tsStruct.release());
	 }		 

	 nCont = electrodeArray.size();
	 for (int i = 0; i < nCont; i++)
	 {
		 basePath = rootPath + "/spikes/electrode" + String(i + 1);
		 const SpikeChannel* info = electrodeArray[i];
		 String sourceName = info->getSourceName() + "_" + String(info->getSourceNodeID());
		 if (info->getSourceSubprocessorCount() > 1) sourceName = sourceName + "." + String(info->getSubProcessorIdx());
		 ancestry.clearQuick();
		 ancestry.add("Timeseries");
		 ancestry.add("SpikeEventSeries");
		 if (!createTimeSeriesBase(basePath, sourceName, "Snapshorts of spike events from data", info->getName(), ancestry)) return false;

		 tsStruct = new TimeSeries();
		 tsStruct->basePath = basePath;

		 dSet = createDataSet(BaseDataType::I16, 0, info->getNumChannels(), info->getTotalSamples(), SPIKE_CHUNK_XSIZE, basePath + "/data");
		 if (dSet == nullptr)
		 {
			 std::cerr << "Error creating dataset for electrode " << i << std::endl;
			 return false;
		 }
		 else
		 {
			 createDataAttributes(basePath, info->getChannelBitVolts(0), info->getChannelBitVolts(0) / 65536, "volt");
		 }
		 tsStruct->baseDataSet = dSet;
		 dSet = createTimestampDataSet(basePath, SPIKE_CHUNK_XSIZE);
		 if (dSet == nullptr) return false;
		 tsStruct->timestampDataSet = dSet;

		 basePath = basePath + "/oe_extra_info";
		 createExtraInfo(basePath, info->getName(), info->getDescription(), info->getIdentifier(), info->getSourceIndex(), info->getSourceTypeIndex());
		 createChannelMetaDataSets(basePath + "/channel_metadata", info);
		 createEventMetaDataSets(basePath + "/spike_metadata", tsStruct, info);

		 spikeDataSets.add(tsStruct.release());


	 }
	
	 nCont = eventArray.size();
	 int nTTL = 0;
	 int nTXT = 0;
	 int nBIN = 0;
	 for (int i = 0; i < nCont; i++)
	 {
		 basePath = rootPath + "/events";
		 const EventChannel* info = eventArray[i];
		 String sourceName = info->getSourceName() + "_" + String(info->getSourceNodeID());
		 if (info->getSourceSubprocessorCount() > 1) sourceName = sourceName + "." + String(info->getSubProcessorIdx());
		 ancestry.clearQuick();
		 ancestry.add("Timeseries");

		 String helpText;

		 switch (info->getChannelType())
		 {
		 case EventChannel::TTL:
			 nTTL += 1;
			 basePath = basePath + "/ttl" + String(nTTL);
			 ancestry.add("IntervalSeries");
			 ancestry.add("TTLSeries");
			 helpText = "Stores the start and stop times for TTL events";
			 break;
		 case EventChannel::TEXT:
			 nTXT += 1;
			 basePath = basePath + "/text" + String(nTXT);
			 ancestry.add("AnnotationSeries");
			 helpText = "Time-stamped annotations about an experiment";
			 break;
		 default:
			 nBIN += 1;
			 basePath = basePath + "/binary" + String(nBIN);
			 ancestry.add("BinarySeries");
			 helpText = "Stores arbitrary binary data";
			 break;
		 }

		 if (!createTimeSeriesBase(basePath, sourceName, helpText, info->getDescription(), ancestry)) return false;

		 tsStruct = new TimeSeries();
		 tsStruct->basePath = basePath;

		 if (info->getChannelType() >= EventChannel::BINARY_BASE_VALUE) //only binary events have length greater than 1
		 {
			 dSet = createDataSet(getEventH5Type(info->getChannelType(), info->getLength()), 0, info->getLength(), EVENT_CHUNK_SIZE, basePath + "/data");;
		 }
		 else
		 {
			 dSet = createDataSet(getEventH5Type(info->getChannelType(), info->getLength()), 0, EVENT_CHUNK_SIZE, basePath + "/data");
		 }

		 if (dSet == nullptr)
		 {
			 std::cerr << "Error creating dataset for event " << info->getName() << std::endl;
			 return false;
		 }
		 else
		 {
			 createDataAttributes(basePath, NAN, NAN, "n/a");
		 }


		 tsStruct->baseDataSet = dSet;
		 dSet = createTimestampDataSet(basePath, EVENT_CHUNK_SIZE);
		 if (dSet == nullptr) return false;
		 tsStruct->timestampDataSet = dSet;

		 dSet = createDataSet(BaseDataType::U8, 0, EVENT_CHUNK_SIZE, basePath + "/control");
		 if (dSet == nullptr) return false;
		 tsStruct->controlDataSet = dSet;

		 if (info->getChannelType() == EventChannel::TTL)
		 {
			 dSet = createDataSet(BaseDataType::U8, 0, info->getDataSize(), EVENT_CHUNK_SIZE, basePath + "/full_word");
			 if (dSet == nullptr) return false;
			 tsStruct->ttlWordDataSet = dSet;
		 }

		 basePath = basePath + "/oe_extra_info";
		 createExtraInfo(basePath, info->getName(), info->getDescription(), info->getIdentifier(), info->getSourceIndex(), info->getSourceTypeIndex());
		 createChannelMetaDataSets(basePath + "/channel_metadata", info);
		 createEventMetaDataSets(basePath + "/event_metadata", tsStruct, info);
		 eventDataSets.add(tsStruct.release());

	 }
	 basePath = rootPath + "/events/sync_messages";
	 ancestry.clearQuick();
	 ancestry.add("Timeseries");
	 ancestry.add("AnnotationSeries");
	 String desc = "Stores recording start timestamps for each processor in text format";
	 if (!createTimeSeriesBase(basePath, "Autogenerated messages", desc, desc, ancestry)) return false;
	 tsStruct = new TimeSeries();
	 tsStruct->basePath = basePath;
	 dSet = createDataSet(BaseDataType::STR(100), 0, 1, basePath + "/data");
	 if (dSet == nullptr)
	 {
		 std::cerr << "Error creating dataset for sync messages" << std::endl;
		 return false;
	 }
	 else
	 {
		 createDataAttributes(basePath, NAN, NAN, "n/a");
	 }
	 tsStruct->baseDataSet = dSet;
	 dSet = createTimestampDataSet(basePath, 1);
	 if (dSet == nullptr) return false;
	 tsStruct->timestampDataSet = dSet;

	 dSet = createDataSet(BaseDataType::U8, 0, 1, basePath + "/control");
	 if (dSet == nullptr) return false;
	 tsStruct->controlDataSet = dSet;
	 syncMsgDataSet = tsStruct;

	 return true;
 }
 
 void NWBFile::stopRecording()
 {
	 int nObjs = continuousDataSets.size();
	 const TimeSeries* tsStruct;
	 for (int i = 0; i < nObjs; i++)
	 {
		 tsStruct = continuousDataSets[i];
		 CHECK_ERROR(setAttribute(BaseDataType::U64, &(tsStruct->numSamples), tsStruct->basePath, "num_samples"));
	 }
	 nObjs = spikeDataSets.size();
	 for (int i = 0; i < nObjs; i++)
	 {
		 tsStruct = spikeDataSets[i];
		 CHECK_ERROR(setAttribute(BaseDataType::U64, &(tsStruct->numSamples), tsStruct->basePath, "num_samples"));
	 }
	 nObjs = eventDataSets.size();
	 for (int i = 0; i < nObjs; i++)
	 {
		 tsStruct = eventDataSets[i];
		 CHECK_ERROR(setAttribute(BaseDataType::U64, &(tsStruct->numSamples), tsStruct->basePath, "num_samples"));
	 }
	 
	 CHECK_ERROR(setAttribute(BaseDataType::U64, &(syncMsgDataSet->numSamples), syncMsgDataSet->basePath, "num_samples"));

	 continuousDataSets.clear();
	 spikeDataSets.clear();
	 eventDataSets.clear();
	 syncMsgDataSet = nullptr;
 }
 
 void NWBFile::writeData(int datasetID, int channel, int nSamples, const float* data, float bitVolts)
 {
	 if (!continuousDataSets[datasetID])
		 return;

	 if (nSamples > bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	 {
		 std::cerr << "Write buffer overrun, resizing to" << nSamples << std::endl;
		 bufferSize = nSamples;
		 scaledBuffer.malloc(nSamples);
		 intBuffer.malloc(nSamples);
	 }

	 double multFactor = 1 / (float(0x7fff) * bitVolts);
	 FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), data, multFactor, nSamples);
	 AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), intBuffer.getData(), nSamples);

	 CHECK_ERROR(continuousDataSets[datasetID]->baseDataSet->writeDataRow(channel, nSamples, BaseDataType::I16, intBuffer));
	 
	 /* Since channels are filled asynchronouysly by the Record Thread, there is no guarantee
		that at a any point in time all channels in a dataset have the same number of filled samples.
		However, since each dataset is filled from a single source, all channels must have the
		same number of samples at acquisition stop. To keep track of the written samples we must chose
		an arbitrary channel, and at the end all channels will be the same. */

	 if (channel == 0) //there will always be a first channel or there wouldn't be dataset
		 continuousDataSets[datasetID]->numSamples += nSamples;
 }

 void NWBFile::writeTimestamps(int datasetID, int nSamples, const double* data)
 {
	 if (!continuousDataSets[datasetID])
		 return;

	 CHECK_ERROR(continuousDataSets[datasetID]->timestampDataSet->writeDataBlock(nSamples, BaseDataType::F64, data));
 }

 void NWBFile::writeSpike(int electrodeId, const SpikeChannel* channel, const SpikeEvent* event)
 {
	 if (!spikeDataSets[electrodeId])
		 return;
	 int nSamples = channel->getTotalSamples() * channel->getNumChannels();

	 if (nSamples > bufferSize) //Shouldn't happen, and if it happens it'll be slow, but better this than crashing. Will be reset on file close and reset.
	 {
		 std::cerr << "Write buffer overrun, resizing to" << nSamples << std::endl;
		 bufferSize = nSamples;
		 scaledBuffer.malloc(nSamples);
		 intBuffer.malloc(nSamples);
	 }

	 double multFactor = 1 / (float(0x7fff) * channel->getChannelBitVolts(0));
	 FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), event->getDataPointer(), multFactor, nSamples);
	 AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), intBuffer.getData(), nSamples);

	 double timestampSec = event->getTimestamp() / channel->getSampleRate();

	 CHECK_ERROR(spikeDataSets[electrodeId]->baseDataSet->writeDataBlock(1, BaseDataType::I16, intBuffer));
	 CHECK_ERROR(spikeDataSets[electrodeId]->timestampDataSet->writeDataBlock(1, BaseDataType::F64, &timestampSec));
	 writeEventMetaData(spikeDataSets[electrodeId], channel, event);

	 spikeDataSets[electrodeId]->numSamples += 1;

 }

 void NWBFile::writeEvent(int eventID, const EventChannel* channel, const Event* event)
 {
	 if (!eventDataSets[eventID])
		 return;
	 
	 const void* dataSrc;
	 BaseDataType type;
	 int8 ttlVal;
	 String text;

	 switch (event->getEventType())
	 {
	 case EventChannel::TTL:
		 ttlVal = (static_cast<const TTLEvent*>(event)->getState() ? 1 : -1) * (event->getChannel() + 1);
		 dataSrc = &ttlVal;
		 type = BaseDataType::I8;
		 break;
	 case EventChannel::TEXT:
		 text = static_cast<const TextEvent*>(event)->getText();
		 dataSrc = text.toUTF8().getAddress();
		 type = BaseDataType::STR(text.length());
		 break;
	 default:
		 dataSrc = static_cast<const BinaryEvent*>(event)->getBinaryDataPointer();
		 type = getEventH5Type(event->getEventType());
		 break;
	 }
	 CHECK_ERROR(eventDataSets[eventID]->baseDataSet->writeDataBlock(1, type, dataSrc));

	 double timeSec = event->getTimestamp() / channel->getSampleRate();

	 CHECK_ERROR(eventDataSets[eventID]->timestampDataSet->writeDataBlock(1, BaseDataType::F64, &timeSec));

	 uint8 controlValue = event->getChannel() + 1;

	 CHECK_ERROR(eventDataSets[eventID]->controlDataSet->writeDataBlock(1, BaseDataType::U8, &controlValue));

	 if (event->getEventType() == EventChannel::TTL)
	 {
		 CHECK_ERROR(eventDataSets[eventID]->ttlWordDataSet->writeDataBlock(1, BaseDataType::U8, static_cast<const TTLEvent*>(event)->getTTLWordPointer()));
	 }
	 
	 eventDataSets[eventID]->numSamples += 1;
 }

 void NWBFile::writeTimestampSyncText(uint16 sourceID, int64 timestamp, float sourceSampleRate, String text)
 {
	 CHECK_ERROR(syncMsgDataSet->baseDataSet->writeDataBlock(1, BaseDataType::STR(text.length()), text.toUTF8()));
	 double timeSec = timestamp / sourceSampleRate;
	 CHECK_ERROR(syncMsgDataSet->timestampDataSet->writeDataBlock(1, BaseDataType::F64, &timeSec));
	 CHECK_ERROR(syncMsgDataSet->controlDataSet->writeDataBlock(1, BaseDataType::U8, &sourceID));
	 syncMsgDataSet->numSamples += 1;
 }

 
 String NWBFile::getFileName()
 {
	 return filename;
 }

  bool NWBFile::createTimeSeriesBase(String basePath, String source, String helpText, String description, StringArray ancestry)
 {
	 if (createGroup(basePath)) return false;
	 CHECK_ERROR(setAttributeStrArray(ancestry, basePath, "ancestry"));
	 CHECK_ERROR(setAttributeStr(" ", basePath, "comments"));
	 CHECK_ERROR(setAttributeStr(description, basePath, "description"));
	 CHECK_ERROR(setAttributeStr("TimeSeries", basePath, "neurodata_type"));
	 CHECK_ERROR(setAttributeStr(source, basePath, "source"));
	 CHECK_ERROR(setAttributeStr(helpText, basePath, "help"));
	 return true;
 }

  void NWBFile::createDataAttributes(String basePath, float conversion, float resolution, String unit)
  {
		  CHECK_ERROR(setAttribute(BaseDataType::F32, &conversion, basePath + "/data", "conversion"));
		  CHECK_ERROR(setAttribute(BaseDataType::F32, &resolution, basePath + "/data", "resolution"));
		  CHECK_ERROR(setAttributeStr(unit, basePath + "/data", "unit"));
  }

  HDF5RecordingData* NWBFile::createTimestampDataSet(String basePath, int chunk_size)
  {
	  HDF5RecordingData* tsSet = createDataSet(BaseDataType::F64, 0, chunk_size, basePath + "/timestamps");
	  if (!tsSet)
		  std::cerr << "Error creating timestamp dataset in " << basePath << std::endl;
	  else
	  {
		  const int32 one = 1;
		  CHECK_ERROR(setAttribute(BaseDataType::I32, &one, basePath + "/timestamps", "interval"));
		  CHECK_ERROR(setAttributeStr("seconds", basePath + "/timestamps", "unit"));
	  }
	  return tsSet;
  }

  bool NWBFile::createExtraInfo(String basePath, String name, String desc, String id, uint16 index, uint16 typeIndex)
  {
	  if (createGroup(basePath)) return false;
	  CHECK_ERROR(setAttributeStr("openephys:<channel_info>/", basePath, "schema_id"));
	  CHECK_ERROR(setAttributeStr(name, basePath, "name"));
	  CHECK_ERROR(setAttributeStr(desc, basePath, "description"));
	  CHECK_ERROR(setAttributeStr(id, basePath, "identifier"));
	  CHECK_ERROR(setAttribute(BaseDataType::U16, &index, basePath, "source_index"));
	  CHECK_ERROR(setAttribute(BaseDataType::U16, &typeIndex, basePath, "source_type_index"));
	  return true;
  }

  bool NWBFile::createChannelMetaDataSets(String basePath, const MetaDataInfoObject* info)
  {
	  if (!info) return false;
	  if (createGroup(basePath)) return false;
	  CHECK_ERROR(setAttributeStr("openephys:<metadata>/", basePath, "schema_id"));
	  int nMetaData = info->getMetaDataCount();
	  
	  for (int i = 0; i < nMetaData; i++)
	  {
		  const MetaDataDescriptor* desc = info->getMetaDataDescriptor(i);
		  String fieldName = "Field_" + String(i+1);
		  String name = desc->getName();
		  String description = desc->getDescription();
		  String identifier = desc->getIdentifier();
		  BaseDataType type = getMetaDataH5Type(desc->getType(), desc->getLength()); //only string types use length, for others is always set to 1. If array types are implemented, change this
		  int length = desc->getType() == MetaDataDescriptor::CHAR ? 1 : desc->getLength(); //strings are a single element of length set in the type (see above) while other elements are saved a
		  HeapBlock<char> data(desc->getDataSize());
		  info->getMetaDataValue(i)->getValue(static_cast<void*>(data.getData()));
		  createBinaryDataSet(basePath, fieldName, type, length, data.getData());
		  String fullPath = basePath + "/" + fieldName;
		  CHECK_ERROR(setAttributeStr("openephys:<metadata>/", fullPath, "schema_id"));
		  CHECK_ERROR(setAttributeStr(name, fullPath, "name"));
		  CHECK_ERROR(setAttributeStr(description, fullPath, "description"));
		  CHECK_ERROR(setAttributeStr(identifier, fullPath, "identifier"));
	  }
	  return true;
  }

 
  bool NWBFile::createEventMetaDataSets(String basePath, TimeSeries* timeSeries, const MetaDataEventObject* info)
  {
	  if (!info) return false;
	  if (createGroup(basePath)) return false;
	  CHECK_ERROR(setAttributeStr("openephys:<metadata>/", basePath, "schema_id"));
	  int nMetaData = info->getEventMetaDataCount();

	  timeSeries->metaDataSet.clear(); //just in case
	  for (int i = 0; i < nMetaData; i++)
	  {
		  const MetaDataDescriptor* desc = info->getEventMetaDataDescriptor(i);
		  String fieldName = "Field_" + String(i+1);
		  String name = desc->getName();
		  String description = desc->getDescription();
		  String identifier = desc->getIdentifier();
		  BaseDataType type = getMetaDataH5Type(desc->getType(), desc->getLength()); //only string types use length, for others is always set to 1. If array types are implemented, change this
		  int length = desc->getType() == MetaDataDescriptor::CHAR ? 1 : desc->getLength(); //strings are a single element of length set in the type (see above) while other elements are saved as arrays
		  String fullPath = basePath + "/" + fieldName;
		  HDF5RecordingData* dSet = createDataSet(type, 0, length, EVENT_CHUNK_SIZE, fullPath);
		  if (!dSet) return false;
		  timeSeries->metaDataSet.add(dSet);

		  CHECK_ERROR(setAttributeStr("openephys:<metadata>/", fullPath, "schema_id"));
		  CHECK_ERROR(setAttributeStr(name, fullPath, "name"));
		  CHECK_ERROR(setAttributeStr(description, fullPath, "description"));
		  CHECK_ERROR(setAttributeStr(identifier, fullPath, "identifier"));
	  }
      return true;
  }

  void NWBFile::writeEventMetaData(TimeSeries* timeSeries, const MetaDataEventObject* info, const MetaDataEvent* event)
  {
	  jassert(timeSeries->metaDataSet.size() == event->getMetadataValueCount());
	  jassert(info->getEventMetaDataCount() == event->getMetadataValueCount());
	  int nMetaData = event->getMetadataValueCount();
	  for (int i = 0; i < nMetaData; i++)
	  {
		  BaseDataType type = getMetaDataH5Type(info->getEventMetaDataDescriptor(i)->getType(), info->getEventMetaDataDescriptor(i)->getLength());
		  timeSeries->metaDataSet[i]->writeDataBlock(1, type, event->getMetaDataValue(i)->getRawValuePointer());
	  }

  }
 
  void NWBFile::createTextDataSet(String path, String name, String text)
  {
	  ScopedPointer<HDF5RecordingData> dSet;

	  if (text.isEmpty()) text = " "; //to avoid 0-length strings, which cause errors
	  BaseDataType type = BaseDataType::STR(text.length());

	  dSet = createDataSet(type, 1, 0, path + "/" + name);
	  if (!dSet) return;
	  dSet->writeDataBlock(1, type, text.toUTF8());
  }

  void NWBFile::createBinaryDataSet(String path, String name, BaseDataType type, int length, void* data)
  {
	  ScopedPointer<HDF5RecordingData> dSet;
	  if ((length < 1) || !data) return;

	  dSet = createDataSet(type, 1, length, 1, path + "/" + name);
	  if (!dSet) return;
	  dSet->writeDataBlock(1, type, data);
  }


  //These two methods whould be easy to adapt to support array types for all base types, for now
  //length is only used for string types.
  NWBFile::BaseDataType NWBFile::getEventH5Type(EventChannel::EventChannelTypes type, int length)
  {
	  switch (type)
	  {
	  case EventChannel::INT8_ARRAY:
		  return BaseDataType::I8;
	  case EventChannel::UINT8_ARRAY:
		  return BaseDataType::U8;
	  case EventChannel::INT16_ARRAY:
		  return BaseDataType::I16;
	  case EventChannel::UINT16_ARRAY:
		  return BaseDataType::U16;
	  case EventChannel::INT32_ARRAY:
		  return BaseDataType::I32;
	  case EventChannel::UINT32_ARRAY:
		  return BaseDataType::U32;
	  case EventChannel::INT64_ARRAY:
		  return BaseDataType::I64;
	  case EventChannel::UINT64_ARRAY:
		  return BaseDataType::U64;
	  case EventChannel::FLOAT_ARRAY:
		  return BaseDataType::F32;
	  case EventChannel::DOUBLE_ARRAY:
		  return BaseDataType::F64;
	  case EventChannel::TEXT:
		  return BaseDataType::STR(length);
	  default:
		  return BaseDataType::I8;
	  }
  }
  NWBFile::BaseDataType NWBFile::getMetaDataH5Type(MetaDataDescriptor::MetaDataTypes type, int length)
  {
	  switch (type)
	  {
	  case MetaDataDescriptor::INT8:
		  return BaseDataType::I8;
	  case MetaDataDescriptor::UINT8:
		  return BaseDataType::U8;
	  case MetaDataDescriptor::INT16:
		  return BaseDataType::I16;
	  case MetaDataDescriptor::UINT16:
		  return BaseDataType::U16;
	  case MetaDataDescriptor::INT32:
		  return BaseDataType::I32;
	  case MetaDataDescriptor::UINT32:
		  return BaseDataType::U32;
	  case MetaDataDescriptor::INT64:
		  return BaseDataType::I64;
	  case MetaDataDescriptor::UINT64:
		  return BaseDataType::U64;
	  case MetaDataDescriptor::FLOAT:
		  return BaseDataType::F32;
	  case MetaDataDescriptor::DOUBLE:
		  return BaseDataType::F64;
	  case MetaDataDescriptor::CHAR:
		  return BaseDataType::STR(length);
	  default:
		  return BaseDataType::I8;
	  }
  }