
#ifndef __GLB_DEBUG_DEFINE_H__
#define __GLB_DEBUG_DEFINE_H__

#define DBG_TYPE_ALL                        0xFFFFFFFF

/* SWITCH DEBUG */
typedef enum
{
    SWTHDBG_TYPE_STP = 0,
    SWTHDBG_TYPE_MLAG,
    SWTHDBG_TYPE_LACP,
    SWTHDBG_TYPE_IGSP,
    SWTHDBG_TYPE_IPSG,
    SWTHDBG_TYPE_LLDP,
    SWTHDBG_TYPE_RPCAPI,
    SWTHDBG_TYPE_MAX
} swthdbg_type_t;

#define SWTHDBG_FLAG_STP_EVENT              0x00000001
#define SWTHDBG_FLAG_STP_PACKET_TX          0x00000002
#define SWTHDBG_FLAG_STP_PACKET_RX          0x00000004
#define SWTHDBG_FLAG_STP_PROTO_CIST         0x00000008
#define SWTHDBG_FLAG_STP_PROTO_MSTI         0x00000010
#define SWTHDBG_FLAG_STP_TIMER              0x00000020

#define SWTHDBG_FLAG_MLAG_EVENT             0x00000001
#define SWTHDBG_FLAG_MLAG_PACKET            0x00000002
#define SWTHDBG_FLAG_MLAG_PROTO             0x00000004
#define SWTHDBG_FLAG_MLAG_TIMER             0x00000008

#define SWTHDBG_FLAG_LACP_EVENT             0x00000001
#define SWTHDBG_FLAG_LACP_PACKET            0x00000002
#define SWTHDBG_FLAG_LACP_PROTO             0x00000004
#define SWTHDBG_FLAG_LACP_TIMER             0x00000008

#define SWTHDBG_FLAG_IGSP_EVENT             0x00000001
#define SWTHDBG_FLAG_IGSP_PACKET_TX         0x00000002
#define SWTHDBG_FLAG_IGSP_PACKET_RX         0x00000004
#define SWTHDBG_FLAG_IGSP_PROTO             0x00000008
#define SWTHDBG_FLAG_IGSP_TIMER             0x00000010

#define SWTHDBG_FLAG_IPSG_EVENT             0x00000001
#define SWTHDBG_FLAG_IPSG_ERROR             0x00000002

#define SWTHDBG_FLAG_LLDP_EVENT              0x00000001
#define SWTHDBG_FLAG_LLDP_PACKET_TX          0x00000002
#define SWTHDBG_FLAG_LLDP_PACKET_RX          0x00000004
#define SWTHDBG_FLAG_LLDP_PROTO              0x00000008
#define SWTHDBG_FLAG_LLDP_TIMER              0x00000010

#define SWTHDBG_FLAG_RPCAPI_REQUEST         0x00000001
#define SWTHDBG_FLAG_RPCAPI_COMMAND         0x00000002

/* ROUTED DEBUG */
typedef enum
{
    RTDBG_TYPE_ROUTE = 0,
    RTDBG_TYPE_ARP,
    RTDBG_TYPE_ARPINSPECTION,
    RTDBG_TYPE_VRRP,
    RTDBG_TYPE_IPSLA,
    RTDBG_TYPE_TRACK,
    RTDBG_TYPE_ND,
    RTDBG_TYPE_NAT,
    RTDBG_TYPE_MAX
} rtdbg_type_t;

/* ROUTE DEBUG*/
#define RTDBG_FLAG_ROUTE_IPV4              0x00000001
#define RTDBG_FLAG_ROUTE_IPV6              0x00000002
#define RTDBG_FLAG_ROUTE_NETLINK              0x00000004

/* ARP DEBUG*/
#define RTDBG_FLAG_ARP_EVENT                0x00000001
#define RTDBG_FLAG_ARP_PACKET               0x00000002
#define RTDBG_FLAG_ARP_PROTO                0x00000004
#define RTDBG_FLAG_ARP_TIMER                0x00000008

/* ARPINSPECTION DEBUG*/
#define RTDBG_FLAG_ARPINSPECTION_EVENT      0x00000001
#define RTDBG_FLAG_ARPINSPECTION_PACKET     0x00000002
#define RTDBG_FLAG_ARPINSPECTION_PROTO      0x00000004
#define RTDBG_FLAG_ARPINSPECTION_TIMER      0x00000008

