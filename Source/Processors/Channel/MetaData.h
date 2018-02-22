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

#ifndef METADATA_H_INCLUDED
#define METADATA_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

class GenericProcessor;
class MetaDataEvent;

/**
Metadata Objects
Any extra info a processor wants to define that can't fit inside the standard fields can be defined as metadata.
A metadata value can be of any type of the listed in MetaDataTypes and with any dimension. For example,
a metadata value of type INTEGER and size 3 would be an array of 3 integers. For strings, simply set
a type of CHAR and a value equal to the maximum length of the string plus one, for the null char.
*/

class PLUGIN_API MetaDataDescriptor
	: public ReferenceCountedObject
{
public:
	enum MetaDataTypes
	{
		CHAR,
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT32,
		UINT32,
		INT64,
		UINT64,
		FLOAT,
		DOUBLE
	};

	/**
	MetaData descriptor constructor
	@param type The primitive type this metadata field will hold
	@param length The length of the data. 1 for single value or mroe for arrays.
	@param name The human-readable name of this metadata field
	@param description A human-readable description of what this field represents
	@param identifier A simple machine-readable name for this metadata value

	name and humanDescription will be saved in most data formats for latter reference
	*/
	MetaDataDescriptor(MetaDataTypes type, unsigned int length, String name, String description, String identifier);
	~MetaDataDescriptor();
	MetaDataDescriptor(const MetaDataDescriptor& other);
	MetaDataDescriptor& operator=(const MetaDataDescriptor& other);

	/** Gets the primitive type of this field */
	MetaDataTypes getType() const;
	/** Gets the number of elements in this field */
	unsigned int getLength() const;
	/** Gets the total data in bytes for this field */
	size_t getDataSize() const;
	/** Gets the human-readable name of this field */
	String getName() const;
	/** Gets the human-readable description of the field */
	String getDescription() const;
	/** Gets the machine-readable identifier for this field */
	String getIdentifier() const;

	/** Returns true if both descriptors have the same type, length and identifier */
	bool isEqual(const MetaDataDescriptor& other) const;
	/** Returns true if both descriptors have the same type, length and identifier */
	bool operator==(const MetaDataDescriptor& other) const;
	/** Returns true if both descriptors have the same type and length, regardless of the identifier */
	bool isSimilar(const MetaDataDescriptor& other) const;

	static size_t getTypeSize(MetaDataTypes type);
private:
	MetaDataDescriptor() = delete;
	String m_name;
	String m_identifier;
	String m_description;
	MetaDataTypes m_type;
	unsigned int m_length;

	JUCE_LEAK_DETECTOR(MetaDataDescriptor);
};

class PLUGIN_API MetaDataValue
	: public ReferenceCountedObject
{
	friend MetaDataEvent; //The Serialize method must access the raw data pointer.
public:
	MetaDataValue(MetaDataDescriptor::MetaDataTypes type, unsigned int length);
	MetaDataValue(const MetaDataDescriptor& desc);
	//To be able to set value at object creation
	MetaDataValue(MetaDataDescriptor::MetaDataTypes type, unsigned int length, const void* data);
	MetaDataValue(const MetaDataDescriptor& desc, const void* data);

	bool isOfType(const MetaDataDescriptor& desc) const;
	bool isOfType(const MetaDataDescriptor* desc) const;

	MetaDataDescriptor::MetaDataTypes getDataType() const;
	unsigned int getDataLength() const;
	size_t getDataSize() const;

	MetaDataValue(const MetaDataValue&);
	MetaDataValue& operator= (const MetaDataValue&);
#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
	MetaDataValue& operator= (MetaDataValue&&);
#endif
	~MetaDataValue();

	//Get-set for strings
	void setValue(const String& data);
	void getValue(String& data) const;

	//Get-set for single-size types. Defined only for the types present on MetaDataTypes
	template <typename T>
	void setValue(T data);

	template <typename T>
	void getValue(T& data) const;

	//Get set for arrays, both in raw form and in juce array form. Defined only for the types present on MetaDataTypes
	template <typename T>
	void setValue(const T* data);

	template <typename T>
	void getValue(T* data) const;

	template <typename T>
	void setValue(const Array<T>& data);

	template <typename T>
	void getValue(Array<T>& data) const;

	/** It is generally not advised to use this method, which can, however, be of use in performance critical modules.
	It is usually preferable to use the strongly typed copy getter methods, when speed is not an issue.
	Keep in mind that any pointer returned by this will become invalid after the block execution.*/
	const void* getRawValuePointer() const;

private:
	MetaDataValue() = delete;
	void setValue(const void* data);
	void allocSpace();
	static size_t getSize(MetaDataDescriptor::MetaDataTypes type, unsigned int length);
	HeapBlock<char> m_data;
	MetaDataDescriptor::MetaDataTypes m_type;
	unsigned int m_length;
	size_t m_size;

	JUCE_LEAK_DETECTOR(MetaDataValue);
};

