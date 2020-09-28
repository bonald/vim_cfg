
#include "proto.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_ptp_residence_time_cache_define.h"
#include "gen/tbl_ptp_residence_time_cache.h"

#include "ptp_error.h"
#include "ptp_util.h"
#include "ptp_desc.h"
#include "ptp_ops.h"
#include "ptp_sock.h"
#include "ptp_msg.h"
#include "ptp_constant.h"
#include "ptp_msg_forward.h"

static uint32
_ptp_msg_forward_skip_port(tbl_ptp_port_t* p_ingress_port, tbl_ptp_port_t* p_egress_port)
{
    if (p_ingress_port == p_egress_port)
    {
        return TRUE;
    }

    if (!p_egress_port->enable || !p_egress_port->link_up)
    {
        return TRUE;
    }

    if (p_ingress_port->port_addr.protocol != p_egress_port->port_addr.protocol)
    {
        return TRUE;
    }

    if (p_ingress_port->vlan_info.vid != p_egress_port->vlan_info.vid)
    {
        return TRUE;
    }

    return FALSE;
}

void
ptp_residence_time_destroy_db(uint32 ifindex, uint32 destroy_all)
{
    //0:delay_req   1:sync   2:destroy_all
    tbl_ptp_residence_time_cache_master_t* p_ptp_residence_time_cache_master = tbl_ptp_residence_time_cache_get_master();
    tbl_ptp_residence_time_cache_t *p_db_ptp_residence_time_cache = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_ptp_residence_time_cache_master->ptp_residence_time_cache_list, p_db_ptp_residence_time_cache, listnode, next)
    {
        
        if ((ifindex == p_db_ptp_residence_time_cache->key.ifindex_belong_to) && (destroy_all == 2 || p_db_ptp_residence_time_cache->key.is_sync_ingress == destroy_all))
        {
            ptp_util_cancel_timer(&p_db_ptp_residence_time_cache->ptm_age_task_timer);
            tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_db_ptp_residence_time_cache->key));
        }
    }
}

static void
_ptp_residence_time_age_entry(void* p_data)
{
    tbl_ptp_residence_time_cache_t* pst_rt = p_data ? (tbl_ptp_residence_time_cache_t*)p_data : NULL;
    
    PTP_PTR_INVALID_RETURN_NULL(pst_rt);
    pst_rt->ptm_age_task_timer = NULL;

    if (pst_rt->key.ifindex_belong_to != 0)
    {
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(pst_rt->key));
    }

    return;
}

static int32
_ptp_residence_time_check_entry(tbl_ptp_residence_time_cache_t *pst_rt)
{
    if (pst_rt->outdated)
    {
        ptp_util_cancel_timer(&pst_rt->ptm_age_task_timer);
        pst_rt->ptm_age_task_timer = ptp_util_add_timer_ms(_ptp_residence_time_age_entry, pst_rt, 0);
    }
    else
    {
        pst_rt->outdated = TRUE;
    }

    return PTP_E_SUCCESS;
}

void
ptp_residence_time_age_db(void)
{
    tbl_ptp_residence_time_cache_master_t* p_ptp_residence_time_cache_master = tbl_ptp_residence_time_cache_get_master();
    tbl_ptp_residence_time_cache_t *p_db_ptp_residence_time_cache = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    
    CTCLIB_SLIST_LOOP_DEL(p_ptp_residence_time_cache_master->ptp_residence_time_cache_list, p_db_ptp_residence_time_cache, listnode, next)
    {
        _ptp_residence_time_check_entry(p_db_ptp_residence_time_cache);
    }

    return;
}

static uint32
_ptp_msg_forward_announce(tbl_ptp_port_t* p_ingress_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    tbl_ptp_port_master_t * p_ptp_port_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    int32 ret;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    PTP_DBG_PACKET_RX("FWD Announce in %s", IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext));

    CTCLIB_SLIST_LOOP_DEL(p_ptp_port_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (ptp_ops_ifp_invalid(p_db_ptp_port)) 
        {
            continue;
        }

        tbl_ptp_port_t* p_egress_port = p_db_ptp_port;
        if (_ptp_msg_forward_skip_port(p_ingress_port, p_egress_port))
        {
            continue;
        }

        ptp_sk_buff_t *tx_skb = ptp_skb_copy(rx_skb);
        if (NULL == tx_skb)
        {
            PTP_LOG_ERR("Cannot allocate memory for forward Announce packet");
            continue;
        }

        ret = ptp_sock_send(p_egress_port, tx_skb, p_rx_ts);
        ptp_free_skb(tx_skb);
        PTP_OP_ERROR_CONTINUE(ret);
    }

    return TRUE;
}

