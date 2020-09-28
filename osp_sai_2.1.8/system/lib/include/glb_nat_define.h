#ifndef __GLB_NAT_DEFINE_H__
#define __GLB_NAT_DEFINE_H__

#define GLB_NAT_RULE_NUM_MAX         16
#define GLB_NAT_POOL_NAME_LEN 31

#define GLB_NAT_INSIDE_IF_NUM 64
#define GLB_NAT_OUTSIDE_IF_NUM 16
#define GLB_NAT_POOL_NUM 16
#define GLB_NAT_POOL_ADDRESS_NUM 253
#define GLB_NAT_SESSION_LIMIT_NODE_NUM_MAX 128

/* NAT forward by chip, set kernel ttl to 20 minutes, and refresh it every 8 minutes.*/
#define NAT_NFCT_FAST_PATH_FWD_TTL_DEFAULT (60*20)
#define NAT_NFCT_FAST_PATH_FWD_TTL_REFRESH_MIN (60*8)

#ifdef _GLB_UML_SYSTEM_
/* 30 seconds timer, processing 128 entry one time*/
#define NAT_SESSION_AGING_TIMER_LEN 30
#define NAT_SESSION_AGING_TIMER_MARK_MAX 1
#define NAT_SESSION_AGING_TIMER_PROC_NUM_ONCE 128
#define NAT_SESSION_SPEC 128
#else
/* one second timer, processing 8k entry within 60 seconds, 134 entries one time.*/
#define NAT_SESSION_AGING_TIMER_LEN 1
#define NAT_SESSION_AGING_TIMER_MARK_MAX 60
#define NAT_SESSION_AGING_TIMER_PROC_NUM_ONCE 134
#define NAT_SESSION_SPEC 8000
#endif

#define NAT_HELPER_NAME_MAX 16
#define NAT_HELPER_NAME_SIZE (NAT_HELPER_NAME_MAX + 1)

enum GLB_NAT_SESSION_CREATOR_E
{
    GLB_NAT_SESSION_CREATOR_INSIDE,     /* session created by upsteam flow, static NAT or dynamic NAT*/
    GLB_NAT_SESSION_CREATOR_OUTSIDE     /* session created by downsteam flow, must be static rule existed*/
};

enum GLB_NAT_NFT_CT_MARK_E
{
    GLB_NAT_NFT_CT_MARK_INVALID    = 0,
    GLB_NAT_NFT_CT_MARK_UPSTREAM   = 1,     /* session created by upsteam flow, set ct mark to 1*/
    GLB_NAT_NFT_CT_MARK_DNSTREAM   = 2      /* session created by downsteam flow, set ct mark to 2*/
};

enum GLB_NAT_SET_SAI_STATUS_E
{
    GLB_NAT_SET_SAI_STATUS_INVALID,
    GLB_NAT_SET_SAI_STATUS_SUCCESS,
    GLB_NAT_SET_SAI_STATUS_FAILED,
    GLB_NAT_SET_SAI_AGAIN
};

enum GLB_NAT_RULE_FLAG_E
{
    GLB_NAT_RULE_FLAG_INSIDE             = 1 << 0,
    GLB_NAT_RULE_FLAG_OUTSIDE            = 1 << 1,
    GLB_NAT_RULE_FLAG_SNAT               = 1 << 2,
    GLB_NAT_RULE_FLAG_DNAT               = 1 << 3,
    GLB_NAT_RULE_FLAG_STATIC             = 1 << 4,
    GLB_NAT_RULE_FLAG_PROTO              = 1 << 5,
    GLB_NAT_RULE_FLAG_IN_LOCAL_IP        = 1 << 6,
    GLB_NAT_RULE_FLAG_IN_LOCAL_PORT      = 1 << 7,
    GLB_NAT_RULE_FLAG_ACL                = 1 << 8,
    GLB_NAT_RULE_FLAG_IN_GLB_IP          = 1 << 9,
    GLB_NAT_RULE_FLAG_IN_GLB_PORT        = 1 << 10,
    GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI     = 1 << 11,
    GLB_NAT_RULE_FLAG_OUTSIDE_IF         = 1 << 12,
    GLB_NAT_RULE_FLAG_POOL               = 1 << 13,
    GLB_NAT_RULE_FLAG_RANDOM             = 1 << 14,

    GLB_NAT_RULE_FLAG_MAX
};

#define NAT_SESSION_TTL_DEFAULT 180

#endif /* __GLB_NAT_DEFINE_H__ */

