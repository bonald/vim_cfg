/**
   @file ctc_usw_learning_aging.c

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
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "ctc_learning_aging.h"
#include "ctc_interrupt.h"
#include "ctc_debug.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_learning_aging.h"
#include "sys_usw_dma.h"
#include "sys_usw_register.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Defines and Macros
 *
 *****************************************************************************/

#define IGNORE_HW_CONTROL               0

#define NUM_8K                          (8 * 1024)
#define NUM_16K                         (16 * 1024)
#define NUM_32K                         (32 * 1024)
#define NUM_64K                         (64 * 1024)

#define SYS_LEARNING_AGING_DBG_OUT(level, FMT, ...)                                           \
    {                                                                                         \
        CTC_DEBUG_OUT(l2, learning_aging, L2_LEARNING_AGING_SYS, level, FMT, ## __VA_ARGS__); \
    }
#define SYS_LEARNING_AGING_INIT_CHECK(lchip)             \
    do {                                \
        SYS_LCHIP_CHECK_ACTIVE(lchip);       \
        if (NULL == pla_master[lchip])  \
        { return CTC_E_NOT_INIT; }      \
    } while(0)

/**
   @brief struct type about aging info
 */
struct sys_learning_aging_master_s
{
    uint32                   aging_max_ptr;
    uint32                   aging_min_ptr;

    sys_learning_aging_fn_t  hw_dma_cb;
    void                     * user_data;

    sal_mutex_t*             p_aging_mutex;
};
typedef struct sys_learning_aging_master_s   sys_learning_aging_master_t;

/****************************************************************************
 *
 * Global and Declaration
 *
 *****************************************************************************/
static sys_learning_aging_master_t* pla_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

extern int32
sys_usw_l2_delete_hw_by_mac_fid(uint8 lchip, mac_addr_t mac, uint16 fid, uint8 mac_limit_en);

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

int32
sys_usw_learning_aging_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    sal_mutex_lock(pla_master[lchip]->p_aging_mutex);

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "##Learing/Aging");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","aging_max_ptr",pla_master[lchip]->aging_max_ptr);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","aging_min_ptr",pla_master[lchip]->aging_min_ptr);
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");

    sal_mutex_unlock(pla_master[lchip]->p_aging_mutex);

    return ret;
}



#define ___LEARNING___


int32
sys_usw_learning_set_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action)
{

    uint32                     cmd       = 0;
    uint32                     field_val = 0;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learning_action);

    if (p_learning_action->action != CTC_LEARNING_ACTION_MAC_TABLE_FULL)
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Set learning cache threshold: %d, action: %d\n",
                                   p_learning_action->value, p_learning_action->action);
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    cmd       = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_macTableFull_f);
    field_val = (p_learning_action->value) ? 1 : 0;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_learning_get_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action)
{
    uint32                     cmd = 0;
    uint32                     field_val = 0;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learning_action);

    cmd       = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_macTableFull_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (field_val)
    {
        p_learning_action->action = CTC_LEARNING_ACTION_MAC_TABLE_FULL;
    }
    return CTC_E_NONE;
}


/**
1. tcam aging(lpm tcam, aging for host route), 1bit stands for 46bits ipv4 entry, total 8K
2. hash aging, 1bits stands for 85bits hash entry, total 196k
3. register aging, host0 cam(32)+flow hash(32)+host1_da(16)+host1_sa(16), total 96
Toal aging: 8k+196k+96
*/
#define ___AGING___

STATIC int32
_sys_usw_aging_set_aging_interval(uint8 lchip, uint8 timer_idx, uint32 age_seconds)
{
    uint32 cmd             = 0;
    uint32 aging_interval  = 0;
    uint32 max_age_seconds = 0;
    uint32 min_age_seconds = 0;
    uint32 core_frequency  = 0;
    uint64 temp0 = 0;
    uint64 temp1 = 0;
    IpeAgingCtl_m aging_ctl;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "[Aging] Set aging interval:aging_index: %d, age_seconds:%d\n", timer_idx, age_seconds);

 /*    core_frequency = sys_usw_get_core_freq(0) * 1000000 temp for testing;*/
    core_frequency = sys_usw_get_core_freq(lchip, 0) * 1000000 /DOWN_FRE_RATE;

    /* max intervale is 0xFFFFFFFF*/
    max_age_seconds = (0xFFFFFFFF / core_frequency) * (pla_master[lchip]->aging_max_ptr-pla_master[lchip]->aging_min_ptr);
    min_age_seconds = 1;
    if (age_seconds < min_age_seconds || age_seconds > max_age_seconds)
    {
        return CTC_E_INVALID_PARAM;
    }

    temp0 = (uint64)age_seconds*core_frequency;
    temp1 = (pla_master[lchip]->aging_max_ptr-pla_master[lchip]->aging_min_ptr);

    aging_interval = temp0/temp1;

    cmd = DRV_IOR(IpeAgingCtl_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &aging_ctl));

    switch(timer_idx)
    {
        case SYS_AGING_TIMER_INDEX_MAC:
            SetIpeAgingCtl(V, gTimmer_0_agingInterval_f, &aging_ctl, aging_interval);
            break;
        case SYS_AGING_TIMER_INDEX_RSV:
            SetIpeAgingCtl(V, gTimmer_1_agingInterval_f, &aging_ctl, 5);
            break;
        case SYS_AGING_TIMER_INDEX_PENDING:
            SetIpeAgingCtl(V, gTimmer_2_agingInterval_f, &aging_ctl, aging_interval);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(IpeAgingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &aging_ctl));

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] Set aging aging_interval: %u max_age_seconds:%u \n", aging_interval, max_age_seconds);

    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_set_hw_sync(uint8 lchip, uint8 b_sync)
{
    FibAccelerationCtl_m fib_acc_ctl;
    uint32                 cmd = 0;
    uint32 hw_learning = 0;
    uint32 dma_sync = 0;
    uint32 ignore_dma = 0;

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    cmd = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_fastLearningEn_f);
    DRV_IOCTL(lchip, 0, cmd, &hw_learning);

    if (hw_learning)
    {
        dma_sync = b_sync;
        ignore_dma = (b_sync ? 0 : 1);
    }
    else
    {
        dma_sync = 0;
        ignore_dma = 0;
    }

    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));
    SetFibAccelerationCtl(V, dmaForFastLearning_f, &fib_acc_ctl, dma_sync);
    SetFibAccelerationCtl(V, dmaForFastAging_f, &fib_acc_ctl, dma_sync);
    SetFibAccelerationCtl(V, fastAgingIgnoreFifoFull_f, &fib_acc_ctl, ignore_dma);
    SetFibAccelerationCtl(V, fastLearningIgnoreFifoFull_f, &fib_acc_ctl, ignore_dma);
    cmd = DRV_IOW(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    return CTC_E_NONE;
}

 int32
