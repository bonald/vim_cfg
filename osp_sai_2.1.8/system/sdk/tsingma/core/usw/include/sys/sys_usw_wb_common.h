/**
 @file sys_usw_wb_common.h

 @date 2016-04-20

 @version v1.0

 The file defines queue api
*/

#ifndef _SYS_USW_WB_COMMON_H_
#define _SYS_USW_WB_COMMON_H_
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
#include "sys_usw_stacking.h"
#include "sys_usw_acl_api.h"
#include "sys_usw_fpa.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_ipmc.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_ip_tunnel.h"
#include "sys_usw_overlay_tunnel.h"
#include "sys_usw_stats.h"
#include "sys_usw_datapath.h"

#include "drv_api.h"

#define SYS_WB_VERSION_L3IF                 CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_IPUC                 CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_IP_TUNNEL    		CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_IPMC                 CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_MPLS                 CTC_WB_VERSION(1,2)
#define SYS_WB_VERSION_STATS                CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_SCL                  CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_ACL                  CTC_WB_VERSION(1,4)
#define SYS_WB_VERSION_VLAN                 CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_ADV_VLAN           	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_OVERLAY          	CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_CHIP                 CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_LINKAGG          	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_PTP                  CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_SYNCETHER    		CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_QOS                  CTC_WB_VERSION(2,0)
#define SYS_WB_VERSION_SECURITY          	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_OAM                  CTC_WB_VERSION(1,2)
#define SYS_WB_VERSION_PORT                 CTC_WB_VERSION(1,3)
#define SYS_WB_VERSION_L2                   CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_STACKING         	CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_NEXTHOP          	CTC_WB_VERSION(2,0)
#define SYS_WB_VERSION_APS                  CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_MIRROR           	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_IPFIX                CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_WLAN             	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_DOT1AE           	CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_DATAPATH           	CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_REGISTER         CTC_WB_VERSION(1,1)
#define SYS_WB_VERSION_INTER_PORT         CTC_WB_VERSION(1,0)
#define SYS_WB_VERSION_PACKET         CTC_WB_VERSION(1,0)

/****************************************************************
*
*adv vlan module
*
****************************************************************/

enum sys_wb_appid_adv_vlan_subid_e
{
    SYS_WB_APPID_SUBID_ADV_VLAN_MASTER,
    SYS_WB_APPID_SUBID_ADV_VLAN_DEFAULT_ENTRY_ID,
    SYS_WB_APPID_SUBID_ADV_VLAN_RANGE_GROUP,
    SYS_WB_APPID_SUBID_ADV_VLAN_MAX
};
typedef enum sys_wb_appid_adv_vlan_subid_e sys_wb_appid_adv_vlan_subid_t;


struct sys_wb_adv_vlan_default_entry_id_s
{
    /*key*/
    uint8 tcam_id;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32 mac_eid;
    uint32 ipv4_eid;
    uint32 ipv6_eid;
};
typedef struct sys_wb_adv_vlan_default_entry_id_s sys_wb_adv_vlan_default_entry_id_t;

struct sys_wb_adv_vlan_range_group_s
{
    /*key*/
    uint8 group_id;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint16 group_flag;
    uint8 rsv1[2];

};
typedef struct sys_wb_adv_vlan_range_group_s sys_wb_adv_vlan_range_group_t;


struct sys_wb_adv_vlan_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
};
typedef struct sys_wb_adv_vlan_master_s sys_wb_adv_vlan_master_t;

/****************************************************************
*
* wlan module
*
****************************************************************/
enum sys_wb_appid_wlan_subid_e
{
    SYS_WB_APPID_WLAN_SUBID_MASTER,
    SYS_WB_APPID_WLAN_SUBID_TUNNEL_SW_ENTRY,
    SYS_WB_APPID_WLAN_SUBID_MAX
};
typedef enum sys_wb_appid_wlan_subid_e sys_wb_appid_wlan_subid_T;

struct sys_wb_wlan_tunnel_sw_entry_s
{
    /*key*/
    uint16 tunnel_id;
    uint8 rsv0[2];
    uint32  calc_key_len[0];

    /*data*/
    union
    {
        ip_addr_t ipv4;         /**< [D2] IPv4 source address */
        ipv6_addr_t ipv6;       /**< [D2] IPv6 source address */
    } ipsa;
    union
    {
        ip_addr_t ipv4;         /**< [D2] IPv4 dest address */
        ipv6_addr_t ipv6;       /**< [D2] IPv6 dest address */
    } ipda;
    uint16 l4_port;             /**< [D2] udp dest port */
    uint16 network_count;       /**< [D2] network tunnel count */
    uint16 bssid_count;         /**< [D2] bssid tunnel count */
    uint16 bssid_rid_count;     /**< [D2] bssid rid tunnel count */
    uint8 is_ipv6;              /**< [D2] ipv4 or ipv6 */
    uint8 rsv1[3];
};
typedef struct sys_wb_wlan_tunnel_sw_entry_s sys_wb_wlan_tunnel_sw_entry_t;

struct sys_wb_wlan_master_s
{
    /*key*/
    uint32 default_clinet_entry_id;
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 l2_trans_edit_offset;
    uint16 default_client_vlan_id;
    uint8  default_client_action;
    uint32 scl_hash0_tunnel_gid;
    uint32 scl_hash1_tunnel_gid;
    uint32 scl_tcam0_tunnel_gid;
    uint32 scl_tcam1_tunnel_gid;
    uint32 scl_hash0_client_local_gid;
    uint32 scl_hash0_client_remote_gid;
    uint32 scl_hash1_client_local_gid;
    uint32 scl_hash1_client_remote_gid;
    uint32 scl_tcam0_client_local_gid;
    uint32 scl_tcam0_client_remote_gid;
    uint32 scl_tcam1_client_local_gid;
    uint32 scl_tcam1_client_remote_gid;
    uint8 rsv1;
};
typedef struct sys_wb_wlan_master_s sys_wb_wlan_master_t;

/****************************************************************
*
* OAM module
*
****************************************************************/
#define SYS_OAM_MAX_MAID_LEN 48
enum sys_wb_appid_oam_subid_e
{
    SYS_WB_APPID_OAM_SUBID_MASTER,
    SYS_WB_APPID_OAM_SUBID_CHAN,
    SYS_WB_APPID_OAM_SUBID_MAID,   /*maid may exist alone, have no relationship with mep, sync in this table*/
    SYS_WB_APPID_OAM_SUBID_OAMID,
    SYS_WB_APPID_OAM_SUBID_MAX
};
typedef enum sys_wb_appid_oam_subid_e sys_wb_appid_oam_subid_t;

struct sys_wb_oam_master_s
{
    /*key*/
    uint8  maid_len_format;            /*need check before updating*/
    uint8  rsv0[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint8  mep_index_alloc_by_sdk;     /*need check before updating*/
    uint8  tp_section_oam_based_l3if;  /*need check before updating*/
    uint8  rsv[2];
    uint32 oam_reource_bfdv6addr;
    uint32 oam_reource_lm;
    uint32 oam_reource_mep;
    uint32 oam_reource_key;
    uint32 defect_to_rdi_bitmap0;
    uint32 defect_to_rdi_bitmap1;
};
typedef struct sys_wb_oam_master_s sys_wb_oam_master_t;

struct sys_wb_oam_chan_s
{
    /*key*/
    uint32 hash_key_index;
    uint32 rmep_hash_key_index;
    uint32 lmep_index;     /*Add to Vector need lmep/rmep index*/
    uint32 rmep_index;     /*P2MP need rmep index*/
    uint8  mep_type;           /*for wb restore*/
    uint8  spaceid;
    uint8  lmep_without_rmep;  /*lmep with rmep or without mep*/
    uint8  md_level;
    uint32 calc_key_len[0];

    /*data*/
    uint32 lmep_flag;
    uint32 rmep_flag;
    uint32 nhid;
    uint32 label;
    uint32 local_discr;        /*for SBFD reflector*/
    uint8  lm_num;
    uint8  link_oam;
    uint16 gport;
    uint8  is_vpws;
    uint8  rmep_md_level;
    uint8  lmep_lm_index_alloced;
    uint8  eth_lm_index_alloced;
    uint8  with_maid;
    uint8  maid_entry_num;
    uint8  ref_cnt;       /*maid*/
    uint8  tp_lm_index_alloced;    /*TP-BFD && TP-Y1731 LM*/
    uint8  lock_en;
    uint8  mip_bitmap;
    uint16 vlan_id;                /*Ethoam vlan id can not get from asic when fid based*/
    uint32 rmep_id;                /*P2P can not get rmep_id from asic*/
    uint32 tp_oam_key_index[2];
    uint8  tp_oam_key_valid[2];
    uint8  rsv[2];
};
typedef struct sys_wb_oam_chan_s sys_wb_oam_chan_t;

struct sys_wb_oam_maid_s
{
    /*key*/
    uint8    mep_type;
    uint8    maid_len;
    uint8    maid_entry_num;
    uint8    ref_cnt;
    uint8    maid[SYS_OAM_MAX_MAID_LEN];
    uint32 calc_key_len[0];

    /*data*/
    uint32   maid_index;
};
typedef struct sys_wb_oam_maid_s sys_wb_oam_maid_t;
struct sys_wb_oam_oamid_s
{
    /*key*/
    uint32 oam_id;
    uint32 gport;
    uint32 calc_key_len[0];

    /*data*/
    void*  p_sys_chan_eth;
    uint32 label[2];
    uint8  space_id[2];
    uint8  rsv[2];
};
typedef struct sys_wb_oam_oamid_s sys_wb_oam_oamid_t;

/****************************************************************
*
* vlan module
*
****************************************************************/
enum sys_wb_appid_vlan_subid_e
{
    SYS_WB_APPID_VLAN_SUBID_MASTER,
    SYS_WB_APPID_VLAN_SUBID_OVERLAY_MAPPING_KEY,
    SYS_WB_APPID_VLAN_SUBID_MAX
};
typedef enum sys_wb_appid_vlan_subid_e sys_wb_appid_vlan_subid_t;

struct sys_wb_vlan_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];
    /*data*/
    uint32 version;
    uint32 vlan_bitmap[128];
    uint32 vlan_def_bitmap[128];
};
typedef struct sys_wb_vlan_master_s sys_wb_vlan_master_t;

struct sys_wb_vlan_overlay_mapping_key_s
{
    /*key*/
    uint32 vn_id;
    uint32 calc_key_len[0];
    /*data*/
    uint16 fid;
};
typedef struct sys_wb_vlan_overlay_mapping_key_s sys_wb_vlan_overlay_mapping_key_t;

