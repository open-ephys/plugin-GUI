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

#include "FileSource.h"


FileSource::FileSource() 
    : fileOpened    (false)
    , numRecords    (0)
    , activeRecord  (-1)
    , filename      ("")
{
}


FileSource::~FileSource()
{
}


int FileSource::getNumRecords() const
{
    return numRecords;
}


String FileSource::getRecordName (int index) const
{
    return infoArray[index].name;
}


int FileSource::getRecordNumChannels (int index) const
{
    return infoArray[index].channels.size();
}


int FileSource::getActiveNumChannels() const
{
    return getRecordNumChannels (activeRecord.get());
}


float FileSource::getRecordSampleRate (int index) const
{
    return infoArray[index].sampleRate;
}


float FileSource::getActiveSampleRate() const
{
    return getRecordSampleRate (activeRecord.get());
}


int64 FileSource::getRecordNumSamples (int index) const
{
    return infoArray[index].numSamples;
}


int64 FileSource::getActiveNumSamples() const
{
    return getRecordNumSamples (activeRecord.get());
}


int FileSource::getActiveRecord() const
{
    return activeRecord.get();
}


RecordedChannelInfo FileSource::getChannelInfo (int recordIndex, int channel) const
{
    return infoArray[recordIndex].channels[channel];
}


RecordedChannelInfo FileSource::getChannelInfo (int channel) const
{
    return getChannelInfo (activeRecord.get(), channel);
}


void FileSource::setActiveRecord (int index)
{
//    activeRecord = index;
    activeRecord.set(index);
    updateActiveRecord();
}


bool FileSource::isFileOpened() const
{
    return fileOpened;
}


String FileSource::getFileName() const
{
    return filename;
}


bool FileSource::OpenFile (File file)
{
    if (Open (file))
    {
        fileOpened = true;
        fillRecordInfo();

        filename = file.getFullPathName();
    }
    else
    {
        fileOpened = false;
        filename = String::empty;
    }

    return fileOpened;
}

bool FileSource::isReady()
{
    return true;
}
