/**
   @file sys_usw_acl.h

   @date 2015-11-01

   @version v3.0

 */
#ifndef _SYS_USW_ACL_H
#define _SYS_USW_ACL_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
*
* Header Files
*
***************************************************************/
#include "ctc_const.h"

#include "ctc_acl.h"
#include "ctc_field.h"
#include "sys_usw_fpa.h"
#include "sys_usw_common.h"
#include "sys_usw_acl_api.h"
#include "sys_usw_ofb.h"

/***************************************************************
*
*  Defines and Macros
*
***************************************************************/

enum sys_acl_action_type
{
    SYS_ACL_ACTION_TCAM_INGRESS,
    SYS_ACL_ACTION_TCAM_EGRESS,
    SYS_ACL_ACTION_HASH_INGRESS,
    SYS_ACL_ACTION_NUM
};

typedef struct
{
    uint8 stag_op;
    uint8 svid_sl;
    uint8 scos_sl;
    uint8 scfi_sl;

    uint8 ctag_op;
    uint8 cvid_sl;
    uint8 ccos_sl ;
    uint8 ccfi_sl ;
    uint32 tpid_index :2;
    uint32 rsv1 :30;

    uint16 profile_id; /* excluded in hash calculation */
    uint8 rsv[2];
}sys_acl_vlan_edit_t;

typedef struct
{
    uint8  flag;
    uint8  range_id[ACL_RANGE_TYPE_NUM];

    uint16 range_bitmap;
    uint16 range_bitmap_mask;
}sys_acl_range_info_t;

typedef struct
{
    ds_t key;
    ds_t mask;
    ds_t action;
}sys_acl_buffer_t;

typedef struct
{
    ctc_slistnode_t head;                     /* keep head top!! */
    ctc_fpa_entry_t fpae;                     /* keep fpae at second place!! */

    sys_acl_group_t * group;                  /* pointer to group node*/

    uint8 key_type;                           /* Refer to ctc_acl_key_type_t */
    uint8 mode;
    uint32 action_bmp[(CTC_ACL_FIELD_ACTION_NUM-1)/32+1];  /*for show action; such as: action_bmp bit0 reprensents CTC_ACL_FIELD_ACTION_CP_TO_CPU */
    uint32 key_bmp[(CTC_FIELD_KEY_NUM-1)/32+1];            /*for show key;    such as: key_bmp    bit0 reprensents CTC_FIELD_KEY_L2_TYPE */
    uint16 minvid;                                         /*for show key;    CTC_FIELD_KEY_SVLAN_RANGE CTC_FIELD_KEY_CVLAN_RANGE */
    uint16 key_reason_id;                                  /*for show key;    CTC_FIELD_KEY_CPU_REASON_ID */
    uint32 key_dst_nhid;                                   /*for show key;    CTC_FIELD_KEY_DST_NHID */

    uint32 hash_valid           : 1,          /*generated hash index*/
	       key_exist            : 1,          /*key exist*/
	       key_conflict         : 1,          /*hash key conflict*/
	       u1_type              : 3,
	       u2_type              : 3,
	       u3_type              : 3,
	       u4_type              : 3,
	       u5_type              : 3,
	       wlan_bmp             : 3,    /*pe->wlan_bmp : 0,CTC_FIELD_KEY_WLAN_RADIO_MAC; 1,CTC_FIELD_KEY_WLAN_RADIO_ID; 2,CTC_FIELD_KEY_WLAN_CTL_PKT */
           macl3basic_key_cvlan_mode:2, /*0,none;1:ip_da;  2:Aware Tunnel Info:CTC_FIELD_KEY_AWARE_TUNNEL_INFO*/
           macl3basic_key_macda_mode:2, /*0,none;1:mac_da; 2:Aware Tunnel Info:CTC_FIELD_KEY_AWARE_TUNNEL_INFO*/
           igmp_snooping        : 1,
           hash_sel_tcp_flags   : 2,
	       rsv1                  : 4;
	uint32 l2_type              :4,
	       l3_type              :4,
	       l4_type              :4,
	       l4_user_type         :4,
	       key_port_mode        :2,     /*0,none;1,port; 2;metata_or_cid*/
	       fwd_key_nsh_mode     :2,     /*0,none;1:nsh;2:udf*/
	       key_l4_port_mode     :2,     /*0,none;1:l4port;2:vxlan;3:gre*/
	       mac_key_vlan_mode    :2,     /*0,none;1:svlan;2:cvlan*/
	       ip_dscp_mode         :2,     /*0,none;1:dscp;2:ecn/PRECEDENCE*/
	       key_mergedata_mode   :2,     /*0,none;1:Wlan Info;2:MergeData*/
	       copp_ext_key_ip_mode :2,     /*0,none;1:ip;2:udf*/
           rsv2                  :2;
 	uint32 u1_bitmap;
	uint16 u3_bitmap;
	uint16  u2_bitmap;
	uint8  u4_bitmap;
	uint16  u5_bitmap;

    uint8 vlan_edit_valid;
    sys_acl_vlan_edit_t* vlan_edit;           /*for tcam, add to vlan_edit_spool*/
    uint16 ether_type;                        /*for tcam, add to cethertype_spool*/
    uint8  ether_type_index;                  /*warmboot restore entry, restore cethertype_spool*/

    sys_acl_range_info_t rg_info;

    uint32 copp_rate;                         /*used for copy entry*/
    uint8  copp_ptr;                          /*used for remvoe entry*/

    uint8  hash_field_sel_id;                 /* Hash select ID,if set to 0 ,indicate all field are vaild */
    uint32 hash_sel_bmp[4];                   /* Hash select bitmap */
    uint32  ad_index;                         /*for hash and pbr, tcam use fpae.offset_a*/
    uint32* hash_action;                      /*when update action*/

    uint16  action_flag;                      /*used for show. */
	uint16  cpu_reason_id;

    /*used for show action field*/
    uint32 stats_id;                          /*used for show action field: CTC_ACL_FIELD_ACTION_STATS*/
    uint32 policer_id;                        /*used for show action field: per entry only have one of the three;
                                                CTC_ACL_FIELD_ACTION_COPP; CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER; CTC_ACL_FIELD_ACTION_MACRO_FLOW_POLICER */
    uint8  cos_index;                         /*used for show action field: when policer is cos hbwp, point which policerptr to use*/
    uint32 nexthop_id;                        /*also used for show action field: CTC_ACL_FIELD_ACTION_REDIRECT & CTC_ACL_FIELD_ACTION_REDIRECT_PORT*/
    uint8  action_type;                       /*for show action; refer to: sys_acl_action_type */
    uint32 udf_id;
    uint32 limit_rate;                        /*for show ipfix action*/
    sys_acl_buffer_t* buffer;
}sys_acl_entry_t;


