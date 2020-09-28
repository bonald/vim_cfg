
#ifndef __TBL_FEA_ACL_TABLE_DEFINE_H__
#define __TBL_FEA_ACL_TABLE_DEFINE_H__

/* TBL_FEA_ACL_TABLE field defines */
typedef enum
{
    TBL_FEA_ACL_TABLE_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_ACL_TABLE_FLD_DIRECT_IN            = 1 ,  /* RW */
    TBL_FEA_ACL_TABLE_FLD_PRIORITY             = 2 ,  /* RW */
    TBL_FEA_ACL_TABLE_FLD_TABLE_NAME           = 3 ,  /* RW */
    TBL_FEA_ACL_TABLE_FLD_SAI_TABLE_ID         = 4 ,  /* RW */
    TBL_FEA_ACL_TABLE_FLD_MAX                  = 5 
} tbl_fea_acl_table_field_id_t;

/* TBL_FEA_ACL_TABLE defines */
typedef struct
{
    uint64               acl_table_id;
} tbl_fea_acl_table_key_t;

typedef struct
{
    tbl_fea_acl_table_key_t key;
    uint32               direct_in;
    uint32               priority;
    char                 table_name[ACL_NAME_FULL_NAME_SIZE+1];
    uint64               sai_table_id;
} tbl_fea_acl_table_t;

typedef struct
{
    ctclib_hash_t        *fea_acl_table_hash;
} tbl_fea_acl_table_master_t;

#endif /* !__TBL_FEA_ACL_TABLE_DEFINE_H__ */

