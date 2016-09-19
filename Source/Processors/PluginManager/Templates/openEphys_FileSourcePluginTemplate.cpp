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

PROCESSORHEADERS

#include <CoreServicesHeader.h>


PROCESSORCLASSNAME::PROCESSORCLASSNAME()
{
}


PROCESSORCLASSNAME::~PROCESSORCLASSNAME()
{
}


bool PROCESSORCLASSNAME::Open (File file)
{
    return true;
}


void PROCESSORCLASSNAME::fillRecordInfo()
{
}


void PROCESSORCLASSNAME::updateActiveRecord()
{
}


void PROCESSORCLASSNAME::seekTo (int64 sample)
{
}


int PROCESSORCLASSNAME::readData (int16* buffer, int nSamples)
{
    return 0;
}


void PROCESSORCLASSNAME::processChannelData (int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{
}


bool PROCESSORCLASSNAME::isReady()
{
}
