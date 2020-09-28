
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"

#include "ptp_constant.h"
#include "ptp_error.h"
#include "ptp_util.h"
#include "ptp_desc.h"
#include "ptp_ops.h"
#include "ptp_sock.h"
#include "ptp_proto.h"
#include "ptp_msg.h"
#include "ptp_msg_issue.h"
#include "ptp_api.h"

static int32
_ptp_calculate_delay_req_interval(void)
{
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    
    /*the min log-sync is -1, so granularity must less than 2^-5 = 31.25ms, so we can use 25ms*/
    const int32 ptp_delay_req_granularity = 25;
    int32 distribution_min = ptp_util_calc_ms(g_p_master->master_min_delay_req_interval);
    int32 distribution_max = ptp_util_calc_ms(g_p_master->master_min_delay_req_interval + 1);
    int32 number = (distribution_max - distribution_min) / ptp_delay_req_granularity;
    int32 interval = distribution_min + (sal_rand() % number) * ptp_delay_req_granularity;
    
    return interval;
}

static void
_ptp_msg_issue_announce(tbl_ptp_port_t *p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_TX("TX Announce in %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_sk_buff_t *tx_skb;
    ptp_common_message_header_t *tx_hdr;

    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Announce packet");
        goto PTP_FINALLY;
    }

    tx_skb->len = PTP_MSG_ANNOUNCE_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    tx_hdr->message_type = PTP_MSG_TYPE_ANNOUNCE;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_ANNOUNCE_LENGTH);
    tx_hdr->domain_number = g_p_master->default_ds.domain_number;

    /*device self priority*/
    tx_hdr->flag_field_1_leap61 = g_p_master->time_prop_ds.leap61;
    tx_hdr->flag_field_1_leap59 = g_p_master->time_prop_ds.leap59;
    tx_hdr->flag_field_1_current_utc_offset_valid = g_p_master->time_prop_ds.current_utc_offset_valid;
    tx_hdr->flag_field_1_ptp_timescale = g_p_master->time_prop_ds.ptp_timescale;
    tx_hdr->flag_field_1_time_traceable = g_p_master->time_prop_ds.time_traceable;
    tx_hdr->flag_field_1_frequency_traceable = g_p_master->time_prop_ds.frequency_traceable;

    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);
    tx_hdr->sequence_id = sal_hton16(p_port->p.sent_announce_sequence_id);
    p_port->p.sent_announce_sequence_id++;

    tx_hdr->control_field = PTP_MSG_CTRL_ALL_OTHERS;
    tx_hdr->log_message_interval = p_port->port_ds.log_announce_interval;

    *(int16*) (tx_skb->data + 44) = sal_hton16(g_p_master->time_prop_ds.current_utc_offset);

    /*device's parent priority*/
    ptp_parent_t *pst_parent = &g_p_master->parent_ds;
    *(uint8*) (tx_skb->data + 47) = pst_parent->grandmaster_priority1;
    *(uint8*) (tx_skb->data + 48) = pst_parent->grandmaster_clock_quality.clk_class;
    *(uint8*) (tx_skb->data + 49) = pst_parent->grandmaster_clock_quality.clk_accuracy;
    *(uint16*) (tx_skb->data + 50) = sal_hton16(pst_parent->grandmaster_clock_quality.offset_scaled_log_variance);
    *(uint8*) (tx_skb->data + 52) = pst_parent->grandmaster_priority2;
    sal_memcpy((tx_skb->data + 53), pst_parent->grandmaster_identity, GLB_PTP_CLOCK_ID_LEN);

    *(uint16*) (tx_skb->data + 61) = sal_hton16(g_p_master->current_ds.steps_removed);
    *(uint8*) (tx_skb->data + 63) = g_p_master->time_prop_ds.time_source;

    int32 ret = ptp_sock_send(p_port, tx_skb, NULL);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to issue Announce packet, ret %d", ret);
        goto PTP_FINALLY;
    }

PTP_FINALLY:

    ptp_free_skb(tx_skb);

    return;
}


