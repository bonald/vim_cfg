
#ifndef __TBL_ACL_NEXTHOP_GROUP_DEFINE_H__
#define __TBL_ACL_NEXTHOP_GROUP_DEFINE_H__

/* TBL_ACL_NEXTHOP_GROUP field defines */
typedef enum
{
    TBL_ACL_NEXTHOP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_NEXTHOP_GROUP_FLD_ACL_NEXTHOP_KEY      = 1 ,  /* RW */
    TBL_ACL_NEXTHOP_GROUP_FLD_MAX                  = 2 
} tbl_acl_nexthop_group_field_id_t;

/* TBL_ACL_NEXTHOP_GROUP defines */
typedef struct
{
    uint32               nexthop_group;
} tbl_acl_nexthop_group_key_t;

typedef struct
{
    tbl_acl_nexthop_group_key_t key;
    uint32               acl_nexthop_key[MAX_MEMBER];
} tbl_acl_nexthop_group_t;

typedef struct
{
    ctclib_hash_t        *acl_nexthop_group_hash;
} tbl_acl_nexthop_group_master_t;

#endif /* !__TBL_ACL_NEXTHOP_GROUP_DEFINE_H__ */

