/**
 @file sys_usw_nexthop_api.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_NEXTHOP_API_H_
#define _SYS_USW_NEXTHOP_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_vlan.h"
#include "ctc_nexthop.h"
#include "ctc_stats.h"

/*global reserved nexthop */
#define SYS_NH_RESOLVED_NHID_FOR_NONE            0
#define SYS_NH_RESOLVED_NHID_FOR_DROP            1
#define SYS_NH_RESOLVED_NHID_FOR_TOCPU           2

#define SYS_NH_RESOLVED_NHID_MAX              3

#define SYS_NHID_EXTERNAL_MAX                  8192
#define SYS_NHID_INTERNAL_START  (SYS_NHID_EXTERNAL_MAX)
#define SYS_NH_INVALID_OFFSET        0xFFFF
#define SYS_NH_INVALID_NHID            0xFFFFFFFF
#define SYS_NH_MET_DROP_UCAST_ID          SYS_RSV_PORT_DROP_ID
#define SYS_NH_DSNH_BY_PASS_FLAG   (1 << 17)    /*Egress Edit Mode for By pass Ingress Chip Edit */
#define SYS_NH_IGS_CHIP_EDIT_MODE  0
#define SYS_NH_EGS_CHIP_EDIT_MODE  1
#define SYS_NH_MISC_CHIP_EDIT_MODE 2
#define SYS_NH_INTERNAL_NHID_BASE 0x80000000
typedef int32 (* nh_traversal_fn)(uint8 lchip, void* nh_info, void* user_data);

enum sys_nh_info_flag_type_e
{
    SYS_NH_INFO_FLAG_IS_UNROV = 0x0001,
    SYS_NH_INFO_FLAG_USE_DSNH8W = 0x0002,
    SYS_NH_INFO_FLAG_IS_APS_EN = 0x0004,
    SYS_NH_INFO_FLAG_HAVE_DSFWD = 0x0008,
    SYS_NH_INFO_FLAG_USE_ECMP_IF = 0x0010,
    SYS_NH_INFO_FLAG_HAVE_L2EDIT = 0x0020,
    SYS_NH_INFO_FLAG_HAVE_L3EDIT = 0x0040,
    SYS_NH_INFO_FLAG_HAVE_INT_ALLOC_DSNH = 0x0080,
    SYS_NH_INFO_FLAG_MPLS_POP_NH = 0x0100,
    SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH = 0x0200,
    SYS_NH_INFO_FLAG_HAVE_RSV_DSNH = 0x0400,
    SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE = 0x0800,
    SYS_NH_INFO_FLAG_LOGIC_PORT = 0x1000,
    SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN = 0x2000,
    SYS_NH_INFO_FLAG_HAVE_SWAP_L2EDIT = 0x4000,
    SYS_NH_INFO_FLAG_LEN_ADJUST_EN = 0x8000,
    SYS_NH_INFO_FLAG_HAVE_L2EDI_3W = 0x10000,
    SYS_NH_INFO_FLAG_HAVE_L2EDI_6W = 0x20000,
    SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD = 0x40000,
    SYS_NH_INFO_FLAG_ROUTE_OVERLAY = 0x80000,
    SYS_NH_INFO_FLAG_LOGIC_PORT_CHECK = 0x100000,
    SYS_NH_INFO_FLAG_LOGIC_REPLI_EN = 0x200000,
    SYS_NH_INFO_FLAG_MCAST_PROFILE = 0x400000,
    SYS_NH_INFO_FLAG_DSFWD_IS_6W = 0x800000,
    SYS_NH_INFO_FLAG_LOOP_USED = 0x1000000
};
typedef enum sys_nh_info_flag_type_e sys_nh_info_flag_type_t;

enum sys_nh_info_mcast_flag_e
{
    SYS_NH_INFO_MCAST_FLAG_IS_MIRROR          =  0x0001,
    SYS_NH_INFO_MCAST_FLAG_STK_MEM_PROFILE    =  0x0002,   /*means mcast member have used mcast profile, only support one member is profile*/
    SYS_NH_INFO_MCAST_FLAG_MEM_PROFILE        =  0x0004,   /*means mcast member have used mcast profile, only support one member is profile*/
    SYS_NH_INFO_MCAST_FLAG_LOGIC_REP_EN       =  0x0008,   /*means mcast group enable logic replicate, for ipmc */

