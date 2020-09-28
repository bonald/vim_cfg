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
*    saitypes.h
*
* Abstract:
*
*    This module contains SAI portable types.
*/

#if !defined (__SAITYPES_H_)
#define __SAITYPES_H_

/** \defgroup SAITYPES SAI - Types definitions.
 *
 *  \{
 */

#if defined(_WIN32)

//
// *nix already has lower-case definitions for types.
//
typedef UINT8  uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef INT32  int32_t;
typedef INT64  int64_t;
typedef UINT64 uint64_t;

typedef  INT32  sai_status_t;
typedef UINT32  sai_switch_profile_id_t;
typedef UINT16  sai_vlan_id_t;
typedef UINT32  sai_attr_id_t;
typedef UINT8   sai_cos_t;
typedef UINT8   sai_queue_index_t;
typedef UINT8   sai_mac_t[6];
typedef UINT32  sai_ip4_t;
typedef UINT8   sai_ip6_t[16];
typedef UINT32  sai_switch_hash_seed_t;

#include <ws2def.h>
#include <ws2ipdef.h>

#if !defined(__BOOL_DEFINED)

typedef enum {
  false,
  true
} _bool;

#define bool _bool

#endif  // __BOOL_DEFINED

//
// N.B. Equal to 260 on Windows
//
#define PATH_MAX MAX_PATH



#else  // #if defined(_WIN32)

#include <stdint.h>
#if !defined(__BOOL_DEFINED)
#define __BOOL_DEFINED
typedef int            bool;   /**< Boolean type */
#endif

#define true 1
#define false 0

#include <sys/types.h>

typedef int32_t  sai_status_t;
typedef uint32_t sai_switch_profile_id_t;
typedef uint16_t sai_vlan_id_t;
typedef uint32_t sai_attr_id_t;
typedef uint8_t  sai_cos_t;
typedef uint8_t  sai_queue_index_t;
typedef uint8_t  sai_mac_t[6];
typedef uint32_t sai_ip4_t;
typedef uint8_t  sai_ip6_t[16];
typedef uint32_t sai_switch_hash_seed_t;

#define _In_
#define _Out_
#define _Inout_
#define _In_reads_z_(_LEN_)
#define _In_reads_opt_z_(_LEN_)

#endif // _WIN32

//
// New common definitions
//
typedef uint64_t sai_uint64_t;
typedef int64_t sai_int64_t;
typedef uint32_t sai_uint32_t;
typedef int32_t sai_int32_t;
typedef uint16_t sai_uint16_t;
typedef int16_t sai_int16_t;
typedef uint8_t sai_uint8_t;
typedef int8_t sai_int8_t;
typedef size_t sai_size_t;
typedef uint64_t sai_object_id_t;

#define SAI_NULL_OBJECT_ID 0L

/**
 * Defines a list of sai object ids used as sai attribute value.
 *
 * - In set attribute function call, the count member defines the number of
 * objects.
 *
 * - In get attribute function call, the function call returns a list of objects
 * to the caller in the list member. The caller is responsible for allocating the
 * buffer for the list member and set the count member to the size of allocated object
 * list. If the size is large enough to accomodate the list of object id, the
 * callee will then fill the list member and set the count member to the actual
 * number of objects.  If the list size is not large enough, the callee will set the
 * count member to the actual number of object id and return
 * SAI_STATUS_BUFFER_OVERFLOW. Once the caller gets such return code, it should
 * use the returned count member to re-allocate list and retry.
 */
typedef struct _sai_object_list_t {
    uint32_t count;
    sai_object_id_t *list;
} sai_object_list_t;

/**
 * @brief sai object type
 */
