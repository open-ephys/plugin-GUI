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

#ifndef BroadcastPayload_h
#define BroadcastPayload_h

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include <climits>
#include <map>
class PLUGIN_API BroadcastPayload
{
public:
    BroadcastPayload();

    BroadcastPayload (String command, DynamicObject::Ptr payload);

    String getCommandName() const;

    bool getIntField (String name, int& value, int lowerBound = INT_MAX, int upperBound = INT_MIN);

    const DynamicObject::Ptr getPayload() const;

private:
    String _commandName;
    DynamicObject::Ptr _payload;
};

#endif