/* VRRP DEBUG */
#define RTDBG_FLAG_VRRP_EVENTS              0x00000001
#define RTDBG_FLAG_VRRP_PACKET_TX           0x00000002
#define RTDBG_FLAG_VRRP_PACKET_RX           0x00000004
#define RTDBG_FLAG_VRRP_PACKET_DETAIL       0x00000008
#define RTDBG_FLAG_VRRP_ALL                 0x0000000f

/* PTP DEBUG */
typedef enum
{
    PTPDBG_TYPE_PTP = 0,
    PTPDBG_TYPE_MAX
} ptpdbg_type_t;

/* PTP DEBUG */
#define PTPDBG_FLAG_COMMUNICATION_DBG                  0x00000001
#define PTPDBG_FLAG_BMC_DBG             0x00000002
#define PTPDBG_FLAG_TIMER_DBG             0x00000004
#define PTPDBG_FLAG_SYNCHRONIZATION_DBG           0x0000008
#define PTPDBG_FLAG_PACKET_RX_DBG          0x00000010
#define PTPDBG_FLAG_PACKET_TX_DBG            0x00000020
#define PTPDBG_FLAG_PACKET_DISCARD_DBG            0x00000040

/* IPSLA DEBUG */
#define RTDBG_FLAG_IPSLA_EVENT              0x00000001
#define RTDBG_FLAG_IPSLA_PACKET             0x00000002

/* TRACK DEBUG */
#define RTDBG_FLAG_TRACK_EVENT              0x00000001

/* CHSM DEBUG */
#define CHSMDBG_FLAG_CHSM_CHASSIS           0x00000001

/* RIP DEBUG */
#define RIPDBG_FLAG_EVENT            	    0x00000001
#define RIPDBG_FLAG_PACKET            	    0x00000002
#define RIPDBG_FLAG_ZEBRA            	    0x00000004

/* OSPF DEBUG */
#define OSPFDBG_FLAG_EVENT                  0x00000001
#define OSPFDBG_FLAG_ISM_EVENTS             0x00000002
#define OSPFDBG_FLAG_ISM_STATUS             0x00000004
#define OSPFDBG_FLAG_ISM_TIMERS             0x00000008
#define OSPFDBG_FLAG_LSA_FLOODING           0x00000010
#define OSPFDBG_FLAG_LSA_GENERATE           0x00000020
#define OSPFDBG_FLAG_LSA_INSTALL            0x00000040
#define OSPFDBG_FLAG_LSA_REFRESH            0x00000080
#define OSPFDBG_FLAG_NSM_EVENTS             0x00000100
#define OSPFDBG_FLAG_NSM_STATUS             0x00000200
#define OSPFDBG_FLAG_NSM_TIMERS             0x00000400
#define OSPFDBG_FLAG_NSSA                   0x00000800
#define OSPFDBG_FLAG_PACKET_DD              0x00001000
#define OSPFDBG_FLAG_PACKET_HELLO           0x00002000
#define OSPFDBG_FLAG_PACKET_LS_ACK          0x00004000
#define OSPFDBG_FLAG_PACKET_LS_REQUEST      0x00008000
#define OSPFDBG_FLAG_PACKET_LS_UPDATE       0x00010000
#define OSPFDBG_FLAG_ZEBRA_INTERFACE        0x00020000
#define OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE     0x00040000
#define OSPFDBG_FLAG_EVENT_ABR              0x00080000
#define OSPFDBG_FLAG_EVENT_IA               0x00100000
#define OSPFDBG_FLAG_EVENT_ROUTE            0x00200000
#define OSPFDBG_FLAG_EVENT_SPF              0x00400000

/* BGP DEBUG */
#define BGPDBG_FLAG_AS4_SEGMENT             0x00000001
#define BGPDBG_FLAG_EVENT                   0x00000002
#define BGPDBG_FLAG_FILTERS                 0X00000004
#define BGPDBG_FLAG_FSM                     0X00000008
#define BGPDBG_FLAG_KEEPALIVES              0X00000010
#define BGPDBG_FLAG_UPDATES_IN              0X00000020
#define BGPDBG_FLAG_UPDATES_OUT             0X00000040
#define BGPDBG_FLAG_ZEBRA                   0X00000080

