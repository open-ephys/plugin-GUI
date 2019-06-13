/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

    Modified by:

    Alessio Buccino     alessiob@ifi.uio.no
    Mikkel Lepperod
    Svenn-Arne Dragly

    Center for Integrated Neuroplasticity CINPLA
    Department of Biosciences
    University of Oslo
    Norway

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

#include "PulsePalOutput.h"


PulsePalOutput::PulsePalOutput()
    : GenericProcessor ("Pulse Pal")
    , channelToChange (0)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    // Init Pulse Pal
    pulsePal.initialize();
    pulsePal.setDefaultParameters();
    pulsePal.updateDisplay("PulsePal Output GUI Connected","Click for menu");
    pulsePalVersion = pulsePal.getFirmwareVersion();

    // Init Pulsa Pal parameter arrays
    m_isBiphasic = vector<int>(PULSEPALCHANNELS, 0);
    m_phase1Duration = vector<float>(PULSEPALCHANNELS, DEF_PHASE_DURATION);
    m_phase2Duration = vector<float>(PULSEPALCHANNELS, DEF_PHASE_DURATION);
    m_interPhaseInterval = vector<float>(PULSEPALCHANNELS, DEF_INTER_PHASE);
    m_phase1Voltage = vector<float>(PULSEPALCHANNELS, DEF_VOLTAGE);
    m_phase2Voltage = vector<float>(PULSEPALCHANNELS, DEF_VOLTAGE);
    m_restingVoltage = vector<float>(PULSEPALCHANNELS, 0);
    m_interPulseInterval = vector<float>(PULSEPALCHANNELS, DEF_INTER_PULSE);
    m_burstDuration = vector<float>(PULSEPALCHANNELS, 0);
    m_interBurstInterval = vector<float>(PULSEPALCHANNELS, 0);
    m_trainDuration = vector<float>(PULSEPALCHANNELS, DEF_TRAINDURATION);
    m_trainDelay = vector<float>(PULSEPALCHANNELS, 0);
    m_linkTriggerChannel1 = vector<int>(PULSEPALCHANNELS, 0);
    m_linkTriggerChannel2 = vector<int>(PULSEPALCHANNELS, 0);
    m_triggerMode = vector<int>(PULSEPALCHANNELS, 0);

    for (int i = 0; i < PULSEPALCHANNELS; ++i)
    {
        channelTtlTrigger.add   (-1);
        channelTtlGate.add      (-1);
        channelState.add        (true);
    }
}

PulsePalOutput::~PulsePalOutput()
{
    pulsePal.updateDisplay ("PULSE PAL v1.0","Click for menu");
}

AudioProcessorEditor* PulsePalOutput::createEditor()
{
    editor = new PulsePalOutputEditor (this, &pulsePal, true);
    return editor;
}

void PulsePalOutput::handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
{
    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
        const int state         = ttl->getState() ? 1 : 0;
        const int eventId       = ttl->getSourceIndex();
        const int sourceId      = ttl->getSourceID();
        const int eventChannel  = ttl->getChannel();

        for (int i = 0; i < PULSEPALCHANNELS; ++i)
        {
            if (channelTtlTrigger[i] != -1)
            {
                EventSources s = sources.getReference (channelTtlTrigger[i]);
                if (eventId == s.eventIndex && sourceId == s.sourceId
                        && eventChannel == s.channel && state)
                {
                    std::cout << "Trigger " << i + 1 << std::endl;
                    pulsePal.triggerChannel (i + 1);
                }
            }
            if (channelTtlGate[i] != -1)
            {
                EventSources s = sources.getReference (channelTtlGate[i]);
                if (eventId == s.eventIndex && sourceId == s.sourceId
                        && eventChannel == s.channel)
                {
                    std::cout << "Gate " << i + 1 << std::endl;
                    if (state == 1)
                        channelState.set (i, true);
                    else
                        channelState.set (i, false);
                }
            }
        }
    }
}


