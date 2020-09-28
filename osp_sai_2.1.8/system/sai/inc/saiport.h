/*
* Copyright (c) 2014 Microsoft Open Technologies, Inc.
*
*    Licensed under the Apache License, Version 2.0 (the "License"); you may
*    not use this file except in compliance with the License. You may obtain
*    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
*    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
*    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
*    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
*    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
*    See the Apache Version 2.0 License for specific language governing
*    permissions and limitations under the License.
*
*    Microsoft would like to thank the following companies for their review and
*    assistance with these files: Intel Corporation, Mellanox Technologies Ltd,
*    Dell Products, L.P., Facebook, Inc
*
* Module Name:
*
*    saiport.h
*
* Abstract:
*
*    This module defines SAI Port API
*
*/

#if !defined (__SAIPORT_H_)
#define __SAIPORT_H_

#include <saitypes.h>

/** \defgroup SAIPORT SAI - Port specific API definitions.
 *
 *  \{
 */

/**
 * @brief Enum defining vlan mapping types.
 */
typedef enum _sai_vlan_mapping_type_t
{
    /** none  */
    SAI_VLAN_MAP_TYPE_NONE,

    /** basic qinq */
	SAI_VLAN_MAP_TYPE_BASIC_QINQ,

    /** selective qinq */
	SAI_VLAN_MAP_TYPE_SELECTIVE_QINQ,

    /** replace for vlan translation */
	SAI_VLAN_MAP_TYPE_VLAN_REPLACE,

} sai_vlan_mapping_type_t;

/**
 * @brief Enum defining vlan mapping types.
 */
typedef enum _sai_port_untag_pvid_type_t
{
    /** un tag-striped svlan id */
    SAI_PORT_UNTAG_SVLAN,

    /** tag-striped svlan id */
	SAI_PORT_TAG_SVLAN,

    /** un tag-striped pvid id */
	SAI_PORT_UNTAG_PVID,

    /** tag-striped pvid id */
	SAI_PORT_TAG_PVID,

} sai_port_untag_pvid_type_t;

/**
 *  @brief Attribute data for SAI_PORT_ATTR_TYPE
 */
typedef enum _sai_port_type_t
{
    /** Actual port. N.B. Different from the physical port. */
    SAI_PORT_TYPE_LOGICAL,

    /** CPU Port */
    SAI_PORT_TYPE_CPU,

} sai_port_type_t;

/*added by hansf 2017/09/13*/
typedef enum _sai_port_qos_trust_t
{
    /** qos trust port */
    SAI_PORT_QOS_TRUST_PORT,

    /** qos trust cos */
    SAI_PORT_QOS_TRUST_COS,

    /** qos trust dscp */
    SAI_PORT_QOS_TRUST_DSCP,
    
} sai_port_qos_trust_t;
/*ended by hansf*/

/**
 *  @brief Attribute data for SAI_PORT_ATTR_OPER_STATUS
 */
typedef enum _sai_port_oper_status_t
{
    /** Unknown */
    SAI_PORT_OPER_STATUS_UNKNOWN,

    /** Up */
    SAI_PORT_OPER_STATUS_UP,

    /** Down */
    SAI_PORT_OPER_STATUS_DOWN,

    /** Test Running */
    SAI_PORT_OPER_STATUS_TESTING,

    /** Not Present */
    SAI_PORT_OPER_STATUS_NOT_PRESENT

} sai_port_oper_status_t;

/**
 * @brief Defines the operational status of the port
 */
typedef struct _sai_port_oper_status_notification_t {

    /** Port id */
    sai_object_id_t port_id;

    /** Port operational status */
    sai_port_oper_status_t port_state;

} sai_port_oper_status_notification_t;

/**
 * @brief Attribute data for SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL
 */
