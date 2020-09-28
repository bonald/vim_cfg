#ifndef _SYS_USW_NEXTHOP_H_
#define _SYS_USW_NEXTHOP_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_const.h"
#include "ctc_avl_tree.h"
#include "ctc_aps.h"
#include "ctc_vector.h"
#include "ctc_spool.h"
#include "ctc_debug.h"
#include "ctc_vlan.h"
#include "ctc_nexthop.h"
#include "ctc_stats.h"
#include "drv_api.h"

#define SYS_SPEC_L3IF_NUM                    4095
#define SYS_NH_INIT_CHECK \
    do { \
		 if ((lchip) >= sys_usw_get_local_chip_num(lchip)){  \
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");\
			return CTC_E_INVALID_CHIP_ID;\
 }\
        if (!p_usw_nh_api_master[lchip]){ SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_NH_MCAST_REPLI_INCR_STEP   4
#define SYS_NH_LOGICAL_REPLI_MAX_NUM   32
#define SYS_NH_EXTERNAL_NHID_MAX_BLK_NUM     16
#define SYS_NH_INTERNAL_NHID_MAX_BLK_NUM     16
#define SYS_NH_INTERNAL_NHID_BLK_SIZE        2048
#define SYS_NH_INTERNAL_NHID_MAX \
    ((g_usw_nh_master[lchip]->max_external_nhid) + (SYS_NH_INTERNAL_NHID_MAX_BLK_NUM * SYS_NH_INTERNAL_NHID_BLK_SIZE))

#define SYS_NH_HASH_MAX_BLK_NUM     256
#define SYS_NH_HASH_BLK_SIZE     1024

#define SYS_NH_INTERNAL_NHID_MAX_SIZE  (64*SYS_NH_HASH_BLK_SIZE)

#define SYS_DSNH_DESTVLANPTR_SPECIAL 0
#define SYS_NH_ROUTED_PORT_VID    0xFFF

#define SYS_NH_TUNNEL_ID_MAX_BLK_NUM     16
#define SYS_NH_ARP_ID_MAX_BLK_NUM       64
#define SYS_NH_MCAST_GROUP_MAX_BLK_SIZE     256
#define SYS_NH_MCAST_MEMBER_BITMAP_SIZE     64

#define SYS_NH_DSNH_OFFSET_MODEROUTED_PORT_VID    0xFFF

#define SYS_NH_CW_NUM    16

#define SYS_NH_IP_TUNNEL_IS_OVERLAY_TUNNEL(p_param)                     \
    ((CTC_TUNNEL_TYPE_NVGRE_IN4 == p_param->tunnel_info.tunnel_type)    \
     ||(CTC_TUNNEL_TYPE_NVGRE_IN6 == p_param->tunnel_info.tunnel_type)  \
     ||(CTC_TUNNEL_TYPE_VXLAN_IN4 == p_param->tunnel_info.tunnel_type)  \
     ||(CTC_TUNNEL_TYPE_VXLAN_IN6 == p_param->tunnel_info.tunnel_type)  \
     ||(CTC_TUNNEL_TYPE_GENEVE_IN4 == p_param->tunnel_info.tunnel_type)  \
     ||(CTC_TUNNEL_TYPE_GENEVE_IN6 == p_param->tunnel_info.tunnel_type))


#define SYS_NH_DSNH_INTERNAL_BASE (g_usw_nh_master[lchip]->internal_nexthop_base)
#define SYS_NH_DSNH_INTERNAL_SHIFT   4
#define SYS_NH_BUILD_INT_OFFSET(offset) \
    (SYS_NH_DSNH_INTERNAL_BASE<<SYS_NH_DSNH_INTERNAL_SHIFT)|offset;

#define SYS_DSNH4WREG_INDEX_FOR_BPE_TRANSPARENT   0x0      /*must 0,1 use dsnexthop8w for bpe ecid tranparent*/
#define SYS_DSNH4WREG_INDEX_FOR_BRG            0xC      /*must be b1100, for GB is b00, shift is 2*/
#define SYS_DSNH4WREG_INDEX_FOR_MIRROR         0xD      /*must be b1101, for GB is b01, shift is 2*/
#define SYS_DSNH4WREG_INDEX_FOR_BYPASS         0xE      /*must be b1110, for GB is b10, shift is 2, (use 8w)*/
#define SYS_DSNH4WREG_INDEX_FOR_WLAN_DECAP     0x2      /*reserve for wlan decap internal used*/

#define SYS_DSNH_INDEX_FOR_REMOTE_MIRROR  0  /*must be 0, Only Used for compatible for GG Ingress Edit Mode,
                                                  for D2 not need reserve remote mirror nexthop ptr*/
#define SYS_DSNH_INDEX_FOR_NONE 1               /*Reserved for doing nothing*/

#define SYS_NH_L2EDIT_HASH_BLOCK_SIZE  16
#define SYS_NH_L2EDIT_HASH_BLOCK_NUM (1024/SYS_NH_L2EDIT_HASH_BLOCK_SIZE)


#define SYS_NH_MPLS_LABEL_EXP_SELECT_PACKET  0
#define SYS_NH_MPLS_LABEL_EXP_SELECT_MAP     1
#define SYS_NH_MAX_EXP_VALUE                 7

#define SYS_NH_CHECK_MPLS_EXP_VALUE(exp)                    \
    do {                                                        \
        if ((exp) > (7)){return CTC_E_INVALID_PARAM; }         \
    } while (0);

#define SYS_NH_CHECK_MPLS_LABVEL_VALUE(label)               \
    {                                                        \
        if ((label) > (0xFFFFF)){                                 \
            return CTC_E_INVALID_PARAM; }              \
    }

#define SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(type)                    \
    do {                                                        \
        if ((type) >= (MAX_CTC_NH_EXP_SELECT_MODE)){return CTC_E_INVALID_PARAM; }         \
    } while (0);

#define GPORT_TO_INDEX(gport)  \
    ((SYS_MAP_CTC_GPORT_TO_GCHIP(gport) == 0x1f) ? \
    (MCHIP_CAP(SYS_CAP_NEXTHOP_PORT_NUM_PER_CHIP) * MCHIP_CAP(SYS_CAP_NEXTHOP_MAX_CHIP_NUM) + SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport)) : \
    (SYS_MAP_CTC_GPORT_TO_GCHIP(gport) * MCHIP_CAP(SYS_CAP_NEXTHOP_PORT_NUM_PER_CHIP) + SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport)))

struct sys_hbnh_brguc_node_info_s
{
    uint32 gport;
    uint32 nhid_brguc;
    uint32 nhid_bypass;

};
typedef struct sys_hbnh_brguc_node_info_s sys_hbnh_brguc_node_info_t;

struct sys_usw_brguc_info_s
{
    ctc_hash_t*   brguc_hash;    /*Maintains ucast bridge DB(no egress vlan trans)*/
    sal_mutex_t*  brguc_mutex;
};
typedef struct sys_usw_brguc_info_s sys_usw_brguc_info_t;

struct sys_nh_eloop_node_s
{
    ctc_slistnode_t head;
    uint32 nhid;
    uint32 edit_ptr;
    uint8 is_l2edit;
    uint8 rsv[1];
    uint16 ref_cnt;
};
typedef struct sys_nh_eloop_node_s sys_nh_eloop_node_t;

struct sys_usw_nh_api_master_s
{
    sys_usw_brguc_info_t brguc_info;
    uint32 max_external_nhid;
    sal_mutex_t*  p_mutex;
};
typedef struct sys_usw_nh_api_master_s sys_usw_nh_api_master_t;

enum sys_nh_entry_pld_op_type_e
{
    SYS_NH_OP_NONE = 0,
    SYS_NH_OP_ROUTE,
    SYS_NH_OP_BRIDGE,
    SYS_NH_OP_BRIDGE_VPLS,             /* BRIDGE_VPLS_VC for SGMAC */
    SYS_NH_OP_BRIDGE_INNER,
    SYS_NH_OP_MIRROR,
    SYS_NH_OP_ROUTE_NOTTL,
    SYS_NH_OP_ROUTE_COMPACT   /*L2Edit merge to Nexthop ipuc/ipmc*/
};
typedef enum sys_nh_entry_pld_op_type_e sys_nh_entry_pld_op_type_t;

enum sys_nh_entry_share_type_e
{
    SYS_NH_SHARE_TYPE_L23EDIT = 0,
    SYS_NH_SHARE_TYPE_VLANTAG = 1,
    SYS_NH_SHARE_TYPE_L2EDIT_VLAN = 2,
    SYS_NH_SHARE_TYPE_SPAN_PACKET = 3,
    SYS_NH_SHARE_TYPE_MAC_DA = 4,
};
typedef enum sys_nh_entry_share_type_e sys_nh_entry_share_type_t;

enum sys_nh_l3edit_tunnelv4_share_type_e
{
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_NULL = 0,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_GRE,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6RD,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_6TO4,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_NVGRE,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_VXLAN,
    SYS_NH_L3EDIT_TUNNELV4_SHARE_TYPE_WLAN
};
typedef enum sys_nh_l3edit_tunnelv4_share_type_e sys_nh_l3edit_tunnelv4_share_type_t;

