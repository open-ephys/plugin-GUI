/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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


ArduinoOutput::ArduinoOutput()
    : GenericProcessor      ("Arduino Output")
    , gateIsOpen            (true)
    , deviceSelected        (false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    addIntParameter(Parameter::GLOBAL_SCOPE, "output_pin", "The Arduino pin to use", 13, 0, 13);
    addIntParameter(Parameter::STREAM_SCOPE, "input_bit", "The TTL bit for triggering output", 1, 1, 16);
    addIntParameter(Parameter::STREAM_SCOPE, "gate_bit", "The TTL bit for gating the output", 0, 0, 16);
}


ArduinoOutput::~ArduinoOutput()
{
    if (arduino.isInitialized())
        arduino.disconnect();
}


AudioProcessorEditor* ArduinoOutput::createEditor()
{
    editor = std::make_unique<ArduinoOutputEditor>(this);
    return editor.get();
}


void ArduinoOutput::setDevice (String devName)
{
    if (devName.length() == 0)
        return;

    Time timer;

    arduino.connect (devName.toStdString());

    if (arduino.isArduinoReady())
    {
        uint32 currentTime = timer.getMillisecondCounter();

        arduino.sendProtocolVersionRequest();
        timer.waitForMillisecondCounter (currentTime + 2000);
        arduino.update();
        arduino.sendFirmwareVersionRequest();

        timer.waitForMillisecondCounter (currentTime + 4000);
        arduino.update();

        std::cout << "firmata v" << arduino.getMajorFirmwareVersion()
                    << "." << arduino.getMinorFirmwareVersion() << std::endl;
    }

    if (arduino.isInitialized())
    {
        std::cout << "Arduino is initialized." << std::endl;
        arduino.sendDigitalPinMode ((int) getParameter("output_pin")->getValue(), ARD_OUTPUT);
        CoreServices::sendStatusMessage (("Arduino initialized at " + devName));
        deviceSelected = true;
        deviceString = devName;

        // need to inform Editor about the change
    }
    else
    {
        std::cout << "Arduino is NOT initialized." << std::endl;
        CoreServices::sendStatusMessage (("Arduino could not be initialized at " + devName));
    }
}


void ArduinoOutput::updateSettings()
{
    isEnabled = deviceSelected;
}


bool ArduinoOutput::stopAcquisition()
{
    arduino.sendDigital ((int) getParameter("output_pin")->getValue(), ARD_LOW);

    return true;
}


void ArduinoOutput::process (AudioSampleBuffer& buffer)
{
    checkForEvents ();
}


void ArduinoOutput::handleEvent(const EventChannel* eventInfo, const EventPacket& event, int sampleNum)
{
    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserialize(event, eventInfo);

        const int eventBit = ttl->getBit() + 1;
        const uint16 eventStream = ttl->getStreamId();

        if (eventBit == int(getParameter(eventStream, "gate_bit")->getValue()))
        {
            if (ttl->getState())
                gateIsOpen = true;
            else
                gateIsOpen = false;
        }

        if (gateIsOpen)
        {
            if (eventBit == int(getParameter(eventStream, "input_bit")->getValue()))
            {

                if (ttl->getState())
                {
                    arduino.sendDigital(
                        int(getParameter(eventStream, "output_channel")->getValue()),
                        ARD_LOW);
                }
                else
                {
                    arduino.sendDigital(
                        int(getParameter(eventStream, "output_channel")->getValue()),
                        ARD_HIGH);
                }
            }
        }
    }
}


void ArduinoOutput::saveCustomParametersToXml(XmlElement* parentElement)
{
    parentElement->setAttribute("device", deviceString);
}

void ArduinoOutput::loadCustomParametersFromXml(XmlElement* xml)
{
    setDevice(xml->getStringAttribute("device", ""));
}
