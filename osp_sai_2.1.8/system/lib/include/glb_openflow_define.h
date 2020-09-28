#ifndef __GLB_OPENFLOW_DEFINE_H__
#define __GLB_OPENFLOW_DEFINE_H__

#include "ofp_hash.h"
#include "glb_const.h"
#include "glb_if_define.h"
#include "glb_stats_define.h"
#include "glb_tp_oam.h"
#include "glb_ip_define.h"

#define GLB_OPENFLOW_TRUNCATE_MIRROR_SESSION_ID 3
#define GLB_OPENFLOW_MIN_TRUNCATE_LENGTH 64
#define GLB_OPENFLOW_MAX_TRUNCATE_LENGTH 144

/* This port is used to decapsulate GRE tunnel traffic. 
 * Note: This port can only be used for GRE flow */
#define GLB_OFPP_GRE_DECAP  0xfff6

/* This port is used to abstract all the MPLS tunnels which are used to carry L2VPN traffic 
 * in one pseudo port, as our implementation will do next MPLS label lookup(corresponding to 
 * VPWS/VPLS PW label) after the MPLS Tunnel label have been popped. 
 * Note: This port can only be used for MPLS egress flow */
#define GLB_OFPP_PW_FWD     0xfff7

#define GLB_OFPP_IN_PORT    0xfff8 /* Where the packet came in. */
#define GLB_OFPP_TABLE      0xfff9 /* Perform actions in flow table. */
#define GLB_OFPP_NORMAL     0xfffa /* Process with normal L2/L3. */
#define GLB_OFPP_FLOOD      0xfffb /* All ports except input port and
                                            * ports disabled by STP. */
#define GLB_OFPP_ALL        0xfffc /* All ports except input port. */
#define GLB_OFPP_CONTROLLER 0xfffd /* Send to controller. */
#define GLB_OFPP_LOCAL      0xfffe /* Local openflow "port". */
#define GLB_OFPP_NONE       0xffff /* Not associated with any port. */

#define GLB_OPENFLOW_INVALID_INPORT 0 /* OFPORT always > 0 */
#define GLB_OPENFLOW_INVALID_TABLE_ID 255 /* A table id which will not be possible to be used currently. */

#define GLB_INVALID_STATS_ID  0xffffffff /* Invalid stats */
/* 
   should be same as flow max output number, this is limited only by multicast group member
   capability, in GG, its 4096. For memory usage concern especially the big memory required
   by hsrv_openflow_group_s, we limit it to 128 to consider the extreme case that 24Q with
   breakout to 96 10G ports..
*/
#ifdef GREATBELT
#define GLB_MAX_MEMBER_PER_MULTICAST_GROUP 64
#else
#define GLB_MAX_MEMBER_PER_MULTICAST_GROUP 288
#endif
#define GLB_BUCKET_NUM_PER_ALL_GROUP      GLB_MAX_MEMBER_PER_MULTICAST_GROUP
/* should be the minimum between the max ecmp member or link agg member, in GG, it's 16. */
#define GLB_BUCKET_NUM_PER_SELECT_GROUP   16
#define GLB_BUCKET_NUM_PER_INDIRECT_GROUP 1
#define GLB_BUCKET_NUM_PER_FF_GROUP       16
#define GLB_LINKAGG_NUM_PER_SELECT_GROUP  16
#define GLB_FF_NUM_GROUP                  63
#define GLB_SELECT_NUM_GROUP              63
#define GLB_BUCKET_NUM_PER_LSP_APS_GROUP       2
#define GLB_BUCKET_NUM_PER_PW_APS_GROUP       2

#define GLB_FLOW_ENTRY_MAX                0x7FFFFFFF
#define GLB_FLOW_ENTRY_PRIORITY_MAX    0xffffffff

#ifdef USW
#define GLB_SCL_GROUP_FOR_HYBRID  0x7FFFFFFF
#else
#define GLB_SCL_GROUP_FOR_HYBRID  0xFFFF0000
#endif
#define GLB_ECMP_MAX_PATH  64

#define GLB_ACTION_NUM_PER_BUCKET         13
#define GLB_OFPP_STR(ofport)    \
    ((ofport) == GLB_OFPP_IN_PORT ? "in_port" : \
     (ofport) == GLB_OFPP_TABLE ? "table" : \
     (ofport) == GLB_OFPP_NORMAL ? "normal" : \
     (ofport) == GLB_OFPP_FLOOD ? "flood" : \
     (ofport) == GLB_OFPP_ALL ? "all" : \
     (ofport) == GLB_OFPP_CONTROLLER ? "controller" : \
     (ofport) == GLB_OFPP_LOCAL ? "local" : \
     (ofport) == GLB_OFPP_NONE ? "none" : \
     (ofport) == GLB_OFPP_GRE_DECAP ? "gre_decap" : \
     (ofport) == GLB_OFPP_PW_FWD ? "pw_fwd" : \
     "unknown")


#define GLB_BOND_IFINDEX_BASE       0x800
#define GLB_BOND_CHIPID             0x1F 

/* Refer to parser_model, GG support up to 9 labels to be parsed... */
#define GLB_MPLS_ACL_MATCH_LABEL_NUM_MAX 9
#define GLB_MPLS_LABEL_MASK 0xfffff000
#define GLB_MPLS_LABEL_SHIFT 12

/* modified by yaom for openflow MPLS L2VPN 20160214 */
#define GLB_OPF_MPLS_LABEL_RESOURCE_NUM 7000
#ifdef GREATBELT
#define GLB_OPF_MAX_POP_LABEL_VALUE 4095
#else
#define GLB_OPF_MAX_POP_LABEL_VALUE 1048575
#endif

#ifdef GOLDENGATE
#define GLB_OPF_MIN_POP_LABEL_VALUE 16
#endif

#ifdef GREATBELT
#define GLB_OPF_MIN_POP_LABEL_VALUE 1
#endif

#ifdef DUET2
#define GLB_OPF_MIN_POP_LABEL_VALUE 16
#endif

#ifdef TSINGMA
#define GLB_OPF_MIN_POP_LABEL_VALUE 16
#endif

#define GLB_OPF_MPLS_RESERVED_LABEL_NUM 1
#define GLB_OPF_MPLS_MAX_VALID_LABEL_VALUE 0xFFFFF

#define GLB_OPF_MPLS_IPV4_EXPLICIT_NULL 0
#define GLB_OPF_MPLS_IPV4_IMPLICIT_NULL 3

#define GLB_OPF_OAM_PACKET_IN_VERSION 1
#define GLB_OPF_OAM_PACKET_IN_OPTION_DEFAULT 0
#define GLB_OPF_OAM_MAX_NUMBER 1000
#define GLB_OPF_OAM_MIN_NUMBER 1
#define GLB_OPF_OAM_APS_GROUP_MAX_NUMBER 500
#define GLB_OPF_OAM_PACKET_IN_DEFAULT_INTERVAL 15   /*10s*/
#define GLB_OPF_APS_PACKET_IN_DEFAULT_INTERVAL 30   /*30s*/
#define GLB_OPF_OAM_PACKET_IN_BUFFER_CNT  30
#define GLB_OPF_OAM_PACKET_IN_CYCLE_APS_BUFFER_CNT  50
#define GLB_OPF_OAM_PACKET_IN_EVENT_TIME_WAIT   1     /*1s*/
#define GLB_OPF_OAM_PACKET_IN_EVENT_BUFFER_CNT  100
#define GLB_OPF_APS_PACKET_IN_EVENT_BUFFER_CNT  50

#define GLB_OPENFLOW_UDF_MAX_NUM    16

#define GLB_OPENFLOW_IP_TUNNEL_IPSA_MAX  16

#define GLB_OPENFLOW_VXLAN_STRIP_LEN     16
#define GLB_OPENFLOW_MIN_STRIP_LEN       4
#define GLB_OPENFLOW_MAX_STRIP_LEN       32

/* for hybrid FDB-based flow */
#define GLB_OPENFLOW_FLOW_TABLE_ID_ACL   0
#define GLB_OPENFLOW_FLOW_TABLE_ID_FDB   20

#define GLB_OPENFLOW_RESV_NHID_FOR_DROP  1

#define GLB_OPENFLOW_FDB_FLOW_MAX_NUM    10000

/*added by hansf for ttp */

#define GLB_OPENFLOW_TTP_GROUP_TYPE(x)   ((x) >> 28)

#define GLB_OPENFLOW_TTP_MAX_BUCKET_PER_FLOOD_GROUP 288
#define GLB_OPENFLOW_TTP_MAX_BUCKET_PER_ECMP_GROUP 16
#define GLB_OPENFLOW_TTP_MAX_BUCKET_PER_INDIRECT_GROUP 1

#define GLB_OPENFLOW_CONTROLLER_TARGET_MAXLEN  100

enum glb_openflow_ttp_group_type_e
{
  /** Group type L2 Interface */
  GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE = 0,
  /** Group type L3 Unicast */
  GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST   = 2,
  /** Group type L2 Flood */
  GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD     = 4,
  /** Group type L3 ECMP */
  GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP      = 7,

  GLB_OPENFLOW_TTP_GROUP_TYPE_MAX,

};

typedef enum
{
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_PORT                      =    0,  /**< Ingress Port Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN                              =   10,  /**< VLAN Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC                   =   20,  /**< Termination MAC Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING                   =   30,  /**< Unicast Routing Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING                          =   50,  /**< Bridging Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL                       =   60,  /**< Ingress ACL Table */
  GLB_OPENFLOW_TTP_FLOW_TABLE_ID_EGRESS_ACL                       =   61,  /**< Egress ACL Table */
} OPENFLOW_TTP_FLOW_TABLE_ID_t;



enum glb_openflow_match_field_type_e
{
    GLB_OPENFLOW_MATCH_FIELD_IN_PORT,
    GLB_OPENFLOW_MATCH_FIELD_METADATA,
    GLB_OPENFLOW_MATCH_FIELD_ETH_SRC_MAC,
    GLB_OPENFLOW_MATCH_FIELD_ETH_DST_MAC,
    GLB_OPENFLOW_MATCH_FIELD_ETH_TYPE,
    GLB_OPENFLOW_MATCH_FIELD_VLAN,
    GLB_OPENFLOW_MATCH_FIELD_VLAN_PCP,
    GLB_OPENFLOW_MATCH_FIELD_CVLAN,
    GLB_OPENFLOW_MATCH_FIELD_IP_DSCP,
    GLB_OPENFLOW_MATCH_FIELD_IP_ECN,
    GLB_OPENFLOW_MATCH_FIELD_IP_FRAG,
    GLB_OPENFLOW_MATCH_FIELD_IP_PROTOCOL,
    GLB_OPENFLOW_MATCH_FIELD_IP_SRC_IP,
    GLB_OPENFLOW_MATCH_FIELD_IP_DST_IP,
    GLB_OPENFLOW_MATCH_FIELD_L4_SRC_PORT,
    GLB_OPENFLOW_MATCH_FIELD_L4_DST_PORT,
    GLB_OPENFLOW_MATCH_FIELD_ICMP_TYPE,
    GLB_OPENFLOW_MATCH_FIELD_ICMP_CODE,
    GLB_OPENFLOW_MATCH_FIELD_ARP_OP_CODE,
    GLB_OPENFLOW_MATCH_FIELD_ARP_SPA,
    GLB_OPENFLOW_MATCH_FIELD_ARP_TPA,
    GLB_OPENFLOW_MATCH_FIELD_TUNNEL_ID,
    GLB_OPENFLOW_MATCH_FIELD_MPLS,
    GLB_OPENFLOW_MATCH_FIELD_L3_UDF,
    GLB_OPENFLOW_MATCH_FIELD_L4_UDF,
    GLB_OPENFLOW_MATCH_FIELD_TRUNCATION,
    GLB_OPENFLOW_MATCH_FIELD_MAX
};

