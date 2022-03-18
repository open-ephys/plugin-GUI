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
    Any extra info a processor or event wants to define that can't fit inside the standard fields can be defined as metadata.
    A metadata value can be of any type of the listed in MetadataType and with any dimension. For example,
    a metadata value of type INTEGER and size 3 would be an array of 3 integers. For strings, simply set
    a type of CHAR and a value equal to the maximum length of the string plus one, for the null char.
*/

/**
 
    Specifies the name, description, and contents of a metadata field
    Each field can contain a specified number of values of a particular data type
    e.g. a color field could be represented as:
        
        MetadataDescriptor( MetadataType::UINT8, 3, "color", "The color of this event", "metadata.color");
 
    A timestamp (in seconds) could be represented as:
 
        MetadataDescriptor( MetadataType::double, 1, "elapsed_time", "The time since the trial was initiated", "metadata.elapsed_time");
 
 */
class PLUGIN_API MetadataDescriptor
	: public ReferenceCountedObject
{
public:
	
    /** Available types */
    enum MetadataType
	{
		CHAR,   // for strings
		INT8,   // signed 8-bit integer
		UINT8,  // unsigned 8-bit integer
		INT16,  // signed 16-bit integer
		UINT16, // unsigned 16-bit integer
		INT32,  // signed 32-bit integer
		UINT32, // unsigned 32-bit integer
		INT64,  // signed 64-bit integer
		UINT64, // unsigned 64-bit integer
		FLOAT,  // floating point value
		DOUBLE  // double value
	};

	/**
	Metadata descriptor constructor
	@param type The primitive type this metadata field will hold
	@param length The length of the data. 1 for single value or more for arrays.
	@param name The human-readable name of this metadata field
	@param description A human-readable description of what this field represents
	@param identifier A simple machine-readable name for this metadata value

	name and description will be saved in most data formats for latter reference
	*/
	MetadataDescriptor(MetadataType type, unsigned int length, String name, String description, String identifier);
	
	/** Destructor */
	~MetadataDescriptor();

	/** Copy constructor */
	MetadataDescriptor(const MetadataDescriptor& other);

	/** Copy construtor */
	MetadataDescriptor& operator=(const MetadataDescriptor& other);

	/** Gets the primitive type of this field */
	MetadataType getType() const;

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

    /** Returns the size (in bytes) of the type for this field */
	static size_t getTypeSize(MetadataType type);
private:
    
    /** Delete default constructor */
	MetadataDescriptor() = delete;
	String m_name;
	String m_identifier;
	String m_description;
	MetadataType m_type;
	unsigned int m_length;

	JUCE_LEAK_DETECTOR(MetadataDescriptor);
};

/**
    Represents a specific value a metadata field can take.
 */
class PLUGIN_API MetadataValue
	: public ReferenceCountedObject
{
	friend MetadataEvent; //the serialize() method must access the raw data pointer
public:

	/** Create a MetadataValue with a specified type and length */
	MetadataValue(MetadataDescriptor::MetadataType type, unsigned int length);
    
    /** Create MetadataValue from a descriptor */
	MetadataValue(const MetadataDescriptor& desc);
    
	/** Create a MetadataValue with data included */
	MetadataValue(MetadataDescriptor::MetadataType type, unsigned int length, const void* data);
    
    /** Create a MetadataValue from a descriptor with data included */
	MetadataValue(const MetadataDescriptor& desc, const void* data);

    /** Returns true if value matches the field descriptor */
	bool isOfType(const MetadataDescriptor& desc) const;
    
    /** Returns true if value matches the field descriptor (using a pointer) */
	bool isOfType(const MetadataDescriptor* desc) const;

    /** Returns the MetadataType of this value */
	MetadataDescriptor::MetadataType getDataType() const;
    
    /** Returns the length of this value */
	unsigned int getDataLength() const;
    
    /** Returns the overall size (in bytes) of the data held in this value */
	size_t getDataSize() const;

    /** Copy constructor */
	MetadataValue(const MetadataValue&);
    
    /** Copy constructor */
	MetadataValue& operator= (const MetadataValue&);
#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    
    /** Move operator*/
	MetadataValue& operator= (MetadataValue&&);
#endif
    /** Destructor */
	~MetadataValue();

	/** Set value (only for CHAR data types) */
	void setValue(const String& data);
    
    /** Get value (only for CHAR data types) */
	void getValue(String& data) const;

	/** Set value for arbitrary single value data types*/
	template <typename T>
	void setValue(T data);

    /** Get value for arbitrary single value data types*/
	template <typename T>
	void getValue(T& data) const;

	/** Set value for arbitrary array value data types */
	template <typename T>
	void setValue(const T* data);

    /** Get value for arbitrary array value data types*/
	template <typename T>
	void getValue(T* data) const;

    /** Set value for arbitrary array value data types*/
	template <typename T>
	void setValue(const Array<T>& data);

    /** Get value for arbitrary array value data types */
	template <typename T>
	void getValue(Array<T>& data) const;

	/** It is generally not advised to use this method, which can, however, be of use in performance critical modules.
	It is usually preferable to use the strongly typed copy getter methods, when speed is not an issue.
	Keep in mind that any pointer returned by this will become invalid after the block execution.*/
	const void* getRawValuePointer() const;

private:
    
    /** Delete default constructor */
	MetadataValue() = delete;
    
    /** Internal set value method */
	void setValue(const void* data);
    
    /** Make space for incoming data */
	void allocSpace();
    
    /** Returns the overall size of this value, given a type and length*/
	static size_t getSize(MetadataDescriptor::MetadataType type, unsigned int length);
	
    HeapBlock<char> m_data;
	MetadataDescriptor::MetadataType m_type;
	unsigned int m_length;
	size_t m_size;

	JUCE_LEAK_DETECTOR(MetadataValue);
};

