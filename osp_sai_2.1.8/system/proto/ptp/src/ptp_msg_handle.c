
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_foreign_define.h"
#include "gen/tbl_ptp_foreign.h"

#include "ptp_constant.h"
#include "ptp_error.h"
#include "ptp_desc.h"
#include "ptp_util.h"
#include "ptp_sock.h"
#include "ptp_ops.h"
#include "ptp_servo.h"
#include "ptp_proto.h"
#include "ptp_bmc.h"
#include "ptp_api.h"
#include "ptp_msg.h"
#include "ptp_msg_issue.h"
#include "ptp_msg_handle.h"

static uint32
_ptp_msg_handle_pdelay_req(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Pdelay_Req in %s, is %s, rx %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state),
        ptp_desc_ptp_timestamp(p_rx_ts));

    ptp_common_message_header_t *rx_hdr, *tx_hdr, *tx_hdr_fu;
    ptp_sk_buff_t *tx_skb, *tx_skb_fu;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    glb_ptp_timestamp_t tx_ts;
    int32 ret = 0; 
    
    /*1.prepare a Pdelay_Resp and a Pdelay_Resp_Follow_Up*/
    /*1.1.allocate buffer for Pdelay_Resp and a Pdelay_Resp_Follow_Up*/
    /*XCALLOC in ptp_alloc_skb so all bit will be set to 0x00*/
    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    tx_skb_fu = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb || !tx_skb_fu)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Pdelay_Resp");
        goto PTP_FINALLY;
    }

    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;
    tx_skb->len = PTP_MSG_PDELAY_RESP_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;
    tx_skb_fu->len = PTP_MSG_PDELAY_RESP_FOLLOW_UP_LENGTH;
    tx_hdr_fu = (ptp_common_message_header_t *) tx_skb_fu->data;

    /*1.2.prepare for Pdelay_Resp message header*/
    tx_hdr->message_type = PTP_MSG_TYPE_PDELAY_RESP;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_PDELAY_RESP_LENGTH);
    tx_hdr->flag_field_0_two_step_flag = g_p_master->default_ds.two_step_flag;
    tx_hdr->control_field = PTP_MSG_CTRL_ALL_OTHERS;
    tx_hdr->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;
    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);

    /*1.3.prepare for Pdelay_Resp_Follow_Up message header*/
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        tx_hdr_fu->message_type = PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP;
        tx_hdr_fu->version_ptp = p_port->port_ds.version_number;
        tx_hdr_fu->message_length = sal_hton16(PTP_MSG_PDELAY_RESP_FOLLOW_UP_LENGTH);
        tx_hdr_fu->control_field = 0x05;
        tx_hdr_fu->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;
        sal_memcpy(tx_hdr_fu->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
        tx_hdr_fu->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);
    }
    
    /*2.copy the correctionField from Pdelay_Req message to the correctionField of the Pdelay_Resp_Follow_Up
        message, and set correctionField of the Pdelay_Resp message to 0*/
    /*DO NOT flip32 since they are same byte order*/
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        tx_hdr_fu->correction_field_msb = rx_hdr->correction_field_msb;
        tx_hdr_fu->correction_field_lsb = rx_hdr->correction_field_lsb;
    }
    else
    {
        tx_hdr->correction_field_msb = rx_hdr->correction_field_msb;
        tx_hdr->correction_field_lsb = rx_hdr->correction_field_lsb;
    }
    
    /*3.Copy the sequenceId field from the Pdelay_Req message to the sequenceId field of the
        Pdelay_Resp and the Pdelay_Resp_Follow_Up messages*/
    /*DO NOT flip32 since they are same byte order*/
    tx_hdr->sequence_id = rx_hdr->sequence_id;
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        tx_hdr_fu->sequence_id = rx_hdr->sequence_id;
    }
    
    /*4.Copy the sourcePortIdentity field from the Pdelay_Req message to the requestingPortIdentity
        field of the Pdelay_Resp and the Pdelay_Resp_Follow_Up messages*/
    sal_memcpy((tx_skb->data + 44), (rx_skb->data + 20), GLB_PTP_PORT_ID_LEN);
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        sal_memcpy((tx_skb_fu->data + 44), (rx_skb->data + 20), GLB_PTP_PORT_ID_LEN);
    }
    
    /*5.Copy the domainNumber field from the Pdelay_Req message to the domainNumber field of the
        Pdelay_Resp and Pdelay_Resp_Follow_Up messages*/
    /*DO NOT flip32 since they are same byte order*/
    tx_hdr->domain_number = rx_hdr->domain_number;
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        tx_hdr_fu->domain_number = rx_hdr->domain_number;
    }
    
    /*we use option 2*/
    /*6.1.In the Pdelay_Resp message, set the requestReceiptTimestamp field to the seconds and
        nanoseconds portion of the time t2, and subtract any fractional nanosecond portion of t2
        from the correctionField*/
    if(tx_hdr->flag_field_0_two_step_flag)
    {
        ptp_util_put_timestamp_into_skb(tx_skb, PTP_MSG_TS_OFFSET, p_rx_ts);
    }

    #ifdef DUET2
    uint64 boot_time_msec = 0;
    int64 total_offset;
    
    //add by weij for bug 45114, use uninterrupt mode
    boot_time_msec = ptp_api_get_global_enable_time();
    tx_ts.sec.lsb = (uint32)(boot_time_msec/1000);
    tx_ts.ns = (uint32)((boot_time_msec%1000)*1000000);
    #endif
    
    /*6.2.the Pdelay_Resp message and generate timestamp t3 upon sending*/
    if(! tx_hdr->flag_field_0_two_step_flag)
    {
         ret = ptp_sock_send(p_port, tx_skb, p_rx_ts);
    }
    else
    {
        #ifndef DUET2
        ret = ptp_sock_send_w_ts_resp(p_port, tx_skb, &tx_ts, NULL);
        #else
        //add by weij for bug 45114, use uninterrupt mode
        ret = ptp_sock_send(p_port, tx_skb, &tx_ts);
        #endif
    }

    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to response Pdelay_Req using Pdelay_Resp, ret %d", ret);
        goto PTP_FINALLY;
    }
    if(! tx_hdr->flag_field_0_two_step_flag)
    {
        goto PTP_FINALLY;
    }
    
    /*6.3.In the Pdelay_Resp_Follow_Up message, set the responseOriginTimestamp field to the
        seconds and nanoseconds portion of the time t3, and add any fractional nanosecond
        portion of t3 to the correctionField*/

    //add by weij for bug 45114, use uninterrupt mode
    #ifndef DUET2
    if((tx_ts.ns + p_port->egress_latency) > PTP_MAX_NS)
    {
        tx_ts.ns = tx_ts.ns + p_port->egress_latency - PTP_MAX_NS;
        tx_ts.sec.lsb = tx_ts.sec.lsb + 1;
    }
    else
    {
        tx_ts.ns = tx_ts.ns + p_port->egress_latency;
    }
    #else
    total_offset = g_p_master->total_offset + boot_time_msec*1000000;
    tx_ts.sec.lsb = total_offset/PTP_MAX_NS;
    tx_ts.ns = total_offset%PTP_MAX_NS;
    #endif
    
    ptp_util_put_timestamp_into_skb(tx_skb_fu, PTP_MSG_TS_OFFSET, &tx_ts);

    /*6.4.Issue the Pdelay_Resp_Follow_Up message*/
    ret = ptp_sock_send(p_port, tx_skb_fu, p_rx_ts);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to response Pdelay_Req using Pdelay_Resp_Follow_Up, ret %d", ret);
        goto PTP_FINALLY;
    }