typedef enum _sai_object_type_t {
    SAI_OBJECT_TYPE_NULL             =  0,
    SAI_OBJECT_TYPE_PORT             =  1,
    SAI_OBJECT_TYPE_LAG              =  2,
    SAI_OBJECT_TYPE_VIRTUAL_ROUTER   =  3,
    SAI_OBJECT_TYPE_NEXT_HOP         =  4,
    SAI_OBJECT_TYPE_NEXT_HOP_GROUP   =  5,
    SAI_OBJECT_TYPE_ROUTER_INTERFACE =  6,
    SAI_OBJECT_TYPE_ACL_TABLE        =  7,
    SAI_OBJECT_TYPE_ACL_ENTRY        =  8,
    SAI_OBJECT_TYPE_ACL_COUNTER      =  9,
    SAI_OBJECT_TYPE_HOST_INTERFACE   = 10,
    SAI_OBJECT_TYPE_MIRROR           = 11,
    SAI_OBJECT_TYPE_SAMPLEPACKET     = 12,
    SAI_OBJECT_TYPE_STP_INSTANCE     = 13,
    SAI_OBJECT_TYPE_TRAP_GROUP       = 14,
    SAI_OBJECT_TYPE_ACL_TABLE_GROUP  = 15,
    SAI_OBJECT_TYPE_POLICER          = 16,
    SAI_OBJECT_TYPE_WRED             = 17,
    SAI_OBJECT_TYPE_QOS_MAPS         = 18,
    SAI_OBJECT_TYPE_QUEUE            = 19,
    SAI_OBJECT_TYPE_SCHEDULER        = 20,
    SAI_OBJECT_TYPE_SCHEDULER_GROUP  = 21,
    SAI_OBJECT_TYPE_BUFFER_POOL      = 22,
    SAI_OBJECT_TYPE_BUFFER_PROFILE   = 23,
    SAI_OBJECT_TYPE_PRIORITY_GROUP   = 24,
    SAI_OBJECT_TYPE_LAG_MEMBER       = 25,
    SAI_OBJECT_TYPE_HASH             = 25,
    SAI_OBJECT_TYPE_UDF              = 26,
    SAI_OBJECT_TYPE_UDF_MATCH        = 27,
    SAI_OBJECT_TYPE_UDF_GROUP        = 28,
    SAI_OBJECT_TYPE_ISOLATION_GROUP  = 29,  /* added by kcao for SAI_EXT_ISOLATION 2015-12-01 */
    SAI_OBJECT_TYPE_VLAN_CLASS_GROUP  = 30,  /* added by yejl for SAI_EXT_VLAN_CLASS_GROUP 2016-09-30 */
    SAI_OBJECT_TYPE_VLAN_CLASS_GROUP_MEMBER  = 31,  /* added by yejl for SAI_EXT_VLAN_CLASS_GROUP_MEMBER 2016-09-30 */
    SAI_OBJECT_TYPE_TAP_GROUP        = 32,
    SAI_OBJECT_TYPE_COPP             = 33,  /* added by chenyq for copp implementation*/
    SAI_OBJECT_TYPE_IPMC             = 34,
    SAI_OBJECT_TYPE_MAX              = 35
} sai_object_type_t;

typedef struct _sai_u8_list_t {
    uint32_t count;
    uint8_t *list;
} sai_u8_list_t;

typedef struct _sai_s8_list_t {
    uint32_t count;
    int8_t  *list;
} sai_s8_list_t;

typedef struct _sai_u16_list_t {
    uint32_t count;
    uint16_t *list;
} sai_u16_list_t;

typedef struct _sai_s16_list_t {
    uint32_t count;
    int16_t  *list;
} sai_s16_list_t;

typedef struct _sai_u32_list_t {
    uint32_t count;
    uint32_t *list;
} sai_u32_list_t;

typedef struct _sai_s32_list_t {
    uint32_t count;
    int32_t  *list;
} sai_s32_list_t;

typedef struct _sai_u32_range_t {
    uint32_t min;
    uint32_t max;
} sai_u32_range_t;

typedef struct _sai_s32_range_t {
    int32_t min;
    int32_t max;
} sai_s32_range_t;

/**
 * @brief Defines a vlan list datastructure
 */
typedef struct _sai_vlan_list_t {

    /** Number of Vlans*/
    uint32_t vlan_count;

    /** List of Vlans*/
    sai_vlan_id_t *vlan_list;

} sai_vlan_list_t;

/**
 *  @brief Attribute data for tagging_mode parameter
 */
typedef enum _sai_vlan_tagging_mode_t
{
    SAI_VLAN_PORT_UNTAGGED,

    SAI_VLAN_PORT_TAGGED,

    SAI_VLAN_PORT_PRIORITY_TAGGED

} sai_vlan_tagging_mode_t;

/**
 *   @brief Port/vlan membership structure
 */
typedef struct _sai_vlan_port_t
{
    sai_object_id_t port_id;

    sai_vlan_tagging_mode_t tagging_mode;

} sai_vlan_port_t;

/**
 * @brief Defines a vlan port list datastructure
 */
