/**
 @file sys_usw_wb_nh.h

 @date 2010-01-13

 @version v2.0

 The file defines queue api
*/

#ifndef _SYS_USW_WB_NH_H_
#define _SYS_USW_WB_NH_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_warmboot.h"
#include "sys_usw_chip.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @brief  define l2 FDB entry sync flags
*/
enum sys_wb_appid_nh_subid_e
{
    SYS_WB_APPID_NH_SUBID_MASTER,
    SYS_WB_APPID_NH_SUBID_BRGUC_NODE,
    SYS_WB_APPID_NH_SUBID_INFO_COM,
    SYS_WB_APPID_NH_SUBID_MPLS_TUNNEL,
    SYS_WB_APPID_NH_SUBID_L2EDIT_4W,
    SYS_WB_APPID_NH_SUBID_L2EDIT_8W,
    SYS_WB_APPID_NH_SUBID_ECMP,
    SYS_WB_APPID_NH_SUBID_INFO_MCAST,
    SYS_WB_APPID_NH_SUBID_ARP,
    SYS_WB_APPID_NH_SUBID_VNI,
    SYS_WB_APPID_NH_SUBID_ELOOP
};
typedef enum sys_wb_appid_nh_subid_e sys_wb_appid_nh_subid_t;

enum sys_wb_nh_bind_type_e
{
    SYS_WB_NH_BIND_TYPE_NONE,
    SYS_WB_NH_BIND_TYPE_ACL,
    SYS_WB_NH_BIND_TYPE_IPUC,
    SYS_WB_NH_BIND_TYPE_MAX
};
typedef enum sys_wb_nh_bind_type_e sys_wb_nh_bind_type_t;

struct sys_wb_nh_info_special_s
{
    uint16 dest_gport;
    uint16 rsv;
};
typedef struct sys_wb_nh_info_special_s sys_wb_nh_info_special_t;

struct sys_wb_nh_info_rspan_s
{
    uint8 remote_chip;
    uint8 rsv[3];
};
typedef struct sys_wb_nh_info_rspan_s sys_wb_nh_info_rspan_t;

struct sys_wb_nh_info_brguc_s
{
    uint16  dest_gport;
    uint8   nh_sub_type; /*sys_nh_param_brguc_sub_type_t*/
    uint8   rsv0;
    uint16 dest_logic_port;
	uint16 service_id;
    uint32 loop_nhid;
    uint32 loop_edit_ptr;
};
typedef struct sys_wb_nh_info_brguc_s sys_wb_nh_info_brguc_t;

struct sys_wb_nh_bind_ipuc_s
{
    uint32 nh_id;
    uint8   rsv0[2];
    uint16 ad_offset;
    uint32 route_flag;  /* sys_ipuc_flag_e  ctc_ipuc_flag_e*/
    uint32 gport;       /* assign output port */
};
typedef struct sys_wb_nh_bind_ipuc_s sys_wb_nh_bind_ipuc_t;

struct sys_wb_nh_l2edit_s
{
    /*key*/
    mac_addr_t mac_da;
    uint16 output_vid;
    uint8  strip_inner_vlan;
    uint8  fpma;
    uint8  is_8w;

    /*data*/
    uint32 offset;
    uint8 is_ecmp_if;
    uint8  output_vlan_is_svlan;
    uint16 ref_cnt;
    uint8  dynamic;
    uint8  is_share_mac;
    uint8  is_dot11;
    uint8  derive_mac_from_label;
    uint8  derive_mcast_mac_for_mpls;
    uint8  derive_mcast_mac_for_trill;
    uint8  map_cos;
    uint8  dot11_sub_type;
    uint8  derive_mcast_mac_for_ip;
    mac_addr_t mac_sa;
    uint8 update_mac_sa;
    uint8 packet_type;
    uint8 cos_domain;
};
typedef struct sys_wb_nh_l2edit_s  sys_wb_nh_l2edit_t;

struct sys_wb_nh_l3edit_v4_s
{
    uint32 ipda;
    uint32 ipsa;
    uint32 ipsa_6to4;

    uint8 ipsa_valid;
    uint8 isatp_tunnel;
    uint8 tunnel6_to4_da;
    uint8 tunnel6_to4_sa;

