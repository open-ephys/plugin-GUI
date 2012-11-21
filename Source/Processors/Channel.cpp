/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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


Channel::Channel(GenericProcessor* p, int n) :
	processor(p), num(n), 
	isEventChannel(false), isEnabled(true), isRecording(false), isMonitored(false), 
	sampleRate(44100.0f), bitVolts(1.0f), eventType(0)

{
	nodeId = p->getNodeId();

	createDefaultName();
}

Channel::Channel(const Channel& ch)
{
	processor = ch.processor;
	isEventChannel = ch.isEventChannel;
	isEnabled = ch.isEnabled;
	isRecording = false;
	isMonitored = false;
	sampleRate = ch.sampleRate;
	bitVolts = ch.bitVolts;
	name = ch.name;
	eventType = ch.eventType;
	nodeId = ch.nodeId;
	num = ch.num;
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