typedef struct _sai_vlan_port_list_t {

    /** Number of ports in a VLAN */
    uint32_t count;

    /** List of ports in a VLAN */
    sai_vlan_port_t *list;

} sai_vlan_port_list_t;

typedef enum _sai_ip_addr_family_t {
    SAI_IP_ADDR_FAMILY_IPV4,

    SAI_IP_ADDR_FAMILY_IPV6
} sai_ip_addr_family_t;

typedef struct _sai_ip_address_t {
    sai_ip_addr_family_t addr_family;
    union {
        sai_ip4_t ip4;
        sai_ip6_t ip6;
    } addr;
} sai_ip_address_t;

typedef struct _sai_ip_prefix_t {
    sai_ip_addr_family_t addr_family;
    union {
        sai_ip4_t ip4;
        sai_ip6_t ip6;
    } addr;
    union {
        sai_ip4_t ip4;
        sai_ip6_t ip6;
    } mask;
} sai_ip_prefix_t;

/**
 * @brief Defines a single ACL filter
 *
 * Note : IPv4 and IPv6 Address expected in Network Byte Order
 */
typedef struct _sai_acl_field_data_t
{
    /**
     * match enable/disable
     */
    bool enable;

    /**
     * Field match mask
     */
    union {
        sai_uint8_t u8;
        sai_int8_t s8;
        sai_uint16_t u16;
        sai_int16_t s16;
        sai_uint32_t u32;
        sai_int32_t s32;
        sai_mac_t mac;
        sai_ip4_t ip4;
        sai_ip6_t ip6;
    } mask;

    /**
     * Expected AND result using match mask above with packet field value where applicable.
     */
    union {
        sai_uint8_t u8;
        sai_int8_t s8;
        sai_uint16_t u16;
        sai_int16_t s16;
        sai_uint32_t u32;
        sai_int32_t s32;
        sai_mac_t mac;
        sai_ip4_t ip4;
        sai_ip6_t ip6;
        sai_object_id_t oid;
        sai_object_list_t objlist;
    } data;
} sai_acl_field_data_t;

/* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
typedef struct _sai_tc_and_color_t
{
    uint16_t tc;
    uint16_t color;
} sai_tc_and_color_t;
/* END: fix gb tc+color bug add by wangqj at 2016/8/11 */

/**
 * @brief Defines a single ACL action
 *
 * Note : IPv4 and IPv6 Address expected in Network Byte Order
 */
typedef struct _sai_acl_action_data_t
{
    /**
     * action enable/disable
     */
    bool enable;
    /**
     * Action parameter
     */
    union {
        sai_uint8_t u8;
        sai_int8_t s8;
        sai_uint16_t u16;
        sai_int16_t s16;
        sai_uint32_t u32;
        sai_int32_t s32;
        sai_mac_t mac;
        sai_ip4_t ip4;
        sai_ip6_t ip6;
        sai_object_id_t oid;
        sai_object_list_t objlist;
        /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
        sai_tc_and_color_t tcandcolor;
        /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
    } parameter;
} sai_acl_action_data_t;

/**
 * @brief Breakout Mode types based on number
 * of SerDes lanes used in a port
 */
typedef enum _sai_port_breakout_mode_type_t
{
    /** 1 lane breakout Mode */
    SAI_PORT_BREAKOUT_MODE_1_LANE = 1,

    /** 2 lanes breakout Mode */
    SAI_PORT_BREAKOUT_MODE_2_LANE = 2,

    /** 4 lanes breakout Mode */
    SAI_PORT_BREAKOUT_MODE_4_LANE = 4,

    /** Breakout mode max count */
    SAI_PORT_BREAKOUT_MODE_MAX
} sai_port_breakout_mode_type_t;

/**
 * @brief Defines breakout mode on a switch port(s)
 */
typedef struct _sai_port_breakout_t
{
    /**
     * Breakout mode type
     */
    sai_port_breakout_mode_type_t breakout_mode;

    /**
     * List of ports to be breakout
     * Break out - typically 1 port; Break in - set of ports
     */
    sai_object_list_t  port_list;
} sai_port_breakout_t;

typedef enum _sai_packet_color_t
{
    SAI_PACKET_COLOR_GREEN,

    SAI_PACKET_COLOR_YELLOW,

    SAI_PACKET_COLOR_RED,

} sai_packet_color_t;

