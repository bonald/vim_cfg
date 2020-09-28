
#ifndef __CDB_CONST_H__
#define __CDB_CONST_H__

#define CDB_SUBSCRIBE_RESP_HDR_LEN     22

#define MAX_ZONE_NAME_LEN       32
#define MIN_ZONE_NAME_LEN       3 /*for bug 28442 modifed by liuyang 2014-5-12*/
#define ZONE_CONFIG_PATH        "/tmp/zone.conf"
#define ZONE_CONFIG_DEST        "/mnt/flash/.zone.conf"

#define MEM_INFO_PATH           "/proc/meminfo"
#define CPU_INFO_PATH           "/tmp/cpu_usage"
#define PYTHON_PS_PATH          "/tmp/python_ps"

#define CHECK_FULL_CDB_PMS      (PM_ID_CCS == pm_id) || (PM_ID_CDS == pm_id) || (PM_ID_FEA == pm_id)

#define MIRROR_DEST_FLAG                (1<<0)
#define MIRROR_DEST_REMOTE_FLAG         (1<<1)
#define MIRROR_SOURCE_IGS_FLAG          (1<<2)
#define MIRROR_SOURCE_EGS_FLAG          (1<<3)

#ifdef _GLB_UML_SYSTEM_
#define CTC_SCP_PROG        " /centec_switch/bin/scp"
#else
#define CTC_SCP_PROG        " /usr/bin/scp"
#endif

#define CTC_FTP_PROG        "ctcftp"
#define CTC_TFTP_PROG       "ctctftp"

#define STRING_SIZE 	16

#define IFNAME_EXT_SIZE     32
#define KERNEL_IFNAME_MAX_SIZE     16		 // kernel use 16 length
#define SYS_INFO_SIZE       128     // #define OEM_INFO_BUFSIZ 256
#define FILE_NAME_SIZE      256 //added by hansf

#define HOSTNAME_SIZE       63
#define DNS_HOSTNAME_SIZE   HOSTNAME_SIZE
#define COMM_NAME_SIZE  64
#define DHCP_NAME_MAX_LEN   63
#define DHCP_NAME_SIZE      128
#define CEM_NAME_SIZE 32
#define IP_ECMP_MAX_PATH  64
/* GB only support 8 ecmp nexthops, modified by liwh for bug 45595, 2017-11-17 */
#define IP_ECMP_MAX_PATH_GB  8
/* liwh end */

#if defined(GOLDENGATE) || defined(USW)
    #define ECMP_MAX_MEMBER  64
#else
    #define ECMP_MAX_MEMBER  8
#endif

/* make IFNAME same length with IFNAME_EXT_SIZE to keep simple. */
#define IFNAME_SIZE         IFNAME_EXT_SIZE
#define DESC_SIZE           64
#define IPSLA_DES_MAX_SIZE  255
#define VLANNAME_SIZE       16
#define ACL_REMARK_SIZE     100
#define TIME_RANGE_SIZE     40
#define TIME_RANGE_NAME_SIZE       TIME_RANGE_SIZE
#define QOS_NAME_SIZE       21
#define TAP_NAME_SIZE 21
#define TAP_DESC_SIZE 81
#define POLICER_NAME_SIZE         QOS_NAME_SIZE
#define QOS_DROP_NAME_SIZE        QOS_NAME_SIZE
#define QOS_SHAPE_NAME_SIZE       QOS_NAME_SIZE
#define LOG_DATA_LEN        256
#define LOG_ITEM_COUNT      1024
#define LSPNAME_SIZE         256
typedef char qos_name_t[QOS_NAME_SIZE+1];
typedef char ifname_info_t[IFNAME_SIZE];
#define USERNAME_SIZE       64
#define USERNAME_CLI_SIZE   32
#define PASSWD_SIZE         256
#define RSAKEY_NAME_SIZE    32
#define LINE_PROTOCOL_BUF_SIZE      64
#define LINE_AUTH_METHOD_NAME_SIZE  32
#define LINE_AUTH_METHOD_LINE_SIZE  128
#define VTY_BUF_SIZE                33
#define IPADDR_BUF_SIZE             128
#define M_RSA_KEY_NAMESZ            32
#define M_RSA_PEM_BUF_SIZE          8192
#define MACADDR_BUF_SIZE        20

#define HTTP_IMAGE_NAME_SIZE        64
#define REBOOT_STR_SIZE             32

#define HASH_FIELD_PROFILE_NAME_SIZE   64
#define HASH_VALUE_PROFILE_NAME_SIZE   64

/*OSPF NETWORK TYPE*/
#define OSPF_TYPE_NONE			0
#define OSPF_TYPE_BROADCAST 	1
#define OSPF_TYPE_NBMA 			2
#define OSPF_TYPE_P2M 			3
#define OSPF_TYPE_P2P 			4

/* OSPF AUTH */
/* syn with quagga define, 2017-03-07, OSPF_AUTH_SIMPLE_SIZE OSPF_AUTH_MD5_SIZE */
#define OSPF_AUTH_SIMPLE_MAX     8
#define OSPF_AUTH_MD5_MAX       16

/*RIP AUTH*/
#define RIP_AUTH_NONE		0
#define RIP_AUTH_TEXT		1
#define RIP_AUTH_MD5		2

#define RIP_SPLIT_ON 		0
#define RIP_SPLIT_OFF 		1
#define RIP_POISONED_ON 	1
#define RIP_POISONED_OFF 	0

#define RIPNG_SPLIT_ON 		0
#define RIPNG_SPLIT_OFF 	1
#define RIPNG_POISONED_ON 	1
#define RIPNG_POISONED_OFF 	0


/* for DHCP */
#define DHCP_SRV_ADDRS_BUF_SIZE     384
#define DHCP_SRV_ADDRS_NUM          8

/* for SNMP agent */
#define SNMP_COMMUNITY_SIZE         256 /* [1, 255] */
#define SNMP_NAME_SIZE              USERNAME_CLI_SIZE  /* [1, 31] */
#define SNMP_ENGINEID_MIN_SIZE      10
#define SNMP_ENGINEID_SIZE          65  /* snmp engine ID hex [10, 64] */
#define SNMP_INFO_SIZE              256 /* snmp system localtion/contact line */
#define SNMP_SUBTREE_SIZE           128 /* snmp view subtree x.y.z.... */
#define SNMP_MASK_SIZE              33  /* view subtree mask hex */
#define SNMP_PASSWORD_MIN_SIZE      8
#define SNMP_PASSWORD_SIZE          64  /* snmp password size [8, 64) */
#define SNMP_SPRINT_MAX_LEN         512 /* snmp cfg data line */
#define SNMP_TAGLIST_MAX_LEN        256 /* snmp tag list [1,256], max 128 tag name */
#define SNMP_DEFAULT_UDPPORT        162
#define SNMP_DEFAULT_TIMEOUT_CTC    2
#define SNMP_DEFAULT_RETRIES_CTC    3
#define SNMP_MAX_CONTEXT  32
#define SNMP_MAX_USER    32
#define SNMP_MAX_VIEW    32
#define SNMP_MAX_GROUP  32
#define SNMP_MAX_ACCESS  32
#define SNMP_MAX_NOTIFY  16
#define SNMP_MAX_TARGET_ADDR  16
#define SNMP_MAX_TARGET_PARAMS  32
#define SNMP_MAX_TRAP_RECEIVER  96
#define SNMP_MAX_TAGS 128

/* for RMON */
#define RMON_DESCR_LENGTH                      127
#define RMON_OWNER_NAME_SIZE                   15
#define RMON_ALARM_VAR_WORD_LENGTH             64

#define RMON_ALARM_VARIABLE_MAXSIZE            12
#define RMON_ETHERSTAT_DATASOURCE_MAXSIZE      11
#define RMON_HISTORY_CONTROL_MAXSIZE           11

#define RMON_DEFAULT_NAME             "RMON_SNMP"
#define RMON_HISTORY_CTRL_BR_DEFAULT           5 
#define RMON_HISTORY_CTRL_BG_DEFAULT           5
#define RMON_HISTORY_CTRL_SAMPLE_INTERVAL_DEFAULT  1800  /* In secs */

#define RMON_EVENT_NONE                        1 
#define RMON_EVENT_LOG                         2
#define RMON_EVENT_TRAP                        3
#define RMON_EVENT_LOG_TRAP                    4

#define RMON_ALARM_ABS                         1
#define RMON_ALARM_DELTA                       2

#define RMON_NO_ALARM                          2
#define RMON_RISE_ALARM                        1
#define RMON_FALL_ALARM                        0

typedef struct
{
    uint32     history_index;
    uint32     stats_index;
} rmon_stats_history_key_t;

/*for NTP*/
#define NTP_MAX_HOST_IP_SIZE   (M_HOSTNAME_MAXSIZE+1)
#define NTP_MAX_KEY_LEN         32