/****************************************************************
*
* l3if module
*
****************************************************************/
enum sys_wb_appid_l3if_subid_e
{
    SYS_WB_APPID_L3IF_SUBID_MASTER,
    SYS_WB_APPID_L3IF_SUBID_PROP,
    SYS_WB_APPID_L3IF_SUBID_ROUTER_MAC,
    SYS_WB_APPID_L3IF_SUBID_ECMP_IF,
    SYS_WB_APPID_L3IF_SUBID_MAX
};
typedef enum sys_wb_appid_l3if_subid_e sys_wb_appid_l3if_subid_t;

struct sys_wb_l3if_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /**data */
    uint32 version;
};
typedef struct sys_wb_l3if_master_s sys_wb_l3if_master_t;

struct sys_wb_l3if_prop_s
{
    /*key*/
    uint32 l3if_id;
    uint32 calc_key_len[0];

    /**data */
    uint16  vlan_id;
    uint16  gport;
    uint8   vaild;
    uint8   l3if_type; /**< the type of l3interface , CTC_L3IF_TYPE_XXX */
    uint8   rtmac_index;
    uint8   rtmac_bmp;
    uint16  vlan_ptr; /**< Vlanptr */
};
typedef struct sys_wb_l3if_prop_s sys_wb_l3if_prop_t;

struct sys_wb_l3if_router_mac_s
{
    /*key*/
    uint32 profile_id;
    uint8  is_inner;
    uint8  rsv[3];
    uint32 calc_key_len[0];

    /**data */
    mac_addr_t mac;
    uint8 ref;
    uint8 rsv0[3];
};
typedef struct sys_wb_l3if_router_mac_s sys_wb_l3if_router_mac_t;

struct sys_wb_l3if_ecmp_if_s
{
    /*key*/
    uint32 ecmp_if_id;
    uint8   index;
    uint8   rsv[3];
    uint32 calc_key_len[0];

    /**data */
    uint16 hw_group_id;
    uint8  intf_count;
    uint8  failover_en;
    uint16 intf_array[64];
    uint32 dsfwd_offset[64];
    uint16 ecmp_group_type;     /* refer to ctc_nh_ecmp_type_t */
    uint16 ecmp_member_base;
    uint32 stats_id;
};
typedef struct sys_wb_l3if_ecmp_if_s sys_wb_l3if_ecmp_if_t;

/****************************************************************
*
* dot1ae module
*
****************************************************************/
enum sys_wb_appid_dot1ae_subid_e
{
    SYS_WB_APPID_DOT1AE_SUBID_MASTER,
    SYS_WB_APPID_DOT1AE_SUBID_CHANNEL,
    SYS_WB_APPID_DOT1AE_SUBID_CHAN_BIND_NODE,
    SYS_WB_APPID_DOT1AE_SUBID_MAX
};
typedef enum sys_wb_appid_dot1ae_subid_e sys_wb_appid_dot1ae_subid_T;

struct sys_wb_dot1ae_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /**data */
    uint32 version;
};
typedef struct sys_wb_dot1ae_master_s sys_wb_dot1ae_master_t;

struct sys_wb_dot1ae_channel_s
{
    /*key*/
    uint8 chan_id;
    uint8 rsv1[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32  dir:2;
    uint32        sc_index:5; /*0~32*/
    uint32        bound:1;
    uint32        valid:1;
    uint32        include_sci:1;
    uint32        an_en:4;/*rx inuse*/
    uint32        next_an:4;/**/
    uint32        an_valid:4;/*valid an cfg*/
    uint32        binding_type:2;
    uint32        rsv:8;
    uint32        gport;
    uint8  key[16*4];
};
typedef struct sys_wb_dot1ae_channel_s sys_wb_dot1ae_channel_t;

struct sys_wb_dot1ae_channel_bind_node_s
{
    /*key*/
    uint8 sc_index;
    uint8 dir;
    uint8 rsv1[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32        value;
};
typedef struct sys_wb_dot1ae_channel_bind_node_s sys_wb_dot1ae_channel_bind_node_t;

/****************************************************************
*
* port module
*
****************************************************************/
enum sys_wb_appid_port_subid_e
{
    SYS_WB_APPID_PORT_SUBID_MASTER,
    SYS_WB_APPID_PORT_SUBID_PROP,
    SYS_WB_APPID_PORT_SUBID_MAC_PROP,
    SYS_WB_APPID_INTERPORT_SUBID_MASTER,
    SYS_WB_APPID_INTERPORT_SUBID_USED,
    SYS_WB_APPID_PORT_SUBID_MAX
};
typedef enum sys_wb_appid_port_subid_e sys_wb_appid_port_subid_t;

struct sys_wb_port_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32  version;
    uint32  mac_bmp[4]; /* used for TAP, record tap dest, 0~1 for slice0, 2~3 for slice1 */
    uint32  igs_isloation_bitmap;
    uint8   use_logic_port_check;
    uint8   use_isolation_id;
    uint8   isolation_group_mode;
    uint32  igs_isloation_bitmap1;
};
typedef struct sys_wb_port_master_s sys_wb_port_master_t;

struct sys_wb_port_igs_prop_s
{
    uint32 port_mac_en        :1;
    uint32 lbk_en             :1;
    uint32 speed_mode         :3;
    uint32 subif_en           :1;
    uint32 training_status    :3;
    uint32 cl73_status        :2;   /* 0-dis, 1-en, 2-finish */
    uint32 rsv     :2;
    uint32 link_intr_en       :1;
    uint32 link_status        :1;
    uint32 rsv0               :17;

    uint32 inter_lport        :16;
    uint32 global_src_port    :16;

    uint16 flag;
    uint16 flag_ext;

    uint8 isolation_id;

    uint32 nhid;
};
typedef struct sys_wb_port_igs_prop_s sys_wb_port_igs_prop_t;

struct sys_wb_port_egs_prop_s
{
    uint16 flag;
    uint16 flag_ext;

    uint8 isolation_id;
    uint8 isolation_ref_cnt;
};
typedef struct sys_wb_port_egs_prop_s sys_wb_port_egs_prop_t;

struct sys_wb_port_prop_s
{
    /*key*/
    uint8  lchip;
    uint8  rsv;
    uint16 lport;
    uint32 calc_key_len[0];

    /*data*/
    sys_wb_port_igs_prop_t igs_port_prop;
    sys_wb_port_egs_prop_t egs_port_prop;
    uint16      network_port;
};
typedef struct sys_wb_port_prop_s sys_wb_port_prop_t;

struct sys_wb_port_mac_prop_s
{
    /*key*/
    uint16 lport;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /*data*/
    uint32 port_mac_en        :1;
    uint32 speed_mode         :3;
    uint32 cl73_enable        :1;   /* 0-dis, 1-en*/
    uint32 old_cl73_status    :2;
    uint32 link_intr_en       :1;
    uint32 link_status        :1;
    uint32 unidir_en          :1;
    uint32 rx_rst_hold        :1;
    uint32 old_phy_ready      :1;
    uint32 rsv1               :20;
};
typedef struct sys_wb_port_mac_prop_s sys_wb_port_mac_prop_t;

struct sys_wb_interport_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv[3];
    uint32 calc_key_len[0];
    /*data*/
    uint32  version;
};
typedef struct sys_wb_interport_master_s sys_wb_interport_master_t;

struct sys_wb_interport_used_s
{
    /*key*/
    uint16 idx1;
    uint16 idx2;
    uint32 calc_key_len[0];
    /*data*/
    uint32 used;
};
typedef struct sys_wb_interport_used_s sys_wb_interport_used_t;

/****************************************************************
*
* mpls module
*
****************************************************************/
enum sys_wb_appid_mpls_subid_e
{
    SYS_WB_APPID_MPLS_SUBID_MASTER,
    SYS_WB_APPID_MPLS_SUBID_ILM_HASH,
    SYS_WB_APPID_MPLS_SUBID_MAX
};
typedef enum sys_wb_appid_mpls_subid_e sys_wb_appid_mpls_subid_t;

struct sys_wb_mpls_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 pop_ad_idx;
    uint32 decap_ad_idx;
    uint32 cur_ilm_num;
    uint8  mpls_tcam0;
    uint8  mpls_tcam1;
    uint8  decap_mode;
    uint8  decap_tcam_group;
    uint16  mpls_tcam_max;
    uint16  mpls_cur_tcam;
};
typedef struct sys_wb_mpls_master_s sys_wb_mpls_master_t;

struct sys_wb_mpls_ilm_hash_s
{
    /*key*/
    uint32 label;
    uint8 spaceid;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /*action*/
    uint8 is_interface;
    uint8 is_tcam;
    uint8 model;
    uint8 type;
    uint8 id_type;
    uint8 bind_dsfwd;
    uint8  u2_type;
    uint8  u4_type;
    uint16 u2_bitmap;
    uint16 u4_bitmap;
    uint32 nh_id;
    uint32 stats_id;
    uint32 ad_index;
    uint8 is_assignport;
    uint16 service_id;
    uint8 actionrsv;
    uint16 oam_id;
    uint8 is_vpws_oam;
    uint8 is_tp_oam;
    uint32 gport;
};
typedef struct sys_wb_mpls_ilm_hash_s sys_wb_mpls_ilm_hash_t;

/****************************************************************
*
* fdb module
*
****************************************************************/
enum sys_wb_appid_l2_subid_e
 {
    SYS_WB_APPID_L2_SUBID_MASTER,
    SYS_WB_APPID_L2_SUBID_FDB,
    SYS_WB_APPID_L2_SUBID_VPORT_2_NHID,
    SYS_WB_APPID_L2_SUBID_FID_NODE,
    SYS_WB_APPID_L2_SUBID_MC,
    SYS_WB_APPID_L2_SUBID_FID_MEMBER,
    SYS_WB_APPID_L2_SUBID_MAX
 };
 typedef enum sys_wb_appid_l2_subid_e sys_wb_appid_l2_subid_t;

struct sys_wb_l2_master_s
{
    /*key*/
    uint8       lchip;
    uint8       rsv[3];
    uint32      calc_key_len[0];
    /*data*/
    uint32      version;
    uint32      l2mc_count;
    uint32      def_count;           /**< default entry count */
    uint32      ad_index_drop;
    uint32      cfg_vport_num;      /**< num of logic port */
    uint8       cfg_hw_learn;       /**< support hw learning. but not force all do hw learning. */
    uint8       static_fdb_limit;
    uint16      cfg_max_fid;        /**< CTC_MAX_FID_ID, 16383=0x3fff*/
    uint16       rchip_ad_rsv[SYS_USW_MAX_GCHIP_CHIP_ID];
    uint8 vp_alloc_ad_en;
    uint8  rsvdown;

};
typedef struct sys_wb_l2_master_s   sys_wb_l2_master_t;

