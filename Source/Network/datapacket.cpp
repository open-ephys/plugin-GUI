#include <iostream>
#include "datapacket.h"
#include <stdio.h>
#include <string.h>

void printBuff(char* buff, int blen){
        char val;
        for (int i=0; i<blen; i++){
                val = *(buff+i);
                std::cout<<"\\"<<(int)val;           
        }
	std::cout<<std::endl;
}


/*------------- TIME ------------*/
void tsToBuff(timestamp_t* t, char* buff, int blen){
//	if (blen<6)
//		std::cout<<"ERROR: Buffer is too short"<<std::endl;
	timestamp_t ts = hton32(*t);	
	memcpy(buff+2, &ts, 4);
}

timestamp_t buffToTs(char *buff, int blen){

//	if (blen<6)
//		std::cout<<"Error buffer is too short"<<std::endl;

	timestamp_t s;
	memcpy(&s, buff+2, 4);	
	s = ntoh32(s);
	return s;
}
/*------------- SPIKE ------------*/
void spikeToBuff(spike_net_t* s, char* buff, int *buff_len){
  //std::cout<<"spikeToBuff(), isn't checking the buffer size, implement this!"<<std::endl;

  buff[0] = typeToChar(NETCOM_UDP_SPIKE); 
  buff[3] = 0; 
  
  uint32_t ts_tx = hton32(s->ts);
  uint16_t name_tx = hton16(s->name);
  uint16_t n_chans_tx = hton16(s->n_chans);
  uint16_t n_samps_per_chan_tx = hton16(s->n_samps_per_chan);
  uint16_t samp_n_bytes_tx = hton16(s->samp_n_bytes);
  rdata_t  data_tx[MAX_FILTERED_BUFFER_TOTAL_SAMPLE_COUNT];
  int16_t  gains_tx[MAX_FILTERED_BUFFER_N_CHANS];
  rdata_t  thresh_tx[MAX_FILTERED_BUFFER_N_CHANS];
  uint16_t trig_ind_tx = hton32(s->trig_ind);

  for(int c = 0; c < s->n_chans * s->n_samps_per_chan; c++){
    // ASSUMES 16-bit data!
    data_tx[c] = hton16(s->data[c]);
  }
  for(int c = 0; c < s->n_chans; c++){
    gains_tx[c] = hton16(s->data[c]);
    thresh_tx[c] = hton16(s->thresh[c]);
  }

  uint16_t cursor = 4;  // start offset 4 bytes (first is packet type, 2nd and 3rd are buff_size 4th is \0
  uint16_t cursor_tx;
  
  memcpy(buff+cursor, &ts_tx,               4);
  cursor += 4;
  memcpy(buff+cursor, &name_tx,             2);
  cursor += 2;
  memcpy(buff+cursor, &n_chans_tx,          2);
  cursor += 2;
  memcpy(buff+cursor, &n_samps_per_chan_tx, 2);
  cursor += 2;
  memcpy(buff+cursor, &samp_n_bytes_tx,        2);
  cursor += 2;
  memcpy(buff+cursor, &data_tx,             (s->n_chans * s->n_samps_per_chan * s->samp_n_bytes));
  cursor += (s->n_chans * s->n_samps_per_chan * s->samp_n_bytes);
  memcpy(buff+cursor, &gains_tx,            (2 * s->n_chans));
  cursor += 2*s->n_chans;
  memcpy(buff+cursor, &thresh_tx,           (2 * s->n_chans));
  cursor += 2*s->n_chans;
  memcpy(buff+cursor, &trig_ind_tx,         2);
  cursor += 2;

  cursor_tx = hton16(cursor);
  memcpy(buff+1,      &cursor_tx,           2);

  buff[cursor] = '\0';
  *buff_len = cursor;

}

void buffToSpike( spike_net_t *s, char* buff, int blen){

  int cursor = 4;
  
  memcpy( &(s->ts),        buff+cursor, 4);
  s->ts = ntoh32(s->ts);
  cursor += 4;
  
  memcpy( &(s->name),      buff+cursor, 2);
  s->name = ntoh16(s->name);
  cursor += 2;

  memcpy( &(s->n_chans),   buff+cursor, 2);
  s->n_chans = ntoh16(s->n_chans);
  cursor += 2;

  memcpy( &(s->n_samps_per_chan), buff+cursor, 2);
  s->n_samps_per_chan = ntoh16(s->n_samps_per_chan);
  cursor += 2;

  memcpy( &(s->samp_n_bytes),     buff+cursor, 2);
  s->samp_n_bytes = ntoh16(s->samp_n_bytes);
  cursor += 2;

  int n_total_samps = s->n_chans * s->n_samps_per_chan;
  int data_bytes = n_total_samps * s->samp_n_bytes;
  int gain_bytes = s->n_chans * 2;    // 2 byte datatype for gain: int16
  int thresh_bytes = s->n_chans * 2;  // 2 byte datatype for thresh: rdata_t

  memcpy( &(s->data),           buff+cursor, data_bytes);
  for(int n = 0; n < n_total_samps; n++)
    s->data[n] = ntoh16(s->data[n]);
  cursor += data_bytes;

  memcpy( &(s->gains),         buff+cursor, gain_bytes);
  cursor += gain_bytes;
  memcpy( &(s->thresh),        buff+cursor, thresh_bytes);
  cursor += thresh_bytes;

  for(int n = 0; n < s->n_chans; n++){
    s->gains[n] = ntoh16(s->gains[n]);
    s->thresh[n] = ntoh16(s->thresh[n]);
  }

  memcpy( &(s->trig_ind),      buff+cursor, 2);
  s->trig_ind = ntoh16(s->trig_ind);
  cursor += 2;

}