    SYS_NH_INFO_MCAST_FLAG_MAX
};
typedef enum sys_nh_info_mcast_flag_e sys_nh_info_mcast_flag_t;

enum sys_nh_entry_table_type_e
{
    SYS_NH_ENTRY_TYPE_NULL,                                             /*0*/

    SYS_NH_ENTRY_TYPE_FWD = SYS_NH_ENTRY_TYPE_NULL,                     /*0*/
    SYS_NH_ENTRY_TYPE_MET,                                              /*1*/
    SYS_NH_ENTRY_TYPE_MET_6W,                                           /*2*/
    SYS_NH_ENTRY_TYPE_MET_FOR_IPMCBITMAP,                               /*3*/
    SYS_NH_ENTRY_TYPE_NEXTHOP_INTERNAL,                                 /*4*/
    SYS_NH_ENTRY_TYPE_NEXTHOP_4W,                                       /*5*/
    SYS_NH_ENTRY_TYPE_NEXTHOP_8W,                                       /*6*/

    /* +++ l2edit +++ */
    SYS_NH_ENTRY_TYPE_L2EDIT_FROM,                                      /*7*/
    SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W = SYS_NH_ENTRY_TYPE_L2EDIT_FROM,  /*7*/
    SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W,                                  /*8*/
    SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W,                                  /*9*/
    SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W,                                  /*10*/
    SYS_NH_ENTRY_TYPE_L2EDIT_FLEX_8W,                                   /*11*/
    SYS_NH_ENTRY_TYPE_L2EDIT_OF_6W,                                     /*12*/
    SYS_NH_ENTRY_TYPE_L2EDIT_LPBK,                                      /*13*/
    SYS_NH_ENTRY_TYPE_L2EDIT_PBB_4W,                                    /*14*/
    SYS_NH_ENTRY_TYPE_L2EDIT_PBB_8W,                                    /*15*/
    SYS_NH_ENTRY_TYPE_L2EDIT_SWAP,                                      /*16, 6w size*/
    SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP,                                /*17, 3w size*/
    SYS_NH_ENTRY_TYPE_L2EDIT_TO = SYS_NH_ENTRY_TYPE_L2EDIT_INNER_SWAP,  /*17*/
    /* --- l2edit --- */

    /* +++ l3edit +++ */
    SYS_NH_ENTRY_TYPE_L3EDIT_FROM,                                      /*18*/
    SYS_NH_ENTRY_TYPE_L3EDIT_FLEX= SYS_NH_ENTRY_TYPE_L3EDIT_FROM,       /*18*/
    SYS_NH_ENTRY_TYPE_L3EDIT_MPLS,                                      /*19*/
    SYS_NH_ENTRY_TYPE_L3EDIT_SPME,                                      /*20*/
    SYS_NH_ENTRY_TYPE_L3EDIT_MPLS_12W,                                  /*21*/
    SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W,                                     /*22*/
    SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W,                                    /*23*/
    SYS_NH_ENTRY_TYPE_L3EDIT_NAT_4W,                                    /*24*/
    SYS_NH_ENTRY_TYPE_L3EDIT_NAT_8W,                                    /*25*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TRILL,                                     /*26*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4,                                 /*27*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6,                                 /*28*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4_IPSA,                            /*29*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA,                            /*30*/
    SYS_NH_ENTRY_TYPE_L3EDIT_TO = SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6_IPSA,  /*30*/
    /* --- l3edit --- */

    /* +++ ecmp +++ */
    SYS_NH_ENTRY_TYPE_ECMP_GROUP,                                        /*31*/
    SYS_NH_ENTRY_TYPE_ECMP_MEMBER,                                       /*32*/
    SYS_NH_ENTRY_TYPE_ECMP_RR_COUNT,                                     /*33*/
    /* --- ecmp --- */

    SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE,                                /*34*/
    SYS_NH_ENTRY_TYPE_DESTMAP_PROFILE,                                    /*35*/
    SYS_NH_ENTRY_TYPE_MAX
};
typedef enum sys_nh_entry_table_type_e sys_nh_entry_table_type_t;

enum sys_nh_param_mcast_opcode_e
{
    SYS_NH_PARAM_MCAST_ADD_MEMBER            = 1,
    SYS_NH_PARAM_MCAST_DEL_MEMBER            = 2
};
typedef enum sys_nh_param_mcast_opcode_e sys_nh_param_mcast_opcode_t;

