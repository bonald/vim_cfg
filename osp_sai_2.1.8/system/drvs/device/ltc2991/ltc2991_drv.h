/****************************************************************************
* $Id$
*  The header file of the ltc2991 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2014-01-26 
* Reason        : First Create.
****************************************************************************/
#ifndef _LTC2991_DRV__
#define _LTC2991_DRV__

#include "ctc_i2c.h"

#define boolean uint8

typedef enum
{
    LTC2991_I2C = 0,        /* access the temperature ltc2991 by I2C bus */
} ltc2991_io_type_e;

/* define the structure including i2c operation paramaters */
struct ltc2991_access_s{
    uint8 reg;         /* the type of the temperature ltc2991 register */     
    uint8* val;              /* pointer of the value buffur */
    uint8 len;
};
typedef struct ltc2991_access_s ltc2991_access_t;

typedef struct ltc2991_io_handle_s ltc2991_io_handle_t;
struct ltc2991_io_handle_s 
{
    int32 (*close)(ltc2991_io_handle_t *);
    int32 (*read)(const ltc2991_io_handle_t *, ltc2991_access_t *);
    int32 (*write)(const ltc2991_io_handle_t *, ltc2991_access_t *);
    void* handle;      /*handle of the low level operations */
};

ltc2991_io_handle_t *ltc2991_io_register(ltc2991_io_type_e type, const void *p_data);

extern const float32 LTC2991_SINGLE_ENDED_lsb;
//! Typical differential LSB weight in volts
extern const float32 LTC2991_DIFFERENTIAL_lsb;
//! Typical VCC LSB weight in volts
extern const float32 LTC2991_VCC_lsb;
//! Typical temperature LSB weight in degrees Celsius (and Kelvin).
//! Used for internal temperature as well as remote diode temperature measurements.
extern const float32 LTC2991_TEMPERATURE_lsb;
//! Typical remote diode LSB weight in volts.
//! Used to readback diode voltage when in temperature measurement mode.
extern const float32 LTC2991_DIODE_VOLTAGE_lsb;

/*! @name I2C_Addresses
@{ */

//! I2C address of the LTC2991.
//! Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
//! Uncomment LTC2991_I2C_ADDRESS to match demo board configuration.
//  Address assignment
// LTC2991 I2C Address                //  AD2       AD1       AD0
//#define LTC2991_I2C_ADDRESS 0x48      //  Low       Low       Low
// #define LTC2991_I2C_ADDRESS 0x49    //  Low       Low       High
// #define LTC2991_I2C_ADDRESS 0x4A    //  Low       High      Low
// #define LTC2991_I2C_ADDRESS 0x4B    //  Low       High      High
 #define LTC2991_I2C_ADDRESS 0x4C    //  High      Low       Low
// #define LTC2991_I2C_ADDRESS 0x4D    //  High      Low       High
// #define LTC2991_I2C_ADDRESS 0x4E    //  High      High      Low
// #define LTC2991_I2C_ADDRESS 0x4F    //  High      High      High

//! LTC2991 Global I2C Address.
#define LTC2991_I2C_GLOBAL_ADDRESS 0x77  //  Global Address

/*! @} */
/*! @name REGISTERS
@{ */

