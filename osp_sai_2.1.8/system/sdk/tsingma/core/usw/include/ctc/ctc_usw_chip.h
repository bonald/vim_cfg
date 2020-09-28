/**
 @file ctc_usw_chip.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-8-24

 @version v2.0

 This module is deal with chip's info, and peripheral device interface
 \p
 1)chip configuration
 \p
 The most important interface for chip initialization is datapath init, which will be provided here.
  \p
 2)MDIO interface
  \p
 These interfaces are used to manage phy devices. If you want to monitor special register in Phy, use another function for auto scan,
 you must set phy/mdio and port mapping relation correctly.
  \p
 3)I2C Master interface
  \p
 These interfaces are used to access sfp/xfp devices. There are two operation types for I2C Master interface operation,
 one is read operation, and the other is write operation.
  \p
 For reading access, CPU can issue either trigger read or polling read.
  \p
 \d trigger read: read operation is triggered once by one read command
 \d polling read: this is continues read operation, which can be used to monitor sfp device state
  \p
 For writing access, CPU can only issue byte write operation.
  \p
 4)Mac Led interface
  \p
 Before using mac led function, you must set mac sequence correctly.
  \p
 5)GPIO interface
  \p
 General Purpose Input and output function interface
  \p
*/

#ifndef _CTC_USW_CHIP_H
#define _CTC_USW_CHIP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_chip.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @defgroup chip CHIP
 @{
*/

/**
 @brief Initialize the chip module and set the local chip number of the linecard

 @param[in] lchip    local chip id

 @param[in] lchip_num may be 1 or 2

 @remark[D2.TM]   Chip module init interface

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_init(uint8 lchip, uint8 lchip_num);

/**
 @brief De-Initialize chip module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the chip configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_deinit(uint8 lchip);

/**
 @brief The function is to get the local chip number of the linecard

 @param[in] lchip    local chip id

 @param[out] lchip_num  value of local chip num

 @remark[D2.TM]   To Get local chip number

 @return CTC_E_XXX
*/
extern int32
ctc_usw_get_local_chip_num(uint8 lchip, uint8* lchip_num);

/**
 @brief The function is to set chip's global chip id

 @param[in] lchip_id    local chip id

 @param[in] gchip_id  value of global chip

 @remark[D2.TM]   To Set global chip id according local chip id

 @return CTC_E_XXX

*/
extern int32
ctc_usw_set_gchip_id(uint8 lchip_id, uint8 gchip_id);

/**
 @brief The function is to get chip's global chip id

 @param[in] lchip_id    local chip id

 @param[out] gchip_id value of global chip

 @remark[D2.TM]   To Get global chip id from local chip id

 @return CTC_E_XXX

*/
extern int32
ctc_usw_get_gchip_id(uint8 lchip_id, uint8* gchip_id);

/**
 @brief The function is to set chip's global cnfig

 @param[in] lchip    local chip id

 @param[in] chip_cfg chip's global config

 @remark[D2.TM]   To Set chip module global configution

 @return CTC_E_XXX

*/
extern int32
ctc_usw_set_chip_global_cfg(uint8 lchip, ctc_chip_global_cfg_t* chip_cfg);

/**
 @brief The function is to get chip's clock

 @param[in] lchip    local chip id

 @param[out] freq frequency of the chip

 @remark[D2.TM]   To Get chip module clock

 @return CTC_E_XXX
*/
extern int32
ctc_usw_get_chip_clock(uint8 lchip, uint16* freq);

/**
 @brief control gephy scan

 @param[in] lchip    local chip id

 @param[in] p_scan_para parameter for scan control

 @remark[D2.TM]   To enable/disable special phy register for auto scan
 \p
                      to set scan interval,  set CTC_CHIP_INTERVAL_OP in op_flag
 \p
                      to set GE bitmap ctrl, set CTC_CHIP_GE_BITMAP_OP in op_flag
 \p
                      to set XG bitmap ctrl, set CTC_CHIP_XG_BITMAP_OP in op_flag
 \p
                      to set XG device link mask, set CTC_CHIP_XG_LINK_MASK_OP in op_flag
 \p
                      to set XG scan twice, set CTC_CHIP_XG_TWICE_OP in op_flag
                      this interface is just set parameter for scan, and will not trigger scan operation, which
                      will be done by interface ctc_chip_set_phy_scan_en
 \p

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);

/**
 @brief set phy auto scan enable

 @param[in] lchip    local chip id

 @param[in] enable  phy auto scan enable or disable

 @remark[D2.TM]   To enable/disable phy auto scan.The interface just set on chip0.If want to set on other chip, please
                using interface :ctc_chip_set_property

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_phy_scan_en(uint8 lchip, bool enable);

/**
 @brief read i2c device register by i2c master interface

 @param[in] lchip    local chip id

 @param[in] p_i2c_para read sfp by i2c master parameter

 @remark[D2.TM]   To read i2c device by i2c master interface
 \p
                     Notice: this interface is only for trigger read
                     parameter slave_bitmap is used to control which sfp/xfp to be accessed, so it is
                     important to set correctly according hardware, also the interface can access more
                     sfp/xfp devices at one time by setting slave_bitmap more bits.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_i2c_read(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para);

/**
 @brief write i2c device register by i2c master interface

 @param[in] lchip    local chip id

 @param[in] i2c_para write i2c device by i2c master parameter

 @remark[D2.TM]  To write i2c device by i2c master interface
 \p
                    Notice: this interface is different with trigger read interface above, for write operation
                    it can only access one i2c device at one time

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_i2c_write(uint8 lchip, ctc_chip_i2c_write_t* i2c_para);

/**
 @brief set i2c auto scan enable

 @param[in] lchip    local chip id

 @param[in] enable  i2c auto scan enable or disable

 @remark[D2.TM]   To enable/disable i2c auto scan.The interface just set on chip0.If want to set on other chip, please
                using interface :ctc_chip_set_property

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_i2c_scan_en(uint8 lchip, bool enable);

/**
 @brief control i2c scan

 @param[in] lchip    local chip id

 @param[in] p_i2c_para parameter for scan control

 @remark[D2.TM]   To enable/disable special i2c register for auto scan
 \p
                      to set scan interval,  set CTC_CHIP_SFP_INTERVAL_OP in op_flag
 \p
                      to set i2c slave bitmap ctrl, set CTC_CHIP_SFP_BITMAP_OP in op_flag
 \p
                      to set i2c reg ctrl,    set CTC_CHIP_SFP_SCAN_REG_OP in op_flag
 \p
                      this interface is just set parameter for scan, and will not trigger scan operation, which
                      will be done by interface ctc_chip_set_i2c_scan_en
 \p

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para);

/**
 @brief read i2c register buf by i2c master interface

 @param[in] lchip    local chip id

 @param[in] p_i2c_scan_read scan sfp register by i2c master parameter

 @remark[D2.TM]   To read i2c register buf by i2c master interface
 \p
                     Notice: this interface is only for scan operation
                     parameter for scan need be set by ctc_chip_set_i2c_scan_para and trigger the scan operation by
                     interface ctc_chip_set_i2c_scan_en, then you can read the scan result by this interface

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_read_i2c_buf(uint8 lchip, ctc_chip_i2c_scan_read_t* p_i2c_scan_read);

/**
 @brief set mac led parameter

 @param[in] lchip    local chip id

 @param[in] p_led_para mac led parameter

 @param[in] led_type  using one led or two leds

 @remark[D2.TM]   To set mac led parameter
  \p
                      to set led mode, set CTC_CHIP_LED_MODE_SET_OP in op_flag
  \p
                      to set led porality, set CTC_CHIP_LED_POLARITY_SET_OP in op_flag
  \p

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type);

/**
 @brief set mac led mapping

 @param[in] lchip    local chip id

 @param[in] p_led_map mac sequence mapping for led

 @remark[D2.TM]   To set mac led mapping
 \p
                     Notice:the mapping must config correctly according hardware
                     the led control signal is genrated serially by chip, so must confirm the mac in the sequence driver
                     according led, also p_mac_id parameter must be allocated enough size according mac_led_num parameter

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map);

/**
 @brief set mac led enable

 @param[in] lchip    local chip id

 @param[in] enable enable mac led function

 @remark[D2.TM]   To set mac led enable.The interface just set mac led on chip0.If want to set mac led on other chip, please
                using interface :ctc_chip_set_property

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_mac_led_en(uint8 lchip, bool enable);

/**
 @brief set gpio interface

 @param[in] lchip    local chip id

 @param[in] gpio_id which gpio should be configured.The interface just set gpio on chip0.If want to set gpio on other chip, please
                using interface :ctc_chip_set_property

 @param[in] mode gpio mode to set

 @remark[D2.TM]   To set set gpio interface

 @return CTC_E_XXX.

*/
extern int32
ctc_usw_chip_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode);

