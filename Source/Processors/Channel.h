/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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


#ifdef WIN32
#include <Windows.h>
#endif

#include "../../JuceLibraryCode/JuceHeader.h"

#include "GenericProcessor.h"

#include <stdio.h>

class GenericProcessor;

/**
  
  Holds metadata about a given channel within a processor.

  The Channel class provides a convenient way to store settings
  for individual channels, and to pass that information between
  processors. It's especially handy for the interactions with the
  AudioNode and RecordNode, which need to access/update Channel
  information for multiple processors at once.

  @see GenericProcessor, RecordNode, AudioNode

*/

class Channel

{
public:

	/** Default constructor for creating Channels from scratch. */
	Channel(GenericProcessor* p, int n);

	/** Copy constructor. */
	Channel(const Channel& ch);

	/** Returns the name of a given channel. */
	String getName();

	/** Restores the default settings for a given channel. */
	void reset();

	/** Sets the processor to which a channel belongs. */
	void setProcessor(GenericProcessor*);

	/** The channel number.*/
	int num;

	/** The ID of the channel's processor.*/
	int nodeId;

	/** Used for EventChannels only.*/
	int eventType;

	// boolean values:
	bool isEventChannel;
	bool isRecording;
	bool isMonitored;
	bool isEnabled;

	/** Pointer to the channel's parent processor. */
	GenericProcessor* processor;

	// crucial information:
	float sampleRate;
	float bitVolts;

	// file info (for disk writing):
	String filename;
	FILE* file;

	String name;

private:

	/** Generates a default name, based on the channel number. */
	void createDefaultName();

};

#endif  // __CHANNEL_H_DABDFE3F__
