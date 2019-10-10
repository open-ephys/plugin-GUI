#ifndef hs_fpga_register_def_h
#define hs_fpga_register_def_h

// Headstage Board Version Register
const unsigned char HS_BRD_VER_ADDR          = 0x00;
const unsigned char HS_BRD_VER_VERSION_POS   = 4;
const unsigned char HS_BRD_VER_VERSION_MASK  = 0xf0;
const unsigned char HS_BRD_VER_REVISION_POS  = 0;
const unsigned char HS_BRD_VER_REVISION_MASK = 0x0f;

// Headstage FPGA Version Register
const unsigned char HS_FPGA_VER_ADDR          = 0x01;
const unsigned char HS_FPGA_VER_VERSION_POS   = 4;
const unsigned char HS_FPGA_VER_VERSION_MASK  = 0xf0;
const unsigned char HS_FPGA_VER_REVISION_POS  = 0;
const unsigned char HS_FPGA_VER_REVISION_MASK = 0x0f;

// Headstage General Control Register #1
const unsigned char HS_GEN_CTRL1_ADDR              = 0x10;
const unsigned char HS_GEN_CTRL1_LED_OFF_POS       = 7;
const unsigned char HS_GEN_CTRL1_LED_OFF_MASK      = 0x80;
const unsigned char HS_GEN_CTRL1_SPI_LINE_SEL_POS  = 5;
const unsigned char HS_GEN_CTRL1_SPI_LINE_SEL_MASK = 0x60;
const unsigned char HS_GEN_CTRL1_TE_POS            = 4;
const unsigned char HS_GEN_CTRL1_TE_MASK           = 0x10;
const unsigned char HS_GEN_CTRL1_BIST_RX_POS       = 3;
const unsigned char HS_GEN_CTRL1_BIST_RX_MASK      = 0x08;
const unsigned char HS_GEN_CTRL1_BIST_TX_POS       = 2;
const unsigned char HS_GEN_CTRL1_BIST_TX_MASK      = 0x04;
const unsigned char HS_GEN_CTRL1_ND_ENABLE_POS     = 1;
const unsigned char HS_GEN_CTRL1_ND_ENABLE_MASK    = 0x02;
const unsigned char HS_GEN_CTRL1_SW_HBEAT_POS      = 0;
const unsigned char HS_GEN_CTRL1_SW_HBEAT_MASK     = 0x01;

// Headstage General Control Register #2
const unsigned char HS_GEN_CTRL2_ADDR                = 0x11;
const unsigned char HS_GEN_CTRL2_ASIC_MODE_POS       = 6;
const unsigned char HS_GEN_CTRL2_ASIC_MODE_MASK      = 0xc0;
const unsigned char HS_GEN_CTRL2_ASIC_PR_RESET_POS   = 5;
const unsigned char HS_GEN_CTRL2_ASIC_PR_RESET_MASK  = 0x20;
const unsigned char HS_GEN_CTRL2_ASIC_RESET_POS      = 4;
const unsigned char HS_GEN_CTRL2_ASIC_RESET_MASK     = 0x10;
const unsigned char HS_GEN_CTRL2_SER_GPIO1_CTRL_POS  = 3;
const unsigned char HS_GEN_CTRL2_SER_GPIO1_CTRL_MASK = 0x08;
const unsigned char HS_GEN_CTRL2_EEPROM_EN_POS       = 2;
const unsigned char HS_GEN_CTRL2_EEPROM_EN_MASK      = 0x04;
const unsigned char HS_GEN_CTRL2_DAC_EN_POS          = 1;
const unsigned char HS_GEN_CTRL2_DAC_EN_MASK         = 0x02;
const unsigned char HS_GEN_CTRL2_HS_RESET_POS        = 0;
const unsigned char HS_GEN_CTRL2_HS_RESET_MASK       = 0x01;

