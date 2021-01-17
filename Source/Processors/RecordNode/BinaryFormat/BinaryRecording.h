#ifndef BINARYRECORDING_H
#define BINARYRECORDING_H

#include <chrono>
#include <iomanip>

#include "../../../Utils/Utils.h"
#include "../RecordEngine.h"
#include "SequentialBlockFile.h"
#include "NpyFile.h"

class BinaryRecording : public RecordEngine
{
public:
	BinaryRecording();
	~BinaryRecording();

	String getEngineID() const override;

	void openFiles(File rootFolder, int experimentNumber, int recordingNumber) override;
	void closeFiles() override;
	void resetChannels() override;
	void writeData(int writeChannel, int realChannel, const float* buffer, int size) override;
	void writeSynchronizedData(int writeChannel, int realChannel, const float* dataBuffer, const double* ftsBuffer, int size) override;
	void writeEvent(int eventIndex, const MidiMessage& event) override;
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

    bool m_saveTTLWords{ true };

	HeapBlock<float> m_scaledBuffer;
	HeapBlock<int16> m_intBuffer;
	HeapBlock<int64> m_tsBuffer;
	int m_bufferSize;
	int m_ftsBufferSize;

	OwnedArray<SequentialBlockFile> m_DataFiles;
	OwnedArray<SequentialBlockFile> m_FTSDataFiles;
	Array<unsigned int> m_channelIndexes;
	Array<unsigned int> m_fileIndexes;
	OwnedArray<EventRecording> m_eventFiles;
	OwnedArray<EventRecording> m_spikeFiles;

	static String jsonTypeValue(BaseType type);
	static String getProcessorString(const InfoObjectCommon* channelInfo);
	
	OwnedArray<NpyFile> m_dataTimestampFiles;
	OwnedArray<NpyFile> m_dataFloatTimestampFiles;
	ScopedPointer<FileOutputStream> m_syncTextFile;

	Array<unsigned int> m_spikeFileIndexes;
    Array<uint16> m_spikeChannelIndexes;

	int m_recordingNum;
	Array<int64> m_startTS;

	const int samplesPerBlock{ 4096 };


};
#endif