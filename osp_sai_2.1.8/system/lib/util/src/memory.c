
#include "glb_macro.h"
#include "sal.h"
#include "ctclib_slab.h"
#include "ctclib_memory.h"

#define CTC_MEMMGR_LOCK \
do { \
    if (g_p_memory_mutex)\
    {\
        sal_mutex_lock(g_p_memory_mutex);\
    }\
}while(0)

#define CTC_MEMMGR_UNLOCK \
do { \
    if (g_p_memory_mutex)\
    {\
        sal_mutex_unlock(g_p_memory_mutex);\
    }\
}while(0)

typedef struct
{
#if 0
    /* TODO : unused variables */
    void       *list;       /* points to type memory list */
    uint32      req_size;   /* total user requested size */
#endif
    uint32      size;       /* total size of memory allocated or free */
    uint32      count;      /* number of blocks allocated or free */
} memory_stats_t;

typedef struct
{
    uint32      type;
    const char *desc;
} memory_desc_t;

static sal_mutex_t *g_p_memory_mutex = NULL;
static memory_stats_t  g_mem_stats[MEM_MAX_TYPE];
static memory_desc_t g_mem_desc[] =
{
    /* LIB MEM define */
    {MEM_TEMP,                      "MEM_TEMP"},
    {MEM_LIB_BITMAP,                "MEM_LIB_BITMAP"},
    {MEM_LIB_HASH,                  "MEM_LIB_HASH"},
    {MEM_LIB_HASH_BACKET_LIST,      "MEM_LIB_HASH_BACKET_LIST"},
    {MEM_LIB_HASH_BACKET,           "MEM_LIB_HASH_BACKET"},
    {MEM_LIB_AVL_TREE,              "MEM_LIB_AVL_TREE"},
    {MEM_LIB_AVL_NODE,              "MEM_LIB_AVL_NODE"},
    {MEM_LIB_VECTOR_DB,             "MEM_LIB_VECTOR_DB"},
    {MEM_LIB_VECTOR_DATA,           "MEM_LIB_VECTOR_DATA"},
    {MEM_LIB_SOCK_MASTER,           "MEM_LIB_SOCK_MASTER"},
    {MEM_LIB_SOCK,                  "MEM_LIB_SOCK"},
    {MEM_LIB_SOCK_SESSION,          "MEM_LIB_SOCK_SESSION"},
    {MEM_LIB_SOCK_DATA,             "MEM_LIB_SOCK_DATA"},
    {MEM_LIB_SOCK_QUEUE,            "MEM_LIB_SOCK_QUEUE"},
    {MEM_LIB_OPF,                   "MEM_LIB_OPF"},
    {MEM_LIB_OPB,                   "MEM_LIB_OPB"},
    {MEM_LIB_SLIST,                 "MEM_LIB_SLIST"},
    {MEM_LIB_SLISTNODE,             "MEM_LIB_SLISTNODE"},
    {MEM_LIB_PROTOBUF,              "MEM_LIB_PROTOBUF"},
    {MEM_LIB_JSON,                  "MEM_LIB_JSON"},
    {MEM_LIB_ACL_IOTIME,            "MEM_LIB_ACL_IOTIME"},
    {MEM_IGMP_RECORD,               "MEM_IGMP_RECORD"},

    /* CDB TBL/DS MEM define */
    {MEM_TBL_MASTER,                "MEM_TBL_MASTER"},
    {MEM_TBL_INTERFACE,             "MEM_TBL_INTERFACE"},
    {MEM_TBL_FEA_PORT_IF,           "MEM_TBL_FEA_PORT_IF"},
    {MEM_TBL_VLAN,                  "MEM_TBL_VLAN"},
    {MEM_TBL_CPU_TRAFFIC,           "MEM_TBL_CPU_TRAFFIC"},
    {MEM_TBL_CPU_TRAFFIC_GROUP,           "MEM_TBL_CPU_TRAFFIC_GROUP"},
    {MEM_TBL_CPU_UTILIZATION,       "MEM_TBL_CPU_UTILIZATION"},
    {MEM_TBL_CPU_LIMIT,             "MEM_TBL_CPU_LIMIT"},
    {MEM_TBL_PVLAN,                 "MEM_TBL_PVLAN"},
    {MEM_TBL_FDB,                   "MEM_TBL_FDB"},
    {MEM_TBL_MCFDB,                 "MEM_TBL_MCFDB"},
    {MEM_TBL_MACFILTER,             "MEM_TBL_MACFILTER"},
    {MEM_TBL_PSFDB,                 "MEM_TBL_PSFDB"},
    {MEM_TBL_IPSG_S_IP,             "MEM_TBL_IPSG_S_IP"},
    {MEM_TBL_IPSG_S_MAC,            "MEM_TBL_IPSG_S_MAC"},
    {MEM_TBL_IPSG_FIB,              "MEM_TBL_IPSG_FIB"},
    {MEM_TBL_MRT,                   "MEM_TBL_MRT"},
    {MEM_TBL_IPMC_GLOBAL,           "MEM_TBL_IPMC_GLOBAL"},
    {MEM_TBL_IPMC_INTF,             "MEM_TBL_IPMC_INTF"},
    {MEM_TBL_PIM_STATIC_RP,         "MEM_TBL_PIM_STATIC_RP"},
    {MEM_TBL_BRG_GLOBAL,            "MEM_TBL_BRG_GLOBAL"},
    {MEM_TBL_LAG_GLOBAL,            "MEM_TBL_LAG_GLOBAL"},
    {MEM_TBL_ECMP_GLOBAL,           "MEM_TBL_ECMP_GLOBAL"},
    {MEM_TBL_RR_PREFIX,             "MEM_TBL_RR_PREFIX"},
    {MEM_TBL_MSTP_PORT,             "MEM_TBL_MSTP_PORT"},
    {MEM_TBL_MSTI_PORT,             "MEM_TBL_MSTI_PORT"},
    {MEM_TBL_MSTP_INSTANCE,         "MEM_TBL_MSTP_INSTANCE"},
    {MEM_TBL_MSTP_GLOBAL,           "MEM_TBL_MSTP_GLOBAL"},
    {MEM_TBL_LLDP_GLOBAL,           "MEM_TBL_LLDP_GLOBAL"},
    {MEM_TBL_LLDP_IF,               "MEM_TBL_LLDP_IF"},
    {MEM_TBL_MLAG,                  "MEM_TBL_MLAG"},
    {MEM_TBL_MLAG_PEER,             "MEM_TBL_MLAG_PEER"},
    {MEM_TBL_MLAG_PORT,             "MEM_TBL_MLAG_PORT"},
    {MEM_TBL_ISOLATION,             "MEM_TBL_ISOLATION"},
    {MEM_TBL_ROUTE_GLOBAL,          "MEM_TBL_ROUTE_GLOBAL"},
	{MEM_TBL_BGP,                 	"MEM_TBL_BGP"},
    {MEM_TBL_BGP_NETWORK,           "MEM_TBL_BGP_NETWORK"},
    {MEM_TBL_BGP_NEIGHBOR,         	"MEM_TBL_BGP_NEIGHBOR"},
    {MEM_TBL_OSPF,                  "MEM_TBL_OSPF"},
    {MEM_TBL_OSPF6,                  "MEM_TBL_OSPF6"},
    {MEM_TBL_OSPF6_INTERFACE,       "MEM_TBL_OSPF6_INTERFACE"},
    {MEM_TBL_OSPF_NETWORK,          "MEM_TBL_OSPF_NETWORK"},
    {MEM_TBL_OSPF_AREA_AUTH,        "MEM_TBL_OSPF_AREA_AUTH"},
    {MEM_TBL_OSPF_AREA_RANGE,        "MEM_TBL_OSPF_AREA_RANGE"},
    {MEM_TBL_OSPF6_AREA_RANGE,        "MEM_TBL_OSPF6_AREA_RANGE"},
    {MEM_TBL_ROUTE_IF,              "MEM_TBL_ROUTE_IF"},
    {MEM_TBL_KERNEL_IF,             "MEM_TBL_KERNEL_IF"},
    {MEM_TBL_IPROUTE_NODE,          "MEM_TBL_ROUTE_NODE"},
    {MEM_TBL_IPROUTE_NODE_ADD_FAIL,          "MEM_TBL_ROUTE_NODE_ADD_FAIL"},
    {MEM_TBL_IPROUTE_NODE_ADD_FAIL_COUNT,          "MEM_TBL_ROUTE_NODE_ADD_FAIL_COUNT"},
    {MEM_TBL_STATIC_ROUTE_CFG,      "MEM_TBL_STATIC_ROUTE_CFG"},
    {MEM_TBL_STATIC_RT_CNT,         "MEM_TBL_STATIC_RT_CNT"},
    {MEM_TBL_ARP_FIB,               "MEM_TBL_ARP_FIB"},
    {MEM_TBL_ARP,                   "MEM_TBL_ARP"},
    {MEM_TBL_ARP_CFG,               "MEM_TBL_ARP_CFG"},
    {MEM_TBL_NEXTHOP,               "MEM_TBL_NH"},
    {MEM_TBL_NEXTHOP_GROUP,         "MEM_TBL_NEXTHOP_GROUP"},
    {MEM_TBL_NEXTHOP_GROUP_ROUTED,         "MEM_TBL_NEXTHOP_GROUP_ROUTED"},
    {MEM_TBL_FEA_NEXTHOP,           "MEM_TBL_FEA_NH"},
    {MEM_TBL_FEA_NH_GROUP,          "MEM_TBL_FEA_NH_GROUP"},
    {MEM_TBL_FEA_NHG_CNT,           "MEM_TBL_FEA_NHG_CNT"},
    {MEM_TBL_SYS_GLOBAL,            "MEM_TBL_SYS_GLOBAL"},
    {MEM_TBL_VERSION,               "MEM_TBL_VERSION"},
    {MEM_TBL_MANAGE_IF,             "MEM_TBL_MANAGE_IF"},
    {MEM_TBL_IPG_GLOBAL,            "MEM_TBL_IPG_GLOBAL"},
    {MEM_TBL_BOOTIMAGE,             "MEM_TBL_BOOTIMAGE"},
    {MEM_TBL_CHASSIS,               "MEM_TBL_CHASSIS"},
    {MEM_TBL_IFNAME_INFO,           "MEM_TBL_IFNAME_INFO"},
    {MEM_TBL_CARD,                  "MEM_TBL_CARD"},
    {MEM_TBL_PORT,                  "MEM_TBL_PORT"},
    {MEM_TBL_FIBER,                 "MEM_TBL_FIBER"},
    {MEM_TBL_SYS_SPEC,              "MEM_TBL_SYS_SPEC"},
    {MEM_TBL_FAN,                   "MEM_TBL_FAN"},
    {MEM_TBL_PSU,                   "MEM_TBL_PSU"},
    {MEM_TBL_LED,                   "MEM_TBL_LED"},
    {MEM_TBL_SENSOR,                "MEM_TBL_SENSOR"},
    {MEM_TBL_VCM,                "MEM_TBL_VCM"},
    {MEM_TBL_POE,                "MEM_TBL_POE"},
    {MEM_TBL_REBOOT_INFO,           "MEM_TBL_REBOOT_INFO"},
    {MEM_TBL_SYSTEM_SUMMARY,        "MEM_TBL_SYSTEM_SUMMARY"},
    {MEM_TBL_MEM_SUMMARY,           "MEM_TBL_MEM_SUMMARY"},
    {MEM_TBL_CPU_MEM_HISTORY,       "MEM_TBL_CPU_MEM_HISTORY"},
    {MEM_TBL_CHSM_DEBUG,            "MEM_TBL_CHSM_DEBUG"},
    {MEM_TBL_SWITCH_DEBUG,          "MEM_TBL_SWITCH_DEBUG"},
    {MEM_TBL_ROUTE_DEBUG,           "MEM_TBL_ROUTE_DEBUG"},
    {MEM_TBL_SSM_DEBUG,             "MEM_TBL_SSM_DEBUG"},
    {MEM_TBL_PTP_DEBUG,             "MEM_TBL_PTP_DEBUG"},
    {MEM_TBL_QUAGGA_DEBUG,          "MEM_TBL_QUAGGA_DEBUG"},
    {MEM_TBL_LSRV_DEBUG,            "MEM_TBL_LSRV_DEBUG"},
    {MEM_TBL_HSRV_DEBUG,            "MEM_TBL_HSRV_DEBUG"},
    {MEM_TBL_HAGT_DEBUG,            "MEM_TBL_HAGT_DEBUG"},
    {MEM_TBL_APP_DEBUG,             "MEM_TBL_APP_DEBUG"},
    {MEM_TBL_OPENFLOW_DEBUG,   "MEM_TBL_OPENFLOW_DEBUG"},
    {MEM_TBL_RIF,                   "MEM_TBL_RIF"},
    {MEM_TBL_FEA_LAG,               "MEM_TBL_FEA_LAG"},
    {MEM_TBL_FEA_GLOBAL,            "MEM_TBL_FEA_GLOBAL"},
    {MEM_TBL_FEA_ACL_TABLE,         "MEM_TBL_FEA_ACL_TABLE"},
    {MEM_TBL_FEA_ACL,               "MEM_TBL_FEA_ACL"},
    {MEM_TBL_FEA_FDB,               "MEM_TBL_FEA_FDB"},
    {MEM_TBL_FEA_BRG_IF,            "MEM_TBL_FEA_BRG_IF"},
    {MEM_TBL_ACL_WORM_FILTER,       "MEM_TBL_ACL_WORM_FILTER"},
    {MEM_TBL_ACL_CONFIG,            "MEM_TBL_ACL_CONFIG"},
    {MEM_TBL_ACE_CONFIG,            "MEM_TBL_ACE_CONFIG"},
    {MEM_TBL_ACL_ENTRY,             "MEM_TBL_ACL_ENTRY"},
    {MEM_TBL_ACL_ENTRY_ACTION,      "MEM_TBL_ACL_ACTION_ENTRY"},
    {MEM_TBL_ACL_NEXTHOP_GROUP,     "MEM_TBL_ACL_NEXTHOP_GROUP"},
    {MEM_TBL_ACL_NEXTHOP,           "MEM_TBL_ACL_NEXTHOP"},
    {MEM_TBL_PMAP,                  "MEM_TBL_PMAP"},
    {MEM_TBL_PMAP_IF,               "MEM_TBL_PMAP_IF"},
    {MEM_TBL_CMAP,                  "MEM_TBL_CMAP"},
    {MEM_TBL_ACL,                   "MEM_TBL_ACL"},
    {MEM_TBL_ACE,                   "MEM_TBL_ACE"},
    {MEM_TBL_TIME_RANGE,            "MEM_TBL_TIME_RANGE"},
    {MEM_TBL_SSH_CFG,               "MEM_TBL_SSH_CFG"},
    {MEM_TBL_SNMP_CFG,              "MEM_TBL_SNMP_CFG"},
    {MEM_TBL_SNMP_VIEW,             "MEM_TBL_SNMP_VIEW"},
    {MEM_TBL_SNMP_COMMUNITY,        "MEM_TBL_SNMP_COMMUNITY"},
    {MEM_TBL_SNMP_CONTEXT,          "MEM_TBL_SNMP_CONTEXT"},
    {MEM_TBL_SNMP_USM_USER,         "MEM_TBL_SNMP_USM_USER"},
    {MEM_TBL_SNMP_GROUP_USM,        "MEM_TBL_SNMP_GROUP_USM"},
    {MEM_TBL_SNMP_ACCESS_USM,       "MEM_TBL_SNMP_ACCESS_USM"},
    {MEM_TBL_SNMP_NOTIFY,           "MEM_TBL_SNMP_NOTIFY"},
    {MEM_TBL_SNMP_TARGET_ADDR,      "MEM_TBL_SNMP_TARGET_ADDR"},
    {MEM_TBL_SNMP_TARGET_PARAMS,    "MEM_TBL_SNMP_TARGET_PARAMS"},
    {MEM_TBL_SNMP_TRAP,             "MEM_TBL_SNMP_TRAP"},
    {MEM_TBL_SNMP_INFORM,           "MEM_TBL_SNMP_INFORM"},
    {MEM_TBL_SNMP_TRAP_MSG,         "MEM_TBL_SNMP_TRAP_MSG"},
    {MEM_TBL_RMON_EVENT,            "MEM_TBL_RMON_EVENT"},
    {MEM_TBL_RMON_ALARM,            "MEM_TBL_RMON_ALARM"},
    {MEM_TBL_RMON_ETHER_STATS,      "MEM_TBL_RMON_ETHER_STATS"},
    {MEM_TBL_RMON_HISTORY,          "MEM_TBL_RMON_HISTORY"},
    {MEM_TBL_RMON_STATS_HISTORY,    "MEM_TBL_RMON_STATS_HISTORY"},
    {MEM_TBL_SYSLOG_CFG,            "MEM_TBL_SYSLOG_CFG"},
    {MEM_TBL_NTP_SERVER,            "MEM_TBL_NTP_SERVER"},
    {MEM_TBL_NTP_ACE,               "MEM_TBL_NTP_ACE"},
    {MEM_TBL_NTP_KEY,               "MEM_TBL_NTP_KEY"},
    {MEM_TBL_NTP_CFG,               "MEM_TBL_NTP_CFG"},
    {MEM_TBL_NTP_IF,                "MEM_TBL_NTP_IF"},
    {MEM_TBL_NTP_SYNCSTATUS,        "MEM_TBL_NTP_IF"},
    {MEM_TBL_USER,                  "MEM_TBL_USER"},
    {MEM_TBL_LOGIN_RECORD,          "MEM_TBL_LOGIN_RECORD"},
    {MEM_TBL_LOGIN_SECURITY_GLB,        "MEM_TBL_LOGIN_SECURITY_GLB"},
    {MEM_TBL_VTY,                   "MEM_TBL_VTY"},
    {MEM_TBL_CONSOLE,               "MEM_TBL_CONSOLE"},
    {MEM_TBL_AUTHEN,                "MEM_TBL_AUTHEN"},
    {MEM_TBL_LOGIN,                 "MEM_TBL_LOGIN"},
    {MEM_TBL_RSA,                   "MEM_TBL_RSA"},
    {MEM_TBL_QOS_DOMAIN,            "MEM_TBL_QOS_DOMAIN"},
    {MEM_TBL_QOS_POLICER_RES,            "MEM_TBL_QOS_POLICER_RES"},
    {MEM_TBL_QOS_MPLS_DOMAIN,            "MEM_TBL_QOS_MPLS_DOMAIN"},
    {MEM_TBL_QOS_POLICER_ACTION_PROFILE,   "MEM_TBL_QOS_POLICER_ACTION_PROFILE"},
    {MEM_TBL_QOS_POLICER_PROFILE,   "MEM_TBL_QOS_POLICER_PROFILE"},
    {MEM_TBL_QOS_DROP_PROFILE,      "MEM_TBL_QOS_DROP_PROFILE"},
    {MEM_TBL_QOS_QUEUE_SHAPE_PROFILE, "MEM_TBL_QOS_QUEUE_SHAPE_PROFILE"},
    {MEM_TBL_QOS_PORT_SHAPE_PROFILE, "MEM_TBL_QOS_PORT_SHAPE_PROFILE"},
    {MEM_TBL_QOS_QUEUE_SMART_BUFFER, "MEM_TBL_QOS_QUEUE_SMART_BUFFER"},
    {MEM_TBL_QOS_GLOBAL,            "MEM_TBL_QOS_GLOBAL"},
    {MEM_TBL_OPENFLOW,              "MEM_TBL_OPENFLOW"},
    {MEM_TBL_OPENFLOW_FLOW,         "MEM_TBL_OPENFLOW_FLOW"},
    {MEM_TBL_OPENFLOW_METER,        "MEM_TBL_OPENFLOW_METER"},
    {MEM_TBL_OPENFLOW_GROUP,        "MEM_TBL_OPENFLOW_GROUP"},
    {MEM_TBL_MIRROR,                "MEM_TBL_MIRROR"},
    {MEM_TBL_MIRROR_MAC_ESCAPE,     "MEM_TBL_MIRROR_MAC_ESCAPE"},
    {MEM_TBL_TAP_GROUP_INGRESS,     "MEM_TBL_TAP_GROUP_INGRESS"},
    {MEM_TBL_TAP_GROUP_INGRESS_FLOW, "MEM_TBL_TAP_GROUP_INGRESS_FLOW"},
    {MEM_TBL_TAP_GROUP_EGRESS,      "MEM_TBL_TAP_GROUP_EGRESS"},
    {MEM_TBL_TAP_GRPUP,             "MEM_TBL_TAP_GRPUP"},
    {MEM_TBL_TAP_FIVE_TUPLE,       "MEM_TBL_TAP_FIVE_TUPLE"},
    {MEM_TBL_TAP_DPI_GLOBAL,       "MEM_TBL_TAP_DPI_GLOBAL"},
    {MEM_TBL_STATIC_DNS,            "MEM_TBL_STATIC_DNS"},
    {MEM_TBL_DYNAMIC_DNS_DOMAIN,    "MEM_TBL_DYNAMIC_DNS_DOMAIN"},
    {MEM_TBL_DYNAMIC_DNS_SERVER,    "MEM_TBL_DYNAMIC_DNS_SERVER"},
    {MEM_TBL_DHCRELAY,              "MEM_TBL_DHCRELAY"},
    {MEM_TBL_DHCSRVGRP,             "MEM_TBL_DHCSRVGRP"},
    {MEM_TBL_DHCP_DEBUG,            "MEM_TBL_DHCP_DEBUG"},
    {MEM_TBL_DHCLIENT,              "MEM_TBL_DHCLIENT"},
    {MEM_TBL_DHCSNOOPING,           "MEM_TBL_DHCSNOOPING"},
    {MEM_TBL_DHCBINDING,            "MEM_TBL_DHCBINDING"},
    {MEM_TBL_IPTABLES_PREVENT,      "MEM_TBL_IPTABLES_PREVENT"},
    {MEM_TBL_ERRDISABLE,            "MEM_TBL_ERRDISABLE"},
    {MEM_TBL_TAP_INTERFACE,         "MEM_TBL_TAP_INTERFACE"},
    {MEM_TBL_TAP_GROUP,             "MEM_TBL_TAP_GROUP"},
    {MEM_TBL_NS_PORT_FORWARDING,    "MEM_TBL_NS_PORT_FORWARDING"},
    {MEM_TBL_LOG_GLOBAL,            "MEM_TBL_LOG_GLOBAL"},
    {MEM_TBL_LOG,                   "MEM_TBL_LOG"},
    {MEM_TBL_SYS_LOAD,              "MEM_TBL_SYS_LOAD"},
    {MEM_TBL_CEM,                   "MEM_TBL_CEM"},
    {MEM_TBL_CLOCK,                 "MEM_TBL_CLOCK"},
    {MEM_TBL_TAG_SUMMER_CLOCK,      "MEM_TBL_TAG_SUMMER_CLOCK"},
    {MEM_TBL_PORT_STATS,            "MEM_TBL_PORT_STATS"},
    {MEM_TBL_VLAN_STATS,            "MEM_TBL_VLAN_STATS"},
    {MEM_TBL_PORT_STATS_RATE,       "MEM_TBL_PORT_STATS_RATE"},
    {MEM_TBL_ACLQOS_IF,         "MEM_TBL_ACLQOS_IF"},
    {MEM_TBL_L2_ACTION,             "MEM_TBL_L2_ACTION"},
    {MEM_TBL_FEA_QOS_DROP_PROFILE, "MEM_TBL_FEA_QOS_DROP_PROFILE"},
    {MEM_TBL_FEA_QOS_DOMAIN, "MEM_TBL_FEA_QOS_DOMAIN"},
    {MEM_TBL_FEA_G8032_RING, "MEM_TBL_FEA_G8032_RING"},
    {MEM_TBL_FEA_G8032_RING_MC_GROUP, "MEM_TBL_FEA_G8032_RING_MC_GROUP"},
    {MEM_TBL_FEA_QOS_QUEUE_SHAPE_PROFILE, "MEM_TBL_FEA_QOS_QUEUE_SHAPE_PROFILE"},
    {MEM_TBL_FEA_QOS_PORT_SHAPE_PROFILE, "MEM_TBL_FEA_QOS_PORT_SHAPE_PROFILE"},
    {MEM_TBL_FEA_PORT_POLICER_APPLY, "MEM_TBL_FEA_PORT_POLICER_APPLY"},
    {MEM_TBL_ACLQOS_IF_STATS, "MEM_TBL_TBL_ACLQOS_IF_STATS"},
    {MEM_TBL_ERRDISABLE_FLAP, "MEM_TBL_ERRDISABLE_FLAP"},
    {MEM_TBL_OPM_GLOBAL,            "MEM_TBL_OPM_GLOBAL"},
    {MEM_TBL_ERPS_RING,             "MEM_TBL_ERPS_RING"},
    {MEM_TBL_ERPS_DOMAIN,           "MEM_TBL_ERPS_DOMAIN"},
    {MEM_TBL_OPM_DEBUG,             "MEM_TBL_OPM_DEBUG"},
    {MEM_TBL_POLICY_MAP_CONFIG,     "MEM_TBL_POLICY_MAP_CONFIG"},
    {MEM_TBL_CLASS_MAP_CONFIG,      "MEM_TBL_CLASS_MAP_CONFIG"},
    {MEM_TBL_CLASS_IN_POLICY_CONFIG, "MEM_TBL_CLASS_IN_POLICY_CONFIG"},
    {MEM_TBL_ACL_IN_CLASS_CONFIG,   "MEM_TBL_ACL_IN_CLASS_CONFIG"},
    {MEM_TBL_CLASS_MAP_ACTION_CONFIG, "MEM_TBL_CLASS_MAP_ACTION_CONFIG"},
    {MEM_TBL_IGSP_GLOBAL,           "MEM_TBL_IGSP_GLOBAL"},
    {MEM_TBL_IGSP_INTF,             "MEM_TBL_IGSP_INTF"},
    {MEM_TBL_IGSP_GROUP,            "MEM_TBL_IGSP_GROUP"},
    {MEM_TBL_AUTH_CFG,              "MEM_TBL_AUTH_CFG"},
    {MEM_TBL_AUTH_SERVER,           "MEM_TBL_AUTH_SERVER"},
    {MEM_TBL_AUTH_SESSION,          "MEM_TBL_AUTH_SESSION"},
    {MEM_TBL_AUTHD_DEBUG,           "MEM_TBL_AUTHD_DEBUG"},
    {MEM_TBL_FEA_ACL_POLICY_ACTION,             "MEM_TBL_FEA_ACL_POLICY_ACTION"},
    {MEM_TBL_DOT1X_GLOBAL,          "MEM_TBL_DOT1X_GLOBAL"},
    {MEM_TBL_DOT1X_PORT,            "MEM_TBL_DOT1X_PORT"},
    {MEM_TBL_DOT1X_RADIUS,          "MEM_TBL_DOT1X_RADIUS"},
    {MEM_TBL_DOT1X_MAC,             "MEM_TBL_DOT1X_MAC"},
    {MEM_TBL_ENABLE,                "MEM_TBL_ENABLE"},
    {MEM_TBL_CHIP,                  "MEM_TBL_CHIP"},
    {MEM_TBL_CLEAR_ACL_POLICY,            "MEM_TBL_CLEAR_ACL_POLICY"},
    {MEM_TBL_AUTHOR,                "MEM_TBL_AUTHOR"},    
    {MEM_TBL_ACCOUNT,               "MEM_TBL_ACCOUNT"},
    {MEM_TBL_ACCOUNTCMD,            "MEM_TBL_ACCOUNTCMD"},
    {MEM_TBL_VLANCLASS_RULE,        "MEM_TBL_VLANCLASS_RULE"},
    {MEM_TBL_VLANCLASS_GROUP,       "MEM_TBL_VLANCLASS_GROUP"},
    {MEM_TBL_ACL_L4_PORT_RANGE,     "MEM_TBL_ACL_L4_PORT_RANGE"},
    {MEM_TBL_ACL_UDF_GROUP,         "MEM_TBL_ACL_UDF_GROUP"},
    {MEM_TBL_ACL_UDF_ENTRY,         "MEM_TBL_ACL_UDF_ENTRY"},
    {MEM_TBL_FEA_PCAP,              "MEM_TBL_FEA_PCAP"},
    {MEM_TBL_CONTROLLER,              "MEM_TBL_CONTROLLER"},
    {MEM_TBL_CPU_PACKETS,              "MEM_TBL_CPU_PACKETS"},
    {MEM_TBL_NS_ROUTE,              "MEM_TBL_NS_ROUTE"},
    {MEM_TBL_NS_ROUTE_IP,           "MEM_TBL_NS_ROUTE_IP"},
    {MEM_TBL_OPENFLOW_INTERFACE,      "MEM_TBL_OPENFLOW_INTERFACE"},
    {MEM_TBL_FEA_OPENFLOW_HOSTIF_ID,      "MEM_TBL_FEA_OPENFLOW_HOSTIF_ID"},
    {MEM_TBL_OPENFLOW_MANAGER,      "MEM_TBL_OPENFLOW_MANAGER"},
    {MEM_TBL_PTP_GLOBAL,            "MEM_TBL_PTP_GLOBAL"},
    {MEM_TBL_PTP_PORT,              "MEM_TBL_PTP_PORT"},
    {MEM_TBL_PTP_FOREIGN,           "MEM_TBL_PTP_FOREIGN"},
    {MEM_TBL_PTP_RESIDENCE_TIME_CACHE,      "MEM_TBL_PTP_RESIDENCE_TIME_CACHE"},
    {MEM_TBL_FEA_PTP_GLOBAL,      "MEM_TBL_FEA_PTP_GLOBAL"},
    {MEM_TBL_FEA_PTP_TS,      "MEM_TBL_FEA_PTP_TS"},
    {MEM_TBL_FEA_TIME,              "MEM_TBL_FEA_TIME"},
    {MEM_TBL_BHM_GLOBAL,            "MEM_TBL_BHM_GLOBAL"},
    {MEM_TBL_BHM_MODULE,            "MEM_TBL_BHM_MODULE"},
    {MEM_TBL_OPENFLOW_TUNNEL_INTERFACE,      "MEM_TBL_OPENFLOW_TUNNEL_INTERFACE"},
    {MEM_TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT,      "MEM_TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT"},
    {MEM_TBL_INBAND_SNAT,           "MEM_TBL_INBAND_SNAT"},
    {MEM_TBL_ARPINSP,               "MEM_TBL_ARPINSP"},
    {MEM_TBL_ARPACL_CONFIG,         "MEM_TBL_ARPACL_CONFIG"},
    {MEM_TBL_ARPACE_CONFIG,         "MEM_TBL_ARPACE_CONFIG"},
    {MEM_TBL_COPP_CFG,              "MEM_TBL_COPP_CFG"},
    {MEM_TBL_FEA_COPP_TRAP_GROUP,   "MEM_TBL_FEA_COPP_TRAP_GROUP"},
    {MEM_TBL_SFLOW_GLOBAL,          "MEM_TBL_SFLOW_GLOBAL"},
    {MEM_TBL_SFLOW_COLLECTOR,       "MEM_TBL_SFLOW_COLLECTOR"},
    {MEM_TBL_SFLOW_COUNTER_PORT,    "MEM_TBL_SFLOW_COUNTER_PORT"},
    {MEM_TBL_VRRP_GLOBAL,           "MEM_TBL_VRRP_GLOBAL"},
    {MEM_TBL_VRRP_VMAC,             "MEM_TBL_VRRP_VMAC"},    
    {MEM_TBL_VRRP_VIP,              "MEM_TBL_VRRP_VIP"},
    {MEM_TBL_VRRP_SESSION,          "MEM_TBL_VRRP_SESSION"},
    {MEM_TBL_SSM_GLOBAL,            "MEM_TBL_SSM_GLOBAL"},
    {MEM_TBL_SSM_PORT,              "MEM_TBL_SSM_PORT"},
    {MEM_TBL_IPSLA_GLOBAL,          "MEM_TBL_IPSLA_GLOBAL"},
    {MEM_TBL_IPSLA_ENTRY,           "MEM_TBL_IPSLA_ENTRY"},
    {MEM_TBL_IPSLA_PACKET_STATS,    "MEM_TBL_IPSLA_PACKET_STATS"},
    {MEM_TBL_IPSLA_TEST_STATS,      "MEM_TBL_IPSLA_TEST_STATS"},
    {MEM_TBL_TRACK_GLOBAL,          "MEM_TBL_TRACK_GLOBAL"},
    {MEM_TBL_TRACK_OBJECT,          "MEM_TBL_TRACK_OBJECT"},
    {MEM_TBL_CONTROLLER_ATTR,   "MEM_TBL_CONTROLLER_ATTR"},
	{MEM_TBL_RIP,                 	"MEM_TBL_RIP"},
	{MEM_TBL_RIPNG,                 "MEM_TBL_RIPNG"},
    {MEM_TBL_RIP_NETWORK,           "MEM_TBL_RIP_NETWORK"},
    {MEM_TBL_RIPNG_NETWORK,         "MEM_TBL_RIPNG_NETWORK"},
    {MEM_TBL_RIP_NEIGHBOR,         	"MEM_TBL_RIP_NEIGHBOR"},
    {MEM_TBL_RIP_PASSIVE_IF,       	"MEM_TBL_RIP_PASSIVE_IF"},
    {MEM_TBL_RIPNG_PASSIVE_IF,      "MEM_TBL_RIPNG_PASSIVE_IF"},
    {MEM_TBL_OAM_SESSION,   "MEM_TBL_OAM_SESSION"},
    {MEM_TBL_OAM_SESSION_UPLOAD,   "MEM_TBL_OAM_SESSION_UPLOAD"},
    {MEM_TBL_LSP_APS_GROUP,   "MEM_TBL_LSP_APS_GROUP"},
    {MEM_TBL_PW_APS_GROUP,   "MEM_TBL_PW_APS_GROUP"},
    {MEM_TBL_LSP_PE,   "MEM_TBL_LSP_PE"},
    {MEM_TBL_LSP_P,   "MEM_TBL_LSP_P"},
    {MEM_TBL_PW_TPE,   "MEM_TBL_PW_TPE"},
    {MEM_TBL_SECTION_OAM,   "MEM_TBL_SECTION_OAM"},
    {MEM_TBL_OAM_FLOW_INFO,   "MEM_TBL_OAM_FLOW_INFO"},
    {MEM_TBL_TPOAM_GLOBAL,   "MEM_TBL_TPOAM_GLOBAL"},
    {MEM_TBL_TPOAM_EXEC_GLOBAL,   "MEM_TBL_TPOAM_EXEC_GLOBAL"},
    {MEM_TBL_APS_GROUP_INFO,   "MEM_TBL_APS_GROUP_INFO"},
    {MEM_TBL_OAM_INLABEL_INFO,   "MEM_TBL_OAM_INLABEL_INFO"},
    {MEM_TBL_OAM_MPLS_APS_CHECK,   "MEM_TBL_OAM_MPLS_APS_CHECK"},
    {MEM_TBL_VLAN_MAPPING_TABLE,    "MEM_TBL_VLAN_MAPPING_TABLE"},
    {MEM_TBL_VLAN_MAPPING_ENTRY,    "MEM_TBL_VLAN_MAPPING_ENTRY"},
    {MEM_TBL_EGRESS_MAPPING_ENTRY,  "MEM_TBL_EGRESS_MAPPING_ENTRY"},
    {MEM_TBL_VLAN_GROUP,            "MEM_TBL_VLAN_GROUP"},
    {MEM_TBL_EVC_ENTRY,             "MEM_TBL_EVC_ENTRY"},
    {MEM_TBL_MAPPED_VLAN_INFO,      "MEM_TBL_MAPPED_VLAN_INFO"},
    {MEM_TBL_L2_PROTOCOL,           "MEM_TBL_L2_PROTOCOL"},
    {MEM_TBL_ND_FIB,                "MEM_TBL_ND_FIB"},
    {MEM_TBL_ND_STATS,              "MEM_TBL_ND_STATS"},
    {MEM_TBL_CPU_MIRROR_GLOBAL,     "MEM_TBL_CPU_MIRROR_GLOBAL"},
    {MEM_TBL_TAP_SERVER,            "MEM_TBL_TAP_SERVER"},
    {MEM_TBL_DHCPV6_DEBUG,          "MEM_TBL_DHCPV6_DEBUG"},
    {MEM_TBL_L2EDIT,                "MEM_TBL_L2EDIT"},
    {MEM_TBL_HASH_FIELD_PROFILE,    "MEM_TBL_HASH_FIELD_PROFILE"},
    {MEM_TBL_HASH_VALUE_PROFILE,    "MEM_TBL_HASH_VALUE_PROFILE"},
    {MEM_TBL_HASH_VALUE_GLOBAL,     "MEM_TBL_HASH_VALUE_GLOBAL"},
    {MEM_TBL_NAT_GLOBAL,            "MEM_TBL_NAT_GLOBAL"},
    {MEM_TBL_NAT_POOL,              "MEM_TBL_NAT_POOL"},
    {MEM_TBL_NAT_RULE,              "MEM_TBL_NAT_RULE"},
    {MEM_TBL_NAT_SESSION,           "MEM_TBL_NAT_SESSION"},
    {MEM_TBL_NAT_SESSION_LIMIT,     "MEM_TBL_NAT_SESSION_LIMIT"},
    {MEM_TBL_NAT_PROXY_ARP,         "MEM_TBL_NAT_PROXY_ARP"},
    {MEM_TBL_NAT_RTIF_INSIDE,       "MEM_TBL_NAT_RTIF_INSIDE"},
    {MEM_TBL_NAT_RTIF_OUTSIDE,      "MEM_TBL_NAT_RTIF_OUTSIDE"},
    {MEM_TBL_G8032_RING,            "MEM_TBL_G8032_RING"},
    {MEM_TBL_CFM_GLOBAL,         "MEM_TBL_CFM_GLOBAL"},         
    {MEM_TBL_CFM_MD,                "MEM_TBL_CFM_MD"},
    {MEM_TBL_CFM_MA,                "MEM_TBL_CFM_MA"},
    {MEM_TBL_CFM_MAID,             "MEM_TBL_CFM_MAID"},
    {MEM_TBL_CFM_LMEP,             "MEM_TBL_CFM_LMEP"},
    {MEM_TBL_CFM_RMEP,             "MEM_TBL_CFM_RMEP"},
	/*add by zhw for multicast*/
    {MEM_TBL_PIM_SM,                "MEM_TBL_PIM_SM"},
    {MEM_TBL_PIM_SM_BSR,            "MEM_TBL_PIM_SM_BSR"},
    {MEM_TBL_PIM_SM_RP,             "MEM_TBL_PIM_SM_RP"},
    {MEM_TBL_PIM_SM_RP_GP,          "MEM_TBL_PIM_SM_RP_GP"},
    {MEM_TBL_PIM_DM,                "MEM_TBL_PIM_DM"},
    {MEM_TBL_IGMP_INTF,             "MEM_TBL_IGMP_INTF"},	
    {MEM_TBL_OSPF_AREA_STUB,        "MEM_TBL_OSPF_AREA_STUB"},
    {MEM_TBL_OSPF6_AREA_STUB,        "MEM_TBL_OSPF6_AREA_STUB"},
    {MEM_TBL_OSPF_AREA_NSSA,        "MEM_TBL_OSPF_AREA_NSSA"},
    {MEM_TBL_OSPF_AREA_VLINK,       "MEM_TBL_OSPF_AREA_VLINK"},

    {MEM_DS_MAC_ACE,                "MEM_DS_MAC_ACE"},
    {MEM_DS_IP_ACE,                 "MEM_DS_IP_ACE"},
    {MEM_DS_BRGIF,                  "MEM_DS_BRGIF"},
    {MEM_DS_LAG,                    "MEM_DS_LAG"},
    {MEM_DS_STORM_CONTROL,          "MEM_DS_STORM_CONTROL"},
    {MEM_DS_LACP,                   "MEM_DS_LACP"},
    {MEM_DS_PMAP_CLASS,             "MEM_DS_PMAP_CLASS"},
    {MEM_DS_PMAP_IF,                "MEM_DS_PMAP_IF"},
    {MEM_DS_CONNECTED,              "MEM_DS_CONNECTED"},
    {MEM_DS_CONNECTED_V6,              "MEM_DS_CONNECTED_V6"},
    {MEM_DS_STATIC_ROUTE_DUP_IPV4_NH,              "MEM_DS_STATIC_ROUTE_DUP_IPV4_NH"},
    {MEM_DS_OSPF_AUTH,              "MEM_DS_OSPF_AUTH"},
    {MEM_DS_RIB,                    "MEM_DS_RIB"},
    {MEM_DS_NEXTHOP,                "MEM_DS_NEXTHOP"},
    {MEM_DS_IF_STATS,               "MEM_DS_IF_STATS"},
    {MEM_DS_ACLQOS_IF,              "MEM_DS_ACLQOS_IF"},
    {MEM_DS_OPENFLOW_IF,            "MEM_DS_OPENFLOW_IF"},
    {MEM_DS_DHCLIENT_IF,            "MEM_DS_DHCLIENT_IF"},
    {MEM_DS_TMPR_STATUS,            "MEM_DS_TMPR_STATUS"},
    {MEM_DS_PSU_STATUS,             "MEM_DS_PSU_STATUS"},
    {MEM_DS_FAN_STATUS,             "MEM_DS_FAN_STATUS"},
    {MEM_DS_PVLAN_COMMUNITY,        "MEM_DS_PVLAN_COMMUNITY"},
    {MEM_DS_CIRCUIT_ID,             "MEM_DS_CIRCUIT_ID"},
    {MEM_DS_REBOOT_INFO,            "MEM_DS_REBOOT_INFO"},
    {MEM_DS_FLUSH_FDB,              "MEM_DS_FLUSH_FDB"},
    {MEM_DS_LLDP_NEIGHBOUR,         "MEM_DS_LLDP_NEIGHBOUR"},
    {MEM_DS_MROUTER_PORT,           "MEM_DS_MROUTER_PORT"},
    {MEM_DS_QUERY,                  "MEM_DS_QUERY"},
    {MEM_DS_L2PROTOCOL,             "MEM_DS_L2PROTOCOL"},
    {MEM_DS_ND_PREFIX,              "MEM_DS_ND_PREFIX"},
    {MEM_DS_JOIN_GROUP,             "MEM_DS_JOIN_GROUP"},
    {MEM_DS_CFM_ERROR,              "MEM_DS_CFM_ERROR"},

    {MEM_STBL_SAI_GLB,              "MEM_STBL_SAI_GLB"},
    {MEM_STBL_PORT_GLB,             "MEM_STBL_PORT_GLB"},
    {MEM_STBL_PORT,                 "MEM_STBL_PORT"},
    {MEM_STBL_HOSTIF,               "MEM_STBL_HOSTIF"},
    {MEM_STBL_LAG_MEMPORT,          "MEM_STBL_LAG_MEMPORT"},
    {MEM_STBL_VR,                   "MEM_STBL_VR"},
    {MEM_STBL_RIF,                  "MEM_STBL_RIF"},
    {MEM_STBL_NEXTHOP,              "MEM_STBL_NEXTHOP"},
    {MEM_STBL_NEXTHOP_GROUP,        "MEM_STBL_NEXTHOP_GROUP"},
    {MEM_STBL_NEIGHBOR,             "MEM_STBL_NEIGHBOR"},
    {MEM_STBL_VLAN,                 "MEM_STBL_VLAN"},
    {MEM_STBL_VLAN_CLASS,           "MEM_STBL_VLAN_CLASS"},
    {MEM_STBL_STP,                  "MEM_STBL_STP"},
    {MEM_STBL_L2MC,                 "MEM_STBL_L2MC"},
    {MEM_STBL_IPMC,                 "MEM_STBL_IPMC"},
    {MEM_STBL_ROUTE,                "MEM_STBL_ROUTE"},
    {MEM_STBL_VRRP_VIP,             "MEM_STBL_VRRP_VIP"},
    {MEM_STBL_COPP_TRAP,            "MEM_STBL_COPP_TRAP"},
    {MEM_STBL_COPP_GROUP,           "MEM_STBL_COPP_GROUP"},
    {MEM_STBL_COPP_REASON,          "MEM_STBL_COPP_REASON"},
    {MEM_STBL_COPP_ENTRY,           "MEM_STBL_COPP_ENTRY"},
    {MEM_STBL_ACL_GLB,              "MEM_STBL_ACL_GLB"},
    {MEM_STBL_ACL_PORT,             "MEM_STBL_ACL_PORT"},
    {MEM_STBL_ACL_TBL,              "MEM_STBL_ACL_TBL"},
    {MEM_STBL_ACL_ENTRY,            "MEM_STBL_ACL_ENTRY"},
    {MEM_STBL_ACL_TUNNEL,           "MEM_STBL_ACL_TUNNEL"},
    {MEM_STBL_ACL_UDF_ENTRY,        "MEM_STBL_ACL_UDF_ENTRY"},
    {MEM_STBL_PORT_RX_STATS,        "MEM_STBL_PORT_RX_STATS"},
    {MEM_STBL_PORT_TX_STATS,        "MEM_STBL_PORT_TX_STATS"},
    {MEM_STBL_NAT_FORWARD,          "MEM_STBL_NAT_FORWARD"},
    {MEM_STBL_QOS_MPLS_DOMAIN,      "MEM_STBL_QOS_MPLS_DOMAIN"},
    {MEM_STBL_QOS_MPLS_FLOW,        "MEM_STBL_QOS_MPLS_FLOW"},
    {MEM_STBL_CFM_LMEP,                 "MEM_STBL_CFM_LMEP"},
    {MEM_STBL_CFM_RMEP,                 "MEM_STBL_CFM_RMEP"},
    
    /* PM MEM define */
    {MEM_PM_TPOAM_LM_DUAL,               "MEM_PM_TPOAM_LM_DUAL"},
    {MEM_PM_TPOAM_PDU,               "MEM_PM_TPOAM_PDU"},
    {MEM_PM_TPOAM_1DM_ENTRY,               "MEM_PM_TPOAM_1DM_ENTRY"},
    {MEM_PM_TPOAM_LB_REPLY_ENTRY,               "MEM_PM_TPOAM_LB_REPLY_ENTRY"},
    {MEM_PM_TPOAM_LB_PKT_BUF,               "MEM_PM_TPOAM_LB_PKT_BUF"},
    {MEM_PM_TPOAM_DM_ENTRY,               "MEM_PM_TPOAM_DM_ENTRY"},
    {MEM_PM_TPOAM_DM_DUAL_ENTRY,               "MEM_PM_TPOAM_DM_DUAL_ENTRY"},
    {MEM_PM_TPOAM_DM_SINGLE_ENTRY,               "MEM_PM_TPOAM_DM_SINGLE_ENTRY"},
    {MEM_PM_TPOAM_EVENT_ENTRY,               "MEM_PM_TPOAM_EVENT_ENTRY"},
    {MEM_PM_TPOAM_HSRV_MIP_ENTRY,               "MEM_PM_TPOAM_HSRV_MIP_ENTRY"},
    {MEM_LACP_PDU,                  "MEM_LACP_PDU"},
    {MEM_MSTP_PDU,                  "MEM_MSTP_PDU"},
    {MEM_LLDP_PDU,                  "MEM_LLDP_PDU"},
    {MEM_PTP_PDU,                  "MEM_PTP_PDU"},
    {MEM_ARPINSP_LOGBUF,            "MEM_ARPINSP_LOGBUF"},
    {MEM_PM_TEMP,                   "MEM_PM_TEMP"},
    {MEM_PM_LIB_MASTER,             "MEM_PM_LIB_MASTER"},
    {MEM_PM_CHSM_MASTER,            "MEM_PM_CHSM_MASTER"},
    {MEM_PM_CHSM_LINECARD,          "MEM_PM_CHSM_LINECARD"},
    {MEM_PM_CHSM_PORT_INFO,         "MEM_PM_CHSM_PORT_INFO"},
    {MEM_PM_CHSM_DIAG,              "MEM_PM_CHSM_DIAG"},
    {MEM_PM_CDS_MASTER,             "MEM_PM_CDS_MASTER"},
    {MEM_PM_SWITCH_MASTER,          "MEM_PM_SWITCH_MASTER"},
    {MEM_PM_ROUTE_MASTER,           "MEM_PM_ROUTE_MASTER"},
    {MEM_PM_RT_TABLE,               "MEM_PM_RT_TABLE"},
    {MEM_PM_OPENFLOW_MASTER,        "MEM_PM_OPENFLOW_MASTER"},
    {MEM_PM_OPENFLOW_COMMON,        "MEM_PM_OPENFLOW_COMMON"},
    {MEM_PM_OPENFLOW_PORT,          "MEM_PM_OPENFLOW_PORT"},
    {MEM_PM_OPENFLOW_FLOW,          "MEM_PM_OPENFLOW_FLOW"},
    {MEM_PM_OPENFLOW_GROUP,         "MEM_PM_OPENFLOW_GROUP"},
    {MEM_PM_OPENFLOW_METER,         "MEM_PM_OPENFLOW_METER"},
    {MEM_PM_OPENFLOW_TUNNEL,        "MEM_PM_OPENFLOW_TUNNEL"},
    {MEM_PM_MSTP_TMP,               "MEM_PM_MSTP_TMP"},
    {MEM_HSRV_HOSTIF_INFO,          "MEM_HSRV_HOSTIF_INFO"},
    {MEM_CTC_SAI_INFO,              "MEM_CTC_SAI_INFO"},
    {MEM_CDS_SUB_CDB,               "MEM_CDS_SUB_CDB"},
    {MEM_CDS_SUB_TBL,               "MEM_CDS_SUB_TBL"},
    {MEM_UDS_TIME_RANGE_TIMER,      "MEM_UDS_TIME_RANGE_TIMER"},
    {MEM_UDS_TIME_RANGE_ARG,        "MEM_UDS_TIME_RANGE_ARG"},
    {MEM_UDS_TIME_RANGE_ARG_ACE,    "MEM_UDS_TIME_RANGE_ARG_ACE"},

    /* FEA MEM define */
    {MEM_FEA_TEMP_TODO,             "MEM_FEA_TEMP_TODO"},
    {MEM_FEA_TEMP,                  "MEM_FEA_TEMP"},
    {MEM_FEA_IFMGR_MODULE,          "MEM_FEA_IFMGR_MODULE"},
    {MEM_FEA_HSRV_MODULE,           "MEM_FEA_HSRV_MODULE"},
    {MEM_FEA_HSRV_NEXTHOP,          "MEM_FEA_HSRV_NEXTHOP"},
    {MEM_FEA_HSRV_VLAN,             "MEM_FEA_HSRV_VLAN"},
    {MEM_FEA_HSRV_TUNNEL,           "MEM_FEA_HSRV_TUNNEL"},
    {MEM_FEA_HSRV_L3IF,             "MEM_FEA_HSRV_L3IF"},
    {MEM_FEA_HSRV_QOS,              "MEM_FEA_HSRV_QOS"},
    {MEM_FEA_HSRV_STATS,            "MEM_FEA_HSRV_STATS"},
    {MEM_FEA_HSRV_RATELIMIT,        "MEM_FEA_HSRV_RATELIMIT"},
    {MEM_FEA_HSRV_OPENFLOW,         "MEM_FEA_HSRV_OPENFLOW"},
    {MEM_FEA_HSRV_OPENFLOW_FLOW,    "MEM_FEA_HSRV_OPENFLOW_FLOW"},
    {MEM_FEA_HSRV_OPENFLOW_ACTION,  "MEM_FEA_HSRV_OPENFLOW_ACTION"},
    {MEM_FEA_HSRV_OPENFLOW_HSRV_ACTION,  "MEM_FEA_HSRV_OPENFLOW_HSRV_ACTION"},
    {MEM_FEA_HSRV_OPENFLOW_GROUP,   "MEM_FEA_HSRV_OPENFLOW_GROUP"},
    {MEM_FEA_HSRV_OPENFLOW_TUNNEL,  "MEM_FEA_HSRV_OPENFLOW_TUNNEL"},
    {MEM_FEA_HSRV_OPENFLOW_NH,      "MEM_FEA_HSRV_OPENFLOW_NH"},
    {MEM_FEA_HSRV_OPENFLOW_PORT,    "MEM_FEA_HSRV_OPENFLOW_PORT"},
    {MEM_FEA_HSRV_OPENFLOW_STATS,   "MEM_FEA_HSRV_OPENFLOW_STATS"},
    {MEM_FEA_HSRV_OPENFLOW_METER,   "MEA_FEA_HSRV_OPENFLOW_METER"},
    {MEM_FEA_HAGT_MODULE,           "MEM_FEA_HAGT_MODULE"},
    {MEM_FEA_HAGT_INTF,             "MEM_FEA_HAGT_INTF"},
    {MEM_FEA_HAGT_MSG,              "MEM_FEA_HAGT_MSG"},
    {MEM_FEA_HAGT_FDB,              "MEM_FEA_HAGT_FDB"},
    {MEM_FEA_HAGT_VLAN,             "MEM_FEA_HAGT_VLAN"},
    {MEM_FEA_HAGT_QOS,              "MEM_FEA_HAGT_QOS"},
    {MEM_FEA_HAGT_RATELIMIT,        "MEM_FEA_HAGT_RATELIMIT"},
    {MEM_FEA_HAGT_STATS,            "MEM_FEA_HAGT_STATS"},
    {MEM_FEA_HAGT_OPENFLOW,         "MEM_FEA_HAGT_OPENFLOW"},
    {MEM_FEA_HAGT_OPENFLOW_TPOAM,         "MEM_FEA_HAGT_OPENFLOW_TPOAM"},
    {MEM_FEA_CPU_TRAFFIC,           "MEM_FEA_CPU_TRAFFIC"},
    {MEM_FEA_PKT,                   "MEM_FEA_PKT"},
    {MEM_FEA_L2MC,                  "MEM_FEA_L2MC"},
    {MEM_FEA_IPUC,                  "MEM_FEA_IPUC"},
    {MEM_FEA_NEIGHBOR,              "MEM_FEA_NEIGHBOR"},
    {MEM_FEA_ECMP,                  "MEM_FEA_ECMP"},
    {MEM_FEA_LCM_MODULE,            "MEM_FEA_LCM_MODULE"},
    {MEM_FEA_PETH_PKT_BUFFER,       "MEM_FEA_PETH_PKT_BUFFER"},
    {MEM_FEA_MIRROR,                "MEM_FEA_MIRROR"},
    {MEM_FEA_SFLOW,                 "MEM_FEA_SFLOW"},
    {MEM_FEA_TAP,                   "MEM_FEA_TAP"},

    /* DRV/LCM MEM define */
    {MEM_DRV_SPI,                   "MEM_DRV_SPI"},
    {MEM_DRV_MDIO,                  "MEM_DRV_MDIO"},
    {MEM_DRV_CLOCK,                 "MEM_DRV_CLOCK"},
    {MEM_DRV_SYSENV,                 "MEM_DRV_SYSENV"},
    {MEM_DRV_PHY,                   "MEM_DRV_PHY"},
    {MEM_DRV_POE,                   "MEM_DRV_POE"},
    {MEM_DRV_FIBER,                 "MEM_DRV_FIBER"},
    {MEM_DRV_EPLD,                  "MEM_DRV_EPLD"},
    {MEM_DRV_FPGA,                  "MEM_DRV_FPGA"},
    {MEM_DRV_I2C,                   "MEM_DRV_I2C"},
    {MEM_DRV_EEPROM,                "MEM_DRV_EEPROM"},
    {MEM_DRV_SENSOR,                "MEM_DRV_SENSOR"},
    {MEM_DRV_POWER,                 "MEM_DRV_POWER"},
    {MEM_DRV_FAN,                   "MEM_DRV_FAN"},
    {MEM_DRV_SWITCH,                "MEM_DRV_SWITCH"},
    {MEM_DRV_LED,                   "MEM_DRV_LED"},
    {MEM_DRV_MUX,                   "MEM_DRV_MUX"},
    {MEM_DRV_GPIO,                  "MEM_DRV_GPIO"},
    {MEM_DRV_BMR,                  "MEM_DRV_BMR"},
    {MEM_DRV_VCM,                  "MEM_DRV_VCM"},
    {MEM_DRV_BOOTROM,               "MEM_DRV_BOOTROM"},
    {MEM_LCM_TEMP,                  "MEM_LCM_TEMP"},
    {MEM_LCM_PORT,                  "MEM_LCM_PORT"},
    {MEM_COLD_LOGGING_BUF,          "MEM_COLD_LOGGING_BUF"},
};

