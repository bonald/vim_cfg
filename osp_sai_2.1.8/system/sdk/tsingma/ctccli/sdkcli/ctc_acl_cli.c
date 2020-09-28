/**
 @date 2009-12-22

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_acl_cli.h"


 /*#include <time.h>*/

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

#define CTC_CLI_ACL_ADD_ENTRY_STR  "acl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|) "
#define CTC_CLI_ACL_ADD_HASH_ENTRY_STR  "acl add (group GROUP_ID|) (entry ENTRY_ID)"

#define CTC_CLI_ACL_ADD_HASH_ENTRY_DESC \
    CTC_CLI_ACL_STR, \
    "Add one entry to software table", \
    CTC_CLI_ACL_GROUP_ID_STR, \
    CTC_CLI_ACL_B_GROUP_ID_VALUE, \
    CTC_CLI_ACL_ENTRY_ID_STR, \
    CTC_CLI_ACL_ENTRY_ID_VALUE

#define CTC_CLI_ACL_ADD_ENTRY_DESC \
    CTC_CLI_ACL_STR, \
    "Add one entry to software table", \
    CTC_CLI_ACL_GROUP_ID_STR, \
    CTC_CLI_ACL_B_GROUP_ID_VALUE, \
    CTC_CLI_ACL_ENTRY_ID_STR, \
    CTC_CLI_ACL_ENTRY_ID_VALUE, \
    CTC_CLI_ACL_ENTRY_PRIO_STR, \
    CTC_CLI_ACL_ENTRY_PRIO_VALUE

/****************************************************************
 *           acl string (parameter)   -begin-                   *
 ****************************************************************/

/*l2 (mac ipv4 ipv6) share this. put this at Head*/
#define CTC_CLI_ACL_KEY_L2_STR_0  \
    " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK             \
             | cvlan VLAN_ID (to-cvlan VLAN_ID |) MASK | ctag-cos COS MASK | ctag-cfi CFI       \
             | svlan VLAN_ID (to-svlan VLAN_ID |) MASK | stag-cos COS MASK | stag-cfi CFI       \
             | layer2-type L2_TYPE MASK | vrange-grp VRANGE_GROUP_ID"
#define CTC_CLI_ACL_KEY_L2_STR_1  \
    "| vlan-num VLAN_NUM MASK | layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK            \
             | stag-valid VALID | ctag-valid VALID                         "

#define CTC_CLI_ACL_PBR_KEY_L2_STR_0  \
    " mac-sa MAC MASK | mac-da MAC MASK | cos COS MASK             \
             | cvlan VLAN_ID MASK | ctag-cos COS MASK | ctag-cfi CFI       \
             | svlan VLAN_ID MASK | stag-cos COS MASK | stag-cfi CFI       \
             | layer2-type L2_TYPE MASK "

#define CTC_CLI_ACL_PBR_KEY_L2_STR_1  \
    "| layer3-type L3_TYPE MASK |eth-type ETH_TYPE MASK "

#define CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_STR  \
    "|l4-protocol L4_PROTOCOL_VALUE L4_PROTOCOL_MASK ({l4-src-port (use-mask|use-range) port0 PORT port1 PORT | \
                                                      l4-dst-port (use-mask|use-range) port0 PORT port1 PORT | \
                                                      icmp-type ICMP_TYPE MASK | \
                                                      icmp-code ICMP_CODE MASK | \
                                                      gre-crks GRE_CRKS MASK | \
                                                      gre-key GRE_KEY MASK (is-nvgre|) | \
                                                      tcp-flags (match-any|match-all) FLAGS |} |)"

#define CTC_CLI_ACL_KEY_IPV4_L4_STR  \
    CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_STR "(igmp-type IGMP_TYPE  MASK|vnid VNID MASK |)"

#define CTC_CLI_ACL_KEY_IPV6_L4_STR  \
    CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_STR "(vnid VNID MASK |)"

/*(ipv4 ipv6) share this*/
#define CTC_CLI_ACL_KEY_L3_STR  \
    "| dscp DSCP MASK |prec IP_PREC MASK                         \
               | ip-frag IP_FRAG  | ip-hdr-error IP_HDR_ERROR        \
               | ip-option IP_OPTION | routed-packet ROUTED_PACKET | pkt-len-range LEN_MIN LEN_MAX  "

#define CTC_CLI_ACL_PBR_IPV6_KEY_L3_STR  \
    "| dscp DSCP MASK |prec IP_PREC MASK  \
               | ip-frag IP_FRAG          \
               | vrfid VRFID MASK"

#define CTC_CLI_ACL_PBR_KEY_L3_STR  \
    "ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D \
               | dscp DSCP MASK |prec IP_PREC MASK                         \
               | ip-frag IP_FRAG           \
               | vrfid VRFID MASK"
/*mac unique*/
#define CTC_CLI_ACL_KEY_MAC_UNIQUE_STR \
    "| arp-op-code ARP_OP_CODE MASK                                \
             | ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D | metadata METADATA MASK     "

/*ipv4 unique (NOTE: DO NOT split line)*/
#define CTC_CLI_ACL_KEY_IPV4_UNIQUE_STR \
    "| ethoam-level VALUE MASK | ethoam-opcode VALUE MASK | ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D | metadata METADATA MASK | udf UDF MASK udf-type TYPE | (arp-packet ARP_PACKET|) (arp-op-code ARP_OP_CODE MASK|) (arp-sender-ip A.B.C.D A.B.C.D|) (arp-target-ip A.B.C.D A.B.C.D|) | ipv4-packet IPV4_PACKET | mpls-label-num PARSER_MPLS_LABEL_NUMBER MASK | mpls-label0 LABEL_0 LABEL_0_MASK | mpls-label1 LABEL_1 LABEL_1_MASK | mpls-label2 LABEL_2 LABEL_2_MASK"
/*ipv6 unique (NOTE: DO NOT split line)  */
#define CTC_CLI_ACL_KEY_IPV6_UNIQUE_STR \
    "| ip-sa X:X::X:X  (host|X:X::X:X) | ip-da X:X::X:X (host|X:X::X:X) | metadata METADATA MASK | udf UDF MASK udf-type TYPE | ext-hdr EXT_HDR EXT_HDR_MASK | flow-label FLOW_LABEL MASK "

#define CTC_CLI_ACL_KEY_IPV6_PBR_UNIQUE_STR \
    "| ip-sa X:X::X:X  (host|X:X::X:X) | ip-da X:X::X:X (host|X:X::X:X) | ext-hdr EXT_HDR EXT_HDR_MASK | flow-label FLOW_LABEL MASK "

/*mpls unique*/
#define CTC_CLI_ACL_KEY_MPLS_UNIQUE_STR \
    "| routed-packet ROUTED_PACKET     \
               | mpls-label0 MPLS_LABEL MASK | mpls-label1 MPLS_LABEL MASK \
               | mpls-label2 MPLS_LABEL MASK | mpls-label3 MPLS_LABEL MASK \
               | mpls-label-number PARSER_MPLS_LABEL_NUMBER MASK | metadata METADATA MASK"
/*action*/
#define CTC_CLI_ACL_ACTION_STR           \
    "(permit|deny|cancel-deny)({stats STATS_ID |deny-bridge| deny-learning | deny-route | force-route | priority PRIORITY | color COLOR | priority-and-color PRIORITY COLOR \
    | trust TRUST | micro-flow-policer MICRO_POLICER_ID |macro-flow-policer MACRO_POLICER_ID\
    | random-log SESSION_ID LOG_PERCENT |copy-to-cpu| disable-ipfix |deny-ipfix-learning| postcard-en\
    | redirect NH_ID | new-dscp DSCP| to-cpu-with-timestamp |meta-data METADATA |fid FID |vrf VRFID \
    | qos-domain QOS_DOMAIN | aging | vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|)(tpid-index TPID_INDEX|) (scfi-sl TAG_SL (new-scfi CFI|)|)|) \
     (ctag-op TAG_OP  (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)| )\
    | strip-packet start-to TO_HEADER packet-type PKT_TYPE (extra-len EX_LEN|) | redirect-with-raw-pkt}|)"

#define CTC_CLI_ACL_PBR_ACTION_STR           \
    "(permit|deny) ({forward NH_ID | ecmp | ttl-check | icmp-check | copy-to-cpu}|)"

/*---------------------------------   hash key --------------------------------*/
#define CTC_CLI_ACL_HASH_L4_STR \
"l4-src-port L4_SRC_PORT| l4-dst-port L4_DST_PORT | icmp-type ICMP_TYPE | icmp-code ICMP_CODE |vni VNI|gre-key KEY|"

#define CTC_CLI_ACL_HASH_IPV4_STR \
"ip-sa A.B.C.D | ip-da A.B.C.D | dscp DSCP | ecn ECN |" CTC_CLI_ACL_HASH_L4_STR

#define CTC_CLI_ACL_HASH_MPLS_STR \
    "mpls-label-num LABEL_NUM| \
     mpls0-label MPLS_LABEL | mpls0-s MPLS_S | mpls0-exp MPLS_EXP | mpls0-ttl MPLS_TTL| \
     mpls1-label MPLS_LABEL | mpls1-s MPLS_S | mpls1-exp MPLS_EXP | mpls1-ttl MPLS_TTL| \
     mpls2-label MPLS_LABEL | mpls2-s MPLS_S | mpls2-exp MPLS_EXP | mpls2-ttl MPLS_TTL|"

#define CTC_CLI_ACL_HASH_MAC_KEY_STR  \
    "{field-sel-id SEL_ID | gport GPORT_ID | is-logic-port IS_LOGIC_PORT | mac-da MAC | mac-sa MAC |\
      eth-type ETH_TYPE | vlan VLAN_ID (to-vlan VLAN_ID |) | cos COS | tag-valid VALID | cfi CFI | is-ctag | vrange-grp VRANGE_GROUP_ID|}"

#define CTC_CLI_ACL_HASH_MPLS_KEY_STR \
    "{field-sel-id SEL_ID| gport GPORT_ID| metadata METADATA|" \
    CTC_CLI_ACL_HASH_MPLS_STR "|}"

#define CTC_CLI_ACL_HASH_IPV4_KEY_STR \
    "{field-sel-id SEL_ID| gport GPORT_ID| is-logic-port IS_LOGIC_PORT| metadata METADATA| " \
    CTC_CLI_ACL_HASH_IPV4_STR  " l4-protocol L4_PROTOCOL| arp-packet ARP_PACKET|}"

#define CTC_CLI_ACL_HASH_IPV6_KEY_STR \
    "{field-sel-id SEL_ID| gport GPORT_ID| ip-sa X:X::X:X | ip-da X:X::X:X | dscp DSCP| l4-type L4_TYPE | "\
    CTC_CLI_ACL_HASH_L4_STR "|}"

#define CTC_CLI_ACL_HASH_L2_L3_KEY_STR \
    "{field-sel-id SEL_ID | gport GPORT_ID | metadata METADATA | mac-da MAC | mac-sa MAC | l3-type L3_TYPE |\
      eth-type ETH_TYPE | cvlan VLAN_ID (to-cvlan VLAN_ID |) | ctag-cos COS | ctag-cfi CFI | svlan VLAN_ID (to-svlan VLAN_ID |) | stag-cos COS | stag-cfi CFI| l4-type L4_TYPE|\
      stag-valid VALID | ctag-valid VALID | vrange-grp VRANGE_GROUP_ID | " \
      CTC_CLI_ACL_HASH_IPV4_STR "ttl TTL| " \
      CTC_CLI_ACL_HASH_MPLS_STR "|}"


/********************** Field Select *******************/
#define CTC_CLI_ACL_HASH_FIELD_PORT_STR "{gport |logic-port |metadata|"

#define CTC_CLI_ACL_HASH_FIELD_L4_STR \
"l4-src-port | l4-dst-port |icmp-type | icmp-code |vni|gre-key|"


#define CTC_CLI_ACL_HASH_FIELD_IPV4_STR \
"ip-sa | ip-da | dscp | ecn |" CTC_CLI_ACL_HASH_FIELD_L4_STR

#define CTC_CLI_ACL_HASH_FIELD_MPLS_STR \
    "mpls-label-num | \
     mpls0-label | mpls0-s | mpls0-exp | mpls0-ttl | \
     mpls1-label | mpls1-s | mpls1-exp | mpls1-ttl | \
     mpls2-label | mpls2-s | mpls2-exp | mpls2-ttl |"

#define CTC_CLI_ACL_HASH_FIELD_MAC_KEY_STR  \
    CTC_CLI_ACL_HASH_FIELD_PORT_STR \
    "mac-da | mac-sa |eth-type | vlan | cos | tag-valid | cfi |}"

#define CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_STR \
    CTC_CLI_ACL_HASH_FIELD_PORT_STR \
    CTC_CLI_ACL_HASH_FIELD_MPLS_STR "}"

#define CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_STR \
    CTC_CLI_ACL_HASH_FIELD_PORT_STR \
    CTC_CLI_ACL_HASH_FIELD_IPV4_STR  " l4-protocol |}"

#define CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_STR \
    CTC_CLI_ACL_HASH_FIELD_PORT_STR \
    "ip-sa| ip-da| dscp| l4-type | "\
    CTC_CLI_ACL_HASH_FIELD_L4_STR "}"

#define CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_STR \
    CTC_CLI_ACL_HASH_FIELD_PORT_STR \
    "mac-da | mac-sa |l3-type |eth-type | cvlan | ctag-cos | ctag-cfi |" \
    "svlan | stag-cos | stag-cfi |stag-valid | ctag-valid | l4-type| " \
    CTC_CLI_ACL_HASH_FIELD_IPV4_STR "ttl | " \
    CTC_CLI_ACL_HASH_FIELD_MPLS_STR "}"

#define CTC_CLI_ACL_KEY_PORT_STR      \
    " (port-bitmap (pbmp3 PORT_BITMAP_3 PORT_BITMAP_2|) PORT_BITMAP_HIGH PORT_BITMAP_LOW (lchip LCHIP|)\
        | vlan-class CLASS_ID | port-class CLASS_ID |pbr-class CLASS_ID |logic-port LOGIC_PORT | gport GPORT)|"

/****************************************************************
 *           acl description    -begin-                         *
 ****************************************************************/

/************** acl hash field select desc ******************/
#define CTC_CLI_ACL_HASH_FIELD_PORT_DESC \
    "PhyPort",          \
    "Logic-port",      \
    "Metadata"

#define CTC_CLI_ACL_HASH_FIELD_L4_DESC  \
    "Layer4 source port",         \
    "Layer4 destination port",    \
    "Icmp type",                  \
    "Icmp code",                  \
    "Vxlan vni",                  \
    "Gre or nvgre key"

#define CTC_CLI_ACL_HASH_FIELD_IPV4_DESC \
    "IPv4 source address",        \
    "IPv4 destination address",   \
    "Dscp",                       \
    "Ecn",                        \
    CTC_CLI_ACL_HASH_FIELD_L4_DESC

#define CTC_CLI_ACL_HASH_FIELD_MPLS_DESC \
    "Mpls label num ",             \
    "Mpls label 0",                \
    "Mpls s-bit 0",                \
    "Mpls Exp 0",                  \
    "Mpls TTL 0",                  \
    "Mpls label 1",                \
    "Mpls s-bit 1",                \
    "Mpls Exp 1",                  \
    "Mpls TTL 1",                  \
    "Mpls label 2",                \
    "Mpls s-bit 2",                \
    "Mpls Exp 2",                  \
    "Mpls TTL 2"

#define CTC_CLI_ACL_HASH_FIELD_MAC_KEY_DESC \
    CTC_CLI_ACL_HASH_FIELD_PORT_DESC,   \
    "MAC destination address",    \
    "MAC source address",         \
    "Ether type",                 \
    "Vlan id",                    \
    "Cos",                        \
    "Tag Valid",                  \
    "CFI"


#define CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_DESC \
    CTC_CLI_ACL_HASH_FIELD_PORT_DESC,   \
    CTC_CLI_ACL_HASH_FIELD_MPLS_DESC

#define CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_DESC \
    CTC_CLI_ACL_HASH_FIELD_PORT_DESC,   \
    CTC_CLI_ACL_HASH_FIELD_IPV4_DESC,   \
    "L4 protocol "


#define CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_DESC \
    CTC_CLI_ACL_HASH_FIELD_PORT_DESC,   \
    "IPv6 source address",        \
    "IPv6 destination address",   \
    "Dscp",                       \
    "Layer4 type",                \
    CTC_CLI_ACL_HASH_FIELD_L4_DESC


#define CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_DESC \
    CTC_CLI_ACL_HASH_FIELD_PORT_DESC,   \
    "MAC destination address",    \
    "MAC source address",         \
    "Layer3 type",                \
    "Ether type",                 \
    "Customer VLAN",              \
    "Customer VLAN CoS",          \
    "Customer VLAN CFI",          \
    "Service VLAN",               \
    "Service VLAN CoS",           \
    "Service VLAN CFI",           \
    "Stag valid",                 \
    "Ctag valid",                 \
    "Layer4 type",                \
    CTC_CLI_ACL_HASH_FIELD_IPV4_DESC,\
    "TTL",                        \
    CTC_CLI_ACL_HASH_FIELD_MPLS_DESC


