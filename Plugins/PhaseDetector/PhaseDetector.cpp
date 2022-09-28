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
    outputLine(0),
    gateLine(0)
{

}

TTLEventPtr PhaseDetectorSettings::createEvent(int64 sample_number, bool state)
{

    TTLEventPtr event = TTLEvent::createTTLEvent(eventChannel,
                                                 sample_number,
                                                 outputLine,
                                                 state);

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


TTLEventPtr PhaseDetectorSettings::clearOutputLine(int64 sample_number)
{

    TTLEventPtr event = TTLEvent::createTTLEvent(eventChannel,
                                                 sample_number,
                                                 lastOutputLine,
                                                 false);

    outputLineChanged = false;

    return event;
}

PhaseDetector::PhaseDetector() : GenericProcessor ("Phase Detector")
{

    addSelectedChannelsParameter(Parameter::STREAM_SCOPE, "Channel", "The continuous channel to analyze", 1);
    addIntParameter(Parameter::STREAM_SCOPE, "TTL_out", "The output TTL line", 1, 1, 16);
    addIntParameter(Parameter::STREAM_SCOPE,"gate_line", "The input TTL line for gating the signal (0 = off)", 0, 0, 16);
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
    else if (param->getName().equalsIgnoreCase("Channel"))
    {
        Array<var>* array = param->getValue().getArray();
        
        if (array->size() > 0)
        {
            int localIndex = int(array->getFirst());
            int globalIndex = getDataStream(param->getStreamId())->getContinuousChannels()[localIndex]->getGlobalIndex();
            settings[param->getStreamId()]->triggerChannel = globalIndex;
        }
        else
        {
            settings[param->getStreamId()]->triggerChannel = -1;
        }
    } 
    else if (param->getName().equalsIgnoreCase("TTL_out"))
    {
        settings[param->getStreamId()]->lastOutputLine = settings[param->getStreamId()]->outputLine;
        settings[param->getStreamId()]->outputLine = (int)param->getValue() - 1;
        settings[param->getStreamId()]->outputLineChanged = true;
    }
    else if (param->getName().equalsIgnoreCase("gate_line"))
    {
        settings[param->getStreamId()]->gateLine = (int)param->getValue() - 1;
    }

}

void PhaseDetector::updateSettings()
{
    settings.update(getDataStreams());

	for (auto stream : getDataStreams())
	{
        // update "settings" objects
        parameterValueChanged(stream->getParameter("phase"));
        parameterValueChanged(stream->getParameter("Channel"));
        parameterValueChanged(stream->getParameter("TTL_out"));
        parameterValueChanged(stream->getParameter("gate_line"));

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



void PhaseDetector::handleTTLEvent (TTLEventPtr event)
{

    const uint16 eventStream = event->getStreamId();
	
    if (settings[eventStream]->gateLine > -1)
    {
     
        if (settings[eventStream]->gateLine == event->getLine())
            settings[eventStream]->isActive = event->getState();
        
    }

}


void PhaseDetector::process (AudioBuffer<float>& buffer)
{
    checkForEvents();

    // loop through the streams
    for (auto stream : getDataStreams())
    {

        if ((*stream)["enable_stream"])
        {
            PhaseDetectorSettings* module = settings[stream->getStreamId()];
            
            const uint16 streamId = stream->getStreamId();
            const int64 firstSampleInBlock = getFirstSampleNumberForBlock(streamId);
            const uint32 numSamplesInBlock = getNumSamplesInBlock(streamId);

            // check to see if it's active and has a channel
            if (module->isActive && module->outputLine >= 0
                && module->triggerChannel >= 0
                && module->triggerChannel < buffer.getNumChannels())
            {
                for (int i = 0; i < numSamplesInBlock; ++i)
                {
                    const float sample = *buffer.getReadPointer(module->triggerChannel, i);

                    if (sample < module->lastSample
                        && sample > 0
                        && module->currentPhase != FALLING_POS)
                    {
                        if (module->detectorType == PEAK)
                        {
                            TTLEventPtr ptr = module->createEvent(
                                                                  firstSampleInBlock + i,
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
                                                                  firstSampleInBlock + i,
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
                                                                  firstSampleInBlock + i,
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
                                                                  firstSampleInBlock + i,
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
                                                                  firstSampleInBlock + i,
                                                                  false);

                            addEvent(ptr, i);

                            //LOGD("TURNING OFF");
                        }
                        else
                        {
                            module->samplesSinceTrigger++;
                        }
                    }

                    if (module->outputLineChanged)
                    {
                        TTLEventPtr ptr = module->clearOutputLine(
                                                                 firstSampleInBlock + i);

                        addEvent(ptr, i);

                    }
                }
            }

            // If event is on when 'None' is selected in channel selector, turn off event
            if (module->wasTriggered && module->triggerChannel < 0)
            {
                TTLEventPtr ptr = module->createEvent(firstSampleInBlock, false);

                addEvent(ptr, 0);
            }
        }

        
    }
}