PTP_FINALLY:

    ptp_free_skb(tx_skb);
    ptp_free_skb(tx_skb_fu);

    /*although sending is fail, the receipt is successful*/
    return TRUE;
}

static uint32
_ptp_msg_handle_pdelay_resp(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Pdelay_Resp in %s, is %s, rx %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state), 
        ptp_desc_ptp_timestamp(p_rx_ts));
        
    ptp_common_message_header_t *hdr;
    ptp_msg_pdelay_resp_t msg;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;

    sal_memset(&msg, 0x00, sizeof(ptp_msg_pdelay_resp_t));
    ptp_msg_unpack_pdelay_resp(rx_skb, &msg);

    #ifdef _GLB_UML_SYSTEM_
    sal_memset(p_rx_ts, 0x00, sizeof(glb_ptp_timestamp_t));
    #endif

    if (!ptp_ops_portid_identical_with_self(p_port, &msg.requesting_port_identity))
    {
        PTP_DBG_PACKET_DISCARD("Pdelay_Resp for current port is not the requestor of the Pdelay_Resp");
        return FALSE;
    }

    if (p_port->p.sent_pdelay_req_sequence_id != sal_ntoh16(hdr->sequence_id) + 1)
    {
        PTP_DBG_PACKET_DISCARD("Pdelay_Resp is too older to match the Pdelay_Req last sent");
        return FALSE;
    }

    /*Store t4 (Fig 35)*/
    p_port->p.pdelay_resp_receive_time.sec.msb = p_rx_ts->sec.msb;
    p_port->p.pdelay_resp_receive_time.sec.lsb = p_rx_ts->sec.lsb;
    p_port->p.pdelay_resp_receive_time.ns = p_rx_ts->ns;

    PTP_DBG_PACKET_RX("Handle Pdelay_Resp sequence id =%d, pdelay_resp_receive_time %s\n", 
        p_port->p.sent_pdelay_req_sequence_id, ptp_desc_ptp_timestamp(p_rx_ts));

    /*store t2 (Fig 35)*/
    p_port->p.pdelay_req_receive_time.sec.msb = msg.request_receipt_timestamp.sec.msb;
    p_port->p.pdelay_req_receive_time.sec.lsb = msg.request_receipt_timestamp.sec.lsb;
    p_port->p.pdelay_req_receive_time.ns = msg.request_receipt_timestamp.ns;

    PTP_DBG_PACKET_RX("Handle Pdelay_Resp sequence id =%d, pdelay_req_receive_time %s\n", 
        p_port->p.sent_pdelay_req_sequence_id, ptp_desc_ptp_timestamp(&(msg.request_receipt_timestamp)));

    /*store the correction field*/
    p_port->p.last_pdelay_resp_correction_field.s.msb = sal_ntoh32(hdr->correction_field_msb);
    p_port->p.last_pdelay_resp_correction_field.s.lsb = sal_ntoh32(hdr->correction_field_lsb);

    /*If asymmetry corrections are required, modify the correctionField of the Pdelay_Resp message per 11.6.5*/
    p_port->p.last_pdelay_resp_correction_field.v += p_port->asymmetry_correction.v;

    if (hdr->flag_field_0_two_step_flag)
    {
        p_port->p.waiting_for_pdelay_resp_follow = TRUE;
    }
    else
    {
        p_port->p.waiting_for_pdelay_resp_follow = FALSE;

        /*Store t3 (Fig 35)*/
        p_port->p.pdelay_resp_send_time.sec.msb = 0;
        p_port->p.pdelay_resp_send_time.sec.lsb = 0;
        p_port->p.pdelay_resp_send_time.ns = 0;

        /*Store t2 (Fig 35)*/
        p_port->p.pdelay_req_receive_time.sec.msb = 0;
        p_port->p.pdelay_req_receive_time.sec.lsb = 0;
        p_port->p.pdelay_req_receive_time.ns = 0;

        ptp_servo_update_peerdelay(p_port);
    }
    
