
// #include "TetrodeSource.h"
// TetrodeSource::TetrodeSource():
// 					nSpikesRead(0),
// 					readingPackets(false){
// 	port = NULL;
// }
// TetrodeSource::TetrodeSource(char *p):
// 					nSpikesRead(0),
// 					readingPackets(false){
// 	port = p;
// }

// bool TetrodeSource::setPort(char *p){
// 	if (readingPackets)
// 		return false;
// 	else
// 		port = p;
// 	return true;
// }	
// bool TetrodeSource::getNextSpike(spike_net_t *spike){
// //	printf("size of spikebuffer:%d\n", (int)spikebuffer.size());
// 	pthread_mutex_lock(&mtx);
// 	if ( spikebuffer.empty()){
// //		std::cout<<"TetrodeSource::getNextSpike(), spikebuffer is empty"<<std::endl;
// 		pthread_mutex_unlock(&mtx);
// 		return false;
// 	}

// 	else{
// 		spike_net_t s;
// 		s = spikebuffer.front();
// 		*spike = s;
// 		spikebuffer.pop();
// //		std::cout<<"TetrodeSource::getNextSpike() popped spike from the buffer"<<std::endl;
// 		pthread_mutex_unlock(&mtx);
// 		return true;
// 	}		
// }

// void TetrodeSource::enableSource(){
// 	pthread_mutex_lock(&mtx);
// 	spikebuffer = std::queue<spike_net_t>();
// 	pthread_mutex_unlock(&mtx);

// 	if (port==NULL)
// 	{
// 		std::cout<<"TetrodeSource::enableSource(), No port specified, it is currently NULL"<<std::endl;
// 	}
// 	std::cout<<"Initializing a new TetrodeSource on port:"<<port<<"\n"<<std::flush;

// 	pthread_t netThread;
// 	net = NetCom::initUdpRx(host,port);
// 	pthread_create(&netThread, NULL, networkThreadFunc, this);
// }
// void TetrodeSource::disableSource(){
// 	readingPackets = false;	
// }

// void TetrodeSource::getNetworkSpikePackets(){
// 	readingPackets = true;
// 	std::cout<<"TetrodeSource::getNetworkSpikePackets starting!"<<std::endl;
// 	while(readingPackets){

// 		spike_net_t newSpike;
// 		NetCom::rxSpike(net, &newSpike);
// 		pthread_mutex_lock(&mtx);

// /*		
// 		while (spikebuffer.size()>MAX_SPIKE_BUFF_SIZE){
// 			std::cout<<"Too many spikes in the buffer, removing some. Spikes in buffer:"<<spikebuffer.size()<<std::endl;
// 			spikebuffer.pop();
// 		}
// 		*/
// 		spikebuffer.push(newSpike);
// 		nSpikesRead++;
// //		printf("Read %d spikes, %d in buffer\n", nSpikesRead, spikebuffer.size());
// //		printf("\t New Spike timestamp:%d\n",newSpike.ts);
// 		pthread_mutex_unlock(&mtx);
		
// 		if (nSpikesRead%100==0)
// 			std::cout<<"TetrodeSource::getNetworkSpikePackets() read a total of:"<<nSpikesRead<<" spikes"<<std::endl;
// 	}
// 	std::cout<<"TetrodeSource::getNetworkSpikePackets ending!"<<std::endl;
// }

// int TetrodeSource::getBufferSize(){
// 	pthread_mutex_lock(&mtx);
// 	return spikebuffer.size();
// 	pthread_mutex_unlock(&mtx);
// }
// void TetrodeSource::flush(){
// 	pthread_mutex_lock(&mtx);
// 	std::queue<spike_net_t> empty;
// 	spikebuffer = empty;
// 	pthread_mutex_unlock(&mtx);
	
// }
// void *networkThreadFunc(void *ptr){
// 	TetrodeSource *tp = (TetrodeSource*) ptr;
// 	tp->getNetworkSpikePackets();
// }



