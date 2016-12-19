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

#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include <JuceHeader.h>
#include "../Channel/InfoObjects.h"
#define EVENT_BASE_SIZE 18
#define SPIKE_BASE_SIZE 16

class GenericProcessor;

/**
Event packet structure:
EventType - 1byte
SubType - 1byte
Source processor ID - 2bytes
Source Subprocessor index - 2 bytes
Source Event index - 2 bytes 
Timestamp - 8 bytes
Event Virtual Channel - 2 bytes
data - variable
*/

/**
Spike packet structure
EventType - 1byte
ElectrodeType - 1byte
Source processor ID - 2bytes
Source Subprocessor index - 2 bytes
Source electrode index - 2 bytes
Timestamp - 8 bytes
Thresholds - 4bytes*nChannels
Data - 4bytes*nChannels*nSamples
*/

enum EventType
{
	SYSYEM_EVENT,
	PROCESSOR_EVENT,
	SPIKE_EVENT
};

enum SystemEventType
{
	TIMESTAMP = 0, //timestamp and buffer size are now the same event
	PARAMETER_CHANGE = 2
};

class PLUGIN_API EventBase
	: public MetaDataEvent
{
public: 
	virtual void serialize(void* dstBuffer, size_t dstSize) const = 0;
	EventType getBaseType() const;

	static EventType getBaseType(const MidiMessage& msg);
	static EventBase* deserializeFromMessage(const MidiMessage& msg, const GenericProcessor* processor);

	static uint16 getSourceID(const MidiMessage& msg);
	static uint16 getSubProcessorIdx(const MidiMessage& msg);
	static uint16 getSourceIndex(const MidiMessage& msg);
protected:
	EventBase(EventType type, uint64 timestamp);
	EventBase() = delete;

	static bool compareMetaData(const MetaDataEventObject* channelInfo, const MetaDataValueArray& metaData);

	const EventType m_baseType;
	const uint64 m_timestamp;
};

class PLUGIN_API Event
	: public EventBase
{
public:
	virtual void serialize(void* dstBuffer, size_t dstSize) const override = 0;
	EventChannel::EventChannelTypes getEventType() const;
	const EventChannel* getChannelInfo() const;

	/** Gets the channel that triggered the event */
	uint16 getChannel() const;

	static EventChannel::EventChannelTypes getEventType(const MidiMessage& msg);
	static Event* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);

protected:
	Event(const EventChannel* channelInfo, uint64 timestamp, uint16 channel);
	Event() = delete;
	bool serializeHeader(EventChannel::EventChannelTypes type, char* buffer, size_t dstSize) const;
	static bool createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel);
	static bool createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel, const MetaDataValueArray& metaData);

	const uint16 m_channel;
	const EventChannel* m_channelInfo;
	const EventChannel::EventChannelTypes m_eventType;

};

class PLUGIN_API TTLEvent
	: public Event
{
public:
	void serialize(void* dstBuffer, size_t dstSize) const override;

	/** Gets the state true ='1' false = '0'*/
	bool getState() const;
	
	const void* getTTLWordPointer() const;

	static TTLEvent* createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, const void* eventData, int dataSize, uint16 channel);
	static TTLEvent* createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, const void* eventData, int dataSize, const MetaDataValueArray& metaData, uint16 channel);
	static TTLEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	TTLEvent() = delete;
	TTLEvent(const EventChannel* channelInfo, uint64 timestamp, uint16 channel, const void* eventData);

	HeapBlock<char> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TTLEvent);
};

class PLUGIN_API TextEvent
	: public Event
{
public:
	void serialize(void* dstBuffer, size_t dstSize) const override;
	String getText() const;

	static TextEvent* createTextEvent(const EventChannel* channelInfo, uint64 timestamp, const String& text, uint16 channel = 0);
	static TextEvent* createTextEvent(const EventChannel* channelInfo, uint64 timestamp, const String& text, const MetaDataValueArray& metaData, uint16 channel = 0);
	static TextEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	TextEvent() = delete;
	TextEvent(const EventChannel* channelInfo, uint64 timestamp, uint16 channel, const String& text);

	const String m_text;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextEvent);
};

class PLUGIN_API BinaryEvent
	: public Event
{
public:
	void serialize(void* dstBuffer, size_t dstSize) const override;

	const void* getBinaryDataPointer() const;
	EventChannel::EventChannelTypes getBinaryType() const;

	template<typename T>
	static BinaryEvent* createBinaryEvent(const EventChannel* channelInfo, uint64 timestamp, const T* data, int dataSize, uint16 channel = 0);

	template<typename T>
	static BinaryEvent* createBinaryEvent(const EventChannel* channelInfo, uint64 timestamp, const T* data, int dataSize, const MetaDataValueArray& metaData, uint16 channel = 0);

	static BinaryEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
	
private:
	BinaryEvent() = delete;
	BinaryEvent(const EventChannel* channelInfo, uint64 timestamp, uint16 channel, const void* data, EventChannel::EventChannelTypes type);
	
	template<typename T>
	static EventChannel::EventChannelTypes getType();

	HeapBlock<char> m_data;
	const EventChannel::EventChannelTypes m_type;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinaryEvent);
};

class PLUGIN_API SpikeEvent
	: public EventBase
{
public:
	/**
	Simple helper class that helds a buffer for spike event creation.
	Gets invalid once it's fed into a spike event, so if code needs to reuse
	it needs to reinitializate it.
	eg.:
	SpikeBuffer spike(channelInfo1);
	add samples
	SpikeEvent* event = createSpikeEvent(channelInfo1, ..., spike); spike gets invalid here
	spike = SpikeBuffer(channelInfo2); reinitialize the buffer for a new spike.
	*/
	class SpikeBuffer
	{
		friend SpikeEvent;
	public:
		SpikeBuffer(const SpikeChannel* channelInfo);
		float* operator[] (const int index);
	private:
		SpikeBuffer() = delete;
		HeapBlock<float> m_data;
		const int m_nChans;
		const int m_nSamps;
		bool m_ready{ true };
	};

	void serialize(void* dstBuffer, size_t dstSize) const override;

	const float* getDataPointer() const;

	const float* getDataPointer(int channel) const;

	float getThreshold(int chan) const;

	static SpikeEvent* createSpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource);
	static SpikeEvent* createSpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, const MetaDataValueArray& metaData);

	static SpikeEvent* deserializeFromMessage(const MidiMessage& msg, const SpikeChannel* channelInfo);
private:
	SpikeEvent() = delete;
	SpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, Array<float> thresholds, HeapBlock<float>& data);
	static SpikeEvent* createBasicSpike(const SpikeChannel* channelInfo, uint64 timestamp, Array<float> threshold, SpikeBuffer& dataSource);

	const Array<float> m_thresholds;
	const SpikeChannel* m_channelInfo;
	HeapBlock<float> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeEvent);
};


#endif