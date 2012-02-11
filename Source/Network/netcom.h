#ifndef NETCOM_H
#define NETCOM_H

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "datapacket.h"
#include "unp.h"

struct NetComDat{
	int sockfd;
	struct sockaddr_in addr_in;
	struct sockaddr_storage their_addr;
};

class NetCom{
  public:
	static NetComDat initUdpTx(char host[], char port[]);
	static NetComDat initUdpRx(char host[], char port[]);
	
	static int txTs(NetComDat net, timestamp_t count, int nTx);
	static timestamp_t rxTs(NetComDat net);

	static int txSpike(NetComDat net, spike_t, int nTx);
	static spike_t rxSpike(NetComDat net);

	static int txWave(NetComDat net, wave_t w, int nTx);
	static void rxWave(NetComDat net, lfp_bank_net_t *lfp);

	static void txBuff(NetComDat net, char * buff, int buff_len);
	static void rxBuff(NetComDat net, char * buff, int *buff_len);

};

void *get_in_addr(struct sockaddr *sa);


#endif

