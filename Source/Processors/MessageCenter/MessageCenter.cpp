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
#define MAX_MSG_LENGTH 512
//---------------------------------------------------------------------

MessageCenter::MessageCenter() :
    GenericProcessor("Message Center"), newEventAvailable(false), isRecording(false), sourceNodeId(0), 
	timestampSource(nullptr), lastTime(0), softTimestamp(0)
{

    setPlayConfigDetails(0, // number of inputs
                         0, // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize
}

MessageCenter::~MessageCenter()
{

}

void MessageCenter::getDefaultEventInfo(Array<DefaultEventInfo>& events, int sub) const
{
	if (sub > 0) return;
	events.add(DefaultEventInfo(EventChannel::TEXT, 1, MAX_MSG_LENGTH));
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

void MessageCenter::process(AudioSampleBuffer& buffer)
{
	softTimestamp = Time::getHighResolutionTicks() - lastTime;
    setTimestampAndSamples(getTimestamp(), 0);
    if (needsToSendTimestampMessage)
    {
		MidiBuffer& eventBuffer = *AccessClass::getProcessorMidiBuffer(this);
        String eventString = "Software time: " + String(getTimestamp(true)) + "@" + String(Time::getHighResolutionTicksPerSecond()) + "Hz";
        
		size_t textSize = eventString.getNumBytesAsUTF8();
		size_t dataSize = 17 + textSize;
		HeapBlock<char> data(dataSize, true);
		data[0] = SYSTEM_EVENT;
		data[1] = TIMESTAMP_SYNC_TEXT;
		*reinterpret_cast<uint16*>(data.getData() + 2) = getNodeId();
		*reinterpret_cast<uint16*>(data.getData() + 4) = 0;
		*reinterpret_cast<uint64*>(data.getData() + 8) = getTimestamp(true);
		memcpy(data.getData() + 16, eventString.toUTF8(), textSize);

		eventBuffer.addEvent(data, dataSize, 0);

        needsToSendTimestampMessage = false;
    }

    if (newEventAvailable)
    {
        //int numBytes = 0;

        String eventString = messageCenterEditor->getLabelString();

		eventString = eventString.dropLastCharacters(eventString.length() - MAX_MSG_LENGTH);

		TextEventPtr event = TextEvent::createTextEvent(getEventChannel(0), getTimestamp(), eventString);
		addEvent(getEventChannel(0), event, 0);

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