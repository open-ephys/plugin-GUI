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

#ifndef BroadcastParser_h
#define BroadcastParser_h

#include <stdio.h>

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "BroadcastPayload.h"

class PLUGIN_API BroadcastParser
{
public:
    /**
    * Builds a message String that can be passed to other Processors using GenericProcessor::broadcastMessage() or 
    * GenericProcessor::sendConfigMessage(). This message String can then be parsed by BroadcastParser::getPayloadForCommand
    * 
    * @param destPlugin String name of the Plugin the message is intended
    * @param command String name of the command this message is building
    * @param payload Map of parameters and their values to be sent to the destination Plugin
    * @returns The constructed message String. This string formats the parameters as a flat JSON string
    */
    static String build (String destPlugin, String command, std::map<String, var> payload);

    /**
    * Deconstructs a message String. Checks the message for an expected destination plugin and an expected command. If
    * the message String's destination and command match the expected parameters then a payload can be extracted.
    * 
    * @param expectedPlugin String name of the destination plugin that should be parsing the message
    * @param expectedCommand String name of the command that this message should contain 
    * @param msg String message that is being parsed
    * @param payload Reference var object that will be loaded with the message contains the expected parameters
    * match the message string
    * @returns if true then the expected parameters match and the payload is valid
    */
    static bool getPayloadForCommand (String expectedPlugin, String expectedCommand, String msg, BroadcastPayload& payload);

private:
};

#endif /* BroadcastParser_h */