_sys_usw_aging_set_register(uint8 lchip)
{
    uint32                 cmd = 0;
    FibAccelerationCtl_m fib_acc_ctl;

    /* 1. mac hash dem */
    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    SetFibAccelerationCtl(V, levelNum0_f, &fib_acc_ctl, 0);
    SetFibAccelerationCtl(V, levelNum1_f, &fib_acc_ctl, 1);
    SetFibAccelerationCtl(V, levelNum2_f, &fib_acc_ctl, 2);
    SetFibAccelerationCtl(V, levelNum3_f, &fib_acc_ctl, 3);
    SetFibAccelerationCtl(V, levelNum4_f, &fib_acc_ctl, 4);
    SetFibAccelerationCtl(V, levelNum5_f, &fib_acc_ctl, 5);
    SetFibAccelerationCtl(V, macAgingTimerIndex0_f, &fib_acc_ctl, 1);   /*used for normal aging*/
    SetFibAccelerationCtl(V, macAgingTimerIndex1_f, &fib_acc_ctl, 3);   /*used for pending aging*/
    SetFibAccelerationCtl(V, macAgingTimerSelect_f, &fib_acc_ctl, 1);
    cmd = DRV_IOW(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    return CTC_E_NONE;
}


/*
    Aging timer0: for normal  fdb entry aging
    Aging timer1: for ipuc entry aging
    Aging timer2: for pending fdb entry aging
*/
STATIC int32
_sys_usw_aging_init(uint8 lchip, ctc_learn_aging_global_cfg_t* cfg)
{
    IpeAgingCtl_m                ipe_aging_ctl;
    IpeAgingScanRangeCtl_m     ipe_aging_range0;
    FibEngineLookupResultCtl_m fib_ctl;
    uint32                         cmd            = 0;
    uint32 val = 0;
    FibHost0HashAgingBase_m   aging_base;

    CTC_ERROR_RETURN(_sys_usw_aging_set_register(lchip));

    /* IpeAgingCtl */
    cmd = DRV_IOR(IpeAgingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_ctl));
    cmd = DRV_IOR(IpeAgingScanRangeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_range0));

    SetIpeAgingCtl(V, gTimmer_0_agingPtr_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_1_agingPtr_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_2_agingPtr_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_0_scanEn_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_1_scanEn_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_2_scanEn_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, agingIndexValid_f, &ipe_aging_ctl, 1);

    SetIpeAgingCtl(V, gTimmer_0_stopOnMaxPtr_f, &ipe_aging_ctl, 0);
    SetIpeAgingCtl(V, gTimmer_1_stopOnMaxPtr_f, &ipe_aging_ctl, 1);
    SetIpeAgingCtl(V, gTimmer_2_stopOnMaxPtr_f, &ipe_aging_ctl, 0);

    if (DRV_IS_DUET2(lchip))
    {
        pla_master[lchip]->aging_max_ptr = 8*1024 + 196*1024 + 96 - 1;  /*8k+196k+96 = 0x33060*/
        pla_master[lchip]->aging_min_ptr = 8*1024;

        cmd = DRV_IOR(FibHost0HashAgingBase_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_base));

        /*For host1 aging base, must 1k align, because of RTL process*/
        val = 148480; /*level0 base:144*1024+64, for 1k align using 145k*/
        SetFibHost0HashAgingBase(V, fibHost1AgingOffset_f, &aging_base, val);
        val = 161;    /*level1 base:145k+16K*/
        SetFibHost0HashAgingBase(V, fibHost1Level1AgingBase_f, &aging_base, val);
        val = 177;    /*level2 base:145k+16K+16k*/
        SetFibHost0HashAgingBase(V, fibHost1Level2AgingBase_f, &aging_base, val);
        val = 185;    /*level3 base:145k+16K+16k+8k*/
        SetFibHost0HashAgingBase(V, fibHost1Level3AgingBase_f, &aging_base, val);
        val = 193;    /*level4 base:145k+16K+16k+8k+8k*/
        SetFibHost0HashAgingBase(V, fibHost1Level4AgingBase_f, &aging_base, val);
        cmd = DRV_IOW(FibHost0HashAgingBase_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_base));
    }
    else
    {
        pla_master[lchip]->aging_max_ptr = 13572*32 - 1;  /*8k+196k+96 = 0x33060*/
        pla_master[lchip]->aging_min_ptr = (16*1024);
    }

    SetIpeAgingScanRangeCtl(V, gTimmer_0_minPtr_f, &ipe_aging_range0, pla_master[lchip]->aging_min_ptr);
    SetIpeAgingScanRangeCtl(V, gTimmer_0_maxPtr_f, &ipe_aging_range0, pla_master[lchip]->aging_max_ptr);

    /*Timer1 used to resolve CPU set aging status, minPtr must set to 0 for tcam aging*/
    SetIpeAgingScanRangeCtl(V, gTimmer_1_minPtr_f, &ipe_aging_range0, 0);/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    SetIpeAgingScanRangeCtl(V, gTimmer_1_maxPtr_f, &ipe_aging_range0, pla_master[lchip]->aging_max_ptr);

    SetIpeAgingScanRangeCtl(V, gTimmer_2_minPtr_f, &ipe_aging_range0, pla_master[lchip]->aging_min_ptr);
    SetIpeAgingScanRangeCtl(V, gTimmer_2_maxPtr_f, &ipe_aging_range0, pla_master[lchip]->aging_max_ptr);

    cmd = DRV_IOW(IpeAgingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_ctl));
    cmd = DRV_IOW(IpeAgingScanRangeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_range0));

    /*aging interval  */
    CTC_ERROR_RETURN(_sys_usw_aging_set_aging_interval(lchip, SYS_AGING_TIMER_INDEX_MAC, 30));  /*30s*/
    CTC_ERROR_RETURN(_sys_usw_aging_set_aging_interval(lchip, SYS_AGING_TIMER_INDEX_RSV, 30));  /*30s*/
    CTC_ERROR_RETURN(_sys_usw_aging_set_aging_interval(lchip, SYS_AGING_TIMER_INDEX_PENDING, 5*60));  /*5*60s*/

    cmd = DRV_IOR(IpeAgingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_ctl));
    SetIpeAgingCtl(V, gTimmer_2_scanEn_f, &ipe_aging_ctl, 1);
    cmd = DRV_IOW(IpeAgingCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aging_ctl));

    /* FibEngineLookupResultCtl */
    cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_ctl));
    SetFibEngineLookupResultCtl(V, gIpv4McastLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gIpv4NatSaLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gIpv6McastLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gIpv6NatSaLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_agingEn_f ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gMacDaLookupResultCtl_agingEn_f     ,&fib_ctl, 0);
    SetFibEngineLookupResultCtl(V, gMacIpv4LookupResultCtl_agingEn_f   ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gMacIpv6LookupResultCtl_agingEn_f   ,&fib_ctl, 1);
    SetFibEngineLookupResultCtl(V, gMacSaLookupResultCtl_agingEn_f     ,&fib_ctl, 0);
    cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_ctl));

    val = 1;
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_updateAgingEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &val));

    sal_mutex_create(&pla_master[lchip]->p_aging_mutex);
    if(!pla_master[lchip]->p_aging_mutex)
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
		return CTC_E_NO_RESOURCE;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_aging_get_host0_aging_ptr(uint8 lchip, uint32 key_index, uint32* aging_ptr)
{
    uint32 cmd = 0;
    FibHost0HashLookupCtl_m fib_ctl;
    uint8 find_flag = 1;
    uint32 tcam_aging_num = 0;

    cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &fib_ctl));
    if (key_index < 32)
    {
        /*cam*/
        *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0IpLookupAgingIndexBase_f, &fib_ctl) + key_index;
    }
    else if (key_index < GetFibHost0HashLookupCtl(V, fibHost0Level1IndexBase_f, &fib_ctl) + 32)
    {
        /*level 0*/
        *aging_ptr = key_index;
    }
    else if (key_index < GetFibHost0HashLookupCtl(V, fibHost0Level2IndexBase_f, &fib_ctl) + 32)
    {
        /*level 1*/
        *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0Level1AgingIndexBase_f, &fib_ctl) + key_index;
    }
    else if (key_index < GetFibHost0HashLookupCtl(V, fibHost0Level3IndexBase_f, &fib_ctl) + 32)
    {
        /*level 2*/
        *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0Level2AgingIndexBase_f, &fib_ctl) + key_index;
    }
    else if (key_index < GetFibHost0HashLookupCtl(V, fibHost0Level4IndexBase_f, &fib_ctl) + 32)
    {
        /*level 3*/
        *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0Level3AgingIndexBase_f, &fib_ctl) + key_index;
    }
    else if (key_index < GetFibHost0HashLookupCtl(V, fibHost0Level5IndexBase_f, &fib_ctl) + 32)
    {
        /*level 4*/
        *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0Level4AgingIndexBase_f, &fib_ctl) + key_index;
    }
    else
    {
        if (GetFibHost0HashLookupCtl(V, fibHost0Level5HashEn_f, &fib_ctl) && !GetFibHost0HashLookupCtl(V, fibHost0Level5MacKeyEn_f, &fib_ctl))
        {
            /*level 5*/
            *aging_ptr = GetFibHost0HashLookupCtl(V, fibHost0Level5AgingIndexBase_f, &fib_ctl) + key_index;
        }
        else
        {
            /*not found*/
            find_flag = 0;
        }
    }

    if (find_flag)
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsAgingStatusTcam_t, &tcam_aging_num);
        tcam_aging_num = tcam_aging_num*32;
        *aging_ptr += tcam_aging_num;
    }
    else
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[Aging]The key-index is out of range! ");
        return CTC_E_HW_INVALID_INDEX;
    }

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] index to AgingPtr:0x%x \n", *aging_ptr);
    return CTC_E_NONE;
}

