#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))

/**
 @file sys_usw_ipuc.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_IPUC_H
#define _SYS_USW_IPUC_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "drv_api.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_hash.h"

#include "ctc_ipuc.h"
#include "ctc_spool.h"
#include "sys_usw_sort.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/* hash table size */
#define IPUC_IPV4_HASH_MASK             0xFFF
#define IPUC_IPV6_HASH_MASK             0xFFF
#define IPUC_AD_HASH_MASK               0xFFF
#define IPUC_RPF_PROFILE_HASH_MASK      0xFFF
#define IPUC_SMALL_TCAM_SIZE            256
#define IPUC_SMALL_TCAM_BLOCK_SIZE      16
#define SYS_IPUC_IPV4_TCAM_BLOCK_NUM    16
#define SYS_IPUC_AD_SPOOL_BLOCK_SIZE    1024

enum sys_ipuc_flag_e
{
    SYS_IPUC_FLAG_RPF_CHECK          = 0x01,      /**< [HB.GB.GG.D2]This route will do RPF check */
    SYS_IPUC_FLAG_TTL_CHECK          = 0x02,      /**< [HB.GB.D2]This route will do ttl check */
    SYS_IPUC_FLAG_ICMP_CHECK         = 0x04,      /**< [HB.GB.GG.D2]This route will do icmp redirect check, only set for normal ipuc nexthop, not ecmp nexthop */
    SYS_IPUC_FLAG_CPU                           = 0x08,      /**< [HB.GB.GG.D2]This route will be copied to CPU */
    SYS_IPUC_FLAG_PROTOCOL_ENTRY  = 0x10,      /**< [HB.GB.GG.D2]This route will enable l3pdu to cpu */
    SYS_IPUC_FLAG_SELF_ADDRESS       = 0x20,      /**< [GB.GG.D2]Indicate it is a host address */
    SYS_IPUC_FLAG_ICMP_ERR_MSG_CHECK = 0x40,     /**< [GB.GG.D2]This route will do icmp error message check */
    SYS_IPUC_FLAG_STATS_EN                      = 0x80,     /**< [GB.GG.D2]Only host route support stats */
    SYS_IPUC_FLAG_ASSIGN_PORT              = 0x100,     /**< [GG.D2]Use the assign port as the output port */
    SYS_IPUC_FLAG_PUBLIC_ROUTE             = 0x200,      /**< [D2]This is a public route */
    SYS_IPUC_FLAG_CANCEL_NAT                 = 0x400,      /**< [GB.GG.D2]This route will not do nat */
    SYS_IPUC_FLAG_HOST_USE_LPM          	= 0x800,      /**< [D2]This route will use as lpm */
    SYS_IPUC_FLAG_MERGE_KEY                 = 0x1000 ,     /** [D2]This entry DsFwd merge into DsIpDa */
    SYS_IPUC_FLAG_DEFAULT                      = 0x2000, /**< [GG.D2]Indicate it is a default router */
    SYS_IPUC_FLAG_IS_IPV6                      = 0x4000,   /**< [GG.D2]Indicate it is a ipv6 router */
    SYS_IPUC_FLAG_IS_TCP_PORT            = 0x8000,    /**< [GG.D2]Indicate l4_dst_port is a TCP port or is a UDP port */
    SYS_IPUC_FLAG_IS_BIND_NH              = 0x10000,   /**< [D2]Indicate the route bind nh */
    SYS_IPUC_FLAG_HASH_CONFLICT       = 0x20000,    /**< [D2]Indicate the hash route is conflict */
    SYS_IPUC_FLAG_CONNECT                   = 0x40000      /**< [HB.GB.GG.D2]This is a connect entry */
};
typedef enum sys_ipuc_flag_e sys_ipuc_flag_t;

enum sys_ipuc_operation_e
{
    DO_HASH = 1,
    DO_ALPM  = 2,
    DO_TCAM = 3
};
typedef enum sys_ipuc_operation_e sys_ipuc_operation_t;

