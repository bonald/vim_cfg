
/* @date 2010-07-01
 
 @version v1.0
 
The file define  all CNOS flow module's  common define .

*/

#ifndef __GLB_FLOW_DEFINE_H__
#define __GLB_FLOW_DEFINE_H__

#define GLB_FLOW_TCP_FLAG_OP_TO_STR(op)                                         \
    (op == GLB_FLOW_TCP_FLAG_OP_ANY ? "ANY" :                                   \
     op == GLB_FLOW_TCP_FLAG_OP_ALL ? "ALL" :                                   \
     "ERROR")

#define GLB_FLOW_L4PORT_OP_TO_STR(op)                                           \
    (op == GLB_FLOW_L4PORT_OPERATOR_EQ ? "EQ" :                                 \
     op == GLB_FLOW_L4PORT_OPERATOR_GT ? "GT" :                                 \
     op == GLB_FLOW_L4PORT_OPERATOR_LT ? "LT" :                                 \
     op == GLB_FLOW_L4PORT_OPERATOR_NEQ ? "NEQ" :                               \
     op == GLB_FLOW_L4PORT_OPERATOR_RANGE ? "RANGE" :                           \
     "ERROR")

#define GLB_FLOW_L2TYPE_TO_STR(type)                                            \
    (type == GLB_FLOW_L2TYPE_ETH_V2 ? "eth2" :                                  \
     type == GLB_FLOW_L2TYPE_ETH_SAP ? "sap" :                                  \
     type == GLB_FLOW_L2TYPE_ETH_SNAP ? "snap" :                                \
     type == GLB_FLOW_L2TYPE_PPP_2B ? "ppp-2b" :                                \
     type == GLB_FLOW_L2TYPE_PPP_1B ? "ppp-1b" :                                \
     type == GLB_FLOW_L2TYPE_RAW_SNAP ? "raw-snap" :                            \
     "error l2-type")

#define GLB_FLOW_L3TYPE_TO_STR(type)                                            \
    (type == GLB_FLOW_L3TYPE_IP_V4 ? "ipv4" :                                   \
     type == GLB_FLOW_L3TYPE_IP_V6 ? "ipv6" :                                   \
     type == GLB_FLOW_L3TYPE_MPLS ? "mpls" :                                    \
     type == GLB_FLOW_L3TYPE_MPLS_MCAST ? "mpls_mcast" :                        \
     type == GLB_FLOW_L3TYPE_ARP ? "arp" :                                      \
     type == GLB_FLOW_L3TYPE_RARP ? "rarp" :                                    \
     type == GLB_FLOW_L3TYPE_EAPOL ? "eapol" :                                  \
     type == GLB_FLOW_L3TYPE_SLOWPROTO ? "slow-proto" :                         \
     type == GLB_FLOW_L3TYPE_CMAC ? "cmac" :                                    \
     type == GLB_FLOW_L3TYPE_PTP ? "ptp" :                                      \
     "error l3-type")

#define GLB_FLOW_L4PROTO_TO_STR(l4protocal)                                     \
    (l4protocal == 1 ? "icmp" :                                                 \
     l4protocal == 58 ? "icmpv6" :                                              \
     l4protocal == 2 ? "igmp" :                                                 \
     l4protocal == 6 ? "tcp" :                                                  \
     l4protocal == 17 ? "udp" :                                                 \
     l4protocal == 0 ? "any" :                                                  \
     "error l4-proto")


/*modified by yanxa, for bug 17674, 2012-03-06*/
#define GLB_FLOW_ETHER_TYPE_ARP     0x0806
#define GLB_FLOW_ETHER_TYPE_RARP    0x8035
#define GLB_FLOW_ETHER_TYPE_IP      0x0800
#define GLB_FLOW_ETHER_TYPE_IPV6    0x86DD
#define GLB_FLOW_ETHER_TYPE_MPLS    0x8847
#define GLB_FLOW_ETHER_TYPE_MMPLS   0x8848  /*multicast mpls*/

