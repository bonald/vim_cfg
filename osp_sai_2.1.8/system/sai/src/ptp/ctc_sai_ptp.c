#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>

#define SAI_PTP_NS_PER_SEC (1000000000)
#define DEBUG_PTP_OFFSET 0
#define SAI_LINKAGG_MAX_MEMBER          16
#define SAI_PTP_OFFSET_TRIMMING_THRESHOLD 250
#define SAI_PTP_OFFSET_ADJUST_STEP 3

ctc_ptp_time_t
calc_ctc_ptp_time_offset(ctc_ptp_time_t *ts1, ctc_ptp_time_t *ts2)
{
    ctc_ptp_time_t ret_val;
    int64 offset_ns = 0;
    
    offset_ns = ((int64)ts1->seconds - (int64)ts2->seconds) * SAI_PTP_NS_PER_SEC + ((int64)ts1->nanoseconds - (int64)ts2->nanoseconds);

    if (offset_ns < 0)
    {
        ret_val.is_negative = 1;
        offset_ns = -offset_ns;
    }
    else
    {
        ret_val.is_negative = 0;
    }

    ret_val.seconds = offset_ns / SAI_PTP_NS_PER_SEC;
    ret_val.nanoseconds = offset_ns % SAI_PTP_NS_PER_SEC;

    return ret_val;
}