typedef enum _sai_port_flow_control_mode_t
{
    /** disable flow control for both tx and rx */
    SAI_PORT_FLOW_CONTROL_DISABLE,

    /** enable flow control for tx only */
    SAI_PORT_FLOW_CONTROL_TX_ONLY,

    /** enable flow control for rx only */
    SAI_PORT_FLOW_CONTROL_RX_ONLY,

    /** enable flow control for both tx and rx */
    SAI_PORT_FLOW_CONTROL_BOTH_ENABLE,

} sai_port_flow_control_mode_t;

/**
 * @brief Attribute data for SAI_PORT_ATTR_INTERNAL_LOOPBACK
 */
typedef enum _sai_port_internal_loopback_mode_t
{
    /** disable internal loopback */
    SAI_PORT_INTERNAL_LOOPBACK_NONE,

    /** port internal loopback at phy module */
    SAI_PORT_INTERNAL_LOOPBACK_PHY,

    /** port internal loopback at mac module */
    SAI_PORT_INTERNAL_LOOPBACK_MAC

} sai_port_internal_loopback_mode_t;

/**
 * @brief Attribute data for SAI_PORT_ATTR_FDB_LEARNING
 */
typedef enum _sai_port_fdb_learning_mode_t
{
    /** Drop packets with unknown source MAC. Do not learn. Do not forward */
    SAI_PORT_LEARN_MODE_DROP,

    /** Do not learn unknown source MAC. Forward based on destination MAC */
    SAI_PORT_LEARN_MODE_DISABLE,

    /** Hardware learning. Learn source MAC. Forward based on destination MAC */
    SAI_PORT_LEARN_MODE_HW,

    /** Trap packets with unknown source MAC to CPU. Do not learn. Do not forward */
    SAI_PORT_LEARN_MODE_CPU_TRAP,

    /** Trap packets with unknown source MAC to CPU. Do not learn. Forward based on destination MAC */
    SAI_PORT_LEARN_MODE_CPU_LOG,

} sai_port_fdb_learning_mode_t;

/**
 * @brief Port Add/Delete Event
 */
typedef enum _sai_port_event_t
{
    /** Create a new active port */
    SAI_PORT_EVENT_ADD,

    /** Delete/Invalidate an existing port */
    SAI_PORT_EVENT_DELETE,

} sai_port_event_t;

/**
 * @brief Defines the port event notification
 */
typedef struct _sai_port_event_notification_t {

    /** Port id */
    sai_object_id_t port_id;

    /** Port event */
    sai_port_event_t port_event;

} sai_port_event_notification_t;

/**
 * @brief Attribute data for SAI_PORT_ATTR_MEDIA_TYPE
 */
typedef enum _sai_port_media_type_t
{
    /** Media not present */
    SAI_PORT_MEDIA_TYPE_NOT_PRESENT,

    /** Media type not known */
    SAI_PORT_MEDIA_TYPE_UNKNONWN,

    /** Media type QSFP fiber optic */
    SAI_PORT_MEDIA_TYPE_QSFP_FIBER,

    /** Media type QSFP copper optic */
    SAI_PORT_MEDIA_TYPE_QSFP_COPPER,

    /** Media type SFP fiber optic */
    SAI_PORT_MEDIA_TYPE_SFP_FIBER,

    /** Media type SFP copper optic */
    SAI_PORT_MEDIA_TYPE_SFP_COPPER,
} sai_port_media_type_t;

/* added by kcao for SAI_EXT_ISOLATION 2015-12-01 */
/**
 * @brief Attribute data for SAI_PORT_ATTR_ISOLATION_DIRECTION
 */
typedef enum _sai_port_isolation_direction_t
{
    /** enable port isolation on both direction including ingress and egress*/
    SAI_PORT_ISOLATION_DIRECTION_BOTH,
    
    /** enable port isolation direction ingress*/
    SAI_PORT_ISOLATION_DIRECTION_INGRESS,

    /** enable port isolation direction egress*/
    SAI_PORT_ISOLATION_DIRECTION_EGRESS,

} sai_port_isolation_direction_t;

