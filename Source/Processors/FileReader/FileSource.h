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

#ifndef FILESOURCE_H_INCLUDED
#define FILESOURCE_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"


struct RecordedChannelInfo
{
    String name;
    float bitVolts;
};


class PLUGIN_API FileSource
{
public:
    FileSource();
    virtual ~FileSource();

    int getNumRecords()     const;
    int getActiveRecord()   const;

    String getRecordName (int index) const;

    float getRecordSampleRate   (int index) const;
    int getRecordNumChannels    (int index) const;
    int64 getRecordNumSamples   (int index) const;

    float getActiveSampleRate() const;
    int getActiveNumChannels()  const;
    int64 getActiveNumSamples() const;

    RecordedChannelInfo getChannelInfo (int recordIndex, int channel) const;
    RecordedChannelInfo getChannelInfo (int channel) const;

    void setActiveRecord (int index);

    bool OpenFile (File file);
    bool isFileOpened()  const;
    String getFileName() const;

    virtual int readData (int16* buffer, int nSamples) = 0;
    virtual void processChannelData (int16* inBuffer, float* outBuffer, int channel, int64 numSamples) = 0;
    virtual void seekTo (int64 sample) = 0;

    virtual bool isReady();

protected:
    struct RecordInfo
    {
        String name;
        Array<RecordedChannelInfo> channels;
        int64 numSamples;
        float sampleRate;
    };
    Array<RecordInfo> infoArray;

    bool fileOpened;
    int numRecords;
    Atomic<int> activeRecord;       // atomic to protect against threaded data race in FileReader
    String filename;


private:
    virtual bool Open (File file) = 0;
    virtual void fillRecordInfo() = 0;
    virtual void updateActiveRecord() = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSource);
};


#endif  // FILESOURCE_H_INCLUDED
