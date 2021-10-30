/*
   ------------------------------------------------------------------

   This file is part of the Open Ephys GUI
   Copyright (C) 2016 Open Ephys

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

#include <stdio.h>

#include "ChannelMappingNode.h"
#include "ChannelMappingEditor.h"

#include "PrbFormat.h"

ChannelMapSettings::ChannelMapSettings()
{

}


void ChannelMapSettings::updateNumChannels(int newChannelCount)
{
    
    if (newChannelCount < channelOrder.size())
    {
        // TODO: deal with case where some channels may be out of range
    }
    else {
        for (int i = channelOrder.size(); i < newChannelCount; i++)
        {
            channelOrder.add(i);
            isEnabled.add(true);
            referenceIndex.add(-1);
        }
    }

    if (referenceChannels.size() < 4)
    {
        for (int i = 0; i < 4; i++)
        {
            referenceChannels.add(-1);
        }
    }
    else {
        for (int i = 0; i < 4; i++)
        {
            if (referenceChannels[i] > newChannelCount)
                referenceChannels.set(i, -1);
        }
    }

}

void ChannelMapSettings::toXml(XmlElement* xml)
{

    for (int ch = 0; ch < channelOrder.size(); ch++)
    {
        XmlElement* node = xml->createNewChildElement("CH");
        xml->setAttribute("index", ch);
        xml->setAttribute("order", channelOrder[ch]);
        xml->setAttribute("enabled", isEnabled[ch]);
        xml->setAttribute("referenceIndex", referenceIndex[ch]);
    }

    for (int i = 0; i < referenceChannels.size(); i++)
    {
        XmlElement* node = xml->createNewChildElement("REFERENCE");
        xml->setAttribute("channel", referenceChannels[i]);
    }

}

void ChannelMapSettings::fromXml(XmlElement* xml)
{
    channelOrder.clear();
    isEnabled.clear();
    referenceIndex.clear();
    referenceChannels.clear();

    int channelIndex = 0;

    forEachXmlChildElement(*xml, channelParams)
    {
        if (channelParams->hasTagName("CH"))
        {
            channelOrder.add(channelParams->getIntAttribute("order", channelIndex));
            isEnabled.add(channelParams->getBoolAttribute("enabled"), true);
            referenceIndex.add(channelParams->getIntAttribute("referenceIndex"), -1);
            channelIndex++;
        }

        if (channelParams->hasTagName("REFERENCE"))
        {
            referenceChannels.add(channelParams->getIntAttribute("channel"));
        }
    }
}

void ChannelMapSettings::toJson(File filename)
{
    PrbFormat::write(filename, *this);
}

void ChannelMapSettings::fromJson(File filename)
{
    PrbFormat::read(filename, *this);
}

// =====================================================

ChannelMappingNode::ChannelMappingNode()
    : GenericProcessor  ("Channel Map")
{
    
    setProcessorType (PROCESSOR_TYPE_FILTER);

}

AudioProcessorEditor* ChannelMappingNode::createEditor()
{
    editor = std::make_unique<ChannelMappingEditor> (this);

    return editor.get();
}


void ChannelMappingNode::updateSettings()
{

    settings.update(getDataStreams());

    Array<ContinuousChannel*> newChannelOrder;
    OwnedArray<ContinuousChannel> channelsToDelete;

    for (auto stream : getDataStreams())
    {
        settings[stream->getStreamId()]->updateNumChannels(stream->getChannelCount());

        for (int ch = 0; ch < stream->getChannelCount(); ch++)
        {
            int localIndex = settings[stream->getStreamId()]->channelOrder[ch];
            int globalIndex = stream->getContinuousChannels()[localIndex]->getGlobalIndex();

            if (settings[stream->getStreamId()]->isEnabled[ch])
            {
                newChannelOrder.add(continuousChannels[globalIndex]);
            }
            else {
                channelsToDelete.add(continuousChannels[globalIndex]);
            }
        }
    }

    continuousChannels.clear(false); // don't delete the remaining channels

    for (auto channel : newChannelOrder)
    {
        continuousChannels.add(channel);
    }

    channelsToDelete.clear(); // delete the unused channels

}


void ChannelMappingNode::setChannelEnabled(uint16 streamId, int channelNum, int isEnabled)
{
    settings[streamId]->isEnabled.set(channelNum, isEnabled);
}

void ChannelMappingNode::setChannelOrder(uint16 streamId, Array<int> order)
{
    settings[streamId]->channelOrder = order;
}

void ChannelMappingNode::setReferenceChannel(uint16 streamId, int referenceNum, int localChannel)
{
    currentStream = streamId;
    currentChannel = referenceNum;

    setParameter(1, localChannel);
}


void ChannelMappingNode::setReferenceIndex(uint16 streamId, int channelNum, int referenceIndex)
{
    currentStream = streamId;
    currentChannel = channelNum;

    setParameter(2, referenceIndex);
}

Array<int> ChannelMappingNode::getChannelOrder(uint16 streamId)
{
    return settings[streamId]->channelOrder;
}

String ChannelMappingNode::loadStreamSettings(uint16 streamId, File& file)
{
    settings[streamId]->fromJson(file);
    
    return ("Loaded stream settings.");
}

String ChannelMappingNode::writeStreamSettings(uint16 streamId, File& file)
{
    settings[streamId]->toJson(file);
    
    return ("Wrote stream settings.");
}


Array<bool> ChannelMappingNode::getChannelEnabledState(uint16 streamId)
{
    return settings[streamId]->isEnabled;
}

int ChannelMappingNode::getReferenceChannel(uint16 streamId, int referenceIndex)
{
    return settings[streamId]->referenceChannels[referenceIndex];
}

Array<int> ChannelMappingNode::getChannelsForReference(uint16 streamId, int referenceIndex)
{
    Array<int> channels;

    for (int i = 0; i < settings[streamId]->referenceIndex.size(); i++)
    {
        if (settings[streamId]->referenceIndex[i] == referenceIndex)
            channels.add(i);
    }

    return channels;
}


void ChannelMappingNode::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex == 1)
    {
        settings[currentStream]->referenceChannels.set (currentChannel, (int) newValue);
    }
    else if (parameterIndex == 2)
    {
        settings[currentStream]->referenceIndex.set (currentChannel, (int) newValue);
    }
}


void ChannelMappingNode::process (AudioBuffer<float>& buffer)
{

    for (auto stream : getDataStreams())
    {
        uint16 streamId = stream->getStreamId();

        for (int ch = 0; ch < stream->getChannelCount(); ch++)
        {
            if (settings[streamId]->referenceIndex[ch] > -1)
            {
                int referenceChannelLocalIndex = settings[streamId]->referenceChannels[settings[streamId]->referenceIndex[ch]];
                
                if (referenceChannelLocalIndex > -1)
                {
                    int referenceChannelGlobalIndex = stream->getContinuousChannels()[referenceChannelLocalIndex]->getGlobalIndex();
                    int channelGlobalIndex = stream->getContinuousChannels()[ch]->getGlobalIndex();

                    buffer.addFrom(channelGlobalIndex, // destChannel
                        0,                             // destStartSample
                        buffer,                        // source
                        referenceChannelGlobalIndex,   // sourceChannel
                        0,                             // sourceStartSample
                        getNumSourceSamples(streamId), // numSamples
                        -1.0f);                        // gain to apply to source (negative for reference)
                }
            }
        }
    }

    /*int j = 0;
    int i = 0;
    int realChan;

    // use copy constructor to set the data to refer to
    AudioBuffer<float> channelBuffer = buffer;

    while (j < getNumOutputs())
    {
        realChan = channelArray[i];
        if ((realChan < channelBuffer.getNumChannels())
            && (enabledChannelArray[realChan]))
        {
            // copy it back into the buffer according to the channel mapping
            buffer.copyFrom (j,                                       // destChannel
                             0,                                       // destStartSample
                             channelBuffer.getReadPointer (realChan), // source
                             getNumSamples (j),                       // numSamples
                             1.0f); // gain to apply to source (positive for original signal)

            // now do the referencing
            if ((referenceArray[realChan] > -1)
                && (referenceChannels[referenceArray[realChan]] > -1)
                && (referenceChannels[referenceArray[realChan]] < channelBuffer.getNumChannels()))
            {
                buffer.addFrom (j,                                                                // destChannel
                                0,                                                                // destStartSample
                                channelBuffer,                                                    // source
                                channelArray[referenceChannels[referenceArray[realChan]]], // sourceChannel
                                0,                                                                // sourceStartSample
                                getNumSamples (j),                                                // numSamples
                                -1.0f); // gain to apply to source (negative for reference)
            }

            ++j;
        }

        ++i;
    }*/
}


void ChannelMappingNode::saveCustomParametersToXml(XmlElement* xml)
{
    for (auto stream : getDataStreams())
    {

        XmlElement* streamParams = xml->createNewChildElement("STREAM");

        settings[stream->getStreamId()]->toXml(streamParams);
    }
}


void ChannelMappingNode::loadCustomParametersFromXml(XmlElement* xml)
{

    std::cout << "Filter node loading custom parameters" << std::endl;
    int streamIndex = 0;
    Array<const DataStream*> availableStreams = getDataStreams();

    forEachXmlChildElement(*xml, streamParams)
    {
        if (streamParams->hasTagName("STREAM"))
        {

            LOGD("STREAM ", streamIndex);
            if (availableStreams.size() > streamIndex)
            {
                LOGD("FOUND IT!");
                settings[availableStreams[streamIndex]->getStreamId()]->fromXml(streamParams);
            }
            else {
                LOGD("DID NOT FIND IT!");
            }

            streamIndex++;
        }
    }

}
