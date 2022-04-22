/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#include "../../../../JuceLibraryCode/JuceHeader.h"

template <class StorageType = int16>
class FileMemoryBlock
{
public:
	FileMemoryBlock(std::shared_ptr<FileOutputStream> file, int blockSize, uint64 offset) :
		m_data(blockSize, true),
		m_file(file),
		m_blockSize(blockSize),
		m_offset(offset),
        m_finalFlushSamples(blockSize)
	{};

	~FileMemoryBlock() {
		if (~m_flushed)
		{
			m_file->write(m_data, m_finalFlushSamples*sizeof(StorageType));
		}
	};

	inline uint64 getOffset() { return m_offset; }
	inline StorageType* getData() { return m_data.getData(); }
	void partialFlush(size_t size)
	{
        m_finalFlushSamples = size;
	}

private:
	HeapBlock<StorageType> m_data;
	std::shared_ptr<FileOutputStream> m_file;
	const int m_blockSize;
	const uint64 m_offset;
    size_t m_finalFlushSamples;
	bool m_flushed{ false };
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileMemoryBlock);
};