/* added by wangjj for SAI_PORT_ATTR_IP_SOURCE_GUARD 2016-03-16 */
/**
 * @brief Attribute data for SAI_PORT_ATTR_IP_SOURCE_GUARD
 */
typedef enum _sai_port_ip_source_guard_t
{
    /** enable port isolation on both direction including ingress and egress*/
    SAI_PORT_IPSG_IP,
    
    /** enable port isolation direction ingress*/
    SAI_PORT_IPSG_MAC,

    /** enable port isolation direction egress*/
    SAI_PORT_IPSG_VLAN,

} sai_port_ip_source_guard_t;

/* added by yejl for SAI_PORT_ATTR_ISOLATION_TYPE 2016-08-02 */
/**
 * @brief Attribute data for SAI_PORT_ATTR_ISOLATION_TYPE
 */
typedef enum _sai_port_isolation_type_t
{
    /** port isolation type*/
    SAI_PORT_ISOLATION_TYPE_DEFAULT = 0,
    
    /** private Vlan type: isolate*/
    SAI_PORT_ISOLATION_PVLAN_TYPE_ISOLATE,
    
    /** private Vlan type: promiscuous*/
    SAI_PORT_ISOLATION_PVLAN_TYPE_PROMISCUOUS,

    /** private Vlan type: community*/
    SAI_PORT_ISOLATION_PVLAN_TYPE_COMMUNITY,

} sai_port_isolation_type_t;

/* added by yejl for SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE 2016-10-10 */
/**
 * @brief Attribute data for SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE
 */
typedef enum _sai_port_vlanclass_active_type_t
{
    /** vlanclass active type*/
    SAI_PORT_VLANCLASS_ACTIVE_TYPE_NONE = 0,

    /** vlanclass active: mac*/
    SAI_PORT_VLANCLASS_ACTIVE_TYPE_MAC,
    
    /** vlanclass active: ipv4*/
    SAI_PORT_VLANCLASS_ACTIVE_TYPE_IP,
    
    /** vlanclass active: protocol*/
    SAI_PORT_VLANCLASS_ACTIVE_TYPE_PROTOCOL,

    SAI_PORT_VLANCLASS_ACTIVE_TYPE_MAX,

} sai_port_vlanclass_active_type_t;
/**
 *  Attribute Id in sai_set_port_attribute() and
 *  sai_get_port_attribute() calls
 */
