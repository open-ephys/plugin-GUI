#include "../../../../JuceLibraryCode/JuceHeader.h"

template <class StorageType = int16>
class FileMemoryBlock
{
public:
	FileMemoryBlock(FileOutputStream* file, int blockSize, uint64 offset) :
		m_data(blockSize, true),
		m_file(file),
		m_blockSize(blockSize),
		m_offset(offset)
	{};

	~FileMemoryBlock() {
		if (~m_flushed)
		{
			m_file->write(m_data, m_blockSize*sizeof(StorageType));
		}
	};

	inline uint64 getOffset() { return m_offset; }
	inline StorageType* getData() { return m_data.getData(); }
	void partialFlush(size_t size, bool markFlushed = true)
	{
		//std::cout << "[RN] flushing last block " << size << std::endl;
		m_file->write(m_data, size*sizeof(StorageType));
		if (markFlushed)
			m_flushed = true;
	}

private:
	HeapBlock<StorageType> m_data;
	FileOutputStream* const m_file;
	const int m_blockSize;
	const uint64 m_offset;
	bool m_flushed{ false };
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileMemoryBlock);
};