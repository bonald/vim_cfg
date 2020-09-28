/* Added by weizj for tunnel */
#include "sal_common.h"

#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_tunnel_priv.h"
#include "hsrv_tunneldb_cdb.h"


/*************************
 * Global and Declaration
 *************************/

extern int32
hsrv_openflow_group_update_group_liveness(uint32 ifindex, bool link_enable);

extern int32
hsrv_openflow_nexthop_lookup_ip_tunnel_nh(hsrv_openflow_ip_nh_tunnel_info_t* p_nh_param,
    hsrv_ofact_ip_tunnel_nh_info_t** pp_tnl_info);

extern int32
hsrv_openflow_nexthop_remove_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_tnl_info);

extern int32
hsrv_openflow_nexthop_send_ip_tunnel_nh(hsrv_ofact_ip_tunnel_nh_info_t* p_info);

extern int32
hsrv_openflow_cdb_bond_set_member_tunnel_type(uint32 ifindex, void *user_data);

extern int32
hsrv_openflow_nexthop_offset_alloc(hsrv_of_nh_offset_t* p_nh_offset);

extern int32
hsrv_neighbor_arp_update_by_dynamic_tunnel_remote_ip(uint32 remote_ip, char*direct_connection_ifname, uint32 is_add);

extern void
hsrv_openflow_get_vxlan_src_port(uint32* p_is_set, uint32* p_vxlan_srcport);

extern void
hsrv_openflow_get_vxlan_dest_port(uint32* p_is_set, uint32* p_vxlan_destport);

hsrv_openflow_tunnel_master_t* g_pst_openflow_tunnel_master = NULL;

uint32 tid_tunnel_type[64] = {0};


#define ____TUNNEL_MSG_API____
/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_update_decap_mode
 * Purpose      : Hsrv layer openflow tunnel send decap mode to hagt layer
 * Input        : N/A
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_update_decap_mode(void)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowTnlDecapModeReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowModDecapMode;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowModDecapMode;
    p_req->decapMode = HSRV_OPENFLOW_TUNNEL_DECAP_MODE;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_port_decap_en
 * Purpose      : Hsrv layer openflow tunnel send port decap enable to hagt layer 
 * Input        : pst_req: fei_openflow_port_decap_req_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_msg_tunnel_port_decap_en(fei_openflow_port_decap_req_t* pst_req)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowPortDecapEnReq_t* p_req = NULL;
    uint16 gport;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowPortDecapEn;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    if (hsrv_openflow_port_is_bond(pst_req->ifindex))
    {
#if 0
        hsrv_openflow_bond_data_t* p_bond_info = NULL;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(pst_req->ifindex, &p_bond_info));
        gport = p_bond_info->gport;
#endif
        hsrv_openflow_bond_get_gport_by_ifindex(pst_req->ifindex, &gport);
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(pst_req->ifindex, &gport));
    }

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowPortDecapEn;
    p_req->gport = gport;
    p_req->type = pst_req->type;
    p_req->enable = pst_req->enable;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_add_decap_flow
 * Purpose      : Hsrv layer openflow tunnel send add decap flow to hagt layer
 * Input        : fid: uint16, p_key: glb_openflow_tnl_key_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_add_decap_flow(uint16 fid, glb_openflow_tnl_key_t* p_key)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDecapFlowReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowAddDecapFlow;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowAddDecapFlow;
    p_req->fid = fid;
    p_req->decapMode = HSRV_OPENFLOW_TUNNEL_DECAP_MODE;
    p_req->type = p_key->type;

    if (p_key->tun_id)
    {
        p_req->tnl.vni = p_key->tun_id;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL);
    }

    if (p_key->metadata)
    {
        p_req->tnl.metadata = p_key->metadata;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA);
    }

    if (p_key->ipSa)
    {
        p_req->tnl.ipSa = p_key->ipSa;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC);
    }

    if (p_key->ipDa)
    {
        p_req->tnl.ipDa = p_key->ipDa;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST);
    }

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg)); 

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_del_decap_flow
 * Purpose      : Hsrv layer openflow tunnel send del decap flow to hagt layer
 * Input        : p_key: glb_openflow_tnl_key_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_del_decap_flow(glb_openflow_tnl_key_t* p_key)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowDecapFlowReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowDelDecapFlow;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowDelDecapFlow;
    p_req->decapMode = HSRV_OPENFLOW_TUNNEL_DECAP_MODE;
    p_req->type = p_key->type;
    if (p_key->tun_id)
    {
        p_req->tnl.vni = p_key->tun_id;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_TUNNEL);
    }

    if (p_key->ipSa)
    {
        p_req->tnl.ipSa = p_key->ipSa;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_SRC);
    }

    if (p_key->ipDa)
    {
        p_req->tnl.ipDa = p_key->ipDa;
        GLB_SET_BIT(p_req->tnl.flags, GLB_OPENFLOW_ACTION_TYPE_SET_IPV4_DST);
    }

    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_del_fid
 * Purpose      : Hsrv layer openflow tunnel send del fid to hagt layer
 * Input        : tun_id: uint32 
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_del_fid(uint32 tun_id)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowFidMapReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowModFid;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowModFid;
    p_req->tunnelId = tun_id;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

// DEPRECATED.
#if 0
/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_create_l3if
 * Purpose      : Hsrv layer openflow tunnel send create l3if to hagt layer
 * Input        : gport: uint16 
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_create_l3if(uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowL3ifReq_t* p_req = NULL;
    uint16 l3ifid;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowL3if;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_l3ifid(gport, &l3ifid));
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowL3if;
    p_req->l3ifid = l3ifid;
    p_req->gport = gport;
    p_req->vlanId = HSRV_OPENFLOW_PORT_INVALID_VLANID;
    p_req->enable = 1;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_destory_l3if
 * Purpose      : Hsrv layer openflow tunnel send destory l3if to hagt layer
 * Input        : gport: uint16
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_destory_l3if(uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowL3ifReq_t* p_req = NULL;
    uint16 l3ifid;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowL3if;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_l3ifid(gport, &l3ifid));
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowL3if;
    p_req->l3ifid = l3ifid;
    p_req->gport = gport;
    p_req->vlanId = HSRV_OPENFLOW_PORT_INVALID_VLANID;
    p_req->enable = 0;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_set_l3if
 * Purpose      : Hsrv layer openflow tunnel send set l3if to hagt layer
 * Input        : gport: uint16 
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_set_l3if(uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetL3ifReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetL3if;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetL3if;
    p_req->gport = gport;
    p_req->enable = 1;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_msg_tunnel_unset_l3if
 * Purpose      : Hsrv layer openflow tunnel send unset l3if to hagt layer
 * Input        : gport: uint16 
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
hsrv_openflow_msg_tunnel_unset_l3if(uint16 gport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowSetL3ifReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowSetL3if;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowSetL3if;
    p_req->gport = gport;
    p_req->enable = 0;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}
#endif

#define ____TUNNEL_HASH_API____
/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_info_hash_make
 * Purpose      : Hsrv layer openflow tunnel tunnel-info hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_info_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_info_t* p_info = NULL;
    uint32 ifindex;

    p_info = (hsrv_openflow_tunnel_info_t*)p_info_void;
    ifindex = p_info->ifindex;

    p_info = NULL;

    return ifindex;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_info_hash_compare
 * Purpose      : Hsrv layer openflow tunnel tunnel-info hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_info_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_info_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_info_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_info_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_info_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->ifindex != p_info_to_lkp->ifindex)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_metadata_hash_make
 * Purpose      : Hsrv layer openflow tunnel metadata hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_metadata_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_metadata_t* metadata_info = NULL;
    uint32_ofp key = 0;

    metadata_info = (hsrv_openflow_tunnel_metadata_t*)p_info_void;
    key = metadata_info->ofport | metadata_info->tun_id;

    metadata_info = NULL;

    return key % HSRV_OPENFLOW_TUNNEL_METADATA_BLOCK_SIZE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_metadata_hash_compare
 * Purpose      : Hsrv layer openflow tunnel metadata hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_metadata_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_metadata_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_metadata_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_metadata_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_metadata_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->ofport != p_info_to_lkp->ofport)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }
    if (p_info_in_bucket->tun_id != p_info_to_lkp->tun_id)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_fid_hash_make
 * Purpose      : Hsrv layer openflow tunnel fid hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_fid_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_fid_t* p_info = NULL;
    uint32 tun_id;

    p_info = (hsrv_openflow_tunnel_fid_t*)p_info_void;
    tun_id = p_info->tun_id;

    p_info = NULL;

    return tun_id;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_fid_hash_compare
 * Purpose      : Hsrv layer openflow tunnel fid hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_fid_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_fid_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_fid_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_fid_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_fid_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->tun_id != p_info_to_lkp->tun_id)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_flow_hash_make
 * Purpose      : Hsrv layer openflow tunnel flow hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_flow_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_decap_flow_t* p_info = NULL;
    uint16 metadata;

    p_info = (hsrv_openflow_tunnel_decap_flow_t*)p_info_void;
    metadata = p_info->metadata;

    p_info = NULL;

    return metadata;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_flow_hash_compare
 * Purpose      : Hsrv layer openflow tunnel flow hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_flow_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_decap_flow_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_decap_flow_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_decap_flow_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_decap_flow_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->metadata != p_info_to_lkp->metadata)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

