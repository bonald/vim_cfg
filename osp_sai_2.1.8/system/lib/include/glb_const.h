
#ifndef __GLB_CONST_H__
#define __GLB_CONST_H__

#include "glb_macro.h"

#define OEM_CENTEC_YEAR                 "2019"

#ifndef CNOS_ARCH
/* supervisor and linecard connection msg */
#define LCM_MSG_VRF                     65
#define LCM_MSG_IP_SERIES_E800          INADDR_ANY
#define LCM_MSG_PORT                    6000
#define LCM_BUF_SZ                      0x1000
#define LCM_SRV_CB_TBL_SZ               0x100
#endif

#define KEEPALIVE_TIME_OUT_COUNT        6
#define KEEPALIVE_INTERVAL              3000
#define PHY_STATUS_MONITOR_INTERVAL     1000
#define FIBER_MODULE_UPDATE_COUNT       5

#define GLB_NAME_INFO_STR_MAX 256
#define GLB_SLOT_NUM_MAX_STACKING     31
#define GLB_SLOT_NUM_MAX            GLB_SLOT_NUM_MAX_STACKING

/** lai_card_status */
typedef enum glb_card_status_e {
    GLB_CARD_STATUS_READY = (1 << 0),
    GLB_CARD_STATUS_INITED = (1 << 1),
    GLB_CARD_STATUS_ALARM = (1 << 2),
    GLB_CARD_STATUS_MAX,
} glb_card_status_t;

#define GLB_CHSM_PORT_NUM_MAX       512

#define ETH_P_IPV4              0x0800
#define ETH_P_IPV6              0x86DD
#define ETH_P_PAE               0x888E
#define ETH_P_CFM               0x8902
#define ETH_P_SLOW_PROTOCOL     0x8809
#define ETH_P_LLDP              0x88CC
#define ETH_P_EAPOL             0x888C
#define ETH_P_BPDU              0x3033
#define ETH_P_ARP               0x0806
#define ETH_P_MPLS              0x8847
#define ETH_P_MPLS_MCAST        0x8848


#define GLB_MAX_DESC_STR_LEN            16

#define GLB_MAX_DPID_STR_LEN            20
#define GLB_MAX_DPID_CHECK_LEN            16

#define VLAN_TAG_LEN                    4
#define MAC_ADDR_LEN                    6
#define GLB_ETH_ADDR_LEN                MAC_ADDR_LEN
#define ETH_HEADER_LEN                  14  /* MAC_ADDR_LEN * 2 + 2 */
#define MAC_ADDR_STR_LEN                16  /* XXXX.XXXX.XXXX */

#define UDP_HEADER_LEN 8
#define IP_HEADER_LEN 20

#define GLB_DEFAULT_TPID                0x8100

#define GLB_DEF_VLAN_ID                 1
#define GLB_MAX_VLAN_ID                 4095
#define GLB_MIN_VLAN_ID                 0
#define GLB_ALL_VLAN_ID                 0
#define GLB_MAX_VLAN_STATS_ID                 128
#define GLB_BITS_NUM_OF_BYTE            8
#define GLB_BITS_NUM_OF_WORD            32
#define GLB_BITS_SHIFT_OF_WORD          5
#define GLB_BITS_MASK_OF_WORD           0x1F
#define GLB_MAX_UINT16_VALUE            0xFFFF 
#define GLB_MAX_UINT32_VALUE            0xFFFFFFFF 
#define GLB_MAX_UINT64_VALUE            0xFFFFFFFFFFFFFFFFLL
#define GLB_MAX_INT64_VALUE             ~((int64)1 << (8*sizeof(int64) -1))
#define GLB_MAX_UINT8_VALUE             0xFF
#define GLB_IPV6_ADDR_LEN_IN_BYTE       16
#define GLB_IPV4_ADDR_LEN_IN_BYTE       4
#define GLB_IPV6_ADDR_LEN_IN_BIT        128
#define GLB_IPV4_ADDR_LEN_IN_BIT        32
#define GLB_MAX_CHIP_NUM                32
#define GLB_MAX_VTY_NUM                 8
#define GLB_MAX_USER_NUM                32
#define GLB_MAX_AUTHEN_LIST_NUM         16
#define GLB_MAX_RSA_NUM                 32
#define GLB_MAX_DNS_HOST_NUM            1024
#define GLB_MAX_AUTHOR_LIST_NUM         16
#define GLB_MAX_ACCOUNT_LIST_NUM        16
#define GLB_MAX_ACCOUNTCMD_LIST_NUM     16

#define GLB_MAX_PRIVILEAGE_NUM          4

#define GLB_VLANCLASS_RULE_ID_MAX       4096 
#define GLB_VLANCLASS_GROUP_ID_MAX      31

#define MAX_DIAG_ENTRY_NUM              128
#define MAX_DUMP_CHIP_MOD_DATA_NUM      132

