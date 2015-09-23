/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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


#include <stdio.h>
#include "SerialInput.h"

const int SerialInput::BAUDRATES[12] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400};

SerialInput::SerialInput()
    : GenericProcessor("Serial Port"), baudrate(0)
{
}

SerialInput::~SerialInput()
{
    serial.close();
}

StringArray SerialInput::getDevices()
{
    vector<ofSerialDeviceInfo> allDeviceInfos = serial.getDeviceList();

    StringArray allDevices;

    for (int i = 0; i < allDeviceInfos.size(); i++)
    {
        allDevices.add(allDeviceInfos[i].getDeviceName());
    }

    return allDevices;
}

Array<int> SerialInput::getBaudrates()
{
    Array<int> allBaudrates(BAUDRATES, 12);
    return allBaudrates;
}

void SerialInput::setDevice(string device)
{
    this->device = device;
}

void SerialInput::setBaudrate(int baudrate)
{
    this->baudrate = baudrate;
}


bool SerialInput::isReady()
{
    if (device == "" || baudrate == 0)
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput connection error!", "Please set device and baudrate to use first!");
        return false;
    }
    if (!serial.setup(device, baudrate))
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput connection error!", "Could not connect to specified serial device. Check log files for details.");
        return false;
    }
    return true;
}

bool SerialInput::disable()
{
    serial.close();
    return true;
}


void SerialInput::process(AudioSampleBuffer&, MidiBuffer& events)
{
    int bytesAvailable = serial.available();

    if (bytesAvailable == OF_SERIAL_ERROR)
    {
        // ToDo: Properly warn about problem here!
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput device access error!", "Could not access serial device.");
        return;
    }

    if (bytesAvailable > 0)
    {

        unsigned char buffer[10000];
        int bytesRead = serial.readBytes(buffer, bytesAvailable);

        if (bytesRead > 0)
        {
            addEvent(events,    // MidiBuffer
                     BINARY_MSG,    // eventType
                     0,         // sampleNum
                     nodeId,    // eventID
                     0,         // eventChannel
                     bytesRead, // numBytes
                     buffer);   // data
        }
        else if (bytesRead < 0)
        {
            // ToDo: Properly warn about problem here!
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput device read error!", "Could not read serial input, even though data should be available.");
            return;
        }
    }
}

AudioProcessorEditor* SerialInput::createEditor()
{
    editor = new SerialInputEditor(this);
    return editor;
}
