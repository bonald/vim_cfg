
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_flow.h"
#include "hsrv_openflow_ttp_flow_db.h"
#include "ctc_ttp_drv_stats.h"

uint32 g_pst_ttp_bridge_flow_table_miss_group_id = 0;
uint32 g_pst_ttp_bridge_flow_table_miss_nhid = 0;
uint32 g_pst_ttp_bridge_flow_table_miss_stats_id = 0;

uint32 g_pst_ttp_unicast_flow_table_miss_group_id = 0;
uint32 g_pst_ttp_unicast_flow_table_miss_nhid = 0;
uint32 g_pst_ttp_unicast_flow_table_miss_stats_id = 0;

uint32 g_pst_ttp_vlan_mapping_flow_table_miss_stats_id = 0;

#define _____EXTERNAL_APIs_____

hsrv_openflow_ttp_flow_t*
hsrv_openflow_get_flow_info(uint32 flow_id)
{
    hsrv_openflow_ttp_flow_t* p_flow = NULL;

    hsrv_openflow_flowdb_lookup_flow(flow_id, &p_flow);
    
    return p_flow;
}

int32
hsrv_openflow_lookup_metadata_mapping(uint64 metadata, metadata_mapping_t** pp_metadata)
{
    hsrv_openflow_flowdb_lookup_metadata_mapping(metadata, pp_metadata);
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id)
{
    return HSRV_E_NONE;
}
#define _____INTERNAL_APIs_____

