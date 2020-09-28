
#ifndef __TBL_IPSLA_ENTRY_DEFINE_H__
#define __TBL_IPSLA_ENTRY_DEFINE_H__

/* TBL_IPSLA_ENTRY field defines */
typedef enum
{
    TBL_IPSLA_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPSLA_ENTRY_FLD_TYPE                 = 1 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_ADMIN_STATE          = 2 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_DESC                 = 3 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_FREQUENCY            = 4 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TIMEOUT              = 5 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_THRESHOLD            = 6 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_INTERVAL             = 7 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_PACKETS_PER_TEST     = 8 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_FAIL_PERCENT         = 9 ,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TTL                  = 10,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TOS                  = 11,  /* RW */
    TBL_IPSLA_ENTRY_FLD_DATA_SIZE            = 12,  /* RW */
    TBL_IPSLA_ENTRY_FLD_DATA_PATTERN         = 13,  /* RW */
    TBL_IPSLA_ENTRY_FLD_INADDR               = 14,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SRC_INADDR           = 15,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SRC_IFNAME           = 16,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SOCK                 = 17,  /* READ */
    TBL_IPSLA_ENTRY_FLD_SEND_SOCK            = 18,  /* READ */
    TBL_IPSLA_ENTRY_FLD_SOCK_FLAG            = 19,  /* RW */
    TBL_IPSLA_ENTRY_FLD_BUF                  = 20,  /* RW */
    TBL_IPSLA_ENTRY_FLD_PACKET_LENGTH        = 21,  /* RW */
    TBL_IPSLA_ENTRY_FLD_T_IPSLA_READ         = 22,  /* READ */
    TBL_IPSLA_ENTRY_FLD_TESTID               = 23,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_FREQUENCY    = 24,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_TIMEOUT      = 25,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_INTERVAL     = 26,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_PROBE_CNT    = 27,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SEQUENCE             = 28,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RTT                  = 29,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SEND_TIME            = 30,  /* RW */
    TBL_IPSLA_ENTRY_FLD_LAST_SEND_TIME       = 31,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SEND_CNT             = 32,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RECEIVE_CNT          = 33,  /* RW */
    TBL_IPSLA_ENTRY_FLD_OVERTHRESHOLD_CNT    = 34,  /* RW */
    TBL_IPSLA_ENTRY_FLD_SENDFAIL_CNT         = 35,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MIN      = 36,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_MAX      = 37,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_RTT_SUM      = 38,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_SUCCESS      = 39,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RUNNING_FAIL_PERCENT = 40,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RETURNCODE           = 41,  /* RW */
    TBL_IPSLA_ENTRY_FLD_ECHO_REPLY           = 42,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TOTAL_RECEIVE        = 43,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TOTAL_SEND           = 44,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RTT_SUM              = 45,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RTT_MAX              = 46,  /* RW */
    TBL_IPSLA_ENTRY_FLD_RTT_MIN              = 47,  /* RW */
    TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_CONFIG_CNT = 48,  /* RW */
    TBL_IPSLA_ENTRY_FLD_PROBE_RESULT_INDEX   = 49,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TEST_RESULT_CONFIG_CNT = 50,  /* RW */
    TBL_IPSLA_ENTRY_FLD_TEST_RESULT_INDEX    = 51,  /* RW */
    TBL_IPSLA_ENTRY_FLD_MAX                  = 52
} tbl_ipsla_entry_field_id_t;

/* TBL_IPSLA_ENTRY defines */
typedef struct
{
    uint16               entry_id;
} tbl_ipsla_entry_key_t;

typedef struct
{
    tbl_ipsla_entry_key_t key;
    uint8                type;                /* IPSLA_TYPE_ECHO */
    uint8                admin_state;         /* Current Admin State */
    char                 desc[IPSLA_DES_MAX_SIZE+1]; /* Description */
    uint16               frequency;
    uint16               timeout;
    uint32               threshold;
    uint16               interval;
    uint16               packets_per_test;
    uint8                fail_percent;
    uint8                ttl;
    uint8                tos;
    uint16               data_size;
    uint32               data_pattern;        /* Data Pattern Default 0xABCDABCD */
    addr_ipv4_t          inaddr;              /* Dest IP */
    addr_ipv4_t          src_inaddr;          /* Source IP */
    char                 src_ifname[IFNAME_SIZE]; /* Source interface name */
    int32                sock;
    int32                send_sock;
    int32                sock_flag;
    uint8                buf[GLB_IPSLA_PKT_BUF]; /* Packet Buf */
    uint32               packet_length;
    ctc_task_t           *t_ipsla_read;
    uint32               testid;              /* Running Time */
    uint16               running_frequency;
    uint16               running_timeout;
    uint16               running_interval;
    uint16               running_probe_cnt;
    uint16               sequence;
    uint32               rtt;
    uint64               send_time;
    uint64               last_send_time;
    uint8                send_cnt;
    uint8                receive_cnt;
    uint8                overthreshold_cnt;
    uint8                sendfail_cnt;
    uint32               running_rtt_min;
    uint32               running_rtt_max;
    uint32               running_rtt_sum;
    uint8                running_success;
    uint8                running_fail_percent;
    uint8                returncode;          /* result and statisitcs */
    uint8                echo_reply;
    uint32               total_receive;
    uint32               total_send;
    uint64               rtt_sum;
    uint32               rtt_max;
    uint32               rtt_min;
    uint16               probe_result_config_cnt; /* statistics config */
    uint16               probe_result_index;
    uint16               test_result_config_cnt;
    uint16               test_result_index;
} tbl_ipsla_entry_t;

typedef struct
{
    tbl_ipsla_entry_t    *ipslaentry_array[GLB_IPSLA_ENTRY_MAX+1];
} tbl_ipsla_entry_master_t;

#endif /* !__TBL_IPSLA_ENTRY_DEFINE_H__ */