/* IPMC DEBUG */
#define IGMPDBG_FLAG_EVENT                  0x00000001
#define IGMPDBG_FLAG_PACKET                 0x00000002
#define IGMPDBG_FLAG_TRACE                  0x00000004

#define MROUTEDBG_FLAG_ON                   0x00000001
#define MROUTEDBG_FLAG_DETAIL               0x00000002

#define PIMDBG_FLAG_EVENT                   0x00000001
#define PIMDBG_FLAG_PACKET                  0x00000002
#define PIMDBG_FLAG_PACKET_DUMP_SEND        0x00000004
#define PIMDBG_FLAG_PACKET_DUMP_RCV         0x00000008
#define PIMDBG_FLAG_TRACE                   0X00000010
#define PIMDBG_FLAG_ZEBRA                   0X00000020

/* ND DEBUG*/
#define RTDBG_FLAG_ND_EVENT                 0x00000001
#define RTDBG_FLAG_ND_PACKET               0x00000002
#define RTDBG_FLAG_ND_PROTO                0x00000004
#define RTDBG_FLAG_ND_TIMER                 0x00000008

/* NAT DEBUG*/
#define RTDBG_FLAG_NAT_EVENT                 0x00000001
#define RTDBG_FLAG_NAT_TIMER                 0x00000002

/* QUAGGA DEBUG */
typedef enum
{
    QUAGGADBG_TYPE_OSPF = 0,
    QUAGGADBG_TYPE_BGP,
    QUAGGADBG_TYPE_IGMP,
    QUAGGADBG_TYPE_MROUTE,
    QUAGGADBG_TYPE_PIM,
    QUAGGADBG_TYPE_MAX
} quaggadbg_type_t;

/* HSRV DEBUG */
typedef enum
{
    HSRVDBG_TYPE_VSWITCH = 0,
    HSRVDBG_TYPE_INTERFACE,
    HSRVDBG_TYPE_MESSAGE,
    HSRVDBG_TYPE_CPU,
    HSRVDBG_TYPE_IP,
    HSRVDBG_TYPE_NEXTHOP,
    HSRVDBG_TYPE_VLAN,
    HSRVDBG_TYPE_FDB,
    HSRVDBG_TYPE_L2MC,
    HSRVDBG_TYPE_IPMC,
    HSRVDBG_TYPE_AGG,
    HSRVDBG_TYPE_IPUC,
    HSRVDBG_TYPE_NEIGHBOR,
    HSRVDBG_TYPE_ECMP,
    HSRVDBG_TYPE_ACL,
    HSRVDBG_TYPE_STATS,
    HSRVDBG_TYPE_QOS,
    HSRVDBG_TYPE_MIRROR,
    HSRVDBG_TYPE_STP,
    HSRVDBG_TYPE_PTP,
    HSRVDBG_TYPE_TPOAM,
    HSRVDBG_TYPE_L2PROTOCOL,
    HSRVDBG_TYPE_MAX
} hsrvdbg_type_t;

#define HSRVDBG_FLAG_VSWITCH_VSWITCH         0x00000001

#define HSRVDBG_FLAG_INTERFACE_L2IF         0x00000001
#define HSRVDBG_FLAG_INTERFACE_L3IF         0x00000002
#define HSRVDBG_FLAG_INTERFACE_IFDB         0x00000004

#define HSRVDBG_FLAG_MESSAGE_PM2HSRV        0x00000001
#define HSRVDBG_FLAG_MESSAGE_HSRV2PM        0x00000002

#define HSRVDBG_FLAG_CPU_TRAFFIC_RX          0x00000001
#define HSRVDBG_FLAG_CPU_TRAFFIC_TX          0x00000002
#define HSRVDBG_FLAG_CPU_TRAFFIC_BPDU        0x00000004
#define HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO  0x00000008
#define HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL       0x00000010
#define HSRVDBG_FLAG_CPU_TRAFFIC_LLDP        0x00000020
#define HSRVDBG_FLAG_CPU_TRAFFIC_ERPS        0x00000040
#define HSRVDBG_FLAG_CPU_TRAFFIC_MACDA       0x00000080
#define HSRVDBG_FLAG_CPU_TRAFFIC_RIP         0x00000100
#define HSRVDBG_FLAG_CPU_TRAFFIC_OSPF        0x00000200
#define HSRVDBG_FLAG_CPU_TRAFFIC_BGP         0x00000400
#define HSRVDBG_FLAG_CPU_TRAFFIC_ARP         0x00000800
#define HSRVDBG_FLAG_CPU_TRAFFIC_DHCP        0x00001000
#define HSRVDBG_FLAG_CPU_TRAFFIC_IPDA        0x00002000
#define HSRVDBG_FLAG_CPU_TRAFFIC_IGMP        0x00004000
#define HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT   0x00008000
#define HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH  0x00010000
#define HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU  0x00020000
#define HSRVDBG_FLAG_CPU_TRAFFIC_OTHER       0x00040000
#define HSRVDBG_FLAG_CPU_TRAFFIC_RAW         0x00080000
#define HSRVDBG_FLAG_CPU_TRAFFIC_PTP         0x00100000

