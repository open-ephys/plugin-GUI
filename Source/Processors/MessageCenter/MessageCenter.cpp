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
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../../AccessClass.h"

//---------------------------------------------------------------------

MessageCenter::MessageCenter() :
    GenericProcessor("Message Center"), newEventAvailable(false), isRecording(false), sourceNodeId(0), 
	timestampSource(nullptr), lastTime(0), softTimestamp(0)
{

    setPlayConfigDetails(0, // number of inputs
                         0, // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize

    Channel* ch = new Channel(this, 0, EVENT_CHANNEL);
    eventChannels.add(ch);

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

bool MessageCenter::enable()
{
    messageCenterEditor->startAcquisition();
	lastTime = Time::getHighResolutionTicks();
	softTimestamp = 0;
    if (sourceNodeId)
    {
        AudioProcessorGraph::Node* node = AccessClass::getProcessorGraph()->getNodeForId(sourceNodeId);
        if (node)
        {
            timestampSource = static_cast<GenericProcessor*>(node->getProcessor());
        }
        else
        {
            std::cout << "Message Center: BAD node id " << sourceNodeId << std::endl;
            timestampSource = nullptr;
            sourceNodeId = 0;
        }
    }
    else
        timestampSource = nullptr;

    return true;
}

bool MessageCenter::disable()
{
    messageCenterEditor->stopAcquisition();
    return true;
}

void MessageCenter::setSourceNodeId(int id)
{
    sourceNodeId = id;
}

int MessageCenter::getSourceNodeId()
{
    return sourceNodeId;
}

int64 MessageCenter::getTimestamp(bool softwareTime)
{
    if (!softwareTime && sourceNodeId > 0)
        return timestampSource->getTimestamp(0);
    else
        return (softTimestamp);
}

void MessageCenter::process(AudioSampleBuffer& buffer, MidiBuffer& eventBuffer)
{
	softTimestamp = Time::getHighResolutionTicks() - lastTime;
    setTimestamp(eventBuffer,getTimestamp());
    if (needsToSendTimestampMessage)
    {
        String eventString = "Software time: " + String(getTimestamp(true)) + "@" + String(Time::getHighResolutionTicksPerSecond()) + "Hz";
        CharPointer_UTF8 data = eventString.toUTF8();

        addEvent(eventBuffer,
                 MESSAGE,
                 0,
                 0,
                 0,
                 data.sizeInBytes(), //It doesn't hurt to send the end-string null and can help avoid issues
                 (uint8*)data.getAddress());

        needsToSendTimestampMessage = false;
    }

    if (newEventAvailable)
    {
        //int numBytes = 0;

        String eventString = messageCenterEditor->getLabelString();

        CharPointer_UTF8 data = eventString.toUTF8();

        addEvent(eventBuffer,
                 MESSAGE,
                 0,
                 0,
                 0,
                 data.sizeInBytes(), //It doesn't hurt to send the end-string null and can help avoid issues
                 (uint8*) data.getAddress());

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