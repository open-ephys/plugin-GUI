//
//  BroadcastPayload.h
//  open-ephys-GUI
//
//  Created by Allen Munk on 7/10/23.
//

#ifndef BroadcastPayload_h
#define BroadcastPayload_h

#include "../../JuceLibraryCode/JuceHeader.h"
#include <map>
#include <climits>
class BroadcastPayload {
public:
	BroadcastPayload(String command, DynamicObject::Ptr payload);

	String getCommandName() const;

	bool getIntField(String name, int& value, int lowerBound = INT_MAX, int upperBound = INT_MIN);


private:
	String _commandName;
	DynamicObject::Ptr _payload;
};


#endif