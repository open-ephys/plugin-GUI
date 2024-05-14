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

#include "BroadcastParser.h"
#include "../Processors/GenericProcessor/GenericProcessor.h"

String BroadcastParser::build (String destPlugin, String command, std::map<String, var> payload)
{
    DynamicObject::Ptr jsonObj = new DynamicObject();
    DynamicObject::Ptr payloadObj = new DynamicObject();
    for (auto val : payload)
    {
        payloadObj->setProperty (val.first, val.second);
    }
    jsonObj->setProperty ("plugin", destPlugin);
    jsonObj->setProperty ("command", command);
    jsonObj->setProperty ("payload", var (payloadObj));
    String returnString = JSON::toString (var (jsonObj), true);
    payloadObj.reset();
    jsonObj.reset();
    return returnString;
}

bool BroadcastParser::getPayloadForCommand (String expectedPlugin, String expectedCommand, String msg, BroadcastPayload& payload)
{
    var parsedMessage = JSON::parse (msg);
    if (! parsedMessage.isObject())
    {
        return false;
    }
    DynamicObject::Ptr jsonMessage = parsedMessage.getDynamicObject();
    if (jsonMessage == nullptr)
    {
        return false;
    }
    String pluginName = jsonMessage->getProperty ("plugin");
    if (pluginName != "" && pluginName != expectedPlugin)
    {
        return false;
    }
    String command = jsonMessage->getProperty ("command");
    if (command != expectedCommand)
    {
        return false;
    }
    var tempPayload = jsonMessage->getProperty ("payload");
    if (tempPayload.getDynamicObject() == nullptr)
    {
        return false;
    }
    payload = BroadcastPayload (command, jsonMessage->getProperty ("payload").getDynamicObject());
    return true;
}