/*for CEM*/
#define CEM_ADDRESS_SIZE              1024
#define CEM_SUBJECT_SIZE              64
#define CEM_BODY_SIZE                 512
/* seconds */
#define M_EXEC_TIMEOUT_MIN_DEFAULT  10
#define M_EXEC_TIMEOUT_MIN_MAX      35791
#define M_EXEC_TIMEOUT_SEC_MAX      2147483

#define MSTP_REGION_NAME_LEN            32
#define MSTP_DIGEST_LEN                 16
#define MSTP_DEFAULT_INSTANCE           0
/*openflow*/
#define OPENFLOW_PROTECTED_VLAN  50
#define OPENFLOW_PACKET_ACTION          128
#define CONTROLLER_NAME_SIZE  64

typedef struct
{
    uint8   data[MSTP_DIGEST_LEN];
} mstp_digest_t;

#define UINT64_STR_LEN      21
typedef enum 
{
    TIME_RANGE_ARG_TYPE_ACE = 0,
    TIME_RANGE_ARG_TYPE_POE,

    TIME_RANGE_ARG_TYPE_MAX
} time_range_arg_type_t;

#define RT_DISTANCE_DEFAULT         1
#define RT_DISTANCE_MIN             1
#define RT_DISTANCE_MAX             255

#define RT_IF_ADDRESS_MAX           9

#define RT_IF_ADDRESS_V6_MAX           8

#define RT_IF_ADDR_SECONDARY        0x01
#define RT_IF_ADDR_ANYCAST          0x02
#define RT_IF_ADDR_VIRTUAL          0x04
#define RT_IF_ADDR_DUPLICATE        0x08
#define RT_IF_ADDRV6_GLOBAL        0x10
#define RT_IF_ADDRV6_LINKLOCAL        0x20
#define RT_IF_ADDRV6_AUTO_LINKLOCAL        0x40
#define RT_IF_ADDRV6_GLOBAL_EUI64        0x80
#define RT_IF_ADDRV6_DAD_IN_PROGRESS       0x100
#define RT_IF_ADDRV6_NA_IN_PROGRESS         0x200
#define RT_IF_ADDRV6_CONFIGURED        0x400
#define RT_IF_ADDRV6_REAL                     0x800

#define LINK_LOCAL_PREFIX_LEN    10
#define LINK_LOCAL_PREFIX_LEN_LINUX_DEFAULT    64



#define EUI64_HOST_BITS                64
#define EUI64_HOST_BYTES              8

/* same to ZEBRA_FLAG_XXX */
#define RIB_FLAG_INTERNAL           0x01
#define RIB_FLAG_SELFROUTE          0x02
#define RIB_FLAG_BLACKHOLE          0x04
#define RIB_FLAG_IBGP               0x08
#define RIB_FLAG_SELECTED           0x10    /* useful */
#define RIB_FLAG_CHANGED            0x20
#define RIB_FLAG_STATIC             0x40

#define NEXTHOP_FLAG_ACTIVE         0x01    /* The nexthop is alive. */
#define NEXTHOP_FLAG_FIB            0x02    /* FIB nexthop. */
#define NEXTHOP_FLAG_RECURSIVE      0x04    /* Recursive nexthop. */
#define NEXTHOP_FLAG_REMOVED        0x08    /* The nexthop is going to be removed. */
#define NEXTHOP_FLAG_OS             0x10    /* The nexthop is synced to OS. */

#define DISTANCE_INFINITY           255

#define ERRDISABLE_DEFAULT_INTERVAL        (300)
#define ERRDIS_NO_ERRDIS                (0)

#define GLB_FDB_LOOP_BUCKET_SIZE_DEF    60000
#define GLB_FDB_LOOP_BUCKET_RATE_DEF    200

#define GLB_LINK_FLAP_MAX_FLAP_COUNT   10
#define GLB_LINK_FLAP_MAX_FLAP_SECOND  10

#define SFLOW_COUNTER_DEFAULT_INTERVAL  20

#define ERPS_MAX_NAME_LEN               16
#define ERPS_MAX_INSTANCE               64
#define G8032_MAX_INSTANCE              4
#define ERPS_MIN_INSTANCE_NO  0
#define ERPS_MAX_INSTANCE_NO  4094

#define  CFM_MD_NAME_MAX_LENGTH_8021AG     43
#define  CFM_MD_NAME_MIN_LENGTH                      1

#define  CFM_MA_NAME_MAX_LENGTH_8021AG     43
#define  CFM_MA_NAME_MIN_LENGTH                      1

#define CFM_MAID_MAX_LENGTH                            48

typedef enum
{
    FLUSH_FDB_TYPE_FDB =0,
    FLUSH_FDB_TYPE_MACFILTER,
    FLUSH_FDB_TYPE_PSFDB,
    FLUSH_FDB_TYPE_MAX,
} flush_fdb_type_t;

typedef enum
{
    FLUSH_FDB_SUBTYPE_STATIC =0,
    FLUSH_FDB_SUBTYPE_DYNAMIC,
    FLUSH_FDB_SUBTYPE_MULTICAST,
    FLUSH_FDB_SUBTYPE_MAX,
} flush_fdb_subtype_t;

typedef enum
{
    FLUSH_FDB_MODE_ALL =0,
    FLUSH_FDB_MODE_PORT,
    FLUSH_FDB_MODE_VLAN,
    FLUSH_FDB_MODE_PORT_VLAN,
    FLUSH_FDB_MODE_MAC,
    /* modified by liwh for bug 37200, 2016-08-25 */
    FLUSH_FDB_MODE_ERPS,
    /* liwh end */
    FLUSH_FDB_MODE_MAX,
} flush_fdb_mode_t;

/* Route types. */
typedef enum
{
    ROUTE_TYPE_DEFAULT = 0,
    ROUTE_TYPE_KERNEL,
    ROUTE_TYPE_CONNECT,
    ROUTE_TYPE_STATIC,
    ROUTE_TYPE_RIP,
    ROUTE_TYPE_RIPNG,
    ROUTE_TYPE_OSPF,
    ROUTE_TYPE_OSPF6,
    ROUTE_TYPE_BGP,
    ROUTE_TYPE_MAX,
} route_type_t;

typedef enum
{
    NH_TYPE_IFINDEX = 0,
    NH_TYPE_IPV4,
    NH_TYPE_IPV4_IFINDEX,
    NH_TYPE_IPV6,
    NH_TYPE_IPV6_IFINDEX,
    NH_TYPE_BLACKHOLE,
    NH_TYPE_LOCAL,
    NH_TYPE_MAX,
} nh_type_t;

typedef enum
{
    AS_INIT       = 0,  /* MAC address not resolved in FEA, This ARP entry should be removed */
    AS_INCOMPLETE    ,  /* MAC address not resolved in FEA, But want to be resolved and sent ARP Request out already */
    AS_REACHABLE     ,  /* MAC address resolved in FEA */
    AS_REFRESH       ,  /* MAC address resolved in FEA, But aging timer expired and sent ARP Request out already */
    //AS_ASYNC       ,  /* MAC address resolved in FEA, because of HW ARP entry ADD or DEL operational fail.*/
    AS_MAX
} as_t;

#define RT_CONNECT_DISTANCE         0

#define ARP_SYN_HW                  0x01
#define ARP_SYN_OS                  0x02
#define ARP_SYN_FDB_AGE             0x04
#define ARP_DONOT_INSTALL           0x08

#define ARP_AGING_TIME_DEFAULT      (60*60) /* default aging delay */
#define ARP_RETRY_TIME_DEFAULT      1       /* default retry delay */
#define ARP_GRA_RSP_TIME_DEFAULT    3      /*default for gra rsp*/

#define ICMP_ERROR_RATELIMIT        1000
#define MAX_STATIC_ROUTE_DEFAULT    1024

/*ARP INSPECTION*/
#define ARPINSP_FILTER_MAX_NUM    20
#define ARPACL_NAME_SIZE          20

/*G8032*/
#define GLB_G8032_RING_DEFAULT_GUARD_TIMER_INTERVAL_MS 500
#define GLB_G8032_RING_DEFAULT_WTR_TIMER_INTERVAL_IN_MIN 5 
#define GLB_G8032_DEF_RAPS_MEL_VALUE        7
/* kernel arp type */
#if 0
typedef enum
{
    ARP_TYPE_IMCOMPLETE = 0,
    ARP_TYPE_DYNAMIC ,
    ARP_TYPE_STATIC,
    ARP_TYPE_MAX,
} kernel_arp_type_t;
#else
typedef enum
{
    ARP_TYPE_DYNAMIC = 0,
    ARP_TYPE_STATIC,
    ARP_TYPE_MAX,
} arp_type_t;
#endif

/* For ip source guard */
enum ipsour_type
{
    IP_NONE,
    IP_FILTERING,
    IP_MAC_FILTERING,
    IP_VLAN_FILTERING,
    IP_MAC_VLAN_FILTERING,
    IP_TYPE_MAX,
};

