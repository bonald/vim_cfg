/**
 @file sys_duet2_mac.h

 @date 2018-09-12

 @version v1.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_DUET2_MAC_H
#define _SYS_DUET2_MAC_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_vlan.h"
#include "ctc_port.h"
#include "ctc_security.h"
#include "ctc_interrupt.h"
#include "sys_usw_datapath.h"


/***************************************************************
 *
 * Macro definition
 *
 ***************************************************************/
#define MAX_HS_MAC_NUM 96
#define MAX_CS_MAC_NUM 32

/***************************************************************
 *
 * Structure definition
 *
 ***************************************************************/
typedef struct mac_link_status_s
{
    uint8 mac_id;
    uint8 link_status; /*0 means linkdown, 1 means linkup*/
}mac_link_status_t;

/***************************************************************
 *
 * Function declaration
 *
 ***************************************************************/
int32 sys_duet2_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);

int32 sys_duet2_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);

int32 sys_duet2_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);
    
int32 sys_duet2_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);
    
int32 sys_duet2_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);
    
int32 sys_duet2_mac_init(uint8 lchip);



#ifdef __cplusplus
}
#endif

#endif

