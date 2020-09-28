
#ifndef __TBL_IPROUTE_NODE_DEFINE_H__
#define __TBL_IPROUTE_NODE_DEFINE_H__

/* TBL_IPROUTE_NODE field defines */
typedef enum
{
    TBL_IPROUTE_NODE_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPROUTE_NODE_FLD_ROUTE_TYPE           = 1 ,  /* READ */
    TBL_IPROUTE_NODE_FLD_DISTANCE             = 2 ,  /* READ */
    TBL_IPROUTE_NODE_FLD_MULTIPATH            = 3 ,  /* READ */
    TBL_IPROUTE_NODE_FLD_NH_KEY               = 4 ,  /* RW */
    TBL_IPROUTE_NODE_FLD_NH_GROUP             = 5 ,  /* RW */
    TBL_IPROUTE_NODE_FLD_MAX                  = 6 
} tbl_iproute_node_field_id_t;

/* TBL_IPROUTE_NODE defines */
typedef route_node_key_t tbl_iproute_node_key_t;

typedef struct
{
    tbl_iproute_node_key_t key;
    uint32               route_type;
    uint32               distance;
    uint32               multipath;
    nexthop_key_t        nh_key;
    nexthop_ecmp_t       nh_group;
} tbl_iproute_node_t;

typedef struct
{
    ctclib_hash_t        *rn_hash;
    ctclib_slist_t       *rn_list;
} tbl_iproute_node_master_t;

#endif /* !__TBL_IPROUTE_NODE_DEFINE_H__ */

