
#ifndef __TBL_IPROUTE_NODE_ADD_FAIL_DEFINE_H__
#define __TBL_IPROUTE_NODE_ADD_FAIL_DEFINE_H__

/* TBL_IPROUTE_NODE_ADD_FAIL field defines */
typedef enum
{
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_DISTANCE             = 1 ,  /* READ */
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_MULTIPATH            = 2 ,  /* READ */
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_NH_KEY               = 3 ,  /* RW */
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_NH_GROUP             = 4 ,  /* RW */
    TBL_IPROUTE_NODE_ADD_FAIL_FLD_MAX                  = 5 
} tbl_iproute_node_add_fail_field_id_t;

/* TBL_IPROUTE_NODE_ADD_FAIL defines */
typedef route_node_key_t tbl_iproute_node_add_fail_key_t;

typedef struct
{
    tbl_iproute_node_add_fail_key_t key;
    uint32               distance;
    uint32               multipath;
    nexthop_key_t        nh_key;
    nexthop_ecmp_t       nh_group;
} tbl_iproute_node_add_fail_t;

typedef struct
{
    ctclib_hash_t        *add_fail_hash;
    ctclib_slist_t       *add_fail_list;
} tbl_iproute_node_add_fail_master_t;

#endif /* !__TBL_IPROUTE_NODE_ADD_FAIL_DEFINE_H__ */