/************** acl hash desc ******************/
/*l2 (mac ipv4 ipv6) share this. put this at Head.*/
#define CTC_CLI_ACL_HASH_L4_DESC  \
    "Layer4 source port",         \
    CTC_CLI_L4_PORT_VALUE,        \
    "Layer4 destination port",    \
    CTC_CLI_L4_PORT_VALUE,        \
    "Icmp type",                  \
    CTC_CLI_ICMP_TYPE_VALUE,      \
    "Icmp code",                  \
    CTC_CLI_ICMP_CODE_VALUE,      \
    "Vxlan vni ID",               \
    CTC_CLI_VNID_VALUE,           \
    "Gre or nvgre Key",           \
    CTC_CLI_GRE_KEY_VALUE


#define CTC_CLI_ACL_HASH_IPV4_DESC \
    "IPv4 source address",        \
    CTC_CLI_IPV4_FORMAT,          \
    "IPv4 destination address",   \
    CTC_CLI_IPV4_FORMAT,          \
    "Dscp",                       \
    CTC_CLI_DSCP_VALUE,           \
    "Ecn",                        \
    CTC_CLI_ECN_VALUE,            \
    CTC_CLI_ACL_HASH_L4_DESC

#define CTC_CLI_ACL_HASH_MPLS_DESC \
    "Mpls label num ",             \
    "Mpls label num. like <0-9>",  \
    "Mpls label 0",                \
    CTC_CLI_MPLS_LABLE_VALUE,      \
    "Mpls s-bit 0",                \
    CTC_CLI_MPLS_S_VALUE,          \
    "Mpls Exp 0",                  \
    CTC_CLI_MPLS_EXP_VALUE,        \
    "Mpls TTL 0",                  \
    CTC_CLI_MPLS_TTL_VALUE,        \
    "Mpls label 1",                \
    CTC_CLI_MPLS_LABLE_VALUE,      \
    "Mpls s-bit 1",                \
    CTC_CLI_MPLS_S_VALUE,          \
    "Mpls Exp 1",                  \
    CTC_CLI_MPLS_EXP_VALUE,        \
    "Mpls TTL 1",                  \
    CTC_CLI_MPLS_TTL_VALUE,        \
    "Mpls label 2",                \
    CTC_CLI_MPLS_LABLE_VALUE,      \
    "Mpls s-bit 2",                \
    CTC_CLI_MPLS_S_VALUE,          \
    "Mpls Exp 2",                  \
    CTC_CLI_MPLS_EXP_VALUE,        \
    "Mpls TTL 2",                  \
    CTC_CLI_MPLS_TTL_VALUE

#define CTC_CLI_ACL_HASH_MAC_KEY_DESC \
    CTC_CLI_FIELD_SEL_ID_DESC,    \
    CTC_CLI_FIELD_SEL_ID_VALUE,   \
    CTC_CLI_GPORT_DESC,           \
    "PhyPort or logic-port, decided by is-logic-port. PhyPort including linkagg",     \
    "Is logic port",              \
    "<0-1>",                      \
    "MAC destination address",    \
    CTC_CLI_MAC_FORMAT,           \
    "MAC source address",         \
    CTC_CLI_MAC_FORMAT,           \
    "Ether type",                 \
    CTC_CLI_ETHER_TYPE_VALUE,     \
    "VLAN, or start vlan of vlan range",  \
    CTC_CLI_VLAN_RANGE_DESC,   \
    "Vlan range, end vlan",            \
    CTC_CLI_VLAN_RANGE_DESC,      \
    "Cos",                        \
    CTC_CLI_COS_RANGE_DESC,       \
    "Tag Valid",                  \
    "<0-1>",                      \
    "CFI",                        \
    CTC_CLI_CFI_RANGE_DESC,       \
    "The tag is ctag. (default is stag)",   \
    "Vlan range group id",         \
    "<0-63>"

#define CTC_CLI_ACL_HASH_MPLS_KEY_DESC \
    CTC_CLI_FIELD_SEL_ID_DESC,    \
    CTC_CLI_FIELD_SEL_ID_VALUE,   \
    CTC_CLI_GPORT_DESC,           \
    "PhyPort or logic-port.",     \
    "Metadata",                   \
    "Metadata Value",             \
    CTC_CLI_ACL_HASH_MPLS_DESC

#define CTC_CLI_ACL_HASH_IPV4_KEY_DESC \
    CTC_CLI_FIELD_SEL_ID_DESC,    \
    CTC_CLI_FIELD_SEL_ID_VALUE,   \
    CTC_CLI_GPORT_DESC,           \
    "PhyPort or logic-port.",     \
    "Is logic port",              \
    "<0-1>",                      \
    "Metadata",                   \
    "Metadata Value",             \
    CTC_CLI_ACL_HASH_IPV4_DESC,   \
    "L4 protocol ",               \
    CTC_CLI_L4_PROTOCOL_VALUE,    \
    "Arp packet",                 \
    CTC_CLI_ARP_PACKET_VALUE

#define CTC_CLI_ACL_HASH_IPV6_KEY_DESC \
    CTC_CLI_FIELD_SEL_ID_DESC,    \
    CTC_CLI_FIELD_SEL_ID_VALUE,   \
    CTC_CLI_GPORT_DESC,           \
    "PhyPort or logic-port.",     \
    "IPv6 source address",        \
    CTC_CLI_IPV6_FORMAT,          \
    "IPv6 destination address",   \
    CTC_CLI_IPV6_FORMAT,          \
    "Dscp",                       \
    CTC_CLI_DSCP_VALUE,           \
    "Layer4 type",                \
    "Layer4 type value. <0-15>",  \
    CTC_CLI_ACL_HASH_L4_DESC


#define CTC_CLI_ACL_HASH_L2_L3_KEY_DESC \
    CTC_CLI_FIELD_SEL_ID_DESC,    \
    CTC_CLI_FIELD_SEL_ID_VALUE,   \
    CTC_CLI_GPORT_DESC,           \
    "PhyPort or logic-port.",     \
    "Metadata",                   \
    "Metadata Value",             \
    "MAC destination address",    \
    CTC_CLI_MAC_FORMAT,           \
    "MAC source address",         \
    CTC_CLI_MAC_FORMAT,           \
    "Layer3 type",                \
    "Layer3 type value. <0-15>",  \
    "Ether type",                 \
    CTC_CLI_ETHER_TYPE_VALUE,     \
    "Customer VLAN, or start vlan of vlan range",  \
    CTC_CLI_VLAN_RANGE_DESC,   \
    "Vlan range, end vlan",            \
    CTC_CLI_VLAN_RANGE_DESC,      \
    "Customer VLAN CoS",          \
    CTC_CLI_COS_RANGE_DESC,       \
    "Customer VLAN CFI",          \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Service VLAN, or start vlan of vlan range",  \
    CTC_CLI_VLAN_RANGE_DESC,   \
    "Vlan range, end vlan",            \
    CTC_CLI_VLAN_RANGE_DESC,      \
    "Service VLAN CoS",           \
    CTC_CLI_COS_RANGE_DESC,       \
    "Service VLAN CFI",           \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Layer4 type",                \
    "Layer4 type value. <0-15>",  \
    "Stag valid",                 \
    "<0-1>",                      \
    "Ctag valid",                 \
    "<0-1>",                      \
    "Vlan range group id",        \
    "<0-63>",                     \
    CTC_CLI_ACL_HASH_IPV4_DESC,   \
    "TTL",                        \
    CTC_CLI_MPLS_TTL_VALUE,       \
    CTC_CLI_ACL_HASH_MPLS_DESC


#define CTC_CLI_ACL_KEY_L2_DESC_0 \
    "MAC source address",         \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "MAC destination address",    \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "Source CoS",                 \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Customer VLAN, or start vlan of vlan range",  \
    CTC_CLI_VLAN_RANGE_DESC,   \
    "Vlan range, end vlan",            \
    CTC_CLI_VLAN_RANGE_DESC,    \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Customer VLAN CoS",          \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Customer VLAN CFI",          \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Service VLAN, or start vlan of vlan range",  \
    CTC_CLI_VLAN_RANGE_DESC,   \
    "Vlan range, end vlan",            \
    CTC_CLI_VLAN_RANGE_DESC,    \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Service VLAN CoS",           \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Service VLAN CFI",           \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Layer 2 Type",               \
    CTC_CLI_PARSER_L2_TYPE_VALUE, \
    CTC_CLI_PARSER_L2_TYPE_MASK,  \
    "Vlan range group id",         \
    "<0-63>"

#define CTC_CLI_ACL_KEY_L2_DESC_1 \
    "VLAN number",               \
    "VLAN NUMBER",                \
    "<0-0xF>",                    \
    "Layer 3 Type",               \
    CTC_CLI_PARSER_L3_TYPE_VALUE, \
    CTC_CLI_PARSER_L3_TYPE_MASK,  \
    "Ether type",                 \
    CTC_CLI_ETHER_TYPE_VALUE,     \
    CTC_CLI_ETHER_TYPE_MASK,      \
    "Stag valid",                 \
    "<0-1>",                      \
    "Ctag valid",                 \
    "<0-1>"

#define CTC_CLI_ACL_PBR_KEY_L2_DESC_0 \
    "MAC source address",         \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "MAC destination address",    \
    CTC_CLI_MAC_FORMAT,           \
    CTC_CLI_MAC_FORMAT,           \
    "Source CoS",                 \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Customer VLAN",              \
    CTC_CLI_VLAN_RANGE_DESC,      \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Customer VLAN CoS",          \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Customer VLAN CFI",          \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Service VLAN",               \
    CTC_CLI_VLAN_RANGE_DESC,      \
    CTC_CLI_VLAN_RANGE_MASK,      \
    "Service VLAN CoS",           \
    CTC_CLI_COS_RANGE_DESC,       \
    CTC_CLI_COS_RANGE_MASK,       \
    "Service VLAN CFI",           \
    CTC_CLI_CFI_RANGE_DESC,       \
    "Layer 2 Type",               \
    CTC_CLI_PARSER_L2_TYPE_VALUE, \
    CTC_CLI_PARSER_L2_TYPE_MASK

#define CTC_CLI_ACL_PBR_KEY_L2_DESC_1 \
    "Layer 3 Type",               \
    CTC_CLI_PARSER_L3_TYPE_VALUE, \
    CTC_CLI_PARSER_L3_TYPE_MASK,  \
    "Ether type",                 \
    CTC_CLI_ETHER_TYPE_VALUE,     \
    CTC_CLI_ETHER_TYPE_MASK

#define CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_DESC               \
    "L4 protocol ",                                         \
    CTC_CLI_L4_PROTOCOL_VALUE,                              \
    CTC_CLI_L4_PROTOCOL_MASK,                               \
    "Layer4 source port",                                   \
    "Use mask.",                                            \
    "Use range. ",                                          \
    "Use mask:Src Port data. Use range: Src port min. ",    \
    CTC_CLI_L4_PORT_VALUE,                                  \
    "Use mask:Src Port mask. Use range: Src port max. ",    \
    CTC_CLI_L4_PORT_VALUE,                                  \
    "Layer4 destination port",                              \
    "Use mask.",                                            \
    "Use range. ",                                          \
    "Use mask:Dst Port data. Use range: Dst port min. ",    \
    CTC_CLI_L4_PORT_VALUE,                                  \
    "Use mask:Dst Port mask. Use range: Dst port max. ",    \
    CTC_CLI_L4_PORT_VALUE,                                  \
    "Icmp type",                                            \
    CTC_CLI_ICMP_TYPE_VALUE,                                \
    CTC_CLI_ICMP_TYPE_MASK,                                 \
    "Icmp code",                                            \
    CTC_CLI_ICMP_CODE_VALUE,                                \
    CTC_CLI_ICMP_CODE_MASK,                                 \
    "Gre CRKS flags",                                              \
    "GRE CRKS FLAGS VALUE",                                  \
    "GRE CRKS FLAGS MASK",                                   \
    "Gre key",                                              \
    CTC_CLI_GRE_KEY_VALUE,                                  \
    CTC_CLI_GRE_KEY_MASK,                                   \
    "is nvgre",                                             \
    "Tcp flags",                                            \
    "Match any tcp flag",                                   \
    "Match all tcp flags",                                  \
    CTC_CLI_TCP_FLAGS

#define CTC_CLI_ACL_KEY_IPV4_L4_DESC        \
    CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_DESC,  \
    "Igmp type",                            \
    CTC_CLI_IGMP_TYPE_VALUE,                \
    CTC_CLI_IGMP_TYPE_MASK,       \
    "Vnid",                                                      \
    CTC_CLI_VNID_VALUE,                             \
    CTC_CLI_VNID_MASK

#define CTC_CLI_ACL_KEY_IPV6_L4_DESC  \
    CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_DESC, \
    "Vnid",                                                      \
    CTC_CLI_VNID_VALUE,                             \
    CTC_CLI_VNID_MASK

/*(ipv4 ipv6) share this*/
#define CTC_CLI_ACL_KEY_L3_DESC                             \
    "Dscp",                                                 \
    CTC_CLI_DSCP_VALUE,                                     \
    CTC_CLI_DSCP_MASK,                                      \
    "Ip precedence",                                        \
    CTC_CLI_IP_PREC_VALUE,                                  \
    CTC_CLI_IP_PREC_MASK,                                   \
    "Ip fragment information",                              \
    CTC_CLI_FRAG_VALUE,                                     \
    "Ip header error",                                            \
    CTC_CLI_IP_HDR_ERROR,                                      \
    "Ip option",                                            \
    CTC_CLI_IP_OPTION,                                      \
    "Routed packet",                                        \
    CTC_CLI_ROUTED_PACKET,                                  \
    "Packet Length Range",                                  \
    "Length Min",                                           \
    "Length Max"

#define CTC_CLI_ACL_PBR_IPV6_KEY_L3_DESC                    \
    "Dscp",                                                 \
    CTC_CLI_DSCP_VALUE,                                     \
    CTC_CLI_DSCP_MASK,                                      \
    "Ip precedence",                                        \
    CTC_CLI_IP_PREC_VALUE,                                  \
    CTC_CLI_IP_PREC_MASK,                                   \
    "Ip fragment information",                              \
    CTC_CLI_FRAG_VALUE,                                     \
    "Vrfid",                                                \
    CTC_CLI_VRFID_ID_DESC,                                  \
    "<0-0x3FFF>"

#define CTC_CLI_ACL_PBR_KEY_L3_DESC                         \
    "IPv4 source address",                                  \
    CTC_CLI_IPV4_FORMAT,                                    \
    CTC_CLI_IPV4_MASK_FORMAT,                               \
    "IPv4 destination address",                             \
    CTC_CLI_IPV4_FORMAT,                                    \
    CTC_CLI_IPV4_MASK_FORMAT,                               \
    "Dscp",                                                 \
    CTC_CLI_DSCP_VALUE,                                     \
    CTC_CLI_DSCP_MASK,                                      \
    "Ip precedence",                                        \
    CTC_CLI_IP_PREC_VALUE,                                  \
    CTC_CLI_IP_PREC_MASK,                                   \
    "Ip fragment information",                              \
    CTC_CLI_FRAG_VALUE,                                     \
    "Vrfid",                                                \
    CTC_CLI_VRFID_ID_DESC,                                  \
    "<0-0x3FFF>"

/*mac unique -desc*/
#define CTC_CLI_ACL_KEY_MAC_UNIQUE_DESC     \
    "Arp operation code",           \
    CTC_CLI_ARP_OP_CODE_VALUE,      \
    CTC_CLI_ARP_OP_CODE_MASK,       \
    "IPv4 source address",          \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "IPv4 destination address",     \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Metadata",                     \
    CTC_CLI_METADATA_VALUE,                   \
    CTC_CLI_METADATA_MASK

/*ipv4 unique*/
#define CTC_CLI_ACL_KEY_IPV4_UNIQUE_DESC    \
    "Ether oam level",              \
    "<0-7>",                        \
    "<0-7>",                        \
    "Ether oam opcode",             \
    "<0-0xFF>",                     \
    "<0-0xFF>",                     \
    "IPv4 source address",          \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "IPv4 destination address",     \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Metadata",                     \
    CTC_CLI_METADATA_VALUE,                   \
    CTC_CLI_METADATA_MASK,                   \
    "Udf",                     \
    "<0-0xFFFFFFFF>",                   \
    "<0-0xFFFFFFFF>",                   \
    "Udf type",                     \
    "<0-1>, 0: L3 udf, 1: L4 udf",                   \
    "Arp packet",                   \
    CTC_CLI_ARP_PACKET_VALUE,       \
    "Arp operation code",           \
    CTC_CLI_ARP_OP_CODE_VALUE,      \
    CTC_CLI_ARP_OP_CODE_MASK,       \
    "Sender Ip address of Arp",            \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Target Ip address of Arp",            \
    CTC_CLI_IPV4_FORMAT,            \
    CTC_CLI_IPV4_MASK_FORMAT,       \
    "Ipv4 packet",                  \
    CTC_CLI_IPV4_PACKET_VALUE,      \
    "Parser mpls label number",     \
    CTC_CLI_MPLS_LABEL_NUMBER_VALUE,\
    CTC_CLI_MPLS_LABEL_NUMBER_MASK, \
    "Mpls label 0, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,     \
    "Mpls label 1, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,     \
    "Mpls label 2, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK

