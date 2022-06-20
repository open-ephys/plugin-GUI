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
    addIntParameter(Parameter::GLOBAL_SCOPE, "output_pin", "The Arduino pin to use", 13, 0, 13);
    addIntParameter(Parameter::STREAM_SCOPE, "input_line", "The TTL line for triggering output", 1, 1, 16);
    addIntParameter(Parameter::STREAM_SCOPE, "gate_line", "The TTL line for gating the output", 0, 0, 16);
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

    LOGC("Selecting device ", devName);

    if (devName.length() == 0)
        return;

    Time timer;

    arduino.connect (devName.toStdString());

    LOGC("Connected");

    if (arduino.isArduinoReady())
    {
        uint32 currentTime = timer.getMillisecondCounter();

        LOGC("Sending protocol version request");
        arduino.sendProtocolVersionRequest();
        
        timer.waitForMillisecondCounter (currentTime + 200);

        LOGC("Updating...");
        arduino.update();

        LOGC("Sending firmware version request...");
        arduino.sendFirmwareVersionRequest();

        timer.waitForMillisecondCounter (currentTime + 500);

        LOGC("Updating...");
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


void ArduinoOutput::process (AudioBuffer<float>& buffer)
{
    checkForEvents ();
}


void ArduinoOutput::handleTTLEvent(TTLEventPtr event)
{

    const int eventBit = event->getLine() + 1;
    DataStream* stream = getDataStream(event->getStreamId());

    if (eventBit == int((*stream)["gate_line"]))
    {
        if (event->getState())
            gateIsOpen = true;
        else
            gateIsOpen = false;
    }

    if (gateIsOpen)
    {
        if (eventBit == int((*stream)["input_line"]))
        {

            if (event->getState())
            {
                arduino.sendDigital(
                    getParameter("output_pin")->getValue(),
                    ARD_LOW);
            }
            else
            {
                arduino.sendDigital(
                    getParameter("output_pin")->getValue(),
                    ARD_HIGH);
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
    ArduinoOutputEditor* ed = (ArduinoOutputEditor*) editor.get();

    ed->updateDevice(xml->getStringAttribute("device", ""));

}
