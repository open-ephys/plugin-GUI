/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include <stdint.h>
#include <math.h>

#include "../Channel/Channel.h"

#define SPIKE_METADATA_SIZE 42
#define MAX_NUMBER_OF_SPIKE_CHANNELS 4
#define MAX_NUMBER_OF_SPIKE_CHANNEL_SAMPLES 80
#define CHECK_BUFFER_VALIDITY true
#define SPIKE_EVENT_CODE 4;
#define MAX_SPIKE_BUFFER_LEN 512 // max length of spike buffer in bytes
                                 // the true max calculated from the spike values below is actually 507

/** Class to store spike data in event channels */
class SpikeChannel : public ChannelExtraData
{
public:
	enum SpikeDataType { Plain = 0, Sorted = 1};

	SpikeChannel(SpikeDataType type, int nChans, void* ptr, int size);

	const SpikeDataType dataType;
	const int numChannels;
};

/** Simple generic function to name spike electrode channels */
String generateSpikeElectrodeName(int numChannels, int index);

#define SPIKE_BASE_CODE 100

/**

  Allows spikes to be transmitted between processors.

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

struct SpikeObject
{

    uint8_t     eventType;
    int64_t    timestamp;
    int64_t    timestamp_software;
    uint16_t    source; // used internally, the index of the electrode in the electrode array
    uint16_t    nChannels;
    uint16_t    nSamples;
    uint16_t    sortedId;   // sorted unit ID (or 0 if unsorted)
    uint16_t    electrodeID; // unique electrode ID (regardless electrode position in the array)
    uint16_t    channel; // the channel in which threshold crossing was detected (index in channel array, not absolute channel number).
    uint8_t     color[3];
    float       pcProj[2];
    uint16_t    samplingFrequencyHz;
    uint16_t    data[MAX_NUMBER_OF_SPIKE_CHANNELS * MAX_NUMBER_OF_SPIKE_CHANNEL_SAMPLES];
    float       gain[MAX_NUMBER_OF_SPIKE_CHANNELS];
    uint16_t    threshold[MAX_NUMBER_OF_SPIKE_CHANNELS];
};



float spikeDataIndexToMicrovolts(SpikeObject *s, int index);

float spikeDataBinToMicrovolts(SpikeObject *s, int bin, int ch = 0);
int microVoltsToSpikeDataBin(SpikeObject *s, float uV, int ch = 0);
float spikeTimeBinToMicrosecond(SpikeObject *s, int bin, int ch=0);
int microSecondsToSpikeTimeBin(SpikeObject *s, float t, int ch=0);

/** Simple method for serializing a SpikeObject into a string of bytes, returns true is the packaged spike buffer is valid */
int packSpike(const SpikeObject* s, uint8_t* buffer, int bufferLength);

/** Simple method for deserializing a string of bytes into a Spike object, returns true is the provided spike buffer is valid */
bool unpackSpike(SpikeObject* s, const uint8_t* buffer, int bufferLength);

/** Checks the validity of the buffer, this should be run before unpacking the buffer */
bool isBufferValid(const uint8_t* buffer, int bufferLength);

/** Computes the validity value for the buffer, this should be called after packing the buffer */
void makeBufferValid(uint8_t* buffer, int bufferLength);

/** Helper function for generating fake spikes in the absence of a real spike source.
  Can be used to generate a sign wave with a fixed Frequency of 1000 hz or a basic spike waveform
  Additionally noise can be added to the waveform for help in diagnosing projection plots */
void generateSimulatedSpike(SpikeObject* s, uint64_t timestamp, int noise);

// Define the << operator for the SpikeObject
// std::ostream& operator<<(std::ostream &strm, const SpikeObject s);

/** Helper function for zeroing out a spike object with a specified number of channels */
void generateEmptySpike(SpikeObject* s, int nChannels, int numSamples);

void printSpike(SpikeObject* s);