typedef struct
{

    uint32 entry_num;
    ctc_fpa_block_t fpab;
}sys_acl_block_t;

/*pkt-len-range and l4-port-range share the resource*/
#define SYS_ACL_FIELD_RANGE_NUM         12

#define SYS_ACL_ENTRY_OP_FLAG_ADD       1
#define SYS_ACL_ENTRY_OP_FLAG_DELETE    2

#define SYS_ACL_ACTION_FLAG_PERMIT      (1 << 0)
#define SYS_ACL_ACTION_FLAG_DISCARD     (1 << 1)
#define SYS_ACL_ACTION_FLAG_REDIRECT    (1 << 2)
#define SYS_ACL_ACTION_FLAG_LOG         (1 << 3)
#define SYS_ACL_ACTION_FLAG_BIND_NH         (1 << 4)
#define SYS_ACL_ACTION_FLAG_UNBIND_NH       (1 << 5)

#define SYS_ACL_GROUP_ID_WLAN_CRYPT_ERROR 0xFFFFFF01
#define SYS_ACL_GROUP_ID_DOT1AE_CRYPT_ERROR 0xFFFFFF02

enum sys_acl_hash_gport_type
{
    ACL_HASH_GPORT_TYPE_NONE       = 0,
    ACL_HASH_GPORT_TYPE_GPORT      = 0x1,
    ACL_HASH_GPORT_TYPE_LOGIC_PORT = 0x2,
    ACL_HASH_GPORT_TYPE_METADATA   = 0x3,
    ACL_HASH_GPORT_TYPE_NUM
};


#define SYS_ACL_USW_GROUP_HASH_SIZE            1024
#define SYS_ACL_USW_GROUP_HASH_BLOCK_SIZE      32
#define SYS_ACL_USW_ENTRY_HASH_SIZE            32768
#define SYS_ACL_USW_ENTRY_HASH_BLOCK_SIZE      1024
#define SYS_ACL_USW_HASH_ACTION_BLOCK_SIZE     4096


#define ACL_HASH_AD_INDEX_OFFSET 31

#define ACL_BITMAP_STATUS_64    0 /* if only ipv6 exist*/
#define ACL_BITMAP_STATUS_16    1 /* if non-ipv6 exist*/

/* used by show only */
#define SYS_ACL_SHOW_IN_HEX   0xFFFFF

#define SYS_ACL_MAP_DRV_AD_INDEX(index, step)  ((1 == (step)) ? (index) : ((index) / 2))
#define SYS_ACL_MAP_DRV_KEY_INDEX(index, step)  ((index) / (step))

