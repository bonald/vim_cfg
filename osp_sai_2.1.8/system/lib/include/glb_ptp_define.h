/****************************************************************************
 * $Id$
 *  Centec PTP related global MACRO, ENUM, Date Structure defines file
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Kun Cao
 * Date          : 2010-07-15 16:00
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __GLB_PTP_DEFINE_H__
#define __GLB_PTP_DEFINE_H__


#define PTP_STR_TO_VAL_ERROR      0xffffff

#define PTP_TWO_STEP_DEF                TRUE
#define PTP_DEVICE_DEF                      PTP_DEVICE_OC
#define PTP_SYNC_INTF_DEF               TRUE

#define PTP_DOMAIN_MIN                      0
#define PTP_DOMAIN_MAX                      255
#define PTP_DOMAIN_DEF                      PTP_DOMAIN_MIN

#define PTP_PRIORITY_MIN                    0
#define PTP_PRIORITY_MAX                    255
#define PTP_PRIORITY_DEF                    128

#define PTP_TIME_SOURCE_DEF                 PTP_TIME_SOURCE_INTERNAL_OSCILLATOR
#define PTP_CLOCK_ACCURACY_DEF              GLB_PTP_UNKNOWN_CLK_ACC
#define PTP_CLOCK_CLASS_DEF                 248
#define PTP_CLOCK_CLASS_SLAVE               255

#define PTP_UTC_OFFSET_MIN                  0
#define PTP_UTC_OFFSET_MAX                  100
#define PTP_UTC_OFFSET_DEF                  0

#define PTP_ANNOUNCE_INTERVAL_MIN           0
#define PTP_ANNOUNCE_INTERVAL_MAX           10
#define PTP_ANNOUNCE_INTERVAL_DEF           1

#define PTP_SYNC_INTERVAL_MIN               -1
#define PTP_SYNC_INTERVAL_MAX               10
#define PTP_SYNC_INTERVAL_DEF               0

#define PTP_MIN_DELAYREQ_INTERVAL_MIN       -1
#define PTP_MIN_DELAYREQ_INTERVAL_MAX       10
#define PTP_MIN_DELAYREQ_INTERVAL_DEF       0

#define PTP_MIN_PDELAYREQ_INTERVAL_MIN      -1
#define PTP_MIN_PDELAYREQ_INTERVAL_MAX      10
#define PTP_MIN_PDELAYREQ_INTERVAL_DEF      0

#define PTP_ANNOUNCE_RECEIPT_TIMEOUT_MIN    3
#define PTP_ANNOUNCE_RECEIPT_TIMEOUT_MAX    255
#define PTP_ANNOUNCE_RECEIPT_TIMEOUT_DEF    3

#define PTP_ASYMMETRY_CORRECTION_MIN        -2000000
#define PTP_ASYMMETRY_CORRECTION_MAX        2000000
#define PTP_ASYMMETRY_CORRECTION_DEF        0
#define PTP_LATENCY_CORRECTION_DEF             0
#define PTP_LATENCY_CORRECTION_MAX            65535

#define PTP_DELAY_MECHANISM_DEF             PTP_DELAY_MECHANISM_E2E
#define PTP_NET_PROTO_DEF                   PTP_NET_PROTO_IEEE_802_3

/* refer to 8.2.5.3.1 portDS.portState Table 8 - PTP state enumeration */
enum ptp_state_e
{
    PTP_STATE_INITIALIZING = 1,
    PTP_STATE_FAULTY,
    PTP_STATE_DISABLED,
    PTP_STATE_LISTENING,
    PTP_STATE_PRE_MASTER,
    PTP_STATE_MASTER,
    PTP_STATE_PASSIVE,
    PTP_STATE_UNCALIBRATED,
    PTP_STATE_SLAVE,
    PTP_STATE_MAX,
    PTP_STATE_MAX_PLUS
};
typedef enum ptp_state_e ptp_state_t;

