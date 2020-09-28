#ifndef __GLB_ACL_DEFINE_H__
#define __GLB_ACL_DEFINE_H__

#include "glb_flow_define.h"
#include "glb_cpu_traffic_define.h"

#define PMAP_NAME_SIZE      20
#define CMAP_NAME_SIZE      20
#define ACL_NAME_SIZE       20
#define ACL_REMARK_SIZE     100
#define ACL_NAME_PREFIX_SIZE       10
#define ACL_NAME_FULL_NAME_SIZE       ACL_NAME_PREFIX_SIZE + ACL_NAME_SIZE
#define ACL_MAX_ACE_SEQUENCE_NUMBER 65535
#define ACL_MAX_FLOW_MATCH_ACE_SEQUENCE_NUMBER 63
#define ACL_CLASS_MAP_CONFIG_NUMBER 600
#define ACL_POLICY_MAP_CONFIG_NUMBER 200

#define MAX_MEMBER          64

#define GLB_IGS_ACL_TBLID          1
#define GLB_EGS_ACL_TBLID          2
#define GLB_IGS_OPENFLOW_TBLID     3
#define GLB_EGS_OPENFLOW_TBLID     4
#define GLB_COPP_TBLID             5
#define GLB_SNAT_TBLID             6


#define GLB_ACL_DEFUALT_MAC_ACL_SEQ   1

#define GLB_ACL_LOG_BIT_LEN         20/*modify by yangl for bug 45988*/
#define GLB_ACL_LOG_UML_BIT_LEN         15
#define GLB_ACL_LOG_GB_BIT_LEN         15
#define GLB_MAX_MSG_ACE_NUM         10

#define GLB_ACL_MAX_MSG_INTF_NUM    20

/*moidfy by chenxw for acl&qos resource mgr, 2013-09-12.*/
#define GLB_ACL_TCAM_BLOCK     1
#define GLB_ACL_MULTICAST_TCAM_BLOCK 1
#define GLB_ACL_MAC_KEY_MERGE_IPV6  1
#define GLB_ACL_MAC_KEY_MERGE_MPLS  2
#define GLB_ACL_MAC_KEY_MERGE_IPV4   3
#define GLB_ACL_ENTRY_RULE_PRIORITY_MAX  0xFFFFFFFF
#define GLB_ACL_TCAM_BLOCK_ENABLE  0xF
#define GLB_ACL_PROCESSING_IPV6    0x2
#define GLB_ACL_PROCESSING_MPLS   0x4
#define GLB_ACL_SMALL_FRAGMENT_OFFSET 3
#define GLB_ACL_MULTICAST_ACL_GROUP 1023

#define GLB_ACL_CONFIG_MAX          4096
#define GLB_ACE_CONFIG_MAX          8192
#define GLB_ACE_ENTRY_MAX           4096
#define GLB_ACL_L4PORT_ACE_NUM_MAX     7
#define GLB_ACL_TCP_FLAGS_ACE_NUM_MAX  4

#define GLB_SAI_ACL_MAX             128
#define GLB_SAI_ACE_MAX             10000

#define GLB_ACL_UDF_GROUP_L3_HEAD      1
#define GLB_ACL_UDF_GROUP_L4_HEAD      2
#define GLB_ACL_UDF_GROUP_NUM_MAX      3 //sdk bug 3 entry del is ok 4 entry del erro
#define GLB_ACL_UDF_VERSION_V4         1
#define GLB_ACL_UDF_VERSION_V6         2

#define GLB_ACL_UDF_FIELD_NUM_MAX      4

#define GLB_ACE_ERSPAN_NUM_MAX         246

#ifdef GOLDENGATE
#define GLB_ACE_INNER_MATCH_NUM_MAX         249
#endif
#ifdef USW
#define GLB_ACE_INNER_MATCH_NUM_MAX         253
#endif

#define GLB_ACL_UDF_OFFSET_L2          0
#define GLB_ACL_UDF_OFFSET_L3          1
#define GLB_ACL_UDF_OFFSET_L4          2

