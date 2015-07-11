/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "ArduinoOutput.h"
#include "ArduinoOutputEditor.h"

#include <stdio.h>

extern "C" PluginManager::Plugin * OpenEphysPlugin()
{
	return new ArduinoOutput();
}

struct PluginInfo {
	size_t version;
	String name;
	size_t type;
} info;

ArduinoOutput::ArduinoOutput()
    : GenericProcessor("Arduino Output"), state(true), deviceSelected(false), outputChannel(13), inputChannel(-1)
{
	info.version = 0.1;
	info.name = "Arduino Output";
	info.type = 0x2;
	printf("Created processor %s\n", info.name.toStdString().c_str());
}

ArduinoOutput::~ArduinoOutput()
{

    if (arduino.isInitialized())
        arduino.disconnect();

}

AudioProcessorEditor* ArduinoOutput::createEditor()
{
    editor = new ArduinoOutputEditor(this, true);
    return editor;
}

void ArduinoOutput::setDevice(String devName)
{

    if (!acquisitionIsActive)
    {

        Time timer;

        arduino.connect(devName.toStdString());

        if (arduino.isArduinoReady())
        {

            uint32 currentTime = timer.getMillisecondCounter();

            arduino.sendProtocolVersionRequest();
            timer.waitForMillisecondCounter(currentTime + 2000);
            arduino.update();
            arduino.sendFirmwareVersionRequest();

            timer.waitForMillisecondCounter(currentTime + 4000);
            arduino.update();

            std::cout << "firmata v" << arduino.getMajorFirmwareVersion()
                      << "." << arduino.getMinorFirmwareVersion() << std::endl;

        }

        if (arduino.isInitialized())
        {

            std::cout << "Arduino is initialized." << std::endl;
            arduino.sendDigitalPinMode(outputChannel, ARD_OUTPUT);
            CoreServices::sendStatusMessage(("Arduino initialized at" + devName).toUTF8());
            deviceSelected = true;
        }
        else
        {
            std::cout << "Arduino is NOT initialized." << std::endl;
			CoreServices::sendStatusMessage(("Arduino could not be initialized at" + devName).toUTF8());
        }
    } else {
		CoreServices::sendStatusMessage("Cannot change device while acquisition is active.");
    }


}


void ArduinoOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

       // std::cout << "Received event from " << eventNodeId <<
        //          " on channel " << eventChannel <<
        //          " with value " << eventId << std::endl;

        if (eventChannel == gateChannel)
        {
            if (eventId == 1)
            {
                state = true;
            } else {
                state = false;
            }
        }

        if (state)
        {
            if (inputChannel == -1 || eventChannel == inputChannel)
            {
                if (eventId == 0)
                {
                    arduino.sendDigital(outputChannel, ARD_LOW);
                }
                else
                {
                    arduino.sendDigital(outputChannel, ARD_HIGH);
                }
            }
        }

        //ArduinoOutputEditor* ed = (ArduinoOutputEditor*) getEditor();
        //ed->receivedEvent();
    }

}

void ArduinoOutput::setParameter(int parameterIndex, float newValue)
{
    // make sure current output channel is off:
    arduino.sendDigital(outputChannel, ARD_LOW);

    if (parameterIndex == 0)
    {
        outputChannel = (int) newValue;
    } else if (parameterIndex == 1)
    {
        inputChannel = (int) newValue;
    } else if (parameterIndex == 2)
    {
        gateChannel = (int) newValue;
        if (gateChannel == -1)
            state = true;
        else
            state = false;
    }
}

void ArduinoOutput::setOutputChannel(int chan)
{
    setParameter(0, chan);
}

void ArduinoOutput::setInputChannel(int chan)
{
    setParameter(1, chan-1);
}

void ArduinoOutput::setGateChannel(int chan)
{
    setParameter(2, chan-1);
}

bool ArduinoOutput::enable()
{
    acquisitionIsActive = true;
	return deviceSelected;
}

bool ArduinoOutput::disable()
{
    arduino.sendDigital(outputChannel, ARD_LOW);
    acquisitionIsActive = false;
	return true;
}

void ArduinoOutput::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events)
{


    checkForEvents(events);


}
