/**
 @file ctc_usw_stats_cli.c

 @date 2010-06-09

 @version v2.0

 This file defines functions for stats internal cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_cli.h"
#include "ctc_const.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"

extern int32
sys_usw_flow_stats_show_status(uint8 lchip);
extern int32
sys_usw_mac_stats_show_status(uint8 lchip);
extern int32
sys_usw_flow_stats_enable_igs_port_discard_stats(uint8 lchip, uint8 enable);
extern int32
sys_usw_flow_stats_clear_igs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_show_igs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_enable_egs_port_discard_stats(uint8 lchip, uint8 enable);
extern int32
sys_usw_flow_stats_clear_egs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_flow_stats_show_egs_port_discard_stats(uint8 lchip, uint32 gport);
extern int32
sys_usw_mac_stats_set_global_property_clear_after_read(uint8 lchip, bool enable);
extern int32
sys_usw_mac_stats_get_global_property_clear_after_read(uint8 lchip, bool* p_enable);
extern int32
sys_usw_flow_stats_set_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable);
extern int32
sys_usw_flow_stats_get_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable);
int32
sys_usw_mac_stats_set_property_pfc(uint8 lchip, uint32 gport, uint8 enable);
int32
sys_usw_mac_stats_get_property_pfc(uint8 lchip, uint32 gport, uint8* p_enable);

CTC_CLI(ctc_cli_usw_stats_show_status,
        ctc_cli_usw_stats_show_status_cmd,
        "show stats status (lchip LCHIP|)",
        "Show",
        "Stats Module",
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret= CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
    	CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_stats_show_status(lchip);
    ret = sys_usw_flow_stats_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_stats_create_statsid_sdc,
        ctc_cli_usw_stats_create_statsid__sdc_cmd,
        "stats create statsid STATS_ID sdc ((ingress|egress)|)",
        CTC_CLI_STATS_STR,
        "Create",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "Sdc",
        "Ingress",
        "Egress")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_stats_statsid_t stats_statsid;

    sal_memset(&stats_statsid, 0, sizeof(ctc_stats_statsid_t));
    CTC_CLI_GET_UINT32_RANGE("stats id", stats_statsid.stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_EGRESS;
    }
    else
    {
        stats_statsid.dir = CTC_INGRESS;
    }

    stats_statsid.type = CTC_STATS_STATSID_TYPE_SDC;

    if(g_ctcs_api_en)
    {
        ret = ctcs_stats_create_statsid(g_api_lchip, &stats_statsid);
    }
    else
    {
        ret = ctc_stats_create_statsid(&stats_statsid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;


}

CTC_CLI(ctc_cli_usw_stats_discard_enable,
        ctc_cli_usw_stats_discard_enable_cmd,
        "stats discard ( ingress | egress ) (enable | disable) (lchip LCHIP| )",
        CTC_CLI_STATS_STR,
        "discard paket",
        "ingress direction",
        "egress direction",
        "enable discard stats",
        "disable discard stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;
    bool enable = FALSE;

    if (CLI_CLI_STR_EQUAL("ingress", 0))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 0))
    {
        dir = CTC_EGRESS;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = TRUE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        enable = FALSE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if(dir == CTC_INGRESS)
    {
        ret = sys_usw_flow_stats_enable_igs_port_discard_stats(lchip, enable);
    }
    else
    {
        ret = sys_usw_flow_stats_enable_egs_port_discard_stats(lchip, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_stats_show_discard,
        ctc_cli_usw_stats_show_discard_cmd,
        "show stats port GPHYPORT_ID ( ingress | egress ) discard",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ingress direction",
        "egress direction",
        "discard statis")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint16 gport_id = 0;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;

    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if(dir == CTC_INGRESS)
    {
        ret = sys_usw_flow_stats_show_igs_port_discard_stats(lchip, gport_id);
    }
    else
    {
        ret = sys_usw_flow_stats_show_egs_port_discard_stats(lchip, gport_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_stats_clear_discard,
        ctc_cli_usw_stats_clear_discard_cmd,
        "clear stats port GPHYPORT_ID ( ingress | egress ) discard",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ingress direction",
        "egress direction",
        "discard statis")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint16 gport_id = 0;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;

    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    if(dir == CTC_INGRESS)
    {
        ret = sys_usw_flow_stats_clear_igs_port_discard_stats(lchip, gport_id);
    }
    else
    {
        ret = sys_usw_flow_stats_clear_egs_port_discard_stats(lchip, gport_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_stats_clear_after_read_enable,
        ctc_cli_usw_stats_clear_after_read_enable_cmd,
        "stats clear after read (mac | flow) (enable | disable) (lchip LCHIP| )",
        CTC_CLI_STATS_STR,
        "clear stats",
        "clear after read",
        "clear after read",
        "clear mac stats",
        "clear flow stats",
        "enable clear stats",
        "disable clear stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    bool enable = FALSE;
    bool flowStats = FALSE;

    if (CLI_CLI_STR_EQUAL("mac", 0))
    {
        flowStats = FALSE;
    }
    else if (CLI_CLI_STR_EQUAL("flow", 0))
    {
        flowStats = TRUE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = TRUE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        enable = FALSE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    
    if(flowStats)
    {
        ret = sys_usw_flow_stats_set_clear_after_read_en(lchip, CTC_STATS_TYPE_FWD, enable);
    }
    else
    {
        ret = sys_usw_mac_stats_set_global_property_clear_after_read(lchip, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_show_clear_after_read_status,
        ctc_cli_usw_show_clear_after_read_status_cmd,
        "show stats clear after read (mac | flow) (lchip LCHIP| )",
        "show",
        CTC_CLI_STATS_STR,
        "clear stats",
        "clear after read",
        "clear after read",
        "clear mac stats",
        "clear flow stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    bool enable = FALSE;
    bool flowStats = FALSE;

    if (CLI_CLI_STR_EQUAL("mac", 0))
    {
        flowStats = FALSE;
    }
    else if (CLI_CLI_STR_EQUAL("flow", 0))
    {
        flowStats = TRUE;
    }

    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    
    if(flowStats)
    {
        ret = sys_usw_flow_stats_get_clear_after_read_en(lchip, CTC_STATS_TYPE_FWD, &enable);
    }
    else
    {
        ret = sys_usw_mac_stats_get_global_property_clear_after_read(lchip, &enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s stats clear on read status: %s\n", flowStats? "flow":"mac", enable? "enable":"disable");
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_stats_set_pfc_property,
        ctc_cli_usw_stats_set_pfc_property_cmd,
        "stats property pfc gport GPHYPORT_ID value VALUE (lchip LCHIP| )",
        CTC_CLI_STATS_STR,
        "stats property",
        "pfc property",
        "gport",
        "gport value",
        "pfc property",
        "pfc property value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 enable = 0;
    uint32 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("enable", enable, argv[1], 0, CTC_MAX_UINT8_VALUE);
    
    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    
    ret = sys_usw_mac_stats_set_property_pfc(lchip, gport, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_show_pfc_property_status,
        ctc_cli_usw_show_pfc_property_status_cmd,
        "show stats property pfc gport GPHYPORT_ID (lchip LCHIP| )",
        "show",
        CTC_CLI_STATS_STR,
        "stats property",
        "pfc property",
        "gport",
        "gport value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 enable = 0;
    uint32 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);
    
    index = 0xFF;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip_id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    

    ret = sys_usw_mac_stats_get_property_pfc(lchip, gport, &enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("gport: 0x%x stats pfc property value: %d\n", gport, enable);
    return CLI_SUCCESS;
}

/**
 @brief  Initialize sdk stats internal module CLIs
*/
int32
ctc_usw_stats_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_create_statsid__sdc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_discard_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_show_discard_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_clear_discard_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_clear_after_read_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_clear_after_read_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_stats_set_pfc_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_show_pfc_property_status_cmd);
    
    return CLI_SUCCESS;
}

int32
ctc_usw_stats_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_create_statsid__sdc_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_discard_enable_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_show_discard_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_clear_discard_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_clear_after_read_enable_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_clear_after_read_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_stats_set_pfc_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_show_pfc_property_status_cmd);

    return CLI_SUCCESS;
}

