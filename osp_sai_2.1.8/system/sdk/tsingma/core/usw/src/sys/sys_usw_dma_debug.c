/**
 @file sys_usw_dma_debug.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-10-23

 @version v2.0

 This file define sys functions

*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "dal.h"
#include "ctc_packet.h"
#include "ctc_interrupt.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_packet.h"
#include "sys_usw_dma.h"
#include "sys_usw_dma_priv.h"
#include "sys_usw_interrupt.h"

extern sys_dma_master_t* p_usw_dma_master[CTC_MAX_LOCAL_CHIP_NUM];
extern dal_op_t g_dal_op;

extern int32
sys_usw_dma_function_pause(uint8 lchip, uint8 func_type, uint8 en);
extern int32
sys_usw_dma_sync_pkt_rx_stats(uint8 lchip);
extern int32
sys_usw_dma_sync_pkt_tx_stats(uint8 lchip);
extern int32
_sys_usw_dma_init_thread(uint8 lchip, uint8 start_chan, uint8 cur_chan, uint16 prio);
extern void
_sys_usw_dma_wr_thread(void* param);

STATIC void
_sys_usw_show_desc_info(uint8 lchip, sys_dma_chan_t* p_chan_info, uint32 index)
{
    sys_dma_desc_t* p_desc = NULL;
    DsDesc_m* p_desc_mem = NULL;
    uint32 timestamp[2] = {0};
    uint64 tm_ns = 0;
    uint32 tm_s = 0;
    uint64 phy_addr = 0;

    p_desc = &(p_chan_info->p_desc[index]);

    p_desc_mem = &(p_desc->desc_info);
    GetDsDescEncap(A, timestamp_f, p_desc_mem, timestamp);
    tm_ns = timestamp[1];
    tm_ns = (tm_ns << 32);
    tm_ns |= timestamp[0];

    tm_s = tm_ns/1000000000;
    tm_ns = tm_ns - tm_s*1000000000;

    COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr, (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-4d %-10p %-10p %-5d %-5d %-2d %-1d %-2d %-3d %-3d %-1d %-1d %-1d %-1d %-1d 0x%-8x 0x%-8x\n",
        index,
        p_desc,
        g_dal_op.phy_to_logic(lchip, phy_addr),
        GetDsDescEncap(V, realSize_f, p_desc_mem),
        GetDsDescEncap(V, cfgSize_f, p_desc_mem),
        GetDsDescEncap(V, dataStruct_f, p_desc_mem),
        GetDsDescEncap(V, done_f, p_desc_mem),
        GetDsDescEncap(V, error_f, p_desc_mem),
        GetDsDescEncap(V, dataError_f, p_desc_mem),
        GetDsDescEncap(V, descError_f, p_desc_mem),
        GetDsDescEncap(V, u2_info_timeout_f, p_desc_mem),
        GetDsDescEncap(V, pause_f, p_desc_mem),
        GetDsDescEncap(V, intrMask_f, p_desc_mem),
        GetDsDescEncap(V, u1_pkt_eop_f, p_desc_mem),
        GetDsDescEncap(V, u1_pkt_sop_f, p_desc_mem),
        (uint32)tm_ns,
        tm_s
        );
    return;
}

#if 0
STATIC int32
_sys_usw_dma_show_dyn(uint8 lchip, uint8 chan_id, uint8 dmasel)
{
    DmaDynInfo_m dyn_info;
    uint32 cmd = 0;
    uint32 tbl_id = 0;

    sal_memset(&dyn_info, 0, sizeof(DmaDynInfo_m));

    tbl_id = DmaDynInfo_t + dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &dyn_info));

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d%-9d%-9d%-9d%-9d%-9d%-6d%-6d%-6d%-6d%-6d\n",
        chan_id,
        GetDmaDynInfo(V, ringWrPtr_f, &dyn_info),
        GetDmaDynInfo(V, ringRdPtr_f, &dyn_info),
        GetDmaDynInfo(V, cacheWrPtr_f, &dyn_info),
        GetDmaDynInfo(V, cacheRdPtr_f, &dyn_info),
        GetDmaDynInfo(V, ringTailPtr_f, &dyn_info),
        GetDmaDynInfo(V, fetchCnt_f, &dyn_info),
        GetDmaDynInfo(V, cacheCnt_f, &dyn_info),
        GetDmaDynInfo(V, dataSeq_f, &dyn_info),
        GetDmaDynInfo(V, procCnt_f, &dyn_info),
        GetDmaDynInfo(V, resCnt_f, &dyn_info)
        );

    return 0;
}

#endif
STATIC char*
_sys_usw_dma_get_chan_desc(uint8 lchip, uint8 chan_id)
{
    switch(GET_CHAN_TYPE(chan_id))
    {
    case DRV_DMA_PACKET_RX0_CHAN_ID:
        return "PtkRx0";
    case DRV_DMA_PACKET_RX1_CHAN_ID:
        return "PtkRx1";
    case DRV_DMA_PACKET_RX2_CHAN_ID:
        return "PtkRx2";
    case DRV_DMA_PACKET_RX3_CHAN_ID:
        return "PtkRx3";
    case DRV_DMA_PACKET_TX0_CHAN_ID:
        return "PtkTx0";
    case DRV_DMA_PACKET_TX1_CHAN_ID:
        return "PtkTx1";
    case DRV_DMA_PACKET_TX2_CHAN_ID:
        return "PtkTx2";
    case DRV_DMA_PACKET_TX3_CHAN_ID:
        return "PtkTx3";
    case DRV_DMA_TBL_WR_CHAN_ID:
        return "TblWr";
    case DRV_DMA_TBL_RD_CHAN_ID:
        return "Tblrd";
    case DRV_DMA_PORT_STATS_CHAN_ID:
        return "Stats";
    case DRV_DMA_FLOW_STATS_CHAN_ID:
        return "FlowStats";
    case DRV_DMA_REG_MAX_CHAN_ID:
        return "QueStats";
    case DRV_DMA_TBL_RD1_CHAN_ID:
        return "Tblrd0";
    case DRV_DMA_TBL_RD2_CHAN_ID:
        return "Tblrd1";
    case DRV_DMA_LEARNING_CHAN_ID:
        return "Learning";
    case DRV_DMA_HASHKEY_CHAN_ID:
        return "HashKey";
    case DRV_DMA_IPFIX_CHAN_ID:
        return "Ipfix";
    case DRV_DMA_SDC_CHAN_ID:
        return "SDC";
    case DRV_DMA_MONITOR_CHAN_ID:
        return "Monitor";

    default:
        return "";
    }

    return "";
}

STATIC int32
_sys_usw_dma_show_chan(uint8 lchip, uint8 chan_id)
{
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    sys_dma_chan_t* p_chan_info = NULL;
    DmaCtlTab_m ctl_tab;
    uint32 used_mem = 0;
    uint16 valid_desc_nmu = 0;
    uint8 sync_chan = 0;
    uint8 no_sync = 0;
    uint8 enable = 0;

    sys_dma_thread_t* p_thread_info = NULL;

    if (chan_id >= SYS_DMA_MAX_CHAN_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_DMA_INIT_CHECK(lchip);

    sys_usw_dma_get_chan_en(lchip, chan_id, &enable);
    if (!enable)
    {
        return CTC_E_NONE;
    }

    p_chan_info = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[chan_id]);

    tbl_id = DmaCtlTab_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ctl_tab));

    valid_desc_nmu = GetDmaCtlTab(V, vldNum_f, &ctl_tab);
    used_mem = sizeof(sys_dma_desc_t) * p_chan_info->desc_depth + p_chan_info->data_size * p_chan_info->desc_depth;

    /*Get sync chan*/
    if (p_chan_info->sync_en)
    {
        sync_chan = p_chan_info->sync_chan;

        /*get sync thread info*/
        p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, sync_chan);
        if (!p_thread_info)
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        no_sync = 1;
    }

    if (!no_sync)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d%-10s%-10d%-7d%-8d%p   %-15d%-10s%4u\n",
        p_chan_info->channel_id,
        _sys_usw_dma_get_chan_desc(lchip,chan_id),
        valid_desc_nmu,
        p_chan_info->desc_depth,
        p_chan_info->current_index,
        (void*)p_chan_info->mem_base,
        used_mem,
        p_thread_info->desc,
        p_thread_info->prio);
    }
    else
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6d%-10s%-10d%-7d%-8d%p   %-15d%-10s%3s\n",
        p_chan_info->channel_id,
        _sys_usw_dma_get_chan_desc(lchip,chan_id),
        valid_desc_nmu,
        p_chan_info->desc_depth,
        p_chan_info->current_index,
        (void*)p_chan_info->mem_base,
        used_mem,
        "-",
        "-");
    }
    return CTC_E_NONE;
}

