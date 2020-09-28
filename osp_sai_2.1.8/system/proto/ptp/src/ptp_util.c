
 #include "proto.h"

 #include "gen/tbl_ptp_port_define.h"
 
#include "ptp_error.h"
#include "ptp_util.h"
#include "ptp_desc.h"

int32
ptp_util_calc_ms(int8 log)
{
#define PTP_MAX_LOG_VALUE 10
#define PTP_MIN_LOG_VALUE -5
    int32 msec = 1000;
    
    log = (log < PTP_MAX_LOG_VALUE) ? log : PTP_MAX_LOG_VALUE;
    log = (log > PTP_MIN_LOG_VALUE) ? log : PTP_MIN_LOG_VALUE;
    msec = (log >= 0) ? (msec << log) : (msec >> (-log));
    
    return msec;
}

ctc_task_t *
ptp_util_add_timer_ms(void (*func) (void *), void *arg, int32 ms)
{
    return ctc_task_add_timer_msec(TASK_PRI_NORMAL, func, arg, ms);
}

ctc_task_t *
ptp_util_add_timer_log(void (*func) (void *), void *arg, int8 log)
{
    return ctc_task_add_timer_msec(TASK_PRI_NORMAL, func, arg, ptp_util_calc_ms(log));
}

ctc_task_t *
ptp_util_add_timer_log_window(void (*func) (void *), void *arg, int8 log, uint8 window)
{
    return ctc_task_add_timer_msec(TASK_PRI_NORMAL, func, arg, (window * ptp_util_calc_ms(log)));
}

void
ptp_util_cancel_timer(ctc_task_t **ppt)
{
    if (ppt && *ppt)
    {
        ctc_task_delete_timer(*ppt);
        *ppt = NULL;
    }
}

int64
ptp_util_time_sub_time(glb_ptp_timestamp_t* p_minuend, glb_ptp_timestamp_t* p_sub)
{
    uint32 ns;
    glb_ptp_timeinterval_t minuend, sub;

    /*calculate second field*/
    minuend.s.msb = p_minuend->sec.msb;
    minuend.s.lsb = p_minuend->sec.lsb;
    sub.s.msb = p_sub->sec.msb;
    sub.s.lsb = p_sub->sec.lsb;
    int64 seconds = minuend.v - sub.v;
    PTP_DBG_SYNC("Time1 subtract time2 \n"
        "[second mbit %d lbit %u, nanosecond %u] - [second mbit %d lbit %u, nanosecond %u]\n", 
        minuend.s.msb, minuend.s.lsb, p_minuend->ns,sub.s.msb, sub.s.lsb, p_sub->ns);
    PTP_DBG_SYNC("[second %"PRId64", nanosecond %u] - [second %"PRId64", nanosecond %u]\n", 
        (minuend.v), p_minuend->ns, (sub.v), p_sub->ns);
    /*calculate nanosecond field*/
    ns = p_minuend->ns;
    if (ns < p_sub->ns)
    {
        ns += GLB_PTP_NS_PER_SEC;
        seconds -= 1;
    }

    ns -= p_sub->ns;
    PTP_DBG_SYNC("Result: [second %"PRId64", nanosecond %u]\n", (seconds), ns);
    
    return ((seconds * GLB_PTP_NS_PER_SEC) + ns);
}

void
ptp_util_get_timestamp_from_skb(ptp_sk_buff_t* skb, uint32 offset, glb_ptp_timestamp_t* p_rx_ts)
{
    uint32* p32;
    uint16* p16;

    p16 = (uint16 *) (skb->data + offset);
    p_rx_ts->sec.msb = sal_ntoh16(*p16);
    p32 = (uint32 *) (p16 + 1);
    p_rx_ts->sec.lsb = sal_ntoh32(*p32);
    p32 = p32 + 1;
    p_rx_ts->ns = sal_ntoh32(*p32);

    ptp_common_message_header_t *hdr;
    hdr = (ptp_common_message_header_t *) skb->data;

    PTP_DBG_PACKET_RX("Sequence id %hu, %s\n", sal_ntoh16(hdr->sequence_id), ptp_desc_ptp_timestamp(p_rx_ts));
    
    #ifdef _GLB_UML_SYSTEM_
    sal_memset(p_rx_ts, 0x00, sizeof(glb_ptp_timestamp_t));
    #endif

    #ifdef PTP_NO_USE_TS
    sal_memset(p_rx_ts, 0x00, sizeof(glb_ptp_timestamp_t));
    #endif

    return;
}

void
ptp_util_put_timestamp_into_skb(ptp_sk_buff_t* skb, uint32 offset, glb_ptp_timestamp_t* p_rx_ts)
{
    uint32* p32;
    uint16* p16;

    p16 = (uint16 *) (skb->data + offset);
    *p16 = sal_hton16(p_rx_ts->sec.msb);
    p32 = (uint32 *) (p16 + 1);
    *p32 = sal_hton32(p_rx_ts->sec.lsb);
    p32 = p32 + 1;
    *p32 = sal_hton32(p_rx_ts->ns);

    return;
}

int32
ptp_util_portid_cmp(glb_ptp_port_id_t *pva, glb_ptp_port_id_t *pvb)
{
    return (sal_memcmp(pva, pvb, GLB_PTP_PORT_ID_LEN));
}

int32
ptp_util_clockid_cmp(glb_ptp_clock_id_t clkida, glb_ptp_clock_id_t clkidb)
{
    return (sal_memcmp(clkida, clkidb, GLB_PTP_CLOCK_ID_LEN));
}

void
ptp_util_clockid_cpy(glb_ptp_clock_id_t clkida, glb_ptp_clock_id_t clkidb)
{
    sal_memcpy(clkida, clkidb, GLB_PTP_CLOCK_ID_LEN);
}