#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "pdelay_reqreT2, second = %d, nansecond = %d intr_seq_id = %d\n",
                p_port->p.pdelay_req_receive_time.sec.lsb, p_port->p.pdelay_req_receive_time.ns, p_port->p.pdelay_req_receive_time.intr_seq_id); 

    sal_fprintf(fp_console, "pdelay_repreT4, second = %d, nansecond = %d intr_seq_id = %d\n",
                p_port->p.pdelay_resp_receive_time.sec.lsb, p_port->p.pdelay_resp_receive_time.ns, p_port->p.pdelay_resp_receive_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    return TRUE;
}

static uint32
_ptp_msg_handle_pdelay_resp_follow_up(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Pdelay_Resp_Follow_Up in %s, is %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state));
        
    ptp_common_message_header_t *hdr;
    ptp_msg_pdelay_resp_followup_t msg;
    glb_ptp_timeinterval_t correction_field;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;

    sal_memset(&msg, 0x00, sizeof(ptp_msg_pdelay_resp_followup_t));
    ptp_msg_unpack_pdelay_resp_follow_up(rx_skb, &msg);

    if (!ptp_ops_portid_identical_with_self(p_port, &msg.requesting_port_identity))
    {
        PTP_DBG_PACKET_DISCARD("Current port is not the requestor of the Pdelay_Resp_Follow_Up");
        return FALSE;
    }

    if (p_port->p.sent_pdelay_req_sequence_id != sal_ntoh16(hdr->sequence_id) + 1)
    {
        PTP_DBG_PACKET_DISCARD("Pdelay_Resp_Follow_Up is too older to match the Pdelay_Req last sent");
        return FALSE;
    }

    if (!p_port->p.waiting_for_pdelay_resp_follow)
    {
        PTP_DBG_PACKET_DISCARD("Pdelay_Resp is not wait for Pdelay_Resp_Follow_Up");
        return FALSE;
    }

    p_port->p.waiting_for_pdelay_resp_follow = FALSE;

    /*Store t3 (Fig 35)*/
    p_port->p.pdelay_resp_send_time.sec.msb = msg.response_origin_timestamp.sec.msb;
    p_port->p.pdelay_resp_send_time.sec.lsb = msg.response_origin_timestamp.sec.lsb;
    p_port->p.pdelay_resp_send_time.ns = msg.response_origin_timestamp.ns;

    PTP_DBG_PACKET_RX("Handle Pdelay_Resp_Follow_Up sequence id =%d, pdelay_resp_send_time %s\n", 
        p_port->p.sent_pdelay_req_sequence_id, ptp_desc_ptp_timestamp(&(msg.response_origin_timestamp)));

