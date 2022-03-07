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

#include "Spike.h"
#include "../GenericProcessor/GenericProcessor.h"

Spike::Spike(const SpikeChannel* spikeChannel_,
	juce::int64 timestamp,
	Array<float> thresholds, 
	HeapBlock<float>& data, 
	uint16 sortedID)

	: EventBase(SPIKE_EVENT, 
		timestamp, 
		spikeChannel_->getSourceNodeId(),
		spikeChannel_->getStreamId(),
		spikeChannel_->getLocalIndex()),

	m_thresholds(thresholds),
	spikeChannel(spikeChannel_),
	m_sortedID(sortedID)
{
	m_data.swapWith(data);
}

Spike::Spike(const Spike& other)
	:EventBase(other),
	m_thresholds(other.m_thresholds),
	spikeChannel(other.spikeChannel),
	m_sortedID(other.m_sortedID)
{
	size_t size = spikeChannel->getDataSize();
	m_data.malloc(size, sizeof(char));
	memcpy(m_data.getData(), other.m_data.getData(), size);
}

Spike::~Spike() {}

const float* Spike::getDataPointer() const
{
	return m_data.getData();
}

uint16 Spike::getSortedId() const
{
	return m_sortedID;
}

const float* Spike::getDataPointer(int channel) const
{
	if ((channel < 0) || (channel >= spikeChannel->getNumChannels()))
	{
		jassertfalse;
		return nullptr;
	}
	return (m_data.getData() + (channel * spikeChannel->getTotalSamples()));
}

float Spike::getThreshold(int chan) const
{
	return m_thresholds[chan];
}

const SpikeChannel* Spike::getChannelInfo() const
{
	return spikeChannel;
}

void Spike::serialize(void* destinationBuffer, size_t bufferSize) const
{
	size_t dataSize = spikeChannel->getDataSize();
	size_t eventSize = dataSize + SPIKE_BASE_SIZE + m_thresholds.size() * sizeof(float);
	size_t totalSize = eventSize + spikeChannel->getTotalEventMetadataSize();

	if (totalSize < bufferSize)
	{
		jassertfalse;
		return;
	}

	char* buffer = static_cast<char*>(destinationBuffer);

	*(buffer + 0) = SPIKE_EVENT;
	*(buffer + 1) = static_cast<char>(spikeChannel->getChannelType());
	*(reinterpret_cast<uint16*>(buffer + 2)) = spikeChannel->getSourceNodeId();
	*(reinterpret_cast<uint16*>(buffer + 4)) = spikeChannel->getStreamId();
	*(reinterpret_cast<uint16*>(buffer + 6)) = spikeChannel->getLocalIndex();
	*(reinterpret_cast<juce::int64*>(buffer + 8)) = m_timestamp;
	*(reinterpret_cast<uint16*>(buffer + 16)) = m_sortedID;

	int memIdx = SPIKE_BASE_SIZE;

	for (int i = 0; i < m_thresholds.size(); i++)
	{
		*(reinterpret_cast<float*>(buffer + memIdx)) = m_thresholds[i];
		memIdx += sizeof(float);
	}

	memcpy((buffer + memIdx), m_data.getData(), dataSize);

	serializeMetadata(buffer + eventSize);
}

Spike* Spike::createBasicSpike(const SpikeChannel* channelInfo, 
	juce::int64 timestamp, 
	Array<float> thresholds, 
	Buffer& dataSource, 
	uint16 sortedID)
{
	if (!dataSource.m_ready)
	{
		jassertfalse;
		return nullptr;
	}
	if (channelInfo->getChannelType() == SpikeChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}
	int nChannels = channelInfo->getNumChannels();
	if (nChannels != dataSource.m_nChans)
	{
		jassertfalse;
		return nullptr;
	}
	int nSamples = channelInfo->getTotalSamples();
	if (nSamples != dataSource.m_nSamps)
	{
		jassertfalse;
		return nullptr;
	}
	if (thresholds.size() != nChannels)
	{
		jassertfalse;
		return nullptr;
	}
	
	dataSource.m_ready = false;

	return new Spike(channelInfo, timestamp, thresholds, dataSource.m_data, sortedID);

}

SpikePtr Spike::createSpike(const SpikeChannel* channelInfo, 
	juce::int64 timestamp, 
	Array<float> thresholds, 
	Spike::Buffer& dataSource, 
	uint16 sortedID)
{
	if (!channelInfo)
	{
		jassertfalse;
		return nullptr;
	}

	
	if (channelInfo->getEventMetadataCount() != 0)
	{
		jassertfalse;
		return nullptr;
	}

	return createBasicSpike(channelInfo, timestamp, thresholds, dataSource, sortedID);	
	
}

