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
// Modified by JS 1/30/2014: Updated op codes for firmware 0_4, added new functions (indicated in comments below)

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include "PulsePal.h"

#define ZERO_uS (uint32_t) 0
#define FIFTY_uS (uint32_t) 50
#define MAX_uS (uint32_t) 3600000000
#define NEWLINE 0xA
#define RETURN 0xD
#define makeLong(msb, byte2, byte3, lsb) ((msb << 24) | (byte2 << 16) | (byte3 << 8) | (lsb)) //JS  2/1/2014

PulsePal::PulsePal()
{
    setDefaultParameters();

}

PulsePal::~PulsePal()
{
    disconnectClient();
    serial.close();
}

void PulsePal::setDefaultParameters()
{

    for (int i = 1; i < 5; i++)
    {
        currentOutputParams[i].isBiphasic = 0;
        currentOutputParams[i].phase1Voltage = 5;
        currentOutputParams[i].phase2Voltage = -5;
        currentOutputParams[i].phase1Duration = 0.001;
        currentOutputParams[i].interPhaseInterval = 0.001;
        currentOutputParams[i].phase2Duration = 0.001;
        currentOutputParams[i].interPulseInterval = 0.01;
        currentOutputParams[i].burstDuration = 0;
        currentOutputParams[i].interBurstInterval = 0;
        currentOutputParams[i].pulseTrainDuration = 1;
        currentOutputParams[i].pulseTrainDelay = 0;
        currentOutputParams[i].linkTriggerChannel1 = 1;
        currentOutputParams[i].linkTriggerChannel2 = 0;
        currentOutputParams[i].customTrainID = 0;
        currentOutputParams[i].customTrainTarget = 0;
        currentOutputParams[i].customTrainLoop = 0;
    }
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
    if (devices.size() == 0)
    {
        std::cout << "No serial devices found!" << std::endl;
        return;
    }

    int id = devices[0].getDeviceID();
    string path = devices[0].getDevicePath();
    string name = devices[0].getDeviceName();

    serial.setup(id, 115200);
    std::cout << "Found!" << std::endl;
}

uint32_t PulsePal::getFirmwareVersion() // JS 1/30/2014
{
    uint32_t firmwareVersion = 0;
    uint8_t handshakeByte = 72;
    uint8_t responseBytes[5] = { 0 };
    serial.writeByte(handshakeByte);
#if defined( TARGET_OSX ) || defined( TARGET_LINUX )
    usleep(100000);
#else
    Sleep(100);
#endif

    serial.readBytes(responseBytes,5);
    firmwareVersion = makeLong(responseBytes[4], responseBytes[3], responseBytes[2], responseBytes[1]);
    return firmwareVersion;
}

void PulsePal::setBiphasic(uint8_t channel, bool isBiphasic)
{
    uint8_t command = 0;

    if (isBiphasic)
    {
        command = 1;
    }

    program(channel, 1, command);
    PulsePal::currentOutputParams[channel].isBiphasic = command; //JS  2/1/2014 (Added this for all single-item programming functions)
}

void PulsePal::setPhase1Voltage(uint8_t channel, float voltage)
{
    program(channel, 2, voltageToByte(voltage));
    PulsePal::currentOutputParams[channel].phase1Voltage = voltage;
}

void PulsePal::setPhase2Voltage(uint8_t channel, float voltage)
{
    program(channel, 3, voltageToByte(voltage));
    PulsePal::currentOutputParams[channel].phase2Voltage = voltage;
}

void PulsePal::setPhase1Duration(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000); //JS  2/1/2014
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 4, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].phase1Duration = timeInSeconds;
}

void PulsePal::setInterPhaseInterval(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 5, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].interPhaseInterval = timeInSeconds;
}

void PulsePal::setPhase2Duration(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 6, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].phase2Duration = timeInSeconds;
}

void PulsePal::setInterPulseInterval(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 7, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].interPhaseInterval = timeInSeconds;
}

void PulsePal::setBurstDuration(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, ZERO_uS, MAX_uS);
    program(channel, 8, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].burstDuration = timeInSeconds;
}

void PulsePal::setBurstInterval(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, ZERO_uS, MAX_uS);
    program(channel, 9, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].interBurstInterval = timeInSeconds;
}

void PulsePal::setPulseTrainDuration(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 10, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].pulseTrainDuration = timeInSeconds;
}

void PulsePal::setPulseTrainDelay(uint8_t channel, float timeInSeconds)
{
    uint32_t timeInMicroseconds = (uint32_t)(timeInSeconds * 1000000);
    constrain(&timeInMicroseconds, FIFTY_uS, MAX_uS);
    program(channel, 11, timeInMicroseconds);
    PulsePal::currentOutputParams[channel].pulseTrainDelay = timeInSeconds;
}

