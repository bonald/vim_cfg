/**
 @file ctc_learning_aging_cli.c

 @date 2010-3-1

 @version v2.0

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

#include "ctc_learning_aging_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_learning_aging.h"
#include "ctc_error.h"
#include "ctc_debug.h"


CTC_CLI(ctc_cli_learning_aging_debug_on,
        ctc_cli_learning_aging_debug_on_cmd,
        "debug learning-aging (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Ctc layer",
        "Sys layer",
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
        typeenum = L2_LEARNING_AGING_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L2_LEARNING_AGING_SYS;
    }

    ctc_debug_set_flag("l2", "learning_aging", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_learning_aging_debug_off,
        ctc_cli_learning_aging_debug_off_cmd,
        "no debug learning-aging (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = L2_LEARNING_AGING_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L2_LEARNING_AGING_SYS;
    }

    ctc_debug_set_flag("l2", "learning_aging", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_learning_aging_show_debug,
        ctc_cli_learning_aging_show_debug_cmd,
        "show debug learning-aging (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = L2_LEARNING_AGING_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L2_LEARNING_AGING_SYS;
    }

    en = ctc_debug_get_flag("l2", "learning_aging", typeenum, &level);
    ctc_cli_out("learning_aging:%s debug %s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

/******************
 Learning CLI CMD
******************/
CTC_CLI(ctc_cli_learning_aging_set_learning_action,
        ctc_cli_learning_aging_set_learning_action_cmd,
        "learning action (always-cpu|cache-full-to-cpu threshold THRESHOLD | cache-only  threshold THRESHOLD |dont-learn |(mac-table-full (enable|disable)))",
        CTC_CLI_LEARNING_STR,
        "Learning action",
        "Learning action always cpu",
        "Learning action full to cpu",
        "Cache threshod",
        CTC_CLI_LEARNING_CACHE_NUM,
        "Learning action cache only",
        "Cache threshod",
        CTC_CLI_LEARNING_CACHE_NUM,
        "Learning action dont learn",
        "Learning mac table full",
        "Learning mac table full enable",
        "Learning mac table full disable")
{
    ctc_learning_action_info_t learning_action;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&learning_action, 0, sizeof(learning_action));
    if (0 == sal_memcmp("always-cpu", argv[0], 10))
    {
        learning_action.action = CTC_LEARNING_ACTION_ALWAYS_CPU;
    }
    else if (0 == sal_memcmp("cache-full-to-cpu", argv[0], 17))
    {
        learning_action.action = CTC_LEARNING_ACTION_CACHE_FULL_TO_CPU;
    }
    else if (0 == sal_memcmp("cache-only", argv[0], 10))
    {
        learning_action.action = CTC_LEARNING_ACTION_CACHE_ONLY;
    }
    else if (0 == sal_memcmp("dont-learn", argv[0], 10))
    {
        learning_action.action = CTC_LEARNING_ACTION_DONLEARNING;
    }

    index = CTC_CLI_GET_ARGC_INDEX("threshold");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("threshold", learning_action.value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-table-full");
    if (0xFF != index)
    {
        learning_action.action = CTC_LEARNING_ACTION_MAC_TABLE_FULL;
        index = CTC_CLI_GET_ARGC_INDEX("enable");
        if (0xFF != index)
        {
            learning_action.value = 1;
        }
        else
        {
            learning_action.value = 0;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_set_learning_action(g_api_lchip, &learning_action);
    }
    else
    {
        ret = ctc_set_learning_action(&learning_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_learning_aging_set_aging_property,
        ctc_cli_learning_aging_set_aging_property_cmd,
        "aging ({fifo-threshold THRESHOLD} | { interval INTERVAL } | { fifo-scan (enable | disable) }|\
        {one-round (enable | disable)}) ((aging-tbl (mac|scl)) |)",
        CTC_CLI_AGING_STR,
        "Fifo threshold",
        "Threshold value <0-15>",
        "Aging interval",
        "Aging interval(unit: seconds)",
        "Aging fifo scan",
        "Open aging fifo scan",
        "Close aging fifo scan",
        "Scan one round",
        "Enable",
        "Disable",
        "Aging table",
        "MAC table",
        "SCL table")
{
    ctc_aging_prop_t type = CTC_AGING_PROP_MAX;
    ctc_aging_table_type_t  tbl_type = CTC_AGING_TBL_MAC;
    uint32 value = 0;
    int32 ret = 0;

    uint32 index;

    /* set fifo threshold */
    index = CTC_CLI_GET_ARGC_INDEX("fifo-threshold");
    if (0xFF != index)
    {
        type = CTC_AGING_PROP_FIFO_THRESHOLD;
        if (argc <= (index + 1))
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("Fifo threshold", value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    /* set aging interval */
    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (0xFF != index)
    {
        type = CTC_AGING_PROP_INTERVAL;
        if (argc <= (index + 1))
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("Aging interval(unit: seconds)", value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    /* set aging scan en */
    index = CTC_CLI_GET_ARGC_INDEX("fifo-scan");
    if (0xFF != index)
    {
        type = CTC_AGING_PROP_AGING_SCAN_EN;
        if (argc <= (index + 1))
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        if (0 == sal_memcmp("enable", argv[index + 1], 6))
        {
            value = 1;
        }
        else if (0 == sal_memcmp("disable", argv[index + 1], 7))
        {
            value = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("one-round");
    if (0xFF != index)
    {
        type = CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED;
        if (argc <= (index + 1))
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        if (CLI_CLI_STR_EQUAL("enable", (index + 1)))
        {
            value = 1;
        }
        else
        {
            value = 0;
        }
    }

    /* set aging table */
    index = CTC_CLI_GET_ARGC_INDEX("aging-tbl");
    if (0xFF != index)
    {
        if (0 == sal_memcmp("scl", argv[index + 1], 4))
        {
            tbl_type = CTC_AGING_TBL_SCL;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aging_set_property(g_api_lchip, tbl_type, type, value);
    }
    else
    {
        ret = ctc_aging_set_property(tbl_type, type, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_learning_aging_show_config,
        ctc_cli_learning_aging_show_config_cmd,
        "show learning-aging config (aging-tbl (mac|scl) |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_LEARNING_AGING_STR,
        "Config param",
        "Aging table",
        "MAC table",
        "SCL table")
{
    uint32 value32 = 0;
    uint8 value8 = 0;
    int32 ret = 0;

    ctc_learning_action_info_t learning_action;
    ctc_aging_table_type_t  tbl_type = CTC_AGING_TBL_MAC;
    uint32 index  = 0;

    sal_memset(&learning_action, 0, sizeof(ctc_learning_action_info_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_get_learning_action(g_api_lchip, &learning_action);
    }
    else
    {
        ret = ctc_get_learning_action(&learning_action);
    }

    switch (learning_action.action)
    {
    case CTC_LEARNING_ACTION_ALWAYS_CPU:
        ctc_cli_out("learning-action is : always cpu \n");
        break;

    case CTC_LEARNING_ACTION_CACHE_FULL_TO_CPU:
        ctc_cli_out("learning-action is : cache full to cpu \n");
        ctc_cli_out("learning-cache-threshold:%d \n", learning_action.value);
        break;

    case CTC_LEARNING_ACTION_CACHE_ONLY:
        ctc_cli_out("learning-action is : cache only \n");
        ctc_cli_out("learning-cache-threshold:%d \n", learning_action.value);
        break;

    case CTC_LEARNING_ACTION_DONLEARNING:
        ctc_cli_out("learning-action is : dont learn \n");
        break;

    case CTC_LEARNING_ACTION_MAC_TABLE_FULL:
        ctc_cli_out("learning-action is : mac table full  \n");
        break;

    default:
        break;
    }

    /* set aging table */
    index = CTC_CLI_GET_ARGC_INDEX("aging-tbl");
    if (0xFF != index)
    {
        if (0 == sal_memcmp("scl", argv[index + 1], 4))
        {
            tbl_type = CTC_AGING_TBL_SCL;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_aging_get_property(g_api_lchip, tbl_type, CTC_AGING_PROP_FIFO_THRESHOLD, &value32);
    }
    else
    {
        ret = ctc_aging_get_property(tbl_type, CTC_AGING_PROP_FIFO_THRESHOLD, &value32);
    }
    value8 = value32 & 0xFF;
    ctc_cli_out("fifo_threshold:%d \n", value8);

    if(g_ctcs_api_en)
    {
        ret = ctcs_aging_get_property(g_api_lchip, tbl_type, CTC_AGING_PROP_INTERVAL, &value32);
    }
    else
    {
        ret = ctc_aging_get_property(tbl_type, CTC_AGING_PROP_INTERVAL, &value32);
    }
    ctc_cli_out("aging_interval:%d \n", value32);

    if(g_ctcs_api_en)
    {
        ret = ctcs_aging_get_property(g_api_lchip, tbl_type, CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED, &value32);
    }
    else
    {
        ret = ctc_aging_get_property(tbl_type, CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED, &value32);
    }

    ctc_cli_out("stop_scan_timer_expired:%s \n", (value32 == TRUE) ? "TRUE" : "FALSE");

    if(g_ctcs_api_en)
    {
        ret = ctcs_aging_get_property(g_api_lchip, tbl_type, CTC_AGING_PROP_AGING_SCAN_EN, &value32);
    }
    else
    {
        ret = ctc_aging_get_property(tbl_type, CTC_AGING_PROP_AGING_SCAN_EN, &value32);
    }

    ctc_cli_out("aging_scan_en:%s\n", (value32 == TRUE) ? "TRUE" : "FALSE");

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


int32
ctc_learning_aging_cli_init(void)
{
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_debug_on_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_debug_off_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_show_debug_cmd);

    /* Learning CLI */
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_set_learning_action_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_set_aging_property_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_learning_aging_show_config_cmd);

    return CLI_SUCCESS;
}

