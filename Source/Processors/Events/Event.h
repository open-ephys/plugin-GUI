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

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include <JuceHeader.h>
#include "../Settings/EventChannel.h"

#define EVENT_BASE_SIZE 24

typedef MidiMessage EventPacket;

class GenericProcessor;

class EventBase;
class Event;
class TTLEvent;
class TextEvent;
class BinaryEvent;

typedef ScopedPointer<EventBase> EventBasePtr;

/**
* 
* Base class for all Event objects (including Spikes)
* 
* Event objects can be packaged into EventPackets and passed between
* processors.
* 
* ==============================
* General EventPacket Structure
* ==============================
* 
* 1 Byte: 8-bit Event type (System, Processor, or Spike)
* 1 Byte: 8-bit Event Sub-type (TTL, Text, Binary, etc.)
* 2 Bytes: 16-bit Source processor ID
* 2 Bytes: 16-bit Source stream ID
* 2 Bytes: 16-bit Source channel index
* 8 Bytes: 64-bit Sample number
* 8 Bytes: double Timestamp (in seconds)
* Variable: Optional data payload
* 
* The EventBase class is part of the Open Ephys Plugin API
* 
*/
class PLUGIN_API EventBase : public MetadataEvent
{
public: 

	/**
	* Supported base event types
	*
	* These are stored as the first byte of an EventPacket,
	* and are used to determine how the event is handled.
	*/
	enum Type
	{
		// Used for information about each buffer; not seen by plugins
		SYSTEM_EVENT,

		// TTL, Text, and Binary events generated and handled by plugins
		PROCESSOR_EVENT,

		// Spike events (see Spike.h)
		SPIKE_EVENT
	};

	/* Destructor */
	virtual ~EventBase();

	/* Serialize event to a destination buffer (pure virtual method) */
	virtual void serialize(void* destinationBuffer, size_t bufferSize) const = 0;

	/* Get the base type from an event object */
	Type getBaseType() const;

	/* Get the event type from an EventPacket object */
	static Type getBaseType(const EventPacket& packet);

	/* Get the event type from a raw byte buffer */
	static Type getBaseType(const uint8* data);

	/* Get the ID of the processor that generated this event */
	uint16 getProcessorId() const;

	/* Get the event processor ID from an EventPacket object */
	static uint16 getProcessorId(const EventPacket& packet);

	/* Get the event processor ID from a raw byte buffer */
	static uint16 getProcessorId(const uint8* data);

	/* Get the ID of the DataStream associated with this event */
	uint16 getStreamId() const;

	/* Get the event source stream ID from an EventPacket object */
	static uint16 getStreamId(const EventPacket& packet);

	/* Get the event source stream ID from a raw byte buffer */
	static uint16 getStreamId(const uint8* data);

	/* Get the index of the event channel that generated this event */
	uint16 getChannelIndex() const;

	/* Get the event channel index from an EventPacket object */
	static uint16 getChannelIndex(const EventPacket& packet);

	/* Get the event channel index from a raw byte buffer */
	static uint16 getChannelIndex(const uint8* data);

	/* Get the sample number from an event object */
	int64 getSampleNumber() const;

	/* Get the sample number from an EventPacket object */
	static int64 getSampleNumber(const EventPacket& packet);
    
    /** Sets the timestamp in seconds*/
    void setTimestampInSeconds(double timestamp);
    
    /** Sets the timestamp in seconds for an EventPacket object*/
    static void setTimestampInSeconds(const EventPacket& packet, double timestamp);
    
    /* Get the timestamp (in seconds) an event object */
    double getTimestampInSeconds() const;

    /* Get the timestamp (in seconds) from an EventPacket object */
    static double getTimestampInSeconds(const EventPacket& packet);

	/* Create an event object from an EventPacket object */
	static EventBasePtr deserialize(const EventPacket& packet, const GenericProcessor* processor);

protected:
	/* Constructor */
	EventBase(Type type, 
			  int64 sampleNumber,
              double timestamp,
			  uint16 processorId, 
			  uint16 streamId, 
		      uint16 channelId);

