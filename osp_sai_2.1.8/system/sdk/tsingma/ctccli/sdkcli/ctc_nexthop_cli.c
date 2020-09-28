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
#include "ctc_nexthop.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#include "ctc_nexthop_cli.h"

#define CTC_CLI_NH_MPLS_ADJ_LEN_DESC \
        "for qos shape adjusting packet length",\
        "LEN"

#define CTC_CLI_NH_MPLS_POP_NH_PARAM_STR "payload-op op-route\
    ((pipe|short-pipe|uniform|)|) (sequence-number SQN_INDEX |) "CTC_CLI_MAC_ARP_STR CTC_CLI_L3IF_ALL_STR"(adjust-length LEN|)"
#define CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC "Mpls pop nexthop payload operation type",                            \
    "POP PHP operation",                     \
    "PHP's Pipe mode",                                                                                          \
    "PHP's Short Pipe mode",                                                                                         \
    "PHP's Uniform mode",                                                                                          \
    "Enable sequence order",                                                                                    \
    "Sequence counter index",                                                                          \
    CTC_CLI_MAC_ARP_DESC,                                                                                       \
    CTC_CLI_L3IF_ALL_DESC,                                                                                      \
    CTC_CLI_NH_MPLS_ADJ_LEN_DESC

#define CTC_CLI_NH_MISC_TRUNC_LEN_STR "truncated-len LENGTH"
#define CTC_CLI_NH_MISC_TRUNC_LEN_DESC "Truncated length", "Length, 0:disable"

extern int32
ctc_app_index_alloc_dsnh_offset(uint8 alloc_dsnh_gchip ,uint8 nh_type ,void* nh_param,
                                uint32 *dsnh_offset,uint32 *entry_num);
