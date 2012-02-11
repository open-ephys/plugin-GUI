/*
  ==============================================================================

    NetworkThread.cpp
    Created: 9 Jun 2011 2:08:29pm
    Author:  jsiegle

  ==============================================================================
*/

#include "NetworkThread.h"

NetworkThread::NetworkThread() : DataThread()
{
	char host[] = "10.121.43.47";
	char port[] = "5227";

	my_netcomdat = my_netcom.initUdpRx(host, port);
		
	dataBuffer = new DataBuffer(8, 4096);

	startThread();

	std::cout << "Network interface created." << std::endl;
}

NetworkThread::~NetworkThread() {
	stopThread(500);
	close(my_netcomdat.sockfd);
	
	// need to close socket in order to reopen
	close(my_netcomdat.sockfd);

	std::cout << "Network interface destroyed." << std::endl;

	delete dataBuffer;
	dataBuffer = 0;
}


void NetworkThread::updateBuffer(){
		
	 NetCom::rxWave (my_netcomdat, &lfp);

	 for (int s = 0; s < lfp.n_samps_per_chan; s++) {
	 	for (int c = 0; c < lfp.n_chans; c++) {
	 		thisSample[c] = float(lfp.data[s*lfp.n_chans + c])/500.0f;
	 	}
	 	dataBuffer->addToBuffer(thisSample,1);
	 }
}