	/* Prevent initialization of an empty event object*/
	EventBase() = delete;

	/* Check whether an event channel has the appropriate metadata*/
	static bool compareMetadata(const MetadataEventObject* channelInfo, const MetadataValueArray& metaData);

	const Type m_baseType;
	const int64 m_sampleNumber;
    double m_timestamp;
	const uint16 m_sourceProcessorId;
	const uint16 m_sourceStreamId;
	const uint16 m_sourceChannelIndex;
};


/**
*
* Holds data for GUI internal events
*
* SystemEvent objects can be packaged into EventPackets and passed between
* processors, but they cannot be directly accessed by plugins.
*
* This class provide a set of static methods to construct and decode 
* system events.
* 
* =============================
* System Event Packet Structure
* =============================
* 
* 1 Byte: Event type (SYSTEM_EVENT)
* 1 Byte: Event Sub-type (Timestamp, Parameter, or Sync text)
* 2 Bytes: Source processor ID
* 2 Bytes: Source stream ID
* 2 Bytes: Zeros (to maintain alignment with other events)
* 8 Bytes: Start sample for block (int64)
* 8 Bytes: Start timestamp for block (double)
* 4 Bytes: Buffer sample number
*  OR
* Variable: Timestamp sync text string
*
* The SystemEvent class is part of the Open Ephys Plugin API
*/ 

class PLUGIN_API SystemEvent : public EventBase
{
public:

	/**
	* Supported SystemEvent sub-types
	*
	* These are stored as the second byte of an EventPacket,
	* and are used to determine how the event is handled.
	*/
	enum Type
	{
		// Timestamp and sample count information for each incoming data buffer
		TIMESTAMP_AND_SAMPLES = 0,

		// Indicates an upstream parameter was changed
		PARAMETER_CHANGE = 2,

		// Special text event sent by Source Processors at the start of recording
		TIMESTAMP_SYNC_TEXT = 3

	};

	/* Create a TIMESTAMP_AND_SAMPLES event */
	static size_t fillTimestampAndSamplesData(HeapBlock<char>& data, 
		const GenericProcessor* proc, 
		uint16 streamId, 
		int64 startSampleForBlock,
        double timestamp,
		uint32 nSamplesInBlock,
		int64 processStartTime,
        int64 sampleIndexOfTimestamp);
		
	/* Create a TIMESTAMP_SYNC_TEXT event */
	static size_t fillTimestampSyncTextData(HeapBlock<char>& data, 
		const GenericProcessor* proc, 
		uint16 streamId,
		int64 startSample,
        double startTimestamp,
		bool softwareTime = false);
	
	/* Get the SystemEvent type from an EventPacket object */
	static Type getSystemEventType(const EventPacket& msg);

	/* Get the sample count from an EventPacket object */
	static uint32 getNumSamples(const EventPacket& msg);

	/* Get the sample count from an EventPacket object */
	static int64 getHiResTicks(const EventPacket& msg);
	
	/* Get the sync text from an EventPacket object */
	static String getSyncText(const EventPacket& msg);

private:

	/* Prevent initialization of an empty SystemEvent object*/
	SystemEvent() = delete;
};

typedef ScopedPointer<Event> EventPtr;

/**
*
* Used to transmit event data between processors
* 
* Includes: TTL, Text, and Binary events
*
* The Event class is part of the Open Ephys Plugin API
*/
class PLUGIN_API Event : public EventBase
{
public:

	/* Destructor */
	virtual ~Event();

	/* Serialize event to a buffer (pure virtual function) */
	virtual void serialize(void* destinationBuffer, size_t bufferSize) const override = 0;

	/* Copy constructor */
	Event(const Event& other);

	/* Prevent move assignment */
	Event& operator=(const Event&) = delete;

	/* Get event type (TTL, TEXT, or BINARY) */
	EventChannel::Type getEventType() const;