enum ptp_bmc_code_e
{
    PTP_BMC_CODE_M1,
    PTP_BMC_CODE_M2,
    PTP_BMC_CODE_M3,
    PTP_BMC_CODE_S1,
    PTP_BMC_CODE_P1,
    PTP_BMC_CODE_P2,
    PTP_BMC_CODE_MAX
};
typedef enum ptp_bmc_code_e ptp_bmc_code_t;

enum ptp_bmc_expires_e
{
    PTP_BMC_EXPIRES_NONE,
    PTP_BMC_EXPIRES_QUALIFICATION_TIMEOUT_EXPIRES,
    PTP_BMC_EXPIRES_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES,
    PTP_BMC_EXPIRES_MAX
};
typedef enum ptp_bmc_expires_e ptp_bmc_expires_t;

enum ptp_device_type_e
{
    PTP_DEVICE_OC,
    PTP_DEVICE_BC,
    PTP_DEVICE_P2PTC,
    PTP_DEVICE_E2ETC,
    PTP_DEVICE_TYPE_MAX
};
typedef enum ptp_device_type_e ptp_device_type_t;

enum glb_ptp_device_type_e
{
    GLB_PTP_DEVICE_OC = 0,          
    GLB_PTP_DEVICE_BC,          
    GLB_PTP_DEVICE_E2E_TC_ONE_STEP,
    GLB_PTP_DEVICE_P2P_TC_ONE_STEP,
    GLB_PTP_DEVICE_E2E_TC_TWO_STEP,
    GLB_PTP_DEVICE_P2P_TC_TWO_STEP,
    GLB_PTP_DEVICE_NONE,
 
    MAX_PTP_DEVICE
};

enum glb_ptp_oper_type_e
{
    GLB_PTP_CAPTURE_ONLY,       /**< [GB] Get timestamp and store in TX Capture FIFO */
    GLB_PTP_REPLACE_ONLY,       /**< [GB] Get timestamp and replace into packet */
    GLB_PTP_NULL_OPERATION,     /**< [GB] Do not take any operation */
    GLB_PTP_CORRECTION          /**< [GB] Get timestamp and calculate correction field value into packet */
};
typedef enum glb_ptp_oper_type_e glb_ptp_oper_type_t;

/*diff with gg*///fixme
enum ptp_delay_mechanism_e
{
    PTP_DELAY_MECHANISM_E2E,
    PTP_DELAY_MECHANISM_P2P,
    PTP_DELAY_MECHANISM_DISABLE,
    PTP_DELAY_MECHANISM_MAX
};
typedef enum ptp_delay_mechanism_e ptp_delay_mechanism_t;

/* refer to 7.4.1 Network transport protocol Table 3 - networkProtocol enumeration */
enum ptp_net_protocol_e
{
    PTP_NET_PROTO_RESERVED,
    PTP_NET_PROTO_UDP_IPV4,
    PTP_NET_PROTO_UDP_IPV6,
    PTP_NET_PROTO_IEEE_802_3,
    PTP_NET_PROTO_DEVICE_NET,
    PTP_NET_PROTO_CONTROL_NET,
    PTP_NET_PROTO_PROFI_NET,
    PTP_NET_PROTO_UDP_IPV4_UNICAST,
    PTP_NET_PROTO_MAX
};
typedef enum ptp_net_protocol_e ptp_net_protocol_t;

struct ptp_sk_buff_s
{
    uint8 *head;
    uint8 *data;
    uint8 *tail;
    uint8 *end;
    uint32 len;
};
typedef struct ptp_sk_buff_s ptp_sk_buff_t;

struct ptp_sockaddr_s
{
    mac_addr_t dest_mac;
    uint16 ether_type;
    uint32 ifindex;
    uint16 vlan_id;
    uint8 vlan_priority;
    uint8 ts_offset;
    uint8 replace;
    uint8 capture; //CTC_PTP_CORRECTION+CTC_PKT_FLAG_PTP_TS_PRECISION
    uint8 correction;
    uint8 is_unicast;
    uint8 seq_id;
    uint32  seconds;            /**< [GB] seconds */
    uint32  nanoseconds;        /**< [GB] nano seconds */
    uint8 ptp_offset;
    uint8 ptp_msg_type;/*add for debug*/
};
typedef struct ptp_sockaddr_s ptp_sockaddr_t;

