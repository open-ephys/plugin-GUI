#include "CyclopsAPI.h"

namespace cyclops
{

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 SINGLE BYTE COMMANDS                          |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

bool start (CyclopsRPC *rpc, const int *channels, int channelCount)
{
    rpc->message[0] = getSingleByteHeader(channels, channelCount) | START;
    rpc->length = 1;
    return true;
}

bool stop (CyclopsRPC *rpc, const int *channels, int channelCount)
{
    rpc->message[0] = getSingleByteHeader(channels, channelCount) | STOP;
    rpc->length = 1;
    return true;
}

bool reset (CyclopsRPC *rpc, const int *channels, int channelCount)
{
    rpc->message[0] = getSingleByteHeader(channels, channelCount) | RESET;
    rpc->length = 1;
    return true;
}

bool swap (CyclopsRPC *rpc, int c1, int c2)
{
    int channels[] = {c1, c2};
    rpc->message[0] = getSingleByteHeader(channels, 2) | SWAP;
    rpc->length = 1;
    return true;
}

bool identify (CyclopsRPC *rpc)
{
    rpc->message[0] = IDENTIFY;
    rpc->length = 1;
    return true;
}

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 MULTI BYTE COMMANDS                           |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/


bool change_source_loop (CyclopsRPC *rpc, int channel, int srcID)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_LOOP;
    rpc->message[1] = (uint8_t) srcID;
    rpc->length = 2;
    return true;
}

bool change_source_one_shot (CyclopsRPC *rpc, int channel, int srcID)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_ONE;
    rpc->message[1] = (uint8_t) srcID;
    rpc->length = 2;
    return true;
}

bool change_source_n_shot (CyclopsRPC *rpc, int channel, int srcID, int shot_cycle = 1)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_N;
    rpc->message[1] = (uint8_t) srcID;
    rpc->message[2] = (uint8_t) ((shot_cycle < 1)? 1 : shot_cycle);
    rpc->length = 3;
    return true;
}

bool change_time_period (CyclopsRPC *rpc, int channel, uint32_t new_period)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_TIME_PERIOD;
    (uint32_t) (rpc->message + 1) = new_period;
    rpc->length = 5;
    return true;
}

bool time_factor (CyclopsRPC *rpc, int channel, float tFactor)
{
    rpc->message[0] = getMultiByteHeader(channel) | TIME_FACTOR;
    (float) (rpc->message + 1) = tFactor;
    rpc->length = 5;
    return true;
}

bool voltage_factor (CyclopsRPC *rpc, int channel, float vFactor)
{
    rpc->message[0] = getMultiByteHeader(channel) | VOLTAGE_FACTOR;
    (float) (rpc->message + 1) = vFactor;
    rpc->length = 5;
    return true;
}

bool voltage_offset (CyclopsRPC *rpc, int channel, int16_t vOffset)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_LOOP;
    (int16_t) (rpc->message + 1) = vOffset;
    rpc->length = 3;
    return true;
}

bool square_on_time (CyclopsRPC *rpc, int channel, uint32_t onTime)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_ON_TIME;
    (uint32_t) (rpc->message + 1) = onTime;
    rpc->length = 5;
    return true;
}

bool square_off_time (CyclopsRPC *rpc, int channel, uint32_t offTime)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_OFF_TIME;
    (uint32_t) (rpc->message + 1) = offTime;
    rpc->length = 5;
    return true;
}

bool square_on_level (CyclopsRPC *rpc, int channel, uint16_t onLevel)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_ON_LEVEL;
    (uint32_t) (rpc->message + 1) = onLevel;
    rpc->length = 3;
    return true;
}

bool square_off_level (CyclopsRPC *rpc, int channel, uint16_t offLevel)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_OFF_LEVEL;
    (uint32_t) (rpc->message + 1) = offLevel;
    rpc->length = 3;
    return true;
}

} // NAMESPACE cyclops

inline static getSingleByteHeader (const int *channels, int num_channels)
{
    int channelMask = 0;
    for (int i=0; i < channelCount; i++)
        channelMask |= (1 << channels[i]);
    return (1 << 7) | channelMask;
}

inline static getMultiByteHeader(channel) (int channel)
{
    return (channel << 5);
}

#endif