/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

SpikeChannel::SpikeChannel (SpikeChannel::Settings settings)
    : ChannelInfoObject (InfoObject::Type::SPIKE_CHANNEL, nullptr),
      ParameterOwner (ParameterOwner::Type::SPIKE_CHANNEL),
      type (settings.type),
      localChannelIndexes (settings.localChannelIndexes),
      numPreSamples (settings.numPrePeakSamples),
      numPostSamples (settings.numPostPeakSamples),
      sendFullWaveform (settings.sendFullWaveform),
      currentSampleIndex (0),
      lastBufferIndex (0),
      useOverflowBuffer (false)
{
    setName (settings.name);
    setDescription (settings.description);
    setIdentifier (settings.identifier);
}

SpikeChannel::~SpikeChannel() {}

SpikeChannel::SpikeChannel (const SpikeChannel& other)
    : ChannelInfoObject (other),
      ParameterOwner (ParameterOwner::Type::SPIKE_CHANNEL),
      type (other.type),
      localChannelIndexes (other.localChannelIndexes),
      numPreSamples (other.getPrePeakSamples()),
      numPostSamples (other.getPostPeakSamples()),
      sendFullWaveform (other.sendFullWaveform),
      currentSampleIndex (0),
      lastBufferIndex (0),
      useOverflowBuffer (false),
      lastStreamId (other.lastStreamId),
      lastStreamName (other.lastStreamName),
      lastStreamSampleRate (other.lastStreamSampleRate),
      lastStreamChannelCount (other.lastStreamChannelCount)
{
    setName (other.getName());
    setDescription (other.getDescription());
    setIdentifier (other.getIdentifier());
}

void SpikeChannel::setDataStream (DataStream* dataStream, bool addToStream)
{
    stream = dataStream;

    channelIsEnabled.clear();

    Array<const ContinuousChannel*> newSourceChannels;

    Array<ContinuousChannel*> availableChannels;

    if (dataStream != nullptr)
    {
        availableChannels = dataStream->getContinuousChannels();
    }

    for (int i = 0; i < getNumChannels(); i++)
    {
        if (i >= localChannelIndexes.size())
        {
            channelIsEnabled.add (false);
            newSourceChannels.add (nullptr);
            continue;
        }

        if (localChannelIndexes[i] < availableChannels.size())
        {
            newSourceChannels.add (availableChannels[localChannelIndexes[i]]);
            channelIsEnabled.add (true);
            continue;
        }

        channelIsEnabled.add (false);
        newSourceChannels.add (nullptr);
    }

    setSourceChannels (newSourceChannels);

    if (dataStream != nullptr)
    {
        lastStreamId = dataStream->getStreamId();
        lastStreamName = dataStream->getName();
        lastStreamSampleRate = dataStream->getSampleRate();
        lastStreamChannelCount = dataStream->getChannelCount();

        if (addToStream)
        {
            dataStream->addChannel (this);
        }
    }
}

/** Find similar stream*/
DataStream* SpikeChannel::findSimilarStream (OwnedArray<DataStream>& streams)
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

void SpikeChannel::setSourceChannels (Array<const ContinuousChannel*>& newChannels)
{
    sourceChannels.clear();
    globalChannelIndexes.clear();

    for (int i = 0; i < newChannels.size(); i++)
    {
        sourceChannels.add (newChannels[i]);

        if (newChannels[i] != nullptr)
            globalChannelIndexes.add (newChannels[i]->getGlobalIndex());
        else
            globalChannelIndexes.add (-1);
    }
}

bool SpikeChannel::detectSpikesOnChannel (int chan) const
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

void SpikeChannel::parameterChangeRequest (Parameter* param)
{
    processorChain.getLast()->parameterChangeRequest (param);
}

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
    return getNumChannels (type);
}

unsigned int SpikeChannel::getNumChannels (SpikeChannel::Type type)
{
    switch (type)
    {
        case SINGLE:
            return 1;
        case STEREOTRODE:
            return 2;
        case TETRODE:
            return 4;
        default:
            return 0;
    }
}

SpikeChannel::Type SpikeChannel::typeFromNumChannels (unsigned int nChannels)
{
    switch (nChannels)
    {
        case 1:
            return SINGLE;
        case 2:
            return STEREOTRODE;
        case 4:
            return TETRODE;
        default:
            return INVALID;
    }
}

size_t SpikeChannel::getDataSize() const
{
    return getTotalSamples() * getNumChannels() * sizeof (float);
}

size_t SpikeChannel::getChannelDataSize() const
{
    return getTotalSamples() * sizeof (float);
}

float SpikeChannel::getChannelBitVolts (int index) const
{
    if (index < 0 || index >= sourceChannels.size())
        return 1.0f;
    else
        return sourceChannels[index]->getBitVolts();
}

String SpikeChannel::getDefaultChannelPrefix (SpikeChannel::Type channelType)
{
    switch (channelType)
    {
        case SpikeChannel::Type::SINGLE:
            return "Electrode ";
        case SpikeChannel::Type::STEREOTRODE:
            return "Stereotrode ";
        case SpikeChannel::Type::TETRODE:
            return "Tetrode ";
        case SpikeChannel::Type::TEMPLATE:
            return "Template ";
        default:
            return "Spike Channel ";
    }
}

String SpikeChannel::getDescriptionFromType (SpikeChannel::Type channelType)
{
    switch (channelType)
    {
        case SpikeChannel::Type::SINGLE:
            return "Single electrode";
        case SpikeChannel::Type::STEREOTRODE:
            return "Stereotrode";
        case SpikeChannel::Type::TETRODE:
            return "Tetrode";
        case SpikeChannel::Type::TEMPLATE:
            return "Template";
        default:
            return "Unknown";
    }
}

String SpikeChannel::getIdentifierFromType (SpikeChannel::Type channelType)
{
    switch (channelType)
    {
        case SpikeChannel::Type::SINGLE:
            return "spikechannel.singleelectrode";
        case SpikeChannel::Type::STEREOTRODE:
            return "spikechannel.stereotrode";
        case SpikeChannel::Type::TETRODE:
            return "spikechannel.tetrode";
        case SpikeChannel::Type::TEMPLATE:
            return "spikechannel.template";
        default:
            return "spikechannel.unknown";
    }
}
