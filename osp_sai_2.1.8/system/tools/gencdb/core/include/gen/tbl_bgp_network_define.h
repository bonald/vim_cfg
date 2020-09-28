
#ifndef __TBL_BGP_NETWORK_DEFINE_H__
#define __TBL_BGP_NETWORK_DEFINE_H__

/* TBL_BGP_NETWORK field defines */
typedef enum
{
    TBL_BGP_NETWORK_FLD_KEY                  = 0 ,  /* READ */
    TBL_BGP_NETWORK_FLD_MASK                 = 1 ,  /* READ */
    TBL_BGP_NETWORK_FLD_MAX                  = 2 
} tbl_bgp_network_field_id_t;

/* TBL_BGP_NETWORK defines */
typedef addr_t tbl_bgp_network_key_t;

typedef struct
{
    tbl_bgp_network_key_t key;
    addr_ipv4_t          mask;
} tbl_bgp_network_t;

typedef struct
{
    ctclib_hash_t        *bgp_network_hash;
    ctclib_slist_t       *bgp_network_list;
} tbl_bgp_network_master_t;

#endif /* !__TBL_BGP_NETWORK_DEFINE_H__ */