int32
sys_usw_dma_show_status(uint8 lchip)
{
    uint8 index = 0;
    uint32 dma_mem_size = 0;
    uint32 used_size = 0;
    uint32 total_used_size = 0;
    dal_dma_info_t dma_info;
    sys_dma_chan_t* p_chan_info = NULL;

    SYS_DMA_INIT_CHECK(lchip);
    sal_memset(&dma_info, 0, sizeof(dal_dma_info_t));
    if (p_usw_dma_master[lchip] == NULL)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DMA does not inited\n");
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    /*get total dma memory*/
    dal_get_dma_info(lchip, &dma_info);
    dma_mem_size = (uint32)dma_info.size;

    /*get total used dma memory*/
    for(index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {
        if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index))
        {
            p_chan_info = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[index]);
            used_size = sizeof(sys_dma_desc_t) * p_chan_info->desc_depth + p_chan_info->data_size * p_chan_info->desc_depth;

            total_used_size += used_size;
        }
    }

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-25s%-3s%-4d%-4s\n", " DMA Total Memory", ":", (dma_mem_size/1024), " K");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-25s%-3s%-4d%-4s\n", " DMA Used Memory", ":", (total_used_size/1024), " K");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-25s%-3s%p\n", " DMA Virtual Base", ":", dma_info.virt_base);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-25s%-3s0x%x\n", " DMA Physical Base(Low)", ":", dma_info.phy_base);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-25s%-3s0x%x\n\n", " DMA Physical Base(High)", ":", dma_info.phy_base_hi);

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-10s%-10s%-7s%-8s%-10s%-15s%-10s%-15s\n",
        "Chan", "Function", "ValidNum", "Depth", "CurIdx", "MemBase", "UsedMem(Byte)", "Thread", "Pri");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");

    for(index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {
        if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index))
        {
            _sys_usw_dma_show_chan(lchip, index);
        }
    }
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}
#if 0
int32
sys_usw_dma_dynamic_info(uint8 lchip)
{
    uint8 index = 0;

    SYS_DMA_INIT_CHECK(lchip);
    if (p_usw_dma_master[lchip] == NULL)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DMA does not inited\n");
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-20s", "CRdPtr-CacheRdPtr");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", "RTaiPtr-RTailPtr");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s\n", "FCnt-FetchCnt");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-20s", "CCnt-CacheCnt");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", "DSeq-DataSeq");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s\n", "PCnt-ProcCnt");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-20s", "RCnt-ResCnt");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", "RWrPtr-RingWrPtr");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s", "RRdPtr-RingRdPtr");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s\n", "CWrPtr-CacheWrPtr");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-6s%-9s%-9s%-9s%-9s%-9s%-6s%-6s%-6s%-6s%-6s\n",
        "Chan", "RWrPtr", "RRdPtr", "CWrPtr", "CRdPtr", "RTaiPtr", "FCnt", "CCnt", "DSeq", "PCnt", "RCnt");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------------------------------\n");
    for(index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {

        if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index) && (p_usw_dma_master[lchip]->dma_chan_info[index].dmasel == 0) && (index != 9))
        {
            _sys_usw_dma_show_dyn(lchip, index, 0);

        }
    }
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