typedef enum _sai_port_attr_t
{
    /** READ-ONLY */

    /** Port Type [sai_port_type_t] */
    SAI_PORT_ATTR_TYPE,

    /** Operational Status [sai_port_oper_status_t] */
    SAI_PORT_ATTR_OPER_STATUS,

    /** Hardware Lane list [sai_u32_list_t] */
    SAI_PORT_ATTR_HW_LANE_LIST,

    /** Breakout mode(s) supported [sai_s32_list_t] */
    SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE,

    /** Current breakout mode [sai_port_breakout_mode_type_t] */
    SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE,

    /** Number of queues on port [uint32_t]*/
    SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES,

    /** List of Queues for the port[sai_object_list_t] */
    SAI_PORT_ATTR_QOS_QUEUE_LIST,

    /** Number of Scheduler groups on port [uint32_t]*/
    SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS,

    /** List of Scheduler groups for the port[sai_object_list_t] */
    SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST,

    /** Query list of supported port speed in Mbps [sai_u32_list_t] */
    SAI_PORT_ATTR_SUPPORTED_SPEED,

    /** Number of ingress priority groups [sai_uint32_t] */
    SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS,

    /** list of ingress priority groups [sai_object_list_t] */
    SAI_PORT_ATTR_PRIORITY_GROUP_LIST,

    /** READ-WRITE */
    /** Speed in Mbps [uint32_t] */
    SAI_PORT_ATTR_SPEED,

    /** Full Duplex setting [bool] (default to TRUE) */
    SAI_PORT_ATTR_FULL_DUPLEX_MODE,

    /** Auto Negotiation configuration [bool] (default to FALSE) */
    SAI_PORT_ATTR_AUTO_NEG_MODE,

    /** Admin Mode [bool], (default to FALSE)*/
    SAI_PORT_ATTR_ADMIN_STATE,

    /** Media Type [sai_port_media_type_t],
     * (default to SAI_PORT_MEDIA_TYPE_NOT_PRESENT) */
    SAI_PORT_ATTR_MEDIA_TYPE,

    /** Port VLAN ID [sai_vlan_id_t]
     * Untagged ingress frames are tagged with Port VLAN ID (PVID)
       (default set to 1) */
    SAI_PORT_ATTR_PORT_VLAN_ID,

    /** Default VLAN Priority [uint8_t]
       (default to 0) */
    SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY,

    /** Ingress Filtering (Drop Frames with Unknown VLANs) [bool]
       (default to FALSE) */
    SAI_PORT_ATTR_INGRESS_FILTERING,

    /** Dropping of untagged frames on ingress [bool]
       (default to FALSE) */
    SAI_PORT_ATTR_DROP_UNTAGGED,

    /** Dropping of tagged frames on ingress [bool]
       (default to FALSE) */
    SAI_PORT_ATTR_DROP_TAGGED,

    /** Internal loopback control [sai_port_internal_loopback_mode_t]
       (default to SAI_PORT_INTERNAL_LOOPBACK_NONE) */
    SAI_PORT_ATTR_INTERNAL_LOOPBACK,

    /** FDB Learning mode [sai_port_fdb_learning_mode_t]
       (default to SAI_PORT_LEARN_MODE_HW) */
    SAI_PORT_ATTR_FDB_LEARNING,

    /** Update DSCP of outgoing packets [bool]
       (default to FALSE) */
    SAI_PORT_ATTR_UPDATE_DSCP,

    /** MTU [uint32_t]
       (default to 1514 bytes*/
    SAI_PORT_ATTR_MTU,

    /** Enable flood (unknown unicast or unknown multicast)
       storm control policer on port [sai_object_id_t]
       Policer id = SAI_NULL_OBJECT_ID to disable policer on port */
    SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID,

    /** Enable broadcast storm control policer on port [sai_object_id_t]
       Policer id = SAI_NULL_OBJECT_ID to disable policer on port */
    SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID,

    /** Enable multicast storm control policer on port [sai_object_id_t]
       Policer id = SAI_NULL_OBJECT_ID to disable policer on port */
    SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID,

    /** [sai_port_flow_control_mode_t]
       (default to SAI_PORT_FLOW_CONTROL_DISABLE) */
    SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL,

    /** Maximum number of learned MAC addresses [uint32_t] */
    SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES,

    /** Action for packets with unknown source mac address
     * when FDB learning limit is reached.
     * [sai_packet_action_t] (default to SAI_PACKET_ACTION_DROP) */
    SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION,

    /** Enable/Disable Mirror session [sai_object_list_t].
     * Enable ingress mirroring by assigning list of mirror session
     * object id as attribute value
     * Disable ingress mirroring by assigning object_count as 0 in objlist */
    SAI_PORT_ATTR_INGRESS_MIRROR_SESSION,

    /** Enable/Disable Mirror session [sai_object_list_t].
     * Enable egress mirroring by assigning list of mirror session
     * object id as attribute value
     * Disable egress mirroring by assigning object_count as 0 in objlist */
    SAI_PORT_ATTR_EGRESS_MIRROR_SESSION,

    /** Enable/Disable Samplepacket session [sai_object_id_t].
     * Enable ingress sampling by assigning samplepacket object id
     * Disable ingress sampling by assigning SAI_NULL_OBJECT_ID as
     * attribute value */
    SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE,

    /** Enable/Disable Samplepacket session [sai_object_id_t].
     * Enable egress sampling by assigning samplepacket object id
     * Disable egress sampling by assigning SAI_NULL_OBJECT_ID as
     * attribute value */
    SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE,

    /** Attach/Detach policer to port [sai_object_id_t],
     * Policer id = SAI_NULL_OBJECT_ID to disable policer on port */
    SAI_PORT_ATTR_POLICER_ID,

    SAI_PORT_ATTR_EGRESS_POLICER_ID,

    /** Port default Traffic class Mapping */
    SAI_PORT_ATTR_QOS_DEFAULT_TC,

   /** Enable DOT1P -> TC MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust Dot1p, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP,

   /** Enable DOT1P -> COLOR MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust Dot1p, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP,

    /** Enable DOT1P -> TC AND COLOR MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust Dot1p, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,

    /** added by hansf 2017/09/12 */
    /** Enable EXP -> TC AND COLOR MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust Exp, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP,

   /** Enable DSCP -> TC MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust DSCP, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP,

   /** Enable DSCP -> COLOR MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust DSCP, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP,

    /** Enable DSCP -> TC AND COLOR MAP [sai_object_id_t] on port
    * MAP id = SAI_NULL_OBJECT_ID to disable map on port.
    * To enable/disable trust DSCP, Map ID should be add/remove on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP,

   /** Enable TC -> Queue MAP [sai_object_id_t]  on port
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map i.e All packets to queue 0 */
    SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP,

   /** Enable TC -> DOT1P MAP [sai_object_id_t]
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP,

    /** Enable TC AND COLOR -> DOT1P MAP [sai_object_id_t]
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,

    /** added by hansf 2017/09/12 */
    /** Enable TC AND COLOR -> EXP MAP [sai_object_id_t]
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP,

   /** Enable TC -> DSCP MAP [sai_object_id_t]
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP,

   /** Enable TC AND COLOR -> DSCP MAP [sai_object_id_t]
    * Map id = SAI_NULL_OBJECT_ID to disable map on port.
    * Default no map */
    SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,

    /** Enable TC -> Priority Group MAP [sai_object_id_t]
     * Map id = SAI_NULL_OBJECT_ID to disable map on port.
     * Default no map */
    SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,

    /** Enable Priority Group -> PFC Priority MAP [sai_object_id_t]
     * Map id = SAI_NULL_OBJECT_ID to disable map on port.
     * Default no map */
    SAI_PORT_ATTR_QOS_PRIORITY_GROUP_TO_PFC_PRIORITY_MAP,

    /** Enable PFC Priority -> Queue MAP [sai_object_id_t]
     * Map id = SAI_NULL_OBJECT_ID to disable map on port.
     * Default no map */
    SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,

    /** Attach WRED to port [sai_object_id_t]
     (mandatory when SAI_PORT_ATTR_QOS_DROP_TYPE =  SAI_QOS_DROP_TYPE_WRED) */
    SAI_PORT_ATTR_QOS_WRED_PROFILE_ID,

    /** Scheduler for port [sai_object_id_t],
     * Default no limits */
    SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID,

    /** Buffer profile for port [sai_object_id_t]*/
    SAI_PORT_ATTR_QOS_BUFFER_PROFILE_ID,

    /** bit vector enable/disable port PFC [sai_uint8_t].
     * Valid from bit 0 to bit 7 */
    SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL,

    /** User based Meta Data [sai_uint32_t]
     * Value Range SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE */
    SAI_PORT_ATTR_META_DATA,

    /** Egress block port list [sai_object_list_t]
     * Traffic ingressing on this port and egressing out of the ports in the
     * given port list will be dropped. */
    SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST,

    /* added by kcao for SAI_EXT_ISOLATION 2015-12-01 begin */
    /** Enable/Disable Port Isolation [sai_object_id_t].
     * Enable port isolation group
     * object id as isolation group id
     * Disable isolation by assigning object_id as 0 */
    SAI_PORT_ATTR_ISOLATION_GROUP,

    /** Set Isolation direction [sai_port_isolation_direction_t].
     * Set Isolation direction by assigning direction 
     * object id as attribute value
     * It is only valid when  SAI_PORT_ATTR_ISOLATION_GROUP is enabled*/
    SAI_PORT_ATTR_ISOLATION_DIRECTION,
    /* added by kcao for SAI_EXT_ISOLATION 2015-12-01 end */

    /* added by YEJL for Isolation Type 2016-07-11 */
    /* sai_port_isolation_type_t */
    SAI_PORT_ATTR_ISOLATION_TYPE,

    /* added by YEJL for Vlanclass Group ID 2016-10-10 */
    SAI_PORT_ATTR_VLANCLASS_GROUP_ID,
    
    /* added by YEJL for Vlanclass Active Type 2016-10-10 */
    /* sai_port_vlanclass_active_type_t */
    SAI_PORT_ATTR_VLANCLASS_ACTIVE_TYPE,
    
    /* added by YEJL for Vlanclass Default Action 2016-10-10 */
    SAI_PORT_ATTR_VLANCLASS_DEFAULT_ACTION,

    /** -- */
    /* added by wangjj for SAI_PORT_ATTR_IP_SOURCE_GUARD, 2016-04-01 */
    /* It is disable by default */
    SAI_PORT_ATTR_IP_SOURCE_GUARD,

    /** added by wangl for replace cos Update cos of outgoing packets [bool]
       (default to FALSE) */
    SAI_PORT_ATTR_UPDATE_COS,

    /* added by liul for dhcprelay excetion type 2016-11-11 */
    SAI_PORT_ATTR_DHCPRELAY_EXCEPTION_TYPE,

    /* added by huwx for egress acl port enable 2017-04-11 */
    SAI_PORT_ATTR_EGRESS_ACL_EN,

    /* support sflow by liwh for bug 43981, 2017-06-05 */
    SAI_PORT_ATTR_SFLOW_EN,
    
    /** Added by wangyb for monitor [sai_uint32_t] */
    SAI_PORT_ATTR_MONITOR_LATENCY_EN,
    
    /** Added by wangyb for monitor [sai_u32_list_t] */
    SAI_PORT_ATTR_MONITOR_LATENCY_THRESHOLD,

    /** Added by hansf for qos trust [sai_uint32_t] 2017/09/13*/
    SAI_PORT_ATTR_QOS_TRUST,

    /** Added by hansf for port default cos [sai_uint32_t] 2017/09/13*/
    SAI_PORT_ATTR_DEFAULT_COS,

    /* added by yu for vlan mapping 2018-06-01 */
    SAI_PORT_ATTR_VLANMAP_TYPE,

    /* added by yu for vlan mapping 2018-06-01 */
    SAI_PORT_ATTR_VLANMAP_DEFAULT_FWD,

    /* added by yu for vlan mapping 2018-06-01 */
    SAI_PORT_ATTR_UNTAG_PVID,

    /* added by yu for vlan range enable 2018-06-01 */
    SAI_PORT_ATTR_VLAN_RANGE_EN,
        
    /* added by shengx for untagged vlan enable 2018-07-24 */
    SAI_PORT_ATTR_UNTAGGED_VLAN,

    /* Custom range base value */
    SAI_PORT_ATTR_CUSTOM_RANGE_BASE  = 0x10000000,
    SAI_PORT_ATTR_CUSTOM_TAP_USED,
    SAI_PORT_ATTR_CUSTOM_METADATA,
    SAI_PORT_ATTR_CUSTOM_RELFECTIVE_EN,

    /** svlan tpid type [sai_uint32_t] */
    SAI_PORT_ATTR_CUSTOM_SVLAN_TPID,

    SAI_PORT_ATTR_CUSTOM_DOT1Q_ETHERTYPE,

    SAI_PORT_ATTR_HASH_VALUE_PROFILE_ID,
    
    SAI_PORT_ATTR_HASH_VALUE_AGG_HASH,

    SAI_PORT_ATTR_PORT_OAM_MAC,   /* system modified by liwh for bug 53010, 2019-07-29 */

    SAI_PORT_ATTR_PORT_CRC_ERROR_CHECK_EN,

    SAI_PORT_ATTR_PORT_CRC_RECALCULATION_EN,
} sai_port_attr_t;

