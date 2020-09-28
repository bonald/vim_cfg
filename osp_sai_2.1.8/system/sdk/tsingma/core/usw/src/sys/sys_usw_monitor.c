#if (FEATURE_MODE == 0)
/**
 @file sys_usw_monitor.c

 @date 2009-10-19

 @version v2.0

 The file contains all monitor APIs of sys layer
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_debug.h"
#include "ctc_error.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_efd.h"

#include "sys_usw_monitor.h"
#include "sys_usw_dma.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
#define SYS_MONITOR_MAX_CHANNEL 64
#define SYS_MONITOR_GET_CHANNEL(lport)  SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport)

#define SYS_MONITOR_LATENCY_MAX_THRD (0xFFFFFFFF << 5)
#define SYS_MONITOR_SC_NUM_MAX  4
#define SYS_MONITOR_BUFFER_MAX_THRD ((1<<14)-1)
#define SYS_MONITOR_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_monitor_master[lchip]){ \
            return CTC_E_NOT_INIT;\
        } \
    } while (0)

struct sys_monitor_master_s
{
     ctc_monitor_fn_t func;
     void * user_data;
     uint32 buffer_stats[SYS_MONITOR_MAX_CHANNEL][2];
     uint64 latency_stats[SYS_MONITOR_MAX_CHANNEL][8];
     uint32 mburst_stats[SYS_MONITOR_MAX_CHANNEL][8];
     uint32 mburst_timestamp[SYS_MONITOR_MAX_CHANNEL];
     uint32 mburst_thrd[8];
     uint32 totoal_stats;
     void* p_buffer;
};
typedef struct sys_monitor_master_s sys_monitor_master_t;
sys_monitor_master_t *p_usw_monitor_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

STATIC int32
_sys_usw_monitor_set_thrd(uint8 lchip, uint8 type, uint8 buffer_type, uint8 index, uint32 thrd, uint8 channel_id)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 step = 0;

    if (CTC_MONITOR_BUFFER == type)
    {
        field_val = thrd;
        switch(buffer_type)
        {
            case CTC_MONITOR_BUFFER_PORT:
                cmd = (index==0)?DRV_IOW(DsErmPortMbCfg_t, DsErmPortMbCfg_mbMinThrd_f):DRV_IOW(DsErmPortMbCfg_t, DsErmPortMbCfg_mbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
                break;

            case CTC_MONITOR_BUFFER_SC:
                step = ErmGlbMbCfg_sc_1_scMbMinThrd_f - ErmGlbMbCfg_sc_0_scMbMinThrd_f;
                cmd = (index==0)?DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_sc_0_scMbMinThrd_f + step * channel_id):
                                 DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_sc_0_scMbMaxThrd_f + step * channel_id);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;

            case CTC_MONITOR_BUFFER_C2C:
                cmd = (index==0)?DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_c2cMbMinThrd_f):DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_c2cMbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;
            case CTC_MONITOR_BUFFER_TOTAL:
                cmd = (index==0)?DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_totalMbMinThrd_f):DRV_IOW(ErmGlbMbCfg_t, ErmGlbMbCfg_totalMbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        uint16 threshold = 0;
        uint16 shift = 0;

        sys_usw_common_get_compress_near_division_value(lchip, thrd,
                                MCHIP_CAP(SYS_CAP_MONITOR_DIVISION_WIDE), MCHIP_CAP(SYS_CAP_MONITOR_SHIFT_WIDE), &threshold, &shift, 0);
        field_val = threshold;
        cmd = DRV_IOW(DsLatencyMonCtl_t, DsLatencyMonCtl_array_0_latencyThrdHigh_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        field_val = shift;
        cmd = DRV_IOW(DsLatencyMonCtl_t, DsLatencyMonCtl_array_0_latencyThrdShift_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_monitor_get_thrd(uint8 lchip, uint8 type, uint8 buffer_type,uint8 index, uint32* thrd, uint8 channel)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 step = 0;

    if (CTC_MONITOR_BUFFER == type)
    {
        switch(buffer_type)
        {
            case CTC_MONITOR_BUFFER_PORT:
                cmd = (index==0)?DRV_IOR(DsErmPortMbCfg_t, DsErmPortMbCfg_mbMinThrd_f): DRV_IOR(DsErmPortMbCfg_t, DsErmPortMbCfg_mbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel, cmd, &field_val));
                break;

            case CTC_MONITOR_BUFFER_SC:
                step = ErmGlbMbCfg_sc_1_scMbMinThrd_f - ErmGlbMbCfg_sc_0_scMbMinThrd_f;
                cmd = (index==0)?DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_sc_0_scMbMinThrd_f + step * channel):
                                 DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_sc_0_scMbMaxThrd_f + step * channel);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;

            case CTC_MONITOR_BUFFER_C2C:
                cmd = (index==0)?DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_c2cMbMinThrd_f): DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_c2cMbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;
            case CTC_MONITOR_BUFFER_TOTAL:
                cmd = (index==0)?DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_totalMbMinThrd_f): DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_totalMbMaxThrd_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
        *thrd = field_val;
    }
    else if(CTC_MONITOR_LATENCY == type)
    {
        uint32 threshold = 0;
        uint32 shift = 0;

        cmd = DRV_IOR(DsLatencyMonCtl_t, DsLatencyMonCtl_array_0_latencyThrdHigh_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &threshold));

        cmd = DRV_IOR(DsLatencyMonCtl_t, DsLatencyMonCtl_array_0_latencyThrdShift_f + index*2);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shift));
        *thrd = (threshold << shift);
    }
    else
    {
        *thrd = p_usw_monitor_master[lchip]->mburst_thrd[index];
    }

    return CTC_E_NONE;

}





STATIC int32
_sys_usw_monitor_get_timer_cfg(uint8 lchip, uint16 scan_timer, uint32* p_scan_interval,  uint32* p_max_ptr)
{
    uint32 delta = 1;
    uint32 core_freq = 0;
    uint32 min_scan_ptr = 0;

    core_freq = sys_usw_get_core_freq(lchip, 0);
    *p_scan_interval = (core_freq*5-1);
    *p_max_ptr = (((scan_timer*core_freq*1000) / delta) / (*p_scan_interval + 1)) + min_scan_ptr - 1;

    return CTC_E_NONE;
}



int32
sys_usw_monitor_set_timer(uint8 lchip, uint8 monitor_type, uint16 scan_timer)
{
    uint32 cmd = 0;
    ErmResrcMonScanCtl_m resrc_mon_scan_ctl;
    LatencyMonScanCtl_m latency_mon_scan_ctl;
    IrmResrcMonScanCtl_m irm_resrc_mon_scan_ctl;

    uint32 max_phy_ptr = 0;
    uint32 min_scan_ptr = 0;
    uint32 max_scan_ptr = 0;
    uint32 scan_interval = 0;

    sal_memset(&resrc_mon_scan_ctl, 0, sizeof(resrc_mon_scan_ctl));
    sal_memset(&latency_mon_scan_ctl, 0, sizeof(latency_mon_scan_ctl));
    if (monitor_type == CTC_MONITOR_BUFFER)
    {
        /*BufferMonitorTimer*/
        sal_memset(&resrc_mon_scan_ctl, 0, sizeof(resrc_mon_scan_ctl));
        max_phy_ptr = MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE) - 1;
        _sys_usw_monitor_get_timer_cfg(lchip, scan_timer, &scan_interval, &max_scan_ptr);
        if(max_scan_ptr > 0xFFFF)
        {
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &resrc_mon_scan_ctl));
        SetErmResrcMonScanCtl(V, resrcMonScanPtrMax_f , &resrc_mon_scan_ctl, max_scan_ptr-2);/*2 is adjust for precision*/
        SetErmResrcMonScanCtl(V, resrcMonScanInterval_f       , &resrc_mon_scan_ctl,  scan_interval);
        cmd = DRV_IOW(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &resrc_mon_scan_ctl));

        cmd = DRV_IOR(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_resrc_mon_scan_ctl));
        SetIrmResrcMonScanCtl(V, resrcMonScanPtrMax_f , &irm_resrc_mon_scan_ctl, max_scan_ptr-2);/*2 is adjust for precision*/
        SetIrmResrcMonScanCtl(V, resrcMonScanInterval_f       , &irm_resrc_mon_scan_ctl,  scan_interval);
        cmd = DRV_IOW(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_resrc_mon_scan_ctl));

    }
    else
    {
        /*LatencyMonitorTimer*/
        sal_memset(&latency_mon_scan_ctl, 0, sizeof(latency_mon_scan_ctl));
        max_phy_ptr = MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE) - 1;
        _sys_usw_monitor_get_timer_cfg(lchip, scan_timer, &scan_interval, &max_scan_ptr);
        if(max_scan_ptr > 0xFFFF)
        {
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &latency_mon_scan_ctl));
        SetLatencyMonScanCtl(V, scanMaxPhyPtr_f , &latency_mon_scan_ctl, max_phy_ptr);
        SetLatencyMonScanCtl(V, scanMinPtr_f     , &latency_mon_scan_ctl, min_scan_ptr);
        SetLatencyMonScanCtl(V, scanMaxPtr_f , &latency_mon_scan_ctl, max_scan_ptr-1);/*1 is adjust for precision*/
        SetLatencyMonScanCtl(V, scanInterval_f        , &latency_mon_scan_ctl, scan_interval);
        cmd = DRV_IOW(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &latency_mon_scan_ctl));
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_monitor_get_scan_timer(uint8 lchip, uint32 *scan_timer, uint32 scan_interval, uint32 max_ptr)
{
    uint32 delta = 1;
    uint32 core_freq = 0;
    uint32 min_scan_ptr = 0;
    uint32 value = 0;
    core_freq = sys_usw_get_core_freq(lchip, 0);
    value = (max_ptr - min_scan_ptr + 1)*(scan_interval + 1)*delta / (core_freq*1000);
    *scan_timer = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_monitor_get_mburst_level(uint8 lchip, uint8 *level, uint32 thrd)
{
    uint8 index = 0;
    for(index = 0; index <= MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL); index ++)
    {
        if(thrd < p_usw_monitor_master[lchip]->mburst_thrd[index] && index != 0)
        {
            *level = index - 1;
        }
        else if(thrd < p_usw_monitor_master[lchip]->mburst_thrd[index] && index == 0)
        {
            *level = 0xff;
        }
        else
        {
            *level = MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_monitor_thrd_check(uint8 lchip, uint8 level, uint32 thrd,uint8 type)
{
    uint32 threshold_min = 0;
    uint32 threshold_max = 0;
    if(level == 0)
    {
        if(type == CTC_MONITOR_GLB_CONFIG_LATENCY_THRD)
        {
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_LATENCY, 0,level+1, &threshold_max, 0));
        }
        else if(type == CTC_MONITOR_GLB_CONFIG_MBURST_THRD)
        {
            threshold_max = p_usw_monitor_master[lchip]->mburst_thrd[level + 1];
        }
        if(threshold_max != 0)
        {
            CTC_MAX_VALUE_CHECK(thrd, threshold_max);
        }

    }
    else if(level == MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL))
    {
        if(type == CTC_MONITOR_GLB_CONFIG_LATENCY_THRD)
        {
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_LATENCY, 0,level-1, &threshold_min, 0));
        }
        else if(type == CTC_MONITOR_GLB_CONFIG_MBURST_THRD)
        {
            threshold_min = p_usw_monitor_master[lchip]->mburst_thrd[level - 1];
        }
        if(threshold_min != 0)
        {
            CTC_MIN_VALUE_CHECK(thrd, threshold_min);
        }
    }
    else
    {
        if(type == CTC_MONITOR_GLB_CONFIG_LATENCY_THRD)
        {
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_LATENCY, 0,level-1, &threshold_min, 0));
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_LATENCY, 0,level+1, &threshold_max, 0));
        }
        else if(type == CTC_MONITOR_GLB_CONFIG_MBURST_THRD)
        {
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_MSG_MBURST_STATUS, 0,level-1, &threshold_min, 0));
            CTC_ERROR_RETURN(_sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_MSG_MBURST_STATUS, 0,level+1, &threshold_max, 0));
        }
        if(threshold_min && threshold_max)
        {
            CTC_VALUE_RANGE_CHECK(thrd, threshold_min, threshold_max);
        }
        else if(threshold_min)
        {
            CTC_MIN_VALUE_CHECK(thrd, threshold_min);
        }
        else if(threshold_max)
        {
            CTC_MAX_VALUE_CHECK(thrd, threshold_max);
        }
    }
    return CTC_E_NONE;
}
int32
sys_usw_monitor_get_timer(uint8 lchip, uint8 monitor_type, uint32 *scan_timer)
{
    uint32 cmd = 0;
    ErmResrcMonScanCtl_m resrc_mon_scan_ctl;
    LatencyMonScanCtl_m latency_mon_scan_ctl;
    uint32 max_scan_ptr = 0;
    uint32 scan_interval = 0;

    sal_memset(&resrc_mon_scan_ctl, 0, sizeof(resrc_mon_scan_ctl));
    sal_memset(&latency_mon_scan_ctl, 0, sizeof(latency_mon_scan_ctl));
    if (monitor_type == CTC_MONITOR_BUFFER)
    {
        /*BufferMonitorTimer*/
        sal_memset(&resrc_mon_scan_ctl, 0, sizeof(resrc_mon_scan_ctl));
        cmd = DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &resrc_mon_scan_ctl));
        max_scan_ptr = GetErmResrcMonScanCtl(V, resrcMonScanPtrMax_f , &resrc_mon_scan_ctl);
        scan_interval = GetErmResrcMonScanCtl(V, resrcMonScanInterval_f , &resrc_mon_scan_ctl);
        max_scan_ptr += 2; /*adjust for precision*/
        _sys_usw_monitor_get_scan_timer(lchip, scan_timer, scan_interval, max_scan_ptr);


    }
    else
    {
        /*LatencyMonitorTimer*/
        sal_memset(&latency_mon_scan_ctl, 0, sizeof(latency_mon_scan_ctl));
        cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &latency_mon_scan_ctl));
        max_scan_ptr = GetLatencyMonScanCtl(V, scanMaxPtr_f , &latency_mon_scan_ctl);
        max_scan_ptr += 1; /*adjust for precision*/
        _sys_usw_monitor_get_scan_timer(lchip, scan_timer, scan_interval, max_scan_ptr);
        scan_interval = GetLatencyMonScanCtl(V, scanInterval_f        , &latency_mon_scan_ctl);
        _sys_usw_monitor_get_scan_timer(lchip, scan_timer, scan_interval, max_scan_ptr);
    }
    return CTC_E_NONE;

}
#if 0
uint8
_sys_usw_monitor_channel_convert(uint8 lchip, uint8 chan_id)
{
    uint32 cmd = 0;
    uint8 temp = 0;
    uint32 max_scan_ptr = 0;
    uint32 scan_ptr = 0;
    QMgrEnqCtl_m q_mgr_enq_ctl;

    /*slice 0 do not need convert */
    if (chan_id < MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE))
    {
        return chan_id;
    }

    /*Get scan max buf ptr */
    cmd = DRV_IOR(QMgrEnqCtl_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, 0, cmd, &q_mgr_enq_ctl));
    max_scan_ptr = GetQMgrEnqCtl(V, maxBufScanPtr_f , &q_mgr_enq_ctl);

    scan_ptr = max_scan_ptr/2 + (chan_id - MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE));
    scan_ptr = scan_ptr & 0x3F;
    temp = MCHIP_CAP(SYS_CAP_MONITOR_MAX_CHAN_PER_SLICE) + scan_ptr;

    return temp;
}
#endif
int32
sys_usw_monitor_set_config(uint8 lchip, ctc_monitor_config_t *p_cfg)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 channel_id = 0;
    uint32 field_val = 0;
    uint32 value_tmp = 0;
    ErmResrcMonScanCtl_m ermresrc_mon_scan_ctl;
    IrmResrcMonScanCtl_m irmresrc_mon_scan_ctl;
    LatencyMonScanCtl_m latency_ctl;
    uint32 monitorirm_en[2] = {0};
    uint32 latency_en[2] = {0};
    uint32 monitorerm_en[2] = {0};

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MONITOR_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(p_cfg->sc, SYS_MONITOR_SC_NUM_MAX - 1);
    CTC_MAX_VALUE_CHECK(p_cfg->dir, CTC_EGRESS);

    sal_memset(&ermresrc_mon_scan_ctl, 0, sizeof(ermresrc_mon_scan_ctl));
    sal_memset(&irmresrc_mon_scan_ctl, 0, sizeof(irmresrc_mon_scan_ctl));
    sal_memset(&latency_ctl, 0, sizeof(latency_ctl));
    field_val = p_cfg->value?1:0;
    if((p_cfg->monitor_type == CTC_MONITOR_BUFFER && (p_cfg->buffer_type == CTC_MONITOR_BUFFER_PORT || p_cfg->buffer_type == CTC_MONITOR_BUFFER_QUEUE)
        && (CTC_MONITOR_CONFIG_MON_SCAN_EN == p_cfg->cfg_type || CTC_MONITOR_CONFIG_MON_INFORM_MIN == p_cfg->cfg_type || CTC_MONITOR_CONFIG_MON_INFORM_MAX == p_cfg->cfg_type || CTC_MONITOR_RETRIEVE_MBURST_STATS == p_cfg->cfg_type))
        || (p_cfg->monitor_type == CTC_MONITOR_LATENCY && CTC_MONITOR_CONFIG_MON_INFORM_MIN != p_cfg->cfg_type &&  CTC_MONITOR_CONFIG_MON_INFORM_MAX != p_cfg->cfg_type &&
            CTC_MONITOR_CONFIG_MON_INTERVAL != p_cfg->cfg_type))
    {
        gport = p_cfg->gport;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
        channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) )
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    switch(p_cfg->cfg_type)
    {
    case CTC_MONITOR_CONFIG_MON_INFORM_EN:
        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            if(p_cfg->buffer_type != CTC_MONITOR_BUFFER_PORT && (DRV_IS_DUET2(lchip)))
            {
                return CTC_E_NOT_SUPPORT;
            }
            cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_microBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_c2cMicroBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_totalMicroBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            field_val = p_cfg->value?15:0;
            cmd = DRV_IOW(ErmMiscCtl_t, ErmMiscCtl_scMicroBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyWatermarkEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        break;

    case CTC_MONITOR_CONFIG_MON_INFORM_MIN:
        field_val = p_cfg->value;
        if(p_cfg->buffer_type != CTC_MONITOR_BUFFER_PORT && (DRV_IS_DUET2(lchip)) &&
            p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER && p_cfg->buffer_type == CTC_MONITOR_BUFFER_SC)
        {
            CTC_MAX_VALUE_CHECK(field_val, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
            _sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,0, field_val, p_cfg->sc);
        }
        else if(p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            CTC_MAX_VALUE_CHECK(field_val, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
            _sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,0, field_val, channel_id);
        }
        else
        {
            field_val = field_val >> 5;
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_latencyInformMinThrd_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }

        break;

    case CTC_MONITOR_CONFIG_MON_INFORM_MAX:
        field_val = p_cfg->value;
        if(p_cfg->buffer_type != CTC_MONITOR_BUFFER_PORT && (DRV_IS_DUET2(lchip)) &&
            p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            return CTC_E_NOT_SUPPORT;
        }
        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER && p_cfg->buffer_type == CTC_MONITOR_BUFFER_SC)
        {
            CTC_MAX_VALUE_CHECK(field_val, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
            _sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,1, field_val, p_cfg->sc);
        }
        else if(p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            CTC_MAX_VALUE_CHECK(field_val, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
            _sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,1, field_val, channel_id);
        }
        else
        {
            field_val = field_val >> 5;
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_latencyInformMaxThrd_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }

        break;


    case CTC_MONITOR_CONFIG_LOG_EN:

        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            if(field_val == 0)
            {
                cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            }
            else
            {
                field_val = 0xff;
                cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            }
        }

        break;

    case CTC_MONITOR_CONFIG_LOG_THRD_LEVEL:
        if (p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            if(p_cfg->value)
            {
                CTC_BIT_SET(field_val, p_cfg->level);
            }
            else
            {
                CTC_BIT_UNSET(field_val, p_cfg->level);
            }
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }

        break;

    case CTC_MONITOR_CONFIG_MON_SCAN_EN:

        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            uint32 value[2];
            ds_t ds;
            /* ERM scan */
            cmd = p_cfg->dir ? DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG): DRV_IOR(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
            switch(p_cfg->buffer_type)
            {
            case CTC_MONITOR_BUFFER_PORT:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    GetErmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, ds, value);
                    if (p_cfg->value)
                    {
                        CTC_BIT_SET(value[channel_id >> 5], channel_id&0x1F);
                    }
                    else
                    {
                        CTC_BIT_UNSET(value[channel_id >> 5], channel_id&0x1F);
                    }
                    SetErmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, ds, value);
                }
                else
                {
                    GetIrmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, ds, value);
                    if (p_cfg->value)
                    {
                        CTC_BIT_SET(value[channel_id >> 5], channel_id&0x1F);
                    }
                    else
                    {
                        CTC_BIT_UNSET(value[channel_id >> 5], channel_id&0x1F);
                    }
                    SetIrmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, ds, value);
                }
                break;
            case CTC_MONITOR_BUFFER_SC:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    SetErmResrcMonScanCtl(V, resrcMonScanPushScEn_f, ds, field_val);
                }
                else
                {
                    SetIrmResrcMonScanCtl(V, resrcMonScanPushScEn_f, ds, field_val);
                }
                break;
            case CTC_MONITOR_BUFFER_TOTAL:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    SetErmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, ds, field_val);
                }
                else
                {
                    SetIrmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, ds, field_val);
                }
                break;
            case CTC_MONITOR_BUFFER_QUEUE:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    cmd = DRV_IOR(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortId_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_tmp));
                    if((field_val == 0 && value_tmp == channel_id)||(field_val == 1))
                    {
                        SetErmResrcMonScanCtl(V, resrcMonScanPushPortDetailedEn_f, ds, field_val);
                        cmd = DRV_IOW(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortEn_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                        cmd = DRV_IOW(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortId_f);
                        field_val = (uint32)channel_id;
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                    }
                }
                else
                {
                    return CTC_E_NOT_SUPPORT;
                }
                break;
            default:
                return CTC_E_INVALID_PARAM;
            }
            cmd = p_cfg->dir ? DRV_IOW(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG):DRV_IOW(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
        }
        else
        {
            uint32 value[2];
            ds_t ds;
            channel_id = channel_id&0x3F;

            cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
            GetLatencyMonScanCtl(A,  reportEn_f, ds, value);
            if (p_cfg->value)
            {
                CTC_BIT_SET(value[channel_id >> 5], channel_id&0x1F);
            }
            else
            {
                CTC_BIT_UNSET(value[channel_id >> 5], channel_id&0x1F);
            }
            SetLatencyMonScanCtl(A, reportEn_f, ds, value);
            cmd = DRV_IOW(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyActMonEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));

        }

        /*check channel monitor enable*/
        cmd = DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ermresrc_mon_scan_ctl));
        GetErmResrcMonScanCtl(A,  resrcMonScanPushChannelEn_f, &ermresrc_mon_scan_ctl, monitorerm_en);

        cmd = DRV_IOR(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irmresrc_mon_scan_ctl));
        GetIrmResrcMonScanCtl(A,  resrcMonScanPushChannelEn_f, &irmresrc_mon_scan_ctl, monitorirm_en);

        cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &latency_ctl));
        GetLatencyMonScanCtl(A,  reportEn_f, &latency_ctl, latency_en);


        if (monitorerm_en[0] || monitorerm_en[1] || monitorirm_en[0] || monitorirm_en[1] || latency_en[0] || latency_en[1] )
        {
            /*monitor scan enable should disable dma monitor timer*/
            CTC_ERROR_RETURN(sys_usw_dma_set_monitor_timer(lchip, 0));
        }
        else
        {
            /*monitor scan disable should enable dma monitor timer, timer cfg 1s for event export*/
            CTC_ERROR_RETURN(sys_usw_dma_set_monitor_timer(lchip, 1000));
        }

        break;

    case  CTC_MONITOR_CONFIG_MON_INTERVAL:
        CTC_ERROR_RETURN(sys_usw_monitor_set_timer(lchip, p_cfg->monitor_type, p_cfg->value));
        break;

    case  CTC_MONITOR_CONFIG_LANTENCY_DISCARD_EN:
        if(field_val == 0 && p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        else if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            field_val = 0xff;
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }
        break;

    case  CTC_MONITOR_CONFIG_LANTENCY_DISCARD_THRD_LEVEL:
        if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            if(p_cfg->value)
            {
                CTC_BIT_SET(field_val, p_cfg->level);
            }
            else
            {
                CTC_BIT_UNSET(field_val, p_cfg->level);
            }
            cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }
        break;
    case  CTC_MONITOR_RETRIEVE_LATENCY_STATS:
        if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            for (value_tmp = 0; value_tmp < 8; value_tmp++)
            {
                p_usw_monitor_master[lchip]->latency_stats[channel_id][value_tmp] = 0;
            }
        }
        break;
    case CTC_MONITOR_RETRIEVE_MBURST_STATS:
        if(p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            cmd = DRV_IOR(ErmMiscCtl_t, ErmMiscCtl_microBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            if(field_val == 0)
            {
                return CTC_E_INVALID_PARAM;
            }
            for (value_tmp = 0; value_tmp < 8; value_tmp++)
            {
                p_usw_monitor_master[lchip]->mburst_stats[channel_id][value_tmp] = 0;
            }
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_monitor_get_config(uint8 lchip, ctc_monitor_config_t *p_cfg)
{
    uint32 cmd = 0;
    uint16 lport = 0;
    uint32 gport = 0;
    uint8 channel_id = 0;
    uint32 value_tmp = 0;
    uint32 field_val = 0;

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MONITOR_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(p_cfg->sc,SYS_MONITOR_SC_NUM_MAX - 1);

    if(CTC_MONITOR_CONFIG_MON_INTERVAL != p_cfg->cfg_type && (p_cfg->buffer_type == CTC_MONITOR_BUFFER_PORT||
        p_cfg->buffer_type == CTC_MONITOR_BUFFER_QUEUE))
    {
        gport = p_cfg->gport;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
        channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) )
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    switch(p_cfg->cfg_type)
    {
    case CTC_MONITOR_CONFIG_MON_INFORM_EN:
        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            cmd = DRV_IOR(ErmMiscCtl_t, ErmMiscCtl_microBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
         p_cfg->value = field_val;
        break;
    case CTC_MONITOR_CONFIG_MON_INFORM_MIN:
        if(p_cfg->monitor_type == CTC_MONITOR_BUFFER && p_cfg->buffer_type == CTC_MONITOR_BUFFER_SC)
        {
            _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,0, &field_val, p_cfg->sc);
        }
        else if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,0, &field_val, channel_id);
        }
        else
        {
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_latencyInformMinThrd_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            field_val = (field_val << 5);
        }
        p_cfg->value = field_val;
        break;

    case CTC_MONITOR_CONFIG_MON_INFORM_MAX:
        if(p_cfg->monitor_type == CTC_MONITOR_BUFFER && p_cfg->buffer_type == CTC_MONITOR_BUFFER_SC)
        {
            _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,1, &field_val, p_cfg->sc);
        }
        else if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_BUFFER, p_cfg->buffer_type,1, &field_val, channel_id);
        }
        else
        {
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_latencyInformMaxThrd_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            field_val = (field_val << 5);
        }
        p_cfg->value = field_val;
        break;


    case CTC_MONITOR_CONFIG_LOG_EN:

        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            cmd = DRV_IOR(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        }
        else
        {
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            field_val = field_val?1:0;
        }
        p_cfg->value = field_val;
        break;

    case CTC_MONITOR_CONFIG_LOG_THRD_LEVEL:
        if (p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        p_cfg->level = field_val;
        break;

    case CTC_MONITOR_CONFIG_MON_SCAN_EN:
        if (p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            uint32 fvalue[2];
            ds_t ds;

            cmd = p_cfg->dir ? DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG): DRV_IOR(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
            switch(p_cfg->buffer_type)
            {
            case CTC_MONITOR_BUFFER_PORT:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    GetErmResrcMonScanCtl(A,  resrcMonScanPushChannelEn_f, ds, fvalue);
                }
                else
                {
                    GetIrmResrcMonScanCtl(A,  resrcMonScanPushChannelEn_f, ds, fvalue);
                }
                field_val = CTC_IS_BIT_SET(fvalue[channel_id >> 5], channel_id&0x1F);
                break;
            case CTC_MONITOR_BUFFER_SC:
                field_val = p_cfg->dir ? GetErmResrcMonScanCtl(V, resrcMonScanPushScEn_f, ds):GetIrmResrcMonScanCtl(V, resrcMonScanPushScEn_f, ds);
                break;
            case CTC_MONITOR_BUFFER_TOTAL:
                field_val = p_cfg->dir ? GetErmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, ds):GetIrmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, ds);
                break;
            case CTC_MONITOR_BUFFER_QUEUE:
                if(p_cfg->dir == CTC_EGRESS)
                {
                    cmd = DRV_IOR(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortEn_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                    cmd = DRV_IOR(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortId_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_tmp));
                    field_val = (field_val && value_tmp == channel_id)? 1 : 0;
                }
                else
                {
                    return CTC_E_NOT_SUPPORT;
                }
                break;
            default:
                return CTC_E_INVALID_PARAM;
            }
        }
        else
        {
            uint32 fvalue[2];
            ds_t ds;

            cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
            GetLatencyMonScanCtl(A,  reportEn_f, ds, fvalue);
            field_val = CTC_IS_BIT_SET(fvalue[channel_id >> 5], channel_id&0x1F);

        }
        p_cfg->value = field_val;
        break;

    case  CTC_MONITOR_CONFIG_MON_INTERVAL:
        CTC_ERROR_RETURN(sys_usw_monitor_get_timer(lchip, p_cfg->monitor_type, &p_cfg->value));
        break;

    case  CTC_MONITOR_CONFIG_LANTENCY_DISCARD_EN:
        if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
            field_val = field_val?1:0;
        }
        p_cfg->value = field_val;
        break;

    case  CTC_MONITOR_CONFIG_LANTENCY_DISCARD_THRD_LEVEL:
        if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            cmd = DRV_IOR(DsDestChannel_t, DsDestChannel_latencyDiscardEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        }
        p_cfg->level = field_val;
        break;
    case  CTC_MONITOR_RETRIEVE_LATENCY_STATS:
        if(p_cfg->monitor_type == CTC_MONITOR_LATENCY)
        {
            uint32 fvalue[2];
            ds_t ds;
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
            GetLatencyMonScanCtl(A,  reportEn_f, ds, fvalue);
            field_val = CTC_IS_BIT_SET(fvalue[channel_id >> 5], channel_id&0x1F);
            if(field_val)
            {
                p_cfg->value = p_usw_monitor_master[lchip]->latency_stats[channel_id][p_cfg->level];
            }
            else
            {
                cmd = DRV_IOR(DsLatencyMonCnt_t, DsLatencyMonCnt_array_0_latencyThrdCnt_f + p_cfg->level);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &p_cfg->value));
            }
        }
        break;
    case CTC_MONITOR_RETRIEVE_MBURST_STATS:
        if(p_cfg->monitor_type == CTC_MONITOR_BUFFER)
        {
            CTC_MAX_VALUE_CHECK(p_cfg->level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
            cmd = DRV_IOR(ErmMiscCtl_t, ErmMiscCtl_microBurstInformEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            if(field_val == 0)
            {
                return CTC_E_INVALID_PARAM;
            }
            p_cfg->value = p_usw_monitor_master[lchip]->mburst_stats[channel_id][p_cfg->level];
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;

}


int32
sys_usw_monitor_get_watermark(uint8 lchip, ctc_monitor_watermark_t *p_watermark)
{
   uint32 cmd = 0;
   uint32 step = 0;
   uint8 channel_id = 0;
   uint16 lport = 0;
   uint32 uc_cnt = 0;
   uint32 mc_cnt = 0;
   uint32 total_cnt = 0;
   uint32 field_value = 0;
   uint64 latency_value = 0;

   SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()",__FUNCTION__);
   SYS_MONITOR_INIT_CHECK();
   if((p_watermark->u.buffer.buffer_type == CTC_MONITOR_BUFFER_PORT && p_watermark->monitor_type == CTC_MONITOR_BUFFER)
        || p_watermark->monitor_type == CTC_MONITOR_LATENCY)
   {
       SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_watermark->gport, lchip, lport);
       channel_id = SYS_GET_CHANNEL_ID(lchip, p_watermark->gport);
       if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
       {
           return CTC_E_INVALID_PARAM;
       }
   }

   if (p_watermark->monitor_type == CTC_MONITOR_BUFFER)
   {
       switch (p_watermark->u.buffer.buffer_type)
       {
       case CTC_MONITOR_BUFFER_PORT :
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOR(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_ucastCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &uc_cnt));
           cmd = DRV_IOR(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_mcastCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &mc_cnt));
           cmd = DRV_IOR(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &total_cnt));

           p_watermark->u.buffer.max_uc_cnt = uc_cnt;
           p_watermark->u.buffer.max_mc_cnt = mc_cnt;
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       else
       {
           cmd = DRV_IOR(DsIrmResrcMonPortMax_t, DsIrmResrcMonPortMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &total_cnt));
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       break;

       case CTC_MONITOR_BUFFER_TOTAL :
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOR(DsErmResrcMonMiscMax_t, DsErmResrcMonMiscMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       else
       {
           cmd = DRV_IOR(DsIrmResrcMonMiscMax_t, DsIrmResrcMonMiscMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       break;

       case CTC_MONITOR_BUFFER_SC :
       CTC_MAX_VALUE_CHECK(p_watermark->u.buffer.sc, SYS_MONITOR_SC_NUM_MAX - 1);
       step = DsErmResrcMonMiscMax_g_1_scCntMax_f - DsErmResrcMonMiscMax_g_0_scCntMax_f;
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOR(DsErmResrcMonMiscMax_t, DsErmResrcMonMiscMax_g_0_scCntMax_f + step*p_watermark->u.buffer.sc);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       else
       {
           cmd = DRV_IOR(DsIrmResrcMonMiscMax_t, DsIrmResrcMonMiscMax_g_0_scCntMax_f + step*p_watermark->u.buffer.sc);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
           p_watermark->u.buffer.max_total_cnt = total_cnt;
       }
       break;

       default:
           return CTC_E_INVALID_PARAM;
       }
   }
   else
   {
       cmd = DRV_IOR(DsLatencyWatermark_t, DsLatencyWatermark_latencyMax_f);
       CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &field_value));
       latency_value = field_value;
       p_watermark->u.latency.max_latency = latency_value << 5;

   }
   return CTC_E_NONE;
}

