/*
   ------------------------------------------------------------------

   This file is part of the Open Ephys GUI
   Copyright (C) 2016 Florian Franzen

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
#define MAX_MSG_SIZE 10000

const int SerialInput::BAUDRATES[12] = 
{
    300
    , 1200
    , 2400
    , 4800
    , 9600
    , 14400
    , 19200
    , 28800
    , 38400
    , 57600
    , 115200
    , 230400
};


SerialInput::SerialInput()
    : GenericProcessor  ("Serial Port")
    , baudrate          (0)
	, lastRecv			(0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);
	dataBuffer.calloc(MAX_MSG_SIZE);
}


SerialInput::~SerialInput()
{
    serial.close();
}

//Since the data needs a meximum buffer size but the actual number of read bytes might be less, let's 
//add that info as a metadata field.
void SerialInput::createEventChannels()
{
	//It's going to be raw binary data, so let's make it uint8
	EventChannel* chan = new EventChannel(EventChannel::UINT8_ARRAY, 1, MAX_MSG_SIZE, CoreServices::getGlobalSampleRate(), this);
	chan->setName("Serial message");
	chan->setDescription("Data received via serial port");
	chan->setIdentifier("external.serial.rawData");
	chan->addEventMetaData(new MetaDataDescriptor(MetaDataDescriptor::UINT64, 1, "Read Bytes", "Number of actual read bytes in the buffer", "eventInfo.data.size"));
	eventChannelArray.add(chan);
}

StringArray SerialInput::getDevices()
{
    vector<ofSerialDeviceInfo> allDeviceInfos = serial.getDeviceList();

    StringArray allDevices;

    for (int i = 0; i < allDeviceInfos.size(); ++i)
    {
        allDevices.add (allDeviceInfos[i].getDeviceName());
    }

    return allDevices;
}


Array<int> SerialInput::getBaudrates() const
{
    Array<int> allBaudrates (BAUDRATES, 12);
    return allBaudrates;
}


void SerialInput::setDevice (string device)
{
    this->device = device;
}

void SerialInput::setBaudrate (int baudrate)
{
    this->baudrate = baudrate;
}


bool SerialInput::isReady()
{
    if (device == "" || baudrate == 0)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "SerialInput connection error!", "Please set device and baudrate to use first!");
        return false;
    }
    if (! serial.setup (device, baudrate))
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "SerialInput connection error!", "Could not connect to specified serial device. Check log files for details.");
        return false;
    }
    return true;
}


bool SerialInput::disable()
{
    serial.close();
    return true;
}


void SerialInput::process (AudioSampleBuffer&)
{
	int64 timestamp = CoreServices::getGlobalTimestamp();
	setTimestampAndSamples(timestamp, 0);

    int bytesAvailable = serial.available();

    if (bytesAvailable == OF_SERIAL_ERROR)
    {
        // ToDo: Properly warn about problem here!
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "SerialInput device access error!", "Could not access serial device.");
        return;
    }

    if (bytesAvailable > 0)
    {

        int bytesRead = serial.readBytes (dataBuffer, bytesAvailable);

        if (bytesRead > 0)
        {
			//Clear the rest of the buffer so we don't send garbage.
			if (bytesRead < lastRecv)
				zeromem(dataBuffer.getData() + bytesRead, lastRecv - bytesRead);
			lastRecv = bytesRead;
			MetaDataValueArray metadata;
			MetaDataValuePtr bufferRead = new MetaDataValue(MetaDataDescriptor::UINT64, 1);
			bufferRead->setValue(static_cast<uint64>(bytesRead));
			metadata.add(bufferRead);
			const EventChannel* chan = getEventChannel(getEventChannelIndex(0, getNodeId()));
			BinaryEventPtr event = BinaryEvent::createBinaryEvent(chan, timestamp, static_cast<uint8*>(dataBuffer.getData()), MAX_MSG_SIZE, metadata);
			addEvent(chan, event, 0);
        }
        else if (bytesRead < 0)
        {
            // ToDo: Properly warn about problem here!
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "SerialInput device read error!", "Could not read serial input, even though data should be available.");
            return;
        }
    }
}


AudioProcessorEditor* SerialInput::createEditor()
{
    editor = new SerialInputEditor (this);
    return editor;
}
