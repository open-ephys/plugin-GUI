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

#include "InfoObjects.h"
#include "../GenericProcessor/GenericProcessor.h"

/** Common classes **/
//Empty protected constructors
HistoryObject::HistoryObject() {}
NamedInfoObject::NamedInfoObject() {}

//NodeInfoBase
NodeInfoBase::NodeInfoBase(uint16 id, uint16 idx, String type, String name) :
m_nodeID(id), m_nodeIdx(idx), m_currentNodeType(type), m_currentNodeName(name)
{}

NodeInfoBase::~NodeInfoBase()
{}

uint16 NodeInfoBase::getCurrentNodeID() const
{
	return m_nodeID;
}

uint16 NodeInfoBase::getCurrentNodeChannelIdx() const
{
	return m_nodeIdx;
}

String NodeInfoBase::getCurrentNodeType() const
{
	return m_currentNodeType;
}

String NodeInfoBase::getCurrentNodeName() const
{
	return m_currentNodeName;
}

//History Object
HistoryObject::~HistoryObject()
{}

String HistoryObject::getHistoricString() const
{
	return m_historicString;
}

void HistoryObject::addToHistoricString(String entry)
{
	if (m_historicString.isEmpty())
		m_historicString = entry;
	else
		m_historicString += (" -> " + entry);
}

//SourceProcessorInfo
SourceProcessorInfo::SourceProcessorInfo(const GenericProcessor* source, uint16 subproc) 
	:	m_sourceNodeID(source->getNodeId()),
		m_sourceSubNodeIndex(subproc), 
		m_sourceType(source->getName()),
		m_sourceName(source->getName()), //TODO: fix those two when we have the ability to rename processors
		m_sourceSubProcessorCount(source->getNumSubProcessors())
{
}

SourceProcessorInfo::~SourceProcessorInfo()
{}

uint16 SourceProcessorInfo::getSourceNodeID() const
{
	return m_sourceNodeID;
}

uint16 SourceProcessorInfo::getSubProcessorIdx() const
{
	return m_sourceSubNodeIndex;
}

String SourceProcessorInfo::getSourceType() const
{
	return m_sourceType;
}

String SourceProcessorInfo::getSourceName() const
{
	return m_sourceName;
}

uint16 SourceProcessorInfo::getSourceSubprocessorCount() const
{
	return m_sourceSubProcessorCount;
}

//NamedInfoObject
NamedInfoObject::~NamedInfoObject()
{}

void NamedInfoObject::setName(String name)
{
	m_name = name;
}

String NamedInfoObject::getName() const
{
	return m_name;
}

void NamedInfoObject::setIdentifier(String identifier)
{
	m_identifier = identifier;
}

String NamedInfoObject::getIdentifier() const
{
	return m_identifier;
}

void NamedInfoObject::setDescription(String description)
{
	m_description = description;
}

String NamedInfoObject::getDescription() const
{
	return m_description;
}

//InfoObjectCommon
InfoObjectCommon::InfoObjectCommon(uint16 idx, uint16 typeidx, float sampleRate, const GenericProcessor* source, uint16 subproc)
	:	NodeInfoBase(source->getNodeId(), idx, source->getName(), source->getName()), //TODO: fix those two when we have the ability to rename processors
		SourceProcessorInfo(source, subproc),
		m_sourceIndex(idx),
		m_sourceTypeIndex(typeidx),
		m_sampleRate(sampleRate)
{
}

InfoObjectCommon::~InfoObjectCommon()
{}

float InfoObjectCommon::getSampleRate() const
{
	return m_sampleRate;
}


uint16 InfoObjectCommon::getSourceIndex() const
{
	return m_sourceIndex;
}

uint16 InfoObjectCommon::getSourceTypeIndex() const
{
	return m_sourceTypeIndex;
}

bool InfoObjectCommon::operator==(const InfoObjectCommon& other) const
{
	return isEqual(other);
}

bool InfoObjectCommon::isEqual(const InfoObjectCommon& other) const
{
	return isEqual(other, false);
}

bool InfoObjectCommon::isSimilar(const InfoObjectCommon& other) const
{
	return isEqual(other, true);
}

bool InfoObjectCommon::isEqual(const InfoObjectCommon& other, bool similar) const
{
	if (getInfoObjectType() != other.getInfoObjectType()) return false;
	if (m_sampleRate != other.m_sampleRate) return false;
	if (!similar && getIdentifier().trim() != other.getIdentifier().trim()) return false;
	return checkEqual(other, similar);
}