static void
_ptp_msg_issue_sync(tbl_ptp_port_t *p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_TX("TX Sync in %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_sk_buff_t *tx_skb = NULL;
    ptp_sk_buff_t *tx_skb_fu = NULL;
    ptp_common_message_header_t *tx_hdr, *tx_hdr_fu;
    int32 ret;
    glb_ptp_timestamp_t tx_ts;

    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Sync packet");
        goto PTP_FINALLY;
    }

    tx_skb->len = PTP_MSG_SYNC_FOLLOW_UP_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    tx_hdr->message_type = PTP_MSG_TYPE_SYNC;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_SYNC_FOLLOW_UP_LENGTH);
    tx_hdr->domain_number = g_p_master->default_ds.domain_number;

    tx_hdr->flag_field_0_two_step_flag = g_p_master->default_ds.two_step_flag;
  //  if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
  //  {
  //      tx_hdr->flag_field_0_two_step_flag = 1;
  //  }

    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);
    tx_hdr->sequence_id = sal_hton16(p_port->p.sent_sync_sequence_id);
    p_port->p.sent_sync_sequence_id++;

    tx_hdr->control_field = PTP_MSG_CTRL_SYNC;

    /*fix bug 46580, sync/follow_up/delay_resp unicast packet is 0x7f*/
    addr_ipv4_t tmp_ip;
    sal_memset(&tmp_ip, 0, sizeof(addr_ipv4_t));
    
    if (0 == sal_memcmp(&tmp_ip, &(p_port->port_addr.dest_ip_addr), sizeof(tmp_ip)))
    {
        tx_hdr->log_message_interval = p_port->port_ds.log_sync_interval;
    }
    else
    {
        tx_hdr->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;
    }

    if (tx_hdr->flag_field_0_two_step_flag)
    {
        tx_skb_fu = ptp_skb_copy(tx_skb);
        if (!tx_skb_fu)
        {
            PTP_LOG_ERR("Cannot allocate memory for issue Follow_Up packet");
            goto PTP_FINALLY;
        }
    }

    #ifdef DUET2
    uint64 boot_time_msec = 0;
    int64 total_offset = 0;
    
    //add by weij for bug 45114, use uninterrupt mode
    boot_time_msec = ptp_api_get_global_enable_time();
    tx_ts.sec.lsb = (uint32)(boot_time_msec/1000);
    tx_ts.ns = (uint32)((boot_time_msec%1000)*1000000);
    #endif

    /*ptp msg header*/
    PTP_DESC_BUFFER_((char *)(tx_hdr), sizeof(ptp_common_message_header_t), TRUE);
    
    PTP_DESC_BUFFER_((char *)(tx_skb->data), tx_skb->len, TRUE);

    //add by weij for bug 45114, use uninterrupt mode
#ifdef DUET2
    ret = ptp_sock_send(p_port, tx_skb, &tx_ts);
#else
    if(! tx_hdr->flag_field_0_two_step_flag)
    {
         ret = ptp_sock_send(p_port, tx_skb, NULL);
    }
    else
    {
        ret = ptp_sock_send_w_ts_resp(p_port, tx_skb, &tx_ts, NULL);
    }
