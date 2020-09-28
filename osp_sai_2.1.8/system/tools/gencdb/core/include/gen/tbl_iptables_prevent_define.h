
#ifndef __TBL_IPTABLES_PREVENT_DEFINE_H__
#define __TBL_IPTABLES_PREVENT_DEFINE_H__

/* TBL_IPTABLES_PREVENT field defines */
typedef enum
{
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP         = 0 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF        = 1 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE      = 2 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP          = 3 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP          = 4 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT    = 5 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ        = 6 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ         = 7 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT      = 8 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT       = 9 ,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT       = 10,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH     = 11,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE = 12,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE = 13,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE = 14,  /* RW */
    TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT           = 15,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT          = 16,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT        = 17,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_UDP_COUNT            = 18,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_TCP_COUNT            = 19,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT      = 20,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT      = 21,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT     = 22,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT   = 23,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT       = 24,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT       = 25,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT = 26,  /* READ */
    TBL_IPTABLES_PREVENT_FLD_MAX                  = 27
} tbl_iptables_prevent_field_id_t;

/* TBL_IPTABLES_PREVENT defines */
typedef struct
{
    uint32               flags;               /* bitmap of GLB_IPTABLES_CFG_FLAG_ */
    uint16               icmp_rate_limit;
    uint16               tcp_rate_limit;
    uint16               udp_rate_limit;
    uint16               small_pkt_length;
    uint32               pkt_statistic_update;
    uint32               small_pkt_static_base;
    uint32               small_pkt_mgmt_static_base;
    uint32               icmp_count;
    uint32               smurf_count;
    uint32               fraggle_count;
    uint32               udp_count;
    uint32               tcp_count;
    uint32               small_pkt_count;
    uint32               icmp_mgmt_count;
    uint32               smurf_mgmt_count;
    uint32               fraggle_mgmt_count;
    uint32               udp_mgmt_count;
    uint32               tcp_mgmt_count;
    uint32               small_pkt_mgmt_count;
} tbl_iptables_prevent_t;

#endif /* !__TBL_IPTABLES_PREVENT_DEFINE_H__ */