#define GLB_ACL_UDF_GROUP_UDF_ID       0xFF  /* indicate the acl udf use udf-id */
#ifdef GOLDENGATE
#define GLB_ACL_UDF_GROUP_LENGTH       1
#define GLB_ACL_UDF_ENTRY_NUM_MAX      8     /* Goldengate 4 l3_udf + 4 l4_udf */
#else
#define GLB_ACL_UDF_GROUP_LENGTH       4
#define GLB_ACL_UDF_ENTRY_NUM_MAX      16
#endif

#define GLB_ACL_UDF_OFFSET_MAX         63
#define GLB_ACL_UDF_MATCH_VLANNUM_MAX  2
#define GLB_ACL_UDF_MATCH_MPLSNUM_MAX  9

#define GLB_ACL_FIELD_ETHER_TYPE_MIN   0x600

/* refer to ctc_parser_l4_type_e */
#define GLB_ACL_UDF_L4_TYPE_NONE       0
#define GLB_ACL_UDF_L4_TYPE_TCP        1
#define GLB_ACL_UDF_L4_TYPE_UDP        2
#define GLB_ACL_UDF_L4_TYPE_GRE        3
#define GLB_ACL_UDF_L4_TYPE_ICMP       6
#define GLB_ACL_UDF_L4_TYPE_IGMP       7
#define GLB_ACL_UDF_L4_TYPE_RDP        12
#define GLB_ACL_UDF_L4_TYPE_SCTP       13
#define GLB_ACL_UDF_L4_TYPE_DCCP       14
#define GLB_ACL_UDF_L4_TYPE_ANY_PROTO  15


#define GLB_ACL_IN_CLASS_MAX      63
#define GLB_CLASS_IN_POLCIY_MAX  1023


#define GLB_ACE_PORT_TYEP_EQ        1
#define GLB_ACE_PORT_TYEP_LT        2
#define GLB_ACE_PORT_TYEP_GT        3
#define GLB_ACE_PORT_TYEP_RANGE     4
#define GLB_ACE_PORT_TYEP_ANY       5
#define GLB_ACE_PORT_GT_MAX         0xFFFD
#define GLB_ACE_PORT_LT_MIN         2

#define GLB_ACL_L4_PROTOCOL_ICMP    1
#define GLB_ACL_L4_PROTOCOL_ICMPv6  58
#define GLB_ACL_L4_PROTOCOL_IGMP    2
#define GLB_ACL_L4_PROTOCOL_IPIP    4
#define GLB_ACL_L4_PROTOCOL_TCP     6
#define GLB_ACL_L4_PROTOCOL_UDP     17
#define GLB_ACL_L4_PROTOCOL_GRE     47
#define GLB_ACL_L4_PROTOCOL_SCTP    132
#define GLB_ACL_L4_PROTOCOL_RDP     27
#define GLB_ACL_L4_PROTOCOL_DCCP    33

#define GLB_ACL_UDP_VXLAN_L4_DEST_PORT     4789

#define GLB_ACL_GRE_GRE         0
#define GLB_ACL_GRE_NVGRE       01

#define GLB_ACL_GRE_NVGRE_KEY_OFFSET 8
#define GLB_ACL_VXLAN_VNI_MASK_VALID 0x00FFFFFF

enum{
    GLB_ACL_STRIP_HEADER_POS_L2 = 1,
    GLB_ACL_STRIP_HEADER_POS_L3,
    GLB_ACL_STRIP_HEADER_POS_L4,
    GLB_ACL_STRIP_HEADER_MAX,
};

#define GLB_MAC_ACE_PROTOCOL_ARP     0x0806
#define GLB_MAC_ACE_PROTOCOL_RARP    0x8035
#define GLB_MAC_ACE_PROTOCOL_IP      0x0800
#define GLB_MAC_ACE_PROTOCOL_IPV6    0x86DD
#define GLB_MAC_ACE_PROTOCOL_MPLS    0x8847

#define GLB_TIME_RANGE_TYPE_ABSOLUTE    (1                              << 0)
#define GLB_TIME_RANGE_TYPE_PERIODIC    (GLB_TIME_RANGE_TYPE_ABSOLUTE   << 1)


