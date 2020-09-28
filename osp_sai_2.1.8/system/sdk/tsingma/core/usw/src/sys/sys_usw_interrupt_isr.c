/**
 @file sys_usw_interrupt_isr.c

 @date 2012-11-01

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_interrupt.h"
#include "ctc_usw_interrupt.h"
#include "sys_usw_chip.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_interrupt_priv.h"
#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
extern int32 sys_usw_mac_isr_cl73_complete_isr(uint8 lchip, void* p_data, void* p_data1);
extern int32 sys_usw_qos_fcdl_state_isr(uint8 lchip, uint8 mac_id);

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

/**
 @brief handle sup fatal interrupt
*/
int32
sys_usw_intr_isr_sup_fatal(uint8 lchip, uint32 intr, void* p_data)
{
    uint32* p_sup_fatal = p_data;   /* gb_sup_interrupt_fatal_t */
    sys_intr_type_t type;
    uint32 idx = 0;
    uint32 offset = 0;
    uint32 i = 0, j = 0;
    uint32 sub_bmp[CTC_INTR_STAT_SIZE] = {0};
    uint32 low_bmp[CTC_INTR_STAT_SIZE] = {0};
    uint8  gchip = 0;
    ctc_interrupt_abnormal_status_t abnormal_status;
    ctc_intr_status_t* p_status = &(abnormal_status.status);
    int32 ret = CTC_E_NONE;
    uint8  action_type = 0;
    uint8 ecc_or_parity = 0;

    tbls_id_t intr_tbl = MaxTblId_t;
    uint16 k=0;

    sal_memset(&abnormal_status, 0, sizeof(abnormal_status));
    type.intr = SYS_INTR_CHIP_FATAL;

    abnormal_status.type.intr = CTC_INTR_CHIP_FATAL;

    /* 2. get & clear sub interrupt status */
    for (i = 0; i < SYS_INTR_SUB_FATAL_MAX; i++)
    {
        idx = i / CTC_UINT32_BITS;
        offset = i % CTC_UINT32_BITS;

        if (0 == p_sup_fatal[idx])
        {
            /* bypass 32Bit 0 */
            i = (idx + 1) * CTC_UINT32_BITS-1;
            continue;
        }

        if (!CTC_IS_BIT_SET(p_sup_fatal[idx], offset))
        {
            continue;
        }
        type.sub_intr = i;
        type.low_intr = INVG;

        /* get sub interrupt status */
        sys_usw_interrupt_get_status(lchip, &type, sub_bmp);

        if (!sub_bmp[0] && !sub_bmp[1] && !sub_bmp[2])
        {
            continue;
        }

        for (j = 0; j < CTC_INTR_STAT_SIZE * CTC_UINT32_BITS; j++)
        {
            if(!CTC_BMP_ISSET(sub_bmp, j))
            {
                continue;
            }

            sal_memset(low_bmp, 0, sizeof(low_bmp));
            type.low_intr = j;

            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sup-intr:%d sub-intr:%d low-intr:%d\n",type.intr,type.sub_intr,type.low_intr);

            /*get low interrupt status*/
            ret = sys_usw_interrupt_get_status(lchip, &type,low_bmp);
            if (ret < 0)
            {
                continue;
            }

            if(low_bmp[0] || low_bmp[1] || low_bmp[2])
            {
                /* clear low interrupt status in one time */
                CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, low_bmp));

                /* process all low interrupt*/
                for(k=0;k<CTC_INTR_STAT_SIZE * CTC_UINT32_BITS; k++)
                {
                    if(!CTC_BMP_ISSET(low_bmp, k))
                    {
                        continue;
                    }

                    sys_usw_interrupt_get_info(lchip, &type, k, &intr_tbl, &action_type, &ecc_or_parity);

                    if ( ecc_or_parity)
                    {
                        /* function interrupt for process ecc or parity */
                        sys_usw_interrupt_proc_ecc_or_parity(lchip, intr_tbl, k);
                    }
                    else if (g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR])
                    {
                        ret = sys_usw_get_gchip_id(lchip, &gchip);
                        if (0 == ret)
                        {
                            abnormal_status.type.intr = CTC_INTR_CHIP_FATAL;
                            abnormal_status.type.sub_intr = i;
                            sal_memcpy(p_status->bmp,low_bmp,sizeof(uint32)*CTC_INTR_STAT_SIZE);
                            abnormal_status.type.low_intr = j;
                            abnormal_status.action = action_type;
                            g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR](gchip, &abnormal_status);
                        }
                    }
                }
            }
            else
            {
                sys_usw_interrupt_get_info(lchip, &type, j, &intr_tbl, &action_type, &ecc_or_parity);

                if ( ecc_or_parity)
                {
                    /* function interrupt for process ecc or parity */
                    sys_usw_interrupt_proc_ecc_or_parity(lchip, intr_tbl, j);
                }
                else if (g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR])
                {
                    ret = sys_usw_get_gchip_id(lchip, &gchip);
                    if (0 == ret)
                    {
                        abnormal_status.type.intr = CTC_INTR_CHIP_FATAL;
                        abnormal_status.type.sub_intr = i;
                        abnormal_status.type.low_intr = j;
                        abnormal_status.action = action_type;
                        g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR](gchip, &abnormal_status);
                    }
                }
            }
        }
        /*clear sub interrup status*/
        type.low_intr = INVG;
        CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, sub_bmp));
    }

    return CTC_E_NONE;
}