#define HSRVDBG_FLAG_IP_ICMP                0x00000001

#define HSRVDBG_FLAG_NEXTHOP_NEXTHOP        0x00000001

#define HSRVDBG_FLAG_VLAN_VLAN              0x00000001

#define HSRVDBG_FLAG_MIRROR_MIRROR      0x00000001

#define HSRVDBG_FLAG_FDB_FDB                0x00000001

#define HSRVDBG_FLAG_L2MC_L2MC              0x00000001

#define HSRVDBG_FLAG_AGG_AGG                0x00000001

#define HSRVDBG_FLAG_IPUC_IPUC              0x00000001

#define HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR      0x00000001

#define HSRVDBG_FLAG_ECMP_ECMP              0x00000001

#define HSRVDBG_FLAG_ACL_ACL                0x00000001

#define HSRVDBG_FLAG_STATS_STATS            0x00000001

#define HSRVDBG_FLAG_IPSOUR_IPSOUR          0x00000001

#define HSRVDBG_FLAG_QOS_CLASS              0x00000001
#define HSRVDBG_FLAG_QOS_POLICER            0x00000002
#define HSRVDBG_FLAG_QOS_PROCESS            0x00000004
#define HSRVDBG_FLAG_QOS_QUEUE              0x00000008

#define HSRVDBG_FLAG_DOT1X_DOT1X            0x00000001

#define HSRVDBG_FLAG_PTP_PKT_RX                0x00000001
#define HSRVDBG_FLAG_PTP_PKT_TX                0x00000002
#define HSRVDBG_FLAG_PTP_CFG                0x00000004
#define HSRVDBG_FLAG_PTP_INTERRUPT                0x00000008

#define HSRVDBG_FLAG_IPMC_IPMC              0x00000001

#define HSRVDBG_FLAG_TPOAM_COMMON          0x00000001
#define HSRVDBG_FLAG_TPOAM_EVENT              0x00000002
#define HSRVDBG_FLAG_TPOAM_UPDATE              0x00000004
#define HSRVDBG_FLAG_TPOAM_PKT_RX   0x00000008
#define HSRVDBG_FLAG_TPOAM_PKT_TX   0x00000010

#define HSRVDBG_FLAG_L2PROTOCOL_PKT_RX   0x00000001
#define HSRVDBG_FLAG_L2PROTOCOL_PKT_TX   0x00000002

#define HSRVDBG_FLAG_NAT_RULE            0x00000001
#define HSRVDBG_FLAG_NAT_SESSION         0x00000002
#define HSRVDBG_FLAG_NAT_TIMER           0x00000004

#define HSRVDBG_FLAG_G8032_EVENT           0x00000001



/* HAGT DEBUG */
typedef enum
{
    HAGTDBG_TYPE_INTERFACE = 0,
    HAGTDBG_TYPE_CPU,
    HAGTDBG_TYPE_NEXTHOP,
    HAGTDBG_TYPE_VLAN,
    HAGTDBG_TYPE_FDB,
    HAGTDBG_TYPE_L2MC,
    HAGTDBG_TYPE_AGG,
    HAGTDBG_TYPE_IPUC,
    HAGTDBG_TYPE_NEIGHBOR,
    HAGTDBG_TYPE_ECMP,
    HAGTDBG_TYPE_ACL,
    HAGTDBG_TYPE_STATS,
    HAGTDBG_TYPE_QOS,
    HAGTDBG_TYPE_MIRROR,
    HAGTDBG_TYPE_MAX
} hagtdbg_type_t;
#define HSRVDBG_FLAG_STP_STP                0x00000001

