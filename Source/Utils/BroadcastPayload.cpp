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

#include "BroadcastPayload.h"


BroadcastPayload::BroadcastPayload() : _commandName ("") {}

BroadcastPayload::BroadcastPayload(String command, DynamicObject::Ptr payload) : _commandName(command), _payload(payload) {}

String BroadcastPayload::getCommandName() const
{
    return _commandName;
}

bool BroadcastPayload::getIntField (String name, int& value, int lowerBound, int upperBound)
{
    if (! _payload->hasProperty (name) || ! _payload->getProperty (name).isInt())
        return false;
    int tempVal = _payload->getProperty (name);
    if ((upperBound != INT_MIN && tempVal > upperBound) || (lowerBound != INT_MAX && tempVal < lowerBound))
        return false;
    value = tempVal;
    return true;
}

const DynamicObject::Ptr BroadcastPayload::getPayload() const
{
    return _payload;
}