    uint8 tunnel6_to4_da_ipv4_prefixlen;
    uint8 tunnel6_to4_da_ipv6_prefixlen;
    uint8 tunnel6_to4_sa_ipv4_prefixlen;
    uint8 tunnel6_to4_sa_ipv6_prefixlen;
};
typedef struct sys_wb_nh_l3edit_v4_s  sys_wb_nh_l3edit_v4_t;

struct sys_wb_nh_l3edit_v6_s
{
    ipv6_addr_t ipda;

    uint8 new_flow_label_valid;
    uint8 new_flow_label_mode;
    uint32 flow_label;
};
typedef struct sys_wb_nh_l3edit_v6_s  sys_wb_nh_l3edit_v6_t;

struct sys_wb_nh_l3edit_s
{
    uint8 share_type;
    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 map_ttl;
    uint8 ttl;
    uint8 dscp_domain;
    uint8 ip_protocol_type;
    uint8 mtu_check_en;
    uint16 mtu_size;
    uint8 ipsa_index;
    uint8 inner_header_valid;
    uint8 inner_header_type;
    uint16 stats_ptr;
    uint16 gre_protocol;
    uint8 gre_version;
    uint8 gre_flags;
    uint32 gre_key;
    uint8 out_l2edit_valid;
    uint8 xerspan_hdr_en;
    uint8 xerspan_hdr_with_hash_en;
    uint32 l2edit_ptr;
    uint8 is_evxlan;
    uint8 evxlan_protocl_index;
    uint8 is_geneve;
    uint16 l4_src_port;
    uint16 l4_dest_port;
    uint8 encrypt_en;
    uint8 encrypt_id;
    uint16 bssid_bitmap;
    uint8 rmac_en;
    uint8 data_keepalive;
    uint8 ecn_mode;
    union
    {
        sys_wb_nh_l3edit_v4_t v4;
        sys_wb_nh_l3edit_v6_t v6;
    }l3;

    uint32 udp_src_port_en : 1;
    uint32 copy_dont_frag  : 1;
    uint32 dont_frag       : 1;
    uint32 derive_dscp     : 1;
    uint32 dscp            : 8;
    uint32 rsv             : 20;
};
typedef struct sys_wb_nh_l3edit_s  sys_wb_nh_l3edit_t;

struct sys_wb_nh_nat_s
{
    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 ip_da_prefix_length;
    uint8 ipv4_embeded_mode;

    uint16 l4_dest_port;
    uint8 replace_ip_da;
    uint8 replace_l4_dest_port;
    union
    {
        uint32 ip_v4;
        ipv6_addr_t ip_v6;
    }ipda;
};
typedef struct sys_wb_nh_nat_s  sys_wb_nh_nat_t;

struct sys_wb_nh_info_ipuc_s
{
    uint16 flag;
    uint16 l3ifid;
    uint32 gport;
    uint16 arp_id;
    uint8  rsv[2];
    uint32 stats_id;
    uint16 protection_arp_id;
    uint16 protection_l3ifid;
    mac_addr_t  mac_da;
    mac_addr_t  protection_mac_da;
    sys_wb_nh_l2edit_t l2edit;
    sys_wb_nh_l2edit_t protect_l2edit;

    /*restore bind info, Notice: bind data should recover from fwd module, eg:ipuc/acl/mpls etc*/
};
typedef struct sys_wb_nh_info_ipuc_s sys_wb_nh_info_ipuc_t;

#define SYS_USW_MAX_ECPN 64
struct sys_wb_nh_info_ecmp_s
{
    uint8    ecmp_cnt;    /*include unresoved nh member*/
    uint8    valid_cnt;   /*valid member*/
    uint8    type;        /*ctc_nh_ecmp_type_t*/
    uint8    failover_en;
    uint32   ecmp_nh_id;
    uint16   ecmp_group_id;
    uint8    random_rr_en;
    uint8    stats_valid;
    uint32   nh_array[SYS_USW_MAX_ECPN];
    uint8    member_id_array[SYS_USW_MAX_ECPN];     /* member id of each ecmp member entry for dlb */
    uint8    entry_count_array[SYS_USW_MAX_ECPN]; /* ecmp member entry count of each member id for dlb */
    uint16 gport;
    uint16 ecmp_member_base;
    uint32 l3edit_offset_base;
    uint32 l2edit_offset_base;
    uint8   mem_num;  /*max member num in group*/
    uint8   rsv[3];
    uint32  stats_id;
};
typedef struct sys_wb_nh_info_ecmp_s sys_wb_nh_info_ecmp_t;


