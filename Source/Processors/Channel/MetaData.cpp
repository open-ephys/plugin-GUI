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

#include "MetaData.h"
#include <string>

//Helper method to check type
template <typename T>
bool checkMetaDataType(MetaDataDescriptor::MetaDataTypes baseType)
{
	switch (baseType)
	{
	case MetaDataDescriptor::CHAR: return std::is_same<char, T>::value;
	case MetaDataDescriptor::INT8: return std::is_same<int8, T>::value;
	case MetaDataDescriptor::UINT8: return std::is_same<uint8, T>::value;
	case MetaDataDescriptor::INT16: return std::is_same<int16, T>::value;
	case MetaDataDescriptor::UINT16: return std::is_same<uint16, T>::value;
	case MetaDataDescriptor::INT32: return std::is_same<int32, T>::value;
	case MetaDataDescriptor::UINT32: return std::is_same<uint32, T>::value;
	case MetaDataDescriptor::INT64: return std::is_same<int64, T>::value;
	case MetaDataDescriptor::UINT64: return std::is_same<uint64, T>::value;
	case MetaDataDescriptor::FLOAT: return std::is_same<float, T>::value;
	case MetaDataDescriptor::DOUBLE: return std::is_same<double, T>::value;
	default: return false;
	}
}

//MetaDataDescriptor

MetaDataDescriptor::MetaDataDescriptor(MetaDataDescriptor::MetaDataTypes t, unsigned int length, String n, String d, String id)
	: m_name(n), m_identifier(id), m_description(d), m_type(t), m_length(length)
{}

MetaDataDescriptor::~MetaDataDescriptor() {};

MetaDataDescriptor::MetaDataDescriptor(const MetaDataDescriptor& other)
	:ReferenceCountedObject(),
	m_name(other.m_name), m_identifier(other.m_identifier), m_description(other.m_description),
	m_type(other.m_type), m_length(other.m_length)
{}

MetaDataDescriptor& MetaDataDescriptor::operator=(const MetaDataDescriptor& other)
{
	m_name = other.m_name;
	m_identifier = other.m_identifier;
	m_description = other.m_description;
	m_type = other.m_type;
	m_length = other.m_length;
	return *this;
}

MetaDataDescriptor::MetaDataTypes MetaDataDescriptor::getType() const { return m_type; }
unsigned int MetaDataDescriptor::getLength() const { return m_length; }
size_t MetaDataDescriptor::getDataSize() const 
{ 
	if (m_type == CHAR)
		return m_length*getTypeSize(m_type) + 1; //account for the null-rerminator
	else
		return m_length*getTypeSize(m_type);
}
String MetaDataDescriptor::getName() const { return m_name; }
String MetaDataDescriptor::getDescription() const { return m_description; }
String MetaDataDescriptor::getIdentifier() const { return m_identifier; }

bool MetaDataDescriptor::isSimilar(const MetaDataDescriptor& other) const
{
	if ((m_type == other.m_type) && (m_length == other.m_length))
		return true;
	else
		return false;
}

bool MetaDataDescriptor::isEqual(const MetaDataDescriptor& other) const
{
	if ((m_type == other.m_type) && (m_length == other.m_length) && m_identifier.trim() == other.m_identifier.trim())
		return true;
	else
		return false;
}

bool MetaDataDescriptor::operator==(const MetaDataDescriptor& other) const
{
	return isEqual(other);
}

size_t MetaDataDescriptor::getTypeSize(MetaDataDescriptor::MetaDataTypes type)
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

//MetaDataValue

//This would be so much easier if VS2012 supported delegating constructors...
MetaDataValue::MetaDataValue(MetaDataDescriptor::MetaDataTypes t, unsigned int length, const void* d)
	: m_type(t), m_length(length), m_size(getSize(t, length))
{	
	allocSpace();
	setValue(d);
}

MetaDataValue::MetaDataValue(MetaDataDescriptor::MetaDataTypes t, unsigned int length) 
	: m_type(t), m_length(length), m_size(getSize(t, length))
{
	allocSpace();
}

MetaDataValue::MetaDataValue(const MetaDataDescriptor& m, const void* d)
	: m_type(m.getType()), m_length(m.getLength()), m_size(m.getDataSize())
{
	allocSpace();
	setValue(d);
}