#define PTP_SERVO_NNS_CACHE_MAX             10
#define PTP_SERVO_PATH_DELAY_CACHE_MAX      10

enum ptp_stats_type_e
{
    PTP_STATS_TX = 0,
    PTP_STATS_RX,
    PTP_STATS_DROP,
    PTP_STATS_MAX,
};
typedef enum ptp_stats_type_e ptp_stats_type_t;

/* refer to 7.6.2.6 timeSource */
enum ptp_time_source_e
{
    PTP_TIME_SOURCE_ATOMIC_CLOCK = 0x10,
    PTP_TIME_SOURCE_GPS = 0x20,
    PTP_TIME_SOURCE_TERRESTRIAL_RADIO = 0x30,
    PTP_TIME_SOURCE_PTP = 0x40,
    PTP_TIME_SOURCE_NTP = 0x50,
    PTP_TIME_SOURCE_HAND_SET = 0x60,
    PTP_TIME_SOURCE_OTHER = 0x90,
    PTP_TIME_SOURCE_INTERNAL_OSCILLATOR = 0xA0
};
typedef enum ptp_time_source_e ptp_time_source_t;


/****************************************************************************
 * clause 13 - PTP message formats
 ****************************************************************************/

/* refer to 13.3.2.2 messageType */
enum ptp_msg_type_e
{
    PTP_MSG_TYPE_SYNC = 0x0,
    PTP_MSG_TYPE_DELAY_REQ = 0x1,
    PTP_MSG_TYPE_PDELAY_REQ = 0x2,
    PTP_MSG_TYPE_PDELAY_RESP = 0x3,
    PTP_MSG_TYPE_FOLLOW_UP = 0x8,
    PTP_MSG_TYPE_DELAY_RESP = 0x9,
    PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP = 0xA,
    PTP_MSG_TYPE_ANNOUNCE = 0xB,
    PTP_MSG_TYPE_SIGNALING = 0xC,
    PTP_MSG_TYPE_MANAGEMENT = 0xD,
    PTP_MSG_TYPE_RESERVED = 0xF,
};
typedef enum ptp_msg_type_e ptp_msg_type_t;

#define PTP_MSG_TYPE_MAX                            PTP_MSG_TYPE_RESERVED

#define PTP_SCALED_NS_BITS 16

#define GLB_PTP_NS_PER_SEC 1000000000
#define GLB_PTP_NNS_PER_NS 1000000000
#define GLB_PTP_UNKNOWN_CLK_ACC 0xFE

/****************************************************************************
 * clause 5 - Data types and on-the-wire formats in a PTP system
 ****************************************************************************/

/* refer to 5.3 Derived data type specifications  */
struct glb_ptp_uint48_s
{
    uint32 lsb;
    uint16 msb;
};
typedef struct glb_ptp_uint48_s glb_ptp_uint48_t;

/* refer to 5.3.2 TimeInterval */
union glb_ptp_timeinterval_u
{
    int64 v;
#if _GLB_HOST_IS_LE
    struct
    {
        uint32 lsb;
        int32 msb;
    } s;
#else
    struct
    {
        int32 msb;
        uint32 lsb;
    } s;
#endif
};
typedef union glb_ptp_timeinterval_u glb_ptp_timeinterval_t;

/* used for internal calculation for convenience */
union glb_ptp_timeinternal_s
{
    int64 v;
#if _GLB_HOST_IS_LE
    struct
    {
        int32 lsb;
        int32 msb;
    } s;
#else
    struct
    {
        int32 msb;
        int32 lsb;
    } s;
#endif
};
typedef union glb_ptp_timeinternal_s glb_ptp_timeinternal_t;

struct glb_ptp_timestamp_s
{
    glb_ptp_uint48_t sec;
    uint32 ns;
};
typedef struct glb_ptp_timestamp_s glb_ptp_timestamp_t;

