/**
 * @file Neuropix_basestation_api.h
 * This file defines the api of this project.
 */
#ifndef Neuropix_basestation_api_h_
#define Neuropix_basestation_api_h_

#include "dll_import_export.h"

class NeuropixConnectionLinkIntf;
class VciInterface;
class neuroprobe;
class field_command;

#include "ConnectionLinkPacket.h"
#include "AdcPacket.h"
#include "ElectrodePacket.h"
#include "BaseConfiguration.h"
#include "TestConfiguration.h"
#include "ShankConfiguration.h"

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <cstdlib>

/**
 * Asic ID register
 */
struct AsicID {
  unsigned int serialNumber; /**< 30 bit */
  char probeType;            /**< 2  bit : indicates option */
};

/**
 * Version number of the api / fpga.
 */
struct VersionNumber {
  unsigned short major; /**< should be compatible for hardware and software */
  unsigned short minor; /**< indicates small updates that did not affect other
                          side */
};

/**
 * General error code.
 */
enum ErrorCode
{
  SUCCESS = 0, /**< successful */
  FAILURE = 1  /**< an error was detected */
};

/**
 * Error code for neuropix_open.
 */
enum OpenErrorCode
{
  OPEN_SUCCESS                 = 0, /**< device opened successfully */
  DATA_LINK_FAILED             = 1, /**< error while opening data link */
  CONFIG_LINK_FAILED           = 2, /**< error while opening config link */
  DEVICE_ALREADY_OPEN          = 3, /**< device was already open */
  WRONG_HW_SW_VERSION          = 4, /**< incompatible hw and sw version */
  CONFIG_DESER_FAILED          = 5, /**< error while configuring deserializer */
  CONFIG_SER_FAILED            = 6, /**< error while configuring serializer */
  CONFIG_HS_ND_ENABLE_FAILED   = 7, /**< error while configuring headstage
                                      ND_ENABLE */
  CONFIG_HS_TE_FAILED          = 8, /**< error while configuring headstage TE */
  CONFIG_HS_PRNRST_FAILED      = 9, /**< error while configuring headstage
                                      PR_NRST */
  CONFIG_HS_NRST_FAILED        = 10, /**< error while configuring headstage NRST */
  CONFIG_HS_MODE_FAILED        = 11, /**< error while configuring headstage MODE */
  CHECK_ASIC_ID_FAILED         = 12, /**< error while reading asic ID from
                                       EEPROM */
  CONFIG_BASE_REG_FAILED       = 13, /**< error while writing base shift
                                       register */
  CONFIG_SHANK_REG_FAILED      = 14, /**< error while writing shank shift
                                       register */
  CONFIG_TEST_REG_FAILED       = 15, /**< error while writing test shift
                                       register */
  CONFIG_TRIGGER_MODE_FAILED   = 16, /**< error while configuring basestation
                                       startTrigger_output_enable */
  CONFIG_NEURAL_START_FAILED   = 17, /**< error while configuring basestation
                                       neural_start */
  CONFIG_SYNC_EXT_START_FAILED = 18, /**< error while configuring basestation
                                       startTrigger_output */
  CONFIG_CALIB_FAILED          = 19, /**< error while configuring members for
                                       calibration */
  CONFIG_EEPROM_FAILED         = 20, /**< error while configuring the eeprom */
  CONFIG_HS_REG_FAILED         = 21, /**< error while configuring headstage
                                       registers */
  CONFIG_DATAMODE_FAILED       = 22  /**< error while setting the datamode */
};

enum ConfigDesError
{
  DES_SUCCESS                  = 0, /**< deserializer configured successfully */
  DES_ERROR                    = 1  /**< deserializer configuration error */
};

/**
 * Error code for neuropix_readADCData and neuropix_readElectrodeData.
 */
enum ReadErrorCode
{
  READ_SUCCESS           = 0, /**< data successfully read */
  NO_DATA_LINK           = 1, /**< data link not connected */
  WRONG_DATA_MODE        = 2, /**< incorrect data mode configured */
  DATA_BUFFER_EMPTY      = 3, /**< data not available in buffer */
  DATA_ERROR             = 4  /**< lfp_ctr doesn't match, or header, ...,
                                 The most probable cause is DRAM FIFO overflow*/
};

/**
 * Status of Basestation Datapath
 */
enum StatusCode
{
  DATAPATH_OK                 = 0, /**< datapath is fine */
  DATAPATH_OVERFLOW           = 1, /**< DRAM fifo overflow detected */
  DATAPATH_SYNC_ERROR         = 2, /**< SPI sync errors detected */
  DATAPATH_CTRS_ERROR         = 3, /**< SPI counters errors detected */
  DATAPATH_TEST_PATTERN_ERROR = 4, /**< test pattern errors detected */
  DATAPATH_DATA_PATTERN_ERROR = 5  /**< TE data pattern errors detected */
};

/**
 * Error code for neuropix_read/writeUART.
 */
enum UartErrorCode
{
  UART_SUCCESS                = 0, /**< data successfully read or written */
  UART_NO_CONFIG_LINK         = 1, /**< no configuration link existing  */
  UART_TIMEOUT                = 2, /**< uart timeout */
  UART_ACK_ERROR              = 3, /**< error in received acknowledge byte */
  UART_RX_OVERFLOW_ERROR      = 4, /**< uart error: rx overflow */
  UART_FRAME_ERROR            = 5, /**< uart protocol error: wrong rx stopbit */
  UART_PARITY_ERROR           = 6, /**< uart error: rx parity wrong */
  UART_UNDERFLOW_ERROR        = 7  /**< reading the command_readdata failed: fifo
                                        empty */

};

/**
 * Error code for accessing the shift registers.
 */
enum ShiftRegisterAccessErrorCode
{
  SHIFTREG_SUCCESS               = 0, /**< access of shift register successful */
  SHIFTREG_GETMODE_ERROR         = 1, /**< error while reading the mode */
  SHIFTREG_SETMODE_ERROR         = 2, /**< error while writing the mode */
  SHIFTREG_HS_UART_GEN_ERROR     = 3, /**< error while writing HS_UART_GEN */
  SHIFTREG_HS_TX_ERROR           = 4, /**< error while writing shift reg */
  SHIFTREG_HS_RX_ERROR           = 5, /**< error while reading shift reg */
  SHIFTREG_BITCHAIN_ERROR        = 6, /**< bitchain incorrect */
  SHIFTREG_OPTION_ERROR          = 7, /**< option incorrect */
  SHIFTREG_CONFIG_ERROR          = 8, /**< error while configuring bs fpga */
  SHIFTREG_ILLEGAL_CHAN_REF_READ = 9  /**<  in case a ChannelRefxxx contains
                                        more than 1 ones when reading out the
                                        base configuration shift register */
};

/**
 * Error code for accessing the digital control signals.
 */
enum DigitalControlErrorCode
{
  DIGCTRL_SUCCESS                = 0, /**< digital control access successful*/
  DIGCTRL_READVAL_ERROR          = 1, /**< uart error while dig ctrl read */
  DIGCTRL_WRITEVAL_ERROR         = 2, /**< uart error while dig ctrl write */
  DIGCTRL_WRITE_VAL_OUT_OF_RANGE = 3  /**< value to write is out of range */
};

/**
 * Status code for the DAC control.
 */
enum DacControlStatusCode
{
  DACCTRL_DISABLED  = 0, /**< DAC control is disabled*/
  DACCTRL_DC        = 1, /**< DAC control is in DC mode*/
  DACCTRL_SINE      = 2  /**< DAC control is in sine mode*/
};

/**
 * Error code for accessing the DAC control signals.
 */
enum DacControlErrorCode
{
  DACCTRL_SUCCESS                = 0, /**< DAC control registers access successful*/
  DACCTRL_READVAL_ERROR          = 1, /**< uart error while DAC ctrl read */
  DACCTRL_WRITEVAL_ERROR         = 2, /**< uart error while DAC ctrl write */
  DACCTRL_WRITE_VAL_OUT_OF_RANGE = 3  /**< value to write is unvalid */
};

/**
 * Error code for accessing the config parameters of the basestation fpga.
 */
enum ConfigAccessErrorCode
{
  CONFIG_SUCCESS          = 0, /**< config access successful */
  CONFIG_ERROR_NO_LINK    = 1, /**< no config link existing  */
  CONFIG_WRONG_MODE_ERROR = 2, /**< accessing electrode param while adc mode */
  CONFIG_WRITE_VAL_ERROR  = 3  /**< value to write (or its size) out of range */
};

