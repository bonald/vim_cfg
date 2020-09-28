/****************************************************************************
 * lcm_debug.h      lcm debug header file.
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianting Huang
 * Date          :2010-08-16
 * Reason        :First Create.
 ****************************************************************************/
#ifndef __LCM_DEBUG_H__
#define __LCM_DEBUG_H__
/****************************************************************************
*
* Header Files 
*
****************************************************************************/
#include "lcm.h"
#include "lcm_client.h"
#include "genlog.h"
#include "ctclib_debug.h"
#include "lcm_client.h"
#include "LcmMsg.h"
#include "glb_macro.h"

/****************************************************************************
* Micros 
****************************************************************************/
/*encapsulation for log*/
extern int8 g_lcm_bringup_print_on;
extern int32 g_lcm_debug_type;
extern int64 g_drv_debug_type;

typedef enum 
{
    LCM_CARD = 1<<0,
    LCM_LCMGT = 1<<1,
    LCM_CHSMTLK = 1<<2,
    LCM_LCSH = 1<<3,
    LCM_DRVS = 1<<4,
    LCM_CLIENT = 1<<5,
    LCM_DEBUG_MAX=1<<6
}E_LCM_DEBUG_TYPE;

#define LCM_PRINT_CONSOLE(fmt, args...)                        \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    sal_fprintf(fp_console, "[%d][%s] "fmt"\n",__LINE__,__FUNCTION__, ##args);   \
    fclose(fp_console);                                        \
}

#undef TOSTR
#define TOSTR(A) #A
/*encapsulation for debug*/
#define LCM_LOG_BRINGUP(fmt, args...)                      \
do{                                                          \
    if(g_lcm_bringup_print_on)                               \
        LCM_PRINT_CONSOLE(fmt,##args)                        \
}while(0);

#define LCM_LOG_DEBUG(typeenum, fmt, args...)         \
do { \
    if (GLB_FLAG_ISSET(g_lcm_debug_type,typeenum)) \
        log_sys(M_MOD_LCM, E_DEBUG, "[%s][%s]"fmt,TOSTR(typeenum),__FUNCTION__,##args); \
} while (0)

#define LCM_LOG_DEBUG_ENABLE(typeenum) \
     GLB_SET_FLAG(g_lcm_debug_type, typeenum); 

#define LCM_LOG_DEBUG_DISABLE(typeenum) \
        GLB_UNSET_FLAG(g_lcm_debug_type, typeenum); 

#define DRV_LOG_DEBUG_ENABLE(typeenum) \
     GLB_SET_FLAG(g_drv_debug_type, typeenum); 

#define DRV_LOG_DEBUG_DISABLE(typeenum) \
        GLB_UNSET_FLAG(g_drv_debug_type, typeenum); 

#ifdef CNOS_OS
#define LCM_LOG_EMERG(fmt, args...)    log_sys(M_MOD_INTERNAL, E_EMERGENCY, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_ALERT(fmt, args...)    log_sys(M_MOD_INTERNAL, E_ALERT, fmt, ##args)
#define LCM_LOG_CRIT(fmt, args...)     log_sys(M_MOD_INTERNAL, E_CRITICAL, fmt, ##args)
#define LCM_LOG_ERR(fmt, args...)      log_sys(M_MOD_INTERNAL, E_ERROR, fmt, ##args)
#define LCM_LOG_WARN(fmt, args...)     log_sys(M_MOD_INTERNAL, E_WARNING, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_NOTICE(fmt, args...)    log_sys(M_MOD_INTERNAL, E_NOTICE, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_INFO(fmt, args...)     log_sys(M_MOD_INTERNAL, E_INFORMATIONAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#else
#define LCM_LOG_EMERG(fmt, args...)    log_diag(M_MOD_LCM, E_EMERGENCY, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_ALERT(fmt, args...)    log_diag(M_MOD_LCM, E_ALERT, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_CRIT(fmt, args...)     log_diag(M_MOD_LCM, E_CRITICAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_ERR(fmt, args...)      log_diag(M_MOD_LCM, E_ERROR, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_WARN(fmt, args...)     log_diag(M_MOD_LCM, E_WARNING, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_NOTICE(fmt, args...)    log_diag(M_MOD_LCM, E_NOTICE, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#define LCM_LOG_INFO(fmt, args...)     log_diag(M_MOD_LCM, E_INFORMATIONAL, "[%d][%s]"fmt, __LINE__, __FUNCTION__, ##args)
#endif

/*error return*/
#define LCM_IF_ERROR_RETURN(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        LCM_LOG_ERR("LCM OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

#define LCM_IF_ERROR_CONTINUE(op) \
{ \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        LCM_LOG_ERR("LCM OP failed: rv = %d", rv);\
        continue; \
    }\
}

/****************************************************************************
* Enums 
****************************************************************************/

/****************************************************************************
* function declare 
****************************************************************************/
int32 lcm_debug_init(void);
void lcm_bringup_log_set(uint8 value);

#endif /*__LCM_DEBUG_H__*/
