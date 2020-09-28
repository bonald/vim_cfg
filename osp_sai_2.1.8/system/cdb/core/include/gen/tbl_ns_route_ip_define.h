
#ifndef __TBL_NS_ROUTE_IP_DEFINE_H__
#define __TBL_NS_ROUTE_IP_DEFINE_H__

/* TBL_NS_ROUTE_IP field defines */
typedef enum
{
    TBL_NS_ROUTE_IP_FLD_KEY                  = 0 ,  /* READ */
    TBL_NS_ROUTE_IP_FLD_IS_INBAND            = 1 ,  /* RW */
    TBL_NS_ROUTE_IP_FLD_CNT                  = 2 ,  /* RW */
    TBL_NS_ROUTE_IP_FLD_MAX                  = 3 
} tbl_ns_route_ip_field_id_t;

/* TBL_NS_ROUTE_IP defines */
typedef struct
{
    addr_t               ip;
} tbl_ns_route_ip_key_t;

typedef struct
{
    tbl_ns_route_ip_key_t key;
    uint32               is_inband;
    int32                cnt;                 /* used cnt */
} tbl_ns_route_ip_t;

typedef struct
{
    ctclib_hash_t        *ns_route_ip_hash;
    ctclib_slist_t       *ns_route_ip_list;
} tbl_ns_route_ip_master_t;

#endif /* !__TBL_NS_ROUTE_IP_DEFINE_H__ */