typedef enum
{
    BOOT_MODE_FLASH = 0,
    BOOT_MODE_TFTP,
    BOOT_MODE_TFTP6,
    BOOT_MODE_MAX
} boot_mode_t;

typedef enum
{
    SSH_VERSION_V1 = 0,
    SSH_VERSION_V2,
    SSH_VERSION_ALL,
    SSH_VERSION_MAX
} ssh_version_t;

#define SSH_AUTH_TYPE_PASSWORD  0x01
#define SSH_AUTH_TYPE_PUBKEY    0x02
#define SSH_AUTH_TYPE_RSA       0x04

#define M_SRV_ADDR_BUF_SZ       256 /* hold 3 ipv4/6 addresses and vrfid */

typedef enum
{
    SYSLOG_FACILITY_KERN = 0,
    SYSLOG_FACILITY_USER,
    SYSLOG_FACILITY_MAIL,
    SYSLOG_FACILITY_DAEMON,
    SYSLOG_FACILITY_AUTH,
    SYSLOG_FACILITY_SYSLOG,
    SYSLOG_FACILITY_LPR,
    SYSLOG_FACILITY_NEWS,
    SYSLOG_FACILITY_UUCP,
    SYSLOG_FACILITY_CRON,
    SYSLOG_FACILITY_AUTHPRIV,
    SYSLOG_FACILITY_FTP,
    SYSLOG_FACILITY_LOCAL0 = 16,
    SYSLOG_FACILITY_LOCAL1,
    SYSLOG_FACILITY_LOCAL2,
    SYSLOG_FACILITY_LOCAL3,
    SYSLOG_FACILITY_LOCAL4,
    SYSLOG_FACILITY_LOCAL5,
    SYSLOG_FACILITY_LOCAL6,
    SYSLOG_FACILITY_LOCAL7,
    SYSLOG_FACILITY_MAX,
} syslog_facility_t;

typedef enum
{
    SYSLOG_SEVERITY_EMERG = 0,
    SYSLOG_SEVERITY_ALERT = 1,
    SYSLOG_SEVERITY_CRIT = 2,
    SYSLOG_SEVERITY_ERR = 3,
    SYSLOG_SEVERITY_WARNING = 4,
    SYSLOG_SEVERITY_NOTICE = 5,
    SYSLOG_SEVERITY_INFO = 6,
    SYSLOG_SEVERITY_DEBUG = 7,
    SYSLOG_SEVERITY_MAX
} syslog_severity_t;

typedef enum
{
    SYSLOG_TIMESTAMP_NONE = 0,  /* None timestamp */
    SYSLOG_TIMESTAMP_BSD,       /* BSD format */
    SYSLOG_TIMESTAMP_RISO,      /* ISO format */
    SYSLOG_TIMESTAMP_RFULL,     /* full format */
    SYSLOG_TIMESTAMP_MAX
} syslog_timestamp_t;

typedef enum
{
    LOGIN_METHOD_NOLOGIN = 0,   /* no login */
    LOGIN_METHOD_LOGIN,         /* login, check line password */
    LOGIN_METHOD_LOCAL,         /* local, check local user + password */
    LOGIN_METHOD_MAX
} login_method_t;

typedef struct
{
    char name[USERNAME_SIZE];
} user_t;

typedef enum
{
    PARITY_NONE = 0,    /* none */
    PARITY_EVEN,        /* even */
    PARITY_ODD,         /* odd */    
    PARITY_MAX
} parity_t;

typedef enum
{
    AUTHEN_METHOD_NONE = 0,         /* No authentication */
    AUTHEN_METHOD_ENABLE_PASSWD,    /* enable password */
    AUTHEN_METHOD_LINE_PASSWD,      /* line password */
    AUTHEN_METHOD_LOCAL,            /* local username */
    AUTHEN_METHOD_RADIUS,           /* Radius server */
    AUTHEN_METHOD_TACACS_PLUS,      /* Tacacs+ server */
    AUTHEN_METHOD_MAX
} authen_method_t;

typedef enum
{
    AUTHOR_METHOD_NONE = 0,         /* No authorization */
    AUTHOR_METHOD_LOCAL,            /* local username */
    AUTHOR_METHOD_RADIUS,           /* Radius server */
    AUTHOR_METHOD_TACACS_PLUS,      /* Tacacs+ server */
    AUTHOR_METHOD_MAX
} author_method_t;

typedef enum
{
    ACCOUNT_METHOD_NONE = 0,         /* No accounting */
    ACCOUNT_METHOD_RADIUS,           /* Radius server */
    ACCOUNT_METHOD_TACACS_PLUS,      /* Tacacs+ server */
    ACCOUNT_METHOD_MAX
} account_method_t;

typedef enum
{
    ACCOUNTCMD_METHOD_NONE = 0,      /* No accounting for commands */
    ACCOUNTCMD_METHOD_TACACS_PLUS,   /* Tacacs+ server */
    ACCOUNTCMD_METHOD_MAX
} accountcmd_method_t;

typedef enum
{
    KEY_TYPE_PUBLIC = 0,
    KEY_TYPE_PRIVATE,
    KEY_TYPE_MAX
} key_type_t;

typedef enum
{
    AGENT_PROC_OPTION_APPEND = 0,
    AGENT_PROC_OPTION_REPLACE,
    AGENT_PROC_OPTION_UNTOUCHED,
    AGENT_PROC_OPTION_DISCARD,
    AGENT_PROC_OPTION_MAX
} agent_proc_option_t;

typedef enum
{
    SNMP_VERSION_V1 = 0,
    SNMP_VERSION_V2C,
    SNMP_VERSION_V3,
    SNMP_VERSION_ALL,
    SNMP_VERSION_MAX
} snmp_version_t;

typedef enum
{
    SNMP_AUTHENTICATION_MD5 = 1,
    SNMP_AUTHENTICATION_SHA,
    SNMP_AUTHENTICATION_MAX
} snmp_authentication_t;

typedef enum
{
    SNMP_PRIVACY_DES = 1,
    SNMP_PRIVACY_AES,
    SNMP_PRIVACY_MAX
} snmp_privacy_t;

typedef enum
{
    DHCBINDING_STATE_REQUEST = 0,
    DHCBINDING_STATE_RUNNING,
    DHCBINDING_STATE_ALL,
    DHCBINDING_STATE_MAX
} dhcbinding_state_t;

typedef enum
{
    DHCBINDING_TYPE_LEARNING = 0,
    DHCBINDING_TYPE_CONFIGED,
    DHCBINDING_TYPE_ALL,
    DHCBINDING_TYPE_MAX
} dhcbinding_type_t;

typedef enum
{
    AUTH_SERVER_TYPE_RADIUS = 0,
    AUTH_SERVER_TYPE_TACPLUS = 1,
    AUTH_SERVER_TYPE_MAX
} auth_server_type_t;

typedef enum
{
    AUTH_SESSION_STATUS_INIT = 0,
    AUTH_SESSION_STATUS_INPROGRESS = 1,
    AUTH_SESSION_STATUS_SUCCESS = 2,
    AUTH_SESSION_STATUS_FAILED = 3,
    AUTH_SESSION_STATUS_DENIED = 4,
    AUTH_SESSION_STATUS_MAX
} auth_session_status_t;

typedef enum
{
    MIRROR_DEST_NONE,
    MIRROR_DEST_VLAN,
    MIRROR_DEST_PORT,
    MIRROR_DEST_GROUP,
    MIRROR_DEST_CPU,
    MIRROR_DEST_MAX
} mirror_dest_type_t;

typedef enum
{
    SNMP_VIEW_CLASS_INCLUDE = 0,
    SNMP_VIEW_CLASS_EXCLUDE,
    SNMP_VIEW_CLASS_MAX
} snmp_view_class_t;

typedef enum
{
    SNMP_COMMUNITY_RO = 0,
    SNMP_COMMUNITY_RW,
    SNMP_COMMUNITY_MAX
} snmp_community_t;

typedef enum
{
    SNMP_LEVEL_NOAUTH = 1,
    SNMP_LEVEL_AUTHNOPRIV,
    SNMP_LEVEL_AUTHPRIV,
    SNMP_LEVEL_MAX
} snmp_usm_level_t;


typedef enum
{
    SCHEDULER_MODE_SP = 0,
    SCHEDULER_MODE_DWRR,
    SCHEDULER_MODE_MAX
} scheduler_mode_t;

typedef enum
{
    CDB_ID_INSTANCE = 0,
    CDB_ID_PVLAN_ISOLATE_ID,
    CDB_ID_MAX
} cdb_id_type_t;

typedef enum
{
    MLAG_ROLE_MASTER = 0,
    MLAG_ROLE_SLAVE,
    MLAG_ROLE_MAX
} mlag_role_t;

typedef uint32 bmp_32_t[1];
typedef uint32 bmp_64_t[2];

#define GLB_LOG_ID_BMP_WORD_MAX      ((LOG_ID_MAX + GLB_UINT32_BITS) / GLB_UINT32_BITS)
typedef uint32_t logid_bmp_t[GLB_LOG_ID_BMP_WORD_MAX];

