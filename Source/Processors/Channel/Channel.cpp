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

#include "Channel.h"


Channel::Channel(GenericProcessor* p, int n, ChannelType t) : nodeIndex(0), index(n), mappedIndex(0), processor(p), type(t)
{
    reset();
}

void Channel::reset()
{
    createDefaultName();

    nodeId = processor->getNodeId();

    sampleRate = 44100.0f;
    bitVolts = 1.0f;
    sourceNodeId = -1;
    isMonitored = false;
    isEnabled = true;
    recordIndex = -1;
    probeId = -1;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    impedance = 0.0f;
    isRecording = false;

}

Channel::Channel(const Channel& ch)
{
    index = ch.index;
    nodeIndex = ch.nodeIndex;
	mappedIndex = ch.mappedIndex;
    nodeId = ch.nodeId;
    processor = ch.processor;
    sampleRate = ch.sampleRate;
    bitVolts = ch.bitVolts;
    type = ch.type;
    sourceNodeId = ch.sourceNodeId;
    isMonitored = ch.isMonitored;
    isEnabled = ch.isEnabled;
    recordIndex = ch.recordIndex;
    name = ch.name;
    probeId = ch.probeId;
    x = ch.x;
    y = ch.y;
    z = ch.z;
    impedance = ch.impedance;
	extraData = ch.extraData;

    setRecordState(false);
}

float Channel::getBitVolts()
{
    return bitVolts;
}

void Channel::setBitVolts(float bv)
{
    bitVolts = bv;
}


void Channel::setProcessor(GenericProcessor* p)
{
    processor = p;
    nodeId = p->getNodeId();
}

String Channel::getName()
{
    return name;

}

void Channel::setRecordState(bool t)
{

    isRecording = t;
    //std::cout << "Setting record status for channel " <<
    //            nodeId << " - " << num << " to " << t << std::endl;

}

void Channel::setType(ChannelType t)
{
    type = t;
}


ChannelType Channel::getType()
{
    return type;
}

void Channel::setName(String name_)
{
    name = name_;
}

void Channel::createDefaultName()
{
    switch (type)
    {
        case HEADSTAGE_CHANNEL:
            name = String("CH");
            break;
        case AUX_CHANNEL:
            name = String("AUX");
            break;
        case ADC_CHANNEL:
            name = String("ADC");
            break;
        case EVENT_CHANNEL:
            name = String("EVENT");
            break;
        case ELECTRODE_CHANNEL:
            name = String("ELEC");
            break;
        case MESSAGE_CHANNEL:
            name = String("MSG");
    }

    name += index;
}

bool Channel::getRecordState()
{
	return isRecording;
}

ChannelExtraData::ChannelExtraData(void* ptr, int size)
	: dataPtr(ptr), dataSize(size)
{
}

ChannelExtraData::~ChannelExtraData()
{
}