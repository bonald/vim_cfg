
#ifndef __TBL_CLASS_MAP_CONFIG_DEFINE_H__
#define __TBL_CLASS_MAP_CONFIG_DEFINE_H__

/* TBL_CLASS_MAP_CONFIG field defines */
typedef enum
{
    TBL_CLASS_MAP_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_CLASS_MAP_CONFIG_FLD_POLICY_MAP_REF       = 1 ,  /* RW */
    TBL_CLASS_MAP_CONFIG_FLD_ACL_REF              = 2 ,  /* RW */
    TBL_CLASS_MAP_CONFIG_FLD_MAX_ACL_PRIORITY     = 3 ,  /* RW */
    TBL_CLASS_MAP_CONFIG_FLD_MAX                  = 4 
} tbl_class_map_config_field_id_t;

/* TBL_CLASS_MAP_CONFIG defines */
typedef struct
{
    char                 name[CMAP_NAME_SIZE+1];
} tbl_class_map_config_key_t;

typedef struct
{
    tbl_class_map_config_key_t key;
    uint32               policy_map_ref;
    uint32               acl_ref;
    uint64               max_acl_priority;
} tbl_class_map_config_t;

typedef struct
{
    ctclib_hash_t        *class_map_config_hash;
    ctclib_slist_t       *class_map_config_list;
} tbl_class_map_config_master_t;

#endif /* !__TBL_CLASS_MAP_CONFIG_DEFINE_H__ */