static const int N_WAVEFORM_SAMPLES = 120;
static const double SPIKE_WAVEFORMS[5][N_WAVEFORM_SAMPLES] =
{
    {
        1.0000, 1.0008, 1.0016, 1.0023, 1.0030, 1.0030, 1.0031, 1.0031, 1.0032, 1.0035, 1.0038, 1.0041,
        1.0053, 1.0074, 1.0095, 1.0116, 1.0167, 1.0235, 1.0303, 1.0370, 1.0478, 1.0596, 1.0713, 1.0831,
        1.1006, 1.1183, 1.1361, 1.1544, 1.1768, 1.1992, 1.2217, 1.2415, 1.2550, 1.2685, 1.2820, 1.2881,
        1.2853, 1.2826, 1.2798, 1.2708, 1.2579, 1.2451, 1.2322, 1.2174, 1.2019, 1.1865, 1.1710, 1.1569,
        1.1428, 1.1288, 1.1149, 1.1031, 1.0912, 1.0793, 1.0681, 1.0589, 1.0496, 1.0404, 1.0318, 1.0240,
        1.0163, 1.0085, 1.0015, 0.9950, 0.9886, 0.9821, 0.9761, 0.9702, 0.9643, 0.9585, 0.9534, 0.9484,
        0.9434, 0.9385, 0.9348, 0.9311, 0.9273, 0.9239, 0.9215, 0.9191, 0.9166, 0.9142, 0.9118, 0.9094,
        0.9070, 0.9048, 0.9028, 0.9008, 0.8987, 0.8968, 0.8948, 0.8928, 0.8908, 0.8893, 0.8878, 0.8862,
        0.8847, 0.8829, 0.8811, 0.8794, 0.8777, 0.8764, 0.8751, 0.8738, 0.8727, 0.8719, 0.8710, 0.8701,
        0.8699, 0.8704, 0.8708, 0.8713, 0.8720, 0.8729, 0.8738, 0.8747, 0.8755, 0.8763, 0.8772, 0.8780
    },
    {
        1.0000, 0.9998, 0.9996, 0.9995, 0.9993, 0.9996, 0.9999, 1.0002, 1.0007, 1.0014, 1.0021, 1.0028,
        1.0041, 1.0062, 1.0083, 1.0103, 1.0154, 1.0220, 1.0286, 1.0353, 1.0470, 1.0599, 1.0729, 1.0858,
        1.1072, 1.1288, 1.1505, 1.1724, 1.1963, 1.2201, 1.2439, 1.2634, 1.2724, 1.2813, 1.2903, 1.2911,
        1.2822, 1.2733, 1.2643, 1.2506, 1.2339, 1.2172, 1.2006, 1.1841, 1.1678, 1.1514, 1.1351, 1.1222,
        1.1096, 1.0970, 1.0846, 1.0743, 1.0639, 1.0536, 1.0437, 1.0355, 1.0272, 1.0190, 1.0113, 1.0044,
        0.9975, 0.9906, 0.9847, 0.9794, 0.9742, 0.9689, 0.9645, 0.9603, 0.9561, 0.9520, 0.9485, 0.9451,
        0.9417, 0.9384, 0.9356, 0.9327, 0.9299, 0.9272, 0.9253, 0.9233, 0.9213, 0.9191, 0.9163, 0.9136,
        0.9109, 0.9086, 0.9067, 0.9047, 0.9028, 0.9009, 0.8991, 0.8972, 0.8954, 0.8940, 0.8927, 0.8913,
        0.8900, 0.8889, 0.8878, 0.8867, 0.8857, 0.8851, 0.8846, 0.8840, 0.8835, 0.8830, 0.8825, 0.8821,
        0.8823, 0.8830, 0.8838, 0.8846, 0.8858, 0.8872, 0.8885, 0.8899, 0.8918, 0.8937, 0.8957, 0.8977
    },
    {
        1.0000, 1.0001, 1.0003, 1.0004, 1.0005, 1.0003, 1.0002, 1.0000, 0.9997, 0.9992, 0.9988, 0.9983,
        0.9982, 0.9987, 0.9991, 0.9995, 1.0025, 1.0069, 1.0114, 1.0158, 1.0252, 1.0356, 1.0461, 1.0566,
        1.0732, 1.0900, 1.1068, 1.1239, 1.1425, 1.1612, 1.1799, 1.1957, 1.2043, 1.2129, 1.2215, 1.2241,
        1.2194, 1.2147, 1.2100, 1.2006, 1.1881, 1.1757, 1.1632, 1.1502, 1.1370, 1.1237, 1.1105, 1.0990,
        1.0876, 1.0761, 1.0650, 1.0558, 1.0467, 1.0376, 1.0290, 1.0218, 1.0146, 1.0075, 1.0009, 0.9953,
        0.9896, 0.9839, 0.9787, 0.9739, 0.9690, 0.9642, 0.9599, 0.9559, 0.9518, 0.9478, 0.9442, 0.9408,
        0.9373, 0.9339, 0.9308, 0.9277, 0.9246, 0.9215, 0.9188, 0.9160, 0.9132, 0.9107, 0.9087, 0.9066,
        0.9045, 0.9026, 0.9009, 0.8991, 0.8973, 0.8959, 0.8945, 0.8932, 0.8919, 0.8907, 0.8895, 0.8883,
        0.8872, 0.8861, 0.8850, 0.8839, 0.8829, 0.8824, 0.8820, 0.8815, 0.8813, 0.8818, 0.8822, 0.8827,
        0.8834, 0.8844, 0.8853, 0.8863, 0.8872, 0.8881, 0.8890, 0.8899, 0.8910, 0.8921, 0.8932, 0.8943
    },
    {
        1.0000, 1.0000, 1.0001, 1.0001, 1.0002, 1.0002, 1.0002, 1.0002, 1.0001, 1.0000, 0.9999, 0.9997,
        1.0000, 1.0008, 1.0016, 1.0024, 1.0053, 1.0095, 1.0137, 1.0179, 1.0261, 1.0353, 1.0444, 1.0536,
        1.0685, 1.0836, 1.0987, 1.1139, 1.1295, 1.1452, 1.1609, 1.1740, 1.1805, 1.1870, 1.1936, 1.1952,
        1.1909, 1.1866, 1.1823, 1.1745, 1.1645, 1.1545, 1.1445, 1.1335, 1.1221, 1.1108, 1.0995, 1.0894,
        1.0794, 1.0694, 1.0596, 1.0514, 1.0433, 1.0351, 1.0274, 1.0208, 1.0142, 1.0077, 1.0016, 0.9963,
        0.9910, 0.9858, 0.9812, 0.9771, 0.9731, 0.9690, 0.9657, 0.9627, 0.9597, 0.9567, 0.9541, 0.9515,
        0.9489, 0.9464, 0.9440, 0.9417, 0.9393, 0.9371, 0.9352, 0.9333, 0.9314, 0.9297, 0.9282, 0.9267,
        0.9253, 0.9238, 0.9224, 0.9210, 0.9196, 0.9184, 0.9171, 0.9159, 0.9146, 0.9139, 0.9131, 0.9124,
        0.9117, 0.9111, 0.9105, 0.9100, 0.9094, 0.9091, 0.9087, 0.9084, 0.9082, 0.9081, 0.9080, 0.9080,
        0.9080, 0.9082, 0.9083, 0.9085, 0.9089, 0.9095, 0.9100, 0.9106, 0.9116, 0.9127, 0.9138, 0.9148
    },
    {
        1.0000, 0.9997, 0.9994, 0.9990, 0.9987, 0.9982, 0.9977, 0.9972, 0.9969, 0.9972, 0.9975, 0.9978,
        0.9987, 1.0002, 1.0017, 1.0032, 1.0063, 1.0103, 1.0144, 1.0184, 1.0251, 1.0325, 1.0400, 1.0474,
        1.0574, 1.0676, 1.0777, 1.0880, 1.0996, 1.1111, 1.1227, 1.1332, 1.1410, 1.1489, 1.1567, 1.1612,
        1.1617, 1.1622, 1.1627, 1.1586, 1.1516, 1.1446, 1.1375, 1.1282, 1.1182, 1.1082, 1.0981, 1.0885,
        1.0789, 1.0693, 1.0598, 1.0520, 1.0442, 1.0364, 1.0290, 1.0227, 1.0165, 1.0103, 1.0046, 0.9998,
        0.9950, 0.9901, 0.9859, 0.9821, 0.9784, 0.9746, 0.9715, 0.9686, 0.9658, 0.9629, 0.9603, 0.9578,
        0.9553, 0.9528, 0.9505, 0.9481, 0.9458, 0.9435, 0.9412, 0.9390, 0.9367, 0.9347, 0.9331, 0.9314,
        0.9298, 0.9285, 0.9275, 0.9265, 0.9255, 0.9247, 0.9239, 0.9232, 0.9224, 0.9218, 0.9212, 0.9206,
        0.9200, 0.9197, 0.9194, 0.9190, 0.9187, 0.9184, 0.9181, 0.9179, 0.9177, 0.9178, 0.9179, 0.9180,
        0.9184, 0.9189, 0.9194, 0.9199, 0.9207, 0.9215, 0.9224, 0.9233, 0.9240, 0.9247, 0.9254, 0.9261
    }
};