/*------------- WAVE ------------*/
void waveToBuff(lfp_bank_net_t* lfp, char* buff, int *blen){
  //TODO IMPLEMENT
  //printf("waveToBuff\n");
  
  int s;

  //old
  //int cursor = 2;
  //NEW
  uint16_t cursor = 4;
  uint16_t cursor_tx;

  int16_t name_tx =              hton16(lfp->name);
  uint32_t ts_tx =               htonl(lfp->ts);
  uint16_t n_chans_tx =          hton16(lfp->n_chans);
  uint16_t n_samps_per_chan_tx = hton16(lfp->n_samps_per_chan);
  uint16_t bytes_per_samp_tx =   hton16(lfp->samp_n_bytes);
  rdata_t data_tx[MAX_FILTERED_BUFFER_TOTAL_SAMPLE_COUNT];  // find out the right minimum size for this array
  int16_t gain_tx[MAX_FILTERED_BUFFER_N_CHANS];

  // flip all the elements in data
  // THIS ASSUMES 16 BIT SAMPLES
  for(s = 0; s < lfp->n_chans * lfp->n_samps_per_chan; s++){
    data_tx[s] = hton16(lfp->data[s]);
  }
  for(s = 0; s < lfp->n_chans; s++){
    gain_tx[s] = hton16(lfp->gains[s]);
  }

  buff[0] = typeToChar(NETCOM_UDP_LFP);
  buff[3] = '\0';

  memcpy(buff+cursor, &ts_tx                          , 4);
  cursor += 4;
  memcpy(buff+cursor, &name_tx                        , 2);
  cursor += 2;
  memcpy(buff+cursor, &n_chans_tx                     , 2);
  cursor += 2;
  memcpy(buff+cursor, &n_samps_per_chan_tx            , 2);
  cursor += 2;
  memcpy(buff+cursor, &bytes_per_samp_tx, 2);
  cursor += 2;
  // is this next line right?  &data_tx or data_tx ???
  memcpy(buff+cursor, &data_tx                        , (s=(lfp->n_chans * lfp->n_samps_per_chan * lfp->samp_n_bytes)));
  cursor += s;
  memcpy(buff+cursor, &gain_tx                        , (s=(lfp->n_chans * 2)));
  cursor += s;

  // NEW
  cursor_tx = hton16(cursor);
  memcpy(buff+1,     &cursor_tx, 2);

  buff[cursor] = '\0';
  *blen = cursor;

  if(false){
    printf("incoming ts: %d.  ts after swap: %d",lfp->ts, ts_tx);
  }

  if(false){
    printf("buffer being written: ");
    for(int n = 0; n < *blen; n++)
      printf("%d:%c\n ",n,buff[n]);
  } 
}

void buffToWave(lfp_bank_net_t *lfp, char* buff){
  //TODO IMPLEMENT
  int cursor = 4;  // skip over the packet_type field

  
  //printf("1");
  //fflush(stdout);
  memcpy( &(lfp->ts), buff+cursor, 4);
  lfp->ts = ntoh32(lfp->ts);
  cursor += 4;
  //printf("2");
  //fflush(stdout);
  memcpy( &(lfp->name), buff+cursor,  2);
  lfp->name = ntoh16(lfp->name);
  cursor +=2;
  //printf("3");
  //fflush(stdout);
  memcpy( &(lfp->n_chans), buff+cursor, 2);
  lfp->n_chans = ntoh16(lfp->n_chans);
  cursor +=2;
  //printf("4");
  //fflush(stdout);
  memcpy( &(lfp->n_samps_per_chan), buff+cursor, 2);
  lfp->n_samps_per_chan = ntoh16(lfp->n_samps_per_chan);
  cursor += 2;
  //printf("5");
  //fflush(stdout);
  memcpy( &(lfp->samp_n_bytes), buff+cursor, 2);
  lfp->samp_n_bytes = ntoh16(lfp->samp_n_bytes);
  cursor += 2;
  //printf("6");
  //fflush(stdout);
  int n_total_samps = lfp->n_chans * lfp->n_samps_per_chan;
  int data_bytes = lfp->n_chans * lfp->n_samps_per_chan * lfp->samp_n_bytes;
  int gain_bytes = lfp->n_chans * lfp->samp_n_bytes;
  //printf("7");
  //fflush(stdout);
  memcpy( &(lfp->data), buff+cursor, data_bytes);
  for(int n = 0; n < n_total_samps; n++){
    //printf("about to do data[%d]\n",n);
    //fflush(stdout);
    lfp->data[n] = ntoh16(lfp->data[n]);
  }
  cursor += data_bytes;
  //printf("8");
  //fflush(stdout);
  memcpy( &(lfp->gains), buff+cursor, gain_bytes);
  //printf("finished memcopy for gains\n");
  for(int n = 0; n < lfp->n_chans; n++){
    //printf("about to do gain[%d] \n",n);
    //fflush(stdout);
    lfp->gains[n] = ntoh16(lfp->gains[n]);
  }
  cursor += gain_bytes;
  //printf("9");
  //fflush(stdout);
}
 
