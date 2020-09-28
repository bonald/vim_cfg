/**
   @file sys_usw_scl.h

   @date 2017-01-23

   @version v1.0

   The file contains all scl APIs of sys layer

 */
 #ifndef _SYS_USW_SCL_API_H
#define _SYS_USW_SCL_API_H

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_common.h"
#include "ctc_scl.h"
#include "ctc_vlan.h"

#include "ctc_field.h"
#include "sys_usw_common.h"


/****************************************************************************
 *
 * Defines and Macros
 *
 *****************************************************************************/

/*the inner Group ID:
||6bit module_id ||6bit sub_type||3bit port_type||1bit lag||10bit port|
 mod id  : refer to ctc_feature_t,
 sub_type :module owner self-define
 port_type :refer to :ctc_field_port_type_t
 lag-is_linkagg
*/

#define SYS_SCL_MAX_UDF_ENTRY_NUM       16
#define SCL_HASH_NUM    2

#define SYS_SCL_ENCODE_INNER_GRP_ID(modid, stype, ptype, lag, port) \
         (0x80000000 + (modid << 20 | stype << 14 | ptype << 11 | lag << 10 | port))

#define SYS_SCL_GET_INNER_GRP_MOD_ID(grp_id) ((grp_id >> 20) & 0x3F)

/* used by show only */
#define SYS_SCL_VLAN_ID_CHECK(vlan_id)           \
    {                                            \
        if ((vlan_id) > CTC_MAX_VLAN_ID) {       \
            return CTC_E_BADID; } \
    }                                            \

#define SYS_SCL_IS_HASH_COM_MODE(pe) (!SCL_ENTRY_IS_TCAM(pe->key_type) && \
 !(pe->resolve_conflict) && (CTC_INGRESS == pe->direction) && !(p_usw_scl_master[lchip]->hash_mode))

typedef enum sys_scl_fwd_type_e
{
    SYS_SCL_FWD_TYPE_DSFWD = 0,
    SYS_SCL_FWD_TYPE_ECMP,
    SYS_SCL_FWD_TYPE_VSI,
    SYS_SCL_FWD_TYPE_VRF,
    SYS_SCL_FWD_TYPE_NUM
} sys_scl_fwd_type_t;

typedef enum
{
    SYS_SCL_KEY_TCAM_VLAN,
    SYS_SCL_KEY_TCAM_MAC,
    SYS_SCL_KEY_TCAM_IPV4,
    SYS_SCL_KEY_TCAM_IPV4_SINGLE,
    SYS_SCL_KEY_TCAM_IPV6,
    SYS_SCL_KEY_TCAM_IPV6_SINGLE,
    SYS_SCL_KEY_TCAM_UDF,       /**< [TM] udf key 160 */
    SYS_SCL_KEY_TCAM_UDF_EXT,       /**< [TM] udf key 320 */
    SYS_SCL_KEY_HASH_PORT,
    SYS_SCL_KEY_HASH_PORT_CVLAN,
    SYS_SCL_KEY_HASH_PORT_SVLAN,
    SYS_SCL_KEY_HASH_PORT_2VLAN,
    SYS_SCL_KEY_HASH_PORT_CVLAN_COS,
    SYS_SCL_KEY_HASH_PORT_SVLAN_COS,
    SYS_SCL_KEY_HASH_MAC,
    SYS_SCL_KEY_HASH_PORT_MAC,
    SYS_SCL_KEY_HASH_SVLAN_MAC,
    SYS_SCL_KEY_HASH_SVLAN,
    SYS_SCL_KEY_HASH_IPV4,
    SYS_SCL_KEY_HASH_PORT_IPV4,
    SYS_SCL_KEY_HASH_IPV6,
    SYS_SCL_KEY_HASH_L2,
    SYS_SCL_KEY_HASH_PORT_IPV6,
    SYS_SCL_KEY_HASH_PORT_SVLAN_DSCP,
    SYS_SCL_KEY_HASH_TUNNEL_IPV4,
    SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE,
    SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF,
    SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA,
    SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA,
    SYS_SCL_KEY_HASH_TUNNEL_IPV6,
    SYS_SCL_KEY_HASH_TUNNEL_IPV6_UDP,
    SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY,
    SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_UC_V4_MODE1,  /* ipdaindex + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_MC_V4_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_V4_MODE1,     /* used by both uc and mc, ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_UC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_NVGRE_MC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_UC_V4_MODE1,  /* ipdaindex + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_MC_V4_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_V4_MODE1,     /* used by both uc and mc, ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_UC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE0,  /* ipda + vn_id*/
    SYS_SCL_KEY_HASH_VXLAN_MC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    SYS_SCL_KEY_HASH_WLAN_IPV4,
    SYS_SCL_KEY_HASH_WLAN_IPV6,
    SYS_SCL_KEY_HASH_WLAN_RMAC,
    SYS_SCL_KEY_HASH_WLAN_RMAC_RID,
    SYS_SCL_KEY_HASH_WLAN_STA_STATUS,
    SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS,
    SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD,
    SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD,
    SYS_SCL_KEY_HASH_TUNNEL_MPLS,
    SYS_SCL_KEY_HASH_ECID,
    SYS_SCL_KEY_HASH_ING_ECID,
    SYS_SCL_KEY_HASH_PORT_CROSS,
    SYS_SCL_KEY_HASH_PORT_VLAN_CROSS,
    SYS_SCL_KEY_HASH_TRILL_UC,          /* ingressNickname + egressNickname */
    SYS_SCL_KEY_HASH_TRILL_UC_RPF,      /* ingressNickname */
    SYS_SCL_KEY_HASH_TRILL_MC,          /* ingressNickname + egressNickname */
    SYS_SCL_KEY_HASH_TRILL_MC_RPF,      /* ingressNickname + egressNickname */
    SYS_SCL_KEY_HASH_TRILL_MC_ADJ,      /* globalSrcPort + egressNickname */
    SYS_SCL_KEY_PORT_DEFAULT_INGRESS,
    SYS_SCL_KEY_PORT_DEFAULT_EGRESS,
    SYS_SCL_KEY_NUM
}sys_scl_key_type_t;

