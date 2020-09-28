
#ifndef __TBL_BGP_NEIGHBOR_DEFINE_H__
#define __TBL_BGP_NEIGHBOR_DEFINE_H__

/* TBL_BGP_NEIGHBOR field defines */
typedef enum
{
    TBL_BGP_NEIGHBOR_FLD_KEY                  = 0 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_REMOTE_AS            = 1 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP        = 2 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_PASSWORD             = 3 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_INTF_NAME            = 4 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_INTF_ADDR            = 5 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_SHUTDOWN             = 6 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF        = 7 ,  /* READ */
    TBL_BGP_NEIGHBOR_FLD_MAX                  = 8 
} tbl_bgp_neighbor_field_id_t;

/* TBL_BGP_NEIGHBOR defines */
typedef addr_t tbl_bgp_neighbor_key_t;

typedef struct
{
    tbl_bgp_neighbor_key_t key;
    uint32               remote_as;
    uint32               ebgp_multihop;
    char                 password[32];
    char                 intf_name[IFNAME_SIZE];
    addr_ipv4_t          intf_addr;
    uint32               shutdown;
    uint32               next_hop_self;
} tbl_bgp_neighbor_t;

typedef struct
{
    ctclib_hash_t        *bgp_neighbor_hash;
    ctclib_slist_t       *bgp_neighbor_list;
} tbl_bgp_neighbor_master_t;

#endif /* !__TBL_BGP_NEIGHBOR_DEFINE_H__ */