enum sys_usw_nh_res_offset_type_e
{
    SYS_NH_RES_OFFSET_TYPE_NONE = 0,

    SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH = SYS_NH_RES_OFFSET_TYPE_NONE,  /*port tagged mode*/
    SYS_NH_RES_OFFSET_TYPE_BYPASS_NH,
    SYS_NH_RES_OFFSET_TYPE_MIRROR_NH,
    SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH,
    /* SYSTEM MODIFIED by liwh for merge bug 100413 & 100427 to support 1br */
    SYS_NH_RES_OFFSET_TYPE_BYPASS_ECID_NH,

    SYS_NH_RES_OFFSET_TYPE_MAX
};
typedef enum sys_usw_nh_res_offset_type_e sys_usw_nh_res_offset_type_t;

enum sys_nh_param_mcast_member_type_s
{
    SYS_NH_PARAM_BRGMC_MEM_LOCAL = 0,
    SYS_NH_PARAM_BRGMC_MEM_LOCAL_WITH_APS_BRIDGE,
    SYS_NH_PARAM_IPMC_MEM_LOCAL,
    SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH,
    SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH_DESTID,
    SYS_NH_PARAM_MCAST_MEM_REMOTE,
    SYS_NH_PARAM_BRGMC_MEM_RAPS,
    SYS_NH_PARAM_MCAST_MEM_LOCAL_LOGIC_REP,
    SYS_NH_PARAM_IPMC_MEM_LOCAL_LOGIC_REP,
    SYS_NH_PARAM_MCAST_MEM_MIRROR_WITH_NH,
    SYS_NH_PARAM_IPMC_MEM_LOOP_BACK,
    SYS_NH_PARAM_MCAST_MEM_INVALID
};
typedef enum sys_nh_param_mcast_member_type_s sys_nh_param_mcast_member_type_t;

enum sys_usw_nh_type_e
{
    SYS_NH_TYPE_NULL,
    SYS_NH_TYPE_MCAST,
    SYS_NH_TYPE_BRGUC,
    SYS_NH_TYPE_IPUC,
    SYS_NH_TYPE_MPLS,
    SYS_NH_TYPE_ECMP, /*For IPUC, MPLS, etc*/
    SYS_NH_TYPE_DROP,
    SYS_NH_TYPE_TOCPU,
    SYS_NH_TYPE_UNROV,
    SYS_NH_TYPE_ILOOP,
    SYS_NH_TYPE_ELOOP,
    SYS_NH_TYPE_RSPAN,
    SYS_NH_TYPE_IP_TUNNEL, /*Added by IP Tunnel*/
    SYS_NH_TYPE_TRILL,
    SYS_NH_TYPE_MISC,
    SYS_NH_TYPE_WLAN_TUNNEL,
    SYS_NH_TYPE_MAX
};
typedef enum sys_usw_nh_type_e sys_usw_nh_type_t;

struct sys_nh_ref_list_node_s
{
    struct sys_nh_info_com_s* p_ref_nhinfo;
    struct sys_nh_ref_list_node_s* p_next;
};
typedef struct sys_nh_ref_list_node_s sys_nh_ref_list_node_t;

struct sys_nh_info_dsfwd_s
{
    uint32 dsfwd_offset;
    uint32 stats_ptr;
    uint32 dsnh_offset;
    uint16 dest_chip;
    uint16 dest_id;
};
typedef struct sys_nh_info_dsfwd_s sys_nh_info_dsfwd_t;

struct sys_nh_info_com_hdr_s
{
    uint8 nh_entry_type; /*sys_usw_nh_type_t*/
    uint8 dsnh_entry_num;
	uint8 adjust_len;
	uint8 rsv;
    uint32 nh_entry_flags; /*sys_nh_info_flag_type_t*/
    uint32 nh_id;
    sys_nh_info_dsfwd_t  dsfwd_info;
    sys_nh_ref_list_node_t* p_ref_nh_list;   /*sys_nh_ref_list_node_t*/
    void* p_data;
};
typedef struct sys_nh_info_com_hdr_s sys_nh_info_com_hdr_t;

