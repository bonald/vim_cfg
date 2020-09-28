
#ifndef __TBL_STATIC_ROUTE_CFG_DEFINE_H__
#define __TBL_STATIC_ROUTE_CFG_DEFINE_H__

/* TBL_STATIC_ROUTE_CFG field defines */
typedef enum
{
    TBL_STATIC_ROUTE_CFG_FLD_KEY                  = 0 ,  /* READ */
    TBL_STATIC_ROUTE_CFG_FLD_DISTANCE             = 1 ,  /* READ */
    TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID             = 2 ,  /* RW */
    TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS         = 3 ,  /* RW */
    TBL_STATIC_ROUTE_CFG_FLD_NEXTHOP_TYPE_V6      = 4 ,  /* RW */
    TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT          = 5 ,  /* RW */
    TBL_STATIC_ROUTE_CFG_FLD_MAX                  = 6 
} tbl_static_route_cfg_field_id_t;

/* TBL_STATIC_ROUTE_CFG defines */
typedef route_node_key_t tbl_static_route_cfg_key_t;

typedef struct
{
    tbl_static_route_cfg_key_t key;
    uint32               distance;
    uint16               track_id;            /* track object id */
    uint16               track_status;        /* track object status */
    uint8                nexthop_type_v6;     /*for ipv6*/
    uint8                dhcp_client;
} tbl_static_route_cfg_t;

typedef struct
{
    ctclib_hash_t        *rst_cfg_hash;
    ctclib_slist_t       *rst_cfg_list;
} tbl_static_route_cfg_master_t;

#endif /* !__TBL_STATIC_ROUTE_CFG_DEFINE_H__ */

