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

#ifndef __ARDUINOOUTPUT_H_F7BDA585__
#define __ARDUINOOUTPUT_H_F7BDA585__

#include <SerialLib.h>
#include <ProcessorHeaders.h>
#include "serial/ofArduino.h"



/**
    *UNDER CONSTRUCTION*

    Provides a serial interface to an Arduino board.

    Based on Open Frameworks ofArduino class.

    @see GenericProcessor
 */
class ArduinoOutput : public GenericProcessor
{
public:
    ArduinoOutput();
    ~ArduinoOutput();

    /** Searches for events and triggers the Arduino output when appropriate. */
    void process (AudioSampleBuffer& buffer, MidiBuffer& events) override;

    /** Currently unused. Future uses may include changing the TTL trigger channel
    or the output channel of the Arduino. */
    void setParameter (int parameterIndex, float newValue) override;

    /** Convenient interface for responding to incoming events. */
    void handleEvent (int eventType, MidiMessage& event, int sampleNum) override;

    /** Called immediately prior to the start of data acquisition. */
    bool enable() override;

    /** Called immediately after the end of data acquisition. */
    bool disable() override;

    /** Creates the ArduinoOutputEditor. */
    AudioProcessorEditor* createEditor() override;

    void setOutputChannel (int);
    void setInputChannel  (int);
    void setGateChannel   (int);

    void setDevice (String deviceString);

    int outputChannel;
    int inputChannel;
    int gateChannel;


private:
    /** An open-frameworks Arduino object. */
    ofArduino arduino;

    bool state;
    bool acquisitionIsActive;
    bool deviceSelected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArduinoOutput);
};




#endif  // __ARDUINOOUTPUT_H_F7BDA585__