static uint32
_ptp_msg_forward_sync(tbl_ptp_port_t* p_ingress_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    tbl_ptp_port_master_t * p_ptp_port_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    ptp_sk_buff_t *tx_skb = NULL;
    ptp_sk_buff_t *tx_skb_fu = NULL;
    ptp_common_message_header_t *tx_hdr = NULL;
    ptp_common_message_header_t *tx_hdr_fu = NULL;
    ptp_common_message_header_t *rx_hdr = NULL;
    glb_ptp_timestamp_t tx_ts;
    glb_ptp_timeinterval_t residence_time;
    int32 ret = 0;

    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    PTP_DBG_PACKET_RX("FWD Sync in %s", IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext));
    
    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;

#if 0 // TODO: Syntonization based on Sync message
    if (rx_hdr->domain_number == g_p_master->default_ds.primary_domain)
    {
        int32 ret = _ptp_msg_syntonization_sync();
        if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_WARN("Cannot syntonization based on Sync message.");
        }
    }
#endif

    CTCLIB_SLIST_LOOP_DEL(p_ptp_port_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (ptp_ops_ifp_invalid(p_db_ptp_port))
        {
            continue;
        }

        tbl_ptp_port_t* p_egress_port = p_db_ptp_port;
        if (_ptp_msg_forward_skip_port(p_ingress_port, p_egress_port))
        {
            continue;
        }

        tx_skb = ptp_skb_copy(rx_skb);
        if (!tx_skb)
        {
            PTP_LOG_ERR("Cannot allocate memory for forward Sync packet");
            continue;
        }

        tx_hdr = (ptp_common_message_header_t *) tx_skb->data;
        tx_hdr->flag_field_0_two_step_flag = 1;

        if (!rx_hdr->flag_field_0_two_step_flag)
        {
            tx_skb_fu = ptp_skb_copy(rx_skb);
            if (!tx_skb_fu)
            {
                PTP_LOG_ERR("Cannot allocate memory for forward Sync packet");
                goto PTP_FINALLY;
            }
        }

        sal_memset(&tx_ts, 0x00, sizeof(tx_ts));
       // int32 ret = ptp_sock_send_w_ts_resp(p_egress_port, tx_skb, &tx_ts, p_rx_ts);
	 ret = ptp_sock_send_w_ts_resp(p_egress_port, tx_skb, &tx_ts, NULL);

        if (PTP_E_IF_BLOCKED == ret)
        {
            goto PTP_FINALLY;
        }
        else if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_ERR("Failed to forward Sync packet to %s, ret %d", ptp_desc_ifname(p_egress_port), ret);
            goto PTP_FINALLY;
        }

        residence_time.v = ptp_util_time_sub_time(&tx_ts, p_rx_ts) << PTP_SCALED_NS_BITS;

        /*11.6.2 c) 2) If the transparent clock is a two-step clock, the correction shall be made by
            adding the value of the ingress path delayAsymmetry to the correctionField of the Follow_Up
            message associated with the received Sync message prior to transmission of the Follow_Up
            message on an egress port.*/
        residence_time.v += p_ingress_port->asymmetry_correction.v;

        if (PTP_DELAY_MECHANISM_P2P == p_ingress_port->port_ds.delay_mechanism)
        {
            residence_time.v += p_ingress_port->port_ds.peer_mean_path_delay << PTP_SCALED_NS_BITS;
        }

        /*two-step sync should not build Follow_Up but wait for Follow_Up*/
        if (rx_hdr->flag_field_0_two_step_flag)
        {
            /*cache residence time to the use of follow_up*/
            tbl_ptp_residence_time_cache_t residence;
            sal_memset(&residence, 0x00, sizeof(tbl_ptp_residence_time_cache_t));
            ptp_util_clockid_cpy(residence.key.source_port_id.clock_id, rx_hdr->source_port_identity_clock_id);
            residence.key.source_port_id.port_num = sal_ntoh16(rx_hdr->source_port_identity_port_num);
            residence.domain = rx_hdr->domain_number;
            residence.residence_time.v = residence_time.v;
            residence.sequence_id = sal_ntoh16(rx_hdr->sequence_id);

            residence.key.is_sync_ingress = TRUE;
            residence.key.ifindex_belong_to = p_egress_port->ifindex;
            residence.ifindex_port_ingress = p_ingress_port->ifindex;

            tbl_ptp_residence_time_cache_t *p_db_residence = NULL;
            
            p_db_residence = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&residence.key);
            if (p_db_residence)
            {
                ptp_util_cancel_timer(&p_db_residence->ptm_age_task_timer);
                p_db_residence->outdated = FALSE;
                p_db_residence->domain = residence.domain;
                p_db_residence->residence_time.v = residence.residence_time.v;
                p_db_residence->sequence_id = residence.sequence_id;
                p_db_residence->ifindex_port_ingress = residence.ifindex_port_ingress;
                tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED);
                tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN);
                tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME);
                tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID);
                tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS);
                goto PTP_FINALLY;
            }
            else
            {
                tbl_ptp_residence_time_cache_add_ptp_residence_time_cache(&residence);
                goto PTP_FINALLY;
            }
        }
        else
        {
            /*build a follow up*/
            tx_hdr_fu = (ptp_common_message_header_t *) tx_skb_fu->data;
            tx_hdr_fu->message_type = PTP_MSG_TYPE_FOLLOW_UP;
            tx_hdr_fu->flag_field_0_two_step_flag = 0;
            tx_hdr_fu->control_field = PTP_MSG_CTRL_FOLLOW_UP;

            tx_hdr_fu->correction_field_msb = sal_hton32(residence_time.s.msb);
            tx_hdr_fu->correction_field_lsb = sal_hton32(residence_time.s.lsb);
            
            PTP_DBG_PACKET_TX("Add residence time for Follow_Up in one step: %u %u\n", 
                tx_hdr_fu->correction_field_msb, tx_hdr_fu->correction_field_lsb);

            ret = ptp_sock_send(p_egress_port, tx_skb_fu, p_rx_ts);
            if (PTP_E_SUCCESS != ret)
            {
                PTP_LOG_ERR("Failed to issue Follow_Up, ret %d", ret);
                goto PTP_FINALLY;
            }
        }