/**
 *  @brief Port counter IDs in sai_get_port_stat_counters() call
 */
typedef enum _sai_port_stat_counter_t
{
    SAI_PORT_STAT_IF_IN_OCTETS,
    SAI_PORT_STAT_IF_IN_UCAST_PKTS,
    SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS,
    SAI_PORT_STAT_IF_IN_DISCARDS,
    SAI_PORT_STAT_IF_IN_ERRORS,
    SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS,
    SAI_PORT_STAT_IF_IN_BROADCAST_PKTS,
    SAI_PORT_STAT_IF_IN_MULTICAST_PKTS,
    SAI_PORT_STAT_IF_IN_VLAN_DISCARDS,
    SAI_PORT_STAT_IF_OUT_OCTETS,
    SAI_PORT_STAT_IF_OUT_UCAST_PKTS,
    SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS,
    SAI_PORT_STAT_IF_OUT_DISCARDS,
    SAI_PORT_STAT_IF_OUT_ERRORS,
    SAI_PORT_STAT_IF_OUT_QLEN,
    SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS,
    SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS,
    SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS,
    SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS,
    SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS,
    SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS,
    SAI_PORT_STAT_ETHER_STATS_FRAGMENTS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS,
    SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS,
    SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS,
    SAI_PORT_STAT_ETHER_STATS_JABBERS,
    SAI_PORT_STAT_ETHER_STATS_OCTETS,
    SAI_PORT_STAT_ETHER_STATS_PKTS,
    SAI_PORT_STAT_ETHER_STATS_COLLISIONS,
    SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS,
    SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS,
    SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS,
    SAI_PORT_STAT_IP_IN_RECEIVES,
    SAI_PORT_STAT_IP_IN_OCTETS,
    SAI_PORT_STAT_IP_IN_UCAST_PKTS,
    SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS,
    SAI_PORT_STAT_IP_IN_DISCARDS,
    SAI_PORT_STAT_IP_OUT_OCTETS,
    SAI_PORT_STAT_IP_OUT_UCAST_PKTS,
    SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS,
    SAI_PORT_STAT_IP_OUT_DISCARDS,
    SAI_PORT_STAT_IPV6_IN_RECEIVES,
    SAI_PORT_STAT_IPV6_IN_OCTETS,
    SAI_PORT_STAT_IPV6_IN_UCAST_PKTS,
    SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS,
    SAI_PORT_STAT_IPV6_IN_MCAST_PKTS,
    SAI_PORT_STAT_IPV6_IN_DISCARDS,
    SAI_PORT_STAT_IPV6_OUT_OCTETS,
    SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS,
    SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS,
    SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS,
    SAI_PORT_STAT_IPV6_OUT_DISCARDS,
    /** get/set WRED green packet count [uint64_t] */
    SAI_PORT_STAT_GREEN_DISCARD_DROPPED_PACKETS,

    /** get/set WRED green byte count [uint64_t] */
    SAI_PORT_STAT_GREEN_DISCARD_DROPPED_BYTES,

    /** get/set WRED yellow packet count [uint64_t] */
    SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_PACKETS,

    /** get/set WRED yellow byte count [uint64_t] */
    SAI_PORT_STAT_YELLOW_DISCARD_DROPPED_BYTES,

    /** get/set WRED red packet count [uint64_t] */
    SAI_PORT_STAT_RED_DISCARD_DROPPED_PACKETS,

    /** get/set WRED red byte count [uint64_t] */
    SAI_PORT_STAT_RED_DISCARD_DROPPED_BYTES,

    /** get/set WRED dropped packets count [uint64_t] */
    SAI_PORT_STAT_DISCARD_DROPPED_PACKETS,

    /** get/set WRED dropped bytes  count [uint64_t] */
    SAI_PORT_STAT_DISCARD_DROPPED_BYTES,

    /** packet size based packets count */
    SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS,
    SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS,
    SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS,

#if 1 /* added by centec huwx */
    SAI_PORT_STAT_IF_IN_PKTS,
    SAI_PORT_STAT_IF_IN_OAM_PKTS,
    SAI_PORT_STAT_IF_IN_RUNTS_PKTS,
    SAI_PORT_STAT_IF_IN_GIANTS_PKTS,
    SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS,
    SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS,
    SAI_PORT_STAT_IF_IN_FRAME_PKTS,
    SAI_PORT_STAT_IF_IN_PAUSE_PKTS,
    SAI_PORT_STAT_IF_IN_BAD_PKTS,
    SAI_PORT_STAT_IF_IN_BAD_OCTETS,
    SAI_PORT_STAT_IF_OUT_PKTS,
    SAI_PORT_STAT_IF_OUT_OAM_PKTS,
    SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS,
    SAI_PORT_STAT_IF_OUT_PAUSE_PKTS
#endif
} sai_port_stat_counter_t;