void PulsePal::setTrigger1Link(uint8_t channel, uint8_t link_state) // JS 1/30/2014
{
    program(channel, 12, link_state);
    PulsePal::currentOutputParams[channel].linkTriggerChannel1 = link_state;
}
void PulsePal::setTrigger2Link(uint8_t channel, uint8_t link_state) // JS 1/30/2014
{
    program(channel, 13, link_state);
    PulsePal::currentOutputParams[channel].linkTriggerChannel2 = link_state;
}
void PulsePal::setCustomTrainID(uint8_t channel, uint8_t ID) // JS 1/30/2014
{
    program(channel, 14, ID);
    PulsePal::currentOutputParams[channel].customTrainID = ID;
}
void PulsePal::setCustomTrainTarget(uint8_t channel, uint8_t target) // JS 1/30/2014
{
    program(channel, 15, target);
    PulsePal::currentOutputParams[channel].customTrainTarget = target;
}
void PulsePal::setCustomTrainLoop(uint8_t channel, uint8_t loop_state) // JS 1/30/2014
{
    program(channel, 16, loop_state);
    PulsePal::currentOutputParams[channel].customTrainLoop = loop_state;
}

void PulsePal::setTriggerMode(uint8_t channel, uint8_t mode) // JS 1/30/2014
{
    program(channel, 128, mode);
    PulsePal::currentInputParams[channel].triggerMode = mode;
}


void PulsePal::program(uint8_t channel, uint8_t paramCode, uint32_t paramValue)
{
    //std::cout << "sending 32-bit message" << std::endl;

    uint8_t message1[3] = {74, paramCode, channel};

    uint8_t message2[4];

    // make sure byte order is little-endian:
    message2[0] = (paramValue & 0xff);
    message2[1] = (paramValue & 0xff00) >> 8;
    message2[2] = (paramValue & 0xff0000) >> 16;
    message2[3] = (paramValue & 0xff00000) >> 24;

    serial.writeBytes(message1, 3);
    serial.writeBytes(message2, 4);

    //std::cout << "Message 1: " << (int) message1[0] << " " << (int) message1[1] << " " << (int) message1[2] << std::endl;
    //std::cout << "Message 2: " << (int) message2[0] << " " << (int) message2[1] << " " << (int) message2[2] <<  " " << (int) message2[3] << std::endl;
}


void PulsePal::program(uint8_t channel, uint8_t paramCode, uint8_t paramValue)
{

    //std::cout << "sending 8-bit message" << std::endl;

    uint8_t message1[3] = {74, paramCode, channel};

    serial.writeBytes(message1, 3);
    serial.writeBytes(&paramValue, 1);

    //std::cout << "Message 1: " << (int) message1[0] << " " << (int) message1[1] << " " << (int) message1[2] << std::endl;
    //std::cout << "Message 2: " << paramValue << std::endl;
}



void PulsePal::triggerChannel(uint8_t chan)
{
    const uint8_t code = 1 << (chan - 1);

    uint8_t bytesToWrite[2] = {77, code};

    serial.writeBytes(bytesToWrite, 2);
}

void PulsePal::triggerChannels(uint8_t channel1, uint8_t channel2, uint8_t channel3, uint8_t channel4) // JS 1/30/2014
{
    uint8_t code = 0;
    code = code + 1 * channel1;
    code = code + 2 * channel2;
    code = code + 4 * channel3;
    code = code + 8 * channel4;

    uint8_t bytesToWrite[2] = { 77, code };

    serial.writeBytes(bytesToWrite, 2);
}

void PulsePal::updateDisplay(string line1, string line2)
{
    string Prefix;
    string Message;
    Message.append(line1);
    Message += 254;
    Message.append(line2);
    Prefix += 78;
    Prefix += Message.size();
    Prefix.append(Message);
    serial.writeBytes((unsigned char*)Prefix.data(), Prefix.size());
}

void PulsePal::setFixedVoltage(uint8_t channel, float voltage) // JS 1/30/2014
{
    uint8_t voltageByte = 0;
    voltageByte = voltageToByte(voltage);
    uint8_t message1[3] = { 79, channel, voltageByte };
    serial.writeBytes(message1, 3);
}

void PulsePal::abortPulseTrains() // JS 1/30/2014
{
    uint8_t message1 = 80;
    serial.writeByte(message1);
}

void PulsePal::disconnectClient() // JS 1/30/2014
{
    uint8_t message1 = 81;
    serial.writeByte(message1);
}

void PulsePal::setContinuousLoop(uint8_t channel, uint8_t state) // JS 1/30/2014
{
    uint8_t message1[3] = {82, channel, state};
    serial.writeBytes(message1, 3);
}