	/* Get the EventChannel info object associated with an event */
	const EventChannel* getChannelInfo() const;

	/* Gets the raw data payload */
	const void* getRawDataPointer() const;

	/* Get event type (TTL, TEXT, or BINARY) from an EventPacket object */
	static EventChannel::Type getEventType(const EventPacket& packet);

	/* Create an Event object from an EventPacket object */
	static EventPtr deserialize(const EventPacket& packet, const EventChannel* channelInfo);

protected:

	/* Constructor */
	Event(const EventChannel* channelInfo, int64 sampleNumber, double timestamp=-1.0);

	/* Prevent the creation of an empty Event object */
	Event() = delete;

	/* Serialize the event header */
	bool serializeHeader(EventChannel::Type type, char* buffer, size_t dstSize) const;

	/* Check validity of an event */
	static bool createChecks(const EventChannel* channelInfo, EventChannel::Type eventType);

	/* Check validity of an event that includes metadata */
	static bool createChecks(const EventChannel* channelInfo, EventChannel::Type eventType, const MetadataValueArray& metaData);

	const EventChannel* m_channelInfo;
	const EventChannel::Type m_eventType;

	HeapBlock<char> m_data;

};

typedef ScopedPointer<TTLEvent> TTLEventPtr;

/**
*
* Used to transmit TTL-like events between processors
* 
* "TTL" stands for "transitor-transitor logic" and typically
* refers to the HI/LOW transitions that are used
* to transmit data between digital devices.
* 
* The GUI uses TTL events to represent any situations
* in which lines can be flipped on and off to represent
* state transitions. Each TTL event channel can track
* the status of up to 256 lines.
* 
* TTL events can also hold arbitrary information in their 
* metadata field, which can be used to annotate individual events.
*
* TTLEvent class is part of the Open Ephys Plugin API
*
*/
class PLUGIN_API TTLEvent : public Event
{
public:

	/* Copy constructor */
	TTLEvent(const TTLEvent& other);

	/* Destructor */
	~TTLEvent();

	/* Prevent move assignment */
	TTLEvent& operator=(const TTLEvent&) = delete;

	/* Serialize the event to a buffer */
	void serialize(void* destinationBuffer, size_t bufferSize) const override;

	/* Gets the state true ='1/ON/HIGH' false = '0/OFF/LOW'*/
	bool getState() const;

	/* Gets the line on which the change occurred.*/
	uint8 getLine() const;

	/* Gets the TTL word (state across first 64 lines) */
	uint64 getWord() const;

	/* Get the event state from an EventPacket object */
	static bool getState(const EventPacket& packet);

	/* Get the event line from an EventPacket object */
	static uint8 getLine(const EventPacket& packet);

	/* Create a TTL event with line and state (TTL word is tracked automatically) */
	static TTLEventPtr createTTLEvent(EventChannel* channelInfo,
		int64 sampleNumber,
		uint8 line,
		bool state);
	
	/* Create a TTL event that specifies the full TTL word*/
	static TTLEventPtr createTTLEvent(const EventChannel* channelInfo, 
		int64 sampleNumber,
		uint8 line, 
		bool state,
		uint64 word);

	/* Create a TTL event that includes metadata*/
	static TTLEventPtr createTTLEvent(EventChannel* channelInfo, 
		int64 sampleNumber,
		uint8 line,
		bool state,
		const MetadataValueArray& metaData);

	/* Deserialize a TTL event from an EventPacket object */
	static TTLEventPtr deserialize(const EventPacket& packet, const EventChannel* channelInfo);

	/* Deserialize a TTL event from a raw byte buffer*/
	static TTLEventPtr deserialize(const uint8* data, const EventChannel* channelInfo);

private:
	/* Prevent the creation of an empty event*/
	TTLEvent() = delete;

	/* Constructor */
	TTLEvent(const EventChannel* channelInfo, int64 sampleNumber, const void* eventData, double timestamp = -1.0);

	JUCE_LEAK_DETECTOR(TTLEvent);
};

