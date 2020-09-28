#ifndef __ARP_DEBUG_H__
#define __ARP_DEBUG_H__

#define ARP_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_ARP, E_DEBUG, fmt, ##args);

#define ARP_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (RT_DBG_IS_ON(module, typeenum)) \
        ARP_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define ARP_PKT_DEBUG(fmt, args...) \
do { \
    ARP_LOG_DEBUG(arp, ARP_PACKET, fmt, ##args); \
} while(0)

#define ARP_EVENT_DEBUG(fmt, args...) \
do { \
    ARP_LOG_DEBUG(arp, ARP_EVENT, fmt, ##args); \
} while(0)

#define ARP_PROTO_DEBUG(fmt, args...) \
do { \
    ARP_LOG_DEBUG(arp, ARP_PROTO, fmt, ##args); \
} while(0)

#define ARP_TIMER_DEBUG(fmt, args...) \
do { \
    ARP_LOG_DEBUG(arp, ARP_TIMER, fmt, ##args); \
} while(0)

#define RT_ARP_SM_DEBUG(_AE_P_, _ADDR_) \
    if (RT_DBG_IS_ON(arp, ARP_PROTO)) \
        ARP_DEBUG_PRINT("%s, arp ip is %s, state is %s", \
        __FUNCTION__, sal_inet_ntop(AF_INET, &(_AE_P_)->key.ip, _ADDR_, INET_ADDRSTRLEN), cdb_enum_val2str(as_strs, AS_MAX, (_AE_P_)->status))

int32
arp_debug_entry_trace(tbl_arp_t *ae_p, arp_entry_action_t op);

int32
arp_debug_pkt_trace(char *ifname, arp_eth_t *arp_p, arp_pkt_action_t act);

int32
arp_debug_cache_overflow();

#endif /* !__ARP_DEBUG_H__ */