#define GLB_MAX_CTC_NEXTHOP_ID          16384
#define GLB_MAX_CTC_NEXTHOP_GROUP_ID    16384

/* DHCP */
#define GLB_MAX_DHCSRVGRP_NUM           16
#define GLB_MAX_DHCP_INTF_NUM           512

#define DHCPSNOOPING_ITEM_TYPE_LEARNING     0x00 /* by learning */
#define DHCPSNOOPING_ITEM_TYPE_CONFIGED     0x01 /* by admin config */

/* binding item limitation */
/*modified by weij for bug 17826, reduce entry size, 2012-02-15*/
#define DHCSNOOPINGV6_MAX_BINDING_ITEMS    4096
#define DHCSNOOPING_MAX_BINDING_ITEMS      4096

/* lease time */
#define DHCSNOOPING_DEFAULT_LEASE_TIME         1200 /* seconds */
#define DHCSNOOPING_DEFAULT_REQUEST_TIMEOUT    15 /* seconds */
#define DHCSNOOPING_DEFAULT_BOOTP_LEASE_TIME   86400 /* seconds = 1 day */

/* save binding database interval */
#define DHCSNOOPING_SAVE_BINDING_INTERVAL_MIN      15
#define DHCSNOOPING_SAVE_BINDING_INTERVAL_MAX      1200
#define DHCSNOOPING_SAVE_BINDING_INTERVAL_DEFAULT  600

#define DHCSNOOPING_MAX_VLANSEGS   16
#define DHCSNOOPING_MAX_CIRCUIT_ID_STRING   64
#define DHCSNOOPING_MAX_REMOTE_ID_STRING   64

#define DHCSNOOPING_TIMEOUT_INTERVAL   1

/* dhcp client */
#define DHCLIENT_BROADCAST_FLAG     0x1
/* by default the broadcast flags is on */
#define DHCLIENT_DEFAULT_FLAGS      (DHCLIENT_BROADCAST_FLAG)
#define DHCLIENT_DEFAULT_ROUTER_DISTANCE        254

/* events from PM to dhclient */
#define DHC_EVENT_ENABLE              0x1  /* Enable client function */
#define DHC_EVENT_DISABLE             0x2  /* Disable client function */
#define DHC_EVENT_RENEWAL             0x4  /* Admin renew */
#define DHC_EVENT_RELEASE             0x8  /* Admin release */
/* events from dhclient to PM */
#define DHC_EVENT_UNBOUND             0x100 /* Uninstall IP & routers */
#define DHC_EVENT_REBOOT              0x200 /* Reboot for smartconfig */

/* notify from dhclient to PM */
#define DHC_NTF_RELEASE_DONE        0x1  /* release done */
#define DHC_NTF_DISABLE_DONE        0x2  /* disable done */
#define DHC_NTF_PANIC               0x4  /* No DHCPOFFERS received */
/* notify from PM to dhclient */
#define DHC_NTF_BOUND_IP_DONE       0x100  /* install ip address done */
#define DHC_NTF_BOUND_ROUTER_DONE   0x200  /* install router done */
#define DHC_NTF_UNBOUND_DONE        0x400  /* uninstall IP & router done */

	/* for dhcp client request ... */
#define M_DHC_REQ_SUBNET_MASK                   0x1
#define M_DHC_REQ_BROADCAST                     0x2
#define M_DHC_REQ_HOSTNAME                      0x4
#define M_DHC_REQ_DNS_SRV                       0x8
#define M_DHC_REQ_DOMAIN_NAME                   0x10
#define M_DHC_REQ_NETBIOS_NAMESRV               0x20
#define M_DHC_REQ_CLASSLESS_STATIC_ROUTE        0x40
#define M_DHC_REQ_CLASSLESS_STATIC_ROUTE_MS     0x80
#define M_DHC_REQ_ROUTER                        0x100
#define M_DHC_REQ_STATIC_ROUTE                  0x200
#define M_DHC_REQ_TFTP_SERVER_IP_ADDR           0x400
#define M_DHC_REQ_VENDOR_SPEC                   0x800

#define GLB_DHCLIENT_FLAG_HEX             (1    << 0)
#define GLB_DHCLIENT_FLAG_ASCII           (1    << 1)
#define GLB_DHCLIENT_FLAG_PORT            (1    << 2)

#define GLB_VRRP_MAX_VRID           255

#define GLB_IPSLA_ENTRY_MAX         255
#define GLB_IPSLA_PKT_BUF           8116
#define GLB_IPSLA_PRESULT_MAX       1000
#define GLB_IPSLA_TRESULT_MAX       10

#define GLB_TRACK_OBJECT_MAX        500