int32
_sys_usw_aging_get_host1_aging_ptr(uint8 lchip, uint32 key_index, uint32* aging_ptr)
{
    uint32 cmd = 0;
    FibHost1HashLookupCtl_m fib_ctl;
    uint32 host1_aging_base = 0;
    uint32 fib_level_thres = 0;
    uint8 find_flag = 0;
    uint32 tcam_aging_num = 0;
    FibHost0HashAgingBase_m aging_base;

    cmd = DRV_IOR(FibHost1HashLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &fib_ctl));
    cmd = DRV_IOR(FibHost0HashAgingBase_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_base));

    host1_aging_base = GetFibHost1HashLookupCtl(V, fibHost1AgingIndexOffset_f, &fib_ctl);
    if (GetFibHost1HashLookupCtl(V, fibHost1Level0HashEn_f, &fib_ctl))
    {
        fib_level_thres = GetFibHost1HashLookupCtl(V, fibHost1Level1IndexBase_f, &fib_ctl);
        if (key_index < fib_level_thres+32)
        {
            if (DRV_IS_DUET2(lchip))
            {
                *aging_ptr = GetFibHost0HashAgingBase(V, fibHost1AgingOffset_f, &aging_base)+key_index;
            }
            else
            {
                *aging_ptr = host1_aging_base+key_index;
            }

            find_flag = 1;
            SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging]Host1 key index:0x%x, level:%u \n", key_index, 0);
        }
    }
    if (GetFibHost1HashLookupCtl(V, fibHost1Level1HashEn_f, &fib_ctl) && !find_flag)
    {
        if(DRV_IS_DUET2(lchip))
        {
            fib_level_thres += GetFibHost1HashLookupCtl(V, fibHost1Level2IndexBase_f, &fib_ctl);
            if (key_index < fib_level_thres+32)
            {
                *aging_ptr = GetFibHost0HashAgingBase(V, fibHost1Level1AgingBase_f, &aging_base)*1024 +
                        key_index - GetFibHost1HashLookupCtl(V, fibHost1Level1IndexBase_f, &fib_ctl);
                find_flag = 1;
                SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging]Host1 key index:0x%x, level:%u \n", key_index, 1);
            }
        }
        else
        {
            *aging_ptr = host1_aging_base+key_index-GetFibHost1HashLookupCtl(V, fibHost1Level1IndexBase_f, &fib_ctl)+GetFibHost1HashLookupCtl(V, fibHost1Level1AgingIndexBase_f, &fib_ctl);
            find_flag = 1;
        }

    }
    if (GetFibHost1HashLookupCtl(V, fibHost1Level2HashEn_f, &fib_ctl) && !find_flag)
    {
        fib_level_thres += GetFibHost1HashLookupCtl(V, fibHost1Level3IndexBase_f, &fib_ctl);
        if (key_index < fib_level_thres+32)
        {
            if (DRV_IS_DUET2(lchip))
            {
                *aging_ptr = GetFibHost0HashAgingBase(V, fibHost1Level2AgingBase_f, &aging_base)*1024 +
                        key_index - GetFibHost1HashLookupCtl(V, fibHost1Level2IndexBase_f, &fib_ctl);
            }
            else
            {
                *aging_ptr = host1_aging_base+key_index+8*1024;
            }
            find_flag = 1;
            SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging]Host1 key index:0x%x, level:%u \n", key_index, 2);
        }
    }
    if (GetFibHost1HashLookupCtl(V, fibHost1Level3HashEn_f, &fib_ctl) && !find_flag)
    {
        fib_level_thres += GetFibHost1HashLookupCtl(V, fibHost1Level4IndexBase_f, &fib_ctl);
        if (key_index < fib_level_thres+32)
        {
            if (DRV_IS_DUET2(lchip))
            {
                *aging_ptr = GetFibHost0HashAgingBase(V, fibHost1Level3AgingBase_f, &aging_base)*1024 +
                        key_index - GetFibHost1HashLookupCtl(V, fibHost1Level3IndexBase_f, &fib_ctl);
            }
            else
            {
                *aging_ptr = host1_aging_base+key_index+40*1024;
            }
            find_flag = 1;
            SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging]Host1 key index:0x%x, level:%u \n", key_index, 3);
        }
    }
    if (GetFibHost1HashLookupCtl(V, fibHost1Level4HashEn_f, &fib_ctl) && !find_flag)
    {
        if (DRV_IS_DUET2(lchip))
        {
            *aging_ptr = GetFibHost0HashAgingBase(V, fibHost1Level4AgingBase_f, &aging_base)*1024 +
                    key_index - GetFibHost1HashLookupCtl(V, fibHost1Level4IndexBase_f, &fib_ctl);
        }
        else
        {
            *aging_ptr = host1_aging_base+key_index+48*1024;
        }
        find_flag = 1;
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging]Host1 key index:0x%x, level:%u \n", key_index, 4);
    }

    if (find_flag)
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsAgingStatusTcam_t, &tcam_aging_num);
        tcam_aging_num = tcam_aging_num*32;
        *aging_ptr += tcam_aging_num;
    }
    else
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[Aging]The key-index is out of range! ");
        return CTC_E_HW_INVALID_INDEX;
    }

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] index to AgingPtr:0x%x \n", *aging_ptr);

    return CTC_E_NONE;
}

