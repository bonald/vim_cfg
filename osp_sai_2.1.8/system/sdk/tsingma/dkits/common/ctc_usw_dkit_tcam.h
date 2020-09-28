#ifndef _CTC_GOLDENGATE_DKIT_TCAM_H_
#define _CTC_GOLDENGATE_DKIT_TCAM_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"

#define DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE    15
#define DKITS_TCAM_PER_TYPE_PRIORITY_NUM     8

#define DKITS_TCAM_IGS_ACL0_KEY(tblid) ((DsAclQosForwardKey320Ing0_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing0_t == tblid)              \
                                       || (DsAclQosKey80Ing0_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing0_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing0_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing0_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing0_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing0_t == tblid)              \
                                       || (DsAclQosMacKey160Ing0_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing0_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing0_t == tblid)                \
                                       || (DsAclQosCidKey160Ing0_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing0_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing0_t == tblid))

#define DKITS_TCAM_IGS_ACL1_KEY(tblid) ((DsAclQosForwardKey320Ing1_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing1_t == tblid)              \
                                       || (DsAclQosKey80Ing1_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing1_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing1_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing1_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing1_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing1_t == tblid)              \
                                       || (DsAclQosMacKey160Ing1_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing1_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing1_t == tblid)                \
                                       || (DsAclQosCidKey160Ing1_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing1_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing1_t == tblid))

#define DKITS_TCAM_IGS_ACL2_KEY(tblid) ((DsAclQosForwardKey320Ing2_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing2_t == tblid)              \
                                       || (DsAclQosKey80Ing2_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing2_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing2_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing2_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing2_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing2_t == tblid)              \
                                       || (DsAclQosMacKey160Ing2_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing2_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing2_t == tblid)                \
                                       || (DsAclQosCidKey160Ing2_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing2_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing2_t == tblid))

#define DKITS_TCAM_IGS_ACL3_KEY(tblid) ((DsAclQosForwardKey320Ing3_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing3_t == tblid)              \
                                       || (DsAclQosKey80Ing3_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing3_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing3_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing3_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing3_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing3_t == tblid)              \
                                       || (DsAclQosMacKey160Ing3_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing3_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing3_t == tblid)                \
                                       || (DsAclQosCidKey160Ing3_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing3_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing3_t == tblid))

#define DKITS_TCAM_IGS_ACL4_KEY(tblid) ((DsAclQosForwardKey320Ing4_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing4_t == tblid)              \
                                       || (DsAclQosKey80Ing4_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing4_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing4_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing4_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing4_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing4_t == tblid)              \
                                       || (DsAclQosMacKey160Ing4_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing4_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing4_t == tblid)                \
                                       || (DsAclQosCidKey160Ing4_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing4_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing4_t == tblid))

#define DKITS_TCAM_IGS_ACL5_KEY(tblid) ((DsAclQosForwardKey320Ing5_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing5_t == tblid)              \
                                       || (DsAclQosKey80Ing5_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing5_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing5_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing5_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing5_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing5_t == tblid)              \
                                       || (DsAclQosMacKey160Ing5_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing5_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing5_t == tblid)                \
                                       || (DsAclQosCidKey160Ing5_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing5_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing5_t == tblid))

#define DKITS_TCAM_IGS_ACL6_KEY(tblid) ((DsAclQosForwardKey320Ing6_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing6_t == tblid)              \
                                       || (DsAclQosKey80Ing6_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing6_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing6_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing6_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing6_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing6_t == tblid)              \
                                       || (DsAclQosMacKey160Ing6_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing6_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing6_t == tblid)                \
                                       || (DsAclQosCidKey160Ing6_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing6_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing6_t == tblid))