int32
sys_usw_monitor_clear_watermark(uint8 lchip, ctc_monitor_watermark_t *p_watermark)
{
   uint32 cmd = 0;
   uint16 lport = 0;
   uint32 step = 0;
   uint8 channel_id = 0;
   uint32 uc_cnt = 0;
   uint32 mc_cnt = 0;
   uint32 total_cnt = 0;
   uint32 field_value = 0;

   SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()",__FUNCTION__);
   SYS_MONITOR_INIT_CHECK();
   if((p_watermark->u.buffer.buffer_type == CTC_MONITOR_BUFFER_PORT && p_watermark->monitor_type == CTC_MONITOR_BUFFER)
        || p_watermark->monitor_type == CTC_MONITOR_LATENCY)
   {
       SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_watermark->gport, lchip, lport);
       channel_id = SYS_GET_CHANNEL_ID(lchip, p_watermark->gport);
       if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM) )
       {
           return CTC_E_INVALID_PARAM;
       }
   }
   if (p_watermark->monitor_type == CTC_MONITOR_BUFFER)
   {
       switch (p_watermark->u.buffer.buffer_type)
       {
       case CTC_MONITOR_BUFFER_PORT :
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOW(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_ucastCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &uc_cnt));
           cmd = DRV_IOW(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_mcastCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &mc_cnt));
           cmd = DRV_IOW(DsErmResrcMonPortMax_t, DsErmResrcMonPortMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &total_cnt));
       }
       else
       {
           cmd = DRV_IOW(DsIrmResrcMonPortMax_t, DsIrmResrcMonPortMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &total_cnt));
       }
       break;

       case CTC_MONITOR_BUFFER_TOTAL :
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOW(DsErmResrcMonMiscMax_t, DsErmResrcMonMiscMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
       }
       else
       {
           cmd = DRV_IOW(DsIrmResrcMonMiscMax_t, DsIrmResrcMonMiscMax_totalCntMax_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
       }
       break;

       case CTC_MONITOR_BUFFER_SC :
       CTC_MAX_VALUE_CHECK(p_watermark->u.buffer.sc, SYS_MONITOR_SC_NUM_MAX - 1);
       step = DsErmResrcMonMiscMax_g_1_scCntMax_f - DsErmResrcMonMiscMax_g_0_scCntMax_f;
       if(CTC_EGRESS == p_watermark->u.buffer.dir)
       {
           cmd = DRV_IOW(DsErmResrcMonMiscMax_t, DsErmResrcMonMiscMax_g_0_scCntMax_f + step*p_watermark->u.buffer.sc);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
       }
       else
       {
           cmd = DRV_IOW(DsIrmResrcMonMiscMax_t, DsIrmResrcMonMiscMax_g_0_scCntMax_f + step*p_watermark->u.buffer.sc);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &total_cnt));
       }
       break;

       default:
           return CTC_E_INVALID_PARAM;
       }
   }
   else
   {
       cmd = DRV_IOW(DsLatencyWatermark_t, DsLatencyWatermark_latencyMax_f);
       CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id&0x3F, cmd, &field_value));
   }

    return CTC_E_NONE;
}


