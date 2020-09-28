
#ifndef _IPSLA_H
#define _IPSLA_H

/* ping */
#define   PACKET_SIZE         10*4096
#define   MAX_WAIT_TIME       5
#define   MAX_NO_PACKETS      1

/* ip sla */
#define   IPSLA_ENTRY_MAX     255

/* return code */
#define   ECHO_UNKNOWN        0
#define   ECHO_OK             1
#define   ECHO_OVERTHRESHOLD  2
#define   ECHO_TIMEOUT        3
#define   ECHO_UNREACHABLE    4

/* type code */
#define   ICMP_TYPE           1

/* error code */
#define   IPSLA_SUCCESS                      0
#define   IPSLA_CREATE_ENTRY_ERR            -1
#define   IPSLA_CREATE_SOCKET_ERR           -2
#define   IPSLA_SET_ERR                     -3
#define   IPSLA_SHOW_ERR                    -4
#define   IPSLA_ENTRY_NOT_EXIST_ERR         -5
#define   IPSLA_ENTRY_NOT_ENABLE_ERR        -6
#define   IPSLA_ENTRY_NOT_DISABLE_ERR       -7
#define   IPSLA_ENTRY_TYPE_ERR              -8
#define   IPSLA_ENTRY_ADDR_ERR              -9
#define   IPSLA_ENTRY_GET_HOSTNAME_ERR      -10
#define   IPSLA_ENTRY_FREQUENCY_ERR         -11
#define   IPSLA_ENTRY_TIMEOUT_ERR           -12
#define   IPSLA_ENTRY_THRESHOLD_ERR         -13
#define   IPSLA_ENTRY_VRF_NOT_EXIST_ERR     -14
#define   IPSLA_ENTRY_BROADCAST_DISALLOWED_ERR  -15
#define   IPSLA_ENTRY_INPUT_IPADDRESS_ERR   -16
#define   IPSLA_ENTRY_SOCKET_SET_ERR        -17
#define   IPSLA_ENTRY_PKT_NEW_ERR           -18
#define   IPSLA_ENTRY_DATA_PATTERN_ERR      -19
#define   IPSLA_ENTRY_STATS_DELTA_ERR       -20
#define   IPSLA_SOCKET_BIND_ERR             -21
#define   IPSLA_GLOBAL_NOT_FOUND_ERR        -22
#define   IPSLA_CREATE_ENTRY_OVER_ERR       -23
#define   IPSLA_ENTRY_SET_OUTRANGE_ERR      -24

/* ping */
#define   IPSLA_PING_SUCCESS                 0
#define   IPSLA_PING_SEND_FAILURE           -1
#define   IPSLA_PING_RECV_FAILURE           -2
#define   ICMP_DATALEN_DEF                   28
#define   ICMP_DATALEN_MAX                   8100
#define   ICMP_DATALEN_MIN                   0
#define   ICMP_PKT_LENGTH(datasize)          (datasize + 8 + 8) /* datasize + timestamp + icmphdr */
#define   ICMP_PKT_BUF                       8136               /* 20+8+8+8100 */
#define   IPSLA_IPADDRESS_LENGTH_MIN         7
#define   IPSLA_IPADDRESS_LENGTH_MAX         15

#define   IPSLA_TYPE_UNKNOWN         0
#define   IPSLA_TYPE_ECHO            1

#define   IPSLA_ENABLE               1
#define   IPSLA_DISABLE              0

