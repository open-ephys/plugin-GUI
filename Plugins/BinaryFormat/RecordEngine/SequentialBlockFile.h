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

#ifndef SEQUENTIALBLOCKFILE_H
#define SEQUENTIALBLOCKFILE_H

#include "FileMemoryBlock.h"

namespace BinaryRecordingEngine
{

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


        //Compile-time parameters
        const int streamBufferSize{ 0 };
        const int blockArrayInitSize{ 128 };

    };

}

#endif