typedef struct sal_timeval timestamp_t;

typedef union
{
    addr_ipv4_t ipv4;
    addr_ipv6_t ipv6;
} nexthop_key_u;

typedef struct
{
    uint16              vrf_id;
    prefix_t            p;
    nexthop_key_u       nh_addr;    
    uint32              ifindex_v6;//for ipv6 route xx xx interface
} route_node_key_t;

typedef struct
{
    uint16              vrf_id;
    prefix_t            p;
    nexthop_key_u       nh_addr;    
    uint32              ifindex_v6;//for ipv6 route xx xx interface
    uint32               distance;
} route_node_key_with_ad_t;

typedef struct
{
    char itf_name[IFNAME_SIZE];
    addr_ipv4_t ip_addr;
}source_type_t;

typedef struct
{
    uint32       update;
    uint32       timeout;
    uint32       garbage;
} rip_timers_t;

typedef struct
{
    uint32            area_id;
} ospf_area_t;

typedef struct
{
    uint32            area_id;
    prefix_t          p;
} ospf_area_range_key_t;

typedef struct
{
    uint32            area_id;
    prefix_t          p;
} ospf6_area_range_key_t;

typedef struct
{
        uint32            area_id;
            addr_ipv4_t       vl_peer;
} ospf_area_vlink_key_t;

typedef struct
{
    uint32               type;      /* nh_type_t */
    char                 ifname[IFNAME_SIZE];
    nexthop_key_u        u;
} nexthop_key_t;

typedef struct
{
    uint32        rr_en;
    uint32        nh_num;
    nexthop_key_t nh[IP_ECMP_MAX_PATH];
} nexthop_ecmp_t;

typedef struct
{
    uint32        nh_num;
    nexthop_key_t nh[ECMP_MAX_MEMBER];
} nexthop_ecmp_routed_t;

typedef enum
{
    DS_BRGIF_ALLOWED_VLAN_ACT_ADD   = 0,
    DS_BRGIF_ALLOWED_VLAN_ACT_DEL   = 1,
    DS_BRGIF_ALLOWED_VLAN_ACT_ALL   = 2,
    DS_BRGIF_ALLOWED_VLAN_ACT_NONE  = 3,
    DS_BRGIF_ALLOWED_VLAN_ACT_MAX
} ds_brgif_allowed_vlan_act_t;

typedef enum
{
    DS_BRGIF_UNTAGGED_VLAN_ACT_ADD   = 0,
    DS_BRGIF_UNTAGGED_VLAN_ACT_DEL   = 1,
    DS_BRGIF_UNTAGGED_VLAN_ACT_MAX
} ds_brgif_untagged_vlan_act_t;

typedef struct
{
    vlan_bmp_t              vlan_bmp;
    uint8                   action;             /* ds_brgif_allowed_vlan_act_t */
} brg_allowed_vlan_t;

typedef enum
{
    CDB_SYNC_START      = 0,
    CDB_SYNC_STOP       = 1,
    CDB_SYNC_MAX
} cdb_sync_t;

typedef enum
{
    CDB_OPER_ADD        = 0,
    CDB_OPER_SET        = 1,
    CDB_OPER_GET        = 2,
    CDB_OPER_DEL        = 3,
    CDB_OPER_SHOW       = 4,
    CDB_OPER_VIEW       = 5,
    CDB_OPER_BUILDCFG   = 6,
    CDB_OPER_SYNC_REFER = 7,
    CDB_OPER_GETJSON    = 8,
    CDB_OPER_MAX
} cdb_oper_t;

typedef enum
{
    CDB_REFER_TBL,
    CDB_REFER_TIMER,
    CDB_REFER_MAX
} cdb_refer_type_t;

typedef enum
{
    CDB_REFER_OPER_ADD,
    CDB_REFER_OPER_SET,
    CDB_REFER_OPER_DEL,
    CDB_REFER_OPER_MAX
} cdb_refer_oper_t;

typedef enum
{
    CDB_CFG_CLI      = 0,
    CDB_CFG_NC_SET   = 1,
    CDB_CFG_NC_CHECK = 2,
    CDB_CFG_MAX
} cdb_cfg_t;

typedef struct
{
    uint16 type;        /* cdb_refer_type_t */
    uint16 id;          /* cdb_tbl_t for CDB_REFER_TBL, */
    uint16 oper;        /* cdb_refer_oper_t */
    uint16 size;        /* object size */
} cdb_sync_refer_hdr_t;

#define CDB_OBJECT_BUF_SIZE     256

typedef struct
{
    cdb_sync_refer_hdr_t    hdr;
    uint8                   object[CDB_OBJECT_BUF_SIZE];
} cdb_sync_refer_t;

typedef struct
{
    mac_addr_t          mac;
    mac_addr_t          mask;
} mirror_mac_escape_key_t;

typedef struct
{
    char arpacl_name[ARPACL_NAME_SIZE+1];
    uint32 seq_no;
} arpace_config_key_t;

typedef struct
{
    char acl_name[ACL_NAME_FULL_NAME_SIZE+1];
    uint32 seq_no;
} ace_config_key_t;

typedef struct
{
    char policy_name[PMAP_NAME_SIZE+1];
    char class_name[CMAP_NAME_SIZE+1];
} class_in_policy_config_key_t;

typedef struct
{
    char class_name[CMAP_NAME_SIZE+1];
    char acl_name[ACL_NAME_FULL_NAME_SIZE+1];
} acl_in_class_config_key_t;

typedef struct
{
    char policy_name[PMAP_NAME_SIZE+1];
    char class_name[CMAP_NAME_SIZE+1];
} class_map_action_config_key_t;

typedef struct
{
    uint8            ipv6;
    mac_addr_t       mac;
    vid_t                vid;                 /* VLAN id */
} binding_key_t;

typedef struct
{
    addr_t               addr;
    auth_server_type_t   type;
    uint32               sin_port;
} auth_server_key_t;

#define M_AUTH_NAME_SIZE        32
typedef struct
{
    char                name[M_AUTH_NAME_SIZE]; /* auth module name */
    uint32              index; /* auth index, pid or port id */
    auth_server_type_t  type;  /* auth server type */
} auth_session_key_t;

typedef struct
{
    char            name[IFNAME_SIZE];    
    uint16          instance;
} msti_port_key_t;

typedef enum 
{
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    DAILY,
    WEEKDAY,
    WEEKEND,
    MAXDAY,
} weekday_t;

typedef enum 
{
    JANUARY = 0,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER,
    MAX
} month_t;

typedef struct 
{
    sal_time_t start;
    sal_time_t end;
} absolute_time_t;

typedef struct
{
    weekday_t day;
    uint8 hour;
    uint8 min;
} per_time_t;

typedef struct 
{
    per_time_t start;
    per_time_t end;
} periodic_time_t;

typedef struct
{
    uint8               type;   /* GLB_TIME_RANGE_TYPE_XXX */
    union
    {
      absolute_time_t ab_time;
      periodic_time_t per_time;
    };
} time_range_value_t;

typedef struct
{
    uint16              type;   /* cdb_node_type_t */
    uint16              id;
    void                *obj;
} cdb_reference_t;

typedef struct
{
    uint16              type;   /* cdb_node_type_t */
    uint16              id;
    ctclib_slist_t      *obj_list;
} cdb_reference_list_t;

typedef struct
{
//  void            *acl;   /* time_range_arg_type_t */
    void            *ace;   /* mac ace/ip ace/etc */
//  uint8           ace_afi;
    uint64           acl_id;
} time_range_arg_ace_t;

typedef struct
{
//    ctclib_list_node_t  node;
    uint8               type;   /* time_range_arg_type_t */
    void *              arg;
} time_range_arg_t;

typedef struct
{
    uint8  status;
    ctc_task_t *timer;
    ctclib_slist_t      arg_list;  /*time_range_arg_t */
    
} time_range_timer_t;

typedef struct
{
    uint8  dest_mac[MAC_ADDR_LEN];
    uint8  src_mac[MAC_ADDR_LEN];
    uint16 ether_type;
} eth_hdr_t;


typedef struct//add by yangl
{
   uint8 version_hdrlen;
   uint8 tp_service;
   uint16 total_length;
   uint16 identifier;
   uint16 flags_offset;
   uint8 time_to_live;
   uint8 protocol;
   uint16 hdr_check;
   addr_ipv4_t src_ip_addr;
   addr_ipv4_t dst_ip_addr;
} ip_v4_hdr;


typedef struct  //add by yangl
{
    uint16 src_Port_id;
    uint16 dst_Port_id;
    uint16 udp_length;
    uint16 udp_CheckSum;
}sflow_udp_hdr;

typedef struct
{
    uint16 src_Port_id;
    uint16 dst_Port_id;
    uint32 seq_num;
    uint32 ack_num;
    uint16 hdrlen_flags;
    uint16 win_size;
    uint16 tcp_CheckSum;
    uint16 data_offset;
}sflow_tcp_hdr;


