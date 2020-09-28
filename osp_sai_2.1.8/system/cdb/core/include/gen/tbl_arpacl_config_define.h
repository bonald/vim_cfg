
#ifndef __TBL_ARPACL_CONFIG_DEFINE_H__
#define __TBL_ARPACL_CONFIG_DEFINE_H__

/* TBL_ARPACL_CONFIG field defines */
typedef enum
{
    TBL_ARPACL_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_ARPACL_CONFIG_FLD_SEQ_REF              = 1 ,  /* RW */
    TBL_ARPACL_CONFIG_FLD_MAX                  = 2 
} tbl_arpacl_config_field_id_t;

/* TBL_ARPACL_CONFIG defines */
typedef struct
{
    char                 name[ACL_NAME_FULL_NAME_SIZE+1];
} tbl_arpacl_config_key_t;

typedef struct
{
    tbl_arpacl_config_key_t key;
    uint32               seq_ref;             /*the sequence_num of arpace config*/
} tbl_arpacl_config_t;

typedef struct
{
    ctclib_hash_t        *arpacl_config_hash;
    ctclib_slist_t       *arpacl_config_list;
} tbl_arpacl_config_master_t;

#endif /* !__TBL_ARPACL_CONFIG_DEFINE_H__ */