#define ACL_IGMP_TYPE_HOST_QUERY 0x11
#define ACL_IGMP_TYPE_HOST_REPORT 0x12
#define ACL_IGMP_TYPE_HOST_DVMRP 0x13
#define ACL_IGMP_TYPE_PIM 0x14
#define ACL_IGMP_TYPE_TRACE 0x15
#define ACL_IGMP_TYPE_V2_REPORT 0x16
#define ACL_IGMP_TYPE_V2_LEAVE 0x17
#define ACL_IGMP_TYPE_MTRACT 0x1f
#define ACL_IGMP_TYPE_MTRACT_RESPONSE 0x1e
#define ACL_IGMP_TYPE_PRECEDENCE 0  /*temp, maybe change later*/
#define ACL_IGMP_TYPE_V3_REPORT 0x22

#define L4_PROTOCAL_NUM 0
#define ICMP_PROTOCAL_NUM 1
#define ICMPV6_PROTOCAL_NUM 58
#define IGMP_PROTOCAL_NUM 2
#define TCP_PROTOCAL_NUM 6
#define UDP_PROTOCAL_NUM 17

enum glb_flow_l2type_e
{
    GLB_FLOW_L2TYPE_ETH_V2   = 1,    /**< L2 packet type: Ethernet II */
    GLB_FLOW_L2TYPE_ETH_SAP  = 2,    /**< L2 packet type: SAP */
    GLB_FLOW_L2TYPE_ETH_SNAP = 3,    /**< L2 packet type: SNAP */
    GLB_FLOW_L2TYPE_PPP_2B   = 4,    /**< L2 packet type: PPP-2B */
    GLB_FLOW_L2TYPE_PPP_1B   = 5,    /**< L2 packet type: PPP-1B */
    GLB_FLOW_L2TYPE_RAW_SNAP = 6     /**< L2 packet type: RAW */
};

enum glb_flow_l3type_e
{
    GLB_FLOW_L3TYPE_IP_V4      = 2,  /**< L3 packet type: IPv4 */
    GLB_FLOW_L3TYPE_IP_V6      = 3,  /**< L3 packet type: IPv6 */
    GLB_FLOW_L3TYPE_MPLS       = 4,  /**< L3 packet type: MPLS */
    GLB_FLOW_L3TYPE_MPLS_MCAST = 5,  /**< L3 packet type: MPLS Multicast */
    GLB_FLOW_L3TYPE_ARP        = 6,  /**< L3 packet type: ARP */
    GLB_FLOW_L3TYPE_RARP       = 7,  /**< L3 packet type: RARP */
    GLB_FLOW_L3TYPE_EAPOL      = 8,  /**< L3 packet type: EAPOL */
    GLB_FLOW_L3TYPE_ETHOAM     = 9,  /**< L3 packet type: Ethernet OAM */
    GLB_FLOW_L3TYPE_SLOWPROTO  = 10, /**< L3 packet type: Slow Protocol */
    GLB_FLOW_L3TYPE_CMAC       = 11, /**< L3 packet type: CMAC */
    GLB_FLOW_L3TYPE_PTP        = 12, /**< L3 packet type: PTP */
    GLB_FLOW_L3TYPE_MAX        = 15  /**< L3 packet type: Invalid */
};

