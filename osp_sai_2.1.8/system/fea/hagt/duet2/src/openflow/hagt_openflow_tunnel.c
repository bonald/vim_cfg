/* Added by weizj for tunnel */
#include "ctc_api.h"
#include "hagt_inc.h"
#include "hagt.h"
#include "hagt_openflow.h"
#include "glb_openflow_define.h"
#include "hagt_openflow_tunnel.h"


/*************************
 * Global and Declaration
 *************************/

#define ____TUNNEL_INTERNAL_APIs____
/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_update_decap_mode
 * Purpose      : Hagt layer openflow tunnel update decap key mode
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_update_decap_mode(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowTnlDecapModeReq_t *req;
    ctc_overlay_tunnel_global_cfg_t init_param;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowModDecapMode;
    sal_memset(&init_param, 0, sizeof(ctc_overlay_tunnel_global_cfg_t));

    if (!req->decapMode)
    {
        init_param.nvgre_mode |= CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI;
        init_param.vxlan_mode |= CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI;
    }

    HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_init(&init_param));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_del_fid
 * Purpose      : Hagt layer openflow tunnel del fid
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_del_fid(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowFidMapReq_t* req = NULL;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowModFid;

    HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_set_fid(req->tunnelId, 0));

    return HAGT_E_NONE;
}

int32
_hagt_openflow_tunnel_set_vxlan_destport(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowVxlanDestportReq_t* req = NULL;
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowVxlanDestport;
    uint32 destport = 0;

    if(req->isSet)
    {
        destport = req->destport;
        HAGT_IF_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_VXLAN_UDP_DEST_PORT, &destport));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_global_ctl_set(CTC_GLOBAL_VXLAN_UDP_DEST_PORT, &destport));
    }

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_port_decap_en
 * Purpose      : Hagt layer openflow tunnel port decap en
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_port_decap_en(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowPortDecapEnReq_t *req;
    ctc_port_scl_property_t prop;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowPortDecapEn;
    sal_memset(&prop, 0, sizeof(ctc_port_scl_property_t));

    prop.direction = CTC_INGRESS;
#if defined (GOLDENGATE) || defined (DUET2)
    prop.action_type = CTC_PORT_SCL_ACTION_TYPE_TUNNEL;
#endif
    switch (req->type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            prop.scl_id = 0;
            prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL;
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            prop.scl_id = 0;
            prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE;
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            prop.scl_id = 0;
            prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN;
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }

    if (req->enable)
    {
        if (GLB_OPENFLOW_TUNNEL_TYPE_L2GRE == req->type)
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_NVGRE_ENABLE, FALSE));
        }
        else if (GLB_OPENFLOW_TUNNEL_TYPE_NVGRE == req->type)
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_NVGRE_ENABLE, TRUE));
        }
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
    }
    else
    {
        prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        HAGT_IF_ERROR_RETURN(ctc_port_set_scl_property(req->gport, &prop));
    }

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_ipuc_map_decap_flow
 * Purpose      : Hagt layer openflow tunnel map ipuc decap flow info
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_ipuc_map_decap_flow(HalMsgOpenFlowDecapFlowReq_t* p_req,
    ctc_ipuc_tunnel_param_t* p_entry, bool is_del)
{
    p_entry->ip_ver = CTC_IP_VER_4;
    p_entry->payload_type = CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE;

    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST))
    {
        p_entry->ip_da.ipv4 = p_req->tnl.ipDa;
    }

    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC))
    {
        p_entry->ip_sa.ipv4 = p_req->tnl.ipSa;
        GLB_SET_FLAG(p_entry->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA);
    }

    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL))
    {
        p_entry->gre_key = p_req->tnl.vni;
        GLB_SET_FLAG(p_entry->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY);
    }

    if (is_del)
    {
        return HAGT_E_NONE;
    }
    
    /* map scl actions */
    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA))
    {
        p_entry->metadata = p_req->tnl.metadata;
        GLB_SET_FLAG(p_entry->flag, CTC_IPUC_TUNNEL_FLAG_METADATA_EN);
    }

    GLB_SET_FLAG(p_entry->flag, CTC_IPUC_TUNNEL_FLAG_QOS_USE_OUTER_INFO);

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_overlay_map_decap_flow
 * Purpose      : Hagt layer openflow tunnel map overlay decap flow info
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_overlay_map_decap_flow(HalMsgOpenFlowDecapFlowReq_t* p_req,
    ctc_overlay_tunnel_param_t* p_entry, bool is_del)
{
    uint16 fid;
    int32 ret;

    /* map scl key */
    if (p_req->decapMode)
    {
        /* ipda + ipsa + vni */
        p_entry->scl_id = 1;
        if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC))
        {
            p_entry->ipsa.ipv4 = p_req->tnl.ipSa;
            GLB_SET_FLAG(p_entry->flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA);
        }
    }
    else
    {
        /* ipda + vni */
        p_entry->scl_id = 0;
    }

    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST))
    {
        p_entry->ipda.ipv4 = p_req->tnl.ipDa;
    }

    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL))
    {
        p_entry->src_vn_id = p_req->tnl.vni;
    }

    if (is_del)
    {
        return HAGT_E_NONE;
    }

    /* ensure fid exist */
    if (0 != p_entry->src_vn_id)
    {
        p_entry->action.dst_vn_id = p_entry->src_vn_id;
        ret = ctc_overlay_tunnel_get_fid(p_entry->src_vn_id, &fid);
        if (CTC_E_NOT_EXIST == ret)
        {
            fid = p_req->fid;
            HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_set_fid(p_entry->src_vn_id, fid));
        }
    }

    /* map scl actions */
    if (GLB_IS_BIT_SET(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA))
    {
        p_entry->metadata = p_req->tnl.metadata;
        GLB_SET_FLAG(p_entry->flag, CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN);
    }

    GLB_SET_FLAG(p_entry->flag, CTC_OVERLAY_TUNNEL_FLAG_QOS_USE_OUTER_INFO);

    return HAGT_E_NONE;
}

