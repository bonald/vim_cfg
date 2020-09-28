#ifndef __HSRV_DEBUG_H__
#define __HSRV_DEBUG_H__

#include "gen/tbl_hsrv_debug_define.h"
#include "gen/tbl_hsrv_debug.h"

#define HSRV_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}

#define HSRV_LOG_EMERG(fmt, args...)    log_sys(M_MOD_HSRV, E_EMERGENCY, fmt, ##args)
#define HSRV_LOG_ALERT(fmt, args...)    log_sys(M_MOD_HSRV, E_ALERT, fmt, ##args)
#define HSRV_LOG_CRIT(fmt, args...)     log_sys(M_MOD_HSRV, E_CRITICAL, fmt, ##args)
#define HSRV_LOG_ERR(fmt, args...)      log_sys(M_MOD_HSRV, E_ERROR, fmt, ##args)
#define HSRV_LOG_WARN(fmt, args...)     log_sys(M_MOD_HSRV, E_WARNING, fmt, ##args)
#define HSRV_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_HSRV, E_NOTICE, fmt, ##args)
#define HSRV_LOG_INFO(fmt, args...)     log_sys(M_MOD_HSRV, E_INFORMATIONAL, fmt, ##args)

extern tbl_hsrv_debug_t *_g_p_tbl_hsrv_debug;

#define HSRV_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_hsrv_debug->module & HSRVDBG_FLAG_##typeenum)

#define HSRV_DEBUG_PRINT(fmt, args...)   \
    log_sys(M_MOD_HSRV, E_DEBUG, fmt, ##args);

#define HSRV_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(module, typeenum)) \
        HSRV_DEBUG_PRINT(fmt, ##args); \
} while (0)

#define HSRV_SWITCH_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(vswitch, VSWITCH_VSWITCH, fmt, ##args); \
} while(0)

#define HSRV_L2IF_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, INTERFACE_L2IF, fmt, ##args); \
} while(0)

#define HSRV_L3IF_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, INTERFACE_L3IF, fmt, ##args); \
} while(0)

#define HSRV_MSG_PM2HSRV_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(message, MESSAGE_PM2HSRV, fmt, ##args); \
} while(0)

#define HSRV_MSG_HSRV2PM_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(message, MESSAGE_HSRV2PM, fmt, ##args); \
} while(0)

#define HSRV_IP_ICMP_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ip, IP_ICMP, fmt, ##args); \
} while(0)

#define HSRV_CPU_PDU_DEBUG(fmt, args...) \
do { \
    /*HSRV_LOG_DEBUG(cpu, CPU_PDU, fmt, ##args); */\
    if (HSRV_DBG_IS_ON(cpu, CPU_PDU)) \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_RX_DEBUG(fmt, args...) \
do { \
    /*HSRV_LOG_DEBUG(cpu, CPU_TRAFFIC, fmt, ##args);*/ \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RX)) \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_TX_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_TX)) \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_BPDU_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_BPDU))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_SLOW_PROTO_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_SLOW_PROTO))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_EAPOL_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_EAPOL))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_LLDP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_LLDP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_ERPS_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_ERPS))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_MACDA_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_MACDA))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_RIP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RIP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_OSPF_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_OSPF))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_BGP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_BGP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_ARP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_ARP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_DHCP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_DHCP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_IPDA_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_IPDA))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_IGMP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_IGMP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_OTHER_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_OTHER))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_RAW_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RAW))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_PTP_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_PTP))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_MAC_LIMIT_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_MAC_LIMIT))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_MAC_MISMATCH_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_MAC_MISMATCH))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_CPU_TRAFFIC_L3COPY_CPU_DEBUG(fmt, args...) \
do { \
    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_L3COPY_CPU))       \
        LOG_SYS_TS(M_MOD_HSRV, E_DEBUG, fmt, ##args);   \
} while(0)

#define HSRV_NEXTHOP_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(nexthop, NEXTHOP_NEXTHOP, fmt, ##args); \
} while(0)

#define HSRV_VLAN_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(vlan, VLAN_VLAN, fmt, ##args); \
} while(0)

#define HSRV_TAP_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tap, VLAN_VLAN, fmt, ##args); \
} while(0)

#define HSRV_IPSOUR_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(vlan, IPSOUR_IPSOUR, fmt, ##args); \
} while(0)

#define HSRV_MIRROR_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(mirror, MIRROR_MIRROR, fmt, ##args); \
} while(0)

#define HSRV_IF_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, VLAN_VLAN, fmt, ##args); \
} while(0)

#define HSRV_FDB_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(fdb, FDB_FDB, fmt, ##args); \
} while(0)

#define HSRV_L2MC_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(l2mc, L2MC_L2MC, fmt, ##args); \
} while(0)

#define HSRV_AGG_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(agg, AGG_AGG, fmt, ##args); \
} while(0)

#define HSRV_IPUC_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ipuc, IPUC_IPUC, fmt, ##args); \
} while(0)

#define HSRV_NEIGHBOR_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(neighbor, NEIGHBOR_NEIGHBOR, fmt, ##args); \
} while(0)