enum glb_flow_action_flag_e
{
    GLB_FLOW_ACTION_FLAG_DISCARD              = 1 << 0,
    GLB_FLOW_ACTION_FLAG_DENY_BRIDGE          = 1 << 1,
    GLB_FLOW_ACTION_FLAG_DENY_LEARN           = 1 << 2,
    GLB_FLOW_ACTION_FLAG_DENY_ROUTE           = 1 << 3,
    GLB_FLOW_ACTION_FLAG_AGING                     = 1 << 4,
    GLB_FLOW_ACTION_FLAG_QOS_DOMAIN              = 1 << 5,
    GLB_FLOW_ACTION_FLAG_STATS                = 1 << 6,
    GLB_FLOW_ACTION_FLAG_REDIRECT_IF          = 1 << 7,
    GLB_FLOW_ACTION_FLAG_LOG               = 1 << 8,
    GLB_FLOW_ACTION_FLAG_DSCP                  = 1 << 9,
    GLB_FLOW_ACTION_FLAG_TRUST                = 1 << 10,
    GLB_FLOW_ACTION_FLAG_PRIORITY             = 1 << 11,
    GLB_FLOW_ACTION_FLAG_MACRO_POLICER         = 1 << 12,
    GLB_FLOW_ACTION_FLAG_INVALID              = 1 << 13,
    GLB_FLOW_ACTION_FLAG_MICRO_POLICER        = 1 << 14,
    GLB_FLOW_ACTION_FLAG_COPY_TO_CPU          = 1 << 15,
    GLB_FLOW_ACTION_FLAG_COPY_TO_CPU_WITH_TIMESTAMP    = 1 << 16,
/* modified by cuixl for bug 22441, GB QoS development, 2013-04-09 */
/*modification start*/
    GLB_FLOW_ACTION_FLAG_STAG_COS                  = 1 << 17,
    GLB_FLOW_ACTION_FLAG_CTAG_COS                  = 1 << 18
/*modification end*/
    
};

enum glb_flow_rule_type_e
{
    GLB_FLOW_RULE_TYPE_MAC,          /* MAC flow rule type */
    GLB_FLOW_RULE_TYPE_IPV4,         /* IPv4 flow rule type */
    GLB_FLOW_RULE_TYPE_IPV6,         /* IPv6 flow filter type */
    GLB_FLOW_RULE_TYPE_EXIP,         /* Extended IP flow rule type */
    GLB_FLOW_RULE_TYPE_EXIPV6,       /* Extended IPv6 flow rule type */
    GLB_FLOW_RULE_TYPE_MPLS,         /* MPLS flow rule type */

    GLB_FLOW_RULE_TYPE_MAX
};
typedef enum glb_flow_rule_type_e glb_flow_rule_type_t;


enum glb_flow_l4port_operator_e
{
    GLB_FLOW_L4PORT_OPERATOR_EQ,         /* x = m */
    GLB_FLOW_L4PORT_OPERATOR_NEQ,        /* x != m */
    GLB_FLOW_L4PORT_OPERATOR_GT,         /* x > m */
    GLB_FLOW_L4PORT_OPERATOR_LT,         /* x < m */
    GLB_FLOW_L4PORT_OPERATOR_RANGE,      /* m < x < n */

    GLB_FLOW_L4PORT_OPERATOR_MAX
};
typedef enum glb_flow_l4port_operator_e glb_flow_l4port_operator_t;


/* TCP flag operator */
enum glb_flow_tcp_flag_operator_e
{
    GLB_FLOW_TCP_FLAG_OP_ANY,     /* any flags being set */
    GLB_FLOW_TCP_FLAG_OP_ALL,     /* all flags being set */

    GLB_FLOW_TCP_FLAG_OP_MAX
};
typedef enum glb_flow_tcp_flag_operator_e glb_acl_tcp_flag_operator_t;


enum glb_flow_tcp_flag_flag_e
{
    GLB_FLOW_TCP_FLAG_URG = 1 << 0,
    GLB_FLOW_TCP_FLAG_ACK = 1 << 1,
    GLB_FLOW_TCP_FLAG_PSH = 1 << 2,
    GLB_FLOW_TCP_FLAG_RST = 1 << 3,
    GLB_FLOW_TCP_FLAG_SYN = 1 << 4,
    GLB_FLOW_TCP_FLAG_FIN = 1 << 5
};


enum glb_flow_ip_frag_e
{
    GLB_FLOW_IP_FRAG_NONE,           /* non fragment or an initial fragment */
    GLB_FLOW_IP_FRAG_FIRST_FRAG, /*first fragment*/
    GLB_FLOW_IP_FRAG_NON_OR_FIRST_FRAG, /*non fragment or first fragment*/
    GLB_FLOW_IP_FRAG_TINY,           /* small fragment */
    GLB_FLOW_IP_FRAG_NON_INITIAL,    /* non initial fragment */
    GLB_FLOW_IP_FRAG_LAST,           /* last fragment */

