/**
   @file sys_usw_scl.h

   @date 2017-01-23

   @version v1.0

   The file contains all acl APIs of sys layer

 */
 #ifndef _SYS_USW_ACL_API_H
#define _SYS_USW_ACL_API_H

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
#include "ctc_acl.h"
#include "ctc_field.h"
#include "sys_usw_common.h"


/****************************************************************************
 *
 * Defines and Macros
 *
 *****************************************************************************/

#define ACL_IGS_BLOCK_MAX_NUM    8      /* ingress acl */
#define ACL_EGS_BLOCK_MAX_NUM    3      /* egress acl */

#define SYS_ACL_HASH_SEL_PROFILE_MAX    16
#define SYS_ACL_MAX_UDF_ENTRY_NUM       16

enum sys_acl_field_key_type_e
{
    SYS_FIELD_KEY_HASH_SEL_ID = 1000,
};

enum sys_acl_hash_key_type
{
    SYS_ACL_HASH_KEY_TYPE_MAC  ,
    SYS_ACL_HASH_KEY_TYPE_IPV4  ,
    SYS_ACL_HASH_KEY_TYPE_L2_L3 ,
    SYS_ACL_HASH_KEY_TYPE_MPLS  ,
    SYS_ACL_HASH_KEY_TYPE_IPV6 ,
    SYS_ACL_HASH_KEY_TYPE_NUM
};

enum sys_acl_range_type
{
    ACL_RANGE_TYPE_NONE,
    ACL_RANGE_TYPE_PKT_LENGTH  = 1,
    ACL_RANGE_TYPE_L4_SRC_PORT = 2,
    ACL_RANGE_TYPE_L4_DST_PORT = 3,
    ACL_RANGE_TYPE_NUM
};

enum sys_acl_count_type
{
    ACL_ALLOC_COUNT_TCAM_80,
    ACL_ALLOC_COUNT_TCAM_160,
    ACL_ALLOC_COUNT_TCAM_320,
    ACL_ALLOC_COUNT_TCAM_640,
    ACL_ALLOC_COUNT_HASH,
    ACL_ALLOC_COUNT_NUM
};

struct sys_acl_udf_entry_s
{
  uint32    key_index     :4 , /*0~15*/
            udf_hit_index :4, /* used to acl key udf_hit_index*/
            key_index_used:1,
            ip_op         :1,
            ip_frag       :1 ,
            mpls_num      :1 ,
            l4_type       :4,
            udf_offset_num:3,
            ip_protocol   :1,
            rsv           :12;
  uint32 udf_id ;
};
typedef struct sys_acl_udf_entry_s   sys_acl_udf_entry_t;

typedef struct
{
    ctc_direction_t dir;
    uint8           type;

    uint8           block_id;      /* block the group belong. For hash, 0xFF. */
    uint8           bitmap_status;
    uint8          lchip ; /*only used for bitmap*/

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
    uint8            key_size;
}sys_acl_group_info_t;

typedef struct
{
    uint32               group_id; /* keep group_id top! */
    uint32               entry_count;
    uint8                lchip;
    uint8                rsv0[3];
    ctc_slist_t          * entry_list;      /* a list that link entries belong to this group */
    sys_acl_group_info_t group_info;        /* group info */
}sys_acl_group_t;

extern int32
sys_usw_acl_init(uint8 lchip, ctc_acl_global_cfg_t* acl_global_cfg);
extern int32
sys_usw_acl_deinit(uint8 lchip);
extern int32
sys_usw_acl_create_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);
extern int32
sys_usw_acl_destroy_group(uint8 lchip, uint32 group_id);
extern int32
sys_usw_acl_install_group(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);
extern int32
sys_usw_acl_uninstall_group(uint8 lchip, uint32 group_id);
extern int32
sys_usw_acl_get_group_info(uint8 lchip, uint32 group_id, ctc_acl_group_info_t* group_info);
extern int32
sys_usw_acl_add_entry(uint8 lchip, uint32 group_id, ctc_acl_entry_t* acl_entry);
extern int32
sys_usw_acl_remove_entry(uint8 lchip, uint32 entry_id);
extern int32
sys_usw_acl_install_entry(uint8 lchip, uint32 entry_id);
extern int32
sys_usw_acl_uninstall_entry(uint8 lchip, uint32 entry_id);
extern int32
sys_usw_acl_remove_all_entry(uint8 lchip, uint32 group_id);
extern int32
sys_usw_acl_set_entry_priority(uint8 lchip, uint32 entry_id, uint32 priority);
extern int32
sys_usw_acl_get_multi_entry(uint8 lchip, ctc_acl_query_t* query);
extern int32
sys_usw_acl_copy_entry(uint8 lchip, ctc_acl_copy_entry_t* copy_entry);
extern int32
sys_usw_acl_set_hash_field_sel(uint8 lchip, ctc_acl_hash_field_sel_t* field_sel);
extern int32
sys_usw_acl_get_hash_field_sel(uint8 lchip, uint8 hash_key_type, uint8 field_sel_id, uint16 key_field_type, uint8* value_out);
extern int32
sys_usw_acl_add_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field);
extern int32
sys_usw_acl_remove_key_field(uint8 lchip, uint32 entry_id, ctc_field_key_t* key_field);
extern int32
sys_usw_acl_add_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field);
extern int32
sys_usw_acl_remove_action_field(uint8 lchip, uint32 entry_id, ctc_acl_field_action_t* action_field);
extern int32
sys_usw_acl_set_field_to_hash_field_sel(uint8 lchip, uint8 key_type, uint8 field_sel_id, ctc_field_key_t* sel_field);
extern int32
sys_usw_acl_add_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
sys_usw_acl_remove_cid_pair(uint8 lchip, ctc_acl_cid_pair_t* cid_pair);
extern int32
sys_usw_acl_show_status(uint8 lchip);
extern int32
sys_usw_acl_show_entry(uint8 lchip, uint8 type, uint32 param, uint8 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
sys_usw_acl_show_group(uint8 lchip, uint8 type);
extern int32
sys_usw_acl_map_ctc_to_sys_hash_key_type(uint8 key_type ,uint8 *o_key_type);
extern int32
sys_usw_acl_get_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
extern int32
sys_usw_acl_get_udf_hit_index(uint8 lchip,  uint32 udf_id, uint8* udf_hit_index);
extern int32
sys_usw_acl_add_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
extern int32
sys_usw_acl_remove_udf_entry(uint8 lchip,  ctc_acl_classify_udf_t* udf_entry);
extern int32
sys_usw_acl_add_udf_entry_key_field(uint8 lchip,  uint32 udf_id, ctc_field_key_t* key_field);
extern int32
sys_usw_acl_remove_udf_entry_key_field(uint8 lchip, uint32 udf_id, ctc_field_key_t* key_field);
extern int32
sys_usw_acl_set_league_mode(uint8 lchip, ctc_acl_league_t* league);
extern int32
sys_usw_acl_get_league_mode(uint8 lchip, ctc_acl_league_t* league);
extern int32
sys_usw_acl_reorder_entry(uint8 lchip, ctc_acl_reorder_t* reorder);
extern int32
sys_usw_acl_set_dot1ae_crypt_error_to_cpu_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_acl_set_wlan_crypt_error_to_cpu_en(uint8 lchip, uint8 enable);
extern int32
sys_usw_acl_get_udf_info(uint8 lchip,  uint32 udf_id, sys_acl_udf_entry_t** p_udf_entry);
extern int32
sys_usw_acl_add_port_field(uint8 lchip, uint32 entry_id, ctc_field_port_t port);
#ifdef __cplusplus
}
#endif
#endif
