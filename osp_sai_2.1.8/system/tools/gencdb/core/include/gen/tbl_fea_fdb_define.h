
#ifndef __TBL_FEA_FDB_DEFINE_H__
#define __TBL_FEA_FDB_DEFINE_H__

/* TBL_FEA_FDB field defines */
typedef enum
{
    TBL_FEA_FDB_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_FDB_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_FEA_FDB_FLD_MLAG_SOURCE_IFINDEX  = 2 ,  /* READ */
    TBL_FEA_FDB_FLD_PORTID               = 3 ,  /* READ */
    TBL_FEA_FDB_FLD_FLAGS                = 4 ,  /* READ */
    TBL_FEA_FDB_FLD_FEA_FAIL             = 5 ,  /* READ */
    TBL_FEA_FDB_FLD_MAX                  = 6 
} tbl_fea_fdb_field_id_t;

/* TBL_FEA_FDB defines */
typedef fdb_key_t tbl_fea_fdb_key_t;

typedef struct tbl_fea_fdb_s
{
    tbl_fea_fdb_key_t    key;
    uint32               ifindex;             /* out interface */
    uint32               mlag_source_ifindex; /* store previous MLAG interface after protect-switch */
    uint64               portid;
    uint32               flags;
    uint32               fea_fail;
    CTCLIB_TAILQ_ENTRY(tbl_fea_fdb_s) port_entry;
    CTCLIB_TAILQ_ENTRY(tbl_fea_fdb_s) vlan_entry;
} tbl_fea_fdb_t;

typedef CTCLIB_TAILQ_HEAD(fea_fdb_tailq_head_s, tbl_fea_fdb_s) fea_fdb_tailq_head_t;

typedef struct
{
    ctclib_hash_t        *fdb_hash;
} tbl_fea_fdb_master_t;

#endif /* !__TBL_FEA_FDB_DEFINE_H__ */

