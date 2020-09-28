/**
 @file ctc_l2_cli.c

 @date 2009-10-30

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_l3if_cli.h"
#include "ctc_l3if.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_debug.h"

/****************************************************************************
 *
* Define
*
*****************************************************************************/
char *prop_str[MAX_CTC_L3IF_PROP_NUM] = {"Route-en",
                                                "Pbr-label",
                                                "Nat-iftype",
                                                "Route-all-pkt",
                                                "IPv4-ucast-en",
                                                "IPv4-mcast-en",
                                                "IPv4-sa-type",
                                                "IPv6-ucast-en",
                                                "IPv6-mcast-en",
                                                "IPv6-sa-type",
                                                "VRFID",
                                                "MTU-en",
                                                "MTU-size",
                                                "MTU-exception-en",
                                                "Route-mac-prefix-type",
                                                "Route-mac-low-8bits",
                                                "Egs-macsa-prefix-type",
                                                "Egs-macsa-low-8bits",
                                                "Egs-mcast-ttl-threshold",
                                                "MPLS-en",
                                                "MPLS-label-space",
                                                "VRF-en",
                                                "IGMP-snooping-en",
                                                "RPF-check-type",
                                                "Contextlabel-exist",
                                                "RPF-permit-default",
                                                "Igs stats ptr",
                                                "Egs stats ptr",
                                                "Category Id",
                                                "Public lookup enable",
                                                "Enable vlan of ipmc",
                                                "Enable interface lable",
                                                "Phb use tunnel hdr",
                                                "Trust dscp",
                                                "Dscp select mode",
                                                "Ingress qos dscp domain",
                                                "Egress qos dscp domain",
                                                "Default dscp"

                                                };

/****************************************************************************
 *
* Function
*
*****************************************************************************/