    GLB_FLOW_IP_FRAG_MAX
};
typedef enum glb_flow_ip_frag_e glb_flow_ip_frag_t;


enum glb_flow_igmp_type_e
{
    GLB_FLOW_IGMP_TYPE_PRECEDENCE       = 0,
    GLB_FLOW_IGMP_TYPE_HOST_QUERY       = 0x11,
    GLB_FLOW_IGMP_TYPE_HOST_REPORT      = 0x12,
    GLB_FLOW_IGMP_TYPE_HOST_DVMRP       = 0x13,
    GLB_FLOW_IGMP_TYPE_PIM              = 0x14,
    GLB_FLOW_IGMP_TYPE_TRACE            = 0x15,
    GLB_FLOW_IGMP_TYPE_V2_REPORT        = 0x16,
    GLB_FLOW_IGMP_TYPE_V2_LEAVE         = 0x17,
    GLB_FLOW_IGMP_TYPE_MTRACT_RESPONSE  = 0x1e,
    GLB_FLOW_IGMP_TYPE_MTRACT           = 0x1f,
    GLB_FLOW_IGMP_TYPE_V3_REPORT        = 0x22
};
typedef enum glb_flow_igmp_type_e glb_flow_igmp_type_t;


enum glb_flow_mac_rule_flag_e
{
    GLB_FLOW_MAC_RULE_FLAG_MACSA       = 1 << 0,
    GLB_FLOW_MAC_RULE_FLAG_MACDA       = 1 << 1,
    GLB_FLOW_MAC_RULE_FLAG_CVLAN_ID    = 1 << 2,
    GLB_FLOW_MAC_RULE_FLAG_SVLAN_ID    = 1 << 3,
    GLB_FLOW_MAC_RULE_FLAG_CTAG_COS    = 1 << 4,
    GLB_FLOW_MAC_RULE_FLAG_STAG_COS    = 1 << 5,
    GLB_FLOW_MAC_RULE_FLAG_L3TYPE      = 1 << 6,
    GLB_FLOW_MAC_RULE_FLAG_L2TYPE      = 1 << 7,
    GLB_FLOW_MAC_RULE_FLAG_ETHTYPE     = 1 << 8,/*Added by yanxa, for bug 17674, 2012-03-06*/
    GLB_FLOW_MAC_RULE_FLAG_ARP_OP_CODE = 1 << 9, /*Added by wangl for GB acl, 2013-1-22*/
    GLB_FLOW_IPV4_EXTEND_RULE_FLAG_ARP_PACKET = 1 << 10,
    GLB_FLOW_IPV4_EXTEND_RULE_FLAG_SENDER_IP = 1 <<11,
    GLB_FLOW_IPV4_EXTEND_RULE_FLAG_TARGET_IP = 1 << 12,
    GLB_FLOW_MAC_RULE_FLAG_UNTAG = 1 << 13         /*modify by chenxw for bug 24076, 2013-09-04.*/
};


enum glb_flow_ipv4_rule_flag_e
{
    GLB_FLOW_IPV4_RULE_FLAG_IPSA           = 1 << 0,
    GLB_FLOW_IPV4_RULE_FLAG_IPDA           = 1 << 1,
    GLB_FLOW_IPV4_RULE_FLAG_L4_PROTO       = 1 << 2,
    GLB_FLOW_IPV4_RULE_FLAG_L4_SRC_PORT    = 1 << 3,
    GLB_FLOW_IPV4_RULE_FLAG_L4_DST_PORT    = 1 << 4,
    GLB_FLOW_IPV4_RULE_FLAG_TCP_FLAG       = 1 << 5,
    GLB_FLOW_IPV4_RULE_FLAG_ICMP_TYPE      = 1 << 6,
    GLB_FLOW_IPV4_RULE_FLAG_IGMP_TYPE      = 1 << 7,
    GLB_FLOW_IPV4_RULE_FLAG_ICMP_CODE      = 1 << 8,
    GLB_FLOW_IPV4_RULE_FLAG_DSCP           = 1 << 9,
    GLB_FLOW_IPV4_RULE_FLAG_IP_FRAG        = 1 << 10,
    GLB_FLOW_IPV4_RULE_FLAG_IP_OPTIONS     = 1 << 11,
    GLB_FLOW_IPV4_RULE_FLAG_ROUTED_PACKET  = 1 << 12,

