/****************************************************************************
 * hagt_debug.h      Hal Agnt debug header file.
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianghong Gu
 * Date          :2010-07-11.
 * Reason        :First Create.
 ****************************************************************************/

#ifndef __HAGT_DEBUG_H__
#define __HAGT_DEBUG_H__

/****************************************************************************
*
* Header Files 
*
****************************************************************************/
#include "genlog.h"

/****************************************************************************
* Micros 
****************************************************************************/

/*huangxt for ecc*/
#define HAGT_LOG_ERR_CONSOLE(fmt, args...)                      \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    sal_fprintf(fp_console, fmt"\n", ##args);   \
    fclose(fp_console);                                        \
}
#define HAGT_LOG_ERR_LOGSYS(fmt, args...)                      \
{                                                              \
    log_sys(M_MOD_HAGT, E_ERROR, fmt, ##args);\
}

/*encapsulation for log*/
#ifdef _GLB_DISTRIBUTE_SYSTEM_
extern uint16 linecard_num;
#define HAGT_LOG_EMERG(fmt, args...)   log_sys(M_MOD_HAGT, E_EMERGENCY, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_ALERT(fmt, args...)   log_sys(M_MOD_HAGT, E_ALERT, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_CRIT(fmt, args...)   log_sys(M_MOD_HAGT, E_CRITICAL, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_ERR(fmt, args...)   log_sys(M_MOD_HAGT, E_ERROR, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_WARN(fmt, args...)   log_sys(M_MOD_HAGT, E_WARNING, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_NOTICE(fmt, args...)  log_sys(M_MOD_HAGT, E_NOTICE, "lc:%d "fmt, linecard_num, ##args)
#define HAGT_LOG_INFO(fmt, args...)   log_sys(M_MOD_HAGT, E_INFORMATIONAL, "lc:%d "fmt, linecard_num, ##args)
#else
#define HAGT_LOG_EMERG(fmt, args...)   log_sys(M_MOD_HAGT, E_EMERGENCY, fmt, ##args)
#define HAGT_LOG_ALERT(fmt, args...)   log_sys(M_MOD_HAGT, E_ALERT, fmt, ##args)
#define HAGT_LOG_CRIT(fmt, args...)   log_sys(M_MOD_HAGT, E_CRITICAL, fmt, ##args)
#define HAGT_LOG_ERR(fmt, args...)   log_sys(M_MOD_HAGT, E_ERROR, fmt, ##args)
#define HAGT_LOG_WARN(fmt, args...)   log_sys(M_MOD_HAGT, E_WARNING, fmt, ##args)
#define HAGT_LOG_NOTICE(fmt, args...)  log_sys(M_MOD_HAGT, E_NOTICE, fmt, ##args)
#define HAGT_LOG_INFO(fmt, args...)   log_sys(M_MOD_HAGT, E_INFORMATIONAL, fmt, ##args)
#endif

/* should be same as tbl_hagt_debug_t */
typedef struct
{
    uint32               interface;           /* bitmap of HAGTDBG_FLAG_INTERFACE_ */
    uint32               cpu;                 /* bitmap of HAGTDBG_FLAG_CPU_ */
    uint32               nexthop;             /* bitmap of HAGTDBG_FLAG_NEXTHOP_ */
    uint32               vlan;                /* bitmap of HAGTDBG_FLAG_VLAN_ */
    uint32               fdb;                 /* bitmap of HAGTDBG_FLAG_FDB_ */
    uint32               l2mc;                /* bitmap of HAGTDBG_FLAG_L2MC_ */
    uint32               agg;                 /* bitmap of HAGTDBG_FLAG_AGG_ */
    uint32               ipuc;                /* bitmap of HAGTDBG_FLAG_IPUC_ */
    uint32               neighbor;            /* bitmap of HAGTDBG_FLAG_NEIGHBOR_ */
    uint32               ecmp;                /* bitmap of HAGTDBG_FLAG_ECMP_ */
    uint32               acl;                 /* bitmap of HAGTDBG_FLAG_ACL_ */
    uint32               stats;               /* bitmap of HAGTDBG_FLAG_STATS_ */
    uint32               qos;                 /* bitmap of HAGTDBG_FLAG_QOS_ */
    uint32               flow;             /* bitmap of HAGTDBG_FLAG_QOS_ */

} hagt_debug_t;

extern hagt_debug_t g_hagt_debug;

#if 0
/* temply use syslog ERROR to replace CTCLIB_PRINT */
/*encapsulation for debug*/
#define HAGT_LOG_DEBUG(mod, sub, typeenum, fmt, args...)\
    CTCLIB_DEBUG_OUT_INFO(HAGT, mod, sub, typeenum, fmt, ##args); 
#endif

#define HAGT_DBG_IS_ON(module, typeenum) \
    (g_hagt_debug.module & HAGTDBG_FLAG_##typeenum)

#define HAGT_DEBUG_PRINT(fmt, args...)   \
    log_sys(M_MOD_HAGT, E_ERROR, fmt, ##args);

#define HAGT_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (HAGT_DBG_IS_ON(module, typeenum)) \
        HAGT_DEBUG_PRINT(fmt, ##args); \
} while (0)

#define HAGT_L2IF_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(interface, INTERFACE_L2IF, fmt, ##args); \
} while(0)

#define HAGT_L3IF_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(interface, INTERFACE_L3IF, fmt, ##args); \
} while(0)

#define HAGT_IFDB_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(interface, INTERFACE_IFDB, fmt, ##args); \
} while(0)

#define HAGT_CPU_PDU_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(cpu, CPU_PDU, fmt, ##args); \
} while(0)

