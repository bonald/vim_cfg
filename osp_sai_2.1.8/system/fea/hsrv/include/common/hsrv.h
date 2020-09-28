/**
 *  Copyright (C) 2011, 2012, 2013 CentecNetworks, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef __HSRV_H__
#define __HSRV_H__


#define HSRV_GPORT_TO_GCHIP(gport) ((gport) >> 8)

#define HSRV_MALLOC(type,size) XMALLOC(type,size)
#define HSRV_CALLOC(type,size) XCALLOC(type,size)
#define HSRV_FREE(type,ptr)    XFREE(type,ptr)

typedef enum hsrv_module_id_e
{
    HSRV_MODULE_MIRROR = 0,
    HSRV_MODULE_VLAN,
    HSRV_MODULE_AGG,
    HSRV_MODULE_INTERFACE,
    HSRV_MODULE_BRIDGE,
    HSRV_MODULE_L2MC,
    HSRV_MODULE_L3MC,
    HSRV_MODULE_L3MC_IPV6,  /*added by chenxw for l3mc ipv6. 2012-07-27*/
    HSRV_MODULE_NEXTHOP,
    HSRV_MODULE_NEIGHBOR,
    HSRV_MODULE_ECMP, /*added by ychen in 2012-04-06*/
    HSRV_MODULE_TUNNEL, /*added by ychen in 2012-07-06, must before ipuc, should add encap nhid*/
    HSRV_MODULE_IPUC,
    HSRV_MODULE_VRRP,
    HSRV_MODULE_VLANCLASS,
    HSRV_MODULE_VLAN_MAPPING,
    HSRV_MODULE_ACL,
    HSRV_MODULE_QOS,
    HSRV_MODULE_STATS,
    HSRV_MODULE_SFLOW,   
    HSRV_MODULE_CPU_TRAFFIC,
    HSRV_MODULE_PTP,    /* Add by kcao for PTP. 2010.07.25 */
    HSRV_MODULE_DEBUG,/*Add by huangxt for debug. 2010.07*/
    HSRV_MODULE_SECURITY,
    HSRV_MODULE_RATELIMIT,
    HSRV_MODULE_DOT1X,
    HSRV_MODULE_OAM_EFM,
    HSRV_MODULE_MPLS,
    HSRV_MODULE_MPLSTP,
    HSRV_MODULE_IP_BINDING,
    HSRV_MODULE_L2PROTOCOL,
    HSRV_MODULE_VLAN_SECURITY,
    HSRV_MODULE_PHY_LOOPBACK,
    HSRV_MODULE_SYNCE,
    HSRV_MODULE_BFD, /* modified by huangxt for BFD develop, 2011-02-23 */
    HSRV_MODULE_CFM, /* modified by liwh to support cfm for bug 14710 */
    HSRV_MODULE_PBR,    /* Add by chenzh for bug 15103, 2011-06-29*/
    HSRV_MODULE_IPT,   /* add by huwx for iptables, bug 15656, 2011-07-27 */
    HSRV_MODULE_TPOAM, /* Added by kcao for MPLS-TP OAM develop, bug 15633, 2011-07-28 */
    HSRV_MODULE_APS,
    HSRV_MODULE_G8031,
    HSRV_MODULE_G8032,        
    HSRV_MODULE_IVI,  
    HSRV_MODULE_VOICE_VLAN,  
    HSRV_MODULE_OPENFLOW, /* Added by Yan Xing'an, 2014-12-15 */
    HSRV_MODULE_TAP,
    
    HSRV_MODULE_MAX
} hsrv_module_id_t;

typedef struct hsrv_module_info_s
{
    uint32 module_id; /* decides the operation order for each module when DB-SYN*/
    HSRV_CFG_SYNC_CB_FUNC sync_prepare_cb; /*DB sync up prepare function*/
    HSRV_CFG_SYNC_CB_FUNC sync_cancel_cb; /*DB sync up cancel function*/
    HSRV_CFG_SYNC_CB_FUNC sync_cb; /*DB sync up function*/
    HSRV_CFG_SYNC_CB_FUNC been_active_cb;    /*Functions after DB sync up*/
    void *p_data_info;/*module detail data information*/
} hsrv_module_info_t;

#define HSRV_AGENTID_SET_SLOT(agentId, slot)\
    do{\
        GLB_SET_BIT(agentId, slot);\
    }   while(0)

#define HSRV_ALL_AGENT_ID               0xFFFFFFFF
#define HSRV_AGENTID_SET_ALL(agentId)   sal_memset(&agentId, 0xff, sizeof(agentId));

#define HSRV_L3_AGENTID_SET_SLOT(agentId, ifindex)\
    do{\
        uint8 slot; \
        uint8 port; \
        uint8 subport; \
        glb_if_type_t iftype; \
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_iftype(ifindex, &iftype)); \
        if (GLB_IF_TYPE_VLAN_IF == iftype || GLB_IF_TYPE_LINKAGG_IF == iftype) \
        { \
            HSRV_AGENTID_SET_ALL(agentId); \
        } \
        else \
        { \
            HSRV_IF_ERROR_RETURN (hsrv_ifdb_get_phy_card_info(ifindex, &slot, &port, &subport)); \
            HSRV_AGENTID_SET_SLOT(agentId, slot); \
        } \
    }   while(0)

struct hsrv_system_spec_info_s
{ 
    glb_stm_resouce_info_t res_info;
    uint8 max_gchip_num;
    uint8 max_lport_per_chip;
};
typedef struct hsrv_system_spec_info_s hsrv_system_spec_info_t;

struct hsrv_master_s
{
    hsrv_module_info_t* p_module_info[HSRV_MODULE_MAX];
    void *p_messge_info;
    hsrv_system_spec_info_t system_info;
};
typedef struct hsrv_master_s hsrv_master_t;

#define HSRV_MODULE_INIT(_pp_module_, _g_master_, _p_module_, _module_, _ds_) \
    (_p_module_) = HSRV_CALLOC(MEM_FEA_HSRV_MODULE, sizeof(hsrv_module_info_t)); \
    HSRV_MEM_CHECK((_p_module_)); \
    (_p_module_)->p_data_info = HSRV_CALLOC(MEM_FEA_HSRV_MODULE, sizeof(_ds_)); \
    if (NULL == (_p_module_)->p_data_info) \
    { \
        HSRV_FREE(MEM_FEA_HSRV_MODULE, (_p_module_)); \
        return HSRV_E_NO_MEMORY; \
    } \
    (_g_master_) = (_p_module_)->p_data_info; \
    (*_pp_module_) = _p_module_; \
    (_p_module_)->module_id = (_module_);

#endif /* !__HSRV_H__ */
