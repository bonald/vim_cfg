#ifndef __LSRV_DEBUG_H__
#define __LSRV_DEBUG_H__

#include "gen/tbl_lsrv_debug_define.h"
#include "gen/tbl_lsrv_debug.h"

#define LSRV_LOG_EMERG(fmt, args...)    log_sys(M_MOD_LSRV, E_EMERGENCY, fmt, ##args)
#define LSRV_LOG_ALERT(fmt, args...)    log_sys(M_MOD_LSRV, E_ALERT, fmt, ##args)
#define LSRV_LOG_CRIT(fmt, args...)     log_sys(M_MOD_LSRV, E_CRITICAL, fmt, ##args)
#define LSRV_LOG_ERR(fmt, args...)      log_sys(M_MOD_LSRV, E_ERROR, fmt, ##args)
#define LSRV_LOG_WARN(fmt, args...)     log_sys(M_MOD_LSRV, E_WARNING, fmt, ##args)
#define LSRV_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_LSRV, E_NOTICE, fmt, ##args)
#define LSRV_LOG_INFO(fmt, args...)     log_sys(M_MOD_LSRV, E_INFORMATIONAL, fmt, ##args)

extern tbl_lsrv_debug_t *_g_p_tbl_lsrv_debug;

#define LSRV_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_lsrv_debug->module & LSRVDBG_FLAG_##typeenum)

#define LSRV_DEBUG_PRINT(fmt, args...)   \
    log_sys(M_MOD_LSRV, E_DEBUG, fmt, ##args);

#define LSRV_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (LSRV_DBG_IS_ON(module, typeenum)) \
        LSRV_DEBUG_PRINT(fmt, ##args); \
} while (0)

/*encapsulation for debug*/
#define LSRV_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}

#if 1
#define LSRV_CARD_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_CARD, fmt, ##args); \
} while(0)

#define LSRV_PORT_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_PORT, fmt, ##args); \
} while(0)


#define LSRV_FIBER_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_FIBER, fmt, ##args); \
} while(0)


#define LSRV_FAN_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_FAN, fmt, ##args); \
} while(0)


#define LSRV_PSU_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_PSU, fmt, ##args); \
} while(0)


#define LSRV_VCM_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_VCM, fmt, ##args); \
} while(0)

#define LSRV_POE_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_POE, fmt, ##args); \
} while(0)


#define LSRV_LED_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_LED, fmt, ##args); \
} while(0)

#define LSRV_SENSOR_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(card, CARD_SENSOR, fmt, ##args); \
} while(0)

#define LSRV_SYNCE_DEBUG(fmt, args...) \
do { \
    LSRV_LOG_DEBUG(synce, SYNCE_SYNCE, fmt, ##args); \
} while(0)
#else
#define LSRV_CARD_DEBUG LSRV_PRINT_CON
#define LSRV_PORT_DEBUG LSRV_PRINT_CON
#define LSRV_FIBER_DEBUG LSRV_PRINT_CON
#define LSRV_FAN_DEBUG LSRV_PRINT_CON
#define LSRV_PSU_DEBUG LSRV_PRINT_CON
#define LSRV_VCM_DEBUG LSRV_PRINT_CON
#define LSRV_SENSOR_DEBUG LSRV_PRINT_CON
#define LSRV_LED_DEBUG LSRV_PRINT_CON
#endif

#define LSRV_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        log_sys(M_MOD_INTERNAL, E_ERROR, "Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
        return(PM_E_INVALID_PARAM); \
    }\
}while(0)

#define LSRV_MEM_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        log_sys(M_MOD_INTERNAL, E_ERROR, "malloc failed, %s:%d", __FUNCTION__, __LINE__);\
        return(PM_E_NO_MEMORY); \
    }\
}while(0)

#define LSRV_IF_ERROR_RETURN(op)                                                          \
do {                                                                                      \
    int32 rv;                                                                             \
    if ((rv = (op)) < 0)                                                                  \
    {                                                                                     \
        log_sys(M_MOD_INTERNAL, E_ERROR, "LAI Server OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__); \
        CTCLIB_PRINT_CON("LAI Server OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv);                                                                       \
    }                                                                                     \
}while(0)

#define LSRV_IF_ERROR_CONTINUE(op)                                                        \
{                                                                                         \
    int32 rv;                                                                             \
    if ((rv = (op)) < 0)                                                                  \
    {                                                                                     \
        log_sys(M_MOD_INTERNAL, E_ERROR, "HAL Server OP failed: rv = %d", rv);                                \
        continue;                                                                         \
    }                                                                                     \
}

#define LSRV_IF_ERROR_GOTO(op, ret, ERR)                                                  \
do {                                                                                      \
    if ((ret = (op)) < 0)                                                                 \
    {                                                                                     \
        log_sys(M_MOD_INTERNAL, E_ERROR, "HAL Server OP failed: rv = %d, %s:%d", ret, __FUNCTION__, __LINE__); \
        goto ERR;                                                                         \
    }                                                                                     \
}while(0)

int32
lsrv_debug_start();

#endif /* !__LSRV_DEBUG_H__ */


