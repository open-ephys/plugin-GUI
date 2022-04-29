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

TTLEventPtr EventTranslatorSettings::createEvent(int64 sample_number, int line, bool state)
{

    TTLEventPtr event = TTLEvent::createTTLEvent(eventChannel,
                                                 sample_number,
                                                 line,
                                                 state);

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
                    settings[streamId]->createEvent(newSampleNumber, ttlLine, state);
                
                addEvent(translatedEvent, 0);
            }
            
            
        }
    }
}
