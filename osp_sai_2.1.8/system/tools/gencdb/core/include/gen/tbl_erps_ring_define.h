
#ifndef __TBL_ERPS_RING_DEFINE_H__
#define __TBL_ERPS_RING_DEFINE_H__

/* TBL_ERPS_RING field defines */
typedef enum
{
    TBL_ERPS_RING_FLD_KEY                  = 0 ,  /* READ */
    TBL_ERPS_RING_FLD_ERPS_RING_STATE      = 1 ,  /* RW */
    TBL_ERPS_RING_FLD_ERPS_RING_LEVEL      = 2 ,  /* RW */
    TBL_ERPS_RING_FLD_INSTANCE_ID_NUM      = 3 ,  /* RW */
    TBL_ERPS_RING_FLD_INSTANCE_ID          = 4 ,  /* RW */
    TBL_ERPS_RING_FLD_NODE_ROLE            = 5 ,  /* RW */
    TBL_ERPS_RING_FLD_EDGE_NODE_ROLE       = 6 ,  /* RW */
    TBL_ERPS_RING_FLD_ERPS_SRPT_ENABLE     = 7 ,  /* RW */
    TBL_ERPS_RING_FLD_PRIMARY_PORT_IFINDEX = 8 ,  /* RW */
    TBL_ERPS_RING_FLD_SECONDARY_PORT_IFINDEX = 9 ,  /* RW */
    TBL_ERPS_RING_FLD_EDGE_PORT_IFINDEX    = 10,  /* RW */
    TBL_ERPS_RING_FLD_COMMON_PORT_IFINDEX  = 11,  /* RW */
    TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX = 12,  /* RW */
    TBL_ERPS_RING_FLD_BLOCKED              = 13,  /* RW */
    TBL_ERPS_RING_FLD_HELLO_TIMER          = 14,  /* READ */
    TBL_ERPS_RING_FLD_FAILURE_TIMER        = 15,  /* READ */
    TBL_ERPS_RING_FLD_PRE_FORWARDING_TIMER = 16,  /* READ */
    TBL_ERPS_RING_FLD_FLUSH_TIMER          = 17,  /* READ */
    TBL_ERPS_RING_FLD_EDGE_HELLO_TIMER     = 18,  /* READ */
    TBL_ERPS_RING_FLD_EDGE_FAILURE_TIMER   = 19,  /* READ */
    TBL_ERPS_RING_FLD_EDGE_BLOCK_TIMER     = 20,  /* READ */
    TBL_ERPS_RING_FLD_HELLO_SEQ            = 21,  /* RW */
    TBL_ERPS_RING_FLD_EXPECT_HELLO_SEQ     = 22,  /* RW */
    TBL_ERPS_RING_FLD_RCV_HELLO_CNT        = 23,  /* READ */
    TBL_ERPS_RING_FLD_RCV_RING_UP_FLUSH_FDB_CNT = 24,  /* READ */
    TBL_ERPS_RING_FLD_RCV_RING_DOWN_FLUSH_FDB_CNT = 25,  /* READ */
    TBL_ERPS_RING_FLD_RCV_LINK_DOWN_CNT    = 26,  /* READ */
    TBL_ERPS_RING_FLD_RCV_EDGE_HELLO_CNT   = 27,  /* READ */
    TBL_ERPS_RING_FLD_RCV_MAJOR_FAULT_CNT  = 28,  /* READ */
    TBL_ERPS_RING_FLD_SEND_HELLO_CNT       = 29,  /* READ */
    TBL_ERPS_RING_FLD_SEND_RING_UP_FLUSH_FDB_CNT = 30,  /* READ */
    TBL_ERPS_RING_FLD_SEND_RING_DOWN_FLUSH_FDB_CNT = 31,  /* READ */
    TBL_ERPS_RING_FLD_SEND_LINK_DOWN_CNT   = 32,  /* READ */
    TBL_ERPS_RING_FLD_SEND_EDGE_HELLO_CNT  = 33,  /* READ */
    TBL_ERPS_RING_FLD_SEND_MAJOR_FAULT_CNT = 34,  /* READ */
    TBL_ERPS_RING_FLD_MAX                  = 35
} tbl_erps_ring_field_id_t;

/* TBL_ERPS_RING defines */
typedef erps_ring_key_t tbl_erps_ring_key_t;

typedef struct
{
    tbl_erps_ring_key_t  key;
    uint8                erps_ring_state;
    uint8                erps_ring_level;
    uint32               instance_id_num;
    uint32               instance_id[ERPS_MAX_INSTANCE];
    uint8                node_role;
    uint8                edge_node_role;
    uint32               erps_srpt_enable;
    uint32               primary_port_ifindex;
    uint32               secondary_port_ifindex;
    uint32               edge_port_ifindex;
    uint32               common_port_ifindex;
    uint32               blocked_port_ifindex;
    uint32               blocked;
    void                 *primary_port_if;    /* primary port interface, refer to tbl_interface_t */
    void                 *secondary_port_if;  /* secondary port interface, refer to tbl_interface_t */
    void                 *edge_port_if;       /* edge port interface, refer to tbl_interface_t */
    void                 *common_port_if;     /* common port interface, refer to tbl_interface_t */
    void                 *blocked_port_if;    /* blocked port interface, refer to tbl_interface_t */
    ctc_task_t           *hello_timer;        /* hello timer */
    ctc_task_t           *failure_timer;      /* failure timer */
    ctc_task_t           *pre_forwarding_timer; /* pre-forwarding timer */
    ctc_task_t           *flush_timer;
    ctc_task_t           *edge_hello_timer;
    ctc_task_t           *edge_failure_timer;
    ctc_task_t           *edge_block_timer;
    uint32               hello_seq;
    uint32               expect_hello_seq;
    uint32               rcv_hello_cnt;
    uint32               rcv_ring_up_flush_fdb_cnt;
    uint32               rcv_ring_down_flush_fdb_cnt;
    uint32               rcv_link_down_cnt;
    uint32               rcv_edge_hello_cnt;
    uint32               rcv_major_fault_cnt;
    uint32               send_hello_cnt;
    uint32               send_ring_up_flush_fdb_cnt;
    uint32               send_ring_down_flush_fdb_cnt;
    uint32               send_link_down_cnt;
    uint32               send_edge_hello_cnt;
    uint32               send_major_fault_cnt;
    void                 *p_erps_domain;      /* refer to tbl_erps_domain_t */
} tbl_erps_ring_t;

typedef struct
{
    ctclib_hash_t        *erps_ring_hash;
    ctclib_slist_t       *erps_ring_list;
} tbl_erps_ring_master_t;

#endif /* !__TBL_ERPS_RING_DEFINE_H__ */

