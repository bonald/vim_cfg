/**
 @file ctc_greatbel_pdu_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2012-8-18

 @version v2.0

This file contains pdu module cli command
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_pdu.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_usw_pdu_cli.h"

extern int32
sys_usw_l2pdu_show_port_action(uint8 lchip, uint16 gport);

extern int32
sys_usw_pdu_show_l2pdu_cfg(uint8 lchip);

extern int32
sys_usw_pdu_show_l3pdu_cfg(uint8 lchip);

CTC_CLI(ctc_cli_usw_pdu_show_l2pdu_port_action,
        ctc_cli_usw_pdu_show_l2pdu_port_action_cmd,
        "show pdu l2pdu port-action port GPORT_ID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "L2pdu Port-ction type",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint16 gport = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l2pdu_show_port_action(lchip, gport);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_usw_pdu_show_global_config,
        ctc_cli_usw_pdu_show_global_config_cmd,
        "show pdu (l2pdu|l3pdu) global-config (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Global configuration info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if (0 == sal_memcmp("l2", argv[0], 2))
    {
        ret = sys_usw_pdu_show_l2pdu_cfg(lchip);
    }

    if (0 == sal_memcmp("l3", argv[0], 2))
    {
        ret = sys_usw_pdu_show_l3pdu_cfg(lchip);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

int32
ctc_usw_pdu_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_pdu_show_l2pdu_port_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_pdu_show_global_config_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_pdu_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_pdu_show_l2pdu_port_action_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_pdu_show_global_config_cmd);

    return CLI_SUCCESS;
}