struct sys_nh_info_com_s
{
    sys_nh_info_com_hdr_t hdr;
    uint32 data;
};
typedef struct sys_nh_info_com_s sys_nh_info_com_t;

struct sys_nh_param_mcast_member_s
{
    sys_nh_param_mcast_member_type_t member_type;
    uint32  ref_nhid; /*Reference other nhid, eg egress vlan translation nhid*/
    uint16  destid; /*local portid, LAGID without 0x1F(eg.0x1F01), global dest chipid, aps bridge groupid*/
    uint16  vid; /*For IPMC*/
    uint8   is_linkagg;
    uint8   lchip; /*Local chip this member will be added to */
    uint8   l3if_type; /*ctc_l3if_type_t*/
    uint8   is_protection_path; /*Used for aps bridge*/
    uint16  logic_port;
    uint8   is_logic_port; /*if set,Met will do logic port check,apply to VPLS/WLAN/PBB/TRILL,etc.*/
    uint8   is_reflective;      /**< [GB] Mcast reflective */
    uint8   is_mirror;
    uint8   is_horizon_split;
    uint8   is_src_port_check_dis;
    uint8   is_destmap_profile;
    uint8   leaf_check_en;
    uint8   mtu_no_chk;
    uint8   cloud_sec_en;
};
typedef struct sys_nh_param_mcast_member_s sys_nh_param_mcast_member_t;

enum sys_nh_param_brguc_sub_type_e
{
    SYS_NH_PARAM_BRGUC_SUB_TYPE_NONE =  0,
    SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC = 1,
    SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS = 2,

    /*Output vlan edit*/
    SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT = 3,
    SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT = 4,

    SYS_NH_PARAM_BRGUC_SUB_TYPE_MAX
};
typedef enum sys_nh_param_brguc_sub_type_e sys_nh_param_brguc_sub_type_t;

struct sys_nh_param_mcast_group_s
{
    uint32                      nhid;
    uint32                      opcode;  /*sys_nh_param_mcast_opcode_t */
    sys_nh_param_mcast_member_t mem_info;
    uint32                     fwd_offset;
    uint32                      stats_id;
    uint8                       stats_valid;
    uint8                       dsfwd_valid;
    uint8                       is_nhid_valid;
    uint8                       is_mirror;
    uint8                       is_mcast_profile;
};
typedef struct sys_nh_param_mcast_group_s sys_nh_param_mcast_group_t;

enum sys_nh_entry_change_type_e
{
    SYS_NH_CHANGE_TYPE_NULL,

    SYS_NH_CHANGE_TYPE_FWD_TO_UNROV,
    SYS_NH_CHANGE_TYPE_UNROV_TO_FWD,
    SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR,

    SYS_NH_CHANGE_TYPE_FWD_TO_DISCARD,
    SYS_NH_CHANGE_TYPE_DISCARD_TO_FWD,

    SYS_NH_CHANGE_TYPE_ADD_DYNTBL,

    SYS_NH_CHANGE_TYPE_NH_DELETE,
    SYS_NH_CHANGE_TYPE_INVALID
};
typedef enum sys_nh_entry_change_type_e sys_nh_entry_change_type_t;

struct sys_nh_brguc_nhid_info_s
{
    uint32 nh_type_flag;
    uint32 brguc_untagged_nhid;
    uint32 brguc_nhid;
    uint32 bypass_nhid;
    uint32 ram_pkt_nhid;   /*No use*/
    uint32 srv_queue_nhid; /*No use*/
};
typedef struct sys_nh_brguc_nhid_info_s sys_nh_brguc_nhid_info_t;  /*No use*/

struct sys_nh_info_dsnh_s
{
    uint8    nh_entry_type; /*nexthop type*/
    uint32   dsnh_offset;
    uint32   dsfwd_offset;
    uint8    nexthop_ext;
    uint8    adjust_len;
    uint8    is_mcast;
    uint8    drop_pkt;
    uint32   gport;       /*ucast output port, for mcast means mcast group ID */
    uint32   dest_map;

    uint8    dsfwd_valid;
    uint8    aps_en;      /*if enable, destid is aps group id*/
    uint8    merge_dsfwd; /*0:may need dsfwd; 1.don't dsfwd;2.must need dsfwd */
    uint8    is_ivi;      /*sub type is ivi of ip-tunnel*/
    uint8    stats_en;

    uint8    to_cpu_en;   /*for arp miss to cpu*/

