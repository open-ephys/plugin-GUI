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
//EventBase

EventType EventBase::getBaseType() const
{
	return m_baseType;
}

EventType EventBase::getBaseType(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	//TODO: remove the mask when the probe system is implemented
	return static_cast<EventType>((*data)&0x7F);
}

juce::int64 EventBase::getTimestamp() const
{
	return m_timestamp;
}

juce::int64 EventBase::getTimestamp(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return *reinterpret_cast<const juce::int64*>(data + 8);
}

EventBase::EventBase(EventType type, juce::int64 timestamp, uint16 sourceID, uint16 sourceSubIdx, uint16 sourceIndex)
	: m_baseType(type), m_timestamp(timestamp), m_sourceID(sourceID), m_sourceSubIdx(sourceSubIdx), m_sourceIndex(sourceIndex)
{}

EventBase::~EventBase() {}

EventBasePtr EventBase::deserializeFromMessage(const MidiMessage& msg, const GenericProcessor* processor)
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
			return Event::deserializeFromMessage(msg, chan).release();
	}
	case SPIKE_EVENT:
	{
		const SpikeChannel* chan = processor->getSpikeChannel(processor->getSpikeChannelIndex(channelIDX, processorID, subProcessorID));
		return SpikeEvent::deserializeFromMessage(msg, chan).release();
	}
	default:
		return nullptr;
		break;
	}
}

bool EventBase::compareMetaData(const MetaDataEventObject* channelInfo, const MetaDataValueArray& metaData)
{
	int metaDataSize = metaData.size();

	if (metaDataSize != channelInfo->getEventMetaDataCount()) return false;

	for (int i = 0; i < metaDataSize; i++)
	{
		if (!metaData[i]->isOfType(channelInfo->getEventMetaDataDescriptor(i))) return false;
	}
	return true;
}

uint16 EventBase::getSourceID(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return *reinterpret_cast<const uint16*>(data + 2);
}

uint16 EventBase::getSourceID() const
{
	return m_sourceID;
}

uint16 EventBase::getSubProcessorIdx(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return *reinterpret_cast<const uint16*>(data + 4);
}

uint16 EventBase::getSubProcessorIdx() const
{
	return m_sourceSubIdx;
}

uint16 EventBase::getSourceIndex(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return *reinterpret_cast<const uint16*>(data + 6);
}

uint16 EventBase::getSourceIndex() const
{
	return m_sourceIndex;
}

//SystemEvent
SystemEventType SystemEvent::getSystemEventType(const MidiMessage& msg)
{
	const uint8* data = msg.getRawData();
	return static_cast<SystemEventType>(*(data + 1));
}

size_t SystemEvent::fillTimestampAndSamplesData(HeapBlock<char>& data, const GenericProcessor* proc, int16 subProcessorIdx, juce::int64 timestamp, uint32 nSamples)
{
	/** Event packet structure
	* SYSTEM_EVENT - 1 byte
	* TIMESTAMP_AND_SAMPLES - 1 byte
	* Source processorID - 2 bytes
	* Source Subprocessor index - 2 bytes
	* Zero-fill (to maintain aligment with other events) - 2 bytes
	* Timestamp - 8 bytes
	* Buffer sample number - 4 bytes
	*/
	const int eventSize = 20;
	data.malloc(eventSize);
	data[0] = SYSTEM_EVENT;
	data[1] = TIMESTAMP_AND_SAMPLES;
	*reinterpret_cast<uint16*>(data.getData() + 2) = proc->getNodeId();
	*reinterpret_cast<uint16*>(data.getData() + 4) = subProcessorIdx;
	data[6] = 0;
	data[7] = 0;
	*reinterpret_cast<juce::int64*>(data.getData() + 8) = timestamp;
	*reinterpret_cast<uint32*>(data.getData() + 16) = nSamples;
	return eventSize;
}