void PulsePalOutput::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);

    switch (parameterIndex)
    {
    case 0:
        channelToChange = (int) newValue;
        break;

    case 1:
        channelTtlTrigger.set (channelToChange, (int) newValue);
        break;

    case 2:
        channelTtlGate.set (channelToChange, (int) newValue);

        if (newValue < 0)
        {
            channelState.set (channelToChange, true);
        }
        else
        {
            channelState.set (channelToChange, false);
        }
        break;

    default:
        std::cout << "Unrecognized parameter index." << std::endl;
    }
}

void PulsePalOutput::process (AudioSampleBuffer& buffer)
{
    checkForEvents ();
}

void PulsePalOutput::addEventSource(EventSources s)
{
    sources.add (s);
}

void PulsePalOutput::clearEventSources()
{
    sources.clear();
}

bool PulsePalOutput::updatePulsePal(int chan)
{
    // check that Pulspal is connected and update parameters for channel chan+1
    if (pulsePalVersion != 0)
    {
        int actual_chan = chan + 1;
        pulsePal.setBiphasic(actual_chan, m_isBiphasic[chan]);
        pulsePal.setPhase1Duration(actual_chan, float(m_phase1Duration[chan])/1000);
        pulsePal.setPhase2Duration(actual_chan, float(m_phase2Duration[chan])/1000);
        pulsePal.setInterPhaseInterval(actual_chan, float(m_interPhaseInterval[chan])/1000);
        pulsePal.setPhase1Voltage(actual_chan, float(m_phase1Voltage[chan]));
        pulsePal.setPhase2Voltage(actual_chan, float(m_phase2Voltage[chan]));
        pulsePal.setRestingVoltage(actual_chan, float(m_restingVoltage[chan]));
        pulsePal.setInterPulseInterval(actual_chan, float(m_interPulseInterval[chan])/1000);
        pulsePal.setBurstDuration(actual_chan, float(m_burstDuration[chan])/1000);
        pulsePal.setBurstInterval(actual_chan, float(m_interBurstInterval[chan])/1000);
        pulsePal.setPulseTrainDuration(actual_chan, float(m_trainDuration[chan])/1000);
        pulsePal.setPulseTrainDelay(actual_chan, float(m_trainDelay[chan])/1000);

        pulsePal.setTrigger1Link(actual_chan, m_linkTriggerChannel1[chan]);
        pulsePal.setTrigger2Link(actual_chan, m_linkTriggerChannel2[chan]);
        pulsePal.setTriggerMode(actual_chan, m_triggerMode[chan]);
        return true;
    }
    else
        return false;
}

bool PulsePalOutput::getIsBiphasic(int chan) const
{
    if (m_isBiphasic[chan])
        return true;
    else
        return false;
}

float PulsePalOutput::getPhase1Duration(int chan) const
{
    return m_phase1Duration[chan];
}

float PulsePalOutput::getPhase2Duration(int chan) const
{
    return m_phase2Duration[chan];
}

float PulsePalOutput::getInterPhaseInt(int chan) const
{
    return m_interPhaseInterval[chan];
}

float PulsePalOutput::getVoltage1(int chan) const
{
    return m_phase1Voltage[chan];
}

float PulsePalOutput::getVoltage2(int chan) const
{
    return m_phase2Voltage[chan];
}

float PulsePalOutput::getRestingVoltage(int chan) const
{
    return m_restingVoltage[chan];
}

float PulsePalOutput::getInterPulseInt(int chan) const
{
    return m_interPulseInterval[chan];
}

float PulsePalOutput::getBurstDuration(int chan) const
{
    return m_burstDuration[chan];
}

float PulsePalOutput::getInterBurstInt(int chan) const
{
    return m_interBurstInterval[chan];
}

float PulsePalOutput::getTrainDuration(int chan) const
{
    return m_trainDuration[chan];
}

float PulsePalOutput::getTrainDelay(int chan) const
{
    return m_trainDelay[chan];
}

int PulsePalOutput::getLinkTriggerChannel1(int chan) const
{
    return m_linkTriggerChannel1[chan];
}

int PulsePalOutput::getLinkTriggerChannel2(int chan) const
{
    return m_linkTriggerChannel2[chan];
}

int PulsePalOutput::getTriggerMode(int chan) const
{
    return m_triggerMode[chan];
}

uint32_t PulsePalOutput::getPulsePalVersion() const
{
    return pulsePalVersion;
}