#define SYS_ACL_METADATA_CHECK(group_type)                      \
    if( (CTC_ACL_GROUP_TYPE_PORT_BITMAP == (group_type))        \
      || (CTC_ACL_GROUP_TYPE_PORT == (group_type))              \
      || (CTC_ACL_GROUP_TYPE_SERVICE_ACL == (group_type)) )      \
    {                                                           \
        SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Flag collision \n"); \
        return CTC_E_INVALID_CONFIG; \
                        \
    }

#define SYS_ACL_VLAN_ID_CHECK(vlan_id)           \
    {                                            \
        if ((vlan_id) > CTC_MAX_VLAN_ID) {       \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Vlan] Invalid vlan id \n");\
           return CTC_E_BADID;\
 } \
    }

#define SYS_ACL_VLAN_ACTION_RESERVE_NUM    63

#define SYS_USW_ACL_GPORT_IS_LOCAL(lchip,gport) \
     (CTC_IS_LINKAGG_PORT(gport) || !SYS_GCHIP_IS_REMOTE(lchip,SYS_MAP_CTC_GPORT_TO_GCHIP(gport)))

#define SYS_USW_ACL_HASH_VALID_CHECK(val)             \
    {                                                  \
        if (1 == val )                                 \
        {                                              \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");\
            return CTC_E_EXIST;\
        }                                              \
    }

#define SYS_USW_ACL_SET_BMP(bmp, bit, is_set) \
    do{\
        if(is_set)\
            CTC_BMP_SET(bmp, bit);\
        else\
            CTC_BMP_UNSET(bmp, bit);\
    }while(0)

#define SYS_USW_ACL_SET_BIT(flag, bit, is_set) \
    do{\
        if(is_set)\
            CTC_BIT_SET(flag, bit);\
        else\
            CTC_BIT_UNSET(flag, bit);\
    }while(0)

/* used for wb */
struct sys_acl_field_range_wb_data_s
{
    uint16  range_id_bitmap;
    uint16  range_id_ref[SYS_ACL_FIELD_RANGE_NUM];
    uint8   range_id_valid_cnt;
};
typedef struct sys_acl_field_range_wb_data_s sys_acl_field_range_wb_data_t;

/*  field range
*   l4_src_port_range and l4_dst_port_range share range_id from 0 ~ (SYS_ACL_L4_PORT_RANGE_NUM-1)
*   pkt_len_range range_id from SYS_ACL_L4_PORT_RANGE_NUM ~ (SYS_ACL_FIELD_RANGE_NUM - 1)
*/
struct sys_acl_field_range_s
{
    uint8  free_num;

    struct
    {
        uint8  range_type;
        uint16 ref;

        uint32 min;
        uint32 max;
    }range[SYS_ACL_FIELD_RANGE_NUM];
};
typedef struct sys_acl_field_range_s sys_acl_field_range_t;

/*cid hash action, used for spool*/
struct sys_acl_cid_action_s
{
    DsCategoryIdPair_m ds_cid_action;
    uint8              is_left;
    uint8              rsv;

    uint16              ad_index;

};
typedef struct sys_acl_cid_action_s sys_acl_cid_action_t;

struct sys_acl_register_s
{
    uint8  ingress_service_acl_en;
    uint8  egress_service_acl_en;
    uint8  brg_pkt_vlan_cid_high_prio;
    uint8  xgpon_gem_port_en;
};
typedef struct sys_acl_register_s   sys_acl_register_t;

enum sys_acl_global_property_e
{
    SYS_ACL_IGS_SERVICE_ACL_EN,
    SYS_ACL_EGS_SERVICE_ACL_EN,
    SYS_ACL_IGS_CID_OVER_GPORT_EN,
    SYS_ACL_EGS_CID_OVER_GPORT_EN,
    SYS_ACL_BRG_PKT_VLAN_CID_HIGH_PRIO,
    SYS_ACL_XGPON_GEM_PORT_EN
};
typedef enum sys_acl_global_property_e sys_acl_global_property_t;

struct sys_hash_sel_filed_union_s
{
    uint8   l4_port_field; 	     /*l4_port_field:  0:none; 1:gre/nvgre: 2:vxlan; 3:l4 port; 4:icmp; 5:igmp */
    uint8   l4_port_field_bmp;   /*for 1:gre/nvgre; bit0:CTC_FIELD_KEY_GRE_KEY;     bit1:CTC_FIELD_KEY_NVGRE_KEY;   (can't exist at the same time)
                                   for 2:vxlan;     CTC_FIELD_KEY_VN_ID     (only one field, no need l4_port_field_bmp)
                                   for 3:l4 port;   bit2:CTC_FIELD_KEY_L4_DST_PORT; bit3:CTC_FIELD_KEY_L4_SRC_PORT; (can exist at the same time)
                                   for 4:icmp;      bit4:CTC_FIELD_KEY_ICMP_CODE;   bit5:CTC_FIELD_KEY_ICMP_TYPE;   (can exist at the same time)
                                   for 5:igmp;      CTC_FIELD_KEY_IGMP_TYPE (only one field, no need l4_port_field_bmp)*/
	uint8   key_port_type;       /*0:none; 1:key port;   2:metadata*/
	uint8   l2_l3_key_ul3_type;  /*0:none; 1:ip; 2:mpls; 3:Unknown_etherType*/

	   /*asic u1Type == 0 表示ipHeaderError/ipOptions/fragInfo/ttl/tcpFlags
        u1Type == 1 表示ipHeaderError/ipOptions/fragInfo/vrfId/isRouteMac
        u1Type == 2 表示ipPktLenRangeId/l4PortRangeBitmap/tcpFlags
        u1Type == 3 ipHeaderError/ipOptions/fragInfo/ttl/layer3HeaderProtocol */
	uint8   l3_key_u1_type;   /*sw :asic u1Type+1 , 0:none; 1:g1; 2:g2; 3:g3; 4:g4. sw not support cfg l3_key_u1_type;default to 1*/
	uint8   ipv6_key_ip_mode;   /*0:none;1;full ip;2:compress ip,sw not support cfg ipv6_key_ip_mode.default to 1*/

};
typedef struct sys_hash_sel_filed_union_s   sys_hash_sel_field_union_t;

typedef int32 (* SYS_CB_ACL_BUILD_KEY_FUNC_T)(uint8 lchip, ctc_field_key_t* pKey, sys_acl_entry_t* pe,uint8 is_add);
typedef int32 (* SYS_CB_ACL_GET_AD_FUNC_T)(uint8 lchip, ctc_acl_field_action_t* p_action, sys_acl_entry_t* pe);

struct sys_acl_league_s
{
    uint8               block_id;
    uint16              lkup_level_start[ACL_IGS_BLOCK_MAX_NUM]; /*each merged lookup level start offset*/
    uint16              lkup_level_count[ACL_IGS_BLOCK_MAX_NUM]; /*each merged lookup level start offset*/
    uint16              lkup_level_bitmap;
    uint8               merged_to;                                /*which priority this lookup level merged to*/
};
typedef struct sys_acl_league_s   sys_acl_league_t;

struct sys_acl_master_s
{
    ctc_hash_t          * group;            /* Hash table save group by gid.*/
    ctc_fpa_t           * fpa;              /* Fast priority arrangement. no array. */
    ctc_hash_t          * entry;            /* Hash table save hash|tcam entry by eid */
    ctc_spool_t         * ad_spool;         /* Share pool save hash action  */
    ctc_spool_t         * vlan_edit_spool;  /* Share pool save vlan edit  */
    ctc_spool_t         * cid_spool;        /* Share pool save cid hash action  */
    sys_acl_block_t     block[ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM];

    uint32              max_entry_priority;

    uint16              hash_sel_profile_count[SYS_ACL_HASH_KEY_TYPE_NUM][SYS_ACL_HASH_SEL_PROFILE_MAX];
    sys_hash_sel_field_union_t              hash_sel_key_union_filed[SYS_ACL_HASH_KEY_TYPE_NUM][SYS_ACL_HASH_SEL_PROFILE_MAX];

    uint8               igs_block_num;      /* number of ingress logic lkup */
    uint8               egs_block_num;      /* number of egress logic lkup */

    sys_acl_league_t    league[ACL_IGS_BLOCK_MAX_NUM + ACL_EGS_BLOCK_MAX_NUM];

    sys_acl_field_range_t field_range;

    uint8               opf_type_vlan_edit;
    uint8               opf_type_ad;    /*mark: in GG, this is managed in ftm*/
    uint8               ofb_type_cid;   /*category id tcam*/
    uint8               opf_type_cid_ad;
    uint8               opf_type_udf;   /*range:0~15*/
    uint32              key_count[ACL_ALLOC_COUNT_NUM];
    sal_mutex_t*        acl_mutex;   /**< mutex for acl */

    sys_acl_udf_entry_t   udf_entry[SYS_ACL_MAX_UDF_ENTRY_NUM];
	SYS_CB_ACL_BUILD_KEY_FUNC_T build_key_func[CTC_ACL_KEY_NUM];
    SYS_CB_ACL_GET_AD_FUNC_T    get_ad_func[SYS_ACL_ACTION_NUM];

    sys_acl_register_t acl_register;

    uint8               wlan_crypt_error_to_cpu;
    uint8               dot1ae_crypt_error_to_cpu;
    uint8               sort_mode;   /*0: entry will be sorted by FPA, 1: entry sorted by user, the entry priority is entry index*/
};
typedef struct sys_acl_master_s   sys_acl_master_t;

#define SYS_ACL_DBG_OUT(level, FMT, ...) \
    CTC_DEBUG_OUT(acl, acl, ACL_SYS, level, FMT, ## __VA_ARGS__)

#define SYS_ACL_DBG_FUNC() \
    SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__)

#define SYS_ACL_INIT_CHECK()         \
    {                                \
        SYS_LCHIP_CHECK_ACTIVE(lchip);          \
        if (!p_usw_acl_master[lchip]) {           \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n"); \
			return CTC_E_NOT_INIT; \
 } \
    }

#define SYS_ACL_CREATE_LOCK(lchip)                   \
    do                                            \
    {                                             \
        sal_mutex_create(&p_usw_acl_master[lchip]->acl_mutex); \
        if (NULL == p_usw_acl_master[lchip]->acl_mutex)  \
        { \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); \
        } \
    } while (0)

#define SYS_USW_ACL_LOCK(lchip) \
    if (p_usw_acl_master[lchip]->acl_mutex) sal_mutex_lock(p_usw_acl_master[lchip]->acl_mutex)

#define SYS_USW_ACL_UNLOCK(lchip) \
    if (p_usw_acl_master[lchip]->acl_mutex) sal_mutex_unlock(p_usw_acl_master[lchip]->acl_mutex)

#define SYS_USW_ACL_RETURN_UNLOCK(ret) \
do \
{ \
    sal_mutex_unlock(p_usw_acl_master[lchip]->acl_mutex); \
    return (ret); \
} while (0)

#define SYS_USW_ACL_ERROR_RETURN_UNLOCK(op) CTC_ERROR_RETURN_WITH_UNLOCK(op, p_usw_acl_master[lchip]->acl_mutex)

#define SYS_USW_ACL_CHECK_GROUP_UNEXIST_UNLOCK(pg)     \
    {                                       \
        if (!pg)                            \
        {                                   \
            sal_mutex_unlock(p_usw_acl_master[lchip]->acl_mutex); \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n"); \
			return CTC_E_NOT_EXIST;\
 \
        }                                   \
    }

/* input rsv group id , return failed */
#define SYS_ACL_IS_RSV_GROUP(gid) \
    ((gid) > CTC_ACL_GROUP_ID_NORMAL && (gid) < CTC_ACL_GROUP_ID_MAX)

#define SYS_ACL_CHECK_RSV_GROUP_ID(gid)      \
    {                                        \
        if SYS_ACL_IS_RSV_GROUP(gid)         \
        {                                    \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group id reserved \n");\
			return CTC_E_BADID;\
 \
        }                                    \
    }

#define SYS_ACL_CHECK_GROUP_ID(gid)        \
    {                                      \
        if (gid >= CTC_ACL_GROUP_ID_MAX) { \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Invalid group id \n");\
			return CTC_E_BADID;\
 }   \
    }

#define SYS_ACL_CHECK_GROUP_TYPE(type)        \
    {                                         \
        if (type >= CTC_ACL_GROUP_TYPE_MAX) { \
            return CTC_E_ACL_GROUP_TYPE; }    \
    }

#define SYS_ACL_CHECK_GROUP_PRIO(dir, priority)                                      \
    {                                                                                \
        if (((CTC_INGRESS == dir) && (priority >= p_usw_acl_master[lchip]->igs_block_num)) || \
            ((CTC_EGRESS == dir) && (priority >= p_usw_acl_master[lchip]->egs_block_num)))    \
        {                                                                            \
            return CTC_E_INVALID_PARAM;                                         \
        }                                                                            \
    }

#define SYS_ACL_CHECK_ENTRY_PRIO(priority)               \
    {                                                    \
        if (priority > p_usw_acl_master[lchip]->max_entry_priority) { \
            return CTC_E_INVALID_PARAM; }           \
    }

#define SYS_ACL_SERVICE_ID_CHECK(sid)    SYS_LOGIC_PORT_CHECK(sid)

#define SYS_ACL_CHECK_GROUP_UNEXIST(pg)     \
    {                                       \
        if (!pg)                            \
        {                                   \
            SYS_ACL_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Acl] Group not exist \n");\
			return CTC_E_NOT_EXIST;\
 \
        }                                   \
    }


#define ACL_ENTRY_IS_TCAM(type)                                            \
    ((CTC_ACL_KEY_HASH_MAC != type) && (CTC_ACL_KEY_HASH_IPV4 != type) &&  \
     (CTC_ACL_KEY_HASH_MPLS != type) && (CTC_ACL_KEY_HASH_IPV6 != type) && \
     (CTC_ACL_KEY_HASH_L2_L3 != type))

#define ACL_SET_HW_MAC(d, s)    SYS_USW_SET_HW_MAC(d, s)
#define ACL_SET_HW_IP6(d, s)    SYS_USW_REVERT_IP6(d, s)
#define ACL_SET_HW_SATPDU_BYTE(d, s)    SYS_USW_REVERT_SATPDU_BYTE(d, s)
#define ACL_SET_HW_SCI_BYTE(d, s) SYS_USW_REVERT_SCI_BYTE(d, s)
#define ACL_SET_HW_UDF(d, s)    SYS_USW_REVERT_UDF(d, s)
#define ACL_SET_HW_UDF_72(d, s)    SYS_USW_REVERT_UDF_72(d, s)


#define ACL_GET_TABLE_ENTYR_NUM(lchip, t, n)    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, t, n))

struct drv_acl_group_info_s
{
    uint8  dir;

    uint32 class_id_data;
    uint32 class_id_mask;

    uint16 gport;
    uint16 gport_mask;

    uint8  gport_type;
    uint8  gport_type_mask;

    uint8  bitmap_base;
    uint8  bitmap_base_mask;

    uint8  bitmap;
    uint8  bitmap_mask;

    uint32 bitmap_64[2];
    uint32 bitmap_64_mask[2];
};
typedef struct drv_acl_group_info_s   drv_acl_group_info_t;

struct drv_acl_port_info_s
{
    uint8  dir;

    uint32 class_id_data;
    uint32 class_id_mask;

    uint16 gport;
    uint16 gport_mask;

    uint8  gport_type;
    uint8  gport_type_mask;

    uint8  bitmap_base;
    uint8  bitmap_base_mask;

    uint8  bitmap;
    uint8  bitmap_mask;

    uint32 bitmap_64[2];
    uint32 bitmap_64_mask[2];
};
typedef struct drv_acl_group_info_s  drv_acl_port_info_t;

typedef struct
{
    uint32             count;
    uint8              detail;
	uint8              entry_type; /*0:tcam;1;hash;2 :both */
    uint8              key_type;
    uint8              show_type;
    uint8              rsv[3];
    ctc_field_key_t*   p_grep;
    uint8              grep_cnt;
} _acl_cb_para_t;
typedef struct sys_acl_traverse_data_s
{
    void* data0;
    void* data1;
    void* data2;
    int32 value0;
}sys_acl_traverse_data_t;

struct sys_usw_acl_league_node_s
{
    uint8 step;
    uint8 status;  /*has been moved*/
    uint8 old_block_id;
    uint16 old_index;
    uint32 entry_id;
    uint32 prio;
    sys_acl_buffer_t*  p_buffer;
};
typedef struct sys_usw_acl_league_node_s sys_usw_acl_league_node_t;
#define ACL_OUTER_ENTRY(pe)    ((sys_acl_entry_t *) ((uint8 *) pe - sizeof(ctc_slistnode_t)))

/*api from acl_func.c*/
extern int32
_sys_usw_acl_get_table_id(uint8 lchip, sys_acl_entry_t* pe, uint32* key_id, uint32* act_id, uint32* hw_index);
extern int32
_sys_usw_acl_build_vlan_edit(uint8 lchip, void* p_ds_edit, sys_acl_vlan_edit_t* p_vlan_edit);
extern int32
_sys_usw_acl_add_key_common_field_group_none(uint8 lchip, sys_acl_entry_t* pe );
extern int32
_sys_usw_acl_rebuild_buffer_from_hw(uint8 lchip, uint32 key_id, uint32 act_id, uint32 hw_index, sys_acl_entry_t* pe);
extern int32
_sys_usw_acl_remove_hw(uint8 lchip, sys_acl_entry_t* pe);
extern int32
_sys_usw_acl_add_hw(uint8 lchip, sys_acl_entry_t* pe, uint32* old_action);
extern int32
_sys_usw_acl_set_hash_sel_bmp(uint8 lchip, sys_acl_entry_t* pe );
extern int32
_sys_usw_acl_set_register(uint8 lchip, sys_acl_register_t* p_reg);
extern int32
_sys_usw_acl_install_vlan_edit(uint8 lchip, sys_acl_vlan_edit_t* p_vlan_edit, uint16* p_prof_idx);
extern int32
_sys_usw_acl_get_hash_index(uint8 lchip, sys_acl_entry_t* pe, uint8 is_lkup,uint8 install);
extern int32
_sys_usw_acl_add_hash_mac_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_ipv4_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_l2l3_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_ipv6_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_mpls_key_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_mac_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_ipv4_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_ipv6_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_mpls_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_add_hash_l2l3_sel_field(uint8 lchip, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_add_dsflow_field(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_get_field_action_hash_igs(uint8 lchip, ctc_acl_field_action_t* p_action, sys_acl_entry_t* pe);
extern int32
_sys_usw_acl_add_mackey160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_l3key160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_l3key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_macl3key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_macl3key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_ipv6key320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_ipv6key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_macipv6key640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_cidkey160_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_shortkey80_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_forwardkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_forwardkey640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_coppkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_coppkey640_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_add_udfkey320_field(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_operate_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field, uint8 is_add);
extern int32
_sys_usw_acl_add_dsacl_field(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_half, uint8 is_add);
extern int32
_sys_usw_acl_add_dsacl_field_wlan_error_to_cpu(uint8 lchip, ctc_acl_field_action_t* action_field, sys_acl_entry_t* pe, uint8 is_half, uint8 is_add);
extern int32
_sys_usw_acl_get_field_action_tcam_igs(uint8 lchip, ctc_acl_field_action_t* p_action, sys_acl_entry_t* pe);
extern int32
_sys_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add);
extern int32
_sys_usw_acl_set_league_table(uint8 lchip, ctc_acl_league_t* league);
extern int32
_sys_usw_acl_tcam_cid_pair_lookup(uint8 lchip, ctc_acl_cid_pair_t* cid_pair, uint32* index_o);
extern int32
_sys_usw_acl_build_cid_pair_action(uint8 lchip, void* p_buffer, ctc_acl_cid_pair_t* cid_pair);
extern int32
_sys_usw_acl_add_hash_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
_sys_usw_acl_add_tcam_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
_sys_usw_acl_remove_tcam_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
_sys_usw_acl_remove_hash_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
_sys_usw_acl_set_cid_priority(uint8 lchip, uint8 is_src_cid, uint8* p_prio_arry);
extern int32
_sys_usw_acl_get_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id, void* ds_sel);
extern int32
_sys_usw_acl_update_action(uint8 lchip, void* data, void* change_nh_param);
extern int32
_sys_usw_acl_update_dsflow_action(uint8 lchip, void* data, void* change_nh_param);
extern int32
_sys_usw_acl_vlan_tag_op_translate(uint8 lchip, uint8 op_in, uint8* o_op, uint8* o_mode);
extern int32
_sys_usw_acl_map_ethertype(uint16 ethertype, uint8* o_l3_type);

/*api from acl.c*/
extern uint8
_sys_usw_acl_get_key_size(uint8 lchip, uint8 by_key_type, sys_acl_entry_t * pe, uint8* o_step);
extern int32
_sys_usw_acl_bind_nexthop(uint8 lchip, sys_acl_entry_t* pe,uint32 nh_id,uint8 ad_type);
extern int32
_sys_usw_acl_unbind_nexthop(uint8 lchip, sys_acl_entry_t* pe);
extern int32
_sys_usw_acl_remove_field_range(uint8 lchip, uint8 range_type, sys_acl_range_info_t* p_range_info);
extern int32
_sys_usw_acl_add_field_range(uint8 lchip, uint8 range_type, uint16 min, uint16 max, sys_acl_range_info_t* p_range_info);
extern int32
_sys_usw_acl_get_compress_ipv6_addr(uint8 lchip,sys_acl_entry_t* pe,uint8 is_da, ipv6_addr_t ipv6, ipv6_addr_t ipv6_64);
extern int32
_sys_usw_acl_get_ipv6_addr_from_compress(uint8 lchip,sys_acl_entry_t* pe,uint8 is_da, ipv6_addr_t in_ipv6_64, ipv6_addr_t out_ipv6_128);
extern int32
_sys_usw_acl_get_udf_info(uint8 lchip,  uint32 udf_id, sys_acl_udf_entry_t **p_udf_entry);
extern int32
_sys_usw_acl_check_vlan_edit(uint8 lchip, ctc_acl_vlan_edit_t*   p_ctc, uint8* vlan_edit);
extern int32
_sys_usw_acl_get_entry_by_eid(uint8 lchip, uint32 eid, sys_acl_entry_t** pe);
extern int32
_sys_usw_acl_get_nodes_by_eid(uint8 lchip, uint32 eid, sys_acl_entry_t** pe, sys_acl_group_t** pg, sys_acl_block_t** pb);
extern int32
_sys_usw_acl_check_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_set_tcam_key_union(uint8 lchip, ctc_field_key_t* key_field, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_hash_traverse_cb_show_inner_gid(sys_acl_group_t* pg, _acl_cb_para_t* cb_para);
extern int32
_sys_usw_acl_hash_traverse_cb_show_outer_gid(sys_acl_group_t* pg, _acl_cb_para_t* cb_para);
extern int32
_sys_usw_acl_show_spool_status(uint8 lchip);
extern int32
_sys_usw_acl_show_entry_all(uint8 lchip, uint8 sort_type, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
_sys_usw_acl_show_entry_by_lkup_level(uint8 lchip, uint8 level, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
_sys_usw_acl_show_entry_by_priority(uint8 lchip, uint8 prio, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
_sys_usw_acl_show_entry_by_group_id(uint8 lchip, uint32 gid, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
_sys_usw_acl_show_entry_by_entry_id(uint8 lchip, uint32 eid, uint8 key_type, uint8 detail, uint8 show_type, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
_sys_usw_acl_get_entry_by_eid(uint8 lchip, uint32 eid, sys_acl_entry_t** pe);
extern int32
_sys_usw_acl_get_group_by_gid(uint8 lchip, uint32 gid, sys_acl_group_t** o_group);
extern int32
_sys_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);
extern int32
_sys_usw_acl_destroy_group(uint8 lchip, uint32 group_id);
extern int32
_sys_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);
extern int32
_sys_usw_acl_uninstall_group(uint8 lchip, uint32 group_id);
extern int32
_sys_usw_acl_install_entry(uint8 lchip, sys_acl_entry_t* pe, uint8 flag, uint8 move_sw);
extern int32
_sys_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* p_ctc_entry);
extern int32
_sys_usw_acl_remove_entry(uint8 lchip, uint32 entry_id);
extern int32
_sys_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id);
extern int32
_sys_usw_acl_set_league_merge(uint8 lchip, sys_acl_league_t* p_league1, sys_acl_league_t* p_league2, uint8 auto_en);
extern int32
_sys_usw_acl_set_league_cancel(uint8 lchip, sys_acl_league_t* p_league1, sys_acl_league_t* p_league2);
extern int32
_sys_usw_acl_reorder_entry_scatter(uint8 lchip, sys_acl_league_t* p_sys_league);
extern int32
_sys_usw_acl_reorder_entry_down_to_up(uint8 lchip, sys_acl_league_t* p_sys_league);
extern int32
_sys_usw_acl_reorder_entry_up_to_down(uint8 lchip, sys_acl_league_t* p_sys_league);
extern int32
_sys_usw_acl_wb_mapping_group(sys_wb_acl_group_t* p_wb_acl_group, sys_acl_group_t* p_acl_group, uint8 is_sync);
extern int32
_sys_usw_acl_wb_mapping_entry(sys_wb_acl_entry_t* p_wb_acl_entry, sys_acl_entry_t* p_acl_entry, uint8 is_sync);
extern int32
_sys_usw_acl_wb_sync_entry(void* bucket_data, void* user_data);
extern int32
_sys_usw_acl_wb_sync_group(void* bucket_data, void* user_data);
extern int32
_sys_usw_acl_free_entry_node_data(void* node_data, void* user_data);
extern int32
_sys_usw_acl_free_group_node_data(void* node_data, void* user_data);
extern int32
_sys_usw_acl_hash_traverse_cb_show_group(sys_acl_group_t* pg, _acl_cb_para_t* cb_para);
extern int32
_sys_usw_acl_unmap_group_info(uint8 lchip, ctc_acl_group_info_t* ctc, sys_acl_group_info_t* sys);
extern int32
_sys_usw_acl_add_group_key_port_field(uint8 lchip, uint32 entry_id,sys_acl_group_t* pg);

extern int32
_sys_usw_acl_opf_init(uint8 lchip);
extern int32
_sys_usw_acl_ofb_init(uint8 lchip);
extern int32
_sys_usw_acl_spool_init(uint8 lchip);
extern int32
_sys_usw_acl_init_league_bitmap(uint8 lchip);
extern int32
_sys_usw_acl_init_vlan_edit(uint8 lchip);
extern int32
_sys_usw_acl_init_global_cfg(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg);
extern int32
_sys_usw_acl_create_rsv_group(uint8 lchip);
extern int32
_sys_usw_acl_update_key_count(uint8 lchip, sys_acl_entry_t* pe, uint8 is_add);
extern int32
_sys_usw_acl_check_hash_sel_field_union(uint8 lchip, uint8 hash_type, uint8 field_sel_id, ctc_field_key_t* sel_field);
extern int32
_sys_usw_acl_set_hash_sel_field_union(uint8 lchip, uint8 hash_type, uint8 field_sel_id, ctc_field_key_t* sel_field, uint8 is_add);
extern int32
_sys_usw_acl_check_udf_entry_key_field_union(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add);
extern int32
_sys_usw_acl_set_udf_entry_key_field_union(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field,uint8 is_add);
extern int32
_sys_usw_acl_get_entry_count_on_lkup_level(uint8 lchip, uint8 block_id, uint8 level, uint32* count);
extern uint32
_sys_usw_acl_hash_make_entry(sys_acl_entry_t* pe);
extern bool
_sys_usw_acl_hash_compare_entry(sys_acl_entry_t* pe0, sys_acl_entry_t* pe1);
extern uint32
_sys_usw_acl_hash_make_group(sys_acl_group_t* pg);
extern bool
_sys_usw_acl_hash_compare_group(sys_acl_group_t* pg0, sys_acl_group_t* pg1);
extern int32
_sys_usw_acl_install_build_key_fn(uint8 lchip);
extern int32
_sys_usw_acl_get_action_fn(uint8 lchip);
extern int32
_sys_usw_acl_get_block_by_pe_fpa(ctc_fpa_entry_t* pe, ctc_fpa_block_t** pb);
extern int32
_sys_usw_acl_entry_move_hw_fpa(ctc_fpa_entry_t* p_fpa_entry, int32 tcam_idx_new);
extern int32
_sys_usw_acl_l4type_ipprotocol_judge(uint8 lchip, uint8 key_type, uint8 hash_sel_id);
extern int32
_sys_usw_acl_unmap_ethertype(uint8 l3_type, uint16* o_ethertype);
extern int32
_sys_usw_acl_unmap_fwd_type(uint8 pkt_fwd_type_drv, uint8* o_pkt_fwd_type);
extern int32
_sys_usw_acl_unmap_ip_frag(uint8 lchip, uint32 frag_info_data, uint32 frag_info_mask, uint8* o_ctc_ip_frag, uint8* o_ctc_ip_frag_mask);
extern int32
_sys_usw_acl_map_l4_type_to_ip_protocol(uint8 lchip, uint8 l3_type, uint8 l4_type, uint32* o_ip_protocol, uint32* o_mask);
extern int32
_sys_usw_acl_map_ip_protocol_to_l4_type(uint8 lchip, uint8 l3_type, uint8 ip_protocol,uint32* o_l4_type, uint32* o_mask);

extern int32
_sys_usw_acl_league_traverse_block(uint8 lchip, uint8 tmp_block_id, uint16* entry_base,
                                                ctc_linklist_t** node_list, uint16* sub_entry_num,
                                                uint16* sub_free_num, sys_usw_acl_league_node_t** node_array);

extern int32 _sys_usw_acl_league_move_entry(uint8 lchip, ctc_linklist_t** node_list, ctc_fpa_entry_t** p_tmp_entries,
                                                uint16* sub_entry_num, uint8 block_id, sys_usw_acl_league_node_t** node_array);
extern int32 _sys_usw_acl_adjust_ext_tcam(uint8 lchip, uint16 ext_tcam_bitmap);
#ifdef __cplusplus
}
#endif
#endif


