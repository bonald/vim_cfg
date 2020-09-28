
#ifndef __PTP_SOCK_H__
#define __PTP_SOCK_H__

uint8 *
ptp_skb_push(ptp_sk_buff_t* skb, uint32 len);

uint8 *
ptp_skb_pull(ptp_sk_buff_t* skb, uint32 len);

uint8 *
ptp_skb_put(ptp_sk_buff_t* skb, uint32 len);

uint8 *
ptp_skb_get(ptp_sk_buff_t* skb, uint32 len);

ptp_sk_buff_t *
ptp_alloc_skb(uint32 size);

void
ptp_init_skb(ptp_sk_buff_t* skb, uint32 size);

ptp_sk_buff_t *
ptp_skb_copy(ptp_sk_buff_t* skb);

void
ptp_free_skb(ptp_sk_buff_t* skb);

int32
ptp_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
ptp_sock_send(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* p_rx_ts);

int32
ptp_sock_send_w_ts_resp(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb, glb_ptp_timestamp_t* tx_ts, glb_ptp_timestamp_t* p_rx_ts);

#endif /*!__PTP_SOCK_H__*/