#define DKITS_TCAM_IGS_ACL7_KEY(tblid) ((DsAclQosForwardKey320Ing7_t == tblid)                \
                                       || (DsAclQosForwardKey640Ing7_t == tblid)              \
                                       || (DsAclQosKey80Ing7_t == tblid)                      \
                                       || (DsAclQosIpv6Key320Ing7_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Ing7_t == tblid)                 \
                                       || (DsAclQosL3Key160Ing7_t == tblid)                   \
                                       || (DsAclQosL3Key320Ing7_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Ing7_t == tblid)              \
                                       || (DsAclQosMacKey160Ing7_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Ing7_t == tblid)                \
                                       || (DsAclQosMacL3Key640Ing7_t == tblid)                \
                                       || (DsAclQosCidKey160Ing7_t  == tblid)                 \
                                       || (DsAclQosCoppKey320Ing7_t == tblid)                 \
                                       || (DsAclQosCoppKey640Ing7_t == tblid))

#define DKITS_TCAM_EGS_ACL0_KEY(tblid) ((DsAclQosKey80Egr0_t == tblid)                        \
                                       || (DsAclQosIpv6Key320Egr0_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Egr0_t == tblid)                 \
                                       || (DsAclQosL3Key160Egr0_t == tblid)                   \
                                       || (DsAclQosL3Key320Egr0_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Egr0_t == tblid)              \
                                       || (DsAclQosMacKey160Egr0_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Egr0_t == tblid)                \
                                       || (DsAclQosMacL3Key640Egr0_t == tblid)                \
                                       || (DsAclQosMacL3Key640Egr0_t == tblid))

#define DKITS_TCAM_EGS_ACL1_KEY(tblid) ((DsAclQosKey80Egr1_t == tblid)                        \
                                       || (DsAclQosIpv6Key320Egr1_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Egr1_t == tblid)                 \
                                       || (DsAclQosL3Key160Egr1_t == tblid)                   \
                                       || (DsAclQosL3Key320Egr1_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Egr1_t == tblid)              \
                                       || (DsAclQosMacKey160Egr1_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Egr1_t == tblid)                \
                                       || (DsAclQosMacL3Key640Egr1_t == tblid))

#define DKITS_TCAM_EGS_ACL2_KEY(tblid) ((DsAclQosKey80Egr2_t == tblid)                        \
                                       || (DsAclQosIpv6Key320Egr2_t == tblid)                 \
                                       || (DsAclQosIpv6Key640Egr2_t == tblid)                 \
                                       || (DsAclQosL3Key160Egr2_t == tblid)                   \
                                       || (DsAclQosL3Key320Egr2_t == tblid)                   \
                                       || (DsAclQosMacIpv6Key640Egr2_t == tblid)              \
                                       || (DsAclQosMacKey160Egr2_t == tblid)                  \
                                       || (DsAclQosMacL3Key320Egr2_t == tblid)                \
                                       || (DsAclQosMacL3Key640Egr2_t == tblid))

#define DKITS_TCAM_IGS_SCL0_CFL(tblid)  ((DsUserId0TcamKey80_t == tblid)                      \
                                       || (DsUserId0TcamKey160_t == tblid)                    \
                                       || (DsUserId0TcamKey320_t == tblid))

#define DKITS_TCAM_IGS_SCL0_KEY(tblid) ((DKITS_TCAM_IGS_SCL0_CFL(tblid))                      \
                                       || (DsUserId0TcamKey80_t == tblid)                     \
                                       || (DsScl0MacKey160_t == tblid)                        \
                                       || (DsScl0L3Key160_t == tblid)                         \
                                       || (DsScl0Ipv6Key320_t == tblid)                       \
                                       || (DsScl0MacL3Key320_t == tblid)                      \
                                       || (DsScl0MacIpv6Key640_t == tblid))

#define DKITS_TCAM_IGS_SCL1_CFL(tblid) ((DsUserId1TcamKey80_t == tblid)                       \
                                       || (DsUserId1TcamKey160_t == tblid)                    \
                                       || (DsUserId1TcamKey320_t == tblid))

#define DKITS_TCAM_IGS_SCL1_KEY(tblid) ((DKITS_TCAM_IGS_SCL1_CFL(tblid))                      \
                                       || (DsUserId1TcamKey80_t == tblid)                     \
                                       || (DsScl1MacKey160_t == tblid)                        \
                                       || (DsScl1L3Key160_t == tblid)                         \
                                       || (DsScl1Ipv6Key320_t == tblid)                       \
                                       || (DsScl1MacL3Key320_t == tblid)                      \
                                       || (DsScl1MacIpv6Key640_t == tblid))