void PulsePal::constrain(uint32_t* value, uint32_t min, uint32_t max)
{

    // value must be a multiple of 100
    if (*value % 100 > 0)
    {
        *value = *value - (*value % 100);
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

    uint8_t output = uint8_t(((voltage+10)/20)*255);

    return output;

}

void PulsePal::programCustomTrain(uint8_t ID, uint8_t nPulses, float customPulseTimes[], float customVoltages[])
{
    int nMessageBytes = (nPulses * 5) + 6;
    // Convert voltages to bytes
    uint8_t voltageBytes[1000] = { 0 };
    float thisVoltage = 0;
    for (int i = 0; i < nPulses; i++)
    {
        thisVoltage = customVoltages[i];
        voltageBytes[i] = voltageToByte(thisVoltage);
    }
    // Convert times to bytes
    uint8_t pulseTimeBytes[4000] = { 0 };
    int pos = 0;
    unsigned long pulseTimeMicroseconds;
    for (int i = 0; i < nPulses; i++)
    {
        pulseTimeMicroseconds = (unsigned long)(customPulseTimes[i] * 1000000);
        pulseTimeBytes[pos] = (uint8_t)(pulseTimeMicroseconds);
        pos++;
        pulseTimeBytes[pos] = (uint8_t)(pulseTimeMicroseconds >> 8);
        pos++;
        pulseTimeBytes[pos] = (uint8_t)(pulseTimeMicroseconds >> 16);
        pos++;
        pulseTimeBytes[pos] = (uint8_t)(pulseTimeMicroseconds >> 24);
        pos++;
    }
    uint8_t* messageBytes = new uint8_t[nMessageBytes];
    if (ID == 2)
    {
        messageBytes[0] = 76; // Op code to program custom train 2
    }
    else
    {
        messageBytes[0] = 75; // Op code to program custom train 1
    }
    messageBytes[1] = 0; // USB packet correction byte
    messageBytes[2] = (uint8_t)(nPulses);
    messageBytes[3] = (uint8_t)(nPulses >> 8);
    messageBytes[4] = (uint8_t)(nPulses >> 16);
    messageBytes[5] = (uint8_t)(nPulses >> 24);
    int timeDataEnd = 6 + (nPulses * 4);
    for (int i = 6; i < timeDataEnd; i++)
    {
        messageBytes[i] = pulseTimeBytes[i - 6];
    }
    for (int i = timeDataEnd; i < nMessageBytes; i++)
    {
        messageBytes[i] = voltageBytes[i - timeDataEnd];
    }
    serial.writeBytes(messageBytes, nMessageBytes);
}

void PulsePal::programAllParams()
{
    uint8_t messageBytes[163] = { 0 };
    messageBytes[0] = 73;
    int pos = 1;
    uint32_t thisTime = 0;
    float thisVoltage = 0;
    uint8_t thisVoltageByte = 0;

    // add time params
    for (int i = 1; i < 5; i++)
    {
        thisTime = (uint32_t)(currentOutputParams[i].phase1Duration * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].interPhaseInterval * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].phase2Duration * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].interPulseInterval * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].burstDuration * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].interBurstInterval * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].pulseTrainDuration * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
        thisTime = (uint32_t)(currentOutputParams[i].pulseTrainDelay * 1000000);
        messageBytes[pos] = (uint8_t)(thisTime);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 8);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 16);
        pos++;
        messageBytes[pos] = (uint8_t)(thisTime >> 24);
        pos++;
    }

    // add single-byte params
    for (int i = 1; i < 5; i++)
    {
        messageBytes[pos] = (uint8_t)currentOutputParams[i].isBiphasic;
        pos++;
        thisVoltage = PulsePal::currentOutputParams[i].phase1Voltage;
        thisVoltageByte = voltageToByte(thisVoltage);
        messageBytes[pos] = thisVoltageByte;
        pos++;
        thisVoltage = PulsePal::currentOutputParams[i].phase2Voltage;
        thisVoltageByte = voltageToByte(thisVoltage);
        messageBytes[pos] = thisVoltageByte;
        pos++;
        messageBytes[pos] = (uint8_t)currentOutputParams[i].customTrainID;
        pos++;
        messageBytes[pos] = (uint8_t)currentOutputParams[i].customTrainTarget;
        pos++;
        messageBytes[pos] = (uint8_t)currentOutputParams[i].customTrainLoop;
        pos++;
    }

    // add trigger channel 1 links
    for (int i = 1; i < 5; i++)
    {
        messageBytes[pos] = (uint8_t)currentOutputParams[i].linkTriggerChannel1;
        pos++;
    }
    // add trigger channel 2 links
    for (int i = 1; i < 5; i++)
    {
        messageBytes[pos] = (uint8_t)currentOutputParams[i].linkTriggerChannel2;
        pos++;
    }

    // add trigger channel modes
    messageBytes[pos] = (uint8_t)currentInputParams[1].triggerMode;
    pos++;
    messageBytes[pos] = (uint8_t)currentInputParams[2].triggerMode;
    pos++;


    serial.writeBytes(messageBytes, 163);
}