CTC_CLI(ctc_cli_l3if_show_l3if_id,
        ctc_cli_l3if_show_l3if_id_cmd,
        "show l3if type (phy-if gport GPORT_ID | vlan-if vlan VLAN_ID |sub-if vlan VLAN_ID gport GPORT_ID | l3if-id L3IF_ID)",
        "Show",
        CTC_CLI_L3IF_STR,
        "L3 interface type",
        "Physical interface",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Vlan interface",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Sub interface",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC)
{
    int32 ret = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    if (CLI_CLI_STR_EQUAL("phy-if",0))
    {
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
        CTC_CLI_GET_UINT32("gport", l3if.gport, argv[2]);
    }
    else if (CLI_CLI_STR_EQUAL("vlan-if",0))
    {
        l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
        CTC_CLI_GET_UINT16("vlan_id", l3if.vlan_id, argv[2]);
    }
    else if (CLI_CLI_STR_EQUAL("sub-if",0))
    {
        l3if.l3if_type = CTC_L3IF_TYPE_SUB_IF;
        CTC_CLI_GET_UINT16("vlan_id", l3if.vlan_id, argv[2]);
        CTC_CLI_GET_UINT32("gport", l3if.gport, argv[4]);
    }
    else if (CLI_CLI_STR_EQUAL("l3if-id",0))
    {
        l3if.l3if_type = 0xFF;
        CTC_CLI_GET_UINT16("l3if_id", l3if_id, argv[1]);
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_get_l3if_id(g_api_lchip, &l3if, &l3if_id);
    }
    else
    {
        ret = ctc_l3if_get_l3if_id(&l3if, &l3if_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        if(CLI_CLI_STR_EQUAL("l3if-id",0))
            {
            switch(l3if.l3if_type)
            {
            case CTC_L3IF_TYPE_PHY_IF:
                ctc_cli_out("L3if type:  Phy interface \n");
                ctc_cli_out("Gport:  0x%x \n", l3if.gport);
                break;
            case CTC_L3IF_TYPE_VLAN_IF:
                ctc_cli_out("L3if type:  Vlan interface \n");
                ctc_cli_out("Vlan:   %d \n", l3if.vlan_id);
                break;
            case CTC_L3IF_TYPE_SUB_IF:
                ctc_cli_out("L3if type:  Sub interface \n");
                ctc_cli_out("Vlan:   %d \n", l3if.vlan_id);
                ctc_cli_out("Gport:  0x%x \n", l3if.gport);
                break;
            case CTC_L3IF_TYPE_SERVICE_IF:
                ctc_cli_out("L3if type:  Service interface \n");
                break;
            }
        }
        else
        {
            ctc_cli_out("Get l3if_id:  %d \n", l3if_id);
        }
    }

    return ret;
}


CTC_CLI(ctc_cli_l3if_create_or_destroy_l3_phy_if,
        ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd,
        "l3if (create | destroy) ifid IFID type phy-if gport GPORT_ID",
        CTC_CLI_L3IF_STR,
        "Create l3 interface",
        "Destroy l3 interface",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "L3 interface type",
        "Physical interface",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("gport", l3if.gport, argv [2], 0, CTC_MAX_UINT32_VALUE);

    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;


    if (0 == sal_memcmp(argv[0], "cr", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_create(l3if_id, &l3if);
        }
    }
    else if (0 == sal_memcmp(argv[0], "de", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_destory(l3if_id, &l3if);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_create_or_destroy_l3_vlan_if,
        ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd,
        "l3if (create | destroy) ifid IFID type vlan-if vlan VLAN_ID",
        CTC_CLI_L3IF_STR,
        "Create l3 interface",
        "Destroy l3 interface",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "L3 interface type",
        "Vlan interface",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vlan_id", l3if.vlan_id, argv [2], 0, CTC_MAX_UINT16_VALUE);

    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;

    if (0 == sal_memcmp(argv[0], "cr", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_create(l3if_id, &l3if);
        }
    }
    else if (0 == sal_memcmp(argv[0], "de", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_destory(l3if_id, &l3if);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_create_or_destroy_l3_sub_if,
        ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd,
        "l3if (create | destroy) ifid IFID type sub-if gport GPORT_ID vlan VLAN_ID",
        CTC_CLI_L3IF_STR,
        "Create l3 interface",
        "Destroy l3 interface",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "L3 interface type",
        "Sub interface",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC)
{
    int32 ret = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT32_RANGE("gport", l3if.gport, argv [2], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("vlan_id", l3if.vlan_id, argv [3], 0, CTC_MAX_UINT16_VALUE);

    l3if.l3if_type = CTC_L3IF_TYPE_SUB_IF;

    if (0 == sal_memcmp(argv[0], "cr", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_create(l3if_id, &l3if);
        }
    }
    else if (0 == sal_memcmp(argv[0], "de", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_destory(l3if_id, &l3if);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_create_or_destroy_l3_service,
        ctc_cli_l3if_create_or_destroy_l3_service_cmd,
        "l3if (create | destroy) ifid IFID type service-if",
        CTC_CLI_L3IF_STR,
        "Create l3 interface",
        "Destroy l3 interface",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "L3 interface type",
        "Service interface")
{
    int32 ret = 0;
    uint16 l3if_id = 0;
    ctc_l3if_t l3if;

    sal_memset(&l3if, 0, sizeof(ctc_l3if_t));

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[1], 0, CTC_MAX_UINT16_VALUE);
    l3if.l3if_type = CTC_L3IF_TYPE_SERVICE_IF;

    if (0 == sal_memcmp(argv[0], "cr", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_create(l3if_id, &l3if);
        }
    }
    else if (0 == sal_memcmp(argv[0], "de", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
        }
        else
        {
            ret = ctc_l3if_destory(l3if_id, &l3if);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_l3if_set_property,
        ctc_cli_l3if_set_property_cmd,
        "l3if ifid IFID ip-property (transmit-en|receive-en|route-en|pbr-label|nat-iftype|route-all-pkt|ipv4-ucast-en|ipv4-mcast-en|ipv4-mcast-rpf-en|\
    ipv4-sa-type|ipv6-ucast-en|ipv6-mcast-en|ipv6-mcast-rpf-en|ipv6-sa-type|vrfid|vrf-en|mtu-en|mtu-exception-en|mtu-size|route-mac-prefix-type|\
    route-mac-low-8bits|egs-macsa-prefix-type|egs-macsa-low-8bits|egs-mcast-ttl-threshold|arp-excp|dhcp-excp|igmp-snooping-en|rpf-check-type|rpf-permit-default|\
    igs-acl-en|igs-acl-vlan-classid-0|igs-acl-vlan-classid-1|egs-acl-en|egs-acl-vlan-classid-0|egs-acl-vlan-classid-1|stats|egs-stats) value VALUE (is-array|)",
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "IP property",
        "Transmission enable, 1:enable, 0:disable",
        "Receive enable, 1:enable, 0:disable",
        "Routing enable, 1:enable, 0:disable",
        "PBR label",
        "The type of nat interface,0- external,1- internal",
        "All packets are routed, 1:enable, 0:disable",
        "IPv4 unicast enable, 1:enable, 0:disable",
        "IPv4 multicast enable, 1:enable,0:disable",
        "IPv4 multicast RPF check is enabled, 1:enable, 0:disable",
        "IPv4 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR",
        "IPv6 unicast enable, 1:enable, 0:disable",
        "IPv6 multicast enable, 1:enable, 0:disable",
        "IPv6 multicast RPF check enable, 1:enable, 0:disable",
        "IPv6 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR",
        CTC_CLI_VRFID_ID_DESC,
        "Enable route lookup with vrfid",
        "Mtu check enable, 1:enable, 0:disable",
        "Mtu exception to cpu enable, 1:enable, 0:disable",
        "Mtu size of the interface, <0-16383>",
        "Route mac prefix type, range<0-2>",
        "Route mac low 8bits, in 0xHH format",
        "Egress macsa prefix type, range<0-2>",
        "Egress macsa low 8bits, in 0xHH format",
        "Multicast TTL threshold, range<0-255>",
        "ARP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard",
        "DHCP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard",
        "IGMP snooping enable, 1:enable, 0:disable",
        "RPF check type, 0:strick mode, 1:loose mode",
        "RPF permit default, 0:disable, 1:enable",
        "Ingress vlan acl enable, bitmap<0-3>",
        "Ingress acl0 vlan classid, range<0-127>",
        "Ingress acl1 vlan classid, range<0-127>",
        "Engress vlan acl enable, bitmap<0-3>",
        "Engress acl0 vlan classid, range<0-127>",
        "Engress acl1 vlan classid, range<0-127>",
        "Ingress Statistic supported",
        "Egress Statistic supported",
        "Value",
        "Value of the property",
        "Use array API")
{
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop;
    ctc_property_array_t l3if_prop_array = {0};
    uint32 value = 0;
    int32  ret = 0;
    uint8 index = 0xFF;
    uint16 array_num = 1;

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "pb", 2))
    {
        l3if_prop = CTC_L3IF_PROP_PBR_LABEL;
    }
    else if (0 == sal_memcmp(argv[1], "na", 2))
    {
        l3if_prop = CTC_L3IF_PROP_NAT_IFTYPE;
    }
    else if (0 == sal_memcmp(argv[1], "route-a", 7))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_ALL_PKT;
    }
    else if (0 == sal_memcmp(argv[1], "route-en", sal_strlen("route-en")))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_EN;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-u", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_UCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-mcast-e", 12))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_MCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-s", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_SA_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-u", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_UCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-mcast-e", 12))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_MCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-s", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_SA_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "vrfid", sal_strlen("vrfid")))
    {
        l3if_prop = CTC_L3IF_PROP_VRF_ID;
    }
    else if (0 == sal_memcmp(argv[1], "vrf-en", sal_strlen("vrf-en")))
    {
        l3if_prop = CTC_L3IF_PROP_VRF_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-en", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-ex", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_EXCEPTION_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-s", 5))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_SIZE;
    }
    else if (0 == sal_memcmp(argv[1], "route-mac-p", 11))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "route-mac-l", 11))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-macsa-p", 11))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "egs-macsa-l", 11))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-mc", 6))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MCAST_TTL_THRESHOLD;
    }
    else if (0 == sal_memcmp(argv[1], "igmp-snooping-en", 4))
    {
        l3if_prop = CTC_L3IF_PROP_IGMP_SNOOPING_EN;
    }
    else if (0 == sal_memcmp(argv[1], "rpf-check-type", sal_strlen("rpf-check-type")))
    {
        l3if_prop = CTC_L3IF_PROP_RPF_CHECK_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "rpf-permit-default", sal_strlen("rpf-permit-default")))
    {
        l3if_prop = CTC_L3IF_PROP_RPF_PERMIT_DEFAULT;
    }
    else if (0 == sal_memcmp(argv[1], "stats", sal_strlen("stats")))
    {
        l3if_prop = CTC_L3IF_PROP_STATS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-stats", sal_strlen("egs-stats")))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_STATS;
    }
    else if(0 == sal_memcmp(argv[1], "pbr-label", sal_strlen("pbr-label")))
    {
        l3if_prop = CTC_L3IF_PROP_PBR_LABEL;
    }
    else
    {
        ctc_cli_out("%% %s \n", "Unrecognized command");
        return CLI_ERROR;
    }

    CTC_CLI_GET_UINT32_RANGE("ip-property", value, argv[2], 0, CTC_MAX_UINT32_VALUE);
    l3if_prop_array.property = l3if_prop;
    l3if_prop_array.data = value;
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-array");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l3if_set_property_array(g_api_lchip, l3if_id, &l3if_prop_array, &array_num);
        }
        else
        {
            ret = ctc_l3if_set_property_array(l3if_id, &l3if_prop_array, &array_num);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l3if_set_property(g_api_lchip, l3if_id, l3if_prop, value);
        }
        else
        {
            ret = ctc_l3if_set_property(l3if_id, l3if_prop, value);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

    }
    return ret;
}

