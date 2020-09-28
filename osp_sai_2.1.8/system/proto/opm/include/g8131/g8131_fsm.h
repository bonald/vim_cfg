#ifndef _G8131_FSM_H_
#define _G8131_FSM_H_

#define G8131_FSM_STATE_STR_SHORT(STATE)         \
  ((STATE) == G8131_FSM_STATE_NR_W ?   "NR_W" :  \
   (STATE) == G8131_FSM_STATE_NR_P ?   "NR_P" :  \
   (STATE) == G8131_FSM_STATE_LO ?     "LO" :    \
   (STATE) == G8131_FSM_STATE_FS ?     "FS" :    \
   (STATE) == G8131_FSM_STATE_SF_W ?   "SF_W" :  \
   (STATE) == G8131_FSM_STATE_SF_P ?   "SF_P" :  \
   (STATE) == G8131_FSM_STATE_SD_W ?   "SD_W" :  \
   (STATE) == G8131_FSM_STATE_SD_P ?   "SD_P" :  \
   (STATE) == G8131_FSM_STATE_MS_P?    "MS_P" :  \
   (STATE) == G8131_FSM_STATE_MS_W?    "MS_W" :  \
   (STATE) == G8131_FSM_STATE_WTR ?    "WTR":    \
   (STATE) == G8131_FSM_STATE_DNR ?    "DNR":    \
   (STATE) == G8131_FSM_STATE_EXER_W ? "EX_W":   \
   (STATE) == G8131_FSM_STATE_EXER_P?  "EX_P" :  \
   (STATE) == G8131_FSM_STATE_RR_W ?   "RR_W" :  \
   (STATE) == G8131_FSM_STATE_RR_P ?   "RR_P" :  \
                                       "N/A")

/* Macro to convert G8131 FSM State value to string */
#define G8131_FSM_STATE_STR(STATE)                                         \
  ((STATE) == G8131_FSM_STATE_NR_W ?   "No Request on working" :                   \
   (STATE) == G8131_FSM_STATE_NR_P ?   "No Request on protection" :                   \
   (STATE) == G8131_FSM_STATE_LO ?     "Lockout Of Protection" :        \
   (STATE) == G8131_FSM_STATE_FS ?     "Forced Switch" :                \
   (STATE) == G8131_FSM_STATE_SF_W ?   "Signal Fail on Working" :       \
   (STATE) == G8131_FSM_STATE_SF_P ?   "Signal Fail on Protection" :    \
   (STATE) == G8131_FSM_STATE_SD_W ?   "Signal Degrade on Working" :    \
   (STATE) == G8131_FSM_STATE_SD_P ?   "Signal Degrade on Protection" : \
   (STATE) == G8131_FSM_STATE_MS_P?    "Manual Switch to Protection" :  \
   (STATE) == G8131_FSM_STATE_MS_W?    "Manual Switch to Working" :     \
   (STATE) == G8131_FSM_STATE_WTR ?    "Wait-to-Restore":               \
   (STATE) == G8131_FSM_STATE_DNR ?    "Do Not Revert":                 \
   (STATE) == G8131_FSM_STATE_EXER_W ? "Exercise on working" :                     \
   (STATE) == G8131_FSM_STATE_EXER_P ? "Exercise on protection" :                     \
   (STATE) == G8131_FSM_STATE_RR_W ?   "Reverse Request on Working" :   \
   (STATE) == G8131_FSM_STATE_RR_P ?   "Reverse Request on Working" :   \
                                       "Invalid")

