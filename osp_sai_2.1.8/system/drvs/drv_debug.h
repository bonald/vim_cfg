#ifndef __DRV_DEBUG__
#define __DRV_DEBUG__

#include "genlog.h"
#include "ctclib_debug.h"
#include "sal_common.h"
#include "drv_specific.h"
#include "glb_macro.h"

#define DRV_MALLOC(type,size) XMALLOC(type,size)
#define DRV_CALLOC(type,size) XCALLOC(type, size)
#define DRV_FREE(type,ptr)    XFREE(type,ptr)

extern int64 g_drv_debug_type;

typedef enum 
{
    DRV_BUS_I2C = 1<<0,
    DRV_BUS_SPI = 1<<1,
    DRV_BUS_MDIO = 1<<2,
    DRV_PHY = 1<<3,
    DRV_EPLD = 1<<4,
    DRV_FIBER = 1<<5,
    DRV_SENSOR = 1<<6,
    DRV_GPIO = 1<<7,
    DRV_MUX = 1<<8,
    DRV_FAN = 1<<9,
    DRV_POWER = 1<<10,
    DRV_EEPROM = 1<<11,
    DRV_CLOCK = 1<<12,
    DRV_VCM = 1<<13,
    DRV_LED = 1<<14,
    DRV_LTC2991 = 1<<15,
    DRV_LTC4151 = 1<<16,
    DRV_VSC3308 = 1<<17,
    DRV_SYSENV = 1<<18,
    DRV_POE = 1<<19,
    DRV_DEBUG_MAX = 1<<20
}E_DRV_DEBUG_TYPE;

#undef TOSTR
#define TOSTR(A) #A

#define DRV_LOG_DEBUG(typeenum, fmt, args...)         \
do { \
    if (GLB_FLAG_ISSET(g_drv_debug_type,typeenum)) \
        log_sys(M_MOD_LCM, E_DEBUG, "[%s][%s]"fmt, TOSTR(typeenum),__FUNCTION__,##args); \
} while (0);


#define DRV_CTC_CHK_PTR(_PTR)                                                   \
do {                                                                        \
    if ((_PTR) == NULL)                                                     \
    {                                                                       \
        DRV_LOG_ERR("%s NULL pointer", __FUNCTION__);                       \
        return -1;                                                          \
    }                                                                       \
}while(0)

#define DRV_CTC_CHK_PTR_NULL(_PTR)                                                   \
    do {                                                                        \
        if ((_PTR) == NULL)                                                     \
        {                                                                       \
            DRV_LOG_ERR("%s NULL pointer", __FUNCTION__);                       \
            return NULL;                                                          \
        }                                                                       \
    }while(0)


#if CTC_BRING_UP
#define DRV_LOG_EMERG(fmt, args...)   log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_ALERT(fmt, args...)   log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_CRIT(fmt, args...)    log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_ERR(fmt, args...)     log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_WARN(fmt, args...)    log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_NOTICE(fmt, args...)  log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_INFO(fmt, args...)    log_ctc_raw_debug("\n[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#else
#define DRV_LOG_EMERG(fmt, args...)   log_diag(M_MOD_DRV, E_EMERGENCY, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_ALERT(fmt, args...)   log_diag(M_MOD_DRV, E_ALERT, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_CRIT(fmt, args...)   log_diag(M_MOD_DRV, E_CRITICAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_ERR(fmt, args...)   log_diag(M_MOD_DRV, E_ERROR, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_WARN(fmt, args...)   log_diag(M_MOD_DRV, E_WARNING, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_NOTICE(fmt, args...)  log_diag(M_MOD_DRV, E_NOTICE, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define DRV_LOG_INFO(fmt, args...)   log_diag(M_MOD_DRV, E_INFORMATIONAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)

#endif
#define DRV_CONSOLE_PRINT(fmt, args...)                        \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    sal_fprintf(fp_console, fmt, ##args);                      \
    fclose(fp_console);                                        \
}

#define DRV_DIAG_LOG(fmt, args...)                        \
do {                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    sal_fprintf(fp_console, fmt, ##args);                      \
    fclose(fp_console);                                        \
    FILE * fp_log = NULL;                                  \
    fp_log = fopen("/tmp/.bootup_diag_log", "a");                  \
    sal_fprintf(fp_log, fmt, ##args);                      \
    fclose(fp_log);                                        \
    log_diag(M_MOD_DRV, E_WARNING, fmt, ##args);    \
} while (0)

#endif /*__DRV_DEBUG__*/