size_t SystemEvent::fillTimestampSyncTextData(HeapBlock<char>& data, const GenericProcessor* proc, int16 subProcessorIdx, juce::int64 timestamp, bool softwareTime)
{
	/** Event packet structure
	* SYSTEM_EVENT - 1 byte
	* TIMESTAMP_SYNC_TEXT - 1 byte
	* Source processorID - 2 bytes
	* Source Subprocessor index - 2 bytes
	* Zero-fill (to maintain aligment with other events) - 2 bytes
	* Timestamp - 8 bytes
	* string - variable
	*/
	String eventString;
	if (softwareTime)
	{
		eventString = "Software time: " + String(timestamp) + "@" + String(Time::getHighResolutionTicksPerSecond()) + "Hz";
	}
	else
	{
		eventString = "Processor: "
			+ proc->getName()
			+ " Id: "
			+ String(proc->getNodeId())
			+ " subProcessor: "
			+ String(subProcessorIdx)
		+" start time: "
			+ String(timestamp)
			+ "@"
			+ String(proc->getSampleRate(subProcessorIdx))
			+ "Hz";
	}
	size_t textSize = eventString.getNumBytesAsUTF8();
	size_t dataSize = 17 + textSize;
	data.allocate(dataSize, true);
	data[0] = SYSTEM_EVENT;
	data[1] = TIMESTAMP_SYNC_TEXT;
	*reinterpret_cast<uint16*>(data.getData() + 2) = proc->getNodeId();
	*reinterpret_cast<uint16*>(data.getData() + 4) = subProcessorIdx;
	*reinterpret_cast<juce::int64*>(data.getData() + 8) = timestamp;
	memcpy(data.getData() + 16, eventString.toUTF8(), textSize);
	return dataSize;
}

uint32 SystemEvent::getNumSamples(const MidiMessage& msg)
{
	if (getBaseType(msg) != SYSTEM_EVENT && getSystemEventType(msg) != TIMESTAMP_AND_SAMPLES)
		return 0;

	return *reinterpret_cast<const uint32*>(msg.getRawData() + 16);
}

String SystemEvent::getSyncText(const MidiMessage& msg)
{
	if (getBaseType(msg) != SYSTEM_EVENT && getSystemEventType(msg) != TIMESTAMP_SYNC_TEXT)
		return String::empty;

	const char* data = reinterpret_cast<const char*>(msg.getRawData());
	return String::fromUTF8(data + 16, msg.getRawDataSize() - 17);
}

//Event
Event::Event(const Event& other)
	: EventBase(other), 
	m_channel(other.m_channel), 
	m_channelInfo(other.m_channelInfo),
	m_eventType(other.m_eventType)
{
	size_t size = other.m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), other.m_data.getData(), size);
}

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

Event::Event(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel)
	: EventBase(PROCESSOR_EVENT, timestamp, channelInfo->getSourceNodeID(), channelInfo->getSubProcessorIdx(), channelInfo->getSourceIndex()),
	m_channel(channel),
	m_channelInfo(channelInfo),
	m_eventType(channelInfo->getChannelType())
{}

Event::~Event() {}

EventPtr Event::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	EventChannel::EventChannelTypes type = channelInfo->getChannelType();
	
	if (type == EventChannel::TTL)
		return TTLEvent::deserializeFromMessage(msg, channelInfo).release();
	else if (type == EventChannel::TEXT)
		return TextEvent::deserializeFromMessage(msg, channelInfo).release();
	else if (type >= EventChannel::INT8_ARRAY && type <= EventChannel::DOUBLE_ARRAY)
		return BinaryEvent::deserializeFromMessage(msg, channelInfo).release();
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
	*(reinterpret_cast<juce::int64*>(buffer + 8)) = m_timestamp;
	*(reinterpret_cast<uint16*>(buffer + 16)) = m_channel;
	return true;
}

bool Event::createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel)
{
	if (!channelInfo) return false;
	if (channelInfo->getChannelType() != eventType) return false;
	if ((channel < 0) || (channel >= channelInfo->getNumChannels())) return false;
	if (channelInfo->getEventMetaDataCount() != 0) return false;
	return true;
}

bool Event::createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel, const MetaDataValueArray& metaData)
{
	if (!channelInfo) return false;
	if (channelInfo->getChannelType() != eventType) return false;
	if ((channel < 0) || (channel >= channelInfo->getNumChannels())) return false;
	if (!compareMetaData(channelInfo, metaData)) return false;
	return true;
}

const void* Event::getRawDataPointer() const
{
	return m_data.getData();
}

//TTLEvent

TTLEvent::TTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const void* eventData)
	: Event(channelInfo, timestamp, channel)
{
	size_t size = m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), eventData, size);
}

TTLEvent::TTLEvent(const TTLEvent& other)
	: Event(other)
{
	size_t size = other.m_channelInfo->getDataSize();
	m_data.malloc(size);
	memcpy(m_data.getData(), other.m_data.getData(), size);
}

TTLEvent::~TTLEvent() {}

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
	memcpy((buffer + EVENT_BASE_SIZE), m_data.getData(), dataSize);
	serializeMetaData(buffer + eventSize);
}