/*ipv6 unique*/
#define CTC_CLI_ACL_KEY_IPV6_UNIQUE_DESC    \
    "IPv6 source address",          \
    CTC_CLI_IPV6_FORMAT,            \
    "Host address",                 \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "IPv6 destination address",     \
    CTC_CLI_IPV6_FORMAT,            \
    "Host address",                 \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "Metadata",                     \
    CTC_CLI_METADATA_VALUE,                   \
    CTC_CLI_METADATA_MASK,                   \
    "Udf",                     \
    "<0-0xFFFFFFFF>",                   \
    "<0-0xFFFFFFFF>",                   \
    "Udf type",                     \
    "<0-1>, 0: L3 udf, 1: L4 udf",                   \
    "Ipv6 Extension header",        \
    CTC_CLI_IPV6_EXT_HDR_VALUE,     \
    CTC_CLI_IPV6_EXT_HDR_MASK,      \
    "Ipv6 Flow label",              \
    CTC_CLI_IPV6_FLOW_LABEL_VALUE,  \
    CTC_CLI_IPV6_FLOW_LABEL_MASK

/*ipv6 pbr unique*/
#define CTC_CLI_ACL_KEY_IPV6_PBR_UNIQUE_DESC    \
    "IPv6 source address",          \
    CTC_CLI_IPV6_FORMAT,            \
    "Host address",                 \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "IPv6 destination address",     \
    CTC_CLI_IPV6_FORMAT,            \
    "Host address",                 \
    CTC_CLI_IPV6_MASK_FORMAT,       \
    "Ipv6 Extension header",        \
    CTC_CLI_IPV6_EXT_HDR_VALUE,     \
    CTC_CLI_IPV6_EXT_HDR_MASK,      \
    "Ipv6 Flow label",              \
    CTC_CLI_IPV6_FLOW_LABEL_VALUE,  \
    CTC_CLI_IPV6_FLOW_LABEL_MASK

/*mpls unique*/
#define CTC_CLI_ACL_KEY_MPLS_UNIQUE_DESC \
    "Routed packet",             \
    CTC_CLI_ROUTED_PACKET,       \
    "Mpls label 0, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,     \
    "Mpls label 1, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,     \
    "Mpls label 2, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,     \
    "Mpls label 3, include: Label + Exp + S + TTL ", \
    CTC_CLI_MPLS_LABEL_VALUE,   \
    CTC_CLI_MPLS_LABEL_MASK,    \
    "Parser mpls label number", \
    CTC_CLI_MPLS_LABEL_NUMBER_VALUE,   \
    CTC_CLI_MPLS_LABEL_NUMBER_MASK, \
    "Metadata",                     \
    CTC_CLI_METADATA_VALUE,                   \
    CTC_CLI_METADATA_MASK

/* action */
#define CTC_CLI_ACL_ACTION_DESC             \
    "Permit",                               \
    "Deny",                                 \
    "Cancel Deny",                          \
    "Statistics",                           \
    CTC_CLI_STATS_ID_VAL,                   \
    "Deny bridge",                          \
    "Deny learning",                        \
    "Deny route",                           \
    "Force route",                          \
    "Priority",                             \
    CTC_CLI_PRIORITY_VALUE,                 \
    "Color",                                \
    CTC_CLI_COLOR_VALUE,                    \
    "Priority and color",                   \
    CTC_CLI_PRIORITY_VALUE,                 \
    CTC_CLI_COLOR_VALUE,                    \
    "Valid trust",                          \
    CTC_CLI_TRUST_VALUE,                    \
    "Valid micro flow policer",             \
    CTC_CLI_MICRO_FLOW_POLICER_ID_VALUE,    \
    "Valid macro flow policer",             \
    CTC_CLI_MACRO_FLOW_POLICER_ID_VALUE,    \
    "Random log",                           \
    CTC_CLI_SESSION_ID_VALUE,               \
    CTC_CLI_LOG_PERCENT_VALUE,              \
    "Copy to cpu",                          \
    "Disable IpFix",                        \
    "Deny IpFix Insert",                    \
    "Packet will be postcard to cpu",       \
    "Redirect to",                          \
    CTC_CLI_NH_ID_STR,                      \
    "Valid dscp",                           \
    CTC_CLI_DSCP_VALUE,                     \
    "Copy to cpu with timestamp",           \
    "Metadata",                 \
    "metadata: <0-0x3FFF>",              \
    CTC_CLI_FID_DESC,                \
    CTC_CLI_FID_ID_DESC,          \
    CTC_CLI_VRFID_DESC,     \
    CTC_CLI_VRFID_ID_DESC,     \
    "Valid qos domain",                     \
    CTC_CLI_QOS_DOMAIN_VALUE,               \
    "Valid aging",                          \
    "Valid vlan edit ",                     \
    "Stag operation type",                  \
    CTC_CLI_ACL_VLAN_TAG_OP_DESC,           \
    "Svid select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New stag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Scos select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New stag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Set svlan tpid index", "Index, the corresponding svlan tpid is in EpeL2TpidCtl",\
    "Scfi select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New stag cfi",                         \
    "0-1",                                  \
    "Ctag operation type",                  \
    CTC_CLI_ACL_VLAN_TAG_OP_DESC,           \
    "Cvid select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New ctag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Ccos select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New ctag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Ccfi select",                          \
    CTC_CLI_ACL_VLAN_TAG_SL_DESC,           \
    "New ctag cfi",                         \
    "0-1",                                  \
    "Valid packet stript",                  \
    "start to L2/L3/L4 header",             \
    "0:None, 1:L2, 2:L3, 3:L4",             \
    "payload packet type",                  \
    "0:eth, 1:ipv4, 2:mpls, 3:ipv6",        \
    "extra-length",                         \
    "unit: byte",                           \
    "Acl do redirect with raw pkt"


#define CTC_CLI_ACL_PBR_ACTION_DESC                     \
    "Permit",                               \
    "Deny",                                 \
    "Forward to nexthop",                   \
    CTC_CLI_NH_ID_STR,                      \
    "Ecmp nexthop",                         \
    "TTL check",                            \
    "ICMP redirect check",                  \
    "Copy to cpu"

#define CTC_CLI_ACL_KEY_PORT_DESC           \
    CTC_CLI_ACL_PORT_BITMAP,                \
    "<3-0>",                                \
    CTC_CLI_ACL_PORT_BITMAP_VALUE,          \
    CTC_CLI_ACL_PORT_BITMAP_VALUE,          \
    CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE,     \
    CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE,      \
    "Lchip",                                \
    CTC_CLI_LCHIP_ID_VALUE,                 \
    "Vlan class ACL",                       \
    CTC_CLI_ACL_VLAN_CLASS_ID_VALUE,        \
    "Port class ACL",                       \
    CTC_CLI_ACL_PORT_CLASS_ID_VALUE,        \
    "Pbr class ACL",                        \
    CTC_CLI_ACL_PBR_CLASS_ID_VALUE,         \
    "Logic port",                           \
    "Logic port value",                     \
    "Gport",                                \
    "Gport value"

/*******************************************\*********************
 *           acl set (code)   -begin-                           *
 ****************************************************************/

#define CTC_CLI_ACL_ADD_ENTRY_SET()                                 \
    sal_memset(acl_entry, 0, sizeof(ctc_acl_entry_t));             \
    CTC_CLI_GET_UINT32("group id", gid, argv[0]);                   \
    CTC_CLI_GET_UINT32("entry id", acl_entry->entry_id, argv[1]);    \
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");               \
    if (INDEX_VALID(index))                                         \
    {                                                               \
       acl_entry->priority_valid = 1;                                \
       CTC_CLI_GET_UINT32("entry priority", acl_entry->priority, argv[index + 1]); \
    }                                                                \



#define CTC_CLI_ACL_ADD_HASH_ENTRY_SET()                              \
    sal_memset(acl_entry, 0, sizeof(ctc_acl_entry_t));                           \
    index = CTC_CLI_GET_ARGC_INDEX("group");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
       CTC_CLI_GET_UINT32("group id", gid, argv[index + 1]);                       \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("entry");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
       CTC_CLI_GET_UINT32("entry id", acl_entry->entry_id, argv[index + 1]);        \
    }                                                                              \



#define CTC_CLI_ACL_HASH_L4_SET()                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("l4-src-port", p_key->l4_src_port, argv[index + 1]);    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("l4-dst-port", p_key->l4_dst_port, argv[index + 1]);    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");                                   \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("icmp-type", p_key->icmp_type, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");                                   \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("icmp-code", p_key->icmp_code, argv[index + 1]);         \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vni");                                  \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("vni", p_key->vni, argv[index + 1]);      \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");                                  \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("gre-key", p_key->gre_key, argv[index + 1]);      \
    }
#define CTC_CLI_ACL_HASH_IPV4_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);          \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);          \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                        \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                   \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ecn");                                         \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("ecn", p_key->ecn, argv[index + 1]);                     \
    }                                                                              \
    CTC_CLI_ACL_HASH_L4_SET()

#define CTC_CLI_ACL_HASH_MPLS_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label-num");                              \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls-label-num", p_key->mpls_label_num, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls0-label");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("mpls0-label", p_key->mpls_label0_label, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls0-s");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls0-s", p_key->mpls_label0_s, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls0-exp");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls0-exp", p_key->mpls_label0_exp, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls0-ttl");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls0-ttl", p_key->mpls_label0_ttl, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls1-label");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("mpls1-label", p_key->mpls_label1_label, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls1-s");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls1-s", p_key->mpls_label1_s, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls1-exp");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls1-exp", p_key->mpls_label1_exp, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls1-ttl");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls1-ttl", p_key->mpls_label1_ttl, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls2-label");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("mpls2-label", p_key->mpls_label2_label, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls2-s");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls2-s", p_key->mpls_label2_s, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls2-exp");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls2-exp", p_key->mpls_label2_exp, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mpls2-ttl");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("mpls2-ttl", p_key->mpls_label2_ttl, argv[index + 1]);\
    }                                                                              \


#define CTC_CLI_ACL_HASH_MAC_KEY_SET()  \
    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("field-sel-id", p_key->field_sel_id, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);                \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("is-logic-port");                               \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("is-logic-port", p_key->gport_is_logic_port, argv[index + 1]); \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("eth-type");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);          \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan");                                \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("vlan", p_key->vlan_id, argv[index + 1]);               \
        index = CTC_CLI_GET_ARGC_INDEX("to-vlan");               \
        if (INDEX_VALID(index))                                 \
        {                                                                                   \
            CTC_CLI_GET_UINT16("vlan-end", p_key->vlan_end, argv[index + 1]);          \
        }                                                                                   \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("cos");                                         \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("cos", p_key->cos, argv[index + 1]);                    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("tag-valid");                                   \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("tag-valid", p_key->tag_valid, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("cfi");                                         \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("cfi", p_key->cfi, argv[index + 1]);                    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("is-ctag");                                     \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        p_key->is_ctag  = 1;                                                       \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("vrange-grp", p_key->vrange_grpid, argv[index + 1]);                \
    }                                \

#define CTC_CLI_ACL_HASH_MPLS_KEY_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("field-sel-id", p_key->field_sel_id, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);                \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("metadata", p_key->metadata, argv[index + 1]);          \
    }                                                                              \
    CTC_CLI_ACL_HASH_MPLS_SET()

#define CTC_CLI_ACL_HASH_IPV4_KEY_SET()                               \
    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("field-sel-id", p_key->field_sel_id, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);                \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("is-logic-port");                               \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("is-logic-port", p_key->gport_is_logic_port, argv[index + 1]); \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("metadata", p_key->metadata, argv[index + 1]);          \
    }                                                                              \
    CTC_CLI_ACL_HASH_IPV4_SET()                                                      \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                     \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("l4-protocol", p_key->l4_protocol, argv[index + 1]);         \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("arp-packet");                                  \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("arp-packet", p_key->arp_packet, argv[index + 1]);      \
    }                                                                              \

#define CTC_CLI_ACL_HASH_IPV6_KEY_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");                                 \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("field-sel-id", p_key->field_sel_id, argv[index + 1]);\
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);                \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_address, argv[index + 1]);          \
        p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);                              \
        p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);                              \
        p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);                              \
        p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);                              \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_address, argv[index + 1]);          \
        p_key->ip_da[0] = sal_htonl(ipv6_address[0]);                              \
        p_key->ip_da[1] = sal_htonl(ipv6_address[1]);                              \
        p_key->ip_da[2] = sal_htonl(ipv6_address[2]);                              \
        p_key->ip_da[3] = sal_htonl(ipv6_address[3]);                              \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                        \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                   \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");                                     \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("l4-type", p_key->l4_type, argv[index + 1]);             \
    }                                                                              \
    CTC_CLI_ACL_HASH_L4_SET()


#define CTC_CLI_ACL_HASH_L2_L3_KEY_SET() \
    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");                                \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT32("field-sel-id", p_key->field_sel_id, argv[index + 1]);  \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("l3-type");                                     \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("l3-type", p_key->l3_type, argv[index + 1]);             \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("gport", p_key->gport, argv[index + 1]);                \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("metadata", p_key->metadata, argv[index + 1]);          \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                                      \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);         \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("eth-type");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);          \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan");                               \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("cvlan", p_key->ctag_vlan, argv[index + 1]);            \
        index = CTC_CLI_GET_ARGC_INDEX("to-cvlan");               \
        if (INDEX_VALID(index))                                 \
        {                                                                                   \
            CTC_CLI_GET_UINT16("cvlan-end", p_key->cvlan_end, argv[index + 1]);          \
        }                                                                                   \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("ctag-cos", p_key->ctag_cos, argv[index + 1]);           \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");                                  \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("ctag-valid", p_key->ctag_valid, argv[index + 1]);       \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);           \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan");                               \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT16("svlan", p_key->stag_vlan, argv[index + 1]);            \
        index = CTC_CLI_GET_ARGC_INDEX("to-svlan");               \
        if (INDEX_VALID(index))                                 \
        {                                                                                   \
            CTC_CLI_GET_UINT16("svlan-end", p_key->svlan_end, argv[index + 1]);          \
        }                                                                                   \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("stag-cos", p_key->stag_cos, argv[index + 1]);           \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");                                  \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("stag-valid", p_key->stag_valid, argv[index + 1]);       \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");                                    \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("stag-cfi", p_key->stag_cfi, argv[index + 1]);           \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");                                     \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("l4-type", p_key->l4_type, argv[index + 1]);             \
    }                                                                              \
    CTC_CLI_ACL_HASH_IPV4_SET()                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("ttl");                                         \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_UINT8("ttl", p_key->ttl, argv[index + 1]);                     \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("vrange-grp", p_key->vrange_grpid, argv[index + 1]);                \
    }                                \
    CTC_CLI_ACL_HASH_MPLS_SET()

/********************** Field Select *******************/
#define CTC_CLI_ACL_HASH_FIELD_PORT_SET() \
    pf->phy_port   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport") );      \
    pf->logic_port = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port") ); \
    pf->metadata   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("metadata") );   \

#define CTC_CLI_ACL_HASH_FIELD_L4_SET()                                          \
    pf->l4_src_port = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port") ); \
    pf->l4_dst_port = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-dst-port") ); \
    pf->icmp_type   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-type") );   \
    pf->icmp_code   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("icmp-code") );   \
    pf->vni   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("vni") );   \
    pf->gre_key   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("gre-key") );   \

#define CTC_CLI_ACL_HASH_FIELD_IPV4_SET() \
    pf->ip_sa = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa") );      \
    pf->ip_da = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-da") );      \
    pf->dscp  = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("dscp") );       \
    pf->ecn   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("ecn") );        \
    CTC_CLI_ACL_HASH_FIELD_L4_SET()

