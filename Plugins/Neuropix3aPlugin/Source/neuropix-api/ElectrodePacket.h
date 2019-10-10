/**
 * @file ElectrodePacket.h
 * This file describes a complete electrode packet,
 * containing 1 lfp value for all channels, and for each channel 12 ap values.
 */
#ifndef ElectrodePacket_h_
#define ElectrodePacket_h_

#include "dll_import_export.h"

class DLL_IMPORT_EXPORT ElectrodePacket
{
public:
  ElectrodePacket();
  ~ElectrodePacket();

  /**
   * This function prints an electrode packet.
   */
  void printPacket();

  /**
   * The start triggers of all 12 iterations.
   */
  bool startTrigger [12];

  /**
   * The synchronization words of all 12 iterations.
   */
  unsigned short synchronization [12];

  /**
   * The 13 counters of all 12 iterations.
   */
  unsigned int ctrs [12][13];

  /**
   * The complete set of 384 lfp data words, 1 per channel.
   */
  float lfpData [384]; // in Volts

  /**
   * 12 sets of 384 ap data words, 1 per channel.
   */
  float apData [12][384]; // in Volts


private:
};

#endif
