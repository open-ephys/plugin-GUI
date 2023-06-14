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

#ifndef INFOOBJECT_H_INCLUDED
#define INFOOBJECT_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "Metadata.h"
#include "../Parameter/ParameterCollection.h"
#include "../Parameter/Parameter.h"

class GenericProcessor;
class ParameterCollection;
class DataStream;

/** This class creates a string that indicates all of the processors a channel has passed through */
class PLUGIN_API HistoryObject
{
protected:
	/** Constructor */
	HistoryObject();

public:
	/** Destructor */
    virtual ~HistoryObject();

	/** Returns the history string */
	String getHistoryString() const;

	/** Adds a new entry in the history string*/
	void addToHistoryString(String entry);

private:
	String m_historyString;
};

/** Base class for the GUI's info objects
*
* Stores a name, description, identifier, and a unique 128-bit ID
*
*/
class PLUGIN_API NamedObject
{
public:
    virtual ~NamedObject();

	/** Sets the object's name*/
	void setName(String name);

	/** Returns the name of a given object*/
	const String getName() const;

	/** Sets the object description*/
	void setDescription(const String& description);

	/** Gets the object description */
	const String& getDescription() const;

	/** Sets a machine-readable identifier */
	void setIdentifier(const String& identifier);

	/** Gets a machine-readable identifier */
	const String& getIdentifier() const;

	/** Checks whether object has a matching UUID (e.g. it was copied) */
	bool operator==(const NamedObject& other) const;

	/** Check whether an object has the same identifier (e.g. is a similar object) */
	bool isSimilar(const NamedObject& other) const;

	/** Gets the object's UUID */
	const Uuid getUniqueId() const;

protected:
	NamedObject();

private:

	String m_name;
	String m_description;
	String m_identifier;
	Uuid m_uuid;

};

/**
* Holds information about some element of the GUI's signal chain.
*
* Inherits from:
*  - NamedObject (stores a name, description, identifier, and unique ID)
*  - MetadataObject (allows this object to be tagged with arbitrary metadata fields)
*  - HistoryObject (stores a string of all the processors this object has passed through)
*
* InfoObjects are used to represent continuous channels, event channels,
* spike channels, processors, data streams, devices, and other information
* about plugins.
*
*/
class PLUGIN_API InfoObject :
	public NamedObject,
	public MetadataObject,
	public HistoryObject
{

public:
	/** Destructor */
    virtual ~InfoObject();

    /** Custom copy constructor -- set isLocal to false and don't copy parameters*/
    InfoObject(const InfoObject& other);

	/** InfoObject::Type*/
	enum Type
	{
		// A channel that's sampled at regular intervals
		CONTINUOUS_CHANNEL,

		// A channel that sends events at arbitrary times
		EVENT_CHANNEL,

		// A channel that sends spike events
		SPIKE_CHANNEL,

		// An object that stores plugin metadata
		CONFIGURATION_OBJECT,

		// Represents information about a plugin
		PROCESSOR_INFO,

		// A collection of synchronously sampled continuous channels
		DATASTREAM_INFO,

		// A hardware device that generates data
		DEVICE_INFO,

		// A component used for data visualization
		VISUALIZER,

		INVALID = 100
	};

	/** Each InfoObject can be associated with a "group"*/
	struct Group
	{
		String name = "default";
		int number = 0;
	};

	/** Each InfoObject can be associated with a coordinate in space*/
	struct Position
	{
		float x = 0;
		float y = 0;
		float z = 0;
		String description = "unknown";
	};

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a parameter to this object**/
    void addParameter(Parameter* p);

    /** Returns a pointer to a parameter with a given name**/
    Parameter* getParameter(String name) const { return parameters[name]; }

	/** Returns true if an object has a parameter with a given name**/
	bool hasParameter(String name) const { return parameters.contains(name); }

    /** Returns a pointer to a parameter with a given name**/
    Array<Parameter*> getParameters() { return parameters.getParameters(); }

	/*Bracket operator returns the value of a parameter*/
    var operator [](String name) const {return parameters[name]->getValue();}

    /** Copies parameters from another InfoObject and clears the original ParameterCollection*/
    void copyParameters(InfoObject* object);

    /** Returns the number of parameters for this object*/
    int numParameters() const { return parameters.size(); }

	/** Initiates parameter value update */
    virtual void parameterChangeRequest(Parameter*) { }

    /** Called when a parameter value is updated, to allow plugin-specific responses*/
    virtual void parameterValueChanged(Parameter*) { }

	/** Returns the type of the InfoObject*/
	const Type getType() const;

	/** Returns the index of the InfoObject within a DataStream*/
	int getLocalIndex() const;

	/** Sets the index of the InfoObject within a DataStream*/
	void setLocalIndex(int idx);

	/** Returns the index of the InfoObject within a processor*/
	int getGlobalIndex() const;

	/** Sets the index of the InfoObject within a processor*/
	void setGlobalIndex(int idx);

	/** Returns the ID of the processor this InfoObject belongs to.*/
	int getNodeId() const;

	/** Sets the ID of the processor this InfoObject belongs to.*/
	void setNodeId(int nodeId);

	/** Returns the name of the processor this InfoObject belongs to.*/
	String getNodeName() const;

	/** Returns the ID of the processor that created this InfoObject.*/
	int getSourceNodeId() const;

	/** Returns the name of the processor that created this InfoObject.*/
	String getSourceNodeName() const;

	/** Indicates that this InfoObject is passing through a new processor.*/
	void addProcessor(GenericProcessor*);

    /** Returns true if this object was created locally, or copied from an upstream processor.*/
    bool isLocal() const;

	Array<GenericProcessor*> processorChain;

	Group group;
	Position position;

    ParameterCollection parameters;


protected:
	/** Constructor*/
	InfoObject(Type type);

	void setSourceNodeId(int nodeId);

	void setSourceNodeName(String nodeName);

private:

	const Type m_type;

	int m_local_index;
	int m_global_index;

	int m_nodeId;
	String m_nodeName;

	int m_sourceNodeId;
	String m_sourceNodeName;

    bool m_isEnabled;

    bool m_isLocal;
};

/**
 *  Holds information about a data channel (Continuous, Event, or Spike)
 *
 *  Every ChannelInfoObject must be associated with only one DataStream
 *
 * */
class PLUGIN_API ChannelInfoObject :
	public InfoObject
{
public :

	/** Constructor */
	ChannelInfoObject(InfoObject::Type type, DataStream* stream);

	/** Destructor */
	virtual ~ChannelInfoObject();

    /** Copy constructor */
    ChannelInfoObject(const ChannelInfoObject& other);

    /** Returns the sample rate for this channel's stream*/
	float getSampleRate() const;

	/** Returns the ID for this channel's stream */
	uint16 getStreamId() const;

	/** Returns the name of this channel's stream */
	String getStreamName() const;

    /** Set's this channel's DataStream and adds it to the stream by default*/
    virtual void setDataStream(DataStream* ds, bool addToStream = true);

    /** true when this channel passes through a Record Node and is set to be recorded */
    bool isRecorded;

protected:
	DataStream* stream;
};
#endif