CTC_BUILD_ASSERT( sizeof(g_mem_desc)/sizeof(memory_desc_t) == MEM_MAX_TYPE );

struct malloc_size {
    uint32 size;
    ctclib_mem_cache_t *cache;
};

struct malloc_size malloc_sizes[] = {
#define MALLOC_SIZE(x) {.size = x},
#include "ctclib_malloc_sizes.h"
#undef MALLOC_SIZE
    {.size = 0xFFFFFFFF}
};

static inline void *
_memmgr_malloc (ctclib_mem_cache_t *cache, uint32 type)
{
    void *p = NULL;

    if (!cache)
        return NULL;

    CTC_MEMMGR_LOCK;

    p = ctclib_mem_cache_alloc(cache);
    if (!p)
    {
        CTC_MEMMGR_UNLOCK;
        return NULL;
    }

    g_mem_stats[type].size += ctclib_mem_cache_get_obj_size(cache);
    g_mem_stats[type].count++;

    CTC_MEMMGR_UNLOCK;
    return p;
}

static inline void *
_memmgr_calloc(ctclib_mem_cache_t *cache, uint32 type)
{
    void *p = NULL;

    if (!cache)
        return NULL;

    CTC_MEMMGR_LOCK;
    p = ctclib_mem_cache_alloc(cache);
    if (!p)
    {
        CTC_MEMMGR_UNLOCK;
        return NULL;
    }
    else
    {
        sal_memset(p, 0, ctclib_mem_cache_get_obj_size(cache));
    }

    g_mem_stats[type].size += ctclib_mem_cache_get_obj_size(cache);
    g_mem_stats[type].count++;

    CTC_MEMMGR_UNLOCK;
    return p;
}