enum sys_nh_l3edit_tunnelv6_share_type_e
{
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NULL = 0,
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_GRE,
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NORMAL,
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_NVGRE,
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_VXLAN,
    SYS_NH_L3EDIT_TUNNELV6_SHARE_TYPE_WLAN
};
typedef enum sys_nh_l3edit_tunnelv6_share_type_e sys_nh_l3edit_tunnelv6_share_type_t;

enum sys_nh_entry_l2edit_type_e
{
    SYS_NH_L2EDIT_TYPE_NONE =0,
    SYS_NH_L2EDIT_TYPE_LOOPBACK,
    SYS_NH_L2EDIT_TYPE_ETH_4W,
    SYS_NH_L2EDIT_TYPE_ETH_8W,
    SYS_NH_L2EDIT_TYPE_MAC_SWAP,
    SYS_NH_L2EDIT_TYPE_FLEX_8W,
    SYS_NH_L2EDIT_TYPE_PBB_4W,
    SYS_NH_L2EDIT_TYPE_PBB_8W,
    SYS_NH_L2EDIT_TYPE_OF,
    SYS_NH_L2EDIT_TYPE_INNER_SWAP,
    SYS_NH_L2EDIT_TYPE_INNNER_DS_LITE,
    SYS_NH_L2EDIT_TYPE_INNNER_DS_LITE_8W,
    SYS_NH_L2EDIT_TYPE_INVALID,

    SYS_NH_L2EDIT_TYPE_MAX
};
typedef enum sys_nh_entry_l2edit_type_e sys_nh_entry_l2edit_type_t;


enum sys_nh_l2_edit_extra_header_type_e
{
    SYS_L2EDIT_EXTRA_HEADER_TYPE_NONE = 0,
    SYS_L2EDIT_EXTRA_HEADER_TYPE_ETHERNET,
    SYS_L2EDIT_EXTRA_HEADER_TYPE_MPLS,
    SYS_L2EDIT_EXTRA_HEADER_TYPE_NO_L2_EDIT
};
typedef enum sys_nh_l2_edit_extra_header_type_e sys_nh_l2_edit_extra_header_type_t;

enum sys_nh_ds_type_e
{
    SYS_NH_DS_TYPE_L3EDIT = 0,
    SYS_NH_DS_TYPE_L2EDIT,
    SYS_NH_DS_TYPE_DISCARD = 2,

    DS_TYPE_RESERVED
};
typedef enum sys_nh_ds_type_e sys_nh_ds_type_t;

enum sys_nh_entry_l3edit_type_e
{
    SYS_NH_L3EDIT_TYPE_NONE =0,
    SYS_NH_L3EDIT_TYPE_MPLS_4W,
    SYS_NH_L3EDIT_TYPE_RESERVED,
    SYS_NH_L3EDIT_TYPE_NAT_4W,
    SYS_NH_L3EDIT_TYPE_NAT_8W,
    SYS_NH_L3EDIT_TYPE_TUNNEL_V4,
    SYS_NH_L3EDIT_TYPE_TUNNEL_V6,
    SYS_NH_L3EDIT_TYPE_L3FLEX,
    SYS_NH_L3EDIT_TYPE_OF8W,
    SYS_NH_L3EDIT_TYPE_OF16W,
    SYS_NH_L3EDIT_TYPE_LOOPBACK,
    SYS_NH_L3EDIT_TYPE_TRILL,
    SYS_NH_L3EDIT_TYPE_MPLS_12W,
    SYS_NH_L3EDIT_TYPE_MAX
};
typedef enum sys_nh_entry_l3edit_type_e sys_nh_entry_l3edit_type_t;

struct sys_nh_offset_attr_s
{
    uint32 offset_base;
    uint16 entry_size;
    uint16 entry_num;
};
typedef struct sys_nh_offset_attr_s sys_nh_offset_attr_t;

struct sys_nh_table_entry_info_s
{
    uint32 table_id;
    uint8  entry_size;
    uint8  opf_pool_type;
    uint8  alloc_dir; /*0:forward  alloc ,1,reverse alloc*/
    uint8  table_8w;

    int32 (*p_func)(uint8, void*, void*, uint32*);
    int32 (*p_update)(uint8, void*);
};
typedef struct sys_nh_table_entry_info_s sys_nh_table_entry_info_t;


struct sys_dsl2edit_of6w_s
{
  uint8    vlan_profile_id;
  ctc_misc_nh_flex_edit_param_t* p_flex_edit;
};
typedef struct sys_dsl2edit_of6w_s  sys_dsl2edit_of6w_t;

struct sys_dsl2edit_s
{

    uint32 offset;

    uint8 ds_type;
    uint8 l2_rewrite_type;
    uint8 derive_mcast_mac_for_ip;
    uint8 is_vlan_valid;

    uint16 output_vlan_id;
    uint8 is_6w;
    uint8 dynamic;

    uint8 cos_domain;
    uint8 use_port_mac;
    uint8 update_mac_sa;
    uint8 strip_inner_vlan;

    mac_addr_t mac_da;
    mac_addr_t mac_sa;
    /*added by wangl for hybrid*/
    uint8 packet_type;
    uint16  ether_type;
    uint8  is_span_ext_hdr;
    uint8 fpma;
    uint8 is_dot11;
    uint8 derive_mac_from_label;
    uint8 derive_mcast_mac_for_mpls;
    uint8 derive_mcast_mac_for_trill;
    uint8 map_cos;
    uint8 dot11_sub_type;
    uint32 dest_map;
    uint32 nexthop_ptr;
    uint8  nexthop_ext;

};
typedef struct sys_dsl2edit_s  sys_dsl2edit_t;

enum sys_usw_nh_param_type_e
{
    SYS_NH_PARAM_DSNH_TYPE_IPUC,
    SYS_NH_PARAM_DSNH_TYPE_BRGUC,
    SYS_NH_PARAM_DSNH_TYPE_BYPASS,
    SYS_NH_PARAM_DSNH_TYPE_IPMC,
    SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE,
    SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_ROUTE,
    SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_L2VPN,
    SYS_NH_PARAM_DSNH_TYPE_MPLS_PHP,
    SYS_NH_PARAM_DSNH_TYPE_RSPAN,
    SYS_NH_PARAM_DSNH_TYPE_MPLS_OP_NONE,
    SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL,
    SYS_NH_PARAM_DSNH_TYPE_TRILL,
    SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL_FOR_MIRROR,
    SYS_NH_PARAM_DSNH_TYPE_XERSPAN,
    SYS_NH_PARAM_DSNH_TYPE_OF,
    SYS_NH_PARAM_DSNH_TYPE_WLANTUNNEL,
    MAX_SYS_NH_PARAM_DSNH_TYPE
}
;
typedef enum sys_usw_nh_param_type_e sys_usw_nh_param_type_t;

enum sys_usw_nh_param_flag_e
{
    SYS_NH_PARAM_FLAG_USE_TTL_FROM_PKT      = 0x0001,    /*PIPE/uniform  .uniform:SYS_NH_PARAM_FLAG_USE_TTL_FROM_PKT  =1*/
    SYS_NH_PARAM_FLAG_USE_MAPPED_STAG_COS   = 0x0002,   /*if set, cos will use qos mapped  cos,else use packet parsered cos or user defined */
    SYS_NH_PARAM_FLAG_USE_MAPPED_CTAG_COS   = 0x0004,   /*if set, dscp will use qos mapped  cos,else use packet parsered cos */
    SYS_NH_PARAM_FLAG_USE_MAPPED_DSCP       = 0x0008,     /*if set, dscp will use qos mapped  dscp,else use packet parsered dscp */
    SYS_NH_PARAM_FLAG_VPLS_NEXTHOP          = 0x0010,
    SYS_NH_PARAM_FLAG_LOGIC_PORT_CHECK      = 0x0020,
    SYS_NH_PARAM_FLAG_L2EDIT_AS_APS_GROUP   = 0x0040,
    SYS_NH_PARAM_FLAG_SWAP_MAC              = 0x0080,
    SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_BRIDGE  = 0x0100,
    SYS_NH_PARAM_FLAG_PHP_SHORT_PIPE        = 0x0200,    /*PHP Shore Pipe*/
    SYS_NH_PARAM_FLAG_ROUTE_NOTTL           = 0x0400,
    SYS_NH_PARAM_FLAG_STATS_EN              = 0x0800,
    SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_VLAN   = 0x1000,
    SYS_NH_PARAM_FLAG_IP_TUNNEL_STRIP_INNER  = 0x2000,
    SYS_NH_PARAM_FLAG_ERSPAN_EN              = 0x4000,
    SYS_NH_PARAM_FLAG_ERSPAN_KEEP_IGS_TS     = 0x8000,
    SYS_NH_PARAM_FLAG_MISC_FLEX_PLD_BRIDGE   = 0x10000,
    SYS_NH_PARAM_FLAG_UDF_EDIT_EN            = 0x20000,
    SYS_NH_PARAM_FLAG_IP_TUNNEL_PLD_NONE     = 0x40000,
    MAX_SYS_NH_PARAM_FLAG
};
typedef enum sys_usw_nh_param_flag_e sys_usw_nh_param_flag_t;