int32
sys_usw_monitor_sync_latency(uint8 lchip, ctc_monitor_data_t *p_data,
                                     DsLatencyMonInfo_m * p_latency, uint64* p_ts)
{
     uint8 info_type = 0;
     uint16 channel_id = 0;
     uint8 idx = 0;
     uint8 gchip = 0;
     uint16 lport = 0;
     uint16 gport = 0;
     uint8 msg_idx = 0;
     ctc_monitor_msg_t *p_msg = NULL;
     uint32 value[2] = {0};

     SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()",__FUNCTION__);
     msg_idx = p_data->msg_num;
     p_msg =  (ctc_monitor_msg_t*)p_data->p_msg +msg_idx;
     p_msg->monitor_type = CTC_MONITOR_LATENCY;
     p_msg->timestamp = *p_ts;
     channel_id = GetDsLatencyMonInfo(V, channelId_f, p_latency);
     /*Need mapping from channel to lport*/
     lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(SYS_GET_LPORT_ID_WITH_CHAN(lchip, channel_id));
     sys_usw_get_gchip_id(lchip, &gchip);
     gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

     info_type = GetDsLatencyMonInfo(V, infoType_f, p_latency);

     if (0 == info_type)
     {
         p_msg->msg_type = CTC_MONITOR_MSG_STATS;
         p_msg->u.latency_stats.gport = gport;

         for (idx = 0; idx < 8; idx++)
         {
             p_msg->u.latency_stats.threshold_cnt[idx]
             =  GetDsLatencyMonInfo(V, data_latencyMonData_0_latencyThrdCnt_f + idx, p_latency);
             p_usw_monitor_master[lchip]->latency_stats[channel_id][idx] += p_msg->u.latency_stats.threshold_cnt[idx];
         }
     }
     else
     {
         p_msg->msg_type = CTC_MONITOR_MSG_EVENT;
         p_msg->u.latency_event.gport = gport;
         p_msg->u.latency_event.event_state = GetDsLatencyMonInfo(V, data_latencyEventData_valid_f, p_latency);
         p_msg->u.latency_event.source_port = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsLatencyMonInfo(V, data_latencyEventData_sourcePort_f, p_latency));
         p_msg->u.latency_event.port = GetDsLatencyMonInfo(V, data_latencyEventData_localPhyPort_f, p_latency);
         p_msg->u.latency_event.level = GetDsLatencyMonInfo(V, data_latencyEventData_region_f, p_latency);
         p_msg->u.latency_event.event_state = (p_msg->u.latency_event.event_state == 1) ? CTC_MONITOR_EVENT_STATE_OCCUR:CTC_MONITOR_EVENT_STATE_CLEAR;

         GetDsLatencyMonInfo(A,  data_latencyEventData_residenceTime_f, p_latency, value);
         SYS_UINT64_SET(p_msg->u.latency_event.latency, value);

         GetDsLatencyMonInfo(A,  data_latencyEventData_timestamp_f, p_latency, value);
         SYS_UINT64_SET(p_msg->timestamp, value);

     }

     msg_idx++;
     p_data->msg_num = msg_idx;
    return CTC_E_NONE;
}

