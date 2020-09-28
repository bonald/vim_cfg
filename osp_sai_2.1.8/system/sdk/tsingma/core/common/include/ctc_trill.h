/**
 @file ctc_trill.h

 @author  Copyright(C) 2005-2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-25

 @version v3.0

   This file contains trill related data structure, enum, macro and proto.
*/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup trill TRILL
 @{
*/

#ifndef _CTC_TRILL_H
#define _CTC_TRILL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common/include/ctc_const.h"
#include "common/include/ctc_common.h"

/**
 @brief define flags for trill route
*/
enum ctc_trill_route_flag_e
{
    CTC_TRILL_ROUTE_FLAG_DISCARD                 = 0x00000001,          /**< [GG.D2.TM] Indicates discard the packet*/
    CTC_TRILL_ROUTE_FLAG_MCAST                   = 0x00000002,           /**< [GG.D2.TM] Indicates mcast*/
    CTC_TRILL_ROUTE_FLAG_SRC_PORT_CHECK          = 0x00000004,   /**< [GG.D2.TM] Source gport check*/
    CTC_TRILL_ROUTE_FLAG_MACSA_CHECK             = 0x00000008,     /**< [GG.D2.TM] Outer MACSA check*/
    CTC_TRILL_ROUTE_FLAG_DEFAULT_ENTRY           = 0x00000010,      /**< [GG.D2.TM] This trill route is default entry*/
    CTC_TRILL_ROUTE_FLAG_MAX
};
typedef enum ctc_trill_route_flag_e ctc_trill_route_flag_t;


/**
 @brief  Define trill route structure
*/
struct ctc_trill_route_s
{
    uint32 flag;                                    /**< [GG.D2.TM] Refer to ctc_trill_route_flag_t*/

    uint16 egress_nickname;                /**< [GG.D2.TM] Egress nickname*/
    uint16 ingress_nickname;               /**< [GG.D2.TM] Ingress nickname*/

    uint16  vlan_id;                              /**< [GG.D2.TM] Inner vlan ID*/
    uint8 rsv[2];

    uint32 nh_id;                                  /**< [GG.D2.TM] Nexthop ID*/
    uint32 src_gport;                            /**< [GG.D2.TM] Source gport*/

    mac_addr_t  mac_sa;                      /**< [GG.D2.TM] Outer MACSA*/
};
typedef struct ctc_trill_route_s ctc_trill_route_t;


/**
 @brief define flags for trill tunnel
*/
enum ctc_trill_tunnel_flag_e
{
    CTC_TRILL_TUNNEL_FLAG_MCAST                  = 0x00000001, /**< [GG.D2.TM] Indicates mcast tunnel*/
    CTC_TRILL_TUNNEL_FLAG_ARP_ACTION             = 0x00000002, /**< [GG.D2.TM] arp packet action type, value see enum ctc_exception_type_t */
    CTC_TRILL_TUNNEL_FLAG_DEFAULT_ENTRY          = 0x00000004, /**< [GG.D2.TM] This trill tunnel is default entry */
    CTC_TRILL_TUNNEL_FLAG_DENY_LEARNING          = 0x00000008, /**< [GG.D2.TM] Deny inner learning after decap*/
    CTC_TRILL_TUNNEL_FLAG_SERVICE_ACL_EN         = 0x00000010, /**< [GG.D2.TM] enable service acl in trill tunnel decap */
    CTC_TRILL_TUNNEL_FLAG_MAX
};
typedef enum ctc_trill_tunnel_flag_e ctc_trill_tunnel_flag_t;

/**
 @brief  Define trill tunnel structure
*/
struct ctc_trill_tunnel_s
{
    uint32 flag;                          /**< [GG.D2.TM] Refer to ctc_trill_tunnel_flag_t*/

    uint16 egress_nickname;      /**< [GG.D2.TM] Egress nickname*/
    uint16 ingress_nickname;     /**< [GG.D2.TM] Ingress nickname*/

    uint32 nh_id;                       /**< [GG.D2.TM] Nexthop for decap, packet will go out directly by this nexthop */

    uint16  fid;                          /**< [GG.D2.TM] Forwarding instance after decap*/
    uint16 logic_src_port;          /**< [GG.D2.TM] Logic source port for the TRILL tunnel */

    uint16 stats_id;                   /**< [GG.D2.TM] Stats id for the tunnel */
    uint8 rsv[2];

    mac_addr_t route_mac;        /* [GG.D2.TM] Route mac for inner packet, 0.0.0 means disable*/
    ctc_exception_type_t arp_action;   /**< [GG.D2.TM] arp packet action type */
};
typedef struct ctc_trill_tunnel_s ctc_trill_tunnel_t;

/**@} end of @defgroup  trill TRILL */
#ifdef __cplusplus
}
#endif

#endif

