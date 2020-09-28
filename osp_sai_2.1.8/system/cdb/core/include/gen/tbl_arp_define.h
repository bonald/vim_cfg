
#ifndef __TBL_ARP_DEFINE_H__
#define __TBL_ARP_DEFINE_H__

#include "gen/ds_static_route_dup_ipv4_nh_define.h"

/* TBL_ARP field defines */
typedef enum
{
    TBL_ARP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ARP_FLD_TYPE                 = 1 ,  /* RW */
    TBL_ARP_FLD_IFNAME               = 2 ,  /* RW */
    TBL_ARP_FLD_VRF_ID               = 3 ,  /* RW */
    TBL_ARP_FLD_MAC_ADDR             = 4 ,  /* RW */
    TBL_ARP_FLD_RIF_ID               = 5 ,  /* RW */
    TBL_ARP_FLD_IFINDEX              = 6 ,  /* READ */
    TBL_ARP_FLD_FLAGS                = 7 ,  /* READ */
    TBL_ARP_FLD_STATUS               = 8 ,  /* READ */
    TBL_ARP_FLD_ROUTE_REF            = 9 ,  /* READ */
    TBL_ARP_FLD_UPTIME               = 10,  /* READ */
    TBL_ARP_FLD_AGING_DELAY          = 11,  /* READ */
    TBL_ARP_FLD_IS_GRATUITOUS        = 12,  /* READ */
    TBL_ARP_FLD_IS_PROXY             = 13,  /* READ */
    TBL_ARP_FLD_IS_ROUTER            = 14,  /* READ */
    TBL_ARP_FLD_RETRY_COUNT          = 15,  /* READ */
    TBL_ARP_FLD_IS_BFD_REF           = 16,  /* READ */
    TBL_ARP_FLD_IS_STATIC_ROUTE_DUP  = 17,  /* READ */
    TBL_ARP_FLD_STATIC_ROUTE_DUP_IPV4_NH = 18,  /* SUB */
    TBL_ARP_FLD_RETRY_TIMER          = 19,  /* READ */
    TBL_ARP_FLD_REFRESH_TIMER        = 20,  /* READ */
    TBL_ARP_FLD_GRA_RSP_TIMER        = 21,  /* READ */
    TBL_ARP_FLD_MAX                  = 22
} tbl_arp_field_id_t;

/* TBL_ARP defines */
typedef struct
{
    addr_ipv4_t          ip;
} tbl_arp_key_t;

typedef struct
{
    tbl_arp_key_t        key;
    uint8                type;                /* arp_type_t */
    char                 ifname[IFNAME_SIZE];
    uint8                vrf_id;
    mac_addr_t           mac_addr;
    uint64               rif_id;
    uint32               ifindex;
    uint32               flags;
    uint8                status;
    uint32               route_ref;
    sal_time_t           uptime;
    uint32               aging_delay;
    uint32               is_gratuitous;       /* if entry is due to gratuitous ARP Request */
    uint32               is_proxy;
    uint32               is_router;
    uint32               retry_count;
    uint32               is_bfd_ref;
    uint8                *rx_buf;             /* receive ARP packet buf */
    uint32               is_static_route_dup;
    cdb_reference_list_t static_route_dup_ipv4_nh;
    ctc_task_t           *retry_timer;
    ctc_task_t           *refresh_timer;
    ctc_task_t           *gra_rsp_timer;
} tbl_arp_t;

typedef struct
{
    ctclib_hash_t        *arp_hash;
    ctclib_slist_t       *arp_list;
} tbl_arp_master_t;

#endif /* !__TBL_ARP_DEFINE_H__ */

