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

#ifndef __DATABUFFER_H_11C6C591__
#define __DATABUFFER_H_11C6C591__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"

/**

	Manages reading and writing data to a circular buffer.

*/

class DataBuffer
{
	
public:
	DataBuffer(int chans, int size);
	~DataBuffer();
	void clear();
	void addToBuffer(float* data, uint64* ts, int16* eventCodes, int numItems);
	int getNumSamples();
	int readAllFromBuffer(AudioSampleBuffer& data, uint64* ts, int16* eventCodes, int maxSize);

private:
	AbstractFifo abstractFifo;
	AudioSampleBuffer buffer;

    uint64* timestampBuffer;
    int16* eventCodeBuffer;

	int numChans;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBuffer);

};


#endif  // __DATABUFFER_H_11C6C591__