#endif
int32
sys_usw_dma_clear_pkt_stats(uint8 lchip, uint8 para)
{
    SYS_DMA_INIT_CHECK(lchip);

    if (para == 0)   /*clear all*/
    {
        sys_usw_dma_sync_pkt_rx_stats(lchip);
        sys_usw_dma_sync_pkt_tx_stats(lchip);
        sal_memset(&(p_usw_dma_master[lchip]->dma_stats), 0, sizeof(p_usw_dma_master[lchip]->dma_stats));
    }
    else if (para == 1)  /*clear rx*/
    {
        uint8 index = 0;
        sys_usw_dma_sync_pkt_rx_stats(lchip);
        for(index=0; index<4; index++ )
        {
            sal_memset(&p_usw_dma_master[lchip]->dma_stats[index], 0, sizeof(p_usw_dma_master[lchip]->dma_stats[index]));
        }
    }
    else      /*clear tx*/
    {
         uint8 index = 0;
         sys_usw_dma_sync_pkt_tx_stats(lchip);
         for(index=0; index<2; index++ )
         {
             sal_memset(&p_usw_dma_master[lchip]->dma_stats[index+4], 0, sizeof(p_usw_dma_master[lchip]->dma_stats[index]));
         }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_show_rx_stats(uint8 lchip)
{
    uint8 index = 0;
    uint64 good_pkt_cnt = 0;
    uint64 good_byte_cnt = 0;
    uint64 bad_pkt_cnt = 0;
    uint64 bad_byte_cnt = 0;

    CTC_ERROR_RETURN(sys_usw_dma_sync_pkt_rx_stats(lchip));

    SYS_DMA_DUMP("\n");
    SYS_DMA_DUMP("DMA Receive Stats:\n");
    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");
    if(DRV_IS_DUET2(lchip))
    {
        SYS_DMA_DUMP("%-6s%-21s%-21s%-21s%-21s\n", "Chan", "RecPktCnt", "TotalByte", "DropPktCnt", "ErrPktCnt");
    }
    else
    {
        SYS_DMA_DUMP("%-6s%-21s%-21s%-21s%-21s\n", "Chan", "GoodPktCnt", "GoodPktByte", "BadPktCnt", "BadPktByte");
    }
    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");
    for(index = 0; index < 4; index++)
    {
        SYS_DMA_DUMP("%-6d%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"\n", index,
                        p_usw_dma_master[lchip]->dma_stats[index].u1.good_pkt_cnt, p_usw_dma_master[lchip]->dma_stats[index].u2.good_byte_cnt,
                        p_usw_dma_master[lchip]->dma_stats[index].u3.bad_pkt_cnt, p_usw_dma_master[lchip]->dma_stats[index].u4.bad_byte_cnt);

        good_pkt_cnt += p_usw_dma_master[lchip]->dma_stats[index].u1.good_pkt_cnt;
        good_byte_cnt += p_usw_dma_master[lchip]->dma_stats[index].u2.good_byte_cnt;
        bad_pkt_cnt += p_usw_dma_master[lchip]->dma_stats[index].u3.bad_pkt_cnt;
        bad_byte_cnt += p_usw_dma_master[lchip]->dma_stats[index].u4.bad_byte_cnt;
    }

    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");
    SYS_DMA_DUMP("%-6s%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"\n", "Total",good_pkt_cnt, good_byte_cnt,
                                                                                    bad_pkt_cnt, bad_byte_cnt);
    SYS_DMA_DUMP("\n");

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_dma_show_tx_stats(uint8 lchip)
{
    uint8 index = 0;
    uint64 good_pkt_cnt = 0;
    uint64 good_byte_cnt = 0;
    uint64 bad_pkt_cnt = 0;
    uint64 bad_byte_cnt = 0;
    uint8  chan_num = 0;

    CTC_ERROR_RETURN(sys_usw_dma_sync_pkt_tx_stats(lchip));
    SYS_DMA_DUMP("\n");
    SYS_DMA_DUMP("DMA Transmit Stats:\n");
    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");
    SYS_DMA_DUMP("%-6s%-21s%-21s%-21s%-21s\n", "Chan", "GoodPktCnt", "GoodPktByte", "BadPktCnt", "BadPktByte");
    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");

    chan_num = DRV_IS_DUET2(lchip)?2:4;
    for(index=0; index < chan_num; index++)
    {
        SYS_DMA_DUMP("%-6d%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"\n", index,
                        p_usw_dma_master[lchip]->dma_stats[index+4].u1.good_pkt_cnt, p_usw_dma_master[lchip]->dma_stats[index+4].u2.good_byte_cnt,
                        p_usw_dma_master[lchip]->dma_stats[index+4].u3.bad_pkt_cnt, p_usw_dma_master[lchip]->dma_stats[index+4].u4.bad_byte_cnt);

        good_pkt_cnt += p_usw_dma_master[lchip]->dma_stats[index+4].u1.good_pkt_cnt;
        good_byte_cnt += p_usw_dma_master[lchip]->dma_stats[index+4].u2.good_byte_cnt;
        bad_pkt_cnt += p_usw_dma_master[lchip]->dma_stats[index+4].u3.bad_pkt_cnt;
        bad_byte_cnt += p_usw_dma_master[lchip]->dma_stats[index+4].u4.bad_byte_cnt;
    }

    SYS_DMA_DUMP("------------------------------------------------------------------------------------\n");
    SYS_DMA_DUMP("%-6s%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"%-21"PRIu64"\n", "Total",good_pkt_cnt, good_byte_cnt,
                                                                                    bad_pkt_cnt, bad_byte_cnt);
    SYS_DMA_DUMP("\n");

    return CTC_E_NONE;
}

int32
sys_usw_dma_show_stats(uint8 lchip)
{
    SYS_DMA_INIT_CHECK(lchip);
    CTC_ERROR_RETURN(_sys_usw_dma_show_rx_stats(lchip));
    CTC_ERROR_RETURN(_sys_usw_dma_show_tx_stats(lchip));

    return CTC_E_NONE;
}

int32
sys_usw_dma_show_desc(uint8 lchip, uint8 chan_id, uint32 start_idx, uint32 end_idx)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    uint32 desc_dep = 0;
    uint32 index = 0;

    SYS_DMA_INIT_CHECK(lchip);

    if (!CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, chan_id))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DMA function is not enable!\n");
        return CTC_E_NONE;
    }

    if (SYS_DMA_MAX_CHAN_NUM <= chan_id)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_dma_chan =  (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[chan_id]);
    desc_dep =  p_dma_chan->desc_depth;

    if (start_idx > end_idx)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (end_idx >= desc_dep)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-22s", "P-Pause");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s", "DR-Data Error");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s\n", "D-Descriptor Done");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-22s", "ER-Total Error");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s", "CSize-Config Size");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s\n", "S-Descriptor SOP");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-22s", "RSize-Actual Size");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s", "T-Descriptor Timeout");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s\n", "M-Interrupt Mask");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-22s", "E-Descriptor EOP");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s", "DS-DataStruct(For Reg)");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-26s\n", "DeE-Descriptor Error");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-38s", "DAddr-Data Memory Address");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-27s\n", "CfgAddr-Descriptor Config Address");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-4s %-10s %-10s %-5s %-5s %-2s %-1s %-2s %-3s %-3s %-1s %-1s %-1s %-1s %-1s %-10s %-10s\n",
        "IDX", "DescAddr", "DAddr", "RSize", "CSize", "DS", "D", "ER", "DER", "DeE", "T", "P", "M", "E", "S", "TS(ns)", "TS(s)");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------------\n");

    for (index = start_idx; index <= end_idx; index++)
    {
        _sys_usw_show_desc_info(lchip, p_dma_chan, index);
    }

    return 0;
}

