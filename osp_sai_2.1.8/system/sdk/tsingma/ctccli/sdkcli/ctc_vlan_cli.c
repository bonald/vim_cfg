/**
 @file ctc_vlan_cli.c

 @date 2009-11-13

 @version v2.0

 The file applies cli to test sdk module of SDK
*/

#include "ctc_vlan_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

CTC_CLI(ctc_cli_vlan_create_vlan,
        ctc_cli_vlan_create_vlan_cmd,
        "vlan create vlan VLAN_ID",
        CTC_CLI_VLAN_M_STR,
        "Create one vlan",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 vlan_id;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_create_vlan(g_api_lchip, vlan_id);
    }
    else
    {
        ret = ctc_vlan_create_vlan(vlan_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_create_vlan_with_vlanptr,
        ctc_cli_vlan_create_vlan_with_vlanptr_cmd,
        "vlan create vlan VLAN_ID ((uservlanptr VLAN_PTR fid FID) | default-entry)  (use-logic-port|)",
        CTC_CLI_VLAN_M_STR,
        "Create one vlan",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "User vlanptr",
        CTC_CLI_VLAN_RANGE_DESC,
        "Forwarding ID",
        CTC_CLI_FID_ID_DESC,
        "Default entry",
        "Use logic port check")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));

    CTC_CLI_GET_UINT16("vlan id", user_vlan.vlan_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("uservlanptr");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("user vlan ptr", user_vlan.user_vlanptr, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("fid", user_vlan.fid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (index != 0xFF)
    {
        user_vlan.flag |= CTC_MAC_LEARN_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (index != 0xFF)
    {
        user_vlan.fid = user_vlan.vlan_id;
        user_vlan.user_vlanptr = user_vlan.vlan_id;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_create_uservlan(g_api_lchip, &user_vlan);
    }
    else
    {
        ret = ctc_vlan_create_uservlan(&user_vlan);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_vlan,
        ctc_cli_vlan_remove_vlan_cmd,
        "vlan remove vlan VLAN_ID",
        CTC_CLI_VLAN_M_STR,
        "Remove one vlan",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = 0;
    uint16 vlan_id;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_destroy_vlan(g_api_lchip, vlan_id);
    }
    else
    {
        ret = ctc_vlan_destroy_vlan(vlan_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));

        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_vlan_set_member_ports,
        ctc_cli_vlan_set_member_ports_cmd,
        "vlan (add | del) ports " CTC_CLI_PORT_BITMAP_STR " vlan VLAN_ID (lchip LCHIP|)",
        CTC_CLI_VLAN_M_STR,
        "Add member ports to vlan",
        "Delete member ports from vlan",
        "Local ports",
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
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 temp = 0;
    uint8 lchip = 0;
    uint16  vlan_id = 0;
    ctc_port_bitmap_t port_bitmap = {0};


    index = CTC_CLI_GET_ARGC_INDEX("pbmp0");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 0", port_bitmap[0], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp1");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 1", port_bitmap[1], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp2");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 2", port_bitmap[2], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp3");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 3", port_bitmap[3], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp4");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 4", port_bitmap[4], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp5");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 5", port_bitmap[5], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp6");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 6", port_bitmap[6], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp7");        /*Append for TM*/
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 7", port_bitmap[7], argv[index + 1]);
        if (temp < index)
        {
            temp = index + 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("pbmp8");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 8", port_bitmap[8], argv[index + 1]);
        if(temp < index)
        {
            temp = index + 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp9");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 9", port_bitmap[9], argv[index + 1]);
        if(temp < index)
        {
            temp = index + 1;
        }
    }

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[temp + 1]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip id", lchip, argv[index + 1]);
    }
    if (0 == sal_memcmp("add", argv[0], sal_strlen("add")))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_vlan_add_ports(g_api_lchip, vlan_id, port_bitmap);
        }
        else
        {
            ret = ctc_vlan_add_ports(lchip, vlan_id, port_bitmap);
        }
    }
    else if (0 == sal_memcmp("del", argv[0], sal_strlen("del")))
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_vlan_remove_ports(g_api_lchip, vlan_id, port_bitmap);
        }
        else
        {
            ret = ctc_vlan_remove_ports(lchip, vlan_id, port_bitmap);
        }
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CTC_E_NONE;
}




