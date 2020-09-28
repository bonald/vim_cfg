
#ifndef __TBL_NEXTHOP_DEFINE_H__
#define __TBL_NEXTHOP_DEFINE_H__

/* TBL_NEXTHOP field defines */
typedef enum
{
    TBL_NEXTHOP_FLD_KEY                  = 0 ,  /* READ */
    TBL_NEXTHOP_FLD_REFCNT               = 1 ,  /* RW */
    TBL_NEXTHOP_FLD_MAX                  = 2 
} tbl_nexthop_field_id_t;

/* TBL_NEXTHOP defines */
typedef nexthop_key_t tbl_nexthop_key_t;

typedef struct
{
    tbl_nexthop_key_t    key;
    uint32               refcnt;
} tbl_nexthop_t;

typedef struct
{
    ctclib_hash_t        *nh_hash;
} tbl_nexthop_master_t;

#endif /* !__TBL_NEXTHOP_DEFINE_H__ */