typedef struct
{
    char  shape[QOS_SHAPE_NAME_SIZE];
    char  drop[QOS_DROP_NAME_SIZE];
    uint8 class;
    uint8 weight;
} aclqos_if_queue_t;

typedef struct
{
    uint32 cir;
    uint32 eir;
    uint32 cbs;
    uint32 ebs;
    uint8 mode;      /* glb_qos_policer_mode_t */      
    uint8 pps_en;
    uint8 direct;      /* 0:ingress, 1: egress */
    uint8 type;        /* 0:port policer, 1: flow policer */
}qos_policer_res_key_t;

typedef struct
{
    char buf[M_RSA_PEM_BUF_SIZE];
} rsa_keystr_t;

typedef struct
{
    uint16 priority;
    uint8  address[MAC_ADDR_LEN];
} mstp_brg_id_t;

typedef enum
{
    MSTP_PATH_COST_1T = 0,
    MSTP_PATH_COST_1D_1998,
    MSTP_PATH_COST_MAX
} mstp_path_cost_t;

typedef struct
{
    addr_t                  ip_addr;                         /* read-only or read-write */
    uint8                   is_mgmt_if;                 
    char                    community[SNMP_COMMUNITY_SIZE];   /* community name */
    uint32                  udpport;                        /* trap port */
} snmp_trap_key_t;

typedef snmp_trap_key_t snmp_inform_key_t;

typedef struct
{
    char name[SNMP_NAME_SIZE];
    //snmp_view_class_t type;
    char subtree[SNMP_SUBTREE_SIZE];
    
} snmp_view_key_t;

typedef struct
{
    char grp_name[SNMP_NAME_SIZE];
    char security_name[SNMP_NAME_SIZE];
    
} snmp_group_usm_key_t;

typedef struct
{
    char info[SNMP_INFO_SIZE];
} snmp_info_t;

typedef struct
{
    char buf[255];
} openflow_keystr_t;

typedef struct
{
    char tap_group[TAP_NAME_SIZE];
    char if_name[IFNAME_SIZE];
} tap_group_ingress_key_t;

typedef struct
{
    char tap_group[TAP_NAME_SIZE];
    char if_name[IFNAME_SIZE];
    char flow_name[ACL_NAME_FULL_NAME_SIZE+1];
} tap_group_ingress_flow_key_t;

typedef struct
{
    char tap_group[TAP_NAME_SIZE];
    char if_name[IFNAME_SIZE];
} tap_group_egress_key_t;

typedef struct
{
    addr_ipv4_t  ip_sa;
    addr_ipv4_t  ip_da;
    uint16       l4_src_port;
    uint16       l4_dst_port;
    uint8        l4_protocol;
    uint8        keysel_type;
} tap_five_tuple_key_t;

typedef enum
{
    TAP_TUPLE5_KEYSEL_ALL      = 0,
    TAP_TUPLE5_KEYSEL_IPDA     = 1,
    TAP_TUPLE5_KEYSEL_IPSA     = 2,
    TAP_TUPLE5_KEYSEL_DSTPORT  = 3,
    TAP_TUPLE5_KEYSEL_SRCPORT  = 4,
    TAP_TUPLE5_KEYSEL_PROTOCOL = 5
} tap_five_tuple_key_select_t;

typedef struct
{
    uint32 id[GLB_AGG_IFINDEX_MAX];
    uint32 ifindex[GLB_AGG_IFINDEX_MAX];
} tap_tuple_acl_entry_id_t;

typedef enum
{
    NS_PORT_FORWARDING_LOCAL = 0,
    NS_PORT_FORWARDING_REMOTE,
    NS_PORT_FORWARDING_TYPE_MAX
} ns_port_forwarding_type_t;

typedef enum
{
    NS_PORT_FORWARDING_TCP = 0,
    NS_PORT_FORWARDING_UDP,
    NS_PORT_FORWARDING_PROTO_MAX
} ns_port_forwarding_proto_t;

typedef struct
{
    ns_port_forwarding_type_t   type;
    ns_port_forwarding_proto_t  proto;
    uint16                      port;
    uint16                      pad;
    addr_ipv4_t                 mapped_addr; /* for local service, addr is always 0.0.0.0,
                                                for remote service, addr default is 169.254.1.2,
                                                and can be 169.254.2.2, 169.254.3.2. */
} ns_port_forwarding_key_t;

typedef enum
{
    ERRDIS_ARP_NUMBERLIMIT = 0,
    ERRDIS_ARP_RATE_LIMIT,
    ERRDIS_BPDU_GUARD,
    ERRDIS_BPDU_LOOP,
    ERRDIS_FDB_LOOP,
    ERRDIS_LINK_MON_FAILURE, 
    ERRDIS_LINK_FLAP,
    ERRDIS_LOOPBACK_DETECTION,
    ERRDIS_MONITOR_LINK,
    ERRDIS_OAM_REM_FAILURE,
    ERRDIS_PORT_SECURITY_VIOLATION,
    ERRDIS_MLAG_RELOAD_DELAY,
    ERRDIS_UDLD_DISABLE,
    ERRDIS_RSN_MAX,
} errdis_rsn_t;

typedef enum
{
    ERRDIS_FLAP_REASON_LINK = 0,   /* Link flap */
    ERRDIS_FLAP_REASON_MAX,
} errdis_flap_rsn_t;

typedef int32 (*FLAP_TIMER_FUNC)(void);

typedef struct
{
    char                    server[CEM_ADDRESS_SIZE];
    char                    from[CEM_ADDRESS_SIZE];
    char                    to[CEM_ADDRESS_SIZE];
    char                    cc[CEM_ADDRESS_SIZE];
    char                    subject[CEM_SUBJECT_SIZE];
    char                    body[CEM_BODY_SIZE];
    char                    attach[CEM_ADDRESS_SIZE]; 
} mail_t;

/*vlan_class rule protocol type*/
typedef enum
{
    VLANCLASS_PROTYPE_NONE = 0,
    VLANCLASS_PROTYPE_IPV4,
    VLANCLASS_PROTYPE_IPV6,
    VLANCLASS_PROTYPE_MPLS,
    VLANCLASS_PROTYPE_MPLSMCAST,
    VLANCLASS_PROTYPE_IPV4ARP,
    VLANCLASS_PROTYPE_IPV4RARP,
    VLANCLASS_PROTYPE_EAPOL,
    VLANCLASS_PROTYPE_8021AG,
    VLANCLASS_PROTYPE_SLOWPROTO,
    VLANCLASS_PROTYPE_PPPOE,
    VLANCLASS_PROTYPE_FLEXL3,
    VLANCLASS_PROTYPE_MAX
} vlanclss_rule_protocol_type_t;

/*vlan_class rule protocol action*/
typedef enum
{
    VLANCLASS_PROTYPE_TO_CPU,
    VLANCLASS_PROTYPE_DISCARD,
    VLANCLASS_PROTYPE_REPLACETAG,
} vlanclss_rule_protocol_action_t;

/* 802.1AB-2005 9.5.2.2 chassis ID subtype */
typedef enum
{
    LLDP_CHASSIS_ID_SUBTYPE_RESERVED                  = 0,
    LLDP_CHASSIS_ID_SUBTYPE_CHASSIS_COM               = 1,
    LLDP_CHASSIS_ID_SUBTYPE_IF_ALIAS                  = 2,
    LLDP_CHASSIS_ID_SUBTYPE_PORT_COM                  = 3,
    LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS               = 4,
    LLDP_CHASSIS_ID_SUBTYPE_NET_ADDRESS               = 5,
    LLDP_CHASSIS_ID_SUBTYPE_IF_NAME                   = 6,
    LLDP_CHASSIS_ID_SUBTYPE_LOCALLY_ASSINGED          = 7,
    LLDP_CHASSIS_ID_SUBTYPE_MAX         
} lldp_chassis_id_sub_type_t;

/* 802.1AB-2005 9.5.3.2 port ID subtype */
typedef enum
{
    LLDP_PORT_ID_SUBTYPE_RESERVED                     = 0,
    LLDP_PORT_ID_SUBTYPE_IF_ALIAS                     = 1,
    LLDP_PORT_ID_SUBTYPE_PORT_COM                     = 2,
    LLDP_PORT_ID_SUBTYPE_MAC_ADDRESS                  = 3,
    LLDP_PORT_ID_SUBTYPE_NET_ADDRESS                  = 4,
    LLDP_PORT_ID_SUBTYPE_IF_NAME                      = 5,
    LLDP_PORT_ID_SUBTYPE_AGENT_CIRCUITID              = 6,
    LLDP_PORT_ID_SUBTYPE_LOCALLY_ASSINGED             = 7,
    LLDP_PORT_ID_SUBTYPE_MAX            
} lldp_port_id_sub_type_t;

#define LLDP_NAME_SIZE              64
#define LLDP_DESCRIPTION_SIZE       255

