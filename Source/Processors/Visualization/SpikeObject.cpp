/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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
#include "memory.h"
#include <stdlib.h>
#include "time.h"

#define MIN(a,b)((a)<(b)?(a):(b))
#define MAX(a,b)((a)<(b)?(b):(a))

// Simple method for serializing a SpikeObject into a string of bytes
int packSpike(const SpikeObject* s, uint8_t* buffer, int bufferSize)
{

    // This method should receive a SpikeObject filled with data,
    // a pointer to a uint8_t buffer (which will hold the serialized SpikeObject,
    // and a integer indicating the bufferSize.

    //int reqBytes = 1 + 4 + 2+2 + 2 + 2 + 2 * s->nChannels * s->nSamples + 2 * s->nChannels * 2;

    int idx = 0;

    memcpy(buffer+idx, &(s->eventType), 1);
    idx += 1;

    memcpy(buffer+idx, &(s->timestamp), 8);
    idx += 8;
    
    memcpy(buffer+idx, &(s->timestamp_software), 8);
    idx += 8;

    memcpy(buffer+idx, &(s->source), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->nChannels), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->nSamples), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->sortedId), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->electrodeID), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->channel), 2);
    idx +=2;

    memcpy(buffer+idx, &(s->color[0]), 1);
    idx +=1;
    memcpy(buffer+idx, &(s->color[1]), 1);
    idx +=1;
    memcpy(buffer+idx, &(s->color[2]), 1);
    idx +=1;

    memcpy(buffer+idx, &(s->pcProj[0]), sizeof(float));
    idx +=sizeof(float);

    memcpy(buffer+idx, &(s->pcProj[1]), sizeof(float));
    idx +=sizeof(float);

    memcpy(buffer+idx, &(s->samplingFrequencyHz), 2);
    idx +=2;
    memcpy(buffer+idx, &(s->data), s->nChannels * s->nSamples * 2);
    idx += s->nChannels * s->nSamples * 2;

    memcpy(buffer+idx, &(s->gain), s->nChannels * 4); // 4 bytes for a float
    idx += s->nChannels * 4;

    memcpy(buffer+idx, &(s->threshold), s->nChannels * 2);
    idx += s->nChannels * 2;

    if (idx >= MAX_SPIKE_BUFFER_LEN)
    {
        std::cout << "Spike is larger than it should be. Size was: " << idx
                  << " Max size is: " << MAX_SPIKE_BUFFER_LEN << std::endl;

    }

    //makeBufferValid(buffer, idx);

    return idx;

}

// Simple method for deserializing a string of bytes into a Spike object
bool unpackSpike(SpikeObject* s, const uint8_t* buffer, int bufferSize)
{
    //if (!isBufferValid(buffer, bufferSize))
    //  return false;

    int idx = 0;

    memcpy(&(s->eventType), buffer+idx, 1);
    idx += 1;

   // if (s->eventType != 4)
   // {
   //     std::cout << "received invalid spike -- incorrect event code" << std::endl;
   //     return false;
   // }

    memcpy(&(s->timestamp), buffer+idx, 8);
    idx += 8;

    memcpy(&(s->timestamp_software), buffer+idx, 8);
    idx += 8;

    memcpy(&(s->source), buffer+idx, 2);
    idx += 2;

    if (s->source < 0 || s->source > 100)
    {
        std::cout << "received invalid spike -- incorrect source" << std::endl;
        return false;
    }

    memcpy(&(s->nChannels), buffer+idx, 2);
    idx +=2;

    if (s->nChannels > 4)
    {
        std::cout << "received invalid spike -- incorrect number of channels" << std::endl;
        return false;
    }

    memcpy(&(s->nSamples), buffer+idx, 2);
    idx +=2;

    if (s->nSamples > 100)
    {
        std::cout << "received invalid spike -- incorrect number of samples" << std::endl;
        return false;
    }


    
    memcpy(&(s->sortedId), buffer+idx, 2);
    idx +=2;

        memcpy(&(s->electrodeID), buffer+idx, 2);
    idx +=2;

        memcpy(&(s->channel), buffer+idx, 2);
    idx +=2;

  memcpy(&(s->color[0]), buffer+idx, 1);
    idx +=1;
  memcpy(&(s->color[1]), buffer+idx, 1);
    idx +=1;
  memcpy(&(s->color[2]), buffer+idx, 1);
    idx +=1;

    

     memcpy(&(s->pcProj[0]), buffer+idx, sizeof(float));
    idx +=sizeof(float);
     memcpy(&(s->pcProj[1]), buffer+idx, sizeof(float));
    idx +=sizeof(float);

    memcpy(&(s->samplingFrequencyHz), buffer+idx, 2);
    idx +=2;


    memcpy(&(s->data), buffer+idx, s->nChannels * s->nSamples * 2);
    idx += s->nChannels * s->nSamples * 2;

    memcpy(&(s->gain), buffer+idx, s->nChannels * 4);
    idx += s->nChannels * 4;

    memcpy(&(s->threshold), buffer+idx, s->nChannels *2);
    idx += s->nChannels * 2;

    //if (idx >= bufferSize)
    //    std::cout << "Buffer Overrun! More data extracted than was given!" << std::endl;

    return true;


}

