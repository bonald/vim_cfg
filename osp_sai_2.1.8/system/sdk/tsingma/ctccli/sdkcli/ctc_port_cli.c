/**
 @file ctc_port_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-14

 @version v2.0

 The file apply clis of port module
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_port_cli.h"

CTC_CLI(ctc_cli_port_set_default_cfg,
        ctc_cli_port_set_default_cfg_cmd,
        "port GPHYPORT_ID default-config",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Set port configure to default")
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_default_cfg(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_port_set_default_cfg(gport);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_port_en,
        ctc_cli_port_set_port_en_cmd,
        "port GPHYPORT_ID port-en (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Set port state",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_port_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_port_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_port_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_port_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_port_set_receive,
        ctc_cli_port_set_receive_cmd,
        "port GPHYPORT_ID receive (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Reception port state",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_receive_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_receive_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_receive_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_receive_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_transmit,
        ctc_cli_port_set_transmit_cmd,
        "port GPHYPORT_ID transmit (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Transmission port state",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_transmit_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_transmit_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_transmit_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_transmit_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_port_set_bridge,
        ctc_cli_port_set_bridge_cmd,
        "port GPHYPORT_ID bridge (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "L2 bridge",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_bridge_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_bridge_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("disable", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_bridge_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_bridge_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_phy_if,
        ctc_cli_port_set_phy_if_cmd,
        "port GPHYPORT_ID phy-if (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Physical interface",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    bool enable  = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_phy_if_en(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_phy_if_en(gport, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_sub_if,
        ctc_cli_port_set_sub_if_cmd,
        "port GPHYPORT_ID sub-if (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Sub interface",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    bool enable  = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_sub_if_en(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_sub_if_en(gport, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_vlan_filtering,
        ctc_cli_port_set_vlan_filtering_cmd,
        "port GPHYPORT_ID vlan-filtering direction (ingress | egress | both)(enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Vlan filtering",
        "Filtering direction",
        "Ingress filtering",
        "Egress filtering",
        "Both igs and egs filtering",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    ctc_direction_t dir = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("in", argv[1], 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp("eg", argv[1], 2))
    {
        dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp("bo", argv[1], 2))
    {
        dir = CTC_BOTH_DIRECTION;
    }


    if (0 == sal_memcmp("en", argv[2], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_vlan_filter_en(g_api_lchip, gport, dir, TRUE);
        }
        else
        {
            ret = ctc_port_set_vlan_filter_en(gport, dir, TRUE);
        }
    }
    else if (0 == sal_memcmp("disable", argv[2], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_vlan_filter_en(g_api_lchip, gport, dir, FALSE);
        }
        else
        {
            ret = ctc_port_set_vlan_filter_en(gport, dir, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_default_vlan,
        ctc_cli_port_set_default_vlan_cmd,
        "port GPHYPORT_ID default vlan VLAN_ID",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Default vlan tag",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[1]);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_default_vlan(g_api_lchip, gport, vlan_id);
    }
    else
    {
        ret = ctc_port_set_default_vlan(gport, vlan_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_vlan_ctl,
        ctc_cli_port_set_vlan_ctl_cmd,
        "port GPHYPORT_ID vlan-ctl (allow-all | drop-all-untagged | drop-all-tagged | drop-all | drop-wo-2tag \
    | drop-w-2tag | drop-stag | drop-non-stag | drop-only-stag | permit-only-stag |drop-all-ctag |drop-non-ctag \
    | drop-only-ctag | permit-only-ctag)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Vlan control",
        "Allow all packet",
        "Drop all untagged packet",
        "Drop all tagged packet",
        "Drop all packet",
        "Drop packet without 2 tag",
        "Drop packet with 2 tag",
        "Drop stagged packet",
        "Drop packet without stag",
        "Drop packet only stagged",
        "Permit packet only with stag",
        "Drop all packet with ctag",
        "Drop packet without ctag",
        "Drop packet only ctagged",
        "Permit packet only with ctag")
{
    int32 ret = 0;
    uint16 gport = 0;
    ctc_vlantag_ctl_t mode = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_strcmp("allow-all", argv[1]))
    {
        mode = CTC_VLANCTL_ALLOW_ALL_PACKETS;
    }
    else if (0 == sal_strcmp("drop-all-untagged", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_UNTAGGED;
    }
    else if (0 == sal_strcmp("drop-all-tagged", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_TAGGED;
    }
    else if (0 == sal_strcmp("drop-all", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL;
    }
    else if (0 == sal_strcmp("drop-wo-2tag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_PACKET_WITHOUT_TWO_TAG;
    }
    else if (0 == sal_strcmp("drop-w-2tag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_PACKET_WITH_TWO_TAG;
    }
    else if (0 == sal_strcmp("drop-stag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_SVLAN_TAG;
    }
    else if (0 == sal_strcmp("drop-non-stag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_NON_SVLAN_TAG;
    }
    else if (0 == sal_strcmp("drop-only-stag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ONLY_SVLAN_TAG;
    }
    else if (0 == sal_strcmp("permit-only-stag", argv[1]))
    {
        mode = CTC_VLANCTL_PERMIT_ONLY_SVLAN_TAG;
    }
    else if (0 == sal_strcmp("drop-all-ctag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_CVLAN_TAG;
    }
    else if (0 == sal_strcmp("drop-non-ctag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ALL_NON_CVLAN_TAG;
    }
    else if (0 == sal_strcmp("drop-only-ctag", argv[1]))
    {
        mode = CTC_VLANCTL_DROP_ONLY_CVLAN_TAG;
    }
    else if (0 == sal_strcmp("permit-only-ctag", argv[1]))
    {
        mode = CTC_VLANCTL_PERMIT_ONLY_CVLAN_TAG;
    }
    else
    {
        mode = CTC_VLANCTL_ALLOW_ALL_PACKETS;
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_port_set_vlan_ctl(g_api_lchip, gport, mode);
    }
    else
    {
        ret = ctc_port_set_vlan_ctl(gport, mode);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_cross_connect,
        ctc_cli_port_set_cross_connect_cmd,
        "port GPHYPORT_ID port-cross-connect nhid NHID",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port cross connect",
        CTC_CLI_NH_ID_STR,
        "0xFFFFFFFF means disable port cross-connect")
{
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    CTC_CLI_GET_UINT32("nhid", nhid, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_cross_connect(g_api_lchip, gport, nhid);
    }
    else
    {
        ret = ctc_port_set_cross_connect(gport, nhid);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_learning,
        ctc_cli_port_set_learning_cmd,
        "port GPHYPORT_ID learning (enable |disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Mac learning",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    bool learning_en = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        learning_en = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_learning_en(g_api_lchip, gport, learning_en);
    }
    else
    {
        ret = ctc_port_set_learning_en(gport, learning_en);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_vlan_domain,
        ctc_cli_port_set_vlan_domain_cmd,
        "port GPHYPORT_ID vlan-domain (cvlan | svlan)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Set vlan domain when the cvlan TPID is the same as svlan TPID",
        "Cvlan domain",
        "Svlan domain")
{
    int32 ret = 0;
    uint16 gport = 0;
    ctc_port_vlan_domain_type_t vlan_domain = CTC_PORT_VLAN_DOMAIN_SVLAN;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("cvlan", argv[1], 5))
    {
        vlan_domain = CTC_PORT_VLAN_DOMAIN_CVLAN;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_vlan_domain(g_api_lchip, gport, vlan_domain);
    }
    else
    {
        ret = ctc_port_set_vlan_domain(gport, vlan_domain);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_stag_tpid_index,
        ctc_cli_port_set_stag_tpid_index_cmd,
        "port GPHYPORT_ID stag-tpid-index direction (ingress|egress|both) INDEX",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Stag TPID index point to stag TPID value",
        "Direction",
        "Ingress",
        "Egress",
        "Both direction",
        "<0-3>")
{
    int32 ret = 0;
    uint16 gport = 0;
    uint8 index = 0;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    CTC_CLI_GET_UINT8("stpid index", index, argv[2]);

    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_stag_tpid_index(g_api_lchip, gport, dir, index);
    }
    else
    {
        ret = ctc_port_set_stag_tpid_index(gport, dir, index);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_dot1q_type,
        ctc_cli_port_set_dot1q_type_cmd,
        "port GPHYPORT_ID dot1q-type (untagged | cvlan-tagged | svlan-tagged | double-tagged)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port's type descript in 802.1q",
        "Packet transmit with untag",
        "Packet transmit with ctag",
        "Packet transmit with stag",
        "Packet transmit with both stag and ctag")
{
    int32 ret = 0;
    uint16 gport = 0;
    ctc_dot1q_type_t type = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("un", argv[1], 2))
    {
        type = CTC_DOT1Q_TYPE_NONE;
    }
    else if (0 == sal_memcmp("cvlan", argv[1], 2))
    {
        type = CTC_DOT1Q_TYPE_CVLAN;
    }
    else if (0 == sal_memcmp("svlan", argv[1], 2))
    {
        type = CTC_DOT1Q_TYPE_SVLAN;
    }
    else if (0 == sal_memcmp("double", argv[1], 2))
    {
        type = CTC_DOT1Q_TYPE_BOTH;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_dot1q_type(g_api_lchip, gport, type);
    }
    else
    {
        ret = ctc_port_set_dot1q_type(gport, type);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_untag_dft_vid_enable,
        ctc_cli_port_set_untag_dft_vid_enable_cmd,
        "port GPHYPORT_ID untag-default-vlan enable (untag-svlan | untag-cvlan)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Untag vlan id when transmit packet vid equal to PVID",
        CTC_CLI_ENABLE,
        "Untag vlan id on stag",
        "Untag vlan id on ctag")
{
    int32 ret;
    uint16 gport;
    bool is_untag_svlan = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("untag-svlan", argv[1], 7))
    {
        is_untag_svlan = TRUE;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_untag_dft_vid(g_api_lchip, gport, TRUE, is_untag_svlan);
    }
    else
    {
        ret = ctc_port_set_untag_dft_vid(gport, TRUE, is_untag_svlan);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_untag_dft_vid_disable,
        ctc_cli_port_set_untag_dft_vid_disable_cmd,
        "port GPHYPORT_ID untag-default-vlan disable",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Untag vlan id when transmit packet vid equal to PVID",
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 gport;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_untag_dft_vid(g_api_lchip, gport, FALSE, FALSE);
    }
    else
    {
        ret = ctc_port_set_untag_dft_vid(gport, FALSE, FALSE);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_protocol_vlan,
        ctc_cli_port_set_protocol_vlan_cmd,
        "port GPHYPORT_ID protocol-vlan (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Protocol vlan on port",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 gport;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        enable = TRUE;
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_port_set_protocol_vlan_en(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_protocol_vlan_en(gport, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_blocking,
        ctc_cli_port_set_blocking_cmd,
        "port GPHYPORT_ID port-blocking {ucast-flooding | mcast-flooding | known-ucast-forwarding | known-mcast-forwarding | bcast-flooding} \
        (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port based blocking",
        "Unknown ucast flooding to this port",
        "Unknown mcast flooding to this port",
        "Known ucast forwarding to this port",
        "Known mcast forwarding to this port",
        "Broadcast flooding to this port",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    uint8 index = 0;
    ctc_port_restriction_t port_restriction;

    sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));

    port_restriction.mode = CTC_PORT_RESTRICTION_PORT_BLOCKING;
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        if(g_ctcs_api_en)
        {
            ctcs_port_get_restriction(g_api_lchip, gport, &port_restriction);
        }
        else
        {
            ctc_port_get_restriction(gport, &port_restriction);
        }

        port_restriction.type = port_restriction.type;

        index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
        if (index != 0xFF)
        {
            CTC_UNSET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_UNKNOW_UCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
        if (index != 0xFF)
        {
            CTC_UNSET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_UNKNOW_MCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
        if (index != 0xFF)
        {
            CTC_UNSET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_KNOW_UCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
        if (index != 0xFF)
        {
            CTC_UNSET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_KNOW_MCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
        if (index != 0xFF)
        {
            CTC_UNSET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_BCAST);
        }
    }
    else
    {
        port_restriction.type = 0;

        index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_UNKNOW_UCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_UNKNOW_MCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_KNOW_UCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_KNOW_MCAST);
        }

        index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
        if (index != 0xFF)
        {
            CTC_SET_FLAG(port_restriction.type, CTC_PORT_BLOCKING_BCAST);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_restriction(g_api_lchip, gport, &port_restriction);
    }
    else
    {
        ret = ctc_port_set_restriction(gport, &port_restriction);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_use_outer_ttl,
        ctc_cli_port_set_use_outer_ttl_cmd,
        "port GPHYPORT_ID use-outer-ttl (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "The decap packet will use outer TTL",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = TRUE;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_use_outer_ttl(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_use_outer_ttl(gport, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_mac_en,
        ctc_cli_port_set_mac_en_cmd,
        "port GPHYPORT_ID mac (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port mac",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport;
    uint16 lport = 0;
    bool enable;

    lport = lport;
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if (g_ctcs_api_en)
    {
        ret = ret ? ret : ctcs_port_set_mac_en(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ret ? ret : ctc_port_set_mac_en(gport, enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_3ap_auto_neg_ability,
        ctc_cli_port_set_3ap_auto_neg_ability_cmd,
        "port GPHYPORT_ID cl73-auto-neg ability {10GBASE-KR| 40GBASE-KR4| 40GBASE-CR4|\
        100GBASE-KR4| 100GBASE-CR4|25GBASE-KR-S| 25GBASE-CR-S| 25GBASE-KR| 25GBASE-CR|25G-RS-FEC-REQ|\
        25G-BASER-FEC-REQ|FEC-ABILITY |FEC-REQ |25GBASE-KR1| 25GBASE-CR1| 50GBASE-KR2| 50GBASE-CR2 |\
        CL91-ABILITY| CL74-ABILITY| CL91-REQ| CL74-REQ| none}",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "802.3 cl73 auto neg",
        "transmission medium, speed and other ability",
        "10GBASE-KR,       CL73 base page ability",
        "40GBASE-KR4,      CL73 base page ability",
        "40GBASE-CR4,      CL73 base page ability",
        "100GBASE-KR4,     CL73 base page ability",
        "100GBASE-CR4,     CL73 base page ability",
        "25GBASE-KR-S,     CL73 base page ability",
        "25GBASE-CR-S,     CL73 base page ability",
        "25GBASE-KR,       CL73 base page ability",
        "25GBASE-CR,       CL73 base page ability",
        "25G RS-FEC requested,           CL73 base page ability",
        "25G BASE-R FEC requested,       CL73 base page ability",
        "CL74 BASE-R FEC supported,      CL73 base page ability",
        "CL74 BASE-R FEC requested,      CL73 base page ability",
        "25GBASE-KR1,                    CL73 extended next page ability",
        "25GBASE-CR1,                    CL73 extended next page ability",
        "50GBASE-KR2,                    CL73 extended next page ability",
        "50GBASE-CR2,                    CL73 extended next page ability",
        "CL91 RS-FEC ability support,    CL73 extended next page ability",
        "CL74 BASE-R FEC ability support,CL73 extended next page ability",
        "CL91 RS-FEC requested,          CL73 extended next page ability",
        "CL74 BASE-R FEC requested,      CL73 extended next page ability",
        "disable cl73 auto neg")
{
    int32 ret = 0;
    uint16 gport;
    uint32 ability = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("10GBASE-KR");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_10GBASE_KR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("40GBASE-KR4");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_40GBASE_KR4;
    }
    index = CTC_CLI_GET_ARGC_INDEX("40GBASE-CR4");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_40GBASE_CR4;
    }
    index = CTC_CLI_GET_ARGC_INDEX("100GBASE-KR4");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_100GBASE_KR4;
    }
    index = CTC_CLI_GET_ARGC_INDEX("100GBASE-CR4");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_100GBASE_CR4;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-KR-S");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25GBASE_KRS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-CR-S");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25GBASE_CRS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-KR");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25GBASE_KR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-CR");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25GBASE_CR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25G-RS-FEC-REQ");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25G_RS_FEC_REQUESTED;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25G-BASER-FEC-REQ");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_25G_BASER_FEC_REQUESTED;
    }
    index = CTC_CLI_GET_ARGC_INDEX("FEC-ABILITY");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_FEC_ABILITY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("FEC-REQ");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CL73_FEC_REQUESTED;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-KR1");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_25GBASE_KR1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("25GBASE-CR1");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_25GBASE_CR1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("50GBASE-KR2");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_50GBASE_KR2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("50GBASE-CR2");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_50GBASE_CR2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("CL91-ABILITY");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_RS_FEC_ABILITY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("CL74-ABILITY");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_BASER_FEC_ABILITY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("CL91-REQ");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_RS_FEC_REQUESTED;
    }
    index = CTC_CLI_GET_ARGC_INDEX("CL74-REQ");
    if (index != 0xFF)
    {
        ability |= CTC_PORT_CSTM_BASER_FEC_REQUESTED;
    }
    /* disable cl73 */
    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (index != 0xFF)
    {
        ability = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_CL73_ABILITY, ability);
    }
    else
    {
        ret = ctc_port_set_property(gport, CTC_PORT_PROP_CL73_ABILITY, ability);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_set_cpumac_en,
        ctc_cli_port_set_cpumac_en_cmd,
        "port cpumac (enable | disable)",
        CTC_CLI_PORT_M_STR,
        "Cpu mac",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    bool enable;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_cpu_mac_en(g_api_lchip, enable);
    }
    else
    {
        ret = ctc_port_set_cpu_mac_en(enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_speed,
        ctc_cli_port_set_speed_cmd,
        "port GPHYPORT_ID speed-mode (eth|fe|ge|2g5|5g|xg)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port speed mode",
        "Ethernet port with 10M",
        "Fast Eth port with 100M",
        "GEth port with 1G",
        "GEth port with 2.5G",
        "GEth port with 5G",
        "GEth port with 10G")
{
    int32 ret = CLI_SUCCESS;
    uint32 gport;
    ctc_port_speed_t speed_mode = 0;

    CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if (CLI_CLI_STR_EQUAL("eth", 1))
    {
        speed_mode = CTC_PORT_SPEED_10M;
    }
    else if (CLI_CLI_STR_EQUAL("fe", 1))
    {
        speed_mode = CTC_PORT_SPEED_100M;
    }
    else if (CLI_CLI_STR_EQUAL("2g5", 1))
    {
        speed_mode = CTC_PORT_SPEED_2G5;
    }
    else if (CLI_CLI_STR_EQUAL("ge", 1))
    {
        speed_mode = CTC_PORT_SPEED_1G;
    }
    else if (CLI_CLI_STR_EQUAL("5g", 1))
    {
        speed_mode = CTC_PORT_SPEED_5G;
    }
    else if (CLI_CLI_STR_EQUAL("xg", 1))
    {
        speed_mode = CTC_PORT_SPEED_10G;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_speed(g_api_lchip, gport, speed_mode);
    }
    else
    {
        ret = ctc_port_set_speed(gport, speed_mode);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_interface_mode,
        ctc_cli_port_set_interface_mode_cmd,
        "port GPHYPORT_ID if-mode (100M|1G|2500|5G|10G|20G|25G|40G|50G|100G)\
            (100BASE-FX|SGMII|2500X|QSGMII|USXGMII-S|USXGMII-M2G5|USXGMII-M5G|XAUI|DXAUI|XFI|KR|CR|KR2|CR2|KR4|CR4)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port interface mode",
        "100M speed",
        "1G speed",
        "2.5G speed",
        "5G speed",
        "10G speed",
        "20G speed",
        "25G speed",
        "40G speed",
        "50G speed",
        "100G speed",
        "100BASE FX type",
        "SGMII type",
        "2500X type",
        "QSGMII type",
        "USXGMII Single type",
        "USXGMII Multi 2.5G type",
        "USXGMII Multi 5G type",
        "XAUI type",
        "DXAUI type",
        "XFI type",
        "KR type",
        "CR type",
        "KR2 type",
        "CR2 type",
        "KR4 type",
        "CR4 type")
{
    int32 ret = CLI_SUCCESS;
    uint32 gport;
    ctc_port_speed_t speed = 0;
    ctc_port_if_type_t interface_type = 0;
    ctc_port_if_mode_t if_mode;

    CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if (CLI_CLI_STR_EQUAL("100M", 1))
    {
        speed = CTC_PORT_SPEED_100M;
    }
    else if (CLI_CLI_STR_EQUAL("1G", 1))
    {
        speed = CTC_PORT_SPEED_1G;
    }
    else if (CLI_CLI_STR_EQUAL("2500", 1))
    {
        speed = CTC_PORT_SPEED_2G5;
    }
    else if (CLI_CLI_STR_EQUAL("5G", 1))
    {
        speed = CTC_PORT_SPEED_5G;
    }
    else if (CLI_CLI_STR_EQUAL("10G", 1))
    {
        speed = CTC_PORT_SPEED_10G;
    }
    else if (CLI_CLI_STR_EQUAL("20G", 1))
    {
        speed = CTC_PORT_SPEED_20G;
    }
    else if (CLI_CLI_STR_EQUAL("25G", 1))
    {
        speed = CTC_PORT_SPEED_25G;
    }
    else if (CLI_CLI_STR_EQUAL("40G", 1))
    {
        speed = CTC_PORT_SPEED_40G;
    }
    else if (CLI_CLI_STR_EQUAL("50G", 1))
    {
        speed = CTC_PORT_SPEED_50G;
    }
    else if (CLI_CLI_STR_EQUAL("100G", 1))
    {
        speed = CTC_PORT_SPEED_100G;
    }
    else
    {
        ctc_cli_out("%% Not supported speed mode \n");
        return CTC_E_INVALID_PARAM;
    }

    if (CLI_CLI_STR_EQUAL("100BASE-FX", 2))
    {
        interface_type = CTC_PORT_IF_FX;
    }
    else if (CLI_CLI_STR_EQUAL("SGMII", 2))
    {
        interface_type = CTC_PORT_IF_SGMII;
    }
    else if (CLI_CLI_STR_EQUAL("2500X", 2))
    {
        interface_type = CTC_PORT_IF_2500X;
    }
    else if (CLI_CLI_STR_EQUAL("QSGMII", 2))
    {
        interface_type = CTC_PORT_IF_QSGMII;
    }
    else if (CLI_CLI_STR_EQUAL("USXGMII-S", 2))
    {
        interface_type = CTC_PORT_IF_USXGMII_S;
    }
    else if (CLI_CLI_STR_EQUAL("USXGMII-M2G5", 2))
    {
        interface_type = CTC_PORT_IF_USXGMII_M2G5;
    }
    else if (CLI_CLI_STR_EQUAL("USXGMII-M5G", 2))
    {
        interface_type = CTC_PORT_IF_USXGMII_M5G;
    }
    else if (CLI_CLI_STR_EQUAL("XAUI", 2))
    {
        interface_type = CTC_PORT_IF_XAUI;
    }
    else if (CLI_CLI_STR_EQUAL("DXAUI", 2))
    {
        interface_type = CTC_PORT_IF_DXAUI;
    }
    else if (CLI_CLI_STR_EQUAL("XFI", 2))
    {
        interface_type = CTC_PORT_IF_XFI;
    }
    else if (CLI_CLI_STR_EQUAL("KR2", 2))
    {
        interface_type = CTC_PORT_IF_KR2;
    }
    else if (CLI_CLI_STR_EQUAL("CR2", 2))
    {
        interface_type = CTC_PORT_IF_CR2;
    }
    else if (CLI_CLI_STR_EQUAL("KR4", 2))
    {
        interface_type = CTC_PORT_IF_KR4;
    }
    else if (CLI_CLI_STR_EQUAL("CR4", 2))
    {
        interface_type = CTC_PORT_IF_CR4;
    }
    else if (CLI_CLI_STR_EQUAL("KR", 2))
    {
        interface_type = CTC_PORT_IF_KR;
    }
    else if (CLI_CLI_STR_EQUAL("CR", 2))
    {
        interface_type = CTC_PORT_IF_CR;
    }

    else
    {
        ctc_cli_out("%% Not supported interface type \n");
        return CTC_E_INVALID_PARAM;
    }

    if_mode.speed = speed;
    if_mode.interface_type = interface_type;

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_interface_mode(g_api_lchip, gport, &if_mode);
    }
    else
    {
        ret = ctc_port_set_interface_mode(gport, &if_mode);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_port_set_flow_ctl,
        ctc_cli_port_set_flow_ctl_cmd,
        "port GPHYPORT_ID flow-ctl (priority-class PRI (pfc-class CLASS|) |) (ingress|egress|both)(enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Flow control",
        "Cos used for PFC",
        "<0-7>",
        "Pfc frame class",
        "<0-7>",
        "Receive pause frame",
        "Transmit pause frame",
        "Both receive and transmit",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index;
    uint16 gport = 0;
    uint8 priority_class = 0;
    uint8 pfc_class = 0;
    ctc_direction_t dir = 0;
    uint8 enable = 0;
    uint8 is_pfc = 0;
    ctc_port_fc_prop_t fc;

    sal_memset(&fc, 0, sizeof(fc));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    index = CTC_CLI_GET_ARGC_INDEX("priority-class");
    if (0xFF != index)
    {
        is_pfc = 1;
        CTC_CLI_GET_UINT8("priority class", priority_class, argv[index + 1]);

        index = CTC_CLI_GET_ARGC_INDEX("pfc-class");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("pfc class", pfc_class, argv[index + 1]);
        }
        else
        {
           pfc_class =  priority_class;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (0xFF != index)
    {
        dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
       enable = 1;
    }


    fc.gport  = gport;
    fc.priority_class    = priority_class;
    fc.enable = enable;
    fc.dir    = dir;
    fc.is_pfc = is_pfc;
    fc.pfc_class = pfc_class;
    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_flow_ctl_en(g_api_lchip, &fc);
    }
    else
    {
        ret = ctc_port_set_flow_ctl_en(&fc);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_show_flow_ctl,
        ctc_cli_port_show_flow_ctl_cmd,
        "show port GPHYPORT_ID flow-ctl (priority-class PRI (pfc-class CLASS|) |) (ingress|egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Flow control",
        "Cos used for PFC",
        "<0-7>",
        "Pfc frame class",
        "<0-7>",
        "Receive pause frame",
        "Transmit pause frame")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_port_fc_prop_t fc;
    char* desc[2] = {"Ingress", "Egress"};
    uint8 index;
    uint8 priority_class = 0;
    uint8 pfc_class = 0;
    ctc_direction_t dir = 0;
    uint8 is_pfc = 0;

    sal_memset(&fc, 0, sizeof(fc));
    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("priority-class");
    if (0xFF != index)
    {
        is_pfc = 1;
        CTC_CLI_GET_UINT8("priority class", priority_class, argv[index + 1]);

        index = CTC_CLI_GET_ARGC_INDEX("pfc-class");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("pfc class", pfc_class, argv[index + 1]);
        }
        else
        {
           pfc_class =  priority_class;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        dir = CTC_EGRESS;
    }

    fc.gport  = gport;
    fc.dir    = dir;
    fc.is_pfc = is_pfc;
    fc.priority_class    = priority_class;
    fc.pfc_class = pfc_class;
    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_flow_ctl_en(g_api_lchip, &fc);
    }
    else
    {
        ret = ctc_port_get_flow_ctl_en(&fc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("==============================\n");

    ctc_cli_out("%-12s:\n", desc[fc.dir]);
    ctc_cli_out("enable      :  %d\n", fc.enable);

    ctc_cli_out("==============================\n");

    return ret;
}

CTC_CLI(ctc_cli_port_set_preamble,
        ctc_cli_port_set_preamble_cmd,
        "port GPHYPORT_ID preamble BYTES",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port preamble",
        CTC_CLI_PORT_PREAMBLE_VAL)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport;
    uint8 preamble;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("preamble bytes", preamble, argv[1], 0, CTC_MAX_UINT8_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_preamble(g_api_lchip, gport, preamble);
    }
    else
    {
        ret = ctc_port_set_preamble(gport, preamble);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_min_frame_size,
        ctc_cli_port_set_min_frame_size_cmd,
        "port GPHYPORT_ID min-frame-size SIZE",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Min frame size",
        CTC_CLI_PORT_MIN_FRAME_SIZE)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport;
    uint8 size;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("min frame size", size, argv[1], 0, CTC_MAX_UINT8_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_min_frame_size(g_api_lchip, gport, size);
    }
    else
    {
        ret = ctc_port_set_min_frame_size(gport, size);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_max_frame_size,
        ctc_cli_port_set_max_frame_size_cmd,
        "port GPHYPORT_ID max-frame-size SIZE",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Max frame size per port",
        CTC_CLI_PORT_MAX_FRAME_SIZE)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint32 value = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("max frame size", value, argv[1], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_max_frame(g_api_lchip, gport, value);
    }
    else
    {
        ret = ctc_port_set_max_frame(gport, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_system_max_frame_size,
        ctc_cli_port_set_system_max_frame_size_cmd,
        "system max-frame-size (size0|size1|size2|size3|size4|size5|size6|size7) BYTES",
        "System attribution",
        "Max frame size to apply port select",
        "Max frame size0",
        "Max Frame size1",
        "Max Frame size2",
        "Max Frame size3",
        "Max frame size4",
        "Max Frame size5",
        "Max Frame size6",
        "Max Frame size7",
        CTC_CLI_PORT_MAX_FRAME_SIZE)
{
    int32 ret = CLI_SUCCESS;
    uint16 max_size;
    ctc_frame_size_t index = CTC_FRAME_SIZE_0;

    CTC_CLI_GET_UINT16_RANGE("max frame size", max_size, argv[1], 0, CTC_MAX_UINT16_VALUE);

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
        ret = ctcs_set_max_frame_size(g_api_lchip, index, max_size);
    }
    else
    {
        ret = ctc_set_max_frame_size(index, max_size);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_show_system_max_frame_size,
        ctc_cli_port_show_system_max_frame_size_cmd,
        "show system max-frame-size",
        CTC_CLI_SHOW_STR,
        "System attribution",
        "Max frame size to apply port select")
{
    int32 ret = CLI_SUCCESS;
    uint8 size_idx = 0;
    uint16 val_16 = 0;

    ctc_cli_out("Show system max frame size.\n");
    ctc_cli_out("--------------\n");
    ctc_cli_out("%-8s%s\n", "Index", "Value");
    ctc_cli_out("--------------\n");

    for (size_idx = 0; size_idx < CTC_FRAME_SIZE_MAX; size_idx++)
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_get_max_frame_size(g_api_lchip, size_idx, &val_16);
    }
    else
    {
        ret = ctc_get_max_frame_size(size_idx, &val_16);
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

CTC_CLI(ctc_cli_port_set_all_mac_en,
        ctc_cli_port_set_all_mac_en_cmd,
        "port all mac (enable |disable)",
        CTC_CLI_PORT_M_STR,
        "All port",
        "Port mac",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip_num = 0;
    uint8 chip_id;
    uint8 gchip;
    bool enable;
    uint16 port_id;
    uint16 gport;
    uint32 max_phy_port = 0;
    uint32 max_port_num_per_chip = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }


    if (g_ctcs_api_en)
    {
        ret = ctcs_get_local_chip_num(g_api_lchip, &lchip_num);
    }
    else
    {
        ret = ctc_get_local_chip_num(&lchip_num);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    /*for sgmac, should disable first*/
    if (TRUE == enable)
    {
        uint8 chip_type = 0;
        for (chip_id = 0; chip_id < lchip_num; chip_id++)
        {
            if (g_ctcs_api_en)
            {
                ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
            }
            else
            {
                ret = ctc_get_gchip_id(chip_id, &gchip);
            }

            if (g_ctcs_api_en)
            {
                chip_type = ctcs_get_chip_type(g_api_lchip);
            }
            else
            {
                chip_type = ctc_get_chip_type();
            }

            if (CTC_CHIP_GREATBELT == chip_type)
            {
                for (port_id = 48; port_id < max_phy_port; port_id++)
                {
                    gport = CTC_MAP_LPORT_TO_GPORT(gchip, port_id);

                    if(g_ctcs_api_en)
                    {
                        ret = ret ? ret : ctcs_port_set_mac_en(g_api_lchip, gport, FALSE);
                    }
                    else
                    {
                        ret = ret ? ret : ctc_port_set_mac_en(gport, FALSE);
                    }
                }
            }
        }
    }

    for (chip_id = 0; chip_id < lchip_num; chip_id++)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
        }
        else
        {
            ret = ctc_get_gchip_id(chip_id, &gchip);
        }

        for (port_id = 0; port_id < max_port_num_per_chip; port_id++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, port_id);

            if(g_ctcs_api_en)
            {
                ret = ctcs_port_set_mac_en(g_api_lchip, gport, enable);
            }
            else
            {
                ret = ctc_port_set_mac_en(gport, enable);
            }
            /* DRV_E_MAC_IS_NOT_USED:9912, DRV_E_DATAPATH_INVALID_PORT_ID:9908 */
            if ((ret < 0) && ((ret != -9911) && (ret != -9907)&&(ret!=CTC_E_INVALID_CONFIG)))
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            }
        }
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_port_set_cpu_mac_en(g_api_lchip, enable);
    }
    else
    {
        ret = ctc_port_set_cpu_mac_en(enable);
    }
    if (ret < 0)
    {
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_port_set_all_port_en,
        ctc_cli_port_set_all_port_en_cmd,
        "port all port-en (enable |disable) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        "All port",
        "Port-en",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    bool enable = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 loop = 0;
    ctc_global_panel_ports_t phy_ports;


    sal_memset(&phy_ports, 0, sizeof(phy_ports));

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    phy_ports.lchip = lchip;

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_PANEL_PORTS, (void*)&phy_ports);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_PANEL_PORTS, (void*)&phy_ports);
    }

    if (ret < 0)
    {
        return CLI_ERROR;
    }
    for (loop = 0; loop < phy_ports.count; loop++)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
        }
        else
        {
            ret = ctc_get_gchip_id(lchip, &gchip);
        }
        if (ret < 0)
        {
            return CLI_ERROR;
        }
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, phy_ports.lport[loop]);
        if(g_ctcs_api_en)
        {
            ret = ctcs_port_set_port_en(g_api_lchip, gport, enable);
        }
        else
        {
            ret = ctc_port_set_port_en(gport, enable);
        }

        /* DRV_E_MAC_IS_NOT_USED:9912, DRV_E_DATAPATH_INVALID_PORT_ID:9908 */
        if ((ret < 0) && ((ret != -9911) && (ret != -9907)&&(ret!=CTC_E_INVALID_CONFIG)))
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        }
    }

    if (ret < 0)
    {
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_pading,
        ctc_cli_port_set_pading_cmd,
        "port GPHYPORT_ID padding (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "L2 padding",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_pading_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_pading_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("disable", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_pading_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_pading_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_random_log,
        ctc_cli_port_set_random_log_cmd,
        "port GPHYPORT_ID random-log direction (ingress|egress|both)(enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port random log",
        "Flow direction",
        "Ingress port log",
        "Egress port log",
        "Both direction",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;
    ctc_direction_t dir = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("in", argv[1], 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp("eg", argv[1], 2))
    {
        dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp("bo", argv[1], 2))
    {
        dir = CTC_BOTH_DIRECTION;
    }


    if (0 == sal_memcmp("en", argv[2], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_random_log_en(g_api_lchip, gport, dir, TRUE);
        }
        else
        {
            ret = ctc_port_set_random_log_en(gport, dir, TRUE);
        }
    }
    else if (0 == sal_memcmp("disable", argv[2], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_random_log_en(g_api_lchip, gport, dir, FALSE);
        }
        else
        {
            ret = ctc_port_set_random_log_en(gport, dir, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_random_threshold,
        ctc_cli_port_set_random_threshold_cmd,
        "port GPHYPORT_ID random-threshold direction (ingress|egress|both) THRESHOLD",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "The percentage of received packet for random log",
        "Flow direction ",
        "Ingress port random log percent",
        "Egress port random log percent",
        "Both direction",
        "Percent value, <0-100>")
{
    int32 ret = 0;
    uint16 gport = 0;
    uint16 threshold = 0;
    ctc_direction_t dir = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("in", argv[1], 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp("eg", argv[1], 2))
    {
        dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp("bo", argv[1], 2))
    {
        dir = CTC_BOTH_DIRECTION;
    }

    CTC_CLI_GET_UINT8_RANGE("threshold", threshold, argv[2], 0, CTC_MAX_UINT8_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_random_log_percent(g_api_lchip, gport, dir, threshold);
    }
    else
    {
        ret = ctc_port_set_random_log_percent(gport, dir, threshold);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_srcdiscard,
        ctc_cli_port_set_srcdiscard_cmd,
        "port GPHYPORT_ID src-discard (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port src_discard",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_srcdiscard_en(g_api_lchip, gport, TRUE);
        }
        else
        {
            ret = ctc_port_set_srcdiscard_en(gport, TRUE);
        }
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_srcdiscard_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_srcdiscard_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_loopback,
        ctc_cli_port_set_loopback_cmd,
        "port GPHYPORT_ID (loopback-to | tap-to) dst-port DST_GPORT (enable | disable) (swap-mac|)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port loopback",
        "Mac tap",
        "Dst port, if dst port equal src port, packet will loopback to self",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Swap-mac enable")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_port_lbk_param_t port_lbk;
    uint16 src_gport = 0;
    uint16 dst_gport = 0;

    sal_memset(&port_lbk, 0, sizeof(ctc_port_lbk_param_t));

    CTC_CLI_GET_UINT16("gport", src_gport, argv[0]);
    port_lbk.src_gport = src_gport;
    CTC_CLI_GET_UINT16("gport", dst_gport, argv[2]);
    port_lbk.dst_gport = dst_gport;

    if (0 == sal_memcmp("en", argv[3], 2))
    {
        port_lbk.lbk_enable = TRUE;
    }
    else if (0 == sal_memcmp("di", argv[3], 2))
    {
        port_lbk.lbk_enable = FALSE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("swap-mac");
    if (0xFF != index)
    {
        port_lbk.lbk_type = CTC_PORT_LBK_TYPE_SWAP_MAC;
    }
    else
    {
        port_lbk.lbk_type = CTC_PORT_LBK_TYPE_BYPASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("loopback-to");
    if (index != 0xFF)
    {
        port_lbk.lbk_mode = CTC_PORT_LBK_MODE_CC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tap-to");
    if (index != 0xFF)
    {
        port_lbk.lbk_mode = CTC_PORT_LBK_MODE_TAP;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_loopback(g_api_lchip, &port_lbk);
    }
    else
    {
        ret = ctc_port_set_loopback(&port_lbk);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_efm_lb_mode_enable,
        ctc_cli_port_efm_lb_mode_enable_cmd,
        "port GPHYPORT_ID efm-loop-back enable (redirect-to-cpu INDEX|)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "EFM loop-back mode",
        "Enable EFM loop-back mode this port",
        "Redirect to cpu",
        "Index <0-15>")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_port_lbk_param_t port_lbk;

    sal_memset(&port_lbk, 0, sizeof(ctc_port_lbk_param_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    port_lbk.src_gport = gport;
    port_lbk.dst_gport = gport;

    port_lbk.lbk_enable = TRUE;
    port_lbk.lbk_type   = CTC_PORT_LBK_TYPE_BYPASS;
    port_lbk.lbk_mode   = CTC_PORT_LBK_MODE_EFM;

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_loopback(g_api_lchip, &port_lbk);
    }
    else
    {
        ret = ctc_port_set_loopback(&port_lbk);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_efm_lb_mode_disable,
        ctc_cli_port_efm_lb_mode_disable_cmd,
        "port GPHYPORT_ID efm-loop-back disable",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "EFM loop-back mode",
        "Disable EFM loop-back mode this port")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    ctc_port_lbk_param_t port_lbk;

    sal_memset(&port_lbk, 0, sizeof(ctc_port_lbk_param_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    port_lbk.src_gport  = gport;
    port_lbk.dst_gport  = gport;

    port_lbk.lbk_enable = FALSE;
    port_lbk.lbk_mode   = CTC_PORT_LBK_MODE_EFM;

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_loopback(g_api_lchip, &port_lbk);
    }
    else
    {
        ret = ctc_port_set_loopback(&port_lbk);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_port_check_enable,
        ctc_cli_port_set_port_check_enable_cmd,
        "port GPHYPORT_ID port-check-enable (enable | disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Src port match check",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);


    if (0 == sal_memcmp("en", argv[1], 2))
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_port_check_en(g_api_lchip, gport, TRUE);
    }
    else
    {
        ret = ctc_port_set_port_check_en(gport, TRUE);
    }
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_port_check_en(g_api_lchip, gport, FALSE);
        }
        else
        {
            ret = ctc_port_set_port_check_en(gport, FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_system_ipg_size,
        ctc_cli_port_set_system_ipg_size_cmd,
        "system set ipg-index INDEX size SIZE",
        "System attribution",
        "Set",
        "Ipg index",
        "Value <0-3>",
        "Size",
        "Value <0-255>")
{
    int32 ret = 0;
    ctc_ipg_size_t index;
    uint8 size;

    CTC_CLI_GET_UINT32("ipg-index", index, argv[0]);
    CTC_CLI_GET_UINT8("size", size, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_set_ipg_size(g_api_lchip, index, size);
    }
    else
    {
        ret = ctc_set_ipg_size(index, size);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_ipg_index,
        ctc_cli_port_set_ipg_index_cmd,
        "port GPHYPORT_ID ipg-index INDEX",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Ipg index",
        "Value <0-3>")
{
    int32 ret = 0;
    uint16 gport;
    ctc_ipg_size_t index;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    CTC_CLI_GET_UINT32("ipg-index", index, argv[1]);


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_ipg(g_api_lchip, gport, index);
    }
    else
    {
        ret = ctc_port_set_ipg(gport, index);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_reflective_bridge,
        ctc_cli_port_set_reflective_bridge_cmd,
        "port GPHYPORT_ID reflective-bridge (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Reflective bridge",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 gport;
    bool enable = TRUE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp("di", argv[1], 2))
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_reflective_bridge_en(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_reflective_bridge_en(gport, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_isolation_id,
        ctc_cli_port_set_isolation_id_cmd,
        "port GPHYPORT_ID isolation-id (direction (ingress|egress|both)|) \
        ({ucast-flooding | mcast-flooding | known-ucast-forwarding | known-mcast-forwarding | bcast-flooding}|) \
        value VALUE",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port isolation",
        "Flow direction ",
        "Ingress",
        "Egress",
        "Both direction",
        "Unknown ucast packet will be isolated",
        "Unknown mcast packet will be isolated",
        "Known ucast packet will be isolated",
        "Known mcast packet will be isolated",
        "Broadcast packet will be isolated",
        "Isolation ID value",
        "<0-31>, 0 means disable")
{
    int32 ret = 0;
    uint16 gport = 0;
    uint8 index = 0xFF;
    ctc_port_restriction_t port_restriction;

    sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    port_restriction.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    port_restriction.type = CTC_PORT_ISOLATION_ALL;

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        port_restriction.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        port_restriction.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (index != 0xFF)
    {
        port_restriction.dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
    if (index != 0xFF)
    {
        port_restriction.type = port_restriction.type | CTC_PORT_ISOLATION_UNKNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
    if (index != 0xFF)
    {
        port_restriction.type = port_restriction.type | CTC_PORT_ISOLATION_UNKNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
    if (index != 0xFF)
    {
        port_restriction.type = port_restriction.type | CTC_PORT_ISOLATION_KNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
    if (index != 0xFF)
    {
        port_restriction.type = port_restriction.type | CTC_PORT_ISOLATION_KNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
    if (index != 0xFF)
    {
        port_restriction.type = port_restriction.type | CTC_PORT_ISOLATION_BCAST;
    }

    CTC_CLI_GET_UINT16("isolation_id", port_restriction.isolated_id, argv[argc - 1]);

    index = CTC_CLI_GET_ARGC_INDEX("value");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("value", port_restriction.isolated_id, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_restriction(g_api_lchip, gport, &port_restriction);
    }
    else
    {
        ret = ctc_port_set_restriction(gport, &port_restriction);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_port_set_port_isolation,
        ctc_cli_port_set_port_isolation_cmd,
        "port isolation (use-isolation-id|) (source VALUE) dest {pbmp0 PORT_BITMAP_0 | pbmp1 PORT_BITMAP_1 | pbmp2 PORT_BITMAP_2 | pbmp3 PORT_BITMAP_3 | \
        pbmp4 PORT_BITMAP_4 | pbmp5 PORT_BITMAP_5 | pbmp6 PORT_BITMAP_6 | pbmp7 PORT_BITMAP_7 | pbmp8 PORT_BITMAP_8 | pbmp9 PORT_BITMAP_9 } \
           (packet-type {ucast-flooding | mcast-flooding | known-ucast-forwarding | known-mcast-forwarding | bcast-flooding}|) (mlag-en(is-linkagg|)|) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        "Port isolation",
        "Use isolation id",
        "Source ",
        "If use isolation id, value is isolation id, else value is port",
        "Destination",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Isolate forward packet type",
        "Unknown ucast packet will be isolated",
        "Unknown mcast packet will be isolated",
        "Known ucast packet will be isolated",
        "Known mcast packet will be isolated",
        "Broadcast packet will be isolated",
        "Mlag enable",
        "if set, dest is linkagg bitmap, used for MLAG port isolation",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint8 lchip = 0;
    ctc_port_isolation_t port_isolation;

    sal_memset(&port_isolation, 0, sizeof(port_isolation));

    index = CTC_CLI_GET_ARGC_INDEX("source");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("source", port_isolation.gport, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-isolation-id");
    if (index != 0xFF)
    {
        port_isolation.use_isolation_id = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp0");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 0", port_isolation.pbm[0], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp1");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 1", port_isolation.pbm[1], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp2");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 2", port_isolation.pbm[2], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp3");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 3", port_isolation.pbm[3], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp4");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 4", port_isolation.pbm[4], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp5");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 5", port_isolation.pbm[5], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp6");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 6", port_isolation.pbm[6], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp7");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 7", port_isolation.pbm[7], argv[index + 1]);
    }

    if (MAX_PORT_NUM_PER_CHIP > 256)
    {
        index = CTC_CLI_GET_ARGC_INDEX("pbmp8");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("port bitmap 8", port_isolation.pbm[8], argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX("pbmp9");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("port bitmap 9", port_isolation.pbm[9], argv[index + 1]);
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |=  CTC_PORT_ISOLATION_UNKNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |=  CTC_PORT_ISOLATION_UNKNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_KNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_KNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_BCAST;
    }


    index = CTC_CLI_GET_ARGC_INDEX("mlag-en");
    if (0xFF != index)
    {
       port_isolation.mlag_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-linkagg");
    if (index != 0xFF)
    {
        port_isolation.is_linkagg = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip id", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_isolation(g_api_lchip, &port_isolation);
    }
    else
    {
        ret = ctc_port_set_isolation(lchip, &port_isolation);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_port_show_port_isolation,
        ctc_cli_port_show_port_isolation_cmd,
        "show port isolation (source VALUE) (packet-type {ucast-flooding | mcast-flooding | known-ucast-forwarding | known-mcast-forwarding | bcast-flooding}|) (mlag-en(is-linkagg|)|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        "Port isolation",
        "Source ",
        "Value",
         "Isolate forward packet type",
        "Unknown ucast packet will be isolated",
        "Unknown mcast packet will be isolated",
        "Known ucast packet will be isolated",
        "Known mcast packet will be isolated",
        "Broadcast packet will be isolated",
        "Mlag enable",
        "if set, dest is linkagg bitmap, used for MLAG port isolation",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint8 lchip = 0;

    ctc_port_isolation_t port_isolation;

    sal_memset(&port_isolation, 0, sizeof(port_isolation));

    index = CTC_CLI_GET_ARGC_INDEX("source");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("source", port_isolation.gport, argv[index+1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |=  CTC_PORT_ISOLATION_UNKNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |=  CTC_PORT_ISOLATION_UNKNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_KNOW_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_KNOW_UCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
    if (index != 0xFF)
    {
        port_isolation.isolation_pkt_type |= CTC_PORT_ISOLATION_BCAST;
    }


    index = CTC_CLI_GET_ARGC_INDEX("mlag-en");
    if (0xFF != index)
    {
       port_isolation.mlag_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-linkagg");
    if (0xFF != index)
    {
       port_isolation.is_linkagg = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip id", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_isolation(g_api_lchip, &port_isolation);
    }
    else
    {
        ret = ctc_port_get_isolation(lchip, &port_isolation);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (port_isolation.mlag_en)
    {
        ctc_cli_out("show port isolation mlag: \n");
        ctc_cli_out("=====================================\n");
        if (!(port_isolation.is_linkagg))
        {
            ctc_cli_out("Source port   : 0x%x\n",    port_isolation.gport);
            ctc_cli_out("pbm0            : 0x%x\n", port_isolation.pbm[0]);
            ctc_cli_out("pbm1            : 0x%x\n", port_isolation.pbm[1]);
            ctc_cli_out("pbm2            : 0x%x\n", port_isolation.pbm[2]);
            ctc_cli_out("pbm3            : 0x%x\n", port_isolation.pbm[3]);
            ctc_cli_out("pbm4            : 0x%x\n", port_isolation.pbm[4]);
            ctc_cli_out("pbm5            : 0x%x\n", port_isolation.pbm[5]);
            ctc_cli_out("pbm6            : 0x%x\n", port_isolation.pbm[6]);
            ctc_cli_out("pbm7            : 0x%x\n", port_isolation.pbm[7]);
        }
        else
        {
            ctc_cli_out("Source port   : 0x%x\n",    port_isolation.gport);
            ctc_cli_out("pbm0            : 0x%x\n", port_isolation.pbm[0]);
            ctc_cli_out("pbm1            : 0x%x\n", port_isolation.pbm[1]);
        }
    }
    else
    {
        ctc_cli_out("show port isolation egress: \n");
        ctc_cli_out("=====================================\n");
        ctc_cli_out("use isolation id: %d\n",   port_isolation.use_isolation_id);
        ctc_cli_out("pbm0            : 0x%x\n", port_isolation.pbm[0]);
        ctc_cli_out("pbm1            : 0x%x\n", port_isolation.pbm[1]);
        ctc_cli_out("pbm8            : 0x%x\n", port_isolation.pbm[8]);
        ctc_cli_out("pbm9            : 0x%x\n", port_isolation.pbm[9]);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_set_internal_port,
        ctc_cli_port_set_internal_port_cmd,
        "port reserve internal-port LOCAL_PORT gchip GCHIP type (iloop|eloop|discard|fwd port GPHYPORT_ID)",
        CTC_CLI_PORT_M_STR,
        "Reserve",
        "Internal port",
        "Internal port value",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Type",
        "Iloop",
        "Eloop",
        "Discard",
        "Fwd",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    ctc_internal_port_assign_para_t port_assign;

    sal_memset(&port_assign, 0, sizeof(port_assign));

    CTC_CLI_GET_UINT16("lport", port_assign.inter_port, argv[0]);
    CTC_CLI_GET_UINT8("gchip", port_assign.gchip, argv[1]);

    if (0 == sal_memcmp("il", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    }
    else if (0 == sal_memcmp("el", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    }
    else if (0 == sal_memcmp("di", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_DISCARD;
    }
    else
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_FWD;
        CTC_CLI_GET_UINT16("gport", port_assign.fwd_gport, argv[4]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_set_internal_port(g_api_lchip, &port_assign);
    }
    else
    {
        ret = ctc_set_internal_port(&port_assign);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_allocate_internal_port,
        ctc_cli_port_allocate_internal_port_cmd,
        "port allocate internal-port gchip GCHIP type ((iloop |eloop nhid NHID) (slice-id SLICE_ID|)|discard|fwd port GPHYPORT_ID)",
        CTC_CLI_PORT_M_STR,
        "Allocate",
        "Internal port",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Type",
        "Iloop",
        "Eloop",
        "NHID",
        CTC_CLI_NH_ID_STR,
        "Slice ID",
        "Slice ID",
        "Discard",
        "Fwd",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_internal_port_assign_para_t port_assign;

    sal_memset(&port_assign, 0, sizeof(port_assign));

    CTC_CLI_GET_UINT8("gchip", port_assign.gchip, argv[0]);

    if (0 == sal_memcmp("il", argv[1], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    }
    else if (0 == sal_memcmp("el", argv[1], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
        CTC_CLI_GET_UINT32("Nhid", port_assign.nhid, argv[3]);
    }
    else if (0 == sal_memcmp("di", argv[1], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_DISCARD;
    }
    else
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_FWD;
        CTC_CLI_GET_UINT16("gport", port_assign.fwd_gport, argv[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("slice-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("slice-id", port_assign.slice_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_alloc_internal_port(g_api_lchip, &port_assign);
    }
    else
    {
        ret = ctc_alloc_internal_port(&port_assign);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%% The allocate internal port is %d\n", port_assign.inter_port);

    return ret;
}

CTC_CLI(ctc_cli_port_release_internal_port,
        ctc_cli_port_release_internal_port_cmd,
        "port release internal-port LOCAL_PORT gchip GCHIP type (iloop|eloop|discard|fwd port GPHYPORT_ID)",
        CTC_CLI_PORT_M_STR,
        "Release",
        "Internal port",
        "Internal Port Id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Type",
        "Iloop",
        "Eloop",
        "Discard",
        "Fwd",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    ctc_internal_port_assign_para_t port_assign;

    sal_memset(&port_assign, 0, sizeof(port_assign));

    CTC_CLI_GET_UINT16("lport", port_assign.inter_port, argv[0]);
    CTC_CLI_GET_UINT8("gchip", port_assign.gchip, argv[1]);

    if (0 == sal_memcmp("il", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    }
    else if (0 == sal_memcmp("el", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    }
    else if (0 == sal_memcmp("di", argv[2], 2))
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_DISCARD;
    }
    else
    {
        port_assign.type = CTC_INTERNAL_PORT_TYPE_FWD;
        CTC_CLI_GET_UINT16("gport", port_assign.fwd_gport, argv[4]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_free_internal_port(g_api_lchip, &port_assign);
    }
    else
    {
        ret = ctc_free_internal_port(&port_assign);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_show_ipg_size,
        ctc_cli_port_show_ipg_size_cmd,
        "show system ipg-index INDEX size",
        CTC_CLI_SHOW_STR,
        "System attribution",
        "Ipg index",
        "Value <0-3>",
        "Size")
{
    int32 ret = 0;
    ctc_ipg_size_t index;
    uint8 size;

    CTC_CLI_GET_UINT32("ipg-index", index, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_get_ipg_size(g_api_lchip, index, &size);
    }
    else
    {
        ret = ctc_get_ipg_size(index, &size);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("ipg size              :%d\n", size);

    return ret;
}

CTC_CLI(ctc_cli_port_show_port_mac,
        ctc_cli_port_show_port_mac_cmd,
        "show port GPHYPORT_ID port-mac",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port MAC")
{
    int32 ret = 0;
    uint16 gport_id = 0;
    mac_addr_t port_mac;

    CTC_CLI_GET_UINT16("gport", gport_id, argv[0]);

    ctc_cli_out("Show GPort:0x%04x\n", gport_id);
    ctc_cli_out("==============================\n");

    sal_memset(&port_mac, 0, sizeof(mac_addr_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_port_mac(g_api_lchip, gport_id, &port_mac);
    }
    else
    {
        ret = ctc_port_get_port_mac(gport_id, &port_mac);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Port MAC              :  %02x%02x.%02x%02x.%02x%02x\n", port_mac[0], port_mac[1], port_mac[2], port_mac[3], port_mac[4], port_mac[5]);

    return ret;
}

CTC_CLI(ctc_cli_port_show_port_info,
        ctc_cli_port_show_port_info_cmd,
        "show port GPHYPORT_ID (all | port-en | receive | transmit | bridge | default-vlan | port-cross-connect \
        | learning-enable | sub-if | phy-if | vlan-ctl | ingress-vlan-filtering | egress-vlan-filtering | \
        vlan-domain |ingress-stpid-index|egress-stpid-index | untag-defalt-vlan| dot1q-type| \
        use-outer-ttl|port-blocking (ucast-flooding|mcast-flooding|known-ucast-forwarding|known-mcast-forwarding|bcast-flooding ) |protocol-vlan| \
        mac-en|signal-detect|speed-mode|preamble|min-frame-size|max-frame-size|stretch-mode|padding|ingress-random-log|egress-random-log| \
        ingress-random-threshold|egress-random-threshold|src-discard|port-check-enable|mac-link-status|ipg | ingress-isolation-id |\
        egress-isolation-id | isolation-id | reflective-bridge| | ((scl-key-type scl-id SCL_ID | vlan-range) direction (ingress | egress)))",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Show all property",
        "Port state",
        "Reception port state",
        "Transmission port state",
        "L2 bridge",
        "Default vlan id",
        "Port cross connect",
        "Mac learning enable",
        "Routed port l3 only",
        "Route enable on port",
        "Vlan tag control",
        "Ingress vlan filtering",
        "Egress vlan filtering",
        "Vlan domain when STPID is the same as CTPID",
        "Ingress stag tpid index",
        "Egress stag tpid index",
        "Untag-defalt-vlan",
        "Port's type descript in 802.1q",
        "Use outer ttl in case of tuunel",
        "Port based blocking",
        "Unknown ucast flooding to this port",
        "Unknown mcast flooding to this port",
        "Known ucast forwarding to this port",
        "Known mcast forwarding to this port",
        "Broadcast flooding to this port",
        "Protocol vlan",
        "Mac enable",
        "Signal detect",
        "Speed mode",
        "Preamble value",
        "Min frame size",
        "Max frame size",
        "Stretch mode",
        "L2 padding",
        "Ingress random log",
        "Egress random log",
        "The percentage of received packet for Ingress random log",
        "The percentage of received packet for egress random log",
        "Src discard",
        "Enable to check src port match",
        "Mac link status",
        "Ipg index",
        "Ingress isolation ID",
        "Egress isolation ID",
        "Isolation Id",
        "Reflective bridge enable",
        "SCL key type",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE,
        "Vlan Range",
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret = CLI_SUCCESS;
    uint32 gport;
    uint8 val_8 = 0;
    uint16 val_16 = 0;
    uint32 val_32 = 0;
    bool val_b = 0;
    bool val2_b = 0;
    ctc_ipg_size_t ipg_index = 0;
    char* str = NULL;
    uint8 index = 0xFF;
    uint8 show_all = 0;
    uint32 value = 0;
    ctc_port_restriction_t port_restrict;
    bool enable = FALSE;
    ctc_direction_t dir = CTC_INGRESS;
    ctc_port_scl_property_t port_scl_property;
    ctc_vlan_range_info_t vrange_info;

    char* igs_scl_hash_key_type[CTC_PORT_IGS_SCL_HASH_TYPE_MAX] = {"disable", "dvid(port cvlan svlan)", "svid",
                                                                   "cvid", "svid-scos", "cvid-ccos",
                                                                   "mac-sa", "port macsa", "mac-da", "port-mac-da",
                                                                   "ipsa", "port-ipsa", "port", "L2", "tunnel",
                                                                   "tunnel-rpf", "tunnel auto", "nvgre", "vxlan", "trill", "svid-dscp","svid-only","svid-mac"};

    char* egs_scl_hash_key_type[CTC_PORT_EGS_SCL_HASH_TYPE_MAX] = {"disable", "dvid", "svid", "cvid", "svid-scos", "cvid-ccos", "port", "port-cross", "port-vlan-cross"};

    char* scl_tcam_key_type[CTC_PORT_IGS_SCL_TCAM_TYPE_MAX + 1] = {"disable",
                                                                   "layer2 tcam key",
                                                                   "layer2/layer3 tcam key",
                                                                   "layer3 tcam key",
                                                                   "vlan tcam key",
                                                                   "layer3 tcam key",
                                                                   "layer3 tcam key",

                                                                   "resolve conflict",
                                                                   "udf tcam key",
                                                                   "udf ext tcam key",

                                                                   ""};
    char port_str[CTC_PORT_PVLAN_COMMUNITY + 1][32] = {{"none"}, {"promiscuous"}, {"isolated"}, {"community"}};

    sal_memset(&port_scl_property, 0, sizeof(ctc_port_scl_property_t));
    sal_memset(&port_restrict, 0, sizeof(ctc_port_restriction_t));
    sal_memset(&vrange_info, 0, sizeof(ctc_vlan_range_info_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        show_all = 1;
    }

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);

    ctc_cli_out("%-45s:  0x%04x\n","Show GPort", gport);
    ctc_cli_out("--------------------------------------------------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("port-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_port_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_port_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port enable", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("receive");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_receive_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_receive_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Receive enable", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("transmit");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_transmit_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_transmit_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Transmit enable",val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("bridge");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_bridge_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_bridge_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Bridge enable", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-vlan");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_default_vlan(g_api_lchip, gport, &val_16);
        }
        else
        {
            ret = ctc_port_get_default_vlan(gport, &val_16);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Default vlan",val_16);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-cross-connect");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_cross_connect(g_api_lchip, gport, &val_32);
        }
        else
        {
            ret = ctc_port_get_cross_connect(gport, &val_32);
        }
        if (ret >= 0)
        {
            if (0xFFFFFFFF == val_32)
            {
                ctc_cli_out("%-45s:  %s\n", "Port cross connect", "Invalid");
            }
            else
            {
                ctc_cli_out("%-45s:  0x%X\n", "Port cross connect", val_32);
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("learning-enable");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_learning_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_learning_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Learning enable", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-if");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_phy_if_en(g_api_lchip, gport, &val_16, &val_b);
        }
        else
        {
            ret = ctc_port_get_phy_if_en(gport, &val_16, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Phy-if enable", val_b);
        }

        if (TRUE == val_b)
        {
            ctc_cli_out("%-45s:  %u\n", "Phy-if L3IFID", val_16);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("sub-if");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_sub_if_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_sub_if_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Sub-if enable", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-ctl");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_vlan_ctl(g_api_lchip, gport, (ctc_vlantag_ctl_t*)&val_32);
        }
        else
        {
            ret = ctc_port_get_vlan_ctl(gport, (ctc_vlantag_ctl_t*)&val_32);
        }
        if (ret >= 0)
        {
            switch (val_32)
            {
            case 0:
                str = "allow-all";
                break;

            case 1:
                str = "drop-all-untagged";
                break;

            case 2:
                str = "drop-all-tagged";
                break;

            case 3:
                str = "drop-all";
                break;

            case 4:
                str = "drop-wo-2tag";
                break;

            case 5:
                str = "drop-w-2tag";
                break;

            case 6:
                str = "drop-stag";
                break;

            case 7:
                str = "drop-non-stag";
                break;

            case 8:
                str = "drop-only-stag";
                break;

            case 9:
                str = "permit-only-stag";
                break;

            case 10:
                str = "drop-all-ctag";
                break;

            case 11:
                str = "drop-non-ctag";
                break;

            case 12:
                str = "drop-only-ctag";
                break;

            case 13:
                str = "permit-only-ctag";
                break;

            case 14:
                str = "allow-all";
                break;

            default:
                break;
            }

            ctc_cli_out("%-45s:  %s\n", "Vlan control", str);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("untag-defalt-vlan");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_untag_dft_vid(g_api_lchip, gport, &val_b, &val2_b);
        }
        else
        {
            ret = ctc_port_get_untag_dft_vid(gport, &val_b, &val2_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Untag default vid", val_b);
            ctc_cli_out("%-45s:  %u\n", "Untag svlan", val2_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-vlan-filtering");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_vlan_filter_en(g_api_lchip, gport, CTC_INGRESS, &val_b);
        }
        else
        {
            ret = ctc_port_get_vlan_filter_en(gport, CTC_INGRESS, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ingress vlan filtering", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-vlan-filtering");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_vlan_filter_en(g_api_lchip, gport, CTC_EGRESS, &val_b);
        }
        else
        {
            ret = ctc_port_get_vlan_filter_en(gport, CTC_EGRESS, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Egress vlan filtering", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_vlan_domain(g_api_lchip, gport, (ctc_port_vlan_domain_type_t*)&val_b);
        }
        else
        {
            ret = ctc_port_get_vlan_domain(gport, (ctc_port_vlan_domain_type_t*)&val_b);
        }
        if (0 == val_b)
        {
            str = "svlan";
        }
        else
        {
            str = "cvlan";
        }

        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %s\n", "Vlan domain", str);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dot1q-type");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_dot1q_type(g_api_lchip, gport, (ctc_dot1q_type_t*)&val_32);
        }
        else
        {
            ret = ctc_port_get_dot1q_type(gport, (ctc_dot1q_type_t*)&val_32);
        }
        if (ret >= 0)
        {
            switch (val_32)
            {
            case CTC_DOT1Q_TYPE_NONE:
                str = "untagged";
                break;

            case CTC_DOT1Q_TYPE_CVLAN:
                str = "cvlan-tagged";
                break;

            case CTC_DOT1Q_TYPE_SVLAN:
                str = "svlan-tagged";
                break;

            case CTC_DOT1Q_TYPE_BOTH:
                str = "double-tagged";
                break;

            default:
                break;
            }

            ctc_cli_out("%-45s:  %s\n", "Dot1q-type",str);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-outer-ttl");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_use_outer_ttl(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_use_outer_ttl(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Use outer ttl", val_b);
        }
    }

    if (0)
    {
        port_restrict.dir = CTC_EGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if ((ret >= 0) && (CTC_PORT_RESTRICTION_PORT_BLOCKING == port_restrict.mode))
        {
            value = (port_restrict.type & CTC_PORT_BLOCKING_UNKNOW_UCAST) ? 1 : 0;
            ctc_cli_out("Port blocking %-31s:  %u\n", "unknown ucast flood", value);

            value = (port_restrict.type & CTC_PORT_BLOCKING_UNKNOW_MCAST) ? 1 : 0;
            ctc_cli_out("Port blocking %-31s:  %u\n", "unknown mcast flood", value);

            value = (port_restrict.type & CTC_PORT_BLOCKING_KNOW_UCAST) ? 1 : 0;
            ctc_cli_out("Port blocking %-31s:  %u\n", "known ucast forwarding", value);

            value = (port_restrict.type & CTC_PORT_BLOCKING_KNOW_MCAST) ? 1 : 0;
            ctc_cli_out("Port blocking %-31s:  %u\n", "known mcast forwarding", value);

            value = (port_restrict.type & CTC_PORT_BLOCKING_BCAST) ? 1 : 0;
            ctc_cli_out("Port blocking %-31s:  %u\n", "bcast flood", value);
        }

        port_restrict.dir = CTC_INGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if (ret >= 0)
        {
            if (CTC_PORT_RESTRICTION_PVLAN == port_restrict.mode)
            {
                ctc_cli_out("%-45s:  %s\n", "Private vlan", port_str[port_restrict.type]);
            }
            else
            {
                ctc_cli_out("%-45s:  %s\n", "Private vlan", "disable");
            }

            if (CTC_PORT_RESTRICTION_PORT_ISOLATION == port_restrict.mode)
            {
                ctc_cli_out("%-45s:  %u\n", "Ingress port isolation id", port_restrict.isolated_id);
            }
            else
            {
                ctc_cli_out("%-45s:  %s\n", "Ingress port isolation id", "disable");
            }
        }

        port_restrict.dir = CTC_EGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if ((ret >= 0) && (CTC_PORT_RESTRICTION_PORT_ISOLATION == port_restrict.mode))
        {
            ctc_cli_out("%-45s:  %u\n", "Egress port isolation id", port_restrict.isolated_id);
        }
        else
        {
            ctc_cli_out("%-45s:  %s\n", "Egress port isolation id", "disable");
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("isolation-id");
    if (index != 0xFF)
    {
        port_restrict.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
        port_restrict.dir = CTC_INGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port ingress isolation ID/Group", port_restrict.isolated_id);
        }

        port_restrict.dir = CTC_EGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port egress isolation ID/Group", port_restrict.isolated_id);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-isolation-id");
    if (index != 0xFF)
    {
        port_restrict.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
        port_restrict.dir = CTC_INGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port ingress isolation ID/Group", port_restrict.isolated_id);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-isolation-id");
    if (index != 0xFF)
    {
        port_restrict.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
        port_restrict.dir = CTC_EGRESS;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port egress isolation ID/Group", port_restrict.isolated_id);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-blocking");
    if (index != 0xFF)
    {
        port_restrict.mode = CTC_PORT_RESTRICTION_PORT_BLOCKING;
        index = CTC_CLI_GET_ARGC_INDEX("ucast-flooding");
        if (index != 0xFF)
        {
            port_restrict.dir = CTC_EGRESS;
            if(g_ctcs_api_en)
            {
                ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
            }
            else
            {
                ret = ctc_port_get_restriction(gport, &port_restrict);
            }
            value = (port_restrict.type & CTC_PORT_BLOCKING_UNKNOW_UCAST) ? 1 : 0;
            ctc_cli_out("%-45s:  %u\n", "Unknown ucast flood block", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mcast-flooding");
        if (index != 0xFF)
        {
            port_restrict.dir = CTC_EGRESS;
            if(g_ctcs_api_en)
            {
                ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
            }
            else
            {
                ret = ctc_port_get_restriction(gport, &port_restrict);
            }
            value = (port_restrict.type & CTC_PORT_BLOCKING_UNKNOW_MCAST) ? 1 : 0;
            ctc_cli_out("%-45s:  %u\n", "Unknown mcast flood block", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-ucast-forwarding");
        if (index != 0xFF)
        {
            port_restrict.dir = CTC_EGRESS;
            if(g_ctcs_api_en)
            {
                ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
            }
            else
            {
                ret = ctc_port_get_restriction(gport, &port_restrict);
            }
            value = (port_restrict.type & CTC_PORT_BLOCKING_KNOW_UCAST) ? 1 : 0;
            ctc_cli_out("%-45s:  %u\n", "Known ucast forwarding block", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("known-mcast-forwarding");
        if (index != 0xFF)
        {
            port_restrict.dir = CTC_EGRESS;
            if(g_ctcs_api_en)
            {
                ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
            }
            else
            {
                ret = ctc_port_get_restriction(gport, &port_restrict);
            }
            value = (port_restrict.type & CTC_PORT_BLOCKING_KNOW_MCAST) ? 1 : 0;
            ctc_cli_out("%-45s:  %u\n", "Known mcast forwarding block", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("bcast-flooding");
        if (index != 0xFF)
        {
            port_restrict.dir = CTC_EGRESS;
            if(g_ctcs_api_en)
            {
                ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
            }
            else
            {
                ret = ctc_port_get_restriction(gport, &port_restrict);
            }
            value = (port_restrict.type & CTC_PORT_BLOCKING_BCAST) ? 1 : 0;
            ctc_cli_out("%-45s:  %u\n", "Bcast flood block", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-stpid-index");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_stag_tpid_index(g_api_lchip, gport, CTC_INGRESS, &val_8);
        }
        else
        {
            ret = ctc_port_get_stag_tpid_index(gport, CTC_INGRESS, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ingress stag tpid index", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-stpid-index");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_stag_tpid_index(g_api_lchip, gport, CTC_EGRESS, &val_8);
        }
        else
        {
            ret = ctc_port_get_stag_tpid_index(gport, CTC_EGRESS, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Egress stag tpid index", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("protocol-vlan");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_protocol_vlan_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_protocol_vlan_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Protocol vlan",val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_mac_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_mac_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "MAC enable",val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("signal-detect");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SIGNAL_DETECT, &val_32);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SIGNAL_DETECT, &val_32);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Signal detect", val_32);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("speed-mode");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_speed(g_api_lchip, gport, (ctc_port_speed_t*)&val_32);
        }
        else
        {
            ret = ctc_port_get_speed(gport, (ctc_port_speed_t*)&val_32);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Speed mode",val_32);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("preamble");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_preamble(g_api_lchip, gport, &val_8);
        }
        else
        {
            ret = ctc_port_get_preamble(gport, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Preamble", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("min-frame-size");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_min_frame_size(g_api_lchip, gport, &val_8);
        }
        else
        {
            ret = ctc_port_get_min_frame_size(gport, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Min frame size", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("max-frame-size");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_max_frame(g_api_lchip, gport, (ctc_frame_size_t*)&val_32);
        }
        else
        {
            ret = ctc_port_get_max_frame(gport, (ctc_frame_size_t*)&val_32);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Max frame size",val_32);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("padding");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_pading_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_pading_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Padding", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-random-log");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_random_log_en(g_api_lchip, gport, CTC_INGRESS, &val_b);
        }
        else
        {
            ret = ctc_port_get_random_log_en(gport, CTC_INGRESS, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ingress random log",val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-random-log");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_random_log_en(g_api_lchip, gport, CTC_EGRESS, &val_b);
        }
        else
        {
            ret = ctc_port_get_random_log_en(gport, CTC_EGRESS, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Egress random log",val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress-random-threshold");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_random_log_percent(g_api_lchip, gport, CTC_INGRESS, &val_8);
        }
        else
        {
            ret = ctc_port_get_random_log_percent(gport, CTC_INGRESS, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u%%\n", "Ingress random log weight", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-random-threshold");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_random_log_percent(g_api_lchip, gport, CTC_EGRESS, &val_8);
        }
        else
        {
            ret = ctc_port_get_random_log_percent(gport, CTC_EGRESS, &val_8);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u%%\n", "Egress random log weight", val_8);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_srcdiscard_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_srcdiscard_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Src-discard", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-check-enable");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_port_check_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_port_check_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Src-match-check", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-link-status");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_mac_link_up(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_mac_link_up(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "MAC-link-status", val_b);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipg");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_ipg(g_api_lchip, gport, &ipg_index);
        }
        else
        {
            ret = ctc_port_get_ipg(gport, &ipg_index);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ipg index", ipg_index);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("reflective-bridge");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_reflective_bridge_en(g_api_lchip, gport, &val_b);
        }
        else
        {
            ret = ctc_port_get_reflective_bridge_en(gport, &val_b);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Reflective bridge", val_b);
        }
    }

    if ((ret < 0) && (!show_all))
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-key-type");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl id", port_scl_property.scl_id, argv[index + 2], 0, CTC_MAX_UINT8_VALUE);

        if (CLI_CLI_STR_EQUAL("ingress", index + 4))
        {
            port_scl_property.direction = CTC_INGRESS;
        }
        else if (CLI_CLI_STR_EQUAL("egress", index + 4))
        {
            port_scl_property.direction = CTC_EGRESS;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_scl_property(g_api_lchip, gport, &port_scl_property);
        }
        else
        {
            ret = ctc_port_get_scl_property(gport, &port_scl_property);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (CTC_INGRESS == port_scl_property.direction)
        {
            ctc_cli_out("%-45s:  %s\n", "Scl hash key type", igs_scl_hash_key_type[port_scl_property.hash_type]);
            ctc_cli_out("%-45s:  %s\n", "Scl tcam key type", scl_tcam_key_type[port_scl_property.tcam_type]);
            ctc_cli_out("%-45s:  %u\n", "Hash vlan range en", !port_scl_property.hash_vlan_range_dis);
            ctc_cli_out("%-45s:  %u\n", "Tcam vlan range en", !port_scl_property.tcam_vlan_range_dis);
        }
        else
        {
            ctc_cli_out("%-45s:  %s\n", "Scl hash key type", egs_scl_hash_key_type[port_scl_property.hash_type]);
        }

        ctc_cli_out("%-45s:  %u\n", "ClassId en", port_scl_property.class_id_en);
        ctc_cli_out("%-45s:  %u\n", "ClassId", port_scl_property.class_id);
        ctc_cli_out("%-45s:  %u\n", "Use logic port en", port_scl_property.use_logic_port_en);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-range");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("ingress", index + 2))
        {
            dir = CTC_INGRESS;
        }
        else if (CLI_CLI_STR_EQUAL("egress", index + 2))
        {
            dir = CTC_EGRESS;
        }

        vrange_info.direction = dir;

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_vlan_range(g_api_lchip, gport, &vrange_info, &enable);
        }
        else
        {
            ret = ctc_port_get_vlan_range(gport, &vrange_info, &enable);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (!enable)
        {
            ctc_cli_out("%-45s:  %s\n", "Vlan range", "disable");
        }
        else
        {
            ctc_cli_out("%-45s:  %s\n", "Vlan range", "enable");
            ctc_cli_out("%-45s:  %u\n", "Vlan range group", vrange_info.vrange_grpid);
        }
    }

    if (show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ROUTE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ROUTE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Route-enable", value);
        }


        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_HW_LEARN_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_HW_LEARN_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Hw-learn", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RAW_PKT_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RAW_PKT_TYPE, &value);
        }

        switch (value)
        {
        case CTC_PORT_RAW_PKT_ETHERNET:
            str = "ethernet";
            break;

        case CTC_PORT_RAW_PKT_IPV4:
            str = "ipv4";
            break;

        case CTC_PORT_RAW_PKT_IPV6:
            str = "ipv6";
            break;

        case CTC_PORT_RAW_PKT_NONE:
            str = "none";
            break;
        }

        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %s\n", "Raw-packet-type", str);
        }


        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if ((ret >= 0) && (port_restrict.type == CTC_PORT_PVLAN_COMMUNITY) && (port_restrict.mode == CTC_PORT_RESTRICTION_PVLAN))
        {
            ctc_cli_out("%-45s:  %s\n", "Pvlan port type", port_str[port_restrict.type]);
            ctc_cli_out("%-45s:  %u\n", "Pvlan port isolated id", port_restrict.isolated_id);
        }
        else if ((ret >= 0) && (port_restrict.mode == CTC_PORT_RESTRICTION_PVLAN))
        {
            ctc_cli_out("%-45s:  %s\n", "Pvlan port type", port_str[port_restrict.type]);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Tunnel-rpf-check", value);
        }


        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RPF_TYPE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Rpf-type", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Pkt-tag-high-priority", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ipv6-lookup", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ipv4-lookup", value);
        }


        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Use-default-lookup", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ipv6-to-mac-key", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ipv4-to-mac-key", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_TRILL_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Trill", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Discard-non-tril", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Discard-tril", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Reflective-bridge", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FCOE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FCOE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Fcoe", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RPF_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Rpf-check-en", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_COS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Replace-stag-cos", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_TPID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_STAG_TPID, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Replace-stag-tpid", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_CTAG_COS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_CTAG_COS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Replace-ctag-cos", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_DSCP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_DSCP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Replace-dscp", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "L3pdu-arp-action", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "L3pdu-dhcp-action", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PTP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PTP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Ptp", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_IS_LEAF, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_IS_LEAF, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Is-leaf", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PRIORITY_TAG_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PRIORITY_TAG_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Priority-tag", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_PCP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DEFAULT_PCP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Default-pcp", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_DEI, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DEFAULT_DEI, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Default-dei", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Nvgre-mcast-no-decap", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Vxlan-mcast-no-decap", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Scl-field-sel-id", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_QOS_POLICY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_QOS_POLICY, &value);
        }

        switch (value)
        {
            case CTC_QOS_TRUST_PORT:
                str = "port";
                break;

            case CTC_QOS_TRUST_OUTER:
                str = "outer";
                break;

            case CTC_QOS_TRUST_COS:
                str = "cos";
                break;

            case CTC_QOS_TRUST_DSCP:
                str = "dscp";
                break;

            case CTC_QOS_TRUST_IP:
                str = "ip-prec";
                break;

            case CTC_QOS_TRUST_STAG_COS:
                str = "stag-cos";
                break;

            case CTC_QOS_TRUST_CTAG_COS:
                str = "ctag-cos";
                break;

            default:
                str = "unexpect!!!!!";
                break;

        }

        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %s\n", "Qos-trust", str);
        }



        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port auto-neg", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port link change interrupt enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TS_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_MAC_TS_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port mac time stamp", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINKSCAN_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINKSCAN_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port linkscan enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_FAILOVER_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_FAILOVER_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port aps failover enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Port linkagg failover enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SNOOPING_PARSER, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SNOOPING_PARSER, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Snooping parser", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Flow lkup use outer", value);
        }


        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Aware tunnel info enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_ENABLE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_NVGRE_ENABLE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Nvgre enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Metadata overwrite port", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Metadata overwrite udf", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Src mismatch exception en", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_EFD_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_EFD_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Efd en", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Add default vlan disable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LOGIC_PORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LOGIC_PORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  0x%04x\n", "Logic port", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_GPORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_GPORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  0x%04x\n", "Gport", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, &value);
        }
        if (ret >= 0)
        {
            if (0xFFFFFFFF == value)
            {
                ctc_cli_out("%-45s:  %s\n", "Aps select group", "disable");
            }
            else
            {
                ctc_cli_out("%-45s:  %u\n", "Aps select group", value);
            }
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_WORKING, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_SELECT_WORKING, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Aps select working path", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ERROR_CHECK, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ERROR_CHECK, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Error check enable", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_MODE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Auto neg mode", value);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_UNIDIR_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_UNIDIR_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Unidirection", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_MAC_TX_IPG,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Mac tx ipg size", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_OVERSUB_PRI, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_OVERSUB_PRI,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Oversub priority", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Trill mcast decap", value);
        }
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FEC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FEC_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:", "Fec-en");
            if (CTC_PORT_FEC_TYPE_NONE == value)
            {
                ctc_cli_out("  %s\n", "NONE");
            }
            else if (CTC_PORT_FEC_TYPE_RS == value)
            {
                ctc_cli_out("  %s\n", "RS");
            }
            else if (CTC_PORT_FEC_TYPE_BASER == value)
            {
                ctc_cli_out("  %s\n", "BASER");
            }
            ctc_cli_out("\n");
        }
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CUT_THROUGH_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CUT_THROUGH_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Cut through enable", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LEARN_DIS_MODE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LEARN_DIS_MODE,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Learn disable mode", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FORCE_MPLS_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FORCE_MPLS_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Force MPLS decap", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FORCE_TUNNEL_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FORCE_TUNNEL_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Force tunnel decap", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CHK_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CHK_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Check CRC", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_STRIP_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Strip CRC", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APPEND_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Append CRC", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_TOD_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APPEND_TOD_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-45s:  %u\n", "Append TOD", value);
        }
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_FEC,  &value);
        }
        if (ret >= 0)
        {
            if (0 == value)
            {
                ctc_cli_out("Auto Neg FEC is none\n");
            }
            else
            {
                ctc_cli_out("Auto Neg FEC is:\n");
                if (value & (1 << CTC_PORT_FEC_TYPE_RS))
                    ctc_cli_out("\tRS\n");
                if (value & (1 << CTC_PORT_FEC_TYPE_BASER))
                    ctc_cli_out("\tBase-R\n");
            }
        }
    }

    if ((ret < 0) && (!show_all))
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_debug_on,
        ctc_cli_port_debug_on_cmd,
        "debug port (port|mac|cl73) (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_PORT_M_STR,
        "Mac function",
        "cl73 Function",
        "CTC Layer",
        "SYS Layer",
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

    if (0 == sal_memcmp(argv[0], "port", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = PORT_CTC;

        }
        else
        {
            typeenum = PORT_SYS;

        }

        ctc_debug_set_flag("port", "port", typeenum, level, TRUE);
    }
    else if (0 == sal_memcmp(argv[0], "mac", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = MAC_CTC;

        }
        else
        {
            typeenum = MAC_SYS;

        }

        ctc_debug_set_flag("port", "mac", typeenum, level, TRUE);
    }
    else if (0 == sal_memcmp(argv[0], "cl73", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = CL73_CTC;

        }
        else
        {
            typeenum = CL73_SYS;

        }

        ctc_debug_set_flag("port", "cl73", typeenum, level, TRUE);
    }


    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_debug_off,
        ctc_cli_port_debug_off_cmd,
        "no debug port (port|mac|cl73) (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_PORT_M_STR,
        "Mac function",
        "cl73 Function",
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "port", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = PORT_CTC;
        }
        else
        {
            typeenum = PORT_SYS;
        }
        ctc_debug_set_flag("port", "port", typeenum, level, FALSE);
    }
    else if (0 == sal_memcmp(argv[0], "mac", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = MAC_CTC;
        }
        else
        {
            typeenum = MAC_SYS;
        }
        ctc_debug_set_flag("port", "mac", typeenum, level, FALSE);
    }
    else if (0 == sal_memcmp(argv[0], "cl73", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = CL73_CTC;
        }
        else
        {
            typeenum = CL73_SYS;
        }
        ctc_debug_set_flag("port", "cl73", typeenum, level, FALSE);
    }


    return CLI_SUCCESS;
}

/*    "Set low 8 bits for da port MAC",
    "Set da port MAC high 40 bits register select",
    "Set low 8 bits for sa port MAC",
    "Set sa port MAC high 40 bits register select",*/

CTC_CLI(ctc_cli_port_set_property,
        ctc_cli_port_set_property_cmd,
        "port GPHYPORT_ID property (((route-en| pkt-tag-high-priority | hw-learn | ipv6-lookup | ipv4-lookup | use-default-lookup | \
        ipv6-to-mac-key | ipv4-to-mac-key | trill-mc-decap | trill| discard-non-tril | discard-tril | reflective-bridge | fcoe | rpf-check-en | \
        replace-stag-cos| replace-stag-tpid|replace-ctag-cos | replace-dscp | l3pdu-arp-action | l3pdu-dhcp-action | tunnel-rpf-check | ptp | \
        rpf-type | is-leaf | priority-tag | default-pcp | default-dei | nvgre-mcast-no-decap | vxlan-mcast-no-decap |auto-neg |\
        linkscan-en |aps-failover | linkagg-failover |link-intr | mac-ts | scl-field-sel-id | snooping-parser |flow-lkup-use-outer | aware-tunnel-info-en |nvgre-en |\
        metadata-overwrite-port | metadata-overwrite-udf | src-mismatch-exception-en | station-move-priority | station-move-action | efd-en | add-default-vlan-dis | \
        aps-select-group | aps-select-working | error-check| fec-en | fcmap | unidirection | mac-ipg | oversub-priority |cut-through-en | loop-with-src-port | xpipe |mux-type\
        |phy-init|phy-en|phy-duplex|phy-medium|phy-loopback) value VALUE) \
        | logic-port value VALUE | gport value VALUE | \
        | (raw-packet-type ( none | ethernet | ipv4 | ipv6 )) | \
        (pvlan-type ( none | promiscuous | isolated | community isolation-id VALUE)) | \
        (qos-trust (port | outer | cos | dscp | ip-prec | stag-cos | ctag-cos))| \
        (auto-neg-mode (1000base-X| sgmii-slaver | sgmii-master))| \
        (check-crc | strip-crc | append-crc | append-tod) value VALUE| \
        (auto-neg-fec {rs|base-r|none}))",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property",
        "Route enable",
        "Packet tag take precedence over all",
        "Hardware learning",
        "Scl ipv6 lookup disable",
        "Scl ipv4 lookup disable",
        "Scl default lookup",
        "Scl MAC key is used for lookup instead of ipv6 key",
        "Scl MAC key is used for lookup instead of ipv4 key",
        "Set trill mcast decap enable",
        "Set trill enable",
        "Set NON-TRILL packet discard",
        "Set TRILL packet discard",
        "Bridge to the same port enable",
        "FCoE enable",
        "RPF check enable",
        "The STAG COS field is replaced by the classified COS result",
        "The STAG TPID field is replaced by the classified TPID result",
        "The CTAG COS field is replaced by the classified COS result",
        "The Dscp field is replace by qos mapping",
        "ARP packet processing type",
        "DHCP packet processing type",
        "RPF check for outer tunnel IP header is enabled",
        "Enable ptp",
        "Strict or loose",
        "This port connect with a leaf node",
        "Priority tagged packet will be sent out",
        "Default cos of vlan tag",
        "Default dei of vlan tag",
        "NvGRE Mcast packet will do not decapsulate",
        "VxLAN Mcast packet will do not decapsulate",
        "Port support linkscan function",
        "Auto negotiation enable",
        "Aps failover enable",
        "Linkagg failover enable",
        "Pcs link change interrupt enable",
        "Timestamp",
        "Scl key field select id <0-3>",
        "Snooping parser",
        "If set,indicate tunnel packet will use outer header to do ACL/IPFIX flow lookup",
        "Aware tunnel info en",
        "Nvgre enable",
        "Metadata overwrite port",
        "Metadata overwrite udf",
        "Source mismatch exception en",
        "Station move priority",
        "Station move action, 0:fwd, 1:discard, 2:discard and to cpu",
        "Efd enable",
        "Add default vlan disable",
        "Aps select group id, 0xFFFFFFFF means disable aps select",
        "Aps select path, 0 means protecting path, else working path",
        "Mac error check",
        "Forword error correction enable, 0:no-fec, 1:RS fec, 2:base-R fec",
        "Fcmap,0x0EFC00-0x0EFCFF, default is 0x0EFC00",
        "Unidirection",
        "Mac Ipg index",
        "Oversubscription priority",
        "Cut through Enable",
        "Loop with source port",
        "X-pipe feature",
        "Mux-type",
        "Phy init",
        "Phy enable",
        "Phy duplex",
        "Phy medium",
        "Phy loopback",
        "Property value",
        "Value",
        "Logic port",
        "Property Value",
        "Value",
        "Gport",
        "Property Value",
        "Value",
        "Packet type",
        "Raw packet type disable",
        "Port parser ethernet raw packet",
        "Port only parser ipv4 raw packet",
        "Port only parser ipv6 raw packet",
        "Pvlan port type",
        "Port is none port",
        "Port is promiscuous port",
        "Port is isolated port",
        "Port is community port",
        "Community port isolated id",
        "0~15",
        "Qos trust policy",
        "Classifies ingress packets with the port default CoS value",
        "Classifies ingress packets with the outer priority value",
        "Classifies ingress packets with the packet CoS values",
        "Classifies ingress packets with the packet DSCP values",
        "Classifies ingress packets with the packet IP-Precedence values",
        "Classifies ingress packets with the packet Stag CoS values",
        "Classifies ingress packets with the packet Ctag CoS values",
        "Auto neg mode",
        "1000Base-X",
        "Sgmii-slaver",
        "Sgmii-master",
        "Check CRC",
        "Strip CRC",
        "Append CRC",
        "Append TOD",
        "Property value",
        "Value",
        "Auto neg FEC",
        "RS FEC",
        "Base-R FEC",
        "No FEC")
{
    int32 ret = 0;
    uint16 gport;
    uint32 value = 0;
    uint8 index = 0xFF;
    ctc_port_restriction_t port_restriction;

    sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("Property_value", value, argv[index + 1]);
    }

    if (CLI_CLI_STR_EQUAL("route-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_ROUTE_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_ROUTE_EN, value);
        }
    }
    if (CLI_CLI_STR_EQUAL("pkt-tag-high-priority", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("hw-learn", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_HW_LEARN_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_HW_LEARN_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("fec-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_FEC_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_FEC_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("ipv6-lookup", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("ipv4-lookup", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("use-default-lookup", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("ipv6-to-mac-key", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("ipv4-to-mac-key", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("trill-mc-decap", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("trill", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_TRILL_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("discard-non-tril", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("discard-tril", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("reflective-bridge", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, value);
        }
    }
    else if ((CLI_CLI_STR_EQUAL("fcoe", 1))
            && (sal_strlen("fcoe") == sal_strlen(argv[1])))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_FCOE_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_FCOE_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("rpf-check-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_RPF_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("replace-stag-cos", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_COS, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("replace-stag-tpid", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_TPID, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_STAG_TPID, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("replace-ctag-cos", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_CTAG_COS, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_CTAG_COS, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("replace-dscp", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_DSCP_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_REPLACE_DSCP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("l3pdu-arp-action", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("l3pdu-dhcp-action", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("tunnel-rpf-check", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("ptp", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PTP_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PTP_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("rpf-type", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_TYPE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_RPF_TYPE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("is-leaf", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_IS_LEAF, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_IS_LEAF, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("priority-tag", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PRIORITY_TAG_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PRIORITY_TAG_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("default-pcp", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_PCP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DEFAULT_PCP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("default-dei", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_DEI, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_DEFAULT_DEI, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("nvgre-mcast-no-decap", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("vxlan-mcast-no-decap", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("mac-ts", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TS_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_MAC_TS_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("error-check", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_ERROR_CHECK, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_ERROR_CHECK, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("scl-field-sel-id", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("raw-packet-type", 1))
    {
        if (CLI_CLI_STR_EQUAL("none", 2))
        {
            value = CTC_PORT_RAW_PKT_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("ethernet", 2))
        {
            value = CTC_PORT_RAW_PKT_ETHERNET;
        }
        else if (CLI_CLI_STR_EQUAL("ipv4", 2))
        {
            value = CTC_PORT_RAW_PKT_IPV4;
        }
        else if (CLI_CLI_STR_EQUAL("ipv6", 2))
        {
            value = CTC_PORT_RAW_PKT_IPV6;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_RAW_PKT_TYPE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_RAW_PKT_TYPE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("pvlan-type", 1))
    {
        port_restriction.mode = CTC_PORT_RESTRICTION_PVLAN;
        if (CLI_CLI_STR_EQUAL("none", 2))
        {
            port_restriction.type = CTC_PORT_PVLAN_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("promiscuous", 2))
        {
            port_restriction.type = CTC_PORT_PVLAN_PROMISCUOUS;
        }
        else if (CLI_CLI_STR_EQUAL("isolated", 2))
        {
            port_restriction.type = CTC_PORT_PVLAN_ISOLATED;
        }
        else if (CLI_CLI_STR_EQUAL("community", 2))
        {
            port_restriction.type = CTC_PORT_PVLAN_COMMUNITY;
        }

        index = 0;
        index = CTC_CLI_GET_ARGC_INDEX("isolation-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("isolated_id", port_restriction.isolated_id, argv[index + 1]);
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_restriction(g_api_lchip, gport, &port_restriction);
        }
        else
        {
            ret = ctc_port_set_restriction(gport, &port_restriction);
        }
    }
    else if (CLI_CLI_STR_EQUAL("qos-trust", 1))
    {
        if (CLI_CLI_STR_EQUAL("port", 2))
        {
            value = CTC_QOS_TRUST_PORT;
        }
        else if (CLI_CLI_STR_EQUAL("outer", 2))
        {
            value = CTC_QOS_TRUST_OUTER;
        }
        else if (CLI_CLI_STR_EQUAL("cos", 2))
        {
            value = CTC_QOS_TRUST_COS;
        }
        else if (CLI_CLI_STR_EQUAL("dscp", 2))
        {
            value = CTC_QOS_TRUST_DSCP;
        }
        else if (CLI_CLI_STR_EQUAL("ip-prec", 2))
        {
            value = CTC_QOS_TRUST_IP;
        }
        else if (CLI_CLI_STR_EQUAL("stag-cos", 2))
        {
            value = CTC_QOS_TRUST_STAG_COS;
        }
        else if (CLI_CLI_STR_EQUAL("ctag-cos", 2))
        {
            value = CTC_QOS_TRUST_CTAG_COS;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_QOS_POLICY, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_QOS_POLICY, value);
        }
    }

    else if (CLI_CLI_STR_EQUAL("auto-neg-mode", 1))
    {
        if (CLI_CLI_STR_EQUAL("1000base-X", 2))
        {
            value = CTC_PORT_AUTO_NEG_MODE_1000BASE_X;
        }
        else if (CLI_CLI_STR_EQUAL("sgmii-slaver", 2))
        {
            value = CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER;
        }
        else if (CLI_CLI_STR_EQUAL("sgmii-master", 2))
        {
            value = CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_AUTO_NEG_MODE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("auto-neg-fec", 1))
    {
        index = CTC_CLI_GET_ARGC_INDEX("none");
        if (index != 0xFF)
        {
            value = CTC_PORT_FEC_TYPE_NONE;
        }
        else
        {
            value = CTC_PORT_FEC_TYPE_NONE;
            index = CTC_CLI_GET_ARGC_INDEX("rs");
            if (index != 0xFF)
            {
                value |= (1 << CTC_PORT_FEC_TYPE_RS);
            }
            index = CTC_CLI_GET_ARGC_INDEX("base-r");
            if (index != 0xFF)
            {
                value |= (1 << CTC_PORT_FEC_TYPE_BASER);
            }
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_AUTO_NEG_FEC, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("linkscan-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_LINKSCAN_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_LINKSCAN_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("auto-neg", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_AUTO_NEG_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("aps-failover", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_APS_FAILOVER_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_APS_FAILOVER_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("linkagg-failover", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("link-intr", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("snooping-parser", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SNOOPING_PARSER, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SNOOPING_PARSER, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("flow-lkup-use-outer", 1))
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("aware-tunnel-info-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("nvgre-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_ENABLE, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_NVGRE_ENABLE, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("metadata-overwrite-port", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("metadata-overwrite-udf", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("src-mismatch-exception-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("station-move-priority", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_STATION_MOVE_PRIORITY, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_STATION_MOVE_PRIORITY, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("station-move-action", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_STATION_MOVE_ACTION, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_STATION_MOVE_ACTION, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("efd-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_EFD_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_EFD_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("add-default-vlan-dis", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("logic-port", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_LOGIC_PORT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_LOGIC_PORT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("gport", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_GPORT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_GPORT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("aps-select-group", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("aps-select-working", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_WORKING, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_APS_SELECT_WORKING, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("fcmap", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_FCMAP, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_FCMAP, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("unidirection", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_UNIDIR_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_UNIDIR_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("mac-ipg", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_MAC_TX_IPG, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("oversub-priority", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_OVERSUB_PRI, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_OVERSUB_PRI, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("cut-through-en", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_CUT_THROUGH_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_CUT_THROUGH_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("loop-with-src-port", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_LOOP_WITH_SRC_PORT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_LOOP_WITH_SRC_PORT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("check-crc", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_CHK_CRC_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_CHK_CRC_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("strip-crc", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_STRIP_CRC_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("append-crc", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_APPEND_CRC_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("append-tod", 1))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_TOD_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_APPEND_TOD_EN, value);
        }
    }
    else if(CLI_CLI_STR_EQUAL("xpipe", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_XPIPE_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_XPIPE_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("phy-init", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_INIT, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PHY_INIT, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("phy-en", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_EN, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PHY_EN, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("phy-duplex", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_DUPLEX, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PHY_DUPLEX, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("phy-medium", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_MEDIUM, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PHY_MEDIUM, value);
        }
    }
    else if (CLI_CLI_STR_EQUAL("phy-loopback", 1))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_LOOPBACK, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, CTC_PORT_PROP_PHY_LOOPBACK, value);
        }
    }
    else
    {
        uint32 prop_type = 0;
        if (CLI_CLI_STR_EQUAL("mux-type", 1))
        {
            prop_type = CTC_PORT_PROP_MUX_TYPE;
        }
        else
        {
            return CLI_SUCCESS;
        }
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_set_property(g_api_lchip, gport, prop_type, value);
        }
        else
        {
            ret = ctc_port_set_property(gport, prop_type, value);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#if defined(SDK_IN_USERMODE)
CTC_CLI(ctc_cli_port_set_property_array,
        ctc_cli_port_set_property_array_cmd,
        "port GPHYPORT_ID array property ((route-en value VALUE)|) ((pkt-tag-high-priority value VALUE)|) ((hw-learn value VALUE)|) ((ipv6-lookup value VALUE)|) ((ipv4-lookup value VALUE)|)\
        ((use-default-lookup value VALUE)|) ((ipv6-to-mac-key value VALUE)|) ((ipv4-to-mac-key value VALUE)|) ((trill-mc-decap value VALUE)|) ((trill value VALUE)|)\
        ((discard-non-tril value VALUE)|) ((discard-tril value VALUE)|) ((reflective-bridge value VALUE)|) ((fcoe value VALUE)|) ((rpf-check-en value VALUE)|) \
        ((replace-stag-cos value VALUE)|) ((replace-stag-tpid value VALUE)|) ((replace-ctag-cos value VALUE)|) ((replace-dscp value VALUE)|) ((l3pdu-arp-action value VALUE)|)\
        ((l3pdu-dhcp-action value VALUE)|) ((tunnel-rpf-check value VALUE)|) ((ptp value VALUE)|) ((rpf-type value VALUE)|) ((is-leaf value VALUE)|) ((priority-tag value VALUE)|)\
        ((default-pcp value VALUE)|) ((default-dei value VALUE)|) ((nvgre-mcast-no-decap value VALUE)|) ((vxlan-mcast-no-decap value VALUE)|) ((auto-neg value VALUE)|)\
        ((linkscan-en value VALUE)|) ((aps-failover value VALUE)|) ((linkagg-failover value VALUE)|) ((link-intr value VALUE)|) ((mac-ts value VALUE)|) ((scl-field-sel-id value VALUE)|)\
        ((snooping-parser value VALUE)|) ((flow-lkup-use-outer value VALUE)|) ((aware-tunnel-info-en value VALUE)|) ((nvgre-en value VALUE)|) ((metadata-overwrite-port value VALUE)|)\
        ((metadata-overwrite-udf value VALUE)|) ((src-mismatch-exception-en value VALUE)|) ((station-move-priority value VALUE)|) ((station-move-action value VALUE)|) ((efd-en value VALUE)|)\
        ((add-default-vlan-dis value VALUE)|) ((aps-select-group value VALUE)|) ((aps-select-working value VALUE)|) ((error-check value VALUE)|) ((fec-en value VALUE)|) ((fcmap value VALUE)|)\
        ((unidirection value VALUE)|) ((mac-ipg value VALUE)|) ((oversub-priority value VALUE)|) ((cut-through-en value VALUE)|) ((loop-with-src-port value VALUE)|) \
        ((logic-port value VALUE)|) ((gport value VALUE)|) \
        ((raw-packet-type ( none | ethernet | ipv4 | ipv6 ))|) \
        ((qos-trust (port | outer | cos | dscp | ip-prec | stag-cos | ctag-cos))|) \
        ((auto-neg-mode (1000base-X| sgmii-slaver | sgmii-master))|) \
        ((check-crc value VALUE)|) ((strip-crc value VALUE)|) ((append-crc value VALUE)|) ((append-tod value VALUE)|) \
        ((auto-neg-fec {rs|base-r|none})|)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "array property",
        "Property",
        "Route enable",
        "Route enable value",
        "Value",
        "Packet tag take precedence over all",
        "Packet tag priority value",
        "Value",
        "Hardware learning",
        "Hardware learning value",
        "Value",
        "Scl ipv6 lookup disable",
        "Scl ipv6 lookup value",
        "Value",
        "Scl ipv4 lookup disable",
        "Scl ipv4 lookup value",
        "Value",
        "Scl default lookup",
        "Scl default lookup value",
        "Value",
        "Scl MAC key is used for lookup instead of ipv6 key",
        "MAC key is used for lookup value",
        "Value",
        "Scl MAC key is used for lookup instead of ipv4 key",
        "MAC key is used for lookup value",
        "Value",
        "Set trill mcast decap enable",
        "Trill mcast decap enable value",
        "Value",
        "Set trill enable",
        "Trill enable value",
        "Value",
        "Set NON-TRILL packet discard",
        "NON-TRILL packet discard value",
        "Value",
        "Set TRILL packet discard",
        "TRILL packet discard value",
        "Value",
        "Bridge to the same port enable",
        "Bridge to the same port enable value",
        "Value",
        "FCoE enable",
        "FCoE enable value",
        "Value",
        "RPF check enable",
        "RPF check enable value",
        "Value",
        "The STAG COS field is replaced by the classified COS result",
        "Stag cos field replace value",
        "Value",
        "The STAG TPID field is replaced by the classified TPID result",
        "Stag tpid field replace value",
        "Value",
        "The CTAG COS field is replaced by the classified COS result",
        "Ctag cos field replace value",
        "Value",
        "The Dscp field is replace by qos mapping",
        "Dscp field replace value",
        "Value",
        "ARP packet processing type",
        "ARP packet processing type value",
        "Value",
        "DHCP packet processing type",
        "DHCP packet processing type",
        "Value",
        "RPF check for outer tunnel IP header is enabled",
        "RPF check outer tunnel IP header enable value",
        "Value",
        "Enable ptp",
        "Ptp enable value",
        "Value",
        "Rpf type",
        "Rpf type value",
        "Value",
        "This port connect with a leaf node",
        "Leaf node enable value",
        "Value",
        "Priority tagged packet will be sent out",
        "Priority tagged enable value",
        "Value",
        "Default cos of vlan tag",
        "Vlan tag default cos value",
        "Value",
        "Default dei of vlan tag",
        "Vlan tag default dei value",
        "Value",
        "NvGRE Mcast packet will do not decapsulate",
        "NvGRE Mcast packet decapsulate value",
        "Value",
        "VxLAN Mcast packet will do not decapsulate",
        "VxLAN Mcast packet decapsulate value",
        "Value",
        "Auto negotiation enable",
        "Auto negotiation enable value",
        "Value",
        "Port support linkscan function",
        "Linkscan enable value",
        "Value",
        "Aps failover enable",
        "Aps failover enable",
        "Value",
        "Linkagg failover enable",
        "Linkagg failover enable",
        "Value",
        "Pcs link change interrupt enable",
        "Link interrupt enable value",
        "Value",
        "Timestamp",
        "Timestamp value",
        "Value",
        "Scl key field select id <0-3>",
        "scl id value",
        "Value",
        "Snooping parser",
        "Snooping parser enable value",
        "Value",
        "If set,indicate tunnel packet will use outer header to do ACL/IPFIX flow lookup",
        "flow lookup use outer header enable",
        "Value",
        "Aware tunnel info en",
        "Aware tunnel info enable value",
        "Value",
        "Nvgre enable",
        "Nvgre enable value",
        "Value",
        "Metadata overwrite port",
        "Metadata overwrite port enable value",
        "Value",
        "Metadata overwrite udf",
        "Metadata overwrite udf enable value",
        "Value",
        "Source mismatch exception en",
        "Source mismatch exception enable value",
        "Value",
        "Station move priority",
        "Station move priority value",
        "Value",
        "Station move action, 0:fwd, 1:discard, 2:discard and to cpu",
        "Station move action value",
        "Value",
        "Efd enable",
        "Efd enable value",
        "Value",
        "Add default vlan disable",
        "Add default vlan disable value",
        "Value",
        "Aps select group id, 0xFFFFFFFF means disable aps select",
        "Aps select group id value",
        "Value",
        "Aps select path, 0 means protecting path, else working path",
        "Work path select value",
        "Value",
        "Mac error check",
        "Mac error check enable value",
        "Value",
        "Forword error correction enable, 0:no-fec, 1:RS fec, 2:base-R fec",
        "Forword error value",
        "Value",
        "Fcmap,0x0EFC00-0x0EFCFF, default is 0x0EFC00",
        "Fcmap value",
        "Value",
        "Unidirection",
        "Unidirection enable value"
        "Value",
        "Mac Ipg index",
        "Mac Ipg index value",
        "Value",
        "Oversubscription priority",
        "Oversubscription priority value",
        "Value",
        "Cut through Enable",
        "Cut through Enable value"
        "Value",
        "Loop with source port",
        "Loop with source port enable value"
        "Value",
        "Logic port",
        "Property Value",
        "Value",
        "Gport",
        "Property Value",
        "Value",
        "Packet type",
        "Raw packet type disable",
        "Port parser ethernet raw packet",
        "Port only parser ipv4 raw packet",
        "Port only parser ipv6 raw packet",
        "Qos trust policy",
        "Classifies ingress packets with the port default CoS value",
        "Classifies ingress packets with the outer priority value",
        "Classifies ingress packets with the packet CoS values",
        "Classifies ingress packets with the packet DSCP values",
        "Classifies ingress packets with the packet IP-Precedence values",
        "Classifies ingress packets with the packet Stag CoS values",
        "Classifies ingress packets with the packet Ctag CoS values",
        "Auto neg mode",
        "1000Base-X",
        "Sgmii-slaver",
        "Sgmii-master",
        "Check CRC",
        "Property value",
        "Value",
        "Strip CRC",
        "Property value",
        "Value",
        "Append CRC",
        "Property value",
        "Value",
        "Append TOD",
        "Property value",
        "Value",
        "Auto neg FEC",
        "RS FEC",
        "Base-R FEC",
        "No FEC")
{
    int32 ret = 0;
    uint16 gport;
    uint8 index = 0xFF;
    ctc_port_restriction_t port_restriction;
    ctc_property_array_t port_prop[MAX_CTC_PORT_PROP_NUM];
    uint16 array_num = 0;

    sal_memset(&port_restriction, 0, sizeof(ctc_port_restriction_t));
    sal_memset(port_prop, 0, sizeof(ctc_property_array_t) * MAX_CTC_PORT_PROP_NUM);

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (CTC_CLI_GET_ARGC_INDEX("route-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_ROUTE_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("pkt-tag-high-priority") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("hw-learn") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_HW_LEARN_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("fec-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_FEC_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("ipv6-lookup") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("ipv4-lookup") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("use-default-lookup") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("ipv6-to-mac-key") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("ipv4-to-mac-key")!= 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("trill-mc-decap") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_TRILL_MCAST_DECAP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("trill") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_TRILL_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("discard-non-tril") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_DISCARD_NON_TRIL_PKT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("discard-tril") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_DISCARD_TRIL_PKT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("reflective-bridge") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("fcoe") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_FCOE_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("rpf-check-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_RPF_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("replace-stag-cos") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_REPLACE_STAG_COS;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("replace-stag-tpid") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_REPLACE_STAG_TPID;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("replace-ctag-cos") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_REPLACE_CTAG_COS;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("replace-dscp") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_REPLACE_DSCP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("l3pdu-arp-action") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_L3PDU_ARP_ACTION;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("l3pdu-dhcp-action") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_L3PDU_DHCP_ACTION;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("tunnel-rpf-check") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_TUNNEL_RPF_CHECK;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("ptp") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_PTP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("rpf-type") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_RPF_TYPE;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("is-leaf") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_IS_LEAF;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("priority-tag") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_PRIORITY_TAG_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("default-pcp") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_DEFAULT_PCP;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("default-dei") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_DEFAULT_DEI;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("nvgre-mcast-no-decap") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("vxlan-mcast-no-decap") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("mac-ts") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_MAC_TS_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("error-check") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_ERROR_CHECK;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("scl-field-sel-id") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("raw-packet-type") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_TRILL_MCAST_DECAP_EN;
        index = CTC_CLI_GET_ARGC_INDEX("raw-packet-type");
        if (CLI_CLI_STR_EQUAL("none", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_RAW_PKT_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("ethernet", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_RAW_PKT_ETHERNET;
        }
        else if (CLI_CLI_STR_EQUAL("ipv4", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_RAW_PKT_IPV4;
        }
        else if (CLI_CLI_STR_EQUAL("ipv6", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_RAW_PKT_IPV6;
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("qos-trust") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_QOS_POLICY;
        index = CTC_CLI_GET_ARGC_INDEX("qos-trust");
        if (CLI_CLI_STR_EQUAL("port", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_PORT;
        }
        else if (CLI_CLI_STR_EQUAL("outer", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_OUTER;
        }
        else if (CLI_CLI_STR_EQUAL("cos", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_COS;
        }
        else if (CLI_CLI_STR_EQUAL("dscp", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_DSCP;
        }
        else if (CLI_CLI_STR_EQUAL("ip-prec", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_IP;
        }
        else if (CLI_CLI_STR_EQUAL("stag-cos", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_STAG_COS;
        }
        else if (CLI_CLI_STR_EQUAL("ctag-cos", index + 1))
        {
            port_prop[array_num].data = CTC_QOS_TRUST_CTAG_COS;
        }
        array_num++;
    }

    if (CTC_CLI_GET_ARGC_INDEX("auto-neg-mode") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_AUTO_NEG_MODE;
        index = CTC_CLI_GET_ARGC_INDEX("auto-neg-mode");
        if (CLI_CLI_STR_EQUAL("1000base-X", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_AUTO_NEG_MODE_1000BASE_X;
        }
        else if (CLI_CLI_STR_EQUAL("sgmii-slaver", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER;
        }
        else if (CLI_CLI_STR_EQUAL("sgmii-master", index + 1))
        {
            port_prop[array_num].data = CTC_PORT_AUTO_NEG_MODE_SGMII_MASTER;
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("auto-neg-fec") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_AUTO_NEG_FEC;
        index = CTC_CLI_GET_ARGC_INDEX("none");
        if (index != 0xFF)
        {
            port_prop[array_num].data = CTC_PORT_FEC_TYPE_NONE;
        }
        else
        {
            index = CTC_CLI_GET_ARGC_INDEX("rs");
            if (index != 0xFF)
            {
                port_prop[array_num].data |= (1 << CTC_PORT_FEC_TYPE_RS);
            }
            index = CTC_CLI_GET_ARGC_INDEX("base-r");
            if (index != 0xFF)
            {
                port_prop[array_num].data |= (1 << CTC_PORT_FEC_TYPE_BASER);
            }
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("linkscan-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_LINKSCAN_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("auto-neg") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_AUTO_NEG_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("aps-failover") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_APS_FAILOVER_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("linkagg-failover") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_LINKAGG_FAILOVER_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("link-intr") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_LINK_INTRRUPT_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("snooping-parser") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SNOOPING_PARSER;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("flow-lkup-use-outer") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("nvgre-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_NVGRE_ENABLE;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("metadata-overwrite-port") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_METADATA_OVERWRITE_PORT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("metadata-overwrite-udf") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_METADATA_OVERWRITE_UDF;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("src-mismatch-exception-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("station-move-priority") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_STATION_MOVE_PRIORITY;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("station-move-action") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_STATION_MOVE_ACTION;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("efd-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_EFD_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("add-default-vlan-dis") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("logic-port") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_LOGIC_PORT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("gport") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_GPORT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("aps-select-group") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_APS_SELECT_GRP_ID;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("aps-select-working") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_APS_SELECT_WORKING;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("fcmap") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_FCMAP;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("unidirection") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_UNIDIR_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("mac-ipg") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_MAC_TX_IPG;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("oversub-priority") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_OVERSUB_PRI;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("cut-through-en") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_CUT_THROUGH_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("loop-with-src-port") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_LOOP_WITH_SRC_PORT;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("check-crc") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_CHK_CRC_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("strip-crc") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_STRIP_CRC_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("append-crc") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_APPEND_CRC_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }
    if (CTC_CLI_GET_ARGC_INDEX("append-tod") != 0xFF)
    {
        port_prop[array_num].property = CTC_PORT_PROP_APPEND_TOD_EN;
        index = CTC_CLI_GET_ARGC_INDEX("value");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("Property_value", port_prop[array_num].data, argv[index + 1]);
        }
        array_num++;
    }

    if (!array_num)
    {
        return CLI_SUCCESS;
    }
    if(g_ctcs_api_en)
    {
         ret = ctcs_port_set_property_array(g_api_lchip, gport, port_prop, &array_num);
    }
    else
    {
        ret = ctc_port_set_property_array(gport, port_prop, &array_num);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
#endif

/*    "Mac-da-low-8bits value",
    "Mac-da-prefix-type value",
    "Low 8 bits for sa port MAC",
    "Sa port MAC high 40 bits register select",*/
CTC_CLI(ctc_cli_port_set_system_port_mac_prefix,
        ctc_cli_port_set_system_port_mac_prefix_cmd,
        "system set port-mac { prefix0 MAC | prefix1 MAC }",
        "System attribution",
        "Set",
        "Port MAC",
        "Port Prefix MAC 0",
        CTC_CLI_MAC_FORMAT,
        "Port Prefix MAC 1",
        "MAC address in HHHH.HHHH.HHHH format, high 32bits must same as port mac prefix 0")
{
    int32 ret   = 0;
    uint8 index = 0;
    ctc_port_mac_prefix_t prefix_mac;
    mac_addr_t          mac_addr;

    sal_memset(&prefix_mac, 0, sizeof(ctc_port_mac_prefix_t));

    index = CTC_CLI_GET_ARGC_INDEX("prefix0");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(prefix_mac.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
        sal_memset(&mac_addr, 0, sizeof(mac_addr_t));
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac_addr, argv[index + 1]);
        sal_memcpy(prefix_mac.port_mac[0], mac_addr, sizeof(mac_addr_t));
    }

    index = CTC_CLI_GET_ARGC_INDEX("prefix1");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(prefix_mac.prefix_type, CTC_PORT_MAC_PREFIX_MAC_1);
        sal_memset(&mac_addr, 0, sizeof(mac_addr_t));
        CTC_CLI_GET_MAC_ADDRESS("mac address", mac_addr, argv[index + 1]);

        sal_memcpy(prefix_mac.port_mac[1], mac_addr, sizeof(mac_addr_t));
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_port_mac_prefix(g_api_lchip, &prefix_mac);
    }
    else
    {
        ret = ctc_port_set_port_mac_prefix(&prefix_mac);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_port_mac_postfix,
        ctc_cli_port_set_port_mac_postfix_cmd,
        "port GPHYPORT_ID (prefix-index INDEX low-8bit-mac MAC|port-mac MAC)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Prefix port mac index",
        "Value <0-1>",
        "Port MAC low 8bit",
        "<0-255>",
        "Port MAC",
        CTC_CLI_MAC_FORMAT)
{
    int32 ret = 0;
    uint16 gport = 0;
    uint8 index = 0;
    uint8 mac   = 0;
    ctc_port_mac_postfix_t post_mac;

    sal_memset(&post_mac, 0, sizeof(ctc_port_mac_postfix_t));

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("port-mac");

    if (0xFF != index)
    {
        CTC_SET_FLAG(post_mac.prefix_type, CTC_PORT_MAC_PREFIX_48BIT);
        CTC_CLI_GET_MAC_ADDRESS("port-mac", post_mac.port_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("prefix-index");

    if (0xFF != index)
    {

        CTC_CLI_GET_UINT8("index", index, argv[index+1]);
        if (0 == index)
        {
            CTC_SET_FLAG(post_mac.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
        }
        else if (1 == index)
        {
            CTC_SET_FLAG(post_mac.prefix_type, CTC_PORT_MAC_PREFIX_MAC_1);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("low-8bit-mac");

    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mac", mac, argv[index + 1]);
        post_mac.low_8bits_mac = mac;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_port_mac_postfix(g_api_lchip, gport, &post_mac);
    }
    else
    {
        ret = ctc_port_set_port_mac_postfix(gport, &post_mac);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_show_port_property,
        ctc_cli_port_show_port_property_cmd,
        "show port GPHYPORT_ID property (all | route-en | pkt-tag-high-priority | hw-learn | \
        ipv6-lookup | ipv4-lookup | use-default-lookup | ipv6-to-mac-key | ipv4-to-mac-key | trill-mc-decap | trill | discard-non-tril | discard-tril | \
        reflective-bridge | fcoe | rpf-check-en | replace-stag-cos | replace-stag-tpid |replace-ctag-cos | replace-dscp | \
        l3pdu-arp-action | l3pdu-dhcp-action | tunnel-rpf-check | ptp | rpf-type | is-leaf | priority-tag | default-pcp | \
        default-dei | nvgre-mcast-no-decap | vxlan-mcast-no-decap | raw-packet-type | pvlan-type | qos-trust | auto-neg | cl73-ability |\
        link-intr | mac-ts | linkscan-en | aps-failover | linkagg-failover | scl-field-sel-id | snooping-parser |flow-lkup-use-outer | aware-tunnel-info-en | nvgre-en | \
        metadata-overwrite-port | metadata-overwrite-udf | src-mismatch-exception-en |station-move-priority | station-move-action | efd-en|add-default-vlan-dis | \
        logic-port | gport | aps-select-group | aps-select-working | error-check | auto-neg-mode | fec-en | fcmap | unidirection | mac-ipg | oversub-priority | link-up|cut-through-en | loop-with-src-port | \
        check-crc | strip-crc | append-crc | append-tod | learn-dis-mode | force-mpls-en | force-tunnel-en | \
        auto-neg-fec | xpipe|mux-type| phy-init | phy-en| phy-duplex | phy-medium | phy-loopback)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property",
        "Show all property",
        "Route enable",
        "Packet tag take precedence over all",
        "Hardware learning",
        "Scl ipv6 lookup disable",
        "Scl ipv4 lookup disable",
        "Scl default lookup",
        "Scl MAC key is used for lookup instead of ipv6 key",
        "Scl MAC key is used for lookup instead of ipv4 key",
        "Set trill mcast decap enable",
        "Trill enable",
        "NON-TRILL packet discard",
        "TRILL packet discard",
        "Bridge to the same port enable",
        "FCoE enable",
        "RPF check enable",
        "The STAG COS field is replaced by the classified COS result",
        "The STAG TPID field is replaced by the classified TPID result",
        "The CTAG COS field is replaced by the classified COS result",
        "The Dscp field is replaced by the qos mapping result",
        "ARP packet processing type",
        "DHCP packet processing type",
        "RPF check for outer tunnel IP header is enabled",
        "For untagged PTP packet and routedPort",
        "Strict or loose",
        "This port connect with a leaf node",
        "Priority tagged packet will be sent out",
        "Default cos of vlantag",
        "Default dei of vlantag",
        "NvGRE Mcast pacekt do not decapsulate",
        "VxLAN Mcast pacekt do not decapsulate",
        "Raw packet type",
        "Pvlan port type",
        "Qos trust policy",
        "auto neg",
        "cl73 ability",
        "link interrupt",
        "Timestamp",
        "Port linkscan function enable",
        "Aps failover enable",
        "Linkagg failover enable",
        "Scl key field select id",
        "Snooping parser",
        "If set,indicate tunnel packet will use outer header to do ACL/IPFIX flow lookup",
        "Aware tunnel info en",
        "nvgre enable",
        "Metadata overwrite port",
        "Metadata overwrite udf",
        "Src mismatch exception en",
        "Station move priority",
        "Station move action, 0:fwd, 1:discard, 2:discard and to cpu",
        "Efd enable",
        "Add default vlan disable",
        "Logic port",
        "Gport",
        "Aps select group id, 0xFFFFFFFF means disable aps select",
        "Aps select path, 0 means protecting path, else working path",
        "Mac error check",
        "Auto neg mode",
        "Forword error correction status",
        "Fcmap,0x0EFC00-0x0EFCFF, default is 0x0EFC00",
        "Unidirection",
        "Mac ipg index",
        "Oversubscription priority",
        "External PHY link status",
        "Cut through enable",
        "Loop with Source port",
        "Check CRC",
        "Strip CRC",
        "Append CRC",
        "Append TOD",
        "Learn disable mode",
        "Force mpls packet decap",
        "Force tunnel packet decap",
        "Auto neg FEC",
        "X-pipe feature",
        "Mux-type",
        "Phy init",
        "Phy enable",
        "Phy duplex",
        "Phy medium",
        "Phy loopback")
{
    int32 ret = 0;
    uint16 gport;
    uint32 value = FALSE;
    char* str = NULL;
    uint8 index = 0xFF;
    uint8 show_all = 0;
    ctc_port_restriction_t port_restrict;
    char port_str[CTC_PORT_PVLAN_COMMUNITY + 1][32] = {{"none"}, {"promiscuous"}, {"isolated"}, {"community"}};

    sal_memset(&port_restrict, 0, sizeof(ctc_port_restriction_t));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        show_all = 1;
    }

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    ctc_cli_out("Show GPort                        :  0x%04x\n", gport);
    ctc_cli_out("-------------------------------------------\n");


    index = CTC_CLI_GET_ARGC_INDEX("route-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ROUTE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ROUTE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Route-enable", value);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("cut-through-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CUT_THROUGH_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CUT_THROUGH_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Cut through enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("pkt-tag-high-priority");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Pkt-tag-high-priority", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("hw-learn");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_HW_LEARN_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_HW_LEARN_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Hw-learn", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("fec-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FEC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FEC_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:", "Fec-en");
            if (CTC_PORT_FEC_TYPE_NONE == value)
            {
                ctc_cli_out("  %s\n", "NONE");
            }
            else if (CTC_PORT_FEC_TYPE_RS == value)
            {
                ctc_cli_out("  %s\n", "RS");
            }
            else if (CTC_PORT_FEC_TYPE_BASER == value)
            {
                ctc_cli_out("  %s\n", "BASER");
            }
            ctc_cli_out("\n");
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-lookup");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Ipv6-lookup", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-lookup");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Ipv4-lookup", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-default-lookup");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Use-default-lookup", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-to-mac-key");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Ipv6-to-mac-key", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-to-mac-key");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Ipv4-to-mac-key", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("trill-mc-decap");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_TRILL_MCAST_DECAP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Trill mcast decap", value);
        }
    }
    if (index == 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("trill");
        if ((index != 0xFF) || show_all)
        {
            if (g_ctcs_api_en)
            {
                ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TRILL_EN, &value);
            }
            else
            {
                ret = ctc_port_get_property(gport, CTC_PORT_PROP_TRILL_EN, &value);
            }
            if (ret >= 0)
            {
                ctc_cli_out("%-34s:  %u\n", "Trill", value);
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-non-tril");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DISCARD_NON_TRIL_PKT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Discard-non-tril", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tril");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DISCARD_TRIL_PKT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Discard-tril", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("reflective-bridge");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Reflective-bridge", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (((index != 0xFF) && (sal_strlen("fcoe") == sal_strlen(argv[index]))) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FCOE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FCOE_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Fcoe", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("rpf-check-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RPF_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Rpf-check-en", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-stag-cos");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_COS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_STAG_COS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Replace-stag-cos", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-stag-tpid");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_STAG_TPID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_STAG_TPID, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Replace-stag-tpid", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-ctag-cos");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_CTAG_COS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_CTAG_COS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Replace-ctag-cos", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-dscp");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_REPLACE_DSCP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_REPLACE_DSCP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Replace-dscp", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3pdu-arp-action");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "L3pdu-arp-action", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3pdu-dhcp-action");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_L3PDU_DHCP_ACTION, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "L3pdu-dhcp-action", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-rpf-check");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_TUNNEL_RPF_CHECK, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Tunnel-rpf-check", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PTP_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PTP_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Ptp", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("rpf-type");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RPF_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RPF_TYPE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Rpf-type", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_IS_LEAF, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_IS_LEAF, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Is-leaf", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-tag");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PRIORITY_TAG_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PRIORITY_TAG_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Priority-tag", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-pcp");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_PCP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DEFAULT_PCP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Default-pcp", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-dei");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_DEFAULT_DEI, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_DEFAULT_DEI, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Default-dei", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("nvgre-mcast-no-decap");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Nvgre-mcast-no-decap", value);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-mcast-no-decap");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Vxlan-mcast-no-decap", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-field-sel-id");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Scl-field-sel-id", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("raw-packet-type");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_RAW_PKT_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_RAW_PKT_TYPE, &value);
        }

        switch (value)
        {
            case CTC_PORT_RAW_PKT_ETHERNET:
                str = "ethernet";
                break;

            case CTC_PORT_RAW_PKT_IPV4:
                str = "ipv4";
                break;

            case CTC_PORT_RAW_PKT_IPV6:
                str = "ipv6";
                break;

            case CTC_PORT_RAW_PKT_NONE:
                str = "none";
                break;
        }

        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %s\n", "Raw-packet-type", str);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("pvlan-type");
    if ((index != 0xFF) || show_all)
    {
        port_restrict.mode = CTC_PORT_RESTRICTION_PVLAN;
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_restriction(g_api_lchip, gport, &port_restrict);
        }
        else
        {
            ret = ctc_port_get_restriction(gport, &port_restrict);
        }
        if ((ret >= 0) && (port_restrict.type == CTC_PORT_PVLAN_COMMUNITY))
        {
            ctc_cli_out("%-34s:  %s\n", "Pvlan port type", port_str[port_restrict.type]);
            ctc_cli_out("%-34s:  %u\n", "Pvlan port isolated id", port_restrict.isolated_id);
        }
        else if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %s\n", "Pvlan port type", port_str[port_restrict.type]);
            ctc_cli_out("This pvlan port can not have isolated id!\n");
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-trust");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_QOS_POLICY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_QOS_POLICY, &value);
        }

        switch (value)
        {
            case CTC_QOS_TRUST_PORT:
                str = "port";
                break;

            case CTC_QOS_TRUST_OUTER:
                str = "outer";
                break;

            case CTC_QOS_TRUST_COS:
                str = "cos";
                break;

            case CTC_QOS_TRUST_DSCP:
                str = "dscp";
                break;

            case CTC_QOS_TRUST_IP:
                str = "ip-prec";
                break;

            case CTC_QOS_TRUST_STAG_COS:
                str = "stag-cos";
                break;

            case CTC_QOS_TRUST_CTAG_COS:
                str = "ctag-cos";
                break;

            default:
                str = "unexpect!!!!!";
                break;

        }

        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %s\n", "Qos-trust", str);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("auto-neg");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port auto-neg", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("cl73-ability");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CL73_ABILITY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CL73_ABILITY, &value);
        }
        if (ret >= 0)
        {
            if(value == 0)
            {
                ctc_cli_out("Port cl73 ability is none\n");
            }
            else
            {
                ctc_cli_out("Port cl73 ability is:\n");
                if(value & CTC_PORT_CL73_10GBASE_KR)
                    ctc_cli_out("%30s\n", "10GBASE-KR");
                if(value & CTC_PORT_CL73_40GBASE_KR4)
                    ctc_cli_out("%30s\n", "40GBASE-KR4");
                if(value & CTC_PORT_CL73_40GBASE_CR4)
                    ctc_cli_out("%30s\n", "40GBASE-CR4");
                if(value & CTC_PORT_CL73_100GBASE_KR4)
                    ctc_cli_out("%30s\n", "100GBASE-KR4");
                if(value & CTC_PORT_CL73_100GBASE_CR4)
                    ctc_cli_out("%30s\n", "100GBASE-CR4");
                if(value & CTC_PORT_CL73_25GBASE_KRS)
                    ctc_cli_out("%30s\n", "25GBASE-K/CR-S");
                if(value & CTC_PORT_CL73_25GBASE_KR)
                    ctc_cli_out("%30s\n", "25GBASE-K/CR");
                if(value & CTC_PORT_CL73_25G_RS_FEC_REQUESTED)
                    ctc_cli_out("%30s\n", "25G-RS-FEC-REQ");
                if(value & CTC_PORT_CL73_25G_BASER_FEC_REQUESTED)
                    ctc_cli_out("%30s\n", "25G-BASER-FEC-REQ");
                if(value & CTC_PORT_CL73_FEC_ABILITY)
                    ctc_cli_out("%30s\n", "FEC-ABILITY");
                if(value & CTC_PORT_CL73_FEC_REQUESTED)
                    ctc_cli_out("%30s\n", "FEC-REQ");
                if(value & CTC_PORT_CSTM_25GBASE_KR1)
                    ctc_cli_out("%30s\n", "25GBASE-KR1");
                if(value & CTC_PORT_CSTM_25GBASE_CR1)
                    ctc_cli_out("%30s\n", "25GBASE-CR1");
                if(value & CTC_PORT_CSTM_50GBASE_KR2)
                    ctc_cli_out("%30s\n", "50GBASE-KR2");
                if(value & CTC_PORT_CSTM_50GBASE_CR2)
                    ctc_cli_out("%30s\n", "50GBASE-CR2");
                if(value & CTC_PORT_CSTM_RS_FEC_ABILITY)
                    ctc_cli_out("%30s\n", "CL91-ABILITY");
                if(value & CTC_PORT_CSTM_BASER_FEC_ABILITY)
                    ctc_cli_out("%30s\n", "CL74-ABILITY");
                if(value & CTC_PORT_CSTM_RS_FEC_REQUESTED)
                    ctc_cli_out("%30s\n", "CL91-REQ");
                if(value & CTC_PORT_CSTM_BASER_FEC_REQUESTED)
                    ctc_cli_out("%30s\n", "CL74-REQ");
                if(value & (CTC_PORT_CSTM_25GBASE_KR1|CTC_PORT_CSTM_25GBASE_CR1|CTC_PORT_CSTM_50GBASE_KR2|CTC_PORT_CSTM_50GBASE_CR2
                  |CTC_PORT_CSTM_RS_FEC_ABILITY|CTC_PORT_CSTM_BASER_FEC_ABILITY|CTC_PORT_CSTM_RS_FEC_REQUESTED|CTC_PORT_CSTM_BASER_FEC_REQUESTED))
                {
                    ctc_cli_out("%30s\n", "Next page is enable");
                }
                else
                {
                    ctc_cli_out("%30s\n", "No next page");
                }
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("link-intr");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINK_INTRRUPT_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port link change interrupt enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-ts");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TS_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_MAC_TS_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port mac time stamp", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkscan-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINKSCAN_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINKSCAN_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port linkscan enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-failover");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_FAILOVER_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_FAILOVER_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port aps failover enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkagg-failover");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINKAGG_FAILOVER_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Port linkagg failover enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("snooping-parser");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SNOOPING_PARSER, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SNOOPING_PARSER, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Snooping parser", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("flow-lkup-use-outer");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Flow lkup use outer", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Aware tunnel info enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("nvgre-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_NVGRE_ENABLE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_NVGRE_ENABLE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Nvgre enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata-overwrite-port");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_PORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Metadata overwrite port", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata-overwrite-udf");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Metadata overwrite udf", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-mismatch-exception-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Src mismatch exception en", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("station-move-priority");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_STATION_MOVE_PRIORITY, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_STATION_MOVE_PRIORITY, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Station move priority", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("station-move-action");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_STATION_MOVE_ACTION, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_STATION_MOVE_ACTION, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Station move actioin", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("efd-en");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_EFD_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_EFD_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Efd en", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("add-default-vlan-dis");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Add default vlan disable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LOGIC_PORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LOGIC_PORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  0x%04x\n", "Logic port", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_GPORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_GPORT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  0x%04x\n", "Gport", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select-group");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_SELECT_GRP_ID, &value);
        }
        if (ret >= 0)
        {
            if (0xFFFFFFFF == value)
            {
                ctc_cli_out("%-34s:  %s\n", "Aps select group", "disable");
            }
            else
            {
                ctc_cli_out("%-34s:  %u\n", "Aps select group", value);
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select-working");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APS_SELECT_WORKING, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APS_SELECT_WORKING, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Aps select working path", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("error-check");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_ERROR_CHECK, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_ERROR_CHECK, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Error check enable", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("auto-neg-mode");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_MODE, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Auto neg mode", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("fcmap");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FCMAP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FCMAP,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  0x0EFC%02X\n", "Fcmap", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("unidirection");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_UNIDIR_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_UNIDIR_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Unidirection", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-ipg");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_MAC_TX_IPG,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Mac tx ipg size", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("oversub-priority");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_OVERSUB_PRI, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_OVERSUB_PRI,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Oversub priority", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("link-up");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LINK_UP, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LINK_UP, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Link-up", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("loop-with-src-port");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LOOP_WITH_SRC_PORT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LOOP_WITH_SRC_PORT,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Loop with source port", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("check-crc");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CHK_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CHK_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Check CRC", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("strip-crc");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_STRIP_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_STRIP_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Strip CRC", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("append-crc");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_CRC_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APPEND_CRC_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Append CRC", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("append-tod");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_APPEND_TOD_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_APPEND_TOD_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Append TOD", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("learn-dis-mode");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_LEARN_DIS_MODE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_LEARN_DIS_MODE,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Learn disable mode", value);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("auto-neg-fec");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_AUTO_NEG_FEC, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_AUTO_NEG_FEC,  &value);
        }
        if (ret >= 0)
        {
            if (0 == value)
            {
                ctc_cli_out("Auto Neg FEC is none\n");
            }
            else
            {
                ctc_cli_out("Auto Neg FEC is:\n");
                if (value & (1 << CTC_PORT_FEC_TYPE_RS))
                    ctc_cli_out("\tRS\n");
                if (value & (1 << CTC_PORT_FEC_TYPE_BASER))
                    ctc_cli_out("\tBase-R\n");
            }
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("force-mpls-en");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FORCE_MPLS_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FORCE_MPLS_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Force MPLS decap", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("force-tunnel-en");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_FORCE_TUNNEL_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_FORCE_TUNNEL_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Force tunnel decap", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("xpipe");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_XPIPE_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_XPIPE_EN,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "X-pipe", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mux-type");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_MUX_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_MUX_TYPE,  &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Mux type", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-init");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_INIT, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PHY_INIT, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Phy init", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-en");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_EN, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PHY_EN, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Phy en", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-duplex");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_DUPLEX, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PHY_DUPLEX, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Phy duplex", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-medium");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_MEDIUM, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PHY_MEDIUM, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Phy medium", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-loopback");
    if ((index != 0xFF) || show_all)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_PHY_LOOPBACK, &value);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_PHY_LOOPBACK, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-34s:  %u\n", "Phy loopback", value);
        }
    }

    ctc_cli_out("------------------------------------------\n");

    if ((ret < 0) && (!show_all))
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_dir_property,
        ctc_cli_port_set_dir_property_cmd,
        "port GPHYPORT_ID dir-property  \
        ( acl-en | acl-field-sel-id | acl-port-classid | acl-port-classid-0 | acl-port-classid-1 | acl-port-classid-2  \
        | acl-port-classid-3 | acl-ipv4-force-mac | acl-ipv6-force-mac | acl-force-use-ipv6 | acl-use-class \
        | service-acl-en | acl-hash-type | acl-tcam-type-0 | acl-tcam-type-1 | acl-tcam-type-2 | acl-tcam-type-3 \
        | acl-use-mapped-vlan | acl-port-bitmap-id | qos-domain | policer-valid \
        | stag-tpid-index | random-log-rate|sd-action) direction (ingress|egress|both) value VALUE ",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port property with direction",
        CTC_CLI_ACL_ENABLE_BITMAP,
        "Port acl hash field select id",
        "Port acl port classid",
        "Port acl port classid0",
        "Port acl port classid1",
        "Port acl port classid2",
        "Port acl port classid3",
        "Port acl ipv4-key force to mac-key",
        "Port acl ipv6-key force to mac-key",
        "Port acl force use ipv6",
        "Port acl use classid",
        "Port service acl enable",
        "Port acl hash type",
        "Port acl tcam type 0",
        "Port acl tcam type 1",
        "Port acl tcam type 2",
        "Port acl tcam type 3",
        "Acl use mapped vlan",
        "Acl port bitmap id",
        "Port qos domain",
        "Port policer valid",
        "Svlan tpid index, the corresponding svlan tpid is in EpeL2TpidCtl",
        "Rate of random threshold,0-0x7fff",
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

    if (CLI_CLI_STR_EQUAL("acl-en", 1))
    {
        prop = CTC_PORT_DIR_PROP_ACL_EN;
    }
    else if CLI_CLI_STR_EQUAL("acl-field-sel-id", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-port-classid", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_CLASSID;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-port-classid-0", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_CLASSID_0;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-port-classid-1", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_CLASSID_1;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-port-classid-2", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_CLASSID_2;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-port-classid-3", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_CLASSID_3;
    }
    else if CLI_CLI_STR_EQUAL("acl-ipv4-force-mac", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_IPV4_FORCE_MAC;
    }
    else if CLI_CLI_STR_EQUAL("acl-ipv6-force-mac", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_IPV6_FORCE_MAC;
    }
    else if CLI_CLI_STR_EQUAL("acl-force-use-ipv6", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_FORCE_USE_IPV6;
    }
    else if CLI_CLI_STR_EQUAL("acl-use-class", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_USE_CLASSID;
    }
    else if CLI_CLI_STR_EQUAL("service-acl-en", 1)
    {
        prop = CTC_PORT_DIR_PROP_SERVICE_ACL_EN;
    }
    else if CLI_CLI_STR_EQUAL("acl-hash-type", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-0", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-1", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-2", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-3", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3;
    }
    else if CLI_CLI_STR_EQUAL("acl-use-mapped-vlan", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN;
    }
    else if CLI_CLI_STR_EQUAL("acl-port-bitmap-id", 1)
    {
        prop = CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID;
    }
    else if (CLI_CLI_STR_EQUAL("qos-domain", 1))
    {
        prop = CTC_PORT_DIR_PROP_QOS_DOMAIN;
    }
    else if (CLI_CLI_STR_EQUAL("policer-valid", 1))
    {
        prop = CTC_PORT_DIR_PROP_PORT_POLICER_VALID;
    }
    else if (CLI_CLI_STR_EQUAL("stag-tpid-index", 1))
    {
        prop = CTC_PORT_DIR_PROP_STAG_TPID_INDEX;
    }
    else if (CLI_CLI_STR_EQUAL("random-log-rate", 1))
    {
        prop = CTC_PORT_DIR_PROP_RANDOM_LOG_RATE;
    }
    else if (CLI_CLI_STR_EQUAL("sd-action", 1))
    {
        prop = CTC_PORT_DIR_PROP_SD_ACTION;
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

CTC_CLI(ctc_cli_port_show_port_dir_property,
        ctc_cli_port_show_port_dir_property_cmd,
        "show port GPHYPORT_ID dir-property (all \
        | acl-en | acl-field-sel-id | acl-port-classid | acl-port-classid-0 | acl-port-classid-1 | acl-port-classid-2 \
        | acl-port-classid-3 | acl-ipv4-force-mac | acl-ipv6-force-mac | acl-force-use-ipv6 | acl-use-class \
        | service-acl-en | acl-hash-type | acl-tcam-type-0 | acl-tcam-type-1 | acl-tcam-type-2 | acl-tcam-type-3 \
        | acl-use-mapped-vlan | acl-port-bitmap-id | qos-domain | policer-valid | stag-tpid-index | random-log-rate|sd-action) \
        direction ( ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Property with direction",
        "All property",
        CTC_CLI_ACL_ENABLE_BITMAP,
        "Port acl hash field select id",
        "Port acl port classid",
        "Port acl port classid0",
        "Port acl port classid1",
        "Port acl port classid2",
        "Port acl port classid3",
        "Port acl ipv4-key force to mac-key",
        "Port acl ipv6-key force to mac-key",
        "Port acl force use ipv6",
        "Port acl use classid",
        "Port service acl enable",
        "Port acl hash type",
        "Port acl tcam type 0",
        "Port acl tcam type 1",
        "Port acl tcam type 2",
        "Port acl tcam type 3",
        "Acl use mapped vlan",
        "Acl port bitmap id",
        "Port qos domain",
        "Port policer valid",
        "Svlan tpid index, the corresponding svlan tpid is in EpeL2TpidCtl",
        "Rate of random threshold,0-0x7fff",
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
    uint8 show_all = 0;

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        show_all = 1;
    }

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

    index = CTC_CLI_GET_ARGC_INDEX("acl-en");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_EN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-en             :  0x%x\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-field-sel-id");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-field-sel-id     :  0x%x\n", value);
        }
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-port-classid");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-classid   :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-port-classid-0");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_CLASSID_0, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID_0, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-classid-0   :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-port-classid-1");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_CLASSID_1, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID_1, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-classid-1   :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-port-classid-2");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_CLASSID_2, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID_2, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-classid-2   :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-port-classid-3");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_CLASSID_3, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_CLASSID_3, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-classid-3   :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-ipv4-force-mac");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_IPV4_FORCE_MAC, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_IPV4_FORCE_MAC, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-ipv4-force-mac :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-ipv6-force-mac");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_IPV6_FORCE_MAC, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_IPV6_FORCE_MAC, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-ipv6-force-mac :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-force-use-ipv6");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_FORCE_USE_IPV6, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_FORCE_USE_IPV6, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-force-use-ipv6 :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-class");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-use-class      :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_SERVICE_ACL_EN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_SERVICE_ACL_EN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Service-acl-en     :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-hash-type");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-hash-type      :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-0");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-tcam-type-0    :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-1");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-tcam-type-1    :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-2");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-tcam-type-2    :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-3");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-tcam-type-3    :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-mapped-vlan");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-use-mapped-vlan:  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-port-bitmap-id");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Acl-port-bitmap-id:  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-domain");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_QOS_DOMAIN, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_QOS_DOMAIN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Qos-domain         :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-valid");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_PORT_POLICER_VALID, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_PORT_POLICER_VALID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Policer-valid      :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-tpid-index");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Stag-tpid-index      :  %d\n", value);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("random-log-rate");
    if (INDEX_VALID(index) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_RANDOM_LOG_RATE, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_RANDOM_LOG_RATE, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("Random-log-rate      :  %d\n", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("sd-action");
    if (INDEX_VALID(index))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_port_get_direction_property(g_api_lchip, gport, CTC_PORT_DIR_PROP_SD_ACTION, dir, &value);
        }
        else
        {
            ret = ctc_port_get_direction_property(gport, CTC_PORT_DIR_PROP_SD_ACTION, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("OAM SD action      :  %u\n", value);
        }
    }
    ctc_cli_out("==============================\n");
    if ((ret < 0) && (!show_all))
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_port_set_scl_property,
        ctc_cli_port_set_scl_property_cmd,
        "port GPHYPORT_ID scl-key-type scl-id SCL_ID direction (ingress | egress) " \
        "type (cvid | svid | cvid-ccos | svid-scos | port | dvid | mac-sa | mac-da | ipv4 | ipv6| " \
        "ipsg-port-mac | ipsg-port-ip | ipsg-ipv6 | port-mac-da | l2 | tunnel-v6 | tunnel {auto|rpf|} | " \
        "nvgre | vxlan | vxlan-gpe | geneve | trill | port-cross | port-vlan-cross | svid-dscp |svid-only|svid-mac| disable) (hash-vlan-range-disable |) " \
        "(tcam-type (mac|ip|ip-single|vlan|tunnel|tunnel-rpf|resolve-conflict|udf|udf-ext| disable) |) (tcam-vlan-range-disable |) "\
        "{(class-id CLASS_ID | ip-src-guard | vlan-class) (hash-use-class-id|) | use-logic-port |} (action-type (scl | flow | tunnel) |)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "SCL key type on port",
        "SCL id",
        CTC_CLI_SCL_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Hash Type",
        "Hash port cvlan",
        "Hash port svlan",
        "Hash port cvlan ccos",
        "Hash port svlan scos",
        "Hash port",
        "Hash port svlan cvlan",
        "Hash macsa",
        "Hash macda",
        "Hash ipsa",
        "Hash ipsa",
        "Hash port macsa",
        "Hash port ipsa",
        "Hash ipsa",
        "Hash port macda",
        "Hash l2",
        "IPv6 based IP tunnel, layer3 tcam key",
        "IP tunnel for IPv4/IPv6 in IPv4, 6to4, ISATAP, GRE with/without key",
        "Auto tunnel, for 6to4, ISATAP, must use auto tunnel",
        "Rpf check for outer header, if set, scl-id parameter must be 1",
        "NvGRE tunnel",
        "VxLAN tunnel",
        "VxLAN-GPE tunnel",
        "GENEVE tunnel",
        "Trill tunnel or check",
        "Port cross hash key",
        "Port vlan cross hash key",
        "Hash port svlan dscp",
        "Hash svlan",
        "Hash svlan mac",
        "Hash disable",
        "Hash vlan range disable",
        "Tcam Type",
        "Tcam mac, layer2 tcam key",
        "Tcam ip, layer2/layer3 tcam key",
        "Tcam ip-single, layer3 tcam key",
        "Tcam vlan, vlan tcam key",
        "Tcam tunnel, tunnel tcam key",
        "Tcam tunnel rpf, tunnel rpf tcam key",
        "Resolve hash conflict",
        "Tcam udf, layer3 tcam key",
        "Tcam udf extend, layer3 tcam key",
        "Tcam disable",
        "Tcam vlan range disable",
        "Class id",
        "Class id",
        "Ip source guard",
        "VLAN class",
        "Hash lookup use class id",
        "Use logic port lookup",
        "Action type",
        "Normal SCL action",
        "Flow SCL action",
        "Tunnel Action")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 index = 0xFF;
    ctc_port_scl_property_t port_scl_property;
    sal_memset(&port_scl_property, 0, sizeof(ctc_port_scl_property_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("scl id", port_scl_property.scl_id, argv[1], 0, CTC_MAX_UINT8_VALUE);

    index = 3; /* hash type*/
    if (CLI_CLI_STR_EQUAL("ingress", 2))
    {
        port_scl_property.direction = CTC_INGRESS;
        if (CLI_CLI_STR_EQUAL("svid-dscp", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_DSCP;
        }
        else if (CLI_CLI_STR_EQUAL("svid-only", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN;
        }
        else if (CLI_CLI_STR_EQUAL("svid-mac", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN_MAC;
        }
        else if (CLI_CLI_STR_EQUAL("dvid", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
        }
        else if (CLI_CLI_STR_EQUAL("svid-scos", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_COS;
        }
        else if (CLI_CLI_STR_EQUAL("cvid-ccos", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN_COS;
        }
        else if (CLI_CLI_STR_EQUAL("svid", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
        }
        else if (CLI_CLI_STR_EQUAL("cvid", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN;
        }
        else if (CLI_CLI_STR_EQUAL("mac-sa", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_MAC_SA;
        }
        else if (CLI_CLI_STR_EQUAL("mac-da", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_MAC_DA;
        }
        else if (CLI_CLI_STR_EQUAL("ipsg-port-mac", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA;
        }
        else if (CLI_CLI_STR_EQUAL("port-mac-da", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_DA;
        }
        else if (CLI_CLI_STR_EQUAL("ipv4", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA;
        }
        else if (CLI_CLI_STR_EQUAL("ipv6", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA;
        }
        else if (CLI_CLI_STR_EQUAL("ipsg-ipv6", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA;
        }
        else if (CLI_CLI_STR_EQUAL("ipsg-port-ip", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA;
        }
        else if (CLI_CLI_STR_EQUAL("l2", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_L2;
        }
        else if (CLI_CLI_STR_EQUAL("port", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT;
        }
        else if (CLI_CLI_STR_EQUAL("tunnel-v6", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL;
            port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_TUNNEL;

        }
        else if (CLI_CLI_STR_EQUAL("tunnel", index))
        {
            if (0xFF != CTC_CLI_GET_ARGC_INDEX("rpf"))
            {
                port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL_RPF;
            }
            else if (0xFF != CTC_CLI_GET_ARGC_INDEX("auto"))
            {
                port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IPV4_TUNNEL_AUTO;
            }
            else
            {
                port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL;
            }
        }
        else if (CLI_CLI_STR_EQUAL("nvgre", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE;
        }
        else if (CLI_CLI_STR_EQUAL("vxlan", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN;
        }
        else if (CLI_CLI_STR_EQUAL("vxlan-gpe", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN;
        }
        else if (CLI_CLI_STR_EQUAL("geneve", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN;
        }
        else if (CLI_CLI_STR_EQUAL("trill", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TRILL;
        }
        else if (CLI_CLI_STR_EQUAL("disable", index))
        {
            port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        }
    }
    else if (CLI_CLI_STR_EQUAL("egress", 2))
    {
        port_scl_property.direction = CTC_EGRESS;
        if (CLI_CLI_STR_EQUAL("dvid", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN;
        }
        else if (CLI_CLI_STR_EQUAL("svid-scos", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN_COS;
        }
        else if (CLI_CLI_STR_EQUAL("cvid-ccos", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN_COS;
        }
        else if (CLI_CLI_STR_EQUAL("svid", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN;
        }
        else if (CLI_CLI_STR_EQUAL("cvid", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN;
        }
        else if (CLI_CLI_STR_EQUAL("port-cross", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_XC;
        }
        else if (CLI_CLI_STR_EQUAL("port-vlan-cross", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_VLAN_XC;
        }
        else if (CLI_CLI_STR_EQUAL("port", index))
        {
            port_scl_property.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-vlan-range-disable");
    if (INDEX_VALID(index))
    {
        port_scl_property.hash_vlan_range_dis = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcam-type");
    if (INDEX_VALID(index))
    {
        index ++;
        if (CLI_CLI_STR_EQUAL("mac", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("ip", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("ip-single", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP_SINGLE;
        }
        else if (CLI_CLI_STR_EQUAL("vlan", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_VLAN;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("tunnel", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("tunnel-rpf", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL_RPF;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("resolve-conflict", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("udf", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_UDF;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("udf-ext", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_UDF_EXT;
        }
        else if (CLI_CLI_STR_EQUAL("disable", index))
        {
            port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcam-vlan-range-disable");
    if (INDEX_VALID(index))
    {
        port_scl_property.tcam_vlan_range_dis = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("class-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("class id", port_scl_property.class_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-src-guard");
    if (INDEX_VALID(index))
    {
        port_scl_property.class_id = CTC_PORT_CLASS_ID_RSV_IP_SRC_GUARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-class");
    if (INDEX_VALID(index))
    {
        port_scl_property.class_id = CTC_PORT_CLASS_ID_RSV_VLAN_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-use-class-id");
    if (INDEX_VALID(index))
    {
        port_scl_property.class_id_en = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (INDEX_VALID(index))
    {
        port_scl_property.use_logic_port_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("action-type");
    if (INDEX_VALID(index))
    {
        index++;
        if (CLI_CLI_STR_EQUAL("scl", index))
        {
            port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        }
        else if (CTC_CLI_STR_EQUAL_ENHANCE("flow", index))
        {
            port_scl_property.action_type= CTC_PORT_SCL_ACTION_TYPE_FLOW;
        }
        else
        {
            port_scl_property.action_type= CTC_PORT_SCL_ACTION_TYPE_TUNNEL;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_scl_property(g_api_lchip, gport, &port_scl_property);
    }
    else
    {
        ret = ctc_port_set_scl_property(gport, &port_scl_property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_port_set_auto_neg,
        ctc_cli_port_set_auto_neg_cmd,
        "port GPHYPORT_ID auto-neg (enable|disable)",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Auto-neg",
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;
    uint16 gport;
    uint32 value = 0;
    uint8 index = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (index != 0xFF)
    {
        value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        value = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_auto_neg(g_api_lchip, gport, value);
    }
    else
    {
        ret = ctc_port_set_auto_neg(gport, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_port_set_link_intr,
        ctc_cli_port_set_link_intr_cmd,
        "port GPHYPORT_ID mac link-intr (enable|disable)",
        CTC_CLI_INTR_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "mac",
        "link change interrupt",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport;
    uint32 value = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_link_intr(g_api_lchip, gport, value);
    }
    else
    {
        ret = ctc_port_set_link_intr(gport, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_show_port_cpu_mac_en,
        ctc_cli_show_port_cpu_mac_en_cmd,
        "show port cpumac enable",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        "cpu mac",
        "Enable")
{
    int32 ret = CLI_SUCCESS;
    bool enable = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_get_cpu_mac_en(g_api_lchip, &enable);
    }
    else
    {
        ret = ctc_port_get_cpu_mac_en(&enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("cpu mac enable      :%d\n", enable);

    return ret;
}

CTC_CLI(ctc_cli_port_set_mac_auth,
        ctc_cli_port_set_mac_auth_cmd,
        "port GPHYPORT_ID mac-auth (enable|disable)",
        CTC_CLI_INTR_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Port base MAC authentication",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = CLI_SUCCESS;
    uint16 gport;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_port_set_mac_auth(g_api_lchip, gport, enable);
    }
    else
    {
        ret = ctc_port_set_mac_auth(gport, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_show_port_capability,
        ctc_cli_show_port_capability_cmd,
        "show port GPHYPORT_ID capability {serdes-info|mac-id|speed-mode|if-type|fec-type|cl73-ability|}",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Capability",
        "Serdes info",
        "Mac id",
        "Speed mode",
        "Interface type",
        "Fec type",
        "CL73 ability")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gport = 0;
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 value2 = 0;
    uint8  show_all = 0;
    uint32 loop = 0;
    uint8  flag = 0;
    uint8  flag1 = 0;
    uint8  flag2 = 0;
    uint32 len = 0;
    uint32 len1 = 0;
    uint32 len2 = 0;
    ctc_port_serdes_info_t serdes_port;
    char* serdes_mode_name[CTC_CHIP_MAX_SERDES_MODE] = {"NONE", "XFI", "SGMII", "XSGMII", "QSGMII", "XAUI", "DXAUI", "XLG",\
            "CG", "2DOT5G", "USXGMII0", "USXGMII1", "USXGMII2", "XXVG", "LG", "100BASEFX"};

    char* speed_mode_name[CTC_PORT_SPEED_MAX] = {"1G", "100M", "10M", "2G5", "10G", "20G", "40G", "100G",\
            "5G", "25G", "50G"};
    char  speed_mode_str[128] = {0};

    char* if_type_name[CTC_PORT_IF_MAX_TYPE] = {"NONE", "SGMII", "2500X", "QSGMII", "USXGMII_S", "USXGMII_M2G5", "USXGMII_M5G", "XAUI",\
            "DXAUI", "XFI", "KR", "CR", "KR2", "CR2", "KR4", "CR4"};
    char  if_type_str[128] = {0};

    char* fec_type_name[CTC_PORT_FEC_TYPE_MAX] = {"NONE", "RS", "BASER"};
    char  fec_type_str[64] = {0};

    char* cl73_name[30] = {"10GBASE-KR", "40GBASE-KR4", "40GBASE-CR4", "100GBASE-KR4", "100GBASE-CR4",\
                           "FEC-ABILITY", "FEC-REQ", "25GBASE-KR-S\n    25GBASE-CR-S", "25GBASE-KR\n    25GBASE-CR",\
                           "25G-RS-FEC-REQ", "25G-BASER-FEC-REQ", "25GBASE-KR1", "25GBASE-CR1", "50GBASE-KR2",\
                           "50GBASE-CR2", "CL91-ABILITY", "CL74-ABILITY", "CL91-REQ", "CL74-REQ"};
    char*  p_cl73_str_l = {0}; /* local ability */
    char*  p_cl73_str_r = {0}; /* remote ability */
    char*  p_cl73_str_c = {0}; /* local configured ability */


    sal_memset(&serdes_port, 0, sizeof(serdes_port));
    CTC_CLI_GET_UINT32("gport", gport, argv[0]);

    if (1 == argc)
    {
        show_all = 1;
    }
    ctc_cli_out("Show GPort          :  0x%04x\n", gport);
    ctc_cli_out("---------------------------------------------------------------------------------------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("serdes-info");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_SERDES_INFO, (void*)&serdes_port);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_SERDES_INFO, (void*)&serdes_port);
        }
        if (ret >= 0)
        {
            if (CTC_CHIP_SERDES_LG_MODE == serdes_port.serdes_mode)
            {
                ctc_cli_out("%-20s:  %u %u\n", "serdes id", serdes_port.serdes_id_array[0], serdes_port.serdes_id_array[1]);
            }
            else
            {
                ctc_cli_out("%-20s:  %u\n", "serdes id", serdes_port.serdes_id);
            }
            ctc_cli_out("%-20s:  %u\n", "serdes num", serdes_port.serdes_num);
            ctc_cli_out("%-20s:  %s\n", "serdes mode", serdes_mode_name[serdes_port.serdes_mode]);
            ctc_cli_out("%-20s:  %u\n", "overclocking", serdes_port.overclocking_speed);
        }
        else
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-id");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_MAC_ID, &value);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_MAC_ID, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-20s:  %u\n", "mac id", value);
        }
        else
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    len = 0;
    flag = 0;
    index = CTC_CLI_GET_ARGC_INDEX("fec-type");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_FEC_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_FEC_TYPE, &value);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        for (loop = 1; loop < CTC_PORT_FEC_TYPE_MAX; loop++)
        {
            if (CTC_IS_BIT_SET(value, loop))
            {
                len += sal_sprintf(fec_type_str+len, "%s/", fec_type_name[loop]);
                flag = 1;
            }
        }
        ctc_cli_out("%-20s:  %s\n", "fec type", flag?fec_type_str:"-");

    }

    len = 0;
    flag = 0;
    index = CTC_CLI_GET_ARGC_INDEX("speed-mode");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_SPEED_MODE, &value);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_SPEED_MODE, &value);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        for (loop = 0; loop < CTC_PORT_SPEED_MAX; loop++)
        {
            if (CTC_IS_BIT_SET(value, loop))
            {
                len += sal_sprintf(speed_mode_str+len, "%s/", speed_mode_name[loop]);
                flag = 1;
            }
        }
        ctc_cli_out("%-20s:  %s\n", "speed mode", flag?speed_mode_str:"-");
    }

    len = 0;
    flag = 0;
    index = CTC_CLI_GET_ARGC_INDEX("if-type");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_IF_TYPE, &value);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_IF_TYPE, &value);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        for (loop = 1; loop < CTC_PORT_IF_MAX_TYPE; loop++)
        {
            if (CTC_IS_BIT_SET(value, loop))
            {
                len += sal_sprintf(if_type_str+len, "%s/", if_type_name[loop]);
                flag = 1;
            }
        }
        ctc_cli_out("%-20s:  %s\n", "interface type", flag?if_type_str:"-");
    }

    len = 0;
    len1 = 0;
    flag = 0;
    flag1 = 0;
    flag2 = 0;
    value = 0;
    value1 = 0;
    value2 = 0;
    index = CTC_CLI_GET_ARGC_INDEX("cl73-ability");
    if ((index != 0xFF) || show_all)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_CL73_ABILITY, &value);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_CL73_ABILITY, &value);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if(g_ctcs_api_en)
        {
             ret = ctcs_port_get_capability(g_api_lchip, gport, CTC_PORT_CAP_TYPE_CL73_REMOTE_ABILITY, &value1);
        }
        else
        {
            ret = ctc_port_get_capability(gport, CTC_PORT_CAP_TYPE_CL73_REMOTE_ABILITY, &value1);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if(g_ctcs_api_en)
        {
            ret = ctcs_port_get_property(g_api_lchip, gport, CTC_PORT_PROP_CL73_ABILITY, &value2);
        }
        else
        {
            ret = ctc_port_get_property(gport, CTC_PORT_PROP_CL73_ABILITY, &value2);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        p_cl73_str_l = (char*)mem_malloc(MEM_CLI_MODULE, 256);
        p_cl73_str_r = (char*)mem_malloc(MEM_CLI_MODULE, 256);
        p_cl73_str_c = (char*)mem_malloc(MEM_CLI_MODULE, 256);
        if ((NULL == p_cl73_str_l) || (NULL == p_cl73_str_r) || (NULL == p_cl73_str_c))
        {
            if (p_cl73_str_l)
            {
                mem_free(p_cl73_str_l);
            }
            if (p_cl73_str_r)
            {
                mem_free(p_cl73_str_r);
            }
            if (p_cl73_str_c)
            {
                mem_free(p_cl73_str_c);
            }
            return CLI_ERROR;
        }
        sal_memset(p_cl73_str_l, 0, 256);
        sal_memset(p_cl73_str_r, 0, 256);
        for (loop = 0; loop < 32; loop++)
        {
            if (CTC_IS_BIT_SET(value, loop))
            {
                len += sal_sprintf(p_cl73_str_l+len, "%s\n    ", cl73_name[loop]);
                flag = 1;
            }
            if (CTC_IS_BIT_SET(value1, loop))
            {
                len1 += sal_sprintf(p_cl73_str_r+len1, "%s\n    ", cl73_name[loop]);
                flag1 = 1;
            }
            if (CTC_IS_BIT_SET(value2, loop))
            {
                len2 += sal_sprintf(p_cl73_str_c+len2, "%s\n    ", cl73_name[loop]);
                flag2 = 1;
            }
        }

        ctc_cli_out("%-20s:", "supported cl73 local ability");
        if(flag)
        {
            ctc_cli_out("\n    %s\n", p_cl73_str_l);
        }
        else
        {
            ctc_cli_out("  - \n");
        }
        ctc_cli_out("%-20s:", "configured cl73 local ability");
        if (flag2)
        {
            ctc_cli_out("\n    %s\n", p_cl73_str_c);
        }
        else
        {
            ctc_cli_out("  - \n");
        }
        ctc_cli_out("%-20s:", "cl73 remote ability");
        if(flag1)
        {
            ctc_cli_out("\n    %s\n", p_cl73_str_r);
        }
        else
        {
            ctc_cli_out("  - \n");
        }
        mem_free(p_cl73_str_l);
        mem_free(p_cl73_str_r);
        mem_free(p_cl73_str_c);
    }

    return ret;
}

CTC_CLI(ctc_cli_show_port_bpe_property,
        ctc_cli_show_port_bpe_property_cmd,
        "show port GPHYPORT_ID bpe-property",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Bpe property")
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    ctc_port_bpe_property_t property;

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);
    if (g_ctcs_api_en)
    {
        ret = ctcs_port_get_bpe_property(g_api_lchip, gport, &property);
    }
    else
    {
        ret = ctc_port_get_bpe_property(gport, &property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show GPort                        :  0x%04x\n", gport);
    ctc_cli_out("-------------------------------------------\n");
    ctc_cli_out("%-34s:  %d\n", "extend_type", property.extend_type);
    ctc_cli_out("%-34s:  %d\n", "name_space", property.name_space);
    ctc_cli_out("%-34s:  %d\n", "ecid", property.ecid);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_show_port_mac_authen_en,
        ctc_cli_show_port_mac_authen_en_cmd,
        "show port GPHYPORT_ID mac-authen-en",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Mac authen enable")
{
    int32 ret = CLI_SUCCESS;
    uint32 gport = 0;
    bool enable = FALSE;

    CTC_CLI_GET_UINT32("gport", gport, argv[0]);
    if (g_ctcs_api_en)
    {
        ret = ctcs_port_get_mac_auth(g_api_lchip, gport, &enable);
    }
    else
    {
        ret = ctc_port_get_mac_auth(gport, &enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show GPort                        :  0x%04x\n", gport);
    ctc_cli_out("-------------------------------------------\n");
    ctc_cli_out("%-34s:  %d\n", "Mac-authen enable", enable);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_show_port_unmap,
        ctc_cli_show_port_unmap_cmd,
        "show port GPHYPORT_ID unmap",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC,
        "gport info")
{
    uint32 gport = 0;
    uint8 type = 0;
    uint8 lport_ext = 0;
    uint8 gchip = 0;
    uint8 lport = 0;
    CTC_CLI_GET_UINT32("gport", gport, argv[0]);
    type= (gport & 0xf0000000) >> 28;
    lport_ext = (gport & 0x38000) >> 15;
    gchip = (gport & 0x7f00) >> 8;
    lport = gport & 0xff;
    if(type>1)
    {
        ctc_cli_out("%%Invalid port type\n");
        return CLI_ERROR;
    }


    if(gchip == 0x1f)
    {
        ctc_cli_out("type: %s\tgchip: %s\tlport: %d\n", gport>>28?"CPU PORT":"Normal PORT", "0x1f(Linkagg)", (lport_ext<<8)|lport);
    }
    else
    {
        ctc_cli_out("type: %s\tgchip: %d\tlport: %d\n", gport>>28?"CPU PORT":"Normal PORT", gchip, (lport_ext<<8)|lport);
    }
    return CLI_SUCCESS;
}
int32
ctc_port_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_default_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_port_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_vlan_domain_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_receive_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_transmit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_bridge_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_sub_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_phy_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_vlan_filtering_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_default_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_vlan_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_cross_connect_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_learning_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_stag_tpid_index_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_dot1q_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_untag_dft_vid_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_untag_dft_vid_disable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_protocol_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_blocking_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_use_outer_ttl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_mac_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_cpumac_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_all_mac_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_all_port_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_speed_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_flow_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_flow_ctl_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_preamble_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_min_frame_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_max_frame_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_pading_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_random_log_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_random_threshold_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_srcdiscard_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_loopback_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_efm_lb_mode_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_efm_lb_mode_disable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_port_check_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_ipg_index_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_isolation_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_reflective_bridge_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_internal_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_allocate_internal_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_release_internal_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_system_ipg_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_system_max_frame_size_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_port_set_system_port_mac_prefix_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_port_mac_postfix_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_mac_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_port_show_ipg_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_system_max_frame_size_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_port_set_property_cmd);
#if defined(SDK_IN_USERMODE)
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_property_array_cmd);
#endif
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_dir_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_dir_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_scl_property_cmd);
     /*install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_info_cmd);*/
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_auto_neg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_port_cpu_mac_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_link_intr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_3ap_auto_neg_ability_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_mac_auth_cmd);

	install_element(CTC_SDK_MODE, &ctc_cli_port_set_port_isolation_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_show_port_isolation_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_port_capability_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_port_bpe_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_port_mac_authen_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_port_unmap_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_set_interface_mode_cmd);

    return CLI_SUCCESS;
}

