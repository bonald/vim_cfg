#ifndef __CHSM_DEBUG_H__
#define __CHSM_DEBUG_H__

#include "genlog.h"
#include "ctclib_debug.h"

/*encapsulation for debug*/
#define CHSM_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}
#if 1
#define CHSM_LOG_EMERG(fmt, args...)      log_sys(M_MOD_CHSM, E_EMERGENCY, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_ALERT(fmt, args...)      log_sys(M_MOD_CHSM, E_ALERT, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_CRIT(fmt, args...)       log_sys(M_MOD_CHSM, E_CRITICAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_ERR(fmt, args...)        log_sys(M_MOD_CHSM, E_ERROR, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_WARN(fmt, args...)       log_sys(M_MOD_CHSM, E_WARNING, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_NOTICE(fmt, args...)     log_sys(M_MOD_CHSM, E_NOTICE, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define CHSM_LOG_INFO(fmt, args...)       log_sys(M_MOD_CHSM, E_INFORMATIONAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)

#define CHSM_DEBUG_INFO(typeenum, fmt, args...) \
do {\
    CHSM_DEBUG_OUT_INFO(CHSM, chsm, typeenum, fmt, ##args);\
}while(0)


#define CHSM_DEBUG_FUNC()  CHSM_DEBUG_INFO(CHSM_CHASSIS, "Enter into %s.", __FUNCTION__)

#define CHSM_DEBUG(typeenum, fmt, args...) \
do {\
    CHSM_DEBUG_INFO(typeenum, fmt, ##args);\
}while(0)

#else
#define CHSM_LOG_EMERG CHSM_PRINT_CON
#define CHSM_LOG_ALERT CHSM_PRINT_CON
#define CHSM_LOG_CRIT CHSM_PRINT_CON
#define CHSM_LOG_ERR CHSM_PRINT_CON
#define CHSM_LOG_WARN CHSM_PRINT_CON
#define CHSM_LOG_NOTICE CHSM_PRINT_CON
#define CHSM_LOG_INFO CHSM_PRINT_CON
#define CHSM_DEBUG CHSM_PRINT_CON
#endif

/*error return*/
#define CHSM_IF_ERROR_RETURN(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        CHSM_LOG_ERR("CHSM OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

#define CHSM_IF_ERROR_CONTINUE(op) \
{ \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        CHSM_LOG_ERR("CHSM OP failed: rv = %d", rv);\
        continue; \
    }\
}

int32
chsm_debug_start();

#endif /* !__CHSM_DEBUG_H__ */
