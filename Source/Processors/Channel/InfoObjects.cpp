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
NodeInfoBase::NodeInfoBase(uint16 id) :
	m_nodeID(id)
{}

unsigned int NodeInfoBase::getCurrentNodeID() const
{
	return m_nodeID;
}

//History Object
String HistoryObject::getHistoricString()
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
		m_sourceName(source->getName()) //TODO: fix those two when we have the ability to rename processors
{
}

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

//NamedInfoObject
void NamedInfoObject::setName(String name)
{
	m_name = name;
}

String NamedInfoObject::getName() const
{
	return m_name;
}

void NamedInfoObject::setDescriptor(String descriptor)
{
	m_descriptor = descriptor;
}

String NamedInfoObject::getDescriptor() const
{
	return m_descriptor;
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
	:	NodeInfoBase(source->getNodeId()),
		SourceProcessorInfo(source, subproc),
		m_sourceIndex(idx),
		m_sourceTypeIndex(typeidx),
		m_sampleRate(sampleRate)
{
}

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
	String descriptor = "continuous.";
	switch (m_type)
	{
	case HEADSTAGE_CHANNEL: 
		name = "CH"; 
		description = "Headstage";
		descriptor += "headstage";
		break;
	case AUX_CHANNEL: 
		name = "AUX "; 
		description = "Auxiliar";
		descriptor += "aux";
		break;
	case ADC_CHANNEL: 
		name = "ADC "; 
		description = "ADC";
		descriptor = "adc";
		break;
	default: 
		setName("INVALID");
		setDescription("Invalid Channel");
		setDescriptor("invalid");
		return;
		break;
	}
	name += " p";
	name += String(getSourceNodeID()) + String(".") + String(getSubProcessorIdx());
	name += " n";
	name += String(getSourceTypeIndex());
	setName(name);
	setDescription(description + " data channel");
	setDescriptor(descriptor);
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
		setDescriptor("invalid");
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
		setDescriptor("genericevent.ttl");
	}
	else if (m_type == TEXT)
	{
		setDescription("Text event");
		setDescriptor("genericevent.text");
	}
	else
	{
		if (m_length > 1)
			setDescription(name + " data array");
		else
			setDescription(name + " single value");
		setDescriptor("genericevent." + name.toLowerCase());
	}
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
		sourceChannelInfo info;
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

Array<sourceChannelInfo> SpikeChannel::getSourceChannelInfo() const
{
	return m_sourceInfo;
}

void SpikeChannel::setGain(float gain)
{
	m_gain = gain;
}

float SpikeChannel::getGain() const
{
	return m_gain;
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
	String descriptor = "spikesource.";
	switch (m_type)
	{
	case SINGLE: 
		name = "SE ";
		description = "Single electrode";
		descriptor += "single";
		break;
	case STEREOTRODE: 
		name = "ST "; 
		description = "Stereotrode";
		descriptor += "stereotrode";
		break;
	case TETRODE: 
		name = "TT ";
		description = "Tetrode";
		descriptor += "tetrode";
		break;
	default: name = "INVALID "; break;
	}
	name += String(" p") + String(getSourceNodeID()) + String(".") + String(getSubProcessorIdx()) + String(" n") + String(getSourceTypeIndex());
	setName(name);
	setDescription(description + " spike data source");
	setDescriptor(descriptor);
}

//ConfigurationObject
ConfigurationObject::ConfigurationObject(String descriptor, GenericProcessor* source, uint16 subproc)
	: SourceProcessorInfo(source, subproc)
{
	setDefaultNameAndDescription();
	setDescriptor(descriptor);
}

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