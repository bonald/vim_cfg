
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_acl.h"

//static ctclib_opf_t *g_pst_ctc_drv_acl_master = NULL;

extern void 
ctc_sai_acl_alloc_entry_id(uint32* p_index);

extern void 
ctc_sai_acl_release_entry_id(uint32 index);

void
ctc_ttp_drv_acl_entry_id_alloc(uint32 *entry_id)
{
    ctc_sai_acl_alloc_entry_id(entry_id);
}

void
ctc_ttp_drv_acl_entry_id_free(uint32 entry_id)
{
    ctc_sai_acl_release_entry_id(entry_id);
}

int32
ctc_ttp_drv_acl_add_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_acl_entry_t acl_entry;
    hsrv_ingress_acl_flow_entry_t *p_entry = &p_flow->flow.flow_data.ingress_acl_flow_entry;
    hsrv_openflow_ttp_port_info_t *p_port = NULL;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;
    uint32 nh_id = 0;
    uint32 mirror_id = 0;
    uint32 acl_group_id = 0;
    int32 rc = 0;
    
    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));

    if (p_entry->match_criteria.ether_type_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
        acl_entry.key.u.ipv4_key.eth_type = p_entry->match_criteria.ether_type;
        acl_entry.key.u.ipv4_key.eth_type_mask = p_entry->match_criteria.ether_type_mask;
    }

    if (p_entry->match_criteria.src_ip_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);
        acl_entry.key.u.ipv4_key.ip_sa = p_entry->match_criteria.src_ip;
        acl_entry.key.u.ipv4_key.ip_sa_mask = p_entry->match_criteria.src_ip_mask;
    }

    if (p_entry->match_criteria.dst_ip_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA);
        acl_entry.key.u.ipv4_key.ip_da = p_entry->match_criteria.dst_ip;
        acl_entry.key.u.ipv4_key.ip_da_mask = p_entry->match_criteria.dst_ip_mask;
    }

    if (p_entry->match_criteria.ip_proto_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL);
        acl_entry.key.u.ipv4_key.l4_protocol = p_entry->match_criteria.ip_proto;
        acl_entry.key.u.ipv4_key.l4_protocol_mask = p_entry->match_criteria.ip_proto_mask;
    }
    
    if (p_entry->match_criteria.src_port_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
        acl_entry.key.u.ipv4_key.l4_src_port_0 = p_entry->match_criteria.src_port;
        acl_entry.key.u.ipv4_key.l4_src_port_1 = p_entry->match_criteria.src_port_mask;
        acl_entry.key.u.ipv4_key.l4_src_port_use_mask = 1;
    }
    
    if (p_entry->match_criteria.dst_port_mask)
    {
        GLB_SET_FLAG(acl_entry.key.u.ipv4_key.sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
        acl_entry.key.u.ipv4_key.l4_dst_port_0 = p_entry->match_criteria.dst_port;
        acl_entry.key.u.ipv4_key.l4_dst_port_1 = p_entry->match_criteria.dst_port_mask;
        acl_entry.key.u.ipv4_key.l4_dst_port_use_mask = 1;
    }

    if (p_flow->stats_supported)
    {
        p_stats = hsrv_openflow_get_stats_info(p_flow->stats_id);
        if (p_stats)
        {
            acl_entry.action.stats_id = p_stats->ctc_stats_id;
            CTC_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_STATS);
        }
    }    

    if (p_entry->discard_action)
    {
        CTC_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_DISCARD);
    }
    else if (p_entry->send_to_controller)
    {
        acl_entry.action.nh_id = ctc_ttp_drv_nexthop_get_controller_nh();
        GLB_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_REDIRECT);
    }
    else if (p_entry->redirect_action)
    {
        HSRV_IF_ERROR_RETURN(ctc_nh_get_l2uc(p_entry->output_port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id));
        acl_entry.action.nh_id = nh_id;
        GLB_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_REDIRECT);
    }
    else if (p_entry->mirror_action)
    {
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_mirror_id_alloc(&mirror_id));
        p_entry->mirror_id = mirror_id;
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_mirror_create(mirror_id, CTC_INGRESS, p_entry->output_port));
        acl_entry.action.flag |= CTC_ACL_ACTION_FLAG_RANDOM_LOG;
        acl_entry.action.log_percent = CTC_LOG_PERCENT_POWER_NEGATIVE_0;
        /*4 mirror session in toall */
        acl_entry.action.log_session_id = mirror_id;
    }
    else if (p_entry->meter_action)
    {
        GLB_SET_FLAG(acl_entry.action.flag, CTC_ACL_ACTION_FLAG_MICRO_FLOW_POLICER);
        acl_entry.action.micro_policer_id = p_entry->meter_id;
    }

    if (p_entry->match_criteria.inport_mask)
    {
        //port-class base acl 
        p_port = hsrv_openflow_port_get_port_info_by_portid(p_entry->match_criteria.inport);
        if (!p_port)
        {
            return HSRV_E_PORT_NOT_EXIST;
        }
        acl_group_id = p_port->group_id;
    }
    else
    {
        //global acl
        acl_group_id = TTP_GLB_ACL_GROUP;
    }
    acl_entry.entry_id = p_flow->entry_id;
    acl_entry.key.type = CTC_ACL_KEY_IPV4;
    acl_entry.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
    acl_entry.priority = p_flow->flow.priority;
    acl_entry.priority_valid = TRUE;

    HSRV_IF_ERROR_RETURN(ctc_acl_add_entry(acl_group_id, &acl_entry));
    HSRV_IF_ERROR_RETURN(ctc_acl_install_entry(acl_entry.entry_id));
    
    return rc;
}

int32
ctc_ttp_drv_acl_remove_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    int32 rc = 0;

    if (p_flow->flow.flow_data.ingress_acl_flow_entry.mirror_action)
    {
        ctc_ttp_drv_mirror_id_free(p_flow->flow.flow_data.ingress_acl_flow_entry.mirror_id);
        ctc_ttp_drv_mirror_remove(p_flow->flow.flow_data.ingress_acl_flow_entry.mirror_id);
        
    }
    rc = ctc_acl_uninstall_entry(p_flow->entry_id);
    if (rc)
    {
        return rc;
    }
    
    return ctc_acl_remove_entry(p_flow->entry_id);
}

int32
ctc_ttp_drv_acl_add_default_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    return 0;
}

int32
ctc_ttp_drv_acl_init()
{
    //g_pst_ctc_drv_acl_master = ctclib_opf_init(1);
    //ctclib_opf_init_offset(g_pst_ctc_drv_acl_master, 0, CTC_ACL_ENTRY_ID_MIN, CTC_ACL_ENTRY_ID_NUM);
    
    return 0;
}



