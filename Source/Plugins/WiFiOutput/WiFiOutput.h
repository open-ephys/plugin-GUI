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

#ifndef __WIFIOUTPUT_H_94D625CE__
#define __WIFIOUTPUT_H_94D625CE__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "WiFiOutputEditor.h"

#include "../../Network/PracticalSocket.h"  // For UDPSocket and SocketException

/**

  Allows the signal chain to send outputs to a client with a specific
  IP address. Used in conjunction with the Arduino WiFly shield, these
  signals can be sent wirelessly.

  @see GenericProcessor, WiFiOutputEditor

*/


class WiFiOutput : public GenericProcessor,
    public Timer

{
public:

    WiFiOutput();
    ~WiFiOutput();

    void process(AudioSampleBuffer& buffer, MidiBuffer& events);
    void setParameter(int parameterIndex, float newValue);

    void handleEvent(int eventType, MidiMessage& event, int sampleNum);

    AudioProcessorEditor* createEditor();

    bool isSink()
    {
        return true;
    }

private:

    UDPSocket socket;

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WiFiOutput);

};


#endif  // __WIFIOUTPUT_H_94D625CE__
