#if (FEATURE_MODE == 0)
/**
 @file sys_usw_efd.c

 @date 2014-10-28

 @version v3.0


*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "sys_usw_chip.h"
#include "sys_usw_efd.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_ftm.h"
#include "sys_usw_nexthop_api.h"

#include "drv_api.h"


/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/


struct sys_efd_master_s
{
    ctc_efd_fn_t func;
    void* userdata;
};
typedef struct sys_efd_master_s sys_efd_master_t;

sys_efd_master_t* p_usw_efd_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_EFD_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(efd, efd, EFD_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

STATIC int32
_sys_usw_global_efd_clear_flowtable(uint8 lchip)
{
    uint16 loop = 0;
    uint32 value = 0;
    uint32 cmd = 0;

    for(loop = 0; loop < MCHIP_CAP(SYS_CAP_EFD_FLOW_STATS); loop++)
    {
        cmd = DRV_IOW(DsElephantFlowState_t, DsElephantFlowState_byteCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
        cmd = DRV_IOW(DsElephantFlowState_t, DsElephantFlowState_sparseCnt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
        if(loop < MCHIP_CAP(SYS_CAP_EFD_FLOW_DETECT_MAX))
        {
            cmd = DRV_IOW(DsElephantFlowDetect0_t, DsElephantFlowDetect0_counter_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
            cmd = DRV_IOW(DsElephantFlowDetect1_t, DsElephantFlowDetect1_counter_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
            cmd = DRV_IOW(DsElephantFlowDetect2_t, DsElephantFlowDetect2_counter_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
            cmd = DRV_IOW(DsElephantFlowDetect3_t, DsElephantFlowDetect3_counter_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &value));
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_efd_change_countmode(uint8 lchip, uint32 countmode)
{
    uint32 cmd = 0;
    EfdEngineCtl_m efd_engine_ctl;

    sal_memset(&efd_engine_ctl, 0, sizeof(efd_engine_ctl));

    if(0 == countmode)
    {
        cmd = DRV_IOR(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));
        SetEfdEngineCtl(V, elephantByteVolumeThrd_f, &efd_engine_ctl, 12000);     /* actual threshold = 12000*16Byte = 192000byte/Tsunit, Tsunit = 30ms. Total 50M bps */
        SetEfdEngineCtl(V, elephantByteVolumeThrd0_f, &efd_engine_ctl, 12000);
        cmd = DRV_IOW(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));
    }
    else
    {
        cmd = DRV_IOR(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));
        SetEfdEngineCtl(V, elephantByteVolumeThrd_f, &efd_engine_ctl, 3000);     /* actual threshold = 3000pkt = 3000pkt/Tsunit, Tsunit = 30ms. Total 0.1M pps */
        SetEfdEngineCtl(V, elephantByteVolumeThrd0_f, &efd_engine_ctl, 3000);
        cmd = DRV_IOW(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_efd_get_granu(uint8 lchip, uint32* granu)
{
    uint32 field_val = 0;

    CTC_ERROR_RETURN(sys_usw_efd_get_global_ctl(lchip, CTC_EFD_GLOBAL_DETECT_GRANU, &field_val));

    if (0 == field_val)
    {
        *granu = 16;
    }
    else if (1 == field_val)
    {
        *granu = 8;
    }
    else if (2 == field_val)
    {
        *granu = 4;
    }
    else if (3 == field_val)
    {
        *granu = 32;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}


int32
sys_usw_efd_set_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    uint32 granu = 0;
    uint32 interval = 0;

    SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(type)
    {
    case CTC_EFD_GLOBAL_MODE_TCP:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_onlyTcpPktDoEfd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_MIN_PKT_LEN:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0x3fff);
        field_val = *(uint32*)value ? 1 : 0;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_elephantPacketLenEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        field_val = *(uint32*)value;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_minElephantPacketLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_DETECT_RATE:
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_countMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        CTC_ERROR_RETURN(sys_usw_efd_get_global_ctl(lchip, CTC_EFD_GLOBAL_DETECT_INTERVAL, &interval));

        if(0 == field_val)
        {
            CTC_ERROR_RETURN(_sys_usw_efd_get_granu(lchip, &granu));
            field_val = ((*(uint32*)value) * interval / granu /8);
            CTC_MAX_VALUE_CHECK(field_val, 0x7FFFFF);
        }
        else
        {
            field_val = (*(uint32*)value) * interval;
            CTC_MAX_VALUE_CHECK(field_val, 0x7FFFFF);
        }

        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_elephantByteVolumeThrd0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_elephantByteVolumeThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_IPG_EN:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_ipgEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_EFD_EN:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_efdEnable_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_PPS_EN:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_countMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        CTC_ERROR_RETURN(_sys_usw_global_efd_clear_flowtable(lchip));
        CTC_ERROR_RETURN(_sys_usw_global_efd_change_countmode(lchip, field_val));
        break;

    case CTC_EFD_GLOBAL_FUZZY_MATCH_EN:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_noFlowTableMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_enableConflictFlowDetect_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_RANDOM_LOG_EN:
        field_val = *(bool *)value ? 1 : 0;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_elephantRandomLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_elephantFlowIdValid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_elephantFlowIdValid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_RANDOM_LOG_RATE:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 0xF);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(IpeEfdCtl_t, IpeEfdCtl_elephantRandomThresholdShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_DETECT_GRANU:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 3);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(EfdEngineCtl_t, EfdEngineCtl_efdCountShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_DETECT_INTERVAL:
        field_val = (*(uint32*)value) * 10;
        CTC_MAX_VALUE_CHECK(field_val, 0x3fffffff);
        cmd = DRV_IOW(EfdEngineTimerCtl_t, EfdEngineTimerCtl_cfgRefDivCountUpdRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        cmd = DRV_IOW(EfdEngineTimerCtl_t, EfdEngineTimerCtl_cfgRefDivAgingRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_AGING_MULT:
        CTC_MAX_VALUE_CHECK(*(uint32*)value, 7);
        field_val = *(uint32*)value;
        cmd = DRV_IOW(EfdEngineTimerCtl_t, EfdEngineTimerCtl_flowLetStateEvictionThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        break;

    case CTC_EFD_GLOBAL_REDIRECT:
        {
            uint32 *array = (uint32*)value;  /*array[0] is enable/disable, array[1] is nhId*/
            uint32 dsfwdPtr = 0;
            uint32 field_val = 0;

            field_val = array[1]?1:0;
            if (array[1])
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, array[1], &dsfwdPtr, 0));
            }
            if (DRV_IS_DUET2(lchip))
            {
                cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_redirectElephantFlowEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_clearElephantFlowDiscard_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_clearElephantFlowException_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_elephantFlowFwdPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsfwdPtr));
            }
            else
            {
                cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_redirectElephantFlowEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_clearElephantFlowDiscard_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_clearElephantFlowException_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
                cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_elephantFlowFwdPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsfwdPtr));
            }
            CTC_ERROR_RETURN(sys_usw_nh_set_efd_redirect_nh(lchip, array[1]));
        }
        break;

    default:
        SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;

}

