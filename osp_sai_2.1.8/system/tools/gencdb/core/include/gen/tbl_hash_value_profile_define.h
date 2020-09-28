
#ifndef __TBL_HASH_VALUE_PROFILE_DEFINE_H__
#define __TBL_HASH_VALUE_PROFILE_DEFINE_H__

/* TBL_HASH_VALUE_PROFILE field defines */
typedef enum
{
    TBL_HASH_VALUE_PROFILE_FLD_KEY                  = 0 ,  /* READ */
    TBL_HASH_VALUE_PROFILE_FLD_DESCRIPTION          = 1 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA          = 2 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_BITMAP_COMMAND       = 3 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_INDEX                = 4 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_AGG_ALL              = 5 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_AGG_INPUT            = 6 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_AGG_STATIC           = 7 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_AGG_DYNAMIC          = 8 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_AGG_RESILIENT        = 9 ,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_ALL             = 10,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC          = 11,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_L3       = 12,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_MPLS     = 13,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_NVGRE    = 14,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_STATIC_VXLAN    = 15,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC         = 16,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_ECMP_DYNAMIC_FLOW_ID = 17,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_REF_CNT              = 18,  /* RW */
    TBL_HASH_VALUE_PROFILE_FLD_MAX                  = 19
} tbl_hash_value_profile_field_id_t;

/* TBL_HASH_VALUE_PROFILE defines */
typedef struct
{
    char                 name[HASH_VALUE_PROFILE_NAME_SIZE+1];
} tbl_hash_value_profile_key_t;

typedef struct
{
    tbl_hash_value_profile_key_t key;
    char                 description[DESC_SIZE];
    uint32               bitmap_data;
    uint32               bitmap_command;
    uint32               index;
    uint32               agg_all;
    uint32               agg_input;
    uint32               agg_static;
    uint32               agg_dynamic;
    uint32               agg_resilient;
    uint32               ecmp_all;
    uint32               ecmp_static;
    uint32               ecmp_static_l3;
    uint32               ecmp_static_mpls;
    uint32               ecmp_static_nvgre;
    uint32               ecmp_static_vxlan;
    uint32               ecmp_dynamic;
    uint32               ecmp_dynamic_flow_id;
    uint32               ref_cnt;
} tbl_hash_value_profile_t;

typedef struct
{
    ctclib_hash_t        *hash_value_profile_hash;
    ctclib_slist_t       *hash_value_profile_list;
} tbl_hash_value_profile_master_t;

#endif /* !__TBL_HASH_VALUE_PROFILE_DEFINE_H__ */

