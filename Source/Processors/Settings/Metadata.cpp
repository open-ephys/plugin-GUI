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

#include "Metadata.h"

#include <string>

//Helper method to check type
template <typename T>
bool checkMetadataType(MetadataDescriptor::MetadataType baseType)
{
	switch (baseType)
	{
	case MetadataDescriptor::CHAR: return std::is_same<char, T>::value;
	case MetadataDescriptor::INT8: return std::is_same<int8, T>::value;
	case MetadataDescriptor::UINT8: return std::is_same<uint8, T>::value;
	case MetadataDescriptor::INT16: return std::is_same<int16, T>::value;
	case MetadataDescriptor::UINT16: return std::is_same<uint16, T>::value;
	case MetadataDescriptor::INT32: return std::is_same<int32, T>::value;
	case MetadataDescriptor::UINT32: return std::is_same<uint32, T>::value;
	case MetadataDescriptor::INT64: return std::is_same<int64, T>::value;
	case MetadataDescriptor::UINT64: return std::is_same<uint64, T>::value;
	case MetadataDescriptor::FLOAT: return std::is_same<float, T>::value;
	case MetadataDescriptor::DOUBLE: return std::is_same<double, T>::value;
	default: return false;
	}
}

//MetadataDescriptor

MetadataDescriptor::MetadataDescriptor(MetadataDescriptor::MetadataType t, unsigned int length, String n, String d, String id)
	: m_name(n), m_identifier(id), m_description(d), m_type(t), m_length(length)
{}

MetadataDescriptor::~MetadataDescriptor() {};

MetadataDescriptor::MetadataDescriptor(const MetadataDescriptor& other)
	:ReferenceCountedObject(),
	m_name(other.m_name), m_identifier(other.m_identifier), m_description(other.m_description),
	m_type(other.m_type), m_length(other.m_length)
{}

MetadataDescriptor& MetadataDescriptor::operator=(const MetadataDescriptor& other)
{
	m_name = other.m_name;
	m_identifier = other.m_identifier;
	m_description = other.m_description;
	m_type = other.m_type;
	m_length = other.m_length;
	return *this;
}

MetadataDescriptor::MetadataType MetadataDescriptor::getType() const { return m_type; }
unsigned int MetadataDescriptor::getLength() const { return m_length; }
size_t MetadataDescriptor::getDataSize() const 
{ 
	if (m_type == CHAR)
		return m_length*getTypeSize(m_type) + 1; //account for the null-rerminator
	else
		return m_length*getTypeSize(m_type);
}
String MetadataDescriptor::getName() const { return m_name; }
String MetadataDescriptor::getDescription() const { return m_description; }
String MetadataDescriptor::getIdentifier() const { return m_identifier; }

bool MetadataDescriptor::isSimilar(const MetadataDescriptor& other) const
{
	if ((m_type == other.m_type) && (m_length == other.m_length))
		return true;
	else
		return false;
}

bool MetadataDescriptor::isEqual(const MetadataDescriptor& other) const
{
	if ((m_type == other.m_type) && (m_length == other.m_length) && m_identifier.trim() == other.m_identifier.trim())
		return true;
	else
		return false;
}

bool MetadataDescriptor::operator==(const MetadataDescriptor& other) const
{
	return isEqual(other);
}

size_t MetadataDescriptor::getTypeSize(MetadataDescriptor::MetadataType type)
{
	switch (type)
	{
	case INT8: return sizeof(int8);
	case UINT8: return sizeof(uint8);
	case INT16 : return sizeof(int16);
	case UINT16: return sizeof(uint16);
	case INT32: return sizeof(int32);
	case UINT32: return sizeof(uint32);
	case INT64: return sizeof(int64);
	case UINT64: return sizeof(uint64);
	case FLOAT: return sizeof(float);
	case DOUBLE: return sizeof(double);
	case CHAR: return sizeof(unsigned char);
	default: return sizeof(int);
	}
}

//MetadataValue

//This would be so much easier if VS2012 supported delegating constructors...
MetadataValue::MetadataValue(MetadataDescriptor::MetadataType t, unsigned int length, const void* d)
	: m_type(t), m_length(length), m_size(getSize(t, length))
{	
	allocSpace();
	setValue(d);
}

MetadataValue::MetadataValue(MetadataDescriptor::MetadataType t, unsigned int length)
	: m_type(t), m_length(length), m_size(getSize(t, length))
{
	allocSpace();
}

MetadataValue::MetadataValue(const MetadataDescriptor& m, const void* d)
	: m_type(m.getType()), m_length(m.getLength()), m_size(m.getDataSize())
{
	allocSpace();
	setValue(d);
}

