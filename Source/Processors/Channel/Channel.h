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

#ifndef __CHANNEL_H_DABDFE3F__
#define __CHANNEL_H_DABDFE3F__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../GenericProcessor/GenericProcessor.h"

#include <stdio.h>

class GenericProcessor;
class ChannelExtraData;

/**

  Holds metadata about a given channel within a processor.

  The Channel class provides a convenient way to store settings
  for individual channels, and to pass that information between
  processors. It's especially handy for the interactions with the
  AudioNode and RecordNode, which need to access/update Channel
  information for multiple processors at once.

  Every processor has a Channel object for each channel it must deal with.
  Usually, settings for each channel will be copied from a source
  node to its destination, but sometimes particular parameters will
  be updated.

  @see GenericProcessor, RecordNode, AudioNode

*/


class Channel

{
public:

    //--------- CONSTRUCTOR / DESTRUCTOR --------//

    /** Default constructor for creating Channels from scratch. */
    Channel(GenericProcessor*, int, ChannelType);

    /** Copy constructor. */
    Channel(const Channel& ch);

    //--------- GET / SET METHODS --------//

    /** Returns the name of a given channel. */
    String getName();

    /** Sets the name of a given channel. */
    void setName(String);

    /** Sets the type of a given channel. */
    void setType(ChannelType t);

    /** Sets the type of a given channel. */
    ChannelType getType();

    /** Sets the processor to which a channel belongs. */
    void setProcessor(GenericProcessor*);

    /** Sets whether or not the channel will record. */
    void setRecordState(bool t); // {isRecording = t;}

    /** Sets whether or not the channel will record. */
	bool getRecordState();

    /** Sets the bitVolts value for this channel. */
    void setBitVolts(float bitVolts);

    /** Returns the bitVolts value for this channel. */
    float getBitVolts();

    // -------- OTHER METHODS ---------//

    /** Restores the default settings for a given channel. */
    void reset();

    //--------------PUBLIC MEMBERS ---------------- //

    /** Channel index within the source processor */
    int nodeIndex;

    /** Channel index within the source processor relative to channel type */
    int index;

	/** Channel index which can be altered by the processor, useful with remappers*/
	int mappedIndex;

    /** The ID of the channel's processor.*/
    int nodeId;

    /** Pointer to the channel's parent processor. */
    GenericProcessor* processor;

    /** Sample rate expected for this channel. */
    float sampleRate;

    /** Bit volts for this channel (i.e., by what must we multiply the ADC integer value to
        convert to the original voltage measurement?). */
    float bitVolts;

    /** Channel "type": neural data, aux, adc, event **/
    ChannelType type;

    /** ID of source node. This is crucial for properly updating timestamps and sample counts. */
    int sourceNodeId;

    /** Toggled when audio monitoring of this channel is enabled or disabled. */
    bool isMonitored;

    /** Toggled when a channel is disabled from further processing. */
    bool isEnabled;

    /** File info (for disk writing). Meaning depends on the RecordEngine being used. */
    int recordIndex;

    /** Holds the name of this channel */
    String name;

    /** The ID of the probe that this channel belongs to (if any) */
    int probeId;

    /** x,y,z location of this channel in space */
    float x,y,z;

    /** Impedance of this channel. */
    float impedance;

	/** For use with special event channels. */
	ReferenceCountedObjectPtr<ChannelExtraData> extraData;

private:

    //-------------- PRIVATE MEMBERS ---------------- //

    /** Stores whether or not the channel is being recorded. */
    bool isRecording;

    /** Generates a default name, based on the channel number. */
    void createDefaultName();

};

class ChannelExtraData : public ReferenceCountedObject
{
public:
	ChannelExtraData(void* data, int size);
	virtual ~ChannelExtraData();
	void* const dataPtr;
	const int dataSize;
};

#endif  // __CHANNEL_H_DABDFE3F__