struct sys_wb_nh_info_mpls_s
{
    uint16   aps_group_id;
    uint8    rsv[2];
    uint32   gport;
    uint16   l3ifid;
    uint16   dest_logic_port; /*H-vpls*/
    uint8    cw_index;
    uint8    aps_enable;
    uint16   arp_id;
    sys_wb_nh_l2edit_t l2edit_inner;
    sys_wb_nh_l2edit_t l2edit_outer;
    uint32 inner_l2_ptr;
    uint32 outer_l2_ptr;
    uint16 dsl3edit_offset;
    uint16 tunnel_id;
    uint16 p_dsl3edit_offset;
    uint16 p_tunnel_id;
	uint16 service_id;
    uint16  ma_idx;
    uint16  dsma_valid: 1;
    uint16  rsv1      : 15;
    uint32  loop_nhid;
    uint32  loop_edit_ptr;
    uint32  p_loop_nhid;
    uint32  p_loop_edit_ptr;
};
typedef struct sys_wb_nh_info_mpls_s sys_wb_nh_info_mpls_t;

struct sys_wb_nh_info_wlan_s
{
    uint32  dsl3edit_offset;
    uint32  gport;
    uint8   flag;                /* sys_nh_wlan_tunnel_flag_t */
    uint8   sa_index;
    uint16  dest_vlan_ptr;
    sys_wb_nh_l2edit_t l2edit_inner;
    sys_wb_nh_l3edit_t l3_edit;
    uint32 inner_l2_ptr;
};
typedef struct sys_wb_nh_info_wlan_s sys_wb_nh_info_wlan_t;

struct sys_wb_nh_info_ip_tunnel_s
{
    uint16 l3ifid;
    uint16 ecmp_if_id;
    uint32 gport;
    uint16 flag; /* sys_nh_ip_tunnel_flag_t */
    uint8  sa_index;
    uint8  rsv;
    uint32 dsl3edit_offset;
    uint32 loop_nhid;

    uint16 dest_vlan_ptr;
    uint16 span_id;

    sys_wb_nh_l2edit_t  l2_inner;
    sys_wb_nh_l2edit_t  l2_outer;
    sys_wb_nh_l3edit_t  l3_edit;
    sys_wb_nh_nat_t     nat;

    uint32 vn_id;
    uint16 dest_logic_port; /*H-overlay*/
    uint16 arp_id;
    uint32 inner_l2_edit_offset;
    uint32 outer_l2_edit_offset;

    uint32 loop_edit_ptr;

    uint16 dot1ae_channel;
    uint8  sc_index;
    uint8  sci_en            : 1;
    uint8  dot1ae_en         : 1;
    uint8  udf_profile_id    : 6;

};
typedef struct sys_wb_nh_info_ip_tunnel_s sys_wb_nh_info_ip_tunnel_t;

struct sys_wb_nh_info_trill_s
{
    uint16 ingress_nickname;
    uint16 egress_nickname;
    uint16 l3ifid;
    uint8  rsv[2];
    uint32 gport;
    uint32 dsl3edit_offset;
    uint32 l2_edit_ptr;
    sys_wb_nh_l2edit_t l2edit_key;
    uint16 dest_vlan_ptr;
    uint8  rsv1[2];
};
typedef struct sys_wb_nh_info_trill_s sys_wb_nh_info_trill_t;

struct sys_wb_nh_info_misc_s
{
    uint32 dsl2edit_offset;
    uint32 dsl3edit_offset;

