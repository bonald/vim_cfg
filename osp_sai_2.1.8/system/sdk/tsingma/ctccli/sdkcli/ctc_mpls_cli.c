/**
 @file ctc_mpls_cli.c

 @date 2010-03-16

 @version v2.0

 The file apply clis of ipuc module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_mpls_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"

#define CTC_CLI_MPLS_L2VPN_OLD_OPT \
    "| igmp-snooping-en | vsi-learning-disable\
        | mac-security-vsi-discard "
#define CTC_CLI_MPLS_POLICER_STR \
    "Set policer type after setting id,default type is flow policer"

#define CTC_CLI_MPLS_COMM_OPT \
    "stats STATS_ID |trust-outer-exp |qos-use-outer-info | acl-use-outer-info | flex-edit |"

#define CTC_CLI_MPLS_COMM_STR \
    "Statistics",\
    "1~0xFFFFFFFF",\
    "Use outer label's exp",\
    "Use the QOS information from outer header",\
    "Use the ACL information from outer header",\
    "Edit label directly, do not need to edit outer eth header"

#define CTC_CLI_MPLS_SPACE_DESC "Label space id, <0-255>, space 0 is platform space"


CTC_CLI(ctc_cli_mpls_add_normal_ilm_tcam,
        ctc_cli_mpls_add_normal_ilm_tcam_cmd,
        "mpls ilm (add | update) space SPACEID label LABEL {nexthop-id NHID  \
        | (uniform | short-pipe | pipe) | (pop (decap |) (interface-label-space SPACEID|))\
        | oam | aps-select GROUP_ID (working-path|protection-path)\
        | trust-outer-exp | (ipv6 | ip) | pwid PWID | control-word \
        | policer-id POLICERID (service-policer|) | is-bos | assign-port GPORT_ID |"CTC_CLI_MPLS_COMM_OPT"} (use-flex |)",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Add ilm",
        "Update Exist Entry",
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, reference to show space",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Uniform model",
        "Short pipe model",
        "Pipe model",
        "Pop operation label",
        "Decap Inner Packet",
        "Output mpls interface label space",
        "Interface label space id, <1-255>, 0 means invalid",
        "OAM Enable",
        "APS select ",
        "GROUP_ID",
        "Indicate working path",
        "Indicate protection path",
        "Use outer label's exp",
        "Inner packet type is ipv6",
        "Inner packet type is ipv4 or ipv6",
        "PW Id",
        "PW Id",
        "Control word used",
        "Set Policer ID",
        "Policer ID 1 - 16383",
        CTC_CLI_MPLS_POLICER_STR,
        "label is VC label",
        "Assign output gport",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_MPLS_COMM_STR,
        "Is tcam")
{
    int32 ret = CLI_SUCCESS;
    uint32 spaceid = 0;
    uint8 index = 0;
    ctc_mpls_ilm_t ilm_info;
    bool b_nh = FALSE;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));


    index = CTC_CLI_GET_ARGC_INDEX("space");

    CTC_CLI_GET_UINT8_RANGE("space", spaceid, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ilm_info.spaceid = spaceid;

    CTC_CLI_GET_UINT32_RANGE("label", ilm_info.label, argv[2], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("nexthop-id");
    if (0xFF != index)
    {
        b_nh = TRUE;
        CTC_CLI_GET_UINT32("nexthop-id", ilm_info.nh_id, argv[index + 1]);
    }

    /* default is uniform mode!!! */
    ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;

    index = CTC_CLI_GET_ARGC_INDEX("pipe");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_PIPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("short-pipe");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_SHORT_PIPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pop");
    if (0xFF != index)
    {
        ilm_info.pop = TRUE;
    }
    else if (FALSE == b_nh)
    {
        ctc_cli_out("%% %s \n\r", "Only pop mode don't need to config nexthop!");
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decap");
    if (0xFF != index)
    {
        ilm_info.decap = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interface-label-space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("interface-label-space", ilm_info.out_intf_spaceid , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        ilm_info.oam_en = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_APS_SELECT;
        CTC_CLI_GET_UINT16_RANGE("aps-select ", ilm_info.flw_vrf_srv_aps.aps_select_grp_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("aps-select ", ilm_info.aps_select_grp_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        if (CLI_CLI_STR_EQUAL("protection-path", index + 2))
        {
            ilm_info.aps_select_protect_path = 1;
        }
        else
        {
            ilm_info.aps_select_protect_path = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_STATS;
        CTC_CLI_GET_UINT32("stats", ilm_info.stats_id, argv[index + 1]);
    }

    ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV4;
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("trust-outer-exp");
    if (0xFF != index)
    {
        ilm_info.trust_outer_exp = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO);
    }

    /* qos use outer info */
    index  = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        ilm_info.qos_use_outer_info = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pwid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("pwid", ilm_info.pwid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-word");
    if (0xFF != index)
    {
        ilm_info.cwen = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("policer-id", ilm_info.policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-policer");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_SERVICE_POLICER_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-bos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_BOS_LABLE );
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    index = CTC_CLI_GET_ARGC_INDEX("flex-edit");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT );
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", ilm_info.gport, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);
    }

    ilm_info.type = CTC_MPLS_LABEL_TYPE_NORMAL;

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_add_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_add_ilm(&ilm_info);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_update_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_update_ilm(&ilm_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_set_ilm_property,
        ctc_cli_mpls_set_ilm_property_cmd,
        "mpls ilm  space SPACEID label LABEL property (qos-domain DOMAIN | \
        (data-discard | to-cpu) VAL | aps-select GROUP_ID (working-path|protection-path) \
        | inner-router-mac MAC | l-lsp EXP | oam-mp-chk-type VAL \
        |qos-map (assign (priority PRIORITY) (color COLOR) | elsp (domain DOMAIN)| llsp (domain DOMAIN) (priority PRIORITY)| disable) \
        |arp-action (copy-to-cpu|forward|redirect-to-cpu|discard |) | stats-enable STATSID | deny-learning-enable VAL | mac-limit-discard-en VAL \
        | mac-limit-action (none|forward|discard|redirect-to-cpu) | station-move-discard-en VAL | metadata VAL | dcn-action VAL) (use-flex|) ",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Set ilm property",
        "Qos domain",
        "0~7 valid domain and 0xFF means disable",
        "MPLS data discard, OAM not",
        "redirect OAM packet to CPU",
        "1 enabel; 0 disable",
        "APS select ",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Indicate working path",
        "Indicate protection path",
        "Router mac for Inner L2 MacDa",
        CTC_CLI_MAC_FORMAT,
        "L-LSP mode",
        "MPLS EXP value(0-7)",
        "OAM check type",
        "OAM check type value",
        "Qos map",
        "Assign mode, User define priority and color, donot using exp domain",
        "priority",
        "priority value",
        "color",
        "color value 1:red,2:yellow,3:green",
        "Elsp mode, priority and color mapping from exp domain",
        "domain",
        "domain value",
        "Llsp mode, user define priority and color maping from exp domain",
        "domain",
        "domain value",
        "priority",
        "priority value",
        "disable qos map",
        "arp action",
        "copy to cpu",
        "forward",
        "redirect to cpu",
        "discard",
        "Enable stats",
        "Stats id",
        "Enable deny learning",
        "Enable: 1;Disable: 0",
        "Enabel mac limit discard",
        "Enable: 1;Disable: 0",
        "Mac limit action",\
        "None",\
        "Normal forwarding",\
        "Discard",\
        "Redirect to cpu",\
        "Enable station move discard",
        "Enable: 1;Disable: 0",
        "Metadata",
        "Metadata value",
        "Dcn action",
        "Dcn value",
        "Is tcam")
{
    int32 ret = CLI_SUCCESS;
    uint32 spaceid  = 0;
    uint8 index     = 0;
    uint32 value    = 0;

    ctc_mpls_property_t mpls_pro;
    ctc_mpls_ilm_t ilm_info;
    mac_addr_t mac_addr;
    ctc_mpls_ilm_qos_map_t ilm_qos_map;
    uint32 arp_action_type = 0;

    sal_memset(&mpls_pro, 0, sizeof(ctc_mpls_property_t));
    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&mac_addr, 0, sizeof(mac_addr_t));
    sal_memset(&ilm_qos_map, 0, sizeof(ctc_mpls_ilm_qos_map_t));


    CTC_CLI_GET_UINT8_RANGE("space", spaceid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    mpls_pro.space_id = spaceid;

    CTC_CLI_GET_UINT32_RANGE("label", mpls_pro.label, argv[1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("qos-domain");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_QOS_DOMAIN;
        CTC_CLI_GET_UINT32("qos domain", value, argv[index + 1]);
        mpls_pro.value  = &value;
    }


    index = CTC_CLI_GET_ARGC_INDEX("data-discard");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_DATA_DISCARD;
        CTC_CLI_GET_UINT32("data discard", value, argv[index + 1]);
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_APS_SELECT;
        CTC_CLI_GET_UINT16_RANGE("aps-select ", ilm_info.aps_select_grp_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        ilm_info.flw_vrf_srv_aps.aps_select_grp_id = ilm_info.aps_select_grp_id;

        index = CTC_CLI_GET_ARGC_INDEX("protection-path");
        if (0xFF != index)
        {
            ilm_info.aps_select_protect_path = 1;
        }

        mpls_pro.value  = (void *)(&ilm_info);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_OAM_TOCPU;
        CTC_CLI_GET_UINT32("data discard", value, argv[index + 1]);
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("inner-router-mac");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_ROUTE_MAC;
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac_addr, argv[index + 1]);
        mpls_pro.value  = &mac_addr;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l-lsp");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_LLSP;
        CTC_CLI_GET_UINT8("MPLS EXP", value, argv[index + 1]);
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam-mp-chk-type");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_OAM_MP_CHK_TYPE;
        CTC_CLI_GET_UINT8("OAM mp chk type", value, argv[index + 1]);
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign");
    if (0xFF != index)
    {
        ilm_qos_map.mode = CTC_MPLS_ILM_QOS_MAP_ASSIGN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("elsp");
    if (0xFF != index)
    {
        ilm_qos_map.mode = CTC_MPLS_ILM_QOS_MAP_ELSP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("llsp");
    if (0xFF != index)
    {
        ilm_qos_map.mode = CTC_MPLS_ILM_QOS_MAP_LLSP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        ilm_qos_map.mode = CTC_MPLS_ILM_QOS_MAP_DISABLE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("priority value", ilm_qos_map.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("color");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("color value", ilm_qos_map.color, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("domain");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("domain value", ilm_qos_map.exp_domain, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-map");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_QOS_MAP;
        mpls_pro.value = &ilm_qos_map;
    }

    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
    if (0xFF != index)
    {
        arp_action_type = CTC_EXCP_FWD_AND_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("forward");
    if (0xFF != index)
    {
        arp_action_type = CTC_EXCP_NORMAL_FWD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("redirect-to-cpu");
    if (0xFF != index)
    {
        arp_action_type = CTC_EXCP_DISCARD_AND_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        arp_action_type = CTC_EXCP_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-action");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_ARP_ACTION;
        mpls_pro.value = &arp_action_type;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats-enable");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("stats id", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_STATS_EN;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("deny-learning-enable");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("enable deny learning", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_DENY_LEARNING_EN;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-discard-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("enable mac limit discard", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_MAC_LIMIT_DISCARD_EN;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-action");
    if (0xFF != index)
    {
        mpls_pro.property_type = CTC_MPLS_ILM_MAC_LIMIT_ACTION;
        if (CLI_CLI_STR_EQUAL("none", index+1))
        {
            value = CTC_MACLIMIT_ACTION_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            value = CTC_MACLIMIT_ACTION_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("discard", index+1))
        {
            value = CTC_MACLIMIT_ACTION_DISCARD;
        }
        else
        {
            value = CTC_MACLIMIT_ACTION_TOCPU;
        }
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("station-move-discard-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("enable station move discard", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_STATION_MOVE_DISCARD_EN;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("metadata", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_METADATA;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dcn-action");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dcan action", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_DCN_ACTION;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("cid", value, argv[index + 1]);
        mpls_pro.property_type = CTC_MPLS_ILM_CID;
        mpls_pro.value  = &value;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        mpls_pro.use_flex = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mpls_set_ilm_property(g_api_lchip, &mpls_pro);
    }
    else
    {
        ret = ctc_mpls_set_ilm_property(&mpls_pro);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_dt2_mpls_add_l3vpn_ilm_tcam,
        ctc_cli_mpls_add_l3vpn_ilm_tcam_cmd,
        "mpls ilm (add | update) l3vpn-vc SPACEID LABEL NHID VRFID {(uniform | short-pipe | pipe)| \
        (ipv6 | ip) | oam |"CTC_CLI_MPLS_COMM_OPT"} (use-flex |) (pwid PWID|)",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Add ilm",
        "Update ilm",
        "L3vpn vc label",
        "Label space id, <0-255>, space 0 is platform space",
        "Label, <0-1048575>",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_VRFID_ID_DESC,
        "Uniform model",
        "Short pipe model",
        "Pipe model",
        "Inner packet type is ipv6",
        "Inner packet type is ipv4 or ipv6",
        "OAM Enable",
        CTC_CLI_MPLS_COMM_STR,
        "Is tcam",
        "PW Id",
        "PW Id")
{
    int32 ret = CLI_SUCCESS;
    uint32 spaceid = 0;
    uint32 vrfid = 0;
    uint8 index = 0;
    ctc_mpls_ilm_t ilm_info;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));

    CTC_CLI_GET_UINT8_RANGE("space", spaceid, argv[1], 0, CTC_MAX_UINT8_VALUE);
    ilm_info.spaceid = spaceid;

    CTC_CLI_GET_UINT32_RANGE("label", ilm_info.label, argv[2], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32("nexthop", ilm_info.nh_id, argv[3]);

    CTC_CLI_GET_UINT16_RANGE("vrf", vrfid, argv[4], 0, CTC_MAX_UINT16_VALUE);
    ilm_info.flw_vrf_srv_aps.vrf_id = vrfid;
    ilm_info.id_type |= CTC_MPLS_ID_VRF;


    ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;

    index = CTC_CLI_GET_ARGC_INDEX("uniform");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;
    }
    else if (0xFF != CTC_CLI_GET_ARGC_INDEX("short-pipe"))
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_SHORT_PIPE;
    }
    else
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_PIPE;
    }

    /* qos use outer info */
    index  = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        ilm_info.qos_use_outer_info = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO);
    }

    ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV4;
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("trust-outer-exp");
    if (0xFF != index)
    {
        ilm_info.trust_outer_exp = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        ilm_info.oam_en = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_STATS;
        CTC_CLI_GET_UINT32("stats", ilm_info.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    index = CTC_CLI_GET_ARGC_INDEX("flex-edit");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT );
    }

    ilm_info.type = CTC_MPLS_LABEL_TYPE_L3VPN;

    index = CTC_CLI_GET_ARGC_INDEX("pwid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("pwid", ilm_info.pwid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_add_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_add_ilm(&ilm_info);
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_update_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_update_ilm(&ilm_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_add_gateway_pw_tcam,
        ctc_cli_mpls_add_gateway_pw_tcam_cmd,
        "mpls gateway (add | update) (space SPACEID |) LABEL logic-port PORT inner-pkt (ip | eth)\
        {vrf VRFID |fid FID|nexthop-id NHID} {encapsulation-mode (raw|tagged) | control-word |} (use-flex |)",
        CTC_CLI_MPLS_M_STR,
        "gateway pw mapping",
        "Add gateway label",
        "label space",
        CTC_CLI_MPLS_SPACE_DESC,
        "SPACEID",
        "Label, <0-1048575>",
        "Logic Port",
        "Logic Port ID",
        "Inner packet",
        "Inner packet type is IP",
        "Inner packet type is Ethernet",
        "VRF",
        CTC_CLI_VRFID_ID_DESC,
        "FID",
        CTC_CLI_FID_ID_DESC,
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "PW encapsulation mode",
        "Raw mode",
        "Tagged mode",
        "Control word used",
        "Is tcam")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_mpls_ilm_t ilm_info;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));

    index = CTC_CLI_GET_ARGC_INDEX("space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("space", ilm_info.spaceid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT32("label", ilm_info.label, argv[index + 2]);
        CTC_CLI_GET_UINT32("logic-port", ilm_info.pwid, argv[index + 3]);
    }
    else
    {
        ilm_info.spaceid = 0;
        CTC_CLI_GET_UINT32("label", ilm_info.label, argv[0]);
        CTC_CLI_GET_UINT32("logic-port", ilm_info.pwid, argv[1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vrf", ilm_info.vrf_id, argv[index + 1]);
        ilm_info.id_type |= CTC_MPLS_ID_VRF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("fid", ilm_info.fid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_RAW;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-word");
    if (0xFF != index)
    {
        ilm_info.cwen = 1;
    }

    ilm_info.pw_mode = CTC_MPLS_L2VPN_RAW;
    index  = CTC_CLI_GET_ARGC_INDEX("encapsulation-mode");
    if (0xFF != index)
    {
        index  = CTC_CLI_GET_ARGC_INDEX("raw");
        if (0xFF != index)
        {
            ilm_info.pw_mode = CTC_MPLS_L2VPN_RAW;
        }
        else
        {
            ilm_info.pw_mode = CTC_MPLS_L2VPN_TAGGED;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nexthop-id", ilm_info.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    ilm_info.type = CTC_MPLS_LABEL_TYPE_GATEWAY;

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_add_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_add_ilm(&ilm_info);
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mpls_update_ilm(g_api_lchip, &ilm_info);
        }
        else
        {
            ret = ctc_mpls_update_ilm(&ilm_info);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_mpls_remove_gateway_pw_tcam,
        ctc_cli_mpls_remove_gateway_pw_tcam_cmd,
        "mpls gateway remove (space SPACEID |) LABEL (use-flex |)",
        CTC_CLI_MPLS_M_STR,
        "gateway pw mapping",
        "Remove gateway pw mapping",
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "gateway pw incoming label, <0-1048575>",
        "Is tcam")
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    ctc_mpls_ilm_t pw_info;

    sal_memset(&pw_info, 0, sizeof(pw_info));
    idx = CTC_CLI_GET_ARGC_INDEX("space");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("space", pw_info.spaceid, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT32("label", pw_info.label, argv[idx + 2]);
    }
    else
    {
        pw_info.spaceid = 0;
        CTC_CLI_GET_UINT32("label", pw_info.label, argv[0]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != idx)
    {
        CTC_SET_FLAG(pw_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mpls_del_ilm(g_api_lchip, &pw_info);
    }
    else
    {
        ret = ctc_mpls_del_ilm(&pw_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_remove_ilm_tcam,
        ctc_cli_mpls_remove_ilm_tcam_cmd,
        "mpls ilm remove space SPACEID label LABEL ( nexthop-id NHID |) (use-flex |)",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Remove ilm(inluding l3vpn-vc, vpls-vc and vpws-vc)",
        "Mpls label space",
        "Label space id, <0-255>, space 0 is platform space",
        "Mpls label",
        "Label, <0-1048575>",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Is tcam")
{
    int32 ret = CLI_SUCCESS;
    uint32 spaceid = 0;
    ctc_mpls_ilm_t ilm_info;
    uint8 index = 0;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));

    CTC_CLI_GET_UINT8_RANGE("space", spaceid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    ilm_info.spaceid = spaceid;

    CTC_CLI_GET_UINT32_RANGE("label", ilm_info.label, argv[1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("nexthop-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nexthop-id", ilm_info.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mpls_del_ilm(g_api_lchip, &ilm_info);
    }
    else
    {
        ret = ctc_mpls_del_ilm(&ilm_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_ac_add_port,
        ctc_cli_mpls_ac_add_port_cmd,
        "mpls l2vpn-ac add PORT (vpws NHID | vpls FID)",
        CTC_CLI_MPLS_M_STR,
        "L2vpn attachment circuit entry",
        "Add l2vpn ac",
        CTC_CLI_GPORT_ID_DESC,
        "Virtual private wire services ",
        CTC_CLI_NH_ID_STR,
        "Virtual private lan services",
        CTC_CLI_FID_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    ctc_vlan_mapping_t vlan_mapping;
    uint32 fid;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp("vpws", argv[1], 4))
    {
        CTC_CLI_GET_UINT32("nexthop", vlan_mapping.u3.nh_id, argv[2]);
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPWS;
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_NHID;
    }
    else
    {
        CTC_CLI_GET_UINT16_RANGE("fid", fid, argv[2], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.u3.fid = fid;
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS;
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_FID;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_ac_add_vlan,
        ctc_cli_mpls_ac_add_vlan_cmd,
        "mpls l2vpn-ac add PORT (svlan | cvlan) VLANID (vpws NHID| vpls FID)",
        CTC_CLI_MPLS_M_STR,
        "L2vpn attachment circuit entry",
        "Add l2vpn ac",
        CTC_CLI_GPORT_ID_DESC,
        "Port binding to l2vpn with svlan",
        "Port binding to l2vpn with cvlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Virtual private wire services ",
        CTC_CLI_NH_ID_STR,
        "Virtual private lan services",
        CTC_CLI_FID_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    uint32 vlan = 0;
    ctc_vlan_mapping_t vlan_mapping;
    uint32 fid;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("vlan", vlan, argv[2], 0, CTC_MAX_UINT16_VALUE);

    if (vlan > CTC_MAX_VLAN_ID)
    {
        ctc_cli_out("%% invalid vlan id \n\r");
        return CLI_ERROR;
    }

    if (0 == sal_memcmp("sv", argv[1], 2))
    {
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
        vlan_mapping.old_svid = vlan;
    }
    else
    {
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        vlan_mapping.old_cvid = vlan;
    }

    if (0 == sal_memcmp("vpws", argv[3], 4))
    {
        CTC_CLI_GET_UINT32("nexthop", vlan_mapping.u3.nh_id, argv[4]);
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPWS;
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_NHID;
    }
    else
    {
        CTC_CLI_GET_UINT16_RANGE("fid", fid, argv[4], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.u3.fid = fid;
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS;
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_FID;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_ac_remove_port,
        ctc_cli_mpls_ac_remove_port_cmd,
        "mpls l2vpn-ac remove PORT",
        CTC_CLI_MPLS_M_STR,
        "L2vpn attachment circuit entry",
        "Remove l2vpn ac",
        CTC_CLI_GPORT_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    ctc_vlan_mapping_t vlan_mapping;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_ac_remove_vlan,
        ctc_cli_mpls_ac_remove_vlan_cmd,
        "mpls l2vpn-ac remove PORT (svlan | cvlan) VLANID",
        CTC_CLI_MPLS_M_STR,
        "L2vpn attachment circuit entry",
        "Remove l2vpn ac",
        CTC_CLI_GPORT_ID_DESC,
        "Port binding to l2vpn with svlan",
        "Port binding to l2vpn with cvlan",
        "0~4094")
{
    int32 ret = CLI_SUCCESS;
    uint32 vlan = 0;
    uint32 gport = 0;
    ctc_vlan_mapping_t vlan_mapping;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("vlan", vlan, argv[2], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp("sv", argv[1], 2))
    {
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
        vlan_mapping.old_svid = vlan;
    }
    else
    {
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
        vlan_mapping.old_cvid = vlan;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_debug_on,
        ctc_cli_mpls_debug_on_cmd,
        "debug mpls (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MPLS_M_STR,
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
        typeenum = MPLS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MPLS_SYS;
    }

    ctc_debug_set_flag("mpls", "mpls", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_debug_off,
        ctc_cli_mpls_debug_off_cmd,
        "no debug mpls (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MPLS_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MPLS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MPLS_SYS;
    }

    ctc_debug_set_flag("mpls", "mpls", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_debug_show,
        ctc_cli_mpls_debug_show_cmd,
        "show debug mpls (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MPLS_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MPLS_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MPLS_SYS;
    }

    en = ctc_debug_get_flag("mpls", "mpls", typeenum, &level);
    ctc_cli_out("mpls:%s debug %s level %s\n\r", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_add_stats,
        ctc_cli_mpls_add_stats_cmd,
        "mpls add stats STATS_ID SPACEID LABEL",
        CTC_CLI_MPLS_M_STR,
        "Add action",
        "Statistics",
        "0~0xFFFFFFFF",
        "Label space id, <0-255>, space 0 is platform space",
        "Label, <0-1048575>")
{
    int32 ret = CLI_SUCCESS;
    ctc_mpls_stats_index_t stats_index;

    sal_memset(&stats_index, 0, sizeof(stats_index));
    CTC_CLI_GET_UINT32_RANGE("stats id", stats_index.stats_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT8_RANGE("space", stats_index.spaceid, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("label", stats_index.label, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if (g_ctcs_api_en)
    {
        ret = ctcs_mpls_add_stats(g_api_lchip, &stats_index);
    }
    else
    {
        ret = ctc_mpls_add_stats(&stats_index);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_remove_stats,
        ctc_cli_mpls_remove_stats_cmd,
        "mpls remove stats SPACEID LABEL",
        CTC_CLI_MPLS_M_STR,
        "Remove action",
        "Statistics",
        "Label space id, <0-255>, space 0 is platform space",
        "Label, <0-1048575>")
{
    int32 ret = CLI_SUCCESS;
    ctc_mpls_stats_index_t stats_index;

    sal_memset(&stats_index, 0, sizeof(stats_index));
    CTC_CLI_GET_UINT8_RANGE("space", stats_index.spaceid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("label", stats_index.label, argv[1], 0, CTC_MAX_UINT32_VALUE);
    stats_index.stats_id = 0;

    if (g_ctcs_api_en)
    {
        ret = ctcs_mpls_remove_stats(g_api_lchip, &stats_index);
    }
    else
    {
        ret = ctc_mpls_remove_stats(&stats_index);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mpls_get_ilm,
        ctc_cli_mpls_get_ilm_cmd,
        "show mpls ilm SPACEID label LABEL detail (use-flex |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MPLS_M_STR,
        "Ilm entries",
        "SPACEID",
        "mpls label",
        "LABEL",
        "Key and associate-data offset info",
        "is tcam")
{
    int32 ret = 0;
    uint32 nh_id[64];
    ctc_mpls_ilm_t mpls_ilm;
    uint8 index = 0;
    char* arr_type[] = {
        "Normal",
        "L3VPN",
        "VPWS",
        "VPLS",
        "GATEWAY"
    };
    char* arr_model[] = {
            "uniform",
            "short pipe",
            "pipe"
    };
    char* arr_encap[] = {
            "Tagged",
            "Raw"
    };
    char* arr_inner_packet_type[] = {
            "IP",
            "IPv4",
            "IPv6",
            "RAW"
    };
    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));

    CTC_CLI_GET_UINT8_RANGE("space-id", mpls_ilm.spaceid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("label", mpls_ilm.label, argv[1], 0, CTC_MAX_UINT32_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        CTC_SET_FLAG(mpls_ilm.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_mpls_get_ilm(g_api_lchip, nh_id, &mpls_ilm);
    }
    else
    {
        ret = ctc_mpls_get_ilm(nh_id, &mpls_ilm);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("-----------------------Detail Information-------------------\r\n");
    ctc_cli_out("%-30s:%-6u \n", "Label", mpls_ilm.label);
    ctc_cli_out("%-30s:%-6u \n", "Space Id", mpls_ilm.spaceid);
    ctc_cli_out("%-30s:%-6u \n", "Nexthop Id", mpls_ilm.nh_id);
    ctc_cli_out("%-30s:%-6u \n", "Stats Id", mpls_ilm.stats_id);
    ctc_cli_out("%-30s:%-6u \n", "Gport", mpls_ilm.gport);
    ctc_cli_out("%-30s:%-6u \n", "Pwid", mpls_ilm.pwid);
    ctc_cli_out("%-30s:%-6s \n", "Type", arr_type[mpls_ilm.type]);
    ctc_cli_out("%-30s:%-6s \n", "Model", arr_model[mpls_ilm.model]);
    if (!mpls_ilm.cwen)
    {
        ctc_cli_out("%-30s:%-6s \n", "Control Word", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "Control Word", "Enable");
    }
    if (!mpls_ilm.pop)
    {
        ctc_cli_out("%-30s:%-6s \n", "POP", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "POP", "Enable");
    }
    if (!mpls_ilm.decap)
    {
        ctc_cli_out("%-30s:%-6s \n", "Decap", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "Decap", "Enable");
    }
    if (!mpls_ilm.aps_select_protect_path)
    {
        ctc_cli_out("%-30s:%-6s \n", "Aps Select Protect Path", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "Aps Select Protect Path", "Enable");
    }
    if (!mpls_ilm.logic_port_type)
    {
        ctc_cli_out("%-30s:%-6s \n", "H-VPLS", "Enable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "H-VPLS", "Disable");
    }
    if (!mpls_ilm.oam_en)
    {
        ctc_cli_out("%-30s:%-6s \n", "OAM", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "OAM", "Enable");
    }
    if (!mpls_ilm.trust_outer_exp)
    {
        ctc_cli_out("%-30s:%-6s \n", "Trust Outer EXP", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "Trust Outer EXP", "Enable");
    }

    ctc_cli_out("%-30s:%-6s \n", "Inner Packet Type", arr_inner_packet_type[mpls_ilm.inner_pkt_type]);

    if (!mpls_ilm.qos_use_outer_info)
    {
        ctc_cli_out("%-30s:%-6s \n", "QOS Use Outer Info", "Disable");
    }
    else
    {
        ctc_cli_out("%-30s:%-6s \n", "QOS Use Outer Info", "Enable");
    }

    ctc_cli_out("%-30s:%-6u \n", "Interface Spaceid", mpls_ilm.out_intf_spaceid);

    ctc_cli_out("%-30s:%-6u \n", "Aps Group Id", mpls_ilm.aps_select_grp_id);

    ctc_cli_out("%-30s:%-6u \n", "Svlan Tpid Index", mpls_ilm.svlan_tpid_index);

    ctc_cli_out("%-30s:%-6s \n", "PW Mode", arr_encap[mpls_ilm.pw_mode]);

    ctc_cli_out("%-30s:%-6u \n", "FID", mpls_ilm.fid);

    ctc_cli_out("%-30s:%-6u \n", "Vrf Id", mpls_ilm.vrf_id);

    ctc_cli_out("%-30s:%-6u \n", "L2vpn Oam Id", mpls_ilm.l2vpn_oam_id);

    ctc_cli_out("%-30s:%-6u \n", "Policer Id", mpls_ilm.policer_id);

    return CLI_SUCCESS;
}

int32
ctc_mpls_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_add_normal_ilm_tcam_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_add_l3vpn_ilm_tcam_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_remove_ilm_tcam_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_add_gateway_pw_tcam_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_remove_gateway_pw_tcam_cmd);

/* mpls l2vpn-ac cmd use vlan mapping */
/*
   install_element(CTC_SDK_MODE, &ctc_cli_mpls_ac_add_port_cmd);
   install_element(CTC_SDK_MODE, &ctc_cli_mpls_ac_add_vlan_cmd);
   install_element(CTC_SDK_MODE, &ctc_cli_mpls_ac_remove_port_cmd);
   install_element(CTC_SDK_MODE, &ctc_cli_mpls_ac_remove_vlan_cmd);*/

    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mpls_debug_off_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_debug_show_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_get_ilm_cmd);


    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_set_ilm_property_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_add_stats_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_mpls_remove_stats_cmd);
    return CLI_SUCCESS;
}

