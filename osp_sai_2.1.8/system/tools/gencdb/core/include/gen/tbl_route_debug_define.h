
#ifndef __TBL_ROUTE_DEBUG_DEFINE_H__
#define __TBL_ROUTE_DEBUG_DEFINE_H__

/* TBL_ROUTE_DEBUG field defines */
typedef enum
{
    TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4           = 0 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6           = 1 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK        = 2 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARP_EVENT            = 3 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARP_PACKET           = 4 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL         = 5 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARP_TIMER            = 6 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT  = 7 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET = 8 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL = 9 ,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER  = 10,  /* RW */
    TBL_ROUTE_DEBUG_FLD_VRRP_ALL             = 11,  /* RW */
    TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS          = 12,  /* RW */
    TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND     = 13,  /* RW */
    TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV     = 14,  /* RW */
    TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL   = 15,  /* RW */
    TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT          = 16,  /* RW */
    TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET         = 17,  /* RW */
    TBL_ROUTE_DEBUG_FLD_TRACK_EVENT          = 18,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ND_EVENT             = 19,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ND_PACKET            = 20,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL          = 21,  /* RW */
    TBL_ROUTE_DEBUG_FLD_ND_TIMER             = 22,  /* RW */
    TBL_ROUTE_DEBUG_FLD_NAT_EVENT            = 23,  /* RW */
    TBL_ROUTE_DEBUG_FLD_NAT_TIMER            = 24,  /* RW */
    TBL_ROUTE_DEBUG_FLD_PIMSM_ALL            = 25,  /* RW */
    TBL_ROUTE_DEBUG_FLD_PIMSM_PIM            = 26,  /* RW */
    TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP           = 27,  /* RW */
    TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER          = 28,  /* RW */
    TBL_ROUTE_DEBUG_FLD_MAX                  = 29
} tbl_route_debug_field_id_t;

/* TBL_ROUTE_DEBUG defines */
typedef struct
{
    uint32               route;               /* bitmap of RTDBG_FLAG_ROUTE_ */
    uint32               arp;                 /* bitmap of RTDBG_FLAG_ARP_ */
    uint32               arpinspection;       /* bitmap of RTDBG_FLAG_ARPINSPECTION_ */
    uint32               vrrp;                /* bitmap of RTDBG_FLAG_VRRP_ */
    uint32               ipsla;               /* bitmap of RTDBG_FLAG_IPSLA_ */
    uint32               track;               /* bitmap of RTDBG_FLAG_TRACK_ */
    uint32               nd;                  /* bitmap of RTDBG_FLAG_ND_ */
    uint32               nat;                 /* bitmap of RTDBG_FLAG_NAT_ */
    uint32               pimsm;               /* bitmap of RTDBG_FLAG_PIMSM_ */
} tbl_route_debug_t;

#endif /* !__TBL_ROUTE_DEBUG_DEFINE_H__ */