MetadataValue::MetadataValue(const MetadataDescriptor& m)
	: m_type(m.getType()), m_length(m.getLength()), m_size(m.getDataSize())
{
	allocSpace();
}

bool MetadataValue::isOfType(const MetadataDescriptor& m) const
{
	return ((m.getType() == m_type) && (m.getLength() == m_length));
}

bool MetadataValue::isOfType(const MetadataDescriptor* m) const
{
	return isOfType(*m);
}

MetadataDescriptor::MetadataType MetadataValue::getDataType() const
{
	return m_type;
}

unsigned int MetadataValue::getDataLength() const
{
	return m_length;
}

size_t MetadataValue::getDataSize() const
{
	return m_size;
}

void MetadataValue::allocSpace()
{
	m_data.calloc(m_size);
}

size_t MetadataValue::getSize(MetadataDescriptor::MetadataType type, unsigned int length)
{
	if (type == MetadataDescriptor::CHAR)
		return length*MetadataDescriptor::getTypeSize(type) + 1; //account for the null-rerminator
	else
		return length*MetadataDescriptor::getTypeSize(type);
}

MetadataValue::MetadataValue(const MetadataValue& v)
	: ReferenceCountedObject(),
	m_type(v.m_type), m_length(v.m_length), m_size(v.m_size)
{
	allocSpace();
	setValue(static_cast<const void*>(v.m_data.getData()));
}

