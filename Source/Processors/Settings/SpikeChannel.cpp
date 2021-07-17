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

#include "SpikeChannel.h"

#include "../GenericProcessor/GenericProcessor.h"

SpikeChannel::SpikeChannel(SpikeChannel::Settings settings)
	: InfoObject(InfoObject::Type::SPIKE_CHANNEL),
	m_type(settings.type) 
{
	/*int n = sourceChannels.size();
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
	}*/

	//setDefaultNameAndDescription();
}

SpikeChannel::~SpikeChannel() { }


SpikeChannel::Type SpikeChannel::getChannelType() const
{
	return m_type;
}

const Array<const ContinuousChannel*> SpikeChannel::getSourceChannels() const
{
	return m_channelInfo;
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

unsigned int SpikeChannel::getNumChannels(SpikeChannel::Type type)
{
	switch (type)
	{
	case SINGLE: return 1;
	case STEREOTRODE: return 2;
	case TETRODE: return 4;
	default: return 0;
	}
}

SpikeChannel::Type SpikeChannel::typeFromNumChannels(unsigned int nChannels)
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
	if (index < 0 || index >= m_channelInfo.size())
		return 1.0f;
	else
		return m_channelInfo[index]->getBitVolts();
}

/*void SpikeChannel::setDefaultNameAndDescription()
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
}*/

/*bool SpikeChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
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
}*/
