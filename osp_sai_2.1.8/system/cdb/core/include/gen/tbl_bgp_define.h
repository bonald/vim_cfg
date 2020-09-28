
#ifndef __TBL_BGP_DEFINE_H__
#define __TBL_BGP_DEFINE_H__

/* TBL_BGP field defines */
typedef enum
{
    TBL_BGP_FLD_KEY                  = 0 ,  /* READ */
    TBL_BGP_FLD_ROUTER_ID            = 1 ,  /* READ */
    TBL_BGP_FLD_REDISTRIBUTE         = 2 ,  /* READ */
    TBL_BGP_FLD_RED_METRIC           = 3 ,  /* READ */
    TBL_BGP_FLD_MAX                  = 4 
} tbl_bgp_field_id_t;

/* TBL_BGP defines */
typedef struct
{
    uint32               as_id;
} tbl_bgp_key_t;

typedef struct
{
    tbl_bgp_key_t        key;
    addr_ipv4_t          router_id;
    uint32               redistribute;
    uint32               red_metric[RT_ROUTE_MAX];
} tbl_bgp_t;

typedef struct
{
    ctclib_hash_t        *bgp_hash;
    ctclib_slist_t       *bgp_list;
} tbl_bgp_master_t;

#endif /* !__TBL_BGP_DEFINE_H__ */