struct sys_wb_l2_fdb_s
{
    mac_addr_t     mac;
    uint16         fid;
    uint8          rsv[2];
    uint32         calc_key_len[0];

    uint32         ad_index;
    uint16         flag;         /**<ctc_l2_flag_t */
    uint16         bind_nh:1;
    uint16         rsv_ad:1;
    uint16         rsv1:14;
    uint32         nh_id;

};
typedef struct sys_wb_l2_fdb_s  sys_wb_l2_fdb_t;

struct sys_wb_l2_mc_s
{
    uint32         key_index;
    uint32         calc_key_len[0];

    uint32         ad_index;
};
typedef struct sys_wb_l2_mc_s  sys_wb_l2_mc_t;

struct sys_wb_l2_vport_2_nhid_s
{
    /*key*/
    uint16  vport;
    uint8   rsv[2];
    uint32  calc_key_len[0];
    /*data*/
    uint32  nhid;
    uint32 ad_idx;
};
typedef struct sys_wb_l2_vport_2_nhid_s sys_wb_l2_vport_2_nhid_t;

struct sys_wb_l2_fid_node_s
{
    uint16         fid;
    uint8          rsv[2];
    uint32         calc_key_len[0];
    uint16         flag;         /**<ctc_l2_dft_vlan_flag_t */
    uint16         mc_gid;
    uint8          share_grp_en;

    uint8          recover_hw_en;
    uint16         cid;
    ctc_port_bitmap_t port_bmp;
    ctc_port_bitmap_t linkagg_bmp;
};
typedef struct sys_wb_l2_fid_node_s sys_wb_l2_fid_node_t;

/****************************************************************
*
* stacking module
*
****************************************************************/
#define SYS_WB_STK_MAX_TRUNK_ID  63
#define SYS_WB_STK_TRUNK_BMP_NUM   ((SYS_WB_STK_MAX_TRUNK_ID + CTC_UINT32_BITS) / CTC_UINT32_BITS)


enum sys_wb_appid_stacking_subid_e
{
    SYS_WB_APPID_STACKING_SUBID_MASTER,
    SYS_WB_APPID_STACKING_SUBID_TRUNK,
    SYS_WB_APPID_STACKING_SUBID_MCAST_DB,
    SYS_WB_APPID_STACKING_SUBID_MAX
};
typedef enum sys_wb_appid_stacking_subid_e sys_wb_appid_stacking_subid_t;

struct sys_wb_stacking_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32  version;
    uint16  stacking_mcast_offset;
    uint8   mcast_mode;
    uint8   binding_trunk;
    uint8   trunk_mode;
    uint8   bind_mcast_en;
    ctc_port_bitmap_t ing_port_en_bmp;
    ctc_port_bitmap_t egr_port_en_bmp;
};
typedef struct sys_wb_stacking_master_s sys_wb_stacking_master_t;

struct sys_wb_stacking_mcast_db_s
{
    /*key*/
    uint8   type;
    uint8   rsv;
    uint16  id;
    uint32 calc_key_len[0];

    /*data*/
    uint32   head_met_offset;
    uint32   tail_met_offset;

    uint8 append_en;
    uint8 alloc_id;
    uint8 rsv1[2];
    uint32 last_tail_offset;

    uint32   trunk_bitmap[SYS_WB_STK_TRUNK_BMP_NUM];
    ctc_port_bitmap_t port_bitmap;
    uint32   cpu_port;
};
typedef struct sys_wb_stacking_mcast_db_s sys_wb_stacking_mcast_db_t;



struct sys_wb_stacking_hdr_encap_s
{
    uint32 hdr_flag;   /**< encap cloud header type, refer to ctc_stacking_hdr_flag_t*/

    mac_addr_t mac_da; /**< cloud header mac da if exist l2 header*/
    mac_addr_t mac_sa; /**< cloud header mac sa if exist l2 header*/

    uint8  vlan_valid; /**< if set, cloud header will encap vlan when using l2 header*/
    uint8  rsv0;
    uint16 vlan_id;   /**< cloud header vlan id if exist vlan */

    union
    {
        ip_addr_t ipv4;             /**< cloud heade ipv4 destination address*/
        ipv6_addr_t ipv6;           /**< cloud heade ipv6 destination address*/
    } ipda;
};
typedef struct sys_wb_stacking_hdr_encap_s sys_wb_stacking_hdr_encap_t;

struct sys_wb_stacking_trunk_s
{
    /*key*/
    uint16 trunk_id;
    uint16 rsv;
    uint32 calc_key_len[0];

    /*data*/
    uint8 max_mem_cnt;
    uint8 replace_en;
    sys_wb_stacking_hdr_encap_t encap_hdr;
    uint8 mode;
};
typedef struct sys_wb_stacking_trunk_s sys_wb_stacking_trunk_t;


/****************************************************************
*
* ACL module
*
****************************************************************/
#define CTC_FPA_KEY_SIZE_NUM 4

enum sys_wb_appid_acl_subid_s
{
    SYS_WB_APPID_ACL_SUBID_MASTER,
    SYS_WB_APPID_ACL_SUBID_GROUP,
    SYS_WB_APPID_ACL_SUBID_ENTRY,
    SYS_WB_APPID_ACL_SUBID_FIELD_RANGE,
    SYS_WB_APPID_ACL_SUBID_TCAM_CID,
    SYS_WB_APPID_ACL_SUBID_MAX
};
typedef enum sys_wb_appid_acl_subid_s sys_wb_appid_acl_subid_t;

struct sys_wb_acl_block_s
{
    uint16      entry_count;  /* entry count on each block. uint16 is enough.*/
    uint16      free_count;   /* entry count left on each block*/

    uint16      start_offset[CTC_FPA_KEY_SIZE_NUM];
    uint16      sub_entry_count[CTC_FPA_KEY_SIZE_NUM];
    uint16      sub_free_count[CTC_FPA_KEY_SIZE_NUM];
    uint16      sub_rsv_count[CTC_FPA_KEY_SIZE_NUM];    /*rsv count can not be taken when adjust resource*/

    uint8       trend_active[CTC_FPA_KEY_SIZE_NUM];
    uint32      increase_trend[CTC_FPA_KEY_SIZE_NUM];
    uint32      decrease_trend[CTC_FPA_KEY_SIZE_NUM];
    uint32      last_priority[CTC_FPA_KEY_SIZE_NUM];
    uint32      move_num;
};
typedef struct sys_wb_acl_block_s sys_wb_acl_block_t;

struct sys_wb_acl_udf_entry_s
{
    uint32    key_index     :4; /*0~15*/
    uint32          udf_hit_index :4; /* used to acl key udf_hit_index*/
    uint32          key_index_used:1;
    uint32          ip_op         :1;
    uint32          ip_frag       :1;
    uint32          mpls_num      :1;
    uint32          l4_type       :4;
    uint32         udf_offset_num:3;
    uint32          ip_protocol   :1;
    uint32          rsv           :12;
    uint32      udf_id;
};
typedef struct sys_wb_acl_udf_entry_s   sys_wb_acl_udf_entry_t;

struct sys_wb_hash_sel_field_union_s
{
    uint8   l4_port_field; 	     /*l4_port_field:  0:none; 1:gre/nvgre: 2:vxlan; 3:l4 port; 4:icmp; 5:igmp */
    uint8   l4_port_field_bmp;   /*for 1:gre/nvgre; bit0:CTC_FIELD_KEY_GRE_KEY;     bit1:CTC_FIELD_KEY_NVGRE_KEY;   (can't exist at the same time)*/
                                 /*for 2:vxlan;     CTC_FIELD_KEY_VN_ID     (only one field, no need l4_port_field_bmp)*/
                                 /*for 3:l4 port;   bit2:CTC_FIELD_KEY_L4_DST_PORT; bit3:CTC_FIELD_KEY_L4_SRC_PORT; (can exist at the same time)*/
                                 /*for 4:icmp;      bit4:CTC_FIELD_KEY_ICMP_CODE;   bit5:CTC_FIELD_KEY_ICMP_TYPE;   (can exist at the same time)*/
                                 /*for 5:igmp;      CTC_FIELD_KEY_IGMP_TYPE (only one field, no need l4_port_field_bmp)*/
    uint8   key_port_type;       /*0:none; 1:key port;   2:metadata*/
    uint8   l2_l3_key_ul3_type;  /*0:none; 1:ip; 2:mpls; 3:Unknown_etherType*/

        /*asic u1Type == 0 ipHeaderError/ipOptions/fragInfo/ttl/tcpFlags*/
        /*u1Type == 1 ipHeaderError/ipOptions/fragInfo/vrfId/isRouteMac*/
        /*u1Type == 2 ipPktLenRangeId/l4PortRangeBitmap/tcpFlags*/
        /*u1Type == 3 ipHeaderError/ipOptions/fragInfo/ttl/layer3HeaderProtocol */
    uint8   l3_key_u1_type;   /*sw :asic u1Type+1 , 0:none; 1:g1; 2:g2; 3:g3; 4:g4. sw not support cfg l3_key_u1_type;default to 1*/
    uint8   ipv6_key_ip_mode;   /*0:none;1;full ip;2:compress ip,sw not support cfg ipv6_key_ip_mode.default to 1*/
};
typedef struct sys_wb_hash_sel_field_union_s   sys_wb_hash_sel_field_union_t;

struct sys_wb_acl_master_s
{
    /*key*/
    uint8 lchip;
    uint8 rsv1[3];
    uint32 calc_key_len[0];

    /*data*/
    uint32  version;
    uint32 key_count[ACL_ALLOC_COUNT_NUM];
    sys_wb_acl_block_t block[ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM];
    sys_wb_acl_udf_entry_t   udf_entry[SYS_ACL_MAX_UDF_ENTRY_NUM];
    uint16              hash_sel_profile_count[SYS_ACL_HASH_KEY_TYPE_NUM][SYS_ACL_HASH_SEL_PROFILE_MAX];
    uint8   sort_mode;
    uint8 rsv2;
    sys_wb_hash_sel_field_union_t              hash_sel_key_union_filed[SYS_ACL_HASH_KEY_TYPE_NUM][SYS_ACL_HASH_SEL_PROFILE_MAX];
};
typedef struct sys_wb_acl_master_s sys_wb_acl_master_t;