enum glb_openflow_tunnel_del_type_e
{
    GLB_OPENFLOW_TUNNEL_OVS_DEL,
    GLB_OPENFLOW_TUNNEL_RUNNING_DOWN
};
typedef enum glb_openflow_tunnel_del_type_e glb_openflow_tunnel_del_type_t;

/*end hansf*/

enum glb_openflow_flow_tnl_type_e
{
    GLB_OPENFLOW_FLOW_TNL_TYPE_NONE,
    GLB_OPENFLOW_FLOW_TNL_TYPE_V4_TO_V6,
    GLB_OPENFLOW_FLOW_TNL_TYPE_V6_TO_V4,
    
    GLB_OPENFLOW_FLOW_TNL_TYPE_MAX
};
enum glb_openflow_flow_payload_packet_type_e
{
    GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_NONE,
    GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_TCP,
    GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_UDP,
    
    GLB_OPENFLOW_FLOW_PAYLOAD_PACKET_TYPE_MAX
};

/*added by hansf for support vlan action before group action in flow*/
enum glb_flow_group_vlan_type_e
{
    GLB_GROUP_VLAN_PUSH = 1<<0,
    GLB_GROUP_VLAN_SET  = 1<<1,
    GLB_GROUP_VLAN_POP  = 1<<2,
    GLB_GROUP_VLAN_SET_PCP  = 1<<3
};

enum glb_flow_default_flow_id_e
{
    GLB_FLOW_ID_DEFAULT_MIN = 0x10000000,

    /* Default profile default entry's entry id */
    /* NOTICE: DEFAULT FLOW ID REDEFINED in adapt_flow.h, keep synced */
    GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS,
    GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_MPLS_MCAST,
    GLB_FLOW_ID_DEFAULT_PROFILE_TABLE0_DEFAULT,

    GLB_FLOW_ID_DEFAULT_MAX
};

/* Added by weizj for tunnel bug 33692 */
enum glb_openflow_flow_priority_e
{
    /* 1 is for sdk default entry */
    GLB_OPENFLOW_FLOW_DEFAULT_ENTRY_PRIORITY = 2,
    GLB_OPENFLOW_FLOW_SLOW_PROTO_ENTRY_PRIORITY = 3,
    GLB_OPENFLOW_FLOW_TABLE_MISS_ENTRY_PRIORITY = 4,
    GLB_OPENFLOW_FLOW_NORMAL_ENTRY_PRIORITY_OFFSET = 5,
};

enum glb_openflow_mac_rule_flag_e
{
    GLB_OPENFLOW_MAC_RULE_FLAG_MACSA       = 1 << 0,
    GLB_OPENFLOW_MAC_RULE_FLAG_MACDA       = 1 << 1,
    GLB_OPENFLOW_MAC_RULE_FLAG_STAG_VALID  = 1 << 2,
    GLB_OPENFLOW_MAC_RULE_FLAG_SVLAN_ID    = 1 << 3,
    GLB_OPENFLOW_MAC_RULE_FLAG_STAG_COS    = 1 << 4,
    GLB_OPENFLOW_MAC_RULE_FLAG_ETHTYPE     = 1 << 5,
    GLB_OPENFLOW_MAC_RULE_FLAG_IPV4_PKT    = 1 << 6,
    GLB_OPENFLOW_MAC_RULE_FLAG_ARP_PKT     = 1 << 7,
    GLB_OPENFLOW_MAC_RULE_FLAG_MPLS_PKT     = 1 << 8,
    GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_VALID  = 1 << 9,
    GLB_OPENFLOW_MAC_RULE_FLAG_CVLAN_ID    = 1 << 10,
    GLB_OPENFLOW_MAC_RULE_FLAG_CTAG_COS    = 1 << 11,
};

enum glb_openflow_arp_rule_flag_e
{
    GLB_OPENFLOW_ARP_RULE_FLAG_ARP_OPCODE = 1 << 0,
    GLB_OPENFLOW_ARP_RULE_FLAG_SENDER_IP  = 1 << 1,
    GLB_OPENFLOW_ARP_RULE_FLAG_TARGET_IP  = 1 << 2,
};

enum glb_openflow_ipv4_rule_flag_e
{
    GLB_OPENFLOW_IPV4_RULE_FLAG_IPSA           = 1 << 0,
    GLB_OPENFLOW_IPV4_RULE_FLAG_IPDA           = 1 << 1,
    GLB_OPENFLOW_IPV4_RULE_FLAG_L4_PROTO       = 1 << 2,
    GLB_OPENFLOW_IPV4_RULE_FLAG_L4_SRC_PORT    = 1 << 3,
    GLB_OPENFLOW_IPV4_RULE_FLAG_L4_DST_PORT    = 1 << 4,
    GLB_OPENFLOW_IPV4_RULE_FLAG_TCP_FLAG       = 1 << 5,
    GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_TYPE      = 1 << 6,
    GLB_OPENFLOW_IPV4_RULE_FLAG_IGMP_TYPE      = 1 << 7,
    GLB_OPENFLOW_IPV4_RULE_FLAG_ICMP_CODE      = 1 << 8,
    GLB_OPENFLOW_IPV4_RULE_FLAG_DSCP           = 1 << 9,
    GLB_OPENFLOW_IPV4_RULE_FLAG_IP_FRAG        = 1 << 10,
    GLB_OPENFLOW_IPV4_RULE_FLAG_ECN            = 1 << 11,
};


enum glb_openflow_ipv6_rule_flag_e
{
    GLB_OPENFLOW_IPV6_RULE_FLAG_IPSA           = 1 << 0,
    GLB_OPENFLOW_IPV6_RULE_FLAG_IPDA           = 1 << 1,
    GLB_OPENFLOW_IPV6_RULE_FLAG_L4_PROTO       = 1 << 2,
    GLB_OPENFLOW_IPV6_RULE_FLAG_L4_SRC_PORT    = 1 << 3,
    GLB_OPENFLOW_IPV6_RULE_FLAG_L4_DST_PORT    = 1 << 4,
    GLB_OPENFLOW_IPV6_RULE_FLAG_TCP_FLAG       = 1 << 5,
    GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_TYPE      = 1 << 6,
    GLB_OPENFLOW_IPV6_RULE_FLAG_ICMP_CODE      = 1 << 7,
    GLB_OPENFLOW_IPV6_RULE_FLAG_DSCP           = 1 << 9,
    GLB_OPENFLOW_IPV6_RULE_FLAG_IP_FRAG        = 1 << 10,
    GLB_OPENFLOW_IPV6_RULE_FLAG_FLOW_LABEL        = 1 << 11,
    GLB_OPENFLOW_IPV6_RULE_FLAG_ECN        = 1 << 11,
};

enum glb_openflow_mpls_rule_flag_e
{
    GLB_OPENFLOW_MPLS_RULE_FLAG_MACSA       = 1 << 0,
    GLB_OPENFLOW_MPLS_RULE_FLAG_MACDA       = 1 << 1,
    GLB_OPENFLOW_MPLS_RULE_FLAG_CVLAN_ID    = 1 << 2,
    GLB_OPENFLOW_MPLS_RULE_FLAG_SVLAN_ID    = 1 << 3,
    GLB_OPENFLOW_MPLS_RULE_FLAG_CTAG_COS    = 1 << 4,
    GLB_OPENFLOW_MPLS_RULE_FLAG_STAG_COS    = 1 << 5,
    GLB_OPENFLOW_MPLS_RULE_FLAG_L2TYPE      = 1 << 6,
    GLB_OPENFLOW_MPLS_RULE_FLAG_LABEL0      = 1 << 7,
    GLB_OPENFLOW_MPLS_RULE_FLAG_LABEL1      = 1 << 8,
    GLB_OPENFLOW_MPLS_RULE_FLAG_LABEL2      = 1 << 9,
    GLB_OPENFLOW_MPLS_RULE_FLAG_LABEL3      = 1 << 10
};

/* Added by weizj for tunnel */
enum glb_openflow_tnl_rule_flag_e
{
    GLB_OPENFLOW_TNL_RULE_FLAG_METADATA     = 1 << 0,
    GLB_OPENFLOW_TNL_RULE_FLAG_TUN_ID     = 1 << 1
};

/* Added by weizj for udf */
enum glb_openflow_udf_flag_e
{
    GLB_OPENFLOW_UDF_TYPE_L3                = 1 << 0,
    GLB_OPENFLOW_UDF_TYPE_L4                = 1 << 1,
    GLB_OPENFLOW_UDF_ETHTYPE                = 1 << 2,
    GLB_OPENFLOW_UDF_L3_PROTOCOL            = 1 << 3,
    GLB_OPENFLOW_UDF_IP_VERSION             = 1 << 4,
    GLB_OPENFLOW_UDF_L4_SRC                 = 1 << 5,
    GLB_OPENFLOW_UDF_L4_DST                 = 1 << 6,
    GLB_OPENFLOW_UDF_OFFSET0                = 1 << 7,
    GLB_OPENFLOW_UDF_OFFSET1                = 1 << 8,
    GLB_OPENFLOW_UDF_OFFSET2                = 1 << 9,
    GLB_OPENFLOW_UDF_OFFSET3                = 1 << 10,
};
/* End by weizj for udf */

/**************************  flow type ****************************************/

enum glb_openflow_flow_type_e
{
    GLB_OF_FLOW_RULE_TYPE_MAC,          /* MAC flow rule type */
    GLB_OF_FLOW_RULE_TYPE_ARP,          /* ARP flow rule type */
    GLB_OF_FLOW_RULE_TYPE_IPV4,         /* IPv4 flow rule type */
    GLB_OF_FLOW_RULE_TYPE_IPV6,         /* IPv6 flow filter type */
    GLB_OF_FLOW_RULE_TYPE_MPLS,         /* MPLS flow rule type */
    GLB_OF_FLOW_RULE_TYPE_MPLS_MCAST,   /* MPLS mcast flow rule type */
                                        /* Deleted by weizj for tunnel bug 33692 */
    GLB_OF_FLOW_RULE_TYPE_MAX
};
typedef enum glb_openflow_flow_type_e glb_openflow_flow_type_t;

enum glb_openflow_flow_profile_type_e {
    GLB_OPENFLOW_FLOW_PROFILE_DEFAULT, /* Currently only default is supported. */
    GLB_OPENFLOW_FLOW_PROFILE_NVCLOUD,
    
    GLB_OPENFLOW_FLOW_PROFILE_MAX
};
typedef enum glb_openflow_flow_profile_type_e glb_openflow_flow_profile_type_t;

/**************************  in/out_port ****************************************/
enum glb_openflow_port_type_e
{
    GLB_OF_PORT_TYPE_NONE = 0,
    GLB_OF_PORT_TYPE_PHYSICAL,
    GLB_OF_PORT_TYPE_TUNNEL,
    GLB_OF_PORT_TYPE_BOND,
    GLB_OF_PORT_TYPE_AGG,
    GLB_OF_PORT_TYPE_OF_RSVD,

    GLB_OF_INPORT_TYPE_MAX,
};
typedef enum glb_openflow_port_type_e glb_openflow_port_type_t;

enum glb_openflow_oam_event_type_e
{
    GLB_OF_OAM_EVENT_TYPE_NONE = 0,  /*cycle*/
    GLB_OF_OAM_EVENT_TYPE_CMM,
    GLB_OF_OAM_EVENT_TYPE_LM_OVER_THRESHOLD,
    GLB_OF_OAM_EVENT_TYPE_DM_OVER_THRESHOLD,
    GLB_OF_OAM_EVENT_TYPE_LM_RETURN_NORMAL,
    GLB_OF_OAM_EVENT_TYPE_DM_RETURN_NORMAL,