/**
 * Error code for calibration procedures.
 */
enum CalibErrorCode
{
  CALIB_SUCCESS              = 0, /**< calibration procedure successful */
  CALIB_SETMODE_ERROR        = 1, /**< error while setting the mode */
  CALIB_SETBASECONFIG_ERROR  = 2, /**< error while setting the base configuration */
  CALIB_WRITESHIFT_ERROR     = 3, /**< error while writing shift register */
  CALIB_SETDATAMODE_ERROR    = 4, /**< error while setting the data mode */
  CALIB_DACCFG_ERROR         = 5, /**< error while setting the dac */
  CALIB_READDATA_ERROR       = 6, /**< error while reading data */
  CALIB_RESETDATAPATH_ERROR  = 7, /**< error while resetting datapath */
  CALIB_PREVIOUSCALIB_ERROR  = 8, /**< results of previous calibrations not
                                    available */
  CALIB_READASICID_ERROR     = 9, /**< error while reading the ASIC ID */
  CALIB_SETSHANKCONFIG_ERROR = 10, /**< error while setting shank configuration
                                   */
  CALIB_CONFIG_BS_ERROR      = 11  /**< error while writing configuration
                                     values to the basestation */
};

/**
 * Error code for accessing the eeprom
 */
enum EepromErrorCode
{
  EEPROM_SUCCESS        = 0, /**< accessing eeprom successful                 */
  EEPROM_EN_ERROR       = 1, /**< error enabling / disabling the eeprom       */
  EEPROM_ACKCLEAR_ERROR = 2, /**< error clearing / reading eeprom error reg   */
  EEPROM_DEV_ADDR_ERROR = 3, /**< error writing the eeprom device address     */
  EEPROM_MSBADDR_ERROR  = 4, /**< error writing the msb of the eeprom address */
  EEPROM_LSBADDR_ERROR  = 5, /**< error writing the lsb of the eeprom address */
  EEPROM_WRITE_ERROR    = 6, /**< error writing the write byte to eeprom      */
  EEPROM_READ_ERROR     = 7, /**< error reading the read byte from eeprom     */
  EEPROM_CMD_ERROR      = 8, /**< error writing the command for eeprom access */
  EEPROM_NO_ACK_ERROR   = 9, /**< no ack received                             */
  EEPROM_ACK_ERROR      = 10,/**< an eeprom acknowledge error was detected    */
  EEPROM_WRITEVAL_ERROR = 11, /**< write value out of valid range             */
  EEPROM_START_SEQ_ERROR = 12  /**< error writing the start seq bit to eeprom */
};

/**
 * Error code for BIST Test 4
 */
enum BistTest4ErrorCode
{
  BISTTEST4_SUCCESS        = 0, /**< BIST Test 4 successful */
  BISTTEST4_NO_DEVICE      = 1, /**< BIST Test 4 no device opened */
  BISTTEST4_LED_ERROR      = 2, /**< BIST Test 4 register read/write failure */
  BISTTEST4_DRAM_ERROR     = 3, /**< BIST Test 4 DRAM selftest failure */
  BISTTEST4_LOOPBACK_ERROR = 4  /**< BIST Test 4 data link loopback failure */
};

/**
 * Error code for BIST Test 5 & 7 & stop 9
 */
enum BistTestErrorCode
{
  BISTTEST_SUCCESS    = 0, /**< BIST Test (started / stopped) successful */
  BISTTEST_NO_DEVICE  = 1, /**< BIST Test no device opened */
  BISTTEST_UART_ERROR = 2  /**< BIST Test UART communication error */
};

/**
 * Error code for BIST Test 6
 */
enum BistTest6ErrorCode
{
   BISTTEST6_SUCCESS     = 0, /**< BIST Test 6 (started) successful */
   BISTTEST6_NO_DEVICE   = 1, /**< BIST Test 6 no device opened */
   BISTTEST6_UART_ERROR  = 2, /**< BIST Test 6 UART communication error */
   BISTTEST6_SER_ERROR   = 3, /**< BIST Test 6 serializer status at address 0x04
                                does not equal 0x87 */
   BISTTEST6_DESER_ERROR = 4, /**< BIST Test 6 deserializer status at address
                                0x04 does not equal 0x87 */
   BISTTEST6_PRBS_ERR    = 5  /**< BIST Test 6 PRBS_ERR not zero */
};

/**
 * Error code for BIST Test 8
 */
enum BistTest8ErrorCode
{
  BISTTEST8_SUCCESS     = 0, /**< BIST Test 8 started / stopped successful */
  BISTTEST8_NO_DEVICE   = 1, /**< BIST Test 8 no device opened */
  BISTTEST8_UART_ERROR  = 2, /**< BIST Test 8 UART communication error */
  BISTTEST8_RANGE_ERR   = 3, /**< BIST Test 8 SPI line out of range */
  BISTTEST8_DIGCTRL_ERR = 4  /**< BIST Test 8 digital control access failed */
};

/**
 * Error code for BIST Test 9
 */
enum BistTest9ErrorCode
{
  BISTTEST9_SUCCESS     = 0, /**< BIST Test 9 started / stopped successful */
  BISTTEST9_NO_DEVICE   = 1, /**< BIST Test 9 no device opened */
  BISTTEST9_DIGCTRL_ERR = 2  /**< BIST Test 9 digital control access failed */
};

/**
 * Error code for reading from a csv file.
 */
enum ReadCsvErrorCode
{
  READCSV_SUCCESS            = 0, /**< Reading the csv file was successful*/
  READCSV_FILE_ERR           = 1, /**< Error opening the filestream of the csv*/
  READCSV_NUMBER_OF_ELEMENTS = 2, /**< Invalid number of elements read*/
  READCSV_OUT_OF_RANGE       = 3  /**< Read element is out of range*/
};

enum AsicMode
{
  ASIC_CONFIGURATION = 0,
  ASIC_CALIBRATION   = 1,
  ASIC_IMPEDANCE     = 2,
  ASIC_RECORDING     = 3
};

class DLL_IMPORT_EXPORT Neuropix_basestation_api
{
public:
  Neuropix_basestation_api();
  ~Neuropix_basestation_api();


  /**
   * This function checks whether or not the data and config link are connected.
   *
   * @return true if data and config link are connected, else false
   */
  bool neuropix_isConnected();

  /**
   * This function establishes a data connection and a config link connection
   * with the FPGA and resets base/shank/test configuration with the default
   * values. It checks the compatibility of hardware and software version, and
   * closes the connection if they are not compatible. It reads the ASIC ID
   * from EEPROM. It reads the ADC calibration from EEPROM, and applies it to
   * the Base registers.
   *
   * @param headstage_select : selection for Penta Connect Board,
   *                           otherwise not relevant. Valid range 0 to 4.
   * @param bs_ip_address : ip address of the BS board (defined in the FPGA build).
   * @return OPEN_SUCCESS if sucessful
   */
  OpenErrorCode neuropix_open(unsigned char headstage_select = 0, std::string bs_ip_address = "10.2.0.1");

  /**
   * This function establishes a playback data connection and a dummy config
   * link connection.
   *
   * @param playbackfile : the path of the playbackfile
   DACTable_
   * @return OPEN_SUCCESS if sucessful
   */
  OpenErrorCode neuropix_open(const std::string & playbackfile);

  /**
   * This function closes the data and config link connection with the device.
   */
  virtual void neuropix_close();

  /**
   * this functions reads ADC calibration from EEPROM, put it in the calibration
   * member, then writes it to the Base Register
   *
   * @return SUCCESS if sucessful
   */
  ErrorCode neuropix_applyAdcCalibrationFromEeprom();

  /**
   * this functions reads Gain calibration from EEPROM, put it in the calibration
   * member, then writes it to the BS FPGA
   *
   * @return SUCCESS if sucessful
   */
  ErrorCode neuropix_applyGainCalibrationFromEeprom();

  /**
   * configure the Deserializer (to be done once after startup)
   *
   * @return SUCCESS if sucessful
   */
  ErrorCode neuropix_configureDeserializer();

  /**
   * configure the Serializer (to be done once after startup)
   *
   * @return SUCCESS if sucessful
   */
  ErrorCode neuropix_configureSerializer();

  /**
   * This function returns the fpga version number.
   *
   * @param version : the version number to return
   *
   * @return FAILURE if no config link connection, SUCCESS otherwise
   */
  ErrorCode neuropix_getHardwareVersion(VersionNumber * version);

