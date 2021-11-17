/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include <stdio.h>
#include "PhaseDetector.h"
#include "PhaseDetectorEditor.h"

PhaseDetectorSettings::PhaseDetectorSettings() :
    samplesSinceTrigger(0),
    lastSample(0.0f),
    isActive(true),
    wasTriggered(false),
    detectorType(PEAK),
    currentPhase(NO_PHASE),
    triggerChannel(0),
    outputBit(0),
    gateBit(0)
{

}

TTLEventPtr PhaseDetectorSettings::createEvent(int64 timestamp, bool state)
{

    TTLEventPtr event = TTLEvent::createTTLEvent(eventChannel,
        timestamp,
        outputBit,
        state);

   // std::cout << outputBit << std::endl;

    if (state)
    {
        samplesSinceTrigger = 0;
        wasTriggered = true;
    }
    else {
        wasTriggered = false;
    }

    return event;
}

PhaseDetector::PhaseDetector() : GenericProcessor ("Phase Detector")
{

    addSelectedChannelsParameter(Parameter::STREAM_SCOPE, "input_channel", "The continuous channel to analyze", 1);
    addIntParameter(Parameter::STREAM_SCOPE, "output_bit", "The output TTL bit", 1, 1, 16);
    addIntParameter(Parameter::STREAM_SCOPE,"gate_bit", "The input TTL bit for gating the signal", 1, 1, 16);
    addCategoricalParameter(Parameter::STREAM_SCOPE,
        "phase",
        "The phase for triggering the output",
        { "PEAK",
         "FALLING ZERO-CROSSING",
         "TROUGH",
         "RISING ZERO-CROSSING"
          },
        0);
}

AudioProcessorEditor* PhaseDetector::createEditor()
{
    editor = std::make_unique<PhaseDetectorEditor> (this);

    return editor.get();
}

void PhaseDetector::parameterValueChanged(Parameter* param)
{
    if (param->getName().equalsIgnoreCase("phase"))
    {
        settings[param->getStreamId()]->detectorType = DetectorType((int) param->getValue());
    } 
    else if (param->getName().equalsIgnoreCase("input_channel"))
    {
        int localIndex = (int)param->getValue();
        int globalIndex = getDataStream(param->getStreamId())->getContinuousChannels()[localIndex]->getGlobalIndex();
        settings[param->getStreamId()]->triggerChannel = globalIndex;
    } 
    else if (param->getName().equalsIgnoreCase("output_bit"))
    {
        settings[param->getStreamId()]->outputBit = (int)param->getValue() - 1;
    }
    else if (param->getName().equalsIgnoreCase("gate_bit"))
    {
        settings[param->getStreamId()]->gateBit = (int)param->getValue() - 1;
    }

}

void PhaseDetector::updateSettings()
{
    settings.update(getDataStreams());

	for (auto stream : getDataStreams())
	{
        // update "settings" objects
        parameterValueChanged(stream->getParameter("phase"));
        parameterValueChanged(stream->getParameter("input_channel"));
        parameterValueChanged(stream->getParameter("output_bit"));
        parameterValueChanged(stream->getParameter("gate_bit"));

        EventChannel::Settings s{
            EventChannel::Type::TTL,
            "Phase detector output",
            "Triggers when the input signal meets a given phase condition",
            "dataderived.phase",
            getDataStream(stream->getStreamId())

        };

		eventChannels.add(new EventChannel(s));
        eventChannels.getLast()->addProcessor(processorInfo.get());
        settings[stream->getStreamId()]->eventChannel = eventChannels.getLast();
	}
}



void PhaseDetector::handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int sampleNum)
{

    if (Event::getEventType(event)  == EventChannel::TTL)
    {
	    TTLEventPtr ttl = TTLEvent::deserialize(event, channelInfo);

        uint16 eventStream = ttl->getStreamId();
		const int eventBit = ttl->getBit();

        if (settings[eventStream]->gateBit == eventBit)
        {
            settings[eventStream]->isActive = true;
        }
        else {
            settings[eventStream]->isActive = false;
        }
    }
}


void PhaseDetector::process (AudioBuffer<float>& buffer)
{
    checkForEvents();

    // loop through the streams
    for (auto stream : getDataStreams())
    {
        PhaseDetectorSettings* module = settings[stream->getStreamId()];

        // check to see if it's active and has a channel
        if (module->isActive && module->outputBit >= 0
            && module->triggerChannel >= 0
            && module->triggerChannel < buffer.getNumChannels())
        {
            for (int i = 0; i < getNumSourceSamples(stream->getStreamId()); ++i)
            {
                const float sample = *buffer.getReadPointer (module->triggerChannel, i);

                if (sample < module->lastSample
                    && sample > 0
                    && module->currentPhase != FALLING_POS)
                {
                    if (module->detectorType == PEAK)
                    {
                        TTLEventPtr ptr = module->createEvent(
                            getTimestamp(module->triggerChannel) + i,
                            true);
                        
                        addEvent(ptr, i);
                        
                        //LOGD("PEAK");
                    }

                    module->currentPhase = FALLING_POS;
                }
                else if (sample < 0
                         && module->lastSample >= 0
                         && module->currentPhase != FALLING_NEG)
                {
                    if (module->detectorType == FALLING_ZERO)
                    {

                        TTLEventPtr ptr = module->createEvent(
                            getTimestamp(module->triggerChannel) + i,
                            true);

                        addEvent(ptr, i);

                        //("FALLING ZERO");
                    }

                    module->currentPhase = FALLING_NEG;
                }
                else if (sample > module->lastSample 
                         && sample < 0 
                         && module->currentPhase != RISING_NEG)
                {
                    if (module->detectorType == TROUGH)
                    {

                        TTLEventPtr ptr = module->createEvent(
                            getTimestamp(module->triggerChannel) + i,
                            true);

                        addEvent(ptr, i);

                        //LOGD("TROUGH");
                    }

                    module->currentPhase = RISING_NEG;
                }
                else if (sample > 0
                         && module->lastSample <= 0
                         && module->currentPhase != RISING_POS)
                {
                    if (module->detectorType == RISING_ZERO)
                    {
                        TTLEventPtr ptr = module->createEvent(
                            getTimestamp(module->triggerChannel) + i,
                            true);

                        addEvent(ptr, i);

                        //LOGD("RISING ZERO");
                    }

                    module->currentPhase = RISING_POS;
                }

                module->lastSample = sample;

                if (module->wasTriggered)
                {
                    if (module->samplesSinceTrigger > 2000)
                    {
                        TTLEventPtr ptr = module->createEvent(
                            getTimestamp(module->triggerChannel) + i,
                            false);

                        addEvent(ptr, i);

                        //LOGD("TURNING OFF");
                    }
                    else
                    {
                        module->samplesSinceTrigger++;
                    }
                }
            }
        }
    }
}


