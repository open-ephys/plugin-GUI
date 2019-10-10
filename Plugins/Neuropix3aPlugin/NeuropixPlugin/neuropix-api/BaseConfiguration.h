/**
 * @file BaseConfiguration.h
 * This file defines the contents of the base configuration shift register.
 */
#ifndef BaseConfiguration_h_
#define BaseConfiguration_h_

#include "dll_import_export.h"

#include <vector>

/**
 * Base configuration error code.
 */
enum BaseConfigErrorCode
{
  BASECONFIG_SUCCESS     = 0, /**< get or set successfully */
  ILLEGAL_ADC_NUMBER     = 1, /**< adc number out of range (valid: 0 to 31) */
  ILLEGAL_CHANNEL_NUMBER = 2, /**< channel number out of range (valid: 0 to 383) */
  ILLEGAL_WRITE_VALUE    = 3, /**< value to write out of range */
  BASECONFIG_WRITE_ERROR = 4, /**< error writing base configuration shift
                                register */
  ILLEGAL_CHAN_REF_READ  = 5  /**< in case a ChannelRefxxx contains more than 1 ones*/
};

/**
 * ADC calibration: comparator offsets (separate for all ADCs).
 */
struct adcComp {
  unsigned char compP; /**< Comparator offset P: 5 bits */
  unsigned char compN; /**< Comparator offset N: 5 bits */
};

/**
 * ADC calibration: subregisters that are common for 2 ADCs.
 */
struct adcPairCommon {
  unsigned char slope;  /**< Transfer curve slope  correction: 3 bits */
  unsigned char fine;   /**< Transfer curve fine   correction: 2 bits */
  unsigned char coarse; /**< Transfer curve coarse correction: 2 bits */
  unsigned char cfix;   /**< Transfer curve offset correction: 4 bits */
};

/**
 * Channel Settings.
 */
struct ChanSetxx {
  unsigned char gAp;    /**< Gain AP  band: 3 bits */
  unsigned char gLfp;   /**< Gain LFP band: 3 bits */
  bool stdbCh;          /**< Set channel in standby when true */
};

/**
 * Channel reference.
 */
struct ChanRefxxx {
  bool r [11];
};

/**
 * Additional settings.
 */
struct AdditionalSettings {
  unsigned char bwHp;    /**< High pass corner frequency: 2 bits */
  bool swBulk;  /**< IA input pair bulk connection */
  bool selBias; /**< Bias selection (0 = bandgap, 1 = external resistor */
  bool testOn;  /**< Enable test structures on ASIC when true */
  bool sarCal;  /**< Enable SAR calibration when true */
};

struct AsicID;

class BaseConfiguration
{
public:
  BaseConfiguration(AsicID * asicid);
  ~BaseConfiguration();

  /**
   * This function sets all parameters to the default values.
   */
  void reset();

  /**
   * this function sets the asic id of this base configuration.
   *
   * @param asicid : the asicid to set
   */
  void setAsicId(AsicID * asicid);

  /**
   * This function combines all members to the chain of bools as needed for the
   * shift register.
   *
   * @param chain : the chain to store result
   */
  void getChain(std::vector<bool> & chain);

  /**
   * This function translates the chain from the shift register to the members
   * of the base configuration.
   *
   * @param chain : the chain to translate
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_READ_VALUE in case a ChannelRefxxx contains more than 1 ones
   */
  BaseConfigErrorCode getBaseConfigFromChain(std::vector<bool> & chain);

  /**
   * This function returns a vector containing the elements of the
   * ADC_calibration_comp array.
   *
   * @return the adc_calibration_comp
   */
  std::vector<adcComp> getAdcCalibrationComp();

  /**
   * This function returns a vector containing the elements of the
   * ADC_calibration_common array.
   *
   * @return the adc_calibration_common
   */
  std::vector<adcPairCommon> getAdcCalibrationCommon();

  /**
   * This function returns a vector containing the elements of the
   * channel_settings array.
   *
   * @return the channel_settings
   */
  std::vector<ChanSetxx> getChannelSettings();

  /**
   * This function returns the true reference index.
   *
   * @param channel  : the index of the channel (valid range: 0 to 383)
   * @param refIndex : the reference index to store result
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channel > 383
   */
  BaseConfigErrorCode getChannelReferenceIndex(unsigned int channel,
                                               unsigned char & refIndex);

  /**
   * This function returns the true reference indices of all channels.
   *
   * @return the vector of channel reference indices
   */
  std::vector<unsigned char> getAllChannelReferenceIndices();

