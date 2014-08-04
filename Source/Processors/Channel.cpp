/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


Channel::Channel(GenericProcessor* p, int n) : num(n), eventType(0), processor(p), sampleRate(44100.0), bitVolts(1.0f), isADCchannel(false),isEventChannel(false), isMonitored(false), isEnabled(true), isRecording(false)

{
    nodeId = p->getNodeId();

    createDefaultName();
}

Channel::Channel(const Channel& ch)
{
    processor = ch.processor;
    isEventChannel = ch.isEventChannel;
    isEnabled = ch.isEnabled;
    isMonitored = false;
    isADCchannel = ch.isADCchannel;
    sampleRate = ch.sampleRate;
    bitVolts = ch.bitVolts;
    name = ch.name;
    eventType = ch.eventType;
    nodeId = ch.nodeId;
    num = ch.num;

    setRecordState(false);
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

void Channel::setName(String name_)
{
    name = name_;
}

void Channel::reset()
{
    createDefaultName();

    sampleRate = 44100.0f;
    bitVolts = 1.0f;

}

void Channel::createDefaultName()
{
    name = String("CH");
    name += (num + 1);
}
