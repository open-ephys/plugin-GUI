/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "../../Utils/Utils.h"
#include "../PluginManager/OpenEphysPlugin.h"

struct RecordedChannelInfo
{
    String name;
    float bitVolts;
    uint8 type;
};

struct EventInfo
{
    std::vector<int16> channels;
    std::vector<int16> channelStates;
    std::vector<int64> sampleNumbers;
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
    virtual bool open (File file) = 0;

    /** Fill the infoArray and eventInfoArray with the relevant information for all recordings*/
    virtual void fillRecordInfo() = 0;

    /** Update the recording to be read in */
    virtual void updateActiveRecord (int index) = 0;

    /** Seek to a specific sample number within the active recording */
    virtual void seekTo (int64 sampleNumber) = 0;

    /** Read in nSamples of float data into a temporary buffer; 
    return the number of samples actually read 
    
    Buffer is structured as sample1_channel1, sample1_channel2, ... sample2_channel1, sample2_channel2, ...

    */
    virtual int readData (float* buffer, int nSamples) = 0;

    /** Add info about events occurring within a sample range */
    virtual void processEventData (EventInfo& info, int64 fromSampleNumber, int64 toSampleNumber) = 0;

    // ------------------------------------------------------------
    //                   VIRTUAL METHODS
    //       (can optionally be overridden by sub-classes)
    // ------------------------------------------------------------

    /** Return false if file is not able to be opened */
    virtual bool isReady();

    // ------------------------------------------------------------
    //                    OTHER METHODS
    //                (used by File Reader)
    // ------------------------------------------------------------

    /** Returns the number of available recorded streams in the recording */
    int getNumRecords() const;

    /** Returns the index of the recorded stream that is currently being read in*/
    int getActiveRecord() const;

    /** Returns the name of a recorded stream, by index. */
    String getRecordName (int index) const;

    /** Returns the sample rate of a recorded stream, by index */
    float getRecordSampleRate (int index) const;

    /** Returns the number of channels in a recorded stream, by index */
    int getRecordNumChannels (int index) const;

    /** Returns the number of samples per channel in a recorded stream, by index */
    int64 getRecordNumSamples (int index) const;

    /** Returns the sample rate of the recorded stream that's currently being read in*/
    float getActiveSampleRate() const;

    /** Returns the number of channels of the recorded stream that's currently being read in */
    int getActiveNumChannels() const;

    /** Returns the number of samples per channel of the recorded stream that's currently being read in*/
    int64 getActiveNumSamples() const;

    /** Returns channel info for one channel within a recorded stream */
    RecordedChannelInfo getChannelInfo (int recordIndex, int channel) const;

    /** Sets the recorded stream to be read in */
    void setActiveRecord (int index);

    /** Opens the file -- calls open() */
    bool openFile (File file);

    /** Returns true if file is currently open */
    bool isFileOpened() const;

    /** Returns the full name of the file */
    String getFileName() const;

    /** Get the event information for the current stream */
    const EventInfo& getEventInfo();

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
        int64 startSampleNumber;
    };
    Array<RecordInfo> infoArray;

    /** Holds information about event channels in a recording */
    std::map<String, EventInfo> eventInfoMap;

    bool fileOpened = false;
    int numRecords = 0;
    Atomic<int> activeRecord = -1; // atomic to protect against threaded data race in FileReader
    String filename = "";

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSource);
};

#endif // FILESOURCE_H_INCLUDED