typedef ReferenceCountedArray<MetadataDescriptor,CriticalSection> MetadataDescriptorArray;
typedef ReferenceCountedArray<MetadataValue,CriticalSection> MetadataValueArray;
typedef ReferenceCountedObjectPtr<MetadataDescriptor> MetadataDescriptorPtr;
typedef ReferenceCountedObjectPtr<MetadataValue> MetadataValuePtr;

/**
    Base class for all InfoObjects that can include metadata
 */
class PLUGIN_API MetadataObject
{
protected:
    
    /** Default constructor */
	MetadataObject();
public:
    
    /** Destructor */
    virtual ~MetadataObject();
    
    /** Add metadata with a field description and value */
	void addMetadata(MetadataDescriptor* desc, MetadataValue* val);
    
    /** Add metadata with a field description and value */
	void addMetadata(const MetadataDescriptor& desc, const MetadataValue& val);
    
    /** Returns the description of a metadata field by index */
	const MetadataDescriptor* getMetadataDescriptor(int index) const;
    
    /** Returns the value of a metada field by index*/
	const MetadataValue* getMetadataValue(int index) const;
    
    /** Finds the index of a metadata field based on type, length, and/or identifier */
	int findMetadata(MetadataDescriptor::MetadataType type, unsigned int length, String identifier = String()) const;
    
    /** Returns the total number of metadata fields */
	const int getMetadataCount() const;
    
    /** Returns true if another object has exactly matching metadata */
	bool hasSameMetadata(const MetadataObject& other) const;
    
    /** Returns true if another object has the same metadata types and lengths */
	bool hasSimilarMetadata(const MetadataObject& other) const;
protected:
	MetadataDescriptorArray m_metadataDescriptorArray;
	MetadataValueArray m_metadataValueArray;
private:
    
    /** Checks whether metadata is similar to the same*/
	bool checkMetadataCoincidence(const MetadataObject& other, bool similar) const;
};

class PLUGIN_API MetadataEventLock
{
	/**GenericProcessor will set this to true when copying channels in the update method so no other processor but the one which
     created the object can call addEventMetadata. Since the events themselves are created by the source processor and are not modified,
     changing the metadata information will cause errors when trying to decode the data embedded in the event itself.*/
	friend class GenericProcessor;
protected:
	bool eventMetadataLock{ false };
    
    /** Constructor */
	MetadataEventLock();
};

/**
    Special class for event and spike info objects, whose events can hold metadata
 */
class PLUGIN_API MetadataEventObject : public MetadataEventLock
{
public:
    
    /** Destructor */
    virtual ~MetadataEventObject();
    
	/** This method will only work when creating the info object, but not for those copied down the chain */
	void addEventMetadata(MetadataDescriptor* desc);
    
    /** This method will only work when creating the info object, but not for those copied down the chain */
	void addEventMetadata(const MetadataDescriptor& desc);
    
    /** Return the metadata descriptor by index */
	const MetadataDescriptor* getEventMetadataDescriptor(int index) const;
    
    /** Find the index of a field by type, length, and (optionally) identifier */
	int findEventMetadata(MetadataDescriptor::MetadataType type, unsigned int length, String identifier = String()) const;
    
    /** Returns the total size (in bytes) of event metadata */
	size_t getTotalEventMetadataSize() const;
    
    /** Returns the total number of metadata fields */
	int getEventMetadataCount() const;
    
    /** gets the largest metadata size, which can be useful to reserve buffers in advance */
	size_t getMaxEventMetadataSize() const;
    
    /** Returns true if another MetadataEvent object has exactly matching metadata fields */
	bool hasSameEventMetadata(const MetadataEventObject& other) const;
    
    /** Returns true if another MetadataEvent object has similar metadata fields */
	bool hasSimilarEventMetadata(const MetadataEventObject& other) const;
protected:
	MetadataDescriptorArray m_eventMetadataDescriptorArray;
	MetadataEventObject();
	size_t m_totalSize{ 0 };
	size_t m_maxSize{ 0 };
private:
	bool checkMetadataCoincidence(const MetadataEventObject& other, bool similar) const;
};

/**
    Base class for event objects that can hold their metadata before serializing
 */
class PLUGIN_API MetadataEvent
{
public:
    
    /** Destructor */
    virtual ~MetadataEvent();
    
    /** Returns the total number of metadata fields in this event */
	int getMetadataValueCount() const;
    
    /** Returns the value for a metadata field by index */
	const MetadataValue* getMetadataValue(int index) const;
protected:
    
    /** Copies metadata into a byte buffer */
	void serializeMetadata(void* dstBuffer) const;
    
    /** Copies metadata from a byte buffer */
	bool deserializeMetadata(const MetadataEventObject* info, const void* srcBuffer, int size);
    
    /** Constructor */
	MetadataEvent();
    
	MetadataValueArray m_metaDataValues;
};

/** Helper function to compare identifier strings */
bool compareIdentifierStrings(const String& identifier, const String& compareWith);

typedef MetadataDescriptor::MetadataType BaseType;

#endif