static int32
_hsrv_openflow_add_metadata_mapping(uint64 metadata, metadata_type_t type)
{
    metadata_mapping_t  *p_metadata = NULL;
    uint16 fid = 0;
    uint16 vrf_id = 0;
    
    hsrv_openflow_flowdb_lookup_metadata_mapping(metadata, &p_metadata);
    if (p_metadata)
    {
        p_metadata->ref_cnt ++;
        return HSRV_E_NONE;
    }
    p_metadata = HSRV_OPENFLOW_FLOW_MALLOC(sizeof(metadata_mapping_t));
    HSRV_OPENFLOW_MEM_CHECK(p_metadata);
    sal_memset(p_metadata, 0, sizeof(metadata_mapping_t));
    p_metadata->metadata = metadata;
    p_metadata->type = type;
    p_metadata->ref_cnt ++;
    if (METADATA_TYPE_FID == type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_alloc_fid(&fid));
        p_metadata->u.fid = fid;
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_fdb_add_default_entry(fid));
        hsrv_openflow_flowdb_add_metadata_mapping(p_metadata);   
    }
    else if(METADATA_TYPE_VRF == type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_alloc_vrf_id(&vrf_id));
        p_metadata->u.vrf_id = vrf_id;
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_ipuc_add_default_entry(vrf_id));
        hsrv_openflow_flowdb_add_metadata_mapping(p_metadata);
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_del_metadata_mapping(uint64 metadata)
{
    metadata_mapping_t  *p_metadata = NULL;
    uint16 fid = 0;
    uint16 vrf_id = 0;
    
    hsrv_openflow_flowdb_lookup_metadata_mapping(metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    if (p_metadata->ref_cnt > 1)
    {
        p_metadata->ref_cnt --;
        return HSRV_E_NONE;
    }
    fid = p_metadata->u.fid;
    vrf_id = p_metadata->u.vrf_id;
    if (METADATA_TYPE_FID == p_metadata->type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_release_fid(fid));
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_fdb_del_default_entry(fid));
    }
    else if(METADATA_TYPE_VRF == p_metadata->type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_release_vrf_id(vrf_id));
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_ipuc_del_default_entry(vrf_id));
    }
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_del_metadata_mapping(metadata));
    HSRV_OPENFLOW_FLOW_FREE(p_metadata);

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_alloc_flow_info(glb_openflow_flow_t* p_msg_flow, hsrv_openflow_ttp_flow_t** pp_flow)
{
    hsrv_openflow_ttp_flow_t* p_flow;
    
    p_flow = HSRV_OPENFLOW_FLOW_MALLOC(sizeof(hsrv_openflow_ttp_flow_t));
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    HSRV_OPENFLOW_MEM_CHECK(p_msg_flow);

    sal_memset(p_flow, 0, sizeof(hsrv_openflow_ttp_flow_t));

    p_flow->flow.table_id = p_msg_flow->table;
    p_flow->flow.flow_id = p_msg_flow->flow_id;
    p_flow->flow.priority = p_msg_flow->priority;
    p_flow->stats_supported = p_msg_flow->stats_en;
    
    *pp_flow = p_flow;
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_map_key(glb_openflow_flow_t *p_msg_flow, hsrv_openflow_ttp_flow_t* p_flow)
{
    hsrv_vlan_flow_entry_t  *pst_vlan_flow = NULL;
    hsrv_termination_mac_flow_entry_t   *pst_ter_mac_flow = NULL;
    hsrv_bridge_flow_entry_t    *pst_bri_mac_flow = NULL;
    hsrv_uincast_routing_flow_entry_t  *pst_route_flow = NULL;
    hsrv_ingress_acl_flow_entry_t   *pst_ingress_acl = NULL;
    glb_openflow_flow_key_t     *pst_flow_match = NULL;
    hsrv_openflow_ttp_port_info_t *p_port = NULL; 

    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    HSRV_OPENFLOW_MEM_CHECK(p_msg_flow);

    pst_flow_match = &p_msg_flow->key;

    if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN == p_flow->flow.table_id)
    {
        pst_vlan_flow = &p_flow->flow.flow_data.vlan_flow_entry;
        p_port = hsrv_openflow_port_get_port_info_by_ifindex(pst_flow_match->in_port.ifindex);
        if (!p_port)
        {
            return HSRV_E_PORT_NOT_EXIST;
        }

        pst_vlan_flow->match_criteria.inport = p_port->gport;
        pst_vlan_flow->match_criteria.vlan_id = p_msg_flow->key.mac.svlan;
        pst_vlan_flow->match_criteria.vlan_id_mask = HSRV_UINT16_MAX;
        if (0 == pst_vlan_flow->match_criteria.vlan_id)
        {
            pst_vlan_flow->match_criteria.vlan_id = GLB_VLAN_DEFAULT_VID;
        }
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC == p_flow->flow.table_id)
    {
        pst_ter_mac_flow = &p_flow->flow.flow_data.termination_mac_flow_entry;
        sal_memcpy(pst_ter_mac_flow->match_criteria.dst_mac, &p_msg_flow->key.mac.mac_da, MAC_ADDR_LEN);
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING == p_flow->flow.table_id)
    {
        pst_route_flow = &p_flow->flow.flow_data.unicast_routing_flow_entry;
        pst_route_flow->match_criteria.metadata = p_msg_flow->key.rule.ipv4_rule.metadata;
        sal_memcpy(&pst_route_flow->match_criteria.dst_ip, &p_msg_flow->key.rule.ipv4_rule.ip_da, sizeof(ip_addr_t));
        sal_memcpy(&pst_route_flow->match_criteria.dst_ip_mask, &p_msg_flow->key.rule.ipv4_rule.ip_da_mask, sizeof(ip_addr_t));
        
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_metadata_mapping(pst_route_flow->match_criteria.metadata, METADATA_TYPE_VRF));
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING == p_flow->flow.table_id)
    {
        pst_bri_mac_flow = &p_flow->flow.flow_data.bridge_flow_entry;
        sal_memcpy(pst_bri_mac_flow->match_criteria.dst_mac, &p_msg_flow->key.mac.mac_da, MAC_ADDR_LEN);
        pst_bri_mac_flow->match_criteria.metadata = p_msg_flow->key.mac.metadata;
        pst_bri_mac_flow->match_criteria.vniid = (uint32)p_msg_flow->key.mac.metadata;

        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_metadata_mapping(pst_bri_mac_flow->match_criteria.metadata, METADATA_TYPE_FID));
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL == p_flow->flow.table_id)
    {
        pst_ingress_acl = &p_flow->flow.flow_data.ingress_acl_flow_entry;
        if (pst_flow_match->in_port.ofport)
        {
            p_port = hsrv_openflow_port_get_port_info_by_ifindex(pst_flow_match->in_port.ifindex);
            if (!p_port)
            {
                return HSRV_E_PORT_NOT_EXIST;
            }
            pst_ingress_acl->match_criteria.inport = p_port->gport;
            pst_ingress_acl->match_criteria.inport_mask = HSRV_UINT32_MAX;
        }
        if(pst_flow_match->mac.flags.eth_type)
        {
            pst_ingress_acl->match_criteria.ether_type = pst_flow_match->mac.eth_type;
            pst_ingress_acl->match_criteria.ether_type_mask = HSRV_UINT16_MAX;
        }
        
        if(pst_flow_match->rule.ipv4_rule.flags.ip_da)
        {
            pst_ingress_acl->match_criteria.dst_ip = pst_flow_match->rule.ipv4_rule.ip_da;
            pst_ingress_acl->match_criteria.dst_ip_mask = pst_flow_match->rule.ipv4_rule.ip_da_mask;
        }
        
        if(pst_flow_match->rule.ipv4_rule.flags.ip_sa)
        {
            pst_ingress_acl->match_criteria.src_ip = pst_flow_match->rule.ipv4_rule.ip_sa;
            pst_ingress_acl->match_criteria.src_ip_mask = pst_flow_match->rule.ipv4_rule.ip_sa_mask;
        }
        
        if(pst_flow_match->rule.ipv4_rule.flags.l4_protocol)
        {
            pst_ingress_acl->match_criteria.ip_proto = pst_flow_match->rule.ipv4_rule.l4_protocol;
            pst_ingress_acl->match_criteria.ip_proto_mask = HSRV_UINT8_MAX;
        }
        
        if(pst_flow_match->rule.ipv4_rule.flags.l4_source_port)
        {
            pst_ingress_acl->match_criteria.src_port = pst_flow_match->rule.ipv4_rule.l4_source_port.port;
            pst_ingress_acl->match_criteria.src_port_mask = HSRV_UINT16_MAX;
        }
        
        if(pst_flow_match->rule.ipv4_rule.flags.l4_dest_port)
        {
            pst_ingress_acl->match_criteria.dst_port = pst_flow_match->rule.ipv4_rule.l4_dest_port.port;
            pst_ingress_acl->match_criteria.dst_port_mask = HSRV_UINT16_MAX;
        }
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_map_action(glb_openflow_flow_t *p_msg_flow, hsrv_openflow_ttp_flow_t* p_flow)
{
    glb_openflow_flow_action_t  *p_action = NULL;
    hsrv_openflow_ttp_group_t   *p_group = NULL;
    hsrv_openflow_ttp_meter_info_t *p_meter = NULL;
    hsrv_openflow_ttp_port_info_t *p_port = NULL;
    
    uint64  metadata = 0;
    uint8   table_id = 0;
    uint32  ofport = 0;
    uint32  ifindex = 0;
    uint32  gport = 0;
    uint32  group_id = 0;
    uint32  meter_id = 0;
    uint32  action_count = 0;
    bool    send_to_controller = 0;
    bool    ttl_dec = 0;
    bool    clear_action = 0;

    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    HSRV_OPENFLOW_MEM_CHECK(p_msg_flow);
    
    for (action_count = 0; action_count < p_msg_flow->actlist.action_count; action_count ++)
    {
        p_action = &p_msg_flow->actlist.actlist[action_count];
        switch(p_action->type)
        {
        case GLB_OPENFLOW_ACTION_TYPE_OUTPUT:
            //maybe use ofport when port adpt complete, to controller
            ofport = p_action->value.output.ofport;
            ifindex = p_action->value.output.ifindex;
            if (GLB_OFPP_CONTROLLER == ofport)
            {
                send_to_controller = TRUE;
            }
            else
            {
                p_port = hsrv_openflow_port_get_port_info_by_ifindex(ifindex);
                if (!p_port)
                {
                    return HSRV_E_PORT_NOT_EXIST;
                }
                gport = p_port->gport;
            }
            break;
        case GLB_OPENFLOW_ACTION_TYPE_WRITE_METADATA:
            metadata = p_action->value.m.metadata;
            break;

        case GLB_OPENFLOW_ACTION_TYPE_GOTO_TABLE:
            table_id = p_action->value.u8;
            break;

        case GLB_OPENFLOW_ACTION_TYPE_GROUP:                
            group_id = p_action->value.u32;
            p_group = hsrv_openflow_group_get_group_info(group_id);
            if (!p_group)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            break;

        case GLB_OPENFLOW_ACTION_TYPE_INST_METER:
            meter_id = p_action->value.u32;
            p_meter = hsrv_openflow_get_meter_info(meter_id);
            if (!p_meter)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            break;
            break;

        case GLB_OPENFLOW_ACTION_TYPE_DEC_TTL:
            ttl_dec = TRUE;
            break;

        case GLB_OPENFLOW_ACTION_TYPE_CLEAR_ACTIONS:
            clear_action = TRUE;
            break;

        default:
            break;
        }
    }

    if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN == p_flow->flow.table_id)
    {
        p_flow->flow.flow_data.vlan_flow_entry.metadata = metadata;
        p_flow->flow.flow_data.vlan_flow_entry.goto_table = table_id;
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_metadata_mapping(metadata, METADATA_TYPE_FID));
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC == p_flow->flow.table_id)
    {
        p_flow->flow.flow_data.termination_mac_flow_entry.metadata = metadata;
        p_flow->flow.flow_data.termination_mac_flow_entry.goto_table = table_id;
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_metadata_mapping(metadata, METADATA_TYPE_VRF));
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING == p_flow->flow.table_id)
    {
        p_flow->flow.flow_data.unicast_routing_flow_entry.group_id = group_id;
        p_flow->flow.flow_data.unicast_routing_flow_entry.goto_table = table_id;
        p_flow->flow.flow_data.unicast_routing_flow_entry.ttl_dec = ttl_dec;
        if (send_to_controller)
        {
            p_flow->flow.flow_data.unicast_routing_flow_entry.send_to_controller = TRUE;
        }
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING == p_flow->flow.table_id)
    {
        p_flow->flow.flow_data.bridge_flow_entry.group_id = group_id;
        p_flow->flow.flow_data.bridge_flow_entry.goto_table = table_id;
    }
    else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL == p_flow->flow.table_id)
    {
        if (send_to_controller) //send to controller
        {
            p_flow->flow.flow_data.ingress_acl_flow_entry.send_to_controller = TRUE;
        }
        else if(clear_action)
        {
            if (ofport) //redirect
            {
                p_flow->flow.flow_data.ingress_acl_flow_entry.redirect_action = TRUE;
            }
            else    //discard
            {
                p_flow->flow.flow_data.ingress_acl_flow_entry.discard_action = TRUE;
            }
        }
        else if(ofport) //mirror
        {
            p_flow->flow.flow_data.ingress_acl_flow_entry.mirror_action = TRUE;
        }
        else if(meter_id)   //meter
        {
            p_flow->flow.flow_data.ingress_acl_flow_entry.meter_action = TRUE;
            p_flow->flow.flow_data.ingress_acl_flow_entry.meter_id = meter_id;
        }
        p_flow->flow.flow_data.ingress_acl_flow_entry.output_port = gport;
        p_flow->flow.flow_data.ingress_acl_flow_entry.clear_action = clear_action;
    }
        
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_add_vlan_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    uint32 entry_id = 0;

    /* alloc scl entry id */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_id_alloc(&entry_id));
    p_flow->entry_id = entry_id;
    
    /* add vlan mapping scl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_vlan_mapping_add(p_flow));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_vlan_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    /* free scl entry id */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_id_free(p_flow->entry_id));

    /* remove vlan mapping scl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_remove(p_flow));

    /* should delete after scl flow remove */
    /* free metadata mapping */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_metadata_mapping(p_flow->flow.flow_data.vlan_flow_entry.metadata));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_add_termination_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    uint32 entry_id = 0;

    /* alloc scl entry id */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_id_alloc(&entry_id));
    p_flow->entry_id = entry_id;

    /* add scl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_terminal_mac_flow_add(p_flow));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_termination_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    /* free scl entry id */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_id_free(p_flow->entry_id));
 
    /* remove scl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_entry_remove(p_flow));

    /* should delete after scl flow remove */
    /* free metadata mapping */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_metadata_mapping(p_flow->flow.flow_data.termination_mac_flow_entry.metadata));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_add_uincast_routing_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    metadata_mapping_t  *p_metadata = NULL;
    hsrv_uincast_routing_flow_entry_t *p_entry = &p_flow->flow.flow_data.unicast_routing_flow_entry;

    /* get metadata mapping */
    hsrv_openflow_flowdb_lookup_metadata_mapping(p_entry->match_criteria.metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    p_entry->match_criteria.vrf = p_metadata->u.vrf_id;

    /* add ipuc entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_ipuc_add_entry(p_flow));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_uincast_routing_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    hsrv_uincast_routing_flow_entry_t *p_entry = &p_flow->flow.flow_data.unicast_routing_flow_entry;

    /* remove ipuc entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_ipuc_remove_entry(p_flow));

    /* free metadata mapping */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_metadata_mapping(p_entry->match_criteria.metadata));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_add_bridging_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    metadata_mapping_t  *p_metadata = NULL;
    hsrv_bridge_flow_entry_t *p_entry = &p_flow->flow.flow_data.bridge_flow_entry;

    /* get metadata mapping */
    hsrv_openflow_flowdb_lookup_metadata_mapping(p_entry->match_criteria.metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    p_entry->match_criteria.fid = p_metadata->u.fid;
    
    /* add fdb entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_fdb_add_entry(p_flow));
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_bridging_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    hsrv_bridge_flow_entry_t *p_entry = &p_flow->flow.flow_data.bridge_flow_entry;
    
    /* remove fdb entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_fdb_remove_entry(p_flow));
    
    /* free metadata mapping */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_metadata_mapping(p_entry->match_criteria.metadata));
    
    return HSRV_E_NONE;
}


