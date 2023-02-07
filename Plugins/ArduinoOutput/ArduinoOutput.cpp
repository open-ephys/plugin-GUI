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


// Debug switch.
#define ARDDEBUG_WANT_EVENT_TATTLE 0


ArduinoOutput::ArduinoOutput()
    : GenericProcessor      ("Arduino Output")
    , ardDigOutput          (13)
    , inputBankIdx          (-1)
    , inputBit              (-1)
    , gateBankIdx           (-1)
    , gateBit               (-1)
    , state                 (true)
    , acquisitionIsActive   (false)
    , deviceSelected        (false)
{
    setProcessorType (PROCESSOR_TYPE_SINK);
}


ArduinoOutput::~ArduinoOutput()
{
    if (arduino.isInitialized())
        arduino.disconnect();
}


AudioProcessorEditor* ArduinoOutput::createEditor()
{
    editor = new ArduinoOutputEditor (this, true);
    return editor;
}


void ArduinoOutput::setDevice (String devName)
{
    if (! acquisitionIsActive)
    {
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
            arduino.sendDigitalPinMode (ardDigOutput, ARD_OUTPUT);
            CoreServices::sendStatusMessage (("Arduino initialized at " + devName));
            deviceSelected = true;
        }
        else
        {
            std::cout << "Arduino is NOT initialized." << std::endl;
            CoreServices::sendStatusMessage (("Arduino could not be initialized at " + devName));
        }
    }
    else
    {
        CoreServices::sendStatusMessage ("Cannot change device while acquisition is active.");
    }
}


void ArduinoOutput::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
{
    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);

        const int eventId       = ttl->getState() ? 1: 0;
        const int eventBankIdx  = getEventChannelIndex(ttl);
        const int eventBit      = ttl->getChannel();

#if ARDDEBUG_WANT_EVENT_TATTLE
        std::cout << "[Ard]  Received event on bank " << eventBankIdx
            << " bit " << eventBit << " with value: " << eventId << std::endl;
#endif

        if ( (eventBankIdx == gateBankIdx) && (eventBit == gateBit) )
        {
            if (eventId == 1)
                state = true;
            else
                state = false;

#if ARDDEBUG_WANT_EVENT_TATTLE
            std::cout << "[Ard]  Set gate state to: " << eventId << std::endl;
#endif
        }

        if (state)
        {
            if ( (inputBankIdx == -1) || (inputBit == -1)
                || ((eventBankIdx == inputBankIdx) && (eventBit == inputBit)) )
            {
                if (eventId == 0)
                {
                    arduino.sendDigital (ardDigOutput, ARD_LOW);
                }
                else
                {
                    arduino.sendDigital (ardDigOutput, ARD_HIGH);
                }

#if ARDDEBUG_WANT_EVENT_TATTLE
                std::cout << "[Ard]  Set DIO " << ardDigOutput << " to: "
                    << eventId << std::endl;
#endif
            }
        }
    }
}


void ArduinoOutput::setParameter (int parameterIndex, float newValue)
{
    // make sure current output channel is off:
    arduino.sendDigital(ardDigOutput, ARD_LOW);

    bool needGateUpdate = false;

    switch (parameterIndex)
    {
        case ARDOUT_PARAM_DIGOUT: ardDigOutput = (int) newValue; break;
        case ARDOUT_PARAM_INBANKIDX: inputBankIdx = (int) newValue; break;
        case ARDOUT_PARAM_INBIT: inputBit = (int) newValue; break;
        case ARDOUT_PARAM_GATEBANKIDX:
            gateBankIdx = (int) newValue;
            needGateUpdate = true;
            break;
        case ARDOUT_PARAM_GATEBIT:
            gateBit = (int) newValue;
            needGateUpdate = true;
            break;
        default: break;
    };

    if (needGateUpdate)
    {
        if ( (gateBankIdx == -1) || (gateBit == -1) )
            state = true;
        else
            state = false;
    }
}


bool ArduinoOutput::enable()
{
    acquisitionIsActive = true;

    return deviceSelected;
}


bool ArduinoOutput::disable()
{
    arduino.sendDigital (ardDigOutput, ARD_LOW);
    acquisitionIsActive = false;

    return true;
}


void ArduinoOutput::process (AudioSampleBuffer& buffer)
{
    checkForEvents ();
}