void PulsePalOutput::setIsBiphasic(int chan, bool isBiphasic)
{
    if (isBiphasic)
        m_isBiphasic[chan] = 1;
    else
        m_isBiphasic[chan] = 0;
}

void PulsePalOutput::setPhase1Duration(int chan, float phaseDuration)
{
    m_phase1Duration[chan] = phaseDuration;
}

void PulsePalOutput::setPhase2Duration(int chan, float phaseDuration)
{
    m_phase2Duration[chan] = phaseDuration;
}

void PulsePalOutput::setInterPhaseInt(int chan, float interPhaseInt)
{
    m_interPhaseInterval[chan] = interPhaseInt;
}

void PulsePalOutput::setVoltage1(int chan, float voltage)
{
    m_phase1Voltage[chan] = voltage;
}

void PulsePalOutput::setVoltage2(int chan, float voltage)
{
    m_phase2Voltage[chan] = voltage;
}

void PulsePalOutput::setRestingVoltage(int chan, float voltage)
{
    m_restingVoltage[chan] = voltage;
}

void PulsePalOutput::setInterPulseInt(int chan, float interPulseInt)
{
    m_interPulseInterval[chan] = interPulseInt;
}

void PulsePalOutput::setBurstDuration(int chan, float burstDuration)
{
    m_burstDuration[chan] = burstDuration;
}

void PulsePalOutput::setInterBurstInt(int chan, float interBurstInt)
{
    m_interBurstInterval[chan] = interBurstInt;
}

void PulsePalOutput::setTrainDuration(int chan, float trainDuration)
{
    m_trainDuration[chan] = trainDuration;
}

void PulsePalOutput::setTrainDelay(int chan, float trainDelay)
{
    m_trainDelay[chan] = trainDelay;
}

void PulsePalOutput::setLinkTriggerChannel1(int chan, int link)
{
    m_linkTriggerChannel1[chan] = link;
}

void PulsePalOutput::setLinkTriggerChannel2(int chan, int link)
{
    m_linkTriggerChannel2[chan] = link;
}

void PulsePalOutput::setTriggerMode(int chan, int mode)
{
    m_triggerMode[chan] = mode;
}

void PulsePalOutput::setTTLsettings(int chan)
{
    m_isBiphasic[chan] = 0;
    m_phase1Duration[chan] = 1;
    m_phase2Duration[chan] = 0;
    m_phase1Voltage[chan] = 5;
    m_phase2Voltage[chan] = 0;
    m_restingVoltage[chan] = 0;
    m_interPhaseInterval[chan] = 0;
    m_interPulseInterval[chan] = 1;
    m_burstDuration[chan] = 0;
    m_interBurstInterval[chan] = 0;
    m_trainDuration[chan] = 2;
    m_trainDelay[chan] = 0;
}

bool PulsePalOutput::checkParameterConsistency(int chan)
{
    bool consistent;

    if (!m_isBiphasic[chan])
        if (m_burstDuration[chan] != 0)
            consistent = (m_phase1Duration[chan] + m_interPulseInterval[chan] < m_burstDuration[chan]) &&
                    (m_burstDuration[chan] + m_interBurstInterval[chan] < m_trainDuration[chan]);
        else
            consistent = (m_phase1Duration[chan] <= m_trainDuration[chan]);
    else
        if (m_burstDuration[chan] != 0)
            consistent =  (m_phase1Duration[chan] + m_phase2Duration[chan] + m_interPhaseInterval[chan]
                           + m_interPulseInterval[chan] < m_burstDuration[chan]) &&
                    (m_burstDuration[chan] + m_interBurstInterval[chan] < m_trainDuration[chan]);
        else
            consistent =  (m_phase1Duration[chan] + m_phase2Duration[chan] + m_interPhaseInterval[chan]
                           <= m_trainDuration[chan]);

    return consistent;
}