// DEPRECATED.
#if 0
/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_l3if_en
 * Purpose      : Hagt layer openflow tunnel enable l3if
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_l3if_en(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowL3ifReq_t* req = NULL;
    ctc_l3if_t entry;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowL3if;
    sal_memset(&entry, 0, sizeof(ctc_l3if_t));

    entry.gport = req->gport;
    entry.vlan_id = req->vlanId;
    entry.l3if_type = CTC_L3IF_TYPE_PHY_IF;

    if (req->enable)
    {
        HAGT_IF_ERROR_RETURN(ctc_l3if_create(req->l3ifid, &entry));
        HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(req->l3ifid, CTC_L3IF_PROP_ROUTE_ALL_PKT, TRUE));

    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_l3if_set_property(req->l3ifid, CTC_L3IF_PROP_ROUTE_ALL_PKT, FALSE));
        HAGT_IF_ERROR_RETURN(ctc_l3if_destory(req->l3ifid, &entry));
    }

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_set_l3if
 * Purpose      : Hagt layer openflow tunnel set l3if
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_set_l3if(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowSetL3ifReq_t* req = NULL;

    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowSetL3if;

    if (req->enable)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_phy_if_en(req->gport, TRUE));
        /* Added by weizj for tunnel bug: 33368, default is CTC_DOT1Q_TYPE_BOTH */
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));
    }
    else
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_phy_if_en(req->gport, FALSE));
        /* Added by weizj for tunnel bug: 33360 */
        HAGT_IF_ERROR_RETURN(ctc_port_set_property(req->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, TRUE));
    }

    return HAGT_E_NONE;
}
#endif