  /**
   * This function returns the api version number.
   *
   * @return version number
   */
  const struct VersionNumber neuropix_getAPIVersion();


  /**
   * This function reads the ASIC ID from the EEPROM, copies it to the member
   * and returns it.
   *
   * @param id : the ASIC ID to return
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_readId(AsicID & id);

  /**
   * This function writes the ASIC ID to the member in de api and to the EEPROM.
   *
   * @param id : the ASIC ID to write
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_writeId(AsicID & id);

  /**
   * This function returns the ASIC ID member of the api.
   *
   * @return the ASIC ID to return
   */
  AsicID neuropix_getId();

  /**
   * This function reads the gain correction factors from the EEPROM and writes
   * them to the gainCorrectionCalibration_ member of the api.
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_readGainCorrection();

  /**
   * This function reads the adc calibration factors from the EEPROM and writes
   * them to the adcPairCommonCalibration_ and adcCompCalibration_ members of
   * the api.
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_readADCCalibration();

  /**
   * This function reads the gain correction factors from the
   * gainCorrectionCalibration_ member of the api and writes them to the EEPROM.
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_writeGainCorrection();

  /**
   * This function reads the adc correction factors from the
   * adcPairCommonCalibration_ and adcCompCalibration_ members of the api and
   * writes them to the EEPROM.
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_writeADCCalibration();

  /**
   * This function writes the ADC slope calibration values (slope, coarse and
   * fine) to the given csv file.
   *
   * @param filename : the filename to write to, (should be .csv) default is
   *                   adcSlopeCalValues.csv
   */
  void neuropix_writeADCSlopeCalibrationToCsv(std::string filename = "adcSlopeCalValues.csv");

  /**
   * This function writes the ADC offset calibration values (cfix) to the given
   * csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   adcOffsetCalValues.csv
   */
  void neuropix_writeADCOffsetCalibrationToCsv(std::string filename = "adcOffsetCalValues.csv");

  /**
   * This function writes the comparator offset calibration values (compP and
   * compN) to the given csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   comparatorCalValues.csv
   */
  void neuropix_writeComparatorCalibrationToCsv(std::string filename = "comparatorCalValues.csv");

  /**
   * This function writes the gain correction values from calibration to the
   * given csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   gainCorrCalValues.csv
   */
  void neuropix_writeGainCorrectionCalibrationToCsv(std::string filename = "gainCorrCalValues.csv");

  /**
   * This function reads the ADC slope calibration values (slope, coarse and
   * fine) from the given csv file.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   adcSlopeCalValues.csv
   *
   * @return READCSV_SUCCES if succesful
   */
  ReadCsvErrorCode neuropix_readADCSlopeCalibrationFromCsv(std::string filename = "adcSlopeCalValues.csv");

  /**
   * This function reads the ADC offset calibration values (cfix) from the given
   * csv file.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   adcOffsetCalValues.csv
   *
   * @return READCSV_SUCCES if succesful
   */
  ReadCsvErrorCode neuropix_readADCOffsetCalibrationFromCsv(std::string filename = "adcOffsetCalValues.csv");

  /**
   * This function reads the comparator calibration values (compP and compN)
   * from the given csv file.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   comparatorCalValues.csv
   *
   * @return READCSV_SUCCES if succesful
   */
  ReadCsvErrorCode neuropix_readComparatorCalibrationFromCsv(std::string filename = "comparatorCalValues.csv");

  /**
   * This function reads the gain calibration values (gain correction factors)
   * from the given csv file.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   gainCalValues.csv
   *
   * @return READCSV_SUCCES if succesful
   */
  ReadCsvErrorCode neuropix_readGainCalibrationFromCsv(std::string filename = "gainCalValues.csv");

  /**
   * This function writes the base configuration register as a bit chain to the
   * given csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   baseConfiguration.csv
   */
  void neuropix_writeBaseConfigurationToCsv(std::string filename = "baseConfiguration.csv");

  /**
   * This function writes the shank configuration register as a bit chain to the
   * given csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   shankConfiguration.csv
   */
  void neuropix_writeShankConfigurationToCsv(std::string filename = "shankConfiguration.csv");

  /**
   * This function writes the test configuration register as a bit chain to the
   * given csv file.
   *
   * @param filename : the filename to write to (should be .csv), default is
   *                   testConfiguration.csv
   */
  void neuropix_writeTestConfigurationToCsv(std::string filename = "testConfiguration.csv");

  /**
   * This function reads the base configuration register as a bit chain from the
   * given csv file, and loads it into the base configuration member of the api.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   baseConfiguration.csv
   *
   * @return READCSV_SUCCESS if succesful
   */
  ReadCsvErrorCode neuropix_readBaseConfigurationFromCsv(std::string filename = "baseConfiguration.csv");

  /**
   * This function reads the shank configuration register as a bit chain from
   * the given csv file, and loads it into the shank configuration member of the
   * api.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   shankConfiguration.csv
   *
   * @return READCSV_SUCCESS if succesful
   */
  ReadCsvErrorCode neuropix_readShankConfigurationFromCsv(std::string filename = "shankConfiguration.csv");

  /**
   * This function reads the test configuration register as a bit chain from the
   * given csv file, and loads it into the test configuration member of the api.
   *
   * @param filename : the filename to read from (should be .csv), default is
   *                   testConfiguration.csv
   *
   * @return READCSV_SUCCESS if succesful
   */
  ReadCsvErrorCode neuropix_readTestConfigurationFromCsv(std::string filename = "testConfiguration.csv");


  /**
   * This function sets the given channel to the given electrode connection, and
   * writes the resulting shank configuration chain to the headstage ASIC.
   *
   * @param channel : the channel number to set
   *                  (valid range: 0 to 383 for option 3,
   *                                0 to 275 for option 4)
   * @param electrode_bank : the electrode connection number to set
   *                         (valid range option 3: 0 to 2 for electrode < 960,
   *                          valid range option 4: 0 to 3 for electrode < 966)
   *
   * @return SHANK_SUCCESS if successful
   */
  ShankConfigErrorCode neuropix_selectElectrode(int channel, int electrode_bank, bool write_to_asic = true);

  /**
   * This function (dis)connects the External Reference (Even and Odd) and
   * writes the resulting shank configuration chain to the headstage ASIC.
   *
   * @param enable : set the ExtRef when True, unset otherwise
   *
   * @return SHANK_SUCCESS if successful
   */
  ShankConfigErrorCode neuropix_setExtRef(bool enable, bool write_to_asic = true);

  /**
   * This function writes the shank configuration chain to the headstage ASIC.
   *
   * @param read_check   : if enabled, for base/shank register only, read shift
   *                       register to check
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_writeShankRegister(bool read_check=true);

  /**
   * This function reads out the shank configuration chain from the headstage
   * ASIC and writes the values to the shank configuration member of the api.
   *
   * @param chain : the chain of shank configuration data to return
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_readShankRegister(std::vector<bool> &
                                                          chain);


  /**
   * This function writes the base configuration chain to the headstage ASIC.
   *
   * @param read_check   : if enabled, for base/shank register only, read shift
   *                       register to check
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_writeBaseRegister(bool read_check=true);

  /**
   * This function reads out the base configuration chain from the headstage
   * ASIC and writes the values to the base configuration member of the api.
   *
   * @param chain : the chain of base register data to return
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_readBaseRegister(std::vector<bool> &
                                                         chain);

  /**
   * This function sets both ap and lfp gain of the given channel to the given
   * values, and writes the resulting base configuration to the base
   * configuration shift register.
   *
   * @param channelnumber : the channel number (valid range: 0 to 383)
   * @param apgain        : the ap gain value (valid range: 0 to 7)
   * @param lfpgain       : the lfp gain value (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383,
   *         ILLEGAL_WRITE_VALUE if apgain > 7 or lfpgain > 7,
   *         BASECONFIG_WRITE_ERROR if error writing the base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_setGain(int channel, int ap_gain, int lfp_gain, bool write_to_asic = true);

  /**
   * This function sets the standby mode of a selected channel to the given
   * value, and writes the resulting base configuration to the base
   * configuration shift register.
   *
   * @param channelnumber : the number of the channel (valid range: 0 to 383)
   * @param standby       : the standby value to write
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_CHANNEL_NUMBER if channelnumber > 383,
   *         BASECONFIG_WRITE_ERROR if error writing the base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_setStdb(int channel, bool standby, bool write_to_asic = true);

  /**
   * This function sets the high pass filter frequency to the given value, and
   * writes the resulting base configuration to the base configuration shift
   * register.
   *
   * @param bwhp : the bwHp value to write
   *               (valid range: 0 = 300Hz, 1 = 500Hz, 3 = 1kHz)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if bwhp > 3 or bwhp == 2
   */
  BaseConfigErrorCode neuropix_setFilter(int filter);