struct sys_nh_param_dsnh_s
{
    uint32 dsnh_offset;
    uint32 l2edit_ptr;    /*arp*/
    uint32 lspedit_ptr;   /*spme/lsp*/
    uint32 l3edit_ptr;    /*inner l3/outer l3*/
    uint32 inner_l2edit_ptr;  /*inner l2*/
    uint16 fid;      /* used for overlaty tunnel only */
    uint16 span_id;
    ctc_vlan_egress_edit_info_t* p_vlan_info;
    uint16  dest_vlan_ptr;
    uint8   dsnh_type;    /*sys_usw_nh_param_type_t*/
    uint32  flag;
    uint8   use_ttl_from_pkt;
    uint8   hvpls;
    uint8   logic_port_check;
    uint8   dscp_en;
    uint16 logic_port;
    mac_addr_t macDa;        /**< 802.3 MAC address */

    uint8 hash_num;
    uint8 cw_index;
    uint8 spme_mode;
    uint8 wlan_tunnel_type;
    uint8 rid;
    uint8 wlan_mc_en;
    uint8 is_dot11;
    uint8 stag_en;
    uint8 mtu_no_chk;
    uint8 loop_edit;
    uint16 stats_ptr;
    uint16 cid;
};
typedef struct sys_nh_param_dsnh_s sys_nh_param_dsnh_t;

struct sys_nh_param_hdr_s
{
    sys_usw_nh_type_t nh_param_type;
	sys_nh_entry_change_type_t change_type;
    uint32 nhid;
    uint8  is_internal_nh;
    uint8  have_dsfwd;
    uint8  stats_valid;
    uint8  is_drop;
	uint32 stats_id;
    uint32 dsfwd_offset;
    uint32 stats_ptr;
};
typedef struct sys_nh_param_hdr_s sys_nh_param_hdr_t;

struct sys_nh_param_com_s
{
    sys_nh_param_hdr_t hdr;
};
typedef struct sys_nh_param_com_s sys_nh_param_com_t;

struct sys_nh_param_special_s
{
    sys_nh_param_hdr_t hdr;
};
typedef struct sys_nh_param_special_s sys_nh_param_special_t;

struct sys_nh_param_brguc_s
{
    sys_nh_param_hdr_t hdr;
    sys_nh_param_brguc_sub_type_t nh_sub_type;
    ctc_vlan_egress_edit_info_t* p_vlan_edit_info;
    ctc_vlan_egress_edit_info_t* p_vlan_edit_info_prot_path;
    uint32 dsnh_offset; /*Used for vlan editing*/
    uint16 gport; /*gport or aps bridge group id*/
    uint16 dest_vlan_ptr;
    uint32 loop_nhid;
    uint32    fwd_offset;
    ctc_vlan_edit_nh_param_t* p_vlan_edit_nh_param;
};
typedef struct sys_nh_param_brguc_s sys_nh_param_brguc_t;

struct sys_nh_param_mcast_s
{
    sys_nh_param_hdr_t hdr;
    uint32 groupid;        /*for mcast profile, means profile id*/
    uint8 opcode;
    uint8 is_mirror;
    uint8 is_mcast_profile;    /*means mcast group is mcast profile*/
    uint8 rsv;
    sys_nh_param_mcast_member_t* p_member;

};
typedef struct sys_nh_param_mcast_s sys_nh_param_mcast_t;

struct sys_nh_param_ipuc_s
{
    sys_nh_param_hdr_t hdr;
    uint8 is_unrov_nh;
    ctc_ip_nh_param_t* p_ipuc_param;
};
typedef struct sys_nh_param_ipuc_s sys_nh_param_ipuc_t;

struct sys_nh_param_ecmp_s
{
    sys_nh_param_hdr_t hdr;
    ctc_nh_ecmp_nh_param_t* p_ecmp_param;
};
typedef struct sys_nh_param_ecmp_s sys_nh_param_ecmp_t;

struct sys_nh_param_mpls_s
{
    sys_nh_param_hdr_t hdr;
    ctc_mpls_nexthop_param_t* p_mpls_nh_param;
    uint32 oam_lm_en      : 1;
    uint32 p_oam_lm_en    : 1;
    uint32 oam_lock_en    : 1;
    uint32 p_oam_lock_en  : 1;
    uint32 oam_mep_index  : 12;
    uint32 p_oam_mep_index  : 12;
    uint32 is_working     : 1;
    uint32 rsv            : 3;
};
typedef struct sys_nh_param_mpls_s sys_nh_param_mpls_t;

/* IP Tunnel nexthop info*/
struct sys_nh_param_ip_tunnel_s
{
    sys_nh_param_hdr_t hdr;
    ctc_ip_tunnel_nh_param_t* p_ip_nh_param;
};
typedef struct sys_nh_param_ip_tunnel_s sys_nh_param_ip_tunnel_t;

struct sys_nh_param_wlan_tunnel_s
{
    sys_nh_param_hdr_t hdr;
    ctc_nh_wlan_tunnel_param_t* p_wlan_nh_param;
};
typedef struct sys_nh_param_wlan_tunnel_s sys_nh_param_wlan_tunnel_t;

struct sys_nh_param_trill_s
{
    sys_nh_param_hdr_t hdr;
    ctc_nh_trill_param_t* p_trill_nh_param;
};
typedef struct sys_nh_param_trill_s sys_nh_param_trill_t;


struct sys_nh_param_iloop_s
{
    sys_nh_param_hdr_t hdr;
    ctc_loopback_nexthop_param_t* p_iloop_param;
};
typedef struct sys_nh_param_iloop_s sys_nh_param_iloop_t;

struct sys_nh_param_rspan_s
{
    sys_nh_param_hdr_t hdr;
    ctc_rspan_nexthop_param_t* p_rspan_param;
    uint32 dsnh_offset;
};
typedef struct sys_nh_param_rspan_s sys_nh_param_rspan_t;

struct sys_nh_param_crscnt_s
{
    bool swap_mac;
    uint16 srcport;
    uint16 destport;
};
typedef struct sys_nh_param_crscnt_s sys_nh_param_crscnt_t;

struct sys_nh_param_misc_s
{
    sys_nh_param_hdr_t hdr;
    ctc_misc_nh_param_t* p_misc_param;
};
typedef struct sys_nh_param_misc_s sys_nh_param_misc_t;


struct sys_nh_info_arp_param_s
{
    uint8 nh_entry_type; /*sys_usw_nh_type_t*/
    uint32 nh_id;
    uint8 is_aps;
    uint32 (* updateNhp)(uint8 lchip, void* arp_info);
};
typedef struct sys_nh_info_arp_param_s sys_nh_info_arp_param_t;

enum sys_nh_arp_flag_e
{
    SYS_NH_ARP_FLAG_DROP                =  0x01,
    SYS_NH_ARP_FLAG_REDIRECT_TO_CPU     =  0x02,
    SYS_NH_ARP_VLAN_VALID               =  0x04,
    SYS_NH_ARP_ECMP_IF                  =  0x08,
    SYS_NH_ARP_PORT_VALID               =  0x10,
    SYS_NH_ARP_IPUC_L2_EDIT             =  0x20,
    SYS_NH_ARP_MAX
};
typedef enum sys_nh_arp_flag_e sys_nh_arp_flag_t;


struct sys_nh_db_arp_s
{
    uint32 arp_id;
    mac_addr_t mac_da;
    uint8   flag;
    uint8   l3if_type;       /**< the type of l3interface , CTC_L3IF_TYPE_XXX */
    uint16  l3if_id;
    uint32  offset;          /**< outer l2 edit for mpls/tunnel vlan & mac edit*/
    uint32  in_l2_offset;    /**< inner l2 edit for ipuc vlan edit*/

    uint16  output_vid;
    uint8   output_vlan_is_svlan;
    uint8   nh_entry_type;

    uint32  destmap_profile;  /**< destmap profile id, used for mpls and tunnel, 0 means do not use destmap profile*/
    uint32  gport;            /**< outgoing port*/

    uint16  ref_cnt;

    uint32  nh_id;
    uint32 (* updateNhp)(uint8 lchip, void* arp_info);
};
typedef struct sys_nh_db_arp_s sys_nh_db_arp_t;



/*=== DB  entry struct  ===*/
struct sys_nh_db_dsl2editeth4w_s
{
    mac_addr_t mac_da;
    uint8 is_ecmp_if;
    uint8 strip_inner_vlan;

    uint16 output_vid;
    uint8  output_vlan_is_svlan;
    uint8  dynamic;

    uint8 is_share_mac;
    uint8 fpma;
    uint8 is_dot11;
    uint8 derive_mac_from_label;

    uint8 derive_mcast_mac_for_mpls;
    uint8 derive_mcast_mac_for_trill;
    uint8 map_cos;
    uint8 dot11_sub_type;

