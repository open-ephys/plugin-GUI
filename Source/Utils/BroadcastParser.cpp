//
//  BroadcastParser.cpp
//  open-ephys-GUI
//
//  Created by Allen Munk on 4/28/23.
//

#include "BroadcastParser.h"
#include "../Processors/GenericProcessor/GenericProcessor.h"

String BroadcastParser::build(String destPlugin, String command, std::map<String, var> payload) {
    DynamicObject::Ptr jsonObj = new DynamicObject();
    DynamicObject::Ptr payloadObj = new DynamicObject();
    for(auto val: payload) {
        payloadObj->setProperty(val.first, val.second);
    }
    jsonObj->setProperty("plugin", destPlugin);
    jsonObj->setProperty("command", command);
    jsonObj->setProperty("payload", var(payloadObj));
    String returnString = JSON::toString(var(jsonObj), true);
    payloadObj.reset();
    jsonObj.reset();
    return returnString;
}






bool BroadcastParser::getPayloadForCommand(String expectedPlugin, String expectedCommand, String msg, BroadcastPayload& payload) {
    var parsedMessage = JSON::parse(msg);
    if(!parsedMessage.isObject()) {
        return false;
    }
    DynamicObject::Ptr jsonMessage = parsedMessage.getDynamicObject();
    if(jsonMessage == nullptr) {
        return false;
    }
    String pluginName= jsonMessage -> getProperty("plugin");
    if(pluginName != "" && pluginName != expectedPlugin) {
        return false;
    }
    String command = jsonMessage -> getProperty("command");
    if(command != expectedCommand) {
        return false;
    }
    var tempPayload = jsonMessage -> getProperty("payload");
    if(tempPayload.getDynamicObject() == nullptr){
        return false;
    }
    payload = BroadcastPayload(command,  jsonMessage -> getProperty("payload").getDynamicObject());
    return true;
}