struct sys_wb_acl_field_range_s
{
    /*key*/
    uint8 range_id;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /*data*/
    uint16 min;
    uint16 max;
    uint16 ref_cnt;
    uint8  range_type;
    uint8  rsv1;
};
typedef struct sys_wb_acl_field_range_s sys_wb_acl_field_range_t;

struct sys_wb_acl_group_s
{
    /*key*/
    uint32  group_id;
    uint32  calc_key_len[0];

    /*data*/
    uint8       type;
    uint8       block_id;      /* block the group belong. For hash, 0xFF. */
    uint8       dir;           /* 0: ingress  1:egress*/
    uint8       bitmap_status;
    union
    {
        ctc_port_bitmap_t port_bitmap;
        uint16            port_class_id;
        uint16            vlan_class_id;
        uint16            l3if_class_id;
        uint16            service_id;
        uint8             pbr_class_id;
        uint16            gport;
    } un;
};
typedef struct sys_wb_acl_group_s sys_wb_acl_group_t;

struct sys_wb_acl_range_info_s
{
    uint8  flag;
    uint8  range_id[ACL_RANGE_TYPE_NUM];

    uint16 range_bitmap;
    uint16 range_bitmap_mask;
};
typedef struct sys_wb_acl_range_info_s sys_wb_acl_range_info_t;

struct sys_wb_acl_entry_s
{
    /*key*/
    uint32 entry_id;
    uint32 calc_key_len[0];

    /*data*/
    uint32 priority;  /* entry priority. . */
    uint32 group_id;
    uint8  key_type;
    uint8 rsv0;
    uint8  flag;      /* entry flag, FPA_ENTRY_FLAG_xxx */
    uint8  step;      /*????*/
    uint32 offset_a;  /* key index. max: 2 local_chips*/

    sys_wb_acl_range_info_t rg_info;
    uint32 ad_index;
    uint32 nexthop_id;
    uint32 copp_rate;    /*used for copy entry*/
    uint8  copp_ptr;     /*used for remvoe entry*/
    uint8  action_flag;
    uint8  mode;
    uint8 rsv1;

    uint32 hash_valid           : 1;          /*generated hash index*/
    uint32       key_exist            : 1;    /*key exist*/
    uint32       key_conflict         : 1;    /*hash key conflict*/
    uint32       u1_type              : 3;
    uint32       u2_type              : 3;
    uint32       u3_type              : 3;
    uint32       u4_type              : 3;
    uint32       u5_type              : 3;
    uint32       wlan_bmp             : 3;    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
    uint32       macl3basic_key_cvlan_mode:2; /*0,none;1:ip_da;  2:Aware Tunnel Info:CTC_FIELD_KEY_AWARE_TUNNEL_INFO*/
    uint32       macl3basic_key_macda_mode:2; /*0,none;1:mac_da; 2:Aware Tunnel Info:CTC_FIELD_KEY_AWARE_TUNNEL_INFO*/
	uint32       igmp_snooping        : 1;
    uint32       hash_sel_tcp_flags   : 2;
	uint32      rsv                   : 4;

    uint32 l2_type              :4;
    uint32       l3_type              :4;
    uint32      l4_type              :4;
    uint32      l4_user_type         :4;
    uint32       key_port_mode        :2;     /*0,none;1,port; 2;metata_or_cid*/
    uint32       fwd_key_nsh_mode     :2;     /*0,1:nsh;2:udf*/
    uint32      key_l4_port_mode     :2;     /*0,1:l4port;2:vxlan;3:gre*/
    uint32       mac_key_vlan_mode    :2;    /*0,1:svlan;2:cvlan*/
    uint32       ip_dscp_mode         :2;     /*0,1:dscp;2:ecn/PRECEDENCE*/
    uint32      key_mergedata_mode   :2;     /*0,1:Wlan Info;2:MergeData*/
    uint32       copp_ext_key_ip_mode :2;     /*0,1:ip;2:udf*/
    uint32      rsv2:2;

    uint16 u1_bitmap;
    uint16 u3_bitmap;

    uint8  u2_bitmap;
    uint8  u4_bitmap;
    uint8  u5_bitmap;
    uint8  action_type;                       /*for show action; refer to: sys_acl_action_type */

    uint32 action_bmp[(CTC_ACL_FIELD_ACTION_NUM-1)/32+1];  /*for show action; such as: action_bmp bit0 reprensents CTC_ACL_FIELD_ACTION_CP_TO_CPU */
    uint32 stats_id;                          /*used for show action field: CTC_ACL_FIELD_ACTION_STATS*/
    uint32 policer_id;                        /*used for show action field: per entry only have one of the three;
                                                CTC_ACL_FIELD_ACTION_COPP; CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER; CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER */

    uint16 ether_type;                        /*for tcam, add to cethertype_spool*/
    uint8  ether_type_index;                  /*warmboot restore entry, restore cethertype_spool*/

    uint32 key_bmp[(CTC_FIELD_KEY_NUM-1)/32+1];            /*for show key;    such as: key_bmp    bit0 reprensents CTC_FIELD_KEY_L2_TYPE */
    uint16 minvid;                                         /*for show key;    CTC_FIELD_KEY_SVLAN_RANGE CTC_FIELD_KEY_CVLAN_RANGE */
    uint16 key_reason_id;                                  /*for show key;    CTC_FIELD_KEY_CPU_REASON_ID */
    uint32 key_dst_nhid;                                   /*for show key;    CTC_FIELD_KEY_DST_NHID */
    uint16 udf_id;
    uint8  hash_sel_id;
    uint8  cos_index;
    uint32 limit_rate;
    uint16 u1_bitmap_high;
    uint8  u2_bitmap_high;
    uint8  u5_bitmap_high;
    uint16 cpu_reason_id;
    uint16 udf_id_high;
    uint8  real_step;
};
typedef struct sys_wb_acl_entry_s sys_wb_acl_entry_t;

/****************************************************************
*
* ipuc module
*
****************************************************************/
#define MAX_CTC_IP_VER 2
#define MAX_STATS_TYPE 10
#define MAX_CALPM_TABLE 2

enum sys_wb_appid_ipuc_subid_e
{
    SYS_WB_APPID_IPUC_SUBID_MASTER,
    SYS_WB_APPID_IPUC_SUBID_ALPM_MASTER,
    SYS_WB_APPID_IPUC_SUBID_OFB,
    SYS_WB_APPID_IPUC_SUBID_INFO,
    SYS_WB_APPID_IPUC_SUBID_INFO1,
    SYS_WB_APPID_IPUC_SUBID_MAX
};
typedef enum sys_wb_appid_ipuc_subid_e sys_wb_appid_ipuc_subid_t;

struct sys_wb_nalpm_prefix_info_s
{
    /*key*/
    uint32 lchip;
    uint16 tcam_idx;
    uint16 rsv;
    uint32 calc_key_len[0];
    /*data*/
    uint16  vrf_id;
    uint16 sram_idx;
    uint32  ad_route_addr[4];
    uint32  pfx_addr[4];
    uint8   pfx_len;
    uint8   ad_route_masklen;
    uint8   ip_ver;
    uint8   rsv1;
};
typedef struct sys_wb_nalpm_prefix_info_s sys_wb_nalpm_prefix_info_t;

struct sys_wb_calpm_master_s
{
    /*key*/
    uint32 lchip;   /*lchip*/
    uint32 calc_key_len[0];

    /*data*/
    uint8 couple_mode;
    uint8 ipsa_enable;
    uint8 prefix_length;   /* defaut use lpm_prefix8 */
    uint8 rsv;
};
typedef struct sys_wb_calpm_master_s sys_wb_calpm_master_t;

struct sys_wb_calpm_info_s
{
    uint16 stage_base[MAX_CALPM_TABLE];
    uint8  idx_mask[MAX_CALPM_TABLE];
    uint16 key_index;
    uint8   masklen_pushed;             /* pushed masklen */
    uint8   is_mask_valid;              /* 0 means have push down, but no match key and do nothing */
    uint8   is_pushed;                  /* if new, need push */
    uint8   rsv;
};
typedef struct sys_wb_calpm_info_s sys_wb_calpm_info_t;

struct sys_wb_nalpm_info_s
{
    uint16 tcam_idx;
    uint8 snake_idx;
    uint8 entry_offset;
};
typedef struct sys_wb_nalpm_info_s sys_wb_nalpm_info_t;

struct sys_wb_ipuc_master_s
{
    /*key*/
    uint32 lchip;   /*lchip*/
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 route_stats[MAX_CTC_IP_VER][MAX_STATS_TYPE];
    uint32 pending_list_len;

    uint8 host_lpm_mode;    /* host route mode, if set 1 means host route can use as lpm */
    uint8 tcam_mode;
    uint8 rsv[2];
};
typedef struct sys_wb_ipuc_master_s sys_wb_ipuc_master_t;

struct sys_wb_ipuc_info_s
{
    /*key*/
    uint16 vrf_id;
    uint16 l4_dst_port;
    uint8  masklen;
    uint8  rsv[3];
    uint32 ip[4];
    uint32 route_flag;
    uint32 calc_key_len[0];

    /*data*/
    uint32 key_index;
    uint32 ad_index;
    uint32 gport;           /* assign output port */
    uint32 nh_id;
    uint16 rpf_id;        /* used to store rpf_id */
    uint16 cid;
    uint16 stats_id;          /*stats id */
    uint8  rpf_mode;        /* refer to sys_rpf_chk_mode_t */
    uint8  route_opt;

    union
    {
        sys_wb_calpm_info_t calpm_info;
        sys_wb_nalpm_info_t nalpm_info;
        uint32 rsv_lpm;
    } lpm_info;
};
typedef struct sys_wb_ipuc_info_s sys_wb_ipuc_info_t;

/****************************************************************
*
* ipuc tunnel module
*
****************************************************************/
enum sys_wb_appid_ip_tunnel_subid_e
{
    SYS_WB_APPID_IP_TUNNEL_SUBID_MASTER,
    SYS_WB_APPID_IP_TUNNEL_SUBID_NATSA_INFO,
    SYS_WB_APPID_IP_TUNNEL_SUBID_MAX
};
typedef enum sys_wb_appid_ip_tunnel_subid_e sys_wb_appid_ip_tunnel_subid_t;

struct sys_wb_ip_tunnel_master_s
{
    /*key*/
    uint32 lchip;   /*lchip*/
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 snat_hash_count; /*snat store in host1 SaPort hash*/
    uint32 snat_tcam_count; /*snat store in tcam*/
    uint32 napt_hash_count; /*napt store in host1 Daport hash*/
};
typedef struct sys_wb_ip_tunnel_master_s sys_wb_ip_tunnel_master_t;