PTP_FINALLY:
        ptp_free_skb(tx_skb);
        ptp_free_skb(tx_skb_fu);
    }

    return TRUE;
}

static uint32
_ptp_msg_forward_follow_up(tbl_ptp_port_t* p_ingress_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
   
    tbl_ptp_port_master_t * p_ptp_port_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    ptp_sk_buff_t *tx_skb_fu = NULL;
    ptp_common_message_header_t *tx_hdr_fu;
    ptp_common_message_header_t *rx_hdr;
    int32 ret = 0;

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    char ifname_ext_2[IFNAME_EXT_SIZE] = {0};

    PTP_DBG_PACKET_RX("FWD Follow_Up in %s", IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext));
    
    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;

#if 0 // TODO: Syntonization based on Sync message
    if (rx_hdr->domain_number == g_p_master->default_ds.primary_domain)
    {
        int32 ret = _ptp_msg_syntonization_sync();
        if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_WARN("Cannot syntonization based on Sync message.");
        }
    }
#endif

    CTCLIB_SLIST_LOOP_DEL(p_ptp_port_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (ptp_ops_ifp_invalid(p_db_ptp_port))
        {
            continue;
        }

        tbl_ptp_port_t* p_egress_port = p_db_ptp_port;
        if (_ptp_msg_forward_skip_port(p_ingress_port, p_egress_port))
        {
            continue;
        }

        tbl_ptp_residence_time_cache_t residence;
        sal_memset(&residence, 0x00, sizeof(tbl_ptp_residence_time_cache_t));
        ptp_util_clockid_cpy(residence.key.source_port_id.clock_id, rx_hdr->source_port_identity_clock_id);
        residence.key.source_port_id.port_num = sal_ntoh16(rx_hdr->source_port_identity_port_num);
        residence.domain = rx_hdr->domain_number;

        residence.key.ifindex_belong_to = p_egress_port->ifindex;
        residence.key.is_sync_ingress = TRUE;
        tbl_ptp_residence_time_cache_t *p_db_residence = NULL;

        p_db_residence = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&(residence.key));
        if (!p_db_residence)
        {
            PTP_DBG_PACKET_DISCARD("%s have no corresponding Sync sent out from %s@%hu on domain %hu",
                    IFNAME_ETH2FULL(p_egress_port->key.name, ifname_ext_2), ptp_desc_clockid(rx_hdr->source_port_identity_clock_id),
                    sal_ntoh16(rx_hdr->source_port_identity_port_num), rx_hdr->domain_number);
            continue;
        }
        
        ptp_util_cancel_timer(&p_db_residence->ptm_age_task_timer);

        if (p_db_residence->ifindex_port_ingress != p_ingress_port->ifindex)
        {
            tbl_ptp_port_t *p_db_ptp_port_tmp = NULL;
            tbl_interface_t *p_db_if = NULL;
            p_db_if = tbl_interface_get_interface_by_ifindex(p_db_residence->ifindex_port_ingress);
            if (NULL == p_db_if)
            {
                continue;
            }
            
            p_db_ptp_port_tmp = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&(p_db_if->key)));
            if (NULL == p_db_ptp_port_tmp)
            {
                continue;
            }
            PTP_DBG_PACKET_DISCARD("Sync is from %s, but the Follow_Up is from %s",
                    IFNAME_ETH2FULL(p_db_ptp_port_tmp->key.name, ifname_ext), IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext_2));
            continue;
        }

        if (p_db_residence->sequence_id != sal_ntoh16(rx_hdr->sequence_id))
        {
            PTP_DBG_PACKET_DISCARD("The sequence id of Sync is %hu, but Follow_Up is %hu",
                    p_db_residence->sequence_id, sal_ntoh16(rx_hdr->sequence_id));
            continue;
        }

        tx_skb_fu = ptp_skb_copy(rx_skb);
        if (!tx_skb_fu)
        {
            PTP_LOG_ERR("Cannot allocate memory for forward Follow_Up packet");
            continue;
        }

        tx_hdr_fu = (ptp_common_message_header_t *) tx_skb_fu->data;

        glb_ptp_timeinterval_t correction_field;
        correction_field.s.msb = sal_ntoh32(tx_hdr_fu->correction_field_msb);
        correction_field.s.lsb = sal_ntoh32(tx_hdr_fu->correction_field_lsb);
        PTP_DBG_PACKET_TX("Follow_Up correction field: %d %u %lld\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);

        /*add residence time*/
        correction_field.v += p_db_residence->residence_time.v;
        PTP_DBG_PACKET_TX("Follow_Up correction field add residence time: %d %u %llu\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);
        
        tx_hdr_fu->correction_field_msb = sal_hton32(correction_field.s.msb);
        tx_hdr_fu->correction_field_lsb = sal_hton32(correction_field.s.lsb);
        ptp_util_cancel_timer(&p_db_residence->ptm_age_task_timer);
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_db_residence->key));
        ret = ptp_sock_send(p_egress_port, tx_skb_fu, p_rx_ts);
        ptp_free_skb(tx_skb_fu);
        if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_ERR("Failed to forward Follow_Up, ret %d", ret);
        }
    }

    return TRUE;
}

