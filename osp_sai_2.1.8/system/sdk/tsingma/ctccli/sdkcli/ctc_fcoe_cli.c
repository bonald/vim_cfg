/**
 @file ctc_fcoe_cli.c

 @date 2015-10-21

 @version v2.0

 The file apply clis of fcoe module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_fcoe_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_fcoe.h"



CTC_CLI(ctc_cli_fcoe_add_route,
        ctc_cli_fcoe_add_route_cmd,
        "fcoe add fcid FCID fid FID {(nexthop NHID|discard)|src-port GPORT_ID|macsa MACSA|src-discard}",
        CTC_CLI_FCOE_M_STR,
        "Add route",
        CTC_CLI_DEST_FCID_DESC,
        CTC_CLI_FCID_RANGE_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Nexthop",
        CTC_CLI_NH_ID_STR,
        "Dest fcid match discard",
        "RPF check src port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Check macsa",
        CTC_CLI_MAC_FORMAT,
        "Source fcid match discard")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_fcoe_route_t fcoe_info;

    sal_memset(&fcoe_info, 0, sizeof(ctc_fcoe_route_t));

    CTC_CLI_GET_UINT32_RANGE("fcid", fcoe_info.fcid, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("fid", fcoe_info.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nexthop");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("next-hop-id", fcoe_info.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");
    if (index != 0xFF)
    {
        fcoe_info.flag |= CTC_FCOE_FLAG_DST_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("src-port");
    if (index != 0xFF)
    {
        fcoe_info.flag |= CTC_FCOE_FLAG_RPF_CHECK;
        CTC_CLI_GET_UINT32_RANGE("src port", fcoe_info.src_gport, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("macsa");
    if (index != 0xFF)
    {
        fcoe_info.flag |= CTC_FCOE_FLAG_MACSA_CHECK;
        CTC_CLI_GET_MAC_ADDRESS("macsa", fcoe_info.mac_sa, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("src-discard");
    if (index != 0xFF)
    {
        fcoe_info.flag |= CTC_FCOE_FLAG_SRC_DISCARD;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_fcoe_add_route(g_api_lchip, &fcoe_info);
    }
    else
    {
        ret = ctc_fcoe_add_route(&fcoe_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_fcoe_remove_route,
        ctc_cli_fcoe_remove_route_cmd,
        "fcoe remove fcid FCID fid FID",
        CTC_CLI_FCOE_M_STR,
        "Remove route",
        CTC_CLI_DEST_FCID_DESC,
        CTC_CLI_FCID_RANGE_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_fcoe_route_t fcoe_info;

    sal_memset(&fcoe_info, 0, sizeof(ctc_fcoe_route_t));


    CTC_CLI_GET_UINT32_RANGE("fcid", fcoe_info.fcid, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("fid", fcoe_info.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_fcoe_remove_route(g_api_lchip, &fcoe_info);
    }
    else
    {
        ret = ctc_fcoe_remove_route(&fcoe_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_fcoe_debug_on,
        ctc_cli_fcoe_debug_on_cmd,
        "debug fcoe (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_FCOE_M_STR,
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

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = FCOE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = FCOE_SYS;
    }

    ctc_debug_set_flag("fcoe", "fcoe", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_fcoe_debug_off,
        ctc_cli_fcoe_debug_off_cmd,
        "no debug fcoe (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_FCOE_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = FCOE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = FCOE_SYS;
    }

    ctc_debug_set_flag("fcoe", "fcoe", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_fcoe_debug_show,
        ctc_cli_fcoe_debug_show_cmd,
        "show debug fcoe (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_FCOE_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = FCOE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = FCOE_SYS;
    }
    en = ctc_debug_get_flag("fcoe", "fcoe", typeenum, &level);
    ctc_cli_out("fcoe:%s debug %s level:%s\n\r", argv[0],
                 en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

int32
ctc_fcoe_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_fcoe_add_route_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_fcoe_remove_route_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_fcoe_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_fcoe_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_fcoe_debug_show_cmd);

    return CLI_SUCCESS;
}