#define DKITS_TCAM_IGS_SCL2_KEY(tblid) ((DsScl2MacKey160_t == tblid)                          \
                                       || (DsScl2L3Key160_t == tblid)                         \
                                       || (DsScl2Ipv6Key320_t == tblid)                       \
                                       || (DsScl2MacL3Key320_t == tblid)                      \
                                       || (DsScl2MacIpv6Key640_t == tblid))


#define DKITS_TCAM_IGS_SCL3_KEY(tblid) ((DsScl3MacKey160_t == tblid)                          \
                                       || (DsScl3L3Key160_t == tblid)                         \
                                       || (DsScl3Ipv6Key320_t == tblid)                       \
                                       || (DsScl3MacL3Key320_t == tblid)                      \
                                       || (DsScl3MacIpv6Key640_t == tblid))

#define DKITS_TCAM_IGS_SCL0_AD(tblid)  ((DsUserId0Tcam_t == tblid)                            \
                                       || (DsTunnelId0Tcam_t == tblid)                        \
                                       || (DsSclFlow0Tcam_t == tblid)                         \
                                       || (DsUserIdTcam160Ad0_t == tblid)                     \
                                       || (DsUserIdTcam320Ad0_t == tblid))

#define DKITS_TCAM_IGS_SCL1_AD(tblid)  ((DsUserId1Tcam_t == tblid)                            \
                                       || (DsTunnelId1Tcam_t == tblid)                        \
                                       || (DsSclFlow1Tcam_t == tblid)                         \
                                       || (DsUserIdTcam160Ad1_t == tblid)                     \
                                       || (DsUserIdTcam320Ad1_t == tblid))

#define DKITS_TCAM_IPLKP0_KEY(tblid)   ((DsLpmTcamIpv4DaPubHalfKey_t == tblid)              \
                                       || (DsLpmTcamIpv6DaPubDoubleKey0_t == tblid)         \
                                       || (DsLpmTcamIpv6SingleKey_t == tblid)                 \
                                       || (DsLpmTcamIpv6SaSingleKey_t == tblid)               \
                                       || (DsLpmTcamIpv6DaPubSingleKey_t == tblid)            \
                                       || (DsLpmTcamIpv6SaPubSingleKey_t == tblid)            \
                                       || (DsLpmTcamIpv4HalfKey_t == tblid)                   \
                                       || (DsLpmTcamIpv6DoubleKey0_t == tblid))

#define DKITS_TCAM_IPLKP1_KEY(tblid)   ((DsLpmTcamIpv4HalfKeyLookup2_t == tblid)              \
                                       || (DsLpmTcamIpv6DoubleKey0Lookup2_t == tblid)         \
                                       || (DsLpmTcamIpv4SaHalfKeyLookup2_t == tblid)          \
                                       || (DsLpmTcamIpv6SaDoubleKey0Lookup2_t == tblid)       \
                                       || (DsLpmTcamIpv4DaPubHalfKeyLookup2_t == tblid)       \
                                       || (DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t == tblid)    \
                                       || (DsLpmTcamIpv4SaPubHalfKeyLookup2_t == tblid)       \
                                       || (DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t == tblid))

#define DKITS_TCAM_NATPBR_KEY(tblid)   ((DsLpmTcamIpv4PbrDoubleKey_t == tblid)                \
                                       || (DsLpmTcamIpv4NatDoubleKey_t == tblid)              \
                                       || (DsLpmTcamIpv6QuadKey_t == tblid)                   \
                                       || (DsLpmTcamIpv6DoubleKey1_t == tblid))

