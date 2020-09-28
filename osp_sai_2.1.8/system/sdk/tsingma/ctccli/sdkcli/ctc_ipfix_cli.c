/**
 @file ctc_ipfix_cli.c

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-24

 @version v3.0

 The file apply clis of ipfix module
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_ipfix_cli.h"

int32 _ctc_cli_ipfix_parser_mpls_key(unsigned char argc,char** argv, ctc_ipfix_data_t *data)
{
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("label-num");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label-num", data->l3_info.mpls.label_num, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[0].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[1].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("sbit", data->l3_info.mpls.label[1].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("exp", data->l3_info.mpls.label[1].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", data->l3_info.mpls.label[1].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[2].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("sbit", data->l3_info.mpls.label[2].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("exp", data->l3_info.mpls.label[2].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", data->l3_info.mpls.label[2].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    return CLI_SUCCESS;
}
/********************** Field Select *******************/
#define CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "(gport |logic-port |metadata)"
#define CTC_CLI_IPFIX_HASH_FIELD_CID_STR "src-cid | dst-cid"

#define CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC \
"Gport",\
"Logic-port",\
"Metadata"

#define CTC_CLI_IPFIX_HASH_FIELD_CID_STR_DESC \
"Source category id",\
"Dest category id"

#define CTC_CLI_IPFIX_HASH_FIELD_MAC_KEY_STR  \
    "{"CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "|"CTC_CLI_IPFIX_HASH_FIELD_CID_STR "|profile-id| mac-da | mac-sa | eth-type | vlan | cos | cfi}"

#define CTC_CLI_IPFIX_HASH_FIELD_MAC_KEY_DESC  \
  CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC,\
  CTC_CLI_IPFIX_HASH_FIELD_CID_STR_DESC,\
  "Profile id",\
  CTC_CLI_MACDA_DESC,\
  CTC_CLI_MACSA_DESC,\
  CTC_CLI_ETHTYPE_DESC,\
  CTC_CLI_VLAN_DESC,\
  "Cos",\
  "CFI"
