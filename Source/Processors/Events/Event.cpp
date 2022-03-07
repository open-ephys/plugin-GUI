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

#include "Event.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "Spike.h"

EventBase::~EventBase() {}

Event::Type EventBase::getBaseType() const
{
	return m_baseType;
}

Event::Type EventBase::getBaseType(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return static_cast<Event::Type> (*data);
}

Event::Type EventBase::getBaseType(const uint8* data)
{
	return static_cast<Event::Type> (*data);
}

uint16 EventBase::getProcessorId() const
{
	return m_sourceProcessorId;
}

uint16 EventBase::getProcessorId(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return *reinterpret_cast<const uint16*>(data + 2);
}

uint16 EventBase::getProcessorId(const uint8* data)
{
	return *reinterpret_cast<const uint16*>(data + 2);
}

uint16 EventBase::getStreamId() const
{
	return m_sourceStreamId;
}

uint16 EventBase::getStreamId(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return *reinterpret_cast<const uint16*>(data + 4);
}


uint16 EventBase::getStreamId(const uint8* data)
{
	return *reinterpret_cast<const uint16*>(data + 4);
}

uint16 EventBase::getChannelIndex() const
{
	return m_sourceChannelIndex;
}

uint16 EventBase::getChannelIndex(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return *reinterpret_cast<const uint16*>(data + 6);
}

uint16 EventBase::getChannelIndex(const uint8* data)
{
	return *reinterpret_cast<const uint16*>(data + 6);
}


juce::int64 EventBase::getTimestamp() const
{
	return m_timestamp;
}

juce::int64 EventBase::getTimestamp(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return *reinterpret_cast<const juce::int64*>(data + 8);
}

EventBase::EventBase(Event::Type type, 
	juce::int64 timestamp, 
	uint16 processorId, 
	uint16 streamId, 
	uint16 channelIndex) : 
	m_baseType(type), 
	m_timestamp(timestamp), 
	m_sourceProcessorId(processorId), 
	m_sourceStreamId(streamId), 
	m_sourceChannelIndex(channelIndex) {}


EventBasePtr EventBase::deserialize(const EventPacket& packet, const GenericProcessor* processor)
{
	if (!processor) return nullptr;

	const uint8* data = packet.getRawData();

	const Type type = static_cast<Type>(*data);
	const uint16 processorId = static_cast<uint16>(*(data + 2));
	const uint16 streamId = static_cast<uint16>(*(data + 4));
	const uint16 channelIndex = static_cast<uint16>(*(data + 6));

	switch (type)
	{
	case PROCESSOR_EVENT:
	{		
		const EventChannel* eventChannel = processor->getEventChannel(processorId, streamId, channelIndex);
		return Event::deserialize(packet, eventChannel).release();
	}
	case SPIKE_EVENT:
	{
		const SpikeChannel* spikeChannel = processor->getSpikeChannel(processorId, streamId, channelIndex);
		return Spike::deserialize(packet, spikeChannel).release();
	}
	default:
		return nullptr;
		break;
	}
}

bool EventBase::compareMetadata(const MetadataEventObject* channelInfo, const MetadataValueArray& metaData)
{
	int metaDataSize = metaData.size();

	if (metaDataSize != channelInfo->getEventMetadataCount()) return false;

	for (int i = 0; i < metaDataSize; i++)
	{
		if (!metaData[i]->isOfType(channelInfo->getEventMetadataDescriptor(i))) return false;
	}
	return true;
}


SystemEvent::Type SystemEvent::getSystemEventType(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();

	return static_cast<Type>(*(data + 1));
}

size_t SystemEvent::fillTimestampAndSamplesData(HeapBlock<char>& data, 
	const GenericProcessor* proc, 
	uint16 streamId, 
	juce::int64 timestamp, 
	uint32 nSamples,
	juce::int64 processStartTime)
{
	const int eventSize = 28;
	data.malloc(eventSize);
	data[0] = SYSTEM_EVENT;													 // 1 byte
	data[1] = TIMESTAMP_AND_SAMPLES;										 // 1 byte
	*reinterpret_cast<uint16*>(data.getData() + 2) = proc->getNodeId();		 // 2 bytes
	*reinterpret_cast<uint16*>(data.getData() + 4) = streamId;				 // 2 bytes
	data[6] = 0;															 // 1 byte 
	data[7] = 0;															 // 1 byte
	*reinterpret_cast<juce::int64*>(data.getData() + 8) = timestamp;		 // 8 bytes
	*reinterpret_cast<uint32*>(data.getData() + 16) = nSamples;				 // 8 bytes
	*reinterpret_cast<juce::int64*>(data.getData() + 20) = processStartTime; // 8 bytes
	return eventSize;
}

