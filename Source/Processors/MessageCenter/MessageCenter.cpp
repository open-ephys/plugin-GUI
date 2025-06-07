/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "../../AccessClass.h"
#include "../../Utils/Utils.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "MessageCenterEditor.h"

#include "../Events/Event.h"

#define MAX_MSG_LENGTH 512
//---------------------------------------------------------------------

MessageCenter::MessageCenter() : GenericProcessor ("Message Center"),
                                 newEventAvailable (false)
{
    setPlayConfigDetails (0, // number of inputs
                          0, // number of outputs
                          44100.0, // sampleRate
                          128); // blockSize

    eventChannel = nullptr;
}

void MessageCenter::addSpecialProcessorChannels()
{
    if (dataStreams.size() == 0)
    {
        DataStream::Settings settings {
            "MessageCenter stream",
            "Description",
            "messagecenter.stream",

            1000.0f
        };

        dataStreams.add (new DataStream (settings));
        dataStreams.getLast()->addProcessor (this);

        EventChannel::Settings eventSettings {
            EventChannel::Type::TEXT,
            "Messages",
            "Broadcasts messages from the MessageCenter",
            "messagecenter.events",

            dataStreams.getLast()
        };

        eventChannels.add (new EventChannel (eventSettings));
        eventChannels.getLast()->addProcessor (this);

        updateChannelIndexMaps();
    }
}

AudioProcessorEditor* MessageCenter::createEditor()
{
    messageCenterEditor = new MessageCenterEditor (this);

    return messageCenterEditor;
}

bool MessageCenter::startAcquisition()
{
    if (messageCenterEditor != nullptr)
        messageCenterEditor->startAcquisition();

    return true;
}

bool MessageCenter::stopAcquisition()
{
    if (messageCenterEditor != nullptr)
        messageCenterEditor->stopAcquisition();

    return true;
}

const EventChannel* MessageCenter::getMessageChannel()
{
    if (eventChannels.size() > 0)
        return eventChannels[0];
    else
        return nullptr;
}

DataStream* MessageCenter::getMessageStream()
{
    if (dataStreams.size() > 0)
        return dataStreams[0];

    return nullptr;
}

void MessageCenter::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex == 1)
    {
        newEventAvailable = true;
    }
}

void MessageCenter::actionListenerCallback (const String& message)
{
    if (messageCenterEditor != nullptr)
        messageCenterEditor->addIncomingMessage (message);
    else
        LOGC (message);
}

void MessageCenter::broadcastMessage (const String& msg)
{
    broadcastMessage (msg, CoreServices::getSystemTime());
}

void MessageCenter::broadcastMessage (const String& msg, const int64 systemTimeMilliseconds)
{
    Message newMessage { msg, systemTimeMilliseconds };
    messageQueue.push (newMessage);

    setParameter (1, 1);
}

void MessageCenter::addOutgoingMessage (const String& msg, const int64 systemTimeMilliseconds)
{
    broadcastMessage (msg, systemTimeMilliseconds);
    addSavedMessage (msg);

    if (messageCenterEditor != nullptr)
        messageCenterEditor->addOutgoingMessage (msg, systemTimeMilliseconds);
}

void MessageCenter::addSavedMessage (const String& msg)
{
    bool foundMatch = false;

    for (auto message : savedMessages)
    {
        if (message == msg)
        {
            foundMatch = true;
            break;
        }
    }

    if (! foundMatch)
    {
        savedMessages.add (msg);
    }
}

void MessageCenter::clearSavedMessages()
{
    savedMessages.clear();
}

Array<String>& MessageCenter::getSavedMessages()
{
    return savedMessages;
}

void MessageCenter::saveStateToXml (XmlElement* parent)
{
    for (auto message : savedMessages)
    {
        XmlElement* messageXml = new XmlElement ("MSG");
        messageXml->setAttribute ("String", message);
        parent->addChildElement (messageXml);
    }
}

void MessageCenter::loadStateFromXml (XmlElement* parent)
{
    for (auto* child : parent->getChildIterator())
    {
        savedMessages.add (child->getStringAttribute ("String"));
    }
}

void MessageCenter::process (AudioBuffer<float>& buffer)
{
    if (newEventAvailable)
    {
        while (! messageQueue.empty())
        {
            Message message = messageQueue.front();

            String eventString = message.message;

            eventString.dropLastCharacters (eventString.length() - MAX_MSG_LENGTH);

            TextEventPtr event = TextEvent::createTextEvent (eventChannels[0],
                                                             message.systemTimeMilliseconds,
                                                             eventString);

            addEvent (event, 0);

            messageQueue.pop();

            LOGD ("Message Center sending message: ", eventString);

            newEventAvailable = false;
        }
    }
}
