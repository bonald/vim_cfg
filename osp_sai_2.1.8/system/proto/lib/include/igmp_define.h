
#ifndef __IGMP_DEFINE_H__
#define __IGMP_DEFINE_H__

#define IGMP_IPV4_ADDR_TO_MAC(IPV4MCA, MAC) \
do { \
    uint8 *_p_ipa = (uint8*)(&IPV4MCA); \
    ((uint8*)(MAC))[0] = 0x01; \
    ((uint8*)(MAC))[1] = 0x00; \
    ((uint8*)(MAC))[2] = 0x5E; \
    ((uint8*)(MAC))[3] = (_p_ipa[1] & 0x7F); \
    ((uint8*)(MAC))[4] = (_p_ipa[2]); \
    ((uint8*)(MAC))[5] = (_p_ipa[3]); \
} while (0)

#define IGMP_ONE_SEC_MILLISECOND                1000

/* 224.0.0.1 */
#ifndef INADDR_ALLHOSTS_GROUP
#define INADDR_ALLHOSTS_GROUP                   (0xE0000001UL)
#endif /* INADDR_ALLHOSTS_GROUP */
/* 224.0.0.2 */
#ifndef INADDR_ALLRTRS_GROUP
#define INADDR_ALLRTRS_GROUP                    (0xE0000002UL)
#endif /* INADDR_ALLRTRS_GROUP */
/* 224.0.0.22 */
#ifndef INADDR_V3RTRS_GROUP
#define INADDR_V3RTRS_GROUP                     (0xE0000016UL)
#endif /* INADDR_V3RTRS_GROUP */
#ifndef INADDR_MAX_LOCAL_GROUP
#define INADDR_MAX_LOCAL_GROUP                  (0xE00000FFUL)
#endif /* INADDR_MAX_LOCAL_GROUP */

#define INADDR_QUERY_ADDR                       (0xC0A80001UL)

#define IGMP_VERSION_1                          1
#define IGMP_VERSION_2                          2
#define IGMP_VERSION_3                          3
#define IGMP_VERSION_MIN                        IGMP_VERSION_1
#define IGMP_VERSION_DEF                        IGMP_VERSION_2
#define IGMP_VERSION_MAX                        IGMP_VERSION_3

#define IGMP_LIMIT_MIN                          1
#ifdef GREATBELT
#define IGMP_LIMIT_DEF                          2024
#else
#define IGMP_LIMIT_DEF                          2048
#endif
#define IGMP_LIMIT_MAX                          2048

/* Last Member Query Count */
#define IGMP_LAST_MEMBER_QUERY_COUNT_MIN        2
#define IGMP_LAST_MEMBER_QUERY_COUNT_DEF        2
#define IGMP_LAST_MEMBER_QUERY_COUNT_MAX        7

/* Last Member Query Interval in msec */
#define IGMP_LAST_MEMBER_QUERY_INTERVAL_MIN     1000
#define IGMP_LAST_MEMBER_QUERY_INTERVAL_DEF     1000
#define IGMP_LAST_MEMBER_QUERY_INTERVAL_MAX     25500

/* IGMP Interface Robustness Variable value */
#define IGMP_ROBUSTNESS_VAR_MIN                 2
#define IGMP_ROBUSTNESS_VAR_DEF                 2
#define IGMP_ROBUSTNESS_VAR_MAX                 7

/* Query Interval in sec */
#define IGMP_QUERY_INTERVAL_MIN                 2
#define IGMP_QUERY_INTERVAL_DEF                 125
#define IGMP_QUERY_INTERVAL_MAX                 18000

/* Query Max-Response Timeout in sec */
#define IGMP_QUERY_RESPONSE_INTERVAL_MIN        1
#define IGMP_QUERY_RESPONSE_INTERVAL_DEF        10
#define IGMP_QUERY_RESPONSE_INTERVAL_MAX        25

#define IGMP_MROUTER_PORT_TIMEOUT_MIN           1
#define IGMP_MROUTER_PORT_TIMEOUT_DEF           255
#define IGMP_MROUTER_PORT_TIMEOUT_MAX           65535

/* Other-Querier Timeout in sec */
#define IGMP_QUERIER_TIMEOUT_MIN                60
#define IGMP_QUERIER_TIMEOUT_DEF    (IGMP_ROBUSTNESS_VAR_DEF * IGMP_QUERY_INTERVAL_DEF + (IGMP_QUERY_RESPONSE_INTERVAL_DEF / 2))
#define IGMP_QUERIER_TIMEOUT_MAX                300

/* IGMP Message-Types */
#define IGMP_MSG_MEMBERSHIP_QUERY               0x11
#define IGMP_MSG_PIM_HELLO                      0x0
#define IGMP_MSG_V1_MEMBERSHIP_REPORT           0x12
#define IGMP_MSG_V2_MEMBERSHIP_REPORT           0x16
#define IGMP_MSG_V2_LEAVE_GROUP                 0x17
#define IGMP_MSG_V3_MEMBERSHIP_REPORT           0x22
#define IGMP_MSG_DVMRP                          0x13

#define IGMP_MSG_V3_REPORT_GRP_REC_MIN_SIZE     8

#define IGMP_ONE_SEC_UNIT                       10

#define IGMP_SNOOPING_TCN_QUERY_CNT             2
#define IGMP_SNOOPING_TCN_QUERY_INTERVAL        10
#define IGMP_SNOOPING_TCN_QUERY_MAX_RES_TIME    5

#define IGMP_GROUP_EXPIRE_INTERVAL              1   /* 1 second */

#define IGMP_HOST_JOIN_GROUP_MAX                500


/* IGMP Group Router Filter-Mode-Change */
typedef enum
{
    IGMP_FMC_INVALID = 0,
    IGMP_FMC_MODE_IS_INCLUDE,
    IGMP_FMC_MODE_IS_EXCLUDE,
    IGMP_FMC_CHANGE_TO_INCLUDE_MODE,
    IGMP_FMC_CHANGE_TO_EXCLUDE_MODE,
    IGMP_FMC_ALLOW_NEW_SOURCES,
    IGMP_FMC_BLOCK_OLD_SOURCES,
    IGMP_FMC_MODE_MAX
} igmp_fmc_mode_t;

/* IGMP Messages related defines */
#ifndef IPOPT_RA
#define IPOPT_RA                                (148)
#endif /* IPOPT_RA */
#define IGMP_IPOPT_RA                           IPOPT_RA

#define IGMP_RA_SIZE                            (4)

typedef enum
{
    IGMP_TYPE_DYNAMIC = 0,
    IGMP_TYPE_STATIC,
    IGMP_TYPE_MAX
} igmp_type_t;

typedef enum
{
    IGMP_UNKNOWN_FLOOD = 0,
    IGMP_UNKNOWN_DISCARD,
    IGMP_UNKNOWN_DISCARD_MAX
} igmp_unknown_discard_t;

#if 1
#define IGSP_DEBUG_INFO(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(IGSP, igsp, typeenum, fmt, ##args);\
}while(0)
#else
#define IGSP_DEBUG_INFO(mod, sub, typeenum, fmt, args...) \
do { \
    log_sys(M_MOD_IGSP, E_ERROR, fmt, ##args); \
} while (0)
#endif

#define IGSP_DEBUG_FUNC(typeenum) \
    IGSP_DEBUG_INFO(typeenum, "Enter into %s.", __FUNCTION__)

#define IGSP_DEBUG(typeenum, fmt, args...) \
do {\
    IGSP_DEBUG_INFO(typeenum, fmt, ##args);\
}while(0)

#endif /* __IGMP_DEFINE_H__ */