size_t SystemEvent::fillTimestampSyncTextData(HeapBlock<char>& data, 
	const GenericProcessor* proc, 
	uint16 streamId, 
	juce::int64 timestamp, 
	bool softwareTime)
{
	String eventString;
	if (softwareTime)
	{
		eventString = "Software Time (milliseconds since midnight Jan 1st 1970 UTC)";
	}
	else
	{
		eventString = "Start Time for " 
			+ proc->getName()
			+ " ("
			+ String(proc->getNodeId())
			+ ") - "
			+ String(proc->getDataStream(streamId)->getName())
			+" @ "
			+ String(proc->getSampleRate(streamId))
			+ " Hz";
	}
	size_t textSize = eventString.getNumBytesAsUTF8();
	size_t dataSize = 17 + textSize;
	data.allocate(dataSize, true);
	data[0] = SYSTEM_EVENT;
	data[1] = TIMESTAMP_SYNC_TEXT;
	*reinterpret_cast<uint16*>(data.getData() + 2) = proc->getNodeId();
	*reinterpret_cast<uint16*>(data.getData() + 4) = streamId;
	*reinterpret_cast<juce::int64*>(data.getData() + 8) = timestamp;
	memcpy(data.getData() + 16, eventString.toUTF8(), textSize);
	return dataSize;
}

uint32 SystemEvent::getNumSamples(const EventPacket& packet)
{
	if (getBaseType(packet) != SYSTEM_EVENT && getSystemEventType(packet) != TIMESTAMP_AND_SAMPLES)
		return 0;

	return *reinterpret_cast<const uint32*>(packet.getRawData() + 16);
}

int64 SystemEvent::getHiResTicks(const EventPacket& packet)
{
	if (getBaseType(packet) != SYSTEM_EVENT && getSystemEventType(packet) != TIMESTAMP_AND_SAMPLES)
		return 0;

	return *reinterpret_cast<const int64*>(packet.getRawData() + 20);
}

String SystemEvent::getSyncText(const EventPacket& packet)
{
	if (getBaseType(packet) != SYSTEM_EVENT && getSystemEventType(packet) != TIMESTAMP_SYNC_TEXT)
		return String();

	const char* data = reinterpret_cast<const char*>(packet.getRawData());

	return String::fromUTF8(data + 16, packet.getRawDataSize() - 17);
}

Event::Event(const Event& other)
	: EventBase(other),
	m_channelInfo(other.m_channelInfo),
	m_eventType(other.m_eventType)
{
	size_t size = other.m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), other.m_data.getData(), size);
}

EventChannel::Type Event::getEventType() const
{
	return m_eventType;
}

const EventChannel* Event::getChannelInfo() const
{
	return m_channelInfo;
}

EventChannel::Type Event::getEventType(const EventPacket& packet)
{
	const uint8* data = packet.getRawData();

	return static_cast<EventChannel::Type>(*(data + 1));
}

Event::Event(const EventChannel* channelInfo, juce::int64 timestamp)
	: EventBase(PROCESSOR_EVENT, 
		timestamp, 
		channelInfo->getSourceNodeId(), 
		channelInfo->getStreamId(), 
		channelInfo->getLocalIndex()),
	m_channelInfo(channelInfo),
	m_eventType(channelInfo->getType())
{}

Event::~Event() {}

EventPtr Event::deserialize(const EventPacket& packet, const EventChannel* eventChannel)
{
	EventChannel::Type type = eventChannel->getType();
	
	if (type == EventChannel::TTL)
		return TTLEvent::deserialize(packet, eventChannel).release();
	else if (type == EventChannel::TEXT)
		return TextEvent::deserialize(packet, eventChannel).release();
	else if (type >= EventChannel::INT8_ARRAY && type <= EventChannel::DOUBLE_ARRAY)
		return BinaryEvent::deserialize(packet, eventChannel).release();
	else return nullptr;
}


