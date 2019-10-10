/**
 * @file Testconfiguration.h
 * This file defines the contents of the test configuration shift register.
 */
#ifndef TestConfiguration_h_
#define TestConfiguration_h_

#include "dll_import_export.h"

#include <vector>

class TestConfiguration
{
public:
  TestConfiguration();
  ~TestConfiguration();

  /**
   * This function sets all members to their default values.
   */
  void reset();

  /**
   * This function combines all members to the chain of bools as needed for the
   * shift register.
   *
   * @param chain : the chain to return
   */
  void getChain(std::vector<bool> & chain);

  /**
   * This function translates the chain from the shift register to the members
   * of the test configuration.
   *
   * @param chain : the chain to translate
   */
  void getTestConfigFromChain(std::vector<bool> & chain);



  bool digi[2];  // selection of internal test points
  bool adcdata;  // test of parallel ADC data
  bool digi2[2]; // selection of internal test points (b:1, a:0)
  bool fclk;     // select filter clk source
  bool muxAna;  // test of mux output selection
  bool muxa;     // select mux control signal source
  bool lfp[4];   // test of lfp channel output selection
  bool ap[4];    // test of ap channel output selection
  bool pocl[2];  // test of pixel output selection (b:1, a:0)


private:

};

#endif
