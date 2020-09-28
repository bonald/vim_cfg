/**
 @file ctc_usw_interrupt.c

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
#include "dal_mpool.h"
#include "ctc_hash.h"
#include "ctc_packet.h"
#include "ctc_l2.h"
#include "ctc_port.h"
#include "ctc_interrupt.h"
#include "ctc_linklist.h"
#include "ctc_warmboot.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_packet.h"
#include "sys_usw_dma.h"
#include "sys_usw_register.h"
#include "sys_usw_dma_priv.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_opf.h"
#include "sys_usw_dmps.h"
#include "drv_api.h"
#include "sal_memmngr.h"
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
sys_dma_master_t* p_usw_dma_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern dal_op_t g_dal_op;
extern int32 drv_get_host_type(uint8 lchip);
int32 sys_usw_dma_performance_test(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx);
int32 sys_usw_dma_clear_pkt_stats(uint8 lchip, uint8 para);
extern int32 sys_usw_l2_flush_fdb(uint8 lchip, ctc_l2_flush_fdb_t* pf);
extern int32 _sys_usw_packet_txinfo_to_rawhdr(uint8 lchip, ctc_pkt_info_t* p_tx_info, uint32* p_raw_hdr_net, uint8 is_skb_buf, uint8 mode, uint8* data);
extern int32
drv_usw_ftm_get_tcam_memory_info(uint8 lchip, uint32 mem_id, uint32* p_mem_addr, uint32* p_entry_num, uint32* p_entry_size);

#define SYS_USW_DMA_STATS_WORD 4
#define SYS_USW_STATS_SIZE 0x240
#define SYS_USW_DMA_PACKETS_PER_INTR 1
#define SYS_USW_DMA_TCAM_SCAN_NUM (DRV_ENUM(DRV_DMA_TCAM_SCAN_DESC_NUM))
/* temply only process slice0 mac */
#define SYS_USW_GET_MAC_ADDR(id, type, addr)\
{\
    if (type == 2)\
    {\
        if (id < 40)\
        {\
            addr = (0x20010000+(id/16)*0x10000+((id%16)/4)*0x2000+0x240*(id%4));\
        }\
        else\
        {\
            addr = (0x20040000+((id-40)/4)*0x2000+0x240*((id-40)%4));\
        }\
    }\
    else \
    {\
        addr = (0x20010000+(id/4)*0x2000);\
    }\
}

#define __STUB__

/****************************************************************************
*
* Function
*
*****************************************************************************/
int32
_sys_usw_dma_pkt_finish_cb(uint8 lchip)
{
    sys_dma_chan_t *p_dma_chan = NULL;
    drv_work_platform_type_t platform_type;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 valid_cnt = 0;
    DmaCtlTab_m ctl_tab;
    uint8 chan_id = 0;

    for (chan_id = SYS_DMA_PACKET_TX0_CHAN_ID; chan_id <= SYS_DMA_PACKET_TX1_CHAN_ID; chan_id++)
    {
        p_dma_chan = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[chan_id]);
        if (!p_dma_chan->pkt_knet_en)
        {
            continue;
        }

        tbl_id = DmaCtlTab_t + p_dma_chan->dmasel;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ctl_tab));

        drv_get_platform_type(lchip, &platform_type);
        if (platform_type == HW_PLATFORM)
        {
            valid_cnt = 1;
        }
        else
        {
            valid_cnt = GetDmaCtlTab(V, vldNum_f, &ctl_tab);
            valid_cnt += 1;
        }
        SetDmaCtlTab(V, vldNum_f, &ctl_tab, valid_cnt);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ctl_tab));

        p_dma_chan->current_index++;
    }

    return CTC_E_NONE;
}

int32
sys_usw_dma_get_chan_type(uint8 lchip, uint8 chan_id)
{
    uint32 type = 0;

    for (type = DRV_DMA_PACKET_RX0_CHAN_ID; type < DRV_DMA_MAX_CHAN_ID; type++)
    {
        if (chan_id == DRV_ENUM(type))
        {
            return type;
        }
    }

    return DRV_DMA_MAX_CHAN_ID;
}


STATIC int32
_sys_usw_dma_get_mac_address(uint8 lchip, uint8 mac_type, uint16 mac_id, uint32* p_addr)
{
    uint32 start_addr = 0;
    uint32 tbl_id = 0;
    uint32 index = 0;

    tbl_id = QuadSgmacStatsRam0_t + mac_id/4;
    switch (mac_type)
    {
        case CTC_PORT_SPEED_1G:
        case CTC_PORT_SPEED_10G:
        case CTC_PORT_SPEED_2G5:
        case CTC_PORT_SPEED_20G:
            index = 40*(mac_id%4);
            break;

        case CTC_PORT_SPEED_40G:
        case CTC_PORT_SPEED_100G:
            index = 0;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, tbl_id, index, &start_addr));
    *p_addr = start_addr;
    return CTC_E_NONE;

}


/* get WORD in chip */
STATIC int32
_sys_usw_dma_rw_get_words_in_chip(uint8 lchip, uint16 entry_words, uint32* words_in_chip)
{
    if (1 == entry_words)
    {
        *words_in_chip = 1;
    }
    else if (2 == entry_words)
    {
        *words_in_chip = 2;
    }
    else if ((3 <= entry_words) && (entry_words <= 4))
    {
        *words_in_chip = 4;
    }
    else if ((5 <= entry_words) && (entry_words <= 8))
    {
        *words_in_chip = 8;
    }
    else if ((9 <= entry_words) && (entry_words <= 16))
    {
        *words_in_chip = 16;
    }
    else if ((17 <= entry_words) && (entry_words <= 32))
    {
        *words_in_chip = 32;
    }
    else if ((33 <= entry_words) && (entry_words <= 64))
    {
        *words_in_chip = 64;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_dma_tx_free_tx_mem(uint8 lchip, void* data_addr)
{
    uint8         unlocked = 0;
    sys_usw_opf_t opf;
    sys_dma_tx_mem_t tx_mem_info;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;

    if(NULL == data_addr)
    {
        return CTC_E_NONE;
    }
    sal_memset(&tx_mem_info, 0, sizeof(sys_dma_tx_mem_t));
    tx_mem_info.addr = (uintptr)data_addr;
    sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
    p_tx_mem_info = ctc_hash_lookup(p_usw_dma_master[lchip]->dma_tx_mem_entry, &tx_mem_info);

    if(p_tx_mem_info)
    {
        ctc_pkt_tx_t* tx_pkt = p_tx_mem_info->tx_pkt;
        ctc_pkt_callback callback =  p_tx_mem_info->callback;
        void*           user_data = p_tx_mem_info->user_data;

        if(p_tx_mem_info->internal_alloc)
        {
            if(0 == p_tx_mem_info->mem_type)
            {
                sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
                opf.pool_index = 0;
                opf.pool_type  = p_usw_dma_master[lchip]->opf_type_dma_tx_mem;
                sys_usw_opf_free_offset(lchip, &opf, 1, p_tx_mem_info->opf_idx);
            }
            else
            {
                if(data_addr)
                {
                    g_dal_op.dma_free(lchip, data_addr);
                }
            }

            if(NULL == ctc_hash_remove(p_usw_dma_master[lchip]->dma_tx_mem_entry, p_tx_mem_info))
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ctc_hash_remove error \n");
                sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
                return CTC_E_NOT_EXIST;
            }
            mem_free(p_tx_mem_info);
        }

        if(callback)
        {/*async, sys_usw_dma_tx_free must be called in callback function when zero copy*/
            sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
            unlocked = 1;
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " async free, [%p]\n", user_data);
            (callback)(tx_pkt,user_data);
        }
    }
    if(!unlocked)
    {
        sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_clear_desc(uint8 lchip, sys_dma_chan_t* p_dma_chan, uint32 sop_index, uint32 buf_count, void* data_addr)
{
    uint32 i = 0;
    uint32 desc_index = 0;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 tbl_id = 0;
    drv_work_platform_type_t platform_type;
    DmaCtlTab_m ctl_tab;
    uint32 cmd = 0;
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_INIT_CHECK(lchip);

    p_base_desc = p_dma_chan->p_desc;

    /*free old mem hang on desc */
    CTC_ERROR_RETURN(_sys_usw_dma_tx_free_tx_mem(lchip, data_addr));

    drv_get_platform_type(lchip, &platform_type);

    desc_index = sop_index;
    for (i = 0; i < buf_count; i++)
    {
        p_desc = &(p_base_desc[desc_index].desc_info);

        SetDsDescEncap(V, done_f, p_desc, 0);
        SetDsDescEncap(V, u1_pkt_sop_f, p_desc, 0);
        SetDsDescEncap(V, u1_pkt_eop_f, p_desc, 0);
        SetDsDescEncap(V, reserved0_f, p_desc, 0);
        if ((1 == SDK_WORK_PLATFORM) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))
        {
            /*Uml need clear realsize*/
            SetDsDescEncap(V, realSize_f, p_desc, 0);
        }

        /*flush dma after write*/
        if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }

        desc_index++;
        if (desc_index >= p_dma_chan->desc_depth)
        {
            desc_index = 0;
        }
    }


    if (p_dma_chan->channel_id != SYS_DMA_PACKET_TX0_CHAN_ID
        && p_dma_chan->channel_id != SYS_DMA_PACKET_TX1_CHAN_ID
        && p_dma_chan->channel_id != SYS_DMA_PACKET_TX2_CHAN_ID
        && p_dma_chan->channel_id != SYS_DMA_PACKET_TX3_CHAN_ID)
    {
        /*return desc resource to DmaCtl */
        tbl_id = DmaCtlTab_t + p_dma_chan->dmasel;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_dma_chan->channel_id, cmd, &ctl_tab);
        if  ((0 == SDK_WORK_PLATFORM) && (0 == SDK_WORK_ENV))
        {
            SetDmaCtlTab(V, vldNum_f, &ctl_tab, buf_count);
        }
        else
        {
            SetDmaCtlTab(V, vldNum_f, &ctl_tab, p_dma_chan->desc_depth);
        }
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_dma_chan->channel_id, cmd, &ctl_tab);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_set_tx_chip(uint8 lchip, DsDesc_m* p_tx_desc_mem, sys_dma_tx_mem_t* p_tx_mem_info, sys_dma_chan_t* p_dma_chan)
{
    uint32 phy_addr = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    drv_work_platform_type_t platform_type;
    uint32 valid_cnt = 0;
    DmaCtlTab_m ctl_tab;
    int32 ret = CTC_E_NONE;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    phy_addr = g_dal_op.logic_to_phy(lchip, (void*)p_tx_mem_info->addr);
    SetDsDescEncap(V, u1_pkt_eop_f, p_tx_desc_mem, 1);
    SetDsDescEncap(V, u1_pkt_sop_f, p_tx_desc_mem, 1);
    SetDsDescEncap(V, cfgSize_f, p_tx_desc_mem, p_tx_mem_info->pkt_len);
    SetDsDescEncap(V, memAddr_f, p_tx_desc_mem, (phy_addr >> 4));
    /*flush dma after write*/
    if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
    {
        g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tx_desc_mem), sizeof(DsDesc_m));
    }

    /* tx valid num */
    tbl_id = DmaCtlTab_t + p_dma_chan->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_dma_chan->channel_id, cmd, &ctl_tab));

    drv_get_platform_type(lchip, &platform_type);
    if (platform_type == HW_PLATFORM)
    {
        valid_cnt = 1;
    }
    else
    {
        valid_cnt = GetDmaCtlTab(V, vldNum_f, &ctl_tab);
        valid_cnt += 1;
    }
    SetDmaCtlTab(V, vldNum_f, &ctl_tab, valid_cnt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_dma_chan->channel_id, cmd, &ctl_tab));

    /*update desc used info */
    p_dma_chan->p_desc_used[p_dma_chan->current_index] = 1;

    /* next descriptor, tx_desc_index: 0~tx_desc_num-1*/
    p_dma_chan->current_index =
        (p_dma_chan->current_index == (p_dma_chan->desc_depth - 1)) ? 0 : (p_dma_chan->current_index + 1);

    return ret;
}

STATIC int32
_sys_usw_dma_wait_desc_finish_tx(uint8 lchip, DsDesc_m* p_tx_desc_mem,sys_dma_chan_t* p_dma_chan)
{
    uint32  cnt = 0;
    int32   ret = CTC_E_NONE;
    bool    done = FALSE;
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        /* check last transmit is done */
        if (p_dma_chan->p_desc_used[p_dma_chan->current_index])
        {
            while(cnt < 100)
            {
                SYS_DMA_INIT_CHECK(lchip);
                /*inval dma before read*/
                if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
                {
                    g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tx_desc_mem), sizeof(DsDesc_m));
                }
                if (GetDsDescEncap(V, done_f, p_tx_desc_mem))
                {
                    done = TRUE;
                    break;
                    /* last transmit is done */
                }
#ifndef PACKET_TX_USE_SPINLOCK
                sal_task_sleep(1);
#else
                sal_udelay(1000);
#endif
                cnt++;
            }

            if (!done)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "last transmit is not done,%d\n", p_dma_chan->current_index);
                p_dma_chan->p_desc_used[p_dma_chan->current_index] = 0;
                ret = CTC_E_DMA;
            }
        }
    return ret;
}
STATIC uint32
_sys_usw_dma_tx_mem_key(sys_dma_tx_mem_t* p_tx_mem)
{
    uint8* k = NULL;
    k = (uint8*)&(p_tx_mem->addr);

    return ctc_hash_caculate(sizeof(uintptr), k);
}
STATIC bool
_sys_usw_dma_tx_mem_compare(sys_dma_tx_mem_t* stored_node, sys_dma_tx_mem_t* lkup_node)
{
    if (!stored_node || !lkup_node)
    {
        return TRUE;
    }
    if (!sal_memcmp((uint8*)&stored_node->addr, (uint8*)&lkup_node->addr, sizeof(uintptr)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_dma_get_tx_session_from_desc_index(uint8 lchip, uint16 desc_idx, uint16* p_id)
{
    uint16 index = 0;
    uint8 find_flag = 0;
    SYS_DMA_INIT_CHECK(lchip);

    for (index = 0; index < SYS_PKT_MAX_TX_SESSION; index++)
    {
        if ((p_usw_dma_master[lchip]->tx_session[index].desc_idx == desc_idx) && p_usw_dma_master[lchip]->tx_session[index].state)
        {
            find_flag = 1;
            *p_id = index;
            break;
        }
    }

    return (find_flag?CTC_E_NONE:CTC_E_NOT_EXIST);
}

void*
_sys_usw_dma_tx_alloc(uint8 lchip, uint32 pkt_size, uint8 internal)
{
    sys_usw_opf_t opf;
    uint32 mem_idx = 0;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;
    int32 ret = CTC_E_NONE;
    void* p_addr = NULL;
    void* data_addr = NULL;
    uint8 alloc_flag1 = 0;
    uint8 alloc_flag2 = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%d:%s() \n",__LINE__,__FUNCTION__);

    if(pkt_size > 9600)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "pkt_size can't > 9600 !\n");
        return NULL;
    }

    p_tx_mem_info = mem_malloc(MEM_DMA_MODULE, sizeof(sys_dma_tx_mem_t));
    if (!p_tx_mem_info)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory for p_tx_mem_info\n");
        return NULL;
    }
    alloc_flag1 = 1;
    sal_memset(p_tx_mem_info, 0, sizeof(sys_dma_tx_mem_t));
    p_tx_mem_info->opf_idx = SYS_DMA_TX_PKT_MEM_NUM;
    p_tx_mem_info->internal_alloc = internal;
    if(SYS_DMA_TX_PKT_MEM_SIZE >= pkt_size)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = 0;
        opf.pool_type  = p_usw_dma_master[lchip]->opf_type_dma_tx_mem;
        ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &mem_idx);
        if(ret == CTC_E_NONE)
        {
            p_tx_mem_info->opf_idx = mem_idx;
            p_tx_mem_info->mem_type = 0;
            p_addr = (void*)p_usw_dma_master[lchip]->tx_mem_pool[mem_idx];
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mem_idx: %u \n",mem_idx);
            alloc_flag2 = 1;
        }
        else
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "opf error,ret: %u \n",ret);
        }
    }

    if( (pkt_size > SYS_DMA_TX_PKT_MEM_SIZE) || ret < 0)
    {
        p_addr = g_dal_op.dma_alloc(lchip, pkt_size + SYS_USW_PKT_HEADER_LEN , 0);
        if(NULL == p_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No dma memory for pkt\n");
            goto END;
        }
        alloc_flag2 = 2;
        p_tx_mem_info->mem_type = 1;
    }

    p_tx_mem_info->addr =(uintptr)p_addr;
    if(NULL == ctc_hash_insert(p_usw_dma_master[lchip]->dma_tx_mem_entry, p_tx_mem_info))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ctc_hash_insert error \n");
        goto END;
    }

    data_addr = (uint8*)p_addr + SYS_USW_PKT_HEADER_LEN;
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_tx_mem_info: %p, addr: %p, mem-type: %d\n",
    p_tx_mem_info,data_addr,p_tx_mem_info->mem_type);
    return data_addr;

END:
    if(alloc_flag2 == 1)
    {
        sys_usw_opf_free_offset(lchip, &opf, 1, mem_idx);
    }
    else if(alloc_flag2 == 2)
    {
        if(p_addr)
        {
            g_dal_op.dma_free(lchip, p_addr);
        }
    }
    if(alloc_flag1)
    {
        mem_free(p_tx_mem_info);
    }
    return NULL;
}

/*!!!!WARNING!!!!! Do not modify the function*/
int32
_sys_usw_dma_tx_get_tx_mem(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx, sys_dma_tx_mem_t** pp_tx_mem_info)
{
    void* new_addr = 0;
    sys_dma_tx_mem_t tx_mem_info;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;

    sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
    sal_memset(&tx_mem_info, 0, sizeof(sys_dma_tx_mem_t));
    tx_mem_info.addr = (uintptr)(p_pkt_tx->skb.data - SYS_USW_PKT_HEADER_LEN);
    p_tx_mem_info = ctc_hash_lookup(p_usw_dma_master[lchip]->dma_tx_mem_entry, &tx_mem_info);
    if(p_tx_mem_info)
    {/*zero copy*/
        /*copy bridge header*/
        sal_memcpy((uint8*)p_tx_mem_info->addr,(p_pkt_tx->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN), SYS_USW_PKT_HEADER_LEN);
        if (p_pkt_tx->callback)
        {
            p_pkt_tx->skb.len +=  SYS_USW_PKT_HEADER_LEN;
        }
    }
    else
    {
        new_addr = _sys_usw_dma_tx_alloc(p_pkt_tx->lchip, p_pkt_tx->skb.len, 1);
        if(NULL == new_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_dma_tx_alloc error\n");
            sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
            return CTC_E_NO_MEMORY;
        }

        if(p_pkt_tx->skb.tail == p_pkt_tx->skb.data)
        {/*only bridge header is in skb.buf*/
            sal_memcpy((uint8*)new_addr - SYS_USW_PKT_HEADER_LEN, p_pkt_tx->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN, SYS_USW_PKT_HEADER_LEN);
            sal_memcpy((uint8*)new_addr, p_pkt_tx->skb.data, p_pkt_tx->skb.len);
            if (p_pkt_tx->callback)
            {
                p_pkt_tx->skb.len +=  SYS_USW_PKT_HEADER_LEN;
            }
        }
        else
        {/*bridge header and pkt data are in skb.buf*/
            sal_memcpy((uint8*)new_addr - SYS_USW_PKT_HEADER_LEN, p_pkt_tx->skb.data, p_pkt_tx->skb.len + SYS_USW_PKT_HEADER_LEN);
            if (p_pkt_tx->callback)
            {
                p_pkt_tx->skb.len +=  SYS_USW_PKT_HEADER_LEN;
            }
        }

        sal_memset(&tx_mem_info, 0, sizeof(sys_dma_tx_mem_t));
        tx_mem_info.addr = (uintptr)((uint8*)new_addr - SYS_USW_PKT_HEADER_LEN);
        p_tx_mem_info = ctc_hash_lookup(p_usw_dma_master[lchip]->dma_tx_mem_entry, &tx_mem_info);
        if(NULL == p_tx_mem_info)
        {
            sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
            return CTC_E_NOT_EXIST;
        }
    }
    sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);

    *pp_tx_mem_info = p_tx_mem_info;

    return CTC_E_NONE;
}
#define _DMA_FUNCTION_INTERFACE

void*
sys_usw_dma_tx_alloc(uint8 lchip, uint32 pkt_size)
{
    void* p_addr = NULL;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%d:%s() \n",__LINE__,__FUNCTION__);
    if((lchip >= g_lchip_num) || NULL == p_usw_dma_master[lchip])
    {
        return NULL;
    }

    sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
    p_addr = _sys_usw_dma_tx_alloc(lchip, pkt_size, 0);
    sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);

    return p_addr;
}

int32
sys_usw_dma_tx_free(uint8 lchip, void* addr)
{
    sys_dma_tx_mem_t tx_mem_info;
    sys_dma_tx_mem_t* p_tx_mem_info;
    sys_usw_opf_t opf;
    int32 ret = CTC_E_NONE;


    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "free address[%p]\n", addr);
    SYS_DMA_INIT_CHECK(lchip);

    sal_memset(&tx_mem_info, 0, sizeof(sys_dma_tx_mem_t));
    tx_mem_info.addr = (uintptr)((uint8*)addr - SYS_USW_PKT_HEADER_LEN);
    sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
    p_tx_mem_info = ctc_hash_lookup(p_usw_dma_master[lchip]->dma_tx_mem_entry, &tx_mem_info);
    if(!p_tx_mem_info)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " can't find the address [%p]\n", addr);
        ret = CTC_E_NOT_EXIST;
        goto END;
    }

    if(p_tx_mem_info->callback == NULL)
    {/*sync mode, process none*/
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " sync mode, process none\n");
        goto END;
    }

    if(0 == p_tx_mem_info->mem_type)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = 0;
        opf.pool_type  = p_usw_dma_master[lchip]->opf_type_dma_tx_mem;
        sys_usw_opf_free_offset(lchip, &opf, 1, p_tx_mem_info->opf_idx);
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " 0 == p_tx_mem_info->mem_type\n");
    }
    else
    {
        g_dal_op.dma_free(lchip, (void*)tx_mem_info.addr);
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " 1 == p_tx_mem_info->mem_type, [%p]\n", (void*)tx_mem_info.addr);

    }
    if(NULL == ctc_hash_remove(p_usw_dma_master[lchip]->dma_tx_mem_entry, p_tx_mem_info))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ctc_hash_remove error \n");
        sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
        return CTC_E_NOT_EXIST;
    }
    mem_free(p_tx_mem_info);
    sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
    return CTC_E_NONE;

END:
    sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
    return ret;
}
/**
 @brief table DMA TX
*/
int32
_sys_usw_dma_read_table(uint8 lchip, DsDesc_m* p_tbl_desc)
{
    sys_dma_chan_t* p_dma_chan = &(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_RD_CHAN_ID]);
    uint32 cmd = 0;
    uint32 rd_cnt = 0;
    bool  done = FALSE;
    uint32 i  = 0;
    int32 ret = 0;
    drv_work_platform_type_t  platform_type;
    uint32 tbl_id = 0;
    DmaCtlTab_m ctl_tab;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Current descriptor, desc_index=%d,done:%d error:%d sop:%d eop:%d desc_len:%d !\n",
                    p_dma_chan->current_index,
                    GetDsDescEncap(V, done_f, p_tbl_desc),
                    GetDsDescEncap(V, error_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_sop_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_eop_f, p_tbl_desc),
                    GetDsDescEncap(V, cfgSize_f, p_tbl_desc));

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "desc_mem_addr_low =0x%x,desc_cfg_addr:0x%x  desc_words:%d !\n",
                    GetDsDescEncap(V, memAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, chipAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, dataStruct_f , p_tbl_desc));

    /* table DMA  valid num */
    tbl_id = DmaCtlTab_t + p_dma_chan->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_RD_CHAN_ID, cmd, &ctl_tab));
    SetDmaCtlTab(V, vldNum_f, &ctl_tab, 1);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_RD_CHAN_ID, cmd, &ctl_tab));

    /* wait for desc done */
    while (rd_cnt < SYS_DMA_TBL_COUNT)
    {
        rd_cnt++;

        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tbl_desc), sizeof(DsDesc_m));
        }
        /* transmit failed */
        if (1 == GetDsDescEncap(V, done_f, p_tbl_desc))
        {
            done  = TRUE;
            break;
        }

        if ((i++) >= 1000)
        {
#ifndef PACKET_TX_USE_SPINLOCK
            sal_task_sleep(1);
#else
/*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*//*SYSTEM MODIFY, Added by yangbs for ipfix, 2017-01-09*/
            sal_task_sleep(10);
#endif
             i = 0;
        }

        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DMA TX wait i = %d!!!!!!!!!!\n", i);
    }

    ret = drv_get_platform_type(lchip, &platform_type);
    if (1)
    {
        if (done == FALSE)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " desc not done!!!\n");
            ret  = CTC_E_DMA;
        }
    }

    /* next descriptor, tbl_desc_index: 0~table_desc_num-1*/
    p_dma_chan->current_index =
        ((p_dma_chan->current_index + 1) == p_dma_chan->desc_depth) ? 0 : (p_dma_chan->current_index + 1);

    return ret;
}

/**
 @brief table DMA RX
*/
int32
_sys_usw_dma_write_table(uint8 lchip, DsDesc_m* p_tbl_desc)
{
    sys_dma_chan_t* p_dma_chan = &(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID]);
    uint32 cmd = 0;
    uint32 wr_cnt = 0;
    bool  done = FALSE;
    int32 ret = 0;
    drv_work_platform_type_t  platform_type = MAX_WORK_PLATFORM;
    uint32 i  = 0;
    uint32 tbl_id = 0;
    DmaCtlTab_m ctl_tab;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "Current descriptor, desc_index=%d,done:%d error:%d sop:%d eop:%d desc_len:%d !\n",
                    p_dma_chan->current_index,
                    GetDsDescEncap(V, done_f, p_tbl_desc),
                    GetDsDescEncap(V, error_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_sop_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_eop_f, p_tbl_desc),
                    GetDsDescEncap(V, cfgSize_f, p_tbl_desc));

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "desc_mem_addr_low =0x%x,desc_cfg_addr:0x%x  desc_words:%d !\n",
                    GetDsDescEncap(V, memAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, chipAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, dataStruct_f , p_tbl_desc));

    /* table DMA  valid num */
    tbl_id = DmaCtlTab_t + p_dma_chan->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &ctl_tab));
    SetDmaCtlTab(V, vldNum_f, &ctl_tab, 1);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &ctl_tab));

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Current descriptor, desc_index=%d,done:%d error:%d sop:%d eop:%d cfg_size:%d !\n",
                    p_dma_chan->current_index,
                    GetDsDescEncap(V, done_f, p_tbl_desc),
                    GetDsDescEncap(V, error_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_sop_f, p_tbl_desc),
                    GetDsDescEncap(V, u1_pkt_eop_f, p_tbl_desc),
                    GetDsDescEncap(V, cfgSize_f, p_tbl_desc));

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "desc_mem_addr_low =0x%x,desc_cfg_addr:0x%x  desc_words:%d !\n",
                    GetDsDescEncap(V, memAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, chipAddr_f, p_tbl_desc),
                    GetDsDescEncap(V, dataStruct_f , p_tbl_desc));

    /* wait table write done */
    while (wr_cnt < SYS_DMA_TBL_COUNT)
    {
        wr_cnt++;
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tbl_desc), sizeof(DsDesc_m));
        }
        /* transmit failed */
        if (1 == GetDsDescEncap(V, done_f, p_tbl_desc))
        {
            done  = TRUE;
            break;
        }

        if ((i++) >= 1000)
        {
#ifndef PACKET_TX_USE_SPINLOCK
            sal_task_sleep(1);
#else
            sal_udelay(1000);
#endif
            i = 0;
        }

        /* SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DMA TX wait i = %d!!!!!!!!!!\n", i);*/
    }

    ret = drv_get_platform_type(lchip, &platform_type);
    if (platform_type == HW_PLATFORM)
    {
        if (done == FALSE)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " desc not done!!!\n");
            ret  = CTC_E_DMA;
        }
    }

    /* next descriptor, tbl_desc_index: 0~table_desc_num-1*/
    p_dma_chan->current_index =
        ((p_dma_chan->current_index + 1) == p_dma_chan->desc_depth) ? 0 : (p_dma_chan->current_index + 1);

    return ret;
}