bool Event::serializeHeader(EventChannel::Type type, char* buffer, size_t dstSize) const
{
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	size_t totalSize = eventSize + m_channelInfo->getTotalEventMetadataSize();
	if (totalSize < dstSize)
	{
		jassertfalse;
		return false;
	}

	*(buffer + 0) = PROCESSOR_EVENT;
	*(buffer + 1) = static_cast<char>(type);
	*(reinterpret_cast<uint16*>(buffer + 2)) = m_channelInfo->getSourceNodeId();
	*(reinterpret_cast<uint16*>(buffer + 4)) = m_channelInfo->getStreamId();
	*(reinterpret_cast<uint16*>(buffer + 6)) = m_channelInfo->getLocalIndex();
	*(reinterpret_cast<juce::int64*>(buffer + 8)) = m_timestamp;
	return true;
}

bool Event::createChecks(const EventChannel* channelInfo, EventChannel::Type eventType)
{
	if (!channelInfo) return false;
	if (channelInfo->getType() != eventType) return false;
	if (channelInfo->getEventMetadataCount() != 0) return false;
	return true;
}

bool Event::createChecks(const EventChannel* channelInfo, EventChannel::Type eventType, const MetadataValueArray& metaData)
{
	if (!channelInfo) return false;
	if (channelInfo->getType() != eventType) return false;
	if (!compareMetadata(channelInfo, metaData)) return false;
	return true;
}

const void* Event::getRawDataPointer() const
{
	return m_data.getData();
}

TTLEvent::TTLEvent(const TTLEvent& other)
	: Event(other)
{
}

TTLEvent::TTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, const void* eventData)
	: Event(channelInfo, timestamp)
{
	size_t size = m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), eventData, size);
}

TTLEvent::~TTLEvent() {}

bool TTLEvent::getState() const
{
	uint8 state_byte = m_data[1];

	return state_byte == 1;
}

bool TTLEvent::getState(const EventPacket& packet)
{
	uint8 state_byte = *reinterpret_cast<const uint8*>(packet.getRawData() + EVENT_BASE_SIZE + 1);

	return state_byte == 1;
}

uint8 TTLEvent::getLine() const
{
	return m_data[0];
}

uint8 TTLEvent::getLine(const EventPacket& packet)
{
	return *reinterpret_cast<const uint8*>(packet.getRawData() + EVENT_BASE_SIZE);
}

uint64 TTLEvent::getWord() const
{
	return *(reinterpret_cast<const uint64*>(m_data + 2));
}

void TTLEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(EventChannel::TTL, buffer, dstSize))
		return;
	
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	/*std::cout << "SERIALIZE!" << std::endl;
	std::cout << eventSize << std::endl;
	std::cout << dataSize << std::endl;
	std::cout << getWord() << std::endl;*/
	memcpy((buffer + EVENT_BASE_SIZE), m_data.getData(), dataSize);
	serializeMetadata(buffer + eventSize);
}

TTLEventPtr TTLEvent::createTTLEvent(EventChannel* channelInfo,
	juce::int64 timestamp,
	uint8 line,
	bool state)
{
	uint8 data[10];

	data[0] = line;
	data[1] = state;

	channelInfo->setLineState(line, state);
	*reinterpret_cast<uint64*>(data + 2) = channelInfo->getTTLWord();

	return new TTLEvent(channelInfo, timestamp, data);
}

TTLEventPtr TTLEvent::createTTLEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	uint8 line,
	bool state,
	uint64 word)
{
	uint8 data[10];

	data[0] = line;
	data[1] = state;
	*reinterpret_cast<uint64*>(data + 2) = word;
	
	return new TTLEvent(channelInfo, timestamp, data);
}

TTLEventPtr TTLEvent::createTTLEvent(EventChannel* channelInfo, 
	juce::int64 timestamp, 
	uint8 line,
	bool state,
	const MetadataValueArray& metaData)
{

	uint8 data[10];

	data[0] = line;
	data[1] = state;
	channelInfo->setLineState(line, state);
	*reinterpret_cast<uint64*>(data + 2) = channelInfo->getTTLWord();

	TTLEventPtr event = new TTLEvent(channelInfo, timestamp, data);
	
	event->m_metaDataValues.addArray(metaData);

	return event;
}