/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
STATIC int32
_sys_usw_aging_get_tcam_aging_ptr(uint8 lchip, uint8 domain_type, uint32 key_index, uint32* aging_ptr)
{
    /* only consider tcam used for NAT SA */
    *aging_ptr = !DRV_IS_DUET2(lchip)?(key_index * 4 + 8192):(key_index * 4 + 4096);
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] Tcam aging_ptr:%d\n", *aging_ptr);
    return CTC_E_NONE;
}
/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

int32
sys_usw_aging_get_aging_ptr(uint8 lchip, uint8 domain_type, uint32 key_index, uint32* aging_ptr)
{
    uint8 current_level = 0;
    uint8 is_cam = 0;
    uint32 cmd = 0;
    FibAccelerationCtl_m fib_ctl;
    uint32 temp_idx = 0;
    uint32 tcam_aging_num = 0;
    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (domain_type == SYS_AGING_DOMAIN_HOST1)
    {
        CTC_ERROR_RETURN(_sys_usw_aging_get_host1_aging_ptr(lchip, key_index, aging_ptr));
        return CTC_E_NONE;
    }
    else if (domain_type == SYS_AGING_DOMAIN_IP_HASH && !DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(_sys_usw_aging_get_host0_aging_ptr(lchip, key_index, aging_ptr));
        return CTC_E_NONE;
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    else if (domain_type == SYS_AGING_DOMAIN_TCAM)
    {
        CTC_ERROR_RETURN(_sys_usw_aging_get_tcam_aging_ptr(lchip, SYS_AGING_DOMAIN_TCAM, key_index, aging_ptr));
        return CTC_E_NONE;
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

    sys_usw_ftm_query_table_entry_num(lchip, DsAgingStatusTcam_t, &tcam_aging_num);
    tcam_aging_num = tcam_aging_num*32;

    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &fib_ctl));

    if (key_index < 32)
    {
        is_cam = 1;
    }
    else if ((key_index >= 32 )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex0_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum0_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex0_f, &fib_ctl) )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex1_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum1_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex1_f, &fib_ctl) )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex2_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum2_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex2_f, &fib_ctl) )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex3_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum3_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex3_f, &fib_ctl) )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex4_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum4_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex4_f, &fib_ctl) )
        && (key_index < GetFibAccelerationCtl(V, demarcationIndex5_f, &fib_ctl)))
    {
        current_level = GetFibAccelerationCtl(V, levelNum5_f, &fib_ctl);
    }
    else if ((key_index >= GetFibAccelerationCtl(V, demarcationIndex5_f, &fib_ctl)))
    {
        current_level = 6;
    }
    else
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[Aging]The key-index is out of range! ");
        return CTC_E_HW_INVALID_INDEX;
    }


    if (is_cam)
    {
        *aging_ptr = key_index;
    }
    else
    {
        uint8 size_mode = 0;
        temp_idx = key_index - 32;

        size_mode = GetFibAccelerationCtl(V, host0HashSizeMode_f, &fib_ctl);

        switch (current_level)
        {
            case 0:
            case 1:
            case 2:
                *aging_ptr = (size_mode? (temp_idx&0x7fff):(temp_idx&0x3fff)) + 32 + 32*1024*current_level;
                break;
            case 3:
            case 4:
            case 5:
                *aging_ptr = (size_mode? (temp_idx&0x3fff):(temp_idx&0x1fff)) + 32 + 32*1024*3 + 16*1024*(current_level-3);
                break;
            case 6:
            {
                temp_idx = temp_idx -  GetFibAccelerationCtl(V, externalMacIndexBase_f, &fib_ctl);
                if (GetFibAccelerationCtl(V, host0ExternalHashSizeMode_f, &fib_ctl))
                {
                    *aging_ptr = temp_idx + GetFibAccelerationCtl(V, externalMacAgingIndexBase_f, &fib_ctl);  /* external, externalMacAgingIndexBase: 155776*/
                }
                else
                {
                    *aging_ptr = (temp_idx >> 15)*64*1024 + (temp_idx&0x7FFF) + GetFibAccelerationCtl(V, externalMacAgingIndexBase_f, &fib_ctl);
                }

            }

            default:
                break;
        }

    }

    if (SYS_AGING_DOMAIN_TCAM != domain_type)
    {
        *aging_ptr += tcam_aging_num;
    }

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] index to AgingPtr:0x%x \n", *aging_ptr);

    return CTC_E_NONE;
}

/*
 +-----------------------+
 |   TCAM       (8K)     |
 |-----------------------|0
 |   MAC CAM    (32)     |32-1
 |-----------------------|32
 |   MAC0/FLOW0/IP3(16K) |32+32K-1
 |-----------------------|32+64K
 |   MAC1/FLOW1 (16K)    |32+64K+32K-1
 |-----------------------|32+64K+64K
 |   MAC2/FLOW2 (16K)    |32+64K+64K+32K-1
 |-----------------------|32+64K+64K+64K
 |   MAC3/FLOW3 (8K)     |32+64K+64K+64K+32K-1
 |-----------------------|32+64K+64K+64K+64K
 |   MAC4       (8K)     |32+64K+64K+64K+64K+8K-1
 |-----------------------|
 |   MAC5       (8k)     |
 |-----------------------|
 |   flow Cam   (32)     |
 |-----------------------|
 |   host1_da Cam  (16)  |
 |-----------------------|
 |   host1_sa Cam  (16)  |
 +-----------------------+
*/


