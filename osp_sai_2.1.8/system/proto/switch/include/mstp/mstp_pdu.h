#ifndef __MSTP_PDU_H__
#define __MSTP_PDU_H__

char*
bpdu_version_val2str(uint8 version);

char*
bpdu_type_val2str(uint8 type);

int32
mstp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
mstp_pdu_tx_tcn(tbl_mstp_port_t *p_db_port);

int32
mstp_pdu_tx_bpdu(tbl_mstp_port_t *p_db_port, uint32 is_rstp);

#endif /* !__MSTP_PDU_H__ */
