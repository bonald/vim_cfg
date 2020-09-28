
#ifndef __PTP_DEBUG_H__
#define __PTP_DEBUG_H__

#include "gen/tbl_ptp_debug_define.h"

#define PTP_LOG_EMERG(fmt, args...)     log_sys(M_MOD_PTP, E_EMERGENCY, fmt, ##args)
#define PTP_LOG_ALERT(fmt, args...)     log_sys(M_MOD_PTP, E_ALERT, fmt, ##args)
#define PTP_LOG_CRIT(fmt, args...)      log_sys(M_MOD_PTP, E_CRITICAL, fmt, ##args)
#define PTP_LOG_ERR(fmt, args...)       log_sys(M_MOD_PTP, E_ERROR, fmt, ##args)
#define PTP_LOG_WARN(fmt, args...)      log_sys(M_MOD_PTP, E_WARNING, fmt, ##args)
#define PTP_LOG_NOTICE(fmt, args...)    log_sys(M_MOD_PTP, E_NOTICE, fmt, ##args)
#define PTP_LOG_INFO(fmt, args...)      log_sys(M_MOD_PTP, E_INFORMATIONAL, fmt, ##args)
#define PTP_LOG_DEBUG(fmt, args...)     log_sys(M_MOD_PTP, E_DEBUG, fmt, ##args);

extern tbl_ptp_debug_t *_g_p_tbl_ptp_debug;

#define PTP_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_ptp_debug->module & PTPDBG_FLAG_##typeenum)

#define PTP_LOG_DEBUG_MODULE(module, typeenum, fmt, args...) \
do { \
    if (PTP_DBG_IS_ON(module, typeenum)) \
        PTP_LOG_DEBUG(fmt, ##args); \
} while(0)



#define PTP_DBG_COMM(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, COMMUNICATION_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_BMC(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, BMC_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_TMR(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, TIMER_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_SYNC(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, SYNCHRONIZATION_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_PACKET_RX(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, PACKET_RX_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_PACKET_TX(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, PACKET_TX_DBG, fmt, ##args); \
} while(0)

#define PTP_DBG_PACKET_DISCARD(fmt, args...) \
do { \
    PTP_LOG_DEBUG_MODULE(ptp, PACKET_DISCARD_DBG, fmt, ##args); \
} while(0)

#endif /*!__PTP_DEBUG_H__*/
