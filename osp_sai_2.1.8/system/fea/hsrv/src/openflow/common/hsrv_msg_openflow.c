

#include "hsrv_inc.h"
#include "hsrv_openflow.h"

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-11 */
#include "glb_g8131_define.h"
/* liwh end */

#include "fei_api_openflow.h"
#include "glb_tp_oam.h"

static glb_openflow_group_t g_tmp_openflow_group;
static uint32 g_copied_buckets;

uint32 g_hsrv_openflow_cvlan_tpid = 0;
uint32 g_hsrv_openflow_svlan_tpid = 0;

extern int32 hsrv_openflow_nexthop_offset_release(hsrv_of_nh_offset_t* p_nh_offset);
extern void ctc_sai_call_cmdel_dma();

int32
hsrv_packet_event_send_by_packet_out(glb_openflow_packet_out_param_t* p_packet_out, void* pkt)
{
    uint32 msg_len = 0;
    uint32 nhid = 0;
    uint32 is_exist = 0;
    int32 ret = 0;
    ctc_pkt_tx_t    pkt_tx;
    ctc_pkt_skb_t   *p_skb = NULL;
    ctc_nh_info_t nh_info;
    uint16 gport = 0;
    glb_openflow_tunnel_port_type_t type = GLB_OPENFLOW_TUNNEL_TYPE_MAX;
    hsrv_of_nh_offset_t  nh_offset;
    

    if(p_packet_out->type == GLB_OPENFLOW_PACKET_OUT_TYPE_TUNNEL)
    {
        ret = hsrv_openflow_tunnel_packet_out_get_tunnel_nhid(p_packet_out, &nhid, &is_exist, &gport);
        ret += hsrv_openflow_tunnel_pacet_out_get_tunnel_type(p_packet_out, &type);
        if(ret)
        {
            return ret;
        }
        msg_len = p_packet_out->packet_len;
        sal_memset(&pkt_tx, 0, sizeof(ctc_pkt_tx_t));
        p_skb = &(pkt_tx.skb);
        ctc_packet_skb_init(p_skb);

        ctc_packet_skb_put(p_skb, msg_len);
        sal_memcpy(p_skb->data, pkt, msg_len);
        pkt_tx.tx_info.flags = CTC_PKT_FLAG_NH_OFFSET_VALID;
        if(GLB_OPENFLOW_TUNNEL_TYPE_L2GRE != type)
        {
            pkt_tx.tx_info.flags |= CTC_PKT_FLAG_NH_OFFSET_IS_8W;
        }
        pkt_tx.tx_info.dest_gport = gport;
        ctc_nh_get_nh_info(nhid, &nh_info);
        pkt_tx.tx_info.nh_offset = nh_info.dsnh_offset[0];

        pkt_tx.mode = CTC_PKT_MODE_DMA;
        pkt_tx.tx_info.is_critical = TRUE;
        pkt_tx.tx_info.ttl = HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TTL;
        ctc_packet_tx(&pkt_tx);
        if(!is_exist)
        {
            ret += hsrv_openflow_tunnel_packet_out_del_tunnel_nhid(nhid);
            sal_memset(&nh_offset,0x0,sizeof(hsrv_of_nh_offset_t));
            if (0 < p_packet_out->tunnel_id)
            {
                ret +=hsrv_openflow_tunnel_release_fid(p_packet_out->tunnel_id);
                nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID;
            }
            else
            {
                nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL;
            }
            nh_offset.nhid = nhid;
            ret +=hsrv_openflow_nexthop_offset_release(&nh_offset);
        }
        return ret;
    }
    if(p_packet_out->type == GLB_OPENFLOW_PACKET_OUT_TYPE_INPUT)
    {
        msg_len = p_packet_out->packet_len;
        sal_memset(&pkt_tx, 0, sizeof(ctc_pkt_tx_t));
        p_skb = &(pkt_tx.skb);
        ctc_packet_skb_init(p_skb);

        ctc_packet_skb_put(p_skb, msg_len);
        sal_memcpy(p_skb->data, pkt, msg_len);

        pkt_tx.tx_info.flags = CTC_PKT_FLAG_INGRESS_MODE;
        hsrv_ifdb_get_portid(p_packet_out->ifindex, &gport);
        pkt_tx.tx_info.dest_gport = gport;
        
        /* add by hansf for USW: USW sdk will check p_pkt_tx->tx_info.ttl > 1, 
            now we set ttl=64, but real packet send out ttl is packet's ttl  */
        #ifdef USW
            pkt_tx.tx_info.ttl = 64;
        #endif
        pkt_tx.mode = CTC_PKT_MODE_DMA;
        pkt_tx.tx_info.is_critical = TRUE;
        ctc_packet_tx(&pkt_tx);
        return HSRV_E_NONE;
    }
    if(p_packet_out->type == GLB_OPENFLOW_PACKET_OUT_TYPE_NORMAL)
    {
        msg_len = p_packet_out->packet_len;
        sal_memset(&pkt_tx, 0, sizeof(ctc_pkt_tx_t));
        p_skb = &(pkt_tx.skb);
        ctc_packet_skb_init(p_skb);

        ctc_packet_skb_put(p_skb, msg_len);
        sal_memcpy(p_skb->data, pkt, msg_len);

        pkt_tx.tx_info.flags = CTC_PKT_FLAG_NH_OFFSET_BYPASS;
        hsrv_ifdb_get_portid(p_packet_out->ifindex, &gport);
        pkt_tx.tx_info.dest_gport = gport;

        pkt_tx.mode = CTC_PKT_MODE_DMA;
        pkt_tx.tx_info.is_critical = TRUE;
        /* add by hansf for USW: USW sdk will check p_pkt_tx->tx_info.ttl > 1, 
            now we set ttl=64, but real packet send out ttl is packet's ttl  */
        #ifdef USW
            pkt_tx.tx_info.ttl = 64;
        #endif
        ctc_packet_tx(&pkt_tx);
        /* added by hansf  for uml, 2019/01/24 */
        ctc_sai_call_cmdel_dma();
        return HSRV_E_NONE;
    }

    return HSRV_E_NONE;
}

