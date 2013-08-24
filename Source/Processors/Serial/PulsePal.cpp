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

#include <vector>
#include <stdio.h>
#include <stdint.h>


#include "PulsePal.h"

#define ZERO_uS (uint32_t) 0
#define FIFTY_uS (uint32_t) 50
#define MAX_uS (uint32_t) 3600000000
#define NEWLINE 0xA
#define RETURN 0xD

PulsePal::PulsePal()
{

}

PulsePal::~PulsePal()
{

    serial.close();
}

void PulsePal::initialize()
{

    std::cout << "Searching for Pulse Pal..." << std::endl;


    //
    // lsusb shows Device 104: ID 1eaf:0004
    // updated udev rules file, but still need to run as root -- no idea why
    //
    // try this instead: sudo chmod o+rw /dev/ttyACM0
    //
    // works fine, but you have to re-do it every time
    //

    vector<ofSerialDeviceInfo> devices = serial.getDeviceList();

   // bool foundDevice = false;

	int id = devices[0].getDeviceID();
        string path = devices[0].getDevicePath();
        string name = devices[0].getDeviceName();

	serial.setup(id, 115200); //115200);
	

}



void PulsePal::setPhase1Duration(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 0, timeInMicroseconds);
}

void PulsePal::setInterPhaseInterval(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 1, timeInMicroseconds);
}

void PulsePal::setPhase2Duration(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 2, timeInMicroseconds);
}

void PulsePal::setInterPulseInterval(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 3, timeInMicroseconds);
}

void PulsePal::setBurstDuration(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, ZERO_uS, MAX_uS);
    program(channel, 4, timeInMicroseconds);
}

void PulsePal::setBurstInterval(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, ZERO_uS, MAX_uS);
    program(channel, 5, timeInMicroseconds);
}

void PulsePal::setStimulusTrainDuration(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 6, timeInMicroseconds);
}

void PulsePal::setStimulusTrainDelay(uint8_t channel, uint32_t timeInMicroseconds)
{
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 7, timeInMicroseconds);
}

void PulsePal::setBiphasic(uint8_t channel, bool isBiphasic)
{
    uint8_t command = 0;

    if (isBiphasic)
    {
        command = 1;
    }

    program(channel, 8, command);
}

void PulsePal::setPhase1Voltage(uint8_t channel, float voltage)
{
    program(channel, 9, voltageToByte(voltage));
}

void PulsePal::setPhase2Voltage(uint8_t channel, float voltage)
{
    program(channel, 10, voltageToByte(voltage));
}

void PulsePal::updateDisplay(string line1, string line2)
{
    uint8_t message1 = 85;

    serial.writeByte(message1);

    serial.writeBytes((unsigned char*) line1.data(), line1.size());
    //serial.writeByte(0);
    //serial.writeByte(RETURN);
    serial.writeByte(254);
    serial.writeBytes((unsigned char*) line2.data(), line2.size());
    //serial.writeByte(0);
    //serial.writeByte(RETURN);

}

void PulsePal::program(uint8_t channel, uint8_t paramCode, uint32_t paramValue)
{
    std::cout << "sending 32-bit message" << std::endl;

    uint8_t message1[3] = {79, paramCode, channel};

    uint8_t message2[4];

    // make sure byte order is little-endian:
    message2[0] = (paramValue & 0xff);
    message2[1] = (paramValue & 0xff00) >> 8;
    message2[2] = (paramValue & 0xff0000) >> 16;
    message2[3] = (paramValue & 0xff00000) >> 24;

    serial.writeBytes(message1, 3);
    serial.writeBytes(message2, 4);

    std::cout << "Message 1: " << (int) message1[0] << " " << (int) message1[1] << " " << (int) message1[2] << std::endl;
    std::cout << "Message 2: " << (int) message2[0] << " " << (int) message2[1] << " " << (int) message2[2] <<  " " << (int) message2[3] << std::endl;
}


void PulsePal::program(uint8_t channel, uint8_t paramCode, uint8_t paramValue)
{

    std::cout << "sending 8-bit message" << std::endl;

    uint8_t message1[3] = {79, paramCode, channel};

    serial.writeBytes(message1, 3);
    serial.writeBytes(&paramValue, 1);

    std::cout << "Message 1: " << (int) message1[0] << " " << (int) message1[1] << " " << (int) message1[2] << std::endl;
    std::cout << "Message 2: " << paramValue << std::endl;
}

void PulsePal::constrain(uint32_t* value, uint32_t min, uint32_t max)
{

    // value must be a multiple of 50
    if (*value % 50 > 0)
    {
        *value = *value - (*value % 50);
    }

    if (*value < min)
    {
        *value = min;
    }

    if (*value > max)
    {
        *value = max;
    }

}

uint8_t PulsePal::voltageToByte(float voltage)
{
    // input: -10 to 10 V
    // output: 0-255

    uint8_t output = (uint8_t)((voltage+10)/20)*255;

    return output;

}


void PulsePal::triggerChannel(uint8_t chan)
{
    const uint8_t code = 1 << (chan-1);

    uint8_t bytesToWrite[2] = {84, code};

    serial.writeBytes(bytesToWrite, 2);
}