static inline uint32
_hsrv_openflow_dynamic_tunnel_info_hash_make(void* p_info_void)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_info = NULL;
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    p_info = (hsrv_openflow_dynamic_tunnel_info_t*)p_info_void;

    pval = (uint8*)&p_info->dynamic_tunnel_info_key;
    for (index = 0; index < sizeof(p_info->dynamic_tunnel_info_key); index++)
    {
        key += pval[index];
    }

    return key;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_info_hash_compare
 * Purpose      : Hsrv layer openflow tunnel tunnel-info hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_dynamic_tunnel_info_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_info_in_bucket = NULL;
    hsrv_openflow_dynamic_tunnel_info_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_dynamic_tunnel_info_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_dynamic_tunnel_info_t*)p_info_to_lkp_void;

    if (0 == sal_memcmp(&p_info_in_bucket->dynamic_tunnel_info_key, &p_info_to_lkp->dynamic_tunnel_info_key, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t)))
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return TRUE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return FALSE;
}


// DEPRECATED.
#if 0
/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_l3if_hash_make
 * Purpose      : Hsrv layer openflow tunnel l3if hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_l3if_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_l3if_t* p_info = NULL;
    uint16 gport;

    p_info = (hsrv_openflow_tunnel_l3if_t*)p_info_void;
    gport = p_info->gport;

    p_info = NULL;

    return gport;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_l3if_hash_compare
 * Purpose      : Hsrv layer openflow tunnel l3if hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_l3if_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_l3if_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_l3if_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_l3if_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_l3if_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->gport != p_info_to_lkp->gport)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}
#endif

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_ofport_hash_make
 * Purpose      : Hsrv layer openflow tunnel ofport hash make
 * Input        : p_info_void: void*
 * Output       : N/A 
 * Return       : ifindex
**************************************************************************************************/
static inline uint32
_hsrv_openflow_tunnel_ofport_hash_make(void* p_info_void)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;
    uint16 metadata;

    p_info = (hsrv_openflow_tunnel_metadata_t*)p_info_void;
    metadata = p_info->metadata;

    p_info = NULL;

    return metadata;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_ofport_hash_compare
 * Purpose      : Hsrv layer openflow tunnel ofport hash compare
 * Input        : p_info_in_bucket_void: void*, p_info_to_lkp_void: void*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_tunnel_ofport_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_tunnel_metadata_t* p_info_in_bucket = NULL;
    hsrv_openflow_tunnel_metadata_t* p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_tunnel_metadata_t*)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_tunnel_metadata_t*)p_info_to_lkp_void;

    if (p_info_in_bucket->metadata != p_info_to_lkp->metadata)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}


#define ____TUNNEL_INTERNAL_API____
/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_alloc_fid
 * Purpose      : Hsrv layer openflow tunnel alloc fid           
 * Input        : N/A           
 * Output       : p_fid: uint16*            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_alloc_fid(uint16* p_fid)
{
    uint32 fid = CTCLIB_OPF_INVALID_OFFSET;
    
    fid = ctclib_opf_alloc_offset(HSRV_OPENFLOW_TUNNEL_FID_OPF, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == fid)
    {
        HSRV_LOG_WARN("Openflow tunnel not enough fid offset resource.");
        *p_fid = 0;
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }
    
    *p_fid = fid & 0xFFFF;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_alloc_metadata
 * Purpose      : Hsrv layer openflow tunnel alloc metadta 
 * Input        : N/A
 * Output       : p_metadata: uint16* 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_alloc_metadata(uint16* p_metadata)
{
    uint32 metadata = CTCLIB_OPF_INVALID_OFFSET;
    
    metadata = ctclib_opf_alloc_offset(HSRV_OPENFLOW_TUNNEL_METADATA_OPF, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == metadata)
    {
        HSRV_LOG_WARN("Openflow tunnel not enough metadata offset resource.");
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }
    
    *p_metadata = metadata & 0xFFFF;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_info
 * Purpose      : Hsrv layer openflow tunnel lookup tunnel-info
 * Input        : ifindex: uint32
 * Output       : pp_info: hsrv_openflow_tunnel_info_t**
 * Return       : HSRV_E_XXX
**************************************************************************************************/
 int32
_hsrv_openflow_tunnel_lookup_info(uint32 ifindex, hsrv_openflow_tunnel_info_t** pp_info)
{
    hsrv_openflow_tunnel_info_t lkp_info;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_info_t));
    lkp_info.ifindex = ifindex;

    p_tnl_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_INFO_HASH, &lkp_info);
    if (p_tnl_info)
    {
        *pp_info = p_tnl_info;
        p_tnl_info = NULL;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_flow
 * Purpose      : Hsrv layer openflow tunnel lookup decap flow
 * Input        : metadata: uint16
 * Output       : pp_info: hsrv_openflow_tunnel_decap_flow_t** 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_tunnel_lookup_flow(uint16 metadata, hsrv_openflow_tunnel_decap_flow_t** pp_info)
{
    hsrv_openflow_tunnel_decap_flow_t lkp_info;
    hsrv_openflow_tunnel_decap_flow_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_decap_flow_t));
    lkp_info.metadata = metadata;

    p_tnl_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_FLOW_HASH, &lkp_info);
    if (p_tnl_info)
    {
        *pp_info = p_tnl_info;
        p_tnl_info = NULL;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_fid
 * Purpose      : Hsrv layer openflow tunnel lookup fid
 * Input        : tun_id: uint32
 * Output       : pp_info: hsrv_openflow_tunnel_fid_t**
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_tunnel_lookup_fid(uint32 tun_id, hsrv_openflow_tunnel_fid_t** pp_info)
{
    hsrv_openflow_tunnel_fid_t lkp_info;
    hsrv_openflow_tunnel_fid_t* p_fid_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_fid_t));
    lkp_info.tun_id = tun_id;

    p_fid_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_FID_HASH, &lkp_info);
    if (p_fid_info)
    {
        *pp_info = p_fid_info;
        p_fid_info = NULL;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_metadata
 * Purpose      : Hsrv layer openflow tunnel lookup metadata
 * Input        : ofport: uint16, tun_id: uint32
 * Output       : pp_info: hsrv_openflow_tunnel_metadata_t**
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_tunnel_lookup_metadata(uint16 ofport, uint32 tun_id, hsrv_openflow_tunnel_metadata_t** pp_info)
{
    hsrv_openflow_tunnel_metadata_t lkp_info;
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_metadata_t));
    lkp_info.ofport = ofport;
    lkp_info.tun_id = tun_id;

    p_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_METADATA_HASH, &lkp_info);
    if (p_info)
    {
        *pp_info = p_info;
        p_info = NULL;
    }

    return HSRV_E_NONE;
}

