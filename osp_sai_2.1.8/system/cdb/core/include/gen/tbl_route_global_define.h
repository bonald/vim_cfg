
#ifndef __TBL_ROUTE_GLOBAL_DEFINE_H__
#define __TBL_ROUTE_GLOBAL_DEFINE_H__

/* TBL_ROUTE_GLOBAL field defines */
typedef enum
{
    TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN = 0 ,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT     = 1 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT     = 2 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT = 3 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARP_COUNT            = 4 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_MULTIPATH_NUM        = 5 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_MAX_STATIC           = 6 ,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ICMP_ERROR_RATELIMIT = 7 ,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN = 8 ,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_ROUTES = 9 ,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_ROUTES  = 10,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_GROUPS  = 11,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_ROUTES = 12,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_HOST_ROUTES  = 13,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV4_ROUTE_NUM = 14,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_STATIC_V6_ROUTES = 15,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_ROUTES = 16,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_ECMP_V6_GROUPS = 17,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_REMOTE_V6_ROUTES = 18,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_CURRENT_INDIRECT_IPV6_ROUTE_NUM = 19,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_LOCAL         = 20,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_STATIC        = 21,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_CONNECTED     = 22,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_RIP           = 23,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_OSPF          = 24,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ACTIVE_BGP           = 25,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_ENTRYNUM = 26,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_CURNUM = 27,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGNUM = 28,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARPINSP_LOGBUF_LOGSEC = 29,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARPINSP_VALIDATE_FLAG = 30,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_FIB_FULL             = 31,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE          = 32,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT         = 33,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED        = 34,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_VTY_NONCONN_COUNT    = 35,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_VTY_MODE             = 36,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARP_AGING_TIMER      = 37,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER   = 38,  /* READ */
    TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_SEC = 39,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ARPRATELIMIT_TIMER_START_TIME_MSEC = 40,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_BUF                  = 41,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_PKT_LEN              = 42,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_ND_COUNT             = 43,  /* RW */
    TBL_ROUTE_GLOBAL_FLD_MAX                  = 44
} tbl_route_global_field_id_t;

/* TBL_ROUTE_GLOBAL defines */
typedef struct
{
    uint32               gratuitous_arp_learn_en;
    uint32               arp_pkt_rx_count;
    uint32               arp_pkt_tx_count;
    uint32               arp_pkt_discard_count;
    uint32               arp_count;
    uint32               multipath_num;
    uint32               max_static;
    uint32               icmp_error_ratelimit;
    uint8                ip_redirects_global_en;
    uint32               current_static_routes;
    uint32               current_ecmp_routes; /*maybe not used*/
    uint32               current_ecmp_groups; /*regardless interface up down*/
    uint32               current_remote_routes;
    uint32               current_host_routes;
    uint32               current_indirect_ipv4_route_num; /*regardless interface up down*/
    uint32               current_static_v6_routes; /*ipv6*/
    uint32               current_ecmp_v6_routes; /*ipv6*/
    uint32               current_ecmp_v6_groups; /*ipv6*//*regardless interface up down*/
    uint32               current_remote_v6_routes; /*ipv6*/
    uint32               current_indirect_ipv6_route_num; /*regardless interface up down*/
    uint32               active_local;
    uint32               active_static;
    uint32               active_connected;
    uint32               active_rip;
    uint32               active_ospf;
    uint32               active_bgp;
    uint32               arpinsp_logbuf_entrynum;
    uint32               arpinsp_logbuf_curnum;
    uint32               arpinsp_logbuf_lognum;
    uint32               arpinsp_logbuf_logsec;
    uint32               arpinsp_validate_flag;
    uint32               fib_full;
    uint32               ipv6_enable;
    uint32               nd_hop_limit;
    uint32               vty_connected;       /* vty connected flag */
    uint32               vty_nonconn_count;   /* vty non-connected cmd count */
    uint32               vty_mode;            /* glb_vty_mode_t */
    ctc_task_t           *arp_aging_timer;
    ctc_task_t           *arpratelimit_timer;
    sal_time_t           arpratelimit_timer_start_time_sec;
    sal_time_t           arpratelimit_timer_start_time_msec;
    uint8                buf[GLB_ND_PKT_BUF]; /* nd packet buffer */
    uint32               pkt_len;
    uint32               nd_count;
} tbl_route_global_t;

#endif /* !__TBL_ROUTE_GLOBAL_DEFINE_H__ */

