/**
 @file sys_usw_l3if.h

 @date 2009-12-7

 @version v2.0

---file comments----
*/
#ifndef _SYS_USW_L3IF
#define _SYS_USW_L3IF
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
*
* Header Files
*
***************************************************************/
#include "ctc_l3if.h"

/**********************************************************************************
                          Defines and Macros
***********************************************************************************/
#define SYS_L3IF_INVALID_L3IF_ID   0
#define SYS_ROUTER_MAC_NUM_PER_ENTRY    8
#define SYS_ROUTER_MAC_PREFIX_NUM       6 

struct sys_l3if_router_mac_s
{
    uint8       postfix_bmp;
    uint8       prefix_idx;
    mac_addr_t  mac;
};
typedef struct sys_l3if_router_mac_s sys_l3if_router_mac_t;

struct sys_l3if_rtmac_profile_s
{
    uint8 lchip;
    uint8 valid_bitmap;
    uint8 match_mode ; /*0:lookup container, 1: exactly, 2:lookup void mac position*/
    uint8 new_node;
    uint16 profile_id;
    uint16  mac_ref[SYS_ROUTER_MAC_NUM_PER_ENTRY];
    sys_l3if_router_mac_t mac_info[SYS_ROUTER_MAC_NUM_PER_ENTRY];   
};
typedef struct sys_l3if_rtmac_profile_s sys_l3if_rtmac_profile_t;

struct sys_l3if_macinner_profile_s
{
    uint8 type;               /*maninner & egs routermac share the struct*/
    uint8 rsv[3];
    mac_addr_t           mac;
    uint16 profile_id;
    
};
typedef struct sys_l3if_macinner_profile_s sys_l3if_macinner_profile_t;

struct sys_l3if_prop_s
{
    uint16  vlan_id;
    uint16  gport;

    uint8   vaild;
    uint8   l3if_type; /**< the type of l3interface , CTC_L3IF_TYPE_XXX */
    uint8   rtmac_bmp; /* routerMacSelBitmap in DsRouterMac*/
    sys_l3if_rtmac_profile_t * p_rtmac_profile;
    sys_l3if_macinner_profile_t* p_egs_rtmac_profile;/* for egress route mac*/
    uint8   user_egs_rtmac;
    uint16  vlan_ptr; /**< Vlanptr */
    uint16  l3if_id; /**< l3if id */
};
typedef struct sys_l3if_prop_s sys_l3if_prop_t;

struct sys_l3if_ecmp_if_s
{
    uint16 hw_group_id;
    uint8  intf_count;
    uint8  failover_en;
    uint16 intf_array[SYS_USW_MAX_ECPN];
    uint32 dsfwd_offset[SYS_USW_MAX_ECPN];
    uint16 ecmp_group_type;     /* refer to ctc_nh_ecmp_type_t */
    uint16 ecmp_member_base;
    uint32 stats_id;
};
typedef struct sys_l3if_ecmp_if_s sys_l3if_ecmp_if_t;

#define SYS_VRFID_CHECK(vrfid)       \
    {                                                                   \
        if (vrfid > MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID))    \
        {                                                               \
            return CTC_E_BADID;                                 \
        }                                                               \
    }

#define SYS_IP_VRFID_CHECK(vrfid, ip_ver)       \
    {                                                                   \
        if (vrfid > MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID))    \
        {                                                               \
            return CTC_E_BADID;                                 \
        }                                                               \
    }

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/

/**********************************************************************************
                    ----  L3 interfaces's  properties  ----
***********************************************************************************/
/**
 @brief    init l3 interface module
*/
extern int32
sys_usw_l3if_init(uint8 lchip, ctc_l3if_global_cfg_t* l3if_global_cfg);

/**
 @brief    deinit l3 interface module
*/
extern int32
sys_usw_l3if_deinit(uint8 lchip);

/**
@brief    Create  l3 interfaces
*/
extern int32
sys_usw_l3if_create(uint8 lchip, uint16 l3if_id, ctc_l3if_t* l3_if);

/**
   @brief    Delete  l3 interfaces
*/
extern int32
sys_usw_l3if_delete(uint8 lchip, uint16 l3if_id, ctc_l3if_t* l3_if);


extern int32
sys_usw_l3if_get_l3if_id(uint8 lchip, ctc_l3if_t* p_l3_if, uint16* l3if_id);

/**
@brief   Judge Routed port  according to global logic port ID
*/
extern bool
sys_usw_l3if_is_port_phy_if(uint8 lchip, uint32 gport);

