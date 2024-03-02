/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#include "SpikeDetectorActions.h"

AddSpikeChannels::AddSpikeChannels(SpikeDetector* processor_,
                                      DataStream* stream_,
                                      SpikeChannel::Type type_,
                                      int count_, //adds multiple channels at once
                                      Array<int> startChannels_,
                                      int nextAvailableChannel_) :
    OpenEphysAction("AddSpikeChannels"),
    spikeDetector(processor_),
    streamKey(stream_->getKey()),
    type(type_),
    count(count_),
    startChannels(startChannels_),
    nextAvailableChannel(nextAvailableChannel_)
{
    settings = nullptr;
}

AddSpikeChannels::~AddSpikeChannels()
{
    if (settings != nullptr)
        delete settings;
}

bool AddSpikeChannels::perform()
{
    if (!startChannels.size())
    {
        for (int i = 0; i < count; i++)
            startChannels.add(nextAvailableChannel+i*SpikeChannel::getNumChannels(type));
    }

    for (int i = 0; i < count; i++)
    {
        //TODO: Should add a convenience function for this
        uint16 streamId = 0;
        for (auto stream : spikeDetector->getDataStreams())
        {
            if (stream->getKey() == streamKey)
            {
                streamId = stream->getStreamId();
                break;
            }
        }
        if (streamId == 0) return false;

        //TODO: Pass stream name instead of streamId once this is working
        SpikeChannel* spikeChannel = spikeDetector->addSpikeChannel(type, streamId, startChannels[i]);
        addedSpikeChannels.add(spikeChannel->getIdentifier());
    }
    spikeDetector->registerUndoableAction(spikeDetector->getNodeId(), this);
    CoreServices::updateSignalChain(spikeDetector);

    return true;
}

bool AddSpikeChannels::undo()
{
    uint16 streamId = 0;
    for (auto stream : spikeDetector->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0) return false;
    for (int i = 0; i < count; i++)
        spikeDetector->removeSpikeChannel(addedSpikeChannels[i]);
    CoreServices::updateSignalChain(spikeDetector);
    return true;
}

void AddSpikeChannels::restoreOwner(GenericProcessor* owner)
{
    LOGD("RESTORING OWNER FOR: AddSpikeChannels");
    spikeDetector = (SpikeDetector*)owner;
}

RemoveSpikeChannels::RemoveSpikeChannels(SpikeDetector* processor_,
                                         DataStream* stream_,
                                         Array<SpikeChannel*> spikeChannelsToRemove_,
                                         Array<int> indeces_) :
    OpenEphysAction("RemoveSpikeChannels"),
    spikeDetector(processor_),
    indeces(indeces_),
    streamKey(stream_->getKey())
{
    settings = std::make_unique<XmlElement>("SPIKE_CHANNELS");

    for (auto spikeChannel : spikeChannelsToRemove_)
    {
        if (spikeChannel->isLocal())
        {
            LOGD("REMOVING SPIKE CHANNEL: ", spikeChannel->getName());

            XmlElement* spikeParamsXml = settings->createNewChildElement("SPIKE_CHANNEL");

            // general settings
            spikeParamsXml->setAttribute("name", spikeChannel->getName());
            spikeParamsXml->setAttribute("description", spikeChannel->getDescription());
            spikeParamsXml->setAttribute("num_channels", (int)spikeChannel->getNumChannels());

            // stream info
            spikeParamsXml->setAttribute("sample_rate", spikeChannel->getSampleRate());
            spikeParamsXml->setAttribute("stream_name", stream_->getName());
            spikeParamsXml->setAttribute("stream_source", stream_->getSourceNodeId());

            // parameters
            spikeChannel->getParameter("local_channels")->toXml(spikeParamsXml);
            spikeChannel->getParameter("thrshlder_type")->toXml(spikeParamsXml);

            for (int ch = 0; ch < (int)spikeChannel->getNumChannels(); ch++)
            {
                spikeChannel->getParameter("abs_threshold" + String(ch+1))->toXml(spikeParamsXml);
                spikeChannel->getParameter("std_threshold" + String(ch+1))->toXml(spikeParamsXml);
                spikeChannel->getParameter("dyn_threshold" + String(ch+1))->toXml(spikeParamsXml);
            }

            spikeChannel->getParameter("waveform_type")->toXml(spikeParamsXml);

            spikeChannelsToRemove.add(spikeChannel->getIdentifier());
        }
    }
}

RemoveSpikeChannels::~RemoveSpikeChannels()
{
}

void RemoveSpikeChannels::restoreOwner(GenericProcessor* processor)
{
    spikeDetector = (SpikeDetector*)processor;
}

bool RemoveSpikeChannels::perform()
{
    for (auto spikeChannel : spikeChannelsToRemove)
        spikeDetector->removeSpikeChannel(spikeChannel);
    spikeDetector->registerUndoableAction(spikeDetector->getNodeId(), this);
    CoreServices::updateSignalChain(spikeDetector);

    return true;
}

bool RemoveSpikeChannels::undo()
{
    int idx = 0;
    for (auto* spikeParamsXml : settings->getChildIterator())
    {
        String name = spikeParamsXml->getStringAttribute("name", "");

        double sample_rate = spikeParamsXml->getDoubleAttribute("sample_rate", 0.0f);
        String stream_name = spikeParamsXml->getStringAttribute("stream_name", "");
        int stream_source = spikeParamsXml->getIntAttribute("stream_source", 0);

        SpikeChannel::Type type = SpikeChannel::typeFromNumChannels(spikeParamsXml->getIntAttribute("num_channels", 1));

        if (!spikeDetector->alreadyLoaded(name, type, stream_source, stream_name))
        {
            uint16 streamId = spikeDetector->findSimilarStream(stream_source, stream_name, sample_rate, true);

            SpikeChannel* spikeChannel = spikeDetector->addSpikeChannel(type, streamId, -1, name, indeces[idx]);

            spikeChannel->getParameter("local_channels")->fromXml(spikeParamsXml);

            SelectedChannelsParameter* param = (SelectedChannelsParameter*)spikeChannel->getParameter("local_channels");
            ((SpikeChannel*)param->getOwner())->localChannelIndexes = param->getArrayValue();

            spikeChannel->getParameter("thrshlder_type")->fromXml(spikeParamsXml);

            for (int ch = 0; ch < SpikeChannel::getNumChannels(type); ch++)
            {
                spikeChannel->getParameter("abs_threshold" + String(ch+1))->fromXml(spikeParamsXml);
                spikeChannel->getParameter("std_threshold" + String(ch+1))->fromXml(spikeParamsXml);
                spikeChannel->getParameter("dyn_threshold" + String(ch+1))->fromXml(spikeParamsXml);
            }

            spikeChannel->getParameter("waveform_type")->fromXml(spikeParamsXml);
        }
        idx++;
    }
    CoreServices::updateSignalChain(spikeDetector);
    return true;
}