static uint32
_ptp_msg_forward_delay_req(tbl_ptp_port_t* p_ingress_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    tbl_ptp_port_master_t * p_ptp_port_master = tbl_ptp_port_get_master();
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    ptp_sk_buff_t *tx_skb = NULL;
    ptp_common_message_header_t *rx_hdr;

    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    PTP_DBG_PACKET_RX("FWD Delay_Req in %s", IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext));
    
    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;

    CTCLIB_SLIST_LOOP_DEL(p_ptp_port_master->ptp_port_list, p_db_ptp_port, listnode, next)
    {
        if (ptp_ops_ifp_invalid(p_db_ptp_port))
        {
            continue;
        }

        tbl_ptp_port_t* p_egress_port = p_db_ptp_port;
        if (_ptp_msg_forward_skip_port(p_ingress_port, p_egress_port))
        {
            continue;
        }

        tx_skb = ptp_skb_copy(rx_skb);
        if (!tx_skb)
        {
            PTP_LOG_ERR("Cannot allocate memory for forward Delay_Req packet");
            continue;
        }

        glb_ptp_timestamp_t tx_ts;
     //   int32 ret = ptp_sock_send_w_ts_resp(p_egress_port, tx_skb, &tx_ts, p_rx_ts);
	  int32 ret = ptp_sock_send_w_ts_resp(p_egress_port, tx_skb, &tx_ts, NULL);

        if (PTP_E_IF_BLOCKED == ret)
        {
            goto PTP_FINALLY;
        }
        else if (PTP_E_SUCCESS != ret)
        {
            PTP_LOG_ERR("Failed to forward Delay_Req packet to %s", ptp_desc_ifname(p_egress_port));
            goto PTP_FINALLY;
        }

        glb_ptp_timeinterval_t residence_time;
        residence_time.v = ptp_util_time_sub_time(&tx_ts, p_rx_ts) << PTP_SCALED_NS_BITS;

        /*11.6.3 c) 2)If the end-to-end transparent clock is a two-step clock, the correction shall be made
            by subtracting the value of the egress path delayAsymmetry of the path connected from the egress
            port of the Delay_Req message from the correctionField of the Delay_Resp message associated with
            the original Delay_Req message prior to transmission of the Delay_Resp message.*/
        residence_time.v -= p_egress_port->asymmetry_correction.v;

        /*cache residence time to the use of delay_resp*/
        tbl_ptp_residence_time_cache_t residence;
        sal_memset(&residence, 0x00, sizeof(tbl_ptp_residence_time_cache_t));
        ptp_util_clockid_cpy(residence.key.source_port_id.clock_id, rx_hdr->source_port_identity_clock_id);
        residence.key.source_port_id.port_num = sal_ntoh16(rx_hdr->source_port_identity_port_num);
        residence.domain = rx_hdr->domain_number;
        residence.residence_time.v = residence_time.v;
        residence.sequence_id = sal_ntoh16(rx_hdr->sequence_id);
        //residence.port.delay_req_ingress = p_ingress_port;
        //residence.p_tree_belong_to = p_egress_port->p_delay_req_cache;
        residence.key.ifindex_belong_to = p_egress_port->ifindex;
        residence.key.is_sync_ingress = FALSE;
        residence.ifindex_port_ingress = p_ingress_port->ifindex;

        tbl_ptp_residence_time_cache_t *p_db_residence = NULL;
            
        p_db_residence = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&residence.key);
        if (p_db_residence)
        {
            ptp_util_cancel_timer(&p_db_residence->ptm_age_task_timer);
            p_db_residence->outdated = FALSE;
            p_db_residence->domain = residence.domain;
            p_db_residence->residence_time.v = residence.residence_time.v;
            p_db_residence->sequence_id = residence.sequence_id;
            //p_db_residence->port.delay_req_ingress = residence.port.delay_req_ingress;
            p_db_residence->ifindex_port_ingress = residence.ifindex_port_ingress;

            tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_OUTDATED);
            tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_DOMAIN);
            tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_RESIDENCE_TIME);
            tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_SEQUENCE_ID);
            tbl_ptp_residence_time_cache_set_ptp_residence_time_cache_field(p_db_residence, TBL_PTP_RESIDENCE_TIME_CACHE_FLD_IFINDEX_PORT_INGRESS);

            goto PTP_FINALLY;
        }
        else
        {
            ret = tbl_ptp_residence_time_cache_add_ptp_residence_time_cache(&residence);
            if (ret != PM_E_NONE)
            {
                PTP_LOG_ERR("Cannot allocate memory for add ptp residence time cache");
            }
            goto PTP_FINALLY;
        }

