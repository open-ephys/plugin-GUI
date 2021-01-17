#ifndef SEQUENTIALBLOCKFILE_H
#define SEQUENTIALBLOCKFILE_H

#include "FileMemoryBlock.h"
#include "../../../Utils/Utils.h"

typedef FileMemoryBlock<int16> FileBlock;

class SequentialBlockFile
{
public:
	SequentialBlockFile(int nChannels, int samplesPerBlock);
	~SequentialBlockFile();

	bool openFile(String filename);
	bool writeChannel(uint64 startPos, int channel, int16* data, int nSamples);

private:
	ScopedPointer<FileOutputStream> m_file;
	const int m_nChannels;
	const int m_samplesPerBlock;
	const int m_blockSize;
	OwnedArray<FileBlock> m_memBlocks;
	Array<int> m_currentBlock;
	size_t m_lastBlockFill;

	void allocateBlocks(uint64 startIndex, int numSamples);

	//Compile-time params
	const int streamBufferSize{ 0 };
	const int blockArrayInitSize{ 128 };

};
#endif // !SEQUENTIALBLOCKFILE_H
