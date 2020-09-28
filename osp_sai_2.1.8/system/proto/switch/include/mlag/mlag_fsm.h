
#ifndef __MLAG_FSM_H__
#define __MLAG_FSM_H__

/* Macro to convert MLAG Peer FSM State value to string */
#define MLAG_PEER_FSM_STATE_STR(STATE)                              \
    ((STATE) == MPF_STATE_IDLE ? "Idle" :                           \
     (STATE) == MPF_STATE_ACTIVE ? "Active" :                       \
     (STATE) == MPF_STATE_OPEN_SENT ? "OpenSent" :                  \
     (STATE) == MPF_STATE_OPEN_CFM ? "OpenConfirm" :                \
     (STATE) == MPF_STATE_ESTABLISHED ? "Established" : "Invalid")

#define MLAG_PEER_FSM_EVENT_STR(EVENT)                              \
    ((EVENT) == MPF_EVENT_MANUAL_START ? "Manual-start" :           \
     (EVENT) == MPF_EVENT_MANUAL_STOP ? "Manual-stop" :             \
     (EVENT) == MPF_EVENT_HOLD_EXP ? "Holdtime-expired" :           \
     (EVENT) == MPF_EVENT_KEEPALIVE_EXP ? "Keepalive-expired" :     \
     (EVENT) == MPF_EVENT_TCP_CONN_CFM ? "Tcp-connect-confirmed" :  \
     (EVENT) == MPF_EVENT_TCP_CONN_FAIL ? "Tcp-connect-failed" :    \
     (EVENT) == MPF_EVENT_OPEN_VALID ? "Open-valid" :               \
     (EVENT) == MPF_EVENT_KEEPALIVE_VALID ? "Keepalive-valid" : "Invalid")

int32
mpf_change_state(tbl_mlag_peer_t *peer, mlag_state_t mpf_state);

#endif /* !__MLAG_FSM_H__ */