enum sys_ipuc_route_mode_e
{
    SYS_PRIVATE_MODE,
    SYS_PUBLIC_MODE,
    MAX_ROUTE_MODE
};
typedef enum sys_ipuc_route_mode_e sys_ipuc_route_mode_t;

enum sys_ipuc_opt_type_e
{
    DO_NONE,
    DO_ADD,
    DO_DEL,
    DO_UPDATE,
    DO_LOOKUP,
    DO_MAX
};
typedef enum sys_ipuc_opt_type_e sys_ipuc_opt_type_t;

enum sys_ipuc_route_stats_e
{
    SYS_IPUC_HOST,          /*masklen = 32 or 128*/
    SYS_IPUC_LPM,          /*masklen < 32 or <128*/
    SYS_IPUC_HASH,          /*store in hash*/
    SYS_IPUC_ALPM,          /*route used calpm store*/
    SYS_IPUC_CONFLICT_TCAM,   /*masklen = 32 or 128, but store in tcam*/
    SYS_IPUC_TCAM,          /*store in private tcam*/
    SYS_IPUC_TCAM_PUB,          /* store in public tcam */
    SYS_IPUC_TCAM_SHORT,     /*store in private short tcam*/
    SYS_IPUC_TCAM_PUB_SHORT,     /* store in public short tcam */
    MAX_STATS_TYPE
};
typedef enum sys_ipuc_route_stats_e sys_ipuc_route_stats_t;

enum sys_ipuc_lookup_mode_e
{
    SYS_HASH_LOOKUP = 0x1,          /* Host HASH is used to lookup IPUC entry */
    SYS_HASH_NAPT_LOOKUP = 0x2,          /* Host HASH is used to lookup IPUC entry */
    SYS_ALPM_LOOKUP = 0x4,          /* CALPM HASH is used to lookup IPUC entry */
    SYS_TCAM_LOOKUP = 0x8,          /* TCAM is used to lookup IPUC da entry */
    SYS_TCAM_PUB_LOOKUP = 0x10,          /* TCAM is used to lookup IPUC public entry */
    SYS_TCAM_SHORT_LOOKUP = 0x20,     /* TCAM is used to lookup IPUC single key entry */
    SYS_TCAM_SA_LOOKUP = 0x40,          /* TCAM is used to lookup IPUC sa entry */
    MAX_LOOKUP_MODE
};
typedef enum sys_ipuc_lookup_mode_e sys_ipuc_lookup_mode_t;

enum sys_ipuc_tbl_type_e
{
    SYS_TBL_HASH,          /* Host HASH is used to store host route */
    SYS_TBL_HASH_NAPT,  /* Host HASH is used to store host napt route */
    SYS_TBL_ALPM,          /* CALPM HASH is used to store calpm pipeline */
    SYS_TBL_TCAM,          /* TCAM is used to store ipda tcam key */
    SYS_TBL_TCAM_PUB,          /* TCAM is used to store public ipda tcam key */
    SYS_TBL_TCAM_SHORT,          /* TCAM is used to store ipda single key */
    SYS_TBL_TCAM_PUB_SHORT,          /* TCAM is used to store public ipda single key */
    MAX_TBL_TYPE
};
typedef enum sys_ipuc_tbl_type_e sys_ipuc_tbl_type_t;

struct sys_ipuc_info_s
{
    uint32 key_index;
    uint32 ad_index;
    uint32 route_flag;  /* sys_ipuc_flag_e */

    uint32 nh_id;
    uint16 vrf_id;
    uint16 rpf_id;        /* used to store rpf_id */
    uint8  route_opt;
    uint8  masklen;
    uint8  rpf_mode;        /* refer to sys_rpf_chk_mode_t */
    uint8  real_lchip;      /* used for rchain mode, provide for nexthop callback */

    uint32 gport;           /* assign output port */
    uint32 stats_id;          /*stats id */
    uint16 l4_dst_port;     /* layer4 destination port, if not 0, indicate NAPT key */
    uint16 cid;

    union
    {
        ip_addr_t ipv4;
        ipv6_addr_t ipv6;
    } ip;
};
typedef struct sys_ipuc_info_s sys_ipuc_info_t;