int32
sys_usw_monitor_sync_buffer(uint8 lchip, ctc_monitor_data_t *p_data,
                                   void* p_buffer, uint64* p_ts, uint8 msg_type, uint8 dir)
{
    uint8 info_type = 0;
    uint16 channel_id = 0;
    uint8 gchip = 0;
    uint16 lport = 0;
    uint16 gport = 0;
    uint8 msg_idx = 0;
    ctc_monitor_msg_t *p_msg = NULL;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 buffer_cnt = 0;
    uint8 queue_type = 0;
    uint8 burst_type = 0;
    uint32 step = 0;
    uint8 level = 0;
    uint8 idx = 0;

    msg_idx = p_data->msg_num;
    p_msg = (ctc_monitor_msg_t*)p_data->p_msg +msg_idx;
    p_msg->monitor_type = CTC_MONITOR_BUFFER;

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(dir == CTC_EGRESS)
    {
        if(msg_type == CTC_MONITOR_MSG_STATS)
        {
            p_msg->u.buffer_stats.dir = CTC_EGRESS;
            info_type = GetTempErmResrcMonInfo(V, monInfoType_f, (uint32*)p_buffer);

            if (0 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->timestamp = *p_ts;
                channel_id = GetTempErmResrcMonInfo(V, u_g1_channelId_f , p_buffer);

                /*Need mapping from channel to lport*/
                lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(SYS_GET_LPORT_ID_WITH_CHAN(lchip, channel_id));
                sys_usw_get_gchip_id(lchip, &gchip);
                gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

                p_msg->u.buffer_stats.gport = gport;
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_PORT;
                p_msg->u.buffer_stats.buffer_cnt  = GetTempErmResrcMonInfo(V, u_g1_pktBufCnt_f, p_buffer);

                p_usw_monitor_master[lchip]->buffer_stats[channel_id][0] = GetTempErmResrcMonInfo(V, u_g1_ucastBufCnt_f, p_buffer);
                p_usw_monitor_master[lchip]->buffer_stats[channel_id][1] = GetTempErmResrcMonInfo(V, u_g1_mcastBufCnt_f, p_buffer);

                msg_idx++;
            }
            else if(1 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->u.buffer_stats.sc = 0;
                p_msg->u.buffer_stats.buffer_cnt  = GetTempErmResrcMonInfo(V, u_g2_0_scCnt_f, p_buffer);
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_SC;
                msg_idx++;
            }
            else if(2 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->u.buffer_stats.buffer_cnt = GetTempErmResrcMonInfo(V, u_g3_totalCnt_f, p_buffer);
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
                p_usw_monitor_master[lchip]->totoal_stats = p_msg->u.buffer_stats.buffer_cnt;
                msg_idx++;
            }
            else if(3 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                cmd = DRV_IOR(ErmResrcMonPortCtl_t, ErmResrcMonPortCtl_resrcMonPortId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(SYS_GET_LPORT_ID_WITH_CHAN(lchip, field_val));
                sys_usw_get_gchip_id(lchip, &gchip);
                gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
                p_msg->u.buffer_stats.queue_id.gport = gport;
                queue_type = GetTempErmResrcMonInfo(V, u_g4_detailedType_f, p_buffer);
                if(0 == queue_type || 1 == queue_type)
                {
                    p_msg->u.buffer_stats.queue_id.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
                }
                p_msg->u.buffer_stats.queue_id.queue_id = GetTempErmResrcMonInfo(V, u_g4_queueOffset_f, p_buffer);
                p_msg->u.buffer_stats.buffer_cnt = GetTempErmResrcMonInfo(V, u_g4_queueCnt_f, p_buffer);
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_QUEUE;
                msg_idx++;
            }
        }
        else if(msg_type == CTC_MONITOR_MSG_EVENT)
        {
            p_msg->msg_type = CTC_MONITOR_MSG_EVENT;
            p_msg->timestamp = *p_ts;
            if (DRV_IS_DUET2(lchip))
            {
                channel_id = GetTempErmMbInfo(V, channelId_f, p_buffer);
                /*Need mapping from channel to lport*/
                lport = SYS_GET_LPORT_ID_WITH_CHAN(lport, channel_id);
                lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
                sys_usw_get_gchip_id(lchip, &gchip);
                gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

                p_msg->u.buffer_event.gport = gport;
                p_msg->u.buffer_event.buffer_type = CTC_MONITOR_BUFFER_PORT;
                p_msg->u.buffer_event.event_state       =  GetTempErmMbInfo(V, microburstValid_f, p_buffer);
                p_msg->u.buffer_event.uc_cnt            =  GetTempErmMbInfo(V, ucastBufCnt_f, p_buffer);
                p_msg->u.buffer_event.mc_cnt            =  GetTempErmMbInfo(V, mcastBufCnt_f, p_buffer);
                p_msg->u.buffer_event.threshold_level   =  GetTempErmMbInfo(V, cngLevel_f, p_buffer);
                p_msg->u.buffer_event.event_state = (p_msg->u.buffer_event.event_state == 1) ? CTC_MONITOR_EVENT_STATE_OCCUR:CTC_MONITOR_EVENT_STATE_CLEAR;
                msg_idx++;
            }
            else
            {
                burst_type = GetTempErmMbInfo(V, microburstType_f, p_buffer);
                switch(burst_type)
                {
                case 1:
                    channel_id = GetTempErmMbInfo(V, u_channel_channelId_f, p_buffer);
                    /*Need mapping from channel to lport*/
                    lport = SYS_GET_LPORT_ID_WITH_CHAN(lport, channel_id);
                    lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);
                    sys_usw_get_gchip_id(lchip, &gchip);
                    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

                    p_msg->u.buffer_event.gport = gport;
                    p_msg->u.buffer_event.buffer_type = CTC_MONITOR_BUFFER_PORT;
                    p_msg->u.buffer_event.uc_cnt            =  GetTempErmMbInfo(V, u_channel_ucastBufCnt_f, p_buffer);
                    p_msg->u.buffer_event.mc_cnt            =  GetTempErmMbInfo(V, u_channel_mcastBufCnt_f, p_buffer);
                    p_msg->u.buffer_event.threshold_level   =  GetTempErmMbInfo(V, u_channel_cngLevel_f, p_buffer);

                    cmd = DRV_IOR(DsErmPortMbCfg_t, DsErmPortMbCfg_mbMaxThrd_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
                    buffer_cnt = p_msg->u.buffer_event.uc_cnt + p_msg->u.buffer_event.mc_cnt;
                    break;
                 case 2:
                    p_msg->u.buffer_event.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
                    p_msg->u.buffer_event.buffer_cnt =  GetTempErmMbInfo(V, u_total_totalCnt_f, p_buffer);
                    cmd = DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_totalMbMaxThrd_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                    buffer_cnt = p_msg->u.buffer_event.buffer_cnt;
                    break;
                 case 3:
                    p_msg->u.buffer_event.buffer_type = CTC_MONITOR_BUFFER_SC;
                    p_msg->u.buffer_event.buffer_cnt =  GetTempErmMbInfo(V, u_sc_scCnt_f, p_buffer);
                    p_msg->u.buffer_event.sc =  GetTempErmMbInfo(V, u_sc_id_f, p_buffer);
                    step = ErmGlbMbCfg_sc_1_scMbMaxThrd_f - ErmGlbMbCfg_sc_0_scMbMaxThrd_f;
                    cmd = DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_sc_0_scMbMaxThrd_f + step * p_msg->u.buffer_event.sc);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                    buffer_cnt = p_msg->u.buffer_event.buffer_cnt;
                    break;
                 case 5:
                    p_msg->u.buffer_event.buffer_type = CTC_MONITOR_BUFFER_C2C;
                    p_msg->u.buffer_event.buffer_cnt =  GetTempErmMbInfo(V, u_c2c_c2cCnt_f, p_buffer);
                    cmd = DRV_IOR(ErmGlbMbCfg_t, ErmGlbMbCfg_c2cMbMaxThrd_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                    buffer_cnt = p_msg->u.buffer_event.buffer_cnt;
                    break;
                }

                p_msg->u.buffer_event.event_state = (buffer_cnt >= field_val) ? CTC_MONITOR_EVENT_STATE_OCCUR:CTC_MONITOR_EVENT_STATE_CLEAR;
                msg_idx++;
            }
            if(p_msg->u.buffer_event.event_state == CTC_MONITOR_EVENT_STATE_OCCUR && p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                p_usw_monitor_master[lchip]->mburst_timestamp[channel_id] = p_msg->timestamp;
            }
            else if(p_msg->u.buffer_event.event_state == CTC_MONITOR_EVENT_STATE_CLEAR && p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                field_val = p_msg->timestamp - p_usw_monitor_master[lchip]->mburst_timestamp[channel_id];
                _sys_usw_monitor_get_mburst_level(lchip, &level, field_val);
                if(level != 0xff)
                {
                    p_msg = (ctc_monitor_msg_t*)p_data->p_msg +msg_idx;
                    p_msg->monitor_type = CTC_MONITOR_BUFFER;
                    p_msg->msg_type = CTC_MONITOR_MSG_MBURST_STATUS;
                    p_msg->timestamp = *p_ts;
                    p_usw_monitor_master[lchip]->mburst_stats[channel_id][level] ++;
                    p_msg->u.mburst_stats.gport = gport;
                    for (idx = 0; idx < 8; idx++)
                    {
                         p_msg->u.mburst_stats.threshold_cnt[idx] = p_usw_monitor_master[lchip]->mburst_stats[channel_id][idx];
                    }
                    msg_idx++;
                }
            }
        }
        p_data->msg_num = msg_idx;
    }
    else if(dir == CTC_INGRESS)
    {
        if(msg_type == CTC_MONITOR_MSG_STATS)
        {
            p_msg->u.buffer_stats.dir = CTC_INGRESS;
            info_type = GetTempIrmResrcMonInfo(V, monInfoType_f, (uint32*)p_buffer);

            if (0 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->timestamp = *p_ts;
                channel_id = GetTempIrmResrcMonInfo(V, u_g1_channelId_f , p_buffer);

                /*Need mapping from channel to lport*/
                lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(SYS_GET_LPORT_ID_WITH_CHAN(lchip, channel_id));
                sys_usw_get_gchip_id(lchip, &gchip);
                gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

                p_msg->u.buffer_stats.gport = gport;
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_PORT;
                p_msg->u.buffer_stats.buffer_cnt  = GetTempIrmResrcMonInfo(V, u_g1_pktBufCnt_f, p_buffer);
                msg_idx++;
            }
            else if(1 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->u.buffer_stats.sc = 0;
                p_msg->u.buffer_stats.buffer_cnt  = GetTempIrmResrcMonInfo(V, u_g2_0_scCnt_f, p_buffer);
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_SC;
                msg_idx++;
            }
            else if(2 == info_type)
            {
                p_msg->msg_type = CTC_MONITOR_MSG_STATS;
                p_msg->u.buffer_stats.buffer_cnt = GetTempIrmResrcMonInfo(V, u_g3_totalCnt_f, p_buffer);
                p_msg->u.buffer_stats.buffer_type = CTC_MONITOR_BUFFER_TOTAL;
                msg_idx++;
            }
        }
        p_data->msg_num = msg_idx;
    }
    return CTC_E_NONE;
}

int32
sys_usw_monitor_sync_data(uint8 lchip, void* p_data)
{
    uint16 entry_num = 0;
    uint16 index = 0;
    DmaToCpuActMonErmFifo_m* p_monitor = NULL;
    sys_dma_info_t* p_dma_info = (sys_dma_info_t*)p_data;
    ctc_monitor_data_t  monitor_data;
    ctc_efd_data_t efd_data;
    ds_t ds;
    uint32 type = 0;
    uint64 ts;
    uint32 value[2] = {0};

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(ds, 0, sizeof(ds));
    sal_memset(&monitor_data, 0, sizeof(monitor_data));
    sal_memset(&efd_data, 0, sizeof(efd_data));

    monitor_data.p_msg = p_usw_monitor_master[lchip]->p_buffer;
    entry_num = p_dma_info->entry_num;

    for (index = 0; index < entry_num; index++)
    {
        if (NULL == p_usw_monitor_master[lchip])
        {
            return CTC_E_NOT_INIT;
        }

        p_monitor = ((DmaToCpuActMonErmFifo_m*)p_dma_info->p_data+index);

        type = GetDmaToCpuActMonErmFifo(V, monType_f, p_monitor);
        GetDmaToCpuActMonErmFifo(A, timestamp_f, p_monitor, value);
        SYS_UINT64_SET(ts, value);

        if (type == 0) /* efd flow aging */
        {
            efd_data.flow_id_array[efd_data.count] = GetDmaToCpuActMonDlbEfdFifo(V, efdMonPtr_f, p_monitor);
            efd_data.count ++;
        }

        if (type == 2)
        {
            DsLatencyMonInfo_m latency;
            GetDmaToCpuActMonEpeLantencyFifo(A,lantencyData_f, p_monitor, &ds);
            sal_memcpy(&latency, ds, sizeof(latency));
            sys_usw_monitor_sync_latency(lchip, &monitor_data, &latency, &ts);
        }

        if (type == 1)
        {
            GetDmaToCpuActMonErmFifo(A,ermMonData_f, p_monitor, &ds);

            /*buffer stats*/
            if(!GetMsErmDmaInfo(V,infoType_f,ds))
            {
                TempErmResrcMonInfo_m buffer;
                sal_memset(&buffer, 0, sizeof(buffer));
                GetMsErmDmaInfo(A,u_g2_resrcMonInfo_f,ds,&buffer);
                sys_usw_monitor_sync_buffer(lchip, &monitor_data, (void*)&buffer, &ts, CTC_MONITOR_MSG_STATS, CTC_EGRESS);
            }
            else/*buffer event*/
            {
                TempErmMbInfo_m buffer;
                sal_memset(&buffer, 0, sizeof(buffer));
                if(GetMsErmDmaInfo(V,u_g1_0_mbInfoValid_f,ds))
                {
                    GetMsErmDmaInfo(A,u_g1_0_mbInfo_f,ds,&buffer);
                    sys_usw_monitor_sync_buffer(lchip, &monitor_data, (void*)&buffer, &ts, CTC_MONITOR_MSG_EVENT, CTC_EGRESS);
                }

                if(GetMsErmDmaInfo(V,u_g1_1_mbInfoValid_f,ds))
                {
                    GetMsErmDmaInfo(A,u_g1_1_mbInfo_f,ds,&buffer);
                    sys_usw_monitor_sync_buffer(lchip, &monitor_data, (void*)&buffer, &ts, CTC_MONITOR_MSG_EVENT, CTC_EGRESS);
                }

            }
        }
        if (type == 3)
        {
            GetDmaToCpuActMonIrmFifo(A,irmMonData_f, p_monitor, &ds);

            /*buffer stats*/
            if(!GetMsIrmDmaInfo(V,infoType_f,ds))
            {
                TempIrmResrcMonInfo_m buffer;
                sal_memset(&buffer, 0, sizeof(buffer));
                GetMsIrmDmaInfo(A,u_g2_resrcMonInfo_f,ds,&buffer);
                sys_usw_monitor_sync_buffer(lchip, &monitor_data, (void*)&buffer, &ts, CTC_MONITOR_MSG_STATS,CTC_INGRESS);
            }
        }
    }




    /*CallBack*/
    if (p_usw_monitor_master[lchip]->func)
    {
        p_usw_monitor_master[lchip]->func(&monitor_data, p_usw_monitor_master[lchip]->user_data);
    }
    if(efd_data.count != 0)
    {
        /* process efd callback */
        sys_usw_efd_sync_data(lchip, &efd_data);
    }

    return CTC_E_NONE;
}


/*every slice alloc 48 ports*/
int32
sys_usw_monitor_sync_queue_stats(uint8 lchip, void* p_data)
{
    /*sys_dma_reg_t* p_dma_reg = (sys_dma_reg_t*)p_data;
    uint8 slice_id = 0;
    uint8* p_addr = NULL;

    slice_id = *((uint8*)p_dma_reg->p_ext);
    p_addr = p_dma_reg->p_data;*/

     /*-ctc_monitor_queue_sample_export(slice_id, p_addr);*/
    /* TODO later, using common cb */
    return CTC_E_NONE;
}



int32
sys_usw_monitor_register_cb(uint8 lchip, ctc_monitor_fn_t callback, void* userdata)
{
    SYS_MONITOR_INIT_CHECK();
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_usw_monitor_master[lchip]->func = callback;
    p_usw_monitor_master[lchip]->user_data = userdata;
    return CTC_E_NONE;
}


int32
sys_usw_monitor_get_cb(uint8 lchip, void **cb, void** user_data)
{
    *cb = p_usw_monitor_master[lchip]->func;
    *user_data = p_usw_monitor_master[lchip]->user_data;
    return CTC_E_NONE;
}



int32
sys_usw_monitor_set_global_config(uint8 lchip, ctc_monitor_glb_cfg_t* p_cfg)
{
    uint32 cmd = 0;

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MONITOR_INIT_CHECK();

    switch(p_cfg->cfg_type)
    {
    case  CTC_MONITOR_GLB_CONFIG_LATENCY_THRD:
        CTC_MAX_VALUE_CHECK(p_cfg->u.thrd.level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
        p_cfg->u.thrd.threshold = p_cfg->u.thrd.threshold >> 5;
        CTC_ERROR_RETURN(_sys_usw_monitor_thrd_check(lchip, p_cfg->u.thrd.level, p_cfg->u.thrd.threshold,CTC_MONITOR_GLB_CONFIG_LATENCY_THRD));
        CTC_ERROR_RETURN(
            _sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY,0, p_cfg->u.thrd.level, p_cfg->u.thrd.threshold, 0));

        break;

    case  CTC_MONITOR_GLB_CONFIG_BUFFER_LOG_MODE:
        CTC_MAX_VALUE_CHECK(p_cfg->u.value, 2);
        cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &(p_cfg->u.value)));

        break;
    case  CTC_MONITOR_GLB_CONFIG_MBURST_THRD:
        CTC_MAX_VALUE_CHECK(p_cfg->u.mburst_thrd.level, MCHIP_CAP(SYS_CAP_MONITOR_LATENCY_MAX_LEVEL));
        CTC_ERROR_RETURN(_sys_usw_monitor_thrd_check(lchip, p_cfg->u.mburst_thrd.level, p_cfg->u.mburst_thrd.threshold,CTC_MONITOR_GLB_CONFIG_MBURST_THRD));
        p_usw_monitor_master[lchip]->mburst_thrd[p_cfg->u.mburst_thrd.level] = p_cfg->u.mburst_thrd.threshold;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;

}


