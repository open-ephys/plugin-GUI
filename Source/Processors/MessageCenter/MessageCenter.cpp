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
    GenericProcessor("Message Center"), 
    newEventAvailable(false)
{

    setPlayConfigDetails(0, // number of inputs
                         0, // number of outputs
                         44100.0, // sampleRate
                         128);    // blockSize

    eventChannel = nullptr;

}


void MessageCenter::addSpecialProcessorChannels() 
{
    processorInfo.reset();
    processorInfo = std::unique_ptr<ProcessorInfoObject>(new ProcessorInfoObject(this));

    if (dataStreams.size() == 0)
    {
        DataStream::Settings settings{
        "MessageCenter stream",
        "Description",
        "messagecenter.stream",

        1000.0f
        };

        dataStreams.add(new DataStream(settings));
        dataStreams.getLast()->addProcessor(processorInfo.get());

        EventChannel::Settings eventSettings{
            EventChannel::Type::TEXT,
            "Messages",
            "Broadcasts messages from the MessageCenter",
            "messagecenter.events",

            dataStreams.getLast()
        };

        eventChannels.add(new EventChannel(eventSettings));
        eventChannels.getLast()->addProcessor(processorInfo.get());

        updateChannelIndexMaps();
    }
    
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

DataStream* MessageCenter::getMessageStream()
{
    if (dataStreams.size() > 0)
        return dataStreams[0];

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
    
    if (newEventAvailable)
    {

        String eventString = messageCenterEditor->getOutgoingMessage();

		eventString = eventString.dropLastCharacters(eventString.length() - MAX_MSG_LENGTH);
        
        int64 ts = CoreServices::getGlobalTimestamp();

		TextEventPtr event = TextEvent::createTextEvent(eventChannels[0],
                                                        ts,
                                                        eventString);

		addEvent(event, 0);

        //std::cout << "Message Center added " << eventString << " with timestamp " <<  ts << std::endl;

        newEventAvailable = false;
    }

}