#define HAGTDBG_FLAG_INTERFACE_L2IF         0x00000001
#define HAGTDBG_FLAG_INTERFACE_L3IF         0x00000002
#define HAGTDBG_FLAG_INTERFACE_IFDB         0x00000004

#define HAGTDBG_FLAG_CPU_PDU                0x00000001
#define HAGTDBG_FLAG_CPU_TRAFFIC            0x00000002

#define HAGTDBG_FLAG_NEXTHOP_NEXTHOP        0x00000001

#define HAGTDBG_FLAG_VLAN_VLAN              0x00000001

#define HAGTDBG_FLAG_FDB_FDB                0x00000001

#define HAGTDBG_FLAG_L2MC_L2MC              0x00000001

#define HAGTDBG_FLAG_AGG_AGG                0x00000001

#define HAGTDBG_FLAG_IPUC_IPUC              0x00000001

#define HAGTDBG_FLAG_NEIGHBOR_NEIGHBOR      0x00000001

#define HAGTDBG_FLAG_ECMP_ECMP              0x00000001

#define HAGTDBG_FLAG_ACL_ACL                0x00000001

#define HAGTDBG_FLAG_STATS_STATS            0x00000001

#define HAGTDBG_FLAG_QOS_CLASS              0x00000001
#define HAGTDBG_FLAG_QOS_POLICER            0x00000002
#define HAGTDBG_FLAG_QOS_PROCESS            0x00000004
#define HAGTDBG_FLAG_QOS_QUEUE              0x00000008
#define HAGTDBG_FLAG_QOS_FLOW               0x00000010
/* HAGT DEBUG */
typedef enum
{
    LSRVDBG_TYPE_CARD = 0,
    LSRVDBG_TYPE_SYNCE,
    LSRVDBG_TYPE_MAX
} lsrvdbg_type_t;

#define LSRVDBG_FLAG_SYNCE_SYNCE       0x00000001

#define LSRVDBG_FLAG_CARD_CARD         0x00000001
#define LSRVDBG_FLAG_CARD_PORT         0x00000002
#define LSRVDBG_FLAG_CARD_FIBER        0x00000004
#define LSRVDBG_FLAG_CARD_FAN          0x00000008
#define LSRVDBG_FLAG_CARD_PSU          0x00000010
#define LSRVDBG_FLAG_CARD_LED          0x00000020
#define LSRVDBG_FLAG_CARD_SENSOR       0x00000040
#define LSRVDBG_FLAG_CARD_VCM          0x00000080
#define LSRVDBG_FLAG_CARD_POE          0x00000100

/* LCM DEBUG */
#define LCMDBG_FLAG_LCM_CARD           0x00000001
#define LCMDBG_FLAG_LCM_LCMGT          0x00000002
#define LCMDBG_FLAG_LCM_CHSMTLK        0x00000004
#define LCMDBG_FLAG_LCM_LCSH           0x00000008


/* DHCP relay/snooping/snoopingv6 debug, please keep sequence!!! */
#define DHCDBG_FLAG_RELAY_ERROR         0x1
#define DHCDBG_FLAG_RELAY_EVENTS        0x2
#define DHCDBG_FLAG_RELAY_PACKET        0x4
#define DHCDBG_FLAG_RELAY_DUMP          0x8
#define DHCDBG_FLAG_SNOOPING_ERROR      0x10
#define DHCDBG_FLAG_SNOOPING_EVENTS     0x20
#define DHCDBG_FLAG_SNOOPING_PACKET     0x40
#define DHCDBG_FLAG_SNOOPING_DUMP       0x80
#define DHCDBG_FLAG_SNOOPINGV6_ERROR    0x100
#define DHCDBG_FLAG_SNOOPINGV6_EVENTS   0x200
#define DHCDBG_FLAG_SNOOPINGV6_PACKET   0x400
#define DHCDBG_FLAG_SNOOPINGV6_DUMP     0x800

#define DHCDBG_FLAG_CLIENT_ERROR        0x1000
#define DHCDBG_FLAG_CLIENT_EVENTS       0x2000
#define DHCDBG_FLAG_CLIENT_PACKET       0x4000
#define DHCDBG_FLAG_CLIENT_DUMP         0x8000

#define DHCDBG_FLAG_CLIENTV6_ERROR        0x10000
#define DHCDBG_FLAG_CLIENTV6_EVENTS       0x20000
#define DHCDBG_FLAG_CLIENTV6_PACKET       0x40000
#define DHCDBG_FLAG_CLIENTV6_DUMP          0x80000

