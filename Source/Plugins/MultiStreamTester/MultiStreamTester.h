/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2018 Open Ephys

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

#ifndef MULTISTREAMTESTER_H_INCLUDED
#define MULTISTREAMTESTER_H_INCLUDED

#include <DataThreadHeaders.h>
#include <array>

class MultiStreamTester : public DataThread
{
public:
	MultiStreamTester(SourceNode* sn);
	~MultiStreamTester();
	bool updateBuffer() override;
	bool foundInputSource() override;
	bool startAcquisition() override;
	bool stopAcquisition() override;
	int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const override;
	int getNumTTLOutputs(int subProcessorIdx) const override;
	float getSampleRate(int subProcessorIdx) const override;
	float getBitVolts(const DataChannel* chan) const override;

	unsigned int getNumSubProcessors() const override;

private:
	struct WaveInfo
	{
		WaveInfo(float f, float s, int n);
		WaveInfo(){};
		float frequency;
		float sampleRate;
		int numChannels;
	};
	Array<WaveInfo> m_waves;
	Array<int64> m_lastSample;
	int64 m_lastTime;
	uint64 m_fakeEvent;
	int64 m_ticksPerSecond;
	Array<float> m_factors;
	std::array<float, 512> m_tmp;
};


#endif