    uint32 gport;
    uint8  is_swap_mac;
    uint8  is_reflective;
    uint8  misc_nh_type;
    uint8  l3edit_entry_type;
    uint8  l2edit_entry_type;
    uint8  vlan_profile_id;
    uint8  rsv[2];
    uint32 next_l3edit_offset;
    uint16 truncated_len;
    uint16 l3ifid;
};
typedef struct sys_wb_nh_info_misc_s sys_wb_nh_info_misc_t;



struct sys_wb_nh_info_com_hdr_s
{
    uint8 dsnh_entry_num;
    uint8 adjust_len;
    uint16 rsv;
    uint32 nh_entry_flags;      /*sys_nh_info_flag_type_t*/
    uint32 dsfwd_offset;
    uint32 stats_ptr;
    uint32 dsnh_offset;
    uint16 dest_chip;
    uint16 dest_id;
};
typedef struct sys_wb_nh_info_com_hdr_s sys_wb_nh_info_com_hdr_t;

struct sys_wb_nh_arp_s
{
    /*key*/
    uint16  arp_id;
	uint16  rsv;
    uint32  calc_key_len[0];

    /*data*/
    mac_addr_t mac_da;
    uint8  flag;
    uint8  l3if_type; /**< the type of l3interface , CTC_L3IF_TYPE_XXX */
    uint32 offset;          /*outer l2 edit for mpls/tunnel vlan & mac edit*/
    uint32 in_l2_offset;    /*inner l2 edit for ipuc vlan edit*/

    uint16 output_vid;
    uint8  output_vlan_is_svlan;
    uint8  rsv1;

    uint32 gport;
    uint32 nh_id;
    uint32 destmap_profile;

    uint16 ref_cnt;
    uint16 l3if_id;

};
typedef struct sys_wb_nh_arp_s sys_wb_nh_arp_t;

struct sys_wb_nh_vni_fid_s
{
    /*key*/
    uint32 vn_id;
    uint32  calc_key_len[0];

    /*data*/
    uint16 fid;
    uint16 ref_cnt;
};
typedef struct sys_wb_nh_vni_fid_s sys_wb_nh_vni_fid_t;

#define SYS_NH_APS_M 2
#define SYS_MPLS_NH_MAX_TUNNEL_LABEL_NUM   12
struct sys_wb_nh_mpls_sr_s
{
    uint32 dsnh_offset;
    uint32 pw_offset;
    uint32 lsp_offset;
    uint32 spme_offset;
    uint32 l2edit_offset;
};
typedef struct sys_wb_nh_mpls_sr_s sys_wb_nh_mpls_sr_t;
struct sys_wb_nh_mpls_tunnel_s
{
    /*key*/
    uint16  tunnel_id;
    uint32 calc_key_len[0];