//DataChannel

DataChannel::DataChannel(DataChannelTypes type, float sampleRate, GenericProcessor* source, uint16 subproc) :
	InfoObjectCommon(source->dataChannelCount++, source->dataChannelTypeCount[type]++, sampleRate, source, subproc),
	m_type(type)
{
	setDefaultNameAndDescription();
}

DataChannel::DataChannel(const DataChannel& ch)
	:	InfoObjectCommon(ch), //Call default copy constructors of base classes
		MetaDataInfoObject(ch),
		HistoryObject(ch),
		m_type(ch.m_type),
		m_bitVolts(ch.m_bitVolts),
		m_isEnabled(true),
		m_isMonitored(false),
		m_isRecording(false)
{
}

DataChannel::~DataChannel()
{
}

InfoObjectCommon::InfoObjectType DataChannel::getInfoObjectType() const
{
	return InfoObjectCommon::DATA_CHANNEL;
}

void DataChannel::setBitVolts(float bitVolts)
{
	m_bitVolts = bitVolts;
}

float DataChannel::getBitVolts() const
{
	return m_bitVolts;
}

void DataChannel::setDataUnits(String unit)
{
	m_unitName = unit;
}

String DataChannel::getDataUnits() const
{
	return m_unitName;
}

DataChannel::DataChannelTypes DataChannel::getChannelType() const
{
	return m_type;
}

bool DataChannel::isEnabled() const
{
	return m_isEnabled;
}

void DataChannel::setEnable(bool e)
{
	m_isEnabled = e;
}

bool DataChannel::isMonitored() const
{
	return m_isMonitored;
}

void DataChannel::setMonitored(bool e)
{
	m_isMonitored = e;
}

void DataChannel::setRecordState(bool t)
{
	m_isRecording = t;
}

bool DataChannel::getRecordState() const
{
	return m_isRecording;
}

void DataChannel::reset()
{
	m_bitVolts = 1.0f;
	m_isEnabled = true;
	m_isMonitored = false;
	m_isRecording = false;
}

void DataChannel::setDefaultNameAndDescription() 
{
	String name;
	String description;
	switch (m_type)
	{
	case HEADSTAGE_CHANNEL: 
		name = "CH"; 
		description = "Headstage";
		break;
	case AUX_CHANNEL: 
		name = "AUX "; 
		description = "Auxiliar";
		break;
	case ADC_CHANNEL: 
		name = "ADC "; 
		description = "ADC";
		break;
	default: 
		setName("INVALID");
		setDescription("Invalid Channel");
		setIdentifier("invalid");
		return;
		break;
	}
	name += " p";
	name += String(getSourceNodeID()) + String(".") + String(getSubProcessorIdx());
	name += " n";
	name += String(getSourceTypeIndex());
	setName(name);
	setDescription(description + " data channel");
	setIdentifier("genericdata.continuous");
}

bool DataChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
{
	const DataChannel& o = dynamic_cast<const DataChannel&>(other);
	if (m_bitVolts != o.m_bitVolts) return false;
	if (!similar && m_unitName != o.m_unitName) return false;
	if (similar)
		return hasSimilarMetadata(o);
	else return hasSameMetadata(o);
}

//EventChannel
EventChannel::EventChannel(EventChannelTypes type, unsigned int nChannels, unsigned int dataLength, float sampleRate, GenericProcessor* source, uint16 subproc)
	: InfoObjectCommon(source->eventChannelCount++, source->eventChannelTypeCount[type]++, sampleRate, source, subproc),
		m_type(type)
{
	m_numChannels = nChannels;
	if (m_type == TTL)
	{
		m_length = (m_numChannels + 7) / 8;
		m_dataSize = m_length;
	}
	else
	{
		m_length = dataLength;
		m_dataSize = dataLength * getTypeByteSize(m_type);
		//for messages, add 1 byte to account for the null terminator
		if (m_type == TEXT) m_dataSize += 1;
	}
	setDefaultNameAndDescription();
}

EventChannel::~EventChannel()
{
}

InfoObjectCommon::InfoObjectType EventChannel::getInfoObjectType() const
{
	return InfoObjectCommon::EVENT_CHANNEL;
}

