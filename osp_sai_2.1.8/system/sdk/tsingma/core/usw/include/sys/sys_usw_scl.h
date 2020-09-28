/**
   @file sys_usw_scl.h

   @date 2017-01-23

   @version v1.0

   The file contains all scl APIs of sys layer

 */
 #ifndef _SYS_USW_SCL_NEW_H
#define _SYS_USW_SCL_NEW_H

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"

#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_spool.h"
#include "ctc_const.h"
#include "ctc_vlan.h"
#include "ctc_qos.h"
#include "ctc_linklist.h"
#include "ctc_debug.h"

#include "ctc_scl.h"

#include "sys_usw_scl_api.h"
#include "sys_usw_chip.h"
#include "sys_usw_fpa.h"
#include "drv_api.h"

/****************************************************************************
 *
 * Defines and Macros
 *
 *****************************************************************************/

#define SYS_USW_SCL_GPORT_IS_LOCAL(lchip,gport) \
     (CTC_IS_LINKAGG_PORT(gport) || !SYS_GCHIP_IS_REMOTE(lchip,SYS_MAP_CTC_GPORT_TO_GCHIP(gport)))

#define SCL_TCAM_NUM    4

#define SYS_SCL_DEFAULT_ENTRY_PORT_BASE                   256
#define SYS_SCL_DEFAULT_ENTRY_BASE                   256
#define SYS_SCL_VEDIT_PRFID_SVLAN_CLASS                   3
#define SYS_SCL_VEDIT_PRFID_SVLAN_SCOS_CLASS              2
#define SYS_SCL_VEDIT_PRFID_SVLAN_DOMAIN                  1

#define SYS_SCL_TRAVERSE_ALL                              0
#define SYS_SCL_TRAVERSE_BY_PORT                          1
#define SYS_SCL_TRAVERSE_BY_GROUP                         2

#define SYS_SCL_MIN_INNER_ENTRY_ID      0x80000000 /* min sdk entry id*/
#define SYS_SCL_MAX_INNER_ENTRY_ID      0xFFFFFFFF /* max sdk entry id*/

#define SYS_SCL_GROUP_LIST_NUM                         3
#define SYS_SCL_GROUP_ID_MAX                           0xFFFFFFFF

#define SCL_KEY_USW_SIZE_80             12
#define SCL_KEY_USW_SIZE_160            24
#define SCL_KEY_USW_SIZE_320            48
#define SCL_KEY_USW_SIZE_640            96
#define SCL_KEY_USW_SIZE_DEFAULT_PORT   4

#define SYS_SCL_HASH_SEL_ID_MAX     4

#define SYS_USW_SCL_LOGIC_PORT_MAX_NUM (DRV_IS_DUET2(lchip)? 0x7FFF: 0xFFFF)

#define SYS_SCL_CHECK_OUTER_GROUP_ID(gid, unlock)        \
    {                                            \
        if ((gid) >= CTC_SCL_GROUP_ID_RSV_MAX) { \
            if((unlock)) {                      \
                sal_mutex_unlock(p_usw_scl_master[lchip]->mutex); }\
            return CTC_E_BADID; }         \
    }

#define SYS_SCL_CHECK_GROUP_TYPE(type)        \
    {                                         \
        if (type >= CTC_SCL_GROUP_TYPE_MAX) { \
            return CTC_E_INVALID_PARAM; }    \
    }

#define SYS_SCL_CHECK_GROUP_PRIO(priority)           \
    {                                                \
        if (priority >= MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)) { \
            return CTC_E_INVALID_PARAM; }       \
    }

#define SYS_SCL_CHECK_ENTRY_ID(eid, unlock)            \
    {                                          \
        if (eid > CTC_SCL_MAX_USER_ENTRY_ID) { \
            if((unlock))                    \
            {                               \
                sal_mutex_unlock(p_usw_scl_master[lchip]->mutex); \
            }                               \
            return CTC_E_INVALID_PARAM; }      \
    }

#define SYS_SCL_CHECK_ENTRY_PRIO(priority)  \
    {                                          \
        if (priority > FPA_PRIORITY_HIGHEST) { \
            return CTC_E_INVALID_PARAM; }      \
    }

