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

#include "InfoObject.h"

#include "DataStream.h"
#include "../GenericProcessor/GenericProcessor.h"


HistoryObject::HistoryObject() { }

HistoryObject::~HistoryObject() { }


String HistoryObject::getHistoryString() const
{
	return m_historyString;
}

void HistoryObject::addToHistoryString(String entry)
{
	if (m_historyString.isEmpty())
		m_historyString = entry;
	else
		m_historyString += (" -> " + entry);
}

NamedObject::NamedObject() {
}

NamedObject::~NamedObject() {}


void NamedObject::setName(String name)
{
	m_name = name;
}

const String NamedObject::getName() const
{
	return m_name;
}

void NamedObject::setDescription(const String& description)
{
	m_description = description;
}

const String& NamedObject::getDescription() const
{
	return m_description;
}

void NamedObject::setIdentifier(const String& identifier)
{
	m_identifier = identifier;
}

const String& NamedObject::getIdentifier() const
{
	return m_identifier;
}

bool NamedObject::operator==(const NamedObject& other) const
{
	return m_uuid == other.getUniqueId();
}

bool NamedObject::isSimilar(const NamedObject& other) const
{
	return m_identifier.compare(other.getIdentifier()) == 0;
}

const Uuid NamedObject::getUniqueId() const
{
	return m_uuid;
}


//InfoObjectCommon
InfoObject::InfoObject(InfoObject::Type type)
	:	m_type(type),
		m_local_index(-1),
		m_global_index(-1),
		m_sourceNodeId(-1),
        m_isEnabled(true),
        m_isLocal(true)
{
}

InfoObject::~InfoObject()
{}

InfoObject::InfoObject(const InfoObject& other) :
    NamedObject(other),
    HistoryObject(other),
    MetadataObject(other),
    m_type(other.m_type),
    m_local_index(other.m_local_index),
    m_global_index(other.m_global_index),
    m_sourceNodeId(other.m_sourceNodeId),
    m_sourceNodeName(other.m_sourceNodeName),
    m_nodeId(other.m_nodeId),
    m_nodeName(other.m_nodeName),
    m_isLocal(false),
    m_isEnabled(other.m_isEnabled),
    group(other.group),
    position(other.position),
    processorChain(other.processorChain)
{
}

const InfoObject::Type InfoObject::getType() const
{
	return m_type;
}

void InfoObject::setLocalIndex(int index)
{
	m_local_index = index;
}

int InfoObject::getLocalIndex() const
{
	return m_local_index;
}

void InfoObject::setGlobalIndex(int index)
{
	m_global_index = index;
}

int InfoObject::getGlobalIndex() const
{
	return m_global_index;
}

void InfoObject::setNodeId(int nodeId)
{
	if (m_sourceNodeId == -1)
		m_sourceNodeId = nodeId;

	m_nodeId = nodeId;
}

int InfoObject::getNodeId() const
{
	return m_nodeId;
}

void InfoObject::setSourceNodeId(int nodeId)
{
    m_sourceNodeId = nodeId;
}

int InfoObject::getSourceNodeId() const
{
	return m_sourceNodeId;
}

void InfoObject::setSourceNodeName(String nodeName)
{
    m_sourceNodeName = nodeName;
}

String InfoObject::getSourceNodeName() const
{
	return m_sourceNodeName;
}

String InfoObject::getNodeName() const
{
	return m_nodeName;
}

void InfoObject::addProcessor(GenericProcessor* processor)
{

	if (processorChain.size() == 0)
		m_sourceNodeName = processor->getName();

	processorChain.add(processor);
	addToHistoryString(processor->getName());
	setNodeId(processor->getNodeId());

	m_nodeName = processor->getName();
}

bool InfoObject::isLocal() const
{
    return m_isLocal;
}


// --------------------------------------------
//     CHANNEL INFO OBJECT
// --------------------------------------------

ChannelInfoObject::ChannelInfoObject(InfoObject::Type type, DataStream* dataStream)
	: InfoObject(type), isRecorded(false)
{
    setDataStream(dataStream); // sets local index
}

ChannelInfoObject::~ChannelInfoObject()
{

}

ChannelInfoObject::ChannelInfoObject(const ChannelInfoObject& other)
 : InfoObject(other), isRecorded(other.isRecorded)
{
}

float ChannelInfoObject::getSampleRate() const
{
    if (stream != nullptr)
        return stream->getSampleRate();
    else
        return 0.0f;
}

uint16 ChannelInfoObject::getStreamId() const
{
    if (stream != nullptr)
        return stream->getStreamId();
    else
        return 0;
}

String ChannelInfoObject::getStreamName() const
{
    if (stream != nullptr)
        return stream->getName();
    else
        return "None";
}

void ChannelInfoObject::setDataStream(DataStream* ds, bool addToStream)
{
    stream = ds;

    if (stream != nullptr && addToStream)
        stream->addChannel(this);
}

/*bool InfoObject::isEqual(const InfoObject& other) const
{
	return isEqual(other, false);
}

bool InfoObject::isSimilar(const InfoObject& other) const
{
	return isEqual(other, true);
}*/

/*bool InfoObject::isEqual(const InfoObject& other, bool similar) const
{
	if (getInfoObjectType() != other.getInfoObjectType()) return false;
	if (m_sampleRate != other.m_sampleRate) return false;
	if (!similar && getIdentifier().trim() != other.getIdentifier().trim()) return false;
	return checkEqual(other, similar);
}*/