int32
sys_usw_efd_get_global_ctl(uint8 lchip, ctc_efd_global_control_type_t type, void* value)
{

    uint32 field_val = 0;
    uint32 field_val1 = 0;
    uint32 cmd = 0;
    uint32 granu = 0;
    uint32 interval = 0;

    SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(type)
    {
    case CTC_EFD_GLOBAL_MODE_TCP:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_onlyTcpPktDoEfd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_MIN_PKT_LEN:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_elephantPacketLenEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_minElephantPacketLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val1));

        *(uint32 *)value = field_val ? field_val1 : 0;
        break;

    case CTC_EFD_GLOBAL_DETECT_RATE:
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_countMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val1));
        CTC_ERROR_RETURN(sys_usw_efd_get_global_ctl(lchip, CTC_EFD_GLOBAL_DETECT_INTERVAL, &interval));
        if (0 == interval)
        {
            return CTC_E_INVALID_CONFIG;
        }

        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_elephantByteVolumeThrd0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

        if(0 == field_val1)
        {
            CTC_ERROR_RETURN(_sys_usw_efd_get_granu(lchip, &granu));
            *(uint32 *)value = field_val * 8 * granu / interval;
        }
        else
        {
            *(uint32 *)value = field_val / interval;
        }
        break;

    case CTC_EFD_GLOBAL_IPG_EN:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_ipgEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_EFD_EN:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_efdEnable_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_PPS_EN:
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_countMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_FUZZY_MATCH_EN:
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_noFlowTableMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_RANDOM_LOG_EN:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_elephantRandomLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(bool *)value = field_val?TRUE:FALSE;
        break;

    case CTC_EFD_GLOBAL_RANDOM_LOG_RATE:
        cmd = DRV_IOR(IpeEfdCtl_t, IpeEfdCtl_elephantRandomThresholdShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value = field_val;
        break;

    case CTC_EFD_GLOBAL_DETECT_GRANU:
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_efdCountShift_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value = field_val;
        break;

    case CTC_EFD_GLOBAL_DETECT_INTERVAL:
        cmd = DRV_IOR(EfdEngineTimerCtl_t, EfdEngineTimerCtl_cfgRefDivCountUpdRstPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value = field_val/10;
        break;

    case CTC_EFD_GLOBAL_AGING_MULT:
        cmd = DRV_IOR(EfdEngineTimerCtl_t, EfdEngineTimerCtl_flowLetStateEvictionThrd_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
        *(uint32 *)value = field_val;
        break;

    case CTC_EFD_GLOBAL_FLOW_ACTIVE_BMP:
        {
            uint32 index = 0;
            uint32 valid_bmp[32] = {0};

            cmd = DRV_IOR(DsElephantFlowState_t, DsElephantFlowState_entryValid_f);
            for (index=0; index<1024; index++)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
                if (field_val)
                {
                    CTC_BIT_SET(valid_bmp[index/32], index%32);
                }
            }

            sal_memcpy((uint32*)value, (uint32*)valid_bmp, sizeof(valid_bmp)); /* this value PTR should at least have 1K bits */
        }
        break;

    case CTC_EFD_GLOBAL_REDIRECT:
        {
            uint32 *array = (uint32*)value;  /*array[0] not care, array[1] is nhId*/
            uint32 field_val = 0;
            uint32 nh_id = 0;

            if (DRV_IS_DUET2(lchip))
            {
                cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_redirectElephantFlowEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            }
            else
            {
                cmd = DRV_IOR(IpeAclQosCtl_t, IpeAclQosCtl_redirectElephantFlowEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
            }
            CTC_ERROR_RETURN(sys_usw_nh_get_efd_redirect_nh(lchip, &nh_id));
            array[1] = field_val?nh_id:0;
        }
        break;

    default:
        SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
sys_usw_efd_register_cb(uint8 lchip, ctc_efd_fn_t callback, void* userdata)
{
    SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_usw_efd_master[lchip]->func = callback;
    p_usw_efd_master[lchip]->userdata = userdata;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_global_efd_init(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 interval = 0;
    uint32 value = 0;
    uint32 countmode = 0;
    IpeEfdCtl_m efd_ctl;
    EfdEngineCtl_m efd_engine_ctl;
    EfdEngineTimerCtl_m efd_timer_ctl;
    IpePortEfdCtl_m port_efd_ctl;
    IpeFwdCtl_m ipe_fwd_ctl;
    IpeAclQosCtl_m ipe_aclqos_ctl;

    sal_memset(&efd_ctl, 0, sizeof(efd_ctl));
    sal_memset(&efd_engine_ctl, 0, sizeof(efd_engine_ctl));
    sal_memset(&efd_timer_ctl, 0, sizeof(efd_timer_ctl));
    sal_memset(&port_efd_ctl, 0, sizeof(port_efd_ctl));
    sal_memset(&ipe_fwd_ctl, 0, sizeof(ipe_fwd_ctl));
    sal_memset(&ipe_aclqos_ctl, 0, sizeof(ipe_aclqos_ctl));

    if (enable)
    {
        cmd = DRV_IOR(EfdEngineCtl_t, EfdEngineCtl_countMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &countmode));

        CTC_ERROR_RETURN(_sys_usw_global_efd_change_countmode(lchip, countmode));
        /* set threshold */
        cmd = DRV_IOR(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));
        SetEfdEngineCtl(V, conflictBypassEfdUpCnt_f, &efd_engine_ctl, 1);
        SetEfdEngineCtl(V, enableConflictFlowDetect_f, &efd_engine_ctl, 0);
        SetEfdEngineCtl(V, efdCountShift_f,          &efd_engine_ctl, 0);       /* 0 means shift right 4bit, default unit = 16Byte*/
        cmd = DRV_IOW(EfdEngineCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_engine_ctl));

        /* set update timer */
        SetEfdEngineTimerCtl(V, efdUpdMinPtr_f, &efd_timer_ctl, 0);
        SetEfdEngineTimerCtl(V, efdUpdMaxPtr_f, &efd_timer_ctl, 0x1FF);
        SetEfdEngineTimerCtl(V, efdUpdEn_f, &efd_timer_ctl, 1);

        SetEfdEngineTimerCtl(V, cfgRefDivCountUpdRstPulse_f,    &efd_timer_ctl, 300);       /* per 30ms update one time */
        SetEfdEngineTimerCtl(V, cfgResetDivCountUpdRstPulse_f,    &efd_timer_ctl, 0);       /* must set 0 */

        /* set flow aging */
        SetEfdEngineTimerCtl(V, flowLetStateMinPtr_f, &efd_timer_ctl, 0);
        SetEfdEngineTimerCtl(V, flowLetStateMaxPtr_f, &efd_timer_ctl, 0x3FF);
        /* (Maxptr - Minprt)*interval/coreFrequence */
        interval = 5000;
        SetEfdEngineTimerCtl(V, flowLetStateEvictionInterval_f, &efd_timer_ctl, interval);
        SetEfdEngineTimerCtl(V, flowLetStateEvictionThrd_f,     &efd_timer_ctl, 4);         /* 4 times */
        SetEfdEngineTimerCtl(V, flowLetStateEvictionUpdEn_f,    &efd_timer_ctl, 1);

        SetEfdEngineTimerCtl(V, cfgRefDivAgingRstPulse_f,    &efd_timer_ctl, 300);          /* per 30ms scan all ptr one time */
        SetEfdEngineTimerCtl(V, cfgResetDivAgingRstPulse_f,    &efd_timer_ctl, 0);          /* must set 0 */

        cmd = DRV_IOW(EfdEngineTimerCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_timer_ctl));

        /* Config pulse timer to 0.1ms, low 8 bit is decimal fraction */
        value = (625*25) << 8;
        cmd = DRV_IOW(RefDivEfdPulse_t, RefDivEfdPulse_cfgRefDivEfdPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = 0;
        cmd = DRV_IOW(RefDivEfdPulse_t, RefDivEfdPulse_cfgResetDivEfdPulse_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        /* enable exception to cpu */
        cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));
        SetIpeFwdCtl(V, newElephantExceptionEn_f, &ipe_fwd_ctl, 1);
        cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));

        /* TM enable exception to cpu */
        cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aclqos_ctl));
        SetIpeAclQosCtl(V, newElephantExceptionEn_f, &ipe_aclqos_ctl, 1);
        cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_aclqos_ctl));

        /* enable efd */
        SetIpeEfdCtl(V, efdEnable_f, &efd_ctl, 1);
        SetIpeEfdCtl(V, ipgEn_f, &efd_ctl, 0);
        SetIpeEfdCtl(V, fragmentPktKeyMode_f, &efd_ctl, 1);
        cmd = DRV_IOW(IpeEfdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_ctl));
    }
    else
    {
        /* disable efd */
        SetIpeEfdCtl(V, efdEnable_f, &efd_ctl, 0);
        cmd = DRV_IOW(IpeEfdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &efd_ctl));
    }


    return CTC_E_NONE;
}