#define DHCPV6DBG_FLAG_CLIENTV6_ERROR        0x1
#define DHCPV6DBG_FLAG_CLIENTV6_EVENTS       0x2
#define DHCPV6DBG_FLAG_CLIENTV6_PACKET       0x4
#define DHCPV6DBG_FLAG_CLIENTV6_DUMP          0x8

/* OPM DEBUG */
typedef enum
{
    OPM_TYPE_ERPS = 0,
    OPM_TYPE_G8131,
    OPM_TYPE_TPOAM,
    OPM_TYPE_G8032,
    OPM_TYPE_CFM,
    OPM_TYPE_MAX
} opmdbg_type_t;

#define OPMDBG_FLAG_ERPS_PACKET         0x01
#define OPMDBG_FLAG_ERPS_TIMER          0x02
#define OPMDBG_FLAG_ERPS_PROTO          0x04
#define OPMDBG_FLAG_ERPS_EVENTS         0x08
#define OPMDBG_FLAG_ERPS_ALL            0x0f

#define OPMDBG_FLAG_G8131_TX            0x01
#define OPMDBG_FLAG_G8131_RX            0x02
#define OPMDBG_FLAG_G8131_EVENT         0x04

/* TPOAM DEBUG */
#define OPMDBG_FLAG_TPOAM_LB_LBM_DBG                  0x00000001
#define OPMDBG_FLAG_TPOAM_LB_LBR_DBG                  0x00000002
#define OPMDBG_FLAG_TPOAM_LM_COMM_DBG              0x00000004
#define OPMDBG_FLAG_TPOAM_LM_LMM_DBG                 0x00000008
#define OPMDBG_FLAG_TPOAM_LM_LMR_DBG                  0x00000010
#define OPMDBG_FLAG_TPOAM_DM_COMM_DBG               0x00000020
#define OPMDBG_FLAG_TPOAM_DM_DMM_DBG                 0x00000040
#define OPMDBG_FLAG_TPOAM_DM_DMR_DBG                  0x00000080
#define OPMDBG_FLAG_TPOAM_DM_1DMTX_DBG              0x00000100
#define OPMDBG_FLAG_TPOAM_DM_1DMRX_DBG              0x00000200
#define OPMDBG_FLAG_TPOAM_PACKET_RX_DBG              0x00000400
#define OPMDBG_FLAG_TPOAM_PACKET_TX_DBG              0x00000800
#define OPMDBG_FLAG_TPOAM_EVENT_DBG                     0x00001000

#define OPMDBG_FLAG_G8032_TX            0x01
#define OPMDBG_FLAG_G8032_RX            0x02
#define OPMDBG_FLAG_G8032_EVENT         0x04
#define OPMDBG_FLAG_G8032_TIMER         0x08
#define OPMDBG_FLAG_G8032_ALL         0x10
/* ROUTED DEBUG */
typedef enum
{
    AUDBG_TYPE_FLAGS = 0,
    AUDBG_TYPE_MAX
} audbg_type_t;

/* AUTHD debug, please keep sequence!!! */
#define AUTHD_DBG_FLAG_EVENT            0x1
#define AUTHD_DBG_FLAG_PACKET           0x2
#define AUTHD_DBG_FLAG_PROTO            0x4
#define AUTHD_DBG_FLAG_TIMER            0x8

#define DOT1X_FLAG_PACKET        0x01
#define DOT1X_FLAG_TIMER         0x02
#define DOT1X_FLAG_PROTO         0x04
#define DOT1X_FLAG_EVENTS        0x08
#define DOT1X_FLAG_ALL           0x0f

#define SFLOW_FLAG_COUNTER       0x01
#define SFLOW_FLAG_SAMPLE        0x02
#define SFLOW_FLAG_PACKET        0x04
#define SFLOW_FLAG_ALL           0x07

/*SSM DEBUG*/
#define SSM_DEBUG_FLAG_EVENT        0x01
#define SSM_DEBUG_FLAG_PACKET_TX    0x02
#define SSM_DEBUG_FLAG_PACKET_RX    0x04
#define SSM_DEBUG_FLAG_TIMER        0x08

/* CFM DEBUG */
#define OPMDBG_FLAG_CFM_EVENT            0x1
#define OPMDBG_FLAG_CFM_PACKET           0x2
#define OPMDBG_FLAG_CFM_TIMER            0x4
#define OPMDBG_FLAG_CFM_ALL                0x07

