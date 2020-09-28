
#ifndef __STBL_IPMC_DEFINE_H__
#define __STBL_IPMC_DEFINE_H__

/* STBL_IPMC field defines */
typedef enum
{
    STBL_IPMC_FLD_KEY                  = 0 ,  /* READ */
    STBL_IPMC_FLD_GROUP_ID             = 1 ,  /* READ */
    STBL_IPMC_FLD_NEXTHOP_ID           = 2 ,  /* READ */
    STBL_IPMC_FLD_COUNTER_ID           = 3 ,  /* RW */
    STBL_IPMC_FLD_NEXTHOP_GROUP_ID     = 4 ,  /* READ */
    STBL_IPMC_FLD_RPF_ROUTER_INTERFACE = 5 ,  /* READ */
    STBL_IPMC_FLD_MAX                  = 6 
} stbl_ipmc_field_id_t;

/* STBL_IPMC defines */
typedef sai_ipmc_entry_t stbl_ipmc_key_t;

typedef struct
{
    stbl_ipmc_key_t      key;
    uint32               group_id;            /* ipmc mcast group ID */
    uint32               nexthop_id;
    uint32               counter_id;          /*add by zhw for ipmc*/
    sai_object_id_t      nexthop_group_id;
    sai_object_id_t      rpf_router_interface;
} stbl_ipmc_t;

typedef struct
{
    ctclib_hash_t        *ipmc_hash;
} stbl_ipmc_master_t;

#endif /* !__STBL_IPMC_DEFINE_H__ */