    GLB_OF_OAM_EVENT_TYPE_MAX,
};
typedef enum glb_openflow_oam_event_type_e glb_openflow_oam_event_type_t;

enum glb_openflow_aps_event_type_e
{
    GLB_OF_APS_EVENT_TYPE_NONE = 0,  /*cycle*/
    GLB_OF_APS_EVENT_TYPE_PATH_CHANGE,

    GLB_OF_APS_EVENT_TYPE_MAX,
};
typedef enum glb_openflow_aps_event_type_e glb_openflow_aps_event_type_t;

enum glb_openflow_oam_packet_type_e
{
    GLB_OF_OAM_PACKET_TYPE_OAM_SESSION = 1,
    GLB_OF_OAM_PACKET_TYPE_APS_GROUP,

    GLB_OF_OAM_PACKET_TYPE_MAX,
};
typedef enum glb_openflow_oam_packet_type_e glb_openflow_oam_packet_type_t;

struct glb_openflow_port_s
{
    glb_openflow_port_type_t type;
    uint32 ifindex;
    uint16 ofport;
    uint16 max_len;     /* valid only when port == controller */
    uint32 tid;
};
typedef struct glb_openflow_port_s glb_openflow_inport_t;
typedef struct glb_openflow_port_s glb_openflow_output_t;

/**************************  MAC key ****************************************/
struct glb_openflow_mac_key_flag_s
{
  uint32 mac_sa:1,
         mac_da:1,
         svlan:1,
         stag_cos:1,
         stag_valid:1,
         l2_type:1,
         l3_type:1,
         eth_type:1,
         arp_packet:1,
         ipv4_packet:1,
         mpls_packet:1,
         cvlan:1,
         ctag_cos:1,
         ctag_valid:1,
         reserved:18;
};
typedef struct glb_openflow_mac_key_flag_s glb_openflow_mac_key_flag_t;

struct glb_openflow_mac_key_s
{
    glb_openflow_mac_key_flag_t flags;
    
    /* mac fields */
    mac_addr_t mac_sa;
    mac_addr_t mac_sa_mask;
    mac_addr_t mac_da;
    mac_addr_t mac_da_mask;
    uint16 svlan;
    uint16 svlan_mask;
    uint8  stag_cos;
    uint8  stag_cos_mask;
    uint8  stag_valid;
    uint8  arp_packet;
    uint8  ipv4_packet;
    uint8  mpls_packet;
    uint16 rsv0;
    uint16 eth_type;
    uint16 eth_type_mask;
    uint16 cvlan;
    uint16 cvlan_mask;
    uint8  ctag_cos;
    uint8  ctag_cos_mask;
    uint8  ctag_valid;

    /*added by hansf for ttp(bridge table)*/
    uint64 metadata;
    
};
typedef struct glb_openflow_mac_key_s glb_openflow_mac_key_t;

/**************************  ARP key ****************************************/
struct glb_openflow_arp_key_flag_s
{
  uint32 sender_ip:1,
         target_ip:1,
         arp_op:1,
         reserved:29;
};
typedef struct glb_openflow_arp_key_flag_s glb_openflow_arp_key_flag_t;

struct glb_openflow_arp_key_s
{
    glb_openflow_arp_key_flag_t flags;

    uint32 sender_ip;
    uint32 sender_ip_mask;
    uint32 target_ip;
    uint32 target_ip_mask;
    
    uint8  arp_op;
    uint8  arp_op_mask;
    uint8  resv[2];
};
typedef struct glb_openflow_arp_key_s glb_openflow_arp_key_t;


/**************************  IPV4 key ****************************************/
struct glb_openflow_ipv4_rule_flag_s
{
    uint32 ip_sa:1,
           ip_da:1,
           l4_protocol:1,
           l4_source_port:1,
           l4_dest_port:1,
           tcp_flags:1,
           icmp_type:1,
           icmp_code:1,
           dscp:1,
           ecn:1,
           ip_frag:1,
           reserved:21;
};
typedef struct glb_openflow_ipv4_rule_flag_s glb_openflow_ipv4_rule_flag_t;

struct glb_openflow_ipv6_rule_flag_s
{
    uint32 ip_sa:1,
           ip_da:1,
           l4_protocol:1,
           l4_source_port:1,
           l4_dest_port:1,
           tcp_flags:1,
           icmp_type:1,
           icmp_code:1,
           ecn:1,
           ip_frag:1,
           flow_label:1,
           dscp:1,
           reserved:19;
};
typedef struct glb_openflow_ipv6_rule_flag_s glb_openflow_ipv6_rule_flag_t;

struct glb_openflow_l4_port_s
{
    uint16 port;
    uint16 port_mask;
};
typedef struct glb_openflow_l4_port_s glb_openflow_l4_port_t;

struct glb_openflow_ipv4_key_s
{
    glb_openflow_ipv4_rule_flag_t flags;
    
    uint32 ip_sa;
    uint32 ip_sa_mask;
    uint32 ip_da;
    uint32 ip_da_mask;
    glb_openflow_l4_port_t l4_source_port;
    glb_openflow_l4_port_t l4_dest_port;
    uint8  dscp;
    uint8  dscp_mask;
    uint8  ecn;
    uint8  ecn_mask;

    uint8  icmp_type;
    uint8  icmp_code;
    uint8  ip_frag;
    uint8  ip_frag_mask;

    uint8  l4_protocol;
    uint8  resv;
    uint16 tcp_flags;

    /*added by hansf for ttp*/
    uint64  metadata;
};
typedef struct glb_openflow_ipv4_key_s glb_openflow_ipv4_key_t;


/**************************  IPV6 key ****************************************/
struct glb_openflow_ipv6_key_s
{
    glb_openflow_ipv6_rule_flag_t flags;                  

    ipv6_addr_t ip_sa;                  
    ipv6_addr_t ip_sa_mask;            
    ipv6_addr_t ip_da;                 
    ipv6_addr_t ip_da_mask;            

    glb_openflow_l4_port_t l4_source_port;
    glb_openflow_l4_port_t l4_dest_port;
    
    uint32 flow_label;                  
    uint32 flow_label_mask;            

    uint8  l4_protocol;               
    uint8  l4_protocol_mask;           
    uint8  icmp_type;                   
    uint8  icmp_type_mask;              

    uint8  icmp_code;                   
    uint8  icmp_code_mask;   
    uint8  ip_frag;
    uint8  ip_frag_mask;
    uint8  dscp;                        
    uint8  dscp_mask;  
    uint8  ecn;
    uint8  ecn_mask;
};
typedef struct glb_openflow_ipv6_key_s glb_openflow_ipv6_key_t;


/**************************  MPLS key ****************************************/
struct glb_openflow_mpls_key_s
{
    uint8 is_acl_based;
    union 
    {
        struct 
        {
            uint32 mpls_label; /* for MPLS L2VPN, TODO: support it by weizj. */
        } ilm; /* MPLS L2VPN usecase we actually use MPLS ILM to support MPLS instead of MPLS ACL. */
        struct 
        {
            uint8 mpls_label_num; /* ACL mask is nonsense for MPLS, same for label field.  */
            uint8 rsv0;
            uint16 rsv1;
            uint32 mpls_label0;/* 32bit in host byte order, contain BoS/TC/TTL for later extension. */
            uint32 mpls_label0_mask;
            uint32 mpls_label1;
            uint32 mpls_label1_mask;
            uint32 mpls_label2;
            uint32 mpls_label2_mask;
        } acl; /* for cubro MPLS, use MPLS ACL for match MPLS packet. */
    } u;
};
typedef struct glb_openflow_mpls_key_s glb_openflow_mpls_key_t;

/**************************  TUNNEL key ****************************************/
/* Added by weizj for tunnel */
enum glb_openflow_tunnel_port_type_e
{
    GLB_OPENFLOW_TUNNEL_TYPE_NONE,
    GLB_OPENFLOW_TUNNEL_TYPE_L2GRE,
    GLB_OPENFLOW_TUNNEL_TYPE_NVGRE,
    GLB_OPENFLOW_TUNNEL_TYPE_VXLAN,
    GLB_OPENFLOW_TUNNEL_TYPE_ALL, /* modified by liwh for bug 48966, 2019-02-26 */
    GLB_OPENFLOW_TUNNEL_TYPE_MAX
};
typedef enum glb_openflow_tunnel_port_type_e glb_openflow_tunnel_port_type_t;

struct glb_openflow_tnl_key_s
{
    glb_openflow_tunnel_port_type_t type;

    uint32 ifindex;
    uint16 metadata;
    uint32 tun_id;
    uint32 tun_id_mask;

    uint32 ipSa;
    uint32 ipDa;
};
typedef struct glb_openflow_tnl_key_s glb_openflow_tnl_key_t;

/**************************  UDF key ****************************************/
/* Added by weizj for udf */
#define GLB_OPENFLOW_UDF_PARSER_LEN 4

struct glb_openflow_udf_key_s
{
#if 0
    uint32 flags;
    uint32 l3_udf;
    uint32 l3_udf_mask;
    uint32 l4_udf;
    uint32 l4_udf_mask;
#endif
    uint8 udf_enable;
    uint32 udf_id;  /* for 580 system, udf_id:0->l3_type, 1->l4_type */
    uint32 udf0;    /* for 580 system, just udf0 and udf0_mask is valid */
    uint32 udf0_mask;
    uint32 udf1;
    uint32 udf1_mask;
    uint32 udf2;
    uint32 udf2_mask;
    uint32 udf3;
    uint32 udf3_mask;
};
typedef struct glb_openflow_udf_key_s glb_openflow_udf_key_t;

/**************************  flow key ****************************************/
struct glb_openflow_flow_key_s
{
    glb_openflow_flow_type_t rule_type;

    glb_openflow_inport_t in_port;
    glb_openflow_mac_key_t mac;
    glb_openflow_tnl_key_t tnl;/* Added by weizj for tunnel */
    glb_openflow_udf_key_t udf;/* Added by weizj for udf */

    union
    {
        glb_openflow_ipv4_key_t ipv4_rule;
        glb_openflow_arp_key_t  arp_rule;
        glb_openflow_ipv6_key_t ipv6_rule;
        glb_openflow_mpls_key_t mpls_rule;
    } rule;
};
typedef struct glb_openflow_flow_key_s glb_openflow_flow_key_t;

/**************************  flow action ****************************************/
/* This will also be used to restrict the action number before output action, which is used by adpt_mpls. */
#define GLB_OPENFLOW_MAX_ACTION_COUNT 256

/* Limited by the CHIP multicast replication capability. */
#define GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW GLB_MAX_MEMBER_PER_MULTICAST_GROUP
#define GLB_OPENFLOW_MAX_OUTPUT_PER_GROUP GLB_MAX_MEMBER_PER_MULTICAST_GROUP

enum glb_openflow_action_type_e
{
    /* value in output. */
    GLB_OPENFLOW_ACTION_TYPE_OUTPUT,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_PUSH_L2,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_POP_L2,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_POP_MPLS,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL,
    /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC,
    /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC,
    /* Note: In OF 1.3 Spec, unlike MPLS label and TC, MPLS TTL can't be set in
     * set_field action, there is a dedicated action OFPAT_SET_MPLS_TTL,
     * value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_GROUP,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_INST_METER,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE,
    /* value in m. */
    GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_DEC_TTL,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL,
    /* no value. */
    GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL,
    /* value in ipv6. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC,
    /* value in ipv6. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA,
    /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA,
    /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA,
    /* value in u16. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE,
    /* value in u8. */
    GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_OAM_INLABEL,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_OAM_POPLABEL,
    /* value in u32. */
    GLB_OPENFLOW_ACTION_TYPE_SET_STRIP_HEADER,
    /*no value*/
    GLB_OPENFLOW_ACTION_TYPE_CLEAR_ACTIONS,
    /* added by hansf for v4/v6 exchange, value in u8 */
    GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_FLOW_TYPE,
    GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_PACKET_TYPE,

