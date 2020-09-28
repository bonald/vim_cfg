#include "ctc_nexthop.h"
#include "drv_api.h"

struct sys_nexthop_s
{

    uint8 update_type;
    uint32 offset;


    uint8  is_nexthop8_w;
    uint8  share_type;
    uint8  payload_operation;
    uint8  next_hop_bypass;

    uint8  is_leaf;
    uint8  aps_bridge_en;
    uint8  mtu_check_en;
    uint8  mirror_tag_add;

    uint8  vlan_xlate_mode;
    uint8  svlan_tagged;
    uint8  svlan_tag_disable;
    uint8  copy_dscp;
    uint8  replace_dscp;
    uint8  stag_cos;
    uint8  svlan_tpid;
    uint8  tagged_mode;
    uint8  output_svlan_id_valid;
    uint8  replace_stag_cos;
    uint8  derive_stag_cos;
    uint8  stag_cfi;
    uint8  svlan_tpid_en;
    uint8  cvlan_strip;      /*1:strip cvlan*/
    uint8  cvlan_tag_disable;
    uint8  ctag_share_mode;
    uint8  output_cvlan_id_valid;
    uint8  replace_ctag_cos;
    uint8  copy_ctag_cos;
    uint8  tpid_swap;
    uint8  cos_swap;
    uint16 output_svlan_id;
    uint16 output_cvlan_id;

    uint32 stats_ptr;
    uint16 dest_vlan_ptr;

    uint8  fid_valid;
    uint8  stp_id_valid;
    uint8  logic_port_check;
    uint8  stp_id;
    uint16 fid;
    uint16 logic_dest_port;

    uint8  use_packet_ttl;
    uint8  ttl_no_dec;
    uint8  outer_edit_valid;
    uint8  outer_edit_location;
    uint8  outer_edit_ptr_type;
    uint8  inner_edit_ptr_type;
    uint8  is_vxlan_route_op;


    uint8 hash_num;
    uint8 xerpsan_en;
    uint8 span_en;
    uint8 keep_igs_ts;
    uint8 cw_index;

    uint32 inner_edit_ptr;
    uint32 outer_edit_ptr;

    mac_addr_t mac_da;
    uint8 is_wlan_encap;
    uint8 wlan_tunnel_type;
    uint8 rid;
    uint8 wlan_mc_en;
    uint8 is_dot11;
    uint8 stag_en;

   uint8 cid_valid;
   uint16 cid;

   uint8 ecid_valid;
   uint8 stag_action;
   uint16 ecid;


};
typedef struct sys_nexthop_s  sys_nexthop_t;

struct sys_fwd_s
{

    uint32 offset;

    uint8 force_back_en;
    uint8 bypass_igs_edit;
    uint8 nexthop_ext;
    uint8 aps_bridge_en;
    uint8 adjust_len_idx;
    uint8 cut_through_dis;
    uint8 is_6w;
    uint8 critical_packet;
    uint32 dest_map;
    uint32 nexthop_ptr;
    uint8 truncate_profile;
    uint8 cloud_sec_en;
};
typedef struct sys_fwd_s  sys_fwd_t;

struct sys_met_s
{
    uint8 is_6w;
    uint8 mcast_mode;
    uint32 next_hop_ptr;

    uint32 port_bitmap[4];
    uint8   port_bitmap_base;

    uint8 force_back_en;
    uint16 logic_dest_port;
    uint8 leaf_check_en;
    uint8 logic_port_check_en;
    uint8 logic_port_type_check;
    uint8 aps_bridge_en;
    uint16 dest_id;
    uint16 replicate_num;

    uint8 is_agg;
    uint8 phy_port_chk_discard;
    uint8 remote_chip;
    uint8 end_local_rep;
    uint16 next_met_entry_ptr;
    uint8 next_met_entry_type;
    uint8 next_hop_ext;
    uint8 is_destmap_profile;
    uint8 cloud_sec_en;
};
typedef struct sys_met_s  sys_met_t;

struct sys_dsl3edit_trill_s
{
    uint8 ds_type;
    uint8 l3_rewrite_type;
    uint8 ttl;
    uint8 mtu_check_en;

    uint16 mtu_size;
    uint8 rsv[2];

    uint16 ingress_nickname;
    uint16 egress_nickname;

    uint32 stats_ptr;
};
typedef struct sys_dsl3edit_trill_s  sys_dsl3edit_trill_t;

struct sys_dsecmp_group_s
{
    uint8 dlb_en;
    uint8 resilient_hash_en;
    uint8 rr_en;
    uint8 member_num;
    uint16 member_base;
    uint8 stats_valid;
    uint8 failover_en;
};
typedef struct sys_dsecmp_group_s  sys_dsecmp_group_t;

struct sys_dsecmp_member_s
{
    uint8 valid0;
    uint16 dsfwdptr0;
    uint8 dest_channel0;

    uint8 valid1;
    uint16 dsfwdptr1;
    uint8 dest_channel1;

    uint8 valid2;
    uint16 dsfwdptr2;
    uint8 dest_channel2;

    uint8 valid3;
    uint16 dsfwdptr3;
    uint8 dest_channel3;
};
typedef struct sys_dsecmp_member_s  sys_dsecmp_member_t;

struct sys_dsecmp_rr_s
{
    uint8 random_rr_en;
    uint8 member_num;
};
typedef struct sys_dsecmp_rr_s  sys_dsecmp_rr_t;



int32
sys_usw_nh_write_asic_table(uint8 lchip, uint8 table_type, uint32 offset, void* value);
int32
sys_usw_nh_update_asic_table(uint8 lchip, uint8 table_type, uint32 offset, void* value);

int32
sys_usw_nh_get_asic_table(uint8 lchip, uint8 table_type, uint32 offset, void* value);

int32
sys_usw_nh_set_asic_table(uint8 lchip, uint8 table_type, uint32 offset, void* value);

int32
sys_usw_nh_table_info_init(uint8 lchip);

int32
sys_usw_nh_global_cfg_init(uint8 lchip);



