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

}

MessageCenter::~MessageCenter()
{

}

void MessageCenter::addSpecialProcessorChannels() 
{

    if (eventChannel == nullptr)
    {
        
        clearSettings();

        eventChannel = new EventChannel(EventChannel::TEXT, 
                                            1, 
                                            MAX_MSG_LENGTH, 
                                            CoreServices::getGlobalSampleRate(), 
                                            this, 0);

        eventChannel->setName("GUI Messages");
        eventChannel->setDescription("Messages from the GUI Message Center");
        eventChannelArray.add(new EventChannel(*eventChannel));

        updateChannelIndexes();
    }
}

AudioProcessorEditor* MessageCenter::createEditor()
{

    messageCenterEditor = new MessageCenterEditor(this);

    return messageCenterEditor;

}

const EventChannel* MessageCenter::getMessageChannel()
{
    return getEventChannel(0);
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
    return true;
}

bool MessageCenter::disable()
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
        //int numBytes = 0;

        String eventString = messageCenterEditor->getOutgoingMessage();

		eventString = eventString.dropLastCharacters(eventString.length() - MAX_MSG_LENGTH);

		TextEventPtr event = TextEvent::createTextEvent(getEventChannel(0), CoreServices::getGlobalTimestamp(), eventString);
		addEvent(getEventChannel(0), event, 0);

        LOGD("Message Center added event.");

        newEventAvailable = false;
    }


}