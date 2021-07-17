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
class MetadataEvent;

/**
Metadata Objects
Any extra info a processor wants to define that can't fit inside the standard fields can be defined as metadata.
A metadata value can be of any type of the listed in MetadataTypes and with any dimension. For example,
a metadata value of type INTEGER and size 3 would be an array of 3 integers. For strings, simply set
a type of CHAR and a value equal to the maximum length of the string plus one, for the null char.
*/

class PLUGIN_API MetadataDescriptor
	: public ReferenceCountedObject
{
public:
	enum MetadataTypes
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
	Metadata descriptor constructor
	@param type The primitive type this metadata field will hold
	@param length The length of the data. 1 for single value or mroe for arrays.
	@param name The human-readable name of this metadata field
	@param description A human-readable description of what this field represents
	@param identifier A simple machine-readable name for this metadata value

	name and humanDescription will be saved in most data formats for latter reference
	*/
	MetadataDescriptor(MetadataTypes type, unsigned int length, String name, String description, String identifier);
	~MetadataDescriptor();
	MetadataDescriptor(const MetadataDescriptor& other);
	MetadataDescriptor& operator=(const MetadataDescriptor& other);

	/** Gets the primitive type of this field */
	MetadataTypes getType() const;
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
	bool isEqual(const MetadataDescriptor& other) const;
	/** Returns true if both descriptors have the same type, length and identifier */
	bool operator==(const MetadataDescriptor& other) const;
	/** Returns true if both descriptors have the same type and length, regardless of the identifier */
	bool isSimilar(const MetadataDescriptor& other) const;

	static size_t getTypeSize(MetadataTypes type);
private:
	MetadataDescriptor() = delete;
	String m_name;
	String m_identifier;
	String m_description;
	MetadataTypes m_type;
	unsigned int m_length;

	JUCE_LEAK_DETECTOR(MetadataDescriptor);
};

class PLUGIN_API MetadataValue
	: public ReferenceCountedObject
{
	friend MetadataEvent; //The Serialize method must access the raw data pointer.
public:
	MetadataValue(MetadataDescriptor::MetadataTypes type, unsigned int length);
	MetadataValue(const MetadataDescriptor& desc);
	//To be able to set value at object creation
	MetadataValue(MetadataDescriptor::MetadataTypes type, unsigned int length, const void* data);
	MetadataValue(const MetadataDescriptor& desc, const void* data);

	bool isOfType(const MetadataDescriptor& desc) const;
	bool isOfType(const MetadataDescriptor* desc) const;

	MetadataDescriptor::MetadataTypes getDataType() const;
	unsigned int getDataLength() const;
	size_t getDataSize() const;

	MetadataValue(const MetadataValue&);
	MetadataValue& operator= (const MetadataValue&);
#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
	MetadataValue& operator= (MetadataValue&&);
#endif
	~MetadataValue();

	//Get-set for strings
	void setValue(const String& data);
	void getValue(String& data) const;

	//Get-set for single-size types. Defined only for the types present on MetadataTypes
	template <typename T>
	void setValue(T data);

	template <typename T>
	void getValue(T& data) const;

	//Get set for arrays, both in raw form and in juce array form. Defined only for the types present on MetadataTypes
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
	MetadataValue() = delete;
	void setValue(const void* data);
	void allocSpace();
	static size_t getSize(MetadataDescriptor::MetadataTypes type, unsigned int length);
	HeapBlock<char> m_data;
	MetadataDescriptor::MetadataTypes m_type;
	unsigned int m_length;
	size_t m_size;

	JUCE_LEAK_DETECTOR(MetadataValue);
};

typedef ReferenceCountedArray<MetadataDescriptor,CriticalSection> MetadataDescriptorArray;
typedef ReferenceCountedArray<MetadataValue,CriticalSection> MetadataValueArray;
typedef ReferenceCountedObjectPtr<MetadataDescriptor> MetadataDescriptorPtr;
typedef ReferenceCountedObjectPtr<MetadataValue> MetadataValuePtr;

//Inherited for all info objects that have metadata
class PLUGIN_API MetadataObject
{
protected:
	MetadataObject();
public:
    virtual ~MetadataObject();
	void addMetadata(MetadataDescriptor* desc, MetadataValue* val);
	void addMetadata(const MetadataDescriptor& desc, const MetadataValue& val);
	const MetadataDescriptor* getMetadataDescriptor(int index) const;
	const MetadataValue* getMetadataValue(int index) const;
	int findMetadata(MetadataDescriptor::MetadataTypes type, unsigned int length, String identifier = String()) const;
	const int getMetadataCount() const;
	bool hasSameMetadata(const MetadataObject& other) const;
	bool hasSimilarMetadata(const MetadataObject& other) const;
protected:
	MetadataDescriptorArray m_metadataDescriptorArray;
	MetadataValueArray m_metadataValueArray;
private:
	bool checkMetadataCoincidence(const MetadataObject& other, bool similar) const;
};

class PLUGIN_API MetadataEventLock
{
	//GenericProcessor will set this to true when copying channels in the update method so no other processor but the one which
	//created the object can call addEventMetadata. This is done this way because since the events themselves are created by the
	//source processot and not modified, changing the metadata information will cause errors when trying to decode the data embedded
	//in the event itself.
	friend class GenericProcessor;
protected:
	bool eventMetadataLock{ false };
	MetadataEventLock();
};

//Special class for event and spike info objects, whose events can hold extra metadata
class PLUGIN_API MetadataEventObject : public MetadataEventLock
{
public:
    virtual ~MetadataEventObject();
	//This method will only work when creating the info object, but not for those copied down the chain
	void addEventMetadata(MetadataDescriptor* desc);
	void addEventMetadata(const MetadataDescriptor& desc);
	const MetadataDescriptor* getEventMetadataDescriptor(int index) const;
	int findEventMetadata(MetadataDescriptor::MetadataTypes type, unsigned int length, String identifier = String()) const;
	size_t getTotalEventMetadataSize() const;
	int getEventMetadataCount() const;
	//gets the largest metadata size, which can be useful to reserve buffers in advance
	size_t getMaxEventMetadataSize() const;
	bool hasSameEventMetadata(const MetadataEventObject& other) const;
	bool hasSimilarEventMetadata(const MetadataEventObject& other) const;
protected:
	MetadataDescriptorArray m_eventMetadataDescriptorArray;
	MetadataEventObject();
	size_t m_totalSize{ 0 };
	size_t m_maxSize{ 0 };
private:
	bool checkMetadataCoincidence(const MetadataEventObject& other, bool similar) const;
};

//And the base from which event objects can hold their metadata before serializing
class PLUGIN_API MetadataEvent
{
public:
    virtual ~MetadataEvent();
	int getMetadataValueCount() const;
	const MetadataValue* getMetadataValue(int index) const;
protected:
	void serializeMetadata(void* dstBuffer) const;
	bool deserializeMetadata(const MetadataEventObject* info, const void* srcBuffer, int size);
	MetadataEvent();
	MetadataValueArray m_metaDataValues;
};

//Helper function to compare identifier strings
bool compareIdentifierStrings(const String& identifier, const String& compareWith);

typedef MetadataDescriptor::MetadataTypes BaseType;

#endif