struct sys_ipuc_aid_key_s
{
    uint16 vrf_id;
    uint8  masklen;
    uint8  ip_ver;
    union
    {
        ip_addr_t ipv4;
        ipv6_addr_t ipv6;
    } ip;
};
typedef struct sys_ipuc_aid_key_s sys_ipuc_aid_key_t;

struct sys_ipuc_param_s
{
    uint8 hash_conflict;
    ctc_ipuc_param_t param;
    sys_ipuc_info_t* info;
    DsIpDa_m* p_dsipda;
    uint8 is_update;
};
typedef struct sys_ipuc_param_s sys_ipuc_param_t;

struct sys_ipuc_ad_spool_s
{
    DsIpDa_m ad;
    uint32 ad_index;
};
typedef struct sys_ipuc_ad_spool_s sys_ipuc_ad_spool_t;

struct sys_ipuc_param_list_s
{
    uint32 ad_index;
    ctc_ipuc_param_t* p_ipuc_param;
    struct sys_ipuc_param_list_s* p_next_param;
};
typedef struct sys_ipuc_param_list_s sys_ipuc_param_list_t;

struct sys_ipuc_pending_route_s
{
    ctc_slistnode_t     node_head;
    sys_ipuc_param_t    sys_param;
    uint8               operation; /*0 add, 1 del, 2 update*/
    sys_ipuc_info_t     old_info;
    volatile uint8 done;
    int32 ret;
};
typedef struct sys_ipuc_pending_route_s sys_ipuc_pending_route_t;

struct sys_ipuc_master_s
{
    sal_mutex_t* mutex;
    uint32 default_base[MAX_CTC_IP_VER];
    uint32 default_nhid[MAX_CTC_IP_VER];

    uint32 max_entry_num[MAX_CTC_IP_VER][MAX_TBL_TYPE];
    uint32 route_stats[MAX_CTC_IP_VER][MAX_STATS_TYPE];

    uint16 short_key_boundary[MAX_ROUTE_MODE]; /*short key and doudle key boundary*/
    uint8 tcam_share_mode[MAX_ROUTE_MODE];
    uint8 share_ofb_type[MAX_ROUTE_MODE];
    uint8 lookup_mode[MAX_CTC_IP_VER]; /* sys_ipuc_lookup_mode_t */
    uint8 version_en[MAX_CTC_IP_VER];
    uint8 host_lpm_mode;
    uint8 snake_per_group;

    ctc_spool_t* ad_spool;
    ctc_hash_t* fib_hash[MAX_CTC_IP_VER];            /* store db of specific prefix ipuc 64k 128*512*/

    ctc_hash_t* cbkt[2]; /* double bucket cache ;*/
    sal_sem_t* alpm_sem;
    sal_task_t* p_alpm_task;
    ctc_slist_t* pending_route_list;
    uint32  pending_list_len;
    uint32  alpm_async:3;   /*0-sync ; 1-pengding list( one buket) cache;2: double bucket cache;3:detecting one buket cache or double bucket cache */
    uint32  aid_en:1;   /*auto Intelligent detection*/
    uint32  aid_list_len:8;   /*auto Intelligent detection*/
    uint32  aid_end:1;   /*0-sync ; 1-pengding list( one buket) cache;2: double bucket cache;3:detecting one buket cache or double bucket cache */
     uint32  rsv:19;

    uint8   cbkt_status[2];/* 0- free;1-writing fib;2,writing done*/
    uint32  cbkt_idx[2];
    sal_systime_t  last_tv;

    ctc_ipuc_callback callback;
    void* user_data;
    sal_mutex_t* lpm_mutex;

    uint32 lpm_tcam_spec[MAX_CTC_IP_VER];
    uint32 lpm_tcam_used_cnt[MAX_CTC_IP_VER];
    uint32 arc[MAX_CTC_IP_VER];         /*auto resolve conflict use TCAM*/
    uint8  alpm_thread_disable;
    uint8  rchain_en;
    uint8  rchain_tail;
    uint16 rchain_boundary; /*Tcam index boundary for each chip*/
    uint32* rchain_stats;
    uint8  rchain_gchip;
};
typedef struct sys_ipuc_master_s sys_ipuc_master_t;