#define GLB_CPU_MEM_MINUTE_SIZE     6
#define GLB_CPU_MEM_HOUR_SIZE       60
#define GLB_CPU_MEM_DAY_PERSIZE     10
#define GLB_CPU_MEM_DAY_SIZE        72

/*added by hansf for controller attribute*/
#define GLB_CONTROLLER_DEFAULT_MAX_BACKOFF     8
#define GLB_CONTROLLER_DEFAULT_INACTIVITY_PROBE    5

#define GLB_L3MC_MAX_MEMBER_NUM_PER_GROUP 64
#define GLB_L3MC_MAX_ROUTE_ENTRY_NUM      1024

#define DHCPV6_UDP_SRC_PORT      546
#define DHCPV6_UDP_DST_PORT      547

#define IPV6_MAX_BYTELEN    16
#define IPV6_HDR_LEN            40
#define UDP_HDR_LEN               8

/* modified by liwh for bug 49365, 2018-11-29 */
#define     DHCPV6_CLIENT_DEFAULT_IANA_T1_TIME        3600  /* 1 hour */
#define     DHCPV6_CLIENT_DEFAULT_IANA_T2_TIME        5400  /* 1.5 hour */
/* liwh end */

typedef struct dhc_req_option
{
	unsigned code; /* option code */
	u_int32_t flag; /* M_DHCP_REQ_... */
	char *name;
} dhc_req_option_t;

/* modified by liwh for bug 49365, 2018-11-26 */
typedef struct mac_header_s
{
    u_int8_t   dst_mac[6];
    u_int8_t   src_mac[6];
    u_int16_t  eth_type;
} mac_header_t;
/*liwh end */

#define M_DHC_REQ_MUST (M_DHC_REQ_SUBNET_MASK|M_DHC_REQ_BROADCAST|M_DHC_REQ_HOSTNAME)
#define M_DHC_REQ_DEFAULT (M_DHC_REQ_MUST | M_DHC_REQ_CLASSLESS_STATIC_ROUTE | M_DHC_REQ_ROUTER | M_DHC_REQ_TFTP_SERVER_IP_ADDR | M_DHC_REQ_CLASSLESS_STATIC_ROUTE_MS | M_DHC_REQ_STATIC_ROUTE)
	
	/* be careful to keep the order */
#define M_DHC_REQ_OPTION_ARR(var) \
	dhc_req_option_t var[] = { \
		{1, M_DHC_REQ_SUBNET_MASK, "subnet-mask"}, \
		{28, M_DHC_REQ_BROADCAST, "broadcast-addr"}, \
		{12, M_DHC_REQ_HOSTNAME, "host-name"}, \
		{6, M_DHC_REQ_DNS_SRV, "dns-nameserver"}, \
		{15, M_DHC_REQ_DOMAIN_NAME, "domain-name"}, \
		{44, M_DHC_REQ_NETBIOS_NAMESRV, "netbios-nameserver"}, \
		{121, M_DHC_REQ_CLASSLESS_STATIC_ROUTE, "classless-static-route"}, \
		{249, M_DHC_REQ_CLASSLESS_STATIC_ROUTE_MS, "classless-static-route-ms"}, \
		{3, M_DHC_REQ_ROUTER, "router"}, \
		{33, M_DHC_REQ_STATIC_ROUTE, "static-route"}, \
		{150, M_DHC_REQ_TFTP_SERVER_IP_ADDR, "tftp-server-address"}, \
		{43, M_DHC_REQ_VENDOR_SPEC, "vendor-specific"}, \
		{0xFF, 0xFF, NULL} \
	}

/* AUTHD */
#define GLB_AUTH_SESSIONS_NUM           255

#define GLB_AUTH_RADIUS_SERVER_NUM      3
#define GLB_AUTH_TACPLUS_SERVER_NUM     3

#define GLB_AUTH_DEFAULT_RETRIES        3
#define GLB_AUTH_RETRIES_MIN            1
#define GLB_AUTH_RETRIES_MAX            100

#define GLB_AUTH_DEFAULT_TIMEOUT        5 /* 5 seconds */
#define GLB_AUTH_TIMEOUT_MIN            1 /* seconds */
#define GLB_AUTH_TIMEOUT_MAX            1000

#define GLB_AUTH_DEFAULT_DEADTIME       300 /* 5 minutes */
#define GLB_AUTH_DEADTIME_MIN           60 /* seconds */
#define GLB_AUTH_DEADTIME_MAX           1200 /* seconds */

#define GLB_AUTH_SECRET_SIZE                64 /* auth key string length */
#define GLB_AUTH_RADIUS_DEFAULT_PORT        1812
#define GLB_AUTH_TACPLUS_DEFAULT_PORT       49

#define GLB_AUTH_RESULT_BUF_SIZE            128 /* auth result message */

#define GLB_AUTH_MSG_BUF_SZ                 8192

