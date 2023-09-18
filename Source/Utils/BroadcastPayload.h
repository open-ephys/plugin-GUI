//
//  BroadcastPayload.h
//  open-ephys-GUI
//
//  Created by Allen Munk on 7/10/23.
//

#ifndef BroadcastPayload_h
#define BroadcastPayload_h

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include <map>
#include <climits>
class PLUGIN_API BroadcastPayload {
public:
    BroadcastPayload();

    BroadcastPayload(const String& command, DynamicObject::Ptr payload);

    String getCommandName() const;

    bool getIntField(const String& name, int& value, int lowerBound = INT_MAX, int upperBound = INT_MIN);

    const DynamicObject::Ptr getPayload() const;


private:
    String _commandName;
    DynamicObject::Ptr _payload;
};


#endif