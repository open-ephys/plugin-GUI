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


class PLUGIN_API BroadcastParser {
public:

    static String build(String destPlugin, String command, std::map<String, var> payload);
    

    static bool checkForCommand(String expectedPlugin, String expectedCommand, String msg, var& payload);
    
    static bool getIntField(DynamicObject::Ptr payload, String name, int& value, int lowerBound = INT_MAX, int upperBound = INT_MIN);


private:
  

    
    
};

#endif /* BroadcastParser_h */