typedef ScopedPointer<TextEvent> TextEventPtr;

/**
*
* Used to transmit events containing a text string
*
* Text events can be broadcast to all processors, allowing
* messages to be transmitted "backwards" through the signal chain
*
* The TextEvent class is part of the Open Ephys Plugin API
*
*/
class PLUGIN_API TextEvent : public Event
{
public:

	/* Copy constructor*/
	TextEvent(const TextEvent& other);

	/* Destructor*/
	~TextEvent();

	/* Prevent move assignment*/
	TextEvent& operator=(const TextEvent&) = delete;

	/* Serialize to destination buffer*/
	void serialize(void* destinationBuffer, size_t bufferSize) const override;

	/* Get the text contents of this event*/
	String getText() const;

	/* Create a TextEvent*/
	static TextEventPtr createTextEvent(const EventChannel* channelInfo, 
		int64 sampleNumber,
		const String& text);

	/* Create a TextEvent with metadata*/
	static TextEventPtr createTextEvent(const EventChannel* channelInfo, 
		int64 sampleNumber,
		const String& text, 
		const MetadataValueArray& metaData);

	/* Deserialize a TextEvent from an EventPacket object*/
	static TextEventPtr deserialize(const EventPacket& packet, const EventChannel* channelInfo);

	/* Deserialize a TextEvent from a raw byte buffer*/
	static TextEventPtr deserialize(const uint8* buffer, const EventChannel* channelInfo);

private:

	/* Prevent initialization of an empty event*/
	TextEvent() = delete;

	/* Constructor */
	TextEvent(const EventChannel* channelInfo, int64 sampleNumber, const String& text, double timestamp =0.0);

	JUCE_LEAK_DETECTOR(TextEvent);
};

typedef ScopedPointer<BinaryEvent> BinaryEventPtr;

/**
*
* Used to transmit events with a custom binary payload
* 
* NOTE: Binary events are not currently supported by GenericProcesor::checkForEvents()
* To send arbitrary messages through the signal chain, processors should either use
* broadcast messages or append metadata to TTL events.
*
* The BinaryEvent class is part of the Open Ephys Plugin API
*
*/
class PLUGIN_API BinaryEvent
	: public Event
{
public:

	/* Copy constructor*/
	BinaryEvent(const BinaryEvent& other);
	
	/* Destructor */
	~BinaryEvent();

	/* Prevent move assignment */
	BinaryEvent& operator=(const BinaryEvent&) = delete;

	/* Serialize event to a destination buffer */
	void serialize(void* destinationBuffer, size_t bufferSize) const override;

	/* Get a pointer to the binary data */
	const void* getBinaryDataPointer() const;

	/* Get the custom data type for this event */
	EventChannel::BinaryDataType getBinaryType() const;

	/* Create a BinaryEvent */
	template<typename T>
	static BinaryEventPtr createBinaryEvent(const EventChannel* channelInfo, 
                                            int64 sampleNumber,
                                            const T* data,
                                            int dataSize);

	/* Create a BinaryEvent with Metadata */
	template<typename T>
	static BinaryEventPtr createBinaryEvent(const EventChannel* channelInfo, 
                                            int64 sampleNumber,
                                            const T* data,
                                            int dataSize,
                                            const MetadataValueArray& metaData);

	/* Deserialize a BinaryEvent from an EventPacket object */
	static BinaryEventPtr deserialize(const EventPacket& packet, const EventChannel* channelInfo);
	
private:

	/* Prevent creation of an empty event*/
	BinaryEvent() = delete;

	/* Constructor */
	BinaryEvent(const EventChannel* channelInfo, 
		int64 sampleNumber,
		const void* data, 
		EventChannel::BinaryDataType type,
        double timestamp = -1.0);
	
	template<typename T>
	static EventChannel::BinaryDataType getType();

	const EventChannel::BinaryDataType m_type;
	JUCE_LEAK_DETECTOR(BinaryEvent);
};


#endif