TTLEventPtr TTLEvent::createTTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, const void* eventData, int dataSize, uint16 channel)
{

	if (!createChecks(channelInfo, EventChannel::TTL, channel))
	{
		jassertfalse;
		return nullptr;
	}
	
	if (dataSize < channelInfo->getDataSize()) 
	{
		jassertfalse;
		return nullptr;
	}
	
	return new TTLEvent(channelInfo, timestamp, channel, eventData);
}

TTLEventPtr TTLEvent::createTTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, const void* eventData, int dataSize, const MetaDataValueArray& metaData, uint16 channel)
{

	if (!createChecks(channelInfo, EventChannel::TTL, channel, metaData))
	{
		jassertfalse;
		return nullptr;
	}

	if (dataSize < channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	TTLEvent* event = new TTLEvent(channelInfo, timestamp, channel, eventData);
	
	event->m_metaDataValues.addArray(metaData);
	return event;
}

TTLEventPtr TTLEvent::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	size_t totalSize = msg.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetaDataSize();

	if (totalSize != (dataSize + EVENT_BASE_SIZE + metaDataSize))
	{
		jassertfalse;
		return nullptr;
	}
	const uint8* buffer = msg.getRawData();
	//TODO: remove the mask when the probe system is implemented
	if (static_cast<EventType>(*(buffer + 0)&0x7F) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getChannelType() != EventChannel::TTL)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<EventChannel::EventChannelTypes>(*(buffer + 1)) != EventChannel::TTL) {
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeID())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getSubProcessorIdx())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getSourceIndex())
	{
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 channel = *(reinterpret_cast<const uint16*>(buffer + 16));

	ScopedPointer<TTLEvent> event = new TTLEvent(channelInfo, timestamp, channel, (buffer + EVENT_BASE_SIZE));
	bool ret = true;
	if (metaDataSize > 0)
		 ret = event->deserializeMetaData(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
 		jassertfalse;
		return nullptr;
	}
}

//TextEvent
TextEvent::TextEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const String& text)
	: Event(channelInfo, timestamp, channel)
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

void TextEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(EventChannel::TEXT, buffer, dstSize))
		return;

	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	//size_t stringSize = m_text.getNumBytesAsUTF8();
	memcpy((buffer + EVENT_BASE_SIZE), m_data, dataSize);
//	if ((dataSize - stringSize) > 0)
//		zeromem((buffer + EVENT_BASE_SIZE + stringSize), dataSize - stringSize);
	serializeMetaData(buffer + eventSize);
}

TextEventPtr TextEvent::createTextEvent(const EventChannel* channelInfo, juce::int64 timestamp, const String& text, uint16 channel)
{
	if (!createChecks(channelInfo, EventChannel::TEXT, channel))
	{
		jassertfalse;
		return nullptr;
	}

	if (text.getNumBytesAsUTF8() > channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	return new TextEvent(channelInfo, timestamp, channel, text);
}

TextEventPtr TextEvent::createTextEvent(const EventChannel* channelInfo, juce::int64 timestamp, const String& text, const MetaDataValueArray& metaData, uint16 channel)
{
	if (!createChecks(channelInfo, EventChannel::TEXT, channel, metaData))
	{
		jassertfalse;
		return nullptr;
	}

	if (text.getNumBytesAsUTF8() > channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	TextEvent* event = new TextEvent(channelInfo, timestamp, channel, text);
	
	event->m_metaDataValues.addArray(metaData);
	return event;
}

TextEventPtr TextEvent::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	size_t totalSize = msg.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetaDataSize();

	if (totalSize != (dataSize + EVENT_BASE_SIZE + metaDataSize))
	{
		jassertfalse;
		return nullptr;
	}
	const uint8* buffer = msg.getRawData();
	//TODO: remove the mask when the probe system is implemented
	if (static_cast<EventType>(*(buffer + 0)&0x7F) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getChannelType() != EventChannel::TEXT)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<EventChannel::EventChannelTypes>(*(buffer + 1)) != EventChannel::TEXT) {
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeID())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getSubProcessorIdx())
	{
		jassertfalse;
		return nullptr;
	}
	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getSourceIndex())
	{
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 channel = *(reinterpret_cast<const uint16*>(buffer + 16));
	String text = String::fromUTF8(reinterpret_cast<const char*>(buffer + EVENT_BASE_SIZE), dataSize);

	ScopedPointer<TextEvent> event = new TextEvent(channelInfo, timestamp, channel, text);
	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetaData(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}
}