#define CHASSIS_ID_TLV_MIN_SIZE     2
#define CHASSIS_ID_TLV_MAX_SIZE     256

#define PORT_ID_TLV_MIN_SIZE        2
#define PORT_ID_TLV_MAX_SIZE        256

#define LLDP_MED_INVENTORY_VALUE_SIZE 32

#define LLDP_IF_NUMBERING_IFINDEX 2
#define MANAGEMENT_ADDRESS_OID_STRING_LENGTH  0

/*lldp_neighber*/
#define LLDP_NAME_MAX_SIZE              64
#define LLDP_VLAN_NAME_SIZE             32
#define LLDP_PROTOCOL_ID_LENGTH         12
#define LLDP_MGMT_ADDR_LENGTH           31
#define LLDP_TIME_MARK_INDEX_MAX        10
#define LLDP_MSG_TX_INTERVAL_MIN        5
#define LLDP_MSG_TX_INTERVAL_MAX        32768        
#define LLDP_MSG_TX_HOLD_MIN            2
#define LLDP_MSG_TX_HOLD_MAX            10
#define LLDP_REINIT_DELAY_MIN           1
#define LLDP_REINIT_DELAY_MAX           10
#define LLDP_TX_DELAY_MIN               1
#define LLDP_TX_DELAY_MAX               8192
#define LLDP_OID_LEN_MAX                128

#define INET_NTOP_BUFSIZ  51
typedef enum
{
    LLDP_IF_DISABLE = 0,
    LLDP_IF_TXRX,
    LLDP_IF_TXONLY,
    LLDP_IF_RXONLY,
    LLDP_IF_MODE_MAX
} lldp_if_mode_t;

typedef enum
{
    LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV4              = 1, 
    LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV6              = 2,
    LLDP_MANAGEMENT_ADDRESS_SUBTYPE_ALL802            = 6,
    LLDP_MANAGEMENT_ADDRESS_SUBTYPE_MAX
} lldp_mgmt_addr_sub_type_t;

typedef enum
{
    MED_TYPE_INVENTORY_HARDWARE_REVISION              = 1,
    MED_TYPE_INVENTORY_FIRMWARE_REVISION              = 2,
    MED_TYPE_INVENTORY_SOFTWARE_REVISION              = 3,
    MED_TYPE_INVENTORY_SERIAL_NUMBER                  = 4,
    MED_TYPE_INVENTORY_MANUFACTURER_NAME              = 5,
    MED_TYPE_INVENTORY_MODEL_NAME                     = 6,
    MED_TYPE_INVENTORY_ASSET_ID                       = 7,
    MED_TYPE_INVENTORY_MAX
} lldp_med_inventoty_t;

#define LLDP_MAX_CIVIC_ADDR_NUM     10
#define LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE 2
#define LLDP_MED_LCI_CIVIC_VALUE_SIZE 232
#define LLDP_MED_LCI_CIVIC_CA 5

#define LLDP_MED_LCI_ELIN_MAX_SIZE  25
#define LLDP_MED_LCI_ELIN_MIN_SIZE  10
#define LLDP_MED_LCI_COORDINATE_SIZE 16

#define MED_LCI_FORMAT_COORDINATE       1
#define MED_LCI_FORMAT_CIVIC            2
#define MED_LCI_FORMAT_ECS_ELIN         3

typedef struct
{
    uint8   len;
    uint8   what;
    uint8   ca_num;
    char    countrycode[LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE + 1];
    char    ca_value[LLDP_MAX_CIVIC_ADDR_NUM][LLDP_MED_LCI_CIVIC_VALUE_SIZE + 1];
    uint8   ca_type[LLDP_MAX_CIVIC_ADDR_NUM];
    uint8   ca_length[LLDP_MAX_CIVIC_ADDR_NUM];
} lldp_civic_address_t;

typedef struct
{
    uint32      type;   /* lldp_chassis_id_sub_type_t */
    uint32      len;
    uint8       id[CHASSIS_ID_TLV_MAX_SIZE];
} lldp_chassis_id_t;

typedef struct
{
    uint32      type;   /* lldp_port_id_sub_type_t */
    uint32      len;
    uint8       id[PORT_ID_TLV_MAX_SIZE];
} lldp_port_id_t;

typedef struct
{
    lldp_chassis_id_t   chassis_id;
    lldp_port_id_t      port_id;
} lldp_msap_id_t;



/* refer to 8.2.1 defaultDS data set member specifications */
/* refer to 8.3.2 transparentClockDefaultDS data set member specifications */
struct ptp_default_ds_s
{
    /* Static members */
    glb_ptp_clock_id_t clock_identity;
    uint32 two_step_flag;
    uint32 number_ports;

    /* Dynamic members */
    glb_ptp_clock_quality_t clock_quality;

    /* Configurable members */
    uint8 priority1;
    uint8 priority2;
    uint8 domain_number;
    uint8 slave_only;

    /* TC Configurable members */
    uint8 delay_mechanism;
    uint8 primary_domain;
};
typedef struct ptp_default_ds_s ptp_default_t;

/* refer to 8.2.2 currentDS data set member specifications */
struct ptp_current_ds_s
{
    /* Dynamic members */
    int64 offset_from_master;
    int64 mean_path_delay;
    uint16 steps_removed;
};
typedef struct ptp_current_ds_s ptp_current_t;

/* refer to 8.2.3 parentDS data set member specifications */
struct ptp_parent_ds_s
{
    /* Dynamic members */
    glb_ptp_port_id_t parent_port_identity;
    uint16 parent_stats;
    uint16 observed_parent_offset_scaled_log_variance;
    int32 observed_parent_clock_phase_change_rate;
    glb_ptp_clock_id_t grandmaster_identity;
    glb_ptp_clock_quality_t grandmaster_clock_quality;
    uint8 grandmaster_priority1;
    uint8 grandmaster_priority2;
};
typedef struct ptp_parent_ds_s ptp_parent_t;

/* refer to 8.2.4 timePropertiesDS data set member specifications */
struct ptp_time_properties_ds_s
{
    /* Dynamic members */
    int16 current_utc_offset;
    uint8 current_utc_offset_valid;
    uint8 leap59;
    uint8 leap61;
    uint8 time_traceable;
    uint8 frequency_traceable;
    uint8 ptp_timescale;
    uint8 time_source;
};
typedef struct ptp_time_properties_ds_s ptp_time_properties_t;

/* refer to 8.2.5 portDS data set member specifications */
struct ptp_port_ds_s
{
    /* Static members */
#if 0 /*the clockid of port_identity is from default ds*/
    glb_ptp_port_id_t port_identity; /* TC shared Static */
#else /*so we only store port_num of port_identity*/
    uint16 port_identity_port_num;
#endif
    /* Dynamic members */
    uint8 port_state;
    int8 log_min_delay_req_interval;
    int64 peer_mean_path_delay; /* TC shared Dynamic */

    /* Configurable members */
    int8 log_announce_interval;
    uint8 announce_receipt_timeout;
    int8 log_sync_interval;
    uint8 delay_mechanism;//ptp delay-mechanism
    int8 log_min_pdelay_req_interval; /* TC shared Dynamic */
    uint8 version_number;

    int8 faulty_flag; /* TC only Dynamic */
};
typedef struct ptp_port_ds_s ptp_port_t;

struct ptp_dump_flag_s
{
    uint32 ptp_dump_flag_tx      :1;
    uint32 ptp_dump_flag_rx      :1;
    uint32 ptp_dump_flag_raw     :1;
    uint32 ptp_dump_flag_dec     :1;
    uint32                       :0;
};
typedef struct ptp_dump_flag_s ptp_dump_flag_t;

struct ptp_vlan_info_s
{
    uint32 vid  :12;
    uint32 cos  :4;
    uint32      :0;
};
typedef struct ptp_vlan_info_s ptp_vlan_info_t;

struct ptp_global_c_s
{
    glb_ptp_timestamp_t last_sync_tx_ts;
    glb_ptp_timestamp_t last_sync_rx_ts;
    int64 last_master_to_slave_delay;
    int64 last_correction_field_with_mean_path_delay;

    int32 servo_nns_cache[PTP_SERVO_NNS_CACHE_MAX];
    int32 mean_nns;
    uint8 servo_offset_cnt;
    uint8 servo_nns_pos;
    uint8 servo_nns_cnt;
    uint8 servo_nns_lock_cnt;

    uint32 servo_nns_ready              :1;
    uint32 servo_nns_lock               :1;
    uint32 servo_nns_first_2_ignored    :1;
    uint32                              :0;
} ;
typedef struct ptp_global_c_s ptp_global_c_t;


/* refer to 13.5.1 General Announce message specifications Table 25 */
struct ptp_msg_announce_s
{
    glb_ptp_timestamp_t origin_timestamp;
    int16 current_utc_offset;
    uint8 grandmaster_priority1;
    uint8 grandmaster_priority2;
    glb_ptp_clock_quality_t grandmaster_clock_quality;
    glb_ptp_clock_id_t grandmaster_identity;
    uint16 steps_removed;
    uint8 time_source;
};
typedef struct ptp_msg_announce_s ptp_msg_announce_t;



