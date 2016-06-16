/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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

#ifndef __CONTCIRCBUF_H
#define __CONTCIRCBUF_H
#include "../../../JuceLibraryCode/JuceHeader.h"
#include <vector>

class ContinuousCircularBuffer
{
public:
	ContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer);
	void reallocate(int N);
	void update(std::vector<std::vector<bool>> contdata, int64 hardware_ts, int64 software_ts, int numpts);
	void update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts);
	void update(int channel, int64 hardware_ts, int64 software_ts, bool rise);
	int GetPtr();
	void addTrialStartToSmartBuffer(int trialID);
	int numCh;
	int subSampling;
	float samplingRate;
	CriticalSection mut;
	int numSamplesInBuf;
	double numTicksPerSecond;
	int ptr;
	int bufLen;
	int leftover_k;
	double buffer_dx;

	std::vector<std::vector<float> > Buf;
	std::vector<bool> valid;
	std::vector<int64> hardwareTS, softwareTS;
};

#endif