#if (FEATURE_MODE == 0)
/**
 @file ctc_wlan_cli.c

 @author  Copyright (C) 2016 Centec Networks Inc.  All rights reserved.

 @date 2016-02-22

 @version v2.0

 The file apply clis of bpe module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_wlan.h"
#include "ctc_wlan_cli.h"

#define CTC_CLI_WLAN_ACL_STR \
    "acl-property {\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|}"

#define CTC_CLI_WLAN_QOS_MAP_STR \
    "qos-map {priority PRIORITY | color COLOR | dscp DSCP | trust-dscp | (trust-scos | trust-ccos) | cos-domain DOMAIN | dscp-domain DOMAIN}"

#define CTC_CLI_WLAN_QOS_MAP_DESC \
     "QoS map",   \
     "Priority",  \
     "<0-0xF>",   \
     "Color",     \
     CTC_CLI_COLOR_VALUE,  \
     "Dscp",  \
     CTC_CLI_DSCP_VALUE, \
     "Trust dscp",   \
     "Trust scos",   \
     "Trust ccos",   \
     "Cos domain",   \
     "VALUE",        \
     "Dscp domain",  \
     "VALUE"

#define CTC_CLI_WLAN_ACL_PARAM_DESC \
        "Acl priority",\
        "priority",\
        "priority value", \
        "Acl lookup type",\
        "type value",\
        "acl use class id",\
        "CLASS_ID",\
        "acl use port-bitmap",\
        "acl use metadata",\
        "acl use logic-port",\
        "acl use mapped vlan"

#define CTC_CLI_WLAN_ACL_PARAM_SET() \
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

#define CTC_CLI_WLAN_QOS_MAP_SET() \
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

#define CTC_CLI_WLAN_TUNNEL_PARAM_STR \
    "({split-mac|decrypt ID | reassemble-en | ttl-check | tunnel-type (wtp2ac|ac2ac) | default-vlan-id VLAN | \
     logic-port LPORT |policer-id POLICER_ID | service-acl-en| resove-hash-conflict}|) {"CTC_CLI_WLAN_ACL_STR"|"CTC_CLI_WLAN_QOS_MAP_STR"|}"

#define CTC_CLI_WLAN_TUNNEL_PARAM_DESC \
        "Split mac mode", \
        "Decrypt enable",\
        "Crypt id",\
        "Reassemble enable",\
        "Do ttl check for tunnel outer header",\
        "Tunnel type",\
        "wtp to ac",\
        "ac to ac",\
        "Default vlan id",\
        "Vlan value",\
        "Logic port",\
        "Logic port value",\
        "Policer Id",\
        "<1-0xFFFF>",\
        "Enable service acl",\
        "Use flex key",\
        "Acl property", \
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_QOS_MAP_DESC

#define CTC_CLI_WLAN_CRYPT_DESC \
        "Aes key", \
        "Aes key value", \
        "Sha key", \
        "Sha key value", \
        "Seq num", \
        "Seq num value", \
        "Epoch", \
        "Epoch value"

#define CTC_CLI_WLAN_TUNNEL_PARAM_SET()  \
    index = CTC_CLI_GET_ARGC_INDEX("split-mac");                             \
    if (INDEX_VALID(index))                                                   \
    {                                                                           \
       tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_SPLIT_MAC;                     \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("decrypt");                               \
    if (INDEX_VALID(index))                                                   \
    {                                                                           \
       CTC_CLI_GET_UINT8("decrypt", tunnel_param.decrypt_id, argv[index + 1]); \
       tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_DECRYPT_ENABLE;                \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("reassemble-en");                         \
    if (INDEX_VALID(index))                                                   \
    {                                                                           \
       tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_REASSEMBLE_ENABLE;             \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("ttl-check");                             \
    if (0xFF != index)                                                          \
    {                                                                           \
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_TTL_CHECK;                    \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("tunnel-type");                           \
    if (0xFF != index)                                                          \
    {                                                                           \
        if (CLI_CLI_STR_EQUAL("wtp2ac", (index+1)))                           \
        {                                                                       \
            tunnel_param.type = CTC_WLAN_TUNNEL_TYPE_WTP_2_AC;                  \
        }                                                                       \
        else if (CLI_CLI_STR_EQUAL("ac2ac", (index+1)))                      \
        {                                                                       \
            tunnel_param.type = CTC_WLAN_TUNNEL_TYPE_AC_2_AC;                   \
        }                                                                       \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("default-vlan-id");                       \
    if (0xFF != index)                                                          \
    {                                                                           \
        CTC_CLI_GET_UINT16("default-vlan-id", tunnel_param.default_vlan_id, argv[index + 1]);  \
    }                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");                             \
    if (0xFF != index)                                                           \
    {                                                                            \
        CTC_CLI_GET_UINT16("logic-port", tunnel_param.logic_port, argv[index + 1]);  \
    }                                                                             \
    index = CTC_CLI_GET_ARGC_INDEX("policer-id");                               \
    if (INDEX_VALID(index))                                                      \
    {                                                                              \
        CTC_CLI_GET_UINT16("policer-id", tunnel_param.policer_id, argv[index + 1]); \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-acl-en");                  \
    if (index != 0xFF)                                                             \
    {                                                                              \
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_SERVICE_ACL_EN;                  \
    }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resove-hash-conflict");                                 \
    if (index != 0xFF)                                                                      \
    {                                                                                       \
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT;                    \
    }

#define CTC_CLI_WLAN_CLIENT_PARAM_STR \
    "(sta STA) (is-local (local|remote tunnel-type (wtp2ac|ac2ac) nexthop-id NHID)) (roam-status (no-roam|pop|poa)) ({vrfid VRFID |\
    mulitcast-en (src-vlan-id VLANID)| stats STATS_ID | policer-id POLICER_ID | vlan-id VLAN | cid CID | logic-port LPORT| \
    resove-hash-conflict}|)("CTC_CLI_WLAN_ACL_STR"| "CTC_CLI_WLAN_QOS_MAP_STR"|)"

#define CTC_CLI_WLAN_CLIENT_PARAM_DESC \
        "Sta",\
        "Sta mac value",\
        "Client is local or remote",\
        "Client is local",\
        "Client roam from remote",\
        "Tunnel type",\
        "Wtp to ac",\
        "Ac to ac",\
        "Nexthop id",\
        "Nexthop id",\
        "Client Roam status",\
        "Client no roam",\
        "Current AC is pop",\
        "Current AC is poa",\
        "Vrfid for L3",\
        "Vrfid value",\
        "Mulitcast enable",\
        "Src vlan id",\
        "Vlan id value",\
        "Stats",\
        "Stats id",\
        "Policer enable",\
        "Policer id",\
        "Vlan, for sta classify",\
        "Vlan value",\
        "Cid",\
        "Cid value",\
        "Logic port",\
        "Logic port value",\
        "Use flex key",\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_ACL_PARAM_DESC,\
        CTC_CLI_WLAN_QOS_MAP_DESC


#define CTC_CLI_WLAN_CLIENT_PARAM_SET()                                     \
    index = CTC_CLI_GET_ARGC_INDEX("sta");                                   \
    if (INDEX_VALID(index))                                                   \
    {                                                                           \
        CTC_CLI_GET_MAC_ADDRESS("sta", client_param.sta, argv[index+1]);     \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("is-local");                           \
    if (0xFF != index)                                                          \
    {                                                                           \
        if (CLI_CLI_STR_EQUAL("local", (index+1)))                           \
        {                                                                       \
            client_param.is_local = 1;                                          \
        }                                                                       \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("tunnel-type");                           \
    if (0xFF != index)                                                          \
    {                                                                           \
        if (CLI_CLI_STR_EQUAL("wtp2ac", (index+1)))                           \
        {                                                                       \
            client_param.tunnel_type = CTC_WLAN_TUNNEL_TYPE_WTP_2_AC;           \
        }                                                                       \
        else if (CLI_CLI_STR_EQUAL("ac2ac", (index+1)))                      \
        {                                                                       \
            client_param.tunnel_type = CTC_WLAN_TUNNEL_TYPE_AC_2_AC;            \
        }                                                                       \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");                           \
    if (0xFF != index)                                                  \
    {                                                                           \
        CTC_CLI_GET_UINT16("vrfid", client_param.vrfid, argv[index+1]); \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("nexthop-id");                            \
    if (0xFF != index)                                                          \
    {                                                                           \
        CTC_CLI_GET_UINT32("nexthop-id", client_param.nh_id, argv[index+1]);   \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("roam-status");                            \
    if (0xFF != index)                                                          \
    {                                                                           \
         if (CLI_CLI_STR_EQUAL("pop", (index+1)))                           \
        {                                                                       \
            client_param.roam_status = 2;                                       \
        }                                                                       \
        else if (CLI_CLI_STR_EQUAL("poa", (index+1)))                      \
        {                                                                       \
            client_param.roam_status = 1;                                       \
        }                                                                       \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("mulitcast-en");                          \
    if (0xFF != index)                                                          \
    {                                                                           \
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY;                  \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("src-vlan-id");                           \
    if (0xFF != index)                                                  \
    {                                                                           \
        CTC_CLI_GET_UINT16("src vlan id", client_param.src_vlan_id, argv[index+1]); \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("stats");                                 \
    if (0xFF != index)                                                          \
    {                                                                           \
        CTC_CLI_GET_UINT16("stats", client_param.stats_id, argv[index + 1]);  \
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_STATS_EN;                     \
    }                                                                           \
    index = CTC_CLI_GET_ARGC_INDEX("policer-id");                            \
    if (0xFF != index)                                                          \
    {                                                                           \
        CTC_CLI_GET_UINT16("policer-id", client_param.policer_id, argv[index + 1]); \
    }                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-id");                                \
    if (0xFF != index)                                                           \
    {                                                                            \
        CTC_CLI_GET_UINT16("vlan-id", client_param.vlan_id, argv[index + 1]);  \
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_MAPPING_VLAN;                  \
    }                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("cid");                                    \
    if (0xFF != index)                                                           \
    {                                                                            \
        CTC_CLI_GET_UINT16("cid", client_param.cid, argv[index + 1]);          \
    }                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");                             \
    if (0xFF != index)                                                           \
    {                                                                            \
        CTC_CLI_GET_UINT16("logic-port", client_param.logic_port, argv[index + 1]);  \
    }                                                                             \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resove-hash-conflict");                                     \
    if (index != 0xFF)                                                                      \
    {                                                                                       \
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT;                    \
    }


CTC_CLI(ctc_cli_wlan_tunnel_add,
        ctc_cli_wlan_tunnel_add_cmd,
        "wlan tunnel (add|update) (mode MODE|) ipsa A.B.C.D ipda A.B.C.D l4-port L4PORT (bssid BSSID|) (radio-id RID|)"CTC_CLI_WLAN_TUNNEL_PARAM_STR,
        CTC_CLI_WLAN_M_STR,
        "Tunnel",
        CTC_CLI_ADD,
        "Update operation",
        "Tunnel mode, default network",
        "Tunnel mode 0:network,1:bssid,2:bssid+rid",
        "Source ip address",
        CTC_CLI_IPV4_FORMAT,
        "Destination ip address",
        CTC_CLI_IPV4_FORMAT,
        "L4 port",
        "L4 port value",
        "Bssid",
        CTC_CLI_MAC_FORMAT,
        "Radio id",
        "Radio id value",
        CTC_CLI_WLAN_TUNNEL_PARAM_DESC)
{
    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM];
    ctc_wlan_tunnel_t tunnel_param;
	ctc_scl_qos_map_t  qos_map;  /**/
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 is_update = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0,idx2 = 0;
    uint8 last_time = 0;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));
    sal_memset(acl_property, 0 ,sizeof(acl_property));
    sal_memset(&qos_map, 0 ,sizeof(qos_map));

    tunnel_param.mode = CTC_WLAN_TUNNEL_MODE_NETWORK;
    tunnel_param.type = CTC_WLAN_TUNNEL_TYPE_WTP_2_AC;

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        is_update = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", tunnel_param.mode, argv[index + 1]);
        index = 3;
    }
    else
    {
        index = 1;
    }

    CTC_CLI_GET_IPV4_ADDRESS("ipsa", tunnel_param.ipsa.ipv4, argv[index]);
    CTC_CLI_GET_IPV4_ADDRESS("ipda", tunnel_param.ipda.ipv4, argv[index+1]);
    CTC_CLI_GET_UINT16("l4-port", tunnel_param.l4_port, argv[index + 2]);

    index = CTC_CLI_GET_ARGC_INDEX("bssid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("bssid", tunnel_param.bssid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("radio-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("radio-id", tunnel_param.radio_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-map");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_WLAN_TUNNEL_FLAG_QOS_MAP);
    }

    CTC_CLI_WLAN_TUNNEL_PARAM_SET()
    CTC_CLI_WLAN_ACL_PARAM_SET()
    CTC_CLI_WLAN_QOS_MAP_SET()
    if((tunnel_param.acl_lkup_num = cnt) > 0)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_WLAN_TUNNEL_FLAG_ACL_EN);
        sal_memcpy(tunnel_param.acl_prop,acl_property,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
    }

    sal_memcpy(&tunnel_param.qos_map, &qos_map, sizeof(qos_map));

    if (is_update)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_update_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_wlan_update_tunnel(&tunnel_param);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_add_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_wlan_add_tunnel(&tunnel_param);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_tunnel_add_v6,
        ctc_cli_wlan_tunnel_add_v6_cmd,
        "wlan tunnel ipv6 (add|update) (mode MODE|) ipsa X:X::X:X ipda X:X::X:X  l4-port L4PORT (bssid BSSID|) (radio-id RID|)"CTC_CLI_WLAN_TUNNEL_PARAM_STR,
        CTC_CLI_WLAN_M_STR,
        "Tunnel",
        "Ipv6",
        CTC_CLI_ADD,
        "Update operation",
        "Tunnel mode, default network",
        "Tunnel mode 0:network,1:bssid,2:bssid+rid",
        "Source ip address",
        CTC_CLI_IPV6_FORMAT,
        "Destination ip address",
        CTC_CLI_IPV6_FORMAT,
        "L4 port",
        "L4 port value",
        "Bssid",
        CTC_CLI_MAC_FORMAT,
        "Radio id",
        "Radio id value",
        CTC_CLI_WLAN_TUNNEL_PARAM_DESC)
{
    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM];
    ctc_wlan_tunnel_t tunnel_param;
    ipv6_addr_t ipv6_address;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 is_update = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0,idx2 = 0;
    uint8 last_time = 0;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));
    sal_memset(acl_property, 0 ,sizeof(acl_property));

    tunnel_param.mode = CTC_WLAN_TUNNEL_MODE_NETWORK;
    tunnel_param.type = CTC_WLAN_TUNNEL_TYPE_WTP_2_AC;
    tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        is_update = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", tunnel_param.mode, argv[index + 1]);
        index = 3;
    }
    else
    {
        index = 1;
    }

    CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[index]);
    /* adjust endian */
    tunnel_param.ipsa.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_param.ipsa.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_param.ipsa.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_param.ipsa.ipv6[3] = sal_htonl(ipv6_address[3]);
    CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index + 1]);
    /* adjust endian */
    tunnel_param.ipda.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_param.ipda.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_param.ipda.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_param.ipda.ipv6[3] = sal_htonl(ipv6_address[3]);
    CTC_CLI_GET_UINT16("l4-port", tunnel_param.l4_port, argv[index + 2]);

    index = CTC_CLI_GET_ARGC_INDEX("bssid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("bssid", tunnel_param.bssid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("radio-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("radio-id", tunnel_param.radio_id, argv[index + 1]);
    }
    CTC_CLI_WLAN_TUNNEL_PARAM_SET()
    CTC_CLI_WLAN_ACL_PARAM_SET()
    if((tunnel_param.acl_lkup_num = cnt) > 0)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_WLAN_TUNNEL_FLAG_ACL_EN);
        sal_memcpy(tunnel_param.acl_prop,acl_property,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
    }

    if (is_update)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_update_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_wlan_update_tunnel(&tunnel_param);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_add_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_wlan_add_tunnel(&tunnel_param);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_tunnel_remove,
        ctc_cli_wlan_tunnel_remove_cmd,
        "wlan tunnel remove (mode MODE|) ipsa A.B.C.D ipda A.B.C.D l4-port L4PORT {bssid BSSID (radio-id RID|)|resove-hash-conflict|}",
        CTC_CLI_WLAN_M_STR,
        "Tunnel",
        "Remove operation",
        "Tunnel mode, default network",
        "Tunnel mode 0:network,1:bssid,2:bssid+rid",
        "Source ip address",
        CTC_CLI_IPV4_FORMAT,
        "Destination ip address",
        CTC_CLI_IPV4_FORMAT,
        "L4 port",
        "L4 port value",
        "Bssid",
        CTC_CLI_MAC_FORMAT,
        "Radio id",
        "Radio id value",
        "Use flex key")
{
    ctc_wlan_tunnel_t tunnel_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));

    tunnel_param.mode = CTC_WLAN_TUNNEL_MODE_NETWORK;

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", tunnel_param.mode, argv[index + 1]);
        index = 2;
    }
    else
    {
        index = 0;
    }

    CTC_CLI_GET_IPV4_ADDRESS("ipsa", tunnel_param.ipsa.ipv4, argv[index]);
    CTC_CLI_GET_IPV4_ADDRESS("ipda", tunnel_param.ipda.ipv4, argv[index + 1]);
    CTC_CLI_GET_UINT16("l4-port", tunnel_param.l4_port, argv[index + 2]);

    index = CTC_CLI_GET_ARGC_INDEX("bssid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("bssid", tunnel_param.bssid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("radio-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("radio-id", tunnel_param.radio_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resove-hash-conflict");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_wlan_remove_tunnel(g_api_lchip, &tunnel_param);
    }
    else
    {
        ret = ctc_wlan_remove_tunnel(&tunnel_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_tunnel_remove_v6,
        ctc_cli_wlan_tunnel_remove_v6_cmd,
        "wlan tunnel ipv6 remove (mode MODE|) ipsa X:X::X:X ipda X:X::X:X  l4-port L4PORT {bssid BSSID (radio-id RID|)|resove-hash-conflict|}",
        CTC_CLI_WLAN_M_STR,
        "Tunnel",
        CTC_CLI_IPV6_STR,
        "Remove operation",
        "Tunnel mode, default network",
        "Tunnel mode value",
        "Source ip address",
        CTC_CLI_IPV6_FORMAT,
        "Destination ip address",
        CTC_CLI_IPV6_FORMAT,
        "L4 port",
        "L4 port value",
        "Bssid",
        "Bssid mac value",
        "Radio id",
        "Radio id value",
        "Use flex key")
{
    ctc_wlan_tunnel_t tunnel_param;
    ipv6_addr_t ipv6_address;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));

    tunnel_param.mode = CTC_WLAN_TUNNEL_MODE_NETWORK;
    tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", tunnel_param.mode, argv[index + 1]);
        index = 2;
    }
    else
    {
        index = 0;
    }

    CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[index]);
    /* adjust endian */
    tunnel_param.ipsa.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_param.ipsa.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_param.ipsa.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_param.ipsa.ipv6[3] = sal_htonl(ipv6_address[3]);
    CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index + 1]);
    /* adjust endian */
    tunnel_param.ipda.ipv6[0] = sal_htonl(ipv6_address[0]);
    tunnel_param.ipda.ipv6[1] = sal_htonl(ipv6_address[1]);
    tunnel_param.ipda.ipv6[2] = sal_htonl(ipv6_address[2]);
    tunnel_param.ipda.ipv6[3] = sal_htonl(ipv6_address[3]);
    CTC_CLI_GET_UINT16("l4-port", tunnel_param.l4_port, argv[index + 2]);

    index = CTC_CLI_GET_ARGC_INDEX("bssid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("bssid", tunnel_param.bssid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("radio-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("radio-id", tunnel_param.radio_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resove-hash-conflict");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_wlan_remove_tunnel(g_api_lchip, &tunnel_param);
    }
    else
    {
        ret = ctc_wlan_remove_tunnel(&tunnel_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_client_add,
        ctc_cli_wlan_client_add_cmd,
        "wlan client (add|update)"CTC_CLI_WLAN_CLIENT_PARAM_STR,
        CTC_CLI_WLAN_M_STR,
        "Client",
        CTC_CLI_ADD,
        "update operation",
        CTC_CLI_WLAN_CLIENT_PARAM_DESC)
{
    ctc_wlan_client_t client_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 is_update = 0;
    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM];
    uint8 cnt = 0;
    uint8 idx1 = 0,idx2 = 0;
    uint8 last_time = 0;
    ctc_scl_qos_map_t qos_map;

    sal_memset(&client_param, 0 ,sizeof(client_param));
    sal_memset(acl_property, 0 ,sizeof(acl_property));
    sal_memset(&qos_map, 0 ,sizeof(qos_map));

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        is_update = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-map");
    if (0xFF != index)
    {
        CTC_SET_FLAG(client_param.flag, CTC_WLAN_CLIENT_FLAG_QOS_MAP);
    }

    CTC_CLI_WLAN_CLIENT_PARAM_SET()
    CTC_CLI_WLAN_ACL_PARAM_SET()
    CTC_CLI_WLAN_QOS_MAP_SET()

    if((client_param.acl_lkup_num = cnt)>0)
    {
        CTC_SET_FLAG(client_param.flag, CTC_WLAN_CLIENT_FLAG_ACL_EN);
        sal_memcpy(client_param.acl,acl_property,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
    }

    sal_memcpy(&client_param.qos_map, &qos_map, sizeof(qos_map));

    if (is_update)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_update_client(g_api_lchip, &client_param);
        }
        else
        {
            ret = ctc_wlan_update_client(&client_param);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_wlan_add_client(g_api_lchip, &client_param);
        }
        else
        {
            ret = ctc_wlan_add_client(&client_param);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_client_remove,
        ctc_cli_wlan_client_remove_cmd,
        "wlan client remove sta STA (is-local (local|remote tunnel-type (wtp2ac|ac2ac))) {mulitcast-en (src-vlan-id VLANID) | resove-hash-conflict|}",
        CTC_CLI_WLAN_M_STR,
        "Client",
        "Remove operation",
        "Sta",
        "Sta mac value",
        "Client is local or remote",
        "Client is local",
        "Client roam from remote",
        "Tunnel type",
        "Wtp to ac",
        "Ac to ac",
        "Mulitcast enable",
        "Src vlan id",
        "Vlan id value",
        "Resove-hash-conflict key")
{
    ctc_wlan_client_t client_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&client_param, 0 ,sizeof(client_param));

    CTC_CLI_GET_MAC_ADDRESS("sta", client_param.sta, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("is-local");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("local", (index+1)))
        {
            client_param.is_local= 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("tunnel-type");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("wtp2ac", (index+1)))
        {
            client_param.tunnel_type = CTC_WLAN_TUNNEL_TYPE_WTP_2_AC;
        }
        else if (CLI_CLI_STR_EQUAL("ac2ac", (index+1)))
        {
            client_param.tunnel_type = CTC_WLAN_TUNNEL_TYPE_AC_2_AC;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("mulitcast-en");
    if (0xFF != index)
    {
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-vlan-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("src vlan id", client_param.src_vlan_id, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resove-hash-conflict");
    if (index != 0xFF)
    {
        client_param.flag |= CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_wlan_remove_client(g_api_lchip, &client_param);
    }
    else
    {
        ret = ctc_wlan_remove_client(&client_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_crypt_set,
        ctc_cli_wlan_crypt_set_cmd,
        "wlan crypt ID (encrypt|decrypt key-id KEY_ID)(set aes-key AES_KEY sha-key SHA_KEY seq-num SEQ_NUM epoch EPOCH |unset)",
        CTC_CLI_WLAN_M_STR,
        "Crypt",
        "Crypt id",
        "Encrypt",
        "Decrypt",
        "Key id",
        "Key id value",
        "Set crypt cfg",
        CTC_CLI_WLAN_CRYPT_DESC,
        "Unset crypt cfg")
{
    ctc_wlan_crypt_t crypt_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&crypt_param, 0, sizeof(crypt_param));

    crypt_param.type = CTC_WLAN_CRYPT_TYPE_ENCRYPT;

    CTC_CLI_GET_UINT8("crypt", crypt_param.crypt_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("decrypt");
    if (0xFF != index)
    {
        crypt_param.type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("key-id", crypt_param.key_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("set");
    if (0xFF != index)
    {
        crypt_param.key.valid = 1;
    }
    else
    {
        crypt_param.key.valid = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aes-key");
    if (0xFF != index)
    {
        if ( sal_strlen(argv[index+1]) <= WLAN_AES_KEY_LENGTH)
        {
            sal_memcpy((uint8*)&crypt_param.key.aes_key, argv[index+1], sal_strlen(argv[index+1]));
        }
        else
        {
            ctc_cli_out("%% %s \n\r", ctc_get_error_desc(CTC_E_INVALID_PARAM));
            return CLI_ERROR;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("sha-key");
    if (0xFF != index)
    {
        if ( sal_strlen(argv[index+1]) <= WLAN_SHA_KEY_LENGTH)
        {
            sal_memcpy((uint8*)&crypt_param.key.sha_key, argv[index+1],  sal_strlen(argv[index+1]));
        }
        else
        {
            ctc_cli_out("%% %s \n\r", ctc_get_error_desc(CTC_E_INVALID_PARAM));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("seq-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT64("seq-num", crypt_param.key.seq_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("epoch");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("epoch", crypt_param.key.epoch, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_wlan_set_crypt(g_api_lchip, &crypt_param);
    }
    else
    {
        ret = ctc_wlan_set_crypt(&crypt_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_crypt_show,
        ctc_cli_wlan_crypt_show_cmd,
        "show wlan crypt ID (type (encrypt|decrypt key-id KEY_ID))",
        "Show",
        CTC_CLI_WLAN_M_STR,
        "Crypt",
        "Crypt id",
        "Crypt type",
        "Encrypt",
        "Decrypt",
        "Key id",
        "Key id value")
{
    ctc_wlan_crypt_t crypt_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 temp_aes_key[WLAN_AES_KEY_LENGTH+1];
    uint8 temp_sha_key[WLAN_SHA_KEY_LENGTH+1];

    sal_memset(&crypt_param, 0, sizeof(crypt_param));
    sal_memset(temp_aes_key, 0, WLAN_AES_KEY_LENGTH+1);
    sal_memset(temp_sha_key, 0, WLAN_SHA_KEY_LENGTH+1);

    crypt_param.type = CTC_WLAN_CRYPT_TYPE_ENCRYPT;

    CTC_CLI_GET_UINT8("crypt", crypt_param.crypt_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("type");
    if (0xFF != index)
    {
        if (CLI_CLI_STR_EQUAL("encrypt", (index+1)))
        {
            crypt_param.type = 0;
        }
        else if (CLI_CLI_STR_EQUAL("decrypt", (index+1)))
        {
            crypt_param.type = 1;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("key-id", crypt_param.key_id, argv[index+1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_wlan_get_crypt(g_api_lchip, &crypt_param);
    }
    else
    {
        ret = ctc_wlan_get_crypt(&crypt_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("wlan crypt %d type %s  \n\r", crypt_param.crypt_id, crypt_param.type?"decrypt":"encrypt");
        if (CTC_WLAN_CRYPT_TYPE_ENCRYPT == crypt_param.type)
        {
            sal_memcpy(temp_aes_key, crypt_param.key.aes_key, WLAN_AES_KEY_LENGTH);
            ctc_cli_out("%-45s:  %s\n", "aes-key", temp_aes_key);
            sal_memcpy(temp_sha_key, crypt_param.key.sha_key, WLAN_SHA_KEY_LENGTH);
            ctc_cli_out("%-45s:  %s\n", "sha-key", temp_sha_key);
            ctc_cli_out("%-45s:  %"PRIu64"\n", "seq-num", crypt_param.key.seq_num);
            ctc_cli_out("%-45s:  %d\n", "epoch", crypt_param.key.epoch);
        }
        else
        {
            ctc_cli_out("%% key-id:%d \n\r",crypt_param.key_id);
            sal_memcpy(temp_aes_key, crypt_param.key.aes_key, WLAN_AES_KEY_LENGTH);
            ctc_cli_out("%-45s:  %s\n", "aes-key", temp_aes_key);
            sal_memcpy(temp_sha_key, crypt_param.key.sha_key, WLAN_SHA_KEY_LENGTH);
            ctc_cli_out("%-45s:  %s\n", "sha-key", temp_sha_key);
            ctc_cli_out("%-45s:  %"PRIu64"\n", "seq-num", crypt_param.key.seq_num);
            ctc_cli_out("%-45s:  %d\n", "epoch", crypt_param.key.epoch);
            ctc_cli_out("%-45s:  %u\n", "valid", crypt_param.key.valid);
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_global_config,
        ctc_cli_wlan_global_config_cmd,
        "wlan global {ctl-pkt-decrypt-en VALUE|dtls-version VALUE|fc-swap-en VALUE|l4-port0 VALUE0|l4-port1 VALUE1|default-local-client-action (mapping-vlan VLANID|discard)}",
        CTC_CLI_WLAN_M_STR,
        "Global",
        "Control packet decrypt",
        "0:disable,1:enable",
        "DTLS version",
        "0xFEFF:DTLS1.0 0xFEFD:DTLS1.2",
        "Frame control swap enable",
        "0:disable, 1:enable",
        "L4 dest port 0",
        "Port 0 value",
        "L4 dest port 1",
        "Port 1 value",
        "Default client action(default:discard)",
        "Mapping Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Discard")
{
    ctc_wlan_global_cfg_t glb_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&glb_param, 0, sizeof(ctc_wlan_global_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_wlan_get_global_cfg(g_api_lchip, &glb_param);
    }
    else
    {
        ret = ctc_wlan_get_global_cfg(&glb_param);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctl-pkt-decrypt-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("ctl pkt decrypt en", glb_param.control_pkt_decrypt_en, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dtls-version");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("dtls version", glb_param.dtls_version, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("fc-swap-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("fc swap en", glb_param.fc_swap_enable, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-port0");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("l4 port0", glb_param.udp_dest_port0, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-port1");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("l4 port1", glb_param.udp_dest_port1, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("default-local-client-action");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("mapping-vlan");
        if (0xFF != index)
        {
            glb_param.default_client_action = 1;
            CTC_CLI_GET_UINT16("mapping vlan", glb_param.default_client_vlan_id, argv[index + 1]);
        }
        else
        {
            glb_param.default_client_action = 0;
            glb_param.default_client_vlan_id = 0;
        }
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_wlan_set_global_cfg(g_api_lchip, &glb_param);
    }
    else
    {
        ret = ctc_wlan_set_global_cfg(&glb_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_show_global_config,
        ctc_cli_wlan_show_global_config_cmd,
        "show wlan global",
        CTC_CLI_SHOW_STR,
        CTC_CLI_WLAN_M_STR,
        "Global")
{
    int32 ret = CLI_SUCCESS;
    ctc_wlan_global_cfg_t glb_param;

    sal_memset(&glb_param, 0, sizeof(ctc_wlan_global_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_wlan_get_global_cfg(g_api_lchip, &glb_param);
    }
    else
    {
        ret = ctc_wlan_get_global_cfg(&glb_param);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-30s%s\n", "wlan global config", "Value");
    ctc_cli_out("----------------------\n");

    ctc_cli_out("%-30s%s\n", "control decrypt", glb_param.control_pkt_decrypt_en?"enable":"disable");
    ctc_cli_out("%-30s%s\n", "roam type", "pop/poa");
    ctc_cli_out("%-30s0x%x\n", "dtls version", glb_param.dtls_version);
    ctc_cli_out("%-30s%s\n", "fc swap", glb_param.fc_swap_enable?"enable":"disable");
    ctc_cli_out("%-30s%d\n", "l4 port 0", glb_param.udp_dest_port0);
    ctc_cli_out("%-30s%d\n", "l4 port 1", glb_param.udp_dest_port1);
    if (CTC_WLAN_DEFAULT_CLIENT_ACTION_DISCARD == glb_param.default_client_action)
    {
        ctc_cli_out("%-s\n", "default local client action: discard");
    }
    else if(CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN == glb_param.default_client_action)
    {
        ctc_cli_out("%-s %d\n", "default local client action: mapping vlan", glb_param.default_client_vlan_id);
    }
    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_debug_on,
        ctc_cli_wlan_debug_on_cmd,
        "debug wlan (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_WLAN_M_STR,
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
    else
    {
        level = CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = WLAN_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = WLAN_SYS;
    }

    ctc_debug_set_flag("wlan", "wlan", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_debug_off,
        ctc_cli_wlan_debug_off_cmd,
        "no debug wlan (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_WLAN_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = WLAN_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = WLAN_SYS;
    }

    ctc_debug_set_flag("wlan", "wlan", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_wlan_debug_show,
        ctc_cli_wlan_debug_show_cmd,
        "show debug wlan (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_WLAN_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = WLAN_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = WLAN_SYS;
    }

    en = ctc_debug_get_flag("wlan", "wlan", typeenum, &level);
    ctc_cli_out("wlan:%s debug %s level %s\n\r", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}


int32
ctc_wlan_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_tunnel_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_tunnel_add_v6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_tunnel_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_tunnel_remove_v6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_client_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_client_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_crypt_set_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_crypt_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_global_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_show_global_config_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_wlan_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_wlan_debug_show_cmd);

    return CLI_SUCCESS;
}

#endif