int32
sys_usw_dma_function_pause(uint8 lchip, uint8 chan_id, uint8 en)
{
    DmaCtlDrainEnable_m dma_drain;
    uint32 cmd = 0;
    uint32 value = (en)?0:1;

    SYS_DMA_INIT_CHECK(lchip);
    if (chan_id > SYS_DMA_MONITOR_CHAN_ID)
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_drain));
    switch(GET_CHAN_TYPE(chan_id))
    {
        case DRV_DMA_LEARNING_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaInfo0DrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_HASHKEY_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaInfo1DrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_IPFIX_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaInfo2DrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_SDC_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaInfo3DrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_MONITOR_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaInfo4DrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_PACKET_RX0_CHAN_ID:
        case DRV_DMA_PACKET_RX1_CHAN_ID:
        case DRV_DMA_PACKET_RX2_CHAN_ID:
        case DRV_DMA_PACKET_RX3_CHAN_ID:
            value =  GetDmaCtlDrainEnable(V,dmaPktRxDrainEn_f, &dma_drain);
            if (en)
            {
                value &= ~(1 << (chan_id - SYS_DMA_PACKET_RX0_CHAN_ID));
            }
            else
            {
                value |= (1 << (chan_id - SYS_DMA_PACKET_RX0_CHAN_ID));
            }
            SetDmaCtlDrainEnable(V,dmaPktRxDrainEn_f, &dma_drain, value);
            break;

       case DRV_DMA_PACKET_TX0_CHAN_ID:
       case DRV_DMA_PACKET_TX1_CHAN_ID:
       case DRV_DMA_PACKET_TX2_CHAN_ID:
       case DRV_DMA_PACKET_TX3_CHAN_ID:
            value =  GetDmaCtlDrainEnable(V,dmaPktTxDrainEn_f, &dma_drain);
            if (en)
            {
                value &= ~(1 << (chan_id - SYS_DMA_PACKET_TX0_CHAN_ID));
            }
            else
            {
                value |= (1 << (chan_id - SYS_DMA_PACKET_TX0_CHAN_ID));
            }
            SetDmaCtlDrainEnable(V,dmaPktTxDrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_TBL_RD_CHAN_ID:
        case DRV_DMA_PORT_STATS_CHAN_ID:
        case DRV_DMA_FLOW_STATS_CHAN_ID:
        case DRV_DMA_REG_MAX_CHAN_ID:
        case DRV_DMA_TBL_RD1_CHAN_ID:
            value =  GetDmaCtlDrainEnable(V,dmaRegRdDrainEn_f, &dma_drain);
            if (en)
            {
                value &= ~(1 << (chan_id - SYS_DMA_TBL_RD_CHAN_ID));
            }
            else
            {
                value |= (1 << (chan_id - SYS_DMA_TBL_RD_CHAN_ID));
            }
            SetDmaCtlDrainEnable(V,dmaRegRdDrainEn_f, &dma_drain, value);
            break;

        case DRV_DMA_TBL_WR_CHAN_ID:
            SetDmaCtlDrainEnable(V,dmaRegWrDrainEn_f, &dma_drain, value);
            break;

        default:
            break;
    }

    cmd = DRV_IOW(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_drain));

    return CTC_E_NONE;
}

int32
sys_usw_dma_write_dynamic_table(uint8 lchip, uint32 tbl_addr, uint32* buffer, uint16 entry_num, uint16 entry_len, uint8 user_dma_mode)
{
    sys_dma_tbl_rw_t dma_cfg;
    sal_memset(&dma_cfg, 0 , sizeof(sys_dma_tbl_rw_t));
    dma_cfg.tbl_addr = tbl_addr;
    dma_cfg.buffer = buffer;
    dma_cfg.entry_num = entry_num;
    dma_cfg.entry_len = entry_len;
    dma_cfg.rflag = 0;
    dma_cfg.user_dma_mode = user_dma_mode;
    return sys_usw_dma_rw_table(lchip, &dma_cfg);
}
/**
 @brief table DMA TX & RX
*/
int32
sys_usw_dma_rw_table(uint8 lchip, sys_dma_tbl_rw_t* tbl_cfg)
{

    int ret = CTC_E_NONE;
    DsDesc_m* p_tbl_desc = NULL;
    uint32* p_tbl_buff = NULL;
    uint32 words_in_chip = 0;
    uint32 words_num = 0;
    uint16 entry_num = 0;
    uint32 tbl_buffer_len = 0;
    uint32 tbl_addr = 0;
    uint16 idx = 0;
    uint32* p_src = NULL;
    uint32* p_dst = NULL;
    uint32* p = NULL;
    sys_dma_chan_t* p_dma_chan = NULL;
    uint32 phy_addr = 0;
    uint8 dma_en = 0;
    sal_mutex_t* p_mutex = NULL;

    SYS_DMA_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(tbl_cfg);

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "operate(1:read, 0:write): %d, table_addr: 0x%x, entry_num: %d, entry_len: %d lchip:%d\n",
                    tbl_cfg->rflag, tbl_cfg->tbl_addr, tbl_cfg->entry_num, tbl_cfg->entry_len, lchip);

    /* mask bit(1:0) */
    tbl_addr = tbl_cfg->tbl_addr;
    /*tbl_addr &= 0xfffffffc;*/

    words_num = (tbl_cfg->entry_len / SYS_DMA_WORD_LEN);
    entry_num = tbl_cfg->entry_num;

    ret = _sys_usw_dma_rw_get_words_in_chip(lchip, words_num, &words_in_chip);
    if (ret < 0)
    {
        return ret;
    }

    tbl_buffer_len = entry_num * words_in_chip * SYS_DMA_WORD_LEN;

    /* check data size should smaller than desc's cfg MAX size */
    if (tbl_buffer_len > 0xffff)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(!tbl_cfg->user_dma_mode || tbl_cfg->rflag)
    {
        if (NULL == g_dal_op.dma_alloc)
        {
            return CTC_E_DRV_FAIL;
        }
        p_tbl_buff = g_dal_op.dma_alloc(lchip, tbl_buffer_len, 0);
        if (NULL == p_tbl_buff)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;

        }
        sal_memset(p_tbl_buff, 0, tbl_buffer_len);
    }
    else
    {
        p_tbl_buff = tbl_cfg->buffer;
    }

    if (tbl_cfg->rflag)
    {
        CTC_ERROR_GOTO(sys_usw_dma_get_chan_en(lchip, SYS_DMA_TBL_RD_CHAN_ID, &dma_en), ret, error);
        if (dma_en == 0)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma Read Function is not enabled!!!\n");
            ret = CTC_E_NOT_SUPPORT;
            goto error;
        }

        p_dma_chan = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_RD_CHAN_ID]);
        p_mutex = p_dma_chan->p_mutex;
        DMA_LOCK(p_mutex);

        p_tbl_desc = &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info);
        if (p_tbl_desc == NULL)
        {
            ret = CTC_E_INVALID_CONFIG;
            DMA_UNLOCK(p_mutex);
            goto error;
        }

        if (g_dal_op.logic_to_phy)
        {
           phy_addr = ((uint32)(g_dal_op.logic_to_phy(lchip, (void*)p_tbl_buff))) >> 4;
        }

        sal_memset(p_tbl_desc, 0, sizeof(DsDesc_m));

        SetDsDescEncap(V, memAddr_f, p_tbl_desc, (phy_addr));
        SetDsDescEncap(V, cfgSize_f, p_tbl_desc, tbl_buffer_len);
        SetDsDescEncap(V, chipAddr_f, p_tbl_desc, (tbl_addr));
        SetDsDescEncap(V, dataStruct_f, p_tbl_desc, ((words_num == 64)?0:words_num));
        /*flush dma after write*/
        if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tbl_desc), sizeof(DsDesc_m));
        }

        /* read */
        ret = _sys_usw_dma_read_table(lchip, p_tbl_desc);
        if (ret < 0)
        {
            DMA_UNLOCK(p_mutex);
            goto error;
        }

        /* get read result */
        for (idx = 0; idx < entry_num; idx++)
        {
            p_src = p_tbl_buff + idx * words_in_chip;
            p_dst = tbl_cfg->buffer + idx * words_num;
            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_src), words_num*SYS_DMA_WORD_LEN);
            }
            sal_memcpy(p_dst, p_src, words_num*SYS_DMA_WORD_LEN);
        }

        /*for debug show result */
        for (idx = 0; idx < entry_num * words_num; idx++)
        {
            p = (uint32*)(tbl_cfg->buffer) + idx;
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Read table(word %d) dma value: 0x%x \n", idx, *p);
        }
        DMA_UNLOCK(p_mutex);
    }
    else
    {
        if (DRV_IS_DUET2(lchip) && p_usw_dma_master[lchip]->pkt_tx_timer_en)
        {
            return CTC_E_INVALID_CONFIG;
        }

        CTC_ERROR_GOTO(sys_usw_dma_get_chan_en(lchip, SYS_DMA_TBL_WR_CHAN_ID, &dma_en), ret, error);
        if (dma_en == 0)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma Write Function is not enabled!!!\n");
            ret = CTC_E_NOT_SUPPORT;
            goto error;
        }

        p_dma_chan = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID]);
        p_mutex = p_dma_chan->p_mutex;
        DMA_LOCK(p_mutex);

        p_tbl_desc = &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info);
        if (p_tbl_desc == NULL)
        {
            ret =  CTC_E_INVALID_CONFIG;
            DMA_UNLOCK(p_mutex);
            goto error;
        }

        if(!tbl_cfg->user_dma_mode)
        {
            /* for debug show orginal data */
            for (idx = 0; idx < entry_num * words_num; idx++)
            {
                p = (uint32*)(tbl_cfg->buffer) + idx;
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Write table(word %d) ogrinal value: 0x%x \n", idx, *p);
            }

            for (idx = 0; idx < entry_num; idx++)
            {
                p_dst = p_tbl_buff + idx * words_in_chip;
                p_src = tbl_cfg->buffer + idx * words_num;

                sal_memcpy(p_dst, p_src, words_num*SYS_DMA_WORD_LEN);
            }

            for (idx = 0; idx < entry_num * words_in_chip; idx++)
            {
                p = (uint32*)(p_tbl_buff) + idx;
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Write table(word %d) dma value: 0x%x \n", idx, *p);
            }
        }

        if (g_dal_op.logic_to_phy)
        {
            phy_addr = ((uint32)(g_dal_op.logic_to_phy(lchip, p_tbl_buff))) >> 4;
        }
        /*flush dma after write*/
        if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, p_tbl_buff), tbl_buffer_len);
        }

        sal_memset(p_tbl_desc, 0, sizeof(DsDesc_m));
        SetDsDescEncap(V, memAddr_f, p_tbl_desc, (phy_addr));
        SetDsDescEncap(V, cfgSize_f, p_tbl_desc, (tbl_buffer_len));
        SetDsDescEncap(V, chipAddr_f, p_tbl_desc, (tbl_addr));
        SetDsDescEncap(V, dataStruct_f, p_tbl_desc, ((words_num == 64)?0:words_num));
        /*flush dma after write*/
        if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_tbl_desc), sizeof(DsDesc_m));
        }

        /* write */
        ret = _sys_usw_dma_write_table(lchip, p_tbl_desc);
        if (ret < 0)
        {
            DMA_UNLOCK(p_mutex);
            goto error;
        }

        DMA_UNLOCK(p_mutex);
    }

error:

    if ((!tbl_cfg->user_dma_mode || tbl_cfg->rflag) && g_dal_op.dma_free)
    {
        g_dal_op.dma_free(lchip, p_tbl_buff);
    }

    return ret;

}

/**
 @brief packet DMA TX
*/

int32
sys_usw_dma_pkt_tx(ctc_pkt_tx_t* p_pkt_tx)
{
    int32 ret = CTC_E_NONE;
    void* tx_pkt_addr = NULL;
    DsDesc_m* p_tx_desc_mem = NULL;
    sys_dma_chan_t* p_dma_chan = NULL;
    uint64 phy_addr = 0;
    uint8 dma_en_tx0 = 0;
    uint8 dma_en_tx1 = 0;
    uint8 chan_idx = 0;
    sal_mutex_t* p_mutex = NULL;
    uint8 lchip = 0;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;

    lchip = p_pkt_tx->lchip;

    SYS_DMA_INIT_CHECK(lchip);

    /* packet length check */
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "data len: %d\n", p_pkt_tx->skb.len);
    SYS_DMA_PKT_LEN_CHECK(p_pkt_tx->skb.len);

    if (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID].pkt_knet_en &&
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].pkt_knet_en)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if(p_pkt_tx->tx_info.flags == 0xFFFFFFFF)
    {
        return sys_usw_dma_performance_test(lchip, p_pkt_tx);
    }

    if (CTC_FLAG_ISSET(p_pkt_tx->tx_info.flags, CTC_PKT_FLAG_SESSION_ID_EN))
    {
        CTC_ERROR_RETURN(sys_usw_dma_set_session_pkt(lchip, p_pkt_tx->tx_info.session_id, p_pkt_tx));
        return CTC_E_NONE;
    }

    dma_en_tx0 = CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX0_CHAN_ID);
    dma_en_tx1 = CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX1_CHAN_ID);
    if (!dma_en_tx0 || (DRV_IS_DUET2(lchip) && !dma_en_tx1 && !p_usw_dma_master[lchip]->pkt_tx_timer_en)
        || (!DRV_IS_DUET2(lchip) && !dma_en_tx1))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma Packet Tx  Function is not enabled!!!\n");
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    if(p_pkt_tx->callback)
    {   /*async*/
        ctc_slist_t* tx_pending_list = NULL;
        sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
        if (!p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID].pkt_knet_en &&
            !p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].pkt_knet_en)
        {
            chan_idx = (p_pkt_tx->tx_info.priority <= 10 || (DRV_IS_DUET2(lchip) && p_usw_dma_master[lchip]->pkt_tx_timer_en)) ? 0 : 1;
        }
        else if (!p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID].pkt_knet_en)
        {
            chan_idx = 0;
        }
        else
        {
            chan_idx = 1;
        }
        if(chan_idx == 1)
        {
            tx_pending_list = p_usw_dma_master[lchip]->tx_pending_list_H;
        }
        else
        {
            tx_pending_list = p_usw_dma_master[lchip]->tx_pending_list_L;
        }
        /*async pkt list requires a lot of memory, so cannot exceed 2k*/
        if (tx_pending_list->count > 2048)
        {
            sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
            return CTC_E_NO_MEMORY;
        }
        sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
        if (p_usw_dma_master[lchip]->tx_pending_list_H)
        CTC_ERROR_RETURN(_sys_usw_dma_tx_get_tx_mem(lchip, p_pkt_tx, &p_tx_mem_info));
        p_tx_mem_info->tx_pkt = p_pkt_tx;
        p_tx_mem_info->pkt_len= p_pkt_tx->skb.len;
        p_tx_mem_info->callback =  p_pkt_tx->callback;
        p_tx_mem_info->user_data = p_pkt_tx->user_data;
        sal_memcpy(&p_tx_mem_info->tx_info, &p_pkt_tx->tx_info, sizeof(ctc_pkt_info_t));

        sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);

        if(chan_idx == 1)
        {
            ctc_slist_add_tail(p_usw_dma_master[lchip]->tx_pending_list_H, &p_tx_mem_info->node_head);
        }
        else
        {
            ctc_slist_add_tail(p_usw_dma_master[lchip]->tx_pending_list_L, &p_tx_mem_info->node_head);
        }

        sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
        CTC_ERROR_RETURN(sal_sem_give(p_usw_dma_master[lchip]->tx_sem));
    }
    else
    {
        /* use which channel should by the packet's priority */
        chan_idx = (p_pkt_tx->tx_info.priority <= 10 || (DRV_IS_DUET2(lchip) && p_usw_dma_master[lchip]->pkt_tx_timer_en))?0:1;
        p_dma_chan = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID + chan_idx]);

        CTC_PTR_VALID_CHECK(p_dma_chan->p_desc_used);

        p_mutex = p_dma_chan->p_mutex;
        DMA_LOCK(p_mutex);

        p_tx_desc_mem = &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info);
        if (p_tx_desc_mem == NULL)
        {
            DMA_UNLOCK(p_mutex);
            return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_GOTO(_sys_usw_dma_wait_desc_finish_tx(lchip, p_tx_desc_mem,p_dma_chan),ret, error0);

        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_tx_desc_mem)<<4), phy_addr);

        if (g_dal_op.phy_to_logic)
        {
            tx_pkt_addr =  g_dal_op.phy_to_logic(lchip, (phy_addr));
        }
        _sys_usw_dma_clear_desc(lchip, p_dma_chan, p_dma_chan->current_index, 1, tx_pkt_addr);

        CTC_ERROR_GOTO(_sys_usw_dma_tx_get_tx_mem(lchip, p_pkt_tx, &p_tx_mem_info), ret, error0);
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_tx_mem_info->addr: %"PRIUPTR",%s()\n",p_tx_mem_info->addr, __FUNCTION__);
        p_tx_mem_info->tx_pkt=p_pkt_tx;
        p_tx_mem_info->pkt_len=p_pkt_tx->skb.len+SYS_USW_PKT_HEADER_LEN;

        CTC_ERROR_GOTO(_sys_usw_dma_set_tx_chip(lchip, p_tx_desc_mem, p_tx_mem_info, p_dma_chan), ret, error0);
        error0:
        DMA_UNLOCK(p_mutex);
    }

    return ret;

}

/**
 @brief Dma register callback function
*/
int32
sys_usw_dma_register_cb(uint8 lchip, uint8 type, void* cb)
{
    if (NULL == p_usw_dma_master[lchip])
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    if (type >= SYS_DMA_CB_MAX_TYPE)
    {
        return CTC_E_INVALID_PARAM;
    }

     p_usw_dma_master[lchip]->dma_cb[type] = (DMA_CB_FUN_P)cb;

    return CTC_E_NONE;
}

/*
Notice:this interface is used to wait dmactl to finish dma op and write back desc to memory
Before using this interface must config dmactl already begin process data and not finish
Now only used for hashdump
*/
int32
sys_usw_dma_wait_desc_done(uint8 lchip, uint8 chan_id)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 desc_done = 0;
    uint32 wait_cnt = 0;

    SYS_DMA_INIT_CHECK(lchip);
    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan_id];
    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;

    p_desc = &p_base_desc[cur_index].desc_info;

    do
    {
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }
        desc_done = GetDsDescEncap(V, done_f, p_desc);
        if (desc_done)
        {
            break;
        }

#ifndef PACKET_TX_USE_SPINLOCK
        sal_task_sleep(1);
#else
        sal_udelay(1000);
#endif
        wait_cnt++;

    }while(wait_cnt < 0x1000);

    if (desc_done)
    {
        return CTC_E_NONE;
    }
    else
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [DMA] Desc not done \n");
			return CTC_E_DMA;

    }

}


int32
sys_usw_dma_clear_chan_data(uint8 lchip, uint8 chan_id)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    int32 ret = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_DMA_INIT_CHECK(lchip);

    if (SYS_DMA_MAX_CHAN_NUM <= chan_id)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_DMA_INIT_CHECK(lchip);
    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan_id];
    DMA_LOCK(p_dma_chan->p_mutex);

    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;

    ret = sys_usw_dma_wait_desc_done(lchip, chan_id);
    if (ret == CTC_E_NONE)
    {
        for(;; cur_index++)
        {
            if (cur_index >= p_dma_chan->desc_depth)
            {
                cur_index = 0;
            }

            p_desc = &p_base_desc[cur_index].desc_info;
            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
            }
            if (GetDsDescEncap(V, done_f, p_desc))
            {
                /* clear Desc and return Desc to DmaCtl*/
               _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
            }
            else
            {
                break;
            }
        }

        p_dma_chan->current_index = cur_index;
    }

    DMA_UNLOCK(p_dma_chan->p_mutex);
    return ret;

}

int32
sys_usw_dma_set_chan_en(uint8 lchip, uint8 chan_id, uint8 chan_en)
{
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    sys_dma_chan_t* p_chan_info = NULL;
    DmaStaticInfo_m static_info;

    SYS_DMA_INIT_CHECK(lchip);
    if(chan_id > MCHIP_CAP(SYS_CAP_DMA_MAX_CHAN_ID))
    {
        return CTC_E_INVALID_PARAM;
    }
    p_chan_info = (sys_dma_chan_t*)&p_usw_dma_master[lchip]->dma_chan_info[chan_id];
    sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));

    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &static_info));
    SetDmaStaticInfo(V, chanEn_f, &static_info, chan_en);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &static_info));

    p_chan_info->chan_en = chan_en;

    return CTC_E_NONE;

}

int32
sys_usw_dma_get_chan_en(uint8 lchip, uint8 chan_id, uint8* chan_en)
{
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    sys_dma_chan_t* p_chan_info = NULL;
    DmaStaticInfo_m static_info;

    SYS_DMA_INIT_CHECK(lchip);

    p_chan_info = (sys_dma_chan_t*)&p_usw_dma_master[lchip]->dma_chan_info[chan_id];
    sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));

    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &static_info));

    *chan_en = GetDmaStaticInfo(V, chanEn_f, &static_info);

    return CTC_E_NONE;

}






int32
sys_usw_dma_sync_hash_dump(uint8 lchip, dma_dump_cb_parameter_t* p_pa, uint16* p_entry_num, void* p_data )
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    DmaFibDumpFifo_m* p_dump = NULL;
    DmaFibDumpFifo_m* p_addr_start = NULL;
    uint32 cur_index = 0;
    uint8 process_cnt = 0;
    uint32 desc_done = 0;
    uint16 wait_cnt = 0;
    uint8 dma_done = 0;
    uint32 real_size = 0;
    /*uint32 cfg_size = 0;*/
    uint64 phy_addr = 0;
    uint32 entry_num = 0;
    int32 ret = 0;
    uint8 end_flag = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_entry_num);
    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_pa);
    SYS_DMA_INIT_CHECK(lchip);

    p_addr_start = (DmaFibDumpFifo_m*)p_data;

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_HASHKEY_CHAN_ID];
    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "    current_index:%d, threshold:%d, acc entry count:%d, is fifo full:%d\n",\
                                            cur_index, p_pa->threshold, p_pa->entry_count, p_pa->fifo_full);
    for(;; cur_index++)
    {
        dma_done = 0;
        wait_cnt = 0;
        if (cur_index >= p_dma_chan->desc_depth)
        {
            cur_index = 0;
        }

        p_desc = &p_base_desc[cur_index].desc_info;

        do
        {
            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
            }
            desc_done = GetDsDescEncap(V, done_f, p_desc);
            if (desc_done)
            {
                dma_done = 1;
                break;
            }

#ifndef PACKET_TX_USE_SPINLOCK
            sal_task_sleep(1);
#else
            sal_udelay(1000);
#endif
            wait_cnt++;

        }while(wait_cnt < 0xffff);

        if (dma_done == 0)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Desc is not done!! \n");
            ret = CTC_E_DMA;
            /* dma not done,  need end current dma operate */
            goto end;
        }

        process_cnt++;

        /* get current desc real size */
        real_size = GetDsDescEncap(V, realSize_f, p_desc);

        /*cfg_size = GetDsDescEncap(V, cfgSize_f, p_desc);*/
        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                                (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);

        p_dump = (DmaFibDumpFifo_m*)g_dal_op.phy_to_logic(lchip, phy_addr);

        entry_num += real_size;

        if (DRV_IS_DUET2(lchip))
        {
            /* check the last one */
            if (GetDmaFibDumpFifo(V, isLastEntry_f, &p_dump[real_size-1]))
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is lastEntry! \n");

                if (GetDmaFibDumpFifo(V, isMac_f, &p_dump[real_size-1]))
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Last entry is Mac entry! \n");
                    /* query is end */
                    *p_entry_num = entry_num;
                    sal_memcpy((uint8*)p_addr_start, (uint8*)p_dump, sizeof(DmaFibDumpFifo_m)*real_size);
                }
                else
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Last entry isnot Mac entry! \n");

                    /* query is end, but last is invalid */
                    *p_entry_num = entry_num-1;
                    sal_memcpy((uint8*)p_addr_start, (uint8*)p_dump, sizeof(DmaFibDumpFifo_m)*(real_size-1));
                }

                end_flag = 1;
            }
            else
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Not Last entry but is Mac entry! \n");

                if (entry_num >= p_pa->threshold)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Exceed, entry_num:%d, threshold:%d real_size:%d!! \n", entry_num, p_pa->threshold, real_size);
                    ret = CTC_E_INVALID_PARAM;
                   _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
                    goto end;
                }

                /* process next desc */
                sal_memcpy((uint8*)p_addr_start, (uint8*)p_dump, sizeof(DmaFibDumpFifo_m)*real_size);
                p_addr_start += real_size;

                if(p_pa->fifo_full && entry_num == p_pa->entry_count)
                {
                        *p_entry_num = entry_num;
                        end_flag = 1;
                }
            }
        }
        else /*TsingMa*/
        {
            /* process next desc */
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DMA p_pa->entry_count:%d, real_size:%d\n", p_pa->entry_count, real_size);
            sal_memcpy((uint8*)p_addr_start, (uint8*)p_dump, sizeof(DmaFibDumpFifo_m)*real_size);
            p_addr_start += real_size;

            if (entry_num == p_pa->entry_count)
            {
                *p_entry_num = entry_num;
                end_flag = 1;
            }
        }

        /* clear desc */
        _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);

        if (end_flag == 1)
        {
            break;
        }
    }

end:
    p_dma_chan->current_index = ((p_dma_chan->current_index + process_cnt) % (p_dma_chan->desc_depth));
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "current_index:%d, process_cnt:%d, ret = %d\n", p_dma_chan->current_index, process_cnt, ret);
    return ret;

}

int32
sys_usw_dma_set_dump_cb(uint8 lchip, void* cb)
{
    SYS_DMA_INIT_CHECK(lchip);
    if (NULL == p_usw_dma_master[lchip])
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

     p_usw_dma_master[lchip]->dma_dump_cb = (DMA_DUMP_FUN_P)cb;

    return CTC_E_NONE;
}

int32
sys_usw_dma_get_dump_cb(uint8 lchip, void**cb, void** user_data)
{
    SYS_DMA_INIT_CHECK(lchip);
    if (NULL == p_usw_dma_master[lchip])
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

     *cb = p_usw_dma_master[lchip]->dma_dump_cb;

    return CTC_E_NONE;
}


int32
sys_usw_dma_get_packet_rx_chan(uint8 lchip, uint16* p_num)
{
    SYS_DMA_INIT_CHECK(lchip);
    *p_num = p_usw_dma_master[lchip]->packet_rx_chan_num;
    return CTC_E_NONE;
}

int32
sys_usw_dma_get_hw_learning_sync(uint8 lchip, uint8* b_sync)
{
    SYS_DMA_INIT_CHECK(lchip);
    *b_sync = p_usw_dma_master[lchip]->hw_learning_sync;
    return CTC_E_NONE;
}

/*timer uinit is ms, 0 means disable*/
int32
sys_usw_dma_set_monitor_timer(uint8 lchip, uint32 timer)
{
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    DmaInfoTimerCfg_m global_timer_ctl;
    DmaInfo4TimerCfg_m info4_timer;
    uint64 time_ns = 0;
    uint64 time_s = 0;

    SYS_DMA_INIT_CHECK(lchip);

    tbl_id = DmaInfo4TimerCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info4_timer));


    tbl_id = DmaInfoTimerCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &global_timer_ctl));

    if (timer)
    {
        time_s = timer/1000;
        time_ns = time_s*1000000000 + (timer%1000)*1000000;

        SetDmaInfoTimerCfg(V, cfgInfo4DescTimerChk_f, &global_timer_ctl, 1);/*monitor*/

        SetDmaInfo4TimerCfg(A, cfgInfo4TimerNs_f, &info4_timer, (uint32*)&time_ns);

        tbl_id = DmaInfo4TimerCfg_t;
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info4_timer));
    }
    else
    {
        SetDmaInfoTimerCfg(V, cfgInfo4DescTimerChk_f, &global_timer_ctl, 0);/*monitor*/
    }

    tbl_id = DmaInfoTimerCfg_t;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &global_timer_ctl));

    return DRV_E_NONE;

}
/*mode: 0 scan once, 1 continues scan, 2 stop scan
uinit is minute*/
int32
sys_usw_dma_set_tcam_scan_mode(uint8 lchip, uint8 mode, uint32 timer)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint64 timer_ns = 0;
    uint32 timer_v[2] = {0};
    uint32 tmp_time = 0;
    DmaRegRd3TrigCfg_m trigger1_timer;
    DmaRegTrigEnCfg_m trigger_ctl;

    SYS_DMA_INIT_CHECK(lchip);
    if(mode > 2)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(mode == 0)
    {
        tmp_time = 1;
    }
    else if(mode == 1)
    {
        /*the max value is 60bits ns,so the max timer is (1<<62-1)/1000/1000/1000/60 minutes*/
        if(timer > 76861433)
        {
            return CTC_E_INVALID_PARAM;
        }
        tmp_time = timer*60;
    }

    if (tmp_time)
    {
        timer_ns = (uint64)tmp_time*1000000*1000/DOWN_FRE_RATE;
        timer_v[0] = timer_ns&0xFFFFFFFF;
        timer_v[1] = (timer_ns >> 32) & 0xFFFFFFFF;
        tbl_id = DmaRegRd3TrigCfg_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));
        SetDmaRegRd3TrigCfg(A, cfgRegRd3TrigNs_f, &trigger1_timer, timer_v);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));
    }

    tbl_id = DmaRegTrigEnCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));
    SetDmaRegTrigEnCfg(V, cfgRegRd3TrigEn_f, &trigger_ctl, (tmp_time?1:0));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));

    return DRV_E_NONE;
}
/*timer unit is ms, 0 means disable*/
int32
sys_usw_dma_set_mac_stats_timer(uint8 lchip, uint32 timer)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint64 timer_ns = 0;
    uint32 timer_v[2] = {0};
    DmaRegRd1TrigCfg_m trigger1_timer;
    DmaRegTrigEnCfg_m trigger_ctl;

    SYS_DMA_INIT_CHECK(lchip);

    if (timer)
    {
        timer_ns = (uint64)timer*1000000;
        timer_v[0] = timer_ns&0xFFFFFFFF;
        timer_v[1] = (timer_ns >> 32) & 0xFFFFFFFF;
        tbl_id = DmaRegRd1TrigCfg_t;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));
        SetDmaRegRd1TrigCfg(A, cfgRegRd1TrigNs_f, &trigger1_timer, timer_v);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));
    }

    tbl_id = DmaRegTrigEnCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));
    SetDmaRegTrigEnCfg(V, cfgRegRd1TrigEn_f, &trigger_ctl, (timer?1:0));    /*set by stats module*/
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));

    return DRV_E_NONE;
}

