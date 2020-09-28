/**
 @file ctc_ipfix_cli.h

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-24

 @version v3.0

 This file contains port cli
*/
#ifndef _CTC_CLI_IPFIX_H
#define _CTC_CLI_IPFIX_H

#include "sal.h"
#include "ctc_cli.h"

/**
 @brief  Initialize sdk IPFIX module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

#ifdef __cplusplus
extern "C" {
#endif

#define CTC_CLI_IPFIX_L4_TYPE_DESC \
"0:None, 1:TCP, 2:UDP, 3:GRE, 4:IPINIP, 5:V6INIP, 6:ICMP, 7:IGMP, 8:SCTP"

#define CTC_CLI_KEY_PORT_STR_DESC \
CTC_CLI_GPORT_DESC,\
CTC_CLI_GPORT_ID_DESC, \
"Logic port",\
"logic port value",\
"Metadata",\
"Metadata value"

#define CTC_CLI_IPFIX_KEY_L2_STR_DESC  \
CTC_CLI_MACDA_DESC,\
CTC_CLI_MAC_FORMAT,\
CTC_CLI_MACSA_DESC,\
CTC_CLI_MAC_FORMAT,\
CTC_CLI_ETHTYPE_DESC,\
"Ether Type value",\
"Cvlan",\
"<0~4k-1>",\
"Cvlan prio",\
"Cvlan prio value",\
"Cvlan cfi",\
"Cvlan cfi value",\
"Svlan",\
"<0~4k-1>",\
"Svlan prio",\
"Svlan prio value",\
"Svlan cfi",\
"Svlan cfi value"

#define CTC_CLI_IPFIX_KEY_L3_STR_DESC  \
"Ip-sa",  \
CTC_CLI_IPV4_FORMAT, \
"Mask-len",  \
"Mask len value<1-32>", \
"Ip-da",  \
CTC_CLI_IPV4_FORMAT,  \
"Mask-len",  \
"Mask len value<0-31>",  \
"Dscp",  \
"Dscp value"

#define CTC_CLI_IPFIX_KEY_L4_STR_DESC \
"L4 Type", \
CTC_CLI_IPFIX_L4_TYPE_DESC, \
"L4-src-port",\
"Scr Port value",\
"L4-dst-port",\
"Dst Port value",\
"ICMP-type ",\
"Type value",\
"ICMP-code",\
"Code value",\
"IGMP-type",\
"TYPE value",\
"TCP flags",\
"TCP flags value",\
"GRE key",\
"GRE key value",\
"Vni",\
"Vni value",\
"L4 sub type",\
"0:None,1:BFD,2:UDPPTP,3:WLAN,4:NTP,5:VXLAN",\
"Aware tunnel info",\
"0:Disable, 1:Enable"

#define CTC_CLI_IPFIX_KEY_CID_STR_DESC \
"Source category id",\
"CID value",\
"Dest category id",\
"CID value"

#define CTC_CLI_IPFIX_DEST_STR_DESC  \
"L2 Mcast dest",\
"L3 Mcast dest",\
"Brocast dest",\
"Aps dest",\
"ECMP dest",\
"Linkagg dest"


#define CTC_CLI_IPFIX_KEY_MPLS_STR_DESC \
"Label-num",\
"Num value",\
"MPLS0-label",\
"Labelvalue",\
"MPLS0-s",\
"Sbit value",\
"MPLS0-exp",\
"Exp value",\
"MPLS0-ttl",\
"TTL value",\
"MPLS1-label",\
"Labelvalue",\
"MPLS1-s",\
"Sbit value",\
"MPLS1-exp",\
"Exp value",\
"MPLS1-ttl",\
"TTL value",\
"MPLS2-label",\
"Labelvalue",\
"MPLS2-s",\
"Sbit value",\
"MPLS2-exp",\
"Exp value",\
"MPLS2-ttl",\
"TTL value"


#define CTC_CLI_IPFIX_KEY_PORT_STR "(gport GPORT |logic-port LPORT |metadata DATA)"
#define CTC_CLI_IPFIX_KEY_L2_STR "mac-da MACDA| mac-sa MACSA|eth-type ETHER| cvlan CVLAN |cvlan-prio PRIO| cvlan-cfi CFI| svlan SVLAN |svlan-prio PRIO|svlan-cfi CFI"
#define CTC_CLI_IPFIX_KEY_L3_STR "ip-sa IPSA mask-len LEN| ip-da IPDA mask-len LEN |dscp DSCP"
#define CTC_CLI_IPFIX_KEY_L4_STR \
"l4-type L4_TYPE |l4-src-port L4SRCPORT| l4-dst-port L4DSTPORT|icmp-type TYPE| icmp-code CODE |igmp-type TYPE|tcp-flags TCP_FLAGS| gre-key GREKEY|vni VNI |l4-sub-type L4_SUB_TYPE |aware-tunnel-info-en VALUE"
#define CTC_CLI_IPFIX_KEY_CID_STR "src-cid CID|dst-cid CID"

#define CTC_CLI_IPFIX_KEY_IPV4_L4_STR \
"ip-protocol PROTOCOL |l4-src-port L4SRCPORT| l4-dst-port L4DSTPORT|icmp-type TYPE| icmp-code CODE |igmp-type TYPE|tcp-flags TCP_FLAGS| ip-frag IP_FRAG |ip-identification IDNTIFICATION|ip-pkt-len PKT_LEN|"\
"ttl TTL| ecn ECN|gre-key GREKEY|vni VNI|l4-sub-type L4_SUB_TYPE | aware-tunnel-info-en VALUE"

#define CTC_CLI_IPFIX_KEY_IPV4_L4_STR_DESC \
"Ip Protocol", \
"Protocol value", \
"L4-src-port",\
"Src Port value",\
"L4-dst-port",\
"Dst Port value",\
"ICMP-type ",\
"Type value",\
"ICMP-code",\
"Code value",\
"IGMP-type",\
"TYPE value",\
"TCP-flags",\
"TCP FLAGS value",\
"IP-frag",\
"FRAG value",\
"IP-identification",\
"IDENTIFICATION",\
"IP packet length",\
"Packet length value",\
"TTL",\
"TTL value",\
"Ecn",\
"Ecn value",\
"GRE key",\
"GRE Key value",\
"Vni",\
"Vni value",\
"L4 Sub Type",\
"0:None,1:BFD,2:UDPPTP,3:WLAN,4:NTP,5:VXLAN",\
"Aware tunnel info",\
"0:Disable, 1:Enable"
 /*#define CTC_CLI_IPFIX_KEY_IPV4_STR "("CTC_CLI_IPFIX_KEY_L3_STR"|" CTC_CLI_IPFIX_KEY_L4_STR ")"*/