/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_add_decap_flow
 * Purpose      : Hagt layer openflow tunnel add decap flow
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_add_decap_flow(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDecapFlowReq_t *req;
    ctc_overlay_tunnel_param_t overlay_entry;
    ctc_ipuc_tunnel_param_t ipuc_entry;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowAddDecapFlow;
    memset(&overlay_entry, 0, sizeof(ctc_overlay_tunnel_param_t));
    memset(&ipuc_entry, 0, sizeof(ctc_ipuc_tunnel_param_t));

    switch(req->type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            HAGT_IF_ERROR_RETURN(_hagt_openflow_ipuc_map_decap_flow(req, &ipuc_entry, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_ipuc_add_tunnel(&ipuc_entry));
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            overlay_entry.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
            HAGT_IF_ERROR_RETURN(_hagt_openflow_overlay_map_decap_flow(req, &overlay_entry, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_add_tunnel(&overlay_entry));
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            overlay_entry.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
            HAGT_IF_ERROR_RETURN(_hagt_openflow_overlay_map_decap_flow(req, &overlay_entry, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_add_tunnel(&overlay_entry));
            break;
        default:
            HAGT_LOG_CRIT("Decap flow type error");
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : _hagt_openflow_tunnel_del_decap_flow
 * Purpose      : Hagt layer openflow tunnel del decap flow
 * Input        : p_msg_data: Hsrv2AgtMsg_t*
 * Output       : N/A
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
_hagt_openflow_tunnel_del_decap_flow(Hsrv2AgtMsg_t* p_msg_data)
{
    HalMsgOpenFlowDecapFlowReq_t *req;
    ctc_overlay_tunnel_param_t overlay_entry;
    ctc_ipuc_tunnel_param_t ipuc_entry;
    
    req = &p_msg_data->opHsrv2Agt.hsrv2AgtMsgOpenFlowDelDecapFlow;
    memset(&overlay_entry, 0, sizeof(ctc_overlay_tunnel_param_t));
    memset(&ipuc_entry, 0, sizeof(ctc_ipuc_tunnel_param_t));

    switch(req->type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            HAGT_IF_ERROR_RETURN(_hagt_openflow_ipuc_map_decap_flow(req, &ipuc_entry, TRUE));
            HAGT_IF_ERROR_RETURN(ctc_ipuc_remove_tunnel(&ipuc_entry));
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            overlay_entry.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
            HAGT_IF_ERROR_RETURN(_hagt_openflow_overlay_map_decap_flow(req, &overlay_entry, TRUE));
            HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_remove_tunnel(&overlay_entry));
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            overlay_entry.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
            HAGT_IF_ERROR_RETURN(_hagt_openflow_overlay_map_decap_flow(req, &overlay_entry, TRUE));
            HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_remove_tunnel(&overlay_entry));
            break;
        default:
            HAGT_LOG_CRIT("Decap flow type error");
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

#define ____TUNNEL_APIs____
/*************************************************************************************************
 * Name         : hagt_openflow_tunnel_set_fid
 * Purpose      : Hagt layer openflow tunnel set fid
 * Input        : vni: uint32, fid: uint16
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
hagt_openflow_tunnel_set_fid(uint32 vni, uint16 fid)
{
    if (HAGT_OPENFLOW_TUNNEL_INVALID_VNI >= vni)
    {
        return HAGT_E_NONE;
    }

    if (HAGT_OPENFLOW_TUNNEL_INVALID_FID > fid)
    {
        return HAGT_E_NONE;
    }

    HAGT_IF_ERROR_RETURN(ctc_overlay_tunnel_set_fid(vni, fid));

    return HAGT_E_NONE;
}

/*************************************************************************************************
 * Name         : hagt_openflow_tunnel_register_callback
 * Purpose      : Hagt layer openflow tunnel msg callback register
 * Input        : N/A
 * Output       : N/A 
 * Return       : HAGT_E_XXX
**************************************************************************************************/
int32
hagt_openflow_tunnel_register_callback(void)
{   
#define Reg(a, b)\
    hagt_message_set_msg_callback(a, b)
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowModDecapMode, _hagt_openflow_tunnel_update_decap_mode);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowModFid, _hagt_openflow_tunnel_del_fid);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortDecapEn, _hagt_openflow_tunnel_port_decap_en);

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddDecapFlow, _hagt_openflow_tunnel_add_decap_flow);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelDecapFlow, _hagt_openflow_tunnel_del_decap_flow);

    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowVxlanDestport, _hagt_openflow_tunnel_set_vxlan_destport);

// DEPRECATED.
#if 0
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowL3if, _hagt_openflow_tunnel_l3if_en);
    Reg(opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetL3if, _hagt_openflow_tunnel_set_l3if);
#endif

#undef Reg    
    return HAGT_E_NONE;
}