    uint8 derive_mcast_mac_for_ip;
    uint8 mac_da_en;
};
typedef struct sys_nh_db_dsl2editeth4w_s sys_nh_db_dsl2editeth4w_t;

struct sys_nh_db_dsl2editeth8w_s
{
    mac_addr_t mac_da;
    uint8 is_ecmp_if;
    uint8 strip_inner_vlan;

    uint16 output_vid;
    uint8  output_vlan_is_svlan;
    uint8  dynamic;

    uint8 is_share_mac;
    uint8 fpma;
    uint8 is_dot11;
    uint8 derive_mac_from_label;

    uint8 derive_mcast_mac_for_mpls;
    uint8 derive_mcast_mac_for_trill;
    uint8 map_cos;
    uint8 dot11_sub_type;

    uint8 derive_mcast_mac_for_ip;
    uint8 mac_da_en;
    /*keep above the same with sys_nh_db_dsl2editeth4w_t*/

    mac_addr_t mac_sa;
    uint8 update_mac_sa;
    uint8 packet_type;
    uint8 cos_domain;
    uint8 rsv;
    uint16 ether_type;
};
typedef struct sys_nh_db_dsl2editeth8w_s sys_nh_db_dsl2editeth8w_t;

struct sys_dsl3edit_tunnelv4_s
{

    uint8 share_type;

    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 map_ttl;
    uint8 ttl;
    uint8 copy_dont_frag;
    uint8 dont_frag;
    uint8 derive_dscp;  /*1:use assign; 1: map; 2:copy*/
    uint8 dscp;
    uint8 dscp_domain;

    uint8 ip_protocol_type;
    uint8 mtu_check_en;
    uint16 mtu_size;

    uint32 ipda;
    uint32 ipsa;
    uint32 ipsa_6to4;
    uint8 ipsa_valid;
    uint8 ipsa_index;

    uint8 inner_header_valid;
    uint8 inner_header_type;
    uint32 stats_ptr;

    uint16 gre_protocol;
    uint8 gre_version;
    uint8 gre_flags;
    uint32 gre_key;

    uint8 isatp_tunnel;

    uint8 tunnel6_to4_da;
    uint8 tunnel6_to4_sa;
    uint8 tunnel6_to4_da_ipv4_prefixlen;
    uint8 tunnel6_to4_da_ipv6_prefixlen;
    uint8 tunnel6_to4_sa_ipv4_prefixlen;
    uint8 tunnel6_to4_sa_ipv6_prefixlen;

    uint8 out_l2edit_valid;
    uint8 xerspan_hdr_en;
    uint8 xerspan_hdr_with_hash_en;

    uint32 l2edit_ptr;

    uint8 is_evxlan;
    uint8 evxlan_protocl_index; /* use for getting protocl from EpePktProcCtl.array[evxlan_protocl_index].vxlanProtocolType,
       how to config the index refer to sys_usw_overlay_tunnel_init(lchip)*/

    uint8 is_geneve     : 1;
    uint8 is_gre_auto   : 1;
    uint8 is_vxlan_auto : 1;
    uint8 rsv           : 5;
    uint8 udp_src_port_en;  /* used for vxlan udp src port */
    uint16 l4_src_port;
    uint16 l4_dest_port;
    uint8 encrypt_en;
    uint8 encrypt_id;
    uint16 bssid_bitmap;
    uint8 rmac_en;
    uint8 data_keepalive;
    uint8 ecn_mode;  /*0:copy, 1:map*/

    uint8 sc_index;
    uint8 sci_en : 1;
    uint8 cloud_sec_en    : 1;
    uint8 udf_profile_id  : 6;

    uint16 udf_edit[4];

};
typedef struct sys_dsl3edit_tunnelv4_s  sys_dsl3edit_tunnelv4_t;

struct sys_dsl3edit_tunnelv6_s
{

    uint8 share_type;

    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 map_ttl;
    uint8 ttl;
    uint8 derive_dscp;
    uint8 tos;
    uint8 dscp_domain;

    uint8 ip_protocol_type;
    uint8 mtu_check_en;
    uint16 mtu_size;

    ipv6_addr_t ipda;
    uint8 ipsa_index;

    uint8 inner_header_valid;
    uint8 inner_header_type;
    uint32 stats_ptr;

    uint16 gre_protocol;
    uint8 gre_version;
    uint8 gre_flags;
    uint32 gre_key;

    uint8 new_flow_label_valid;
    uint8 new_flow_label_mode;
    uint32 flow_label;
    uint8 out_l2edit_valid;

    uint8 xerspan_hdr_en;
    uint8 xerspan_hdr_with_hash_en;
    uint32 l2edit_ptr;

    uint8 is_evxlan;
    uint8 evxlan_protocl_index; /* use for getting protocl from EpePktProcCtl.array[evxlan_protocl_index].vxlanProtocolType,
       how to config the index refer to sys_usw_overlay_tunnel_init(lchip)*/

    uint8 is_geneve     : 1;
    uint8 is_gre_auto   : 1;
    uint8 is_vxlan_auto : 1;
    uint8 rsv           : 5;
    uint8 udp_src_port_en;  /* used for vxlan udp src port */
    uint16 l4_dest_port;
    uint16 l4_src_port;
    uint8 encrypt_en;
    uint8 encrypt_id;
    uint16 bssid_bitmap;
    uint8 rmac_en;
    uint8 data_keepalive;
    uint8 ecn_mode;  /*0:copy, 1:map*/

    uint8 sc_index;
    uint8 sci_en : 1;
    uint8 cloud_sec_en    : 1;
    uint8 udf_profile_id  : 6;

    uint16 udf_edit[4];
};
typedef struct sys_dsl3edit_tunnelv6_s  sys_dsl3edit_tunnelv6_t;

struct sys_dsl3edit_nat4w_s
{
    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 ip_da_prefix_length;
    uint8 ipv4_embeded_mode;

    uint32 ipda;
    uint16 l4_dest_port;
    uint8 replace_ip_da;
    uint8 replace_l4_dest_port;
};
typedef struct sys_dsl3edit_nat4w_s  sys_dsl3edit_nat4w_t;

struct sys_dsl3edit_nat8w_s
{
    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 ip_da_prefix_length;
    uint8 ipv4_embeded_mode;

    ipv6_addr_t ipda;
    uint16 l4_dest_port;
    uint8 replace_ip_da;
    uint8 replace_l4_dest_port;

};
typedef struct sys_dsl3edit_nat8w_s  sys_dsl3edit_nat8w_t;

struct sys_dsmpls_s
{

    uint32 offset;

    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 next_editptr_valid;
    uint8 outer_editptr_type;
    uint16 outer_editptr;

    uint16 logic_dest_port;
    uint16 mpls_oam_index;
    uint8 oam_en;
    uint32 statsptr;

    uint32 label;
    uint8 label_type;
    uint8 mcast_label;
    uint8 entropy_label_en;
    uint8 src_dscp_type;
    uint8 derive_label;
    uint8 derive_exp;
    uint8 exp;
    uint8 map_ttl_mode;
    uint8 map_ttl;
    uint8 ttl_index;
    uint8 mpls_domain;
    uint8 discard;
    uint8 discard_type;

    uint32 label_full[10];    /*label+ttl+exp*/
    uint8  label_num;

};
typedef struct sys_dsmpls_s  sys_dsmpls_t;

struct sys_nh_db_edit_s
{
    uint8 edit_type;    /*refer to sys_nh_entry_table_type_t*/
    uint8 rsv0[3];
    union
    {
        sys_nh_db_dsl2editeth8w_t  l2edit_8w;
        sys_nh_db_dsl2editeth4w_t  l2edit_4w;
        sys_dsl3edit_tunnelv4_t l3edit_v4;
        sys_dsl3edit_tunnelv6_t l3edit_v6;
        sys_dsl3edit_nat4w_t   nat_4w;
        sys_dsl3edit_nat8w_t   nat_8w;
        sys_dsmpls_t           l3edit_mpls;
    }data;

    uint32 calc_key_len[0];
    uint32 offset;
};
typedef struct sys_nh_db_edit_s sys_nh_db_edit_t;

struct sys_nh_info_special_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16 dest_gport;
    uint16 rsv;
};
typedef struct sys_nh_info_special_s sys_nh_info_special_t;

struct sys_nh_info_rspan_s
{
    sys_nh_info_com_hdr_t hdr;
    uint8 remote_chip;
    uint8 rsv[3];
};
typedef struct sys_nh_info_rspan_s sys_nh_info_rspan_t;

struct sys_nh_info_brguc_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16  dest_gport;
    uint8 nh_sub_type; /*sys_nh_param_brguc_sub_type_t*/
    uint8 rsv0;
    uint16 dest_logic_port;
	uint16 service_id;
    uint32 loop_nhid;
};
typedef struct sys_nh_info_brguc_s sys_nh_info_brguc_t;

struct sys_nh_info_ipuc_edit_info_s{

