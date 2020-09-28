/****************************************************************************
* $Id$
*  The header file of the ltc2991 api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liangf 
* Date          : 2014-01-26 
* Reason        : First Create.
****************************************************************************/
#ifndef __VCM_LTC2991_H__
#define __VCM_LTC2991_H__

#include "vcm_drv.h"

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
 #define VCM_LTC2991_I2C_ADDRESS 0x4C    //  High      Low       Low
// #define LTC2991_I2C_ADDRESS 0x4D    //  High      Low       High
// #define LTC2991_I2C_ADDRESS 0x4E    //  High      High      Low
// #define LTC2991_I2C_ADDRESS 0x4F    //  High      High      High

//! LTC2991 Global I2C Address.
#define VCM_LTC2991_I2C_GLOBAL_ADDRESS 0x77  //  Global Address

/*! @} */
/*! @name REGISTERS
@{ */

#define VCM_LTC2991_STATUS_LOW_REG              0x00    //!< Data_Valid Bits(V1 Through V8)
#define VCM_LTC2991_CHANNEL_ENABLE_REG          0x01    //!< Channel Enable, Vcc, T_internal Conversion Status, Trigger
#define VCM_LTC2991_CONTROL_V1234_REG           0x06    //!< V1, V2, V3, and V4 Control Register
#define VCM_LTC2991_CONTROL_V5678_REG           0x07    //!< V5, V6, V7, AND V8 Control Register
#define VCM_LTC2991_CONTROL_PWM_Tinternal_REG   0x08    //!< PWM Threshold and T_internal Control Register
#define VCM_LTC2991_PWM_THRESHOLD_MSB_REG       0x09    //!< PWM Threshold
#define VCM_LTC2991_V1_MSB_REG                  0x0A    //!< V1, or T_R1 T MSB
#define VCM_LTC2991_V1_LSB_REG                  0x0B    //!< V1, or T_R1 T LSB
#define VCM_LTC2991_V2_MSB_REG                  0x0C    //!< V2, V1-V2, or T_R2 Voltage MSB
#define VCM_LTC2991_V2_LSB_REG                  0x0D    //!< V2, V1-V2, or T_R2 Voltage LSB
#define VCM_LTC2991_V3_MSB_REG                  0x0E    //!< V3, or T_R2 T MSB
#define VCM_LTC2991_V3_LSB_REG                  0x0F    //!< V3, or T_R2 T LSB
#define VCM_LTC2991_V4_MSB_REG                  0x10    //!< V4, V3-V4, or T_R2 Voltage MSB
#define VCM_LTC2991_V4_LSB_REG                  0x11    //!< V4, V3-V4, or T_R2 Voltage LSB
#define VCM_LTC2991_V5_MSB_REG                  0x12    //!< V5, or T_R3 T MSB
#define VCM_LTC2991_V5_LSB_REG                  0x13    //!< V5, or T_R3 T LSB
#define VCM_LTC2991_V6_MSB_REG                  0x14    //!< V6, V5-V6, or T_R3 Voltage MSB
#define VCM_LTC2991_V6_LSB_REG                  0x15    //!< V6, V5-V6, or T_R3 Voltage LSB
#define VCM_LTC2991_V7_MSB_REG                  0x16    //!< V7, or T_R4 T MSB
#define VCM_LTC2991_V7_LSB_REG                  0x17    //!< V7, or T_R4 T LSB
#define VCM_LTC2991_V8_MSB_REG                  0x18    //!< V8, V7-V8, or T_R4 Voltage MSB
#define VCM_LTC2991_V8_LSB_REG                  0x19    //!< V8, V7-V8, or T_R4 Voltage LSB
#define VCM_LTC2991_T_Internal_MSB_REG          0x1A    //!< T_Internal MSB
#define VCM_LTC2991_T_Internal_LSB_REG          0x1B    //!< T_Internal LSB
#define VCM_LTC2991_Vcc_MSB_REG                 0x1C    //!< Vcc MSB
#define VCM_LTC2991_Vcc_LSB_REG                 0x1D    //!< Vcc LSB

/*! @} */
/*! @name LTC2991_CHANNEL_ENABLE_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CHANNEL_ENABLE_REG to configure settings.
    Bitwise AND with value read from LTC2991_CHANNEL_ENABLE_REG to determine present setting.
@{ */

