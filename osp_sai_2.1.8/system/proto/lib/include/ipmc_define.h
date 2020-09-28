
#ifndef _IPMC_DEFINE_H
#define _IPMC_DEFINE_H

#define MCAST_ROUTE_ENABLE   1
#define MCAST_ROUTE_DISABLE  1

#define PIM_REGISTER_SUPPRESSION_TIME_DEFAULT    60
#define PIM_REGISTER_PROBE_TIME_DEFAULT          5
#define PIM_KEEPALIVE_PERIOD                     210
#define PIM_DEFAULT_PACKET_PROCESS               3
#define PIM_DEFAULT_HELLO_PERIOD                 30   /* seconds, RFC 4601: 4.11 */
#define PIM_DEFAULT_HELLO_HOLDTIME               (3.5 * PIM_DEFAULT_HELLO_PERIOD)   /* seconds, RFC 4601: 4.11 */
#define PIM_DEFAULT_TRIGGERED_HELLO_DELAY        5    /* seconds, RFC 4601: 4.11 */
#define PIM_DEFAULT_DR_PRIORITY                  1    /* RFC 4601: 4.3.1 */
#define PIM_DEFAULT_PROPAGATION_DELAY_MSEC       500  /* RFC 4601: 4.11.  Timer Values */
#define PIM_DEFAULT_OVERRIDE_INTERVAL_MSEC       2500 /* RFC 4601: 4.11.  Timer Values */
#define PIM_DEFAULT_CAN_DISABLE_JOIN_SUPPRESSION 0    /* boolean */
#define PIM_DEFAULT_T_PERIODIC                   60   /* RFC 4601: 4.11.  Timer Values */

#define PIM_IGMP_BUFSIZE_READ         (20000)
#define PIM_IGMP_BUFSIZE_WRITE        (20000)

#define PIM_IGMP_MEMBERSHIP_QUERY     (0x11)
#define PIM_IGMP_V1_MEMBERSHIP_REPORT (0x12)
#define PIM_IGMP_V2_MEMBERSHIP_REPORT (0x16)
#define PIM_IGMP_V2_LEAVE_GROUP       (0x17)
#define PIM_IGMP_V3_MEMBERSHIP_REPORT (0x22)

#define IGMP_V3_REPORT_HEADER_SIZE    (8)
#define IGMP_V3_GROUP_RECORD_MIN_SIZE (8)
#define IGMP_V3_MSG_MIN_SIZE          (IGMP_V3_REPORT_HEADER_SIZE + IGMP_V3_GROUP_RECORD_MIN_SIZE)
#define IGMP_V12_MSG_SIZE             (8)

#define IGMP_V3_GROUP_RECORD_TYPE_OFFSET       (0)
#define IGMP_V3_GROUP_RECORD_AUXDATALEN_OFFSET (1)
#define IGMP_V3_GROUP_RECORD_NUMSOURCES_OFFSET (2)
#define IGMP_V3_GROUP_RECORD_GROUP_OFFSET      (4)
#define IGMP_V3_GROUP_RECORD_SOURCE_OFFSET     (8)
#define IGMP_CHECKSUM_OFFSET                   (2)

/* RFC 3376: 8.1. Robustness Variable - Default: 2 */
#define IGMP_DEFAULT_ROBUSTNESS_VARIABLE           (2)

/* RFC 3376: 8.2. Query Interval - Default: 125 seconds */
#define IGMP_GENERAL_QUERY_INTERVAL                (125)

/* RFC 3376: 8.3. Query Response Interval - Default: 10 seconds */
#define IGMP_QUERY_MAX_RESPONSE_TIME               (10)

/* RFC 3376: 8.8. Last Member Query Interval - Default: 10 deciseconds */
#define IGMP_SPECIFIC_QUERY_MAX_RESPONSE_TIME_DSEC (10)

#define IGMP_DEFAULT_VERSION (3)

enum pim_spt_switchover {
	PIM_SPT_IMMEDIATE,
	PIM_SPT_INFINITY,
};

#endif /* _IPMC_DEFINE_H */

