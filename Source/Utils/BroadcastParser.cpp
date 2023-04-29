//
//  BroadcastParser.cpp
//  open-ephys-GUI
//
//  Created by Allen Munk on 4/28/23.
//

#include "BroadcastParser.h"

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


bool BroadcastParser::getIntField(DynamicObject::Ptr payload, String name, int& value, int lowerBound, int upperBound) {
    if(!payload->hasProperty(name) || !payload->getProperty(name).isInt())
        return false;
    int tempVal = payload->getProperty(name);
    if((upperBound != INT_MIN && tempVal > upperBound) || (lowerBound != INT_MAX && tempVal < lowerBound))
        return false;
    value = tempVal;
    return true;
}



bool BroadcastParser::checkForCommand(String expectedPlugin, String expectedCommand, String msg, var& payload) {
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
    payload = jsonMessage -> getProperty("payload");
    return true;
}
