#ifndef __GLB_CPU_TRAFFIC_DEFINE_H__
#define __GLB_CPU_TRAFFIC_DEFINE_H__

#define GLB_CPU_TRAFFIC_MAX_RATE   1000000  /* kbps */

#define GLB_CPU_TRAFFIC_REASON_CLASS_MAX            6
#define GLB_CPU_TRAFFIC_MAX_NAME_LEN            30
#define GLB_DEF_CPU_TRAFFIC_TOTAL_RATE            2048 /* pps */

#define GLB_MAX_CPU_REASON_MSG_NUM          32

#define GLB_PDU_L2HDR_PROTO_CFM  0x8902
#define GLB_PDU_L2HDR_PROTO_SMART_LINK  0x0127
#define GLB_PDU_L2HDR_PROTO_L2PING      0x9009
/* modified by yaom for udld, 2011.7.6 */
#define GLB_PDU_L2HDR_PROTO_UDLD        0x0111

/* modified by liuyang for lldp, 2012-11-1 */
#define GLB_PDU_L2HDR_PROTO_LLDP        0x88CC

/* modified by sunh for bug 27530, 2014-03-06 */
#define GLB_PDU_L2HDR_PROTO_LBDT        0x9998
#define GLB_PDU_L3HDR_PROTO_OSPF   89
#define GLB_PDU_L3HDR_PROTO_PIM    103
#define GLB_PDU_L3HDR_PROTO_VRRP   112
#define GLB_PDU_L3HDR_PROTO_RSVP   46
#define GLB_PDU_L3HDR_PROTO_ICMPv6 58
#define GLB_PDU_L3HDR_PROTO_IGMP    2

#define GLB_PDU_L4PORT_RIP              520
#define GLB_PDU_L4PORT_RIPng            521
#define GLB_PDU_L4PORT_BGP              179
#define GLB_PDU_L4PORT_LDP              646
#define GLB_PDU_L4PORT_MPLS_OAM         0x3503
#define GLB_PDU_L4PORT_BFD              3784

/* modified by liwh for bug 17437 , 2011-12-21 */
#define GLB_NEXTHOP_BYPASS_TO_CPU             0x3fffd

#define GLB_PKT_SFLOW_IGS                   1
#define GLB_PKT_SFLOW_EGS                   2
#define GLB_CPU_TRAFFIC_DEFAULT_RATE                  2048
#define GLB_CPU_TRAFFIC_CLASS0_DEFAULT_RATE           1024

enum glb_packet_tocpu_reason_e 
{
    /*
    * cdefault trap-id  
    */

   GLB_CPU_TRAFFIC_DEFAULT,
    /*
    * control protocol 
    */
    /*
    * switch trap 
    */

   GLB_CPU_TRAFFIC_STP,

   GLB_CPU_TRAFFIC_PVRST, 

   GLB_CPU_TRAFFIC_IGMP_TYPE_QUERY,   

   GLB_CPU_TRAFFIC_IGMP_TYPE_LEAVE,   

   GLB_CPU_TRAFFIC_IGMP_TYPE_V1_REPORT,  

   GLB_CPU_TRAFFIC_IGMP_TYPE_V2_REPORT,   

   GLB_CPU_TRAFFIC_IGMP_TYPE_V3_REPORT,  

   GLB_CPU_TRAFFIC_SWITCH_CUSTOM_RANGE_BASE,

   GLB_CPU_TRAFFIC_ERPS,

    /*
    * router trap 
    */
   GLB_CPU_TRAFFIC_ARP,
    
   GLB_CPU_TRAFFIC_ARP_REQUEST,  
 
   GLB_CPU_TRAFFIC_ARP_RESPONSE,

   GLB_CPU_TRAFFIC_DHCP,  

   GLB_CPU_TRAFFIC_EAPOL,  /*modified by yejl for bug 49856, 2018-11-19*/

   GLB_CPU_TRAFFIC_IGMP,

   GLB_CPU_TRAFFIC_LLDP,

   GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC, /*modified by yejl for bug 50657, 2018-01-05*/

   GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC, /*modified by yejl for bug 50657, 2019-01-05*/