enum sys_scl_field_key_type_e
{
    SYS_SCL_FIELD_KEY_IP_DA_INDEX = CTC_FIELD_KEY_NUM,
    SYS_SCL_FIELD_KEY_L4_USER_TYPE,
    SYS_SCL_FIELD_KEY_NAMESPACE,
    SYS_SCL_FIELD_KEY_ECID,
    SYS_SCL_FIELD_KEY_COMMON,
    SYS_SCL_FIELD_KEY_AD_INDEX,
    SYS_SCL_FIELD_KEY_MPLS_LABEL,
    SYS_SCL_FIELD_KEY_MPLS_LABEL_SPACE,
    SYS_SCL_FIELD_KEY_MPLS_IS_INTERFACEID,

    SYS_SCL_FIELD_KEY_NUM
};
typedef enum sys_scl_field_key_type_e sys_scl_field_key_type_t;

enum sys_scl_field_action_type_e
{
    /*Ingress Egress Flow share*/

    SYS_SCL_FIELD_ACTION_TYPE_WLAN_DECAP  = CTC_SCL_FIELD_ACTION_TYPE_NUM,     /**< sys_scl_wlan_decap_t */
    SYS_SCL_FIELD_ACTION_TYPE_TRILL,                  /**< sys_scl_trill_t */
    SYS_SCL_FIELD_ACTION_TYPE_OVERLAY,                /**< sys_scl_overlay_t */
    SYS_SCL_FIELD_ACTION_TYPE_IP_TUNNEL_DECAP,        /**< sys_scl_iptunnel_t */
    SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT,            /**< sys_scl_wlan_encap_t */
    SYS_SCL_FIELD_ACTION_TYPE_ROUTER_MAC,
    SYS_SCL_FIELD_ACTION_TYPE_QOS_USE_OUTER_INFO,
    SYS_SCL_FIELD_ACTION_TYPE_IPFIX_USE_OUTER_INFO,
    SYS_SCL_FIELD_ACTION_TYPE_DOT1BR_PE,              /**< sys_scl_dot1br_t */
    SYS_SCL_FIELD_ACTION_TYPE_ARP_ACTION,             /**< ctc_port_arp_action_type_t */
    SYS_SCL_FIELD_ACTION_TYPE_IS_HALF,

