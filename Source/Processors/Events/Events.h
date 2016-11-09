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

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "../Channel/InfoObjects.h"

class GenericProcessor;

/**
Event packet structure:
EventType - 1byte
SubType - 1byte
Source processor ID - 2bytes
Source Subprocessor index - 2 bytes
Source Channel index (except system events)
Timestamp - 8 bytes (except for system events that are not timestamp)
data - variable
*/

enum EventType
{
	SYSYEM_EVENT,
	PROCESSOR_EVENT,
	SPIKE_EVENT
};

enum SystemEventType
{
	TIMESTAMP = 0,
	BUFFER_SIZE = 1,
	PARAMETER_CHANGE = 2
};

class PLUGIN_API EventBase
	: public MetaDataEvent
{
public: 
	virtual void Serialize(void* dstBuffer, size_t dstSize) const = 0;
	EventType getBaseType() const;

	static EventType getBaseType(const MidiMessage& msg);
	static EventBase* deserializeFromMessage(const MidiMessage& msg, const GenericProcessor* processor);
protected:
	EventBase(EventType type, uint64 timestamp);
	EventBase() = delete;

	static bool compareMetaData(const EventChannel* channelInfo, const MetaDataValueArray& metaData);

	const EventType m_baseType;
	const uint64 m_timestamp;
};

class PLUGIN_API Event
	: public EventBase
{
public:
	virtual void Serialize(void* dstBuffer, size_t dstSize) const override = 0;
	EventChannel::EventChannelTypes getEventType() const;
	const EventChannel* getChannelInfo() const;

	static EventChannel::EventChannelTypes getEventType(const MidiMessage& msg);
	static Event* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);

protected:
	Event(const EventChannel* channelInfo, uint64 timestamp);
	Event() = delete;

	const EventChannel* m_channelInfo;
	const EventChannel::EventChannelTypes m_eventType;

};

class PLUGIN_API TTLEvent
	: public Event
{
public:
	void Serialize(void* dstBuffer, size_t dstSize) const override;

	/** Gets the channel that triggered the event */
	unsigned int getChannel() const;
	/** Gets the state true ='1' false = '0'*/
	bool getState() const;
	
	const void* getTTLWordPointer() const;

	static TTLEvent* createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, unsigned int channel,const void* eventData);
	static TTLEvent* createTTLEvent(const EventChannel* channelInfo, uint64 timestamp, unsigned int channel, const void* eventData, const MetaDataValueArray& metaData);
	static TTLEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	TTLEvent() = delete;
	TTLEvent(const EventChannel* channelInfo, uint64 timestamp, unsigned int channel, const void* eventData);

	const unsigned int m_channel;
	HeapBlock<char> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TTLEvent);
};

class PLUGIN_API MessageEvent
	: public Event
{
public:
	void Serialize(void* dstBuffer, size_t dstSize) const override;
	String getMessage() const;

	static MessageEvent* createMessageEvent(const EventChannel* channelInfo, uint64 timestamp, const String& msg);
	static MessageEvent* createMessageEvent(const EventChannel* channelInfo, uint64 timestamp, const String& msg, const MetaDataValueArray& metaData);
	static MessageEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
private:
	MessageEvent() = delete;
	MessageEvent(const EventChannel* channelInfo, uint64 timestamp, const String& msg);

	const String m_msg;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MessageEvent);
};

class PLUGIN_API BinaryEvent
	: public Event
{
public:
	void Serialize(void* dstBuffer, size_t dstSize) const override;

	const void* getBinaryDataPointer() const;

	template<typename T>
	static BinaryEvent* createBinaryEvent(const EventChannel* channelInfo, uint64 timestamp, const T* data);

	template<typename T>
	static BinaryEvent* createBinaryEvent(const EventChannel* channelInfo, uint64 timestamp, const T* data, const MetaDataValueArray& metaData);

	static BinaryEvent* deserializeFromMessage(const MidiMessage& msg, const EventChannel* channelInfo);
	
private:
	BinaryEvent() = delete;
	BinaryEvent(const EventChannel* channelInfo, uint64 timestamp, const void* data);

	HeapBlock<char> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinaryEvent);
};

class PLUGIN_API SpikeEvent
	: public EventBase
{
public:
	struct SpikeDataSource
	{
		/** Buffer in which the samples are stored */
		const AudioSampleBuffer* buffer; 
		/** Array with the channels within the buffer from which the data is grabbed
		It must contain the same number of entries as channels the electrode has */
		Array<unsigned int> channels; 
		/** Array with the positions within the channels from which the data is grabbed.
		If can contain either the same number of entries as channels the electrode has
		or just one entry, in which case that sample number will be used for all channels.
		*/
		Array<unsigned int> positions; 

	};
	void Serialize(void* dstBuffer, size_t dstSize) const override;

	const float* getDataPointer() const;

	const float* getDataPointer(int channel) const;

	float getThreshold() const;

	static SpikeEvent* createSpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, float threshold, const SpikeDataSource& dataSource);
	static SpikeEvent* createSpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, float threshold, const SpikeDataSource& dataSource, const MetaDataValueArray& metaData);

	static SpikeEvent* deserializeFromMessage(const MidiMessage& msg, const SpikeChannel* channelInfo);
private:
	SpikeEvent() = delete;
	SpikeEvent(const SpikeChannel* channelInfo, uint64 timestamp, float threshold, Array<float*> data);

	const float m_threshold;
	HeapBlock<float> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeEvent);
};


#endif