#define CTC_CLI_ACL_HASH_FIELD_MPLS_SET() \
    pf->mpls_label_num    = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls-label-num")); \
    pf->mpls_label0_label = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls0-label"));    \
    pf->mpls_label0_s     = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls0-s"));        \
    pf->mpls_label0_exp   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls0-exp"));      \
    pf->mpls_label0_ttl   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls0-ttl"));      \
    pf->mpls_label1_label = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls1-label"));    \
    pf->mpls_label1_s     = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls1-s"));        \
    pf->mpls_label1_exp   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls1-exp"));      \
    pf->mpls_label1_ttl   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls1-ttl"));      \
    pf->mpls_label2_label = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls2-label"));    \
    pf->mpls_label2_s     = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls2-s"));        \
    pf->mpls_label2_exp   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls2-exp"));      \
    pf->mpls_label2_ttl   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mpls2-ttl"));      \

#define CTC_CLI_ACL_HASH_FIELD_MAC_KEY_SET()  \
    CTC_CLI_ACL_HASH_FIELD_PORT_SET()         \
    pf->mac_da      = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da"));    \
    pf->mac_sa      = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa"));    \
    pf->eth_type    = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("eth-type"));  \
    pf->vlan_id     = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan"));      \
    pf->cos         = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("cos"));       \
    pf->tag_valid   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("tag-valid")); \
    pf->cfi         = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("cfi"));       \

#define CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_SET() \
    CTC_CLI_ACL_HASH_FIELD_PORT_SET()         \
    CTC_CLI_ACL_HASH_FIELD_MPLS_SET()

#define CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_SET() \
    CTC_CLI_ACL_HASH_FIELD_PORT_SET()         \
    CTC_CLI_ACL_HASH_FIELD_IPV4_SET()         \
    pf->l4_protocol = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-protocol")); \

#define CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_SET() \
    CTC_CLI_ACL_HASH_FIELD_PORT_SET() \
    pf->ip_sa  = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa") );      \
    pf->ip_da  = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-da") );      \
    pf->dscp   = INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("dscp") );       \
    pf->l4_type= INDEX_VALID( CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-type") );    \
    CTC_CLI_ACL_HASH_FIELD_L4_SET()

#define CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_SET() \
    CTC_CLI_ACL_HASH_FIELD_PORT_SET() \
    pf->mac_da     = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da"));      \
    pf->mac_sa     = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa"));      \
    pf->l3_type    = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3-type"));     \
    pf->eth_type   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("eth-type"));    \
    pf->ctag_vlan  = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan"));       \
    pf->ctag_cos   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cos"));    \
    pf->ctag_valid = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-valid"));  \
    pf->ctag_cfi   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cfi"));    \
    pf->stag_vlan  = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan"));       \
    pf->stag_cos   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cos"));    \
    pf->stag_valid = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-valid"));  \
    pf->stag_cfi   = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cfi"));    \
    pf->l4_type    = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-type"));     \
    CTC_CLI_ACL_HASH_FIELD_IPV4_SET() \
    pf->ttl        = INDEX_VALID(CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl")); \
    CTC_CLI_ACL_HASH_FIELD_MPLS_SET()

#define CTC_CLI_ACL_KEY_L2_SET_0(TYPE)                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                                           \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);                \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MAC_SA);                        \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");                                           \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);                \
        CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MAC_DA);                        \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index + 1]);                       \
        index1 = CTC_CLI_GET_ARGC_INDEX("to-cvlan");               \
        if (INDEX_VALID(index1))                                 \
        {                                                                                   \
            CTC_CLI_GET_UINT16("cvlan-end", p_key->cvlan_end, argv[index1 + 1]);          \
            index += 2;                                                                     \
        }                                                                                   \
        CTC_CLI_GET_UINT16("cvlan mask", p_key->cvlan_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_CVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cos", p_key->ctag_cos, argv[index + 1]);                  \
        CTC_CLI_GET_UINT8("ctag-cos mask", p_key->ctag_cos_mask, argv[index + 2]);        \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE## _KEY_FLAG_CTAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);                  \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_CTAG_CFI);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("svlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);                       \
        index1 = CTC_CLI_GET_ARGC_INDEX("to-svlan");               \
        if (INDEX_VALID(index1))                                 \
        {                                                                                   \
            CTC_CLI_GET_UINT16("svlan-end", p_key->svlan_end, argv[index1 + 1]);                  \
            index += 2;                                                                     \
        }                                                                                   \
        CTC_CLI_GET_UINT16("svlan mask", p_key->svlan_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_SVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cos", p_key->stag_cos, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("stag-cos mask", p_key->stag_cos_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_STAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cfi", p_key->stag_cfi, argv[index + 1]);                \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_STAG_CFI);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("vrange-grp");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("vrange-grp", p_key->vrange_grpid, argv[index + 1]);                \
    }

#define    CTC_CLI_ACL_KEY_L2_SET_0_1(TYPE)                                               \
    index = CTC_CLI_GET_ARGC_INDEX("layer2-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer2-type", p_key->l2_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer2-type mask", p_key->l2_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L2_TYPE);                       \
    }

#define    CTC_CLI_ACL_KEY_L2_SET_1(TYPE)                                               \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-num");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("vlan-num", p_key->vlan_num, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("vlan-num-mask", p_key->vlan_num_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_VLAN_NUM);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("layer3-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer3-type", p_key->l3_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer3-type-mask", p_key->l3_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L3_TYPE);                       \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("eth-type");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);               \
        CTC_CLI_GET_UINT16("eth-type-mask", p_key->eth_type_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ETH_TYPE);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-valid", p_key->stag_valid, argv[index + 1]);            \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_STAG_VALID);                    \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-valid", p_key->ctag_valid, argv[index + 1]);            \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_CTAG_VALID);                    \
    }

#define CTC_CLI_ACL_PBR_KEY_L2_SET_0(TYPE)                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");                                           \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);                \
        CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MAC_SA);                        \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");                                           \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);                \
        CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MAC_DA);                        \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("cvlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("cvlan", p_key->cvlan, argv[index + 1]);                       \
        CTC_CLI_GET_UINT16("cvlan mask", p_key->cvlan_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_CVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cos", p_key->ctag_cos, argv[index + 1]);                  \
        CTC_CLI_GET_UINT8("ctag-cos mask", p_key->ctag_cos_mask, argv[index + 2]);        \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE## _KEY_FLAG_CTAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);                  \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_CTAG_CFI);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("svlan");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("svlan", p_key->svlan, argv[index + 1]);                       \
        CTC_CLI_GET_UINT16("svlan mask", p_key->svlan_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_SVLAN);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cos", p_key->stag_cos, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("stag-cos mask", p_key->stag_cos_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_STAG_COS);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("stag-cfi", p_key->stag_cfi, argv[index + 1]);                \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_STAG_CFI);                      \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("layer2-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer2-type", p_key->l2_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer2-type mask", p_key->l2_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L2_TYPE);                       \
    }

#define    CTC_CLI_ACL_PBR_KEY_L2_SET_1(TYPE)                                               \
    index = CTC_CLI_GET_ARGC_INDEX("layer3-type");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("layer3-type", p_key->l3_type, argv[index + 1]);              \
        CTC_CLI_GET_UINT8("layer3-type-mask", p_key->l3_type_mask, argv[index + 2]);    \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L3_TYPE);                       \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("eth-type");                                         \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("eth-type", p_key->eth_type, argv[index + 1]);               \
        CTC_CLI_GET_UINT16("eth-type-mask", p_key->eth_type_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ETH_TYPE);                      \
    }

#define   CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_SET(TYPE)                                   \
        CTC_CLI_GET_UINT8("l4-protocol", p_key->l4_protocol, argv[index + 1]);           \
        CTC_CLI_GET_UINT8("l4-protocol mask", p_key->l4_protocol_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L4_PROTOCOL);                  \
        index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            if (CLI_CLI_STR_EQUAL("use-mask", index + 1))    \
            {                                                                          \
                p_key->l4_src_port_use_mask = 1;                                       \
            }                                                                          \
            CTC_CLI_GET_UINT16("src port0", p_key->l4_src_port_0, argv[index + 3]);    \
            CTC_CLI_GET_UINT16("src port1", p_key->l4_src_port_1, argv[index + 5]);    \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_L4_SRC_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");                                 \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            if (CLI_CLI_STR_EQUAL("use-mask", index + 1))    \
            {                                                                          \
                p_key->l4_dst_port_use_mask = 1;                                       \
            }                                                                          \
            CTC_CLI_GET_UINT16("dst port0", p_key->l4_dst_port_0, argv[index + 3]);    \
            CTC_CLI_GET_UINT16("dst port1", p_key->l4_dst_port_1, argv[index + 5]);    \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_L4_DST_PORT);      \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-type");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-type", p_key->icmp_type, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-type mask", p_key->icmp_type_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_ICMP_TYPE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("icmp-code");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("icmp-code", p_key->icmp_code, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("icmp-code mask", p_key->icmp_code_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_ICMP_CODE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            if (CLI_CLI_STR_EQUAL("match-any", index + 1))  \
            {                                                                          \
                p_key->tcp_flags_match_any = 1;                                        \
            }                                                                          \
            CTC_CLI_GET_UINT8("tcp-flags", p_key->tcp_flags, argv[index + 2]);         \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_TCP_FLAGS);        \
        }


#define   CTC_CLI_ACL_KEY_IPV4_L4_SET(TYPE)                                            \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                     \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_SET(TYPE)                                         \
        index = CTC_CLI_GET_ARGC_INDEX("igmp-type");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT8("igmp-type", p_key->igmp_type, argv[index + 1]);           \
            CTC_CLI_GET_UINT8("igmp-type-mask", p_key->igmp_type_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_IGMP_TYPE);        \
        }                                                                              \
        index = CTC_CLI_GET_ARGC_INDEX("vnid");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT32("vnid", p_key->vni, argv[index + 1]);           \
            CTC_CLI_GET_UINT32("vnid mask", p_key->vni_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_VNI);        \
        }                                                                              \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-crks");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT8("gre-crks", p_key->gre_crks, argv[index + 1]);              \
            CTC_CLI_GET_UINT8("gre-crks mask", p_key->gre_crks_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_GRE_CRKS);     \
        }                                                                            \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-key");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT32("gre-key", p_key->gre_key, argv[index + 1]);              \
            CTC_CLI_GET_UINT32("gre-key mask", p_key->gre_key_mask, argv[index + 2]);    \
            index = CTC_CLI_GET_ARGC_INDEX("is-nvgre");                                 \
            if (INDEX_VALID(index))                                                     \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_NVGRE_KEY);   \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_GRE_KEY);     \
            }                                                                        \
        }                                                                            \
    }

#define   CTC_CLI_ACL_KEY_IPV6_L4_SET(TYPE)                                            \
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");                                     \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_SET(TYPE)                                         \
        index = CTC_CLI_GET_ARGC_INDEX("vnid");                                   \
        if (INDEX_VALID(index))                                                        \
        {                                                                              \
            CTC_CLI_GET_UINT32("vnid", p_key->vni, argv[index + 1]);           \
            CTC_CLI_GET_UINT32("vnid mask", p_key->vni_mask, argv[index + 2]); \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_VNI);        \
        }                                                                              \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-crks");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT8("gre-crks", p_key->gre_crks, argv[index + 1]);              \
            CTC_CLI_GET_UINT8("gre-crks mask", p_key->gre_crks_mask, argv[index + 2]);    \
            SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_GRE_CRKS);     \
        }                                                                               \
        index =  CTC_CLI_GET_ARGC_INDEX("gre-key");                                     \
        if (INDEX_VALID(index))                                                        \
        {                                                                               \
            CTC_CLI_GET_UINT32("gre-key", p_key->gre_key, argv[index + 1]);              \
            CTC_CLI_GET_UINT32("gre-key mask", p_key->gre_key_mask, argv[index + 2]);    \
            index = CTC_CLI_GET_ARGC_INDEX("is-nvgre");                                 \
            if (INDEX_VALID(index))                                                     \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_NVGRE_KEY);   \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_GRE_KEY);     \
            }                                                                        \
        }                                                                            \
    }


#define   CTC_CLI_ACL_KEY_L3_SET(TYPE)                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                       \
        CTC_CLI_GET_UINT8("dscp mask", p_key->dscp_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_DSCP);                         \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("prec");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip precedence", p_key->dscp, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ip precedence mask", p_key->dscp_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_PRECEDENCE);                   \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");                                         \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-frag", p_key->ip_frag, argv[index + 1]);                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_FRAG);                      \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-hdr-error");                                         \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-hdr-error", p_key->ip_hdr_error, argv[index + 1]);                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_HDR_ERROR);                      \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-option");                                       \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-option", p_key->ip_option, argv[index + 1]);               \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_OPTION);                    \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("routed-packet");                                   \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("routed-packet", p_key->routed_packet, argv[index + 1]);       \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ROUTED_PACKET);                \
    }                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("pkt-len-range");                                   \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT32("pkt-len-min", p_key->pkt_len_min, argv[index + 1]);       \
        CTC_CLI_GET_UINT32("pkt-len-max", p_key->pkt_len_max, argv[index + 2]);       \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_PKT_LEN_RANGE);                \
    }

#define   CTC_CLI_ACL_PBR_IPV6_KEY_L3_SET(TYPE)                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                       \
        CTC_CLI_GET_UINT8("dscp mask", p_key->dscp_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_DSCP);                         \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("prec");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip precedence", p_key->dscp, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ip precedence mask", p_key->dscp_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_PRECEDENCE);                   \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");                                         \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-frag", p_key->ip_frag, argv[index + 1]);                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_FRAG);                      \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");                                           \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT16("vrfid", p_key->vrfid, argv[index + 1]);                     \
        CTC_CLI_GET_UINT16("vrfid mask", p_key->vrfid_mask, argv[index + 2]);           \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_VRFID);                        \
    }

#define   CTC_CLI_ACL_PBR_KEY_L3_SET(TYPE)                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);          \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa-mask", p_key->ip_sa_mask, argv[index + 2]);\
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_SA);                    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                       \
    if (INDEX_VALID(index))                                                        \
    {                                                                              \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);          \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da-mask", p_key->ip_da_mask, argv[index + 2]);\
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_DA);                    \
    }                                                                              \
    index = CTC_CLI_GET_ARGC_INDEX("dscp");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("dscp", p_key->dscp, argv[index + 1]);                       \
        CTC_CLI_GET_UINT8("dscp mask", p_key->dscp_mask, argv[index + 2]);             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_DSCP);                         \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("prec");                                            \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip precedence", p_key->dscp, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ip precedence mask", p_key->dscp_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_PRECEDENCE);                   \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");                                         \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT8("ip-frag", p_key->ip_frag, argv[index + 1]);                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_FRAG);                      \
    }                                                                                  \
    index = CTC_CLI_GET_ARGC_INDEX("vrfid");                                           \
    if (INDEX_VALID(index))                                                            \
    {                                                                                  \
        CTC_CLI_GET_UINT16("vrfid", p_key->vrfid, argv[index + 1]);                     \
        CTC_CLI_GET_UINT16("vrfid mask", p_key->vrfid_mask, argv[index + 2]);           \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_VRFID);                        \
    }

#define    CTC_CLI_ACL_KEY_MAC_UNIQUE_SET(TYPE)                                         \
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT16("arp-op-code", p_key->arp_op_code, argv[index + 1]);           \
        CTC_CLI_GET_UINT16("arp-op-code-mask", p_key->arp_op_code_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ARP_OP_CODE);                   \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);                 \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa-mask", p_key->ip_sa_mask, argv[index + 2]);       \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_SA);                         \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);                 \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da-mask", p_key->ip_da_mask, argv[index + 2]);       \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_DA);                         \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("metadata", p_key->metadata, argv[index + 1]);            \
        CTC_CLI_GET_UINT32("metadata-mask", p_key->metadata_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_METADATA);                    \
    }

