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

#include <stdio.h>
#include "WiFiOutput.h"

WiFiOutput::WiFiOutput()
    : GenericProcessor("WiFi Output")
{
}

WiFiOutput::~WiFiOutput()
{

}

AudioProcessorEditor* WiFiOutput::createEditor()
{
    editor = new WiFiOutputEditor(this, true);
    return editor;
}

void WiFiOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
        startTimer((int) float(event.getTimeStamp())/getSampleRate()*1000.0);
    }

}

void WiFiOutput::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);

}

void WiFiOutput::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events)
{


    checkForEvents(events);


}

void WiFiOutput::timerCallback()
{
    std::cout << "FIRE!" << std::endl;

    try
    {
        socket.sendTo("hi",2,"169.254.1.1",2000);

        WiFiOutputEditor* ed = (WiFiOutputEditor*) getEditor();
        ed->receivedEvent();
    }
    catch (SocketException& e)
    {
        // don't do anything
    }

    stopTimer();
}
