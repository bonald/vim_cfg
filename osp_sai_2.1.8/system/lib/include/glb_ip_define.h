#ifndef __GLB_IP_DEFINE_H__
#define __GLB_IP_DEFINE_H__

#define MY_PRINTF(fmt, args...)\
do {\
    if (0)\
    sal_printf("%s "fmt"\n", __func__, ##args);\
}while(0)


#define ROUTE_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_ROUTE, E_DEBUG, "%s %d "fmt, __func__, __LINE__, ##args);

#define ROUTE_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (RT_DBG_IS_ON(module, typeenum)) \
        ROUTE_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define DEBUG_ROUTE_IPV4(fmt, args...) \
do { \
    ROUTE_LOG_DEBUG(route, ROUTE_IPV4, fmt, ##args); \
} while(0)

#define DEBUG_ROUTE_IPV6(fmt, args...) \
do { \
    ROUTE_LOG_DEBUG(route, ROUTE_IPV6, fmt, ##args); \
} while(0)

#define DEBUG_ROUTE_NETLINK(fmt, args...) \
do { \
    ROUTE_LOG_DEBUG(route, ROUTE_NETLINK, fmt, ##args); \
} while(0)

#ifndef IN6_IS_ADDR_6TO4
/*% Is IPv6 address 6to4? */
#define IN6_IS_ADDR_6TO4(a) \
    (((a)->s6_addr[0] == 0x20) && ((a)->s6_addr[1] == 0x02))
#endif

//#define USE_FPM_IPUC

typedef enum { AFI_IP = 1, AFI_IP6 = 2, AFI_L2VPN = 4, AFI_MAX = 5 } afi_t;

#define GLB_SSH_PORT_DEF        22
#define GLB_TELNET_PORT_DEF     23
#define GLB_HTTP_PORT_DEF       80
#define GLB_HTTP_TIMEOUT_DEF    600
#define GLB_HTTPS_PORT_DEF      443
#define GLB_HTTP_PORT_INTERNAL  1024

typedef enum
{
    GLB_SERVICE_TYPE_TELNET = 0x01,
    GLB_SERVICE_TYPE_SSH    = 0x02,
    GLB_SERVICE_TYPE_HTTP   = 0x04,
    GLB_SERVICE_TYPE_MAX
} glb_service_type_t;

enum nexthop_types_t {
	NEXTHOP_TYPE_IFINDEX = 1,  /* Directly connected.  */
	NEXTHOP_TYPE_IPV4,	 /* IPv4 nexthop.  */
	NEXTHOP_TYPE_IPV4_IFINDEX, /* IPv4 nexthop with ifindex.  */
	NEXTHOP_TYPE_IPV6,	 /* IPv6 nexthop.  */
	NEXTHOP_TYPE_IPV6_IFINDEX, /* IPv6 nexthop with ifindex.  */
	NEXTHOP_TYPE_BLACKHOLE,    /* Null0 nexthop.  */
};

typedef enum
{
    GLB_RPCAPI_AUTH_NONE = 0,
    GLB_RPCAPI_AUTH_BASIC = 1,
} glb_rpcapi_auth_mode_t;

typedef uint32 in4_addr;
typedef struct sal_in4_addr addr_ipv4_t;
typedef struct in6_addr addr_ipv6_t;

typedef struct
{
    uint8       family;
    uint8       prefixlen;
    uint16      pad;
    addr_ipv4_t prefix;
} prefix_ipv4_t;

typedef union
{
    uint8       prefix;     /* for get pointer */
    addr_ipv4_t prefix4;
    addr_ipv6_t prefix6;
    uint8       val[16];
} addr_u;

typedef struct
{
    uint8       family;
    addr_u      u;
} addr_t;

typedef struct
{
    uint8       family;
    uint8       prefixlen;
    uint16      pad;
    addr_u      u;
} prefix_t;

#define GLB_IPV4_MAX_PREFIXLEN 32
#define GLB_IPV4_STR_LEN 16   
#define GLB_IPV6_UINT8_LEN 16   
#define GLB_IPV6_STR_LEN 40   

#define GLB_MAX_NH_ECMP_NUM   8
#define GLB_MAX_IPV4_MASK_LEN 32
#define GLB_MAX_IPV6_MASK_LEN 128
/*added by ychen in 2012-04-05 for ecmp*/
#define GLB_MAX_ECMP_GROUPS   256/*(2 ^GLB_MAX_NH_ECMP_NUM)*/

#define GLB_INVALID_IPV4_ADDR 0x00000000

#define GLB_ARP_DEFAULT_RETRANS_TIME 1
#define GLB_ARP_DEFAULT_TIMEOUT      3600
#define GLB_IPV6_ADDR_IS_LINKLOCAL(ip) \
    (((ip[0]) & 0xFFC00000) == 0xFE800000)

/* support vrrp modified by liwh for bug 45215, 2017-09-17 */
#define     GLB_VRRP_VERSION_V2            2
#define     GLB_VRRP_VERSION_V3            3

#define     GLB_VRRP_ADVT_INT_DFLT         1

#define     GLB_VRRP_SECONDS_MODE          0
#define     GLB_VRRP_MILLI_SECONDS_MODE    1

#define     GLB_VRRP_PREEMPT_DELAY_DFLT    0

#define     GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY  100

#define     GLB_VRRP_ADVT_MSEC_MIN         100

#define     GLB_VRRP_MAC_VALID             0
#define     GLB_VRRP_MAC_INVALID           1
#define     GLB_VRRP_MAC_INTERFACE         2

/*reference to SYS_ROUTER_MAC_ENTRY_NUM, reserve 1 for global*/
#define     GLB_VRRP_MAX_SESSION           31 
/* liwh end */

#define   GLB_IPSLA_PING_TTL_DEFAULT             64
#define   GLB_IPSLA_PING_TOS_DEFAULT             0
#define   GLB_IPSLA_PING_FAIL_PERCENT_DEFAULT    100
#define   GLB_IPSLA_PING_PROBE_COUNT_DEFAULT     3
#define   GLB_IPSLA_FAIL_PERCENT_DEFAULT         100
#define   GLB_IPSLA_DATA_PATTERN_DEFAULT         (0xABCDABCD)
#define   GLB_IPSLA_DATA_SIZE_DEFAULT            28
#define   GLB_IPSLA_RESULT_PROBE_NUM_DEFAULT     50
#define   GLB_IPSLA_RESULT_TEST_NUM_DEFAULT      5
#define   GLB_IPSLA_FREQUENCY_DEFAULT    60
#define   GLB_IPSLA_TIMEOUT_DEFAULT      5
#define   GLB_IPSLA_THRESHOLD_DEFAULT    5000
#define   GLB_IPSLA_INTERVAL_DEFAULT     6

#define   GLB_TRACK_INTERFACE_LINKSTATE         1
#define   GLB_TRACK_RTR_REACHABILITY            5
#define   GLB_TRACK_RTR_STATE                   6

#define GLB_ND_FLAG_NONE                0 
#define GLB_ND_FLAG_PERMANENT      (1 << 1)
#define GLB_ND_FLAG_DYNAMIC          (1 << 2)   /* only used for neigh match */
#define GLB_ND_FLAG_ACTIVE             (1 << 3)  /* in kernel & HW */
#define GLB_ND_FLAG_ROUTER            (1 << 4) 
#define GLB_ND_FLAG_FDB_AGE           (1 << 5)

#define GLB_IPV6_MAX_BYTELEN    16

#define GLB_IPV6_MIN_MTU    1280

#define IPV6_PREFIX_IS_LINK_LOCAL(p) \
    (((p)->prefixlen >= 10) && (((p)->u.val[0] & 0xff) == 0xFE) && (((p)->u.val[1] & 0xC0) == 0x80))

#define IPV6_PREFIX_IS_MULTICAST(p) \
    (((p)->prefixlen >= 8) && ((p)->u.val[0] == 0xFF))

/* modified by liwh for bug 51257, 2019-03-18 */
#define GLB_OSPF_COST_TIME       30
/* liwh end */

/* modified by liwh for bug 51014, 2019-03-28 */
#define GLB_OSPF_REDISTRIBUTE_CONNECTED_TIME       30
#define GLB_OSPF_REDISTRIBUTE_STATIC_TIME               30
/* liwh end */

enum glb_ip_ver_e
{
    GLB_IP_VER_4,
    GLB_IP_VER_6,
    GLB_IP_VER_MAX,
};
typedef enum glb_ip_ver_e glb_ip_ver_t;

typedef enum
{
    GLB_IPUC_REMOTE,        /* Nexthop is remote */    
    GLB_IPUC_LOCAL,         /* Nexthop is directly attached */
    GLB_IPUC_BLACKHOLE,     /* Drop */
    GLB_IPUC_FTN,           /* Nexthop is a FTN */
    GLB_IPUC_TYPE_MAX
} glb_ipuc_nexthop_type_t;

enum glb_ipuc_flag_e
{
    GLB_IPUC_FLAG_RPF_CHECK     = 0x01,      /**< This route will do RPF check */
    GLB_IPUC_FLAG_TTL_CHECK     = 0x02,      /**< This route will do ttl check */
    GLB_IPUC_FLAG_ICMP_CHECK    = 0x04,      /**< This route will do icmp redirect check */
    GLB_IPUC_FLAG_CPU           = 0x08,      /**< Packets hitting this route will be copied to CPU */
    GLB_IPUC_FLAG_VRF           = 0x10,      /**< Packets hitting this route will be copied to CPU */
    GLB_IPUC_FLAG_NEIGHBOR      = 0x20,      /**< This is a ARP entry, only set when mask length is 32 on IPv4 or 128 on IPv6 */
    GLB_IPUC_FLAG_CONNECT       = 0x40,      /**< This is a CONNECT entry */
    GLB_IPUC_FLAG_SELF_ADDRESS  = 0x80,    /**< [GB]Indicate it is a PTP or OAM entry */
    GLB_MAX_IPUC_FLAG           = 0xFF
};
typedef enum glb_ipuc_flag_e glb_ipuc_flag_t;

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
    GLB_ECMP_HASH_KEY_INNER_MACSA     = 1 << 9,
    GLB_ECMP_HASH_KEY_INNER_MACDA     = 1 << 10,
    GLB_ECMP_HASH_KEY_INNER_IPSA     = 1 << 11,
    GLB_ECMP_HASH_KEY_INNER_IPDA     = 1 << 12,
    GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL     = 1 << 13,
    GLB_ECMP_HASH_KEY_INNER_SRC_PORT     = 1 << 14,
    GLB_ECMP_HASH_KEY_INNER_DST_PORT     = 1 << 15,

    GLB_ECMP_HASH_KEY_DEFAULT = GLB_ECMP_HASH_KEY_IPSA      |
                                 GLB_ECMP_HASH_KEY_IPDA      |
                                 GLB_ECMP_HASH_KEY_IPPRO     |
                                 GLB_ECMP_HASH_KEY_SRC_PORT  |
                                 GLB_ECMP_HASH_KEY_DST_PORT,

    GLB_ECMP_HASH_KEY_ALL      = GLB_ECMP_HASH_KEY_IPSA      |
                                 GLB_ECMP_HASH_KEY_IPDA      |
                                 GLB_ECMP_HASH_KEY_IPPRO     |
                                 GLB_ECMP_HASH_KEY_SRC_PORT  |
                                 GLB_ECMP_HASH_KEY_DST_PORT
#if defined GOLDENGATE || defined (DUET2)
                               | GLB_ECMP_HASH_KEY_MACSA     |
                                 GLB_ECMP_HASH_KEY_MACDA     |
                                 GLB_ECMP_HASH_KEY_ETHERTYPE
#endif
};
typedef enum glb_parser_ecmp_hash_key_e glb_parser_ecmp_hash_key_e;

#define   GLB_MAX_ECMP_ROUND_ROBIN   16

typedef enum glb_ecmp_hash_algo_e
{
    GLB_ECMP_HASH_ALGO_XOR,     /*default type*/
    GLB_ECMP_HASH_ALGO_CRC
}glb_ecmp_hash_algo_t;

enum glb_ecmp_load_balance_e
{
   GLB_ECMP_STATIC_LOAD_BALANCE,
   GLB_ECMP_DYNAMIC_LOAD_BALANCE,
   GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE,
   GLB_ECMP_SELF_HEALING_LOAD_BALANCE,
   GLB_ECMP_XERSPAN_LOAD_BALANCE  
};
typedef enum glb_ecmp_load_balance_e glb_ecmp_load_balance_t;

/*glb_ecmp_load_balance_mode_e need to synchronize with ctc_nh_ecmp_dlb_mode_s in sdk*/
enum glb_ecmp_load_balance_mode_e
{
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL = 0x1,/*ECMP dynamic load balancing normal mode:if inactivity duration
                                                        lapsed, use optimal member, else use assigned member.  This is default mode*/
    
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_FIRST = 0x2,/*ECMP dynamic load balancing first mode: only new flow
                                                        use optimal member for the first time, than use assigned member */
   
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_PACKET = 0x4, /*ECMP dynamic load balancing packet mode: use optimal member
                                                        every 1024 packets */
   
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT = 0x8,/*Only elephant(big) flow do ECMP dynamic load blance,
                                                        can combine with up flags */
   
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP = 0x10,/**< [GG] Only TCP flow do ECMP dynamic load blance */
   GLB_ECMP_DYNAMIC_LOAD_BALANCE_MAX 
};

typedef enum
{
    GLB_VTY_MODE_EXEC = 0,
    GLB_VTY_MODE_CONFIG,
    GLB_VTY_MODE_INTERFACE,
    GLB_VTY_MODE_ROUTER,
    GLB_VTY_MODE_MAX
} glb_vty_mode_t;

/* Zebra route's' types. */
#define ZEBRA_ROUTE_SYSTEM               0
#define ZEBRA_ROUTE_KERNEL               1
#define ZEBRA_ROUTE_CONNECT              2
#define ZEBRA_ROUTE_STATIC               3
#define ZEBRA_ROUTE_RIP                  4
#define ZEBRA_ROUTE_RIPNG                5
#define ZEBRA_ROUTE_OSPF                 6
#define ZEBRA_ROUTE_OSPF6                7
#define ZEBRA_ROUTE_ISIS                 8
#define ZEBRA_ROUTE_BGP                  9
#define ZEBRA_ROUTE_PIM                  10
#define ZEBRA_ROUTE_NHRP                 11
#define ZEBRA_ROUTE_HSLS                 12
#define ZEBRA_ROUTE_OLSR                 13
#define ZEBRA_ROUTE_TABLE                14
#define ZEBRA_ROUTE_LDP                  15
#define ZEBRA_ROUTE_VNC                  16
#define ZEBRA_ROUTE_VNC_DIRECT           17
#define ZEBRA_ROUTE_VNC_DIRECT_RH        18
#define ZEBRA_ROUTE_BGP_DIRECT           19
#define ZEBRA_ROUTE_BGP_DIRECT_EXT       20
#define ZEBRA_ROUTE_ALL                  21
#define ZEBRA_ROUTE_MAX                  22

/*add by zhw for mc*/
#define   GLB_IPV4_MROUTE_O_MEMBER_MAX       64
#define   GLB_IPV4_MROUTE_MAX_ENTRY_NUM      512
/*end add zhw*/
#endif /*__GLB_IP_DEFINE_H__*/

