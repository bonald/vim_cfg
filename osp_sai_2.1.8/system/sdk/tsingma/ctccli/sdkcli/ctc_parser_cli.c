/**
 @file ctc_parser_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-28

 @version v2.0

This file contains parser module cli command
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_parser_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_parser.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_linkagg.h"

#define CTC_CLI_PARSER_UDF_SET(udf)                                          \
    index = CTC_CLI_GET_ARGC_INDEX("layer3");                                \
    if (index != 0xFF)                                                       \
    {                                                                        \
        udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;                               \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("layer4");                                \
    if (index != 0xFF)                                                       \
    {                                                                        \
        udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;                               \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");                            \
    if (index != 0xFF)                                                       \
    {                                                                        \
        CTC_CLI_GET_UINT16("ether-type", udf.ether_type, argv[index + 1]);   \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("ipv4");                                  \
    if (index != 0xFF)                                                       \
    {                                                                        \
        udf.ip_version = CTC_IP_VER_4;                                       \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");                                  \
    if (index != 0xFF)                                                       \
    {                                                                        \
        udf.ip_version = CTC_IP_VER_6;                                       \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                  \
    if (index != 0xFF)                                                       \
    {                                                                        \
        CTC_CLI_GET_UINT8("l4-protocol", udf.l3_header_protocol, argv[index + 1]); \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");                           \
    if (index != 0xFF)                                                       \
    {                                                                        \
        CTC_CLI_GET_UINT16("l4-src-port", udf.l4_src_port, argv[index + 1]); \
        udf.is_l4_src_port = 1;                                              \
    }                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");                           \
    if (index != 0xFF)                                                       \
    {                                                                        \
        CTC_CLI_GET_UINT16("l4-dst-port", udf.l4_dst_port, argv[index + 1]); \
        udf.is_l4_dst_port = 1;                                              \
    }                                                                        \

CTC_CLI(ctc_cli_parser_debug_on,
        ctc_cli_parser_debug_on_cmd,
        "debug parser (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PARSER_STR,
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
        typeenum = PARSER_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PARSER_SYS;
    }

    ctc_debug_set_flag("parser", "parser", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_debug_off,
        ctc_cli_parser_debug_off_cmd,
        "no debug parser (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PARSER_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PARSER_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PARSER_SYS;
    }

    ctc_debug_set_flag("parser", "parser", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_debug,
        ctc_cli_parser_show_debug_cmd,
        "show debug parser (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PARSER_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PARSER_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PARSER_SYS;
    }

    en = ctc_debug_get_flag("parser", "parser", typeenum, &level);
    ctc_cli_out("parser:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_set_linkagg_hash,
        ctc_cli_parser_set_linkagg_hash_cmd,
        "parser linkagg-hash {l2 {vlan|cos|ethertype|double-vlan|macsa|macda|port|}|\
        ip {l3protocol|ipsa|ipda|dscp|ipv6-flow-label|ecn|}|\
        l4 {l4-type|l4-user-type|src-port|dst-port|tcp-flag|tcp-ecn|gre-key|nvgre-vsid|nvgre-flow-id|vxlan-vni|vxlan-src-port|}|\
        pbb {b-vlan|b-cos|b-cfi|b-macsa|b-macda|isid|}|\
        mpls {m-l3protocol|m-ipsa|m-ipda|}|\
        fcoe {sid|did|}|\
        trill {ingress-nickname|egress-nickname|}|\
        common deviceinfo| udf udf-id UDF_ID bitmap VALUE} (inner|)",
        CTC_CLI_PARSER_STR,
        "Linkagg hash field",
        "Layer2 ethernet",
        "Add vlan into linkagg hash",
        "Add cos into linkagg hash",
        "Add ethertype into linkagg hash",
        "Add double vlan into linkagg hash",
        "Add macsa into linkagg hash",
        "Add macda into linkagg hash",
        "Add global port into linkagg hash",
        "Layer3 ip",
        "Add l3protocol into linkagg hash",
        "Add ipsa into linkagg hash",
        "Add ipda into linkagg hash",
        "Add dscp into linkagg hash",
        "Add ipv6 flow label into linkagg hash",
        "Add ecn into linkagg hash",
        "Layer4",
        "Add layer4 type into linkagg hash",
        "Add layer4 user type into linkagg hash",
        "Add layer4 src port into linkagghash",
        "Add dst port into linkagg hash",
        "Add tcp flag into linkagg flag",
        "Add tcp ecn into linkagg flag",
        "Add gre key into linkagg hash",
        "Add nvgre vsid into linkagg hash",
        "Add nvgre flow id into linkagg hash",
        "Add vxlan vni into linkagg hash",
        "Add vxlan src port into linkagg hash",
        "Provider backbone",
        "Add bvlan into linkagg hash",
        "Add bcos into linkagg hash",
        "Add bcfi into linkagg hash",
        "Add bmacsa into linkagg hash",
        "Add bmacda into linkagg hash",
        "Add isid into linkagg hash",
        "Mpls",
        "Add mpls l3protocol into linkagg hash",
        "Add mpls ipsa into linkagg hash",
        "Add mpls ipda into linkagg hash",
        "Fcoe",
        "Add sid into linkagg hash",
        "Add did into linkagg hash",
        "Trill",
        "Add ingress nickname into linkagg hash",
        "Add egress nickname into linkagg hash",
        "Common",
        "Add deviceinfo into linkagg hash",
        "Udf",
        "UDF ID",
        "Value",
        "Value",
        "Value",
        "Inner")
{
    int32 ret = 0;
    uint32 pre_index = 0;
    ctc_linkagg_psc_t linkagg_hash_ctl;

    sal_memset(&linkagg_hash_ctl, 0, sizeof(ctc_linkagg_psc_t));

    pre_index = CTC_CLI_GET_ARGC_INDEX("l2");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_VLAN;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_COS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ethertype");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_ETHERTYPE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("double-vlan");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_DOUBLE_VLAN;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_MACSA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_MACDA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l2_flag |= CTC_LINKAGG_PSC_L2_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_INNER;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l3protocol");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IP_PROTOCOL;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IP_IPSA;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IP_IPDA;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IP_DSCP;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipv6-flow-label");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IPV6_FLOW_LABEL;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_IP_ECN;
        linkagg_hash_ctl.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_SRC_PORT;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dst-port");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_DST_PORT;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_GRE_KEY;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_TYPE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_USER_TYPE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-flag");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_TCP_FLAG;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_TCP_ECN;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-vsid");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_NVGRE_VSID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-flow-id");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-vni");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_L4_VXLAN_VNI;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-src-port");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
        linkagg_hash_ctl.l4_flag |= CTC_LINKAGG_PSC_VXLAN_L4_SRC_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("pbb");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_PBB;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-vlan");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_BVLAN;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-cos");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_BCOS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-cfi");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_BCFI;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-macsa");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_BMACSA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-macda");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_BMACDA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("isid");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.pbb_flag |= CTC_LINKAGG_PSC_PBB_ISID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-l3protocol");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_MPLS_PROTOCOL;
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_USE_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipsa");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_MPLS_IPSA;
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_USE_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipda");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_MPLS_IPDA;
        linkagg_hash_ctl.mpls_flag |= CTC_LINKAGG_PSC_USE_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("sid");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.fcoe_flag |= CTC_LINKAGG_PSC_FCOE_SID;
        linkagg_hash_ctl.fcoe_flag |= CTC_LINKAGG_PSC_USE_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("did");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.fcoe_flag |= CTC_LINKAGG_PSC_FCOE_DID;
        linkagg_hash_ctl.fcoe_flag |= CTC_LINKAGG_PSC_USE_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.trill_flag |= CTC_LINKAGG_PSC_TRILL_INGRESS_NICKNAME;
        linkagg_hash_ctl.trill_flag |= CTC_LINKAGG_PSC_USE_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.trill_flag |= CTC_LINKAGG_PSC_TRILL_EGRESS_NICKNAME;
        linkagg_hash_ctl.trill_flag |= CTC_LINKAGG_PSC_USE_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("common");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_COMMON;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("deviceinfo");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.common_flag |= CTC_LINKAGG_PSC_COMMON_DEVICEINFO;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_UDF;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf-id");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT16("udf-id", linkagg_hash_ctl.udf_id, argv[pre_index + 1]);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("bitmap");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT32("bitmap", linkagg_hash_ctl.udf_bitmap, argv[pre_index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_linkagg_set_psc(g_api_lchip, &linkagg_hash_ctl);
    }
    else
    {
        ret = ctc_linkagg_set_psc(&linkagg_hash_ctl);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_linkagg_hash,
        ctc_cli_parser_show_linkagg_hash_cmd,
        "show parser linkagg-hash {l2 {vlan|cos|ethertype|double-vlan|macsa|macda|port}|\
        ip {use-ip|l3protocol|ipsa|ipda|dscp|ipv6-flow-label|ecn}|\
        l4 {use-l4|l4-type|l4-user-type|src-port|dst-port|tcp-flag|tcp-ecn|gre-key|nvgre-vsid|nvgre-flow-id|vxlan-vni|vxlan-src-port}|\
        pbb {b-vlan|b-cos|b-cfi|b-macsa|b-macda|isid}|\
        mpls {use-mpls|m-l3protocol|m-ipsa|m-ipda}|\
        fcoe {use-fcoe|sid|did}|\
        trill {use-trill|ingress-nickname|egress-nickname}|\
        common deviceinfo| udf udf-id UDF_ID} (inner|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        "Linkagg hash field",
        "Layer2 ethernet",
        "Add vlan into linkagg hash",
        "Add cos into linkagg hash",
        "Add ethertype into linkagg hash",
        "Add double vlan into linkagg hash",
        "Add macsa into linkagg hash",
        "Add macda into linkagg hash",
        "Add global port into linkagg hash",
        "Layer3 ip",
        "Use ip hash",
        "Add l3protocol into linkagg hash",
        "Add ipsa into linkagg hash",
        "Add ipda into linkagg hash",
        "Add dscp into linkagg hash",
        "Add ipv6 flow label into linkagg hash",
        "Add ecn into linkagg hash",
        "Layer4",
        "Use layer4 hash",
        "Add layer4 type into linkagg hash",
        "Add layer4 user type into ecmp hash",
        "Add src port into linkagg hash",
        "Add dst port into linkagg hash",
        "Add tcp flag into linkagg hash",
        "Add tcp ecn into linkagg hash",
        "Add gre key into linkagg hash",
        "Add nvgre vsid into linkagg hash",
        "Add nvgre flow id into linkagg hash",
        "Add vxlan vni into linkagg hash",
        "Add vxlan src port into linkagg hash",
        "Provider backbone",
        "Add bvlan into linkagg hash",
        "Add bcos into linkagg hash",
        "Add bcfi into linkagg hash",
        "Add bmacsa into linkagg hash",
        "Add bmacda into linkagg hash",
        "Add isid into linkagg hash",
        "Mpls",
        "Use mpls hash",
        "Add mpls l3protocol into linkagg hash",
        "Add mpls ipsa into linkagg hash",
        "Add mpls ipda into linkagg hash",
        "Fcoe",
        "Use fcoe hash",
        "Add sid into linkagg hash",
        "Add did into linkagg hash",
        "Trill",
        "Use trill hash",
        "Add ingress nickname into linkagg hash",
        "Add egress nickname into linkagg hash",
        "Common",
        "Add deviceinfo into linkagg hash",
        "Udf",
        "UDF ID",
        "Value",
        "Inner")
{
    int32 ret = 0;
    uint32 pre_index = 0;
    ctc_linkagg_psc_t linkagg_hash_ctl;

    sal_memset(&linkagg_hash_ctl, 0, sizeof(ctc_linkagg_psc_t));

    pre_index = CTC_CLI_GET_ARGC_INDEX("l2");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("pbb");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_PBB;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("common");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_COMMON;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_UDF;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf-id");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT16("udf-id", linkagg_hash_ctl.udf_id, argv[pre_index + 1]);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner");
    if (0xFF != pre_index)
    {
        linkagg_hash_ctl.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_INNER;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_linkagg_get_psc(g_api_lchip, &linkagg_hash_ctl);
    }
    else
    {
        ret = ctc_linkagg_get_psc(&linkagg_hash_ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_VLAN)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_COS)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ethertype");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_ETHERTYPE)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("double-vlan");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_DOUBLE_VLAN)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_MACSA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_MACDA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l2_flag) & CTC_LINKAGG_PSC_L2_PORT)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("use-ip");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_USE_IP)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l3protocol");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IP_PROTOCOL)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IP_IPSA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IP_IPDA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IP_DSCP)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipv6-flow-label");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IPV6_FLOW_LABEL)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.ip_flag) & CTC_LINKAGG_PSC_IP_ECN)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("use-l4");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_USE_L4)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_SRC_PORT)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dst-port");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_DST_PORT)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_GRE_KEY)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_TYPE)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_USER_TYPE)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-flag");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_TCP_FLAG)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_TCP_ECN)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-vsid");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_NVGRE_VSID)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-flow-id");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-vni");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_L4_VXLAN_VNI)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-src-port");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.l4_flag) & CTC_LINKAGG_PSC_VXLAN_L4_SRC_PORT)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-vlan");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_BVLAN)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-cos");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_BCOS)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-cfi");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_BCFI)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-macsa");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_BMACSA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("b-macda");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_BMACDA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("isid");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.pbb_flag) & CTC_LINKAGG_PSC_PBB_ISID)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("use-mpls");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.mpls_flag) & CTC_LINKAGG_PSC_USE_MPLS)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-l3protocol");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.mpls_flag) & CTC_LINKAGG_PSC_MPLS_PROTOCOL)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipsa");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.mpls_flag) & CTC_LINKAGG_PSC_MPLS_IPSA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipda");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.mpls_flag) & CTC_LINKAGG_PSC_MPLS_IPDA)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("use-fcoe");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.fcoe_flag) & CTC_LINKAGG_PSC_USE_FCOE)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("sid");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.fcoe_flag) & CTC_LINKAGG_PSC_FCOE_SID)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("did");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.fcoe_flag) & CTC_LINKAGG_PSC_FCOE_DID)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("use-trill");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.trill_flag) & CTC_LINKAGG_PSC_USE_TRILL)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.trill_flag) & CTC_LINKAGG_PSC_TRILL_INGRESS_NICKNAME)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.trill_flag) & CTC_LINKAGG_PSC_TRILL_EGRESS_NICKNAME)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("deviceinfo");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.common_flag) & CTC_LINKAGG_PSC_COMMON_DEVICEINFO)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        if ((linkagg_hash_ctl.psc_type_bitmap) & CTC_LINKAGG_PSC_TYPE_UDF)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
            ctc_cli_out("UDF-ID     VALUE\n");
            ctc_cli_out("%-10d : %d\n", linkagg_hash_ctl.udf_id, linkagg_hash_ctl.udf_bitmap);
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_set_ecmp_hash,
        ctc_cli_parser_set_ecmp_hash_cmd,
        "parser ecmp-hash {ip {l3protocol|dscp|flow-label|ipsa|ipda|}|\
        l4 {l4-type|l4-user-type|src-port|dst-port|tcp-flag|tcp-ecn|gre-key|nvgre-vsid|nvgre-flow-id|vxlan-vni|vxlan-src-port|vxlan-dst-port|}|\
        mpls {m-l3protocol|m-dscp|m-flow-label|m-ipsa|m-ipda|}|\
        fcoe {sid|did|}|\
        trill {inner-vlan|ingress-nickname|egress-nickname|}|\
        l2 {vlan|cos|macsa|macda|port|}|common deviceinfo| udf udf-id UDF_ID bitmap VALUE} {inner|efd|dlb|dlb-efd|}",
        CTC_CLI_PARSER_STR,
        "Ecmp hash field",
        "Layer3 ip",
        "Add l3protocol into ecmp hash",
        "Add dscp into ecmp hash",
        "Add ipv6 flow label into ecmp hash",
        "Add ipsa into ecmp hash",
        "Add ipda into ecmp hash",
        "Layer4",
        "Add type into ecmp hash",
        "Add user type into ecmp hash",
        "Add src port into ecmp hash",
        "Add dst port into ecmp hash",
        "Add tcp flag into ecmp flag",
        "Add tcp ecn into ecmp flag",
        "Add gre key into ecmp hash",
        "Add nvgre vsid into ecmp hash",
        "Add nvgre flow id into ecmp hash",
        "Add vxlan vni into ecmp hash",
        "Add vxlan src port into ecmp hash",
        "Add vxlan dst port into ecmp hash",
        "Mpls",
        "Add mpls l3protocol into ecmp hash",
        "Add mpls dscp into ecmp hash",
        "Add mpls ipv6 flow label into ecmp hash",
        "Add mpls ipsa into ecmp hash",
        "Add mpls ipda into ecmp hash",
        "Fcoe",
        "Add sid into ecmp hash",
        "Add did into ecmp hash",
        "Trill",
        "Add inner vlan id into ecmp hash",
        "Add ingress nickname into ecmp hash",
        "Add egress nickname into ecmp hash",
        "Layer2 ethernet",
        "Add vlan into ecmp hash",
        "Add cos into ecmp hash",
        "Add macsa into ecmp hash",
        "Add macda into ecmp hash",
        "Add global port into ecmp hash",
        "Common",
        "Add deviceinfo into linkagg hash",
        "Udf",
        "UDF ID",
        "Value",
        "Value",
        "Value",
        "Inner mode",
        "Efd mode",
        "Dlb mode",
        "Dlb efd mode")
{
    int32 ret = 0;
    uint32 pre_index = 0;
    ctc_parser_ecmp_hash_ctl_t ecmp_hash_ctl;

    sal_memset(&ecmp_hash_ctl, 0, sizeof(ctc_parser_ecmp_hash_ctl_t));

    pre_index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l3protocol");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("flow-label");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dst-port");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_GRE_KEY;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TYPE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_USER_TYPE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-flag");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_FLAG;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_ECN;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-vsid");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_VSID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-flow-id");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_FLOW_ID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-vni");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_VXLAN_VNI;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-src-port");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-dst-port");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-l3protocol");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.mpls_flag |= CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-dscp");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.mpls_flag |= CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-flow-label");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.mpls_flag |= CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipsa");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.mpls_flag |= CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipda");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.mpls_flag |= CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("sid");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.fcoe_flag |= CTC_PARSER_FCOE_ECMP_HASH_FLAGS_SID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("did");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.fcoe_flag |= CTC_PARSER_FCOE_ECMP_HASH_FLAGS_DID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner-vlan");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.trill_flag |= CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.trill_flag |= CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.trill_flag |= CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME;
    }

    /* set l2 ecmp flag */
    pre_index = CTC_CLI_GET_ARGC_INDEX("l2");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_ECMP_HASH_FLAGS_COS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_CTAG_VID;
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_STAG_VID;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_PORT;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dlb-efd");
    if (0xFF != pre_index)
    {
         ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_DLB_EFD;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dlb");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_DLB;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("common");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_COMMON;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("deviceinfo");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.common_flag|= CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_UDF;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf-id");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT16("udf-id", ecmp_hash_ctl.udf_id, argv[pre_index + 1]);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("bitmap");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT32("bitmap", ecmp_hash_ctl.udf_bitmap, argv[pre_index + 1]);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("efd");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_EFD;
    }

    if(g_ctcs_api_en)
    {
        ret = (0xFF != pre_index) ? ctcs_parser_set_efd_hash_field(g_api_lchip, &ecmp_hash_ctl) :
              ctcs_parser_set_ecmp_hash_field(g_api_lchip, &ecmp_hash_ctl);
    }
    else
    {
        ret = (0xFF != pre_index) ? ctc_parser_set_efd_hash_field(&ecmp_hash_ctl) :
              ctc_parser_set_ecmp_hash_field(&ecmp_hash_ctl);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_ecmp_hash,
        ctc_cli_parser_show_ecmp_hash_cmd,
        "show parser ecmp-hash {ip {l3protocol|dscp|flow-label|ipsa|ipda|} \
        | l4 {l4-type|l4-user-type|src-port|dst-port|tcp-flag|tcp-ecn|gre-key|nvgre-vsid|nvgre-flow-id|vxlan-vni|vxlan-src-port|vxlan-dst-port|} \
        | mpls {m-l3protocol|m-dscp|m-flow-label|m-ipsa|m-ipda|}  \
        | fcoe {sid|did|}  \
        | trill {inner-vlan|ingress-nickname|egress-nickname|}  \
        | l2 {vlan|cos|macsa|macda|port|}|common deviceinfo| udf udf-id UDF_ID}  {inner|efd|dlb|dlb-efd|}",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        "Ecmp hash field",
        "Layer3 ip",
        "Add l3protocol into ecmp hash",
        "Add dscp into ecmp hash",
        "Add ipv6 flow label into ecmp hash",
        "Add ipsa into ecmp hash",
        "Add ipda into ecmp hash",
        "Layer4",
        "Add type into ecmp hash",
        "Add user type into ecmp hash",
        "Add src port into ecmp hash",
        "Add dst port into ecmp hash",
        "Add tcp flag into ecmp flag",
        "Add tcp ecn into ecmp flag",
        "Add gre key into ecmp hash",
        "Add nvgre vsid into ecmp hash",
        "Add nvgre flow id into ecmp hash",
        "Add vxlan vni into ecmp hash",
        "Add vxlan src port into ecmp hash",
        "Add vxlan dst port into ecmp hash",
        "Mpls",
        "Add mpls l3protocol into ecmp hash",
        "Add mpls dscp into ecmp hash",
        "Add mpls ipv6 flow label into ecmp hash",
        "Add mpls ipsa into ecmp hash",
        "Add mpls ipda into ecmp hash",
        "Fcoe",
        "Add sid into ecmp hash",
        "Add did into ecmp hash",
        "Trill",
        "Add inner vlan id into ecmp hash",
        "Add ingress nickname into ecmp hash",
        "Add egress nickname into ecmp hash",
        "Layer2 ethernet",
        "Add vlan into ecmp hash",
        "Add cos into ecmp hash",
        "Add macsa into ecmp hash",
        "Add macda into ecmp hash",
        "Add global port into ecmp hash",
        "Common",
        "Add deviceinfo into linkagg hash",
        "Udf",
        "UDF ID",
        "Value",
        "Inner",
        "Efd",
        "Dlb",
        "Dlb-efd")
{
    int32 ret = 0;
    uint32 pre_index = 0, flag = 0, hash_type_bitmap = 0, brief = 0;
    ctc_parser_ecmp_hash_ctl_t ecmp_hash_ctl;

    sal_memset(&ecmp_hash_ctl, 0, sizeof(ctc_parser_ecmp_hash_ctl_t));

    pre_index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_FCOE;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l2");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dlb-efd");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_DLB_EFD;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("common");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_COMMON;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_UDF;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf-id");
    if (0xFF != pre_index)
    {
        CTC_CLI_GET_UINT16("udf-id", ecmp_hash_ctl.udf_id, argv[pre_index + 1]);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dlb");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_DLB;
    }

    hash_type_bitmap = ecmp_hash_ctl.hash_type_bitmap;

    if (0 == (hash_type_bitmap & (~(CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER | CTC_PARSER_ECMP_HASH_TYPE_FLAGS_DLB_EFD))))
    {
        ecmp_hash_ctl.hash_type_bitmap |= ~(CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER | CTC_PARSER_ECMP_HASH_TYPE_FLAGS_DLB_EFD);
        brief = 1;
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("efd");
    if (0xFF != pre_index)
    {
        ecmp_hash_ctl.hash_type_bitmap |= CTC_PARSER_HASH_TYPE_FLAGS_EFD;
    }

    if(g_ctcs_api_en)
    {
        ret = (0xFF != pre_index)? ctcs_parser_get_efd_hash_field(g_api_lchip, &ecmp_hash_ctl):
               ctcs_parser_get_ecmp_hash_field(g_api_lchip, &ecmp_hash_ctl);
    }
    else
    {
        ret = (0xFF != pre_index)? ctc_parser_get_efd_hash_field(&ecmp_hash_ctl) :
               ctc_parser_get_ecmp_hash_field(&ecmp_hash_ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-16s : %s\n", "Field", "Status");
    ctc_cli_out("-------------------------\n");

    pre_index = CTC_CLI_GET_ARGC_INDEX("l3protocol");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("flow-label");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL)|| (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL))
        {
            ctc_cli_out("%-16s : %s\n", "l3protocol", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "l3protocol", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP))
        {
            ctc_cli_out("%-16s : %s\n", "dscp", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "dscp", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL))
        {
            ctc_cli_out("%-16s : %s\n", "flow-label", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "flow-label", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA))
        {
            ctc_cli_out("%-16s : %s\n", "ipsa", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "ipsa", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag,  CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA))
        {
            ctc_cli_out("%-16s : %s\n", "ipda", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "ipda", "disable");
            }
        }
    }

    flag = 0;

    pre_index = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("dst-port");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_GRE_KEY);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TYPE);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_USER_TYPE);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-flag");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_FLAG);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_ECN);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-vsid");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_VSID);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("nvgre-flow-id");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_FLOW_ID);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-vni");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_VXLAN_VNI);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-src-port");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("vxlan-dst-port");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
       && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT))
        {
            ctc_cli_out("%-16s : %s\n", "src-port", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "src-port", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT))
        {
            ctc_cli_out("%-16s : %s\n", "dst-port", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "dst-port", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
       && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_GRE_KEY) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_GRE_KEY))
        {
            ctc_cli_out("%-16s : %s\n", "gre-key", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "gre-key", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TYPE) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TYPE))
        {
            ctc_cli_out("%-16s : %s\n", "l4-type", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "l4-type", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_USER_TYPE) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_USER_TYPE))
        {
            ctc_cli_out("%-16s : %s\n", "l4-user-type", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "l4-user-type", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_FLAG) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_FLAG))
        {
            ctc_cli_out("%-16s : %s\n", "tcl-flag", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "tcl-flag", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
       && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_ECN) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_TCP_ECN))
        {
            ctc_cli_out("%-16s : %s\n", "tcp-ecn", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "tcp-ecn", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_VSID) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_VSID))
        {
            ctc_cli_out("%-16s : %s\n", "nvgre-vsid", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "nvgre-vsid", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_FLOW_ID) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_NVGRE_FLOW_ID))
        {
            ctc_cli_out("%-16s : %s\n", "nvgre-flow-id", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "nvgre-flow-id", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_VXLAN_VNI) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_L4_VXLAN_VNI))
        {
            ctc_cli_out("%-16s : %s\n", "vxlan-vni", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "vxlan-vni", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT))
        {
            ctc_cli_out("%-16s : %s\n", "vxlan-src-port", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "vxlan-src-port", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_DST_PORT))
        {
            ctc_cli_out("%-16s : %s\n", "vxlan-dst-port", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "vxlan-dst-port", "disable");
            }
        }
    }

    flag = 0;

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-l3protocol");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-dscp");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-flow-label");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipsa");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("m-ipda");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.mpls_flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_PROTOCOL))
        {
            ctc_cli_out("%-16s : %s\n", "m-l3protocol", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "m-l3protocol", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.mpls_flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_DSCP))
        {
            ctc_cli_out("%-16s : %s\n", "m-dscp", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "m-dscp", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.mpls_flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPV6_FLOW_LABEL))
        {
            ctc_cli_out("%-16s : %s\n", "m-flow-label", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "m-flow-label", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.mpls_flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPSA))
        {
            ctc_cli_out("%-16s : %s\n", "m-ipsa", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "m-ipsa", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.mpls_flag, CTC_PARSER_MPLS_ECMP_HASH_FLAGS_IPDA))
        {
            ctc_cli_out("%-16s : %s\n", "m-ipda", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "m-ipda", "disable");
            }
        }
    }

    flag = 0;

    pre_index = CTC_CLI_GET_ARGC_INDEX("sid");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_FCOE_ECMP_HASH_FLAGS_SID);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("did");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_FCOE_ECMP_HASH_FLAGS_DID);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_FCOE)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_FCOE_ECMP_HASH_FLAGS_SID) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.fcoe_flag, CTC_PARSER_FCOE_ECMP_HASH_FLAGS_SID))
        {
            ctc_cli_out("%-16s : %s\n", "sid", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "sid", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_FCOE)
       && (CTC_FLAG_ISSET(flag, CTC_PARSER_FCOE_ECMP_HASH_FLAGS_DID) || (!flag)))
    {
        if ((ecmp_hash_ctl.fcoe_flag) & CTC_PARSER_FCOE_ECMP_HASH_FLAGS_DID)
        {
            ctc_cli_out("%-16s : %s\n", "did", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "did", "disable");
            }
        }
    }

    flag = 0;

    pre_index = CTC_CLI_GET_ARGC_INDEX("inner-vlan");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.trill_flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INNER_VLAN_ID))
        {
            ctc_cli_out("%-16s : %s\n", "inner-vlan", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "inner-vlan", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.trill_flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_INGRESS_NICKNAME))
        {
            ctc_cli_out("%-16s : %s\n", "ingress-nickname", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "ingress-nickname", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_TRILL)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.trill_flag, CTC_PARSER_TRILL_ECMP_HASH_FLAGS_EGRESS_NICKNAME))
        {
            ctc_cli_out("%-16s : %s\n", "egress-nickname", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "egress-nickname", "disable");
            }
        }
    }

    flag = 0;

    /* show l2 ecmp flag */
    pre_index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID);
        CTC_SET_FLAG(flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_COS);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA);
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != pre_index)
    {
        CTC_SET_FLAG(flag, CTC_PARSER_L2_HASH_FLAGS_PORT);
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2)
        && ((CTC_FLAG_ISSET(flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID)
        && CTC_FLAG_ISSET(flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID)) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID))
        {
            ctc_cli_out("%-16s : %s\n", "cvlan", "enable");
        }
        else
        {
            ctc_cli_out("%-16s : %s\n", "cvlan", "disable");
        }
        if(CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID))
        {
            ctc_cli_out("%-16s : %s\n", "svlan", "enable");
        }
        else
        {
            ctc_cli_out("%-16s : %s\n", "svlan", "enable");
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_COS) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_COS))
        {
            ctc_cli_out("%-16s : %s\n", "cos", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "cos", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA))
        {
            ctc_cli_out("%-16s : %s\n", "macsa", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "macsa", "disable");
            }
        }
    }

    if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA))
        {
            ctc_cli_out("%-16s : %s\n", "macda", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "macda", "disable");
            }
        }
    }

     if (CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2)
        && (CTC_FLAG_ISSET(flag, CTC_PARSER_L2_HASH_FLAGS_PORT) || (!flag)))
    {
        if (CTC_FLAG_ISSET(ecmp_hash_ctl.l2_flag, CTC_PARSER_L2_HASH_FLAGS_PORT))
        {
            ctc_cli_out("%-16s : %s\n", "port", "enable");
        }
        else
        {
            if (!brief)
            {
                ctc_cli_out("%-16s : %s\n", "port", "disable");
            }
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("deviceinfo");
    if (0xFF != pre_index)
    {
        if ((ecmp_hash_ctl.common_flag) & CTC_PARSER_COMMON_HASH_FLAGS_DEVICEINFO)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    pre_index = CTC_CLI_GET_ARGC_INDEX("udf");
    if (0xFF != pre_index)
    {
        if ((ecmp_hash_ctl.hash_type_bitmap) & CTC_PARSER_HASH_TYPE_FLAGS_UDF)
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "enable");
            ctc_cli_out("UDF-ID     VALUE\n");
            ctc_cli_out("%-10d : %d\n", ecmp_hash_ctl.udf_id, ecmp_hash_ctl.udf_bitmap);

        }
        else
        {
            ctc_cli_out("%s : %s\n", argv[pre_index], "disable");
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_set_tpid,
        ctc_cli_parser_l2_set_tpid_cmd,
        "parser l2 ethernet tpid (cvlan|itag|bvlan|svlan0|svlan1|svlan2|svlan3|cntag|evb) TPID",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Type ID",
        "Customer vlan",
        "Itag tpid",
        "Backbone vlan",
        "Service vlan0",
        "Service vlan1",
        "Service vlan2",
        "Service vlan3",
        "Cntag tpid",
        "evb tpid",
        "Tpid value<0x0 - 0xFFFF>")
{
    uint16 value = 0;
    ctc_parser_l2_tpid_t tp = 0;
    int32 ret = 0;

    if (0 == sal_memcmp(argv[0], "cvlan", 5))
    {
        tp = CTC_PARSER_L2_TPID_CVLAN_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "itag", 4))
    {
        tp = CTC_PARSER_L2_TPID_ITAG_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "bvlan", 5))
    {
        tp = CTC_PARSER_L2_TPID_BLVAN_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "svlan0", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_0;
    }
    else if (0 == sal_memcmp(argv[0], "svlan1", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_1;
    }
    else if (0 == sal_memcmp(argv[0], "svlan2", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_2;
    }
    else if (0 == sal_memcmp(argv[0], "svlan3", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_3;
    }
    else if (0 == sal_memcmp(argv[0], "cntag", 5))
    {
        tp = CTC_PARSER_L2_TPID_CNTAG_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "evb", 3))
    {
        tp = CTC_PARSER_L2_TPID_EVB_TPID;
    }

    CTC_CLI_GET_UINT16_RANGE("tpid_value", value, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_tpid(g_api_lchip, tp, value);
    }
    else
    {
        ret = ctc_parser_set_tpid(tp, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l2_tpid,
        ctc_cli_parser_show_l2_tpid_cmd,
        "show parser l2 ethernet tpid ((cvlan|itag|bvlan|svlan0|svlan1|svlan2|svlan3|cntag|evb) |)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Type ID",
        "Customer vlan",
        "Itag tpid",
        "Backbone vlan",
        "Service vlan0",
        "Service vlan1",
        "Service vlan2",
        "Service vlan3",
        "Cntag tpid",
        "Evb tpid")
{

    uint16 value = 0, i = 0;
    ctc_parser_l2_tpid_t tp = 0;
    int32 ret = 0;
    char* desc[] = {"cvlan", "itag", "bvlan", "svlan0", "svlan1", "svlan2", "svlan3", "cntag", "evb"};

    ctc_cli_out("%-9s%s\n", "Type", "Value");
    ctc_cli_out("---------------\n");

    if (0 == argc)
    {
        for (i = 0; i <= CTC_PARSER_L2_TPID_EVB_TPID; i++)
        {
            if(g_ctcs_api_en)
            {
                 ret = ctcs_parser_get_tpid(g_api_lchip, i, &value);
            }
            else
            {
                ret = ctc_parser_get_tpid(i, &value);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("%-9s0x%x\n", desc[i], value);
        }
        return CLI_SUCCESS;
    }

    if (0 == sal_memcmp(argv[0], "cvlan", 5))
    {
        tp = CTC_PARSER_L2_TPID_CVLAN_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "itag", 4))
    {
        tp = CTC_PARSER_L2_TPID_ITAG_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "bvlan", 5))
    {
        tp = CTC_PARSER_L2_TPID_BLVAN_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "svlan0", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_0;
    }
    else if (0 == sal_memcmp(argv[0], "svlan1", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_1;
    }
    else if (0 == sal_memcmp(argv[0], "svlan2", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_2;
    }
    else if (0 == sal_memcmp(argv[0], "svlan3", 6))
    {
        tp = CTC_PARSER_L2_TPID_SVLAN_TPID_3;
    }
    else if (0 == sal_memcmp(argv[0], "cntag", 5))
    {
        tp = CTC_PARSER_L2_TPID_CNTAG_TPID;
    }
    else if (0 == sal_memcmp(argv[0], "evb", 3))
    {
        tp = CTC_PARSER_L2_TPID_EVB_TPID;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_tpid(g_api_lchip, tp, &value);
    }
    else
    {
        ret = ctc_parser_get_tpid(tp, &value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-9s0x%x\n", argv[0], value);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_set_vlan_pas_num,
        ctc_cli_parser_l2_set_vlan_pas_num_cmd,
        "parser l2 ethernet vlan-pas-num VLAN_NUM",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Vlan parser num",
        "Vlan num <0-2>")
{
    int32 ret = 0;
    uint8 value = 0;

    CTC_CLI_GET_UINT8_RANGE("vlan_num", value, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_vlan_parser_num(g_api_lchip, value);
    }
    else
    {
        ret = ctc_parser_set_vlan_parser_num(value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_parser_show_l2_vlan_pas_num,
        ctc_cli_parser_show_l2_vlan_pas_num_cmd,
        "show parser l2 ethernet vlan-pas-num",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Vlan parser num")
{
    int32 ret = 0;
    uint8 value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_vlan_parser_num(g_api_lchip, &value);
    }
    else
    {
        ret = ctc_parser_get_vlan_parser_num(&value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("vlan-pas-num: 0x%x\n", value);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_set_max_len,
        ctc_cli_parser_l2_set_max_len_cmd,
        "parser l2 ethernet max-length LEN",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Max length",
        "Max length value <0x0 - 0xFFFF>")
{
    int32 ret = 0;
    uint16 value = 0;

    CTC_CLI_GET_UINT16_RANGE("max length", value, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_max_length_field(g_api_lchip, value);
    }
    else
    {
        ret = ctc_parser_set_max_length_field(value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_parser_show_l2_max_len,
        ctc_cli_parser_show_l2_max_len_cmd,
        "show parser l2 ethernet max-length",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Ethernet l2",
        "Max length")
{
    int32 ret = 0;
    uint16 value = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_max_length_filed(g_api_lchip, &value);
    }
    else
    {
        ret = ctc_parser_get_max_length_filed(&value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("max_length: 0x%x\n", value);

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_parser_l2_set_pbb_header,
        ctc_cli_parser_l2_set_pbb_header_cmd,
        "parser l2 pbb header nca-check-en (enable|disable) outer-vlan-is-cvlan (enable|disable) vlan-pas-num VAL",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Provider backbone",
        "Header",
        "Nca enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Outer vlan is cvlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Vlan parsing num",
        "Vlan parsing num value<0-3>")
{
    ctc_parser_pbb_header_t ctl;
    int32 ret = 0;

    sal_memset(&ctl, 0, sizeof(ctc_parser_pbb_header_t));

    if (0 == sal_memcmp(argv[0], "enable", sal_strlen("enable")))
    {
        ctl.nca_value_en = 1;
    }

    if (0 == sal_memcmp(argv[1], "enable", sal_strlen("enable")))
    {
        ctl.outer_vlan_is_cvlan = 1;
    }

    CTC_CLI_GET_UINT8_RANGE("vlan parsing num", ctl.vlan_parsing_num, argv[2], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_pbb_header(g_api_lchip, &ctl);
    }
    else
    {
        ret = ctc_parser_set_pbb_header(&ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l2_pbb_header,
        ctc_cli_parser_show_l2_pbb_header_cmd,
        "show parser l2 pbb header",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Provider backbone",
        "Header")
{
    ctc_parser_pbb_header_t ctl;
    int32 ret = 0;

    sal_memset(&ctl, 0, sizeof(ctc_parser_pbb_header_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_pbb_header(g_api_lchip, &ctl);
    }
    else
    {
        ret = ctc_parser_get_pbb_header(&ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "nca check enable", ctl.nca_value_en);
    ctc_cli_out("%s : 0x%x\n", "outer vlan is cvlan", ctl.outer_vlan_is_cvlan);
    ctc_cli_out("%s : 0x%x\n", "vlan parsing num", ctl.vlan_parsing_num);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_set_l2flex,
        ctc_cli_parser_l2_set_l2flex_cmd,
        "parser l2 flex mac-da-basic-offset VAL protocol-basic-offset VAL min-len LEN l2-basic-offset VAL",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Flexible config",
        "Mac da basic offset",
        "Offset value <0-26>",
        "Header protocol basic offset",
        "Offset value <0-30>",
        "Minium length",
        "Length value <0x0-0xFF>",
        "Layer2 basic offset",
        CTC_CLI_L2FLEX_BASIC_OFFSET)
{
    ctc_parser_l2flex_header_t ctl;
    int32 ret = 0;

    sal_memset(&ctl, 0, sizeof(ctc_parser_l2flex_header_t));

    CTC_CLI_GET_UINT8_RANGE("mac da basic offset", ctl.mac_da_basic_offset, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("header protocol basic offset", ctl.header_protocol_basic_offset, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("min length", ctl.min_length, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("l2 basic offset", ctl.l2_basic_offset, argv[3], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_l2_flex_header(g_api_lchip, &ctl);
    }
    else
    {
        ret = ctc_parser_set_l2_flex_header(&ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l2_l2flex,
        ctc_cli_parser_show_l2_l2flex_cmd,
        "show parser l2 flex",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Flexible config")
{
    ctc_parser_l2flex_header_t ctl;
    int32 ret = 0;

    sal_memset(&ctl, 0, sizeof(ctc_parser_l2flex_header_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_l2_flex_header(g_api_lchip, &ctl);
    }
    else
    {
        ret = ctc_parser_get_l2_flex_header(&ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "mac da basic offset", ctl.mac_da_basic_offset);
    ctc_cli_out("%s : 0x%x\n", "header protocol basic offset", ctl.header_protocol_basic_offset);
    ctc_cli_out("%s : 0x%x\n", "minium length", ctl.min_length);
    ctc_cli_out("%s : 0x%x\n", "layer2 basic offset", ctl.l2_basic_offset);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_mapping_l3type,
        ctc_cli_parser_l2_mapping_l3type_cmd,
        "parser l2 mapping-l3-type mapping index INDEX (l2-type-none | l2-type-eth-v2 | l2-type-eth-sap | l2-type-eth-snap| \
        l2-type-ppp-1b | l2-type-ppp-2b ) (l2-hdr-ptl L2_HDR_PTL mask MASK l3-type L3-TYPE (additional-offset OFFSET|))",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Mapping Layer3 type for Layer3 parsing",
        "Mapping l3 type entry",
        "Entry index",
        CTC_CLI_MAPPING_L3TYPE_INDEX,
        "None l2 type",
        "l2 type ethernet-V2",
        "l2 type ethernet-sap",
        "l2 type ethernet-snap",
        "l2 type ppp 1b",
        "l2 type ppp 2b",
        "Layer2 header protocol",
        "Layer2 header protocol value <0x0-0xFFFF>",
        "Mask",
        "Mask value <0x0-0xFFFFFFFF>,when greater than 0x7FFFFF,as 0x7FFFFF to process ",
        "Layer3 type",
        "Layer3 type value <13-15>",
        "Addition offset to be added",
        "Offset value <0-15>")
{
    ctc_parser_l2_protocol_entry_t entry;
    uint8 index = 0;
    int32 ret = 0;
    uint8 map_index = 0;

    sal_memset(&entry, 0, sizeof(ctc_parser_l2_protocol_entry_t));
    CTC_CLI_GET_UINT8_RANGE("index", map_index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-none");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_NONE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-eth-v2");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_ETH_V2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-eth-sap");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_ETH_SAP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-eth-snap");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_ETH_SNAP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-ppp-1b");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_PPP_1B;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-ppp-2b");
    if (index != 0xFF)
    {
        entry.l2_type = CTC_PARSER_L2_TYPE_PPP_2B;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-hdr-ptl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l2 hdr ptl", entry.l2_header_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mask");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("mask", entry.mask, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("l3 type", entry.l3_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("additional-offset");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("additional offset", entry.addition_offset, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_map_l3_type(g_api_lchip, map_index, &entry);
    }
    else
    {
        ret = ctc_parser_map_l3_type(map_index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l2_unmapping_l3type,
        ctc_cli_parser_l2_unmapping_l3type_cmd,
        "parser l2 mapping-l3-type unmapping index INDEX",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L2_HDR_STR,
        "Mapping Layer3 type for Layer3 parsing",
        "Unmapping l3 type entry",
        "Entry index",
        CTC_CLI_MAPPING_L3TYPE_INDEX)
{
    uint8 index = 0;
    int32 ret = 0;

    CTC_CLI_GET_UINT8_RANGE("unmapped index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_unmap_l3_type(g_api_lchip, index);
    }
    else
    {
        ret = ctc_parser_unmap_l3_type(index);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_set_ip_frag,
        ctc_cli_parser_l3_set_ip_frag_cmd,
        "parser l3 ip small-frag-offset OFFSET",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Layer3 ip",
        "Small fragment offset",
        "<0-3>, multiple of 8 bytes")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;
    uint8 offset = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("offset", offset, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
        global_cfg.small_frag_offset = offset;
        ret = ret ? ret : ctcs_parser_set_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
        global_cfg.small_frag_offset = offset;
        ret = ret ? ret : ctc_parser_set_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l3_ip_frag,
        ctc_cli_parser_show_l3_ip_frag_cmd,
        "show parser l3 ip small-frag-offset",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Layer3 ip",
        "Small fragment offset")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "Small frag offset(multiple of 8 bytes)", global_cfg.small_frag_offset);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_set_l3flex,
        ctc_cli_parser_l3_set_l3flex_cmd,
        "parser l3 flex ipda-basic-offset OFFSET min-len LEN l3-basic-offset OFFSET ptl-byte-select VAL",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Flexible config",
        "Ip da basic offset",
        "Offset value <0x0-0x18>",
        "Minium length",
        "Length value <0x0-0xFF>",
        "Layer3 basic offset",
        "Offset value <0x0-0xFF>",
        "Protocol byte select",
        "Value <0x0-0x1F>")
{
    ctc_parser_l3flex_header_t l3flex_header;
    int32 ret = 0;

    sal_memset(&l3flex_header, 0, sizeof(ctc_parser_l3flex_header_t));

    CTC_CLI_GET_UINT8_RANGE("ip da basic offset", l3flex_header.ipda_basic_offset, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("minium length", l3flex_header.l3min_length, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("layer3 basic offset", l3flex_header.l3_basic_offset, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("protocol byte select ", l3flex_header.protocol_byte_sel, argv[3], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_l3_flex_header(g_api_lchip, &l3flex_header);
    }
    else
    {
        ret = ctc_parser_set_l3_flex_header(&l3flex_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l3_l3flex,
        ctc_cli_parser_show_l3_l3flex_cmd,
        "show parser l3 flex",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Flexible config")
{

    ctc_parser_l3flex_header_t l3flex_header;
    int32 ret = 0;

    sal_memset(&l3flex_header, 0, sizeof(ctc_parser_l3flex_header_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_l3_flex_header(g_api_lchip, &l3flex_header);
    }
    else
    {
        ret = ctc_parser_get_l3_flex_header(&l3flex_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "ip da basic offset", l3flex_header.ipda_basic_offset);
    ctc_cli_out("%s : 0x%x\n", "minium length", l3flex_header.l3min_length);
    ctc_cli_out("%s : 0x%x\n", "layer3 basic offset", l3flex_header.l3_basic_offset);
    ctc_cli_out("%s : 0x%x\n", "protocol byte select", l3flex_header.protocol_byte_sel);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_set_trill_header,
        ctc_cli_parser_l3_set_trill_header_cmd,
        "parser l3 trill header inner-tpid TPID rbridge-ethertype TPYE",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Layer3 trill",
        "Header",
        "Inner-tpid",
        "Inner-tpid value <0x0 - 0xFFFF>",
        "Rbridge channel ether type",
        "Type value <0x0 - 0xFFFF>")
{
    ctc_parser_trill_header_t  trill_header;
    int32 ret = 0;

    sal_memset(&trill_header, 0, sizeof(ctc_parser_trill_header_t));

    CTC_CLI_GET_UINT16_RANGE("inner tpid", trill_header.inner_tpid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("rbridge channel ether type", trill_header.rbridge_channel_ether_type, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_trill_header(g_api_lchip, &trill_header);
    }
    else
    {
        ret = ctc_parser_set_trill_header(&trill_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l3_trill_header,
        ctc_cli_parser_show_l3_trill_header_cmd,
        "show parser l3 trill header inner-tpid rbridge-ethertype",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Layer3 trill",
        "Header",
        "Inner tpid",
        "Rbridge channel ether type")
{
    ctc_parser_trill_header_t  trill_header;
    int32 ret = 0;

    sal_memset(&trill_header, 0, sizeof(ctc_parser_trill_header_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_trill_header(g_api_lchip, &trill_header);
    }
    else
    {
        ret = ctc_parser_get_trill_header(&trill_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "inner tpid", trill_header.inner_tpid);
    ctc_cli_out("%s : 0x%x\n", "rbridge channel ethertype", trill_header.rbridge_channel_ether_type);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_mapping_l4type,
        ctc_cli_parser_l3_mapping_l4type_cmd,
        "parser l3 mapping-l4-type mapping index INDEX hdr-ptl HDR_PTL hdr-mask HDR_MASK l3-type L3-TYPE type-mask TYPE_MASK \
        l4-type L4-TYPE (additional-offset OFFSET|)",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Mapping layer4 type",
        "Mapping l4 type entry",
        "Entry index",
        CTC_CLI_MAPPING_L4TYPE_INDEX,
        "Layer3 header protocol",
        "Protocol value <0x0-0xFF>",
        "Layer3 header protocol mask",
        "Layer3 header protocol mask value <0x0-0xFF>",
        "Layer3 type",
        "Layer3 type value <0-15>",
        "Layer3 type mask",
        "Layer3 type mask value <0x0-0xFF>,when greater than 0xF,as 0xF to process",
        "Layer4 type",
        "Layer4 type value <0-15>",
        "Addition offset to be added",
        "Offset value<0-15>")
{
    ctc_parser_l3_protocol_entry_t entry;
    uint8 index = 0;

    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_parser_l3_protocol_entry_t));
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    CTC_CLI_GET_UINT8_RANGE("hdr protocol", entry.l3_header_protocol, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("hdr protocol mask", entry.l3_header_protocol_mask, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("layer3 type", entry.l3_type, argv[3], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("layer3 type mask", entry.l3_type_mask, argv[4], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("layer4 type", entry.l4_type, argv[5], 0, CTC_MAX_UINT8_VALUE);
    if (argc > 6)
    {
        CTC_CLI_GET_UINT8_RANGE("additional offset", entry.addition_offset, argv[7], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_map_l4_type(g_api_lchip, index, &entry);
    }
    else
    {
        ret = ctc_parser_map_l4_type(index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_unmapping_l4type,
        ctc_cli_parser_l3_unmapping_l4type_cmd,
        "parser l3 mapping-l4-type unmapping index INDEX ",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "Mapping layer4 type",
        "Unmapping l4 type entry",
        "Entry index",
        CTC_CLI_MAPPING_L4TYPE_INDEX)
{
    uint8 index = 0;
    int32 ret = 0;

    CTC_CLI_GET_UINT8_RANGE("unmapping index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_unmap_l4_type(g_api_lchip, index);
    }
    else
    {
        ret = ctc_parser_unmap_l4_type(index);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l4_set_flex,
        ctc_cli_parser_l4_set_flex_cmd,
        "parser l4 flex dest-port-basic-offset OFFSET min-len LEN",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Flexible operation",
        "Offset to l4 flex dest port",
        "OFFSET <0x0 - 0x1E>",
        "Layer4 minum length",
        "Minum length value <0x0 - 0x1F>")
{
    ctc_parser_l4flex_header_t l4flex_header;
    int32 ret = 0;

    sal_memset(&l4flex_header, 0, sizeof(ctc_parser_l4flex_header_t));

    CTC_CLI_GET_UINT8_RANGE("dest port basic offset", l4flex_header.dest_port_basic_offset, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("min length", l4flex_header.l4_min_len, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_l4_flex_header(g_api_lchip, &l4flex_header);
    }
    else
    {
        ret = ctc_parser_set_l4_flex_header(&l4flex_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l4_flex,
        ctc_cli_parser_show_l4_flex_cmd,
        "show parser l4 flex",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Flexible operation")
{
    ctc_parser_l4flex_header_t l4flex_header;
    int32 ret = 0;

    sal_memset(&l4flex_header, 0, sizeof(ctc_parser_l4flex_header_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_l4_flex_header(g_api_lchip, &l4flex_header);
    }
    else
    {
        ret = ctc_parser_get_l4_flex_header(&l4flex_header);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "Offset to l4 flex dest port", l4flex_header.dest_port_basic_offset);
    ctc_cli_out("%s : 0x%x\n", "Layer4 minum length", l4flex_header.l4_min_len);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l4_set_app,
        ctc_cli_parser_l4_set_app_cmd,
        "parser l4 app ptp-en (enable|disable) ntp-en (enable|disable) bfd-en (enable|disable) vxlan-en (enable|disable)",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Udp application operation",
        "Layer4 ptp enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Layer4 ntp enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Layer4 bfd enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Layer4 vxlan enable",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    ctc_parser_l4app_ctl_t  l4app_ctl;
    int32 ret = 0;

    sal_memset(&l4app_ctl, 0, sizeof(ctc_parser_l4app_ctl_t));

    if (0 == sal_memcmp(argv[0], "enable", sal_strlen("enable")))
    {
        l4app_ctl.ptp_en = 1;
    }

    if (0 == sal_memcmp(argv[1], "enable", sal_strlen("enable")))
    {
        l4app_ctl.ntp_en = 1;
    }

    if (0 == sal_memcmp(argv[2], "enable", sal_strlen("enable")))
    {
        l4app_ctl.bfd_en = 1;
    }

    if (0 == sal_memcmp(argv[3], "enable", sal_strlen("enable")))
    {
        l4app_ctl.vxlan_en = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_l4_app_ctl(g_api_lchip, &l4app_ctl);
    }
    else
    {
        ret = ctc_parser_set_l4_app_ctl(&l4app_ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l4_app,
        ctc_cli_parser_show_l4_app_cmd,
        "show parser l4 app",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Udp application operation")
{
    ctc_parser_l4app_ctl_t  l4app_ctl;
    int32 ret = 0;

    sal_memset(&l4app_ctl, 0, sizeof(ctc_parser_l4app_ctl_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_l4_app_ctl(g_api_lchip, &l4app_ctl);
    }
    else
    {
        ret = ctc_parser_get_l4_app_ctl(&l4app_ctl);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "ptp    enable", l4app_ctl.ptp_en);
    ctc_cli_out("%s : 0x%x\n", "ntp    enable", l4app_ctl.ntp_en);
    ctc_cli_out("%s : 0x%x\n", "bfd    enable", l4app_ctl.bfd_en);
    ctc_cli_out("%s : 0x%x\n", "vxlan  enable", l4app_ctl.vxlan_en);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l4_set_app_data,
        ctc_cli_parser_l4_set_app_data_cmd,
        "parser l4 app-data set index INDEX dst-port-mask DST_PORT_MASK dst-port-val DST_PORT_VAL \
        is-tcp-mask IS_TCP_MASK is-tcp-val IS_TCP_VAL is-udp-mask IS_UDP_MASK is-udp-val IS_UDP_VAL",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Application entry",
        "Add layer4 application",
        "Entry index",
        "Index Value <0 - 3>",
        "Dest port mask",
        "Dest port mask value <0x0 -0xFFFF>",
        "Dest port value",
        "Dest port value <0x0 -0xFFFF>",
        "Is tcp mask",
        "Is tcp mask value <0 - 1>",
        "Is tcp value",
        "Is tcp value value <0 - 1>",
        "Is udp mask",
        "Is udp mask value <0 - 1>",
        "Is udp value",
        "Is udp Value value <0 - 1>")
{
    ctc_parser_l4_app_data_entry_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_parser_l4_app_data_entry_t));
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    CTC_CLI_GET_UINT16_RANGE("l4 dest port mask", entry.l4_dst_port_mask, argv[1], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT16_RANGE("l4 dest port value", entry.l4_dst_port_value, argv[2], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is tcp mask", entry.is_tcp_mask, argv[3], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is tcp value", entry.is_tcp_value, argv[4], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is udp mask", entry.is_udp_mask, argv[5], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is udp value", entry.is_udp_value, argv[6], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_l4_app_data_ctl(g_api_lchip, index, &entry);
    }
    else
    {
        ret = ctc_parser_set_l4_app_data_ctl(index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_l4_app_data,
        ctc_cli_parser_show_l4_app_data_cmd,
        "show parser l4 app-data index VAL",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_L4_HDR_STR,
        "Application entry",
        "Entry index",
        "Index Value <0 - 3>")
{
    ctc_parser_l4_app_data_entry_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_parser_l4_app_data_entry_t));

    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_l4_app_data_ctl(g_api_lchip, index, &entry);
    }
    else
    {
        ret = ctc_parser_get_l4_app_data_ctl(index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%s : 0x%x\n", "dest port mask", entry.l4_dst_port_mask);
    ctc_cli_out("%s : 0x%x\n", "dest port value", entry.l4_dst_port_value);
    ctc_cli_out("%s : 0x%x\n", "is tcp mask", entry.is_tcp_mask);
    ctc_cli_out("%s : 0x%x\n", "is tcp value", entry.is_tcp_value);
    ctc_cli_out("%s : 0x%x\n", "is udp mask", entry.is_udp_mask);
    ctc_cli_out("%s : 0x%x\n", "is udp value", entry.is_udp_value);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_udf_add,
        ctc_cli_parser_udf_add_cmd,
        "parser udf add (layer3 ether-type ETHER_TYPE \
        | layer4 ip-version (ipv4|ipv6) (l4-protocol PROTOCOL)  \
        {l4-src-port LAYER4_SRC_PORT | l4-dst-port LAYER4_DST_PORT |}) \
        (index INDEX|) \
        offset OFFSET1 (OFFSET2 (OFFSET3 (OFFSET4|) |) |)",
        CTC_CLI_PARSER_STR,
        CTC_CLI_PARSER_UDF,
        "Add user define format",
        "Layer3 udf",
        "Ether type",
        "<0-0xFFFF>",
        "Layer4 udf",
        "IP version",
        "IPv4",
        "IPv6",
        "Layer4 protocol",
        CTC_CLI_L4_PROTOCOL_VALUE,
        "Layer4 source port",
        "<0-0xFFFF>",
        "Layer4 dest port",
        "<0-0xFFFF>",
        "Index",
        "Specific index",
        "Udf parser offset",
        CTC_CLI_PARSER_UDF_OFFSET,
        CTC_CLI_PARSER_UDF_OFFSET,
        CTC_CLI_PARSER_UDF_OFFSET,
        CTC_CLI_PARSER_UDF_OFFSET)
{
    int32 ret = CLI_SUCCESS;
    uint8 n = 1, i = 0;
    uint32 index = 0;
    ctc_parser_udf_t udf;

    sal_memset(&udf, 0, sizeof(ctc_parser_udf_t));
    CTC_CLI_PARSER_UDF_SET(udf)
    index = CTC_CLI_GET_ARGC_INDEX("index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("index", index, argv[index + 1]);
        n += 2;
    }
    else
    {
        index = 0xFFFFFFFF;
    }

    if (CTC_PARSER_UDF_TYPE_L3_UDF == udf.type)
    {
        n += 2;
    }
    else
    {
        n += 4;
        if (udf.is_l4_src_port)
        {
            n += 2;
        }
        if (udf.is_l4_dst_port)
        {
            n += 2;
        }
    }

    for (i = 0; i < argc - n; i++)
    {
        CTC_CLI_GET_UINT8("offset", udf.udf_offset[i], argv[n + i]);
    }
    udf.udf_num = argc - n;

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_udf(g_api_lchip, index, &udf);
    }
    else
    {
        ret = ctc_parser_set_udf(index, &udf);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_parser_udf_remove,
        ctc_cli_parser_udf_remove_cmd,
        "parser udf remove (layer3 ether-type ETHER_TYPE \
        | layer4 {ip-version (ipv4|ipv6) | (l4-protocol PROTOCOL)  \
        | l4-src-port LAYER4_SRC_PORT | l4-dst-port LAYER4_DST_PORT |}|index INDEX)",
        CTC_CLI_PARSER_STR,
        CTC_CLI_PARSER_UDF,
        "Remove user define format",
        "Layer3 udf",
        "Ether type",
        "<0-0xFFFF>",
        "Layer4 udf",
        "IP version",
        "IPv4",
        "IPv6",
        "Layer4 protocol",
        CTC_CLI_L4_PROTOCOL_VALUE,
        "Layer4 source port",
        "<0-0xFFFF>",
        "Layer4 dest port",
        "<0-0xFFFF>",
        "Index",
        "Specific index")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0;
    ctc_parser_udf_t udf;

    sal_memset(&udf, 0, sizeof(ctc_parser_udf_t));

    CTC_CLI_PARSER_UDF_SET(udf)
    index = CTC_CLI_GET_ARGC_INDEX("index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("index", index, argv[index + 1]);
    }
    else
    {
        index = 0xFFFFFFFF;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_udf(g_api_lchip, index, &udf);
    }
    else
    {
        ret = ctc_parser_set_udf(index, &udf);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_parser_show_udf,
        ctc_cli_parser_show_udf_cmd,
        "show parser udf (layer3 ether-type ETHER_TYPE \
        | layer4 {ip-version (ipv4|ipv6) | (l4-protocol PROTOCOL)  \
        | l4-src-port LAYER4_SRC_PORT | l4-dst-port LAYER4_DST_PORT |})",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        CTC_CLI_PARSER_UDF,
        "Layer3 udf",
        "Ether type",
        "<0-0xFFFF>",
        "Layer4 udf",
        "IP version",
        "IPv4",
        "IPv6",
        "Layer4 protocol",
        CTC_CLI_L4_PROTOCOL_VALUE,
        "Layer4 source port",
        "<0-0xFFFF>",
        "Layer4 dest port",
        "<0-0xFFFF>")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0;
    uint8  i = 0;
    char  desc[50];
    ctc_parser_udf_t udf;

    sal_memset(&udf, 0, sizeof(ctc_parser_udf_t));
    CTC_CLI_PARSER_UDF_SET(udf)

    index = 0xFFFFFFFF;
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_udf(g_api_lchip, index, &udf);
    }
    else
    {
        ret = ctc_parser_get_udf(index, &udf);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");
    ctc_cli_out(" ----------------------\n");
    ctc_cli_out(" %-16s%s\n", "UdfOffset", "Value");
    ctc_cli_out(" ----------------------\n");

    for (i = 0; i < udf.udf_num; i++)
    {
        sal_sprintf(desc, "ParserOffset%u", i);
        ctc_cli_out(" %-16s%u\n", desc, udf.udf_offset[i]);
    }
    ctc_cli_out("\n");

    return ret;
}

CTC_CLI(ctc_cli_parser_global_set_hash_type,
        ctc_cli_parser_global_set_hash_type_cmd,
        "parser global hash ecmp-hash-type TYPE linkagg-hash-type TYPE",
        CTC_CLI_PARSER_STR,
        "Layer2 3 4",
        "Hash",
        "Ecmp hash type, if set 0 use xor, else use crc",
        "Type value <0x0 - 0xFF>,when greater than 1,as 1 to process",
        "Linkagg hash type, if set 0 use xor, else use crc",
        "Type value <0x0 - 0xFF>,when greater than 1,as 1 to process")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }

    CTC_CLI_GET_UINT8_RANGE("ecmp hash type", global_cfg.ecmp_hash_type, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("linkagg hash type", global_cfg.linkagg_hash_type, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ret ? ret : ctcs_parser_set_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ret ? ret : ctc_parser_set_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_global_hash_type,
        ctc_cli_parser_show_global_hash_type_cmd,
        "show parser global hash-type",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        "Layer2 3 4",
        "Hash type")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out(" %-10s%s\n", "Hash", "Type");
    ctc_cli_out(" ---------------\n");
    ctc_cli_out(" %-10s%s\n", "Ecmp",    global_cfg.ecmp_hash_type ? "crc" : "xor");
    ctc_cli_out(" %-10s%s\n\n", "Linkagg", global_cfg.linkagg_hash_type ? "crc" : "xor");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_l3_type,
        ctc_cli_parser_l3_type_cmd,
        "parser l3-type (ipv4 |ipv6 |mpls |mpls-mcast |arp |\
        rarp | eapol | fcoe | trill | ethoam | slow-proto | cmac | ptp) (enable | disable)",
        CTC_CLI_PARSER_STR,
        CTC_CLI_L3_HDR_STR,
        "ipv4",
        "ipv6",
        "mpls",
        "mpls mcast",
        "arp",
        "rarp",
        "eapol",
        "fcoe",
        "trill",
        "ethoam",
        "slow-proto",
        "cmac",
        "ptp",
        "enable l3 type parser",
        "disable l3 type parser")
{
    ctc_parser_l3_type_t l3_type = 0;
    bool enable = FALSE;
    int32 ret = 0;

    if (0 == sal_memcmp(argv[0], "ipv4", sal_strlen("ipv4")))
    {
        l3_type = CTC_PARSER_L3_TYPE_IPV4;
    }

    if (0 == sal_memcmp(argv[0], "ipv6", sal_strlen("ipv6")))
    {
        l3_type = CTC_PARSER_L3_TYPE_IPV6;
    }

    if (0 == sal_memcmp(argv[0], "mpls", sal_strlen("mpls")))
    {
        l3_type = CTC_PARSER_L3_TYPE_MPLS;
    }

    if (0 == sal_memcmp(argv[0], "mpls-mcast", sal_strlen("mpls-mcast")))
    {
        l3_type = CTC_PARSER_L3_TYPE_MPLS_MCAST;
    }

    if (0 == sal_memcmp(argv[0], "arp", sal_strlen("arp")))
    {
        l3_type = CTC_PARSER_L3_TYPE_ARP;
    }

    if (0 == sal_memcmp(argv[0], "fcoe", sal_strlen("fcoe")))
    {
        l3_type = CTC_PARSER_L3_TYPE_FCOE;
    }

    if (0 == sal_memcmp(argv[0], "trill", sal_strlen("trill")))
    {
        l3_type = CTC_PARSER_L3_TYPE_TRILL;
    }

    if (0 == sal_memcmp(argv[0], "ethoam", sal_strlen("ethoam")))
    {
        l3_type = CTC_PARSER_L3_TYPE_ETHER_OAM;
    }

    if (0 == sal_memcmp(argv[0], "slow-proto", sal_strlen("slow-proto")))
    {
        l3_type = CTC_PARSER_L3_TYPE_SLOW_PROTO;
    }

    if (0 == sal_memcmp(argv[0], "cmac", sal_strlen("cmac")))
    {
        l3_type = CTC_PARSER_L3_TYPE_CMAC;
    }

    if (0 == sal_memcmp(argv[0], "ptp", sal_strlen("ptp")))
    {
        l3_type = CTC_PARSER_L3_TYPE_PTP;
    }

    if (CLI_CLI_STR_EQUAL("enable", 1))
    {
        enable = TRUE;
    }
    else if (CLI_CLI_STR_EQUAL("disable", 1))
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_enable_l3_type(g_api_lchip, l3_type, enable);
    }
    else
    {
        ret = ctc_parser_enable_l3_type(l3_type, enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_parser_global_config,
        ctc_cli_parser_global_config_cmd,
        "parser global (ecmp-hash|linkagg-hash|dlb-efd-hash) \
         tunnel {udp VALUE|vpn-mpls VALUE|mpls VALUE|ip VALUE|gre VALUE|nvgre VALUE|vxlan VALUE| trill VALUE}",
        CTC_CLI_PARSER_STR,
        "Global",
        "Ecmp hash",
        "Linkagg hash",
        "Dlb efd hash",
        "Tunnel",
        "Udp",
        CTC_CLI_PARSER_HASH_MODE,
        "Mpls vpn",
        CTC_CLI_PARSER_HASH_MODE,
        "Mpls",
        CTC_CLI_PARSER_HASH_MODE,
        "IP",
        CTC_CLI_PARSER_HASH_MODE,
        "Gre",
        CTC_CLI_PARSER_HASH_MODE,
        "Nvgre",
        CTC_CLI_PARSER_HASH_MODE,
        "Vxlan",
        CTC_CLI_PARSER_HASH_MODE,
        "Trill",
        CTC_CLI_PARSER_HASH_MODE)
{
    uint8 index = 0;
    uint8* p_h_hash_mode = NULL;
    int32 ret = CLI_SUCCESS;
    ctc_parser_global_cfg_t global_cfg;
    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecmp-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.ecmp_tunnel_hash_mode;
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkagg-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.linkagg_tunnel_hash_mode;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dlb-efd-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.dlb_efd_tunnel_hash_mode;
    }

    if(NULL == p_h_hash_mode)
    {
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("udp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("udp", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_UDP], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpn-mpls");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("vpn-mpls", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_MPLS_VPN], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("mpls", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_MPLS], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("ip", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_IP], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("gre", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_GRE], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("nvgre", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_NVGRE], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("vxlan", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_VXLAN], argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("trill", p_h_hash_mode[CTC_PARSER_TUNNEL_TYPE_TRILL], argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_set_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_set_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_parser_show_global_config,
        ctc_cli_parser_show_global_config_cmd,
        "show parser global (ecmp-hash|linkagg-hash|dlb-efd-hash) tunnel-info",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        "Global",
        "Ecmp hash",
        "Linkagg hash",
        "Dlb efd hash",
        "Tunnel info")
{
    uint8 index = 0, type = 0;
    uint8* p_h_hash_mode = NULL;
    int32 ret = CLI_SUCCESS;
    ctc_parser_global_cfg_t global_cfg;
    char* hash_mode[] = {"Outer", "Merge", "Inner"};
    char* tunnel_type[] = {"Udp", "VpnMpls", "Mpls", "IP", "Trill", "Pbb", "Gre", "Nvgre", "Vxlan"};

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("ecmp-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.ecmp_tunnel_hash_mode;
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkagg-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.linkagg_tunnel_hash_mode;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dlb-efd-hash");
    if (index != 0xFF)
    {
        p_h_hash_mode = global_cfg.dlb_efd_tunnel_hash_mode;
    }

    if(NULL == p_h_hash_mode)
    {
        return CLI_ERROR;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-13s%s\n", "TunnelType", "HashMode");
    ctc_cli_out("----------------------\n");

    for (type = 0; type < CTC_PARSER_TUNNEL_TYPE_MAX; type++)
    {
        ctc_cli_out("%-13s%s\n", tunnel_type[type], hash_mode[p_h_hash_mode[type]]);
    }
    ctc_cli_out("\n");

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_parser_global_set_symmtric_hash,
        ctc_cli_parser_global_set_symmtric_hash_cmd,
        "parser global symmtric-hash (enable|disable)",
        CTC_CLI_PARSER_STR,
        "Global config",
        "Symmtric hash",
        "enable",
        "disable")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }

    if(!sal_strncmp("en", argv[0], 2))
    {
        global_cfg.symmetric_hash_en = 1;
    }
    else
    {
        global_cfg.symmetric_hash_en = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ret ? ret : ctcs_parser_set_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ret ? ret : ctc_parser_set_global_cfg(&global_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_parser_global_show_symmtric_hash,
        ctc_cli_parser_global_show_symmtric_hash_cmd,
        "show parser global symmtric-hash",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PARSER_STR,
        "Global config",
        "Symmtric hash")
{
    ctc_parser_global_cfg_t global_cfg;
    int32 ret = 0;

    sal_memset(&global_cfg, 0, sizeof(ctc_parser_global_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_parser_get_global_cfg(g_api_lchip, &global_cfg);
    }
    else
    {
        ret = ctc_parser_get_global_cfg(&global_cfg);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-13s%s\n", "Symmtric-hash : ", global_cfg.symmetric_hash_en?"Enable":"Disable");

    return CLI_SUCCESS;
}
int32
ctc_parser_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_parser_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_debug_cmd);

    /*set l2 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_set_tpid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_set_vlan_pas_num_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_set_max_len_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_set_pbb_header_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_set_l2flex_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_mapping_l3type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l2_unmapping_l3type_cmd);

    /*set l3 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_set_ip_frag_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_set_l3flex_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_set_trill_header_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_mapping_l4type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_unmapping_l4type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l3_type_cmd);

    /*set l4 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l4_set_flex_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l4_set_app_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_l4_set_app_data_cmd);

    /*set udf parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_udf_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_udf_remove_cmd);

    /*global*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_set_linkagg_hash_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_set_ecmp_hash_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_global_set_hash_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_global_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_global_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_global_set_symmtric_hash_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_global_show_symmtric_hash_cmd);
    /*show l2 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l2_tpid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l2_vlan_pas_num_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l2_max_len_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l2_pbb_header_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l2_l2flex_cmd);

    /*show l3 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l3_ip_frag_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l3_l3flex_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l3_trill_header_cmd);

    /*show l4 parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l4_flex_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l4_app_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_l4_app_data_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_linkagg_hash_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_ecmp_hash_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_global_hash_type_cmd);

    /*show udf parser*/
    install_element(CTC_SDK_MODE, &ctc_cli_parser_show_udf_cmd);

    return CLI_SUCCESS;
}