   GLB_CPU_TRAFFIC_MLAG,

   GLB_CPU_TRAFFIC_OSPF,

   GLB_CPU_TRAFFIC_LACP,

   GLB_CPU_TRAFFIC_PIM, 

   GLB_CPU_TRAFFIC_VRRP, 

   GLB_CPU_TRAFFIC_BGP, 

   GLB_CPU_TRAFFIC_DHCPV6,  

   GLB_CPU_TRAFFIC_OSPFV6,  

   GLB_CPU_TRAFFIC_VRRPV6, 

   GLB_CPU_TRAFFIC_BGPV6, 

   GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY,  

   GLB_CPU_TRAFFIC_IPV6_MLD_V1_V2 ,   
 
   GLB_CPU_TRAFFIC_IPV6_MLD_V1_REPORT,

   GLB_CPU_TRAFFIC_IPV6_MLD_V1_DONE, 

   GLB_CPU_TRAFFIC_MLD_V2_REPORT ,

   GLB_CPU_TRAFFIC_L3_MTU_ERROR,
  
   GLB_CPU_TRAFFIC_TTL_ERROR,  /* Modified by kcao 2017-05-08 for bug 43927, OSPF ExStart -> Full need uses TTL ERROR */

   GLB_CPU_TRAFFIC_COPY_TO_CPU, /** Forward to cpu reason, extend for copp by huwx */

   GLB_CPU_TRAFFIC_ROUTER_CUSTOM_RANGE_BASE,

   GLB_CPU_TRAFFIC_PTP ,/*for ptp add by chenc*/

    /* 
    * pipeline exceptions   
    */

   GLB_CPU_TRAFFIC_L3_RPF,  
 
   GLB_CPU_TRAFFIC_L3_ASSERT, 

   GLB_CPU_TRAFFIC_CUSTOM_EXCEPTION_RANGE_BASE,

   GLB_CPU_TRAFFIC_OTHER_DEFINE,

   GLB_CPU_TRAFFIC_PACKET_IN,

   GLB_CPU_TRAFFIC_SSH,

   GLB_CPU_TRAFFIC_TELNET,

   GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR,
   
   GLB_CPU_TRAFFIC_MAX,
   
};
typedef enum glb_packet_tocpu_reason_e glb_packet_tocpu_reason_t;

#define GLB_PKT_OPENFLOW_TOCPU_TABLE_NUM 4
typedef enum
{
    GLB_PKT_OPENFLOW_TOCPU_TABLE,
    GLB_PKT_OPENFLOW_TOCPU_TABLE_MAX = GLB_PKT_OPENFLOW_TOCPU_TABLE + GLB_PKT_OPENFLOW_TOCPU_TABLE_NUM,
    GLB_PKT_OPENFLOW_TOCPU_GROUP = GLB_PKT_OPENFLOW_TOCPU_TABLE_MAX,
    GLB_PKT_OPENFLOW_TOCPU_LOCAL,

    GLB_PKT_OPENFLOW_TOCPU_MAX
} glb_packet_tocpu_openflow_reason_t;

typedef enum
{
    GLB_CPU_TRAFFIC_GROUP_0,
    GLB_CPU_TRAFFIC_GROUP_1,
    GLB_CPU_TRAFFIC_GROUP_2,
    GLB_CPU_TRAFFIC_GROUP_3,
    GLB_CPU_TRAFFIC_GROUP_4,
    GLB_CPU_TRAFFIC_GROUP_5,

    GLB_CPU_TRAFFIC_GROUP_MAX
} glb_cpu_traffic_group_t;

#define GLB_CPU_TRAFFIC_QUEUE_MAP_0  0
#define GLB_CPU_TRAFFIC_QUEUE_MAP_1  1
#define GLB_CPU_TRAFFIC_QUEUE_MAP_2  2
#define GLB_CPU_TRAFFIC_QUEUE_MAP_3  3
#define GLB_CPU_TRAFFIC_QUEUE_MAP_4  4
#define GLB_CPU_TRAFFIC_QUEUE_MAP_5  5