    /*ecmp info*/
    uint8    ecmp_valid;
    uint8    is_ecmp_intf;
    uint16   ecmp_group_id;
    uint8    ecmp_num;
    uint8    valid_cnt;   /*valid ecmp member*/
    uint8    ecmp_cnt;    /*include valid & unresoved ecmp member*/
    uint32   nh_array[SYS_USW_MAX_ECPN]; /*valid nh array[valid_cnt] -->unresoved nh*/

    /*oam aps info*/
    uint8    protection_path;   /*[input]mep on PW label*/
    uint8    mep_on_tunnel;	    /*[output]mep on tunnel label*/
    uint16   oam_aps_group_id;  /*if mep on PW label,oam_aps_group_id is tunnel aps group id;
                                  if mep on tunnel label, oam_aps_group_id is tunnel group id */
    uint16   oam_aps_en;
    uint8    have_l2edit;
    uint8    cloud_sec_en;

    /* ip-tunnel info */
    uint8   re_route;
    uint8   use_destmap_profile;
    uint16  arp_id;

    uint8 individual_nexthop;/*if set, means two dsnexthop for aps*/
    uint16 tunnel_id;

    /* Merge Dsfwd update*/
    void* bind_data;   /*used for merge dsfwd update*/
    int32 (* updateAd) (uint8 lchip, void* data, void* change_nh_param);

};
typedef struct sys_nh_info_dsnh_s sys_nh_info_dsnh_t;

struct sys_nh_update_dsnh_param_s
{

    void*     data;
    int32 (* updateAd) (uint8 lchip, void* data, void* change_nh_param);

};
typedef struct sys_nh_update_dsnh_param_s sys_nh_update_dsnh_param_t;

struct sys_nh_param_dsfwd_s
{
    uint32 dsfwd_offset;
    uint32 dsnh_offset;
    uint8 is_mcast;
    uint8 is_reflective;
    uint8 drop_pkt;
    uint8 nexthop_ext;
	uint8 is_lcpu;
	uint8 is_egress_edit;
	uint8 adjust_len_idx;
    uint8 aps_type;
    uint16 dest_id; /*UcastId or McastId*/
    uint8 dest_chipid; /*Destination global chip id*/
    uint8 cut_through_dis;
    uint32 stats_ptr; /*This entry's target local chipid*/
    uint32 is_cpu;
    uint8  use_destmap_profile;
    uint8  truncate_profile_id;
    uint8  cloud_sec_en;
    uint8  is_6w;
};
typedef struct sys_nh_param_dsfwd_s sys_nh_param_dsfwd_t;

struct sys_nh_param_dsmet_s
{
    uint16 dest_id;
    uint16 next_met_entry_ptr;
    uint8 remote_chip;
    uint8 end_local_rep;
    uint32 met_offset;
    uint32 next_hop_ptr;
};
typedef struct sys_nh_param_dsmet_s sys_nh_param_dsmet_t;

struct sys_nh_update_oam_info_s
{
    uint8    is_protection_path;
    uint8    lock_en;
	uint8    lm_en;   /*if lm_en,indicate oam_mep_index is MEP identify */
    uint8    dsma_en; /*update dsma to nexthop info*/
	uint16   oam_mep_index;
    uint16   ma_idx;  /*valid when dsma_en is set to 1*/
    uint32   mpls_label;
    uint8    update_type; /*0:lm&lock, 1:dsma, 2:mep index*/
    uint8    mep_type;    /*0:bfd, 1:ethoam*/
    uint32   mep_index;
};
typedef struct sys_nh_update_oam_info_s sys_nh_update_oam_info_t;

struct sys_nh_repli_offset_node_s
{
    ctc_slistnode_t head;
    uint32 start_offset;
    uint32 bitmap;
    uint32 dest_gport;
    uint32 group_id;
};
typedef struct sys_nh_repli_offset_node_s sys_nh_repli_offset_node_t;

struct sys_nh_info_misc_s
{
    sys_nh_info_com_hdr_t hdr;
    uint32 dsl2edit_offset;
	uint32 dsl3edit_offset;
    uint32 next_l3edit_offset;

    uint32 gport;
    uint8  is_swap_mac;
    uint8  is_reflective;

	uint8  misc_nh_type;
	uint8  l3edit_entry_type;
	uint8  l2edit_entry_type;
	uint8  vlan_profile_id;