struct ptp_pdu_stats_s
{
    uint32 announce;
    uint32 sync;
    uint32 delay_req;
    uint32 delay_resp;
    uint32 follow_up;
    uint32 pdelay_req;
    uint32 pdelay_resp;
    uint32 pdelay_resp_follow_up;
    uint32 unknown;
};
typedef struct ptp_pdu_stats_s ptp_pdu_stats_t;

typedef ptp_pdu_stats_t ptp_port_stats_t[PTP_STATS_MAX];


struct ptp_port_p_s
{
    /*used to handle message*/
    uint16 sent_announce_sequence_id;
    uint16 sent_sync_sequence_id;
    uint16 sent_delay_req_sequence_id;
    uint16 sent_pdelay_req_sequence_id;

    /*about sync packet send and rcv record*/
    uint16 recv_sync_sequence_id;
    glb_ptp_timestamp_t sync_send_time;
    glb_ptp_timestamp_t sync_receive_time;
    glb_ptp_timeinterval_t last_sync_correction_field;
    int64 last_delay_m_s;//the calc delay last time
    uint32 waiting_for_follow;

    /*about delay_req packet send and rcv record*/
    glb_ptp_timestamp_t delay_req_send_time;
    glb_ptp_timestamp_t delay_req_receive_time;
    glb_ptp_timeinterval_t last_delay_resp_correction_field;

    /*about pdelay_req pdelay_resp packet send and rcv record*/
    glb_ptp_timestamp_t pdelay_req_send_time;
    glb_ptp_timestamp_t pdelay_resp_send_time;
    glb_ptp_timestamp_t pdelay_req_receive_time;
    glb_ptp_timestamp_t pdelay_resp_receive_time;
    glb_ptp_timeinterval_t last_pdelay_resp_correction_field;
    uint32 waiting_for_pdelay_resp_follow;

    int64 delay_m_s;
    int64 delay_s_m;
    int64 pdelay_m_s;
    int64 pdelay_s_m;
    int64 offset_adjusted;
};
typedef struct ptp_port_p_s ptp_port_p_t;

struct ptp_port_c_s
{
    int32 path_delay_cache[PTP_SERVO_PATH_DELAY_CACHE_MAX];/*record last serveral times of path delay, for calc even path delay*/
    uint32 servo_pdc_pos                        :8;/*tmp variaty, position of path_delay_cache*/
    uint32 servo_pdc_cnt                        :8;
    uint32 servo_pdc_rty                        :8;
    uint32                                      :0;
};
typedef struct ptp_port_c_s ptp_port_c_t;


typedef struct
{
    glb_ptp_port_id_t foreign_master_port_identity;
    uint32               ifindex;
} ptp_foreign_master_t;

struct ptp_residence_time_cache_s
{
    glb_ptp_port_id_t    source_port_id;      /*source port id*/
    uint32               ifindex_belong_to;
    uint8 is_sync_ingress;/*true: sync_ingress; false: delay_req_ingress. replace p_tree_belong_to*/
};
typedef struct ptp_residence_time_cache_s ptp_residence_time_cache_t;


#if _GLB_HOST_IS_LE
struct ptp_common_message_header_s
{
    uint32 message_type                                     :4;
    uint32 transport_specific                               :4;
    uint32 version_ptp                                      :4;
    uint32                                                  :4;
    uint32 message_length                                   :16;

    uint32 domain_number                                    :8;
    uint32                                                  :8;

    uint32 flag_field_0_alternate_master_flag               :1;
    uint32 flag_field_0_two_step_flag                       :1;
    uint32 flag_field_0_unicast_flag                        :1;
    uint32                                                  :2;
    uint32 flag_field_0_ptp_profile_specific_1              :1;
    uint32 flag_field_0_ptp_profile_specific_2              :1;
    uint32                                                  :1;

    uint32 flag_field_1_leap61                              :1;
    uint32 flag_field_1_leap59                              :1;
    uint32 flag_field_1_current_utc_offset_valid            :1;
    uint32 flag_field_1_ptp_timescale                       :1;
    uint32 flag_field_1_time_traceable                      :1;
    uint32 flag_field_1_frequency_traceable                 :1;
    uint32                                                  :2;

    uint32 correction_field_msb                             :32;

    uint32 correction_field_lsb                             :32;

    uint32                                                  :32;

    glb_ptp_clock_id_t source_port_identity_clock_id;

    uint32 source_port_identity_port_num                    :16;
    uint32 sequence_id                                      :16;

    uint32 control_field                                    :8;
    uint32 log_message_interval                             :8;
    uint32                                                  :16;
};
#else
struct ptp_common_message_header_s
{
    uint32 transport_specific                               :4;
    uint32 message_type                                     :4;
    uint32                                                  :4;/*reserved*/
    uint32 version_ptp                                      :4;
    uint32 message_length                                   :16;

    uint32 domain_number                                    :8;
    uint32                                                  :8;/*reserved*/

    uint32                                                  :1;/*reserved*/
    uint32 flag_field_0_ptp_profile_specific_2              :1;
    uint32 flag_field_0_ptp_profile_specific_1              :1;
    uint32                                                  :2;/*reserved*/
    uint32 flag_field_0_unicast_flag                        :1;
    uint32 flag_field_0_two_step_flag                       :1;
    uint32 flag_field_0_alternate_master_flag               :1;

    uint32                                                  :2;/*reserved*/
    uint32 flag_field_1_frequency_traceable                 :1;
    uint32 flag_field_1_time_traceable                      :1;
    uint32 flag_field_1_ptp_timescale                       :1;
    uint32 flag_field_1_current_utc_offset_valid            :1;
    uint32 flag_field_1_leap59                              :1;
    uint32 flag_field_1_leap61                              :1;

    uint32 correction_field_msb                             :32;

    uint32 correction_field_lsb                             :32;

    uint32                                                  :32;/*reserved*/

    glb_ptp_clock_id_t source_port_identity_clock_id;/*8 bytes*/

    uint32 source_port_identity_port_num                    :16;
    uint32 sequence_id                                      :16;

    uint32 control_field                                    :8;
    uint32 log_message_interval                             :8;
    uint32                                                  :16;
};
#endif
typedef struct ptp_common_message_header_s ptp_common_message_header_t;


typedef int32
(*ptp_msg_handle_proc_t)(void *p_port, ptp_sk_buff_t *skb, glb_ptp_timestamp_t *rx_ts);

typedef void
(*ptp_msg_issue_proc_t)(void *p_port);

typedef int32
(*ptp_msg_forward_proc_t)(void *p_port, ptp_sk_buff_t *skb, glb_ptp_timestamp_t *rx_ts);

typedef int32
(*ptp_msg_forward_test_proc_t)(char *str_to_printf);

struct ptp_msg_proc_ops_s
{
    ptp_msg_handle_proc_t handle_proc[PTP_MSG_TYPE_MAX];
    ptp_msg_issue_proc_t issue_proc[PTP_MSG_TYPE_MAX];
    ptp_msg_forward_proc_t forward_proc[PTP_MSG_TYPE_MAX];
    ptp_msg_forward_test_proc_t forward_test_proc[PTP_MSG_TYPE_MAX];
};
typedef struct ptp_msg_proc_ops_s ptp_msg_proc_ops_t;

struct kernel_if_ipv6_addr_s
{
    addr_ipv6_t ip_addr_v6[RT_IF_ADDRESS_V6_MAX];
    uint32 masklen_v6[RT_IF_ADDRESS_V6_MAX];
};
typedef struct kernel_if_ipv6_addr_s kernel_if_ipv6_addr_t;

typedef struct
{
    uint8               mac_addr[MAC_ADDR_LEN];
    uint16              rx_ttl;
    char                ifname[IFNAME_SIZE];
    lldp_msap_id_t      key;
} lldpdu_t;

typedef struct
{
    uint16           domain_id;
    uint16           ring_id;
} erps_ring_key_t;

typedef struct
{
    uint32           instance_id;
    uint32           ifindex;
} fea_g8032_ring_key_t;

typedef enum
{
    IGMP_MROUTE_STATIC = 0,
    IGMP_MROUTE_DYNAMIC,
    IGMP_MROUTE_TYPE_MAX
} igmp_mrouter_type_t;

typedef struct
{
    vid_t               vid;
    addr_ipv4_t         ip;
    uint32              ifindex;    /* interface should be key for each has different expire time */
} igsp_group_key_t;

typedef struct
{
    vid_t               vrf_id;
    addr_ipv4_t         source;
    addr_ipv4_t         group;
} mrt_key_t;

typedef struct
{
    addr_t               addr;
    uint16               sin_port;
} dot1x_radius_key_t;

typedef struct
{
    uint32               ifindex;
    mac_addr_t           mac;
} dot1x_mac_key_t;