void *
memmgr_malloc(uint32 type, uint32 size)
{
    uint32 i = 0;

    if (__builtin_constant_p(size)) {
#define MALLOC_SIZE(x) if (size <= x) goto _found; else i++;
#include "ctclib_malloc_sizes.h"
#undef MALLOC_SIZE
_found:
        return _memmgr_malloc(malloc_sizes[i].cache, type);
    }

#ifndef _CTC_X86_64_
    if (size <= 32 && size > 0)    /*0~32 bytes*/
        return _memmgr_malloc(malloc_sizes[(size -1)>>2].cache, type);
    else
        i = 8;
#endif

    while (size > malloc_sizes[i].size)
        i++;

    return _memmgr_malloc(malloc_sizes[i].cache, type);
}

void *
memmgr_calloc(uint32 type, uint32 size)
{
    uint32 i = 0;

    if (__builtin_constant_p(size)) {
#define MALLOC_SIZE(x) if (size <= x) goto _found; else i++;
#include "ctclib_malloc_sizes.h"
#undef MALLOC_SIZE
_found:
        return _memmgr_calloc(malloc_sizes[i].cache, type);
    }

#ifndef _CTC_X86_64_
    if (size <= 32 && size > 0)     /*0~32 bytes*/
        return _memmgr_calloc(malloc_sizes[(size -1)>>2].cache, type);
    else
        i = 8;
#endif

    while (size > malloc_sizes[i].size)
        i++;

    return _memmgr_calloc(malloc_sizes[i].cache, type);
}