/* scan port stats database interval */
#define GLB_PORT_STATS_INTERVAL   3    
#define GLB_PORT_POOL_STATS_INTERVAL    50  /* change to 50ms to enhance zabbix port stats */
#define GLB_PORT_DISCARD_STATS_INTERVAL 60
#define GLB_PORT_POOL_QUEUE_STATS_INTERVAL    100  /* added by hansf for dump-queue-stats 100ms per port 8 queue */

/* SNMP */
#define MAX_SNMP_VIEW_NUM               32
#define MAX_SNMP_COMMUNITY_NUM          16

#define GLB_ERROR_BASE                  -1000
#define GLB_OUTER_VLAN_IS_SVLAN         1
#define GLB_PROTO_ERROR_BASE            -2000
#define GLB_HSRV_ERROR_BASE             -3000
#define GLB_HAGT_ERROR_BASE             -4000
#define GLB_CTCLIB_ERROR_BASE           -5000
#define GLB_LCSH_ERROR_BASE             -6000

#define GLB_VLAN_HDR_LEN                4

#define GLB_INVALID_FD                  (-1)
#define GLB_INVALID_CHIPID              (-1)
#define GLB_INVALID_SLOTID              (-1)
#define GLB_INVALID_PORTID              (-1)

/* modified by wangjj for bug 43211, 2017-03-23 */
#define OSPF_AREA_LIMIT_DEFAULT                 3000

#define GLB_PETH_PKT_BUF_LEN            9800
#define GLB_ETH_P_ASIC                  0x5A5A
#define GLB_NETLINK_MSG_TYPE            0xA5A5

#define GLB_DEF_VRFID                   0
#define GLB_MAX_VRFID                   64
#define GLB_MAX_VRF_NAME_LEN            16

#define M_HOSTNAME_MINSIZE 1
#define M_HOSTNAME_MAXSIZE    63

/* Trap type. */
#define GLB_TRAP_TYPE_COLDSTART         1
#define GLB_TRAP_TYPE_WARMSTART        (1 << 1)
#define GLB_TRAP_TYPE_LINKDOWN         (1 << 2)
#define GLB_TRAP_TYPE_LINKUP           (1 << 3)
#define GLB_TRAP_TYPE_SYSTEM           (1 << 4)
#define GLB_TRAP_TYPE_VRRP             (1 << 5)
#define GLB_TRAP_TYPE_PTP_MASTER_CHANGE             (1 << 6)
#define GLB_TRAP_TYPE_PTP_STATE_CHANGE             (1 << 7)
#define GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE             (1 << 8)

#define GLB_TRAP_TYPE_ALL  GLB_TRAP_TYPE_COLDSTART | GLB_TRAP_TYPE_WARMSTART \
                         | GLB_TRAP_TYPE_LINKDOWN | GLB_TRAP_TYPE_LINKUP \
                         | GLB_TRAP_TYPE_SYSTEM | GLB_TRAP_TYPE_VRRP

#define GLB_TRAP_ALL_CONFIG_TIMER      30
#define GLB_SNMP_ENABLE_CONFIG_TIMER   30

#define GLB_SFLOW_COLLECTOR_DEFAULT_PORT        6343

#define GLB_SFLOW_TIME_SEC2MSEC 1000  /* msec */

typedef enum
{
    GLB_EFD_GRANU_4B = 2,
    GLB_EFD_GRANU_8B = 3,
    GLB_EFD_GRANU_16B = 4,
    GLB_EFD_GRANU_32B = 5,
} glb_efd_granularity_e;
#define GLB_EFD_DEFAULT_SPEED 50/*50Mbps*/
#define GLB_EFD_DEFAULT_TIMEINTERVAL 30/*30ms*/
#define GLB_EFD_DEFAULT_GRANU GLB_EFD_GRANU_16B
#define GLB_EFD_DEFAULT_IPG_EN 0
#define GLB_EFD_DEFAULT_AGINGTIMER 30/*30ms*/

/*Added by xgu for customer requirement, bug 38251, 2016-4-29, start*/
typedef enum
{
    GLB_EFD_GLB_PROP_CFG_GRANULARITY         = (1<<0),
    GLB_EFD_GLB_PROP_CFG_SPEED               = (1<<1),
    GLB_EFD_GLB_PROP_CFG_TIME_INTERVAL       = (1<<2),
    GLB_EFD_GLB_PROP_CFG_AGING_TIMER         = (1<<3),
    GLB_EFD_GLB_PROP_CFG_IPGEN               = (1<<4),
    GLB_EFD_GLB_PROP_CFG_REDIRECT_IFINDEX    = (1<<5),
    GLB_EFD_GLB_PROP_CFG_PER_FLOW_REDIRECT_IFINDEX    = (1<<6),
    GLB_EFD_UPDATE_VALID_FLOW_ENTRY_BMP      = (1<<7),
    
} glb_efd_prop_cfg_e;

