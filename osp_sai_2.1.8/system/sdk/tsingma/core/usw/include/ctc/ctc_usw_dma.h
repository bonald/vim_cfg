/**
 @file ctc_usw_dma.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2009-10-28

 @version v2.0

\p
DMA represent Direct Memory Access. For The Module Asic have 2 Dma Controllers, which perform data transmission between
chip ram and system memory. Dma controller can only use physical address, but CPU using virtual address, So before DMA module init
DAL module must provide several interfaces below:
\b
\d dma_alloc: Alloc Dma memory
\d dma_free: Free Dma memory
\d dal_logic_to_phy: Convert logic address to physical address
\d dal_phy_to_logic: Convert physical address to logic address

\p
\b
DMA module is resource module in SDK, it provides methods for other module using. DMA can provide several functions below:
\d Packet receive and transmit
\d Table read/write
\d Port stats
\d Learning and aging
\d IPFIX
\d Monitor
\d SDC
\d HashDump

*/

#ifndef _CTC_USW_DMA_H
#define _CTC_USW_DMA_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_dma.h"

/**
 @brief Init dma module

 @param[in] lchip    local chip id

 @param[in] dma_global_cfg      packet DMA, table DMA, learning DMA and stats DMA information

 @remark[D2.TM] init dma with dma global cfg

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dma_init(uint8 lchip, ctc_dma_global_cfg_t* dma_global_cfg);

/**
 @brief Deinit dma module

 @param[in] lchip    local chip id

 @remark[D2.TM] deinit dma with lchip

 @return CTC_E_XXX

*/
extern int32
ctc_usw_dma_deinit(uint8 lchip);

#ifdef __cplusplus
}
#endif

#endif /*end of _CTC_USW_DMA_H*/

