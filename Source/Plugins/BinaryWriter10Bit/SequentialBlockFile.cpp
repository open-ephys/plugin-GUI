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

#include "SequentialBlockFile.h"
#include <bitset>

using namespace BinaryWriter10Bit::BinaryRecordingEngine;

SequentialBlockFile::SequentialBlockFile(int nChannels, int samplesPerBlock) :
m_file(nullptr),
m_nChannels(nChannels),
m_samplesPerBlock(samplesPerBlock),
m_blockSize(nChannels*samplesPerBlock),
m_lastBlockFill(0)
{
	m_memBlocks.ensureStorageAllocated(blockArrayInitSize);
	for (int i = 0; i < nChannels; i++)
	{
		m_currentBlock.add(-1);
	}
}

SequentialBlockFile::~SequentialBlockFile()
{
	//Ensure that all remaining blocks are flushed in order. Keep the last one
	int n = m_memBlocks.size();
	for (int i = 0; i < n - 1; i++)
	{
		m_memBlocks.remove(0);
	}

	//manually flush the last one to avoid trailing zeroes
	m_memBlocks[0]->partialFlush(m_lastBlockFill * m_nChannels);
}

bool SequentialBlockFile::openFile(String filename)
{
	File file(filename);
	Result res = file.create();
	if (res.failed())
	{
		std::cerr << "Error creating file " << filename << ":" << res.getErrorMessage() << std::endl;
		return false;
	}
	m_file = file.createOutputStream(streamBufferSize);
	if (!m_file)
		return false;

	m_memBlocks.add(new FileBlock(m_file, m_blockSize, 0));
	return true;
}

//namespace {
//	namespace AudioDataConverters10Bit {
//		/** Similar to juce::AudioDataConverter::convertFloatToInt16LE, except bitmasks the 10 most significant
//		*	bits (negative flag + 9 highest digits) and overlaps 10-bit values onto 16-bit ones.
//		*
//		*  Example:
//		*       & Samp 1          & Samp 2          & Samp 3-N
//		*		[00101110 10010110 10010101 01001011 ... ]
//		*		|		 &  	  |
//		*       00000011 11111111 -> (masking an LE 16-bit int)
//		*								|
//		*								v (combine this value with next, and next, ...)
//		*		[10100101 10010100 1011...]
//		*		 &Samp 1    &Samp 2    &Samp 3-N ...
//		*
//		*
//		*	Returns the new number of 16-bit samples packed with overlapping 10-bit values.
//		*/
//		size_t convertFloatTo10BitBEPackedInt16(const float* source, void* dest, int numSamples, int destBytesPerSample = 2)
//		{
//			const double maxVal = (double)0x7fff;
//			char* intData = static_cast<char*> (dest);
//
//			bool first = true;
//			uint8 bitOffset = 0;
//
//			if (dest != (void*)source || destBytesPerSample <= 4)
//			{
//				for (int i = 0; i < numSamples; ++i)
//				{
//					uint16 val = (short)roundToInt(jlimit(-maxVal, maxVal, maxVal * source[i]));
//					if (first)
//					{
//						first = false;
//						DBG("raw uint16 value: " + String(val));
//						DBG("raw uint16 binary: " << std::bitset<8>(*(uint8*)&val).to_string() << std::bitset<8>(*(((uint8*)&val) + 1)).to_string());
//						DBG("raw uint16 bitset binary: " << std::bitset<16>(val).to_string());
//						DBG("raw uint16 value interpreted as signed: " << String(*(int16*)&val));
//						auto longVal = std::bitset<16>(val).to_ulong();
//						DBG("raw uint16 value as ulong bitset: " << std::bitset<8 * sizeof(unsigned long)>(longVal).to_string());
//						DBG("raw uint16 value as ulong" << String(longVal));
//					}
//					*(uint16*)intData = ByteOrder::swapIfBigEndian(val >> 6);
//					intData += destBytesPerSample;
//				}
//			}
//			else
//			{
//				intData += destBytesPerSample * numSamples;
//
//				for (int i = numSamples; --i >= 0;)
//				{
//					intData -= destBytesPerSample;
//					*(uint16*)intData = ByteOrder::swapIfBigEndian((uint16)(short)roundToInt(jlimit(-maxVal, maxVal, maxVal * source[i])));
//				}
//			}
//
//			return 0;
//		}
//	}
//}