#define SYS_SCL_CHECK_GROUP_UNEXIST(pg, unlock)     \
    {                                       \
        if (!pg)                            \
        {                                   \
            if((unlock))                    \
            {                               \
                sal_mutex_unlock(p_usw_scl_master[lchip]->mutex); \
            }                               \
            return CTC_E_NOT_EXIST; \
        }                                   \
    }

#define SYS_SCL_SET_MAC_HIGH(dest_h, src)    \
    {                                        \
        dest_h = ((src[0] << 8) | (src[1])); \
    }

#define SYS_SCL_SET_MAC_LOW(dest_l, src)                                       \
    {                                                                          \
        dest_l = ((src[2] << 24) | (src[3] << 16) | (src[4] << 8) | (src[5])); \
    }

#define SYS_SCL_LOCK(lchip) \
    if(p_usw_scl_master[lchip]->mutex) sal_mutex_lock(p_usw_scl_master[lchip]->mutex)

#define SYS_SCL_UNLOCK(lchip) \
    if(p_usw_scl_master[lchip]->mutex) sal_mutex_unlock(p_usw_scl_master[lchip]->mutex)

#define CTC_USW_RETURN_SCL_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_usw_scl_master[lchip]->mutex); \
        return (op); \
    } while (0)

#define CTC_USW_ERROR_RETURN_SCL_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_scl_master[lchip]->mutex); \
            return rv; \
        } \
    } while (0)

#define SYS_SCL_INIT_CHECK()         \
    {                                \
        LCHIP_CHECK(lchip);          \
        if (!p_usw_scl_master[lchip]) {           \
            return CTC_E_NOT_INIT; } \
    }

#define SCL_ENTRY_IS_TCAM(type)                                                   \
    ((SYS_SCL_KEY_TCAM_VLAN == type) || (SYS_SCL_KEY_TCAM_MAC == type) ||         \
     (SYS_SCL_KEY_TCAM_IPV4 == type) || (SYS_SCL_KEY_TCAM_IPV4_SINGLE == type) || \
     (SYS_SCL_KEY_TCAM_IPV6 == type) || (SYS_SCL_KEY_TCAM_IPV6_SINGLE == type) || \
     (SYS_SCL_KEY_TCAM_UDF == type) || (SYS_SCL_KEY_TCAM_UDF_EXT == type))

#define SYS_SCL_CHECK_MODE_TYPE(type)  \
    {                                          \
        if (type >= 2) { \
            return CTC_E_INVALID_PARAM; }      \
    }

#define SYS_SCL_DBG_OUT(level, FMT, ...)    CTC_DEBUG_OUT(scl, scl, SCL_SYS, level, FMT, ## __VA_ARGS__)

#define SET_VLANXC   SetDsEgressXcOamDoubleVlanPortHashKey
#define GET_VLANXC   GetDsEgressXcOamDoubleVlanPortHashKey

#define SYS_SCL_GET_STEP(fpa_size)  \
        (((fpa_size)==CTC_FPA_KEY_SIZE_80)?1:(((fpa_size)==CTC_FPA_KEY_SIZE_160)?2:\
                (((fpa_size)==CTC_FPA_KEY_SIZE_320)?4:(((fpa_size)==CTC_FPA_KEY_SIZE_640)?8:1))))

#define SCL_INNER_ENTRY_ID(eid)          ((eid >= SYS_SCL_MIN_INNER_ENTRY_ID) && (eid <= SYS_SCL_MAX_INNER_ENTRY_ID))

typedef struct
{
    DsVlanActionProfile_m action_profile;
    uint16 profile_id;
    uint8  rsv[2];
}sys_scl_sp_vlan_edit_t;


struct sys_scl_action_acl_s
{
    /* key */
    DsSclAclControlProfile_m  acl_control_profile;
    uint8 is_scl;
    uint32 calc_key_len[0];
    /* data */
    uint8 profile_id;
};
typedef struct sys_scl_action_acl_s sys_scl_action_acl_t;