#define HAGT_CPU_TRAFFIC_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(cpu, CPU_TRAFFIC, fmt, ##args); \
} while(0)

#define HAGT_NEXTHOP_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(nexthop, NEXTHOP_NEXTHOP, fmt, ##args); \
} while(0)

#define HAGT_VLAN_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(vlan, VLAN_VLAN, fmt, ##args); \
} while(0)

#define HAGT_FDB_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(fdb, FDB_FDB, fmt, ##args); \
} while(0)

#define HAGT_L2MC_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(l2mc, L2MC_L2MC, fmt, ##args); \
} while(0)

#define HAGT_AGG_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(agg, AGG_AGG, fmt, ##args); \
} while(0)

#define HAGT_IPUC_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(ipuc, IPUC_IPUC, fmt, ##args); \
} while(0)

#define HAGT_NEIGHBOR_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(neighbor, NEIGHBOR_NEIGHBOR, fmt, ##args); \
} while(0)

#define HAGT_ECMP_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(ecmp, ECMP_ECMP, fmt, ##args); \
} while(0)

#define HAGT_ACL_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(acl, ACL_ACL, fmt, ##args); \
} while(0)

#define HAGT_QOS_CLASS_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, QOS_CLASS, fmt, ##args); \
} while(0)

#define HAGT_QOS_POLICER_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, QOS_POLICER, fmt, ##args); \
} while(0)

#define HAGT_QOS_PROCESS_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, QOS_PROCESS, fmt, ##args); \
} while(0)

#define HAGT_QOS_QUEUE_DEBUG(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, QOS_QUEUE, fmt, ##args); \
} while(0)

/*error return*/
#define HAGT_IF_ERROR_RETURN(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

#define HAGT_IF_ERROR_CONTINUE(op) \
{ \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        HAGT_LOG_ERR("HAL Agent OP failed: rv = %d", rv);\
        continue; \
    }\
}

/*pointer check*/
#define HAGT_CHECK_PTR(ptr)                                                               \
if(!ptr)                                                                                  \
{                                                                                         \
    return HAGT_E_INVALID_PTR;                                                            \
}

/*pointer check*/
#define HAGT_CHECK_PTR_RETURN_VOID(ptr)                                                               \
if(!ptr)                                                                                  \
{                                                                                         \
    return;                                                            \
}

/****************************************************************************
* Enums 
****************************************************************************/


/****************************************************************************
* function declare 
****************************************************************************/
int32
hagt_debug_init(void);

#endif /* __HSRV_DEBUG_H__*/
