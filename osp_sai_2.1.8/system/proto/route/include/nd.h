
#ifndef __ND_H__
#define __ND_H__

#define ND_LOG_EMERG(fmt, args...)    log_sys(M_MOD_ND, E_EMERGENCY, fmt, ##args)
#define ND_LOG_ALERT(fmt, args...)    log_sys(M_MOD_ND, E_ALERT, fmt, ##args)
#define ND_LOG_CRIT(fmt, args...)     log_sys(M_MOD_ND, E_CRITICAL, fmt, ##args)
#define ND_LOG_ERR(fmt, args...)      log_sys(M_MOD_ND, E_ERROR, fmt, ##args)
#define ND_LOG_WARN(fmt, args...)     log_sys(M_MOD_ND, E_WARNING, fmt, ##args)
#define ND_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_ND, E_NOTICE, fmt, ##args)
#define ND_LOG_INFO(fmt, args...)     log_sys(M_MOD_ND, E_INFORMATIONAL, fmt, ##args)

#define IPV6_PREFIX_IS_LOOPBACK(p) \
    (((p)->prefixlen == 128) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[0]) == 0) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[1]) == 0)\
    && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[2]) == 0) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[3]) == 1))

#define IPV6_PREFIX_IS_UNSPECIFIED(p) \
    (((p)->prefixlen == 128) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[0]) == 0) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[1]) == 0)\
    && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[2]) == 0) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[3]) == 0))

#define IPV6_PREFIX_IS_LOOPBACK_AND_UNSPECIFIED(p) \
    (((p)->prefixlen == 127) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[0]) == 0) && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[1]) == 0)\
    && (sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[2]) == 0) && ((sal_ntoh32((p)->u.prefix6.__in6_u.__u6_addr32[3]) & 0xFFFFFFFE) == 0))