#define GLB_PKT_FLAGS_TX_COPY_TO_KERNEL         0x01
#define GLB_PKT_FLAGS_TX_MCAST                  0x02    /* for example, IGMP query need flood in VLAN */

typedef enum
{
    GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN =1,
    GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MAX = GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN + GLB_PKT_OPENFLOW_TOCPU_MAX,

    GLB_PKT_CUSTOM_TOCPU_MAX,
} glb_packet_custom_tocpu_reason_t;

typedef enum
{
    GLB_PKT_UNKNOWN,
    GLB_PKT_IGMP,
    GLB_PKT_BPDU,
    GLB_PKT_LACP,
    GLB_PKT_CFM,
    GLB_PKT_EFM,
    GLB_PKT_MAC_SEC,
    GLB_PKT_SFLOW,
    GLB_PKT_ARP,
    GLB_PKT_DHCP,
    GLB_PKT_IPSTACK,
    GLB_PKT_MTU_EXCP,
    GLB_PKT_PTP, 
    GLB_PKT_SSM,  
    GLB_PKT_EAPOL,
    GLB_PKT_L3LPBK,
    GLB_PKT_MPLS_OAM,
    GLB_PKT_MPLS_OAM_LBK,
    GLB_PKT_L2_PING,
    GLB_PKT_L2_UDLD,
    GLB_PKT_TPOAM,
    GLB_PKT_L2_LLDP,
    GLB_PKT_L2_LBDT,        /* modified by sunh for bug 27530, 2014-03-10 */
    GLB_PKT_MAC_BY_PASS,    /*Add by huangxt for 802.1X mac based*/
    GLB_PKT_RMT,
    GLB_PKT_OPENFLOW,
    GLB_PKT_ERPS,
    GLB_PKT_VRRP,
    GLB_PKT_L2_PROTOCOL_MAC,
    GLB_PKT_L2_GROUP_MAC,
    GLB_PKT_ND,   /* support nd modified by liwh for bug 47547, 2018-06-28 */
    GLB_PKT_CPU_MIRROR,
    GLB_PKT_PIM,
    GLB_PKT_RAPS,
    GLB_PKT_TYPE_MAX 
} glb_pkt_type_t;

/* l2-pdu port action */
enum glb_pdu_l2pdu_port_action_e
{
    GLB_PDU_L2PDU_PORT_ACTION_REDIRECT_TO_CPU = 0,
    GLB_PDU_L2PDU_PORT_ACTION_COPY_TO_CPU,
    GLB_PDU_L2PDU_PORT_ACTION_FWD,
    GLB_PDU_L2PDU_PORT_ACTION_DISCARD,
    
    GLB_PDU_L2PDU_PORT_ACTION_MAX
};
typedef enum glb_pdu_l2pdu_port_action_e glb_pdu_l2pdu_port_action_t;

#define COPP_REASON_STR_LEN_MAX   30

struct copp_reason_info_s
{
    uint32 reason_id;
    char reason_map_str[COPP_REASON_STR_LEN_MAX];
    uint32 default_rate;
    uint32 default_class_id;
    uint8 visable;  /*visable(configurable) in CLI*/
    
};
typedef struct copp_reason_info_s copp_reason_info_t;

