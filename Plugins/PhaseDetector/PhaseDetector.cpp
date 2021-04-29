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


PhaseDetector::PhaseDetector()
    : GenericProcessor      ("Phase Detector")
    , activeModule          (-1)
    , risingPos             (false)
    , risingNeg             (false)
    , fallingPos            (false)
    , fallingNeg            (false)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
	lastNumInputs = 0;
}


PhaseDetector::~PhaseDetector()
{
}


AudioProcessorEditor* PhaseDetector::createEditor()
{
    editor = new PhaseDetectorEditor (this, true);

    //std::cout << "Creating editor." << std::endl;

    return editor;
}

void PhaseDetector::addModule()
{
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0f;
    m.type = NONE;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.phase = NO_PHASE;

    modules.add (m);
}


void PhaseDetector::setActiveModule (int i)
{
    activeModule = i;
}


void PhaseDetector::setParameter (int parameterIndex, float newValue)
{
    DetectorModule& module = modules.getReference (activeModule);

    if (parameterIndex == 1) // module type
    {
        int val = (int) newValue;

        switch (val)
        {
            case 0:
                module.type = NONE;
                break;

            case 1:
                module.type = PEAK;
                break;

            case 2:
                module.type = FALLING_ZERO;
                break;

            case 3:
                module.type = TROUGH;
                break;

            case 4:
                module.type = RISING_ZERO;
                break;

            default:
                module.type = NONE;
        }
    }
    else if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int) newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int) newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int) newValue;
        if (module.gateChan < 0)
        {
            module.isActive = true;
        }
        else
        {
            module.isActive = false;
        }
    }
}

//Usually, to be more ordered, we'd create the event channels overriding the createEventChannels() method.
//However, since in this case there a couple of things we need to do prior to creating the channels (resetting
//the modules input channels in case the channel count changes, to reflect the same change on the combo box)
//we think it's better to do all in this method, that gets always called after all the create*Channels.
void PhaseDetector::updateSettings()
{
	moduleEventChannels.clear();
	for (int i = 0; i < modules.size(); i++)
	{
		if (getNumInputs() != lastNumInputs)
			modules.getReference(i).inputChan = -1;
		const DataChannel* in = getDataChannel(modules[i].inputChan);
		EventChannel *ev;
		if (in)
			ev = new EventChannel(EventChannel::TTL, 8, 1, in, this);
		else
			ev = new EventChannel(EventChannel::TTL, 8, 1, -1, this);

		ev->setName("Phase detector output " + String(i + 1));
		ev->setDescription("Triggers when the input signal mets a given phase condition");
		String identifier = "dataderived.phase.";
		String typeDesc;
		switch (modules[i].type)
		{
		case PEAK: typeDesc = "Positive peak"; identifier += "peak.positve";  break;
		case FALLING_ZERO: typeDesc = "Zero crossing with negative slope"; identifier += "zero.negative";  break;
		case TROUGH: typeDesc = "Negative peak"; identifier += "peak.negative"; break;
		case RISING_ZERO: typeDesc = "Zero crossing with positive slope"; identifier += "zero.positive"; break;
		default: typeDesc = "No phase selected"; break;
		}
		ev->setIdentifier(identifier);
		MetaDataDescriptor md(MetaDataDescriptor::CHAR, 34, "Phase Type", "Description of the phase condition", "channelInfo.extra");
		MetaDataValue mv(md);
		mv.setValue(typeDesc);
		ev->addMetaData(md, mv);
		if (in)
		{
			md = MetaDataDescriptor(MetaDataDescriptor::UINT16, 3, "Source Channel",
				"Index at its source, Source processor ID and Sub Processor index of the channel that triggers this event", "source.channel.identifier.full");
			mv = MetaDataValue(md);
			uint16 sourceInfo[3];
			sourceInfo[0] = in->getSourceIndex();
			sourceInfo[1] = in->getSourceNodeID();
			sourceInfo[2] = in->getSubProcessorIdx();
			mv.setValue(static_cast<const uint16*>(sourceInfo));
			ev->addMetaData(md, mv);
		}
		eventChannelArray.add(ev);
		moduleEventChannels.add(ev);
	}
	lastNumInputs = getNumInputs();
}


bool PhaseDetector::enable()
{
    return true;
}


void PhaseDetector::handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int sampleNum)
{
    // MOVED GATING TO PULSE PAL OUTPUT!
    // now use to randomize phase for next trial

    //std::cout << "GOT EVENT." << std::endl;

    if (Event::getEventType(event)  == EventChannel::TTL)
    {
		TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, channelInfo);

        // int eventNodeId = *(dataptr+1);
		const int eventId = ttl->getState() ? 1 : 0;
		const int eventChannel = ttl->getChannel();

        for (int i = 0; i < modules.size(); ++i)
        {
            DetectorModule& module = modules.getReference (i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                    module.isActive = true;
                else
                    module.isActive = false;
            }
        }
    }
}


void PhaseDetector::process (AudioSampleBuffer& buffer)
{
    checkForEvents ();

    // loop through the modules
    for (int m = 0; m < modules.size(); ++m)
    {
        DetectorModule& module = modules.getReference (m);

        // check to see if it's active and has a channel
        if (module.isActive && module.outputChan >= 0
            && module.inputChan >= 0
            && module.inputChan < buffer.getNumChannels())
        {
            for (int i = 0; i < getNumSamples (module.inputChan); ++i)
            {
                const float sample = *buffer.getReadPointer (module.inputChan, i);

                if (sample < module.lastSample
                    && sample > 0
                    && module.phase != FALLING_POS)
                {
                    if (module.type == PEAK)
                    {
						uint8 ttlData = 1 << module.outputChan;
						TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
						addEvent(moduleEventChannels[m], event, i);
                        module.samplesSinceTrigger = 0;
                        module.wasTriggered = true;
                    }

                    module.phase = FALLING_POS;
                }
                else if (sample < 0
                         && module.lastSample >= 0
                         && module.phase != FALLING_NEG)
                {
                    if (module.type == FALLING_ZERO)
                    {
						uint8 ttlData = 1 << module.outputChan;
						TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
						addEvent(moduleEventChannels[m], event, i);
                        module.samplesSinceTrigger = 0;
                        module.wasTriggered = true;
                    }

                    module.phase = FALLING_NEG;
                }
                else if (sample > module.lastSample && sample < 0 && module.phase != RISING_NEG)
                {
                    if (module.type == TROUGH)
                    {
						uint8 ttlData = 1 << module.outputChan;
						TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
						addEvent(moduleEventChannels[m], event, i);
                        module.samplesSinceTrigger = 0;
                        module.wasTriggered = true;
                    }

                    module.phase = RISING_NEG;
                }
                else if (sample > 0
                         && module.lastSample <= 0
                         && module.phase != RISING_POS)
                {
                    if (module.type == RISING_ZERO)
                    {
						uint8 ttlData = 1 << module.outputChan;
						TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
						addEvent(moduleEventChannels[m], event, i);
                        module.samplesSinceTrigger = 0;
                        module.wasTriggered = true;
                    }

                    module.phase = RISING_POS;
                }

                module.lastSample = sample;

                if (module.wasTriggered)
                {
                    if (module.samplesSinceTrigger > 1000)
                    {
						uint8 ttlData = 0;
						TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
						addEvent(moduleEventChannels[m], event, i);
                        module.wasTriggered = false;
                    }
                    else
                    {
                        module.samplesSinceTrigger++;
                    }
                }
            }
        }
    }
}


void PhaseDetector::estimateFrequency()
{
}

