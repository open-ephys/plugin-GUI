/**
 * @file AdcPacket.h
 * This file describes a complete adc packet,
 * containing for each of 13 iterations for each adc 1 data value.
 */
#ifndef AdcPacket_h_
#define AdcPacket_h_

#include "dll_import_export.h"

/**
 * AdcSubPacket : contains 1 counter value and all 32 adc data words.
 */
struct AdcSubPacket {
  unsigned int ctr;
  float adcData [32]; // in Volts
};

class DLL_IMPORT_EXPORT AdcPacket
{
public:
  AdcPacket();
  ~AdcPacket();

  /**
   * This function prints an electrode packet.
   */
  void printPacket();

  /**
   * A complete adc packet consists of 13 AdcSubPacket of each 32 adc data words.
   */
  AdcSubPacket adcSubPackets [13];

private:
};

#endif