/**
 * Routine Description:
 *   @brief Set port attribute value.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_set_port_attribute_fn)(
    _In_ sai_object_id_t port_id,
    _In_ const sai_attribute_t *attr
    );


/**
 * Routine Description:
 *   @brief Get port attribute value.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_get_port_attribute_fn)(
    _In_ sai_object_id_t port_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    );

/**
 * Routine Description:
 *   @brief Get port statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] counter_ids - specifies the array of counter ids
 *    @param[in] number_of_counters - number of counters in the array
 *    @param[out] counters - array of resulting counter values.
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_get_port_stats_fn)(
    _In_ sai_object_id_t port_id,
    _In_ const sai_port_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    );

/**
 * Routine Description:
 *   @brief Clear port statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *    @param[in] counter_ids - specifies the array of counter ids
 *    @param[in] number_of_counters - number of counters in the array
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_clear_port_stats_fn)(
    _In_ sai_object_id_t port_id,
    _In_ const sai_port_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters
    );

/**
 * Routine Description:
 *   @brief Clear port's all statistics counters.
 *
 * Arguments:
 *    @param[in] port_id - port id
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef sai_status_t (*sai_clear_port_all_stats_fn)(
    _In_ sai_object_id_t port_id
    );

/**
 * Routine Description:
 *   Port state change notification
 *   Passed as a parameter into sai_initialize_switch()
 *
 * Arguments:
 *   @param[in] count - number of notifications
 *   @param[in] data  - array of port operational status
 *
 * Return Values:
 *    None
 */
typedef void (*sai_port_state_change_notification_fn)(
    _In_ uint32_t count,
    _In_ sai_port_oper_status_notification_t *data
    );

/**
 * Routine Description:
 *   @brief Port event notification
 *
 * Arguments:
 *    @param[in] count - number of notifications
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*sai_port_event_notification_fn)(
    _In_ uint32_t count,
    _In_ sai_port_event_notification_t *data
    );
/**
 * @brief Port methods table retrieved with sai_api_query()
 */
typedef struct _sai_port_api_t
{
    sai_set_port_attribute_fn       set_port_attribute;
    sai_get_port_attribute_fn       get_port_attribute;
    sai_get_port_stats_fn           get_port_stats;
    sai_clear_port_stats_fn         clear_port_stats;
    sai_clear_port_all_stats_fn     clear_port_all_stats;

} sai_port_api_t;

/**
 * \}
 */
#endif // __SAIPORT_H_