#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "pdelay_repseT3, second = %d, nansecond = %d intr_seq_id = %d\n",
                p_port->p.pdelay_resp_send_time.sec.lsb, p_port->p.pdelay_resp_send_time.ns, p_port->p.pdelay_resp_send_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    correction_field.s.msb = sal_ntoh32(hdr->correction_field_msb);
    correction_field.s.lsb = sal_ntoh32(hdr->correction_field_lsb);

    p_port->p.last_pdelay_resp_correction_field.v += correction_field.v;
    ptp_servo_update_peerdelay(p_port);

    return TRUE;
}

static uint32
_ptp_msg_handle_announce(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Announce in %s, is %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state));
        
    int32 ret = 0;
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_common_message_header_t *hdr;
    glb_ptp_port_id_t src_port_id;
    ptp_msg_announce_t msg;
    tbl_ptp_foreign_t *p_ptp_foreign = NULL;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;

    sal_memset(&msg, 0x00, sizeof(ptp_msg_announce_t));
    ptp_msg_unpack_announce(rx_skb, &msg);

    if (msg.steps_removed >= 255)
    {
        PTP_DBG_PACKET_DISCARD("Discard announce by steps_removed %hu", msg.steps_removed);
        return FALSE;
    }

    if (p_port->port_ds.log_announce_interval != hdr->log_message_interval)
    {
        PTP_LOG_WARN("Received announce packet with an inconsistent announce interval");
        PTP_DBG_PACKET_DISCARD("Announce packet log_message_interval %hhu, current port log_announce_interval %hhu", 
            hdr->log_message_interval, p_port->port_ds.log_announce_interval);
        return FALSE;
    }

    sal_memcpy(src_port_id.clock_id, hdr->source_port_identity_clock_id, GLB_PTP_CLOCK_ID_LEN);
    src_port_id.port_num = sal_ntoh16(hdr->source_port_identity_port_num);

    if (0 == ptp_util_clockid_cmp(g_p_master->default_ds.clock_identity, src_port_id.clock_id))
    {
        PTP_DBG_PACKET_RX("Announce packet was from myself");
        /*9.3.2.5 a) if S was sent from port "r" or from any other port on the clock containing port "r"*/
        if (p_port->port_ds.port_identity_port_num > src_port_id.port_num)
        {
            /*9.5.2.3*/
            p_port->recommended_state = PTP_STATE_PASSIVE;
            ptp_proto_to_state(p_port);
        }

        PTP_DBG_PACKET_DISCARD("Discard announce for incoming from the same clock");
        return FALSE;
    }

    if (0 == ptp_util_clockid_cmp(g_p_master->parent_ds.parent_port_identity.clock_id, src_port_id.clock_id))
    {
        PTP_DBG_PACKET_RX("Announce was from current parent, restart receipt timer");
        /*Reset Timer handling Announce receipt timeout*/
        ptp_announce_receipt_time_restart(p_port);

        if (g_p_master->parent_ds.parent_port_identity.port_num == src_port_id.port_num)
        {
            ptp_bmc_update_s1(p_port, hdr, &msg);
        }
    }
    else if (p_port->rbest)
    {
        PTP_DBG_PACKET_RX("Current rbest exist");
        p_ptp_foreign = (tbl_ptp_foreign_t *)p_port->rbest;
        if (0 == ptp_util_portid_cmp(&(p_ptp_foreign->key.foreign_master_port_identity), &src_port_id))
        {
            PTP_DBG_PACKET_RX("Anounnce was from current rbest, restart receipt timer");
            ptp_announce_receipt_time_restart(p_port);
        }
    }

    PTP_DBG_PACKET_RX("Announce from clock_id %s, port_num %d",
        ptp_desc_clockid(src_port_id.clock_id), src_port_id.port_num);

    ret = ptp_bmc_add_foreign(p_port, rx_skb);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_DBG_BMC("Add foreign failed in %s", ifname_ext);
        return FALSE;
    }

    return TRUE;
}