static int32 
_hsrv_openflow_post_sys_startup(void* p_msg_data)
{
    fei_openflow_post_sys_startup_t* pst_req;
    pst_req  = (fei_openflow_post_sys_startup_t*) p_msg_data;
    return fei_api_openflow_post_sys_startup(pst_req);
}

static int32 
_hsrv_openflow_do_barrier(void* p_msg_data)
{
    int32 rc = 0;
    
    return HSRV2OPENFLOW_MSG_SEND_NOTIFY_NORMAL(FEI_OPENFLOW_DO_BARRIER, &rc, sizeof(rc));    
}

int g_test = 0;
static int32
_hsrv_openflow_port_do_barrier_by_ovs(void* p_msg_data)
{
    fei_openflow_port_do_barrier_t* pst_req = NULL;
    tbl_interface_t* p_interface = NULL;
    int32 is_exist = 0;
    
    pst_req = (fei_openflow_port_do_barrier_t*) p_msg_data;

    p_interface = tbl_interface_get_interface_by_ifindex(pst_req->ifindex);
    if(NULL != p_interface)
    {
        is_exist = 1;
        if(g_test == 0)
        {
           g_test ++;
           is_exist = 0;
           
        }
    }

    return HSRV2OPENFLOW_MSG_SEND_NOTIFY_NORMAL(FEI_OPENFLOW_DO_PORT_BARRIER_BY_OVS, &is_exist, sizeof(is_exist));
}

static int32
_hsrv_openflow_port_do_barrier_by_switch(void* p_msg_data)
{
    fei_openflow_port_do_barrier_t* pst_req = NULL;
    tbl_interface_t* p_interface = NULL;
    int32 is_exist = 0;
    
    pst_req = (fei_openflow_port_do_barrier_t*) p_msg_data;

    p_interface = tbl_interface_get_interface_by_ifindex(pst_req->ifindex);
    if(NULL != p_interface)
    {
        is_exist = 1;
    }

    return HSRV2SWITCH_MSG_SEND_RESP(FEI_OPENFLOW_DO_PORT_BARRIER_BY_SWITCH, &is_exist, sizeof(is_exist));
}

static int32 
_hsrv_openflow_port_enable(void* p_msg_data)
{
    fei_openflow_port_enable_t* pst_req;
    pst_req  = (fei_openflow_port_enable_t*) p_msg_data;
    
    return fei_api_openflow_port_enable(pst_req);
}

static int32
_hsrv_openflow_port_operation_agg(void* p_msg_data)
{
    fei_openflow_port_operation_agg_t* pst_req;
    pst_req  = (fei_openflow_port_operation_agg_t*) p_msg_data;

    return fei_api_openflow_port_operation_agg(pst_req);
}

static int32
_hsrv_openflow_port_set_protected_vlan(void* p_msg_data)
{
    fei_openflow_port_set_protected_vlan_t* pst_req;
    pst_req  = (fei_openflow_port_set_protected_vlan_t*) p_msg_data;

    return fei_api_openflow_port_set_protected_vlan(pst_req);
}

static int32 
_hsrv_openflow_port_obey_vlan_filter(void* p_msg_data)
{
    fei_openflow_port_ignore_vlan_filter_t* pst_req;
    pst_req  = (fei_openflow_port_ignore_vlan_filter_t*) p_msg_data;
    
    return fei_api_openflow_port_ignore_vlan_filter(pst_req);
}

static int32 
_hsrv_openflow_port_tunnel_mpls_disable(void* p_msg_data)
{
    fei_openflow_port_tunnel_mpls_disable_t* pst_req;
    pst_req  = (fei_openflow_port_tunnel_mpls_disable_t*) p_msg_data;
    
    return fei_api_openflow_port_tunnel_mpls_disable(pst_req);
}

static int32 
_hsrv_openflow_port_native_vlan_config(void* p_msg_data)
{
    fei_openflow_port_native_vlan_config_t* pst_req;
    pst_req  = (fei_openflow_port_native_vlan_config_t*) p_msg_data;
    
    return fei_api_openflow_port_native_vlan_enable(pst_req);
}


static int32 
_hsrv_openflow_port_set_port_act(void* p_msg_data)
{
    fei_openflow_port_pkt_action_t* pst_req;
    pst_req  = (fei_openflow_port_pkt_action_t*) p_msg_data;
    
    return fei_api_openflow_port_set_port_action(pst_req);
}

static int32 
_hsrv_openflow_port_notify_link(void* p_msg_data)
{
    fei_openflow_port_notify_link_t* pst_req;
    pst_req  = (fei_openflow_port_notify_link_t*) p_msg_data;
    
    return fei_api_openflow_port_notify_link(pst_req);
}

