
#ifndef __TBL_NEXTHOP_GROUP_ROUTED_DEFINE_H__
#define __TBL_NEXTHOP_GROUP_ROUTED_DEFINE_H__

/* TBL_NEXTHOP_GROUP_ROUTED field defines */
typedef enum
{
    TBL_NEXTHOP_GROUP_ROUTED_FLD_KEY                  = 0 ,  /* READ */
    TBL_NEXTHOP_GROUP_ROUTED_FLD_NHG_OID              = 1 ,  /* RW */
    TBL_NEXTHOP_GROUP_ROUTED_FLD_REF_CNT              = 2 ,  /* RW */
    TBL_NEXTHOP_GROUP_ROUTED_FLD_ECMP_HASH            = 3 ,  /* RW */
    TBL_NEXTHOP_GROUP_ROUTED_FLD_MAX                  = 4 
} tbl_nexthop_group_routed_field_id_t;

/* TBL_NEXTHOP_GROUP_ROUTED defines */
typedef nexthop_ecmp_routed_t tbl_nexthop_group_routed_key_t;

typedef struct
{
    tbl_nexthop_group_routed_key_t key;
    uint64               nhg_oid;
    uint32               ref_cnt;
    uint32               ecmp_hash;
} tbl_nexthop_group_routed_t;

typedef struct
{
    ctclib_hash_t        *nhg_hash;
    ctclib_slist_t       *nhg_list;
} tbl_nexthop_group_routed_master_t;

#endif /* !__TBL_NEXTHOP_GROUP_ROUTED_DEFINE_H__ */