  /**
   * This function sets the calibration bits of a selected ADC pair, and writes
   * the resulting base configuration to the base configuration shift register.
   *
   * @param adcPairIndex : the adc pair index (valid range: 0 to 15,
   *                       pair index 0 is adc 1 and 3,
   *                       pair index 1 is adc 2 and 4, etc
   * @param compPxx      : the compP value to write (valid range: 0 to 31)
   * @param compNxx      : the compN value to write (valid range: 0 to 31)
   * @param compPyy      : the compP value to write (valid range: 0 to 31)
   * @param compNyy      : the compN value to write (valid range: 0 to 31)
   * @param slope        : the slope value to write (valid range: 0 to 7)
   * @param fine         : the fine value to write (valid range: 0 to 3)
   * @param coarse       : the coarse value to write (valid range: 0 to 3)
   * @param cfix         : the cfix value to write (valid range: 0 to 15)
   *
   * @return BASECONFIG_SUCCESS if successful
   */
  BaseConfigErrorCode neuropix_ADCCalibration(unsigned char adcPairIndex,
                                              unsigned char compPxx,
                                              unsigned char compNxx,
                                              unsigned char compPyy,
                                              unsigned char compNyy,
                                              unsigned char slope,
                                              unsigned char fine,
                                              unsigned char coarse,
                                              unsigned char cfix);


  /**
   * This function selects the given reference input for the given channel, by
   * setting the channel reference of the given index to true, and the other
   * reference values to false, and writes the resulting base configuration to
   * the base configuration shift register.
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
   *         ILLEGAL_WRITE_VALUE if index > 10,
   *         BASECONFIG_WRITE_ERROR if an error occurs writing the
   *         base configuration shift register
   */
  BaseConfigErrorCode neuropix_setReference(int channel, int reference, bool write_to_asic = true);


  /**
   * This function writes the test configuration chain to the headstage ASIC.
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_writeTestRegister();


  /**
   * This function writes the ASIC MODE field of the Headstage General Control
   * Register #2 of the headstage FPGA.
   *
   * @param mode : the mode value to write (range 0 to 3) or use AsicMode enum
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_mode(unsigned char mode);

  /**
   * This function reads the ASIC MODE field of the Headstage General Control
   * Register #2 of the headstage FPGA.
   *
   * @param mode : the readout mode value
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_readMode(unsigned char & mode);

  /**
   * This function writes the TE field of the Headstage General Control
   * Register #1 of the headstage FPGA.
   *
   * @param te : the te value to write (valid range: 0 to 1)
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_te(unsigned char te);

  /**
   * This function writes the ND_ENABLE field of the Headstage General Control
   * Register #1 of the headstage FPGA.
   *
   * @param ndenable : the desired value for ndenable
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_ndEnable(bool ndenable);

  /**
   * This function writes the SW_HBEAT field of the Headstage General
   * Control Register #1 of the headstage FPGA.
   *
   * @param hs_sw_hbeat : the desired value for hs_sw_hbeat
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_HSSwHbeat(bool hs_sw_hbeat);

  /**
   * This function writes the HS_RESET field of the Headstage General
   * Control Register #2 of the headstage FPGA.
   *
   * @param hs_reset : the desired value for hs_reset
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_HSReset(bool hs_reset);

  /**
   * This function writes the BIST_RX field of the Headstage General
   * Control Register #1 of the headstage FPGA.
   *
   * @param bist_rx : the desired value for bist_rx
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_bistRx(bool bist_rx);

  /**
   * This function writes the BIST_TX field of the Headstage General
   * Control Register #1 of the headstage FPGA.
   *
   * @param bist_tx : the desired value for bist_tx
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_bistTx(bool bist_tx);

  /**
   * This function writes the ASIC_RESET field of the Headstage General Control
   * Register #2 of the headstage FPGA.
   *
   * @param nrst : the desired value for nrst
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_nrst(bool nrst);

  /**
   * This function writes the ASIC_PR_RESET field of the Headstage General
   * Control Register #2 of the headstage FPGA.
   *
   * @param pr_nrst : the desired value for pr_nrst
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_prnrst(bool pr_nrst);

  /**
   * This function writes the LED OFF field of the Headstage General
   * Control Register #1 of the headstage FPGA.
   *
   * @param led_off : the desired value for led_off
   *
   * @return DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_ledOff(bool led_off);

  /**
   * This function gets all digital control values.
   *
   * @param mode            : asic mode value
   * @param te              : te value
   * @param nrst            : value of nrst
   * @param pr_nrst         : value of pr_nrst
   * @param nd_enable       : nd enable value
   * @param sw_hbeat        : sw hbeat value
   * @param headstage_reset : value of the headstage reset
   * @param bist_rx         : value of bist_rx
   * @param bist_tx         : value of bist_tx
   * @param led_off         : value of led_off
   *
   * @return DIGCTRL_READVAL_ERROR when an error occurred while reading a
   * register, DIGCTRL_SUCCESS if successful
   */
  DigitalControlErrorCode neuropix_getDigCtrl(unsigned char & mode,
                                              bool & te,
                                              bool & nrst,
                                              bool & pr_nrst,
                                              bool & nd_enable,
                                              bool & sw_hbeat,
                                              bool & headstage_reset,
                                              bool & bist_rx,
                                              bool & bist_tx,
                                              bool & led_off);

  /**
   * This function writes the given gain correction factors to the registermap
   * of the basestation fpga.
   *
   * @param gaincorr : the gain correction factors to write
   *
   * @return CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_gainCorrection(std::vector<unsigned short>
                                                & gaincorr);

  /**
   * This function gets the gain correction factors from the registermap of the
   * basestation fpga.
   *
   * @param gaincorr : the gain correction factors to return
   *
   * @return CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getGainCorrection(std::vector<unsigned short>
                                                   & gaincorr);

  /**
   * This function sets the data mode to the given value.
   *
   * @param mode: adc mode = false, electrode mode = true
   *
   * @return FAILURE when datamodel does not exist, SUCCESS otherwise
   */
  ErrorCode neuropix_datamode(bool mode);

  /**
   * This function returns the current data mode.
   *
   * @return data mode: 0 is adc mode, 1 is electrode mode
   */
  bool neuropix_getDatamode();

  /**
   * This function reads the data mode from the basestation.
   *
   * @param mode : the data mode
   *
   * @return NO_DATA_LINK when datamodel does not exist,
   *         READ_SUCCESS if succesful
   */
  ReadErrorCode neuropix_readDatamodeFromBS(bool & mode);

  /**
   * This function reads an electrode packet of data.
   *
   * @param result: the resulting packet of electrode data
   *
   * @return error of ReadErrorCode
   */
  virtual ReadErrorCode neuropix_readElectrodeData(ElectrodePacket & result);

  /**
   * This function reads an adc packet of data.
   *
   * @param result: the resulting packet of adc data
   *
   * @return error of ReadErrorCode
   */
  virtual ReadErrorCode neuropix_readADCData(AdcPacket & result);

  /**
   * @return : status of datapath
   */
  StatusCode neuropix_checkBasestationDatapath();

  /**
   * @return : filling level, in %
   */
  float neuropix_fifoFilling();

  /**
   * This function resets the datapath of the basestation fpga:
   * - first the data_generator, decoupling fifos, deserializer_intf, scale,
   *   reorder, dma "functional cores" are put in reset (their corresponding
   *   configuration registers are not reset)
   * - next the DRAM fifo is flushed
   * - then the reset from the first step is released
   *
   * @return SUCCESS if successful
   */
  ErrorCode neuropix_resetDatapath();


  /**
   * This function runs the recalibration procedure
   *
   * @return SUCCESS if successful
   */
  ErrorCode neuropix_recalib();

