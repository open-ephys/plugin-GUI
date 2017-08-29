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
 
 #ifndef NWBRECORDING_H
 #define NWBRECORDING_H
 
 #include <RecordingLib.h>
 #include "NWBFormat.h"
 
 namespace NWBRecording {
		class NWBRecordEngine : public RecordEngine
		{
		public:
			NWBRecordEngine();
			~NWBRecordEngine();
			//Those are only the basic set of calls. Look at RecordEngine.cpp and RecordEngine.h for description on all possible hooks and the order they're called.
			String getEngineID() const override;
			void openFiles(File rootFolder, int experimentNumber, int recordingNumber) override;
			void closeFiles() override;
			void writeData(int writeChannel, int realChannel, const float* buffer, int size) override;
			void writeEvent(int eventIndex, const MidiMessage& event) override;
			void addSpikeElectrode(int index,const  SpikeChannel* elec) override;
			void writeSpike(int electrodeIndex, const SpikeEvent* spike) override;
			void writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float sourceSampleRate, String text) override;
			void resetChannels() override;
			void setParameter(EngineParameter& parameter) override;
			
			static RecordEngineManager* getEngineManager();
			
		private:
			ScopedPointer<NWBFile> recordFile;
			Array<int> datasetIndexes;
			Array<int> writeChannelIndexes;

			Array<ContinuousGroup> continuousChannels;
			Array<const EventChannel*> eventChannels;
			Array<const SpikeChannel*> spikeChannels;

			HeapBlock<double> tsBuffer;
			size_t bufferSize;

			String identifierText;
			
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NWBRecordEngine);

			
		}; 
 }
 
 #endif