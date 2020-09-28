
#ifndef __TBL_PTP_DEBUG_DEFINE_H__
#define __TBL_PTP_DEBUG_DEFINE_H__

/* TBL_PTP_DEBUG field defines */
typedef enum
{
    TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG = 0 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG        = 1 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG      = 2 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG = 3 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG  = 4 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG  = 5 ,  /* RW */
    TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG = 6 ,  /* RW */
    TBL_PTP_DEBUG_FLD_MAX                  = 7 
} tbl_ptp_debug_field_id_t;

/* TBL_PTP_DEBUG defines */
typedef struct
{
    uint32               ptp;                 /* bitmap of PTPDBG_FLAG_ */
} tbl_ptp_debug_t;

#endif /* !__TBL_PTP_DEBUG_DEFINE_H__ */