  /**
   * This function selects the sine wave output channel of DAC A and configures
   * its frequency, offset and amplitude.
   *
   * @param frequency:
   * Sine wave frequency selection:
   * 0000’b : 7617 Hz (power-on default)
   * 0001’b : 3809 Hz
   * 0010’b : 1904 Hz
   * 0011’b : 952.1 Hz
   * 0100’b : 476.1 Hz
   * 0101’b : 238.0 Hz
   * 0110’b : 119.0 Hz
   * 0111’b : 59.51 Hz
   * 1000’b : 29.75 Hz
   * 1001’b : 14.88 Hz
   * 1010’b : 7.439 Hz
   * 1011’b : 3.719 Hz
   * 1100’b : 1.860 Hz
   * 1101’b : 0.9298 Hz
   * 1110’b : 0.4649 Hz
   * 1111’b : 0.2325 Hz
   *
   * @param fixed_offset: if this bool is set, a fixed offset of 0.6V will be
   * used
   *
   * @param amplitude:
   * Sine wave amplitude selection:
   * 0000’b : 5mV p.p. voltage level (power-on default)
   * 0001’b : 10mV p.p. voltage level
   * 0010’b : 100mV p.p. voltage level
   * 0011’b : 200mV p.p. voltage level
   * 0100’b : 300mV p.p. voltage level
   * 0101’b : 400mV p.p. voltage level
   * 0110’b : 500mV p.p. voltage level
   * 0111’b : 600mV p.p. voltage level
   * 1000’b : 700mV p.p. voltage level
   * 1001’b : 800mV p.p. voltage level
   * 1010’b : 900mV p.p. voltage level
   * 1011’b : 1000mV p.p. voltage level
   * 1100’b - 1111’b : reserved
   *
   * @return error of DacControlErrorCode
   */
  DacControlErrorCode neuropix_generateSine(int frequency, bool fixed_offset,
                                            int amplitude);

  /**
   * This function selects the DC output channel of DAC A and configures its
   * voltage level.
   *
   * @param voltage : the DC output voltage in mV
   *
   * @return error of DacControlErrorCode
   */
  DacControlErrorCode neuropix_generateDC(std::string voltage);

  /**
   * This function gets the DAC Control values.
   *
   * @param frequency : sine wave frequency
   * @param amplitude : sine wave amplitude
   * @param offset    : offset enabled when true, disabled when false
   * @param voltage   : DC output voltage level
   * @param status    : the status of the DAC control
   *
   * @return error of DacControlErrorCode
   */
  DacControlErrorCode neuropix_getDacControl(int & frequency, int & amplitude,
                                             bool & offset, std::string &
                                             voltage, DacControlStatusCode &
                                             status);

  /**
   * This function reads a register of a device.
   *
   * @param device: device address
   * @param address: register address
   * @param data: read register value
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_readUart(unsigned char device, unsigned char address, unsigned char & data);

  /**
   * This function writes a register of a device.
   *
   * @param device: device address
   * @param address: register address
   * @param data: register value to write
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_writeUart(unsigned char device, unsigned char address, unsigned char data);

  /**
   * This function reads a register of the deserializer.
   *
   * @param address: register address
   * @param value: read register value
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_deserializerReadRegister(unsigned char address, unsigned char & value);

  /**
   * This function writes a register of the deserializer.
   *
   * @param address: register address
   * @param value: register value to write
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_deserializerWriteRegister(unsigned char address, unsigned char value);

  /**
   * This function reads a register of the serializer.
   *
   * @param address: register address
   * @param value: read register value
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_serializerReadRegister(unsigned char address, unsigned char & value);

  /**
   * This function writes a register of the serializer.
   *
   * @param address: register address
   * @param value: register value to write
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_serializerWriteRegister(unsigned char address, unsigned char value);

  /**
   * This function reads a register of the headstage FPGA.
   *
   * @param address: register address
   * @param value: read register value
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_headstageReadRegister(unsigned char address, unsigned char & value);

  /**
   * This function writes a register of the headstage FPGA.
   *
   * @param address: register address
   * @param value: register value to write
   *
   * @return error of UartErrorCode
   */
  UartErrorCode neuropix_headstageWriteRegister(unsigned char address, unsigned char value);

  /**
   * This function returns the current option (probe type of the ASIC ID) of the
   * member of this api.
   *
   * @return the option
   */
  unsigned char neuropix_getOption();


  /**
   *  This function sets the AP gain of every channel to the given value, and
   *  writes the resulting base configuration to the base configuration shift
   *  register.
   *
   * @param apgain : the ap gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if apgain > 7,
   *         BASECONFIG_WRITE_ERROR if error writing base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_writeAllAPGains(int apgain);

  /**
   * This function sets the LFP gain of every channel to the given value, and
   * writes the resulting base configuration to the base configuration shift
   * register.
   *
   * @param lfpgain : the lfp gain value to write (valid range: 0 to 7)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if lfpgain > 7,
   *         BASECONFIG_WRITE_ERROR if error writing base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_writeAllLFPGains(int lfpgain);

  /**
   * This function sets the standby value of every channel to the given value,
   * and writes the resulting base configuration to the base configuration shift
   * register.
   *
   * @param standby : the standby value to write
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         BASECONFIG_WRITE_ERROR if error writing base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_writeAllStandby(bool standby);

  /**
   * This function sets the reference value of every channel to the given value,
   * and writes the resulting base configuration to the base configuration shift
   * register.
   *
   * @param refindex   : the reference index to enable
   *                     (valid range: 0 to 10 for options 1-3,
   *                                   0 to  7 for option 4)
   *
   * @return BASECONFIG_SUCCESS if successful,
   *         ILLEGAL_WRITE_VALUE if index out of range,
   *         BASECONFIG_WRITE_ERROR if error writing base
   *         configuration shift register
   */
  BaseConfigErrorCode neuropix_writeAllReferences(unsigned char reference);


  /**
   * This function calibrates the offset of each ASIC ADC, by searching the
   * ideal cfix parameter.
   *
   * @param fileName : the file name to save the results to, should be .csv,
   *                   default file name is results_adcOffsetCalibration.csv
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_ADCOffsetCalibration(std::string fileName = "results_adcOffsetCalibration.csv");

  /**
   * This function calibrates the slope of each ASIC ADC, by searching the ideal
   * slope, coarse and fine parameters.
   *
   * @param fileName : the file name to save the results to, should be .csv,
   *                   default file name is results_adcSlopeCalibration.csv
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_ADCSlopeCalibration(std::string fileName = "results_adcSlopeCalibration.csv");

  /**
   * This function calibrates the comparator of each ASIC ADC, by searching the
   * ideal compP and compN parameters.
   *
   * @param fileName : the file name to save the results to, should be .csv,
   *                   default file name is results_comparatorCalibration.csv
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_comparatorCalibration(std::string fileName = "results_comparatorCalibration.csv");

  /**
   * This function calculates the gain correction parameters for each electrode
   * on the ASIC.
   *
   * @param fileName : the file name to save the results to, should be .csv,
   *                   default file name is results_gainCalibration.csv
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_gainCalibration(std::string fileName = "results_gainCalibration.csv");


  /**
   * This function calculates the impedance for each electrode.
   *
   * @param fileName : the file name to save the results to, should be .csv,
   *                   default file name is results_impedanceMeasurement.csv
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_impedanceMeasurement(std::string fileName = "results_impedanceMeasurement.csv");



  /**
   * This function writes the given string to the lcd of the basestation fpga.
   *
   * @param line0 : the data for the first line (must be 16 characters)
   * @param line1 : the data for the second line (must be 16 characters)
   *
   * @return CONFIG_ERROR_NO_LINK if no configuration link existing,
   *         CONFIG_WRITE_VAL_ERROR if length of line1 or line2 is not 16,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_writeLCD(std::string line0 = "  Hello World   ",
                                          std::string line1 = "                ");


  /**
   * This function converts the current AsicID to an unsigned int.
   *
   * @return the unsigned int that contains the ASIC ID parts.
   */
  unsigned int neuropix_asicIdStructToUnsignedInt();

  /**
   * This function converts an unsigned int to the AsicID struct.
   *
   * @param u : the unsigned int to convert
   *
   * @return the AsicID
   */
  AsicID neuropix_asicIdUnsignedIntToStruct(unsigned int u);

  BaseConfiguration baseConfiguration;
  TestConfiguration testConfiguration;
  ShankConfiguration shankConfiguration;


  /**
   * ADC samples are encoded in 10bits, This function returns the scale factor
   * used to convert to Voltage.
   *
   * @returns the scale factor from ADC sample to Voltage, in V.
   */
  float neuropix_getScaleFactorToVoltage();