    GLB_OPENFLOW_ACTION_TYPE_UNSUPPORTED,
};
typedef enum glb_openflow_action_type_e glb_openflow_action_type_t;      

enum glb_openflow_action_type_next_e
{
        /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_SRC,
    /* value in mac. */
    GLB_OPENFLOW_ACTION_TYPE_NEXT_SET_INNER_ETH_DST,
};
typedef enum glb_openflow_action_type_next_e glb_openflow_action_type_next_t;      
        
struct glb_openflow_metadata_s
{
    uint64 metadata;
    uint64 mask;
};
typedef struct glb_openflow_metadata_s glb_openflow_metadata_t;

#define GLB_OPENFLOW_ACTION_TYPE_TO_STR(type)                              \
    ((type == GLB_OPENFLOW_ACTION_TYPE_OUTPUT )       ? "OUTPUT"         : \
     (type == GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN )    ? "PUSH_VLAN"      : \
     (type == GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN )   ? "STRIP_VLAN"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_PUSH_L2 )      ? "PUSH_L2"        : \
     (type == GLB_OPENFLOW_ACTION_TYPE_POP_L2 )       ? "POP_L2"         : \
     (type == GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS )    ? "PUSH_MPLS"      : \
     (type == GLB_OPENFLOW_ACTION_TYPE_POP_MPLS )     ? "POP_MPLS"       : \
     (type == GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS ) ? "POP_ALL_MPLS"       : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL )   ? "TUNNEL"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC )  ? "ETH_SRC"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST )  ? "ETH_DST"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP ) ? "VLAN_PCP"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID ) ? "VLAN_VID"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL)? "MPLS_LBL"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC )  ? "MPLS_TC"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL ) ? "MPLS_TTL"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_GROUP )        ? "GROUP"          : \
     (type == GLB_OPENFLOW_ACTION_TYPE_INST_METER)    ? "METER"          : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE)     ? "QUEUE"      : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC)  ? "IPV4_SRC"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST)  ? "IPV4_DST"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC)    ? "TP_SRC"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST)    ? "TP_DST"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC)   ? "TCP_SRC"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST)   ? "TCP_DST"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC)   ? "UDP_SRC"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST)   ? "UDP_DST"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC)  ? "SCTP_SRC"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST)  ? "SCTP_DST"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL)    ? "IP_TTL"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE)    ? "GOTO_TABLE" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA)? "METADATA"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN)    ? "IP_ECN"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP)   ? "IP_DSCP"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_DEC_TTL)       ? "DEC_TTL"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL)    ? "IP_TTL"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL)  ? "DEC_MPLS_TTL" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC)  ? "IPV6_SRC"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST)  ? "IPV6_DST"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL)? "IPV6LABEL" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP)    ? "ARP_OP"     : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA)   ? "ARP_SPA"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA)   ? "ARP_TPA"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA)   ? "ARP_SHA"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA)   ? "ARP_THA"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE)  ? "ETH_TYPE"   : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE) ? "ICMPV4_TYPE" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE) ? "ICMPV4_CODE" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE) ? "ICMPV6_TYPE" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE) ? "ICMPV6_CODE" : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO)    ? "IP_PROTO"    : \
     (type == GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE )   ? "TRUNCATE"    : \
     "UNKNOWN")

/*
 * Action fields are used for combo action in hsrv/hagt
 */
#define GLB_DEFINE_OFACT_FLD                         \
    DEFINE_OFACT_FLD(OUTPUT,        "output"       ) \
    DEFINE_OFACT_FLD(TUNNEL_ID,     "tunnel_id"    ) \
    DEFINE_OFACT_FLD(METER,         "meter"        ) \
    DEFINE_OFACT_FLD(QUEUE_ID,      "queue_id"     ) \
    DEFINE_OFACT_FLD(MACSA,         "macsa"        ) \
    DEFINE_OFACT_FLD(MACDA,         "macda"        ) \
    DEFINE_OFACT_FLD(SVLAN_VID,     "svid"         ) \
    DEFINE_OFACT_FLD(SVLAN_COS,     "scos"         ) \
    DEFINE_OFACT_FLD(CVLAN_VID,     "cvid"         ) \
    DEFINE_OFACT_FLD(CVLAN_COS,     "ccos"         ) \
    DEFINE_OFACT_FLD(STRIP_SVLAN,   "strip_svlan"  ) \
    DEFINE_OFACT_FLD(STRIP_CVLAN,   "strip_cvlan"  ) \
    DEFINE_OFACT_FLD(PUSH_SVLAN,    "push_svlan"   ) \
    DEFINE_OFACT_FLD(PUSH_CVLAN,    "push_cvlan"   ) \
    DEFINE_OFACT_FLD(ETH_TYPE,      "eth_type"     ) \
    DEFINE_OFACT_FLD(IPV4_,         "ipv4"         ) \
    DEFINE_OFACT_FLD(IPV6_,         "ipv6"         ) \
    DEFINE_OFACT_FLD(ARP_,          "arp"          ) \
    DEFINE_OFACT_FLD(MPLS_,         "mpls"         ) \
    DEFINE_OFACT_FLD(TCP_,          "tcp"          ) \
    DEFINE_OFACT_FLD(UDP_,          "udp"          ) \
    DEFINE_OFACT_FLD(SCTP_,         "sctp"         ) \
    DEFINE_OFACT_FLD(IPSA,          "ipda"         ) \
    DEFINE_OFACT_FLD(IPDA,          "ipda"         ) \
    DEFINE_OFACT_FLD(SPORT,         "sport"        ) \
    DEFINE_OFACT_FLD(DPORT,         "dport"        ) \
    DEFINE_OFACT_FLD(IP_TTL_,       "ip_ttl"       ) \
    DEFINE_OFACT_FLD(IP_DSCP,       "ip_dscp"      ) \
    DEFINE_OFACT_FLD(IP_ECN,        "ip_ecn"       ) \
    DEFINE_OFACT_FLD(IP_PROTO,      "ip_proto"     ) \
    DEFINE_OFACT_FLD(DEC_IP_TTL,    "dec_ip_ttl"   ) \
    DEFINE_OFACT_FLD(DEC_TTL,       "dec_ttl"      ) \
    DEFINE_OFACT_FLD(IPV6_LBL,      "label"        ) \
    DEFINE_OFACT_FLD(ARP_OP,        "op"           ) \
    DEFINE_OFACT_FLD(ARP_SPA,       "spa"          ) \
    DEFINE_OFACT_FLD(ARP_TPA,       "tpa"          ) \
    DEFINE_OFACT_FLD(ARP_SHA,       "sha"          ) \
    DEFINE_OFACT_FLD(ARP_THA,       "tha"          ) \
    DEFINE_OFACT_FLD(ICMP_TYPE,     "icmp_type"    ) \
    DEFINE_OFACT_FLD(ICMP_CODE,     "icmp_code"    )
#define GLB_OFPACT_FLD(FLD) \
    GLB_OPENFLOW_ACTION_FIELD_##FLD

/* action field enum */
enum glb_openflow_action_field_e
{
#define DEFINE_OFACT_FLD(FLD, STR)     \
    GLB_OPENFLOW_ACTION_FIELD_##FLD,
    GLB_DEFINE_OFACT_FLD
#undef DEFINE_OFACT_FLD
    GLB_OPENFLOW_ACTION_FIELD_MAX,
};
typedef enum glb_openflow_action_field_e glb_openflow_action_field_t;
#define GLB_OFPACT_FLD_FLAG_NUM ((GLB_OPENFLOW_ACTION_FIELD_MAX+31)/32)

/* action field bitmap operation */
#define GLB_OFPACT_FLD_SET(flags, FLD)\
    GLB_SET_BIT(flags[GLB_OFPACT_FLD(FLD)/32], GLB_OFPACT_FLD(FLD)%32)

#define GLB_OFPACT_FLD_UNSET(flags, FLD)\
    GLB_CLEAR_BIT(flags[GLB_OFPACT_FLD(FLD)/32], GLB_OFPACT_FLD(FLD)%32)

#define GLB_OFPACT_FLD_ISSET(flags, FLD)\
    GLB_IS_BIT_SET(flags[GLB_OFPACT_FLD(FLD)/32], GLB_OFPACT_FLD(FLD)%32)

enum glb_openflow_mpls_nexthop_type_e
{
    GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L2VPN,
    GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_PUSH_L3VPN,

    /* modified by yaom for openflow MPLS L2VPN 20160214 */
    GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_SWAP_MPLS,
    GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_POP_PHP,

    GLB_OPENFLOW_MPLS_NEXTHOP_TYPE_MAX,
};
typedef enum glb_openflow_mpls_nexthop_type_e glb_openflow_mpls_nexthop_type_t;

enum glb_openflow_mpls_nexthop_tag_op_e
{
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_NONE,
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_INSERT,
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_STRIP,
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_REPLACE,
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_KEEP_UNCHANGE,
        
    GLB_OPENFLOW_MPLS_NEXTHOP_TAG_OP_MAX
};
typedef enum glb_openflow_mpls_nexthop_tag_op_e glb_openflow_mpls_nexthop_tag_op_t;


enum glb_openflow_mpls_output_action_seq_type_e
{
    /* for cubro mpls. */
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_0 = 0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_1,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_0, /* keep original vlan unchanged. */
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_1, /* strip outer vlan tag. */

    /*These macro is the index of valid_action_seq_with_mpls_prior_output*/
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1,
    /*4 type corresponding to different customer payload vlan tag rewrite behavior*/
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_1,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_2,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_3,
    /*4 type corresponding to different customer payload vlan tag rewrite behavior*/
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_0,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_1,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_2,
    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_3,

    GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_VALID_MAX,
};
typedef enum glb_openflow_mpls_output_action_seq_type_e glb_openflow_mpls_output_action_seq_type_t;

/* for cubro mpls. */
#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_POP_ALL_MPLS(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_1 == (mpls_output_action_seq_no)))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_TAG_PACKET_IN_L2VPN(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_1 == (mpls_output_action_seq_no)))

/*The pop mpls rule need install and ILM(incoming label mapping) to ASIC*/
#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_MPLS_EGRESS_RULE(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1 == (mpls_output_action_seq_no)) || \
     ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_0 <= (mpls_output_action_seq_no)) && \
      (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_3 >= (mpls_output_action_seq_no))))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_MPLS_PUSH_RULE(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0 == (mpls_output_action_seq_no)) || \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1 == (mpls_output_action_seq_no)) || \
     ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_0 <= (mpls_output_action_seq_no)) && \
      (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_3 >= (mpls_output_action_seq_no))))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_NORMAL_MPLS_PUSH(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0 == (mpls_output_action_seq_no)))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_NORMAL_MPLS(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0 <= (mpls_output_action_seq_no)) && \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1 >= (mpls_output_action_seq_no)))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_L2VPN_INGRESS(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_0 <= (mpls_output_action_seq_no)) && \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_3 >= (mpls_output_action_seq_no)))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_IS_L2VPN_EGRESS(mpls_output_action_seq_no) \
    ((GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_0 <= (mpls_output_action_seq_no)) && \
     (GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_3 >= (mpls_output_action_seq_no)))

#define GLB_OPENFLOW_MPLS_ACTION_SEQ_TO_STR(seq_no) \
    ((seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_0) ? "pop_all_mpls" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_POP_ALL_MPLS_1) ? "pop_all_mpls(push_l2)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_0) ? "tag_pkt_in_l2vpn(keep_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_TAG_PACKET_IN_L2VPN_1) ? "tag_pkt_in_l2vpn(strip_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_PUSH_0) ? "l3vpn_push" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_0) ? "lsp_swap" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_SWAP_1) ? "spme" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_0) ? "l3vpn_pop" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_NORMAL_POP_1) ? "pwfwd_pop" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_0) ? "l2vpn_ingress(strip_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_1) ? "l2vpn_ingress(insert_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_2) ? "l2vpn_ingress(replace_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_INGRESS_3) ? "l2vpn_ingress(keep_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_0) ? "l2vpn_egress(strip_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_1) ? "l2vpn_egress(insert_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_2) ? "l2vpn_egress(replace_svlan)" : \
     (seq_no == GLB_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_L2VPN_EGRESS_3) ? "l2vpn_egress(keep_svlan)" : \
      "INVALID")  