    uint16 truncated_len;
    uint16 l3ifid;
	void* data;
    int32 (* updateAd) (uint8 lchip,void* ipuc_node, void* dsnh_info);
};
typedef struct sys_nh_info_misc_s sys_nh_info_misc_t;

struct sys_nh_info_mcast_s
{
    sys_nh_info_com_hdr_t hdr;
    uint16 basic_met_offset;
    uint16 physical_replication_num;
    uint8  mcast_flag;              /*refer to sys_nh_info_mcast_flag_t*/
    uint8  mirror_ref_cnt;
    uint8  first_phy_if_ucastid;    /* the first phyif ucast id in ipmc*/
    uint8  first_phy_if_is_linkagg; /* the first phyif ucast id in ipmc*/

    uint16 profile_ref_cnt;
    uint32 profile_met_offset; /*0-stacking; 1-mcast profile */
    uint32 profile_nh_id;      /* mcast profile nhid or stacking profile id*/
    ctc_list_pointer_t   p_mem_list; /*sys_nh_mcast_meminfo_t*/
};
typedef struct sys_nh_info_mcast_s sys_nh_info_mcast_t;
/*Common Functions*/
extern int32
sys_usw_nh_api_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg);
extern int32
sys_usw_nh_get_dsfwd_offset(uint8 lchip, uint32 nhid, uint32* p_offset, uint8 in_cb);
extern int32
sys_usw_nh_get_emcp_if_l2edit_offset(uint8 lchip, uint32* p_l2edit_offset);
/*Ucast bridge functions*/
extern int32
sys_usw_brguc_nh_create(uint8 lchip, uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type);
extern int32
sys_usw_brguc_nh_delete(uint8 lchip, uint32 gport);
extern int32
sys_usw_l2_get_ucast_nh(uint8 lchip, uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type, uint32* nhid);
extern int32
sys_usw_nh_get_mcast_nh(uint8 lchip, uint32 group_id, uint32* nhid);
extern int32
sys_usw_nh_update_dot1ae(uint8 lchip, void* param);
extern int32
_sys_usw_nh_get_dsnh_offset_by_nhid(uint8 lchip, uint32 nhid,  uint32 *p_dsnh_offset, uint8 * p_use_dsnh8w);
extern int32
sys_usw_nh_get_l3ifid(uint8 lchip, uint32 nhid, uint16* p_l3ifid);
extern int32
sys_usw_nh_get_port(uint8 lchip, uint32 nhid, uint8* aps_brg_en, uint32* gport);
extern bool
sys_usw_nh_is_cpu_nhid(uint8 lchip, uint32 nhid);
/*Egress Vlan translation*/
extern int32
sys_usw_egress_vlan_edit_nh_create(uint8 lchip, uint32 nhid, uint32 gport,
                                         ctc_vlan_egress_edit_info_t* p_vlan_info,
                                         uint32 dsnh_offset,
                                         ctc_vlan_edit_nh_param_t* p_nh_param);
extern int32
sys_usw_egress_vlan_edit_nh_delete(uint8 lchip, uint32 nhid);

/*APS Egress Vlan translation*/
extern int32
sys_usw_aps_egress_vlan_edit_nh_create(uint8 lchip, uint32 nhid,
                                             uint32 dsnh_offset, uint16 aps_bridge_id,
                                             ctc_vlan_egress_edit_info_t* p_vlan_info_working_path,
                                             ctc_vlan_egress_edit_info_t* p_vlan_info_protection_path);