#ifndef ptp_glb_cfg
sai_status_t
ctc_sai_ptp_set_device_type(const sai_attribute_t *attr)
{
    ctc_ptp_device_type_t ctc_type = 0;
    uint32 sai_type = 0;
    int32 sdk_ret = 0;
    
    sai_type = attr->value.u32;
    switch (sai_type)
    {
    case SAI_PTP_DEVICE_NONE:
        ctc_type = CTC_PTP_DEVICE_NONE;
        break;

    case SAI_PTP_DEVICE_OC:
        ctc_type = CTC_PTP_DEVICE_OC;
        break;

    case SAI_PTP_DEVICE_BC:
        ctc_type = CTC_PTP_DEVICE_BC;
        break;
        
    case SAI_PTP_DEVICE_E2E_TC:
        ctc_type = CTC_PTP_DEVICE_E2E_TC;
        break;
        
    case SAI_PTP_DEVICE_P2P_TC:
        ctc_type = CTC_PTP_DEVICE_P2P_TC;
        break;

    default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sdk_ret = ctc_ptp_set_device_type(ctc_type);
    if (SAI_STATUS_SUCCESS != sdk_ret)
    {
        CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, device_type %d\n", __FUNCTION__, __LINE__, sdk_ret, ctc_type);
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_ptp_set_timestamp(const sai_attribute_t *attr)
{
    uint8 lchip_id = 0;
    struct timeval tv;
    ctc_ptp_time_t cpu_timestamp;
    ctc_ptp_time_t chip_timestamp;
    ctc_ptp_time_t offset;
    int32 sdk_ret = SAI_STATUS_SUCCESS;

    sal_memset(&cpu_timestamp, 0, sizeof(cpu_timestamp));
    sal_memset(&chip_timestamp, 0, sizeof(chip_timestamp));
    sal_memset(&offset, 0, sizeof(offset));

    /* 1. get chip timestamp */
    ctc_ptp_get_clock_timestamp(lchip_id, &chip_timestamp);

    /* 2. get CPU timestamp */
    if (0 == attr->value.s32list.count)
    {
        gettimeofday(&tv, NULL);
        cpu_timestamp.seconds = tv.tv_sec;
        cpu_timestamp.nanoseconds = tv.tv_usec * 1000;
    }
    else if (2 == attr->value.s32list.count)
    {
        cpu_timestamp.seconds = attr->value.s32list.list[0];
        cpu_timestamp.nanoseconds = attr->value.s32list.list[1];
    }
    else
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* 3. calcuate offset */
    offset = calc_ctc_ptp_time_offset(&cpu_timestamp, &chip_timestamp);

    CTC_SAI_DEBUG("cpu_ts: %u.%09u, chip_ts: %u.%09u, offset: %s%u.%09u", 
        cpu_timestamp.seconds, cpu_timestamp.nanoseconds, chip_timestamp.seconds, chip_timestamp.nanoseconds,
        offset.is_negative ? "-" : "", offset.seconds, offset.nanoseconds);


    /* 4. write to CHIP */
    sdk_ret = ctc_ptp_adjust_clock_offset(lchip_id, &offset);
    if (SAI_STATUS_SUCCESS != sdk_ret)
    {
        CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, set timestamp seconds %d %d %d\n", __FUNCTION__, __LINE__, sdk_ret, offset.seconds, offset.nanoseconds, offset.nano_nanoseconds);
    }
    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_ptp_set_adjust_offset(const sai_attribute_t *attr)
{
    uint8 lchip_id = 0;
    int32 sdk_ret = SAI_STATUS_SUCCESS;
    int64 offset = 0;

    ctc_ptp_time_t frc_offset;
    sal_memset(&frc_offset, 0x00, sizeof(frc_offset));

    offset = attr->value.s64;
    
    if (offset < 0)
    {
        frc_offset.is_negative = 1;
        offset = -offset;
    }

    frc_offset.seconds = offset / SAI_PTP_NS_PER_SEC;
    frc_offset.nanoseconds = offset % SAI_PTP_NS_PER_SEC;

    /* 2. calcuate offset */
#if 0
extern void log_sys(int, int, const char *fmt, ...);
    log_sys(2, 7, "offset: %s%u.%09u", offset.is_negative ? "-" : "", offset.seconds, offset.nanoseconds);
#endif
    
    // TODO: add sync-interface?
    if (frc_offset.seconds != 0 || frc_offset.nanoseconds >= SAI_PTP_OFFSET_TRIMMING_THRESHOLD)
    {
        sdk_ret = ctc_ptp_adjust_clock_offset(lchip_id, &frc_offset);
        if (SAI_STATUS_SUCCESS != sdk_ret)
        {
            CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, frc_offset seconds %d %d %d\n", __FUNCTION__, __LINE__, sdk_ret, frc_offset.seconds, frc_offset.nanoseconds, frc_offset.nano_nanoseconds);
        }
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    else
    {
        int32 tuning_times = frc_offset.nanoseconds/ SAI_PTP_OFFSET_ADJUST_STEP;
        int32 tuning_residuum = frc_offset.nanoseconds% SAI_PTP_OFFSET_ADJUST_STEP;
        int32 tuning_cut = 0;

        frc_offset.nanoseconds = SAI_PTP_OFFSET_ADJUST_STEP;
        for (tuning_cut = 0; tuning_cut < tuning_times; tuning_cut++)
        {
            CTC_SAI_DEBUG("DEBUG_PTP_OFFSET  ADJ ACTUAL (%u@%u:%u)",
                frc_offset.is_negative, frc_offset.seconds, frc_offset.nanoseconds);
            sdk_ret = ctc_ptp_adjust_clock_offset(lchip_id, &frc_offset);
            if (sdk_ret != SAI_STATUS_SUCCESS)
            {
                CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, frc_offset seconds %d %d %d, tuning_cut %d, tuning_times %d\n", 
                    __FUNCTION__, __LINE__, sdk_ret, frc_offset.seconds, frc_offset.nanoseconds, frc_offset.nano_nanoseconds, tuning_cut, tuning_times);
                
                return ctc_sai_get_error_from_sdk_error(sdk_ret);
            }
        }

        if (0 != tuning_residuum)
        {
            frc_offset.nanoseconds = tuning_residuum;
            CTC_SAI_DEBUG("DEBUG_PTP_OFFSET  ADJ ACTUAL (%u@%u:%u)", 
                frc_offset.is_negative, frc_offset.seconds, frc_offset.nanoseconds);
            sdk_ret = ctc_ptp_adjust_clock_offset(lchip_id, &frc_offset);
            if (sdk_ret != SAI_STATUS_SUCCESS)
            {
                CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, frc_offset seconds %d %d %d, tuning_cut %d, tuning_times %d\n", 
                    __FUNCTION__, __LINE__, sdk_ret, frc_offset.seconds, frc_offset.nanoseconds, frc_offset.nano_nanoseconds, tuning_cut, tuning_times);
                    
                return ctc_sai_get_error_from_sdk_error(sdk_ret);
            }
        }
    
    }

    return sdk_ret;
}


sai_status_t
ctc_sai_ptp_get_hardware_second(const sai_attribute_t *attr)
{
    int32 sdk_ret = 0;
    ctc_ptp_time_t frc;
    uint8 gchip = 0;

    sal_memset(&frc, 0x00, sizeof(frc));
    sdk_ret = ctc_ptp_get_clock_timestamp(gchip, &frc);
    if (SAI_STATUS_SUCCESS != sdk_ret)
    {
        CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, get_hardware_second seconds %d %d %d\n", __FUNCTION__, __LINE__, sdk_ret, frc.seconds, frc.nanoseconds, frc.nano_nanoseconds);
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    
    attr->value.s32list.list[0] = (int32)frc.seconds;
    attr->value.s32list.list[1] = (int32)frc.nanoseconds;
    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

#endif

#ifndef ptp_port_def
sai_status_t
ctc_sai_ptp_set_port_adjust_delay(uint32 gport_num, uint16 *gport, uint32 ptp_adjust_delay_type, int64 value)
{
    int32 sdk_ret = SAI_STATUS_SUCCESS;
    uint32 index = 0;
    uint16 gport_tmp = 0;

    for (index = 0; index < gport_num; index++)
    {
        gport_tmp = gport[index];

        /*asymmery correction*/
        if (ptp_adjust_delay_type == CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY)
        {
            sdk_ret = ctc_ptp_set_adjust_delay(gport_tmp, CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY, value);
            if (sdk_ret != SAI_STATUS_SUCCESS)
            {
                CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, set_port_adjust_delay type %d value %"PRId64"\n", __FUNCTION__, __LINE__, sdk_ret, ptp_adjust_delay_type, value);
                return ctc_sai_get_error_from_sdk_error(sdk_ret);
            }
            value = -value;
        }
        
        sdk_ret = ctc_ptp_set_adjust_delay(gport_tmp, ptp_adjust_delay_type, value);
        if (sdk_ret != SAI_STATUS_SUCCESS)
        {
            CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, set_port_adjust_delay type %d value %"PRId64"\n", __FUNCTION__, __LINE__, sdk_ret, ptp_adjust_delay_type, value);
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    
   return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_ptp_set_port_enable(uint32 gport_num, uint16 *gport, uint32 enable)
{
    int32 sdk_ret = SAI_STATUS_SUCCESS;
    uint32 index = 0;
    uint16 gport_tmp = 0;
    
    for (index = 0; index < gport_num; index++)
    {
        gport_tmp = gport[index];
        sdk_ret = ctc_port_set_property(gport_tmp, CTC_PORT_PROP_PTP_EN, enable);
        if (sdk_ret != SAI_STATUS_SUCCESS)
        {
            CTC_SAI_DEBUG("ERROR %s %d sdk_ret %d, set_port_enable %d\n", __FUNCTION__, __LINE__, sdk_ret, enable);
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

#endif
sai_status_t
ctc_sai_set_ptp_attribute(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t  *attr
    )
{
    int32 ret = SAI_STATUS_SUCCESS;
    uint16 gport[SAI_LINKAGG_MAX_MEMBER];
    uint32_t            list_idx    = 0;
    uint32 index = 0;
    uint32 ptp_adjust_delay_type;
    
    if (attr[index].id == SAI_PTP_ATTR_GPORT)
    {
        /*for (list_idx = 0; list_idx < attr[index].value.u16list.count; list_idx++)
        {
            gport[list_idx] = attr[index].value.u16list.list[list_idx];
        }*/

        gport[list_idx] = attr[index].value.u16;
        list_idx++;
        index++;
    }
    
    switch(attr[index].id)
    {
    /*glb cfg*/
    case SAI_PTP_ATTR_DEVICE_TYPE:
        ret = ctc_sai_ptp_set_device_type(attr);
        break;
    case SAI_PTP_ATTR_TIMESTAMP:
        ret = ctc_sai_ptp_set_timestamp(attr);
        break;
    case SAI_PTP_ATTR_ADJUST_OFFSET:
        ret = ctc_sai_ptp_set_adjust_offset(attr);
        break;
    case SAI_PTP_ATTR_ADJUST_FFO:
        /*ggsys Î´Ö§³Ö*/
        //ret = ctc_sai_ptp_set_adjust_ffo(attr);
        break;
    case SAI_PTP_ATTR_SYSTEM_TIME:
        ret = ctc_sai_ptp_set_device_type(attr);
        break;
    case SAI_PTP_ATTR_HARDWARE_SECOND:
        ret = ctc_sai_ptp_set_timestamp(attr);
        break;

    /*port cfg*/
    case SAI_PTP_ATTR_PORT_PATH_DELAY:
        ptp_adjust_delay_type = CTC_PTP_ADJUST_DELAY_PATH_DELAY;
        ret = ctc_sai_ptp_set_port_adjust_delay(list_idx, gport, ptp_adjust_delay_type, attr[index].value.s64);
        break;
    case SAI_PTP_ATTR_PORT_INGRESS_LATENCY_CORRECTION:
        ptp_adjust_delay_type = CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY;
        ret = ctc_sai_ptp_set_port_adjust_delay(list_idx, gport, ptp_adjust_delay_type, attr[index].value.s32);
        break;
    case SAI_PTP_ATTR_PORT_EGRESS_LATENCY_CORRECTION:
        ptp_adjust_delay_type = CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY;
        ret = ctc_sai_ptp_set_port_adjust_delay(list_idx, gport, ptp_adjust_delay_type, attr[index].value.s32);
        break;
    case SAI_PTP_ATTR_PORT_ASYMMETRY_CORRECTION:
        ptp_adjust_delay_type = CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY;
        ret = ctc_sai_ptp_set_port_adjust_delay(list_idx, gport, ptp_adjust_delay_type, attr[index].value.s64);
        break;
    case SAI_PTP_ATTR_PORT_ENABLE:
        ret = ctc_sai_ptp_set_port_enable(list_idx, gport, attr[index].value.u32);
        break;
    }
    
    return ret;
}

sai_status_t
ctc_sai_get_ptp_attribute(
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    int32 ret = SAI_STATUS_SUCCESS;
    uint32 index = 0;
    
    switch(attr_list[index].id)
    {
    case SAI_PTP_ATTR_HARDWARE_SECOND:
        ret = ctc_sai_ptp_get_hardware_second(&(attr_list[index]));
        break;
    }
    return ret;
}

sai_status_t
ctc_sai_ptp_db_init()
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_parser_l4app_ctl_t l4app_ctl;

    /*added by yejl: ptp init lead to the error in DUET2 board:
      pci read error! cmd_status = 1f0111, line = 230, offset = 0xa00300, lchip = 0
      tbl_id:4610, index:0, start_data_addr:00a00300 , ret:-9930
    */
    /*MODIFIED by chenc for bug 48939, 2018-09-13 
    550 duet2 support ptp init */
    //#ifndef DUET2
    ctc_ptp_init(NULL);
    //#endif

    sal_memset(&l4app_ctl, 0x0, sizeof(ctc_parser_l4app_ctl_t));
    l4app_ctl.ptp_en = TRUE;
    ret = ctc_parser_set_l4_app_ctl(&l4app_ctl);

    return ret;
}

#define ________SAI_PTP_INNER_FUNC

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = ctc_sai_ptp_db_init();
    if (ret != SAI_STATUS_SUCCESS)
    {
        return ret;
    }
    preg->init_status = INITIALIZED;

out:
    return ret;

    goto out;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_ptp_api_t g_sai_api_func = {
        .set_ptp_attribute               = ctc_sai_set_ptp_attribute,
        .get_ptp_attribute               = ctc_sai_get_ptp_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_PTP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_PTP_OUTER_FUNC
sai_status_t
ctc_sai_ptp_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