static int32
_hsrv_openflow_flow_add_ingress_acl_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    uint32 entry_id = 0;

    /*alloc acl entry id*/
    ctc_ttp_drv_acl_entry_id_alloc(&entry_id);
    p_flow->entry_id = entry_id;

    /* add acl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_acl_add_entry(p_flow));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_ingress_acl_flow(hsrv_openflow_ttp_flow_t* p_flow)
{  
    /*free acl entry id*/
    ctc_ttp_drv_acl_entry_id_free(p_flow->entry_id);

    /* remove acl entry */
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_acl_remove_entry(p_flow));

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_add_drv_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
        
    switch(p_flow->flow.table_id)
    {
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_vlan_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_termination_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_uincast_routing_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_bridging_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_ingress_acl_flow(p_flow));
        break;

    default:
        break;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_del_drv_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
        
    switch(p_flow->flow.table_id)
    {
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_vlan_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_termination_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_uincast_routing_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_bridging_flow(p_flow));
        break;
    case GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL:
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_ingress_acl_flow(p_flow));
        break;

    default:
        break;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_release_flow_info(hsrv_openflow_ttp_flow_t* p_flow)
{
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    HSRV_OPENFLOW_FLOW_FREE(p_flow);
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_create_stats(hsrv_openflow_ttp_flow_t* p_flow)
{
    uint32 stats_id = 0;

    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    //stats_supported inherit from ovs
    if (p_flow->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_add_stats(GLB_STATS_TYPE_OPENFLOW_FLOW, p_flow->flow.flow_id, p_flow->flow.table_id, &stats_id));
        p_flow->stats_id = stats_id;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_delete_stats(hsrv_openflow_ttp_flow_t* p_flow)
{
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    if (p_flow->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_del_stats(p_flow->stats_id));
        p_flow->stats_id = 0;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_flow_clear_stats(hsrv_openflow_ttp_flow_t* p_flow)
{
    HSRV_OPENFLOW_MEM_CHECK(p_flow);
    if (p_flow->stats_supported)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_clear_stats(p_flow->stats_id));
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_add_fei_flow(glb_openflow_flow_t* p_glb_flow)
{
    hsrv_openflow_ttp_flow_t* p_flow;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(p_glb_flow->flow_id, &p_flow));
    if (p_flow)
    {
        return HSRV_E_ENTRY_EXIST;
    }
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_alloc_flow_info(p_glb_flow, &p_flow));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_map_key(p_glb_flow, p_flow));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_map_action(p_glb_flow, p_flow));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_create_stats(p_flow));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_add_drv_flow(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_add_flow(p_flow));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_del_fei_flow(uint32 flow_id)
{
    hsrv_openflow_ttp_flow_t* p_flow = NULL;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(flow_id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_del_drv_flow(p_flow));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_delete_stats(p_flow));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_del_flow(flow_id));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_release_flow_info(p_flow));
    //TODO clear stats?
    
    return HSRV_E_NONE;
}