int32
sys_usw_aging_get_key_index(uint8 lchip, uint32 aging_ptr, uint8* domain_type, uint32* key_index)
{
    uint32 aging_idx = 0;
    uint8  level_id = 0;
    uint32 level_used[6] = {0};
    FibAccelerationCtl_m fib_ctl;
    uint32 cmd = 0;
    uint32 hash_en = 0;
    uint32 couple_mode = 0;
    uint32 tcam_aging_num = 0;
    uint8  step = FibAccelerationCtl_host0Level1HashEn_f-FibAccelerationCtl_host0Level0HashEn_f;
    CTC_PTR_VALID_CHECK(domain_type);
    CTC_PTR_VALID_CHECK(key_index);



   sys_usw_ftm_query_table_entry_num(lchip, DsAgingStatusTcam_t, &tcam_aging_num);
   tcam_aging_num = tcam_aging_num*32;

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0,cmd, &fib_ctl));

    for (level_id = 0; level_id < 6; level_id++)
    {
        DRV_IOR_FIELD(lchip, FibAccelerationCtl_t, FibAccelerationCtl_host0Level0HashEn_f+step*level_id, &hash_en, &fib_ctl);
        DRV_IOR_FIELD(lchip, FibAccelerationCtl_t, FibAccelerationCtl_host0HashSizeMode_f+step*level_id, &couple_mode, &fib_ctl);

        if (level_id <= 3)
        {
            level_used[level_id] = hash_en?(couple_mode?0:NUM_16K):NUM_32K;
        }
        else
        {
            level_used[level_id] = hash_en?(couple_mode?0:NUM_8K):NUM_16K;
        }
    }

    if (aging_ptr < tcam_aging_num)
    {
        /*Tcam aging*/
        *domain_type = SYS_AGING_DOMAIN_TCAM;
        *key_index   = aging_ptr;
    }
    else
    {
        aging_idx = aging_ptr - tcam_aging_num;

        if (aging_idx < 32)
        {
            /*Host0 cam*/
            *domain_type = SYS_AGING_DOMAIN_MAC_HASH;
            *key_index   = aging_idx;
        }
        else
        {
            *domain_type = SYS_AGING_DOMAIN_MAC_HASH;

            aging_idx = aging_idx - 32;
            level_id = aging_idx/NUM_32K;
            if (level_id == 0)
            {
                /*Level 0 32K */
                *key_index   = aging_idx;
            }
            else if (level_id == 1)
            {
                /*Level 1 32K */
                *key_index   = aging_idx - level_used[0];
            }
            else if (level_id == 2)
            {
                /*Level 2 32K */
                *key_index   = aging_idx - level_used[0] - level_used[1];
            }
            else
            {
                level_id = aging_idx/NUM_16K;
                if (level_id == 6)
                {
                    /*Level 3 16K*/
                    *key_index   = aging_idx - level_used[0] - level_used[1] - level_used[2];
                }
                else if (level_id == 7)
                {
                    /*Level 4 16K*/
                    *key_index   = aging_idx - level_used[0] - level_used[1] - level_used[2] - level_used[3];
                }
                else if (level_id == 8)
                {
                    /*level 5 16K*/
                    *key_index   = aging_idx - level_used[0] - level_used[1] - level_used[2] - level_used[3] - level_used[4];
                }
            }
        }
    }

    return CTC_E_NONE;
}

/**
   @brief This function is to set the aging properties
 */
int32
sys_usw_aging_set_property(uint8 lchip, ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32 value)
{
    uint32 cmd         = 0;
    uint16 field       = 0;
    uint8  timer_index = 0;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "[Aging] Set aging property:tbl_type %d, aging_prop:%d, value:%d\n", tbl_type, aging_prop, value);

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    if (tbl_type == CTC_AGING_TBL_MAC)
    {
        timer_index = SYS_AGING_TIMER_INDEX_MAC;
    }
    else
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    switch (aging_prop)
    {
    case CTC_AGING_PROP_FIFO_THRESHOLD:
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    case CTC_AGING_PROP_INTERVAL:

        return(_sys_usw_aging_set_aging_interval(lchip, timer_index, value));

    case CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED:
        field = IpeAgingCtl_gTimmer_0_stopOnMaxPtr_f;
        value = !!value;
        break;

    case CTC_AGING_PROP_AGING_SCAN_EN:
        field = IpeAgingCtl_gTimmer_0_scanEn_f + (timer_index - 1)*4;
        value = !!value;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd       = DRV_IOW(IpeAgingCtl_t, field);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));


    return CTC_E_NONE;
}

/**
   @brief This function is to get the aging properties
 */
int32
sys_usw_aging_get_property(uint8 lchip, ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32* value)
{
    uint32 cmd         = 0;
    uint16 field       = 0;
    uint8  timer_index = 0;
    uint32 core_frequency = 0;
    uint64 temp_val = 0;
    uint64 temp = 0;

    CTC_PTR_VALID_CHECK(value);
    SYS_LEARNING_AGING_INIT_CHECK(lchip);

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "[Aging] Get aging property:tbl_type %d, aging_prop:%d\n", tbl_type, aging_prop);

    if (tbl_type == CTC_AGING_TBL_MAC)
    {
        timer_index = SYS_AGING_TIMER_INDEX_MAC;
    }
    else
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;
    }

    switch (aging_prop)
    {
    case CTC_AGING_PROP_FIFO_THRESHOLD:
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;


    case CTC_AGING_PROP_INTERVAL:

        field = IpeAgingCtl_gTimmer_0_agingInterval_f + (timer_index - 1)*4;
        break;

    case CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED:
        field = IpeAgingCtl_gTimmer_0_stopOnMaxPtr_f + (timer_index - 1)*4;
        break;

    case CTC_AGING_PROP_AGING_SCAN_EN:
        field = IpeAgingCtl_gTimmer_0_scanEn_f + (timer_index - 1)*4;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }


    cmd       = DRV_IOR(IpeAgingCtl_t, field);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));

    if (aging_prop == CTC_AGING_PROP_INTERVAL)
    {
        core_frequency = sys_usw_get_core_freq(lchip, 0) * 1000000/DOWN_FRE_RATE;
        temp_val = (*value);
        temp = (temp_val+1)*(pla_master[lchip]->aging_max_ptr-pla_master[lchip]->aging_min_ptr);
        *value = (temp-1)/core_frequency;
    }
    return CTC_E_NONE;
}

int32
sys_usw_aging_output_aging_status_by_ptr(uint8 lchip, uint32 domain_type, uint32 aging_ptr, uint8 *status)
{
    DsAgingStatus_m ds_aging_status;
    DsAging_m        ds_aging;
    uint32            status_tbl = 0;
    uint32            status_ptr = 0;
    uint32            aging_status = 0;
    uint32           tcam_aging_num = 0;

   sys_usw_ftm_query_table_entry_num(lchip, DsAgingStatusTcam_t, &tcam_aging_num);
   tcam_aging_num = tcam_aging_num*32;

    status_ptr = (SYS_AGING_DOMAIN_TCAM == domain_type) ? aging_ptr : (aging_ptr - tcam_aging_num);
    status_tbl = (SYS_AGING_DOMAIN_TCAM == domain_type) ? DsAgingStatusTcam_t : DsAgingStatusFib_t;

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, status_ptr >> 5,
                               DRV_IOR(status_tbl, DRV_ENTRY_FLAG), &ds_aging_status));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOR(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));
    aging_status = DRV_GET_FIELD_V(lchip, status_tbl, DsAgingStatus_array_0_agingStatus_f + (status_ptr & 0x1f), &ds_aging_status);

    *status = (uint8)aging_status;

    return CTC_E_NONE;
}
#if 0
int32
sys_usw_aging_set_aging_status_by_ptr(uint8 lchip, uint32 domain_type, uint32 aging_ptr, bool enable)
{
    DsAgingStatus_m ds_aging_status;
    DsAging_m        ds_aging;
    uint32            status_tbl = 0;
    uint32            status_ptr = 0;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    status_ptr = (SYS_AGING_DOMAIN_TCAM == domain_type) ? aging_ptr : (aging_ptr - SYS_TCAM_AGING_NUM);
    status_tbl = (SYS_AGING_DOMAIN_TCAM == domain_type) ? DsAgingStatusTcam_t : DsAgingStatusFib_t;


    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] Set aging status-- AgingPtr:0x%x \n", aging_ptr);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, status_ptr >> 5,
                               DRV_IOR(status_tbl, DRV_ENTRY_FLAG), &ds_aging_status));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOR(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));

    if (enable)
    {
        /* always 0. leave the control to global. */
        DRV_SET_FIELD_V(lchip, DsAging_t, DsAging_array_0_hardwareAgingEn_f+ (aging_ptr & 0x1f) * 2, &ds_aging, 0);
        DRV_SET_FIELD_V(lchip, status_tbl, DsAgingStatus_array_0_agingStatus_f + (status_ptr & 0x1f), &ds_aging_status, 1);
    }
    else
    {
        DRV_SET_FIELD_V(lchip, status_tbl, DsAgingStatus_array_0_agingStatus_f + (status_ptr & 0x1f), &ds_aging_status, 0);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, status_ptr >> 5,
                               DRV_IOW(status_tbl, DRV_ENTRY_FLAG), &ds_aging_status));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOW(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));

    return CTC_E_NONE;
}