    /* extended mac rule flags */
    GLB_FLOW_IPV4_RULE_FLAG_MACSA          = 1 << 13,
    GLB_FLOW_IPV4_RULE_FLAG_MACDA          = 1 << 14,
    GLB_FLOW_IPV4_RULE_FLAG_CVLAN_ID       = 1 << 15,
    GLB_FLOW_IPV4_RULE_FLAG_SVLAN_ID       = 1 << 16,
    GLB_FLOW_IPV4_RULE_FLAG_CTAG_COS       = 1 << 17,
    GLB_FLOW_IPV4_RULE_FLAG_STAG_COS       = 1 << 18,
    GLB_FLOW_IPV4_RULE_FLAG_L3TYPE         = 1 << 19,
    GLB_FLOW_IPV4_RULE_FLAG_L2TYPE         = 1 << 20,
    GLB_FLOW_IPV4_RULE_FLAG_ETHTYPE        = 1 << 21/*Added by yanxa, for bug 17674, 2012-03-06*/
};


enum glb_flow_ipv6_rule_flag_e
{
    GLB_FLOW_IPV6_RULE_FLAG_IPSA           = 1 << 0,
    GLB_FLOW_IPV6_RULE_FLAG_IPDA           = 1 << 1,
    GLB_FLOW_IPV6_RULE_FLAG_L4_PROTO       = 1 << 2,
    GLB_FLOW_IPV6_RULE_FLAG_L4_SRC_PORT    = 1 << 3,
    GLB_FLOW_IPV6_RULE_FLAG_L4_DST_PORT    = 1 << 4,
    GLB_FLOW_IPV6_RULE_FLAG_TCP_FLAG       = 1 << 5,
    GLB_FLOW_IPV6_RULE_FLAG_ICMP_TYPE      = 1 << 6,
    GLB_FLOW_IPV6_RULE_FLAG_ICMP_CODE      = 1 << 7,
    GLB_FLOW_IPV6_RULE_FLAG_DSCP           = 1 << 9,
    GLB_FLOW_IPV6_RULE_FLAG_IP_FRAG        = 1 << 10,
    GLB_FLOW_IPV6_RULE_FLAG_IP_OPTION      = 1 << 11,
    GLB_FLOW_IPV6_RULE_FLAG_ROUTED_PACKET  = 1 << 12,

    /* extended mac rule flags */
    GLB_FLOW_IPV6_RULE_FLAG_MACSA          = 1 << 16,
    GLB_FLOW_IPV6_RULE_FLAG_MACDA          = 1 << 17,
    GLB_FLOW_IPV6_RULE_FLAG_CVLAN_ID       = 1 << 18,
    GLB_FLOW_IPV6_RULE_FLAG_SVLAN_ID       = 1 << 19,
    GLB_FLOW_IPV6_RULE_FLAG_CTAG_COS       = 1 << 20,
    GLB_FLOW_IPV6_RULE_FLAG_STAG_COS       = 1 << 21,
    GLB_FLOW_IPV6_RULE_FLAG_L3TYPE         = 1 << 22,
    GLB_FLOW_IPV6_RULE_FLAG_L2TYPE         = 1 << 23,
    GLB_FLOW_IPV6_RULE_FLAG_ETHTYPE        = 1 << 24,
    GLB_FLOW_IPV6_RULE_FLAG_FLOW_LABEL     = 1 << 25
};