static const double WAVEFORM_SCALE[N_WAVEFORM_SAMPLES] =
{
    1.0027, 1.0035, 1.0045, 1.0057, 1.0073, 1.0093, 1.0117, 1.0146, 1.0181, 1.0223, 1.0273, 1.0331,
    1.0398, 1.0474, 1.0559, 1.0652, 1.0754, 1.0862, 1.0976, 1.1092, 1.1208, 1.1321, 1.1429, 1.1526,
    1.1612, 1.1681, 1.1733, 1.1764, 1.1774, 1.1763, 1.1730, 1.1678, 1.1607, 1.1520, 1.1420, 1.1310,
    1.1194, 1.1075, 1.0956, 1.0840, 1.0729, 1.0625, 1.0529, 1.0442, 1.0365, 1.0298, 1.0240, 1.0190,
    1.0149, 1.0115, 1.0087, 1.0065, 1.0046, 1.0031, 1.0019, 1.0008, 0.9998, 0.9988, 0.9978, 0.9966,
    0.9953, 0.9937, 0.9919, 0.9897, 0.9872, 0.9842, 0.9808, 0.9769, 0.9726, 0.9679, 0.9629, 0.9575,
    0.9519, 0.9462, 0.9406, 0.9351, 0.9300, 0.9253, 0.9213, 0.9180, 0.9155, 0.9140, 0.9135, 0.9140,
    0.9155, 0.9180, 0.9213, 0.9253, 0.9300, 0.9351, 0.9406, 0.9462, 0.9519, 0.9574, 0.9628, 0.9678,
    0.9725, 0.9768, 0.9806, 0.9840, 0.9869, 0.9894, 0.9915, 0.9933, 0.9948, 0.9959, 0.9969, 0.9976,
    0.9982, 0.9987, 0.9990, 0.9993, 0.9995, 0.9996, 0.9997, 0.9998, 0.9999, 0.9999, 0.9999, 1.0000,
};

#endif //SPIKEOBJECT_H_