  /**
   * reads the BaseStation onboard temperature sensor and convert to Celsius
   *
   * @param temperature : the temperature of the BaseStation FPGA
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
   ConfigAccessErrorCode neuropix_readTemperature(float & temperature);

  /**
   * Start recording data. All data read from the basestation fpga over the
   * tcp/ip datalink (via neuropix_readADCData or neuropix_readElectrodeData)
   * will be recorded to file
   *
   * @param filename : name of the file to which data will be written (optional
   * parameter filename, default value = "datalog.npx")
   *
   * @return SUCCESS if successful, FAILURE is no data connection
   */
  ErrorCode neuropix_startRecording(const std::string filename = "datalog.npx");

  /**
   * Stop recording data.
   *
   * @return SUCCESS if successful, FAILURE is no data connection
   */
  ErrorCode neuropix_stopRecording();

  /**
   * Enable/Disable the Basestation generator.
   *
   * @param enable : true to enable generator, false to disable
   *        mode   : mode for data generator
   *                 0 : output constantly adc number
   *                 1 : all ADC = r_packet_ctr ( = sync_counter LSB + 1)
   *                 2 : all ADC = r_packet_ctr ( = sync_counter MSB + 1)
   *                 3 : all ADC = triangular signal with configurable period
   *        period : period of triangular signal in mode 3
   *                 0 : period = 2*2^10*2^0 / 23.4MHz = 87.5us
   *                 1 : period = 2*2^10*2^1 / 23.4MHz = 175.0us
   *                 2 : period = 2*2^10*2^2 / 23.4MHz = 350.1us
   *                 3 : period = 2*2^10*2^3 / 23.4MHz = 700.2us
   *                 4 : period = 2*2^10*2^4 / 23.4MHz = 1.4ms
   *                 5 : period = 2*2^10*2^5 / 23.4MHz = 2.8ms
   *                 6 : period = 2*2^10*2^6 / 23.4MHz = 5.6ms
   *                 7 : period = 2*2^10*2^7 / 23.4MHz = 11.2ms
   *                 8 : period = 2*2^10*2^8 / 23.4MHz = 22.4ms
   *                 9 : period = 2*2^10*2^9 / 23.4MHz = 44.8ms
   *                 10 : period = 2*2^10*2^10 / 23.4MHz = 98.6ms
   *                 ...
   *                 20 : period = 2*2^10*2^20 / 23.4MHz = 91.8s
   *                 21 : period = 2*2^10*2^21 / 23.4MHz = 183.6s
   *                 22 : period = 2*2^10*2^22 / 23.4MHz = 367.1s
   *                 23 : period = 2*2^10*2^23 / 23.4MHz = 734.2s
   *                 24 : period = 2*2^10*2^24 / 23.4MHz = 1468.4s
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_enableBasestationGenerator(bool enable, unsigned char mode = 0, unsigned int period = 0);

  /**
   * This function gets the basestation data generator parameters.
   *
   * @param enable : true if enabled, false if disabled
   * @param mode   : mode for data generator
   * @param period : period of triangular signal in mode 3
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getBasestationGeneratorEnabled(bool & enable,
                                                                unsigned char &
                                                                mode, unsigned
                                                                int & period);


  /**
   * This function sets the adcPairCommonCalibration_ member of the api.
   *
   * @param adcPairCommonCalibration : new value, vector of 16 adcPairCommon
   *
   * @return SUCCESS if successful, FAILURE if size doesn't match
   */
  ErrorCode neuropix_setADCPairCommonCalibration(
      std::vector<struct adcPairCommon> adcPairCommonCalibration);

  /**
   * This function sets the adcCompCalibration_ member of the api.
   *
   * @param adcCompCalibration : new value, vector of 32 adcComp
   *
   * @return SUCCESS if successful, FAILURE if size doesn't match
   */
  ErrorCode neuropix_setADCCompCalibration(
      std::vector<struct adcComp> adcCompCalibration);

  /**
   * This function sets the gainCorrectionCalibration_ member of the api.
   *
   * @param gainCorrectionCalibration : new value, vector of unsigned short of size 384 for Option 1 and 2, 960 for Option 3, 966 for Option 4
   *
   * @return SUCCESS if successful, FAILURE if size doesn't match
   */
  ErrorCode neuropix_setGainCorrectionCalibration(
      std::vector<unsigned short> gainCorrectionCalibration);

  /**
   * This function returns the adcPairCommonCalibration_ member of the api.
   *
   * @param adcPairCommonCalibration : current value, vector of 16.
   *
   * @return SUCCESS if successful, FAILURE if not available
   */
  ErrorCode neuropix_getADCPairCommonCalibration(
      std::vector<struct adcPairCommon> & adcPairCommonCalibration);

  /**
   * This function returns the adcCompCalibration_ member of the api.
   *
   * @param adcCompCalibration : current value, vector of 32
   *
   * @return SUCCESS if successful, FAILURE if not available
   */
  ErrorCode neuropix_getADCCompCalibration(
      std::vector<struct adcComp> & adcCompCalibration);

  /**
   * This function returns the gainCorrectionCalibration_ member of the api.
   *
   * @param gainCorrectionCalibration : current value, vector of unsigned short of size 384 for Option 1 and 2, 960 for Option 3, 966 for Option 4
   *
   * @return SUCCESS if successful, FAILURE if not available
   */
  ErrorCode neuropix_getGainCorrectionCalibration(
      std::vector<unsigned short> & gainCorrectionCalibration);

  /**
   * This function returns the impedanceMeasurement_ member of the api.
   *
   * @param impedanceMeasurement : current value, vector of int of size 384 for Option 1 and 2, 960 for Option 3, 966 for Option 4
   *
   * @return SUCCESS if successful, FAILURE if not available
   */
  ErrorCode neuropix_getImpedanceMeasurement(
      std::vector<int> & impedanceMeasurement);

  /**
   * run BIST test 4 : test leds, test LCD, test DRAM (selftest), test TCP data link
   * (loopback)
   *
   * @return BISTTEST4_SUCCESS if successful
   *         BISTTEST4_NO_DEVICE if no device opened
   *         BISTTEST4_LED_ERROR if register read/write failure
   *         BISTTEST4_DRAM_ERROR if DRAM selftest failure
   *         BISTTEST4_LOOPBACK_ERROR if data link loopback failure
   */
  BistTest4ErrorCode neuropix_test4();

  /**
   * run BIST test 5 : read HeadStage FPGA version + SW_HBEAT blink test @ 2Hz
   *
   * @param hs_brd_ver  : HeadStage Board Version result
   *        hs_fpga_ver : HeadStage FPGA Version result
   *        time_sec    : length of SW_HBEAT blink test, in seconds
   *
   * @return BISTTEST_SUCCESS if successful
   *         BISTTEST_NO_DEVICE if no device opened
   *         BISTTEST_UART_ERROR if UART communication error
   */
  BistTestErrorCode neuropix_test5(unsigned char & hs_brd_ver,
                                   unsigned char & hs_fpga_ver,
                                   unsigned int time_sec);


  /**
   * start BIST test 6 : Ser/Des PRBS pattern
   *
   * @return BISTTEST6_SUCCESS if started successful
   *         BISTTEST6_NO_DEVICE if no device opened
   *         BISTTEST6_UART_ERROR if UART communication error
   *         BISTTEST6_SER_ERROR if serializer status at address 0x04 does not
   *         equal 0x87
   *         BISTTEST6_DESER_ERROR if deserializer status at address 0x04 does
   *         not equal 0x87
   *         BISTTEST6_PRBS_ERR if PRBS_ERR is not zero
   */
  BistTest6ErrorCode neuropix_startTest6();

  /**
   * stop BIST test 6 : Ser/Des PRBS pattern
   *
   * @param prbs_err : error counter result
   *
   * @return BISTTEST6_SUCCESS if successful,
   *         BISTTEST6_NO_DEVICE if no device opened,
   *         BISTTEST6_UART_ERROR if UART communication error,
   *         BISTTEST6_SER_ERROR if serializer status at address 0x04 does not
   *         equal 0x87
   *         BISTTEST6_DESER_ERROR if deserializer status at address 0x04 does
   *         not equal 0x87
   *         BISTTEST6_PRBS_ERR if PRBS_ERR is not zero
   */
  BistTest6ErrorCode neuropix_stopTest6(unsigned char & prbs_err);