#if 0

struct hsrv_openflow_debug_data_s
{
    FILE* fp;
    int32 i;
};
typedef struct hsrv_openflow_debug_data_s hsrv_openflow_flow_debug_data_t;

struct hsrv_openflow_debug_data1_s
{
    uint32 actlist_ptr;
    bool exist;
};
typedef struct hsrv_openflow_debug_data1_s hsrv_openflow_flow_debug_data1_t;

#define _____FLOW_ID_____




static int32
hsrv_openflow_lookup_flow_info(glb_openflow_flow_t* p_msg_flow, hsrv_openflow_flow_t** pp_flow)
{
    hsrv_openflow_flow_t* p_flow;
    glb_openflow_flow_key_t key;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(p_msg_flow->flow_id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /* flow action may be changed, so we need release old actions */
    if (p_flow->flow.actlist.actlist)
    {
        HSRV_OPENFLOW_ACT_FREE(p_flow->flow.actlist.actlist);
    }

    /*tunnel should use hsrv flow key, some para created in hsrv*/
    sal_memcpy(&key, &(p_flow->flow.key), sizeof(glb_openflow_flow_key_t));
    sal_memcpy(&p_flow->flow, p_msg_flow, sizeof(glb_openflow_flow_t));
    sal_memcpy(&p_flow->flow.key, &key, sizeof(glb_openflow_flow_key_t));
    *pp_flow = p_flow;

    return HSRV_E_NONE;
}

static int32
hsrv_openflow_release_flow_info(hsrv_openflow_flow_t* p_flow)
{
    if (p_flow->flow.actlist.actlist)
    {
        HSRV_OPENFLOW_ACT_FREE(p_flow->flow.actlist.actlist);
    }
    HSRV_OPENFLOW_FLOW_FREE(p_flow);
    
    return HSRV_E_NONE;
}


static int32
hsrv_openflow_move_flow_info(hsrv_openflow_flow_info_t* p_from, hsrv_openflow_flow_info_t* p_to)
{
    memcpy(p_to, p_from, sizeof(hsrv_openflow_flow_info_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_reset_flow_info(p_from));

    return HSRV_E_NONE;
}

#endif

#define _____HAL_APIs_____

int32 
fei_api_openflow_add_flow(fei_openflow_add_flow_req_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_fei_flow(&pst_req->flow));
    
    return HSRV_E_NONE;
}

int32
fei_api_openflow_mod_flow(fei_openflow_mod_flow_req_t *pst_req)
{  
    //delete old flow
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_fei_flow(pst_req->flow.flow_id));

    //add new flow
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_fei_flow(&pst_req->flow));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_del_flow(fei_openflow_del_flow_req_t *pst_req)
{
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_fei_flow(pst_req->flow_id));
    
    return HSRV_E_NONE;
}

