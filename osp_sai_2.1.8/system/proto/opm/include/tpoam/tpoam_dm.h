#ifndef __TPOAM_DM_H__
#define __TPOAM_DM_H__

/* DM status for CLI */
#define TPOAM_PER_DMM_STATUS_WAITING    1           /* DMM waiting for DMR */
#define TPOAM_PER_DMM_STATUS_RX_DMR     2           /* DMM recveive a valid DMR */

/* 1000000000 ns = 1s */
#define TPOAM_NS_PER_SEC                1000000000

int32
_tpoam_dm_event_clear(tbl_oam_session_t* pst_oam, tbl_oam_session_upload_t * pst_oam_upload);

uint32
tpoam_get_delay_threshold(tbl_oam_session_t* pst_oam);

int32
tpoam_dm_clear_1dm(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_lm_dual_enable_input_t* bb);

void
tpoam_dm_destroy_list_dmm(tbl_oam_session_t* pst_oam);

int32
tpoam_dm_deinit_dm_session(tbl_oam_session_t* pst_oam);

int32
tpoam_dm_start(tbl_oam_session_t* pst_oam, tpoam_dm_para_t* pst_para, uint32 on_demand);

int32
tpoam_dm_rx_1dm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_dm_rx_dmr(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_dm_rx_dmm(tbl_oam_session_t* pst_oam, tpoam_rx_pdu_t* pst_pdu);

int32
tpoam_dm_stop(tbl_oam_session_t* pst_oam);

int32
tpoam_dm_init(tbl_oam_session_t* pst_oam);

int32
tpoam_dm_deinit(tbl_oam_session_t* pst_oam);

void
tpoam_dm_para_init(tpoam_dm_para_t* pst_para);

#endif /* !__TPOAM_DM_H__ */