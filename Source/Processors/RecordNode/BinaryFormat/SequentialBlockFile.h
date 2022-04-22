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

#ifndef SEQUENTIALBLOCKFILE_H
#define SEQUENTIALBLOCKFILE_H

#include "FileMemoryBlock.h"
#include "../../../Utils/Utils.h"

#include "../../PluginManager/PluginClass.h"

typedef FileMemoryBlock<int16> FileBlock;

/**
 
    Writes data to a flat binary file of int16s
 
    Often referred to as "dat" or "bin" format, these files contain data
 samples in the following order:
 
    1. CH1 Sample 1
    2. CH2 Sample 1
    3. CH3 Sample 1
    ...
    N. CHN Sample 1
    N + 1. CH1 Sample 2
    N + 2. CH2 Sample 2
    ...
    
 */

class PLUGIN_API SequentialBlockFile
{
public:
    
    /** Creates a file with nChannels */
	SequentialBlockFile(int nChannels, int samplesPerBlock = 4096);
    
    /** Destructor */
	~SequentialBlockFile();

    /** Opens the file at the requested path */
	bool openFile(String filename);
    
    /** Writes nSamples of data for a particular channel */
	bool writeChannel(uint64 startPos, int channel, int16* data, int nSamples);

private:
	std::shared_ptr<FileOutputStream> m_file;
	const int m_nChannels;
	const int m_samplesPerBlock;
	const int m_blockSize;
	OwnedArray<FileBlock> m_memBlocks;
	Array<int> m_currentBlock;
	size_t m_lastBlockFill;

    /** Allocates data for a startIndex / numSamples combination */
	void allocateBlocks(uint64 startIndex, int numSamples);

	/** Compile-time params */
	const int streamBufferSize{ 0 };
	const int blockArrayInitSize{ 128 };

};
#endif // !SEQUENTIALBLOCKFILE_H