MetaDataValue::MetaDataValue(const MetaDataDescriptor& m)
	: m_type(m.getType()), m_length(m.getLength()), m_size(m.getDataSize())
{
	allocSpace();
}

bool MetaDataValue::isOfType(const MetaDataDescriptor& m) const
{
	return ((m.getType() == m_type) && (m.getLength() == m_length));
}

bool MetaDataValue::isOfType(const MetaDataDescriptor* m) const
{
	return isOfType(*m);
}

MetaDataDescriptor::MetaDataTypes MetaDataValue::getDataType() const
{
	return m_type;
}

unsigned int MetaDataValue::getDataLength() const
{
	return m_length;
}

size_t MetaDataValue::getDataSize() const
{
	return m_size;
}

void MetaDataValue::allocSpace()
{
	m_data.calloc(m_size);
}

size_t MetaDataValue::getSize(MetaDataDescriptor::MetaDataTypes type, unsigned int length)
{
	if (type == MetaDataDescriptor::CHAR)
		return length*MetaDataDescriptor::getTypeSize(type) + 1; //account for the null-rerminator
	else
		return length*MetaDataDescriptor::getTypeSize(type);
}

MetaDataValue::MetaDataValue(const MetaDataValue& v)
	: ReferenceCountedObject(),
	m_type(v.m_type), m_length(v.m_length), m_size(v.m_size)
{
	allocSpace();
	setValue(static_cast<const void*>(v.m_data.getData()));
}

MetaDataValue& MetaDataValue::operator=(const MetaDataValue& v)
{
	m_size = v.m_size;
	m_length = v.m_length;
	m_type = v.m_type;
	allocSpace();
	memcpy(m_data.getData(), v.m_data.getData(), m_size);
	return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
MetaDataValue& MetaDataValue::operator=(MetaDataValue&& v)
{
	m_size = v.m_size;
	m_length = v.m_length;
	m_type = v.m_type;
	m_data.swapWith(v.m_data);
	return *this;
}
#endif

MetaDataValue::~MetaDataValue() {}


void MetaDataValue::setValue(const String& data)
{
	jassert(m_type == MetaDataDescriptor::CHAR);
	jassert(data.length() <= m_size - 1);
	strncpy(m_data.getData(),data.toUTF8().getAddress(), data.length());
}

void MetaDataValue::getValue(String& data) const
{
	jassert(m_type == MetaDataDescriptor::CHAR);
	data = String::createStringFromData(m_data.getData(), m_length);
}

template <typename T>
void MetaDataValue::setValue(T data)
{
	jassert(m_length == 1);
	jassert(checkMetaDataType<T>(m_type));
	*(reinterpret_cast<T*>(m_data.getData())) = data;
}

template <typename T>
void MetaDataValue::getValue(T& data) const
{
	jassert(checkMetaDataType<T>(m_type));
	data = *(reinterpret_cast<T*>(m_data.getData()));
}

template <typename T>
void MetaDataValue::setValue(const T* data)
{
	jassert(checkMetaDataType<T>(m_type));
	memcpy(m_data.getData(), data, m_size);
}

template <typename T>
void MetaDataValue::getValue(T* data) const
{
	jassert(checkMetaDataType<T>(m_type));
	memcpy(data, m_data.getData(), m_size);
}

//Using this version of the method in normal processor operation is not recommended
//However it is useful for format-agnostic processes.
template<>
void MetaDataValue::getValue<void>(void* data) const
{
	memcpy(data, m_data.getData(), m_size);
}

const void* MetaDataValue::getRawValuePointer() const
{
	return m_data.getData();
}

template <typename T>
void MetaDataValue::setValue(const Array<T>& data)
{
	jassert(checkMetaDataType<T>(m_type));
	memcpy(m_data.getData(), data.begin(), m_size);
}

template <typename T>
void MetaDataValue::getValue(Array<T>& data) const
{
	jassert(checkMetaDataType<T>(m_type));
	data.addArray(reinterpret_cast<const T*>(m_data.getData()), m_length);
}

void MetaDataValue::setValue(const void* data)
{
	memcpy(m_data.getData(), data, m_size);
}

//Actual template instantiations at the end of the file

//MetaDataInfoObject

MetaDataInfoObject::MetaDataInfoObject() {}

MetaDataInfoObject::~MetaDataInfoObject() {}

void MetaDataInfoObject::addMetaData(MetaDataDescriptor* desc, MetaDataValue* val)
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
	m_metaDataDescriptorArray.add(desc);
	m_metaDataValueArray.add(val);
}

