
#ifndef __ARP_LOG_H__
#define __ARP_LOG_H__

extern log_message_t g_arp_log_message[];

enum nsm_arp_log_key_s
{   
    ARP_4_DB_CACHE_OVERFLOW = 1,
    ARP_4_DUPLICATE_IP_ADDR,
    ARP_4_DISCARD_PACKET,
    ARP_3_MEM_EXCEED,
    ARP_6_LOG_KEY_MAX,
};
typedef enum nsm_arp_log_key_s nsm_arp_log_key_t;

#define ARP_LOG_USER(eSeverity, code, type, ...)\
do {\
    if(code < ARP_6_LOG_KEY_MAX)\
    {\
         log_sys_rate_limit(M_MOD_ARP, eSeverity, type, g_arp_log_message[code].str, ##__VA_ARGS__); \
    } \
}while(0)

#endif /* __ARP_LOG_H__ */