void PulsePalOutput::adjustParameters(int chan)
{
    if (!m_isBiphasic[chan])
        if (m_burstDuration[chan] != 0)
        {
            m_burstDuration[chan] = m_phase1Duration[chan] + m_interPulseInterval[chan] + 1;
            m_trainDuration[chan] = m_burstDuration[chan] + m_interBurstInterval[chan] + 1;
        }
        else
            m_trainDuration[chan] = m_phase1Duration[chan] + m_interPulseInterval[chan] + 1;
    else
        if (m_burstDuration[chan] != 0)
        {
            m_burstDuration[chan] = m_phase1Duration[chan] + m_phase2Duration[chan] + m_interPhaseInterval[chan]
                    + m_interPulseInterval[chan] + 1;
            m_trainDuration[chan] = m_burstDuration[chan] + m_interBurstInterval[chan] + 1;
        }
        else
            m_trainDuration[chan] =  m_phase1Duration[chan] + m_phase2Duration[chan] + m_interPhaseInterval[chan]
                    + m_interPulseInterval[chan] + 1;
}

void PulsePalOutput::saveCustomParametersToXml(XmlElement *parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("PulsePalOutput");
    for (int i=0; i<PULSEPALCHANNELS; i++)
    {
        XmlElement* chan = new XmlElement(String("Channel_")+=String(i+1));
        chan->setAttribute("id", i);
        chan->setAttribute("biphasic", m_isBiphasic[i]);
        chan->setAttribute("phase1", m_phase1Duration[i]);
        chan->setAttribute("phase2", m_phase1Duration[i]);
        chan->setAttribute("interphase", m_interPhaseInterval[i]);
        chan->setAttribute("voltage1", m_phase1Voltage[i]);
        chan->setAttribute("voltage2", m_phase2Voltage[i]);
        chan->setAttribute("restingvoltage", m_restingVoltage[i]);
        chan->setAttribute("interpulse", m_interPulseInterval[i]);
        chan->setAttribute("burstduration", m_burstDuration[i]);
        chan->setAttribute("interburst", m_interBurstInterval[i]);
        chan->setAttribute("trainduration", m_trainDuration[i]);
        chan->setAttribute("traindelay", m_trainDelay[i]);
        chan->setAttribute("link2trigger1", m_linkTriggerChannel1[i]);
        chan->setAttribute("link2trigger2", m_linkTriggerChannel2[i]);
        chan->setAttribute("triggermode", m_triggerMode[i]);
        mainNode->addChildElement(chan);
    }
}

void PulsePalOutput::loadCustomParametersFromXml ()
{
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement (*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName ("PulsePalOutput"))
            {
                forEachXmlChildElement(*mainNode, chan)
                {
                    int id = chan->getIntAttribute("id");
                    int biphasic = chan->getIntAttribute("biphasic");
                    double phase1 = chan->getDoubleAttribute("phase1");
                    double phase2 = chan->getDoubleAttribute("phase2");
                    double interphase = chan->getDoubleAttribute("interphase");
                    double voltage1 = chan->getDoubleAttribute("voltage1");
                    double voltage2 = chan->getDoubleAttribute("voltage2");
                    double resting = chan->getDoubleAttribute("restingvoltage");
                    double interpulse = chan->getDoubleAttribute("interpulse");
                    double burst = chan->getDoubleAttribute("burstduration");
                    double interburst = chan->getDoubleAttribute("interburst");
                    double trainduration = chan->getDoubleAttribute("trainduration");
                    double traindelay = chan->getDoubleAttribute("traindelay");
                    int link21 = chan->getIntAttribute("link2trigger1");
                    int link22 = chan->getIntAttribute("link2trigger2");
                    int trigger = chan->getIntAttribute("triggermode");
                    m_isBiphasic[id] = biphasic;
                    m_phase1Duration[id] = phase1;
                    m_phase2Duration[id] = phase2;
                    m_interPhaseInterval[id] = interphase;
                    m_phase1Voltage[id] = voltage1;
                    m_phase2Voltage[id] = voltage2;
                    m_restingVoltage[id] = resting;
                    m_interPulseInterval[id] = interpulse;
                    m_burstDuration[id] = burst;
                    m_interBurstInterval[id] = interburst;
                    m_trainDuration[id] = trainduration;
                    m_trainDelay[id] = traindelay;
                    m_linkTriggerChannel1[id] = link21;
                    m_linkTriggerChannel2[id] = link22;
                    m_triggerMode[id] = trigger;
                }
            }
        }
    }
}