// Checks the validity of the buffer, this should be run before unpacking and after packing the buffer
bool isBufferValid(const uint8_t* buffer, int bufferSize)
{

    if (!CHECK_BUFFER_VALIDITY)
        return true;

    uint16_t runningSum = 0;
    uint16_t value = 0;

    int idx;

    for (idx = 0; idx < bufferSize - 2; idx += 2)
    {
        memcpy(&value, buffer + idx, 2);
        runningSum += value;
    }

    uint16_t integrityCheck = 0;
    memcpy(&integrityCheck, buffer + idx + 2, 2);

    std::cout << integrityCheck << " == " << runningSum << std::endl;

    return (integrityCheck == runningSum);
}

void makeBufferValid(uint8_t* buffer, int bufferSize)
{
    if (!CHECK_BUFFER_VALIDITY)
        return;

    uint16_t runningSum = 0;
    uint16_t value = 0;

    int idx;

    for (idx = 0; idx < bufferSize; idx += 2)
    {
        memcpy(&value, buffer + idx, 2);
        runningSum += value;
    }

    // put the check at the end of the buffer
    memcpy(buffer + MAX_SPIKE_BUFFER_LEN - 2, &runningSum, 2);

}

void generateSimulatedSpike(SpikeObject* s, uint64_t timestamp, int noise)
{
    //std::cout<<"generateSimulatedSpike()"<<std::endl;

    uint16_t trace[][32] =
    {
        {
            880,    900,    940,    1040,   1290,   1790,   2475,   2995,   3110,   2890,
            2505,   2090,   1720,   1410,   1155,   945,    775,    635,    520,    420,
            340,    265,    205,    155,    115,    80,     50,     34,     10,     34,     50,     80
        },
        {
            1040,   1090,   1190,   1350,   1600,   1960,   2380,   2790,   3080,   3140,
            2910,   2430,   1810,   1180,   680,    380,    270,    320,    460,    630,
            770,    870,    940,    970,    990,    1000,   1000,   1000,   1000,   1000,  1000,    1000
        },
        {
            1000,   1000,   1000,   1000,   1000,   1040,   1140,   1440,   2040,   2240,
            2400,   2340,   2280,   1880,   1640,   920,    520,    300,    140,    040,
            20,     20,     40,     100,    260,    500,    740,    900,    960,    1000,   1000,   1000
        }
    };


    // We don't want to shift the waveform but scale it, and we don't want to scale
    // the baseline, just the peak of the waveform
    float scale[32] =
    {
        1.0, 1.0, 1.0, 1.0, 1.1, 1.2, 1.3, 1.5, 1.7, 2.0, 2.1, 2.2, 2.1, 2.0, 1.7, 1.5,
        1.3, 1.2, 1.1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    };

    uint16_t gain = 2000;

    s->eventType = SPIKE_EVENT_CODE;
    s->timestamp = timestamp;
    s->source = 0;
    s->nChannels = 4;
    s->nSamples = 32;
    s->electrodeID = 0;
    int idx=0;

    int waveType = rand()%2; // Pick one of the three predefined waveshapes to generate
    int shift = 1000 + 32768;

    for (int i=0; i<4; i++)
    {
        s->gain[i] = gain;
        s->threshold[i] = 4000;
        double scaleExponent = (double)(rand()%26+2) / 10.0f;  // Scale the wave between 50% and 150%

        for (int j=0; j<32; j++)
        {

            int n = 0;
            if (noise>0)
            {
                n = rand() % noise - noise/2;
            }

            s->data[idx] = (trace[waveType][j] + n)  * pow(double(scale[j]),scaleExponent) + shift;
            idx = idx+1;
        }
    }

}