#define GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_FIRST(p_flow_action_list, p_first_flow_action) \
do \
{ \
    p_first_flow_action = &((p_flow_action_list)[0]); \
} while(0)

#define GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT(p_cur_flow_action, p_next_flow_action) \
do \
{ \
    p_next_flow_action = (p_cur_flow_action) + 1; \
} while(0)


#define GLB_OPENFLOW_MPLS_DECODE_ACTION(action, decoded_action, expect_type) \
do \
{ \
    if (expect_type != (action)->type) \
        goto parse_error_out; \
    decoded_action = action; \
}while(0)            

/*There may be some TC or TTL settings between push_mpls and set_mpls_label actions, we should
  decode them out
  NOTE: TC or TTL can in any order, if there are two TC settings or two TTL settings, the last
  will make effect*/
#define GLB_OPENFLOW_MPLS_DECODE_PUSH_MPLS_ACTION(start_action, push_mpls, set_mpls_tc, set_mpls_ttl, set_mpls_label) \
do \
{ \
    glb_openflow_action_type_t action_type; \
    GLB_OPENFLOW_MPLS_DECODE_ACTION(start_action, push_mpls, GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS); \
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
    action_type = (start_action)->type; \
    if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL != action_type) \
    { \
        if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC == action_type) \
        { \
            set_mpls_tc = start_action; \
        } else if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL == action_type) \
        { \
            set_mpls_ttl = start_action; \
        } \
        else \
        { \
            goto parse_error_out; \
        } \
        /*fetch next possible TC or TTL settings, if there are 2 ttl settings 
          here, we do not treat it as error, the last will override the prior settings*/ \
        GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
        action_type = (start_action)->type; \
        if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL != action_type) \
        { \
            if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC == action_type) \
            { \
                set_mpls_tc = start_action; \
            } else if (GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL == action_type) \
            { \
                set_mpls_ttl = start_action; \
            } \
            else \
            { \
                goto parse_error_out; \
            } \
            GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
            GLB_OPENFLOW_MPLS_DECODE_ACTION((start_action), set_mpls_label, GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL); \
        } \
        else \
        { \
            GLB_OPENFLOW_MPLS_DECODE_ACTION((start_action), set_mpls_label, GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL); \
        } \
    } \
    else \
    { \
        GLB_OPENFLOW_MPLS_DECODE_ACTION((start_action), set_mpls_label, GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL); \
    } \
}while(0)

    
/*There may be possible VLAN_VID settings between push_l2 and output actions, we must decode
  them out
  NOTE: VLAN_VID and SET_DL_DST can in any order, but DL_DST must be present*/
#define GLB_OPENFLOW_MPLS_DECODE_PUSH_L2_OUTPUT_ACTION(start_action, push_l2, set_dl_dst, \
                                          set_vlan_vid, output) \
do \
{ \
    glb_openflow_action_type_t action_type; \
    bool set_dl_dst_present = FALSE; \
    GLB_OPENFLOW_MPLS_DECODE_ACTION(start_action, push_l2, GLB_OPENFLOW_ACTION_TYPE_PUSH_L2); \
    GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
    action_type = (start_action)->type; \
    if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT != action_type) \
    { \
        if (GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST == action_type) \
        { \
            set_dl_dst = start_action; \
            set_dl_dst_present = TRUE; \
        } else if (GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID == action_type) \
        { \
            set_vlan_vid = start_action; \
        } \
        else \
        { \
            goto parse_error_out; \
        } \
        /*fetch next possible actions*/ \
        GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
        action_type = (start_action)->type; \
        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT != action_type) \
        { \
            if (GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST == action_type) \
            { \
                set_dl_dst = start_action; \
                set_dl_dst_present = TRUE; \
            } else if (GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID == action_type) \
            { \
                set_vlan_vid = start_action; \
            } \
            else \
            { \
                goto parse_error_out; \
            } \
            GLB_OPENFLOW_MPLS_FLOW_ACTION_GET_NEXT((start_action), (start_action)); \
            GLB_OPENFLOW_MPLS_DECODE_ACTION((start_action), output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT); \
        } \
        else \
        { \
            GLB_OPENFLOW_MPLS_DECODE_ACTION((start_action), output, GLB_OPENFLOW_ACTION_TYPE_OUTPUT); \
        } \
    } \
    else \
    { \
        goto parse_error_out; \
    } \
    if (!set_dl_dst_present) \
        goto parse_error_out; \
}while(0)

struct glb_openflow_flow_action_s
{
    glb_openflow_action_type_t type;
    union 
    {
        uint8  u8;
        uint16 u16;
        uint32 u32;
        uint8  mac[MAC_ADDR_LEN];
        glb_openflow_metadata_t m;

        glb_openflow_output_t output;
        ipv6_addr_t ipv6;
    } value;
};
typedef struct glb_openflow_flow_action_s glb_openflow_flow_action_t;

struct glb_openflow_flow_action_list_s
{
    uint32 action_count;
    glb_openflow_flow_action_t* actlist;
};
typedef struct glb_openflow_flow_action_list_s glb_openflow_flow_action_list_t;


/**************************  flow ****************************************/
struct glb_openflow_flow_s
{
    uint32 priority;
    uint32 flow_id;
    glb_openflow_flow_profile_type_t profile_type; /* for access embedded structure in union corectly. */
    uint8  table;
    uint8  stats_en;
    uint8 flow_tnl_type;    /* glb_openflow_flow_tnl_type_e */
    uint8 payload_packet_type;  /* glb_openflow_flow_payload_packet_type_e */
    union 
    {
        struct 
        {
            uint8 mpls_output_count;
            uint16 mpls_output[GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW];
            glb_openflow_mpls_output_action_seq_type_t mpls_output_action_seq_type;
        } dft;
        struct
        {
        } nvcloud;
    } u; /* profile specific data. */
    
    glb_openflow_flow_key_t key;
    glb_openflow_flow_action_list_t actlist;
};
typedef struct glb_openflow_flow_s glb_openflow_flow_t;

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
struct glb_openflow_tpoam_s
{
    uint32               flag;
    
    uint32               oam_type;
    uint32               inlabel;
    uint32               outlabel_lsp;
    uint32               outlabel_pw;
    uint32               out_ifindex;
    uint32               vid;
    uint32               oam_session_id;

    uint32               is_mip;
    uint32               inlabel_mip;
    uint8                 megid[GLB_TPOAM_MEGID_LEN];
    uint32               mepid;
    uint32               rmepid;
    uint32               cc_enable;
    uint32               cc_interval;
    uint32               exp;
    uint32               lm_enable;
    uint32               lm_type;
    uint32               lm_cos_type;
    uint32               lm_cos_value;
    uint32               lm_stats_interval;
    uint32               dm_enable;
    uint32               mepIndex;
    uint32               rmepIndex;
    uint32               nh_id;
    uint32               offset;
    uint32               tunnel_id;
    uint32               gport;
    uint32               lsp_outlabel_exp_is_set;
    uint32               lsp_exp;
    uint32               lsp_ttl;
    uint32               pw_outlabel_exp_is_set;
    uint32               pw_exp;
    uint32               pw_ttl;
    uint32               level;
    uint32               event_bmp;  /* stored for fast APS */
    uint32               aps_group_id;
    uint32               lsp_aps_group_id;    /* valid for two level aps */
    uint32               aps_group_offset;   /* aps group id for chip */
    uint32               rdi_enable;
    uint32               is_working_path;
    mac_addr_t        nexthop_mac;
 };
typedef struct glb_openflow_tpoam_s glb_openflow_tpoam_t;
/* liwh end */

#ifdef USW
#ifndef _GLB_HOST_IS_LE
struct glb_openflow_oam_cycle_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
    
    uint32 average_dm_value;
    uint32 variance_dm_value;
    uint32 lm_far_end_frame_loss;
    uint32 lm_far_end_frame_loss_ratio;
    uint32 lm_near_end_frame_loss;
    uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_cycle_packet_in_message_header_s glb_openflow_oam_cycle_packet_in_message_header_t;

struct glb_openflow_oam_event_ccm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_ccm_packet_in_message_header_s glb_openflow_oam_event_ccm_packet_in_message_header_t;

struct glb_openflow_oam_event_dm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_dm_packet_in_message_header_s glb_openflow_oam_event_dm_packet_in_message_header_t;

struct glb_openflow_oam_event_lm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_lm_packet_in_message_header_s glb_openflow_oam_event_lm_packet_in_message_header_t;


struct glb_openflow_oam_event_aps_group_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    uint32 option                                                  :4;
    uint32                                                 :4;
    uint32 path_is_protection                                  :4;
    uint32 reserve0                                                  :4;
    uint32 reserve                                                   :8;
};
typedef struct glb_openflow_oam_event_aps_group_packet_in_message_header_s glb_openflow_oam_event_aps_group_packet_in_message_header_t;

#else
struct glb_openflow_oam_cycle_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    uint32 average_dm_value;
    uint32 variance_dm_value;
    uint32 lm_far_end_frame_loss;
    uint32 lm_far_end_frame_loss_ratio;
    uint32 lm_near_end_frame_loss;
    uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_cycle_packet_in_message_header_s glb_openflow_oam_cycle_packet_in_message_header_t;

struct glb_openflow_oam_event_ccm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_ccm_packet_in_message_header_s glb_openflow_oam_event_ccm_packet_in_message_header_t;

struct glb_openflow_oam_event_dm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*0:oam session, 1:aps group, glb_openflow_oam_packet_type_t*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event, glb_openflow_oam_event_type_t*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    //uint32 event_dm_value;
};
typedef struct glb_openflow_oam_event_dm_packet_in_message_header_s glb_openflow_oam_event_dm_packet_in_message_header_t;

struct glb_openflow_oam_event_lm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    //uint32 lm_far_end_frame_loss;
    //uint32 lm_far_end_frame_loss_ratio;
    //uint32 lm_near_end_frame_loss;
    //uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_event_lm_packet_in_message_header_s glb_openflow_oam_event_lm_packet_in_message_header_t;


struct glb_openflow_oam_event_aps_group_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;
    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    uint32                                                 :4;
    uint32 option                                                  :4;
    uint32 reserve0                                                  :4;
    uint32 path_is_protection                                  :4;
    uint32 reserve                                                   :8;
};
typedef struct glb_openflow_oam_event_aps_group_packet_in_message_header_s glb_openflow_oam_event_aps_group_packet_in_message_header_t;
#endif

#else

#ifdef _GLB_HOST_IS_LE
struct glb_openflow_oam_cycle_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
    
    uint32 average_dm_value;
    uint32 variance_dm_value;
    uint32 lm_far_end_frame_loss;
    uint32 lm_far_end_frame_loss_ratio;
    uint32 lm_near_end_frame_loss;
    uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_cycle_packet_in_message_header_s glb_openflow_oam_cycle_packet_in_message_header_t;

struct glb_openflow_oam_event_ccm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_ccm_packet_in_message_header_s glb_openflow_oam_event_ccm_packet_in_message_header_t;

struct glb_openflow_oam_event_dm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_dm_packet_in_message_header_s glb_openflow_oam_event_dm_packet_in_message_header_t;

