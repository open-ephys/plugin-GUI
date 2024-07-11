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

#ifndef ChannelMapActions_h
#define ChannelMapActions_h

#include <ProcessorHeaders.h>

#include "ChannelMap.h"

class MapChannelsAction : public ProcessorAction
{
public:
    /** Constructor*/
    MapChannelsAction (ChannelMap* processor,
                       DataStream* stream,
                       Array<int> nextChannelOrder);

    /** Destructor */
    ~MapChannelsAction();

    void restoreOwner (GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform() override;

    /** Undo the action*/
    bool undo() override;

    XmlElement* settings;

private:
    ChannelMap* processor;
    String streamKey;
    Array<int> prevChannelOrder;
    Array<int> nextChannelOrder;
};

class EnableChannelAction : public ProcessorAction
{
public:
    /** Constructor*/
    EnableChannelAction (ChannelMap* processor,
                         DataStream* stream,
                         int channel,
                         bool wasEnabled);

    /** Destructor */
    ~EnableChannelAction();

    void restoreOwner (GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform() override;

    /** Undo the action*/
    bool undo() override;

    XmlElement* settings;

private:
    ChannelMap* processor;
    String streamKey;
    int channel;
    bool wasEnabled;
};

class ResetStreamAction : public ProcessorAction
{
public:
    /** Constructor*/
    ResetStreamAction (ChannelMap* processor,
                       DataStream* stream);

    /** Destructor */
    ~ResetStreamAction();

    void restoreOwner (GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform() override;

    /** Undo the action*/
    bool undo() override;

    XmlElement* settings;

private:
    ChannelMap* processor;
    String streamKey;

    Array<int> channelOrder;
    Array<bool> channelStates;
};

#endif /* ChannelMapActions_h */
