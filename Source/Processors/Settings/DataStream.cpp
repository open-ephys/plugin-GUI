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

#include "DataStream.h"

#include "ContinuousChannel.h"
#include "EventChannel.h"
#include "SpikeChannel.h"

uint16 DataStream::nextId = 10000;

DataStream::DataStream(DataStream::Settings settings) 
	: InfoObject(InfoObject::Type::DATASTREAM_INFO),
	source(settings.source),
	m_sample_rate(settings.sample_rate),
	m_channel_count(0)

{
	setName(settings.name);
	setDescription(settings.description);
	setIdentifier(settings.identifier);

	streamId = nextId++;
}

DataStream::~DataStream()
{
	nextId--;
}

void DataStream::clearChannels()
{
	continuousChannels.clear();
	eventChannels.clear();
	spikeChannels.clear();
}

void DataStream::addChannel(InfoObject* channel)
{
	if (channel->getType() == InfoObject::Type::CONTINUOUS_CHANNEL)
	{
		continuousChannels.add((ContinuousChannel*) channel);
		continuousChannels.getLast()->setLocalIndex(continuousChannels.size() - 1);
		continuousChannels.getLast()->stream = this;
		m_channel_count++;
	}

	else if (channel->getType() == InfoObject::Type::EVENT_CHANNEL)
	{
		eventChannels.add((EventChannel*) channel);
		eventChannels.getLast()->setLocalIndex(eventChannels.size() - 1);
		eventChannels.getLast()->stream = this;
	}
		
	else if (channel->getType() == InfoObject::Type::SPIKE_CHANNEL)
	{
		spikeChannels.add((SpikeChannel*) channel);
		spikeChannels.getLast()->setLocalIndex(spikeChannels.size() - 1);
		spikeChannels.getLast()->stream = this;
	}
		
}

float DataStream::getSampleRate() const
{
	return m_sample_rate;
}

int DataStream::getChannelCount() const
{
	return m_channel_count;
}

Array<ContinuousChannel*> DataStream::getContinuousChannels()
{
	return continuousChannels;
}

Array<EventChannel*> DataStream::getEventChannels()
{
	return eventChannels;
}

Array<SpikeChannel*> DataStream::getSpikeChannels()
{
	return spikeChannels;
}