    /*data*/
    uint32  gport; /*if SYS_NH_MPLS_TUNNEL_FLAG_APS is set, it is aps group id*/
    uint16  l3ifid;
    uint8   label_num;
    uint16  flag; /*sys_nh_dsmet_flag_t,SYS_NH_MPLS_TUNNEL_FLAG_XX*/
    uint16  ref_cnt;
    uint16  l2edit_offset[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  spme_offset[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  arp_id[SYS_NH_APS_M][SYS_NH_APS_M];
    uint16  lsp_offset[SYS_NH_APS_M];

    uint16  stats_ptr ;
    uint16  p_stats_ptr;

    sys_wb_nh_l2edit_t l2edit[SYS_NH_APS_M][SYS_NH_APS_M];
    sys_wb_nh_mpls_sr_t sr[SYS_NH_APS_M][SYS_MPLS_NH_MAX_TUNNEL_LABEL_NUM];
    uint8    sr_loop_num[SYS_NH_APS_M];
    uint8  rsv[3];
};
typedef struct sys_wb_nh_mpls_tunnel_s  sys_wb_nh_mpls_tunnel_t;


struct sys_wb_nh_info_mcast_s
{
   /*key*/
    uint32   basic_met_offset;
    uint32   dsmet_offset;
    uint16   vid;
    uint32   calc_key_len[0];

    /*data*/
    sys_wb_nh_info_com_hdr_t hdr;
   /*group info*/
    uint32   nh_id;
    uint16   physical_replication_num;
    uint8    mcast_flag;
    uint8    mirror_ref_cnt;

    /*member info*/

    uint16   next_dsmet_offset;
    uint16   flag; /*sys_nh_dsmet_flag_t,SYS_NH_DSMET_FLAG_XX*/
    uint16   logic_port;

    uint32   dsnh_offset;
    uint16   ref_nhid;
    uint16   ucastid;


    uint16   replicate_num; /*replicate_num for IPMC; port num of l2mc port bitmap*/
    uint32   pbm[4];

    uint8    free_dsnh_offset_cnt;
    uint8    member_type;
    uint8    port_type;
    uint8    entry_type;
    uint16   ref_nhid_hi;
};
typedef struct sys_wb_nh_info_mcast_s sys_wb_nh_info_mcast_t;

struct sys_wb_nh_info_com_s
{
    /*key*/
    uint32  nh_id;
    uint32  nh_type; /*sys_usw_nh_type_t*/
    uint32  calc_key_len[0];

    /*data*/
    sys_wb_nh_info_com_hdr_t hdr;
    union {
        sys_wb_nh_info_misc_t    misc;
        sys_wb_nh_info_trill_t   trill;
        sys_wb_nh_info_ip_tunnel_t ip_tunnel;
        sys_wb_nh_info_mpls_t    mpls;
        sys_wb_nh_info_ecmp_t    ecmp;
        sys_wb_nh_info_special_t spec;
        sys_wb_nh_info_rspan_t   rspan;
        sys_wb_nh_info_brguc_t   brguc;
        sys_wb_nh_info_ipuc_t    ipuc;
        sys_wb_nh_info_wlan_t    wlan;
    }info;
};
typedef struct sys_wb_nh_info_com_s sys_wb_nh_info_com_t;

struct sys_wb_nh_ipsa_v4_node_s
{
    uint32  count;
    ip_addr_t ipv4;
};
typedef struct sys_wb_nh_ipsa_v4_node_s sys_wb_nh_ipsa_v4_node_t;

struct sys_wb_nh_ipsa_v6_node_s
{
    uint32  count;
    ipv6_addr_t ipv6;
};
typedef struct sys_wb_nh_ipsa_v6_node_s sys_wb_nh_ipsa_v6_node_t;

#define SYS_NH_CW_NUM 16
#define SYS_SPEC_L3IF_NUM               4095
struct sys_wb_nh_master_s
{
    /*key*/
    uint32 lchip;   /*lchip*/
    uint32 calc_key_len[0];

    /**data */
    uint32    version;
    uint32     max_external_nhid;
    uint16     max_tunnel_id;
    uint32     ipmc_dsnh_offset[SYS_SPEC_L3IF_NUM];
    uint16     ecmp_if_resolved_l2edit;
    uint16     reflective_resolved_dsfwd_offset;
    uint16     cw_ref_cnt[SYS_NH_CW_NUM];
    uint16     max_ecmp;               /**<  the maximum ECMP paths allowed for a route */
    uint16     cur_ecmp_cnt;
    sys_wb_nh_ipsa_v4_node_t ipv4[16];        /*for restore ipsa*/
    sys_wb_nh_ipsa_v6_node_t ipv6[16];        /*for restore ipsa*/
    uint8      reflective_brg_en;
    uint8      ip_use_l2edit;
    uint8      ipmc_logic_replication;
    uint8      pkt_nh_edit_mode;
    uint8      no_dsfwd ;
    uint8      rspan_nh_in_use;          /**<if set,rsapn nexthop have already used */
    uint32     udf_profile_bitmap;       /* for restore udf used profile */
    uint16     udf_ether_type[8];        /* for restore udf ether type for each udf profile */
    uint32     efd_nh_id;                /* for restore efd redirect nexthop id*/
};
typedef struct sys_wb_nh_master_s sys_wb_nh_master_t;


struct sys_wb_nh_brguc_node_s
{
    /*key*/
    uint32 gport;   /*lchip*/
    uint32 calc_key_len[0];

    /*data*/
    uint32 nhid_brguc;
    uint32 nhid_bypass;
};
typedef struct sys_wb_nh_brguc_node_s sys_wb_nh_brguc_node_t;

#ifdef __cplusplus
}
#endif

#endif
