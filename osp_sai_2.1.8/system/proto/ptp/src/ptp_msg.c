
#include "proto.h"
#include "ptp_error.h"
#include "ptp_constant.h"
#include "glb_ptp_define.h"
#include "ptp_datatype.h"
#include "ptp_msg.h"
#include "ptp_util.h"

void
ptp_msg_unpack_announce(ptp_sk_buff_t* skb, ptp_msg_announce_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->origin_timestamp);
    msg->current_utc_offset = sal_ntoh16(*(uint16*) (skb->data + 44));
    msg->grandmaster_priority1 = *(uint8*) (skb->data + 47);
    msg->grandmaster_clock_quality.clk_class = *(uint8*) (skb->data + 48);
    msg->grandmaster_clock_quality.clk_accuracy = *(uint8*) (skb->data + 49);
    msg->grandmaster_clock_quality.offset_scaled_log_variance = sal_ntoh16(*(uint16*) (skb->data + 50));
    msg->grandmaster_priority2 = *(uint8*) (skb->data + 52);
    sal_memcpy(msg->grandmaster_identity, (skb->data + 53), GLB_PTP_CLOCK_ID_LEN);
    msg->steps_removed = sal_ntoh16(*(uint16*) (skb->data + 61));
    msg->time_source = *(uint8*) (skb->data + 63);
    return;
}

void
ptp_msg_unpack_sync(ptp_sk_buff_t* skb, ptp_msg_sync_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->origin_timestamp);
    return;
}

void
ptp_msg_unpack_follow_up(ptp_sk_buff_t* skb, ptp_msg_follow_up_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->precise_origin_timestamp);
    return;
}

void
ptp_msg_unpack_delay_resp(ptp_sk_buff_t* skb, ptp_msg_delay_resp_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->receive_timestamp);
    sal_memcpy(msg->requesting_port_identity.clock_id, (skb->data + 44), GLB_PTP_CLOCK_ID_LEN);
    msg->requesting_port_identity.port_num = sal_ntoh16(*(uint16*) (skb->data + 52));
    return;
}

void
ptp_msg_unpack_pdelay_resp(ptp_sk_buff_t* skb, ptp_msg_pdelay_resp_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->request_receipt_timestamp);
    sal_memcpy(msg->requesting_port_identity.clock_id, (skb->data + 44), GLB_PTP_CLOCK_ID_LEN);
    msg->requesting_port_identity.port_num = sal_ntoh16(*(uint16*) (skb->data + 52));
    return;
}

void
ptp_msg_unpack_pdelay_resp_follow_up(ptp_sk_buff_t* skb, ptp_msg_pdelay_resp_followup_t* msg)
{
    ptp_util_get_timestamp_from_skb(skb, PTP_MSG_TS_OFFSET, &msg->response_origin_timestamp);
    sal_memcpy(msg->requesting_port_identity.clock_id, (skb->data + 44), GLB_PTP_CLOCK_ID_LEN);
    msg->requesting_port_identity.port_num = sal_ntoh16(*(uint16*) (skb->data + 52));
    return;
}