int32
fei_api_openflow_clear_flow_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    hsrv_openflow_ttp_flow_t* p_flow = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(pst_req->id, &p_flow));
    if (NULL == p_flow)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_flow_clear_stats(p_flow));

    return HSRV_E_NONE;
}

#define _____DEBUG_____

#if 0

static void 
hsrv_openflow_show_flow_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_ttp_flow_t* p_flow = (hsrv_openflow_ttp_flow_t*)phb->data;
    hsrv_openflow_flow_debug_data_t* p_data = user_data; 
    uint32 nhid = 0;
    
    sal_fprintf(p_data->fp, "%5d %10u",
            p_data->i, 
            p_flow->flow.flow_id);

    if (p_flow->u.dft.var.normal)
    {
        sal_fprintf(p_data->fp, " normal");
    }
    else
    {
        if (p_flow->u.dft.var.group_info.group_id != OFP_GROUP_INVALID_GROUP_ID)
        {
            nhid = p_flow->u.dft.var.group_info.group_nhid;
        }
        else if (p_flow->u.dft.var.actlist)
        {
            nhid = p_flow->u.dft.var.actlist->nh_info.main_nh.nh_offset.nhid;
        }
        else
        {
            nhid = p_flow->u.dft.var.nh_info.main_nh.nh_offset.nhid;
        }
        
        sal_fprintf(p_data->fp, "%10u", nhid);
    }
    if (p_flow->u.dft.var.actlist)
    {
        sal_fprintf(p_data->fp, "     %p", 
            p_flow->u.dft.var.actlist);
    }
    else
    {
        sal_fprintf(p_data->fp, "%15s", "NULL");
    }
    sal_fprintf(p_data->fp, "%9u", p_flow->flow.stats_en);
    sal_fprintf(p_data->fp, "%9u", p_flow->u.dft.cnst.stats_id);
    sal_fprintf(p_data->fp, "%30s", 
                GLB_OPENFLOW_MPLS_ACTION_SEQ_TO_STR(p_flow->flow.u.dft.mpls_output_action_seq_type));
    sal_fprintf(p_data->fp, "\n");

    p_data->i ++;
}