typedef struct
{
    /* key */
    DsUserId_m  action;
    uint8       action_type;         /* to distinguish dsuserid or dssclflow or dstunnelid */
    uint32      calc_key_len[0];

    /* data */
    uint8      is_half;
    uint8     priority;
    uint16     ad_index;
}sys_scl_sw_hash_ad_t;


typedef struct
{
    uint32         group_id;
    uint8          type;
    uint8          priority;        /* group priority */
    uint32         gport;           /* port_class_id/gport/logic_port */

    ctc_slist_t*   entry_list;      /* a list that link entries belong to this group */
} sys_scl_sw_group_t;

/* field type for show */
enum sys_scl_show_field_key_type_e
{
    SYS_SCL_SHOW_FIELD_KEY_PORT,
    SYS_SCL_SHOW_FIELD_KEY_DST_GPORT,
    SYS_SCL_SHOW_FIELD_KEY_CVLAN_ID,
    SYS_SCL_SHOW_FIELD_KEY_SVLAN_ID,
    SYS_SCL_SHOW_FIELD_KEY_MAC_SA,
    SYS_SCL_SHOW_FIELD_KEY_MAC_DA,
    SYS_SCL_SHOW_FIELD_KEY_IP_SA,
    SYS_SCL_SHOW_FIELD_KEY_IP_DA,
    SYS_SCL_SHOW_FIELD_KEY_IPV6_SA,
    SYS_SCL_SHOW_FIELD_KEY_IPV6_DA,
    SYS_SCL_SHOW_FIELD_KEY_L4_SRC_PORT,
    SYS_SCL_SHOW_FIELD_KEY_L4_DST_PORT,
    SYS_SCL_SHOW_FIELD_KEY_GRE_KEY,
    SYS_SCL_SHOW_FIELD_KEY_VN_ID,
    SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL0,
    SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL1,
    SYS_SCL_SHOW_FIELD_KEY_MPLS_LABEL2,
    SYS_SCL_SHOW_FIELD_KEY_NUM
};
typedef enum sys_scl_show_field_key_type_e sys_scl_show_field_key_type_t;

typedef struct sys_scl_sw_entry_s
{
    ctc_slistnode_t    head;                /* keep head top!! */

    /*key*/
    uint32 entry_id;

    /*action*/
    uint8 lchip;
    uint8 direction;
    uint8 key_type;                         /* sys_scl_key_type_t/sys_scl_tunnel_type_t: SCL Hash/Tunnel Hash/TCAM */
    uint8 action_type;                      /* sys_scl_action_type_t:DsUserid/Egress/DsTunnel/DsSclFlow */

    uint32 resolve_conflict     : 1,
           is_half              : 1,
           rpf_check_en         : 1,
           uninstall            : 1,        /* indicate the entry is installed or not */
           hash_valid           : 1,        /* generated hash index */
           key_exist            : 1,        /* key already exit */
           key_conflict         : 1,        /* hash conflict, no memory */
	       u1_type              : 3,
	       u2_type              : 3,
	       u3_type              : 3,
	       u4_type              : 3,
	       u5_type              : 3,
	       is_service_policer   : 1,
	       bind_nh              : 1,
	       vpws_oam_en          : 1,
	       is_default           : 1,
	       is_pending           : 1,
	       rsv0                 : 5;
    uint32 l2_type              : 4,
	       l3_type              : 4,
	       l4_type              : 4,
	       l4_user_type         : 4,
	       mac_key_vlan_mode    : 2,      /* 0:none, 1:svlan, 2:cvlan */
	       mac_key_macda_mode   : 2,      /* 0:none, 1:macda, 2:customer id */
	       key_l4_port_mode     : 2,      /* 0:none, 1:l4port, 2:vxlan, 3:gre */
	       rsv1                 : 10;
	uint32 u1_bitmap;
	uint16 u3_bitmap;
	uint16  u2_bitmap;
	uint8  u4_bitmap;
	uint16  u5_bitmap;
	uint8  hash_field_sel_id;                 /* Hash select ID */
    uint32 hash_sel_bmp[2];                   /* Hash select bitmap */
    uint32 nexthop_id;
    uint32 stats_id;
    uint32 key_index;
    uint16 ad_index;
    uint16 policer_id;
    uint32 vn_id;
    uint32 action_bmp[(SYS_SCL_FIELD_ACTION_TYPE_MAX-1)/32+1];
    uint32 key_bmp;        /*for show key, refer to sys_scl_show_field_key_type_t */
    uint16 vlan_profile_id;
    uint16 acl_profile_id;
    uint16 ether_type;
    uint8  ether_type_index;
    uint8  inner_rmac_index;                        /* Store the newest index */
    uint16 service_id;
    uint8  cos_idx;
    uint8  rsv2;
    mac_addr_t old_inner_rmac;                      /* Old router mac, add only for roll back */
    sys_scl_sw_group_t* group;                      /* pointer to group node */
    sys_scl_sp_vlan_edit_t* vlan_edit;              /* pointer to vlan_edit_spool node */
    sys_scl_action_acl_t* acl_profile;              /* pointer to acl_spool node */
    sys_scl_sw_hash_ad_t* hash_ad;                  /* pointer to hash_ad_spool node */
    sys_scl_sw_temp_entry_t* temp_entry;            /* if temp_entry is NULL,indicate install entry,else uninstall entry */

    /*used for roollback*/
    struct sys_scl_sw_entry_s* p_pe_backup;

}sys_scl_sw_entry_t;