int32
sys_usw_dma_sync_pkt_rx_stats(uint8 lchip)
{
    uint8  index = 0;
    uint64 temp_frame = 0;
    uint64 temp_byte = 0;
    uint32 cmd = 0;
    DmaPktRxStats_m stats;
    SYS_DMA_INIT_CHECK(lchip);

    cmd = DRV_IOR(DmaPktRxStats_t, DRV_ENTRY_FLAG);
    if(DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats));
        p_usw_dma_master[lchip]->dma_stats[0].u1.total_pkt_cnt += GetDmaPktRxStats(V, dmaPktRx0FrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[0].u2.total_byte_cnt += GetDmaPktRxStats(V, dmaPktRx0ByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[0].u3.drop_cnt      += GetDmaPktRxStats(V, dmaPktRx0DropCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[0].u4.error_cnt    += GetDmaPktRxStats(V, dmaPktRx0ErrorCnt_f, &stats);

        p_usw_dma_master[lchip]->dma_stats[1].u1.total_pkt_cnt += GetDmaPktRxStats(V, dmaPktRx1FrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[1].u2.total_byte_cnt += GetDmaPktRxStats(V, dmaPktRx1ByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[1].u3.drop_cnt      += GetDmaPktRxStats(V, dmaPktRx1DropCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[1].u4.error_cnt    += GetDmaPktRxStats(V, dmaPktRx1ErrorCnt_f, &stats);

        p_usw_dma_master[lchip]->dma_stats[2].u1.total_pkt_cnt += GetDmaPktRxStats(V, dmaPktRx2FrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[2].u2.total_byte_cnt += GetDmaPktRxStats(V, dmaPktRx2ByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[2].u3.drop_cnt      += GetDmaPktRxStats(V, dmaPktRx2DropCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[2].u4.error_cnt    += GetDmaPktRxStats(V, dmaPktRx2ErrorCnt_f, &stats);

        p_usw_dma_master[lchip]->dma_stats[3].u1.total_pkt_cnt += GetDmaPktRxStats(V, dmaPktRx3FrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[3].u2.total_byte_cnt += GetDmaPktRxStats(V, dmaPktRx3ByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[3].u3.drop_cnt      += GetDmaPktRxStats(V, dmaPktRx3DropCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[3].u4.error_cnt    += GetDmaPktRxStats(V, dmaPktRx3ErrorCnt_f, &stats);

        return CTC_E_NONE;
    }

    for(index=0; index < 8; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &stats));
        temp_frame = GetDmaPktRxStats(V, frameCntHi_f, &stats);
        temp_byte = GetDmaPktRxStats(V, byteCntHi_f, &stats);
        switch(index)
        {
        case 0:
            p_usw_dma_master[lchip]->dma_stats[0].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[0].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 1:
            p_usw_dma_master[lchip]->dma_stats[0].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[0].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 2:
            p_usw_dma_master[lchip]->dma_stats[1].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[1].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 3:
            p_usw_dma_master[lchip]->dma_stats[1].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[1].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 4:
            p_usw_dma_master[lchip]->dma_stats[2].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[2].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 5:
            p_usw_dma_master[lchip]->dma_stats[2].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[2].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        case 6:
            p_usw_dma_master[lchip]->dma_stats[3].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[3].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        default:
            p_usw_dma_master[lchip]->dma_stats[3].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktRxStats(V, frameCntLo_f, &stats));
            p_usw_dma_master[lchip]->dma_stats[3].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktRxStats(V, byteCntLo_f, &stats));
            break;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_dma_sync_pkt_tx_stats(uint8 lchip)
{
    uint8 index = 0;
    uint64 temp_frame = 0;
    uint64 temp_byte = 0;
    uint32 cmd = 0;
    DmaPktTxStats_m stats;
    SYS_DMA_INIT_CHECK(lchip);

    if(DRV_IS_DUET2(lchip))
    {
        sal_memset(&stats, 0, sizeof(DmaPktTxStats_m));
        cmd = DRV_IOR(DmaPktTxStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats));
        p_usw_dma_master[lchip]->dma_stats[4].u4.bad_byte_cnt += GetDmaPktTxStats(V, dmaPktTx0BadByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[4].u3.bad_pkt_cnt    += GetDmaPktTxStats(V, dmaPktTx0BadFrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[4].u2.good_byte_cnt  += GetDmaPktTxStats(V, dmaPktTx0GoodByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[4].u1.good_pkt_cnt   += GetDmaPktTxStats(V, dmaPktTx0GoodFrameCnt_f, &stats);

        p_usw_dma_master[lchip]->dma_stats[5].u4.bad_byte_cnt += GetDmaPktTxStats(V, dmaPktTx1BadByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[5].u3.bad_pkt_cnt    += GetDmaPktTxStats(V, dmaPktTx1BadFrameCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[5].u2.good_byte_cnt  += GetDmaPktTxStats(V, dmaPktTx1GoodByteCnt_f, &stats);
        p_usw_dma_master[lchip]->dma_stats[5].u1.good_pkt_cnt   += GetDmaPktTxStats(V, dmaPktTx1GoodFrameCnt_f, &stats);

        return CTC_E_NONE;
    }
    for(index=0; index < 8; index++)
    {
        cmd = DRV_IOR(DmaPktTxStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &stats));
        temp_frame = GetDmaPktTxStats(V, frameCntHi_f, &stats);
        temp_byte = GetDmaPktTxStats(V, byteCntHi_f, &stats);
        switch(index)
        {
            case 0:
                p_usw_dma_master[lchip]->dma_stats[4].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[4].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 1:
                p_usw_dma_master[lchip]->dma_stats[4].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[4].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 2:
                p_usw_dma_master[lchip]->dma_stats[5].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[5].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 3:
                p_usw_dma_master[lchip]->dma_stats[5].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[5].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 4:
                p_usw_dma_master[lchip]->dma_stats[6].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[6].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 5:
                p_usw_dma_master[lchip]->dma_stats[6].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[6].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            case 6:
                p_usw_dma_master[lchip]->dma_stats[7].u1.good_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[7].u2.good_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
            default:
                p_usw_dma_master[lchip]->dma_stats[7].u3.bad_pkt_cnt += (temp_frame<<32 | GetDmaPktTxStats(V, frameCntLo_f, &stats));
                p_usw_dma_master[lchip]->dma_stats[7].u4.bad_byte_cnt += (temp_byte<<32 | GetDmaPktTxStats(V, byteCntLo_f, &stats));
                break;
        }
    }
    return CTC_E_NONE;
}
/*timer uinit is s, 0 means disable*/
int32
sys_usw_dma_set_pkt_timer(uint8 lchip, uint32 timer, uint8 enable)
{
    uint32 cmd = 0;
    DmaStaticInfo_m static_info;
    uint32 session_num = 0;
    uint32 field_val = 0;
    DmaCtlTab_m tab_ctl;
    uint32 timer_v[2] = {0};
    DmaRegWrTrigCfg_m wr_timer;
    uint32 value_en = 0;
    uint64 timer_l = 0;
    uint8 tx_session_chan = 0;

    SYS_DMA_INIT_CHECK(lchip);

    if (DRV_IS_DUET2(lchip))
    {
        tx_session_chan = SYS_DMA_PACKET_TX1_CHAN_ID;
    }
    else
    {
        tx_session_chan = SYS_DMA_PKT_TX_TIMER_CHAN_ID;
    }

    if (!p_usw_dma_master[lchip]->pkt_tx_timer_en || !p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan].p_desc)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (enable == TRUE)
    {
        uint32 valid_num = 0;

        if (!p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan].desc_num)
        {
            return CTC_E_NOT_READY;
        }

        if (DRV_IS_DUET2(lchip))
        {
            p_usw_dma_master[lchip]->tx_timer = timer;

            cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &tab_ctl));
            valid_num = GetDmaCtlTab(V, vldNum_f, &tab_ctl);
            if (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_num > valid_num)
            {
                SetDmaCtlTab(V, vldNum_f, &tab_ctl, (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_num-valid_num));
                cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &tab_ctl));
            }

            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 1);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));

            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 1);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));

            /*cfg real ring depth*/
            session_num = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth;
            cmd = DRV_IOW(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &session_num));

            /*cfg wr register trigger interrupt per 255 desc*/
            session_num = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_depth;
            value_en = (session_num<255)?session_num:255;
            cmd = DRV_IOW(DmaRegWrIntrCntCfg_t, DmaRegWrIntrCntCfg_cfgRegWrIntrCnt_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_en));

            session_num = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth;
            timer_l = (uint64)timer*1000000/session_num; /*uints is ms*/
            timer_v[0] = timer_l&0xFFFFFFFF;
            timer_v[1] = (timer_l >> 32) & 0xFFFFFFFF;
            cmd = DRV_IOR(DmaRegWrTrigCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wr_timer));
            SetDmaRegWrTrigCfg(A, cfgRegWrTrigNs_f, &wr_timer, timer_v);
            cmd = DRV_IOW(DmaRegWrTrigCfg_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wr_timer));
            value_en = 1;

        }
        else
        {
            if (p_usw_dma_master[lchip]->tx_timer && (p_usw_dma_master[lchip]->tx_timer != timer))
            {
                return CTC_E_INVALID_CONFIG;
            }


            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));
            if (GetDmaStaticInfo(V, chanEn_f, &static_info))
            {
                return CTC_E_HW_BUSY;
            }

            /*cfg real ring depth*/
            session_num = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].desc_depth;
            cmd = DRV_IOW(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &session_num));

            cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &tab_ctl));
            if (GetDmaCtlTab(V, vldNum_f, &tab_ctl) < session_num)
            {
                SetDmaCtlTab(V, vldNum_f, &tab_ctl, (session_num-GetDmaCtlTab(V, vldNum_f, &tab_ctl)));
                cmd = DRV_IOW(DmaCtlTab_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip,SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &tab_ctl));
            }

            cmd = DRV_IOR(DmaCtlAutoMode_t, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            CTC_BIT_SET(field_val, SYS_DMA_PKT_TX_TIMER_CHAN_ID);
            cmd = DRV_IOW(DmaCtlAutoMode_t, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

            cmd = DRV_IOR(DmaMiscCfg_t, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            CTC_BIT_SET(field_val, (SYS_DMA_PKT_TX_TIMER_CHAN_ID-SYS_DMA_PACKET_TX0_CHAN_ID));
            cmd = DRV_IOW(DmaMiscCfg_t, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 1);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));

        }
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 0);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));

            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 0);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));
            value_en = 0;
        }
        else
        {
            /*uint32 cnt = 0;
            uint32 clear_done = 0;*/

            cmd = DRV_IOR(DmaCtlAutoMode_t, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            CTC_BIT_UNSET(field_val, SYS_DMA_PKT_TX_TIMER_CHAN_ID);
            cmd = DRV_IOW(DmaCtlAutoMode_t, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

            cmd = DRV_IOR(DmaMiscCfg_t, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            CTC_BIT_UNSET(field_val, (SYS_DMA_PKT_TX_TIMER_CHAN_ID-SYS_DMA_PACKET_TX0_CHAN_ID));
            cmd = DRV_IOW(DmaMiscCfg_t, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, chanEn_f, &static_info, 0);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));

            /* clear dma channel dynmamic*/
            /*field_val = (1 << SYS_DMA_PKT_TX_TIMER_CHAN_ID);
            cmd = DRV_IOW(DmaClearCtl_t, DmaClearCtl_dmaClearEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

            cnt = 0;
            do
            {
                cmd = DRV_IOR(DmaClearPend_t, DmaClearPend_dmaClearPending_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                clear_done = !CTC_IS_BIT_SET(field_val, SYS_DMA_PKT_TX_TIMER_CHAN_ID);
                cnt++;
            } while(!clear_done&&(cnt<0xffff));

            cmd = DRV_IOR(DmaClearPend_t, DmaClearPend_dmaClearPending_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            clear_done = !CTC_IS_BIT_SET(field_val, SYS_DMA_PKT_TX_TIMER_CHAN_ID);
            if (!clear_done)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tx timer channel cannot clear cnt:%u\n", cnt);
                return CTC_E_DMA;
            }

            cmd = DRV_IOW(DmaCtlTab_t, DmaCtlTab_vldNum_f);
            field_val = 0;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &field_val));*/
        }
    }

    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(DmaRegTrigEnCfg_t, DmaRegTrigEnCfg_cfgRegWrTrigEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_en));

        cmd = DRV_IOW(DmaRegIntrEnCfg_t, DmaRegIntrEnCfg_cfgRegWrDmaIntrEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_en));
    }

    return CTC_E_NONE;

}

int32
sys_usw_dma_set_session_pkt(uint8 lchip, uint16 session_id, ctc_pkt_tx_t* p_pkt)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    DsDesc_m* p_sys_desc = NULL;
    uint64 phy_addr = 0;
    void*  p_mem_addr = NULL;
    uint16 desc_idx = 0;
    uint8 tx_enable = 0;
    uint32 max_session = 0;
    uint32 phy_addr_low = 0;
    uint8 is_update = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tx_start = 0;
    uint16 last_session_id = 0;
    uint8 is_remove = 0;
    uint8 tx_session_chan = 0;

    SYS_DMA_INIT_CHECK(lchip);

    if (DRV_IS_DUET2(lchip))
    {
        tx_session_chan = SYS_DMA_PACKET_TX1_CHAN_ID;
    }
    else
    {
        tx_session_chan = SYS_DMA_PKT_TX_TIMER_CHAN_ID;
    }

    if (!p_usw_dma_master[lchip]->pkt_tx_timer_en || !p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan].p_desc)
    {
        return CTC_E_INVALID_CONFIG;
    }

    tx_enable = !CTC_FLAG_ISSET(p_pkt->tx_info.flags, CTC_PKT_FLAG_SESSION_PENDING_EN);
    if (DRV_IS_DUET2(lchip))
    {
        max_session = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_depth;
    }
    else
    {
        max_session = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].desc_depth;
    }

    /*new session check session ID*/
    if (tx_enable && !p_usw_dma_master[lchip]->tx_session[session_id].state &&
        (p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan].desc_num >= max_session) && DRV_IS_DUET2(lchip))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_pkt->skb.len > p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan].data_size-40)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[tx_session_chan];

    if (DRV_IS_DUET2(lchip))
    {
        p_mem_addr = g_dal_op.dma_alloc(lchip, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN), 0);
        if (NULL == p_mem_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_mem_addr, 0, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN));
        phy_addr = g_dal_op.logic_to_phy(lchip, p_mem_addr);
        phy_addr_low = phy_addr&0xFFFFFFFF;
        if(p_pkt->skb.tail == p_pkt->skb.data)
        {/*only bridge header is in skb.buf*/
            sal_memcpy((uint8*)p_mem_addr, p_pkt->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN, SYS_USW_PKT_HEADER_LEN);
            sal_memcpy((uint8*)p_mem_addr + SYS_USW_PKT_HEADER_LEN, p_pkt->skb.data, p_pkt->skb.len);
        }
        else
        {/*bridge header and pkt data are in skb.buf*/
            sal_memcpy((uint8*)p_mem_addr, p_pkt->skb.data, p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN);
        }

        cmd = DRV_IOR(DmaStaticInfo_t, DmaStaticInfo_chanEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &tx_start));
        if (tx_enable)
        {
            if (p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                /*session already tx enable, update desc info*/
                desc_idx = p_usw_dma_master[lchip]->tx_session[session_id].desc_idx;
            }
            else
            {
                /*new added session, cfg desc info*/
                desc_idx = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth;
            }
            p_sys_desc = &(p_dma_chan->p_desc[desc_idx].desc_info);
            SetDsDescEncap(V, memAddr_f, p_sys_desc, (phy_addr_low >> 4));
            SetDsDescEncap(V, cfgSize_f, p_sys_desc, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN));
            SetDsDescEncap(V, u1_pkt_eop_f, p_sys_desc, 1);
            SetDsDescEncap(V, u1_pkt_sop_f, p_sys_desc, 1);
            /*flush dma after write*/
            if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_sys_desc), sizeof(DsDesc_m));
            }

            if (!p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                p_usw_dma_master[lchip]->tx_session[session_id].desc_idx = desc_idx;
                p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_num++;
                p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth++;
            }
        }
        else if (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_num &&
            p_usw_dma_master[lchip]->tx_session[session_id].state)
        {
            /*remove tx session, tx timer already start, need re-order dma desc*/
            CTC_ERROR_RETURN(_sys_usw_dma_get_tx_session_from_desc_index(lchip,
                (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_num-1), &last_session_id));
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_num--;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth--;
            is_remove = 1;
        }
        is_update = (p_usw_dma_master[lchip]->tx_session[session_id].state != tx_enable) && tx_start;
        if (is_update)
        {
            uint32 timer_v[2] = {0};
            DmaRegWrTrigCfg_m wr_timer;
            uint64 timer_l = 0;
            uint16 cnt = 0;
            uint8 clear_done = 0;

            /*1. disable table wr trigger */
            value = 0;
            cmd = DRV_IOW(DmaRegTrigEnCfg_t, DmaRegTrigEnCfg_cfgRegWrTrigEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            /*2. confirm tx channel valid num is 0*/
            value = 0;
            do
            {
                cmd = DRV_IOR(DmaCtlTab_t, DmaCtlTab_vldNum_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &value));
                cnt++;
            } while(value&&(cnt<0xffff));

            cmd = DRV_IOR(DmaCtlTab_t, DmaCtlTab_vldNum_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &value));
            if (value)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tx timer channel cannot clear cnt:%u\n", cnt);
                return CTC_E_DMA;
            }

            /*3. clear dma channel dynamic info*/
            value = (1 << SYS_DMA_PACKET_TX1_CHAN_ID);
            cmd = DRV_IOW(DmaClearCtl_t, DmaClearCtl_dmaClearEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            /*3.1 wait clear action done*/
            cnt = 0;
            do
            {
                cmd = DRV_IOR(DmaClearCtl_t, DmaClearCtl_dmaClearPending_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                clear_done = !CTC_IS_BIT_SET(value, SYS_DMA_PACKET_TX1_CHAN_ID);
                cnt++;
            } while(!clear_done&&(cnt<0xffff));

            cmd = DRV_IOR(DmaClearCtl_t, DmaClearCtl_dmaClearPending_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            clear_done = !CTC_IS_BIT_SET(value, SYS_DMA_PACKET_TX1_CHAN_ID);
            if (!clear_done)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Tx timer channel cannot clear cnt:%u\n", cnt);
                return CTC_E_DMA;
            }

            if (is_remove)
            {
                uint16 last_desc_idx = 0;
                uint16 last_size = 0;
                DsDesc_m* p_last_desc = NULL;

                /*disble session tx function, move last sesssion in dma channel to remove position*/
                desc_idx = p_usw_dma_master[lchip]->tx_session[session_id].desc_idx;
                last_desc_idx = p_usw_dma_master[lchip]->tx_session[last_session_id].desc_idx;
                p_sys_desc = &(p_dma_chan->p_desc[desc_idx].desc_info);
                p_last_desc = &(p_dma_chan->p_desc[last_desc_idx].desc_info);
                phy_addr_low = (uint32)p_usw_dma_master[lchip]->tx_session[last_session_id].phy_addr;

                /*inval dma before read*/
                if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
                {
                    g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_last_desc), sizeof(DsDesc_m));
                }
                last_size = GetDsDescEncap(V, cfgSize_f, p_last_desc);
                SetDsDescEncap(V, memAddr_f, p_sys_desc, (phy_addr_low >> 4));
                SetDsDescEncap(V, cfgSize_f, p_sys_desc, last_size);
                /*flush dma after write*/
                if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
                {
                    g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_sys_desc), sizeof(DsDesc_m));
                }
                p_usw_dma_master[lchip]->tx_session[last_session_id].desc_idx = desc_idx;
            }

            /*4. update tx channel ring depth*/
            value = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth;
            cmd = DRV_IOW(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &value));

            /*4. re-config table wr trigger timer*/
            value = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth;
            if(value)
            {
                timer_l = (uint64)p_usw_dma_master[lchip]->tx_timer*1000000/value; /*uints is ms*/
                timer_v[0] = timer_l&0xFFFFFFFF;
                timer_v[1] = (timer_l >> 32) & 0xFFFFFFFF;
                cmd = DRV_IOR(DmaRegWrTrigCfg_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wr_timer));
                SetDmaRegWrTrigCfg(A, cfgRegWrTrigNs_f, &wr_timer, timer_v);
                cmd = DRV_IOW(DmaRegWrTrigCfg_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &wr_timer));

                /*5. recover table wr trigger function*/
                value = 1;
                cmd = DRV_IOW(DmaRegTrigEnCfg_t, DmaRegTrigEnCfg_cfgRegWrTrigEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            }
        }
        else if (is_remove)
        {
            uint16 last_desc_idx = 0;
            uint16 last_size = 0;
            DsDesc_m* p_last_desc = NULL;

            /* move last sesssion in dma channel to remove position*/
            desc_idx = p_usw_dma_master[lchip]->tx_session[session_id].desc_idx;
            last_desc_idx = p_usw_dma_master[lchip]->tx_session[last_session_id].desc_idx;
            p_sys_desc = &(p_dma_chan->p_desc[desc_idx].desc_info);
            p_last_desc = &(p_dma_chan->p_desc[last_desc_idx].desc_info);
            phy_addr_low = (uint32)p_usw_dma_master[lchip]->tx_session[last_session_id].phy_addr;

            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_last_desc), sizeof(DsDesc_m));
            }
            last_size = GetDsDescEncap(V, cfgSize_f, p_last_desc);
            SetDsDescEncap(V, memAddr_f, p_sys_desc, (phy_addr_low >> 4));
            SetDsDescEncap(V, cfgSize_f, p_sys_desc, last_size);
            /*flush dma after write*/
            if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_sys_desc), sizeof(DsDesc_m));
            }
            p_usw_dma_master[lchip]->tx_session[last_session_id].desc_idx = desc_idx;
        }

        /*store session info*/
        if (p_usw_dma_master[lchip]->tx_session[session_id].phy_addr)
        {
            uint32* logic_addr = NULL;
            /*free old memory*/
            logic_addr = g_dal_op.phy_to_logic(lchip, p_usw_dma_master[lchip]->tx_session[session_id].phy_addr);
            g_dal_op.dma_free(lchip, logic_addr);
            p_usw_dma_master[lchip]->tx_session[session_id].phy_addr = 0;
        }

        p_usw_dma_master[lchip]->tx_session[session_id].state = tx_enable;
        if (!is_remove)
        {
            p_usw_dma_master[lchip]->tx_session[session_id].phy_addr = phy_addr;
        }
    }
    else
    {
        if (tx_enable)
        {
            uint32* logic_addr = NULL;

            /*enable session tx*/
            if (p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                /*session already tx enable, update desc info*/
                desc_idx = p_usw_dma_master[lchip]->tx_session[session_id].desc_idx;
            }
            else
            {
                /*get free desc index, temp using session id as desc index*/
                desc_idx = session_id;
            }

            if (desc_idx >= p_dma_chan->desc_depth)
            {
                return CTC_E_NONE;
            }

            p_mem_addr = g_dal_op.dma_alloc(lchip, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN), 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_mem_addr, 0, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN));
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
            if(p_pkt->skb.tail == p_pkt->skb.data)
            {/*only bridge header is in skb.buf*/
                sal_memcpy((uint8*)p_mem_addr, p_pkt->skb.buf + CTC_PKT_HDR_ROOM - SYS_USW_PKT_HEADER_LEN, SYS_USW_PKT_HEADER_LEN);
                sal_memcpy((uint8*)p_mem_addr + SYS_USW_PKT_HEADER_LEN, p_pkt->skb.data, p_pkt->skb.len);
            }
            else
            {/*bridge header and pkt data are in skb.buf*/
                sal_memcpy((uint8*)p_mem_addr, p_pkt->skb.data, p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN);
            }

            p_sys_desc = &(p_dma_chan->p_desc[desc_idx].desc_info);

            sys_usw_dma_function_pause(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 1);
            sal_task_sleep(10);
            SetDsDescEncap(V, memAddr_f, p_sys_desc, (phy_addr >> 4));
            SetDsDescEncap(V, cfgSize_f, p_sys_desc, (p_pkt->skb.len + SYS_USW_PKT_HEADER_LEN));
            SetDsDescEncap(V, u1_pkt_eop_f, p_sys_desc, 1);
            SetDsDescEncap(V, u1_pkt_sop_f, p_sys_desc, 1);
            /*flush dma after write*/
            if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_sys_desc), sizeof(DsDesc_m));
            }
            sys_usw_dma_function_pause(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 0);

            if (!p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                p_usw_dma_master[lchip]->tx_session[session_id].desc_idx = desc_idx;
            }

            /*free old data memory*/
            if (p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                logic_addr = g_dal_op.phy_to_logic(lchip, p_usw_dma_master[lchip]->tx_session[session_id].phy_addr);
                g_dal_op.dma_free(lchip, logic_addr);
            }
            p_usw_dma_master[lchip]->tx_session[session_id].phy_addr = phy_addr;
        }
        else
        {
            uint32* logic_addr = NULL;

            if (!p_usw_dma_master[lchip]->tx_session[session_id].state)
            {
                return CTC_E_NONE;
            }

            sys_usw_dma_function_pause(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 1);
            sal_task_sleep(10);
            /*disable session tx*/
            desc_idx = p_usw_dma_master[lchip]->tx_session[session_id].desc_idx;
            p_sys_desc = &(p_dma_chan->p_desc[desc_idx].desc_info);
            SetDsDescEncap(V, memAddr_f, p_sys_desc, 0);
            SetDsDescEncap(V, cfgSize_f, p_sys_desc, 0);
            SetDsDescEncap(V, u1_pkt_eop_f, p_sys_desc, 1);
            SetDsDescEncap(V, u1_pkt_sop_f, p_sys_desc, 1);
            /*flush dma after write*/
            if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_sys_desc), sizeof(DsDesc_m));
            }
            sys_usw_dma_function_pause(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 0);

            /*free data memory*/
            logic_addr = g_dal_op.phy_to_logic(lchip, p_usw_dma_master[lchip]->tx_session[session_id].phy_addr);
            g_dal_op.dma_free(lchip, logic_addr);
            p_usw_dma_master[lchip]->tx_session[session_id].phy_addr = 0;
        }

        p_usw_dma_master[lchip]->tx_session[session_id].state = tx_enable;
    }
    return CTC_E_NONE;
}
STATIC int32 _sys_usw_dma_reset_channel(uint8 lchip, uint16 chan_id)
{
    uint32 value = 0;
    uint32 cmd   = 0;
    uint32 cnt = 0;
    uint8  clear_done = 0;

    value = (1 << chan_id);
    cmd = DRV_IOW(DmaClearCtl_t, DmaClearCtl_dmaClearEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cnt = 0;
    cmd = DRV_IOR(DmaClearPend_t, DmaClearPend_dmaClearPending_f);
    do
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        clear_done = !CTC_IS_BIT_SET(value, chan_id);
        cnt++;
    } while(!clear_done&&(cnt<0xffff));

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    clear_done = !CTC_IS_BIT_SET(value, chan_id);
    if (!clear_done)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Channel %u can not reset\n", chan_id);
		return CTC_E_DMA;
    }

    return CTC_E_NONE;
}