#endif
int32
sys_usw_aging_set_aging_status(uint8 lchip, sys_aging_domain_type_t domain_type, uint32 key_index, uint8 timer, uint8 status)
{
    uint8 hit = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;
    uint32 tbl_id = DsAging_t;
    uint32 cmd1 = DRV_IOW(IpeAgingScanRangeCtl_t, IpeAgingScanRangeCtl_gTimmer_1_maxPtr_f);
    uint32 value = 1;
    int32 ret    =  CTC_E_NONE;
    uint32 aging_ptr = 0;
    uint8 use_aging_ptr = (domain_type == SYS_AGING_DOMAIN_HOST1)?1:0;
    drv_work_platform_type_t work_plarform;
    uint32 aging_tcam_base = 0;

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] key_index:%d, domain_type:%d, timer:%d, status:%u\n",
                               key_index, domain_type, timer, status);
    SYS_LEARNING_AGING_INIT_CHECK(lchip);

    use_aging_ptr = (((domain_type == SYS_AGING_DOMAIN_IP_HASH)&& !DRV_IS_DUET2(lchip))||((domain_type == SYS_AGING_DOMAIN_TCAM))) ? 1 : use_aging_ptr; /*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    aging_tcam_base = DRV_IS_DUET2(lchip) ? 8*1024 : 16*1024;
    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    if (use_aging_ptr)
    {
        if (domain_type == SYS_AGING_DOMAIN_HOST1)
        {
            _sys_usw_aging_get_host1_aging_ptr(lchip, key_index, &aging_ptr);
        }
        else if (domain_type == SYS_AGING_DOMAIN_IP_HASH) /*SYSTEM MODIFIED by taocy for SDK bug 107372*/
        {
            _sys_usw_aging_get_host0_aging_ptr(lchip, key_index, &aging_ptr);
        }
		/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
        else if (domain_type == SYS_AGING_DOMAIN_TCAM)
        {
            _sys_usw_aging_get_tcam_aging_ptr(lchip, domain_type, key_index, &aging_ptr);
        }
		/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
        CTC_BIT_SET(in.flag, DRV_ACC_AGING_EN);
    }
    drv_get_platform_type(lchip, &work_plarform);

    sal_mutex_lock(pla_master[lchip]->p_aging_mutex);
    if (!status && (DRV_IS_DUET2(lchip) || (DRV_IS_TSINGMA(lchip)&&(domain_type == SYS_AGING_DOMAIN_TCAM))) && work_plarform == HW_PLATFORM) /*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    {
        uint16 loop ;
        uint32 cmdr_scan = DRV_IOR(IpeAgingCtl_t, IpeAgingCtl_gTimmer_1_scanEn_f);
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_aging_get_aging_status(lchip, domain_type, key_index, &hit), pla_master[lchip]->p_aging_mutex);
        if(hit == 0)
        {
            sal_mutex_unlock(pla_master[lchip]->p_aging_mutex);
            return CTC_E_NONE;
        }
        if (domain_type != SYS_AGING_DOMAIN_TCAM) /*SYSTEM MODIFIED by taocy for SDK bug 107372*/
        {
            in.type = DRV_ACC_TYPE_UPDATE;
            in.op_type = DRV_ACC_OP_BY_INDEX;
            in.index = use_aging_ptr?(aging_ptr-8192):key_index;
            in.timer_index = SYS_AGING_TIMER_INDEX_RSV;
            in.data = &status;
            in.tbl_id = tbl_id;

            CTC_ERROR_RETURN_WITH_UNLOCK(drv_acc_api(lchip, &in, &out), pla_master[lchip]->p_aging_mutex);
        }
		/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
        else
        {
            uint32 cmd = 0;
            uint32 value = SYS_AGING_TIMER_INDEX_RSV;
            uint8 step = 0;
            step = DsAging_array_1_agingIndex_f - DsAging_array_0_agingIndex_f;
            cmd = DRV_IOW(DsAging_t, DsAging_array_0_agingIndex_f + (aging_ptr%32)*step);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, aging_ptr>>5, cmd, &value), ret, error_proc);
        }
		/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, DRV_IOW(IpeAgingCtl_t, IpeAgingCtl_gTimmer_1_scanEn_f), &value), ret, error_proc);

        value = 1;
        for(loop=0; loop < 0xffff; loop++)
        {
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmdr_scan, &value), ret, error_proc);
            if(value == 0)
            {
                break;
            }
        }

        CTC_ERROR_GOTO(sys_usw_aging_get_aging_status(lchip, domain_type, key_index, &hit), ret, error_proc);
        if(hit == 1)
        {
            SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[Aging] Set Aging status error, key index:0x%x, timer scan en:%d\n",key_index, value);
            ret = CTC_E_HW_FAIL;
        }
    }
error_proc:
    /*Ensure aging ptr is min ptr*/
    DRV_IOCTL(lchip, 0, cmd1, &pla_master[lchip]->aging_min_ptr);
    DRV_IOCTL(lchip, 0, cmd1, &pla_master[lchip]->aging_max_ptr);

    if (domain_type != SYS_AGING_DOMAIN_TCAM)/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    {
        in.type = DRV_ACC_TYPE_UPDATE;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.index = use_aging_ptr?(aging_ptr - aging_tcam_base):key_index;
        in.timer_index = timer;
        in.data = &status;
        in.tbl_id = tbl_id;
        ret = drv_acc_api(lchip, &in, &out);
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    else
    {
        uint32 cmd = 0;
        uint32 value = 0;
        uint8 step = 0;
        step = DsAging_array_1_agingIndex_f - DsAging_array_0_agingIndex_f;
        cmd = DRV_IOW(DsAging_t, DsAging_array_0_agingIndex_f + (aging_ptr%32)*step);
        DRV_IOCTL(lchip, aging_ptr >> 5, cmd, &value);
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

    sal_mutex_unlock(pla_master[lchip]->p_aging_mutex);
    return ret;
}

int32
sys_usw_aging_get_aging_status(uint8 lchip, sys_aging_domain_type_t domain_type, uint32 key_index, uint8* hit)
{
    uint32  aging_ptr  = 0;
    uint8   status = 0;
    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_aging_get_aging_ptr(lchip, domain_type, key_index, &aging_ptr));
    CTC_ERROR_RETURN(sys_usw_aging_output_aging_status_by_ptr(lchip, domain_type, aging_ptr, &status));

    if (NULL != hit)
    {
        *hit = status;
    }

    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "[Aging] key_index:%d, domain_type:%d, hit:%d\n",
                               key_index, domain_type, hit ? *hit : 0);

    return CTC_E_NONE;
}


