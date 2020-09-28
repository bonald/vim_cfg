/**
 @file ctc_trill_cli.c

 @date 2013-10-25

 @version v3.0

 The file apply clis of trill module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_trill.h"
#include "ctc_trill_cli.h"



CTC_CLI(ctc_cli_trill_route,
        ctc_cli_trill_route_cmd,
        "trill route (add|remove) (mcast|) ({egs-nickname NICKNAME (vlan VLAN_ID |)| igs-nickname NICKNAME} | default-entry) {discard | nhid NHID| src-port GPORT_ID| macsa MAC|}",
        CTC_CLI_TRILL_M_STR,
        "Route entry",
        "Add route entry",
        "Remove route entry",
        "Mcast",
        "Egress nickname",
        "Egress nickname",
        CTC_CLI_VLAN_DESC,
        "Vlan id",
        "Ingress nickname",
        "Ingress nickname",
        "Default entry",
        "Discard entry",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT)
{
    ctc_trill_route_t trill_route;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&trill_route, 0 ,sizeof(trill_route));

    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_route.flag, CTC_TRILL_ROUTE_FLAG_MCAST);
    }

    index = CTC_CLI_GET_ARGC_INDEX("egs-nickname");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nickname", trill_route.egress_nickname, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igs-nickname");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nickname", trill_route.ingress_nickname, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_route.flag, CTC_TRILL_ROUTE_FLAG_DISCARD);
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_route.flag, CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nhid", trill_route.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vlan", trill_route.vlan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("src-port", trill_route.src_gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        CTC_SET_FLAG(trill_route.flag, CTC_TRILL_ROUTE_FLAG_SRC_PORT_CHECK);
    }

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", trill_route.mac_sa, argv[index + 1]);
        CTC_SET_FLAG(trill_route.flag, CTC_TRILL_ROUTE_FLAG_MACSA_CHECK);
    }


    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_trill_add_route(g_api_lchip, &trill_route);
        }
        else
        {
            ret = ctc_trill_add_route(&trill_route);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_trill_remove_route(g_api_lchip, &trill_route);
        }
        else
        {
            ret = ctc_trill_remove_route(&trill_route);
        }
    }
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}




CTC_CLI(ctc_cli_trill_tunnel,
        ctc_cli_trill_tunnel_cmd,
        "trill tunnel (add|remove|update) (mcast|) ({egs-nickname NICKNAME| igs-nickname NICKNAME} | default-entry) {nhid NHID| fid FID| tunnel-src-port PORTID|stats STATS_ID\
        |inner-router-mac MAC|deny-learning|arp-action (copy-to-cpu|forward|redirect-to-cpu|discard) |service-acl-en |}",
        CTC_CLI_TRILL_M_STR,
        "Trill tunnel entry",
        "Add tunnel entry",
        "Remove tunnel entry",
        "Update tunnel entry",
        "Mcast",
        "Egress nickname",
        "Egress nickname",
        "Ingress nickname",
        "Ingress nickname",
        "Default entry",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Trill tunnel source port",
        "Trill tunnel source port, 0 means not used",
        "Statistic",
        CTC_CLI_STATS_ID_VAL,
        "Router mac for Inner L2 MacDa",
        CTC_CLI_MAC_FORMAT,
        "Deny learning",
        "ARP packet action",
        "Forwarding and to cpu",
        "Normal forwarding",
        "Redirect to cpu",
        "Discard",
        "Service acl enable")
{
    ctc_trill_tunnel_t trill_tunnel;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&trill_tunnel, 0 ,sizeof(trill_tunnel));

    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_tunnel.flag, CTC_TRILL_TUNNEL_FLAG_MCAST);
    }

    index = CTC_CLI_GET_ARGC_INDEX("egs-nickname");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nickname", trill_tunnel.egress_nickname, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igs-nickname");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nickname", trill_tunnel.ingress_nickname, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_tunnel.flag, CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("nhid", trill_tunnel.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("fid", trill_tunnel.fid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-src-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("tunnel-src-port", trill_tunnel.logic_src_port, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("stats", trill_tunnel.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("inner-router-mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", trill_tunnel.route_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_tunnel.flag, CTC_TRILL_TUNNEL_FLAG_DENY_LEARNING);
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_tunnel.flag, CTC_TRILL_TUNNEL_FLAG_ARP_ACTION);

        if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            trill_tunnel.arp_action = CTC_EXCP_NORMAL_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("redirect-to-cpu", index+1))
        {
            trill_tunnel.arp_action = CTC_EXCP_DISCARD_AND_TO_CPU;
        }
        else if (CLI_CLI_STR_EQUAL("copy-to-cpu", index+1))
        {
            trill_tunnel.arp_action = CTC_EXCP_FWD_AND_TO_CPU;
        }
        else
        {
            trill_tunnel.arp_action = CTC_EXCP_DISCARD;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(trill_tunnel.flag, CTC_TRILL_TUNNEL_FLAG_SERVICE_ACL_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_trill_add_tunnel(g_api_lchip, &trill_tunnel);
        }
        else
        {
            ret = ctc_trill_add_tunnel(&trill_tunnel);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_trill_remove_tunnel(g_api_lchip, &trill_tunnel);
        }
        else
        {
            ret = ctc_trill_remove_tunnel(&trill_tunnel);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_trill_update_tunnel(g_api_lchip, &trill_tunnel);
        }
        else
        {
            ret = ctc_trill_update_tunnel(&trill_tunnel);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_trill_debug_on,
        ctc_cli_trill_debug_on_cmd,
        "debug trill (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_TRILL_M_STR,
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
    else
    {
        level = CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = TRILL_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = TRILL_SYS;
    }

    ctc_debug_set_flag("trill", "trill", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_trill_debug_off,
        ctc_cli_trill_debug_off_cmd,
        "no debug trill (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_TRILL_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = TRILL_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = TRILL_SYS;
    }

    ctc_debug_set_flag("trill", "trill", typeenum, level, FALSE);

    return CLI_SUCCESS;
}


int32
ctc_trill_cli_init(void)
{

    install_element(CTC_SDK_MODE, &ctc_cli_trill_route_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_trill_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_trill_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_trill_debug_off_cmd);

    return CLI_SUCCESS;
}

