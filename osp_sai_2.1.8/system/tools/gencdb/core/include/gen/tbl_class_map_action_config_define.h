
#ifndef __TBL_CLASS_MAP_ACTION_CONFIG_DEFINE_H__
#define __TBL_CLASS_MAP_ACTION_CONFIG_DEFINE_H__

/* TBL_CLASS_MAP_ACTION_CONFIG field defines */
typedef enum
{
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG          = 1 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC               = 2 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR            = 3 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP             = 4 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID           = 5 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER              = 6 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID         = 7 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID         = 8 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS             = 9 ,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS             = 10,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX = 11,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG      = 12,  /* RW */
    TBL_CLASS_MAP_ACTION_CONFIG_FLD_MAX                  = 13
} tbl_class_map_action_config_field_id_t;

/* TBL_CLASS_MAP_ACTION_CONFIG defines */
typedef class_map_action_config_key_t tbl_class_map_action_config_key_t;

typedef struct
{
    tbl_class_map_action_config_key_t key;
    uint32               action_flag;
    uint32               new_tc;
    int32                new_color;
    uint8                new_dscp;
    uint32               session_id;
    char                 policer[POLICER_NAME_SIZE]; /* policer name */
    uint32               new_svlan_id;
    uint32               new_cvlan_id;
    uint8                new_scos;
    uint8                new_ccos;
    uint32               redirect_port_ifindex;
    char                 red_unexist_agg[IFNAME_SIZE];
} tbl_class_map_action_config_t;

typedef struct
{
    ctclib_hash_t        *class_map_action_config_hash;
    ctclib_slist_t       *class_map_action_config_list;
} tbl_class_map_action_config_master_t;

#endif /* !__TBL_CLASS_MAP_ACTION_CONFIG_DEFINE_H__ */

