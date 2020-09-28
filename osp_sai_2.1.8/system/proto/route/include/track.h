
#ifndef _TRACK_H
#define _TRACK_H

/* track object type*/
#define   TRACK_UNCONFIG                    0
#define   TRACK_INTERFACE_LINKSTATE         1
#define   TRACK_INTERFACE_IP_ROUTING        2
#define   TRACK_IP_ROUTE_REACHABILITY       3
#define   TRACK_IP_ROUTE_METRIC_THRESHOLD   4
#define   TRACK_RTR_REACHABILITY            5
#define   TRACK_RTR_STATE                   6
#define   TRACK_BFD_INDEX                   7

#define   TRACK_OBJECT_MAX                  500

/* track object state*/
#define   TRACK_STATE_INVALID               0
#define   TRACK_STATE_UP                    1
#define   TRACK_STATE_DOWN                  2

/* track command */
#define   TRACK_SUCCESS                     0
#define   TRACK_OBJECT_EXIST_ERR           -1
#define   TRACK_CREATE_OBJECT_ERR          -2
#define   TRACK_SET_ERR                    -3
#define   TRACK_SHOW_ERR                   -4
#define   TRACK_INTERFACE_NOT_EXIST_ERR    -5
#define   TRACK_OBJECT_NOT_EXIST_ERR       -6
#define   TRACK_RTR_ENTRY_NOT_EXIST_ERR    -7

#define   TRACK_IP_DIFFERENT_SUBNET         0
#define   TRACK_IP_SAME                     1
#define   TRACK_IP_SAME_SUBNET              2

/* track delay up */
#define   TRACK_DELAY_UP_CONTINUE           1
#define   TRACK_DELAY_UP_END                2

/* track delay down */
#define   TRACK_DELAY_DOWN_CONTINUE         1
#define   TRACK_DELAY_DOWN_END              2

#define   TRACK_BY_VRRP                     1

#define   TRACK_TIMER_INTERVAL_DEFAULT      1

/* track interface state*/
#define   TRACK_IF_STATE_UNKNOWN            1
#define   TRACK_IF_STATE_DOWN               2
#define   TRACK_IF_STATE_UP                 3
#define   TRACK_IF_STATE_DELETE             4

#define   TRACK_OBJECTID_MIN                1
#define   TRACK_OBJECTID_MAX                500
#define   TRACK_DELAY_MIN                   0
#define   TRACK_DELAY_MAX                   180

#define   TRACK_MAX_STATIC_RT_REFCNT        64
#define   TRACK_UPDATE_STATIC_RT_CNT        8

/* track debug */
uint32 track_debug_is_on(uint32 flag);

#define TRACK_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (track_debug_is_on(flag)) \
        log_sys(M_MOD_TRACK, E_DEBUG, fmt, ##args); \
} while (0)

#define TRACK_LOG_PRINT(fmt, args...) \
    log_sys(M_MOD_TRACK, E_DEBUG, fmt, ##args);


int32
track_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
track_cmd_process_object(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
track_cmd_show_track(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* _TRACK_H */