    SYS_SCL_FIELD_ACTION_TYPE_VN_ID,                  /**< data0: vn_id */
    SYS_SCL_FIELD_ACTION_TYPE_VPLS,                   /**< just for show */
    SYS_SCL_FIELD_ACTION_TYPE_VPWS,                   /**< just for show */
    SYS_SCL_FIELD_ACTION_TYPE_MPLS,                     /**< Mpls Action, refer to ds_t. */
    SYS_SCL_FIELD_ACTION_TYPE_RPF_CHECK,
    SYS_SCL_FIELD_ACTION_TYPE_DOT1AE_INGRESS,       /**< [TM] Dot1Ae ingress process */
    SYS_SCL_FIELD_ACTION_TYPE_IPV4_BASED_L2MC,      /**< [TM] Force MAC DA Lookup to IPv4 Lookup, data0: enable use 1 and disable use 0 */
    SYS_SCL_FIELD_ACTION_TYPE_ACTION_COMMON,        /**< [TM] Only for dsType use */
    SYS_SCL_FIELD_ACTION_TYPE_AWARE_TUNNEL_INFO_EN, /**< [TM] If set, indicate use pkt inner info as lookup key, and outer l4 info can also be used as lookup key */
    SYS_SCL_FIELD_ACTION_TYPE_MAX

};
typedef enum sys_scl_field_action_type_e sys_scl_field_action_type_t;

typedef enum
{
    SYS_SCL_ACTION_INGRESS,
    SYS_SCL_ACTION_EGRESS,
    SYS_SCL_ACTION_TUNNEL,
    SYS_SCL_ACTION_FLOW,
    SYS_SCL_ACTION_MPLS,
    SYS_SCL_ACTION_NUM
}sys_scl_action_type_t;

enum sys_scl_tunnel_action_tunnel_type_e
{
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_WLAN ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_VXLAN ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_NVGRE ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_TRILL ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_PBB ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_GRE ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_IPV6_IN_IP ,
    SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_MAX
};
typedef enum sys_scl_tunnel_action_tunnel_type_e sys_scl_tunnel_action_tunnel_type_t;

typedef struct
{
    uint32 stag_op     : 3,
           svid_sl     : 2,
           scos_sl     : 2,
           scfi_sl     : 2,
           ctag_op     : 3,
           cvid_sl     : 2,
           ccos_sl     : 2,
           ccfi_sl     : 2,
           vlan_domain : 2,
           tpid_index  : 2,
           rsv         : 10;

}sys_scl_sw_vlan_edit_t;

struct sys_scl_dot1br_s
{
    uint8   lport_valid;
    uint16  lport;
    uint8   src_gport_valid;
    uint32  src_gport;
    uint8   src_discard;
    uint8   exception_en;
    uint8   bypass_all;
};
typedef struct sys_scl_dot1br_s sys_scl_dot1br_t;

struct sys_scl_wlan_client_s
{
    uint8   forward_on_status;
    uint8   not_local_sta;
    uint8   sta_roam_status;
    uint8   sta_status_chk_en;
    uint8   is_vlan_edit;
    uint16  vrfid;
    ctc_scl_vlan_edit_t vlan_edit;
    uint8 acl_lkup_num;
    ctc_acl_property_t acl_prop[CTC_MAX_ACL_LKUP_NUM];/**< [D2] overwrite acl */
};
typedef struct sys_scl_wlan_client_s sys_scl_wlan_client_t;

/* TM append dot1ae ingress process */
struct sys_scl_dot1ae_ingress_s
    {
        uint32 dot1ae_p2p_mode0_en:1;
        uint32 dot1ae_p2p_mode1_en:1;
        uint32 dot1ae_p2p_mode2_en:1;
        uint32 encrypt_disable_acl:1;
        uint32 need_dot1ae_decrypt:1;
        uint32 dot1ae_sa_index:7;
        uint32 rsv:20;
        uint16 dsfwdptr;
        uint16 logic_src_port;
  };
  typedef struct sys_scl_dot1ae_ingress_s sys_scl_dot1ae_ingress_t;

