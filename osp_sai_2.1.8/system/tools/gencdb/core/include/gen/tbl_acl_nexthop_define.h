
#ifndef __TBL_ACL_NEXTHOP_DEFINE_H__
#define __TBL_ACL_NEXTHOP_DEFINE_H__

/* TBL_ACL_NEXTHOP field defines */
typedef enum
{
    TBL_ACL_NEXTHOP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_NEXTHOP_FLD_EDIT_FLAGS           = 1 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_PORT                 = 2 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_SRC_MAC              = 3 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_DST_MAC              = 4 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_VLAN                 = 5 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_SRC_IPV4             = 6 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_DST_IPV4             = 7 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_DSCP                 = 8 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_L4_SRC_PORT          = 9 ,  /* RW */
    TBL_ACL_NEXTHOP_FLD_L4_DST_PORT          = 10,  /* RW */
    TBL_ACL_NEXTHOP_FLD_REF                  = 11,  /* RW */
    TBL_ACL_NEXTHOP_FLD_MAX                  = 12
} tbl_acl_nexthop_field_id_t;

/* TBL_ACL_NEXTHOP defines */
typedef struct
{
    uint32               acl_nexthop_id;
} tbl_acl_nexthop_key_t;

typedef struct
{
    tbl_acl_nexthop_key_t key;
    uint32               edit_flags;
    uint32               port;
    mac_addr_t           src_mac;
    mac_addr_t           dst_mac;
    uint32               vlan;
    addr_ipv4_t          src_ipv4;
    addr_ipv4_t          dst_ipv4;
    uint32               dscp;
    uint32               l4_src_port;
    uint32               l4_dst_port;
    uint32               ref;
} tbl_acl_nexthop_t;

typedef struct
{
    ctclib_hash_t        *acl_nexthop_hash;
} tbl_acl_nexthop_master_t;

#endif /* !__TBL_ACL_NEXTHOP_DEFINE_H__ */

