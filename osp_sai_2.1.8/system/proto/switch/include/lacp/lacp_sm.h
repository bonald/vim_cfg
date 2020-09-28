#ifndef __LACP_SM_H__
#define __LACP_SM_H__

int32
lacp_sm_receive_process_event(tbl_interface_t *p_db_if, lacp_rx_event_t event);

int32
lacp_sm_pertx_process_event(tbl_interface_t *p_db_if, lacp_pertx_event_t event);

int32
lacp_sm_mux_process_event(tbl_interface_t *p_db_if, lacp_mux_event_t event);

int32
lacp_sm_receive_cfg_update_state(tbl_interface_t *p_db_if);

int32
lacp_sm_pertx_cfg_update_state(tbl_interface_t *p_db_if);

int32
lacp_sm_mux_cfg_update_state(tbl_interface_t *p_db_if);

int32
lacp_sm_tx_lacpdu(tbl_interface_t *p_db_if);

#endif /* !__LACP_SM_H__ */
