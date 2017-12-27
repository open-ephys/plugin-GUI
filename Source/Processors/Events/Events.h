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
#define SPIKE_BASE_SIZE 18

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

The first bit of EventType will be set to 1 when the event is recorded to avoid re-recording events. This will go away when the probe system is implemented.
*/

/**
Spike packet structure
EventType - 1byte
ElectrodeType - 1byte
Source processor ID - 2bytes
Source Subprocessor index - 2 bytes
Source electrode index - 2 bytes
Timestamp - 8 bytes
sortedID - 2 bytes (defaults to 0)
Thresholds - 4bytes*nChannels
Data - 4bytes*nChannels*nSamples
*/
class EventBase;
class Event;
class TTLEvent;
class TextEvent;
class BinaryEvent;
class SpikeEvent;

enum EventType
{
	SYSTEM_EVENT,
	PROCESSOR_EVENT,
	SPIKE_EVENT
};

enum SystemEventType
{
	TIMESTAMP_AND_SAMPLES = 0, //timestamp and buffer size are now the same event
	PARAMETER_CHANGE = 2,
	TIMESTAMP_SYNC_TEXT = 3 //Special text message, not associated with any event channel, for sourcenodes to send timestamp text messages
};

typedef ScopedPointer<EventBase> EventBasePtr;
class PLUGIN_API EventBase
	: public MetaDataEvent
{
public: 
	virtual ~EventBase();
	virtual void serialize(void* dstBuffer, size_t dstSize) const = 0;
	EventType getBaseType() const;
	juce::int64 getTimestamp() const;
	uint16 getSourceID() const;
	uint16 getSubProcessorIdx() const;
	uint16 getSourceIndex() const;

	static EventType getBaseType(const MidiMessage& msg);
	static EventBasePtr deserializeFromMessage(const MidiMessage& msg, const GenericProcessor* processor);

	static uint16 getSourceID(const MidiMessage& msg);
	static uint16 getSubProcessorIdx(const MidiMessage& msg);
	static uint16 getSourceIndex(const MidiMessage& msg);
	static juce::int64 getTimestamp(const MidiMessage &msg);
protected:
	EventBase(EventType type, juce::int64 timestamp, uint16 sourceID, uint16 subIdx, uint16 sourceIndex);
	EventBase() = delete;

	static bool compareMetaData(const MetaDataEventObject* channelInfo, const MetaDataValueArray& metaData);

	const EventType m_baseType;
	const juce::int64 m_timestamp;
	const uint16 m_sourceID;
	const uint16 m_sourceSubIdx;
	const uint16 m_sourceIndex;
};

//Since system events are quite simple, might differ wildly and are only used by a few subsystems, mostly GUI internals
//Instead of creating proper objects for each one it's easier to just provide a set of static methods
//To construct and decode them
class PLUGIN_API SystemEvent
	: public EventBase
{
public:
	static size_t fillTimestampAndSamplesData(HeapBlock<char>& data, const GenericProcessor* proc, int16 subProcessorIdx, juce::int64 timestamp, uint32 nSamples);
	static size_t fillTimestampSyncTextData(HeapBlock<char>& data, const GenericProcessor* proc, int16 subProcessorIdx, juce::int64 timestamp, bool softwareTime = false);
	static SystemEventType getSystemEventType(const MidiMessage& msg);
	static uint32 getNumSamples(const MidiMessage& msg);
	static String getSyncText(const MidiMessage& msg);
private:
	SystemEvent() = delete;
};

typedef ScopedPointer<Event> EventPtr;
class PLUGIN_API Event
	: public EventBase
{
public:
	virtual ~Event();
	virtual void serialize(void* dstBuffer, size_t dstSize) const override = 0;
	Event(const Event& other);
	Event& operator=(const Event&) = delete;

	EventChannel::EventChannelTypes getEventType() const;
	const EventChannel* getChannelInfo() const;

	/** Gets the channel that triggered the event */
	uint16 getChannel() const;

	/* Gets the raw data payload */
	const void* getRawDataPointer() const;

	static EventChannel::EventChannelTypes getEventType(const MidiMessage& msg);
	static EventPtr deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);

protected:
	Event(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel);
	Event() = delete;
	bool serializeHeader(EventChannel::EventChannelTypes type, char* buffer, size_t dstSize) const;
	static bool createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel);
	static bool createChecks(const EventChannel* channelInfo, EventChannel::EventChannelTypes eventType, uint16 channel, const MetaDataValueArray& metaData);

	const uint16 m_channel;
	const EventChannel* m_channelInfo;
	const EventChannel::EventChannelTypes m_eventType;

	HeapBlock<char> m_data;

};