// DEPRECATED.
#if 0
/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_l3if
 * Purpose      : Hsrv layer openflow tunnel lookup l3if
 * Input        : gport: uint16
 * Output       : pp_info: hsrv_openflow_tunnel_l3if_t**
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_tunnel_lookup_l3if(uint16 gport, hsrv_openflow_tunnel_l3if_t** pp_info)
{
    hsrv_openflow_tunnel_l3if_t lkp_info;
    hsrv_openflow_tunnel_l3if_t* p_fid_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_l3if_t));
    lkp_info.gport = gport;

    p_fid_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_L3IF_HASH, &lkp_info);
    if (p_fid_info)
    {
        *pp_info = p_fid_info;
        p_fid_info = NULL;
    }

    return HSRV_E_NONE;
}
#endif

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_lookup_ofport
 * Purpose      : Hsrv layer openflow tunnel lookup ofport
 * Input        : metadata: uint16
 * Output       : pp_info: hsrv_openflow_tunnel_metadata_t**
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static int32
_hsrv_openflow_tunnel_lookup_ofport(uint16 metadata, hsrv_openflow_tunnel_metadata_t** pp_info)
{
    hsrv_openflow_tunnel_metadata_t lkp_info;
    hsrv_openflow_tunnel_metadata_t* p_fid_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_metadata_t));
    lkp_info.metadata = metadata;

    p_fid_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_OFPORT_HASH, &lkp_info);
    if (p_fid_info)
    {
        *pp_info = p_fid_info;
        p_fid_info = NULL;
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_add_ofport
 * Purpose      : Hsrv layer openflow tunnel add ofport      
 * Input        : metadata: uint16, ofport: uint16, tun_id: uint32           
 * Output       : N/A       
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_add_ofport(uint16 metadata, uint16 ofport, uint32 tun_id)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;

    _hsrv_openflow_tunnel_lookup_ofport(metadata, &p_info);
    if (p_info)
    {
        p_info->ref_cnt ++;
        return HSRV_E_NONE;
    }

    p_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_metadata_t));
    HSRV_MEM_CHECK(p_info);
    sal_memset(p_info, 0, sizeof(hsrv_openflow_tunnel_metadata_t));

    p_info->metadata = metadata;
    p_info->ofport = ofport;
    p_info->tun_id = tun_id;
    p_info->ref_cnt ++;

    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_OFPORT_HASH, p_info, ctclib_hash_alloc_intern);

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_del_ofport
 * Purpose      : Hsrv layer openflow tunnel delete ofport       
 * Input        : metadata: uint16          
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_del_ofport(uint16 metadata)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;

    _hsrv_openflow_tunnel_lookup_ofport(metadata, &p_info);
    if (!p_info)
    {
        return HSRV_E_NONE;
    }

    p_info->ref_cnt --;
    if (0 == p_info->ref_cnt)
    {
        /* delete from hash */
        ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_OFPORT_HASH, p_info);
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_get_fid
 * Purpose      : Hsrv layer openflow tunnel get fid          
 * Input        : tun_id: uint32           
 * Output       : p_label: uint16*            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_get_fid(uint32 tun_id, uint16* p_label)
{
    hsrv_openflow_tunnel_fid_t* p_info = NULL;
    uint16 fid;

    _hsrv_openflow_tunnel_lookup_fid(tun_id, &p_info);
    if (p_info)
    {
        *p_label = p_info->fid;
        p_info->ref_cnt ++;
        return HSRV_E_NONE;
    }

    p_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_fid_t));
    HSRV_MEM_CHECK(p_info);
    sal_memset(p_info, 0, sizeof(hsrv_openflow_tunnel_fid_t));

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_alloc_fid(&fid));
    p_info->tun_id = tun_id;
    p_info->fid = fid;
    p_info->ref_cnt ++;

    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_FID_HASH, p_info, ctclib_hash_alloc_intern);
    HSRV_OPENFLOW_TUNNEL_FID_NUM ++;

    *p_label = p_info->fid;

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_del_fid
 * Purpose      : Hsrv layer openflow tunnel delete fid         
 * Input        : tun_id: uint32           
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_del_fid(uint32 tun_id)
{
    hsrv_openflow_tunnel_fid_t* p_info = NULL;
    uint16 fid;

    _hsrv_openflow_tunnel_lookup_fid(tun_id, &p_info);
    if (!p_info)
    {
        HSRV_LOG_WARN("Openflow tunnel delete fid error.");
        return HSRV_E_NONE;
    }

    /* delete fid */
    p_info->ref_cnt --;
    if (0 == p_info->ref_cnt)
    {
        fid = p_info->fid;
        /* delete from hash */
        ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_FID_HASH, p_info);
        /* delete from opf */
        ctclib_opf_free_offset(HSRV_OPENFLOW_TUNNEL_FID_OPF, 0, 1, fid);
        /* send delete msg */
        hsrv_openflow_msg_tunnel_del_fid(tun_id);

        HSRV_OPENFLOW_TUNNEL_FID_NUM --;
        HSRV_OPENFLOW_TUNNEL_FREE(p_info);
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_get_metadata
 * Purpose      : Hsrv layer openflow tunnel get metadata         
 * Input        : ofport: uint16, tun_id: uint32           
 * Output       : p_metadata: uint16*            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_get_metadata(uint16 ofport, uint32 tun_id, uint16* p_metadata)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;
    uint16 metadata;

    _hsrv_openflow_tunnel_lookup_metadata(ofport, tun_id, &p_info);
    if (p_info)
    {
        *p_metadata = p_info->metadata;
        p_info->ref_cnt ++;
        return HSRV_E_NONE;
    }

    p_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_metadata_t));
    HSRV_MEM_CHECK(p_info);
    sal_memset(p_info, 0, sizeof(hsrv_openflow_tunnel_metadata_t));

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_alloc_metadata(&metadata));
    p_info->metadata = metadata;
    p_info->tun_id = tun_id;
    p_info->ofport = ofport;
    p_info->ref_cnt ++;

    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_METADATA_HASH, p_info, ctclib_hash_alloc_intern);
    HSRV_OPENFLOW_TUNNEL_METADATA_NUM ++;

    /* add ofport */
    if (ofport)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_add_ofport(metadata, ofport, tun_id));
    }

    *p_metadata = p_info->metadata;

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_del_metadata
 * Purpose      : Hsrv layer openflow tunnel delete metadata          
 * Input        : ofport: uint16, tun_id: uint32           
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_del_metadata(uint16 ofport, uint32 tun_id)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;
    uint32 metadata;

    _hsrv_openflow_tunnel_lookup_metadata(ofport, tun_id, &p_info);
    if (!p_info)
    {
        HSRV_LOG_WARN("Openflow tunnel delete metadata error.");
        return HSRV_E_NONE;
    }
    
    /* delete metadata */
    p_info->ref_cnt --;
    if (0 == p_info->ref_cnt)
    {
        metadata = p_info->metadata;
        /* delete ofport */
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_del_ofport(metadata));
        /* delete from hash */
        ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_METADATA_HASH, p_info);
        /* delete from opf */
        ctclib_opf_free_offset(HSRV_OPENFLOW_TUNNEL_METADATA_OPF, 0, 1, metadata);

        HSRV_OPENFLOW_TUNNEL_METADATA_NUM --;
        HSRV_OPENFLOW_TUNNEL_FREE(p_info);
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_bond_decap_en
 * Purpose      : Hsrv layer openflow tunnel set bind port decap enable        
 * Input        : phb: hash_bakcet, user_data: void*     
 * Output       : N/A            
 * Return       : void
**************************************************************************************************/
void
_hsrv_openflow_tunnel_bond_decap_en(ctclib_hash_backet_t* phb, void *user_data)
{
    int32 ret;
    hsrv_openflow_slave_data_t* p_slave_info = phb->data;
    fei_openflow_port_decap_req_t* pst_req = user_data;

    if (p_slave_info)
    {
        pst_req->ifindex = p_slave_info->ifindex;
        ret = hsrv_openflow_msg_tunnel_port_decap_en(pst_req);
        if (HSRV_E_NONE != ret)
        {
            HSRV_LOG_ERR("Hsrv enable slave port decap fail: ifindex = %d rv = %d", pst_req->ifindex, ret);
        }
    }

    return;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_add_decap_flow
 * Purpose      : Hsrv layer openflow tunnel add decap flow         
 * Input        : fid: uint16, p_flow: glb_openflow_tnl_key_t*     
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_add_decap_flow(uint16 fid, glb_openflow_tnl_key_t* p_flow)
{
    hsrv_openflow_tunnel_decap_flow_t* p_info = NULL;
    uint16 metadata;

    metadata = p_flow->metadata;

    _hsrv_openflow_tunnel_lookup_flow(metadata, &p_info);
    if (p_info)
    {
        p_info->ref_cnt ++;
        return HSRV_E_NONE;
    }

    /* send add msg */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_add_decap_flow(fid, p_flow));

    p_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_decap_flow_t));
    HSRV_MEM_CHECK(p_info);
    sal_memset(p_info, 0, sizeof(hsrv_openflow_tunnel_decap_flow_t));

    p_info->metadata = metadata;
    sal_memcpy(&p_info->tun_key, p_flow, sizeof(glb_openflow_tnl_key_t));
    p_info->ref_cnt ++;

    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_FLOW_HASH, p_info, ctclib_hash_alloc_intern);
    HSRV_OPENFLOW_TUNNEL_FLOW_NUM ++;

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_del_decap_flow
 * Purpose      : Hsrv layer openflow tunnel delete decap flow      
 * Input        : metadata: uint16
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_del_decap_flow(uint16 metadata)
{
    hsrv_openflow_tunnel_decap_flow_t* p_info = NULL;

    _hsrv_openflow_tunnel_lookup_flow(metadata, &p_info);
    if (!p_info)
    {
        HSRV_LOG_WARN("Openflow tunnel delete decap flow error.");
        return HSRV_E_NONE;
    }

    /* delete metadata */
    p_info->ref_cnt --;
    if (0 == p_info->ref_cnt)
    {
        /* send delete msg */
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_del_decap_flow(&p_info->tun_key));

        /* delete from hash */
        ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_FLOW_HASH, p_info);

        HSRV_OPENFLOW_TUNNEL_FLOW_NUM --;
        HSRV_OPENFLOW_TUNNEL_FREE(p_info);
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}

static void
_hsrv_openflow_tunnel_group_update_group_liveness(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl = (hsrv_openflow_tunnel_info_t*)phb->data;
    fei_openflow_port_notify_link_t* p_data = user_data;

    if(p_tnl->tunnel_info.bond_port_ifindex == p_data->ifindex)
    {
        hsrv_openflow_group_update_group_liveness(p_tnl->ifindex, p_data->enable);
    }
}

int32 hsrv_openflow_tunnel_group_update(fei_openflow_port_notify_link_t *pst_req)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, _hsrv_openflow_tunnel_group_update_group_liveness, pst_req);

    return 0;
}

// DEPRECATED.

#if 0
/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_set_l3if_slave
 * Purpose      : Hsrv layer openflow tunnel l3if enable        
 * Input        : ifindex: uint32           
 * Output       : N/A          
 * Return       : void
