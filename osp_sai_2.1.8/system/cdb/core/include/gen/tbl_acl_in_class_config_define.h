
#ifndef __TBL_ACL_IN_CLASS_CONFIG_DEFINE_H__
#define __TBL_ACL_IN_CLASS_CONFIG_DEFINE_H__

/* TBL_ACL_IN_CLASS_CONFIG field defines */
typedef enum
{
    TBL_ACL_IN_CLASS_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY         = 1 ,  /* RW */
    TBL_ACL_IN_CLASS_CONFIG_FLD_MAX                  = 2 
} tbl_acl_in_class_config_field_id_t;

/* TBL_ACL_IN_CLASS_CONFIG defines */
typedef acl_in_class_config_key_t tbl_acl_in_class_config_key_t;

typedef struct
{
    tbl_acl_in_class_config_key_t key;
    uint64               acl_priority;
} tbl_acl_in_class_config_t;

typedef struct
{
    ctclib_hash_t        *acl_in_class_config_hash;
    ctclib_slist_t       *acl_in_class_config_list;
} tbl_acl_in_class_config_master_t;

#endif /* !__TBL_ACL_IN_CLASS_CONFIG_DEFINE_H__ */