   uint16  l3ifid;
   uint16  arp_id;
   mac_addr_t  mac_da;
   uint32 l2_edit_ptr;
   sys_nh_db_dsl2editeth4w_t* p_dsl2edit;
};
typedef struct sys_nh_info_ipuc_edit_info_s sys_nh_info_ipuc_edit_info_t;

struct sys_nh_oam_info_s
{
	uint32 mep_index :16;
	uint32 mep_type  :8;
    uint32 rsv       :8;
    uint16 ref_cnt;
};
typedef struct sys_nh_oam_info_s sys_nh_oam_info_t;

struct sys_nh_ref_oam_node_s
{
    sys_nh_oam_info_t ref_oam;
    struct sys_nh_ref_oam_node_s* p_next;
};
typedef struct sys_nh_ref_oam_node_s sys_nh_ref_oam_node_t;

enum sys_nh_ipuc_flag_e
{
    SYS_NH_IPUC_FLAG_MERGE_DSFWD        = 0x0001
};
typedef enum sys_nh_ipuc_flag_e sys_nh_ipuc_flag_t;

struct sys_nh_info_ipuc_s
{
    sys_nh_info_com_hdr_t             hdr;
    mac_addr_t  mac_da;
    uint16  flag;
    uint16 l3ifid;
    uint32 gport;
    uint16 arp_id;
    uint8 l2edit_8w;
    uint8 rsv[1];
    uint32 stats_id;
    sys_nh_info_ipuc_edit_info_t   *protection_path;
	sys_nh_db_dsl2editeth4w_t* p_dsl2edit;          /*working path*/
    uint32 l2_edit_ptr;
    void* data;
    int32 (* updateAd) (uint8 lchip,void* ipuc_node, void* dsnh_info);
    sys_nh_ref_oam_node_t* p_ref_oam_list;   /*sys_nh_ref_list_node_t*/
};
typedef struct sys_nh_info_ipuc_s sys_nh_info_ipuc_t;

enum sys_nh_dsmet_flag_e
{
    SYS_NH_DSMET_FLAG_IS_LINKAGG              =  0x0001,
    SYS_NH_DSMET_FLAG_ENDLOCAL                =  0x0002,
    SYS_NH_DSMET_FLAG_PORT_CHK_DISCARD        =  0x0004,
    SYS_NH_DSMET_FLAG_LOGIC_PORT_CHK          =  0x0008,
    SYS_NH_DSMET_FLAG_USE_DSNH8W              =  0x0010,
    SYS_NH_DSMET_FLAG_USE_PBM                 =  0x0020,
    SYS_NH_DSMET_FLAG_USE_RSV_NH              =  0x0040,
    SYS_NH_DSMET_FLAG_APS_SHARE_NH            =  0x0080,
    SYS_NH_DSMET_FLAG_HORIZON_SPLIT_EN        =  0x0100,
    SYS_NH_DSMET_FLAG_REFECTIVE               =  0x0200,
    SYS_NH_DSMET_FLAG_DESTMAP_PROFILE         =  0x0400,
    SYS_NH_DSMET_FLAG_LEAF_CHECK_EN           =  0x0800,
    SYS_NH_DSMET_FLAG_CLOUD_SEC_EN            =  0x1000,
    SYS_NH_DSMET_FLAG_APPEND_DROP             =  0x2000,
    SYS_NH_DSMET_FLAG_MAX
};
typedef enum sys_nh_dsmet_flag_e sys_nh_dsmet_flag_t;

struct sys_nh_info_dsmet_s
{
    uint16   flag; /*sys_nh_dsmet_flag_t,SYS_NH_DSMET_FLAG_XX*/
    uint16   logic_port;
    uint32   dsmet_offset;
    uint32   next_dsmet_offset;
    uint32   dsnh_offset;
    uint32   ref_nhid;
    uint16   ucastid;
    uint16   vid; /*For IPMC*/
    uint16    replicate_num; /*replicate_num for IPMC; port num of l2mc port bitmap*/

    uint16* vid_list;
    uint32   pbm[4];
    uint8    free_dsnh_offset_cnt;
    uint8    member_type;
    uint8    port_type;
    uint8    entry_type;
};
typedef struct sys_nh_info_dsmet_s sys_nh_info_dsmet_t;
struct sys_nh_mcast_meminfo_s
{
    ctc_list_pointer_node_t list_head;
    sys_nh_info_dsmet_t dsmet;
};
typedef struct sys_nh_mcast_meminfo_s sys_nh_mcast_meminfo_t;

struct sys_nh_info_ecmp_s
{
    sys_nh_info_com_hdr_t hdr;
    uint8          ecmp_cnt;    /*include unresoved nh member*/
    uint8          valid_cnt;   /*valid member*/
    uint8          type;        /*ctc_nh_ecmp_type_t*/
    uint8          failover_en;
    uint32         ecmp_nh_id;
    uint16         ecmp_group_id;
    uint8          random_rr_en;
    uint8          stats_valid;
    uint32* nh_array;
    uint8* member_id_array;     /* member id of each ecmp member entry for dlb */
    uint8* entry_count_array; /* ecmp member entry count of each member id for dlb */

    uint16 gport;
    uint32 l3edit_offset_base;
    uint32 l2edit_offset_base;

    uint16 ecmp_member_base;
    uint8 mem_num;  /*max member num in group*/
    uint8 resv;
    uint32 stats_id;
};
typedef struct sys_nh_info_ecmp_s sys_nh_info_ecmp_t;

enum sys_nh_mpls_tunnel_flag_e
{
    SYS_NH_MPLS_TUNNEL_FLAG_STATS          =  0x01,
    SYS_NH_MPLS_TUNNEL_FLAG_APS  =  0x02,
    SYS_NH_MPLS_TUNNEL_FLAG_TWO_LAYER_APS  =  0x04,
    SYS_NH_MPLS_TUNNEL_FLAG_WORKING  =  0x08,
    SYS_NH_MPLS_TUNNEL_FLAG_PROTECTION  =  0x10,
    SYS_NH_MPLS_TUNNEL_FLAG_EXSIT_SPME  =  0x20,
    SYS_NH_MPLS_TUNNEL_FLAG_IS_SPME  =  0x40,
    SYS_NH_MPLS_TUNNEL_FLAG_IS_L2EDIT8W  =  0x80,
    SYS_NH_MPLS_TUNNEL_FLAG_IS_SR  =  0x100,
    SYS_NH_MPLS_TUNNEL_FLAG_MAX
};

enum sys_nh_aps_e
{
    SYS_NH_APS_W,
    SYS_NH_APS_P,
    SYS_NH_APS_M
};

struct sys_nh_mpls_sr_s
{
    uint32 dsnh_offset;
    uint32 pw_offset;
    uint32 lsp_offset;
    uint32 spme_offset;
    uint32 l2edit_offset;
};
typedef struct sys_nh_mpls_sr_s sys_nh_mpls_sr_t;

struct sys_nh_db_mpls_tunnel_s
{
    uint32  gport; /*if SYS_NH_MPLS_TUNNEL_FLAG_APS is set, it is aps group id*/
    uint16  l3ifid;
    uint16  tunnel_id;
    uint16  ref_cnt;
    uint16   flag; /*sys_nh_dsmet_flag_t,SYS_NH_MPLS_TUNNEL_FLAG_XX*/
    uint8    label_num;
    uint8    sr_loop_num[SYS_NH_APS_M];

