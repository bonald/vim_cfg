#ifndef __TPOAM_LM_H__
#define __TPOAM_LM_H__

#define TPOAM_LM_PRIORITY_FOR_ALL_COS      8

/* LM status for CLI */
#define NSM_TPOAM_PER_LMM_STATUS_WAITING    1           /* LMM waiting for LMR */
#define NSM_TPOAM_PER_LMM_STATUS_RX_LMR     2           /* LMM recveive a valid LMR */

enum tpoam_direction_e
{
    TPOAM_DIR_EAST,
    TPOAM_DIR_WEST,
    TPOAM_DIR_NONE, /* ignore for tunnel */
    TPOAM_DIR_MAX
};
typedef enum tpoam_direction_e tpoam_direction_t;

struct tpoam_lm_single_enable_input
{
  enum tpoam_lm_cos_type cos_type;
  uint8  cos_value;
  /* modified by yaom for proactive single lm 20140416 */
  uint8  interval;
  uint16  cache_size;
  uint32    is_proactive;
};
typedef struct tpoam_lm_single_enable_input tpoam_lm_single_enable_input_t;


struct tpoam_single_lm_entry_s
{
    /* counter value */
    uint32  TxFCf;
    uint32  RxFCf;
    uint32  TxFCb;
    uint32  RxFCl;

    /* lmm cos, used for per-cos */
    uint32  lmm_cos_value;
    
    /* Far end frame loss from start time */
    /* Frame Loss (far-end) = |TxFCf[tc] 每 TxFCf[tp]| 每 |RxFCf[tc] 每 RxFCf[tp]| */
    uint32 lm_far_end_frame_loss; 

    /* Frame Loss ration = Frame Loss * 1000000 / |TxFCf[tc] 每 TxFCf[tp]| */
    uint32 lm_far_end_frame_loss_ratio; 

    /* Near end frame loss from start time */
    /* Frame Loss (near-end) = |TxFCb[tc] 每 TxFCb[tp]| 每 |RxFCl[tc] 每 RxFCl[tp]| */
    uint32 lm_near_end_frame_loss;

    /* Frame Loss ration = Frame Loss * 1000000 / |TxFCb[tc] 每 TxFCb[tp]| */
    uint32 lm_near_end_frame_loss_ratio; 

    /* receive time */
    sal_time_t  recv_time;

    /* modified by yaom for proactive single lm 20140416 */
    uint32 error;
};
typedef struct tpoam_single_lm_entry_s tpoam_single_lm_entry_t;


struct tpoam_dual_lm_entry_s
{
    /* counter value */
    uint32  TxFCf;
    uint32  RxFCb;
    uint32  TxFCb;
    uint32  RxFCl;

    /* ccm cos, used for per-cos */
    uint32  ccm_cos_value;
    
    /* Far end frame loss from start time */
    /* Frame Loss (far-end) = |TxFCb[tc] - TxFCb[tp]| - |RxFCb[tc] - RxFCb[tp]| */
    uint32 lm_far_end_frame_loss; 

    /* Frame Loss ration = Frame Loss * 1000000 / |TxFCb[tc] - TxFCb[tp]| */
    uint32 lm_far_end_frame_loss_ratio; 

    /* Near end frame loss from start time */
    /* Frame Loss (near-end) = |TxFCf[tc] - TxFCf[tp]| - |RxFCl[tc] - RxFCl[tp]| */
    uint32 lm_near_end_frame_loss;

    /* Frame Loss ration = Frame Loss * 1000000 / |TxFCf[tc] - TxFCf[tp]| */
    uint32 lm_near_end_frame_loss_ratio; 

    /* receive time */
    sal_time_t  recv_time;
    uint32 signal_status;
};
typedef struct tpoam_dual_lm_entry_s tpoam_dual_lm_entry_t;


struct tpoam_lm_dual_enable_input
{
  enum tpoam_lm_cos_type cos_type;
  uint8  cos_value;
  uint8  stats_interval;
  uint16  cache_size;
};
typedef struct tpoam_lm_dual_enable_input tpoam_lm_dual_enable_input_t;

uint32
tpoam_lm_get_local_threshold(tbl_oam_session_t* pst_oam);

uint32
tpoam_lm_get_remote_threshold(tbl_oam_session_t* pst_oam);

void
tpoam_lm_destroy_list_dual_lm(tbl_oam_session_t* pst_oam);

int32
tpoam_lm_rx_lmm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_lm_rx_lmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

void
tpoam_lm_send_proactive_lmm(void* p_data);

int32
tpoam_lm_start_calc();

int32
tpoam_lm_stop_calc();

//int32
//tpoam_lm_stop(tbl_oam_session_t* pst_oam);

int32
tpoam_lm_dual_counters_handle_hsrv_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

//int32
//tpoam_lm_init(tbl_oam_session_t* pst_oam);

//int32
//tpoam_lm_deinit(tbl_oam_session_t* pst_oam);
#endif /* !__TPOAM_LM_H__ */