#define VCM_LTC2991_V7_V8_TR4_ENABLE              0x80  //!< Enable V7-V8 measurements, including TR4 temperature
#define VCM_LTC2991_V5_V6_TR3_ENABLE              0x40  //!< Enable V5-V6 measurements, including TR3 temperature
#define VCM_LTC2991_V3_V4_TR2_ENABLE              0x20  //!< Enable V3-V4 measurements, including TR2 temperature
#define VCM_LTC2991_V1_V2_TR1_ENABLE              0x10  //!< Enable V1-V2 measurements, including TR1 temperature
#define VCM_LTC2991_VCC_TINTERNAL_ENABLE          0x08  //!< Enable Vcc internal voltage measurement
#define VCM_LTC2991_ENABLE_ALL_CHANNELS           0xF8  //!< Use to enable all LTC2991 channels.  Equivalent to bitwise OR'ing all channel enables.
#define VCM_LTC2991_BUSY                          0x04  //!< LTC2991 Busy Bit

/*! @} */
/*! @name LTC2991_CONTROL_V1234_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_V1234_REGG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_V1234_REG to determine present setting.
@{ */

#define VCM_LTC2991_V3_V4_FILTER_ENABLE           0x80 //!< Enable filters on V3-V4
#define VCM_LTC2991_V3_V4_KELVIN_ENABLE           0x40 //!< Enable V3-V4 for Kelvin. Otherwise, Celsius.
#define VCM_LTC2991_V3_V4_TEMP_ENABLE             0x20 //!< Enable V3-V4 temperature mode.
#define VCM_LTC2991_V3_V4_DIFFERENTIAL_ENABLE     0x10 //!< Enable V3-V4 differential mode.  Otherwise, single-ended.
#define VCM_LTC2991_V1_V2_FILTER_ENABLE           0x08 //!< Enable filters on V1-V2
#define VCM_LTC2991_V1_V2_KELVIN_ENABLE           0x04 //!< Enable V1-V2 for Kelvin. Otherwise, Celsius.
#define VCM_LTC2991_V1_V2_TEMP_ENABLE             0x02 //!< Enable V1-V2 temperature mode.
#define VCM_LTC2991_V1_V2_DIFFERENTIAL_ENABLE     0x01 //!< Enable V1-V2 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_V5678_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_V5678_REG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_V5678_REG to determine present setting.
@{ */

#define VCM_LTC2991_V7_V8_FILTER_ENABLE           0x80 //!< Enable filters on V7-V8
#define VCM_LTC2991_V7_V8_KELVIN_ENABLE           0x40 //!< Enable V7-V8 for Kelvin. Otherwise, Celsius.
#define VCM_LTC2991_V7_V8_TEMP_ENABLE             0x20 //!< Enable V7-V8 temperature mode.
#define VCM_LTC2991_V7_V8_DIFFERENTIAL_ENABLE     0x10 //!< Enable V7-V8 differential mode.  Otherwise, single-ended.
#define VCM_LTC2991_V5_V6_FILTER_ENABLE           0x08 //!< Enable filters on V5-V6
#define VCM_LTC2991_V5_V6_KELVIN_ENABLE           0x04 //!< Enable V5-V6 for Kelvin. Otherwise, Celsius.
#define VCM_LTC2991_V5_V6_TEMP_ENABLE             0x02 //!< Enable V5-V6 temperature mode.
#define VCM_LTC2991_V5_V6_DIFFERENTIAL_ENABLE     0x01 //!< Enable V5-V6 differential mode.  Otherwise, single-ended.

/*! @} */
/*! @name LTC2991_CONTROL_PWM_Tinternal_REG SETTINGS
    Bitwise OR settings, and write to LTC2991_CONTROL_PWM_Tinternal_REG to configure settings.
    Bitwise AND with value read from LTC2991_CONTROL_PWM_Tinternal_REG to determine present setting.
@{ */

#define VCM_LTC2991_PWM_0                         0x80 //!< PWM threshold Least Significant Bit
#define VCM_LTC2991_PWM_INVERT                    0x40 //!< Invert PWM
#define VCM_LTC2991_PWM_ENABLE                    0x20 //!< Enable PWM
#define VCM_LTC2991_REPEAT_MODE                   0x10 //!< Enable Repeated Aquisition Mode
#define VCM_LTC2991_INT_FILTER_ENABLE             0x08 //!< Enable Internal Temperature Filter
#define VCM_LTC2991_INT_KELVIN_ENABLE             0x04 //!< Enable internal temperature for Kelvin. Otherwise, Celsius.
/*!@} */

int32 vcm_ltc2991_register_driver(vcm_handle_t* hdl, vcm_info_t* vcm_info);

#endif 