int32
sys_usw_dma_set_packet_timer_cfg(uint8 lchip, uint16 max_session, uint16 interval, uint16 pkt_len, uint8 is_destroy)
{
    uint32 desc_num = 0;
    sys_dma_desc_t* p_sys_desc_pad = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 phy_addr = 0;
    uint32 cmd = 0;
    DmaStaticInfo_m static_info;
    DmaPktTx3TrigCtl_m tx_timer;
    uint32 timer_v[2] = {0};
    uint64 timer = 0;
    DmaWeightCfg_m dma_weight;
    uint32 cfg_addr = 0;
    void*  p_mem_addr = NULL;
    sys_dma_desc_t* p_sys_desc_tbl = NULL;

    SYS_DMA_INIT_CHECK(lchip);

    if (!p_usw_dma_master[lchip]->pkt_tx_timer_en)
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (!is_destroy)
    {
        if (DRV_IS_DUET2(lchip))
        {
            if (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].p_desc ||
                p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].p_desc)
            {
                return CTC_E_IN_USE;
            }

            /*alloc table write trigger member data*/
            p_mem_addr = g_dal_op.dma_alloc(lchip, 4, 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                return CTC_E_NO_MEMORY;
            }
        }
        else
        {
            if (p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].p_desc)
            {
                return CTC_E_IN_USE;
            }

            p_usw_dma_master[lchip]->tx_timer = interval;
        }
        /*1. process packet tx channel*/
        p_sys_desc_pad = (sys_dma_desc_t*)g_dal_op.dma_alloc(lchip, (max_session) * sizeof(sys_dma_desc_t), 0);
        if (NULL == p_sys_desc_pad)
        {
            g_dal_op.dma_free(lchip, p_mem_addr);
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }

        if (DRV_IS_DUET2(lchip))
        {
            p_sys_desc_tbl = (sys_dma_desc_t*)g_dal_op.dma_alloc(lchip, (max_session)*sizeof(sys_dma_desc_t), 0);
            if (!p_sys_desc_tbl)
            {
                g_dal_op.dma_free(lchip, p_mem_addr);
                g_dal_op.dma_free(lchip, p_sys_desc_pad);
                return CTC_E_NO_MEMORY;
            }

            sal_memset(p_sys_desc_tbl, 0, sizeof(sys_dma_desc_t)*max_session);
        }

        sal_memset(p_sys_desc_pad, 0, sizeof(sys_dma_desc_t)*max_session);

        for (desc_num = 0; desc_num < max_session; desc_num++)
        {
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);
            SetDsDescEncap(V, u1_pkt_eop_f, p_desc, 1);
            SetDsDescEncap(V, u1_pkt_sop_f, p_desc, 1);
            if (DRV_IS_DUET2(lchip))
            {
                SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
                SetDsDescEncap(V, cfgSize_f, p_desc, 41);
            }
            else
            {
                SetDsDescEncap(V, memAddr_f, p_desc, 0);
                SetDsDescEncap(V, cfgSize_f, p_desc, 0);
            }
            /*flush dma after write*/
            if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
            }
        }

        if (DRV_IS_DUET2(lchip))
        {
            /*2. process table write trigger channel*/
            for (desc_num = 0; desc_num < max_session; desc_num++)
            {
                p_desc = (DsDesc_m*)&(p_sys_desc_tbl[desc_num].desc_info);

                *((uint32*)p_mem_addr) = 1;
                phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
                cfg_addr = 0x00028b80 + SYS_DMA_PACKET_TX1_CHAN_ID*4;
                SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
                SetDsDescEncap(V, cfgSize_f, p_desc, 4);
                SetDsDescEncap(V, chipAddr_f, p_desc, cfg_addr);
                SetDsDescEncap(V, dataStruct_f, p_desc, 1);
                SetDsDescEncap(V, pause_f, p_desc, 1);
                /*flush dma after write*/
                if ((NULL != g_dal_op.dma_cache_flush) && (NULL != g_dal_op.logic_to_phy))
                {
                    g_dal_op.dma_cache_flush(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
                }
            }

            /*3. cfg static infor for dma channel:MemBase, ring depth */
            sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_pad);
            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
            SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_PACKET_TX1_CHAN_ID, cmd, &static_info));

            sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_tbl);
            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
            SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
            SetDmaStaticInfo(V, ringDepth_f, &static_info, max_session);
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &static_info));

            /*4. cfg weight */
            sal_memset(&dma_weight, 0, sizeof(DmaWeightCfg_m));
            cmd = DRV_IOR(DmaWeightCfg_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &dma_weight));
            DRV_SET_FIELD_V(lchip, DmaWeightCfg_t, DmaWeightCfg_cfgChan4Weight_f, &dma_weight, SYS_DMA_LOW_WEIGHT);
            DRV_SET_FIELD_V(lchip, DmaWeightCfg_t, DmaWeightCfg_cfgChan5Weight_f, &dma_weight, SYS_DMA_LOW_WEIGHT);
            cmd = DRV_IOW(DmaWeightCfg_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &dma_weight));

            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].p_desc = p_sys_desc_pad;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].mem_base = (uintptr)p_sys_desc_pad;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].channel_id = SYS_DMA_PACKET_TX1_CHAN_ID;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_num = 0;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].desc_depth = 0;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].data_size = pkt_len+40;

            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].p_desc = p_sys_desc_tbl;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].mem_base = (uintptr)p_sys_desc_tbl;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].channel_id = SYS_DMA_TBL_WR_CHAN_ID;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_num = max_session;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_depth = max_session;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].data_size = 4;
            CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, SYS_DMA_TBL_WR_CHAN_ID, 1));
        }
        else
        {
            /*3. cfg static infor for dma channel:MemBase, ring depth */
            sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_pad);
            cmd = DRV_IOR(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));
            SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
            SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
            cmd = DRV_IOW(DmaStaticInfo_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, cmd, &static_info));

            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].p_desc = p_sys_desc_pad;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].mem_base = (uintptr)p_sys_desc_pad;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].channel_id = SYS_DMA_PKT_TX_TIMER_CHAN_ID;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].desc_num = max_session;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].desc_depth = max_session;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].data_size = pkt_len+40;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].chan_en = 1;

            /*tm cfg packet tx timer*/
            timer = (uint64)interval*1000000/DOWN_FRE_RATE/max_session; /*1ms*/
            timer_v[0] = timer&0xFFFFFFFF;
            timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
            cmd = DRV_IOR(DmaPktTx3TrigCtl_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &tx_timer));
            SetDmaPktTx3TrigCtl(A, cfgPktTx3TrigNs_f, &tx_timer, timer_v);
            SetDmaPktTx3TrigCtl(V, cfgPktTx3TrigEn_f, &tx_timer, 1);
            cmd = DRV_IOW(DmaPktTx3TrigCtl_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &tx_timer));
        }
        CTC_ERROR_RETURN(sys_usw_dma_set_chan_en(lchip, SYS_DMA_PKT_TX_TIMER_CHAN_ID, 1));
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            uint64 phy_addr = 0;
            uint32* logic_addr = NULL;
            uint8  chan_id[2] = {SYS_DMA_PACKET_TX1_CHAN_ID, SYS_DMA_TBL_WR_CHAN_ID};
            uint8  index  = 0;
            uint8  mem_free = 1;

            if (!p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX1_CHAN_ID].p_desc ||
                !p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].p_desc)
            {
                return CTC_E_NONE;
            }

            for (index = 0; index < 2; index++)
            {
                mem_free = 1;
                p_sys_desc_pad = p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].p_desc;
                /*clear data*/
                for (desc_num = 0; desc_num < p_usw_dma_master[lchip]->dma_chan_info[chan_id[index]].desc_depth; desc_num++)
                {
                    p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);
                    /*inval dma before read*/
                    if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
                    {
                        g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
                    }
                    COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                                    (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);
                    logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);
                    if (logic_addr && mem_free)
                    {
                        /*table wr desc data using only one data memory, just need free once*/
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

            sal_memset(p_usw_dma_master[lchip]->tx_session, 0, sizeof(sys_dma_timer_session_t)*SYS_PKT_MAX_TX_SESSION);

        }
        else
        {
            uint64 phy_addr = 0;
            uint32* logic_addr = NULL;
            uint8  chan_id  = SYS_DMA_PKT_TX_TIMER_CHAN_ID;

            if (!p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].p_desc)
            {
                return CTC_E_NONE;
            }

            p_sys_desc_pad = p_usw_dma_master[lchip]->dma_chan_info[chan_id ].p_desc;
            /*clear data*/
            for (desc_num = 0; desc_num < p_usw_dma_master[lchip]->dma_chan_info[chan_id].desc_depth; desc_num++)
            {
                p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);
                /*inval dma before read*/
                if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
                {
                    g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
                }
                COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                                (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);
                logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);
                if (logic_addr)
                {
                    /*table wr desc data using only one data memory, just need free once*/
                    g_dal_op.dma_free(lchip, logic_addr);
                }
            }

            g_dal_op.dma_free(lchip, p_sys_desc_pad);
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].p_desc = NULL;
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].mem_base = 0;
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].channel_id = 0;
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].desc_num = 0;
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].desc_depth = 0;
            p_usw_dma_master[lchip]->dma_chan_info[chan_id].data_size = 0;
            p_usw_dma_master[lchip]->tx_timer = 0;
            p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PKT_TX_TIMER_CHAN_ID].chan_en = 0;

            sal_memset(p_usw_dma_master[lchip]->tx_session, 0, sizeof(sys_dma_timer_session_t)*SYS_PKT_MAX_TX_SESSION);
        }
    }

    return CTC_E_NONE;
}
int32
sys_usw_dma_performance_test(uint8 lchip, ctc_pkt_tx_t* p_pkt_tx)
{
    int32    ret = CTC_E_NONE;
    uint32   cmd = 0;
    uint32   value = 1;
    DmaCtlTab_m       ctl_tab;
    DmaTsDebugStats_m debug_stats;
    DmaTestModeLog_m  test_log;
    DmaRegTrigEnCfg_m reg_trig;
    DmaRegTrigEnCfg_m reg_trig_w;
    DmaCtlDrainEnable_m dma_drain;
    DmaCtlDrainEnable_m dma_drain_w;
    sys_dma_tx_mem_t  tx_mem_info;
    uint32 start_time[2] = {0};
    uint32 end_time[2] = {0};
    uint32 value_array[2] = {0};
    uint64 byte_count = 0;
    uint64 pkt_count = 0;
    uint64 start_time_v = 0;
    uint64 end_time_v = 0;
    sys_dma_chan_t* p_dma_chan = NULL;
    uint8  chan_id = 0;
    drv_work_platform_type_t platform_type;

    drv_get_platform_type(lchip, &platform_type);
    if (platform_type != HW_PLATFORM)
    {
        return CTC_E_NONE;
    }
    cmd = DRV_IOR(DmaCtlTab_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_DMA_PACKET_TX0_CHAN_ID, cmd, &ctl_tab));
    if(GetDmaCtlTab(V, vldNum_f, &ctl_tab))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Dma tx is working, can not do performance test\n");
        return CTC_E_HW_BUSY;
    }
    sal_memset(&tx_mem_info, 0, sizeof(tx_mem_info));
    p_dma_chan = &(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID]);
    DMA_LOCK(p_dma_chan->p_mutex);

    cmd = DRV_IOR(DmaRegTrigEnCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &reg_trig);
    sal_memset(&reg_trig_w, 0, sizeof(reg_trig_w));
    cmd = DRV_IOW(DmaRegTrigEnCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &reg_trig_w);

    cmd = DRV_IOR(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dma_drain);
    sal_memset(&dma_drain_w, 0, sizeof(dma_drain_w));
    SetDmaCtlDrainEnable(V,dmaPktTxDrainEn_f, &dma_drain_w, 1);
    cmd = DRV_IOW(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dma_drain_w);

    chan_id = SYS_DMA_PACKET_TX0_CHAN_ID;

    CTC_ERROR_GOTO(_sys_usw_dma_reset_channel(lchip, chan_id), ret, error_end);
    value = 1;
    cmd  = DRV_IOW(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &value), ret, error_end);
    CTC_ERROR_GOTO(sys_usw_dma_clear_pkt_stats(lchip, 0), ret, error_end);

    value = 0;
    cmd = DRV_IOW(DmaPktStatsCfg_t, DmaPktStatsCfg_clearOnRead_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error_end);

    value = 1;
    cmd = DRV_IOW(DmaMiscCfg_t, DmaMiscCfg_cfgDmaPktTestEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error_end);

    if(g_dal_op.dma_alloc)
    {
        tx_mem_info.addr = (uintptr)g_dal_op.dma_alloc(lchip, p_pkt_tx->skb.len + SYS_USW_PKT_HEADER_LEN, 0);
    }
    sal_memcpy((uint8*)tx_mem_info.addr, p_pkt_tx->skb.data, p_pkt_tx->skb.len + SYS_USW_PKT_HEADER_LEN);
    tx_mem_info.pkt_len = p_pkt_tx->skb.len + SYS_USW_PKT_HEADER_LEN;

    CTC_ERROR_GOTO(_sys_usw_dma_set_tx_chip(lchip, &(p_dma_chan->p_desc[0].desc_info), &tx_mem_info, p_dma_chan), ret, error_end);

    cmd = DRV_IOR(DmaTestModeLog_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &test_log), ret, error_end);

    sal_task_sleep(2*60*1000);

    cmd = DRV_IOR(DmaTsDebugStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &debug_stats), ret, error_end);

    GetDmaTsDebugStats(A, dmaTsLogChan0_f, &debug_stats, start_time);
    GetDmaTsDebugStats(A, dmaTsLogChan1_f, &debug_stats, end_time);
    GetDmaTsDebugStats(A, dmaTsLogChan3_f, &debug_stats, value_array);
    byte_count = value_array[1]&0x7FF;
    byte_count = (byte_count<< 32)|value_array[0];
    pkt_count = GetDmaTestModeLog(V, dmaPktTxTestPktCnt_f, &test_log);
    pkt_count = (pkt_count<<20)|((value_array[1]) >> 12);
    start_time_v = start_time[1];
    start_time_v = start_time_v << 32 | start_time[0];
    end_time_v = end_time[1];
    end_time_v = end_time_v << 32 | end_time[0];

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-36s\n", "Pcie Performance Test");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-36s\n", "====================================");
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %-20"PRIu64"\n", "packet count", pkt_count-256);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %-20"PRIu64"\n", "packet byte(kbyte)", (byte_count-256*(tx_mem_info.pkt_len))/1024);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %-20"PRIu64"\n", "used   time(s)", ((end_time_v-start_time_v)/1000/1000/1000));
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %-20"PRIu64"\n", "pps", (pkt_count-256)/((end_time_v-start_time_v)/1000/1000/1000));
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-16s: %-20"PRIu64"\n", "pbs(Mbits/s)", (byte_count-256*(tx_mem_info.pkt_len))*8/((end_time_v-start_time_v)/1000/1000/1000)/1024/1024);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-36s\n", "====================================");
error_end:
    value = 0;
    cmd = DRV_IOW(DmaMiscCfg_t, DmaMiscCfg_cfgDmaPktTestEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    value = 1;
    cmd = DRV_IOW(DmaPktStatsCfg_t, DmaPktStatsCfg_clearOnRead_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    sys_usw_dma_clear_pkt_stats(lchip, 0);

    _sys_usw_dma_reset_channel(lchip, chan_id);

    value = p_dma_chan->desc_depth;
    cmd  = DRV_IOW(DmaStaticInfo_t, DmaStaticInfo_ringDepth_f);
    DRV_IOCTL(lchip, chan_id, cmd, &value);
    if(tx_mem_info.addr && g_dal_op.dma_free)
    {
        g_dal_op.dma_free(lchip, (void*)tx_mem_info.addr);
    }

    p_dma_chan->current_index = 0;
    cmd = DRV_IOW(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dma_drain);

    cmd = DRV_IOW(DmaRegTrigEnCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &reg_trig);

    DMA_UNLOCK(p_dma_chan->p_mutex);
    return ret;
}

#define _DMA_ISR_BEGIN

int32
_sys_usw_dma_knet_chan_rx_func(uint8 lchip, uint8 chan, uint8 dmasel)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 desc_done = 0;
    uint32 kernel_done = 0;
    uint32 process_ct = 0;

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan];
    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;

    for (;; cur_index++)
    {
        if (cur_index >= p_dma_chan->desc_depth)
        {
            cur_index = 0;
        }

        if (process_ct >= p_dma_chan->desc_depth)
        {
            break;
        }

        p_desc = &(p_base_desc[cur_index].desc_info);
        desc_done = GetDsDescEncap(V, done_f, p_desc);
        kernel_done = GetDsDescEncap(V, reserved0_f, p_desc);

        if (!desc_done)
        {
            break;
        }
        else if (!kernel_done)
        {
            do
            {
                kernel_done = GetDsDescEncap(V, reserved0_f, p_desc);
                if (sys_usw_chip_check_active(lchip))
                {
                    break;
                }
            } while(!kernel_done);
        }

        process_ct++;

        _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
    }
    p_dma_chan->current_index = cur_index;

    return CTC_E_NONE;
}

/**
@brief  DMA packet rx function process
*/
int32
_sys_usw_dma_pkt_rx_func(uint8 lchip, uint8 chan, uint8 dmasel)
{
    ctc_pkt_buf_t pkt_buf[64];
    ctc_pkt_rx_t pkt_rx;
    ctc_pkt_rx_t* p_pkt_rx = &pkt_rx;
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 sop_index = 0;
    uint32 buf_count = 0;
    uint64 phy_addr = 0;
    uint32 process_cnt = 0;
    uint32 index = 0;
    uint32 is_sop = 0;
    uint32 is_eop = 0;
    uint32 desc_done = 0;
    uint8 need_eop = 0;
    uint32 wait_cnt = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s(), %d, %d\n", __FUNCTION__, chan, dmasel);

    sal_memset(p_pkt_rx, 0, sizeof(ctc_pkt_rx_t));
    p_pkt_rx->mode = CTC_PKT_MODE_DMA;
    p_pkt_rx->pkt_buf = pkt_buf;

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan];
    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;
    for (;; cur_index++)
    {
        SYS_DMA_INIT_CHECK(lchip);

        if (cur_index >= p_dma_chan->desc_depth)
        {
            cur_index = 0;
        }

        p_desc = &(p_base_desc[cur_index].desc_info);
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }
        desc_done = GetDsDescEncap(V, done_f, p_desc);

        if (0 == desc_done)
        {
            if (need_eop)
            {
                 SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Desc not done, But need eop!!desc index %d\n", cur_index);

                while(wait_cnt < 0xffff)
                {
                    /*inval dma before read*/
                    if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
                    {
                        g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
                    }
                    if (GetDsDescEncap(V, done_f, p_desc))
                    {
                        break;
                    }
                    wait_cnt++;
                }

                /* Cannot get EOP, means no EOP packet error, just clear desc*/
                if (wait_cnt >= 0xffff)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "No EOP, desc index %d, buf_count %d\n", cur_index, buf_count);
                   _sys_usw_dma_clear_desc(lchip, p_dma_chan, sop_index, buf_count, NULL);
                   buf_count = 0;
                   need_eop = 0;
                   break;
                }
                wait_cnt = 0;
            }
            else
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "No desc is not done, processed %d desc index %d\n", process_cnt, cur_index);
                break;
            }
        }

        is_sop = GetDsDescEncap(V, u1_pkt_sop_f, p_desc);
        is_eop = GetDsDescEncap(V, u1_pkt_eop_f, p_desc);

        /*Before get EOP, next packet SOP come, no EOP packet error, drop error packet */
        if (need_eop && is_sop)
        {
            _sys_usw_dma_clear_desc(lchip, p_dma_chan, sop_index, buf_count, NULL);
            buf_count = 0;
            need_eop = 0;
        }

        /* Cannot get SOP, means no SOP packet error, just clear desc*/
        if (0 == buf_count)
        {
            if (0 == is_sop)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                                "[DMA] PKT RX error, lchip %d chan %d index %d first is not SOP\n", lchip, chan, cur_index);
                _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
                goto error;
            }
        }

        if (is_sop)
        {
            sop_index = cur_index;
            p_pkt_rx->pkt_len = 0;
            need_eop = 1;
        }

        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                        (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);

        /*Max desc num for one packet is 64*/
        if (buf_count < 64)
        {
            p_pkt_rx->pkt_buf[buf_count].data = (uint8*)g_dal_op.phy_to_logic(lchip, phy_addr);
            p_pkt_rx->pkt_buf[buf_count].len = GetDsDescEncap(V, realSize_f, p_desc);
        }
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, phy_addr, p_pkt_rx->pkt_buf[buf_count].len);
        }

        buf_count++;

        if (is_eop)
        {
            p_pkt_rx->buf_count = buf_count;
            p_pkt_rx->dma_chan = chan;

            for (index = 0; index < buf_count; index++)
            {
                p_pkt_rx->pkt_len += p_pkt_rx->pkt_buf[index].len;
            }

            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO,
                            "[DMA] PKT RX lchip %d dma_chan %d buf_count %d pkt_len %d curr_index %d\n",
                            lchip, p_pkt_rx->dma_chan, p_pkt_rx->buf_count, p_pkt_rx->pkt_len,
                            cur_index);

            sys_usw_packet_rx(lchip, p_pkt_rx);

            process_cnt += buf_count;
           _sys_usw_dma_clear_desc(lchip, p_dma_chan, sop_index, buf_count, NULL);

            buf_count = 0;
            need_eop = 0;
        }

error:
        /* one interrupt process desc_depth max, for other channel using same sync channel to be processed in time */
        if ((process_cnt >= p_dma_chan->desc_depth) && (!need_eop))
        {
            cur_index++;
            break;
        }
    }

    SYS_DMA_INIT_CHECK(lchip);
    p_dma_chan->current_index = (cur_index>=p_dma_chan->desc_depth)?(cur_index%p_dma_chan->desc_depth):cur_index;

    return CTC_E_NONE;

}

#if 0
uint32 learn_cnt = 0;
sal_systime_t tv0;
sal_systime_t tv1;
#endif

/**
@brief  DMA info function process
*/
STATIC int32
_sys_usw_dma_info_func(uint8 lchip, uint8 chan, uint8 dmasel)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 desc_done = 0;
    uint32 process_cnt = 0;
    uint64 phy_addr = 0;
    uint32 real_size = 0;
    sys_dma_info_t dma_info;
    uint8 index = 0;
    uint32* logic_addr = NULL;
    uint8 need_process = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[DMA] ISR TX lchip %d chan %d dmasel:%d\n", lchip, chan, dmasel);

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);

    sal_memset(&dma_info, 0, sizeof(sys_dma_info_t));

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan];

    p_base_desc = p_dma_chan->p_desc;
    cur_index = p_dma_chan->current_index;

    while(1)
    {
        SYS_DMA_INIT_CHECK(lchip);
        if (cur_index >= p_usw_dma_master[lchip]->dma_chan_info[chan].desc_depth)
        {
            cur_index = 0;
        }

        p_desc = &(p_base_desc[cur_index].desc_info);
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }
        desc_done = GetDsDescEncap(V, done_f, p_desc);

        /* get realsize from desc */
        real_size = GetDsDescEncap(V, realSize_f, p_desc);

        if (1 == desc_done)
        {
            need_process = 1;
        }
        else
        {
            need_process = 0;
        }

        if (!need_process)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "No desc is not done, processed %d desc\n", process_cnt);
            break;
        }

        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);

        logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);

        if (real_size > p_dma_chan->cfg_size)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Info realsize error real:%d cfg:%d\n", real_size, p_dma_chan->cfg_size);
            return CTC_E_INVALID_PARAM;
        }

        switch(p_dma_chan->func_type)
        {
            case  CTC_DMA_FUNC_HW_LEARNING:
#if 0
if (learn_cnt == 0)
{
    sal_gettime(&tv0);
}
#endif
                for (index = 0; index < real_size/SYS_DMA_LEARN_AGING_SYNC_CNT; index++)
                {
                    dma_info.entry_num = SYS_DMA_LEARN_AGING_SYNC_CNT;
                    dma_info.p_data = (void*)((sys_dma_learning_info_t*)logic_addr + index*SYS_DMA_LEARN_AGING_SYNC_CNT);
 /*learn_cnt += SYS_DMA_LEARN_AGING_SYNC_CNT;*/
                    process_cnt += SYS_DMA_LEARN_AGING_SYNC_CNT;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_LERNING])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_LERNING](lchip, (void*)&dma_info);
                    }
                }

                if (real_size%SYS_DMA_LEARN_AGING_SYNC_CNT)
                {
                    dma_info.entry_num = real_size%SYS_DMA_LEARN_AGING_SYNC_CNT;
                    dma_info.p_data = (void*)((sys_dma_learning_info_t*)logic_addr + index*SYS_DMA_LEARN_AGING_SYNC_CNT);

 /*learn_cnt += dma_info.entry_num;*/

                    process_cnt += dma_info.entry_num;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_LERNING])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_LERNING](lchip, (void*)&dma_info);
                    }
                }

                break;

            case  CTC_DMA_FUNC_IPFIX:
                for (index = 0; index < real_size/SYS_DMA_IPFIX_SYNC_CNT; index++)
                {
                    dma_info.entry_num = SYS_DMA_IPFIX_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuIpfixAccFifo_m*)logic_addr +index*SYS_DMA_IPFIX_SYNC_CNT);

                    process_cnt += SYS_DMA_IPFIX_SYNC_CNT;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_IPFIX])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_IPFIX](lchip, (void*)&dma_info);
                    }
                }

                if (real_size%SYS_DMA_IPFIX_SYNC_CNT)
                {
                    dma_info.entry_num = real_size%SYS_DMA_IPFIX_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuIpfixAccFifo_m*)logic_addr + index*SYS_DMA_IPFIX_SYNC_CNT);

                    process_cnt +=  dma_info.entry_num;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_IPFIX])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_IPFIX](lchip, (void*)&dma_info);
                    }
                }
                break;

            case CTC_DMA_FUNC_SDC:
                for (index = 0; index < (real_size)/SYS_DMA_SDC_SYNC_CNT; index++)
                {
                    dma_info.entry_num = SYS_DMA_SDC_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuSdcFifo_m*)logic_addr +index*SYS_DMA_SDC_SYNC_CNT);

                    process_cnt += SYS_DMA_SDC_SYNC_CNT;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_SDC_STATS])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_SDC_STATS](lchip, (void*)&dma_info);
                    }
                }

                if ((real_size)%SYS_DMA_SDC_SYNC_CNT)
                {
                    dma_info.entry_num = (real_size)%SYS_DMA_SDC_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuSdcFifo_m*)logic_addr + index*SYS_DMA_SDC_SYNC_CNT);

                    process_cnt +=  dma_info.entry_num;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_SDC_STATS])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_SDC_STATS](lchip, (void*)&dma_info);
                    }
                }
                break;
            case CTC_DMA_FUNC_MONITOR:

                for (index = 0; index < real_size/SYS_DMA_MONITOR_SYNC_CNT; index++)
                {
                    dma_info.entry_num = SYS_DMA_MONITOR_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuActMonErmFifo_m*)logic_addr + (index)*SYS_DMA_MONITOR_SYNC_CNT);
                    process_cnt +=  SYS_DMA_MONITOR_SYNC_CNT;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_MONITOR])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_MONITOR](lchip, (void*)&dma_info);
                    }
                }

                if (real_size%SYS_DMA_MONITOR_SYNC_CNT)
                {
                    dma_info.entry_num = real_size%SYS_DMA_MONITOR_SYNC_CNT;
                    dma_info.p_data = (void*)((DmaToCpuActMonErmFifo_m*)logic_addr + (index)*SYS_DMA_MONITOR_SYNC_CNT);
                    process_cnt +=  dma_info.entry_num;

                    if (p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_MONITOR])
                    {
                        p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_MONITOR](lchip, (void*)&dma_info);
                    }
                }

                break;
            default:
                break;
        }

#if 0
if (learn_cnt == 49152)
{
sal_gettime(&tv1);
}
#endif
        _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
        cur_index++;
        /* one interrupt process 1000 entry, for other channel using same sync channel to be processed in time */
        if (process_cnt >= 1000)
        {
            break;
        }
    }

    SYS_DMA_INIT_CHECK(lchip);
    p_dma_chan->current_index = (cur_index>=p_dma_chan->desc_depth)?(cur_index%p_dma_chan->desc_depth):cur_index;

    return CTC_E_NONE;
}


int32
_sys_usw_dma_wr_func(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    DmaCtlTab_m tab_ctl;
    uint32 valid_num = 0;
    uint32 session_num = 0;

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);

    session_num = p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID].desc_depth;

    sal_memset(&tab_ctl, 0, sizeof(DmaCtlTab_m));
    tbl_id = DmaCtlTab_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &tab_ctl));
    valid_num = GetDmaCtlTab(V, vldNum_f, &tab_ctl);
    if (session_num > valid_num)
    {
        SetDmaCtlTab(V, vldNum_f, &tab_ctl, (session_num-valid_num));
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        (DRV_IOCTL(lchip, SYS_DMA_TBL_WR_CHAN_ID, cmd, &tab_ctl));
    }
    return CTC_E_NONE;
}