static int32 
_hsrv_openflow_bond_create(void *p_msg_data)
{
    fei_openflow_bond_create_t* pst_req;
    pst_req  = (fei_openflow_bond_create_t*) p_msg_data;

    return fei_api_openflow_bond_create(pst_req);
}

static int32 
_hsrv_openflow_bond_update_slave(void *p_msg_data)
{
    fei_openflow_bond_update_slave_t* pst_req;
    pst_req  = (fei_openflow_bond_update_slave_t*) p_msg_data;

    return fei_api_openflow_bond_update_slave(pst_req);
}

static int32 
_hsrv_openflow_bond_attach_slave(void *p_msg_data)
{
    fei_openflow_bond_attach_slave_t *pst_req;
    pst_req  = (fei_openflow_bond_attach_slave_t *) p_msg_data;

    return fei_api_openflow_bond_attach_slave(pst_req);
}

static int32 
_hsrv_openflow_bond_set_mac(void *p_msg_data)
{
    fei_openflow_bond_set_mac_t *pst_req;
    pst_req  = (fei_openflow_bond_set_mac_t *) p_msg_data;

    return fei_api_openflow_bond_set_mac(pst_req);
}

static int32 
_hsrv_openflow_port_set_config(void *p_msg_data)
{
    fei_openflow_port_set_config_t* pst_req;
    pst_req  = (fei_openflow_port_set_config_t*) p_msg_data;
    
    return fei_api_openflow_port_set_config(pst_req);
}

static int32 
_hsrv_openflow_port_add_to_bridge(void *p_msg_data)
{
    fei_openflow_port_add_to_bridge_req_t* pst_req;
    pst_req  = (fei_openflow_port_add_to_bridge_req_t*) p_msg_data;
    
    return fei_api_openflow_port_add_to_bridge(pst_req);
}

static int32 
_hsrv_openflow_add_flow(void* p_msg_data)
{
    fei_openflow_add_flow_req_t* p_msg_req;
    fei_openflow_add_flow_req_t req;
    int32 rc = HSRV_E_NONE;
    int32 action_len;
    int32 req_len;
    
    req_len = sizeof(fei_openflow_add_flow_req_t);
    p_msg_req = (fei_openflow_add_flow_req_t*) p_msg_data;
    
    action_len = sizeof(glb_openflow_flow_action_t) *
        p_msg_req->flow.actlist.action_count;
    
    sal_memcpy(&req, p_msg_req, req_len);

    if (action_len)
    {
        req.flow.actlist.actlist = HSRV_OPENFLOW_ACT_MALLOC(action_len);
        if (!req.flow.actlist.actlist)
        {
            return HSRV_E_NO_MEMORY;
        }
        
        sal_memcpy(req.flow.actlist.actlist, p_msg_req + 1, action_len);
    }
    
    rc = fei_api_openflow_add_flow(&req);
    /* req->flow.actlist.actlist is used in hsrv, so
       We do not free it here */
    
    return rc;
}

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
static int32 
_hsrv_openflow_add_tpoam(void* p_msg_data)
{
    int32                                                 rc = HSRV_E_NONE;
    fei_openflow_add_tpoam_req_t*           p_msg_req;

    p_msg_req = (fei_openflow_add_tpoam_req_t*) p_msg_data;
    rc = fei_api_openflow_add_tpoam(p_msg_req);

    return rc;
}

static int32 
_hsrv_openflow_del_tpoam(void* p_msg_data)
{
    int32                                                 rc = HSRV_E_NONE;
    fei_openflow_del_tpoam_req_t*           p_msg_req;

    p_msg_req = (fei_openflow_del_tpoam_req_t*) p_msg_data;
    rc = fei_api_openflow_del_tpoam(p_msg_req);

    return rc;
}

static int32 
_hsrv_openflow_update_tpoam(void* p_msg_data)
{
    int32                                                 rc = HSRV_E_NONE;
    fei_openflow_update_tpoam_req_t*           p_msg_req;

    p_msg_req = (fei_openflow_update_tpoam_req_t*) p_msg_data;
    rc = fei_api_openflow_update_tpoam(p_msg_req);

    return rc;
}

static int32 
_hsrv_openflow_g8131_fast_aps_en(void* p_msg_data)
{
    int32                                                 rc = HSRV_E_NONE;
    fei_g8131_mpls_fast_aps_req_t*           p_msg_req;

    p_msg_req = (fei_g8131_mpls_fast_aps_req_t*) p_msg_data;
    rc = fei_api_openflow_g8131_fast_aps_en(p_msg_req);

    return rc;
}

static int32 
_hsrv_openflow_g8131_protection_en(void* p_msg_data)
{
    int32                                                 rc = HSRV_E_NONE;
    fei_g8131_mpls_protection_en_req_t*           p_msg_req;

    p_msg_req = (fei_g8131_mpls_protection_en_req_t*) p_msg_data;
    rc = fei_api_openflow_g8131_protection_en(p_msg_req);

    return rc;
}

static int32 
_hsrv_openflow_set_pw_mode(void* p_msg_data)
{
    int32                                 rc = HSRV_E_NONE;
    uint32                               *p_msg_req;

    p_msg_req = (uint32*) p_msg_data;
    rc = fei_api_set_pw_mode(p_msg_req);

    return rc;
}
/* liwh end */

