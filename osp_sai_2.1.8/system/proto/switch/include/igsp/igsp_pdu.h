
#ifndef __IGSP_PDU_H__
#define __IGSP_PDU_H__

#include "igmp_define.h"

int32
igsp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);
int32
igsp_pdu_rx_pim(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);
int32
igsp_pdu_tx_general_query(tbl_igsp_intf_t *p_db_igsp_if, uint32 max_response_time, uint32 is_tcn_query);

int32
igsp_pdu_tx_group_query(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address);

int32
igsp_pdu_tx_group_record_leave(tbl_igsp_group_t *p_db_igsp_grp);

int32
igsp_pdu_forward_report(tbl_interface_t *p_src_db_if, tbl_igsp_intf_t *p_db_igsp_if, const ctc_msg_t *p_msg);

int32
igsp_pdu_host_tx_report(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_db_group, uint32 is_join);

#endif /* __IGSP_PDU_H__ */

