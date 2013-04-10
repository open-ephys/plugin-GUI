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

#ifndef __PULSEPAL_H_F2B7B63E__
#define __PULSEPAL_H_F2B7B63E__

#include <string.h>

#include "ofSerial.h"

/**

  Interfaces with the PulsePal from Lucid Biosystems

  (www.lucidbiosystems.com)

  @see PulsePalOutput

*/

class PulsePal
{
public:
	PulsePal();
	~PulsePal();

	void initialize();

	void setPhase1Duration(uint8_t channel, uint32_t timeInMicroseconds);
	void setInterPhaseInterval(uint8_t channel, uint32_t timeInMicroseconds);
	void setPhase2Duration(uint8_t channel, uint32_t timeInMicroseconds);
	void setInterPulseInterval(uint8_t channel, uint32_t timeInMicroseconds);
	void setBurstDuration(uint8_t channel, uint32_t timeInMicroseconds);
	void setBurstInterval(uint8_t channel, uint32_t timeInMicroseconds);
	void setStimulusTrainDuration(uint8_t channel, uint32_t timeInMicroseconds);
	void setStimulusTrainDelay(uint8_t channel, uint32_t timeInMicroseconds);
	void setBiphasic(uint8_t channel, bool isBiphasic);
	void setPhase1Voltage(uint8_t channel, float voltage);
	void setPhase2Voltage(uint8_t channel, float voltage);

	void triggerChannel(uint8_t channel);

	void updateDisplay(string line1, string line2);

private:

	void constrain(uint32_t* value, uint32_t min, uint32_t max);

	void program(uint8_t channel, uint8_t paramCode, uint32_t paramValue);
	void program(uint8_t channel, uint8_t paramCode, uint8_t paramValue);

	uint8_t voltageToByte(float voltage);

	ofSerial serial;

};



#endif  // __PULSEPAL_H_F2B7B63E__
