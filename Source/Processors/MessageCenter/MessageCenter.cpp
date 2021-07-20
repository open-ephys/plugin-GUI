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
#include "../../Utils/Utils.h"

#include "../Events/Event.h"
#include "../Settings/ProcessorInfo.h"

#define MAX_MSG_LENGTH 512
//---------------------------------------------------------------------

MessageCenter::MessageCenter() :
GenericProcessor("Message Center"), newEventAvailable(false), isRecording(false)
{

    setPlayConfigDetails(0, // number of inputs
                         0, // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize

    eventChannel = nullptr;

    needsToSendTimestampMessage = true;

}

MessageCenter::~MessageCenter()
{

}

void MessageCenter::addSpecialProcessorChannels() 
{
    processorInfo.reset();
    processorInfo = std::unique_ptr<ProcessorInfoObject>(new ProcessorInfoObject(this));

    clearSettings();

    EventChannel::Settings settings{
        EventChannel::Type::TEXT,
        "Messages",
        "Broadcasts messages from the MessageCenter",
        "messagecenter.events"
    };

    eventChannels.add(new EventChannel(settings));
    eventChannels.getLast()->addProcessor(processorInfo.get());

    updateChannelIndexMaps();
}

AudioProcessorEditor* MessageCenter::createEditor()
{

    messageCenterEditor = new MessageCenterEditor(this);

    return messageCenterEditor;

}

const EventChannel* MessageCenter::getMessageChannel()
{
    if (eventChannels.size() > 0)
        return eventChannels[0];
    else
        return nullptr;
}

void MessageCenter::setParameter(int parameterIndex, float newValue)
{
    if (parameterIndex == 1)
    {
        newEventAvailable = true;
        messageCenterEditor->messageReceived(true);
    }

}

bool MessageCenter::startAcquisition()
{
    messageCenterEditor->startAcquisition();
    return true;
}

bool MessageCenter::stopAcquisition()
{
    messageCenterEditor->stopAcquisition();
    return true;
}

void MessageCenter::process(AudioSampleBuffer& buffer)
{
    if (needsToSendTimestampMessage)
    {
		MidiBuffer& eventBuffer = *AccessClass::ExternalProcessorAccessor::getMidiBuffer(this);
		HeapBlock<char> data;
		size_t dataSize = SystemEvent::fillTimestampSyncTextData(data, this, 0, CoreServices::getGlobalTimestamp(), true);

		eventBuffer.addEvent(data, dataSize, 0);

        needsToSendTimestampMessage = false;
    }

    if (newEventAvailable)
    {

        String eventString = messageCenterEditor->getOutgoingMessage();

		eventString = eventString.dropLastCharacters(eventString.length() - MAX_MSG_LENGTH);

		TextEventPtr event = TextEvent::createTextEvent(eventChannels[0], CoreServices::getGlobalTimestamp(), eventString);

		addEvent(eventChannels[0], event, 0);

        LOGD("Message Center added ", eventString);

        newEventAvailable = false;
    }

}