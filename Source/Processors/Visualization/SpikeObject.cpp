/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SpikeObject.h"
#include <iostream>
#include "memory.h"
#include <stdlib.h>



// Simple method for serializing a SpikeObject into a string of bytes
bool packSpike(SpikeObject *s, char* buffer, int bufferSize){

	int idx = 0;
	memcpy(buffer+idx, &(s->timestamp), 4);
	idx += 4;

	memcpy(buffer+idx, &(s->source), 2);
	idx +=2;

	memcpy(buffer+idx, &(s->nChannels), 2);
	idx +=2;

	memcpy(buffer+idx, &(s->nSamples), 2);
	idx +=2;

	memcpy(buffer+idx, &(s->data), s->nChannels * s->nSamples * 2);
	idx += s->nChannels * s->nSamples * 2;

	memcpy(buffer+idx, &(s->gain), s->nChannels);
	idx += s->nChannels;

	memcpy(buffer+idx, &(s->threshold), s->nChannels);
	idx += s->nChannels;

if (idx>=bufferSize)
		std::cout<<"Buffer Overrun! More data packaged than space provided!"<<std::endl;
	// makeBufferValid(buffer, bufferSize);

}

// Simple method for deserializing a string of bytes into a Spike object
bool unpackSpike(SpikeObject *s, char* buffer, int bufferSize){
	// if !(isBufferValid(buffer, bufferSize));
	// 	return false;

	int idx = 0;
	
	memcpy( &(s->timestamp), buffer+idx, 4);
	idx += 4;

	memcpy( &(s->source), buffer+idx, 2);
	idx += 2;

	memcpy( &(s->nChannels), buffer+idx, 2);
	idx +=2;

	memcpy( &(s->nSamples), buffer+idx, 2);
	idx +=2;

	memcpy( &(s->data), buffer+idx, s->nChannels * s->nSamples * 2);
	idx += s->nChannels * s->nSamples * 2;

	memcpy( &(s->gain), buffer+idx, s->nChannels * 2);
	idx += s->nChannels * 2;

	memcpy( &(s->threshold), buffer+idx, s->nChannels *2);
	idx += s->nChannels * 2;

	if (idx>=bufferSize)
		std::cout<<"Buffer Overrun! More data extracted than was given!"<<std::endl;



}

// Checks the validity of the buffer, this should be run before unpacking and after packing the buffer
bool isBufferValid(char *buffer, int bufferSize){

	if (! CHECK_BUFFER_VALIDITY )
		return true;

	uint16_t runningSum = 0;
	uint16_t value = 0;
	
	int idx = 0;

	for (idx; idx<bufferSize-2; idx+=2){
		memcpy(buffer + idx, &value, 2);
		runningSum += value;
	}

	uint16_t integrityCheck = 0;
	memcpy(buffer + idx, &integrityCheck, 2);

	std::cout<<integrityCheck<<" == "<< runningSum <<std::endl;
	return (integrityCheck == runningSum);
}

void makeBufferValid(char *buffer, int bufferSize){
	if (! CHECK_BUFFER_VALIDITY )
		return;

	uint16_t runningSum = 0;
	uint16_t value = 0;
	
	int idx = 0;

	for (idx; idx<bufferSize-2; idx+=2){
		memcpy(buffer + idx, &value, 2);
		runningSum += value;
	}

	memcpy(&runningSum, buffer + idx, 2);

}

SpikeObject generateSimulatedSpike(uint64_t timestamp, int noise, bool sineWave)
{

	uint16_t realSpikeWave[32] =
	{ 1880,	 	1900,	1940,	2040,	2290,	2790,	3475,	3995, 	4110, 	3890,
      3505,		3090,	2720,	2410, 	2155,  	1945,	1775,	1635,	1520, 	1420,
      1340,		1265,	1205,	1155,	1115,	1080,	1050,	1034,	1010, 	1001,
      1000, 	1000};

    uint16_t sineSpikeWave[32] = 
    {	78, 	90, 	101, 	111,	120,	126,	129,	130,
		129,	126,	120,	111,	101,	90,		78,		65,
		52,		40,		29,		19,		11,		5,		2,		1,
		2,		5,		11,		19,		29,		40,		52,		65};

    uint16_t *trace;

    uint16_t gain;


    if(sineWave){
    	trace = sineSpikeWave;
    	gain = 100;
    }
    else{
    	trace = realSpikeWave;
    	gain = 5;
    }


    SpikeObject s;
    s.timestamp = timestamp;
    s.source = 0;
    s.nChannels = 4;
    s.nSamples = 32;

    int idx=0;
    for (int i=0; i<s.nSamples; i++)
    {
            s.gain[i] = gain;
            s.threshold[i] = 12000;

			for (int j=0; j<s.nChannels; j++){
				
				int n = 0;
				if (noise>0){
					n = rand() % noise - noise/2;
				}

                s.data[idx++] = (trace[i]*gain) + n;
            }
    }

    return s;
}


// std::ostream& operator<<(std::ostream &strm, const SpikeObject s){

// 	strm << " SpikeObject:\n";
// 	strm << "\tTimestamp:" << s.timestamp;
// 	strm << "\tSource:" << s.source;
// 	strm << "\tnChannels:" <<s.nChannels;
// 	strm <<"\tnSamples" << s.nSamples;
// 	strm <<"\n\t 8 Data Samples:";
// 	for (int i=0; i<8; i++)
// 		strm << s.data[i]<<" ";
// 	return strm;
// }