  /**
   * start BIST test 7 : Headstage Neural data test pattern
   *
   * @return BISTTEST_SUCCESS if started successful
   *         BISTTEST_NO_DEVICE if no device opened
   *         BISTTEST_UART_ERROR if UART communication error
   */
  BistTestErrorCode neuropix_startTest7();

  /**
   * start BIST test 7 : Headstage Neural data test pattern
   *
   * @return BISTTEST_SUCCESS if stopped successful
   *         BISTTEST_NO_DEVICE if no device opened
   *         BISTTEST_UART_ERROR if UART communication error
   */
  BistTestErrorCode neuropix_stopTest7();

  /**
   * BIST test 7 results
   *
   * @return number of errors in data test pattern
   */
  unsigned int neuropix_test7GetErrorCounter();

  /**
   * BIST test 7 results
   *
   * @return total number of patterns checked
   */
  unsigned int neuropix_test7GetTotalChecked();

  /**
   * BIST test 7 results
   *
   * @return mask of errors in data test pattern: bit 0 to 3 is respectively SPI
   * line 1 to 4
   */
  unsigned char neuropix_test7GetErrorMask();

  /**
   * start BIST test 8a (TE = true) or test 8b (TE = false)
   *
   * @param te : TE value.
   *        spi_line : 0 to 3, the SPI line to test.
   *
   * @return BISTTEST8_SUCCESS if started successful
   *         BISTTEST8_NO_DEVICE if no device opened
   *         BISTTEST8_UART_ERROR if UART communication error
   *         BISTTEST8_RANGE_ERR if SPI line out of range
   *         BISTTEST8_DIGCTRL_ERR if digital control access failed
   */
  BistTest8ErrorCode neuropix_startTest8(bool te, unsigned char spi_line);

  /**
   * stop BIST test 8a/b
   *
   * @return BISTTEST8_SUCCESS if stopped successful
   *         BISTTEST8_NO_DEVICE if no device opened
   *         BISTTEST8_UART_ERROR if UART communication error
   *         BISTTEST8_DIGCTRL_ERR if digital control access failed
   */
  BistTest8ErrorCode neuropix_stopTest8();

  /**
   * read result of BIST test 8a (TE = true) or test 8b (TE = false)
   *
   * @param te : TE value.
   *        spi_adc_err : to store spi_adc_err bit
   *        spi_ctr_err : to store spi_ctr_err bit
   *        spi_sync_err : to store spi_sync_err bit
   *
   * @return BISTTEST_SUCCESS if successful
   *         BISTTEST_NO_DEVICE if no device opened
   *         BISTTEST_UART_ERROR if UART communication error
   */
  BistTestErrorCode neuropix_test8GetErrorCounter(bool te,
                                                     bool & spi_adc_err,
                                                     bool & spi_ctr_err,
                                                     bool & spi_sync_err);


  /**
   * start BIST test 9a (TE = true) or test 9b (TE = false)
   *
   * @param te : TE value.
   *
   * @return BISTTEST9_SUCCESS if started successful
   *         BISTTEST9_NO_DEVICE if no device opened
   *         BISTTEST9_DIGCTRL_ERR if digital control access failed
   */
  BistTest9ErrorCode neuropix_startTest9(bool te);

  /**
   * stop BIST test 9a/b
   *
   * @return BISTTEST_SUCCESS if stopped successful
   *         BISTTEST_NO_DEVICE if no device opened
   *         BISTTEST_UART_ERROR if UART communication error
   */
  BistTestErrorCode neuropix_stopTest9();

  /**
   * read result of BIST test 9a/b
   *
   * @param sync_errors    : to store the number of sync errors
   *        sync_total     : to store total number of syncs checks (1 check =
   *                         all 4 syncs are correct in 1 spi packet)
   *        ctr_errors     : to store the number of counter errors
   *        ctr_total      : to store the total number of counter 10 lsbs and 10
   *                         msbs checks (1 check = all 4 10 lsbs or all 4 10
   *                         msbs are correct)
   *        te_spi0_errors : to store the number of data errors on spi line 0
   *        te_spi1_errors : to store the number of data errors on spi line 1
   *        te_spi2_errors : to store the number of data errors on spi line 2
   *        te_spi3_errors : to store the number of data errors on spi line 3
   *        te_spi_total   : to store the total number of data checked per spi
   *                         line
   */
  void neuropix_test9GetResults( unsigned int & sync_errors,
                                   unsigned int & sync_total,
                                   unsigned int & ctr_errors,
                                   unsigned int & ctr_total,
                                   unsigned int & te_spi0_errors,
                                   unsigned int & te_spi1_errors,
                                   unsigned int & te_spi2_errors,
                                   unsigned int & te_spi3_errors,
                                   unsigned int & te_spi_total);

  /**
   * This functions drives an orange LED on BS-Connect board
   *
   * @param enable : if true the orange LED with be on
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setBSSwHbeat(bool enable);

  /**
   * This function checks whether or not the orange LED on the BS-Connect board
   * is on.
   *
   * @param enable : if true the orange LED with be on
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getBSSwHbeat(bool & enable);

  /**
   * This function handles the start trigger procedure, using the NEURAL_START.
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setNeuralStart();

  /**
   * This function gets the value of NEURAL_START.
   *
   * @param value : NEURAL_START value
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getNeuralStart(bool & value);


  /**
   * This function configures the trigger mode.
   *
   * @param drive_sync_ext_start : if true, SYNC_EXT_START will be driven by the
   * FPGA
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_triggerMode(bool drive_sync_ext_start);

  /**
   * This function checks the trigger mode.
   *
   * @param drive_sync_ext_start : if true, SYNC_EXT_START will be driven by the
   * FPGA
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getTriggerMode(bool & drive_sync_ext_start);

  /**
   * This function sets the value of SYNC_EXT_START.
   *
   * @param value : the value to drive on SYNC_EXT_START pin
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setSyncExtStart(bool value);

  /**
   * This function gets the value of SYNC_EXT_START.
   *
   * @param value : the value to drive on SYNC_EXT_START pin
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getSyncExtStart(bool & value);

  /**
   * This function gets the version number of the basestation connect board.
   *
   * @param version_major : the BaseStation Connect board version number
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getBSVersion(unsigned char & version_major);

  /**
   * This function gets the revision number of the basestation connect board.
   *
   * @param version_minor : the BaseStation Connect board revision number
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getBSRevision(unsigned char & version_minor);

  /**
   * This function controls the power of the BaseStation Connect board.
   *
   * @param enable : power enable
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_supplyBS(bool enable);

  /**
   * This function checks whether or not the power of the BaseStation Connect
   * board is enabled.
   *
   * @param enable : power is enabled when true, disabled when false
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getSupplyBS(bool & enable);

  /**
   * This function enables / disables data zeroing in the basestation fpga.
   *
   * @param enable : false = data zeroing disabled, true = data zeroing enabled
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setDataZeroing(bool enable);

  /**
   * This function checks whether or not data zeroing is enabled in the
   * basestation fpga.
   *
   * @param enabled : false when data zeroing is disabled, true when enabled
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getDataZeroing(bool & enabled);

  /**
   * This function enables / disables lock checking in the basestation fpga.
   *
   * @param enable : false = lock checking disabled, true = lock checking enabled
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setLockChecking(bool enable);

  /**
   * This function checks whether or not lock checking is enabled in the
   * basestation fpga.
   *
   * @param enable : false when lock checking is disabled, true when enabled
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_getLockChecking(bool & enabled);

  /**
   * This function enables / disables the datapath bypass for the raw mode.
   * In this mode the inputs of the bs fpga that come from the deserializer are
   * directly sent to the SDRAM buffer.
   *
   * @param b : false = bypass disable (normal mode), true = bypass enabled (
   *            raw mode)
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode neuropix_setBypassDatapath(bool b);

  /**
   * This function allows to read data from the tcp/ip link without processing it.
   *
   * @param packetsize : number of bytes
   *
   * @return the ConnectionLinkPacket
   */
  ConnectionLinkPacket neuropix_readUnprocessed(unsigned int packetsize);

