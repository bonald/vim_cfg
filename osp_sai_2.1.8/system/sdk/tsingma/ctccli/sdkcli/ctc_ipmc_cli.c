#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_ipuc_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"

#define CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR \
    "(sub-if (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) vlanid VLAN_ID | \
    vlan-if vlanid VLAN_ID (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) (vlan-port|) | \
    phy-if (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) |nexthop-id NHID (assign-port GPORT_ID|)|gport GPORT_ID)"
#define CTC_CLI_IPMC_L3IF_ALL_DESC \
    "Sub interface", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    "VLAN interface", CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    "Vlan port", "Physical interface/remote chip id", \
    CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    "Nexthop ID", CTC_CLI_NH_ID_STR, "Specify assign dest gport", CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC
#define CTC_CLI_IPMC_NH_ALL_STR  "nexthop-id NHID"
#define CTC_CLI_IPMC_NH_ALL_DESC  "Add member by nexthop, can be used in ipmc and ip based l2mc", CTC_CLI_NH_ID_STR

static ctc_ipmc_group_info_t ipmc_group_info;

CTC_CLI(ctc_cli_ipmc_rpf_debug_on,
        ctc_cli_ipmc_rpf_debug_on_cmd,
        "debug rpf (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "RPF",
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
        typeenum = RPF_CTC;
    }
    else
    {
        typeenum = RPF_SYS;
    }

    ctc_debug_set_flag("ip", "rpf", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_rpf_debug_off,
        ctc_cli_ipmc_rpf_debug_off_cmd,
        "no debug rpf (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "RPF",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = RPF_CTC;
    }
    else
    {
        typeenum = RPF_SYS;
    }

    ctc_debug_set_flag("ip", "rpf", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_mcast_debug_on,
        ctc_cli_ipmc_mcast_debug_on_cmd,
        "debug mcast (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MCAST_M_STR,
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
        typeenum = MCAST_CTC;
    }
    else
    {
        typeenum = MCAST_SYS;
    }

    ctc_debug_set_flag("mcast", "mcast", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_mcast_debug_off,
        ctc_cli_ipmc_mcast_debug_off_cmd,
        "no debug mcast (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MCAST_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MCAST_CTC;
    }
    else
    {
        typeenum = MCAST_SYS;
    }

    ctc_debug_set_flag("mcast", "mcast", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_debug_on,
        ctc_cli_ipmc_debug_on_cmd,
        "debug ipmc (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MCAST_M_STR,
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
        typeenum = IPMC_CTC;
    }
    else
    {
        typeenum = IPMC_SYS;
    }

    ctc_debug_set_flag("ip", "ipmc", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_debug_off,
        ctc_cli_ipmc_debug_off_cmd,
        "no debug ipmc (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Ipmc",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPMC_CTC;
    }
    else
    {
        typeenum = IPMC_SYS;
    }

    ctc_debug_set_flag("ip", "ipmc", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv4_group,
        ctc_cli_ipmc_add_ipv4_group_cmd,
        "ipmc add (group GROUPID|) group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (share-group|nhid NHID|) ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu | copy-tocpu | drop | bidipim RP_ID (bidipim-fail-to-cpu |) } | )",
        CTC_CLI_IPMC_M_STR,
        "Add",
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        "VRF id or FID",
        "VRF id or FID value",
        "Share mcast group",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "TTL check enable",
        "RPF check enable",
        "If RPF check fail, sent to cpu",
        "Statistic supported",
        "0~0xFFFFFFFF",
        "Do IP based L2 mcast",
        "Ptp-entry",
        "Redirect to cpu",
        "Copy to cpu",
        "Drop",
        "Bidipim",
        "Rp id",
        "If bidipim check fail, sent to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
    uint8  param_idx = 0;
//    ctc_ipmc_group_info_t ipmc_group_info;

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("groupId", ipmc_group_info.group_id, argv[index + 1]);
        param_idx += 2;
    }

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[param_idx]);
    param_idx++;

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[param_idx], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;
    param_idx++;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[param_idx]);
    param_idx++;

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[param_idx], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;
    param_idx++;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[param_idx], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;
    param_idx++;

    index = CTC_CLI_GET_ARGC_INDEX("share-group");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_SHARE_GROUP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_WITH_NEXTHOP;
        CTC_CLI_GET_UINT32("nhid", ipmc_group_info.nhid, argv[index + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ttl-check", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("rpf-check", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_CHECK;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("rpf-fail-to-cpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
        if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_STATS;
        CTC_CLI_GET_UINT32("stats-id", ipmc_group_info.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-based-l2mc", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ptp-entry", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("redirect-tocpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("copy-tocpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_COPY_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bidipim");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_BIDPIM_CHECK;
        CTC_CLI_GET_UINT8_RANGE("rp-id", ipmc_group_info.rp_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bidipim-fail-to-cpu");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_BIDIPIM_FAIL_BRIDGE_AND_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_group(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv6_group,
        ctc_cli_ipmc_add_ipv6_group_cmd,
        "ipmc ipv6 add (group GROUPID|) group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (share-group|nhid NHID|) ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu | copy-tocpu | drop | bidipim RP_ID (bidipim-fail-to-cpu |) } | )",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Add",
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Share mcast group",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "TTL check enable",
        "RPF check enable",
        "If RPF check fail, sent to cpu",
        "Statistic supported",
        "0~0xFFFFFFFF",
        "Do IP based L2 mcast",
        "Ptp-entry",
        "Redirect to cpu",
        "Copy to cpu",
        "Drop",
        "Bidipim",
        "Rp id",
        "If bidipim check fail, sent to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint16 vrf_id = 16;
    uint32 masklen = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint8  param_idx = 0;
//    ctc_ipmc_group_info_t ipmc_group_info;

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 0;

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("groupId", ipmc_group_info.group_id, argv[index + 1]);
        param_idx += 2;
    }
    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[param_idx]);
    param_idx++;

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[param_idx], 0, CTC_MAX_UINT8_VALUE);
    param_idx++;
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[param_idx]);
    param_idx++;

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[param_idx], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;
    param_idx++;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[param_idx], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;
    param_idx++;

    index = CTC_CLI_GET_ARGC_INDEX("share-group");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_SHARE_GROUP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_WITH_NEXTHOP;
        CTC_CLI_GET_UINT32("nhid", ipmc_group_info.nhid, argv[index + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ttl-check", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("rpf-check", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_CHECK;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("rpf-fail-to-cpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
        if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_STATS;
        CTC_CLI_GET_UINT32("stats-id", ipmc_group_info.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-based-l2mc", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ptp-entry", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("redirect-tocpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("copy-tocpu", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_COPY_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bidipim");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_BIDPIM_CHECK;
        CTC_CLI_GET_UINT8_RANGE("rp-id", ipmc_group_info.rp_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bidipim-fail-to-cpu");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_BIDIPIM_FAIL_BRIDGE_AND_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_group(&ipmc_group_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_remove_ipv4_group,
        ctc_cli_ipmc_remove_ipv4_group_cmd,
        "ipmc remove group group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID {ip-based-l2mc | keep-empty-entry |}",
        CTC_CLI_IPMC_M_STR,
        "Remove",
        CTC_CLI_IPMC_GROUP_DESC,
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        "Remove all member from group")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("keep-empty-entry");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_KEEP_EMPTY_ENTRY;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_remove_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_remove_group(&ipmc_group_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s :%d \n\r", ctc_get_error_desc(ret), ret);
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_remove_ipv6_group,
        ctc_cli_ipmc_remove_ipv6_group_cmd,
        "ipmc ipv6 remove group group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID {ip-based-l2mc | keep-empty-entry |}",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Remove",
        CTC_CLI_IPMC_GROUP_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        "Remove all member from group")
{
    uint16 vrf_id = 0;
    int32 ret = CLI_SUCCESS;
    uint32 masklen = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("keep-empty-entry");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_KEEP_EMPTY_ENTRY;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_remove_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_remove_group(&ipmc_group_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv4_member,
        ctc_cli_ipmc_add_ipv4_member_cmd,
        "ipmc add member group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID " \
        "(ip-based-l2mc |) ("CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR"(remote-chip|) | re-route-member){leaf-check-en |logic-port LOGIC_PORT|mtu-no-chk|}",
        CTC_CLI_IPMC_M_STR,
        "Add",
        CTC_CLI_IPMC_MEMBER_DESC,
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "IP based l2 mcast",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Indicate the member is remote chip entry",
        "Add a member port for re-route",
        "Leaf check enable",
        "Logic dest port",
        "Logic dest port value",
        "Disable MTU check")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;

 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;
    ipmc_group_info.member_number   = 1;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
    index = CTC_CLI_GET_SPECIFIC_INDEX("phy-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("re-route-member");
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].re_route = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("sub-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_SUB_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        CTC_CLI_GET_UINT32_RANGE("portId", ipmc_group_info.ipmc_member[0].global_port, argv[index + 5 + 1], 0, CTC_MAX_UINT32_VALUE);

    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("vlanid", 5);
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanid", ipmc_group_info.ipmc_member[0].vlan_id, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-port", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].vlan_port = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nexthop-id", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
        ipmc_group_info.ipmc_member[0].is_nh = TRUE;
        CTC_CLI_GET_UINT32("NexthopID", ipmc_group_info.ipmc_member[0].nh_id, argv[index + 5 + 1]);

        index = CTC_CLI_GET_SPECIFIC_INDEX("assign-port", 5);
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("portId", ipmc_group_info.ipmc_member[0].global_port, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
            CTC_SET_FLAG(ipmc_group_info.ipmc_member[0].flag, CTC_IPMC_MEMBER_FLAG_ASSIGN_PORT);
        }
    }


    index = CTC_CLI_GET_SPECIFIC_INDEX("remote-chip", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].remote_chip   = TRUE;
    }
    ipmc_group_info.ipmc_member[0].port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(ipmc_group_info.ipmc_member[0].port_bmp);
    if (ipmc_group_info.ipmc_member[0].port_bmp[0] || ipmc_group_info.ipmc_member[0].port_bmp[1] ||
        ipmc_group_info.ipmc_member[0].port_bmp[8] || ipmc_group_info.ipmc_member[0].port_bmp[9])
    {
        ipmc_group_info.ipmc_member[0].port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", ipmc_group_info.ipmc_member[0].gchip_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("leaf-check-en");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ipmc_group_info.ipmc_member[0].flag, CTC_IPMC_MEMBER_FLAG_LEAF_CHECK_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT16("logic port", ipmc_group_info.ipmc_member[0].logic_dest_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mtu-no-chk");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ipmc_group_info.ipmc_member[0].flag, CTC_IPMC_MEMBER_FLAG_MTU_NO_CHECK);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_member(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_member(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv6_member,
        ctc_cli_ipmc_add_ipv6_member_cmd,
        "ipmc ipv6 add member group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID " \
        "(ip-based-l2mc|) ("CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR"(remote-chip|) | re-route-member){leaf-check-en |logic-port LOGIC_PORT|mtu-no-chk|}",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Add",
        CTC_CLI_IPMC_MEMBER_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_VRFID_DESC,
        "IP based l2 mcast",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Indicate the member is remote chip entry",
        "Add a member port for re-route",
         "Leaf check enable",
        "Logic dest port",
        "Logic dest port value",
        "Disable MTU check")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};

 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 1;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
    index = CTC_CLI_GET_SPECIFIC_INDEX("phy-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("sub-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_SUB_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("re-route-member");
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].re_route = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        CTC_CLI_GET_UINT16_RANGE("portId", ipmc_group_info.ipmc_member[0].global_port, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);

    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlanid", 5);
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanId", ipmc_group_info.ipmc_member[0].vlan_id, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-port", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].vlan_port = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nexthop-id", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
        ipmc_group_info.ipmc_member[0].is_nh = TRUE;
        CTC_CLI_GET_UINT32("NexthopID", ipmc_group_info.ipmc_member[0].nh_id, argv[index + 5 + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("remote-chip", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].remote_chip   = TRUE;

    }
    ipmc_group_info.ipmc_member[0].port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(ipmc_group_info.ipmc_member[0].port_bmp);
    if (ipmc_group_info.ipmc_member[0].port_bmp[0] || ipmc_group_info.ipmc_member[0].port_bmp[1] ||
        ipmc_group_info.ipmc_member[0].port_bmp[8] || ipmc_group_info.ipmc_member[0].port_bmp[9])
    {
        ipmc_group_info.ipmc_member[0].port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", ipmc_group_info.ipmc_member[0].gchip_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("leaf-check-en");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ipmc_group_info.ipmc_member[0].flag, CTC_IPMC_MEMBER_FLAG_LEAF_CHECK_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT16("logic port", ipmc_group_info.ipmc_member[0].logic_dest_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mtu-no-chk");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(ipmc_group_info.ipmc_member[0].flag, CTC_IPMC_MEMBER_FLAG_MTU_NO_CHECK);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_member(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_member(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_remove_ipv4_member,
        ctc_cli_ipmc_remove_ipv4_member_cmd,
        "ipmc remove member group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID" \
        "(ip-based-l2mc|) ("CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR"(remote-chip|) | re-route-member)",
        CTC_CLI_IPMC_M_STR,
        "Remove",
        CTC_CLI_IPMC_MEMBER_DESC,
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "IP based l2 mcast",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Indicate the member is remote chip entry",
        "Add a member port for re-route")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint8 index = 0;
    uint16 vrf_id = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;
    ipmc_group_info.member_number   = 1;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
    index = CTC_CLI_GET_SPECIFIC_INDEX("phy-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("sub-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_SUB_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("re-route-member");
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].re_route = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        CTC_CLI_GET_UINT16_RANGE("portId", ipmc_group_info.ipmc_member[0].global_port, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlanid", 5);
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanid", ipmc_group_info.ipmc_member[0].vlan_id, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-port", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].vlan_port = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nexthop-id", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
        ipmc_group_info.ipmc_member[0].is_nh = TRUE;
        CTC_CLI_GET_UINT32("NexthopID", ipmc_group_info.ipmc_member[0].nh_id, argv[index + 5 + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("remote-chip", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].remote_chip   = TRUE;

    }
    ipmc_group_info.ipmc_member[0].port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(ipmc_group_info.ipmc_member[0].port_bmp);
    if (ipmc_group_info.ipmc_member[0].port_bmp[0] || ipmc_group_info.ipmc_member[0].port_bmp[1] ||
        ipmc_group_info.ipmc_member[0].port_bmp[8] || ipmc_group_info.ipmc_member[0].port_bmp[9])
    {
        ipmc_group_info.ipmc_member[0].port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", ipmc_group_info.ipmc_member[0].gchip_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_remove_member(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_remove_member(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_remove_ipv6_member,
        ctc_cli_ipmc_remove_ipv6_member_cmd,
        "ipmc ipv6 remove member group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID "\
        "(ip-based-l2mc|) ("CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR"(remote-chip|) | re-route-member)",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Remove",
        CTC_CLI_IPMC_MEMBER_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_VRFID_DESC,
        "IP based l2 mcast",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Indicate the member is remote chip entry",
        "Add a member port for re-route")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};

 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 1;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    index = CTC_CLI_GET_SPECIFIC_INDEX("phy-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("sub-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_SUB_IF;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-if", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("re-route-member");
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].re_route = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        CTC_CLI_GET_UINT16_RANGE("portId", ipmc_group_info.ipmc_member[0].global_port, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);

    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlanid", 5);
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanId", ipmc_group_info.ipmc_member[0].vlan_id, argv[index + 5 + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-port", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].vlan_port = TRUE;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nexthop-id", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.member_number   = 1;
        ipmc_group_info.ipmc_member[0].l3_if_type = MAX_L3IF_TYPE_NUM;
        ipmc_group_info.ipmc_member[0].is_nh = TRUE;
        CTC_CLI_GET_UINT32("NexthopID", ipmc_group_info.ipmc_member[0].nh_id, argv[index + 5 + 1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("remote-chip", 5);
    if (index != 0xFF)
    {
        ipmc_group_info.ipmc_member[0].remote_chip   = TRUE;

    }

    ipmc_group_info.ipmc_member[0].port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(ipmc_group_info.ipmc_member[0].port_bmp);
    if (ipmc_group_info.ipmc_member[0].port_bmp[0] || ipmc_group_info.ipmc_member[0].port_bmp[1] ||
        ipmc_group_info.ipmc_member[0].port_bmp[8] || ipmc_group_info.ipmc_member[0].port_bmp[9])
    {
        ipmc_group_info.ipmc_member[0].port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", ipmc_group_info.ipmc_member[0].gchip_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_remove_member(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_remove_member(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

static ctc_ipmc_group_info_t g_ipmc_rpf_profile;

CTC_CLI(ctc_cli_ipmc_rpf_profile,
        ctc_cli_ipmc_rpf_profile_cmd,
        "ipmc rpf profile {rpf-if0 IFID0|rpf-if1 IFID1|rpf-if2 IFID2|rpf-if3 IFID3|rpf-if4 IFID4|rpf-if5 IFID5|rpf-if6 IFID6|rpf-if7 IFID7|rpf-if8 IFID8|rpf-if9 IFID9|rpf-if10 IFID10|rpf-if11 IFID11|rpf-if12 IFID12|rpf-if13 IFID13|rpf-if14 IFID14|rpf-if15 IFID15}",
        CTC_CLI_IPMC_M_STR,
        "Rpf",
        "Profile",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set")
{
    int8 idx = 0;
    uint8 index = 0;
    char  prefix_buf_str[16] = "";

    sal_memset(&g_ipmc_rpf_profile,0,sizeof(ctc_ipmc_group_info_t));

    for(idx = 0; idx < CTC_IP_MAX_RPF_IF; idx++)
    {
        sal_sprintf(prefix_buf_str, "%s%d","rpf-if",idx);
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE(prefix_buf_str);
        if (index != 0xFF)
        {
            g_ipmc_rpf_profile.rpf_intf_valid[idx] = 1;
            CTC_CLI_GET_UINT16_RANGE("rpf-if", g_ipmc_rpf_profile.rpf_intf[idx], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_update_ipv4_rpf,
        ctc_cli_ipmc_update_ipv4_rpf_cmd,
        "ipmc update rpf group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (rpf-profile | {rpf-if0 IFID0|rpf-if1 IFID1|rpf-if2 IFID2|rpf-if3 IFID3} |) {more-rpf-to-cpu |}",
        CTC_CLI_IPMC_M_STR,
        "Update",
        "Reverse path forward",
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Use RPF-profile",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        "More than 4 rpf interfaces, send to CPU for checking")
{
    uint8 index = 0;
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
     /*ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version = CTC_IP_VER_4;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if0");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[0] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[0], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if1");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[1] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[1], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if2");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[2] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[2], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if3");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[3] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[3], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-profile");
    if (index != 0xFF)
    {
        sal_memcpy(ipmc_group_info.rpf_intf,g_ipmc_rpf_profile.rpf_intf,sizeof(uint16) * CTC_IP_MAX_RPF_IF);
        sal_memcpy(ipmc_group_info.rpf_intf_valid,g_ipmc_rpf_profile.rpf_intf_valid,sizeof(uint8) * CTC_IP_MAX_RPF_IF);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_update_rpf(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_update_rpf(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_update_ipv6_rpf,
        ctc_cli_ipmc_update_ipv6_rpf_cmd,
        "ipmc ipv6 update rpf group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (rpf-profile | {rpf-if0 IFID0|rpf-if1 IFID1|rpf-if2 IFID2|rpf-if3 IFID3} |) {more-rpf-to-cpu |}",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Update",
        "Reverse path forward",
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Use RPF-profile",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        CTC_CLI_L3IF_ID_DESC,
        "L3if value, means Port if rpfCheckAgainstPort set",
        "More than 4 rpf interfaces, send to CPU for checking")
{
    uint8 index = 0;
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/
    uint32 ipv6_address[4] = {0, 0, 0, 0};

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version = CTC_IP_VER_6;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if0");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[0] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[0], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if1");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[1] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[1], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if2");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[2] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[2], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-if3");
    if (index != 0xFF)
    {
        ipmc_group_info.rpf_intf_valid[3] = 1;
        CTC_CLI_GET_UINT16_RANGE("rpf-if", ipmc_group_info.rpf_intf[3], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-profile");
    if (index != 0xFF)
    {
        sal_memcpy(ipmc_group_info.rpf_intf,g_ipmc_rpf_profile.rpf_intf,sizeof(uint16) * CTC_IP_MAX_RPF_IF);
        sal_memcpy(ipmc_group_info.rpf_intf_valid,g_ipmc_rpf_profile.rpf_intf_valid,sizeof(uint8) * CTC_IP_MAX_RPF_IF);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_update_rpf(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_update_rpf(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv4_default_entry,
        ctc_cli_ipmc_add_ipv4_default_entry_cmd,
        "ipmc add default vrf VRFID (to-cpu | drop | fallback-bridge)",
        CTC_CLI_IPMC_M_STR,
        "Add",
        "Default action",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Send the packet that hit the default entry to cpu",
        "Drop the packet that hit the default entry",
        "Do RPF check and fallback bridge to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    uint16 vrf_id = 0;

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_DROP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fallback-bridge");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_CHECK;
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_group(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_ipv6_default_entry,
        ctc_cli_ipmc_add_ipv6_default_entry_cmd,
        "ipmc ipv6 add default vrf VRFID (to-cpu | drop | fallback-bridge)",
        CTC_CLI_IPMC_M_STR,
        "IPV6",
        "Add",
        "Default action",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Send the packet that hit the default entry to cpu and do bridge",
        "Drop the packet that hit the default entry",
        "Do RPF check and fallback bridge to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    uint16 vrf_id = 0;

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version = CTC_IP_VER_6;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_DROP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fallback-bridge");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_CHECK;
        ipmc_group_info.flag |= CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_add_group(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_add_group(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_set_ipv4_force_route,
        ctc_cli_ipmc_set_ipv4_force_route_cmd,
        "ipmc force-route {ip-addr0 A.B.C.D MASK_LEN | ip-addr1 A.B.C.D MASK_LEN} {force-bridge (disable|) |force-unicast (disable|)}",
        CTC_CLI_IPMC_M_STR,
        "Force route",
        "IPv4 address0",
        CTC_CLI_IPV4_FORMAT,
        "The length of ipv4 address mask",
        "IPv4 address1",
        CTC_CLI_IPV4_FORMAT,
        "The length of ipv4 address mask",
        "Force bridge",
        "Disable",
        "Force unicast",
        "Disable")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint8 index = 0xFF;
    ctc_ipmc_force_route_t p_data;

    sal_memset(&p_data, 0, sizeof(ctc_ipmc_force_route_t));
    p_data.ip_version = CTC_IP_VER_4;

    index = CTC_CLI_GET_ARGC_INDEX("ip-addr0");
    if (index != 0xFF)
    {
        p_data.ipaddr0_valid = 1;
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 ip address0", p_data.ip_addr0.ipv4, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[index+2], 0, CTC_MAX_UINT8_VALUE);
        p_data.addr0_mask = masklen;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-addr1");
    if (index != 0xFF)
    {
        p_data.ipaddr1_valid = 1;
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 ip address1", p_data.ip_addr1.ipv4, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[index+2], 0, CTC_MAX_UINT8_VALUE);
        p_data.addr1_mask = masklen;
    }

    index = CTC_CLI_GET_ARGC_INDEX("force-bridge");
    if (index != 0xFF)
    {
        if ((argc > index + 1) && CTC_CLI_STR_EQUAL_ENHANCE("disable", index+1))
        {
            p_data.force_bridge_en = 0;
        }
        else
        {
            p_data.force_bridge_en = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("force-unicast");
    if (index != 0xFF)
    {
        if ((argc > index + 1) && CTC_CLI_STR_EQUAL_ENHANCE("disable", index+1))
        {
            p_data.force_ucast_en = 0;
        }
        else
        {
            p_data.force_ucast_en = 1;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_set_mcast_force_route(g_api_lchip, &p_data);
    }
    else
    {
        ret = ctc_ipmc_set_mcast_force_route(&p_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_set_ipv6_force_route,
        ctc_cli_ipmc_set_ipv6_force_route_cmd,
        "ipmc ipv6 force-route {ip-addr0 X:X::X:X MASK_LEN | ip-addr1 X:X::X:X MASK_LEN} {force-bridge (disable|) |force-unicast (disable|)}",
        CTC_CLI_IPMC_M_STR,
        "Ipv6",
        "Force route",
        "IPv6 address0",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask",
        "IPv6 address1",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask",
        "Force bridge",
        "Disable",
        "Force unicast",
        "Disable")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint8 index = 0xFF;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    ctc_ipmc_force_route_t p_data;

    sal_memset(&p_data, 0, sizeof(ctc_ipmc_force_route_t));

    p_data.ip_version = CTC_IP_VER_6;

    index = CTC_CLI_GET_ARGC_INDEX("ip-addr0");
    if (index != 0xFF)
    {
        p_data.ipaddr0_valid = 1;
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        p_data.ip_addr0.ipv6[0] = sal_htonl(ipv6_address[0]);
        p_data.ip_addr0.ipv6[1] = sal_htonl(ipv6_address[1]);
        p_data.ip_addr0.ipv6[2] = sal_htonl(ipv6_address[2]);
        p_data.ip_addr0.ipv6[3] = sal_htonl(ipv6_address[3]);
        CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        p_data.addr0_mask = masklen;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-addr1");
    if (index != 0xFF)
    {
        p_data.ipaddr1_valid = 1;
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        p_data.ip_addr1.ipv6[0] = sal_htonl(ipv6_address[0]);
        p_data.ip_addr1.ipv6[1] = sal_htonl(ipv6_address[1]);
        p_data.ip_addr1.ipv6[2] = sal_htonl(ipv6_address[2]);
        p_data.ip_addr1.ipv6[3] = sal_htonl(ipv6_address[3]);
        CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);
        p_data.addr1_mask = masklen;
    }

    index = CTC_CLI_GET_ARGC_INDEX("force-bridge");
    if (index != 0xFF)
    {
        if ((argc > index + 1) && CTC_CLI_STR_EQUAL_ENHANCE("disable", index+1))
        {
            p_data.force_bridge_en = 0;
        }
        else
        {
            p_data.force_bridge_en = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("force-unicast");
    if (index != 0xFF)
    {
        if ((argc > index + 1) && CTC_CLI_STR_EQUAL_ENHANCE("disable", index+1))
        {
            p_data.force_ucast_en = 0;
        }
        else
        {
            p_data.force_ucast_en = 1;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_set_mcast_force_route(g_api_lchip, &p_data);
    }
    else
    {
        ret = ctc_ipmc_set_mcast_force_route(&p_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_set_rp,
        ctc_cli_ipmc_set_rp_cmd,
        "ipmc (add|remove) bidipim RP_ID (rp-if0 IFID0 (rp-if1 IFID1 (rp-if2 IFID2 (rp-if3 IFID3|) |) |))",
        CTC_CLI_IPMC_M_STR,
        "Add",
        "Remove",
        "Bidi-pim",
        "Rp id",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC)
{
    uint8 index = 0;
    int32 ret  = CLI_SUCCESS;
    ctc_ipmc_rp_t ipmc_rp_info;

    sal_memset(&ipmc_rp_info, 0, sizeof(ctc_ipmc_rp_t));

    CTC_CLI_GET_UINT8_RANGE("rp-id", ipmc_rp_info.rp_id, argv[1], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("rp-if0");
    if (index != 0xFF)
    {
        ipmc_rp_info.rp_intf_count = ipmc_rp_info.rp_intf_count + 1;
        CTC_CLI_GET_UINT16_RANGE("rp-if", ipmc_rp_info.rp_intf[0], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

    }

    index = CTC_CLI_GET_ARGC_INDEX("rp-if1");
    if (index != 0xFF)
    {
        ipmc_rp_info.rp_intf_count = ipmc_rp_info.rp_intf_count + 1;
        CTC_CLI_GET_UINT16_RANGE("rp-if", ipmc_rp_info.rp_intf[1], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rp-if2");
    if (index != 0xFF)
    {
        ipmc_rp_info.rp_intf_count = ipmc_rp_info.rp_intf_count + 1;
        CTC_CLI_GET_UINT16_RANGE("rp-if", ipmc_rp_info.rp_intf[2], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("rp-if3");
    if (index != 0xFF)
    {
        ipmc_rp_info.rp_intf_count = ipmc_rp_info.rp_intf_count + 1;
        CTC_CLI_GET_UINT16_RANGE("rp-if", ipmc_rp_info.rp_intf[3], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (index != 0xFF)
    {
        if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_add_rp_intf(g_api_lchip, &ipmc_rp_info);
    }
    else
    {
        ret = ctc_ipmc_add_rp_intf(&ipmc_rp_info);
    }

    }
    else
    {
        if(g_ctcs_api_en)
        {
           ret = ctcs_ipmc_remove_rp_intf(g_api_lchip, &ipmc_rp_info);
        }
        else
        {
            ret = ctc_ipmc_remove_rp_intf(&ipmc_rp_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipmc_add_default_entry,
        ctc_cli_ipmc_add_default_entry_cmd,
        "ipmc (ipv6 |) default (to-cpu | drop | fallback-bridge)",
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPV6_STR,
        "Default",
        "Send the packet that hit the default entry to cpu",
        "Drop the packet that hit the default entry",
        "Do RPF check and fallback bridge to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    uint8 ip_version = 0;
    ctc_ipmc_default_action_t action = CTC_IPMC_DEFAULT_ACTION_MAX;

    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (index != 0xFF)
    {
        ip_version = CTC_IP_VER_6;
    }
    else
    {
        ip_version = CTC_IP_VER_4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu");
    if (index != 0xFF)
    {
        action = CTC_IPMC_DEFAULT_ACTION_TO_CPU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("drop");
    if (index != 0xFF)
    {
        action = CTC_IPMC_DEFAULT_ACTION_DROP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fallback-bridge");
    if (index != 0xFF)
    {
        action = CTC_IPMC_DEFAULT_ACTION_FALLBACK_BRIDGE;
    }

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_add_default_entry(g_api_lchip, ip_version, action);
    }
    else
    {
        ret = ctc_ipmc_add_default_entry(ip_version, action);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_set_ipv4_group_hit,
        ctc_cli_usw_ipmc_set_ipv4_group_hit_cmd,
        "ipmc group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID hit VALUE ",
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        "VRF id or FID",
        "VRF id or FID value",
        "Entry hit status",
        "HIT VAULE")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
	uint8 hit = 0;
	
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;
    CTC_CLI_GET_UINT8_RANGE("hit", hit, argv[5], 0, CTC_MAX_UINT8_VALUE);
    
    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_set_entry_hit(g_api_lchip, &ipmc_group_info,hit);
    }
    else
    {
        ret = ctc_ipmc_set_entry_hit(&ipmc_group_info,hit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_set_ipv6_group_hit,
        ctc_cli_usw_ipmc_set_ipv6_group_hit_cmd,
        "ipmc ipv6 group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID hit VALUE",
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Entry hit status",
        "HIT VALUE")
{
    int32 ret  = CLI_SUCCESS;
    uint16 vrf_id = 16;
    uint32 masklen = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
	uint8 hit = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 0;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;
    CTC_CLI_GET_UINT8_RANGE("hit", hit, argv[5], 0, CTC_MAX_UINT8_VALUE);
    

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_set_entry_hit(g_api_lchip, &ipmc_group_info,hit);
    }
    else
    {
        ret = ctc_ipmc_set_entry_hit(&ipmc_group_info,hit);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv4_group_hit,
        ctc_cli_usw_ipmc_show_ipv4_group_hit_cmd,
        "show ipmc group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID hit",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        "VRF id or FID",
        "VRF id or FID value",
        "Entry hit status")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
	uint8 hit = 0;
	
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[0]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[2]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;
    
    
    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_get_entry_hit(g_api_lchip, &ipmc_group_info,&hit);
    }
    else
    {
        ret = ctc_ipmc_get_entry_hit(&ipmc_group_info,&hit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit value:%d \n\r", hit);


    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv6_group_hit,
        ctc_cli_usw_ipmc_show_ipv6_group_hit_cmd,
        "show ipmc ipv6 group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID hit",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        CTC_CLI_IPMC_GROUP_DESC,
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Entry hit status")
{
    int32 ret  = CLI_SUCCESS;
    uint16 vrf_id = 16;
    uint32 masklen = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
	uint8 hit = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 0;


    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[2]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[3], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;
    
    

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipmc_get_entry_hit(g_api_lchip, &ipmc_group_info,&hit);
    }
    else
    {
        ret = ctc_ipmc_get_entry_hit(&ipmc_group_info,&hit);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit value:%d \n\r", hit);



    return CLI_SUCCESS;
}

int32
ctc_ipmc_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_mcast_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_mcast_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_rpf_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_rpf_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv4_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_remove_ipv4_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv4_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_remove_ipv4_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_update_ipv4_rpf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv4_default_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv6_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_remove_ipv6_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv6_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_remove_ipv6_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_update_ipv6_rpf_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_ipv6_default_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_set_ipv4_force_route_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_set_ipv6_force_route_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_rpf_profile_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_set_rp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipmc_add_default_entry_cmd);

	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_set_ipv4_group_hit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_set_ipv6_group_hit_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_group_hit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_group_hit_cmd);

    return CLI_SUCCESS;
}