CTC_CLI(ctc_cli_vlan_set_member_port,
        ctc_cli_vlan_set_member_port_cmd,
        "vlan (add |del) port GPHYPORT_ID vlan VLAN_ID ",
        CTC_CLI_VLAN_M_STR,
        "Add member port to vlan",
        "Delete member port from vlan",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32   ret = 0;
    uint16 vlan_id = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[2]);
    CTC_CLI_GET_UINT16("gport", gport, argv[1]);


    if (0 == sal_memcmp("a", argv[0], 1))
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_port(g_api_lchip, vlan_id, gport);
    }
    else
    {
        ret = ctc_vlan_add_port(vlan_id, gport);
    }
    }
    else if (0 == sal_memcmp("d", argv[0], 1))
    {
        if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_port(g_api_lchip, vlan_id, gport);
    }
    else
    {
        ret = ctc_vlan_remove_port(vlan_id, gport);
    }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_receive,
        ctc_cli_vlan_set_receive_cmd,
        "vlan VLAN_ID receive (enable |disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "If set, receive on the vlan is enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_receive_en(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_receive_en(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_ipv4_l2mc,
        ctc_cli_vlan_set_ipv4_l2mc_cmd,
        "vlan VLAN_ID ipv4-l2mc (enable |disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Ipv4 based l2mc",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_IPV4_BASED_L2MC, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_IPV4_BASED_L2MC, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_ipv6_l2mc,
        ctc_cli_vlan_set_ipv6_l2mc_cmd,
        "vlan VLAN_ID ipv6-l2mc (enable |disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Ipv6 based l2mc",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_IPV6_BASED_L2MC, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_IPV6_BASED_L2MC, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_transmit,
        ctc_cli_vlan_set_transmit_cmd,
        "vlan VLAN_ID transmit (enable | disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "If set, transmit is enable on the vlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_transmit_en(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_transmit_en(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_bridge,
        ctc_cli_vlan_set_bridge_cmd,
        "vlan VLAN_ID bridge (enable | disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "If set, bridge is enable on the vlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_bridge_en(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_bridge_en(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_stpid,
        ctc_cli_vlan_set_stpid_cmd,
        "vlan VLAN_ID stpid STPID",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Stp instance id",
        "<0-127>")
{
    int32 ret;
    uint16 vlan_id;
    uint8 stp_id;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    CTC_CLI_GET_UINT8_RANGE("stp id", stp_id, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stp_set_vlan_stpid(g_api_lchip, vlan_id, stp_id);
    }
    else
    {
        ret = ctc_stp_set_vlan_stpid(vlan_id, stp_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_fid,
        ctc_cli_vlan_set_fid_cmd,
        "vlan VLAN_ID vrfid VRF_ID",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC)
{
    int32 ret;
    uint16 vlan_id;
    uint16 vrf_id;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    CTC_CLI_GET_UINT16("vrfid", vrf_id, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_fid(g_api_lchip, vlan_id, vrf_id);
    }
    else
    {
        ret = ctc_vlan_set_fid(vlan_id, vrf_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_learning,
        ctc_cli_vlan_set_learning_cmd,
        "vlan VLAN_ID mac-learning (enable|disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Mac learning on the vlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = TRUE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("dis", argv[1], 3))
    {
        is_enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_learning_en(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_learning_en(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_set_igmp_snooping,
        ctc_cli_vlan_set_igmp_snooping_cmd,
        "vlan VLAN_ID igmp-snooping (enable|disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "IGMP snooping on vlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = TRUE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("dis", argv[1], 3))
    {
        is_enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_igmp_snoop_en(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_igmp_snoop_en(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_arp_dhcp_excp,
        ctc_cli_vlan_set_arp_dhcp_excp_cmd,
        "vlan VLAN_ID (arp|dhcp|fip) action (copy-to-cpu|forward|redirect-to-cpu|discard)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "ARP packet of vlan",
        "DHCP packet of vlan",
        "FIP packet of vlan",
        "Exception action",
        "Forwarding and to cpu",
        "Normal forwarding",
        "Redirect to cpu",
        "Discard")
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    ctc_exception_type_t type;

    CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (CLI_CLI_STR_EQUAL("forward", 2))
    {
        type = CTC_EXCP_NORMAL_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("redirect-to-cpu", 2))
    {
        type = CTC_EXCP_DISCARD_AND_TO_CPU;
    }
    else if (CLI_CLI_STR_EQUAL("copy-to-cpu", 2))
    {
        type = CTC_EXCP_FWD_AND_TO_CPU;
    }
    else
    {
        type = CTC_EXCP_DISCARD;
    }

    if (CLI_CLI_STR_EQUAL("arp", 1))
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_vlan_set_arp_excp_type(g_api_lchip, vlan_id, type);
        }
        else
        {
            ret = ctc_vlan_set_arp_excp_type(vlan_id, type);
        }
    }
    else if (CLI_CLI_STR_EQUAL("dhcp", 1))
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_vlan_set_dhcp_excp_type(g_api_lchip, vlan_id, type);
        }
        else
        {
            ret = ctc_vlan_set_dhcp_excp_type(vlan_id, type);
        }
    }
    else if (CLI_CLI_STR_EQUAL("fip", 1))
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_FIP_EXCP_TYPE, type);
        }
        else
        {
            ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_FIP_EXCP_TYPE, type);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_drop_unknown_pkt,
        ctc_cli_vlan_set_drop_unknown_pkt_cmd,
        "vlan VLAN_ID (drop-unknown-ucast|drop-unknown-mcast|drop-unknown-bcast|unknown-mcast-copy-tocpu) (enable|disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Drop unknown ucast packet",
        "Drop unknown mcast packet",
        "Drop unknown bcast packet",
        "Unknown mcast packet copy to cpu",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    uint32 value = 0;
    ctc_vlan_property_t type = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("drop-unknown-ucast", argv[1], sal_strlen("drop-unknown-ucast")))
    {
        type = CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN;
    }
    else if(0 == sal_memcmp("drop-unknown-mcast", argv[1], sal_strlen("drop-unknown-mcast")))
    {
        type = CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN;
    }
    else if(0 == sal_memcmp("drop-unknown-bcast", argv[1], sal_strlen("drop-unknown-bcast")))
    {
        type = CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN;
    }
    else if(0 == sal_memcmp("unknown-mcast-copy-tocpu", argv[1], sal_strlen("unknown-mcast-copy-tocpu")))
    {
        type = CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU;
    }

    if (0 == sal_memcmp("en", argv[2], 2))
    {
        value = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, type, value);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, type, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_port_tag,
        ctc_cli_vlan_set_port_tag_cmd,
        "vlan VLAN_ID port GPHYPORT_ID (tagged|untagged)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Gport id",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Vlan tagged on this port",
        "Vlan untagged on this port")
{
    int32 ret;
    uint16 vlan_id;
    uint16 gport;
    bool is_enable = FALSE;
    uint8 index = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    CTC_CLI_GET_UINT16("gport", gport, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("tagged");
    if (0xFF != index)
    {
        is_enable = 1;
    }
    else
    {
        is_enable = 0;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_tagged_port(g_api_lchip, vlan_id, gport, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_tagged_port(vlan_id, gport, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_ports_tag,
        ctc_cli_vlan_set_ports_tag_cmd,
        "vlan VLAN_ID ports {pbmp0 PORT_BITMAP_0 | pbmp1 PORT_BITMAP_1 | pbmp2 PORT_BITMAP_2 | \
                                     pbmp3 PORT_BITMAP_3 | pbmp4 PORT_BITMAP_4 | pbmp5 PORT_BITMAP_5 | \
                                     pbmp6 PORT_BITMAP_6 | pbmp7 PORT_BITMAP_7 | \
                                     pbmp8 PORT_BITMAP_8 | pbmp9 PORT_BITMAP_9 } tagged (lchip LCHIP|)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Local ports",
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
        "Vlan tagged on ports",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16  vlan_id = 0;
    uint32 max_port_num_per_chip = 0;
    ctc_port_bitmap_t port_bitmap = {0};
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("pbmp0");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 0", port_bitmap[0], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp1");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 1", port_bitmap[1], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp2");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 2", port_bitmap[2], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp3");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 3", port_bitmap[3], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp4");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 4", port_bitmap[4], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp5");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 5", port_bitmap[5], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp6");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 6", port_bitmap[6], argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pbmp7");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 7", port_bitmap[7], argv[index + 1]);
    }
    if (max_port_num_per_chip > 256)
    {
        index = CTC_CLI_GET_ARGC_INDEX("pbmp8");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("port bitmap 8", port_bitmap[8], argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX("pbmp9");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("port bitmap 9", port_bitmap[9], argv[index + 1]);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip id", lchip, argv[index + 1]);
    }
    if (g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_tagged_ports(g_api_lchip, vlan_id, port_bitmap);
    }
    else
    {
        ret = ctc_vlan_set_tagged_ports(lchip, vlan_id, port_bitmap);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CTC_E_NONE;

}

CTC_CLI(ctc_cli_vlan_set_ptp_clock_id,
        ctc_cli_vlan_set_ptp_clock_id_cmd,
        "vlan VLAN_ID ptp-clock-id CLOCK_ID",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Ptp clock id",
        "<0-3>, 0 is disable")
{
    int32 ret;
    uint16 vlan_id;
    uint32 value;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    CTC_CLI_GET_UINT32("ptp clock id", value, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_PTP_EN, value);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_PTP_EN, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_vlan_class_default_entry,
        ctc_cli_vlan_add_vlan_class_default_entry_cmd,
        "vlan classifier add-mismatch (mac|ipv4|ipv6) (discard|do-nothing|send-to-cpu)",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add vlan classify default entry",
        "Mac based vlan",
        "Ipv4 based vlan",
        "Ipv6 based vlan",
        "Discard packet",
        "Normal forwarding",
        "Redirect to CPU")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_vlan_class_type_t type = 0;
    ctc_vlan_miss_t vlan_mismatch;

    sal_memset(&vlan_mismatch, 0, sizeof(ctc_vlan_miss_t));

    if (CLI_CLI_STR_EQUAL("mac", 0))
    {
        type = CTC_VLAN_CLASS_MAC;
    }
    else if (CLI_CLI_STR_EQUAL("ipv4", 0))
    {
        type = CTC_VLAN_CLASS_IPV4;
    }
    else if (CLI_CLI_STR_EQUAL("ipv6", 0))
    {
        type = CTC_VLAN_CLASS_IPV6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("do-nothing");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
    }

    index = CTC_CLI_GET_ARGC_INDEX("send-to-cpu");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_TO_CPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_default_vlan_class(g_api_lchip, type, &vlan_mismatch);
    }
    else
    {
        ret = ctc_vlan_add_default_vlan_class(type, &vlan_mismatch);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_vlan_class_default_entry,
        ctc_cli_vlan_remove_vlan_class_default_entry_cmd,
        "vlan classifier remove-mismatch (mac|ipv4|ipv6)",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Remove vlan classify default entry",
        "Mac based vlan",
        "IPv4 based vlan",
        "IPv6 based vlan")
{
    int32 ret = 0;
    ctc_vlan_class_type_t type = 0;

    if (CLI_CLI_STR_EQUAL("mac", 0))
    {
        type = CTC_VLAN_CLASS_MAC;
    }
    else if (CLI_CLI_STR_EQUAL("ipv4", 0))
    {
        type = CTC_VLAN_CLASS_IPV4;
    }
    else if (CLI_CLI_STR_EQUAL("ipv6", 0))
    {
        type = CTC_VLAN_CLASS_IPV6;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_default_vlan_class(g_api_lchip, type);
    }
    else
    {
        ret = ctc_vlan_remove_default_vlan_class(type);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_all_vlan_class,
        ctc_cli_vlan_remove_all_vlan_class_cmd,
        "vlan classifier flush-all (mac-vlan|ipv4-vlan|ipv6-vlan|protocol-vlan)",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Flush all entries",
        "Mac based vlan",
        "Ipv4 based vlan",
        "Ipv6 based vlan",
        "Protocol based vlan")
{
    int32 ret;
    ctc_vlan_class_type_t type;

    if (CLI_CLI_STR_EQUAL("mac-vlan", 0))
    {
        type = CTC_VLAN_CLASS_MAC;
    }
    else if (CLI_CLI_STR_EQUAL("ipv4-vlan", 0))
    {
        type = CTC_VLAN_CLASS_IPV4;
    }
    else if (CLI_CLI_STR_EQUAL("ipv6-vlan", 0))
    {
        type = CTC_VLAN_CLASS_IPV6;
    }
    else
    {
        type = CTC_VLAN_CLASS_PROTOCOL;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_all_vlan_class(g_api_lchip, type);
    }
    else
    {
        ret = ctc_vlan_remove_all_vlan_class(type);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_add_vlan_mapping,
        ctc_cli_vlan_add_vlan_mapping_cmd,
        "vlan mapping add port GPORT_ID {svid VLAN_ID (to-svid VLAN_ID |) | cvid VLAN_ID (to-cvid VLAN_ID |) \
        | vrange-grp VRANGE_GROUP_ID | stag-cos COS | ctag-cos COS | macsa MAC | ipv4sa A.B.C.D | ipv6sa X:X::X:X | use-logic-port | default-entry |} \
        mapping-to {new-cvid VLAN_ID | new-scos COS | new-ccos COS | {service-id SERVICEID | policer-id POLICERID |} ((ingress | egress) |) (service-acl-en |) \
        (service-policer-en|) (service-queue-en|) | new-svid VLAN_ID | vlan-domain VLAN_DOMAIN | tpid-index TPID_INDEX |logic-port GPHYPORT_ID (logic-port-type |) \
        | aps-select GROUP_ID (protected-vlan VLAN_ID |) (working-path | protection-path) \
        | (fid FID (vpls-fid ({vpls-learning-dis | maclimit-en | is-leaf} |) |) | nexthop NHID (vpws-nh (is-leaf |) | vlan-switching |)) \
        | stag-op TAG_OP (sl-svid TAG_SL|) (sl-scos TAG_SL|) | ctag-op TAG_OP (sl-cvid TAG_SL|) (sl-ccos TAG_SL|) \
        | user-vlanptr VLAN_PTR | vn-id VN_ID | stp-id STP_ID | (oam L2VPN_OAM_ID) | stats STATS_ID | aging|use-flex | scl-id SCL_ID | igmp-snooping-en | vrf-id VRFID | priority-and-color PRIORITY COLOR | mac-limit-discard-en| learning-disable}",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        "<0-63>",
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 SA",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 SA",
        CTC_CLI_IPV6_FORMAT,
        "Port is logic Port",
        "Default entry",
        "Mapping to",
        "Mapped cvlan",
        "Vlan id value",
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New ctag cos",
        CTC_CLI_COS_RANGE_DESC,
        "Service queue id",
        "Service id",
        "Policer id",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Ingress service",
        "Egress service",
        "Enable service acl",
        "Enable service policer",
        "Enable service queue",
        "Mapped svlan",
        "Vlan id value",
        "Mapped vlan domain",
        CTC_CLI_VLAN_DOMAIN_DESC,
        "Set svlan tpid index",
        "Index, the corresponding svlan tpid is in EpeL2TpidCtl",
        "Logic port",
        "Reference to global config",
        "Logic-port-type",
        "Aps selector",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Protected vlan, maybe working vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "This is working path",
        "This is protection path",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Vpls fid",
        "Disable vsi learning",
        "Enable vsi mac security",
        "It is leaf node in E-Tree networks",
        "Nexthop instance to forward",
        CTC_CLI_NH_ID_STR,
        "Vpws nexthop",
        "It is leaf node in E-Tree networks",
        "Vlan switching",
        "Stag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Svid select",
        CTC_CLI_TAG_SL_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "Ctag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Cvid select",
        CTC_CLI_TAG_SL_DESC,
        "Ccos select",
        CTC_CLI_TAG_SL_DESC,
        "User vlanptr",
        CTC_CLI_USER_VLAN_PTR,
        "Overlay tunnel virtual net ID",
        "<0x1-0xFFFFFF>",
        "STP Id",
        "STP ID value",
        "OAM enable",
        "L2vpn oam id, equal to fid when VPLS",
        "Statistic",
        CTC_CLI_STATS_ID_VAL,
        "Enable aging",
        "Use tcam key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "IGMP-snooping-en",
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Priority and color",
        CTC_CLI_PRIORITY_VALUE,
        CTC_CLI_COLOR_VALUE,
        "Enable mac limit discard",
        "Disable learning")
{
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_vlan_mapping_t vlan_mapping;
    mac_addr_t macsa;
    ipv6_addr_t ipv6_address;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(vlan_mapping.macsa, macsa, sizeof(mac_addr_t));
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_MAC_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("IPv4 sa", vlan_mapping.ipv4_sa, argv[index + 1]);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV4_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("IPv6 sa", ipv6_address, argv[index + 1]);

        vlan_mapping.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
        vlan_mapping.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
        vlan_mapping.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
        vlan_mapping.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV6_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mapping.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-ccos", vlan_mapping.new_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vlan-domain", vlan_mapping.vlan_domain, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tpid-index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("tpid-index", vlan_mapping.tpid_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", vlan_mapping.logic_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT;

        index = CTC_CLI_GET_ARGC_INDEX("logic-port-type");
        if (0xFF != index)
        {
            vlan_mapping.logic_port_type = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("aps select group", vlan_mapping.aps_select_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("protected-vlan");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("aps protected vlan", vlan_mapping.protected_vlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        else
        {
            vlan_mapping.protected_vlan = 0;
        }

        index = CTC_CLI_GET_ARGC_INDEX("working-path");
        if (0xFF != index)
        {
            vlan_mapping.is_working_path = 1;
        }
        else
        {
            vlan_mapping.is_working_path = 0;
        }

        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_APS_SELECT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("fid", vlan_mapping.u3.fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_FID;
        index = CTC_CLI_GET_ARGC_INDEX("vpls-fid");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("vpls-learning-dis");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("maclimit-en");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_MACLIMIT_EN;
        }

        index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_ETREE_LEAF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("nexthop", vlan_mapping.u3.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_NHID;

        index = CTC_CLI_GET_ARGC_INDEX("vpws-nh");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPWS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_ETREE_LEAF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-cvid", vlan_mapping.new_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("service-id", vlan_mapping.service_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("policer-id", vlan_mapping.policer_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-policer-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("service-queue-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_QUEUE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag-op", vlan_mapping.stag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-svid", vlan_mapping.svid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag-op", vlan_mapping.ctag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-cvid", vlan_mapping.cvid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-ccos", vlan_mapping.ccos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VLANPTR;
        CTC_CLI_GET_UINT16_RANGE("user-vlanptr", vlan_mapping.user_vlanptr, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vn-id");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VN_ID;
        CTC_CLI_GET_UINT32_RANGE("vnid", vlan_mapping.u3.vn_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("stp-id");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_STP_ID;
        CTC_CLI_GET_UINT8_RANGE("stp-id", vlan_mapping.stp_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_L2VPN_OAM;
        CTC_CLI_GET_UINT16_RANGE("l2vpn-oam-id", vlan_mapping.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("stats-id", vlan_mapping.stats_id, argv[index + 1]);
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mapping.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-snooping-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_IGMP_SNOOPING_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrf-id");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VRFID;
        CTC_CLI_GET_UINT16("vrf id", vlan_mapping.u3.vrf_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-and-color");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("color_id", vlan_mapping.color, argv[index + 2]);
        CTC_CLI_GET_UINT8("priority_id", vlan_mapping.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-discard-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("learning-disable");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS;
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
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_get_vlan_mapping,
        ctc_cli_vlan_get_vlan_mapping_cmd,
        "show vlan mapping port GPORT_ID ({svid VLAN_ID (to-svid VLAN_ID |) | cvid VLAN_ID (to-cvid VLAN_ID |) \
        | vrange-grp VRANGE_GROUP_ID | stag-cos COS | ctag-cos COS | macsa MAC | ipv4sa A.B.C.D | ipv6sa X:X::X:X | use-logic-port | use-flex} |) \
        (scl-id SCL_ID |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        "<0-63>",
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 SA",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 SA",
        CTC_CLI_IPV6_FORMAT,
        "Port is logic Port",
        "Use tcam key",
        "Scl id",
        "SCL ID")
{
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_vlan_mapping_t vlan_mapping;
    mac_addr_t macsa;
    ipv6_addr_t ipv6_address;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(vlan_mapping.macsa, macsa, sizeof(mac_addr_t));
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_MAC_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("IPv4 sa", vlan_mapping.ipv4_sa, argv[index + 1]);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV4_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("IPv6 sa", ipv6_address, argv[index + 1]);

        vlan_mapping.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
        vlan_mapping.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
        vlan_mapping.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
        vlan_mapping.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV6_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("scl-id", vlan_mapping.scl_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_get_vlan_mapping(gport, &vlan_mapping);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
        return ret;
    }

    ctc_cli_out("\n-----------------------------------------\n");
    ctc_cli_out("Vlan mapping action\n");
    ctc_cli_out("-----------------------------------------\n");
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SVID))
    {
        ctc_cli_out("%-30s: %d\n", "New svlan id", vlan_mapping.new_svid);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SCOS))
    {
        ctc_cli_out("%-30s: %d\n", "New svlan cos", vlan_mapping.new_scos);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_CVID))
    {
        ctc_cli_out("%-30s: %d\n", "New cvlan id", vlan_mapping.new_cvid);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_CCOS))
    {
        ctc_cli_out("%-30s: %d\n", "New cvlan cos", vlan_mapping.new_ccos);
    }
    /*no corresponding action flag to use*/
    if (vlan_mapping.stag_op)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan operation", vlan_mapping.stag_op);
    }
    if (vlan_mapping.svid_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan selection", vlan_mapping.svid_sl);
    }
    if (vlan_mapping.scos_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan cos selection", vlan_mapping.scos_sl);
    }
    if (vlan_mapping.ctag_op)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan operation", vlan_mapping.ctag_op);
    }
    if (vlan_mapping.cvid_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan selection", vlan_mapping.cvid_sl);
    }
    if (vlan_mapping.ccos_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan cos selection", vlan_mapping.ccos_sl);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT))
    {
        ctc_cli_out("%-30s: %d\n", "Logic source port", vlan_mapping.logic_src_port);
        if (vlan_mapping.logic_port_type)
        {
            ctc_cli_out("%-30s: %d\n", "Logic port type", vlan_mapping.logic_port_type);
        }
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_APS_SELECT))
    {
        ctc_cli_out("%-30s: %d\n", "Aps select group id", vlan_mapping.aps_select_group_id);
        if (vlan_mapping.protected_vlan)
        {
            ctc_cli_out("%-30s: %d\n", "Protected vlan", vlan_mapping.protected_vlan);
        }
        if (vlan_mapping.is_working_path)
        {
            ctc_cli_out("%-30s: %s\n", "Working_path", "TRUE");
        }
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_FID))
    {
        ctc_cli_out("%-30s: %d\n", "Fid", vlan_mapping.u3.fid);
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_VPLS))
        {
            ctc_cli_out("VPLS application enable\n");
        }
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS))
        {
            ctc_cli_out("VPLS learning disable\n");
        }
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_MACLIMIT_EN))
        {
            ctc_cli_out("Mac limit enable\n");
        }
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF))
        {
            ctc_cli_out("Leaf node in E-Tree\n");
        }
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_NHID))
    {
        ctc_cli_out("%-30s: %u\n", "Nexthop id", vlan_mapping.u3.nh_id);
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_VPWS))
        {
            ctc_cli_out("VPWS application enable\n");
        }
        if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF))
        {
            ctc_cli_out("Leaf node in E-Tree\n");
        }
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID))
    {
        ctc_cli_out("%-30s: %d\n", "Service id", vlan_mapping.service_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR))
    {
        ctc_cli_out("%-30s: %d\n", "VlanPtr", vlan_mapping.user_vlanptr);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_VN_ID))
    {
        ctc_cli_out("%-30s: %d\n", "Virtual subnet id", vlan_mapping.u3.vn_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN))
    {
        ctc_cli_out("Service acl of HQoS enable\n");
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN))
    {
        ctc_cli_out("Service policer of HQoS enable\n");
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_STATS_EN))
    {
        ctc_cli_out("%-30s: %d\n", "Stats id", vlan_mapping.stats_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_QUEUE_EN))
    {
        ctc_cli_out("Service queue enable\n");
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_STP_ID))
    {
        ctc_cli_out("%-30s: %d\n", "Stp id", vlan_mapping.stp_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_L2VPN_OAM))
    {
        ctc_cli_out("%-30s: %d\n", "L2vpn oam id", vlan_mapping.l2vpn_oam_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN))
    {
        ctc_cli_out("%-30s: %d\n", "Vlan domain", vlan_mapping.vlan_domain);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_VRFID))
    {
        ctc_cli_out("%-30s: %d\n", "Vrfid", vlan_mapping.u3.vrf_id);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_IGMP_SNOOPING_EN))
    {
        ctc_cli_out("IGMP snooping enable\n");
    }
    /*no corresponding action flag to use*/
    if (vlan_mapping.policer_id)
    {
        ctc_cli_out("%-30s: %d\n", "Policer id", vlan_mapping.policer_id);
    }
    if(vlan_mapping.color)
    {
        ctc_cli_out("%-30s: %d\n", "color", vlan_mapping.color);
        ctc_cli_out("%-30s: %d\n", "priority", vlan_mapping.priority);
    }
    return ret;
}


