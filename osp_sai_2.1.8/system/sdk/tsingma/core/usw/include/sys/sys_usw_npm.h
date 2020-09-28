#if (FEATURE_MODE == 0)
/**
 @file sys_usw_npm.h

 @date 2014-10-28

 @version v3.0

Macro, data structure for system common operations

*/

#ifndef _SYS_USW_NPM
#define _SYS_USW_NPM
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_npm.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
#define SYS_NPM_MAX_SESSION_NUM 8

#define SYS_NPM_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(npm, npm, NPM_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

struct sys_npm_master_s
{
    uint64 total_delay_all[SYS_NPM_MAX_SESSION_NUM];
    uint64 total_far_delay_all[SYS_NPM_MAX_SESSION_NUM];

    uint64 tx_pkts_all[SYS_NPM_MAX_SESSION_NUM];
    uint64 rx_pkts_all[SYS_NPM_MAX_SESSION_NUM];
    uint64 tx_bytes_all[SYS_NPM_MAX_SESSION_NUM];
    uint64 rx_bytes_all[SYS_NPM_MAX_SESSION_NUM];
    uint64 max_delay[SYS_NPM_MAX_SESSION_NUM];
    uint64 min_delay[SYS_NPM_MAX_SESSION_NUM];
    uint64 max_jitter[SYS_NPM_MAX_SESSION_NUM];
    uint64 min_jitter[SYS_NPM_MAX_SESSION_NUM];
    uint64 total_jitter_all[SYS_NPM_MAX_SESSION_NUM];
    uint16 config_dmm_bitmap;
    uint16 tx_dmm_en_bitmap;
    sal_mutex_t*  npm_mutex;
    uint8  is_ntp_ts[SYS_NPM_MAX_SESSION_NUM];
};
typedef struct sys_npm_master_s sys_npm_master_t;


extern int32
sys_usw_npm_set_config(uint8 lchip, ctc_npm_cfg_t* p_cfg);

extern int32
sys_usw_npm_get_config(uint8 lchip, ctc_npm_cfg_t* p_cfg);

extern int32
sys_usw_npm_set_transmit_en(uint8 lchip, uint8 session_id, uint8 enable);

extern int32
sys_usw_npm_get_stats(uint8 lchip, uint8 session_id, ctc_npm_stats_t* p_stats);

extern int32
sys_usw_npm_set_global_config(uint8 lchip, ctc_npm_global_cfg_t* p_npm);

extern int32
sys_usw_npm_get_global_config(uint8 lchip, ctc_npm_global_cfg_t* p_npm);

extern int32
sys_usw_npm_clear_stats(uint8 lchip, uint8 session_id);

extern int32
sys_usw_npm_init(uint8 lchip);

extern int32
sys_usw_npm_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif

