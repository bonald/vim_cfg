bool
tbl_sys_spec_global_is_inited()
{
    return _g_p_tbl_sys_spec_global->is_inited;
}

uint32
tbl_sys_spec_global_set_inited(uint32 value)
{
    _g_p_tbl_sys_spec_global->is_inited = value;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_cur_type()
{
    return _g_p_tbl_sys_spec_global->cur_type;
}

uint32
tbl_sys_spec_global_set_cur_type(uint32 type)
{
    if (GLB_STM_MODE_MAX <= type)
    {
        return PM_E_INVALID_PARAM;
    }
    _g_p_tbl_sys_spec_global->cur_type = type;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_next_boot_type()
{
    return _g_p_tbl_sys_spec_global->next_boot_type;
}

uint32
tbl_sys_spec_global_set_next_boot_type(uint32 type)
{
    if (GLB_STM_MODE_MAX <= type)
    {
        return PM_E_INVALID_PARAM;
    }
    _g_p_tbl_sys_spec_global->next_boot_type = type;
    return PM_E_NONE;
}

bool
tbl_sys_spec_global_is_hardware_config()
{
    return _g_p_tbl_sys_spec_global->hardware_config;
}

uint32
tbl_sys_spec_global_set_hardware_config(uint32 value)
{
    _g_p_tbl_sys_spec_global->hardware_config = value;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_met_num()
{
    return _g_p_tbl_sys_spec_global->glb_met_number;
}

uint32
tbl_sys_spec_global_set_met_num(uint32 num)
{
    _g_p_tbl_sys_spec_global->glb_met_number = num;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_nh_num()
{
    return _g_p_tbl_sys_spec_global->glb_nexthop_number;
}

uint32
tbl_sys_spec_global_set_nh_num(uint32 num)
{
    _g_p_tbl_sys_spec_global->glb_nexthop_number = num;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_openflow_met_num()
{
    return _g_p_tbl_sys_spec_global->openflow_glb_met_number;
}

uint32
tbl_sys_spec_global_set_openflow_met_num(uint32 num)
{
    _g_p_tbl_sys_spec_global->openflow_glb_met_number = num;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_openflow_nh_num()
{
    return _g_p_tbl_sys_spec_global->openflow_glb_nexthop_number;
}

uint32
tbl_sys_spec_global_set_openflow_nh_num(uint32 num)
{
    _g_p_tbl_sys_spec_global->openflow_glb_nexthop_number = num;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_get_openflow_linkagg_interface_max_tid()
{
    return _g_p_tbl_sys_spec_global->openflow_linkagg_interface_max_tid;
}

uint32
tbl_sys_spec_global_set_openflow_linkagg_interface_max_tid(uint32 tid)
{
    _g_p_tbl_sys_spec_global->openflow_linkagg_interface_max_tid = tid;
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_set_ctrl_flag(uint32 flag)
{
    GLB_SET_FLAG(_g_p_tbl_sys_spec_global->ctrl_flag, flag);
    return PM_E_NONE;
}

uint32
tbl_sys_spec_global_unset_ctrl_flag(uint32 flag)
{
    GLB_UNSET_FLAG(_g_p_tbl_sys_spec_global->ctrl_flag, flag);
    return PM_E_NONE;
}

bool
tbl_sys_spec_global_check_ctrl_flag(uint32 flag)
{
    return GLB_FLAG_ISSET(_g_p_tbl_sys_spec_global->ctrl_flag, flag);
}

uint32
tbl_sys_spec_global_get_spec_limit(tbl_sys_spec_global_field_id_t field_id)
{
    switch (field_id)
    {
    case TBL_SYS_SPEC_GLOBAL_FLD_UCAST_FDB:
        return _g_p_tbl_sys_spec_global->ucast_fdb;

    case TBL_SYS_SPEC_GLOBAL_FLD_VLAN_INSTANCE:
        return _g_p_tbl_sys_spec_global->vlan_instance;

    case TBL_SYS_SPEC_GLOBAL_FLD_MAC_FILTER:
        return _g_p_tbl_sys_spec_global->mac_filter;

    case TBL_SYS_SPEC_GLOBAL_FLD_MAC_BASED_VLAN_CLASS:
        return _g_p_tbl_sys_spec_global->mac_based_vlan_class;

    case TBL_SYS_SPEC_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS:
        return _g_p_tbl_sys_spec_global->ipv4_based_vlan_class;

    case TBL_SYS_SPEC_GLOBAL_FLD_IPV4_SOURCE_GUARD:
        return _g_p_tbl_sys_spec_global->ipv4_source_guard;

    case TBL_SYS_SPEC_GLOBAL_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT:
        return _g_p_tbl_sys_spec_global->vlan_mapping_entry_applied_port;

    case TBL_SYS_SPEC_GLOBAL_FLD_DOT1X_MAC:
        return _g_p_tbl_sys_spec_global->dot1x_mac;

    case TBL_SYS_SPEC_GLOBAL_FLD_REMOTE_ROUTES:
        return _g_p_tbl_sys_spec_global->remote_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_HOST_ROUTES:
        return _g_p_tbl_sys_spec_global->host_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_ECMP_ROUTES:
        return _g_p_tbl_sys_spec_global->ecmp_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_PBR_ENTRIES:
        return _g_p_tbl_sys_spec_global->pbr_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_L2MC_ENTRIES:
        return _g_p_tbl_sys_spec_global->l3mc_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_L3MC_ENTRIES:
        return _g_p_tbl_sys_spec_global->l3mc_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_L2MC_MEMBER_PORTS:
        return _g_p_tbl_sys_spec_global->l2mc_member_ports;

    case TBL_SYS_SPEC_GLOBAL_FLD_L3MC_MEMBER_PORTS:
        return _g_p_tbl_sys_spec_global->l3mc_member_ports;

    case TBL_SYS_SPEC_GLOBAL_FLD_VLAN_MEMBER_PORTS:
        return _g_p_tbl_sys_spec_global->vlan_member_ports;

    case TBL_SYS_SPEC_GLOBAL_FLD_VOICE_VLAN:
        return _g_p_tbl_sys_spec_global->voice_vlan;

    case TBL_SYS_SPEC_GLOBAL_FLD_FLOW_ENTRIES_INGRESS:
        return _g_p_tbl_sys_spec_global->flow_entries_ingress;

    case TBL_SYS_SPEC_GLOBAL_FLD_FLOW_ENTRIES_EGRESS:
        return _g_p_tbl_sys_spec_global->flow_entries_egress;

    case TBL_SYS_SPEC_GLOBAL_FLD_CFM_LOCAL_AND_REMOTE_MEPS:
        return _g_p_tbl_sys_spec_global->cfm_local_and_remote_meps;

    case TBL_SYS_SPEC_GLOBAL_FLD_G8031_GROUPS:
        return _g_p_tbl_sys_spec_global->g8031_groups;

    case TBL_SYS_SPEC_GLOBAL_FLD_G8032_RINGS:
        return _g_p_tbl_sys_spec_global->g8032_rings;

    case TBL_SYS_SPEC_GLOBAL_FLD_G8032_MEMBER_PORTS_PER_RING:
        return _g_p_tbl_sys_spec_global->g8032_member_ports_per_ring;

    case TBL_SYS_SPEC_GLOBAL_FLD_FTN_ENTRIES:
        return _g_p_tbl_sys_spec_global->ftn_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_ILM_ENTRIES:
        return _g_p_tbl_sys_spec_global->ilm_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_MPLS_LABLES:
        return _g_p_tbl_sys_spec_global->mpls_lables;

    case TBL_SYS_SPEC_GLOBAL_FLD_VPWS:
        return _g_p_tbl_sys_spec_global->vpws;

    case TBL_SYS_SPEC_GLOBAL_FLD_LSP_PE:
        return _g_p_tbl_sys_spec_global->lsp_pe;

    case TBL_SYS_SPEC_GLOBAL_FLD_LSP_P:
        return _g_p_tbl_sys_spec_global->lsp_p;

    case TBL_SYS_SPEC_GLOBAL_FLD_VPLS_PEER:
        return _g_p_tbl_sys_spec_global->vpls_peer;

    case TBL_SYS_SPEC_GLOBAL_FLD_VPLS_AC:
        return _g_p_tbl_sys_spec_global->vpls_ac;

    case TBL_SYS_SPEC_GLOBAL_FLD_VSI:
        return _g_p_tbl_sys_spec_global->vsi;

    case TBL_SYS_SPEC_GLOBAL_FLD_LSP_OAM:
        return _g_p_tbl_sys_spec_global->lsp_oam;

    case TBL_SYS_SPEC_GLOBAL_FLD_PW_OAM:
        return _g_p_tbl_sys_spec_global->pw_oam;

    case TBL_SYS_SPEC_GLOBAL_FLD_MPLS_APS_TUNNEL:
        return _g_p_tbl_sys_spec_global->mpls_aps_tunnel;

    case TBL_SYS_SPEC_GLOBAL_FLD_CFM_LCK:
        return _g_p_tbl_sys_spec_global->cfm_lck;

    case TBL_SYS_SPEC_GLOBAL_FLD_IP_TUNNEL:
        return _g_p_tbl_sys_spec_global->ip_tunnel;

    case TBL_SYS_SPEC_GLOBAL_FLD_IVI_PEERS:
        return _g_p_tbl_sys_spec_global->ivi_peers;

    case TBL_SYS_SPEC_GLOBAL_FLD_HOST_V6_ROUTES:
        return _g_p_tbl_sys_spec_global->host_v6_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_ECMP_V6_ROUTES:
        return _g_p_tbl_sys_spec_global->ecmp_v6_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_PBR_V6_ENTRIES:
        return _g_p_tbl_sys_spec_global->pbr_v6_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_L3MC_V6_ENTRIES:
        return _g_p_tbl_sys_spec_global->l3mc_v6_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_L2MC_V6_ENTRIES:
        return _g_p_tbl_sys_spec_global->l2mc_v6_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_FLOW_V6_ENTRIES:
        return _g_p_tbl_sys_spec_global->flow_v6_entries;

    case TBL_SYS_SPEC_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS:
        return _g_p_tbl_sys_spec_global->ipv6_based_vlan_class;

    case TBL_SYS_SPEC_GLOBAL_FLD_IPV6_SOURCE_GUARD:
        return _g_p_tbl_sys_spec_global->ipv6_source_guard;

    case TBL_SYS_SPEC_GLOBAL_FLD_REMOTE_V6_ROUTES:
        return _g_p_tbl_sys_spec_global->remote_v6_routes;

    case TBL_SYS_SPEC_GLOBAL_FLD_L3MC_V6_MEMBER_PORTS:
        return _g_p_tbl_sys_spec_global->l3mc_v6_member_ports;

    case TBL_SYS_SPEC_GLOBAL_FLD_L2MC_V6_MEMBER_PORTS:
        return _g_p_tbl_sys_spec_global->l3mc_v6_member_ports;

    case TBL_SYS_SPEC_GLOBAL_FLD_VLAN_MAPPING_DEFAULT_ENTRY:
        return _g_p_tbl_sys_spec_global->vlan_mapping_default_entry;

    case TBL_SYS_SPEC_GLOBAL_FLD_BFD_SESSIONS:
        return _g_p_tbl_sys_spec_global->bfd_sessions;

    case TBL_SYS_SPEC_GLOBAL_FLD_NVGRE_TUNNEL_ID_MAPPINGS:
        return _g_p_tbl_sys_spec_global->nvgre_tunnel_id_mappings;

    case TBL_SYS_SPEC_GLOBAL_FLD_NVGRE_PEERS:
        return _g_p_tbl_sys_spec_global->nvgre_peers;

    case TBL_SYS_SPEC_GLOBAL_FLD_NVGRE_TUNNELS:
        return _g_p_tbl_sys_spec_global->nvgre_tunnels;

    case TBL_SYS_SPEC_GLOBAL_FLD_OPENFLOW_FLOW_ENTRIES:
        return _g_p_tbl_sys_spec_global->openflow_flow_entries;

    default:
        return 0;
    }
}