/**
 @brief set gpio output value

 @param[in] lchip    local chip id

 @param[in] gpio_id which gpio should be configured

 @param[in]  out_para this para is only used in output mode, it control the output is high or low.
                The interface just set gpio on chip0.If want to set gpio on other chip, please
                using interface :ctc_chip_set_property

 @remark[D2.TM]   To set gpio output value

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para);

/**
 @brief get gpio input value

 @param[in] lchip    local chip id

 @param[in] gpio_id which gpio should be configured

 @param[out]  in_value this para is only used in input mode, it indicate the input is high or low
                The interface just get gpio on chip0.If want to get gpio on other chip, please
                using interface :ctc_chip_set_property

 @remark[D2.TM]   To get gpio input value

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value);

/**
 @brief switch chip access type

 @param[in] lchip    local chip id

 @param[in]  access_type access type

 @remark[D2.TM]   To switch chip access type
 \p
                    default chip access type is pcie

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_access_type(uint8 lchip, ctc_chip_access_type_t access_type);

/**
 @brief The function is to dynamic switch serdes mode

 @param[in] lchip_id    local chip id

 @param[in] p_serdes_info serdes info, refer to ctc_chip_serdes_info_t

 @remark[D2.TM]   to dynamic swith serdes mode

 Notice: this function must be supported by datapath, when using this function, must confirm the datapath is supported!!!

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_set_serdes_mode(uint8 lchip_id, ctc_chip_serdes_info_t* p_serdes_info);

/**
 @brief The function is mdio interface to read phy

 @param[in] lchip_id    local chip id

 @param[in] type phy type to be access, Ge phy or Xg phy

 @param[in] p_para phy parameter

 @remark[D2.TM]   this interface is used for access phy using mdio parameter, if you want to access phy by GPORT parameter,
                please using the interface:ctc_chip_read_xgphy_reg/ctc_chip_read_gephy_reg

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_mdio_read(uint8 lchip_id, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

/**
 @brief The function is mdio interface to write phy

 @param[in] lchip_id    local chip id

 @param[in] type phy type to be access, Ge phy or Xg phy

 @param[in] p_para phy parameter

 @remark[D2.TM]   this interface is used for access phy using mdio parameter, if you want to access phy by GPORT parameter,
                please using the interface:ctc_chip_write_gephy_reg/ctc_chip_write_xgphy_reg

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_mdio_write(uint8 lchip_id, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

/**
 @brief The function is set mdio clock frequency

 @param[in] lchip    local chip id

 @param[in] type mdio type, refer to ctc_chip_mdio_type_t

 @param[in] freq for mdio clock

 @remark[D2.TM] set mdio clock

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq);

/**
 @brief The function is get mdio clock frequency

 @param[in] lchip    local chip id

 @param[in] type mdio type, refer to ctc_chip_mdio_type_t

 @param[out] p_freq for mdio clock

 @remark[D2.TM] get mdio clock

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* p_freq);

/**
 @brief Set port to mdio and port to phy mapping

 @param[in] lchip    local chip id

 @param[in] phy_mapping_para  port to mdio and port to phy mapping table

 @remark[D2.TM]   To Set port/mdio and port/phy mapping on local chip0. If want to set other chip
            please using the interface : ctc_chip_set_property
 \p
                     Notice: before use mdio interface, Must configure this mapping
 \p

 @return  CTC_E_XXX

*/
extern int32
ctc_usw_chip_set_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para);