//BinaryEvent
BinaryEvent::BinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const void* data, EventChannel::EventChannelTypes type)
	: Event(channelInfo, timestamp, channel),
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

EventChannel::EventChannelTypes BinaryEvent::getBinaryType() const
{
	return m_type;
}

template<typename T>
EventChannel::EventChannelTypes BinaryEvent::getType()
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

	return EventChannel::INVALID;
}

void BinaryEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	char* buffer = static_cast<char*>(dstBuffer);
	if (!serializeHeader(m_type, buffer, dstSize))
		return;
	
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + EVENT_BASE_SIZE;
	memcpy((buffer + EVENT_BASE_SIZE), m_data.getData(), dataSize);
	serializeMetaData(buffer + eventSize);
}

template<typename T>
BinaryEventPtr BinaryEvent::createBinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, const T* data, int dataSize, uint16 channel)
{
	EventChannel::EventChannelTypes type = getType<T>();
	if (type == EventChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	if (!createChecks(channelInfo, type, channel))
	{
		jassertfalse;
		return nullptr;
	}

	if (dataSize < channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	return new BinaryEvent(channelInfo, timestamp, channel, data, type);
}

template<typename T>
BinaryEventPtr BinaryEvent::createBinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, const T* data, int dataSize, const MetaDataValueArray& metaData, uint16 channel)
{
	EventChannel::EventChannelTypes type = getType<T>();
	if (type == EventChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	if (!createChecks(channelInfo, type, channel, metaData))
	{
		jassertfalse;
		return nullptr;
	}

	if (dataSize < channelInfo->getDataSize())
	{
		jassertfalse;
		return nullptr;
	}

	BinaryEvent* event = new BinaryEvent(channelInfo, timestamp, channel, data, type);
	event->m_metaDataValues.addArray(metaData);
	return event;
}

BinaryEventPtr BinaryEvent::deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo)
{
	size_t totalSize = msg.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t metaDataSize = channelInfo->getTotalEventMetaDataSize();

	if (totalSize != (dataSize + EVENT_BASE_SIZE + metaDataSize))
	{
		jassertfalse;
		return nullptr;
	}
	const uint8* buffer = msg.getRawData();
	//TODO: remove the mask when the probe system is implemented
	if (static_cast<EventType>(*(buffer + 0)&0x7F) != PROCESSOR_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (channelInfo->getChannelType() < EventChannel::BINARY_BASE_VALUE || channelInfo->getChannelType() >= EventChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	const EventChannel::EventChannelTypes type = static_cast<const EventChannel::EventChannelTypes>(*(buffer + 1));
	if (type != channelInfo->getChannelType())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeID())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getSubProcessorIdx())
	{
		jassertfalse;
		return nullptr;
	}
	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getSourceIndex())
	{
		jassertfalse;
		return nullptr;
	}
	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 channel = *(reinterpret_cast<const uint16*>(buffer + 16));

	ScopedPointer<BinaryEvent> event = new BinaryEvent(channelInfo, timestamp, channel, (buffer + EVENT_BASE_SIZE), type);
	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetaData(channelInfo, (buffer + EVENT_BASE_SIZE + dataSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}
}

//SpikeEvent
SpikeEvent::SpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, HeapBlock<float>& data, uint16 sortedID)
	: EventBase(SPIKE_EVENT, timestamp, channelInfo->getSourceNodeID(), channelInfo->getSubProcessorIdx(), channelInfo->getSourceIndex()),
	m_thresholds(thresholds),
	m_channelInfo(channelInfo),
	m_sortedID(sortedID)
{
	m_data.swapWith(data);
}

SpikeEvent::SpikeEvent(const SpikeEvent& other)
	:EventBase(other),
	m_thresholds(other.m_thresholds),
	m_channelInfo(other.m_channelInfo),
	m_sortedID(other.m_sortedID)
{
	size_t size = m_channelInfo->getDataSize();
	m_data.malloc(size, sizeof(char));
	memcpy(m_data.getData(), other.m_data.getData(), size);
}

SpikeEvent::~SpikeEvent() {}

const float* SpikeEvent::getDataPointer() const
{
	return m_data.getData();
}

uint16 SpikeEvent::getSortedID() const
{
	return m_sortedID;
}