**************************************************************************************************/
void
_hsrv_openflow_tunnel_set_l3if_slave(ctclib_hash_backet_t* phb, void *user_data)
{
    uint16 gport;
    int32 ret;
    hsrv_openflow_slave_data_t* p_slave_info = phb->data;

    if (p_slave_info)
    {
        if (FALSE == p_slave_info->enable)
        {
            return ;
        }

        hsrv_ifdb_get_portid(p_slave_info->ifindex, &gport);
        ret = hsrv_openflow_msg_tunnel_set_l3if(gport);
        if (HSRV_E_NONE != ret)
        {
            HSRV_LOG_ERR("Hsrv set slave l3if property fail: ifindex = %d rv = %d", p_slave_info->ifindex, ret);
        }
    }

    return ;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_unset_l3if_slave
 * Purpose      : Hsrv layer openflow tunnel delete l3if        
 * Input        : ifindex: uint32           
 * Output       : N/A            
 * Return       : void
**************************************************************************************************/
void
_hsrv_openflow_tunnel_unset_l3if_slave(ctclib_hash_backet_t* phb, void *user_data)
{
    uint16 gport;
    int32 ret;
    hsrv_openflow_slave_data_t* p_slave_info = phb->data;

    if (p_slave_info)
    {
        if (FALSE == p_slave_info->enable)
        {
            return ;
        }

        hsrv_ifdb_get_portid(p_slave_info->ifindex, &gport);
        ret = hsrv_openflow_msg_tunnel_unset_l3if(gport);
        if (HSRV_E_NONE != ret)
        {
            HSRV_LOG_ERR("Hsrv unset slave l3if property fail: ifindex = %d rv = %d", p_slave_info->ifindex, ret);
        }
    }

    return ;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_l3if_enable
 * Purpose      : Hsrv layer openflow tunnel l3if enable        
 * Input        : ifindex: uint32           
 * Output       : N/A          
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_l3if_enable(uint32 ifindex)
{
    hsrv_openflow_tunnel_l3if_t* p_info = NULL;
    hsrv_openflow_bond_data_t* p_bond_info = NULL;
    uint16 gport;

    if (hsrv_openflow_port_is_bond(ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(ifindex, &p_bond_info));
        p_bond_info->ref_count ++;
        gport = p_bond_info->gport;
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    }

    _hsrv_openflow_tunnel_lookup_l3if(gport, &p_info);
    if (p_info)
    {
        p_info->ref_cnt ++;
        return HSRV_E_NONE;
    }

    p_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_l3if_t));
    HSRV_MEM_CHECK(p_info);
    sal_memset(p_info, 0, sizeof(hsrv_openflow_tunnel_l3if_t));

    p_info->gport = gport;
    p_info->ref_cnt ++;

    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_L3IF_HASH, p_info, ctclib_hash_alloc_intern);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_create_l3if(gport));

    if (hsrv_openflow_port_is_bond(ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(ifindex, &p_bond_info));

        ctclib_hash_iterate(p_bond_info->bond_slave_hash, _hsrv_openflow_tunnel_set_l3if_slave, NULL);        
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_set_l3if(gport));
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_tunnel_l3if_disable
 * Purpose      : Hsrv layer openflow tunnel delete l3if        
 * Input        : ifindex: uint32           
 * Output       : N/A            
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
_hsrv_openflow_tunnel_l3if_disable(uint32 ifindex)
{
    hsrv_openflow_tunnel_l3if_t* p_info = NULL;
    hsrv_openflow_bond_data_t* p_bond_info = NULL;
    uint16 gport;

    if (hsrv_openflow_port_is_bond(ifindex))
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(ifindex, &p_bond_info));
        p_bond_info->ref_count --;
        gport = p_bond_info->gport;
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    }

    _hsrv_openflow_tunnel_lookup_l3if(gport, &p_info);
    if (!p_info)
    {
        return HSRV_E_NONE;
    }

    p_info->ref_cnt --;
    if (0 == p_info->ref_cnt)
    {
        /* delete from hash */
        ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_L3IF_HASH, p_info);
        /* send delete msg */
        hsrv_openflow_msg_tunnel_destory_l3if(gport);

        if (hsrv_openflow_port_is_bond(ifindex))
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(ifindex, &p_bond_info));

            ctclib_hash_iterate(p_bond_info->bond_slave_hash, _hsrv_openflow_tunnel_unset_l3if_slave, NULL);        
        }
        else
        {
            hsrv_openflow_msg_tunnel_unset_l3if(gport);
        }
    }

    p_info = NULL;
    
    return HSRV_E_NONE;
}
#endif

#define ____TUNNEL_HAL_APIs____
/*************************************************************************************************
 * Name         : fei_api_openflow_tunnel_port_decap_en
 * Purpose      : Hsrv layer openflow tunnel set bind port decap enable           
 * Input        : p_req: fei_openflow_add_tunnel_req_t*               
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_tunnel_port_decap_en(fei_openflow_port_decap_req_t* pst_req)
{
    uint32 ifindex;

    HSRV_OPENFLOW_PTR_CHECK(pst_req);
    
    ifindex = pst_req->ifindex;

    if (hsrv_openflow_port_is_bond(ifindex))
    {
        tid_tunnel_type[ifindex - GLB_BOND_IFINDEX_BASE] = pst_req->type;
        hsrv_openflow_cdb_bond_set_member_tunnel_type(ifindex, pst_req);
    }
    else
    {
#if 0
        if (!hsrv_openflow_phyport_is_openflow_port(ifindex))
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
#endif
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_port_decap_en(pst_req));
    }

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : fei_api_openflow_tunnel_update_decap_mode
 * Purpose      : Hsrv layer openflow tunnel update decap mode
 * Input        : p_req: fei_openflow_tunnel_decap_mode_req_t*            
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_tunnel_update_decap_mode(fei_openflow_tunnel_decap_mode_req_t* p_req)
{
    uint8 decap_mode;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    decap_mode = p_req->decap_mode;

    if (HSRV_OPENFLOW_TUNNEL_PORT_NUM > HSRV_OPENFLOW_TUNNEL_ZERO)
    {
        return HSRV_E_NONE;
    }

    if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE == decap_mode)
    {
        return HSRV_E_NONE;
    }

    if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE_1 == decap_mode)
    {
        HSRV_OPENFLOW_TUNNEL_DECAP_MODE = HSRV_OPENFLOW_TUNNEL_DECAP_MODE_1;
    }
    else if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2 == decap_mode)
    {
        HSRV_OPENFLOW_TUNNEL_DECAP_MODE = HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_update_decap_mode());

    return HSRV_E_NONE;
}

/* Added by weizj for tunnel bug 33958 */
/*************************************************************************************************
 * Name         : fei_api_openflow_tunnel_update_vtep_ip
 * Purpose      : Hsrv layer openflow tunnel update vtep ip
 * Input        : p_req: fei_openflow_vtep_ip_req_t*            
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_tunnel_update_vtep_ip(fei_openflow_vtep_ip_req_t* p_req)
{
    uint32 vtep_ip;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    vtep_ip = p_req->ip_addr;

    if (HSRV_OPENFLOW_TUNNEL_PORT_NUM > HSRV_OPENFLOW_TUNNEL_ZERO)
    {
        return HSRV_E_NONE;
    }

    HSRV_OPENFLOW_TUNNEL_VTEP_IP = vtep_ip;

    return HSRV_E_NONE;
}
/* End by weizj for tunnel */

/*************************************************************************************************
 * Name         : fei_api_openflow_tunnel_add_port
 * Purpose      : Hsrv layer openflow tunnel add tunnel port   
 * Input        : p_req: fei_openflow_add_tunnel_req_t*               
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
fei_api_openflow_tunnel_add_port(fei_openflow_add_tunnel_req_t* p_req)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_req);

    if (HSRV_OPENFLOW_TUNNEL_PORT_NUM > HSRV_OPENFLOW_TUNNEL_PORT_MAX)
    {
        HSRV_OPENFLOW_LOG_WARN("Openflow tunnel port exceed the max size");
        return HSRV_E_EXCEED_MAX_SIZE;
    }

    /* alloc memery */
    p_tnl_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_tunnel_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tnl_info);
    sal_memset(p_tnl_info, 0, sizeof(hsrv_openflow_tunnel_info_t));

    p_tnl_info->ifindex = p_req->tunnel_info.tunnel_ifindex;
    sal_memcpy(&p_tnl_info->tunnel_info, &p_req->tunnel_info, sizeof(glb_openflow_tunnel_info_t));
    if(p_tnl_info->tunnel_info.is_dynamic)
    {
        hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &(p_tnl_info->tunnel_info));
        hsrv_neighbor_arp_update_by_dynamic_tunnel_remote_ip(p_tnl_info->tunnel_info.remote_ip, p_tnl_info->tunnel_info.direct_connection_ifname, 1);
    }
 
    ctclib_hash_get(HSRV_OPENFLOW_TUNNEL_INFO_HASH, p_tnl_info, ctclib_hash_alloc_intern);
    HSRV_OPENFLOW_TUNNEL_PORT_NUM ++;

    // DEPRECATED. HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_l3if_enable(p_req->tunnel_info.bond_port_ifindex));

    p_tnl_info = NULL;

    return HSRV_E_NONE; 
}

/*************************************************************************************************
 * Name         : fei_api_openflow_tunnel_del_port
 * Purpose      : Hsrv layer openflow tunnel delete tunnel port
 * Input        : p_req: fei_openflow_del_tunnel_req_t*             
 * Output       : N/A                
 * Return       : HSRV_E_NONE, HSRV_E_ENTRY_NOT_EXIST
**************************************************************************************************/
int32
fei_api_openflow_tunnel_delete_port(fei_openflow_del_tunnel_req_t* p_req)
{
    hsrv_openflow_tunnel_info_t lkp_info;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_req);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_tunnel_info_t));
    lkp_info.ifindex = p_req->ifindex;

    /* lookup hash */
    p_tnl_info = ctclib_hash_lookup(HSRV_OPENFLOW_TUNNEL_INFO_HASH, &lkp_info);
    if (NULL == p_tnl_info)
    {
        HSRV_OPENFLOW_LOG_WARN("Tunnel port:%d not in hsrv db", p_req->ifindex);
        return HSRV_E_ENTRY_NOT_EXIST; 
    }

    if(p_tnl_info->tunnel_info.is_dynamic)
    {
        hsrv_neighbor_arp_update_by_dynamic_tunnel_remote_ip(p_tnl_info->tunnel_info.remote_ip, p_tnl_info->tunnel_info.direct_connection_ifname, 0);
    }

    /* release hash data */
    ctclib_hash_release(HSRV_OPENFLOW_TUNNEL_INFO_HASH, p_tnl_info);

    // DEPRECATED. HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_l3if_disable(p_tnl_info->tunnel_info.bond_port_ifindex));

    /* free memery */
    HSRV_OPENFLOW_TUNNEL_FREE(p_tnl_info);
    HSRV_OPENFLOW_TUNNEL_PORT_NUM --;

    p_tnl_info = NULL;

    return HSRV_E_NONE;
}