void MetaDataInfoObject::addMetaData(const MetaDataDescriptor& desc, const MetaDataValue& val)
{
	if (desc.getType() != val.getDataType() || desc.getLength() != val.getDataLength())
	{
		jassertfalse;
		return;
	}
	m_metaDataDescriptorArray.add(new MetaDataDescriptor(desc));
	m_metaDataValueArray.add(new MetaDataValue(val));
}

const MetaDataDescriptor* MetaDataInfoObject::getMetaDataDescriptor(int index) const
{
	return m_metaDataDescriptorArray[index];
}

const MetaDataValue* MetaDataInfoObject::getMetaDataValue(int index) const
{
	return m_metaDataValueArray[index];
}

const int MetaDataInfoObject::getMetaDataCount() const
{
	return m_metaDataDescriptorArray.size();
}

int MetaDataInfoObject::findMetaData(MetaDataDescriptor::MetaDataTypes type, unsigned int length, String identifier) const
{
	int nMetaData = m_metaDataDescriptorArray.size();
	for (int i = 0; i < nMetaData; i++)
	{
		MetaDataDescriptorPtr md = m_metaDataDescriptorArray[i];
		if (md->getType() == type && md->getLength() == length && compareIdentifierStrings(identifier,md->getIdentifier()))
			return i;
	}
	return -1;
}

bool MetaDataInfoObject::hasSameMetadata(const MetaDataInfoObject& other) const
{
	return checkMetaDataCoincidence(other, false);
}

bool MetaDataInfoObject::hasSimilarMetadata(const MetaDataInfoObject& other) const
{
	return checkMetaDataCoincidence(other, true);
}