int32
sys_usw_dma_table_rw(uint8 lchip, uint8 is_read, uint32 addr, uint16 entry_len, uint16 entry_num, uint32* p_value)
{
    int32 ret = 0;
    sys_dma_tbl_rw_t tbl_cfg;

    SYS_DMA_INIT_CHECK(lchip);
    sal_memset(&tbl_cfg, 0, sizeof(sys_dma_tbl_rw_t));

    if (is_read)
    {
        tbl_cfg.rflag = 1;
        tbl_cfg.tbl_addr = addr;
        tbl_cfg.entry_len = entry_len;
        tbl_cfg.entry_num = entry_num;
        tbl_cfg.buffer = p_value;
        ret = sys_usw_dma_rw_table(lchip, &tbl_cfg);
    }
    else
    {
        tbl_cfg.rflag = 0;
        tbl_cfg.tbl_addr = addr;
        tbl_cfg.entry_len = entry_len;
        tbl_cfg.entry_num = entry_num;
        tbl_cfg.buffer = p_value;
        ret = sys_usw_dma_rw_table(lchip, &tbl_cfg);
    }


    return ret;
}

int32
sys_usw_dma_show_tx_list(uint8 lchip)
{
    int32 ret = 0;
    uint8 i = 0;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;

    SYS_DMA_INIT_CHECK(lchip);
    for(i  = 0; i < 2 ; i++)
    {
        if(i == 0)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------Hight priority-------------\n");
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-20s%-20s\n","mem_type","opf_idx","addr");
            p_tx_mem_info = (sys_dma_tx_mem_t*)(p_usw_dma_master[lchip]->tx_pending_list_H->head);
        }
        else
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------Low priority-------------\n");
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-20s%-20s\n","mem_type","opf_idx","addr");
            p_tx_mem_info = (sys_dma_tx_mem_t*)(p_usw_dma_master[lchip]->tx_pending_list_L->head);
        }
        while(p_tx_mem_info)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20d%-20d%-20p\n",p_tx_mem_info->mem_type, p_tx_mem_info->opf_idx, (void*)p_tx_mem_info->addr);
            p_tx_mem_info = (sys_dma_tx_mem_t*)p_tx_mem_info->node_head.next;
        }
    }

    return ret;
}

