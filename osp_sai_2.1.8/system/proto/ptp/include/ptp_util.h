
#ifndef __PTP_UTIL_H__
#define __PTP_UTIL_H__

int32
ptp_util_calc_ms(int8 log);

ctc_task_t *
ptp_util_add_timer_ms(void (*func) (void *), void *arg, int32 ms);

ctc_task_t *
ptp_util_add_timer_log(void (*func) (void *), void *arg, int8 log);

ctc_task_t *
ptp_util_add_timer_log_window(void (*func) (void *), void *arg, int8 log, uint8 window);

void
ptp_util_cancel_timer(ctc_task_t **ppt);

int64
ptp_util_time_sub_time(glb_ptp_timestamp_t* p_minuend, glb_ptp_timestamp_t* p_sub);

void
ptp_util_get_timestamp_from_skb(ptp_sk_buff_t* skb, uint32 offset, glb_ptp_timestamp_t* p_rx_ts);

void
ptp_util_put_timestamp_into_skb(ptp_sk_buff_t* skb, uint32 offset, glb_ptp_timestamp_t* p_rx_ts);

int32
ptp_util_portid_cmp(glb_ptp_port_id_t *pva, glb_ptp_port_id_t *pvb);

int32
ptp_util_clockid_cmp(glb_ptp_clock_id_t clkida, glb_ptp_clock_id_t clkidb);

void
ptp_util_clockid_cpy(glb_ptp_clock_id_t clkida, glb_ptp_clock_id_t clkidb);

#endif /*!__PTP_UTIL_H__*/