PTP_FINALLY:
        ptp_free_skb(tx_skb);
    }

    return TRUE;
}

static uint32
_ptp_msg_forward_delay_resp(tbl_ptp_port_t* p_ingress_port, ptp_sk_buff_t* rx_skb, glb_ptp_timestamp_t* p_rx_ts)
{
    ptp_common_message_header_t *rx_hdr;
    rx_hdr = (ptp_common_message_header_t *) rx_skb->data;

    ptp_sk_buff_t *tx_skb = NULL;
    ptp_common_message_header_t *tx_hdr;

    ptp_msg_delay_resp_t msg;
    sal_memset(&msg, 0x00, sizeof(ptp_msg_delay_resp_t));
    ptp_msg_unpack_delay_resp(rx_skb, &msg);

    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    PTP_DBG_PACKET_RX("FWD Delay_Resp in %s", IFNAME_ETH2FULL(p_ingress_port->key.name, ifname_ext));

    tbl_ptp_residence_time_cache_t residence;
    sal_memset(&residence, 0x00, sizeof(tbl_ptp_residence_time_cache_t));
    ptp_util_clockid_cpy(residence.key.source_port_id.clock_id, msg.requesting_port_identity.clock_id);
    residence.key.source_port_id.port_num = msg.requesting_port_identity.port_num;
    residence.key.is_sync_ingress = FALSE;
    residence.key.ifindex_belong_to = p_ingress_port->ifindex;//todo
    residence.domain = rx_hdr->domain_number;

    tbl_ptp_residence_time_cache_t *p_residence = NULL;
    p_residence = tbl_ptp_residence_time_cache_get_ptp_residence_time_cache(&(residence.key));
    if (NULL == p_residence)
    {
        PTP_DBG_PACKET_DISCARD("%s have no corresponding Delay_Req sent out from %s@%hu on domain %hu",
                ifname_ext, ptp_desc_clockid(rx_hdr->source_port_identity_clock_id),
                sal_ntoh16(rx_hdr->source_port_identity_port_num), rx_hdr->domain_number);
        return FALSE;
    }

    ptp_util_cancel_timer(&p_residence->ptm_age_task_timer);

    if (p_residence->ifindex_port_ingress == p_ingress_port->ifindex)
    {
        PTP_DBG_PACKET_DISCARD("Delay_Req and Delay_Resp is both from %s", ifname_ext);
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_residence->key));
        return FALSE;
    }

    if (p_residence->sequence_id != sal_ntoh16(rx_hdr->sequence_id))
    {
        PTP_DBG_PACKET_DISCARD("The sequence id of Delay_Req is %hu, but Delay_Resp is %hu",
                p_residence->sequence_id, sal_ntoh16(rx_hdr->sequence_id));
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_residence->key));
        return FALSE;
    }

    /*if cannot forward, return TRUE because the receipt is successful*/
    tbl_interface_t *p_db_if = tbl_interface_get_interface_by_ifindex(p_residence->ifindex_port_ingress);
    tbl_ptp_port_t* p_egress_port = tbl_ptp_port_get_ptp_port((tbl_ptp_port_key_t *)(&p_db_if->key));
    if (_ptp_msg_forward_skip_port(p_ingress_port, p_egress_port))
    {
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_residence->key));//todo ,not need
        return TRUE;
    }

    tx_skb = ptp_skb_copy(rx_skb);
    if (!tx_skb)
    {
        PTP_LOG_ERR("Cannot allocate memory for forward Delay_Resp packet");
        tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_residence->key));
        return TRUE;
    }

    tx_hdr = (ptp_common_message_header_t *) tx_skb->data;

    glb_ptp_timeinterval_t correction_field;
    correction_field.s.msb = sal_ntoh32(tx_hdr->correction_field_msb);
    correction_field.s.lsb = sal_ntoh32(tx_hdr->correction_field_lsb);
    PTP_DBG_PACKET_TX("Delay_Resp correction field: %d %u %lld\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);

    /*add residence time*/
    correction_field.v += p_residence->residence_time.v;
    PTP_DBG_PACKET_TX("Delay_Resp correction field add residence time: %d %u %lld\n", correction_field.s.msb, correction_field.s.lsb, correction_field.v);
    
    tx_hdr->correction_field_msb = sal_hton32(correction_field.s.msb);
    tx_hdr->correction_field_lsb = sal_hton32(correction_field.s.lsb);

    tbl_ptp_residence_time_cache_del_ptp_residence_time_cache(&(p_residence->key));

    int32 ret = ptp_sock_send(p_egress_port, tx_skb, p_rx_ts);
    ptp_free_skb(tx_skb);
    if (PTP_E_SUCCESS != ret)
    {
        PTP_LOG_ERR("Failed to forward Delay_Resp, ret %d", ret);
    }

    return TRUE;
}

