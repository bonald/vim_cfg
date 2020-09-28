#ifndef _G8032_FSM_H_
#define _G8032_FSM_H_
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"

#define G8032_FSM_STATE_STR(STATE)   \
 ((STATE) == G8032_NODE_STATE_IDLE ? "Idle" :                \
 (STATE) == G8032_NODE_STATE_PROTECTION ? "Protection" :    \
 (STATE) == G8032_NODE_STATE_MANUAL_SWITCH? "Manual Switch" :    \
 (STATE) == G8032_NODE_STATE_FORCE_SWITCH? "Force Switch" :    \
 (STATE) == G8032_NODE_STATE_PENDING? "Pending" : "N/A")

#define G8032_FSM_ROLE_STR(ROLE)   \
 ((ROLE) == G8032_NODE_ROLE_RPL_OWNER ? "Owner" :                \
 (ROLE) == G8032_NODE_ROLE_RPL_NEIGHBOR ? "Neighbor" :    \
 (ROLE) == G8032_NODE_ROLE_RPL_NEXT_NEIGHBOR? "Next-Neighbor" : "N/A" )

#define  G8032_CURR_EVENT_STR(EVENT) \
 ((EVENT) == G8032_PRI_REQ_R_APS_NR ? "NR" :         \
 (EVENT) == G8032_PRI_REQ_R_APS_NR_RB ? "NR-RB":             \
 (EVENT) == G8032_PRIO_REQ_R_APS_WTR_RUNNING ? "WTR Running" :  \
 (EVENT) == G8032_PRI_REQ_WTR_EXPIRES ? "WTR Expires" :      \
 (EVENT) == G8032_PRI_REQ_R_APS_SF  ?  "Remote SF" :         \
 (EVENT) == G8032_PRI_REQ_L_CLEAR_SF ? "Local Clear SF":     \
 (EVENT) == G8032_PRI_REQ_L_SF ? "Local SF":                 \
 (EVENT) == G8032_PRI_REQ_L_CLEAR ? "Local Clear":           \
 (EVENT) == G8032_PRI_REQ_L_FS ? "Local FS":                 \
 (EVENT) == G8032_PRI_REQ_R_APS_FS ? "Remote FS":            \
 (EVENT) == G8032_PRI_REQ_L_MS ? "Local MS":                 \
 (EVENT) == G8032_PRI_REQ_R_APS_MS ? "Remote MS":            \
 (EVENT) == G8032_PRIO_REQ_R_APS_WTB_RUNNING ? "WTB Running":\
 (EVENT) == G8032_PRI_REQ_WTB_EXPIRES ? "WTB Expires" : "N/A") 

#define  G8032_CURR_PKT_TYPE(PKT_TYPE) \
 ((PKT_TYPE) == G8032_PRI_REQ_R_APS_NR ? "raps_nr" : \
 (PKT_TYPE) == G8032_PRI_REQ_R_APS_NR_RB ? "raps_nr_rb": \
 (PKT_TYPE) == G8032_PRI_REQ_R_APS_SF  ?  "raps_sf" :  \
 (PKT_TYPE) == G8032_PRI_REQ_R_APS_FS ? "raps_fs": \
 (PKT_TYPE) == G8032_PRI_REQ_R_APS_MS ? "raps_ms": \
 (PKT_TYPE) == G8032_PRI_REQ_R_APS_EVENT ? "raps_event" : "N/A")\


typedef struct
{
   uint8_t  mel_version;
   uint8_t  opcode;
   uint8_t  flags;   
   uint8_t  tlv_offset;
   uint8_t  state_sub;
   uint8_t  status;
   uint8_t  node_id[6];
}raps_pdu_all_t;

int32
g8032_fsm_clear_db_ms_info(tbl_g8032_ring_t *p_g8032_ring);

int32
g8032_fsm_clear_db_fs_info(tbl_g8032_ring_t *p_g8032_ring);

int32
g8032_fsm_ignore(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_raps_nr(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_raps_nr_rb(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_wtr_expires(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_raps_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_local_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_local_clear(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_local_fs(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_raps_ms(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_local_ms(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_wtb_expires(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_raps_fs(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_local_clear_sf(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);

int32
g8032_fsm_stop_transmit_raps(tbl_g8032_ring_t *p_g8032_ring);

int32
g8032_fsm_init (tbl_g8032_ring_t *p_g8032_ring);

int32
g8032_fsm_flush_port_fdb(tbl_g8032_ring_t *p_g8032_ring, uint8 flush_fdb);

int32
g8032_fsm_set_aps_protection_en(tbl_g8032_ring_t *p_g8032_ring);

#endif
