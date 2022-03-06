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

#ifndef EVENTCHANNEL_H_INCLUDED
#define EVENTCHANNEL_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "Metadata.h"
#include "InfoObject.h"

/**
*
* Holds information about a channel capable of sending Events
* between processors.
* 
* An EventChannel can send TTL (most common), TEXT, or CUSTOM events.
*
* The EventChannel class is part of the Open Ephys Plugin API
*
*/
class PLUGIN_API EventChannel : public ChannelInfoObject, public MetadataEventObject
{
public:

	/**
	* Supported EventChannel types
	*
	* These determine the type of events the EventChannel can transmit
	*/
	enum Type
	{
		// Most common - used for tracking the status of up to 256 TTL bits
		TTL = 3,

		// Used by the MessageCenter to broadcast messages to all processors;
		// not commonly used by plugins
		TEXT = 5,

		// Used to send custom events as binary blobs (not commonly used)
		CUSTOM = 7,

		//For error checking
		INVALID = 100,
	};

	/**
	* Supported data types for Event payload
	*
	* These should be treated by the RecordEngines as simple binary blobs when writing to disk
	*/
	enum BinaryDataType
	{
		BINARY_BASE_VALUE = 10,
		CHAR_ARRAY,
		INT8_ARRAY,
		UINT8_ARRAY,
		INT16_ARRAY,
		UINT16_ARRAY,
		INT32_ARRAY,
		UINT32_ARRAY,
		INT64_ARRAY,
		UINT64_ARRAY,
		FLOAT_ARRAY,
		DOUBLE_ARRAY,
	};

	/* Settings required to initialize an EventChannel*/
	struct Settings
	{
		Type type;

		String name;
		String description;
		String identifier;

		DataStream* stream;

		int maxTTLBits = 8;

		BinaryDataType customDataType = BINARY_BASE_VALUE;
		int customDataLength = 0;

	};

	/** Default constructor
	@param settings - The settings for this channel.
	*/
	EventChannel(Settings settings);

	/* Destructor */
	virtual ~EventChannel();

	/* Get the EventChannel type (TTL, TEXT, or CUSTOM) */
	const Type getType() const;

	/* Get the EventChannel binary data type (INT8, INT32, etc.) */
	const BinaryDataType getBinaryDataType() const;

	/** Gets the size of the event payload
		-For TTL, it returns the number of bytes used to track bit status
		-For message events, the number of characters
		-For custom events, the number of elements in the typed array.
	*/
	unsigned int getLength() const;

	/** Gets the size of the event payload in bytes*/
	size_t getDataSize() const;

	/** Gets the size of the event payload in bytes*/
	int getMaxTTLBits() const;

	/** Gets the size in bytes of an element depending of the type*/
	static size_t getBinaryDataTypeSize(BinaryDataType type);

	/** Handy method to get an equivalent metadata value type for the main event data*/
	static BaseType getEquivalentMetadataType(const EventChannel& ev);

	/** Handy method to get an equivalent metadata value type for the main event data*/
	BaseType getEquivalentMetadataType() const;

	/** Sets the label for a particular line (for TTL events)*/
	void setLineLabel(int line, String label);

	/** Sets the label for a particular line (for TTL events)*/
	String getLineLabel(int line) const;

	/** Set the state of a TTL line (used for tracking TTL word states)*/
	void setLineState(int line, bool state);

	/** Returns the current 64-bit TTL word for this channel */
	uint64 getTTLWord();

private:

	const Type m_type;
	BinaryDataType m_binaryDataType;
	size_t m_dataSize;
	unsigned int m_length;
	unsigned int m_maxTTLBits;
	uint64 m_TTLWord;

	Array<String> lineLabels;

	JUCE_LEAK_DETECTOR(EventChannel);
};


#endif