static int32
_hsrv_openflow_packet_out(void* p_msg_data)
{
    fei_openflow_packet_out_req_t* p_msg_req;
    int32 rc = HSRV_E_NONE;
    void* pkt = NULL;
    
    p_msg_req = (fei_openflow_packet_out_req_t*)p_msg_data;

    pkt = HSRV_OPENFLOW_ACT_MALLOC(p_msg_req->packet_out.packet_len);
    if(pkt)
    {
        sal_memcpy(pkt, p_msg_req + 1, p_msg_req->packet_out.packet_len);
    }
    else
    {
        return -1;
    }

    hsrv_packet_event_send_by_packet_out(&p_msg_req->packet_out, pkt);

    HSRV_OPENFLOW_ACT_FREE(pkt);
    return rc;
}

static int32 
_hsrv_openflow_mod_flow(void* p_msg_data)
{
    fei_openflow_mod_flow_req_t* p_msg_req;
    fei_openflow_mod_flow_req_t req;
    int32 rc = HSRV_E_NONE;
    int32 action_len;
    int32 req_len;
    
    req_len = sizeof(fei_openflow_mod_flow_req_t);
    p_msg_req = (fei_openflow_mod_flow_req_t*) p_msg_data;
    
    action_len = sizeof(glb_openflow_flow_action_t) *
        p_msg_req->flow.actlist.action_count;
    
    sal_memcpy(&req, p_msg_req, req_len);

    if (action_len)
    {
        req.flow.actlist.actlist = HSRV_OPENFLOW_ACT_MALLOC(action_len);
        if (!req.flow.actlist.actlist)
        {
            return HSRV_E_NO_MEMORY;
        }
        
        sal_memcpy(req.flow.actlist.actlist, p_msg_req + 1, action_len);
    }
    rc = fei_api_openflow_mod_flow(&req);
    /* req->flow.actlist.actlist is used in hsrv, so
       We do not free it here */
    
    return rc;
}

static int32 
_hsrv_openflow_del_flow(void* p_msg_data)
{
    fei_openflow_del_flow_req_t* pst_req;
    pst_req  = (fei_openflow_del_flow_req_t*) p_msg_data;
    
    return fei_api_openflow_del_flow(pst_req);
}

static int32 
_hsrv_openflow_reset_flow(void* p_msg_data)
{
    fei_openflow_reset_flow_req_t* pst_req;
    pst_req  = (fei_openflow_reset_flow_req_t*) p_msg_data;
    
    return fei_api_openflow_reset_flow(pst_req);
}

/* Added by weizj for tunnel bug 33724 */
/* Modifed by weizj for bug 35135 */
static int32 
_hsrv_openflow_set_drop_ingress(void* p_msg_data)
{
    fei_openflow_set_drop_ingress* pst_req;
    pst_req  = (fei_openflow_set_drop_ingress*) p_msg_data;

    if (pst_req->is_group)
    {
        return fei_api_openflow_set_group_drop_ingress(pst_req);
    }

    return fei_api_openflow_set_flow_drop_ingress(pst_req);
}

static int32 
_hsrv_openflow_flow_update_all_action_list(void* p_msg_data)
{
    fei_openflow_update_all_action_list_req_t* pst_req;
    pst_req  = (fei_openflow_update_all_action_list_req_t*) p_msg_data;
    
    return fei_api_openflow_flow_update_all_action_list(pst_req);
}

/* Added by weizj for udf */
static int32 
_hsrv_openflow_udf_enable(void* p_msg_data)
{
    fei_openflow_set_udf_parser* pst_req;
    pst_req  = (fei_openflow_set_udf_parser*) p_msg_data;
    
    return fei_api_openflow_udf_enable(pst_req);
}

static int32 
_hsrv_openflow_e2e_enable(void* p_msg_data)
{
    fei_openflow_set_e2e_parser* pst_req;
    pst_req  = (fei_openflow_set_e2e_parser*) p_msg_data;

    Hsrv2AgtMsg_t hsrv_msg;
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetE2e;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetE2e.enable = pst_req->enable;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return 0;
}

static int32 
_hsrv_openflow_set_stpid(void* p_msg_data)
{
    fei_openflow_set_stpid* pst_req;
    pst_req  = (fei_openflow_set_stpid*) p_msg_data;

    Hsrv2AgtMsg_t hsrv_msg;
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetStpid;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetStpid.stpid = pst_req->stpid;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    g_hsrv_openflow_svlan_tpid = pst_req->stpid;

    return 0;
}

void
_hsrv_openflow_get_svlan_tpid(uint16* stpid)
{
    *stpid = g_hsrv_openflow_svlan_tpid;
}

static int32 
_hsrv_openflow_set_ctpid(void* p_msg_data)
{
    fei_openflow_set_ctpid* pst_req;
    pst_req  = (fei_openflow_set_ctpid*) p_msg_data;

    Hsrv2AgtMsg_t hsrv_msg;
    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetCtpid;
    hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetCtpid.ctpid = pst_req->ctpid;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    g_hsrv_openflow_cvlan_tpid = pst_req->ctpid;

    return 0;
}

void
_hsrv_openflow_get_cvlan_tpid(uint16* ctpid)
{
    *ctpid = g_hsrv_openflow_cvlan_tpid;
}

static int32 
_hsrv_openflow_set_udf_parser(void* p_msg_data)
{
    fei_openflow_set_udf_parser* pst_req;
    pst_req  = (fei_openflow_set_udf_parser*) p_msg_data;
    
    return fei_api_openflow_set_udf_parser(pst_req);
}
/* End by weizj for udf */