#endif
    
    if (PTP_E_IF_BLOCKED == ret)
    {
        PTP_LOG_ERR("Failed to issue Sync packet to %s for blocked", ptp_desc_ifname(p_port));
        goto PTP_FINALLY;
    }
    else if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to issue Sync packet to %s, ret %d", ptp_desc_ifname(p_port), ret);
        goto PTP_FINALLY;
    }

    /*one-step sync should not build followup*/
    if (!tx_hdr->flag_field_0_two_step_flag)
    {
        goto PTP_FINALLY;
    }
    //add by weij for bug 45114, use uninterrupt mode
    #ifdef DUET2
    total_offset = g_p_master->total_offset + boot_time_msec*1000000;
    tx_ts.sec.lsb = total_offset/PTP_MAX_NS;
    tx_ts.ns = total_offset%PTP_MAX_NS;
    #else
    if((tx_ts.ns + p_port->egress_latency) > PTP_MAX_NS)
    {
        tx_ts.ns = tx_ts.ns + p_port->egress_latency - PTP_MAX_NS;
        tx_ts.sec.lsb = tx_ts.sec.lsb + 1;
    }
    else
    {
        tx_ts.ns = tx_ts.ns + p_port->egress_latency;
    }

    PTP_DBG_PACKET_TX("Set packet sequence id %d %s for %s",
                tx_hdr->sequence_id, ptp_desc_ptp_timestamp(&tx_ts), IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    #endif
    
    tx_hdr_fu = (ptp_common_message_header_t *) tx_skb_fu->data;
    tx_hdr_fu->message_type = PTP_MSG_TYPE_FOLLOW_UP;
    tx_hdr_fu->flag_field_0_two_step_flag = 0;
    tx_hdr_fu->control_field = PTP_MSG_CTRL_FOLLOW_UP;

    PTP_DBG_PACKET_TX("TX Follow_Up in %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    PTP_DESC_BUFFER_((char *)tx_skb_fu->data, tx_skb_fu->len, TRUE);
    ptp_util_put_timestamp_into_skb(tx_skb_fu, PTP_MSG_TS_OFFSET, &tx_ts);
    PTP_DESC_BUFFER_((char *)tx_skb_fu->data, tx_skb_fu->len, TRUE);
    
    ret = ptp_sock_send(p_port, tx_skb_fu, NULL);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to issue Follow_Up packet, ret %d", ret);
        goto PTP_FINALLY;
    }

PTP_FINALLY:

    ptp_free_skb(tx_skb);
    ptp_free_skb(tx_skb_fu);
    
}

static void
_ptp_msg_issue_delay_req(tbl_ptp_port_t *p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_TX("TX Delay_Req in %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_sk_buff_t *tx_skb;
    ptp_common_message_header_t *tx_hdr;
    int32 ret = 0;
#ifdef DUET2
    glb_ptp_timestamp_t tx_ts;
    uint64 boot_time_msec = 0;
    int64 total_offset;
#endif
    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Delay_Req packet");
        goto PTP_FINALLY;
    }

    tx_skb->len = PTP_MSG_DELAY_REQ_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    tx_hdr->message_type = PTP_MSG_TYPE_DELAY_REQ;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_DELAY_REQ_LENGTH);
    tx_hdr->domain_number = g_p_master->default_ds.domain_number;
    tx_hdr->flag_field_0_two_step_flag = g_p_master->default_ds.two_step_flag;

    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);
    tx_hdr->sequence_id = sal_hton16(p_port->p.sent_delay_req_sequence_id);
    p_port->p.sent_delay_req_sequence_id++;

    glb_ptp_timeinterval_t correction_field;
    correction_field.v = -(p_port->asymmetry_correction.v);
    
    PTP_DBG_PACKET_TX("Issue Delay_Req correction_field: %d %u %lld\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);
    
    tx_hdr->correction_field_msb = sal_hton32(correction_field.s.msb);
    tx_hdr->correction_field_lsb = sal_hton32(correction_field.s.lsb);

    tx_hdr->control_field = PTP_MSG_CTRL_DELAY_REQ;
    tx_hdr->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;

    #ifdef DUET2
    //add by weij for bug 45114, use uninterrupt mode
    boot_time_msec = ptp_api_get_global_enable_time();
    tx_ts.sec.lsb = (uint32)(boot_time_msec/1000);
    tx_ts.ns = (uint32)((boot_time_msec%1000)*1000000);
    //GLB_CONSOLE_PRINT("--%s:%d--%u:%u------\n", __FUNCTION__, __LINE__, tx_ts.sec.lsb, tx_ts.ns);
    #endif

    //add by weij for bug 45114, use uninterrupt mode
    #ifdef DUET2
    ret = ptp_sock_send(p_port, tx_skb, &tx_ts);

    total_offset = g_p_master->total_offset + boot_time_msec*1000000;
    p_port->p.delay_req_send_time.sec.lsb = total_offset/PTP_MAX_NS;
    p_port->p.delay_req_send_time.ns = total_offset%PTP_MAX_NS;
    #else
    ret = ptp_sock_send_w_ts_resp(p_port, tx_skb, &p_port->p.delay_req_send_time, NULL);
    #endif
    if((p_port->p.delay_req_send_time.ns + p_port->egress_latency) > PTP_MAX_NS)
    {
        p_port->p.delay_req_send_time.ns = p_port->p.delay_req_send_time.ns + p_port->egress_latency - PTP_MAX_NS;
        p_port->p.delay_req_send_time.sec.lsb = p_port->p.delay_req_send_time.sec.lsb + 1;
    }
    else
    {
        p_port->p.delay_req_send_time.ns = p_port->p.delay_req_send_time.ns + p_port->egress_latency;
    }

    //GLB_CONSOLE_PRINT("--%s:%d----%u--%"PRIu64":%"PRIu64"--\n", __FUNCTION__, __LINE__,  p_port->p.delay_req_send_time.sec.lsb,  g_p_master->total_offset, boot_time_msec*1000000);

#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "delay_reqT3, offsets3 = %d, offsetn3 = %d , intr_seq_id = %d \n",
    p_port->p.delay_req_send_time.sec.lsb, p_port->p.delay_req_send_time.ns, p_port->p.delay_req_send_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    if (PTP_E_IF_BLOCKED == ret)
    {
        PTP_LOG_ERR("Failed to issue Delay_Req packet to %s for blocked", ptp_desc_ifname(p_port));
        goto PTP_FINALLY;
    }
    else if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to issue Delay_Req packet to %s, ret %d", ptp_desc_ifname(p_port), ret);
        goto PTP_FINALLY;
    }

    p_port->p.delay_m_s = p_port->p.last_delay_m_s;
    p_port->p.offset_adjusted = 0;