struct hal_ptp_get_tx_ts_req_s
{
    uint32 ifindex;
    uint32 type;
    uint16 seq_id;
};
typedef struct hal_ptp_get_tx_ts_req_s hal_ptp_get_tx_ts_req_t;

struct hal_ptp_get_tx_ts_resp_s
{
    uint32 invalid;
    uint32 blocked;
    glb_ptp_timestamp_t ts;
};
typedef struct hal_ptp_get_tx_ts_resp_s hal_ptp_get_tx_ts_resp_t;

struct hal_ptp_get_hardware_second_resp_s
{
    int32 second;
    int32 nanoseconds;
};
typedef struct hal_ptp_get_hardware_second_resp_s hal_ptp_get_hardware_second_resp_t;
#define GLB_PTP_CLOCK_ID_LEN  8
#define GLB_PTP_PORT_ID_LEN   10
typedef uint8 glb_ptp_clock_id_t[GLB_PTP_CLOCK_ID_LEN];

struct glb_ptp_port_id_s
{
    glb_ptp_clock_id_t clock_id;
    uint16 port_num;
};
typedef struct glb_ptp_port_id_s glb_ptp_port_id_t;

#define GLB_PTP_MAX_PORT_ADDR_LEN 16

struct glb_ptp_port_address_s
{
    uint16 protocol;
    uint16 length;
    addr_ipv4_t dest_ip_addr;/*added by wangl for ptp unicast 2014-8-26*//*not exist with ip_addr*/
    union {
        uint8 pad[GLB_PTP_MAX_PORT_ADDR_LEN];
        mac_addr_t mac_addr;
        addr_ipv4_t ip_addr;
        addr_ipv6_t ipv6_addr;
    } field;
};
typedef struct glb_ptp_port_address_s glb_ptp_port_address_t;

struct glb_ptp_clock_quality_s
{
    uint8 clk_class;
    uint8 clk_accuracy;
    uint16 offset_scaled_log_variance;
};
typedef struct glb_ptp_clock_quality_s glb_ptp_clock_quality_t;

struct glb_ptp_tlv_s
{
    uint16 type;
    uint16 length;
    uint8* value;
};
typedef struct glb_ptp_tlv_s glb_ptp_tlv_t;

struct glb_ptp_text_s
{
    uint16 length;
    uint8* text;
};
typedef struct glb_ptp_text_s glb_ptp_text_t;

/****************************************************************************
 * clause Annex D - Transport of PTP over User Datagram Protocol over
 * Internet Protocol Version 4
 ****************************************************************************/
#define GLB_PTP_IPV4_ETYPE                  0x0800
#define GLB_PTP_IPV6_ETYPE                  0x86DD

/* refer to D.2 UDP port numbers */
#define GLB_PTP_UDP_EVENT_PORT              319
#define GLB_PTP_UDP_GENERAL_PORT            320

/* refer to D.3 IPv4 multicast addresses */
/*"224.0.1.129"*/
#define GLB_PTP_IPV4_ADDRESS_PRIMARY        0xE0000181
/*"224.0.0.107"*/
#define GLB_PTP_IPV4_ADDRESS_PDELAY         0xE000006B

#define GLB_PTP_IP_MAC_ADDRESS_PRIMARY      {0x01, 0x00, 0x5e, 0x00, 0x01, 0x81}
#define GLB_PTP_IP_MAC_ADDRESS_PDELAY       {0x01, 0x00, 0x5e, 0x00, 0x00, 0x6b}

/****************************************************************************
 * clause Annex F - Transport of PTP over IEEE 802.3 /Ethernet
 ****************************************************************************/
/* refer to F.2 Ethertype */
#define GLB_PTP_1588_ETYPE                  0x88F7

/* refer to F.3 Multicast MAC Addresses */
#define GLB_PTP_MAC_ADDRESS_DEFAULT         {0x01, 0x1B, 0x19, 0x00, 0x00, 0x00}
#define GLB_PTP_MAC_ADDRESS_PDELAY          {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E}



#endif /*!__GLB_PTP_DEFINE_H__*/
