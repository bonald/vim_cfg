
/**
 @file sys_usw_dma_priv.h

 @date 2012-11-30

 @version v2.0

*/
#ifndef _SYS_USW_DMA_DEBUG_H
#define _SYS_USW_DMA_DEBUG_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_debug.h"
#include "ctc_const.h"

extern int32
sys_usw_dma_show_stats(uint8 lchip);

extern int32
sys_usw_dma_show_status(uint8 lchip);
extern int32
sys_usw_dma_show_tx_list(uint8 lchip);
extern int32
sys_usw_dma_dynamic_info(uint8 lchip);

extern int32
sys_usw_dma_show_desc(uint8 lchip, uint8 chan_id, uint32 start_idx, uint32 end_idx);

extern int32
sys_usw_dma_clear_stats(uint8 lchip);
extern int32
sys_usw_dma_function_pause(uint8 lchip, uint8 chan_id, uint8 en);

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#ifdef __cplusplus
}
#endif

#endif

