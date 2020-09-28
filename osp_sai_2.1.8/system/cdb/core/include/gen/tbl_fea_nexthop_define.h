
#ifndef __TBL_FEA_NEXTHOP_DEFINE_H__
#define __TBL_FEA_NEXTHOP_DEFINE_H__

/* TBL_FEA_NEXTHOP field defines */
typedef enum
{
    TBL_FEA_NEXTHOP_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_NEXTHOP_FLD_NHID                 = 1 ,  /* RW */
    TBL_FEA_NEXTHOP_FLD_MAX                  = 2 
} tbl_fea_nexthop_field_id_t;

/* TBL_FEA_NEXTHOP defines */
typedef nexthop_key_t tbl_fea_nexthop_key_t;

typedef struct
{
    tbl_fea_nexthop_key_t key;
    uint64               nhid;
} tbl_fea_nexthop_t;

typedef struct
{
    ctclib_hash_t        *nh_hash;
} tbl_fea_nexthop_master_t;

#endif /* !__TBL_FEA_NEXTHOP_DEFINE_H__ */

