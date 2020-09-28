/**
 @file ctc_security.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

   This file contains all security related data structure, enum, macro and proto.
*/

#ifndef _CTC_SECURITY_H_
#define _CTC_SECURITY_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"

/**
 @defgroup security SECURITY
 @{
*/
#define CTC_SECURITY_STORMCTL_EVENT_MAX_NUM   32   /**< [TM] The max num of stormctl event for isr call back*/
/**
 @brief Security mac limit action type
*/
enum ctc_maclimit_action_e
{
    CTC_MACLIMIT_ACTION_NONE,     /**< [GB.GG.D2.TM] Packet will forwarding and learning mac if reach mac learning limitation*/
    CTC_MACLIMIT_ACTION_FWD,      /**< [GB.GG.D2.TM] Packet will forwarding but no learning mac if reach mac learning limitation*/
    CTC_MACLIMIT_ACTION_DISCARD,  /**< [GB.GG.D2.TM] Packet will discard and no learning mac if reach mac learning limitation*/
    CTC_MACLIMIT_ACTION_TOCPU,     /**< [GB.GG.D2.TM] Packet will discard and no learning, redirect to cpu if reach mac learning limitation*/
    CTC_MACLIMIT_ACTION_TOCPU_NODISC /*SYSTEM MODIFY, Added by xgu for SDK bug 282, 2013-1-15*/
};
typedef enum ctc_maclimit_action_e ctc_maclimit_action_t;

/**
 @brief Security ip source guard type
*/
enum ctc_security_ip_source_guard_type_e
{
    CTC_SECURITY_BINDING_TYPE_IP,           /**< [GB.GG.D2.TM] Check item ip*/
    CTC_SECURITY_BINDING_TYPE_IP_VLAN,      /**< [GB.GG.D2.TM] Check item ip and vlan*/
    CTC_SECURITY_BINDING_TYPE_IP_MAC,       /**< [GB.GG.D2.TM] Check item ip and mac*/
    CTC_SECURITY_BINDING_TYPE_IPV6_MAC,     /**< [GB.GG.D2.TM] Check item ipv6 and mac*/
    CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN,  /**< [GB.GG.D2.TM] Check item ip and mac and vlan*/
    CTC_SECURITY_BINDING_TYPE_MAC,          /**< [GB.GG.D2.TM] Check item mac*/
    CTC_SECURITY_BINDING_TYPE_MAC_VLAN,     /**< [GB.GG.D2.TM] Check item mac and vlan*/
    CTC_SECURITY_BINDING_TYPE_MAX
};
typedef enum ctc_security_ip_source_guard_type_e ctc_security_ip_source_guard_type_t;

/**
 @brief Security storm control operation type
*/
enum ctc_security_storm_ctl_op_e
{
    CTC_SECURITY_STORM_CTL_OP_PORT, /**< [GB.GG.D2.TM] Port based storm control*/
    CTC_SECURITY_STORM_CTL_OP_VLAN, /**< [GB.GG.D2.TM] Vlan based storm control*/
    CTC_SECURITY_STORM_CTL_OP_MAX
};
typedef enum ctc_security_storm_ctl_op_e ctc_security_storm_ctl_op_t;

/**
 @brief Security storm control type
*/
enum ctc_security_storm_ctl_type_e
{
    CTC_SECURITY_STORM_CTL_BCAST,          /**< [GB.GG.D2.TM] Broadcast storm control*/
    CTC_SECURITY_STORM_CTL_KNOWN_MCAST,    /**< [GB.GG.D2.TM] Known multicast storm control*/
    CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST,  /**< [GB.GG.D2.TM] Unknown multicast storm control*/
    CTC_SECURITY_STORM_CTL_ALL_MCAST,      /**< [GB.GG.D2.TM] Known and unknown multicast storm control*/
    CTC_SECURITY_STORM_CTL_KNOWN_UCAST,    /**< [GB.GG] Known unicast storm control*/
    CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST,  /**< [GB.GG.D2.TM] Unknown unicast storm control*/
    CTC_SECURITY_STORM_CTL_ALL_UCAST,      /**< [GB.GG] Known and unknown unicast storm control*/
    CTC_SECURITY_STORM_CTL_MAX
};
typedef enum ctc_security_storm_ctl_type_e ctc_security_storm_ctl_type_t;