/**
 @brief handle sup normal interrupt
*/
int32
sys_usw_intr_isr_sup_normal(uint8 lchip, uint32 intr, void* p_data)
{
    uint32* p_sup_normal = (uint32*)p_data;
    sys_intr_type_t type;
    uint32 idx = 0;
    uint32 offset = 0;
    uint32 i = 0;
    uint32 j = 0;
    uint8 gchip = 0 ;
    ctc_interrupt_abnormal_status_t abnormal_status;
    ctc_intr_status_t* p_status = &(abnormal_status.status);
    int32 ret = CTC_E_NONE;
    uint32  sub_bmp[CTC_INTR_STAT_SIZE] = {0};
    uint32  low_bmp[CTC_INTR_STAT_SIZE] = {0};
    uint8   action_type = 0;
    uint8 ecc_or_parity = 0;
    tbls_id_t intr_tbl = MaxTblId_t;
    uint8 loop = 0;
    uint8 mac_id = 0;
     sal_memset(&abnormal_status, 0, sizeof(abnormal_status));

    type.intr = SYS_INTR_CHIP_NORMAL;

    /* 2. get & clear sub interrupt status */
    for (i = 0; i < SYS_INTR_SUB_NORMAL_MAX; i++)
    {
        idx = i / CTC_UINT32_BITS;
        offset = i % CTC_UINT32_BITS;

        if (0 == p_sup_normal[idx])
        {
            /* bypass 32Bit 0 */
            i = (idx + 1) * CTC_UINT32_BITS-1;
            continue;
        }

        if (!CTC_IS_BIT_SET(p_sup_normal[idx], offset))
        {
            continue;
        }
        type.sub_intr = i;
        type.low_intr = INVG;

        /* get sub interrupt status */
        sys_usw_interrupt_get_status(lchip, &type, sub_bmp);

        if (!sub_bmp[0] && !sub_bmp[1] && !sub_bmp[2])
        {
            continue;
        }

        for (j = 0; j < CTC_INTR_STAT_SIZE * CTC_UINT32_BITS; j++)
        {
            if(!CTC_BMP_ISSET(sub_bmp, j))
            {
                continue;
            }

            sal_memset(low_bmp, 0, sizeof(low_bmp));
            type.low_intr = j;

            /* get low interrupt status */
            ret = sys_usw_interrupt_get_status(lchip, &type, low_bmp);
            if (ret < 0)
            {
                continue;
            }
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sup-intr:%d sub-intr:%d low-intr:%d\n",type.intr,type.sub_intr,type.low_intr);

            if(low_bmp[0] || low_bmp[1] || low_bmp[2])
            {
                /*TBD, need to be optimized*/
                if (DRV_IS_DUET2(lchip) &&
                    ((type.sub_intr == SYS_INTR_SUB_NORMAL_RLM_HS && type.low_intr >= 42 && type.low_intr <= 65)
                || (type.sub_intr == SYS_INTR_SUB_NORMAL_RLM_CS && type.low_intr >= 8 && type.low_intr <= 23)))
                {
                    uint8  serdes_id = 0;
                    uint8  cl73_ok = 0;
                    /* clear low interrupt status in one time */
                    CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, low_bmp));

                    if(type.sub_intr == SYS_INTR_SUB_NORMAL_RLM_HS)
                    {
                        serdes_id = type.low_intr - 42;
                    }
                    else
                    {
                        serdes_id = 24 + type.low_intr - 8;
                    }

                     /* intAnLinkGood_f*/
                    if(CTC_BMP_ISSET(low_bmp, 1))
                    {
                        cl73_ok = 1;
                    }
                     /*intAnIncompatibleLink*/
                    else if(CTC_BMP_ISSET(low_bmp, 4))
                    {
                        cl73_ok = 0;
                    }
                    sys_usw_mac_isr_cl73_complete_isr(lchip, &serdes_id, &cl73_ok);
                }
                else if(type.sub_intr == SYS_INTR_SUB_NORMAL_RLM_MAC && type.low_intr <= 15 &&
                     (CTC_BMP_ISSET(low_bmp, 18)||CTC_BMP_ISSET(low_bmp, 19)||CTC_BMP_ISSET(low_bmp, 20)||CTC_BMP_ISSET(low_bmp, 21)))
                {
                    for(loop = 0; loop < 4; loop ++)
                    {
                        if(CTC_BMP_ISSET(low_bmp, (18+loop)))
                        {
                            mac_id = type.low_intr * 4 + loop;
                            sys_usw_qos_fcdl_state_isr(lchip, mac_id);
                        }
                    }
                    /* clear low interrupt status in one time */
                    CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, low_bmp));
                }
                else
                {
                    uint16 k=0;
                    /* clear low interrupt status in one time */
                    CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, low_bmp));
                    /* process all low interrupt*/
                    for(k=0;k<CTC_INTR_STAT_SIZE * CTC_UINT32_BITS; k++)
                    {
                        if(!CTC_BMP_ISSET(low_bmp, k))
                        {
                            continue;
                        }

                        sys_usw_interrupt_get_info(lchip, &type, k, &intr_tbl, &action_type, &ecc_or_parity);

                        if ( ecc_or_parity)
                        {
                            /* function interrupt for process ecc or parity */
                            sys_usw_interrupt_proc_ecc_or_parity(lchip, intr_tbl, k);
                        }
                        else if (g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR])
                        {
                            ret = sys_usw_get_gchip_id(lchip, &gchip);
                            if (0 == ret)
                            {
                                abnormal_status.type.intr = CTC_INTR_CHIP_NORMAL;
                                abnormal_status.type.sub_intr = i;
                                sal_memcpy(p_status->bmp,low_bmp,sizeof(uint32)*CTC_INTR_STAT_SIZE);
                                abnormal_status.type.low_intr = j;
                                abnormal_status.action = action_type;
                                g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR](gchip, &abnormal_status);
                            }
                        }
                    }
                }
            }
            else
            {
                type.low_intr = INVG;
                sys_usw_interrupt_get_info(lchip, &type, j, &intr_tbl, &action_type, &ecc_or_parity);
                if ( ecc_or_parity)
                {
                    /* function interrupt for process ecc or parity */
                    sys_usw_interrupt_proc_ecc_or_parity(lchip, intr_tbl, j);
                }
                else if (g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR])
                {
                    ret = sys_usw_get_gchip_id(lchip, &gchip);
                    if (0 == ret)
                    {
                        abnormal_status.type.intr = CTC_INTR_CHIP_NORMAL;
                        abnormal_status.type.sub_intr = i;
                         /*sal_memcpy(p_status->bmp,low_bmp,sizeof(uint32)*CTC_INTR_STAT_SIZE);*/
                        abnormal_status.type.low_intr = j;
                        abnormal_status.action = action_type;
                        g_usw_intr_master[lchip].event_cb[CTC_EVENT_ABNORMAL_INTR](gchip, &abnormal_status);
                    }
                }
            }
        }
        /*clear sub interrup status*/
        type.low_intr = INVG;
        CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, sub_bmp));
    }

    return CTC_E_NONE;
}