int32
_ctc_nexthop_cli_alloc_dsnh_offset(uint8 alloc_dsnh_gchip,uint8 nh_type, void* nh_param)
{
    int ret = CLI_SUCCESS;
	uint32 dsnh_offset = 0;
	uint32 entry_num = 0;

	ret = ctc_app_index_alloc_dsnh_offset(alloc_dsnh_gchip ,nh_type ,nh_param,&dsnh_offset,&entry_num);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

	if (0 == entry_num)
    {
        ctc_cli_out("%%No need to allocate dsnh_offset. \n");
    }
    else
    {
        ctc_cli_out("%%Need to allocate dsnh_offset,dsnh_offset: %u entry_num: %d\n",dsnh_offset, entry_num);
    }
    return ret;

}
int32
_ctc_vlan_parser_egress_vlan_edit(ctc_vlan_egress_edit_info_t* vlan_edit, char** argv, uint16 argc)
{
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-edit-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("CVLAN Edit Type", vlan_edit->cvlan_edit_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-edit-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("SVLAN Edit Type", vlan_edit->svlan_edit_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("output-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("OUTPUT Cvid", vlan_edit->output_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("output-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("OUTPUT Svid", vlan_edit->output_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("swap-tpid");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_TPID_SWAP_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("swap-cos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-valid");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-svlan-cos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
        CTC_CLI_GET_UINT8_RANGE("OUTPUT Stag Cos", vlan_edit->stag_cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("map-svlan-cos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_MAP_SVLAN_COS);
    }

    index = CTC_CLI_GET_ARGC_INDEX("swap-vlan");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN);
    }


    index = CTC_CLI_GET_ARGC_INDEX("svlan-valid");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
    }

    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("user-vlanptr", vlan_edit->user_vlanptr, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->flag, CTC_VLAN_NH_ETREE_LEAF);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-tpid-index");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID);
        CTC_CLI_GET_UINT8_RANGE("svlan-tpid-index", vlan_edit->svlan_tpid_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-aware");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->edit_flag, CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pass-through");
    if (0xFF != index)
    {
        CTC_SET_FLAG(vlan_edit->flag, CTC_VLAN_NH_PASS_THROUGH);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_xlate,
        ctc_cli_nh_add_xlate_cmd,
        "nexthop add egs-vlan-edit NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (port GPORT_ID) ("CTC_CLI_NH_EGS_VLAN_EDIT_STR ") \
        {service-queue | length-adjust-en | horizon-split|loop-nhid NHID|stats STATS_ID|mtu-check-en mtu-size MTU | logic-dest-port LOGIC_PORT |cid CID|service-id SERVICE|logic-port-check|}" ,
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_EGS_VLANTRANS_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_NH_EGS_VLAN_EDIT_DESC,
        "Service queue",
        "Enable adjust length for shaping",
        "Enable horizon split in VPLS networks",
        "Loopback nexthop id",
        CTC_CLI_NH_ID_STR,
        "Statistics",
        "0~0xFFFFFFFF",
        "Mtu check enable",
        "Mtu size",
        "Mtu size value",
        "Logic port",
        "Logic Port value",
        "Cid",
        "Cid value",
        "Serive id",
        "Serive id value",
        "Enable check logic port")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint32 gport = 0xFFFF;
    ctc_vlan_egress_edit_info_t edit_info;
    uint8 idx1;
    ctc_vlan_edit_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    sal_memset(&edit_info, 0, sizeof(ctc_vlan_egress_edit_info_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    idx1 = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != idx1)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[idx1 + 1]);
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx1)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[idx1 + 1]);
    }

    ret = _ctc_vlan_parser_egress_vlan_edit(&edit_info, &argv[0], argc);

    idx1 = CTC_CLI_GET_ARGC_INDEX("service-queue");
    if (0xFF != idx1)
    {
        edit_info.flag |= CTC_VLAN_NH_SERVICE_QUEUE_FLAG;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("length-adjust-en");
    if (0xFF != idx1)
    {
        edit_info.flag |= CTC_VLAN_NH_LENGTH_ADJUST_EN;
    }


    idx1 = CTC_CLI_GET_ARGC_INDEX("horizon-split");
    if (0xFF != idx1)
    {
        edit_info.flag |= CTC_VLAN_NH_HORIZON_SPLIT_EN;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("loop-nhid");
    if (0xFF != idx1)
    {
        CTC_CLI_GET_UINT16("loop Nexthop ID", edit_info.loop_nhid, argv[idx1 + 1]);
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(idx1))
    {
        CTC_CLI_GET_UINT32("stats-id", edit_info.stats_id, argv[idx1 + 1]);
        edit_info.flag |= CTC_VLAN_NH_STATS_EN;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("mtu-check-en");
    if (INDEX_VALID(idx1))
    {
        CTC_CLI_GET_UINT32("mtu-size", edit_info.mtu_size, argv[idx1 + 2]);
        edit_info.flag |= CTC_VLAN_NH_MTU_CHECK_EN;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("logic-dest-port");
    if (INDEX_VALID(idx1))
    {
        nh_param.logic_port_valid = 1;
        CTC_CLI_GET_UINT32("logic-port", nh_param.logic_port, argv[idx1 + 1]);
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("cid");
    if (INDEX_VALID(idx1))
    {
        CTC_CLI_GET_UINT16_RANGE("cid", nh_param.cid, argv[idx1+1], 0, CTC_MAX_UINT16_VALUE);;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (INDEX_VALID(idx1))
    {
        CTC_CLI_GET_UINT16("service-id", nh_param.service_id, argv[idx1 + 1]);
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("logic-port-check");
    if (INDEX_VALID(idx1))
    {
        nh_param.logic_port_check = 1;
    }

    nh_param.dsnh_offset = dsnh_offset;
    nh_param.gport_or_aps_bridge_id = gport;
    nh_param.vlan_edit_info = edit_info;

    if (CLI_ERROR == ret)
    {
        return CLI_ERROR;
    }

    idx1 = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != idx1)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[idx1 + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_XLATE, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_xlate(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_xlate(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_xlate,
        ctc_cli_nh_remove_xlate_cmd,
        "nexthop remove egs-vlan-edit NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_EGS_VLANTRANS_STR,
        CTC_CLI_NH_ID_STR)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid;

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_xlate(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_xlate(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_get_l2uc,
        ctc_cli_nh_get_l2uc_cmd,
        "show nexthop brguc port GPORT (bypass |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "L2Uc nexthop",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Bypass nexthop")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0;
    uint32 gport = 0;
    uint32 sub_type = 0;

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);

    if (2 == argc)
    {
        sub_type = CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS;
    }
    else
    {
        sub_type = CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_get_l2uc(g_api_lchip, gport,sub_type, &nhid);
    }
    else
    {
        ret = ctc_nh_get_l2uc(gport,sub_type, &nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("Gport:0x%.4x L2Uc Nexthop Id:%u\n", gport, nhid);
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_aps_xlate,
        ctc_cli_nh_add_aps_xlate_cmd,
        "nexthop add aps-egs-vlan-edit NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (aps-bridge-id APS_BRIDGE_ID) " \
        "(working-path "CTC_CLI_NH_EGS_VLAN_EDIT_STR " )" \
        "((protection-path "CTC_CLI_NH_EGS_VLAN_EDIT_STR ")|)" "{logic-port-check|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_APS_EGS_VLANTRANS_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_APS_BRIDGE_ID_STR,
        CTC_CLI_APS_BRIDGE_ID_DESC,
        CTC_CLI_NH_APS_WORKING_PATH_DESC,
        CTC_CLI_NH_EGS_VLAN_EDIT_DESC,
        CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
        CTC_CLI_NH_EGS_VLAN_EDIT_DESC,
        "Enable check logic port")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint16  aps_bridge_id = 0;
    uint8 tmp_argi, tmp_argj, argn;
    ctc_vlan_egress_edit_info_t edit_info_w;
    ctc_vlan_egress_edit_info_t edit_info_p;
    ctc_vlan_edit_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    sal_memset(&edit_info_w, 0, sizeof(ctc_vlan_egress_edit_info_t));
    sal_memset(&edit_info_p, 0, sizeof(ctc_vlan_egress_edit_info_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("aps-bridge-id", aps_bridge_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF != tmp_argi)
    {
        tmp_argj = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (0xFF != tmp_argj)
        {
            argn = tmp_argj - tmp_argi;
            ret = _ctc_vlan_parser_egress_vlan_edit(&edit_info_w, &argv[tmp_argi], argn);
            ret = ret ? ret : _ctc_vlan_parser_egress_vlan_edit(&edit_info_p, &argv[tmp_argj], (argc - tmp_argj));
        }
        else
        {
            argn = argc - tmp_argi;
            ret = _ctc_vlan_parser_egress_vlan_edit(&edit_info_w, &argv[tmp_argi], argn);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("logic-port-check");
    if (0xFF != tmp_argi)
    {
        nh_param.logic_port_check = 1;
    }

    nh_param.aps_en = TRUE;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.gport_or_aps_bridge_id = aps_bridge_id;
    nh_param.vlan_edit_info = edit_info_w;
    nh_param.vlan_edit_info_p = edit_info_p;

    if (CLI_ERROR == ret)
    {
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_XLATE, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_xlate(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_xlate(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_aps_xlate,
        ctc_cli_nh_remove_aps_xlate_cmd,
        "nexthop remove aps-egs-vlan-edit NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_APS_EGS_VLANTRANS_STR,
        CTC_CLI_NH_ID_STR)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid;

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_xlate(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_xlate(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_ipuc,
        ctc_cli_nh_add_ipuc_cmd,
        "nexthop add ipuc NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (unrov | fwd "CTC_CLI_MAC_ARP_STR "{macsa MAC|})"CTC_CLI_L3IF_ALL_STR "(loop-nhid NHID |)"\
        "((protection-path (aps-bridge-id APS_BRIDGE_ID) "CTC_CLI_MAC_ARP_STR CTC_CLI_L3IF_ALL_STR ")|) {no-ttl-dec| strip-mac | fpma |mtu-no-chk|cid CID|merge-dsfwd|adjust-length LEN|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_IPUC_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Unresolved nexthop",
        "Forward Nexthop",
        CTC_CLI_MAC_ARP_DESC,
        "Update inner source MAC address",
        "Inner source MAC address",
        CTC_CLI_L3IF_ALL_DESC,
        "Loopback nexthop id",
        CTC_CLI_NH_ID_STR,
        "Protection path for aps",
        CTC_CLI_APS_BRIDGE_ID_STR,
        CTC_CLI_APS_BRIDGE_ID_DESC,
        CTC_CLI_MAC_ARP_DESC,
        CTC_CLI_L3IF_ALL_DESC,
        "TTL no dec",
        "Strip MAC and vlan",
        "FPMA mode for FCoE MACDA",
        "Disable MTU check",
        "Cid",
        "Cid value",
        "Merge Dsfwd",
        "for qos shape adjusting packet length",
        "LEN")
{
    uint32 dsnh_offset = 0;
    uint32 nhid = 0;
    int32 ret = 0;
    mac_addr_t mac;
    ctc_nh_oif_info_t oif;
    uint8 index = 0;
    uint8 tmp_argi = 0;

    ctc_ip_nh_param_t nh_param = {0};

    sal_memset(&mac, 0, sizeof(mac_addr_t));
    sal_memset(&oif, 0, sizeof(ctc_nh_oif_info_t));
    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");

    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("Nexthop offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("no-ttl-dec");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_TTL_NO_DEC);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("strip-mac");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_STRIP_MAC);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fpma");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_FPMA);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-no-chk");
    if (0xFF != tmp_argi)
    {
        nh_param.mtu_no_chk = 1;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("merge-dsfwd");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_MERGE_DSFWD);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
         uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_IPUC, &nh_param));
        return CLI_SUCCESS;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, oif.gport, oif.vid);

        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));

        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_UNROV);
        nh_param.dsnh_offset = dsnh_offset;
	}

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        index = tmp_argi;

        tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("mac", index);
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[index + tmp_argi + 1]);
            CTC_CLI_NH_PARSE_L3IF(tmp_argi, oif.gport, oif.vid);
        }

        tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("arp-id", index);
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("arp", nh_param.arp_id, argv[index + tmp_argi + 1]);
            CTC_CLI_NH_PARSE_L3IF(tmp_argi, oif.gport, oif.vid);
        }

        tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("macsa", index);
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac sa address", nh_param.mac_sa, argv[index + tmp_argi + 1]);
        }

        /*vlan port if*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-port");
        if (0xFF != tmp_argi)
        {
            oif.is_l2_port = 1;
        }
        sal_memcpy(nh_param.mac, mac, sizeof(mac_addr_t));
        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));

        nh_param.dsnh_offset = dsnh_offset;

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("loop-nhid");
        if (tmp_argi != 0xFF)
        {
            CTC_CLI_GET_UINT32("loop nhid", nh_param.loop_nhid , argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("cid");
        if (tmp_argi != 0xFF)
        {
            CTC_CLI_GET_UINT16("cid", nh_param.cid, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT8("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (0xFF != tmp_argi)
        {
            index = tmp_argi;
            nh_param.aps_en = 1;

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT16("aps-bridge-id", nh_param.aps_bridge_group_id, argv[tmp_argi + 1]);
            }

            tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("mac", index);
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_MAC_ADDRESS("mac address", nh_param.p_mac, argv[index + tmp_argi + 1]);
                CTC_CLI_NH_PARSE_L3IF(tmp_argi + index , nh_param.p_oif.gport, nh_param.p_oif.vid);
            }

            tmp_argi = CTC_CLI_GET_SPECIFIC_INDEX("arp-id", index);
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT16("arp", nh_param.p_arp_id, argv[index+ tmp_argi + 1]);
                CTC_CLI_NH_PARSE_L3IF(tmp_argi + index, nh_param.p_oif.gport, nh_param.p_oif.vid);
            }

        }

    }
   if(g_ctcs_api_en)
   {
       ret = ctcs_nh_add_ipuc(g_api_lchip, nhid, &nh_param);
   }
   else
   {
      ret = ctc_nh_add_ipuc(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_ipuc,
        ctc_cli_nh_remove_ipuc_cmd,
        "nexthop remove ipuc NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_IPUC_STR,
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_ipuc(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_ipuc(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_update_ipuc,
        ctc_cli_nh_update_ipuc_cmd,
        "nexthop update ipuc NHID (fwd-attr "CTC_CLI_MAC_ARP_STR CTC_CLI_L3IF_ALL_STR " |fwd2unrov|unrov2fwd (dsnh-offset OFFSET|) "CTC_CLI_MAC_ARP_STR CTC_CLI_L3IF_ALL_STR "{macsa MAC|}){no-ttl-dec| strip-mac | fpma |mtu-no-chk|cid CID|merge-dsfwd|adjust-length LEN|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_IPUC_STR,
        CTC_CLI_NH_ID_STR,
        "Forward attribute(include port, vid and mac)",
        CTC_CLI_MAC_ARP_DESC,
        CTC_CLI_L3IF_ALL_DESC,
        "Forward nexthop to unresolved nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_MAC_ARP_DESC,
        CTC_CLI_L3IF_ALL_DESC,
        "Update inner source MAC address",
        "Inner source MAC address",
        "TTL no dec",
        "Strip MAC and vlan",
        "FPMA mode for FCoE MACDA",
        "Disable MTU check",
        "Cid",
        "Cid value",
        "Merge Dsfwd",
        "for qos shape adjusting packet length",
        "LEN")

{
    uint32 nhid;
    int32 ret;
    mac_addr_t mac;
    ctc_nh_oif_info_t oif;
    ctc_ip_nh_param_t nh_param = {0};
    uint8 tmp_argi = 0;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);
    sal_memset(&mac, 0, sizeof(mac_addr_t));
    sal_memset(&oif, 0, sizeof(ctc_nh_oif_info_t));

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("no-ttl-dec");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_TTL_NO_DEC);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("strip-mac");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_STRIP_MAC);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fpma");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_FPMA);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-no-chk");
    if (0xFF != tmp_argi)
    {
        nh_param.mtu_no_chk = 1;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("cid");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("cid", nh_param.cid, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("merge-dsfwd");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_MERGE_DSFWD);
    }

    switch (argv[1][3])
    {
    case '-':
        if (0 == sal_memcmp(argv[2], "m", 1))
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[3]);
        }
        else
        {
            CTC_CLI_GET_UINT16("arp", nh_param.arp_id, argv[3]);
        }
        CTC_CLI_NH_PARSE_L3IF(0,oif.gport, oif.vid);

        /*vlan port if*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-port");
        if (0xFF != tmp_argi)
        {
            oif.is_l2_port = 1;
        }

        sal_memcpy(nh_param.mac, mac, sizeof(mac_addr_t));
        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));

        nh_param.upd_type = CTC_NH_UPD_FWD_ATTR;
        if(g_ctcs_api_en)
        {
             ret = ctcs_nh_update_ipuc(g_api_lchip, nhid, &nh_param);
        }
        else
        {
            ret = ctc_nh_update_ipuc(nhid, &nh_param);
        }

        break;

    case 'o':
        if (0 == sal_memcmp(argv[2], "m", 1))
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[3]);
        }
        else
        {
            CTC_CLI_GET_UINT16("arp", nh_param.arp_id, argv[3]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("macsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac sa address", nh_param.mac_sa, argv[tmp_argi+1]);
        }

        CTC_CLI_NH_PARSE_L3IF(0, oif.gport, oif.vid);

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("DsNexthop Table offset", nh_param.dsnh_offset, argv[tmp_argi + 1]);
        }

        /*vlan port if*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-port");
        if (0xFF != tmp_argi)
        {
            oif.is_l2_port = 1;
        }
        sal_memcpy(nh_param.mac, mac, sizeof(mac_addr_t));
        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));

        nh_param.upd_type = CTC_NH_UPD_UNRSV_TO_FWD;
        if(g_ctcs_api_en)
        {
             ret = ctcs_nh_update_ipuc(g_api_lchip, nhid, &nh_param);
        }
        else
        {
            ret = ctc_nh_update_ipuc(nhid, &nh_param);
        }

        break;

    case '2':

        nh_param.upd_type = CTC_NH_UPD_FWD_TO_UNRSV;
        if(g_ctcs_api_en)
        {
             ret = ctcs_nh_update_ipuc(g_api_lchip, nhid, &nh_param);
        }
        else
        {
            ret = ctc_nh_update_ipuc(nhid, &nh_param);
        }

        break;

    default:
        ctc_cli_out("%% Invalid Parameters\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_NH_MPLS_PUSH_PARAM_STR  "payload-op (op-none|op-route|op-l2vpn "CTC_CLI_NH_EGS_VLAN_EDIT_STR " (mac-da MAC mac-sa MAC |))\
     (tunnel-id TUNNEL_ID|) (label1 LABLE ttl1 TTL exp1 EXP exp1-type EXP_TYPE (exp1-domain DOMAIN |)(map-ttl1|) (is-mcast1|)|) \
     (label2 LABLE ttl2 TTL exp2 EXP exp2-type EXP_TYPE (exp2-domain DOMAIN |)(map-ttl2|) (is-mcast2|)|) \
     {martini-cw (no-seq|per-pw-seq seq-index INDEX|glb-seq0|glb-seq1|cw CONTROL_WORD)|  \
      mtu-check-en mtu-size MTU | stats STATS_ID | adjust-length LEN |loop-nhid NHID|}"

#define CTC_CLI_NH_MPLS_PUSH_PARAM_DESC      "Mpls push nexthop payload operation type",   \
    "Payload could be ip, mpls or ethernet(Swap label on LSR|Pop label and do label Swap on LER)", \
    "Payload could be ip, will decreace ip header's ttl (L3VPN|FTN)",           \
    "Payload could be ethernet, will edit ethernet's vlan tag (L2VPN)",     \
    CTC_CLI_NH_EGS_VLAN_EDIT_DESC,                                 \
    "Inner MAC da",\
    CTC_CLI_MAC_FORMAT,\
    "Inner MAC sa",\
    CTC_CLI_MAC_FORMAT,\
    CTC_CLI_NH_MPLS_TUNNEL,                                            \
    CTC_CLI_NH_MPLS_TUNNEL_ID,                                               \
    "MPLS label1",                                                  \
    "MPLS label1 valule",                                           \
    "MPLS label1's ttl",                                            \
    "MPLS label1's ttl value(0~255)",                                      \
    "MPLS EXP1",                                                    \
    "MPLS EXP1 value(0-7)",                                              \
    "MPLS EXP1 type(0-2)",                                               \
    "MPLS EXP1 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP1 domain",                                                                              \
    "MPLS EXP1 domain ID",                                                                           \
    "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label1 is mcast label",                                   \
    "MPLS label2",                                                  \
    "MPLS label2 valule",                                           \
    "MPLS label2's ttl",                                            \
    "MPLS label2's ttl value(0~255)",                               \
    "MPLS EXP2",                                                    \
    "MPLS EXP2 value(0-7)",                                         \
    "MPLS EXP2 type(0-2)",                                          \
    "MPLS EXP2 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP2 domain",                                                                              \
    "MPLS EXP2 domain ID",                                                                           \
    "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label2 is mcast label",                                   \
    "First label is martini control word",                          \
    "Martini sequence type is none",                                \
    "Martini sequence type is per-pw",                              \
    "Martini per-pw sequence index",                                \
    "Sequence index value",                                         \
    "Martini sequence type is global sequence type0",               \
    "Martini sequence type is global sequence type1",               \
    "Control word",                                                 \
    "Control word value",                                           \
    "Mtu check enable",                                             \
    "Mtu size",                                                     \
    "Mtu size value",                                    \
    "Statistics",                                                   \
    CTC_CLI_STATS_ID_VAL,                                           \
    CTC_CLI_NH_MPLS_ADJ_LEN_DESC,                                   \
    "Loop nhid",                                          \
    CTC_CLI_NH_ID_STR

#define CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_STR \
    CTC_CLI_MAC_ARP_STR CTC_CLI_L3IF_ALL_STR "{macsa MAC|}  (label1 LABLE ttl1 TTL exp1 EXP exp1-type EXP_TYPE (exp1-domain DOMAIN|)(map-ttl1|) (is-mcast1|)|) \
    (label2 LABLE ttl2 TTL exp2 EXP exp2-type EXP_TYPE (exp2-domain DOMAIN|)(map-ttl2|) (is-mcast2|)|) \
    (label3 LABLE ttl3 TTL exp3 EXP exp3-type EXP_TYPE (exp3-domain DOMAIN|)(map-ttl3|) (is-mcast3|)|) \
    (label4 LABLE ttl4 TTL exp4 EXP exp4-type EXP_TYPE (exp4-domain DOMAIN|)(map-ttl4|) (is-mcast4|)|) \
    (label5 LABLE ttl5 TTL exp5 EXP exp5-type EXP_TYPE (exp5-domain DOMAIN|)(map-ttl5|) (is-mcast5|)|) \
    (label6 LABLE ttl6 TTL exp6 EXP exp6-type EXP_TYPE (exp6-domain DOMAIN|)(map-ttl6|) (is-mcast6|)|) \
    (label7 LABLE ttl7 TTL exp7 EXP exp7-type EXP_TYPE (exp7-domain DOMAIN|)(map-ttl7|) (is-mcast7|)|) \
    (label8 LABLE ttl8 TTL exp8 EXP exp8-type EXP_TYPE (exp8-domain DOMAIN|)(map-ttl8|) (is-mcast8|)|) \
    (label9 LABLE ttl9 TTL exp9 EXP exp9-type EXP_TYPE (exp9-domain DOMAIN|)(map-ttl9|) (is-mcast9|)|) \
    (label10 LABLE ttl10 TTL exp10 EXP exp10-type EXP_TYPE (exp10-domain DOMAIN|)(map-ttl10|) (is-mcast10|)|) \
    (label11 LABLE ttl11 TTL exp11 EXP exp11-type EXP_TYPE (exp11-domain DOMAIN|)(map-ttl11|) (is-mcast11|)|) \
    (label12 LABLE ttl12 TTL exp12 EXP exp12-type EXP_TYPE (exp12-domain DOMAIN|)(map-ttl12|) (is-mcast12|)|) \
    (is-sr|)(stats STATS_ID|)(loop-nhid NHID|)"

#define CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC\
    "MPLS label",                                                  \
    "MPLS label valule",                                           \
    "MPLS label's ttl",                                            \
    "MPLS label's ttl value(0~255)",                                      \
    "MPLS EXP",                                                    \
    "MPLS EXP value(0-7)",                                              \
    "MPLS EXP type(0-2)",                                              \
    "MPLS EXP type value,0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP domain",                                                                              \
    "MPLS EXP domain ID",                                                                           \
    "MPLS label's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label is mcast label"

#define CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_DESC   CTC_CLI_MAC_ARP_DESC,    \
    CTC_CLI_L3IF_ALL_DESC,                                          \
    "Update inner source MAC address", \
    "Inner source MAC address", \
    "MPLS label1",                                                  \
    "MPLS label1 valule",                                           \
    "MPLS label1's ttl",                                            \
    "MPLS label1's ttl value(0~255)",                                      \
    "MPLS EXP1",                                                    \
    "MPLS EXP1 value(0-7)",                                              \
    "MPLS EXP1 type(0-2)",                                              \
    "MPLS EXP1 type value,0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP1 domain",                                                                              \
    "MPLS EXP1 domain ID",                                                                           \
    "MPLS label1's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label1 is mcast label",                                   \
    "MPLS label2",                                                  \
    "MPLS label2 valule",                                           \
    "MPLS label2's ttl",                                            \
    "MPLS label2's ttl value(0~255)",                                      \
    "MPLS EXP2",                                                    \
    "MPLS EXP2 value(0-7)",                                              \
    "MPLS EXP2 type(0-2)",                                               \
    "MPLS EXP2 type value, 0:user-define EXP to outer header; 1 Use EXP value from EXP map; 2: Copy packet EXP to outer header", \
    "MPLS EXP2 domain",                                                                              \
    "MPLS EXP2 domain ID",                                                                           \
    "MPLS label2's ttl mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "MPLS label2 is mcast label",                                   \
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    CTC_CLI_NH_MPLS_TUNNEL_LABEL_DESC,\
    "MPLS segment route",\
    "MPLS label stats enable",                                      \
    CTC_CLI_STATS_ID_VAL, \
    "Loopback nexthop id",   \
    CTC_CLI_NH_ID_STR

#define CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(__label_param, __tmp_argi,  __label_str, __map_ttl_str, __is_mcast_str,__exp_domain_str)     \
    do {                                                                                                         \
        __tmp_argi = CTC_CLI_GET_ARGC_INDEX(__label_str);             \
        if (0xFF != __tmp_argi)                                                                                 \
        { \
            CTC_SET_FLAG(__label_param.lable_flag, CTC_MPLS_NH_LABEL_IS_VALID);                 \
            CTC_CLI_GET_UINT32_RANGE(__label_str, __label_param.label, argv[(__tmp_argi + 1)], 0, CTC_MAX_UINT32_VALUE);       \
            CTC_CLI_GET_UINT8_RANGE("ttl", __label_param.ttl, argv[(__tmp_argi + 3)], 0, CTC_MAX_UINT8_VALUE);           \
            CTC_CLI_GET_UINT8_RANGE("exp", __label_param.exp, argv[(__tmp_argi + 5)], 0, CTC_MAX_UINT8_VALUE);              \
            CTC_CLI_GET_UINT8_RANGE("exp-type", __label_param.exp_type, argv[(__tmp_argi + 7)], 0, CTC_MAX_UINT8_VALUE);    \
            __tmp_argi = CTC_CLI_GET_ARGC_INDEX(__map_ttl_str);     \
            if (0xFF != __tmp_argi)                                                                             \
            {                                                                                                   \
                CTC_SET_FLAG(__label_param.lable_flag, CTC_MPLS_NH_LABEL_MAP_TTL);              \
            }                                                                                                   \
            __tmp_argi = CTC_CLI_GET_ARGC_INDEX(__is_mcast_str);   \
            if (0xFF != __tmp_argi)                                                                             \
            {                                                                                                   \
                CTC_SET_FLAG(__label_param.lable_flag, CTC_MPLS_NH_LABEL_IS_MCAST);             \
            }                                                                                                   \
            __tmp_argi = CTC_CLI_GET_ARGC_INDEX(__exp_domain_str);\
           if (0xFF != __tmp_argi)\
           {\
               CTC_CLI_GET_UINT8("exp domain", __label_param.exp_domain, argv[__tmp_argi + 1]);\
           }\
            label_num++; \
            arrayi++; \
        }                                                                                                       \
    } while (0);

#define CTC_CLI_NH_MULTI_PARSE_MPLS_LABEL_PARAM(tunnel_label, __tmp_argi, lable_num)     \
    do {                                                                                                         \
        uint8 lable_i = 0;\
        char label_str[16] = {0};\
        char map_ttl_str[16] = {0};\
        char is_mcast_str[16] = {0};\
        char exp_domain_str[16] = {0};\
        for (lable_i = 0; lable_i < lable_num; lable_i++)\
        {\
            sal_sprintf(label_str, "%s%d", "label", lable_i + 1);\
            sal_sprintf(map_ttl_str, "%s%d", "map-ttl", lable_i + 1);\
            sal_sprintf(is_mcast_str, "%s%d", "is-mcast", lable_i + 1);\
            sal_sprintf(exp_domain_str, "%s%d%s", "exp", lable_i + 1,"-domain");\
            CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(tunnel_label[arrayi], __tmp_argi,  label_str, map_ttl_str, is_mcast_str, exp_domain_str);\
        }                                                                                              \
    } while (0);

STATIC int32
_ctc_nexthop_cli_parse_mpls_push_nexthop(char** argv, uint16 argc,
                                         ctc_mpls_nexthop_push_param_t* p_nhinfo)
{
    uint8 tmp_argi, arrayi = 0;
    uint8 label_num = 0;
    int32 ret = CLI_SUCCESS;
    mac_addr_t mac;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("op-none");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_NONE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("op-route");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_ROUTE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("op-l2vpn");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;
        ret = _ctc_vlan_parser_egress_vlan_edit(&p_nhinfo->nh_com.vlan_info, &argv[tmp_argi], (argc - tmp_argi));

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-da");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
            sal_memcpy(p_nhinfo->nh_com.mac, mac, sizeof(mac_addr_t));
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-sa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
            sal_memcpy(p_nhinfo->nh_com.mac_sa, mac, sizeof(mac_addr_t));
        }

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("tunnel-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("mtu size", p_nhinfo->tunnel_id, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("martini-cw");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->martini_encap_valid   = TRUE;
        p_nhinfo->martini_encap_type    = 0;

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("cw");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->martini_encap_type = 1;
            CTC_CLI_GET_UINT32("Martini label, control word", p_nhinfo->seq_num_index, argv[(tmp_argi + 1)]);
        }

    }

    CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(p_nhinfo->push_label[arrayi], tmp_argi,  "label1", "map-ttl1", "is-mcast1", "exp1-domain");
    CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(p_nhinfo->push_label[arrayi], tmp_argi,  "label2", "map-ttl2", "is-mcast2", "exp2-domain");
    p_nhinfo->label_num = label_num;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-check-en");
    if (0xFF != tmp_argi)
    {
        uint16 mtu_size;
        CTC_CLI_GET_UINT16("mtu size", mtu_size, argv[(tmp_argi + 2)]);
        p_nhinfo->mtu_size = mtu_size;
        p_nhinfo->mtu_check_en = TRUE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->stats_valid = 1;
        CTC_CLI_GET_UINT32("stats id", p_nhinfo->stats_id, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("loop-nhid");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("loop nhid", p_nhinfo->loop_nhid, argv[(tmp_argi + 1)]);
    }
    return ret;
}

STATIC int32
_ctc_nexthop_cli_parse_mpls_tunnel_label(char** argv, uint16 argc,
                                         ctc_mpls_nexthop_tunnel_info_t* p_tunnel_param)
{
    uint8 tmp_argi, arrayi = 0;
    uint8 label_num = 0;
    int ret = CLI_SUCCESS;
    mac_addr_t mac;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != tmp_argi)
    {

        /*mac MAC (sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) payload-op (op-none|op-route|op-l2vpn ...*/
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
        sal_memcpy(p_tunnel_param->mac, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("arp-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("arp id", p_tunnel_param->arp_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac sa address", mac, argv[tmp_argi + 1]);
        sal_memcpy(p_tunnel_param->mac_sa, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_tunnel_param->oif.gport, p_tunnel_param->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_tunnel_param->oif.gport, p_tunnel_param->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("routed-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_tunnel_param->oif.gport, p_tunnel_param->oif.vid);
    }

    CTC_CLI_NH_MULTI_PARSE_MPLS_LABEL_PARAM(p_tunnel_param->tunnel_label, tmp_argi, CTC_MPLS_NH_MAX_TUNNEL_LABEL_NUM);
    p_tunnel_param->label_num = label_num;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-sr");
    if (0xFF != tmp_argi)
    {
        p_tunnel_param->is_sr = 1;
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        p_tunnel_param->stats_valid = 1;
        CTC_CLI_GET_UINT32("stats id", p_tunnel_param->stats_id, argv[(tmp_argi + 1)]);
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("loop-nhid");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("loop nhid", p_tunnel_param->loop_nhid, argv[(tmp_argi + 1)]);
    }
    return ret;
}

CTC_CLI(ctc_cli_nh_add_mpls_tunnel_label,
        ctc_cli_nh_add_mpls_tunnel_label_cmd,
        "nexthop (add|update) (mpls-tunnel TUNNEL_ID) "\
        "(aps-bridge-id APS_BRIDGE_ID (aps-level2 (working-lsp|protecting-lsp)|)|) "\
        "(working-path (is-spme|) "CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_STR ") "\
        "(protection-path "CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_STR " |)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MPLS_TUNNEL,
        CTC_CLI_NH_MPLS_TUNNEL_ID,
        "Aps bridge id, effective when tunnel has protection path",
        CTC_CLI_APS_BRIDGE_ID_DESC,
        "LSP+SPME 2 level aps",
        "This tunnel is working lsp",
        "This tunnel is protecting lsp",
        CTC_CLI_NH_APS_WORKING_PATH_DESC,
        "the tunnel is spme",
        CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_DESC,
        CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
        CTC_CLI_NH_MPLS_TUNNEL_LABEL_PARAM_DESC)
{
    ctc_mpls_nexthop_tunnel_param_t    nh_tunnel_param;
    uint16 tunnel_id = 0;
    int32 ret  = CLI_SUCCESS;
    uint8 tmp_argi = 0;
    bool   add = TRUE;
    uint32 working_end = 0;

    sal_memset(&nh_tunnel_param, 0, sizeof(nh_tunnel_param));

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != tmp_argi)
    {
        add = FALSE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mpls-tunnel");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("APS bridge id", tunnel_id, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("APS bridge id", nh_tunnel_param.aps_bridge_group_id, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("aps-level2");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_tunnel_param.aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("protecting-lsp");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_tunnel_param.aps_flag, CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION);
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("protection-path");
    if (0xFF != tmp_argi)
    {
        /* nh_tunnel_param.aps_en  = TRUE; */
        CTC_SET_FLAG(nh_tunnel_param.aps_flag, CTC_NH_MPLS_APS_EN);
        ret = _ctc_nexthop_cli_parse_mpls_tunnel_label(&(argv[tmp_argi]), argc - tmp_argi, &nh_tunnel_param.nh_p_param);
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        working_end = tmp_argi;
    }
    else
    {
        working_end = argc;
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF != tmp_argi)
    {
        ret = _ctc_nexthop_cli_parse_mpls_tunnel_label(&(argv[tmp_argi]), working_end - tmp_argi, &nh_tunnel_param.nh_param);
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-spme");
        if (0xFF != tmp_argi)
        {
            nh_tunnel_param.nh_param.is_spme = 1;
        }

    }

    if (add)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_add_mpls_tunnel_label(g_api_lchip, tunnel_id, &nh_tunnel_param);
        }
        else
        {
            ret = ctc_nh_add_mpls_tunnel_label(tunnel_id, &nh_tunnel_param);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_update_mpls_tunnel_label(g_api_lchip, tunnel_id, &nh_tunnel_param);
        }
        else
        {
            ret = ctc_nh_update_mpls_tunnel_label(tunnel_id, &nh_tunnel_param);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_swap_mpls_tunnel,
        ctc_cli_nh_swap_mpls_tunnel_cmd,
        "nexthop mpls-tunnel swap (source TUNNEL_ID) (destination TUNNEL_ID)",
        CTC_CLI_NH_M_STR,
        "mpls tunnel",
        "swap mpls tunnel",
        "Source Tunnel",
        CTC_CLI_NH_MPLS_TUNNEL_ID,
        "Destination Tunnel",
        CTC_CLI_NH_MPLS_TUNNEL_ID)
{
    uint16 old_tunnel_id = 0;
    uint16 new_tunnel_id = 0;
    int32 ret  = CLI_SUCCESS;
    uint8 tmp_argi = 0;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("source");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("Old tunnel id", old_tunnel_id, argv[(tmp_argi + 1)]);
        CTC_CLI_GET_UINT16("New tunnel id", new_tunnel_id, argv[(tmp_argi + 3)]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_swap_mpls_tunnel_label(g_api_lchip, old_tunnel_id, new_tunnel_id);
    }
    else
    {
        ret = ctc_nh_swap_mpls_tunnel_label(old_tunnel_id, new_tunnel_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_mpls_tunnel_label,
        ctc_cli_nh_remove_mpls_tunnel_label_cmd,
        "nexthop remove (mpls-tunnel TUNNEL_ID)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_MPLS_TUNNEL,
        CTC_CLI_NH_MPLS_TUNNEL_ID)
{
    uint16 tunnel_id = 0;
    int32 ret  = CLI_SUCCESS;
    uint8 tmp_argi = 0;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mpls-tunnel");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("APS bridge id", tunnel_id, argv[(tmp_argi + 1)]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_mpls_tunnel_label(g_api_lchip, tunnel_id);
    }
    else
    {
        ret = ctc_nh_remove_mpls_tunnel_label(tunnel_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_nh_add_mpls_push_nexthop,
        ctc_cli_nh_add_mpls_push_nexthop_cmd,
        "nexthop add mpls NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) push (unrov|fwd ((is-hvpls|) vpls-port LOGIC_PORT|) ("CTC_CLI_NH_MPLS_PUSH_PARAM_STR ")(service-queue|service-id SERVICE|))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MPLS_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Mpls push nexthop",
        "Unresolved nexthop",
        "Forward Nexthop",
        "Enable H-VPLS in VPLS network",
        "Destination logic port in VPLS network",
        "MPLS vpls port value",
        CTC_CLI_NH_MPLS_PUSH_PARAM_DESC,
        "Service queue",
        "Serive id",
        "Serive id value")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0, tmp_argi;
    ctc_mpls_nexthop_push_param_t nhinfo;
    ctc_mpls_nexthop_param_t nh_param = {0};

    sal_memset(&nhinfo, 0, sizeof(ctc_mpls_nexthop_push_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
       nh_param.flag |= CTC_MPLS_NH_IS_UNRSV;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        if (_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nhinfo))
        {
            return CLI_ERROR;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("service-queue");
        if (0xFF != tmp_argi)
        {
            CTC_SET_FLAG(nhinfo.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_SERVICE_QUEUE_EN);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-hvpls");
    if (0xFF != tmp_argi)
    {
       nh_param.flag |= CTC_MPLS_NH_IS_HVPLS;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vpls-port");
    if (0xFF != tmp_argi)
    {
       nh_param.logic_port_valid  = TRUE ;
        CTC_CLI_GET_UINT16("logic port", nh_param.logic_port, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.dsnh_offset = dsnh_offset;

    nh_param.nh_para.nh_param_push = nhinfo;
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MPLS, &nh_param));
        return CLI_SUCCESS;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("service-id", nh_param.service_id, argv[(tmp_argi + 1)]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_mpls(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_update_mpls_push_nexthop,
        ctc_cli_nh_update_mpls_push_nexthop_cmd,
        "nexthop update mpls NHID push ( fwd-attr|unrov2fwd (dsnh-offset OFFSET|)) ((is-hvpls|) vpls-port LOGIC_PORT|) ("CTC_CLI_NH_MPLS_PUSH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MPLS_STR,
        CTC_CLI_NH_ID_STR,
        "Mpls push nexthop",
        "Update forward nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        "Enable H-VPLS in VPLS network",
        "Destination logic port in VPLS network",
        "MPLS vpls port value",
        CTC_CLI_NH_MPLS_PUSH_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid;
    uint8 tmp_argi;
    ctc_mpls_nexthop_push_param_t nhinfo;
    ctc_mpls_nexthop_param_t nh_param = {0};

    sal_memset(&nh_param, 0, sizeof(nh_param));
    sal_memset(&nhinfo, 0, sizeof(ctc_mpls_nexthop_push_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    if (_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[1]), (argc - 1), &nhinfo))
    {
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov2fwd");
    if (0xFF != tmp_argi)
    {
        nh_param.upd_type  = CTC_NH_UPD_UNRSV_TO_FWD;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("DsNexthop Table offset", nh_param.dsnh_offset, argv[tmp_argi + 1]);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd-attr");
    if (0xFF != tmp_argi)
    {
        nh_param.upd_type  = CTC_NH_UPD_FWD_ATTR;

    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-hvpls");
    if (0xFF != tmp_argi)
    {
       nh_param.flag |= CTC_MPLS_NH_IS_HVPLS;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vpls-port");
    if (0xFF != tmp_argi)
    {
        nh_param.logic_port_valid  = TRUE ;
        CTC_CLI_GET_UINT16("logic port", nh_param.logic_port, argv[(tmp_argi + 1)]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.nh_para.nh_param_push = nhinfo;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_mpls(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_aps_mpls_push_nexthop,
        ctc_cli_nh_add_aps_mpls_push_nexthop_cmd,
        "nexthop add aps-mpls NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (aps-bridge-id APS_BRIDGE_ID) push ((is-hvpls|) vpls-port LOGIC_PORT|) "\
        "(working-path "CTC_CLI_NH_MPLS_PUSH_PARAM_STR ")" \
        "(protection-path  "CTC_CLI_NH_MPLS_PUSH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "APS MPLS nexthop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_APS_BRIDGE_ID_STR,
        CTC_CLI_APS_BRIDGE_ID_DESC,
        "Mpls push nexthop",
        "Enable H-VPLS in VPLS network",
        "Destination logic port in VPLS network",
        "MPLS vpls port value",
        CTC_CLI_NH_APS_WORKING_PATH_DESC,
        CTC_CLI_NH_MPLS_PUSH_PARAM_DESC,
        CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
        CTC_CLI_NH_MPLS_PUSH_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0, tmp_arg_w, tmp_arg_p, aps_bridge_id, tmp_argi = 0;
    ctc_mpls_nexthop_push_param_t nhinfo_w, nhinfo_p;
    ctc_mpls_nexthop_param_t nh_param = {0};

    sal_memset(&nhinfo_w, 0, sizeof(ctc_mpls_nexthop_push_param_t));
    sal_memset(&nhinfo_p, 0, sizeof(ctc_mpls_nexthop_push_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_arg_w)
    {
        CTC_CLI_GET_UINT32_RANGE("dsnh_offset", dsnh_offset, argv[tmp_arg_w + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    /*2. APS Bridge ID*/
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
    if (0xFF == tmp_arg_w)
    {
        ctc_cli_out("Invalid input\n");
    }
    CTC_CLI_GET_UINT32("APS bridge id", aps_bridge_id, argv[(tmp_arg_w + 1)]);

    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("is-hvpls");
    if (0xFF != tmp_arg_w)
    {
       nh_param.flag |= CTC_MPLS_NH_IS_HVPLS;
    }

    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("vpls-port");
    if (0xFF != tmp_arg_w)
    {
        nh_param.logic_port_valid  = TRUE ;
        CTC_CLI_GET_UINT16("logic port", nh_param.logic_port, argv[(tmp_arg_w + 1)]);
    }

    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF == tmp_arg_w)
    {
        ctc_cli_out("Invalid input\n");
    }
    else
    {
        tmp_arg_p = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (0xFF == tmp_arg_p)
        {
            if(_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_arg_w ]), (argc -tmp_arg_w ), &nhinfo_w))
            {
                return CLI_ERROR;
            }
        }
        else
        {
            if(_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_arg_w ]), (tmp_arg_p -tmp_arg_w ), &nhinfo_w))
            {
                return CLI_ERROR;
            }
            if (_ctc_nexthop_cli_parse_mpls_push_nexthop( &(argv[tmp_arg_p ]), (argc - tmp_arg_p), &nhinfo_p))
            {
                return CLI_ERROR;
            }
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.aps_en  = TRUE;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.aps_bridge_group_id = aps_bridge_id;
    nh_param.nh_para.nh_param_push = nhinfo_w;
    nh_param.nh_p_para.nh_p_param_push = nhinfo_p;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MPLS, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_mpls(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc( ret ));
        return CLI_ERROR;
    }
    return ret;
}
CTC_CLI(ctc_cli_nh_update_aps_mpls_push_nexthop,
    ctc_cli_nh_update_aps_mpls_push_nexthop_cmd,
    "nexthop update aps-mpls NHID (aps-bridge-id APS_BRIDGE_ID) push (fwd-attr|unrov2fwd) ((is-hvpls|) vpls-port LOGIC_PORT|) "\
    "(working-path "CTC_CLI_NH_MPLS_PUSH_PARAM_STR" )"\
    "(protection-path "CTC_CLI_NH_MPLS_PUSH_PARAM_STR"|) ",
    CTC_CLI_NH_M_STR,
    CTC_CLI_NH_UPDATE_STR,
    "APS MPLS nexthop",
    CTC_CLI_NH_ID_STR,
    CTC_CLI_APS_BRIDGE_ID_STR,
    CTC_CLI_APS_BRIDGE_ID_DESC,
    "Mpls push nexthop",
    "Update forward nexthop",
    "Unresolved nexthop to forward nexthop",
    "Enable H-VPLS in VPLS network",
    "Destination logic port in VPLS network",
    "MPLS vpls port value",
    CTC_CLI_NH_APS_WORKING_PATH_DESC,
    CTC_CLI_NH_MPLS_PUSH_PARAM_DESC,
    CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
    CTC_CLI_NH_MPLS_PUSH_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0, dsnh_offset = 0, tmp_arg_w = 0, tmp_arg_p = 0, aps_bridge_id = 0, tmp_argi = 0;
    ctc_mpls_nexthop_push_param_t nhinfo_w, nhinfo_p;
    ctc_mpls_nexthop_param_t nh_param = {0};
    sal_memset(&nhinfo_w, 0, sizeof(ctc_mpls_nexthop_push_param_t));
    sal_memset(&nhinfo_p, 0, sizeof(ctc_mpls_nexthop_push_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
    if (0xFF != tmp_arg_w)
    {
       CTC_CLI_GET_UINT32("APS bridge id", aps_bridge_id, argv[(tmp_arg_w + 1)]);
  	}
	tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("is-hvpls");
    if (0xFF != tmp_arg_w)
    {
       nh_param.flag |= CTC_MPLS_NH_IS_HVPLS;
    }

    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("vpls-port");
    if (0xFF != tmp_arg_w)
    {
        nh_param.logic_port_valid  = TRUE ;
        CTC_CLI_GET_UINT16("logic port", nh_param.logic_port, argv[(tmp_arg_w + 1)]);
    }
    /*3. Working path*/
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF == tmp_arg_w)
    {
        ctc_cli_out("Invalid input\n");
    }
    else
    {
        /*4. Protection path*/
        tmp_arg_p = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (0xFF == tmp_arg_p)
        {
            /*.parser working path param*/
            if (_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_arg_w ]), (argc - tmp_arg_w), &nhinfo_w))
            {
                return CLI_ERROR;
            }
        }
        else
        {
            /*.parser working path param*/
            if (_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_arg_w ]), (tmp_arg_p - tmp_arg_w), &nhinfo_w))
            {
                return CLI_ERROR;
            }

            /*.parser protection path param*/
            if (_ctc_nexthop_cli_parse_mpls_push_nexthop(&(argv[tmp_arg_p ]), (argc - tmp_arg_p), &nhinfo_p))
            {
                return CLI_ERROR;
            }
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    nh_param.upd_type  = CTC_NH_UPD_FWD_ATTR;
    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    nh_param.aps_en  = TRUE;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.aps_bridge_group_id = aps_bridge_id;
    nh_param.nh_para.nh_param_push = nhinfo_w;
    nh_param.nh_p_para.nh_p_param_push = nhinfo_p;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_mpls(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

STATIC int32
_ctc_nexthop_cli_parse_mpls_pop_nexthop(char** argv, uint16 argc,
                                        ctc_mpls_nexthop_pop_param_t* p_nhinfo)
{
    uint8 tmp_argi = 0;
    mac_addr_t mac;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("op-route");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->nh_com.opcode = CTC_MPLS_NH_PHP;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != tmp_argi)
    {
        tmp_argi++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi]);

        sal_memcpy(p_nhinfo->nh_com.mac, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("arp-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("arp id", p_nhinfo->arp_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->nh_com.oif.gport, p_nhinfo->nh_com.oif.vid);

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->nh_com.oif.gport, p_nhinfo->nh_com.oif.vid);

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("routed-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->nh_com.oif.gport, p_nhinfo->nh_com.oif.vid);

    }

    p_nhinfo->ttl_mode = CTC_MPLS_TUNNEL_MODE_UNIFORM;
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("pipe");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->ttl_mode = CTC_MPLS_TUNNEL_MODE_PIPE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("short-pipe");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->ttl_mode = CTC_MPLS_TUNNEL_MODE_SHORT_PIPE;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_mpls_pop_nexthop,
        ctc_cli_nh_add_mpls_pop_nexthop_cmd,
        "nexthop add mpls NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) pop (unrov|fwd ("CTC_CLI_NH_MPLS_POP_NH_PARAM_STR ") (service-queue|))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MPLS_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Mpls pop nexthop",
        "Unresolved nexthop",
        "Forward Nexthop",
        CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC,
        "Service queue")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint8 tmp_argi;
    ctc_mpls_nexthop_pop_param_t nhinfo;
    bool unresolved = FALSE;
    ctc_mpls_nexthop_param_t nh_param = {0};

    sal_memset(&nhinfo, 0, sizeof(ctc_mpls_nexthop_pop_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    /* CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[1]); */

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32_RANGE("dsnh_offset", dsnh_offset, argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        if (_ctc_nexthop_cli_parse_mpls_pop_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nhinfo))
        {
            ctc_cli_out("Parse command fail\n");
            return CLI_ERROR;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("service-queue");
        if (0xFF != tmp_argi)
        {
            CTC_SET_FLAG(nhinfo.nh_com.mpls_nh_flag, CTC_MPLS_NH_FLAG_SERVICE_QUEUE_EN);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
        unresolved = TRUE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    nh_param.nh_prop = CTC_MPLS_NH_POP_TYPE;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.flag = (unresolved) ? CTC_MPLS_NH_IS_UNRSV : 0;
    nh_param.nh_para.nh_param_pop = nhinfo;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MPLS, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_mpls(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_update_mpls_pop_nexthop,
        ctc_cli_nh_update_mpls_pop_nexthop_cmd,
        "nexthop update mpls NHID pop (fwd-attr|unrov2fwd (dsnh-offset OFFSET|)) ("CTC_CLI_NH_MPLS_POP_NH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MPLS_STR,
        CTC_CLI_NH_ID_STR,
        "Mpls pop nexthop",
        "Update forward nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid;
    uint8 tmp_argi;
    ctc_mpls_nexthop_param_t nh_param = {0};
    ctc_mpls_nexthop_pop_param_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(ctc_mpls_nexthop_pop_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("payload-op");
    if (0xFF == tmp_argi)
    {
        ctc_cli_out("Invalid input\n");
        return CLI_ERROR;
    }

    if (_ctc_nexthop_cli_parse_mpls_pop_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nhinfo))
    {
        ctc_cli_out("Parse command fail\n");
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov2fwd");
    if (0xFF != tmp_argi)
    {
        nh_param.nh_prop = CTC_MPLS_NH_POP_TYPE;
        nh_param.upd_type  = CTC_NH_UPD_UNRSV_TO_FWD;

        nh_param.nh_para.nh_param_pop = nhinfo;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("DsNexthop Table offset", nh_param.dsnh_offset, argv[tmp_argi + 1]);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &nh_param);
        }
        else
        {
            ret = ctc_nh_update_mpls(nhid, &nh_param);
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd-attr");
    if (0xFF != tmp_argi)
    {
        nh_param.nh_prop = CTC_MPLS_NH_POP_TYPE;
        nh_param.upd_type  = CTC_NH_UPD_FWD_ATTR;

        nh_param.nh_para.nh_param_pop = nhinfo;
        if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_mpls(nhid, &nh_param);
    }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_aps_mpls_pop_nexthop,
        ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd,
        "nexthop add aps-mpls NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (aps-bridge-id APS_BRIDGE_ID) pop (working-path "CTC_CLI_NH_MPLS_POP_NH_PARAM_STR \
        "protection-path "CTC_CLI_NH_MPLS_POP_NH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "Aps mpls nexthop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_APS_BRIDGE_ID_STR,
        CTC_CLI_APS_BRIDGE_ID_DESC,
        "Mpls pop nexthop",
        CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
        CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC,
        CTC_CLI_NH_APS_PROTECTION_PATH_DESC,
        CTC_CLI_NH_MPLS_POP_NH_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0, aps_bridge_id = 0;
    uint8 tmp_arg_w, tmp_arg_p, tmp_argi = 0;
    ctc_mpls_nexthop_pop_param_t nhinfo_w, nhinfo_p;
    ctc_mpls_nexthop_param_t nh_param = {0};

    sal_memset(&nhinfo_w, 0, sizeof(ctc_mpls_nexthop_pop_param_t));
    sal_memset(&nhinfo_p, 0, sizeof(ctc_mpls_nexthop_pop_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_arg_w)
    {
        CTC_CLI_GET_UINT32_RANGE("dsnh_offset", dsnh_offset, argv[tmp_arg_w + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    /*2. APS Bridge ID*/
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("aps-bridge-id");
    if (0xFF == tmp_arg_w)
    {
        ctc_cli_out("Invalid input\n");
    }

    CTC_CLI_GET_UINT32("APS bridge id", aps_bridge_id, argv[(tmp_arg_w + 1)]);

    /*3. Working path*/
    tmp_arg_w = CTC_CLI_GET_ARGC_INDEX("working-path");
    if (0xFF == tmp_arg_w)
    {
        ctc_cli_out("Invalid input\n");
    }

    /*4. Protection path*/
    tmp_arg_p = CTC_CLI_GET_ARGC_INDEX("protection-path");
    if (0xFF == tmp_arg_p)
    {
        ctc_cli_out("Invalid input\n");
    }

    if (_ctc_nexthop_cli_parse_mpls_pop_nexthop(&(argv[tmp_arg_w + 1]), (tmp_arg_p - (tmp_arg_w + 1)), &nhinfo_w))
    {
        return CLI_ERROR;
    }

    if (_ctc_nexthop_cli_parse_mpls_pop_nexthop(&(argv[tmp_arg_p + 1]), (argc - tmp_arg_p - 1), &nhinfo_p))
    {
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("adjust-length", nh_param.adjust_length, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    nh_param.nh_prop = CTC_MPLS_NH_POP_TYPE;
    nh_param.aps_en = TRUE;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.aps_bridge_group_id = aps_bridge_id;
    nh_param.nh_para.nh_param_pop = nhinfo_w;
    nh_param.nh_p_para.nh_p_param_pop = nhinfo_p;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MPLS, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_mpls(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_update_mpls_nexthop_to_unrov,
        ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd,
        "nexthop update mpls NHID fwd2unrov",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MPLS_STR,
        CTC_CLI_NH_ID_STR,
        "Forward nexthop to unresolved nexthop")
{
    uint32 nhid;
    int32 ret;
    ctc_mpls_nexthop_param_t nh_param = {0};

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    nh_param.nh_prop = CTC_MPLS_NH_NONE;
    nh_param.upd_type  = CTC_NH_UPD_FWD_TO_UNRSV;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mpls(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_mpls(nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_mpls_nexthop,
        ctc_cli_nh_remove_mpls_nexthop_cmd,
        "nexthop remove mpls NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        "Mpls or aps-mpls nexthop",
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_mpls(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_mpls(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_iloop,
        ctc_cli_nh_add_iloop_cmd,
        "nexthop add iloop NHID lpbk-lport LPORT_ID (customerid-valid (map-exp|)|) (inner-pkt-type TYPE |) (vpls-port|) (sequence-number SQN_INDEX |) (words-remove VALUE|) (service-queue-en |)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_ILOOP_STR,
        CTC_CLI_NH_ID_STR,
        "Loopback local phy port",
        "<0-255>",
        "CustomerId is valid(eg. VC label)",
        "Map priority and color from exp of VC label",
        "Specify inner packet type",
        "Inner packet type, if set, indicates that the packet is from a VPLS tunnel port",
        "VPLS port, If set, indicates that the packet is from a VPLS tunnel port",
        "Control word using sequence order",                                                       \
        "Sequence counter index",
        "Words to be removed from loopback packet header, not include bytes of customerId",
        "One word stands for 4 bytes, not include customerId 4 bytes",
        "If set, indicates that the packets will do service queue processing")
{
    ctc_loopback_nexthop_param_t iloop_nh;
    uint32 nhid, tmp_value = 0;
    uint8 tmp_argi;
    int32 ret;

    sal_memset(&iloop_nh, 0, sizeof(ctc_loopback_nexthop_param_t));
    CTC_CLI_GET_UINT32_RANGE("NexthopID", nhid, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("lport", iloop_nh.lpbk_lport, argv[1], 0, CTC_MAX_UINT16_VALUE);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("customerid-valid");
    if (0xFF != tmp_argi)
    {
        iloop_nh.customerid_valid = TRUE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("inner-pkt-type");
    if (0xFF != tmp_argi)
    {
        iloop_nh.inner_packet_type_valid = TRUE;
        CTC_CLI_GET_UINT8_RANGE("Inner packet type", tmp_value, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
        if (tmp_value >= MAX_CTC_PARSER_PKT_TYPE)
        {
            ctc_cli_out("Invalid inner packet type %d\n", tmp_value);
            return CLI_ERROR;
        }
        iloop_nh.inner_packet_type = tmp_value;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vpls-port");
    if (0xFF != tmp_argi)
    {
        iloop_nh.logic_port = TRUE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("sequence-number");
    if (0xFF != tmp_argi)
    {
        iloop_nh.sequence_en = TRUE;
        CTC_CLI_GET_UINT8_RANGE("Sequence counter index", tmp_value, argv[tmp_argi + 1], 0, 255);
        iloop_nh.sequence_counter_index = tmp_value;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("words-remove");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("Inner packet type", tmp_value, argv[tmp_argi + 1]);
        if (tmp_value >= 32)
        {
            ctc_cli_out("Invalid remove words %d, should less than 32\n", tmp_value);
            return CLI_ERROR;
        }
        iloop_nh.words_removed_from_hdr = tmp_value;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("service-queue-en");
    if (0xFF != tmp_argi)
    {
        iloop_nh.service_queue_en = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_iloop(g_api_lchip, nhid, &iloop_nh);
    }
    else
    {
        ret = ctc_nh_add_iloop(nhid, &iloop_nh);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_iloop,
        ctc_cli_nh_remove_iloop_cmd,
        "nexthop remove iloop NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_ILOOP_STR,
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_iloop(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_iloop(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_rspan,
        ctc_cli_nh_add_rspan_cmd,
        "nexthop add rspan NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (vlan VLAN_ID) (remote-chip|)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_RSPAN_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "CrossChip Rspan")
{
    ctc_rspan_nexthop_param_t rspan;
    uint32 nhid;
    uint8 tmp_arg_i = 0;

    int32 ret;

    sal_memset(&rspan, 0, sizeof(ctc_rspan_nexthop_param_t));

    CTC_CLI_GET_UINT32_RANGE("NexthopID", nhid, argv[0], 0, CTC_MAX_UINT32_VALUE);
    tmp_arg_i = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_arg_i)
    {
        CTC_CLI_GET_UINT32_RANGE("dsnh_offset", rspan.dsnh_offset, argv[tmp_arg_i + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    tmp_arg_i = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != tmp_arg_i)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan", rspan.rspan_vid, argv[tmp_arg_i + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_arg_i = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != tmp_arg_i)
    {
        rspan.remote_chip = TRUE;
    }

    tmp_arg_i = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_arg_i)
    {
         uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_arg_i + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_RSPAN, &rspan));
        return CLI_SUCCESS;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_rspan(g_api_lchip, nhid, &rspan);
    }
    else
    {
        ret = ctc_nh_add_rspan(nhid, &rspan);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_rspan,
        ctc_cli_nh_remove_rspan_cmd,
        "nexthop remove rspan NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_RSPAN_STR,
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32_RANGE("NexthopID", nhid, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_rspan(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_rspan(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_debug_on,
        ctc_cli_nh_debug_on_cmd,
        "debug nexthop (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_NH_M_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    int32 ret = CLI_SUCCESS;
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

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        typeenum = NH_CTC;
    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        typeenum = NH_SYS;
    }

    ret = ctc_debug_set_flag("nexthop", "nexthop", typeenum, level, TRUE);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_debug_off,
        ctc_cli_nh_debug_off_cmd,
        "no debug nexthop (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_NH_M_STR,
        "Ctc layer",
        "Sys layer")
{
    int32 ret = CLI_SUCCESS;
    uint32 typeenum = 0;
    uint8 level = 0;

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        typeenum = NH_CTC;
    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        typeenum = NH_SYS;
    }

    ret = ctc_debug_set_flag("nexthop", "nexthop", typeenum, level, FALSE);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_ecmp,
        ctc_cli_nh_add_ecmp_cmd,
        "nexthop add ecmp ECMPID (NHID0 (NHID1 (NHID2 (NHID3 (NHID4 (NHID5 (NHID6 (NHID7 |) |) |) |) |) |) |) |) \
        ((dynamic | rr | random-rr | xerspan) |) {failover-en | stats STATS_ID | member-num NUM |}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "ECMP group",
        CTC_CLI_NH_ID_STR,
        "Member nexthop ID 0",
        "Member nexthop ID 1",
        "Member nexthop ID 2",
        "Member nexthop ID 3",
        "Member nexthop ID 4",
        "Member nexthop ID 5",
        "Member nexthop ID 6",
        "Member nexthop ID 7",
        "Dynamic load balance",
        "Round robin",
        "Random round robin",
        "Erspan used for symmetric hash",
        "Resilient hash",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "member num in this group",
        "number")
{
    ctc_nh_ecmp_nh_param_t nh_param;
    uint32 nhid, ecmp_nhid;
    int i;
    uint8 index = 0, count = 0;
    int32 ret;

    sal_memset(&nh_param, 0, sizeof(nh_param));

    CTC_CLI_GET_UINT32("ECMP nexthop ID", ecmp_nhid, argv[0]);


    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        nh_param.type = CTC_NH_ECMP_TYPE_DLB;
        count = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rr");
    if (index != 0xFF)
    {
        nh_param.type = CTC_NH_ECMP_TYPE_RR;
        count = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random-rr");
    if (index != 0xFF)
    {
        nh_param.type = CTC_NH_ECMP_TYPE_RANDOM_RR;
        count = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xerspan");
    if (index != 0xFF)
    {
        nh_param.type = CTC_NH_ECMP_TYPE_XERSPAN;
        count = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("failover-en");
    if (index != 0xFF)
    {
        nh_param.failover_en = 1;
        count ++;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("stats", nh_param.stats_id, argv[index + 1]);
        count += 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("member-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("member num", nh_param.member_num, argv[index + 1]);
        count += 2;
    }

    for (i = 1; i < (argc - count); i++)
    {
        CTC_CLI_GET_UINT32("nexthop", nhid, argv[i]);
        nh_param.nhid[i - 1] = nhid;
    }

    nh_param.nh_num = argc - 1 - count;

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_ecmp(g_api_lchip, ecmp_nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_ecmp(ecmp_nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_ecmp,
        ctc_cli_nh_remove_ecmp_cmd,
        "nexthop remove ecmp ECMPID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        "ECMP group",
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("ecmp", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_ecmp(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_ecmp(nhid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_update_ecmp,
        ctc_cli_nh_update_ecmp_cmd,
        "nexthop update ecmp ECMPID (NHID0 (NHID1 (NHID2 (NHID3 (NHID4 (NHID5 (NHID6 (NHID7 |) |) |) |) |) |) |) |) (add | remove)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        "ECMP group",
        CTC_CLI_NH_ID_STR,
        "Member nexthop ID 0",
        "Member nexthop ID 1",
        "Member nexthop ID 2",
        "Member nexthop ID 3",
        "Member nexthop ID 4",
        "Member nexthop ID 5",
        "Member nexthop ID 6",
        "Member nexthop ID 7",
        "Add member",
        "Remove member")
{
    ctc_nh_ecmp_nh_param_t nh_param;
    int32 ret;
    uint32 ecmp_nhid, nhid;
    uint8 i = 0, index = 0, count = 0;

    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    CTC_CLI_GET_UINT32("ecmp", ecmp_nhid, argv[0]);


    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (index != 0xFF)
    {
        nh_param.upd_type = CTC_NH_ECMP_ADD_MEMBER;
        count = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (index != 0xFF)
    {
        nh_param.upd_type = CTC_NH_ECMP_REMOVE_MEMBER;
        count = 1;
    }


    for (i = 1; i < (argc - count); i++)
    {
        CTC_CLI_GET_UINT32("nexthop", nhid, argv[i]);
        nh_param.nhid[i - 1] = nhid;
    }

    nh_param.nh_num = argc - 1 - count;

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_ecmp(g_api_lchip, ecmp_nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_ecmp(ecmp_nhid, &nh_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_nh_cfg_max_ecmp,
        ctc_cli_nh_cfg_max_ecmp_cmd,
        "nexthop max-ecmp MAX_ECMP",
        CTC_CLI_NH_M_STR,
        CTC_CLI_MAX_ECMP_DESC,
        CTC_CLI_MAX_ECMP_DESC)
{

    int32 ret;
    uint16 max_ecmp;

    CTC_CLI_GET_UINT32("ecmp", max_ecmp, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_set_max_ecmp(g_api_lchip, max_ecmp);
    }
    else
    {
        ret = ctc_nh_set_max_ecmp(max_ecmp);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_show_max_ecmp,
        ctc_cli_nh_show_max_ecmp_cmd,
        "show nexthop max-ecmp",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        CTC_CLI_MAX_ECMP_DESC)
{

    int32 ret;
    uint16 max_ecmp;

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_get_max_ecmp(g_api_lchip, &max_ecmp);
    }
    else
    {
        ret = ctc_nh_get_max_ecmp(&max_ecmp);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Max-ecmp :%d \n\r", max_ecmp);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_mcast,
        ctc_cli_nh_add_mcast_cmd,
        "nexthop add mcast NHID (group GROUP_ID|) (mirror |mcast-profile|) (stats STATS_ID |)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR,
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "If set,the mcast group will be applied to mirror",
        "If set,the mcast group is profile",
        CTC_CLI_STATS_STR,
        "0~0xFFFFFFFF")
{
    uint32 nhid;
    uint32 groupid = 0;
    uint8 index = 0;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    int32 ret;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("group", groupid, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mirror");
    if (index != 0xFF)
    {
        nh_mcast_group.is_mirror = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (index != 0xFF)
    {
        nh_mcast_group.stats_en = 1;
        CTC_CLI_GET_UINT32("stats-id", nh_mcast_group.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcast-profile");
    if (index != 0xFF)
    {
        nh_mcast_group.is_profile = 1;
    }

    nh_mcast_group.mc_grp_id = groupid;

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_mcast(g_api_lchip, nhid, &nh_mcast_group);
    }
    else
    {
        ret = ctc_nh_add_mcast(nhid, &nh_mcast_group);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_mcast,
        ctc_cli_nh_remove_mcast_cmd,
        "nexthop remove mcast NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_mcast(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_mcast(nhid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_remove_l2mc_member,
        ctc_cli_nh_add_remove_l2mc_member_cmd,
        "nexthop mcast NHID (add | remove) bridge-member (port GPORT_ID | (" CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP|)))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        "The bridge member of multicast nexthop",
        "Global port/Remote chip id",
        "Local member  gport:alloc_dsnh_gchip(8bit) +local phy port(8bit)",
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    uint32 nhid;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    int32 ret;
    uint8 index = 0;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

    nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;

    if (CLI_CLI_STR_EQUAL("a", 1))
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    }
    else
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    }

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", nh_mcast_group.mem_info.destid, argv[index + 1]);
    }

    nh_mcast_group.mem_info.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(nh_mcast_group.mem_info.port_bmp);
    if (nh_mcast_group.mem_info.port_bmp[0] || nh_mcast_group.mem_info.port_bmp[1] ||
        nh_mcast_group.mem_info.port_bmp[8] || nh_mcast_group.mem_info.port_bmp[9])
    {
        nh_mcast_group.mem_info.port_bmp_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", nh_mcast_group.mem_info.gchip_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mcast(g_api_lchip, nhid, &nh_mcast_group);
    }
    else
    {
        ret = ctc_nh_update_mcast(nhid, &nh_mcast_group);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_remove_ipmc_member,
        ctc_cli_nh_add_remove_ipmc_member_cmd,
        "nexthop mcast NHID (add | remove) ip-member "CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR "{leaf-check-en |logic-port LOGIC_PORT|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        "The ip member of multicast nexthop",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Leaf check enable",
        "Logic dest port",
        "Logic dest port value")
{
    uint32 nhid;
    uint8 index;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    int32 ret;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

    nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_IPMC_LOCAL;

    if (CLI_CLI_STR_EQUAL("a", 1))
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    }
    else
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    }

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("phy-if");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.l3if_type = CTC_L3IF_TYPE_SUB_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-if");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlanid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanid", nh_mcast_group.mem_info.vid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", nh_mcast_group.mem_info.destid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);

    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-port");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.is_vlan_port = 1;
    }
    nh_mcast_group.mem_info.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(nh_mcast_group.mem_info.port_bmp);
    if (nh_mcast_group.mem_info.port_bmp[0] || nh_mcast_group.mem_info.port_bmp[1] ||
        nh_mcast_group.mem_info.port_bmp[8] || nh_mcast_group.mem_info.port_bmp[9])
    {
        nh_mcast_group.mem_info.port_bmp_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", nh_mcast_group.mem_info.gchip_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("leaf-check-en");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(nh_mcast_group.mem_info.flag, CTC_MCAST_NH_FLAG_LEAF_CHECK_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT16("logic port", nh_mcast_group.mem_info.logic_dest_port, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mcast(g_api_lchip, nhid, &nh_mcast_group);
    }
    else
    {
        ret = ctc_nh_update_mcast(nhid, &nh_mcast_group);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_remove_nh_member,
        ctc_cli_nh_add_remove_nh_member_cmd,
        "nexthop mcast NHID (add | remove) nh-member NH_ID {reflective |source-check-disable| port GPORT_ID| }",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        "The nexthop member of multicast nexthop",
        CTC_CLI_NH_ID_STR,
        "Reflect  to source port",
        "Port check disable",
        "Global port",
        CTC_CLI_GPHYPORT_ID_DESC)
{
    uint32 nhid;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    int32 ret;
    uint8 index;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

    nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;

    if (CLI_CLI_STR_EQUAL("a", 1))
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    }
    else
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    }

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);
    CTC_CLI_GET_UINT32("ref nhid",  nh_mcast_group.mem_info.ref_nhid, argv[2]);

    index = CTC_CLI_GET_ARGC_INDEX("reflective");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.is_reflective = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("source-check-disable");
    if (index != 0xFF)
    {
        nh_mcast_group.mem_info.is_source_check_dis = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(nh_mcast_group.mem_info.flag, CTC_MCAST_NH_FLAG_ASSIGN_PORT);
        CTC_CLI_GET_UINT32_RANGE("gport",  nh_mcast_group.mem_info.destid, argv[index+1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mcast(g_api_lchip, nhid, &nh_mcast_group);
    }
    else
    {
        ret = ctc_nh_update_mcast(nhid, &nh_mcast_group);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_add_remove_remote_member,
        ctc_cli_nh_add_remove_remote_member_cmd,
        "nexthop mcast NHID (add | remove) remote-member port GPORT_ID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_MCAST_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        "The member of multicast nexthop",
        "Global port/Remote chip id",
        "remote chip entry,gport:  alloc_dsnh_gchip(8bit) + remote alloc_dsnh_gchip id(8bit)")
{
    uint32 nhid;
    ctc_mcast_nh_param_group_t nh_mcast_group;
    int32 ret;

    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));

    nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_REMOTE;

    if (CLI_CLI_STR_EQUAL("a", 1))
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    }
    else
    {
        nh_mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    }

    CTC_CLI_GET_UINT32("nexthop", nhid, argv[0]);
    CTC_CLI_GET_UINT16_RANGE("remote",  nh_mcast_group.mem_info.destid, argv[2], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_mcast(g_api_lchip, nhid, &nh_mcast_group);
    }
    else
    {
        ret = ctc_nh_update_mcast(nhid, &nh_mcast_group);
    }
    if (ret < 0)
   {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#define CTC_CLI_NH_IP_TUNNEL_UDP "{l4-srcport L4SRC|l4-destport L4DST|}"
#define CTC_CLI_NH_IP_TUNNEL_UDP_DESC     \
    "UDP l4 source port",                 \
    "L4 source port value",                \
    "UDP l4 dest port",                 \
    "L4 dest port value"

#define CTC_CLI_NH_IP_TUNNEL_OVERLAY "(nvgre | vxlan | vxlan-gpe | geneve) {strip-vlan | dst-vnid VN_ID"\
  "{cross-vni inner-macda MAC_DA (update-inner-macsa (inner-macsa MAC_SA |)| )"\
  "| inner-pkt-type TYPE |  } | h-overlay | new-l4srcport L4PORT |}"
#define CTC_CLI_NH_IP_TUNNEL_V4 "tunnel-v4 (ip6in4 ("CTC_CLI_NH_IP_TUNNEL_UDP"|)|ip4in4 (ip-bfd|vccv-pw-bfd|) ("CTC_CLI_NH_IP_TUNNEL_UDP" |)|ip6to4 (manual-ipsa|) | "\
    "6rd (sp-prefix-len SPPREFIXLEN |) (v4-sa-masklen SAMASKLEN|) (v4-da-masklen DAMASKLEN|) | "\
    "isatap |grein4 protocol PROTOCOL {gre-key GREKEY | mirror | xerspan-ext-hdr span-id SPAN_ID (copy-hash-en|) (keep-igs-ts|) | strip-vlan| }"\
    "|" CTC_CLI_NH_IP_TUNNEL_OVERLAY ") {ipsa A.B.C.D | ipda A.B.C.D|} "
#define CTC_CLI_NH_IP_TUNNEL_V6 "tunnel-v6 (ip4in6 (ip-bfd|vccv-pw-bfd|) ("CTC_CLI_NH_IP_TUNNEL_UDP" |)| ip6in6 ("CTC_CLI_NH_IP_TUNNEL_UDP"|)| grein6 protocol PROTOCOL"\
    "{gre-key GREKEY| mirror| xerspan-ext-hdr span-id SPAN_ID (copy-hash-en|) (keep-igs-ts|) | strip-vlan |} |" CTC_CLI_NH_IP_TUNNEL_OVERLAY \
    ") ipsa X:X::X:X ipda X:X::X:X"
#define CTC_CLI_NH_IP_TUNNEL_IVI "ivi (6to4 | 4to6 v6-prefix-addr X:X::X:X) prefix-len LENGTH (prefix-obey-rfc6052|)"
#define CTC_CLI_NH_IP_TUNNEL_NAT "nat new-ipda A.B.C.D {new-l4dstport L4PORT | replace-ip|}"
#define CTC_CLI_NH_IP_TUNNEL_TYPE "type (" CTC_CLI_NH_IP_TUNNEL_V4 "|" CTC_CLI_NH_IP_TUNNEL_V6 \
    "|" CTC_CLI_NH_IP_TUNNEL_IVI "|" CTC_CLI_NH_IP_TUNNEL_NAT "| none |)"
#define CTC_CLI_NH_IP_TUNNEL_FLAG "{copy-dont-frag | dont-frag DONT_FRAG | re-route | dscp DSCP dscp-select SEL_MODE (dscp-domain DOMAIN |)|"\
   "ttl TTL (map-ttl|) | mtu-check-en mtu-size MTU | flow-label-mode MODE (flow-label-value VALUE|) | logic-dest-port PORT | logic-port-check |"\
   "stats-id STATSID | loop-nhid NHID| ecn-select ECN_SEL | inner-no-ttl-dec | dot1ae-en CHAN |udf-profile PROFILE|udf-replace DATA1 DATA2 DATA3 DATA4|vxlan-gbp-en|adjust-length LEN|}"

#define CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_STR  CTC_CLI_MAC_ARP_STR "(macsa MAC|)"CTC_CLI_L3IF_ALL_STR CTC_CLI_NH_IP_TUNNEL_TYPE CTC_CLI_NH_IP_TUNNEL_FLAG

#define CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_DESC  CTC_CLI_MAC_ARP_DESC,  \
    CTC_CLI_MACSA_DESC,                                             \
    CTC_CLI_MAC_FORMAT,                                             \
    CTC_CLI_L3IF_ALL_DESC,                                          \
    "Tunnel type",                                                  \
    "Tunnel over ipv4",                                             \
    "Ipv6inIpv4",                                                   \
    CTC_CLI_NH_IP_TUNNEL_UDP_DESC,                                  \
    "Ipv4inIpv4",                                                   \
    "IP-BFD enable the nexthop",                                    \
    "VCCV-PW-BFD enable the nexthop",                               \
    CTC_CLI_NH_IP_TUNNEL_UDP_DESC,                                  \
    "6to4",                                                         \
    "Use Manual IPSA",                                              \
    "6rd",                                                          \
    "6rd IPv6 SP prefix length (default is 32)",                    \
    "6rd IPv6 SP prefix length (0:16 1:24 2:32 3:40 4:48 5:56)",    \
    "IPv4 source address mask length (default is 32) ",             \
    "IPv4 source address mask length ",                             \
    "IPv4 destination address mask length (default is 32)",         \
    "IPv4 destination address mask length ",                        \
    "ISATAP",                                                       \
    "GRE in Ipv4",                                                  \
    "Protocol type of GRE Header",                                  \
    "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", \
    "GRE header with GRE key",                                      \
    "GRE key value", \
    "Mirror over GRE tunnel",                                       \
    "Xerspan with extend header",\
    "Span id",\
    "Span id",\
    "Xerspan extend header include symmetric hash ",\
    "If set, will keep igress timestamp, else egess timestamp",\
    "Strip original vlan",             \
    "NvGRE",                                                        \
    "VxLAN",                                    \
    "VxLAN-GPE",                               \
    "GENEVE",                                          \
    "Strip original vlan",             \
    "Destnation virtual sub net ID, only set if source and dest vnid is different",\
    "<0x1-0xFFFFFF>",                                         \
    "The nexthop is cross two different Virtual sub net",           \
    "Inner destination MAC address",                                \
    CTC_CLI_MAC_FORMAT,                                             \
    "Update inner source MAC address",    \
    "Inner source MAC address",                                \
    CTC_CLI_MAC_FORMAT,                                             \
    "Payload type",  \
    "0: Ethernet; 1: IPv4; 3: IPv6", \
    "Hierarchy overlay tunnel",\
    "New layer4 source port",                                         \
    "Value",                                              \
    "Ip sa",                                                        \
    CTC_CLI_IPV4_FORMAT,                                            \
    "Ip da",                                                        \
    CTC_CLI_IPV4_FORMAT,                                            \
    "Tunnel over ipv6",                                             \
    "Ipv4inIpv6",                                                   \
    "IP-BFD enable the nexthop",                                    \
    "VCCV-PW-BFD enable the nexthop",                               \
    CTC_CLI_NH_IP_TUNNEL_UDP_DESC,                                  \
    "Ipv6inIpv6",                                                   \
    CTC_CLI_NH_IP_TUNNEL_UDP_DESC,                                  \
    "GRE in Ipv6",                                                  \
    "Protocol type of GRE Header",                                  \
    "Protocol type of GRE Header(Ipv4:0x0800,ipv6:0x86DD,ethernet:0x6558)", \
    "GRE header with GRE key",                                      \
    "GRE key 32 bit",                                               \
    "Mirror over GRE tunnel",                                       \
    "Xerspan with extend header",\
    "Span id",\
    "Span id",\
    "Xerspan extend header include symmetric hash ",\
    "If set, will keep igress timestamp, else egess timestamp",\
    "Strip original vlan",             \
    "NvGRE in IPv6",                                                \
    "VxLAN in IPv6",                                                \
    "VxLAN-GPE in IPv6",                                                \
    "GENEVE in IPv6",                                                \
    "Strip original vlan",                                       \
    "Destnation virtual sub net ID, only set if source and dest vnid is different",\
    "<0x1-0xFFFFFF>",                                         \
    "The nexthop is cross two different Virtual sub net",           \
    "Inner destination MAC address",                                \
    CTC_CLI_MAC_FORMAT,                                             \
    "Update inner source MAC address",    \
    "Inner source MAC address",                                \
    CTC_CLI_MAC_FORMAT,                                             \
    "Payload type",  \
    "0: Ethernet; 1: IPv4; 3: IPv6", \
    "Hierarchy overlay tunnel",\
    "New layer4 source port",                                         \
    "Value",                                              \
    "Ip sa",                                                        \
    CTC_CLI_IPV6_FORMAT,                                            \
    "Ip da",                                                        \
    CTC_CLI_IPV6_FORMAT,                                            \
    "IVI",                                                          \
    "IPv6 translate to IPv4",                                       \
    "IPv4 translate to IPv6",                                       \
    "IPv6 prefix address",                                          \
    CTC_CLI_IPV6_FORMAT,                                            \
    "Prefix length",                                                \
    "Length value 0-32, 1-40, 2-48, 3-56, 4-64, 5-72, 6-80, 7-96",  \
    "Prefix obey rfc6052, if set, IPv6 address's 64 to 71 bits are set to zero",    \
    "NAT",                                                          \
    "New ipda",                                                     \
    CTC_CLI_IPV4_FORMAT,                                            \
    "New layer4 dest port",                                         \
    "Value",                                              \
    "Nat only replace IP, mac not change",                          \
    "None",                                               \
    "If set , means new dont frag will copy payload ip dont frag ", \
    "Use defined dont frag",                                        \
    "Dont frag",                                                    \
    "If set,encap packet will re-route with new tunnel header",     \
    CTC_CLI_NH_DSNH_TUNNEL_DSCP_DESC,                      \
    CTC_CLI_NH_DSNH_TUNNEL_DSCP_VAL,                    \
    "Dscp select mode",                                       \
    "Dscp select mode, refer to ctc_nh_dscp_select_mode_t", \
    "Dscp domain",                                                  \
    "Domain ID",                                                    \
    "TTL",                                                          \
    "TTL value",                                                    \
    "TTL mode, if set means new ttl will be (oldTTL - specified TTL) otherwise new ttl is specified TTL", \
    "Mtu check enable",                                             \
    "Mtu size",                                                     \
    "Mtu size value",                                               \
    "IPv6 flow label mode (default is do not set flow label value)",\
    "IPv6 flow label mode:0-Do not set flow label valu, 1-Use (user-define flow label + header hash)to outer header , 2- Use user-define flow label to outer header", \
    "IPv6 flow label value (default is 0)",                         \
    "IPv6 flow label value",                                        \
    "Logic destination port assigned for this tunnel",              \
    "Logic port value",                                             \
    "Logic port check",                                             \
    CTC_CLI_STATS_ID_DESC,                                          \
    CTC_CLI_STATS_ID_VAL,                                           \
    "Loopback nexthop id",\
    CTC_CLI_NH_ID_STR,\
    "Ecn select mode", \
    "Ecn mode:0-use ecn 0, 1-user-define ECN, 2-Use ECN value from ECN map, 3-Copy packet ECN",\
    "Inner TTL no dec", \
    "Dot1ae enable", \
    "Dot1ae channel id",  \
    "UDF Profile ID",   \
    "Profile",          \
    "UDF replace",      \
    "Replace data1",    \
    "Replace data2",    \
    "Replace data3",    \
    "Replace data4",    \
    "Vxlan GBP enable", \
    "For qos shape adjusting packet length", \
    "LEN"

STATIC int32
_ctc_nexthop_cli_parse_ip_tunnel_nexthop(char** argv, uint16 argc,
                                         ctc_ip_tunnel_nh_param_t* p_nhinfo)
{   /*mac MAC (sub-if port GPORT_ID vlan VLAN_ID | vlan-if vlan VLAN_ID port GPORT_ID | routed-port GPORT_ID) */
    uint8 tmp_argi;
    int ret = CLI_SUCCESS;
    mac_addr_t mac;
    ipv6_addr_t ipv6_address;
    uint8 is_overlay = 0;

    /*mac info*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);

        sal_memcpy(p_nhinfo->mac, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("arp-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("arp id", p_nhinfo->arp_id, argv[tmp_argi + 1]);
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac sa address", mac, argv[tmp_argi + 1]);

        sal_memcpy(p_nhinfo->mac_sa, mac, sizeof(mac_addr_t));
    }

    /*ecmp l3if*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ecmp-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.ecmp_if_id, p_nhinfo->oif.vid);
    }

    /*vlan port if*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-port");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->oif.is_l2_port = 1;
    }

    /*l3if info*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("routed-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    /*tunnel v4 ip header info*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("tunnel-v4");
    if (0xFF != tmp_argi)
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip6in4");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV6_IN4;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("l4-srcport", p_nhinfo->tunnel_info.l4_src_port, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("l4-destport", p_nhinfo->tunnel_info.l4_dst_port, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip4in4");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN4;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip6to4");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_6TO4;
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("manual-ipsa");
            if (0xFF != tmp_argi)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_6TO4_USE_MANUAL_IPSA;
            }
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("6rd");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_6RD;
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("sp-prefix-len");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT8("sp-prefix-len", p_nhinfo->tunnel_info.sixrd_info.sp_prefix_len, argv[tmp_argi + 1]);
            }
            else
            {
                p_nhinfo->tunnel_info.sixrd_info.sp_prefix_len = 2;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("v4-sa-masklen");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT8("v4-sa-masklen", p_nhinfo->tunnel_info.sixrd_info.v4_sa_masklen, argv[tmp_argi + 1]);
            }
            else
            {
                p_nhinfo->tunnel_info.sixrd_info.v4_sa_masklen = 32;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("v4-da-masklen");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT8("v4-da-masklen", p_nhinfo->tunnel_info.sixrd_info.v4_da_masklen, argv[tmp_argi + 1]);
            }
            else
            {
                p_nhinfo->tunnel_info.sixrd_info.v4_da_masklen = 32;
            }
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("isatap");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_ISATAP;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("grein4");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN4;

            CTC_CLI_GET_UINT16_RANGE("PROTOCOL", p_nhinfo->tunnel_info.gre_info.protocol_type, argv[tmp_argi + 2], 0, CTC_MAX_UINT16_VALUE);
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("gre-key");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT32_RANGE("gre-key", p_nhinfo->tunnel_info.gre_info.gre_key, argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("mirror");
            if (0xFF != tmp_argi)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_MIRROR;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("xerspan-ext-hdr");
            if (0xFF != tmp_argi)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR;

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("span-id");
                if (0xFF != tmp_argi)
                {
                    CTC_CLI_GET_UINT16("span id", p_nhinfo->tunnel_info.span_id, argv[tmp_argi + 1]);
                }

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("copy-hash-en");
                if (0xFF != tmp_argi)
                {
                    p_nhinfo->tunnel_info.flag  |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR_HASH;
                }

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("keep-igs-ts");
                if (0xFF != tmp_argi)
                {
                    p_nhinfo->tunnel_info.flag  |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_KEEP_IGS_TS;
                }

            }

            p_nhinfo->tunnel_info.gre_info.gre_ver = 0;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("nvgre");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NVGRE_IN4;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
            p_nhinfo->tunnel_info.flag = CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("geneve");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GENEVE_IN4;
            is_overlay = 1;
        }

        /*IPSA IPDA*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", p_nhinfo->tunnel_info.ip_sa.ipv4, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipda", p_nhinfo->tunnel_info.ip_da.ipv4, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("copy-dont-frag");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_COPY_DONT_FRAG;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dont-frag");
        if (0xFF != tmp_argi)
        {
            uint8 dont_farg = 0;
            CTC_CLI_GET_UINT8("dont-frag", dont_farg, argv[tmp_argi + 1]);
            if (dont_farg)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_DONT_FRAG;
            }
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan-gbp-en");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN;
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("tunnel-v6");
    if (0xFF != tmp_argi)    /*tunnel v6 ip header info*/
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip4in6");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN6;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip6in6");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV6_IN6;
        }


        tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("l4-srcport", p_nhinfo->tunnel_info.l4_src_port, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16("l4-destport", p_nhinfo->tunnel_info.l4_dst_port, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("grein6");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN6;

            CTC_CLI_GET_UINT16_RANGE("PROTOCOL", p_nhinfo->tunnel_info.gre_info.protocol_type, argv[tmp_argi + 2], 0, CTC_MAX_UINT16_VALUE);
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("gre-key");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_UINT32_RANGE("gre-key", p_nhinfo->tunnel_info.gre_info.gre_key, argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("mirror");
            if (0xFF != tmp_argi)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_MIRROR;
            }

            tmp_argi = CTC_CLI_GET_ARGC_INDEX("xerspan-ext-hdr");
            if (0xFF != tmp_argi)
            {
                p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR;

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("span-id");
                if (0xFF != tmp_argi)
                {
                    CTC_CLI_GET_UINT16("span id", p_nhinfo->tunnel_info.span_id, argv[tmp_argi + 1]);
                }

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("copy-hash-en");
                if (0xFF != tmp_argi)
                {
                    p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR_HASH;
                }

                tmp_argi = CTC_CLI_GET_ARGC_INDEX("keep-igs-ts");
                if (0xFF != tmp_argi)
                {
                    p_nhinfo->tunnel_info.flag  |= CTC_IP_NH_TUNNEL_FLAG_XERSPN_KEEP_IGS_TS;
                }

            }

            p_nhinfo->tunnel_info.gre_info.gre_ver = 0;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("none");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NONE;
        }

        /*IPSA IPDA*/
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[tmp_argi + 1]);
            /* adjust endian */
            p_nhinfo->tunnel_info.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
            p_nhinfo->tunnel_info.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
            p_nhinfo->tunnel_info.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
            p_nhinfo->tunnel_info.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[tmp_argi + 1]);
            /* adjust endian */
            p_nhinfo->tunnel_info.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
            p_nhinfo->tunnel_info.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
            p_nhinfo->tunnel_info.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
            p_nhinfo->tunnel_info.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("nvgre");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NVGRE_IN6;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN6;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN6;
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE;
            is_overlay = 1;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("geneve");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GENEVE_IN6;
            is_overlay = 1;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("vxlan-gbp-en");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN;
        }
    }

    if (is_overlay)
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("inner-pkt-type");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT8_RANGE("inner-pkt-type", p_nhinfo->tunnel_info.inner_packet_type, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dst-vnid");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32_RANGE("dst-vnid", p_nhinfo->tunnel_info.vn_id, argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("inner-macda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
            sal_memcpy(p_nhinfo->tunnel_info.inner_macda, mac, sizeof(mac_addr_t));
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("update-inner-macsa");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA;
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("inner-macsa");
            if (0xFF != tmp_argi)
            {
                CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
                sal_memcpy(p_nhinfo->tunnel_info.inner_macsa, mac, sizeof(mac_addr_t));
            }
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("cross-vni");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("h-overlay");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_HOVERLAY;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-l4srcport");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT;
            CTC_CLI_GET_UINT16_RANGE("new-l4srcport", p_nhinfo->tunnel_info.l4_src_port, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ivi");
    if (0xFF != tmp_argi)
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("6to4");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IVI_6TO4;
        }
        else
        {
            p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IVI_4TO6;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("v6-prefix-addr");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("v6-prefix-addr", ipv6_address, argv[tmp_argi + 1]);
            /* adjust endian */
            p_nhinfo->tunnel_info.ivi_info.ipv6[0] = sal_htonl(ipv6_address[0]);
            p_nhinfo->tunnel_info.ivi_info.ipv6[1] = sal_htonl(ipv6_address[1]);
            p_nhinfo->tunnel_info.ivi_info.ipv6[2] = sal_htonl(ipv6_address[2]);
            p_nhinfo->tunnel_info.ivi_info.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("prefix-len");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32_RANGE("prefix-len", p_nhinfo->tunnel_info.ivi_info.prefix_len, argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("prefix-obey-rfc6052");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.ivi_info.is_rfc6052 = 1;
        }
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("nat");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_NAT;

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("new-ipda", p_nhinfo->tunnel_info.ip_da.ipv4, argv[tmp_argi + 1]);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("new-l4dstport");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT16_RANGE("new-l4dstport", p_nhinfo->tunnel_info.l4_dst_port, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ip");
        if (0xFF != tmp_argi)
        {
            p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_NAT_REPLACE_IP;
        }
    }

    /*re-route*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("re-route");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR;
    }

    /*DSCP*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", p_nhinfo->tunnel_info.dscp_or_tos, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp-select");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp-select", p_nhinfo->tunnel_info.dscp_select, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ecn-select");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn-select", p_nhinfo->tunnel_info.ecn_select, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dscp-domain");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("dscp-domain", p_nhinfo->tunnel_info.dscp_domain, argv[tmp_argi + 1]);
    }

    /*TTL*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", p_nhinfo->tunnel_info.ttl, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("map-ttl");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_MAP_TTL);
    }

    /*MTU*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-check-en");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_MTU_CHECK);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-size");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("mtu-size", p_nhinfo->tunnel_info.mtu_size, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-label-mode");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("flow label mode", p_nhinfo->tunnel_info.flow_label_mode, argv[tmp_argi + 1]);
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-label-value");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("flow label value", p_nhinfo->tunnel_info.flow_label, argv[tmp_argi + 1]);
        }
        else
        {
            p_nhinfo->tunnel_info.flow_label = 0;
        }
    }
    else
    {
        p_nhinfo->tunnel_info.flow_label_mode = CTC_NH_FLOW_LABEL_NONE;
    }
    /*TTL*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_IP_BFD);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vccv-pw-bfd");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_VCCV_PW_BFD);
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("logic-dest-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("logic dest port", p_nhinfo->tunnel_info.logic_port, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("logic-port-check");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_LOGIC_PORT_CHECK);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("loop-nhid");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("loop nhid", p_nhinfo->loop_nhid , argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", p_nhinfo->tunnel_info.stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("strip-vlan");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("inner-no-ttl-dec");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_INNER_TTL_NO_DEC);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dot1ae-en");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("dot1ae channel", p_nhinfo->tunnel_info.dot1ae_chan_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("udf-profile");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("udf profile", p_nhinfo->tunnel_info.udf_profile_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("udf-replace");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("replace data", p_nhinfo->tunnel_info.udf_replace_data[0], argv[tmp_argi + 1]);
        CTC_CLI_GET_UINT16("replace data", p_nhinfo->tunnel_info.udf_replace_data[1], argv[tmp_argi + 2]);
        CTC_CLI_GET_UINT16("replace data", p_nhinfo->tunnel_info.udf_replace_data[2], argv[tmp_argi + 3]);
        CTC_CLI_GET_UINT16("replace data", p_nhinfo->tunnel_info.udf_replace_data[3], argv[tmp_argi + 4]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("adjust-length");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8("adjust-length", p_nhinfo->tunnel_info.adjust_length, argv[tmp_argi + 1]);
    }

    return ret;
}

CTC_CLI(ctc_cli_add_ip_tunnel,
        ctc_cli_add_ip_tunnel_cmd,
        "nexthop add ip-tunnel NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (unrov |fwd ("CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_STR "))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_IP_TUNNEL_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Unresolved nexthop",
        "Forward Nexthop",
        CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_DESC)
{

    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0, dsnh_offset = 0, tmp_argi = 0;
    ctc_ip_tunnel_nh_param_t nh_param = {0};

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_UNROV);
        nh_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NONE;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        if (_ctc_nexthop_cli_parse_ip_tunnel_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nh_param))
        {
            return CLI_ERROR;
        }
    }

    nh_param.dsnh_offset = dsnh_offset;
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
         uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_IP_TUNNEL, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_ip_tunnel(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_ip_tunnel(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_update_ip_tunnel,
        ctc_cli_update_ip_tunnel_cmd,
        "nexthop update ip-tunnel NHID ( fwd-attr|unrov2fwd (dsnh-offset OFFSET|)) ("CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_STR ")",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_IP_TUNNEL_STR,
        CTC_CLI_NH_ID_STR,
        "Update forward nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_IP_TUNNELV4_NH_PARAM_DESC)
{

    int32 ret  = CLI_SUCCESS;

    uint32 nhid;
    uint8 tmp_argi;
    ctc_ip_tunnel_nh_param_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(ctc_ip_tunnel_nh_param_t));
    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    if (_ctc_nexthop_cli_parse_ip_tunnel_nexthop(&(argv[1]), (argc - 1), &nhinfo))
    {
        return CLI_ERROR;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov2fwd");
    if (0xFF != tmp_argi)
    {

        nhinfo.upd_type = CTC_NH_UPD_UNRSV_TO_FWD;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT32("DsNexthop Table offset", nhinfo.dsnh_offset, argv[tmp_argi + 1]);
        }

        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_update_ip_tunnel(g_api_lchip, nhid, &nhinfo);
        }
        else
        {
            ret = ctc_nh_update_ip_tunnel(nhid, &nhinfo);
        }

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd-attr");
    if (0xFF != tmp_argi)
    {

        nhinfo.upd_type = CTC_NH_UPD_FWD_ATTR;
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_update_ip_tunnel(g_api_lchip, nhid, &nhinfo);
        }
        else
        {
            ret = ctc_nh_update_ip_tunnel(nhid, &nhinfo);
        }

    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_update_ip_tunnel_to_unrov,
        ctc_cli_update_ip_tunnel_to_unrov_cmd,
        "nexthop update ip-tunnel NHID fwd2unrov",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_IP_TUNNEL_STR,
        CTC_CLI_NH_ID_STR,
        "Forward nexthop to unresolved nexthop")
{
    int32 ret = 0;

    uint32 nhid;

    ctc_ip_tunnel_nh_param_t nhinfo;

    sal_memset(&nhinfo, 0, sizeof(nhinfo));

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    nhinfo.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_NONE;
    nhinfo.upd_type = CTC_NH_UPD_FWD_TO_UNRSV;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_ip_tunnel(g_api_lchip, nhid, &nhinfo);
    }
    else
    {
        ret = ctc_nh_update_ip_tunnel(nhid, &nhinfo);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_remove_ip_tunnel,
        ctc_cli_remove_ip_tunnel_cmd,
        "nexthop remove ip-tunnel NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_IP_TUNNEL_STR,
        CTC_CLI_NH_ID_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_ip_tunnel(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_ip_tunnel(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_nh_add_misc_l2hdr,
        ctc_cli_nh_add_misc_l2hdr_cmd,
        "nexthop add misc replace-l2hdr NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (reflective |port GPORT_ID) " \
        "("CTC_CLI_NH_MISC_MAC_EDIT_STR" |)" "{stats STATS_ID|"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MISC_STR,
        "Replace L2 Header",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "If set,packet will be reflective to ingress port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Swap macda and macsa",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint32 gport = 0;
    uint8 tmp_argi;
    mac_addr_t mac;
    ctc_misc_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);

    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("reflective");
    if (tmp_argi != 0xFF)
    {
        nh_param.misc_param.l2edit.is_reflective = 1;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("port");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[tmp_argi + 1]);
    }

    nh_param.type  = CTC_MISC_NH_TYPE_REPLACE_L2HDR;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-mac");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.l2edit.flag |= CTC_MISC_NH_L2_EDIT_SWAP_MAC;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-da");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

        nh_param.misc_param.l2edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_DA;

        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);

        sal_memcpy(nh_param.misc_param.l2edit.mac_da, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-sa");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

        nh_param.misc_param.l2edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_SA;

        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);

        sal_memcpy(nh_param.misc_param.l2edit.mac_sa, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-svlan");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

         /* nh_param.misc_param.l2edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_SVLAN_TAG;*/
         /*CTC_CLI_GET_UINT16_RANGE("replace svlan", nh_param.misc_param.l2edit.vlan_id, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);*/
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", nh_param.stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated len", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    nh_param.gport = gport;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.type = CTC_MISC_NH_TYPE_REPLACE_L2HDR;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MISC, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_misc_l2l3hdr,
        ctc_cli_nh_add_misc_l2l3hdr_cmd,
        "nexthop add misc replace-l2-l3hdr NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (reflective |port GPORT_ID) " \
        "("CTC_CLI_NH_MISC_MAC_EDIT_STR" |) (vlan-edit "CTC_CLI_NH_EGS_VLAN_EDIT_STR "|)" \
        "{replace-ipda A.B.C.D |replace-l4dest-port L4PORT|stats STATS_ID|"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MISC_STR,
        "Replace L2 & L3 Header",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "If set,packet will be reflective to ingress port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Swap macda and macsa",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Edit vlan",
        CTC_CLI_NH_EGS_VLAN_EDIT_DESC,
        "Replace ipda",
        CTC_CLI_IPV4_FORMAT,
        "Replace layer4 dest port",
        "Value",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint32 gport = 0;
    uint8 tmp_argi;
    mac_addr_t mac;
    ctc_misc_nh_param_t nh_param;
    ctc_vlan_egress_edit_info_t edit_info;


    sal_memset(&edit_info, 0, sizeof(ctc_vlan_egress_edit_info_t));
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);

    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("reflective");
    if (tmp_argi != 0xFF)
    {
        nh_param.misc_param.l2_l3edit.is_reflective = 1;
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("port");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[tmp_argi + 1]);
    }

    nh_param.type  = CTC_MISC_NH_TYPE_REPLACE_L2HDR;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-mac");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_SWAP_MAC;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-da");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_DA;

        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);

        sal_memcpy(nh_param.misc_param.l2_l3edit.mac_da, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-sa");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_SA;

        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);

        sal_memcpy(nh_param.misc_param.l2_l3edit.mac_sa, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-svlan");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }

         /* nh_param.misc_param.l2edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_SVLAN_TAG;*/
         /*CTC_CLI_GET_UINT16_RANGE("replace svlan", nh_param.misc_param.l2edit.vlan_id, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);*/
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("cvlan-edit-type");
    if (0xFF != tmp_argi)
    {
         /* _ctc_vlan_parser_egress_vlan_edit(&edit_info, &argv[tmp_argi-1], (argc));*/
        ret = _ctc_vlan_parser_egress_vlan_edit(&edit_info, &argv[0], argc);
        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_VLAN_EDIT;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ipda");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_IPV4_ADDRESS("replace-ipda", nh_param.misc_param.l2_l3edit.ipda, argv[tmp_argi + 1]);
        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L3_EDIT_REPLACE_IPDA;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-l4dest-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("replace-l4dest-port", nh_param.misc_param.l2_l3edit.dst_port, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L3_EDIT_REPLACE_DST_PORT;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", nh_param.stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    nh_param.gport = gport;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.misc_param.l2_l3edit.vlan_edit_info = edit_info;
    nh_param.type = CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR;

    if (CLI_ERROR == ret)
    {
        return CLI_ERROR;
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
         uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MISC, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_misc_l2hdr,
        ctc_cli_nh_remove_misc_l2hdr_cmd,
        "nexthop remove misc NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_MISC_STR,
        CTC_CLI_NH_ID_STR)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid;

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_misc(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_misc(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_misc_flexhdr,
        ctc_cli_nh_add_misc_flexhdr_cmd,
        "nexthop (add|update) misc replace-flex-hdr NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (reflective |port GPORT_ID|is-oif "CTC_CLI_L3IF_ALL_STR ") (l2-hdr {"\
        CTC_CLI_NH_MISC_MAC_EDIT_STR "| replace-ethtype ETH_TYPE |" CTC_CLI_NH_FLEX_VLAN_EDIT_STR "|bridge-op|} |)" \
        "("CTC_CLI_NH_MISC_MPLS_EDIT_STR "|"  CTC_CLI_NH_FLEX_ARPHDR_STR  "| ipv4-hdr { (swap-ip | {replace-ipsa A.B.C.D | replace-ipda A.B.C.D}) |" \
        CTC_CLI_NH_FLEX_L3HDR_STR  " | replace-protocol PROTOCOL|} ("  CTC_CLI_NH_FLEX_L4HDR_STR  "|)" \
        "| ipv6-hdr { (swap-ip| {replace-ipsa X:X::X:X | replace-ipda X:X::X:X }) |"  CTC_CLI_NH_FLEX_L3HDR_STR  \
        "| flow-label FLOW_LABEL|replace-protocol PROTOCOL|}" "(" CTC_CLI_NH_FLEX_L4HDR_STR "|)" "|) {stats STATS_ID |cid CID|mtu-check-en |"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MISC_STR,
        "Replace Flex Header",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "If set,packet will be reflective to ingress port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Outgoing is interface",
        CTC_CLI_L3IF_ALL_DESC,
        "L2 header",
        "Swap macda and macsa",
        "Mac destination address",
        CTC_CLI_MAC_FORMAT,
        "Mac source address",
        CTC_CLI_MAC_FORMAT,
        "Replace ether type",
        "<0-0xFFFF>",
        CTC_CLI_NH_FLEX_VLAN_EDIT_DESC,
        "Do bridge opreation",
        CTC_CLI_NH_MISC_MPLS_EDIT_DESC,
        CTC_CLI_NH_FLEX_ARPHDR_DESC,
        "Ipv4 header",
        "Swap packet ipda and ipsa",
        "Replace ipsa",
        CTC_CLI_IPV4_FORMAT,
        "Replace ipda",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_NH_FLEX_L3HDR_DESC,
        "Replace protocol",
        "<0-255>",
        CTC_CLI_NH_FLEX_L4HDR_DESC,
        "Ipv6 header",
        "Swap packet ipda and ipsa",
        "Replace ipsa",
        CTC_CLI_IPV6_FORMAT,
        "Replace ipda",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_NH_FLEX_L3HDR_DESC,
        "Flow label",
        "<0-0xFFFFF>",
        "Replace protocol",
        "<0-255>",
        CTC_CLI_NH_FLEX_L4HDR_DESC,
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "Cid",
        "Cid value",
        "Mtu check enable",
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    uint32 gport = 0;
    uint8 tmp_argi;
    mac_addr_t mac;
    ipv6_addr_t ipv6_address;
    ctc_misc_nh_param_t nh_param;
    ctc_nh_oif_info_t oif;
    uint8 label_num = 0;
    uint8 arrayi = 0;
    uint8 is_add = 0;

    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));
    sal_memset(&oif, 0, sizeof(ctc_nh_oif_info_t));

    nh_param.misc_param.flex_edit.dscp_select = CTC_NH_DSCP_SELECT_NONE;

    if (0 == sal_memcmp(argv[0], "add", 3))
    {
        is_add = 1;
    }

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[1]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("reflective");
    if (tmp_argi != 0xFF)
    {
        nh_param.misc_param.flex_edit.is_reflective = 1;
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("port");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-oif");
    if (0xFF != tmp_argi)
    {
        nh_param.is_oif = 1;
        CTC_CLI_NH_PARSE_L3IF(tmp_argi + 1, oif.gport, oif.vid);
        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));
    }

     /*L2*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("l2-hdr");
    if (tmp_argi != 0xFF)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
    }
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-mac");
        if (0xFF != tmp_argi)
        {
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_SWAP_MACDA;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-da");
        if (0xFF != tmp_argi)
        {
            if (tmp_argi == (argc - 1))
            {
                ctc_cli_out("%% Incomplete command\n");
                return CLI_ERROR;
            }
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACDA;
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
            sal_memcpy(nh_param.misc_param.flex_edit.mac_da, mac, sizeof(mac_addr_t));
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mac-sa");
        if (0xFF != tmp_argi)
        {
            if (tmp_argi == (argc - 1))
            {
                ctc_cli_out("%% Incomplete command\n");
                return CLI_ERROR;
            }
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACSA;
            CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
            sal_memcpy(nh_param.misc_param.flex_edit.mac_sa, mac, sizeof(mac_addr_t));
        }
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ethtype");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("replace-ethtype", nh_param.misc_param.flex_edit.ether_type, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_ETHERTYPE;
    }
    CTC_CLI_NH_FLEX_VLAN_EDIT_SET(nh_param.misc_param.flex_edit)

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("bridge-op");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_OP_BRIDGE;
    }

     /*Arp*/
    CTC_CLI_NH_FLEX_ARPHDR_SET(nh_param.misc_param.flex_edit)

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-mpls-label");
    if (0xff != tmp_argi)
    {

        nh_param.misc_param.flex_edit.packet_type = CTC_MISC_NH_PACKET_TYPE_MPLS;
        CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(nh_param.misc_param.flex_edit.label[0], tmp_argi,  "label1", "map-ttl1", "is-mcast1", "exp1-domain");
        CTC_CLI_NH_PARSE_MPLS_LABEL_PARAM(nh_param.misc_param.flex_edit.label[1], tmp_argi,  "label2", "map-ttl2", "is-mcast2", "exp2-domain");
        nh_param.misc_param.flex_edit.label_num = label_num;
    }

     /*ipv4*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipv4-hdr");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR;
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_IPV4;

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-ip");
        if (0xFF != tmp_argi)
        {
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_SWAP_IP;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("replace-ipsa", nh_param.misc_param.flex_edit.ip_sa.ipv4, argv[tmp_argi + 1]);
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV4_ADDRESS("replace-ipda", nh_param.misc_param.flex_edit.ip_da.ipv4, argv[tmp_argi + 1]);
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA;
        }
    }

     /*ipv6*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ipv6-hdr");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR;
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("swap-ip");
        if (0xFF != tmp_argi)
        {
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_SWAP_IP;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ipsa");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("replace-ipsa", ipv6_address, argv[tmp_argi + 1]);
            nh_param.misc_param.flex_edit.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
            nh_param.misc_param.flex_edit.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
            nh_param.misc_param.flex_edit.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
            nh_param.misc_param.flex_edit.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA;
        }
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-ipda");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_IPV6_ADDRESS("replace-ipda", ipv6_address, argv[tmp_argi + 1]);
            nh_param.misc_param.flex_edit.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
            nh_param.misc_param.flex_edit.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
            nh_param.misc_param.flex_edit.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
            nh_param.misc_param.flex_edit.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA;
        }

        tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-label");
        if (0xFF != tmp_argi)
        {
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_FLOW_LABEL;
            CTC_CLI_GET_UINT32("flow-label", nh_param.misc_param.flex_edit.flow_label, argv[tmp_argi + 1]);
        }
    }

     /*ipv4 and ipv6 common*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("replace-protocol");
    if (tmp_argi != 0xFF)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_PROTOCOL;
        CTC_CLI_GET_UINT8("replace-protocol", nh_param.misc_param.flex_edit.protocol, argv[tmp_argi + 1]);
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", nh_param.stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("cid");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("cid", nh_param.cid, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-check-en");
    if (0xFF != tmp_argi)
    {
        nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_MTU_CHECK;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    CTC_CLI_NH_FLEX_L3HDR_SET(nh_param.misc_param.flex_edit)
    CTC_CLI_NH_FLEX_L4HDR_SET(nh_param.misc_param.flex_edit)

    nh_param.gport = gport;
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.type = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MISC, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = is_add?ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param):ctcs_nh_update_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = is_add?ctc_nh_add_misc(nhid, &nh_param):ctc_nh_update_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_misc_cpu_reason,
        ctc_cli_nh_add_misc_cpu_reason_cmd,
        "nexthop (add|update) misc cpu-reason NHID reason-id ID {port GPORT_ID|"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|lport-en VALUE|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MISC_STR,
        "TO CPU with custom reason",
        CTC_CLI_NH_ID_STR,
        "CPU Reason",
        "CPU Reason ID",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC,
        "Lport enable",
        "Lport enable value")
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0;
    ctc_misc_nh_param_t nh_param;
    uint16 cpu_reason_id = 0;
    uint32 gport = 0;
    uint8 tmp_argi;
    uint8 is_add = 0;
    uint8 lport_en = 0;

    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    if (0 == sal_memcmp(argv[0], "add", 3))
    {
        is_add = 1;
    }

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[1]);

    CTC_CLI_GET_UINT16("CPU Reason ID", cpu_reason_id, argv[2]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("port");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("lport-en");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT8("lport en", lport_en, argv[tmp_argi + 1]);
    }

    nh_param.type = CTC_MISC_NH_TYPE_TO_CPU;
    nh_param.gport = gport;
    nh_param.misc_param.cpu_reason.cpu_reason_id = cpu_reason_id;
    nh_param.misc_param.cpu_reason.lport_en = lport_en;

    if(g_ctcs_api_en)
    {
        ret = is_add?ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param):ctcs_nh_update_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = is_add?ctc_nh_add_misc(nhid, &nh_param):ctc_nh_update_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_nh_add_misc_leaf_spine,
        ctc_cli_nh_add_misc_leaf_spine_cmd,
        "nexthop add misc leaf-spine NHID port GPORT_ID {"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_MISC_STR,
        "Leaf to spine",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0;
    ctc_misc_nh_param_t nh_param;
    uint32 gport = 0;
    uint8 tmp_argi = 0;

    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    CTC_CLI_GET_UINT32("gport", gport, argv[1]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    nh_param.type = CTC_MISC_NH_TYPE_LEAF_SPINE;
    nh_param.gport = gport;


    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_nh_add_misc_over_l2_with_ts,
        ctc_cli_nh_add_misc_over_l2_with_ts_cmd,
        "nexthop (add|update) misc over-l2-hdr NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) ((port GPORT_ID | is-oif "CTC_CLI_L3IF_ALL_STR ") mac-da MAC  mac-sa MAC) (vlan-id VLAN |) (ether-type ETHTYPE) (flow-id FLOWID|) (vlan-edit "CTC_CLI_NH_EGS_VLAN_EDIT_STR "|) \
        (without-ext-hdr|){stats STATS_ID|cid CID|"CTC_CLI_NH_MISC_TRUNC_LEN_STR"|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_MISC_STR,
        "Over l2 header with time stamp",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Outgoing is interface",
        CTC_CLI_L3IF_ALL_DESC,
        "Mac destination address",
        CTC_CLI_MAC_FORMAT,
        "Mac source address",
        CTC_CLI_MAC_FORMAT,
        "Vlan id",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ether type",
        "<0-0xFFFF>",
        "Flow id",
        "Flow id value",
        "Vlan edit for raw packet",
        CTC_CLI_NH_EGS_VLAN_EDIT_DESC,
        "Without extender header",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "Cid",
        "Cid Value",
        CTC_CLI_NH_MISC_TRUNC_LEN_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid, dsnh_offset = 0;
    ctc_misc_nh_param_t nh_param;
    uint8 tmp_argi;
    uint32 gport = 0;
    mac_addr_t mac;
    ctc_nh_oif_info_t oif;
    ctc_vlan_egress_edit_info_t edit_info;
    uint8 is_add = 0;
    uint8 without_ext_hdr = 0;

    sal_memset(&edit_info, 0, sizeof(ctc_vlan_egress_edit_info_t));
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));
    sal_memset(&oif, 0, sizeof(ctc_nh_oif_info_t));

    if (0 == sal_memcmp(argv[0], "add", 3))
    {
        is_add = 1;
    }

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[1]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("port");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("is-oif");
    if (0xFF != tmp_argi)
    {
        nh_param.is_oif = 1;
        CTC_CLI_NH_PARSE_L3IF(tmp_argi + 1, oif.gport, oif.vid);
        sal_memcpy(&(nh_param.oif), &oif, sizeof(ctc_nh_oif_info_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
        sal_memcpy(nh_param.misc_param.over_l2edit.mac_da, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != tmp_argi)
    {
        if (tmp_argi == (argc - 1))
        {
            ctc_cli_out("%% Incomplete command\n");
            return CLI_ERROR;
        }
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac, argv[tmp_argi + 1]);
        sal_memcpy(nh_param.misc_param.over_l2edit.mac_sa, mac, sizeof(mac_addr_t));
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ether-type");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("ether-type", nh_param.misc_param.over_l2edit.ether_type, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("flow-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("flow-id", nh_param.misc_param.over_l2edit.flow_id, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }


    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan-id", nh_param.misc_param.over_l2edit.vlan_id, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("cvlan-edit-type");
    if (0xFF != tmp_argi)
    {
        ret = _ctc_vlan_parser_egress_vlan_edit(&edit_info, &argv[0], argc);
        nh_param.misc_param.over_l2edit.flag |= CTC_MISC_NH_OVER_L2_EDIT_VLAN_EDIT;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("without-ext-hdr");
    if (0xFF != tmp_argi)
    {
        without_ext_hdr = 1;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", nh_param.stats_id, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("cid");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("cid", nh_param.cid, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT16("truncated length", nh_param.truncated_len, argv[tmp_argi + 1]);
    }

    nh_param.type = (without_ext_hdr?CTC_MISC_NH_TYPE_OVER_L2:CTC_MISC_NH_TYPE_OVER_L2_WITH_TS);
    nh_param.dsnh_offset = dsnh_offset;
    nh_param.gport = gport;
    nh_param.misc_param.over_l2edit.vlan_info = edit_info;
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_MISC, &nh_param));
        return CLI_SUCCESS;
    }

    if(g_ctcs_api_en)
    {
        ret = is_add?ctcs_nh_add_misc(g_api_lchip, nhid, &nh_param):ctcs_nh_update_misc(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = is_add?ctc_nh_add_misc(nhid, &nh_param):ctc_nh_update_misc(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_nh_add_nh_mac,
        ctc_cli_nh_add_nh_mac_cmd,
        "nexthop add arp-id ARP_ID ((mac MAC|) {vlan VLAN_ID | gport GPHYPORT_ID (ecmp-if|)|})(discard | to-cpu|)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "ARP",
        "ARP ID",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "If selected ,indicate configure ecmp interface member's MACDA",
        "Drop",
        "Redirect to CPU")
{
    uint8 index = 0;
    int32 ret  = CLI_SUCCESS;
    ctc_nh_nexthop_mac_param_t param;
    uint16 arp_id = 0;

    sal_memset(&param, 0, sizeof(ctc_nh_nexthop_mac_param_t));

    CTC_CLI_GET_UINT16("ARP ID", arp_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", param.mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan");
    if (0xFF != index)
    {
        param.flag |= CTC_NH_NEXTHOP_MAC_VLAN_VALID;
        CTC_CLI_GET_UINT16("vlan", param.vlan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("gport", param.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ecmp-if");
    if (0xFF != index)
    {
        param.flag |= CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");
    if (0xFF != index)
    {
        param.flag |= CTC_NH_NEXTHOP_MAC_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("to-cpu");
    if (0xFF != index)
    {
        param.flag |= CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_nexthop_mac(g_api_lchip, arp_id, &param);
    }
    else
    {
      ret = ctc_nh_add_nexthop_mac(arp_id, &param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_nh_remove_nh_mac,
        ctc_cli_nh_remove_nh_mac_cmd,
        "nexthop remove arp-id ARP_ID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        "ARP",
        "ARP ID")
{

    int32 ret  = CLI_SUCCESS;
    uint16 arp_id = 0;

    CTC_CLI_GET_UINT16("ARP ID", arp_id, argv[0]);

    if(g_ctcs_api_en)
    {
       ret = ctcs_nh_remove_nexthop_mac(g_api_lchip, arp_id);
    }
    else
    {
        ret = ctc_nh_remove_nexthop_mac(arp_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_update_nh_mac,
        ctc_cli_nh_update_nh_mac_cmd,
        "nexthop update arp-id ARP_ID ((mac MAC|) {vlan VLAN_ID | gport GPHYPORT_ID (ecmp-if|)|})(discard | to-cpu|)",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        "ARP",
        "ARP ID",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "If selected ,indicate configure ecmp interface member's MACDA",
        "Drop",
        "Redirect to CPU")
{
    uint8 index = 0;
    int32 ret  = CLI_SUCCESS;
    ctc_nh_nexthop_mac_param_t new_param;
    uint16 arp_id = 0;

    sal_memset(&new_param, 0, sizeof(ctc_nh_nexthop_mac_param_t));

    CTC_CLI_GET_UINT16("ARP ID", arp_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", new_param.mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan");
    if (0xFF != index)
    {
        new_param.flag |= CTC_NH_NEXTHOP_MAC_VLAN_VALID;
        CTC_CLI_GET_UINT16("vlan", new_param.vlan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (0xFF != index)
    {
        if (!sal_memcmp("ecmp-if",  argv[index + 1], sizeof("ecmp-if")))
        {
            new_param.flag |= CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP;
        }

        CTC_CLI_GET_UINT32("gport", new_param.gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ecmp-if");
    if (0xFF != index)
    {
        new_param.flag |= CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");
    if (0xFF != index)
    {
        new_param.flag |= CTC_NH_NEXTHOP_MAC_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("to-cpu");
    if (0xFF != index)
    {
        new_param.flag |= CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_update_nexthop_mac(g_api_lchip, arp_id, &new_param);
    }
    else
    {
        ret = ctc_nh_update_nexthop_mac(arp_id, &new_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_NH_TRILL_PARAM_STR  "{egs-nickname NICKNAME | igs-nickname NICKNAME | ttl TTL | mtu-check-en mtu-size MTU | stats-id STATSID |}"
#define CTC_CLI_NH_TRILL_PARAM_DESC  \
       "Egress nickname",\
       "Egress nickname",\
        "Ingress nickname",\
        "Ingress nickname",\
        "TTL",\
        "TTL",\
        "Mtu check enable",\
        "Mtu size", \
        "Mtu size value",\
        CTC_CLI_STATS_ID_DESC,\
        CTC_CLI_STATS_ID_VAL

STATIC int32
_ctc_nexthop_cli_parse_trill_nexthop(char** argv, uint16 argc,
                                         ctc_nh_trill_param_t* p_nhinfo)
{
    uint8 tmp_argi;
    int ret = CLI_SUCCESS;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", p_nhinfo->mac, argv[tmp_argi + 1]);
    }

    /*vlan port if*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-port");
    if (0xFF != tmp_argi)
    {
        p_nhinfo->oif.is_l2_port = 1;
    }

    /*l3if info*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("vlan-if");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("routed-port");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_NH_PARSE_L3IF(tmp_argi, p_nhinfo->oif.gport, p_nhinfo->oif.vid);
    }

    /*nickname*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("egs-nickname");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("nickname", p_nhinfo->egress_nickname, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("igs-nickname");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("nickname", p_nhinfo->ingress_nickname, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*TTL*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", p_nhinfo->ttl, argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    /*MTU*/
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-check-en");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(p_nhinfo->flag, CTC_NH_TRILL_MTU_CHECK);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("mtu-size");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT16_RANGE("mtu-size", p_nhinfo->mtu_size, argv[tmp_argi + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stats-id");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("stats id", p_nhinfo->stats_id, argv[tmp_argi + 1]);
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_add_trill_nexthop,
        ctc_cli_nh_add_trill_nexthop_cmd,
        "nexthop add trill NHID (dsnh-offset OFFSET|alloc-dsnh-offset gchip GCHIP|) (unrov | fwd mac MAC)"CTC_CLI_L3IF_ALL_STR CTC_CLI_NH_TRILL_PARAM_STR,
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        CTC_CLI_NH_TRILL_STR,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_NH_ALLOC_DSNH_OFFSET_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Unresolved nexthop",
        "Forward Nexthop",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_L3IF_ALL_DESC,
        CTC_CLI_NH_TRILL_PARAM_DESC)
{

    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0, dsnh_offset = 0, tmp_argi = 0;
    ctc_nh_trill_param_t nh_param ;

    sal_memset(&nh_param, 0 , sizeof(nh_param));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32("DsNexthop Table offset", dsnh_offset, argv[tmp_argi + 1]);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov");
    if (0xFF != tmp_argi)
    {
        CTC_SET_FLAG(nh_param.flag, CTC_IP_NH_FLAG_UNROV);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd");
    if (0xFF != tmp_argi)
    {
        if (_ctc_nexthop_cli_parse_trill_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nh_param))
        {
            return CLI_ERROR;
        }
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("alloc-dsnh-offset");
    if (0xFF != tmp_argi)
    {
        uint8  alloc_dsnh_gchip = 0;
        CTC_CLI_GET_UINT8("alloc_dsnh_gchip", alloc_dsnh_gchip, argv[tmp_argi + 2]);
        CTC_ERROR_RETURN(_ctc_nexthop_cli_alloc_dsnh_offset(alloc_dsnh_gchip,CTC_NH_TYPE_TRILL, &nh_param));
        return CLI_SUCCESS;
    }

    nh_param.dsnh_offset = dsnh_offset;
    if (g_ctcs_api_en)
    {
        ret = ctcs_nh_add_trill(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_add_trill(nhid, &nh_param);
    }


    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_update_trill_nexthop,
        ctc_cli_nh_update_trill_nexthop_cmd,
        "nexthop update trill NHID (fwd2unrov| \
        ((fwd-attr|unrov2fwd (dsnh-offset OFFSET|)) mac MAC "CTC_CLI_L3IF_ALL_STR CTC_CLI_NH_TRILL_PARAM_STR"))",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_UPDATE_STR,
        CTC_CLI_NH_TRILL_STR,
        CTC_CLI_NH_ID_STR,
        "Forward nexthop to unresolved nexthop",
        "Update forward nexthop",
        "Unresolved nexthop to forward nexthop",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_L3IF_ALL_DESC,
        CTC_CLI_NH_TRILL_PARAM_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint32 nhid = 0, tmp_argi = 0;
    ctc_nh_trill_param_t nh_param ;

    sal_memset(&nh_param, 0 , sizeof(nh_param));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd2unrov");
    if (0xFF != tmp_argi)
    {
        nh_param.upd_type = CTC_NH_UPD_FWD_TO_UNRSV;
    }
    else
    {
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("fwd-attr");
        if (0xFF != tmp_argi)
        {
            nh_param.upd_type = CTC_NH_UPD_FWD_ATTR;
        }
        else
        {
            uint32 temp_argi = 0;
            tmp_argi = CTC_CLI_GET_ARGC_INDEX("unrov2fwd");
            if (0xFF != tmp_argi)
            {
                temp_argi = CTC_CLI_GET_ARGC_INDEX("dsnh-offset");
                if (0xFF != temp_argi)
                {
                    CTC_CLI_GET_UINT32("DsNexthop Table offset", nh_param.dsnh_offset, argv[temp_argi + 1]);
                    tmp_argi = temp_argi;
                }
                nh_param.upd_type = CTC_NH_UPD_UNRSV_TO_FWD;
            }
        }
    }

    if ((CTC_NH_UPD_FWD_ATTR == nh_param.upd_type) || (CTC_NH_UPD_UNRSV_TO_FWD == nh_param.upd_type))
    {
        if (_ctc_nexthop_cli_parse_trill_nexthop(&(argv[tmp_argi]), (argc - tmp_argi), &nh_param))
        {
            return CLI_ERROR;
        }
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_nh_update_trill(g_api_lchip, nhid, &nh_param);
    }
    else
    {
        ret = ctc_nh_update_trill(nhid, &nh_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_remove_trill_nexthop,
        ctc_cli_nh_remove_trill_nexthop_cmd,
        "nexthop remove trill NHID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        CTC_CLI_NH_IP_TUNNEL_STR,
        CTC_CLI_NH_TRILL_STR)
{
    uint32 nhid;
    int32 ret;

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_trill(g_api_lchip, nhid);
    }
    else
    {
        ret = ctc_nh_remove_trill(nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}



CTC_CLI(ctc_cli_nh_get_nhid_by_groupid,
        ctc_cli_nh_get_nhid_by_groupid_cmd,
        "show nexthop mcast (group-id GROUPID) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "Mcast",
        "Mcast group",
        "Group id",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 nhid = 0;
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint32 group_id = 0;


	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("group-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("group-id", group_id, argv[index + 1]);
    }


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_get_mcast_nh(g_api_lchip, group_id, &nhid);
    }
    else
    {
        ret = ctc_nh_get_mcast_nh(group_id, &nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return ret;
    }

    ctc_cli_out("Mcast Nexthop Id:%u\n", nhid);
    return ret;
}

CTC_CLI(ctc_cli_nh_get_resolved_dsnh_offset,
        ctc_cli_nh_get_resolved_dsnh_offset_cmd,
        "show nexthop resoved dsnh-offset (bridge|bypass|mirror) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        "Resoved",
        "Dsnh offset",
        "Reserved L2Uc Nexthop",
        "Reserved bypass Nexthop",
        "Reserved mirror Nexthop",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    uint32 dsnh_offset = 0;
    ctc_nh_reserved_dsnh_offset_type_t type = CTC_NH_RES_DSNH_OFFSET_TYPE_MAX;
    char* nh_type_str[CTC_NH_RES_DSNH_OFFSET_TYPE_MAX + 1] = {"Bridge_nh", "Bypass_nh", "Mirror_nh"};

	index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bridge");
    if (0xFF != index)
    {
        type = CTC_NH_RES_DSNH_OFFSET_TYPE_BRIDGE_NH;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bypass");
    if (0xFF != index)
    {
        type = CTC_NH_RES_DSNH_OFFSET_TYPE_BYPASS_NH;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mirror");
    if (0xFF != index)
    {
        type = CTC_NH_RES_DSNH_OFFSET_TYPE_MIRROR_NH;
    }


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_get_resolved_dsnh_offset(g_api_lchip, type, &dsnh_offset);
    }
    else
    {
        ret = ctc_nh_get_resolved_dsnh_offset(type, &dsnh_offset);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return ret;
    }

    ctc_cli_out("%-20s :%s\n", "nexthop type", nh_type_str[type]);
    ctc_cli_out("%-20s :%u\n", "resoved offset", dsnh_offset);
    return ret;
}

CTC_CLI(ctc_cli_nh_get_nh_info,
        ctc_cli_nh_get_nh_info_cmd,
        "show nexthop NHID info (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ID_STR,
        "Nexthop info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 nhid;
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 loop = 0;
    uint8 flag = 0;
    uint8 idx0 = 0;
    uint8 idx1 = 0;
    uint32 gport = 0;
    uint16 lport = 0;
    ctc_nh_info_t nh_info;
    char* str[4] = {"L2 MCAST", "IP MCAST", "LOCAL NH", "REMOTE"};
    char buf[10] = {0};
    char buf_s[1] = {0};

    sal_memset(&nh_info, 0, sizeof(ctc_nh_info_t));

    /* the maximum member num which can be got in one time */
    nh_info.buffer_len = 16;

    MALLOC_ZERO(MEM_NEXTHOP_MODULE, nh_info.buffer, nh_info.buffer_len * sizeof(ctc_mcast_nh_param_member_t));

    CTC_CLI_GET_UINT32("NexthopID", nhid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    do
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_get_nh_info(lchip, nhid, &nh_info);
        }
        else
        {
            ret = ctc_nh_get_nh_info(nhid, &nh_info);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
            return ret;
        }

        if (CTC_FLAG_ISSET(nh_info.flag, CTC_NH_INFO_FLAG_IS_MCAST))
        {
            /* Mcast Nexthop */
            if (flag == 0)
            {
                ctc_cli_out("Destid: \n");
                ctc_cli_out(" Gport     : BRGMC & IPMC local member\n");
                ctc_cli_out(" Profile Id: Remote member\n");
                ctc_cli_out(" NHID      : Local nhid member\n");
                ctc_cli_out("\r\n");

                ctc_cli_out("Flags: \n");
                ctc_cli_out(" D-Source check discard   R-Reflective\n");
                ctc_cli_out("\r\n");

                ctc_cli_out("%-15s%-8s%-8s%-s\n", "member_type", "vid", "Flags", "destid");
                ctc_cli_out("--------------------------------------\n");
                flag = 1;
            }

            for (loop = 0; loop < nh_info.valid_number; loop++)
            {
                buf[0] = '\0';
                if (nh_info.buffer[loop].is_source_check_dis)
                {
                    buf_s[0] = 'D';
                    sal_strncat(buf, buf_s, 1);
                }
                if (nh_info.buffer[loop].is_reflective)
                {
                    buf_s[0] = 'R';
                    sal_strncat(buf, buf_s, 1);
                }
                if ('\0' == buf[0])
                {
                    buf_s[0] = '-';
                    sal_strncat(buf, buf_s, 1);
                }

                if ((CTC_NH_PARAM_MEM_BRGMC_LOCAL == nh_info.buffer[loop].member_type) || (CTC_NH_PARAM_MEM_IPMC_LOCAL == nh_info.buffer[loop].member_type))
                {
                    if (nh_info.buffer[loop].port_bmp_en)
                    {
                        /* port bitmap mode */
                        for (idx0 = 0; idx0 < 16; idx0++)
                        {
                            if (nh_info.buffer[loop].port_bmp[idx0] == 0)
                            {
                                continue;
                            }
                            for (idx1 = 0; idx1 < CTC_UINT32_BITS; idx1++)
                            {
                                if (CTC_IS_BIT_SET(nh_info.buffer[loop].port_bmp[idx0], idx1))
                                {
                                    lport = (idx0 * CTC_UINT32_BITS + idx1);
                                    gport = CTC_MAP_LPORT_TO_GPORT(nh_info.buffer[loop].gchip_id, lport);
                                    ctc_cli_out("%-15s%-8d", str[nh_info.buffer[loop].member_type], nh_info.buffer[loop].vid);
                                    ctc_cli_out("%-8s", buf);
                                    ctc_cli_out("0x%04X\n", gport);
                                }
                            }
                        }
                    }
                    else
                    {
                        /* lport */
                        gport = CTC_MAP_LPORT_TO_GPORT(nh_info.buffer[loop].gchip_id, nh_info.buffer[loop].destid);
                        ctc_cli_out("%-15s%-8d", str[nh_info.buffer[loop].member_type], nh_info.buffer[loop].vid);
                        ctc_cli_out("%-8s", buf);
                        ctc_cli_out("0x%04X\n", gport);
                    }
                }
                else if (CTC_NH_PARAM_MEM_REMOTE == nh_info.buffer[loop].member_type)
                {
                    ctc_cli_out("%-15s%-8d", str[nh_info.buffer[loop].member_type], nh_info.buffer[loop].vid);
                    ctc_cli_out("%-8s", buf);
                    ctc_cli_out("0x%04X\n", nh_info.buffer[loop].destid);
                }
                else if (CTC_NH_PARAM_MEM_LOCAL_WITH_NH == nh_info.buffer[loop].member_type)
                {
                    ctc_cli_out("%-15s%-8d", str[nh_info.buffer[loop].member_type], nh_info.buffer[loop].vid);
                    ctc_cli_out("%-8s", buf);
                    ctc_cli_out("%-u\n", nh_info.buffer[loop].ref_nhid);
                }
            }
            /* clear statistic */
            nh_info.valid_number = 0;
            nh_info.start_index = nh_info.next_query_index;

        }
        else if (CTC_FLAG_ISSET(nh_info.flag, CTC_NH_INFO_FLAG_IS_ECMP))
        {
            /* ECMP Nexthop */
            ctc_cli_out("%-8s%-15s\n", "NHID", "TYPE");
            ctc_cli_out("-------------------\n");

            ctc_cli_out("%-8dECMP\n", nhid);

            ctc_cli_out("\n%-6s%s\n", "No.", "MEMBER_NHID");
            ctc_cli_out("-------------------\n");

            for (loop = 0; loop < nh_info.valid_ecmp_cnt; loop++)
            {
                ctc_cli_out("%-6d%d\n", loop, nh_info.ecmp_mem_nh[loop]);
            }

            nh_info.is_end = 1;
        }
        else if (CTC_FLAG_ISSET(nh_info.flag, CTC_NH_INFO_FLAG_IS_APS_EN))
        {
            /* APS Nexthop */
            ctc_cli_out("%-15s%-15d\n", "NHID", nhid);
            ctc_cli_out("---------------------------\n");
            ctc_cli_out("%-15s0x%-8X\n", "Flag:", nh_info.flag);
            ctc_cli_out("%-15s%-8d\n", "Group id:", nh_info.gport);
            nh_info.is_end = 1;
        }
        else
        {
            /* Ucast Nexthop */
            ctc_cli_out("%-15s%-15d\n", "NHID", nhid);
            ctc_cli_out("%-15s%-15d\n", "nh-type", nh_info.nh_type);
            ctc_cli_out("---------------------------\n");
            ctc_cli_out("%-15s0x%-8X\n", "Flag:", nh_info.flag);
            ctc_cli_out("%-15s0x%04X\n", "Gport:", nh_info.gport);
            ctc_cli_out("%-15s0x%-8X\n", "Dsnh offset:", nh_info.dsnh_offset[lchip]);

            nh_info.is_end = 1;
        }


    }while (!nh_info.is_end);

    return ret;
}

CTC_CLI(ctc_cli_nh_add_udf_profile,
        ctc_cli_nh_add_udf_profile_cmd,
        "nexthop add udf-profile PROFILE_ID offset-type TYPE length LEN udf-data DATA {ether-type TYPE|option-hdr-length LEN|}{udf-edit EDIT_TYPE1 OFFSET1 LEN1  \
        (EDIT_TYPE2 OFFSET2 LEN2 (EDIT_TYPE3 OFFSET3 LEN3 (EDIT_TYPE4 OFFSET4 LEN4 |)|)|)|}",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_ADD_STR,
        "UDF Profile",
        "Profile ID",
        "UDF offset type",
        "Type value(0:IP+UDP+UDF, 1:IP+GRE(without key)+UDF, 2:IP+GRE(with key)+UDF, 3:IP+UDF, 4:RAW)",
        "UDF length",
        "Length",
        "UDF date",
        "Data",
        "Ether type",
        "Type",
        "Option hdr length",
        "Length",
        "UDF edit",
        "UDF edit type1(0:replace none, 1:replace from nexthop, 2:replace from payload length, 3:replace from metadata)",
        "UDF edit offset1",
        "UDF edit len1",
        "UDF edit type2(0:replace none, 1:replace from nexthop, 2:replace from payload length, 3:replace from metadata)",
        "UDF edit offset2",
        "UDF edit len2",
        "UDF edit type3(0:replace none, 1:replace from nexthop, 2:replace from payload length, 3:replace from metadata)",
        "UDF edit offset3",
        "UDF edit len3",
        "UDF edit type4(0:replace none, 1:replace from nexthop, 2:replace from payload length, 3:replace from metadata)",
        "UDF edit offset4",
        "UDF edit len4")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 edit_num = 0;
    uint8 sub_idx = 0;

    ctc_nh_udf_profile_param_t profile_param;
    sal_memset(&profile_param, 0, sizeof(profile_param));

    CTC_CLI_GET_UINT8("PROFILE ID", profile_param.profile_id, argv[0]);
    CTC_CLI_GET_UINT8("OFFSET TYPE", profile_param.offset_type, argv[1]);
    CTC_CLI_GET_UINT16("UDF length", profile_param.edit_length, argv[2]);

    for (sub_idx = 0; sub_idx < 4; sub_idx++)
    {
        sal_sscanf(argv[3]+8*sub_idx, "%8x", (uint32*)&profile_param.edit_data[sub_idx]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ether-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("ether-type", profile_param.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("option-hdr-length");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("option-hdr-length", profile_param.hdr_option_length, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("udf-edit");
    if (0xFF != index)
    {
        edit_num = (argc - index - 1)/3;

        for (sub_idx = 0; sub_idx < edit_num; sub_idx++)
        {
            CTC_CLI_GET_UINT8("type", profile_param.edit[sub_idx].type, argv[index + 1 + sub_idx*3]);
            CTC_CLI_GET_UINT8("offset", profile_param.edit[sub_idx].offset, argv[index + 2 + sub_idx*3]);
            CTC_CLI_GET_UINT8("length", profile_param.edit[sub_idx].length, argv[index + 3 + sub_idx*3]);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_udf_profile(g_api_lchip, &profile_param);
    }
    else
    {
        ret = ctc_nh_add_udf_profile(&profile_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return ret;
    }

    return ret;
}

CTC_CLI(ctc_cli_nh_remove_udf_profile,
        ctc_cli_nh_remove_udf_profile_cmd,
        "nexthop remove udf-profile ID",
        CTC_CLI_NH_M_STR,
        CTC_CLI_NH_DEL_STR,
        "UDF Profile",
        "Profile ID")
{
    uint16 profile_id = 0;
    int32 ret;

    CTC_CLI_GET_UINT16("profile", profile_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_udf_profile(g_api_lchip, profile_id);
    }
    else
    {
        ret = ctc_nh_remove_udf_profile(profile_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_nh_set_global_config,
        ctc_cli_nh_set_global_config_cmd,
        "nexthop global config (max-ecmp ECMP)",
        CTC_CLI_NH_M_STR,
        "Global",
        "Config",
        "Max ecmp",
        "Ecmp value")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_nh_global_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_nh_global_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("max-ecmp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("max-ecmp", glb_cfg.max_ecmp, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_set_global_config(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_nh_set_global_config(&glb_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return ret;
    }

    return ret;
}

int32
ctc_nexthop_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_xlate_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_xlate_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_get_l2uc_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_ipuc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_ipuc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_ipuc_cmd);

#ifndef HUMBER
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_mpls_tunnel_label_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_mpls_tunnel_label_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_mpls_push_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_mpls_push_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_aps_mpls_push_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_aps_mpls_push_nexthop_cmd);
#endif

    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_mpls_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_mpls_pop_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_mpls_pop_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_aps_mpls_pop_nexthop_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_mpls_nexthop_to_unrov_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_swap_mpls_tunnel_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_aps_xlate_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_aps_xlate_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_iloop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_iloop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_rspan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_rspan_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_ecmp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_ecmp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_ecmp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_cfg_max_ecmp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_show_max_ecmp_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_add_ip_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_update_ip_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_update_ip_tunnel_to_unrov_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_remove_ip_tunnel_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_mcast_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_mcast_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_remove_l2mc_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_remove_ipmc_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_remove_nh_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_remove_remote_member_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_l2hdr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_misc_l2hdr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_l2l3hdr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_flexhdr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_over_l2_with_ts_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_cpu_reason_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_misc_leaf_spine_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_nh_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_nh_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_nh_mac_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_trill_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_trill_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_update_trill_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_get_nhid_by_groupid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_get_resolved_dsnh_offset_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_get_nh_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_set_global_config_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_nh_add_udf_profile_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_nh_remove_udf_profile_cmd);

    return CLI_SUCCESS;
}

