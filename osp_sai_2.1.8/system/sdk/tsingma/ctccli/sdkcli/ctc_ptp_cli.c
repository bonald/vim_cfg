/**
 @file ctc_ptp_cli.c

 @date 2010-06-09

 @version v2.0

 This file defines functions for ptp cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_ptp_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"

CTC_CLI(ctc_cli_ptp_debug_on,
        ctc_cli_ptp_debug_on_cmd,
        "debug ptp (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PTP_M_STR,
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PTP_PTP_CTC;
    }
    else
    {
        typeenum = PTP_PTP_SYS;
    }

    ctc_debug_set_flag("ptp", "ptp", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_debug_off,
        ctc_cli_ptp_debug_off_cmd,
        "no debug ptp (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PTP_M_STR,
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PTP_PTP_CTC;
    }
    else
    {
        typeenum = PTP_PTP_SYS;
    }

    ctc_debug_set_flag("ptp", "ptp", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_global_property,
        ctc_cli_ptp_set_global_property_cmd,
        "ptp global-property (ucast-en | sync-cpu-en | signaling-cpu-en | management-cpu-en | \
        delay-request-en | port-based-en | cache-aging-time | interface-selected) value VALUE",
        CTC_CLI_PTP_M_STR,
        "Global property",
        "PTP ucast will be processed",
        "TC will copy sync and follow_up message to CPU",
        "Copy signaling message to CPU",
        "Copy management message to CPU",
        "P2P transparent clock will process Delay_Request message",
        "PTP will be enabled by port or vlan, default by port",
        "Aging time for timestamp",
        "Select Sync Interface or ToD Interface to synchronize from master clock",
        "Property value",
        "Value")
{
    int32 ret = 0;
    uint32 value;

    CTC_CLI_GET_UINT32("Property_value", value, argv[1]);

    if (CLI_CLI_STR_EQUAL("ucast-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_PTP_UCASE_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_PTP_UCASE_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("sync-cpu-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SYNC_COPY_TO_CPU_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_SYNC_COPY_TO_CPU_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("signaling-cpu-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SIGNALING_COPY_TO_CPU_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_SIGNALING_COPY_TO_CPU_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("management-cpu-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_MANAGEMENT_COPY_TO_CPU_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_MANAGEMENT_COPY_TO_CPU_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("delay-request-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_DELAY_REQUEST_PROCESS_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_DELAY_REQUEST_PROCESS_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("port-based-en", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("cache-aging-time", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("interface-selected", 0))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT, value);
        }
        else
        {
            ret = ctc_ptp_set_global_property(CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT, value);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_global_property,
        ctc_cli_ptp_show_global_property_cmd,
        "show ptp global-property (all |ucast-en | sync-cpu-en | signaling-cpu-en | management-cpu-en | \
        delay-request-en | port-based-en | cache-aging-time | interface-selected)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        "Global property",
        "Show all property",
        "PTP ucast will be processed",
        "TC will copy sync and follow_up message to CPU",
        "TC will copy signaling message to CPU",
        "TC will copy management message to CPU",
        "P2P transparent clock will process Delay_Request message",
        "PTP will be enabled by port or vlan, default by port",
        "Aging time for timestamp",
        "Select Sync Interface or ToD Interface to synchronize from master clock")
{
    int32 ret = 0;
    uint32 value = 0;
    uint8 index = 0xFF;
    uint8 show_all = 0;

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        show_all = 1;
    }

    ctc_cli_out("=================================\n");
    index = CTC_CLI_GET_ARGC_INDEX("ucast-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_PTP_UCASE_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_PTP_UCASE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp ucast-en               :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("sync-cpu-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SYNC_COPY_TO_CPU_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_SYNC_COPY_TO_CPU_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp sync-cpu-en            :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("signaling-cpu-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SIGNALING_COPY_TO_CPU_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_SIGNALING_COPY_TO_CPU_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp signaling-cpu-en         :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("management-cpu-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_MANAGEMENT_COPY_TO_CPU_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_MANAGEMENT_COPY_TO_CPU_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp management-cpu-en      :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("delay-request-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_DELAY_REQUEST_PROCESS_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_DELAY_REQUEST_PROCESS_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp delay-request-en       :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-based-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_PORT_BASED_PTP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp port-based-en          :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cache-aging-time");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_CACHE_AGING_TIME, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp cache-aging-time       :%d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("interface-selected");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_global_property(g_api_lchip, CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT, &value);
        }
        else
        {
            ret = ctc_ptp_get_global_property(CTC_PTP_GLOBAL_PROP_SYNC_OR_TOD_INPUT_SELECT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("ptp interface-selected     :%d\n", value);
        }
    }

    ctc_cli_out("=================================\n");

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_ptp_show_clock_time,
        ctc_cli_ptp_show_clock_time_cmd,
        "show ptp CHIP time",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Local clock time")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_time_t ts;

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_clock_timestamp(g_api_lchip, &ts);
    }
    else
    {
        ret = ctc_ptp_get_clock_timestamp(lchip, &ts);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d clock time:\n", lchip);
    ctc_cli_out("second             :%u\n", ts.seconds);
    ctc_cli_out("nanosecond         :%u\n", ts.nanoseconds);
    ctc_cli_out("nano nanosecond    :%u\n", ts.nano_nanoseconds);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_adjust_clock_offset,
        ctc_cli_ptp_adjust_clock_offset_cmd,
        "ptp CHIP adjust-offset (negative |) (seconds SECOND) (nanoseconds NS) ",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Adjust clock offset",
        "The offset is negative",
        "Seconds of offset",
        "<0-0xFFFFFFFF>",
        "Nanoseconds of offset",
        "<0-(10^9-1)>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_ptp_time_t offset;

    sal_memset(&offset, 0, sizeof(offset));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("negative");
    if (index != 0xFF)
    {
        offset.is_negative = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("seconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("Seconds of offset", offset.seconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nanoseconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nanoseconds of offset", offset.nanoseconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_adjust_clock_offset(g_api_lchip, &offset);
    }
    else
    {
        ret = ctc_ptp_adjust_clock_offset(lchip, &offset);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_clock_drift,
        ctc_cli_ptp_set_clock_drift_cmd,
        "ptp CHIP drift (negative | ) (nanoseconds NS)",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Clock drift",
        "The drift is negative",
        "Nanoseconds of drift",
        "<0-(10^9/quanta-1)>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_ptp_time_t drift;

    sal_memset(&drift, 0, sizeof(drift));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("negative");
    if (index != 0xFF)
    {
        drift.is_negative = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nanoseconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nanoseconds of offset", drift.nanoseconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_set_clock_drift(g_api_lchip, &drift);
    }
    else
    {
        ret = ctc_ptp_set_clock_drift(lchip, &drift);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_clock_drift,
        ctc_cli_ptp_show_clock_drift_cmd,
        "show ptp CHIP drift",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Local clock drift")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_time_t drift;
    char* str = "FALSE";

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_clock_drift(g_api_lchip, &drift);
    }
    else
    {
        ret = ctc_ptp_get_clock_drift(lchip, &drift);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    if (drift.is_negative)
    {
        str = "true";
    }

    ctc_cli_out("local chip %d clock drift:\n", lchip);
    ctc_cli_out("is negative        :%s\n", str);
    ctc_cli_out("nanosecond         :%u\n", drift.nanoseconds);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_device_type,
        ctc_cli_ptp_set_device_type_cmd,
        "ptp device-type ( none | oc | bc | e2e-tc | p2p-tc) ",
        CTC_CLI_PTP_M_STR,
        "PTP device type",
        "Not PTP device,process the PTP massage as normal packet",
        "Ordinary clock",
        "Boundary clock",
        "End-to-end transparent clock or E2ETC+OC/BC",
        "Peer-to-peer transparent clock or P2PTC+OC/BC")
{
    int32 ret = 0;
    ctc_ptp_device_type_t type = MAX_CTC_PTP_DEVICE;

    if (CLI_CLI_STR_EQUAL("none", 0))
    {
        type = CTC_PTP_DEVICE_NONE;
    }
    else if (CLI_CLI_STR_EQUAL("oc", 0))
    {
        type = CTC_PTP_DEVICE_OC;
    }
    else if (CLI_CLI_STR_EQUAL("bc", 0))
    {
        type = CTC_PTP_DEVICE_BC;
    }
    else if (CLI_CLI_STR_EQUAL("e2e-tc", 0))
    {
        type = CTC_PTP_DEVICE_E2E_TC;
    }
    else if (CLI_CLI_STR_EQUAL("p2p-tc", 0))
    {
        type = CTC_PTP_DEVICE_P2P_TC;
    }

    if (type != MAX_CTC_PTP_DEVICE)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_set_device_type(g_api_lchip, type);
        }
        else
        {
            ret = ctc_ptp_set_device_type(type);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_device_type,
        ctc_cli_ptp_show_device_type_cmd,
        "show ptp device-type",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        "PTP device type")
{
    int32 ret = 0;
    ctc_ptp_device_type_t type;
    char* str = NULL;

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_device_type(g_api_lchip, &type);
    }
    else
    {
        ret = ctc_ptp_get_device_type(&type);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    switch (type)
    {
    case CTC_PTP_DEVICE_NONE:
        str = "none";
        break;

    case CTC_PTP_DEVICE_OC:
        str = "oc";
        break;

    case CTC_PTP_DEVICE_BC:
        str = "bc";
        break;

    case CTC_PTP_DEVICE_E2E_TC:
        str = "e2e-tc";
        break;

    case CTC_PTP_DEVICE_P2P_TC:
        str = "p2p-tc";
        break;

    default:
        return CLI_ERROR;

    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("ptp device-type    :%s\n", str);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_adjust_delay,
        ctc_cli_ptp_set_adjust_delay_cmd,
        "ptp adjust-delay GPHYPORT_ID (ingress-latency VALUE|) (egress-latency VALUE|)  \
                                      (ingress-asymmetry HIGH4_VALUE LOW32_VALUE (negative|)|) \
                                      (egress-asymmetry HIGH4_VALUE LOW32_VALUE (negative|)|)  \
                                      (path-delay HIGH4_VALUE LOW32_VALUE|)",
        CTC_CLI_PTP_M_STR,
        "Delay value used for adjust PTP message pass the device",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Latency from phy to mac on RX",
        "<0-0xFFFF>",
        "Latency from mac to phy on TX",
        "<0-0xFFFF>",
        "Asymmetry on ingress",
        "high 4bits value <0-0xF>",
        "low 32bits value <0-0xFFFFFFFF>",
        "The value is negative",
        "Asymmetry on egress",
        "high 4bits value <0-0xF>",
        "low 32bits value <0-0xFFFFFFFF>",
        "The value is negative",
        "Path delay",
        "high 4bits value <0-0xF>",
        "low 32bits value <0-0xFFFFFFFF>")
{
    int32  ret = 0;
    uint8  index = 0xFF;
    uint32 gport = 0;
    uint32 ingress_latency = 0;
    uint32 egress_latency = 0;
    uint32 ingress_asymmetry_low32 = 0;
    uint32 ingress_asymmetry_high4 = 0;
    int64  ingress_asymmetry = 0;
    int64  egress_asymmetry_low32 = 0;
    uint32 egress_asymmetry_high4 = 0;
    int64  egress_asymmetry = 0;
    uint32 path_delay_low32 = 0;
    uint32 path_delay_high4 = 0;
    int64  path_delay = 0;

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress-latency");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("ingress-latency", ingress_latency, argv[index + 1]);

        if (ingress_latency > CTC_MAX_UINT16_VALUE)
        {
            ctc_cli_out("% Invalid ingress-latency value\n");
            return CLI_ERROR;
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_ptp_set_adjust_delay(g_api_lchip, gport, CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY, ingress_latency);
        }
        else
        {
            ret = ctc_ptp_set_adjust_delay(gport, CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY, ingress_latency);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-latency");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("egress-latency", egress_latency, argv[index + 1]);

        if (egress_latency > CTC_MAX_UINT16_VALUE)
        {
            ctc_cli_out("% Invalid egress-latency value\n");
            return CLI_ERROR;
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_ptp_set_adjust_delay(g_api_lchip, gport, CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY, egress_latency);
        }
        else
        {
            ret = ctc_ptp_set_adjust_delay(gport, CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY, egress_latency);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-asymmetry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("ingress-asymmetry", ingress_asymmetry_high4, argv[index + 1]);

        if (ingress_asymmetry_high4 > 0xF)
        {
            ctc_cli_out("%% Invalid ingress asymmetry high 4bits value\n");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32("ingress-asymmetry low 32bits", ingress_asymmetry_low32, argv[index + 2]);

        ingress_asymmetry = ((uint64)ingress_asymmetry_high4 << 32) | ingress_asymmetry_low32;

        index = CTC_CLI_GET_ARGC_INDEX("negative");
        if (0xFF != index)
        {
            ingress_asymmetry = -ingress_asymmetry;
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_ptp_set_adjust_delay(g_api_lchip, gport, CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY, ingress_asymmetry);
        }
        else
        {
            ret = ctc_ptp_set_adjust_delay(gport, CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY, ingress_asymmetry);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-asymmetry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("egress-asymmetry", egress_asymmetry_high4, argv[index + 1]);

        if (egress_asymmetry_high4 > 0xF)
        {
            ctc_cli_out("%% Invalid egress asymmetry high 4bits value!\n");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32("egress-asymmetry low 32bits", egress_asymmetry_low32, argv[index + 2]);
        egress_asymmetry = ((uint64)egress_asymmetry_high4 << 32) | egress_asymmetry_low32;

        index = CTC_CLI_GET_ARGC_INDEX("negative");
        if (0xFF != index)
        {
            egress_asymmetry = -egress_asymmetry;
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_ptp_set_adjust_delay(g_api_lchip, gport, CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY, egress_asymmetry);
        }
        else
        {
            ret = ctc_ptp_set_adjust_delay(gport, CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY, egress_asymmetry);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("path-delay");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("path-delay", path_delay_high4, argv[index + 1]);

        if (path_delay_high4 > 0xF)
        {
            ctc_cli_out("%% Invalid path-delay high 4bits value\n");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32("path-delay low 32bits", path_delay_low32, argv[index + 2]);
        path_delay = ((uint64)path_delay_high4 << 32) | path_delay_low32;

        if(g_ctcs_api_en)
        {
            ret = ctcs_ptp_set_adjust_delay(g_api_lchip, gport, CTC_PTP_ADJUST_DELAY_PATH_DELAY, path_delay);
        }
        else
        {
            ret = ctc_ptp_set_adjust_delay(gport, CTC_PTP_ADJUST_DELAY_PATH_DELAY, path_delay);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_adjust_delay,
        ctc_cli_ptp_show_adjust_delay_cmd,
        "show ptp adjust-delay GPHYPORT_ID delay-type ( ingress-latency | egress-latency | ingress-asymmetry | egress-asymmetry | path-delay)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        "Delay value used for adjust PTP message pass the device",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Delay type",
        "Latency from phy to mac on RX",
        "Latency from mac to phy on TX",
        "Asymmetry on ingress",
        "Asymmetry on egress",
        "Path delay")
{
    int32 ret = 0;
    uint32 gport = 0;
    int64 delay_value = 0;
    uint32 high_value = 0;
    uint32 low_value = 0;
    ctc_ptp_adjust_delay_type_t type = MAX_CTC_PTP_ADJUST_DELAY;
    char* str = NULL;
    char* true_or_false = "false";

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);

    if (CLI_CLI_STR_EQUAL("ingress-latency", 1))
    {
        type = CTC_PTP_ADJUST_DELAY_INGRESS_LATENCY;
        str = "ptp ingress latency value";
    }
    else if (CLI_CLI_STR_EQUAL("egress-latency", 1))
    {
        type = CTC_PTP_ADJUST_DELAY_EGRESS_LATENCY;
        str = "ptp egress latency value";
    }
    else if (CLI_CLI_STR_EQUAL("ingress-asymmetry", 1))
    {
        type = CTC_PTP_ADJUST_DELAY_INGRESS_ASYMMETRY;
        str = "ptp ingress asymmetry value";
    }
    else if (CLI_CLI_STR_EQUAL("egress-asymmetry", 1))
    {
        type = CTC_PTP_ADJUST_DELAY_EGRESS_ASYMMETRY;
        str = "ptp ingress asymmetry value";
    }
    else if (CLI_CLI_STR_EQUAL("path-delay", 1))
    {
        type = CTC_PTP_ADJUST_DELAY_PATH_DELAY;
        str = "ptp path latency value";
    }

    if (type != MAX_CTC_PTP_ADJUST_DELAY)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_ptp_get_adjust_delay(g_api_lchip, gport, type, &delay_value);
        }
        else
        {
            ret = ctc_ptp_get_adjust_delay(gport, type, &delay_value);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (delay_value < 0)
    {
        true_or_false = "true";
        delay_value = -delay_value;
    }

    high_value = (delay_value >> 32) & 0xFFFFFFFF;
    low_value = delay_value & 0xFFFFFFFF;

    ctc_cli_out("==============================\n");
    ctc_cli_out("%s:\n", str);
    ctc_cli_out("is negtive         :%s\n", true_or_false);
    ctc_cli_out("high 32-bit value  :%u\n", high_value);
    ctc_cli_out("low 32-bit value   :%u\n", low_value);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_sync_intf,
        ctc_cli_ptp_set_sync_intf_cmd,
        "ptp CHIP sync (mode ( input | output)) (lock (enable | disable)) (accuracy ACCURACY) (sync-clock-hz SYNC_CLOCK_HZ) \
        (sync-pulse-hz SYNC_PULSE_HZ) (sync-pulse-duty SYNC_PULSE_DUTY) (epoch EPOCH)",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Sync interface config",
        "Sync interface work mode",
        "Input mode",
        "Out mode",
        "Set lock",
        "Enable lock",
        "Disable lock",
        "Timer accuracy",
        "<32-49,or 0xFE>",
        "SyncClock signal frequency",
        "<0-25000000>",
        "SyncPulse signal frequency",
        "<0-250000>",
        "SyncPulse signal duty",
        "<1-99>",
        "Bit clock cycles before time code",
        "<0-63>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_ptp_sync_intf_cfg_t sync_intf_cfg;

    sal_memset(&sync_intf_cfg, 0, sizeof(sync_intf_cfg));
    sync_intf_cfg.mode = 0;
    sync_intf_cfg.lock = 0;

    CTC_CLI_GET_UINT8("local-chip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (index != 0xFF)
    {
        if (0 == sal_memcmp("ou", argv[index + 1], 2))
        {
            sync_intf_cfg.mode = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("lock");
    if (index != 0xFF)
    {
        if (0 == sal_memcmp("en", argv[index + 1], 2))
        {
            sync_intf_cfg.lock = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("accuracy");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("accuracy", sync_intf_cfg.accuracy, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sync-clock-hz");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("sync-clock-hz", sync_intf_cfg.sync_clock_hz, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sync-pulse-hz");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("sync-pulse-hz", sync_intf_cfg.sync_pulse_hz, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sync-pulse-duty");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("sync-pulse-duty", sync_intf_cfg.sync_pulse_duty, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("epoch");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("epoch", sync_intf_cfg.epoch, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_set_sync_intf(g_api_lchip, &sync_intf_cfg);
    }
    else
    {
        ret = ctc_ptp_set_sync_intf(lchip, &sync_intf_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_sync_intf,
        ctc_cli_ptp_show_sync_intf_cmd,
        "show ptp CHIP sync",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Sync interface config")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_sync_intf_cfg_t sync_intf_cfg;
    char* mode = "input";
    char* lock = "disable";

    sal_memset(&sync_intf_cfg, 0, sizeof(sync_intf_cfg));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_sync_intf(g_api_lchip, &sync_intf_cfg);
    }
    else
    {
        ret = ctc_ptp_get_sync_intf(lchip, &sync_intf_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (1 == sync_intf_cfg.mode)
    {
        mode = "output";
    }

    if (1 == sync_intf_cfg.lock)
    {
        lock = "enable";
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d sync interface config:\n", lchip);
    ctc_cli_out("mode           :%s\n", mode);
    ctc_cli_out("lock           :%s\n", lock);
    ctc_cli_out("accuracy       :%u\n", sync_intf_cfg.accuracy);
    ctc_cli_out("sync-clock-hz  :%u\n", sync_intf_cfg.sync_clock_hz);
    ctc_cli_out("sync-pulse-hz  :%u\n", sync_intf_cfg.sync_pulse_hz);
    ctc_cli_out("sync-pulse-duty:%u\n", sync_intf_cfg.sync_pulse_duty);
    ctc_cli_out("epoch          :%u\n", sync_intf_cfg.epoch);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_sync_intf_toggle_time,
        ctc_cli_ptp_set_sync_intf_toggle_time_cmd,
        "ptp CHIP sync-toggle (seconds SECOND) (nanoseconds NS) (nano-nanoseconds NNS)",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Sync interface toggle time",
        "Seconds of offset",
        "<0-0xFFFFFFFF>",
        "Nanoseconds of offset",
        "<0-(10^9-1)>",
        "Nano nanoseconds of offset",
        "<0-(10^9-1)>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_ptp_time_t toggle;

    sal_memset(&toggle, 0, sizeof(toggle));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("seconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("Seconds of offset", toggle.seconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nanoseconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nanoseconds of offset", toggle.nanoseconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nano-nanoseconds");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nano-nanoseconds of offset", toggle.nano_nanoseconds, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_set_sync_intf_toggle_time(g_api_lchip, &toggle);
    }
    else
    {
        ret = ctc_ptp_set_sync_intf_toggle_time(lchip, &toggle);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_sync_intf_toggle_time,
        ctc_cli_ptp_show_sync_intf_toggle_time_cmd,
        "show ptp CHIP sync-toggle",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Sync interface toggle time")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_time_t toggle;

    sal_memset(&toggle, 0, sizeof(toggle));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_sync_intf_toggle_time(g_api_lchip, &toggle);
    }
    else
    {
        ret = ctc_ptp_get_sync_intf_toggle_time(lchip, &toggle);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d sync interface toggle time:\n", lchip);
    ctc_cli_out("second             :%u\n", toggle.seconds);
    ctc_cli_out("nanosecond         :%u\n", toggle.nanoseconds);
    ctc_cli_out("nano nanosecond    :%u\n", toggle.nano_nanoseconds);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_sync_intf_rx_code,
        ctc_cli_ptp_show_sync_intf_rx_code_cmd,
        "show ptp CHIP sync-rx-code",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Sync interface input time code")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_sync_intf_code_t input_code;

    sal_memset(&input_code, 0, sizeof(input_code));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_sync_intf_code(g_api_lchip, &input_code);
    }
    else
    {
        ret = ctc_ptp_get_sync_intf_code(lchip, &input_code);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d sync interface input time code:\n", lchip);
    ctc_cli_out("second             :%u\n", input_code.seconds);
    ctc_cli_out("nanosecond         :%u\n", input_code.nanoseconds);
    ctc_cli_out("lock               :%u\n", input_code.lock);
    ctc_cli_out("accuracy           :%u\n", input_code.accuracy);
    ctc_cli_out("crc_error          :%u\n", input_code.crc_error);
    ctc_cli_out("crc                :%u\n", input_code.crc);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_clear_sync_intf_rx_code,
        ctc_cli_ptp_clear_sync_intf_rx_code_cmd,
        "ptp CHIP sync-rx-code-clear",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Clear sync interface input time code")
{
    int32 ret = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_clear_sync_intf_code(g_api_lchip);
    }
    else
    {
        ret = ctc_ptp_clear_sync_intf_code(lchip);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_tod_intf,
        ctc_cli_ptp_set_tod_intf_cmd,
        "ptp CHIP tod (mode ( input | output)) (pulse-duty DUTY) (pulse-delay DELAY) (stop-bit NUMBER) (epoch EPOCH)",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Tod interface config",
        "Tod interface work mode",
        "Input mode",
        "Out mode",
        "1PPS signal duty",
        "<1-99>",
        "1PPS signal delay between master and slave",
        "<0-999999999>",
        "How many stop bits will be sent",
        "<0-0xFFFF>",
        "Bit clock cycles before time code",
        CTC_CLI_TOD_EPOCH_RANGE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_ptp_tod_intf_cfg_t ptp_tod_cfg;

    sal_memset(&ptp_tod_cfg, 0, sizeof(ptp_tod_cfg));
    ptp_tod_cfg.mode = 0;

    CTC_CLI_GET_UINT8("local-chip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (index != 0xFF)
    {
        if (0 == sal_memcmp("ou", argv[index + 1], 2))
        {
            ptp_tod_cfg.mode = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("pulse-duty");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("pulse-duty", ptp_tod_cfg.pulse_duty, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pulse-delay");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("pulse-delay", ptp_tod_cfg.pulse_delay, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("stop-bit");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("stop-bit", ptp_tod_cfg.stop_bit_num, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("epoch");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("epoch", ptp_tod_cfg.epoch, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_set_tod_intf(g_api_lchip, &ptp_tod_cfg);
    }
    else
    {
        ret = ctc_ptp_set_tod_intf(lchip, &ptp_tod_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_tod_intf,
        ctc_cli_ptp_show_tod_intf_cmd,
        "show ptp CHIP tod",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Tod interface config")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_tod_intf_cfg_t ptp_tod_cfg;
    char* mode = "input";

    sal_memset(&ptp_tod_cfg, 0, sizeof(ptp_tod_cfg));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_tod_intf(g_api_lchip, &ptp_tod_cfg);
    }
    else
    {
        ret = ctc_ptp_get_tod_intf(lchip, &ptp_tod_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (1 == ptp_tod_cfg.mode)
    {
        mode = "output";
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d tod interface config:\n", lchip);
    ctc_cli_out("mode           :%s\n", mode);
    ctc_cli_out("pulse-duty     :%u\n", ptp_tod_cfg.pulse_duty);
    ctc_cli_out("pulse-delay    :%u\n", ptp_tod_cfg.pulse_delay);
    ctc_cli_out("stop-bit       :%u\n", ptp_tod_cfg.stop_bit_num);
    ctc_cli_out("epoch          :%u\n", ptp_tod_cfg.epoch);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_set_tod_intf_tx_code,
        ctc_cli_ptp_set_tod_intf_tx_code_cmd,
        "ptp CHIP tod-tx-code (class CLASS) (id ID) (length LEN) (leap LEAP) (1pps-status PPSTATUS) (accuracy ACCURACY) \
        (clock-src SRC) (src-status SRCSTATUS) (alarm ALARM)",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Tod interface output time code config",
        "Class of messag",
        "<0-0xFF>",
        "Identify of this message",
        "<0-0xFF>",
        "Length of the payload of this message",
        "<0-0xFFFF>",
        "Leap seconds (GPS-UTC)",
        "<-128 - 127>",
        "1PPS status",
        "<0-255>",
        "1PPS accuracy",
        "<0-0xFF>",
        "Clock source",
        "<0-3>",
        "Clock source work status",
        "<0-0xFFFF>",
        "Monitor alarm",
        "<0-0xFFFF>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    int16 leap_second = 0;
    ctc_ptp_tod_intf_code_t tod_tx_code;

    sal_memset(&tod_tx_code, 0, sizeof(tod_tx_code));

    CTC_CLI_GET_UINT8("local-chip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("class");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("class", tod_tx_code.msg_class, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("id", tod_tx_code.msg_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("length");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("length", tod_tx_code.msg_length, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("leap");
    if (index != 0xFF)
    {
        CTC_CLI_GET_INTEGER_RANGE("leap", leap_second, argv[index + 1], -128, 127);
    }

    tod_tx_code.leap_second = leap_second & 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("1pps-status");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("1pps-status", tod_tx_code.pps_status, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("accuracy");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("accuracy", tod_tx_code.pps_accuracy, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("clock-src");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("clock-src", tod_tx_code.clock_src, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-status");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-status", tod_tx_code.clock_src_status, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("alarm");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("alarm", tod_tx_code.monitor_alarm, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_set_tod_intf_tx_code(g_api_lchip, &tod_tx_code);
    }
    else
    {
        ret = ctc_ptp_set_tod_intf_tx_code(lchip, &tod_tx_code);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_tod_intf_tx_code,
        ctc_cli_ptp_show_tod_intf_tx_code_cmd,
        "show ptp CHIP tod-tx-code",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Tod interface output time code config")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_tod_intf_code_t tod_tx_code;

    sal_memset(&tod_tx_code, 0, sizeof(tod_tx_code));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_tod_intf_tx_code(g_api_lchip, &tod_tx_code);
    }
    else
    {
        ret = ctc_ptp_get_tod_intf_tx_code(lchip, &tod_tx_code);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d tod interface output time code config:\n", lchip);
    ctc_cli_out("class          :%u\n", tod_tx_code.msg_class);
    ctc_cli_out("id             :%u\n", tod_tx_code.msg_id);
    ctc_cli_out("length         :%u\n", tod_tx_code.msg_length);
    ctc_cli_out("leap           :%d\n", tod_tx_code.leap_second);
    ctc_cli_out("1pps-status    :%u\n", tod_tx_code.pps_status);
    ctc_cli_out("accuracy       :%u\n", tod_tx_code.pps_accuracy);
    ctc_cli_out("clock-src      :%u\n", tod_tx_code.clock_src);
    ctc_cli_out("src-status     :%u\n", tod_tx_code.clock_src_status);
    ctc_cli_out("alarm          :%u\n", tod_tx_code.monitor_alarm);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_tod_intf_rx_code,
        ctc_cli_ptp_show_tod_intf_rx_code_cmd,
        "show ptp CHIP tod-rx-code",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Tod interface input time code")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_ptp_tod_intf_code_t tod_rx_code;

    sal_memset(&tod_rx_code, 0, sizeof(tod_rx_code));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_tod_intf_rx_code(g_api_lchip, &tod_rx_code);
    }
    else
    {
        ret = ctc_ptp_get_tod_intf_rx_code(lchip, &tod_rx_code);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d tod interface input time code:\n", lchip);
    ctc_cli_out("message char0  :%u\n", tod_rx_code.msg_char0);
    ctc_cli_out("message char1  :%u\n", tod_rx_code.msg_char1);
    ctc_cli_out("message class  :%u\n", tod_rx_code.msg_class);
    ctc_cli_out("message id     :%u\n", tod_rx_code.msg_id);
    ctc_cli_out("message length :%u\n", tod_rx_code.msg_length);
    ctc_cli_out("week seconds   :%u\n", tod_rx_code.gps_second_time_of_week);
    ctc_cli_out("week           :%u\n", tod_rx_code.gps_week);
    ctc_cli_out("leap seconds   :%u\n", tod_rx_code.leap_second);
    ctc_cli_out("1pps status    :%u\n", tod_rx_code.pps_status);
    ctc_cli_out("1pps accuracy  :%u\n", tod_rx_code.pps_accuracy);
    ctc_cli_out("crc            :%u\n", tod_rx_code.crc);
    ctc_cli_out("crc error      :%u\n", tod_rx_code.crc_error);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_clear_tod_intf_rx_code,
        ctc_cli_ptp_clear_tod_intf_rx_code_cmd,
        "ptp CHIP tod-rx-code-clear",
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Clear tod interface input time code")
{
    int32 ret = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_clear_tod_intf_code(g_api_lchip);
    }
    else
    {
        ret = ctc_ptp_clear_tod_intf_code(lchip);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_show_captured_ts,
        ctc_cli_ptp_show_captured_ts_cmd,
        "show ptp CHIP captured-ts type (pkt-tx lport LPORT seq-id ID | (sync-pulse-rx | tod-1pps-rx) (seq-id ID)) ",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PTP_M_STR,
        CTC_CLI_CHIP_ID_DESC,
        "Timestamp captured by local clock",
        "Captured type",
        "Ts captured by mac tx",
        "Local port",
        "<0-0xFF>",
        "Sequence ID of the ts",
        "<0-3>",
        "Ts captured by syncPulse rx",
        "Ts captured by ToD 1PPS rx",
        "Sequence ID of the ts",
        "<0-15>")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 is_tx = 0;
    ctc_ptp_capured_ts_t captured_ts;
    char* str = NULL;

    sal_memset(&captured_ts, 0, sizeof(captured_ts));

    CTC_CLI_GET_UINT8("Local-lchip", lchip, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("pkt-tx");
    if (index != 0xFF)
    {
        captured_ts.type = CTC_PTP_CAPTURED_TYPE_TX;
        CTC_CLI_GET_UINT16_RANGE("lport", captured_ts.u.lport, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("seq-id", captured_ts.seq_id, argv[index + 4], 0, CTC_MAX_UINT8_VALUE);
        str = "packet tx ts";
        is_tx = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sync-pulse-rx");
    if (index != 0xFF)
    {
        captured_ts.type = CTC_PTP_CAPTURED_TYPE_RX;
        captured_ts.u.captured_src = CTC_PTP_TIEM_INTF_SYNC_PULSE;
        CTC_CLI_GET_UINT8_RANGE("seq-id", captured_ts.seq_id, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        str = "syncpulse rx ts";
    }

    index = CTC_CLI_GET_ARGC_INDEX("tod-1pps-rx");
    if (index != 0xFF)
    {
        captured_ts.type = CTC_PTP_CAPTURED_TYPE_RX;
        captured_ts.u.captured_src = CTC_PTP_TIEM_INTF_TOD_1PPS;
        CTC_CLI_GET_UINT8_RANGE("seq-id", captured_ts.seq_id, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        str = "tod 1pps rx ts";
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_get_captured_ts(g_api_lchip, &captured_ts);
    }
    else
    {
        ret = ctc_ptp_get_captured_ts(lchip, &captured_ts);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");
    ctc_cli_out("local chip %d captured %s:\n", lchip, str);
    if (is_tx)
    {
        ctc_cli_out("lport              :%u\n", captured_ts.u.lport);
    }

    ctc_cli_out("Sequence id        :%u\n", captured_ts.seq_id);
    ctc_cli_out("second             :%u\n", captured_ts.ts.seconds);
    ctc_cli_out("nanosecond         :%u\n", captured_ts.ts.nanoseconds);
    ctc_cli_out("==============================\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_add_clock,
        ctc_cli_ptp_add_clock_cmd,
        "ptp add clock-index INDEX msg-type TYPE {copy-to-cpu | discard | residence-time | path-delay | igs-asym-delay | egs-asym-delay}",
        CTC_CLI_PTP_M_STR,
        "Add clock",
        "Clock index",
        "<2-7>, 0 is disable, 1 is reserved",
        "Message type",
        "<0-15>",
        "Copy to cpu",
        "Discard",
        "Residence time",
        "Path delay",
        "Ingress asym delay",
        "egress asym delay")
{
    uint8 index = 0;
    uint8 type = 0;
    int32 ret = CLI_SUCCESS;
    ctc_ptp_clock_t ptp_clock;

    sal_memset(&ptp_clock, 0, sizeof(ctc_ptp_clock_t));
    CTC_CLI_GET_UINT8("clock-id", ptp_clock.clock_id, argv[0]);
    CTC_CLI_GET_UINT8_RANGE("type",   type, argv[1], 0, CTC_PTP_MSG_TYPE_MAX-1);

    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_COPY_TO_CPU);
    }
    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_DISCARD);
    }
    index = CTC_CLI_GET_ARGC_INDEX("residence-time");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_RESIDENCE_TIME);
    }
    index = CTC_CLI_GET_ARGC_INDEX("path-delay");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_PATH_DELAY);
    }
    index = CTC_CLI_GET_ARGC_INDEX("igs-asym-delay");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_IGS_ASYM_DELAY);
    }
    index = CTC_CLI_GET_ARGC_INDEX("egs-asym-delay");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ptp_clock.action_flag[type], CTC_PTP_ACTION_FLAG_EGS_ASYM_DELAY);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_add_device_clock(g_api_lchip, &ptp_clock);
    }
    else
    {
        ret = ctc_ptp_add_device_clock(&ptp_clock);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ptp_remove_clock,
        ctc_cli_ptp_remove_clock_cmd,
        "ptp remove clock-index INDEX",
        CTC_CLI_PTP_M_STR,
        "Remove clock",
        "Clock index",
        "Index",
        "<0-X>")
{
    int32  ret = 0;
    ctc_ptp_clock_t ptp_clock;

    sal_memset(&ptp_clock, 0, sizeof(ctc_ptp_clock_t));

    CTC_CLI_GET_UINT8("clock-id", ptp_clock.clock_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ptp_remove_device_clock(g_api_lchip, &ptp_clock);
    }
    else
    {
        ret = ctc_ptp_remove_device_clock(&ptp_clock);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/**
 @brief  Initialize sdk ptp module CLIs
*/
int32
ctc_ptp_cli_init(void)
{
    /* ptp cli under sdk mode */
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_global_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_adjust_clock_offset_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_clock_drift_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_device_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_adjust_delay_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_sync_intf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_sync_intf_toggle_time_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_tod_intf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_set_tod_intf_tx_code_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_clear_sync_intf_rx_code_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_global_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_clock_time_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_clock_drift_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_device_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_adjust_delay_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_sync_intf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_sync_intf_toggle_time_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_sync_intf_rx_code_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_tod_intf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_tod_intf_tx_code_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_tod_intf_rx_code_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_clear_tod_intf_rx_code_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_show_captured_ts_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_add_clock_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ptp_remove_clock_cmd);

    return CLI_SUCCESS;
}