struct sys_ipuc_traverse_s
{
    uint32 value0;
    uint32 value1;
    void* data;
    void* data1;
    void* data2;
    void* data3;
    void* fn;
    uint8 lchip;
};
typedef struct sys_ipuc_traverse_s sys_ipuc_traverse_t;

enum sys_ipuc_tcam_key_type_e
{
    SYS_IPUC_TCAM_FLAG_INVALID,
    SYS_IPUC_TCAM_FLAG_TCAM,           /* sys_ipuc_info_t */
    SYS_IPUC_TCAM_FLAG_ALPM,
};
typedef enum sys_ipuc_tcam_key_type_e sys_ipuc_tcam_key_type_t;

struct sys_ipuc_tcam_data_s
{
    uint32 key_index;
    uint32 ad_index;
    sys_ipuc_opt_type_t opt_type;
    uint8 key_type;
    uint8 masklen;
    uint8 rsv[2];
    ctc_ipuc_param_t  *ipuc_param;
    void* info;
};
typedef struct sys_ipuc_tcam_data_s sys_ipuc_tcam_data_t;

struct sys_ipuc_ofb_cb_s
{
    uint32 type:3;
    uint32 ip_ver :1;
    uint32 route_mode :1;
    uint32 masklen :8;
    uint32 rsv:19;
    void* user_data;
    void* priv_data;
};
typedef struct sys_ipuc_ofb_cb_s sys_ipuc_ofb_cb_t;


/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

 #define SYS_IPUC_DBG_OUT(level, FMT, ...)                              \
    {                                                                       \
        CTC_DEBUG_OUT(ip, ipuc, IPUC_SYS, level, FMT, ##__VA_ARGS__);  \
    }

#define SYS_IPUC_INIT_CHECK                                        \
    {                                                              \
        LCHIP_CHECK(lchip);                                        \
        if (p_usw_ipuc_master[lchip] == NULL)                          \
        {                                                          \
            return CTC_E_NOT_INIT;                                 \
        }                                                          \
    }

#define SYS_IPUC_MASK_LEN_CHECK(ver, len)         \
    {                                                 \
        if (ver == CTC_IP_VER_4)    \
        {   \
            if ((len) > CTC_IPV4_ADDR_LEN_IN_BIT){  \
                return CTC_E_INVALID_PARAM; }   \
        }   \
        else    \
        {   \
            if ((len) > CTC_IPV6_ADDR_LEN_IN_BIT){  \
                return CTC_E_INVALID_PARAM; }   \
        }   \
    }

#define IS_MAX_MASKLEN(ver, len)    (((ver == CTC_IP_VER_4) && (len == CTC_IPV4_ADDR_LEN_IN_BIT)) ||  \
        ((ver == CTC_IP_VER_6) && (len == CTC_IPV6_ADDR_LEN_IN_BIT)))    \

