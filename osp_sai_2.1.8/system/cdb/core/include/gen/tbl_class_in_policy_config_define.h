
#ifndef __TBL_CLASS_IN_POLICY_CONFIG_DEFINE_H__
#define __TBL_CLASS_IN_POLICY_CONFIG_DEFINE_H__

/* TBL_CLASS_IN_POLICY_CONFIG field defines */
typedef enum
{
    TBL_CLASS_IN_POLICY_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_CLASS_IN_POLICY_CONFIG_FLD_CLASS_PRIORITY       = 1 ,  /* RW */
    TBL_CLASS_IN_POLICY_CONFIG_FLD_MAX                  = 2 
} tbl_class_in_policy_config_field_id_t;

/* TBL_CLASS_IN_POLICY_CONFIG defines */
typedef class_in_policy_config_key_t tbl_class_in_policy_config_key_t;

typedef struct
{
    tbl_class_in_policy_config_key_t key;
    uint64               class_priority;
} tbl_class_in_policy_config_t;

typedef struct
{
    ctclib_hash_t        *class_in_policy_config_hash;
    ctclib_slist_t       *class_in_policy_config_list;
} tbl_class_in_policy_config_master_t;

#endif /* !__TBL_CLASS_IN_POLICY_CONFIG_DEFINE_H__ */