EventChannel::EventChannelTypes EventChannel::getChannelType() const
{
	return m_type;
}

unsigned int EventChannel::getNumChannels() const
{
	return m_numChannels;
}

unsigned int EventChannel::getLength() const
{
	return m_length;
}

size_t EventChannel::getDataSize() const
{
	return m_dataSize;
}

void EventChannel::setShouldBeRecorded(bool status)
{
	m_shouldBeRecorded = status;
}

bool EventChannel::getShouldBeRecorded() const
{
	return m_shouldBeRecorded;
}

size_t EventChannel::getTypeByteSize(EventChannel::EventChannelTypes type)
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
	default: return sizeof(char);
	}
}

void EventChannel::setDefaultNameAndDescription()
{
	String name;
	switch (m_type)
	{
	case TTL: name = "TTL"; break;
	case TEXT: name = "TEXT"; break;
	case INT8_ARRAY: name = "INT8"; break;
	case UINT8_ARRAY: name = "UINT8"; break;
	case INT16_ARRAY: name = "INT16"; break;
	case UINT16_ARRAY: name = "UINT16"; break;
	case INT32_ARRAY: name = "INT32"; break;
	case UINT32_ARRAY: name = "UINT32"; break;
	case INT64_ARRAY: name = "INT64"; break;
	case UINT64_ARRAY: name = "UINT64"; break;
	default: 
		setName("INVALID");
		setDescription("Invalid channel");
		setIdentifier("invalid");
		return;
		break;
	}
	name += "p";
	name += String(getSourceNodeID()) + String(".") + String(getSubProcessorIdx());
	name += " n";
	name += String(getSourceTypeIndex());
	setName(name);
	if (m_type == TTL)
	{
		setDescription("TTL data input");
	}
	else if (m_type == TEXT)
	{
		setDescription("Text event");
	}
	else
	{
		if (m_length > 1)
			setDescription(name + " data array");
		else
			setDescription(name + " single value");
	}
	setIdentifier("genericevent");
}


BaseType EventChannel::getEquivalentMetaDataType(const EventChannel& ev)
{
	switch (ev.getChannelType())
	{
	case EventChannel::TEXT:
		return MetaDataDescriptor::CHAR;
	case EventChannel::INT8_ARRAY:
		return MetaDataDescriptor::INT8;
	case EventChannel::UINT8_ARRAY:
		return MetaDataDescriptor::UINT8;
	case EventChannel::INT16_ARRAY:
		return MetaDataDescriptor::INT16;
	case EventChannel::UINT16_ARRAY:
		return MetaDataDescriptor::UINT16;
	case EventChannel::INT32_ARRAY:
		return MetaDataDescriptor::INT32;
	case EventChannel::UINT32_ARRAY:
		return MetaDataDescriptor::UINT32;
	case EventChannel::INT64_ARRAY:
		return MetaDataDescriptor::INT64;
	case EventChannel::UINT64_ARRAY:
		return MetaDataDescriptor::UINT64;
	case EventChannel::FLOAT_ARRAY:
		return MetaDataDescriptor::FLOAT;
	case EventChannel::DOUBLE_ARRAY:
		return MetaDataDescriptor::DOUBLE;
	default:
		return MetaDataDescriptor::UINT8;
	}
}

BaseType EventChannel::getEquivalentMetaDataType() const
{
	return getEquivalentMetaDataType(*this);
}

bool EventChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
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
}

//SpikeChannel

SpikeChannel::SpikeChannel(ElectrodeTypes type, GenericProcessor* source, const Array<const DataChannel*>& sourceChannels, uint16 subproc)
	: InfoObjectCommon(source->spikeChannelCount++, source->spikeChannelTypeCount[type]++, sourceChannels[0]->getSampleRate(), source, subproc),
	m_type(type) //We define the sample rate of the whole spike to be equal to that of the first channel. A spike composed from channels from different sample rates has no sense
{
	int n = sourceChannels.size();
	jassert(n == getNumChannels(type));
	for (int i = 0; i < n; i++)
	{
		SourceChannelInfo info;
		const DataChannel* chan = sourceChannels[i];
		info.processorID = chan->getSourceNodeID();
		info.subProcessorID = chan->getSubProcessorIdx();
		info.channelIDX = chan->getSourceIndex();
		m_sourceInfo.add(info);
		m_channelBitVolts.add(chan->getBitVolts());
	}
	setDefaultNameAndDescription();
}

