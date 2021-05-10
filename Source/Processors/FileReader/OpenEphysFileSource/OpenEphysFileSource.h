/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2021 Open Ephys

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

#ifndef OPENEPHYSFILESOURCE_H_INCLUDED
#define OPENEPHYSFILESOURCE_H_INCLUDED

#include "../FileSource.h"
#include "../../../Utils/Utils.h"

namespace OpenEphysSource
{

	class OpenEphysFileSource : public FileSource
	{
	public:
		OpenEphysFileSource();
		~OpenEphysFileSource();

		int readData(int16* buffer, int nSamples) override;

		void seekTo(int64 sample) override;

		void processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples) override;

		bool isReady() override;

	private:
		bool Open(File file) override;
		void fillRecordInfo() override;
		void updateActiveRecord() override;

		void readSamples(int16* buffer, int64 samplesToRead);

		struct ChannelInfo
        {
			int id;
            String name;
            double bitVolts;
			String filename;
            long int startPos;
        };

		struct ProcInfo
		{
			int id;
			float sampleRate;
			std::vector<ChannelInfo> channels;
		};

		struct Recording 
		{
			int id;
			int sampleRate;
			std::map<int, ProcInfo> processors;
		};

		OwnedArray<MemoryMappedFile> dataFiles;

        std::map<int, Recording> recordings;

		File m_rootPath;
		int64 m_samplePos;

		int64 blockIdx;
		int64 samplesLeftInBlock;
		
	};
}

#endif