/* group */
static int32
_hsrv_openflow_add_group(void* p_msg_data)
{
    uint32 bucket_size = 0;
    uint32 action_size = 0;
    uint32 bucket_idx = 0;
    uint32 buckets_len_once = 0;
    fei_openflow_add_group_req_t* pst_req = NULL;
    glb_openflow_bucket_t* p_bucket = NULL;
    void* p_tmp = NULL;
    pst_req  = (fei_openflow_add_group_req_t*) p_msg_data;

    switch(pst_req->partial_type)
    {
        case FEI_OPENFLOW_GROUP_PARTIAL_TYPE_HEAD:
            /* !!! convert glb_oepnflow_group to glb_openflow_partial_group_head_t by statically cast.
             * It's not needed because we mee copy all port. 
             * memset(&g_tmp_openflow_group, 0, sizeof(g_tmp_openflow_group));
             */
            sal_memcpy(&g_tmp_openflow_group, &pst_req->u.head.group, sizeof(glb_openflow_group_t));
            g_copied_buckets = 0;

            bucket_size = pst_req->u.head.group.n_buckets * sizeof(glb_openflow_bucket_t);
            g_tmp_openflow_group.bucket_list = HSRV_OPENFLOW_GROUP_MALLOC(bucket_size);
            sal_memset(g_tmp_openflow_group.bucket_list, 0, bucket_size);
            break;
        case FEI_OPENFLOW_GROUP_PARTIAL_TYPE_BUCKET:
            /* When sending fei message, the bucket count is set in the head, here we can safely
             * copy the whole buckets to simplify the implementation.
             */
            p_tmp = (void*)(pst_req + 1);
            buckets_len_once = pst_req->u.bucket.buckets_len;
            for (bucket_idx = 0; bucket_idx < buckets_len_once; bucket_idx ++)
            {
                p_bucket = &g_tmp_openflow_group.bucket_list[g_copied_buckets + bucket_idx];
                sal_memcpy(p_bucket, p_tmp, sizeof(glb_openflow_bucket_t));
                p_tmp += sizeof(glb_openflow_bucket_t);

                if (p_bucket->n_actions)
                {
                    action_size = p_bucket->n_actions * sizeof(glb_openflow_flow_action_t);
                    p_bucket->actlist = HSRV_OPENFLOW_GROUP_MALLOC(action_size);
                    sal_memcpy(p_bucket->actlist, p_tmp, action_size);
                    p_tmp += p_bucket->n_actions * sizeof(glb_openflow_flow_action_t);
                }
                
            }
            g_copied_buckets += pst_req->u.bucket.buckets_len;
            break;
        case FEI_OPENFLOW_GROUP_PARTIAL_TYPE_TAIL:
            break;
        default:
            break;
    }
    
    if (pst_req->partial_type != FEI_OPENFLOW_GROUP_PARTIAL_TYPE_TAIL)
    {
        return HSRV_E_NONE;
    }
    
    fei_api_openflow_add_group(&g_tmp_openflow_group);

    /* release bucket actions */
    for (bucket_idx = 0; bucket_idx < g_tmp_openflow_group.n_buckets; bucket_idx ++)
    {
        p_bucket = &g_tmp_openflow_group.bucket_list[bucket_idx];
        if (p_bucket->actlist)
        {
            HSRV_OPENFLOW_GROUP_FREE(p_bucket->actlist);
            p_bucket->actlist = NULL;
        }
    }

    /* release bucket */
    if (g_tmp_openflow_group.bucket_list)
    {
        HSRV_OPENFLOW_GROUP_FREE(g_tmp_openflow_group.bucket_list);
    }
    return HSRV_E_NONE;
}

static int32 
_hsrv_openflow_del_group(void* p_msg_data)
{
    fei_openflow_del_group_req_t* pst_req;
    pst_req  = (fei_openflow_del_group_req_t*) p_msg_data;

    return fei_api_openflow_del_group(pst_req);
}

/* meter, Added by weizj */
static int32 
_hsrv_openflow_add_meter(void* p_msg_data)
{
    fei_openflow_add_meter_req_t* pst_req;
    pst_req  = (fei_openflow_add_meter_req_t*) p_msg_data;

    return fei_api_openflow_add_meter(pst_req);
}

static int32
_hsrv_openflow_mod_meter(void* p_msg_data)
{
    fei_openflow_mod_meter_req_t* pst_req;
    pst_req  = (fei_openflow_mod_meter_req_t*) p_msg_data;

    return fei_api_openflow_mod_meter(pst_req);
}

static int32
_hsrv_openflow_del_meter(void* p_msg_data)
{
    fei_openflow_del_meter_req_t* pst_req;
    pst_req  = (fei_openflow_del_meter_req_t*) p_msg_data;

    return fei_api_openflow_del_meter(pst_req);
}

static int32
_hsrv_openflow_mod_meter_ipg_status(void* p_msg_data)
{
    fei_openflow_mod_meter_ipg_req_t* pst_req;
    pst_req  = (fei_openflow_mod_meter_ipg_req_t*) p_msg_data;

    return fei_api_openflow_mod_meter_ipg_status(pst_req);
}
/* End by weizj for meter */

/* meter, Added by weizj */
static int32 
_hsrv_openflow_add_queue_shape(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_add_queue_shape_req_t* pst_req;
    pst_req  = (fei_openflow_add_queue_shape_req_t*) p_msg_data;

    return fei_api_openflow_add_queue_shape(pst_req);
#endif
    return 0;
}

