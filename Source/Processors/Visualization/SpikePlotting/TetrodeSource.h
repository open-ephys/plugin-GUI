// #ifndef TETRODE_SOURCE_H_
// #define TETRODE_SOURCE_H_

// #include <iostream>
// #include <queue>

// #include <pthread.h>
// #include <stdio.h>
// #include <iostream>
// #include <stdlib.h>
// #include <assert.h>
// #include <pthread.h>

// //#include "netcom.h"
// #include "../SpikeObject.h"


// class TetrodeSource{
	
// 	char host[128];// = "127.0.0.1";
// 	char * port;
// 	NetComDat net; // = NetCom::initUdpRx(host,port);
// 	static int const MAX_SPIKE_BUFF_SIZE = 4000;
// 	std::queue<spike_net_t> spikebuffer;
// 	uint32_t nSpikesRead;
// 	pthread_mutex_t mtx;
// 	spike_net_t tmpSpike;
// 	bool readingPackets;


// public:
// 	double padLeft, padRight, padTop, padBottom;
	
// 	TetrodeSource();
// 	TetrodeSource(char *p);
	
// 	bool setPort(char *p);
// 	void enableSource();
// 	void disableSource();
// 	void getNetworkSpikePackets();
// 	bool getNextSpike(spike_net_t * spike);	
	
// 	uint32_t getNSpikesRead();
// 	int getBufferSize();
// 	void flush();	
// };


// void* networkThreadFunc(void* arg);

// #endif