struct glb_openflow_oam_event_lm_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    
    uint32 option                                                  :4;
    uint32 ccm_status                                                :4;

    uint32 dm_valid                                                :1;    /*1 is valid*/
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32                                                                 :2;
    uint32 reserve0                                                  :4;

    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_lm_packet_in_message_header_s glb_openflow_oam_event_lm_packet_in_message_header_t;


struct glb_openflow_oam_event_aps_group_packet_in_message_header_s
{
    uint32 version                               :4;
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    uint32 option                                                  :4;
    uint32                                                 :4;
    uint32 path_is_protection                                  :4;
    uint32 reserve0                                                  :4;
    uint32 reserve                                                   :8;
};
typedef struct glb_openflow_oam_event_aps_group_packet_in_message_header_s glb_openflow_oam_event_aps_group_packet_in_message_header_t;

#else
struct glb_openflow_oam_cycle_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    uint32 average_dm_value;
    uint32 variance_dm_value;
    uint32 lm_far_end_frame_loss;
    uint32 lm_far_end_frame_loss_ratio;
    uint32 lm_near_end_frame_loss;
    uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_cycle_packet_in_message_header_s glb_openflow_oam_cycle_packet_in_message_header_t;

struct glb_openflow_oam_event_ccm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
};
typedef struct glb_openflow_oam_event_ccm_packet_in_message_header_s glb_openflow_oam_event_ccm_packet_in_message_header_t;

struct glb_openflow_oam_event_dm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*0:oam session, 1:aps group, glb_openflow_oam_packet_type_t*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event, glb_openflow_oam_event_type_t*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    //uint32 event_dm_value;
};
typedef struct glb_openflow_oam_event_dm_packet_in_message_header_s glb_openflow_oam_event_dm_packet_in_message_header_t;

struct glb_openflow_oam_event_lm_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;

    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;

    uint32 ccm_status                                                :4;
    uint32 option                                                  :4;

    uint32 reserve0                                                  :4;
    uint32                                                                 :2;
    uint32 lm_valid                                                 :1;    /*1 is valid*/
    uint32 dm_valid                                                :1;    /*1 is valid*/
    
    
    uint32 reserve                                                 :8;
    
    //uint32 lm_far_end_frame_loss;
    //uint32 lm_far_end_frame_loss_ratio;
    //uint32 lm_near_end_frame_loss;
    //uint32 lm_near_end_frame_loss_ratio;
};
typedef struct glb_openflow_oam_event_lm_packet_in_message_header_s glb_openflow_oam_event_lm_packet_in_message_header_t;


struct glb_openflow_oam_event_aps_group_packet_in_message_header_s
{
    uint32 type                                     :4;  /*1:oam session, 2:aps group*/
    uint32 version                               :4;
    uint32 sub_type                                      :8; /*for oam event*/
    uint32 id                                   :16; /*session id or aps group*/

    uint32 length                                    :8;
    uint32                                                 :4;
    uint32 option                                                  :4;
    uint32 reserve0                                                  :4;
    uint32 path_is_protection                                  :4;
    uint32 reserve                                                   :8;
};
typedef struct glb_openflow_oam_event_aps_group_packet_in_message_header_s glb_openflow_oam_event_aps_group_packet_in_message_header_t;
#endif

#endif

/* Added by weizj for udf */
struct glb_openflow_udf_parser_s
{
    uint8  udf_type;
    uint8  is_del;
    uint16 ether_type;
    uint16 protocol;
    uint8  ip_version;
    uint16 l4_src;
    uint16 l4_dst;
    uint8  offset0;
    uint8  length0;
    uint8  offset1;
    uint8  length1;
    uint8  offset2;
    uint8  length2;
    uint8  offset3;
    uint8  length3;
};
typedef struct glb_openflow_udf_parser_s glb_openflow_udf_parser_t;

/**************************  label ****************************************/
enum glb_openflow_label_type_e
{
    GLB_OPENFLOW_LABEL_GLOBAL,
    GLB_OPENFLOW_LABEL_PORT,
};
typedef enum glb_openflow_label_type_e glb_openflow_label_type_t;

/**************************  stats ****************************************/
/* Flow stats */
struct glb_openflow_flow_stats_s
{
    glb_openflow_stats_t flow_stats;
};
typedef struct glb_openflow_flow_stats_s glb_openflow_flow_stats_t;

/**************************  group ****************************************/
enum glb_openflow_group_type_e
{
    GLB_OF_GROUP_TYPE_ALL,
    GLB_OF_GROUP_TYPE_SELECT,
    GLB_OF_GROUP_TYPE_INDIRECT,
    GLB_OF_GROUP_TYPE_FF,
    GLB_OF_GROUP_TYPE_LSP_APS,
    GLB_OF_GROUP_TYPE_PW_APS,

    GLB_OF_GROUP_TYPE_MAX,
};
typedef enum glb_openflow_group_type_e glb_openflow_group_type_t;

enum glb_openflow_group_alloc_type_e
{
    GLB_OF_GROUP_ALLOC_TYPE_MCAST,
    GLB_OF_GROUP_ALLOC_TYPE_ECMP,

    GLB_OF_GROUP_ALLOC_TYPE_MAX,
};
typedef enum glb_openflow_group_alloc_type_e glb_openflow_group_alloc_type_t;

/**************************  tunnel ****************************************/

enum glb_openflow_tunnel_info_fdb_status_s
{
    GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE,
    GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_READY,
    GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_READY 
};
typedef enum glb_openflow_tunnel_info_fdb_status_s glb_openflow_tunnel_info_fdb_status_s;

struct glb_openflow_tunnel_multipatch_info_s
{
    char bind_port_name[GLB_IFNAME_SZ];            /**< bind port name */
    uint8 nexthop_mac[MAC_ADDR_LEN];               /**< nexthop mac address */ 
    uint16 vlan_id;
    uint32 bond_port_ifindex;
    uint32 nexthop_ip;
    bool arp_is_ready;
    glb_openflow_tunnel_info_fdb_status_s fdb_status;
};
typedef struct glb_openflow_tunnel_multipatch_info_s glb_openflow_tunnel_multipatch_info_t;

/* Added by weizj for tunnel */
struct glb_openflow_tunnel_info_s
{
    glb_openflow_tunnel_port_type_t type;          /**< interface type */

    uint16 ofport;                                 /**< OVS port number */
    uint32 tunnel_ifindex;                         /**< tunnel port ifindex */
    uint32 bond_port_ifindex;                      /**< bond port ifindex */

    uint32 local_ip;                               /**< local ip address */
    uint32 remote_ip;                              /**< remote ip address */
    uint16 vlan_id;                                /**< vlan id */

    /* Modifed by weizj for tunnel bug 33369 */
    char tunnel_name[GLB_IFNAME_SZ];               /**< tunnel name */
    char bind_port_name[GLB_IFNAME_SZ];            /**< bind port name */
    uint8 nexthop_mac[MAC_ADDR_LEN];               /**< nexthop mac address */ 
    glb_openflow_tunnel_multipatch_info_t multipath[GLB_ECMP_MAX_PATH];
    uint32 nexthop_ip;
    bool is_dynamic;
    bool route_is_ready;
    uint32 route_mask_len;
    bool arp_is_ready;
    uint32 multipath_cnt;
    uint32 interface_status; /*0 is down, 1 is up*/
    uint32 is_direct_connection;
    glb_openflow_tunnel_info_fdb_status_s fdb_status;
    char direct_connection_ifname[GLB_IFNAME_SZ];
};
typedef struct glb_openflow_tunnel_info_s glb_openflow_tunnel_info_t;

/**************************  parser ****************************************/
enum glb_parser_linkagg_hash_use_e
{
    GLB_PARSER_LAG_HASH_USE_MAC = 1 << 0,
    GLB_PARSER_LAG_HASH_USE_IP  = 1 << 1,
    GLB_PARSER_LAG_HASH_USE_L4  = 1 << 2
};
typedef enum glb_parser_linkagg_hash_use_e glb_parser_linkagg_hash_use_t;

enum glb_parser_linkagg_hash_key_e
{
    GLB_PARSER_LAG_HASH_KEY_PORT     = 1 << 0,
    GLB_PARSER_LAG_HASH_KEY_MACSA    = 1 << 1,
    GLB_PARSER_LAG_HASH_KEY_MACDA    = 1 << 2,
    GLB_PARSER_LAG_HASH_KEY_VID      = 1 << 3,
    GLB_PARSER_LAG_HASH_KEY_COS      = 1 << 4,
    GLB_PARSER_LAG_HASH_KEY_L2PRO    = 1 << 5,

    GLB_PARSER_LAG_HASH_KEY_MAC_ALL  = GLB_PARSER_LAG_HASH_KEY_PORT  |
                                       GLB_PARSER_LAG_HASH_KEY_MACSA |
                                       GLB_PARSER_LAG_HASH_KEY_MACDA |
                                       GLB_PARSER_LAG_HASH_KEY_VID   |
                                       GLB_PARSER_LAG_HASH_KEY_COS   |
                                       GLB_PARSER_LAG_HASH_KEY_L2PRO,

    GLB_PARSER_LAG_HASH_KEY_IPSA     = 1 << 6,
    GLB_PARSER_LAG_HASH_KEY_IPDA     = 1 << 7,
    GLB_PARSER_LAG_HASH_KEY_IPPRO    = 1 << 8,

    GLB_PARSER_LAG_HASH_KEY_IP_ALL   = GLB_PARSER_LAG_HASH_KEY_IPSA  |
                                       GLB_PARSER_LAG_HASH_KEY_IPDA  |
                                       GLB_PARSER_LAG_HASH_KEY_IPPRO,

    GLB_PARSER_LAG_HASH_KEY_SRC_PORT = 1 << 9,
    GLB_PARSER_LAG_HASH_KEY_DST_PORT = 1 << 10,

    GLB_PARSER_LAG_HASH_KEY_L4_ALL   = GLB_PARSER_LAG_HASH_KEY_SRC_PORT |
                                       GLB_PARSER_LAG_HASH_KEY_DST_PORT
};
typedef enum glb_parser_linkagg_hash_key_e glb_parser_linkagg_hash_key_t;
#if 0
enum glb_parser_ecmp_hash_key_e
{
    GLB_ECMP_HASH_KEY_IPSA      = 1 << 0,
    GLB_ECMP_HASH_KEY_IPDA      = 1 << 1,
    GLB_ECMP_HASH_KEY_IPPRO     = 1 << 2,
    GLB_ECMP_HASH_KEY_DSCP      = 1 << 3,
    GLB_ECMP_HASH_KEY_SRC_PORT  = 1 << 4,
    GLB_ECMP_HASH_KEY_DST_PORT  = 1 << 5,
    GLB_ECMP_HASH_KEY_MACSA     = 1 << 6,
    GLB_ECMP_HASH_KEY_MACDA     = 1 << 7,
    GLB_ECMP_HASH_KEY_ETHERTYPE = 1 << 8,

