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

#ifndef SPIKEOBJECT_H_
#define SPIKEOBJECT_H_

#include <iostream>
#include <stdint.h>
#include <math.h>

#define MAX_NUMBER_OF_SPIKE_CHANNELS 4
#define MAX_NUMBER_OF_SPIKE_CHANNEL_SAMPLES 60
#define CHECK_BUFFER_VALIDITY true
#define SPIKE_EVENT_CODE 4;
#define MAX_SPIKE_BUFFER_LEN 512 // the true max calculated from the spike values below is actually 507

struct SpikeObject{

  uint8_t     eventType;
  uint64_t    timestamp;                                           
  uint16_t    source;                                        
  uint16_t    nChannels;                                      
  uint16_t    nSamples;
  uint16_t    data[MAX_NUMBER_OF_SPIKE_CHANNELS * MAX_NUMBER_OF_SPIKE_CHANNEL_SAMPLES]; 
  uint16_t    gain[MAX_NUMBER_OF_SPIKE_CHANNELS];           
  uint16_t    threshold[MAX_NUMBER_OF_SPIKE_CHANNELS];    

};

/*
  For transmission between processors SpikeObjects must be packaged up into buffers that can fit into MidiEvents
  The following two methods can be used to package the above spike object into a buffer and  unpackage a buffer
  into a SpikeObject.  

  The buffer is LittleEndian (thank Intel) and the byte order is the same as the SpikeObject definition. 
  IE. the first 2 bytes are the timestamp, the next two bytes are the source identifier, etc... with the last
  set of bytes corresponding to the thresholds of the different channels.

  Finally the buffer will have an additional byte on the end that is used to check the integerity of the entire package.
  The way this works is the buffer is divivded up into a series of 16 bit unsigned integers. The sum of all these integers
  (except the last 16 bit integer) is taken and the sum should equal that 16 bit integer. If not then the data is corrupted 
  and should be dropped or dealt with another way.
*/

// Simple method for serializing a SpikeObject into a string of bytes, returns true is the packaged spike buffer is valid
int packSpike(SpikeObject *s, uint8_t* buffer, int bufferLength);

// Simple method for deserializing a string of bytes into a Spike object, returns true is the provided spike buffer is valid
bool unpackSpike(SpikeObject *s, uint8_t* buffer, int bufferLength);

// Checks the validity of the buffer, this should be run before unpacking the buffer
bool isBufferValid(uint8_t *buffer, int bufferLength);

// Computes the validity value for the buffer, this should be called after packing the buffer
void makeBufferValid(uint8_t *buffer, int bufferLength);

// Help function for generating fake spikes in the absence of a real spike source. 
// Can be used to generate a sign wave with a fixed Frequency of 1000 hz or a basic spike waveform
// Additionally noise can be added to the waveform for help in diagnosing projection plots
void generateSimulatedSpike(SpikeObject *s, uint64_t timestamp, int noise);

// Define the << operator for the SpikeObject
// std::ostream& operator<<(std::ostream &strm, const SpikeObject s);

// Helper function for zeroing out a spike object with a specified number of channels
void generateEmptySpike(SpikeObject *s, int nChannels);

void printSpike(SpikeObject *s);


#endif //SPIKEOBJECT_H_