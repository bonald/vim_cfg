#if (FEATURE_MODE == 0)
/**
 @file sys_usw_sync_ether.h

 @date 2012-10-18

 @version v2.0

*/
#ifndef _SYS_USW_SYNC_ETHER_H
#define _SYS_USW_SYNC_ETHER_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_sync_ether.h"

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_sync_ether_init(uint8 lchip);

extern int32
sys_usw_sync_ether_set_cfg(uint8 lchip, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg);

extern int32
sys_usw_sync_ether_get_cfg(uint8 lchip, uint8 sync_ether_clock_id, ctc_sync_ether_cfg_t* p_sync_ether_cfg);

extern int32
sys_usw_sync_ether_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif

