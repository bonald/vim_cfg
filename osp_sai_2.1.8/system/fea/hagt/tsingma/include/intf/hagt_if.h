/****************************************************************************
* $Id$
*  Centec Interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-07-19 
* Reason        : First Create.
****************************************************************************/

#ifndef __HAGT_IF_H__
#define __HAGT_IF_H__

#include "hagt.h"
#include "ctc_const.h"
#include "glb_if_define.h"
#include "hagt_chip.h"

typedef uint32  hagt_if_flag_t; /* HAGT_IF_PORT_FLAG_XXX */

#define HAGT_IF_PORT_FLAG_RUNNING                (1 << 0)
#define HAGT_IF_PORT_FLAG_ROUTED_EN              (1 << 1)

#define HAGT_IF_PORT_FLAG_ALLOW_TAGGED           (1 << 2)
#define HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED         (1 << 3)

#define HAGT_IF_PORT_FLAG_MIRROR_DEST            (1 << 4)

#define HAGT_IF_PORT_FLAG_AGGREGATED             (1 << 5)
#define HAGT_IF_PORT_FLAG_AGG_OPRED              (1 << 6)
#if 0 /*not support stand-alone flag*/
#define HAGT_IF_PORT_FLAG_STATE_STAND_ALONE      (1 << 7)
#endif
#define HAGT_IF_PORT_FLAG_DIS_DISTRIBUTOR        (1 << 8)
#define HAGT_IF_PORT_FLAG_DIS_COLLECTOR          (1 << 9)

#define HAGT_IF_PORT_FLAG_DOT1X_UNAUTHED         (1 << 10)
#define HAGT_IF_PORT_FLAG_DOT1X_DIR_IN           (1 << 11)
#define HAGT_IF_PORT_FLAG_DOT1X_DIR_OUT          (1 << 12)

#define HAGT_IF_PORT_FLAG_EFM_RX_DISCARD         (1 << 14)

#define HAGT_IF_PORT_FALG_L2VPN_PORT_MODE     (1 << 15)
#define HAGT_IF_PORT_FALG_L2VPN_VLAN_MODE     (1 << 16)
#define HAGT_IF_PORT_FALG_L2VPN_ENABLE             (1<<17)

/*for qinq and vlan translation*/
#define HAGT_IF_PORT_FLAG_VLAN_TRANSLATION_ENABLE    (1 << 18) /* enable vlan-translation */
#define HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE   (1 << 19) /* enable qinq */
#define HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_SELECTIVE     (1 << 20) /*basic qinq or selective qinq*/

#define HAGT_IF_PORT_FLAG_VLAN_CLASS_ENABLE     (1 << 21)
#define HAGT_IF_PORT_FLAG_INGRESS_FILTER_EN     (1 << 22)
#define HAGT_IF_PORT_FLAG_EGRESS_FILTER_EN     (1 << 23)

#define HAGT_IF_PORT_FLAG_EFM_PAR_ACTION_LB    (1 << 24)
#define HAGT_IF_PORT_FLAG_EFM_PAR_ACTION_DISCARD    (1 << 25)
#define HAGT_IF_PORT_FLAG_EFM_MUX_ACTION_DISCARD    (1 << 26)
#define HAGT_IF_PORT_FLAG_EFM_LINK_MONITOR    (1 << 27)

#define HAGT_IF_PORT_FLAG_PHY_LPBK_EXTERNAL   (1 << 28)

/* Added by yanxa, for bug 16102, 2011-08-31 */
#define HAGT_IF_PORT_FLAG_IS_10G_PORT         (1 << 29)
#define HAGT_IF_PORT_FLAG_IS_STACKING_PORT         (1 << 30)
/*added by ychen for bug 25183 in 2013-10-12*/
#define HAGT_IF_PORT_FLAG_ACL_APPLY                (1 << 31)

/* Added by Yan Xing'an, for openflow, 2015-01-07 */
#define HAGT_IF_PORT_FLAG_OPENFLOW_IGNORE_VLAN_FILTER_CTL (1 << 7)

/* define for flow control */
#define HAGT_IF_PORT_FLOWCTRL_RECV      0
#define HAGT_IF_PORT_FLOWCTRL_SEND      1

#if 0
#define HAGT_IF_STATS_LOCK
#define HAGT_IF_STATS_UNLOCK
#else
/* modified by cuixl for bug hagt if, 2012-02-24 */
#define HAGT_IF_STATS_LOCK \
    sal_mutex_lock(g_pst_hagt_if_master->p_port_stats_mutex)