#define ND_PTR_CHECK(ptr) \
    do { \
        if (NULL == ptr) \
        {\
            sal_printf("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
            return(PM_E_INVALID_PARAM); \
        }\
    }while(0)

enum nd_state
{
    NES_NONE,
    NES_INCOMPLETE,
    NES_REACHABLE,
    NES_STALE,
    NES_DELAY,
    NES_PROBE,
    NES_STATE_MAX,
};

bool nd_debug_is_on(uint32 flag);

#define ND_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (nd_debug_is_on(flag)) \
        log_sys(M_MOD_ND, E_DEBUG, fmt, ##args); \
} while (0)

#define ND_EVENT_DEBUG(fmt, args...) \
do { \
    ND_LOG_DEBUG(RTDBG_FLAG_ND_EVENT, fmt, ##args); \
} while(0)

#define ND_PKT_DEBUG(fmt, args...) \
do { \
    ND_LOG_DEBUG(RTDBG_FLAG_ND_PACKET, fmt, ##args); \
} while(0)

#define ND_PROTO_DEBUG(fmt, args...) \
do { \
    ND_LOG_DEBUG(RTDBG_FLAG_ND_PROTO, fmt, ##args); \
} while(0)

#define ND_TIMER_DEBUG(fmt, args...) \
do { \
    ND_LOG_DEBUG(RTDBG_FLAG_ND_TIMER, fmt, ##args); \
} while(0)

#define ND_PTR_CHECK(ptr) \
    do { \
        if (NULL == ptr) \
        {\
            sal_printf("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
            return(PM_E_INVALID_PARAM); \
        }\
    }while(0)
    
/* RA */
#define MAX_INITIAL_RTR_ADVERT_INTERVAL    16000    /* milliseconds */
#define MAX_INITIAL_RTR_ADVERTISEMETNS      3          /* transmissions */
#define MAX_FINAL_RTR_ADVERTISEMENTS         3          /* transmissions */
#define MIN_DELAY_BETWEEN_RAS                      3000     /* milliseconds */
#define MAX_RA_DELAY_TIME                               500      /* milliseconds */
#define ADV_SEND_ADVERTISEMENTS                   0          /* default not to send advertisement */
#define RTR_ADV_INTERVAL_FACTOR                   0.33
#define RTR_ADV_INTERVAL_FACTOR_MAX           0.75
#define RTR_ADV_INTERVAL_THRESHOLE              9
#define UPPER_MAX_TRT_ADV_INTERVAL              1800000    /* milliseconds */
#define LOWER_MAX_TRT_ADV_INTERVAL             4               /* milliseconds */
#define UPPER_MIN_TRT_ADV_INTERVAL               1350000    /* milliseconds */
#define LOWER_MIN_TRT_ADV_INTERVAL              3              /* milliseconds */
#define LOWER_MIN_TRT_ADV_INTERVAL_MSEC    30            /* milliseconds */

#define MAX_RTR_SOLICITATION_DELAY                1000         /*milliseconds */
#define RTR_SOLICITATION_INTERVAL                   4000         /* milliseconds */
#define MAX_RTR_SOLICITATIONS                          3              /* transmissions */

/* Reachable time */
#define REACHABLE_TIME              30000    /* milliseconds */
#define ADV_REACHABLE_TIME      0    /* unspecified */
#define MIN_RANDOM_FACTOR      0.5
#define MAX_RANDOM_FACTOR      1.5

/* Retrans time */
#define ADV_RETRANS_TIMER          0    /* unspecified */
#define DELAY_TIMER                      30000    /* timer from STALE to DELAY in milliseconds */
#define MAX_RETRANS_TIMER         3600000    /* milliseconds */
#define MIN_RETRANS_TIMER          1000    /* milliseconds */
#define ROUTE_RETRANS_TIMER     3000    /* milliseconds */

/* Router lifetime */
#define MAX_ADV_ROUTER_LIFETIME    9000          /* seconds */
#define MIN_ADV_ROUTER_LIFETIME    0          /* seconds */

/* DAD */
#define MAX_MULTICAST_SOLICIT                       3         /* transmissions */
#define MAX_UNICAST_SOLICIT                           3         /* transmissions */
#define MAX_ANYCAST_DELAY_TIME                    1000    /* milliseconds */
#define MAX_NEIGHBOR_ADVERTISEMENT           3         /* transmissions */
#define MIN_DUP_ADDR_DETECT_TRANSMITS     0         /* transmissions */
#define MAX_DUP_ADDR_DETECT_TRANSMITS     600      /* transmissions */

#define DELAY_FIRST_PROBE_TIME    5000    /* milliseconds */

#define MAX_IPV6_HOP_LIMIT            255
#define MIN_IPV6_HOP_LIMIT            1

#define ND_UNSOLICITED_NA              1   /* transmissions */

/* only up to 16 prefixes can be added, must not be less than 
    MAX_INTERFACE_IPV6_ADDRESS_NUM */ 
#define MAX_ADV_PREFIX_NUM                           9
#define MAX_INTERFACE_IPV6_ADDRESS_NUM    9

#define MIN_DAD_DELAY_TIME                            20     /* milliseconds */

#define ND_CACHE_KEY_SIZE                      (IPV6_MAX_BITLEN + sizeof(struct interface *) * 8)
#define ND_CACHE_KEY_SIZE_BYTE            (IPV6_MAX_BYTELEN + sizeof(struct interface *))
#define ND_MSG_SIZE                                 (16 * 1024)
#define NSM_RTADV_CMSG_BUF_LEN          1024

//#define IPV6_HDR_LEN                    40
#define ETH_HDR_LEN                     14

#define ICMPV6_MIN_RS_LEN                     8
#define ICMPV6_MIN_RA_LEN                     16
#define ICMPV6_MIN_NS_LEN                     24
#define ICMPV6_MIN_NA_LEN                     24
#define ICMPV6_MIN_REDIRECT_LEN         40
#define ICMPV6_MAX_LEN                           256

#define ND_HDR_HOP_LIMIT           255
#define ALLNODE                             "ff02::1"
#define ALLROUTER                         "ff02::2"
#define SOLICITED_PREFIX              "ff02::1:ff00:0"
#define EUI64_HOST_BITS               64
#define EUI64_HOST_BYTES            8
#define SOLICITED_PREFIX_LEN      104
#define LINK_LOCAL_PREFIX_LEN    10


#define ND_IF_RTADV_ON                                  0x2    /* RA is working */
#define ND_IF_ENABLE                                       0x4    /* link-local address verified successfully */
/* RFC 2461 section 6.2.4. For the first few advertisements. Or the information contained 
  in Router Advertisements may change through actions of system management. */
#define ND_IF_RTADV_INITIAL                           0x8
#define ND_IF_ACL_ENABLED                              0x10
#define ND_IF_RTADV_SOLICITED_PENDING       0x20
#define ND_IF_RTADV_MTU                                0x40

#define ND_ENCODE_PUTC(V)                \
    do {                                                  \
        *(pnt) = (V) & 0xFF;                       \
        pnt += 1;                                      \
        *size += 1;                                   \
    } while (0)
    
#define ND_ENCODE_PUTW(V)         \
    do {                                            \
        *(pnt) = ((V) >> 8) & 0xFF;       \
        *(pnt + 1) = (V) & 0xFF;           \
        pnt += 2;                                \
        *size += 2;                             \
    } while (0)
    
#define ND_ENCODE_PUTL(V)              \
    do {                                                \
        *(pnt) = ((V) >> 24) & 0xFF;         \
        *(pnt + 1) = ((V) >> 16) & 0xFF;   \
        *(pnt + 2) = ((V) >> 8) & 0xFF;    \
        *(pnt + 3) = (V) & 0xFF;               \
        pnt += 4;                                    \
        *size += 4;                                 \
    } while (0)

#define ND_ENCODE_PUT(P, L)                                  \
        do {                                                                  \
            sal_memcpy((void*)(pnt), (void*)(P), (L));         \
            pnt += (L);                                                    \
            *size += (L);                                                 \
        } while (0)
    
/*
  ICMPv6 types
*/
#define SAL_ND_ROUTER_SOLICIT           133
#define SAL_ND_ROUTER_ADVERT            134
#define SAL_ND_NEIGHBOR_SOLICIT        135
#define SAL_ND_NEIGHBOR_ADVERT         136
#define SAL_ND_REDIRECT                        137

#define  SAL_ND_OPT_SOURCE_LINKADDR         1
#define  SAL_ND_OPT_TARGET_LINKADDR         2
#define  SAL_ND_OPT_PREFIX_INFORMATION    3
#define  SAL_ND_OPT_REDIRECTED_HEADER     4
#define  SAL_ND_OPT_MTU                                5
#define  SAL_ND_OPT_ADVINTERVAL                  7
#define  SAL_ND_OPT_HOMEAGENT_INFO           8
#define  SAL_ND_OPT_MAX                                 9

#define SAL_ND_NA_FLAG_ROUTER         0x80
#define SAL_ND_NA_FLAG_SOLICITED     0x40
#define SAL_ND_NA_FLAG_OVERRIDE      0x20

struct nd_in6_pseudo_hdr
{
    struct sal_in6_addr in6_src;
    struct  sal_in6_addr in6_dst;
    uint32       length;
    uint8        zero [3];
    uint8        nxt_hdr;
};

typedef struct nd_message_header_s
{
    uint8   dst_mac[6];
    uint8   src_mac[6];
    uint16  eth_type;
} nd_message_header_t;

struct ipv6hdr {
#if _GLB_HOST_IS_LE
    uint8     priority:4;
    uint8     version:4;
#else
    uint8     version:4;
    uint8     priority:4;
#endif
    uint8     flow_lbl[3];
    uint16   payload_len;
    uint8     nexthdr;
    uint8     hop_limit;
    struct sal_in6_addr    saddr;
    struct sal_in6_addr    daddr;
};

struct icmp6hdr {
       uint8     icmp6_type;
       uint8     icmp6_code;
       uint16   icmp6_cksum;

       union {
           uint32   un_data32[1];
           uint16   un_data16[2];
           uint8     un_data8[4];
       } ;
};

/* neighbor solicitation */
struct icmp6_neighbor_solicit {
    struct icmp6hdr        nd_ns_hdr;
    struct sal_in6_addr   nd_ns_target;   /*target address */
};

/* neighbor advertisement */
struct icmp6_neighbor_advert {
    struct icmp6hdr        nd_na_hdr;
    struct sal_in6_addr   nd_na_target;   /*target address */
};

/* router advertisement */
struct icmp6_router_advert {
    uint8                    icmp6_type;
    uint8                    icmp6_code;
    uint16                  icmp6_cksum;
    uint8                    curhoplimit;
    uint8                    flags;
    uint16                  router_lifetime;
    uint32                  reachable;   /* reachable time */
    uint32                  retransmit;  /* retransmit timer */
     /* could be followed by options */
};

/* router solicitation */
struct icmp6_router_solicit {
    uint8                    icmp6_type;
    uint8                    icmp6_code;
    uint16                  icmp6_cksum;
    uint32                  reserved;
     /* could be followed by options */
};

struct nd_opt_src_link_addr {
    uint8                       type;
    uint8                       length;
    mac_addr_t              lladdr; 
};

struct nd_opt_target_link_addr {
    uint8                       type;
    uint8                       length;
    mac_addr_t              lladdr; 
};

/* prefix information */
struct nd_opt_prefix_info {
    uint8              nd_opt_pi_type;
    uint8              nd_opt_pi_len;
    uint8              nd_opt_pi_prefix_len;
    uint8              nd_opt_pi_flags_reserved;
    uint32            nd_opt_pi_valid_time;
    uint32            nd_opt_pi_preferred_time;
    uint32            nd_opt_pi_reserved2;
    addr_ipv6_t    nd_opt_pi_prefix;
};

#define ADDR_IS_SOLICITED(a)   (\
(a)->s6_addr32[0] == sal_hton32(0xff020000) && \
(a)->s6_addr32[1] == sal_hton32(0x00000000) && \
(a)->s6_addr32[2] == sal_hton32(0x00000001) && \
(a)->s6_addr [12] == 0xff \
)

#define SAME_SOLICITED(a, b)   (\
(a).s6_addr [13] == (b).s6_addr [13] && \
(a).s6_addr16[7] == (b).s6_addr16[7] \
)

#define ADDR_IS_ALLNODE(a)     (\
(a)->s6_addr32[0] == sal_hton32(0xff020000) && \
(a)->s6_addr32[1] == sal_hton32(0x00000000) && \
(a)->s6_addr32[2] == sal_hton32(0x00000000) && \
(a)->s6_addr32[3] == sal_hton32(0x00000001) \
)

#define ADDR_IS_ALLROUTE(a)    (\
(a)->s6_addr32[0] == sal_hton32(0xff020000) && \
(a)->s6_addr32[1] == sal_hton32(0x00000000) && \
(a)->s6_addr32[2] == sal_hton32(0x00000000) && \
(a)->s6_addr32[3] == sal_hton32(0x00000002) \
)

#define ND6_OPT_SPACE(len) (((len)+2+7)&~7)

#define NES_IS_DETACHED(neigh)    \
    (GLB_INVALID_IFINDEX == neigh->ifindex)
    
#define NES_IS_VALID(neigh)    \
    ((neigh->state >= NES_REACHABLE) && (neigh->state <= NES_PROBE))
    
#define NES_IS_ACTIVE(neigh)    \
    (FLAG_ISSET(neigh->flags, GLB_ND_FLAG_ACTIVE))
    
#define NES_HAS_LLADDR(neigh)    \
    (((neigh->state >= NES_REACHABLE) && (neigh->state <= NES_PROBE))   \
    (FLAG_ISSET(neigh->flags, GLB_ND_FLAG_PERMANENT)))

#define ND_IF_RA_INTERVAL_RANDOM(ndif)        \
    ((sal_rand() % (ndif->nd_ra_interval_max -                        \
    ndif->nd_ra_interval_min + 1)) + ndif->nd_ra_interval_min)

#define SAL_ND_RA_FLAG_MANAGED    0x80
#define SAL_ND_RA_FLAG_OTHER      0x40
#define SAL_ND_RA_FLAG_HOME_AGENT 0x20

/*
** ICMPv6 prefix information
*/
#define SAL_ND_OPT_PI_FLAG_ONLINK        0x80
#define SAL_ND_OPT_PI_FLAG_AUTO          0x40
#define SAL_ND_OPT_PI_FLAG_ROUTER        0x20


/* nd options */
enum 
{
    ND6_OPT_PREFIX_INFO_END = 0,
    ND6_OPT_SOURCE_LL_ADDR = 1,     /* RFC2461 */
    ND6_OPT_TARGET_LL_ADDR = 2,     /* RFC2461 */
    ND6_OPT_PREFIX_INFO = 3,            /* RFC2461 */
    ND6_OPT_REDIRECT_HDR = 4,         /* RFC2461 */
    ND6_OPT_MTU = 5,                          /* RFC2461 */
    ND6_OPT_ARRAY_MAX
};

struct nd_options 
{
    uint8   *nd_opt_array[ND6_OPT_ARRAY_MAX];
};

#define ND_OPTION_LEN_SHIFT    3

/* Neighbor discovery option header */
struct nd_opt_hdr {
    uint8    nd_opt_type;
    uint8    nd_opt_len;
    /* followed by option specific data*/
};

#endif /* !__ND_H__ */


