#ifndef __AUTHD_DEBUG_H__
#define __AUTHD_DEBUG_H__

extern tbl_authd_debug_t *_g_p_tbl_authd_debug;

#define AUTHD_ERR_PRINT(fmt, args...) \
    log_sys(M_MOD_AUTHD, E_ERROR, fmt, ##args);

#define AUTHD_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_AUTHD, E_DEBUG, fmt, ##args);

#define AUTHD_DBG_IS_ON(typeenum) \
    (_g_p_tbl_authd_debug->flags & AUTHD_DBG_FLAG_##typeenum)

#define AUTHD_LOG_DEBUG(typeenum, fmt, args...) \
do { \
    if (AUTHD_DBG_IS_ON(typeenum)) \
        AUTHD_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define AUTHD_PKT_DEBUG(fmt, args...) \
do { \
    AUTHD_LOG_DEBUG(PACKET, fmt, ##args); \
} while(0)

#define AUTHD_EVENT_DEBUG(fmt, args...) \
do { \
    AUTHD_LOG_DEBUG(EVENT, fmt, ##args); \
} while(0)

#define AUTHD_PROTO_DEBUG(fmt, args...) \
do { \
    AUTHD_LOG_DEBUG(PROTO, fmt, ##args); \
} while(0)

#define AUTHD_TIMER_DEBUG(fmt, args...) \
do { \
    AUTHD_LOG_DEBUG(TIMER, fmt, ##args); \
} while(0)

#endif /* !__AUTHD_DEBUG_H__ */