#define CTC_CLI_IPFIX_KEY_MPLS_STR \
    "label-num NUM| \
     mpls0-label LABEL| mpls0-s SBIT| mpls0-exp EXP| mpls0-ttl TTL| \
     mpls1-label LABEL| mpls1-s SBIT| mpls1-exp EXP| mpls1-ttl TTL| \
     mpls2-label LABEL| mpls2-s SBIT| mpls2-exp EXP| mpls2-ttl TTL"

#define CTC_CLI_IPFIX_L2_L3_KEY_STR \
    "{profile-id PROFILE_ID |" CTC_CLI_IPFIX_KEY_PORT_STR "|" CTC_CLI_IPFIX_KEY_L2_STR "|" \
    CTC_CLI_IPFIX_KEY_L3_STR "|ip-protocol PROTOCOL|ip-identification IDENTIFICATION|ip-frag IP_FRAG_VALUE|ttl TTL|ecn ECN|vrfid VRFID|wlan-radio-id ID| wlan-radio-mac MAC |wlan-is-ctl-pkt|"CTC_CLI_IPFIX_KEY_L4_STR"|"\
    CTC_CLI_IPFIX_KEY_MPLS_STR

#define CTC_CLI_IPFIX_L2_L3_KEY_STR_DESC \
"Config profile id",\
"Profile id",\
CTC_CLI_KEY_PORT_STR_DESC,\
CTC_CLI_IPFIX_KEY_L2_STR_DESC,\
CTC_CLI_IPFIX_KEY_L3_STR_DESC,\
"IP Protocol",\
"Protocol value",\
"IP-identification",\
"IDENTIFICATION",\
"IP frag",\
"IP_FRAG_VALUE",\
"TTL",\
"TTL value",\
"Ecn",\
"Ecn value",\
"Vrfid",\
"Vrfid value",\
"Radio-id",\
"ID value",\
"Radio mac",\
CTC_CLI_MAC_FORMAT,\
"Is wlan control packet",\
CTC_CLI_IPFIX_KEY_L4_STR_DESC,\
CTC_CLI_IPFIX_KEY_MPLS_STR_DESC
extern int32
ctc_ipfix_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif
