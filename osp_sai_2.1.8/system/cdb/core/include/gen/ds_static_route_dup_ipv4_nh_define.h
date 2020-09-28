
#ifndef __DS_STATIC_ROUTE_DUP_IPV4_NH_DEFINE_H__
#define __DS_STATIC_ROUTE_DUP_IPV4_NH_DEFINE_H__

/* DS_STATIC_ROUTE_DUP_IPV4_NH field defines */
typedef enum
{
    DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_KEY                  = 0 ,  /* READ */
    DS_STATIC_ROUTE_DUP_IPV4_NH_FLD_MAX                  = 1 
} ds_static_route_dup_ipv4_nh_field_id_t;

/* DS_STATIC_ROUTE_DUP_IPV4_NH defines */
typedef struct
{
    addr_ipv4_t          address;
} ds_static_route_dup_ipv4_nh_key_t;

typedef struct
{
    ds_static_route_dup_ipv4_nh_key_t key;
} ds_static_route_dup_ipv4_nh_t;

#endif /* !__DS_STATIC_ROUTE_DUP_IPV4_NH_DEFINE_H__ */

