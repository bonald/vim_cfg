#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file ctc_ipuc_cli.c

 @date 2009-12-30

 @version v2.0

 The file apply clis of ipuc module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_ipuc_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_ipuc.h"

extern int32
ctc_usw_ipuc_set_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8 hit);
extern int32
ctc_usw_ipuc_get_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ipuc_nat_sa_param, uint8* hit);


extern int32
sys_usw_ipuc_db_show(uint8 lchip, ctc_ip_ver_t ip_ver, uint32* p_ipuc_data,uint32 detail);
extern int32
sys_usw_ipuc_dump_ipuc_info(uint8 lchip, ctc_ipuc_param_t * p_ctc_param_info);
#ifdef DUET2
extern int32
sys_duet2_calpm_db_show(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint32 detail);
extern int32
sys_duet2_calpm_enable_dma(uint8 lchip, uint8 enable);
extern int32
sys_duet2_get_calpm_prefix_mode(uint8 lchip);
#endif
extern int32
sys_usw_ipuc_show_status(uint8 lchip);
extern int32
_sys_usw_ipuc_rpf_check_port(uint8 lchip, uint8 enable);
extern int32
sys_usw_ipuc_show_debug_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);
extern int32
sys_usw_ip_tunnel_show_natsa_db(uint8 lchip, ctc_ip_ver_t ip_ver, uint32 detail);
#ifdef TSINGMA
extern int32
sys_tsingma_nalpm_dump_route_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver, uint16 tcam_idx);
extern int32
sys_tsingma_nalpm_dump_pfx_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver);
extern int32
sys_usw_ipuc_set_route_spec(uint8 lchip, uint32 v4_spec, uint32 v6_spec);
extern int32
sys_tsingma_nalpm_auto_merge(uint8 lchip, uint8 enable);
extern  int32
sys_tsingma_nalpm_merge(uint8 lchip, uint32 vrf_id, uint8 ip_ver);
extern int32
sys_usw_ipuc_set_aid(uint8 lchip, uint8 aid_en,uint8 aid_list_len, uint8 async);
extern int32
sys_usw_ipuc_show_aid_info(uint8 lchip);
#endif

CTC_CLI(ctc_cli_usw_ipuc_show_ipv4,
        ctc_cli_usw_ipuc_show_ipv4_cmd,
        "show ipuc (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipuc_db_show(lchip, CTC_IP_VER_4, NULL,FALSE);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#ifdef DUET2
CTC_CLI(ctc_cli_usw_ipuc_show_ipv4_calpm_info,
        ctc_cli_usw_ipuc_show_ipv4_calpm_info_cmd,
        "show ipuc calpm VRFID (all |prefix A.B.C.D MASK_LEN) (detail|node|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "Show ipuc calpm prefix route info",
        CTC_CLI_VRFID_ID_DESC", all vrf is MAX_VRF_NUM",
        "All ipv4 calpm route",
        "Ipv4 prefix, support prefix or prefix+8",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "show detail info",
        "show detail and node info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 vrfid = 0;
    uint32 detail = 0;
    ctc_ipuc_param_t ipuc_info = {0};
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        detail = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("node");
    if (0xFF != index)
    {
        detail = 2;
    }

    ipuc_info.ip_ver = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF == index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("prefix");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[index+1]);
            CTC_CLI_GET_UINT8_RANGE("mask", ipuc_info.masklen, argv[index+2], 0, CTC_MAX_UINT8_VALUE);
        }
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_duet2_calpm_db_show(lchip, &ipuc_info, detail);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv6_calpm_info,
        ctc_cli_usw_ipuc_show_ipv6_calpm_info_cmd,
        "show ipuc calpm ipv6 VRFID (all | prefix X:X::X:X MASK_LEN |) (detail|node|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "Show ipuc calpm prefix route info",
        "Ip version 6",
        CTC_CLI_VRFID_ID_DESC", all vrf is MAX_VRF_NUM",
        "All ipv6 calpm route",
        "Ipv6 prefix, support 48 or 56",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "show detail info",
        "show detail and node info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 vrfid = 0;
    uint32 detail = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info = {0};
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        detail = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("node");
    if (0xFF != index)
    {
        detail = 1;
    }

    ipuc_info.ip_ver = CTC_IP_VER_6;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF == index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("prefix");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);

            /* adjust endian */
            ipuc_info.ip.ipv6[0] = sal_htonl(ipv6_address[0]);
            ipuc_info.ip.ipv6[1] = sal_htonl(ipv6_address[1]);
            ipuc_info.ip.ipv6[2] = sal_htonl(ipv6_address[2]);
            ipuc_info.ip.ipv6[3] = sal_htonl(ipv6_address[3]);
            CTC_CLI_GET_UINT8_RANGE("mask", ipuc_info.masklen, argv[index+2], 0, CTC_MAX_UINT8_VALUE);
        }
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_duet2_calpm_db_show(lchip, &ipuc_info, detail);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_write_lpm_by_dma,
        ctc_cli_usw_ipuc_write_lpm_by_dma_cmd,
        "ipuc dma (enable|disable) (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "DMA",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index;
    bool dma_enable;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        dma_enable = TRUE;
    }
    else
    {
        dma_enable = FALSE;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_duet2_calpm_enable_dma(lchip, dma_enable);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_prefix_mode,
        ctc_cli_usw_ipuc_show_prefix_mode_cmd,
        "show ipuc prefix mode (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "prefix",
        "mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
    	CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_duet2_get_calpm_prefix_mode(lchip);

    ctc_cli_out("LPM Prefix Mode : %d\n",ret);

    return CLI_SUCCESS;
}