int32
sys_usw_monitor_init_global(uint8 lchip, void * p_global_cfg)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 array_val[2] = {0};
    uint8 channel_id = 0;
    ErmGlbMbCfg_m erm_glb_cfg;
    ErmResrcMonScanCtl_m erm_mon_ctl;
    IrmResrcMonScanCtl_m irm_mon_ctl;

    /*EpePktProcCtl_latencyInformEn_f*/
    field_val = 1;
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_latencyInformEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sal_memset(&erm_mon_ctl, 0 , sizeof(ErmResrcMonScanCtl_m));
    sal_memset(&irm_mon_ctl, 0 , sizeof(IrmResrcMonScanCtl_m));
    sal_memset(&erm_glb_cfg, 0 , sizeof(ErmGlbMbCfg_m));
    sal_memset(array_val,0,sizeof(array_val));
    cmd = DRV_IOR(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_mon_ctl));
    SetErmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, &erm_mon_ctl, array_val);
    SetErmResrcMonScanCtl(V, resrcMonScanEn_f, &erm_mon_ctl, 0);
    SetErmResrcMonScanCtl(V, resrcMonScanPushGroupDetailedEn_f, &erm_mon_ctl, 0);
    SetErmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, &erm_mon_ctl, 0);
    SetErmResrcMonScanCtl(V, resrcMonScanPushPortDetailedEn_f, &erm_mon_ctl, 0);
    SetErmResrcMonScanCtl(V, resrcMonScanPushScEn_f, &erm_mon_ctl, 0);
    cmd = DRV_IOW(ErmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_mon_ctl));

    cmd = DRV_IOR(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_mon_ctl));
    SetIrmResrcMonScanCtl(A, resrcMonScanPushChannelEn_f, &irm_mon_ctl, array_val);
    SetIrmResrcMonScanCtl(V, resrcMonScanEn_f, &irm_mon_ctl, 0);
    SetIrmResrcMonScanCtl(V, resrcMonScanPushMiscEn_f, &irm_mon_ctl, 0);
    SetIrmResrcMonScanCtl(V, resrcMonScanPushPortDetailedEn_f, &irm_mon_ctl, 0);
    SetIrmResrcMonScanCtl(V, resrcMonScanPushScEn_f, &irm_mon_ctl, 0);
    cmd = DRV_IOW(IrmResrcMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &irm_mon_ctl));

    field_val = 0xFFFF;
    cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_latencyRandomThrd_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstRandomThrd_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_latencyLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_latencyDiscardEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(ErmResrcMonScanCtl_t, ErmResrcMonScanCtl_resrcMonScanEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IrmResrcMonScanCtl_t, IrmResrcMonScanCtl_resrcMonScanEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(LatencyMonScanCtl_t, LatencyMonScanCtl_scanEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 0;
    cmd = DRV_IOW(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 0, 0, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 1, 16, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 2, 24, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 3, 32, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 4, 40, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 5, 48, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 6, 56, 0));
    CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_LATENCY, 0, 7, 64, 0));
    field_val = 1;
    for(channel_id = 0; channel_id < 64; channel_id ++)
    {
        CTC_ERROR_RETURN(_sys_usw_monitor_set_thrd(lchip, CTC_MONITOR_BUFFER, CTC_MONITOR_BUFFER_PORT, 1,MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD), channel_id));
        cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyActMonEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
    }
    /* init mburst thrd value */
    p_usw_monitor_master[lchip]->mburst_thrd[1] = 2000;
    p_usw_monitor_master[lchip]->mburst_thrd[2] = 5000;
    p_usw_monitor_master[lchip]->mburst_thrd[3] = 20000;
    p_usw_monitor_master[lchip]->mburst_thrd[4] = 400000;
    p_usw_monitor_master[lchip]->mburst_thrd[5] = 1000000;
    p_usw_monitor_master[lchip]->mburst_thrd[6] = 5000000;
    p_usw_monitor_master[lchip]->mburst_thrd[7] = 10000000;

    cmd = DRV_IOR(ErmGlbMbCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_glb_cfg));
    SetErmGlbMbCfg(V, c2cMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    SetErmGlbMbCfg(V, sc_0_scMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    SetErmGlbMbCfg(V, sc_1_scMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    SetErmGlbMbCfg(V, sc_2_scMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    SetErmGlbMbCfg(V, sc_3_scMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    SetErmGlbMbCfg(V, totalMbMaxThrd_f, &erm_glb_cfg, MCHIP_CAP(SYS_CAP_MONITOR_BUFFER_MAX_THRD));
    cmd = DRV_IOW(ErmGlbMbCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &erm_glb_cfg));

    return CTC_E_NONE;

}


int32
sys_usw_monitor_show_status(uint8 lchip)
{
    uint8 level = 0;
    uint32 threshold = 0;
    uint32 interval = 0;
    uint32 field_val = 0;
    uint32 cmd = 0;

    SYS_MONITOR_INIT_CHECK();

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Buffer Status:\n");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");
    sys_usw_monitor_get_timer(lchip, CTC_MONITOR_BUFFER, &interval);
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Scan intervel: %d<ms>\n", interval);
 /*-TBD    cmd = DRV_IOR(EpeMiscLogCtl_t, EpeMiscLogCtl_microBurstLogVictims_f);*/
 /*  CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));*/
 /*  SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "log mode     : %s\n", log_mode[field_val]);*/

    cmd = DRV_IOR(EpeMiscLogCtl_t, EpeMiscLogCtl_microburstLogMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (field_val == 0)
    {
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "log mode     : causer");
    }
    else if (field_val == 1)
    {
	  SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "log mode     : victim");
    }
    else
    {
	  SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "log mode     : all");
    }

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n\n----------------------------------------\n");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Latency Status:\n");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");
    sys_usw_monitor_get_timer(lchip, CTC_MONITOR_LATENCY, &interval);
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Scan intervel: %d<ms>\n", interval);

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-10s  %-10s\n", " latency level", "threshold");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------\n");
    for (level = 0; level < 8; level++)
    {
        _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_LATENCY, 0,level, &threshold, 0);
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d  %10u\n", level, (threshold << 5));
    }
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-10s  %-10s\n", " mburst level", "threshold");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------\n");
    for (level = 0; level < 8; level++)
    {
        _sys_usw_monitor_get_thrd(lchip, CTC_MONITOR_MSG_MBURST_STATUS, 0,level, &threshold, 0);
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d  %10u\n", level, threshold );
    }
    return CTC_E_NONE;
}


