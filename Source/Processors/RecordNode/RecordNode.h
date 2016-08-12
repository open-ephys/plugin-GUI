/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __RECORDNODE_H_FB9B1CA7__
#define __RECORDNODE_H_FB9B1CA7__
#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include <map>
#include <atomic>


#include "../GenericProcessor/GenericProcessor.h"
#include "../Channel/Channel.h"
#include "EventQueue.h"

#define WRITE_BLOCK_LENGTH 1024
#define DATA_BUFFER_NBLOCKS 300
#define EVENT_BUFFER_NEVENTS 512
#define SPIKE_BUFFER_NSPIKES 512

struct SpikeRecordInfo;
struct SpikeObject;
class RecordEngine;
class RecordThread;
class DataQueue;

/**

  Receives inputs from all processors that want to save their data.
  Writes data to disk using fwrite.

  Receives a signal from the ControlPanel to begin recording.

  @see GenericProcessor, ControlPanel

*/

class RecordNode : public GenericProcessor,
    public FilenameComponentListener
{
public:

    RecordNode();
    ~RecordNode();

    /** Handle incoming data and decide which files and events to write to disk.
    */
    void process(AudioSampleBuffer& buffer, MidiBuffer& eventBuffer);


    /** Overrides implementation in GenericProcessor; used to change recording parameters
        on the fly.

        parameterIndex = 0: stop recording
        parameterIndex = 1: start recording
        parameterIndex = 2:
              newValue = 0: turn off recording for current channel
              newValue = 1: turn on recording for current channel
    */
    void setParameter(int parameterIndex, float newValue);

	/** returns current experiment number */
	int getExperimentNumber();
	/** returns current recording number */
	int getRecordingNumber();

    /** Called by the processor graph for each processor that could record data
    */
    void registerProcessor(GenericProcessor* sourceNode);
    /** Called by the processor graph for each recordable channel
    */
    void addInputChannel(GenericProcessor* sourceNode, int chan);

    bool enable();
    bool disable();

    /** returns channel names and whether we record them */
    void getChannelNamesAndRecordingStatus(StringArray& names, Array<bool>& recording);

    /** Get channel stored in channelPointers array
    */
    Channel* getDataChannel(int index);

    /** Called by the ControlPanel to determine the amount of space
        left in the current dataDirectory.
    */
    float getFreeSpace();

    /** Selects a channel relative to a particular processor with ID = id
    */
    void setChannel(Channel* ch);

    /** Turns recording on and off for a particular channel.

        Channel numbers are absolute (based on RecordNode channel mapping).
    */
    void setChannelStatus(Channel* ch, bool status);

    /** Used to clear all connections prior to the start of acquisition.
    */
    void resetConnections();

    /** Callback to indicate when user has chosen a new data directory.
    */
    void filenameComponentChanged(FilenameComponent*);

    /** Creates a new data directory in the location specified by the fileNameComponent.
    */
    void createNewDirectory();


    File getDataDirectory()
    {
        return rootFolder;
    }

    /** Adds a Record Engine to use
    */
    void registerRecordEngine(RecordEngine* engine);

    /** Clears the list of active Record Engines
    */
    void clearRecordEngines();

    /** Must be called by a spike recording source on the "enable" method
    */
    void registerSpikeSource(GenericProcessor* processor);

    /** Registers an electrode group for spike recording
    Must be called by a spike recording source on the "enable" method
    after the call to registerSpikeSource
    */
    int addSpikeElectrode(SpikeRecordInfo* elec);

    /** Called by a spike recording source to write a spike to file
    */
    void writeSpike(SpikeObject& spike, int electrodeIndex);

    SpikeRecordInfo* getSpikeElectrode(int index);

    /** Signals when to create a new data directory when recording starts.*/
    bool newDirectoryNeeded;

    std::atomic<bool> isRecording;

    /** Generate a Matlab-compatible datestring */
    String generateDateString();

	/** Get the last settings.xml in string form. Since the string will be large, returns a const ref.*/
	const String& getLastSettingsXml() const;

private:

    /** Keep the RecordNode informed of acquisition and record states.
    */
    bool isProcessing;

    /** User-selectable directory for saving data files. Currently
        defaults to the user's home directory.
    */
    File dataDirectory;

    /** Automatically generated folder for each recording session.
    */
    File rootFolder;


    /** Integer timestamp saved for each buffer.
    */
    int64 timestamp;

    /** Integer to keep track of number of recording sessions in the same file */
    int recordingNumber;

    /** Used to generate timestamps if none are given.
    */
    Time timer;

    /** Pointers to all continuous channels */
    Array<Channel*> channelPointers;

    /** Pointers to all event channels */
    Array<Channel*> eventChannelPointers;

	Array<int> channelMap;

    OwnedArray<SpikeRecordInfo> spikeElectrodePointers;

    int spikeElectrodeIndex;

    int experimentNumber;
    bool hasRecorded;
    bool settingsNeeded;
	std::atomic<bool> setFirstBlock;
    /** Generates a default directory name, based on the current date and time */
    String generateDirectoryName();

    /** Cycle through the event buffer, looking for data to save */
    void handleEvent(int eventType, MidiMessage& event, int samplePos);

    /**RecordEngines loaded**/
    OwnedArray<RecordEngine> engineArray;

	ScopedPointer<RecordThread> m_recordThread;
	ScopedPointer<DataQueue> m_dataQueue;
	ScopedPointer<EventMsgQueue> m_eventQueue;
	ScopedPointer<SpikeMsgQueue> m_spikeQueue;
	
	Array<int> m_recordedChannelMap;

	String m_lastSettingsText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordNode);

};



#endif  // __RECORDNODE_H_FB9B1CA7__
