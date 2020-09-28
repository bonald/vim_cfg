/**
 @file ctc_efd_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_efd.h"
#include "ctc_efd_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_debug.h"

CTC_CLI(ctc_cli_efd_debug_on,
        ctc_cli_efd_debug_on_cmd,
        "debug efd (ctc | sys) (debug-level {func|param|info|error|export} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_EFD_M_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR,
        CTC_CLI_DEBUG_LEVEL_EXPORT)
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

        index = CTC_CLI_GET_ARGC_INDEX("export");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_EXPORT;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PARSER_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PARSER_SYS;
    }

    ctc_debug_set_flag("efd", "efd", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_efd_debug_off,
        ctc_cli_efd_debug_off_cmd,
        "no debug efd (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_EFD_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = EFD_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = EFD_SYS;
    }

    ctc_debug_set_flag("efd", "efd", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_efd_show_debug,
        ctc_cli_efd_show_debug_cmd,
        "show debug efd (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_EFD_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = EFD_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = EFD_SYS;
    }
    en = ctc_debug_get_flag("efd", "efd", typeenum, &level);
    ctc_cli_out("efd:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_efd_set_global_ctl,
        ctc_cli_efd_set_global_ctl_cmd,
        "efd set global-property (tcp-mode|priority-en|priority|color-en|color|min-pkt-len|detect-rate|ipg-en|pps-en|randomlog-en|randomlog-rate|fuzzy-matching-en|efd-en| \
        detect-granu|detect-interval|aging-mult|redirect FLOW_ID) value VALUE",
        CTC_CLI_EFD_M_STR,
        "Set",
        "Global property",
        "Tcp mode",
        "Priority enable",
        "Priority",
        "Color enable",
        "Color",
        "Min packet length",
        "Detect rate, unit:kbps or kpps",
        "IPG enable",
        "Pps mode enable, disable is bps mode",
        "Random log enable",
        "Random log rate, percent = 1/(2^(15-rate)",
        "Fuzzy match mode enable,disable is exact match mode",
        "EFD enable,default is enable",
        "Detect granularity of packet length, 0:16B, 1:8B, 2:4B, 3:32B",
        "Learning and aging time interval,unit:ms",
        "Aging time multiplier",
        "Redirect nexthop id per flow id/global control",
        "Flow id",
        "Value",
        "Value")
{
    uint8  index = 0;
    uint32 value = 0;
    int32  ret  = CLI_SUCCESS;
    uint32 type = 0;
    uint32 nh_value[2] = {0};

    index = CTC_CLI_GET_ARGC_INDEX("tcp-mode");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_MODE_TCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_PRIORITY_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_PRIORITY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("color-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_COLOR_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("color");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_COLOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("min-pkt-len");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_MIN_PKT_LEN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detect-rate");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_DETECT_RATE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipg-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_IPG_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pps-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_PPS_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("randomlog-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_RANDOM_LOG_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("randomlog-rate");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_RANDOM_LOG_RATE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fuzzy-matching-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_FUZZY_MATCH_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("efd-en");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_EFD_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("detect-granu");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_DETECT_GRANU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("detect-interval");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_DETECT_INTERVAL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aging-mult");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_AGING_MULT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("redirect");
    if (index != 0xFF)
    {
        type = CTC_EFD_GLOBAL_REDIRECT;
        CTC_CLI_GET_UINT32("flow id", nh_value[0], argv[index + 1]);
        CTC_CLI_GET_UINT32("nexthop id", nh_value[1], argv[index + 2]);

        if (g_ctcs_api_en)
        {
            ret = ctcs_efd_set_global_ctl(g_api_lchip, type, &nh_value);
        }
        else
        {
            ret = ctc_efd_set_global_ctl(type, &nh_value);
        }
    }
    else
    {
        CTC_CLI_GET_UINT32("value", value, argv[1]);
        if (g_ctcs_api_en)
        {
            ret = ctcs_efd_set_global_ctl(g_api_lchip, type, &value);
        }
        else
        {
            ret = ctc_efd_set_global_ctl(type, &value);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_efd_show_global_ctl,
        ctc_cli_efd_show_global_ctl_cmd,
        "show efd global-property",
        CTC_CLI_SHOW_STR,
        CTC_CLI_EFD_M_STR,
        "Global property")
{
    uint32 value = 0;
    int32  ret  = CLI_SUCCESS;
    uint32 bmp_value[64] = {0};
    uint8 index = 0;
    uint32 nh_value[2] = {0};

    ctc_cli_out("Type                       Value\n", value);
    ctc_cli_out("---------------------------------\n", value);

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_EFD_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_EFD_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "EFD-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_MODE_TCP, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_MODE_TCP, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Tcp-mode", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_PRIORITY_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_PRIORITY_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Priority-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_PRIORITY, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_PRIORITY, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Priority", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_COLOR_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_COLOR_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Color-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_COLOR, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_COLOR, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Color", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_MIN_PKT_LEN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_MIN_PKT_LEN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Min pkt len", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_IPG_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_IPG_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Ipg-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_PPS_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_PPS_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Pps-en", value);
    }

    if(0 == value)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_DETECT_RATE, &value);
        }
        else
        {
            ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_DETECT_RATE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s:  %u\n", "Detect rate(kbps)", value);
        }
    }
    if(1 == value)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_DETECT_RATE, &value);
        }
        else
        {
            ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_DETECT_RATE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s:  %u\n", "Detect rate(kpps)", value);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_FUZZY_MATCH_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_FUZZY_MATCH_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Fuzzy-match-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_RANDOM_LOG_EN, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_RANDOM_LOG_EN, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Random-log-en", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_RANDOM_LOG_RATE, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_RANDOM_LOG_RATE, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Random-log-rate", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_DETECT_GRANU, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_DETECT_GRANU, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Detect-granularity", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_DETECT_INTERVAL, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_DETECT_INTERVAL, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Detect-interval(ms)", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_AGING_MULT, &value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_AGING_MULT, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:  %u\n", "Aging-multi", value);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_FLOW_ACTIVE_BMP, &bmp_value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_FLOW_ACTIVE_BMP, &bmp_value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s \n", "Flow-active-bmp");
        for (index = 0; index < 64; index++)
        {
            ctc_cli_out("%14s %-4u-%-4u:  0x%x\n", "bit ",index*32, (index+1)*32-1, bmp_value[index]);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_efd_get_global_ctl(g_api_lchip, CTC_EFD_GLOBAL_REDIRECT, &nh_value);
    }
    else
    {
        ret = ctc_efd_get_global_ctl(CTC_EFD_GLOBAL_REDIRECT, &nh_value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-24s:   %u\n", "Redirect nhid", nh_value[1]);
    }

    return ret;
}


int32
ctc_efd_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_efd_set_global_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_efd_show_global_ctl_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_efd_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_efd_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_efd_show_debug_cmd);
    return 0;
}