#endif

CTC_CLI(ctc_cli_usw_ipuc_show_ipv6,
        ctc_cli_usw_ipuc_show_ipv6_cmd,
        "show ipuc ipv6 (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipuc_db_show(lchip, CTC_IP_VER_6, NULL,FALSE);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_state,
        ctc_cli_usw_ipuc_show_state_cmd,
        "show ipuc status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "IPUC configure and state",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipuc_show_status(lchip);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_arrange_fragment,
        ctc_cli_usw_ipuc_arrange_fragment_cmd,
        "ipuc arrange fragment",
        CTC_CLI_IPUC_M_STR,
        "Arrange calpm fragment",
        "Arrange calpm fragment")
{
    int32 ret = CLI_ERROR;

    if(g_ctcs_api_en)
    {
       ret = ctcs_ipuc_arrange_fragment(g_api_lchip, NULL);
    }
    else
    {
        ret = ctc_ipuc_arrange_fragment(NULL);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv4_info,
        ctc_cli_usw_ipuc_show_ipv4_info_cmd,
        "show ipuc VRFID A.B.C.D MASK_LEN (end-mask MASK_LEN|)(is-neighbor |is-public |use-lpm | l4-dst-port L4PORT (tcp-port|) |) (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "Show ipuc ipv4 route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "End mask length",
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "This route is a Neighbor",
        "This route is a public router",
        "Host route use lpm",
        "Layer4 destination port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "If set, indicate it is a tcp port, or is a udp port",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 vrfid = 0;
    uint32 masklen = 0;
    uint32 end_masklen = 0;
    ctc_ipuc_param_t ipuc_info = {0};
    uint8  index = 0;
    uint8 i = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-neighbor");
    if (0xFF != index)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-public");
    if (0xFF != index)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst-port", ipuc_info.l4_dst_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcp-port");
        if (index != 0xFF)
        {
            ipuc_info.is_tcp_port = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("end-mask");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("end-mask", end_masklen, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipuc_info.masklen = masklen;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(0xff == CTC_CLI_GET_ARGC_INDEX_ENHANCE("end-mask"))
    {
        ret = sys_usw_ipuc_show_debug_info(lchip, &ipuc_info);
    }
    else
    {
        ctc_cli_out("Offset: T-TCAM    S-SRAM\n\r");
        ctc_cli_out("Flags:  R-RPF check    T-TTL check    I-ICMP redirect check      C-Send to CPU\n\r");
        ctc_cli_out("        X-Connect      P-Protocol entry           S-Self address     U-TCP port\n\r");
        ctc_cli_out("        E-ICMP error msg check    B-Public route    M-Merge dsfwd      L-Host use Lpm\n\r");
        ctc_cli_out("        O-None flag\n\r");
        ctc_cli_out("---------------------------------------------------------------------------------------\n\r");
        ctc_cli_out("VRF   Route                      NHID   Flags   In_SRAM\n\r");
        ctc_cli_out("---------------------------------------------------------------------------------------\n\r");
        for(i=masklen; i<=end_masklen; i++)
        {
            ipuc_info.masklen = i;
            sys_usw_ipuc_dump_ipuc_info(lchip, &ipuc_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv6_info,
        ctc_cli_usw_ipuc_show_ipv6_info_cmd,
        "show ipuc ipv6 VRFID X:X::X:X MASK_LEN (end-mask MASK_LEN|)(is-neighbor | is-public| use-lpm |)(lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        "show ipuc ipv6 route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "End mask length",
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "This route is a Neighbor",
        "This route is a public router",
        "Host route use lpm",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 vrfid = 0;
    uint32 masklen = 0;
    uint32 end_masklen = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info = {0};
    uint8  index = 0;
    uint8 i = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-neighbor");
    if (0xFF != index)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-public");
    if (0xFF != index)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("end-mask");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("end-mask", end_masklen, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    ipuc_info.ip_ver = CTC_IP_VER_6;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[1]);

    /* adjust endian */
    ipuc_info.ip.ipv6[0] = sal_htonl(ipv6_address[0]);
    ipuc_info.ip.ipv6[1] = sal_htonl(ipv6_address[1]);
    ipuc_info.ip.ipv6[2] = sal_htonl(ipv6_address[2]);
    ipuc_info.ip.ipv6[3] = sal_htonl(ipv6_address[3]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipuc_info.masklen = masklen;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(0xff == CTC_CLI_GET_ARGC_INDEX_ENHANCE("end-mask"))
    {
        ret = sys_usw_ipuc_show_debug_info(lchip, &ipuc_info);
    }
    else
    {
        ctc_cli_out("Offset: T-TCAM    S-SRAM\n\r");
        ctc_cli_out("Flags:  R-RPF check    T-TTL check    I-ICMP redirect check      C-Send to CPU\n\r");
        ctc_cli_out("        X-Connect      P-Protocol entry           S-Self address     U-TCP port\n\r");
        ctc_cli_out("        E-ICMP error msg check    B-Public route    M-Merge dsfwd      L-Host use Lpm\n\r");
        ctc_cli_out("        O-None flag\n\r");
        ctc_cli_out("---------------------------------------------------------------------------------------\n\r");
        ctc_cli_out("VRF   Route                                      NHID   Flags   In_SRAM\n\r");
        ctc_cli_out("---------------------------------------------------------------------------------------\n\r");
        for(i = masklen; i <= end_masklen; i++)
        {
            ipuc_info.masklen = i;
            sys_usw_ipuc_dump_ipuc_info(lchip, &ipuc_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_rpf_check_port,
        ctc_cli_usw_ipuc_rpf_check_port_cmd,
        "ipuc rpf-check-port (enable|disable) (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "Rpf check by port",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index;
    uint8 enable;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    ret = _sys_usw_ipuc_rpf_check_port(lchip, enable);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv4_nat_sa,
        ctc_cli_usw_ipuc_show_ipv4_nat_sa_cmd,
        "show ipuc nat-sa (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ip_tunnel_show_natsa_db(lchip, CTC_IP_VER_4, FALSE);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_set_ipv4_nat_sa_hit,
        ctc_cli_usw_ipuc_set_ipv4_nat_sa_hit_cmd,
        "ipuc nat-sa VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|) (hit VALUE) (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        "Layer4 source port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "Indicate l4-src-port is tcp port or not. If set is TCP port else is UDP port",
        "Entry hit status",
        "HIT VALUE",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint8 index = 0;
    ctc_ipuc_nat_sa_param_t nat_info = {0};
    uint8 hit  = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    nat_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", nat_info.ipsa.ipv4, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src-port", nat_info.l4_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcp-port");
        if (index != 0xFF)
        {
            nat_info.flag |= CTC_IPUC_NAT_FLAG_USE_TCP_PORT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("hit");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("hit", hit, argv[index + 1]);
        hit = hit ? 1 : 0;
    }
    
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    
  
    ret = ctc_usw_ipuc_set_natsa_entry_hit(lchip, &nat_info, hit);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_ipuc_show_ipv4_nat_sa_hit,
        ctc_cli_usw_ipuc_show_ipv4_nat_sa_hit_cmd,
        "show ipuc nat-sa VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|) hit (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        "Layer4 source port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "Indicate l4-src-port is tcp port or not. If set is TCP port else is UDP port",
        "Entry hit status",   
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint8 index = 0;
    ctc_ipuc_nat_sa_param_t nat_info = {0};
    uint8 hit  = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    nat_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", nat_info.ipsa.ipv4, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src-port", nat_info.l4_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcp-port");
        if (index != 0xFF)
        {
            nat_info.flag |= CTC_IPUC_NAT_FLAG_USE_TCP_PORT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    
    
    ret = ctc_usw_ipuc_get_natsa_entry_hit(lchip, &nat_info, &hit);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit value:%d \n\r", hit);
    return CLI_SUCCESS;
}



#ifdef TSINGMA
CTC_CLI(ctc_cli_usw_ipuc_show_prefix_mode,
        ctc_cli_usw_ipuc_show_prefix_mode_cmd,
        "show ipuc prefix mode (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "prefix",
        "mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
    	CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
 /*    ret = sys_usw_get_calpm_prefix_mode(lchip);*/

    ctc_cli_out("LPM Prefix Mode : %d\n",ret);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_tsingma_ipuc_show_lpm_trie,
        ctc_cli_tsingma_ipuc_show_lpm_trie_cmd,
        "show ipuc nalpm trie (prefix | (route  tcam-idx TCAM_IDX)) (vrfid VRFID ip-ver VER) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "NALPM",
        "DB Tree",
        "router  common prefix",
        "routers share a prefix",
        "tcam key idx",
        "index value",
        "vrfid",
        "vrfid value",
        "ip-version",
        "0 ipv4, 1 ipv6",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;
    uint16 tcam_idx = 0;
    uint16 vrfid = 0;
    uint8 ip_ver = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vrfid", vrfid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-ver");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("ip-ver", ip_ver, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("tcam-idx");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("tcam-idx", tcam_idx, argv[index + 1]);
        lchip = g_ctcs_api_en?g_api_lchip:lchip;
        ret = sys_tsingma_nalpm_dump_route_trie( lchip,  vrfid,  ip_ver,  tcam_idx);
    }
    else
    {
        ret = sys_tsingma_nalpm_dump_pfx_trie( lchip,  vrfid,  ip_ver);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_route_spec,
        ctc_cli_usw_ipuc_route_spec_cmd,
        "ipuc lpm-prefix {v4 NUM|v6 NUM} (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "LPM tcam-prefix spec",
        "IPv4-num",
        "IPv4 value",
        "IPv6-num",
        "IPv6 value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint8  index = 0;
    uint32 v4_spec = 0;
    uint32 v6_spec = 0;
    int32 ret = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en ? g_api_lchip:lchip;

    index = CTC_CLI_GET_ARGC_INDEX("v4");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("v4", v4_spec, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("v6");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("v6", v6_spec, argv[index + 1]);
    }

    ret = sys_usw_ipuc_set_route_spec(lchip,  v4_spec,  v6_spec);
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_usw_ipuc_aid,
        ctc_cli_usw_ipuc_aid_cmd,
        "ipuc aid  {aid-en VALUE| aid-list-len LEN |async MODE} (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "AI detect",
        "aid-en",
        "0 or 1",
        "aid-list-len",
        "Length",
        "alpm async mode",
        "Mode",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    uint8 aid_en =0;
    uint8 aid_list_len = 16;
    uint8 async = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en ? g_api_lchip:lchip;

    index = CTC_CLI_GET_ARGC_INDEX("aid-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("aid-en", aid_en, argv[index + 1]);
        async = aid_en;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aid-list-len");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("aid-list-len", aid_list_len, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("async");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("async mode", async, argv[index + 1]);
    }

    sys_usw_ipuc_set_aid(lchip,  aid_en, aid_list_len,  async);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_aid_show,
        ctc_cli_usw_ipuc_aid_show_cmd,
        "show ipuc aid info (lchip LCHIP|)",
       CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        "aid",
        "Infomation"
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;

    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en ? g_api_lchip:lchip;

    sys_usw_ipuc_show_aid_info(lchip);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_tsingma_ipuc_set_arrange_fragment_enable,
    ctc_cli_tsingma_ipuc_set_arrange_fragment_enable_cmd,
    "ipuc nalpm arrange-fragment (auto-run ENABLE)(lchip LCHIP|)",
    CTC_CLI_IPUC_M_STR,
    "Nalpm",
    "Arrange-fragment",
    "Auto run when delete routes",
    "Value",
    "Lchip",
    "Value")
{
    int32 ret = CLI_ERROR;
    uint8 index = 0;
    uint8 enable = 0;
    uint8 lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("auto-run");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("auto-run", enable, argv[index + 1]);
    }
    ret = sys_tsingma_nalpm_auto_merge(lchip, enable);

    return ret;
}

CTC_CLI(ctc_cli_usw_ipuc_merger,
        ctc_cli_usw_ipuc_merger_cmd,
        "ipuc merge vrf-id ID ip-ver VER (lchip LCHIP|)",
        CTC_CLI_IPUC_M_STR,
        "merge",
        "vrfid",
        "Value",
        "ip version",
        "Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_ERROR;
    uint8 lchip = 0;
    uint8  index = 0;
    uint32 vrfid = 0;
    uint8 ip_ver = 0;


    CTC_CLI_GET_UINT32("vrfid", vrfid, argv[0]);
    CTC_CLI_GET_UINT8("ip-ver", ip_ver, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_tsingma_nalpm_merge(lchip, vrfid, ip_ver);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#endif

int32
ctc_usw_ipuc_cli_init(void)
{

#ifdef SDK_INTERNAL_CLI_SHOW
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_rpf_check_port_cmd);
#ifdef DUET2
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_write_lpm_by_dma_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_prefix_mode_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_ipv4_calpm_info_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_ipv6_calpm_info_cmd);
#endif
#ifdef TSINGMA
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_prefix_mode_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_tsingma_ipuc_show_lpm_trie_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_route_spec_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_aid_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_aid_show_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_tsingma_ipuc_set_arrange_fragment_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_merger_cmd);
#endif
#endif


    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_arrange_fragment_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_state_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv6_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_nat_sa_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_set_ipv4_nat_sa_hit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_nat_sa_hit_cmd);
    return CLI_SUCCESS;
}

int32
ctc_usw_ipuc_cli_deinit(void)
{

#ifdef SDK_INTERNAL_CLI_SHOW
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_rpf_check_port_cmd);
#ifdef DUET2
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_write_lpm_by_dma_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_prefix_mode_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_ipv4_calpm_info_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_ipv6_calpm_info_cmd);
#endif
#ifdef TSINGMA
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_show_prefix_mode_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_tsingma_ipuc_show_lpm_trie_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_route_spec_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_aid_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_ipuc_aid_show_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_tsingma_ipuc_set_arrange_fragment_enable_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_merger_cmd);
#endif
#endif

#ifdef DUET2
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_arrange_fragment_cmd);
#endif
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv6_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_state_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv6_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_nat_sa_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_set_ipv4_nat_sa_hit_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_nat_sa_hit_cmd);

    return CLI_SUCCESS;
}

#endif