int32
hsrv_openflow_show_flow_brief(FILE *fp)
{
    hsrv_openflow_flow_master_t* p_master = g_pst_openflow_flow_master;
    hsrv_openflow_flow_debug_data_t data;
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW FLOW---------------------\n");
    sal_fprintf(fp, "Flow hash count    : %d\n", p_master->p_flow_hash->count);
    sal_fprintf(fp, "Actlist hash count : %d\n", p_master->p_actlist_hash->count);
    sal_fprintf(fp, "Global acl label   : %d\n", p_master->global_acl_label);
    sal_fprintf(fp, "----------------------------------------------------\n");

    sal_fprintf(fp, "%5s %10s %10s %15s %9s %9s %30s\n",
        "index", "flow_id", "nhid", "actlist",
        "stats_en", 
        "stats_id",
        "mpls_seqtype");
    sal_fprintf(fp, "----------------------------------------------------\n");

    data.fp = fp;
    data.i  = 0;
    
    ctclib_hash_iterate(HSRV_OPENFLOW_FLOW_HASH, hsrv_openflow_show_flow_brief__, &data);
    
    return HSRV_E_NONE;
}

void
hsrv_openflow_show_flow_nh_info(FILE *fp, hsrv_openflow_nexthop_info_t* p_nh_info)
{
    int32 member_count = 0;

    sal_fprintf(fp, "nexthop_info  \n");
    sal_fprintf(fp, "  use_mcast  : %d\n", p_nh_info->use_mcast);
    sal_fprintf(fp, "  main_nh    : %10d %10d %10s\n", 
        p_nh_info->main_nh.nh_offset.nhid,
        p_nh_info->main_nh.nh_offset.offset,
        HSRV_OF_NH_TYPE_STR(p_nh_info->main_nh.nh_offset.nh_type));
    if (p_nh_info->use_mcast)
    {
        sal_fprintf(fp, "  members    : %10s %10s %10s %12s\n", 
            "nhid", "offset", "type", "nexthop_ptr");
        sal_fprintf(fp, "               %10s %10s %10s %12s\n", 
            "----------", "----------", "----------", "------------");
        for (member_count = 0; member_count < p_nh_info->member_count; member_count ++)
        {
            sal_fprintf(fp, "  member %3d : %10d %10d %12s ",
                member_count,
                p_nh_info->member_nh[member_count].nh_offset.nhid,
                p_nh_info->member_nh[member_count].nh_offset.offset,
                HSRV_OF_NH_TYPE_STR(p_nh_info->member_nh[member_count].nh_offset.nh_type));
            if (p_nh_info->member_nh[member_count].share_info)
            {
                sal_fprintf(fp, "  %p",
                    p_nh_info->member_nh[member_count].share_info);
            }
            else
            {
                sal_fprintf(fp, "(NULL)");
            }
            sal_fprintf(fp, "\n");
        }
    }
}

