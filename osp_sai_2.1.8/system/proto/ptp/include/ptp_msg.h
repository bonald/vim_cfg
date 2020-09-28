
#ifndef __PTP_MSG_H__
#define __PTP_MSG_H__

void
ptp_msg_unpack_announce(ptp_sk_buff_t* skb, ptp_msg_announce_t* msg);

void
ptp_msg_unpack_sync(ptp_sk_buff_t* skb, ptp_msg_sync_t* msg);

void
ptp_msg_unpack_follow_up(ptp_sk_buff_t* skb, ptp_msg_follow_up_t* msg);

void
ptp_msg_unpack_delay_resp(ptp_sk_buff_t* skb, ptp_msg_delay_resp_t* msg);

void
ptp_msg_unpack_pdelay_resp(ptp_sk_buff_t* skb, ptp_msg_pdelay_resp_t* msg);

void
ptp_msg_unpack_pdelay_resp_follow_up(ptp_sk_buff_t* skb, ptp_msg_pdelay_resp_followup_t* msg);

#endif /*!__PTP_MSG_H__*/