int32
sys_usw_monitor_clear_stats(uint8 lchip,  uint32 gport)
{
    uint8 channel_id = 0;
    uint16 lport = 0;
    uint8 i = 0;

    SYS_MONITOR_INIT_CHECK();

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_INVALID_PARAM;
    }
    p_usw_monitor_master[lchip]->buffer_stats[channel_id][0] = 0;
    p_usw_monitor_master[lchip]->buffer_stats[channel_id][1] = 0;

    for (i = 0; i < 8; i++)
    {
        p_usw_monitor_master[lchip]->latency_stats[channel_id][i] = 0;
    }

    return CTC_E_NONE;
}


int32
sys_usw_monitor_show_stats(uint8 lchip, uint32 gport)
{
    uint8 i = 0;
    uint8 channel_id = 0;
    uint16 lport = 0;
    char *buffer[2] = {"unicat", "mcast"};
    char *latecny[8] = {"zone0", "zone1", "zone2", "zone3", "zone4", "zone5", "zone6", "zone7"};

    SYS_MONITOR_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (channel_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s %-10s\n", "BUFFER", "COUNT");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------\n");
    for (i = 0; i < 2; i++)
    {
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s :%u\n", buffer[i],
                            p_usw_monitor_master[lchip]->buffer_stats[channel_id][i]);
    }

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n%-15s %-10s\n", "LATENCY", "COUNT");
    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------\n");
    for (i = 0; i < 8; i++)
    {
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s :%"PRIu64"\n", latecny[i],
                            p_usw_monitor_master[lchip]->latency_stats[channel_id][i]);
    }

    return CTC_E_NONE;
}