int32
hsrv_openflow_show_flow(FILE *fp, uint32 flow_id)
{
    hsrv_openflow_flow_t* p_flow;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_lookup_flow(flow_id, &p_flow));
    if (NULL == p_flow)
    {
        sal_fprintf(fp, "flow_id %u not exist in db.\n", flow_id);
        return HSRV_E_NONE;
    }
    sal_fprintf(fp, "flow_id      : %u\n", flow_id);

    sal_fprintf(fp, "const_info \n");
    sal_fprintf(fp, "  entry_id   : %u\n", p_flow->u.dft.cnst.entry_id);
    sal_fprintf(fp, "  label_id   : %u\n", p_flow->u.dft.cnst.label_id);

    sal_fprintf(fp, "var_info \n");
    sal_fprintf(fp, "  normal     : %d\n", p_flow->u.dft.var.normal);
    sal_fprintf(fp, "  output_cnt : %d\n", p_flow->u.dft.var.output_count);s
    if (p_flow->u.dft.var.actlist)
    {
        sal_fprintf(fp, "  actlist    : %p\n", 
            p_flow->u.dft.var.actlist);
    }
    else
    {
        sal_fprintf(fp, "  actlist    : NULL\n");
    }
    hsrv_openflow_show_flow_nh_info(fp, &p_flow->u.dft.var.nh_info);

    sal_fprintf(fp, "group_info \n");

    sal_fprintf(fp, "meter_info \n");

    sal_fprintf(fp, "queue_info \n");

    sal_fprintf(fp, "\n");


    return HSRV_E_NONE;
}
#endif

