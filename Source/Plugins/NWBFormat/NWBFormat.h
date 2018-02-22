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

#ifndef NWBFORMAT_H
#define NWBFORMAT_H

#include <OpenEphysHDF5Lib/HDF5FileFormat.h>
#include <RecordingLib.h>
using namespace OpenEphysHDF5;

namespace NWBRecording
{
	typedef Array<const DataChannel*> ContinuousGroup;
	class TimeSeries
	{
	public:
		ScopedPointer<HDF5RecordingData> baseDataSet;
		ScopedPointer<HDF5RecordingData> timestampDataSet;
		ScopedPointer<HDF5RecordingData> controlDataSet; //for all but spikes
		ScopedPointer<HDF5RecordingData> ttlWordDataSet; //just for ttl events
		OwnedArray<HDF5RecordingData> metaDataSet;
		String basePath;
		uint64 numSamples{ 0 };
	};

	class NWBFile : public HDF5FileBase
	{
	public:
		NWBFile(String fName, String ver, String idText); //with whatever arguments it's necessary
		~NWBFile();
		bool startNewRecording(int recordingNumber, const Array<ContinuousGroup>& continuousArray,
			const Array<const EventChannel*>& eventArray, const Array<const SpikeChannel*>& electrodeArray);
		void stopRecording();
		void writeData(int datasetID, int channel, int nSamples, const float* data, float bitVolts);
		void writeTimestamps(int datasetID, int nSamples, const double* data);
		void writeSpike(int electrodeId, const SpikeChannel* channel, const SpikeEvent* event);
		void writeEvent(int eventID, const EventChannel* channel, const Event* event);
		void writeTimestampSyncText(uint16 sourceID, int64 timestamp, float sourceSampleRate, String text);
		String getFileName() override;
		void setXmlText(const String& xmlText);

	protected:
		int createFileStructure() override;

	private:

		void createTextDataSet(String path, String name, String text);
		void createBinaryDataSet(String path, String name, HDF5FileBase::BaseDataType type, int length, void* data);
		static HDF5FileBase::BaseDataType getEventH5Type(EventChannel::EventChannelTypes type, int length = 1);
		static HDF5FileBase::BaseDataType getMetaDataH5Type(MetaDataDescriptor::MetaDataTypes type, int length = 1);

		bool createTimeSeriesBase(String basePath, String source, String helpText, String description, StringArray ancestry);
		bool createExtraInfo(String basePath, String name, String desc, String id, uint16 index, uint16 typeIndex);
		HDF5RecordingData* createTimestampDataSet(String basePath, int chunk_size);
		void createDataAttributes(String basePath, float conversion, float resolution, String unit);
		bool createChannelMetaDataSets(String basePath, const MetaDataInfoObject* info);
		bool createEventMetaDataSets(String basePath, TimeSeries* timeSeries, const MetaDataEventObject* info);

		void writeEventMetaData(TimeSeries* timeSeries, const MetaDataEventObject* info, const MetaDataEvent* event);
		

		const String filename;
		const String GUIVersion;

		OwnedArray<TimeSeries>  continuousDataSets;
		OwnedArray<TimeSeries> spikeDataSets;
		OwnedArray<TimeSeries> eventDataSets;
		ScopedPointer<TimeSeries> syncMsgDataSet;

		const String* xmlText;
		const String identifierText;

		HeapBlock<float> scaledBuffer;
		HeapBlock<int16> intBuffer;
		size_t bufferSize;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NWBFile);

	};

}

#endif