    uint16  l2edit_offset[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  spme_offset[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  lsp_offset[SYS_NH_APS_M];

    uint16  stats_ptr ;
    uint16  p_stats_ptr;
    void*   p_l3_edit;

    sys_nh_db_dsl2editeth4w_t*  p_dsl2edit_4w[SYS_NH_APS_M][SYS_NH_APS_M];

    uint16 arp_id[SYS_NH_APS_M][SYS_NH_APS_M];
    sys_nh_ref_list_node_t* p_ref_nh_list;   /*sys_nh_ref_list_node_t*/
    sys_nh_mpls_sr_t* sr[SYS_NH_APS_M];
    uint32 loop_nhid[SYS_NH_APS_M][SYS_NH_APS_M];
};
typedef struct sys_nh_db_mpls_tunnel_s sys_nh_db_mpls_tunnel_t;

struct sys_nh_info_mpls_edit_info_s
{
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel;
    uint16 dsl3edit_offset;
    uint32 loop_nhid;
};
typedef struct sys_nh_info_mpls_edit_info_s sys_nh_info_mpls_edit_info_t;


struct sys_nh_info_mpls_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16   aps_group_id;
    uint32   gport;
    uint16   l3ifid;
	uint16   dest_logic_port; /*H-vpls*/
	uint16    service_id; /*Service Qos*/
    uint8   cw_index;
    uint16  arp_id;
    sys_nh_info_mpls_edit_info_t working_path;
    sys_nh_info_mpls_edit_info_t* protection_path;  /*when PW aps enbale*/
    sys_nh_db_dsl2editeth4w_t* p_dsl2edit;
    void*  p_dsl2edit_inner;
    uint32 inner_l2_ptr;
    uint32 outer_l2_ptr;
    uint32 nhid;
	void* data;
    int32 (* updateAd) (uint8 lchip,void* mpls_node, void* dsnh_info);
    uint16  ma_idx;
    uint16  dsma_valid: 1;
    uint16  rsv       :15;
    sys_nh_ref_oam_node_t* p_ref_oam_list;   /*sys_nh_ref_list_node_t*/
};
typedef struct sys_nh_info_mpls_s sys_nh_info_mpls_t;

enum sys_nh_ip_tunnel_flag_e
{
    SYS_NH_IP_TUNNEL_FLAG_IN_V4       =  0x0001,
    SYS_NH_IP_TUNNEL_FLAG_IN_V6       =  0x0002,
    SYS_NH_IP_TUNNEL_FLAG_IVI_6TO4    =  0x0004,
    SYS_NH_IP_TUNNEL_FLAG_IVI_4TO6    =  0x0008,
    SYS_NH_IP_TUNNEL_FLAG_NAT_V4      =  0x0010,
    SYS_NH_IP_TUNNEL_FLAG_L2EDIT8W    =  0x0020,
    SYS_NH_IP_TUNNEL_REROUTE          =  0x0040,
    SYS_NH_IP_TUNNEL_KEEP_IGS_TS      =  0x0080,
    SYS_NH_IP_TUNNEL_LOOP_NH          =  0x0100,
    SYS_NH_IP_TUNNEL_FLAG_MAX
};
typedef enum sys_nh_ip_tunnel_flag_e sys_nh_ip_tunnel_flag_t;


struct sys_nh_info_ip_tunnel_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16 l3ifid;
    uint16 ecmp_if_id;
    uint32 gport;
    uint16  flag;           /* sys_nh_ip_tunnel_flag_t */
    uint8  sa_index;
    uint8  rsv;
    uint32 dsl3edit_offset;
    uint32 loop_nhid;

    uint16 dest_vlan_ptr;
    uint16 span_id;
    uint32 inner_l2_edit_offset;
    uint32 outer_l2_edit_offset;
    sys_nh_db_dsl2editeth4w_t*  p_dsl2edit_4w;
    void*  p_l3edit;
    void*  p_dsl2edit_inner;

    uint32 vn_id;
    uint16 dest_logic_port; /*H-overlay*/
    uint16 arp_id;
	void* data;
    int32 (* updateAd) (uint8 lchip,void* ipuc_node, void* dsnh_info);

    uint16 dot1ae_channel;
    uint8  sc_index;
    uint8  sci_en            : 1;
    uint8  dot1ae_en         : 1;
    uint8  udf_profile_id    : 6;
};
typedef struct sys_nh_info_ip_tunnel_s sys_nh_info_ip_tunnel_t;

enum sys_nh_wlan_tunnel_flag_e
{
    SYS_NH_WLAN_TUNNEL_FLAG_IN_V4       =  0x0001,
    SYS_NH_WLAN_TUNNEL_FLAG_IN_V6       =  0x0002,
    SYS_NH_WLAN_TUNNEL_FLAG_MAX
};
typedef enum sys_nh_wlan_tunnel_flag_e sys_nh_wlan_tunnel_flag_t;

struct sys_nh_info_wlan_tunnel_s
{
    sys_nh_info_com_hdr_t hdr;
    uint32 gport;
    uint32 dsl3edit_offset;

    uint8  flag; /* sys_nh_wlan_tunnel_flag_t */
    uint8 sa_index;
    uint16 dest_vlan_ptr;

    void*  p_dsl2edit_inner;
    void*  p_dsl3edit_tunnel;
    uint8  use_multi_l2_ptr;   /*for l3 mcast fallbackbridge*/
    uint8  l2_edit_type;
    uint8 rsv[2];

    uint32 inner_l2_ptr;
};
typedef struct sys_nh_info_wlan_tunnel_s sys_nh_info_wlan_tunnel_t;

struct sys_nh_info_trill_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16 ingress_nickname;
    uint16 egress_nickname;
    uint16 l3ifid;
    uint32 gport;
    uint32 dsl3edit_offset;
    sys_nh_db_dsl2editeth4w_t*  p_dsl2edit_4w;
    uint16 dest_vlan_ptr;
    uint32 l2_edit_ptr;
	void* data;
    int32 (* updateAd) (uint8 lchip,void* trill_node, void* dsnh_info);
};
typedef struct sys_nh_info_trill_s sys_nh_info_trill_t;

typedef uint32 (* updatenh_fn ) (uint8 lchip, void* p_arp_db);

typedef int32 (* p_sys_nh_create_cb_t)(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, \
                                       sys_nh_info_com_t* p_com_db);
typedef int32 (* p_sys_nh_delete_cb_t)(uint8 lchip, sys_nh_info_com_t* p_data, \
                                       uint32* p_nhid);
typedef int32 (* p_sys_nh_update_cb_t)(uint8 lchip, sys_nh_info_com_t* p_data, \
                                       sys_nh_param_com_t* p_com_nh_para);
struct sys_nh_ip_tunnel_sa_v4_node_s
{
    uint32  count;
    ip_addr_t ipv4;
};
typedef struct sys_nh_ip_tunnel_sa_v4_node_s sys_nh_ip_tunnel_sa_v4_node_t;

struct sys_nh_ip_tunnel_sa_v6_node_s
{
    uint32  count;
    ipv6_addr_t ipv6;
};
typedef struct sys_nh_ip_tunnel_sa_v6_node_s sys_nh_ip_tunnel_sa_v6_node_t;

struct sys_dsl2edit_vlan_edit_s
{
    DsOfEditVlanActionProfile_m data;
	uint8 profile_id;
	uint8 rsv2[3];

};
typedef struct sys_dsl2edit_vlan_edit_s sys_dsl2edit_vlan_edit_t;

struct sys_nh_mcast_traverse_s
{
    void*   user_data;
    ctc_nh_mcast_traverse_fn fn;
    uint8   lchip;
};
typedef struct sys_nh_mcast_traverse_s sys_nh_mcast_traverse_t;

struct sys_usw_nh_master_s
{
    p_sys_nh_create_cb_t callbacks_nh_create[SYS_NH_TYPE_MAX];
    p_sys_nh_delete_cb_t callbacks_nh_delete[SYS_NH_TYPE_MAX];
    p_sys_nh_update_cb_t callbacks_nh_update[SYS_NH_TYPE_MAX];
    uint32            max_external_nhid;
    uint16            max_tunnel_id;
    uint32            ipmc_dsnh_offset[SYS_SPEC_L3IF_NUM];
    uint16            ecmp_if_resolved_l2edit;
    uint16            reflective_resolved_dsfwd_offset;

    ctc_hash_t*       nhid_hash;
    ctc_vector_t*     tunnel_id_vec;
    ctc_vector_t*     mcast_group_vec;
    ctc_hash_t*       arp_id_hash;
    ctc_spool_t*      p_edit_spool;
    ctc_spool_t*      p_l2edit_vprof;
    ctc_slist_t*      eloop_list;
    ctc_slist_t*      repli_offset_list;

    sys_nh_offset_attr_t sys_nh_resolved_offset[SYS_NH_RES_OFFSET_TYPE_MAX];
    sys_nh_table_entry_info_t nh_table_info_array[SYS_NH_ENTRY_TYPE_MAX];

    uint32            fatal_excp_base;
    uint32            max_glb_nh_offset;
    uint32            max_glb_met_offset;
    uint32*           p_occupid_nh_offset_bmp;       /*Occupid global DsNexthop  offset bitmap, */
    uint32*           p_occupid_met_offset_bmp;      /*Occupid global DsMET  offset bitmap, */

    uint8             ipmc_logic_replication;
    uint8             pkt_nh_edit_mode;            /*0: ingress edit, 1: bridge is ingress edit, ipuc/ip-tunnel/mpls/misc depends on dsnh offset is zero or not,  2: misc edit,  */
	uint8             no_dsfwd ;

    uint16            cw_ref_cnt[SYS_NH_CW_NUM];

    uint16            max_ecmp;                    /**<  the maximum ECMP paths allowed for a route */
    uint16            max_ecmp_group_num;

    uint16            cur_ecmp_cnt;
    uint8             reflective_brg_en;
    uint8             ip_use_l2edit;
    uint16            rspan_vlan_id;                /**<rsapn vlan id, TBD, Remove*/
    uint32            nhid_used_cnt[SYS_NH_TYPE_MAX];
    uint32            nhtbl_used_cnt[SYS_NH_ENTRY_TYPE_MAX];


    uint8             nh_opf_type;
    uint8             ecmp_group_opf_type;
    uint8             ecmp_member_opf_type;
    uint8             met_mode;      /**<indicate first met use 6w or 3w, 0:3w, 1:6w*/

    uint32            internal_nexthop_base;
    void *            ipsa[MAX_CTC_IP_VER];

	/*Stats share with cid and bpe ecid*/
    uint8             cid_use_4w;   /**<global ctl cid use 4w or 8w, if using 4w, nexthop stats function cannot used, default using 4w mode*/
    uint8             bpe_en;       /**<global ctl bpe use ,if using 4w, nexthop stats function cannot used, default using stats mode*/