#define GLB_L2GRE_KEY_LEN_16        16
#define GLB_L2GRE_KEY_LEN_20        20
#define GLB_L2GRE_KEY_LEN_24        24
#define GLB_L2GRE_KEY_LEN_32        32
#define GLB_L2GRE_KEY_NUM_16_MAX    0xFFFF
#define GLB_L2GRE_KEY_NUM_20_MAX    0xFFFFF
#define GLB_L2GRE_KEY_NUM_24_MAX    0XFFFFFF
#define GLB_L2GRE_KEY_NUM_32_MAX    0XFFFFFFFF

#define GLB_ACL_SET_POLICER     (1 << 0)
#define GLB_ACL_SET_TC           (GLB_ACL_SET_POLICER << 1)
#define GLB_ACL_SET_COLOR    (GLB_ACL_SET_TC << 1)
#define GLB_ACL_SET_DSCP    (GLB_ACL_SET_COLOR << 1)
#define GLB_ACL_SET_MIRROR_SESSION    (GLB_ACL_SET_DSCP << 1)
#define GLB_ACL_SET_NEW_SVLAN_ID    (GLB_ACL_SET_MIRROR_SESSION << 1)
#define GLB_ACL_SET_NEW_CVLAN_ID    (GLB_ACL_SET_NEW_SVLAN_ID << 1)
#define GLB_ACL_SET_NEW_SCOS_VALUE    (GLB_ACL_SET_NEW_CVLAN_ID << 1)
#define GLB_ACL_SET_NEW_CCOS_VALUE    (GLB_ACL_SET_NEW_SCOS_VALUE << 1)
#define GLB_ACL_SET_REDIRECT_VALUE      (GLB_ACL_SET_NEW_CCOS_VALUE << 1)

#define GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS     (1 << 0)
#define GLB_ACL_POLICY_POLICY_STATS_FLAGS_CLEAR_STATS    (GLB_ACL_POLICY_POLICY_STATS_FLAGS_GET_STATS << 1)

#define GLB_TAP_TRUNCATION_MAX             4

/*added by wangl for GB qos acl*/
#define GLB_ACL_GET_ENTRY_ID(rule_id, label, entry_id) \
        (entry_id = (rule_id << 2) + (label << 22))
struct glb_acl_action_flag_s
{
    uint32 discard:1,
           deny_bridge:1,
           deny_learning:1,
           deny_route:1,
           stats:1,
           trust:1,
           invalid:1,
           priority:1,
           micro_flow_policer:1,
           macro_flow_policer:1,
           log:1,
           copy_to_cpu:1,
           redirect:1,
           dscp:1,
           copy_to_cpu_with_timestamp :1,
           qos_domain:1,
           aging:1,
           reserved:15;
};
typedef struct glb_acl_action_flag_s glb_acl_action_flag_t;
#if 0
#define GLB_MAC_ACE_PERMIT          (1                          << 0)
#define GLB_MAC_ACE_DENY            (GLB_MAC_ACE_PERMIT         << 1)

