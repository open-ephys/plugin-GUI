/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen

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

#ifndef RECORDENGINE_H_INCLUDED
#define RECORDENGINE_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Channel/Channel.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Visualization/SpikeObject.h"

struct SpikeRecordInfo
{
    String name;
    int numChannels;
    int sampleRate;

    int recordIndex;
};

class RecordNode;

class RecordEngine : public AccessClass
{
public:
    RecordEngine();
    ~RecordEngine();
	/** All the public methods are called by RecordNode:
	When acquisition starts (in the specified order):
		1-resetChannels
		2-registerProcessor, addChannel, registerSpikeSource, addspikeelectrode
		3-startAcquisition
	When recording starts (in the specified order):
		1-directoryChanged (if needed)
		2-openFiles
	During recording:
		writeData, writeEvent, writeSpike, updateTimeStamp
	When recording stops:
		closeFiles
	*/

	/** Called when recording starts to open all needed files
	*/
    virtual void openFiles(File rootFolder, int experimentNumber, int recordingNumber) = 0;
	
	/** Called when recording stops to close all files
		and do all the necessary cleanups
	*/
    virtual void closeFiles() = 0;

	/** Write continuous data.
		This method gets the full data buffer, it must query getRecordState for
		each registered channel to determine which channels to actually write to disk
	*/
    virtual void writeData(AudioSampleBuffer& buffer, int nSamples) = 0;

	/** Write a single event to disk.
	*/
    virtual void writeEvent(int eventType, MidiMessage& event, int samplePosition) = 0;
	
	/** Called when acquisition starts once for each processor that might record continuous data
	*/
	virtual void registerProcessor(GenericProcessor* processor);

	/** Called after registerProcessor, once for each output
		channel of the processor
	*/
	virtual void addChannel(int index, Channel* chan) = 0;

	/** Called when acquisition starts once for each processor that might record spikes
	*/
	virtual void registerSpikeSource(GenericProcessor* processor);

	/** Called after registerSpikesource, once for each channel group
	*/
    virtual void addSpikeElectrode(int index, SpikeRecordInfo* elec) = 0;

	/** Write a spike to disk
	*/
    virtual void writeSpike(const SpikeObject& spike, int electrodeIndex) = 0;

	/** Called when a new acquisition starts, to clean all channel data 
		before registering the processors
	*/
    virtual void resetChannels();

	/** Called every time a new timestamp event is received
	*/
    virtual void updateTimeStamp(int64 timestamp);

	/** Called after all channels and spike groups have been registered, 
		just before acquisition starts
	*/
    virtual void startAcquisition();

	/** Called when the recording directory changes during an acquisition
	*/
    virtual void directoryChanged();

protected:
	/** Functions to access RecordNode arrays and utilities
	*/

	/** Gets the specified channel from the channel array stored in RecordNode
	*/
    Channel* getChannel(int index);

	/** Gets the specified channel group info structure from the array stored in RecordNode
	*/
    SpikeRecordInfo* getSpikeElectrode(int index);

	/** Generate a Matlab-compatible datestring 
	*/
    String generateDateString();

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordEngine);
};



#endif  // RECORDENGINE_H_INCLUDED
