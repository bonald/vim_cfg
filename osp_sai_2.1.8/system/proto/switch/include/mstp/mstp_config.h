#ifndef __MSTP_CONFIG_H__
#define __MSTP_CONFIG_H__

#define MSTP_DEFAULT_TYPE               MSTP_TYPE_RSTP
#define MSTP_DEF_PATH_COST              MSTP_PATH_COST_1T

#define MSTP_TIMER_DEF_MIGRATE_TIME     3       /* 17.13.9 */
#define MSTP_TIMER_DEF_HELLO_TIME       2       /* 17.13.6 [1, 2] */
#define MSTP_TIMER_DEF_MAX_AGE          20      /* 17.13.8 [6, 40] */
#define MSTP_TIMER_DEF_FWD_DELAY        15      /* 17.13.5 [4, 30] */
#define MSTP_TIMER_DEF_AGEING_TIME      300

#define MSTP_TX_HOLD_COUNT              6       /* 17.13.12 [1, 10] */

#define MSTP_DEF_BRIDGE_PRIORITY        32768
#define MSTP_DEF_PORT_PRIORITY          128

#define MSTP_DEF_MAX_HOPS               20
#define MSTP_REGION_NAME                ""

#define MSTP_DEF_TCN_THRESHOLD          1
#define MSTP_MAX_TCN_THRESHOLD          255

#define MSTP_REVISION_LEVEL_DEF         0
#define MSTP_REVISION_LEVEL_MIN         0
#define MSTP_REVISION_LEVEL_MAX         65535

#define MSTP_TIMER_MIN_MAX_AGE          6
#define MSTP_TIMER_MAX_MAX_AGE          40
#define MSTP_TIMER_MIN_FWD_DELAY        4
#define MSTP_TIMER_MAX_FWD_DELAY        30
#define MSTP_TIMER_MIN_HELLO_TIME       1
#define MSTP_TIMER_MAX_HELLO_TIME       10

#define MST_INSTANCES_MAX               GLB_STP_INSTANCE_MAX

#endif /* !__MSTP_CONFIG_H__ */