typedef struct
{
    addr_t               addr;
    uint16               udp_port;
} sflow_collector_key_t;

typedef enum
{
    NS_ROUTE_FLAG_INVALID = 0,
    NS_ROUTE_FLAG_OPENFLOW_CONTROLLER = 1,
    NS_ROUTE_FLAG_NTP_SERVER          = 2,
    NS_ROUTE_FLAG_NTP_PEER            = 3,
    NS_ROUTE_FLAG_LOGGING_SERVER      = 4,
    NS_ROUTE_FLAG_SNMP_SERVER         = 5,
    NS_ROUTE_FLAG_OPENFLOW_MANAGER    = 6,
    NS_ROUTE_FLAG_RADIUS_SERVER       = 7,
    NS_ROUTE_FLAG_TACACS_SERVER       = 8,
    NS_ROUTE_FLAG_SFLOW_SERVER        = 9,
    NS_ROUTE_FLAG_MAX
} ns_route_flag_t;

#define NS_ROUTE_FLAG_TO_STRING(flag) (flag == NS_ROUTE_FLAG_OPENFLOW_CONTROLLER ? "openflow controller" : \
                                       flag == NS_ROUTE_FLAG_NTP_SERVER          ? "ntp server"          : \
                                       flag == NS_ROUTE_FLAG_NTP_PEER            ? "ntp peer"            : \
                                       flag == NS_ROUTE_FLAG_LOGGING_SERVER      ? "logging server"      : \
                                       flag == NS_ROUTE_FLAG_SNMP_SERVER         ? "snmp server"         : \
                                       flag == NS_ROUTE_FLAG_OPENFLOW_MANAGER    ? "openflow manager"    : \
                                       flag == NS_ROUTE_FLAG_RADIUS_SERVER       ? "radius server"       : \
                                       flag == NS_ROUTE_FLAG_TACACS_SERVER       ? "tacacs server"       : \
                                       flag == NS_ROUTE_FLAG_SFLOW_SERVER        ? "sflow server"        : \
                                       "invalid flag")

typedef enum
{
    NS_ROUTE_TCP = 0,
    NS_ROUTE_UDP,
    NS_ROUTE_SSL,
    NS_ROUTE_MAX
} ns_route_proto_t;

#define NS_ROUTE_PROTO_TO_STRING(proto) (proto == NS_ROUTE_TCP   ? "tcp" : \
                                         proto == NS_ROUTE_UDP   ? "udp" : \
                                         proto == NS_ROUTE_SSL   ? "ssl" : \
                                         "invalid flag")

typedef struct
{
    addr_t           ip;     /* remote ip address */
    uint16           port;   /* remote port number */
    ns_route_proto_t proto;  /* tcp or udp */
} ns_route_key_t;


typedef enum
{
    OPENFLOW_MANAGER_PROTO_TCP = 0,
    OPENFLOW_MANAGER_PROTO_MAX
} openflow_manager_proto_t;

typedef enum
{
    OPENFLOW_MANAGER_MODE_ACTIVE = 0,
    OPENFLOW_MANAGER_MODE_PASSIVE,
    OPENFLOW_MANAGER_MODE_MAX
} openflow_manager_mode_t;

typedef struct
{
    addr_ipv4_t              ip;
    uint16                   port;
    openflow_manager_proto_t proto;
} openflow_manager_key_t;
    
/* start: TBL_INBAND_SNAT */
typedef enum
{
    INBAND_SNAT_TYPE_INVALID = 0,
    INBAND_SNAT_TYPE_OPENFLOW_CONTROLLER = 1,
    INBAND_SNAT_TYPE_NTP_SERVER          = 2,
    INBAND_SNAT_TYPE_NTP_PEER            = 3,
    INBAND_SNAT_TYPE_LOGGING_SERVER      = 4,
    INBAND_SNAT_TYPE_SNMP_SERVER         = 5,
    INBAND_SNAT_TYPE_OPENFLOW_MANAGER    = 6,
    INBAND_SNAT_TYPE_RADIUS_SERVER       = 7,
    INBAND_SNAT_TYPE_TACACS_SERVER       = 8,
    INBAND_SNAT_TYPE_SFLOW_SERVER        = 9,
    INBAND_SNAT_TYPE_TFTP_FTP_SCP_SERVER        = 10,    /* modified by liwh for bug 53343, 2019-08-28 */
    INBAND_SNAT_TYPE_MAX
} inband_snat_type_t;

#define INBAND_SNAT_TYPE_TO_STRING(type) (type == INBAND_SNAT_TYPE_OPENFLOW_CONTROLLER ? "openflow controller" : \
                                          type == INBAND_SNAT_TYPE_NTP_SERVER          ? "ntp server"          : \
                                          type == INBAND_SNAT_TYPE_NTP_PEER            ? "ntp peer"            : \
                                          type == INBAND_SNAT_TYPE_LOGGING_SERVER      ? "logging server"      : \
                                          type == INBAND_SNAT_TYPE_SNMP_SERVER         ? "snmp server"         : \
                                          type == INBAND_SNAT_TYPE_OPENFLOW_MANAGER    ? "openflow manager"    : \
                                          type == INBAND_SNAT_TYPE_RADIUS_SERVER       ? "radius server"       : \
                                          type == INBAND_SNAT_TYPE_TACACS_SERVER       ? "tacacs server"       : \
                                          type == INBAND_SNAT_TYPE_SFLOW_SERVER        ? "sflow server"        : \
                                          type == INBAND_SNAT_TYPE_TFTP_FTP_SCP_SERVER   ? "tftp or ftp or scp server"        : \
                                          "invalid type")

typedef enum
{
    INBAND_IP_PROTO_TCP = 0,
    INBAND_IP_PROTO_UDP,
    INBAND_IP_PROTO_MAX
} inband_snat_proto_t;

typedef struct
{
    addr_ipv4_t         ip;     /* remote ip address */
    uint16              port;   /* remote port number */
    inband_snat_proto_t proto;  /* tcp or udp */
} inband_snat_key_t;

/* end: TBL_INBAND_SNAT */

/*add by zhw for mc*/
#define PIM_SM_IP_LEN 20
typedef struct 
{
    char    ip[PIM_SM_IP_LEN];
    uint32  masklen;
} pim_sm_gprefix_t;                                                                                                                                                                                         
 
typedef struct 
{
    char    ip[PIM_SM_IP_LEN];
    char    group[PIM_SM_IP_LEN];
    uint32  masklen;
} pim_static_rp_addr_t;
/*end add zhw*/

typedef struct
{
    uint32     entry_id;
    uint32     packet_index;
} ipsla_packet_stats_key_t;

typedef struct
{
    uint32     entry_id;
    uint32     test_index;
} ipsla_test_stats_key_t;

typedef struct
{
    prefix_t               addr;
    uint32               ifindex;
} nd_prefix_key_t;

typedef struct
{
    uint32       time;
    uint32       seq;
} cfm_time_key_t;

typedef struct
{
    uint8 ip_protocol;
    uint8 resv;
    uint16 vrf_id;                /* not support for the moment*/

    addr_t inside_local_ip;
    addr_t inside_global_ip;
    addr_t outside_local_ip;
    addr_t outside_global_ip;

    union 
    {
        struct 
        {
            uint16 inside_local;
            uint16 inside_global;
            uint16 outside_local;
            uint16 outside_global;
        }l4_port;
        struct 
        {
            uint8 type;
            uint8 code;
            uint16 ID;
        }icmp;
    }proto_info;
} nat_session_key_t;


typedef struct
{
    uint32               flags;               /* only support inside snat for the moment. not support inside dnat, outside snat, outside dnat. flags GLB_NAT_RULE_FLAG_E*/
    addr_t               in_local_ip;              /* inside local IP*/
    addr_t               in_glb_ip;              /* inside global IP*/
    uint8                protocol;            /* 1~255, ICMP, TCP, UDP and so on*/
    uint8                resv;
    uint16               in_local_port;           /* inside local port*/
    uint16               in_glb_port;            /* inside global port*/
    uint16               in_glb_port_hi;         /* inside global port high*/
    char                 oif[IFNAME_SIZE+1];
    char                 acl[ACL_NAME_FULL_NAME_SIZE+1]; /* acl as snat filter*/
    char                 pool[GLB_NAT_POOL_NAME_LEN+1]; /* address pool*/
} nat_rule_key_t;

typedef struct
{
    uint32 ifindex;
    addr_t inside_global_ip;
} nat_proxy_arp_key_t;

typedef struct
{
    uint16      vlan_id;
    uint8        name[CFM_MD_NAME_MAX_LENGTH_8021AG+1];    /* md name */
} cfm_ma_key_t;

typedef struct
{
    uint16      level;
    uint16      vlan_id;
    uint32      ifindex;    /* md name */
} cfm_lmep_key_t;

typedef struct
{
    uint16      level;
    uint16      vlan_id;
    uint32      rmep_id;    /* md name */
} cfm_rmep_key_t;

#endif /* !__CDB_CONST_H__ */
