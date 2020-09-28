#ifndef __ARPINSP_API_H__
#define __ARPINSP_API_H__

#include "bsd_queue.h"
#include "genlog.h"
#include "proto_debug.h"

#define ARPINSP_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_ARPINSPECTION, E_DEBUG, fmt, ##args);

#define ARPINSP_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (RT_DBG_IS_ON(module, typeenum)) \
        ARPINSP_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define ARPINSP_PKT_DEBUG(fmt, args...) \
do { \
    ARPINSP_LOG_DEBUG(arpinspection, ARPINSPECTION_PACKET, fmt, ##args); \
} while(0)

#define ARPINSP_EVENT_DEBUG(fmt, args...) \
do { \
    ARPINSP_LOG_DEBUG(arpinspection, ARPINSPECTION_EVENT, fmt, ##args); \
} while(0)

#define ARPINSP_PROTO_DEBUG(fmt, args...) \
do { \
    ARPINSP_LOG_DEBUG(arpinspection, ARPINSPECTION_PROTO, fmt, ##args); \
} while(0)

#define ARPINSP_TIMER_DEBUG(fmt, args...) \
do { \
    ARPINSP_LOG_DEBUG(arpinspection, ARPINSPECTION_TIMER, fmt, ##args); \
} while(0)


#define M_TIME_BUFSIZE  32
#define M_LOG_STRSIZE   128
#define M_LOG_BUFSIZE   160
#define MAC_STRING_LEN  20
#define IP_STRING_LEN   16
#define LOGGING_BUF_SIZE  8192

/* log item structure */
typedef struct tagArpInspLogItem
{
    TAILQ_ENTRY(tagArpInspLogItem) entry;
    char *pszLog;                     /* log string */
    char time_str[M_TIME_BUFSIZE];
    uint32 nLen;                       /* length of the buffer */
    E_SEVERITY severy;
} S_ArpInspLogItem;

/* log buffer head */
typedef TAILQ_HEAD(tagLogQueue, tagArpInspLogItem) S_ArpInspLogBuf;

void
arpinsp_write_log_buffer_timer_cb(void *p_argv);

int32
arpinsp_valid_check(tbl_interface_t* p_db_if, arp_eth_t *arp_p, eth_hdr_t *eth_p, uint32 vid);

S_ArpInspLogItem *
arpinsp_malloc_logitem(uint32 nLen);

void 
arpinsp_free_logitem(S_ArpInspLogItem *pItem);

#endif /* !__ARPINSP_API_H__ */