typedef ReferenceCountedArray<MetaDataDescriptor,CriticalSection> MetaDataDescriptorArray;
typedef ReferenceCountedArray<MetaDataValue,CriticalSection> MetaDataValueArray;
typedef ReferenceCountedObjectPtr<MetaDataDescriptor> MetaDataDescriptorPtr;
typedef ReferenceCountedObjectPtr<MetaDataValue> MetaDataValuePtr;

//Inherited for all info objects that have metadata
class PLUGIN_API MetaDataInfoObject
{
protected:
	MetaDataInfoObject();
public:
    virtual ~MetaDataInfoObject();
	void addMetaData(MetaDataDescriptor* desc, MetaDataValue* val);
	void addMetaData(const MetaDataDescriptor& desc, const MetaDataValue& val);
	const MetaDataDescriptor* getMetaDataDescriptor(int index) const;
	const MetaDataValue* getMetaDataValue(int index) const;
	int findMetaData(MetaDataDescriptor::MetaDataTypes type, unsigned int length, String identifier = String::empty) const;
	const int getMetaDataCount() const;
	bool hasSameMetadata(const MetaDataInfoObject& other) const;
	bool hasSimilarMetadata(const MetaDataInfoObject& other) const;
protected:
	MetaDataDescriptorArray m_metaDataDescriptorArray;
	MetaDataValueArray m_metaDataValueArray;
private:
	bool checkMetaDataCoincidence(const MetaDataInfoObject& other, bool similar) const;
};

class PLUGIN_API MetaDataEventLock
{
	//GenericProcessor will set this to true when copying channels in the update method so no other processor but the one which
	//created the object can call addEventMetaData. This is done this way because since the events themselves are created by the
	//source processot and not modified, changing the metadata information will cause errors when trying to decode the data embedded
	//in the event itself.
	friend class GenericProcessor;
protected:
	bool eventMetaDataLock{ false };
	MetaDataEventLock();
};

//Special class for event and spike info objects, whose events can hold extra metadata
class PLUGIN_API MetaDataEventObject : public MetaDataEventLock
{
public:
    virtual ~MetaDataEventObject();
	//This method will only work when creating the info object, but not for those copied down the chain
	void addEventMetaData(MetaDataDescriptor* desc);
	void addEventMetaData(const MetaDataDescriptor& desc);
	const MetaDataDescriptor* getEventMetaDataDescriptor(int index) const;
	int findEventMetaData(MetaDataDescriptor::MetaDataTypes type, unsigned int length, String identifier = String::empty) const;
	size_t getTotalEventMetaDataSize() const;
	int getEventMetaDataCount() const;
	//gets the largest metadata size, which can be useful to reserve buffers in advance
	size_t getMaxEventMetaDataSize() const;
	bool hasSameEventMetadata(const MetaDataEventObject& other) const;
	bool hasSimilarEventMetadata(const MetaDataEventObject& other) const;
protected:
	MetaDataDescriptorArray m_eventMetaDataDescriptorArray;
	MetaDataEventObject();
	size_t m_totalSize{ 0 };
	size_t m_maxSize{ 0 };
private:
	bool checkMetaDataCoincidence(const MetaDataEventObject& other, bool similar) const;
};

//And the base from which event objects can hold their metadata before serializing
class PLUGIN_API MetaDataEvent
{
public:
    virtual ~MetaDataEvent();
	int getMetadataValueCount() const;
	const MetaDataValue* getMetaDataValue(int index) const;
protected:
	void serializeMetaData(void* dstBuffer) const;
	bool deserializeMetaData(const MetaDataEventObject* info, const void* srcBuffer, int size);
	MetaDataEvent();
	MetaDataValueArray m_metaDataValues;
};

//Helper function to compare identifier strings
bool compareIdentifierStrings(const String& identifier, const String& compareWith);

typedef MetaDataDescriptor::MetaDataTypes BaseType;

#endif