/* inband, Added by weizj */
static int32
_hsrv_openflow_inband_set_config(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_set_config_req_t* pst_req;
    
    pst_req = (fei_openflow_inband_set_config_req_t*) p_msg_data;

    return fei_api_openflow_inband_set_config(pst_req);
#endif
    return 0;
}

int32
_hsrv_openflow_inband_create_vif(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_vif_req_t* pst_req;

    pst_req = (fei_openflow_inband_vif_req_t*) p_msg_data;

    return fei_api_openflow_inband_create_vif(pst_req);
#endif
    return 0;
}

int32
_hsrv_openflow_inband_destroy_vif(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_vif_req_t* pst_req;

    pst_req = (fei_openflow_inband_vif_req_t*) p_msg_data;

    return fei_api_openflow_inband_destroy_vif(pst_req);
#endif 
    return 0;
}
/* End by weizj for inband */

int32
_hsrv_openflow_inband_set_uplink_port(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_uplink_port_req_t* pst_req;

    pst_req = (fei_openflow_inband_uplink_port_req_t*) p_msg_data;

    return fei_api_openflow_inband_set_uplink_port(pst_req);
#endif
    return 0;
}

int32
_hsrv_openflow_inband_unset_uplink_port(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_unset_uplink_port_req_t* pst_req;

    pst_req = (fei_openflow_inband_unset_uplink_port_req_t*) p_msg_data;

    return fei_api_openflow_inband_unset_uplink_port(pst_req);
#endif
    return 0;
}

int32
_hsrv_openflow_inband_set_downlink_port(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_downlink_port_req_t* pst_req;

    pst_req = (fei_openflow_inband_downlink_port_req_t*) p_msg_data;

    return fei_api_openflow_inband_set_downlink_port(pst_req);
#endif
    return 0;
}

int32
_hsrv_openflow_inband_unset_downlink_port(void* p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_inband_unset_downlink_port_req_t* pst_req;

    pst_req = (fei_openflow_inband_unset_downlink_port_req_t*) p_msg_data;

    return fei_api_openflow_inband_unset_downlink_port(pst_req);
#endif
    return 0;
}

/* tunnel, Added by weizj */
static int32 
_hsrv_openflow_tunnel_port_decap_en(void* p_msg_data)
{
    fei_openflow_port_decap_req_t* pst_req;
    pst_req  = (fei_openflow_port_decap_req_t*) p_msg_data;
    
    return fei_api_openflow_tunnel_port_decap_en(pst_req);
}

static int32
_hsrv_openflow_tunnel_mod_decap_mode(void* p_msg_data)
{
    fei_openflow_tunnel_decap_mode_req_t* pst_req;
    pst_req  = (fei_openflow_tunnel_decap_mode_req_t*) p_msg_data;

    return fei_api_openflow_tunnel_update_decap_mode(pst_req);
}

/* Added by weizj for tunnel bug 33958 */
static int32
_hsrv_openflow_tunnel_mod_vtep_ip(void* p_msg_data)
{
    fei_openflow_vtep_ip_req_t* pst_req;
    pst_req  = (fei_openflow_vtep_ip_req_t*) p_msg_data;

    return fei_api_openflow_tunnel_update_vtep_ip(pst_req);
}

static int32
_hsrv_openflow_tunnel_add_port(void* p_msg_data)
{
    fei_openflow_add_tunnel_req_t* pst_req;
    pst_req  = (fei_openflow_add_tunnel_req_t*) p_msg_data;

    return fei_api_openflow_tunnel_add_port(pst_req);
}

static int32
_hsrv_openflow_tunnel_del_port(void* p_msg_data)
{
    fei_openflow_del_tunnel_req_t* pst_req;
    pst_req  = (fei_openflow_del_tunnel_req_t*) p_msg_data;

    return fei_api_openflow_tunnel_delete_port(pst_req);
}
/* End by weizj for tunnel */

/* stats */
static int32
_hsrv_openflow_get_stats(void* p_msg_data)
{
    int32 rc = HSRV_E_NONE;

    return rc;
}

static int32
_hsrv_openflow_clear_stats(void* p_msg_data)
{
    int32 rc = HSRV_E_NONE;
    fei_openflow_clear_stats_req_t* pst_req;
    pst_req  = (fei_openflow_clear_stats_req_t*) p_msg_data;

    switch (pst_req->res_type)
    {
    case FEI_OPENFLOW_STATS_RES_TYPE_FLOW:
        rc = fei_api_openflow_clear_flow_stats(pst_req);
        break;

    case FEI_OPENFLOW_STATS_RES_TYPE_GROUP:
        rc = fei_api_openflow_clear_group_stats(pst_req);
        break;

    case FEI_OPENFLOW_STATS_RES_TYPE_METER:
        /* Added by weizj for meter */
        rc = fei_api_openflow_clear_meter_stats(pst_req);
        break;

    default:
        break;
    }
    return rc;
}

/* parser */
static int32
_hsrv_openflow_set_ecmp_hash_key(void* p_msg_data)
{
    fei_openflow_parser_set_ecmp_hash_key_req_t* pst_req;
    pst_req  = (fei_openflow_parser_set_ecmp_hash_key_req_t*) p_msg_data;

    return fei_api_openflow_set_ecmp_hash_key(pst_req);
}

