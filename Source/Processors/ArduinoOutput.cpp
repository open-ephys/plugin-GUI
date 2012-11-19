/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#include <stdio.h>

ArduinoOutput::ArduinoOutput()
	: GenericProcessor("Arduino Output"), state(false)
{

}

ArduinoOutput::~ArduinoOutput()
{

}

AudioProcessorEditor* ArduinoOutput::createEditor()
{
    editor = new ArduinoOutputEditor(this);
 	return editor;
}

void ArduinoOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
    	uint8* dataptr = event.getRawData();

    	int eventNodeId = *(dataptr+1);
    	int eventId = *(dataptr+2);
    	int eventChannel = *(dataptr+3);

    	 std::cout << "Received event from " << eventNodeId <<
    	              " on channel " << eventChannel << 
    	              " with value " << eventId << std::endl;

    	if (state)
        {
            arduino.sendDigital(13, ARD_LOW);
            state = false;
        } else {
            arduino.sendDigital(13, ARD_HIGH);
            state = true;
        }

        //ArduinoOutputEditor* ed = (ArduinoOutputEditor*) getEditor();
        //ed->receivedEvent();
    }
    
}

void ArduinoOutput::setParameter (int parameterIndex, float newValue)
{

}

bool ArduinoOutput::enable()
{

    Time timer;

#if JUCE_LINUX
	arduino.connect("ttyACM0");
#endif
#if JUCE_WIN32
	arduino.connect("COM1");
#endif
#if JUCE_MAC
    arduino.connect("tty.usbmodemfd121");
#endif
    

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
        arduino.sendDigitalPinMode(13, ARD_OUTPUT);

    } else {
        std::cout << "Arduino is NOT initialized." << std::endl;
    }
}

bool ArduinoOutput::disable()
{


}

void ArduinoOutput::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{
	

	checkForEvents(events);
	

}