static uint32
_ptp_msg_handle_sync(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Sync in %s, is %s, rx %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state), 
        ptp_desc_ptp_timestamp(p_rx_ts));
        
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_common_message_header_t *hdr;
    glb_ptp_port_id_t src_port_id;
    ptp_msg_sync_t msg;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;
    
    sal_memset(&msg, 0x00, sizeof(ptp_msg_sync_t));
    ptp_msg_unpack_sync(rx_skb, &msg);

    sal_memcpy(src_port_id.clock_id, hdr->source_port_identity_clock_id, GLB_PTP_CLOCK_ID_LEN);
    src_port_id.port_num = sal_ntoh16(hdr->source_port_identity_port_num);

    #ifdef _GLB_UML_SYSTEM_
    sal_memset(p_rx_ts, 0x00, sizeof(glb_ptp_timestamp_t));
    #endif

    if (ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &src_port_id))
    {
        PTP_DBG_PACKET_DISCARD("Discard Sync that not from current parent");
        return FALSE;
    }

    /*sync packet receive time, from skaddr*/
    /*Figure 12 T2*/
    p_port->p.sync_receive_time.sec.msb = p_rx_ts->sec.msb;
    p_port->p.sync_receive_time.sec.lsb = p_rx_ts->sec.lsb;
    p_port->p.sync_receive_time.ns = p_rx_ts->ns;
    p_port->p.recv_sync_sequence_id = sal_ntoh16(hdr->sequence_id);

#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "Sync_recT2, offsets4 = %d, offsetn4 = %d intr_seq_id = %d\n",
                p_port->p.sync_receive_time.sec.lsb, p_port->p.sync_receive_time.ns, p_port->p.sync_receive_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    PTP_DBG_PACKET_RX("Handle Sync sequence id =%d, rx %s\n", 
        p_port->p.recv_sync_sequence_id, ptp_desc_ptp_timestamp(p_rx_ts));

    /*Save correctionField of Sync message*/
    p_port->p.last_sync_correction_field.s.msb = sal_ntoh32(hdr->correction_field_msb);
    p_port->p.last_sync_correction_field.s.lsb = sal_ntoh32(hdr->correction_field_lsb);

    uint32 a = sal_ntoh32(hdr->correction_field_msb);
    uint32 b = sal_ntoh32(hdr->correction_field_lsb);
    
    PTP_DBG_PACKET_RX("Correction field msb %u lsb %u, asymmetry correction %lld\n", a, b, p_port->asymmetry_correction.v);
    //PTP_DBG_PACKET_RX("msb %u lsb %u\n", p_port->p.last_sync_correction_field.s.msb, p_port->p.last_sync_correction_field.s.msb);
    
    /*If asymmetry corrections are required, modify the correctionField of the Pdelay_Resp message per 11.6.5*/
    p_port->p.last_sync_correction_field.v += p_port->asymmetry_correction.v;

    PTP_DBG_PACKET_RX("Add last sync correction to asymmetry correction, result %lld\n", p_port->p.last_sync_correction_field.v);
    
    if (hdr->flag_field_0_two_step_flag)
    {
        p_port->p.waiting_for_follow = TRUE;
    }
    else
    {
        p_port->p.waiting_for_follow = FALSE;

        /*sync packet send time, from packet tail*/
        /*Figure 12 T1*/
        p_port->p.sync_send_time.sec.msb = msg.origin_timestamp.sec.msb;
        p_port->p.sync_send_time.sec.lsb = msg.origin_timestamp.sec.lsb;
        p_port->p.sync_send_time.ns = msg.origin_timestamp.ns;

        PTP_DBG_PACKET_RX("Handle Sync sequence id =%d, sync send time %s\n", 
            p_port->p.recv_sync_sequence_id, ptp_desc_ptp_timestamp(&(msg.origin_timestamp)));
        
        ptp_servo_update_offset(p_port);
        ptp_servo_update_clock(p_port);
    }

    return TRUE;
}

