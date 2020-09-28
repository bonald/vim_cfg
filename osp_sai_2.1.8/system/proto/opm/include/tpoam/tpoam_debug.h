#ifndef __TPOAM_DEBUG_H__
#define __TPOAM_DEBUG_H__

#include "gen/tbl_opm_debug_define.h"

#define TPOAM_OP_ERR_NO_RET(_op) \
do \
{ \
    int32 rv = 0; \
    if ((rv = (_op)) < 0) \
    { \
        TPOAM_LOG_ERR("Operation failed, %s.", tpoam_err_desc(rv)); \
    } \
} \
while (0)


#define TPOAM_OP_ERR_RET(_op) \
do \
{ \
    int32 rv = 0; \
    if ((rv = (_op)) < 0) \
    { \
        TPOAM_LOG_ERR("Operation failed, %s.", tpoam_err_desc(rv)); \
        return (rv); \
    } \
} \
while (0)


#define TPOAM_LOG_EMERG(fmt, args...)     log_sys(M_MOD_TPOAM, E_EMERGENCY, fmt, ##args)
#define TPOAM_LOG_ALERT(fmt, args...)     log_sys(M_MOD_TPOAM, E_ALERT, fmt, ##args)
#define TPOAM_LOG_CRIT(fmt, args...)      log_sys(M_MOD_TPOAM, E_CRITICAL, fmt, ##args)
#define TPOAM_LOG_ERR(fmt, args...)       log_sys(M_MOD_TPOAM, E_ERROR, fmt, ##args)
#define TPOAM_LOG_WARN(fmt, args...)      log_sys(M_MOD_TPOAM, E_WARNING, fmt, ##args)
#define TPOAM_LOG_NOTICE(fmt, args...)    log_sys(M_MOD_TPOAM, E_NOTICE, fmt ##args)
#define TPOAM_LOG_INFO(fmt, args...)      log_sys(M_MOD_TPOAM, E_INFORMATIONAL, fmt, ##args)
#define TPOAM_LOG_DEBUG(fmt, args...)     log_sys(M_MOD_TPOAM, E_DEBUG, fmt, ##args);

extern tbl_opm_debug_t *_g_p_tbl_opm_debug;

#define TPOAM_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_opm_debug->module & OPMDBG_FLAG_TPOAM_##typeenum)

#define TPOAM_LOG_DEBUG_MODULE(module, typeenum, fmt, args...) \
do { \
    if (TPOAM_DBG_IS_ON(module, typeenum)) \
        TPOAM_LOG_DEBUG(fmt, ##args); \
} while(0)

#define TPOAM_DBG_LB_LBM(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, LB_LBM_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_LB_LBR(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, LB_LBR_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_LM_COMM(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, LM_COMM_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_LM_LMM(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, LM_LMM_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_LM_LMR(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, LM_LMR_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_DM_COMM(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, DM_COMM_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_DM_DMM(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, DM_DMM_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_DM_DMR(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, DM_DMR_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_DM_1DMTX(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, DM_1DMTX_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_DM_1DMRX(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, DM_1DMRX_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_PACKET_RX(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, PACKET_RX_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_PACKET_TX(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, PACKET_TX_DBG, fmt, ##args); \
} while(0)

#define TPOAM_DBG_EVENT(fmt, args...) \
do { \
    TPOAM_LOG_DEBUG_MODULE(tpoam, EVENT_DBG, fmt, ##args); \
} while(0)

#define TPOAM_OP_ERR_RET_NO_LOG(_op) \
do \
{ \
    int32 rv = 0; \
    if ((rv = (_op)) < 0) \
    { \
        return (rv); \
    } \
} \
while (0)

int32
tpoam_cmd_process_tpoam_debug(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* !__TPOAM_DEBUG_H__ */

