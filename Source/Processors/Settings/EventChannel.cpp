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

#include "EventChannel.h"
#include "DataStream.h"

#define MAX_MSG_LENGTH 512

EventChannel::EventChannel(Settings settings) :
	InfoObject(InfoObject::Type::EVENT_CHANNEL),
	m_type(settings.type), m_maxTTLBits(settings.maxTTLBits)
{
	setName(settings.name);
	setDescription(settings.description);
	setIdentifier(settings.identifier);

	if (m_type == TTL)
	{
		// 1 byte for the bit number
		// 1 byte for the status
		jassert(m_maxTTLBits <= 256);

		m_length = 2; 
		m_dataSize = 2;
		m_binaryDataType = UINT8_ARRAY;
	}
	else if (m_type == TEXT)
	{
		m_length = MAX_MSG_LENGTH;
		m_dataSize = MAX_MSG_LENGTH + 1; // add 1 for termination char
		m_binaryDataType = CHAR_ARRAY;
	}
	else if (m_type == CUSTOM)
	{
		jassert(settings.customDataType > BINARY_BASE_VALUE);
		jassert(settings.customDataLength > 0);

		m_length = settings.customDataLength;
		m_dataSize = m_length * getBinaryDataTypeSize(settings.customDataType);
	}
}

EventChannel::~EventChannel()
{
}

const EventChannel::Type EventChannel::getType() const
{
	return m_type;
}

const EventChannel::BinaryDataType EventChannel::getBinaryDataType() const
{
	return m_binaryDataType;
}


unsigned int EventChannel::getLength() const
{
	return m_length;
}


size_t EventChannel::getDataSize() const
{
	return m_dataSize;
}

int EventChannel::getMaxTTLBits() const
{
	return m_maxTTLBits;
}



size_t EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType type)
{
	switch (type)
	{
	case INT8_ARRAY: return sizeof(int8);
	case UINT8_ARRAY: return sizeof(uint8);
	case INT16_ARRAY: return sizeof(int16);
	case UINT16_ARRAY: return sizeof(uint16);
	case INT32_ARRAY: return sizeof(int32);
	case UINT32_ARRAY: return sizeof(uint32);
	case INT64_ARRAY: return sizeof(int64);
	case UINT64_ARRAY: return sizeof(uint64);
	case FLOAT_ARRAY: return sizeof(float);
	case DOUBLE_ARRAY: return sizeof(double);
	default: return sizeof(char);
	}
}

BaseType EventChannel::getEquivalentMetadataType(const EventChannel& ev)
{
	switch (ev.getBinaryDataType())
	{
	case EventChannel::TEXT:
		return MetadataDescriptor::CHAR;
	case EventChannel::INT8_ARRAY:
		return MetadataDescriptor::INT8;
	case EventChannel::UINT8_ARRAY:
		return MetadataDescriptor::UINT8;
	case EventChannel::INT16_ARRAY:
		return MetadataDescriptor::INT16;
	case EventChannel::UINT16_ARRAY:
		return MetadataDescriptor::UINT16;
	case EventChannel::INT32_ARRAY:
		return MetadataDescriptor::INT32;
	case EventChannel::UINT32_ARRAY:
		return MetadataDescriptor::UINT32;
	case EventChannel::INT64_ARRAY:
		return MetadataDescriptor::INT64;
	case EventChannel::UINT64_ARRAY:
		return MetadataDescriptor::UINT64;
	case EventChannel::FLOAT_ARRAY:
		return MetadataDescriptor::FLOAT;
	case EventChannel::DOUBLE_ARRAY:
		return MetadataDescriptor::DOUBLE;
	default:
		return MetadataDescriptor::UINT8;
	}
}

BaseType EventChannel::getEquivalentMetadataType() const
{
	return getEquivalentMetadataType(*this);
}

/*bool EventChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
{
	const EventChannel& o = dynamic_cast<const EventChannel&>(other);
	if (m_type != o.m_type) return false;
	if (m_numChannels != o.m_numChannels) return false;
	if (m_length != o.m_length) return false;
	if (similar && !hasSimilarMetadata(o)) return false;
	if (!similar && !hasSameMetadata(o)) return false;
	if (similar && !hasSimilarEventMetadata(o)) return false;
	if (!similar && !hasSameEventMetadata(o)) return false;
	return true;
}*/

/*EventChannel::EventTimestampOrigin EventChannel::getTimestampOrigin() const
{
	return m_timestampOrigin;
}

uint16 EventChannel::getTimestampOriginProcessor() const
{
	return m_timestampOriginProcessor;
}

uint16 EventChannel::getTimestampOriginSubProcessor() const
{
	return m_timestampOriginSubProcessor;
}*/