static uint32
_ptp_msg_handle_follow_up(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Follow_Up in %s, is %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state));
        
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_common_message_header_t *hdr;
    glb_ptp_port_id_t src_port_id;
    ptp_msg_follow_up_t msg;
    glb_ptp_timeinterval_t correction_field;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;

    sal_memset(&msg, 0x00, sizeof(ptp_msg_follow_up_t));
    ptp_msg_unpack_follow_up(rx_skb, &msg);

    sal_memcpy(src_port_id.clock_id, hdr->source_port_identity_clock_id, GLB_PTP_CLOCK_ID_LEN);
    src_port_id.port_num = sal_ntoh16(hdr->source_port_identity_port_num);

    if (ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &src_port_id))
    {
        PTP_DBG_PACKET_DISCARD("Follow_Up not from current parent");
        return FALSE;
    }

    if (p_port->p.recv_sync_sequence_id != sal_ntoh16(hdr->sequence_id))
    {
        PTP_DBG_PACKET_DISCARD("Follow_Up not match current sync");
        return FALSE;
    }

    if (!p_port->p.waiting_for_follow)
    {
        PTP_DBG_PACKET_DISCARD("Sync is not waiting for the follow_up");
        return FALSE;
    }

    p_port->p.waiting_for_follow = FALSE;

    correction_field.s.msb = sal_ntoh32(hdr->correction_field_msb);
    correction_field.s.lsb = sal_ntoh32(hdr->correction_field_lsb);

    uint32 a = sal_ntoh32(hdr->correction_field_msb);
    uint32 b = sal_ntoh32(hdr->correction_field_lsb);
    
    PTP_DBG_PACKET_RX("Correction field msb %u lsb %u, last sync correction field %lld\n", a, b, p_port->p.last_sync_correction_field.v);
    
    p_port->p.last_sync_correction_field.v += correction_field.v;
    PTP_DBG_PACKET_RX("Add last sync correction field to correction field, result %lld\n", p_port->p.last_sync_correction_field.v);

    /*sync packet send time, from packet tail*/
    /*Figure 12 T1*/
    p_port->p.sync_send_time.sec.msb = msg.precise_origin_timestamp.sec.msb;
    p_port->p.sync_send_time.sec.lsb = msg.precise_origin_timestamp.sec.lsb;
    p_port->p.sync_send_time.ns = msg.precise_origin_timestamp.ns;

    PTP_DBG_PACKET_RX("Handle Follow_Up sequence id %d, sync send time %s\n", 
            p_port->p.recv_sync_sequence_id, ptp_desc_ptp_timestamp(&(msg.precise_origin_timestamp)));
#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "Sync_senT1, offsets4 = %d, offsetn4 = %d intr_seq_id = %d\n",
               p_port->p.sync_send_time.sec.lsb,  p_port->p.sync_send_time.ns, p_port->p.sync_send_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    ptp_servo_update_offset(p_port);
    ptp_servo_update_clock(p_port);

    return TRUE;
}