CTC_CLI(ctc_cli_l3if_set_mpls_property,
        ctc_cli_l3if_set_mpls_property_cmd,
        "l3if ifid IFID mpls-property (mpls-en value VALUE | mpls-label-space value VALUE | contextlabel-exist value VALUE)",
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "MPLS property",
        "MPLS enable",
        "Value",
        "1: enable, 0: disable, other value process as 1",
        "MPLS label space",
        "Value",
        "<0-255>, space 0 is platform space",
        "Identify mpls packets on this interface have context label",
        "Value",
        "1: exist, 0: none, other value process as 1")
{
    uint8 idx = 0;
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop = CTC_L3IF_PROP_ROUTE_EN;
    uint32 value = 0;
    int32  ret = 0;

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    if (0 == sal_memcmp(argv[1], "mpls-en", sal_strlen("mpls-en")))
    {
        l3if_prop = CTC_L3IF_PROP_MPLS_EN;

        idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("value");
        if (INDEX_VALID(idx))
        {
            CTC_CLI_GET_UINT32("mpls-en", value, argv[idx + 1]);
        }
    }
    else if (0 == sal_memcmp(argv[1], "mpls-label", sal_strlen("mpls-label")))
    {
        l3if_prop = CTC_L3IF_PROP_MPLS_LABEL_SPACE;

        idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("value");
        if (INDEX_VALID(idx))
        {
            CTC_CLI_GET_UINT8("mpls-label", value, argv[idx + 1]);
        }
    }
    else if (0 == sal_memcmp(argv[1], "contextlabel-exist", sal_strlen("contextlabel-exist")))
    {
        l3if_prop = CTC_L3IF_PROP_CONTEXT_LABEL_EXIST;

        idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("value");
        if (INDEX_VALID(idx))
        {
            CTC_CLI_GET_UINT32("contextlabel-exist", value, argv[idx + 1]);
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_property(g_api_lchip, l3if_id, l3if_prop, value);
    }
    else
    {
        ret = ctc_l3if_set_property(l3if_id, l3if_prop, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_l3if_set_router_mac,
        ctc_cli_l3if_set_router_mac_cmd,
        "l3if router-mac MAC",
        CTC_CLI_L3IF_STR,
        "System Router mac",
        CTC_CLI_MAC_FORMAT)
{
    mac_addr_t mac_addr;
    int32 ret = 0;

    sal_memset(&mac_addr, 0, sizeof(mac_addr_t));

    /*parse MAC address */

    CTC_CLI_GET_MAC_ADDRESS("mac address", mac_addr, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_router_mac(g_api_lchip, mac_addr);
    }
    else
    {
        ret = ctc_l3if_set_router_mac(mac_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_set_interface_router_mac,
        ctc_cli_l3if_set_interface_router_mac_cmd,
        "l3if ifid IFID router-mac (update (is-egress |) | append | delete (is-egress |)|) MAC0 (MAC1 (MAC2 (MAC3 |) |) |)",
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Interface Router mac",
        "overwrite interface routermac(default)",
        "mac is for interface egress",
        "append new interface routermac",
        "delete interface routermac",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_FORMAT)
{
    int32 ret = 0;
    int32 i = 0;
    int32 j = 0;
    uint16 l3if_id = 0;
    ctc_l3if_router_mac_t rtmac;
    uint32 index = 0;
    mac_addr_t mac = {0};
    sal_memset(&rtmac, 0, sizeof(ctc_l3if_router_mac_t));
    /*parse MAC address */
    CTC_CLI_GET_UINT16("l3if_id", l3if_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        rtmac.mode = 0;
        i = 2;
        index = CTC_CLI_GET_ARGC_INDEX("is-egress");
        if (0xFF != index)
        {
            rtmac.dir= 1;
            i = 3;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("append");
    if (0xFF != index)
    {
        rtmac.mode = 1;
        i = 2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("delete");
    if (0xFF != index)
    {
        rtmac.mode = 2;
        i = 2;
        index = CTC_CLI_GET_ARGC_INDEX("is-egress");
        if (0xFF != index)
        {
            rtmac.dir= 1;
            i = 3;
        }
    }

    if(i == 0)
    {
        i = 1;
    }
    for (; i < argc; i++)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", rtmac.mac[j], argv[i]);
        j++;
    }
    rtmac.num = j;

    if ((!sal_memcmp(rtmac.mac[0], mac, sizeof(mac_addr_t)))
        && (!sal_memcmp(rtmac.mac[1], mac, sizeof(mac_addr_t)))
        && (!sal_memcmp(rtmac.mac[2], mac, sizeof(mac_addr_t)))
        && (!sal_memcmp(rtmac.mac[3], mac, sizeof(mac_addr_t))))
    {
        rtmac.num = 0;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_interface_router_mac(g_api_lchip, l3if_id, rtmac);
    }
    else
    {
        ret = ctc_l3if_set_interface_router_mac(l3if_id, rtmac);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_set_vmac_prefix,
        ctc_cli_l3if_set_vmac_prefix_cmd,
        "l3if vmac prefix-type PRE_TYPE prefix-value MAC",
        CTC_CLI_L3IF_STR,
        "VRRP mac",
        "Vmac prefix type",
        "Prefix type",
        "Vmac prefix value",
        "MAC address in HHHH.HHHH.HHHH format, the high 40 bits is used to prefix")
{
    uint8 prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_INVALID;
    int32 ret = 0;
    mac_addr_t mac_40bit;

    sal_memset(&mac_40bit, 0, sizeof(mac_40bit));

    if (0 == sal_memcmp("0", argv[0], 1))
    {
        prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0;
    }
    else if (0 == sal_memcmp("1", argv[0], 1))
    {
        prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE1;
    }

    /*parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", mac_40bit, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_vmac_prefix(g_api_lchip, prefix_type, mac_40bit);
    }
    else
    {
        ret = ctc_l3if_set_vmac_prefix(prefix_type, mac_40bit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l3if_add_vmac_low_8bits,
        ctc_cli_l3if_add_vmac_low_8bits_cmd,
        "l3if vmac ifid IFID add prefix-type PRE_TYPE low-8bits-mac-index MAC_INDEX low-8bits-mac LOW_8BIT_MAC",
        CTC_CLI_L3IF_STR,
        "VRRP mac",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Add low 8bits of router mac",
        "Vmac prefix type",
        "Prefix type",
        "The index of vmac entry",
        "Index range <0-3>",
        "Low 8bits mac",
        "The value of low 8bits mac, in 0xHH format")
{
    uint16 l3if_id = 0;
    int32  ret = 0;
    uint8  prefix_type = 0;
    ctc_l3if_vmac_t l3if_vmac;

    sal_memset(&l3if_vmac, 0, sizeof(ctc_l3if_vmac_t));

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("prefix_type", prefix_type, argv[1], 0, CTC_MAX_UINT8_VALUE);
    if (0 == prefix_type)
    {
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0;
    }
    else if (1 == prefix_type)
    {
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE1;
    }
    else if (2 == prefix_type)
    {
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_RSV_ROUTER_MAC;
    }
    else if (3 == prefix_type)
    {
        l3if_vmac.prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_INVALID;
    }
    else
    {
        ctc_cli_out("%% [L3if] The prefix_type for vmac is invalid! \n");
        return CLI_ERROR;
    }

    CTC_CLI_GET_UINT8_RANGE("low_8bits_mac_index", l3if_vmac.low_8bits_mac_index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("low_8bits_mac", l3if_vmac.low_8bits_mac, argv[3], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_add_vmac_low_8bit(g_api_lchip, l3if_id, &l3if_vmac);
    }
    else
    {
        ret = ctc_l3if_add_vmac_low_8bit(l3if_id, &l3if_vmac);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l3if_remove_vmac_low_8bits,
        ctc_cli_l3if_remove_vmac_low_8bits_cmd,
        "l3if vmac ifid IFID remove low-8bits-mac-index MAC_INDEX",
        CTC_CLI_L3IF_STR,
        "VRRP mac",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Remove low 8bits of router mac",
        "The index of vmac entry",
        "Index range <0-3>")
{
    uint32 l3if_id = 0;
    ctc_l3if_vmac_t l3if_vmac;
    int32  ret = 0;

    sal_memset(&l3if_vmac, 0, sizeof(ctc_l3if_vmac_t));

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("low_8bits_mac_index", l3if_vmac.low_8bits_mac_index, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_remove_vmac_low_8bit(g_api_lchip, l3if_id, &l3if_vmac);
    }
    else
    {
        ret = ctc_l3if_remove_vmac_low_8bit(l3if_id, &l3if_vmac);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_show_property,
        ctc_cli_l3if_show_property_cmd,
        "show l3if ifid IFID ip-property (transmit-en|receive-en|route-en|pbr-label|nat-iftype|route-all-pkt\
        |ipv4-ucast-en|ipv4-mcast-en|ipv4-mcast-rpf-en|ipv4-sa-type|ipv6-ucast-en|ipv6-mcast-en|ipv6-mcast-rpf-en\
        |ipv6-sa-type|vrfid|vrf-en|mtu-en|mtu-exception-en|mtu-size|route-mac-prefix-type|route-mac-low-8bits\
        |egs-macsa-prefix-type|egs-macsa-low-8bits|egs-mcast-ttl-threshold|arp-excp|dhcp-excp|igmp-snooping-en\
        |rpf-check-type|rpf-permit-default|igs-acl-en|igs-acl-vlan-classid-0|igs-acl-vlan-classid-1|egs-acl-en\
        |egs-acl-vlan-classid-0|egs-acl-vlan-classid-1|igs-stats|egs-stats|all)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "IP property",
        "Transmission enable, 1:enable, 0:disable",
        "Reception enable, 1:enable, 0:disable",
        "Routing enable, 1:enable, 0:disable",
        "PBR label",
        "The type of nat interface,0- external,1- internal",
        "All packets are routed, 1:enable, 0:disable",
        "IPv4 unicast enable, 1:enable, 0:disable",
        "IPv4 multicast enable, 1:enable,0:disable",
        "IPv4 multicast RPF check is enabled, 1:enable, 0:disable",
        "IPv4 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR",
        "IPv6 unicast enable, 1:enable, 0:disable",
        "IPv6 multicast enable, 1:enable, 0:disable",
        "IPv6 multicast RPF check enable, 1:enable, 0:disable",
        "IPv6 sa type, 0:none, 1:RPF, 2:NAT, 3:PBR",
        CTC_CLI_VRFID_ID_DESC,
        "Enable route lookup with vrfid",
        "Mtu check enable, 1:enable, 0:disable",
        "Mtu exception to cpu enable, 1:enable, 0:disable",
        "Mtu size of the interface, <0-16383>",
        "Route mac prefix type, range<0-2>",
        "Route mac low 8bits, in 0xHH format",
        "Egress macsa prefix type, range<0-2>",
        "Egress macsa low 8bits, in 0xHH format",
        "Multicast TTL threshold, range<0-255>",
        "ARP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard",
        "DHCP exception type, 0:copy-to-cpu, 1:forwarding, 2:discard&cpu, 3:discard",
        "IGMP snooping enable, 1:enable, 0:disable",
        "RPF check type, 0:strick mode, 1:loose mode",
        "RPF permit default, 0:disable, 1:enable",
        "Ingress vlan acl enable, bitmap<0-3>",
        "Ingress acl0 vlan classid, range<0-127>",
        "Ingress acl1 vlan classid, range<0-127>",
        "Engress vlan acl enable, bitmap<0-3>",
        "Engress acl0 vlan classid, range<0-127>",
        "Engress acl1 vlan classid, range<0-127>",
        "Ingress Statistic supported",
        "Egress Statistic supported",
        "Show all ip property")
{
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop;
    uint32 value = 0;
    int32  ret = 0;
    uint32 temp_prop;
    bool print_head = FALSE;
    char s[50], f[50];

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "pb", 2))
    {
        l3if_prop = CTC_L3IF_PROP_PBR_LABEL;
    }
    else if (0 == sal_memcmp(argv[1], "na", 2))
    {
        l3if_prop = CTC_L3IF_PROP_NAT_IFTYPE;
    }
    else if (0 == sal_memcmp(argv[1], "route-a", 7))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_ALL_PKT;
    }
    else if (0 == sal_memcmp(argv[1], "route-en", sal_strlen("route-en")))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_EN;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-u", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_UCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-mcast-e", 12))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_MCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv4-s", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV4_SA_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-u", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_UCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-mcast-e", 12))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_MCAST;
    }
    else if (0 == sal_memcmp(argv[1], "ipv6-s", 6))
    {
        l3if_prop = CTC_L3IF_PROP_IPV6_SA_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "vrfid", 5))
    {
        l3if_prop = CTC_L3IF_PROP_VRF_ID;
    }
    else if (0 == sal_memcmp(argv[1], "vrf-en", sal_strlen("vrf-en")))
    {
        l3if_prop = CTC_L3IF_PROP_VRF_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-en", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-ex", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_EXCEPTION_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mtu-s", 5))
    {
        l3if_prop = CTC_L3IF_PROP_MTU_SIZE;
    }
    else if (0 == sal_memcmp(argv[1], "route-mac-prefix-t", sal_strlen("route-mac-prefix-t")))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "route-mac-low-8", sal_strlen("route-mac-low-8")))
    {
        l3if_prop = CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-macsa-p", 11))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "egs-macsa-l", 11))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-mc", 6))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MCAST_TTL_THRESHOLD;
    }
    else if (0 == sal_memcmp(argv[1], "igmp-snooping-en", 4))
    {
        l3if_prop = CTC_L3IF_PROP_IGMP_SNOOPING_EN;
    }
    else if (0 == sal_memcmp(argv[1], "rpf-check-type", sal_strlen("rpf-check-type")))
    {
        l3if_prop = CTC_L3IF_PROP_RPF_CHECK_TYPE;
    }
    else if (0 == sal_memcmp(argv[1], "rpf-permit-default", sal_strlen("rpf-permit-default")))
    {
        l3if_prop = CTC_L3IF_PROP_RPF_PERMIT_DEFAULT;
    }
    else if (0 == sal_memcmp(argv[1], "all", sal_strlen("all")))
    {
        l3if_prop = MAX_CTC_L3IF_PROP_NUM;
    }
    else if (0 == sal_memcmp(argv[1], "igs-stats", sal_strlen("igs-stats")))
    {
        l3if_prop = CTC_L3IF_PROP_STATS;
    }
    else if (0 == sal_memcmp(argv[1], "egs-stats", sal_strlen("egs-stats")))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_STATS;
    }
    else if(0 == sal_memcmp(argv[1], "pbr-label", sal_strlen("pbr-label")))
    {
        l3if_prop = CTC_L3IF_PROP_PBR_LABEL;
    }
    else
    {
        ctc_cli_out("%% %s \n", "Unrecognized command");
        return CLI_ERROR;
    }

    if (MAX_CTC_L3IF_PROP_NUM == l3if_prop)
    {
        for (temp_prop = CTC_L3IF_PROP_ROUTE_EN ; temp_prop < MAX_CTC_L3IF_PROP_NUM ; temp_prop++)
        {
            if ((CTC_L3IF_PROP_MPLS_EN == temp_prop)
                    || (CTC_L3IF_PROP_MPLS_LABEL_SPACE == temp_prop)
                    || (CTC_L3IF_PROP_CONTEXT_LABEL_EXIST == temp_prop)
                    || (CTC_L3IF_PROP_STATS == temp_prop)
                    || (CTC_L3IF_PROP_EGS_STATS == temp_prop))
            {
                continue;
            }

            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, temp_prop, &value);
            }
            else
            {
                ret = ctc_l3if_get_property(l3if_id, temp_prop, &value);
            }
            if (ret < 0)
            {
                continue;
            }
            if (!print_head)
            {
                ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, "Property", "Value");
                ctc_cli_out("-----------------------------------------\n");
                print_head = TRUE;
            }
            ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[temp_prop], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
        }
        if (!print_head)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, l3if_prop, &value);
        }
        else
        {
            ret = ctc_l3if_get_property(l3if_id, l3if_prop, &value);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if (!print_head)
        {
            ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, "Property", "Value");
            ctc_cli_out("-----------------------------------------\n");
            print_head = TRUE;
        }
        ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[l3if_prop], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
    }

    if (CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE == l3if_prop)
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_PREFIX_TYPE;
    }
    else if (CTC_L3IF_PROP_ROUTE_MAC_LOW_8BITS == l3if_prop)
    {
        l3if_prop = CTC_L3IF_PROP_EGS_MAC_SA_LOW_8BITS;
    }
    else
    {
        l3if_prop = MAX_CTC_L3IF_PROP_NUM;
    }

    if (MAX_CTC_L3IF_PROP_NUM != l3if_prop)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, l3if_prop, &value);
        }
        else
        {
            ret = ctc_l3if_get_property(l3if_id, l3if_prop, &value);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[l3if_prop], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_show_mpls_property,
        ctc_cli_l3if_show_mpls_property_cmd,
        "show l3if ifid IFID mpls-property (mpls-en | mpls-label-space | contextlabel-exist | all)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "MPLS property",
        "MPLS is enabled",
        "MPLS label space id",
        "Identify mpls packets on this interface have context label",
        "Show all mpls property")
{
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop;
    uint32 value = 0;
    int32  ret = 0;
    char s[50], f[50];
    bool print_head = FALSE;
    uint8 mpls_property[] = {CTC_L3IF_PROP_MPLS_EN, CTC_L3IF_PROP_MPLS_LABEL_SPACE, CTC_L3IF_PROP_CONTEXT_LABEL_EXIST};

    sal_memset(&l3if_prop, 0, sizeof(ctc_l3if_property_t));

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "mpls-e", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MPLS_EN;
    }
    else if (0 == sal_memcmp(argv[1], "mpls-l", 6))
    {
        l3if_prop = CTC_L3IF_PROP_MPLS_LABEL_SPACE;
    }
    else if (0 == sal_memcmp(argv[1], "contextlabel-exist", 6))
    {
        l3if_prop = CTC_L3IF_PROP_CONTEXT_LABEL_EXIST;
    }
    else if (0 == sal_memcmp(argv[1], "all", sal_strlen("all")))
    {
        l3if_prop = MAX_CTC_L3IF_PROP_NUM;
    }

    if (MAX_CTC_L3IF_PROP_NUM == l3if_prop)
    {
        for (l3if_prop = 0 ; l3if_prop < sizeof(mpls_property); l3if_prop++)
        {
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, mpls_property[l3if_prop], &value);
            }
            else
            {
                ret = ctc_l3if_get_property(l3if_id, mpls_property[l3if_prop], &value);
            }
            if (ret < 0)
            {
                continue;
            }
            if (!print_head)
            {
                ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, "Property", "Value");
                ctc_cli_out("-----------------------------------------\n");
                print_head = TRUE;
            }
            ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[mpls_property[l3if_prop]], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
        }
        if (!print_head)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, l3if_prop, &value);
        }
        else
        {
            ret = ctc_l3if_get_property(l3if_id, l3if_prop, &value);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if (!print_head)
        {
            ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, "Property", "Value");
            ctc_cli_out("-----------------------------------------\n");
            print_head = TRUE;
        }
        ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[l3if_prop], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_show_router_mac,
        ctc_cli_l3if_show_router_mac_cmd,
        "show l3if router-mac (ifid IFID (ingress | egress |)|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        "Router mac"
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC)
{
    int ret = 0;
    mac_addr_t mac_addr;
    uint8 index = 0;
    uint16 l3if_id = 0;
    ctc_l3if_router_mac_t rtmac;
    int32 i = 0;
    uint32 value = 0;
    uint8 mac_index = 0;
    ctc_l3if_vmac_t l3if_vmac;
    mac_addr_t mac_40bit;
    uint8 chip_type = 0;

    sal_memset(&mac_addr, 0, sizeof(mac_addr_t));
    sal_memset(&rtmac, 0, sizeof(rtmac));

    if(g_ctcs_api_en)
    {
        chip_type = ctcs_get_chip_type(g_api_lchip);
    }
    else
    {
        chip_type = ctc_get_chip_type();
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        rtmac.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ifid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        if (CTC_CHIP_GREATBELT !=  chip_type)
        {
            ctc_cli_out("The interface router mac:\n");
            do
            {
                if (g_ctcs_api_en)
                {
                    ret = ctcs_l3if_get_interface_router_mac(g_api_lchip, l3if_id, &rtmac);
                }
                else
                {
                    ret = ctc_l3if_get_interface_router_mac(l3if_id, &rtmac);
                }
                if (ret < 0)
                {
                    ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                    return CLI_ERROR;
                }
                for (i = 0; i < rtmac.num; i++)
                {
                    ctc_cli_out("\t\t%.2x%.2x.%.2x%.2x.%.2x%.2x\n",
                                rtmac.mac[i][0], rtmac.mac[i][1], rtmac.mac[i][2], rtmac.mac[i][3], rtmac.mac[i][4], rtmac.mac[i][5]);
                }
                sal_memset(rtmac.mac, 0, 4*sizeof(mac_addr_t));
            }
            while (rtmac.next_query_index != 0xffffffff && rtmac.next_query_index != 0);
        }
        else
        {
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE, &value);
            }
            else
            {
                ret = ctc_l3if_get_property(l3if_id, CTC_L3IF_PROP_ROUTE_MAC_PREFIX_TYPE, &value);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            if (CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_RSV_ROUTER_MAC == value)
            {
                if(g_ctcs_api_en)
                {
                     ret = ctcs_l3if_get_router_mac(g_api_lchip, mac_addr);
                }
                else
                {
                    ret = ctc_l3if_get_router_mac(mac_addr);
                }
                if (ret < 0)
                {
                    ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                    return CLI_ERROR;
                }
                ctc_cli_out("The router mac is :%.2x%.2x.%.2x%.2x.%.2x%.2x \n",
                        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            }
            else
            {
                ctc_cli_out("The route mac is :\n");
                ctc_cli_out("%-14s%s\n", "INDEX", "VALUE");
                ctc_cli_out("-----------------------\n");
                for (mac_index = 0; mac_index < 4; mac_index++)
                {
                    l3if_vmac.low_8bits_mac_index = mac_index;

                    if(g_ctcs_api_en)
                    {
                         ret = ctcs_l3if_get_vmac_low_8bit(g_api_lchip, l3if_id, &l3if_vmac);
                    }
                    else
                    {
                        ret = ctc_l3if_get_vmac_low_8bit(l3if_id, &l3if_vmac);
                    }
                    if (ret < 0)
                    {
                        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                        return CLI_ERROR;
                    }

                    if ( 3 == l3if_vmac.prefix_type)
                    {
                        ctc_cli_out("%-14u%s\n", l3if_vmac.low_8bits_mac_index, "-");
                    }
                    else
                    {
                        if(g_ctcs_api_en)
                        {
                             ret = ctcs_l3if_get_vmac_prefix(g_api_lchip, l3if_vmac.prefix_type, mac_40bit);
                        }
                        else
                        {
                            ret = ctc_l3if_get_vmac_prefix(l3if_vmac.prefix_type, mac_40bit);
                        }
                        if (ret < 0)
                        {
                            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                            return CLI_ERROR;
                        }
                        ctc_cli_out("%-14u%.2x%.2x.%.2x%.2x.%.2x%.2x \n",
                                l3if_vmac.low_8bits_mac_index,
                                mac_40bit[0], mac_40bit[1], mac_40bit[2], mac_40bit[3], mac_40bit[4],l3if_vmac.low_8bits_mac);
                    }
                }
            }
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_get_router_mac(g_api_lchip, mac_addr);
        }
        else
        {
            ret = ctc_l3if_get_router_mac(mac_addr);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("The router mac is :%.2x%.2x.%.2x%.2x.%.2x%.2x \n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    }

    return ret;
}

CTC_CLI(ctc_cli_l3if_show_vmac_prefix,
        ctc_cli_l3if_show_vmac_prefix_cmd,
        "show l3if vmac prefix-type PRE_TYPE",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        "VRRP mac",
        "Vmac prefix type",
        "Prefix type, 0 or 1")
{
    uint8 prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE_INVALID;
    int32 ret = 0;
    mac_addr_t mac_40bit;

    sal_memset(&mac_40bit, 0, sizeof(mac_addr_t));

    if (0 == sal_memcmp("0", argv[0], 1))
    {
        prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE0;
    }
    else if (0 == sal_memcmp("1", argv[0], 1))
    {
        prefix_type = CTC_L3IF_ROUTE_MAC_PFEFIX_TYPE1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_get_vmac_prefix(g_api_lchip, prefix_type, mac_40bit);
    }
    else
    {
        ret = ctc_l3if_get_vmac_prefix(prefix_type, mac_40bit);
    }
    ctc_cli_out("The VRRP mac prefix type %d is :%.2x%.2x.%.2x%.2x.%.2x\n", prefix_type,
                mac_40bit[0], mac_40bit[1], mac_40bit[2], mac_40bit[3], mac_40bit[4]);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l3if_show_vmac_low_8bits,
        show_l3if_vmac_low_8bits_cmd,
        "show l3if vmac low-8bits ifid IFID low-8bits-mac-index MAC_INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        "VRRP mac",
        "Low 8 bits",
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "The index of vmac entry",
        "Index range <0-3>")
{
    uint32 l3if_id = 0;
    ctc_l3if_vmac_t l3if_vmac;
    int32  ret = 0;

    sal_memset(&l3if_vmac, 0, sizeof(ctc_l3if_vmac_t));

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("low_8bits_mac_index", l3if_vmac.low_8bits_mac_index, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_get_vmac_low_8bit(g_api_lchip, l3if_id, &l3if_vmac);
    }
    else
    {
        ret = ctc_l3if_get_vmac_low_8bit(l3if_id, &l3if_vmac);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("l3if_id = %d, low_8bits_mac_index = %d, prefix_type:%d (3: invalid type) low_8bits_mac_value = 0x%x\n",
                l3if_id, l3if_vmac.low_8bits_mac_index, l3if_vmac.prefix_type, l3if_vmac.low_8bits_mac);

    return ret;
}

CTC_CLI(ctc_cli_l3if_set_vrf_stats_en,
        ctc_cli_l3if_set_vrf_stats_en_cmd,
        "l3if stats vrfid VRFID (enable|disable) (statsid STATS_ID|) (ucast|all|)",
        CTC_CLI_L3IF_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        "Enable",
        "Disable",
        "Stats id",
        CTC_CLI_STATS_ID_VAL,
        "Stats only ucast packets",
        "Stats all route packets")
{
    ctc_l3if_vrf_stats_t vrf_stats;
    uint8 index = 0xFF;
    int32  ret = 0;

    sal_memset(&vrf_stats, 0, sizeof(vrf_stats));

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrf_stats.vrf_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "en", 2))
    {
        vrf_stats.enable = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("statsid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("statsid", vrf_stats.stats_id, argv[index+1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        vrf_stats.type = CTC_L3IF_VRF_STATS_ALL;
    }
    else
    {
        vrf_stats.type = CTC_L3IF_VRF_STATS_UCAST;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_vrf_stats_en(g_api_lchip, &vrf_stats);
    }
    else
    {
        ret = ctc_l3if_set_vrf_stats_en(&vrf_stats);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_l3if_debug_on,
        ctc_cli_l3if_debug_on_cmd,
        "debug l3if (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L3IF_STR,
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
        typeenum = L3IF_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L3IF_SYS;
    }

    ctc_debug_set_flag("l3if", "l3if", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l3if_debug_off,
        ctc_cli_l3if_debug_off_cmd,
        "no debug l3if (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L3IF_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = L3IF_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L3IF_SYS;
    }

    ctc_debug_set_flag("l3if", "l3if", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l3if_show_debug,
        ctc_cli_l3if_show_debug_cmd,
        "show debug l3if (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L3IF_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = L3IF_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = L3IF_SYS;
    }

    en = ctc_debug_get_flag("l3if", "l3if", typeenum, &level);
    ctc_cli_out("l3if:%s debug %s level %s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l3if_create_or_destroy_l3_ecmp_if,
        ctc_cli_l3if_create_or_destroy_l3_ecmp_if_cmd,
        "l3if (create | destroy) ecmp-if IFID ({dynamic | failover-en | statsid STATS_ID} |)",
        CTC_CLI_L3IF_STR,
        "Create l3 interface",
        "Destroy l3 interface",
        CTC_CLI_ECMP_L3IF_ID_DESC,
        "Ecmp if ID",
        "Dynamic load balance",
        "Failover enable",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL)
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 l3if_id = 0;
    ctc_l3if_ecmp_if_t ecmp_if;

    sal_memset(&ecmp_if, 0, sizeof(ctc_l3if_ecmp_if_t));

    CTC_CLI_GET_UINT16("ifid", l3if_id, argv[1]);
    ecmp_if.ecmp_if_id = l3if_id;

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        ecmp_if.ecmp_type = CTC_NH_ECMP_TYPE_DLB;
    }
    else
    {
        ecmp_if.ecmp_type = CTC_NH_ECMP_TYPE_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("failover-en");
    if (index != 0xFF)
    {
        ecmp_if.failover_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("statsid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("stats id", ecmp_if.stats_id, argv[index + 1]);
    }

    if (0 == sal_memcmp(argv[0], "create", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_create_ecmp_if(g_api_lchip, &ecmp_if);
        }
        else
        {
            ret = ctc_l3if_create_ecmp_if(&ecmp_if);
        }
    }
    else if (0 == sal_memcmp(argv[0], "destroy", 2))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_destory_ecmp_if(g_api_lchip, l3if_id);
        }
        else
        {
            ret = ctc_l3if_destory_ecmp_if(l3if_id);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l3if_add_or_remove_ecmpif_member_ifid,
        ctc_cli_l3if_add_or_remove_ecmpif_member_ifid_cmd,
        "l3if ecmp-if IFID (add | remove) member-if IFID",
        CTC_CLI_L3IF_STR,
        CTC_CLI_ECMP_L3IF_ID_DESC,
        "Ecmp if ID",
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC)
{
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;
    uint16 ecmp_ifid = 0;
    uint16 member_ifid = 0;

    CTC_CLI_GET_UINT16("ecmp ifid", ecmp_ifid, argv[0]);
    CTC_CLI_GET_UINT16("member ifid", member_ifid, argv[2]);

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (index != 0xFF)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_add_ecmp_if_member(g_api_lchip, ecmp_ifid, member_ifid);
        }
        else
        {
            ret = ctc_l3if_add_ecmp_if_member(ecmp_ifid, member_ifid);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_remove_ecmp_if_member(g_api_lchip, ecmp_ifid, member_ifid);
        }
        else
        {
            ret = ctc_l3if_remove_ecmp_if_member(ecmp_ifid, member_ifid);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l3if_set_rtmac_prefix,
        ctc_cli_l3if_set_rtmac_prefix_cmd,
        "l3if rtmac-prefix PRE_IDX prefix MAC",
        CTC_CLI_L3IF_STR,
        "router mac prefix",
        "index value",
        "rtmac prefix",
        "MAC address in HHHH.HHHH.HHHH format, the high 40 bits is used to prefix")
{
    uint32 prefix_idx = 0;
    int32 ret = 0;
    mac_addr_t mac_40bit;

    sal_memset(&mac_40bit, 0, sizeof(mac_40bit));

    CTC_CLI_GET_UINT32("prefix index", prefix_idx, argv[0]);

    /*parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", mac_40bit, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_vmac_prefix(g_api_lchip, prefix_idx, mac_40bit);
    }
    else
    {
        ret = ctc_l3if_set_vmac_prefix(prefix_idx, mac_40bit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l3if_show_rtmac_prefix,
        ctc_cli_l3if_show_rtmac_prefix_cmd,
        "show l3if rtmac-prefix PRE_IDX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        "router mac prefix index",
        "Prefix index value")
{
    uint32 prefix_idx;
    int32 ret = 0;
    mac_addr_t mac_40bit;

    sal_memset(&mac_40bit, 0, sizeof(mac_addr_t));

    CTC_CLI_GET_UINT32("prefix-index", prefix_idx, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_get_vmac_prefix(g_api_lchip, prefix_idx, mac_40bit);
    }
    else
    {
        ret = ctc_l3if_get_vmac_prefix(prefix_idx, mac_40bit);
    }
    ctc_cli_out("The VRRP mac prefix %d is :%.2x%.2x.%.2x%.2x.%.2x\n", prefix_idx,
                mac_40bit[0], mac_40bit[1], mac_40bit[2], mac_40bit[3], mac_40bit[4]);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

int32
ctc_l3if_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_create_or_destroy_l3_phy_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_create_or_destroy_l3_vlan_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_create_or_destroy_l3_sub_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_create_or_destroy_l3_service_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_create_or_destroy_l3_ecmp_if_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_l3if_id_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_mpls_property_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_router_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_interface_router_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_vmac_prefix_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_add_vmac_low_8bits_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_add_or_remove_ecmpif_member_ifid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_remove_vmac_low_8bits_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_property_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_router_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_vmac_prefix_cmd);
    install_element(CTC_SDK_MODE, &show_l3if_vmac_low_8bits_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_vrf_stats_en_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_mpls_property_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l3if_set_rtmac_prefix_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l3if_show_rtmac_prefix_cmd);

    return CLI_SUCCESS;
}