/*Added by yanxa, for mpls acl, bug 17222, 2011-12-08*/
enum glb_flow_mpls_rule_flag_e
{
    GLB_FLOW_MPLS_RULE_FLAG_MACSA       = 1 << 0,
    GLB_FLOW_MPLS_RULE_FLAG_MACDA       = 1 << 1,
    GLB_FLOW_MPLS_RULE_FLAG_CVLAN_ID    = 1 << 2,
    GLB_FLOW_MPLS_RULE_FLAG_SVLAN_ID    = 1 << 3,
    GLB_FLOW_MPLS_RULE_FLAG_CTAG_COS    = 1 << 4,
    GLB_FLOW_MPLS_RULE_FLAG_STAG_COS    = 1 << 5,
    GLB_FLOW_MPLS_RULE_FLAG_L2TYPE      = 1 << 6,
    GLB_FLOW_MPLS_RULE_FLAG_LABEL0      = 1 << 7,
    GLB_FLOW_MPLS_RULE_FLAG_LABEL1      = 1 << 8,
    GLB_FLOW_MPLS_RULE_FLAG_LABEL2      = 1 << 9,
    GLB_FLOW_MPLS_RULE_FLAG_LABEL3      = 1 << 10
};

/*-------------------------- MAC filter --------------------------*/

struct glb_flow_mac_rule_flag_s
{
  uint32 mac_sa:1,
         cvlan:1,
         svlan:1,
         l3_type:1,
         eth_type:1,/*Added by yanxa, for bug 17674, 2012-03-06*/
         mac_da:1,
         l2_type:1,
         ctag_cos:1,
         stag_cos:1,
         arp_packet:1,
         arp_op_code:1,/*added by wangl, for GB ACL 2013-1-22*/
         sender_ip:1,
         target_ip:1,
         untag:1, /*modify by chenxw for bug 24076, 2013-09-04.*/
         reserved:18;
};
typedef struct glb_flow_mac_rule_flag_s glb_flow_mac_rule_flag_t;

struct glb_flow_mac_rule_s
{
   glb_flow_mac_rule_flag_t flag;

    uint32 sender_ip;
    uint32 sender_ip_mask;
    uint32 target_ip;
    uint32 target_ip_mask;
    mac_addr_t mac_sa;
    mac_addr_t mac_sa_mask;
    mac_addr_t mac_da;
    mac_addr_t mac_da_mask;
    uint16 cvlan;
    uint16 svlan;
    uint8  ctag_cos;
    uint8  stag_cos;
    uint8  l3_type;
    uint8  l2_type;
    uint16 eth_type;/*Added by yanxa, for bug 17674, 2012-03-06*/
    uint16 eth_type_mask;/*Added by yanxa, for bug 17674, 2012-03-06*/
    uint16 arp_op_code;
};
typedef struct glb_flow_mac_rule_s glb_flow_mac_rule_t;


/*-------------------------- IPv4 filter --------------------------*/

struct glb_flow_ipv4_rule_flag_s
{
    uint32 ip_sa:1,
           ip_da:1,
           l4_protocol:1,
           l4_source_port:1,
           l4_dest_port:1,
           tcp_flag:1,
           icmp_type:1,
           igmp_type:1,
           icmp_code:1,
           dscp:1,
           ip_frag:1,
           ip_options:1,
           routed_packet:1,
           reserved:19;
};
typedef struct glb_flow_ipv4_rule_flag_s glb_flow_ipv4_rule_flag_t;


/* l4 port parameter */
struct glb_flow_l4_port_s
{
    glb_flow_l4port_operator_t operator;
    uint16 port;      /* first port */
    uint16 port2;     /* second port */
};
typedef struct glb_flow_l4_port_s glb_flow_l4_port_t;

/* TCP flag parameter */
struct glb_flow_tcp_flag_s
{
    glb_acl_tcp_flag_operator_t operator;
    uint8 tcp_flags;    /* GLB_FLOW_TCP_FLAG_XXX */
    uint8 reserved[3];
};
typedef struct glb_flow_tcp_flag_s glb_flow_tcp_flag_t;


struct glb_flow_ipv4_rule_s
{
    glb_flow_ipv4_rule_flag_t flag;

