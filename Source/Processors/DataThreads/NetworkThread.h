/*
  ==============================================================================

    NetworkThread.h
    Created: 9 Jun 2011 2:08:29pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __NETWORKTHREAD_H_DD31EB15__
#define __NETWORKTHREAD_H_DD31EB15__

#include <stdio.h>

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../../Network/unp.h"
#include "../../Network/netcom.h"
#include "../../Network/datapacket.h"

#include "DataThread.h"

class NetworkThread : public DataThread
{
public:

	NetworkThread();
	~NetworkThread();

private:

	NetCom my_netcom;
	NetComDat my_netcomdat;

	lfp_bank_net_t lfp;

	DataBuffer* dataBuffer;

	float thisSample[8];

	void updateBuffer();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkThread);

};



#endif  // __NETWORKTHREAD_H_DD31EB15__