    uint8             vxlan_opf_type_inner_fid;

    uint32            xlate_nh_offset;
    uint32            mpls_edit_offset;    /* for xgpon reserved*/
    ctc_hash_t*       vxlan_vni_hash;      /* used to record vn_id and fid map */
    uint32            internal_nh_used_num;
    uint16            udf_profile_ref_cnt[8];
    uint32            udf_profile_bitmap;
    uint16            udf_ether_type[8];
    uint32            efd_nh_id;          /* efd global redirect nexthop id*/
};
typedef struct sys_usw_nh_master_s sys_usw_nh_master_t;

struct sys_nh_vni_mapping_key_s
{
    uint32 vn_id;

    uint16 fid;
    uint16 ref_cnt;
};
typedef struct sys_nh_vni_mapping_key_s sys_nh_vni_mapping_key_t;

#define SYS_NH_CREAT_LOCK(mutex_type)                   \
    do                                                  \
    {                                                   \
        sal_mutex_create(&mutex_type);                   \
        if (NULL == mutex_type){                         \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); }  \
    } while (0)

#define SYS_NH_BRGUC_LOCK \
    if(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex) sal_mutex_lock(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex)

#define SYS_NH_BRGUC_UNLOCK \
    if(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex) sal_mutex_unlock(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex)

#define SYS_NH_LOCK \
    if(p_usw_nh_api_master[lchip]->p_mutex) sal_mutex_lock(p_usw_nh_api_master[lchip]->p_mutex)

#define SYS_NH_UNLOCK \
    if(p_usw_nh_api_master[lchip]->p_mutex) sal_mutex_unlock(p_usw_nh_api_master[lchip]->p_mutex)

#define SYS_NH_DESTROY_LOCK(mutex_type) \
    if(mutex_type) sal_mutex_destroy(mutex_type)

#define SYS_NH_EDIT_MODE()   g_usw_nh_master[lchip]->pkt_nh_edit_mode

#define SYS_NH_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(nexthop, nexthop, NH_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

/*
   1. DsFwd.nexthopPtr[17]: global disable logic port type
   2. DsFwd.nexthopPtr[16]: Used to store loopback localPhyPort[7];
   3. DsFwd.nexthopPtr[15]: custom exp map valid
   4. DsFwd.nexthopPtr[14:12]: packet type
   5. DsFwd.nexthopPtr[11]: packet type valid
   6. DsFwd.nexthopPtr[10]: vpls port type;
   7. DsFwd.nexthopPtr[9:8]:bytes_to_remove; cm_packet_header.next_hop_ext
   8. DsFwd.nexthopPtr[7]:  use outer ttl
   9. DsFwd.nexthopPtr[6:0]: Used to store loopback localPhyPort[6:0];
*/
#define SYS_NH_ENCODE_ILOOP_DSNH(lport, logic_port, cid_valid, remove_words, clear_logic_port) \
    ((lport & 0x7F) | (((lport>>7)&0x1)<<16)| ((remove_words & 0x3) << 8) | (logic_port << 10) | (clear_logic_port <<17) )


extern sys_usw_nh_master_t* g_usw_nh_master[CTC_MAX_LOCAL_CHIP_NUM];

extern int32
sys_usw_nh_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg);
extern int32
sys_usw_nh_get_nh_master(uint8 lchip, sys_usw_nh_master_t** p_nh_master);

extern int32
sys_usw_nh_mpls_add_cw(uint8 lchip, uint32 cw, uint8* cw_index,bool is_add);

extern int32
sys_usw_nh_add_mcast_db(uint8 lchip, uint32 group_id, void* data);

extern int32
sys_usw_nh_del_mcast_db(uint8 lchip, uint32 group_id);

extern int32
sys_usw_nh_traverse_mcast_db(uint8 lchip, vector_traversal_fn2 fn, void* data);

extern int32
sys_usw_nh_write_entry_dsnh4w(uint8 lchip, sys_nh_param_dsnh_t* p_dsnh_param);
extern int32
sys_usw_nh_write_entry_dsnh8w(uint8 lchip, sys_nh_param_dsnh_t* p_dsnh_param);

extern int32
sys_usw_nh_write_entry_dsfwd(uint8 lchip, sys_nh_param_dsfwd_t* p_dsfwd_param);

extern int32
sys_usw_nh_check_max_glb_met_offset(uint8 lchip, uint32 offset);

extern int32
sys_usw_nh_check_max_glb_nh_offset(uint8 lchip, uint32 offset);

extern int32
sys_usw_nh_get_max_glb_nh_offset(uint8 lchip, uint32* offset);

extern int32
_sys_usw_nh_set_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set);

extern int32
sys_usw_nh_set_glb_nh_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set);
extern int32
_sys_usw_nh_check_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,
                                      bool should_not_inuse);
extern int32
sys_usw_nh_check_glb_nh_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,
                                     bool should_not_inuse);
extern int32
sys_usw_nh_get_resolved_offset(uint8 lchip, sys_usw_nh_res_offset_type_t type,
                                     uint32* p_offset);
extern int32
_sys_usw_nh_get_nhinfo(uint8 lchip, uint32 nhid, sys_nh_info_dsnh_t* p_nhinfo);
extern int32
sys_usw_nh_get_ipmc_dsnh_offset(uint8 lchip, uint16 l3ifid, uint8 mtu_no_chk, uint32* p_dsnh_offset);
extern int32
sys_usw_nh_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_nh_info_com_t** pp_nhinfo);
extern int32
sys_usw_nh_get_mirror_info_by_nhid(uint8 lchip, uint32 nhid, uint32* dsnh_offset, uint32* gport, bool enable);
extern int32
sys_usw_nh_global_dync_entry_set_default(uint8 lchip, uint32 min_offset, uint32 max_offset);
extern int32
sys_usw_nh_offset_alloc(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32* p_offset);
extern int32
sys_usw_nh_offset_alloc_with_multiple(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                            uint32 entry_num, uint16 multi, uint32* p_offset);
extern int32
sys_usw_nh_reverse_offset_alloc(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                      uint32 entry_num, uint32* p_offset);
extern int32
sys_usw_nh_reverse_offset_alloc_with_multiple(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                                    uint32 entry_num, uint16 multi, uint32* p_offset);
extern int32
sys_usw_nh_offset_free(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                             uint32 entry_num, uint32 offset);
extern int32
sys_usw_nh_copy_nh_entry_flags(uint8 lchip, sys_nh_info_com_hdr_t* p_old_hdr,sys_nh_info_com_hdr_t *p_new_hdr);
extern int32
sys_usw_nh_create_brguc_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_brguc_cb(uint8 lchip, sys_nh_info_com_t* data, uint32* p_nhid);

extern int32
sys_usw_nh_update_brguc_cb(uint8 lchip, sys_nh_info_com_t* p_com_db,
                                  sys_nh_param_com_t* p_com_nh_para);
extern int32
sys_usw_nh_create_mcast_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_mcast_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);
extern int32
sys_usw_nh_update_mcast_cb(uint8 lchip, sys_nh_info_com_t* p_nh_info,
                                 sys_nh_param_com_t* p_para /*Member info*/);
extern int32
sys_usw_nh_create_ipuc_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_ipuc_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);
extern int32
sys_usw_nh_update_ipuc_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para);

extern int32
sys_usw_nh_create_special_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                   sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_special_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);

extern int32
sys_usw_nh_update_ecmp_member(uint8 lchip, uint32 nhid, uint8 change_type);

extern int32
sys_usw_nh_create_ecmp_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db);

extern int32
sys_usw_nh_delete_ecmp_cb(uint8 lchip, sys_nh_info_com_t* p_ecmp_param, uint32* p_nhid);

extern int32
sys_usw_nh_update_ecmp_cb(uint8 lchip, sys_nh_info_com_t* p_ecmp_info, sys_nh_param_com_t* p_ecmp_param);

extern int32
_sys_usw_nh_get_max_ecmp(uint8 lchip, uint16* max_ecmp);
extern int32
sys_usw_nh_remove_route_l2edit_outer(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w);

extern int32
sys_usw_nh_remove_l2edit_6w_inner(uint8 lchip, sys_nh_db_dsl2editeth8w_t* p_dsl2edit_8w);

extern int32
sys_usw_nh_add_route_l2edit_outer(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint32* p_offset);   /*DsL2Edit*/

extern int32
sys_usw_nh_add_route_l2edit_8w_outer(uint8 lchip, sys_nh_db_dsl2editeth8w_t** p_dsl2edit_8w, uint32* p_offset);

extern int32
sys_usw_nh_remove_route_l2edit_8w_outer(uint8 lchip, sys_nh_db_dsl2editeth8w_t* p_dsl2edit_8w);

extern int32
sys_usw_nh_add_l2edit_3w_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint8 type, uint32* p_offset);  /*DsL2Edit*/

extern int32
sys_usw_nh_remove_l2edit_3w_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w, uint8 l2_edit_type);

