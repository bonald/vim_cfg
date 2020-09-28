/**
 @file ctc_security_cli.c

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
#include "ctc_security_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_security.h"
#include "ctc_error.h"
#include "ctc_debug.h"

CTC_CLI(ctc_cli_security_debug_on,
        ctc_cli_security_debug_on_cmd,
        "debug security (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SECURITY_STR,
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
        typeenum = SECURITY_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = SECURITY_SYS;
    }

    ctc_debug_set_flag("security", "security", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_security_debug_off,
        ctc_cli_security_debug_off_cmd,
        "no debug security (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SECURITY_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = SECURITY_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = SECURITY_SYS;
    }

    ctc_debug_set_flag("security", "security", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_security_show_debug,
        ctc_cli_security_show_debug_cmd,
        "show debug security (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_SECURITY_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = SECURITY_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = SECURITY_SYS;
    }

    en = ctc_debug_get_flag("security", "security", typeenum, &level);
    ctc_cli_out("security:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

/* #1 ctc_mac_security_set_port_security*/
CTC_CLI(ctc_cli_security_set_port_security,
        ctc_cli_security_set_port_security_cmd,
        "security port-security port GPHYPORT_ID (enable|disable)",
        CTC_CLI_SECURITY_STR,
        "Port based mac security",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport_id = 0;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = TRUE;
    }
    else if (CLI_CLI_STR_EQUAL("disable", 1))
    {
        enable = FALSE;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_set_port_security(g_api_lchip, gport_id, enable);
    }
    else
    {
        ret = ctc_mac_security_set_port_security(gport_id, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

/* #2 ctc_mac_security_set_port_mac_limit*/
CTC_CLI(ctc_cli_security_mac_security_set_port_learn_limit,
        ctc_cli_security_mac_security_set_port_learn_limit_cmd,
        "security mac-limit port GPHYPORT_ID action (none|fwd|discard|redirect-to-cpu)",
        CTC_CLI_SECURITY_STR,
        "Mac limit",
        "Port based mac limit",
        CTC_CLI_GPHYPORT_ID_DESC,
        "The action of mac limit",
        "Do nothing",
        "Forwarding packet",
        "Discard packet",
        "Redirect to cpu")
{
    int32 ret = 0;
    uint16 gport_id = 0;

    ctc_maclimit_action_t  action = CTC_MACLIMIT_ACTION_NONE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("n", 1))
    {
        action = CTC_MACLIMIT_ACTION_NONE;
    }
    else if (CLI_CLI_STR_EQUAL("f", 1))
    {

        action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("r", 1))
    {

        action = CTC_MACLIMIT_ACTION_TOCPU;
    }

    if (CLI_CLI_STR_EQUAL("d", 1))
    {

        action = CTC_MACLIMIT_ACTION_DISCARD;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_set_port_mac_limit(g_api_lchip, gport_id, action);
    }
    else
    {
        ret = ctc_mac_security_set_port_mac_limit(gport_id, action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

/* #3 ctc_mac_security_set_vlan_mac_limit*/
CTC_CLI(ctc_cli_security_mac_security_set_vlan_learn_limit,
        ctc_cli_security_mac_security_set_vlan_learn_limit_cmd,
        "security mac-limit vlan VLAN_ID action (none|fwd|discard|redirect-to-cpu)",
        CTC_CLI_SECURITY_STR,
        "Mac limit",
        "Vlan based mac limit",
        CTC_CLI_VLAN_RANGE_DESC,
        "The action of mac limit",
        "Do nothing",
        "Forwarding packet",
        "Discard packet",
        "Redirect to cpu")
{
    int32 ret = 0;
    uint16 vlan_id = 0;

    ctc_maclimit_action_t  action = CTC_MACLIMIT_ACTION_NONE;

    CTC_CLI_GET_UINT16_RANGE("vlan_id", vlan_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("n", 1))
    {
        action = CTC_MACLIMIT_ACTION_NONE;
    }
    else if (CLI_CLI_STR_EQUAL("f", 1))
    {

        action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("r", 1))
    {

        action = CTC_MACLIMIT_ACTION_TOCPU;
    }

    if (CLI_CLI_STR_EQUAL("d", 1))
    {

        action = CTC_MACLIMIT_ACTION_DISCARD;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_set_vlan_mac_limit(g_api_lchip, vlan_id, action);
    }
    else
    {
        ret = ctc_mac_security_set_vlan_mac_limit(vlan_id, action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

/* #4 ctc_mac_security_get_port_security*/
CTC_CLI(ctc_cli_security_mac_security_show_port_security,
        ctc_cli_security_mac_security_show_port_security_cmd,
        "show security port-security port GPHYPORT_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Port based mac security",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 gport_id = 0;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_get_port_security(g_api_lchip, gport_id, &enable);
    }
    else
    {
        ret = ctc_mac_security_get_port_security(gport_id, &enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("===============================\n");
    ctc_cli_out("gport:0x%04X  security enable:%d\n", gport_id, enable);
    ctc_cli_out("===============================\n");

    return CLI_SUCCESS;
}

/* #6 ctc_mac_security_get_port_mac_limit*/
CTC_CLI(ctc_cli_security_mac_security_show_mac_limit,
        ctc_cli_security_mac_security_show_mac_limit_cmd,
        "show security mac-limit (port |vlan) VALUE",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Mac limit",
        "Port based mac limit",
        "Vlan based mac limit",
        "The value of gport Id or vlan Id")
{
    int32 ret = 0;
    uint16 value = 0;
    ctc_maclimit_action_t  action;
    char action_str[CTC_MACLIMIT_ACTION_TOCPU + 1][32] = {{"Do nothing"}, {"forwarding packet"}, {"discard packet"}, {"redirect to cpu"}};

    CTC_CLI_GET_UINT16_RANGE("value id", value, argv[1], 0, CTC_MAX_UINT16_VALUE);

    ctc_cli_out("===============================\n");
    if (CLI_CLI_STR_EQUAL("port", 0))
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_get_port_mac_limit(g_api_lchip, value, &action);
    }
    else
    {
        ret = ctc_mac_security_get_port_mac_limit(value, &action);
    }
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("Port:%d action:%s\n", value, action_str[action]);

    }
    else
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_get_vlan_mac_limit(g_api_lchip, value, &action);
    }
    else
    {
        ret = ctc_mac_security_get_vlan_mac_limit(value, &action);
    }
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("Vlan:%d  action:%s\n", value, action_str[action]);

    }

    ctc_cli_out("===============================\n");

    return CLI_SUCCESS;

}

/* #8 ctc_ip_source_guard_add_entry*/
CTC_CLI(ctc_cli_security_ip_src_guard_add_entry,
        ctc_cli_security_ip_src_guard_add_entry_cmd,
        "security ip-src-guard add port GPORT_ID {ipv4 A.B.C.D |mac MAC|vlan VLAN_ID (is-svlan |)|use-logic-port|use-flex | scl-id SCL_ID |}",
        CTC_CLI_SECURITY_STR,
        "Ip source guard",
        "Add ip source guard entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan id is svlan id",
        "Port is logic Port",
        "Use tcam key",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 index_ipv4 = 0;
    uint8 index_mac = 0;
    uint8 index_vlan = 0;

    mac_addr_t macsa;
    ctc_security_ip_source_guard_elem_t elem;

    sal_memset(macsa, 0, sizeof(mac_addr_t));
    sal_memset(&elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));
    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAX;

    CTC_CLI_GET_UINT16_RANGE("gport", elem.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index_ipv4 = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (index_ipv4 != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", elem.ipv4_sa, argv[index_ipv4 + 1]);
        elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    }

    index_mac = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index_mac != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index_mac + 1]);

        sal_memcpy(&(elem.mac_sa), &macsa, sizeof(mac_addr_t));
    }

    index_vlan = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index_vlan != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", elem.vid, argv[index_vlan + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl id", elem.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if ((index_ipv4 != 0xFF) && (index_mac == 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP;
    }

    if ((index_ipv4 != 0xFF) && (index_mac != 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC;
    }

    if ((index_ipv4 != 0xFF) && (index_mac == 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_VLAN;
    }

    if ((index_ipv4 != 0xFF) && (index_mac != 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN;
    }

    if ((index_ipv4 == 0xFF) && (index_mac != 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAC;
    }

    if ((index_ipv4 == 0xFF) && (index_mac != 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAC_VLAN;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_ip_source_guard_add_entry(g_api_lchip, &elem);
    }
    else
    {
        ret = ctc_ip_source_guard_add_entry(&elem);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/* #9 ctc_ip_source_guard_remove_entry*/
CTC_CLI(ctc_cli_security_ip_src_guard_remove_entry,
        ctc_cli_security_ip_src_guard_remove_entry_cmd,
        "security ip-src-guard remove port GPORT_ID { ipv4 A.B.C.D |mac MAC|vlan VLAN_ID (is-svlan |)|use-logic-port|use-flex | scl-id SCL_ID |} ",
        CTC_CLI_SECURITY_STR,
        "Ip source guard",
        "Remove ip source guard entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan id is svlan id",
        "Port is logic Port",
        "Use tcam key",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 index_ipv4 = 0;
    uint8 index_mac = 0;
    uint8 index_vlan = 0;

    mac_addr_t macsa;
    ctc_security_ip_source_guard_elem_t elem;

    sal_memset(macsa, 0, sizeof(mac_addr_t));
    sal_memset(&elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));
    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAX;

    CTC_CLI_GET_UINT16_RANGE("gport", elem.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index_ipv4 = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (index_ipv4 != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", elem.ipv4_sa, argv[index_ipv4 + 1]);
        elem.ipv4_or_ipv6 = CTC_IP_VER_4;
    }

    index_mac = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index_mac != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index_mac + 1]);

        sal_memcpy(&(elem.mac_sa), &macsa, sizeof(mac_addr_t));
    }

    index_vlan = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index_vlan != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", elem.vid, argv[index_vlan + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl id", elem.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if ((index_ipv4 != 0xFF) && (index_mac == 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP;
    }

    if ((index_ipv4 != 0xFF) && (index_mac != 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC;
    }

    if ((index_ipv4 != 0xFF) && (index_mac == 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_VLAN;
    }

    if ((index_ipv4 != 0xFF) && (index_mac != 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN;
    }

    if ((index_ipv4 == 0xFF) && (index_mac != 0xFF) && (index_vlan == 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAC;
    }

    if ((index_ipv4 == 0xFF) && (index_mac != 0xFF) && (index_vlan != 0xFF))
    {
        elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAC_VLAN;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_ip_source_guard_remove_entry(g_api_lchip, &elem);
    }
    else
    {
        ret = ctc_ip_source_guard_remove_entry(&elem);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_security_ipv6_src_guard_add_entry,
        ctc_cli_security_ipv6_src_guard_add_entry_cmd,
        "security ip-src-guard add port GPORT_ID ipv6 X:X::X:X mac MAC {use-flex | scl-id SCL_ID |}",
        CTC_CLI_SECURITY_STR,
        "Ip source guard",
        "Add ip source guard entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Use tcam key",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret = 0;
    ipv6_addr_t ipv6_address;
    uint8 index = 0;
    mac_addr_t macsa;
    ctc_security_ip_source_guard_elem_t elem;

    sal_memset(macsa, 0, sizeof(mac_addr_t));
    sal_memset(&elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));
    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAX;

    CTC_CLI_GET_UINT16_RANGE("gport", elem.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[1]);
    elem.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
    elem.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
    elem.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
    elem.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_MAC_ADDRESS("mac source address", macsa, argv[2]);
    sal_memcpy(&(elem.mac_sa), &macsa, sizeof(mac_addr_t));

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl id", elem.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IPV6_MAC;
    elem.ipv4_or_ipv6 = CTC_IP_VER_6;

    if(g_ctcs_api_en)
    {
        ret = ctcs_ip_source_guard_add_entry(g_api_lchip, &elem);
    }
    else
    {
        ret = ctc_ip_source_guard_add_entry(&elem);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_security_ipv6_src_guard_remove_entry,
        ctc_cli_security_ipv6_src_guard_remove_entry_cmd,
        "security ip-src-guard remove port GPORT_ID ipv6 X:X::X:X mac MAC {use-flex | scl-id SCL_ID |}",
        CTC_CLI_SECURITY_STR,
        "Ip source guard",
        "Remove ip source guard entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        "MAC address, not necessary",
        CTC_CLI_MAC_FORMAT,
        "Use tcam key",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    ipv6_addr_t ipv6_address;
    ctc_security_ip_source_guard_elem_t elem;

    sal_memset(&elem, 0, sizeof(ctc_security_ip_source_guard_elem_t));

    CTC_CLI_GET_UINT16_RANGE("gport", elem.gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_MAX;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[1]);
    elem.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
    elem.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
    elem.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
    elem.ipv6_sa[3] = sal_htonl(ipv6_address[3]);


    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        elem.flag |= CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl id", elem.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    elem.ip_src_guard_type = CTC_SECURITY_BINDING_TYPE_IPV6_MAC;
    elem.ipv4_or_ipv6 = CTC_IP_VER_6;

    if(g_ctcs_api_en)
    {
        ret = ctcs_ip_source_guard_remove_entry(g_api_lchip, &elem);
    }
    else
    {
        ret = ctc_ip_source_guard_remove_entry(&elem);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/* #10  ctc_storm_ctl_set_cfg*/
CTC_CLI(ctc_cli_security_storm_ctl_set_config,
        ctc_cli_security_storm_ctl_set_config_cmd,
        "security storm-ctl (port GPHYPORT_ID| vlan VLAN_ID) (enable|disable) mode (pps | bps | kpps | kbps) (threshold THRESHOLD_NUM) \
    type(broadcast|known-multicast|unknown-multicast|all-multicast|known-unicast|unknown-unicast|all-unicast)\
    (discard-to-cpu|)",
        CTC_CLI_SECURITY_STR,
        "Storm control",
        "Port based Storm control",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Vlan based Storm control",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "The mode of storm control",
        "Packet per second",
        "Bytes per second",
        "1000 Packet per second",
        "1000 Bytes per second",
        "Threshold",
        "Bps <0-PortMaxSpeed/8>, must be in multiples of 1000, pps:(<0-PortMaxSpeed/(8*84)>) ",
        "The type of storm control",
        "Broadcast",
        "Known multicast",
        "Unknown multicast",
        "All multicast, known multicast + unknown multicast",
        "Known unicast",
        "Unknown unicast",
        "All unicast, known unicast + unknown unicast",
        "Discarded packet by storm control and redirect to cpu")
{
    uint16 gport_id = 0;
    uint16 vlan_id = 0;
    uint32 value = 0;
    int32 ret = 0;
    uint8 index = 0;
    ctc_security_storm_ctl_type_t type = 0;
    ctc_security_stmctl_cfg_t  stmctl_cfg;

    sal_memset(&stmctl_cfg, 0, sizeof(stmctl_cfg));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        stmctl_cfg.gport = gport_id;
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        stmctl_cfg.vlan_id = vlan_id;
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_VLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        stmctl_cfg.storm_en = 1;
    }
    else
    {
        stmctl_cfg.storm_en = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pps");
    if (0xFF != index)
    {
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_PPS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bps");
    if (0xFF != index)
    {
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_BPS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("kpps");
    if (0xFF != index)
    {
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_KPPS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("kbps");
    if (0xFF != index)
    {
        stmctl_cfg.mode = CTC_SECURITY_STORM_CTL_MODE_KBPS;
    }


    index = CTC_CLI_GET_ARGC_INDEX("threshold");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("threshold", value, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        stmctl_cfg.threshold = value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("broadcast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_BCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_KNOWN_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_KNOWN_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_ALL_UCAST;
    }

    stmctl_cfg.type = type;

    index = CTC_CLI_GET_ARGC_INDEX("discard-to-cpu");
    if (0xFF != index)
    {
        stmctl_cfg.discarded_to_cpu = 1;
    }
    else
    {
        stmctl_cfg.discarded_to_cpu = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_storm_ctl_set_cfg(g_api_lchip, &stmctl_cfg);
    }
    else
    {
        ret = ctc_storm_ctl_set_cfg(&stmctl_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* #11  ctc_storm_ctl_get_cfg*/
CTC_CLI(ctc_cli_security_storm_ctl_show_cfg,
        ctc_cli_security_storm_ctl_show_cfg_cmd,
        "show security storm-ctl (port GPHYPORT_ID| vlan VLAN_ID) \
    (broadcast|known-multicast|unknown-multicast|all-multicast|known-unicast|unknown-unicast|all-unicast)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Storm control",
        "Port configure",
        CTC_CLI_GPORT_DESC,
        "Vlan based Storm control",
        CTC_CLI_VLAN_RANGE_DESC,
        "Broadcast",
        "Known multicast",
        "Unknown multicast",
        "All multicast, known multicast + unknown multicast",
        "Known unicast",
        "Unknown unicast",
        "All unicast, known unicast + unknown unicast")
{
    uint16 gport_id = 0;
    uint16 vlan_id = 0;
    int32 ret = 0;
    ctc_security_storm_ctl_type_t type = 0;
    ctc_security_stmctl_cfg_t stmctl_cfg;
    uint16 index  = 0;

    sal_memset(&stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", gport_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        stmctl_cfg.gport = gport_id;
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_PORT;
        ctc_cli_out("gport 0x%04X storm control:\n", gport_id);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        stmctl_cfg.vlan_id = vlan_id;
        stmctl_cfg.op = CTC_SECURITY_STORM_CTL_OP_VLAN;
        ctc_cli_out("Vlan %d storm control:\n", vlan_id);
    }

    index = CTC_CLI_GET_ARGC_INDEX("broadcast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_BCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_KNOWN_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-multicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_KNOWN_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-unicast");
    if (0xFF != index)
    {
        type = CTC_SECURITY_STORM_CTL_ALL_UCAST;
    }

    stmctl_cfg.type  = type;
    if(g_ctcs_api_en)
    {
        ret = ctcs_storm_ctl_get_cfg(g_api_lchip, &stmctl_cfg);
    }
    else
    {
        ret = ctc_storm_ctl_get_cfg(&stmctl_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\ttype:");

    switch (stmctl_cfg.type)
    {
    case CTC_SECURITY_STORM_CTL_BCAST:
        ctc_cli_out("broadcast\n");
        break;

    case CTC_SECURITY_STORM_CTL_KNOWN_MCAST:
        ctc_cli_out("known-multicast\n");
        break;

    case CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST:
        ctc_cli_out("unknown-multicast\n");
        break;

    case CTC_SECURITY_STORM_CTL_ALL_MCAST:
        ctc_cli_out("all-multicast\n");
        break;

    case CTC_SECURITY_STORM_CTL_KNOWN_UCAST:
        ctc_cli_out("known-unicast\n");
        break;

    case CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST:
        ctc_cli_out("unknown-unicast\n");
        break;

    case CTC_SECURITY_STORM_CTL_ALL_UCAST:
        ctc_cli_out("all-unicast\n");
        break;

    default:
        ctc_cli_out("none\n");
        return CLI_ERROR;
        break;
    }

    ctc_cli_out("\tenable:%d\n", stmctl_cfg.storm_en);
    if (CTC_SECURITY_STORM_CTL_MODE_PPS == stmctl_cfg.mode)
    {
        ctc_cli_out("\tthreshold:%u (packets per second)\n", stmctl_cfg.threshold);
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_BPS == stmctl_cfg.mode)
    {
        ctc_cli_out("\tthreshold:%u (bytes per second)\n", stmctl_cfg.threshold);
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_KPPS == stmctl_cfg.mode)
    {
        ctc_cli_out("\tthreshold:%u (1000 packets per second)\n", stmctl_cfg.threshold);
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_KBPS == stmctl_cfg.mode)
    {
        ctc_cli_out("\tthreshold:%u (1000 bytes per second)\n", stmctl_cfg.threshold);
    }


    ctc_cli_out("\texception enable:%d\n", stmctl_cfg.discarded_to_cpu);
    ctc_cli_out("\tdiscarded_to_cpu:%d\n", stmctl_cfg.discarded_to_cpu);
    return CLI_SUCCESS;
}

/* #12 ctc_storm_ctl_get_global_cfg*/
CTC_CLI(ctc_cli_security_storm_ctl_show_global_cfg,
        ctc_cli_security_storm_ctl_show_global_cfg_cmd,
        "show security storm-ctl glb-cfg",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Storm control",
        "Global configure")
{
    int32 ret = 0;
    ctc_security_stmctl_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_security_stmctl_glb_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_storm_ctl_get_global_cfg(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_storm_ctl_get_global_cfg(&glb_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Global storm control:\n");
    ctc_cli_out("\tipg_en:%d\n", glb_cfg.ipg_en);
    ctc_cli_out("\tunicast storm ctl mode:%d\n", glb_cfg.ustorm_ctl_mode);
    ctc_cli_out("\tmulticast storm ctl mode:%d\n", glb_cfg.mstorm_ctl_mode);
    ctc_cli_out("\tgranularity mode:%d <0 - 100 mode, 1 - 1000 mode, 3 - 5000 mode>\n", glb_cfg.granularity);

    return CLI_SUCCESS;
}

/* #13 ctc_port_isolation_set_route_obey_isolated_en*/
CTC_CLI(ctc_cli_security_port_isolation_set_route_obey,
        ctc_cli_security_port_isolation_set_route_obey_cmd,
        "security port-isolation route-obey (enable|disable)",
        CTC_CLI_SECURITY_STR,
        "Port isolation",
        "Route obey isolate",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    bool enable = FALSE;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else if (CLI_CLI_STR_EQUAL("disable", 0))
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_isolation_set_route_obey_isolated_en(g_api_lchip, enable);
    }
    else
    {
        ret = ctc_port_isolation_set_route_obey_isolated_en(enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* #15 ctc_port_isolation_get_route_obey_isolated_en*/
CTC_CLI(ctc_cli_security_port_isolation_show_route_obey,
        ctc_cli_security_port_isolation_show_route_obey_cmd,
        "show security port-isolation route-obey",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Port isolation",
        "Route obey isolate")
{
    int32 ret = 0;
    bool enable = FALSE;

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_isolation_get_route_obey_isolated_en(g_api_lchip, &enable);
    }
    else
    {
        ret = ctc_port_isolation_get_route_obey_isolated_en(&enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("===============================\n");
    ctc_cli_out("Route-obey enable:%d\n", enable);
    ctc_cli_out("===============================\n");

    return CLI_SUCCESS;
}

/* #16 ctc_storm_ctl_set_global_cfg*/
CTC_CLI(ctc_cli_security_storm_ctl_set_global_cfg,
        ctc_cli_security_storm_ctl_set_global_cfg_cmd,
        "security storm-ctl {ipg (enable|disable) | ucast-ctl-mode (enable|disable)\
        | mcast-ctl-mode (enable|disable)| granu VALUE |}",
        CTC_CLI_SECURITY_STR,
        "Storm control",
        "Ipg",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Ucast control mode",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Mcast control mode",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Granularity",
        "0:100, 1:1000, 2:10000, 3:5000, default and other:100, pps or bps")
{
    int32 ret = 0;
    uint8 value = 0;
    uint8 index = 0;
    ctc_security_stmctl_glb_cfg_t stmctl_glb_cfg;

    sal_memset(&stmctl_glb_cfg, 0, sizeof(ctc_security_stmctl_glb_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_storm_ctl_get_global_cfg(g_api_lchip, &stmctl_glb_cfg);
    }
    else
    {
        ret = ctc_storm_ctl_get_global_cfg(&stmctl_glb_cfg);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipg");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            stmctl_glb_cfg.ipg_en = 1;
        }
        else if (CLI_CLI_STR_EQUAL("disable", index + 1))
        {
            stmctl_glb_cfg.ipg_en = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ucast-ctl-mode");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            stmctl_glb_cfg.ustorm_ctl_mode = 1;
        }
        else if (CLI_CLI_STR_EQUAL("disable", index + 1))
        {
            stmctl_glb_cfg.ustorm_ctl_mode = 0;
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX("mcast-ctl-mode");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            stmctl_glb_cfg.mstorm_ctl_mode = 1;
        }
        else if (CLI_CLI_STR_EQUAL("disable", index + 1))
        {
            stmctl_glb_cfg.mstorm_ctl_mode = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("granu");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("granularity", value, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

        stmctl_glb_cfg.granularity = value;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_storm_ctl_set_global_cfg(g_api_lchip, &stmctl_glb_cfg);
    }
    else
    {
        ret = ctc_storm_ctl_set_global_cfg(&stmctl_glb_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* #18 ctc_mac_security_set_system_mac_limit*/
CTC_CLI(ctc_cli_security_mac_security_set_system_mac_limit,
        ctc_cli_security_mac_security_set_system_mac_limit_cmd,
        "security mac-limit system action (none|fwd|discard|redirect-to-cpu)",
        CTC_CLI_SECURITY_STR,
        "Mac limit",
        "System based mac limit",
        "The action of mac limit",
        "Do nothing",
        "Forwarding packet",
        "Discard packet",
        "Redirect to cpu")
{
    int32 ret = 0;

    ctc_maclimit_action_t  action = CTC_MACLIMIT_ACTION_NONE;

    if (CLI_CLI_STR_EQUAL("n", 0))
    {
        action = CTC_MACLIMIT_ACTION_NONE;
    }
    else if (CLI_CLI_STR_EQUAL("f", 0))
    {

        action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("r", 0))
    {

        action = CTC_MACLIMIT_ACTION_TOCPU;
    }

    if (CLI_CLI_STR_EQUAL("d", 0))
    {

        action = CTC_MACLIMIT_ACTION_DISCARD;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_set_system_mac_limit(g_api_lchip, action);
    }
    else
    {
        ret = ctc_mac_security_set_system_mac_limit(action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/* #19 ctc_mac_security_get_system_mac_limit*/
CTC_CLI(ctc_cli_security_mac_security_show_system_mac_limit,
        ctc_cli_security_mac_security_show_system_mac_limit_cmd,
        "show security mac-limit system",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Mac limit",
        "System based mac limit")
{
    int32 ret = 0;
    ctc_maclimit_action_t  action;
    char action_str[CTC_MACLIMIT_ACTION_TOCPU + 1][32] = {{"Do Nothing"}, {"forwarding packet"}, {"discard packet"}, {"redirect to cpu"}};

    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_get_system_mac_limit(g_api_lchip, &action);
    }
    else
    {
        ret = ctc_mac_security_get_system_mac_limit(&action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("System mac limit action:%s\n", action_str[action]);

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_security_mac_security_set_learn_limit,
        ctc_cli_security_mac_security_set_learn_limit_cmd,
        "security learn-limit type (port GPHYPORT_ID |vlan VLAN_ID |system) (maximum NUM) action (fwd|discard|redirect-to-cpu)",
        CTC_CLI_SECURITY_STR,
        "Learn limit",
        "Learn limit type",
        "Port based learn limit",
        CTC_CLI_GPORT_DESC,
        "Vlan based learn limit",
        CTC_CLI_VLAN_RANGE_DESC,
        "System based learn limit",
        "Maximum number",
        "0xFFFFFFFF means disable",
        "The action of mac limit",
        "Forwarding packet",
        "Discard packet",
        "Redirect to cpu")
{
    int32 ret = 0;
    ctc_security_learn_limit_t learn_limit;
    uint8 index = 0;

    sal_memset(&learn_limit, 0, sizeof(ctc_security_learn_limit_t));

    if (CLI_CLI_STR_EQUAL("port", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("gport", learn_limit.gport, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("vlan", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vlan id", learn_limit.vlan, argv[index], 0, CTC_MAX_UINT16_VALUE);

    }
    else if (CLI_CLI_STR_EQUAL("system", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
    }

    index ++;

    if (CLI_CLI_STR_EQUAL("maximum", index))
    {
        index ++;
        CTC_CLI_GET_UINT32_RANGE("max num", learn_limit.limit_num, argv[index], 0, CTC_MAX_UINT32_VALUE);
    }

    index ++;

    if (CLI_CLI_STR_EQUAL("f", index))
    {
        learn_limit.limit_action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("d", index))
    {

        learn_limit.limit_action  = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else if (CLI_CLI_STR_EQUAL("r", index))
    {

        learn_limit.limit_action  = CTC_MACLIMIT_ACTION_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_set_learn_limit(g_api_lchip, &learn_limit);
    }
    else
    {
        ret = ctc_mac_security_set_learn_limit(&learn_limit);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_security_mac_security_show_learn_limit,
        ctc_cli_security_mac_security_show_learn_limit_cmd,
        "show security learn-limit type (port GPHYPORT_ID |vlan VLAN_ID |system)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SECURITY_STR,
        "Learn limit",
        "Learn limit type",
        "Port based learn limit",
        CTC_CLI_GPORT_DESC,
        "Vlan based learn limit",
        CTC_CLI_VLAN_RANGE_DESC,
        "System based learn limit")
{
    int32 ret = 0;
    ctc_security_learn_limit_t learn_limit;
    uint8 index = 0;
    uint32 limit_num = 0;
    uint8 action = 0;
    char action_str[CTC_MACLIMIT_ACTION_TOCPU + 1][32] = {{"None"}, {"forwarding packet"}, {"discard packet"}, {"redirect to cpu"}};
    ctc_l2_fdb_query_t Query;

    sal_memset(&learn_limit, 0, sizeof(ctc_security_learn_limit_t));
    sal_memset(&Query, 0, sizeof(ctc_l2_fdb_query_t));

    Query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    Query.query_hw = 1;

    if (CLI_CLI_STR_EQUAL("port", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("gport", learn_limit.gport, argv[index], 0, CTC_MAX_UINT16_VALUE);
        Query.gport = learn_limit.gport;
    }
    else if (CLI_CLI_STR_EQUAL("vlan", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN;
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vlan id", learn_limit.vlan, argv[index], 0, CTC_MAX_UINT16_VALUE);
        Query.fid = learn_limit.vlan;

    }
    else if (CLI_CLI_STR_EQUAL("system", index))
    {
        learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
        Query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mac_security_get_learn_limit(g_api_lchip, &learn_limit);
    }
    else
    {
        ret = ctc_mac_security_get_learn_limit(&learn_limit);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    action = learn_limit.limit_action;
    limit_num = learn_limit.limit_num;

    if (CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == learn_limit.limit_type)
    {
        ctc_cli_out("Port 0x%x :\n", learn_limit.gport);
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN == learn_limit.limit_type)
    {
        ctc_cli_out("Vlan 0x%x :\n", learn_limit.vlan);
    }
    else
    {
        ctc_cli_out("System :\n");
    }

    ctc_cli_out("Mac limit action %s\n", action_str[action]);
    if (0xFFFFFFFF != limit_num)
    {
        ctc_cli_out("Max number is  0x%x\n", limit_num);
    }
    else
    {
        ctc_cli_out("Max number is  %s\n", "disabled");
    }

    ctc_cli_out("Running number is  0x%x\n", learn_limit.limit_count);

    return ret;
}

int32
ctc_security_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_security_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_show_debug_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_set_vlan_learn_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_set_port_learn_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_show_mac_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_set_system_mac_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_show_system_mac_limit_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_set_port_security_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_show_port_security_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_storm_ctl_set_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_storm_ctl_show_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_storm_ctl_show_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_storm_ctl_set_global_cfg_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_ip_src_guard_add_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_ip_src_guard_remove_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_port_isolation_set_route_obey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_port_isolation_show_route_obey_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_security_ipv6_src_guard_add_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_ipv6_src_guard_remove_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_set_learn_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_security_mac_security_show_learn_limit_cmd);

    return CLI_SUCCESS;

}

