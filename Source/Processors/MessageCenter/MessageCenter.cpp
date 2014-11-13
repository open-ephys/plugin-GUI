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

#include "MessageCenter.h"
#include "MessageCenterEditor.h"

//---------------------------------------------------------------------

MessageCenter::MessageCenter() :
    GenericProcessor("Message Center"), newEventAvailable(false), isRecording(false), sourceNodeId(0)
{

    setPlayConfigDetails(0, // number of inputs
                         0, // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize

}

MessageCenter::~MessageCenter()
{

}

AudioProcessorEditor* MessageCenter::createEditor()
{

    messageCenterEditor = new MessageCenterEditor(this);

    return messageCenterEditor;

}

void MessageCenter::setParameter(int parameterIndex, float newValue)
{
    if (isRecording)
    {
        newEventAvailable = true;
        messageCenterEditor->messageReceived(true);
    }
    else
    {
        messageCenterEditor->messageReceived(false);
    }

}

void MessageCenter::setSourceNodeId(int id)
{
    sourceNodeId = id;
}

int MessageCenter::getSourceNodeId()
{
    return sourceNodeId;
}

void MessageCenter::process(AudioSampleBuffer& buffer, MidiBuffer& eventBuffer)
{

    if (newEventAvailable)
    {
        int numBytes = 0;

        String eventString = messageCenterEditor->getLabelString();

        CharPointer_UTF8 data = eventString.toUTF8();
        int realId = getNodeId();

        //Fake node ID to the specified source for the event timestamps
        if (sourceNodeId > 0)
            setNodeId(sourceNodeId);

        addEvent(eventBuffer,
                 MESSAGE,
                 0,
                 0,
                 0,
                 data.length()+1, //It doesn't hurt to send the end-string null and can help avoid issues
                 (uint8*) data.getAddress());

        setNodeId(realId);

        newEventAvailable = false;
    }


}

void MessageCenter::addSourceProcessor(GenericProcessor* p)
{
    messageCenterEditor->addSourceProcessor(p);
}

void MessageCenter::removeSourceProcessor(GenericProcessor* p)
{
    messageCenterEditor->removeSourceProcessor(p);
}