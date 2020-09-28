
#ifndef __TBL_POLICY_MAP_CONFIG_DEFINE_H__
#define __TBL_POLICY_MAP_CONFIG_DEFINE_H__

/* TBL_POLICY_MAP_CONFIG field defines */
typedef enum
{
    TBL_POLICY_MAP_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_POLICY_MAP_CONFIG_FLD_INTF_REF             = 1 ,  /* RW */
    TBL_POLICY_MAP_CONFIG_FLD_CLASS_REF            = 2 ,  /* RW */
    TBL_POLICY_MAP_CONFIG_FLD_MAX_CLASS_PRIORITY   = 3 ,  /* RW */
    TBL_POLICY_MAP_CONFIG_FLD_MAX                  = 4 
} tbl_policy_map_config_field_id_t;

/* TBL_POLICY_MAP_CONFIG defines */
typedef struct
{
    char                 name[PMAP_NAME_SIZE+1];
} tbl_policy_map_config_key_t;

typedef struct
{
    tbl_policy_map_config_key_t key;
    uint32               intf_ref;
    uint32               class_ref;
    uint64               max_class_priority;
} tbl_policy_map_config_t;

typedef struct
{
    ctclib_hash_t        *policy_map_config_hash;
    ctclib_slist_t       *policy_map_config_list;
} tbl_policy_map_config_master_t;

#endif /* !__TBL_POLICY_MAP_CONFIG_DEFINE_H__ */

