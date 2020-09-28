
#ifndef __TBL_CLEAR_ACL_POLICY_DEFINE_H__
#define __TBL_CLEAR_ACL_POLICY_DEFINE_H__

/* TBL_CLEAR_ACL_POLICY field defines */
typedef enum
{
    TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID           = 0 ,  /* RW */
    TBL_CLEAR_ACL_POLICY_FLD_ACL_ID               = 1 ,  /* RW */
    TBL_CLEAR_ACL_POLICY_FLD_MAX                  = 2 
} tbl_clear_acl_policy_field_id_t;

/* TBL_CLEAR_ACL_POLICY defines */
typedef struct
{
    uint64               counter_id;
    uint64               acl_id;
} tbl_clear_acl_policy_t;

#endif /* !__TBL_CLEAR_ACL_POLICY_DEFINE_H__ */

