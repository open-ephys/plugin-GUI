/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef HEADERGUARD
#define HEADERGUARD

#include <FileSourceHeaders.h>


class PROCESSORCLASSNAME : public FileSource
{
public:
    PROCESSORCLASSNAME();
    ~PROCESSORCLASSNAME();

    int readData (int16* buffer, int nSamples) override;

    void seekTo (int64 sample) override;

    void processChannelData (int16* inBuffer, float* outBuffer, int channel, int64 numSamples) override;

    bool isReady() override;


private:
    bool Open (File file) override;
    void fillRecordInfo() override;
    void updateActiveRecord() override;
};


#endif // HEADERGUARD
