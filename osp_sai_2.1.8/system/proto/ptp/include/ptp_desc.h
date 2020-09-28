
#ifndef __PTP_DESC_H__
#define __PTP_DESC_H__

#define PTP_DESC_BUFFER_(A, B, is_tx) \
ptp_desc_buffer(A, B, is_tx); 

void
ptp_desc_buffer(char *buffer, int32 buffer_size, uint32 is_tx);

char *
ptp_desc_bool(uint32 v);

char *
ptp_desc_enable(uint32 v);

char *
ptp_desc_ifname(tbl_ptp_port_t* ppo);

char *
ptp_desc_uint64(uint64 v);

char *
ptp_desc_float64(float64 v);

char *
ptp_desc_encap(ptp_net_protocol_t net_protocol);

char *
ptp_desc_dev_type(ptp_device_type_t type);

char *
ptp_desc_two_step(uint32 two_step);

char *
ptp_desc_delay_mechanism(ptp_delay_mechanism_t delay_mechanism);

uint32
ptp_desc_delay_mechanism_str2val(char *str);

char *
ptp_desc_state(ptp_state_t state);

char *
ptp_desc_ptp_timestamp(glb_ptp_timestamp_t *ptp_timestamp);

uint32
ptp_desc_dev_type_str2val(char *str);

uint32
ptp_desc_time_source_str2val(char *str);

uint32
ptp_desc_two_step_str2val(char *str);

uint32
ptp_desc_statistics_packet_type_str2val(char *str);

#endif /*!__PTP_DESC_H__*/