STATIC int32 _ctc_cli_ipfix_parser_mac_hash_field_sel(unsigned char argc,char** argv, ctc_ipfix_hash_mac_field_sel_t *mac_field)
{
    uint8 index = 0;
    uint8 start_index = 0;
    start_index = CTC_CLI_GET_ARGC_INDEX("mac-key");

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport",start_index);
    if(index != 0xFF)
    {
        mac_field->gport= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start_index);
    if(index != 0xFF)
    {
        mac_field->logic_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("metadata",start_index);
    if(index != 0xFF)
    {
        mac_field->metadata= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("profile-id",start_index);
    if(index != 0xFF)
    {
        mac_field->profile_id = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("src-cid",start_index);
    if(index != 0xFF)
    {
        mac_field->src_cid = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("dst-cid",start_index);
    if(index != 0xFF)
    {
        mac_field->dst_cid = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mac-da",start_index);
    if(index != 0xFF)
    {
        mac_field->mac_da = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mac-sa",start_index);
    if(index != 0xFF)
    {
        mac_field->mac_sa = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("eth-type",start_index);
    if(index != 0xFF)
    {
        mac_field->eth_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan",start_index);
    if(index != 0xFF)
    {
        mac_field->vlan_id= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("cfi",start_index);
    if(index != 0xFF)
    {
        mac_field->cfi= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("cos",start_index);
    if(index != 0xFF)
    {
        mac_field->cos= 1;
    }
 /*    index = CTC_CLI_GET_SPECIFIC_INDEX("vni",start_index);*/
 /*    if(index != 0xFF)*/
 /*    {*/
 /*       mac_field->vni= 1;*/
 /*    }*/

 /*    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan-valid",start_index);*/
 /*    if(index != 0xFF)*/
 /*    {*/
 /*       mac_field->vlan_valid = 1;*/
 /*    }*/

    return CLI_SUCCESS;
}

#define CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR \
    "label-num | \
     mpls0-label | mpls0-s | mpls0-exp | mpls0-ttl | \
     mpls1-label | mpls1-s | mpls1-exp | mpls1-ttl | \
     mpls2-label | mpls2-s | mpls2-exp | mpls2-ttl"

#define CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR_DESC \
 "Label-num",\
 "MPLS0-label",\
 "MPLS0-s",\
 "MPLS0-exp",\
 "MPLS0-ttl",\
 "NPLS1-label",\
 "NPLS1-s",\
 "MPLS1-exp",\
 "MPLS1-ttl",\
 "MPLS2-label",\
 "MPLS2-s",\
 "MPLS2-exp",\
 "MPLS2-ttl"

#define CTC_CLI_IPFIX_HASH_FIELD_MPLS_KEY_STR \
    "{"CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "|profile-id |"CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR"}"

#define CTC_CLI_IPFIX_HASH_FIELD_MPLS_KEY_STR_DESC \
    CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC,\
    "Profile id",\
    CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR_DESC


STATIC int32 _ctc_cli_ipfix_parser_mpls_hash_field_sel(unsigned char argc,char** argv, ctc_ipfix_hash_mpls_field_sel_t *mpls_field)
{
    uint8 index = 0;
    uint8 start_index = 0;
    start_index = CTC_CLI_GET_ARGC_INDEX("mpls-key");

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport",start_index);
    if(index != 0xFF)
       {
          mpls_field->gport= 1;
       }
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start_index);
    if(index != 0xFF)
       {
          mpls_field->logic_port= 1;
       }
    index = CTC_CLI_GET_SPECIFIC_INDEX("metadata",start_index);
    if(index != 0xFF)
    {
      mpls_field->metadata= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("profile-id",start_index);
    if(index != 0xFF)
    {
        mpls_field->profile_id = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("label-num",start_index);
    if(index != 0xFF)
       {
          mpls_field->label_num= 1;
       }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-label",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label0_label = 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-s",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label0_s= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-exp",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label0_exp= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-ttl",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label0_ttl = 1;
       }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-label",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label1_label = 1;
       }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-s",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label1_s= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-exp",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label1_exp= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-ttl",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label1_ttl = 1;
       }
        index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-label",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label2_label = 1;
       }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-s",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label2_s= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-exp",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label2_exp= 1;
       }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-ttl",start_index);
    if(index != 0xFF)
       {
          mpls_field->mpls_label2_ttl = 1;
       }

    return CLI_SUCCESS;
}
#define CTC_CLI_IPFIX_HASH_FIELD_L4_STR \
"l4-src-port | l4-dst-port | icmp-type | icmp-code | igmp-type"

#define CTC_CLI_IPFIX_HASH_FIELD_L4_STR_DESC \
"L4-src-port",\
"L4-dst-port",\
"Icmp-type ",\
"Icmp-code",\
"Igmp-type"

#define CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR \
"ip-sa mask-len LEN| ip-da mask-len LEN| dscp|ttl|ecn|ip-identification|ip-frag|vrfid|tcp-flags FLAGS|" CTC_CLI_IPFIX_HASH_FIELD_L4_STR

#define CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR_DESC \
"IP-sa",\
"Mask-len",\
"Mask len value<1-32>",\
"IP-da",\
"Mask-len",\
"Mask len value<1-32>",\
"DSCP",\
"TTL",\
"Ecn",\
"IP-idntificaiton",\
"IP-frag",\
"Vrfid",\
"TCP-flags",\
"TCP-flags bitmap <1-0xFF>",\
CTC_CLI_IPFIX_HASH_FIELD_L4_STR_DESC

#define CTC_CLI_IPFIX_HASH_FIELD_IPV4_KEY_STR \
    "{" CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "|profile-id|"CTC_CLI_IPFIX_HASH_FIELD_CID_STR "|"CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR"|ip-pkt-len|gre-key|nvgre-key|ip-protocol|vni|l4-sub-type }"

#define CTC_CLI_IPFIX_HASH_FIELD_IPV4_KEY_STR_DESC \
   CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC, \
   "Profile id",\
   CTC_CLI_IPFIX_HASH_FIELD_CID_STR_DESC,\
   CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR_DESC,\
   "IP-pkt-len",\
   "GRE-key",\
   "NVGRE-key",\
   "IP-protocol",\
   "Vxlan-vni",\
   "L4-sub-type"

STATIC int32 _ctc_cli_ipfix_parser_ipv4_hash_field_sel(unsigned char argc,char** argv, ctc_ipfix_hash_ipv4_field_sel_t *ipv4_field)
{
    uint8 index = 0;
    uint8 mask_len = 0;
    uint8 start_index = 0;
    start_index = CTC_CLI_GET_ARGC_INDEX("ipv4-key");

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport",start_index);
    if(index != 0xFF)
    {
        ipv4_field->gport= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start_index);
    if(index != 0xFF)
    {
        ipv4_field->logic_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("metadata",start_index);
    if(index != 0xFF)
    {
        ipv4_field->metadata= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("profile-id",start_index);
    if(index != 0xFF)
    {
        ipv4_field->profile_id = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("src-cid",start_index);
    if(index != 0xFF)
    {
        ipv4_field->src_cid = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("dst-cid",start_index);
    if(index != 0xFF)
    {
        ipv4_field->dst_cid = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-da",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_da= 1;
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);
        ipv4_field->ip_da_mask = mask_len;

    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-sa",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_sa = 1;
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);
        ipv4_field->ip_sa_mask = mask_len;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("dscp",start_index);
    if(index != 0xFF)
    {
        ipv4_field->dscp = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ttl",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ttl = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ecn",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ecn = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-src-port",start_index);
    if(index != 0xFF)
    {
        ipv4_field->l4_src_port= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-dst-port",start_index);
    if(index != 0xFF)
    {
        ipv4_field->l4_dst_port= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-identification",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_identification = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-pkt-len",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_pkt_len= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-frag",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_frag= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("tcp-flags",start_index);
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tcp-flags", ipv4_field->tcp_flags, argv[index+start_index+1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vrfid",start_index);
    if(index != 0xFF)
    {
        ipv4_field->vrfid= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-type",start_index);
    if(index != 0xFF)
    {
        ipv4_field->icmp_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-code",start_index);
    if(index != 0xFF)
    {
        ipv4_field->icmp_code= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("igmp-type",start_index);
    if(index != 0xFF)
    {
        ipv4_field->igmp_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gre-key",start_index);
    if(index != 0xFF)
    {
        ipv4_field->gre_key= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("nvgre-key",start_index);
    if(index != 0xFF)
    {
        ipv4_field->nvgre_key= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-protocol",start_index);
    if(index != 0xFF)
    {
        ipv4_field->ip_protocol= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-sub-type",start_index);
    if(index != 0xFF)
    {
        ipv4_field->l4_sub_type = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("vni",start_index);
    if(index != 0xFF)
    {
        ipv4_field->vxlan_vni= 1;
    }

    return CLI_SUCCESS;
}

#define CTC_CLI_IPFIX_HASH_FIELD_IPV6_KEY_STR \
"{" CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "|profile-id|"CTC_CLI_IPFIX_HASH_FIELD_CID_STR"|ip-sa mask-len LEN|ip-da mask-len LEN |l4-type|l4-sub-type|flow-label|dscp|ttl|ecn|gre-key |nvgre-key |vni|wlan-radio-id|wlan-radio-mac|wlan-ctl-pkt|aware-tunnel-info-en|ip-protocol|ip-frag|tcp-flags FLAGS|" CTC_CLI_IPFIX_HASH_FIELD_L4_STR " }"

#define CTC_CLI_IPFIX_HASH_FIELD_IPV6_KEY_STR_DESC \
    CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC,\
    "Profile id",\
    CTC_CLI_IPFIX_HASH_FIELD_CID_STR_DESC,\
    "IP-sa",\
    "Mask-len",\
    "Mask len value<4,8,12,...,128>",\
    "IP-da",\
    "Mask-len",\
    "Mask len value<4,8,12,...,128>",\
    "L4-type",\
    "L4-sub-type",\
    "Flow-label",\
    "DSCP",\
    "TTL",\
    "Ecn",\
    "GRE-key",\
    "NVGRE-key",\
    "Vxlan-vni",\
    "Wlan-radio-id",\
    "Wlan-radio-mac",\
    "Wlan-ctl-pkt",\
    "Aware-tunnel-info-en",\
    "IP-protocol",\
    "IP-frag",\
    "Tcp-flags",\
    "Tcp-flags bitmap <1-0xFF>",\
    ""CTC_CLI_IPFIX_HASH_FIELD_L4_STR_DESC

STATIC int32 _ctc_cli_ipfix_parser_ipv6_hash_field_sel(unsigned char argc,char** argv, ctc_ipfix_hash_ipv6_field_sel_t *ipv6_field)
{
    uint8 index = 0;
    uint8 start_index = 0;
    uint8 mask_len = 0;

    start_index = CTC_CLI_GET_ARGC_INDEX("ipv6-key");

    index = CTC_CLI_GET_SPECIFIC_INDEX("ttl",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ttl = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ecn",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ecn= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-frag",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ip_frag = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("tcp-flags",start_index);
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tcp-flags", ipv6_field->tcp_flags, argv[index+start_index+1]);
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("gport",start_index);
    if(index != 0xFF)
    {
        ipv6_field->gport= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start_index);
    if(index != 0xFF)
    {
        ipv6_field->logic_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("metadata",start_index);
    if(index != 0xFF)
    {
        ipv6_field->metadata= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("profile-id",start_index);
    if(index != 0xFF)
    {
        ipv6_field->profile_id = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("src-cid",start_index);
    if(index != 0xFF)
    {
        ipv6_field->src_cid = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("dst-cid",start_index);
    if(index != 0xFF)
    {
        ipv6_field->dst_cid = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-da",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ip_da= 1;
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);

        ipv6_field->ip_da_mask = mask_len;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-sa",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ip_sa = 1;
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);

        ipv6_field->ip_sa_mask = mask_len;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("dscp",start_index);
    if(index != 0xFF)
    {
        ipv6_field->dscp = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gre-key",start_index);
    if(index != 0xFF)
    {
        ipv6_field->gre_key = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nvgre-key",start_index);
    if(index != 0xFF)
    {
        ipv6_field->nvgre_key = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vni",start_index);
    if(index != 0xFF)
    {
        ipv6_field->vxlan_vni= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-radio-id",start_index);
    if(index != 0xFF)
    {
        ipv6_field->wlan_radio_id = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-radio-mac",start_index);
    if(index != 0xFF)
    {
        ipv6_field->wlan_radio_mac = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-ctl-pkt",start_index);
    if(index != 0xFF)
    {
        ipv6_field->wlan_ctl_packet = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("aware-tunnel-info-en",start_index);
    if(index != 0xFF)
    {
        ipv6_field->aware_tunnel_info_en= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-protocol",start_index);
    if(index != 0xFF)
    {
        ipv6_field->ip_protocol = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("flow-label",start_index);
    if(index != 0xFF)
    {
        ipv6_field->flow_label= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-dst-port",start_index);
    if(index != 0xFF)
    {
        ipv6_field->l4_dst_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-src-port",start_index);
    if(index != 0xFF)
    {
        ipv6_field->l4_src_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-type",start_index);
    if(index != 0xFF)
    {
        ipv6_field->icmp_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-code",start_index);
    if(index != 0xFF)
    {
        ipv6_field->icmp_code= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("igmp-type",start_index);
    if(index != 0xFF)
    {
        ipv6_field->igmp_type = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-type",start_index);
    if(index != 0xFF)
    {
        ipv6_field->l4_type = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-sub-type",start_index);
    if(index != 0xFF)
    {
        ipv6_field->l4_sub_type = 1;
    }

    return CLI_SUCCESS;
}
#define CTC_CLI_IPFIX_HASH_FIELD_L2_L3_KEY_STR \
    "{"CTC_CLI_IPFIX_HASH_FIELD_PORT_STR "|profile-id|"CTC_CLI_IPFIX_HASH_FIELD_CID_STR"|mac-da | mac-sa |eth-type | cvlan | ctag-cos | ctag-cfi |" \
    "svlan | stag-cos | stag-cfi | ip-protocol|l4-type|l4-sub-type|" \
    CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR " |gre-key|nvgre-key|vni|wlan-radio-id|wlan-radio-mac|wlan-ctl-pkt|aware-tunnel-info-en|" \
    CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR "}"


#define CTC_CLI_IPFIX_HASH_FIELD_L2_L3_KEY_STR_DESC \
    CTC_CLI_IPFIX_HASH_FIELD_PORT_STR_DESC,\
    "Profile id",\
    CTC_CLI_IPFIX_HASH_FIELD_CID_STR_DESC,\
    CTC_CLI_MACDA_DESC,\
    CTC_CLI_MACSA_DESC,\
    "Eth-type",\
    "Cvlan",\
    "Ctag-cos",\
    "Ctag-cfi",\
    "Svlan",\
    "Stag-cos",\
    "Stag-cfi",\
    "IP protocol",\
    "L4-type",\
    "L4-sub-type",\
    CTC_CLI_IPFIX_HASH_FIELD_IPV4_STR_DESC,\
    "GRE-key",\
    "NVGRE-key",\
    "Vxlan-vni",\
    "Wlan-radio-id",\
    "Wlan-radio-mac",\
    "Wlan-ctl-pkt",\
    "Aware-tunnel-info-en",\
    CTC_CLI_IPFIX_HASH_FIELD_MPLS_STR_DESC

STATIC int32 _ctc_cli_ipfix_parser_l2l3_hash_field_sel(uint8 argc,char** argv, ctc_ipfix_hash_l2_l3_field_sel_t *merge_field)
{
    uint8 index = 0;
    uint8 mask_len = 0;
    uint8 start_index = 0;
    start_index = CTC_CLI_GET_ARGC_INDEX("l2l3-key");

    index = CTC_CLI_GET_SPECIFIC_INDEX("gport",start_index);
    if(index != 0xFF)
    {
        merge_field->gport= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("logic-port",start_index);
    if(index != 0xFF)
    {
        merge_field->logic_port= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("metadata",start_index);
    if(index != 0xFF)
    {
        merge_field->metadata= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("profile-id",start_index);
    if(index != 0xFF)
    {
        merge_field->profile_id = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mac-da",start_index);
    if(index != 0xFF)
    {
        merge_field->mac_da = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mac-sa",start_index);
    if(index != 0xFF)
    {
        merge_field->mac_sa = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("eth-type",start_index);
    if(index != 0xFF)
    {
        merge_field->eth_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("src-cid",start_index);
    if(index != 0xFF)
    {
        merge_field->src_cid = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("dst-cid",start_index);
    if(index != 0xFF)
    {
        merge_field->dst_cid = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("cvlan",start_index);
    if(index != 0xFF)
    {
        merge_field->ctag_vlan= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ctag-cfi",start_index);
    if(index != 0xFF)
    {
        merge_field->ctag_cfi= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ctag-cos",start_index);
    if(index != 0xFF)
    {
        merge_field->ctag_cos= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("svlan",start_index);
    if(index != 0xFF)
    {
        merge_field->stag_vlan= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("stag-cfi",start_index);
    if(index != 0xFF)
    {
        merge_field->stag_cfi= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("stag-cos",start_index);
    if(index != 0xFF)
    {
        merge_field->stag_cos= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-type",start_index);
    if(index != 0xFF)
    {
        merge_field->l4_type= 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-sub-type",start_index);
    if(index != 0xFF)
    {
        merge_field->l4_sub_type = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-da",start_index);
    if(index != 0xFF)
    {
        merge_field->ip_da= 1;

        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);
        merge_field->ip_da_mask = mask_len;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-sa",start_index);
    if(index != 0xFF)
    {
        merge_field->ip_sa = 1;
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[start_index+index+2],0, CTC_MAX_UINT8_VALUE);
        merge_field->ip_sa_mask = mask_len;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("dscp",start_index);
    if(index != 0xFF)
    {
        merge_field->dscp = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-src-port",start_index);
    if(index != 0xFF)
    {
        merge_field->l4_src_port= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("l4-dst-port",start_index);
    if(index != 0xFF)
    {
        merge_field->l4_dst_port= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-type",start_index);
    if(index != 0xFF)
    {
        merge_field->icmp_type= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("icmp-code",start_index);
    if(index != 0xFF)
    {
        merge_field->icmp_code= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("igmp-type",start_index);
    if(index != 0xFF)
    {
        merge_field->igmp_type = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ecn",start_index);
    if(index != 0xFF)
    {
        merge_field->ecn= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("gre-key",start_index);
    if(index != 0xFF)
    {
        merge_field->gre_key= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("nvgre-key",start_index);
    if(index != 0xFF)
    {
        merge_field->nvgre_key= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vni",start_index);
    if(index != 0xFF)
    {
        merge_field->vxlan_vni= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-radio-id",start_index);
    if(index != 0xFF)
    {
        merge_field->wlan_radio_id = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-radio-mac",start_index);
    if(index != 0xFF)
    {
        merge_field->wlan_radio_mac = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("wlan-ctl-pkt",start_index);
    if(index != 0xFF)
    {
        merge_field->wlan_ctl_packet = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("aware-tunnel-info-en",start_index);
    if(index != 0xFF)
    {
        merge_field->aware_tunnel_info_en = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ttl",start_index);
    if(index != 0xFF)
    {
        merge_field->ttl= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-identification",start_index);
    if(index != 0xFF)
    {
        merge_field->ip_identification = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-protocol",start_index);
    if(index != 0xFF)
    {
        merge_field->ip_protocol = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("tcp-flags",start_index);
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tcp-flags", merge_field->tcp_flags, argv[index+start_index+1]);
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("ip-frag",start_index);
    if(index != 0xFF)
    {
        merge_field->ip_frag = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("vrfid",start_index);
    if(index != 0xFF)
    {
        merge_field->vrfid = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("label-num",start_index);
    if(index != 0xFF)
    {
        merge_field->label_num= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-label",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label0_label = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-s",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label0_s= 1;
    }


    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-exp",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label0_exp= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls0-ttl",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label0_ttl = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-label",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label1_label = 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-s",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label1_s= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-exp",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label1_exp= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls1-ttl",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label1_ttl = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-label",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label2_label = 1;
    }
    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-s",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label2_s= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-exp",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label2_exp= 1;
    }

    index = CTC_CLI_GET_SPECIFIC_INDEX("mpls2-ttl",start_index);
    if(index != 0xFF)
    {
        merge_field->mpls_label2_ttl = 1;
    }

    return CLI_SUCCESS;
}


#define CTC_CLI_IPFIX_HASH_FIELD_KEY_STR  \
    "(mac-key" CTC_CLI_IPFIX_HASH_FIELD_MAC_KEY_STR \
    "|ipv4-key" CTC_CLI_IPFIX_HASH_FIELD_IPV4_KEY_STR \
    "|ipv6-key" CTC_CLI_IPFIX_HASH_FIELD_IPV6_KEY_STR \
    "|mpls-key" CTC_CLI_IPFIX_HASH_FIELD_MPLS_KEY_STR \
    "|l2l3-key" CTC_CLI_IPFIX_HASH_FIELD_L2_L3_KEY_STR")"

#define CTC_CLI_IPFIX_HASH_FIELD_KEY_STR_DESC  \
    "Mac-key", CTC_CLI_IPFIX_HASH_FIELD_MAC_KEY_DESC,\
    "IPv4-key", CTC_CLI_IPFIX_HASH_FIELD_IPV4_KEY_STR_DESC,\
    "IPv6-key", CTC_CLI_IPFIX_HASH_FIELD_IPV6_KEY_STR_DESC,\
    "MPLS-key", CTC_CLI_IPFIX_HASH_FIELD_MPLS_KEY_STR_DESC,\
    "L2l3-key", CTC_CLI_IPFIX_HASH_FIELD_L2_L3_KEY_STR_DESC


#if 0
STATIC int32 _ctc_cli_usw_ipfix_parser_mpls_key(unsigned char argc,char** argv, ctc_ipfix_data_t *data)
{
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("label-num");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label-num", data->l3_info.mpls.label_num, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[0].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls0-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("label", data->l3_info.mpls.label[0].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[1].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("sbit", data->l3_info.mpls.label[1].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("exp", data->l3_info.mpls.label[1].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls1-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", data->l3_info.mpls.label[1].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-label");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("label", data->l3_info.mpls.label[2].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-s");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("sbit", data->l3_info.mpls.label[2].sbit, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-exp");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("exp", data->l3_info.mpls.label[2].exp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mpls2-ttl");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", data->l3_info.mpls.label[2].ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    return CLI_SUCCESS;
}
#endif

CTC_CLI(ctc_cli_ipfix_set_hash_field_sel,
         ctc_cli_ipfix_set_hash_field_sel_cmd,
         "ipfix hash-field-sel FIELD_ID" CTC_CLI_IPFIX_HASH_FIELD_KEY_STR,
         CTC_CLI_IPFIX_M_STR,
         CTC_CLI_FIELD_SEL_ID_DESC,
         CTC_CLI_FIELD_SEL_ID_VALUE,
         CTC_CLI_IPFIX_HASH_FIELD_KEY_STR_DESC
         )
{
    ctc_ipfix_hash_field_sel_t field;
    uint8 start_index = 0;
    int32 ret = 0;

    sal_memset(&field, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    CTC_CLI_GET_UINT8("hash-field-sel",field.field_sel_id, argv[0]);

    start_index = CTC_CLI_GET_ARGC_INDEX("mac-key");
    if (start_index != 0xFF)
    {
        _ctc_cli_ipfix_parser_mac_hash_field_sel(argc, &argv[0],&field.u.mac);
        field.key_type = CTC_IPFIX_KEY_HASH_MAC;
    }
    start_index = CTC_CLI_GET_ARGC_INDEX("ipv4-key");
    if (start_index != 0xFF)
    {
        _ctc_cli_ipfix_parser_ipv4_hash_field_sel(argc, &argv[0], &field.u.ipv4);
        field.key_type = CTC_IPFIX_KEY_HASH_IPV4;
    }
    start_index = CTC_CLI_GET_ARGC_INDEX("ipv6-key");
    if (start_index != 0xFF)
    {
        _ctc_cli_ipfix_parser_ipv6_hash_field_sel(argc, &argv[0], &field.u.ipv6);
        field.key_type = CTC_IPFIX_KEY_HASH_IPV6;
    }
    start_index = CTC_CLI_GET_ARGC_INDEX("mpls-key");
    if (start_index != 0xFF)
    {
        _ctc_cli_ipfix_parser_mpls_hash_field_sel(argc, &argv[0], &field.u.mpls);
        field.key_type = CTC_IPFIX_KEY_HASH_MPLS;
    }
    start_index = CTC_CLI_GET_ARGC_INDEX("l2l3-key");
    if (start_index != 0xFF)
    {
        _ctc_cli_ipfix_parser_l2l3_hash_field_sel(argc, &argv[0], &field.u.l2_l3);
        field.key_type = CTC_IPFIX_KEY_HASH_L2_L3;
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_set_hash_field_sel(g_api_lchip, &field);
    }
    else
    {
        ret = ctc_ipfix_set_hash_field_sel(&field);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return 0;
}

CTC_CLI(ctc_cli_ipfix_set_global_cfg,
         ctc_cli_ipfix_set_global_cfg_cmd,
         "ipfix global {aging-interval INTERVAL|conflict-export VALUE"\
         "|packet-cnt VALUE|bytes-cnt VALUE|time-interval VALUE|sample-mode VALUE|tcp-detect-en VALUE"\
         "|sw-learning VALUE|new-flow-export-en VALUE|unkown-pkt-dest-type VALUE| threshold VALUE |latency-type VALUE|clear-conflict-stats}",
         CTC_CLI_IPFIX_M_STR,
         "Global config",
         "Aging Interval",
         "Interval time(s)",
         "Conflict-export",
         CTC_CLI_BOOL_VAR_STR,
         "Number of packets to be export",
         "Value",
         "Size of bytes to be export",
         "Bytes VALUE",
         "Number of time interval to be export",
         "Time interval VALUE",
         "Sample mode",
         "0-all packets;1-unkown packet",
         "Tcp end detect enable",
         "0-disable, 1-enable",
         "Sw learning",
         "0-disable, 1-enable",
         "New flow export",
         "0-do not export to CPU, 1-export to CPU using DMA",
         "Unkown pkt dest type",
         "0:using group id, 1:using vlan id",
         "Flow usage threshold",
         "Value",
         "Latecny type, 0: latency, 1:jitter",
         "Value")
{


    ctc_ipfix_global_cfg_t ipfix_cfg;
    int32 ret = 0;
    uint8 index = 0;
    uint64 byte_cnt = 0;

    sal_memset(&ipfix_cfg, 0, sizeof(ctc_ipfix_global_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_get_global_cfg(g_api_lchip, &ipfix_cfg);
    }
    else
    {
        ret = ctc_ipfix_get_global_cfg(&ipfix_cfg);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aging-interval");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT32("aging-interval", ipfix_cfg.aging_interval, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("conflict-export");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("conflict-export", ipfix_cfg.conflict_export, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("packet-cnt");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT32("packet-cnt", ipfix_cfg.pkt_cnt, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bytes-cnt");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT64("bytes-cnt", byte_cnt, argv[index+1]);
       ipfix_cfg.bytes_cnt = byte_cnt;
    }

    index = CTC_CLI_GET_ARGC_INDEX("time-interval");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT32("time-interval", ipfix_cfg.times_interval, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sample-mode");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("sample-mode", ipfix_cfg.sample_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-detect-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("tcp-detect-en", ipfix_cfg.tcp_end_detect_en, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sw-learning");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("sw-learning", ipfix_cfg.sw_learning_en, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-flow-export-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("new-flow-export-en", ipfix_cfg.new_flow_export_en, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("unkown-pkt-dest-type");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("unkown-pkt-dest-type", ipfix_cfg.unkown_pkt_dest_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("threshold");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT16("threshold", ipfix_cfg.threshold, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("latency-type");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("latency type", ipfix_cfg.latency_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("clear-conflict-stats");
    if(index != 0xFF)
    {
       ipfix_cfg.conflict_cnt = 0;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_set_global_cfg(g_api_lchip, &ipfix_cfg);
    }
    else
    {
        ret = ctc_ipfix_set_global_cfg(&ipfix_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return 0;
}
CTC_CLI(ctc_cli_ipfix_debug_on,
        ctc_cli_ipfix_debug_on_cmd,
        "debug ipfix (ctc|sys) (debug-level {func|param|info|error|export} |) (log LOG_FILE |)",
        CTC_CLI_DEBUG_STR,
        "Ipfix module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR,
        CTC_CLI_DEBUG_LEVEL_EXPORT,
        CTC_CLI_DEBUG_MODE_LOG,
        CTC_CLI_LOG_FILE)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR|CTC_DEBUG_LEVEL_EXPORT;
    uint8 index = 0;
    char file[MAX_FILE_NAME_SIZE];

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

        index = CTC_CLI_GET_ARGC_INDEX("export");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_EXPORT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("log");
    if (index != 0xFF)
    {
        sal_strcpy((char*)&file, argv[index + 1]);
        level |= CTC_DEBUG_LEVEL_LOGFILE;
        ctc_debug_set_log_file("ipfix", "ipfix", (void*)&file, 1);
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPFIX_CTC;
    }
    else
    {
        typeenum = IPFIX_SYS;

    }

    ctc_debug_set_flag("ipfix", "ipfix", typeenum, level, TRUE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_debug_off,
        ctc_cli_ipfix_debug_off_cmd,
        "no debug ipfix (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Ipfix Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPFIX_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = IPFIX_SYS;
    }

    ctc_debug_set_flag("ipfix", "ipfix", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_show_debug,
        ctc_cli_ipfix_show_debug_cmd,
        "show debug ipfix (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Ipfix Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = IPFIX_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = IPFIX_SYS;
    }
    en = ctc_debug_get_flag("ipfix", "ipfix", typeenum, &level) == TRUE;
    ctc_cli_out("Ipfix:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}


#define __IPFIX_CPU_ADD_ENTRY__

CTC_CLI(ctc_cli_ipfix_add_l2_hashkey,
        ctc_cli_ipfix_add_l2_hashkey_cmd,
        "ipfix add mac-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID |" CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR"|" CTC_CLI_IPFIX_KEY_L2_STR "} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key",
        "L2 key",
        "By",
        "By Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        CTC_CLI_IPFIX_KEY_L2_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MAC;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan prio", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_add_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_add_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_del_l2_hashkey,
        ctc_cli_ipfix_del_l2_hashkey_cmd,
        "ipfix delete mac-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID |" CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR "|" CTC_CLI_IPFIX_KEY_L2_STR "} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Delete key",
        "L2 key",
        "By",
        "By Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        CTC_CLI_IPFIX_KEY_L2_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MAC;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan prio", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_delete_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_delete_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_ipfix_add_l2l3_hashkey,
        ctc_cli_ipfix_add_l2l3_hashkey_cmd,
        "ipfix add l2l3-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID)" CTC_CLI_IPFIX_L2_L3_KEY_STR"|"CTC_CLI_IPFIX_KEY_CID_STR "}(dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "L2L3 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_IPFIX_L2_L3_KEY_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_L2_L3;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }


    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan piro", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_add_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_add_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_del_l2l3_hashkey,
        ctc_cli_ipfix_del_l2l3_hashkey_cmd,
        "ipfix delete l2l3-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID) "CTC_CLI_IPFIX_L2_L3_KEY_STR"|"CTC_CLI_IPFIX_KEY_CID_STR"} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "L2L3 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_IPFIX_L2_L3_KEY_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_L2_L3;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan piro", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_delete_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_delete_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_add_ipv4_hashkey,
        ctc_cli_ipfix_add_ipv4_hashkey_cmd,
        "ipfix add ipv4-key by key (src-gport SRCPORT| hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR"|vrfid VRFID_VALUE|"CTC_CLI_IPFIX_KEY_L3_STR"|"CTC_CLI_IPFIX_KEY_IPV4_L4_STR"} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Ipv4 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "Vrfid",
        "Vrfid value",
        CTC_CLI_IPFIX_KEY_L3_STR_DESC,
        CTC_CLI_IPFIX_KEY_IPV4_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;

    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-pkt-len", use_data.l3_info.ipv4.ip_pkt_len, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_add_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_add_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_del_ipv4_hashkey,
        ctc_cli_ipfix_del_ipv4_hashkey_cmd,
        "ipfix delete ipv4-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID |"CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR"|vrfid VRFID_VALUE|"CTC_CLI_IPFIX_KEY_L3_STR"|"CTC_CLI_IPFIX_KEY_IPV4_L4_STR"} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Ipv4 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "vrfid",
        "VRFID_VALUE",
        CTC_CLI_IPFIX_KEY_L3_STR_DESC,
        CTC_CLI_IPFIX_KEY_IPV4_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;

    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-pkt-len", use_data.l3_info.ipv4.ip_pkt_len, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_delete_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_delete_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_add_mpls_hashkey,
        ctc_cli_ipfix_add_mpls_hashkey_cmd,
        "ipfix add mpls-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR "|" CTC_CLI_IPFIX_KEY_MPLS_STR "}" " (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Mpls key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_MPLS_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MPLS;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_add_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_add_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_del_mpls_hashkey,
        ctc_cli_ipfix_del_mpls_hashkey_cmd,
        "ipfix delete mpls-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR "|" CTC_CLI_IPFIX_KEY_MPLS_STR "}" " (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Mpls key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_MPLS_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MPLS;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_delete_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_delete_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_add_ipv6_hashkey,
        ctc_cli_ipfix_add_ipv6_hashkey_cmd,
        "ipfix add ipv6-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID |"CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR "| ip-sa X:X::X:X mask-len LEN| ip-da X:X::X:X mask-len LEN|dscp DSCP|flow-label FLOW_LABEL|ttl TTL|ecn ECN|tcp-flags VALUE|ip-frag FRAG|ip-protocol PROTOCOL|wlan-radio-id ID| wlan-radio-mac MAC |wlan-is-ctl-pkt|" CTC_CLI_IPFIX_KEY_L4_STR "} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Ipv6 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "IP-sa",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "IP-da",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "DSCP",
        "DSCP value",
        "Flow label",
        "Flow label value, it will cover l4_port",
        "TTL",
        "TTL Value",
        "Ecn",
        "Ecn value",
        "TCP-flags",
        "Flags value",
        "IP-frag",
        "IP fragement value",
        "IP-protocol",
        "IP protocol value",
        "Radio-id",
        "ID value",
        "Radio mac",
        CTC_CLI_MAC_FORMAT,
        "Is wlan control packet",
        CTC_CLI_IPFIX_KEY_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    ipv6_addr_t ipv6_address;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipsa_masklen = mask_len;
        use_data.l3_info.ipv6.ipsa[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipsa[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipsa[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipsa[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipda_masklen = mask_len;
        use_data.l3_info.ipv6.ipda[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipda[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipda[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipda[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv6.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("flow-label");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("flow label", use_data.l3_info.ipv6.flow_label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ttl", use_data.l3_info.ipv6.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ecn", use_data.l3_info.ipv6.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_add_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_add_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ipfix_del_ipv6_hashkey,
        ctc_cli_ipfix_del_ipv6_hashkey_cmd,
        "ipfix delete ipv6-key by key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR"|"CTC_CLI_IPFIX_KEY_CID_STR"| ip-sa X:X::X:X mask-len LEN| ip-da X:X::X:X mask-len LEN|dscp DSCP|flow-label FLOW_LABEL|ttl TTL|ecn ECN|tcp-flags VALUE|ip-frag FRAG|ip-protocol PROTOCOL|wlan-radio-id ID|wlan-radio-mac MAC|wlan-is-ctl-pkt|" CTC_CLI_IPFIX_KEY_L4_STR "} (dir DIR)",
        CTC_CLI_IPFIX_M_STR,
        "Add key by cpu",
        "Ipv6 key",
        "By",
        "Key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "IP-sa",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "IP-da",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "DSCP",
        "DSCP value",
        "Flow label",
        "Flow label value, it will cover l4_port",
        "TTL",
        "TTL Value",
        "Ecn",
        "Ecn value",
        "TCP-flags",
        "Flags value",
        "IP-frag",
        "IP fragement value",
        "IP-protocol",
        "IP protocol value",
        "Radio-id",
        "ID value",
        "Radio mac",
        CTC_CLI_MAC_FORMAT,
        "Is wlan control packet",
        CTC_CLI_IPFIX_KEY_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>"
        )
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    ipv6_addr_t ipv6_address;
    int32 ret = CLI_SUCCESS;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipsa_masklen = mask_len;
        use_data.l3_info.ipv6.ipsa[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipsa[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipsa[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipsa[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipda_masklen = mask_len;
        use_data.l3_info.ipv6.ipda[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipda[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipda[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipda[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv6.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("flow-label");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("flow label", use_data.l3_info.ipv6.flow_label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ttl", use_data.l3_info.ipv6.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ecn", use_data.l3_info.ipv6.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_delete_entry(g_api_lchip, &use_data);
    }
    else
    {
        ret = ctc_ipfix_delete_entry(&use_data);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#define ___IPFIX_CPU_ADD_ENTRY_END__

#if (FEATURE_MODE == 0)


CTC_CLI(ctc_cli_ipfix_set_flow_cfg,
        ctc_cli_ipfix_set_flow_cfg_cmd,
        "ipfix flow-cfg PROFILE_ID (ingress|egress) {sampling-mode VALUE sampling-type TYPE sampling-interval INTERVAL | tcpend-detect-dis VALUE | flow-type TYPE"\
        "|learn-disable VALUE |log-pkt-count VALUE}",
        CTC_CLI_IPFIX_M_STR,
        "Flow config",
        "Profile id",
        "Ingress",
        "Egress",
        "Sampling mode",
        "0:fixed interval mode ,1:random mode",
        "Sampling type",
        "0:based on all packets, 1: based on unkown packet",
        "Sampling interval(packet cnt)",
        CTC_CLI_UINT32_VAR_STR,
        "TCP End Detect",
        CTC_CLI_BOOL_VAR_STR,
        "Flow type",
        "0-all packet;1-non-discard packet;2-discard packet",
        "Learn disable",
        "Disable learn new flow based port",
        "Log the first N packets for new flow",
        "Log packets count")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_ipfix_flow_cfg_t flow_cfg;

    sal_memset(&flow_cfg, 0, sizeof(flow_cfg));

    CTC_CLI_GET_UINT8("flow", flow_cfg.profile_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if(index != 0xFF)
    {
        flow_cfg.dir = CTC_INGRESS;
    }
    else
    {
        flow_cfg.dir = CTC_EGRESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_get_flow_cfg(g_api_lchip, &flow_cfg);
    }
    else
    {
        ret = ctc_ipfix_get_flow_cfg(&flow_cfg);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcpend-detect-dis");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tcpend-detect", flow_cfg.tcp_end_detect_disable, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("learn-disable");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("learn-disable", flow_cfg.learn_disable, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sampling-mode");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("sampling-mode", flow_cfg.sample_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sampling-type");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("sampling-type", flow_cfg.sample_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sampling-interval");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT16("sampling-interval", flow_cfg.sample_interval, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-type");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("flow-type", flow_cfg.flow_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("log-pkt-count");
    if(index != 0xFF)
    {
        CTC_CLI_GET_UINT8("log-pkt-count", flow_cfg.log_pkt_count, argv[index+1]);
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_ipfix_set_flow_cfg(g_api_lchip, &flow_cfg);
    }
    else
    {
        ret = ctc_ipfix_set_flow_cfg(&flow_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_ipfix_show_flow_cfg,
        ctc_cli_ipfix_show_flow_cfg_cmd,
        "show ipfix flow-cfg PROFILE_ID (ingress|egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPFIX_M_STR,
        "Flow config for ipfix",
        "Profile-id",
        "Ingress",
        "Egress")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_ipfix_flow_cfg_t ipfix_cfg;

    sal_memset(&ipfix_cfg, 0, sizeof(ctc_ipfix_flow_cfg_t));

    CTC_CLI_GET_UINT8("flow-cfg", ipfix_cfg.profile_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if(index != 0xFF)
    {
        ipfix_cfg.dir = CTC_INGRESS;
    }
    else
    {
        ipfix_cfg.dir = CTC_EGRESS;
    }
    ctc_cli_out("\n");

    if(g_ctcs_api_en)
    {
         ret = ctcs_ipfix_get_flow_cfg(g_api_lchip,  &ipfix_cfg);
    }
    else
    {
        ret = ctc_ipfix_get_flow_cfg(&ipfix_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return ret;
    }

    ctc_cli_out("%s\n", (ipfix_cfg.dir)?"Egress Direction Config":"Ingress Direction Config" );
    ctc_cli_out("%-25s: %d\n", "Sample Mode", ipfix_cfg.sample_mode);
    ctc_cli_out("%-25s: %d\n", "Sample Type", ipfix_cfg.sample_type);
    ctc_cli_out("%-25s: %d\n", "Sample Pkt Interval", ipfix_cfg.sample_interval);
    ctc_cli_out("%-25s: %s\n", "Tcp End Detect", ipfix_cfg.tcp_end_detect_disable?"Disable":"Enable");
    ctc_cli_out("%-25s: %s\n", "Learn Enable", ipfix_cfg.learn_disable?"Disable":"Enable");
    ctc_cli_out("%-25s: %s\n", "Flow Type", (ipfix_cfg.flow_type==0)?"All packet":((ipfix_cfg.flow_type==1)?"No Discard packet":"Discard packet"));
    ctc_cli_out("%-25s: %d\n", "Log-pkt-count", ipfix_cfg.log_pkt_count);

    return CLI_SUCCESS;
}

#endif

int32
ctc_ipfix_cli_init(void)
{
#if (FEATURE_MODE == 0)
	install_element(CTC_SDK_MODE, &ctc_cli_ipfix_set_flow_cfg_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_ipfix_show_flow_cfg_cmd);
#endif

    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_set_hash_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_set_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_show_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_debug_on_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_add_l2_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_add_l2l3_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_add_ipv4_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_add_mpls_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_add_ipv6_hashkey_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_del_l2_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_del_l2l3_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_del_ipv4_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_del_mpls_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ipfix_del_ipv6_hashkey_cmd);
    return CLI_SUCCESS;

}

