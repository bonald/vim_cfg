
#ifndef __STBL_NEXTHOP_GROUP_DEFINE_H__
#define __STBL_NEXTHOP_GROUP_DEFINE_H__

/* STBL_NEXTHOP_GROUP field defines */
typedef enum
{
    STBL_NEXTHOP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_TYPE                 = 1 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_IPMC_GROUP_ID        = 2 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_ECMP_TYPE            = 3 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_FAILOVER_EN          = 4 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_NH_LIST              = 5 ,  /* READ */
    STBL_NEXTHOP_GROUP_FLD_MAX                  = 6 
} stbl_nexthop_group_field_id_t;

/* STBL_NEXTHOP_GROUP defines */
typedef struct
{
    uint32               group_id;
} stbl_nexthop_group_key_t;

typedef struct
{
    stbl_nexthop_group_key_t key;
    uint32               type;
    uint32               ipmc_group_id;
    uint32               ecmp_type;
    uint32               failover_en;
    sdb_reference_list_t nh_list;             /* list of STBL_NEXTHOP */
} stbl_nexthop_group_t;

typedef struct
{
    stbl_nexthop_group_t *array[GLB_MAX_CTC_NEXTHOP_GROUP_ID+1];
} stbl_nexthop_group_master_t;

#endif /* !__STBL_NEXTHOP_GROUP_DEFINE_H__ */