#define ____TUNNEL_MODULE_APIs____
/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_add_decap_flow
 * Purpose      : Hsrv layer openflow tunnel add decap flow
 * Input        : p_flow: glb_openflow_flow_t*             
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_tunnel_add_decap_flow(glb_openflow_flow_t* p_flow)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    uint16 ofport = 0;
    uint32 tun_id = 0;
    uint16 fid = 0;
    uint16 metadata = 0;
    
    HSRV_OPENFLOW_PTR_CHECK(p_flow);

    /* Modifed by weizj for tunnel bug 33692 */
    if (GLB_OF_PORT_TYPE_TUNNEL != p_flow->key.in_port.type && !p_flow->key.tnl.tun_id)
    {
        return HSRV_E_NONE;
    }

    ofport = p_flow->key.in_port.ofport;
    tun_id = p_flow->key.tnl.tun_id;

    /* get tunnel info */
    if (ofport)
    {
        _hsrv_openflow_tunnel_lookup_info(p_flow->key.tnl.ifindex, &p_tnl_info);
        if (!p_tnl_info)
        {
            HSRV_OPENFLOW_LOG_ERR("Openflow tunnel not find tunnel info:%d", p_flow->key.tnl.ifindex);
            return HSRV_E_ENTRY_NOT_EXIST;
        }

        p_flow->key.tnl.ipDa = p_tnl_info->tunnel_info.local_ip;
        p_flow->key.tnl.ipSa = p_tnl_info->tunnel_info.remote_ip;
        p_flow->key.tnl.type = p_tnl_info->tunnel_info.type;
    }
    
    /* get metadata */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_get_metadata(ofport, tun_id, &metadata));
    p_flow->key.tnl.metadata = metadata;

    /* get fid */
    if (tun_id)
    {
        /* Modifed by weizj for tunnel bug 33434 */
        if (p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_get_fid(tun_id, &fid));
        }
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_add_decap_flow(fid, &p_flow->key.tnl));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_del_decap_flow
 * Purpose      : Hsrv layer openflow tunnel delete decap flow
 * Input        : p_flow: glb_openflow_flow_t*             
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_tunnel_del_decap_flow(glb_openflow_flow_t* p_flow)
{
    uint16 metadata;
    uint16 ofport;
    uint32 tun_id;

    HSRV_OPENFLOW_PTR_CHECK(p_flow);

    metadata = p_flow->key.tnl.metadata;
    ofport = p_flow->key.in_port.ofport;
    tun_id = p_flow->key.tnl.tun_id;

    if (HSRV_OPENFLOW_TUNNEL_ZERO >= metadata)
    {
        return HSRV_E_NONE;
    }
    
    /* delete metadata */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_del_metadata(ofport, tun_id));

    /* delete fid */
    if (tun_id)
    {
        /* Modifed by weizj for tunnel bug 33434 */
        if (p_flow->key.tnl.type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            p_flow->key.tnl.type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_del_fid(tun_id));
        }
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_del_decap_flow(metadata));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_alloc_tunnel_nh
 * Purpose      : hsrv layer openflow tunnel alloc tunnel nexthop          
 * Input        : p_param: hsrv_ofnh_tunnel_param_t*, p_mem_info: hsrv_of_nh_info_t*
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_tunnel_alloc_tunnel_nh(hsrv_ofnh_tunnel_param_t* p_param, hsrv_of_nh_info_t* p_mem_info)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
 #ifdef V580_OPENFLOW   
    mac_addr_t port_mac;
 #endif
    uint16 gport;
    uint16 fid;
    uint32 tnl_ifindex;
    uint32 bind_ifindex;

    HSRV_OPENFLOW_PTR_CHECK(p_param);
    HSRV_OPENFLOW_PTR_CHECK(p_mem_info);

    /* get tunnel info */
    tnl_ifindex = p_param->tnl.tunnel_ifindex;
    _hsrv_openflow_tunnel_lookup_info(tnl_ifindex, &p_tnl_info);
    if (!p_tnl_info)
    {
        HSRV_OPENFLOW_LOG_ERR("Not find tunnel info:%d", tnl_ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /* get bind port gport */
    bind_ifindex = p_tnl_info->tunnel_info.bond_port_ifindex;
    if (hsrv_openflow_port_is_bond(bind_ifindex))
    {
        hsrv_ifdb_get_agg_portid(bind_ifindex, &gport);
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(bind_ifindex, &gport));
    }

    /* Modifed by weizj for tunnel bug 33649 */
    p_param->tnl.gport = gport;
    if (GLB_MAX_VLAN_ID == p_tnl_info->tunnel_info.vlan_id)
    {
        p_param->tnl.vlan_id = GLB_MIN_VLAN_ID;
    }
    else
    {
        p_param->tnl.vlan_id = p_tnl_info->tunnel_info.vlan_id;
    }
    
    p_param->tnl.ttl = HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TTL;
    p_param->tnl.dscp_or_tos = HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TOS;

    /* Added by weizj for tunnel bug 33408 */
    if (GLB_OFPACT_FLD_ISSET(p_param->p_action_combo->flag, STRIP_SVLAN))
    {
        p_param->tnl.strip_svlan = 1;
    }
    if(GLB_OFPACT_FLD_ISSET(p_param->p_action_combo->flag, MACSA))
    {
        sal_memcpy(p_param->tnl.inner_mac_sa, p_param->p_action_combo->mac_sa, MAC_ADDR_LEN);
    }
    if(GLB_OFPACT_FLD_ISSET(p_param->p_action_combo->flag, MACDA))
    {
        sal_memcpy(p_param->tnl.inner_mac_da, p_param->p_action_combo->mac_da, MAC_ADDR_LEN);
    }
    
    p_param->tnl.tunnel_type = p_tnl_info->tunnel_info.type;
    p_param->tnl.inner.tunnel_key = p_param->p_action_combo->tunnel_id;
    p_param->tnl.inner.protocol_type = HSRV_OPENFLOW_TUNNEL_PROTOCOL_TYPE_TRANSPARENT_BRIDGING;
    
    p_param->tnl.ip_sa.ipv4 = p_tnl_info->tunnel_info.local_ip;
    p_param->tnl.ip_da.ipv4 = p_tnl_info->tunnel_info.remote_ip;
    sal_memcpy(p_param->tnl.mac_da, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
#ifdef V580_OPENFLOW
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_system_routemac(&port_mac));

    sal_memcpy(p_param->tnl.mac_sa, port_mac, sizeof(mac_addr_t));
#endif
    if (p_param->tnl.inner.tunnel_key)
    {
        if (p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_get_fid(p_param->tnl.inner.tunnel_key, &fid));
            p_param->tnl.fid = fid;
        }
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_ip_tunnel_nh(&p_param->tnl, p_mem_info));

    p_tnl_info = NULL;
  
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_release_fid
 * Purpose      : Hsrv layer openflow tunnel release tunnel fid mapping         
 * Input        : tun_id: uint32      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_tunnel_release_fid(uint32 tun_id)
{
    if (0 == tun_id)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_del_fid(tun_id));

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_get_info_by_metadata
 * Purpose      : Hsrv layer openflow tunnel get ofport by metadata     
 * Input        : tun_id: uint32      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id)
{
    hsrv_openflow_tunnel_metadata_t* p_info = NULL;
    uint16 metadata;

    metadata = meta_data & 0xFFFF; 
    _hsrv_openflow_tunnel_lookup_ofport(metadata, &p_info);
    if (p_info)
    {
        *p_ofport = p_info->ofport;
        *p_tun_id = p_info->tun_id;
    }

    p_info = NULL;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_port_is_tunnel
 * Purpose      : Hsrv layer openflow port is tunnel port
 * Input        : ifindex: uint32             
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Added by weizj for group bug 37038 */
bool
hsrv_openflow_port_is_tunnel(uint32 ifindex)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    _hsrv_openflow_tunnel_lookup_info(ifindex, &p_tnl_info);
    if (!p_tnl_info)
    {
        return FALSE;
    }

    return TRUE;
}

bool
hsrv_openflow_tunnel_get_link_status(uint32 ifindex)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    _hsrv_openflow_tunnel_lookup_info(ifindex, &p_tnl_info);

    if(p_tnl_info)
    {
        return hsrv_openflow_phyport_get_link_status(ifindex);
    }

    return FALSE;
}

