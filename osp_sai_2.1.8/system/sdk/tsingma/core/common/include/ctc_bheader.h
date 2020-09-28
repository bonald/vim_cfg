/**
 @file ctc_bheader.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2012-3-14

 @version v2.0

 This file define the bheader struct
*/

#ifndef _CTC_BHEADER_H
#define _CTC_BHEADER_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_common.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup bheader BHEADER
 @{
*/

/**
 @brief  Define bheader packet type
*/
enum ctc_bheader_pkt_type_e
{
    CTC_BHEADER_ETH_PKT,
    CTC_BHEADER_IPV4_PKT,
    CTC_BHEADER_MPLS_PKT,
    CTC_BHEADER_IPV6_PKT,
    CTC_BHEADER_MPLS_MCAST_PKT,
    CTC_BHEADER_FLEX_PKT1,
    CTC_BHEADER_FLEX_PKT2,
    MAX_CTC_BHEADER_PKT_TYPE
};
typedef enum ctc_bheader_pkt_type_e ctc_bheader_pkt_type_t;

/**
 @brief  Define bheader nexthop type
*/
enum ctc_bheader_nexthop_type_e
{
    CTC_BHEADER_NEXTHOP_TYPE_BYPASS = 0,
    CTC_BHEADER_NEXTHOP_TYPE_BRIDGE,
    CTC_BHEADER_NEXTHOP_TYPE_UNTAG,
    CTC_BHEADER_NEXTHOP_TYPE_USER,
    MAX_CTC_BHEADER_NEXTHOP_TYPE
};
typedef enum ctc_bheader_nexthop_type_e ctc_bheader_nexthop_type_t;

/**
 @brief  Define bheader operation type
*/
enum ctc_bheader_operation_type_e
{
    CTC_BHEADER_OPERATION_NORMAL,
    CTC_BHEADER_OPERATION_PTP,
    CTC_BHEADER_OPERATION_NAT,
    CTC_BHEADER_OPERATION_OAM,
    CTC_BHEADER_OPERATION_UCAST,
    CTC_BHEADER_OPERATION_BCAST,
    CTC_BHEADER_OPERATION_L2MCAST,
    CTC_BHEADER_OPERATION_L3MCAST,
    MAX_CTC_BHEADER_OPERATION_TYPE
};
typedef enum ctc_bheader_operation_type_e ctc_bheader_operation_type_t;

/**
 @brief Define bheader transmit flag
*/
enum ctc_bheader_tx_flag_e
{
    CTC_BHEADER_ENCAP_NEXTHOP_FLAG     = (1U << 1),  /**< Flag to indicate nexthop is set */
    CTC_BHEADER_ENCAP_OPT_TYPE_FLAG    = (1U << 2),  /**< Flag to indicate operation type is set */
    CTC_BHEADER_ENCAP_HASH_FLAG        = (1U << 3),  /**< Flag to indicate hash is set */
    CTC_BHEADER_ENCAP_SRC_SVID_FLAG    = (1U << 4),  /**< Flag to indicate source svlan id is set */
    CTC_BHEADER_ENCAP_SRC_CVID_FLAG    = (1U << 5),  /**< Flag to indicate source cvlan id is set */
    CTC_BHEADER_ENCAP_SRC_COS_FLAG     = (1U << 6),  /**< Flag to indicate source cos is set */
    CTC_BHEADER_ENCAP_TTL_FLAG         = (1U << 7),  /**< Flag to indicate ttl is set */
    CTC_BHEADER_ENCAP_PKT_TYPE_FLAG    = (1U << 8),  /**< Flag to indicate packet type is set */
    CTC_BHEADER_ENCAP_PRIORITY_FLAG    = (1U << 9),  /**< Flag to indicate priority is set */
    CTC_BHEADER_ENCAP_COLOR_FLAG       = (1U << 10), /**< Flag to indicate color is set */
    CTC_BHEADER_ENCAP_TIMESTAMP_FLAG   = (1U << 11), /**< Flag to indicate ptp is set */
    CTC_BHEADER_ENCAP_MCAST_FLAG       = (1U << 12), /**< Flag to indicate is multicast packet or not */
    CTC_BHEADER_ENCAP_CRITICAL_FLAG    = (1U << 13), /**< Flag to indicate packet is critical or not */
    CTC_BHEADER_ENCAP_NEXTHOP8W_FLAG   = (1U << 14)  /**< Flag to indicate whether use 8W nexthop */
};
typedef enum ctc_bheader_tx_flag_e ctc_bheader_tx_flag_t;

/**
 @brief Define bheader packet to cpu reason
*/
struct ctc_bheader_to_cpu_reason_s
{
    uint8 mod;                  /**< module */
    uint8 exp_index;            /**< exception index */
    union
    {
        uint8 exp_sub_index;    /**< exception sub index, if cpu destination is single cpu */
        uint8 cpu_mac;          /**< cpu mac, if cpu destination is multiple cpu */
    } sub;
    uint8 rsv0[2];
};
typedef struct ctc_bheader_to_cpu_reason_s ctc_bheader_to_cpu_reason_t;

/**
 @brief Define bheader receive packet info
*/
struct ctc_bheader_rx_info_s
{
    ctc_bheader_pkt_type_t pkt_type;    /**< packet type */
    ctc_bheader_to_cpu_reason_t reason; /**< the reason of packet to cpu */
    uint64 seconds;     /**< time stamp second value, 1588v1 timestamp low 32bit valid, 1588v2 timestamp low 48bit valid*/
    uint32 nanoseconds; /**< time stamp nanosecond value */
    uint32 src_gport;                   /**< source port */
    uint16 vrfid;                       /**< vrfid or fid */
    uint16 svlan_id;                    /**< source svlan ID */
    uint16 cvlan_id;                    /**< source cvlan ID */
    uint8  svlanid_valid;               /**< source svlanId valid */
    uint8  cvlanid_valid;               /**< source cvlanId valid */
    uint8  src_cos;                     /**< source cos */
    uint8  priority;                    /**< priority <0-63> */
    uint8  color;                       /**< color <1-3> */
    uint8  rsv[3];
    void*  data;                         /**< packet data */
    uint16 data_len;                     /**< data length */
};
typedef struct ctc_bheader_rx_info_s ctc_bheader_rx_info_t;

/**
 @brief Define bheader transmit packet info
*/
struct ctc_bheader_tx_info_s
{
    uint32 flag;                                /**< CTC_PKT_DRV_ENCAP_XXX */
    ctc_bheader_nexthop_type_t nh_type;         /**< nexthop type */
    ctc_bheader_operation_type_t operation_type; /**< operation type */
    ctc_bheader_pkt_type_t  pkt_type;           /**< packet type */
    uint32 nh_offset;                           /**< Ds nexthop offset in global dynamic sram */
    uint32 hash;                                /**< hash used for linkagg */
    uint32 src_gport;                           /**< global source port*/
    uint16 destid;                              /**< destination local portid(queue id(8bit) + local port id(8bit)) or dest mcast group id */
    uint16 svlan_id;                            /**< source svlan ID */
    uint16 cvlan_id;                            /**< source cvlan ID */
    uint8  dest_chipid;                         /**< destination chip id */
    uint8  src_cos;                             /**< source cos */
    uint8  ttl;                                 /**< TTL */
    uint8  priority;                            /**< priority <0-63> */
    uint8  color;                               /**< color <1-3> */
    uint8  ts_offset;                           /**< time stamp offset */
    uint8  ts_version;                          /**< time stamp version */
    uint8  ts_replace;                          /**< time stamp replace or not, if 1 replace*/
    void*  data;                                /**< packet data */
    uint16 data_len;                            /**< data length */

};
typedef struct ctc_bheader_tx_info_s ctc_bheader_tx_info_t;

/**@} end of @defgroup bheader BHEADER */

#ifdef __cplusplus
}
#endif

#endif /*_CTC_BHEADER_H*/

