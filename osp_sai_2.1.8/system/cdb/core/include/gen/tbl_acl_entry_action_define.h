
#ifndef __TBL_ACL_ENTRY_ACTION_DEFINE_H__
#define __TBL_ACL_ENTRY_ACTION_DEFINE_H__

/* TBL_ACL_ENTRY_ACTION field defines */
typedef enum
{
    TBL_ACL_ENTRY_ACTION_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG          = 1 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_DENY                 = 2 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_STATS_ENALE          = 3 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_POLICER              = 4 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_NEXTHOP_GROUP        = 5 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_LOGEN                = 6 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_TC                   = 7 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_COLOR                = 8 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_DSCP                 = 9 ,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_SNAT_CANCEL          = 10,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID           = 11,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID         = 12,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID         = 13,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS             = 14,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS             = 15,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX = 16,  /* RW */
    TBL_ACL_ENTRY_ACTION_FLD_MAX                  = 17
} tbl_acl_entry_action_field_id_t;

/* TBL_ACL_ENTRY_ACTION defines */
typedef struct
{
    uint64               aclid;
} tbl_acl_entry_action_key_t;

typedef struct
{
    tbl_acl_entry_action_key_t key;
    uint32               action_flag;
    uint32               deny;
    uint32               stats_enale;
    char                 policer[POLICER_NAME_SIZE];
    uint32               nexthop_group;
    uint32               logen;
    uint32               tc;
    uint32               color;
    uint8                dscp;
    uint8                snat_cancel;
    uint32               session_id;
    uint32               new_svlan_id;
    uint32               new_cvlan_id;
    uint8                new_scos;
    uint8                new_ccos;
    uint32               redirect_port_ifindex;
} tbl_acl_entry_action_t;

typedef struct
{
    ctclib_hash_t        *acl_entry_action_hash;
} tbl_acl_entry_action_master_t;

#endif /* !__TBL_ACL_ENTRY_ACTION_DEFINE_H__ */

