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


class BroadcastParser {
public:

    static String build(String destPlugin, String command, std::map<String, var> payload);
    

    static bool checkForCommand(String expectedPlugin, String expectedCommand, String msg, var& payload);


private:
    static bool getIntField(DynamicObject::Ptr payload, String name, int& value, int lowerBound, int upperBound);

    
    
};

#endif /* BroadcastParser_h */