#define COPP_REASON_DEFAULT_VALUE_BPDU            64
#define COPP_REASON_DEFAULT_VALUE_SLOW_PROTO      64
#define COPP_REASON_DEFAULT_VALUE_EAPOL           64
#define COPP_REASON_DEFAULT_VALUE_LLDP            64
#define COPP_REASON_DEFAULT_VALUE_IGMP            128
#define COPP_REASON_DEFAULT_VALUE_ERPS            64
#define COPP_REASON_DEFAULT_VALUE_ARP             160
#define COPP_REASON_DEFAULT_VALUE_DHCP            320
#define COPP_REASON_DEFAULT_VALUE_OSPF            64    /* modified by liwh for bug 47889, 2019-02-20 */
#define COPP_REASON_DEFAULT_VALUE_PIM             384
#define COPP_REASON_DEFAULT_VALUE_VRRP            256
#define COPP_REASON_DEFAULT_VALUE_BGP             512
#define COPP_REASON_DEFAULT_VALUE_MAGMT_TRA       1600 /*modified by yejl for bug 48597, 2018-11-27, 8096 -> 1000*/
#define COPP_REASON_DEFAULT_VALUE_TELNET          512
#define COPP_REASON_DEFAULT_VALUE_SSH             512 
#define COPP_REASON_DEFAULT_VALUE_MLAG            256
#define COPP_REASON_DEFAULT_VALUE_PACKET_IN       160
#define COPP_REASON_DEFAULT_VALUE_PTP             128
#define COPP_REASON_DEFAULT_VALUE_TTL_FAIL        64
#define COPP_REASON_DEFAULT_VALUE_MTU_FAIL        64    /*modified by yejl for bug 51254, 2019-03-20*/
#define COPP_REASON_DEFAULT_VALUE_L2PRO_PROTOCOL_MAC        256
#define COPP_REASON_DEFAULT_VALUE_L2PRO_GROUP_MAC        256
#define COPP_REASON_DEFAULT_VALUE_CPU_MIRROR        256
#define COPP_REASON_DEFAULT_VALUE_ND             64



#define GLB_PKT_MAP_TYPE_TO_STR(_type_) \
    (_type_ == GLB_PKT_UNKNOWN      ?  "unknown"        : \
     _type_ == GLB_PKT_IGMP         ?  "igmp"           : \
     _type_ == GLB_PKT_BPDU         ?  "bpdu"           : \
     _type_ == GLB_PKT_LACP         ?  "lacp"           : \
     _type_ == GLB_PKT_CFM          ?  "cfm"            : \
     _type_ == GLB_PKT_EFM          ?  "efm"            : \
     _type_ == GLB_PKT_MAC_SEC      ?  "mac-security"   : \
     _type_ == GLB_PKT_SFLOW        ?  "sflow"          : \
     _type_ == GLB_PKT_ARP          ?  "arp"            : \
     _type_ == GLB_PKT_DHCP         ?  "dhcp"           : \
     _type_ == GLB_PKT_IPSTACK      ?  "ipstack"        : \
     _type_ == GLB_PKT_MTU_EXCP     ?  "mtu-expection"  : \
     _type_ == GLB_PKT_PTP          ?  "ptp"            : \
     _type_ == GLB_PKT_SSM          ?  "ssm"            : \
     _type_ == GLB_PKT_EAPOL        ?  "eapol"          : \
     _type_ == GLB_PKT_L3LPBK       ?  "l3-loopback"    : \
     _type_ == GLB_PKT_MPLS_OAM     ?  "mpls-oam"       : \
     _type_ == GLB_PKT_MPLS_OAM_LBK ?  "mpls-loopback"  : \
     _type_ == GLB_PKT_L2_PING      ?  "l2-ping"        : \
     _type_ == GLB_PKT_L2_UDLD      ?  "udld"           : \
     _type_ == GLB_PKT_TPOAM        ?  "mplstp-oam"     : \
     _type_ == GLB_PKT_L2_LLDP      ?  "lldp"           : \
     _type_ == GLB_PKT_L2_LBDT      ?  "loopback-detect": \
     _type_ == GLB_PKT_MAC_BY_PASS  ?  "mac-bypass"     : \
     _type_ == GLB_PKT_RMT          ?  "rmt"            : \
     _type_ == GLB_PKT_OPENFLOW     ?  "openflow"       : \
     _type_ == GLB_PKT_ERPS         ?  "erps"           : \
     "invalid")


/* flexible l2-pdu classification key definition */
struct glb_pdu_flex_l2pdu_key_s 
{
    uint8_t  flex_l2pdu;   /* glb_pdu_flex_l2pdu_t */
    uint8_t  key_type;     /* glb_pdu_l2pdu_key_type_t */
    uint8_t  reserved[2];

    union 
    {
        struct
        {
            uint8_t macda[6];
            uint8_t macda_mask[6];
        } l2pdu_by_mac;

        uint16_t ether_type;
    } key;
};
typedef struct glb_pdu_flex_l2pdu_key_s glb_pdu_flex_l2pdu_key_t;

#endif /* __GLB_CPU_TRAFFIC_DEFINE_H__*/
