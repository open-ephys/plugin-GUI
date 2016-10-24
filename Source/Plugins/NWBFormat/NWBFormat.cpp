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

 NWBFile::NWBFile(String fName, String ver, String idText) : HDF5FileBase(), filename(fName), GUIVersion(ver), spikeMaxSize(0), identifierText(idText)
 {
	 //Init stuff
	 readyToOpen=true; //In KWIK this is in initFile, but the new recordEngine methods make it safe for it to be here
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
	if (createGroup("/acquisition/timeseries/continuous")) return -1;
	if (createGroup("/acquisition/timeseries/spikes")) return -1;
	if (createGroup("/acquisition/timeseries/messages")) return -1;
	if (createGroup("/acquisition/timeseries/events")) return -1;

	if (createGroup("/general/data_collection")) return -1;

	CHECK_ERROR(setAttributeStr(String("OpenEphys GUI v") + GUIVersion, "/general/data_collection", "software"));
	CHECK_ERROR(setAttributeStr(*xmlText, "/general/data_collection", "configuration"));
	
	//TODO: Add default datasets
	//Modify this one once we have JUCE4 to allow UTC time 
	String time = Time::getCurrentTime().formatted("%Y-%m-%dT%H:%M:%S");
	createTextDataSet("", "file_create_date", time);
	createTextDataSet("", "identifier", identifierText);
	createTextDataSet("", "nwb_version", "NWB-1.0.4_beta");
	createTextDataSet("", "session_description", " ");
	createTextDataSet("", "session_start_time", time);
	
	return 0;
}
 
 bool NWBFile::startNewRecording(int recordingNumber, const Array<NWBRecordingInfo>& continuousInfo, const Array<NWBRecordingInfo>& spikeInfo)
 {
	 //Created each time a new recording is started. Creates the specific file structures and attributes
	 //for that specific recording
	 HDF5RecordingData* dSet;
	 String basePath;
	 int nCont = continuousInfo.size();
	 for (int i = 0; i < nCont; i++)
	 {
		 basePath = "/acquisition/timeseries/continuous";
		 basePath = basePath + "/processor" + String(continuousInfo[i].processorId) + "_" + String(continuousInfo[i].sourceId);
		 if (createGroupIfDoesNotExist(basePath)) return false;
		 basePath = basePath + "/recording" + String(recordingNumber);
		 if (createGroup(basePath)) return false;
		 dSet = createRecordingStructures(basePath, continuousInfo[i], "Stores acquired voltage data from extracellular recordings", CHUNK_XSIZE, "ElectricalSeries");
		 continuousDataSetsTS.add(dSet);
		 basePath += "/data";
		 continuousBasePaths.add(basePath);
		 dSet = createDataSet(I16, 0, continuousInfo[i].nChannels, CHUNK_XSIZE, basePath);
		 continuousDataSets.add(dSet); //even if it's null, to not break ordering.
		 if (!dSet)
			 std::cerr << "Error creating data dataset for " << continuousInfo[i].processorId << std::endl;
		 else
		 {
			 float bV = continuousInfo[i].bitVolts;
			 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
			 bV = bV / float(65536);
			 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
			 CHECK_ERROR(setAttributeStr("volt", basePath, "unit"));
		 }
		 continuousInfoStructs.add(continuousInfo[i]);
	 }

	 nCont = spikeInfo.size();
	 for (int i = 0; i < nCont; i++)
	 {
		 basePath = "/acquisition/timeseries/spikes";
		 basePath = basePath + "/electrode" + String(i);
		 if (createGroupIfDoesNotExist(basePath)) return false;
		 basePath = basePath + "/recording" + String(recordingNumber);
		 if (createGroup(basePath)) return false;
		 dSet = createRecordingStructures(basePath, spikeInfo[i], "Snapshorts of spike events from data", SPIKE_CHUNK_XSIZE, "SpikeEventSeries");
		 spikeDataSetsTS.add(dSet);
		 basePath += "/data";
		 spikeBasePaths.add(basePath);
		 dSet = createDataSet(I16, 0, spikeInfo[i].nChannels, spikeInfo[i].nSamplesPerSpike, SPIKE_CHUNK_XSIZE, basePath);
		 spikeDataSets.add(dSet); //even if it's null, to not break ordering.
		 if (!dSet)
			 std::cerr << "Error creating spike dataset for " << spikeInfo[i].processorId << std::endl;
		 else
		 {
			 float bV = spikeInfo[i].bitVolts;
			 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
			 bV = bV / float(65536);
			 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
			 CHECK_ERROR(setAttributeStr("volt", basePath, "unit"));
		 }
		 spikeInfoStructs.add(spikeInfo[i]);
		 numSpikes.add(0);
		 if (spikeMaxSize < (spikeInfo[i].nChannels * spikeInfo[i].nSamplesPerSpike))
		 {
			 spikeMaxSize = (spikeInfo[i].nChannels * spikeInfo[i].nSamplesPerSpike);
			 transformBlock.malloc(spikeMaxSize);
		 }
	 }

	 NWBRecordingInfo singleInfo;
	 singleInfo.bitVolts = NAN;
	 singleInfo.nChannels = 0;
	 singleInfo.nSamplesPerSpike = 0;
	 singleInfo.processorId = 0;
	 singleInfo.sampleRate = 0;
	 singleInfo.sourceId = 0;
	 singleInfo.spikeElectrodeName = " ";
	 singleInfo.sourceName = "All processors";

	 basePath = "/acquisition/timeseries/events";
	 basePath = basePath + "/recording" + String(recordingNumber);
	 if (createGroup(basePath)) return false;
	 dSet = createRecordingStructures(basePath, singleInfo, "Stores the start and stop times for events",EVENT_CHUNK_SIZE, "IntervalSeries");
	 eventsDataSetTS = dSet;
	 eventsControlDataSet = createDataSet(U8, 0, EVENT_CHUNK_SIZE, basePath + "/control");
	 if (!eventsControlDataSet)
		 std::cerr << "Error creating events control dataset" << std::endl;

	 basePath += "/data";
	 eventsBasePath = basePath;
	 dSet = createDataSet(I8, 0, EVENT_CHUNK_SIZE, basePath);
	 eventsDataSet = dSet; //even if it's null, to not break ordering.
	 if (!dSet)
		 std::cerr << "Error creating events dataset " << std::endl;
	 else
	 {
		 float bV = NAN;
		 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
		 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
		 CHECK_ERROR(setAttributeStr("n/a", basePath, "unit"));
	 }
	 numEvents = 0;

	 basePath = "/acquisition/timeseries/messages";
	 basePath = basePath + "/recording" + String(recordingNumber);
	 if (createGroup(basePath)) return false;
	 dSet = createRecordingStructures(basePath, singleInfo, "Time-stamped annotations about an experiment", EVENT_CHUNK_SIZE, "AnnotationSeries");
	 messagesDataSetTS = dSet;
	 basePath += "/data";
	 messagesBasePath = basePath;
	 dSet = createDataSet(STR, 0, EVENT_CHUNK_SIZE, basePath);
	 messagesDataSet = dSet; //even if it's null, to not break ordering.
	 if (!dSet)
		 std::cerr << "Error creating messages dataset " << std::endl;
	 else
	 {
		 float bV = NAN;
		 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
		 CHECK_ERROR(setAttribute(F32, &bV, basePath, "resolution"));
		 CHECK_ERROR(setAttributeStr("n/a", basePath, "unit"));
	 }
	 numMessages = 0;
	 return true;
 }
 
 void NWBFile::stopRecording()
 {
	 uint64 n;
	 int nObjs = continuousDataSets.size();
	 for (int i = 0; i < nObjs; i++)
	 {
		 n = numContinuousSamples[i];
		 CHECK_ERROR(setAttribute(U64, &n, continuousBasePaths[i], "num_samples"));
	 }
	 nObjs = spikeDataSets.size();
	 for (int i = 0; i < nObjs; i++)
	 {
		 n = numSpikes[i];
		 CHECK_ERROR(setAttribute(I32, &n, spikeBasePaths[i], "num_samples"));
	 }
	 CHECK_ERROR(setAttribute(I32, &numEvents, eventsBasePath, "num_samples"));
	 CHECK_ERROR(setAttribute(I32, &numMessages, messagesBasePath, "num_samples"));

	 continuousDataSets.clear();
	 continuousDataSetsTS.clear();
	 continuousBasePaths.clear();
	 numContinuousSamples.clear();

	 spikeDataSets.clear();
	 spikeDataSetsTS.clear();
	 spikeBasePaths.clear();
	 numSpikes.clear();

	 eventsDataSet = nullptr;
	 eventsDataSetTS = nullptr;
	 eventsControlDataSet = nullptr;

	 messagesDataSet = nullptr;
	 messagesDataSetTS = nullptr;

 }
 
 void NWBFile::writeData(int datasetID, int channel, int nSamples, const int16* data)
 {
	 if (!continuousDataSets[datasetID])
		 return;

	 CHECK_ERROR(continuousDataSets[datasetID]->writeDataRow(channel, nSamples, I16, data));
	 
	 /* Since channels are filled asynchronouysly by the Record Thread, there is no guarantee
		that at a any point in time all channels in a dataset have the same number of filled samples.
		However, since each dataset is filled from a single source, all channels must have the
		same number of samples at acquisition stop. To keep track of the written samples we must chose
		an arbitrary channel, and at the end all channels will be the same. */

	 if (channel == 0) //there will always be a first channel or there wouldn't be dataset
		 numContinuousSamples.set(datasetID, numContinuousSamples[datasetID] + nSamples);
 }

 void NWBFile::writeTimestamps(int datasetID, int nSamples, const double* data)
 {
	 if (!continuousDataSetsTS[datasetID])
		 return;

	 CHECK_ERROR(continuousDataSetsTS[datasetID]->writeDataBlock(nSamples, F64, data));
 }

 void NWBFile::writeSpike(int electrodeId, const uint16* data, uint64 timestamp)
 {
	 if (!spikeDataSets[electrodeId])
		 return;
	 int totValues = spikeInfoStructs[electrodeId].nChannels * spikeInfoStructs[electrodeId].nSamplesPerSpike;
	 for (int i = 0; i < totValues; i++)
		 transformBlock[i] = data[i] - 32768;
	 CHECK_ERROR(spikeDataSets[electrodeId]->writeDataBlock(1, I16, transformBlock));
	 double timestampSec = timestamp / spikeInfoStructs[electrodeId].sampleRate;
	 CHECK_ERROR(spikeDataSetsTS[electrodeId]->writeDataBlock(1, U64, &timestampSec));
	 numSpikes.set(electrodeId, numSpikes[electrodeId] + 1);

 }

 void NWBFile::writeTTLEvent(int channel, int id, uint8 source, uint64 timestamp)
 {
	 int8 data = id != 0 ? channel : -channel;
	 CHECK_ERROR(eventsDataSet->writeDataBlock(1, I8, &data));
	 CHECK_ERROR(eventsDataSetTS->writeDataBlock(1, U64, &timestamp));
	 CHECK_ERROR(eventsControlDataSet->writeDataBlock(1, U8, &source));
	 numEvents += 1;
 }

 void NWBFile::writeMessage(const char* msg, uint64 timestamp)
 {
	 CHECK_ERROR(messagesDataSet->writeDataBlock(1, STR, msg));
	 CHECK_ERROR(messagesDataSetTS->writeDataBlock(1, U64, &timestamp));
	 numMessages += 1;
 }
 
 String NWBFile::getFileName()
 {
	 return filename;
 }

  HDF5RecordingData* NWBFile::createRecordingStructures(String basePath, const NWBRecordingInfo& info, String helpText, int chunk_size, String ancestry)
 {
	 StringArray ancestryStrings;
	 ancestryStrings.add("TimeSeries");
	 ancestryStrings.add(ancestry);
	 CHECK_ERROR(setAttributeStrArray(ancestryStrings, basePath, "ancestry"));
	 CHECK_ERROR(setAttributeStr(" ", basePath, "comments"));
	 CHECK_ERROR(setAttributeStr(info.spikeElectrodeName, basePath, "description"));
	 CHECK_ERROR(setAttributeStr("TimeSeries", basePath, "neurodata_type"));
	 CHECK_ERROR(setAttributeStr(info.sourceName, basePath, "source"));
	 CHECK_ERROR(setAttributeStr(helpText, basePath, "help"));
	 HDF5RecordingData* tsSet = createDataSet(HDF5FileBase::F64, 0, chunk_size, basePath + "/timestamps");
	 if (!tsSet)
		 std::cerr << "Error creating timestamp dataset for " << info.processorId << std::endl;
	 else
	 {
		 const int32 one = 1;
		 CHECK_ERROR(setAttribute(HDF5FileBase::I32, &one, basePath + "/timestamps", "interval"));
		 CHECK_ERROR(setAttributeStr("seconds", basePath + "/timestamps", "unit"));
	 }
	 return tsSet;

 }
 
  void NWBFile::createTextDataSet(String path, String name, String text)
  {
	  ScopedPointer<HDF5RecordingData> dSet;

	  if (text.isEmpty()) text = " "; //to avoid 0-length strings, which cause errors

	  dSet = createDataSet(STR, 1, 0, path + "/" + name);
	  if (!dSet) return;
	  dSet->writeDataBlock(1, STR, text.toUTF8());
  }