#define    CTC_CLI_ACL_KEY_IPV4_UNIQUE_SET(TYPE)                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("layer3-type");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT8("layer3-type", p_key->l3_type, argv[index + 1]);                               \
        CTC_CLI_GET_UINT8("layer3-type-mask", p_key->l3_type_mask, argv[index + 2]);                     \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_L3_TYPE);                                        \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("ethoam-level");                                                      \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT8("ethoam-level-value", p_key->ethoam_level, argv[index + 1]);                   \
        CTC_CLI_GET_UINT8("ethoam-level-mask", p_key->ethoam_level_mask, argv[index + 2]);               \
        SET_FLAG(p_key->sub3_flag, CTC_ACL_##TYPE##_KEY_SUB3_FLAG_ETHOAM_LEVEL);                         \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("ethoam-opcode");                                                     \
    if (INDEX_VALID(index))                                                                             \
    {                                                                                                    \
        CTC_CLI_GET_UINT8("ethoam-opcode-value", p_key->ethoam_op_code, argv[index + 1]);                \
        CTC_CLI_GET_UINT8("ethoam-opcode-mask", p_key->ethoam_op_code_mask, argv[index + 2]);            \
        SET_FLAG(p_key->sub3_flag, CTC_ACL_##TYPE##_KEY_SUB3_FLAG_ETHOAM_OPCODE);                        \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                                             \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);                                \
        CTC_CLI_GET_IPV4_ADDRESS("ip-sa-mask", p_key->ip_sa_mask, argv[index + 2]);                      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_SA);                                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                                             \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);                                \
        CTC_CLI_GET_IPV4_ADDRESS("ip-da-mask", p_key->ip_da_mask, argv[index + 2]);                      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_DA);                                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("arp-packet");                                                        \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT8("arp-packet", p_key->arp_packet, argv[index + 1]);                             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ARP_PACKET);                                     \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT16("arp-op-code", p_key->arp_op_code, argv[index + 1]);                          \
        CTC_CLI_GET_UINT16("arp-op-code-mask", p_key->arp_op_code_mask, argv[index + 2]);                \
        SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_ARP_OP_CODE);                            \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("arp-sender-ip");                                                         \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_IPV4_ADDRESS("arp-sender-ip", p_key->sender_ip, argv[index + 1]);                        \
        CTC_CLI_GET_IPV4_ADDRESS("arp-sender-ip-mask", p_key->sender_ip_mask, argv[index + 2]);              \
        SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_ARP_SENDER_IP);                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("arp-target-ip");                                                         \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_IPV4_ADDRESS("arp-target-ip", p_key->target_ip, argv[index + 1]);                        \
        CTC_CLI_GET_IPV4_ADDRESS("arp-target-ip-mask", p_key->target_ip_mask, argv[index + 2]);              \
        SET_FLAG(p_key->sub_flag, CTC_ACL_##TYPE##_KEY_SUB_FLAG_ARP_TARGET_IP);                          \
    }                                                                                                    \
                                                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-packet");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT8("ipv4-packet", p_key->ipv4_packet, argv[index + 1]);                           \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IPV4_PACKET);                                    \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label-num");                                                    \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT32("mpls-label-num", p_key->mpls_label_num, argv[index + 1]);                    \
        CTC_CLI_GET_UINT32("mpls-label-num mask", p_key->mpls_label_num_mask, argv[index + 2]);          \
        SET_FLAG(p_key->sub1_flag, CTC_ACL_##TYPE##_KEY_SUB1_FLAG_MPLS_LABEL_NUM);                       \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT32("mpls-label0", p_key->mpls_label0, argv[index + 1]);                          \
        CTC_CLI_GET_UINT32("mpls-label0 mask", p_key->mpls_label0_mask, argv[index + 2]);                \
        SET_FLAG(p_key->sub1_flag, CTC_ACL_##TYPE##_KEY_SUB1_FLAG_MPLS_LABEL0);                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT32("mpls-label1", p_key->mpls_label1, argv[index + 1]);                          \
        CTC_CLI_GET_UINT32("mpls-label1 mask", p_key->mpls_label1_mask, argv[index + 2]);                \
        SET_FLAG(p_key->sub1_flag, CTC_ACL_##TYPE##_KEY_SUB1_FLAG_MPLS_LABEL1);                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");                                                       \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT32("mpls-label2", p_key->mpls_label2, argv[index + 1]);                          \
        CTC_CLI_GET_UINT32("mpls-label2 mask", p_key->mpls_label2_mask, argv[index + 2]);                \
        SET_FLAG(p_key->sub1_flag, CTC_ACL_##TYPE##_KEY_SUB1_FLAG_MPLS_LABEL2);                          \
    }                                                                                                    \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                                          \
    if (INDEX_VALID(index))                                                                              \
    {                                                                                                    \
        CTC_CLI_GET_UINT32("metadata", p_key->metadata, argv[index + 1]);                                \
        CTC_CLI_GET_UINT32("metadata-mask", p_key->metadata_mask, argv[index + 2]);                      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_METADATA);                                       \
    }                                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("udf");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("udf", udf, argv[index + 1]);            \
        CTC_CLI_GET_UINT32("udf-mask", udf_mask, argv[index + 2]);      \
        p_key->udf[0] = udf >> 24;   \
        p_key->udf[1] = (udf >> 16) & 0xFF;   \
        p_key->udf[2] = (udf >> 8) & 0xFF;   \
        p_key->udf[3] = udf  & 0xFF;   \
        p_key->udf_mask[0] = udf_mask >> 24;   \
        p_key->udf_mask[1] = (udf_mask >> 16) & 0xFF;   \
        p_key->udf_mask[2] = (udf_mask >> 8) & 0xFF;   \
        p_key->udf_mask[3] = udf_mask  & 0xFF;   \
        index = CTC_CLI_GET_ARGC_INDEX("udf-type");                                       \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("udf type", p_key->udf_type, argv[index + 1]);            \
        }                                                                                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_UDF);                    \
    }

#define    CTC_CLI_ACL_KEY_IPV6_UNIQUE_SET(TYPE)                                      \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_address, argv[index + 1]);               \
        p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);                                 \
        if CLI_CLI_STR_EQUAL("host", index + 2)                                          \
        {                                                                             \
            p_key->ip_sa_mask[0] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[1] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[2] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[3] = 0xFFFFFFFF;                                            \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            CTC_CLI_GET_IPV6_ADDRESS("ip-sa-mask", ipv6_address, argv[index + 2]);          \
            p_key->ip_sa_mask[0] = sal_htonl(ipv6_address[0]);                            \
            p_key->ip_sa_mask[1] = sal_htonl(ipv6_address[1]);                            \
            p_key->ip_sa_mask[2] = sal_htonl(ipv6_address[2]);                            \
            p_key->ip_sa_mask[3] = sal_htonl(ipv6_address[3]);                            \
        }                                                                             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_SA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_address, argv[index + 1]);               \
        p_key->ip_da[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_da[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_da[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_da[3] = sal_htonl(ipv6_address[3]);                                 \
        if CLI_CLI_STR_EQUAL("host", index + 2)                                          \
        {                                                                             \
            p_key->ip_da_mask[0] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[1] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[2] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[3] = 0xFFFFFFFF;                                            \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            CTC_CLI_GET_IPV6_ADDRESS("ip-da-mask", ipv6_address, argv[index + 2]);          \
            p_key->ip_da_mask[0] = sal_htonl(ipv6_address[0]);                            \
            p_key->ip_da_mask[1] = sal_htonl(ipv6_address[1]);                            \
            p_key->ip_da_mask[2] = sal_htonl(ipv6_address[2]);                            \
            p_key->ip_da_mask[3] = sal_htonl(ipv6_address[3]);                            \
        }                                                                             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_DA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("flow-label");                                     \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_UINT32("flow-label", p_key->flow_label, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("flow-label mask", p_key->flow_label_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_FLOW_LABEL);                  \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("metadata", p_key->metadata, argv[index + 1]);            \
        CTC_CLI_GET_UINT32("metadata-mask", p_key->metadata_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_METADATA);                    \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("udf");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("udf", udf, argv[index + 1]);            \
        CTC_CLI_GET_UINT32("udf-mask", udf_mask, argv[index + 2]);      \
        p_key->udf[0] = udf >> 24;   \
        p_key->udf[1] = (udf >> 16) & 0xFF;   \
        p_key->udf[2] = (udf >> 8) & 0xFF;   \
        p_key->udf[3] = udf  & 0xFF;   \
        p_key->udf_mask[0] = udf_mask >> 24;   \
        p_key->udf_mask[1] = (udf_mask >> 16) & 0xFF;   \
        p_key->udf_mask[2] = (udf_mask >> 8) & 0xFF;   \
        p_key->udf_mask[3] = udf_mask  & 0xFF;   \
        index = CTC_CLI_GET_ARGC_INDEX("udf-type");                                       \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("udf type", p_key->udf_type, argv[index + 1]);            \
        }                                                                                   \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_UDF);                    \
    }


#define    CTC_CLI_ACL_KEY_IPV6_PBR_SET(TYPE)                                      \
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-sa", ipv6_address, argv[index + 1]);               \
        p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);                                 \
        if CLI_CLI_STR_EQUAL("host", index + 2)                                          \
        {                                                                             \
            p_key->ip_sa_mask[0] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[1] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[2] = 0xFFFFFFFF;                                            \
            p_key->ip_sa_mask[3] = 0xFFFFFFFF;                                            \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            CTC_CLI_GET_IPV6_ADDRESS("ip-sa-mask", ipv6_address, argv[index + 2]);          \
            p_key->ip_sa_mask[0] = sal_htonl(ipv6_address[0]);                            \
            p_key->ip_sa_mask[1] = sal_htonl(ipv6_address[1]);                            \
            p_key->ip_sa_mask[2] = sal_htonl(ipv6_address[2]);                            \
            p_key->ip_sa_mask[3] = sal_htonl(ipv6_address[3]);                            \
        }                                                                             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_SA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");                                          \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_IPV6_ADDRESS("ip-da", ipv6_address, argv[index + 1]);               \
        p_key->ip_da[0] = sal_htonl(ipv6_address[0]);                                 \
        p_key->ip_da[1] = sal_htonl(ipv6_address[1]);                                 \
        p_key->ip_da[2] = sal_htonl(ipv6_address[2]);                                 \
        p_key->ip_da[3] = sal_htonl(ipv6_address[3]);                                 \
        if CLI_CLI_STR_EQUAL("host", index + 2)                                          \
        {                                                                             \
            p_key->ip_da_mask[0] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[1] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[2] = 0xFFFFFFFF;                                            \
            p_key->ip_da_mask[3] = 0xFFFFFFFF;                                            \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            CTC_CLI_GET_IPV6_ADDRESS("ip-da-mask", ipv6_address, argv[index + 2]);          \
            p_key->ip_da_mask[0] = sal_htonl(ipv6_address[0]);                            \
            p_key->ip_da_mask[1] = sal_htonl(ipv6_address[1]);                            \
            p_key->ip_da_mask[2] = sal_htonl(ipv6_address[2]);                            \
            p_key->ip_da_mask[3] = sal_htonl(ipv6_address[3]);                            \
        }                                                                             \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_IP_DA);                       \
    }                                                                                 \
    index = CTC_CLI_GET_ARGC_INDEX("flow-label");                                     \
    if (INDEX_VALID(index))                                                           \
    {                                                                                 \
        CTC_CLI_GET_UINT32("flow-label", p_key->flow_label, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("flow-label mask", p_key->flow_label_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_FLOW_LABEL);                  \
    }

#define  CTC_CLI_ACL_KEY_MPLS_UNIQUE_SET(TYPE)                                          \
    index = CTC_CLI_GET_ARGC_INDEX("routed-packet");                                    \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT8("routed-packet", p_key->routed_packet, argv[index + 1]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_ROUTED_PACKET);                 \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("mpls-label0", p_key->mpls_label0, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("mpls-label0 mask", p_key->mpls_label0_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MPLS_LABEL0);                   \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("mpls-label1", p_key->mpls_label1, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("mpls-label1 mask", p_key->mpls_label1_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MPLS_LABEL1);                   \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("mpls-label2", p_key->mpls_label2, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("mpls-label2 mask", p_key->mpls_label2_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MPLS_LABEL2);                   \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label3");                                      \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("mpls-label3", p_key->mpls_label3, argv[index + 1]);           \
        CTC_CLI_GET_UINT32("mpls-label3 mask", p_key->mpls_label3_mask, argv[index + 2]); \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MPLS_LABEL3);                     \
    }                                                                                     \
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label-number");                                  \
    if (INDEX_VALID(index))                                                              \
    {                                                                                     \
        CTC_CLI_GET_UINT32("mpls_label_num", p_key->mpls_label_num, argv[index + 1]);     \
        CTC_CLI_GET_UINT32("mpls_label_num mask", p_key->mpls_label_num_mask, argv[index + 2]);     \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_MPLS_LABEL_NUM);                        \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");                                       \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        CTC_CLI_GET_UINT32("metadata", p_key->metadata, argv[index + 1]);            \
        CTC_CLI_GET_UINT32("metadata-mask", p_key->metadata_mask, argv[index + 2]);      \
        SET_FLAG(p_key->flag, CTC_ACL_##TYPE##_KEY_FLAG_METADATA);                    \
    }


#define    CTC_CLI_ACL_ACTION_SET()                                                          \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny");                                                  \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DISCARD);                               \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("cancel-deny");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_CANCEL_DISCARD);                        \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("stats");                                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_STATS);                                 \
        CTC_CLI_GET_UINT32("stats id", p_action->stats_id, argv[index + 1]);                 \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-bridge");                                   \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DENY_BRIDGE);                           \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-learning");                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DENY_LEARNING);                         \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("deny-route");                                    \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DENY_ROUTE);                            \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("force-route");                                    \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_FORCE_ROUTE);                            \
    }                                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU);                           \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("disable-ipfix");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DISABLE_IPFIX);                           \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("deny-ipfix-learning");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DENY_IPFIX_LEARNING);                           \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("postcard-en");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_POSTCARD_EN);                           \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("to-cpu-with-timestamp");                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_COPY_TO_CPU_WITH_TIMESTAMP);             \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("aging");                                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_AGING);                                 \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("priority-and-color");                                    \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                        \
        CTC_CLI_GET_UINT8("priority", p_action->priority, argv[index + 1]);                    \
        CTC_CLI_GET_UINT8("color", p_action->color, argv[index + 2]);                          \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PRIORITY_AND_COLOR);                    \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("priority");                          \
    if (INDEX_VALID(index))                                                                 \
    {                                                                         \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PRIORITY); \
        CTC_CLI_GET_UINT8("priority_id", p_action->priority, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("color");                          \
    if (INDEX_VALID(index))                                                    \
    {                                                                         \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_COLOR); \
        CTC_CLI_GET_UINT8("color_id", p_action->color, argv[index + 1]);   \
    }                                                                         \
    index = CTC_CLI_GET_ARGC_INDEX("trust");                                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT8("trust", p_action->trust, argv[index + 1]);                          \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_TRUST);                                 \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("micro-flow-policer");                                    \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT16("micro-flow-policer", p_action->micro_policer_id, argv[index + 1]); \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);                    \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("macro-flow-policer");                                    \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT16("macro-flow-policer", p_action->macro_policer_id, argv[index + 1]); \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_MACRO_FLOW_POLICER);                    \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("random-log");                                            \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT8("log-session-id", p_action->log_session_id, argv[index + 1]);        \
        CTC_CLI_GET_UINT8("log-percent", p_action->log_percent, argv[index + 2]);              \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_RANDOM_LOG);                            \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("redirect");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT32("nexthop-id", p_action->nh_id, argv[index + 1]);                    \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_REDIRECT);                              \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("new-dscp");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT8("dscp", p_action->dscp, argv[index + 1]);                            \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_DSCP);                                  \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("meta-data");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT32("metadata", p_action->metadata, argv[index + 1]);                    \
        CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_METADATA);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX("fid");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT16("fid", p_action->fid, argv[index + 1]);                    \
        CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_FID);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX("vrf");                                     \
    if (INDEX_VALID(index))                                                                          \
    {                                                                                                \
        CTC_CLI_GET_UINT16("vrf id", p_action->vrf_id, argv[index + 1]);                    \
        CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_VRFID);                        \
    }                                                                      \
    index = CTC_CLI_GET_ARGC_INDEX("qos-domain");                                            \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT8("qos-domain", p_action->qos_domain, argv[index + 1]);                \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_QOS_DOMAIN);                            \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("strip-packet");                                            \
    if (INDEX_VALID(index))                                                             \
    {                                                                                   \
        index = CTC_CLI_GET_ARGC_INDEX("start-to");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("start-to", p_action->packet_strip.start_packet_strip, argv[index + 1]);     \
        }                                                                               \
        index = CTC_CLI_GET_ARGC_INDEX("extra-len");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("extra-len", p_action->packet_strip.strip_extra_len, argv[index + 1]);     \
        }                                                                               \
        index = CTC_CLI_GET_ARGC_INDEX("packet-type");                               \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("packet-type", p_action->packet_strip.packet_type, argv[index + 1]);     \
        }                                                                               \
    }                                                                                   \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");                                            \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("stag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT);                            \
            CTC_CLI_GET_UINT8("stag-op", p_action->vlan_edit.stag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("svid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("svid-sl", p_action->vlan_edit.svid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-svid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-svid", p_action->vlan_edit.svid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scos-sl", p_action->vlan_edit.scos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scos", p_action->vlan_edit.scos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("scfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("scfi-sl", p_action->vlan_edit.scfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-scfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-scfi", p_action->vlan_edit.scfi_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ctag-op");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_VLAN_EDIT);                            \
            CTC_CLI_GET_UINT8("ctag-op", p_action->vlan_edit.ctag_op, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("cvid-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("cvid-sl", p_action->vlan_edit.cvid_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-cvid");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT16("new-cvid", p_action->vlan_edit.cvid_new, argv[index + 1]);      \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccos-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccos-sl", p_action->vlan_edit.ccos_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccos");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccos", p_action->vlan_edit.ccos_new, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("ccfi-sl");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("ccfi-sl", p_action->vlan_edit.ccfi_sl, argv[index + 1]);         \
        }                                                                                       \
        index = CTC_CLI_GET_ARGC_INDEX("new-ccfi");                                              \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("new-ccfi", p_action->vlan_edit.ccfi_new, argv[index + 1]);         \
        }                                                                                        \
        index = CTC_CLI_GET_ARGC_INDEX("tpid-index");                                            \
        if (INDEX_VALID(index))                                                                  \
        {                                                                                       \
            CTC_CLI_GET_UINT8("tpid-index", p_action->vlan_edit.tpid_index, argv[index + 1]);   \
        }                                                                                       \
    }                                                                                            \
    index = CTC_CLI_GET_ARGC_INDEX("redirect-with-raw-pkt");                                 \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                        \
        CTC_SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_REDIRECT_WITH_RAW_PKT);                              \
    }