/**
 * Defines qos map types.
 * Examples:
 * dot1p/dscp --> TC
 * dot1p/dscp --> Color
 * dot1p/dscp --> TC + Color
 * Tc --> dot1p/Dscp.
 * Tc + color --> dot1p/Dscp.
 * Tc --> Egress Queue.
 */

typedef struct _sai_qos_map_params_t
{
    /** Traffic class */
    sai_cos_t   tc;

    /** added by hansf for exp mapping */
    sai_uint8_t   exp;

    /** DSCP value */
    sai_uint8_t dscp;

    /** Dot1p value */
    sai_uint8_t dot1p;

    /** PFC priority value */
    sai_uint8_t prio;

    /** Priority group value */
    sai_uint8_t pg;

    /** Egress port queue UOID is not known at the time of map creation.
     * Using queue index for maps. */
    sai_queue_index_t    queue_index;

    /** Color of the packet */
    sai_packet_color_t color;

} sai_qos_map_params_t;

typedef struct _sai_qos_map_t
{
    /** Input parameters to match */
    sai_qos_map_params_t key;

    /** Output map parameters */
    sai_qos_map_params_t value;

} sai_qos_map_t;

typedef struct _sai_qos_map_list_t
{
    /** Number of entries in the map  */
    uint32_t count;
    /** Map list */
    sai_qos_map_t *list;
} sai_qos_map_list_t;


typedef struct _sai_sflow_list_t
{
    /** sflow sample direction  */
    sai_int8_t dir;
    
    /* sflow threshold */
    uint16_t threshold;

    sai_int8_t enable;
} sai_sflow_t;


typedef struct _sai_vrrp_list_t
{
    sai_mac_t         mac[4];
    sai_uint32_t      num;
} sai_vrrp_t;

struct sai_packet_ts_s
{
    sai_uint32_t  seconds;            /**< [GB] seconds */
    sai_uint32_t  nanoseconds;        /**< [GB] nano seconds */
};
typedef struct sai_packet_ts_s sai_packet_ts_t;

enum sai_ptp_oper_type_e
{
    SAI_PTP_CAPTURE_ONLY,       /**< [GB] Get timestamp and store in TX Capture FIFO */
    SAI_PTP_REPLACE_ONLY,       /**< [GB] Get timestamp and replace into packet */
    SAI_PTP_NULL_OPERATION,     /**< [GB] Do not take any operation */
    SAI_PTP_CORRECTION          /**< [GB] Get timestamp and calculate correction field value into packet */
};
typedef enum sai_ptp_oper_type_e sai_ptp_oper_type_t;

struct sai_pkt_ptp_info_s
{
    sai_packet_ts_t     ts;         /**< [GB.GG] [RX,TX] PTP timestamp */
    sai_ptp_oper_type_t oper;       /**< [GB.GG] [TX] PTP operation type */
    sai_uint8_t               seq_id;     /**< [GB.GG] [TX] PTP capture sequence Id, only for oper equal to CAPTURE_ONLY */
    sai_uint8_t               ts_offset;  /**< [GG]  [TX] PTP timestamp offset */
    sai_uint8_t               vlan_priority;
    sai_uint8_t               is_unicast;
    sai_uint16_t            vlan_id;
    sai_uint64_t            vr_id;
};
typedef struct sai_pkt_ptp_info_s sai_pkt_ptp_info_t;

struct sai_pkt_oam_info_s
{
    sai_uint32_t  type;               /**< [GB.GG] [RX,TX] OAM type, refer to ctc_oam_type_t */
    sai_uint32_t  flags;              /**< [GB.GG]  [RX,TX] OAM flags, refer to ctc_pkt_oam_flag_t */
    sai_uint32_t  dm_ts_offset;       /**< [GB.GG] [TX] [DM] Offset in bytes of timestamp */
    sai_uint16_t  vid;                /**< [GB.GG] [TX] VLAN ID, valid for UP MEP */
    sai_uint16_t  mep_index;          /**< [GB.GG] [RX] index of table MEP/RMEP */
    sai_packet_ts_t dm_ts;      /**< [GB.GG] [RX] [DM] timestamp */
    sai_uint32_t  lm_fcl;             /**< [GB.GG] [RX] [LM] RxFCl value at the time LMR frame was received */
    sai_uint16_t dest_gport;
    sai_uint8_t ttl;
    sai_uint8_t src_cos;
    sai_uint8_t priority;
    sai_uint32_t  bit_flags;
    sai_uint32_t  nh_id;
    sai_uint8_t color;
    sai_uint8_t is_bypass;
    sai_uint16_t src_gport;
};
typedef struct sai_pkt_oam_info_s sai_pkt_oam_info_t;

