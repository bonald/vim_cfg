

#include "hsrv_inc.h"
#include "hsrv_openflow.h"

#include "glb_g8131_define.h"
#include "fei_api_openflow.h"


static glb_openflow_group_t g_tmp_openflow_group;
static uint32 g_copied_buckets;

int32
hsrv_packet_event_send_by_packet_out(glb_openflow_packet_out_param_t* p_packet_out, void* pkt)
{
    uint32 msg_len = 0;
    uint32 nhid = 0;
    int32 ret = 0;
    ctc_pkt_tx_t    pkt_tx;
    ctc_pkt_skb_t   *p_skb = NULL;
    ctc_nh_info_t nh_info;
    uint16 gport = 0;
    glb_openflow_tunnel_port_type_t type = GLB_OPENFLOW_TUNNEL_TYPE_MAX;

    if(p_packet_out->type == GLB_OPENFLOW_PACKET_OUT_TYPE_TUNNEL)
    {
        ret = hsrv_openflow_group_get_tunnel_info(p_packet_out, &nhid, &gport);
        type = GLB_OPENFLOW_TUNNEL_TYPE_VXLAN;
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
        ctc_packet_tx(&pkt_tx);

        ret = hsrv_openflow_group_del_tunnel_info(nhid, p_packet_out->tunnel_id);
        
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
        ctc_packet_tx(&pkt_tx);
        return HSRV_E_NONE;
    }


    return HSRV_E_NONE;
}

static int32 
_hsrv_openflow_do_barrier(void* p_msg_data)
{
    int32 rc = 0;
    
    return HSRV2OPENFLOW_MSG_SEND_NOTIFY_NORMAL(FEI_OPENFLOW_DO_BARRIER, &rc, sizeof(rc));    
}

static int32 
_hsrv_openflow_port_enable(void* p_msg_data)
{
    fei_openflow_port_enable_t* pst_req;
    pst_req  = (fei_openflow_port_enable_t*) p_msg_data;
    
    return fei_api_openflow_port_enable(pst_req);
}

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
    /* not like hybrid
    we should free req->flow.actlist here because we dont need it in hsrv */
    HSRV_OPENFLOW_ACT_FREE(req.flow.actlist.actlist);
    
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
    /* not like hybrid
    we should free req->flow.actlist here because we dont need it in hsrv */
    HSRV_OPENFLOW_ACT_FREE(req.flow.actlist.actlist);
    
    return rc;
}

static int32 
_hsrv_openflow_del_flow(void* p_msg_data)
{
    fei_openflow_del_flow_req_t* pst_req;
    pst_req  = (fei_openflow_del_flow_req_t*) p_msg_data;
    
    return fei_api_openflow_del_flow(pst_req);
}


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


int32
hsrv_openflow_register_cb(void)
{

    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_DO_BARRIER, _hsrv_openflow_do_barrier);

    /* flow */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_ADD, _hsrv_openflow_add_flow);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_MOD, _hsrv_openflow_mod_flow);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_FLOW_DEL, _hsrv_openflow_del_flow);


    /* group */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_GROUP_ADD, _hsrv_openflow_add_group);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_GROUP_DEL, _hsrv_openflow_del_group);

    /* meter */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_ADD, _hsrv_openflow_add_meter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_MOD, _hsrv_openflow_mod_meter);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_METER_DEL, _hsrv_openflow_del_meter);

    /* port */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PORT_ENABLE,  _hsrv_openflow_port_enable);

    /* packet out */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_PACKET_OUT, _hsrv_openflow_packet_out);

    /* stats */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_STATS_CLEAR, _hsrv_openflow_clear_stats);

    /* tunnel */
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_ADD, _hsrv_openflow_tunnel_add_port);
    hsrv_message_set_cb_from_fei(FEI_OPENFLOW_TUNNEL_DEL, _hsrv_openflow_tunnel_del_port);
 
    return HSRV_E_NONE;
}

