/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2018 Open Ephys

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

#ifndef BINARYFILESOURCE_H_INCLUDED
#define BINARYFILESOURCE_H_INCLUDED

#include "../FileSource.h"
#include "../../../Utils/Utils.h"

/** 
	
	Reads data from a directory that conforms to the standards
	of the Open Ephys "Binary Format."

	The files are indexed by a "structure.oebin" file, which 
	is what is loaded into the File Reader.

*/
namespace BinarySource
{
	class BinaryFileSource : public FileSource
	{
	public:

		/** Constructor */
		BinaryFileSource();

		/** Destructor */
		~BinaryFileSource() { }

		/** Attempt to open file and return true if successful */
		bool open(File file) override;

		/** Sets the index of the active recording*/
		void updateActiveRecord(int index) override;

		/** Fills in metadata about the available channels */
		void fillRecordInfo() override;

		/** Seek to a specific sample number within the active recording*/
		void seekTo(int64 sample) override;

		/** Read in nSamples of continuous data into a buffer */
		int readData(int16* buffer, int nSamples) override;

		/** Convert nSamples of data from int16 to float */
		void processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples) override;

		/** Add info about events occurring within a sample range */
		void processEventData(EventInfo &info, int64 startTimestamp, int64 stopTimestamp) override;

		int64 loopCount;

	private:
		
		int numActiveChannels;
		Array<float> bitVolts;

		ScopedPointer<MemoryMappedFile> m_dataFile;
		var m_jsonData;
		Array<File> m_dataFileArray;

		File m_rootPath;
		int64 m_samplePos;

		const unsigned int EVENT_HEADER_SIZE_IN_BYTES = 128;
		const unsigned int BYTES_PER_EVENT = 2;

		bool hasEventData;
		
	};
}

#endif