int32
hsrv_openflow_tunnel_set_bond_slave_decap_en(uint32 ifindex, glb_openflow_tunnel_port_type_t type, uint32 enable)
{
    fei_openflow_port_decap_req_t pst_req;

    sal_memset(&pst_req, 0, sizeof(fei_openflow_port_decap_req_t));
    pst_req.ifindex = ifindex;
    pst_req.enable = enable;
    pst_req.type = type;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_port_decap_en(&pst_req));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_packet_out_del_tunnel_nhid(uint32 nh_id)
{
    hsrv_ofact_ip_tunnel_nh_info_t tnl_info;
    sal_memset(&tnl_info, 0x0, sizeof(hsrv_ofact_ip_tunnel_nh_info_t));
    tnl_info.nh_offset.nhid = nh_id;
    hsrv_openflow_nexthop_remove_ip_tunnel_nh(&tnl_info);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_pacet_out_get_tunnel_type(glb_openflow_packet_out_param_t* p_param, glb_openflow_tunnel_port_type_t* p_type)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    
    _hsrv_openflow_tunnel_lookup_info(p_param->ifindex, &p_tnl_info);
    if (!p_tnl_info)
    {
        HSRV_OPENFLOW_LOG_ERR("Not find tunnel info:%d", p_param->ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    *p_type = p_tnl_info->tunnel_info.type;

    return 0;
}

int32
hsrv_openflow_tunnel_packet_out_get_tunnel_nhid(glb_openflow_packet_out_param_t* p_param, uint32* p_nh_id, uint32* p_is_exist, uint16* p_phy_gport)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_ip_nh_tunnel_info_t nh_param;
    hsrv_ofact_ip_tunnel_nh_info_t* p_tnl_lookup_info = NULL;
    hsrv_ofact_ip_tunnel_nh_info_t tnl_creat_info;
    uint32 bind_ifindex = 0;
    uint16 gport = 0;
    uint32 index = 0;
    uint16 fid = 0;

    hsrv_of_nh_offset_t nh_offset;

    sal_memset(&nh_param, 0x0, sizeof(hsrv_openflow_ip_nh_tunnel_info_t));
    sal_memset(&nh_offset, 0x0, sizeof(hsrv_of_nh_offset_t));
    sal_memset(&tnl_creat_info, 0x0, sizeof(hsrv_ofact_ip_tunnel_nh_info_t));
    _hsrv_openflow_tunnel_lookup_info(p_param->ifindex, &p_tnl_info);
    if (!p_tnl_info)
    {
        HSRV_OPENFLOW_LOG_ERR("Not find tunnel info:%d", p_param->ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    if(p_tnl_info->tunnel_info.is_dynamic)
    {
        if(p_tnl_info->tunnel_info.multipath_cnt > 1)
        {
            for(index = 0; index < p_tnl_info->tunnel_info.multipath_cnt; index ++)
            {
                if(p_tnl_info->tunnel_info.multipath[index].arp_is_ready)
                {
                    bind_ifindex = p_tnl_info->tunnel_info.multipath[index].bond_port_ifindex;
                    nh_param.vlan_id = p_tnl_info->tunnel_info.multipath[index].vlan_id;
                    sal_memcpy(nh_param.mac_da, p_tnl_info->tunnel_info.multipath[index].nexthop_mac, sizeof(mac_addr_t));
                }
            }
        }
        else if(p_tnl_info->tunnel_info.multipath_cnt == 1)
        {
            bind_ifindex = p_tnl_info->tunnel_info.bond_port_ifindex;
            nh_param.vlan_id = p_tnl_info->tunnel_info.vlan_id;
            sal_memcpy(nh_param.mac_da, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
        }
    }
    else
    {
        bind_ifindex = p_tnl_info->tunnel_info.bond_port_ifindex;
        if (GLB_MAX_VLAN_ID == p_tnl_info->tunnel_info.vlan_id)
        {
            nh_param.vlan_id = GLB_MIN_VLAN_ID;
        }
        else
        {
            nh_param.vlan_id = p_tnl_info->tunnel_info.vlan_id;
        }
        sal_memcpy(nh_param.mac_da, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
    }
        /* get bind port gport */
    
    if (hsrv_openflow_port_is_bond(bind_ifindex))
    {
        hsrv_ifdb_get_agg_portid(bind_ifindex, &gport);
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(bind_ifindex, &gport));
    }

    nh_param.gport = gport;
    *p_phy_gport = gport;

    nh_param.ttl = HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TTL;
    nh_param.dscp_or_tos = HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TOS;
    
    nh_param.tunnel_type = p_tnl_info->tunnel_info.type;
    nh_param.inner.tunnel_key = p_param->tunnel_id;
    nh_param.inner.protocol_type = HSRV_OPENFLOW_TUNNEL_PROTOCOL_TYPE_TRANSPARENT_BRIDGING;
    
    nh_param.ip_sa.ipv4 = p_tnl_info->tunnel_info.local_ip;
    nh_param.ip_da.ipv4 = p_tnl_info->tunnel_info.remote_ip;
    nh_param.tunnel_ifindex = p_tnl_info->ifindex;

     if (nh_param.inner.tunnel_key)
     {
        if (p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            p_tnl_info->tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_get_fid(nh_param.inner.tunnel_key, &fid));
           nh_param.fid = fid;
        }
     }

     hsrv_openflow_nexthop_lookup_ip_tunnel_nh(&nh_param, &p_tnl_lookup_info);
     if (p_tnl_lookup_info)
     {
        *p_is_exist = 1;
        *p_nh_id = p_tnl_lookup_info->nh_offset.nhid;
        return 0;
     }

    if (0 < nh_param.inner.tunnel_key)
    {
        nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID;
    }
    else
    {
        nh_offset.nh_type = HSRV_OF_NH_TYPE_NH_IP_TUNNEL;
    }
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_offset_alloc(&nh_offset));
    sal_memcpy(&tnl_creat_info.param, &nh_param, sizeof(hsrv_openflow_ip_nh_tunnel_info_t));
    sal_memcpy(&tnl_creat_info.nh_offset, &nh_offset, sizeof(hsrv_of_nh_offset_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_send_ip_tunnel_nh(&tnl_creat_info));

    *p_nh_id =  tnl_creat_info.nh_offset.nhid;
    *p_is_exist = 0;

    return 0;
}

int32
hsrv_openflow_set_vxlan_destport(uint32 is_set, uint32 destport)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowVxlanDestportReq_t* p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowVxlanDestport;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);
    
    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowVxlanDestport;
    p_req->isSet = is_set;
    p_req->destport = destport;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    p_req = NULL;

    return HSRV_E_NONE;
}

//DEPRECATED.
#if 0
int32
hsrv_openflow_tunnel_set_bond_slave_l3if_property(uint16 gport, uint32 enable)
{
    if (enable)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_set_l3if(gport));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_tunnel_unset_l3if(gport));
    }
    
    return HSRV_E_NONE;
}
#endif

#define ____TUNNEL_DEBUG____
/* Added by weizj for tunnel bug 33662 */
/*************************************************************************************************
 * Name         : _hsrv_openflow_port_quick_sort
 * Purpose      : Hsrv layer openflow tunnel port quick sort    
 * Input        : port_data: uint32* len: int
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void
_hsrv_openflow_port_quick_sort(uint32 *port_data, int32 left_len, int32 right_len)
{
    int32 left = left_len;
    int32 right = right_len;
    uint32 key = port_data[left_len];

    if (left >= right)
    {
        return;
    }

    while(left < right)
    {
        while(left < right && key <= port_data[right])
        {
            right --;
        }
        port_data[left] = port_data[right];

        while(left < right && key >= port_data[left])
        {
            left ++;
        }
        port_data[right] = port_data[left];
    }
    port_data[left] = key;
    _hsrv_openflow_port_quick_sort(port_data, left_len, left - 1);
    _hsrv_openflow_port_quick_sort(port_data, left + 1, right_len);
}

/* Added by weizj for tunnel bug 33662 */
/*************************************************************************************************
 * Name         : _hsrv_openflow_find_port_hash
 * Purpose      : Hsrv layer openflow tunnel find port info         
 * Input        : phb: ctclib_hash_backet_t*, user_data: void*
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void
_hsrv_openflow_find_port_hash(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl = (hsrv_openflow_tunnel_info_t*)phb->data;
    hsrv_openflow_tunnel_debug_data_t* p_data = user_data;

    p_data->ifindex[p_data->i] = p_tnl->ifindex;
    p_data->i ++;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_show_port_hash
 * Purpose      : Hsrv layer openflow tunnel find port info         
 * Input        : p_tnl: hsrv_openflow_tunnel_info_t*, index: int32 fp: FILE*
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Modifed by weizj for tunnel bug 33523 */
/* Modifed by weizj for tunnel bug 33662 */
static void
_hsrv_openflow_show_port_hash(hsrv_openflow_tunnel_info_t* p_tnl, int32 index, FILE *fp)
{
#define TEMP_STR_LEN 32
    char* type = NULL;
    struct in_addr ip_addr;
    char mac_str[TEMP_STR_LEN] = {0};
    char ip_remote[TEMP_STR_LEN/2];
    char ip_source[TEMP_STR_LEN/2];
    char vlan_id_str[TEMP_STR_LEN/2] = {0};
    uint8 link_status = 0;

    switch (p_tnl->tunnel_info.type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            type = "l2gre";
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            type = "nvgre";
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            type = "vxlan";
            break;
        default:
            type = "None";
    }
    
    sal_memset(&ip_addr, 0, sizeof(struct in_addr));
    #ifdef USW
    ip_addr.s_addr = htonl(p_tnl->tunnel_info.remote_ip);
    #else
    ip_addr.s_addr = p_tnl->tunnel_info.remote_ip;
    #endif
    inet_ntop (AF_INET, &ip_addr.s_addr, ip_remote, sizeof(ip_remote));
    sal_memset(&ip_addr, 0, sizeof(struct in_addr));
    #ifdef USW
    ip_addr.s_addr = htonl(p_tnl->tunnel_info.local_ip);
    #else
    ip_addr.s_addr = p_tnl->tunnel_info.local_ip;
    #endif
    inet_ntop (AF_INET, &ip_addr.s_addr, ip_source, sizeof(ip_source));

    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            p_tnl->tunnel_info.nexthop_mac[0], p_tnl->tunnel_info.nexthop_mac[1],
            p_tnl->tunnel_info.nexthop_mac[2], p_tnl->tunnel_info.nexthop_mac[3],
            p_tnl->tunnel_info.nexthop_mac[4], p_tnl->tunnel_info.nexthop_mac[5]);

    if (GLB_MAX_VLAN_ID == p_tnl->tunnel_info.vlan_id)
    {
        sprintf(vlan_id_str, "None");
    }
    else
    {
        sprintf(vlan_id_str, "%u", p_tnl->tunnel_info.vlan_id);
    }

    link_status = hsrv_openflow_port_get_link_status(p_tnl->tunnel_info.bond_port_ifindex);

    sal_fprintf(fp, "%5d %5s %8s %4d %9s %15s %15s %18s %7s %4s\n",
                index, type, p_tnl->tunnel_info.tunnel_name,
                p_tnl->tunnel_info.ofport, p_tnl->tunnel_info.bind_port_name,ip_source,
                ip_remote, mac_str, vlan_id_str, link_status ? "UP" : "DOWN");