typedef enum
{
    GLB_OPF_PKT_TYPE_STP                        = 1,
    GLB_OPF_PKT_TYPE_LACP,
    GLB_OPF_PKT_TYPE_LLDP,
    GLB_OPF_PKT_TYPE_IGMP,
    GLB_OPF_PKT_TYPE_ERPS,
    GLB_OPF_PKT_TYPE_DOT1X,
    GLB_OPF_PKT_TYPE_ARP,
    GLB_OPF_PKT_TYPE_DHCP,
    GLB_OPF_PKT_TYPE_OSPF,
    GLB_OPF_PKT_TYPE_PIM,
    GLB_OPF_PKT_TYPE_VRRP,
    GLB_OPF_PKT_TYPE_BGP,
    GLB_OPF_PKT_TYPE_MAX,

} glb_opf_pkt_type_e;

typedef enum
{
    GLB_OPF_PKT_ACTION_FORWARD,
    GLB_OPF_PKT_ACTION_OVS,
    GLB_OPF_PKT_ACTION_SYSTEM,
    GLB_OPF_PKT_ACTION_MAX,

} glb_opf_pkt_action_e;


typedef uint16_t vid_t;
typedef uint16_t vrf_id_t;

#define SO_VRF          100
#define SO_PERMVRF      101
#define SO_SETVRFID     102

#define MGMT_VRF        64

#define LCM_SRV_CB_TBL_SZ               0x100

/*Added by xgu, 2013-1-25, SDK mcast groupid is from 1, 
0 is reserved by SDK for stacking keeplive group, 
in this function _sys_greatbelt_nh_offset_init*/
#define GLB_MCAST_GROUPID_BASE     5000    /*reserve for CTC_OPF_SAI_L2MC_GROUP_ID*/
#define GLB_MCAST_GROUPID_FOR_MIRROR_0     (GLB_MCAST_GROUPID_BASE)

#define GLB_MCAST_GROUPID_RESERVED_MAX (GLB_MCAST_GROUPID_FOR_MIRROR_0 + 1)
#define GLB_MCAST_GROUPID_START   (GLB_MCAST_GROUPID_RESERVED_MAX)
#define GLB_SFLOW_MAX_RATE        1048576/*modify by yangl for bug 45988*/
#define GLB_TPID_INDEX_0   0
#define GLB_TPID_INDEX_1   1
#define GLB_TPID_INDEX_2   2
#define GLB_TPID_INDEX_3   3
#define GLB_ETH_P_8021Q     0x8100

#define GLB_MAX_ERPS_DOMAIN         256
#define GLB_DEFAULT_INSTANCE        0

#define OPMGLB_FLAG_ERPS_MODE_HUAWEI     0x1
#define OPMGLB_FLAG_ERPS_MODE_UNRELOAD   0x2

#define RADIUS_SHARED_SECRET_LEN                64
#define RADIUS_SERVER_MAX_COUNT                 3
#define AUTH_USER_NAMLEN                        63 
#define RADIUS_MAX_HOSTNAME_LEN                 64
#define RADIUS_MAX_STATE_LEN                    48
#define RADIUS_AUTHENTICATOR_LEN                16
#define RADIUS_SESSION_BUFFER                   256
#define RADIUS_RETRANSMIT_DEFAULT               3
#define RADIUS_TIMEOUT_DEFAULT                  5
#define RADIUS_DEADTIME_DEFAULT                 5

#define AUTH_PORT_CTRL_INVALID                  0
#define AUTH_PORT_CTRL_FORCE_UNAUTHORIZED       1 
#define AUTH_PORT_CTRL_FORCE_AUTHORIZED         2
#define AUTH_PORT_CTRL_AUTO                     3
#define AUTH_PORT_CTRL_MAX                      4

#define DOT1X_DEFAULT_PAE_REAUTHMAX      2
#define DOT1X_DEFAULT_PAE_QUIETPERIOD    60
#define DOT1X_DEFAULT_PAE_TXPERIOD       30
#define DOT1X_DEFAULT_REAUTH_PERIOD      3600
#define DOT1X_DEFAULT_BE_SUPP_TIMEOUT    30
#define DOT1X_DEFAULT_BE_SERVER_TIMEOUT  30

#define AUTH_EAPOL_PROTOCOL_VERSION_1    1  
#define AUTH_EAPOL_PROTOCOL_VERSION_2   2

#define AUTH_MAC_REJECT          (1<<0)
#define AUTH_MAC_ACCEPT          (1<<1)
#define AUTH_MAC_BYPASS          (1<<2)
#define AUTH_MAC_REAUTH_ACCEPT   (1<<3) /*only for bypass mac entry*/
#define AUTH_MAC_EAPOL_REAUTH    (1<<4) /*for eapol reauthentication*/

#define AUTH_PORT_MODE_PORT  0
#define AUTH_PORT_MODE_MAC   1

