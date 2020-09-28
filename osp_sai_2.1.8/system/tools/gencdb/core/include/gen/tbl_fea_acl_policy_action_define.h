
#ifndef __TBL_FEA_ACL_POLICY_ACTION_DEFINE_H__
#define __TBL_FEA_ACL_POLICY_ACTION_DEFINE_H__

/* TBL_FEA_ACL_POLICY_ACTION field defines */
typedef enum
{
    TBL_FEA_ACL_POLICY_ACTION_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CLASS_GET_POLICER_STATS = 1 ,  /* READ */
    TBL_FEA_ACL_POLICY_ACTION_FLD_FLAGS_CALSS_CLEAR_POLICER_STATS = 2 ,  /* READ */
    TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID           = 3 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_NAME         = 4 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_POLICER_ID_REF       = 5 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_PACKET         = 6 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_GREEN_BYTE           = 7 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_PACKET        = 8 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_YELLOW_BYTE          = 9 ,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_RED_PACKET           = 10,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_RED_BYTE             = 11,  /* RW */
    TBL_FEA_ACL_POLICY_ACTION_FLD_MAX                  = 12
} tbl_fea_acl_policy_action_field_id_t;

/* TBL_FEA_ACL_POLICY_ACTION defines */
typedef struct
{
    uint64               acl_policy_action_id;
} tbl_fea_acl_policy_action_key_t;

typedef struct
{
    tbl_fea_acl_policy_action_key_t key;
    uint32               flags;
    uint64               policer_id;
    char                 policer_name[POLICER_NAME_SIZE];
    uint64               policer_id_ref;
    uint64               green_packet;
    uint64               green_byte;
    uint64               yellow_packet;
    uint64               yellow_byte;
    uint64               red_packet;
    uint64               red_byte;
} tbl_fea_acl_policy_action_t;

typedef struct
{
    ctclib_hash_t        *fea_acl_policy_action_hash;
} tbl_fea_acl_policy_action_master_t;

#endif /* !__TBL_FEA_ACL_POLICY_ACTION_DEFINE_H__ */