/*Only used for test tx timer*/
int32
sys_usw_dma_set_timer_init_en(uint8 lchip)
{
    uint32 cmd = 0;
    DmaStaticInfo_m static_info;
    uint16 desc_num = 0;
    sys_dma_desc_t* p_sys_desc_pad = NULL;
    DsDesc_m* p_desc = NULL;
    uint64 phy_addr = 0;
    uint32* logic_addr = NULL;
    uint8  chan_id[2] = {SYS_DMA_PKT_TX_TIMER_CHAN_ID, SYS_DMA_TBL_WR_CHAN_ID};
    uint8  index  = 0;
    uint8  mem_free = 1;
    sys_dma_thread_t* p_thread_info = NULL;
    char buffer[SAL_TASK_MAX_NAME_LEN]={0};

    SYS_DMA_INIT_CHECK(lchip);

    if (1 == p_usw_dma_master[lchip]->pkt_tx_timer_en)
    {
        return CTC_E_NONE;
    }

    if (!DRV_IS_DUET2(lchip))
    {
        p_usw_dma_master[lchip]->pkt_tx_timer_en = 1;
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX3_CHAN_ID);
        return CTC_E_NONE;
    }

    /*1. disable tx1 channel*/
    CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 0));
    CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, SYS_DMA_TBL_WR_CHAN_ID, 0));

    cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));
    SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
    SetDmaStaticInfo(V, ringBase_f, &static_info, 0);
    SetDmaStaticInfo(V, ringDepth_f, &static_info, 0);
    cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID,  cmd, &static_info));

    /*2. free channel resource*/
    for (index = 0; index < 2; index++)
    {
        mem_free = 1;
        p_sys_desc_pad = p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].p_desc;
        if (!p_sys_desc_pad)
        {
            continue;
        }

        p_sys_desc_pad = p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].p_desc;
        /*clear data*/
        for (desc_num = 0; desc_num < p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].desc_depth; desc_num++)
        {
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);
            COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);
            logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);
            if (logic_addr && mem_free)
            {
                g_dal_op.dma_free(lchip, logic_addr);
                if (index == 1)
                {
                    mem_free = 0;
                }
            }
        }

        g_dal_op.dma_free(lchip, p_sys_desc_pad);
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].p_desc = NULL;
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].mem_base = 0;
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].channel_id = 0;
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].desc_num = 0;
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].desc_depth = 0;
        p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].data_size = 0;
    }

    p_usw_dma_master[lchip]->pkt_tx_timer_en = 1;
    p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_TBL_WR_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_TBL_WR_CHAN_ID,
                 SYS_DMA_TBL_WR_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));

    p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, SYS_DMA_TBL_WR_CHAN_ID);
    if (!p_thread_info)
    {
        /*means no need to create sync thread*/
        return CTC_E_NONE;
    }
    else
    {
        /* create dma learning thread */
        sal_sprintf(buffer, "Dmawr-%d", lchip);
        sal_task_create(&p_thread_info->p_sync_task, buffer,
                              SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _sys_usw_dma_wr_thread, (void*)p_thread_info);
    }
    return 0;
}

