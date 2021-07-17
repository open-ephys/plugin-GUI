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

class ProcessorInfoObject;
class DataStream;

/** This class creates a string that indicates all of the processors a channel has passed through */
class PLUGIN_API HistoryObject
{
protected:
	HistoryObject();

public:
    virtual ~HistoryObject();

	/** Returns the history string */
	String getHistoryString() const;

	/** Adds a new entry in the history string*/
	void addToHistoryString(String entry);

private:
	String m_historyString;
};


class PLUGIN_API NamedObject
{
public:
    virtual ~NamedObject();

	/** Sets the object's name*/
	void setName(String name);

	/** Returns the name of a given object*/
	const String getName() const;

	/** Sets the object description*/
	void setDescription(String& description);

	/** Gets the object description */
	const String getDescription() const;

	/** Sets a machine-readable identifier */
	void setIdentifier(String& identifier);

	/** Gets a machine-readable identifier */
	const String getIdentifier() const;

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

/** Common class for all info objects */
class PLUGIN_API InfoObject :
	public NamedObject, public MetadataObject, public HistoryObject
{

public:
    virtual ~InfoObject();

	enum Type
	{
		CONTINUOUS_CHANNEL,
		EVENT_CHANNEL,
		SPIKE_CHANNEL,
		CONFIGURATION_OBJECT,
		PROCESSOR_INFO,
		DATASTREAM_INFO,
		DEVICE_INFO,
		INVALID = 100
	};

	struct Group
	{
		String name = "default";
		int number = 0;
	};

	struct Position
	{
		float x = 0;
		float y = 0;
		float z = 0;
		String description = "unknown";
	};

	const Type getType() const;

	int getLocalIndex() const;
	void setLocalIndex(int idx);

	int getGlobalIndex() const;
	void setGlobalIndex(int idx);

	int getNodeId() const;
	void setNodeId(int nodeId);

	int getSourceNodeId() const;

	int getStreamId() const;
	
	void addProcessor(ProcessorInfoObject*);

	Array<ProcessorInfoObject*> processorChain;

	DataStream* stream;
	Group group;
	Position position;

protected:
	InfoObject(Type type);

private:

	int m_sourceNodeId;

	const Type m_type;
	int m_local_index;
	int m_global_index;
	int m_nodeId;
	
};


#endif