#define AUTH_CTRL_DIR_BOTH   0
#define AUTH_CTRL_DIR_IN     1

#define AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT      (1 << 0)
#define AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT   (1 << 1)
#define AUTH_RADIUS_CONFIG_DEFAULT_KEY          (1 << 2)

#define RADIUS_SERVER_PORT   1812

#define AUTH_PORT_STATUS_UNKNOWN            0
#define AUTH_PORT_STATUS_AUTHORIZED         1
#define AUTH_PORT_STATUS_UNAUTHORIZED       2
#define AUTH_PORT_STATUS_MAX                3

#define SESSION_TYPE_8021X                  0
#define SESSION_TYPE_USER_AUTHEN            1
#define SESSION_TYPE_MAC_AUTHEN_BYPASS      2

#define AUTH_EAPOL_HDR_LEN         4
#define AUTH_EAPOL_MIN_LEN         60
#define AUTH_EAPOL_MAX_LEN         1700

#define CISCO_PRIVILEGE_MIN    0
#define CISCO_PRIVILEGE_TWO    1
#define CISCO_PRIVILEGE_THREE  10
#define CISCO_PRIVILEGE_MAX    15
#define PRIVILEGE_INVALID      16

#define GLB_SYSTEM_OID 27975

#define VTY_PRIVILEGE_MIN             1
#define VTY_PRIVILEGE_LEVEL_TWO       2
#define VTY_PRIVILEGE_LEVEL_THREE     3
#define VTY_PRIVILEGE_VR_MAX          4

#define GLB_MAX_CMD_BUFSZ            1024  

#define GLB_SFLOW_PKT_BUF            1400   

#define GLB_SFLOW_DEFAULT_RATE       8192
#define GLB_SFLOW_USW_MAX_RATE       32768
#define GLB_SFLOW_GB_DEFAULT_RATE       16384
#define GLB_SFLOW_MAX_LOG_THRESHOLD  0x7FFF/*modify by yangl for bug 45988*/
#define GLB_SFLOW_MAX_LOG_UML_THRESHOLD  0x7FFF
#define GLB_SFLOW_MAX_LOG_GB_THRESHOLD  0x1FFF
#define GLB_SFLOW_INGRESS_PACKET     1
#define GLB_SFLOW_EGRESS_PACKET      2

/* modified by liwh for bug 44506, 2017-06-28 */
#define GLB_OSPF_NETWORK_MAX         32
/* liwh end */

#define GLB_OSPF_AREA_RANGE_MAX         32

/* modified by liwh for bug 43976, 2017-08-07 */
#define GLB_OSPF_DEAD_INTERVAL_MULTI_HELLO     4
/* liwh end */

#define DEFAULT_MGMT_IF_ADDR        "192.168.0.1"
#define DEFAULT_MGMT_IF_PREFIX_LEN  24

#define GLB_VRRP_PKT_BUF            256        


/*SSM*/
#define SSM_CLOCKID_STR_LEN         64
#define SSM_CLOCK_ID_LEN            8 
#define SSM_INTERN_INTERVAL_SEND    1000
#define SSM_INTERN_INTERVAL_RECV    5000
#define SSM_SWITCH_MODE_RESTORE_TIME_IS_3S 3
#define SSM_PDU_QLV_EVENT_FIELD     6
#define SSM_PDU_QLV_QUALITY_FIELD   13
#define SSM_PDU_QLV_EXTEND_TYPE_FIELD 14
#define SSM_PDU_QLV_CLOCKID_FIELD   17
#define SSM_PDU_QLV_HOP_FIELD       25

#define SSM_MAX_DEVICE_PORTS        50

/*custom e1 if-index */
#define SSM_E1_IFINDEX              9999

#define SSM_API_IGNORE_IFP_DOWN     (1<<0)
#define SSM_API_IGNORE_IFP_UP       (1<<1)

/*Port default dnu group id*/
#define SSM_DEFAULT_DNU_GROUP       0
#define SSM_DNU_GROUP_MIN           0
#define SSM_DNU_GROUP_MAX           255

/*SSM default priority*/
#define SSM_IF_DEFAULT_PRIORITY     255
#define SSM_CLOCK_PRIORITY_DEFAUTL  255
#define SSM_PRIORITY_MIN             1
#define SSM_PRIORITY_MAX            255

/*SSM default hop*/
#define SSM_DEFAULT_PATH_HOP       0

/* support nd modified by liwh for bug 47547, 2018-06-28 */
#define GLB_ND_PKT_BUF                   1500
/* liwh end */

enum  ssm_option_mode_e
{
    SSM_OPTION_MODE_1 = 1,
    SSM_OPTION_MODE_2
};
typedef enum ssm_option_mode_e ssm_option_mode_t;

