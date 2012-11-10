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


/**
  
  Holds metadata about a given channel within a processor.

  @see GenericProcessor, RecordNode, AudioNode

*/

class Channel

{
public:

	// Default constructor:
	Channel(GenericProcessor* p, int n);

	// Copy constructor:
	Channel(const Channel& ch);

	String getName();

	void reset();

	// channel number:
	int num;

	// boolean values:
	bool isEventChannel;
	bool isRecording;
	bool isMonitored;
	bool isEnabled;

	// pointer to parent processor:
	GenericProcessor* processor;
	
	// crucial information:
	float sampleRate;
	float bitVolts;

	// file info (for disk writing):
	String filename;
	FILE* file;


private:

	String name;

	void createDefaultName();

};

#endif  // __CHANNEL_H_DABDFE3F__