static uint32
_ptp_msg_handle_delay_req(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Delay_Req in %s, is %s, rx %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state), 
        ptp_desc_ptp_timestamp(p_rx_ts));
        
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_sk_buff_t *tx_skb;
    ptp_common_message_header_t *rx_hdr, *tx_hdr;
    int32 ret = 0;

    /*1.prepare a Delay_Resp*/
    /*1.1.allocate buffer for Delay_Resp*/
    /*XCALLOC in ptp_alloc_skb so all bit will be set to 0x00*/
    tx_skb = ptp_alloc_skb(PTP_MSG_BUFFER_LENGTH);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for issue Delay_Resp");
        goto PTP_FINALLY;
    }

    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;
    tx_skb->len = PTP_MSG_DELAY_RESP_LENGTH;
    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    /*1.2.prepare for Delay_Resp message header*/
    tx_hdr->message_type = PTP_MSG_TYPE_DELAY_RESP;
    tx_hdr->version_ptp = p_port->port_ds.version_number;
    tx_hdr->message_length = sal_hton16(PTP_MSG_DELAY_RESP_LENGTH);
    tx_hdr->control_field = PTP_MSG_CTRL_DELAY_RESP;
    
    /*fix bug 46580, sync/follow_up/delay_resp unicast packet is 0x7f*/
    addr_ipv4_t tmp_ip;
    sal_memset(&tmp_ip, 0, sizeof(addr_ipv4_t));
    
    if (0 == sal_memcmp(&tmp_ip, &(p_port->port_addr.dest_ip_addr), sizeof(tmp_ip)))
    {
        tx_hdr->log_message_interval = p_port->port_ds.log_min_delay_req_interval;
    }
    else
    {
        tx_hdr->log_message_interval = PTP_LOG_MSG_INTERVAL_DEFAULT;
    }
    
    sal_memcpy(tx_hdr->source_port_identity_clock_id, g_p_master->default_ds.clock_identity, GLB_PTP_CLOCK_ID_LEN);
    tx_hdr->source_port_identity_port_num = sal_hton16(p_port->port_ds.port_identity_port_num);

    /*2.Copy the sequenceId field from the Delay_Req message to the sequenceId field of the Delay_Resp message*/
    /*DO NOT flip32 since they are same byte order*/
    tx_hdr->sequence_id = rx_hdr->sequence_id;

    /*3.Copy the sourcePortIdentity field from the Delay_Req message to the requestingPortIdentity
        field of the Delay_Resp message*/
    sal_memcpy((tx_skb->data + 44), (rx_skb->data + 20), GLB_PTP_PORT_ID_LEN);

    /*4.Copy the domainNumber field from the Delay_Req message to the domainNumber field of the Delay_Resp message*/
    /*DO NOT flip32 since they are same byte order*/
    tx_hdr->domain_number = rx_hdr->domain_number;

    /*5.Set the correctionField of the Delay_Resp message to 0*/
    /*6.Add the correctionField of the Delay_Req message to the correctionField of the Delay_Resp message*/
    tx_hdr->correction_field_msb = rx_hdr->correction_field_msb;
    tx_hdr->correction_field_lsb = rx_hdr->correction_field_lsb;

    PTP_DBG_PACKET_RX("Delay_Req correction field: %u %u\n", tx_hdr->correction_field_msb, tx_hdr->correction_field_lsb);

    /*7.Set the receiveTimestamp field of the Delay_Resp message to the seconds and nanoseconds portion of the time t4*/
    ptp_util_put_timestamp_into_skb(tx_skb, PTP_MSG_TS_OFFSET, p_rx_ts);

    ret = ptp_sock_send(p_port, tx_skb, p_rx_ts);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to response Delay_Req using Delay_Resp, ret %d", ret);
        goto PTP_FINALLY;
    }

PTP_FINALLY:

    ptp_free_skb(tx_skb);

    /*although sending is fail, the receipt is successful*/
    return TRUE;
}