#define GLB_MAC_ACE_COS             (GLB_MAC_ACE_INNNER_VLAN    << 1)
#define GLB_MAC_ACE_INNER_COS       (GLB_MAC_ACE_COS            << 1)
#define GLB_MAC_ACE_ARP_PKT         (GLB_MAC_ACE_ETHTYPE        << 1)
#define GLB_MAC_ACE_ARP_OPCODE      (GLB_MAC_ACE_ARP_PKT        << 1)
#define GLB_MAC_ACE_ARP_SENDERIP    (GLB_MAC_ACE_ARP_OPCODE     << 1)
#define GLB_MAC_ACE_ARP_TARGETIP    (GLB_MAC_ACE_ARP_SENDERIP   << 1)
#define GLB_MAC_ACE_INVALID         (GLB_MAC_ACE_ARP_TARGETIP   << 1)
#define GLB_MAC_ACE_TIMER_ACTIVE    (GLB_MAC_ACE_INVALID        << 1)
#define GLB_IP_ACE_PERMIT           (GLB_MAC_ACE_INVALID        << 0)
#define GLB_IP_ACE_DENY             (GLB_IP_ACE_PERMIT          << 1)
#define GLB_IP_ACE_IPFRAG           (GLB_IP_ACE_DSTPORT         << 1)
#define GLB_IP_ACE_TCPFLAG          (GLB_IP_ACE_IPFRAG          << 1)
#define GLB_IP_ACE_IPOPTION         (GLB_IP_ACE_TCPFLAG         << 1)
#define GLB_IP_ACE_DSCP             (GLB_IP_ACE_IPOPTION        << 1)
#define GLB_IP_ACE_IPPREC           (GLB_IP_ACE_DSCP            << 1)
#define GLB_IP_ACE_ROUTEDPKT        (GLB_IP_ACE_IPPREC          << 1)
#define GLB_IP_ACE_ICMPTYPE         (GLB_IP_ACE_ROUTEDPKT       << 1)
#define GLB_IP_ACE_ICMPCODE         (GLB_IP_ACE_ICMPTYPE        << 1)
#define GLB_IP_ACE_IGMPTYPE         (GLB_IP_ACE_ICMPCODE        << 1)
#define GLB_IP_ACE_INVALID          (GLB_IP_ACE_IGMPTYPE        << 1)
#define GLB_IP_ACE_TIMER_ACTIVE     (GLB_IP_ACE_INVALID         << 1)
#define GLB_IP_ACE_L4PROTOCOL       (GLB_IP_ACE_IPDA            << 1)
#define GLB_MAC_ACE_L2TYPE          (GLB_MAC_ACE_INNNER_VLAN    << 1)
#define GLB_MAC_ACE_L3TYPE          (GLB_MAC_ACE_L2TYPE         << 1)
#endif

#define GLB_MAC_ACE_MACSA           (1   << 0)        /* 1 << 0 */
#define GLB_MAC_ACE_MACDA           (1   << 1)        /* 1 << 1 */
#define GLB_MAC_ACE_UNTAG           (1   << 2)        /* 1 << 2 */
#define GLB_MAC_ACE_VLAN            (1   << 3)        /* 1 << 3 */
#define GLB_MAC_ACE_INNNER_VLAN     (1   << 4)        /* 1 << 4 */
#define GLB_MAC_ACE_ETHTYPE         (1   << 5)        /* 1 << 5 */
#define GLB_IP_ACE_IPSA             (1   << 6)        /* 1 << 6 */
#define GLB_IP_ACE_IPDA             (1   << 7)        /* 1 << 7 */
#define GLB_IP_ACE_IP_PROTOCOL      (1   << 8)        /* 1 << 8 */
#define GLB_IP_ACE_SRCPORT          (1   << 9)        /* 1 << 9 */
#define GLB_IP_ACE_DSTPORT          (1   << 10)       /* 1 << 10 */
#define GLB_IP_ACE_COS              (1   << 11)       /* 1 << 11 */
#define GLB_IP_ACE_INNER_COS        (1   << 12)       /* 1 << 12 */
#define GLB_IP_ACE_DSCP             (1   << 13)       /* 1 << 13 */
#define GLB_IP_ACE_IP_FRAG          (1   << 14)       /* 1 << 14 */
#define GLB_IP_ACE_IGMP_TYPE        (1   << 15)       /* 1 << 15 */
#define GLB_IP_ACE_ICMP_TYPE        (1   << 16)       /* 1 << 16 */
#define GLB_IP_ACE_ICMP_CODE        (1   << 17)       /* 1 << 17 */
#define GLB_IP_ACE_TCP_FLAG         (1   << 18)       /* 1 << 18 */
#define GLB_IP_ACE_PRECEDENCE       (1   << 19)       /* 1 << 19 */
#define GLB_IP_ACE_TIME_RANGE       (1   << 20)       /* 1 << 20 */
#define GLB_IP_ACE_INVALID          (1   << 21)       /* 1 << 21 */
#define GLB_IP_ACE_TIMER_ACTIVE     (1   << 22)       /* 1 << 22 */
#define GLB_IP_ACE_OPTIONS          (1   << 23)       /* 1 << 23 */
#define GLB_IP_ACE_TAP_ACTION       (1   << 24)       /* 1 << 24 */
#define GLB_IP_ACE_TAP_TRUNCATION   (1   << 25)       /* 1 << 25 */
#define GLB_IP_ACE_VXLAN_VNI        (1   << 26)       /* 1 << 26 */
#define GLB_IP_ACE_TAP_UNTAG        (1   << 27)       /* 1 << 27 */
#define GLB_IP_ACE_TAP_MARK_SOURCE  (1   << 28)       /* 1 << 28 */
#define GLB_IP_ACE_TAP_FLOW_MATCH   (1   << 29)       /* 1 << 29 */
#define GLB_IP_ACE_GRE_KEY          (1   << 30)       /* 1 << 30 */
#define GLB_IP_ACE_UDF              (1   << 31)	      /* 1 << 31 */
/* key_ipv6_flags */
#define GLB_IPV6_ACE_IPSA             (1   << 0)        /* 1 << 0 */
#define GLB_IPV6_ACE_IPDA             (1   << 1)        /* 1 << 1 */
#define GLB_IPV6_ACE_FLOW_LABEL       (1   << 2)        /* 1 << 2 */

