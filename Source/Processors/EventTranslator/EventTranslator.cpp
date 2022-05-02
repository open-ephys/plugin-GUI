/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2022 Open Ephys

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

#include "EventTranslator.h"
#include "EventTranslatorEditor.h"

EventTranslatorSettings::EventTranslatorSettings()
{

}

TTLEventPtr EventTranslatorSettings::createEvent(int64 sample_number,
                                                 double timestamp,
                                                 int line,
                                                 bool state)
{

    TTLEventPtr event = TTLEvent::createTTLEvent(eventChannel,
                                                 sample_number,
                                                 line,
                                                 state);

    event->setTimestampInSeconds(timestamp);
    
    return event;
}


EventTranslator::EventTranslator() : GenericProcessor("Event Translator")
{
    addIntParameter(Parameter::STREAM_SCOPE,"sync_line", "The TTL sync line for a given stream", 0, 0, 16);
}

EventTranslator::~EventTranslator()
{

}

AudioProcessorEditor* EventTranslator::createEditor()
{
    editor = std::make_unique<EventTranslatorEditor>(this);

    return editor.get();
}


void EventTranslator::updateSettings()
{
    
    settings.update(getDataStreams());

    synchronizer.prepareForUpdate();

    for (auto stream : getDataStreams())
    {
        
        const uint16 streamId = stream->getStreamId();
        
        synchronizer.addDataStream(streamId, stream->getSampleRate());

        EventChannel::Settings s{
            EventChannel::Type::TTL,
            "Event Translator output",
            "Translates events from the main stream to all other streams",
            "translator.events",
            getDataStream(streamId)

        };

        eventChannels.add(new EventChannel(s));
        eventChannels.getLast()->addProcessor(processorInfo.get());
        settings[streamId]->eventChannel = eventChannels.getLast();
    }
    
    synchronizer.finishedUpdate();
    
}

bool EventTranslator::startAcquisition()
{
    synchronizer.startAcquisition();
    
    return true;
}

bool EventTranslator::stopAcquisition()
{
    synchronizer.stopAcquisition();
    
    return true;
}

void EventTranslator::process (AudioBuffer<float>& buffer)
{
    checkForEvents();
}


void EventTranslator::handleTTLEvent(TTLEventPtr event)
{
    const uint16 eventStream = event->getStreamId();
    const int ttlLine = event->getLine();
    const int64 sampleNumber = event->getSampleNumber();
    
    if (synchronizer.getSyncLine(eventStream) == ttlLine)
    {
        synchronizer.addEvent(eventStream, ttlLine, sampleNumber);

        return;
    }
    
    if (eventStream == synchronizer.mainStreamId && synchronizer.isStreamSynced(eventStream))
    {
        
        //std::cout << "TRANSLATE!" << std::endl;
        
        const bool state = event->getState();
        
        double timestamp = synchronizer.convertSampleNumberToTimestamp(eventStream, sampleNumber);
        
        for (auto stream : getDataStreams())
        {
            
            const uint16 streamId = stream->getStreamId();

            if (streamId == eventStream)
                continue; // don't translate events back to the main stream
            
            if (synchronizer.isStreamSynced(streamId))
            {

                //std::cout << "original sample number: " <<sampleNumber << std::endl;
                //std::cout << "original timestamp: " <<timestamp << std::endl;
                
                int64 newSampleNumber = synchronizer.convertTimestampToSampleNumber(streamId, timestamp);
                
                //std::cout << "new sample number (" << streamId << "): " << newSampleNumber << std::endl;
                //std::cout << std::endl;
                
                if (newSampleNumber < getFirstSampleNumberForBlock(streamId))
                    newSampleNumber = getFirstSampleNumberForBlock(streamId);
                
                TTLEventPtr translatedEvent =
                    settings[streamId]->createEvent(newSampleNumber, timestamp, ttlLine, state);
                
                addEvent(translatedEvent, 0);
            }
            
            
        }
    }
}


void EventTranslator::saveCustomParametersToXml(XmlElement* xml)
{

    for (auto stream : getDataStreams())
    {

        const uint16 streamId = stream->getStreamId();

        XmlElement* streamXml = xml->createNewChildElement("STREAM");

        streamXml->setAttribute("isMainStream", synchronizer.mainStreamId == streamId);
        streamXml->setAttribute("sync_line", getSyncLine(streamId));
        streamXml->setAttribute("name", stream->getName());
        streamXml->setAttribute("source_node_id", stream->getSourceNodeId());
        streamXml->setAttribute("sample_rate", stream->getSampleRate());
        streamXml->setAttribute("channel_count", stream->getChannelCount());
    }
}


void EventTranslator::loadCustomParametersFromXml(XmlElement* xml)
{
    
    for (auto* subNode : xml->getChildIterator())
    {
        if (subNode->hasTagName("STREAM"))
        {

            ParameterCollection* parameterCollection = new ParameterCollection();

            parameterCollection->owner.channel_count = subNode->getIntAttribute("channel_count");
            parameterCollection->owner.name = subNode->getStringAttribute("name");
            parameterCollection->owner.sample_rate = subNode->getDoubleAttribute("sample_rate");
            parameterCollection->owner.channel_count = subNode->getIntAttribute("channel_count");
            parameterCollection->owner.sourceNodeId = subNode->getIntAttribute("source_node_id");
            
            savedDataStreamParameters.add(parameterCollection);
        }
    }
    
    for (auto stream : dataStreams)
    {
        int matchingIndex = findMatchingStreamParameters(stream);
        const uint16 streamId = stream->getStreamId();
        
        if (matchingIndex > -1)
        {
            int savedStreamIndex = -1;
            
            for (auto* subNode : xml->getChildIterator())
            {
                if (subNode->hasTagName("STREAM"))
                {
                    savedStreamIndex++;
                    
                    if (savedStreamIndex == matchingIndex)
                    {
                        if (subNode->getBoolAttribute("isMainStream", false))
                        {
                            setMainDataStream(streamId);
                        }
                        
                        setSyncLine(streamId, subNode->getIntAttribute("sync_line", 0));
                        
                    }
                }
            }
        }
    }
}