extern int32
sys_usw_aps_egress_vlan_edit_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_mcast_nh_create(uint8 lchip, uint32 groupid,  sys_nh_param_mcast_group_t* p_nh_mcast_group);
extern int32
sys_usw_mcast_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_mcast_nh_update(uint8 lchip, sys_nh_param_mcast_group_t* p_nh_mcast_group);
extern int32
sys_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param);
extern int32
sys_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id);
extern int32
sys_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param);
extern int32
sys_usw_ipuc_nh_create(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_nh_param);
extern int32
sys_usw_ipuc_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_ipuc_nh_update(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_ipuc_param, sys_nh_entry_change_type_t update_type);
extern int32
sys_usw_nh_bind_dsfwd_cb(uint8 lchip, uint32 nhid, sys_nh_update_dsnh_param_t* p_update_dsnh);
extern int32
sys_usw_nh_get_aps_working_path(uint8 lchip, uint32 nhid, uint32* gport, uint32* nexthop_ptr, bool* p_protect_en);
extern int32
sys_usw_nh_get_nhinfo(uint8 lchip, uint32 nhid, sys_nh_info_dsnh_t* p_nhinfo, uint8 in_cb);
extern int32
sys_usw_mpls_nh_create_tunnel_label(uint8 lchip, uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_tunnel_param);
extern int32
sys_usw_mpls_nh_remove_tunnel_label(uint8 lchip, uint16 tunnel_id);
extern int32
sys_usw_mpls_nh_update_tunnel_label(uint8 lchip, uint16 tunnel_id, ctc_mpls_nexthop_tunnel_param_t* p_tunnel_param);
extern int32
sys_usw_mpls_nh_swap_tunnel(uint8 lchip, uint16 old_tunnel_id,uint16 new_tunnel_id);
extern int32
sys_usw_mpls_nh_create(uint8 lchip, uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param);
extern int32
sys_usw_mpls_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_mpls_nh_update(uint8 lchip, uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param, sys_nh_entry_change_type_t change_type);
extern int32
sys_usw_nh_update_oam_en(uint8 lchip, uint32 nhid,sys_nh_update_oam_info_t *p_oam_info);
extern int32
sys_usw_iloop_nh_create(uint8 lchip, uint32 nhid, ctc_loopback_nexthop_param_t* p_nh_param);
extern int32
sys_usw_iloop_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_rspan_nh_create(uint8 lchip, uint32* nhid, uint32 dsnh_offset, ctc_rspan_nexthop_param_t* p_nh_param);
extern int32
sys_usw_rspan_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_nh_dump_mcast(uint8 lchip, uint32 nhid, bool detail);
extern int32
sys_usw_nh_dump(uint8 lchip, uint32 nhid, bool detail);
extern int32
sys_usw_ecmp_nh_create(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* pdata);
extern int32
sys_usw_ecmp_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_ecmp_nh_update(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* pdata);
extern int32
sys_usw_nh_set_max_ecmp(uint8 lchip, uint16 max_ecmp);
extern int32
sys_usw_ip_tunnel_nh_create(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param);
extern int32
sys_usw_ip_tunnel_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_ip_tunnel_nh_update(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param);
extern int32
sys_usw_wlan_tunnel_nh_create(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param);
extern int32
sys_usw_wlan_tunnel_nh_delete(uint8 lchip, uint32 nhid);
extern int32
sys_usw_wlan_tunnel_nh_update(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param);
extern int32
sys_usw_nh_wlan_create_reserve_nh(uint8 lchip, sys_nh_entry_table_type_t nh_type, uint32* p_data);
extern int32
sys_usw_nh_add_misc(uint8 lchip, uint32* nhid, ctc_misc_nh_param_t* p_nh_param, bool is_internal_nh);
extern int32
sys_usw_nh_remove_misc(uint8 lchip, uint32 nhid);
extern int32
sys_usw_nh_trill_create(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param);
extern int32
sys_usw_nh_trill_remove(uint8 lchip, uint32 nhid);
extern int32
sys_usw_nh_trill_update(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param);
extern int32
sys_usw_nh_api_deinit(uint8 lchip);
extern int32
sys_usw_nh_unbind_dsfwd_cb(uint8 lchip, uint32 nhid);
extern int32
sys_usw_nh_get_nhinfo_by_nhid(uint8 lchip, uint32 nhid, sys_nh_info_com_t** pp_nhinfo);
extern int32
sys_usw_nh_get_resolved_offset(uint8 lchip, sys_usw_nh_res_offset_type_t type, uint32* p_offset);
extern int32
sys_usw_nh_is_logic_repli_en(uint8 lchip, uint32 nhid, uint8 *is_repli_en);
extern int32
sys_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id);
extern int32
sys_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param);
extern int32
sys_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param);
extern int32
sys_usw_nh_del_ipmc_dsnh_offset(uint8 lchip, uint16 l3ifid);
extern int32
sys_usw_nh_get_current_ecmp_group_num(uint8 lchip, uint16* cur_group_num);
extern int32
sys_usw_nh_alloc_ecmp_offset(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32* p_offset);
extern int32
sys_usw_nh_free_ecmp_offset(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32 offset);
extern int32
sys_usw_nh_get_max_ecmp_group_num(uint8 lchip, uint16* max_group_num);
extern int32
sys_usw_nh_get_max_ecmp(uint8 lchip, uint16* max_ecmp);
extern int32
sys_usw_nh_alloc(uint8 lchip, sys_nh_entry_table_type_t entry_type,uint32 entry_num, uint32* p_offset);
extern int32
sys_usw_nh_free(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32 offset);
extern int32
sys_usw_nh_add_dsfwd(uint8 lchip, sys_nh_param_dsfwd_t* p_dsfwd_param);
extern uint8
sys_usw_nh_get_edit_mode(uint8 lchip);
extern int32
sys_usw_nh_add_dsmet(uint8 lchip, sys_nh_param_dsmet_t* p_met_param);
extern int32
sys_usw_nh_set_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set);
extern int32
sys_usw_nh_check_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,bool should_not_inuse);
extern int32
sys_usw_nh_traverse_mcast_db(uint8 lchip, vector_traversal_fn2 fn, void* data);
extern int32
sys_usw_nh_update_dsfwd(uint8 lchip, uint32 *offset, uint32 dest_map, uint32 dsnh_offset, uint8 dsnh_8w,
                               uint8 del, uint8 critical_packet);