#define   IPSLA_ENTRYID_MIN          1
#define   IPSLA_ENTRYID_MAX          255
#define   IPSLA_FREQUENCY_MIN        1
#define   IPSLA_FREQUENCY_MAX        4800
#define   IPSLA_TIMEOUT_MIN          1
#define   IPSLA_TIMEOUT_MAX          4800
#define   IPSLA_INTERVAL_MIN         1
#define   IPSLA_INTERVAL_MAX         4800
#define   IPSLA_THRESHOLD_MIN        1
#define   IPSLA_THRESHOLD_MAX        4800000
#define   IPSLA_TTL_MIN              1
#define   IPSLA_TTL_MAX              255
#define   IPSLA_TOS_MIN              0
#define   IPSLA_TOS_MAX              255
#define   IPSLA_DATA_SIZE_MIN        0
#define   IPSLA_DATA_SIZE_MAX        8100
#define   IPSLA_FAIL_PERCENT_MIN     1
#define   IPSLA_FAIL_PERCENT_MAX     100
#define   IPSLA_PACKETS_PER_TEST_MIN 1
#define   IPSLA_PACKETS_PER_TEST_MAX 10
#define   IPSLA_PROBE_RESULT_CNT_MIN 0
#define   IPSLA_PROBE_RESULT_CNT_MAX 1000
#define   IPSLA_TEST_RESULT_CNT_MIN  1
#define   IPSLA_TEST_RESULT_CNT_MAX  10
#define   IPSLA_DES_MAX_SIZE         255

#define IPSLA_SOCK_FLAG_ERR_BIND_DEVICE      (1 << 0)
#define IPSLA_SOCK_FLAG_ERR_BIND_VRF         (1 << 1)
#define IPSLA_SOCK_FLAG_ERR_CREATE           (1 << 2)
#define IPSLA_SOCK_FLAG_ERR_FILTER           (1 << 3)
#define IPSLA_SOCK_FLAG_ERR_SET_OPTION       (1 << 4)
#define IPSLA_SOCK_FLAG_ERR_BIND_SRCIP       (1 << 5)

enum ipsla_echo_reply_code_e
{
    IPSLA_NO_ECHO_REPLY = 0,
    IPSLA_GET_ECHO_REPLY,
    IPSLA_GET_ECHO_OVERTHRESHOLD,
    IPSLA_GET_ECHO_FAILED,
    IPSLA_ECHO_REPLY_CODE_MAX
};
typedef enum ipsla_echo_reply_code_e ipsla_echo_reply_code_t;

#define   IPSLA_RESULT_PROBE_NUM_DEFAULT      50
#define   IPSLA_RESULT_TEST_NUM_DEFAULT       5


/* ipsla debug */
uint32 ipsla_debug_is_on(uint32 flag);

#define IPSLA_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (ipsla_debug_is_on(flag)) \
        log_sys(M_MOD_IPSLA, E_DEBUG, fmt, ##args); \
} while (0)

#define IPSLA_LOG_PRINT(fmt, args...) \
    log_sys(M_MOD_IPSLA, E_DEBUG, fmt, ##args);


#define IPSLA_TIME_CHECK(timeout, interval, frequency, cnt)                                                                            \
    do {                                                                                                                               \
        if (timeout >= interval)                                                                                                       \
        {                                                                                                                              \
            CFG_CONFLICT_RET("Timeout should be little than interval");                                                              \
            return IPSLA_SET_ERR;                                                                                                      \
        }                                                                                                                              \
                                                                                                                                       \
        if (interval >= frequency)                                                                                                     \
        {                                                                                                                              \
            CFG_CONFLICT_RET("Interval should be little than frequency");                                                            \
            return IPSLA_SET_ERR;                                                                                                      \
        }                                                                                                                              \
                                                                                                                                       \
        if (frequency <= interval * ( cnt - 1 ) + timeout)                                                                             \
        {                                                                                                                              \
            CFG_CONFLICT_RET("User configuration should allowed the rule:frequency > interval * ( packets-per-test - 1 ) + timeout");\
            return IPSLA_SET_ERR;                                                                                                      \
        }                                                                                                                              \
    } while (0)

#define IPSLA_TIME_CHECK_PARAM(timeout, interval, frequency, cnt)  \
        (timeout >= interval || interval >= frequency || (frequency <= interval * ( cnt - 1 ) + timeout))

int32
ipsla_cmd_show_ipsla(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsla_cmd_process_act_clear(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsla_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsla_cmd_process_entry(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* _IPSLA_H */