// Headstage General Status Register
const unsigned char HS_GEN_STAT_ADDR                = 0x20;
const unsigned char HS_GEN_STAT_SER_GPIO1_STAT_POS  = 1;
const unsigned char HS_GEN_STAT_SER_GPIO1_STAT_MASK = 0x02;
const unsigned char HS_GEN_STAT_SER_GPO_STAT_POS    = 0;
const unsigned char HS_GEN_STAT_SER_GPO_STAT_MASK   = 0x01;

// Headstage BIST Error Register #1
const unsigned char HS_BIST_ERR1_ADDR               = 0x30;
const unsigned char HS_BIST_ERR1_BIST_TX_ERROR_POS  = 0;
const unsigned char HS_BIST_ERR1_BIST_TX_ERROR_MASK = 0x07;

// Headstage BIST Error Register #2
const unsigned char HS_BIST_ERR2_ADDR              = 0x31;
const unsigned char HS_BIST_ERR2_SPI_ADC_ERR_POS   = 2;
const unsigned char HS_BIST_ERR2_SPI_ADC_ERR_MASK  = 0x04;
const unsigned char HS_BIST_ERR2_SPI_CTR_ERR_POS   = 1;
const unsigned char HS_BIST_ERR2_SPI_CTR_ERR_MASK  = 0x02;
const unsigned char HS_BIST_ERR2_SPI_SYNC_ERR_POS  = 0;
const unsigned char HS_BIST_ERR2_SPI_SYNC_ERR_MASK = 0x01;

// Headstage BIST Error Register #3
const unsigned char HS_BIST_ERR3_ADDR              = 0x32;
const unsigned char HS_BIST_ERR3_SPI_ERR_MODE_POS  = 7;
const unsigned char HS_BIST_ERR3_SPI_ERR_MODE_MASK = 0x80;
const unsigned char HS_BIST_ERR3_SPI_WORD_SEL_POS  = 0;
const unsigned char HS_BIST_ERR3_SPI_WORD_SEL_MASK = 0x0f;

// Headstage BIST Error Register #4
const unsigned char HS_BIST_ERR4_ADDR              = 0x33;
const unsigned char HS_BIST_ERR4_SPI_TX_ERROR_POS  = 0;
const unsigned char HS_BIST_ERR4_SPI_TX_ERROR_MASK = 0x0f;

// Headstage DAC Control Register #1
const unsigned char HS_DAC_CTRL1_ADDR               = 0x40;
const unsigned char HS_DAC_CTRL1_DAC_AMPLITUDE_POS  = 4;
const unsigned char HS_DAC_CTRL1_DAC_AMPLITUDE_MASK = 0xf0;
const unsigned char HS_DAC_CTRL1_DAC_PATTERN_POS    = 2;
const unsigned char HS_DAC_CTRL1_DAC_PATTERN_MASK   = 0x0c;
const unsigned char HS_DAC_CTRL1_DAC_PATTERN_DC     = 0;
const unsigned char HS_DAC_CTRL1_DAC_PATTERN_SINE   = 1;
const unsigned char HS_DAC_CTRL1_DAC_PATTERN_SINE_OFFSET = 2;
const unsigned char HS_DAC_CTRL1_DAC_CH_SEL_POS     = 0;
const unsigned char HS_DAC_CTRL1_DAC_CH_SEL_MASK    = 0x03;

// Headstage DAC Control Register #2
const unsigned char HS_DAC_CTRL2_ADDR            = 0x41;
const unsigned char HS_DAC_CTRL2_DAC_DC_MSB_POS  = 0;
const unsigned char HS_DAC_CTRL2_DAC_DC_MSB_MASK = 0xff;

// Headstage DAC Control Register #3
const unsigned char HS_DAC_CTRL3_ADDR            = 0x42;
const unsigned char HS_DAC_CTRL3_DAC_DC_LSB_POS  = 0;
const unsigned char HS_DAC_CTRL3_DAC_DC_LSB_MASK = 0xff;

// Headstage DAC Control Register #4
const unsigned char HS_DAC_CTRL4_ADDR               = 0x43;
const unsigned char HS_DAC_CTRL4_DAC_FREQUENCY_POS  = 0;
const unsigned char HS_DAC_CTRL4_DAC_FREQUENCY_MASK = 0x0f;