/* OPM DEBUG */
typedef enum
{
    SSM_TYPE_SSM = 0,
    SSM_TYPE_MAX
} ssmdbg_type_t;


/* APPCFG DEBUG */
typedef enum
{
    APPDBG_TYPE_SNMP = 0,
    APPDBG_TYPE_RMON,
    APPDBG_TYPE_NTP,
    APPDBG_TYPE_MAX
} appdbg_type_t;

/* SNMP DEBUG */
#define APPDBG_FLAG_SNMP_AGENT              0x00000001

/* RMON DEBUG */
#define APPDBG_FLAG_RMON_EVENT              0x00000001
#define APPDBG_FLAG_RMON_ALARM              0x00000002
#define APPDBG_FLAG_RMON_STATS              0x00000004
#define APPDBG_FLAG_RMON_HISTORY            0x00000008

/* NTP DEBUG */
#define APPDBG_FLAG_NTP_DEBUG_LEVEL         0x00000001

/* NETCONF DEBUG */
#define APPDBG_FLAG_NETCONF_RPC              0x00000001
#define APPDBG_FLAG_NETCONF_RPC_CLEAR   0x00000002
#define APPDBG_FLAG_NETCONF_DEBUG          0x00000004

/* OPENFLOW DEBUG */
typedef enum
{
    OPENFLOWDBG_TYPE_OPENFLOW = 0,
    OPENFLOWDBG_TYPE_MAX
}openflowdbg_type_t;

#define OPENFLOWDBG_FLAG_VCONN      0x00000001
#define OPENFLOWDBG_FLAG_RCONN      0x00000002

#define DEBUG_DECLAR(name)    \
extern tbl_##name##_t * _g_##name##_handle; \
extern int _g_module_##name##_id;

#define DEBUG_DEFINE(name)    \
tbl_##name##_t * _g_##name##_handle = NULL; \
int _g_module_##name##_id = -1;

#define DEBUG_INIT(name, modid)  \
do { \
    _g_##name##_handle = tbl_##name##_get_##name(); \
    _g_module_##name##_id = (modid); \
} while (0)

#define IS_DEBUG_ON(name, var, tblid) \
(_g_##name##_handle->var & (1 << (tblid)))

#define DEBUG_OUT(name, var, tblid, ...) \
do                                                                              \
{                                                                               \
    if ((_g_##name##_handle->var & (1 << (tblid))))                           \
    {                                                                           \
        log_sys(_g_module_##name##_id, E_DEBUG,                                 \
                   __VA_ARGS__);                                                \
    }                                                                           \
}                                                                               \
while (0)

extern void
gen_dbg_print_hex_emacs(int modid, const u_char *cp, unsigned int length, unsigned int offset);

#define DEBUG_OUT_HEX(name, var, tblid, msg, buf, len, offset) \
do                                                                                   \
{                                                                                    \
    if ((_g_##name##_handle->var & (1 << (tblid))))                                \
    {                                                                                \
        log_sys(_g_module_##name##_id, E_DEBUG, msg);                                \
        gen_dbg_print_hex_emacs(_g_module_##name##_id, (buf), (len), (offset));     \
    }                                                                                \
}                                                                                    \
while (0)

#define DEBUG_SHOW(name, var, ...) \
do                                                                              \
{                                                                               \
    log_sys(_g_module_##name##_id, E_DEBUG,              \
                   __VA_ARGS__);                                               \
}                                                                              \
while (0)

#define DEBUG_SHOW_HEX(name, var, msg, buf, len, offset) \
do                                                                                \
{                                                                                 \
    log_sys(_g_module_##name##_id, E_DEBUG, msg);                                 \
    gen_dbg_print_hex_emacs(_g_module_##name##_id, (buf), (len), (offset));       \
}                                                                                 \
while (0)

/*add by zhw for PIMSM DEBUG*/
#define RTDBG_FLAG_PIMSM_ALL				0x00000001
#define RTDBG_FLAG_PIMSM_PIM				0x00000002
#define RTDBG_FLAG_PIMSM_IGMP				0x00000004
#define RTDBG_FLAG_PIMSM_TIMER				0x00000008
/*end add zhw*/

#endif /* !__GLB_DEBUG_DEFINE_H__ */