void generateEmptySpike(SpikeObject* s, int nChannels, int numSamples)
{

    s->eventType = SPIKE_EVENT_CODE;
    s->timestamp = 0;
    s->source = 0;
    s->nChannels = 1;
    s->nSamples = numSamples;
    s->electrodeID = 0;
    s->samplingFrequencyHz = 30000;
    s->sortedId = 0;
    s->color[0] = s->color[1] = s->color[2] = 128;
    s->pcProj[0] = s->pcProj[1] = 0;


    int idx = 0;
    for (int i=0; i<s->nChannels; i++)
    {
        s->gain[i] = 0.0;
        s->threshold[i] = 0;
        for (int j=0; j<s->nSamples; j++)
        {
            s->data[idx] = 0;
            idx = idx+1;
        }
    }
}

void printSpike(SpikeObject* s)
{

    std::cout<< " SpikeObject:\n";
    std::cout<< "\tTimestamp:" << s->timestamp;
    std::cout<< "\tSource:" << s->source;
    std::cout<< "\tnChannels:" <<s->nChannels;
    std::cout<<"\tnSamples" << s->nSamples;
    std::cout<<"\n\t 8 Data Samples:";
    for (int i=0; i<8; i++)
        std::cout<<s->data+i<<" ";
    std::cout<<std::endl;
}

float spikeDataBinToMicrovolts(SpikeObject *s, int bin, int ch)
{
    jassert(ch >= 0 && ch < s->nChannels);
    jassert(bin >= 0 && ch < s->nSamples);
    float v= float(s->data[bin+ch*s->nSamples]-32768)/float(s->gain[ch])*1000.0f;
    return v;
}


float spikeDataIndexToMicrovolts(SpikeObject *s, int index)
{
    int gain_index = index / s->nSamples;
    jassert(gain_index >= 0 && gain_index < s->nChannels);
    float v= float(s->data[index]-32768)/float(s->gain[gain_index])*1000.0f;
    return v;
}



int microVoltsToSpikeDataBin(SpikeObject *s, float uV, int ch)
{
    return uV/1000.0f*float(s->gain[ch])+32768;
}




float spikeTimeBinToMicrosecond(SpikeObject *s, int bin, int ch)
{
    float spikeTimeSpan = 1.0f/s->samplingFrequencyHz * s->nSamples * 1e6;
    return float(bin)/(s->nSamples-1) * spikeTimeSpan;
}

int microSecondsToSpikeTimeBin(SpikeObject *s, float t, int ch)
{
    // Lets say we have 32 samples per wave form

    // t = 0 corresponds to the left most index.
    float spikeTimeSpan = (1.0f/s->samplingFrequencyHz * s->nSamples)*1e6;
    return MIN(s->nSamples-1, MAX(0,t/spikeTimeSpan * (s->nSamples-1)));
}


SpikeChannel::SpikeChannel(SpikeDataType type, int nChans, void* ptr, int size)
	: ChannelExtraData(ptr, size), dataType(type), numChannels(nChans)
{
}

String generateSpikeElectrodeName(int numChannels, int index)
{
	String name;
	switch (numChannels)
	{
	case 1:
		name = String("SE");
		break;
	case 2:
		name = String("ST");
		break;
	case 4:
		name = String("TT");
		break;
	default:
		name = String("ELEC");
		break;
	}
	return name + String(index);
}