// Headstage Configuration Shift Control Register #1
const unsigned char HS_CONF_SC1_ADDR        = 0x50;
const unsigned char HS_CONF_SC1_SR_SEL_POS  = 0;
const unsigned char HS_CONF_SC1_SR_SEL_MASK = 0x03;
const unsigned char HS_CONF_SC1_SR_SEL_BASE = 0;
const unsigned char HS_CONF_SC1_SR_SEL_SHANK = 1;
const unsigned char HS_CONF_SC1_SR_SEL_TEST = 2;

// Headstage Configuration Shift Control Register #2
const unsigned char HS_CONF_SC2_ADDR          = 0x51;
const unsigned char HS_CONF_SC2_TRANSMIT_POS  = 0;
const unsigned char HS_CONF_SC2_TRANSMIT_MASK = 0xff;

// Headstage Configuration Shift Control Register #3
const unsigned char HS_CONF_SC3_ADDR         = 0x52;
const unsigned char HS_CONF_SC3_RECEIVE_POS  = 0;
const unsigned char HS_CONF_SC3_RECEIVE_MASK = 0xff;

// Headstage EEPROM General Register
const unsigned char HS_EEPROM_GEN_ADDR           = 0x60;
const unsigned char HS_EEPROM_GEN_START_SEQ_POS  = 0;
const unsigned char HS_EEPROM_GEN_START_SEQ_MASK = 0x01;

// Headstage EEPROM Error/Status Register
const unsigned char HS_EEPROM_ERR_ADDR         = 0x61;
const unsigned char HS_EEPROM_ERR_ACK_SEQ_POS  = 4;
const unsigned char HS_EEPROM_ERR_ACK_SEQ_MASK = 0x10;
const unsigned char HS_EEPROM_ERR_ACK_ERR_POS  = 0;
const unsigned char HS_EEPROM_ERR_ACK_ERR_MASK = 0x01;

// Headstage EEPROM Command Register
const unsigned char HS_EEPROM_CMD_ADDR         = 0x62;
const unsigned char HS_EEPROM_CMD_COMMAND_POS  = 0;
const unsigned char HS_EEPROM_CMD_COMMAND_MASK = 0xff;
const unsigned char HS_EEPROM_CMD_READ         = 0x03;
const unsigned char HS_EEPROM_CMD_WRITE        = 0x6c;
const unsigned char HS_EEPROM_CMD_WREN         = 0x96;

// Headstage EEPROM Write Register
const unsigned char HS_EEPROM_WR_ADDR            = 0x63;
const unsigned char HS_EEPROM_WR_WRITE_DATA_POS  = 0;
const unsigned char HS_EEPROM_WR_WRITE_DATA_MASK = 0xff;

// Headstage EEPROM Read Register
const unsigned char HS_EEPROM_RD_ADDR           = 0x64;
const unsigned char HS_EEPROM_RD_READ_DATA_POS  = 0;
const unsigned char HS_EEPROM_RD_READ_DATA_MASK = 0xff;

// Headstage EEPROM Device Address Register
const unsigned char HS_EEPROM_DEV_ADDR                = 0x65;
const unsigned char HS_EEPROM_DEV_DEVICE_ADDRESS_POS  = 0;
const unsigned char HS_EEPROM_DEV_DEVICE_ADDRESS_MASK = 0xff;
const unsigned char HS_EEPROM_DEV_VALUE               = 0xA0;

// Headstage EEPROM Word Address Register #1
const unsigned char HS_EEPROM_WA1_ADDR        = 0x66;
const unsigned char HS_EEPROM_WA1_WA_MSB_POS  = 0;
const unsigned char HS_EEPROM_WA1_WA_MSB_MASK = 0xff;

// Headstage EEPROM Word Address Register #2
const unsigned char HS_EEPROM_WA2_ADDR        = 0x67;
const unsigned char HS_EEPROM_WA2_WA_LSB_POS  = 0;
const unsigned char HS_EEPROM_WA2_WA_LSB_MASK = 0xff;

#endif