bool MetaDataInfoObject::checkMetaDataCoincidence(const MetaDataInfoObject& other, bool similar) const
{
	int nMetaData = m_metaDataDescriptorArray.size();
	if (nMetaData != other.m_metaDataDescriptorArray.size()) return false;
	for (int i = 0; i < nMetaData; i++)
	{
		MetaDataDescriptorPtr md = m_metaDataDescriptorArray[i];
		MetaDataDescriptorPtr mdo = other.m_metaDataDescriptorArray[i];
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

//MetaDataEventObject

MetaDataEventObject::MetaDataEventObject() {}

MetaDataEventObject::~MetaDataEventObject() {}

void MetaDataEventObject::addEventMetaData(MetaDataDescriptor* desc)
{
	if (eventMetaDataLock)
	{
		//throw assertion when debugging
		jassertfalse;
		return;
	}
	m_eventMetaDataDescriptorArray.add(desc);
	size_t size = desc->getDataSize();
	m_totalSize += size;
	if (m_maxSize < size)
		m_maxSize = size;
}

void MetaDataEventObject::addEventMetaData(const MetaDataDescriptor& desc)
{
	if (eventMetaDataLock)
	{
		//throw assertion when debugging
		jassertfalse;
		return;
	}
	m_eventMetaDataDescriptorArray.add(new MetaDataDescriptor(desc));
	size_t size = desc.getDataSize();
	m_totalSize += size;
	if (m_maxSize < size)
		m_maxSize = size;
}

size_t MetaDataEventObject::getTotalEventMetaDataSize() const
{
	return m_totalSize;
}

const MetaDataDescriptor* MetaDataEventObject::getEventMetaDataDescriptor(int index) const
{
	return m_eventMetaDataDescriptorArray[index];
}

int MetaDataEventObject::getEventMetaDataCount() const
{
	return m_eventMetaDataDescriptorArray.size();
}

int MetaDataEventObject::findEventMetaData(MetaDataDescriptor::MetaDataTypes type, unsigned int length, String descriptor) const
{
	int nMetaData = m_eventMetaDataDescriptorArray.size();
	for (int i = 0; i < nMetaData; i++)
	{
		MetaDataDescriptorPtr md = m_eventMetaDataDescriptorArray[i];
		if (md->getType() == type && md->getLength() == length && compareIdentifierStrings(descriptor,md->getIdentifier()))
			return i;
	}
	return -1;
}

bool MetaDataEventObject::hasSameEventMetadata(const MetaDataEventObject& other) const
{
	return checkMetaDataCoincidence(other, false);
}

bool MetaDataEventObject::hasSimilarEventMetadata(const MetaDataEventObject& other) const
{
	return checkMetaDataCoincidence(other, true);
}

bool MetaDataEventObject::checkMetaDataCoincidence(const MetaDataEventObject& other, bool similar) const
{
	int nMetaData = m_eventMetaDataDescriptorArray.size();
	if (nMetaData != other.m_eventMetaDataDescriptorArray.size()) return false;
	for (int i = 0; i < nMetaData; i++)
	{
		MetaDataDescriptorPtr md = m_eventMetaDataDescriptorArray[i];
		MetaDataDescriptorPtr mdo = other.m_eventMetaDataDescriptorArray[i];
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

size_t MetaDataEventObject::getMaxEventMetaDataSize() const
{
	return m_maxSize;
}

//MetaDataEvent
MetaDataEvent::MetaDataEvent() {}

MetaDataEvent::~MetaDataEvent() {}

int MetaDataEvent::getMetadataValueCount() const
{
	return m_metaDataValues.size();
}

const MetaDataValue* MetaDataEvent::getMetaDataValue(int index) const
{
	return m_metaDataValues[index];
}

void MetaDataEvent::serializeMetaData(void* dstBuffer) const
{
	int metaDataSize = m_metaDataValues.size();
	char* buffer = static_cast<char*>(dstBuffer);
	size_t ptrIndex = 0;

	for (int i = 0; i < metaDataSize; i++)
	{
		MetaDataValuePtr val = m_metaDataValues[i];
		memcpy(buffer + ptrIndex, val->m_data.getData(), val->m_size);
		ptrIndex += val->m_size;
	}
}

bool MetaDataEvent::deserializeMetaData(const MetaDataEventObject* info, const void* srcBuffer, int size)
{
	MetaDataValueArray metaData;
	int nMetaData = info->getEventMetaDataCount();
	size_t memIndex = 0;
	for (int i = 0; i < nMetaData; i++)
	{
		const MetaDataDescriptor* desc = info->getEventMetaDataDescriptor(i);
		size_t dataSize = desc->getDataSize();
		if ((memIndex + dataSize) > size) return false; //check for buffer boundaries
		
		metaData.add(new MetaDataValue(*desc, (static_cast<const char*>(srcBuffer) + memIndex)));
		memIndex += dataSize;
	}
	m_metaDataValues.swapWith(metaData);
	return true;
}

MetaDataEventLock::MetaDataEventLock() {}

//Specific instantiations for templated metadata members.
//This is done this way for two reasons
//1-To have the actual binary code in the same translation unit, instead of replicated between GUI and plugins, as would happen if
//it were on the header.
//2-To define a fixed set of types that can be used, instead of a general instantiation. Trying to use this with other types
//will throuw a linker error.

template PLUGIN_API void MetaDataValue::setValue<char>(char);
template PLUGIN_API void MetaDataValue::setValue<int8>(int8);
template PLUGIN_API void MetaDataValue::setValue<uint8>(uint8);
template PLUGIN_API void MetaDataValue::setValue<int16>(int16);
template PLUGIN_API void MetaDataValue::setValue<uint16>(uint16);
template PLUGIN_API void MetaDataValue::setValue<int32>(int32);
template PLUGIN_API void MetaDataValue::setValue<uint32>(uint32);
template PLUGIN_API void MetaDataValue::setValue<int64>(int64);
template PLUGIN_API void MetaDataValue::setValue<uint64>(uint64);
template PLUGIN_API void MetaDataValue::setValue<float>(float);
template PLUGIN_API void MetaDataValue::setValue<double>(double);

template PLUGIN_API void MetaDataValue::getValue<char>(char&) const;
template PLUGIN_API void MetaDataValue::getValue<int8>(int8&) const;
template PLUGIN_API void MetaDataValue::getValue<uint8>(uint8&) const;
template PLUGIN_API void MetaDataValue::getValue<int16>(int16&) const;
template PLUGIN_API void MetaDataValue::getValue<uint16>(uint16&) const;
template PLUGIN_API void MetaDataValue::getValue<int32>(int32&) const;
template PLUGIN_API void MetaDataValue::getValue<uint32>(uint32&) const;
template PLUGIN_API void MetaDataValue::getValue<int64>(int64&) const;
template PLUGIN_API void MetaDataValue::getValue<uint64>(uint64&) const;
template PLUGIN_API void MetaDataValue::getValue<float>(float&) const;
template PLUGIN_API void MetaDataValue::getValue<double>(double&) const;

template PLUGIN_API void MetaDataValue::setValue<char>(const char*);
template PLUGIN_API void MetaDataValue::setValue<int8>(const int8*);
template PLUGIN_API void MetaDataValue::setValue<uint8>(const uint8*);
template PLUGIN_API void MetaDataValue::setValue<int16>(const int16*);
template PLUGIN_API void MetaDataValue::setValue<uint16>(const uint16*);
template PLUGIN_API void MetaDataValue::setValue<int32>(const int32*);
template PLUGIN_API void MetaDataValue::setValue<uint32>(const uint32*);
template PLUGIN_API void MetaDataValue::setValue<int64>(const int64*);
template PLUGIN_API void MetaDataValue::setValue<uint64>(const uint64*);
template PLUGIN_API void MetaDataValue::setValue<float>(const float*);
template PLUGIN_API void MetaDataValue::setValue<double>(const double*);

template PLUGIN_API void MetaDataValue::getValue<char>(char*) const;
template PLUGIN_API void MetaDataValue::getValue<int8>(int8*) const;
template PLUGIN_API void MetaDataValue::getValue<uint8>(uint8*) const;
template PLUGIN_API void MetaDataValue::getValue<int16>(int16*) const;
template PLUGIN_API void MetaDataValue::getValue<uint16>(uint16*) const;
template PLUGIN_API void MetaDataValue::getValue<int32>(int32*) const;
template PLUGIN_API void MetaDataValue::getValue<uint32>(uint32*) const;
template PLUGIN_API void MetaDataValue::getValue<int64>(int64*) const;
template PLUGIN_API void MetaDataValue::getValue<uint64>(uint64*) const;
template PLUGIN_API void MetaDataValue::getValue<float>(float*) const;
template PLUGIN_API void MetaDataValue::getValue<double>(double*) const;

template PLUGIN_API void MetaDataValue::setValue<char>(const Array<char>&);
template PLUGIN_API void MetaDataValue::setValue<int8>(const Array<int8>&);
template PLUGIN_API void MetaDataValue::setValue<uint8>(const Array<uint8>&);
template PLUGIN_API void MetaDataValue::setValue<int16>(const Array<int16>&);
template PLUGIN_API void MetaDataValue::setValue<uint16>(const Array<uint16>&);
template PLUGIN_API void MetaDataValue::setValue<int32>(const Array<int32>&);
template PLUGIN_API void MetaDataValue::setValue<uint32>(const Array<uint32>&);
template PLUGIN_API void MetaDataValue::setValue<int64>(const Array<int64>&);
template PLUGIN_API void MetaDataValue::setValue<uint64>(const Array<uint64>&);
template PLUGIN_API void MetaDataValue::setValue<float>(const Array<float>&);
template PLUGIN_API void MetaDataValue::setValue<double>(const Array<double>&);

template PLUGIN_API void MetaDataValue::getValue<char>(Array<char>&) const;
template PLUGIN_API void MetaDataValue::getValue<int8>(Array<int8>&) const;
template PLUGIN_API void MetaDataValue::getValue<uint8>(Array<uint8>&) const;
template PLUGIN_API void MetaDataValue::getValue<int16>(Array<int16>&) const;
template PLUGIN_API void MetaDataValue::getValue<uint16>(Array<uint16>&) const;
template PLUGIN_API void MetaDataValue::getValue<int32>(Array<int32>&) const;
template PLUGIN_API void MetaDataValue::getValue<uint32>(Array<uint32>&) const;
template PLUGIN_API void MetaDataValue::getValue<int64>(Array<int64>&) const;
template PLUGIN_API void MetaDataValue::getValue<uint64>(Array<uint64>&) const;
template PLUGIN_API void MetaDataValue::getValue<float>(Array<float>&) const;
template PLUGIN_API void MetaDataValue::getValue<double>(Array<double>&) const;

template PLUGIN_API void MetaDataValue::getValue<void>(void*) const;

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