/****************************************************************************
* $Id$
*  Centec sysenv information related global MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2019-01-16
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_SYSENV_DEFINE_H__
#define __GLB_SYSENV_DEFINE_H__

enum sysenv_info_list_e
{
    SYSENV_SERIAL_PARAM = 0,
    SYSENV_BOOTVER,
    SYSENV_BOOTCMD,
    SYSENV_BOOTUP,
    SYSENV_SERIALNO,
    SYSENV_SYS_MAC,
    SYSENV_DATAPATH,
    SYSENV_TCAM_PROFILE,
    SYSENV_BR_PROFILE,

    SYSENV_CHASSIS_TYPE,
    SYSENV_BP_CHASSIS_TYPE,
    SYSENV_BP_SYS_MAC,
    SYSENV_LC_ERR_REBOOT,
    SYSENV_CHASSIS_FAN_MANAGE,
    SYSENV_OEM_INFO,
    SYSENV_FREQ_TYPE,
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    SYSENV_SMARTCFG,
#endif /* !HAVE_SMARTCFG */
    SYSENV_DATAPATH_MODE, /*added by jqiu, for XG-1G mode static change. 20131008*/
    /* default config for management: user/pin/vlan 1 ip address
     * Total 24 bytes, 0X1248-125F
     *   byte 01-08: user name, "nouser" for no username
     *   byte 09-16: pin, "nopin" for no password
     *   byte 17-20: ipv4 address for mgmt (u_int32_t), 0.0.0.0 for no ip address
     *   byte 21-24: ipv4 address for mgmt interface (u_int32_t), 0.0.0.0 for no ip address
     */
    SYSENV_MGMT_CFG,
    SYSENV_PORT_INFO,
    SYSENV_REBOOT_INFO,/*0x55 mean reboot command ,0xaa mean sensor high tempture*/
    SYSENV_PHY_SERIES_LED,
    SYSENV_SW_VER, /*fix bug39335, support to define sw_ver, liangf, 2016-07-06*/
    SYSENV_BOARD_MATERIAL,
    SYSENV_MAX
};
typedef enum sysenv_info_list_e sysenv_info_list_t;

struct sysenv_info_s
{
   uint8 sysenv_idx;
   uint8 reserved;
   uint16 base_addr;
};
typedef struct sysenv_info_s sysenv_info_t;

#endif
