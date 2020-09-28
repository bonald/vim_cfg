
#ifndef __TBL_PSFDB_DEFINE_H__
#define __TBL_PSFDB_DEFINE_H__

/* TBL_PSFDB field defines */
typedef enum
{
    TBL_PSFDB_FLD_KEY                  = 0 ,  /* READ */
    TBL_PSFDB_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_PSFDB_FLD_PORT_SECURITY        = 2 ,  /* READ */
    TBL_PSFDB_FLD_MAX                  = 3 
} tbl_psfdb_field_id_t;

/* TBL_PSFDB defines */
typedef fdb_key_t tbl_psfdb_key_t;

typedef struct
{
    tbl_psfdb_key_t      key;
    uint32               ifindex;
    uint32               flags;
} tbl_psfdb_t;

typedef struct
{
    ctclib_hash_t        *psfdb_hash;
} tbl_psfdb_master_t;

#endif /* !__TBL_PSFDB_DEFINE_H__ */