/* key_flag2 */
#define GLB_IP_ACE_STRIP_HEADER_POS     (1   << 0)	      /* 1 << 0 */
#define GLB_IP_ACE_STRIP_HEADER_OFFSET  (1   << 1)	      /* 1 << 1 */
#define GLB_IP_ACE_ERSPAN_ID  (1   << 2)	      /* 1 << 1 */
#define GLB_IP_ACE_STRIP_INNER_VXLAN_HEADER  (1   << 3)	      /* 1 << 1 */
#define GLB_IP_ACE_ADD_L2GRE_KEY             (1   << 4)	      /* 1 << 1 */

/* key_arp_flags */
#define GLB_ARP_ACE_OP_CODE             (1   << 0)	      /* 1 << 0 */
#define GLB_ARP_ACE_SENDER_IP           (1   << 1)	      /* 1 << 1 */
#define GLB_ARP_ACE_TARGET_IP           (1   << 2)	      /* 1 << 2 */

/* acl udf key flags */
#define GLB_ACL_UDF_ETHTYPE     (1   << 0)        /* 1 << 0 */
#define GLB_ACL_UDF_VLANNUM     (1   << 1)        /* 1 << 1 */
#define GLB_ACL_UDF_IPPROTO     (1   << 2)        /* 1 << 2 */
#define GLB_ACL_UDF_L4_TYPE     (1   << 3)        /* 1 << 3 */
#define GLB_ACL_UDF_SRCPORT     (1   << 4)        /* 1 << 4 */
#define GLB_ACL_UDF_DSTPORT     (1   << 5)        /* 1 << 5 */
#define GLB_ACL_UDF_MPLSNUM     (1   << 6)        /* 1 << 5 */


#define GLB_PMAP_CLASS_PRIORITY             (1                          << 0)
#define GLB_PMAP_CLASS_COLOR                (GLB_PMAP_CLASS_PRIORITY    << 1)
#define GLB_PMAP_CLASS_TRUST                (GLB_PMAP_CLASS_COLOR       << 1)
#define GLB_PMAP_CLASS_MONITOR              (GLB_PMAP_CLASS_TRUST       << 1)
#define GLB_PMAP_CLASS_REDIRECT             (GLB_PMAP_CLASS_MONITOR     << 1)
#define GLB_PMAP_CLASS_STATS                (GLB_PMAP_CLASS_REDIRECT    << 1)
#define GLB_PMAP_CLASS_DOMAIN               (GLB_PMAP_CLASS_STATS       << 1)
#define GLB_PMAP_CLASS_COS                  (GLB_PMAP_CLASS_DOMAIN      << 1)
#define GLB_PMAP_CLASS_DSCP                 (GLB_PMAP_CLASS_COS         << 1)
#define GLB_PMAP_CLASS_CTAG_COS             (GLB_PMAP_CLASS_DSCP        << 1)
#define GLB_PMAP_CLASS_STAG_COS             (GLB_PMAP_CLASS_CTAG_COS    << 1)
#define GLB_PMAP_CLASS_POLICER              (GLB_PMAP_CLASS_STAG_COS    << 1)
#define GLB_PMAP_CLASS_AGGREGATE_POLICER    (GLB_PMAP_CLASS_POLICER     << 1)
#define GLB_PMAP_CLASS_PHB                  (GLB_PMAP_CLASS_AGGREGATE_POLICER     << 1)
#define GLB_PMAP_CLASS_TAP                  (GLB_PMAP_CLASS_PHB     << 1)