struct sys_scl_overlay_s
{
    uint8   is_vxlan;
    uint8   use_outer_ttl;
    uint8   ttl_check_en;
    uint8   inner_taged_chk_mode;
    uint8   payloadOffset;
    uint8   payload_pktType;
    uint8   is_tunnel;
    uint8   inner_packet_lookup;
    uint8   vlan_domain;
    uint8   logic_port_type;
    uint8   acl_qos_use_outer_info;
    uint8   acl_qos_use_outer_info_valid;
    uint8   acl_key_merge_inner_and_outer_hdr;
    uint8   acl_key_merge_inner_and_outer_hdr_valid;
    uint8   ipfix_and_microflow_use_outer_info;
    uint8   ipfix_and_microflow_use_outer_info_valid;
    uint8   classify_use_outer_info;
    uint8   classify_use_outer_info_valid;
    uint8   router_mac_profile_en;
    uint8   router_mac_profile;
    uint8   arp_ctl_en;
    uint8   arp_exception_type;
    uint8   gre_option;
    uint8   acl_lkup_num;
    uint16  svlan_tpid;
    uint32  logic_src_port;
    ctc_acl_property_t acl_pro[CTC_MAX_ACL_LKUP_NUM];
};
typedef struct sys_scl_overlay_s sys_scl_overlay_t;


struct sys_scl_wlan_decap_s
{
    uint8    user_default_vlan_tag_valid;
    uint8    user_default_vlan_valid ;
    uint8    inner_packet_lkup_en;
    uint8    ttl_check_en;
    uint8    is_tunnel;
    uint8    is_actoac_tunnel ;
    uint8    wlan_reassemble_en ;
    uint8    wlan_cipher_en ;
    uint16   user_default_vid ;
    uint8    user_default_cos ;
    uint8    user_default_cfi ;
    uint8    decrypt_key_index ;
    uint16   wlan_tunnel_id;
    uint16   ds_fwd_ptr;
    uint8   payloadOffset;
    uint8   payload_pktType;
    uint8  acl_lkup_num;
    ctc_acl_property_t acl_prop[CTC_MAX_ACL_LKUP_NUM];/**< [D2] overwrite acl */
};
typedef struct sys_scl_wlan_decap_s sys_scl_wlan_decap_t;


struct sys_scl_iptunnel_s
{
    uint8   payload_offset;
    uint8   payload_pktType;
    uint8   gre_options;
    uint8   tunnel_type;

    uint16  l3_if[CTC_IPUC_IP_TUNNEL_MAX_IF_NUM];                /* for rpf check*/
    ctc_acl_property_t acl_prop[CTC_MAX_ACL_LKUP_NUM];      /* overwrite acl*/

    uint32  nexthop_id;
    uint16  metadata;
    uint16  logic_port;
    uint8   acl_lkup_num;
    uint8   rsv0[3];

    uint32   inner_lkup_en :1;
    uint32   inner_pub_lkup_en :1;
    uint32   isatap_check_en :1;
    uint32   ttl_check_en :1;
    uint32   use_outer_ttl :1;
    uint32   rpf_check_en :1;
    uint32   rpf_check_fail_tocpu :1;
    uint32   nexthop_id_valid :1;
    uint32   meta_data_valid :1;
    uint32   rpf_check_request :1;
    uint32   classify_use_outer_info :1;
    uint32   classify_use_outer_info_valid :1;
    uint32   acl_use_outer_info :1;
    uint32   acl_use_outer_info_valid :1;
    uint32   rsv1:18;
};
typedef struct sys_scl_iptunnel_s sys_scl_iptunnel_t;

struct sys_scl_trill_s
{
    uint8    is_tunnel;
    uint8    inner_packet_lookup;
    uint8    bind_check_en;
    uint8    bind_mac_sa;
    uint8    multi_rpf_check;
    uint8    src_port_check_en;
    uint8    router_mac_profile_en;
    uint8    router_mac_profile;
    uint8    arp_ctl_en;
    uint8    arp_exception_type;
    uint32  src_gport;                      /**< [GG] Source gport */
    mac_addr_t  mac_sa;                      /**< [GG] Outer MACSA */
};
typedef struct sys_scl_trill_s sys_scl_trill_t;


#define  SYS_SCL_MAX_KEY_SIZE_IN_WORD 24     /* the current max key size is 96 bytes, that is 24 words */