int32
sys_usw_monitor_testing_en(uint8 lchip, bool enable)
{
    #if 0
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 value[4];
    ds_t ds;
    uint32 channel_id = 0;
	uint8 i = 0;

	field_val = enable?1:0;

	for(i=0;i<4;i++)
	{
		value[i] = enable?0xFFFFFFFF:0;
	}

    cmd = DRV_IOW(QMgrEnqCtl_t, QMgrEnqCtl_activeBufMonEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(QMgrEnqCtl_t, QMgrEnqCtl_microBurstInformEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(QMgrEnqCtl_t, QMgrEnqCtl_activeBufMonTcEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(QMgrEnqCtl_t, QMgrEnqCtl_activeBufMonTotalEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOR(QMgrEnqCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetQMgrEnqCtl(A,  activeBufMonChanEn_f, ds, value);
    cmd = DRV_IOW(QMgrEnqCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
    SetLatencyMonScanCtl(A,  reportEn_f, ds, value);
    cmd = DRV_IOW(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    cmd = DRV_IOR(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 1, cmd, ds));
    SetLatencyMonScanCtl(A,  reportEn_f, ds, value);
    cmd = DRV_IOW(LatencyMonScanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 1, cmd, ds));

    for (channel_id = 0; channel_id < 128; channel_id++)
    {
        cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyActMonEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyInformEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
        cmd = DRV_IOW(DsDestChannel_t, DsDestChannel_latencyWatermarkEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &field_val));
    }
    #endif
    return 0;

}

void
sys_usw_monitor_process_isr(ctc_monitor_data_t * info, void* userdata)
{
    uint8 index = 0;
    uint8 i = 0;
     char* monitor_str[2] = {"B","L"};
     char* msg_str[3] = {"E","S","MB"};
     ctc_monitor_msg_t *p_msg = NULL;

    if (info == NULL)
    {
        return;
    }

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"-------------------------<B,L>: Buffer, Latency <E,S>:Event,Stats---------------------------------\n");

    for (index = 0; index < info->msg_num; index++)
    {
        p_msg = &info->p_msg[index];

        if (p_msg->monitor_type == CTC_MONITOR_LATENCY
            && p_msg->msg_type == CTC_MONITOR_MSG_EVENT)
        {
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %s%s %8s%04x %8s%04x %8s%-8d %8s%"PRIu64" %8s%-8d %8s%04x\n",
                      index,
                      "TS:",
                      p_msg->timestamp,
                      monitor_str[p_msg->monitor_type],
                      msg_str[p_msg->msg_type],
                      "Gport:0x",
                      p_msg->u.latency_event.gport,
                      "SrcPort:0x",
                      p_msg->u.latency_event.source_port,
                      "State:",
                      p_msg->u.latency_event.event_state,
                      "Latency:",
                      p_msg->u.latency_event.latency,
                      "Level:",
                     p_msg->u.latency_event.level,
                      "port:",
                      p_msg->u.latency_event.port);
        }

        if (p_msg->monitor_type == CTC_MONITOR_LATENCY
            && p_msg->msg_type == CTC_MONITOR_MSG_STATS)
        {
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %s%s %8s%04x %8s",
                      index,
                      "TS:",
                      p_msg->timestamp,
                      monitor_str[p_msg->monitor_type],
                      msg_str[p_msg->msg_type],
                      "Gport:0x",
                      p_msg->u.latency_stats.gport,
                      "thrd:");

            for (i = 0; i < 8; i++)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8u",  p_msg->u.latency_stats.threshold_cnt[i]);
            }
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " \n");
        }


        if (p_msg->monitor_type == CTC_MONITOR_BUFFER
            && p_msg->msg_type == CTC_MONITOR_MSG_EVENT)
        {
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %s%s",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type]);
            if(p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8s%04x %8s%-8u %8s%-8u %8s%-8d %8s%-8d\n",
                          "Gport:0x",
                           p_msg->u.buffer_event.gport,
                           "UcCnt:",
                           p_msg->u.buffer_event.uc_cnt,
                           "McCnt:",
                           p_msg->u.buffer_event.mc_cnt,
                           "State:",
                           p_msg->u.buffer_event.event_state,
                           "CngLevel:",
                           p_msg->u.buffer_event.threshold_level);
            }
            else if(p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_TOTAL)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8s%-8u %8s%-8d\n",
                           "TotalCnt:",
                           p_msg->u.buffer_event.buffer_cnt,
                           "State:",
                           p_msg->u.buffer_event.event_state);
            }
            else if(p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_SC)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8s%-8d %8s%-8u %8s%-8d\n",
                          "SC:",
                           p_msg->u.buffer_event.sc,
                           "ScCnt:",
                           p_msg->u.buffer_event.buffer_cnt,
                           "State:",
                           p_msg->u.buffer_event.event_state);
            }
            else if(p_msg->u.buffer_event.buffer_type == CTC_MONITOR_BUFFER_C2C)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8s%-8u %8s%-8d\n",
                           "C2cCnt:",
                           p_msg->u.buffer_event.buffer_cnt,
                           "State:",
                           p_msg->u.buffer_event.event_state);
            }
        }

        if (p_msg->monitor_type == CTC_MONITOR_BUFFER
             && p_msg->msg_type == CTC_MONITOR_MSG_STATS && p_msg->u.buffer_stats.dir == CTC_EGRESS)
        {

            if (p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%04x %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "EGRESS",
                          "Gport:0x",
                          p_msg->u.buffer_stats.gport,
                          "PktCnt:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }

            else if(p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_SC)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "EGRESS",
                          "ScCnt:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }
            else if(p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_TOTAL)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "EGRESS",
                          "Total:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }
            else if(p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_QUEUE)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%04x %8s %8s%-8d%8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "EGRESS",
                          "Gport:0x",
                          p_msg->u.buffer_stats.queue_id.gport,
                          "Queue Type:NETWORK_EGRESS",
                          "Queue ID:",
                          p_msg->u.buffer_stats.queue_id.queue_id,
                          "Queue cnt:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }
        }

        if (p_msg->monitor_type == CTC_MONITOR_BUFFER
             && p_msg->msg_type == CTC_MONITOR_MSG_STATS && p_msg->u.buffer_stats.dir == CTC_INGRESS)
        {
            if (p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_PORT)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%04x %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "INGRESS",
                          "Gport:0x",
                          p_msg->u.buffer_stats.gport,
                          "PktCnt:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }

            else if(p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_SC)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "INGRESS",
                          "ScCnt:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }
            else if(p_msg->u.buffer_stats.buffer_type == CTC_MONITOR_BUFFER_TOTAL)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %8s%s %8s %8s%-8u\n",
                          index,
                          "TS:",
                          p_msg->timestamp,
                          monitor_str[p_msg->monitor_type],
                          msg_str[p_msg->msg_type],
                          "INGRESS",
                          "Total:",
                          p_msg->u.buffer_stats.buffer_cnt);
            }
        }
        if (p_msg->monitor_type == CTC_MONITOR_BUFFER
            && p_msg->msg_type == CTC_MONITOR_MSG_MBURST_STATUS)
        {
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "No.%-4d %-4s%"PRIu64"  %s%s %8s%04x %8s",
                      index,
                      "TS:",
                      p_msg->timestamp,
                      monitor_str[p_msg->monitor_type],
                      msg_str[p_msg->msg_type],
                      "Gport:0x",
                      p_msg->u.mburst_stats.gport,
                      "thrd cnt:");

            for (i = 0; i < 8; i++)
            {
                SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " %8u",  p_msg->u.mburst_stats.threshold_cnt[i]);
            }
            SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, " \n");
        }
    }

    SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "-------------------------------------------------------------------------------------\n");

    return;
}