PTP_FINALLY:

    ptp_free_skb(tx_skb);

    return;
}

static void
_ptp_msg_issue_pdelay_req(tbl_ptp_port_t *p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_TX("TX Pdelay_Req in %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext));
    
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_sk_buff_t *tx_skb;
    ptp_common_message_header_t *tx_hdr;
    int32 ret = 0;
#ifdef DUET2
    glb_ptp_timestamp_t tx_ts;
    uint64 boot_time_msec = 0;
    int64 total_offset;
#endif
    
    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Pdelay_Req packet");
        goto PTP_FINALLY;
    }

    tx_skb->len = PTP_MSG_PDELAY_REQ_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    tx_hdr->message_type = PTP_MSG_TYPE_PDELAY_REQ;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_PDELAY_REQ_LENGTH);
    if (ptp_ops_is_tc())
    {
        tx_hdr->domain_number = g_p_master->default_ds.primary_domain;
    }
    else
    {
        tx_hdr->domain_number = g_p_master->default_ds.domain_number;
    }

    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);
    tx_hdr->sequence_id = sal_hton16(p_port->p.sent_pdelay_req_sequence_id);
    p_port->p.sent_pdelay_req_sequence_id++;

    glb_ptp_timeinterval_t correction_field;
    correction_field.v = -(p_port->asymmetry_correction.v);
    
    PTP_DBG_PACKET_TX("Issue Pdelay_Req correction_field: %d %u %lld\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);
    
    tx_hdr->correction_field_msb = sal_hton32(correction_field.s.msb);
    tx_hdr->correction_field_lsb = sal_hton32(correction_field.s.lsb);
    tx_hdr->flag_field_0_two_step_flag = g_p_master->default_ds.two_step_flag;

    tx_hdr->control_field = PTP_MSG_CTRL_ALL_OTHERS;
    tx_hdr->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;

    #ifdef DUET2
    //add by weij for bug 45114, use uninterrupt mode
    boot_time_msec = ptp_api_get_global_enable_time();
    tx_ts.sec.lsb = (uint32)(boot_time_msec/1000);
    tx_ts.ns = (uint32)((boot_time_msec%1000)*1000000);
    #endif

    //add by weij for bug 45114, use uninterrupt mode
#ifdef DUET2
    ret = ptp_sock_send(p_port, tx_skb, &tx_ts);

    total_offset = g_p_master->total_offset + boot_time_msec*1000000;
    p_port->p.pdelay_req_send_time.sec.lsb = total_offset/PTP_MAX_NS;
    p_port->p.pdelay_req_send_time.ns = total_offset%PTP_MAX_NS;
#else
    ret = ptp_sock_send_w_ts_resp(p_port, tx_skb, &p_port->p.pdelay_req_send_time, NULL);
#endif

    if((p_port->p.pdelay_req_send_time.ns + p_port->egress_latency) > PTP_MAX_NS)
    {
        p_port->p.pdelay_req_send_time.ns = p_port->p.pdelay_req_send_time.ns + p_port->egress_latency - PTP_MAX_NS;
        p_port->p.pdelay_req_send_time.sec.lsb = p_port->p.pdelay_req_send_time.sec.lsb + 1;
    }
    else
    {
        p_port->p.pdelay_req_send_time.ns = p_port->p.pdelay_req_send_time.ns + p_port->egress_latency;
    }

#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "pdelay_reqT1, sencond = %d, nansecond3 = %d , intr_seq_id = %d \n",
    p_port->p.pdelay_req_send_time.sec.lsb, p_port->p.pdelay_req_send_time.ns, p_port->p.pdelay_req_send_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to issue Pdelay_Req packet to %s, ret %d", ptp_desc_ifname(p_port), ret);
        goto PTP_FINALLY;
    }

    p_port->p.offset_adjusted = 0;

