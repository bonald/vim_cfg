
#ifndef __TBL_FEA_PTP_GLOBAL_DEFINE_H__
#define __TBL_FEA_PTP_GLOBAL_DEFINE_H__

/* TBL_FEA_PTP_GLOBAL field defines */
typedef enum
{
    TBL_FEA_PTP_GLOBAL_FLD_GLOBAL_ENABLE        = 0 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_DEVICE_TYPE          = 1 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_RETRY_CNT            = 2 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_READY                = 3 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_BLOCKED              = 4 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_TYPE                 = 5 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_TX_IFINDEX           = 6 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_TX_SEQ_ID            = 7 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_INTR_COUNT           = 8 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE = 9 ,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_HSRV_PTP_TIMER       = 10,  /* RW */
    TBL_FEA_PTP_GLOBAL_FLD_MAX                  = 11
} tbl_fea_ptp_global_field_id_t;

/* TBL_FEA_PTP_GLOBAL defines */
typedef struct
{
    uint32               global_enable;
    uint16               device_type;
    uint8                retry_cnt;
    uint32               ready;
    uint32               blocked;
    uint32               type;
    uint32               tx_ifindex;
    uint16               tx_seq_id;
    uint16               intr_count;
    uint32               acl_update_time_range; /*update time range*/
    ctc_task_t           *hsrv_ptp_timer;
} tbl_fea_ptp_global_t;

#endif /* !__TBL_FEA_PTP_GLOBAL_DEFINE_H__ */

