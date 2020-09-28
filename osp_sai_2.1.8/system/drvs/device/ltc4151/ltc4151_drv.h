/****************************************************************************
* $Id$
*  The header file of the ltc4151 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2014-01-26 
* Reason        : First Create.
****************************************************************************/
#ifndef _LTC4151_DRV__
#define _LTC4151_DRV__

#include "ctc_i2c.h"

typedef enum
{
    LTC4151_I2C = 0,        /* access the temperature ltc4151 by I2C bus */
} ltc4151_io_type_e;

/* define the structure including i2c operation paramaters */
struct ltc4151_access_s{
    uint8 reg;         /* the type of the temperature ltc4151 register */     
    uint8* val;              /* pointer of the value buffur */
    uint8 len;
};
typedef struct ltc4151_access_s ltc4151_access_t;

typedef struct ltc4151_io_handle_s ltc4151_io_handle_t;
struct ltc4151_io_handle_s 
{
    int32 (*close)(ltc4151_io_handle_t *);
    int32 (*read)(const ltc4151_io_handle_t *, ltc4151_access_t *);
    int32 (*write)(const ltc4151_io_handle_t *, ltc4151_access_t *);
    void* handle;      /*handle of the low level operations */
};

ltc4151_io_handle_t *ltc4151_io_register(ltc4151_io_type_e type, const void *p_data);

//! Address Choices:
//! To choose an address, comment out all options except the
//! configuration on the demo board.

/*!
| LTC4151 I2C Address | Value      |   AD1    |   AD2    |
| :------------------ | :------:   | :------: | :------: |
| LTC4151_I2C_ADDRESS | 0x67       |   High   |   Low    |
| LTC4151_I2C_ADDRESS | 0x68       |  Float   |  High    |
| LTC4151_I2C_ADDRESS | 0x69       |  High    |  High    |
| LTC4151_I2C_ADDRESS | 0x6A       |  Float   |  Float   |
| LTC4151_I2C_ADDRESS | 0x6B       |  Float   |  Low     |
| LTC4151_I2C_ADDRESS | 0x6C       |  Low     |  High    |
| LTC4151_I2C_ADDRESS | 0x6D       |  High    |  Float   |
| LTC4151_I2C_ADDRESS | 0x6E       |  Low     |  Float   |
| LTC4151_I2C_ADDRESS | 0x6F       |  Low     |  Low     |
| LTC4151_I2C_GLOBAL_ADDRESS | 0x66 |  X      |  X       |
*/

/*! @name I2C_Addresses
@{ */

// Address assignment
// LTC4151 I2C Address                 //  AD1       AD0
#define LTC4151_I2C_ADDRESS 0x67    //  High      Low
// #define LTC4151_I2C_ADDRESS 0x68    //  Float     High
// #define LTC4151_I2C_ADDRESS 0x69    //  High      High
// #define LTC4151_I2C_ADDRESS 0x6A    //  Float     Float
// #define LTC4151_I2C_ADDRESS 0x6B    //  Float     Low
// #define LTC4151_I2C_ADDRESS 0x6C    //  Low       High
// #define LTC4151_I2C_ADDRESS 0x6D    //  High      Float
// #define LTC4151_I2C_ADDRESS 0x6E    //  Low       Float
// #define LTC4151_I2C_ADDRESS 0x6F      //  Low       Low

// LTC4151 Global I2C Address.
// #define LTC4151_I2C_GLOBAL_ADDRESS 0x66  //  Global Address

//! @} */

//! Note: Register 0x07 is Reserved.
/*!
| Registers                 |   Location    | Descrition                                |
| :------------------------ | :-----------: | :---------------------------------------- |
| LTC4151_SENSE_MSB_REG     | 0x00          | ADC Current Sense Voltage Data (8 MSB)    |
| LTC4151_SENSE_LSB_REG     | 0x01          | ADC Current Sense Voltage Data (4 LSB)    |
| LTC4151_VIN_MSB_REG       | 0x02          | ADC V_IN Voltage Data (8 MSB)             |
| LTC4151_VIN_LSB_REG       | 0x03          | ADC V_IN Voltage Data (4 LSB)             |
| LTC4151_ADIN_MSB_REG      | 0x04          | ADC ADIN Voltage Data (8 MSB)             |
| LTC4151_ADIN_LSB_REG      | 0x05          | ADC ADIN Voltage Data (4 LSB)             |
| LTC4151_CONTROL_REG       | 0x06          | Controls ADC Operation Mode and Test Mode |

*/

