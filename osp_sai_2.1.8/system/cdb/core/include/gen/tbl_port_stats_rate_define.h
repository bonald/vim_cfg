
#ifndef __TBL_PORT_STATS_RATE_DEFINE_H__
#define __TBL_PORT_STATS_RATE_DEFINE_H__

/* TBL_PORT_STATS_RATE field defines */
typedef enum
{
    TBL_PORT_STATS_RATE_FLD_KEY                  = 0 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_PORT_ID              = 1 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OCTETS_RCV_RATE      = 2 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_PKTS_RCV_RATE        = 3 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OCTETS_SEND_RATE     = 4 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_PKTS_SEND_RATE       = 5 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL        = 6 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_INPUT_RATE           = 7 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_INPUT_RATE_STATE     = 8 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD = 9 ,  /* RW */
    TBL_PORT_STATS_RATE_FLD_INPUT_RATE_RESUME    = 10,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE          = 11,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_STATE    = 12,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD = 13,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_RESUME   = 14,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD = 15,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL = 16,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_PACKETS = 17,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_TOTAL_PACKETS = 18,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_STATE = 19,  /* RW */
    TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_RUNNING_INTERVAL = 20,  /* RW */
    TBL_PORT_STATS_RATE_FLD_T_OUTPUT_DISCARD_TIMER = 21,  /* READ */
    TBL_PORT_STATS_RATE_FLD_MAX                  = 22
} tbl_port_stats_rate_field_id_t;

/* TBL_PORT_STATS_RATE defines */
typedef struct
{
    uint32               ifindex;
} tbl_port_stats_rate_key_t;

typedef struct
{
    tbl_port_stats_rate_key_t key;
    uint32               port_id;
    uint64               octets_rcv_rate;
    uint64               pkts_rcv_rate;
    uint64               octets_send_rate;
    uint64               pkts_send_rate;
    uint16               load_interval;       /* load interval */
    uint8                input_rate;          /* bandwidth usage rate */
    uint8                input_rate_state;    /* bandwidth state for syslog */
    uint8                input_rate_threshold; /* bandwidth threshold for syslog */
    uint8                input_rate_resume;   /* bandwidth resume rate for syslog */
    uint8                output_rate;         /* bandwidth usage rate */
    uint8                output_rate_state;   /* bandwidth state for syslog */
    uint8                output_rate_threshold; /* bandwidth threshold for syslog */
    uint8                output_rate_resume;  /* bandwidth resume rate for syslog */
    uint64               output_discard_threshold; /* discard threshold for syslog */
    uint32               output_discard_interval; /* discard stats interval for syslog */
    uint64               output_discard_packets; /* discard packets in a interval */
    uint64               output_discard_total_packets; /* discard packets get from qos queue */
    uint8                output_discard_state; /* discard state for syslog */
    uint32               output_discard_running_interval; /* discard running interval for syslog */
    ctc_task_t           *t_output_discard_timer; /* discard timer */
} tbl_port_stats_rate_t;

typedef struct
{
    ctclib_hash_t        *port_stats_rate_hash;
    ctclib_slist_t       *port_stats_rate_list;
} tbl_port_stats_rate_master_t;

#endif /* !__TBL_PORT_STATS_RATE_DEFINE_H__ */

