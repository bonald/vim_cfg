/**
 @file ctc_nexthop_cli.c

 @date 2009-11-30

 @version v2.0

 The file apply clis of port module
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_l2.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#include "ctc_nexthop_cli.h"

enum cli_usw_nh_type_e
{
    CLI_GOLDENGATE_NH_TYPE_NULL,
    CLI_GOLDENGATE_NH_TYPE_MCAST,
    CLI_GOLDENGATE_NH_TYPE_BRGUC,
    CLI_GOLDENGATE_NH_TYPE_IPUC,
    CLI_GOLDENGATE_NH_TYPE_MPLS,
    CLI_GOLDENGATE_NH_TYPE_ECMP, /*For IPUC, MPLS, etc*/
    CLI_GOLDENGATE_NH_TYPE_DROP,
    CLI_GOLDENGATE_NH_TYPE_TOCPU,
    CLI_GOLDENGATE_NH_TYPE_UNROV,
    CLI_GOLDENGATE_NH_TYPE_ILOOP,
    CLI_GOLDENGATE_NH_TYPE_ELOOP,
    CLI_GOLDENGATE_NH_TYPE_RSPAN,
    CLI_GOLDENGATE_NH_TYPE_IP_TUNNEL,
    CLI_GOLDENGATE_NH_TYPE_TRILL,
    CLI_GOLDENGATE_NH_TYPE_MISC,
    CLI_GOLDENGATE_NH_TYPE_WLAN_TUNNEL,
    CLI_GOLDENGATE_NH_TYPE_MAX
};
typedef enum cli_usw_nh_type_e cli_usw_nh_type_t;


extern int32
sys_usw_nh_dump_all(uint8 lchip, cli_usw_nh_type_t nh_type, uint8 dump_type);

extern int32
sys_usw_nh_display_current_global_sram_info(uint8 lchip);

extern int32
sys_usw_nh_dump_mpls_tunnel(uint8 lchip, uint16 tunnel_id, bool detail);

extern int32
sys_usw_nh_dump(uint8 lchip, uint32 nhid, bool detail);

extern int32
sys_usw_nh_dump_mcast_group(uint8 lchip, uint32 group_id, bool detail);

extern int32
sys_usw_nh_dump_arp(uint8 lchip, uint16 arp_id, bool detail);

extern int32
sys_usw_nh_dump_resource_usage(uint8 lchip);