#undef TEMP_STR_LEN
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_show_fid_hash
 * Purpose      : Hsrv layer openflow tunnel show fid info         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void
_hsrv_openflow_show_fid_hash(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_fid_t* p_fid = (hsrv_openflow_tunnel_fid_t*)phb->data;
    hsrv_openflow_tunnel_debug_data_t* p_data = user_data;

    sal_fprintf(p_data->fp, "index: %-3d  tun_id: %d  fid: %d  ref_cnt: %d\n",
        p_data->i,
        p_fid->tun_id,
        p_fid->fid,
        p_fid->ref_cnt);

    p_data->i ++;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_show_metadata_hash
 * Purpose      : Hsrv layer openflow tunnel show metadata info         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void
_hsrv_openflow_show_metadata_hash(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_metadata_t* p_metadata = (hsrv_openflow_tunnel_metadata_t*)phb->data;
    hsrv_openflow_tunnel_debug_data_t* p_data = user_data;

    sal_fprintf(p_data->fp, "index: %-3d  ofport: %d  tun_id: %d  metadata: %d  ref_cnt: %d\n",
        p_data->i,
        p_metadata->ofport,
        p_metadata->tun_id,
        p_metadata->metadata,
        p_metadata->ref_cnt);

    p_data->i ++;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_show_tunnel_brief
 * Purpose      : Hsrv layer openflow tunnel show brief         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_show_tunnel_brief(FILE* fp)
{
    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    sal_fprintf(fp, "----------------- HSRV OPENFLOW TUNNEL -----------------\n");
    sal_fprintf(fp, "Support Port Max        : %d\n", HSRV_OPENFLOW_TUNNEL_PORT_MAX);
    sal_fprintf(fp, "Support Fid Stat        : 65535\n");
    sal_fprintf(fp, "Support Matadata Stat   : 65535\n");
    sal_fprintf(fp, "Support Decap-flow Stat : 65535\n");

    sal_fprintf(fp, "\nCreate Port      : %d\n", HSRV_OPENFLOW_TUNNEL_PORT_NUM);
    sal_fprintf(fp, "Alloc Fid        : %d\n", HSRV_OPENFLOW_TUNNEL_FID_NUM);
    sal_fprintf(fp, "Alloc Metadata   : %d\n", HSRV_OPENFLOW_TUNNEL_METADATA_NUM);
    sal_fprintf(fp, "Alloc Decap-flow : %d\n", HSRV_OPENFLOW_TUNNEL_FLOW_NUM);

    if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE)
    {
         sal_fprintf(fp, "Decap mode       : ipda + ipsa + vni\n");
    }
    else
    {
         sal_fprintf(fp, "Decap mode       : ipda + vni\n");
    }
    sal_fprintf(fp, "--------------------------------------------------------\n");

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_show_tunnel_port_all
 * Purpose      : Hsrv layer openflow tunnel show all of port info         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
/* Modifed by weizj for tunnel bug 33662 */
/* Modifed by weizj for tunnel bug 33958 */
int32
hsrv_openflow_show_tunnel_port_all(FILE *fp)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_tunnel_debug_data_t data;
    struct in_addr ip_addr;
    char ip_str[16];
    int32 i = 0;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    sal_memset(&data, 0, sizeof(hsrv_openflow_tunnel_debug_data_t));
    data.fp = fp;
    data.i  = 0;

    sal_fprintf(fp, "The Maximum of tunnel ports is %d, currently %d tunnel ports is valid.\n\n", 
                HSRV_OPENFLOW_TUNNEL_PORT_MAX, HSRV_OPENFLOW_TUNNEL_PORT_NUM);

    /* modified by liwh for bug 48883, 2019-02-26 */
    sal_fprintf(fp, "Default tunnel_type of bind_port is vxlan, if you want to use other type of tunnel, \n");
    sal_fprintf(fp, "modify tunnel mode on the bind_port.\n\n");
    /* liwh end */

    if (HSRV_OPENFLOW_TUNNEL_VTEP_IP != HSRV_OPENFLOW_TUNNEL_ZERO)
    {
        ip_addr.s_addr = HSRV_OPENFLOW_TUNNEL_VTEP_IP;
        inet_ntop (AF_INET, &ip_addr.s_addr, ip_str, sizeof(ip_str));
        //sal_fprintf(fp, "%s: %s\n", "Local vtep ip", ip_str);
    }
    else
    {
        //sal_fprintf(fp, "%s: %s\n", "Local vtep ip", "None");
    }

    if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE_1 == HSRV_OPENFLOW_TUNNEL_DECAP_MODE)
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "ipda + vni");
    }
    else if(HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2 == HSRV_OPENFLOW_TUNNEL_DECAP_MODE)
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "ipda + ipsa + vni [default]");
    }
    else
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "None");
    }

    sal_fprintf(fp, "%5s %5s %8s %4s %9s %15s %15s %18s %7s %4s\n",
                    "index", " type", "name  ", "port", "bind_port", "source_ip",
                    "remote_ip   ", "remote_mac    ", "vlan_id", "link");
    sal_fprintf(fp, "----- ----- -------- ---- --------- --------------- --------------- ------------------ ------- ----\n");

    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, _hsrv_openflow_find_port_hash, &data);

    _hsrv_openflow_port_quick_sort(data.ifindex, 0, data.i - 1);

    if (!data.i)
    {
        return HSRV_E_NONE;
    }

    for (i = 0; i < data.i; i++)
    {
        _hsrv_openflow_tunnel_lookup_info(data.ifindex[i], &p_tnl_info);
        if (p_tnl_info)
        {
            _hsrv_openflow_show_port_hash(p_tnl_info, i + 1, fp);
        }
    }

    return HSRV_E_NONE;
}

static void
_hsrv_openflow_second_show_port_hash(hsrv_openflow_tunnel_info_t* p_tnl, int32 index, FILE *fp)
{
#define TEMP_STR_LEN 32
    char* type = NULL;
    struct in_addr ip_addr;
    char mac_str[TEMP_STR_LEN] = {0};
    char ip_remote[TEMP_STR_LEN/2];
    char ip_source[TEMP_STR_LEN/2];
    char vlan_id_str[TEMP_STR_LEN/2] = {0};
    uint8 link_status = 0;

    switch (p_tnl->tunnel_info.type)
    {
        case GLB_OPENFLOW_TUNNEL_TYPE_L2GRE:
            type = "l2gre";
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_NVGRE:
            type = "nvgre";
            break;
        case GLB_OPENFLOW_TUNNEL_TYPE_VXLAN:
            type = "vxlan";
            break;
        default:
            type = "None";
    }
    
    sal_memset(&ip_addr, 0, sizeof(struct in_addr));
    #ifdef USW
    ip_addr.s_addr = htonl(p_tnl->tunnel_info.remote_ip);
    #else
    ip_addr.s_addr = p_tnl->tunnel_info.remote_ip;
    #endif
    inet_ntop (AF_INET, &ip_addr.s_addr, ip_remote, sizeof(ip_remote));
    sal_memset(&ip_addr, 0, sizeof(struct in_addr));

    #ifdef USW
    ip_addr.s_addr = htonl(p_tnl->tunnel_info.local_ip);
    #else
    ip_addr.s_addr = p_tnl->tunnel_info.local_ip;
    #endif
    inet_ntop (AF_INET, &ip_addr.s_addr, ip_source, sizeof(ip_source));

    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            p_tnl->tunnel_info.nexthop_mac[0], p_tnl->tunnel_info.nexthop_mac[1],
            p_tnl->tunnel_info.nexthop_mac[2], p_tnl->tunnel_info.nexthop_mac[3],
            p_tnl->tunnel_info.nexthop_mac[4], p_tnl->tunnel_info.nexthop_mac[5]);

    if (GLB_MAX_VLAN_ID == p_tnl->tunnel_info.vlan_id)
    {
        sprintf(vlan_id_str, "None");
    }
    else
    {
        sprintf(vlan_id_str, "%u", p_tnl->tunnel_info.vlan_id);
    }

    if(p_tnl->tunnel_info.is_dynamic)
    {
        link_status = p_tnl->tunnel_info.interface_status;
    }
    else
    {
        link_status = hsrv_openflow_port_get_link_status(p_tnl->tunnel_info.bond_port_ifindex);
    }
    if(index)
    {
        sal_fprintf(fp, "index %d\n", index);
    }
    sal_fprintf(fp, "    type:            %s\n", type);
    sal_fprintf(fp, "    name:            %s\n", p_tnl->tunnel_info.tunnel_name);
    sal_fprintf(fp, "    port:            %d\n", p_tnl->tunnel_info.ofport);
    sal_fprintf(fp, "    source-ip:       %s\n", ip_source);
    sal_fprintf(fp, "    remote_ip:       %s\n", ip_remote);
    sal_fprintf(fp, "    link:            %s\n", link_status ? "UP" : "DOWN");
    sal_fprintf(fp, "    dynamic:         %s\n", p_tnl->tunnel_info.is_dynamic ? "TRUE" : "FALSE");
    if(!p_tnl->tunnel_info.is_dynamic)
    {
        sal_fprintf(fp, "    bind_port:       %s\n", p_tnl->tunnel_info.bind_port_name);
        sal_fprintf(fp, "    remote_mac:      %s\n", mac_str);
        sal_fprintf(fp, "    vlan_id:         %s\n", vlan_id_str);
    }
    sal_fprintf(fp, "----------------------------------------\n");