/*! @name REGISTERS
@{ */
// Registers
#define LTC4151_SENSE_MSB_REG       0x00  // ADC Current Sense Voltage Data (8 MSB)
#define LTC4151_SENSE_LSB_REG       0x01  // ADC Current Sense Voltage Data (4 LSB)
#define LTC4151_VIN_MSB_REG         0x02  // ADC V_IN Voltage Data (8 MSB) 
#define LTC4151_VIN_LSB_REG         0x03  // ADC V_IN Voltage Data (4 LSB)
#define LTC4151_ADIN_MSB_REG        0x04  // ADC ADIN Voltage Data (8 MSB)
#define LTC4151_ADIN_LSB_REG        0x05  // ADC ADIN Voltage Data (4 LSB)
#define LTC4151_CONTROL_REG         0x06  // Controls ADC Operation Mode and Test Mode
//!@} */

//! Bitwise AND  with channel register, and write to LTC4151_CONTROL_REG.
/*!
| Commands and Channel Registers    |   Value   |   Descrition                          |
| :----------------------------     |   :---:   |   :---------                          |
| LTC4151_CONTINUOUS_MODE           |   0x00    |   Sets LTC4151 in Continuous Mode     |
| LTC4151_SNAPSHOT_MODE             |   0x80    |   Sets LTC4151 in Snapshot Mode       |
| LTC4151_SENSE_CHANNEL_REG         |   0x00    |   ADC Sense Channel For Snapshot Mode |
| LTC4151_VIN_CHANNEL_REG           |   0x20    |   DC V_IN Channel For Snapshot Mode   |
| LTC4151_ADIN_CHANNEL_REG          |   0x40    |   ADC ADIN Channel For Snapshot Mode  |
*/

/*! @name LTC4151 Commands
@{ */

#define LTC4151_SENSE_CHANNEL_REG   0x00  // ADC Sense Channel For Snapshot Mode
#define LTC4151_VIN_CHANNEL_REG     0x20  // ADC V_IN Channel For Snapshot Mode
#define LTC4151_ADIN_CHANNEL_REG    0x40  // ADC ADIN Channel For Snapshot Mode

#define LTC4151_CONTINUOUS_MODE 0x00
#define LTC4151_SNAPSHOT_MODE   0x80
//! @} */
#if 0
//! Write one byte to an LTC4151 register.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int8 ltc4151_write(ltc4151_handle_t* handle,   //!< I2C address of the LTC4151. Configured by tying the ADR0 and ADR1 pins high or low. See Table 1 of datasheet.
                     uint8 adc_command,   //!< The "command byte" for the LTC4151.
                     uint8 code           //!< Value that will be written to the register.
                    );

//! Reads a 12-bit value from LTC4151
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int8 ltc4151_read_12_bits(ltc4151_handle_t* handle,    //!< I2C address of the LTC4151. Configured by tying the ADR0 and ADR1 pins high or low. See Table 1 of datasheet.
                            uint8 adc_command,    //!< The "command byte" for the LTC4151.
                            uint16 *adc_code      //!< Returns 12-bit value read from the adc
                           );
#endif
//! Calculates the LTC4151 sense current in Amps given "resistor" value in ohms and "LTC4151_sense_lsb" LSB weight in volts.
//! @return Returns the current in Amperes.
float32 ltc4151_code_to_sense_current(uint16 adc_code,      //!< Code read from the adc (from a function such as LTC4151_read_12_bits)
                                    float32 resistor,         //!< The sense resistor value
                                    float32 LTC4151_sense_lsb //!< Current LSB weight.
                                   );

//! Calculates the LTC4151 V_IN voltage given "LTC_vin_lsb" LSB weight in volts
//! @return the V_IN voltage in volts.
float32 ltc4151_code_to_vin_voltage(uint16 adc_code,        //!< Code read from the adc (from a function such as LTC4151_read_12_bits)
                                  float32 LTC4151_vin_lsb     //!< Voltage LSB weight.
                                 );

//! Calculates the LTC4151 ADIN voltage in mV given "LTC4151_adin_lsb" LSB weight in mV
//! @return the ADIN voltage in volts.
float32 ltc4151_code_to_ADIN_voltage(uint16 adc_code,       //!< Code read from the adc (from a function such as LTC4151_read_12_bits)
                                   float32 LTC4151_adin_lsb   //!< ADIN voltage LSB weight.
                                  );

#endif 