typedef struct
{
    ctc_fpa_entry_t     fpae;
    sys_scl_sw_entry_t* entry;
}sys_scl_sw_tcam_entry_t;


typedef struct
{
    uint32  key_index;
    uint8   action_type;
    uint8   scl_id;
    uint32  calc_key_len[0];

    uint32 entry_id;

}sys_scl_hash_key_entry_t;


typedef struct
{
    ctc_fpa_block_t fpab;
}sys_scl_sw_block_t;

typedef struct
{
    /* key */
    uint32 key[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    uint32 mask[SYS_SCL_MAX_KEY_SIZE_IN_WORD];
    uint8  scl_id;
    uint8  rsv[3];
    /* ad */
    sys_scl_sw_entry_t* p_entry;
}sys_scl_tcam_entry_key_t;

typedef int32 (* SYS_CB_SCL_BUILD_KEY_FUNC_T)(uint8 lchip, ctc_field_key_t* pKey, sys_scl_sw_entry_t* pe,uint8 is_add);
typedef int32 (* SYS_CB_SCL_BUILD_AD_FUNC_T)(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe,uint8 is_add);
typedef int32 (* SYS_CB_SCL_GET_AD_FUNC_T)(uint8 lchip, ctc_scl_field_action_t* p_action, sys_scl_sw_entry_t* pe);

struct sys_scl_master_s
{
    ctc_hash_t         * group;                               /* Hash table save group by gid.*/
    ctc_fpa_t          * fpa;                                 /* Fast priority arrangement. no array. */
    ctc_hash_t         * entry;                               /* Hash table save hash|tcam entry by eid */
    ctc_hash_t         * hash_key_entry;                      /* Hash table save hash entry by key */
    ctc_hash_t         * tcam_entry_by_key;                   /* Hash table save tcam entry by key */
    ctc_spool_t        * ad_spool[2];                            /* Share pool save hash action */
    ctc_spool_t        * vlan_edit_spool;                     /* Share pool save vlan edit */
    ctc_spool_t        * acl_spool;                           /* Share pool save acl control */
    sys_scl_sw_block_t block[SCL_TCAM_NUM];
    sal_mutex_t        * mutex;
    SYS_CB_SCL_BUILD_KEY_FUNC_T build_key_func[SYS_SCL_KEY_NUM ];
    SYS_CB_SCL_BUILD_AD_FUNC_T build_ad_func[SYS_SCL_ACTION_NUM];
    SYS_CB_SCL_GET_AD_FUNC_T get_ad_func[SYS_SCL_ACTION_NUM];
    uint8              opf_type_vlan_edit;
    uint8              opf_type_entry_id;
    uint8              opf_type_flow_acl_profile;
    uint8              opf_type_tunnel_acl_profile;
    uint8              fpa_size[SYS_SCL_KEY_NUM];
    uint8              key_size[SYS_SCL_KEY_NUM];
    uint32             igs_default_base[SCL_HASH_NUM];
    uint32             tunnel_default_base;

    uint32             entry_count[SYS_SCL_ACTION_NUM];
    uint32             egs_entry_num;
    uint16             hash_sel_profile_count[SYS_SCL_HASH_SEL_ID_MAX];
    uint8              hash_mode;                  /* 0:hash1 and hash2 use same index, 1:hash1 and hash2 use different index, 2:only one hash, default: 0 */
    sys_scl_sw_entry_t* buffer[SYS_USW_MAX_PORT_NUM_PER_CHIP][SYS_SCL_ACTION_TUNNEL + 1];
};
typedef struct sys_scl_master_s   sys_scl_master_t;

extern sys_scl_master_t* p_usw_scl_master[CTC_MAX_LOCAL_CHIP_NUM];

/*internal api*/

extern int32
sys_usw_scl_vlan_tag_op_translate(uint8 lchip, uint8 op_in, uint8* op_out, uint8* mode_out);

extern int32
sys_usw_scl_vlan_tag_op_untranslate(uint8 lchip, uint8 op_in, uint8 mo_in, uint8* op_out);

extern int32
sys_usw_scl_check_vlan_edit(uint8 lchip, ctc_scl_vlan_edit_t* vlan_edit, uint8* do_nothing);

extern int32
sys_usw_scl_get_acl_control_profile(uint8 lchip, uint8 is_tunnel, uint8* lkup_num, ctc_acl_property_t* p_acl_profile, void* p_buffer);

extern void*
sys_usw_scl_add_vlan_edit_action_profile(uint8 lchip,ctc_scl_vlan_edit_t *vlan_edit, void* old_vlan_edit_profile);

extern int32
sys_usw_scl_remove_vlan_edit_action_profile(uint8 lchip, sys_scl_sp_vlan_edit_t* p_vlan_edit);

extern void*
sys_usw_scl_add_acl_control_profile(uint8 lchip,uint8 is_tunnel,uint8 lkup_num,ctc_acl_property_t acl_profile[CTC_MAX_ACL_LKUP_NUM],uint16 *profile_id);

extern int32
sys_usw_scl_remove_acl_control_profile(uint8 lchip, sys_scl_action_acl_t* p_acl_profile);

extern int32
_sys_usw_scl_create_group(uint8 lchip, uint32 group_id, ctc_scl_group_info_t* group_info, uint8 inner);

extern int32
_sys_usw_scl_get_action_field(uint8 lchip, uint32 entry_id, ctc_scl_field_action_t* p_field_action);

extern int32
_sys_usw_scl_get_default_action(uint8 lchip, sys_scl_default_action_t* p_default_action);

extern int32
_sys_usw_scl_get_group_by_gid(uint8 lchip, uint32 gid, sys_scl_sw_group_t** pg);

extern int32
_sys_usw_scl_init_build_key_and_action_cb(uint8 lchip);

extern int32
_sys_usw_scl_build_field_action_igs(uint8 lchip, ctc_scl_field_action_t* pAction, sys_scl_sw_entry_t* pe, uint8 is_add);

extern int32
_sys_usw_scl_get_block_by_pe_fpa(ctc_fpa_entry_t* pe, ctc_fpa_block_t** pb);

extern int32
_sys_usw_scl_entry_move_hw_fpa(ctc_fpa_entry_t* pe, int32 tcam_idx_new);

extern int32
_sys_usw_scl_get_table_id(uint8 lchip, uint8 block_id, sys_scl_sw_entry_t* pe, uint32* key_id, uint32* act_id);

extern int32
_sys_usw_scl_get_index(uint8 lchip, uint32 key_id, sys_scl_sw_entry_t* pe, uint32* o_key_index, uint32* o_ad_index);

#ifdef __cplusplus
}
#endif
#endif