    GLB_ECMP_HASH_KEY_ALL      = GLB_ECMP_HASH_KEY_IPSA      |
                                 GLB_ECMP_HASH_KEY_IPDA      |
                                 GLB_ECMP_HASH_KEY_IPPRO     |
                                 GLB_ECMP_HASH_KEY_SRC_PORT  |
                                 GLB_ECMP_HASH_KEY_DST_PORT  |
                                 GLB_ECMP_HASH_KEY_MACSA     |
                                 GLB_ECMP_HASH_KEY_MACDA     |
                                 GLB_ECMP_HASH_KEY_ETHERTYPE
};
typedef enum adpt_parser_ecmp_hash_key_e adpt_parser_ecmp_hash_key_t;
#endif
enum glb_parser_flow_hash_key_e
{
    GLB_FLOW_HASH_KEY_IPSA      = 1 << 0,
    GLB_FLOW_HASH_KEY_IPDA      = 1 << 1,
    GLB_FLOW_HASH_KEY_IPPRO     = 1 << 2,
    GLB_FLOW_HASH_KEY_SRC_PORT  = 1 << 3,
    GLB_FLOW_HASH_KEY_DST_PORT  = 1 << 4,
    GLB_FLOW_HASH_KEY_NVGRE_VSIID = 1<<5,
    GLB_FLOW_HASH_KEY_VXLAN_VNI = 1<<6,
    GLB_FLOW_HASH_KEY_INNER_IPSA      = 1 << 7,
    GLB_FLOW_HASH_KEY_INNER_IPDA      = 1 << 8,
    GLB_FLOW_HASH_KEY_INNER_IPPRO     = 1 << 9,
    GLB_FLOW_HASH_KEY_INNER_SRC_PORT  = 1 << 10,
    GLB_FLOW_HASH_KEY_INNER_DST_PORT  = 1 << 11,
    /*L2*/
    GLB_FLOW_HASH_KEY_INPUT                = 1<< 12,
    GLB_FLOW_HASH_KEY_SRCMAC             = 1<< 13,
    GLB_FLOW_HASH_KEY_DESTMAC           = 1<<14,
    
    GLB_FLOW_HASH_KEY_OUTER_DEFAULT      = GLB_FLOW_HASH_KEY_IPSA      |
                                 GLB_FLOW_HASH_KEY_IPDA      |
                                 GLB_FLOW_HASH_KEY_IPPRO     |
                                 GLB_FLOW_HASH_KEY_SRC_PORT  |
                                 GLB_FLOW_HASH_KEY_DST_PORT
};
typedef enum glb_parser_flow_hash_key_e glb_parser_flow_hash_key_t;

struct glb_openflow_bucket_s
{
    uint32 n_actions;         /**< total actions number */
    uint32 watch_port;        /**< watch port ifindex */
    uint32 watch_group;       /**< watch group, not support now */
    uint16 weight;            /**< select group weight */
    uint8 mpls_output_action_seq_type;
    uint8 mpls_output_count;

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-26 */
    uint32 label0;          /* pw label */
    uint32 label1;          /* lsp label */
    uint32 vid;
    mac_addr_t nexthop_mac;
    uint16 ofport;         /* output port */
    uint32 ifindex;
    
    uint32 lsp_aps_group_id;
    /* liwh end */

    uint16 mpls_ttl0;
    uint16 mpls_tc0;
    uint16 mpls_tc0_is_set;
    uint16 mpls_ttl1;
    uint16 mpls_tc1;
    uint16 mpls_tc1_is_set;

    glb_openflow_flow_action_t* actlist;      /**< glb_openflow_flow_action_t */
};
typedef struct glb_openflow_bucket_s glb_openflow_bucket_t;

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-26 */
struct glb_openflow_tpoam_bucket_s
{
    uint32 label0;          /* pw label */
    uint32 label1;          /* lsp label */
    uint32 vid;
    mac_addr_t nexthop_mac;
    uint16 ofport;         /* output port ifindex */
    uint16 gport;
    uint16 rsv;
    uint32 lsp_aps_group_id;
    uint32 ifindex;

    void* p_nh_info;    /* hsrv_of_nh_info_t* */
    uint16 mpls_ttl0;
    uint16 mpls_tc0;
    uint16 mpls_tc0_is_set;
    uint16 mpls_ttl1;
    uint16 mpls_tc1;
    uint16 mpls_tc1_is_set;
};
typedef struct glb_openflow_tpoam_bucket_s glb_openflow_tpoam_bucket_t;
/* liwh end */
struct glb_openflow_aps_lsp_check_s
{
    uint32 lsp_label;          /* pw label */
    uint32 ifindex;
    mac_addr_t nexthop_mac;
    uint32 vid;
    uint32 mpls_tc;
    uint32 mpls_tc_is_set;
    uint32 mpls_ttl;
};
typedef struct glb_openflow_aps_lsp_check_s glb_openflow_aps_lsp_check_t;

struct glb_openflow_group_s
{
    uint32 group_id;
    glb_openflow_group_type_t group_type;

    uint32 n_buckets;
    uint16 stats_en;
    uint16 linkagg_en;
    uint32 choose_bucket_idx;
    uint16 update_ff_group_en; /*added by hansf to sync ff group referrenc info to hsrv*/

    /* nested group */
    uint32 nested_group_count;
    /*mod by hansf for support group chain:all+all,all+ff*/
    //uint32 nested_group[GLB_LINKAGG_NUM_PER_SELECT_GROUP];
    uint32 nested_group[GLB_BUCKET_NUM_PER_ALL_GROUP];

    /*referred group*/
    uint32 referred_group_count;
    uint32 referred_group[GLB_BUCKET_NUM_PER_ALL_GROUP];

    /* !!! This member should always be the LAST one to easy copy between partial group. */
    glb_openflow_bucket_t* bucket_list;   /**< glb_openflow_bucket_t */
};
typedef struct glb_openflow_group_s glb_openflow_group_t;

/* Since group message is so big if we want to support 126 bucket which exceed cds MSG_SIZE(10240).
   We break the message to partial message and reconstruct the complete message in hsrv after hsrv
   receive the last chunk.

   The breaked message looks like this:

   [part 1] partial_group_head messsage
   [part 2] partial_group_bucket message (bucket in the message == PARTIAL_GROUP_BUCKET_NUMBER)
   [part 3] partial_group_tail messsage, and empty message which is only used to signal end of message
*/

/* MSG_SIZE in ctc_msg.h is 10240, we take 9600 to break the fei group to more small chunk which will not exceed 10240 per partial message. */
#define GLB_OPENFLOW_GROUP_ADD_MOD_MAX_MSG_SIZE 9600
/* Modifed by weizj for bug 36855 */
#define GLB_OPENFLOW_GROUP_BUCKET_MAX_MSG_SIZE  (GLB_ACTION_NUM_PER_BUCKET * sizeof(glb_openflow_flow_action_t) + sizeof(glb_openflow_bucket_t))
#define GLB_OPENFLOW_PARTIAL_GROUP_BUCKET_COUNT (GLB_OPENFLOW_GROUP_ADD_MOD_MAX_MSG_SIZE  / GLB_OPENFLOW_GROUP_BUCKET_MAX_MSG_SIZE)

/* NOTE: this structure is glb_openflow_buckets_list_t without buckets. */
struct glb_openflow_partial_buckets_list_s
{
    uint32 n_buckets;
    uint32 choose_bucket_idx;    /**< only for fast failover group */
    uint32 choose_bucket_changed;  /**< only for fast failover group */
};
typedef struct glb_openflow_partial_buckets_list_s glb_openflow_partial_buckets_list_t;

/* NOTE: this structure is glb_openflow_group_t without containing buckets in glb_openflow_buckets_list_t.  */
struct glb_openflow_partial_group_head_s
{
    glb_openflow_group_t group;
};
typedef struct glb_openflow_partial_group_head_s glb_openflow_partial_group_head_t;

struct glb_openflow_partial_group_bucket_s
{
    uint32 buckets_len;
};
typedef struct glb_openflow_partial_group_bucket_s glb_openflow_partial_group_bucket_t;

struct glb_openflow_partial_group_tail_s
{
};
typedef struct glb_openflow_partial_group_tail_s glb_openflow_partial_group_tail_t;
/**************************  meter ****************************************/
/* Added by weizj for meter */
enum glb_openflow_meter_band_type_e
{
    GLB_OPENFLOW_METER_BAND_TYPE_DROP,
    GLB_OPENFLOW_METER_BAND_TYPE_DSCP_REMARK,

    GLB_OPENFLOW_METER_BAND_TYPE_MAX
};
typedef enum glb_openflow_meter_band_type_e glb_openflow_meter_band_type_t;

struct glb_openflow_meter_s
{
    uint32 meter_id;
    uint8  stats_enable;

    glb_openflow_meter_band_type_t band_type;
    uint32 rate;
    uint32 burst_size;
};
typedef struct glb_openflow_meter_s glb_openflow_meter_t;

struct glb_openflow_queue_s
{
    uint16 queue_id;
    uint32 cir;
    uint32 pir;
    uint32 ifindex;
    uint8 shape_en;
};
typedef struct glb_openflow_queue_s glb_openflow_queue_t;

struct glb_openflow_meter_ipg_s
{
    uint8  enable;
};
typedef struct glb_openflow_meter_ipg_s glb_openflow_meter_ipg_t;

/**************************  inband ****************************************/
/* Added by weizj for inband */
struct glb_openflow_inband_s
{
    uint16 stag;
};
typedef struct glb_openflow_inband_s glb_openflow_inband_t;

/* for cubro mpls.
   The following parameter is copied and modified from sdk. */
#define GLB_OPENFLOW_MPLS_DEFAULT_TTL 255
#define GLB_OPENFLOW_MPLS_DEFAULT_EXP 0

#define GLB_OPENFLOW_MPLS_MAX_TUNNEL_NUM 10000 /* should less than CTC_NH_DEFAULT_MAX_MPLS_TUNNEL_NUM to handle possible
                                                 L2EDIT allocation by init. */

#define GLB_OPENFLOW_NH_MAX_NUM 16384
#define GLB_NH_MAX_NUM 2880
#define GLB_OPENFLOW_MET_MAX_NUM 8600
#define GLB_MET_MAX_NUM 656

enum gbl_openflow_mpls_nh_label_type_flag_e
{
    GLB_OPENFLOW_MPLS_NH_LABEL_IS_VALID = (1 << 0), 
    GLB_OPENFLOW_MPLS_NH_LABEL_MAP_TTL  = (1 << 1), 
    GLB_OPENFLOW_MPLS_NH_LABEL_IS_MCAST  = (1 << 2), 
};
typedef enum gbl_openflow_mpls_nh_label_type_flag_e gbl_openflow_mpls_nh_label_type_flag_t;

enum glb_openflow_mpls_nh_exp_select_mode_e
{
    GLB_OPENFLOW_MPLS_NH_EXP_SELECT_ASSIGN,  /**< [HB.GB.GG] Use user-define EXP to outer header.*/
    GLB_OPENFLOW_MPLS_NH_EXP_SELECT_MAP,     /**< [HB.GB.GG] Use EXP value from EXP map */
    GLB_OPENFLOW_MPLS_NH_EXP_SELECT_PACKET,      /**< [HB.GB.GG] Copy packet EXP to outer header */
    GLB_OPENFLOW_MPLS_NH_EXP_MAX             /* Must be last */
};
typedef enum glb_openflow_mpls_exp_select_mode_e glb_openflow_mpls_exp_select_mode_t;

struct glb_openflow_mpls_nh_label_param_s
{
    uint8  label_flag; /**< [HB.GB.GG] MPLS label flag,gbl_openflow_mpls_nh_label_type_flag_t */
    uint8  ttl;        /**< [HB.GB.GG] TTL value.
                                      if CTC_MPLS_NH_LABEL_MAP_TTL set to 1:
                                        FTN Pipe mode ttl set to 0,else 1.
                                      else ttl will fare up to (0£¬1£¬2£¬8£¬15£¬16£¬31£¬32£¬60£¬63£¬64£¬65£¬127£¬128£¬254£¬255)
                                      according to user defined value*/
    uint8  exp_type;   /**< [HB.GB.GG] New MPLS exp type in this label,pls refer to ctc_nh_exp_select_mode_t*/
    uint8  exp;        /**< [HB.GB.GG] EXP value*/
    uint32 label;      /**< [HB.GB.GG] MPLS label value*/
};
typedef struct glb_openflow_mpls_nh_label_param_s glb_openflow_mpls_nh_label_param_t;