#if 0
int32
sys_usw_aging_set_aging_timer(uint8 lchip, uint32 key_index, uint8 timer)
{
    uint32            aging_ptr  = 0;
    DsAging_m        ds_aging;

    CTC_ERROR_RETURN(sys_usw_aging_get_aging_ptr(lchip, SYS_AGING_DOMAIN_MAC_HASH, key_index, &aging_ptr));

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOR(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));
    DRV_SET_FIELD_V(lchip, DsAging_t, DsAging_array_0_agingIndex_f + (aging_ptr & 0x1f) * 2, &ds_aging, timer);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOW(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));

    return CTC_E_NONE;

}
#endif
int32
sys_usw_aging_get_aging_timer(uint8 lchip, uint8 domain_type, uint32 key_index, uint8* p_timer)
{
    uint32            aging_ptr  = 0;
    DsAging_m        ds_aging;
    SYS_LEARNING_AGING_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_aging_get_aging_ptr(lchip, domain_type, key_index, &aging_ptr));

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOR(DsAging_t, DRV_ENTRY_FLAG), &ds_aging));
    *p_timer = DRV_GET_FIELD_V(lchip, DsAging_t, DsAging_array_0_agingIndex_f + (aging_ptr & 0x1f) * 2, &ds_aging);

    return CTC_E_NONE;
}



#define ___LEARNING_AGING___

/* this callback is for sample code. specially for testing HW learning send DMA. */
#if 0
int32
sys_usw_learning_aging_register_cb2(uint8 lchip, sys_learning_aging_fn_t cb)
{
    pla_master[lchip]->hw_dma_cb    = cb;

    return CTC_E_NONE;
}


int32
sys_usw_learning_aging_register_cb(uint8 lchip, ctc_learning_aging_fn_t cb, void* user_data)
{
    pla_master[lchip]->sw_dma_cb    = cb;
    pla_master[lchip]->user_data = user_data;

    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_get_learn_cb(uint8 lchip, void **cb, void** user_data)
{
    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    *user_data = pla_master[lchip]->user_data;
    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_L2_SW_LEARNING, (CTC_INTERRUPT_EVENT_FUNC*)cb));
    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_get_aging_cb(uint8 lchip, void **cb, void** user_data)
{
    SYS_LEARNING_AGING_INIT_CHECK(lchip);
    *user_data = pla_master[lchip]->user_data;
    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_L2_SW_AGING, (CTC_INTERRUPT_EVENT_FUNC*)cb));
    return CTC_E_NONE;
}