/**
 @brief Get port to mdio and port to phy mapping

 @param[in] lchip    local chip id

 @param[out] phy_mapping_para  port to mdio and port to phy mapping table

 @remark[D2.TM]   To Get port/mdio and port/phy mapping

 @return CTC_E_XXX

*/
extern int32
ctc_usw_chip_get_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para);
/**@} end of @defgroup   */

/**
 @brief set chip property

 @param[in] lchip_id    local chip id

 @param[in] chip_prop chip property enum, refer to ctc_chip_property_t

 @param[in] p_value chip property value to set

 @remark[D2.TM] set chip property

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_set_property(uint8 lchip_id,  ctc_chip_property_t chip_prop,  void* p_value);

/**
 @brief get chip property

 @param[in] lchip_id    local chip id

 @param[in] chip_prop chip property enum, refer to ctc_chip_property_t

 @param[out] p_value chip property get value

 @remark[D2.TM] get chip property

 @return CTC_E_XXX
*/
extern int32
ctc_usw_chip_get_property(uint8 lchip_id,  ctc_chip_property_t chip_prop,  void* p_value);

/**
 @brief Get chip sensor value

 @param[in] lchip_id    local chip id

 @param[in]  type sensor type, temperature(C) or voltage(mV)

 @param[out] p_value sensor value. MSB of the p_value is symbol for temperature, 1 meas subzero.

 @remark[D2.TM] get sensor of local chip

 @return CTC_E_XXX

*/
extern int32
ctc_usw_get_chip_sensor(uint8 lchip_id, ctc_chip_sensor_type_t type, uint32* p_value);

/**
 @brief The function is to init datapath

 @param[in] lchip_id    local chip id

 @param[in] p_global_cfg datapath serdes config and core clock

 @remark[D2.TM]   Init Datapath

 @return CTC_E_XXX
*/
extern int32
ctc_usw_datapath_init(uint8 lchip_id, void* p_global_cfg);

/**
 @brief The function is to deinit datapath

 @param[in] lchip    local chip id

 @remark[D2.TM]   DeInit Datapath

 @return CTC_E_XXX
*/
extern int32
ctc_usw_datapath_deinit(uint8 lchip);
/**@} end of @defgroup   */

#ifdef __cplusplus
}
#endif

#endif