PTP_FINALLY:

    ptp_free_skb(tx_skb);

    return;
}

static void
_ptp_announce_trigger(void* p_data)
{
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*)p_data : NULL;
    if(NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    p_port->ptm_announce_timer = NULL;
    if (p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_ANNOUNCE])
    {
        /*_ptp_msg_issue_announce(p_port);*/
        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_ANNOUNCE](p_port);
    }

    ptp_util_cancel_timer(&p_port->ptm_announce_timer);
    p_port->ptm_announce_timer = ptp_util_add_timer_log(_ptp_announce_trigger, p_port, p_port->port_ds.log_announce_interval);
    
    return ;
}

static void
_ptp_sync_trigger(void* p_data)
{
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*)p_data : NULL;
    if(NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    p_port->ptm_sync_timer = NULL;
    if (p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_SYNC])
    {
        /*_ptp_msg_issue_sync(p_port);*/
        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_SYNC](p_port);
    }

    ptp_util_cancel_timer(&p_port->ptm_sync_timer);
    p_port->ptm_sync_timer = ptp_util_add_timer_log(_ptp_sync_trigger, p_port, p_port->port_ds.log_sync_interval);
    
    return ;
}

static void
_ptp_delay_req_trigger(void* p_data)
{
    int32 interval = 0;
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*) p_data : NULL;
    if(NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    p_port->ptm_delay_timer = NULL;
    if (p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_DELAY_REQ])
    {
        /*_ptp_msg_issue_delay_req(p_port);*/
        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_DELAY_REQ](p_port);
    }

    interval = _ptp_calculate_delay_req_interval();
    ptp_util_cancel_timer(&p_port->ptm_delay_timer);
    p_port->ptm_delay_timer = ptp_util_add_timer_ms(_ptp_delay_req_trigger, p_port, interval);
    
    return ;
}

static void
_ptp_pdelay_req_trigger(void* p_data)
{
    tbl_ptp_port_t *p_port = p_data ? (tbl_ptp_port_t*)p_data : NULL;
    if(NULL == p_port)
    {
        PTP_LOG_ERR("Input parameter ptp port is NULL");
        return;
    }

    p_port->ptm_pdelay_timer = NULL;
    if (p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ])
    {
        /*_ptp_msg_issue_pdelay_req(p_port);*/
        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ](p_port);
    }

    ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
    p_port->ptm_pdelay_timer = ptp_util_add_timer_log(_ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
    
    return ;
}