#endif
#define __SYNC_DMA_INFO__
STATIC int32
_sys_usw_learning_aging_decode_leaning_data(uint8 lchip, void* p_info, ctc_learning_cache_entry_t* p_entry)
{
    DmaFibLearnFifo_m* p_learning = (DmaFibLearnFifo_m*)p_info;
    hw_mac_addr_t             mac_sa   = { 0 };

    GetDmaFibLearnFifo(A, macSa_f, p_learning, mac_sa);
    SYS_USW_SET_USER_MAC(p_entry->mac, mac_sa);
    p_entry->fid                = GetDmaFibLearnFifo(V, vsiId_f, p_learning);

    if (!GetDmaFibLearnFifo(V, isGlobalSrcPort_f, p_learning))
    {
        p_entry->logic_port = GetDmaFibLearnFifo(V, learningSourcePort_f, p_learning);
        p_entry->is_logic_port = 1;
    }
    else
    {
        p_entry->is_logic_port = 0;
        p_entry->global_src_port = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaFibLearnFifo(V, learningSourcePort_f, p_learning));
    }

    p_entry->cvlan_id = GetDmaFibLearnFifo(V, uShare_gLearning_oldCvlanId_f, p_learning);
    p_entry->svlan_id = GetDmaFibLearnFifo(V, uShare_gLearning_oldSvlanId_f, p_learning);

    p_entry->ether_oam_md_level = GetDmaFibLearnFifo(V, uShare_gLearning_etherOamLevel_f, p_learning);
    p_entry->is_ether_oam       = GetDmaFibLearnFifo(V, uShare_gLearning_isEtherOam_f, p_learning);

    p_entry->mac_sa_0to31 = (p_entry->mac[5] | (p_entry->mac[4] << 8) |
        (p_entry->mac[3] << 16) | (p_entry->mac[2] << 24));
    p_entry->mac_sa_32to47 = (p_entry->mac[1] | (p_entry->mac[0] << 8));
    p_entry->is_hw_sync =  GetDmaFibLearnFifo(V, fastLearningEn_f, p_learning);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_learning_aging_decode_aging_data(uint8 lchip, void* p_info, ctc_aging_info_entry_t* p_entry, uint8* is_ip)
{
    DmaFibLearnFifo_m* p_aging = (DmaFibLearnFifo_m*)p_info;
    uint8   hash_type = 0;
    uint32  dsmac_index = 0;
    DsMac_m dsmac;
    uint32 cmd = 0;

    hw_mac_addr_t             mac_sa   = { 0 };
    mac_addr_t mac_tmp;

    hash_type = GetDmaFibLearnFifo(V, uShare_gAging_hashType_f, p_aging);

    if (hash_type == FIBHOST0PRIMARYHASHTYPE_MAC)
    {
        /*1.1 mac aging */
        GetDmaFibLearnFifo(A, macSa_f, p_aging, mac_sa);
        SYS_USW_SET_USER_MAC(p_entry->mac, mac_sa);
        p_entry->fid                = GetDmaFibLearnFifo(V, vsiId_f, p_aging);
        p_entry->is_hw_sync        = GetDmaFibLearnFifo(V, uShare_gAging_hwAging_f, p_aging);
        p_entry->aging_type = CTC_AGING_TYPE_MAC;
        *is_ip = 0;

        dsmac_index   = GetDmaFibLearnFifo(V, uShare_gAging_dsAdIndex_f, p_aging);
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsmac_index, cmd, &dsmac));

        p_entry->is_logic_port = GetDsMac(V, learnSource_f, &dsmac);
        if (p_entry->is_logic_port)
        {
            p_entry->gport = GetDsMac(V, logicSrcPort_f, &dsmac);
        }
        else
        {
            p_entry->gport = GetDsMac(V, globalSrcPort_f, &dsmac);
        }
    }
    #if 0
    else if(hash_type == FIBHOST0PRIMARYHASHTYPE_IPV4)
    {
        /*1.2 Ipv4 aging */
        aging_ptr = GetDmaFibLearnFifo(V, uShare_gAging_agingPtr_f, p_aging);
        CTC_ERROR_RETURN(sys_usw_aging_get_key_index(lchip, aging_ptr, &domain_type, &key_index));
        if (domain_type != SYS_AGING_DOMAIN_MAC_HASH)
        {
             return CTC_E_INVALID_PARAM;
        }

        fib_acc_in.tbl_id = DsFibHost0Ipv4HashKey_t;
        fib_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        fib_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        fib_acc_in.index = key_index+32;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));

        p_entry->aging_type = CTC_AGING_TYPE_IPV4_UCAST;
        p_entry->ip_da.ipv4 = GetDsFibHost0Ipv4HashKey(V, ipDa_f, &(fib_acc_out.data));
        p_entry->vrf_id = GetDsFibHost0Ipv4HashKey(V, vrfId_f, &(fib_acc_out.data));
        p_entry->masklen = 32;
        *is_ip = 1;
    }
    else if(hash_type == FIBHOST0PRIMARYHASHTYPE_IPV6UCAST)
    {
        /*1.3 Ipv6 aging */
        aging_ptr = GetDmaFibLearnFifo(V, uShare_gAging_agingPtr_f, p_aging);
        CTC_ERROR_RETURN(sys_usw_aging_get_key_index(lchip, aging_ptr, &domain_type, &key_index));
        if (domain_type != SYS_AGING_DOMAIN_MAC_HASH)
        {
            return CTC_E_INVALID_PARAM;
        }

        fib_acc_in.tbl_id = DsFibHost0Ipv6UcastHashKey_t;
        fib_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        fib_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
        fib_acc_in.index = key_index+32;
        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));

        p_entry->aging_type = CTC_AGING_TYPE_IPV6_UCAST;
        GetDsFibHost0Ipv6UcastHashKey(A, ipDa_f, &(fib_acc_out.data), &(p_entry->ip_da.ipv6));
        p_entry->vrf_id = GetDsFibHost0Ipv6UcastHashKey(V, vrfId_f, &(fib_acc_out.data));
        p_entry->masklen = 128;

        temp = p_entry->ip_da.ipv6[0];
        p_entry->ip_da.ipv6[0] = p_entry->ip_da.ipv6[3];
        p_entry->ip_da.ipv6[3] = temp;
        temp = p_entry->ip_da.ipv6[1];
        p_entry->ip_da.ipv6[1] = p_entry->ip_da.ipv6[2];
        p_entry->ip_da.ipv6[2] = temp;
        *is_ip = 1;
    }
    #endif
    else
    {
        SYS_USW_SET_USER_MAC(mac_tmp, mac_sa);
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Invalid Aging hash type:%d, mac: %.4x.%.4x.%.4x, fid:%d\n", hash_type,
           mac_tmp[0], mac_tmp[2], mac_tmp[4], GetDmaFibLearnFifo(V, vsiId_f, p_aging));

        /*Not support*/
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_sync_data(uint8 lchip, void* p_info)
{
    uint32                     index    = 0;
    uint32                     i        = 0;
    sys_dma_learning_info_t   * p_dma  = NULL;
    DmaFibLearnFifo_m      * p_fifo = NULL;
    static ctc_l2_addr_t l2_addr;
    static ctc_learning_cache_t ctc_learning;
    static ctc_aging_fifo_info_t ctc_aging;
    hw_mac_addr_t             mac_sa   = { 0 };
    uint8   is_aging = 0;
    uint8 is_ip_aging = 0;
    sys_dma_info_t* p_dma_info = NULL;
    uint8 gchip = 0;
    uint8 leaning_cnt = 0;
    uint8 aging_cnt = 0;
    int32 ret = 0;
    uint32 ad_idx = 0;
    CTC_INTERRUPT_EVENT_FUNC cb = NULL;

    CTC_PTR_VALID_CHECK(p_info);
    CTC_PTR_VALID_CHECK(pla_master[lchip]);
    SYS_LEARNING_AGING_INIT_CHECK(lchip);

    ctc_learning.sync_mode = 1;
    ctc_aging.sync_mode = 1;

    p_dma_info = (sys_dma_info_t*)p_info;

    p_dma = (sys_dma_learning_info_t *) p_dma_info->p_data;
    for (i = 0; i < p_dma_info->entry_num; i++)
    {
        if (i >= SYS_DMA_LEARN_AGING_SYNC_CNT)
        {
            return CTC_E_INVALID_PARAM;
        }

        p_fifo   = &p_dma[i].learn_info;
        is_aging  = !GetDmaFibLearnFifo(V, isLearning_f, p_fifo);

        if (!is_aging)
        {
            _sys_usw_learning_aging_decode_leaning_data(lchip, p_fifo, &(ctc_learning.learning_entry[leaning_cnt]));
            leaning_cnt++;
        }
        else
        {
            if ((GetDmaFibLearnFifo(V, uShare_gAging_hashType_f, p_fifo) == FIBHOST0PRIMARYHASHTYPE_MAC) &&
                GetDmaFibLearnFifo(V, isGlobalSrcPort_f, p_fifo))
            {
                /*for pending aging , sdk process direct , do not sync to sys*/
                l2_addr.fid = GetDmaFibLearnFifo(V, vsiId_f, p_fifo);
                ad_idx = GetDmaFibLearnFifo(V, uShare_gAging_dsAdIndex_f, p_fifo);

                GetDmaFibLearnFifo(A, macSa_f, p_fifo, mac_sa);
                SYS_USW_SET_USER_MAC(l2_addr.mac, mac_sa);
                ret = sys_usw_l2_remove_pending_fdb(lchip, &l2_addr, ad_idx);
                if (ret)
                {
                    SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Pending Aging fail!!\n");
                }
                continue;
            }

            _sys_usw_learning_aging_decode_aging_data(lchip, p_fifo, &(ctc_aging.aging_entry[aging_cnt]), &is_ip_aging);
            aging_cnt++;
        }
        index ++;
    }

    if (index == 0)
    {
        return CTC_E_NONE;
    }

    ctc_learning.entry_num = leaning_cnt;
    ctc_aging.fifo_idx_num = aging_cnt;

    /*ip aging is needed even if hw learning mode */
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));

    /* call event callback function */
    if (leaning_cnt)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_L2_SW_LEARNING, &cb))
        if (cb)
        {
            cb(gchip, &ctc_learning);
        }
    }

    if (aging_cnt)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_L2_SW_AGING, &cb))
        if (cb)
        {
            cb(gchip, &ctc_aging);
        }
    }

    return CTC_E_NONE;
}



STATIC int32
_sys_usw_learning_init(uint8 lchip, ctc_learn_aging_global_cfg_t* cfg)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    CTC_ERROR_RETURN(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_LERNING,
        sys_usw_learning_aging_sync_data));

     field_val = 1;
     cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_macLearningMode_f);
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
#if 0
    drv_usw_agent_register_cb(DRV_AGENT_CB_GET_LEARN, (void*)sys_usw_learning_aging_get_learn_cb);
    drv_usw_agent_register_cb(DRV_AGENT_CB_GET_AGING, (void*)sys_usw_learning_aging_get_aging_cb);
#endif
    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_init(uint8 lchip, ctc_learn_aging_global_cfg_t* cfg)
{
    if (pla_master[lchip])
    {
        return CTC_E_NONE;
    }

    MALLOC_ZERO(MEM_FDB_MODULE, pla_master[lchip], sizeof(sys_learning_aging_master_t));
    if (!pla_master[lchip])
    {
        SYS_LEARNING_AGING_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }
    CTC_ERROR_RETURN(_sys_usw_learning_init(lchip, cfg));
    CTC_ERROR_RETURN(_sys_usw_aging_init(lchip, cfg));

    return CTC_E_NONE;
}

int32
sys_usw_learning_aging_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == pla_master[lchip])
    {
        return CTC_E_NONE;
    }
    sal_mutex_destroy(pla_master[lchip]->p_aging_mutex);
    mem_free(pla_master[lchip]);

    return CTC_E_NONE;
}


