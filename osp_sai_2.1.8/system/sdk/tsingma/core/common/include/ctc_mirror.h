/**
 @file ctc_mirror.h

 @date 2010-1-7

 @version v2.0

 This file define the types used in APIs

*/

#ifndef _MIRROR_H
#define _MIRROR_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "common/include/ctc_const.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup mirror MIRROR
 @{
*/
/**
 @brief define max session id
*/
#define MAX_CTC_MIRROR_SESSION_ID  4  /**< [GB.GG.D2.TM] mirror max session num*/
/**
 @brief Denote setting session
*/
enum ctc_mirror_session_type_e
{
    CTC_MIRROR_L2SPAN_SESSION = 0,  /**< [GB.GG.D2.TM] mirror l2span session*/
    CTC_MIRROR_L3SPAN_SESSION = 1,  /**< [GB.GG.D2.TM] mirror l3span session*/
    CTC_MIRROR_ACLLOG_SESSION = 2,  /**< [GB.GG.D2.TM] mirror acllog session*/
    CTC_MIRROR_CPU_SESSION    = 3,  /**< [GB.GG.D2.TM] mirror cpu session*/
    CTC_MIRROR_IPFIX_LOG_SESSION,   /**< [D2.TM] mirror ipfix log session*/
    MAX_CTC_MIRROR_SESSION_TYPE
};
typedef enum ctc_mirror_session_type_e ctc_mirror_session_type_t;

#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_L4_PORT       0x00000001  /**< [GG.D2.TM] ipv4 layer4 port as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IP_ADDR       0x00000002  /**< [GG.D2.TM] ipv4 address as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_UDP        0x00000004  /**< [GG.D2.TM] ipv4 packet's layer4 isUdp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_TCP        0x00000008  /**< [GG.D2.TM] ipv4 packet's layer4 isTcp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_DCCP        0x00000010  /**< [TM] ipv4 packet's layer4 isDccp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_SCTP        0x00000020  /**< [TM] ipv4 packet's layer4 isSctp as hash field of symmetrical hash for erspan*/

#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_L4_PORT       0x00000001  /**< [GG.D2.TM] ipv6 layer4 port as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IP_ADDR       0x00000002  /**< [GG.D2.TM] ipv6 address as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_UDP        0x00000004  /**< [GG.D2.TM] ipv6 packet's layer4 isUdp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_TCP        0x00000008  /**< [GG.D2.TM] ipv6 packet's layer4 isTcp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_DCCP        0x00000010  /**< [TM] ipv6 packet's layer4 isDccp as hash field of symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_SCTP        0x00000020  /**< [TM] ipv6 packet's layer4 isSctp as hash field of symmetrical hash for erspan*/

#define    CTC_MIRROR_ERSPAN_PSC_TYPE_IPV4   0x1  /**< [GG.D2.TM] ipv4 symmetrical hash for erspan*/
#define    CTC_MIRROR_ERSPAN_PSC_TYPE_IPV6   0x2  /**< [GG.D2.TM] ipv6 symmetrical hash for erspan*/

struct ctc_mirror_erspan_psc_s
{
    uint32  type;       /**< [GG.D2.TM] refer to CTC_MIRROR_ERSPAN_PSC_TYPE_XXX*/
    uint32  ipv4_flag;  /**< [GG.D2.TM] refer to CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_XXX, 0 meaning disable */
    uint32  ipv6_flag;  /**< [GG.D2.TM] refer to CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_XXX, 0 meaning disable */
};
typedef struct ctc_mirror_erspan_psc_s ctc_mirror_erspan_psc_t;

/**
 @brief Denote setting port/vlan mirror drop packet
*/
enum ctc_mirror_discard_e
{
    CTC_MIRROR_L2SPAN_DISCARD = 0x1,       /**< [GB.GG.D2.TM] mirror qos session drop packet*/
    CTC_MIRROR_L3SPAN_DISCARD = 0x2,       /**< [GB.GG.D2.TM] mirror vlan session drop packet*/
    CTC_MIRROR_ACLLOG_PRI_DISCARD = 0x4,   /**< [GB.GG.D2.TM] mirror acl priority session drop packet*/
    CTC_MIRROR_IPFIX_DISCARD = 0x8         /**< [D2.TM] mirror ipfix log drop packet*/
};
typedef enum ctc_mirror_discard_e ctc_mirror_discard_t;

/**
 @brief Denote mirror port/vlan/acl nexthop info
*/
enum ctc_mirror_info_type_e
{
    CTC_MIRROR_INFO_PORT,                /**< [GG.D2.TM] port mirror info*/
    CTC_MIRROR_INFO_VLAN,                /**< [GG.D2.TM] vlan mirror info*/
    CTC_MIRROR_INFO_ACL,                 /**< [GG.D2.TM] acl mirror info*/
    CTC_MIRROR_INFO_NUM
};
typedef enum ctc_mirror_info_type_e ctc_mirror_info_type_t;

/**
 @brief Define mirror source information used in Mirror API
*/
struct ctc_mirror_dest_s
{
    uint8   acl_priority;  /**< [GB.GG.D2.TM] GB and GG: mirror acl priority 0,1,2,3*/
    uint8   session_id;    /**< [GB.GG.D2.TM] mirror session id, range<0-3>*/
    uint8   is_rspan;      /**< [GB.GG.D2.TM] if set, session is remote mirror; if not set, session is local mirror*/
    uint8   vlan_valid;    /**< [GB.GG.D2.TM] used in remote mirror, if set, remote mirror edit info by rspan.vlan_id; Otherwise, by rspan.nh_id*/
    uint32  dest_gport;    /**< [GB.GG.D2.TM] mirror destination global port*/

    uint8   truncation_en; /**< [GG] Packet will be truncated. The truncation length refer to CTC_QOS_GLB_CFG_TRUNCATION_LEN */
    uint8   rsv[3];
    uint32  truncated_len; /**< [D2.TM] The length of packet after truncate, value 0 is disable*/

    union
    {
        uint16 vlan_id;  /**< [GB.GG.D2.TM] rspan over L2*/
        uint32 nh_id;    /**< [GB.GG.D2.TM] rspan edit info by nexthop*/
    } rspan;
    ctc_mirror_session_type_t type;  /**< [GB.GG.D2.TM] mirror session type, see enum ctc_mirror_session_type_t*/
    ctc_direction_t dir;             /**< [GB.GG.D2.TM] direction of mirror, ingress or egress*/
};
typedef struct ctc_mirror_dest_s ctc_mirror_dest_t;

/**
 @brief Defien rspan escape info, packet with this info will be not mirror
*/
struct ctc_mirror_rspan_escape_s
{
    mac_addr_t mac0;       /**< [GB.GG.D2.TM] mac0 value,all should be 0 if no care*/
    mac_addr_t mac_mask0;  /**< [GB.GG.D2.TM] mac0 mask, all should be 0xFF if no care*/
    mac_addr_t mac1;       /**< [GB.GG.D2.TM] mac1 value,all should be 0 if no care*/
    mac_addr_t mac_mask1;  /**< [GB.GG.D2.TM] mac1 mask, all should be 0xFF if no care*/
};
typedef struct ctc_mirror_rspan_escape_s ctc_mirror_rspan_escape_t;

/**@} end of @defgroup mirror  */

#ifdef __cplusplus
}
#endif

#endif

