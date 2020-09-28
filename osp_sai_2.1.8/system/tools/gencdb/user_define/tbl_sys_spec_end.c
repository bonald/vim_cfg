tbl_sys_spec_t*
tbl_sys_spec_get_sys_spec_by_type(uint32 type)
{
    if (GLB_STM_MODE_MAX <= type)
    {
        return NULL;
    }
    return (_g_p_tbl_sys_spec_master->sys_spec_array[type]);
}

uint32
tbl_sys_spec_get_sys_spec_field_by_type(uint32 type, int32 field_id)
{
    if (GLB_STM_MODE_MAX <= type)
    {
        return PM_E_NONE;
    }
    tbl_sys_spec_t* p_sys_spec = _g_p_tbl_sys_spec_master->sys_spec_array[type];

    if (FLD_MATCH(TBL_SYS_SPEC_FLD_KEY, field_id))
    {
                return p_sys_spec->key.type;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_UCAST_FDB, field_id))
    {
                return p_sys_spec->ucast_fdb;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_INSTANCE, field_id))
    {
                return p_sys_spec->vlan_instance;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_FILTER, field_id))
    {
                return p_sys_spec->mac_filter;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS, field_id))
    {
                return p_sys_spec->mac_based_vlan_class;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS, field_id))
    {
                return p_sys_spec->ipv4_based_vlan_class;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD, field_id))
    {
                return p_sys_spec->ipv4_source_guard;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT, field_id))
    {
                return p_sys_spec->vlan_mapping_entry_applied_port;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DOT1X_MAC, field_id))
    {
                return p_sys_spec->dot1x_mac;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_ROUTES, field_id))
    {
                return p_sys_spec->static_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_ROUTES, field_id))
    {
                return p_sys_spec->indirect_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_ROUTES, field_id))
    {
                return p_sys_spec->host_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_ROUTES, field_id))
    {
                return p_sys_spec->ecmp_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_ENTRIES, field_id))
    {
                return p_sys_spec->pbr_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_ENTRIES, field_id))
    {
                return p_sys_spec->l2mc_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_ENTRIES, field_id))
    {
                return p_sys_spec->l3mc_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS, field_id))
    {
                return p_sys_spec->l2mc_member_ports;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS, field_id))
    {
                return p_sys_spec->l3mc_member_ports;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS, field_id))
    {
                return p_sys_spec->vlan_member_ports;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VOICE_VLAN, field_id))
    {
                return p_sys_spec->voice_vlan;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS, field_id))
    {
                return p_sys_spec->flow_entries_ingress;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS, field_id))
    {
                return p_sys_spec->flow_entries_egress;
    }
	if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS, field_id))
    {
                return p_sys_spec->flow6_entries_ingress;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS, field_id))
    {
                return p_sys_spec->flow6_entries_egress;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS, field_id))
    {
                return p_sys_spec->cfm_local_and_remote_meps;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8031_GROUPS, field_id))
    {
                return p_sys_spec->g8031_groups;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_RINGS, field_id))
    {
                return p_sys_spec->g8032_rings;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING, field_id))
    {
                return p_sys_spec->g8032_member_ports_per_ring;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FTN_ENTRIES, field_id))
    {
                return p_sys_spec->ftn_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ILM_ENTRIES, field_id))
    {
                return p_sys_spec->ilm_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_LABLES, field_id))
    {
                return p_sys_spec->mpls_lables;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPWS, field_id))
    {
                return p_sys_spec->vpws;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_PE, field_id))
    {
                return p_sys_spec->lsp_pe;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_P, field_id))
    {
                return p_sys_spec->lsp_p;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_PEER, field_id))
    {
                return p_sys_spec->vpls_peer;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_AC, field_id))
    {
                return p_sys_spec->vpls_ac;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VSI, field_id))
    {
                return p_sys_spec->vsi;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_OAM, field_id))
    {
                return p_sys_spec->lsp_oam;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PW_OAM, field_id))
    {
                return p_sys_spec->pw_oam;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL, field_id))
    {
                return p_sys_spec->mpls_aps_tunnel;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LCK, field_id))
    {
                return p_sys_spec->cfm_lck;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IP_TUNNEL, field_id))
    {
                return p_sys_spec->ip_tunnel;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IVI_PEERS, field_id))
    {
                return p_sys_spec->ivi_peers;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_V6_ROUTES, field_id))
    {
                return p_sys_spec->host_v6_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES, field_id))
    {
                return p_sys_spec->ecmp_v6_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES, field_id))
    {
                return p_sys_spec->pbr_v6_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES, field_id))
    {
                return p_sys_spec->l3mc_v6_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES, field_id))
    {
                return p_sys_spec->l2mc_v6_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES, field_id))
    {
                return p_sys_spec->flow_v6_entries;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS, field_id))
    {
                return p_sys_spec->ipv6_based_vlan_class;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD, field_id))
    {
                return p_sys_spec->ipv6_source_guard;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES, field_id))
    {
                return p_sys_spec->indirect_v6_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES, field_id))
    {
                return p_sys_spec->static_v6_routes;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS, field_id))
    {
                return p_sys_spec->l3mc_v6_member_ports;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS, field_id))
    {
                return p_sys_spec->l2mc_v6_member_ports;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY, field_id))
    {
                return p_sys_spec->vlan_mapping_default_entry;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_BFD_SESSIONS, field_id))
    {
                return p_sys_spec->bfd_sessions;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS, field_id))
    {
                return p_sys_spec->nvgre_tunnel_id_mappings;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_PEERS, field_id))
    {
                return p_sys_spec->nvgre_peers;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNELS, field_id))
    {
                return p_sys_spec->nvgre_tunnels;
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES, field_id))
    {
                return p_sys_spec->openflow_flow_entries;
    }
    return PM_E_NONE;
}