/**
@brief DMA stats function process
*/
int32
sys_usw_dma_stats_func(uint8 lchip, uint8 chan, uint8 dmasel)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    sys_dma_desc_info_t* p_desc_info = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 desc_done = 0;
    uint32 process_cnt = 0;
    uint64 phy_addr = 0;
    uint32 real_size = 0;
    uint32* logic_addr = NULL;
    uint8 mac_id = 0;
    sys_dma_reg_t dma_reg;
    uint8 slice_id = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[DMA] ISR TX lchip %d chan %d dmasel:%d\n", lchip, chan, dmasel);

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);

    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan];
    DMA_LOCK(p_dma_chan->p_mutex);
    p_base_desc = p_dma_chan->p_desc;
    p_desc_info = p_dma_chan->p_desc_info;
    if (!p_desc_info && (chan == SYS_DMA_FLOW_STATS_CHAN_ID || chan == SYS_DMA_PORT_STATS_CHAN_ID))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "desc info not init\n");
        DMA_UNLOCK(p_dma_chan->p_mutex);
        return CTC_E_NOT_INIT;
    }
    if (0 == p_dma_chan->chan_en)
    {
        DMA_UNLOCK(p_dma_chan->p_mutex);
        return CTC_E_NOT_SUPPORT;
    }

    /* gg dma stats process desc from index 0 to max_desc*/
    for (cur_index = p_dma_chan->current_index; cur_index < p_dma_chan->desc_depth; cur_index++)
    {
        p_desc = &(p_base_desc[cur_index].desc_info);
        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }
        desc_done = GetDsDescEncap(V, done_f, p_desc);

        if (0 == desc_done)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "No desc is not done, processed %d desc\n", process_cnt);
            break;
        }

        process_cnt++;

        if (chan == SYS_DMA_PORT_STATS_CHAN_ID)
        {
            mac_id = p_desc_info[cur_index].value0;
            if (DRV_IS_DUET2(lchip) && !SYS_MAC_IS_VALID(lchip, mac_id))
            {
                _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);
                continue;
            }
        }

        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);

        logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);
        dma_reg.p_data = logic_addr;

        /* get realsize from desc */
        real_size = GetDsDescEncap(V, realSize_f, p_desc);
        if ((real_size != p_dma_chan->cfg_size) && (0 != p_dma_chan->cfg_size))
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ag stats realsize error real:%d cfg:%d\n", real_size, p_dma_chan->cfg_size);
            DMA_UNLOCK(p_dma_chan->p_mutex);
            return CTC_E_INVALID_PARAM;
        }

        if (chan == SYS_DMA_PORT_STATS_CHAN_ID)
        {

            dma_reg.p_ext = &mac_id;
            if ( p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_PORT_STATS])
            {
                p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_PORT_STATS](lchip, (void*)&dma_reg);
            }
        }
        else if (chan == SYS_DMA_FLOW_STATS_CHAN_ID)
        {
            dma_reg.p_ext = &(p_desc_info[cur_index].value0);
            if ( p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_FLOW_STATS])
            {
                p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_FLOW_STATS](lchip, (void*)&dma_reg);
            }
        }
        else
        {
            if (DRV_IS_DUET2(lchip))
            {
                slice_id = (cur_index%2);
                dma_reg.p_ext = &slice_id;
                if ( p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_FLOW_STATS])
                {
                    p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_FLOW_STATS](lchip, (void*)&dma_reg);
                }
            }
            else
            {
                uint8 type = cur_index;
                dma_reg.p_ext = &type;
                if ( p_usw_dma_master[lchip]&&p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_DOT1AE_STATS])
                {
                    p_usw_dma_master[lchip]->dma_cb[SYS_DMA_CB_TYPE_DOT1AE_STATS](lchip, (void*)&dma_reg);
                }
            }
        }

        _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);

    }

    p_dma_chan->current_index = ((p_dma_chan->current_index + process_cnt) % (p_dma_chan->desc_depth));

    sys_usw_dma_sync_pkt_rx_stats(lchip);
    sys_usw_dma_sync_pkt_tx_stats(lchip);
    DMA_UNLOCK(p_dma_chan->p_mutex);

    return CTC_E_NONE;
}

/**
@brief DMA packet rx thread for packet rx channel
*/
STATIC void
_sys_usw_dma_pkt_rx_thread(void* param)
{
    sys_dma_thread_t* p_thread_info = (sys_dma_thread_t*)param;
    int32 prio = 0;
    int32 ret = 0;
    uint8 dmasel = 0;
    sys_intr_type_t type;
    uint8 act_chan = 0;
    sys_dma_chan_t* p_dma_chan = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    DmaCtlIntrFunc_m intr_ctl;
    uint8 lchip = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    prio = p_thread_info->prio;
    lchip = p_thread_info->lchip;
    sal_task_set_priority(prio);

    while (1)
    {
        ret = sal_sem_take(p_thread_info->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        ret = sys_usw_chip_get_reset_hw_en(lchip);
        if (ret > 0)
        {
            /*in reset hw recover*/
            return;
        }

        type.intr = SYS_INTR_DMA+dmasel;

        /* scan all channel using same sync channel, process one by one */
        for(index = 0; index < p_thread_info->chan_num; index++)
        {
            act_chan = p_thread_info->chan_id[index];

            /* check channel is enable or not */
            if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
            {
                continue;
            }

            dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;
            p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[act_chan];
            cur_index = p_dma_chan->current_index;
            p_desc = &(p_dma_chan->p_desc[cur_index].desc_info);

            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+act_chan;
            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
            }
            if (GetDsDescEncap(V, done_f, p_desc) == 0)
            {
                /* cur channel have no desc to process */
                /* just release mask channel isr */
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Interrupt Occur, But no desc need process!!!chan:%d \n", act_chan);
                sys_usw_interrupt_set_en(lchip, &type, TRUE);
                continue;
            }

            if  (p_dma_chan->pkt_knet_en)
            {
                 ret = _sys_usw_dma_knet_chan_rx_func(lchip, act_chan, dmasel);
                 if (ret == CTC_E_NOT_INIT)
                {
                    return;
                }
            }
            else
            {
                ret = _sys_usw_dma_pkt_rx_func(lchip, act_chan, dmasel);
                if (ret == CTC_E_NOT_INIT)
                {
                    return;
                }
            }
            /* release mask channel isr */
            sys_usw_interrupt_set_en(lchip, &type, TRUE);

            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)&(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)), sizeof(DsDesc_m));
            }
            /*trigger interrupt once more, to avoid desc do not process completely*/
            if (GetDsDescEncap(V, done_f, &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)))
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dma Trigger interrupt chan:%d \n", act_chan);
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
                SetDmaCtlIntrFunc(V, dmaIntrValidVec_f, &intr_ctl, (1<<act_chan));
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
            }
        }
    }

    return;
}

/*SYSTEM MODIFIED by xgu for cmodel performance optimize, bug 49399, 2018-10-11*/
#if SDK_WORK_PLATFORM
int32
sys_usw_dma_pkt_rx_process(uint8 chan_id)
{
    sys_dma_thread_t* p_thread_info;
    int32 ret = 0;
    uint8 dmasel = 0;
    uint8 act_chan = 0;
    sys_dma_chan_t* p_dma_chan = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    DmaCtlIntrFunc_m intr_ctl;
    uint8 lchip = 0;
    uint8 sync_chan = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sync_chan = p_usw_dma_master[0]->dma_chan_info[chan_id].sync_chan;
    p_thread_info = ctc_vector_get(p_usw_dma_master[0]->p_thread_vector, sync_chan);
    if (!p_thread_info)
    {
        /*means no need to create sync thread*/
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma init wrong please check!!chan:%d \n", chan_id);
        return -1;
    }

    /* scan all channel using same sync channel, process one by one */
    for(index = 0; index < p_thread_info->chan_num; index++)
    {
        act_chan = p_thread_info->chan_id[index];

        /* check channel is enable or not */
        if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
        {
            continue;
        }

        dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;
        p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[act_chan];
        cur_index = p_dma_chan->current_index;
        p_desc = &(p_dma_chan->p_desc[cur_index].desc_info);

        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
        }
        if (GetDsDescEncap(V, done_f, p_desc) == 0)
        {
            /* cur channel have no desc to process */
            /* just release mask channel isr */
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Interrupt Occur, But no desc need process!!!chan:%d \n", act_chan);
            continue;
        }

        if  (p_dma_chan->pkt_knet_en)
        {
            return -1;
        }
        else
        {
            ret = _sys_usw_dma_pkt_rx_func(lchip, act_chan, dmasel);
            if (ret == CTC_E_NOT_INIT)
            {
                return ret;
            }
        }

        /*inval dma before read*/
        if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
        {
            g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)&(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)), sizeof(DsDesc_m));
        }
        /*trigger interrupt once more, to avoid desc do not process completely*/
        if (GetDsDescEncap(V, done_f, &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)))
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dma Trigger interrupt chan:%d \n", act_chan);
            cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
            SetDmaCtlIntrFunc(V, dmaIntrValidVec_f, &intr_ctl, (1<<act_chan));
            cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
        }
    }

    return CTC_E_NONE;
}
#endif


STATIC int32
_sys_usw_dma_async_tx_by_channel(uint8 lchip,uint8 chan_idx)
{

    int32 ret = CTC_E_NONE;
    DsDesc_m* p_tx_desc_mem = NULL;
    sys_dma_chan_t* p_dma_chan = NULL;
    uint64 phy_addr = 0;
    sal_mutex_t* p_mutex = NULL;
    sys_dma_tx_mem_t* p_tx_mem_info = NULL;
    ctc_slist_t* tx_list = NULL;
    void* logic_addr = NULL;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_INIT_CHECK(lchip);

    p_dma_chan = (sys_dma_chan_t*)&(p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PACKET_TX0_CHAN_ID + chan_idx]);
    CTC_PTR_VALID_CHECK(p_dma_chan->p_desc_used);

    p_mutex = p_dma_chan->p_mutex;

    if(0 == chan_idx)
    {
        tx_list = p_usw_dma_master[lchip]->tx_pending_list_L;
    }
    else
    {
        tx_list = p_usw_dma_master[lchip]->tx_pending_list_H;
    }
    DMA_LOCK(p_mutex);
    while(tx_list->count > 0)
    {
        if(0 != sys_usw_chip_check_active(lchip))
        {
            DMA_UNLOCK(p_mutex);
            return CTC_E_INVALID_CHIP_ID;
        }
        p_tx_desc_mem = &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info);
        if (p_tx_desc_mem == NULL)
        {
            DMA_UNLOCK(p_mutex);
            return CTC_E_DMA;
        }
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dma_chan->current_index(%u)\n", p_dma_chan->current_index);
        CTC_ERROR_GOTO(_sys_usw_dma_wait_desc_finish_tx(lchip, p_tx_desc_mem,p_dma_chan),ret, error);
        /*process old pkt on this desc*/
        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_tx_desc_mem)<<4), phy_addr);

        if (g_dal_op.phy_to_logic)
        {
            logic_addr= g_dal_op.phy_to_logic(lchip, (phy_addr));
        }
        _sys_usw_dma_clear_desc(lchip, p_dma_chan, p_dma_chan->current_index, 1, logic_addr);

        /* send new pkt */
        sal_spinlock_lock(p_usw_dma_master[lchip]->p_tx_mutex);
        p_tx_mem_info = (sys_dma_tx_mem_t*)CTC_SLISTHEAD(tx_list);
        if(!p_tx_mem_info)
        {
            sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);
            break;
        }
        ctc_slist_delete_node(tx_list, (ctc_slistnode_t*)p_tx_mem_info);
        sal_spinlock_unlock(p_usw_dma_master[lchip]->p_tx_mutex);

        CTC_ERROR_GOTO(_sys_usw_packet_txinfo_to_rawhdr(lchip, &p_tx_mem_info->tx_info, (uint32*)p_tx_mem_info->addr, 1, CTC_PKT_MODE_DMA, NULL), ret, error);

        CTC_ERROR_GOTO(_sys_usw_dma_set_tx_chip(lchip, p_tx_desc_mem, p_tx_mem_info, p_dma_chan), ret, error);
    }
    error:
    DMA_UNLOCK(p_mutex);
    return ret;
}


/**
@brief DMA packet tx thread for async
*/
STATIC void
_sys_usw_dma_pkt_tx_thread(void *param)
{
    int32 ret = 0;
    uint8 lchip = (uintptr)param;
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    while (1)
    {
        ret = sal_sem_take(p_usw_dma_master[lchip]->tx_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }

        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        ret = sys_usw_chip_get_reset_hw_en(lchip);
        if (ret > 0)
        {
            /*in reset hw recover*/
            return;
        }

        if(p_usw_dma_master[lchip]->tx_pending_list_H->count > 0)
        {
            ret = _sys_usw_dma_async_tx_by_channel(lchip,1);
            if (CTC_E_NOT_INIT == ret)
            {
                break;
            }
        }
        if(p_usw_dma_master[lchip]->tx_pending_list_L->count > 0)
        {
            ret = _sys_usw_dma_async_tx_by_channel(lchip,0);
            if (CTC_E_NOT_INIT == ret)
            {
                break;
            }
        }
    }

    return;
}
STATIC int32
_sys_usw_dma_tcam_scan_func(uint8 lchip, uint16 chan, uint8 dmasel)
{
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    sys_dma_desc_info_t* p_desc_info = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 cur_index = 0;
    uint32 desc_done = 0;
    uint32 process_cnt = 0;
    uint64 phy_addr = 0;
    uint32* logic_addr = NULL;
    drv_ser_dma_tcam_param_t tcam_param;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[DMA] Tcam scan isr, lchip %d chan %d dmasel:%d\n", lchip, chan, dmasel);

    /* init check */
    SYS_DMA_INIT_CHECK(lchip);
    p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[chan];
    DMA_LOCK(p_dma_chan->p_mutex);
    p_base_desc = p_dma_chan->p_desc;
    p_desc_info = p_dma_chan->p_desc_info;
    if (!p_desc_info)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "desc info not init\n");
        DMA_UNLOCK(p_dma_chan->p_mutex);
        return CTC_E_NOT_INIT;
    }

    /*for tcam scan once*/
    {
        uint8  mode = 0;
        /*uint32 interval = 0;*/
        drv_ser_scan_info_t ecc_scan_info;

        sal_memset(&ecc_scan_info, 0, sizeof(drv_ser_scan_info_t));
        ecc_scan_info.type = DRV_SER_SCAN_TYPE_TCAM;
        drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE,&ecc_scan_info);
        mode =  ecc_scan_info.mode;
        /*interval =  ecc_scan_info.scan_interval;*/

        if(mode == 0)
        {
            sys_usw_dma_set_tcam_scan_mode(lchip, 2, 0);
        }
    }

    for (cur_index = p_dma_chan->current_index; cur_index < p_dma_chan->desc_depth; cur_index++)
    {
        p_desc = &(p_base_desc[cur_index].desc_info);
        desc_done = GetDsDescEncap(V, done_f, p_desc);

        if (0 == desc_done)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "No desc is not done, processed %d desc\n", process_cnt);
            break;
        }

        process_cnt++;

        COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr,             \
                            (GetDsDescEncap(V, memAddr_f, p_desc)<<4), phy_addr);

        logic_addr = g_dal_op.phy_to_logic(lchip, phy_addr);

        sal_memset(&tcam_param, 0, sizeof(tcam_param));

        GetDsDescEncap(A, timestamp_f, p_desc, &(tcam_param.time_stamp));
        tcam_param.mem_id = p_dma_chan->p_desc_info[cur_index].value0 >> 8;
        tcam_param.sub_mem_id = p_dma_chan->p_desc_info[cur_index].value0&0xFF;
        tcam_param.p_memory = logic_addr;
        tcam_param.entry_size = p_dma_chan->p_desc_info[cur_index].value1;
        drv_ser_set_cfg(lchip, DRV_SER_CFG_TYPE_DMA_RECOVER_TCAM, &tcam_param);

        _sys_usw_dma_clear_desc(lchip, p_dma_chan, cur_index, 1, NULL);

    }

    p_dma_chan->current_index = ((p_dma_chan->current_index + process_cnt) % (p_dma_chan->desc_depth));

    DMA_UNLOCK(p_dma_chan->p_mutex);

    return CTC_E_NONE;
}
STATIC void
_sys_usw_dma_tcam_scan_thread(void* param)
{
    int32 prio = 0;
    int32 ret = 0;
    uint8 dmasel = 0;
    uint8 act_chan = 0;
    sys_intr_type_t type;
    sys_dma_thread_t* p_thread_info = (sys_dma_thread_t*)param;
    uint8 index = 0;
    uint8 lchip = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    prio = p_thread_info->prio;
    lchip = p_thread_info->lchip;

    sal_task_set_priority(prio);

    while (1)
    {
        ret = sal_sem_take(p_thread_info->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }

        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        ret = sys_usw_chip_get_reset_hw_en(lchip);
        if (ret > 0)
        {
            /*in reset hw recover*/
            return;
        }

        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        type.intr = SYS_INTR_DMA+dmasel;

        for(index = 0; index < p_thread_info->chan_num; index++)
        {
            act_chan = p_thread_info->chan_id[index];

            /* check channel is enable or not */
            if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
            {
                continue;
            }

            dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;

            /* interrupt should be sync channel interrupt */
            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+act_chan;

            ret = _sys_usw_dma_tcam_scan_func(lchip, act_chan, dmasel);

            if (ret == CTC_E_NOT_INIT)
            {
                return;
            }

            /* release mask channel isr */
            sys_usw_interrupt_set_en(lchip, &type, TRUE);

        }

    }

    return;
}
/**
@brief DMA stats thread for stats channel
*/
STATIC void
_sys_usw_dma_stats_thread(void* param)
{
    int32 prio = 0;
    int32 ret = 0;
    uint8 dmasel = 0;
    uint8 act_chan = 0;
    sys_intr_type_t type;
    sys_dma_thread_t* p_thread_info = (sys_dma_thread_t*)param;
    uint8 index = 0;
    uint8 lchip = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    prio = p_thread_info->prio;
    lchip = p_thread_info->lchip;

    sal_task_set_priority(prio);

    while (1)
    {
        ret = sal_sem_take(p_thread_info->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }

        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        ret = sys_usw_chip_get_reset_hw_en(lchip);
        if (ret > 0)
        {
            /*in reset hw recover*/
            return;
        }

        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        type.intr = SYS_INTR_DMA+dmasel;

        /* scan all channel using same sync channel, process one by one */
        for(index = 0; index < p_thread_info->chan_num; index++)
        {
            act_chan = p_thread_info->chan_id[index];

            /* check channel is enable or not */
            if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
            {
                continue;
            }

            dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;

            /* interrupt should be sync channel interrupt */
            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+act_chan;

            ret = sys_usw_dma_stats_func(lchip, act_chan, dmasel);
            if (ret == CTC_E_NOT_INIT)
            {
                return;
            }

            /* release mask channel isr */
            sys_usw_interrupt_set_en(lchip, &type, TRUE);

        }

    }

    return;
}

void
_sys_usw_dma_wr_thread(void* param)
{
    int32 prio = 0;
    int32 ret = 0;
    uint8 dmasel = 0;
    uint8 act_chan = 0;
    sys_intr_type_t type;
    sys_dma_thread_t* p_thread_info = (sys_dma_thread_t*)param;
    uint8 index = 0;
    uint8 lchip = 0;

    prio = p_thread_info->prio;
    lchip = p_thread_info->lchip;

    sal_task_set_priority(prio);

    while (1)
    {
        ret = sal_sem_take(p_thread_info->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        type.intr = SYS_INTR_DMA+dmasel;

        /* scan all channel using same sync channel, process one by one */
        for(index = 0; index < p_thread_info->chan_num; index++)
        {
            act_chan = p_thread_info->chan_id[index];
            /* check channel is enable or not */
            if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
            {
                continue;
            }
            dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;
            /* interrupt should be sync channel interrupt */
            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+act_chan;
            ret = _sys_usw_dma_wr_func(lchip);
            if (ret == CTC_E_NOT_INIT)
            {
                return;
            }
            /* release mask channel isr */
            sys_usw_interrupt_set_en(lchip, &type, TRUE);
        }
    }

    return;
}

STATIC void
_sys_usw_dma_info_thread(void* param)
{
    int32 prio = 0;
    int32 ret = 0;
    sys_dma_thread_t* p_thread_info = (sys_dma_thread_t*)param;
    uint8 index = 0;
    uint8 dmasel = 0;
    sys_intr_type_t type;
    uint8 act_chan = 0;
    sys_dma_chan_t* p_dma_chan = NULL;
    /*DsDesc_m* p_desc = NULL;*/
    /*uint32 cur_index = 0;*/
    uint32 cmd = 0;
    DmaCtlIntrFunc_m intr_ctl;
    uint8 lchip = 0;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    prio = p_thread_info->prio;
    lchip = p_thread_info->lchip;

    sal_task_set_priority(prio);

    while (1)
    {
        ret = sal_sem_take(p_thread_info->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        ret = sys_usw_chip_get_reset_hw_en(lchip);
        if (ret > 0)
        {
            /*in reset hw recover*/
            return;
        }

        type.intr = SYS_INTR_DMA+dmasel;

        /* scan all channel using same sync channel, process one by one */
        for(index = 0; index < p_thread_info->chan_num; index++)
        {
            act_chan = p_thread_info->chan_id[index];

            /* check channel is enable or not */
            if (p_usw_dma_master[lchip]->dma_chan_info[act_chan].chan_en == 0)
            {
                continue;
            }

            dmasel = p_usw_dma_master[lchip]->dma_chan_info[act_chan].dmasel;

            p_dma_chan = &p_usw_dma_master[lchip]->dma_chan_info[act_chan];
            /*cur_index = p_dma_chan->current_index;*/
            /*p_desc = &(p_dma_chan->p_desc[cur_index].desc_info);*/

            /* interrupt should be sync channel interrupt */
            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+act_chan;

            ret = _sys_usw_dma_info_func(lchip, act_chan, dmasel);
            if (ret == CTC_E_NOT_INIT)
            {
                return;
            }

            /* release mask channel isr */
            sys_usw_interrupt_set_en(lchip, &type, TRUE);

            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)&(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)), sizeof(DsDesc_m));
            }
            if (GetDsDescEncap(V, done_f, &(p_dma_chan->p_desc[p_dma_chan->current_index].desc_info)))
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dma Trigger interrupt chan:%d \n", act_chan);
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
                if(!DRV_IS_DUET2(lchip))
                {
                    SetDmaCtlIntrFunc(V, dmaSupIntrVec_f, &intr_ctl, (1<<act_chan));
                }
                else
                {
                    SetDmaCtlIntrFunc(V, dmaIntrValidVec_f, &intr_ctl, (1<<act_chan));
                }
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                (DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
            }
        }
    }

    return;
}

/**
@brief DMA function interrupt serve routing
*/
int32
sys_usw_dma_isr_func(uint8 lchip, uint32 intr, void* p_data)
{
    uint8 chan = 0;
    uint8 sync_chan = 0;
    uint32* p_dma_func = (uint32*)p_data;
    sys_intr_type_t type;
    sys_dma_thread_t* p_thread_info = NULL;

    SYS_DMA_INIT_CHECK(lchip);
    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[DMA Ctl%d] Func ISR lchip %d intr %d sub_intr 0x%08X\n",
                    (intr==SYS_INTR_DMA)?0:1, lchip, intr, p_dma_func[0]);

    for (chan = 0; chan < SYS_DMA_MAX_CHAN_NUM; chan++)
    {
        if (CTC_BMP_ISSET(p_dma_func, chan))
        {
            type.intr = SYS_INTR_DMA + p_usw_dma_master[lchip]->dma_chan_info[chan].dmasel;
            type.sub_intr = SYS_INTR_SUB_DMA_FUNC_CHAN_0+chan;

            sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan].sync_chan;
            sys_usw_interrupt_set_en(lchip, &type, FALSE);

            p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, sync_chan);
            if (!p_thread_info)
            {
                /*means no need to create sync thread*/
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma init wrong please check!!chan:%d \n", chan);
                continue;
            }

            /* push sync sem */
            CTC_ERROR_RETURN(sal_sem_give(p_thread_info->p_sync_sem));
        }
    }

    return CTC_E_NONE;
}


#define _DMA_INIT_BEGIN

/**
@brief DMA sync mechanism init for function interrupt
*/
int32
_sys_usw_dma_sync_init(uint8 lchip, uintptr chan)
{
    int32 ret = 0;
    int32 prio = 0;
    sys_dma_thread_t* p_thread_info = NULL;
    char buffer[SAL_TASK_MAX_NAME_LEN];
    uint64 cpu_mask = 0;
    switch (GET_CHAN_TYPE(chan))
    {
        case DRV_DMA_PACKET_RX0_CHAN_ID:
        case DRV_DMA_PACKET_RX1_CHAN_ID:
        case DRV_DMA_PACKET_RX2_CHAN_ID:
        case DRV_DMA_PACKET_RX3_CHAN_ID:

            prio = p_usw_dma_master[lchip]->dma_thread_pri[chan];

            p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, chan);
            if (!p_thread_info)
            {
                /*means no need to create sync thread*/
                return CTC_E_NONE;
            }
            else
            {
    	        sal_sprintf(buffer, "ctcPktRx%d", (uint8)chan);

                cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
                ret = sys_usw_task_create(lchip, &p_thread_info->p_sync_task, buffer,
                                      SAL_DEF_TASK_STACK_SIZE, prio,SAL_TASK_TYPE_PACKET,cpu_mask, _sys_usw_dma_pkt_rx_thread, (void*)p_thread_info);
                if (ret < 0)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;

                }

                sal_memcpy(p_thread_info->desc, buffer, sizeof(buffer));
            }
            break;

        case DRV_DMA_PACKET_TX0_CHAN_ID:
        case DRV_DMA_PACKET_TX1_CHAN_ID:
        case DRV_DMA_PACKET_TX2_CHAN_ID:
        case DRV_DMA_PACKET_TX3_CHAN_ID:
        case DRV_DMA_TBL_RD_CHAN_ID:
            /*no need TODO */
            break;
        case DRV_DMA_TBL_WR_CHAN_ID:
            prio = p_usw_dma_master[lchip]->dma_thread_pri[chan];
            p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, chan);
            if (!p_thread_info)
            {
                /*means no need to create sync thread*/
                return CTC_E_NONE;
            }
            else
            {
                sal_sprintf(buffer, "Dmawr%d-%d", (uint8)(chan-SYS_DMA_TBL_WR_CHAN_ID), lchip);

                /* create dma learning thread */
                ret = sal_task_create(&p_thread_info->p_sync_task, buffer,
                                      SAL_DEF_TASK_STACK_SIZE, prio, _sys_usw_dma_wr_thread, (void*)p_thread_info);
                if (ret < 0)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        			return CTC_E_NOT_INIT;
                }

                sal_memcpy(p_thread_info->desc, buffer, sizeof(buffer));
            }
            break;

        case DRV_DMA_PORT_STATS_CHAN_ID:
        case DRV_DMA_FLOW_STATS_CHAN_ID:
        case DRV_DMA_TBL_RD1_CHAN_ID:
            prio = p_usw_dma_master[lchip]->dma_thread_pri[chan];
            p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, chan);
            if (!p_thread_info)
            {
                /*means no need to create sync thread*/
                return CTC_E_NONE;
            }
            else
            {
                sal_sprintf(buffer, "DmaStats%d", (uint8)(chan-SYS_DMA_PORT_STATS_CHAN_ID));

                cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
                /* create dma stats thread */
                ret = sys_usw_task_create(lchip,&p_thread_info->p_sync_task, buffer,
                                      SAL_DEF_TASK_STACK_SIZE, prio,SAL_TASK_TYPE_STATS,cpu_mask,_sys_usw_dma_stats_thread, (void*)p_thread_info);
                if (ret < 0)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }

                sal_memcpy(p_thread_info->desc, buffer, sizeof(buffer));
            }
            break;

     case DRV_DMA_REG_MAX_CHAN_ID:
            prio = p_usw_dma_master[lchip]->dma_thread_pri[chan];
            p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, chan);
            if (!p_thread_info)
            {
                /*means no need to create sync thread*/
                return CTC_E_NONE;
            }
            else
            {
                sal_sprintf(buffer, "TcamScan-%d", lchip);

                ret = sal_task_create(&p_thread_info->p_sync_task, buffer,
                                      SAL_DEF_TASK_STACK_SIZE, prio, _sys_usw_dma_tcam_scan_thread, (void*)p_thread_info);
                if (ret < 0)
                {
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
                    return CTC_E_NOT_INIT;
                }

                sal_memcpy(p_thread_info->desc, buffer, sizeof(buffer));
            }
            break;

    case DRV_DMA_HASHKEY_CHAN_ID:
            break;

    case DRV_DMA_LEARNING_CHAN_ID:
    case DRV_DMA_IPFIX_CHAN_ID:
    case DRV_DMA_SDC_CHAN_ID:
    case DRV_DMA_MONITOR_CHAN_ID:
        prio = p_usw_dma_master[lchip]->dma_thread_pri[chan];
        p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, chan);
        if (!p_thread_info)
        {
            /*means no need to create sync thread*/
            return CTC_E_NONE;
        }
        else
        {
            sal_sprintf(buffer, "DmaInfo%d", (uint8)(chan-SYS_DMA_LEARNING_CHAN_ID));

            /* create dma learning thread */
            cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
            ret = sys_usw_task_create(lchip,&p_thread_info->p_sync_task, buffer,
                                  SAL_DEF_TASK_STACK_SIZE, prio,SAL_TASK_TYPE_FDB,cpu_mask,_sys_usw_dma_info_thread, (void*)p_thread_info);
            if (ret < 0)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

            }

            sal_memcpy(p_thread_info->desc, buffer, sizeof(buffer));
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/**
@brief DMA init for pkt dma
*/
STATIC int32
_sys_usw_dma_pkt_init(uint8 lchip, sys_dma_chan_t* p_chan_info)
{
    uint32 desc_num = 0;
    sys_dma_desc_t* p_sys_desc_pad = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 phy_addr = 0;
    DmaStaticInfo_m static_info;
    DmaCtlTab_m  tab_ctl;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    void*  p_mem_addr = NULL;
    DmaWeightCfg_m dma_weight;
    uint32 valid_cnt = 0;
    uint8 ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(p_chan_info);

    desc_num  = p_chan_info->desc_depth;
    /* cfg desc num */
    p_sys_desc_pad = (sys_dma_desc_t*)g_dal_op.dma_alloc(lchip, (desc_num) * sizeof(sys_dma_desc_t), 0);
    if (NULL == p_sys_desc_pad)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_sys_desc_pad, 0, sizeof(sys_dma_desc_t)*desc_num);

    /* cfg per desc data only for packet rx*/
    if ((p_chan_info->data_size) && (p_chan_info->channel_id < SYS_DMA_PACKET_TX0_CHAN_ID))
    {
        for (desc_num = 0; desc_num < p_chan_info->desc_num; desc_num++)
        {
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);

             p_mem_addr = g_dal_op.dma_alloc(lchip, p_chan_info->data_size, 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

            }

            sal_memset(p_mem_addr, 0, p_chan_info->data_size);

            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
            SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
            SetDsDescEncap(V, cfgSize_f, p_desc, p_chan_info->cfg_size);
        }
    }

    /* channel mutex, only use fot DMA Tx */