SpikePtr Spike::createSpike(const SpikeChannel* channelInfo, 
	juce::int64 timestamp, 
	Array<float> thresholds, 
	Spike::Buffer& dataSource,
	const MetadataValueArray& metaData,
	uint16 sortedID)
{
	if (!channelInfo)
	{
		jassertfalse;
		return nullptr;
	}

	if (!compareMetadata(channelInfo, metaData))
	{
		jassertfalse;
		return nullptr;
	}
	Spike* event = createBasicSpike(channelInfo, timestamp, thresholds, dataSource, sortedID);
	if (!event)
	{
		jassertfalse;
		return nullptr;
	}

	event->m_metaDataValues.addArray(metaData);
	return event;
}

void Spike::setSortedId(uint16 sortedId)
{
	uint8* modifiableBuffer = const_cast<uint8*>(buffer);

	*(reinterpret_cast<uint16*>(modifiableBuffer + 16)) = sortedId;
}

SpikePtr Spike::deserialize(const uint8* buffer, const SpikeChannel* channelInfo)
{
	int nChans = channelInfo->getNumChannels();
	size_t dataSize = channelInfo->getDataSize();
	size_t thresholdSize = nChans * sizeof(float);
	size_t metaDataSize = channelInfo->getTotalEventMetadataSize();

	if (static_cast<Event::Type>(*(buffer)) != SPIKE_EVENT)
	{
		jassertfalse;
		return nullptr;
	}

	if (static_cast<SpikeChannel::Type>(*(buffer + 1)) != channelInfo->getChannelType())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 2) != channelInfo->getSourceNodeId())
	{
		jassertfalse;
		return nullptr;
	}

	if (*reinterpret_cast<const uint16*>(buffer + 4) != channelInfo->getStreamId())
	{
		jassertfalse;
		return nullptr;
	}
	if (*reinterpret_cast<const uint16*>(buffer + 6) != channelInfo->getLocalIndex())
	{
		jassertfalse;
		return nullptr;
	}

	juce::int64 timestamp = *(reinterpret_cast<const juce::int64*>(buffer + 8));
	uint16 sortedID = *(reinterpret_cast<const uint16*>(buffer + 16));
	Array<float> thresholds;
	thresholds.addArray(reinterpret_cast<const float*>(buffer + SPIKE_BASE_SIZE), nChans);
	HeapBlock<float> data;
	data.malloc(dataSize, sizeof(char));
	memcpy(data.getData(), (buffer + SPIKE_BASE_SIZE + thresholdSize), dataSize);

	SpikePtr event = new Spike(channelInfo, timestamp, thresholds, data, sortedID);
	event->buffer = buffer;

	bool ret = true;
	if (metaDataSize > 0)
		ret = event->deserializeMetadata(channelInfo, (buffer + SPIKE_BASE_SIZE + dataSize + thresholdSize), metaDataSize);

	if (ret)
		return event.release();
	else
	{
		jassertfalse;
		return nullptr;
	}
}

SpikePtr Spike::deserialize(const EventPacket& packet, const SpikeChannel* channelInfo)
{

	if (channelInfo->getChannelType() == SpikeChannel::INVALID)
	{
		jassertfalse;
		return nullptr;
	}

	return deserialize(packet.getRawData(), channelInfo);

}

Spike::Buffer::Buffer(const SpikeChannel* channelInfo)
	: m_nChans(channelInfo->getNumChannels()),
	  m_nSamps(channelInfo->getTotalSamples()),
      spikeChannel(channelInfo)
{

	m_data.malloc(m_nChans*m_nSamps);

}

void  Spike::Buffer::set(const int chan, const int samp, const float value)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && samp >= 0 && chan < m_nChans && samp < m_nSamps);
	m_data[samp + chan*m_nSamps] = value;
}

void  Spike::Buffer::set(const int index, const float value)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(index >= 0 && index < m_nChans * m_nSamps);
	m_data[index] = value;
}

void  Spike::Buffer::set(const int chan, const float* source, const int n)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && chan < m_nChans && n <= m_nSamps);
	memcpy(m_data.getData(), source, n*sizeof(float));
}

void  Spike::Buffer::set(const int chan, const int start, const float* source, const int n)
{
	if (!m_ready)
	{
		jassertfalse;
		return;
	}
	jassert(chan >= 0 && chan < m_nChans && (n + start) <= m_nSamps);
	memcpy(m_data.getData() + start, source, n*sizeof(float));
}

float Spike::Buffer::get(const int chan, const int samp)
{
	if (!m_ready)
	{
		jassertfalse;
		return 0;
	}
	jassert(chan >= 0 && samp >= 0 && chan < m_nChans && samp < m_nSamps);
	return m_data[chan*m_nSamps + samp];
}

float Spike::Buffer::get(const int index)
{
	if (!m_ready)
	{
		jassertfalse;
		return 0;
	}
	jassert(index >= 0 && index < m_nChans * m_nSamps);
	return m_data[index];
}

const float* Spike::Buffer::getRawPointer()
{
	if (!m_ready)
	{
		jassertfalse;
		return nullptr;
	}
	return m_data.getData();
}
