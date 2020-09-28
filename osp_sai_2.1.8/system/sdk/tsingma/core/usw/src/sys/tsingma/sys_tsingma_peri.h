/**
 @file sys_tsingma_peri.h

 @date 2018-03-06

 @version v1.0

 The file contains all chip related APIs of sys layer
*/

#ifndef _SYS_TSINGMA_PERI_H
#define _SYS_TSINGMA_PERI_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
 /*#include "sal.h"*/
 /*#include "ctc_const.h"*/
 /*#include "ctc_chip.h"*/
 /*#include "ctc_debug.h"*/
 /*#include "sys_usw_common.h"*/
#include "sys_usw_peri.h"

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
extern int32
sys_tsingma_peri_init(uint8 lchip);

extern int32
sys_tsingma_peri_mdio_init(uint8 lchip);

extern int32
sys_tsingma_peri_set_phy_scan_cfg(uint8 lchip);

extern int32
sys_tsingma_peri_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);

extern int32
sys_tsingma_peri_get_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para);

extern int32
sys_tsingma_peri_read_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

extern int32
sys_tsingma_peri_write_phy_reg(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para);

extern int32
sys_tsingma_peri_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq);

extern int32
sys_tsingma_peri_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq);

extern int32
sys_tsingma_peri_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner);

extern int32
sys_tsingma_peri_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map);

extern int32
sys_tsingma_peri_set_mac_led_en(uint8 lchip, bool enable);

extern int32
sys_tsingma_peri_get_mac_led_en(uint8 lchip, bool* enable);

extern int32
sys_tsingma_peri_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode);

extern int32
sys_tsingma_peri_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para);

extern int32
sys_tsingma_peri_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value);

extern int32
sys_tsingma_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void* p_data);

extern int32
sys_tsingma_peri_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value);

#ifdef __cplusplus
}
#endif

#endif

