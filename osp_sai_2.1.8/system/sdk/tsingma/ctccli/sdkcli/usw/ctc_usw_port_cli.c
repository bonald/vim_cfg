/**
 @file ctc_greatbel_port_cli.c

 @date 2012-04-06

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_port_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

extern int32 sys_usw_mac_get_monitor_rlt(uint8 lchip);
extern int32 sys_usw_port_set_internal_property(uint8 lchip, uint16 gport, uint32 port_prop, uint32 value);
extern int32 sys_usw_mac_show_port_maclink(uint8 lchip);
extern int32 sys_usw_mac_set_3ap_training_en(uint8 lchip, uint16 gport, uint32 enable);
extern int32 sys_usw_mac_get_3ap_training_en(uint8 lchip, uint16 gport, uint32* p_status);
extern int32 sys_usw_port_set_isolation_mode(uint8 lchip, uint8 choice_mode);
extern int32 sys_usw_mac_set_monitor_link_enable(uint8 lchip, uint32 enable);
extern int32 sys_usw_mac_get_monitor_link_enable(uint8 lchip, uint32* p_value);
extern int32 sys_usw_internal_port_show(uint8 lchip);
extern int32 sys_usw_mac_self_checking(uint8 lchip, uint32 gport);

CTC_CLI(ctc_cli_usw_port_set_vlan_range,
        ctc_cli_usw_port_set_vlan_range_cmd,
        "port GPHYPORT_ID vlan-range direction (ingress|egress) (vrange-grp VRANGE_GROUP_ID|) (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Vlan range on port",
        "Direction",
        "Ingress",
        "Egress",
        "Vlan range group id",
        "<0-63>",
        "Enable vlan range",
        "Disable vlan range")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    bool enable = FALSE;
    uint8  index = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_direction_t dir = CTC_INGRESS;

    sal_memset(&vrange_info, 0, sizeof(ctc_vlan_range_info_t));
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("vlan range group id", vrange_info.vrange_grpid, argv[index + 1]);
    }

    if (0xFF != CTC_CLI_GET_ARGC_INDEX("enable"))
    {
        enable = TRUE;
    }
    else if (0xFF != CTC_CLI_GET_ARGC_INDEX("disable"))
    {
        enable = FALSE;
    }

    vrange_info.direction = dir;

    if(g_ctcs_api_en)
    {
       ret = ctcs_port_set_vlan_range(g_api_lchip, gport, &vrange_info, enable);
    }
    else
    {
        ret = ctc_port_set_vlan_range(gport, &vrange_info, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_set_system_min_frame_size,
        ctc_cli_usw_port_set_system_min_frame_size_cmd,
        "system min-frame-size (size0|size1|size2|size3|size4|size5|size6|size7) BYTES",
        "System attribution",
        "Min frame size to apply port select",
        "Min frame size0",
        "Min Frame size1",
        "Min Frame size2",
        "Min Frame size3",
        "Min frame size4",
        "Min Frame size5",
        "Min Frame size6",
        "Min Frame size7",
        CTC_CLI_PORT_MIN_FRAME_SIZE)
{
    int32 ret = CLI_SUCCESS;
    uint16 min_size;
    ctc_frame_size_t index = CTC_FRAME_SIZE_0;

    CTC_CLI_GET_UINT16_RANGE("min frame size", min_size, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("size0", 0))
    {
        index = CTC_FRAME_SIZE_0;
    }
    else if (CLI_CLI_STR_EQUAL("size1", 0))
    {
        index = CTC_FRAME_SIZE_1;
    }
    else if (CLI_CLI_STR_EQUAL("size2", 0))
    {
        index = CTC_FRAME_SIZE_2;
    }
    else if (CLI_CLI_STR_EQUAL("size3", 0))
    {
        index = CTC_FRAME_SIZE_3;
    }
    else if (CLI_CLI_STR_EQUAL("size4", 0))
    {
        index = CTC_FRAME_SIZE_4;
    }
    else if (CLI_CLI_STR_EQUAL("size5", 0))
    {
        index = CTC_FRAME_SIZE_5;
    }
    else if (CLI_CLI_STR_EQUAL("size6", 0))
    {
        index = CTC_FRAME_SIZE_6;
    }
    else if (CLI_CLI_STR_EQUAL("size7", 0))
    {
        index = CTC_FRAME_SIZE_7;
    }

    if(g_ctcs_api_en)
    {
       ret = ctcs_set_min_frame_size(g_api_lchip, index, min_size);
    }
    else
    {
        ret = ctc_set_min_frame_size(index, min_size);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_system_min_frame_size,
        ctc_cli_usw_port_show_system_min_frame_size_cmd,
        "show system min-frame-size",
        CTC_CLI_SHOW_STR,
        "System attribution",
        "Min frame size to apply port select")
{
    int32 ret = CLI_SUCCESS;
    uint8 size_idx = 0;
    uint16 val_16 = 0;

    ctc_cli_out("Show system min frame size.\n");
    ctc_cli_out("--------------\n");
    ctc_cli_out("%-8s%s\n", "Index", "Value");
    ctc_cli_out("--------------\n");

    for (size_idx = 0; size_idx < CTC_FRAME_SIZE_MAX; size_idx++)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_get_min_frame_size(g_api_lchip, size_idx, &val_16);
        }
        else
        {
            ret = ctc_get_min_frame_size(size_idx, &val_16);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-8u%u\n", size_idx, val_16);
    }
    ctc_cli_out("--------------\n");

    return CTC_E_NONE;
}

CTC_CLI(ctc_cli_usw_port_set_port_sec,
        ctc_cli_usw_port_set_port_sec_cmd,
        "port GPHYPORT_ID port-security (enable|disable) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port security",
        "enable",
        "disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint16 gport = 0;
    uint32 value = 0;
    uint32 property = 0;
    uint8 index = 0;


    /* SYS_PORT_PROP_SECURITY_EN */
    property = 11;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(0 == sal_memcmp(argv[1], "enable", sal_strlen("enable")))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_port_set_internal_property(lchip, gport, property, value);

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_port_set_property_wlan,
        ctc_cli_usw_port_set_property_wlan_cmd,
        "port GPHYPORT_ID property (wlan-port-type (none|decap-with-decrypt|decap-without-decrypt) | (wlan-route-en | dscp-mode | default-dscp|replace-pcp-dei) value VALUE)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property",
        "Wlan port type",
        "disable wlan port",
        "not support decrypt",
        "support decrypt",
        "Wlan route ",
        "Dscp select mode, refer to ctc_dscp_select_mode_t",
        "Default dscp",
        "Replace pcp dei",
        "Property value",
        "Value")
{
    int32 ret = 0;
    uint16 gport;
    uint32 value = 0;
    uint8 index = 0xFF;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("Property_value", value, argv[index + 1]);
    }

    if (CLI_CLI_STR_EQUAL("wlan-port-type", 1))
    {
        if (CLI_CLI_STR_EQUAL("none", 2))
        {
            value = CTC_PORT_WLAN_PORT_TYPE_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("decap-with-decrypt", 2))
        {
            value = CTC_PORT_WLAN_PORT_TYPE_DECAP_WITH_DECRYPT;
        }
        else if (CLI_CLI_STR_EQUAL("decap-without-decrypt", 2))
        {
            value = CTC_PORT_WLAN_PORT_TYPE_DECAP_WITHOUT_DECRYPT;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_WLAN_PORT_TYPE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_WLAN_PORT_TYPE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("wlan-route-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_WLAN_PORT_ROUTE_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_WLAN_PORT_ROUTE_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("dscp-mode", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DSCP_SELECT_MODE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DSCP_SELECT_MODE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("default-dscp", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_DSCP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DEFAULT_DSCP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("replace-pcp-dei", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_PCP_DEI, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_PCP_DEI, value);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_port_get_property_wlan,
        ctc_cli_usw_port_get_property_wlan_cmd,
        "show port GPHYPORT_ID property (wlan-port-type | wlan-route-en | dscp-mode | default-dscp|replace-pcp-dei) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property",
        "wlan port type",
        "wlan route enable",
        "Dscp select mode, refer to ctc_dscp_select_mode_t",
        "Default dscp",
        "Replace pcp dei",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;
    uint16 gport = 0;
    uint32 value = 0;
    uint32 property = 0;
    char* p_desc[] = {"none", "decap-with-decrypt", "decap-without-decrypt", NULL};

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("wlan-port-type");
    if (0xFF != index)
    {
        property = CTC_PORT_PROP_WLAN_PORT_TYPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-route-en");
    if (0xFF != index)
    {
        property = CTC_PORT_PROP_WLAN_PORT_ROUTE_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dscp-mode");
    if (0xFF != index)
    {
        property = CTC_PORT_PROP_DSCP_SELECT_MODE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("default-dscp");
    if (0xFF != index)
    {
        property = CTC_PORT_PROP_DEFAULT_DSCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-pcp-dei");
    if (0xFF != index)
    {
        property = CTC_PORT_PROP_REPLACE_PCP_DEI;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_port_get_property(g_api_lchip, gport, property, &value);
    }
    else
    {
        ret = ctc_port_get_property(gport, property, &value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        if (CTC_PORT_PROP_WLAN_PORT_TYPE == property)
        {
            ctc_cli_out("GPort:0x%04X, wlan port type:%s\n", gport, p_desc[value]);
        }
        else if (CTC_PORT_PROP_WLAN_PORT_ROUTE_EN == property)
        {
            if (!value)
            {
                ctc_cli_out("GPort:0x%04X, wlan route: disable\n", gport);
            }
            else
            {
                ctc_cli_out("GPort:0x%04X, wlan route: enable\n", gport);
            }
        }
        else if (CTC_PORT_PROP_DSCP_SELECT_MODE == property)
        {
            ctc_cli_out("GPort:0x%04X, Dscp select mode: %u\n", gport, value);
        }
        else if (CTC_PORT_PROP_DEFAULT_DSCP == property)
        {
            ctc_cli_out("GPort:0x%04X, Default dscp: %u\n", gport, value);
        }
        else if (CTC_PORT_PROP_REPLACE_PCP_DEI == property)
        {
            ctc_cli_out("GPort:0x%04X, Replace pcp dei: %u\n", gport, value);
        }
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_port_maclink,
        ctc_cli_usw_port_show_port_maclink_cmd,
        "show port mac-link (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        "Port mac link status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_show_port_maclink(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_port_set_monitor_link,
        ctc_cli_usw_port_set_monitor_link_cmd,
        "port link monitor ( enable | disable ) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        "Link status",
        "Monitor port link",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 value = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        value = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_set_monitor_link_enable(lchip, value);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_port_get_monitor_link,
        ctc_cli_usw_port_get_monitor_link_cmd,
        "show port link monitor status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        "Link status",
        "Monitor port link",
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 value = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_get_monitor_link_enable(lchip, &value);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-45s:  %s\n","Link Monitor", value?"enable":"disable");

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_port_show_monitor_rlt,
        ctc_cli_usw_port_show_monitor_rlt_cmd,
        "show port link monitor (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Device",
        "Information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_get_monitor_rlt(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_port_get_3ap_status,
        ctc_cli_usw_port_get_3ap_status_cmd,
        "show port GPHYPORT_ID 3ap-status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "802.3ap training status"
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;
    uint32 gport = 0;
    uint32 value = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ctc_cli_out("%-45s:  0x%04x\n","Show GPort", gport);
    ctc_cli_out("--------------------------------------------------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_mac_get_3ap_training_en(lchip, gport, &value);
    if (ret >= 0)
    {
        /*sys_port_cl72_status_t*/
        if(value == 0)
            ctc_cli_out("%-45s:  %s\n", "3ap Status", "Disable");
        else if(value == 1)
            ctc_cli_out("%-45s:  %s\n", "3ap Status", "In progress");
        else if(value == 2)
            ctc_cli_out("%-45s:  %s\n", "3ap Status", "Fail");
        else if(value == 3)
            ctc_cli_out("%-45s:  %s\n", "3ap Status", "Ok");
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_usw_port_set_3ap_training_en,
        ctc_cli_usw_port_set_3ap_training_en_cmd,
        "port GPHYPORT_ID 3ap-train (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "802.3ap training cfg flag",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint16 gport;
    uint32 enable;
    uint8 lchip = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    ret = sys_usw_mac_set_3ap_training_en(lchip, gport, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_port_set_acl_hash_property,
        ctc_cli_usw_port_set_acl_hash_property_cmd,
        "port GPHYPORT_ID acl-hash-property acl-en (disable |(enable "\
        "hash-field-sel-id FIELD-SEL-ID hash-lkup-type TYPE (use-mapped-vlan |)))",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Acl hash property on port",
        "Port hash acl en",
        "Disable function",
        "Enable function",
        "Hash field select id",
        "Id value",
        "Hash lookup type",
        "Type value",
        "use mapped vlan")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 index = 0;
    ctc_acl_property_t prop;

    /*default is use port_bitmap*/
    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    prop.flag |= CTC_ACL_PROP_FLAG_USE_HASH_LKUP;

    CTC_CLI_GET_UINT16("port", gport, argv[0] );

    if(CLI_CLI_STR_EQUAL("disable",1))
    {
        prop.acl_en = 0;
    }
    else
    {
        prop.acl_en = 1;
    }

    if(prop.acl_en)
    {
        index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel-id");
        if(0xFF != index)
        {
            CTC_CLI_GET_UINT8("hash field select id",prop.hash_field_sel_id,argv[index+1] );
            CTC_CLI_GET_UINT8("hash field select id",prop.hash_lkup_type,argv[index+3] );
        }

        index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");
        if(0xFF != index)
        {
            prop.flag |= CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_acl_property(g_api_lchip, gport, &prop);
    }
    else
    {
        ret = ctc_port_set_acl_property(gport, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_acl_hash_property,
        ctc_cli_usw_port_show_acl_hash_property_cmd,
        "show port GPHYPORT_ID acl-hash-property",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Acl hash property")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_acl_property_t prop;

    /*default is use port_bitmap*/
    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    prop.flag |= CTC_ACL_PROP_FLAG_USE_HASH_LKUP;

    CTC_CLI_GET_UINT16("port", gport, argv[0] );

    ctc_cli_out("Port:0x%04x ACL Hash Property\n", gport);
    ctc_cli_out("================================\n");
    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_acl_property(g_api_lchip, gport, &prop);
    }
    else
    {
        ret = ctc_port_get_acl_property(gport, &prop);
    }

    if(prop.acl_en)
    {
        ctc_cli_out("Hash field select id:   %d\n",prop.hash_field_sel_id);
        ctc_cli_out("Hash lookup type:   %d\n",prop.hash_lkup_type);
        ctc_cli_out("Use mapped vlan :   %d\n",(prop.flag & CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
    }
    else
    {
        ctc_cli_out("Hash acl disable\n");
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("================================\n");

    return ret;
}

CTC_CLI(ctc_cli_usw_port_set_acl_property,
        ctc_cli_usw_port_set_acl_property_cmd,
        "port GPHYPORT_ID acl-property priority PRIORITY_ID direction (ingress | egress ) acl-en (disable |(enable "\
        "tcam-lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext |"\
        "cid | interface | forward | forward-ext | udf) {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan |use-wlan|}))",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Acl property on port",
        "Acl priority",
        "PRIORITY_ID",
        "direction",
        "ingress",
        "egress",
        "Port acl en",
        "Disable function",
        "Enable function",
        "Tcam lookup type",
        "L2 key",
        "L3 key",
        "L2 L3 key",
        "Vlan key",
        "L3 extend key",
        "L2 L3 extend key",
        "CID  key",
        "Interface key",
        "Forward key",
        "Forward extend key",
        "Udf key",
        "Port acl use class id",
        "CLASS_ID",
        "Port acl use port-bitmap",
        "Port acl use metadata",
        "Port acl use logic port",
        "use mapped vlan",
        "Port acl use wlan info")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 index = 0;
    ctc_acl_property_t prop;

    /*default is use port_bitmap*/
    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT16("port", gport, argv[0] );
    CTC_CLI_GET_UINT8("priority",prop.acl_priority,argv[1] );

    if(CLI_CLI_STR_EQUAL("disable",3))
    {
        prop.acl_en = 0;
    }
    else
    {
        prop.acl_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if(0xFF != index)
    {
        prop.direction= CTC_INGRESS;

    }
    else
    {
        prop.direction= CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcam-lkup-type");
    if(0xFF != index)
    {
        if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3;
        }
        else if(CLI_CLI_STR_EQUAL("vlan",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_VLAN;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l3-ext",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3_EXT;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3-ext",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;
        }
        else if(CLI_CLI_STR_EQUAL("cid",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;
        }
        else if(CLI_CLI_STR_EQUAL("interface",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_INTERFACE;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("forward",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("forward-ext",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("udf",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_UDF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("class-id");
    if(0xFF != index)
    {
        CTC_CLI_GET_UINT8("class-id",prop.class_id, argv[index + 1] );
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-port-bitmap");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-metadata");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_METADATA);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-wlan");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_WLAN);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_acl_property(g_api_lchip, gport, &prop);
    }
    else
    {
        ret = ctc_port_set_acl_property(gport, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_acl_property,
        ctc_cli_usw_port_show_acl_property_cmd,
        "show port GPHYPORT_ID acl-property direction (ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port acl property",
        "Direction",
        "Ingress",
        "Egress")
{
    int32  ret = CLI_SUCCESS;
    uint8  idx = 0;
    uint16 gport = 0;
    uint8  lkup_num = 0;
    ctc_acl_property_t prop;
    char* tcam_key_str[CTC_ACL_TCAM_LKUP_TYPE_MAX] = {
        "l2","l2-l3","l3","vlan","l3-ext","l2-l3-ext",
        "sgt","interface","forward", "forward-ext","copp-key", "udf-key"
    };

    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT16("port", gport, argv[0]);

    if(CLI_CLI_STR_EQUAL("ingress",1))
    {
        prop.direction = CTC_INGRESS;
        lkup_num = 8;
    }
    else if(CLI_CLI_STR_EQUAL("egress",1 ))
    {
        prop.direction = CTC_EGRESS;
        lkup_num = 3;
    }
    ctc_cli_out("==========================================================================================\n");
    ctc_cli_out("Prio dir acl-en tcam-lkup-type class-id port-bitmap logic-port mapped-vlan metadata wlan\n");

    for(idx =0; idx < lkup_num; idx++)
    {
        prop.acl_priority = idx;
        if(g_ctcs_api_en)
        {
            ret = ctcs_port_get_acl_property(g_api_lchip, gport, &prop);
        }
        else
        {
            ret = ctc_port_get_acl_property(gport, &prop);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-5d%-4s%-7d",prop.acl_priority,prop.direction ? "egs":"igs",prop.acl_en);
        if(prop.acl_en)
        {
            ctc_cli_out("%-15s%-9d%-12d%-11d%-12d%-9d%-10d\n", tcam_key_str[prop.tcam_lkup_type], prop.class_id,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP) ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT)  ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN) ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_METADATA)    ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_WLAN)      ? 1 : 0);
        }
        else
        {
            ctc_cli_out("%-15s%-9s%-12s%-11s%-12s%-9s%-10s\n","-","-","-","-","-","-","-");
        }
    }

    ctc_cli_out("==========================================================================================\n");
    return ret;

}


CTC_CLI(ctc_cli_usw_port_set_bpe_property,
        ctc_cli_usw_port_set_bpe_property_cmd,
        "port GPHYPORT_ID bpe-property  (extend-type (none| extend-port ecid ECID | cascade-port name-space NAMESPACE|upstream-port name-space NAMESPACE)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Bpe property on port",
        "Extend type",
        "None",
        "Extend port",
        "Ecid",
        "Ecid value",
        "Cascade port",
        "Name space",
        "Name space value",
        "Upstream port",
        "Name space",
        "Name space value")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 index = 0;
    ctc_port_bpe_property_t prop;

    /*default is use port_bitmap*/
    sal_memset(&prop,0,sizeof(prop));

    CTC_CLI_GET_UINT16("port", gport, argv[0] );


    index = CTC_CLI_GET_ARGC_INDEX("extend-port");
    if(0xFF != index)
    {
        prop.extend_type = CTC_PORT_BPE_8021BR_PE_EXTEND;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cascade-port");
    if(0xFF != index)
    {
        prop.extend_type = CTC_PORT_BPE_8021BR_PE_CASCADE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("upstream-port");
    if (0xFF != index)
    {
        prop.extend_type = CTC_PORT_BPE_8021BR_PE_UPSTREAM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("Ecid", prop.ecid, argv[index + 1] );
    }

    index = CTC_CLI_GET_ARGC_INDEX("name-space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("name space", prop.name_space, argv[index + 1] );
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_bpe_property(g_api_lchip, gport, &prop);
    }
    else
    {
        ret = ctc_port_set_bpe_property(gport, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_set_dir_property,
        ctc_cli_usw_port_set_dir_property_cmd,
        "port GPHYPORT_ID dir-property  \
        ( qos-cos-domain | qos-dscp-domain | dot1ae-en|dot1ae-chan)  \
        direction (ingress|egress|both) value VALUE ",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port property with direction",
        "Port qos cos domain",
        "Port qos dscp domain",
        "Port dot1ae enable",
        "Dot1ae channel ID",
        "SD action",
        "Flow direction",
        "Ingress",
        "Egress",
        "Both direction",
        "Property value",
        "Value")
{
    uint8 index = 0;
    int32 ret = 0;
    uint16 gport = 0;
    uint32 value = 0;
    ctc_direction_t dir = CTC_INGRESS;
    uint32  prop = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    CTC_CLI_GET_UINT32("value", value, argv[argc - 1]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (INDEX_VALID(index))
    {
        dir = CTC_BOTH_DIRECTION;
    }


    if (CLI_CLI_STR_EQUAL("qos-cos-domain", 1))
    {
        prop = CTC_PORT_DIR_PROP_QOS_COS_DOMAIN;
    }
    else if (CLI_CLI_STR_EQUAL("qos-dscp-domain", 1))
    {
        prop = CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN;
    }
    else if (CLI_CLI_STR_EQUAL("dot1ae-en", 1))
    {
        prop = CTC_PORT_DIR_PROP_DOT1AE_EN;
    }
    else if (CLI_CLI_STR_EQUAL("dot1ae-chan", 1))
    {
        prop = CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_direction_property(g_api_lchip, gport, prop, dir, value);
    }
    else
    {
        ret = ctc_port_set_direction_property(gport, prop, dir, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_port_dir_property,
        ctc_cli_usw_port_show_port_dir_property_cmd,
        "show port GPHYPORT_ID dir-property ( \
        | qos-cos-domain | qos-dscp-domain | dot1ae-en |dot1ae-chan ) \
        direction ( ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property with direction",
        "Port qos cos domain",
        "Port qos dscp domain",
        "Dot1ae enable",
        "Dot1ae channel ID",
        "SD action",
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret = 0;
    uint16 gport;
    uint32 value;
    ctc_direction_t dir = 0;
    uint8 index = 0xFF;


    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    ctc_cli_out("==============================\n");

    if CLI_CLI_STR_EQUAL("in", 2)
    {
        dir = CTC_INGRESS;
        ctc_cli_out("Ingress:\n");
    }
    else if CLI_CLI_STR_EQUAL("eg", 2)
    {
        dir = CTC_EGRESS;
        ctc_cli_out("Egress:\n");
    }


    index = CTC_CLI_GET_ARGC_INDEX("qos-cos-domain");
    if (INDEX_VALID(index))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_QOS_COS_DOMAIN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_QOS_COS_DOMAIN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Qos-cos-domain         :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-dscp-domain");
    if (INDEX_VALID(index))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Qos-dscp-domain         :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dot1ae-en");
    if (INDEX_VALID(index))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_DOT1AE_EN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_DOT1AE_EN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Dot1AE en      :  %s\n", value?"enable":"disable");
        }
    }

	index = CTC_CLI_GET_ARGC_INDEX("dot1ae-chan");
    if (INDEX_VALID(index))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Dot1AE Chan ID      :  %u\n", value);
        }
    }

	 ctc_cli_out("==============================\n");
    if ((ret < 0))
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_set_property,
        ctc_cli_usw_port_set_property_cmd,
        "port GPHYPORT_ID property ((cid|tx-cid-hdr-en|acl-aware-tunnel-info-en | ipfix-aware-tunnel-info-en | \
                                  acl-flow-lkup-use-outer | ipfix-flow-lkup-use-outer|logic-port-check|sd-action|learn-dis-mode|force-mpls-en|force-tunnel-en| \
                                  lb-hash-ecmp-profile| lb-hash-lag-profile| discard-unencrypt-pkt| wlan-decap-with-rid |inband-cpu-traffic-en) value VALUE)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port property",
        "Port cid",
        "Tx packet with cid header",
        "Acl aware tunnel info",
        "Ipfix aware tunnel info",
        "Acl flow lookup use outer info",
        "Ipfix flow lookup use outer info",
        "Logic port check",
        "Signal degrade action: 0-none; 1-discard oam pkt; 2-discard data pkt; 3-discard all",
        "Learning disable mode",
        "Force MPLS packet decap",
        "Force Tunnel packet decap",
        "Load balance hash ecmp profile id",
        "Load balance hash lag profile id",
        "Discard unencrypt packet",
        "WLAN decap with radio ID",
        "Inband CPU trafffic enable",
        "Property value",
        "Value")
{
    int32 ret = 0;
    uint16 gport = 0;
    uint32 value = 0;
    uint16 prop  = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    CTC_CLI_GET_UINT32("value", value, argv[argc - 1]);

    if (CLI_CLI_STR_EQUAL("cid", 1))
    {
        prop = CTC_PORT_PROP_CID;
    }
    else if (CLI_CLI_STR_EQUAL("tx-cid-hdr-en", 1))
    {
        prop = CTC_PORT_PROP_TX_CID_HDR_EN;
    }
    else if (CLI_CLI_STR_EQUAL("acl-aware-tunnel-info", 1))
    {
        prop = CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN;
    }
    else if (CLI_CLI_STR_EQUAL("ipfix-aware-tunnel-info", 1))
    {
        prop = CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN;
    }
    else if (CLI_CLI_STR_EQUAL("acl-flow-lkup-use-outer", 1))
    {
        prop = CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD;
    }
    else if (CLI_CLI_STR_EQUAL("ipfix-flow-lkup-use-outer", 1))
    {
        prop = CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD;
    }
    else if (CLI_CLI_STR_EQUAL("logic-port-check", 1))
    {
        prop = CTC_PORT_PROP_LOGIC_PORT_CHECK_EN;
    }
    else if (CLI_CLI_STR_EQUAL("sd-action", 1))
    {
        prop = CTC_PORT_PROP_SD_ACTION;
    }
    else if (CLI_CLI_STR_EQUAL("learn-dis-mode", 1))
    {
        prop = CTC_PORT_PROP_LEARN_DIS_MODE;
    }
    else if (CLI_CLI_STR_EQUAL("force-mpls-en", 1))
    {
        prop = CTC_PORT_PROP_FORCE_MPLS_EN;
    }
    else if (CLI_CLI_STR_EQUAL("force-tunnel-en", 1))
    {
        prop = CTC_PORT_PROP_FORCE_TUNNEL_EN;
    }
    else if (CLI_CLI_STR_EQUAL("lb-hash-ecmp-profile", 1))
    {
        prop = CTC_PORT_PROP_LB_HASH_ECMP_PROFILE;
    }
    else if (CLI_CLI_STR_EQUAL("lb-hash-lag-profile", 1))
    {
        prop = CTC_PORT_PROP_LB_HASH_LAG_PROFILE;
    }
    else if (CLI_CLI_STR_EQUAL("discard-unencrypt-pkt", 1))
    {
        prop = CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT;
    }
    else if (CLI_CLI_STR_EQUAL("wlan-decap-with-rid", 1))
    {
        prop = CTC_PORT_PROP_WLAN_DECAP_WITH_RID;
    }
    else if (CLI_CLI_STR_EQUAL("inband-cpu-traffic-en", 1))
    {
        prop = CTC_PORT_PROP_INBAND_CPU_TRAFFIC_EN;
    }
    
    if(g_ctcs_api_en)
    {
         ret = ctcs_port_set_property(g_api_lchip, gport, prop, value);
    }
    else
    {
        ret = ctc_port_set_property(gport, prop, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_port_show_property,
        ctc_cli_usw_port_show_property_cmd,
        "show port GPHYPORT_ID property (cid|tx-cid-hdr-en|acl-aware-tunnel-info-en | ipfix-aware-tunnel-info-en | \
         acl-flow-lkup-use-outer | ipfix-flow-lkup-use-outer|logic-port-check|sd-action| lb-hash-ecmp-profile| lb-hash-lag-profile| \
         discard-unencrypt-pkt| wlan-decap-with-rid)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port property",
        "Port cid",
        "Tx packet with cid header",
        "Acl aware tunnel info",
        "Ipfix aware tunnel info",
        "Acl flow lookup use outer info",
        "Ipfix flow lkup use outer info",
        "Logic port check",
        "Signal degrade action: 0-none; 1-discard oam pkt; 2-discard data pkt; 3-discard all",
        "Load balance hash ecmp profile id",
        "Load balance hash lag profile id",
        "Discard unencrypt packet",
        "WLAN decap with radio ID")
{
    int32 ret = 0;
    uint16 gport = 0;
    uint32 value = 0;
    uint32  prop = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    ctc_cli_out("Show GPort                        :  0x%04x\n", gport);
    ctc_cli_out("-------------------------------------------\n");

    if(CLI_CLI_STR_EQUAL("cid", 1))
    {
        prop = CTC_PORT_PROP_CID;
    }
    else if(CLI_CLI_STR_EQUAL("tx-cid-hdr-en", 1))
    {
        prop = CTC_PORT_PROP_TX_CID_HDR_EN;
    }
    else if (CLI_CLI_STR_EQUAL("acl-aware-tunnel-info", 1))
    {
        prop = CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN;
    }
    else if (CLI_CLI_STR_EQUAL("ipfix-aware-tunnel-info", 1))
    {
        prop = CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN;
    }
    else if (CLI_CLI_STR_EQUAL("acl-flow-lkup-use-outer", 1))
    {
        prop = CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD;
    }
    else if (CLI_CLI_STR_EQUAL("ipfix-flow-lkup-use-outer", 1))
    {
        prop = CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD;
    }
    else if (CLI_CLI_STR_EQUAL("logic-port-check", 1))
    {
        prop = CTC_PORT_PROP_LOGIC_PORT_CHECK_EN;
    }
    else if (CLI_CLI_STR_EQUAL("sd-action", 1))
    {
        prop = CTC_PORT_PROP_SD_ACTION;
    }
    else if (CLI_CLI_STR_EQUAL("lb-hash-ecmp-profile", 1))
    {
        prop = CTC_PORT_PROP_LB_HASH_ECMP_PROFILE;
    }
    else if (CLI_CLI_STR_EQUAL("lb-hash-lag-profile", 1))
    {
        prop = CTC_PORT_PROP_LB_HASH_LAG_PROFILE;
    }
    else if (CLI_CLI_STR_EQUAL("discard-unencrypt-pkt", 1))
    {
        prop = CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT;
    }
    else if (CLI_CLI_STR_EQUAL("wlan-decap-with-rid", 1))
    {
        prop = CTC_PORT_PROP_WLAN_DECAP_WITH_RID;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_property(g_api_lchip, gport, prop,&value);
    }
    else
    {
        ret = ctc_port_get_property(gport, prop, &value);
    }
    if (ret >= 0)
    {
        if(CTC_PORT_PROP_CID == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "cid", value);
        }
        else if(CTC_PORT_PROP_TX_CID_HDR_EN == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "tx with cid hdr", value);
        }
        else if (CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "acl-aware-tunnel-info", value);
        }
        else if (CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "ipfix-aware-tunnel-info", value);
        }
        else if (CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "acl-flow-lkup-use-outer", value);
        }
        else if (CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "ipfix-flow-lkup-use-outer", value);
        }
        else if (CTC_PORT_PROP_LOGIC_PORT_CHECK_EN == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "logic-port-check", value);
        }
        else if (CTC_PORT_PROP_SD_ACTION == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "sd-action", value);
        }
        else if (CTC_PORT_PROP_LB_HASH_ECMP_PROFILE == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "ecmp-profile", value);
        }
        else if (CTC_PORT_PROP_LB_HASH_LAG_PROFILE == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "lag-profile", value);
        }
        else if (CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "discard-unencrypt-pkt", value);
        }
        else if (CTC_PORT_PROP_WLAN_DECAP_WITH_RID == prop)
        {
            ctc_cli_out("%-34s:  %-3d\n", "wlan-decap-with-rid", value);
        }
        ctc_cli_out("------------------------------------------\n");
    }
    else
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_usw_port_set_isolation_mode,
        ctc_cli_usw_port_set_isolation_mode_cmd,
        "port isolation mode (port-mode|group-mode) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        "Port isolation",
        "Port isolation mode",
        "Port isolation basing port",
        "Port isolation basing group",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 choice_mode = 0;
    uint8 index = 0xFF;
    uint8 lchip = 0;
    int32 ret = 0;
    index = CTC_CLI_GET_ARGC_INDEX("port-mode");
    if (index != 0xFF)
    {
        choice_mode = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX("group-mode");
    if (index != 0xFF)
    {
        choice_mode = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_port_set_isolation_mode(lchip, choice_mode);
    return ret;
}

CTC_CLI(ctc_cli_port_show_internal_port,
        ctc_cli_port_show_internal_port_cmd,
        "show internal-port status (lchip LCHIP|)",
        "Show",
        "Internal port",
        "Status",
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

    ret = sys_usw_internal_port_show(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_port_self_checking,
    ctc_cli_usw_port_self_checking_cmd,
    "port  GPORT self-checking (lchip LCHIP|)",
    CTC_CLI_PORT_M_STR,
    CTC_CLI_GPHYPORT_ID_DESC,
    "Self-checking",
    CTC_CLI_LCHIP_ID_STR,
    CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 index = 0;
    uint32 gport = 0;
    uint8 lchip = 0;
    int32  ret = CLI_SUCCESS;

    CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_mac_self_checking(lchip, gport);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_port_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_system_min_frame_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_system_min_frame_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_vlan_range_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_port_sec_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_port_maclink_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_show_monitor_rlt_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_acl_hash_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_acl_hash_property_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_dir_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_port_dir_property_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_3ap_training_en_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_get_3ap_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_property_wlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_get_property_wlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_internal_port_cmd);

    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_isolation_mode_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_bpe_property_cmd);

    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_monitor_link_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_get_monitor_link_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_port_self_checking_cmd);


    return CLI_SUCCESS;
}

int32
ctc_usw_port_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_system_min_frame_size_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_system_min_frame_size_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_vlan_range_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_port_sec_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_port_maclink_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_show_monitor_rlt_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_acl_hash_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_acl_hash_property_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_dir_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_port_dir_property_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_3ap_training_en_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_get_3ap_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_show_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_property_wlan_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_get_property_wlan_cmd);

    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_isolation_mode_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_set_bpe_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_port_show_internal_port_cmd);

    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_set_monitor_link_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_port_get_monitor_link_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_port_self_checking_cmd);


    return CLI_SUCCESS;
}