int32
sys_usw_monitor_init(uint8 lchip, void * p_global_cfg)
{
    if (p_usw_monitor_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_monitor_master[lchip] =  (sys_monitor_master_t*)mem_malloc(MEM_MONITOR_MODULE, sizeof(sys_monitor_master_t));
    if (NULL == p_usw_monitor_master[lchip])
    {
        SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_monitor_master[lchip], 0, sizeof(sys_monitor_master_t));

    p_usw_monitor_master[lchip]->p_buffer = mem_malloc(MEM_MONITOR_MODULE, sizeof(ctc_monitor_msg_t)*(MCHIP_CAP(SYS_CAP_MONITOR_SYNC_CNT)*4));
    if (NULL == p_usw_monitor_master[lchip]->p_buffer)
    {
          SYS_MONITOR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    CTC_ERROR_RETURN( sys_usw_monitor_set_timer(lchip, CTC_MONITOR_BUFFER, 1));
    CTC_ERROR_RETURN( sys_usw_monitor_set_timer(lchip, CTC_MONITOR_LATENCY, 1));

    CTC_ERROR_RETURN(sys_usw_monitor_init_global(lchip, p_global_cfg));

    CTC_ERROR_RETURN(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_MONITOR,
        sys_usw_monitor_sync_data));

    if (!p_usw_monitor_master[lchip]->func)
    {
        sys_usw_monitor_register_cb(lchip, sys_usw_monitor_process_isr, NULL);
    }

    drv_usw_agent_register_cb(DRV_AGENT_CB_GET_MONITOR, (void*)sys_usw_monitor_get_cb);

    return CTC_E_NONE;
}



int32
sys_usw_monitor_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_monitor_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_monitor_master[lchip]->p_buffer);
    mem_free(p_usw_monitor_master[lchip]);


    return CTC_E_NONE;
}

#endif