#define LTC2991_STATUS_LOW_REG              0x00    //!< Data_Valid Bits(V1 Through V8)
#define LTC2991_CHANNEL_ENABLE_REG          0x01    //!< Channel Enable, Vcc, T_internal Conversion Status, Trigger
#define LTC2991_CONTROL_V1234_REG           0x06    //!< V1, V2, V3, and V4 Control Register
#define LTC2991_CONTROL_V5678_REG           0x07    //!< V5, V6, V7, AND V8 Control Register
#define LTC2991_CONTROL_PWM_Tinternal_REG   0x08    //!< PWM Threshold and T_internal Control Register
#define LTC2991_PWM_THRESHOLD_MSB_REG       0x09    //!< PWM Threshold
#define LTC2991_V1_MSB_REG                  0x0A    //!< V1, or T_R1 T MSB
#define LTC2991_V1_LSB_REG                  0x0B    //!< V1, or T_R1 T LSB
#define LTC2991_V2_MSB_REG                  0x0C    //!< V2, V1-V2, or T_R2 Voltage MSB
#define LTC2991_V2_LSB_REG                  0x0D    //!< V2, V1-V2, or T_R2 Voltage LSB
#define LTC2991_V3_MSB_REG                  0x0E    //!< V3, or T_R2 T MSB
#define LTC2991_V3_LSB_REG                  0x0F    //!< V3, or T_R2 T LSB
#define LTC2991_V4_MSB_REG                  0x10    //!< V4, V3-V4, or T_R2 Voltage MSB
#define LTC2991_V4_LSB_REG                  0x11    //!< V4, V3-V4, or T_R2 Voltage LSB
#define LTC2991_V5_MSB_REG                  0x12    //!< V5, or T_R3 T MSB
#define LTC2991_V5_LSB_REG                  0x13    //!< V5, or T_R3 T LSB
#define LTC2991_V6_MSB_REG                  0x14    //!< V6, V5-V6, or T_R3 Voltage MSB
#define LTC2991_V6_LSB_REG                  0x15    //!< V6, V5-V6, or T_R3 Voltage LSB
#define LTC2991_V7_MSB_REG                  0x16    //!< V7, or T_R4 T MSB
#define LTC2991_V7_LSB_REG                  0x17    //!< V7, or T_R4 T LSB
#define LTC2991_V8_MSB_REG                  0x18    //!< V8, V7-V8, or T_R4 Voltage MSB
#define LTC2991_V8_LSB_REG                  0x19    //!< V8, V7-V8, or T_R4 Voltage LSB
#define LTC2991_T_Internal_MSB_REG          0x1A    //!< T_Internal MSB
#define LTC2991_T_Internal_LSB_REG          0x1B    //!< T_Internal LSB
#define LTC2991_Vcc_MSB_REG                 0x1C    //!< Vcc MSB
#define LTC2991_Vcc_LSB_REG                 0x1D    //!< Vcc LSB

/*! @} */
/*! @name LTC2991_CHANNEL_ENABLE_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CHANNEL_ENABLE_REG to configure settings.
    Bitwise AND with value read from LTC2991_CHANNEL_ENABLE_REG to determine present setting.
@{ */

#define LTC2991_V7_V8_TR4_ENABLE              0x80  //!< Enable V7-V8 measurements, including TR4 temperature
#define LTC2991_V5_V6_TR3_ENABLE              0x40  //!< Enable V5-V6 measurements, including TR3 temperature
#define LTC2991_V3_V4_TR2_ENABLE              0x20  //!< Enable V3-V4 measurements, including TR2 temperature
#define LTC2991_V1_V2_TR1_ENABLE              0x10  //!< Enable V1-V2 measurements, including TR1 temperature
#define LTC2991_VCC_TINTERNAL_ENABLE          0x08  //!< Enable Vcc internal voltage measurement
#define LTC2991_ENABLE_ALL_CHANNELS           0xF8  //!< Use to enable all LTC2991 channels.  Equivalent to bitwise OR'ing all channel enables.
#define LTC2991_BUSY                          0x04  //!< LTC2991 Busy Bit

/*! @} */
/*! @name LTC2991_CONTROL_V1234_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_V1234_REGG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_V1234_REG to determine present setting.
@{ */

