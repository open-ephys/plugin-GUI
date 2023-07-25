//
//  BroadcastParser.hpp
//  open-ephys-GUI
//
//  Created by Allen Munk on 4/28/23.
//

#ifndef BroadcastParser_h
#define BroadcastParser_h

#include <stdio.h>

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "BroadcastPayload.h"


class PLUGIN_API BroadcastParser {
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
    static String build(String destPlugin, String command, std::map<String, var> payload);
    
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
    static bool getPayloadForCommand(String expectedPlugin, String expectedCommand, String msg, BroadcastPayload& payload);
    

private:
  

    
    
};

#endif /* BroadcastParser_h */