  /**
   * SHOULD BE PRIVATE! PUBLIC FOR DEBUG
   * This function sets the shift select to the given value and writes the given
   * chain over uart to the given shift register, using the algorithm for
   * writing a shift register (write twice with read back in order to check).
   *
   * @param shift_select : shift register selection
   *                       (0 for base configuration shift register,
   *                        1 for shank configuration shift register,
   *                        2 for test configuration shift register)
   * @param chain        : the chain to write
   * @param read_check   : if enabled, for base/shank register only, read shift
   *                       register to check
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_writeShiftRegister(unsigned char shift_select,
                                                  std::vector<bool> & chain,
                                                  bool read_check=true);

  /**
   * SHOULD BE PRIVATE! PUBLIC FOR DEBUG
   * This function reads the chain from the wanted shift register, using the
   * algorithm for reading a shift register (read and write back).
   *
   * @param shift_select : shift register selection
   * @param chain        : the chain to return
   * @param shift_length : the length of the shift register that will be read
   *
   * @return SHIFTREG_SUCCESS if successful
   */
  ShiftRegisterAccessErrorCode neuropix_readShiftRegister(unsigned char shift_select,
                                                 std::vector<bool> & chain,
                                                 unsigned int shift_length);

  /**
   * This function handles writing to the EEPROM.
   *
   * @param address : the address to write to
   * @param byte    : the byte to write
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_writeEeprom(short int address, unsigned char byte);

  /**
   * This function handles reading from the EEPROM.
   *
   * @param address : the address to read from
   * @param byte    : the byte that was read to return
   *
   * @return EEPROM_SUCCESS if successful
   */
  EepromErrorCode neuropix_readEeprom(short int address, unsigned char & byte);

  /**
   * start logging information in api_log.txt
   */
  void neuropix_startLog();

protected:

  /**
   * This function organises a burst of uart accesses, controlled by a vector of
   * commands.
   *
   * @param commands : the vector of field_command, with the commands that
   *                   control the uart.
   * @param readdata : the vector of data bytes that are read over uart.
   *
   * @return error of UartErrorCode (the first error that was flagged)
   */
  virtual UartErrorCode uartBurst(std::vector<field_command> & commands,
                                  std::vector<unsigned char> & readdata);

  NeuropixConnectionLinkIntf * tcpConnectionLink_;
  VciInterface * vciIntf_;
  neuroprobe * datamodel_;

  std::vector<struct adcPairCommon> adcPairCommonCalibration_;
  std::vector<struct adcComp> adcCompCalibration_;
  std::vector<unsigned short> gainCorrectionCalibration_;
  std::vector<int> impedanceMeasurement_;
  bool adcSlopeCalibrationRun_;
  bool adcOffsetCalibrationRun_;
  bool comparatorCalibrationRun_;
  bool gainCalibrationRun_;

  // should be private, moved for tests
  AsicID asicId_;
private:

  // settings
  bool electrodeMode_; // false is ADC mode

  unsigned char fakeEeprom_[2048]; // used when dummyMode_ is set (playback)
  bool dummyMode_; // when in playback, shift reg accesses are bypassed, and read/write UART
  std::ofstream logFile_;

  // to avoid many alloc/free, allocate only once
  float ** slopesPerSlopecalAdc_;
  double * time_;
  float * electrodedata_;

  std::map<std::string, unsigned short> DACTable_;

  std::string ip_address;

  /**
   * This function handles the startup configuration steps for adc calibration.
   *
   * @return CALIB_SUCCESS if successful
   */
  CalibErrorCode neuropix_calibrationStartupConfiguration();

  /**
   * This function sets NEURAL_START
   *
   * @param value : the value to set on NEURAL_START
   *
   * @return CONFIG_ERROR_NO_LINK if no datalink,
   *         CONFIG_SUCCESS if successful
   */
  ConfigAccessErrorCode setNeuralStart(bool value);

  /**
   * This function writes the calibrated slope, coarse and fine values for each
   * ADC pair to a csv file, with the ADC slope that resulted from the
   * calibration calculations.
   *
   * @param adcSlopes : the slopes per ADC that resulted in the best combined
   *                    distances
   * @param fileName  : the name of the file to save to
   */
  void writeSlopeCalibrationToCsv(float adcSlopes[32], std::string fileName);

  /**
   * This function writes the calibrated cfix values for each ADC pair to a csv
   * file, with the ADC offset code that resulted from the calibration
   * calculations.
   *
   * @param adcPairOutputCode : the digital output codes per ADC pair that
   *                            resulted in the optimal cfix values
   * @param fileName          : the name of the file to save to
   */
  void writeOffsetCalibrationToCsv(float adcPairOutput[32],
                                   std::string fileName);

  /**
   * This function writes the calibrated comparator CompP and CompN values to a
   * csv file, with the corresponding percentage of ones that resulted from the
   * calibration calculations.
   *
   * @param onesPerAdc : the percentage of ones that resulted in the optimal
   *                     comparator values
   * @param fileName   : the name of the file to save to
   */
  void writeComparatorCalibrationToCsv(double onesPerAdc[32],
                                       std::string fileName);

  /**
   * This function writes the calibrated gain correction factors to a csv file,
   * with the corresponding signal amplitudes that resulted from the calibration
   * calculations.
   *
   * @param amplitudesPerElectrode : the signal amplitudes that resulted in the
   *                                 optimal gain correction factors
   * @param fileName               : the name of the file to save to
   */
  void writeGainCalibrationToCsv(std::vector<double> amplitudesPerElectrode,
                                 std::string fileName);

  /**
   * This function writes the measured impedances for each electrode to a csv
   * file.
   *
   * @param fileName : the name of the file to save to
   */
  void writeImpedancesToCsv(std::string fileName);

  /**
   * This function adds the slope, coarse and fine values, as stored in the
   * calibration member, to the given (opened) filestream.
   *
   * @param filestream : the opened filestream to add the values to.
   */
  void addSlopeCalibrationValuesToFilestream(std::ofstream & filestream);

  /**
   * This function adds the cfix values as stored in the calibration member, to
   * the given (opened) filestream.
   *
   * @param filestream : the opened filestream to add the values to.
   */
  void addOffsetCalibrationValuesToFilestream(std::ofstream & filestream);

  /**
   * This function adds the compP and compN values, as stored in the calibration
   * member, to the given (opened) filestream.
   *
   * @param filestream : the opened filestream to add the values to.
   */
  void addComparatorCalibrationValuesToFilestream(std::ofstream & filestream);

  /**
   * This function adds the gain correction factors, as stored in the
   * calibration member, to the given (opened) filestream.
   *
   * @param filestream : the opened filestream to add the values to.
   */
  void addGainCalibrationValuesToFilestream(std::ofstream & filestream);

  /**
   * This function handles one string line of a csv file, and splits it into a
   * vector of strings.
   *
   * @param line   : the string line to split
   * @param vector : the string vector of the splitted cells
   */
  void splitCsvLineIntoCells(std::string line, std::vector<std::string> &
                             cells);

  /**
   * This function handles the writing of a chain to a csv file.
   *
   * @param filename : the file to write to (should be .csv)
   * @param chain    : the chain to write
   */
  void writeChainToCsv(std::string filename, std::vector<bool> & chain);

  /**
   * This function handles the reading a chain from a csv file.
   *
   * @param filename : the file to read from (should be .csv)
   * @param chain    : the chain that was read
   *
   * @return READCSV_SUCCESS if succesful
   */
  ReadCsvErrorCode readChainFromCsv(std::string filename, std::vector<bool> & chain);

  /**
   * This function resets all relevant headstage registers.
   *
   * @return UART_SUCCESS if succesful
   */
  UartErrorCode resetHeadstageRegisters();

  /**
   * This function sets up a configuration link.
   *
   * @return SUCCESS if succesful
   */
  ErrorCode setupConfigLink();

  /**
   * flush DRAM fifo / data connection
   */
  virtual ErrorCode neuropix_flushData();

  /**
   * This function skips the chosen amount of adc data.
   *
   * @param numberOfPackets : the number of adc packets to skip
   *
   * @return READ_SUCCESS if succesful
   */
  virtual ReadErrorCode skipAdcData(unsigned int numberOfPackets);

  /**
   * This function skips the chosen amount of electrode data.
   *
   * @param numberOfPackets : the number of electrode packets to skip
   *
   * @return READ_SUCCESS if succesful
   */
  virtual ReadErrorCode skipElectrodeData(unsigned int numberOfPackets);

  // method to wait x milliseconds
  void neuropix_milliSleep(unsigned int milliseconds);

  /**
   * This function makes a field_command for UART burst
   *
   * @return field_command structure
   */
  field_command makeFieldCommand(bool read_not_write,
                                 unsigned char device,
                                 unsigned char address,
                                 unsigned char data = 0);
};


#endif