#define LTC2991_V3_V4_FILTER_ENABLE           0x80 //!< Enable filters on V3-V4
#define LTC2991_V3_V4_KELVIN_ENABLE           0x40 //!< Enable V3-V4 for Kelvin. Otherwise, Celsius.
#define LTC2991_V3_V4_TEMP_ENABLE             0x20 //!< Enable V3-V4 temperature mode.
#define LTC2991_V3_V4_DIFFERENTIAL_ENABLE     0x10 //!< Enable V3-V4 differential mode.  Otherwise, single-ended.
#define LTC2991_V1_V2_FILTER_ENABLE           0x08 //!< Enable filters on V1-V2
#define LTC2991_V1_V2_KELVIN_ENABLE           0x04 //!< Enable V1-V2 for Kelvin. Otherwise, Celsius.
#define LTC2991_V1_V2_TEMP_ENABLE             0x02 //!< Enable V1-V2 temperature mode.
#define LTC2991_V1_V2_DIFFERENTIAL_ENABLE     0x01 //!< Enable V1-V2 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_V5678_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_V5678_REG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_V5678_REG to determine present setting.
@{ */

#define LTC2991_V7_V8_FILTER_ENABLE           0x80 //!< Enable filters on V7-V8
#define LTC2991_V7_V8_KELVIN_ENABLE           0x40 //!< Enable V7-V8 for Kelvin. Otherwise, Celsius.
#define LTC2991_V7_V8_TEMP_ENABLE             0x20 //!< Enable V7-V8 temperature mode.
#define LTC2991_V7_V8_DIFFERENTIAL_ENABLE     0x10 //!< Enable V7-V8 differential mode.  Otherwise, single-ended.
#define LTC2991_V5_V6_FILTER_ENABLE           0x08 //!< Enable filters on V5-V6
#define LTC2991_V5_V6_KELVIN_ENABLE           0x04 //!< Enable V5-V6 for Kelvin. Otherwise, Celsius.
#define LTC2991_V5_V6_TEMP_ENABLE             0x02 //!< Enable V5-V6 temperature mode.
#define LTC2991_V5_V6_DIFFERENTIAL_ENABLE     0x01 //!< Enable V5-V6 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_PWM_Tinternal_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_PWM_Tinternal_REG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_PWM_Tinternal_REG to determine present setting.
@{ */

#define LTC2991_PWM_0                         0x80 //!< PWM threshold Least Significant Bit
#define LTC2991_PWM_INVERT                    0x40 //!< Invert PWM
#define LTC2991_PWM_ENABLE                    0x20 //!< Enable PWM
#define LTC2991_REPEAT_MODE                   0x10 //!< Enable Repeated Aquisition Mode
#define LTC2991_INT_FILTER_ENABLE             0x08 //!< Enable Internal Temperature Filter
#define LTC2991_INT_KELVIN_ENABLE             0x04 //!< Enable internal temperature for Kelvin. Otherwise, Celsius.
/*!@} */
#if 0
//! Reads a 14-bit adc_code from LTC2991.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 ltc2991_adc_read(ltc2991_handle_t* handle,    //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                        uint8 msb_register_address,        /*!< Address of the LTC2991 MSB register to be read. This is also known as the "command byte".
                                                     Two sequential 8-bit registers are read, starting with the msb_register_address.*/
                        int16 *adc_code,      //!< returns 14-bit value read from the adc
                        int8 *data_valid      //!< returns the status of the DATA_VALID bit. *data_valid=0 indicates stale data
                       );

//! Reads a 14-bit adc_code from the LTC2991 but enforces a maximum timeout.
//! Similar to LTC2991_adc_read except it repeats until the data_valid bit is set, it fails to receive an I2C acknowledge, or the timeout (in milliseconds)
//! expires. It keeps trying to read from the LTC2991 every millisecond until the data_valid bit is set (indicating new data since the previous
//! time this register was read) or until it fails to receive an I2C acknowledge (indicating an error on the I2C bus).
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 ltc2991_adc_read_timeout(ltc2991_handle_t* handle,    //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                                uint8 msb_register_address,        /*!< Address of the LTC2991 MSB register to be read. This is also known as the "command byte".
                                                             Two sequential 8-bit registers will be read, starting with the msb_register_address.*/
                                int16 *adc_code,      //!< returns 14-bit value read from the adc
                                int8 *data_valid,     //!< returns the status of the DATA_VALID bit. *data_valid=0 indicates stale data
                                uint16 timeout        /*!< maximum timeout in millisceonds. If at any time a NACK is received the function aborts.
                                                             If the timeout is reached without valid data (*data_valid=1) the function exits.*/
                               );

//! Reads new data (even after a mode change) by flushing old data and waiting for the data_valid bit to be set.
//! This function simplifies adc reads when modes are changing.  For example, if V1-V2 changes from temperature mode
//! to differential voltage mode, the data in the register may still correspond to the temperature reading immediately
//! after the mode change.  Flushing one reading and waiting for a new reading guarantees fresh data is received.
//! If the timeout is reached without valid data (*data_valid=1) the function exits.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 ltc2991_adc_read_new_data(ltc2991_handle_t* handle,   //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                                 uint8 msb_register_address,       /*!< Address of the LTC2991 MSB register to be read. This is also known as the "command byte".
                                                             Two sequential 8-bit registers will be read, starting with the msb_register_address.*/
                                 int16 *adc_code,     //!< returns 14-bit value read from the adc
                                 int8 *data_valid,    //!< returns the status of the DATA_VALID bit. *data_valid=0 indicates stale data
                                 uint16 timeout       /*!< maximum timeout in millisceonds. If at any time a NACK is received the function aborts.
                                                             If the timeout is reached without valid data (*data_valid=1) the function exits.*/
                                );

