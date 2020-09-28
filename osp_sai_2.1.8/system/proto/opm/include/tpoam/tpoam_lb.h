#ifndef __TPOAM_LB_H__
#define __TPOAM_LB_H__

#define TPOAM_PER_LB_STATUS_WAITING         1       /* LBM timeout */
#define TPOAM_PER_LB_STATUS_RX_LBR          2       /* LBM recveive a valid LBR in time */
#define TPOAM_PER_LB_STATUS_TO              3       /* LBM waiting for LBR */

#define TPOAM_Y1731_LBM_TX_CNT_DEF      1
#define TPOAM_Y1731_LBM_TX_CNT_MIN      1
#define TPOAM_Y1731_LBM_TX_CNT_MAX      255

#define TPOAM_Y1731_LBM_TIMEOUT_DEF     5
#define TPOAM_Y1731_LBM_TIMEOUT_MIN     1
#define TPOAM_Y1731_LBM_TIMEOUT_MAX     255

#define TPOAM_Y1731_LBM_CMD_TIMEOUT_DEF     10000

int32
tpoam_lb_rx_lbm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_lb_rx_lbr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_lb_start_lb_check(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para);

int32
tpoam_lb_start_lb_send(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para);

int32
tpoam_lb_start_lb(tbl_oam_session_t* pst_oam, tpoam_lbm_para_t* pst_lbm_para);

int32
tpoam_lb_init(tbl_oam_session_t* pst_oam);

int32
tpoam_lb_deinit(tbl_oam_session_t* pst_oam);

int32
tpoam_lb_stop_lb(tbl_oam_session_t* pst_oam);

void
tpoam_lb_clean(tbl_oam_session_t* pst_oam);

#endif /* !__TPOAM_LB_H__ */