#undef TEMP_STR_LEN
}


int32
hsrv_openflow_second_show_tunnel_port_one(FILE *fp, char **argv)
{
    uint32 ifindex = 0;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    tbl_interface_t* p_interface = NULL;
    p_interface = tbl_interface_get_interface_by_name(argv[0]);
    if(NULL == p_interface)
    {
        sal_fprintf(fp, "%%%%This tunnel port is not exist!");
        return HSRV_E_NONE;
    }
    ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
    _hsrv_openflow_tunnel_lookup_info(ifindex, &p_tnl_info);
    if(p_tnl_info)
    {
        _hsrv_openflow_second_show_port_hash(p_tnl_info, 0, fp);
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_second_show_tunnel_port_all(FILE *fp, uint32 type)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_tunnel_debug_data_t data;
    struct in_addr ip_addr;
    char ip_str[16];
    int32 i = 0;
    uint32 src_port_is_set = 0;
    uint32 dest_port_is_set = 0;
    uint32 src_port = 0;
    uint32 dest_port = 0;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    sal_memset(&data, 0, sizeof(hsrv_openflow_tunnel_debug_data_t));
    data.fp = fp;
    data.i  = 0;

    sal_fprintf(fp, "The Maximum of tunnel ports is %d, currently %d tunnel ports is valid.\n\n", 
                HSRV_OPENFLOW_TUNNEL_PORT_MAX, HSRV_OPENFLOW_TUNNEL_PORT_NUM);

    /* modified by liwh for bug 48883, 2019-02-26 */
    sal_fprintf(fp, "Default tunnel_type of bind_port is vxlan, if you want to use other type of\n"); 
    sal_fprintf(fp, "tunnel, modify tunnel mode on the bind_port.\n\n");
    /* liwh end */

    hsrv_openflow_get_vxlan_src_port(&src_port_is_set, &src_port);
    hsrv_openflow_get_vxlan_dest_port(&dest_port_is_set, &dest_port);
    if(src_port_is_set)
    {
        sal_fprintf(fp, "Vxlan source port : %d\n", src_port);
    }
    else
    {
        sal_fprintf(fp, "Vxlan source port is dynamic\n");
    }

    if(dest_port_is_set)
    {
        sal_fprintf(fp, "Vxlan dest port : %d\n", dest_port);
    }
    else
    {
        sal_fprintf(fp, "Vxlan dest port is default: %d\n", dest_port);
    }
    sal_fprintf(fp, "\n");

    if (HSRV_OPENFLOW_TUNNEL_VTEP_IP != HSRV_OPENFLOW_TUNNEL_ZERO)
    {
        ip_addr.s_addr = HSRV_OPENFLOW_TUNNEL_VTEP_IP;
        inet_ntop (AF_INET, &ip_addr.s_addr, ip_str, sizeof(ip_str));
    }

    if (HSRV_OPENFLOW_TUNNEL_DECAP_MODE_1 == HSRV_OPENFLOW_TUNNEL_DECAP_MODE)
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "ipda + vni");
    }
    else if(HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2 == HSRV_OPENFLOW_TUNNEL_DECAP_MODE)
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "ipda + ipsa + vni [default]");
    }
    else
    {
        sal_fprintf(fp, "%s: %s\n", "Decap mode", "None");
    }
    
    sal_fprintf(fp, "----------------------------------------\n");
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, _hsrv_openflow_find_port_hash, &data);

    _hsrv_openflow_port_quick_sort(data.ifindex, 0, data.i - 1);

    if (!data.i)
    {
        return HSRV_E_NONE;
    }

    for (i = 0; i < data.i; i++)
    {
        _hsrv_openflow_tunnel_lookup_info(data.ifindex[i], &p_tnl_info);

        /* modified by liwh for bug 48966, 2019-02-26 */
        if ((GLB_OPENFLOW_TUNNEL_TYPE_ALL != type) && (p_tnl_info->tunnel_info.type != type))
        {
            continue;
        }
        /* liwh end */
        
        if (p_tnl_info)
        {
            _hsrv_openflow_second_show_port_hash(p_tnl_info, i + 1, fp);
        }
    }

    return HSRV_E_NONE;
}


/* Deleted by weizj for tunnel bug 33523 */

/*************************************************************************************************
 * Name         : hsrv_openflow_show_tunnel_fid_all
 * Purpose      : Hsrv layer openflow tunnel show all of fid info         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_show_tunnel_fid_all(FILE *fp)
{
    hsrv_openflow_tunnel_debug_data_t data;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    data.fp = fp;

    sal_fprintf(fp, "--------------- HSRV OPENFLOW TUNNEL FID ---------------\n");
    data.i  = 1;
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_FID_HASH, _hsrv_openflow_show_fid_hash, &data);
    sal_fprintf(fp, "--------------------------------------------------------\n");

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_show_tunnel_metadata_all
 * Purpose      : Hsrv layer openflow tunnel show all of metadata info         
 * Input        : fp: FILE *      
 * Output       : N/A                
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_show_tunnel_metadata_all(FILE *fp)
{
    hsrv_openflow_tunnel_debug_data_t data;

    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);

    data.fp = fp;

    sal_fprintf(fp, "------------ HSRV OPENFLOW TUNNEL METADATA ------------\n");
    data.i  = 1;
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_METADATA_HASH, _hsrv_openflow_show_metadata_hash, &data);
    sal_fprintf(fp, "-------------------------------------------------------\n");

    return HSRV_E_NONE;
}

#define ____TUNNEL_INIT___
/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_start
 * Purpose      : hsrv openflow tunnel init module           
 * Input        : N/A               
 * Output       : N/A                
 * Return       : HSRV_E_NONE
**************************************************************************************************/
int32
hsrv_openflow_tunnel_start(void)
{
    g_pst_openflow_tunnel_master = HSRV_OPENFLOW_TUNNEL_CALLOC(sizeof(hsrv_openflow_tunnel_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_tunnel_master);

    HSRV_OPENFLOW_TUNNEL_INFO_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_info_hash_make,
        _hsrv_openflow_tunnel_info_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_INFO_HASH);

    HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH = ctclib_hash_create(_hsrv_openflow_dynamic_tunnel_info_hash_make,
        _hsrv_openflow_dynamic_tunnel_info_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH);

    HSRV_OPENFLOW_TUNNEL_METADATA_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_metadata_hash_make,
        _hsrv_openflow_tunnel_metadata_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_INFO_HASH);

    HSRV_OPENFLOW_TUNNEL_FID_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_fid_hash_make,
        _hsrv_openflow_tunnel_fid_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_FID_HASH);

    HSRV_OPENFLOW_TUNNEL_FLOW_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_flow_hash_make,
        _hsrv_openflow_tunnel_flow_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_FLOW_HASH);

// DEPRECATED.
#if 0
    HSRV_OPENFLOW_TUNNEL_L3IF_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_l3if_hash_make,
        _hsrv_openflow_tunnel_l3if_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_L3IF_HASH);
#endif

    HSRV_OPENFLOW_TUNNEL_OFPORT_HASH = ctclib_hash_create(_hsrv_openflow_tunnel_ofport_hash_make,
        _hsrv_openflow_tunnel_ofport_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TUNNEL_OFPORT_HASH);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_tunnel_init
 * Purpose      : hsrv openflow tunnel init module           
 * Input        : p_system_info: hsrv_system_spec_info_t                
 * Output       : N/A                
 * Return       : HSRV_E_NONE
**************************************************************************************************/
int32
hsrv_openflow_tunnel_init(hsrv_system_spec_info_t* p_system_info)
{
    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_tunnel_master);
 
    /* init */
    HSRV_OPENFLOW_TUNNEL_DECAP_MODE   = HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2;
    
    HSRV_OPENFLOW_TUNNEL_PORT_NUM     = HSRV_OPENFLOW_TUNNEL_ZERO;
    HSRV_OPENFLOW_TUNNEL_FID_NUM      = HSRV_OPENFLOW_TUNNEL_ZERO;
    HSRV_OPENFLOW_TUNNEL_PORT_MAX     = HSRV_OPENFLOW_TUNNEL_PORT_MAX_SIZE;
    HSRV_OPENFLOW_TUNNEL_METADATA_NUM = HSRV_OPENFLOW_TUNNEL_ZERO;

    /* init fid and metadata pool */
    HSRV_OPENFLOW_TUNNEL_METADATA_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OPENFLOW_TUNNEL_METADATA_OPF,
                            0,
                            1,
                            HSRV_OPENFLOW_TUNNEL_METADATA_MAX);

    HSRV_OPENFLOW_TUNNEL_FID_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset (HSRV_OPENFLOW_TUNNEL_FID_OPF,
                            0,
                            1,
                            HSRV_OPENFLOW_TUNNEL_FID_MAX);
    
    return HSRV_E_NONE;
}