/**
 @brief Security storm control mode
*/
enum ctc_security_storm_ctl_mode_e
{
    CTC_SECURITY_STORM_CTL_MODE_PPS,    /**< [GB.GG.D2.TM] Packet per second*/
    CTC_SECURITY_STORM_CTL_MODE_BPS,    /**< [GB.GG.D2.TM] Bytes per second*/
    CTC_SECURITY_STORM_CTL_MODE_KPPS,   /**< [GG.D2.TM] K Packet per second*/
    CTC_SECURITY_STORM_CTL_MODE_KBPS,   /**< [GG.D2.TM] K Bytes per second*/
    CTC_SECURITY_STORM_CTL_MODE_MAX
};
typedef enum ctc_security_storm_ctl_mode_e ctc_security_storm_ctl_mode_t;

/**
 @brief Security storm control granularity
*/
enum ctc_security_storm_ctl_granularity_e
{
    CTC_SECURITY_STORM_CTL_GRANU_100,    /**< [GB.GG] Granularity is 100   bps or pps*/
    CTC_SECURITY_STORM_CTL_GRANU_1000,   /**< [GB.GG] Granularity is 1000  bps or pps*/
    CTC_SECURITY_STORM_CTL_GRANU_10000,  /**< [GB] Granularity is 10000 bps or pps*/
    CTC_SECURITY_STORM_CTL_GRANU_5000,   /**< [GG] Granularity is 5000 bps or pps*/

    CTC_SECURITY_STORM_CTL_GRANU_MAX
};
typedef enum ctc_security_storm_ctl_granularity_e ctc_security_storm_ctl_granularity_t;

/**
 @brief Secuirty learn limit type
*/
enum ctc_security_learn_limit_type_e
{
    CTC_SECURITY_LEARN_LIMIT_TYPE_PORT,       /**< [GB.GG.D2.TM] Learn limit for port*/
    CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN,       /**< [GB.GG.D2.TM] Learn limit for vlan*/
    CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM,     /**< [GB.GG.D2.TM] Learn limit for system*/

    CTC_SECURITY_LEARN_LIMIT_TYPE_MAX
};
typedef enum ctc_security_learn_limit_type_e ctc_security_learn_limit_type_t;


/**
 @brief Secuirty data structure for learn limit
*/
struct ctc_security_learn_limit_s
{
    ctc_security_learn_limit_type_t  limit_type;        /**< [GB.GG.D2.TM] Learn limit type must be ctc_security_learn_limit_type_t*/

    uint32 gport;                                       /**< [GB.GG.D2.TM] Global source port when limit_type is CTC_SECURITY_LEARN_LIMIT_TYPE_PORT*/
    uint16 vlan;                                        /**< [GB.GG.D2.TM] Vlan ID when limit_type is CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN*/
    uint32 limit_count;                                 /**< [GB.GG.D2.TM] mac limit count*/
    uint32 limit_num;                                   /**< [GB.GG.D2.TM] mac limit threshold, max num means disable mac limit*/
    uint32 limit_action;                                /**< [GB.GG.D2.TM] action when reach threshold, refer to ctc_maclimit_action_t*/
};
typedef struct ctc_security_learn_limit_s ctc_security_learn_limit_t;


/**
 @brief enum value used in ip source guard flag APIs
*/
enum ctc_security_ip_source_guard_flag_e
{
    CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX      = 0x01,               /**< [D2.TM] if set, use tcam to resolve conflict*/
    CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT   = 0x02,               /**< [D2.TM] if set, ip source guard key use logic port as port*/
    CTC_SECURITY_IP_SOURCE_GUARD_FLAG_MAX
};
typedef enum ctc_security_ip_source_guard_flag_e ctc_security_ip_source_guard_flag_t;


