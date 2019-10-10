/**
 * @file ShankConfiguration.h
 * This file defines the content of a shank configuration register.
 */
#ifndef ShankConfiguration_h_
#define ShankConfiguration_h_

#include "dll_import_export.h"

#include <vector>

/**
 * Shank configuration error code.
 */
enum ShankConfigErrorCode
{
  SHANK_SUCCESS              = 0, /**< access was sucessful                   */
  SHANK_ILLEGAL_OPTION       = 1, /**< illegal option for use of shank config */
  SHANK_ILLEGAL_CHANNEL      = 2, /**< channel number out of range            */
  SHANK_ILLEGAL_CONNECTION   = 3, /**< connection number out of range         */
  SHANK_ILLEGAL_CHAIN_CONFIG = 4, /**< multiple electrodes connected to channel*/
  SHANK_WRITE_ERROR          = 5  /**< error writing shank configuration shift
                                    register */
};

enum ElectrodeConnection
{
  NONE = 0xff,
  ZERO = 0,
  ONE = 1,
  TWO = 2,
  THREE = 3
};

struct AsicID;

class ShankConfiguration
{
public:
  ShankConfiguration(AsicID * asicid);
  ~ShankConfiguration();

  /**
   * This function sets all members to their default values.
   */
  void reset();

  /**
   * This function sets the asic id of this shank configuration.
   *
   * @param asicid : the asicid to set
   */
  void setAsicId(AsicID * asicid);

  /**
   * This function combines all members to the chain of bools as needed for the
   * shift register.
   *
   * @param chain : the chain to return
   *
   * @return SHANK_SUCCESS if sucessful
   */
  ShankConfigErrorCode getChain(std::vector<bool> & chain);

  /**
   * This function translates the chain from the shift register to the members
   * of the shank configuration.
   *
   * @param chain : the chain to translate
   *
   * @return SHANK_SUCCESS if sucessful
   */
  ShankConfigErrorCode getShankConfigFromChain(std::vector<bool> & chain);

  /**
   * This function sets the given channel to the given electrode connection.
   *
   * @param channel : the channel number to set
   *                  (valid range: 0 to 383 for option 3,
   *                                0 to 275 for option 4)
   * @param connection : the connection number to set
   *                     (valid range option 3: 0 to 2 or 0xff(=NONE) for electrode < 960,
   *                      valid range option 4: 0 to 3 or 0xff(=NONE) for electrode < 966)
   *
   * @return SHANK_SUCCESS if sucessful
   */
  ShankConfigErrorCode setElectrodeConnection(unsigned int channel,
                                              unsigned char connection);

  /**
   * This function is similar to setElectrodeConnection, but electrode number is provided instead
   * @param electrode : valid range 0 to 383 for option 1 and 2,
   *                                0 to 960 for option 3,
   *                                0 to 966 for option 4
   *
   * @return SHANK_SUCCESS if sucessful
   */
  ShankConfigErrorCode enableElectrode(unsigned int electrode);

  /**
   * This function returns vector of the electrode connections.
   *
   * @param connections : the vector of the electrode connections to return
   */
  void getElectrodeConnections(std::vector<unsigned char> & connections);


  /**
   * @return Even Reference bit
   */
  bool getEvenReference();
  /**
   * @return Odd Reference bit
   */
  bool getOddReference();
  /**
   * @param enable : Even Reference value to set
   */
  void setEvenReference(bool enable);
  /**
   * @param enable : Odd Reference value to set
   */
  void setOddReference(bool enable);

  /**
   * This functions disconnects all electrodes for the Internal Reference channels
   */
  ShankConfigErrorCode disableAllInternalReferences();

private:
  ElectrodeConnection electrodeConnection_[384];
  AsicID * asicId_;
  bool evenReference_;
  bool oddReference_;

};

#endif