void
ptp_forward_register(tbl_ptp_port_t *p_port)
{
    sal_memset(p_port->msg_proc_ops.forward_proc, 0x00, sizeof(ptp_msg_forward_proc_t) * PTP_MSG_TYPE_MAX);
    
    if (ptp_ops_is_tc())
    {
        p_port->msg_proc_ops.forward_proc[PTP_MSG_TYPE_ANNOUNCE] = (ptp_msg_forward_proc_t)_ptp_msg_forward_announce;
        p_port->msg_proc_ops.forward_proc[PTP_MSG_TYPE_SYNC] = (ptp_msg_forward_proc_t)_ptp_msg_forward_sync;
        p_port->msg_proc_ops.forward_proc[PTP_MSG_TYPE_FOLLOW_UP] = (ptp_msg_forward_proc_t)_ptp_msg_forward_follow_up;
        if (PTP_DELAY_MECHANISM_E2E == p_port->port_ds.delay_mechanism)
        {
            p_port->msg_proc_ops.forward_proc[PTP_MSG_TYPE_DELAY_REQ] = (ptp_msg_forward_proc_t)_ptp_msg_forward_delay_req;
            p_port->msg_proc_ops.forward_proc[PTP_MSG_TYPE_DELAY_RESP] = (ptp_msg_forward_proc_t)_ptp_msg_forward_delay_resp;
        }
    }

    return;
}
