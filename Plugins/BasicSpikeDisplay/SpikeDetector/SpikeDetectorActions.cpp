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
                                      int count_, //adds multiple channels atonce
                                      Array<int> startChannels_) :
    processor(processor_),
    streamId(stream_->getStreamId()),
    type(type_),
    count(count_),
    startChannels(startChannels_)
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
    for (int i = 0; i < count; i++)
    {
        int startChannel = -1;

        if (i < startChannels.size())
            startChannel = startChannels[i];

        processor->addSpikeChannel(type, streamId, startChannel);
    }
    return true;
}

bool AddSpikeChannels::undo()
{
    for (int i = 0; i < count; i++)
        processor->removeSpikeChannel(processor->getSpikeChannelsForStream(streamId).getLast());
    return true;
}

RemoveSpikeChannels::RemoveSpikeChannels(SpikeDetector* processor_,
                                         DataStream* stream_,
                                         Array<SpikeChannel*> spikeChannelsToRemove_) :
    processor(processor_),
    spikeChannelsToRemove(spikeChannelsToRemove_),
    streamId(stream_->getStreamId())
{
    settings = std::make_unique<XmlElement>("SPIKE_CHANNELS");

    for (auto spikeChannel : spikeChannelsToRemove)
    {
        if (spikeChannel->isLocal())
        {

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
        }
    }
}

RemoveSpikeChannels::~RemoveSpikeChannels()
{
}

bool RemoveSpikeChannels::perform()
{
    for (auto spikeChannel : spikeChannelsToRemove)
        processor->removeSpikeChannel(spikeChannel);
    return true;
}

bool RemoveSpikeChannels::undo()
{
    for (auto* spikeParamsXml : settings->getChildIterator())
    {
        String name = spikeParamsXml->getStringAttribute("name", "");

        //std::cout << "SPIKE CHANNEL NAME: " << name << std::endl;

        double sample_rate = spikeParamsXml->getDoubleAttribute("sample_rate", 0.0f);
        String stream_name = spikeParamsXml->getStringAttribute("stream_name", "");
        int stream_source = spikeParamsXml->getIntAttribute("stream_source", 0);

        SpikeChannel::Type type = SpikeChannel::typeFromNumChannels(spikeParamsXml->getIntAttribute("num_channels", 1));

        if (!processor->alreadyLoaded(name, type, stream_source, stream_name))
        {
            uint16 streamId = processor->findSimilarStream(stream_source, stream_name, sample_rate, true);

            SpikeChannel* spikeChannel = processor->addSpikeChannel(type, streamId, -1, name);

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
    }
    return true;
}