TTLEventPtr TTLEvent::deserialize(const uint8* buffer, const EventChannel* channelInfo)
{

	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetadataSize();

	if (static_cast<Event::Type>(*(buffer + 0)) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getType() != EventChannel::TTL)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<EventChannel::Type>(*(buffer + 1)) != EventChannel::TTL) {
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeId())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getStreamId())
	{

		uint16 left = *reinterpret_cast<const uint16*>(buffer + 4);
		uint16 right = channelInfo->getStreamId();
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getLocalIndex())
	{
		uint16 left = *reinterpret_cast<const uint16*>(buffer + 6);
		uint16 right = channelInfo->getLocalIndex();
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));

	ScopedPointer<TTLEvent> event = new TTLEvent(channelInfo,
		timestamp,
		(buffer + EVENT_BASE_SIZE));

	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetadata(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}
}

TTLEventPtr TTLEvent::deserialize(const EventPacket& packet, const EventChannel* channelInfo)
{
	
	return deserialize(packet.getRawData(), channelInfo);
	
}

TextEvent::TextEvent(const EventChannel* channelInfo, juce::int64 timestamp, const String& text)
	: Event(channelInfo, timestamp)
{
	m_data.calloc(channelInfo->getDataSize());
	text.copyToUTF8(m_data.getData(), channelInfo->getDataSize());
}

TextEvent::~TextEvent() {}

TextEvent::TextEvent(const TextEvent& other)
	: Event(other)
{
}

String TextEvent::getText() const
{
	return String(m_data.getData(), m_channelInfo->getLength());
}

void TextEvent::serialize(void* destinationBuffer, size_t bufferSize) const
{
	char* buffer = static_cast<char*>(destinationBuffer);

	if (!serializeHeader(EventChannel::TEXT, buffer, bufferSize))
		return;

	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;

	memcpy((buffer + EVENT_BASE_SIZE), m_data, dataSize);

	serializeMetadata(buffer + eventSize);
}

TextEventPtr TextEvent::createTextEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	const String& text)
{

	if (text.getNumBytesAsUTF8() > channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	return new TextEvent(channelInfo, timestamp, text);
}

TextEventPtr TextEvent::createTextEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	const String& text, 
	const MetadataValueArray& metaData)
{
	if (text.getNumBytesAsUTF8() > channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	TextEvent* event = new TextEvent(channelInfo, timestamp, text);
	
	event->m_metaDataValues.addArray(metaData);
	return event;
}

TextEventPtr TextEvent::deserialize(const uint8* buffer, const EventChannel* channelInfo)
{
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetadataSize();

	if (static_cast<Event::Type> (*(buffer + 0)) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getType() != EventChannel::TEXT)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<EventChannel::Type>(*(buffer + 1)) != EventChannel::TEXT) {
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeId())
	{
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 channel = *(reinterpret_cast<const uint16*>(buffer + 16));
	String text = String::fromUTF8(reinterpret_cast<const char*>(buffer + EVENT_BASE_SIZE), dataSize);

	ScopedPointer<TextEvent> event = new TextEvent(channelInfo, timestamp, text);

	bool ret = true;

	if (metaDataSize > 0)
		ret = event->deserializeMetadata(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}

}

TextEventPtr TextEvent::deserialize(const EventPacket& packet, const EventChannel* channelInfo)
{

	return deserialize(packet.getRawData(), channelInfo);

}

BinaryEvent::BinaryEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	const void* data,
	EventChannel::BinaryDataType type)
	: Event(channelInfo, timestamp),
	m_type(type)
{
	size_t size = m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), data, size);
}

BinaryEvent::BinaryEvent(const BinaryEvent& other)
	:Event(other),
	m_type(other.m_type)
{
}

BinaryEvent::~BinaryEvent() {}

const void* BinaryEvent::getBinaryDataPointer() const
{
	return m_data.getData();
}

EventChannel::BinaryDataType BinaryEvent::getBinaryType() const
{
	return m_type;
}

