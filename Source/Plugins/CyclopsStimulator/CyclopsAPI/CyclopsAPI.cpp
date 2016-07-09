#include "CyclopsAPI.h"
#include <string.h>

namespace cyclops
{

inline static uint8_t getSingleByteHeader (const int *_channels, int num_channels)
{
    int channelMask = 0;
    for (int i=0; i < num_channels; i++)
        channelMask |= (1 << (_channels[i]));
    return (1 << 7) | channelMask;
}

inline static uint8_t getMultiByteHeader (int _channel)
{
    return (_channel << 5);
}

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
    int ch[] = {c1, c2};
    rpc->message[0] = getSingleByteHeader(ch, 2) | SWAP;
    rpc->length = 1;
    return true;
}

bool identify (CyclopsRPC *rpc)
{
    rpc->message[0] = IDENTITY;
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
    rpc->length = CHANGE_SOURCE_LOOP_LEN;
    return true;
}

bool change_source_one_shot (CyclopsRPC *rpc, int channel, int srcID)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_ONE;
    rpc->message[1] = (uint8_t) srcID;
    rpc->length = CHANGE_SOURCE_ONE_LEN;
    return true;
}

bool change_source_n_shot (CyclopsRPC *rpc, int channel, int srcID, int shot_cycle /* = 1*/)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_N;
    rpc->message[1] = (uint8_t) srcID;
    rpc->message[2] = (uint8_t) ((shot_cycle < 1)? 1 : shot_cycle);
    rpc->length = CHANGE_SOURCE_N_LEN;
    return true;
}

bool change_time_period (CyclopsRPC *rpc, int channel, uint32_t new_period)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_TIME_PERIOD;
    //*reinterpret_cast<uint32_t*> (rpc->message + 1) = new_period;
    memcpy(rpc->message+1, &new_period, sizeof(uint32_t));
    rpc->length = CHANGE_TIME_PERIOD_LEN;
    return true;
}

bool time_factor (CyclopsRPC *rpc, int channel, float tFactor)
{
    rpc->message[0] = getMultiByteHeader(channel) | TIME_FACTOR;
    //*reinterpret_cast<float*> (rpc->message + 1) = tFactor;
    memcpy(rpc->message+1, &tFactor, sizeof(float));
    rpc->length = TIME_FACTOR_LEN;
    return true;
}

bool voltage_factor (CyclopsRPC *rpc, int channel, float vFactor)
{
    rpc->message[0] = getMultiByteHeader(channel) | VOLTAGE_FACTOR;
    //*reinterpret_cast<float*> (rpc->message + 1) = vFactor;
    memcpy(rpc->message+1, &vFactor, sizeof(float));
    rpc->length = VOLTAGE_FACTOR_LEN;
    return true;
}

bool voltage_offset (CyclopsRPC *rpc, int channel, int16_t vOffset)
{
    rpc->message[0] = getMultiByteHeader(channel) | CHANGE_SOURCE_LOOP;
    //*reinterpret_cast<int16_t*> (rpc->message + 1) = vOffset;
    memcpy(rpc->message+1, &vOffset, sizeof(int16_t));
    rpc->length = CHANGE_SOURCE_LOOP_LEN;
    return true;
}

bool square_on_time (CyclopsRPC *rpc, int channel, uint32_t onTime)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_ON_TIME;
    //*reinterpret_cast<uint32_t*> (rpc->message + 1) = onTime;
    memcpy(rpc->message+1, &onTime, sizeof(uint32_t));
    rpc->length = SQUARE_ON_TIME_LEN;
    return true;
}

bool square_off_time (CyclopsRPC *rpc, int channel, uint32_t offTime)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_OFF_TIME;
    //*reinterpret_cast<uint32_t*> (rpc->message + 1) = offTime;
    memcpy(rpc->message+1, &offTime, sizeof(uint32_t));
    rpc->length = SQUARE_OFF_TIME_LEN;
    return true;
}

bool square_on_level (CyclopsRPC *rpc, int channel, uint16_t onLevel)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_ON_LEVEL;
    //*reinterpret_cast<uint32_t*> (rpc->message + 1) = onLevel;
    memcpy(rpc->message+1, &onLevel, sizeof(uint16_t));
    rpc->length = SQUARE_ON_LEVEL_LEN;
    return true;
}

bool square_off_level (CyclopsRPC *rpc, int channel, uint16_t offLevel)
{
    rpc->message[0] = getMultiByteHeader(channel) | SQUARE_OFF_LEVEL;
    //*reinterpret_cast<uint32_t*> (rpc->message + 1) = offLevel;
    memcpy(rpc->message+1, &offLevel, sizeof(uint16_t));
    rpc->length = SQUARE_OFF_LEVEL_LEN;
    return true;
}

} // NAMESPACE cyclops