#define IPV4_MASK(ip, len)                                                          \
    {                                                                                   \
        uint32 mask = (len) ? ~((1 << (CTC_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0;    \
        (ip) &= mask;                                                                   \
    }

#define IPV6_MASK(ip, len)                              \
    {                                                       \
        uint8 feedlen = CTC_IPV6_ADDR_LEN_IN_BIT - (len);   \
        uint8 sublen = feedlen % 32;                        \
        int8 index = feedlen / 32;                          \
        if (sublen)                                          \
        {                                                   \
            uint32 mask = ~((1 << sublen) - 1);             \
            (ip)[index] &= mask;                            \
        }                                                   \
        index--;                                            \
        for (; index >= 0; index--)                          \
        {                                                   \
            (ip)[index] = 0;                                \
        }                                                   \
    }

#define SYS_IP_ADDR_MASK(ip, len, ver)      \
    {                                           \
        if (CTC_IP_VER_4 == (ver))               \
        {                                       \
            IPV4_MASK((ip.ipv4), (len));        \
        }                                       \
        else                                    \
        {                                       \
            IPV6_MASK((ip.ipv6), (len));        \
        }                                       \
    }

#define SYS_IP_FUNC_DBG_DUMP(val)                                                           \
    {                                                                                           \
        if ((CTC_IP_VER_4 == val->ip_ver))                                                \
        {                                                                                       \
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,                                             \
                             "nh_id :%d  vrf_id:%d  route_flag:0x%x  masklen:%d  ip_ver:%s  ip:0x%x\n",        \
                             val->nh_id, val->vrf_id, val->route_flag, val->masklen,                         \
                             "IPv4", val->ip.ipv4);                                                          \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,                                            \
                             "nh_id :%d  vrf_id:%d  route_flag:0x%x  masklen:%d  ip_ver:%s  ip:%x-%x-%x-%x\n",  \
                             val->nh_id, val->vrf_id, val->route_flag, val->masklen,                         \
                             "IPv6", val->ip.ipv6[3], val->ip.ipv6[2],                                        \
                             val->ip.ipv6[1], val->ip.ipv6[0]);                                               \
        }                                                                                       \
    }

#define SYS_IP_ADDRESS_SORT(val)           \
    {                                          \
        if (CTC_IP_VER_6 == (val)->ip_ver)      \
        {                                      \
            uint32 t;                          \
            t = val->ip.ipv6[0];               \
            val->ip.ipv6[0] = val->ip.ipv6[3]; \
            val->ip.ipv6[3] = t;               \
                                           \
            t = val->ip.ipv6[1];               \
            val->ip.ipv6[1] = val->ip.ipv6[2]; \
            val->ip.ipv6[2] = t;               \
        }                                      \
    }

#define SYS_IP_ADDRESS_SORT2(val)           \
    {                                          \
        if (CTC_IP_VER_6 == (val).ip_ver)      \
        {                                      \
            uint32 t;                          \
            t = val.ip.ipv6[0];               \
            val.ip.ipv6[0] = val.ip.ipv6[3]; \
            val.ip.ipv6[3] = t;               \
                                           \
            t = val.ip.ipv6[1];               \
            val.ip.ipv6[1] = val.ip.ipv6[2]; \
            val.ip.ipv6[2] = t;               \
        }                                      \
    }

#define SYS_IPUC_CTC2SYS_FLAG_CONVERT(ctc_route_flag, sys_route_flag)   \
{   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_RPF_CHECK))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_RPF_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_TTL_CHECK))  \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_TTL_CHECK);   \
    }   \
    if (!CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_TTL_NO_CHECK))  \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_TTL_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_ICMP_CHECK)) \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_ICMP_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_CPU))    \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_CPU); \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_PROTOCOL_ENTRY))    \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_PROTOCOL_ENTRY); \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_SELF_ADDRESS))  \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_SELF_ADDRESS);  \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_ICMP_ERR_MSG_CHECK)) \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_ICMP_ERR_MSG_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_STATS_EN))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_STATS_EN);    \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_ASSIGN_PORT))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_ASSIGN_PORT);    \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE);    \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_CANCEL_NAT))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_CANCEL_NAT);    \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_HOST_USE_LPM);    \
    }   \
    if (CTC_FLAG_ISSET(ctc_route_flag, CTC_IPUC_FLAG_CONNECT))   \
    {   \
        CTC_SET_FLAG(sys_route_flag, SYS_IPUC_FLAG_CONNECT);    \
    }   \
}

#define SYS_IPUC_SYS2CTC_FLAG_CONVERT(ctc_route_flag, sys_route_flag)   \
{   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_RPF_CHECK))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_RPF_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_TTL_CHECK))  \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_TTL_CHECK);   \
        CTC_UNSET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_TTL_NO_CHECK);   \
    }   \
    else\
    {   \
        CTC_UNSET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_TTL_CHECK);   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_TTL_NO_CHECK);  \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_ICMP_CHECK)) \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_ICMP_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_CPU))    \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_CPU); \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_PROTOCOL_ENTRY))    \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_PROTOCOL_ENTRY); \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_SELF_ADDRESS))  \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_SELF_ADDRESS);  \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_ICMP_ERR_MSG_CHECK)) \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_ICMP_ERR_MSG_CHECK);   \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_STATS_EN))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_STATS_EN);    \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_ASSIGN_PORT))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_ASSIGN_PORT);    \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE);    \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_CANCEL_NAT))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_CANCEL_NAT);    \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_HOST_USE_LPM))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);    \
    }   \
    if (CTC_FLAG_ISSET(sys_route_flag, SYS_IPUC_FLAG_CONNECT))   \
    {   \
        CTC_SET_FLAG(ctc_route_flag, CTC_IPUC_FLAG_CONNECT);    \
    }   \
}