  /**
   * This function returns the additional settings
   *
   * @param settings : to store result
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode getAdditionalSettings(AdditionalSettings & settings);

  /**
   * This function sets the compP value of the adc_comp of the given adc to the
   * given value.
   *
   * @param adcnumber : the number of the adc (valid range: 0 to 31)
   * @param compP     : the compP value to write (valid range: 0 to 31)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcnumber > 31,
   *         ILLEGAL_WRITE_VALUE if compP > 31
   */
  BaseConfigErrorCode setAdcCompP(unsigned char adcnumber,
                                  unsigned char compP);

  /**
   * This function sets the compN value of the adc_comp of the given adc to the
   * given value.
   *
   * @param adcnumber : the number of the adc (valid range: 0 to 31)
   * @param compN     : the compN value to write (valid range: 0 to 31)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcnumber > 31,
   *         ILLEGAL_WRITE_VALUE if compP > 31
   */
  BaseConfigErrorCode setAdcCompN(unsigned char adcnumber,
                                  unsigned char compN);

  /**
   * This function sets the compP/n value of all adc
   *
   * @param adcComps : vector of 32 adcComp
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcComps size is not 32
   *         ILLEGAL_WRITE_VALUE if compP > 31
   *         ILLEGAL_WRITE_VALUE if compN > 31
   */
  BaseConfigErrorCode setAdcComp(std::vector<adcComp> adcComps);

  /**
   * This function sets the slope value of the adc pair of the given adc to the
   * given value.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15)
   * @param slope        : the slope value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcPairIndex > 15,
   *         ILLEGAL_WRITE_VALUE if slope > 7
   */
  BaseConfigErrorCode setAdcPairSlope(unsigned char adcPairIndex,
                                      unsigned char slope);

  /**
   * This function sets the fine value of the adc pair of the given adc to the
   * given value.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15)
   * @param fine         : the fine value to write (valid range: 0 to 3)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcPairIndex > 15,
   *         ILLEGAL_WRITE_VALUE if fine > 3
   */
  BaseConfigErrorCode setAdcPairFine(unsigned char adcPairIndex,
                                     unsigned char fine);

  /**
   * This function sets the coarse value of the adc pair of the given adc to the
   * given value.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15)
   * @param coarse       : the coarse value to write (valid range: 0 to 3)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcPairIndex > 15,
   *         ILLEGAL_WRITE_VALUE if coarse > 3
   */
  BaseConfigErrorCode setAdcPairCoarse(unsigned char adcPairIndex,
                                       unsigned char coarse);

  /**
   * This function sets the slope/coarse/fine value of the adc pair of the
   * given adc to the given value.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15)
   * @param slope        : the slope value to write (valid range: 0 to 7)
   * @param coarse       : the coarse value to write (valid range: 0 to 3)
   * @param fine         : the fine value to write (valid range: 0 to 3)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcPairIndex > 15,
   *         ILLEGAL_WRITE_VALUE if slope > 7
   *         ILLEGAL_WRITE_VALUE if coarse > 3
   *         ILLEGAL_WRITE_VALUE if fine > 3
   */
  BaseConfigErrorCode setAdcPairSlope(unsigned char adcPairIndex,
                                      unsigned char slope,
                                      unsigned char coarse,
                                      unsigned char fine);

  /**
   * This function sets the cfix value of the adc pair of the given adc to the
   * given value.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15)
   * @param cfix         : the cfix value to write (valid range: 0 to 15)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER if adcPairIndex > 15,
   *         ILLEGAL_WRITE_VALUE if cfix > 15
   */
  BaseConfigErrorCode setAdcPairCfix(unsigned char adcPairIndex,
                                     unsigned char cfix);

  /**
   * This function sets the slope value of all adc pair
   *
   * @param adcPairs : 16 adcPairCommon
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_ADC_NUMBER  if adcPairs size is not 16
   *         ILLEGAL_WRITE_VALUE if slope > 7
   *         ILLEGAL_WRITE_VALUE if coarse > 3
   *         ILLEGAL_WRITE_VALUE if fine > 3
   *         ILLEGAL_WRITE_VALUE if cfix > 15
   */
  BaseConfigErrorCode setAdcPair(std::vector<adcPairCommon> adcPairs);

  /**
   * This function sets the ap gain of the given channel to the given value.
   *
   * @param channelnumber : the number of the channel (valid range: 0 to 383)
   * @param apgain        : the ap gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383,
   *         ILLEGAL_WRITE_VALUE if apgain > 7
   */
  BaseConfigErrorCode setChannelSettingApGain(unsigned int channelnumber,
                                              unsigned char apgain);

  /**
   * This function sets the lfp gain of the given channel to the given value.
   *
   * @param channelnumber : the number of the channel (valid range: 0 to 383)
   * @param lfpgain       : the lfp gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383,
   *         ILLEGAL_WRITE_VALUE if lfpgain > 7
   */
  BaseConfigErrorCode setChannelSettingLfpGain(unsigned int channelnumber,
                                               unsigned char lfpgain);

