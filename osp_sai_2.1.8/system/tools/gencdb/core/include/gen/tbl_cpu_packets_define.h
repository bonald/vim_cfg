
#ifndef __TBL_CPU_PACKETS_DEFINE_H__
#define __TBL_CPU_PACKETS_DEFINE_H__

/* TBL_CPU_PACKETS field defines */
typedef enum
{
    TBL_CPU_PACKETS_FLD_TOTAL                = 0 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX                   = 1 ,  /* RW */
    TBL_CPU_PACKETS_FLD_TX                   = 2 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_BPDU              = 3 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_SLOW_PROTO        = 4 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_EAPOL             = 5 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_LLDP              = 6 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_ERPS              = 7 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_MACDA             = 8 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_RIP               = 9 ,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_OSPF              = 10,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_BGP               = 11,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_ARP               = 12,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_DHCP              = 13,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_IPDA              = 14,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_IGMP              = 15,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_MAC_LIMIT         = 16,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_MAC_MISMATCH      = 17,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_L3COPY_CPU        = 18,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_TTL_ERROR         = 19,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_PTP               = 20,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_TPOAM             = 21,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_L2PRO_PROTOCOL_MAC = 22,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_L2PRO_GROUP_MAC   = 23,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_CPU_MIRROR        = 24,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_ND                = 25,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_PACKETIN          = 26,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_MTU_FAIL          = 27,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_G8032             = 28,  /* RW */
    TBL_CPU_PACKETS_FLD_RX_OTHER             = 29,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_IGMP              = 30,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_BPDU              = 31,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_SLOW_PROTO        = 32,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_ARP               = 33,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_DHCP              = 34,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_LLDP              = 35,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_ERPS              = 36,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_EAPOL             = 37,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_PTP               = 38,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_TPOAM             = 39,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_L2PRO_PROTOCOL_MAC = 40,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_L2PRO_GROUP_MAC   = 41,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_CPU_MIRROR        = 42,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_ND                = 43,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_G8032             = 44,  /* RW */
    TBL_CPU_PACKETS_FLD_TX_OTHER             = 45,  /* RW */
    TBL_CPU_PACKETS_FLD_MAX                  = 46
} tbl_cpu_packets_field_id_t;

/* TBL_CPU_PACKETS defines */
typedef struct
{
    uint64               total;
    uint64               rx;
    uint64               tx;
    uint64               rx_bpdu;
    uint64               rx_slow_proto;
    uint64               rx_eapol;
    uint64               rx_lldp;
    uint64               rx_erps;
    uint64               rx_macda;
    uint64               rx_rip;
    uint64               rx_ospf;
    uint64               rx_bgp;
    uint64               rx_arp;
    uint64               rx_dhcp;
    uint64               rx_ipda;
    uint64               rx_igmp;
    uint64               rx_mac_limit;
    uint64               rx_mac_mismatch;
    uint64               rx_l3copy_cpu;
    uint64               rx_ttl_error;
    uint64               rx_ptp;
    uint64               rx_tpoam;
    uint64               rx_l2pro_protocol_mac;
    uint64               rx_l2pro_group_mac;
    uint64               rx_cpu_mirror;
    uint64               rx_nd;
    uint64               rx_packetin;
    uint64               rx_mtu_fail;
    uint64               rx_g8032;
    uint64               rx_other;
    uint64               tx_igmp;
    uint64               tx_bpdu;
    uint64               tx_slow_proto;
    uint64               tx_arp;
    uint64               tx_dhcp;
    uint64               tx_lldp;
    uint64               tx_erps;
    uint64               tx_eapol;
    uint64               tx_ptp;
    uint64               tx_tpoam;
    uint64               tx_l2pro_protocol_mac;
    uint64               tx_l2pro_group_mac;
    uint64               tx_cpu_mirror;
    uint64               tx_nd;
    uint64               tx_g8032;
    uint64               tx_other;
} tbl_cpu_packets_t;

#endif /* !__TBL_CPU_PACKETS_DEFINE_H__ */