struct sys_wb_ip_tunnel_natsa_info_s
{
    /*key*/
    ip_addr_t ipv4;
    uint16 l4_src_port;
    uint16 vrf_id;
    uint8  is_tcp_port;
    uint32 calc_key_len[0];

    /*data*/
    uint16 ad_offset;
    uint16 key_offset;
    ip_addr_t new_ipv4;
    uint16 new_l4_src_port;
    uint8  in_tcam;
};
typedef struct sys_wb_ip_tunnel_natsa_info_s sys_wb_ip_tunnel_natsa_info_t;

/****************************************************************
*
* ipmc module
*
****************************************************************/
#define SYS_IPMC_TYPE_MAX 4

enum sys_wb_appid_ipmc_subid_e
{
    SYS_WB_APPID_IPMC_SUBID_MASTER,
    SYS_WB_APPID_IPMC_SUBID_GROUP_NODE,
    SYS_WB_APPID_IPMC_SUBID_G_NODE,
    SYS_WB_APPID_IPMC_SUBID_MAX
};
typedef enum sys_wb_appid_ipuc_subid_e sys_wb_appid_ipmc_subid_t;

struct sys_wb_ipmc_group_node_s
{
    /*key*/
    uint8  ip_version:1;
    uint8  is_l2mc:1;
    uint8  rsv:6;
    uint8  vrfid;              /**< [HB.GB.GG] VRF Id ,if IP-L2MC,used as FID */
    uint8  src_ip_mask_len;
    uint8  group_ip_mask_len;
    uint32 group_addr[4];         /**< [HB.GB.GG] IPv4 Group address */
    uint32 src_addr[4];           /**< [HB.GB.GG] IPv4 Source address */
    uint32 calc_key_len[0];

    /*data*/
    uint16 ad_index;
    uint16 group_id;
    uint8  share_grp:1;
    uint8  is_drop:1;
    uint8  with_nexthop:1;
    uint8  is_rd_cpu;
    uint8  rsv1:4;
};
typedef struct sys_wb_ipmc_group_node_s sys_wb_ipmc_group_node_t;

struct sys_wb_ipmc_g_node_s
{
    /*key*/
    uint16 ipmc_type;
    uint16 vrfid;
    uint32 ip[4];
    uint32 calc_key_len[0];

    /*data*/
    uint32  ref_cnt;
    uint16  pointer;
    uint8   is_group;
    uint8   rsv;
};
typedef struct sys_wb_ipmc_g_node_s sys_wb_ipmc_g_node_t;

struct sys_wb_ipmc_master_s
{
    /*key*/
    uint32 lchip;   /*lchip*/
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 ipmc_group_cnt[SYS_IPMC_TYPE_MAX];
    uint32 ipmc_group_sg_cnt[SYS_IPMC_TYPE_MAX];
};
typedef struct sys_wb_ipmc_master_s sys_wb_ipmc_master_t;

/****************************************************************
*
* scl module
*
****************************************************************/
#define  SYS_SCL_MAX_KEY_SIZE_IN_WORD 24

enum sys_wb_appid_scl_subid_e
{
    SYS_WB_APPID_SCL_SUBID_MASTER,
    SYS_WB_APPID_SCL_SUBID_GROUP,
    SYS_WB_APPID_SCL_SUBID_ENTRY,
    SYS_WB_APPID_SCL_SUBID_HASH_KEY_ENTRY,
    SYS_WB_APPID_SCL_SUBID_TCAM_KEY_ENTRY,
    SYS_WB_APPID_SCL_SUBID_MAX
};
typedef enum sys_wb_appid_scl_subid_e sys_wb_appid_scl_subid_t;

struct sys_wb_scl_master_s
{
    /*key*/
    uint32   lchip;
    uint8    block_id;
    uint8    key_size_type;
    uint32   calc_key_len[0];

    /*data*/
    uint32    version;
    uint16    start_offset;
    uint16    sub_entry_count;
    uint16    sub_free_count;
};
typedef struct sys_wb_scl_master_s sys_wb_scl_master_t;

struct sys_wb_scl_group_s
{
    /*key*/
    uint32  group_id;
    uint32  calc_key_len[0];

    /*data*/
    uint8   type;
    uint8   priority;        /* group priority */
    uint32  gport;           /* port_class_id/gport/logic_port */
};
typedef struct sys_wb_scl_group_s sys_wb_scl_group_t;

struct sys_wb_scl_hash_key_entry_s
{
    /*key*/
    uint32  key_index;
    uint8   action_type;
    uint8   scl_id;
    uint32  calc_key_len[0];

    /*data*/
    uint32  entry_id;
};
typedef struct sys_wb_scl_hash_key_entry_s sys_wb_scl_hash_key_entry_t;

struct sys_wb_scl_tcam_key_entry_s
{
    /* key */
    uint8  lchip;
    uint8  scl_id;
    uint32 key[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    uint32 mask[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    uint32 calc_key_len[0];
    /*data*/
    uint32  entry_id;
};
typedef struct sys_wb_scl_tcam_key_entry_s sys_wb_scl_tcam_key_entry_t;

struct sys_wb_scl_entry_s
{
    /*key*/
    uint32 entry_id;
    uint32 calc_key_len[0];
    /*data*/
    uint8 lchip;
    uint8 direction;
    uint8 key_type;                              /* sys_scl_key_type_t/sys_scl_tunnel_type_t: SCL Hash/Tunnel Hash/TCAM */
    uint8 action_type;                           /* sys_scl_action_type_t:DsUserid/Egress/DsTunnel/DsSclFlow */

    uint32 resolve_conflict     :1;
    uint32       is_half              :1;
    uint32       rpf_check_en         :1;
    uint32       uninstall            :1;             /* indicate the entry is installed or not */
    uint32       hash_valid           :1;             /* generated hash index */
    uint32       key_exist            :1;             /* key already exit */
    uint32       key_conflict         :1;             /* hash conflict, no memory */
    uint32      u1_type              :3;
    uint32       u2_type              :3;
    uint32       u3_type              :3;
    uint32       u4_type              :3;
    uint32      u5_type              :3;
    uint32       is_service_policer   :1;
    uint32       bind_nh              :1;
    uint32       acl_profile_valid    :1;
    uint32       vpws_oam_en          :1;
    uint32       rsv0                 :6;

    uint32 l2_type              :4;
    uint32       l3_type              :4;
    uint32       l4_type              :4;
    uint32       l4_user_type         :4;
    uint32       mac_key_vlan_mode    :2;             /* 0:none, 1:svlan, 2:cvlan */
    uint32      mac_key_macda_mode   :2;             /* 0:none, 1:macda, 2:customer id */
    uint32       key_l4_port_mode     :2;             /* 0:none, 1:l4port, 2:vxlan, 3:gre */
    uint32      rsv1                 :10;
    uint16 u1_bitmap;
    uint16 u3_bitmap;
    uint8  u2_bitmap;
    uint8  u4_bitmap;
    uint8  u5_bitmap;
    uint8  hash_field_sel_id;                    /* Hash select ID */
    uint32 hash_sel_bmp[2];                      /* Hash select bitmap */

    uint32 priority;
    uint32 nexthop_id;
    uint32 stats_id;
    uint32 key_index;
    uint16 ad_index;
    uint16 policer_id;
    uint32 vn_id;
    uint32 action_bmp[(SYS_SCL_FIELD_ACTION_TYPE_MAX-1)/32+1];
    uint32 group_id;                             /* get the entry's correspongding group address from group hash */
    uint16 vlan_edit_profile_id;
    uint8  acl_profile_id;
    uint8  is_scl;                               /* used to indicate whether is DsSclAclControlProfile_t or DsTunnelAclControlProfile_t */
    uint16 ether_type;
    uint8  ether_type_index;
    uint8  lsv;
    uint16 service_id;
    uint8  cos_idx;
    uint8  rsv2;
    uint32 key_bmp;
    uint16  u1_bitmap_high;
    uint8  u2_bitmap_high;
    uint8  u5_bitmap_high;
};
typedef struct sys_wb_scl_entry_s sys_wb_scl_entry_t;

/****************************************************************
*
* ipfix module
*
****************************************************************/
enum sys_wb_appid_ipfix_subid_e
{
    SYS_WB_APPID_IPFIX_SUBID_MASTER,
    SYS_WB_APPID_IPFIX_SUBID_MAX
};
typedef enum sys_wb_appid_ipfix_subid_e sys_wb_appid_ipfix_subid_t;

struct sys_wb_ipfix_master_s
{
    /*key*/
    uint8   lchip;
    uint8   rsv[3];
    uint32  calc_key_len[0];

    /*data*/
    uint32  version;
    uint32  max_ptr;
    uint32  aging_interval;
    uint32  exp_cnt_stats[8];
    uint64  exp_pkt_cnt_stats[8];
    uint64  exp_pkt_byte_stats[8];
    uint32  sip_interval[8];
};
typedef struct sys_wb_ipfix_master_s sys_wb_ipfix_master_t;

/****************************************************************
*
* linkagg module
*
****************************************************************/
enum sys_wb_appid_linkagg_subid_e
{
    SYS_WB_APPID_LINKAGG_SUBID_MASTER,
    SYS_WB_APPID_LINKAGG_SUBID_GROUP,
    SYS_WB_APPID_LINKAGG_SUBID_CHANGROUP,
    SYS_WB_APPID_LINKAGG_SUBID_MAX
};
typedef enum sys_wb_appid_linkagg_subid_e sys_wb_appid_linkagg_subid_t;

struct sys_wb_linkagg_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint8 linkagg_mode;
    uint8 rsv[3];
};
typedef struct sys_wb_linkagg_master_s sys_wb_linkagg_master_t;

struct sys_wb_linkagg_group_s
{
    /*key*/
    uint32 tid;              /* linkAggId */
    uint32 calc_key_len[0];

    /*data*/
    uint16   flag;
    uint16   max_member_num;
    uint16   real_member_num;
    uint8    mode;             /* ctc_linkagg_group_mode_t */
    uint8    ref_cnt;          /*for channel linkagg, bpe cb cascade*/
};
typedef struct sys_wb_linkagg_group_s sys_wb_linkagg_group_t;

/****************************************************************
*
* overlay tunnel module
*
****************************************************************/
#define SYS_OVERLAT_TUNNEL_MAX_IP_INDEX       4

enum sys_wb_appid_overlay_tunnel_subid_e
{
    SYS_WB_APPID_OVERLAY_SUBID_TUNNEL_MASTER,
};
typedef enum sys_wb_appid_overlay_tunnel_subid_e sys_wb_appid_overlay_tunnel_subid_t;

struct sys_wb_overlay_tunnel_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 ipda_index_cnt[SYS_OVERLAT_TUNNEL_MAX_IP_INDEX];
    uint8 could_sec_en;
    uint8 rsv[3];
};
typedef struct sys_wb_overlay_tunnel_master_s sys_wb_overlay_tunnel_master_t;