const float* SpikeEvent::getDataPointer(int channel) const
{
	if ((channel < 0) || (channel >= m_channelInfo->getNumChannels()))
	{
		jassertfalse;
		return nullptr;
	}
	return (m_data.getData() + (channel*m_channelInfo->getTotalSamples()));
}

float SpikeEvent::getThreshold(int chan) const
{
	return m_thresholds[chan];
}

const SpikeChannel* SpikeEvent::getChannelInfo() const
{
	return m_channelInfo;
}

void SpikeEvent::serialize(void* dstBuffer, size_t dstSize) const
{
	size_t dataSize = m_channelInfo->getDataSize();
	size_t eventSize = dataSize + SPIKE_BASE_SIZE + m_thresholds.size() * sizeof(float);
	size_t totalSize = eventSize + m_channelInfo->getTotalEventMetaDataSize();
	if (totalSize < dstSize)
	{
		jassertfalse;
		return;
	}

	char* buffer = static_cast<char*>(dstBuffer);

	*(buffer + 0) = SPIKE_EVENT;
	*(buffer + 1) = static_cast<char>(m_channelInfo->getChannelType());
	*(reinterpret_cast<uint16*>(buffer + 2)) = m_channelInfo->getSourceNodeID();
	*(reinterpret_cast<uint16*>(buffer + 4)) = m_channelInfo->getSubProcessorIdx();
	*(reinterpret_cast<uint16*>(buffer + 6)) = m_channelInfo->getSourceIndex();
	*(reinterpret_cast<juce::int64*>(buffer + 8)) = m_timestamp;
	*(reinterpret_cast<uint16*>(buffer + 16)) = m_sortedID;
	int memIdx = SPIKE_BASE_SIZE;
	for (int i = 0; i < m_thresholds.size(); i++)
	{
		*(reinterpret_cast<float*>(buffer + memIdx)) = m_thresholds[i];
		memIdx += sizeof(float);
	}
	memcpy((buffer + memIdx), m_data.getData(), dataSize);
	serializeMetaData(buffer + eventSize);
}

SpikeEvent* SpikeEvent::createBasicSpike(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, uint16 sortedID)
{
	if (!dataSource.m_ready)
	{
		jassertfalse;
		return nullptr;
	}
	if (channelInfo->getChannelType() == SpikeChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}
	int nChannels = channelInfo->getNumChannels();
	if (nChannels != dataSource.m_nChans)
	{
		jassertfalse;
		return nullptr;
	}
	int nSamples = channelInfo->getTotalSamples();
	if (nSamples != dataSource.m_nSamps)
	{
		jassertfalse;
		return nullptr;
	}
	if (thresholds.size() != nChannels)
	{
		jassertfalse;
		return nullptr;
	}
	dataSource.m_ready = false;
	return new SpikeEvent(channelInfo, timestamp, thresholds, dataSource.m_data, sortedID);

}

SpikeEventPtr SpikeEvent::createSpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, uint16 sortedID)
{
	if (!channelInfo)
	{
		jassertfalse;
		return nullptr;
	}

	
	if (channelInfo->getEventMetaDataCount() != 0)
	{
		jassertfalse;
		return nullptr;
	}

	return createBasicSpike(channelInfo, timestamp, thresholds, dataSource, sortedID);	
	
}

SpikeEventPtr SpikeEvent::createSpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, uint16 sortedID, const MetaDataValueArray& metaData)
{
	if (!channelInfo)
	{
		jassertfalse;
		return nullptr;
	}

	if (!compareMetaData(channelInfo, metaData))
	{
		jassertfalse;
		return nullptr;
	}
	SpikeEvent* event = createBasicSpike(channelInfo, timestamp, thresholds, dataSource, sortedID);
	if (!event)
	{
		jassertfalse;
		return nullptr;
	}

	event->m_metaDataValues.addArray(metaData);
	return event;
}