enum  ssm_device_role_e
{
    SSM_DEVICE_MAST = 1,
    SSM_DEVICE_SLAV,
    SSM_DEVICE_STAND
};
typedef enum ssm_device_role_e ssm_device_role_t;

enum  ssm_gephy_role_e
{
    GEPHY_ROLE_MASTER = 1,
    GEPHY_ROLE_SLAVE,
    GEPHY_ROLE_STANDY
};
typedef enum ssm_gephy_role_e ssm_gephy_role_t;


enum  ssm_quality_e
{
    SSM_QL_STU = 0,
    SSM_QL_PRS = 1,
    SSM_QL_PRC = 2,
    SSM_QL_INV3 = 3,    /*Invalid QL for Flag of N/A*/
    SSM_QL_TNC = 4,     /*or QL-SSU-A */
    SSM_QL_ST2 = 7,
    SSM_QL_SSUB = 8,
    SSM_QL_EEC2 = 10,    /*or QL-ST3 */
    SSM_QL_EEC1 = 11,    /*or QL-SEC */
    SSM_QL_SMC = 12,
    SSM_QL_ST3E = 13,
    SSM_QL_PROV = 14,
    SSM_QL_DNU = 15        /*or QL-DUS */ 
};
typedef enum ssm_quality_e ssm_quality_t;

enum  ssm_if_ssm_pdu_e
{
    SSM_PDU_NONE = 0,
    SSM_PDU_RX,
    SSM_PDU_TX,
    SSM_PDU_ALL
};
typedef enum ssm_if_ssm_pdu_e ssm_if_ssm_pdu_t;

enum  ssm_device_clock_status_e
{
    SSM_CLOCK_LOCKMAST = 1,
    SSM_CLOCK_FREERUN,
    SSM_CLOCK_HOLDOVER,
    SSM_CLOCK_LOCK,
    SSM_CLOCK_UNLOCK
};
typedef enum ssm_device_role_e ssm_device_clock_status_t;

enum  clock_recovery_select_e
{
    CLOCK_RECOVERY_SELECT_SYNCE,
    CLOCK_RECOVERY_SELECT_BITS1,
    CLOCK_RECOVERY_SELECT_BITS2
};
typedef enum clock_recovery_select_e clock_recovery_select_t;

#define SYNCE_STATE_LOCKMAST              1
#define SYNCE_STATE_FREERUN               2
#define SYNCE_STATE_HOLDOVER              3
#define SYNCE_STATE_LOCK                  4
#define SYNCE_STATE_UNLOCK                5

#define SYNCE_HOLDOFF_TIME_MAX            1800  /*millisecond*/
#define SYNCE_HOLDOFF_TIME_MIN            300  /*millisecond*/
#define SYNCE_HOLDOFF_DEFAULT_TIME        500  /*millisecond*/

#define SYNCE_WTR_TIME_MAX                720  /*second*/
#define SYNCE_WTR_TIME_MIN                0   /*second*/
#define SYNCE_WTR_DEFAULT_TIME            60   /*second*/
/*end SSM*/

typedef enum
{
    GLB_TPID_TYPE_INVALID,
    GLB_TPID_TYPE_1,
    GLB_TPID_TYPE_2
} glb_tpid_type_t;

enum glb_direction_e
{
    GLB_INGRESS,
    GLB_EGRESS,
    GLB_BOTH_DIRECTION,
};
typedef enum glb_direction_e glb_direction_t;

typedef enum glb_err_e
{
    GLB_E_NONE = 0,
        
    GLB_E_GENERAL = GLB_ERROR_BASE,/*-6000*/
    GLB_E_NO_MEMORY,
    GLB_E_RESOURCE_FULL,    
    GLB_E_INVALID_PARAM,
    GLB_E_INVALID_PTR,
    
    GLB_E_SOCK_CREATE_ERR,/*-5995*/
    GLB_E_SOCK_BIND_ERR,
    GLB_E_SOCK_ACCEPT_ERR,
    GLB_E_SOCK_READ_ERR,
    GLB_E_SOCK_WRITE_ERR,
    
    GLB_E_MSG_LEN_TOO_SHORT,/*-5990*/
    GLB_E_MSG_LEN_TOO_LONG,
    GLB_E_MSG_TYPE_EER,    
    GLB_E_NOT_INIT,    
    GLB_E_ENTRY_NOT_EXIST,    
    
    GLB_E_ENTRY_EXISTED,/*-5985*/
    GLB_E_ENTRY_CONFLICT, 
    GLB_E_ENTRY_EXCEED_MAX, 
    GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY,
    GLB_ERR_SET_SECURITY_MAX,
    GLB_E_PORT_SECURITY_RESOURCE_FULL, /*Added by yanxa for support vlan security, bug 14013, 2011-02-12*/
    GLB_E_VLAN_SECURITY_RESOURCE_FULL, /*Added by yanxa for support vlan security, bug 14013, 2011-02-12*/
    GLB_E_VPLS_SECURITY_RESOURCE_FULL, /*Added by chengw for support vpls security 2011-04-27 */

    GLB_E_AGG_MEMBER, 
    GLB_E_NOT_LEARNING_ENABLE, 
    GLB_E_PORT_NOT_BELONG_TO_VID, /*Added by yejl for support port belongs to vid, bug 39502, 2016-07-13*/
    GLB_E_STP_DISABLE_LEARN, 
    GLB_E_IS_MIRROR_DEST_PORT, /*Added by yejl for check is mirror dest port, bug 49056, 2018-01-25*/
    GLB_E_IS_G8032_CONTROL_VLAN, /*Added by yangl for check is g8032 ring control vlan,2019-06-12*/
    GLB_E_MAX
} glb_err_t;

