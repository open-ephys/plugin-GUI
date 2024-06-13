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