static uint32
_ptp_msg_handle_delay_resp(tbl_ptp_port_t* p_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("Handle Delay_Resp in %s, is %s", 
        IFNAME_ETH2FULL(p_port->key.name, ifname_ext), ptp_desc_state(p_port->port_ds.port_state));
        
    tbl_ptp_global_t *g_p_master = tbl_ptp_global_get_ptp_global();
    ptp_common_message_header_t *hdr;
    glb_ptp_port_id_t src_port_id;
    ptp_msg_delay_resp_t msg;
    
    hdr = (ptp_common_message_header_t *) rx_skb->data;

    sal_memset(&msg, 0x00, sizeof(ptp_msg_delay_resp_t));
    ptp_msg_unpack_delay_resp(rx_skb, &msg);

    sal_memcpy(src_port_id.clock_id, hdr->source_port_identity_clock_id, GLB_PTP_CLOCK_ID_LEN);
    src_port_id.port_num = sal_ntoh16(hdr->source_port_identity_port_num);

    if (ptp_util_portid_cmp(&g_p_master->parent_ds.parent_port_identity, &src_port_id))
    {
        PTP_DBG_PACKET_DISCARD("Delay_Resp not from current parent");
        return FALSE;
    }

    if (!ptp_ops_portid_identical_with_self(p_port, &msg.requesting_port_identity))
    {
        PTP_DBG_PACKET_DISCARD("Delay_Resp current port is not the requestor of the Delay_Resp");
        return FALSE;
    }

    if (p_port->p.sent_delay_req_sequence_id != sal_ntoh16(hdr->sequence_id) + 1)
    {
        PTP_DBG_PACKET_DISCARD("Delay_Resp is too older to match the Delay_Req last sent");
        return FALSE;
    }

    /*Figure 12 T4*/
    p_port->p.delay_req_receive_time.sec.msb = msg.receive_timestamp.sec.msb;
    p_port->p.delay_req_receive_time.sec.lsb = msg.receive_timestamp.sec.lsb;
    p_port->p.delay_req_receive_time.ns = msg.receive_timestamp.ns;

    p_port->p.last_delay_resp_correction_field.s.msb = sal_ntoh32(hdr->correction_field_msb);
    p_port->p.last_delay_resp_correction_field.s.lsb = sal_ntoh32(hdr->correction_field_lsb);

#if 0
    FILE * fp_console = NULL;                        
    fp_console = fopen("/tmp/debug", "a+"); 
    sal_fprintf(fp_console, "delay_resT4, offsets4 = %d, offsetn4 = %d, intr_seq_id = %d \n",
    p_port->p.delay_req_receive_time.sec.lsb, p_port->p.delay_req_receive_time.ns, p_port->p.delay_req_receive_time.intr_seq_id); 
    fclose(fp_console); 
#endif

    ptp_servo_update_delay(p_port);

    int8 master_min_delay_req_interval = (int8) hdr->log_message_interval;
    if (PTP_MIN_DELAYREQ_INTERVAL_MIN <= master_min_delay_req_interval
        && master_min_delay_req_interval <= PTP_MIN_DELAYREQ_INTERVAL_MAX)
    {
        g_p_master->master_min_delay_req_interval = master_min_delay_req_interval;
    }
    else
    {
        g_p_master->master_min_delay_req_interval = PTP_ANNOUNCE_INTERVAL_DEF;
    }

    /*although sending is fail, the receipt is successful*/
    return TRUE;
}

void
ptp_handle_register(tbl_ptp_port_t *p_port)
{
    sal_memset(p_port->msg_proc_ops.handle_proc, 0x00, sizeof(ptp_msg_handle_proc_t) * PTP_MSG_TYPE_MAX);
    if (ptp_ops_is_tc())
    {
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_req;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp_follow_up;
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
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_req;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp_follow_up;
        }

        p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_ANNOUNCE] = (ptp_msg_handle_proc_t)_ptp_msg_handle_announce;
        break;

    case PTP_STATE_MASTER:
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_req;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp_follow_up;
        }
        else if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_DELAY_REQ] = (ptp_msg_handle_proc_t)_ptp_msg_handle_delay_req;
        }

        p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_ANNOUNCE] = (ptp_msg_handle_proc_t)_ptp_msg_handle_announce;
        break;

    case PTP_STATE_UNCALIBRATED:
    case PTP_STATE_SLAVE:
        p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_SYNC] = (ptp_msg_handle_proc_t)_ptp_msg_handle_sync;
        p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_FOLLOW_UP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_follow_up;
        if (PTP_DELAY_MECHANISM_P2P == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_REQ] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_req;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp;
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_PDELAY_RESP_FOLLOW_UP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_pdelay_resp_follow_up;
        }
        else if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_DELAY_RESP] = (ptp_msg_handle_proc_t)_ptp_msg_handle_delay_resp;
        }

        p_port->msg_proc_ops.handle_proc[PTP_MSG_TYPE_ANNOUNCE] = (ptp_msg_handle_proc_t)_ptp_msg_handle_announce;
        break;

    default:
        break;
    }
}