static int32
_hsrv_openflow_set_linkagg_hash_key(void* p_msg_data)
{
    fei_openflow_parser_set_linkagg_hash_key_req_t* pst_req;
    pst_req  = (fei_openflow_parser_set_linkagg_hash_key_req_t*) p_msg_data;

    return fei_api_openflow_set_linkagg_hash_key(pst_req);
}

static int32
_hsrv_openflow_set_mpls_parser(void* p_msg_data)
{
    fei_openflow_parser_set_mpls_parser_req_t* pst_req;
    pst_req  = (fei_openflow_parser_set_mpls_parser_req_t*) p_msg_data;

    return fei_api_openflow_set_mpls_parser(pst_req);
}

static int32 
_hsrv_openflow_stub_create(void *p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_stub_create_t* pst_req;
    pst_req  = (fei_openflow_stub_create_t*) p_msg_data;
    
    return fei_api_openflow_stub_create(pst_req);
#endif
    return 0;
}

static int32 
_hsrv_openflow_stub_enable(void *p_msg_data)
{
#ifdef V580_OPENFLOW
    fei_openflow_stub_enable_t* pst_req;
    pst_req  = (fei_openflow_stub_enable_t*) p_msg_data;
    
    return fei_api_openflow_stub_enable(pst_req);
#endif 
    return 0;
}

#ifdef V580_OPENFLOW
static int32 
_hsrv_openflow_set_efd_tcp_only_enable(void *p_msg_data)
{

    fei_openflow_efd_set_efd_tcp_only_enable_req_t* pst_req;
    pst_req  = (fei_openflow_efd_set_efd_tcp_only_enable_req_t*) p_msg_data;
    
    return fei_api_openflow_set_efd_tcp_only_enable(pst_req);
}
#endif

#ifdef V580_OPENFLOW
static int32
_hsrv_openflow_set_efd_global_config(void *p_msg_data)
{

    fei_openflow_efd_set_efd_global_config_req_t* pst_req;
    pst_req = (fei_openflow_efd_set_efd_global_config_req_t*)p_msg_data;

    return fei_api_openflow_set_efd_global_config(pst_req);
}
#endif 
#ifdef V580_OPENFLOW
static int32
_hsrv_openflow_set_efd_flow_hash_mode(void *p_msg_data)
{

    fei_openflow_efd_set_flow_hash_mode_req_t* pst_req;
    pst_req = (fei_openflow_efd_set_flow_hash_mode_req_t*)p_msg_data;

    return fei_api_openflow_set_efd_flow_hash_mode(pst_req);
}
#endif

#ifdef V580_OPENFLOW
static int32
_hsrv_openflow_set_vxlan_nvgre_merge_enable(void *p_msg_data)
{

    fei_openflow_efd_set_vxlan_nvgre_merge_enable_req_t* pst_req;
    pst_req = (fei_openflow_efd_set_vxlan_nvgre_merge_enable_req_t*)p_msg_data;

    return fei_api_openflow_set_vxlan_nvgre_merge_enable(pst_req);
}
#endif
#ifdef V580_OPENFLOW
static int32
_hsrv_openflow_set_efd_if_enable(void *p_msg_data)
{

    fei_openflow_efd_set_if_enable_req_t* pst_req;
    pst_req = (fei_openflow_efd_set_if_enable_req_t*)p_msg_data;

    return fei_api_openflow_set_efd_if_enable(pst_req);
}
#endif
#ifdef V580_OPENFLOW
static int32
_hsrv_openflow_set_efd_if_vm_aware_enable(void *p_msg_data)
{

    fei_openflow_efd_set_if_vm_aware_enable_req_t* pst_req;
    pst_req = (fei_openflow_efd_set_if_vm_aware_enable_req_t*)p_msg_data;

    return fei_api_openflow_set_efd_if_vm_aware_enable(pst_req);
}
#endif