#define MAC_ACL_MAC_LEN 6
#define GLB_TIME_RANGE_MAX_NUM    256
#define GLB_ADD_GRE_HEADER_ENTRY_MAX_NUM 4

/*added by guoxd for vxlan*/
#define GLB_ADD_VXLAN_HEADER_ENTRY_MAX_NUM 4 
/*ended by guoxd*/

/* TCP flag operator */
enum glb_acl_tcp_flag_operator_e
{
    GLB_ACL_TCP_FLAG_OP_ALL,     /* all flags being set */
    GLB_ACL_TCP_FLAG_OP_ANY,     /* any flags being set */

    GLB_ACL_TCP_FLAG_OP_MAX
};
typedef enum glb_acl_tcp_flag_operator_e glb_acl_tcp_flag_op_t;


enum glb_acl_tcp_flag_flag_e
{
    GLB_ACL_TCP_FLAG_FIN = 1 << 0,
    GLB_ACL_TCP_FLAG_SYN = 1 << 1,
    GLB_ACL_TCP_FLAG_RST = 1 << 2,
    GLB_ACL_TCP_FLAG_PSH = 1 << 3,
    GLB_ACL_TCP_FLAG_ACK = 1 << 4,
    GLB_ACL_TCP_FLAG_URG = 1 << 5
};

enum glb_acl_ip_frag_e
{
    GLB_ACL_IP_FRAG_NONE,           /* non fragment or an initial fragment */
    GLB_ACL_IP_FRAG_FIRST_FRAG, /*first fragment*/
    GLB_ACL_IP_FRAG_NON_OR_FIRST_FRAG, /*non fragment or first fragment*/
    GLB_ACL_IP_FRAG_TINY,           /* small fragment */
    GLB_ACL_IP_FRAG_NON_FIRST,    /* not first fragment */
    GLB_ACL_IP_FRAG_YES,           /* any fragment */

    GLB_ACL_IP_FRAG_MAX
};
typedef enum glb_acl_ip_frag_e glb_acl_ip_frag_t;

enum glb_acl_igmp_type_e
{
    GLB_ACL_IGMP_TYPE_PRECEDENCE       = 0,
    GLB_ACL_IGMP_TYPE_HOST_QUERY       = 0x11,
    GLB_ACL_IGMP_TYPE_HOST_REPORT      = 0x12,
    GLB_ACL_IGMP_TYPE_HOST_DVMRP       = 0x13,
    GLB_ACL_IGMP_TYPE_PIM              = 0x14,
    GLB_ACL_IGMP_TYPE_TRACE            = 0x15,
    GLB_ACL_IGMP_TYPE_V2_REPORT        = 0x16,
    GLB_ACL_IGMP_TYPE_V2_LEAVE         = 0x17,
    GLB_ACL_IGMP_TYPE_MTRACT_RESPONSE  = 0x1e,
    GLB_ACL_IGMP_TYPE_MTRACT           = 0x1f,
    GLB_ACL_IGMP_TYPE_V3_REPORT        = 0x22
};
typedef enum glb_acl_igmp_type_e glb_acl_igmp_type_t;