#define    CTC_CLI_ACL_PBR_ACTION_SET()                                                          \
    index = CTC_CLI_GET_ARGC_INDEX("permit");                                                \
    if (!INDEX_VALID(index))                                                                 \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_DENY);                               \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("forward");                                              \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        CTC_CLI_GET_UINT32("nexthop-id", p_action->nh_id, argv[index + 1]);                    \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_FWD);                              \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("ecmp");                                                 \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_ECMP);                                 \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("ttl-check");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_TTL_CHECK);                           \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("icmp-check");                                         \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_ICMP_CHECK);                         \
    }                                                                                        \
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");                                           \
    if (INDEX_VALID(index))                                                                  \
    {                                                                                        \
        SET_FLAG(p_action->flag, CTC_ACL_ACTION_FLAG_PBR_CPU);                           \
    }


#define    CTC_CLI_ACL_KEY_PORT_SET()                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("port-bitmap");                                          \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("pbmp3")))                                   \
        {                                                                                   \
            index = (CTC_CLI_GET_ARGC_INDEX("pbmp3"));                                      \
            CTC_CLI_GET_UINT32("port-bitmap 3", pbmp[3], argv[index + 1]);                  \
            CTC_CLI_GET_UINT32("port-bitmap 2", pbmp[2], argv[index + 2]);                  \
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 3]);                  \
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 4]);                  \
            cli_acl_port_bitmap_mapping(p_key->port.port_bitmap, pbmp);                     \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 1]);                  \
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 2]);                  \
            cli_acl_port_bitmap_mapping(p_key->port.port_bitmap, pbmp);                     \
        }                                                                                   \
        index = CTC_CLI_GET_ARGC_INDEX("lchip");                                            \
        if (INDEX_VALID(index))                                                             \
        {                                                                                   \
            CTC_CLI_GET_UINT8("lchip id", p_key->port.lchip, argv[index + 1]);              \
        }                                                                                   \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_PORT_BITMAP;                                \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("vlan-class");                                           \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        CTC_CLI_GET_UINT16("vlan class id", p_key->port.vlan_class_id, argv[index + 1]);    \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_VLAN_CLASS;                                 \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("port-class");                                           \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        CTC_CLI_GET_UINT16("port class id", p_key->port.port_class_id, argv[index + 1]);    \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_PORT_CLASS;                                 \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("pbr-class");                                            \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        CTC_CLI_GET_UINT8("pbr class id", p_key->port.pbr_class_id, argv[index + 1]);       \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_PBR_CLASS;                                  \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");                                           \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        CTC_CLI_GET_UINT16("logic port", p_key->port.logic_port, argv[index + 1]);          \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_LOGIC_PORT;                                 \
    }                                                                                       \
    index = CTC_CLI_GET_ARGC_INDEX("gport");                                                \
    if (INDEX_VALID(index))                                                                 \
    {                                                                                       \
        CTC_CLI_GET_UINT16("gport", p_key->port.gport, argv[index + 1]);                    \
        p_key->port.type  = CTC_FIELD_PORT_TYPE_GPORT;                                      \
    }                                                                                       \

#define CTC_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define CTC_BIT_SET(flag, bit)      ((flag) = (flag) | (1 << (bit)))
#define CTC_BIT_UNSET(flag, bit)    ((flag) = (flag) & (~(1 << (bit))))

static ctc_acl_entry_t acl_entry_com;

void
cli_acl_port_bitmap_mapping(ctc_port_bitmap_t dst, ctc_port_bitmap_t src)
{
    uint32   convt = 0;
    uint32   idx0  = 0;
    uint32   idx1  = 0;
    for(idx0 = 0; idx0 < 4; idx0++)
    {
        for(idx1 = 0; idx1 < CTC_UINT32_BITS;idx1++)
        {
            if (CTC_IS_BIT_SET(src[idx0], idx1))
            {
                convt = (idx0* CTC_UINT32_BITS + idx1);
                CTC_BIT_SET(dst[convt /CTC_UINT32_BITS], (convt %CTC_UINT32_BITS));
            }
        }
    }

    return;
}

STATIC void
cli_acl_dump_group_info(ctc_acl_group_info_t* ginfo)
{
    int8 word = 0;
    char* dir[3] = {"Ingress", "Egress", "Both-direction"};
    char* type = NULL;

    if ((ginfo->dir > CTC_BOTH_DIRECTION) || (ginfo->type >= CTC_ACL_GROUP_TYPE_MAX))
    {
        ctc_cli_out("get acl group info wrong!\n");
        return;
    }

    ctc_cli_out("  direction  : %s\n", dir[ginfo->dir]);
    ctc_cli_out("  lchip      : %d\n", ginfo->lchip);
    ctc_cli_out("  priority   : %d\n", ginfo->priority);

    switch (ginfo->type)
    {
    case CTC_ACL_GROUP_TYPE_PORT_BITMAP:
        ctc_cli_out("  port bitmap :");
        for (word = CTC_PORT_BITMAP_IN_WORD - 1; word >= 0; word--)
        {
            ctc_cli_out(" 0x%08x %s", ginfo->un.port_bitmap[word], ((!(word % 8)) && word) ? "\n              " : "");
        }
        ctc_cli_out("\n");
        type = "PORT_BITMAP";
        break;

    case CTC_ACL_GROUP_TYPE_GLOBAL:
        ctc_cli_out("  global\n");

        type = "GLOBAL";
        break;

    case CTC_ACL_GROUP_TYPE_HASH:
        ctc_cli_out("  hash\n");

        type = "HASH";
        break;

    case CTC_ACL_GROUP_TYPE_PORT_CLASS:
        ctc_cli_out("  port-class : %d \n", ginfo->un.port_class_id);

        type = "PORT_CLASS";
        break;

    case CTC_ACL_GROUP_TYPE_VLAN_CLASS:
        ctc_cli_out("  vlan-class : %d \n", ginfo->un.vlan_class_id);

        type = "VLAN_CLASS";
        break;

    case CTC_ACL_GROUP_TYPE_SERVICE_ACL:
        ctc_cli_out("  service id : %d \n", ginfo->un.service_id);

        type = "SERVICE_ACL";
        break;

    case CTC_ACL_GROUP_TYPE_PBR_CLASS:
        ctc_cli_out("  pbr-class : %d \n", ginfo->un.pbr_class_id);

        type = "PBR_CLASS";
        break;

    case CTC_ACL_GROUP_TYPE_PORT:
        ctc_cli_out("  gport : 0x%04x \n", ginfo->un.gport);

        type = "GPORT";
        break;

    case CTC_ACL_GROUP_TYPE_L3IF_CLASS:
        ctc_cli_out("  l3if-class : %d\n", ginfo->un.l3if_class_id);

        type = "L3IF_CLASS";
        break;

    case CTC_ACL_GROUP_TYPE_NONE:
        ctc_cli_out("  none\n");

        type = "NONE";
        break;

    default:
        type = "XXXX";
        break;

    }

    ctc_cli_out("  group-type : %s\n", type);

    return;

}