typedef struct
{
    uint32 key[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    uint32 mask[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    ds_t  action;                      /* for tcam, DsUserId0Tcam is the same size with DsUserId, for egress, the ad size is smaller than DsUserId */

}sys_scl_sw_temp_entry_t;

struct sys_scl_lkup_key_s
{
    uint8  key_type;                            /**< [in] key type */
    uint8  action_type;                         /**< [in] action type */
    uint8  group_priority;                      /**< [in] group priority, used for deciding which tcam to look up */
    uint8  resolve_conflict;                    /**< [in] only used for resolve conflict, diff it with normal tcam that is resolve conflict entry is 1, normal tcam is 0 */
    uint32 entry_id;                            /**< [out] entry id */
    uint32 group_id;                            /**< [in] group id, add this to confirm that different application with the same key will not be incorrectly removed */
    sys_scl_sw_temp_entry_t temp_entry;    /**< [in] the key which used to be look up */

};
typedef struct sys_scl_lkup_key_s sys_scl_lkup_key_t;

struct sys_scl_default_action_s
{
    sys_scl_key_type_t  hash_key_type;
    uint8  action_type;
    uint8  is_vlan_class;
    uint8  is_rollback;  /* pending and data0 == 0 has two situations: 1.previous op success and install to hw; 2.previous op fail and do rollback */
    uint8  scl_id;
    uint16 lport;
    uint32 eid;

    ctc_vlan_miss_t* vlan_miss;
    uint32 ad_index[SCL_HASH_NUM];

    ctc_scl_field_action_t* field_action;
};
typedef struct sys_scl_default_action_s sys_scl_default_action_t;

extern int32
sys_usw_scl_init(uint8 lchip, void* scl_global_cfg);

extern int32
sys_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner);

extern int32
sys_usw_scl_destroy_group(uint8 lchip, uint32 group_id, uint8 inner);

extern int32
sys_usw_scl_install_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner);

extern int32
sys_usw_scl_uninstall_group(uint8 lchip, uint32 group_id, uint8 inner);

extern int32
sys_usw_scl_get_group_info(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner);

extern int32
sys_usw_scl_add_entry(uint8 lchip, uint32 group_id, ctc_scl_entry_t* scl_entry, uint8 inner);

extern int32
sys_usw_scl_remove_entry(uint8 lchip, uint32 entry_id, uint8 inner);

extern int32
sys_usw_scl_install_entry(uint8 lchip, uint32 entry_id, uint8 inner);

extern int32
sys_usw_scl_uninstall_entry(uint8 lchip, uint32 entry_id, uint8 inner);

extern int32
sys_usw_scl_remove_all_entry(uint8 lchip, uint32 group_id, uint8 inner);

extern int32
sys_usw_scl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority, uint8 inner);

extern int32
sys_usw_scl_get_multi_entry(uint8 lchip, ctc_scl_query_t* query, uint8 inner);

extern int32
sys_usw_scl_copy_entry(uint8 lchip, ctc_scl_copy_entry_t* copy_entry, uint8 inner);

extern int32
sys_usw_scl_update_action(uint8 lchip, uint32 entry_id, ctc_scl_action_t* action, uint8 inner);

extern int32
sys_usw_scl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key);

extern int32
sys_usw_scl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* p_field_key);

extern int32
sys_usw_scl_add_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);

extern int32
sys_usw_scl_get_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);

extern int32
sys_usw_scl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);

extern int32
sys_usw_scl_set_default_action(uint8 lchip, sys_scl_default_action_t* p_default_action);

extern int32
sys_usw_scl_get_default_action(uint8 lchip, sys_scl_default_action_t* action);

extern int32
sys_usw_scl_set_hash_field_sel(uint8 lchip, ctc_scl_hash_field_sel_t* field_sel);

extern int32
sys_usw_scl_get_group_type(uint8 lchip, uint32 entry_id, uint8* group_type);

extern int32
sys_usw_scl_construct_lkup_key(uint8 lchip, ctc_field_key_t* p_field_key, sys_scl_lkup_key_t* p_lkup_key);

/*sys_scl_lkup_key_t must be memset 0 before construct it*/
extern int32
sys_usw_scl_get_entry_id_by_lkup_key(uint8 lchip, sys_scl_lkup_key_t* p_lkup_key);

extern int32
sys_usw_scl_get_overlay_tunnel_entry_status(uint8 lchip, uint8 key_type, uint8 mod_id, uint32* entry_count);

extern int32
sys_usw_scl_get_overlay_tunnel_dot1ae_info(uint8 lchip, uint32 entry_id, sys_scl_dot1ae_ingress_t* p_dot1ae_ingress);

extern int32
sys_usw_scl_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif
#endif