/* default interrupt configuration */
sys_intr_mapping_t sys_usw_intr_default[] =
{
    {0,    SYS_INTR_CHIP_FATAL,                     "Chip Fatal",             NULL},
    {0,    SYS_INTR_CHIP_NORMAL,                    "Chip Normal",            NULL},
    {0,    SYS_INTR_FUNC_PTP_TS_CAPTURE_FIFO,        "PTP Ts Capture",    NULL},
    {0,    SYS_INTR_FUNC_PTP_TOD_PULSE_IN,           "PTP TOD Pulse In",       NULL},
    {0,    SYS_INTR_FUNC_PTP_TOD_CODE_IN_RDY,       "PTP TOD Code Ready",  NULL},
    {0,    SYS_INTR_FUNC_PTP_SYNC_PULSE_IN,       "PTP Sync Pulse In",     NULL},
    {0,    SYS_INTR_FUNC_PTP_SYNC_CODE_IN_RDY,   "PTP Sync Code Ready",    NULL},
    {0,    SYS_INTR_FUNC_PTP_SYNC_CODE_IN_ACC,   "PTP Sync Code ACC",      NULL},
    {0,    SYS_INTR_FUNC_STATS_STATUS_ADDR,      "statistics ",         NULL},
    {0,    SYS_INTR_FUNC_MET_LINK_SCAN_DONE,     "HW Based APS Failover",  NULL},
    {0,    SYS_INTR_FUNC_MDIO_CHANGE_0,          "MDIO Change 0",         NULL},
    {0,    SYS_INTR_FUNC_MDIO_CHANGE_1,          "MDIO Change 1",         NULL},
    {0,    SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN,     "Chan Link Scan",              NULL},
    {0,    SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW,  "IPfix Useage",         NULL},
    {0,    SYS_INTR_FUNC_FIB_ACC_LEARN_OUT_FIFO,  "Fib Acc Learning FIFO",         NULL},
    {0,    SYS_INTR_FUNC_OAM_DEFECT_CACHE,       "OAM Defect",           NULL},

    {0,    SYS_INTR_FUNC_BSR_C2C_PKT_CONGESTION,       "BSR C2C Packet Congestion",           NULL},
    {0,    SYS_INTR_FUNC_BSR_TOTAL_CONGESTION,         "BSR Total Packet Congestion",           NULL},
    {0,    SYS_INTR_FUNC_BSR_OAM_CONGESTION,           "BSR OAM Congestion",           NULL},
    {0,    SYS_INTR_FUNC_BSR_DMA_CONGESTION,           "BSR DMA Congestion",           NULL},
    {0,    SYS_INTR_FUNC_BSR_CRITICAL_PKT_CONGESTION,  "BSR Critical Packet Congestion",           NULL},
    {INVG,    SYS_INTR_FUNC_OAM_AUTO_GEN,                 "OAM Auto Gen",           NULL},

    {0,    SYS_INTR_PCS_LINK_31_0,               "MAC PCS Link 0",         NULL},
    {0,    SYS_INTR_PCS_LINK_47_32,              "MAC PCS Link 1",         NULL},

    {0,    SYS_INTR_FUNC_MDIO_XG_CHANGE_0,          "MDIO XG Change 0",         NULL},
    {0,    SYS_INTR_FUNC_MDIO_XG_CHANGE_1,          "MDIO XG Change 1",         NULL},
    {0,    SYS_INTR_PCIE_BURST_DONE,             "PCIe Burst Done",      NULL},
    {0,    SYS_INTR_DMA,                       "DMA ",                NULL},
    {0,    SYS_INTR_MCU_SUP,                       "Mcu Sup Intr",                NULL},
    {0,    SYS_INTR_MCU_NET,                       "Mcu Net Intr ",                NULL}
};

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
ctc_intr_group_t sys_usw_intr_group_default[] =
{
 /*-    {0, 16, SAL_TASK_PRIO_DEF,  "Function normal group"}    //for pin*/
    {0, 0, SAL_TASK_PRIO_DEF,  "Function normal group"}    /*for msi*/
};

/**
 @brief get default interrupt global configuration
*/
int32
sys_usw_interrupt_get_default_global_cfg(uint8 lchip, sys_intr_global_cfg_t* p_intr_cfg)
{
    uint32 group_count = 0;
    uint32 intr_count = 0;

    /* use default global configuration in sys */
    group_count = COUNTOF(sys_usw_intr_group_default);
    intr_count = COUNTOF(sys_usw_intr_default);
    if (group_count > SYS_USW_MAX_INTR_GROUP)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (intr_count > CTC_INTR_MAX_INTR)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_intr_cfg->group_count = group_count;
    p_intr_cfg->intr_count = intr_count;
    sal_memcpy(&(p_intr_cfg->group), sys_usw_intr_group_default, sizeof(sys_usw_intr_group_default));
    sal_memcpy(&(p_intr_cfg->intr), sys_usw_intr_default, sizeof(sys_usw_intr_default));
    p_intr_cfg->intr_mode  = 0;

    return CTC_E_NONE;
}