enum sys_wb_appid_aps_subid_e
{
    SYS_WB_APPID_APS_SUBID_MASTER,
    SYS_WB_APPID_APS_SUBID_NODE,
    SYS_WB_APPID_APS_SUBID_MAX
};
typedef enum sys_wb_appid_aps_subid_e sys_wb_appid_aps_subid_t;

struct sys_wb_aps_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /**data */
    uint32 version;
};
typedef struct sys_wb_aps_master_s sys_wb_aps_master_t;

struct sys_wb_aps_node_s
{
    /*key*/
    uint16  group_id;
    uint16  rsv;
    uint32  calc_key_len[0];

    /*data*/
    uint16 w_l3if_id;                   /**< working l3if id */
    uint16 p_l3if_id;                   /**< protection l3if id */
    uint16 raps_group_id;
    uint16 flag;                        /**< it's the sys_aps_flag_t value */
    uint32 nh_id;
};
typedef struct sys_wb_aps_node_s sys_wb_aps_node_t;

/****************************************************************
*
* stats module
*
****************************************************************/
#define SYS_STATS_RAM_MAX 15
#define SYS_STATS_TYPE_MAX 18
enum sys_wb_appid_stats_subid_e
{
    SYS_WB_APPID_STATS_SUBID_MASTER,
    SYS_WB_APPID_STATS_SUBID_STATSID,
    SYS_WB_APPID_STATS_SUBID_STATSPTR,
    SYS_WB_APPID_STATS_SUBID_MAC_STATS,
    SYS_WB_APPID_STATS_SUBID_MAX
};
typedef enum sys_wb_appid_stats_subid_e sys_wb_appid_stats_subid_t;

struct sys_wb_stats_prop_s
{
    uint16 used_cnt_ram[SYS_STATS_RAM_MAX];
    uint16 used_cnt;
    uint16 ram_bmp;
    uint8  rsv[2];
};
typedef struct sys_wb_stats_prop_s sys_wb_stats_prop_t;

struct sys_wb_stats_ram_s
{
    uint32 stats_bmp;
    uint16 used_cnt;
    uint8 acl_priority;
    uint8 rsv;
};
typedef struct sys_wb_stats_ram_s sys_wb_stats_ram_t;

struct sys_wb_stats_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /*data*/
    uint32                 version;
    sys_wb_stats_prop_t    stats_type[CTC_BOTH_DIRECTION][SYS_STATS_TYPE_MAX];
    sys_wb_stats_ram_t     stats_ram[SYS_STATS_RAM_MAX];
};
typedef struct sys_wb_stats_master_s sys_wb_stats_master_t;

struct sys_wb_mac_stats_rx_s
{
    uint64 mac_stats_rx_bytes[SYS_STATS_MAC_RCV_NUM];
    uint64 mac_stats_rx_pkts[SYS_STATS_MAC_RCV_NUM];
};
typedef  struct sys_wb_mac_stats_rx_s sys_wb_mac_stats_rx_t;

struct sys_wb_mac_stats_tx_s
{
    uint64 mac_stats_tx_bytes[SYS_STATS_MAC_SEND_NUM];
    uint64 mac_stats_tx_pkts[SYS_STATS_MAC_SEND_NUM];
};
typedef struct sys_wb_mac_stats_tx_s sys_wb_mac_stats_tx_t;

struct sys_wb_qmac_stats_s
{
    sys_wb_mac_stats_rx_t mac_stats_rx[SYS_STATS_XQMAC_PORT_MAX];
    sys_wb_mac_stats_tx_t mac_stats_tx[SYS_STATS_XQMAC_PORT_MAX];
};
typedef struct sys_wb_qmac_stats_s sys_wb_qmac_stats_t;

struct sys_wb_mac_stats_s
{
    /*key*/
    uint32 index;
    uint32 calc_key_len[0];

    /*data*/
    sys_wb_qmac_stats_t  qmac_stats_table;
};
typedef struct sys_wb_mac_stats_s sys_wb_mac_stats_t;

struct sys_wb_stats_statsid_s
{
    /*key*/
    uint32 stats_id;
    uint32 calc_key_len[0];

    /*data*/
    uint32 stats_ptr;
    uint32 hw_stats_ptr;
    uint8  stats_id_type:6;
    uint8  dir:1;
    uint8  color_aware:1;
    uint8  acl_priority:7;
    uint8  is_vc_label:1;
};
typedef struct sys_wb_stats_statsid_s sys_wb_stats_statsid_t;

struct sys_wb_stats_statsptr_s
{
    /*key*/
    uint32 stats_ptr;
    uint32 calc_key_len[0];

    /*data*/
};
typedef struct sys_wb_stats_statsptr_s sys_wb_stats_statsptr_t;

/****************************************************************
*
* mirror module
*
****************************************************************/
enum sys_wb_appid_mirror_subid_e
{
    SYS_WB_APPID_MIRROR_SUBID_MASTER,
    SYS_WB_APPID_MIRROR_SUBID_DEST,
    SYS_WB_APPID_MIRROR_SUBID_MAX
};
typedef enum sys_wb_appid_mirror_subid_e sys_wb_appid_mirror_subid_t;

struct sys_wb_mirror_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint32 version;
};
typedef struct sys_wb_mirror_master_s sys_wb_mirror_master_t;

struct sys_wb_mirror_dest_s
{
   /*key*/
   uint8 type;
   uint8 dir;
   uint8 session_id;
   uint32 calc_key_len[0];

   /*data*/
   uint32 nh_id;
   uint16 iloop_port;
};
typedef struct sys_wb_mirror_dest_s sys_wb_mirror_dest_t;

/****************************************************************
*
* ofb module
*
****************************************************************/
struct sys_wb_ofb_s
{
   /*key*/
   uint8 type;
   uint8 ofb_type;
   uint16 block_id;
   uint32 calc_key_len[0];

   /*data*/
   uint32      all_left_b; /* left unused boundary */
   uint32      all_right_b;    /* right unused boundary */
   uint32      all_of_num;   /*the number of all offset */
   uint8 adj_dir; /*for empty_grow*/
};
typedef struct sys_wb_ofb_s sys_wb_ofb_t;

/****************************************************************
*
* ptp module
*
****************************************************************/
/* refer to ctc_ptp_device_type_t */
enum sys_wb_ptp_device_type_e
{
    SYS_WB_PTP_DEVICE_NONE = 0,
    SYS_WB_PTP_DEVICE_OC,
    SYS_WB_PTP_DEVICE_BC,
    SYS_WB_PTP_DEVICE_E2E_TC,
    SYS_WB_PTP_DEVICE_P2P_TC,

    MAX_SYS_WB_PTP_DEVICE
};
typedef enum sys_wb_ptp_device_type_e sys_wb_ptp_device_type_t;

enum sys_wb_appid_ptp_subid_e
{
    SYS_WB_APPID_PTP_SUBID_MASTER,
    SYS_WB_APPID_PTP_SUBID_MAX
};
typedef enum sys_wb_appid_ptp_subid_e sys_wb_appid_ptp_subid_t;

struct sys_wb_ptp_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint32 version;
    uint8 quanta;
    uint8 intf_selected;
    uint16 cache_aging_time;
    sys_wb_ptp_device_type_t device_type;
    uint8 tod_1pps_duty;
    uint32 tod_1pps_delay;
};
typedef struct sys_wb_ptp_master_s sys_wb_ptp_master_t;

/****************************************************************
*
* sync_ether module
*
****************************************************************/
/* DUET2 IS 2; TM IS 3 */
#define SYS_WB_SYNCETHER_CLOCK_MAX 3

enum sys_wb_appid_syncether_subid_e
{
    SYS_WB_APPID_SYNCETHER_SUBID_MASTER,
    SYS_WB_APPID_SYNCETHER_SUBID_MAX
};
typedef enum sys_wb_appid_syncether_subid_e sys_wb_appid_syncether_subid_t;

struct sys_wb_syncether_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint32 version;
    uint8 recovered_clock_lport[SYS_WB_SYNCETHER_CLOCK_MAX];
};
typedef struct sys_wb_syncether_master_s sys_wb_syncether_master_t;

/****************************************************************
*
* chip module
*
****************************************************************/
enum sys_wb_appid_chip_subid_e
{
    SYS_WB_APPID_CHIP_SUBID_MASTER,
    SYS_WB_APPID_CHIP_SUBID_MAX
};
typedef enum sys_wb_appid_chip_subid_e sys_wb_appid_chip_subid_t;

struct sys_wb_chip_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];

    /*data*/
    uint32 version;
    uint32 dev_id;
};
typedef struct sys_wb_chip_master_s sys_wb_chip_master_t;

/****************************************************************
*
* qos module
*
****************************************************************/
/* refer to ctc_qos_policer_type_t */
enum sys_wb_qos_policer_type_e
{
    SYS_WB_QOS_POLICER_TYPE_PORT,
    SYS_WB_QOS_POLICER_TYPE_FLOW,
    SYS_WB_QOS_POLICER_TYPE_VLAN,
    SYS_WB_QOS_POLICER_TYPE_SERVICE,
    SYS_WB_QOS_POLICER_TYPE_COPP,
    SYS_WB_QOS_POLICER_TYPE_MFP,

    SYS_WB_QOS_POLICER_TYPE_MAX
};
typedef enum sys_wb_qos_policer_type_e sys_wb_qos_policer_type_t;

/* refer to ctc_direction_t */
enum sys_wb_qos_direction_e
{
    SYS_WB_QOS_INGRESS,        /**< Ingress direction */
    SYS_WB_QOS_EGRESS,         /**< Egress direction */
    SYS_WB_QOS_BOTH_DIRECTION  /**< Both Ingress and Egress direction */
};
typedef enum sys_wb_qos_direction_e sys_wb_qos_direction_t;

/* refer to ctc_qos_policer_level_t */
enum sys_wb_qos_policer_level_e
{
    SYS_WB_QOS_POLICER_LEVEL_NONE,
    SYS_WB_QOS_POLICER_LEVEL_0,
    SYS_WB_QOS_POLICER_LEVEL_1,