/**
 @brief Security data structure for ip source guard
*/
struct ctc_security_ip_source_guard_elem_s
{
    uint32 flag;                                            /**< [D2.TM] Supported ip source guard bitmap, see ctc_security_ip_source_guard_flag_t */
    ctc_security_ip_source_guard_type_t ip_src_guard_type;  /**< [GB.GG.D2.TM] Ip source guard type referred to ctc_security_ip_source_guard_type_t*/
    mac_addr_t mac_sa;                                      /**< [GB.GG.D2.TM] Source mac address*/
    ipv6_addr_t ipv6_sa;                                    /**< [GB.GG.D2.TM] Source ipv6 address*/
    ip_addr_t ipv4_sa;                                      /**< [GB.GG.D2.TM] Source ipv4 address*/
    uint16 vid;                                             /**< [GB.GG.D2.TM] Vlan id*/
    uint32 gport;                                           /**< [GB.GG.D2.TM] Global port, if ip_src_guard_type is CTC_SECURITY_BINDING_TYPE_IPV6_MAC,
                                                                               gport can not be used to be binding*/
    uint8 ipv4_or_ipv6;                                     /**< [GB.GG.D2.TM] Judge use ipv4 or ipv6*/
    uint8 scl_id;                            /**< [D2.TM] TCAM  SCL lookup ID.There are 2 scl lookup<0-1>, and each has its own feature */
    uint8 rsv[3];
};
typedef struct ctc_security_ip_source_guard_elem_s ctc_security_ip_source_guard_elem_t;

/**
 @brief Secuirty data structure for storm control per port
*/
struct ctc_security_stmctl_cfg_s
{
    ctc_security_storm_ctl_type_t type;  /**< [GB.GG.D2.TM] Type for storm control*/
    ctc_security_storm_ctl_mode_t mode;  /**< [GB.GG.D2.TM] Mode for storm control*/
    uint32 gport;                        /**< [GB.GG.D2.TM] Port based  storm control*/
    uint8    storm_en;                   /**< [GB.GG.D2.TM] Storm control enable*/
    uint8    discarded_to_cpu;           /**< [GB.GG.D2.TM] Discarded packet by storm control and redirect to cpu*/
    uint8    rsv[2];
    uint32   threshold;                  /**< [GB.GG.D2.TM] Threshold for storm control, according to mode*/
    uint8    op;                         /**< [GB.GG.D2.TM] Base is vlan or port storm ctl, refer to ctc_security_storm_ctl_op_t*/
    uint8    rsv0;
    uint16  vlan_id;                     /**< [GB.GG.D2.TM] Vlan based  storm control*/

};
typedef struct ctc_security_stmctl_cfg_s ctc_security_stmctl_cfg_t;

/**
 @brief Secuirty data structure for storm control global
*/
struct ctc_security_stmctl_glb_cfg_s
{
    uint8 ipg_en;           /**< [GB.GG.D2.TM] Ipg is considered in storm control operation*/
    uint8 ustorm_ctl_mode;  /**< [GB.GG] If set, known and unknown unicast storm is separated*/
    uint8 mstorm_ctl_mode;  /**< [GB.GG.D2.TM] If set, known and unknown multicast storm is separated*/
    uint8 rsv;
    uint16 granularity;     /**< [GB.GG] Granularity for storm control, ctc_security_storm_ctl_granularity_t*/
};
typedef struct ctc_security_stmctl_glb_cfg_s ctc_security_stmctl_glb_cfg_t;

struct ctc_security_stormctl_event_s
{
    uint8 valid_num;                              /**< [TM] Indicated valid event num */
    uint8 rsv[3];
    ctc_security_stmctl_cfg_t  stormctl_state[CTC_SECURITY_STORMCTL_EVENT_MAX_NUM ];       /**< [TM]  stormctl state*/
};
typedef struct ctc_security_stormctl_event_s ctc_security_stormctl_event_t;



/**@} end of @defgroup  security SECURITY */

#ifdef __cplusplus
}
#endif

#endif