enum glb_acl_type_e
{
    GLB_ACL_TYPE_MAC = 0,
    GLB_ACL_TYPE_IP,
    GLB_ACL_TYPE_IP6,
    GLB_ACL_TYPE_EXIP,
    GLB_ACL_TYPE_EXIP6,
    GLB_ACL_TYPE_MPLS,
    GLB_ACL_TYPE_EX_MPLS,
    GLB_ACL_TYPE_MAX,
};
typedef enum glb_acl_type_e glb_acl_type_t;

enum glb_acl_packet_type_e
{
    GLB_ACL_PACKET_TYPE_UNKNOW       = 0,
    GLB_ACL_PACKET_TYPE_VXLAN,
    GLB_ACL_PACKET_TYPE_GRE,
    GLB_ACL_PACKET_TYPE_NVGRE,
    GLB_ACL_PACKET_TYPE_IPIP,
    GLB_ACL_PACKET_TYPE_MAX
};
typedef enum glb_acl_packet_type_e glb_acl_packet_type_t;

struct glb_acl_action_s
{
    glb_acl_action_flag_t flag;

    uint32 redirect_if_index;;

    uint8 log_id;
    uint8 log_weight;   /* 0 ~ 15, log_prob = 1/2^(15-log_weight) */
    uint8 priority;
    uint8 color;
    uint8 trust;
    uint16 micro_policer_id;
    uint16 macro_policer_id;
    uint8 dscp;
    uint8 qos_domain;
};
typedef struct glb_acl_action_s glb_acl_action_t;


struct glb_ace_s
{
    glb_acl_action_t action;
    glb_flow_rule_t rule;
    uint32 seq_num;
    uint8  reserved[3];
};
typedef struct glb_ace_s glb_ace_t;


struct glb_acl_access_grp_s
{
    char  sz_name[ACL_NAME_FULL_NAME_SIZE+4];

    uint8  acl_type;     /* GLB_FLOW_FILTER_XXX */
    uint8  ace_num;    
    uint8  reserved[2];
    
    glb_ace_t ace[GLB_MAX_MSG_ACE_NUM];
};
typedef struct glb_acl_access_grp_s glb_acl_access_grp_t;

struct glb_acl_clear_stats_intf_s
{
    uint32 ifindex;
    uint8  dir;
    uint8  reserved[3];
};
typedef struct glb_acl_clear_stats_intf_s glb_acl_clear_stats_intf_t;

struct glb_acl_clear_stats_all_req_s
{
    uint8  acl_type;
    uint8  intf_num;
    uint8  reserved[2];
    glb_acl_clear_stats_intf_t if_list[GLB_ACL_MAX_MSG_INTF_NUM];
};
typedef struct glb_acl_clear_stats_all_req_s glb_acl_clear_stats_all_req_t;


enum glb_acl_policy_operation_type_e
{
    GLB_ACL_POLICY_OP_IF_PMAP = 0,
    GLB_ACL_POLICY_OP_PMAP_CLASS,
    GLB_ACL_POLICY_OP_CLASS_ACL,
    GLB_ACL_POLICY_OP_ACL_ACE,
    GLB_ACL_POLICY_OP_MAX,
};
typedef enum glb_acl_policy_operation_type_e glb_acl_policy_operation_type_t;


enum glb_acl_policy_op_action_type_e
{
    GLB_ACL_POLICY_OP_ACTION_ADD = 0,
    GLB_ACL_POLICY_OP_ACTION_DEL,
    GLB_ACL_POLICY_OP_ACTION_UPDATE,
    GLB_ACL_POLICY_OP_ACTION_MAX,
};
typedef enum glb_acl_policy_op_action_type_e glb_acl_policy_op_action_type_t;


enum glb_tap_grp_igr_flow_op_type_e
{
    GLB_TAP_GRP_IGR_FLOW_OP_ADD_ACE = 1,
    GLB_TAP_GRP_IGR_FLOW_OP_DEL_ACE,
    GLB_TAP_GRP_IGR_FLOW_OP_MAX,
};
typedef enum glb_tap_grp_igr_flow_op_type_e glb_tap_grp_igr_flow_op_type_t;

#endif /* __GLB_ACL_DEFINE_H__ */
