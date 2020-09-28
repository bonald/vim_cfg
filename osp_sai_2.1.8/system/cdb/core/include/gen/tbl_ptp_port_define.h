
#ifndef __TBL_PTP_PORT_DEFINE_H__
#define __TBL_PTP_PORT_DEFINE_H__

/* TBL_PTP_PORT field defines */
typedef enum
{
    TBL_PTP_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_PTP_PORT_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_PTP_PORT_FLD_HSRV_PATH_DELAY      = 2 ,  /* RW */
    TBL_PTP_PORT_FLD_PORT_DS              = 3 ,  /* RW */
    TBL_PTP_PORT_FLD_PORT_ADDR            = 4 ,  /* RW */
    TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION = 5 ,  /* RW */
    TBL_PTP_PORT_FLD_INGRESS_LATENCY      = 6 ,  /* RW */
    TBL_PTP_PORT_FLD_EGRESS_LATENCY       = 7 ,  /* RW */
    TBL_PTP_PORT_FLD_DUMP_FLAG            = 8 ,  /* RW */
    TBL_PTP_PORT_FLD_ENABLE               = 9 ,  /* RW */
    TBL_PTP_PORT_FLD_LAG_ID               = 10,  /* RW */
    TBL_PTP_PORT_FLD_LINK_UP              = 11,  /* RW */
    TBL_PTP_PORT_FLD_DROP_ANNOUNCE        = 12,  /* RW */
    TBL_PTP_PORT_FLD_PACKET_STATS         = 13,  /* RW */
    TBL_PTP_PORT_FLD_VLAN_INFO            = 14,  /* RW */
    TBL_PTP_PORT_FLD_RECOMMENDED_STATE    = 15,  /* RW */
    TBL_PTP_PORT_FLD_BMC_CODE             = 16,  /* RW */
    TBL_PTP_PORT_FLD_BMC_EXPIRES          = 17,  /* RW */
    TBL_PTP_PORT_FLD_SLAVE_CHANGE_MASTER  = 18,  /* RW */
    TBL_PTP_PORT_FLD_SYNCHRONIZATION_FAULT = 19,  /* RW */
    TBL_PTP_PORT_FLD_MASTER_CLOCK_SELECTED = 20,  /* RW */
    TBL_PTP_PORT_FLD_PTP_SEQ_ID           = 21,  /* RW */
    TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_OFFSET_TIMES = 22,  /* RW */
    TBL_PTP_PORT_FLD_UNCALIBRATED_UPDATE_LIMIT_TIMES = 23,  /* RW */
    TBL_PTP_PORT_FLD_P                    = 24,  /* RW */
    TBL_PTP_PORT_FLD_C                    = 25,  /* RW */
    TBL_PTP_PORT_FLD_PTM_ANNOUNCE_TIMER   = 26,  /* READ */
    TBL_PTP_PORT_FLD_PTM_SYNC_TIMER       = 27,  /* READ */
    TBL_PTP_PORT_FLD_PTM_DELAY_TIMER      = 28,  /* READ */
    TBL_PTP_PORT_FLD_PTM_PDELAY_TIMER     = 29,  /* READ */
    TBL_PTP_PORT_FLD_PTM_ANNOUNCE_RECEIPT_TIMER = 30,  /* READ */
    TBL_PTP_PORT_FLD_PTM_QUALIFICATION_TIMER = 31,  /* READ */
    TBL_PTP_PORT_FLD_P_TIMER_FAULTY_RECOVER_TIMER = 32,  /* READ */
    TBL_PTP_PORT_FLD_MSG_PROC_OPS         = 33,  /* READ */
    TBL_PTP_PORT_FLD_MAX                  = 34
} tbl_ptp_port_field_id_t;

/* TBL_PTP_PORT defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_ptp_port_key_t;

typedef struct
{
    tbl_ptp_port_key_t   key;
    uint32               ifindex;
    int64                hsrv_path_delay;     /*for hsrv set path delay*/
    ptp_port_t           port_ds;
    glb_ptp_port_address_t port_addr;           /*ptp protocol (mac|udp)*/
    glb_ptp_timeinterval_t asymmetry_correction; /*ptp asymmetry-correction CORRECTION<-2000000, 2000000>*/
    int32                ingress_latency;     /*ptp ingress-latency <0-65535>*/
    int32                egress_latency;      /*ptp egress-latency <0-65535>*/
    ptp_dump_flag_t      dump_flag;           /*debug*/
    uint32               enable;              /*interface enable*/
    uint32               lag_id;              /*belong to agg lag_id*/
    uint32               link_up;             /*interface up status*/
    uint32               drop_announce;       /*ptp drop-announce*//*not support*/
    ptp_port_stats_t     packet_stats;        /*packet statistics*/
    ptp_vlan_info_t      vlan_info;           /*ptp tagging vlan <1-4094> (cos <0-7>|)*/
    uint8                recommended_state;   /*recommended state by BMC*/
    uint8                bmc_code;            /*update DS by this code */
    uint8                bmc_expires;         /*expire type*/
    uint32               slave_change_master; /*slave change master flag*/
    uint32               synchronization_fault; /*exsit fault in synchronization*/
    uint32               master_clock_selected;
    uint16               ptp_seq_id;          /*packet seq id, for get time stamp*/
    uint8                uncalibrated_update_offset_times;
    uint8                uncalibrated_update_limit_times;
    ptp_port_p_t         p;                   /*about packet sequence_id /send_time / rcv time /delay m_s*/
    ptp_port_c_t         c;                   /*about delay calc*/
    void                 *rbest;              /*port r best*/
    ctc_task_t           *ptm_announce_timer;
    ctc_task_t           *ptm_sync_timer;
    ctc_task_t           *ptm_delay_timer;
    ctc_task_t           *ptm_pdelay_timer;
    ctc_task_t           *ptm_announce_receipt_timer;
    ctc_task_t           *ptm_qualification_timer;
    ctc_task_t           *p_timer_faulty_recover_timer;
    ptp_msg_proc_ops_t   msg_proc_ops;        /*according to different clock type and state to register different packet process function*/
} tbl_ptp_port_t;

typedef struct
{
    ctclib_hash_t        *ptp_port_hash;
    ctclib_slist_t       *ptp_port_list;
} tbl_ptp_port_master_t;

#endif /* !__TBL_PTP_PORT_DEFINE_H__ */

