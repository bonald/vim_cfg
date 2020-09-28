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

#define CTC_CLI_IPUC_MASK_LEN 5
#define CTC_CLI_IPUC_L4PORT_LEN 7
#define CTC_CLI_IP_TUNNEL_ACL_STR \
    "acl-property {\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}}"

#define CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC \
        "Acl priority",\
        "Acl priority value",\
        "Acl lookup type",\
        "type value",\
        "acl use class id",\
        "CLASS_ID",\
        "acl use port-bitmap",\
        "acl use metadata",\
        "acl use logic-port",\
        "acl use mapped vlan"

#define CTC_CLI_IPUC_QOS_MAP_STR \
    "qos-map {priority PRIORITY | color COLOR | dscp DSCP | trust-dscp | (trust-scos | trust-ccos) | cos-domain DOMAIN | dscp-domain DOMAIN}"

#define CTC_CLI_IPUC_QOS_MAP_DESC \
     "QoS map",   \
     "Priority",  \
     "Priority",   \
     "Color",     \
     CTC_CLI_COLOR_VALUE,  \
     "Dscp",  \
     CTC_CLI_DSCP_VALUE, \
     "Trust-dscp",   \
     "Trust-scos",   \
     "Trust-ccos",   \
     "Cos domain",   \
     "VALUE",        \
     "Dscp domain",  \
     "VALUE"

#define CTC_CLI_IP_TUNNEL_ACL_PARAM_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("acl-property");\
    if (index != 0xFF)\
    {\
        index = CTC_CLI_GET_ARGC_INDEX("priority");\
        if (0xFF != index)\
        {\
            idx1 = index;\
            idx2 = 0;\
            while(last_time < 1)\
            {\
                idx2 = ctc_cli_get_prefix_item(&argv[idx1+1], argc-(idx1+1), "priority", 0);\
                if(0xFF == idx2)\
                {\
                    last_time = 1;\
                    idx2 = argc-1-idx1-1;\
                }\
                CTC_CLI_GET_UINT8("priority", acl_property[cnt].acl_priority, argv[idx1 + 1]);\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "lookup-type", 0);\
                if (0xFF != index)\
                {\
                    CTC_CLI_GET_UINT8("lookup-type", acl_property[cnt].tcam_lkup_type, argv[idx1 + index + 2]);\
                }\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "class-id", 0);\
                if (0xFF != index)\
                {\
                    CTC_CLI_GET_UINT8("class-id", acl_property[cnt].class_id, argv[idx1 + index + 2]);\
                }\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-logic-port", 0);\
                if(0xFF != index)\
                {\
                    CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);\
                }\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-port-bitmap", 0);\
                if(0xFF != index)\
                {\
                    CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);\
                }\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-metadata", 0);\
                if(0xFF != index)\
                {\
                    CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_METADATA);\
                }\
                index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-mapped-vlan", 0);\
                if(0xFF != index)\
                {\
                    CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);\
                }\
                cnt++;\
                idx1 = idx1 + idx2 + 1;\
            }\
        }\
    }

#define CTC_CLI_IPUC_QOS_MAP_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("qos-map");\
    if (0xFF != index) \
    {\
        index = CTC_CLI_GET_ARGC_INDEX("priority");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID); \
            CTC_CLI_GET_UINT8("priority", qos_map.priority, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("color");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("color", qos_map.color, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID); \
            CTC_CLI_GET_UINT8("dscp", qos_map.dscp, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-dscp");\
        if (0xFF != index)\
        {\
            qos_map.trust_dscp = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-scos");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID); \
            qos_map.trust_cos = 0;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-ccos");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID); \
            qos_map.trust_cos = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("cos-domain");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("cos-domain", qos_map.cos_domain, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp-domain");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("dscp-domain", qos_map.dscp_domain, argv[index + 1]);\
        }\
    }


#define CTC_CLI_RPF_SET_GLOBAL_SRC_PORT(port, num)                                                         \
{                                                                                                          \
    if ((index + 1 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 1])))                              \
    {                                                                                                      \
        CTC_CLI_GET_UINT32("gport", port[0], argv[index + 1]);                                             \
        num++;                                                                                             \
                                                                                                           \
        if ((index + 2 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 2])))                          \
        {                                                                                                  \
            CTC_CLI_GET_UINT32("gport", port[1], argv[index + 2]);                                         \
            num++;                                                                                         \
                                                                                                           \
            if ((index + 3 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 3])))                      \
            {                                                                                              \
                CTC_CLI_GET_UINT32("gport", port[2], argv[index + 3]);                                     \
                num++;                                                                                     \
                                                                                                           \
                if ((index + 4 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 4])))                  \
                {                                                                                          \
                    CTC_CLI_GET_UINT32("gport", port[3], argv[index + 4]);                                 \
                    num++;                                                                                 \
                                                                                                           \
                    if ((index + 5 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 5])))              \
                    {                                                                                      \
                        CTC_CLI_GET_UINT32("gport", port[4], argv[index + 5]);                             \
                        num++;                                                                             \
                                                                                                           \
                        if ((index + 6 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 6])))          \
                        {                                                                                  \
                            CTC_CLI_GET_UINT32("gport", port[5], argv[index + 6]);                         \
                            num++;                                                                         \
                                                                                                           \
                            if ((index + 7 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 7])))      \
                            {                                                                              \
                                CTC_CLI_GET_UINT32("gport", port[6], argv[index + 7]);                     \
                                num++;                                                                     \
                                                                                                           \
                                if ((index + 8 < argc) && (0 == ctc_cmd_judge_is_num(argv[index + 8])))  \
                                {                                                                          \
                                    CTC_CLI_GET_UINT32("gport", port[7], argv[index + 8]);                 \
                                    num++;                                                                 \
                                }                                                                          \
                            }                                                                              \
                        }                                                                                  \
                    }                                                                                      \
                }                                                                                          \
            }                                                                                              \
        }                                                                                                  \
    }                                                                                                      \
}


