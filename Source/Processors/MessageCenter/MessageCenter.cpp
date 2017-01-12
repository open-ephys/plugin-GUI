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
GenericProcessor("Message Center"), newEventAvailable(false), isRecording(false), sourceNodeId(0), sourceNodeSubIdx(0),
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

void MessageCenter::addSpecialProcessorChannels(Array<EventChannel*>& channels) 
{
	EventChannel* chan = new EventChannel(EventChannel::TEXT, 1, MAX_MSG_LENGTH, getGlobalSampleRate(), this, 0);
	chan->setName("GUI Messages");
	chan->setDescription("Messages from the GUI Message Center");
	channels.add(chan);
	eventChannelArray.add(new EventChannel(*chan));
	updateChannelIndexes();
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

void MessageCenter::setSourceNodeId(int id, int sub)
{
    sourceNodeId = id;
	sourceNodeSubIdx = sub;
	AudioProcessorGraph::Node* node = AccessClass::getProcessorGraph()->getNodeForId(sourceNodeId);
	if (node)
	{
		timestampSource = static_cast<GenericProcessor*>(node->getProcessor());
	}
}

int MessageCenter::getSourceNodeId()
{
    return sourceNodeId;
}

int MessageCenter::getSourceSubIdx()
{
	return sourceNodeSubIdx;
}

int64 MessageCenter::getGlobalTimestamp(bool softwareTime)
{
	if (!softwareTime && sourceNodeId > 0)
		return timestampSource->getSourceTimestamp(sourceNodeId, sourceNodeSubIdx);
    else
        return (softTimestamp);
}

float MessageCenter::getGlobalSampleRate()
{
	if (sourceNodeId > 0)
		return timestampSource->getSampleRate(sourceNodeSubIdx);
	else
		return Time::getHighResolutionTicksPerSecond();
}

void MessageCenter::process(AudioSampleBuffer& buffer)
{
	softTimestamp = Time::getHighResolutionTicks() - lastTime;
    setTimestampAndSamples(getGlobalTimestamp(), 0);
    if (needsToSendTimestampMessage)
    {
		MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
		HeapBlock<char> data;
		size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, this, 0, getTimestamp(true), true);

		eventBuffer.addEvent(data, dataSize, 0);

        needsToSendTimestampMessage = false;
    }

    if (newEventAvailable)
    {
        //int numBytes = 0;

        String eventString = messageCenterEditor->getLabelString();

		eventString = eventString.dropLastCharacters(eventString.length() - MAX_MSG_LENGTH);

		TextEventPtr event = TextEvent::createTextEvent(getEventChannel(0), getGlobalTimestamp(), eventString);
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