/**
@brief   get port is subif port
*/
extern bool
sys_usw_l3if_is_port_sub_if(uint8 lchip, uint32 gport);

/**
   @brief    Config  l3 interface's  properties
*/
extern int32
sys_usw_l3if_set_property(uint8 lchip, uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32 value);

/**
@brief    Get  l3 interface's properties according to interface id
*/
extern int32
sys_usw_l3if_get_property(uint8 lchip, uint16 l3if_id, ctc_l3if_property_t l3if_prop, uint32* value);

/**
@brief    Set  l3 interface's acl properties according to interface id
*/
extern int32
sys_usw_l3if_set_acl_property(uint8 lchip, uint16 l3if_id, ctc_acl_property_t* acl_prop);

/**
@brief    Get  l3 interface's acl properties according to interface id
*/
extern int32
sys_usw_l3if_get_acl_property(uint8 lchip, uint16 l3if_id, ctc_acl_property_t* acl_prop);

/**
@brief    Set  l3 interface's properties according to interface id
*/
extern int32
sys_usw_l3if_set_exception3_en(uint8 lchip, uint16 l3if_id, uint8 index, bool enbale);

/**
@brief   Get  l3 interface's properties according to interface id
*/
extern int32
sys_usw_l3if_get_exception3_en(uint8 lchip, uint16 l3if_id, uint8 index, bool* enbale);

/**
@brief   Get  l3 interface's info,such as l3if id,vlanPtr etc
*/
extern int32
sys_usw_l3if_get_l3if_info(uint8 lchip, uint8 type, sys_l3if_prop_t* l3if_prop);

/**
@brief   Get  l3 interface's info,such as l3if id,vlanPtr etc
*/
extern int32
sys_usw_l3if_get_l3if_info_with_port_and_vlan(uint8 lchip, uint32 gport, uint16 vlan_id, sys_l3if_prop_t* l3if_prop);

/**********************************************************************************
                   ----  L3 interfaces's router mac    ----
***********************************************************************************/
/**
@brief    Config  router mac
*/
extern int32
sys_usw_l3if_set_router_mac(uint8 lchip, mac_addr_t mac_addr);

/**
@brief    Get  router mac
*/
extern int32
sys_usw_l3if_get_router_mac(uint8 lchip, mac_addr_t mac_addr);

/**
     @brief      Config l3 interface router mac
*/
extern int32
sys_usw_l3if_set_interface_router_mac(uint8 lchip, uint16 l3if_id, ctc_l3if_router_mac_t router_mac);

/**
     @brief       Get l3 interface router mac
*/
extern int32
sys_usw_l3if_get_interface_router_mac(uint8 lchip, uint16 l3if_id, ctc_l3if_router_mac_t* router_mac);

extern int32
sys_usw_l3if_set_vmac_prefix(uint8 lchip,  uint32 prefix_idx, mac_addr_t mac_40bit);

extern int32
sys_usw_l3if_get_vmac_prefix(uint8 lchip,  uint32 prefix_idx, mac_addr_t mac_40bit);

extern int32
sys_usw_l3if_get_default_entry_mode(uint8 lchip);

extern int32
sys_usw_l3if_set_vrf_stats_en(uint8 lchip, ctc_l3if_vrf_stats_t* p_vrf_stats);

extern int32
sys_usw_l3if_get_vrf_statsid(uint8 lchip, uint16 vrfid, uint32* p_statsid);

extern int32
sys_usw_l3if_create_ecmp_if(uint8 lchip, ctc_l3if_ecmp_if_t* p_ecmp_if);

extern int32
sys_usw_l3if_destory_ecmp_if(uint8 lchip, uint16 ecmp_if_id);

extern int32
sys_usw_l3if_add_ecmp_if_member(uint8 lchip, uint16 ecmp_if_id, uint16 l3if_id);

extern int32
sys_usw_l3if_remove_ecmp_if_member(uint8 lchip, uint16 ecmp_if_id, uint16 l3if_id);

extern int32
sys_usw_l3if_get_ecmp_if_info(uint8 lchip, uint16 ecmp_if_id, sys_l3if_ecmp_if_t* p_ecmp_if);

extern uint32
sys_usw_l3if_binding_inner_router_mac(uint8 lchip, mac_addr_t* p_mac_addr, uint8* route_mac_index);

extern int32
sys_usw_l3if_unbinding_inner_router_mac(uint8 lchip, uint8 route_mac_index);

#ifdef __cplusplus
}
#endif

#endif /*_SYS_USW_L3IF*/

