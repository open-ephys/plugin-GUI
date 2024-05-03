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

}

EventTranslator::~EventTranslator()
{

}


void EventTranslator::registerParameters()
{
    // Main sync stream parameter
    addSelectedStreamParameter(Parameter::PROCESSOR_SCOPE, 
        "main_sync", 
        "Main Sync Stream", 
        "Use this stream as main sync", 
        {}, 
        0, 
        true);

    // Sync line selection parameter
    addTtlLineParameter(Parameter::STREAM_SCOPE, 
        "sync_line", 
        "Sync Line", 
        "Event line to use for sync signal", 
        8, 
        true, 
        false, 
        true);
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
        
        synchronizer.addDataStream(stream->getKey(), stream->getSampleRate());

        EventChannel::Settings s{
            EventChannel::Type::TTL,
            "Event Translator output",
            "Translates events from the main stream to all other streams",
            "translator.events",
            getDataStream(streamId)

        };

        eventChannels.add(new EventChannel(s));
        eventChannels.getLast()->addProcessor(this);
        settings[streamId]->eventChannel = eventChannels.getLast();
    }
    
    synchronizer.finishedUpdate();
    
}


void EventTranslator::parameterValueChanged(Parameter* p)
{
    if (p->getName() == "sync_line")
	{
		synchronizer.setSyncLine(getDataStream(p->getStreamId())->getKey(), ((TtlLineParameter*)p)->getSelectedLine());
	}
	else if (p->getName() == "main_sync")
	{
		Array<String> streamNames = ((SelectedStreamParameter*)p)->getStreamNames();
		for (auto stream : dataStreams)
		{
			String key = stream->getKey();
			if (key == streamNames[((SelectedStreamParameter*)p)->getSelectedIndex()])
			{
				synchronizer.setMainDataStream(stream->getKey());
				break;
			}
		}
	}
	else
	{
		LOGD("Event Translator: unknown parameter changed: ", p->getName());
	}

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
    const String eventStreamKey = getDataStream(eventStream)->getKey();
    const int ttlLine = event->getLine();
    const int64 sampleNumber = event->getSampleNumber();
    
    if (synchronizer.getSyncLine(eventStreamKey) == ttlLine)
    {
        synchronizer.addEvent(eventStreamKey, ttlLine, sampleNumber);

        return;
    }
    
    if (eventStreamKey == synchronizer.mainStreamKey && synchronizer.isStreamSynced(eventStreamKey))
    {
        
        //std::cout << "TRANSLATE!" << std::endl;
        
        const bool state = event->getState();
        
        double timestamp = synchronizer.convertSampleNumberToTimestamp(eventStreamKey, sampleNumber);
        
        for (auto stream : getDataStreams())
        {
            
            const uint16 streamId = stream->getStreamId();
            const String streamKey = stream->getKey();

            if (streamKey == eventStreamKey)
                continue; // don't translate events back to the main stream
            
            if (synchronizer.isStreamSynced(streamKey))
            {

                //std::cout << "original sample number: " <<sampleNumber << std::endl;
                //std::cout << "original timestamp: " <<timestamp << std::endl;
                
                int64 newSampleNumber = synchronizer.convertTimestampToSampleNumber(streamKey, timestamp);
                
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
