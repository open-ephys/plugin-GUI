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
#include "../../Utils/Utils.h"

struct RecordedChannelInfo
{
    String name;
    float bitVolts;
};

struct EventInfo
{
    std::vector<int16> channels;
    std::vector<int16> channelStates;
    std::vector<int64> timestamps;
    std::vector<String> text;
};

/** 

    Represents a class capable of reading data from a file
    in a particular format.

    FileSource plugins must be derived from this class.
    
*/
class PLUGIN_API FileSource
{
public:

    /** Constructor */
    FileSource();

    /** Destructor */
    virtual ~FileSource();

    // ------------------------------------------------------------
    //                  PURE VIRTUAL METHODS 
    //        (must be implemented by all File Sources)
    // ------------------------------------------------------------
    
    /** Attempt to open the file, and return true if successful */
    virtual bool open(File file) = 0;

    /** Fill the infoArray and eventInfoArray with the relevant information for all recordings*/
    virtual void fillRecordInfo() = 0;

    /** Update the recording to be read in */
    virtual void updateActiveRecord(int index) = 0;

    /** Seek to a specific sample number within the active recording */
    virtual void seekTo(int64 sample) = 0;

    /** Read in nSamples of int16 data into a temporary buffer; return the number of samples actually read */
    virtual int readData(int16* buffer, int nSamples) = 0;

    /** Convert nSamples of data from int16 to float */
    virtual void processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 nSamples) = 0;

    /** Add info about events occurring within a sample range */
    virtual void processEventData(EventInfo& info, int64 startTimestamp, int64 stopTimestamp) = 0;
    
    // ------------------------------------------------------------
    //                   VIRTUAL METHOD
    //       (can optionally be overriden by sub-classes)
    // ------------------------------------------------------------

    /** Return false if file is not able to be opened */
    virtual bool isReady();

    // ------------------------------------------------------------
    //                    OTHER METHODS
    //                (used by File Reader)
    // ------------------------------------------------------------

    /** Returns the number of available recordings in the file */
    int getNumRecords()     const;

    /** Returns the index of the recording that is currently being read in*/
    int getActiveRecord()   const;

    /** Returns the name of a recording, by index. */
    String getRecordName (int index) const;

    /** Returns the sample rate of a recording, by index */
    float getRecordSampleRate   (int index) const;

    /** Returns the number of channels in a recording, by index */
    int getRecordNumChannels    (int index) const;

    /** Returns the number of samples in a recording, by index */
    int64 getRecordNumSamples   (int index) const;

    /** Returns the sample rate of the recording that's currently being read in*/
    float getActiveSampleRate() const;

    /** Returns the number of channels of the recording that's currently being read in */
    int getActiveNumChannels()  const;

    /** Returns the number of samples in the recording that's currently being read in*/
    int64 getActiveNumSamples() const;

    /** Returns channel info for one channel within a recording */
    RecordedChannelInfo getChannelInfo (int recordIndex, int channel) const;

    /** Sets the recording to be read in */
    void setActiveRecord (int index);

    /** Opens the file -- calls open() */
    bool openFile (File file);

    /** Returns true if file is currently open */
    bool isFileOpened()  const;

    /** Returns the full name of the file */
    String getFileName() const;

    /** Get the event information for the current stream */
    const EventInfo& getEventInfo();

    /** Keep track of how many times the recording has looped */
    int64 loopCount;

protected:

    /** Holds the name of the current stream */
    String currentStream;

    /** Holds information about continuous channels in a recording */
    struct RecordInfo
    {
        String name;
        Array<RecordedChannelInfo> channels;
        int64 numSamples;
        float sampleRate;
        int64 startTimestamp;
    };
    Array<RecordInfo> infoArray;

    /** Holds information about event channels in a recording */
    std::map<String, EventInfo> eventInfoMap;

    bool fileOpened;
    int numRecords;
    Atomic<int> activeRecord; // atomic to protect against threaded data race in FileReader
    String filename;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSource);
};


#endif  // FILESOURCE_H_INCLUDED