CTC_CLI(ctc_cli_acl_create_group,
        ctc_cli_acl_create_group_cmd,
        "acl create group GROUP_ID {priority PRIORITY |key-size SIZE|} direction (ingress|egress|both) (lchip LCHIP|) \
        (port-bitmap (pbmp3 PORT_BITMAP_3 PORT_BITMAP_2|) PORT_BITMAP_HIGH PORT_BITMAP_LOW| \
        global| vlan-class CLASS_ID | port-class CLASS_ID |service-id SERVICE_ID |pbr-class CLASS_ID| gport GPORT | none)",
        CTC_CLI_ACL_STR,
        "Create ACL group",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_S_GROUP_ID_VALUE,
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Key size",
        "Key size value",
        "Direction",
        "Ingress",
        "Egress",
        "Both direction",
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        CTC_CLI_ACL_PORT_BITMAP,
        "<3-0>",
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE,
        "Global ACL, mask ports and vlans",
        "Vlan class ACL, against a class(group) of vlan",
        CTC_CLI_ACL_VLAN_CLASS_ID_VALUE,
        "Port class ACL, against a class(group) of port",
        CTC_CLI_ACL_PORT_CLASS_ID_VALUE,
        "Service ACL, against a service",
        CTC_CLI_ACL_SERVICE_ID_VALUE,
        "Pbr class ACL, against a class(group) of l3 source interface",
        CTC_CLI_ACL_PBR_CLASS_ID_VALUE,
        "Port ACL, care gport",
        CTC_CLI_GPORT_ID_DESC,
        "None type")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 group_id = 0;
    ctc_acl_group_info_t ginfo;
    ctc_port_bitmap_t pbmp = {0};

    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("group prioirty", ginfo.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-size");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("key size", ginfo.key_size, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-bitmap");
    if (INDEX_VALID(index))
    {
        if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("pbmp3")))
        {
            index = (CTC_CLI_GET_ARGC_INDEX("pbmp3"));
            CTC_CLI_GET_UINT32("port-bitmap 3", pbmp[3], argv[index + 1]);
            CTC_CLI_GET_UINT32("port-bitmap 2", pbmp[2], argv[index + 2]);
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 3]);
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 4]);

            cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
        }
        else
        {
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 1]);
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 2]);

            cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
        }

        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_BITMAP;

    }

    index = CTC_CLI_GET_ARGC_INDEX("global");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_GLOBAL;

    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("vlan class id", ginfo.un.vlan_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_VLAN_CLASS;

    }

    index = CTC_CLI_GET_ARGC_INDEX("port-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("port class id", ginfo.un.port_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_CLASS;

    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("service id", ginfo.un.service_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_SERVICE_ACL;

    }

    index = CTC_CLI_GET_ARGC_INDEX("pbr-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("pbr class id", ginfo.un.pbr_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PBR_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", ginfo.un.gport, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_NONE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_create_group(g_api_lchip, group_id, &ginfo);
    }
    else
    {
        ret = ctc_acl_create_group(group_id, &ginfo);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_destroy_group,
        ctc_cli_acl_destroy_group_cmd,
        "acl destroy group GROUP_ID",
        CTC_CLI_ACL_STR,
        "Destroy ACL group",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_S_GROUP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 group_id = 0;

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_destroy_group(g_api_lchip, group_id);
    }
    else
    {
        ret = ctc_acl_destroy_group(group_id);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

/****************************************************************
 *           acl description    -end-                           *
 ****************************************************************/

CTC_CLI(ctc_cli_acl_add_mac_entry,
        ctc_cli_acl_add_mac_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR      \
        " mac-entry ({ "               \
        CTC_CLI_ACL_KEY_PORT_STR       \
        CTC_CLI_ACL_KEY_L2_STR_0       \
        CTC_CLI_ACL_KEY_L2_STR_1       \
        CTC_CLI_ACL_KEY_MAC_UNIQUE_STR \
        " }|)"                         \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        " Mac entry ",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_KEY_L2_DESC_0,
        CTC_CLI_ACL_KEY_L2_DESC_1,
        CTC_CLI_ACL_KEY_MAC_UNIQUE_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 index1 = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_mac_key_t* p_key = &acl_entry->key.u.mac_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_MAC;
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_KEY_L2_SET_0(MAC)
    CTC_CLI_ACL_KEY_L2_SET_0_1(MAC)
    CTC_CLI_ACL_KEY_L2_SET_1(MAC)
    CTC_CLI_ACL_KEY_MAC_UNIQUE_SET(MAC)
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_hash_mac_entry,
        ctc_cli_acl_add_hash_mac_entry_cmd,
        CTC_CLI_ACL_ADD_HASH_ENTRY_STR      \
        " hash-mac-entry "                  \
        CTC_CLI_ACL_HASH_MAC_KEY_STR        \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_HASH_ENTRY_DESC,
        " Hash mac entry ",
        CTC_CLI_ACL_HASH_MAC_KEY_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = CTC_ACL_GROUP_ID_HASH_MAC;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_hash_mac_key_t* p_key = &acl_entry->key.u.hash_mac_key;
    ctc_acl_action_t* p_action = &acl_entry->action;

    CTC_CLI_ACL_ADD_HASH_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_HASH_MAC;
    CTC_CLI_ACL_HASH_MAC_KEY_SET()
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_acl_add_ipv4_entry,
        ctc_cli_acl_add_ipv4_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR       \
        " ipv4-entry (key-size (single|double)|) ({ "\
        CTC_CLI_ACL_KEY_PORT_STR        \
        CTC_CLI_ACL_KEY_L2_STR_0        \
        CTC_CLI_ACL_KEY_L2_STR_1        \
        CTC_CLI_ACL_KEY_L3_STR          \
        CTC_CLI_ACL_KEY_IPV4_L4_STR     \
        CTC_CLI_ACL_KEY_IPV4_UNIQUE_STR \
        " }|)"                          \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        " Ipv4 entry ",
        "Key size",
        "Single width",
        "Double width",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_KEY_L2_DESC_0,
        CTC_CLI_ACL_KEY_L2_DESC_1,
        CTC_CLI_ACL_KEY_L3_DESC,
        CTC_CLI_ACL_KEY_IPV4_L4_DESC,
        CTC_CLI_ACL_KEY_IPV4_UNIQUE_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 index1 = 0xFF;
    uint32 gid = 0;
    uint32 udf = 0;
    uint32 udf_mask = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_ipv4_key_t* p_key = &acl_entry->key.u.ipv4_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_IPV4;
    index = CTC_CLI_GET_ARGC_INDEX("key-size");
    p_key->key_size = CTC_ACL_KEY_SIZE_SINGLE;
    if (INDEX_VALID(index))
    {
        if (CLI_CLI_STR_EQUAL("double", (index+1)))
        {
            p_key->key_size = CTC_ACL_KEY_SIZE_DOUBLE;
        }
        else if (CLI_CLI_STR_EQUAL("single", (index+1)))
        {
            p_key->key_size = CTC_ACL_KEY_SIZE_SINGLE;
        }
    }
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_KEY_L2_SET_0(IPV4)
    CTC_CLI_ACL_KEY_L2_SET_0_1(IPV4)
    CTC_CLI_ACL_KEY_L2_SET_1(IPV4)
    CTC_CLI_ACL_KEY_L3_SET(IPV4)
    CTC_CLI_ACL_KEY_IPV4_L4_SET(IPV4)
    CTC_CLI_ACL_KEY_IPV4_UNIQUE_SET(IPV4)
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_hash_ipv4_entry,
        ctc_cli_acl_add_hash_ipv4_entry_cmd,
        CTC_CLI_ACL_ADD_HASH_ENTRY_STR       \
        " hash-ipv4-entry "                  \
        CTC_CLI_ACL_HASH_IPV4_KEY_STR   \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_HASH_ENTRY_DESC,
        " Hash ipv4 entry ",
        CTC_CLI_ACL_HASH_IPV4_KEY_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = CTC_ACL_GROUP_ID_HASH_IPV4;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_hash_ipv4_key_t* p_key = &acl_entry->key.u.hash_ipv4_key;
    ctc_acl_action_t* p_action = &acl_entry->action;

    CTC_CLI_ACL_ADD_HASH_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_HASH_IPV4;
    CTC_CLI_ACL_HASH_IPV4_KEY_SET()
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_hash_ipv6_entry,
        ctc_cli_acl_add_hash_ipv6_entry_cmd,
        CTC_CLI_ACL_ADD_HASH_ENTRY_STR       \
        " hash-ipv6-entry "                  \
        CTC_CLI_ACL_HASH_IPV6_KEY_STR   \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_HASH_ENTRY_DESC,
        " Hash ipv6 entry ",
        CTC_CLI_ACL_HASH_IPV6_KEY_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = CTC_ACL_GROUP_ID_HASH_IPV6;
    ipv6_addr_t ipv6_address;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_hash_ipv6_key_t* p_key = &acl_entry->key.u.hash_ipv6_key;
    ctc_acl_action_t* p_action = &acl_entry->action;

    CTC_CLI_ACL_ADD_HASH_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_HASH_IPV6;
    CTC_CLI_ACL_HASH_IPV6_KEY_SET()
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_hash_mpls_entry,
        ctc_cli_acl_add_hash_mpls_entry_cmd,
        CTC_CLI_ACL_ADD_HASH_ENTRY_STR       \
        " hash-mpls-entry "                  \
        CTC_CLI_ACL_HASH_MPLS_KEY_STR   \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_HASH_ENTRY_DESC,
        " Hash mpls entry ",
        CTC_CLI_ACL_HASH_MPLS_KEY_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = CTC_ACL_GROUP_ID_HASH_MPLS;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_hash_mpls_key_t* p_key = &acl_entry->key.u.hash_mpls_key;
    ctc_acl_action_t* p_action = &acl_entry->action;

    CTC_CLI_ACL_ADD_HASH_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_HASH_MPLS;
    CTC_CLI_ACL_HASH_MPLS_KEY_SET()
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_hash_l2_l3_entry,
        ctc_cli_acl_add_hash_l2_l3_entry_cmd,
        CTC_CLI_ACL_ADD_HASH_ENTRY_STR       \
        " hash-l2-l3-entry "                  \
        CTC_CLI_ACL_HASH_L2_L3_KEY_STR   \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_HASH_ENTRY_DESC,
        " Hash l2-l3 entry ",
        CTC_CLI_ACL_HASH_L2_L3_KEY_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = CTC_ACL_GROUP_ID_HASH_L2_L3;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_hash_l2_l3_key_t* p_key = &acl_entry->key.u.hash_l2_l3_key;
    ctc_acl_action_t* p_action = &acl_entry->action;

    CTC_CLI_ACL_ADD_HASH_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_HASH_L2_L3;
    CTC_CLI_ACL_HASH_L2_L3_KEY_SET()
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_ipv6_entry,
        ctc_cli_acl_add_ipv6_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR       \
        " ipv6-entry ({"                \
        CTC_CLI_ACL_KEY_PORT_STR        \
        CTC_CLI_ACL_KEY_L2_STR_0        \
        CTC_CLI_ACL_KEY_L2_STR_1        \
        CTC_CLI_ACL_KEY_L3_STR          \
        CTC_CLI_ACL_KEY_IPV6_L4_STR     \
        CTC_CLI_ACL_KEY_IPV6_UNIQUE_STR \
        " }|)"                          \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        " Ipv6 entry ",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_KEY_L2_DESC_0,
        CTC_CLI_ACL_KEY_L2_DESC_1,
        CTC_CLI_ACL_KEY_L3_DESC,
        CTC_CLI_ACL_KEY_IPV6_L4_DESC,
        CTC_CLI_ACL_KEY_IPV6_UNIQUE_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 index1 = 0xFF;
    uint32 gid = 0;
    uint32 udf = 0;
    uint32 udf_mask = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_ipv6_key_t* p_key = &acl_entry->key.u.ipv6_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ipv6_addr_t ipv6_address;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_IPV6;
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_KEY_L2_SET_0(IPV6)
    CTC_CLI_ACL_KEY_L2_SET_0_1(IPV6)
    CTC_CLI_ACL_KEY_L2_SET_1(IPV6)
    CTC_CLI_ACL_KEY_L3_SET(IPV6)
    CTC_CLI_ACL_KEY_IPV6_L4_SET(IPV6)
    CTC_CLI_ACL_KEY_IPV6_UNIQUE_SET(IPV6)
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_mpls_entry,
        ctc_cli_acl_add_mpls_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR       \
        " mpls-entry ({"                \
        CTC_CLI_ACL_KEY_PORT_STR        \
        CTC_CLI_ACL_KEY_L2_STR_0        \
        CTC_CLI_ACL_KEY_MPLS_UNIQUE_STR \
        " }|)"                          \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        " Mpls entry ",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_KEY_L2_DESC_0,
        CTC_CLI_ACL_KEY_MPLS_UNIQUE_DESC,
        CTC_CLI_ACL_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 index1 = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_mpls_key_t* p_key = &acl_entry->key.u.mpls_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_MPLS;
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_KEY_L2_SET_0(MPLS)
    CTC_CLI_ACL_KEY_MPLS_UNIQUE_SET(MPLS)
    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_pbr_ipv4_entry,
        ctc_cli_acl_add_pbr_ipv4_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR       \
        " pbr-ipv4-entry ({ "           \
        CTC_CLI_ACL_KEY_PORT_STR        \
        CTC_CLI_ACL_PBR_KEY_L3_STR          \
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_STR     \
        " }|)"                          \
        CTC_CLI_ACL_PBR_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        "Pbr Ipv4 entry ",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_PBR_KEY_L3_DESC,
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_DESC,
        CTC_CLI_ACL_PBR_ACTION_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_pbr_ipv4_key_t* p_key = &acl_entry->key.u.pbr_ipv4_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_PBR_IPV4;
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_PBR_KEY_L3_SET(PBR_IPV4)
    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");
    if (INDEX_VALID(index))
    {
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_SET(PBR_IPV4)
    }
    CTC_CLI_ACL_PBR_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_add_pbr_ipv6_entry,
        ctc_cli_acl_add_pbr_ipv6_entry_cmd,
        CTC_CLI_ACL_ADD_ENTRY_STR       \
        " pbr-ipv6-entry ({"                \
        CTC_CLI_ACL_KEY_PORT_STR        \
        CTC_CLI_ACL_PBR_KEY_L2_STR_0        \
        CTC_CLI_ACL_PBR_KEY_L2_STR_1    \
        CTC_CLI_ACL_PBR_IPV6_KEY_L3_STR \
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_STR     \
        CTC_CLI_ACL_KEY_IPV6_PBR_UNIQUE_STR \
        " }|)"                          \
        CTC_CLI_ACL_PBR_ACTION_STR,
        CTC_CLI_ACL_ADD_ENTRY_DESC,
        "Pbr Ipv6 entry ",
        CTC_CLI_ACL_KEY_PORT_DESC,
        CTC_CLI_ACL_PBR_KEY_L2_DESC_0,
        CTC_CLI_ACL_PBR_KEY_L2_DESC_1,
        CTC_CLI_ACL_PBR_IPV6_KEY_L3_DESC,
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_DESC,
        CTC_CLI_ACL_KEY_IPV6_PBR_UNIQUE_DESC,
        CTC_CLI_ACL_PBR_ACTION_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_acl_entry_t* acl_entry = &acl_entry_com;
    ctc_acl_pbr_ipv6_key_t* p_key = &acl_entry->key.u.pbr_ipv6_key;
    ctc_acl_action_t* p_action = &acl_entry->action;
    ipv6_addr_t ipv6_address;
    ctc_port_bitmap_t pbmp = {0};

    CTC_CLI_ACL_ADD_ENTRY_SET()
    acl_entry->key.type = CTC_ACL_KEY_PBR_IPV6;
    CTC_CLI_ACL_KEY_PORT_SET()
    CTC_CLI_ACL_PBR_KEY_L2_SET_0(PBR_IPV6)

    CTC_CLI_ACL_PBR_KEY_L2_SET_1(PBR_IPV6)
    CTC_CLI_ACL_PBR_IPV6_KEY_L3_SET(PBR_IPV6)

    index = CTC_CLI_GET_ARGC_INDEX("l4-protocol");
    if (INDEX_VALID(index))
    {
        CTC_CLI_ACL_KEY_IPV4_IPV6_L4_COM_SET(PBR_IPV6)
    }
    CTC_CLI_ACL_KEY_IPV6_PBR_SET(PBR_IPV6)
    CTC_CLI_ACL_PBR_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_add_entry(g_api_lchip, gid, acl_entry);
    }
    else
    {
        ret = ctc_acl_add_entry(gid, acl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_remove_entry,
        ctc_cli_acl_remove_entry_cmd,
        "acl remove entry ENTRY_ID",
        CTC_CLI_ACL_STR,
        "Remove one entry from software table",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_remove_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_acl_remove_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_remove_all_entry,
        ctc_cli_acl_remove_all_entry_cmd,
        "acl remove-all group GROUP_ID",
        CTC_CLI_ACL_STR,
        "Remove all entries of a specified group from software table",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_B_GROUP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_remove_all_entry(g_api_lchip, gid);
    }
    else
    {
        ret = ctc_acl_remove_all_entry(gid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_set_entry_priority,
        ctc_cli_acl_set_entry_priority_cmd,
        "acl entry ENTRY_ID priority PRIORITY",
        CTC_CLI_ACL_STR,
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        CTC_CLI_ACL_ENTRY_PRIO_STR,
        CTC_CLI_ACL_ENTRY_PRIO_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;
    uint32 prio = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);
    CTC_CLI_GET_UINT32("entry priority", prio, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_entry_priority(g_api_lchip, eid, prio);
    }
    else
    {
        ret = ctc_acl_set_entry_priority(eid, prio);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_update_action,
        ctc_cli_acl_update_action_cmd,
        "acl update entry ENTRY_ID " \
        CTC_CLI_ACL_ACTION_STR,
        CTC_CLI_ACL_STR,
        "Update action",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        CTC_CLI_ACL_ACTION_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 eid = 0;
    ctc_acl_action_t   action;
    ctc_acl_action_t*  p_action = &action;

    sal_memset(&action, 0, sizeof(ctc_acl_action_t));

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    CTC_CLI_ACL_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_update_action(g_api_lchip, eid, p_action);
    }
    else
    {
        ret = ctc_acl_update_action(eid, p_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_update_pbr_action,
        ctc_cli_acl_update_pbr_action_cmd,
        "acl update pbr-entry ENTRY_ID " \
        CTC_CLI_ACL_PBR_ACTION_STR,
        CTC_CLI_ACL_STR,
        "Update pbr action",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        CTC_CLI_ACL_PBR_ACTION_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 eid = 0;
    ctc_acl_action_t   action;
    ctc_acl_action_t*  p_action = &action;

    sal_memset(&action, 0, sizeof(ctc_acl_action_t));

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    CTC_CLI_ACL_PBR_ACTION_SET()

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_update_action(g_api_lchip, eid, p_action);
    }
    else
    {
        ret = ctc_acl_update_action(eid, p_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_copy_entry,
        ctc_cli_acl_copy_entry_cmd,
        "acl entry ENTRY_ID copy-to group GROUP_ID entry ENTRY_ID ",
        CTC_CLI_ACL_STR,
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        "Copy to",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_B_GROUP_ID_VALUE,
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    ctc_acl_copy_entry_t copy_entry;

    sal_memset(&copy_entry, 0, sizeof(copy_entry));

    CTC_CLI_GET_UINT32("source entry id", copy_entry.src_entry_id, argv[0]);
    CTC_CLI_GET_UINT32("dest group id", copy_entry.dst_group_id, argv[1]);
    CTC_CLI_GET_UINT32("dest entry id", copy_entry.dst_entry_id, argv[2]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_copy_entry(g_api_lchip, &copy_entry);
    }
    else
    {
        ret = ctc_acl_copy_entry(&copy_entry);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_install_group,
        ctc_cli_acl_install_group_cmd,
        "acl install group GROUP_ID (direction (ingress|egress|both) (lchip LCHIP|) \
        (port-bitmap (pbmp3 PORT_BITMAP_3 PORT_BITMAP_2|) PORT_BITMAP_HIGH PORT_BITMAP_LOW| \
        global| vlan-class CLASS_ID | port-class CLASS_ID |service-id SERVICE_ID| \
        pbr-class CLASS_ID | gport GPORT | none)|)",
        CTC_CLI_ACL_STR,
        "Install to hardware",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_B_GROUP_ID_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Both direction",
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        CTC_CLI_ACL_PORT_BITMAP,
        "<3-0>",
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE,
        "Global ACL, mask ports and vlans",
        "Vlan class ACL, against a class(group) of vlan",
        CTC_CLI_ACL_VLAN_CLASS_ID_VALUE,
        "Port class ACL, against a class(group) of port",
        CTC_CLI_ACL_PORT_CLASS_ID_VALUE,
        "Service ACL, against a service",
        CTC_CLI_ACL_SERVICE_ID_VALUE,
        "Pbr class ACL, against a class(group) of l3 source interface",
        CTC_CLI_ACL_PBR_CLASS_ID_VALUE,
        "Port ACL, care gport",
        CTC_CLI_GPORT_ID_DESC,
        "None type")
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_acl_group_info_t ginfo;
    ctc_acl_group_info_t* pinfo = NULL;
    ctc_port_bitmap_t pbmp = {0};

    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if (argc > 1)
    {
        pinfo = &ginfo;

        index = CTC_CLI_GET_ARGC_INDEX("ingress");
        if (INDEX_VALID(index))
        {
            ginfo.dir = CTC_INGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("egress");
        if (INDEX_VALID(index))
        {
            ginfo.dir = CTC_EGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("both");
        if (INDEX_VALID(index))
        {
            ginfo.dir = CTC_BOTH_DIRECTION;
        }

        index = CTC_CLI_GET_ARGC_INDEX("lchip");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("port-bitmap");
        if (INDEX_VALID(index))
        {
            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("pbmp3")))
            {
                index = (CTC_CLI_GET_ARGC_INDEX("pbmp3"));
                CTC_CLI_GET_UINT32("port-bitmap 3", pbmp[3], argv[index + 1]);
                CTC_CLI_GET_UINT32("port-bitmap 2", pbmp[2], argv[index + 2]);
                CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 3]);
                CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 4]);

                cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
            }
            else
            {
                CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 1]);
                CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 2]);

                cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
            }

            ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_BITMAP;
        }

        index = CTC_CLI_GET_ARGC_INDEX("global");
        if (INDEX_VALID(index))
        {
            ginfo.type  = CTC_ACL_GROUP_TYPE_GLOBAL;

        }

        index = CTC_CLI_GET_ARGC_INDEX("vlan-class");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT16("vlan class id", ginfo.un.vlan_class_id, argv[index + 1]);
            ginfo.type  = CTC_ACL_GROUP_TYPE_VLAN_CLASS;

        }

        index = CTC_CLI_GET_ARGC_INDEX("port-class");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT16("port class id", ginfo.un.port_class_id, argv[index + 1]);
            ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_CLASS;

        }

        index = CTC_CLI_GET_ARGC_INDEX("service-id");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT16("service id", ginfo.un.service_id, argv[index + 1]);
            ginfo.type  = CTC_ACL_GROUP_TYPE_SERVICE_ACL;

        }

        index = CTC_CLI_GET_ARGC_INDEX("pbr-class");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT16("pbr class id", ginfo.un.pbr_class_id, argv[index + 1]);
            ginfo.type  = CTC_ACL_GROUP_TYPE_PBR_CLASS;

        }

        index = CTC_CLI_GET_ARGC_INDEX("gport");
        if (INDEX_VALID(index))
        {
            CTC_CLI_GET_UINT16("gport", ginfo.un.gport, argv[index + 1]);
            ginfo.type  = CTC_ACL_GROUP_TYPE_PORT;
        }

        index = CTC_CLI_GET_ARGC_INDEX("none");
        if (INDEX_VALID(index))
        {
            ginfo.type  = CTC_ACL_GROUP_TYPE_NONE;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_install_group(g_api_lchip, gid, pinfo);
    }
    else
    {
        ret = ctc_acl_install_group(gid, pinfo);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_uninstall_group,
        ctc_cli_acl_uninstall_group_cmd,
        "acl uninstall group GROUP_ID",
        CTC_CLI_ACL_STR,
        "Uninstall from hardware",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_B_GROUP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_uninstall_group(g_api_lchip, gid);
    }
    else
    {
        ret = ctc_acl_uninstall_group(gid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_install_entry,
        ctc_cli_acl_install_entry_cmd,
        "acl install entry ENTRY_ID",
        CTC_CLI_ACL_STR,
        "Install to hardware",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_install_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_acl_install_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_uninstall_entry,
        ctc_cli_acl_uninstall_entry_cmd,
        "acl uninstall entry ENTRY_ID",
        CTC_CLI_ACL_STR,
        "Uninstall from hardware",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint32 eid = 0;

    CTC_CLI_GET_UINT32("entry id", eid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_uninstall_entry(g_api_lchip, eid);
    }
    else
    {
        ret = ctc_acl_uninstall_entry(eid);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_set_hash_mac_field_sel,
        ctc_cli_acl_set_hash_mac_field_sel_cmd,
        "acl set hash-key-type mac field-sel-id SEL_ID" \
        CTC_CLI_ACL_HASH_FIELD_MAC_KEY_STR,
        CTC_CLI_ACL_STR,
        "Set",
        "hash Key type",
        "MAC Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_ACL_HASH_FIELD_MAC_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_acl_hash_field_sel_t sel;
    ctc_acl_hash_mac_field_sel_t* pf = &sel.u.mac;

    sal_memset(&sel, 0, sizeof(sel));

    sel.hash_key_type = CTC_ACL_KEY_HASH_MAC;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_ACL_HASH_FIELD_MAC_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_acl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_set_hash_ipv4_field_sel,
        ctc_cli_acl_set_hash_ipv4_field_sel_cmd,
        "acl set hash-key-type ipv4 field-sel-id SEL_ID" \
        CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_STR,
        CTC_CLI_ACL_STR,
        "Set",
        "hash Key type",
        "IPV4 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_acl_hash_field_sel_t sel;
    ctc_acl_hash_ipv4_field_sel_t* pf = &sel.u.ipv4;

    sal_memset(&sel, 0, sizeof(sel));

    sel.hash_key_type = CTC_ACL_KEY_HASH_IPV4;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_ACL_HASH_FIELD_IPV4_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_acl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_acl_set_hash_ipv6_field_sel,
        ctc_cli_acl_set_hash_ipv6_field_sel_cmd,
        "acl set hash-key-type ipv6 field-sel-id SEL_ID" \
        CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_STR,
        CTC_CLI_ACL_STR,
        "Set",
        "hash Key type",
        "IPV6 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_acl_hash_field_sel_t sel;
    ctc_acl_hash_ipv6_field_sel_t* pf = &sel.u.ipv6;

    sal_memset(&sel, 0, sizeof(sel));

    sel.hash_key_type = CTC_ACL_KEY_HASH_IPV6;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_ACL_HASH_FIELD_IPV6_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_acl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_set_hash_mpls_field_sel,
        ctc_cli_acl_set_hash_mpls_field_sel_cmd,
        "acl set hash-key-type mpls field-sel-id SEL_ID" \
        CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_STR,
        CTC_CLI_ACL_STR,
        "Set",
        "hash Key type",
        "MPLS Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_acl_hash_field_sel_t sel;
    ctc_acl_hash_mpls_field_sel_t* pf = &sel.u.mpls;

    sal_memset(&sel, 0, sizeof(sel));

    sel.hash_key_type = CTC_ACL_KEY_HASH_MPLS;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_ACL_HASH_FIELD_MPLS_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_acl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_acl_set_hash_l2_l3_field_sel,
        ctc_cli_acl_set_hash_l2_l3_field_sel_cmd,
        "acl set hash-key-type l2-l3 field-sel-id SEL_ID" \
        CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_STR,
        CTC_CLI_ACL_STR,
        "Set",
        "hash Key type",
        "L2_L3 Hash Key",
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_acl_hash_field_sel_t sel;
    ctc_acl_hash_l2_l3_field_sel_t* pf = &sel.u.l2_l3;

    sal_memset(&sel, 0, sizeof(sel));

    sel.hash_key_type = CTC_ACL_KEY_HASH_L2_L3;

    CTC_CLI_GET_UINT32("field-sel-id", sel.field_sel_id, argv[0]);

    CTC_CLI_ACL_HASH_FIELD_L2_L3_KEY_SET();

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_hash_field_sel(g_api_lchip, &sel);
    }
    else
    {
        ret = ctc_acl_set_hash_field_sel(&sel);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



CTC_CLI(ctc_cli_acl_show_group_info,
        ctc_cli_acl_show_group_info_cmd,
        "show acl group-info group GROUP_ID ((size ENTRY_SIZE | entry START (END|)) |)",
        "Show",
        CTC_CLI_ACL_STR,
        "Group info",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_S_GROUP_ID_VALUE,
        "Entry size",
        "0: show entry count in this group, other: show spcecified count of entry",
        "Show Entry",
        "Start entry id",
        "End entry id")
{
    int32 ret = CLI_SUCCESS;
    uint32 gid = 0;
    uint8 size_index = 0xFF;
    uint8 entry_index = 0xFF;
    ctc_acl_group_info_t ginfo;
    uint32 idx = 0;
    uint32 start_entry_id = 0;
    uint32 end_entry_id = 0xffffffff;
    uint32* p_array = NULL;
    ctc_acl_query_t query;
    char str[35] = {0};
    char format[10] = {0};

    sal_memset(&query, 0, sizeof(ctc_acl_query_t));
    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_get_group_info(g_api_lchip, gid, &ginfo);
    }
    else
    {
        ret = ctc_acl_get_group_info(gid, &ginfo);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        cli_acl_dump_group_info(&ginfo);
    }

    query.group_id = gid;
    size_index = CTC_CLI_GET_ARGC_INDEX("size");
    if (INDEX_VALID(size_index))
    {
        CTC_CLI_GET_UINT32("entry size", query.entry_size, argv[size_index + 1]);
    }

    entry_index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (INDEX_VALID(entry_index))
    {
        CTC_CLI_GET_UINT32("start entry id", start_entry_id, argv[entry_index + 1]);
        if((entry_index + 2) < argc)
        {
            CTC_CLI_GET_UINT32("end entry id", end_entry_id, argv[entry_index + 2]);
        }
    }

    if ((0xFF != size_index) || (0xFF != entry_index))
    {
        p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * (query.entry_size));
        query.entry_array = p_array;
        if(g_ctcs_api_en)
        {
             ret = ctcs_acl_get_multi_entry(g_api_lchip, &query);
        }
        else
        {
            ret = ctc_acl_get_multi_entry(&query);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            mem_free(p_array);
            return CLI_ERROR;
        }

        ctc_cli_out("  entry count: %u\n", query.entry_count);
        ctc_cli_out("  -----------------------\n");
        ctc_cli_out("  %-11s%s\n", "No.", "ENTRY_ID");
        ctc_cli_out("  -----------------------\n");

        if (query.entry_size)
        {
            for (idx = 0; idx < query.entry_count; idx++)
            {
                ctc_cli_out("  %-11u%s\n", idx, CTC_DEBUG_HEX_FORMAT(str, format, query.entry_array[idx], 8, U));
            }
            mem_free(p_array);
            return ret;
        }

        if (start_entry_id <= end_entry_id)
        {
            mem_free(p_array);
            query.group_id = gid;
            query.entry_size  = query.entry_count;
            p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * (query.entry_size));
            query.entry_array = p_array;

            if(g_ctcs_api_en)
            {
                 ret = ctcs_acl_get_multi_entry(g_api_lchip, &query);
            }
            else
            {
                ret = ctc_acl_get_multi_entry(&query);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                mem_free(p_array);
                return CLI_ERROR;
            }
        }
        else
        {
            ctc_cli_out("start_entry_id should less than end_entry_id\n");
            mem_free(p_array);
            return CLI_ERROR;
        }

        for (idx = 0; idx < query.entry_count; idx++)
        {
            if (((query.entry_array[idx]) >= start_entry_id)
                && ((query.entry_array[idx]) <= end_entry_id))
            {
                ctc_cli_out("  %-11u%s\n", idx, CTC_DEBUG_HEX_FORMAT(str, format, query.entry_array[idx], 8, U));
                continue;
            }
        }
        mem_free(p_array);
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_show_entry_info,
        ctc_cli_acl_show_entry_info_cmd,
        "show acl entry-info lookup-level VALUE ((size ENTRY_SIZE | entry START (END|))) (ingress | egress)",
        "Show",
        CTC_CLI_ACL_STR,
        "Entry info",
        "By lookup level",
        "Lookup level",
        "Entry size",
        "0: show entry count in this lookup level, other: show spcecified count of entry",
        "Show Entry",
        "Start entry id",
        "End entry id",
        "Ingress",
        "Egress")
{
    int32 ret = CLI_SUCCESS;
    uint32 level = 0;
    uint8 size_index = 0xFF;
    uint8 entry_index = 0xFF;
    uint32 idx = 0;
    uint8 index = 0xFF;
    uint8 dir = CTC_INGRESS;
    uint32 start_entry_id = 0;
    uint32 end_entry_id = 0xffffffff;
    uint32* p_array = NULL;
    ctc_acl_query_t query;
    char str[35] = {0};
    char format[10] = {0};

    sal_memset(&query, 0, sizeof(ctc_acl_query_t));

    CTC_CLI_GET_UINT32("lookup-level", level, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }

    query.query_mode = CTC_ACL_QUERY_MODE_LKUP_LEVEL;
    query.dir = dir;
    query.lkup_level = level;
    size_index = CTC_CLI_GET_ARGC_INDEX("size");
    if (INDEX_VALID(size_index))
    {
        CTC_CLI_GET_UINT32("entry size", query.entry_size, argv[size_index + 1]);
    }

    entry_index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (INDEX_VALID(entry_index))
    {
        CTC_CLI_GET_UINT32("start entry id", start_entry_id, argv[entry_index + 1]);
        if((entry_index + 2) < argc)
        {
            CTC_CLI_GET_UINT32("end entry id", end_entry_id, argv[entry_index + 2]);
        }
    }

    if ((0xFF != size_index) || (0xFF != entry_index))
    {
        p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * (query.entry_size));
        query.entry_array = p_array;
        if(g_ctcs_api_en)
        {
             ret = ctcs_acl_get_multi_entry(g_api_lchip, &query);
        }
        else
        {
            ret = ctc_acl_get_multi_entry(&query);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            mem_free(p_array);
            return CLI_ERROR;
        }

        ctc_cli_out("  entry count: %u\n", query.entry_count);
        ctc_cli_out("  -----------------------\n");
        ctc_cli_out("  %-11s%s\n", "No.", "ENTRY_ID");
        ctc_cli_out("  -----------------------\n");

        if (query.entry_size)
        {
            for (idx = 0; idx < query.entry_count; idx++)
            {
                ctc_cli_out("  %-11u%s\n", idx, CTC_DEBUG_HEX_FORMAT(str, format, query.entry_array[idx], 8, U));
            }
            mem_free(p_array);
            return ret;
        }

        if (start_entry_id <= end_entry_id)
        {
            mem_free(p_array);
            query.entry_size  = query.entry_count;
            p_array = mem_malloc(MEM_CLI_MODULE, sizeof(uint32) * (query.entry_size));
            query.entry_array = p_array;

            if(g_ctcs_api_en)
            {
                 ret = ctcs_acl_get_multi_entry(g_api_lchip, &query);
            }
            else
            {
                ret = ctc_acl_get_multi_entry(&query);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                mem_free(p_array);
                return CLI_ERROR;
            }
        }
        else
        {
            ctc_cli_out("start_entry_id should less than end_entry_id\n");
            mem_free(p_array);
            return CLI_ERROR;
        }

        for (idx = 0; idx < query.entry_count; idx++)
        {
            if (((query.entry_array[idx]) >= start_entry_id)
                && ((query.entry_array[idx]) <= end_entry_id))
            {
                ctc_cli_out("  %-11u%s\n", idx, CTC_DEBUG_HEX_FORMAT(str, format, query.entry_array[idx], 8, U));
                continue;
            }
        }
        mem_free(p_array);
    }

    return ret;
}

