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

#include <stdio.h>

#include "ChannelMapActions.h"

MapChannelsAction::MapChannelsAction (ChannelMap* processor_,
                                      DataStream* stream,
                                      Array<int> newMap) : ProcessorAction ("MapChannels"),
                                                           processor (processor_),
                                                           streamKey (stream->getKey()),
                                                           prevChannelOrder (processor->getChannelOrder (stream->getStreamId())),
                                                           nextChannelOrder (newMap)
{
    settings = nullptr;
}

MapChannelsAction::~MapChannelsAction()
{
    if (settings != nullptr)
        delete settings;
}

bool MapChannelsAction::perform()
{
    uint16 streamId = 0;
    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0)
        return false;

    processor->setChannelOrder (streamId, nextChannelOrder);

    processor->registerUndoableAction (processor->getNodeId(), this);

    CoreServices::updateSignalChain (processor);

    return true;
}

bool MapChannelsAction::undo()
{
    uint16 streamId = 0;
    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0)
        return false;

    processor->setChannelOrder (streamId, prevChannelOrder);

    processor->registerUndoableAction (processor->getNodeId(), this);

    CoreServices::updateSignalChain (processor);

    return true;
}

void MapChannelsAction::restoreOwner (GenericProcessor* owner)
{
    processor = (ChannelMap*) owner;
}

EnableChannelAction::EnableChannelAction (ChannelMap* processor_,
                                          DataStream* stream,
                                          int channel_,
                                          bool wasEnabled_) : ProcessorAction ("EnableChannel"),
                                                              processor (processor_),
                                                              streamKey (stream->getKey()),
                                                              channel (channel_),
                                                              wasEnabled (wasEnabled_)
{
    settings = nullptr;
}

EnableChannelAction::~EnableChannelAction()
{
    if (settings != nullptr)
        delete settings;
}

bool EnableChannelAction::perform()
{
    uint16 streamId = 0;
    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0)
        return false;

    processor->setChannelEnabled (streamId, channel, ! wasEnabled);

    processor->registerUndoableAction (processor->getNodeId(), this);

    CoreServices::updateSignalChain (processor);

    return true;
}

bool EnableChannelAction::undo()
{
    uint16 streamId = 0;
    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0)
        return false;

    processor->setChannelEnabled (streamId, channel, wasEnabled);

    processor->registerUndoableAction (processor->getNodeId(), this);

    CoreServices::updateSignalChain (processor);

    return true;
}

void EnableChannelAction::restoreOwner (GenericProcessor* owner)
{
    processor = (ChannelMap*) owner;
}

ResetStreamAction::ResetStreamAction (ChannelMap* processor_,
                                      DataStream* stream) : ProcessorAction ("ResetStream"),
                                                            processor (processor_),
                                                            streamKey (stream->getKey()),
                                                            channelOrder (processor->getChannelOrder (stream->getStreamId())),
                                                            channelStates (processor->getChannelEnabledState (stream->getStreamId()))
{
    settings = nullptr;
}

ResetStreamAction::~ResetStreamAction()
{
    if (settings != nullptr)
        delete settings;
}

bool ResetStreamAction::perform()
{
    uint16 streamId = 0;
    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }
    if (streamId == 0)
        return false;

    processor->resetStream (streamId);

    processor->registerUndoableAction (processor->getNodeId(), this);

    CoreServices::updateSignalChain (processor);

    return true;
}

bool ResetStreamAction::undo()
{
    uint16 streamId = 0;

    for (auto stream : processor->getDataStreams())
    {
        if (stream->getKey() == streamKey)
        {
            streamId = stream->getStreamId();
            break;
        }
    }

    if (streamId == 0)
        return false;

    processor->setChannelOrder (streamId, channelOrder);

    for (int i = 0; i < channelStates.size(); i++)
    {
        processor->setChannelEnabled (streamId, i, channelStates[i]);
    }

    CoreServices::updateSignalChain (processor);

    return true;
}

void ResetStreamAction::restoreOwner (GenericProcessor* owner)
{
    processor = (ChannelMap*) owner;
}