CTC_CLI(ctc_cli_vlan_get_egress_vlan_mapping,
        ctc_cli_vlan_get_egress_vlan_mapping_cmd,
        "show vlan mapping egress port GPORT_ID ({svid VLAN_ID (to-svid VLAN_ID |) | cvid VLAN_ID (to-cvid VLAN_ID |) \
        | vrange-grp VRANGE_GROUP_ID | stag-cos COS | ctag-cos COS | use-logic-port} |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Egress",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        "<0-63>",
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Port is logic Port")
{
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_egress_vlan_mapping_t vlan_mapping;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_EGRESS_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS;
    }


    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_egress_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_get_egress_vlan_mapping(gport, &vlan_mapping);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
        return ret;
    }

    ctc_cli_out("\n-----------------------------------------\n");
    ctc_cli_out("Egress Vlan mapping action\n");
    ctc_cli_out("-----------------------------------------\n");
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID))
    {
        ctc_cli_out("%-30s: %d\n", "New svlan id", vlan_mapping.new_svid);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS))
    {
        ctc_cli_out("%-30s: %d\n", "New svlan cos", vlan_mapping.new_scos);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID))
    {
        ctc_cli_out("%-30s: %d\n", "New cvlan id", vlan_mapping.new_cvid);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS))
    {
        ctc_cli_out("%-30s: %d\n", "New cvlan cos", vlan_mapping.new_ccos);
    }
    if (CTC_FLAG_ISSET(vlan_mapping.action, CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCFI))
    {
        ctc_cli_out("%-30s: %d\n", "New svlan cfi", vlan_mapping.new_scfi);
    }
    /*no corresponding action flag to use*/
    if (vlan_mapping.stag_op)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan operation", vlan_mapping.stag_op);
    }
    if (vlan_mapping.svid_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan selection", vlan_mapping.svid_sl);
    }
    if (vlan_mapping.scos_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan cos selection", vlan_mapping.scos_sl);
    }
    if (vlan_mapping.scfi_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Svlan cfi selection", vlan_mapping.scfi_sl);
    }
    if (vlan_mapping.ctag_op)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan operation", vlan_mapping.ctag_op);
    }
    if (vlan_mapping.cvid_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan selection", vlan_mapping.cvid_sl);
    }
    if (vlan_mapping.ccos_sl)
    {
        ctc_cli_out("%-30s: %d\n", "Cvlan cos selection", vlan_mapping.ccos_sl);
    }
    /*no corresponding action flag to use*/
    if(vlan_mapping.color)
    {
        ctc_cli_out("%-30s: %d\n", "color", vlan_mapping.color);
        ctc_cli_out("%-30s: %d\n", "priority", vlan_mapping.priority);
    }
    return ret;
}


