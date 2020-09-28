/**
 @file sys_usw_vlan.h

 @date 2009-10-17

 @version v2.0

*/
#ifndef _SYS_USW_VLAN_H
#define _SYS_USW_VLAN_H
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
#include "ctc_stats.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
enum sys_vlan_property_e
{
    SYS_VLAN_PROP_STP_ID,
    SYS_VLAN_PROP_INTERFACE_ID,
    SYS_VLAN_PROP_SRC_QUEUE_SELECT,
    SYS_VLAN_PROP_EGRESS_FILTER_EN,
    SYS_VLAN_PROP_INGRESS_FILTER_EN,
    SYS_VLAN_PROP_CID,
    /*below belong to vsi.*/
    SYS_VLAN_PROP_UCAST_DISCARD,
    SYS_VLAN_PROP_MCAST_DISCARD,
    SYS_VLAN_PROP_BCAST_DISCARD,
    SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION,
    SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD,
/* below belong to vlan profile. */
    SYS_VLAN_PROP_VLAN_STORM_CTL_PTR,
    SYS_VLAN_PROP_VLAN_STORM_CTL_EN,
    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID,
    SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY0,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY1,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY2,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_ROUTED_ONLY3,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL0,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL1,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL2,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_LABEL3,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN3,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN2,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN1,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_EN0,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE1,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE2,
    SYS_VLAN_PROP_INGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE3,
    SYS_VLAN_PROP_INGRESS_VLAN_POLICER_EN,
    SYS_VLAN_PROP_INGRESS_STATSPTR,
    SYS_VLAN_PROP_FORCE_IPV6LOOKUP,
    SYS_VLAN_PROP_FORCE_IPV4LOOKUP,
    SYS_VLAN_PROP_FIP_EXCEPTION_TYPE,
    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID,
    SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN,
    SYS_VLAN_PROP_EGRESS_VLAN_ACL_ROUTED_ONLY,
    SYS_VLAN_PROP_EGRESS_VLAN_ACL_LABEL0,
    SYS_VLAN_PROP_EGRESS_VLAN_ACL_EN0,
    SYS_VLAN_PROP_EGRESS_VLAN_ACL_TCAM_LOOKUP_TYPE0,
    SYS_VLAN_PROP_EGRESS_VLAN_POLICER_EN,
    SYS_VLAN_PROP_EGRESS_STATSPTR,
    SYS_VLAN_PROP_PIM_SNOOP_EN,
    SYS_VLAN_PROP_PTP_CLOCK_ID,


    SYS_VLAN_PROP_MAX

};
typedef enum sys_vlan_property_e  sys_vlan_property_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_vlan_init(uint8 lchip, ctc_vlan_global_cfg_t* vlan_global_cfg);

extern int32
sys_usw_vlan_deinit(uint8 lchip);

extern int32
sys_usw_vlan_create_vlan(uint8 lchip, uint16 vlan_id);
extern int32
sys_usw_vlan_create_uservlan(uint8 lchip, ctc_vlan_uservlan_t* user_vlan);
extern int32
sys_usw_vlan_destory_vlan(uint8 lchip, uint16 vlan_id);
extern int32
sys_usw_vlan_get_vlan_ptr(uint8 lchip, uint16 vlan_id, uint16* vlan_ptr);

extern int32
sys_usw_vlan_add_port(uint8 lchip, uint16 vlan_id, uint32 gport);
extern int32
sys_usw_vlan_add_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);
extern int32
sys_usw_vlan_remove_port(uint8 lchip, uint16 vlan_id, uint32 gport);
extern int32
sys_usw_vlan_remove_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);
extern int32
sys_usw_vlan_get_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap);

extern int32
sys_usw_vlan_set_tagged_port(uint8 lchip, uint16 vlan_id, uint32 gport, bool tagged);
extern int32
sys_usw_vlan_set_tagged_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap);
extern int32
sys_usw_vlan_get_tagged_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap);

extern int32
sys_usw_vlan_set_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32 value);
extern int32
sys_usw_vlan_get_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32* value);

extern int32
sys_usw_vlan_set_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32  value);

extern int32
sys_usw_vlan_get_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32* p_value);

extern int32
sys_usw_vlan_set_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32 value);
extern int32
sys_usw_vlan_get_internal_property(uint8 lchip, uint16 vlan_id, sys_vlan_property_t vlan_prop, uint32* value);
extern int32
sys_usw_vlan_overlay_get_vni_fid_mapping_mode(uint8 lchip, uint8* mode);
extern int32
sys_usw_vlan_overlay_set_vni_fid_mapping_mode(uint8 lchip, uint8 mode);
extern int32
sys_usw_vlan_overlay_remove_fid (uint8 lchip,  uint32 vn_id);
extern int32
sys_usw_vlan_overlay_set_fid (uint8 lchip,  uint32 vn_id, uint16 fid );
extern int32
sys_usw_vlan_overlay_get_fid (uint8 lchip,  uint32 vn_id, uint16* p_fid );
extern int32
sys_usw_vlan_overlay_get_vn_id (uint8 lchip,  uint16 fid, uint32* p_vn_id);
extern int32
sys_usw_vlan_set_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop);
extern int32
sys_usw_vlan_get_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop);
extern int32
sys_usw_vlan_set_mac_auth(uint8 lchip, uint16 vlan_id, bool enable);
extern int32
sys_usw_vlan_get_mac_auth(uint8 lchip, uint16 vlan_id, bool* enable);
#ifdef __cplusplus
}
#endif

#endif