    uint32 ip_sa;
    uint32 ip_sa_mask;
    uint32 ip_da;
    uint32 ip_da_mask;
    glb_flow_l4_port_t l4_source_port;
    glb_flow_l4_port_t l4_dest_port;
    glb_flow_ip_frag_t ip_frag;
    glb_flow_tcp_flag_t tcp_flag;
    uint8  ip_options;
    uint8  routed_packet;
    uint8  dscp;
    uint8  l4_protocol;
    uint8  icmp_type;
    uint8  icmp_code;
    uint8  igmp_type;
    uint8  reserved;
};
typedef struct glb_flow_ipv4_rule_s glb_flow_ipv4_rule_t;

/*-------------------------- IPv6 filter --------------------------*/

struct glb_flow_ipv6_rule_flag_s
{
    uint32 ip_sa:1,
           ip_da:1,
           l4_protocol:1,
           l4_source_port:1,
           l4_dest_port:1,
           tcp_flag:1,
           icmp_type:1,
           icmp_code:1,
           igmp_type:1,
           dscp:1,
           ip_frag:1,
           ip_options:1,
           routed_packet:1,
           routing:1,
           authen:1,
           destopts:1,
           flow_label:1,
           reserved:15;
};
typedef struct glb_flow_ipv6_rule_flag_s glb_flow_ipv6_rule_flag_t;

struct glb_flow_ipv6_rule_s
{
    glb_flow_ipv6_rule_flag_t flag;

    ipv6_addr_t sprefix;
    ipv6_addr_t dprefix;
    uint8  sprefix_len;
    uint8  dprefix_len;
    uint8  ip_frag;        /* GLB_ACL_IP_FRAG_XXX */
    uint8  ip_options;
    glb_flow_l4_port_t l4_source_port;
    glb_flow_l4_port_t l4_dest_port;
    glb_flow_tcp_flag_t tcp_flag;
    uint8  routed_packet;
    uint8  l4_protocol;
    uint8  icmp_type;
    uint8  icmp_code;
    uint32 flow_label;
    uint8  ext_hdr;
    uint8  dscp;
    uint8  reserved[2];
};
typedef struct glb_flow_ipv6_rule_s glb_flow_ipv6_rule_t;

/*Added by yanxa, for mpls acl, bug 17222, 2011-12-08*/
/*-------------------------- MPLS filter --------------------------*/

struct glb_flow_mpls_rule_flag_s
{
  uint32 mac_sa:1,
         cvlan:1,
         svlan:1,
         l3_type:1,
         mac_da:1,
         l2_type:1,
         ctag_cos:1,
         stag_cos:1,
         label0:1,
         label1:1,
         label2:1,
         label3:1;
};
typedef struct glb_flow_mpls_rule_flag_s glb_flow_mpls_rule_flag_t;

struct glb_flow_mpls_rule_s
{
   glb_flow_mpls_rule_flag_t flag;

    mac_addr_t mac_sa;
    mac_addr_t mac_sa_mask;
    mac_addr_t mac_da;
    mac_addr_t mac_da_mask;
    uint16 cvlan;
    uint16 svlan;
    uint8  ctag_cos;
    uint8  stag_cos;
    uint8  l3_type;
    uint8  l2_type;

    uint32 label0;
    uint32 label0_mask;
    uint32 label1;
    uint32 label1_mask;
    uint32 label2;
    uint32 label2_mask;
    uint32 label3;
    uint32 label3_mask;
};
typedef struct glb_flow_mpls_rule_s glb_flow_mpls_rule_t;

struct glb_flow_rule_s
{
    glb_flow_rule_type_t rule_type;
    
    uint8 discard;
    uint8 invalid;
    uint8 reserved[2];
    
    union
    {
        glb_flow_mac_rule_t  mac_rule;
        glb_flow_ipv4_rule_t ipv4_rule;
        glb_flow_ipv6_rule_t ipv6_rule;
        glb_flow_mpls_rule_t mpls_rule;
    } rule;
};
typedef struct glb_flow_rule_s glb_flow_rule_t;


#endif /* __GLB_FLOW_DEFINE_H__*/