void
ptp_trigger_register(tbl_ptp_port_t *p_port)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_TMR("%s is %s tc, recommended_state is %s, delay_mechanism is %s", IFNAME_ETH2FULL(p_port->key.name, ifname_ext), 
        ptp_ops_is_tc() ? "" : "not", ptp_desc_state(p_port->recommended_state), ptp_desc_delay_mechanism(p_port->port_ds.delay_mechanism));
    PTP_DBG_BMC("%s is %s tc, recommended_state is %s, delay_mechanism is %s", ifname_ext, 
        ptp_ops_is_tc() ? "" : "not", ptp_desc_state(p_port->recommended_state), ptp_desc_delay_mechanism(p_port->port_ds.delay_mechanism));
        
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    if(NULL == g_p_master)
    {
        PTP_DBG_BMC("Ptp global does not exist");
        return;
    }
    
    if (ptp_ops_is_tc())
    {
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            if (p_port->enable && p_port->link_up && g_p_master->global_enable)
            {
                ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
                p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                        _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
            }
            else
            {
                PTP_DBG_TMR("Cancel ptm_pdelay_timer for p2ptc");
                ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            }
        }

        return;
    }

    switch (p_port->recommended_state)
    {
    case PTP_STATE_LISTENING:
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_announce_receipt_time_restart(p_port);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }

        break;
    case PTP_STATE_PRE_MASTER:
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_util_cancel_timer(&p_port->ptm_announce_receipt_timer);
        ptp_qualification_start(p_port);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }

        break;
    case PTP_STATE_MASTER:
        ptp_util_cancel_timer(&p_port->ptm_announce_receipt_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        p_port->ptm_sync_timer = ptp_util_add_timer_log(_ptp_sync_trigger, p_port, p_port->port_ds.log_sync_interval);
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        p_port->ptm_announce_timer = ptp_util_add_timer_log(_ptp_announce_trigger, p_port, p_port->port_ds.log_announce_interval);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }

        break;
    case PTP_STATE_PASSIVE:
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_announce_receipt_time_restart(p_port);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }

        break;
    case PTP_STATE_UNCALIBRATED:
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_announce_receipt_time_restart(p_port);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }

        break;
    case PTP_STATE_SLAVE:
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_announce_receipt_time_restart(p_port);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
            p_port->ptm_pdelay_timer = ptp_util_add_timer_log(
                    _ptp_pdelay_req_trigger, p_port, p_port->port_ds.log_min_pdelay_req_interval);
        }
        else if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            int32 interval = _ptp_calculate_delay_req_interval();
            ptp_util_cancel_timer(&p_port->ptm_delay_timer);
            p_port->ptm_delay_timer = ptp_util_add_timer_ms(_ptp_delay_req_trigger, p_port, interval);
        }

        break;
    case PTP_STATE_INITIALIZING:
    case PTP_STATE_FAULTY:
    case PTP_STATE_DISABLED:
    default:
        PTP_DBG_BMC("Cancel all timer in %s %s", ifname_ext, 
            ptp_desc_state(p_port->recommended_state));
        ptp_util_cancel_timer(&p_port->ptm_announce_timer);
        ptp_util_cancel_timer(&p_port->ptm_sync_timer);
        ptp_util_cancel_timer(&p_port->ptm_delay_timer);
        ptp_util_cancel_timer(&p_port->ptm_pdelay_timer);
        ptp_util_cancel_timer(&p_port->ptm_announce_receipt_timer);
        ptp_util_cancel_timer(&p_port->ptm_qualification_timer);
        break;
    }
}

void
ptp_issue_register(tbl_ptp_port_t *p_port)
{
    sal_memset(p_port->msg_proc_ops.issue_proc, 0x00, sizeof(ptp_msg_issue_proc_t) * PTP_MSG_TYPE_MAX);
    if (ptp_ops_is_tc())
    {
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_issue_proc_t)_ptp_msg_issue_pdelay_req;
        }

        return;
    }

    switch (p_port->recommended_state)
    {
    case PTP_STATE_INITIALIZING:
    case PTP_STATE_FAULTY:
    case PTP_STATE_DISABLED:
        break;

    case PTP_STATE_LISTENING:
    case PTP_STATE_PRE_MASTER:
    case PTP_STATE_PASSIVE:
    case PTP_STATE_UNCALIBRATED:
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_issue_proc_t)_ptp_msg_issue_pdelay_req;
        }

        break;

    case PTP_STATE_MASTER:
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_issue_proc_t)_ptp_msg_issue_pdelay_req;
        }

        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_ANNOUNCE] = (ptp_msg_issue_proc_t)_ptp_msg_issue_announce;
        p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_SYNC] = (ptp_msg_issue_proc_t)_ptp_msg_issue_sync;
        break;

    case PTP_STATE_SLAVE:
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_issue_proc_t)_ptp_msg_issue_pdelay_req;
        }
        else if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.issue_proc[PTP_MSG_TYPE_DELAY_REQ] = (ptp_msg_issue_proc_t)_ptp_msg_issue_delay_req;
        }

        break;

    default:
        break;
    }
}

