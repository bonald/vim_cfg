#ifndef __SFLOW_API_HEADER_H__
#define __SFLOW_API_HEADER_H__

int32
sflow_api_get_sflow_enable(void);

int32
sflow_api_counter_sampling_enable(tbl_interface_t *p_db_if, uint32 enable);

int32
sflow_pkt_func(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
sflow_api_set_sflow_counter_interval(uint32 counter_interval);

#endif