template<typename T>
EventChannel::BinaryDataType BinaryEvent::getType()
{
	if (std::is_same<int8, T>::value) return EventChannel::INT8_ARRAY;
	if (std::is_same<uint8, T>::value) return EventChannel::UINT8_ARRAY;
	if (std::is_same<int16, T>::value) return EventChannel::INT16_ARRAY;
	if (std::is_same<uint16, T>::value) return EventChannel::UINT16_ARRAY;
	if (std::is_same<int32, T>::value) return EventChannel::INT32_ARRAY;
	if (std::is_same<uint32, T>::value) return EventChannel::UINT32_ARRAY;
	if (std::is_same<juce::int64, T>::value) return EventChannel::INT64_ARRAY;
	if (std::is_same<juce::uint64, T>::value) return EventChannel::UINT64_ARRAY;
	if (std::is_same<float, T>::value) return EventChannel::FLOAT_ARRAY;
	if (std::is_same<double, T>::value) return EventChannel::DOUBLE_ARRAY;

	return EventChannel::BINARY_BASE_VALUE;
}

void BinaryEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(m_channelInfo->getType(), buffer, dstSize))
		return;
	
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	memcpy((buffer + EVENT_BASE_SIZE), m_data.getData(), dataSize);
	serializeMetadata(buffer + eventSize);
}

template<typename T>
BinaryEventPtr BinaryEvent::createBinaryEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	const T* data, 
	int dataSize)
{
	EventChannel::BinaryDataType type = getType<T>();

	if (dataSize < channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	return new BinaryEvent(channelInfo, timestamp, data, type);
}

template<typename T>
BinaryEventPtr BinaryEvent::createBinaryEvent(const EventChannel* channelInfo, 
	juce::int64 timestamp, 
	const T* data, 
	int dataSize, 
	const MetadataValueArray& metaData)
{
	EventChannel::BinaryDataType type = getType<T>();

	if (dataSize < channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	BinaryEvent* event = new BinaryEvent(channelInfo, timestamp, data, type);
	event->m_metaDataValues.addArray(metaData);
	return event;
}

BinaryEventPtr BinaryEvent::deserialize(const EventPacket& packet, const EventChannel* channelInfo)
{
	size_t totalSize = packet.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetadataSize();

	if (totalSize != (dataSize + EVENT_BASE_SIZE + metaDataSize))
	{
		jassertfalse;
		return nullptr;
	}
	const uint8* buffer = packet.getRawData();
	//TODO: remove the mask when the probe system is implemented
	if (static_cast<Event::Type>(*(buffer + 0)&0x7F) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getType() < EventChannel::BINARY_BASE_VALUE || channelInfo->getType() >= EventChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	const EventChannel::Type type = static_cast<const EventChannel::Type>(*(buffer + 1));
	if (type != channelInfo->getType())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeId())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getStreamId())
	{
		jassertfalse;
		return nullptr;
	}
	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getLocalIndex())
	{
		jassertfalse;
		return nullptr;
	}
	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));

	ScopedPointer<BinaryEvent> event = new BinaryEvent(channelInfo, 
		timestamp, 
		(buffer + EVENT_BASE_SIZE), 
		channelInfo->getBinaryDataType());

	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetadata(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}
}

//Template definitions
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int8>(const EventChannel*, juce::int64, const int8* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint8>(const EventChannel*, juce::int64, const uint8* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int16>(const EventChannel*, juce::int64, const int16* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint16>(const EventChannel*, juce::int64, const uint16* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int32>(const EventChannel*, juce::int64, const int32* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint32>(const EventChannel*, juce::int64, const uint32* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::int64>(const EventChannel*, juce::int64, const juce::int64* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::uint64>(const EventChannel*, juce::int64, const juce::uint64* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<float>(const EventChannel*, juce::int64, const float* data, int);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<double>(const EventChannel*, juce::int64, const double* data, int);

template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int8>(const EventChannel*, juce::int64, const int8* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint8>(const EventChannel*, juce::int64, const uint8* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int16>(const EventChannel*, juce::int64, const int16* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint16>(const EventChannel*, juce::int64, const uint16* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int32>(const EventChannel*, juce::int64, const int32* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint32>(const EventChannel*, juce::int64, const uint32* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::int64>(const EventChannel*, juce::int64, const juce::int64* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::uint64>(const EventChannel*, juce::int64, const juce::uint64* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<float>(const EventChannel*, juce::int64, const float* data, int, const MetadataValueArray&);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<double>(const EventChannel*, juce::int64, const double* data, int, const MetadataValueArray&);