#ifndef PACKET_TX_USE_SPINLOCK
    ret = sal_mutex_create(&(p_chan_info->p_mutex));
#else
    ret = sal_spinlock_create((sal_spinlock_t**)&(p_chan_info->p_mutex));
#endif
    if (ret || !(p_chan_info->p_mutex))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

    }

    /* cfg static infor for dmc channel:MemBase, ring depth */
    sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
    phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_pad);
    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));
    SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
    SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
    SetDmaStaticInfo(V, ringDepth_f, &static_info, p_chan_info->desc_depth);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));

    /* cfg DmaCtlTab for VldNum */
    if ((p_chan_info->channel_id == SYS_DMA_PACKET_TX0_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX1_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX2_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX3_CHAN_ID))
    {
        valid_cnt = 0;
    }
    else
    {
        valid_cnt = p_chan_info->desc_num;
    }

    sal_memset(&tab_ctl, 0, sizeof(DmaCtlTab_m));
    tbl_id = DmaCtlTab_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));
    SetDmaCtlTab(V, vldNum_f, &tab_ctl, valid_cnt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));

    /* cfg weight */
    sal_memset(&dma_weight, 0, sizeof(DmaWeightCfg_m));
    tbl_id = DmaWeightCfg_t + p_chan_info->dmasel;
    field_id = DmaWeightCfg_cfgChan0Weight_f + p_chan_info->channel_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));
    DRV_SET_FIELD_V(lchip, tbl_id, field_id, &dma_weight, (p_chan_info->weight));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));

    if ((p_chan_info->channel_id == SYS_DMA_PACKET_TX0_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX1_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX2_CHAN_ID)
        || (p_chan_info->channel_id == SYS_DMA_PACKET_TX3_CHAN_ID))
    {
        /*packet tx need allocate memory for record desc used state */
        p_chan_info->p_desc_used = (uint8*)mem_malloc(MEM_DMA_MODULE, p_chan_info->desc_num);
        if (NULL == p_chan_info->p_desc_used)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_chan_info->p_desc_used, 0, p_chan_info->desc_num);
    }

    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_desc = p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].mem_base = (uintptr)p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_desc_used = p_chan_info->p_desc_used;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_mutex = p_chan_info->p_mutex;

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_dma_tcam_scan_init(uint8 lchip, uint32 mem_id, uint16* p_desc_index, sys_dma_desc_t* p_sys_desc_pad,
                                                                            sys_dma_desc_info_t* p_sys_desc_info)
{
    uint32 cfg_addr = 0;
    uint32 cfg_size = 0;
    uint32 entry_num = 0;
    uint32 adjust_entry_size = 0;
    uint32 per_entry_size = 0;
    uint32 phy_addr = 0;
    void*  p_mem_addr = NULL;
    DsDesc_m* p_desc = NULL;

    uint8  max_sub_id = 0;
    uint8  sub_id = 0;

    CTC_ERROR_RETURN(drv_usw_ftm_get_tcam_memory_info(lchip, mem_id, &cfg_addr, &entry_num,  &per_entry_size));
    if(entry_num == 0)
    {
        return CTC_E_NONE;
    }
    _sys_usw_dma_rw_get_words_in_chip(lchip, per_entry_size, &adjust_entry_size);

    cfg_size = entry_num*adjust_entry_size*SYS_DMA_WORD_LEN;
    if(cfg_size > 0xFFFF)
    {
        max_sub_id = 2;
        cfg_size = cfg_size/2;
    }
    else
    {
        max_sub_id = 1;
    }

    for(sub_id = 0; sub_id < max_sub_id; sub_id++)
    {
        cfg_addr += (cfg_size*sub_id);
        p_sys_desc_info[(*p_desc_index)].value0 = mem_id << 8 | sub_id;
        p_sys_desc_info[(*p_desc_index)].value1 = adjust_entry_size;
        p_desc = (DsDesc_m*)&(p_sys_desc_pad[(*p_desc_index)].desc_info);

        p_mem_addr = g_dal_op.dma_alloc(lchip, cfg_size, 0);
        if (NULL == p_mem_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_mem_addr, 0, cfg_size);
        phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);

        SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
        SetDsDescEncap(V, cfgSize_f, p_desc, cfg_size);
        SetDsDescEncap(V, chipAddr_f, p_desc, cfg_addr);
        SetDsDescEncap(V, dataStruct_f, p_desc, per_entry_size);
        if ((*p_desc_index) == 0)
        {
            /*first desc should cfg pause*/
            SetDsDescEncap(V, pause_f, p_desc, 1);
        }
        (*p_desc_index)++;
    }

    return CTC_E_NONE;
}
/**
@brief DMA init for reg dma
*/
STATIC int32
_sys_usw_dma_reg_init(uint8 lchip, sys_dma_chan_t* p_chan_info)
{
    uint32 desc_num = 0;
    sys_dma_desc_t* p_sys_desc_pad = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 phy_addr = 0;
    DmaStaticInfo_m static_info;
    DmaCtlTab_m  tab_ctl;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    void*  p_mem_addr = NULL;
    DmaWeightCfg_m dma_weight;
    uint32 valid_cnt = 0;
    uint32 cfg_addr = 0;
    DmaPktStatsCfg_m stats_cfg;
    uint8 mac_id = 0;
    uint16 index = 0;
    uint8 mac_num = 0;
    uint8 mac_type = 0;
    uint32 tbl_idx = 0;
    int32 ret = 0;
    sys_dma_desc_info_t* p_sys_desc_info = NULL;

    CTC_PTR_VALID_CHECK(p_chan_info);

    desc_num  = p_chan_info->desc_depth;

    /* cfg desc num */
    p_sys_desc_pad = (sys_dma_desc_t*)g_dal_op.dma_alloc(lchip, (desc_num) * sizeof(sys_dma_desc_t), 0);
    if (NULL == p_sys_desc_pad)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }
    if(p_chan_info->channel_id == SYS_DMA_PORT_STATS_CHAN_ID || p_chan_info->channel_id == SYS_DMA_FLOW_STATS_CHAN_ID)
    {
        p_sys_desc_info = (sys_dma_desc_info_t*)mem_malloc(MEM_DMA_MODULE, (desc_num)*sizeof(sys_dma_desc_info_t));
        if (!p_sys_desc_info)
        {
            g_dal_op.dma_free(lchip, p_sys_desc_pad);
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_sys_desc_info, 0, sizeof(sys_dma_desc_info_t)*desc_num);
    }
    sal_memset(p_sys_desc_pad, 0, sizeof(sys_dma_desc_t)*desc_num);

    if (p_chan_info->channel_id == SYS_DMA_PORT_STATS_CHAN_ID)
    {
        mac_num = MCHIP_CAP(SYS_CAP_STATS_XQMAC_PORT_NUM)*MCHIP_CAP(SYS_CAP_STATS_XQMAC_RAM_NUM);
        for (mac_id = 0; mac_id < mac_num; mac_id++)
        {
            /* TODO all mac valid */
            p_sys_desc_info[mac_id].value0 = mac_id;
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[mac_id].desc_info);

            p_mem_addr = g_dal_op.dma_alloc(lchip, p_chan_info->data_size, 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;
            }

            sal_memset(p_mem_addr, 0, p_chan_info->data_size);

            _sys_usw_dma_get_mac_address(lchip, mac_type, mac_id,  &cfg_addr);
            /*use burst to read*/
            cfg_addr |= 0x1;
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
            SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
            SetDsDescEncap(V, cfgSize_f, p_desc, p_chan_info->cfg_size);
            SetDsDescEncap(V, chipAddr_f, p_desc, cfg_addr);
            SetDsDescEncap(V, dataStruct_f, p_desc, SYS_USW_DMA_STATS_WORD);
            if (mac_id == 0)
            {
                /*first desc should cfg pause*/
                SetDsDescEncap(V, pause_f, p_desc, 1);
            }
        }
    }
    else if (p_chan_info->channel_id == SYS_DMA_FLOW_STATS_CHAN_ID)
    {
#ifndef PACKET_TX_USE_SPINLOCK
        ret = sal_mutex_create(&(p_chan_info->p_mutex));
#else
        ret = sal_spinlock_create((sal_spinlock_t**)&(p_chan_info->p_mutex));
#endif
        if (ret || !(p_chan_info->p_mutex))
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;
        }

        for (index = 0; index < p_chan_info->desc_num; index++)
        {
            p_mem_addr = g_dal_op.dma_alloc(lchip, p_chan_info->data_size, 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
    			return CTC_E_NO_MEMORY;
            }

            tbl_idx = (index%MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM))*MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE);
            drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsStats_t, tbl_idx, &cfg_addr);
            /*use burst to read*/
            cfg_addr |= 0x1;
            p_sys_desc_info[index].value0 = index%MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM);
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[index].desc_info);

            sal_memset(p_mem_addr, 0, p_chan_info->data_size);
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
            SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
            SetDsDescEncap(V, cfgSize_f, p_desc, p_chan_info->cfg_size);
            SetDsDescEncap(V, chipAddr_f, p_desc, cfg_addr);
            SetDsDescEncap(V, dataStruct_f, p_desc, 2);
            SetDsDescEncap(V, pause_f, p_desc, ((index%MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM) == 0)?1:0));
        }
    }
    else if (p_chan_info->channel_id == SYS_DMA_REG_MAX_CHAN_ID)
    {
        uint16 mem_id = 0;
        p_sys_desc_info = (sys_dma_desc_info_t*)mem_malloc(MEM_DMA_MODULE, (desc_num)*sizeof(sys_dma_desc_info_t));
        if (!p_sys_desc_info)
        {
            g_dal_op.dma_free(lchip, p_sys_desc_pad);
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_sys_desc_info, 0, sizeof(sys_dma_desc_info_t)*desc_num);

        /*flow tcam*/
        for(mem_id = DRV_FTM_TCAM_KEY0; mem_id < DRV_FTM_TCAM_KEYM; mem_id++)
        {
            CTC_ERROR_RETURN(_sys_usw_dma_tcam_scan_init(lchip, mem_id, &index, p_sys_desc_pad, p_sys_desc_info));
        }
        for(mem_id = DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
        {
            CTC_ERROR_RETURN(_sys_usw_dma_tcam_scan_init(lchip, mem_id, &index, p_sys_desc_pad, p_sys_desc_info));
        }

        CTC_ERROR_RETURN(_sys_usw_dma_tcam_scan_init(lchip, DRV_FTM_QUEUE_TCAM, &index, p_sys_desc_pad, p_sys_desc_info));
        CTC_ERROR_RETURN(_sys_usw_dma_tcam_scan_init(lchip, DRV_FTM_CID_TCAM, &index, p_sys_desc_pad, p_sys_desc_info));
    }
    else if (p_chan_info->channel_id == SYS_DMA_TBL_RD1_CHAN_ID)
    {
        uint16 data_size = 0;
        uint8  dword = 0;
        for (index = 0; index < p_chan_info->desc_depth; index++)
        {
            if(0 == index%3)
            {
                data_size = sizeof(DsDot1AeDecryptGlobalStats_m);
                dword = sizeof(DsDot1AeDecryptGlobalStats_m)/4;
                drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsDot1AeDecryptGlobalStats_t, 0, &cfg_addr);
            }
            else if(1 == index%3)
            {
                data_size = sizeof(DsDot1AeEncryptStats_m)*256;
                dword = sizeof(DsDot1AeEncryptStats_m)/4;
                drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsDot1AeEncryptStats_t, 0, &cfg_addr);
            }
            else
            {
                data_size = sizeof(DsDot1AeDecryptStats_m)*1024;
                dword = sizeof(DsDot1AeDecryptStats_m)/4;
                drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsDot1AeDecryptStats_t, 0, &cfg_addr);
            }
            /*use burst to read*/
            /*cfg_addr |= 0x1;*/
            p_mem_addr = g_dal_op.dma_alloc(lchip, data_size, 0);
            if (NULL == p_mem_addr)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			    return CTC_E_NO_MEMORY;
            }

            sal_memset(p_mem_addr, 0, data_size);
            phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
            p_desc = (DsDesc_m*)&(p_sys_desc_pad[index].desc_info);

            SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
            SetDsDescEncap(V, cfgSize_f, p_desc, data_size);
            SetDsDescEncap(V, chipAddr_f, p_desc, cfg_addr);
            SetDsDescEncap(V, dataStruct_f, p_desc, dword);
            if ((index%3) == 0)
            {
                /*first desc should cfg pause*/
                SetDsDescEncap(V, pause_f, p_desc, 1);
            }
        }
    }
    else if (p_chan_info->channel_id == SYS_DMA_TBL_WR_CHAN_ID || p_chan_info->channel_id == SYS_DMA_TBL_RD_CHAN_ID)
    {
        /*for write channel, create mux*/
#ifndef PACKET_TX_USE_SPINLOCK
        ret = sal_mutex_create(&(p_chan_info->p_mutex));
#else
        ret = sal_spinlock_create((sal_spinlock_t**)&(p_chan_info->p_mutex));
#endif
        if (ret || !(p_chan_info->p_mutex))
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
			return CTC_E_NO_RESOURCE;

        }
    }

    /* cfg static infor for dmc channel:MemBase, ring depth */
    sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
    phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_pad);
    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));
    SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
    SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
    SetDmaStaticInfo(V, ringDepth_f, &static_info, p_chan_info->desc_depth);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));

    /* cfg DmaCtlTab for VldNum */
    valid_cnt = p_chan_info->desc_num;

    sal_memset(&tab_ctl, 0, sizeof(DmaCtlTab_m));
    tbl_id = DmaCtlTab_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));
    SetDmaCtlTab(V, vldNum_f, &tab_ctl, valid_cnt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));

    /* cfg weight */
    sal_memset(&dma_weight, 0, sizeof(DmaWeightCfg_m));
    tbl_id = DmaWeightCfg_t + p_chan_info->dmasel;
    field_id = DmaWeightCfg_cfgChan0Weight_f + p_chan_info->channel_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));
    DRV_SET_FIELD_V(lchip, tbl_id, field_id, &dma_weight, (p_chan_info->weight));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));

    /* cfg clear on read */
    tbl_id = DmaPktStatsCfg_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_cfg));
    SetDmaPktStatsCfg(V, clearOnRead_f, &stats_cfg, 1);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_cfg));

    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_desc = p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].mem_base = (uintptr)p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_mutex = p_chan_info->p_mutex;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_desc_info = p_sys_desc_info;
    return CTC_E_NONE;
}

/**
@brief DMA init for info dma
*/
STATIC int32
_sys_usw_dma_info_init(uint8 lchip, sys_dma_chan_t* p_chan_info)
{
    uint32 desc_num = 0;
    sys_dma_desc_t* p_sys_desc_pad = NULL;
    DsDesc_m* p_desc = NULL;
    uint32 phy_addr = 0;
    DmaStaticInfo_m static_info;
    DmaCtlTab_m  tab_ctl;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    void*  p_mem_addr = NULL;
    DmaWeightCfg_m dma_weight;
    uint32 valid_cnt = 0;

    CTC_PTR_VALID_CHECK(p_chan_info);

    desc_num  = p_chan_info->desc_depth;

    /* cfg desc num */
    p_sys_desc_pad = (sys_dma_desc_t*)g_dal_op.dma_alloc(lchip, (desc_num) * sizeof(sys_dma_desc_t), 0);
    if (NULL == p_sys_desc_pad)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_sys_desc_pad, 0, sizeof(sys_dma_desc_t)*desc_num);

    /* cfg per desc data */
    for (desc_num = 0; desc_num < p_chan_info->desc_num; desc_num++)
    {
        p_desc = (DsDesc_m*)&(p_sys_desc_pad[desc_num].desc_info);

        p_mem_addr = g_dal_op.dma_alloc(lchip, p_chan_info->data_size, 0);
        if (NULL == p_mem_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_mem_addr, 0, p_chan_info->data_size);

        phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_mem_addr);
        SetDsDescEncap(V, memAddr_f, p_desc, (phy_addr >> 4));
        SetDsDescEncap(V, cfgSize_f, p_desc, p_chan_info->cfg_size);
    }

    /* cfg static infor for dmc channel:MemBase, ring depth */
    sal_memset(&static_info, 0, sizeof(DmaStaticInfo_m));
    phy_addr = (uint32)g_dal_op.logic_to_phy(lchip, p_sys_desc_pad);
    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));
    SetDmaStaticInfo(V, highBase_f, &static_info, p_usw_dma_master[lchip]->dma_high_addr);
    SetDmaStaticInfo(V, ringBase_f, &static_info, (phy_addr >> 4));
    SetDmaStaticInfo(V, ringDepth_f, &static_info, p_chan_info->desc_depth);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));

    /* cfg DmaCtlTab for VldNum */
    valid_cnt = p_chan_info->desc_num;

    sal_memset(&tab_ctl, 0, sizeof(DmaCtlTab_m));
    tbl_id = DmaCtlTab_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));
    SetDmaCtlTab(V, vldNum_f, &tab_ctl, valid_cnt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &tab_ctl));

    /* cfg weight */
    sal_memset(&dma_weight, 0, sizeof(DmaWeightCfg_m));
    tbl_id = DmaWeightCfg_t + p_chan_info->dmasel;
    field_id = DmaWeightCfg_cfgChan0Weight_f + p_chan_info->channel_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));
    DRV_SET_FIELD_V(lchip, tbl_id, field_id, &dma_weight, (p_chan_info->weight));
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dma_weight));

    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_desc = p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].mem_base = (uintptr)p_sys_desc_pad;
    p_usw_dma_master[lchip]->dma_chan_info[p_chan_info->channel_id].p_mutex = p_chan_info->p_mutex;

    return CTC_E_NONE;
}

/**
@brief DMA common init
*/
STATIC int32
_sys_usw_dma_common_init(uint8 lchip, sys_dma_chan_t* p_chan_info)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    DmaStaticInfo_m static_info;
    uint32 chan_en = 0;

    switch(GET_CHAN_TYPE(p_chan_info->channel_id))
    {
    case DRV_DMA_PACKET_RX0_CHAN_ID:
    case DRV_DMA_PACKET_RX1_CHAN_ID:
    case DRV_DMA_PACKET_RX2_CHAN_ID:
    case DRV_DMA_PACKET_RX3_CHAN_ID:
    case DRV_DMA_PACKET_TX0_CHAN_ID:
    case DRV_DMA_PACKET_TX1_CHAN_ID:
    case DRV_DMA_PACKET_TX2_CHAN_ID:
    case DRV_DMA_PACKET_TX3_CHAN_ID:

        /*using tx1 channel as timer tx channel*/
        if (p_usw_dma_master[lchip]->pkt_tx_timer_en && (p_chan_info->channel_id == SYS_DMA_PKT_TX_TIMER_CHAN_ID))
        {
            /*enable auto mode for tx on timer*/
            tbl_id = DmaCtlAutoMode_t;
            cmd = DRV_IOR(tbl_id, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_en));
            CTC_BIT_SET(chan_en, p_chan_info->channel_id);
            cmd = DRV_IOW(tbl_id, DmaCtlAutoMode_dmaAutoMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_en));

            tbl_id = DmaMiscCfg_t;
            cmd = DRV_IOR(tbl_id, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_en));
            CTC_BIT_SET(chan_en, (SYS_DMA_PKT_TX_TIMER_CHAN_ID-SYS_DMA_PACKET_TX0_CHAN_ID));
            cmd = DRV_IOW(tbl_id, DmaMiscCfg_cfgDmaRdNullDis_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &chan_en));

            chan_en = 0;
            break;
        }
        CTC_ERROR_RETURN(_sys_usw_dma_pkt_init(lchip, p_chan_info));
        chan_en = 1;
        break;

    case DRV_DMA_TBL_WR_CHAN_ID:
    case DRV_DMA_TBL_RD_CHAN_ID:
    case DRV_DMA_PORT_STATS_CHAN_ID:
    case DRV_DMA_FLOW_STATS_CHAN_ID:
    case DRV_DMA_REG_MAX_CHAN_ID:
    case DRV_DMA_TBL_RD1_CHAN_ID:
    case DRV_DMA_TBL_RD2_CHAN_ID:
        CTC_ERROR_RETURN(_sys_usw_dma_reg_init(lchip, p_chan_info));
        chan_en = 1;
        break;

    case DRV_DMA_LEARNING_CHAN_ID:
    case DRV_DMA_HASHKEY_CHAN_ID:
    case DRV_DMA_IPFIX_CHAN_ID:
    case DRV_DMA_SDC_CHAN_ID:
    case DRV_DMA_MONITOR_CHAN_ID:
        CTC_ERROR_RETURN(_sys_usw_dma_info_init(lchip, p_chan_info));
        chan_en = 1;
        break;

    default:
        break;

    }

    tbl_id = DmaStaticInfo_t + p_chan_info->dmasel;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));
    SetDmaStaticInfo(V, chanEn_f, &static_info, chan_en);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_chan_info->channel_id, cmd, &static_info));

    return CTC_E_NONE;
}


/**
@brief DMA intr init
   packet rx: per EOP
   Packet tx: no need
   RegRd:  no need
   RegWr: no need
   port stats: desc cnt
   Info: timer + per desc
*/
STATIC int32
_sys_usw_dma_intr_init(uint8 lchip, uint8 dma_idx, ctc_dma_global_cfg_t* p_global_cfg)
{
    DmaPktIntrEnCfg_m pkt_intr;
    DmaRegIntrEnCfg_m reg_intr;
    DmaRegRdIntrCntCfg_m reg_intr_cnt;
    DmaInfoIntrEnCfg_m info_intr;
    DmaInfoIntrCntCfg_m intr_cnt;
    DmaPktIntrCntCfg_m  pkt_intr_cnt;
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    uint32 filed_value;
    uint8 mac_num = 0;

    /* cfg intr for packet rx, only using slice0 dmactl for dma packet, slice1 bufretr forwarding to dmactl0  */
    sal_memset(&pkt_intr, 0, sizeof(DmaPktIntrEnCfg_m));
    tbl_id = DmaPktIntrEnCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_intr));
    SetDmaPktIntrEnCfg(V, cfgPktRx0EopIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx1EopIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx2EopIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx3EopIntrEn_f, &pkt_intr, 1);

    SetDmaPktIntrEnCfg(V, cfgPktRx0DmaIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx1DmaIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx2DmaIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktRx3DmaIntrEn_f, &pkt_intr, 1);
    SetDmaPktIntrEnCfg(V, cfgPktTx0EopIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx0DmaIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx1EopIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx1DmaIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx2DmaIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx2EopIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx2IntrTimerEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx3DmaIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx3EopIntrEn_f, &pkt_intr, 0);
    SetDmaPktIntrEnCfg(V, cfgPktTx3IntrTimerEn_f, &pkt_intr, 0);

    filed_value = 0;
    SetDmaPktIntrEnCfg(V, cfgPktRx0IntrTimerEn_f, &pkt_intr, filed_value);
    SetDmaPktIntrEnCfg(V, cfgPktRx1IntrTimerEn_f, &pkt_intr, filed_value);
    SetDmaPktIntrEnCfg(V, cfgPktRx2IntrTimerEn_f, &pkt_intr, filed_value);
    SetDmaPktIntrEnCfg(V, cfgPktRx3IntrTimerEn_f, &pkt_intr, filed_value);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_intr));

    /*cfg per 128 packts trigger interrupt*/
    tbl_id = DmaPktIntrCntCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    sal_memset(&pkt_intr_cnt, 0, sizeof(pkt_intr_cnt));
    SetDmaPktIntrCntCfg(V, cfgPktRx0IntrCnt_f, &pkt_intr_cnt,SYS_USW_DMA_PACKETS_PER_INTR);
    SetDmaPktIntrCntCfg(V, cfgPktRx1IntrCnt_f, &pkt_intr_cnt,SYS_USW_DMA_PACKETS_PER_INTR);
    SetDmaPktIntrCntCfg(V, cfgPktRx2IntrCnt_f, &pkt_intr_cnt,SYS_USW_DMA_PACKETS_PER_INTR);
    SetDmaPktIntrCntCfg(V, cfgPktRx3IntrCnt_f, &pkt_intr_cnt,SYS_USW_DMA_PACKETS_PER_INTR);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_intr_cnt));

    /* cfg intr for Reg Dma(only port stats and dma pkt stats ) */
    sal_memset(&reg_intr, 0, sizeof(DmaRegIntrEnCfg_m));
    sal_memset(&reg_intr_cnt, 0, sizeof(reg_intr_cnt));
    tbl_id = DmaRegIntrEnCfg_t + dma_idx;
    SetDmaRegIntrEnCfg(V, cfgRegRd0DescIntrEn_f, &reg_intr, 0);
    SetDmaRegIntrEnCfg(V, cfgRegRd0DmaIntrEn_f, &reg_intr, 0);
    SetDmaRegIntrEnCfg(V, cfgRegRd1DmaIntrEn_f, &reg_intr, 1);
    SetDmaRegIntrEnCfg(V, cfgRegRd2DmaIntrEn_f, &reg_intr, 1);
    SetDmaRegIntrEnCfg(V, cfgRegRd3DmaIntrEn_f, &reg_intr, 1);
    SetDmaRegIntrEnCfg(V, cfgRegRd4DmaIntrEn_f, &reg_intr, 1);
    SetDmaRegIntrEnCfg(V, cfgRegRd5DmaIntrEn_f, &reg_intr, 1);
    SetDmaRegIntrEnCfg(V, cfgRegWrDmaIntrEn_f, &reg_intr, 0);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &reg_intr));

    mac_num = MCHIP_CAP(SYS_CAP_STATS_XQMAC_PORT_NUM)*MCHIP_CAP(SYS_CAP_STATS_XQMAC_RAM_NUM);
    tbl_id = DmaRegRdIntrCntCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &reg_intr_cnt));
    SetDmaRegRdIntrCntCfg(V, cfgRegRd1IntrCnt_f, &reg_intr_cnt, mac_num);
    SetDmaRegRdIntrCntCfg(V, cfgRegRd2IntrCnt_f, &reg_intr_cnt, MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM)*10);
    SetDmaRegRdIntrCntCfg(V, cfgRegRd3IntrCnt_f, &reg_intr_cnt, SYS_USW_DMA_TCAM_SCAN_NUM);
    SetDmaRegRdIntrCntCfg(V, cfgRegRd4IntrCnt_f, &reg_intr_cnt, 20);
    SetDmaRegRdIntrCntCfg(V, cfgRegRd5IntrCnt_f, &reg_intr_cnt, 20);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &reg_intr_cnt));

    /* cfg intr for Info Dma */
    sal_memset(&info_intr, 0, sizeof(DmaInfoIntrEnCfg_m));
    tbl_id = DmaInfoIntrEnCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info_intr));
    SetDmaInfoIntrEnCfg(V, cfgInfo0DescIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo1DescIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo2DescIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo3DescIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo4DescIntrEn_f, &info_intr, 0);

    SetDmaInfoIntrEnCfg(V, cfgInfo0DmaIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo1DmaIntrEn_f, &info_intr, 0);
    SetDmaInfoIntrEnCfg(V, cfgInfo2DmaIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo3DmaIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo4DmaIntrEn_f, &info_intr, 1);

    SetDmaInfoIntrEnCfg(V, cfgInfo0TimerIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo1TimerIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo2TimerIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo3TimerIntrEn_f, &info_intr, 1);
    SetDmaInfoIntrEnCfg(V, cfgInfo4TimerIntrEn_f, &info_intr, 1);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info_intr));

    tbl_id = DmaInfoIntrCntCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intr_cnt));
    SetDmaInfoIntrCntCfg(V, cfgInfo4IntrCnt_f, &intr_cnt, 10);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intr_cnt));

    return CTC_E_NONE;
}

