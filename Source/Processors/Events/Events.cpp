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

#include "Events.h"
#include "../GenericProcessor/GenericProcessor.h"
#define EVENT_BASE_SIZE 18

//EventBase

EventType EventBase::getBaseType() const
{
	return m_baseType;
}

EventType EventBase::getBaseType(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return static_cast<EventType>(*data);
}

EventBase::EventBase(EventType type, uint64 timestamp)
	: m_baseType(type), m_timestamp(timestamp)
{}

EventBase* EventBase::deserializeFromMessage(const MidiMessage& msg, const GenericProcessor* processor)
{
	if (!processor) return nullptr;

	const uint8* data = msg.getRawData();
	const EventType type = static_cast<EventType>(*data);
	const uint16 processorID = static_cast<uint16>(*(data + 2));
	const uint16 subProcessorID = static_cast<uint16>(*(data + 4));
	const uint16 channelIDX = static_cast<uint16>(*(data + 6));

	switch (type)
	{
	case PROCESSOR_EVENT:
	{		const EventChannel* chan = processor->getEventChannel(processor->getEventChannelIndex(channelIDX, processorID, subProcessorID));
			return Event::deserializeFromMessage(msg, chan);
	}
	case SPIKE_EVENT:
	{
		const SpikeChannel* chan = processor->getSpikeChannel(processor->getSpikeChannelIndex(channelIDX, processorID, subProcessorID));
		return SpikeEvent::deserializeFromMessage(msg, chan);
	}
	default:
		return nullptr;
		break;
	}
}

bool EventBase::compareMetaData(const EventChannel* channelInfo, const MetaDataValueArray& metaData)
{
	int metaDataSize = metaData.size();

	if (metaDataSize != channelInfo->getEventMetaDataCount()) return false;

	for (int i = 0; i < metaDataSize; i++)
	{
		if (!metaData[i]->isOfType(channelInfo->getEventMetaDataDescriptor(i))) return false;
	}
	return true;
}

//Event
EventChannel::EventChannelTypes Event::getEventType() const
{
	return m_eventType;
}

const EventChannel* Event::getChannelInfo() const
{
	return m_channelInfo;
}

EventChannel::EventChannelTypes Event::getEventType(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return static_cast<EventChannel::EventChannelTypes>(*(data + 1));
}

Event::Event(const EventChannel* channelInfo, uint64 timestamp, uint16 channel)
	: EventBase(PROCESSOR_EVENT, timestamp),
	m_channel(channel),
	m_channelInfo(channelInfo),
	m_eventType(channelInfo->getChannelType())
{}

Event* Event::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	EventChannel::EventChannelTypes type = channelInfo->getChannelType();
	
	if (type == EventChannel::TTL)
		return TTLEvent::deserializeFromMessage(msg, channelInfo);
	else if (type == EventChannel::TEXT)
		return TextEvent::deserializeFromMessage(msg, channelInfo);
	else if (type >= EventChannel::INT8_ARRAY && type <= EventChannel::DOUBLE_ARRAY)
		return BinaryEvent::deserializeFromMessage(msg, channelInfo);
	else return nullptr;
}

uint16 Event::getChannel() const
{
	return m_channel;
}

bool Event::serializeHeader(EventChannel::EventChannelTypes type, char* buffer, size_t dstSize) const
{
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	size_t totalSize = eventSize + m_channelInfo->getTotalEventMetaDataSize();
	if (totalSize < dstSize)
	{
		jassertfalse;
		return false;
	}

	*(buffer + 0) = PROCESSOR_EVENT;
	*(buffer + 1) = static_cast<char>(type);
	*(reinterpret_cast<uint16*>(buffer + 2)) = m_channelInfo->getSourceNodeID();
	*(reinterpret_cast<uint16*>(buffer + 4)) = m_channelInfo->getSubProcessorIdx();
	*(reinterpret_cast<uint16*>(buffer + 6)) = m_channelInfo->getSourceIndex();
	*(reinterpret_cast<uint64*>(buffer + 8)) = m_timestamp;
	*(reinterpret_cast<uint64*>(buffer + 16)) = m_channel;
	return true;
}


//TTLEvent

TTLEvent::TTLEvent(const EventChannel* channelInfo, uint64 timestamp, uint16 channel, const void* eventData)
	: Event(channelInfo, timestamp, channel)
{
	size_t size = m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), eventData, size);
}

bool TTLEvent::getState() const
{
	int byteIndex = m_channel / 8;
	int bitIndex = m_channel % 8;

	char data = m_data[byteIndex];
	return ((1 << bitIndex) & data);
}

const void* TTLEvent::getTTLWordPointer() const
{
	return m_data.getData();
}

void TTLEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(EventChannel::TTL, buffer, dstSize))
		return;
	
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	memcpy((buffer + 18), m_data.getData(), dataSize);
	serializeMetaData(buffer + eventSize);
}

TTLEvent* TTLEvent::createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, const void* eventData, uint16 channel)
{

	if (!channelInfo) return nullptr;
	if (channelInfo->getChannelType() != EventChannel::TTL) return nullptr;
	if ((channel < 0) || (channel >= channelInfo->getNumChannels())) return nullptr;
	if (channelInfo->getEventMetaDataCount() != 0) return nullptr;
	
	return new TTLEvent(channelInfo, timestamp, channel, eventData);
}

TTLEvent* TTLEvent::createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, const void* eventData, const MetaDataValueArray& metaData, uint16 channel)
{

	if (!channelInfo) return nullptr;
	if (channelInfo->getChannelType() != EventChannel::TTL) return nullptr;
	if ((channel < 0) || (channel >= channelInfo->getNumChannels())) return nullptr;
	if (!compareMetaData(channelInfo, metaData)) return nullptr;

	TTLEvent* ttl = new TTLEvent(channelInfo, timestamp, channel, eventData);
	
	ttl->m_metaDataValues.addArray(metaData);
	return ttl;
}

TTLEvent* TTLEvent::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	size_t totalSize = msg.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetaDataSize();

	if (totalSize != (dataSize + EVENT_BASE_SIZE + metaDataSize))
	{
		return nullptr;
	}
	const uint8* buffer = msg.getRawData();
	if ((buffer + 0) != PROCESSOR_EVENT) return nullptr;
	if ((buffer + 1) != EventChannel::TTL) return nullptr;

	uint64 timestamp = *(reinterpret_cast<const uint64*>(buffer + 8));
	uint16 channel = *(reinterpret_cast<const uint16*>(buffer + 16));

	ScopedPointer<TTLEvent> ttl = new TTLEvent(channelInfo, timestamp, channel, (buffer + 16));
	bool ret;
	if (metaDataSize > 0)
		 ret = ttl->deserializeMetaData(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return ttl.release();
	else
		return nullptr;
}

//TextEvent
TextEvent::TextEvent(const EventChannel* channelInfo, uint64 timestamp, uint16 channel, const String& text)
	: Event(channelInfo, timestamp, channel),
	m_text(text)
{
}

String TextEvent::getText() const
{
	return m_text;
}

void TextEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(EventChannel::TTL, buffer, dstSize))
		return;

	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	memcpy((buffer + 18), m_data.getData(), dataSize);
	serializeMetaData(buffer + eventSize);

}