#ifndef __SSM_API_H__
#define __SSM_API_H__

#include "bsd_queue.h"
#include "genlog.h"
#include "proto_debug.h"
#include "gen/tbl_ssm_debug_define.h"

/*SSM DEBUG*/
extern tbl_ssm_debug_t *_g_p_tbl_ssm_debug;
#define SSM_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_ssm_debug->module & SSM_DEBUG_##typeenum)

#define SSM_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_SSM, E_DEBUG, fmt, ##args);

#define SSM_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (SSM_DBG_IS_ON(module, typeenum)) \
        SSM_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define SSM_TXPKT_DEBUG(fmt, args...) \
do { \
    SSM_LOG_DEBUG(ssm, FLAG_PACKET_TX, fmt, ##args); \
} while(0)

#define SSM_RXPKT_DEBUG(fmt, args...) \
do { \
    SSM_LOG_DEBUG(ssm, FLAG_PACKET_RX, fmt, ##args); \
} while(0)

#define SSM_EVENT_DEBUG(fmt, args...) \
do { \
    SSM_LOG_DEBUG(ssm, FLAG_EVENT, fmt, ##args); \
} while(0)

#define SSM_TIMER_DEBUG(fmt, args...) \
do { \
    SSM_LOG_DEBUG(ssm, FLAG_TIMER, fmt, ##args); \
} while(0)

/*SSM DEBUG end*/

/* global APIs */
int32
ssm_set_synce_global_enable(uint32 synce_enable);

int32
ssm_set_synce_local_priority(uint32 local_priority);

int32
ssm_set_synce_system_quality(uint32 option_mode, uint32 system_quality);

int32
ssm_set_synce_device_role(uint32 device_role);

int32
ssm_set_synce_ssm_enable(uint32 ssm_enable);

int32
ssm_set_synce_ssm_extend_enable(uint32 extend_enable);

int32
ssm_set_synce_holdover_enable(uint32 holdover_en);

int32
ssm_set_synce_selected_ifindex(uint32 ifindex);

int32
ssm_set_synce_holdoff_time(uint32 holdoff_time);

int32
ssm_set_synce_wait_to_restore_time(uint32 wtr_time);

int32
ssm_set_synce_clock_id(ssm_clock_id_t clock_id);

/* global APIs end*/

/*Interface API*/
int32
ssm_set_if_synce_enable(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_ssm_pdu_direction(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_priority(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_replace_clockid_enable(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_dnu_group_id(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_tx_quality_value(tbl_ssm_port_t* p_ssm_port);

int32
ssm_set_if_rx_quality_value(tbl_ssm_port_t* p_ssm_port);


#endif /* !__SSM_API_H__ */