int32
hsrv_openflow_register_cb(void)
{
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_POST_SYS_STARTUP, _hsrv_openflow_post_sys_startup);

    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_DO_BARRIER, _hsrv_openflow_do_barrier);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_DO_PORT_BARRIER_BY_OVS, _hsrv_openflow_port_do_barrier_by_ovs);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_DO_PORT_BARRIER_BY_SWITCH, _hsrv_openflow_port_do_barrier_by_switch);

    /* port */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_ENABLE,  _hsrv_openflow_port_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_OPERATION_AGG,  _hsrv_openflow_port_operation_agg);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_SET_PROTECTED_VLAN,  _hsrv_openflow_port_set_protected_vlan);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_IGNORE_VLAN_FILTER,  _hsrv_openflow_port_obey_vlan_filter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_TUNNEL_MPLS_DISABLE,  _hsrv_openflow_port_tunnel_mpls_disable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_SET_PKT_ACTION,  _hsrv_openflow_port_set_port_act);

    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_NATIVE_VLAN_CONFIG,  _hsrv_openflow_port_native_vlan_config);

    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_NOTIFY_LINK,  _hsrv_openflow_port_notify_link);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_SET_CONFIG,  _hsrv_openflow_port_set_config);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_ADD_TO_BRIDGE,  _hsrv_openflow_port_add_to_bridge);

    /* bond */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_BOND_CREATE,  _hsrv_openflow_bond_create);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_BOND_UPDATE_STATE,  _hsrv_openflow_bond_update_slave);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_BOND_ATTACH_SLAVE,  _hsrv_openflow_bond_attach_slave);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_BOND_SET_MAC,  _hsrv_openflow_bond_set_mac);

    /* flow */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_ADD, _hsrv_openflow_add_flow);

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_ADD_TPOAM, _hsrv_openflow_add_tpoam);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_DEL_TPOAM, _hsrv_openflow_del_tpoam);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_UPDATE_TPOAM, _hsrv_openflow_update_tpoam);
    hsrv_message_set_cb_from_fei(FEI_G8131_MPLS_FAST_APS_EN, _hsrv_openflow_g8131_fast_aps_en);
    hsrv_message_set_cb_from_fei(FEI_G8131_MPLS_PROTECTION_EN, _hsrv_openflow_g8131_protection_en);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_SET_PW_MODE, _hsrv_openflow_set_pw_mode);
    /* liwh end */
        
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_MOD, _hsrv_openflow_mod_flow);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_DEL, _hsrv_openflow_del_flow);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_RESET, _hsrv_openflow_reset_flow);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_UPDATE_ALL_ACTION_LIST,  _hsrv_openflow_flow_update_all_action_list);
    /* Added by weizj for tunnel bug 33724 */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_DROP_INGRESS, _hsrv_openflow_set_drop_ingress);
    /* Added by weizj for udf */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_UDF_ENABLE, _hsrv_openflow_udf_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_E2E_ENABLE, _hsrv_openflow_e2e_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_SET_STPID, _hsrv_openflow_set_stpid);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_SET_CTPID, _hsrv_openflow_set_ctpid);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_SET_UDF_PARSER, _hsrv_openflow_set_udf_parser);

    /* group */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_GROUP_ADD, _hsrv_openflow_add_group);
    //hsrv_message_set_cb_from_fei(FEI_OPENFLOW_GROUP_MOD, _hsrv_openflow_mod_group);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_GROUP_DEL, _hsrv_openflow_del_group);

    /* meter, Added by weizj */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_ADD, _hsrv_openflow_add_meter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_MOD, _hsrv_openflow_mod_meter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_DEL, _hsrv_openflow_del_meter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_MOD_IPG, _hsrv_openflow_mod_meter_ipg_status);

    /*QOS*/
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_QUEUE_SHAPE_ADD, _hsrv_openflow_add_queue_shape);

    /* inband, Added by weizj */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_VIF_ADD, _hsrv_openflow_inband_create_vif);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_VIF_DEL, _hsrv_openflow_inband_destroy_vif);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_SET_CONFIG, _hsrv_openflow_inband_set_config);
#ifdef V580_OPENFLOW
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_UPLINK_PORT, _hsrv_openflow_inband_set_uplink_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_DOWNLINK_PORT, _hsrv_openflow_inband_set_downlink_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_UNSET_UPLINK_PORT, _hsrv_openflow_inband_unset_uplink_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_INBAND_UNSET_DOWNLINK_PORT, _hsrv_openflow_inband_unset_downlink_port);
#endif    

    /* tunnel, Added by weizj */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_ADD, _hsrv_openflow_tunnel_add_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_DEL, _hsrv_openflow_tunnel_del_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_PORT_DECAP_EN, _hsrv_openflow_tunnel_port_decap_en);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_DECAP_MODE_MOD, _hsrv_openflow_tunnel_mod_decap_mode);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_VTEP_IP_MOD, _hsrv_openflow_tunnel_mod_vtep_ip);

    /* stats */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_STATS_GET, _hsrv_openflow_get_stats);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_STATS_CLEAR, _hsrv_openflow_clear_stats);

    /* parser */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PARSER_SET_ECMP_HASH_KEY, _hsrv_openflow_set_ecmp_hash_key);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PARSER_SET_LINKAGG_HASH_KEY, _hsrv_openflow_set_linkagg_hash_key);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PARSER_SET_MPLS_PARSER, _hsrv_openflow_set_mpls_parser);

    /* stub */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_STUB_CREATE,  _hsrv_openflow_stub_create);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_STUB_ENABLE,  _hsrv_openflow_stub_enable);

    /*EFD*/
#ifdef V580_OPENFLOW
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_EFD_TCP_ONLY_ENABLE,  _hsrv_openflow_set_efd_tcp_only_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_EFD_GLOBAL_CONFIG,  _hsrv_openflow_set_efd_global_config);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_EFD_FLOW_HASH_MODE,  _hsrv_openflow_set_efd_flow_hash_mode);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_EFD_IF_ENABLE,  _hsrv_openflow_set_efd_if_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_EFD_IFVM_AWARE_ENABLE,  _hsrv_openflow_set_efd_if_vm_aware_enable);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_EFD_SET_VXLAN_NVGRE_MERGE_ENABLE,  _hsrv_openflow_set_vxlan_nvgre_merge_enable);
#endif
    /**/
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PACKET_OUT, _hsrv_openflow_packet_out);


    hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgTpoamEvent, hsrv_tpoam_rx_hagt_event);
    
    //hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgTpoamLmDualReq, _hsrv_tpoam_rx_hagt_lm_dual_counters);
    hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgEthOamCfmLmDualReq, hsrv_tpoam_rx_hagt_lm_dual_counters);
    
    //hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgTpoamLmSDReq, _hsrv_tpoam_rx_hagt_lm_sd);
    hsrv_message_set_cb_from_hagt(opHagt2Srv_PR_hagt2SrvMsgEthOamCfmLmSDReq, hsrv_tpoam_rx_hagt_lm_sd);
    
    return HSRV_E_NONE;
}

