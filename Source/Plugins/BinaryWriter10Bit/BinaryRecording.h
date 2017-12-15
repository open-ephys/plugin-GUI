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
#ifndef BINARYRECORDING_H
#define BINARYRECORDING_H

#include <RecordingLib.h>
#include "SequentialBlockFile.h"
#include "NpyFile.h"

namespace BinaryWriter10Bit
{
	namespace BinaryRecordingEngine
	{

		class BinaryRecording : public RecordEngine
		{
		public:
			BinaryRecording();
			~BinaryRecording();

			String getEngineID() const override;
			void openFiles(File rootFolder, int experimentNumber, int recordingNumber) override;
			void closeFiles() override;
			void writeData(int writeChannel, int realChannel, const float* buffer, int size) override;
			void writeEvent(int eventIndex, const MidiMessage& event) override;
			void resetChannels() override;
			void addSpikeElectrode(int index, const SpikeChannel* elec) override;
			void writeSpike(int electrodeIndex, const SpikeEvent* spike) override;
			void writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float, String text) override;
			void setParameter(EngineParameter& parameter) override;

			static RecordEngineManager* getEngineManager();

		private:

			class EventRecording
			{
			public:
				ScopedPointer<NpyFile> mainFile;
				ScopedPointer<NpyFile> timestampFile;
				ScopedPointer<NpyFile> metaDataFile;
				ScopedPointer<NpyFile> channelFile;
				ScopedPointer<NpyFile> extraFile;
			};


			NpyFile* createEventMetadataFile(const MetaDataEventObject* channel, String fileName, DynamicObject* jsonObject);
			void createChannelMetaData(const MetaDataInfoObject* channel, DynamicObject* jsonObject);
			void writeEventMetaData(const MetaDataEvent* event, NpyFile* file);
			void increaseEventCounts(EventRecording* rec);
			static String jsonTypeValue(BaseType type);
			static String getProcessorString(const InfoObjectCommon* channelInfo);

			bool m_saveTTLWords{ true };

			HeapBlock<float> m_scaledBuffer;
			HeapBlock<int16> m_intBuffer;
			HeapBlock<int64> m_tsBuffer;
			int m_bufferSize;

			OwnedArray<SequentialBlockFile>  m_DataFiles;
			Array<unsigned int> m_channelIndexes;
			Array<unsigned int> m_fileIndexes;
			OwnedArray<EventRecording> m_eventFiles;
			OwnedArray<EventRecording> m_spikeFiles;
			OwnedArray<NpyFile> m_dataTimestampFiles;
			ScopedPointer<FileOutputStream> m_syncTextFile;

			Array<unsigned int> m_spikeFileIndexes;
			Array<uint16> m_spikeChannelIndexes;

			int m_recordingNum;
			Array<int64> m_startTS;


			//Compile-time constants
			//const int samplesPerBlock{ 4096 };
			int samplesPerBlock{ 4096 };

		};
        
    };
    
    namespace AudioDataConverters10Bit
    {
        inline uint32 convertFloatToPackedInt10LE(const float *source, void* dest, int numSamples)
        {
            static const double maxVal = (double)0x7fff;
            char* intData = static_cast<char*> (dest);
            static const uint16 bitMask = 0xFFC0;
            uint8 bitOffset = 0;
            
            uint32 bytesWritten = 0;
            
            for (int i = 0; i < numSamples; ++i)
            {
                const uint16 val = ((uint16)(short)roundToInt(jlimit(-maxVal, maxVal, maxVal * source[i]))) & bitMask;
                if (bitOffset == 0)
                {
                    *(uint16*)intData = ByteOrder::swapIfBigEndian(val);
                    ++bytesWritten;
                }
                else
                {
                    *(uint16*)intData += ByteOrder::swapIfBigEndian(val >> bitOffset);
                    intData += 2;
                    
                    if (bitOffset > 4) {
                        *(uint16*)(intData) = ByteOrder::swapIfBigEndian(val << (16 - bitOffset));
                        ++bytesWritten;
                    }
                }
                if ((bitOffset += 10) >= 16) bitOffset %= 16;
            }
            
            return bytesWritten;
        }
    };

}

#endif