enum dkits_tcam_acl_key_e
{
    DKITS_TCAM_ACL_KEY_MAC160                   = 0x0,
    DKITS_TCAM_ACL_KEY_L3160                    = 0x1,
    DKITS_TCAM_ACL_KEY_L3320                    = 0x2,
    DKITS_TCAM_ACL_KEY_IPV6320                  = 0x3,
    DKITS_TCAM_ACL_KEY_IPV6640                  = 0x4,
    DKITS_TCAM_ACL_KEY_MACL3320                 = 0x5,
    DKITS_TCAM_ACL_KEY_MACL3640                 = 0x6,
    DKITS_TCAM_ACL_KEY_MACIPV6640               = 0x7,
    DKITS_TCAM_ACL_KEY_CID160                   = 0x8,
    DKITS_TCAM_ACL_KEY_SHORT80                  = 0x9,
    DKITS_TCAM_ACL_KEY_FORWARD320               = 0xa,
    DKITS_TCAM_ACL_KEY_FORWARD640               = 0xb,
    DKITS_TCAM_ACL_KEY_COPP320                  = 0xc,
    DKITS_TCAM_ACL_KEY_COPP640                  = 0xd,
    DKITS_TCAM_ACL_KEY_NUM
};
typedef enum dkits_tcam_acl_key_e dkits_tcam_acl_key_t;

enum dkits_tcam_scl_key_e
{
    DKITS_TCAM_SCL_KEY_L2                      = 0x0,
    DKITS_TCAM_SCL_KEY_L3_IPV4                 = 0x1,
    DKITS_TCAM_SCL_KEY_L3_IPV6                 = 0x2,
    DKITS_TCAM_SCL_KEY_L2L3_IPV4               = 0x3,
    DKITS_TCAM_SCL_KEY_L2L3_IPV6               = 0x4,
    DKITS_TCAM_SCL_KEY_USERID_3W               = 0x5,
    DKITS_TCAM_SCL_KEY_USERID_6W               = 0x6,
    DKITS_TCAM_SCL_KEY_USERID_12W              = 0x7,
    DKITS_TCAM_SCL_KEY_NUM
};
typedef enum dkits_tcam_scl_key_e dkits_tcam_scl_key_t;

enum dkits_tcam_scl_ad_e
{
    DKITS_TCAM_SCL_AD_USERID,
    DKITS_TCAM_SCL_AD_TUNNEL,
    DKITS_TCAM_SCL_AD_FLOW,
    DKITS_TCAM_SCL_AD_NUM
};
typedef enum dkits_tcam_scl_ad_e dkits_tcam_scl_ad_t;

enum dkits_tcam_ip_key_e
{
    DKITS_TCAM_IP_KEY_IPV4UC,
    DKITS_TCAM_IP_KEY_IPV6UC,
    DKITS_TCAM_IP_KEY_NUM
};
typedef enum dkits_tcam_ip_key_e dkits_tcam_ip_key_t;

enum dkits_tcam_natpbr_key_e
{
    DKITS_TCAM_NATPBR_KEY_IPV4PBR,
    DKITS_TCAM_NATPBR_KEY_IPV6PBR,
    DKITS_TCAM_NATPBR_KEY_IPV4NAT,
    DKITS_TCAM_NATPBR_KEY_IPV6NAT,
    DKITS_TCAM_NATPBR_KEY_NUM
};
typedef enum dkits_tcam_natpbr_key_e dkits_tcam_natpbr_key_t;

enum dkits_tcam_block_type_e
{
    DKITS_TCAM_BLOCK_TYPE_IGS_SCL0,
    DKITS_TCAM_BLOCK_TYPE_IGS_SCL1,
    DKITS_TCAM_BLOCK_TYPE_IGS_SCL2,
    DKITS_TCAM_BLOCK_TYPE_IGS_SCL3,

    DKITS_TCAM_BLOCK_TYPE_IGS_ACL0,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL1,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL2,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL3,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL4,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL5,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL6,
    DKITS_TCAM_BLOCK_TYPE_IGS_ACL7,

    DKITS_TCAM_BLOCK_TYPE_EGS_ACL0,
    DKITS_TCAM_BLOCK_TYPE_EGS_ACL1,
    DKITS_TCAM_BLOCK_TYPE_EGS_ACL2,

