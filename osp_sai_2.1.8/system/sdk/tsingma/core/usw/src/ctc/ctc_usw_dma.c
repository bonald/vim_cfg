/**
 @file ctc_usw_dma.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-30

 @version v2.0

 This file define ctc functions

*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_packet.h"
#include "ctc_learning_aging.h"
#include "ctc_usw_dma.h"
#include "sys_usw_dma.h"
#include "sys_usw_common.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
*
* Function
*
*****************************************************************************/

/**
 @brief Init dma module

 @param[in] dma_global_cfg      packet DMA, table DMA, learning DMA and stats DMA information

 @return CTC_E_XXX

*/
int32
ctc_usw_dma_init(uint8 lchip, ctc_dma_global_cfg_t* dma_global_cfg)
{
    ctc_dma_global_cfg_t global_cfg;
    uint8 index = 0;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    if (dma_global_cfg == NULL)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_dma_global_cfg_t));

        CTC_BIT_SET(global_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
        CTC_BIT_SET(global_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
        CTC_BIT_SET(global_cfg.func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);
        CTC_BIT_SET(global_cfg.func_en_bitmap, CTC_DMA_FUNC_IPFIX);

        global_cfg.pkt_rx_chan_num = 4;

        for (index = 0; index < 4; index++)
        {
            global_cfg.pkt_rx[index].desc_num  = 1024;
            global_cfg.pkt_rx[index].priority      = SAL_TASK_PRIO_DEF;
            global_cfg.pkt_rx[index].dmasel      = 0;
            global_cfg.pkt_rx[index].data         = 256;
        }

        global_cfg.pkt_tx.desc_num = 32;
        global_cfg.pkt_tx.priority = SAL_TASK_PRIO_DEF;
        global_cfg.pkt_tx.dmasel = 0;

        dma_global_cfg = &global_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dma_init(lchip, dma_global_cfg));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_dma_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_dma_deinit(lchip));
    }

    return CTC_E_NONE;
}