SpikeChannel::~SpikeChannel()
{}

InfoObjectCommon::InfoObjectType SpikeChannel::getInfoObjectType() const
{
	return InfoObjectCommon::SPIKE_CHANNEL;
}

SpikeChannel::ElectrodeTypes SpikeChannel::getChannelType() const
{
	return m_type;
}

Array<SourceChannelInfo> SpikeChannel::getSourceChannelInfo() const
{
	return m_sourceInfo;
}

void SpikeChannel::setNumSamples(unsigned int preSamples, unsigned int postSamples)
{
	m_numPreSamples = preSamples;
	m_numPostSamples = postSamples;
}

unsigned int SpikeChannel::getPrePeakSamples() const
{
	return m_numPreSamples;
}

unsigned int SpikeChannel::getPostPeakSamples() const
{
	return m_numPostSamples;
}

unsigned int SpikeChannel::getTotalSamples() const
{
	return m_numPostSamples + m_numPreSamples;
}

unsigned int SpikeChannel::getNumChannels() const
{
	return getNumChannels(m_type);
}

unsigned int SpikeChannel::getNumChannels(SpikeChannel::ElectrodeTypes type)
{
	switch (type)
	{
	case SINGLE: return 1;
	case STEREOTRODE: return 2;
	case TETRODE: return 4;
	default: return 0;
	}
}

SpikeChannel::ElectrodeTypes SpikeChannel::typeFromNumChannels(unsigned int nChannels)
{
	switch (nChannels)
	{
	case 1: return SINGLE;
	case 2: return STEREOTRODE;
	case 4: return TETRODE;
	default: return INVALID;
	}
}

size_t SpikeChannel::getDataSize() const
{
	return getTotalSamples()*getNumChannels()*sizeof(float);
}

size_t SpikeChannel::getChannelDataSize() const
{
	return getTotalSamples()*sizeof(float);
}

float SpikeChannel::getChannelBitVolts(int index) const
{
	if (index < 0 || index >= m_channelBitVolts.size())
		return 1.0f;
	else
		return m_channelBitVolts[index];
}

void SpikeChannel::setDefaultNameAndDescription()
{
	String name;
	String description;
	switch (m_type)
	{
	case SINGLE: 
		name = "SE ";
		description = "Single electrode";
		break;
	case STEREOTRODE: 
		name = "ST "; 
		description = "Stereotrode";
		break;
	case TETRODE: 
		name = "TT ";
		description = "Tetrode";
		break;
	default: name = "INVALID "; break;
	}
	name += String(" p") + String(getSourceNodeID()) + String(".") + String(getSubProcessorIdx()) + String(" n") + String(getSourceTypeIndex());
	setName(name);
	setDescription(description + " spike data source");
	setIdentifier("spikesource");
}

bool SpikeChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
{
	const SpikeChannel& o = dynamic_cast<const SpikeChannel&>(other);
	if (m_type != o.m_type) return false;
	if (m_numPostSamples != o.m_numPostSamples) return false;
	if (m_numPreSamples != o.m_numPreSamples) return false;

	int nChans = m_channelBitVolts.size();
	if (nChans != o.m_channelBitVolts.size()) return false;
	for (int i = 0; i < nChans; i++)
	{
		if (m_channelBitVolts[i] != o.m_channelBitVolts[i]) return false;
	}

	if (similar && !hasSimilarMetadata(o)) return false;
	if (!similar && !hasSameMetadata(o)) return false;
	if (similar && !hasSimilarEventMetadata(o)) return false;
	if (!similar && !hasSameEventMetadata(o)) return false;
	return true;
}

//ConfigurationObject
ConfigurationObject::ConfigurationObject(String identifier, GenericProcessor* source, uint16 subproc)
	: SourceProcessorInfo(source, subproc)
{
	setDefaultNameAndDescription();
	setIdentifier(identifier);
}

ConfigurationObject::~ConfigurationObject()
{}

void ConfigurationObject::setShouldBeRecorded(bool status)
{
	m_shouldBeRecorded = status;
}

bool ConfigurationObject::getShouldBeRecorded() const
{
	return m_shouldBeRecorded;
}

void ConfigurationObject::setDefaultNameAndDescription()
{
	setName("Configuration Object");
	setDescription("Generic configuration object");	
}