CTC_CLI(ctc_cli_usw_nh_show_nexthop_by_nhid_type,
        ctc_cli_usw_nh_show_nexthop_by_nhid_type_cmd,
        "show nexthop (NHID | mcast group GROUP) (detail |)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ID_STR,
        "Mcast",
        "Mcast group",
        "Group id <1-Max>",
        "Display detail information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 nhid;
    int32 ret = CLI_ERROR;
    bool detail = FALSE;
    uint8 index = 0;
    uint8 lchip = 0;

  	index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("detail");
    if (0xFF != index)
    {
         detail = TRUE;
    }

	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast");
    if (0xFF != index)
    {
        uint32 group_id = 0;
        CTC_CLI_GET_UINT32("Group id", group_id, argv[index + 2]);
        ret = sys_usw_nh_dump_mcast_group(lchip, group_id, detail);
    }
    else
    {
        CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);
        ret = sys_usw_nh_dump(lchip, nhid, detail);
    }

    if (ret)
    {
        ctc_cli_out("%% Dump nexthop fail\n");
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_nh_show_nexthop_all_by_type,
        ctc_cli_usw_nh_show_nexthop_all_by_type_cmd,
        "show nexthop all ((mcast|brguc|ipuc|ecmp|mpls|iloop|rspan|ip-tunnel|misc|trill|wlan-tunnel)|) (internal|external|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "All nexthop",
        "Multicast nexthop",
        "Bridge unicast nexthop",
        "Ipuc nexthop",
        "ECMP nexthop",
        "MPLS nexthop",
        "ILoop nexthop",
        "RSPAN nexthop",
        "Ip tunnel nexthop",
        "Misc nexthop",
        "Trill nexthop",
        "Wlan tunnel nexthop",
        "Internal nexthop",
        "External nexthop",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 type = CLI_GOLDENGATE_NH_TYPE_MAX;
    uint8 dump_type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("brguc");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_BRGUC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipuc");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_IPUC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_MPLS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_ECMP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("iloop");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_ILOOP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rspan");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_RSPAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-tunnel");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_IP_TUNNEL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_TRILL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("misc");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_MISC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-tunnel");
    if (0xFF != index)
    {
        type = CLI_GOLDENGATE_NH_TYPE_WLAN_TUNNEL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("internal");
    if (0xFF != index)
    {
        dump_type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("external");
    if (0xFF != index)
    {
        dump_type = 2;
    }

    ret = sys_usw_nh_dump_all(lchip, type, dump_type);
    if (ret)
    {
        ctc_cli_out("%% Dump nexthop fail\n");
    }

    return ret;
}

extern int32
sys_usw_nh_set_ipmc_logic_replication(uint8 lchip, uint8 enable);
extern int32
sys_usw_nh_set_pkt_nh_edit_mode(uint8 lchip, uint8 edit_mode);

CTC_CLI(ctc_cli_usw_nh_cfg_global_param,
        ctc_cli_usw_nh_cfg_global_param_cmd,
        "nexthop (ipmc-logic-replication) value VALUE (lchip LCHIP|)",
        CTC_CLI_NH_M_STR,
        "IPMC-logic_replication",
        "Value",
        "The Value of paramer configuration",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
	uint8  index = 0 ;
    uint8 value;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if (0 == sal_memcmp(argv[0], "ipmc-logic-replication", 4))
    {
        CTC_CLI_GET_UINT8("Value", value, argv[1]);
        ret = sys_usw_nh_set_ipmc_logic_replication(lchip, value);
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_nh_dump_mpls_tunnel,
        ctc_cli_usw_nh_dump_mpls_tunnel_cmd,
        "show nexthop mpls-tunnel TUNNEL_ID (detail|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_MPLS_TUNNEL,
        CTC_CLI_NH_MPLS_TUNNEL_ID,
        "Display detail information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint16 tunnel_id = 0;
	uint8  index = 0 ;
    bool detail = FALSE;

    CTC_CLI_GET_UINT16("Value", tunnel_id, argv[0]);

	index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("detail");
    if (0xFF != index)
    {
         detail = TRUE;
    }

	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_nh_dump_mpls_tunnel(lchip, tunnel_id, detail);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

extern int32
sys_usw_nh_set_reflective_brg_en(uint8 lchip, uint8 enable);

CTC_CLI(ctc_cli_usw_nh_set_reflective_bridge,
        ctc_cli_usw_nh_set_reflective_bridge_cmd,
        "nexthop mcast-reflective-bridge (enable|disable)(lchip LCHIP|)",
        CTC_CLI_NH_M_STR,
        "Mcast packet will support reflective bridge",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
	uint8  index = 0 ;
    uint8 enable = FALSE;

    if (0 == sal_strncmp("enable", argv[0], sizeof("enable")))
    {
        enable = TRUE;
    }

	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_nh_set_reflective_brg_en(lchip, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_nh_show_resource_usage,
        ctc_cli_usw_nh_show_resource_usage_cmd,
        "show nexthop status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "Nexthop's resource used status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
	uint8  index = 0 ;

	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_nh_dump_resource_usage(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_usw_nh_dump_arp_id,
        ctc_cli_usw_nh_dump_arp_id_cmd,
        "show nexthop arp-id ARP_ID (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "ARP ID",
        "ARP ID Value",
        "Display detail information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint16 arp_id = 0;
    uint8  index = 0 ;
    bool detail = FALSE;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT16("Value", arp_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("detail");
    if (0xFF != index)
    {
         detail = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_nh_dump_arp(lchip, arp_id, detail);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_NH_WLAN_TUNNEL_TYPE "type ( tunnel-v4  ipsa A.B.C.D  ipda A.B.C.D| tunnel-v6 ipsa X:X::X:X ipda X:X::X:X) l4-src-port L4SRCPORT l4-dest-port L4DESTPORT"
#define CTC_CLI_NH_WLAN_TUNNEL_FLAG "{ radio-mac MAC | encrypt ID |roam-status (pop|poa)|multicast-en (bssid-bitmap VALUE|)|split-mac {dot11-sub-type VALUE | map-cos (cos-domain DOMAIN |)} | is-route-packet | mac-da MACDA |vlan-id VLANID| wds bssid-da BSSID| dscp DSCP dscp-select SEL_MODE (dscp-domain DOMAIN |)|"\
   "ttl TTL (map-ttl|) | frag-en mtu-size MTU | flow-label-mode MODE (flow-label-value VALUE|) | logic-dest-port PORT | logic-port-check|"\
   "stats-id STATSID | data-keepalive | untag |(copy-dont-frag|set-dont-frag)| ecn-select ECN_SEL | }"

#define CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_STR    CTC_CLI_NH_WLAN_TUNNEL_TYPE CTC_CLI_NH_WLAN_TUNNEL_FLAG

#define CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_DESC   \
    "Tunnel type",                                                  \
    "Tunnel over ipv4",                                             \
    "Ip sa",                                                        \
    CTC_CLI_IPV4_FORMAT,                                            \
    "Ip da",                                                        \
    CTC_CLI_IPV4_FORMAT,                                            \
    "Tunnel over ipv6",                                             \
    "Ip sa",                                                        \
    CTC_CLI_IPV6_FORMAT,                                            \
    "Ip da",                                                        \
    CTC_CLI_IPV6_FORMAT,                                            \
    "Layer4 src port",                                              \
    "Value <0-65535>",                                              \
    "Layer4 dest port",                                             \
    "Value <0-65535>",                                              \
    "Radio mac enable",                                             \
    "Radio mac value",                                              \
    "Encrypt enable",                                               \
    "Encrypt id ",                                                  \
    "Roam status",                                                  \
    "Pop role",                                                     \
    "Poa role",                                                     \
    "Multicast enbale",                                             \
    "Bssid bitmap for wtp replication",                             \
    "Bssid bitmap value, max 16 bits",                              \
    "Translate 802.3 to 802.11",                                    \
    "Dot11 sub type",                                               \
    "Dot11 sub type value: <0-15>",                                 \
    "Map cos",                                                      \
    "Cos domain",                                                   \
    "Domain ID",                                                    \
    "Is route packet",                                              \
    "Inner mac da",                                                 \
    "Inner mac da value",                                           \
    "Vlan id",                                                      \
    "Vlan id value",                                                \
    "WDS enable",                                                   \
    "Bssid mac da",                                                 \
    "Vssid mac da value",                                           \
    CTC_CLI_NH_DSNH_TUNNEL_DSCP_DESC,                               \
    CTC_CLI_NH_DSNH_TUNNEL_DSCP_VAL,                                \
    "Dscp select mode",                                       \
    "Dscp select mode, refer to ctc_nh_dscp_select_mode_t", \
    "Dscp domain",                                                  \
    "Domain ID",                                                    \
    "TTL",                                                          \
    "TTL value: <0-0xFF>",                                          \
    "TTL mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "Frag enable",                                                  \
    "Mtu size",                                                     \
    "Mtu size value <0-0x3fff>",                                    \
    "IPv6 flow label mode (default is do not set flow label value)",\
    "IPv6 flow label mode:0-Do not set flow label valu, 1-Use (user-define flow label + header hash)to outer header , 2- Use user-define flow label to outer header", \
    "IPv6 flow label value (default is 0)",                         \
    "IPv6 flow label value <0-0xFFFFF>",                            \
    "Logic destination port assigned for this tunnel",              \
    "Logic port value : <1-0x3FFF>",                                \
    "Logic port check",                                             \
    CTC_CLI_STATS_ID_DESC,                                          \
    CTC_CLI_STATS_ID_VAL,                                           \
    "Data keepalive packet",                                        \
    "Untag packet",                                                 \
    "copy paload dont-frag to outer ip header",                     \
    "set dont-frag in outer ip header",                             \
    "Ecn select mode", \
    "Ecn mode:0-use ecn 0, 1-user-define ECN, 2-Use ECN value from ECN map, 3-Copy packet ECN"

STATIC int32
_ctc_nexthop_cli_parse_wlan_tunnel_nexthop(char** argv, uint16 argc,
                                         ctc_nh_wlan_tunnel_param_t* p_nhinfo)
{
    uint8 tmp_argi;
    int ret = CLI_SUCCESS;
    mac_addr_t mac;
    ipv6_addr_t ipv6_address;


    /*tunnel v4 ip header info*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("tunnel-v4");
    if (0xFF != tmp_argi)
    {
        /*IPSA IPDA*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", p_nhinfo->ip_sa.ipv4, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipda", p_nhinfo->ip_da.ipv4, argv[tmp_argi + 1]);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("tunnel-v6");
    if (0xFF != tmp_argi)    /*tunnel v6 ip header info*/
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_IPV6);
        /*IPSA IPDA*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[tmp_argi + 1]);
            /* adjust endian */
            p_nhinfo->ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
            p_nhinfo->ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
            p_nhinfo->ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
            p_nhinfo->ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[tmp_argi + 1]);
            /* adjust endian */
            p_nhinfo->ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
            p_nhinfo->ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
            p_nhinfo->ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
            p_nhinfo->ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (0xFF != tmp_argi)
    {
            CTC_CLI_GET_UINT16("l4 src port", p_nhinfo->l4_src_port, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-dest-port");
    if (0xFF != tmp_argi)
    {
            CTC_CLI_GET_UINT16("l4 dest port", p_nhinfo->l4_dst_port, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("radio-mac");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio mac", mac, argv[tmp_argi + 1]);
        sal_memcpy(p_nhinfo->radio_mac, mac, sizeof(mac_addr_t));
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_RADIO_MAC_EN);
    }

    /*encrypt*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("encrypt");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_ENCRYPT_EN;
        CTC_CLI_GET_UINT8("encrypt id", p_nhinfo->encrypt_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("roam-status");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_POP_POA_ROAM;

        if (!sal_memcmp(argv[tmp_argi + 1], "pop", sizeof("pop")))
        {
            p_nhinfo->is_pop = 1;
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("multicast-en");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_MC_EN;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("bssid-bitmap");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("bssid bitmap", p_nhinfo->bssid_bitmap, argv[tmp_argi + 1]);
        }
    }

    /*split-mac*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("split-mac");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN;

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dot11-sub-type");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT8("dot11 sub type", p_nhinfo->dot11_sub_type, argv[tmp_argi + 1]);
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("map-cos");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS;
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("cos-domain");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT8("cos domain", p_nhinfo->cos_domain, argv[tmp_argi + 1]);
            }
        }
    }

    /*is-route-packet*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-route-packet");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-da");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac da", mac, argv[tmp_argi + 1]);
            sal_memcpy(p_nhinfo->mac_da, mac, sizeof(mac_addr_t));
        }
    }

    /*macda*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac da", mac, argv[tmp_argi + 1]);
        sal_memcpy(p_nhinfo->mac_da, mac, sizeof(mac_addr_t));
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("vlan id", p_nhinfo->vlan_id, argv[tmp_argi + 1]);
    }

    /*wds*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("wds");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->flag |= CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("bssid-da");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("bssid mac da", mac, argv[tmp_argi + 1]);
            sal_memcpy(p_nhinfo->mac_da, mac, sizeof(mac_addr_t));
        }
    }

    /*DSCP*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("dscp", p_nhinfo->dscp_or_tos, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp-select");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("dscp-select", p_nhinfo->dscp_select, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ecn-select");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("ecn-select", p_nhinfo->ecn_select, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp-domain");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("dscp-domain", p_nhinfo->dscp_domain, argv[tmp_argi + 1]);
    }

    /*TTL*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("ttl", p_nhinfo->ttl, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("map-ttl");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_MAP_TTL);
    }

    /*MTU*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("frag-en");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_FRAGMENT_EN);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-size");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("mtu-size", p_nhinfo->mtu_size, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-label-mode");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("flow label mode", p_nhinfo->flow_label_mode, argv[tmp_argi + 1]);
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-label-value");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("flow label value", p_nhinfo->flow_label, argv[tmp_argi + 1]);
        }
        else
        {
            p_nhinfo->flow_label = 0;
        }
    }
    else
    {
        p_nhinfo->flow_label_mode = CTC_NH_FLOW_LABEL_NONE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("logic-dest-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("logic dest port", p_nhinfo->logic_port, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("logic-port-check");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_LOGIC_PORT_CHECK);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", p_nhinfo->stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("data-keepalive");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_KEEPALIVE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("untag");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_UNTAG_EN);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("copy-dont-frag");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_COPY_DONT_FRAG);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("set-dont-frag");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_WLAN_TUNNEL_FLAG_DONT_FRAG);
    }

    return ret;
}

CTC_CLI(ctc_cli_add_wlan_tunnel,
        ctc_cli_add_wlan_tunnel_cmd,
        "nexthop add wlan-tunnel NHID (dsnh-offset OFFSET|) (unrov |fwd ("CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_STR "))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "Wlan Tunnel nexthop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        "Unresolved nexthop",
        "Forward Nexthop",
        CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_DESC)
{

    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0, dsnh_offset = 0, tmp_argi = 0;
    ctc_nh_wlan_tunnel_param_t nh_param = {0};

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
         /*CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_UNROV);*/
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        if (_ctc_nexthop_cli_parse_wlan_tunnel_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nh_param))
        {
            return CLI_ERROR;
        }
    }

    nh_param.dsnh_offset = dsnh_offset;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_wlan_tunnel(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_wlan_tunnel(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_update_wlan_tunnel,
        ctc_cli_update_wlan_tunnel_cmd,
        "nexthop update wlan-tunnel NHID ( fwd-attr|unrov2fwd) ("CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        "Wlan Tunnel nexthop",
        CTC_CLI_NH_ID_STR,
        "Update forward nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_WLAN_TUNNEL_NH_PARAM_DESC)
{

    int32 ret  = CLI_SUCCESS;

    uint32 nhid;
    uint8 tmp_argi;
    ctc_nh_wlan_tunnel_param_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(ctc_nh_wlan_tunnel_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    if (_ctc_nexthop_cli_parse_wlan_tunnel_nexthop(&(argv[1]), (argc - 1), &nhinfo))
    {
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov2fwd");
    if (0xFF != tmp_argi)
    {

        nhinfo.upd_type = CTC_NH_UPD_UNRSV_TO_FWD;
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_update_wlan_tunnel(g_api_lchip, nhid, &nhinfo);
        }
        else
        {
            ret = ctc_nh_update_wlan_tunnel(nhid, &nhinfo);
        }

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd-attr");
    if (0xFF != tmp_argi)
    {

        nhinfo.upd_type = CTC_NH_UPD_FWD_ATTR;
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_update_wlan_tunnel(g_api_lchip, nhid, &nhinfo);
        }
        else
        {
            ret = ctc_nh_update_wlan_tunnel(nhid, &nhinfo);
        }

    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_update_wlan_tunnel_to_unrov,
        ctc_cli_update_wlan_tunnel_to_unrov_cmd,
        "nexthop update wlan-tunnel NHID fwd2unrov",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        "Wlan Tunnel nexthop",
        CTC_CLI_NH_ID_STR,
        "Forward nexthop to unresolved nexthop")
{
    int32 ret = 0;

    uint32 nhid;

    ctc_nh_wlan_tunnel_param_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(ctc_nh_wlan_tunnel_param_t));

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    nhinfo.upd_type = CTC_NH_UPD_FWD_TO_UNRSV;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_wlan_tunnel(g_api_lchip, nhid, &nhinfo);
    }
    else
    {
        ret = ctc_nh_update_wlan_tunnel(nhid, &nhinfo);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_remove_wlan_tunnel,
        ctc_cli_remove_wlan_tunnel_cmd,
        "nexthop remove wlan-tunnel NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        "Wlan Tunnel nexthop",
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_wlan_tunnel(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_wlan_tunnel(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

int32
ctc_usw_nexthop_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_nexthop_by_nhid_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_nexthop_all_by_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_nh_dump_mpls_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_resource_usage_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_nh_dump_arp_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_add_wlan_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_update_wlan_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_update_wlan_tunnel_to_unrov_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_remove_wlan_tunnel_cmd);

    /*GB CLIs*/
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_nh_cfg_global_param_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_nh_set_reflective_bridge_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_nexthop_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_nexthop_by_nhid_type_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_nexthop_all_by_type_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_nh_dump_mpls_tunnel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_nh_show_resource_usage_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_nh_dump_arp_id_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_add_wlan_tunnel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_update_wlan_tunnel_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_update_wlan_tunnel_to_unrov_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_remove_wlan_tunnel_cmd);

    /*GB CLIs*/
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_nh_cfg_global_param_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_nh_set_reflective_bridge_cmd);

    return CLI_SUCCESS;
}

