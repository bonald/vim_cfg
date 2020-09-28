
#ifndef __TBL_HASH_VALUE_GLOBAL_DEFINE_H__
#define __TBL_HASH_VALUE_GLOBAL_DEFINE_H__

/* TBL_HASH_VALUE_GLOBAL field defines */
typedef enum
{
    TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA          = 0 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_COMMAND       = 1 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_AGG_HASH             = 2 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_ALL             = 3 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC          = 4 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_L3       = 5 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_MPLS     = 6 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_NVGRE    = 7 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_STATIC_VXLAN    = 8 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC         = 9 ,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_ECMP_DYNAMIC_FLOW_ID = 10,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_LBID                 = 11,  /* RW */
    TBL_HASH_VALUE_GLOBAL_FLD_MAX                  = 12
} tbl_hash_value_global_field_id_t;

/* TBL_HASH_VALUE_GLOBAL defines */
typedef struct
{
    uint32               bitmap_data;
    uint32               bitmap_command;
    uint32               agg_hash;
    uint32               ecmp_all;
    uint32               ecmp_static;
    uint32               ecmp_static_l3;
    uint32               ecmp_static_mpls;
    uint32               ecmp_static_nvgre;
    uint32               ecmp_static_vxlan;
    uint32               ecmp_dynamic;
    uint32               ecmp_dynamic_flow_id;
    uint32               lbid;
} tbl_hash_value_global_t;

#endif /* !__TBL_HASH_VALUE_GLOBAL_DEFINE_H__ */