#define HSRV_ECMP_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ecmp, ECMP_ECMP, fmt, ##args); \
} while(0)

#define HSRV_IPMC_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ipmc, IPMC_IPMC, fmt, ##args); \
} while(0)

#define HSRV_ACL_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ecmp, ACL_ACL, fmt, ##args); \
} while(0)

#define HSRV_QOS_CLASS_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, QOS_CLASS, fmt, ##args); \
} while(0)

#define HSRV_QOS_POLICER_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, QOS_POLICER, fmt, ##args); \
} while(0)

#define HSRV_QOS_PROCESS_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, QOS_PROCESS, fmt, ##args); \
} while(0)

#define HSRV_QOS_QUEUE_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(interface, QOS_QUEUE, fmt, ##args); \
} while(0)

#define HSRV_DOT1X_DEBUG(fmt, args...) \
do { \
    if (_g_p_tbl_hsrv_debug->dot1x & HSRVDBG_FLAG_DOT1X_DOT1X)  \
       HSRV_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define HSRV_STP_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(stp, STP_STP, fmt, ##args); \
} while(0)

#define HSRV_PTP_RX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ptp, PTP_PKT_RX, fmt, ##args); \
} while(0)

#define HSRV_PTP_TX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ptp, PTP_PKT_TX, fmt, ##args); \
} while(0)

#define HSRV_PTP_CFG_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ptp, PTP_CFG, fmt, ##args); \
} while(0)

#define HSRV_PTP_INTERRUPT_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(ptp, PTP_INTERRUPT, fmt, ##args); \
} while(0)

#define HSRV_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        HSRV_LOG_ERR("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
        return(HSRV_E_INVALID_PTR); \
    }\
}while(0)

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-12 */
#define HSRV_TPOAM_COMMON_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tpoam, TPOAM_COMMON, fmt, ##args); \
} while(0)

#define HSRV_TPOAM_EVENT_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tpoam, TPOAM_EVENT, fmt, ##args); \
} while(0)

#define HSRV_TPOAM_UPDATE_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tpoam, TPOAM_UPDATE, fmt, ##args); \
} while(0)

#define HSRV_TPOAM_PKT_RX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tpoam, TPOAM_PKT_RX, fmt, ##args); \
} while(0)

#define HSRV_TPOAM_PKT_TX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(tpoam, TPOAM_PKT_TX, fmt, ##args); \
} while(0)
/* liwh end */

#define HSRV_L2PROTOCOL_PKT_RX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(l2protocol, L2PROTOCOL_PKT_RX, fmt, ##args); \
} while(0)

#define HSRV_L2PROTOCOL_PKT_TX_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(l2protocol, L2PROTOCOL_PKT_TX, fmt, ##args); \
} while(0)

#define HSRV_NAT_RULE_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(nat, NAT_RULE, fmt, ##args); \
} while(0)

#define HSRV_NAT_SESSION_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(nat, NAT_SESSION, fmt, ##args); \
} while(0)

#define HSRV_NAT_TIMER_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(nat, NAT_TIMER, fmt, ##args); \
} while(0)

#define HSRV_G8032_EVENT_DEBUG(fmt, args...) \
do { \
    HSRV_LOG_DEBUG(g8032, G8032_EVENT, fmt, ##args); \
} while(0)

#define HSRV_MEM_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        HSRV_LOG_ERR("malloc failed, %s:%d", __FUNCTION__, __LINE__);\
        return(HSRV_E_NO_MEMORY); \
    }\
}while(0)

#define HSRV_IF_ERROR_RETURN(op)                                                          \
do {                                                                                      \
    int32 rv;                                                                             \
    if ((rv = (op)) < 0)                                                                  \
    {                                                                                     \
        HSRV_LOG_ERR("HAL Server OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__); \
        return(rv);                                                                       \
    }                                                                                     \
}while(0)

#define HSRV_IF_ERROR_RETURN_NULL(op)                                                          \
do {                                                                                      \
    int32 rv;                                                                             \
    if ((rv = (op)) < 0)                                                                  \
    {                                                                                     \
        HSRV_LOG_ERR("HAL Server OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__); \
        return;                                                                       \
    }                                                                                     \
}while(0)

#define HSRV_IF_ERROR_CONTINUE(op)                                                        \
{                                                                                         \
    int32 rv;                                                                             \
    if ((rv = (op)) < 0)                                                                  \
    {                                                                                     \
        HSRV_LOG_ERR("HAL Server OP failed: rv = %d", rv);                                \
        continue;                                                                         \
    }                                                                                     \
}

#define HSRV_IF_ERROR_GOTO(op, ret, ERR)                                                  \
do {                                                                                      \
    if ((ret = (op)) < 0)                                                                 \
    {                                                                                     \
        HSRV_LOG_ERR("HAL Server OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__); \
        goto ERR;                                                                         \
    }                                                                                     \
}while(0)

int32
hsrv_debug_start();

#endif /* !__HSRV_DEBUG_H__ */

