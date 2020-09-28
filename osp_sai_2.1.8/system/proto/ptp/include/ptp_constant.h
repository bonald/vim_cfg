
#ifndef __PTP_CONSTANT_H__
#define __PTP_CONSTANT_H__

#include "glb_const.h"

#define PTP_SECOND_BT_UNIX_WITH_GPS 315964819

/****************************************************************************
 * clause 7 - Characterization of PTP entities
 ****************************************************************************/

/* refer to 7.1 Domains Table 2 - domainNumber */
enum ptp_domain_e
{
    PTP_DOMAIN_DEFAULT,
    PTP_DOMAIN_ALTERNATE1,
    PTP_DOMAIN_ALTERNATE2,
    PTP_DOMAIN_ALTERNATE3
};
typedef enum ptp_domain_e ptp_domain_t;

/* refer to 7.5.5 versionNumber */
#define PTP_VERSION_1       1
#define PTP_VERSION_2       2
#define PTP_VERSION_DEF     PTP_VERSION_2

#define PTP_MSG_TYPE_MAX                            PTP_MSG_TYPE_RESERVED

/* refer to 13. PTP message formats */
#define PTP_MSG_BUFFER_LENGTH                       256

#define PTP_MSG_HEADER_LENGTH                       34
#define PTP_MSG_ANNOUNCE_LENGTH                     64
#define PTP_MSG_SYNC_FOLLOW_UP_LENGTH               44
#define PTP_MSG_PDELAY_REQ_LENGTH                   54
#define PTP_MSG_DELAY_REQ_LENGTH                    44
#define PTP_MSG_DELAY_RESP_LENGTH                   54
#define PTP_MSG_PDELAY_RESP_LENGTH                  54
#define PTP_MSG_PDELAY_RESP_FOLLOW_UP_LENGTH        54
#define PTP_MSG_PEER_LENGTH                         54

#define PTP_MSG_TS_OFFSET                           34

/* refer to 13.3.2.10 controlField */
enum ptp_msg_ctrl_e
{
    PTP_MSG_CTRL_SYNC,
    PTP_MSG_CTRL_DELAY_REQ,
    PTP_MSG_CTRL_FOLLOW_UP,
    PTP_MSG_CTRL_DELAY_RESP,
    PTP_MSG_CTRL_MANAGEMENT,
    PTP_MSG_CTRL_ALL_OTHERS
};
typedef enum ptp_msg_ctrl_e ptp_msg_ctrl_t;

/* refer to 13.3.2.11 logMessageInterval */
#define PTP_LOG_MSG_INTERVAL_DEFAULT        0x7F

/* non-standard constants */
#define PTP_VLAN_ID_MIN                     1
#define PTP_VLAN_ID_MAX                     4094
#define PTP_VLAN_ID_NONE                    0
#define PTP_VLAN_COS_MIN                    0
#define PTP_VLAN_COS_MAX                    7
#define PTP_TOCPU_VLAN_NUMBER_MAX          10

#define PTP_RESIDENCETIME_CACHE_SIZE        4

#define PTP_SERVO_NNS_CACHE_MAX             10
#define PTP_SERVO_PATH_DELAY_CACHE_MAX      10

#define PTP_CLOCK_TOCPU_VID_DEF            0
#define PTP_CLCOK_TOD_SET_PULSE_DELAY_DEF 0

#define PTP_LEAP_NONE                       0
#define PTP_LEAP_59                         1
#define PTP_LEAP_61                         2

#define PTP_NUMBER_OF_ALTERNATE_MASTERS_MIN 0
#define PTP_NUMBER_OF_ALTERNATE_MASTERS_MAX 1
#define PTP_NUMBER_OF_ALTERNATE_MASTERS_DEF 0

#define PTP_TOD_PULSE_DELAY_CORRECTION_MIN  1
#define PTP_TOD_PULSE_DELAY_CORRECTION_MAX  1000000

#define PTP_PARENT_STATS_DEF                FALSE

#define PTP_SKB_RESERVE 64

#define DBG_PTP_TYPE_FIELD       0
#define DBG_PTP_TYPE_ALL         1

/*PTP_OBSERVED_PARENT_OFFSET_SCALED_LOG_VARIANCE_DEF*/
#define PTP_OBSERVED_PARENT_OSLV_DEF        0xFFFF

/*PTP_OBSERVED_PARENT_CLOCK_PHASE_CHANGE_RATE_DEF*/
#define PTP_OBSERVED_PARENT_CPCR_DEF        0x7FFFFFFF

//#define PTP_DEFAULT_CLOCK_VARIANCE          -4000
#define PTP_DEFAULT_CLOCK_VARIANCE          0xF060

#define PTP_MAX_NS    1000000000
/* 9.3.2.4.5 require at least five */
#define PTP_FOREIGN_MASTER_CAPACITY_MAX     5

enum ptp_direction_e
{
    PTP_DIR_NONE,
    PTP_DIR_INGRESS,
    PTP_DIR_EGRESS,
    PTP_DIR_BOTH_DIRECTION,
};
typedef enum ptp_direction_e ptp_direction_t;

enum ptp_dump_mode_e
{
    PTP_DUMP_MODE_NONE,
    PTP_DUMP_MODE_RAW,
    PTP_DUMP_MODE_DEC,
    PTP_DUMP_MODE_BOTH,
};
typedef enum ptp_dump_mode_e ptp_dump_mode_t;

#define PTP_UDP_IP_HDR_LEN (UDP_HEADER_LEN + IP_HEADER_LEN)
#define IN_LOOPBACKNET          127
#define    IN_CLASSA_NSHIFT        24

#define PAL_IPTOS_PREC_INTERNETCONTROL (1 << 0)

#endif /*!__PTP_CONSTANT_H__*/

