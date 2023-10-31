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
	: ChannelInfoObject(InfoObject::Type::SPIKE_CHANNEL, nullptr),
	ParameterOwner(ParameterOwner::Type::SPIKE_CHANNEL),
	type(settings.type),
    localChannelIndexes(settings.localChannelIndexes),
	numPreSamples(settings.numPrePeakSamples),
	numPostSamples(settings.numPostPeakSamples),
    sendFullWaveform(settings.sendFullWaveform),
    currentSampleIndex(0),
    lastBufferIndex(0),
    useOverflowBuffer(false)
{
	setName(settings.name);
	setDescription(settings.description);
	setIdentifier(settings.identifier);
}


SpikeChannel::~SpikeChannel() { }

SpikeChannel::SpikeChannel(const SpikeChannel& other)
 : ChannelInfoObject(other),
	 ParameterOwner(ParameterOwner::Type::SPIKE_CHANNEL),
     type(other.type),
     localChannelIndexes(other.localChannelIndexes),
     numPreSamples(other.getPrePeakSamples()),
     numPostSamples(other.getPostPeakSamples()),
     sendFullWaveform(other.sendFullWaveform),
     currentSampleIndex(0),
     lastBufferIndex(0),
     useOverflowBuffer(false),
     lastStreamId(other.lastStreamId),
     lastStreamName(other.lastStreamName),
     lastStreamSampleRate(other.lastStreamSampleRate),
     lastStreamChannelCount(other.lastStreamChannelCount)
{
    setName(other.getName());
    setDescription(other.getDescription());
    setIdentifier(other.getIdentifier());
}


void SpikeChannel::setDataStream(DataStream* dataStream, bool addToStream)
{
    stream = dataStream;
    
    channelIsEnabled.clear();
    
    Array<const ContinuousChannel*> newSourceChannels;
    
    Array<ContinuousChannel*> availableChannels;
    
    //std::cout << "Setting data stream to " << dataStream->getName() << " (" << dataStream->getStreamId() << ")" << std::endl;
    //std::cout << "Num local channels in SpikeChannel: " << localChannelIndexes.size() << std::endl;
    
    
    if (dataStream != nullptr)
    {
        availableChannels = dataStream->getContinuousChannels();
        //std::cout << "Num local channels in dataStream: " << dataStream->getChannelCount() << std::endl;
    } //else {
      //  std::cout << "Num local channels in dataStream: 0" << std::endl;
   // }
        
    
    for (int i = 0; i < getNumChannels(); i++)
    {
        if (i >= localChannelIndexes.size())
        {
            //std::cout << "Adding continuous channel NULL (not enough available)" << std::endl;
            channelIsEnabled.add(false);
            newSourceChannels.add(nullptr);
            continue;
        }
            
        if (localChannelIndexes[i] < availableChannels.size())
        {
            //std::cout << "Adding continuous channel " << availableChannels[localChannelIndexes[i]]->getName() << std::endl;
            newSourceChannels.add(availableChannels[localChannelIndexes[i]]);
            channelIsEnabled.add(true);
            continue;
        }
        
        //std::cout << "Adding continuous channel NULL (no data stream)" << std::endl;
        channelIsEnabled.add(false);
        newSourceChannels.add(nullptr);
    }
    
    setSourceChannels(newSourceChannels);
    
    if (dataStream != nullptr)
    {
        lastStreamId = dataStream->getStreamId();
        lastStreamName = dataStream->getName();
        lastStreamSampleRate = dataStream->getSampleRate();
        lastStreamChannelCount = dataStream->getChannelCount();
        
        if (addToStream)
        {
            //std::cout << "Data stream adding spike channel" << std::endl;
            dataStream->addChannel(this);
        }
            
    }

}

/** Find similar stream*/
DataStream* SpikeChannel::findSimilarStream(OwnedArray<DataStream>& streams)
{
    for (auto stream : streams)
    {
        // matching ID -- looks great
        if (stream->getStreamId() == lastStreamId)
            return stream;
    }
    
    for (auto stream : streams)
   {
       if (stream->getName() == lastStreamName)
           return stream;
   }
    
    for (auto stream : streams)
    {
        if (stream->getSampleRate() == lastStreamSampleRate)
            return stream;
    }
    
    return nullptr;
}


SpikeChannel::Type SpikeChannel::getChannelType() const
{
	return type;
}

bool SpikeChannel::isValid() const
{
    for (auto ch : getSourceChannels())
    {
        if (ch == nullptr)
            return false;
    }
    
    return true;
}


const Array<const ContinuousChannel*>& SpikeChannel::getSourceChannels() const
{
	return sourceChannels;
}

