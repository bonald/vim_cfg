#ifndef __DIAG_CLI_H__
#define __DIAG_CLI_H__

#define CLI_GEPHY_DESC          "GE PHY"
#define CLI_XGPHY_DESC          "XG PHY"
#define CLI_FIBER_DESC          "Fiber module (SFP, XFP, etc.)"

#define CLI_READ_DESC           "Read registers"
#define CLI_WRITE_DESC          "Write registers"
#define CLI_CONFIG_PORT          "config port"

#define CLI_PORT_IDX_DESC       "Index of port"
#define CLI_ADDRESS_DESC        "Address of register"
#define CLI_COUNT_DESC           "The counts of sequential registers you need to read"

#define CLI_BYTE_VALUE_DESC     "Byte Value"
#define CLI_SHORT_VALUE_DESC    "Short Value"
#define CLI_DWORD_VALUE_DESC    "DWORD Value"

#define CLI_CLOCK_DESC          "CLOCK"
#define CLI_CLOCK_IDX_DESC      "Index of clock"

#define CLI_CHIP_IDX_DESC       "Index of chip"
#define CLI_BOARD_IDX_DESC       "Index of board"

#define CLI_EPLD_DESC          "epld"
#define CLI_EPLD_IDX_DESC       "Index of epld"

#define CLI_SENSOR_DESC       "Sensor"
#define CLI_SENSOR_IDX_DESC   "Index of sensor "
#define CLI_SENSOR_REG_DESC   "Address of sensor register"
#define CLI_SENSOR_REG_LEN_DESC   "length of sensor register"
#define CLI_SENSOR_TYPE_DESC   "type: 0-current, 1-Low, 2-High, 3-Thsyt, 4-Crit"

#define CLI_GPIO_CHIP_DESC            "Gpio chip"
#define CLI_GPIO_CHIP_IDX_DESC        "Index of gpio chip "
#define CLI_GPIO_CHIP_REG_DESC        "Address of gpio chip register"
#define CLI_GPIO_CHIP_FIBER_ID_DESC   "Fiber id"
#define CLI_GPIO_CHIP_REG_LEN_DESC    "length of gpio chip register"

#define CLI_MUX_CHIP_DESC            "Mux chip"
#define CLI_MUX_CHIP_IDX_DESC        "Index of mux chip "
#define CLI_MUX_CHIP_REG_DESC        "Address of mux chip register"
#define CLI_MUX_CHIP_REG_LEN_DESC    "length of mux chip register"

#define CLI_FAN_DESC "Fan"
#define CLI_FAN_IDX_DESC  "Index of fan module"
#define CLI_FAN_REG_DESC "Address of fan register"

#define CLI_POWER_DESC "Power"
#define CLI_POWER_IDX_DESC  "Index of power module"
#define CLI_POWER_REG_DESC "Address of power register"

#define CLI_EEPROM_DESC          "eeprom"
#define CLI_EEPROM_IDX_DESC       "Index of eeprom"

#define CLI_SYSENV_DESC          "sysenv"
#define CLI_SYSENV_IDX_DESC       "Index of sysenv"

#define CLI_VSC3308_DESC        "VSC3308"
#define CLI_VSC3308_REG_DESC    "Address of VSC3308 register"
#define CLI_VSC3308_IDX_DESC    "Index of VSC3308"

#define CLI_I2C_DESC          "I2c"

#define CLI_ACCESS_DIRECT_DESC     "directly access device "
#define CLI_ACCESS_INDIRECT_DESC   "indirectly access device "

#define CLI_PHY_CLK_SEL   "Clock selection: 0x000-serial media, 0x001-copper phy, 0x010-copper phy TCLK"
#define CLI_PHY_CLK_FREQ  "Clock freqency: 0x000-25MHz, 0x001-125MHz, 0x010-31.25MHz"
#define CLI_PHY_CLK_SQUELCH "Clock squelch: 0x00-MAX, 0x01-MED, 0x10-MIN, 0x11-NONE"
#define CLI_REG_DATA_LEN "date len"

#define CLI_LCM_BRINGUP_LOG "enable or disable lcm bringup log"
#define CLI_LCM_LOG_DEBUG "enable or disable lcm debug log"

#endif /* __DIAG_CLI_H__ */