char *
memmgr_strdup(uint32 type, const char *str)
{
    char   *str2 = NULL;
    uint32  len = 0;

    len = sal_strlen(str);
    str2 = memmgr_malloc(type, len + 1);
    if (str2 == NULL)
    {
        return NULL;
    }
    sal_strcpy(str2, str);

    return str2;
}

void
memmgr_free (uint32 type, void *ptr)
{
    uint32 size = 0;

    if (ptr == NULL)
        return;

    CTC_MEMMGR_LOCK;
    size = ctclib_mem_cache_get_obj_size_by_ptr(ptr);
    g_mem_stats[type].size -= size;
    g_mem_stats[type].count--;

    sal_memset(ptr, 0xFE, size);

    ctclib_mem_cache_free(ptr);
    CTC_MEMMGR_UNLOCK;
}

#define MEMMGR_LOG_ERR sal_printf

int32
memmgr_init(uint32 multi_thread)
{
    int32 i = 0;
    int32 rc = 0;

    /* Added by kcao to check mtype str mismatch, 2011-08-06 */
    //memmgr_check_mtype_str();

    /* modified by kcao for bug 13571, 2010-12-15 */
    if (multi_thread)
    {
        rc = sal_mutex_create(&g_p_memory_mutex);
        if (rc < 0)
        {
            MEMMGR_LOG_ERR("MEMMGR::Error while create a mutex");
            return rc;
        }
    }

    ctclib_mem_cache_init();

    while (malloc_sizes[i].size != 0xFFFFFFFF)
    {
        malloc_sizes[i].cache = ctclib_mem_cache_create(malloc_sizes[i].size, CTC_SLAB_CACHE_INTERNAL);
        i++;
    }

    return 0;
}

int32
memory_init(uint32 multi_thread)
{
    int32 rc = 0;
    rc = memmgr_init(multi_thread);
    return rc;
}

int32
memory_dump(FILE *fp)
{
    memory_stats_t *p_stats = NULL;
    int32 i;
    const char *desc = NULL;
    uint32 count;
    uint32 size;

    sal_fprintf(fp, "%-6s %-32s  %-12s %-12s\n", "Type", "Description", "Alloc Count", "Alloc Size");
    sal_fprintf(fp, "-------------------------------------------------------------------\n");

    for (i = 0; i < MEM_MAX_TYPE; i++)
    {
        desc = NULL;
        p_stats = &g_mem_stats[i];
        desc = g_mem_desc[i].desc;
        count = p_stats->count;
        size = p_stats->size;
        if (count || size)
        {
            sal_fprintf(fp, "%-6d %-32s: %-12d %-12d\n", i, (desc) ? desc : "NULL", count, size);
        }
    }

    return 0;
}