    SYS_WB_QOS_POLICER_LEVEL_MAX
};
typedef enum sys_wb_qos_policer_level_e sys_wb_qos_policer_level_t;

/* refer to ctc_qos_policer_mode_t */
enum sys_wb_qos_policer_mode_e
{
    SYS_WB_QOS_POLICER_MODE_STBM,
    SYS_WB_QOS_POLICER_MODE_RFC2697,
    SYS_WB_QOS_POLICER_MODE_RFC2698,
    SYS_WB_QOS_POLICER_MODE_RFC4115,
    SYS_WB_QOS_POLICER_MODE_MEF_BWP,

    SYS_WB_QOS_POLICER_MODE_MAX
};
typedef enum sys_wb_qos_policer_mode_e sys_wb_qos_policer_mode_t;

/* refer to ctc_qos_color_t */
enum sys_wb_qos_color_e
{
    SYS_WB_QOS_COLOR_NONE,
    SYS_WB_QOS_COLOR_RED,
    SYS_WB_QOS_COLOR_YELLOW,
    SYS_WB_QOS_COLOR_GREEN,

    MAX_SYS_WB_QOS_COLOR
};
typedef enum sys_wb_qos_color_e sys_wb_qos_color_t;

/* refer to sys_qos_policer_level_t */
enum sys_wb_qos_policer_sys_level_e
{
    SYS_WB_QOS_POLICER_IPE_POLICING_0,
    SYS_WB_QOS_POLICER_IPE_POLICING_1,
    SYS_WB_QOS_POLICER_EPE_POLICING_0,
    SYS_WB_QOS_POLICER_EPE_POLICING_1
};
typedef enum sys_wb_qos_policer_sys_level_e  sys_wb_qos_policer_sys_level_t;

/* refer to ctc_qos_policer_action_flag_t */
enum sys_wb_qos_policer_action_flag_e
{
    SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID = 1U << 0,
    SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID = 1U << 1,
    SYS_WB_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID = 1U << 2
};
typedef enum sys_wb_qos_policer_action_flag_e sys_wb_qos_policer_action_flag_t;

/* refer to sys_qos_policer_type_t */
enum sys_wb_qos_policer_sys_type_e
{
    SYS_WB_QOS_POLICER_SYS_TYPE_PORT,
    SYS_WB_QOS_POLICER_SYS_TYPE_VLAN,
    SYS_WB_QOS_POLICER_SYS_TYPE_FLOW,
    SYS_WB_QOS_POLICER_SYS_TYPE_MACRO_FLOW,
    SYS_WB_QOS_POLICER_SYS_TYPE_SERVICE,
    SYS_WB_QOS_POLICER_SYS_TYPE_COPP,
    SYS_WB_QOS_POLICER_SYS_TYPE_MFP
};
typedef enum sys_wb_qos_policer_sys_type_e  sys_wb_qos_policer_sys_type_t;

/* refer to sys_extend_que_type_t */
enum sys_wb_qos_extend_que_type_e
{
    SYS_WB_QOS_EXTEND_QUE_TYPE_SERVICE,
    SYS_WB_QOS_EXTEND_QUE_TYPE_BPE,
    SYS_WB_QOS_EXTEND_QUE_TYPE_C2C,
    SYS_WB_QOS_EXTEND_QUE_TYPE_LOGIC_PORT,

    SYS_WB_QOS_EXTEND_QUE_TYPE_MAX
};
typedef enum sys_wb_qos_extend_que_type_e sys_wb_qos_extend_que_type_t;

/* refer to ctc_pkt_cpu_reason_dest_t */
enum sys_wb_qos_pkt_cpu_reason_dest_e
{
    SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU = 0,
    SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_PORT,
    SYS_WB_QOS_PKT_CPU_REASON_TO_REMOTE_CPU,
    SYS_WB_QOS_PKT_CPU_REASON_TO_DROP,
    SYS_WB_QOS_PKT_CPU_REASON_TO_NHID,
    SYS_WB_QOS_PKT_CPU_REASON_TO_LOCAL_CPU_ETH,

    SYS_WB_QOS_PKT_CPU_REASON_TO_MAX_COUNT
};
typedef enum sys_wb_qos_pkt_cpu_reason_dest_e sys_wb_qos_pkt_cpu_reason_dest_t;

enum sys_wb_appid_qos_subid_e
{
    SYS_WB_APPID_QOS_SUBID_MASTER,
    SYS_WB_APPID_QOS_SUBID_POLICER,
    SYS_WB_APPID_QOS_SUBID_POLICER_MFP_PROFILE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_MASTER,
    SYS_WB_APPID_QOS_SUBID_QUEUE_QUEUE_NODE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_GROUP_NODE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_FC_PROFILE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_PROFILE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_PORT_EXTENDER,
    SYS_WB_APPID_QOS_SUBID_CPU_REASON,
    SYS_WB_APPID_QOS_SUBID_QUEUE_FC_DROPTH_PROFILE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_PFC_DROPTH_PROFILE,
    SYS_WB_APPID_QOS_SUBID_QUEUE_LOGIC_SRC_PORT,
    SYS_WB_APPID_QOS_SUBID_QUEUE_DESTPORT,

    SYS_WB_APPID_QOS_SUBID_MAX
};
typedef enum sys_wb_appid_qos_subid_e sys_wb_appid_qos_subid_t;

struct sys_wb_qos_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint32 version;
};
typedef struct sys_wb_qos_master_s sys_wb_qos_master_t;

struct sys_wb_qos_policer_s
{
    /* key */
    uint8  type; /*ctc_qos_policer_type_t*/
    uint8  dir;   /*ctc_direction_t */
    uint16 id;
    uint32 calc_key_len[0];

    /* data */
    uint8  level;
    uint8  split_en;

    uint8  hbwp_en;
    uint8  is_pps;
    uint16 cos_bitmap; /*bitmap of hbwp enable*/
    uint8  cos_index;
    uint8  sf_en;
    uint8  cf_total_dis;

    uint8  policer_mode;
    uint8  cf_en;
    uint8  is_color_aware;
    uint8  use_l3_length;

    uint8  drop_color;
    uint8  entry_size;

    uint8  stats_en;
    uint8  stats_mode;
    uint8  stats_num;

    uint8  stats_useX;
    uint8  stats_useY;

    uint32 cir;
    uint32 cbs;

    uint32 pir;
    uint32 pbs;

    uint32 cir_max;
    uint32 pir_max;

    uint8  policer_lvl;

    uint8  prio_green;
    uint8  prio_yellow;
    uint8  prio_red;
    uint32 flag;                /*refer to ctc_qos_policer_action_flag_t*/

    uint8 sys_policer_type;     /*refer to sys_qos_policer_type_t*/
    uint8 is_installed;
    uint16 policer_ptr;

    uint16 stats_ptr[8];

    uint8 is_sys_profileX_valid[8];
    uint8 sys_profileX_profile_id[8];

    uint8 is_sys_profileY_valid[8];
    uint8 sys_profileY_profile_id[8];

    uint8 is_sys_action_valid[8];
    uint8 sys_action_profile_id[8];
    uint8 sys_action_dir[8];

    uint8 is_sys_copp_valid;
    uint8 sys_copp_profile_id;
    uint8 color_merge_mode;
};
typedef struct sys_wb_qos_policer_s sys_wb_qos_policer_t;

struct sys_wb_qos_queue_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint8 store_chan_shp_en;
    uint8 store_que_shp_en;
    uint8 monitor_drop_en;
    uint8 shp_pps_en;
    uint8 queue_thrd_factor;
};
typedef struct sys_wb_qos_queue_master_s sys_wb_qos_queue_master_t;

struct sys_wb_qos_queue_queue_node_s
{
    /* key */
    uint16 queue_id;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /* data */
    uint8 is_shp_profile_valid;
    uint8 shp_profile_id;
    uint8 shp_profile_queue_type;

    uint8 is_meter_profile_valid;
    uint8 meter_profile_id;

    uint8 is_drop_wtd_profile_valid;
    uint8 drop_wtd_profile_id;

    uint8 is_drop_wred_profile_valid;
    uint8 drop_wred_profile_id;
};
typedef struct sys_wb_qos_queue_queue_node_s sys_wb_qos_queue_queue_node_t;

struct sys_wb_qos_queue_group_node_s
{
    /* key */
    uint16 group_id;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /* data */
    uint8 chan_id;
    uint16 shp_bitmap;

    uint8 is_shp_profile_valid;
    uint8 shp_profile_id;

    uint8 is_meter_profile_valid;
    uint8 meter_profile_id;
};
typedef struct sys_wb_qos_queue_group_node_s sys_wb_qos_queue_group_node_t;

struct sys_wb_qos_queue_fc_profile_s
{
    /* key */
    uint8 chan_id;
    uint8 rsv[3];
    uint32 calc_key_len[0];

    /* data */
    uint8 fc_profile_id;
};
typedef struct sys_wb_qos_queue_fc_profile_s sys_wb_qos_queue_fc_profile_t;

struct sys_wb_qos_queue_pfc_profile_s
{
    /* key */
    uint8 chan_id;
    uint8 priority_class;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /* data */
    uint8 pfc_profile_id;
};
typedef struct sys_wb_qos_queue_pfc_profile_s sys_wb_qos_queue_pfc_profile_t;

struct sys_wb_qos_queue_port_extender_s
{
    /* key */
    uint8  lchip;
    uint8 rsv;
    uint16 lport;
    uint8 channel;
    uint8 type;     /** sys_extend_que_type_t */
    uint16 service_id;
    uint16 logic_src_port;
    uint16 rsv1;
    uint32 calc_key_len[0];

    /* data */
    uint16 group_id;
    uint16 key_index;
    uint16 logic_dst_port;
    uint16 rsv2;
};
typedef struct sys_wb_qos_queue_port_extender_s sys_wb_qos_queue_port_extender_t;

struct sys_wb_qos_cpu_reason_s
{
    /* key */
    uint16 reason_id;
    uint8 rsv[2];
    uint32 calc_key_len[0];

    /* data */
    uint8 dest_type;
    uint8 user_define_mode;   /*0-none;1-DsFwd/ misc nexthop;2.alloc excep ID*/
    uint16 excp_id;
    uint32 dsfwd_offset;
    uint16 sub_queue_id;
    uint16 ref_cnt;
    uint32 dest_port;
    uint32 dest_map;
    uint8 dir;
};
typedef struct sys_wb_qos_cpu_reason_s sys_wb_qos_cpu_reason_t;