SpikeEventPtr SpikeEvent::deserializeFromMessage(const MidiMessage& msg, const SpikeChannel* channelInfo)
{
	int nChans = channelInfo->getNumChannels();
	size_t totalSize = msg.getRawDataSize();
	size_t dataSize = channelInfo->getDataSize();
	size_t thresholdSize = nChans*sizeof(float);
	size_t metaDataSize = channelInfo->getTotalEventMetaDataSize();

	if (channelInfo->getChannelType() == SpikeChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	if (totalSize != (thresholdSize + dataSize + SPIKE_BASE_SIZE + metaDataSize))
	{
		jassertfalse;
		return nullptr;
	}
	const uint8* buffer = msg.getRawData();
	//TODO: remove the mask when the probe system is implemented
	if (static_cast<EventType>(*(buffer + 0)&0x7F) != SPIKE_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<SpikeChannel::ElectrodeTypes>(*(buffer + 1)) != channelInfo->getChannelType())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeID())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getSubProcessorIdx())
	{
		jassertfalse;
		return nullptr;
	}
	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getSourceIndex())
	{
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 sortedID = *(reinterpret_cast<const uint16*>(buffer + 16));
	Array<float> thresholds;
	thresholds.addArray(reinterpret_cast<const float*>(buffer + SPIKE_BASE_SIZE), nChans);
	HeapBlock<float> data;
	data.malloc(dataSize, sizeof(char));
	memcpy(data.getData(), (buffer + SPIKE_BASE_SIZE + thresholdSize), dataSize);

	ScopedPointer<SpikeEvent> event = new SpikeEvent(channelInfo, timestamp, thresholds, data, sortedID);

	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetaData(channelInfo, (buffer + SPIKE_BASE_SIZE + dataSize + thresholdSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{

		jassertfalse;
		return nullptr;
	}
}

SpikeEvent::SpikeBuffer::SpikeBuffer(const SpikeChannel* channelInfo)
	: m_nChans(channelInfo->getNumChannels()),
	m_nSamps(channelInfo->getTotalSamples())
{
	m_data.malloc(m_nChans*m_nSamps);
}

void  SpikeEvent::SpikeBuffer::set(const int chan, const int samp, const float value)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && samp >= 0 && chan < m_nChans && samp < m_nSamps);
	m_data[samp + chan*m_nSamps] = value;
}

void  SpikeEvent::SpikeBuffer::set(const int index, const float value)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(index >= 0 && index < m_nChans * m_nSamps);
	m_data[index] = value;
}

void  SpikeEvent::SpikeBuffer::set(const int chan, const float* source, const int n)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && chan < m_nChans && n <= m_nSamps);
	memcpy(m_data.getData(), source, n*sizeof(float));
}

void  SpikeEvent::SpikeBuffer::set(const int chan, const int start, const float* source, const int n)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && chan < m_nChans && (n + start) <= m_nSamps);
	memcpy(m_data.getData() + start, source, n*sizeof(float));
}

float SpikeEvent::SpikeBuffer::get(const int chan, const int samp)
{
	if (!m_ready)
	{
		jassertfalse;
		return 0;
	}
	jassert(chan >= 0 && samp >= 0 && chan < m_nChans && samp < m_nSamps);
	return m_data[chan*m_nSamps + samp];
}

float SpikeEvent::SpikeBuffer::get(const int index)
{
	if (!m_ready)
	{
		jassertfalse;
		return 0;
	}
	jassert(index >= 0 && index < m_nChans * m_nSamps);
	return m_data[index];
}

const float* SpikeEvent::SpikeBuffer::getRawPointer()
{
	if (!m_ready)
	{
		jassertfalse;
		return nullptr;
	}
	return m_data.getData();
}

//Template definitions
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int8>(const EventChannel*, juce::int64, const int8* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint8>(const EventChannel*, juce::int64, const uint8* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int16>(const EventChannel*, juce::int64, const int16* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint16>(const EventChannel*, juce::int64, const uint16* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int32>(const EventChannel*, juce::int64, const int32* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint32>(const EventChannel*, juce::int64, const uint32* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::int64>(const EventChannel*, juce::int64, const juce::int64* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::uint64>(const EventChannel*, juce::int64, const juce::uint64* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<float>(const EventChannel*, juce::int64, const float* data, int, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<double>(const EventChannel*, juce::int64, const double* data, int, uint16);

template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int8>(const EventChannel*, juce::int64, const int8* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint8>(const EventChannel*, juce::int64, const uint8* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int16>(const EventChannel*, juce::int64, const int16* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint16>(const EventChannel*, juce::int64, const uint16* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<int32>(const EventChannel*, juce::int64, const int32* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<uint32>(const EventChannel*, juce::int64, const uint32* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::int64>(const EventChannel*, juce::int64, const juce::int64* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<juce::uint64>(const EventChannel*, juce::int64, const juce::uint64* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<float>(const EventChannel*, juce::int64, const float* data, int, const MetaDataValueArray&, uint16);
template PLUGIN_API BinaryEventPtr BinaryEvent::createBinaryEvent<double>(const EventChannel*, juce::int64, const double* data, int, const MetaDataValueArray&, uint16);