CTC_CLI(ctc_cli_acl_debug_on,
        ctc_cli_acl_debug_on_cmd,
        "debug acl (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_ACL_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

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
        ctc_debug_set_flag("acl", "acl", ACL_CTC, level, TRUE);

    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        ctc_debug_set_flag("acl", "acl", ACL_SYS, level, TRUE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_acl_debug_off,
        ctc_cli_acl_debug_off_cmd,
        "no debug acl (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_ACL_STR,
        "Ctc layer",
        "Sys layer",
        "ACL Label",
        "ACL entry")
{
    uint8 level = CTC_DEBUG_LEVEL_INFO;

    if (CLI_CLI_STR_EQUAL("ctc", 0))
    {
        ctc_debug_set_flag("acl", "acl", ACL_CTC, level, FALSE);

    }
    else if (CLI_CLI_STR_EQUAL("sys", 0))
    {
        ctc_debug_set_flag("acl", "acl", ACL_SYS, level, FALSE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_acl_debug_show,
        ctc_cli_acl_debug_show_cmd,
        "show debug acl",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_ACL_STR)
{
    uint8 level = CTC_DEBUG_LEVEL_NONE;
    uint8 en   = 0;

    en = ctc_debug_get_flag("acl", "acl", ACL_CTC, &level);

    ctc_cli_out(" acl %s debug %s level:%s\n", "CTC", en ? "on" : "off", ctc_cli_get_debug_desc(level));

    en = ctc_debug_get_flag("acl", "acl", ACL_SYS, &level);
    ctc_cli_out(" acl %s debug %s level:%s\n", "SYS", en ? "on" : "off", ctc_cli_get_debug_desc(level));

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_acl_set_league_mode,
        ctc_cli_acl_set_league_mode_cmd,
        "acl priority PRIORITY league-mode VALUE (expand-ext-tcam BITMAP|)(ingress | egress)(auto-move-en |) ",
        CTC_CLI_ACL_STR,
        "ACL priority",
        "ACL priority value",
        "League mode",
        "League mode value, lookup level bitmap in binary mode like 11100000 means merge level 0-2 from left",
        "expand-ext-tcam",
        "Bitmap value",
        "Ingress",
        "Egress",
        "Auto move entry enable")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint8 dir = CTC_INGRESS;
    uint8 prio = 0;
    uint16 bmp = 0;
    int32 i = 0;
    uint32 len = 0;
    char* str_bmp = NULL;
    ctc_acl_league_t acl_league;

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }


    CTC_CLI_GET_UINT8("acl-priority", prio, argv[0]);

    str_bmp = argv[1];
    len = sal_strlen(str_bmp);
    for (i=0; i<len; i++)
    {
        if ((str_bmp[i] != '0') && (str_bmp[i] != '1'))
        {
            ctc_cli_out("%% Failed: Invalid league-mode string \n");
            return CLI_ERROR;
        }
        if (str_bmp[i] == '1')
        {
            bmp |= (1 << i);
        }
    }

    sal_memset(&acl_league, 0, sizeof(ctc_acl_league_t));

    acl_league.lkup_level_bitmap = bmp;
    str_bmp = NULL;
    bmp = 0;

    index = CTC_CLI_GET_ARGC_INDEX("expand-ext-tcam");
    if (index != 0xFF)
    {
        str_bmp = argv[index+1];
        len = sal_strlen(str_bmp);
        for (i=0; i<len; i++)
        {
            if ((str_bmp[i] != '0') && (str_bmp[i] != '1'))
            {
                ctc_cli_out("%% Failed: Invalid league-mode string \n");
                return CLI_ERROR;
            }
            if (str_bmp[i] == '1')
            {
                bmp |= (1 << i);
            }
        }
        acl_league.ext_tcam_bitmap = bmp;
    }

    index = CTC_CLI_GET_ARGC_INDEX("auto-move-en");
    if (index != 0xFF)
    {
        acl_league.auto_move_en = 1;
    }
    acl_league.acl_priority = prio;
    acl_league.dir = dir;

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_set_league_mode(g_api_lchip, &acl_league);
    }
    else
    {
        ret = ctc_acl_set_league_mode(&acl_league);
    }

    if (ret)
    {
        ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_acl_entry_reorder,
        ctc_cli_acl_entry_reorder_cmd,
        "acl priority PRIORITY entry reorder (scatter | down-to-up | up-to-down) (ingress | egress) ",
        CTC_CLI_ACL_STR,
        "ACL priority",
        "ACL priority value",
        "ACL entry",
        "Reorder",
        "Scatter",
        "Down to up",
        "Up to down")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint8 dir = CTC_INGRESS;
    uint8 prio = 0;
    uint8 mode = 0;
    ctc_acl_reorder_t reorder;

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        dir = CTC_EGRESS;
    }

    CTC_CLI_GET_UINT8("acl-priority", prio, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("scatter");
    if (index != 0xFF)
    {
        mode = CTC_ACL_REORDER_MODE_SCATTER;
    }
    index = CTC_CLI_GET_ARGC_INDEX("down-to-up");
    if (index != 0xFF)
    {
        mode = CTC_ACL_REORDER_MODE_DOWNTOUP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("up-to-down");
    if (index != 0xFF)
    {
        mode = CTC_ACL_REORDER_MODE_UPTODOWN;
    }

    sal_memset(&reorder, 0, sizeof(ctc_acl_reorder_t));
    reorder.acl_priority = prio;
    reorder.dir = dir;
    reorder.mode = mode;

    if(g_ctcs_api_en)
    {
        ret = ctcs_acl_reorder_entry(g_api_lchip, &reorder);
    }
    else
    {
        ret = ctc_acl_reorder_entry(&reorder);
    }

    if (ret)
    {
        ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_acl_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_acl_create_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_destroy_group_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_mac_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_ipv4_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_ipv6_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_mpls_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_pbr_ipv4_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_pbr_ipv6_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_hash_mac_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_hash_ipv4_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_hash_mpls_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_hash_ipv6_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_add_hash_l2_l3_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_remove_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_remove_all_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_entry_priority_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_update_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_update_pbr_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_copy_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_install_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_uninstall_group_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_install_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_uninstall_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_hash_mac_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_hash_ipv4_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_hash_mpls_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_hash_ipv6_field_sel_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_hash_l2_l3_field_sel_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_show_group_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_show_entry_info_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_debug_show_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_acl_set_league_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_acl_entry_reorder_cmd);
 /*  install_element(CTC_SDK_MODE, &ctc_cli_acl_test_sort_performance_cmd);*/
 /*  install_element(CTC_SDK_MODE, &ctc_cli_acl_test_sort_performance2_cmd);*/


    return 0;
}