typedef ScopedPointer<TTLEvent> TTLEventPtr;
class PLUGIN_API TTLEvent
	: public Event
{
public:
	TTLEvent(const TTLEvent& other);
	~TTLEvent();
	TTLEvent& operator=(const TTLEvent&) = delete;

	void serialize(void* dstBuffer, size_t dstSize) const override;

	/** Gets the state true ='1' false = '0'*/
	bool getState() const;
	
	const void* getTTLWordPointer() const;

	static TTLEventPtr createTTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, const void* eventData, int dataSize, uint16 channel);
	static TTLEventPtr createTTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, const void* eventData, int dataSize, const MetaDataValueArray& metaData, uint16 channel);
	static TTLEventPtr deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	TTLEvent() = delete;
	TTLEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const void* eventData);

	JUCE_LEAK_DETECTOR(TTLEvent);
};

typedef ScopedPointer<TextEvent> TextEventPtr;
class PLUGIN_API TextEvent
	: public Event
{
public:
	TextEvent(const TextEvent& other);
	~TextEvent();
	TextEvent& operator=(const TextEvent&) = delete;

	void serialize(void* dstBuffer, size_t dstSize) const override;
	String getText() const;

	static TextEventPtr createTextEvent(const EventChannel* channelInfo, juce::int64 timestamp, const String& text, uint16 channel = 0);
	static TextEventPtr createTextEvent(const EventChannel* channelInfo, juce::int64 timestamp, const String& text, const MetaDataValueArray& metaData, uint16 channel = 0);
	static TextEventPtr deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	TextEvent() = delete;
	TextEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const String& text);

	JUCE_LEAK_DETECTOR(TextEvent);
};

typedef ScopedPointer<BinaryEvent> BinaryEventPtr;
class PLUGIN_API BinaryEvent
	: public Event
{
public:
	BinaryEvent(const BinaryEvent& other);
	~BinaryEvent();
	BinaryEvent& operator=(const BinaryEvent&) = delete;

	void serialize(void* dstBuffer, size_t dstSize) const override;

	const void* getBinaryDataPointer() const;
	EventChannel::EventChannelTypes getBinaryType() const;

	template<typename T>
	static BinaryEventPtr createBinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, const T* data, int dataSize, uint16 channel = 0);

	template<typename T>
	static BinaryEventPtr createBinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, const T* data, int dataSize, const MetaDataValueArray& metaData, uint16 channel = 0);

	static BinaryEventPtr deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
	
private:
	BinaryEvent() = delete;
	BinaryEvent(const EventChannel* channelInfo, juce::int64 timestamp, uint16 channel, const void* data, EventChannel::EventChannelTypes type);
	
	template<typename T>
	static EventChannel::EventChannelTypes getType();

	const EventChannel::EventChannelTypes m_type;
	JUCE_LEAK_DETECTOR(BinaryEvent);
};

typedef ScopedPointer<SpikeEvent> SpikeEventPtr;
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
	class PLUGIN_API SpikeBuffer
	{
		friend SpikeEvent;
	public:
		SpikeBuffer(const SpikeChannel* channelInfo);
		void set(const int chan, const int samp, const float value);
		void set(const int index, const float value);
		void set(const int chan, const float* source, const int n);
		void set(const int chan, const int start, const float* source, const int n);
		float get(const int chan, const int samp);
		float get(const int index);
		//Caution advised with this method, as the pointer can become inaccessible
		const float* getRawPointer();
	private:
		SpikeBuffer() = delete;
		HeapBlock<float> m_data;
		const int m_nChans;
		const int m_nSamps;
		bool m_ready{ true };
	};
	SpikeEvent(const SpikeEvent& other);
	~SpikeEvent();
	SpikeEvent& operator=(const SpikeEvent&) = delete;

	void serialize(void* dstBuffer, size_t dstSize) const override;

	const SpikeChannel* getChannelInfo() const;

	const float* getDataPointer() const;

	const float* getDataPointer(int channel) const;

	float getThreshold(int chan) const;

	uint16 getSortedID() const;

	static SpikeEventPtr createSpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, uint16 sortedID);
	static SpikeEventPtr createSpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, SpikeBuffer& dataSource, uint16 sortedID, const MetaDataValueArray& metaData);

	static SpikeEventPtr deserializeFromMessage(const MidiMessage& msg, const SpikeChannel* channelInfo);
private:
	SpikeEvent() = delete;
	SpikeEvent(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> thresholds, HeapBlock<float>& data, uint16 sortedID);
	static SpikeEvent* createBasicSpike(const SpikeChannel* channelInfo, juce::int64 timestamp, Array<float> threshold, SpikeBuffer& dataSource, uint16 sortedID);

	const Array<float> m_thresholds;
	const SpikeChannel* m_channelInfo;
	const uint16 m_sortedID;
	HeapBlock<float> m_data;
	JUCE_LEAK_DETECTOR(SpikeEvent);
};


#endif