namespace {namespace AudioData10BitHelper {

	uint16 endianSafeLeftShift(uint16 val, uint8 bitsToMove)
	{
		return ByteOrder::swapIfBigEndian(ByteOrder::swapIfBigEndian(val) << bitsToMove);
	}

	uint16 endianSafeRightShift(uint16 val, uint8 bitsToMove)
	{
		return ByteOrder::swapIfBigEndian(ByteOrder::swapIfBigEndian(val) >> bitsToMove);
	}
} }

//

bool SequentialBlockFile::writeChannel(uint64 startPos, int channel, int16* data, int nSamples)
{
	if (!m_file)
		return false;

	int bIndex = m_memBlocks.size() - 1;
	if ((bIndex < 0) || (m_memBlocks[bIndex]->getOffset() + m_samplesPerBlock) < (startPos + nSamples))
		allocateBlocks(startPos, nSamples);

	for (bIndex = m_memBlocks.size() - 1; bIndex >= 0; bIndex--)
	{
		if (m_memBlocks[bIndex]->getOffset() <= startPos)
			break;
	}
	if (bIndex < 0)
	{
		std::cerr << "BINARY WRITER: Memory block unloaded ahead of time for chan " << channel << " start " << startPos << " ns " << nSamples << " first " << m_memBlocks[0]->getOffset() << std::endl;
		for (int i = 0; i < m_nChannels; i++)
			std::cout << "channel " << i << " last block " << m_currentBlock[i] << std::endl;
		return false;
	}
	int writtenSamples = 0;
	int startIdx = startPos - m_memBlocks[bIndex]->getOffset();
	int startMemPos = startIdx * m_nChannels;
	int dataIdx = 0;
	int lastBlockIdx = m_memBlocks.size() - 1;
	while (writtenSamples < nSamples)
	{
		int16* blockPtr = m_memBlocks[bIndex]->getData();
		int samplesToWrite = jmin((nSamples - writtenSamples), (m_samplesPerBlock - startIdx));
		for (int i = 0; i < samplesToWrite; i++)
		{
			*(blockPtr + startMemPos + channel + i * m_nChannels) = *(data + dataIdx);
			dataIdx++;
		}
		writtenSamples += samplesToWrite;

		//Update the last block fill index
		size_t samplePos = startIdx + samplesToWrite;
		if (bIndex == lastBlockIdx && samplePos > m_lastBlockFill)
		{
			m_lastBlockFill = samplePos;
		}

		startIdx = 0;
		startMemPos = 0;
		bIndex++;
	}
	m_currentBlock.set(channel, bIndex - 1); //store the last block a channel was written in
	return true;
}

void SequentialBlockFile::allocateBlocks(uint64 startIndex, int numSamples)
{
	//First deallocate full blocks
	//Search for the earliest unused block;
	unsigned int minBlock = 0xFFFFFFFF; //large number;
	for (int i = 0; i < m_nChannels; i++)
	{
		if (m_currentBlock[i] < minBlock)
			minBlock = m_currentBlock[i];
	}

	//Update block indexes
	for (int i = 0; i < m_nChannels; i++)
	{
		m_currentBlock.set(i, m_currentBlock[i] - minBlock);
	}

	m_memBlocks.removeRange(0, minBlock);

	//for (int i = 0; i < minBlock; i++)
	//{
		//Not the most efficient way, as it has to move back all the elements, but it's a simple array of pointers, so it's quick enough
	//	m_memBlocks.remove(0);
	//}
	
	//Look for the last available position and calculate needed space
	uint64 lastOffset = m_memBlocks.getLast()->getOffset();
	uint64 maxAddr = lastOffset + m_samplesPerBlock - 1;
	uint64 newSpaceNeeded = numSamples - (maxAddr - startIndex);
	int newBlocks = (newSpaceNeeded + m_samplesPerBlock - 1) / m_samplesPerBlock; //Fast ceiling division

	for (int i = 0; i < newBlocks; i++)
	{
		lastOffset += m_samplesPerBlock;
		m_memBlocks.add(new FileBlock(m_file, m_blockSize, lastOffset));
	}
	if (newBlocks > 0)
		m_lastBlockFill = 0; //we've added some new blocks, so the last one will be empty
}