typedef enum
{
    SYS_LOAD_NORMAL = 0,
    SYS_LOAD_WARNING,
    SYS_LOAD_ALERT,
    SYS_LOAD_CRITICAL,
    SYS_LOAD_MAX
} sys_load_state_t;

#define SYS_LOAD_UPDATE_INTERVAL            5   /* seconds */

#define SYS_LOAD_WARNING_CLI_WAIT_MS        10
#define SYS_LOAD_ALERT_CLI_WAIT_MS          30

#define SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE      2000
#define SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING     10000

#define SYS_LOAD_STATE_FILE                 "/tmp/sys_load"

enum {
  ARP_TYPE_NULL = 0,
  ARP_TYPE_REQUEST,
  ARP_TYPE_RESPONSE,
  ARP_TYPE_RARP_REQUEST,
  ARP_TYPE_RARP_RESPONSE,
};

#define GLB_TAP_GROUP_MAX       512
#define GLB_TAP_TRUNCATION_MAX  4

enum {
  UNTAG_MODE_NULL = 0,
  UNTAG_MODE_AUTO,
  UNTAG_MODE_SVLAN,
  UNTAG_MODE_CVLAN,
};

#define BHM_KEEPALIVE_INTERVAL              3       /* seconds */
#define BHM_DEATH_THRESHOLD                 100     /* times */
#define BHM_SHUTDOWN_CODE                   0x5A5A

typedef enum
{
    GLB_BHM_RELOAD,
    GLB_BHM_SHUTDOWN,
    GLB_BHM_WARNING,
    GLB_BHM_ACTION_MAX
} glb_bhm_action_t;

typedef enum
{
    OSPF_NSSA_TRANSLATE_ROLE_DEFAULT = 0,
    OSPF_NSSA_TRANSLATE_ROLE_CANDIDATE,
    OSPF_NSSA_TRANSLATE_ROLE_NEVER,
    OSPF_NSSA_TRANSLATE_ROLE_ALWAYS,
    OSPF_NSSA_TRANSLATE_ROLE_MAX
} ospf_nssa_translate_role_t;

typedef enum
{
    OSPF_VLINK_AUTH_TYPE_DEFAULT = 0,
    OSPF_VLINK_AUTH_TYPE_NULL,
    OSPF_VLINK_AUTH_TYPE_MD5,
    OSPF_VLINK_AUTH_TYPE_MAX
} ospf_vlink_auth_type_t;

#define CMD_BUF_16              16
#define CMD_BUF_32              32
#define CMD_BUF_64              64
#define CMD_BUF_128             128
#define CMD_BUF_256             256
#define CMD_BUF_512             512
#define CMD_BUF_1024            1024
#define CMD_BUF_2048            2048
#define CMD_BUF_4096            4096

#define CMD_TRANSFORM_CHAR      '\\'
#define CMD_QUOTATION_CHAR      '"'
#define CMD_ZERO_CHAR           '\0'
#define CMD_DIVISION_CHAR       '/'
#define CMD_DIVISION_STR       "/"
#define CMD_DIVISION_CHAR_DS    '|'
#define CMD_LINE_CHAR           '\n'
#define CMD_COMMA_CHAR          ','
#define CMD_COMMA_CHAR_STR      ","
#define CMD_EQUAL_CHAR          '='
#define CMD_KEY_DIV_CHAR        '#'
#define CMD_EQUAL_STR           "="
#define CMD_KEY_DIV_STR         "#"
#define CMD_DS_LEFT_BRA_CHAR    '{'
#define CMD_DS_RIGHT_BRA_CHAR   '}'
#define CMD_MODE_INDENT         "  "

#ifdef TAPPRODUCT
#define CMD_NO_STR  "no"
#else
#define CMD_NO_STR  "no"
#endif

#define _DESC(desc) 1

/*add by zhw for mc, scan mrt data stats*/
#define GLB_MRT_POLLING_STATS_INTERVAL   10  


#endif /* !__GLB_CONST_H__*/