#define G8131_FSM_EVENT_STR_SHORT(EVENT)                        \
  ((EVENT) == G8131_FSM_L_EVENT_LO ?                "LO(L)" :   \
   (EVENT) == G8131_FSM_L_EVENT_FS ?                "FS(L)" :   \
   (EVENT) == G8131_FSM_L_EVENT_SF_W ?              "SF_W(L)":  \
   (EVENT) == G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF ? "WRSF(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_SF_P ?              "SF_P(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF ? "PRSF(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_SD_W ?              "SD_W(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_W_RCOVERS_FROM_SD ? "WRSD(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_SD_P ?              "SD_P(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_P_RCOVERS_FROM_SD ? "PRSD(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_MS_P ?              "MS_P(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_MS_W ?              "MS_W(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_CLEAR ?             "CLR(L)" :  \
   (EVENT) == G8131_FSM_L_EVENT_EXER ?              "EXER(L)" : \
   (EVENT) == G8131_FSM_L_EVENT_WTR ?               "WTR(L)" :  \
   (EVENT) == G8131_FSM_R_EVENT_LO ?                "LO(R)" :   \
   (EVENT) == G8131_FSM_R_EVENT_SF_P ?              "SF_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_FS ?                "FS(R)" :   \
   (EVENT) == G8131_FSM_R_EVENT_SF_W ?              "SF_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_SD_W ?              "SD_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_SD_P ?              "SD_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_MS_P?               "MS_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_MS_W?               "MS_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_WTR ?               "WTR(R)" :  \
   (EVENT) == G8131_FSM_R_EVENT_EXER_W ?            "EX_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_EXER_P ?            "EX_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_RR_W?               "RR_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_RR_P?               "RR_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_NR_NULL ?           "NR_W(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_NR_NORMAL ?         "NR_P(R)" : \
   (EVENT) == G8131_FSM_R_EVENT_DNR ?               "DNR(R)" :  \
                                                    "N/A")

/* Macro to convert G8131 FAM Event value to string */
#define G8131_FSM_EVENT_STR(EVENT)                                                       \
  ((EVENT) == G8131_FSM_L_EVENT_LO ?                "Lockout(L)" :                    \
   (EVENT) == G8131_FSM_L_EVENT_FS ?                "Forced switch(L)" :              \
   (EVENT) == G8131_FSM_L_EVENT_SF_W ?              "SF on working(L)":               \
   (EVENT) == G8131_FSM_L_EVENT_W_RCOVERS_FROM_SF ? "Working recovers from SF(L)" :   \
   (EVENT) == G8131_FSM_L_EVENT_SF_P ?              "SF on protection(L)" :           \
   (EVENT) == G8131_FSM_L_EVENT_P_RCOVERS_FROM_SF ? "Protection recovers from SF(L)" :\
   (EVENT) == G8131_FSM_L_EVENT_SD_W ?              "SD on working(L)" :              \
   (EVENT) == G8131_FSM_L_EVENT_W_RCOVERS_FROM_SD ? "Working recovers from SD(L)" :   \
   (EVENT) == G8131_FSM_L_EVENT_SD_P ?              "SD on protection(L)" :           \
   (EVENT) == G8131_FSM_L_EVENT_P_RCOVERS_FROM_SD ? "Protection recovers from SD(L)" :\
   (EVENT) == G8131_FSM_L_EVENT_MS_P ?              "Manual switch to protection(L)" :\
   (EVENT) == G8131_FSM_L_EVENT_MS_W ?              "Manual switch to working(L)" :   \
   (EVENT) == G8131_FSM_L_EVENT_CLEAR ?             "Clear(L)" :                      \
   (EVENT) == G8131_FSM_L_EVENT_EXER ?              "Exercise(L)" :                   \
   (EVENT) == G8131_FSM_L_EVENT_WTR ?               "WTR timer expires(L)" :          \
   (EVENT) == G8131_FSM_R_EVENT_LO ?                "Lockout(R)" :                         \
   (EVENT) == G8131_FSM_R_EVENT_SF_P ?              "SF on protection(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_FS ?                "Forced switch(R)" :                         \
   (EVENT) == G8131_FSM_R_EVENT_SF_W ?              "SF on working(R)" :                         \
   (EVENT) == G8131_FSM_R_EVENT_SD_P ?              "SD on protection(R)" :                \
   (EVENT) == G8131_FSM_R_EVENT_SD_W ?              "SD on working(R)" :              \
   (EVENT) == G8131_FSM_R_EVENT_MS_P ?              "Manual switch to protection(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_MS_W ?              "Manual switch to working(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_WTR ?               "WTR timer expires(R)" :                        \
   (EVENT) == G8131_FSM_R_EVENT_EXER_W ?            "Exercise on working(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_EXER_P ?            "Exercise on protection(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_RR_W ?              "Reverse Request on Working(R)" :                       \
   (EVENT) == G8131_FSM_R_EVENT_RR_P ?              "Reverse Request on protection(R)" :                      \
   (EVENT) == G8131_FSM_R_EVENT_NR_NULL ?           "No Request, r/b = null(R)" :              \
   (EVENT) == G8131_FSM_R_EVENT_NR_NORMAL ?         "No Request, r/b = normal(R)" :            \
   (EVENT) == G8131_FSM_R_EVENT_DNR ?               "Do Not Revert(R)" :                      \
                                                    "Invalid")

/* Macro to convert G8131 FSM State value to string */
#define G8131_DEFECT_STR(DEFECT)                                               \
  ((DEFECT) == G8131_A_BIT_MISMATCH_ERROR ? "A Bit Mismatch Defect" :          \
   (DEFECT) == G8131_B_BIT_MISMATCH_ERROR ? "B Bit Mismatch Defect" :          \
   (DEFECT) == G8131_D_BIT_MISMATCH_ERROR ? "D Bit Mismatch Defect" :          \
   (DEFECT) == G8131_R_BIT_MISMATCH_ERROR ? "R Bit Mismatch Defect" :          \
   (DEFECT) == G8131_T_BIT_MISMATCH_ERROR ? "T Bit Mismatch Defect" :          \
   (DEFECT) == G8131_INVALID_REQ_STATE_IN_PDU ?                       \
                              "Invalid Request State" :              \
   (DEFECT) == G8131_APS_PDU_RCVD_ON_WORKING_INTERFACE ?                       \
                              "APS Received on Working Entity" :              \
                                                        "Not in defect mode")


/* Macro to convert G8131 FSM Request State value to string */
#define G8131_REQUEST_STATE_STR(STATE)                                         \
  ((STATE) == G8131_REQ_STATE_NR ?   "No Request" :                   \
   (STATE) == G8131_REQ_STATE_DNR ?   "Do Not Revert" :                   \
   (STATE) == G8131_REQ_STATE_RR ?     "Reverse Request" :        \
   (STATE) == G8131_REQ_STATE_EXER ?     "Exercise" :                \
   (STATE) == G8131_REQ_STATE_WTR ?   "Wait-to-Restore" :       \
   (STATE) == G8131_REQ_STATE_MS ?   "Manual Switch" :    \
   (STATE) == G8131_REQ_STATE_SD ?   "Signal Degrade" :    \
   (STATE) == G8131_REQ_STATE_SF_W ?   "Signal Fail for Working" : \
   (STATE) == G8131_REQ_STATE_SF_P?    "Signal Fail for Protection" :  \
   (STATE) == G8131_REQ_STATE_FS?    "Force Switch" :     \
   (STATE) == G8131_REQ_STATE_LO ?    "Lockout of Protection":\
                                      "Invalid")

#define G8131_FSM_GET_CUR_STATE(p)        (p)->current_state
#define G8131_FSM_GET_LAST_STATE(p)       (p)->last_state
#define G8131_FSM_SET_CUR_STATE(p,s)      (p)->current_state = (s)
#define G8131_FSM_SET_LAST_STATE(p,s)     (p)->last_state = (s)

int32
g8131_fsm_ignore(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_nochange(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_overrule(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_tbd(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_set_request_signal(tbl_aps_group_info_t* p_db_ag_info, uint32 new_state);

int32
g8131_fsm_change_state(tbl_aps_group_info_t* p_db_ag_info, uint32 new_state);

int32
g8131_fsm_update_regular_state(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_do_aps_switch(tbl_aps_group_info_t* p_db_ag_info, uint32 is_working_active);

int32
g8131_fsm_update_state_switch_to_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_update_state_switch_to_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_handle_sf_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_handle_sf_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_handle_sd_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_handle_sd_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32 
g8131_fsm_far_nr_w_handle_nr_null(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_nr_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_nr_p_handle_nr_null(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_nr_p_handle_nr_normal(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_nr_p_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_lo_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_fs_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_ms_p_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_sf_w_handle_wrsf(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_sf_p_handle_prsf(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_sd_w_handle_wrsd(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_local_sd_p_handle_prsd(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_ms_p_handle_ms_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_exer_w_handle_wtr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_exer_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_rr_w_handle_wtr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_far_rr_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event);

int32
g8131_fsm_execute (tbl_aps_group_info_t* p_db_ag_info, uint32 event);

#endif /* _OPM_g8131_PDU_H_ */