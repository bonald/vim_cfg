
#ifndef __TBL_ARP_FIB_DEFINE_H__
#define __TBL_ARP_FIB_DEFINE_H__

/* TBL_ARP_FIB field defines */
typedef enum
{
    TBL_ARP_FIB_FLD_KEY                  = 0 ,  /* READ */
    TBL_ARP_FIB_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_ARP_FIB_FLD_TYPE                 = 2 ,  /* READ */
    TBL_ARP_FIB_FLD_VRF_ID               = 3 ,  /* READ */
    TBL_ARP_FIB_FLD_IFNAME               = 4 ,  /* RW */
    TBL_ARP_FIB_FLD_FLAGS                = 5 ,  /* READ */
    TBL_ARP_FIB_FLD_MAC_ADDR             = 6 ,  /* READ */
    TBL_ARP_FIB_FLD_IS_ROUTER            = 7 ,  /* READ */
    TBL_ARP_FIB_FLD_ROUTE_REF            = 8 ,  /* READ */
    TBL_ARP_FIB_FLD_MAX                  = 9 
} tbl_arp_fib_field_id_t;

/* TBL_ARP_FIB defines */
typedef struct
{
    addr_ipv4_t          ip;
} tbl_arp_fib_key_t;

typedef struct
{
    tbl_arp_fib_key_t    key;
    uint32               ifindex;
    uint8                type;                /* arp_type_t */
    uint32               vrf_id;
    char                 ifname[IFNAME_SIZE];
    uint32               flags;
    mac_addr_t           mac_addr;
    uint32               is_router;
    uint32               route_ref;
} tbl_arp_fib_t;

typedef struct
{
    ctclib_hash_t        *arp_fib_hash;
} tbl_arp_fib_master_t;

#endif /* !__TBL_ARP_FIB_DEFINE_H__ */