#define CTC_CLI_IPUC_PRINT_HEADER(ip_ver) \
    ctc_cli_out("Flags:  R-RPF check     T-TTL check     C-Send to CPU     I-ICMP redirect check\n\r");\
    ctc_cli_out("        X-Connect       O-None flag     S-Self address    Y-To fabric\n\r");\
    ctc_cli_out("        B-Public route  L-Host use Lpm  P-Protocol entry  Z-Cancel nat \n\r");\
    ctc_cli_out("        N-Neighbor      F-Stats enable  A-Aging enable    M-Assign port\n\r");\
    ctc_cli_out("        E-ICMP error msg check\n\r");\
    ctc_cli_out("---------------------------------------------------------------------------------------\n\r");\
    if (ip_ver == CTC_IP_VER_4)\
    {\
        ctc_cli_out("VRF   Route                           NHID   Flags   StatsId   CID\n\r");\
    }\
    else\
    {\
        ctc_cli_out("VRF   Route                                         NHID   Flags   StatsId   CID\n\r");\
    }\
    ctc_cli_out("---------------------------------------------------------------------------------------\n\r");\

CTC_CLI(ctc_cli_ipuc_add_ipv4,
        ctc_cli_ipuc_add_ipv4_cmd,
        "ipuc add VRFID A.B.C.D MASK_LEN NHID {ecmp | {ttl-check | ttl-no-check} | rpf-check (port PORT0 (PORT1 (PORT2 (PORT3 (PORT4 (PORT5 (PORT6 (PORT7 |)|)|)|)|)|)|)|) | icmp-check | icmp-err-check | copy-to-cpu | protocol-entry | self-address | is-neighbor | is-connect ((l3if L3IFID) |) | aging | l4-dst-port L4PORT (tcp-port|) | assign-port PORT | stats-id STATISID | to-fabric | cancel-nat | is-public | cid CID | use-lpm |  \
         macda MAC |}",
        CTC_CLI_IPUC_M_STR,
        "Add route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        CTC_CLI_NH_ID_STR,
        "ECMP group",
        "TTL check enable",
        "TTL check disable",
        "RPF check enable",
        "RPF permit global source port",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ICMP redirect check",
        "ICMP error message check",
        "Copy to cpu",
        "Protocol-entry",
        "Self address, used for oam or ptp",
        "This route is a Neighbor",
        "This route is a Connect route",
        "L3 interface of this connect route",
        "L3 interface ID",
        "Aging enable(only for host route)",
        "Layer4 destination port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "If set, indicate it is a tcp port, or is a udp port",
        "Specify assign dest gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Stats id",
        CTC_CLI_STATS_ID_VAL,
        "Packet will been sent to fabric under spine-leaf mode",
        "This route will not do nat",
        "Is-public route",
        "Category Id",
        "Category Id Value",
        "Host route use lpm",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT)
{
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint8 masklen = 0;
    uint8 index = 0;
    uint32 gport = 0;
    ctc_ipuc_param_t ipuc_info = {0};

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipuc_info.masklen = masklen;

    CTC_CLI_GET_UINT32("nexthop id", ipuc_info.nh_id, argv[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-no-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TTL_NO_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_RPF_SET_GLOBAL_SRC_PORT(ipuc_info.rpf_port, ipuc_info.rpf_port_num);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ICMP_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-err-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ICMP_ERR_MSG_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("copy-to-cpu");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("protocol-entry");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("self-address");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-neighbor");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-connect");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CONNECT;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("l3 interface", ipuc_info.l3_inf, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("aging");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_AGING_EN;
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

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("assign-port");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ASSIGN_PORT;
        CTC_CLI_GET_UINT32("gport", gport, argv[index + 1]);
        ipuc_info.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("stats-id", ipuc_info.stats_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        ipuc_info.route_flag |= CTC_IPUC_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("to-fabric");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TO_FABRIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cancel-nat");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CANCEL_NAT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-public");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cid", ipuc_info.cid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("macda");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", ipuc_info.mac, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add(g_api_lchip, &ipuc_info);
    }
    else
    {
        ret = ctc_ipuc_add(&ipuc_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_ipuc_add_ipv6,
        ctc_cli_ipuc_add_ipv6_cmd,
        "ipuc ipv6 add VRFID X:X::X:X MASK_LEN NHID {ecmp | {ttl-check | ttl-no-check} | rpf-check (port PORT0 (PORT1 (PORT2 (PORT3 (PORT4 (PORT5 (PORT6 (PORT7 |)|)|)|)|)|)|)|) | icmp-check | icmp-err-check | copy-to-cpu | protocol-entry | self-address | is-neighbor | is-connect ((l3if L3IFID)|) | aging | assign-port PORT | stats-id STATISID | to-fabric | cancel-nat | is-public | cid CID | use-lpm|}",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        "Add ipv6 route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        CTC_CLI_NH_ID_STR,
        "ECMP group",
        "TTL check enable",
        "TTL check disable",
        "RPF check enable",
        "RPF permit global source port",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ICMP redirect check",
        "ICMP error message check",
        "Copy to cpu",
        "Protocol-entry",
        "Self address, used for oam or ptp",
        "This route is a Neighbor",
        "This route is a Connect route",
        "L3 interface of this connect route",
        "L3 interface ID",
        "Aging enable",
        "Specify assign dest gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Stats id",
        CTC_CLI_STATS_ID_VAL,
        "Packet will been sent to fabric under spine-leaf mode",
        "This route will not do nat",
        "Is-public route",
        "Category Id",
        "Category Id Value",
        "Host route use lpm")
{
    uint32 gport = 0;
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint32 masklen = 0;
    uint8 index = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info;

    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
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

    CTC_CLI_GET_UINT32("nexthop id", ipuc_info.nh_id, argv[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-no-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TTL_NO_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_RPF_SET_GLOBAL_SRC_PORT(ipuc_info.rpf_port, ipuc_info.rpf_port_num);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ICMP_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-err-check");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ICMP_ERR_MSG_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("copy-to-cpu");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("protocol-entry");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("self-address");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-neighbor");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-connect");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CONNECT;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("l3 interface", ipuc_info.l3_inf, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("aging");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_AGING_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("assign-port");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_ASSIGN_PORT;
        CTC_CLI_GET_UINT32("gport", gport, argv[index + 1]);
        ipuc_info.gport = gport;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("stats-id", ipuc_info.stats_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        ipuc_info.route_flag |= CTC_IPUC_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("to-fabric");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_TO_FABRIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cancel-nat");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_CANCEL_NAT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-public");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("cid", ipuc_info.cid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add(g_api_lchip, &ipuc_info);
    }
    else
    {
        ret = ctc_ipuc_add(&ipuc_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_add_ipv4_tunnel,
        ctc_cli_ipuc_add_ipv4_tunnel_cmd,
        "ipuc tunnel add A.B.C.D {vrf VRFID | nhid NHID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY | gre-checksum | gre-seq-num \
        | ip-sa A.B.C.D | ecmp | ttl-check | use-outer-ttl | lookup-by-outer | isatap-source-check | copy-to-cpu    \
        | rpf-check (l3if l3ifid0 VALUE  (l3ifid1 VALUE | ) (l3ifid2 VALUE |) (l3ifid3 VALUE |) (more-rpf |) |)     \
        | use-outer-info | follow-port | qos-use-outer-info | qos-follow-port | policer-id POLICER_ID | service-id SERVICE_ID | logic-port GPHYPORT_ID | metadata METADATA | service-acl-en | service-queue-en \
        | service-policer-en | stats STATS_ID | discard | use-flex | scl-id SCL_ID | cid CID| rpf-check-disable | "CTC_CLI_IP_TUNNEL_ACL_STR" | "CTC_CLI_IPUC_QOS_MAP_STR"|}",
        CTC_CLI_IPUC_M_STR,
        "This route is ip tunnel",
        "Add ip tunnel route",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_VRFID_ID_DESC ", inner route will lookup by this VRFID",
        CTC_CLI_VRFID_ID_DESC ", inner route will lookup by this VRFID",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_ID_STR,
        "Tunnel payload layer 4 type (default is IPv6)",
        "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6",
        "Set Gre key value",
        "Gre key value",
        "Gre tunnel with checksum",
        "Gre tunnel with sequence number",
        "Set IP Sa",
        CTC_CLI_IPV4_FORMAT,
        "ECMP group",
        "Do ttl check for tunnel outer header",
        "Use outer ttl for later process , or use inner ttl",
        "Packet will do router by tunnel outer header , or use payload header",
        "ISATAP source adderss check",
        "Copy to CPU",
        "RPF check enable",
        "L3 interface of this tunnel route, only need when do RPF check and packet do router by inner header",
        "The 1st L3 interface ID",
        "The 1st L3 interface ID , 0 means invalid",
        "The 2nd L3 interface ID",
        "The 2nd L3 interface ID , 0 means invalid",
        "The 3rd L3 interface ID",
        "The 3rd L3 interface ID , 0 means invalid",
        "The 4th L3 interface ID",
        "The 4th L3 interface ID , 0 means invalid",
        "RPF check fail packet will send to cpu",
        "Use outer header info do acl lookup, default use the inner",
        "Use port config do acl lookup",
        "Use outer dscp/cos do qos classification, default use inner",
        "Use port config do qos classification",
        "Policer id",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service Id",
        CTC_CLI_SCL_SERVICE_ID_VALUE,
        "Logic port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Metadata",
        "metadata",
        "Enable service acl",
        "Enable service queue",
        "Enable service policer",
        "Enable stats",
        CTC_CLI_STATS_ID_VAL,
        "Discard",
        "Use tcam key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Category Id",
        "Category Id Value",
        "Rpf check disable",
        "Acl property",
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IPUC_QOS_MAP_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0,idx2 = 0;
    uint8 last_time = 0;
    ctc_scl_qos_map_t  qos_map = {0};
    ctc_ipuc_tunnel_param_t tunnel_info = {0};
    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM];

    tunnel_info.payload_type = 2;
    CTC_CLI_GET_IPV4_ADDRESS("ip", tunnel_info.ip_da.ipv4, argv[0]);
    sal_memset(acl_property, 0 ,sizeof(acl_property));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vrf");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN;
        CTC_CLI_GET_UINT16("vrf id", tunnel_info.vrf_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nhid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("nexthop id", tunnel_info.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel payload type", tunnel_info.payload_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-key");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY;
        CTC_CLI_GET_UINT32_RANGE("Gre key", tunnel_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-checksum");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_CHECKSUM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-seq-num");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_SEQ_NUM;
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA;
        CTC_CLI_GET_IPV4_ADDRESS("IP Sa", tunnel_info.ip_sa.ipv4, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-outer-ttl");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_OUTER_TTL;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lookup-by-outer");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("isatap-source-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ISATAP_CHECK_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid0");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 1nd l3 interface", tunnel_info.l3_inf[0], argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid1");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 2nd l3 interface", tunnel_info.l3_inf[1], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid2");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 3rd l3 interface", tunnel_info.l3_inf[2], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid3");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 4th l3 interface", tunnel_info.l3_inf[3], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("more-rpf");
        if (index != 0xFF)
        {
            tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_MORE_RPF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-outer-info");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("follow-port");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("qos-use-outer-info");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_QOS_USE_OUTER_INFO;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("qos-follow-port");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_QOS_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("policer-id", tunnel_info.policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("service-id", tunnel_info.service_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("logic-port", tunnel_info.logic_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("metadata", tunnel_info.metadata, argv[index + 1]);
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_METADATA_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-acl-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_QUEUE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-policer-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("stats", tunnel_info.stats_id, argv[index + 1]);
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_info.scl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("cid", tunnel_info.cid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check-disable");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE;
    }

    CTC_CLI_IP_TUNNEL_ACL_PARAM_SET();
    if (cnt > 0)
    {
        tunnel_info.acl_lkup_num = cnt;
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_EN;
        sal_memcpy(tunnel_info.acl_prop,acl_property, sizeof(ctc_acl_property_t)* cnt);
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-map");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_info.flag, CTC_IPUC_TUNNEL_FLAG_QOS_MAP);
    }
    CTC_CLI_IPUC_QOS_MAP_SET();
    sal_memcpy(&tunnel_info.qos_map, &qos_map, sizeof(qos_map));

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add_tunnel(g_api_lchip, &tunnel_info);
    }
    else
    {
        ret = ctc_ipuc_add_tunnel(&tunnel_info);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_ipuc_add_ipv6_tunnel,
        ctc_cli_ipuc_add_ipv6_tunnel_cmd,
        "ipuc ipv6 tunnel add X:X::X:X {vrf VRFID |nhid NHID | tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY  | gre-checksum | gre-seq-num " \
        "| ip-sa X:X::X:X | ecmp | ttl-check | use-outer-ttl | lookup-by-outer | copy-to-cpu "\
        "| rpf-check (l3if l3ifid0 VALUE  (l3ifid1 VALUE | ) (l3ifid2 VALUE |) (l3ifid3 VALUE |) (more-rpf |) |) "    \
        "| use-outer-info | follow-port | qos-use-outer-info | qos-follow-port "\
        "| policer-id POLICER_ID | service-id SERVICE_ID | logic-port GPHYPORT_ID | metadata METADATA | service-acl-en | service-queue-en | service-policer-en "       \
        "| stats STATS_ID | discard | scl-id SCL_ID |cid CID | rpf-check-disable | use-flex | "CTC_CLI_IP_TUNNEL_ACL_STR"| "CTC_CLI_IPUC_QOS_MAP_STR"|}",
        CTC_CLI_IPUC_M_STR,
        "Add ipv6 route",
        "This route is ip tunnel",
        "Add ip tunnel route",
        CTC_CLI_IPV6_FORMAT,
        "used to do route lookup, Inner route will lookup by this VRFID (HUMBER not support)",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_ID_STR,
        "Tunnel payload layer 4 type (default is IPv4)",
        "Tunnel payload layer 4 type: 0 for GRE, 1 for IPv4, 2 for IPv6",
        "Set Gre key value",
        "Gre key value",
        "Gre tunnel with checksum",
        "Gre tunnel with sequence number",
        "Set IP Sa",
        CTC_CLI_IPV6_FORMAT,
        "ECMP group (Only HUMBER support)",
        "Do ttl check for tunnel outer header",
        "Use outer ttl for later process , or use inner ttl",
        "Packet will do router by tunnel outer header , or use payload header",
        "Copy to CPU (Only HUMBER support)",
        "RPF check enable",
        "L3 interface of this tunnel route, only need when do RPF check and packet do router by inner header",
        "The 1st L3 interface ID",
        "The 1st L3 interface ID , 0 means invalid",
        "The 2nd L3 interface ID",
        "The 2nd L3 interface ID , 0 means invalid",
        "The 3rd L3 interface ID",
        "The 3rd L3 interface ID , 0 means invalid",
        "The 4th L3 interface ID",
        "The 4th L3 interface ID , 0 means invalid",
        "RPF check fail packet will send to cpu",
        "Use outer header info do acl lookup, default use the inner",
        "Use port config do acl lookup",
        "Use outer dscp/cos do qos classification, default use inner",
        "Use port config do qos classification",
        "Policer id",
        CTC_CLI_QOS_FLOW_PLC_VAL,
        "Service Id",
        CTC_CLI_SCL_SERVICE_ID_VALUE,
        "Logic port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Metadata",
        "Metadata",
        "Enable service acl",
        "Enable service queue",
        "Enable service policer",
        "Enable stats",
        "Stats ID",
        "Discard",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Category Id",
        "Category Id Value",
        "Rpf check disable",
        "Use tcam key",
        "Acl property",
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IP_TUNNEL_ACL_PARAM_DESC,
        CTC_CLI_IPUC_QOS_MAP_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0,idx2 = 0;
    uint8 last_time = 0;
    ipv6_addr_t ipv6_address;
    ctc_scl_qos_map_t  qos_map = {0};
    ctc_ipuc_tunnel_param_t tunnel_info = {0};
    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM];

    tunnel_info.ip_ver = CTC_IP_VER_6;
    tunnel_info.payload_type = 1;
    sal_memset(acl_property, 0 ,sizeof(acl_property));

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);
    /* adjust endian */
    tunnel_info.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_info.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_info.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_info.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vrf");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN;
        CTC_CLI_GET_UINT16("vrf id", tunnel_info.vrf_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nhid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("nexthop id", tunnel_info.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel payload type", tunnel_info.payload_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-key");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY;
        CTC_CLI_GET_UINT32_RANGE("Gre key", tunnel_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-checksum");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_CHECKSUM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-seq-num");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_SEQ_NUM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA;
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        /* adjust endian */
        tunnel_info.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_info.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_info.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_info.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-outer-ttl");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_OUTER_TTL;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("lookup-by-outer");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid0");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 1nd l3 interface", tunnel_info.l3_inf[0], argv[index + 1]);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid1");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 2nd l3 interface", tunnel_info.l3_inf[1], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid2");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 3rd l3 interface", tunnel_info.l3_inf[2], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3ifid3");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("The 4th l3 interface", tunnel_info.l3_inf[3], argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("more-rpf");
        if (index != 0xFF)
        {
            tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_MORE_RPF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-outer-info");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("follow-port");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("qos-use-outer-info");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_QOS_USE_OUTER_INFO;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("qos-follow-port");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_QOS_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("policer-id", tunnel_info.policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("service-id", tunnel_info.service_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("logic-port", tunnel_info.logic_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("metadata", tunnel_info.metadata, argv[index + 1]);
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_METADATA_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-acl-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-queue-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_QUEUE_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-policer-en");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_SERVICE_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("stats", tunnel_info.stats_id, argv[index + 1]);
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("discard");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_info.scl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("cid", tunnel_info.cid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check-disable");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE;
    }

    CTC_CLI_IP_TUNNEL_ACL_PARAM_SET();
    if (cnt > 0)
    {
        tunnel_info.acl_lkup_num = cnt;
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_ACL_EN;
        sal_memcpy(tunnel_info.acl_prop,acl_property, sizeof(ctc_acl_property_t)* cnt);
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-map");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_info.flag, CTC_IPUC_TUNNEL_FLAG_QOS_MAP);
    }
    CTC_CLI_IPUC_QOS_MAP_SET();
    sal_memcpy(&tunnel_info.qos_map, &qos_map, sizeof(qos_map));

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add_tunnel(g_api_lchip, &tunnel_info);
    }
    else
    {
        ret = ctc_ipuc_add_tunnel(&tunnel_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_remove_ipv4,
        ctc_cli_ipuc_remove_ipv4_cmd,
        "ipuc remove VRFID A.B.C.D MASK_LEN NHID ( ecmp |) {is-neighbor |is-public | use-lpm |} (l4-dst-port L4PORT (tcp-port|)|)",
        CTC_CLI_IPUC_M_STR,
        "Remove route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        CTC_CLI_NH_ID_STR,
        "ECMP group",
        "Neighbor route",
        "Public route",
        "Host route use lpm",
        "Layer4 destination port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "If set, indicate it is a tcp port, or is a udp port")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint32 vrfid = 0;
    uint32 masklen = 0;
    ctc_ipuc_param_t ipuc_info = {0};

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    ipuc_info.masklen = masklen;

    CTC_CLI_GET_UINT32("nexthop id", ipuc_info.nh_id, argv[3]);

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

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-neighbor");
    if(index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-public");
    if(index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_remove(g_api_lchip, &ipuc_info);
    }
    else
    {
        ret = ctc_ipuc_remove(&ipuc_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_remove_ipv6,
        ctc_cli_ipuc_remove_ipv6_cmd,
        "ipuc ipv6 remove VRFID X:X::X:X MASK_LEN NHID ( ecmp |) {is-neighbor | is-public | use-lpm |}",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        "Remove ipv6 route",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        CTC_CLI_NH_ID_STR,
        "ECMP group",
        "Neighbor route",
        "Public route",
        "Host route use lpm")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint32 vrfid = 0;
    uint32 masklen = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info = {0};

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

    CTC_CLI_GET_UINT32("nexthop id", ipuc_info.nh_id, argv[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-neighbor");
    if(index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-public");
    if(index != 0xFF)
    {
        ipuc_info.route_flag = CTC_IPUC_FLAG_PUBLIC_ROUTE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-lpm");
    if (index != 0xFF)
    {
        ipuc_info.route_flag |= CTC_IPUC_FLAG_HOST_USE_LPM;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_remove(g_api_lchip, &ipuc_info);
    }
    else
    {
        ret = ctc_ipuc_remove(&ipuc_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_ipuc_add_ipv4_nat_sa,
        ctc_cli_ipuc_add_ipv4_nat_sa_cmd,
        "ipuc nat-sa add VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|) (new-ipsa A.B.C.D) (new-l4-src-port L4PORT|)",
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        "Add",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        "Layer4 source port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "Indicate l4-src-port is tcp port or not. If set is TCP port else is UDP port",
        "New ipsa",
        CTC_CLI_IPV4_FORMAT,
        "Layer4 destination port for NAPT",
        CTC_CLI_L4_PORT_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint8 index = 0;
    ctc_ipuc_nat_sa_param_t nat_info = {0};

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

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("new-ipsa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("new ip", nat_info.new_ipsa.ipv4, argv[index+1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("new-l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("new-l4-src-port", nat_info.new_l4_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add_nat_sa(g_api_lchip, &nat_info);
    }
    else
    {
        ret = ctc_ipuc_add_nat_sa(&nat_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_ipuc_remove_ipv4_nat_sa,
        ctc_cli_ipuc_remove_ipv4_nat_sa_cmd,
        "ipuc nat-sa remove VRFID A.B.C.D (l4-src-port L4PORT (tcp-port|)|)",
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        "Remove",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        "Layer4 source port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "If set, indicate it is a tcp port, or is a udp port")
{
    int32 ret = CLI_SUCCESS;
    uint32 vrfid = 0;
    uint8 index = 0;
    ctc_ipuc_nat_sa_param_t nat_info = {0};

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

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_remove_nat_sa(g_api_lchip, &nat_info);
    }
    else
    {
        ret = ctc_ipuc_remove_nat_sa(&nat_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_remove_ipv4_tunnel,
        ctc_cli_ipuc_remove_ipv4_tunnel_cmd,
        "ipuc tunnel remove A.B.C.D ({tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY  | ip-sa A.B.C.D | use-flex | rpf-check | scl-id SCL_ID} | ) ",
        CTC_CLI_IPUC_M_STR,
        "This route is ip tunnel",
        "Remove ip tunnel route",
        CTC_CLI_IPV4_FORMAT,
        "Tunnel payload layer 4 type (default is IPv6)",
        "Tunnel payload layer 4 type: 0 for GRE, 1 for IPv4, 2 for IPv6",
        "Set Gre key value",
        "Gre key value",
        "Set IP Sa",
        CTC_CLI_IPV4_FORMAT,
        "Use tcam key",
        "RPF check key need remove",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_ipuc_tunnel_param_t tunnel_info = {0};

    tunnel_info.payload_type = 2;
    CTC_CLI_GET_IPV4_ADDRESS("ip", tunnel_info.ip_da.ipv4, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel payload type", tunnel_info.payload_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-key");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY;
        CTC_CLI_GET_UINT32_RANGE("Gre key", tunnel_info.gre_key, argv[index + 1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA;
        CTC_CLI_GET_IPV4_ADDRESS("IP Sa", tunnel_info.ip_sa.ipv4, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_info.scl_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_remove_tunnel(g_api_lchip, &tunnel_info);
    }
    else
    {
        ret = ctc_ipuc_remove_tunnel(&tunnel_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_remove_ipv6_tunnel,
        ctc_cli_ipuc_remove_ipv6_tunnel_cmd,
        "ipuc ipv6 tunnel remove X:X::X:X ({tunnel-payload-type TUNNEL_PAYLOAD_TYPE | gre-key GREKEY  | ip-sa X:X::X:X | rpf-check | scl-id SCL_ID | use-flex} | )",
        CTC_CLI_IPUC_M_STR,
        "IPv6 route",
        "This route is ip tunnel",
        "Remove ip tunnel route",
        CTC_CLI_IPV6_FORMAT,
        "Tunnel payload layer 4 type (default is IPv4)",
        "Tunnel payload layer 4 type: \r\n0 for GRE \r\n1 for IPv4 \r\n2 for IPv6",
        "Set Gre key value",
        "Gre key value",
        "Set IP Sa",
        CTC_CLI_IPV6_FORMAT,
        "RPF check key need remove",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Use tcam key")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_tunnel_param_t tunnel_info = {0};

    tunnel_info.ip_ver = CTC_IP_VER_6;
    tunnel_info.payload_type = 1;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);
    /* adjust endian */
    tunnel_info.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_info.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_info.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_info.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel payload type", tunnel_info.payload_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-key");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY;
        CTC_CLI_GET_UINT32_RANGE("Gre key", tunnel_info.gre_key, argv[index + 1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA;
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        /* adjust endian */
        tunnel_info.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_info.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_info.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_info.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_info.scl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_info.flag |= CTC_IPUC_TUNNEL_FLAG_USE_FLEX;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_remove_tunnel(g_api_lchip, &tunnel_info);
    }
    else
    {
        ret = ctc_ipuc_remove_tunnel(&tunnel_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_add_default,
        ctc_cli_ipuc_add_default_cmd,
        "ipuc (ipv6 | ) default NHID",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        "Default route",
        CTC_CLI_NH_ID_STR)
{
    int32 ret = CLI_SUCCESS;
    uint32 nh_id = 0;
    uint8 ip_ver;

    CTC_CLI_GET_UINT32("nexthop id", nh_id, argv[argc - 1]);
    if (argc == 1)
    {
        ip_ver = CTC_IP_VER_4;
    }
    else
    {
        ip_ver = CTC_IP_VER_6;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_add_default_entry(g_api_lchip, ip_ver, nh_id);
    }
    else
    {
        ret = ctc_ipuc_add_default_entry(ip_ver, nh_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_set_global_property,
        ctc_cli_ipuc_set_global_property_cmd,
        "ipuc global-config {ipv4-martian-check (enable | disable) | ipv6-martian-check (enable | disable) | mcast-match-check (enable | disable) | ipv4-sa-search (enable | disable) | ipv6-sa-search (enable | disable) | ip-ttl-threshold THRESHOLD | async-en (enable | disable) | pending-list-len LENGTH}",
        CTC_CLI_IPUC_M_STR,
        "Global config for ip module",
        "Martian address check for ipv4",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Martian address check for ipv6",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Unmatched muticast MAC/IP address check",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "IPSA lookup for IPv4",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "IPSA lookup for IPv6",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Threshold for TTL check",
        "TTL threshold value ",
        "Async enable when config route",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Pending list length for async mode",
        "Length value")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint16 ip_ttl_threshold = 0;
    ctc_ipuc_global_property_t global_prop = { 0 };

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv4-martian-check");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_V4_MARTIAN_CHECK_EN;

        global_prop.v4_martian_check_en =
            (0 == sal_memcmp(argv[index + 1], "enable", sal_strlen("enable"))) ? 1 : 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6-martian-check");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_V6_MARTIAN_CHECK_EN;

        global_prop.v6_martian_check_en =
            (0 == sal_memcmp(argv[index + 1], "enable", sal_strlen("enable"))) ? 1 : 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast-match-check");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_MCAST_MACDA_IP_UNMATCH_CHECK;

        global_prop.mcast_match_check_en =
            (0 == sal_memcmp(argv[index + 1], "enable", sal_strlen("enable"))) ? 1 : 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-ttl-threshold");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_TTL_THRESHOLD;
        CTC_CLI_GET_UINT8("THRESHOLD", ip_ttl_threshold, argv[index + 1]);
        global_prop.ip_ttl_threshold = ip_ttl_threshold;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("async-en");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_ASYNC_EN;

        global_prop.async_en =
            (0 == sal_memcmp(argv[index + 1], "enable", sal_strlen("enable"))) ? 1 : 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("pending-list-len");
    if (0xFF != index)
    {
        global_prop.valid_flag |= CTC_IP_GLB_PROP_PENDING_LIST_LEN;
        CTC_CLI_GET_UINT32("pending list len", global_prop.pending_list_len, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_set_global_property(g_api_lchip, &global_prop);
    }
    else
    {
        ret = ctc_ipuc_set_global_property(&global_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_debug_on,
        ctc_cli_ipuc_debug_on_cmd,
        "debug ipuc (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_IPUC_M_STR,
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

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPUC_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = IPUC_SYS;
    }

    ctc_debug_set_flag("ip", "ipuc", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_debug_off,
        ctc_cli_ipuc_debug_off_cmd,
        "no debug ipuc (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_IPUC_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPUC_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = IPUC_SYS;
    }

    ctc_debug_set_flag("ip", "ipuc", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipuc_debug_show,
        ctc_cli_ipuc_debug_show_cmd,
        "show debug ipuc (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_IPUC_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPUC_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = IPUC_SYS;
    }
    en = ctc_debug_get_flag("ipuc", "ipuc", typeenum, &level);
    ctc_cli_out("ipuc:%s debug %s level:%s\n\r", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

STATIC int32
_ctc_cli_ipuc_traverse_fn(ctc_ipuc_param_t* p_ipuc_param, void* user_data)
{
    uint32  route_flag = 0;
    char buf[CTC_IPV6_ADDR_STR_LEN];
    char buf2[20] = {0};
    char buf3[20] = {0};
    char buf_s[1] = {0};
    uint8 is_ipuc_get = 0;

    is_ipuc_get = user_data?0:1;

    sal_sprintf(buf2, "/%d", p_ipuc_param->masklen);
    if (p_ipuc_param->l4_dst_port)
    {
        sal_sprintf(buf3, "/%d", p_ipuc_param->l4_dst_port);
    }


    ctc_cli_out("%-5d", p_ipuc_param->vrf_id);

   if (p_ipuc_param->ip_ver == CTC_IP_VER_4)
    {
        uint32 tempip = sal_ntohl(p_ipuc_param->ip.ipv4);
        sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, CTC_CLI_IPUC_MASK_LEN);
        sal_strncat(buf, buf3, CTC_CLI_IPUC_L4PORT_LEN);
        ctc_cli_out("%-30s", buf);
    }
    else
    {
        uint32 ipv6_address[4] = {0, 0, 0, 0};

        ipv6_address[0] = sal_ntohl(p_ipuc_param->ip.ipv6[0]);
        ipv6_address[1] = sal_ntohl(p_ipuc_param->ip.ipv6[1]);
        ipv6_address[2] = sal_ntohl(p_ipuc_param->ip.ipv6[2]);
        ipv6_address[3] = sal_ntohl(p_ipuc_param->ip.ipv6[3]);

        sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, CTC_CLI_IPUC_MASK_LEN);
        ctc_cli_out("%-44s", buf);
    }

    buf2[0] = '\0';
    route_flag = p_ipuc_param->route_flag;


    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_RPF_CHECK))
    {
        buf_s[0] = 'R';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_TTL_CHECK))
    {
        buf_s[0] = 'T';
        sal_strncat(buf2, buf_s, 1);
    }
    else if (!CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_TTL_NO_CHECK))
    {
        buf_s[0] = 'T';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_ICMP_CHECK))
    {
        buf_s[0] = 'I';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_NEIGHBOR))
    {
        buf_s[0] = 'N';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_CONNECT))
    {
        buf_s[0] = 'X';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_STATS_EN))
    {
        buf_s[0] = 'F';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_AGING_EN))
    {
        buf_s[0] = 'A';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_ASSIGN_PORT))
    {
        buf_s[0] = 'M';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_TO_FABRIC))
    {
        buf_s[0] = 'Y';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_CANCEL_NAT))
    {
        buf_s[0] = 'Z';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_ICMP_ERR_MSG_CHECK))
    {
        buf_s[0] = 'E';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_CPU))
    {
        buf_s[0] = 'C';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_PROTOCOL_ENTRY))
    {
        buf_s[0] = 'P';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, CTC_IPUC_FLAG_SELF_ADDRESS))
    {
        buf_s[0] = 'S';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE))
    {
        buf_s[0] = 'B';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
    {
        buf_s[0] = 'L';
        sal_strncat(buf2, buf_s, 1);
    }

    if ('\0' == buf2[0])
    {
        buf_s[0] = 'O';
        sal_strncat(buf2, buf_s, 1);
    }

    ctc_cli_out("   %-4d   %-5s",p_ipuc_param->nh_id, buf2);

    if (is_ipuc_get)
    {
        uint16 i = 0;
        ctc_cli_out("   %-4d      %-4d",p_ipuc_param->stats_id,p_ipuc_param->cid);
        if (p_ipuc_param->rpf_port_num)
        {
            ctc_cli_out("Interface for RPF check      :");
        }

        for (i = 0 ;i < p_ipuc_param->rpf_port_num; i++)
        {
            ctc_cli_out(" %d" , p_ipuc_param->rpf_port[i]);
        }
    }
    ctc_cli_out("\r\n");

    return CTC_E_NONE;
}


CTC_CLI(ctc_cli_ipuc_get,
        ctc_cli_ipuc_get_cmd,
        "show ipuc info (VRFID A.B.C.D MASK_LEN|ipv6 VRFID X:X::X:X MASK_LEN)",
        CTC_CLI_SHOW_STR,
        "Ipuc",
        "Ipuc Info",
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        CTC_CLI_IPV6_STR,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT)
{
    uint8 index = 0;
    int32 ret = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_param;

    sal_memset(&ipuc_param, 0 ,sizeof(ctc_ipuc_param_t));
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6");
    if (index != 0xFF)
    {
        ipuc_param.ip_ver = CTC_IP_VER_6;
        CTC_CLI_GET_IPV6_ADDRESS("ip", ipv6_address, argv[index + 2]);
        ipuc_param.ip.ipv6[0] = sal_htonl(ipv6_address[0]);
        ipuc_param.ip.ipv6[1] = sal_htonl(ipv6_address[1]);
        ipuc_param.ip.ipv6[2] = sal_htonl(ipv6_address[2]);
        ipuc_param.ip.ipv6[3] = sal_htonl(ipv6_address[3]);
        CTC_CLI_GET_UINT16_RANGE("vrfid", ipuc_param.vrf_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT8_RANGE("mask", ipuc_param.masklen, argv[index + 3], 0, CTC_MAX_UINT8_VALUE);
    }
    else
    {
        ipuc_param.ip_ver = CTC_IP_VER_4;
        CTC_CLI_GET_UINT16_RANGE("vrfid", ipuc_param.vrf_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_param.ip.ipv4, argv[1]);
        CTC_CLI_GET_UINT8_RANGE("mask", ipuc_param.masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }



    if (g_ctcs_api_en)
    {
        ret = ctcs_ipuc_get(g_api_lchip, &ipuc_param);
    }
    else
    {
        ret = ctc_ipuc_get(&ipuc_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_IPUC_PRINT_HEADER(ipuc_param.ip_ver);


    _ctc_cli_ipuc_traverse_fn(&ipuc_param, NULL);
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_ipuc_traverse,
        ctc_cli_ipuc_traverse_cmd,
        "ipuc traverse (ipv6|)",
        CTC_CLI_IPUC_M_STR,
        "Traverse",
        CTC_CLI_IPV6_STR)
{
    uint8 index = 0;
    int32 ret = 0;
    uint8 ip_ver = 0;
    uint8 data = 0;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6");
    if (index != 0xFF)
    {
        ip_ver = CTC_IP_VER_6;
    }
    else
    {
        ip_ver = CTC_IP_VER_4;
    }

    CTC_CLI_IPUC_PRINT_HEADER(ip_ver);

    if (g_ctcs_api_en)
    {
        ret = ctcs_ipuc_traverse(g_api_lchip,ip_ver, _ctc_cli_ipuc_traverse_fn, (void*)&data);
    }
    else
    {
        ret = ctc_ipuc_traverse(ip_ver, _ctc_cli_ipuc_traverse_fn, (void*)&data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_set_ipv4_hit,
        ctc_cli_usw_ipuc_set_ipv4_hit_cmd,
        "ipuc VRFID A.B.C.D MASK_LEN hit VALUE",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "Entry hit status",
        "HIT VALUE")
{
    int32 ret = CLI_ERROR;
	uint8 hit = 0;
	uint32 vrfid = 0;
    uint32 masklen = 0;
    ctc_ipuc_param_t ipuc_info = {0};
	 sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.ip_ver = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
	ipuc_info.masklen = masklen;
	CTC_CLI_GET_UINT8_RANGE("hit", hit, argv[3], 0, CTC_MAX_UINT8_VALUE);

	if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_set_entry_hit(g_api_lchip, &ipuc_info, hit);
    }
    else
    {
        ret = ctc_ipuc_set_entry_hit(&ipuc_info, hit);
    }
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_set_ipv6_hit,
        ctc_cli_usw_ipuc_set_ipv6_hit_cmd,
        "ipuc ipv6 VRFID X:X::X:X MASK_LEN hit VALUE",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "Entry hit status",
        "HIT VALUE")
{
    int32 ret = CLI_ERROR;
	uint32 vrfid = 0;
    uint32 masklen = 0;
	uint8 hit = 0;
	ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info = {0};
	sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
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
	CTC_CLI_GET_UINT8_RANGE("hit", hit, argv[3], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_set_entry_hit(g_api_lchip, &ipuc_info, hit);
    }
    else
    {
        ret = ctc_ipuc_set_entry_hit(&ipuc_info, hit);
    }
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv4_hit,
        ctc_cli_usw_ipuc_show_ipv4_hit_cmd,
        "show ipuc VRFID A.B.C.D MASK_LEN hit",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_LEN_FORMAT,
        "Entry hit status")
{
    int32 ret = CLI_ERROR;
	uint8 hit = 0;
	uint32 vrfid = 0;
    uint32 masklen = 0;
    ctc_ipuc_param_t ipuc_info = {0};
	sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.ip_ver = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    ipuc_info.vrf_id = vrfid;

    CTC_CLI_GET_IPV4_ADDRESS("ip", ipuc_info.ip.ipv4, argv[1]);

    CTC_CLI_GET_UINT8_RANGE("mask", masklen, argv[2], 0, CTC_MAX_UINT8_VALUE);
	ipuc_info.masklen = masklen;

     if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_get_entry_hit(g_api_lchip, &ipuc_info, &hit);
    }
    else
    {
        ret = ctc_ipuc_get_entry_hit(&ipuc_info, &hit);
    }
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit value:%d \n\r", hit);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipuc_show_ipv6_hit,
        ctc_cli_usw_ipuc_show_ipv6_hit_cmd,
        "show ipuc ipv6 VRFID X:X::X:X MASK_LEN hit",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_LEN_FORMAT,
        "Entry hit status")
{
    int32 ret = CLI_ERROR;
	uint8 hit = 0;
	uint32 vrfid = 0;
    uint32 masklen = 0;
	ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t ipuc_info = {0};
	sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
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

     if(g_ctcs_api_en)
    {
        ret = ctcs_ipuc_get_entry_hit(g_api_lchip, &ipuc_info, &hit);
    }
    else
    {
        ret = ctc_ipuc_get_entry_hit(&ipuc_info, &hit);
    }
    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit value:%d \n\r", hit);
    return CLI_SUCCESS;
}



int32
ctc_ipuc_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_ipv4_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_ipv4_nat_sa_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_ipv6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_remove_ipv4_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_remove_ipv4_nat_sa_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_remove_ipv6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_ipv4_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_ipv6_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_remove_ipv4_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_remove_ipv6_tunnel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_add_default_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipuc_set_global_property_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_ipuc_debug_on_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_ipuc_debug_off_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_ipuc_debug_show_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_ipuc_get_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_ipuc_traverse_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_set_ipv4_hit_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_set_ipv6_hit_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv4_hit_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_usw_ipuc_show_ipv6_hit_cmd);

    return CLI_SUCCESS;
}