    DKITS_TCAM_BLOCK_TYPE_LPM_LKP0,
    DKITS_TCAM_BLOCK_TYPE_LPM_LKP1,
    DKITS_TCAM_BLOCK_TYPE_NAT_PBR,

    DKITS_TCAM_BLOCK_TYPE_CATEGORYID,
    DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE,

    DKITS_TCAM_BLOCK_TYPE_NUM,
    DKITS_TCAM_BLOCK_TYPE_INVALID = DKITS_TCAM_BLOCK_TYPE_NUM
};
typedef enum dkits_tcam_block_type_e dkits_tcam_block_type_t;

enum dkits_tcam_lpm_usage_type_e
{
    DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA                             = 0, /* only public Ipda */
    DKITS_TCAM_LPM_USAGE_PRIVATE_IPDA                            = 1, /* only private ipda */
    DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA_IPSA_HALF                   = 2, /* public ipda+ipsa, half size, performance */
    DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA_IPSA_FULL                   = 3, /* public ipda+ipsa, full size, no performance */
    DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA_PRIVATE_IPDA_HALF           = 4, /* public and private ipda, half size, performance */
    DKITS_TCAM_LPM_USAGE_PRIVATE_IPDA_IPSA_HALF                  = 5, /* private ipda+ipsa, half size, performance */
    DKITS_TCAM_LPM_USAGE_PRIVATE_IPDA_IPSA_FULL                  = 6, /* private ipda+ipsa, full size, no performance, default Mode */
    DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF = 7, /* public ipda+ipsa, private ipda+ipsa, half size, no performance */
    DKITS_TCAM_LPM_USAGE_NUM,
    DKITS_TCAM_LPM_USAGE_INVALID = DKITS_TCAM_LPM_USAGE_NUM
};
typedef enum dkits_tcam_lpm_usage_type_e dkits_tcam_lpm_usage_type_t;

enum dkits_tcam_lpm_class_type_e
{
    DKITS_TCAM_LPM_CLASS_IGS_LPM0,             /* private ipda && private ipda/ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI,        /* private ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB,        /* public ipda && public ipda/ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB,        /* public ipsa */

    DKITS_TCAM_LPM_CLASS_IGS_LPM0_LK2,         /* private ipda && private ipda/ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI_LK2,    /* private ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB_LK2,    /* public ipda && public ipda/ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB_LK2,    /* public ipsa */
    DKITS_TCAM_LPM_CLASS_IGS_NAT_PBR,          /* nat pbr */
    DKITS_TCAM_LPM_CLASS_NUM,
    DKITS_TCAM_LPM_CLASS_INVALID = DKITS_TCAM_LPM_CLASS_NUM
};
typedef enum dkits_tcam_lpm_class_type_e dkits_tcam_lpm_class_type_t;

struct ctc_dkits_tcam_tbl_s
{
    tbls_id_t tblid;
    uint16    key_type;
    uint32    sid;        /*share id*/
};
typedef struct ctc_dkits_tcam_tbl_s ctc_dkits_tcam_tbl_t;

extern char* tcam_block_desc[];
extern ctc_dkits_tcam_tbl_t ctc_dkits_tcam_tbl[DKITS_TCAM_BLOCK_TYPE_NUM][DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE];

extern int32
ctc_usw_dkits_tcam_capture_start(void*);

extern int32
ctc_usw_dkits_tcam_capture_stop(void*);

extern int32
ctc_usw_dkits_tcam_capture_show(void*);

extern int32
ctc_usw_dkits_tcam_scan(void*);

extern int32
ctc_usw_dkits_tcam_show_key_type(uint8 lchip);

extern int32
ctc_usw_dkits_tcam_mem2tbl(uint8, uint32, uint32, uint32*);

extern int32
ctc_usw_dkits_tcam_parser_key_type(uint8 , tbls_id_t, tbl_entry_t*, uint32*, uint32*);

extern int32
ctc_usw_dkits_tcam_read_tcam_key(uint8, tbls_id_t, uint32, uint32*, tbl_entry_t*);

extern int32
ctc_usw_dkits_tcam_entry_offset(uint8, tbls_id_t, uint32, uint32*, uint32*);


#ifdef __cplusplus
}
#endif

#endif