#define _____INIT_____

int32
hsrv_openflow_flow_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flowdb_start());

    /*table_miss entry create*/
    ctc_ttp_drv_nexthop_alloc_mc_groupid(&g_pst_ttp_bridge_flow_table_miss_group_id);
    ctc_ttp_drv_nexthop_alloc_mc_groupid(&g_pst_ttp_unicast_flow_table_miss_group_id);
    
    ctc_ttp_drv_nexthop_alloc_nhid(&g_pst_ttp_bridge_flow_table_miss_nhid);
    ctc_ttp_drv_nexthop_alloc_nhid(&g_pst_ttp_unicast_flow_table_miss_nhid);
    
    ctc_ttp_drv_stats_create(HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST, &g_pst_ttp_bridge_flow_table_miss_stats_id);
    ctc_ttp_drv_stats_create(HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST, &g_pst_ttp_unicast_flow_table_miss_stats_id);
    ctc_ttp_drv_stats_create(HSRV_OPENFLOW_TTP_STATS_TYPE_SCL, &g_pst_ttp_vlan_mapping_flow_table_miss_stats_id);

    ctc_ttp_drv_nexthop_add_mcast_nh(g_pst_ttp_bridge_flow_table_miss_group_id, 
                                                    g_pst_ttp_bridge_flow_table_miss_nhid, 
                                                    g_pst_ttp_bridge_flow_table_miss_stats_id);

    ctc_ttp_drv_nexthop_add_mcast_nh(g_pst_ttp_unicast_flow_table_miss_group_id, 
                                                    g_pst_ttp_unicast_flow_table_miss_nhid, 
                                                    g_pst_ttp_unicast_flow_table_miss_stats_id);

    ctc_ttp_drv_nexthop_add_mcast_member(g_pst_ttp_bridge_flow_table_miss_nhid, TTP_DROP_NEXTHOP);
    ctc_ttp_drv_nexthop_add_mcast_member(g_pst_ttp_unicast_flow_table_miss_nhid, TTP_DROP_NEXTHOP);

    return HSRV_E_NONE;
}

#define _____END_LINE_____