#define HAGT_IF_STATS_UNLOCK \
    sal_mutex_unlock(g_pst_hagt_if_master->p_port_stats_mutex)
#endif

#define HAGT_IF_STATS_MAX_ITEM 64

#define HAGT_IF_UINT64_TO_OCTET_STR(data, str_s, str_d)                \
do{                                                                 \
    int32 i;                                                        \
    str_s.buf = str_d;                                              \
    str_s.size = sizeof(uint64);                                    \
    for(i = 0; i < sizeof(uint64); i++)                             \
    {                                                               \
        str_d[i] = (uint8)((data >> (i*8))&0xFF);                  \
    }                                                               \
}while(0)


/*error return*/
#define HAGT_IF_STATS_ERROR_RETURN_WITH_UNLOCK(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        HAGT_IF_STATS_UNLOCK;    \
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

    
/*error return*/
#define HAGT_IF_STATS_ERROR_RETURN(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

/******************************************************************************
*
* Functions
*
******************************************************************************/
/******************************************************************************
 * Name         : hagt_ifdb_set_port_flag
 * Purpose      : set vlan tag ctrl, transmit enable or dot1q type
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
                flag:    port flag, which is used to calculate the vlan tag ctrl, dot1q type and transmit enable
                enalbe:  1=enalbe, flag set; 0=enable, flag unset
 * Output       : N/A
 * Return       : HAGT_E_NOT_FOUND: port is not found or not in this linecard; HAGT_E_NONE: ok
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_set_port_flag (uint16 gportid, hagt_if_flag_t flag, bool enable);
/******************************************************************************
 * Name         : hagt_ifdb_get_port_flag
 * Purpose      : get port flag
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
                p_flag:    port flag, which is used to calculate the vlan tag ctrl, dot1q type and transmit enable            
 * Output       : N/A
 * Return       : HAGT_E_xxxx
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_get_port_flag(uint16 gportid, hagt_if_flag_t* p_flag);
/***************************************************************************************************
 * Name         : hagt_if_init 
 * Purpose      : init hagt interface module, malloc module memory and init       
 * Input        : pst_master: hagt master           
 * Output       : N/A              
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/

/******************************************************************************
 * Name         : hagt_ifdb_get_port_ifname
 * Purpose      : get interface name by gportid
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
 * Output       : ifname
 * Return       : ifname(NULL if not fount the interface)
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_get_port_ifname(uint16 gportid, char *ifname);

extern int32
hagt_if_init (hagt_master_t* pst_master);

/***************************************************************************************************
 * Name         : hagt_l3if_clear_all_attributes 
 * Purpose      : clear all layer3 interface features before interface delete      
 * Input        : l3ifid: layer3 interface ifid  
                : p_l3_if: layer3 interface info
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : 1)nsm will call API to unset l3if features after l3if is deleted, so here we should 
                    clear all l3if features before interface deleted.
                  2)we can not use sending destroy interface msg in hsrv_ifdb_remove_intf(), because 
                     physical interface(routed port) does not support it.
***************************************************************************************************/
extern int32
hagt_l3if_clear_all_attributes (uint16 l3ifid, ctc_l3if_t* p_l3_if);

/***************************************************************************************************
 * Name         : hagt_l3if_set_default_attributes 
 * Purpose      : when layer3 interface create, set default attributes      
 * Input        : l3ifid: layer3 interface ifid       
                : p_l3_if: layer3 interface info
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A 
***************************************************************************************************/
extern int32
hagt_l3if_set_default_attributes(uint16 l3ifid, ctc_l3if_t* p_l3_if);

/***************************************************************************************************
 * Name         : hagt_l3if_mapping_intf_type 
 * Purpose      : mapping glb interface type to ctc interface type       
 * Input        : iftype: glb_if_type_t           
 * Output       : p_ctc_iftype: ctc_l3if_type_t              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
extern int32
hagt_l3if_mapping_intf_type(uint32 iftype, uint8* p_ctc_iftype);

#define HAGT_GPORT_IS_LOCAL(gportid) (hagt_chip_port_is_local(gportid))

int32
hagt_if_create_destroy_port(uint16 gportid, uint16 frame_size, mac_addr_t mac_addr, bool enable,
                            uint16 slotno, uint16 portno, uint16 subportno);
int32 hagt_if_set_stack_port_act (uint16 gportid, uint32 trunk);
int32 hagt_if_is_stacking_port (uint16 gportid);
/*added by ychen for bug 25183 in 2013-10-11*/
extern int32 hagt_if_apply_global_acl_lkp(uint32* enable);
#endif /*__HAGT_IF_H__*/

