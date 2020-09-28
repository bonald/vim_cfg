/**
 @file sys_usw_dmps.h

 @author  Copyright (C) 2018 Centec Networks Inc.  All rights reserved.

 @date 2018-09-12

 @version v2.0

*/

#ifndef _SYS_USW_DMPS_H
#define _SYS_USW_DMPS_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define  SYS_GET_LPORT_ID_WITH_CHAN(lchip, chan_id)     sys_usw_dmps_get_lport_with_chan(lchip, chan_id)
#define  SYS_MAC_IS_VALID(lchip, mac_id)                sys_usw_dmps_check_mac_valid(lchip, mac_id)

/**
 @brief Dmps port property
*/
enum sys_dmps_port_property_e
{
    SYS_DMPS_PORT_PROP_EN,
    SYS_DMPS_PORT_PROP_LINK_UP,
    SYS_DMPS_PORT_PROP_TX_IPG,
    SYS_DMPS_PORT_PROP_MAC_ID,
    SYS_DMPS_PORT_PROP_CHAN_ID,
    SYS_DMPS_PORT_PROP_SLICE_ID,
    SYS_DMPS_PORT_PROP_SERDES,
    SYS_DMPS_PORT_PROP_PORT_TYPE,
    SYS_DMPS_PORT_PROP_SPEED_MODE,
    SYS_DMPS_PORT_PROP_WLAN_ENABLE,
    SYS_DMPS_PORT_PROP_DOT1AE_ENABLE,
    SYS_DMPS_PORT_PROP_MAC_STATS_ID,
    SYS_DMPS_PORT_PROP_XPIPE_ENABLE,
    SYS_DMPS_PORT_PROP_MAX
};
typedef enum sys_dmps_port_property_e  sys_dmps_port_property_t;

typedef int32 (* SYS_CALLBACK_DMPS_FUN_P)  (uint8 lchip, uint32 gport);

enum sys_dmps_lport_type_e
{
    SYS_DMPS_NETWORK_PORT,
    SYS_DMPS_OAM_PORT,
    SYS_DMPS_DMA_PORT,
    SYS_DMPS_ILOOP_PORT,
    SYS_DMPS_ELOOP_PORT,
    SYS_DMPS_WLANDEC_PORT,
    SYS_DMPS_WLANENC_PORT,
    SYS_DMPS_WLANREASSEMBLE_PORT,
    SYS_DMPS_MAC_DECRYPT_PORT,
    SYS_DMPS_MAC_ENCRYPT_PORT,
    SYS_DMPS_RSV_PORT
};
typedef enum sys_dmps_lport_type_e sys_dmps_lport_type_t;

struct sys_dmps_serdes_info_s
{
    uint8 serdes_id[4];
    uint8 serdes_mode;
    uint8 serdes_num;
};
typedef struct sys_dmps_serdes_info_s sys_dmps_serdes_info_t;

/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);
extern int32
sys_usw_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);
extern int32
sys_usw_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);
extern int32
sys_usw_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);
extern int32
sys_usw_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);
extern int32
sys_usw_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32
sys_usw_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);
extern int32
sys_usw_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);

extern int32
sys_usw_dmps_set_port_property(uint8 lchip, uint32 gport, sys_dmps_port_property_t dmps_port_prop, void *p_value);

extern int32
sys_usw_dmps_get_port_property(uint8 lchip, uint32 gport, sys_dmps_port_property_t dmps_port_prop, void *p_value);

/* Used by sys layer to set local phy port to mac mapping */
extern int32
sys_usw_dmps_set_port_chan_mac_mapping(uint8 lchip, uint32 gport, uint8 chan_id, uint8 mac_id);

/* read table to get local phy port with channel */
extern uint16
sys_usw_dmps_get_lport_with_chan(uint8 lchip, uint8 chan_id);


#if 1   /* will delete later */
extern bool
sys_usw_dmps_check_mac_valid(uint8 lchip, uint8 mac_id);

extern uint8
sys_usw_dmps_get_mac_type(uint8 lchip, uint8 mac_id);

extern uint16
sys_usw_dmps_get_lport_with_mac(uint8 lchip, uint8 mac_id);

extern uint16
sys_usw_dmps_get_lport_with_mac_tbl_id(uint8 lchip, uint8 mac_tbl_id);
#endif

extern uint32
SYS_GET_MAC_ID(uint8 lchip, uint32 gport);

extern uint32
SYS_GET_CHANNEL_ID(uint8 lchip, uint32 gport);

extern uint16
sys_usw_dmps_get_core_clock(uint8 lchip, uint8 core_type);

extern int32
sys_usw_dmps_set_dlb_chan_type(uint8 lchip, uint8 chan_id);
#ifdef __cplusplus
}
#endif

#endif