typedef struct _sai_cfm_list_t
{
    char                          maid[48];
    sai_object_id_t          glPort;
    sai_uint16_t              vlanid;
    sai_uint8_t                direction;
    sai_uint8_t                level;
    
    sai_uint16_t              mepid;
    sai_uint8_t	              mpAddrMode;
    sai_uint8_t	              enableDM;
    
    sai_uint8_t	              active;
    sai_uint8_t	              ccmInterval;
    sai_uint8_t	              tpidType;
    sai_uint8_t	              ccmGportid;
    
     sai_uint8_t	              mcGroupidForUpMep;
     sai_uint8_t	              mepOnCpu;
     sai_uint8_t	              addChan;
     sai_uint8_t	              vlanMaxLevel;
    
     sai_uint8_t	              portMaxLevel;
     sai_uint16_t	       mepIndex;
     sai_uint8_t	              mode;                /* Y.1731 or 802.1ag */

     sai_uint16_t	       mep_num;
     sai_uint8_t	              cci_enable;
     sai_uint8_t               ccm_vlan_cos;

     sai_mac_t                 rmep_mac;
     sai_uint16_t	       rmepIndex;

     sai_uint16_t              rmepid;
     sai_uint8_t                d_unexp_mep;         /* Incorrect MEP ID */
     sai_uint8_t                reserved2;
     
     sai_uint8_t                d_mismerge;           /* Incorrect MAID */
     sai_uint8_t                d_meg_lvl;              /* Low level */
     sai_uint8_t                rdi;                          /* Remote Defect Indication */
     sai_uint8_t                first_pkt_rx;           /* Receive first correct packet */
     
     sai_uint8_t                d_loc;                       /* Don't receive ccm packets in 3.5 */
     sai_uint8_t                d_unexp_period;      /* Incorrect ccm period */
     sai_uint8_t                ma_sa_mismatch;    /* Incorrect source mac address for RMEP */
     sai_uint8_t                rmep_last_rdi;         /* Remote last rdi */

     sai_uint32_t              ifindex;                     /* ifindex is used when cfm interrupt */
} sai_cfm_t;

/**
 * @brief Data Type to use enum's as attribute value is sai_int32_t s32
 *
 */
typedef union {
    bool booldata;
    char chardata[48];
    sai_uint8_t u8;
    sai_int8_t s8;
    sai_uint16_t u16;
    sai_int16_t s16;
    sai_uint32_t u32;
    sai_int32_t s32;
    sai_uint64_t u64;
    sai_int64_t s64;
    sai_mac_t mac;
    sai_ip4_t ip4;
    sai_ip6_t ip6;
    sai_ip_address_t ipaddr;
    sai_object_id_t oid;
    sai_object_list_t objlist;
    sai_u8_list_t u8list;
    sai_s8_list_t s8list;
    sai_u16_list_t u16list;
    sai_s16_list_t s16list;
    sai_u32_list_t u32list;
    sai_s32_list_t s32list;
    sai_u32_range_t u32range;
    sai_s32_range_t s32range;
    sai_vlan_list_t vlanlist;
    sai_vlan_port_list_t vlanportlist;
    sai_acl_field_data_t aclfield;
    sai_acl_action_data_t aclaction;
    sai_port_breakout_t portbreakout;
    sai_qos_map_list_t qosmap;
    sai_sflow_t       sflow;
    sai_vrrp_t        vrrp;
    sai_pkt_ptp_info_t ptp;
    sai_pkt_oam_info_t tpoam;
    sai_cfm_t         cfm;
} sai_attribute_value_t;

typedef struct _sai_attribute_t {
    sai_attr_id_t id;
    sai_attribute_value_t value;
} sai_attribute_t;

/**
 @brief define direction
*/
enum ctc_sai_copp_entry_type_e
{
    CTC_SAI_SSH = 1,        /**< SSH entry */
    CTC_SAI_TELNET,         /**< TELNET entry */
    CTC_SAI_MLAG,           /**< MLAG entry */
};

/**
 *\}
 */
#endif // __SAITYPES_H_