  /**
   * This function sets the standby of the given channel to the given value.
   *
   * @param channelnumber : the number of the channel (valid range: 0 to 383)
   * @param standby       : the standby value to write
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383
   */
  BaseConfigErrorCode setChannelSettingStandby(unsigned int channelnumber,
                                               bool standby);

  /**
   * This function sets all channel settings to the given values.
   *
   * @param channelsettings : the channelsettings to write
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if vector size != 384,
   *         ILLEGAL_WRITE_VALUE if apgain  > 7,
   *         ILLEGAL_WRITE_VALUE if lfpgain > 7
   */
  BaseConfigErrorCode setChannelSettings(std::vector<ChanSetxx> &
                                         channelsettings);

  /**
   * This function sets the channel reference of the given index to true,
   * and the other reference values to false.
   *
   * @param channel : the number of the channel
   *                  (valid range: 0 to 383 for options 1-3,
   *                                0 to 275 for option 4)
   * @param index   : the reference index to enable
   *                  (valid range: 0 to 10 for options 1-3,
   *                                0 to  7 for option 4)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383,
   *         ILLEGAL_WRITE_VALUE if index > 10 for options 1-3,
   *         or index > 7 for option 4
   */
  BaseConfigErrorCode setChannelReferenceIndex(unsigned int channel,
                                               unsigned int index);

  /**
   * This function sets all channel reference indices to the given values.
   *
   * @param channelrefs : the channel reference indices to write
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if vector size != 384,
   *         ILLEGAL_WRITE_VALUE if index > 10
   */
  BaseConfigErrorCode setAllChannelReferenceIndices(std::vector<unsigned char> &
                                                    channelrefs);

  /**
   * This function sets the high pass corner frequency of the additional
   * settings to the given value.
   *
   * @param bwhp : the bwHp value to write (valid range: 0 to 3)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if bwhp > 3 or bwhp == 2
   */
  BaseConfigErrorCode setAdditionalSettingBwHp(unsigned int bwhp);

  /**
   * This function sets the bulk connection of the additional settings to the
   * given value.
   *
   * @param swbulk : the swBulk value to write
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode setAdditionalSettingSwBulk(bool swbulk);

  /**
   * This function sets the bias selection of the additional settings to the
   * given value.
   *
   * @param selbias : the selBias value to write
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode setAdditionalSettingSelBias(bool selbias);

  /**
   * This function sets the test on of the additional settings to the
   * given value.
   *
   * @param teston : the testOn value to write
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode setAdditionalSettingTestOn(bool teston);

  /**
   * This function sets the SAR calibration of the additional settings to the
   * given value.
   *
   * @param sarcal : the sarCal value to write
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode setAdditionalSettingSarCal(bool sarcal);

  /**
   * This function sets the AP gain of every channel to the given value.
   *
   * @param apgain : the ap gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if apgain > 7
   */
  BaseConfigErrorCode setAllAPGains(unsigned char apgain);

  /**
   * This function sets the LFP gain of every channel to the given value.
   *
   * @param lfpgain : the lfp gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if lfpgain > 7
   */
  BaseConfigErrorCode setAllLFPGains(unsigned char lfpgain);

  /**
   * This function sets the standby value of every channel to the given value.
   *
   * @param standby : the standby value to write
   *
   * @return BASECONFIG_SUCCESS
   */
  BaseConfigErrorCode setAllStandby(bool standby);

  /**
   * This function sets the reference indexes of all channels to the given
   * value.
   *
   * @param refindex   : the reference index to enable
   *                     (valid range: 0 to 10 for options 1-3,
   *                                   0 to  7 for option 4)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if index out of range
   */
  BaseConfigErrorCode setAllReferences(unsigned char refindex);


private:

  /**
   * This function switches lsbs and msbs from the given variable and puts them
   * at the end of the given chain.
   *
   * @param variable : the variable of which to switch the bits
   * @param length   : the amount of valid bits
   * @param chain    : the chain to which the switched bits are pushed
   */
  void switchBitsToChain(unsigned char variable, unsigned char length,
                         std::vector<bool> & chain);

  /**
   * This function pops the given amount of bits from the back of the given
   * chain and switches msbs and lsbs before returning the resulting number.
   *
   * @param length : the amount of bits to pop
   * @param chain  : the chain from which the bits are popped
   *
   * @return the resulting number
   */
  unsigned char switchBitsFromChain(unsigned char length,
                                    std::vector<bool> & chain);

  adcComp adcCalibrationComp_ [32];
  adcPairCommon adcCalibrationCommon_ [16];
  ChanSetxx channelSettings_  [384];
  ChanRefxxx channelReference_ [384];
  AdditionalSettings additionalSettings_;
  AsicID * asicId_;
};

#endif
