
#ifndef __TBL_NEXTHOP_GROUP_DEFINE_H__
#define __TBL_NEXTHOP_GROUP_DEFINE_H__

/* TBL_NEXTHOP_GROUP field defines */
typedef enum
{
    TBL_NEXTHOP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_NEXTHOP_GROUP_FLD_NH_GROUP             = 1 ,  /* RW */
    TBL_NEXTHOP_GROUP_FLD_NHG_OID              = 2 ,  /* RW */
    TBL_NEXTHOP_GROUP_FLD_REF_CNT              = 3 ,  /* RW */
    TBL_NEXTHOP_GROUP_FLD_ECMP_HASH            = 4 ,  /* RW */
    TBL_NEXTHOP_GROUP_FLD_MAX                  = 5 
} tbl_nexthop_group_field_id_t;

/* TBL_NEXTHOP_GROUP defines */
typedef nexthop_ecmp_t tbl_nexthop_group_key_t;

typedef struct
{
    tbl_nexthop_group_key_t key;
    nexthop_ecmp_t       nh_group;
    uint64               nhg_oid;
    uint32               ref_cnt;
    uint32               ecmp_hash;
} tbl_nexthop_group_t;

typedef struct
{
    ctclib_hash_t        *nhg_hash;
    ctclib_slist_t       *nhg_list;
} tbl_nexthop_group_master_t;

#endif /* !__TBL_NEXTHOP_GROUP_DEFINE_H__ */

