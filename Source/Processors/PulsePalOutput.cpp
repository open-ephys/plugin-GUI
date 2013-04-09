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
#include <string.h>
#include "PulsePalOutput.h"



PulsePalOutput::PulsePalOutput()
    : GenericProcessor("Pulse Pal")
{

	std::cout << "Searching for Pulse Pal..." << std::endl;

    //
	// lsusb shows Device 104: ID 1eaf:0004
    // updated udev rules file, but still need to run as root -- no idea why
    //

	vector<ofSerialDeviceInfo> devices = serial.getDeviceList();

	bool foundDevice = false;

	for (int devNum; devNum < devices.size(); devNum++)
	{
		int id = devices[devNum].getDeviceID();
		string path = devices[devNum].getDevicePath();
        string name = devices[devNum].getDeviceName();

       // std::cout << "Device name: " << name << std::endl;

        #ifdef JUCE_LINUX
            string acm0 = "ACM0";
        #endif

        #ifdef JUCE_MAC
            string acm0 = "usbmodemfa131";
        #endif


        size_t index = path.find(acm0);

        if (index != std::string::npos) // only open ttyACM0
        {

            serial.setup(id, 115200);

            uint8_t bytesToWrite[2] = {59, 59};

            serial.writeBytes(bytesToWrite, 2);

            // while (serial.available() == 0)
            // {
            //     serial.writeByte(59);
            // }

            uint8_t resp = serial.readByte();

            if (resp == 5)
            {
                std::cout << "FOUND A PULSE PAL." << std::endl;
                foundDevice = true;
            }

            break;
        }

	}

    triggerPulsePalChannel(1);

}

PulsePalOutput::~PulsePalOutput()
{
    serial.close();
}

AudioProcessorEditor* PulsePalOutput::createEditor()
{
    editor = new PulsePalOutputEditor(this, true);
    return editor;
}

void PulsePalOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
       // do something cool
    }

}

void PulsePalOutput::triggerPulsePalChannel(uint8_t chan)
{
    //uint8_t bytesToWrite[2] = {84, chan};

    //serial.writeBytes(bytesToWrite, 2);
}

void PulsePalOutput::setParameter(int parameterIndex, float newValue)
{

}

void PulsePalOutput::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events,
                         int& nSamples)
{


    checkForEvents(events);


}
