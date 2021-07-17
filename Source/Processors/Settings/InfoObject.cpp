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
#include "ProcessorInfo.h"

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

void NamedObject::setDescription(String& description)
{
	m_description = description;
}

const String NamedObject::getDescription() const
{
	return m_description;
}

void NamedObject::setIdentifier(String& identifier)
{
	m_identifier = identifier;
}

const String NamedObject::getIdentifier() const
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
	:	m_type(type), m_local_index(0), m_sourceNodeId(-1), stream(nullptr)
{
}

InfoObject::~InfoObject()
{}

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

int InfoObject::getSourceNodeId() const
{
	return m_sourceNodeId;
}

int InfoObject::getStreamId() const
{
	if (stream != nullptr)
		return stream->streamId;
	else
		return -1;
}


void InfoObject::addProcessor(ProcessorInfoObject* processor)
{
	processorChain.add(processor);
	addToHistoryString(processor->getName());
	setNodeId(processor->getNodeId());
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

