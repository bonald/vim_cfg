#ifndef __G8032_DEFINE_H__
#define __G8032_DEFINE_H__
#include "genlog.h"
#include "proto_debug.h"
#include "gen/tbl_opm_debug_define.h"
/* G.8032 FSM States. */
#define G8032_RING_VLAN_TAG   {0x81, 0x00}
#define G8032_RING_PACKET_TYPE   {0x89,0x02}
#define G8032_RING_ID_MAXIMUM 2048
#define G8032_RING_MAX_VALUE_ON_PORT 2
#define G8032_RING_DEF_FLUSH_FDB_INTERVAL 10
#define G8032_RING_DEF_TX_RAPS_INTERVAL 5
enum
{
    G8032_NODE_STATE_IDLE = 0,
    G8032_NODE_STATE_PROTECTION,
    G8032_NODE_STATE_MANUAL_SWITCH,
    G8032_NODE_STATE_FORCE_SWITCH,
    G8032_NODE_STATE_PENDING,
    G8032_FSM_STATE_MAX
};

enum
{
    G8032_NODE_ROLE_RPL_OWNER =1,
    G8032_NODE_ROLE_RPL_NEIGHBOR ,
    G8032_NODE_ROLE_RPL_NEXT_NEIGHBOR ,
    G8032_NODE_ROLE_NA,
    G8032_NODE_ROLE_MAX
};

/* G.8032 FSM Events. */
enum
{
    G8032_PRI_REQ_R_APS_NR = 0,
    G8032_PRI_REQ_R_APS_NR_RB,
    G8032_PRIO_REQ_R_APS_WTR_RUNNING,
    G8032_PRI_REQ_WTR_EXPIRES,
    G8032_PRI_REQ_R_APS_SF,
    G8032_PRI_REQ_L_CLEAR_SF,
    G8032_PRI_REQ_L_SF,
    G8032_PRI_REQ_L_CLEAR,
    G8032_PRI_REQ_L_FS,
    G8032_PRI_REQ_R_APS_FS,
    G8032_PRI_REQ_L_MS,
    G8032_PRI_REQ_R_APS_MS,
    G8032_PRIO_REQ_R_APS_WTB_RUNNING,
    G8032_PRI_REQ_WTB_EXPIRES,
    G8032_PRI_REQ_R_APS_EVENT,
    G8032_FSM_EVENT_MAX     
};

/*G8032 DEBUG*/
extern tbl_opm_debug_t *_g_p_tbl_opm_debug;
#define G8032_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_opm_debug->module & OPMDBG_FLAG_##typeenum)

#define G8032_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_G8032, E_DEBUG, fmt, ##args);

#define G8032_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (G8032_DBG_IS_ON(module, typeenum)) \
        G8032_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define G8032_TX_DEBUG(fmt, args...) \
do { \
    G8032_LOG_DEBUG(g8032, G8032_TX, fmt, ##args); \
} while(0)

#define G8032_RX_DEBUG(fmt, args...) \
do { \
    G8032_LOG_DEBUG(g8032, G8032_RX, fmt, ##args); \
} while(0)

#define G8032_EVENT_DEBUG(fmt, args...) \
do { \
    G8032_LOG_DEBUG(g8032, G8032_EVENT, fmt, ##args); \
} while(0)

#define G8032_TIMER_DEBUG(fmt, args...) \
do { \
    G8032_LOG_DEBUG(g8032, G8032_TIMER, fmt, ##args); \
} while(0)

/*G8032 DEBUG end*/

#endif