MetadataValue& MetadataValue::operator=(const MetadataValue& v)
{
	m_size = v.m_size;
	m_length = v.m_length;
	m_type = v.m_type;
	allocSpace();
	memcpy(m_data.getData(), v.m_data.getData(), m_size);
	return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
MetadataValue& MetadataValue::operator=(MetadataValue&& v)
{
	m_size = v.m_size;
	m_length = v.m_length;
	m_type = v.m_type;
	m_data.swapWith(v.m_data);
	return *this;
}
#endif

MetadataValue::~MetadataValue() {}


void MetadataValue::setValue(const String& data)
{
	jassert(m_type == MetadataDescriptor::CHAR);
	jassert(data.length() <= m_size - 1);
	strncpy(m_data.getData(),data.toUTF8().getAddress(), data.length());
}

void MetadataValue::getValue(String& data) const
{
	jassert(m_type == MetadataDescriptor::CHAR);
	data = String::createStringFromData(m_data.getData(), m_length);
}

template <typename T>
void MetadataValue::setValue(T data)
{
	jassert(m_length == 1);
	jassert(checkMetadataType<T>(m_type));
	*(reinterpret_cast<T*>(m_data.getData())) = data;
}

template <typename T>
void MetadataValue::getValue(T& data) const
{
	jassert(checkMetadataType<T>(m_type));
	data = *(reinterpret_cast<T*>(m_data.getData()));
}

template <typename T>
void MetadataValue::setValue(const T* data)
{
	jassert(checkMetadataType<T>(m_type));
	memcpy(m_data.getData(), data, m_size);
}

template <typename T>
void MetadataValue::getValue(T* data) const
{
	jassert(checkMetadataType<T>(m_type));
	memcpy(data, m_data.getData(), m_size);
}

//Using this version of the method in normal processor operation is not recommended
//However it is useful for format-agnostic processes.
template<>
void MetadataValue::getValue<void>(void* data) const
{
	memcpy(data, m_data.getData(), m_size);
}

const void* MetadataValue::getRawValuePointer() const
{
	return m_data.getData();
}

template <typename T>
void MetadataValue::setValue(const Array<T>& data)
{
	jassert(checkMetadataType<T>(m_type));
	memcpy(m_data.getData(), data.begin(), m_size);
}

template <typename T>
void MetadataValue::getValue(Array<T>& data) const
{
	jassert(checkMetadataType<T>(m_type));
	data.addArray(reinterpret_cast<const T*>(m_data.getData()), m_length);
}

void MetadataValue::setValue(const void* data)
{
	memcpy(m_data.getData(), data, m_size);
}

//Actual template instantiations at the end of the file

//MetadataObject

MetadataObject::MetadataObject() {}

MetadataObject::~MetadataObject() {}

void MetadataObject::addMetadata(MetadataDescriptor* desc, MetadataValue* val)
{
	if (desc->getType() != val->getDataType() || desc->getLength() != val->getDataLength())
	{
		jassertfalse;
		//This will cause a segfault if the software tries to use the pointers after calling this method
		//Since this method should NEVER be called with non-matching metadata description and value, it's
		//better than just leave dangling pointers.
		delete desc;
		delete val;
		return;
	}
	m_metadataDescriptorArray.add(desc);
	m_metadataValueArray.add(val);
}

void MetadataObject::addMetadata(const MetadataDescriptor& desc, const MetadataValue& val)
{
	if (desc.getType() != val.getDataType() || desc.getLength() != val.getDataLength())
	{
		jassertfalse;
		return;
	}
	m_metadataDescriptorArray.add(new MetadataDescriptor(desc));
	m_metadataValueArray.add(new MetadataValue(val));
}

const MetadataDescriptor* MetadataObject::getMetadataDescriptor(int index) const
{
	return m_metadataDescriptorArray[index];
}

const MetadataValue* MetadataObject::getMetadataValue(int index) const
{
	return m_metadataValueArray[index];
}

const int MetadataObject::getMetadataCount() const
{
	return m_metadataDescriptorArray.size();
}

int MetadataObject::findMetadata(MetadataDescriptor::MetadataType type, unsigned int length, String identifier) const
{
	int nMetadata = m_metadataDescriptorArray.size();
	for (int i = 0; i < nMetadata; i++)
	{
		MetadataDescriptorPtr md = m_metadataDescriptorArray[i];
		if (md->getType() == type && md->getLength() == length && compareIdentifierStrings(identifier,md->getIdentifier()))
			return i;
	}
	return -1;
}

bool MetadataObject::hasSameMetadata(const MetadataObject& other) const
{
	return checkMetadataCoincidence(other, false);
}

bool MetadataObject::hasSimilarMetadata(const MetadataObject& other) const
{
	return checkMetadataCoincidence(other, true);
}

bool MetadataObject::checkMetadataCoincidence(const MetadataObject& other, bool similar) const
{
	int nMetadata = m_metadataDescriptorArray.size();
	if (nMetadata != other.m_metadataDescriptorArray.size()) return false;
	for (int i = 0; i < nMetadata; i++)
	{
		MetadataDescriptorPtr md = m_metadataDescriptorArray[i];
		MetadataDescriptorPtr mdo = other.m_metadataDescriptorArray[i];
		if (similar)
		{
			if (!md->isSimilar(*mdo)) return false;
		}
		else
		{
			if (!md->isEqual(*mdo)) return false;
		}
	}
	return true;
}

//MetadataEventObject

MetadataEventObject::MetadataEventObject() {}

MetadataEventObject::~MetadataEventObject() {}

void MetadataEventObject::addEventMetadata(MetadataDescriptor* desc)
{
	if (eventMetadataLock)
	{
		//throw assertion when debugging
		jassertfalse;
		return;
	}
	m_eventMetadataDescriptorArray.add(desc);
	size_t size = desc->getDataSize();
	m_totalSize += size;
	if (m_maxSize < size)
		m_maxSize = size;
}

void MetadataEventObject::addEventMetadata(const MetadataDescriptor& desc)
{
	if (eventMetadataLock)
	{
		//throw assertion when debugging
		jassertfalse;
		return;
	}
	m_eventMetadataDescriptorArray.add(new MetadataDescriptor(desc));
	size_t size = desc.getDataSize();
	m_totalSize += size;
	if (m_maxSize < size)
		m_maxSize = size;
}

size_t MetadataEventObject::getTotalEventMetadataSize() const
{
	return m_totalSize;
}

const MetadataDescriptor* MetadataEventObject::getEventMetadataDescriptor(int index) const
{
	return m_eventMetadataDescriptorArray[index];
}

int MetadataEventObject::getEventMetadataCount() const
{
	return m_eventMetadataDescriptorArray.size();
}

int MetadataEventObject::findEventMetadata(MetadataDescriptor::MetadataType type, unsigned int length, String descriptor) const
{
	int nMetadata = m_eventMetadataDescriptorArray.size();
	for (int i = 0; i < nMetadata; i++)
	{
		MetadataDescriptorPtr md = m_eventMetadataDescriptorArray[i];
		if (md->getType() == type && md->getLength() == length && compareIdentifierStrings(descriptor,md->getIdentifier()))
			return i;
	}
	return -1;
}

bool MetadataEventObject::hasSameEventMetadata(const MetadataEventObject& other) const
{
	return checkMetadataCoincidence(other, false);
}

bool MetadataEventObject::hasSimilarEventMetadata(const MetadataEventObject& other) const
{
	return checkMetadataCoincidence(other, true);
}

bool MetadataEventObject::checkMetadataCoincidence(const MetadataEventObject& other, bool similar) const
{
	int nMetadata = m_eventMetadataDescriptorArray.size();
	if (nMetadata != other.m_eventMetadataDescriptorArray.size()) return false;
	
    for (int i = 0; i < nMetadata; i++)
	{
		MetadataDescriptorPtr md = m_eventMetadataDescriptorArray[i];
		MetadataDescriptorPtr mdo = other.m_eventMetadataDescriptorArray[i];
		
        if (similar)
		{
			if (!md->isSimilar(*mdo)) return false;
		}
		else
		{
			if (!md->isEqual(*mdo)) return false;
		}
	}
	return true;
}

size_t MetadataEventObject::getMaxEventMetadataSize() const
{
	return m_maxSize;
}

//MetadataEvent
MetadataEvent::MetadataEvent() {}

MetadataEvent::~MetadataEvent() {}

int MetadataEvent::getMetadataValueCount() const
{
	return m_metaDataValues.size();
}

const MetadataValue* MetadataEvent::getMetadataValue(int index) const
{
	return m_metaDataValues[index];
}

void MetadataEvent::serializeMetadata(void* dstBuffer) const
{
	int metaDataSize = m_metaDataValues.size();
	char* buffer = static_cast<char*>(dstBuffer);
	size_t ptrIndex = 0;

	for (int i = 0; i < metaDataSize; i++)
	{
		MetadataValuePtr val = m_metaDataValues[i];
		memcpy(buffer + ptrIndex, val->m_data.getData(), val->m_size);
		ptrIndex += val->m_size;
	}
}

bool MetadataEvent::deserializeMetadata(const MetadataEventObject* info, const void* srcBuffer, int size)
{
	MetadataValueArray metaData;
	int nMetadata = info->getEventMetadataCount();
	size_t memIndex = 0;
	for (int i = 0; i < nMetadata; i++)
	{
		const MetadataDescriptor* desc = info->getEventMetadataDescriptor(i);
		size_t dataSize = desc->getDataSize();
		if ((memIndex + dataSize) > size) return false; //check for buffer boundaries
		
		metaData.add(new MetadataValue(*desc, (static_cast<const char*>(srcBuffer) + memIndex)));
		memIndex += dataSize;
	}
	m_metaDataValues.swapWith(metaData);
	return true;
}

MetadataEventLock::MetadataEventLock() {}

//Specific instantiations for templated metadata members.
//This is done this way for two reasons
//1-To have the actual binary code in the same translation unit, instead of replicated between GUI and plugins, as would happen if
//it were on the header.
//2-To define a fixed set of types that can be used, instead of a general instantiation. Trying to use this with other types
//will throuw a linker error.

template PLUGIN_API void MetadataValue::setValue<char>(char);
template PLUGIN_API void MetadataValue::setValue<int8>(int8);
template PLUGIN_API void MetadataValue::setValue<uint8>(uint8);
template PLUGIN_API void MetadataValue::setValue<int16>(int16);
template PLUGIN_API void MetadataValue::setValue<uint16>(uint16);
template PLUGIN_API void MetadataValue::setValue<int32>(int32);
template PLUGIN_API void MetadataValue::setValue<uint32>(uint32);
template PLUGIN_API void MetadataValue::setValue<int64>(int64);
template PLUGIN_API void MetadataValue::setValue<uint64>(uint64);
template PLUGIN_API void MetadataValue::setValue<float>(float);
template PLUGIN_API void MetadataValue::setValue<double>(double);

template PLUGIN_API void MetadataValue::getValue<char>(char&) const;
template PLUGIN_API void MetadataValue::getValue<int8>(int8&) const;
template PLUGIN_API void MetadataValue::getValue<uint8>(uint8&) const;
template PLUGIN_API void MetadataValue::getValue<int16>(int16&) const;
template PLUGIN_API void MetadataValue::getValue<uint16>(uint16&) const;
template PLUGIN_API void MetadataValue::getValue<int32>(int32&) const;
template PLUGIN_API void MetadataValue::getValue<uint32>(uint32&) const;
template PLUGIN_API void MetadataValue::getValue<int64>(int64&) const;
template PLUGIN_API void MetadataValue::getValue<uint64>(uint64&) const;
template PLUGIN_API void MetadataValue::getValue<float>(float&) const;
template PLUGIN_API void MetadataValue::getValue<double>(double&) const;

template PLUGIN_API void MetadataValue::setValue<char>(const char*);
template PLUGIN_API void MetadataValue::setValue<int8>(const int8*);
template PLUGIN_API void MetadataValue::setValue<uint8>(const uint8*);
template PLUGIN_API void MetadataValue::setValue<int16>(const int16*);
template PLUGIN_API void MetadataValue::setValue<uint16>(const uint16*);
template PLUGIN_API void MetadataValue::setValue<int32>(const int32*);
template PLUGIN_API void MetadataValue::setValue<uint32>(const uint32*);
template PLUGIN_API void MetadataValue::setValue<int64>(const int64*);
template PLUGIN_API void MetadataValue::setValue<uint64>(const uint64*);
template PLUGIN_API void MetadataValue::setValue<float>(const float*);
template PLUGIN_API void MetadataValue::setValue<double>(const double*);

template PLUGIN_API void MetadataValue::getValue<char>(char*) const;
template PLUGIN_API void MetadataValue::getValue<int8>(int8*) const;
template PLUGIN_API void MetadataValue::getValue<uint8>(uint8*) const;
template PLUGIN_API void MetadataValue::getValue<int16>(int16*) const;
template PLUGIN_API void MetadataValue::getValue<uint16>(uint16*) const;
template PLUGIN_API void MetadataValue::getValue<int32>(int32*) const;
template PLUGIN_API void MetadataValue::getValue<uint32>(uint32*) const;
template PLUGIN_API void MetadataValue::getValue<int64>(int64*) const;
template PLUGIN_API void MetadataValue::getValue<uint64>(uint64*) const;
template PLUGIN_API void MetadataValue::getValue<float>(float*) const;
template PLUGIN_API void MetadataValue::getValue<double>(double*) const;

template PLUGIN_API void MetadataValue::setValue<char>(const Array<char>&);
template PLUGIN_API void MetadataValue::setValue<int8>(const Array<int8>&);
template PLUGIN_API void MetadataValue::setValue<uint8>(const Array<uint8>&);
template PLUGIN_API void MetadataValue::setValue<int16>(const Array<int16>&);
template PLUGIN_API void MetadataValue::setValue<uint16>(const Array<uint16>&);
template PLUGIN_API void MetadataValue::setValue<int32>(const Array<int32>&);
template PLUGIN_API void MetadataValue::setValue<uint32>(const Array<uint32>&);
template PLUGIN_API void MetadataValue::setValue<int64>(const Array<int64>&);
template PLUGIN_API void MetadataValue::setValue<uint64>(const Array<uint64>&);
template PLUGIN_API void MetadataValue::setValue<float>(const Array<float>&);
template PLUGIN_API void MetadataValue::setValue<double>(const Array<double>&);

template PLUGIN_API void MetadataValue::getValue<char>(Array<char>&) const;
template PLUGIN_API void MetadataValue::getValue<int8>(Array<int8>&) const;
template PLUGIN_API void MetadataValue::getValue<uint8>(Array<uint8>&) const;
template PLUGIN_API void MetadataValue::getValue<int16>(Array<int16>&) const;
template PLUGIN_API void MetadataValue::getValue<uint16>(Array<uint16>&) const;
template PLUGIN_API void MetadataValue::getValue<int32>(Array<int32>&) const;
template PLUGIN_API void MetadataValue::getValue<uint32>(Array<uint32>&) const;
template PLUGIN_API void MetadataValue::getValue<int64>(Array<int64>&) const;
template PLUGIN_API void MetadataValue::getValue<uint64>(Array<uint64>&) const;
template PLUGIN_API void MetadataValue::getValue<float>(Array<float>&) const;
template PLUGIN_API void MetadataValue::getValue<double>(Array<double>&) const;

/// Compiler warning: Explicit instantiation of `getValue<void>` that occurs after an explicit specialization has no effect
//template PLUGIN_API void MetadataValue::getValue<void>(void*) const;

//Helper function to compare identifier strings
bool compareIdentifierStrings(const String& identifier, const String& compareWith)
{
	if (identifier.isEmpty()) return true;

	int i1 = 0, i2 = 0;
	StringArray s1 = StringArray::fromTokens(identifier, ".", "");
	StringArray s2 = StringArray::fromTokens(compareWith, ".", "");
	int n1 = s1.size();
	int n2 = s2.size();

	while (i1 < n1 && i2 < n2)
	{
		bool wild = false;
		if (s1[i1] == "*")
		{
			do
			{
				i1++;
				if (i1 >= n1) return true;
			} while (s1[i1] != "*");
				wild = true;
		}
		bool match = false;
		do
		{
			if (i2 >= n2) return false;
			if (s1[i1].equalsIgnoreCase(s2[i2]))
				match = true;
			else if (!wild)
				return false;
			i2++;

		} while (!match);
		i1++;
	}
	if (i2 < n2) return false;
	else return true;
}
