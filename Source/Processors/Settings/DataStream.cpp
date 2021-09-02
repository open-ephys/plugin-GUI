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
	m_sample_rate(settings.sample_rate),
	device(nullptr)
{
	setName(settings.name);
	setDescription(settings.description);
	setIdentifier(settings.identifier);

	streamId = nextId++;
}

DataStream::~DataStream()
{
}

void DataStream::clearChannels()
{
	continuousChannels.clear();
	spikeChannels.clear();
	eventChannels.clear();
}

void DataStream::addChannel(InfoObject* channel)
{
	if (channel->getType() == InfoObject::Type::CONTINUOUS_CHANNEL)
	{
		continuousChannels.add((ContinuousChannel*)channel);
		continuousChannels.getLast()->setLocalIndex(continuousChannels.size() - 1);
	}

	else if (channel->getType() == InfoObject::Type::EVENT_CHANNEL)
	{
		eventChannels.add((EventChannel*)channel);
		eventChannels.getLast()->setLocalIndex(eventChannels.size() - 1);
	}

	else if (channel->getType() == InfoObject::Type::SPIKE_CHANNEL)
	{
		spikeChannels.add((SpikeChannel*)channel);
		spikeChannels.getLast()->setLocalIndex(spikeChannels.size() - 1);
	}
		
}

float DataStream::getSampleRate() const
{
	return m_sample_rate;
}

int DataStream::getChannelCount() const
{
	return continuousChannels.size();
}

uint16 DataStream::getStreamId() const
{
	return streamId;
}

bool DataStream::hasDevice() const
{
	if (device != nullptr)
		return true;
	else
		return false;
}

Array<ContinuousChannel*> DataStream::getContinuousChannels() const
{
	return continuousChannels;
}

Array<EventChannel*> DataStream::getEventChannels() const
{
	return eventChannels;
}

Array<SpikeChannel*> DataStream::getSpikeChannels() const
{
	return spikeChannels;
}

template <class T>
void StreamSettings<T>::update(Array<const DataStream*> streams)
{

	Array<uint16> currentStreamIds;

	for (auto stream : streams)
	{
		currentStreamIds.add(stream->getStreamId());

		if (!settingsMap.contains(streamId))
		{
			settingsArray.add(new T());
			settingsStreamIds.add(streamId);
			settingsMap.set(streamId, settingsArray.getLast());
		}
	}

	Array<T*> settingsToDelete;
	Array<uint16> streamIdsToDelete;

	for (int i = 0; i < settingsArray.size(); i++)
	{
		if (!currentStreamIds.contains(settingsStreamIds[i]))
		{
			settingsToDelete.add(settingsArray[i]);
			streamIdsToDelete.add(settingsStreamIds[i]);
		}
	}

	for (auto settings : settingsToDelete)
	{
		settingsArray.remove(settings);
	}

	for (auto streamId : streamIdsToDelete)
	{
		settingsStreamIds.remove(streamId);
		settingsMap.remove(streamId);
	}
}

template<class T>
T* StreamSettings<T>::operator [](uint16 streamId)
{
	if (settingsMap.contains(streamId))
		return settingsMap[streamId];

	return nullptr;
}