
#ifndef __STBL_NEXTHOP_DEFINE_H__
#define __STBL_NEXTHOP_DEFINE_H__

/* STBL_NEXTHOP field defines */
typedef enum
{
    STBL_NEXTHOP_FLD_KEY                  = 0 ,  /* READ */
    STBL_NEXTHOP_FLD_NH_OID               = 1 ,  /* READ */
    STBL_NEXTHOP_FLD_NH_TYPE              = 2 ,  /* READ */
    STBL_NEXTHOP_FLD_MAX                  = 3 
} stbl_nexthop_field_id_t;

/* STBL_NEXTHOP defines */
typedef sai_neighbor_entry_t stbl_nexthop_key_t;

typedef struct
{
    stbl_nexthop_key_t   key;
    sai_object_id_t      nh_oid;
    uint32               nh_type;
} stbl_nexthop_t;

typedef struct
{
    ctclib_hash_t        *nexthop_hash;
    stbl_nexthop_t       *array[GLB_MAX_CTC_NEXTHOP_ID+1];
} stbl_nexthop_master_t;

#endif /* !__STBL_NEXTHOP_DEFINE_H__ */

