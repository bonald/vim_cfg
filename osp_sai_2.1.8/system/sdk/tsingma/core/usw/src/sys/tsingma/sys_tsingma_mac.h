/**
 @file sys_tsingma_mac.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2017-12-29

 @version v2.0

*/

#ifndef _SYS_TSINGMA_MAC_H
#define _SYS_TSINGMA_MAC_H
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
 /*#include "ctc_vlan.h"*/
 /*#include "ctc_port.h"*/
 /*#include "ctc_security.h"*/
 /*#include "ctc_interrupt.h"*/
#include "sys_tsingma_datapath.h"
#include "sys_usw_mac.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
enum sys_func_intr_feature_e
{
    FUNC_INTR_AN_COMPLETE = 0,     //funcIntrHssL[]AnCompleteMcu
    FUNC_INTR_AN_LINK_GOOD,        //funcIntrHssL[]AnLinkGoodMcu
    FUNC_INTR_AN_PAGE_RX,          //funcIntrHssL[]AnPageRxMcu
    FUNC_INTR_RX_UPDATE_REQ,       //funcIntrHssL[]RxUpdateReqMcu
    FUNC_INTR_TRAIN_FAIL,          //funcIntrHssL[]TrainFailMcu
    FUNC_INTR_TRAIN_OK,            //funcIntrHssL[]TrainOkMcu
    FUNC_INTR_BUTT
};
typedef enum sys_func_intr_feature_e sys_func_intr_feature_t;

/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
sys_tsingma_mac_get_port_capability(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t** p_port);

int32
_sys_tsingma_get_mac_mii_pcs_index(uint8 lchip, uint8 mac_id, uint8 serdes_id, uint8 pcs_mode, uint8* sgmac_idx,
                                              uint8* mii_idx, uint8* pcs_idx, uint8* internal_mac_idx);

/*external property*/

extern int32
sys_tsingma_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port);

extern int32
_sys_tsingma_mac_set_fec_en(uint8 lchip, uint16 lport, uint32 type);

extern int32
sys_tsingma_mac_get_internal_property(uint8 lchip, uint32 gport, ctc_port_property_t property, uint32* p_value);

extern int32
sys_tsingma_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);

extern int32
sys_tsingma_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);

extern int32
sys_tsingma_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode);

extern int32
_sys_tsingma_mac_set_mac_config(uint8 lchip, uint16 lport, sys_datapath_lport_attr_t* port_attr, uint8 is_init);

extern int32
sys_tsingma_mac_isr_cl73_complete_isr(uint8 lchip, void* p_data, void* p_data1);

extern int32
sys_tsingma_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);

extern int32 sys_tsingma_mac_init(uint8 lchip);
extern int32
sys_tsingma_mac_self_checking(uint8 lchip, uint32 gport);

#ifdef __cplusplus
}
#endif

#endif

