/****************************************************************************
* $Id$
*  Centec eeprom information related global MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-26 
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_EEPROM_DEFINE_H__
#define __GLB_EEPROM_DEFINE_H__

enum eeprom_info_list_e
{
    EEPROM_SERIAL_PARAM = 0,
    EEPROM_BOOTVER,
    EEPROM_BOOTCMD,
    EEPROM_BOOTUP,
    EEPROM_SERIALNO,
    EEPROM_SYS_MAC,
    EEPROM_DATAPATH,
    EEPROM_TCAM_PROFILE,
    EEPROM_BR_PROFILE,
    EEPROM_0_MANU_TEST,
    EEPROM_BOOTUP_DIAG_LEVEL,
    EEPROM_BOOTUP_DIAG_STRESS_TIME,
    
    EEPROM_CHASSIS_TYPE,
    EEPROM_BP_CHASSIS_TYPE,
    EEPROM_BP_SYS_MAC,
    EEPROM_LC_ERR_REBOOT,
    EEPROM_CHASSIS_FAN_MANAGE,
    EEPROM_OEM_INFO,
    EEPROM_FREQ_TYPE,
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    EEPROM_SMARTCFG,
#endif /* !HAVE_SMARTCFG */
    EEPROM_DATAPATH_MODE, /*added by jqiu, for XG-1G mode static change. 20131008*/
    /* default config for management: user/pin/vlan 1 ip address
     * Total 24 bytes, 0X1248-125F
     *   byte 01-08: user name, "nouser" for no username
     *   byte 09-16: pin, "nopin" for no password
     *   byte 17-20: ipv4 address (u_int32_t), 0.0.0.0 for no ip address
     *   byte 21-24: reserverd
     */
    EEPROM_MGMT_CFG,
    EEPROM_PORT_INFO,
    EEPROM_REBOOT_INFO,/*0x55 mean reboot command ,0xaa mean sensor high tempture*/
    EEPROM_SW_VER, /*fix bug39335, support to define sw_ver, liangf, 2016-07-06*/
    EEPROM_BOARD_MATERIAL, 
    EEPROM_MAX
};
typedef enum eeprom_info_list_e eeprom_info_list_t;

struct eeprom_info_s
{
   uint8 eeprom_idx;
   uint8 reserved;
   uint16 base_addr;
};
typedef struct eeprom_info_s eeprom_info_t;


#endif
