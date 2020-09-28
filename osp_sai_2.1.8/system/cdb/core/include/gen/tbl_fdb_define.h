
#ifndef __TBL_FDB_DEFINE_H__
#define __TBL_FDB_DEFINE_H__

/* TBL_FDB field defines */
typedef enum
{
    TBL_FDB_FLD_KEY                  = 0 ,  /* READ */
    TBL_FDB_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_FDB_FLD_STATIC               = 2 ,  /* READ */
    TBL_FDB_FLD_SECURITY             = 3 ,  /* READ */
    TBL_FDB_FLD_HYBRID_FLOW          = 4 ,  /* READ */
    TBL_FDB_FLD_MAX                  = 5 
} tbl_fdb_field_id_t;

/* TBL_FDB defines */
typedef fdb_key_t tbl_fdb_key_t;

typedef struct
{
    tbl_fdb_key_t        key;
    uint32               ifindex;             /* out interface */
    uint32               flags;
} tbl_fdb_t;

typedef struct
{
    ctclib_hash_t        *fdb_hash;
} tbl_fdb_master_t;

#endif /* !__TBL_FDB_DEFINE_H__ */