//! Reads an 8-bit register from the LTC2991 using the standard repeated start format.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 ltc2991_register_read(ltc2991_handle_t* handle,       //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                             uint8 register_address,  //!< Address of the LTC2991 register to be read. This is also known as the "command byte".
                             uint8 *register_data     //!< returns 8-bit value read from the LTC2991 register.
                            );

//! Write one byte to an LTC2991 register.
//! Writes to an 8-bit register inside the LTC2991 using the standard I2C repeated start format.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 ltc2991_register_write(ltc2991_handle_t* handle,      //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                              uint8 register_address, //!< Address of the LTC2991 register to be overwritten.  This is also known as the "command byte".
                              uint8 register_data     //!< Value that will be written to the register.
                             );


//! Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
//! bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
int32 _ltc2991_register_set_clear_bits(ltc2991_handle_t* handle,         //!< I2C address of the LTC2991. Configured by tying the ADR0, ADR1, and ADR2 pins high or low. See Table 1 of datasheet.
                                       uint8 register_address,    //!< Address of the LTC2991 register to be modified.
                                       uint8 bits_to_set,         //!< bits_to_set will be bitwise OR'd with the register.
                                       uint8 bits_to_clear        //!< bits_to_clear will be inverted and bitwise AND'd with the register
                                      );
#endif
//! Calculates the LTC2991 single-ended input voltages
//! @return the single-ended voltage in volts
float32 ltc2991_code_to_single_ended_voltage(int16 adc_code,                //!< code read from the adc (from a function such as LTC2991_adc_read)
    float32 LTC2991_single_ended_lsb   //!< single-ended LSB weight. If not calibrated, use LTC2991_SINGLE_ENDED_LSB
                                          );

//! Calculates the LTC2991 Vcc voltage
//! @return the Vcc voltage in volts
float32 ltc2991_code_to_vcc_voltage(int16 adc_code,                 //!< code read from the adc (from a function such as LTC2991_adc_read)
                                  float32 LTC2991_single_ended_lsb    //!< Vcc LSB weight. If not calibrated, use LTC2991_VCC_LSB
                                 );

//! Calculates the LTC2991 differential input voltage.
//! @return the differential voltage in volts
float32 ltc2991_code_to_differential_voltage(int16 adc_code,                //!< code read from the adc (from a function such as LTC2991_adc_read)
    float32 LTC2991_differential_lsb   //!< differential LSB weight. If not calibrated, use LTC2991_DIFFERENTIAL_LSB
                                          );

//! Calculates the LTC2991 temperature
//! @return the temperature in degrees Celsius or degrees Kevlin (dependent on mode setting).
float32 ltc2991_temperature(int16 adc_code,                 //!< code read from the adc (from a function such as LTC2991_adc_read).
                          float32 LTC2991_temperature_lsb,     //!< temperature LSB weight. If not calibrated, use LTC2991_TEMPERATURE_LSB
                          boolean unit                     //!< The temperature unit, true for Kelvin, false for Celsius
                         );

//! Calcultates the LTC2991 diode voltage
//! @return the diode voltage in volts.
float32 ltc2991_code_to_diode_voltage(int16 adc_code,               //!< code read from the adc (from a function such as LTC2991_adc_read)
                                    float32 LTC2991_diode_voltage_lsb //!< diode voltage LSB weight. If not calibrated, use LTC2991_DIODE_VOLTAGE_LSB
                                   );
#endif 

