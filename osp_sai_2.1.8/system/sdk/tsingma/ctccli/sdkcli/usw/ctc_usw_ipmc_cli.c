#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_hash.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_ipuc.h"

extern int32
sys_usw_ipmc_db_show_group_info(uint8 lchip, uint8 ip_version, ctc_ipmc_group_info_t* p_node);

extern int32
sys_usw_ipmc_db_show_member_info(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_db_show_count(uint8 lchip);

extern int32
sys_usw_ipmc_show_status(uint8 lchip);
extern int32
sys_usw_rpf_profile_dump_status(uint8 lchip,uint8 is_ipmc);
extern int32
sys_usw_ipmc_show_default_entry(uint8 lchip, uint8 ip_version, uint16 vrf_start, uint16 vrf_end);
extern int32
sys_usw_ipmc_show_rp_intf(uint8 lchip, uint8 rp_id);
extern int32
sys_usw_ipmc_set_bidipim_mode(uint8 lchip, uint8 bidipim_mode);
extern int32
sys_usw_ipmc_dump_group_node_from_db(uint8 lchip, uint16 group_id);


static ctc_ipmc_group_info_t ipmc_group_info;

CTC_CLI(ctc_cli_usw_ipmc_set_bidipim_mode,
    ctc_cli_usw_ipmc_set_bidipim_mode_cmd,
    "ipmc global set bidipim mode VALUE (lchip LCHIP|)",
    CTC_CLI_IPMC_M_STR,
    "Ipmc global configuration",
    "Set",
    "Bidipim attribute",
    "Mode",
    "Value",
    CTC_CLI_LCHIP_ID_STR,
    CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint8 value = 0;
    uint8 index = 0;
    int32 ret = 0;
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    CTC_CLI_GET_UINT8_RANGE("Value", value, argv[0], 0, CTC_MAX_UINT8_VALUE);

    ret = sys_usw_ipmc_set_bidipim_mode(lchip, value);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv4_group_info,
        ctc_cli_usw_ipmc_show_ipv4_group_info_cmd,
        "show ipmc group info (group GROUPID|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "IP multicast group",
        "Information, including source and group IP adress, etc.",
        "Specified group ID",
        "Group ID",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 ip_version = 0;
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint16 group_id = 0;

    ip_version = CTC_IP_VER_4;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("group", group_id, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(0xFF == CTC_CLI_GET_ARGC_INDEX("group"))
    {
        ret = sys_usw_ipmc_db_show_group_info(lchip, ip_version, NULL);
    }
    else
    {
        ret = sys_usw_ipmc_dump_group_node_from_db(lchip, group_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv6_group_info,
        ctc_cli_usw_ipmc_show_ipv6_group_info_cmd,
        "show ipmc ipv6 group info (group GROUPID|)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "IPv6",
        CTC_CLI_IPMC_M_STR,
        "IP multicast group",
        "Information, including source and group IP adress, etc.",
        "Specified group ID",
        "Group ID",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 ip_version = 0;
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint16 group_id = 0;

    ip_version = CTC_IP_VER_6;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("group", group_id, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(0xFF == CTC_CLI_GET_ARGC_INDEX("group"))
    {
        ret = sys_usw_ipmc_db_show_group_info(lchip, ip_version, NULL);
    }
    else
    {
        ret = sys_usw_ipmc_dump_group_node_from_db(lchip, group_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv4_group_param,
        ctc_cli_usw_ipmc_show_ipv4_group_param_cmd,
        "show ipmc group info group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (ip-based-l2mc|) "\
        "(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "IP multicast group",
        "Information, including source and group IP adress, etc.",
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ipmc_group_info.ip_version = CTC_IP_VER_4;

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

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_get_group_info(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_get_group_info(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_db_show_group_info(lchip, ipmc_group_info.ip_version,&ipmc_group_info);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }



    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv6_group_param,
        ctc_cli_usw_ipmc_show_ipv6_group_param_cmd,
        "show ipmc ipv6 group info group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (ip-based-l2mc|) "\
        "(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "IPv6",
        CTC_CLI_IPMC_M_STR,
        "IP multicast group",
        "Information, including source and group IP adress, etc.",
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};

 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

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

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_get_group_info(g_api_lchip, &ipmc_group_info);
    }
    else
    {
        ret = ctc_ipmc_get_group_info(&ipmc_group_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_db_show_group_info(lchip, ipmc_group_info.ip_version,&ipmc_group_info);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv4_member_info,
        ctc_cli_usw_ipmc_show_ipv4_member_info_cmd,
        "show ipmc member info group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID (ip-based-l2mc|) "\
        "(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "Members of a given multicast group",
        "Information, including gport, vlan, l3 type.",
        "Group address, class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "The length of group address mask <32>",
        "Source address",
        CTC_CLI_IPV4_FORMAT,
        "The length of source address mask <0 or 32>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ipmc_group_info.ip_version = CTC_IP_VER_4;

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

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_db_show_member_info(lchip, &ipmc_group_info);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipv6_member_info,
        ctc_cli_usw_ipmc_show_ipv6_member_info_cmd,
        "show ipmc ipv6 member info group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID (ip-based-l2mc|) "\
        "(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Members of a given multicast group",
        "Information, including gport, vlan, l3 type.",
        "Group address, IPv6 address begin with FF",
        CTC_CLI_IPV6_FORMAT,
        "The length of group address mask <128>",
        "Source address",
        CTC_CLI_IPV6_FORMAT,
        "The length of source address mask <0 or 128>",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Do IP based L2 mcast",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};

 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

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

    index = CTC_CLI_GET_ARGC_INDEX("ip-based-l2mc");
    if (index != 0xFF)
    {
        ipmc_group_info.flag |= CTC_IPMC_FLAG_L2MC;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_db_show_member_info(lchip, &ipmc_group_info);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_ipmc_status,
        ctc_cli_usw_ipmc_show_ipmc_status_cmd,
        "show ipmc status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "Ipmc and L2mc based ip Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ret = sys_usw_ipmc_db_show_count(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_ipmc_get_ipv4_force_route,
        ctc_cli_usw_ipmc_get_ipv4_force_route_cmd,
        "show ipmc force-route",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "Force route")
{
    int32 ret  = CLI_SUCCESS;
    uint32 tempip = 0;
    char buf[sizeof "255.255.255.255"] = "";
    ctc_ipmc_force_route_t p_data;

    sal_memset(&p_data, 0, sizeof(ctc_ipmc_force_route_t));
    p_data.ip_version = CTC_IP_VER_4;

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_get_mcast_force_route(g_api_lchip, &p_data);
    }
    else
    {
        ret = ctc_ipmc_get_mcast_force_route(&p_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("--------Ipmc ipv4 force route------\n\r");
    ctc_cli_out("Force Bridge       : %d\n\r",p_data.force_bridge_en);
    ctc_cli_out("Force Unicast      : %d\n\r",p_data.force_ucast_en);
    if(p_data.ipaddr0_valid)
    {
        tempip = sal_ntohl(p_data.ip_addr0.ipv4);
        sal_inet_ntop(AF_INET, &tempip, buf, sizeof(buf));
        ctc_cli_out("ip_addr0/mask      : %s/%d\n\r",buf,p_data.addr0_mask);
    }else{
        ctc_cli_out("ip_addr0/mask      : N/A\n\r");
    }

    if(p_data.ipaddr1_valid)
    {
        tempip = sal_ntohl(p_data.ip_addr1.ipv4);
        sal_inet_ntop(AF_INET, &tempip, buf, sizeof(buf));
        ctc_cli_out("ip_addr1/mask      : %s/%d\n\r",buf,p_data.addr1_mask);
    }else{
        ctc_cli_out("ip_addr1/mask      : N/A\n\r");
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_get_ipv6_force_route,
        ctc_cli_usw_ipmc_get_ipv6_force_route_cmd,
        "show ipmc ipv6 force-route",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "IPv6",
        "Force route")
{
    int32 ret  = CLI_SUCCESS;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    char buf[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"] = "";
    ctc_ipmc_force_route_t p_data;

    sal_memset(&p_data, 0, sizeof(ctc_ipmc_force_route_t));
    p_data.ip_version = CTC_IP_VER_6;

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipmc_get_mcast_force_route(g_api_lchip, &p_data);
    }
    else
    {
        ret = ctc_ipmc_get_mcast_force_route(&p_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("--------Ipmc ipv6 force route------\n\r");
    ctc_cli_out("Force Bridge       : %d\n\r",p_data.force_bridge_en);
    ctc_cli_out("Force Unicast      : %d\n\r",p_data.force_ucast_en);
    if(p_data.ipaddr0_valid)
    {
        ipv6_address[0] = sal_htonl(p_data.ip_addr0.ipv6[0]);
        ipv6_address[1] = sal_htonl(p_data.ip_addr0.ipv6[1]);
        ipv6_address[2] = sal_htonl(p_data.ip_addr0.ipv6[2]);
        ipv6_address[3] = sal_htonl(p_data.ip_addr0.ipv6[3]);
        sal_inet_ntop(AF_INET6, ipv6_address, buf, sizeof(buf));
        ctc_cli_out("ip_addr0/mask      : %s/%d\n\r",buf,p_data.addr0_mask);
    }else{
        ctc_cli_out("ip_addr0/mask      : N/A\n\r");
    }

    if(p_data.ipaddr1_valid)
    {
        ipv6_address[0] = sal_htonl(p_data.ip_addr1.ipv6[0]);
        ipv6_address[1] = sal_htonl(p_data.ip_addr1.ipv6[1]);
        ipv6_address[2] = sal_htonl(p_data.ip_addr1.ipv6[2]);
        ipv6_address[3] = sal_htonl(p_data.ip_addr1.ipv6[3]);
        sal_inet_ntop(AF_INET6, ipv6_address, buf, sizeof(buf));
        ctc_cli_out("ip_addr1/mask      : %s/%d\n\r",buf,p_data.addr1_mask);
    }else{
        ctc_cli_out("ip_addr1/mask      : N/A\n\r");
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_rpf_profile,
        ctc_cli_usw_ipmc_rpf_profile_cmd,
        "show (ipmc|ipuc) rpf profile",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPUC_M_STR,
        "RPF",
        "RPF Profile status")
{
    int32 ret  = CLI_SUCCESS;
    uint8 ipmc = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ipmc");
    if (index != 0xFF)
    {
       ipmc = 1;
    }
    ret = sys_usw_rpf_profile_dump_status(g_api_lchip, ipmc);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_add_ipv4_group,
        ctc_cli_usw_ipmc_add_ipv4_group_cmd,
        "ipmc add group GROUPID group-address A.B.C.D MASK_LEN source-address A.B.C.D MASK_LEN vrf VRFID use-vlan ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu | bidipim RP_ID (bidipim-fail-to-cpu |) } | )",
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
        "Vrfid lookup as svlan id",
        "TTL check enable",
        "RPF check enable",
        "If RPF check fail, sent to cpu",
        "Statistic supported",
        "0~0xFFFFFFFF",
        "Do IP based L2 mcast",
        "Ptp-entry",
        "Redirect to cpu",
        "Bidipim",
        "Rp id",
        "If bidipim check fail, sent to cpu")
{
    int32 ret  = CLI_SUCCESS;
    uint32 masklen = 0;
    uint16 vrf_id = 0;
    uint8  index = 0;
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("groupId", ipmc_group_info.group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.group_addr, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipmc_group_info.address.ipv4.src_addr, argv[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[4], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[5], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv4.vrfid = vrf_id;

    ipmc_group_info.flag |= CTC_IPMC_FLAG_USE_VLAN;

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

CTC_CLI(ctc_cli_usw_ipmc_add_ipv6_group,
        ctc_cli_usw_ipmc_add_ipv6_group_cmd,
        "ipmc ipv6 add group GROUPID group-address X:X::X:X MASK_LEN source-address X:X::X:X MASK_LEN vrf VRFID use-vlan ({ttl-check |rpf-check |rpf-fail-to-cpu |stats STATS_ID | ip-based-l2mc | ptp-entry | redirect-tocpu | bidipim RP_ID (bidipim-fail-to-cpu |) } | )",
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
        "Vrfid lookup as svlan id",
        "TTL check enable",
        "RPF check enable",
        "If RPF check fail, sent to cpu",
        "Statistic supported",
        "0~0xFFFFFFFF",
        "Do IP based L2 mcast",
        "Ptp-entry",
        "Redirect to cpu",
        "Bidipim",
        "Rp id",
        "If bidipim check fail, sent to cpu",
        "Vrfid lookup as svlan id")
{
    int32 ret  = CLI_SUCCESS;
    uint16 vrf_id = 16;
    uint32 masklen = 0;
    uint8  index = 0;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
 /*    ctc_ipmc_group_info_t ipmc_group_info;*/

    sal_memset(&ipmc_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    ipmc_group_info.ip_version      = CTC_IP_VER_6;
    ipmc_group_info.member_number   = 0;

    CTC_CLI_GET_UINT16_RANGE("groupId", ipmc_group_info.group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[1]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.group_ip_mask_len = masklen;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[3]);

    /* adjust endian */
    ipmc_group_info.address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
    ipmc_group_info.address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
    ipmc_group_info.address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
    ipmc_group_info.address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[4], 0, CTC_MAX_UINT8_VALUE);
    ipmc_group_info.src_ip_mask_len = masklen;

    CTC_CLI_GET_UINT16_RANGE("vrf", vrf_id, argv[5], 0, CTC_MAX_UINT16_VALUE);
    ipmc_group_info.address.ipv6.vrfid = vrf_id;

    ipmc_group_info.flag |= CTC_IPMC_FLAG_USE_VLAN;

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

CTC_CLI(ctc_cli_usw_ipmc_show_default_entry,
        ctc_cli_usw_ipmc_show_default_entry_cmd,
        "show ipmc (ipv6|) default vrf START END",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "Ipv6",
        "Default action",
        CTC_CLI_VRFID_DESC,
        "Start vrfid",
        "End vrfid")
{
    int32 ret  = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 ip_version = 0;
    uint16 start_vrfid = 0;
    uint16 end_vrfid = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (index != 0xFF)
    {
        ip_version = CTC_IP_VER_6;
    }
    else
    {
        ip_version = CTC_IP_VER_4;
    }

    CTC_CLI_GET_UINT16_RANGE("vrf", start_vrfid, argv[argc-2], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vrf", end_vrfid, argv[argc-1], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipmc_show_default_entry(lchip, ip_version, start_vrfid, end_vrfid);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipmc_show_rp,
        ctc_cli_usw_ipmc_show_rp_cmd,
        "show ipmc bidipim RP_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPMC_M_STR,
        "Bidi-pim",
        "Rp id")
{
    uint8 rp_id = 0;
    int32 ret  = CLI_SUCCESS;

    CTC_CLI_GET_UINT8_RANGE("rp-id", rp_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    ret = sys_usw_ipmc_show_rp_intf(g_api_lchip, rp_id);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_ipmc_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_group_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_group_param_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_member_info_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_group_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_group_param_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_member_info_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipmc_status_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_get_ipv4_force_route_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_get_ipv6_force_route_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_rpf_profile_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_add_ipv4_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_add_ipv6_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_rp_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipmc_set_bidipim_mode_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_ipmc_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_group_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_group_param_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv4_member_info_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_group_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_group_param_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipv6_member_info_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_ipmc_status_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_get_ipv4_force_route_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_get_ipv6_force_route_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_rpf_profile_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_add_ipv4_group_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_add_ipv6_group_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_default_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipmc_show_rp_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipmc_set_bidipim_mode_cmd);

    return CLI_SUCCESS;
}

#endif