CTC_CLI(ctc_cli_vlan_update_vlan_mapping,
        ctc_cli_vlan_update_vlan_mapping_cmd,
        "vlan mapping update port GPORT_ID {svid VLAN_ID (to-svid VLAN_ID |) | cvid VLAN_ID (to-cvid VLAN_ID |) \
        | vrange-grp VRANGE_GROUP_ID | stag-cos COS | ctag-cos COS | macsa MAC | ipv4sa A.B.C.D | ipv6sa X:X::X:X  | default-entry | } \
        mapping-to {new-cvid VLAN_ID | new-scos COS | new-ccos COS | {service-id SERVICEID | policer-id POLICERID |} ((ingress | egress) |) (service-acl-en |) \
        (service-policer-en|) (service-queue-en|) | new-svid VLAN_ID | vlan-domain VLAN_DOMAIN | tpid-index TPID_INDEX |logic-port GPHYPORT_ID (logic-port-type |) \
        | aps-select GROUP_ID (protected-vlan VLAN_ID |) (working-path | protection-path) \
        | (fid FID (vpls-fid ({vpls-learning-dis | maclimit-en | is-leaf} |) |) | nexthop NHID (vpws-nh (is-leaf |) | vlan-switching |)) \
        | stag-op TAG_OP (sl-svid TAG_SL|) (sl-scos TAG_SL|) | ctag-op TAG_OP (sl-cvid TAG_SL|) (sl-ccos TAG_SL|) \
        | user-vlanptr VLAN_PTR | vn-id VN_ID | stp-id STP_ID | (oam L2VPN_OAM_ID) | stats STATS_ID | aging | use-flex | scl-id SCL_ID | mac-limit-discard-en| learning-disable}",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Update all actions in vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        "<0-63>",
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 SA",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 SA",
        CTC_CLI_IPV6_FORMAT,
        "Default entry",
        "Mapping to",
        "Mapped cvlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New ctag cos",
        CTC_CLI_COS_RANGE_DESC,
        "Service queue id",
        "Service id",
        "Policer id",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Ingress service ",
        "Egress service ",
        "Enable service acl",
        "Enable service policer",
        "Enable service queue",
        "Mapped svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Mapped vlan domain",
        CTC_CLI_VLAN_DOMAIN_DESC,
        "Set svlan tpid index",
        "Index, the corresponding svlan tpid is in EpeL2TpidCtl",
        "Logic port",
        "Reference to global config",
        "Logic-port-type",
        "Aps selector",
        CTC_CLI_APS_GROUP_ID_DESC,
        "Protected vlan, maybe working vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "This is working path",
        "This is protection path",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Vpls fid",
        "Disable vsi learning",
        "Enable vsi mac security",
        "It is leaf node in E-Tree networks",
        "Nexthop instance to forward",
        CTC_CLI_NH_ID_STR,
        "Vpws nexthop",
        "It is leaf node in E-Tree networks",
        "Vlan switching",
        "Stag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Svid select",
        CTC_CLI_TAG_SL_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "Ctag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Cvid select",
        CTC_CLI_TAG_SL_DESC,
        "Ccos select",
        CTC_CLI_TAG_SL_DESC,
        "User vlanptr",
        CTC_CLI_USER_VLAN_PTR,
        "Overlay tunnel virtual net ID",
        "<0x1-0xFFFFFFFF>",
        "STP Id",
        "STP ID value",
        "OAM enable",
        "L2vpn oam id, equal to fid when VPLS",
        "Statistic",
        "0~0xFFFFFFFF",
        "Enable aging",
        "Use tcam key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Enable mac limit discard",
        "Disable learning")
{
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_vlan_mapping_t vlan_mapping;
    mac_addr_t macsa;
    ipv6_addr_t ipv6_address;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(vlan_mapping.macsa, macsa, sizeof(mac_addr_t));
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_MAC_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("IPv4 sa", vlan_mapping.ipv4_sa, argv[index + 1]);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV4_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("IPv6 sa", ipv6_address, argv[index + 1]);

        vlan_mapping.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
        vlan_mapping.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
        vlan_mapping.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
        vlan_mapping.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV6_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mapping.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-ccos", vlan_mapping.new_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vlan-domain", vlan_mapping.vlan_domain, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VLAN_DOMAIN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tpid-index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("tpid-index", vlan_mapping.tpid_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", vlan_mapping.logic_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT;

        index = CTC_CLI_GET_ARGC_INDEX("logic-port-type");
        if (0xFF != index)
        {
            vlan_mapping.logic_port_type = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("aps select group", vlan_mapping.aps_select_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("protected-vlan");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("aps protected vlan", vlan_mapping.protected_vlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
        else
        {
            vlan_mapping.protected_vlan = 0;
        }

        index = CTC_CLI_GET_ARGC_INDEX("working-path");
        if (0xFF != index)
        {
            vlan_mapping.is_working_path = 1;
        }
        else
        {
            vlan_mapping.is_working_path = 0;
        }

        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_APS_SELECT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("fid", vlan_mapping.u3.fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_FID;
        index = CTC_CLI_GET_ARGC_INDEX("vpls-fid");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("vpls-learning-dis");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("maclimit-en");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_MACLIMIT_EN;
        }

        index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_ETREE_LEAF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("nexthop", vlan_mapping.u3.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_NHID;

        index = CTC_CLI_GET_ARGC_INDEX("vpws-nh");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPWS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("is-leaf");
        if (0xFF != index)
        {
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_ETREE_LEAF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-cvid", vlan_mapping.new_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("service-id", vlan_mapping.service_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SERVICE_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("policer-id", vlan_mapping.policer_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-policer-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("service-queue-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_SERVICE_QUEUE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag-op", vlan_mapping.stag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-svid", vlan_mapping.svid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag-op", vlan_mapping.ctag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-cvid", vlan_mapping.cvid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-ccos", vlan_mapping.ccos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VLANPTR;
        CTC_CLI_GET_UINT16_RANGE("user-vlanptr", vlan_mapping.user_vlanptr, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vn-id");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_VN_ID;
        CTC_CLI_GET_UINT32_RANGE("vnid", vlan_mapping.u3.vn_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("stp-id");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_STP_ID;
        CTC_CLI_GET_UINT8_RANGE("stp-id", vlan_mapping.stp_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_L2VPN_OAM;
        CTC_CLI_GET_UINT16_RANGE("l2vpn-oam-id", vlan_mapping.l2vpn_oam_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("stats-id", vlan_mapping.stats_id, argv[index + 1]);
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mapping.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-discard-en");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("learning-disable");
    if (0xFF != index)
    {
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_update_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_update_vlan_mapping(gport, &vlan_mapping);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_vlan_mapping,
        ctc_cli_vlan_remove_vlan_mapping_cmd,
        "vlan mapping remove port GPORT_ID (((({svid VLAN_ID (to-svid VLAN_ID |) | cvid VLAN_ID(to-cvid VLAN_ID |)} (vrange-grp VRANGE_GROUP_ID |) |) ({stag-cos COS | ctag-cos COS} |) ({macsa MAC | ipv4sa A.B.C.D | ipv6sa X:X::X:X} |)) {use-flex|scl-id SCL_ID|}) (use-logic-port|) |)(default-entry |) (scl-id SCL_ID |)",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Remove vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "802.1ad svlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "802.1ad cvlan, or start vlan of vlan range",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        "<0-63>",
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 SA",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 SA",
        CTC_CLI_IPV6_FORMAT,
        "Use tcam key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Port is logic Port",
        "Default entry",
        "scl id",
        "scl id value")
{
    uint8 index = 0xFF;
    int32 ret = 0;
    uint16 gport = 0;
    mac_addr_t macsa;
    ctc_vlan_mapping_t vlan_mapping;
    ipv6_addr_t ipv6_address;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(vlan_mapping.macsa, macsa, sizeof(mac_addr_t));
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_MAC_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("IPv4 sa", vlan_mapping.ipv4_sa, argv[index + 1]);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV4_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6sa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("IPv6 sa", ipv6_address, argv[index + 1]);

        vlan_mapping.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
        vlan_mapping.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
        vlan_mapping.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
        vlan_mapping.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_IPV6_SA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mapping.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mapping.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
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
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_vlan_mapping_default_entry,
        ctc_cli_vlan_add_vlan_mapping_default_entry_cmd,
        "vlan mapping add port GPORT_ID mismatch (discard|do-nothing|send-to-cpu|service-id SERVICEID (ingress|egress)|user-vlanptr VLAN_PTR|append-stag new-svid VLAN_ID (sl-scos TAG_SL|) (new-scos COS|)) \
        (scl-id SCL_ID|)",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Mismatch action",
        "Discard packet",
        "Normal forwarding",
        "Redirect to CPU",
        "Service queue id",
        "Service id",
        "Ingress service queue",
        "Egress service queue",
        "User vlanptr",
        CTC_CLI_USER_VLAN_PTR,
        "Append stag",
        "New svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "scl id",
        "scl id value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 gport = 0;
    ctc_vlan_miss_t vlan_mismatch;

    sal_memset(&vlan_mismatch, 0, sizeof(ctc_vlan_miss_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("do-nothing");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
    }

    index = CTC_CLI_GET_ARGC_INDEX("send-to-cpu");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_TO_CPU;
    }

#if 0
    index = CTC_CLI_GET_ARGC_INDEX("default-svlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mismatch.svlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DEF_SVLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-cvlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mismatch.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DEF_CVLAN;
    }

#endif

    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");
    if (0xFF != index)
    {
        vlan_mismatch.flag |= CTC_VLAN_MISS_ACTION_OUTPUT_VLANPTR;
        CTC_CLI_GET_UINT16_RANGE("user-vlanptr", vlan_mismatch.user_vlanptr, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("append-stag");
    if (0xFF != index)
    {
        vlan_mismatch.flag |= CTC_VLAN_MISS_ACTION_APPEND_STAG;
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mismatch.new_svid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mismatch.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mismatch.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mismatch.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_default_vlan_mapping(g_api_lchip, gport, &vlan_mismatch);
    }
    else
    {
        ret = ctc_vlan_add_default_vlan_mapping(gport, &vlan_mismatch);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_vlan_mapping_default_entry,
        ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd,
        "vlan mapping remove port GPORT_ID mismatch",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Remove vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Default entry")
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_default_vlan_mapping(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_vlan_remove_default_vlan_mapping(gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_all_vlan_mapping,
        ctc_cli_vlan_remove_all_vlan_mapping_cmd,
        "vlan mapping flush-all port GPORT_ID",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Flush all vlan mapping hash entries by port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_all_vlan_mapping_by_port(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_vlan_remove_all_vlan_mapping_by_port(gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_all_egress_vlan_mapping,
        ctc_cli_vlan_remove_all_egress_vlan_mapping_cmd,
        "vlan mapping egress flush-all port GPORT_ID",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Egress",
        "Flush all vlan mapping entries by port",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_all_egress_vlan_mapping_by_port(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_vlan_remove_all_egress_vlan_mapping_by_port(gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_egress_vlan_mapping,
        ctc_cli_vlan_add_egress_vlan_mapping_cmd,
        "vlan mapping egress add port GPORT_ID  { svid VLAN_ID (to-svid VLAN_ID| ) (stag-cos COS|) | cvid VLAN_ID (to-cvid VLAN_ID | ) (ctag-cos COS | ) | vrange-grp VRANGE_GROUP_ID | use-logic-port | default-entry |} " \
        "mapping-to {new-cvid VLAN_ID|new-svid VLAN_ID|new-scos COS|new-ccos COS|new-scfi CFI|stag-op TAG_OP  (sl-svid TAG_SL|) (sl-scos TAG_SL|) {sl-scfi TAG_SL|tpid-index TPID_INDEX|} |" \
        "ctag-op TAG_OP  (sl-cvid TAG_SL|) (sl-ccos TAG_SL|) | aging | priority-and-color PRIORITY COLOR}",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Use egress vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Port is logic Port",
        "Default entry",
        "Mapping to",
        "Mapped cvlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Mapped svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New ctag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New stag cfi",
        CTC_CLI_CFI_RANGE_DESC,
        "Stag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Svid select",
        CTC_CLI_TAG_SL_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "Scfi select",
        CTC_CLI_TAG_SL_DESC,
        "Set svlan tpid index",
        "Index, the corresponding svlan tpid is in EpeL2TpidCtl",
        "Ctag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Cvid select",
        CTC_CLI_TAG_SL_DESC,
        "Ccos select",
        CTC_CLI_TAG_SL_DESC,
        "Enable aging",
        "Priority and color",
        CTC_CLI_PRIORITY_VALUE,
        CTC_CLI_COLOR_VALUE)
{
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_egress_vlan_mapping_t vlan_mapping;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_EGRESS_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-cvid", vlan_mapping.new_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mapping.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-ccos", vlan_mapping.new_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scfi");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scfi", vlan_mapping.new_scfi, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCFI;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag-op", vlan_mapping.stag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-svid", vlan_mapping.svid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scfi");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scfi_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tpid-index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("tpid-index", vlan_mapping.tpid_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ctag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag-op", vlan_mapping.ctag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-cvid", vlan_mapping.cvid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-ccos", vlan_mapping.ccos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-and-color");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("color_id", vlan_mapping.color, argv[index + 2]);
        CTC_CLI_GET_UINT8("priority_id", vlan_mapping.priority, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_egress_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_add_egress_vlan_mapping(gport, &vlan_mapping);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_egress_vlan_mapping,
        ctc_cli_vlan_remove_egress_vlan_mapping_cmd,
        "vlan mapping egress remove port GPORT_ID { svid VLAN_ID (to-svid VLAN_ID| ) (stag-cos COS|) | cvid VLAN_ID (to-cvid VLAN_ID | ) (ctag-cos COS | ) | vrange-grp VRANGE_GROUP_ID | use-logic-port | default-entry | } ",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Use egress vlan mapping",
        "Remove vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "802.1ad svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "802.1ad cvlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range, end vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Port is logic Port",
        "Default entry")
{
    uint8 index = 0xFF;
    uint8 stag_cos = 0;
    uint8 ctag_cos = 0;
    uint16 svid = 0;
    uint16 cvid = 0;
    int32 ret = 0;
    uint16 gport = 0;
    ctc_egress_vlan_mapping_t vlan_mapping;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_EGRESS_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", svid, argv[index + 1], 0, 4095);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", cvid, argv[index + 1], 0, 4095);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end svlan", vlan_mapping.svlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("end cvlan", vlan_mapping.cvlan_end, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("vrange-grp", vlan_mapping.vrange_grpid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-entry");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", stag_cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", ctag_cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS;
    }

    vlan_mapping.old_svid = (svid & 0xFFF);
    vlan_mapping.old_cvid = (cvid & 0xFFF);

    vlan_mapping.old_scos = stag_cos;
    vlan_mapping.old_ccos = ctag_cos;


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_egress_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
    }
    else
    {
        ret = ctc_vlan_remove_egress_vlan_mapping(gport, &vlan_mapping);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_egress_vlan_mapping_default_entry,
        ctc_cli_vlan_add_egress_vlan_mapping_default_entry_cmd,
        "vlan mapping egress add port GPORT_ID mismatch (discard|do-nothing)",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Use egress vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Mismatch action",
        "Dicard packet",
        "Normal forwaring")
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 gport = 0;
    ctc_vlan_miss_t vlan_mismatch;

    sal_memset(&vlan_mismatch, 0, sizeof(ctc_vlan_miss_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("do-nothing");
    if (0xFF != index)
    {
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
    }

#if 0
    index = CTC_CLI_GET_ARGC_INDEX("default-svlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mismatch.svlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DEF_SVLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-cvlan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mismatch.cvlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mismatch.flag = CTC_VLAN_MISS_ACTION_DEF_CVLAN;
    }

#endif


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_default_egress_vlan_mapping(g_api_lchip, gport, &vlan_mismatch);
    }
    else
    {
        ret = ctc_vlan_add_default_egress_vlan_mapping(gport, &vlan_mismatch);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_egress_vlan_mapping_default_entry,
        ctc_cli_vlan_remove_egress_vlan_mapping_default_entry_cmd,
        "vlan mapping egress remove port GPORT_ID mismatch",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Use egress vlan mapping",
        "Remove vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Default entry")
{
    int32 ret = 0;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_default_egress_vlan_mapping(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_vlan_remove_default_egress_vlan_mapping(gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_create_vlan_vrange,
        ctc_cli_vlan_create_vlan_vrange_cmd,
        "vlan range create vrange-grp GROUP_ID direction (ingress|egress) (is-svlan|is-cvlan)",
        CTC_CLI_VLAN_M_STR,
        "Vlan range",
        "Create",
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Is svlan range",
        "Is cvlan range")
{
    int32 ret;
    uint8  vrange_grpid = 0;
    bool is_svlan = FALSE;
    ctc_direction_t dir = 0;
    ctc_vlan_range_info_t vrange_info;

    CTC_CLI_GET_UINT8_RANGE("vlan range groupid", vrange_grpid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    if (CLI_CLI_STR_EQUAL("is-svlan", 2))
    {
        is_svlan = TRUE;
    }
    else if (CLI_CLI_STR_EQUAL("is-cvlan", 2))
    {
        is_svlan = FALSE;
    }

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = vrange_grpid;

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_create_vlan_range_group(g_api_lchip, &vrange_info, is_svlan);
    }
    else
    {
        ret = ctc_vlan_create_vlan_range_group(&vrange_info, is_svlan);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_destroy_vlan_vrange,
        ctc_cli_vlan_destroy_vlan_vrange_cmd,
        "vlan range destroy vrange-grp GROUP_ID direction (ingress|egress) ",
        CTC_CLI_VLAN_M_STR,
        "Vlan range",
        "Destroy",
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret;
    uint8  vrange_grpid = 0;
    ctc_direction_t dir = 0;
    ctc_vlan_range_info_t vrange_info;

    CTC_CLI_GET_UINT8_RANGE("vlan range groupid", vrange_grpid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = vrange_grpid;

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_destroy_vlan_range_group(g_api_lchip, &vrange_info);
    }
    else
    {
        ret = ctc_vlan_destroy_vlan_range_group(&vrange_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_add_vlan_vrange,
        ctc_cli_vlan_add_vlan_vrange_cmd,
        "vlan range add  MINVID to MAXVID vrange-grp GROUP_ID direction (ingress|egress) (is-acl|)",
        CTC_CLI_VLAN_M_STR,
        "Vlan range",
        "Add",
        CTC_CLI_VLAN_RANGE_DESC,
        "To",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Is acl vlan range")
{
    int32 ret;
    uint16 vlan_start = 0;
    uint16 vlan_end = 0;
    uint8  vrange_grpid = 0;
    uint8  index = 0;
    ctc_direction_t dir = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t vlan_range;

    CTC_CLI_GET_UINT16_RANGE("vlan start", vlan_start, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vlan end", vlan_end, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("vlan range groupid", vrange_grpid, argv[2], 0, CTC_MAX_UINT8_VALUE);
    if (CLI_CLI_STR_EQUAL("ingress", 3))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 3))
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-acl");
    if (0xFF != index)
    {
        vlan_range.is_acl = 1;
    }
    else
    {
        vlan_range.is_acl = 0;
    }

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = vrange_grpid;
    vlan_range.vlan_start = vlan_start;
    vlan_range.vlan_end = vlan_end;
    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_range(g_api_lchip, &vrange_info, &vlan_range);
    }
    else
    {
        ret = ctc_vlan_add_vlan_range(&vrange_info, &vlan_range);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_remove_vlan_vrange,
        ctc_cli_vlan_remove_vlan_vrange_cmd,
        "vlan range remove MINVID to MAXVID vrange-grp GROUP_ID direction (ingress|egress) (is-acl|)",
        CTC_CLI_VLAN_M_STR,
        "Vlan range",
        "Remove",
        CTC_CLI_VLAN_RANGE_DESC,
        "To",
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Is acl vlan range")
{
    int32 ret;
    uint16 vlan_start = 0;
    uint16 vlan_end = 0;
    uint8  vrange_grpid = 0;
    uint8  index = 0;
    ctc_direction_t dir = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_t vlan_range;

    CTC_CLI_GET_UINT16_RANGE("vlan start", vlan_start, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vlan end", vlan_end, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("vlan range groupid", vrange_grpid, argv[2], 0, CTC_MAX_UINT8_VALUE);

    if (CLI_CLI_STR_EQUAL("ingress", 3))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 3))
    {
        dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-acl");
    if (0xFF != index)
    {
        vlan_range.is_acl = 1;
    }
    else
    {
        vlan_range.is_acl = 0;
    }

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = vrange_grpid;
    vlan_range.vlan_start = vlan_start;
    vlan_range.vlan_end = vlan_end;

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_range(g_api_lchip, &vrange_info, &vlan_range);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_range(&vrange_info, &vlan_range);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_get_vlan_vrange,
        ctc_cli_vlan_get_vlan_vrange_cmd,
        "show vlan range group GROUP_ID direction (ingress|egress)",
        "Do show",
        CTC_CLI_VLAN_M_STR,
        "Vlan range",
        "Vlan range group id",
        CTC_CLI_VLAN_RANGE_GRP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret;
    uint8  vrange_grpid = 0;
    uint8 idx = 0;
    ctc_direction_t dir = 0;
    ctc_vlan_range_info_t vrange_info;
    ctc_vlan_range_group_t vrange_group;
    uint8 count = 0;
    bool is_svlan = FALSE;

    CTC_CLI_GET_UINT8_RANGE("vlan range groupid", vrange_grpid, argv[0], 0, CTC_MAX_UINT8_VALUE);
    if (CLI_CLI_STR_EQUAL("ingress", 1))
    {
        dir = CTC_INGRESS;
    }
    else if (CLI_CLI_STR_EQUAL("egress", 1))
    {
        dir = CTC_EGRESS;
    }

    vrange_info.direction = dir;
    vrange_info.vrange_grpid = vrange_grpid;

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_vlan_range(g_api_lchip, &vrange_info, &vrange_group, &count);
    }
    else
    {
        ret = ctc_vlan_get_vlan_range(&vrange_info, &vrange_group, &count);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }
    else
    {
        ctc_cli_out("%-13s%-10s%-11s%s\n", "VRANGE_GRP", "DIR", "IS_SVLAN", "VALID_CNT");
        ctc_cli_out("-------------------------------------------\n");
        ctc_cli_out("%-13u", vrange_grpid);

        if (CTC_INGRESS == dir)
        {
            ctc_cli_out("%-10s", "INGRESS");
        }
        else
        {
            ctc_cli_out("%-10s", "EGRESS");
        }

        if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_vlan_range_type(g_api_lchip, &vrange_info, &is_svlan);
    }
    else
    {
        ret = ctc_vlan_get_vlan_range_type(&vrange_info, &is_svlan);
    }

        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        }
        else
        {
            ctc_cli_out("%-11s", is_svlan ? "YES" : "NO");
        }

        ctc_cli_out("%u\n", count);

        ctc_cli_out("\nMember Range\n");
        ctc_cli_out("--------------------------\n");
        ctc_cli_out("%-6s%-7s%-7s%s\n", "No.", "MIN", "MAX","Is-acl");
        for (idx = 0; idx < count; idx++)
        {
            ctc_cli_out("%-6u%-07u%-07u%u\n", idx, vrange_group.vlan_range[idx].vlan_start, vrange_group.vlan_range[idx].vlan_end, vrange_group.vlan_range[idx].is_acl);
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_vlan_add_protocol_vlan,
        ctc_cli_vlan_add_protocol_vlan_cmd,
        "vlan classifier add protocol ether-type ETHER_TYPE vlan VLAN_ID ((cos COS)|)",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add one vlan classification entry",
        "Protocol based vlan",
        "Ethertype",
        "Legal ether type",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Cos",
        CTC_CLI_COS_RANGE_DESC)
{
    int32 ret = 0;
    int32 index = 0xFF;
    uint16  ether_type = 0;
    ctc_vlan_class_t pro_vlan;

    sal_memset(&pro_vlan, 0, sizeof(ctc_vlan_class_t));
    pro_vlan.type = CTC_VLAN_CLASS_PROTOCOL;

    CTC_CLI_GET_UINT16_RANGE("ether-type", ether_type, argv[0], 0, CTC_MAX_UINT16_VALUE);
    pro_vlan.vlan_class.ether_type = ether_type;

    CTC_CLI_GET_UINT16_RANGE("protocol vlan id", pro_vlan.vlan_id, argv[1], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("cos", pro_vlan.cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        pro_vlan.flag |= CTC_VLAN_CLASS_FLAG_OUTPUT_COS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_class(g_api_lchip, &pro_vlan);
    }
    else
    {
        ret = ctc_vlan_add_vlan_class(&pro_vlan);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));

        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_protocol_vlan,
        ctc_cli_vlan_remove_protocol_vlan_cmd,
        "vlan classifier remove protocol ether-type ETHER_TYPE",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Remove one vlan classification entry",
        "Protocol based vlan",
        "Ethertype",
        "Legal ether type")
{
    int32 ret = 0;
    uint16 ether_type = 0;
    ctc_vlan_class_t pro_vlan;

    sal_memset(&pro_vlan, 0, sizeof(ctc_vlan_class_t));
    pro_vlan.type = CTC_VLAN_CLASS_PROTOCOL;

    CTC_CLI_GET_UINT16_RANGE("ether-type", ether_type, argv[0], 0, CTC_MAX_UINT16_VALUE);
    pro_vlan.vlan_class.ether_type = ether_type;

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_class(g_api_lchip, &pro_vlan);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_class(&pro_vlan);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));

        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_mac_vlan_class,
        ctc_cli_vlan_add_mac_vlan_class_cmd,
        "vlan classifier add mac (macsa MAC| macda MAC) \
        vlan VLAN_ID {cos COS|use-flex|resolve-conflict|scl-id SCL_ID|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add one entry",
        "Mac based vlan",
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Cos",
        CTC_CLI_COS_RANGE_DESC,
        "Use tcam key",
        "Resolve conflict",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret;
    uint8 index = 0xFF;
    uint16 vlan_id;
    ctc_vlan_class_t vlan_class;
    mac_addr_t macsa;
    mac_addr_t macda;
    uint8 idx = 0;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));
    vlan_class.type = CTC_VLAN_CLASS_MAC;

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(&(vlan_class.vlan_class.mac), &macsa, sizeof(mac_addr_t));
        idx = idx + 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("dest mac address", macda, argv[index + 1]);

        sal_memcpy(&(vlan_class.vlan_class.mac), &macda, sizeof(mac_addr_t));
        idx = idx + 2;
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_MACDA;
    }

    CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_id, argv[idx], 0, CTC_MAX_UINT16_VALUE);
    vlan_class.vlan_id = vlan_id;

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("cos", vlan_class.cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_OUTPUT_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_add_vlan_class(&vlan_class);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_mac_vlan_class,
        ctc_cli_vlan_remove_mac_vlan_class_cmd,
        "vlan classifier remove mac (macsa MAC|macda MAC) {use-flex|resolve-conflict|scl-id SCL_ID|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Remove one entry",
        "Mac based vlan",
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Use tcam key",
        "Resolve conflict",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret;
    uint8 index = 0xFF;
    ctc_vlan_class_t vlan_class;
    mac_addr_t macsa;
    mac_addr_t macda;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));

    vlan_class.type = CTC_VLAN_CLASS_MAC;

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", macsa, argv[index + 1]);

        sal_memcpy(&(vlan_class.vlan_class.mac), &macsa, sizeof(mac_addr_t));
    }

    index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("dest mac address", macda, argv[index + 1]);

        sal_memcpy(&(vlan_class.vlan_class.mac), &macda, sizeof(mac_addr_t));
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_MACDA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_add_ipv4_vlan_class,
        ctc_cli_vlan_add_ipv4_vlan_class_cmd,
        "vlan classifier add ipv4 {ip-sa A.B.C.D (mask A.B.C.D|)|ip-da A.B.C.D (mask A.B.C.D|)|mac-sa MAC|mac-da MAC|l3-type (arp|ipv4|L3TYPE)|l4-type (udp|tcp|L4TYPE) \
        |l4-srcport L4PORT |l4-destport L4PORT} vlan VLAN_ID {cos COS|scl-id SCL_ID|use-flex|resolve-conflict|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add one vlan classification entry",
        "IPv4 based vlan",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 source mask",
        CTC_CLI_IPV4_MASK_FORMAT,
        "IPv4 destination address",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 destination mask",
        CTC_CLI_IPV4_MASK_FORMAT,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Layer3 type",
        "Layer3 type ARP",
        "Layer3 type IPv4",
        "Layer3 type id: 0~15",
        "Layer4 type",
        "Layer4 type UDP",
        "Layer4 type TCP",
        "Layer4 type id: 0~15",
        "Layer4 source port",
        "<0-65535>",
        "Layer4 destination port",
        "<0-65535>",
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Cos",
        CTC_CLI_COS_RANGE_DESC,
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Use tcam key",
        "Resolve conflict")
{

    uint8 index = 0xFF;
    uint8 elem = 0;
    uint8 idx = 0;
    uint16 vlan_id = 0;
    int32 ret = 0;
    mac_addr_t macsa;
    mac_addr_t macda;
    ctc_vlan_class_t vlan_class;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));
    vlan_class.type = CTC_VLAN_CLASS_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (0xFF != index)
    {
        elem = elem | 0x01;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (0xFF != index)
    {
        elem = elem | 0x02;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != index)
    {
        elem = elem | 0x04;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != index)
    {
        elem = elem | 0x08;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3-type");
    if (0xFF != index)
    {
        elem = elem | 0x10;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != index)
    {
        elem = elem | 0x20;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
    if (0xFF != index)
    {
        elem = elem | 0x40;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
    if (0xFF != index)
    {
        elem = elem | 0x80;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    if (0x01 != elem)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
        if (0 != (elem & 0x01))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            idx = idx + 2;
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
            if (CLI_CLI_STR_EQUAL("mask", index + 2))
            {
                idx = idx + 2;
                CTC_CLI_GET_IPV4_ADDRESS("ipv4 sa mask", vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, argv[index + 3]);
            }
        }

        if (0 != (elem & 0x02))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-da");
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 destination address", vlan_class.vlan_class.flex_ipv4.ipv4_da, argv[index + 1]);
            idx = idx + 2;
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_da_mask, 0xFF, sizeof(ip_addr_t));
            if (CLI_CLI_STR_EQUAL("mask", index + 2))
            {
                CTC_CLI_GET_IPV4_ADDRESS("ipv4 da mask", vlan_class.vlan_class.flex_ipv4.ipv4_da_mask, argv[index + 3]);
                idx = idx + 2;
            }
        }

        if (0 != (elem & 0x04))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.macsa), &macsa, sizeof(mac_addr_t));
            idx = idx + 2;
        }

        if (0 != (elem & 0x08))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-da");
            CTC_CLI_GET_MAC_ADDRESS("mac-da", macda, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.macda), &macda, sizeof(mac_addr_t));
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("L4port", vlan_class.vlan_class.flex_ipv4.l4src_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("L4port", vlan_class.vlan_class.flex_ipv4.l4dest_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l3-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("ipv4", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l3_type = CTC_PARSER_L3_TYPE_IPV4;
            }
            else if (CLI_CLI_STR_EQUAL("arp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l3_type = CTC_PARSER_L3_TYPE_ARP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer3 type", vlan_class.vlan_class.flex_ipv4.l3_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }

            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("udp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l4_type = CTC_PARSER_L4_TYPE_UDP;
            }
            else if (CLI_CLI_STR_EQUAL("tcp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l4_type = CTC_PARSER_L4_TYPE_TCP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer4 type", vlan_class.vlan_class.flex_ipv4.l4_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }

            idx = idx + 2;
        }
    }
    else if (0x01 == elem)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
        if (CLI_CLI_STR_EQUAL("mask", index + 2))
        {
            vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            idx = idx + 2;
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 sa mask", vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, argv[index + 3]);
            idx = idx + 2;
        }
        else if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
            idx = idx + 2;
        }
        else
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.ipv4_sa, argv[index + 1]);
            idx = idx + 2;
        }
    }

    CTC_CLI_GET_UINT16_RANGE("vlan_class id", vlan_id, argv[idx], 0, CTC_MAX_UINT16_VALUE);
    vlan_class.vlan_id = vlan_id;

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("cos", vlan_class.cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_OUTPUT_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_add_vlan_class(&vlan_class);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_ipv4_vlan_class,
        ctc_cli_vlan_remove_ipv4_vlan_class_cmd,
        "vlan classifier remove ipv4 {ip-sa A.B.C.D (mask A.B.C.D|)|ip-da A.B.C.D (mask A.B.C.D|)|mac-sa MAC|mac-da MAC|l3-type (arp|ipv4|L3TYPE)|l4-type (udp|tcp|L4TYPE) \
        |l4-srcport L4PORT |l4-destport L4PORT} {scl-id SCL_ID|use-flex|resolve-conflict|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Remove one vlan classification entry",
        "IPv4 based vlan",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 source mask",
        CTC_CLI_IPV4_MASK_FORMAT,
        "IPv4 destination address",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 destination mask",
        CTC_CLI_IPV4_MASK_FORMAT,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Layer3 type",
        "Layer3 type ARP",
        "Layer3 type IPv4",
        "Layer3 type id: 0~15",
        "Layer4 type",
        "Layer4 type UDP",
        "Layer4 type TCP",
        "Layer4 type id: 0~15",
        "Layer4 source port",
        "<0-65535>",
        "Layer4 destination port",
        "<0-65535>",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Use tcam key",
        "Resolve conflict")
{
    uint8 index = 0xFF;
    uint8 elem = 0;
    int32 ret = 0;
    mac_addr_t macsa;
    mac_addr_t macda;
    ctc_vlan_class_t vlan_class;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));
    vlan_class.type = CTC_VLAN_CLASS_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (0xFF != index)
    {
        elem = elem | 0x01;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (0xFF != index)
    {
        elem = elem | 0x02;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != index)
    {
        elem = elem | 0x04;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != index)
    {
        elem = elem | 0x08;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3-type");
    if (0xFF != index)
    {
        elem = elem | 0x10;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != index)
    {
        elem = elem | 0x20;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
    if (0xFF != index)
    {
        elem = elem | 0x40;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
    if (0xFF != index)
    {
        elem = elem | 0x80;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    if (0x01 != elem)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
        if (0 != (elem & 0x01))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
            if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
            {
                CTC_CLI_GET_IPV4_ADDRESS("ipv4 sa mask", vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, argv[index + 3]);
            }
        }

        if (0 != (elem & 0x02))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-da");
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 destination address", vlan_class.vlan_class.flex_ipv4.ipv4_da, argv[index + 1]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_da_mask, 0xFF, sizeof(ip_addr_t));
            if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
            {
                CTC_CLI_GET_IPV4_ADDRESS("ipv4 da mask", vlan_class.vlan_class.flex_ipv4.ipv4_da_mask, argv[index + 3]);
            }
        }

        if (0 != (elem & 0x04))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.macsa), &macsa, sizeof(mac_addr_t));
        }

        if (0 != (elem & 0x08))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-da");
            CTC_CLI_GET_MAC_ADDRESS("mac-da", macda, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv4.macda), &macda, sizeof(mac_addr_t));
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 source port", vlan_class.vlan_class.flex_ipv4.l4src_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 destination port", vlan_class.vlan_class.flex_ipv4.l4dest_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("l3-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("ipv4", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l3_type = CTC_PARSER_L3_TYPE_IPV4;
            }
            else if (CLI_CLI_STR_EQUAL("arp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l3_type = CTC_PARSER_L3_TYPE_ARP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer3 type", vlan_class.vlan_class.flex_ipv4.l3_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("udp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l4_type = CTC_PARSER_L4_TYPE_UDP;
            }
            else if (CLI_CLI_STR_EQUAL("tcp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv4.l4_type = CTC_PARSER_L4_TYPE_TCP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer4 type", vlan_class.vlan_class.flex_ipv4.l4_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }
        }
    }
    else if (0x01 == elem)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
        if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
        {
            vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 sa mask", vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, argv[index + 3]);
        }
        else if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.flex_ipv4.ipv4_sa, argv[index + 1]);
            sal_memset(&vlan_class.vlan_class.flex_ipv4.ipv4_sa_mask, 0xFF, sizeof(ip_addr_t));
        }
        else
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.ipv4_sa, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}
#if 1
CTC_CLI(ctc_cli_vlan_add_ipv6_vlan_class,
        ctc_cli_vlan_add_ipv6_vlan_class_cmd,
        "vlan classifier add ipv6 {ip-sa X:X::X:X (mask X:X::X:X|)|ip-da X:X::X:X (mask X:X::X:X|)|mac-sa MAC|mac-da MAC|l4-srcport L4PORT |l4-destport L4PORT| \
        l4-type (udp|tcp|L4TYPE)} vlan VLAN_ID {cos COS|scl-id SCL_ID|use-flex|resolve-conflict|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add one vlan classification entry",
        "IPv6 based vlan",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 source mask",
        CTC_CLI_IPV6_MASK_FORMAT,
        "IPv6 destination address",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 destination mask",
        CTC_CLI_IPV6_MASK_FORMAT,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Layer4 source port",
        "<0-65535>",
        "Layer4 destination port",
        "<0-65535>",
        "Layer4 type",
        "Layer4 type UDP",
        "Layer4 type TCP",
        "Layer4 type id: 0~15",
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Cos",
        CTC_CLI_COS_RANGE_DESC,
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Use tcam key",
        "Resolve conflict")
{
    uint8 index = 0xFF;
    uint8 elem = 0;
    int32 ret = 0;
    uint16 vlan_id = 0;
    ctc_vlan_class_t vlan_class;
    ipv6_addr_t ipv6_address;
    mac_addr_t macsa;
    mac_addr_t macda;
    uint8 idx = 0;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));
    vlan_class.type = CTC_VLAN_CLASS_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (0xFF != index)
    {
        elem = elem | 0x01;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (0xFF != index)
    {
        elem = elem | 0x02;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != index)
    {
        elem = elem | 0x04;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != index)
    {
        elem = elem | 0x08;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != index)
    {
        elem = elem | 0x10;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
    if (0xFF != index)
    {
        elem = elem | 0x20;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
    if (0xFF != index)
    {
        elem = elem | 0x40;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    if (0x01 != elem)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
        if (0 != (elem & 0x01))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            idx = idx + 2;

            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
            if (CLI_CLI_STR_EQUAL("mask", index + 2))
            {
                CTC_CLI_GET_IPV6_ADDRESS("ipv6 sa mask", ipv6_address, argv[index + 3]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = sal_htonl(ipv6_address[0]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = sal_htonl(ipv6_address[1]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = sal_htonl(ipv6_address[2]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = sal_htonl(ipv6_address[3]);
                idx = idx + 2;
            }
        }

        if (0 != (elem & 0x02))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-da");
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 destination address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[3] = sal_htonl(ipv6_address[3]);
            idx = idx + 2;

            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_da_mask, 0xFF, sizeof(ipv6_addr_t));
            if (CLI_CLI_STR_EQUAL("mask", index + 2))
            {
                CTC_CLI_GET_IPV6_ADDRESS("ipv6 da mask", ipv6_address, argv[index + 3]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[0] = sal_htonl(ipv6_address[0]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[1] = sal_htonl(ipv6_address[1]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[2] = sal_htonl(ipv6_address[2]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[3] = sal_htonl(ipv6_address[3]);
                idx = idx + 2;
            }
        }

        if (0 != (elem & 0x04))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv6.macsa), &macsa, sizeof(mac_addr_t));
            idx = idx + 2;
        }

        if (0 != (elem & 0x08))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-da");
            CTC_CLI_GET_MAC_ADDRESS("mac-da", macda, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv6.macda), &macda, sizeof(mac_addr_t));
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 source port", vlan_class.vlan_class.flex_ipv6.l4src_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 destination port", vlan_class.vlan_class.flex_ipv6.l4dest_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
            idx = idx + 2;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("udp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv6.l4_type = CTC_PARSER_L4_TYPE_UDP;
            }
            else if (CLI_CLI_STR_EQUAL("tcp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv6.l4_type = CTC_PARSER_L4_TYPE_TCP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer4 type", vlan_class.vlan_class.flex_ipv6.l4_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }

            idx = idx + 2;
        }
    }
    else if (0x01 == elem)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
        if (CLI_CLI_STR_EQUAL("mask", index + 2))
        {
            vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            idx = idx + 2;
            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 sa mask", ipv6_address, argv[index + 3]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = sal_htonl(ipv6_address[3]);
            idx = idx + 2;

        }
        else if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
            idx = idx + 2;
        }
        else
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            idx = idx + 2;
        }
    }

    CTC_CLI_GET_UINT16_RANGE("vlan_class id", vlan_id, argv[idx], 0, CTC_MAX_UINT16_VALUE);
    vlan_class.vlan_id = vlan_id;

    index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("cos", vlan_class.cos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_OUTPUT_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_add_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_add_vlan_class(&vlan_class);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_remove_ipv6_vlan_class,
        ctc_cli_vlan_remove_ipv6_vlan_class_cmd,
        "vlan classifier remove ipv6 {ip-sa X:X::X:X (mask X:X::X:X|)|ip-da X:X::X:X (mask X:X::X:X|)|mac-sa MAC|mac-da MAC|l4-srcport L4PORT |l4-destport L4PORT| \
    l4-type (udp|tcp|L4TYPE)} {scl-id SCL_ID|use-flex|resolve-conflict|}",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Remove one vlan classification entry",
        "IPv6 based vlan",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 source mask",
        CTC_CLI_IPV6_MASK_FORMAT,
        "IPv6 destination address",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 destination mask",
        CTC_CLI_IPV6_MASK_FORMAT,
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MACDA_DESC,
        CTC_CLI_MAC_FORMAT,
        "Layer4 source port",
        "<0-65535>",
        "Layer4 destination port",
        "<0-65535>",
        "Layer4 type",
        "Layer4 type UDP",
        "Layer4 type TCP",
        "Layer4 type id: 0~15",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Use tcam key",
        "Resolve conflict")
{
    uint8 index = 0xFF;
    uint8 elem = 0;
    int32 ret = 0;
    ctc_vlan_class_t vlan_class;
    ipv6_addr_t ipv6_address;
    mac_addr_t macsa;
    mac_addr_t macda;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&macsa, 0, sizeof(mac_addr_t));
    sal_memset(&macda, 0, sizeof(mac_addr_t));
    vlan_class.type = CTC_VLAN_CLASS_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (0xFF != index)
    {
        elem = elem | 0x01;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (0xFF != index)
    {
        elem = elem | 0x02;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != index)
    {
        elem = elem | 0x04;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != index)
    {
        elem = elem | 0x08;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != index)
    {
        elem = elem | 0x10;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
    if (0xFF != index)
    {
        elem = elem | 0x20;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
    if (0xFF != index)
    {
        elem = elem | 0x40;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
    }

    if (0x01 != elem)
    {
        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
        if (0 != (elem & 0x01))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);

            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
            if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
            {
                CTC_CLI_GET_IPV6_ADDRESS("ipv6 sa mask", ipv6_address, argv[index + 3]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = sal_htonl(ipv6_address[0]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = sal_htonl(ipv6_address[1]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = sal_htonl(ipv6_address[2]);
                vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = sal_htonl(ipv6_address[3]);
            }
        }

        if (0 != (elem & 0x02))
        {
            index = CTC_CLI_GET_ARGC_INDEX("ip-da");
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 destination address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_da[3] = sal_htonl(ipv6_address[3]);

            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_da_mask, 0xFF, sizeof(ipv6_addr_t));
            if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
            {
                CTC_CLI_GET_IPV6_ADDRESS("ipv6 da mask", ipv6_address, argv[index + 3]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[0] = sal_htonl(ipv6_address[0]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[1] = sal_htonl(ipv6_address[1]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[2] = sal_htonl(ipv6_address[2]);
                vlan_class.vlan_class.flex_ipv6.ipv6_da_mask[3] = sal_htonl(ipv6_address[3]);
            }
        }

        if (0 != (elem & 0x04))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv6.macsa), &macsa, sizeof(mac_addr_t));
        }

        if (0 != (elem & 0x08))
        {
            index = CTC_CLI_GET_ARGC_INDEX("mac-da");
            CTC_CLI_GET_MAC_ADDRESS("mac-da", macda, argv[index + 1]);

            sal_memcpy(&(vlan_class.vlan_class.flex_ipv6.macda), &macda, sizeof(mac_addr_t));
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-srcport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 source port", vlan_class.vlan_class.flex_ipv6.l4src_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-destport");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("layer4 destination port", vlan_class.vlan_class.flex_ipv6.l4dest_port, argv[index + 1],
                                     0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("l4-type");
        if (0xFF != index)
        {
            if (CLI_CLI_STR_EQUAL("udp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv6.l4_type = CTC_PARSER_L4_TYPE_UDP;
            }
            else if (CLI_CLI_STR_EQUAL("tcp", index + 1))
            {
                vlan_class.vlan_class.flex_ipv6.l4_type = CTC_PARSER_L4_TYPE_TCP;
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("layer4 type", vlan_class.vlan_class.flex_ipv6.l4_type, argv[index + 1],
                                        0, CTC_MAX_UINT8_VALUE);
            }
        }
    }
    else if (0x01 == elem)
    {
        index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
        if (((index + 2) < argc) && (CLI_CLI_STR_EQUAL("mask", index + 2)))
        {
            vlan_class.flag |= CTC_VLAN_CLASS_FLAG_USE_FLEX;
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 sa mask", ipv6_address, argv[index + 3]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask[3] = sal_htonl(ipv6_address[3]);

        }
        else if(CTC_FLAG_ISSET(vlan_class.flag, CTC_VLAN_CLASS_FLAG_USE_FLEX))
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.flex_ipv6.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
            sal_memset(vlan_class.vlan_class.flex_ipv6.ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));
        }
        else
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 1]);
            vlan_class.vlan_class.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
            vlan_class.vlan_class.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
            vlan_class.vlan_class.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
            vlan_class.vlan_class.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_class.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");
    if (0xFF != index)
    {

        vlan_class.flag |= CTC_VLAN_CLASS_FLAG_RESOLVE_CONFLICT;

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_remove_vlan_class(g_api_lchip, &vlan_class);
    }
    else
    {
        ret = ctc_vlan_remove_vlan_class(&vlan_class);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}
#endif

CTC_CLI(ctc_cli_vlan_set_dir_property,
        ctc_cli_vlan_set_dir_property_cmd,
        "vlan VLAN_ID dir-property  \
        ( acl-en | acl-vlan-classid | acl-vlan-classid-1 | acl-vlan-classid-2 | acl-vlan-classid-3 \
        | acl-tcam-type-0 | acl-tcam-type-1 | acl-tcam-type-2 | acl-tcam-type-3 \
        | acl-routed-pkt-only | stats-id) \
        direction (ingress|egress|both) value VALUE ",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan property with direction",
        CTC_CLI_ACL_ENABLE_BITMAP,
        "Vlan acl vlan classid",
        "Vlan acl vlan classid1",
        "Vlan acl vlan classid2",
        "Vlan acl vlan classid3",
        "Vlan acl tcam type 0",
        "Vlan acl tcam type 1",
        "Vlan acl tcam type 2",
        "Vlan acl tcam type 3",
        "Vlan acl against routed-packet only",
        "Vlan stats-id",
        "Flow direction",
        "Ingress",
        "Egress",
        "Both direction",
        "Property value",
        "Value")
{
    int32 ret = 0;
    uint16 vid = 0;
    uint32 value = 0;
    ctc_direction_t dir = CTC_INGRESS;
    uint32  prop = 0;

    CTC_CLI_GET_UINT16("vlan_id", vid, argv[0]);
    CTC_CLI_GET_UINT32("Property_value", value, argv[3]);

    if CLI_CLI_STR_EQUAL("in", 2)
    {
        dir = CTC_INGRESS;
    }
    else if CLI_CLI_STR_EQUAL("eg", 2)
    {
        dir = CTC_EGRESS;
    }
    else if CLI_CLI_STR_EQUAL("bo", 2)
    {
        dir = CTC_BOTH_DIRECTION;
    }

    if CLI_CLI_STR_EQUAL("acl-en", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_EN;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-vlan-classid-1", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_CLASSID_1;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-vlan-classid-2", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_CLASSID_2;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-vlan-classid-3", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_CLASSID_3;
    }
    else if CTC_CLI_STR_EQUAL_ENHANCE("acl-vlan-classid", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_CLASSID;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-0", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-1", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-2", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2;
    }
    else if CLI_CLI_STR_EQUAL("acl-tcam-type-3", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3;
    }
    else if CLI_CLI_STR_EQUAL("acl-routed-pkt-only", 1)
    {
        prop = CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY;
    }

    else if CLI_CLI_STR_EQUAL("stats-id", 1)
    {
        prop = CTC_VLAN_DIR_PROP_VLAN_STATS_ID;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_direction_property(g_api_lchip, vid, prop, dir, value);
    }
    else
    {
        ret = ctc_vlan_set_direction_property(vid, prop, dir, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_show_dir_property,
        ctc_cli_vlan_show_dir_property_cmd,
        "show vlan VLAN_ID dir-property \
        ( all | acl-en | acl-vlan-classid | acl-vlan-classid-1 | acl-vlan-classid-2 | acl-vlan-classid-3 \
        | acl-tcam-type-0 | acl-tcam-type-1 | acl-tcam-type-2 | acl-tcam-type-3 \
        | acl-routed-pkt-only | stats-id) \
        direction (ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan property with direction",
        "All property",
        CTC_CLI_ACL_ENABLE_BITMAP,
        "Vlan acl vlan classid",
        "Vlan acl vlan classid1",
        "Vlan acl vlan classid2",
        "Vlan acl vlan classid3",
        "Vlan acl tcam type 0",
        "Vlan acl tcam type 1",
        "Vlan acl tcam type 2",
        "Vlan acl tcam type 3",
        "Vlan acl against routed-packet only",
        "Vlan stats-id",
        "Flow direction",
        "Ingress",
        "Egress")
{
    int32 ret = 0;
    uint16 vid;
    uint32 value = 0;
    ctc_direction_t dir = 0;
    uint8 index = 0xFF;
    uint8 show_all = 0;
    ctc_stats_basic_t stats_result;

    sal_memset(&stats_result, 0, sizeof(stats_result));

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        show_all = 1;
    }

    CTC_CLI_GET_UINT16("vlan_id", vid, argv[0]);

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

    ctc_cli_out("%-24s%s\n", "Property", "Value");
    ctc_cli_out("------------------------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("acl-en");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_EN, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_EN, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-en:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-vlan-classid");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_CLASSID, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_CLASSID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-vlan-classid:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-vlan-classid-1");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_1, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_1, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-vlan-classid-1:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-vlan-classid-2");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_2, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_2, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-vlan-classid-2:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-vlan-classid-3");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_3, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_CLASSID_3, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-vlan-classid-3:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-0");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_0, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-tcam-type-0:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-1");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_1, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-tcam-type-1:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-2");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_2, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-tcam-type-2:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-type-3");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_TCAM_LKUP_TYPE_3, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-tcam-type-3:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-routed-pkt-only");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_ACL_ROUTED_PKT_ONLY, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "acl-routed-pkt-only0:", value);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats-id");
    if (INDEX_VALID(index) || show_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_direction_property(g_api_lchip, vid, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, dir, &value);
        }
        else
        {
            ret = ctc_vlan_get_direction_property(vid, CTC_VLAN_DIR_PROP_VLAN_STATS_ID, dir, &value);
        }
        if (ret >= 0)
        {
            ctc_cli_out("%-24s%d\n", "vlan stats-id:", value);
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

CTC_CLI(ctc_cli_vlan_debug_on,
        ctc_cli_vlan_debug_on_cmd,
        "debug vlan (vlan|vlan-class|vlan-mapping|protocol-vlan)(ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_VLAN_M_STR,
        "Basic Vlan sub module",
        "Vlan class sub mdoule",
        "Vlan mapping sub module",
        "Protocol vlan sub module",
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    char sub[16];
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
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

    sal_memset(sub, 0, sizeof(char) * 16);
    sal_strcpy(sub, argv[0]);

    if ('-' == sub[4])
    {
        sub[4] = '_';
    }

    if ('-' == sub[5])
    {
        sub[5] = '_';
    }

    if ('-' == sub[8])
    {
        sub[8] = '_';
    }

    if (0 == sal_memcmp(argv[1], "ctc", 3))
    {
        typeenum = 0;
    }
    else
    {
        typeenum = 1;
    }

    ctc_debug_set_flag("vlan", sub, typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_vlan_debug_off,
        ctc_cli_vlan_debug_off_cmd,
        "no debug vlan (vlan|vlan-class|vlan-mapping|protocol-vlan) (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan sub module",
        "Vlan class sub mdoule",
        "Vlan mapping sub module",
        "Protocol vlan sub module",
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    char sub[16];
    uint8 level = 0;

    sal_memset(sub, 0, sizeof(char) * 16);
    sal_strcpy(sub, argv[0]);

    if ('-' == sub[4])
    {
        sub[4] = '_';
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = 0;
    }
    else
    {
        typeenum = 1;
    }

    ctc_debug_set_flag("vlan", sub, typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_vlan_show_vlan_info,
        ctc_cli_vlan_show_vlan_info_cmd,
        "show vlan VLAN_ID (member-port | receive | transmit | bridge | mac-learning | fid | stpid| igmp-snooping \
        | arp | dhcp | fip | tagged-port |ipv4-l2mc |ipv6-l2mc |ptp |drop-unknown-ucast | drop-unknown-mcast |drop-unknown-bcast |unknown-mcast-copy-tocpu | mac-auth |all) (gchip GCHIP_ID|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan member port",
        "Vlan receive state",
        "Vlan transmit state",
        "Vlan bridge state",
        "Vlan learning state",
        "Fid of vlan",
        "Stp instance in vlan",
        "IGMP snooping",
        "ARP exception action",
        "DHCP exception action",
        "FIP exception action",
        "Vlan tagged port",
        "Ipv4 based l2mc",
        "Ipv6 based l2mc",
        "ptp",
        "Drop unknown ucast",
        "Drop unknown mcast",
        "Drop unknown bcast",
        "Unknown mcast packet copy to cpu",
        "MAC authentication",
        "All",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_port_bitmap_t port_bitmap = {0};
    ctc_port_bitmap_t port_bitmap_tagged = {0};
    uint16 vlan_id;
    uint8  gchip = 0;
    uint16 bit_cnt;
    uint16 port_count = 0;
    uint32 value = 0;
    bool val = 0;
    uint16 fid_val = 0;
    uint8 stpid_val = 0;
    uint8 print_port = 0;
    uint8 is_all = 0;
    uint16 portid = 0;
    uint32 max_port_num_per_chip = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF != index)
    {
        is_all = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("gchip id", gchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("member-port");
    if ((0xFF != index) || is_all)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_ports(g_api_lchip, vlan_id, gchip, port_bitmap);
        }
        else
        {
            ret = ctc_vlan_get_ports(vlan_id, gchip, port_bitmap);
        }

        print_port = 1;
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (is_all)
        {
            ctc_cli_out("%-8s%-8s%-8s%s\n", "No.", "Chip", "Member", "Tagged(gport)");
            ctc_cli_out("-------------------------------\n");

            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_tagged_ports(g_api_lchip, vlan_id, gchip, port_bitmap_tagged);
            }
            else
            {
                ret = ctc_vlan_get_tagged_ports(vlan_id, gchip, port_bitmap_tagged);
            }
            print_port = 1;
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));

                return CLI_ERROR;
            }

            for (bit_cnt = 0; bit_cnt < max_port_num_per_chip; bit_cnt++)
            {
                portid = bit_cnt;
                if (CTC_BMP_ISSET(port_bitmap, portid))
                {
                    if (CTC_BMP_ISSET(port_bitmap_tagged, portid))
                    {
                        ctc_cli_out("%-8u%-8u0x%04X  %s\n", port_count, gchip, CTC_MAP_LPORT_TO_GPORT(gchip, portid), "Tagged");
                    }
                    else
                    {
                        ctc_cli_out("%-8u%-8u0x%04X  %s\n", port_count, gchip, CTC_MAP_LPORT_TO_GPORT(gchip, portid), "Untagged");
                    }
                    port_count++;
                }
            }

            ctc_cli_out("-------------------------------\n");
            ctc_cli_out("member port count:%u\n\n", port_count);
        }
        else
        {
            ctc_cli_out("%-8s%-8s%s\n","No.","Chip","Member(gport)");
            ctc_cli_out("------------------------\n");
            for (bit_cnt = 0; bit_cnt < max_port_num_per_chip; bit_cnt++)
            {
                portid = bit_cnt;
                if (CTC_BMP_ISSET(port_bitmap, portid))
                {
                    ctc_cli_out("%-8u0x%-8X0x%04X\n", port_count, gchip, CTC_MAP_LPORT_TO_GPORT(gchip, portid));
                    port_count++;
                }
            }

            ctc_cli_out("------------------------\n");
            ctc_cli_out("member port count:%u\n\n", port_count);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("tagged-port");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_vlan_get_tagged_ports(g_api_lchip, vlan_id, gchip, port_bitmap);
        }
        else
        {
            ret = ctc_vlan_get_tagged_ports(vlan_id, gchip, port_bitmap);
        }

        print_port = 1;
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));

            return CLI_ERROR;
        }

        ctc_cli_out("%-8s%-8s%s\n", "No.", "chip", "Tagged_Member(gport)");
        ctc_cli_out("-------------------------------\n");

        for (bit_cnt = 0; bit_cnt < max_port_num_per_chip; bit_cnt++)
        {
            portid = bit_cnt;
            if (CTC_BMP_ISSET(port_bitmap, portid))
            {
                ctc_cli_out("%-8u%-8u0x%04X\n", port_count, gchip, CTC_MAP_LPORT_TO_GPORT(gchip, portid));
                port_count++;
            }
        }

        ctc_cli_out("-------------------------------\n");
        ctc_cli_out("tagged port count:%u\n\n", port_count);
    }

    if (!print_port || is_all)
    {
        ctc_cli_out("%-24s%s\n", "Property", "Value");
        ctc_cli_out("---------------------------------------\n");

        index = CTC_CLI_GET_ARGC_INDEX("receive");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_receive_en(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_receive_en(vlan_id, &val);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "receive en", val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("transmit");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_transmit_en(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_transmit_en(vlan_id, &val);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-24s%d\n", "transmit en", val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("bridge");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_bridge_en(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_bridge_en(vlan_id, &val);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "bridge en", val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("mac-learning");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_learning_en(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_learning_en(vlan_id, &val);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "learning en", val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("fid");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_fid(g_api_lchip, vlan_id, &fid_val);
            }
            else
            {
                ret = ctc_vlan_get_fid(vlan_id, &fid_val);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%u\n", "fid", fid_val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("stpid");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_stp_get_vlan_stpid(g_api_lchip, vlan_id, &stpid_val);
            }
            else
            {
                ret = ctc_stp_get_vlan_stpid(vlan_id, &stpid_val);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%u\n", "stpid", stpid_val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("igmp-snooping");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_igmp_snoop_en(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_igmp_snoop_en(vlan_id, &val);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "igmp snooping", val);
        }

        index = CTC_CLI_GET_ARGC_INDEX("arp");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_arp_excp_type(g_api_lchip, vlan_id, (ctc_exception_type_t*)&value);
            }
            else
            {
                ret = ctc_vlan_get_arp_excp_type(vlan_id, (ctc_exception_type_t*)&value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%s\n", "arp exception",
                        ((CTC_EXCP_FWD_AND_TO_CPU == value) ? "FWD_AND_CPU"
                        :(CTC_EXCP_NORMAL_FWD == value) ? "NORMAL_FWD"
                        : (CTC_EXCP_DISCARD_AND_TO_CPU == value) ? "DISCARD_AND_CPU"
                        : "DISCARD"));
        }

        index = CTC_CLI_GET_ARGC_INDEX("dhcp");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_dhcp_excp_type(g_api_lchip, vlan_id, (ctc_exception_type_t*)&value);
            }
            else
            {
                ret = ctc_vlan_get_dhcp_excp_type(vlan_id, (ctc_exception_type_t*)&value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%s\n", "dhcp exception",
                        ((CTC_EXCP_FWD_AND_TO_CPU == value) ? "FWD_AND_CPU"
                        :(CTC_EXCP_NORMAL_FWD == value) ? "NORMAL_FWD"
                        : (CTC_EXCP_DISCARD_AND_TO_CPU == value) ? "DISCARD_AND_CPU"
                        : "DISCARD"));
        }

        index = CTC_CLI_GET_ARGC_INDEX("fip");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_FIP_EXCP_TYPE, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_FIP_EXCP_TYPE, &value);
            }

            /*if show all info do not return error although ret < 0*/
            if (ret < 0 && !is_all)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            else if(ret >= 0)
            {
                ctc_cli_out("%-24s%s\n", "fip exception",
                        ((CTC_EXCP_FWD_AND_TO_CPU == value) ? "FWD_AND_CPU"
                        :(CTC_EXCP_NORMAL_FWD == value) ? "NORMAL_FWD"
                        : (CTC_EXCP_DISCARD_AND_TO_CPU == value) ? "DISCARD_AND_CPU"
                        : "DISCARD"));
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv4-l2mc");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_IPV4_BASED_L2MC, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_IPV4_BASED_L2MC, &value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "ipv4 based l2mc enable", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-l2mc");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_IPV6_BASED_L2MC, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_IPV6_BASED_L2MC, &value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "ipv6 based l2mc enable", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ptp");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_PTP_EN, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_PTP_EN, &value);
            }
            /*if show all info do not return error although ret < 0*/
            if (ret < 0 && !is_all)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            else if(ret >= 0)
            {
                ctc_cli_out("%-24s%d\n", "ptp clock id", value);
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("drop-unknown-ucast");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_UCAST_EN, &value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "drop unknown ucast", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("drop-unknown-mcast");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_MCAST_EN, &value);
            }

            if (ret < 0)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "drop unknown mcast", value);
        }

        index = CTC_CLI_GET_ARGC_INDEX("drop-unknown-bcast");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_DROP_UNKNOWN_BCAST_EN, &value);
            }

            if (ret < 0 && !is_all)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            else if(ret >= 0)
            {
                ctc_cli_out("%-24s%d\n", "drop unknown bcast", value);
            }

        }

        index = CTC_CLI_GET_ARGC_INDEX("unknown-mcast-copy-tocpu");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU, &value);
            }
            else
            {
                ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_UNKNOWN_MCAST_COPY_TO_CPU, &value);
            }

            if (ret < 0 && !is_all)
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            else if(ret >= 0)
            {
                ctc_cli_out("%-24s%d\n", "unknown mcast to cpu", value);
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("mac-auth");
        if ((0xFF != index) || is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_mac_auth(g_api_lchip, vlan_id, &val);
            }
            else
            {
                ret = ctc_vlan_get_mac_auth(vlan_id, &val);
            }

            if (ret < 0 && !is_all )
            {
                ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            ctc_cli_out("%-24s%d\n", "mac authentication", val);
        }

        if (is_all)
        {
            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_direction_property(g_api_lchip, vlan_id, CTC_VLAN_DIR_PROP_ACL_EN, CTC_INGRESS, &value);
            }
            else
            {
                ret = ctc_vlan_get_direction_property(vlan_id, CTC_VLAN_DIR_PROP_ACL_EN, CTC_INGRESS, &value);
            }

            if (ret >= 0)
            {
                ctc_cli_out("%-24s%d\n","ingress acl en", value);
            }

            if (g_ctcs_api_en)
            {
                 ret = ctcs_vlan_get_direction_property(g_api_lchip, vlan_id, CTC_VLAN_DIR_PROP_ACL_EN, CTC_EGRESS, &value);
            }
            else
            {
                ret = ctc_vlan_get_direction_property(vlan_id, CTC_VLAN_DIR_PROP_ACL_EN, CTC_EGRESS, &value);
            }

            if (ret >= 0)
            {
                ctc_cli_out("%-24s%d\n","egress acl en", value);
            }
        }

        ctc_cli_out("\n");
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_mac_auth,
        ctc_cli_vlan_set_mac_auth_cmd,
        "vlan VLAN_ID mac-auth (enable | disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan base MAC authentication",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = FALSE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("en", argv[1], 2))
    {
        is_enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_mac_auth(g_api_lchip, vlan_id, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_mac_auth(vlan_id, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


int32
ctc_vlan_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_create_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_create_vlan_with_vlanptr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_member_ports_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_member_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_receive_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_transmit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_bridge_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_stpid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_fid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_learning_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_igmp_snooping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_arp_dhcp_excp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_drop_unknown_pkt_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_ipv4_l2mc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_ipv6_l2mc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_dir_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_show_dir_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_port_tag_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_ports_tag_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_ptp_clock_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_mac_auth_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_show_vlan_info_cmd);

    return CLI_SUCCESS;
}

int32
ctc_advanced_vlan_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_vlan_class_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_vlan_class_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_all_vlan_class_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_get_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_update_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_vlan_mapping_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_vlan_mapping_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_all_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_egress_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_get_egress_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_egress_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_egress_vlan_mapping_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_egress_vlan_mapping_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_all_egress_vlan_mapping_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_vlan_vrange_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_vlan_vrange_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_get_vlan_vrange_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_create_vlan_vrange_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_destroy_vlan_vrange_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_protocol_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_protocol_vlan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_mac_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_mac_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_ipv4_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_ipv4_vlan_class_cmd);
#if 1
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_add_ipv6_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_remove_ipv6_vlan_class_cmd);
#endif

    return CLI_SUCCESS;
}

