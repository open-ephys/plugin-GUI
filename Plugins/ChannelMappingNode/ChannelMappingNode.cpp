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

    Array<int> newChannelOrder;
    Array<bool> newIsEnabled;

    // handle case where channel order is reduced
    for (int i = 0; i < channelOrder.size(); i++)
    {
        if (channelOrder[i] < newChannelCount)
        {
            newChannelOrder.add(channelOrder[i]);
            newIsEnabled.add(isEnabled[i]);
        }
    }

    channelOrder = newChannelOrder;
    isEnabled = newIsEnabled;

    // add more channels if necessary
    for (int i = channelOrder.size(); i < newChannelCount; i++)
    {
        channelOrder.add(i);
        isEnabled.add(true);
    }

    numChannels = newChannelCount;

}

void ChannelMapSettings::toXml(XmlElement* xml)
{

    for (int ch = 0; ch < channelOrder.size(); ch++)
    {
        XmlElement* node = xml->createNewChildElement("CH");
        node->setAttribute("index", channelOrder[ch]);
        node->setAttribute("enabled", isEnabled[ch]);
    }

}

void ChannelMapSettings::fromXml(XmlElement* xml)
{
    channelOrder.clear();
    isEnabled.clear();

    int channelIndex = 0;

    for (auto* channelParams : xml->getChildIterator())
    {
        if (channelParams->hasTagName("CH"))
        {
            
            //std::cout << "ORDER: " << channelParams->getIntAttribute("index") << ", ENABLED: " << channelParams->getBoolAttribute("enabled") << std::endl;
            
            channelOrder.add(channelParams->getIntAttribute("index", channelIndex));
            isEnabled.add(channelParams->getBoolAttribute("enabled", true));
            
            
            channelIndex++;
        } else {
            std::cout << channelParams->getTagName() << std::endl;
        }
    }
}

void ChannelMapSettings::toJson(File filename)
{
    PrbFormat::write(filename, this);
}

void ChannelMapSettings::fromJson(File filename)
{
    PrbFormat::read(filename, this);
}

void ChannelMapSettings::setStream(const DataStream* stream)
{
    numChannels = stream->getChannelCount();
    sampleRate = stream->getSampleRate();
    streamName = stream->getName();
    streamId = stream->getStreamId();
    sourceNodeId = stream->getSourceNodeId();
}

void ChannelMapSettings::reset()
{
    channelOrder.clear();
    isEnabled.clear();

    for (int i = 0; i < numChannels; i++)
    {
        channelOrder.add(i);
        isEnabled.add(true);
    }
}

// =====================================================

ChannelMappingNode::ChannelMappingNode()
    : GenericProcessor  ("Channel Map")
{
    
}

AudioProcessorEditor* ChannelMappingNode::createEditor()
{
    editor = std::make_unique<ChannelMappingEditor> (this);

    return editor.get();
}


ChannelMapSettings* ChannelMappingNode::findMatchingStreamSettings(ChannelMapSettings* s)
{
    for (auto streamId : previousStreamIds)
    {
        if ((s->sourceNodeId == settings[streamId]->sourceNodeId) &&
            (s->streamName == settings[streamId]->streamName))
        {
            // perfect match
            return settings[streamId];
        }

        if ((s->streamName == settings[streamId]->streamName))
        {
            // matching name
            return settings[streamId];
        }
    }

    return nullptr;
}

void ChannelMappingNode::updateSettings()
{

    settings.update(getDataStreams());

    for (auto stream : getDataStreams())
    {

        const uint16 streamId = stream->getStreamId();

        if ( settings[streamId]->sourceNodeId == -1) // no stream applied yet
        {
            settings[streamId]->setStream(stream);

            ChannelMapSettings* s = findMatchingStreamSettings(settings[streamId]);

            if (s != nullptr)
            {
                settings[streamId]->channelOrder = s->channelOrder;
                settings[streamId]->isEnabled = s->isEnabled;
            }

            previousStreamIds.add(streamId);

        }
        
        settings[streamId]->updateNumChannels(stream->getChannelCount());

        if ((*stream)["enable_stream"])
        {
            Array<ContinuousChannel*> newChannelOrder;

            for (int ch = 0; ch < stream->getChannelCount(); ch++)
            {

                int localIndex = settings[streamId]->channelOrder[ch];
                Array<ContinuousChannel*> channelsForStream = stream->getContinuousChannels();

                int globalIndex = channelsForStream[localIndex]->getGlobalIndex();

                if (settings[streamId]->isEnabled[ch])
                {
                    newChannelOrder.add(channelsForStream[localIndex]);
                }

            }

            DataStream* currentStream = getDataStream(streamId);

            currentStream->clearContinuousChannels();

            for (int i = 0; i < newChannelOrder.size(); i++)
                currentStream->addChannel(newChannelOrder[i]);
        }

    }

}

void ChannelMappingNode::setChannelEnabled(uint16 streamId, int channelNum, int isEnabled)
{
    settings[streamId]->isEnabled.set(channelNum, isEnabled);
}

void ChannelMappingNode::setChannelOrder(uint16 streamId, Array<int> order)
{
    settings[streamId]->channelOrder = order;
}

void ChannelMappingNode::resetStream(uint16 streamId)
{
    settings[streamId]->reset();
}

Array<int> ChannelMappingNode::getChannelOrder(uint16 streamId)
{
    return settings[streamId]->channelOrder;
}

String ChannelMappingNode::loadStreamSettings(uint16 streamId, File& file)
{
    settings[streamId]->fromJson(file);
    
    return ("Loaded Channel Map settings from " + file.getFileName());
}

String ChannelMappingNode::writeStreamSettings(uint16 streamId, File& file)
{
    settings[streamId]->toJson(file);
    
    return ("Wrote Channel Map settings to " + file.getFileName());
}


Array<bool> ChannelMappingNode::getChannelEnabledState(uint16 streamId)
{
    return settings[streamId]->isEnabled;
}



void ChannelMappingNode::process (AudioBuffer<float>& buffer)
{
    // nothing needed here, since the mapping takes place at the connection level
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

    int streamIndex = 0;
    Array<const DataStream*> availableStreams = getDataStreams();

    for (auto* streamParams : xml->getChildIterator())
    {
        if (streamParams->hasTagName("STREAM"))
        {
            if (availableStreams.size() > streamIndex)
            {
                settings[availableStreams[streamIndex]->getStreamId()]->fromXml(streamParams);
            }
            else {
            }

            streamIndex++;
        }
    }

}