extern int32
sys_usw_nh_get_fatal_excp_dsnh_offset(uint8 lchip, uint32* p_offset);
extern int32
sys_usw_nh_get_max_external_nhid(uint8 lchip, uint32* nhid);
extern int32
sys_usw_nh_get_mirror_info_by_nhid(uint8 lchip, uint32 nhid, uint32* dsnh_offset, uint32* gport, bool enable);
extern int32
sys_usw_nh_get_reflective_dsfwd_offset(uint8 lchip, uint32* p_dsfwd_offset);
extern int32
sys_usw_nh_map_destmap_to_port(uint8 lchip, uint32 destmap, uint32 *gport);
extern int32
sys_usw_nh_set_bpe_en(uint8 lchip, bool enable);
extern int32
sys_usw_nh_get_mcast_member(uint8 lchip, uint32 nhid, ctc_nh_info_t* p_nh_info);
extern int32
sys_usw_nh_get_dsnh_offset_by_nhid(uint8 lchip, uint32 nhid,  uint32 *p_dsnh_offset, uint8 * p_use_dsnh8w);
extern int32
sys_usw_nh_get_mcast_member_info(uint8 lchip, sys_nh_info_mcast_t* p_mcast_db, ctc_nh_info_t* p_nh_info);
extern int32
sys_usw_nh_vxlan_vni_init(uint8 lchip);
extern int32
sys_usw_nh_alloc_ecmp_offset_from_position(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32 offset);
extern int32
sys_usw_nh_alloc_from_position(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32 offset);
extern bool
sys_usw_nh_is_ipmc_logic_rep_enable(uint8 lchip);
extern int32
sys_usw_nh_set_ipmc_logic_replication(uint8 lchip, uint8 enable);
extern uint8
sys_usw_nh_get_met_mode(uint8 lchip);
extern int32
sys_usw_nh_add_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit);
extern int32
sys_usw_nh_remove_udf_profile(uint8 lchip, uint8 profile_id);
extern int32
sys_usw_misc_nh_update(uint8 lchip, uint32 nhid, ctc_misc_nh_param_t* p_nh_param,
                             sys_nh_entry_change_type_t update_type);
extern int32
sys_usw_nh_traverse_mpls_nexthop(uint8 lchip, uint16 tunnel_id, nh_traversal_fn cb, void* user_data);
extern int32
sys_usw_nh_set_efd_redirect_nh(uint8 lchip, uint32 nh_id);
extern int32
sys_usw_nh_get_efd_redirect_nh(uint8 lchip, uint32* p_nh_id);
extern int32
sys_usw_nh_traverse_mcast(uint8 lchip, ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data);
extern int32
sys_usw_nh_set_vpws_vpnid(uint8 lchip, uint32 nhid, uint32 vpn_id);
extern int32
sys_usw_nexthop_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param);
/*#endif*/
#ifdef __cplusplus
}
#endif

#endif /*_SYS_NEXTHOP_H_*/

