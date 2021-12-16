#ifndef BINARYRECORDING_H
#define BINARYRECORDING_H

#include <chrono>
#include <iomanip>
#include <memory>

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
	void writeEvent(int eventIndex, const EventPacket& packet) override;
	void writeSpike(int electrodeIndex, const Spike* spike) override;
	void writeTimestampSyncText(uint64 streamId, int64 timestamp, float sampleRate, String text) override;
	void setParameter(EngineParameter& parameter) override;

	static RecordEngineManager* getEngineManager();

private:

    class EventRecording
    {
    public:

		std::unique_ptr<NpyFile> data;
		std::unique_ptr<NpyFile> samples;
		std::unique_ptr<NpyFile> timestamps;
		std::unique_ptr<NpyFile> channels;
		std::unique_ptr<NpyFile> extra;

    };

    std::unique_ptr<NpyFile> createEventMetadataFile(const MetadataEventObject* channel, String fileName, DynamicObject* jsonObject);
	void createChannelMetadata(const MetadataObject* channel, DynamicObject* jsonObject);
    void writeEventMetadata(const MetadataEvent* event, NpyFile* file);
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
	static String getProcessorString(const InfoObject* channelInfo);
	
	OwnedArray<NpyFile> m_dataTimestampFiles;
	OwnedArray<NpyFile> m_dataFloatTimestampFiles;
	std::unique_ptr<FileOutputStream> m_syncTextFile;

	Array<unsigned int> m_spikeFileIndexes;
    Array<uint16> m_spikeChannelIndexes;

	int m_recordingNum;
	Array<int64> m_startTS;

	const int samplesPerBlock{ 4096 };


};
#endif