extern int32
sys_usw_nh_add_l2edit_6w_inner(uint8 lchip, sys_nh_db_dsl2editeth8w_t** p_dsl2edit_8w, uint32* p_offset);

extern int32
sys_usw_nh_add_swap_l2edit_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t** p_dsl2edit_4w, uint32* p_offset);

extern int32
sys_usw_nh_remove_swap_l2edit_inner(uint8 lchip, sys_nh_db_dsl2editeth4w_t* p_dsl2edit_4w);

extern int32
sys_usw_nh_lkup_mpls_tunnel(uint8 lchip, uint16 tunnel_id, sys_nh_db_mpls_tunnel_t** pp_mpls_tunnel);

int32
sys_usw_nh_add_mpls_tunnel(uint8 lchip, uint16 tunnel_id, sys_nh_db_mpls_tunnel_t* p_mpls_tunnel);
int32
sys_usw_nh_remove_mpls_tunnel(uint8 lchip, uint16 tunnel_id);
extern int32
sys_usw_nh_create_mpls_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_mpls_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);
extern int32
sys_usw_nh_update_mpls_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para);
extern int32
sys_usw_nh_create_iloop_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_iloop_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);
extern int32
sys_usw_nh_update_iloop_cb(uint8 lchip, sys_nh_info_com_t* p_iloop_info, sys_nh_param_com_t* p_iloop_param);

extern int32
sys_usw_nh_create_rspan_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_rspan_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);

extern int32
sys_usw_nh_create_misc_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db);
extern int32
sys_usw_nh_delete_misc_cb(uint8 lchip, sys_nh_info_com_t* p_flex_info, uint32* p_nhid);
extern int32
sys_usw_nh_update_misc_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para);

extern int32
sys_usw_nh_misc_init(uint8 lchip);

extern int32
sys_usw_nh_api_create(uint8 lchip, sys_nh_param_com_t* p_nh_com_para);
extern int32
sys_usw_nh_api_delete(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nhid_type);
extern int32
sys_usw_nh_api_update(uint8 lchip, uint32 nhid, sys_nh_param_com_t* p_nh_com_para);
extern int32
sys_usw_nh_api_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg);

extern int32
sys_usw_nh_get_entry_dsfwd(uint8 lchip, uint32 dsfwd_offset, void* p_dsfwd);

extern int32
sys_usw_nh_update_entry_dsfwd(uint8 lchip, uint32 *offset,
                               uint32 dest_map,
                               uint32 dsnh_offset,
                               uint8 dsnh_8w,
                               uint8 del,
                               uint8 critical_packet);

extern int32
sys_usw_nh_get_reflective_brg_en(uint8 lchip, uint8 *enable);

extern int32
sys_usw_nh_add_stats_action(uint8 lchip, uint32 nhid, uint16 vrfid);

extern int32
sys_usw_nh_del_stats_action(uint8 lchip, uint32 nhid);

extern int32
sys_usw_nh_reset_stats_result(uint8 lchip, ctc_nh_stats_info_t* stats_info);

extern int32
sys_usw_nh_create_ip_tunnel_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                     sys_nh_info_com_t* p_com_db);

extern int32
sys_usw_nh_delete_ip_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);

extern int32
sys_usw_nh_update_ip_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                     sys_nh_param_com_t* p_para);
extern int32
sys_usw_nh_ip_tunnel_update_dot1ae(uint8 lchip, void* param);

extern int32
sys_usw_nh_create_wlan_tunnel_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                     sys_nh_info_com_t* p_com_db);

extern int32
sys_usw_nh_delete_wlan_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);

extern int32
sys_usw_nh_update_wlan_tunnel_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                     sys_nh_param_com_t* p_para);

extern int32
sys_usw_nh_ip_tunnel_init(uint8 lchip);

extern int32
sys_usw_nh_create_trill_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db);

extern int32
sys_usw_nh_delete_trill_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid);

extern int32
sys_usw_nh_update_trill_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db, sys_nh_param_com_t* p_para);

extern int32
sys_usw_nh_offset_alloc_from_position(uint8 lchip, sys_nh_entry_table_type_t entry_type,uint32 entry_num, uint32 start_offset);

extern uint32
sys_usw_nh_get_arp_num(uint8 lchip);

extern sys_nh_db_arp_t*
sys_usw_nh_lkup_arp_id(uint8 lchip, uint16 arp_id);

extern int32
sys_usw_nh_add_arp_id(uint8 lchip, sys_nh_db_arp_t* p_arp);

extern int32
sys_usw_nh_remove_arp_id(uint8 lchip, sys_nh_db_arp_t* p_arp);

extern int32
sys_usw_nh_write_entry_arp(uint8 lchip, sys_nh_db_arp_t* p_ds_arp);

extern int32
sys_usw_nh_bind_arp_cb(uint8 lchip, sys_nh_info_arp_param_t* p_arp_parm, uint16 arp_id);
extern int32
sys_usw_nh_unbind_arp_cb(uint8 lchip, uint16 arp_id, uint8 use_cb);

extern int32
sys_usw_nh_get_arp_oif(uint8 lchip, uint16 arp_id, ctc_nh_oif_info_t *p_oif, uint8* p_mac, uint8* is_drop);
extern int32
sys_usw_nh_update_dsipda_cb(uint8 lchip, uint32 nhid, sys_nh_info_com_t* p_nh_db);
extern int32
sys_usw_nh_deinit(uint8 lchip);
extern int32
sys_usw_nh_add_l3edit_tunnel(uint8 lchip, void** p_dsl3edit, uint8 edit_type, uint32* p_offset);
extern int32
sys_usw_nh_remove_l3edit_tunnel(uint8 lchip, void* p_dsl3edit, uint8 edit_type);
extern int32
sys_usw_nh_swap_nexthop_offset(uint8 lchip, uint32 nhid, uint32 nhid2);
extern int32
sys_usw_nh_add_loopback_l2edit(uint8 lchip, uint32 nhid, uint8 is_l2edit, uint32* p_edit_ptr);
extern int32
sys_usw_nh_remove_loopback_l2edit(uint8 lchip, uint32 nhid, uint8 is_l2edit);
extern int32
sys_usw_nh_set_bpe_en(uint8 lchip, bool enable);
extern int32
_sys_usw_brguc_nh_create_by_type(uint8 lchip, uint32 gport, sys_nh_param_brguc_sub_type_t nh_type);
extern int32
_sys_usw_misc_nh_create(uint8 lchip, uint32 nhid, sys_usw_nh_type_t nh_param_type);
extern int32
_sys_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id);
extern int32
_sys_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param);
extern int32
_sys_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param);
extern int32
_sys_usw_nh_write_entry_dsfwd(uint8 lchip, sys_nh_param_dsfwd_t* p_dsfwd_param);
extern int32
_sys_usw_nh_get_nh_master(uint8 lchip, sys_usw_nh_master_t** p_out_nh_master);
extern int32
_sys_usw_nh_api_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_usw_nh_master_t* p_nh_master,
                                         sys_nh_info_com_t** pp_nhinfo);
extern int32
_sys_usw_nh_dsnh_init_for_bpe_transparent(uint8 lchip, sys_nh_offset_attr_t* p_offset_attr);
extern int32
_sys_usw_nh_vxlan_vni_init(uint8 lchip);

extern int32
sys_usw_nh_update_loopback_l2edit(uint8 lchip, uint32 nhid);

extern int32
sys_usw_nh_merge_dsfwd_disable(uint8 lchip, uint32 nhid);
extern int32
_sys_usw_nh_get_resolved_offset(uint8 lchip, sys_usw_nh_res_offset_type_t type, uint32* p_offset);
extern bool
_sys_usw_nh_is_ipmc_logic_rep_enable(uint8 lchip);
extern int32
_sys_usw_nh_set_ipmc_logic_replication(uint8 lchip, uint8 enable);
extern int32
_sys_usw_nh_free_offset_by_nhinfo(uint8 lchip, sys_usw_nh_type_t nh_type, sys_nh_info_com_t* p_nhinfo);
extern int32
_sys_usw_nh_get_mcast_nh(uint8 lchip, uint32 group_id, uint32* nhid);
extern int32
_sys_usw_nh_add_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit);
extern int32
_sys_usw_nh_remove_udf_profile(uint8 lchip, uint8 profile_id);
extern int32
_sys_usw_nh_get_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit);
extern int32
sys_usw_nh_update_ad(uint8 lchip, uint32 nhid, uint8 update_type, uint8 is_arp);
extern int32
_sys_usw_nh_update_oam_mep(uint8 lchip, sys_nh_ref_oam_node_t* p_head, uint16 arp_id);
extern int32
_sys_usw_nh_update_oam_ref_info(uint8 lchip, void* p_nhinfo, sys_nh_oam_info_t* p_oam_info, uint8 is_add);
extern int32
_sys_usw_nh_traverse_mcast(uint8 lchip, ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data);
extern int32
_sys_usw_nexthop_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param);
#ifdef __cplusplus
}
#endif

#endif /*_SYS_USW_NEXTHOP_INTERNAL_H_*/