void
sys_usw_efd_process_isr(ctc_efd_data_t* info, void* userdata)
{
    uint8 i = 0;

    for (i=0; i<info->count; i++)
    {
        SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "Efd FlowId %d is aginged !!\n", info->flow_id_array[i]);
    }
}

void
sys_usw_efd_sync_data(uint8 lchip, ctc_efd_data_t* info)
{
    if (NULL != p_usw_efd_master[lchip] && p_usw_efd_master[lchip]->func)
    {
        p_usw_efd_master[lchip]->func(info, p_usw_efd_master[lchip]->userdata);
    }
}

int32
sys_usw_efd_init(uint8 lchip)
{
    uint32 entry_num = 0;
    /* check init */
    if (NULL != p_usw_efd_master[lchip])
    {
        return CTC_E_NONE;
    }
    p_usw_efd_master[lchip] = (sys_efd_master_t*)mem_malloc(MEM_EFD_MODULE, sizeof(sys_efd_master_t));
    if (NULL == p_usw_efd_master[lchip])
    {
        SYS_EFD_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_efd_master[lchip], 0, sizeof(sys_efd_master_t));

    CTC_ERROR_RETURN(_sys_usw_global_efd_init(lchip, TRUE));

    if (!p_usw_efd_master[lchip]->func)
    {
        sys_usw_efd_register_cb(lchip, sys_usw_efd_process_isr, NULL);
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsElephantFlowState_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_EFD_FLOW_ENTRY_NUM) = entry_num;
    return CTC_E_NONE;
}

int32
sys_usw_efd_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (!p_usw_efd_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_efd_master[lchip]);

    return CTC_E_NONE;
}

#endif