/**
@brief DMA timer init only usefull for InfoDma and port stats
  Info0:learning/aging, Info1:HashKey, Info2:Ipfix
  Info3:SDC,
  Info4:Monitor:Do not using timeout interrupt, desc full interrupt
  port stats: reg1
  table read: reg0

*/
STATIC int32
_sys_usw_dma_timer_init(uint8 lchip, uint8 dma_idx)
{
    DmaInfoTimerCfg_m global_timer_ctl;
    DmaInfo0TimerCfg_m info0_timer;
    DmaInfo1TimerCfg_m info1_timer;
    DmaInfo4TimerCfg_m info4_timer;
    DmaInfo2TimerCfg_m info2_timer;
    DmaRegRd1TrigCfg_m trigger1_timer;
    DmaRegRd1TrigCfg_m trigger2_timer;
    DmaRegRd1TrigCfg_m trigger3_timer;
    DmaRegTrigEnCfg_m trigger_ctl;
    DmaInfoThrdCfg_m thrd_cfg;
    DmaPktIntrTimerCfg_m pkt_timer;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint64 timer = 0;
    uint32 timer_v[2] = {0};

    if (dma_idx >= SYS_DMA_CTL_USED_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&global_timer_ctl, 0, sizeof(global_timer_ctl));
    sal_memset(&info0_timer, 0, sizeof(info0_timer));
    sal_memset(&info1_timer, 0, sizeof(info1_timer));
    sal_memset(&info4_timer, 0, sizeof(info4_timer));
    sal_memset(&trigger1_timer, 0, sizeof(trigger1_timer));
    sal_memset(&trigger_ctl, 0, sizeof(trigger_ctl));
    sal_memset(&pkt_timer, 0, sizeof(pkt_timer));

    cmd = DRV_IOR(DmaInfoThrdCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &thrd_cfg));

    SetDmaInfoThrdCfg(V, cfgInfo0DmaThrd_f, &thrd_cfg, 1); /*learning max is 8 */
    SetDmaInfoThrdCfg(V, cfgInfo2DmaThrd_f, &thrd_cfg, 1); /*ipfix max is 4 */

    SetDmaInfoThrdCfg(V, cfgInfo1DmaThrd_f, &thrd_cfg, 1);
    SetDmaInfoThrdCfg(V, cfgInfo3DmaThrd_f, &thrd_cfg, 1);
    SetDmaInfoThrdCfg(V, cfgInfo4DmaThrd_f, &thrd_cfg, 1);
    cmd = DRV_IOW(DmaInfoThrdCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &thrd_cfg));

    /* for packet rx ,100ms trigger interrupt, cfgPktRxIntrTimerCnt 1 means 2ns*/
    tbl_id = DmaPktIntrTimerCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_timer));
    timer = (uint64)(1000*1000*1000/SYS_USW_DMA_PACKETS_PER_INTR/2);
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    SetDmaPktIntrTimerCfg(A, cfgPktRxIntrTimerCnt_f, &pkt_timer, timer_v);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pkt_timer));

    /* for hashdump dump 1 entry need 16 cycles, cfg timer out is 72k hashram
         16*72*1024*1.67ns = 2ms */
    timer = (uint64)2000000/DOWN_FRE_RATE; /*2ms*/
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    tbl_id = DmaInfo1TimerCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info1_timer));
    SetDmaInfo1TimerCfg(A, cfgInfo1TimerNs_f, &info1_timer, timer_v);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info1_timer));

    /*for monitor set 1mS*/
    timer = (uint64)1000000/DOWN_FRE_RATE; /*1ms*/
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    tbl_id = DmaInfo4TimerCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info4_timer));
    SetDmaInfo4TimerCfg(A, cfgInfo4TimerNs_f, &info4_timer, timer_v);
    tbl_id = DmaInfo4TimerCfg_t + dma_idx;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info4_timer));

    /*learning using default*/

    /*for ipfix set 1s*/
    timer = (uint64)1000000000/DOWN_FRE_RATE; /*1s*/
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    tbl_id = DmaInfo2TimerCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info2_timer));
    SetDmaInfo2TimerCfg(A, cfgInfo2TimerNs_f, &info2_timer, timer_v);
    tbl_id = DmaInfo2TimerCfg_t + dma_idx;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &info2_timer));

    tbl_id = DmaInfoTimerCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &global_timer_ctl));

    SetDmaInfoTimerCfg(V, cfgInfo0DescTimerChk_f, &global_timer_ctl, 1);  /*learning*/
    SetDmaInfoTimerCfg(V, cfgInfo0ReqTimerChk_f, &global_timer_ctl, 0);
    SetDmaInfoTimerCfg(V, cfgInfo0TimerEnd_f, &global_timer_ctl, 1);

    SetDmaInfoTimerCfg(V, cfgInfo2DescTimerChk_f, &global_timer_ctl, 1);  /*ipfix*/
    SetDmaInfoTimerCfg(V, cfgInfo2ReqTimerChk_f, &global_timer_ctl, 0);
    SetDmaInfoTimerCfg(V, cfgInfo2TimerEnd_f, &global_timer_ctl, 1);

    SetDmaInfoTimerCfg(V, cfgInfo1DescTimerChk_f, &global_timer_ctl, 1); /*hashdump*/
    SetDmaInfoTimerCfg(V, cfgInfo1ReqTimerChk_f, &global_timer_ctl, 0);
    SetDmaInfoTimerCfg(V, cfgInfo1TimerEnd_f, &global_timer_ctl, 1);

    SetDmaInfoTimerCfg(V, cfgInfo3DescTimerChk_f, &global_timer_ctl, 1);  /*sdc*/
    SetDmaInfoTimerCfg(V, cfgInfo3ReqTimerChk_f, &global_timer_ctl, 0);
    SetDmaInfoTimerCfg(V, cfgInfo3TimerEnd_f, &global_timer_ctl, 1);

    SetDmaInfoTimerCfg(V, cfgInfo4DescTimerChk_f, &global_timer_ctl, 1);/*monitor*/
    SetDmaInfoTimerCfg(V, cfgInfo4ReqTimerChk_f, &global_timer_ctl, 0);
    SetDmaInfoTimerCfg(V, cfgInfo4TimerEnd_f, &global_timer_ctl, 1);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &global_timer_ctl));

    /* cfg port stats trigger function */
    timer = (uint64)1*60*1000000000/DOWN_FRE_RATE; /*1min*/
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    tbl_id = DmaRegRd1TrigCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));
    SetDmaRegRd1TrigCfg(A, cfgRegRd1TrigNs_f, &trigger1_timer, timer_v);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger1_timer));

    /* cfg dma flow stats trigger function */
    timer = (uint64)1*1000000000/DOWN_FRE_RATE; /*1s*/
    timer_v[0] = timer&0xFFFFFFFF;
    timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
    tbl_id = DmaRegRd2TrigCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger2_timer));
    SetDmaRegRd2TrigCfg(A, cfgRegRd2TrigNs_f, &trigger2_timer, timer_v);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger2_timer));

    if (DRV_IS_TSINGMA(lchip))
    {
        /* cfg dma dot1ae stats trigger function */
        timer = (uint64)60*1000*1000*1000/DOWN_FRE_RATE;
        timer_v[0] = timer&0xFFFFFFFF;
        timer_v[1] = (timer >> 32) & 0xFFFFFFFF;
        tbl_id = DmaRegRd4TrigCfg_t + dma_idx;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger3_timer));
        SetDmaRegRd3TrigCfg(A, cfgRegRd3TrigNs_f, &trigger3_timer, timer_v);
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger3_timer));
    }

    tbl_id = DmaRegTrigEnCfg_t + dma_idx;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));
    SetDmaRegTrigEnCfg(V, cfgRegRd1TrigEn_f, &trigger_ctl, 0);    /*set by stats module*/
    SetDmaRegTrigEnCfg(V, cfgRegRd2TrigEn_f, &trigger_ctl, 1);
    SetDmaRegTrigEnCfg(V, cfgRegRd3TrigEn_f, &trigger_ctl, 0);
    SetDmaRegTrigEnCfg(V, cfgRegRd4TrigEn_f, &trigger_ctl, 1);
    SetDmaRegTrigEnCfg(V, cfgRegRd5TrigEn_f, &trigger_ctl, 1);
    SetDmaRegTrigEnCfg(V, cfgRegWrTrigEn_f, &trigger_ctl, 0);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &trigger_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_crc_init(uint8 lchip, uint8 dma_idx)
{
    uint32 tbl_id = 0;
    uint32 cmd = 0;
    DmaPktRxCrcCfg_m rx_crc;
    DmaPktTxCrcCfg_m tx_crc;

    sal_memset(&rx_crc, 0, sizeof(DmaPktRxCrcCfg_m));
    sal_memset(&tx_crc, 0, sizeof(DmaPktTxCrcCfg_m));

    SetDmaPktRxCrcCfg(V,cfgPktRx0CrcValid_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx0CrcPadEn_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx1CrcValid_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx1CrcPadEn_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx2CrcValid_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx2CrcPadEn_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx3CrcValid_f, &rx_crc, 0);
    SetDmaPktRxCrcCfg(V,cfgPktRx3CrcPadEn_f, &rx_crc, 0);

    SetDmaPktTxCrcCfg(V, cfgPktTxCrcValid_f, &tx_crc, 0);
    SetDmaPktTxCrcCfg(V, cfgPktTxCrcChkEn_f, &tx_crc, 0);
    SetDmaPktTxCrcCfg(V, cfgPktTxCrcPadEn_f, &tx_crc, 1);

    tbl_id = DmaPktRxCrcCfg_t + dma_idx;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_crc));

    tbl_id = DmaPktTxCrcCfg_t + dma_idx;
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_crc));

    return CTC_E_NONE;

}

/**
@brief Get Dma channel config
*/
STATIC int32
_sys_usw_dma_get_chan_cfg(uint8 lchip, uint8 chan_id, ctc_dma_global_cfg_t* ctc_cfg, sys_dma_chan_t* sys_cfg)
{
    uint32 desc_size = 0;
    uint32 desc_num = 0;
    uint8 mac_num = 0;
    ctc_dma_chan_cfg_t* tmp_chan_cfg = NULL;

    switch(GET_CHAN_TYPE(chan_id))
    {
        case DRV_DMA_PACKET_RX0_CHAN_ID:
        case DRV_DMA_PACKET_RX1_CHAN_ID:
        case DRV_DMA_PACKET_RX2_CHAN_ID:
        case DRV_DMA_PACKET_RX3_CHAN_ID:
            /* dma pcket rx using 256bytes as one block for one transfer */
            desc_size = (ctc_cfg->pkt_rx[chan_id].data < 256)?256:(ctc_cfg->pkt_rx[chan_id].data);
            desc_num = (ctc_cfg->pkt_rx[chan_id].desc_num)?(ctc_cfg->pkt_rx[chan_id].desc_num):64;
            desc_num = (desc_num > SYS_DMA_MAX_PACKET_RX_DESC_NUM)?SYS_DMA_MAX_PACKET_RX_DESC_NUM:desc_num;

            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_PACKET_RX0_CHAN_ID+chan_id;
            sys_cfg->current_index = 0;
            sys_cfg->pkt_knet_en = ctc_cfg->pkt_rx[chan_id].pkt_knet_en;
            sys_cfg->cfg_size = desc_size;
            sys_cfg->data_size = desc_size;
            sys_cfg->desc_depth = desc_num;
            sys_cfg->desc_num = desc_num;
            sys_cfg->func_type = CTC_DMA_FUNC_PACKET_RX;
            sys_cfg->dmasel = 0;  /*only support dmactl0 for packet */
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            /* for 4 dma rx channel, channel 0 is high priority */
            if (chan_id == 0)
            {
                sys_cfg->weight = SYS_DMA_HIGH_WEIRGH + 2;
            }
            else
            {
                sys_cfg->weight = SYS_DMA_HIGH_WEIRGH;
            }

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[chan_id], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_PACKET_TX0_CHAN_ID:
        case DRV_DMA_PACKET_TX1_CHAN_ID:
        case DRV_DMA_PACKET_TX2_CHAN_ID:
        case DRV_DMA_PACKET_TX3_CHAN_ID:

            if (p_usw_dma_master[lchip]->pkt_tx_timer_en && (chan_id == SYS_DMA_PKT_TX_TIMER_CHAN_ID))
            {
                sys_cfg->channel_id = chan_id;
                break;
            }

            tmp_chan_cfg = ctc_cfg->pkt_tx_ext[GET_CHAN_TYPE(chan_id) - DRV_DMA_PACKET_TX0_CHAN_ID].desc_num ?
                (&ctc_cfg->pkt_tx_ext[GET_CHAN_TYPE(chan_id) - DRV_DMA_PACKET_TX0_CHAN_ID]) : (&ctc_cfg->pkt_tx);
            desc_num = (tmp_chan_cfg->desc_num)?(tmp_chan_cfg->desc_num):8;
            desc_num = (desc_num > SYS_DMA_MAX_PACKET_TX_DESC_NUM)?SYS_DMA_MAX_PACKET_TX_DESC_NUM:desc_num;

            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = chan_id;
            sys_cfg->current_index = 0;
            sys_cfg->pkt_knet_en = tmp_chan_cfg->pkt_knet_en;
            sys_cfg->cfg_size = SYS_DMA_MTU;
            sys_cfg->data_size = SYS_DMA_MTU;
            sys_cfg->desc_depth = desc_num;
            sys_cfg->desc_num = desc_num;
            sys_cfg->func_type = CTC_DMA_FUNC_PACKET_TX;
            sys_cfg->dmasel = 0;  /*only support dmactl0 for packet */
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            /* for 2 dma tx channel, channel tx0 is high priority */
            if (chan_id == SYS_DMA_PACKET_TX0_CHAN_ID)
            {
                sys_cfg->weight = SYS_DMA_HIGH_WEIRGH + 2;
            }
            else
            {
                sys_cfg->weight = SYS_DMA_HIGH_WEIRGH;
            }

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[chan_id], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_TBL_RD_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_TBL_RD_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 0;
            sys_cfg->data_size = 0;
            sys_cfg->desc_depth = 100;
            sys_cfg->desc_num = 0;
            sys_cfg->func_type = CTC_DMA_FUNC_TABLE_R;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_RD_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_TBL_WR_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_TBL_WR_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 0;
            sys_cfg->data_size = 0;
            sys_cfg->desc_depth = 100;
            sys_cfg->desc_num = 0;
            sys_cfg->func_type = CTC_DMA_FUNC_TABLE_W;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_WR_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_PORT_STATS_CHAN_ID:
            mac_num = MCHIP_CAP(SYS_CAP_STATS_XQMAC_PORT_NUM)*MCHIP_CAP(SYS_CAP_STATS_XQMAC_RAM_NUM);
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_PORT_STATS_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 40*4*4;
            sys_cfg->data_size = 40*4*4;
            sys_cfg->desc_depth = mac_num;
            sys_cfg->desc_num = mac_num;
            sys_cfg->func_type = CTC_DMA_FUNC_STATS;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_PORT_STATS_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_LEARNING_CHAN_ID:
            desc_num = (ctc_cfg->learning.desc_num)?(ctc_cfg->learning.desc_num):64;
            desc_num = (desc_num > SYS_DMA_MAX_LEARNING_DESC_NUM)?SYS_DMA_MAX_LEARNING_DESC_NUM:desc_num;

            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_LEARNING_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 64;
            /* need confirm later for performance */
            sys_cfg->data_size = 64*sizeof(sys_dma_learning_info_t);
            sys_cfg->desc_depth = desc_num;
            sys_cfg->desc_num = desc_num;
            sys_cfg->func_type = CTC_DMA_FUNC_HW_LEARNING;
            sys_cfg->dmasel = ctc_cfg->learning.dmasel;
            sys_cfg->weight = SYS_DMA_MID_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_LEARNING_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_IPFIX_CHAN_ID:
            desc_num = (ctc_cfg->ipfix.desc_num)?(ctc_cfg->ipfix.desc_num):64;
            desc_num = (desc_num > SYS_DMA_MAX_IPFIX_DESC_NUM)?SYS_DMA_MAX_IPFIX_DESC_NUM:desc_num;

            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_IPFIX_CHAN_ID;

            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 64;
            sys_cfg->data_size = 64*sizeof(DmaToCpuIpfixAccFifo_m);
            sys_cfg->desc_depth = desc_num;
            sys_cfg->desc_num = desc_num;
            sys_cfg->func_type = CTC_DMA_FUNC_IPFIX;
            sys_cfg->dmasel = ctc_cfg->ipfix.dmasel;
            sys_cfg->weight = SYS_DMA_MID_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_IPFIX_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_SDC_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_SDC_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 256;
            sys_cfg->data_size = 256*sizeof(DmaToCpuSdcFifo_m);
            sys_cfg->desc_depth = 64;
            sys_cfg->desc_num = 64;
            sys_cfg->func_type = CTC_DMA_FUNC_SDC;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_MID_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_SDC_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_MONITOR_CHAN_ID:
            /*for monitor function per monitor interval process 128 entries, Dma allocate 1k entry data memory,
                So entry desc consume time is :1024/128*interval
             */
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_MONITOR_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 1024;
            sys_cfg->data_size = 1024*sizeof(DmaToCpuActMonIrmFifo_m);
            sys_cfg->desc_depth = 64;
            sys_cfg->desc_num = 64;
            sys_cfg->func_type = CTC_DMA_FUNC_MONITOR;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_MONITOR_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_HASHKEY_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_HASHKEY_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 256;   /* for optimal should eq dump threshold */
            sys_cfg->data_size = 256*sizeof(DmaFibDumpFifo_m);
            sys_cfg->desc_depth = 64;
            sys_cfg->desc_num = 64;
            sys_cfg->func_type = 0;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_MID_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_HASHKEY_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        /*used for sync flow stats, using 4 desc, every desc sync Max Dsstats/4 entry num*/
        case DRV_DMA_FLOW_STATS_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_FLOW_STATS_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE)*sizeof(DsStats_m);  /*Notice:for reg dma, size must eq ds(2 n)*entry num */
            sys_cfg->data_size = MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE)*sizeof(DsStats_m);
            sys_cfg->desc_depth = MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM)*10;
            sys_cfg->desc_num = MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM)*10;
            sys_cfg->func_type = 0;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_FLOW_STATS_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_REG_MAX_CHAN_ID:
            sys_cfg->chan_en = drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE, NULL);
            sys_cfg->channel_id = SYS_DMA_REG_MAX_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 0;  /*Notice:should get by ftm module when used */
            sys_cfg->data_size = 0;
            sys_cfg->desc_depth = SYS_USW_DMA_TCAM_SCAN_NUM;
            sys_cfg->desc_num = SYS_USW_DMA_TCAM_SCAN_NUM;
            sys_cfg->func_type = 0;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_REG_MAX_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        case DRV_DMA_TBL_RD1_CHAN_ID:
            sys_cfg->chan_en = 1;
            sys_cfg->channel_id = SYS_DMA_TBL_RD1_CHAN_ID;
            sys_cfg->current_index = 0;
            sys_cfg->cfg_size = 0;  /*Using for dot1ae stats in TM */
            sys_cfg->data_size = 0;
            sys_cfg->desc_depth = 3;
            sys_cfg->desc_num = 3;
            sys_cfg->func_type = 0;
            sys_cfg->dmasel = 0;
            sys_cfg->weight = SYS_DMA_LOW_WEIGHT;
            sys_cfg->sync_chan = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_chan;
            sys_cfg->sync_en = p_usw_dma_master[lchip]->dma_chan_info[chan_id].sync_en;

            sal_memcpy(&p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_TBL_RD1_CHAN_ID], sys_cfg, sizeof(sys_dma_chan_t));
            break;

        default:
            return CTC_E_INVALID_PARAM;

    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_dma_tx_func_init(uint8 lchip, ctc_dma_chan_cfg_t pkt_tx_cfg)
{
    int32 ret = CTC_E_NONE;
    uint32 idx = 0;
    void*  p_mem_addr = NULL;
    uint64 cpu_mask = 0;

    /*1. alloc tx pkt mem pool*/
    for(;idx < SYS_DMA_TX_PKT_MEM_NUM; idx++)
    {
        p_mem_addr = g_dal_op.dma_alloc(lchip, SYS_DMA_TX_PKT_MEM_SIZE, 0);
        if(NULL == p_mem_addr)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;
        }
        p_usw_dma_master[lchip]->tx_mem_pool[idx] = (uintptr)p_mem_addr;
    }

    ret = sal_spinlock_create(&(p_usw_dma_master[lchip]->p_tx_mutex));

    if (ret || !(p_usw_dma_master[lchip]->p_tx_mutex))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
		return CTC_E_NO_RESOURCE;

    }

    /*2. init hash for tx pkt mem */
    p_usw_dma_master[lchip]->dma_tx_mem_entry = ctc_hash_create(SYS_DMA_TX_PKT_MEM_NUM/SYS_DMA_TX_MEM_BLOCK_SIZE,
                                    SYS_DMA_TX_MEM_BLOCK_SIZE,
                                    (hash_key_fn) _sys_usw_dma_tx_mem_key,
                                    (hash_cmp_fn) _sys_usw_dma_tx_mem_compare);

    /*3 create async tx thread*/
    ret = sal_sem_create(&p_usw_dma_master[lchip]->tx_sem, 0);
    if (ret < 0)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
		return CTC_E_NOT_INIT;

    }

    cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
    ret = sys_usw_task_create(lchip,&p_usw_dma_master[lchip]->p_async_tx_task, "dma_async_tx",
                          SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF,SAL_TASK_TYPE_PACKET,cpu_mask, _sys_usw_dma_pkt_tx_thread, (void*)(uintptr)lchip);
    if (ret < 0)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    return ret;
}
int32
_sys_usw_dma_init_thread(uint8 lchip, uint8 start_chan, uint8 cur_chan, uint16 prio)
{
    uint8 temp_idx = 0;
    uint8 need_merge = 0;
    sys_dma_thread_t* p_thread_info = NULL;
    int32 ret = 0;

    for (temp_idx = start_chan; temp_idx < cur_chan; temp_idx++)
    {
        if (p_usw_dma_master[lchip]->dma_thread_pri[temp_idx] == prio)
        {
            need_merge = 1;
            break;
        }
    }

    if (need_merge)
    {
        p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, temp_idx);
        if (!p_thread_info)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

        }
        p_thread_info->lchip = lchip;
        p_thread_info->chan_num++;
        p_thread_info->chan_id[p_thread_info->chan_num-1] = cur_chan;
        p_usw_dma_master[lchip]->dma_chan_info[cur_chan].sync_chan = temp_idx;
        p_usw_dma_master[lchip]->dma_chan_info[cur_chan].sync_en = 1;
    }
    else
    {
        p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, cur_chan);
        if (p_thread_info)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

        }

        /*create new thread info*/
        p_thread_info = (sys_dma_thread_t*)mem_malloc(MEM_DMA_MODULE, sizeof(sys_dma_thread_t));
        if (!p_thread_info)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_thread_info, 0, sizeof(sys_dma_thread_t));

        p_thread_info->chan_num = 1;
        p_thread_info->chan_id[0] = cur_chan;
        p_thread_info->prio = prio;
        p_thread_info->lchip = lchip;

        ret = sal_sem_create(&p_thread_info->p_sync_sem, 0);
        if (ret < 0)
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

        }

        ctc_vector_add(p_usw_dma_master[lchip]->p_thread_vector, cur_chan, (void*)p_thread_info);

        p_usw_dma_master[lchip]->dma_chan_info[cur_chan].sync_chan = cur_chan;
        p_usw_dma_master[lchip]->dma_chan_info[cur_chan].sync_en = 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_init_db(uint8 lchip, ctc_dma_global_cfg_t* p_cfg)
{
    /* rx channel num should get from enq module, TODO */
    uint8 rx_chan_num = p_cfg->pkt_rx_chan_num;
    uint8 rx_chan_idx = 0;
    uint16 pri = SAL_TASK_PRIO_DEF;
    uint32 dot1ae_en = 0;

    if (rx_chan_num > SYS_DMA_RX_CHAN_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_usw_dma_master[lchip]->packet_rx_chan_num = rx_chan_num;

    /* default enable these function */
    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_HASHKEY_CHAN_ID);
    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_TBL_RD_CHAN_ID);
    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_TBL_WR_CHAN_ID);
    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_SDC_CHAN_ID);

    p_usw_dma_master[lchip]->pkt_tx_timer_en = CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_TIMER_PACKET)?1:0;

    /* init packet rx */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_RX))
    {
        for (rx_chan_idx = 0; rx_chan_idx < rx_chan_num; rx_chan_idx++)
        {
            SYS_DMA_DMACTL_CHECK(p_cfg->pkt_rx[rx_chan_idx].dmasel);
            pri = p_cfg->pkt_rx[rx_chan_idx].priority;
            if (pri == 0)
            {
                pri = SAL_TASK_PRIO_DEF;
            }

            CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, rx_chan_idx);
            p_usw_dma_master[lchip]->dma_thread_pri[rx_chan_idx] = pri;
            CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, 0, rx_chan_idx, pri));
        }
    }

    /* init packet tx */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_TX))
    {
        SYS_DMA_DMACTL_CHECK(p_cfg->pkt_tx.dmasel);
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX0_CHAN_ID);
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX1_CHAN_ID);

        if(!DRV_IS_DUET2(lchip) && p_usw_dma_master[lchip]->pkt_tx_timer_en)
        {
            CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX3_CHAN_ID);
        }

        _sys_usw_dma_tx_func_init(lchip, p_cfg->pkt_tx);
    }

    /* init learning */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING))
    {
        SYS_DMA_DMACTL_CHECK(p_cfg->learning.dmasel);
        pri = p_cfg->learning.priority;
        if (pri == 0)
        {
            pri = SAL_TASK_PRIO_DEF;
        }

        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_LEARNING_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_LEARNING_CHAN_ID] = pri;
       CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_LEARNING_CHAN_ID,
                SYS_DMA_LEARNING_CHAN_ID, pri));
    }

    p_usw_dma_master[lchip]->hw_learning_sync = p_cfg->hw_learning_sync_en;


    /* init ipfix */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_IPFIX))
    {
        SYS_DMA_DMACTL_CHECK(p_cfg->ipfix.dmasel);
        pri = p_cfg->ipfix.priority;
        if (pri == 0)
        {
            pri = SAL_TASK_PRIO_DEF;
        }
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_IPFIX_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_IPFIX_CHAN_ID] = pri;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_LEARNING_CHAN_ID, SYS_DMA_IPFIX_CHAN_ID, pri));
    }

    /* init sdc, temply enable sdc always */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_SDC))
    {
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_SDC_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_SDC_CHAN_ID] = SAL_TASK_PRIO_DEF;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_LEARNING_CHAN_ID, SYS_DMA_SDC_CHAN_ID,
            SAL_TASK_PRIO_DEF));
    }

    /* init monitor */
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_MONITOR))
    {
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_MONITOR_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_MONITOR_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_LEARNING_CHAN_ID, SYS_DMA_MONITOR_CHAN_ID,
             SAL_TASK_PRIO_DEF));
    }

    /* init port stats*/
    if (CTC_IS_BIT_SET(p_cfg->func_en_bitmap, CTC_DMA_FUNC_STATS))
    {
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PORT_STATS_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_PORT_STATS_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_PORT_STATS_CHAN_ID,
             SYS_DMA_PORT_STATS_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));
    }

    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_FLOW_STATS_CHAN_ID);
    p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_FLOW_STATS_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
    CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_PORT_STATS_CHAN_ID,
             SYS_DMA_FLOW_STATS_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));

    if (p_usw_dma_master[lchip]->pkt_tx_timer_en && DRV_IS_DUET2(lchip))
    {
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_TBL_WR_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_TBL_WR_CHAN_ID,
                 SYS_DMA_TBL_WR_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));
    }

    CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_REG_MAX_CHAN_ID);
    p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_REG_MAX_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
    CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_REG_MAX_CHAN_ID,
         SYS_DMA_REG_MAX_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));

    sys_usw_dmps_get_port_property(lchip, 0, SYS_DMPS_PORT_PROP_DOT1AE_ENABLE, &dot1ae_en);
    if(dot1ae_en && DRV_IS_TSINGMA(lchip))
    {
        /*init dot1ae stats*/
        CTC_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_TBL_RD1_CHAN_ID);
        p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_TBL_RD1_CHAN_ID] = SAL_TASK_PRIO_NICE_LOW;
        CTC_ERROR_RETURN(_sys_usw_dma_init_thread(lchip, SYS_DMA_PORT_STATS_CHAN_ID,
             SYS_DMA_TBL_RD1_CHAN_ID, SAL_TASK_PRIO_NICE_LOW));
    }
    return CTC_E_NONE;
}
#if 0
/*temply using thread to monitor dma function, delete when interrupt is ready*/
void
sys_usw_dma_scan_thread()
{
    uint8 chan_id = 0;
    uint8 chen_en = 0;
    uint8 sync_chan = 0;
    sys_dma_thread_t* p_thread_info = NULL;
    sys_dma_chan_t* p_dma_chan = NULL;
    sys_dma_desc_t* p_base_desc = NULL;
    uint16 cur_index = 0;
    DsDesc_m* p_desc = NULL;
	uint8 lchip = 0;

    while(1)
    {
        for (chan_id = 0; chan_id < SYS_DMA_MAX_CHAN_NUM; chan_id++)
        {
            if ((chan_id > SYS_DMA_PACKET_RX3_CHAN_ID) && (chan_id < SYS_DMA_LEARNING_CHAN_ID))
            {
                /*no need monitor*/
                continue;
            }

            sys_usw_dma_get_chan_en(0, chan_id, &chen_en);
            if (!chen_en)
            {
                continue;
            }

            p_dma_chan = &p_usw_dma_master[0]->dma_chan_info[chan_id];
            p_base_desc = p_dma_chan->p_desc;
            cur_index = p_dma_chan->current_index;

            p_desc = &(p_base_desc[cur_index].desc_info);
            /*inval dma before read*/
            if ((NULL != g_dal_op.dma_cache_inval) && (NULL != g_dal_op.logic_to_phy))
            {
                g_dal_op.dma_cache_inval(lchip, g_dal_op.logic_to_phy(lchip, (void*)p_desc), sizeof(DsDesc_m));
            }
            if (GetDsDescEncap(V, done_f, p_desc))
            {
                sync_chan = p_usw_dma_master[0]->dma_chan_info[chan_id].sync_chan;
                p_thread_info = ctc_vector_get(p_usw_dma_master[0]->p_thread_vector, sync_chan);
                if (!p_thread_info)
                {
                    /*means no need to create sync thread*/
                    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Dma init wrong please check!!chan:%d \n", chan_id);
                    continue;
                }

                /* push sync sem */
                (sal_sem_give(p_thread_info->p_sync_sem));
            }
        }

        sal_task_sleep(1);
    }
    return;
}
#endif
/**
 @brief reset hw dma callback
*/
int
sys_usw_dma_reset_hw(uint8 lchip, void* user_param)
{
    ctc_dma_global_cfg_t dma_cfg;
    uint32 index = 0;
    DMA_CB_FUN_P dma_cb_backup[SYS_DMA_CB_MAX_TYPE];

    sal_memset(&dma_cfg, 0, sizeof(ctc_dma_global_cfg_t));

    if (1 == p_usw_dma_master[lchip]->pkt_tx_timer_en )
    {
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_TIMER_PACKET);
    }

    /* packet rx*/
    for (index = 0; index < SYS_DMA_RX_CHAN_NUM; index++)
    {
        if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index))
        {
            dma_cfg.pkt_rx[index].dmasel = 0;
            dma_cfg.pkt_rx[index].priority = p_usw_dma_master[lchip]->dma_thread_pri[index];

            dma_cfg.pkt_rx[index].desc_num = p_usw_dma_master[lchip]->dma_chan_info[index].desc_num;
            dma_cfg.pkt_rx[index].data = p_usw_dma_master[lchip]->dma_chan_info[index].data_size;
            dma_cfg.pkt_rx[index].pkt_knet_en = p_usw_dma_master[lchip]->dma_chan_info[index].pkt_knet_en;
            CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
        }
    }
    dma_cfg.pkt_rx_chan_num = p_usw_dma_master[lchip]->packet_rx_chan_num;

    /* packet tx */
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX0_CHAN_ID)
        || CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PACKET_TX1_CHAN_ID))
    {
        uint8 tx_max_chan = 0;

        dma_cfg.pkt_tx.dmasel = 0;
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);

        if (DRV_IS_DUET2(lchip))
        {
            tx_max_chan = SYS_DMA_PACKET_TX1_CHAN_ID;
        }
        else
        {
            tx_max_chan = SYS_DMA_PACKET_TX3_CHAN_ID;
        }
        for (index = SYS_DMA_PACKET_TX0_CHAN_ID; index<= tx_max_chan; index ++)
        {
            dma_cfg.pkt_tx_ext[GET_CHAN_TYPE(index) - DRV_DMA_PACKET_TX0_CHAN_ID].pkt_knet_en = p_usw_dma_master[lchip]->dma_chan_info[index].pkt_knet_en;
            dma_cfg.pkt_tx_ext[GET_CHAN_TYPE(index) - DRV_DMA_PACKET_TX0_CHAN_ID].desc_num = p_usw_dma_master[lchip]->dma_chan_info[index].desc_num;
        }
    }

    /* learning */
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_LEARNING_CHAN_ID))
    {
        dma_cfg.learning.dmasel = 0;
        dma_cfg.learning.priority = p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_LEARNING_CHAN_ID];
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);

        dma_cfg.learning.desc_num =  p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_LEARNING_CHAN_ID].desc_num;
    }
    dma_cfg.hw_learning_sync_en = p_usw_dma_master[lchip]->hw_learning_sync;

    /* ipfix */
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_IPFIX_CHAN_ID))
    {
        dma_cfg.ipfix.dmasel = 0;
        dma_cfg.ipfix.priority = p_usw_dma_master[lchip]->dma_thread_pri[SYS_DMA_IPFIX_CHAN_ID];
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_IPFIX);
        dma_cfg.ipfix.desc_num =  p_usw_dma_master[lchip]->dma_chan_info[SYS_DMA_IPFIX_CHAN_ID].desc_num;
    }

    /* sdc, temply enable sdc always */
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_SDC_CHAN_ID))
    {
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_SDC);
    }

    /* monitor */
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_MONITOR_CHAN_ID))
    {
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_MONITOR);
    }

    /* port stats*/
    if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, SYS_DMA_PORT_STATS_CHAN_ID))
    {
        CTC_BIT_SET(dma_cfg.func_en_bitmap, CTC_DMA_FUNC_STATS);
    }

    for (index = 0; index < SYS_DMA_CB_MAX_TYPE; index++)
    {
        dma_cb_backup[index] = p_usw_dma_master[lchip]->dma_cb[index];
    }
    sys_usw_dma_deinit(lchip);
    sys_usw_dma_init(lchip, &dma_cfg);

    for (index = 0; index < SYS_DMA_CB_MAX_TYPE; index++)
    {
         p_usw_dma_master[lchip]->dma_cb[index] = dma_cb_backup[index];
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_dma_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint8 i = 0;

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Dma");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","packet_rx_chan_num",p_usw_dma_master[lchip]->packet_rx_chan_num);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pkt_rx_size_per_desc",p_usw_dma_master[lchip]->pkt_rx_size_per_desc);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","dma_en_flag",p_usw_dma_master[lchip]->dma_en_flag);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","dma_high_addr",p_usw_dma_master[lchip]->dma_high_addr);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","hw_learning_sync",p_usw_dma_master[lchip]->hw_learning_sync);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","polling_mode",p_usw_dma_master[lchip]->polling_mode);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","opf_type_dma_tx_mem",p_usw_dma_master[lchip]->opf_type_dma_tx_mem);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pkt_tx_timer_en",p_usw_dma_master[lchip]->pkt_tx_timer_en);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","tx_timer",p_usw_dma_master[lchip]->tx_timer);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","wb_reloading",p_usw_dma_master[lchip]->wb_reloading);
    SYS_DUMP_DB_LOG(p_f, "%-30s:","dma_thread_pri");
    for(i = 0; i < SYS_DMA_MAX_CHAN_NUM; i++)
    {
        SYS_DUMP_DB_LOG(p_f, "[%u]",p_usw_dma_master[lchip]->dma_thread_pri[i]);
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_dma_master[lchip]->opf_type_dma_tx_mem, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return CTC_E_NONE;
}