void SpikeChannel::setSourceChannels(Array<const ContinuousChannel*>& newChannels)
{

	//jassert(newChannels.size() == sourceChannels.size());

    //std::cout << "SETTING SOURCE CHANNELS: " << newChannels.size() << std::endl;

	sourceChannels.clear();
    globalChannelIndexes.clear();

	for (int i = 0; i < newChannels.size(); i++)
	{
		sourceChannels.add(newChannels[i]);
        //std::cout << "Spike channel adding source channel: " << newChannels[i] << std::endl;
        
        if (newChannels[i] != nullptr)
            globalChannelIndexes.add(newChannels[i]->getGlobalIndex());
        else
            globalChannelIndexes.add(-1);

        //std::cout << " >>> Global index: " << globalChannelIndexes[i] << std::endl;
	}
    
}

bool SpikeChannel::detectSpikesOnChannel(int chan) const
{
    
    if (chan >= 0 && chan < getNumChannels())
        return channelIsEnabled[chan];
    else
        return false;
}

void SpikeChannel::reset()
{
    currentSampleIndex = 0;
    lastBufferIndex = 0;
    useOverflowBuffer = false;
}

void SpikeChannel::parameterChangeRequest(Parameter* param)
{
	processorChain.getLast()->parameterChangeRequest(param);
}

/*SpikeChannel::ThresholdType SpikeChannel::getThresholdType() const
{
	return thresholdType;
}


void SpikeChannel::setThresholdType(SpikeChannel::ThresholdType tType) 
{
	thresholdType = tType;
}


float SpikeChannel::getThreshold(int channelIndex) const
{
	if (channelIndex > -1 && channelIndex < thresholds.size())
	{
		return thresholds[channelIndex];
	}
	else {
		return 0.0f;
	}
}

void SpikeChannel::setThreshold(int channelIndex, float threshold)
{
	if (channelIndex > -1 && channelIndex < thresholds.size())
	{
		return thresholds.set(channelIndex, threshold);
	}
}

bool SpikeChannel::sendsFullWaveform() const
{
	return sendFullWaveform;
}

void SpikeChannel::shouldSendFullWaveform(bool state)
{
	sendFullWaveform = state;
}

bool SpikeChannel::getSourceChannelState(int channelIndex) const
{
	if (channelIndex > -1 && channelIndex < detectSpikesOnChannel.size())
	{
		return detectSpikesOnChannel[channelIndex];
	}
	else {
		return false;
	}
}

void SpikeChannel::setSourceChannelState(int channelIndex, bool state)
{
	if (channelIndex > -1 && channelIndex < detectSpikesOnChannel.size())
	{
		return thresholds.set(channelIndex, state);
	}
}

void SpikeChannel::setNumSamples(unsigned int preSamples, unsigned int postSamples)
{
	numPreSamples = preSamples;
	numPostSamples = postSamples;
}*/

unsigned int SpikeChannel::getPrePeakSamples() const
{
	return numPreSamples;
}

unsigned int SpikeChannel::getPostPeakSamples() const
{
	return numPostSamples;
}

unsigned int SpikeChannel::getTotalSamples() const
{
    if (sendFullWaveform)
        return numPostSamples + numPreSamples;
    else
        return 1;
}

unsigned int SpikeChannel::getNumChannels() const
{
	return getNumChannels(type);
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
	return getTotalSamples() * getNumChannels() * sizeof(float);
}

size_t SpikeChannel::getChannelDataSize() const
{
	return getTotalSamples()*sizeof(float);
}

float SpikeChannel::getChannelBitVolts(int index) const
{
	if (index < 0 || index >= sourceChannels.size())
		return 1.0f;
	else
		return sourceChannels[index]->getBitVolts();
}


String SpikeChannel::getDefaultChannelPrefix(SpikeChannel::Type channelType)
{
	switch (channelType)
	{
	case SpikeChannel::Type::SINGLE:
		return "Electrode ";
	case SpikeChannel::Type::STEREOTRODE:
		return "Stereotrode ";
	case SpikeChannel::Type::TETRODE:
		return "Tetrode ";
	default:
		return "Spike Channel ";
	}
}

String SpikeChannel::getDescriptionFromType(SpikeChannel::Type channelType)
{
	switch (channelType)
	{
	case SpikeChannel::Type::SINGLE:
		return "Single electrode";
	case SpikeChannel::Type::STEREOTRODE:
		return "Stereotrode";
	case SpikeChannel::Type::TETRODE:
		return "Tetrode";
	default:
		return "Unknown";
	}
}

String SpikeChannel::getIdentifierFromType(SpikeChannel::Type channelType)
{
	switch (channelType)
	{
	case SpikeChannel::Type::SINGLE:
		return "spikechannel.singleelectrode";
	case SpikeChannel::Type::STEREOTRODE:
		return "spikechannel.stereotrode";
	case SpikeChannel::Type::TETRODE:
		return "spikechannel.tetrode";
	default:
		return "spikechannel.unknown";
	}
}


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