struct glb_openflow_mpls_tunnel_param_s
{
    glb_openflow_mpls_nh_label_param_t tunnel_label; // TODO: not used in curbro mpls, will be used in MPLS L2VPN.
    uint16 gport; /* output port */
    uint16 vid; /* set to 0 if no outer vlan id set*/
    mac_addr_t nexthop_mac;
};
typedef struct glb_openflow_mpls_tunnel_param_s glb_openflow_mpls_tunnel_param_t;

/**************************  msg ****************************************/
typedef struct 
{
    uint32 logical_src_port;
    uint32 fid;
    uint32 payload_offset;
    uint32 metadata;
    uint16 tnl_ofport;  /* Added by weizj for tunnel */
} glb_openflow_msg_tail_t;


typedef struct 
{
    char ifname[GLB_IFNAME_SZ];
    uint32 is_add;
    addr_ipv4_t ipv4;
} glb_openflow_dynamic_tunnel_msg_t;

/**************************  flow action *********************************/
static inline void
glb_openflow_print_action(FILE*fp, glb_openflow_flow_action_t* p_action)
{
    uint8* p_ip;
    uint32* p_ipv6;
    
    switch (p_action->type) 
    {
    case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
        if (p_action->value.output.ofport == GLB_OFPP_ALL)
        {
            sal_fprintf(fp, "all");
        }
        else if (p_action->value.output.ofport == GLB_OFPP_CONTROLLER)
        {
            sal_fprintf(fp, "controller:%u", p_action->value.output.max_len);
        }
        else if (p_action->value.output.ofport == GLB_OFPP_IN_PORT)
        {
            sal_fprintf(fp, "in_port");
        }
        else
        {
            sal_fprintf(fp, "output:%u", p_action->value.output.ofport);
        }
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN:
        sal_fprintf(fp, "push_vlan:0x%x", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN:
        sal_fprintf(fp, "strip_vlan");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_L2:
        sal_fprintf(fp, "push_l2");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_POP_L2:
        sal_fprintf(fp, "pop_l2");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS:
        sal_fprintf(fp, "push_mpls:0x%x", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_POP_MPLS:
        sal_fprintf(fp, "pop_mpls");
        break;

    case GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS:
        sal_fprintf(fp, "pop_all_mpls");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL:
        sal_fprintf(fp, "tunnel:%u", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC:
        sal_fprintf(fp, "set_field:%02x:%02x:%02x:%02x:%02x:%02x->eth_src",
            p_action->value.mac[0], p_action->value.mac[1],
            p_action->value.mac[2], p_action->value.mac[3],
            p_action->value.mac[4], p_action->value.mac[5]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST:
        sal_fprintf(fp, "set_field:%02x:%02x:%02x:%02x:%02x:%02x->eth_dst",
            p_action->value.mac[0], p_action->value.mac[1],
            p_action->value.mac[2], p_action->value.mac[3],
            p_action->value.mac[4], p_action->value.mac[5]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID:
        sal_fprintf(fp, "set_field:%u->dl_vlan", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP:
        sal_fprintf(fp, "set_field:%u->vlan_pcp", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL:
        sal_fprintf(fp, "set_field:%u->mpls_label", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC:
        sal_fprintf(fp, "set_field:%u->mpls_tc", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL:
        sal_fprintf(fp, "set_field:%u->mpls_ttl", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_GROUP:
        sal_fprintf(fp, "group:%u", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_INST_METER:
        sal_fprintf(fp, "meter:%u", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE:
        sal_fprintf(fp, "set_queue:%u", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC:
        p_ip = (uint8*)&p_action->value.u32;
        sal_fprintf(fp, "set_field:%u.%u.%u.%u->nw_src", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST:
        p_ip = (uint8*)&p_action->value.u32;
        sal_fprintf(fp, "set_field:%u.%u.%u.%u->nw_dst", 
            p_ip[3], p_ip[2], p_ip[1], p_ip[0]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC:
        sal_fprintf(fp, "set_field:%u->tp_src", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST:
        sal_fprintf(fp, "set_field:%u->tp_dst", p_action->value.u16);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC:
        sal_fprintf(fp, "set_field:%u->tcp_src", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST:
        sal_fprintf(fp, "set_field:%u->tcp_dst", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC:
        sal_fprintf(fp, "set_field:%u->udp_src", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST:
        sal_fprintf(fp, "set_field:%u->udp_dst", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC:
        sal_fprintf(fp, "set_field:%u->sctp_src", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST:
        sal_fprintf(fp, "set_field:%u->sctp_dst", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE:
        sal_fprintf(fp, "goto_table:%u", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
        sal_fprintf(fp, "metadata:%"PRIu64, p_action->value.m.metadata);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL:
        sal_fprintf(fp, "dec_ip_ttl");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN:
        sal_fprintf(fp, "set_field:%u->ip_ecn", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP:
        sal_fprintf(fp, "set_field:%u->ip_dscp", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
        sal_fprintf(fp, "dec_ttl");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL:
        sal_fprintf(fp, "set_field:%u->nw_ttl", p_action->value.u8);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL:
        sal_fprintf(fp, "dec_mpls_ttl");
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC:
        p_ipv6 = p_action->value.ipv6;
        sal_fprintf(fp, "set_field:%08X:%08X:%08X:%08X->nw_src", 
            p_ipv6[0], p_ipv6[1], p_ipv6[2], p_ipv6[3]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST:
        p_ipv6 = p_action->value.ipv6;
        sal_fprintf(fp, "set_field:%08X:%08X:%08X:%08X->nw_dst", 
            p_ipv6[0], p_ipv6[1], p_ipv6[2], p_ipv6[3]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL:
        sal_fprintf(fp, "set_field:%u->ipv6_label", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP:
        sal_fprintf(fp, "set_field:%u->arp_op", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA:
        sal_fprintf(fp, "set_field:%u->arp_spa", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA:
        sal_fprintf(fp, "set_field:%u->arp_tpa", p_action->value.u32);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA:
        sal_fprintf(fp, "set_field:%02x:%02x:%02x:%02x:%02x:%02x->arp_sha",
            p_action->value.mac[0], p_action->value.mac[1],
            p_action->value.mac[2], p_action->value.mac[3],
            p_action->value.mac[4], p_action->value.mac[5]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA:
        sal_fprintf(fp, "set_field:%02x:%02x:%02x:%02x:%02x:%02x->arp_tha",
            p_action->value.mac[0], p_action->value.mac[1],
            p_action->value.mac[2], p_action->value.mac[3],
            p_action->value.mac[4], p_action->value.mac[5]);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE:
        sal_fprintf(fp, "set_field:%u->eth_type", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE:
        sal_fprintf(fp, "set_field:%u->icmp_type", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE:
        sal_fprintf(fp, "set_field:%u->icmp_code", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE:
        sal_fprintf(fp, "set_field:%u->icmpv6_type", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE:
        sal_fprintf(fp, "set_field:%u->icmpv6_code", p_action->value.u16);
        break;
        
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO:
        sal_fprintf(fp, "set_field:%u->nw_proto", p_action->value.u16);
        break;

    case GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE:
        sal_fprintf(fp, "truncate:%u", p_action->value.u32);
        break;
    case GLB_OPENFLOW_ACTION_TYPE_SET_OAM_INLABEL:
        sal_fprintf(fp, "oam_inlabel:%u", p_action->value.u32);

    case GLB_OPENFLOW_ACTION_TYPE_SET_OAM_POPLABEL:
        sal_fprintf(fp, "oam_poplabel:%u", p_action->value.u32);
    case GLB_OPENFLOW_ACTION_TYPE_CLEAR_ACTIONS:
        sal_fprintf(fp ,"clear-actions");
    case GLB_OPENFLOW_ACTION_TYPE_UNSUPPORTED:
        sal_fprintf(fp, "%s", GLB_OPENFLOW_ACTION_TYPE_TO_STR(p_action->type));
        break;        
    default:
        break;
    }
}

static inline void
glb_openflow_action_hash(uint32* p_hash, glb_openflow_flow_action_t* p_action)
{
    switch (p_action->type) 
    {
    case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_uint16(p_hash, p_action->value.output.ofport);
        break;

    /* u8 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO:
    case GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_FLOW_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_PACKET_TYPE:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_uint8(p_hash, p_action->value.u8);
        break;
        
    /* u16 */
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN:
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_uint16(p_hash, p_action->value.u16);
        break;
        
    /* u32 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_uint32(p_hash, p_action->value.u32);
        break;
        
    /* mac */
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_mac(p_hash, p_action->value.mac);
        break;
        
    /* ipv6 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST:
        ofp_hash_key_uint32(p_hash, p_action->type);
        ofp_hash_key_array(p_hash, (uint8*)&p_action->value.ipv6, sizeof(ipv6_addr_t));
        break;
        
    /* no value */
    case GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_L2:
    case GLB_OPENFLOW_ACTION_TYPE_POP_L2:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_POP_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS: 
    case GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL:
        ofp_hash_key_uint32(p_hash, p_action->type);
        break;
        
    /* ignored in actlist */
    case GLB_OPENFLOW_ACTION_TYPE_GROUP:
    case GLB_OPENFLOW_ACTION_TYPE_INST_METER:
    case GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE:
    case GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE:
    case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
    case GLB_OPENFLOW_ACTION_TYPE_UNSUPPORTED:
    default:
        break;
    }
}

static inline int32
glb_openflow_action_is_same(glb_openflow_flow_action_t* p_action1, glb_openflow_flow_action_t* p_action2)
{
    /* type must be same */
    if (p_action1->type != p_action2->type)
    {
        return FALSE;
    }

    switch (p_action1->type) 
    {
    /* output */
    case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
        if (p_action1->value.output.ofport != p_action2->value.output.ofport)
        {
            return FALSE;
        }
        break;

    /* u8 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_PCP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV4_CODE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ICMPV6_CODE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IP_PROTO:
    case GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_FLOW_TYPE:
    case GLB_OPENFLOW_ACTION_TYPE_SET_V4_V6_PACKET_TYPE:
        if (p_action1->value.u8 != p_action2->value.u8)
        {
            return FALSE;
        }
        break;

    /* u16 */
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_VLAN:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_SET_VLAN_VID:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_SCTP_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_OP:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_TYPE:
        if (p_action1->value.u16 != p_action2->value.u16)
        {
            return FALSE;
        }
        break;

    /* u32 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_MPLS_LABEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_LABEL:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SPA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_TPA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE:
        if (p_action1->value.u32 != p_action2->value.u32)
        {
            return FALSE;
        }
        break;

    /* mac */
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ETH_DST:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_SHA:
    case GLB_OPENFLOW_ACTION_TYPE_SET_ARP_THA:
        if (0 != sal_memcmp(p_action1->value.mac, p_action2->value.mac, MAC_ADDR_LEN))
        {
            return FALSE;
        }
        break;

    /* ipv6 */
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_SRC:
    case GLB_OPENFLOW_ACTION_TYPE_SET_IPV6_DST:
        if (0 != sal_memcmp(&p_action1->value.ipv6, &p_action2->value.ipv6, sizeof(ipv6_addr_t)))
        {
            return FALSE;
        }
        break;

    /* no value */
    case GLB_OPENFLOW_ACTION_TYPE_STRIP_VLAN:
    case GLB_OPENFLOW_ACTION_TYPE_PUSH_L2:
    case GLB_OPENFLOW_ACTION_TYPE_POP_L2:
    case GLB_OPENFLOW_ACTION_TYPE_POP_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_POP_ALL_MPLS:
    case GLB_OPENFLOW_ACTION_TYPE_GROUP:
    case GLB_OPENFLOW_ACTION_TYPE_INST_METER:
    case GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE:
    case GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE:
    case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
    case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_DEC_MPLS_TTL:
    case GLB_OPENFLOW_ACTION_TYPE_UNSUPPORTED:
    default:
        break;
    }

    return TRUE;
}
#endif /*! __GLB_OPENFLOW_DEFINE_H__ */