struct sys_wb_qos_logicsrc_port_s
{
    /* key */
    uint16 service_id;
    uint16 logic_src_port;
    uint32 calc_key_len[0];

    /* data */
};
typedef struct sys_wb_qos_logicsrc_port_s sys_wb_qos_logicsrc_port_t;

struct sys_wb_qos_destport_s
{
    /* key */
    uint16 service_id;
    uint8 lport;
    uint8 lchip;
    uint32 calc_key_len[0];

    /* data */
};
typedef struct sys_wb_qos_destport_s sys_wb_qos_destport_t;

/* refer to sys_usw_security_stmctl_mode_t */
enum sys_wb_security_stmctl_mode_e
{
    SYS_WB_SECURITY_MODE_MERGE,
    SYS_WB_SECURITY_MODE_SEPERATE,
    SYS_WB_SECURITY_MODE_NUM
};
typedef enum sys_wb_security_stmctl_mode_e sys_wb_security_stmctl_mode_t;

enum sys_wb_appid_security_subid_e
{
    SYS_WB_APPID_SECURITY_SUBID_MASTER,
    SYS_WB_APPID_SECURITY_SUBID_TRHOLD_PROFILE,

    SYS_WB_APPID_SECURITY_SUBID_MAX
};
typedef enum sys_wb_appid_security_subid_e sys_wb_appid_security_subid_t;

struct sys_wb_security_master_s
{
    /* key */
    uint32 lchip;
    uint32 calc_key_len[0];

    /* data */
    uint32 version;
    uint8 ipsg_flag;
    uint8 ip_src_guard_def;
    uint8 tcam_group0;
    uint8 tcam_group1;
    uint8 hash_group0;
    uint8 hash_group1;
    uint8 rsv[3];
};
typedef struct sys_wb_security_master_s sys_wb_security_master_t;

struct sys_wb_security_trhold_profile_s
{
    /* key */
    uint32 value;
    uint32 calc_key_len[0];

    /* data */
    uint32 index;
};
typedef struct sys_wb_security_trhold_profile_s sys_wb_security_trhold_profile_t;

/****************************************************************
*
* datapath module
*
****************************************************************/
enum sys_wb_appid_datapath_subid_e
{
    SYS_WB_APPID_DATAPATH_SUBID_MASTER,
    SYS_WB_APPID_DATAPATH_SUBID_HSS_ATTRIBUTE,
    SYS_WB_APPID_DATAPATH_SUBID_MAX
};
typedef enum sys_wb_appid_datapath_subid_e sys_wb_appid_datapath_subid_t;

struct sys_wb_datapath_lport_attr_s
{
    uint8 port_type;  /*refer to sys_dmps_lport_type_t */
    uint8 mac_id;
    uint8 chan_id;
    uint8 speed_mode;  /*refer to sys_port_speed_t, only usefull for network port*/

    uint8 slice_id;
    uint8 serdes_num;  /*serdes num of port*/
    uint8 pcs_mode; /*refer to ctc_chip_serdes_mode_t*/
    uint8 serdes_id;

    uint8 is_serdes_dyn;
    uint8 is_first;   /* 40G/100G, 4 interrupts will process the first*/
    uint8 flag;       /* for D2 50G, if serdes lane 0/1 and 2/3 form 50G port, flag is eq to 0;
                                     if serdes lane 2/1 and 0/3 form 50G port, flag is eq to 1; */
    uint8 interface_type;    /* refer to ctc_port_if_type_t */

    uint8 an_fec;      /* refer to ctc_port_fec_type_t */
    uint8 code_err_count;   /* code error count*/
    uint8 pcs_reset_en;
    uint8 tbl_id;           /*TM ONLY  table id for mac table 0~71*/

    uint8 sgmac_idx;
    uint8 mii_idx;
    uint8 pcs_idx;
    uint8 internal_mac_idx;

    uint8 first_led_mode;
    uint8 sec_led_mode;

    uint8 xpipe_en;
    uint8 pmac_id;
    uint8 emac_id;
    uint8 pmac_chanid;
    uint8 emac_chanid;
};
typedef struct sys_wb_datapath_lport_attr_s sys_wb_datapath_lport_attr_t;

struct sys_wb_datapath_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];
    /*data*/
    uint32 version;
    sys_wb_datapath_lport_attr_t port_attr[SYS_MAX_LOCAL_SLICE_NUM][256];
    sys_datapath_qm_choice_t qm_choice;     /*for TM only*/
    uint8  serdes_mac_map[4][32];           /*for TM only*/
    uint16 mii_lport_map[4][32];            /*for TM only*/
    uint8  glb_xpipe_en;            /*for TM and later*/
};
typedef struct sys_wb_datapath_master_s sys_wb_datapath_master_t;

struct sys_wb_datapath_serdes_info_s
{
    uint8 mode;        /*pcs mode, refer to ctc_chip_serdes_mode_t*/
    uint8 pll_sel;       /* 0: disable,1:plla. 2:pllb */
    uint8 rate_div;     /*0: full, 1:1/2, 2:1/4, 3:1/8*/
    uint8 bit_width;   /* bit width*/
    uint16 lport;        /*chip local phy port*/
    uint8 port_num;  /*serdes port count*/
    uint8 rx_polarity;  /*0:normal, 1:reverse*/
    uint8 tx_polarity;  /*0:normal, 1:reverse*/
    uint8 lane_id;  /*serdes internal lane id */
    uint8 is_dyn;
    uint8 group;
    uint16 overclocking_speed; /*serdes over clock*/
    uint8 rsv[2];
};
typedef struct sys_wb_datapath_serdes_info_s   sys_wb_datapath_serdes_info_t;

struct sys_wb_datapath_hss_attribute_s
{
    /*key*/
    uint32 hss_idx;
    uint32 calc_key_len[0];
    /*data*/
    uint8 hss_type;   /*0:Hss15G, 1:Hss28G, refer sys_datapath_hss_type_t*/
    uint8 hss_id;        /*for hss15g is 0~9, for hss28g is 0~3*/
    uint8 plla_mode;  /*refer to sys_datapath_hssclk_t */
    uint8 pllb_mode;  /*refer to sys_datapath_hssclk_t */
    uint8 pllc_mode;  /*(TM ONLY) refers to sys_datapath_12g_cmu_type_t*/
    uint8 core_div_a[SYS_DATAPATH_CORE_NUM];  /*refer to sys_datapath_clktree_div_t, usefull plla is used*/
    uint8 core_div_b[SYS_DATAPATH_CORE_NUM];  /*refer to sys_datapath_clktree_div_t, usefull plla is used*/
    uint8 intf_div_a;   /*refer to sys_datapath_clktree_div_t, usefull pllb is used*/
    uint8 intf_div_b;   /*refer to sys_datapath_clktree_div_t, usefull pllb is used*/
    sys_wb_datapath_serdes_info_t  serdes_info[HSS15G_LANE_NUM];   /*hss28G only 4 serdes info is valid, refer to sys_datapath_serdes_info_t  */
    uint8 plla_ref_cnt;      /*indicate used lane number for the plla, if the num decrease to 0, set plla to disable*/
    uint8 pllb_ref_cnt;      /*indicate used lane number for the pllb, if the num decrease to 0, set pllb to disable*/
    uint8 pllc_ref_cnt;      /*indicate used lane number for the pllc, if the num decrease to 0, set pllc to disable  For TM only*/
    uint16 clktree_bitmap; /*indicate clktree need cfg, refer to  sys_datapath_clktree_op_t */
};
typedef struct sys_wb_datapath_hss_attribute_s sys_wb_datapath_hss_attribute_t;

/****************************************************************
*
* ftm module
*
****************************************************************/
enum sys_wb_appid_ftm_subid_e
{
    SYS_WB_APPID_FTM_SUBID_AD,
    SYS_WB_APPID_FTM_SUBID_MAX
};
struct sys_wb_ftm_ad_s
{
    /*key*/
    uint8 opf_pool_type;
    uint8 opf_pool_index;
    uint16 sub_id;
    uint32 calc_key_len[0];
    /*data*/
    uint32 start;
    uint32 end;
    uint8 reverse;
};
typedef struct sys_wb_ftm_ad_s sys_wb_ftm_ad_t;

/****************************************************************
*
* register module
*
****************************************************************/
enum sys_wb_appid_register_subid_e
{
    SYS_WB_APPID_REGISTER_SUBID_MASTER,
    SYS_WB_APPID_REGISTER_SUBID_TRUNC_POOL,
    SYS_WB_APPID_REGISTER_SUBID_MAX
};
typedef enum sys_wb_appid_register_subid_e sys_wb_appid_register_subid_t;

struct sys_wb_register_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];
    /*data*/
    uint32 version;
    uint8  oam_coexist_mode;
    uint8  rsv[3];
};
typedef struct sys_wb_register_master_s sys_wb_register_master_t;

struct sys_wb_register_trunc_pool_s
{
    /*key*/
    uint8 profile_id;
    uint8 rsv[3];
    uint32 calc_key_len[0];
    /*data*/
    uint32 ref_cnt;
};
typedef struct sys_wb_register_trunc_pool_s sys_wb_register_trunc_pool_t;

/****************************************************************
*
* packet module
*
****************************************************************/
#define WB_PKT_MAX_NETIF_NAME_LEN        16
enum sys_wb_appid_packet_subid_e
{
    SYS_WB_APPID_PACKET_SUBID_MASTER,
    SYS_WB_APPID_PACKET_SUBID_NETIF,
    SYS_WB_APPID_PACKET_SUBID_MAX
};
typedef enum sys_wb_appid_packet_subid_e sys_wb_appid_packet_subid_t;

struct sys_wb_pkt_master_s
{
    /*key*/
    uint32 lchip;
    uint32 calc_key_len[0];
    /*data*/
    uint32 version;
};
typedef struct sys_wb_pkt_master_s sys_wb_pkt_master_t;

struct sys_wb_pkt_netif_s
{
    /*key*/
    uint8 type;     /**<[TM] Network interface type, 0: associated with physical switch port, 1: associated with specified vlan id */
    uint8 rsv;
    uint16 vlan;    /**<[TM] Vlan id associated with this interface */
    uint32 gport;  /**<[TM] Local port associated with this interface */
    uint32 calc_key_len[0];
    /*data*/
    mac_addr_t mac; /**<[TM] MAC address associated with this interface */
    char name[WB_PKT_MAX_NETIF_NAME_LEN]; /**<[TM] Network interface name */
};
typedef struct sys_wb_pkt_netif_s sys_wb_pkt_netif_t;

#ifdef __cplusplus
}
#endif

#endif