/**
 @brief init dma module and allocate necessary memory
*/
int32
sys_usw_dma_init(uint8 lchip, ctc_dma_global_cfg_t* dma_global_cfg)
{
    int32 ret = 0;
    uint8 index = 0;
    sys_dma_chan_t dma_chan_info;
#ifdef _SAL_LINUX_UM
    dal_dma_chan_t knet_dma_chan_info;
#endif
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 init_done = FALSE;
    drv_work_platform_type_t platform_type;
    DmaCtlDrainEnable_m dma_drain;
    uint16 init_cnt = 0;
    uint32 tbl_id1 = 0;
    uint32 tbl_id2 = 0;
    dal_dma_info_t dma_info;
    DmaMiscCfg_m misc_cfg;
    host_type_t byte_order;
    DmaCtlIntrFunc_m dma_intr_func;
    sys_usw_opf_t opf;
    uint8 warmboot_reload = 0;
    DmaEndianCtl_m          dma_edn_ctl;

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dma_drain, 0, sizeof(DmaCtlDrainEnable_m));
    if (p_usw_dma_master[lchip] && p_usw_dma_master[lchip]->wb_reloading != 1)
    {
        return CTC_E_NONE;
    }

    if (!p_usw_dma_master[lchip])
    {
        p_usw_dma_master[lchip] = (sys_dma_master_t*)mem_malloc(MEM_DMA_MODULE, sizeof(sys_dma_master_t));
        if (NULL == p_usw_dma_master[lchip])
        {
            SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }

        sal_memset(p_usw_dma_master[lchip], 0, sizeof(sys_dma_master_t));

        /* init opf for tx pkt mem */
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_dma_master[lchip]->opf_type_dma_tx_mem, 1, "opf-type-dma-tx-mem"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_dma_master[lchip]->opf_type_dma_tx_mem;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, SYS_DMA_TX_PKT_MEM_NUM));

        p_usw_dma_master[lchip]->packet_rx_chan_num = dma_global_cfg->pkt_rx_chan_num;
    }
    else
    {
        warmboot_reload = 1;
    }

    if ((CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        p_usw_dma_master[lchip]->wb_reloading = 1;
        return CTC_E_NONE;
    }
    else if (p_usw_dma_master[lchip]->wb_reloading)
    {
        p_usw_dma_master[lchip]->wb_reloading = 0;
    }

    if(NULL == (p_usw_dma_master[lchip]->tx_pending_list_H= ctc_slist_new()))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret =  CTC_E_NO_MEMORY;
        goto error;
    }

    if(NULL == (p_usw_dma_master[lchip]->tx_pending_list_L= ctc_slist_new()))
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    p_usw_dma_master[lchip]->p_thread_vector = ctc_vector_init(4, SYS_DMA_MAX_CHAN_NUM / 4);
    if (NULL == p_usw_dma_master[lchip]->p_thread_vector)
    {
        SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret =  CTC_E_NO_MEMORY;
        goto error;
    }


    sys_usw_dma_set_dump_cb(lchip, sys_usw_dma_sync_hash_dump);

    _sys_usw_dma_init_db(lchip, dma_global_cfg);

    ret = drv_get_platform_type(lchip, &platform_type);

    byte_order = drv_get_host_type(lchip);
    sal_memset(&dma_info, 0 ,sizeof(dal_dma_info_t));
    dal_get_dma_info(lchip, &dma_info);

#ifdef _SAL_LINUX_UM
   if (g_dal_op.soc_active)
    {
        uint64 phy_addr = 0;
        COMBINE_64BITS_DATA(dma_info.phy_base_hi,             \
                dma_info.phy_base, phy_addr);
        phy_addr -= 0x80000000;
        dma_info.phy_base_hi = phy_addr >>32;
    }
#endif
    p_usw_dma_master[lchip]->dma_high_addr = dma_info.phy_base_hi;
     /*-GET_HIGH_32BITS(dma_info.phy_base, p_usw_dma_master[lchip]->dma_high_addr);*/
    if (p_usw_dma_master[lchip]->dma_high_addr)
    {
         /*-SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DMA Memory exceed 4G!!!!!!!!!\n");*/
    }
    /*vxworks TLP set 128*/
#ifdef _SAL_VXWORKS
    field_val = 0;
    cmd = DRV_IOW(Pcie0SysCfg_t, Pcie0SysCfg_pcie0PcieMaxRdSize_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error);
#endif

    if (g_dal_op.soc_active)
    {
        field_val = 0xFFFFFFF;
        cmd = DRV_IOW(DmaIntfBmpCfg_t, DmaIntfBmpCfg_dmaDataChanBmp_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error);
        cmd = DRV_IOW(DmaIntfBmpCfg_t, DmaIntfBmpCfg_dmaDescChanBmp_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error);
    }

    /* 1. public config, need config 2 dmactl */
    for (index = 0; index < SYS_DMA_CTL_USED_NUM; index++)
    {
        field_val = 1;
        tbl_id1 = DmaCtlInit_t + index;
        tbl_id2 = DmaCtlInitDone_t + index;
        cmd = DRV_IOW(tbl_id1, DmaCtlInit_dmaInit_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error);
        if (platform_type == HW_PLATFORM)
        {
            /* wait for init done */
            while (init_cnt < SYS_DMA_INIT_COUNT)
            {
                cmd = DRV_IOR(tbl_id2, DmaCtlInitDone_dmaInitDone_f);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, error);

                /* init failed */
                if (field_val)
                {
                    init_done = TRUE;
                    break;
                }

                init_cnt++;
            }

            if (init_done == FALSE)
            {
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DMACtl %d init falied!\n", index);
                SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
    			ret = CTC_E_NOT_INIT;
                goto error;
            }
        }

        if (byte_order == HOST_LE)
        {
            tbl_id1 = DmaMiscCfg_t + index;
            cmd = DRV_IOR(tbl_id1, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &misc_cfg), ret, error);

            SetDmaMiscCfg(V, cfgDmaRegRdEndian_f, &misc_cfg, 1);
            SetDmaMiscCfg(V, cfgDmaRegWrEndian_f, &misc_cfg, 1);
            SetDmaMiscCfg(V, cfgDmaInfoEndian_f, &misc_cfg, 1);
            SetDmaMiscCfg(V, cfgToCpuDescEndian_f, &misc_cfg, 0);
            SetDmaMiscCfg(V, cfgFrCpuDescEndian_f, &misc_cfg, 0);

            cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &misc_cfg), ret, error);
        }
        tbl_id1 = DmaEndianCtl_t + index;
        cmd = DRV_IOR(tbl_id1, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &dma_edn_ctl);
        if (byte_order == HOST_LE)
        {
            /*DmaEndianCtl.cfgDmaPktRxEndian  0x0*/
            field_val = 0x0;
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaPktRxEndian_f, &field_val, &dma_edn_ctl);
        }
        else
        {
            field_val = 1;
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaRegRdEndian_f, &field_val, &dma_edn_ctl);
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaRegWrEndian_f, &field_val, &dma_edn_ctl);
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaInfoEndian_f, &field_val, &dma_edn_ctl);
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgFrCpuDescEndian_f, &field_val, &dma_edn_ctl);
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgToCpuDescEndian_f, &field_val, &dma_edn_ctl);
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaPktTxEndian_f, &field_val, &dma_edn_ctl);
            field_val = 0;
            DRV_IOW_FIELD(lchip, DmaEndianCtl_t, DmaEndianCtl_cfgDmaPktRxEndian_f, &field_val, &dma_edn_ctl);
        }
        cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &dma_edn_ctl);
#ifdef PCIE_SWAP_EN
        {
            tbl_id1 = DmaMiscCfg_t + index;
            cmd = DRV_IOR(tbl_id1, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &misc_cfg), ret, error);

            /*packet rx/tx config endian to swap*/
            field_val = GetDmaMiscCfg(V, cfgDmaPktRxEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgDmaPktRxEndian_f, &misc_cfg, field_val);
            field_val = GetDmaMiscCfg(V, cfgDmaPktTxEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgDmaPktTxEndian_f, &misc_cfg, field_val);
            /*reg wr/rd config endian to swap*/
            field_val = GetDmaMiscCfg(V, cfgDmaRegRdEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgDmaRegRdEndian_f, &misc_cfg, field_val);
            field_val = GetDmaMiscCfg(V, cfgDmaRegWrEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgDmaRegWrEndian_f, &misc_cfg, field_val);
            /*info config endian to swap*/
            field_val = GetDmaMiscCfg(V, cfgDmaInfoEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgDmaInfoEndian_f, &misc_cfg, field_val);
            /*cpu desc config endian to swap*/
            field_val = GetDmaMiscCfg(V, cfgToCpuDescEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgToCpuDescEndian_f, &misc_cfg, field_val);
            field_val = GetDmaMiscCfg(V, cfgFrCpuDescEndian_f, &misc_cfg);
            field_val = field_val?0:1;
            SetDmaMiscCfg(V, cfgFrCpuDescEndian_f, &misc_cfg, field_val);

            cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &misc_cfg), ret, error);
        }
#endif
        /* init dma endian, using default config  */
         CTC_ERROR_GOTO(_sys_usw_dma_intr_init(lchip, index, dma_global_cfg), ret, error);
        /* cfg Info Dma timer */
         CTC_ERROR_GOTO(_sys_usw_dma_timer_init(lchip, index), ret, error);
       /* cfg packet crc */
        CTC_ERROR_GOTO(_sys_usw_dma_crc_init(lchip, index), ret, error);

    }

    /* 2.  per channel config, just config the dmactl which have the function  */
    for (index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {
        if (CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index))
        {
            sal_memset(&dma_chan_info, 0, sizeof(sys_dma_chan_t));
            CTC_ERROR_GOTO(_sys_usw_dma_get_chan_cfg(lchip, index, dma_global_cfg, &dma_chan_info), ret, error);
            CTC_ERROR_GOTO(_sys_usw_dma_common_init(lchip, &dma_chan_info), ret, error);
            CTC_ERROR_GOTO(_sys_usw_dma_sync_init(lchip, index), ret, error);
#ifdef _SAL_LINUX_UM
            if (dma_chan_info.pkt_knet_en)
            {
                sal_memset(&knet_dma_chan_info, 0, sizeof(dal_dma_chan_t));
                knet_dma_chan_info.lchip = lchip;
                knet_dma_chan_info.channel_id = index;
                knet_dma_chan_info.dmasel = 0;
                knet_dma_chan_info.active = 1;
                knet_dma_chan_info.desc_num = p_usw_dma_master[lchip]->dma_chan_info[index].desc_num;
                knet_dma_chan_info.desc_depth = p_usw_dma_master[lchip]->dma_chan_info[index].desc_depth;
                knet_dma_chan_info.data_size = p_usw_dma_master[lchip]->dma_chan_info[index].data_size;
                knet_dma_chan_info.mem_base = g_dal_op.logic_to_phy(lchip, (void*)p_usw_dma_master[lchip]->dma_chan_info[index].mem_base);

                ret = dal_dma_chan_register(lchip, &knet_dma_chan_info);
                if (ret < 0)
                {
                    p_usw_dma_master[lchip]->dma_chan_info[index].pkt_knet_en = 0;
                }
            }
#else
            p_usw_dma_master[lchip]->dma_chan_info[index].pkt_knet_en = 0;
#endif
        }
    }

    /*just for uml and cmodel sim platform*/
    if ((1 == SDK_WORK_PLATFORM) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))
    {
        cmd = DRV_IOR(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &dma_drain), ret, error);
        SetDmaCtlDrainEnable(V, dmaInfo0DrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaInfo1DrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaInfo2DrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaInfo3DrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaInfo4DrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaPktRxDrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaPktTxDrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaRegRdDrainEn_f, &dma_drain, 1);
        SetDmaCtlDrainEnable(V, dmaRegWrDrainEn_f, &dma_drain, 1);
        cmd = DRV_IOW(DmaCtlDrainEnable_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &dma_drain), ret, error);
    }

    drv_usw_agent_register_cb(DRV_AGENT_CB_SET_DMA_RX,   (void*)sys_usw_dma_register_cb);
    drv_usw_agent_register_cb(DRV_AGENT_CB_SET_DMA_DUMP, (void*)sys_usw_dma_set_dump_cb);
    drv_usw_agent_register_cb(DRV_AGENT_CB_GET_DMA_DUMP, (void*)sys_usw_dma_get_dump_cb);
    drv_usw_agent_register_cb(DRV_AGENT_CB_PKT_TX,       (void*)sys_usw_dma_pkt_tx);

    /*call interrupt module to register isr*/
    CTC_ERROR_GOTO(sys_usw_interrupt_register_isr(lchip, SYS_INTR_DMA , sys_usw_dma_isr_func), ret, error);

    /*clear dma interrupt*/
    cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 2, cmd, &dma_intr_func), ret, error);
    cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 3, cmd, &dma_intr_func), ret, error);
#ifdef _SAL_LINUX_UM
    dal_dma_finish_cb_register(_sys_usw_dma_pkt_finish_cb);
#endif

    drv_ser_register_hw_reset_cb(lchip, DRV_SER_HW_RESET_CB_TYPE_DMA, sys_usw_dma_reset_hw);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_DMA, _sys_usw_dma_dump_db), ret, error);

    if (CTC_WB_ENABLE && warmboot_reload)
    {
        ctc_l2_flush_fdb_t flush_fdb;
        /*flush pending entry to relearning when warmboot reloading done*/
        sal_memset(&flush_fdb, 0, sizeof(ctc_l2_flush_fdb_t));
        flush_fdb.flush_flag = CTC_L2_FDB_ENTRY_PENDING;
        flush_fdb.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
        sys_usw_l2_flush_fdb(lchip, &flush_fdb);
    }

    return CTC_E_NONE;

error:
    sys_usw_dma_deinit(lchip);
    return ret;
}


STATIC int32
_sys_usw_dma_free_node_data(void* node_data, void* user_data)
{
    if (node_data)
    {
        mem_free(node_data);
    }

    return CTC_E_NONE;
}

int32
sys_usw_dma_deinit(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 1;
#ifdef _SAL_LINUX_UM
    dal_dma_chan_t knet_dma_chan_info;
#endif
    sys_dma_thread_t* p_thread_info = NULL;
    sys_dma_chan_t*   p_dma_chan = NULL;
    uint32 index = 0;
    uint8 sync_chan = 0;
    uint32  desc_index = 0;
    void* logic_addr = 0;
    uint32 low_phy_addr = 0;
    uint64 phy_addr = 0;
    DsDesc_m* p_desc = NULL;

    LCHIP_CHECK(lchip);
    if (NULL == p_usw_dma_master[lchip])
    {
        return CTC_E_NONE;
    }

    SYS_DMA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
#ifdef _SAL_LINUX_UM
    dal_dma_finish_cb_register(NULL);
#endif

    /*disable all dma channel*/
    for (index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {
        if (!CTC_IS_BIT_SET(p_usw_dma_master[lchip]->dma_en_flag, index))
        {
            continue;
        }
        sys_usw_dma_set_chan_en(lchip, index, 0);
        p_dma_chan = (sys_dma_chan_t*)&p_usw_dma_master[lchip]->dma_chan_info[index];
        if (p_dma_chan->p_mutex)
        {
#ifndef PACKET_TX_USE_SPINLOCK
            sal_mutex_destroy(p_dma_chan->p_mutex);
#else
            sal_spinlock_destroy((sal_spinlock_t*)p_dma_chan->p_mutex);
#endif
            p_dma_chan->p_mutex = NULL;
        }
        if (p_dma_chan->p_desc_used)
        {
            mem_free(p_dma_chan->p_desc_used);
            p_dma_chan->p_desc_used = NULL;
        }
        if(p_dma_chan->p_desc_info)
        {
            mem_free(p_dma_chan->p_desc_info);
            p_dma_chan->p_desc_info= NULL;
        }
#ifdef _SAL_LINUX_UM
        if (p_dma_chan->pkt_knet_en)
        {
            sal_memset(&knet_dma_chan_info, 0, sizeof(dal_dma_chan_t));
            knet_dma_chan_info.lchip = lchip;
            knet_dma_chan_info.channel_id = index;
            knet_dma_chan_info.active = 0;

            dal_dma_chan_register(lchip, &knet_dma_chan_info);
        }
#endif
    }

    for (index=0; index< SYS_DMA_MAX_CHAN_NUM; index++)
    {
        sync_chan = p_usw_dma_master[lchip]->dma_chan_info[index].sync_chan;
        p_thread_info = ctc_vector_get(p_usw_dma_master[lchip]->p_thread_vector, sync_chan);
        if (p_thread_info)
        {
            if (NULL != p_thread_info->p_sync_sem)
            {
                sal_sem_give(p_thread_info->p_sync_sem);
                sal_task_sleep(1);
                sal_task_destroy(p_thread_info->p_sync_task);
                p_thread_info->p_sync_task = NULL;
                sal_sem_destroy(p_thread_info->p_sync_sem);
                p_thread_info->p_sync_sem = NULL;
            }
        }
    }

    cmd = DRV_IOW(SupResetCtl_t, SupResetCtl_resetDmaCtl_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    sys_usw_set_dma_channel_drop_en(lchip, TRUE);

    sys_usw_opf_deinit(lchip, p_usw_dma_master[lchip]->opf_type_dma_tx_mem);

    /*free vector data*/
    ctc_vector_traverse(p_usw_dma_master[lchip]->p_thread_vector, (vector_traversal_fn)_sys_usw_dma_free_node_data, NULL);
    ctc_vector_release(p_usw_dma_master[lchip]->p_thread_vector);

    /*free hash*/
    ctc_hash_traverse_remove(p_usw_dma_master[lchip]->dma_tx_mem_entry, (hash_traversal_fn)_sys_usw_dma_free_node_data, NULL);
    ctc_hash_free(p_usw_dma_master[lchip]->dma_tx_mem_entry);

    /*wake up thread, to skip auto*/
    if(p_usw_dma_master[lchip]->tx_sem)
    {
        sal_sem_give(p_usw_dma_master[lchip]->tx_sem);
        sal_task_sleep(1);
    }
    if (p_usw_dma_master[lchip]->p_async_tx_task)
    {
        sal_task_destroy(p_usw_dma_master[lchip]->p_async_tx_task);
    }

    /*free slist*/
    ctc_slist_delete(p_usw_dma_master[lchip]->tx_pending_list_H);
    ctc_slist_delete(p_usw_dma_master[lchip]->tx_pending_list_L);

    if(p_usw_dma_master[lchip]->tx_sem)
    {
        sal_sem_destroy(p_usw_dma_master[lchip]->tx_sem);
    }
    if(p_usw_dma_master[lchip]->p_tx_mutex)
    {
        sal_spinlock_destroy(p_usw_dma_master[lchip]->p_tx_mutex);
    }

    for (index = 0; index < SYS_DMA_MAX_CHAN_NUM; index++)
    {
        p_dma_chan = (sys_dma_chan_t*)&p_usw_dma_master[lchip]->dma_chan_info[index];

        if (p_dma_chan && (p_dma_chan->data_size || (SYS_DMA_REG_MAX_CHAN_ID == p_dma_chan->channel_id)))
        {
            for (desc_index = 0; desc_index < p_dma_chan->desc_num; desc_index++)
            {
                p_desc = &p_dma_chan->p_desc[desc_index].desc_info;
                if (p_desc)
                {
                    low_phy_addr = GetDsDescEncap(V, memAddr_f, p_desc);
                    COMBINE_64BITS_DATA(p_usw_dma_master[lchip]->dma_high_addr, low_phy_addr<<4, phy_addr);
                    logic_addr = (void*)g_dal_op.phy_to_logic(lchip, phy_addr);
                    if (NULL != logic_addr)
                    {
                        g_dal_op.dma_free(lchip, (void*)logic_addr);
                    }
                }
            }
        }
        if (p_dma_chan && (NULL != p_dma_chan->p_desc))
        {
            g_dal_op.dma_free(lchip, p_dma_chan->p_desc);
            p_dma_chan->p_desc = NULL;
        }
    }

    /* free tx pkt mem pool*/
    for(index =0;index < SYS_DMA_TX_PKT_MEM_NUM; index++)
    {
        if (0 != p_usw_dma_master[lchip]->tx_mem_pool[index])
        {
            g_dal_op.dma_free(lchip, (void*)p_usw_dma_master[lchip]->tx_mem_pool[index]);
            p_usw_dma_master[lchip]->tx_mem_pool[index] = 0;
        }
    }

    /*free master*/
    mem_free(p_usw_dma_master[lchip]);

    /*clear resetDmaCtl */
    field_val = 0;
    cmd = DRV_IOW(SupResetCtl_t, SupResetCtl_resetDmaCtl_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*clear credit about dmactl related module*/
    /*1. flow acc*/
    field_val = 0;
    cmd = DRV_IOW(FlowAccCreditStatus_t, FlowAccCreditStatus_flowAccDmaCreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*2. bufretr */
    cmd = DRV_IOW(BufRetrvCreditStatus_t, BufRetrvCreditStatus_bufRetrvDma0CreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    cmd = DRV_IOW(BufRetrvCreditStatus_t, BufRetrvCreditStatus_bufRetrvDma1CreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    cmd = DRV_IOW(BufRetrvCreditStatus_t, BufRetrvCreditStatus_bufRetrvDma2CreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    cmd = DRV_IOW(BufRetrvCreditStatus_t, BufRetrvCreditStatus_bufRetrvDma3CreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*3. fibacc */
    field_val = 0;
    cmd = DRV_IOW(FibAccCreditUsed_t, FibAccCreditUsed_fibAccDmaCreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*4. Qmgr */
    cmd = DRV_IOW(QMgrEnqCreditStatus_t, QMgrEnqCreditStatus_dmaQMgrCreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*5. EPEHdr */
    cmd = DRV_IOW(EpeHdrProcCreditStatus_t, EpeHdrProcCreditStatus_dmaCreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);

    /*6. Bufstore */
    cmd = DRV_IOW(DmaBufStoreCreditStatus_t, DmaBufStoreCreditStatus_dmaBufStoreCreditUsed_f);
    DRV_IOCTL(lchip, 0, cmd, &field_val);
    return CTC_E_NONE;
}