#define SYS_IPUC_VER(ptr)       (CTC_FLAG_ISSET((ptr)->route_flag, SYS_IPUC_FLAG_IS_IPV6) ? CTC_IP_VER_6 : CTC_IP_VER_4)
#define SYS_IPUC_VER_VAL(val)   (CTC_FLAG_ISSET((val).route_flag, SYS_IPUC_FLAG_IS_IPV6) ? CTC_IP_VER_6 : CTC_IP_VER_4)

#define IS_SHORT_KEY(route_mode, key_index)     \
    (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SHORT_LOOKUP) && \
        (key_index >= p_usw_ipuc_master[lchip]->short_key_boundary[route_mode]))

#define SYS_IPUC_LPM_TCAM_MAX_INDEX 12288

extern int32
sys_usw_ipuc_alloc_tcam_key_index(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_usw_ipuc_free_tcam_key_index(uint8 lchip, sys_ipuc_tcam_data_t *p_data);

extern int32
sys_usw_ipuc_add(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_usw_ipuc_remove(uint8 lchip,ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_usw_ipuc_get(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_usw_ipuc_add_default_entry(uint8 lchip, uint8 ip_ver, uint32 nh_id);

extern int32
sys_usw_get_tbl_id(uint8 lchip, uint8 ip_ver, uint8 is_da, sys_ipuc_tbl_type_t tbl_type, uint32* key_id, uint32* ad_id);

extern int32
sys_usw_get_tbl_name(uint8 ip_ver, uint8 is_da, sys_ipuc_tbl_type_t tbl_type, uint32* key_id, uint32* ad_id);

extern int32
sys_usw_ipuc_set_global_property(uint8 lchip, ctc_ipuc_global_property_t* p_global_prop);

extern int32
sys_usw_ipuc_arrange_fragment(uint8 lchip);

extern int32
sys_usw_ipuc_hash_free_node_data(void* node_data, void* user_data);

extern int32
sys_usw_ipuc_traverse(uint8 lchip, uint8 ip_ver, ctc_ipuc_traverse_fn fn, void* data);

extern int32
sys_usw_ipuc_show_debug_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param);

extern int32
sys_usw_ipuc_init(uint8 lchip, ctc_ipuc_global_cfg_t* p_ipuc_global_cfg);

extern int32
sys_usw_ipuc_deinit(uint8 lchip);

extern int32
sys_usw_ipuc_set_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8 hit);

extern int32
sys_usw_ipuc_get_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8* hit);

int32
_sys_usw_ipuc_write_ipda(uint8 lchip, sys_ipuc_param_t * p_sys_param, uint8 by_user);

int32
_sys_usw_ipuc_read_ipda(uint8 lchip, uint16 ad_index, void * dsipda);

int32
_sys_usw_ipuc_add_ad_profile(uint8 lchip, sys_ipuc_param_t * p_sys_param, void * ad_profile, uint8 update);

int32
_sys_usw_ipuc_remove_ad_profile(uint8 lchip, uint32 ad_index);

int32
_sys_usw_ipuc_db_lookup(uint8 lchip, sys_ipuc_param_t * p_sys_param);

int32
_sys_usw_ipuc_build_ipda(uint8 lchip, sys_ipuc_param_t * p_sys_param);

int32
_sys_usw_ipuc_unbuild_ipda(uint8 lchip, sys_ipuc_param_t * p_sys_param);

#ifdef __cplusplus
}
#endif

#endif


#endif

