
#include "proto.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "cdb_data_cmp.h"

tbl_sys_spec_master_t *_g_p_tbl_sys_spec_master = NULL;

int32
tbl_sys_spec_add_sys_spec_sync(tbl_sys_spec_t *p_sys_spec, uint32 sync)
{
    tbl_sys_spec_master_t *p_master = _g_p_tbl_sys_spec_master;
    tbl_sys_spec_t *p_db_sys_spec = NULL;

    /* 1. lookup entry exist */
    if (tbl_sys_spec_get_sys_spec(&p_sys_spec->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_sys_spec = XCALLOC(MEM_TBL_SYS_SPEC, sizeof(tbl_sys_spec_t));
    if (NULL == p_db_sys_spec)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_sys_spec, p_sys_spec, sizeof(tbl_sys_spec_t));

    /* 4. add to db */
    p_master->sys_spec_array[p_sys_spec->key.type] = p_db_sys_spec;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SYS_SPEC, p_db_sys_spec);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SYS_SPEC, p_db_sys_spec);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sys_spec_del_sys_spec_sync(tbl_sys_spec_key_t *p_sys_spec_key, uint32 sync)
{
    tbl_sys_spec_master_t *p_master = _g_p_tbl_sys_spec_master;
    tbl_sys_spec_t *p_db_sys_spec = NULL;

    /* 1. lookup entry exist */
    p_db_sys_spec = tbl_sys_spec_get_sys_spec(p_sys_spec_key);
    if (NULL == p_db_sys_spec)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SYS_SPEC, p_db_sys_spec);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SYS_SPEC, p_db_sys_spec);
        #endif 
    }

    /* 3. delete from db */
    p_master->sys_spec_array[p_sys_spec_key->type] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_SYS_SPEC, p_db_sys_spec);

    return PM_E_NONE;
}

int32
tbl_sys_spec_set_sys_spec_field_sync(tbl_sys_spec_t *p_sys_spec, tbl_sys_spec_field_id_t field_id, uint32 sync)
{
    tbl_sys_spec_t *p_db_sys_spec = NULL;

    /* 1. lookup entry exist */
    p_db_sys_spec = tbl_sys_spec_get_sys_spec(&p_sys_spec->key);
    if (NULL == p_db_sys_spec)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SYS_SPEC_FLD_UCAST_FDB:
        p_db_sys_spec->ucast_fdb = p_sys_spec->ucast_fdb;
        break;

    case TBL_SYS_SPEC_FLD_STATIC_FDB:
        p_db_sys_spec->static_fdb = p_sys_spec->static_fdb;
        break;

    case TBL_SYS_SPEC_FLD_MSTP_INSTANCE:
        p_db_sys_spec->mstp_instance = p_sys_spec->mstp_instance;
        break;

    case TBL_SYS_SPEC_FLD_VLAN_INSTANCE:
        p_db_sys_spec->vlan_instance = p_sys_spec->vlan_instance;
        break;

    case TBL_SYS_SPEC_FLD_MAC_FILTER:
        p_db_sys_spec->mac_filter = p_sys_spec->mac_filter;
        break;

    case TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS:
        p_db_sys_spec->mac_based_vlan_class = p_sys_spec->mac_based_vlan_class;
        break;

    case TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS:
        p_db_sys_spec->ipv4_based_vlan_class = p_sys_spec->ipv4_based_vlan_class;
        break;

    case TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD:
        p_db_sys_spec->ipv4_source_guard = p_sys_spec->ipv4_source_guard;
        break;

    case TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT:
        p_db_sys_spec->vlan_mapping_entry_applied_port = p_sys_spec->vlan_mapping_entry_applied_port;
        break;

    case TBL_SYS_SPEC_FLD_DOT1X_MAC:
        p_db_sys_spec->dot1x_mac = p_sys_spec->dot1x_mac;
        break;

    case TBL_SYS_SPEC_FLD_INDIRECT_ROUTES:
        p_db_sys_spec->indirect_routes = p_sys_spec->indirect_routes;
        break;

    case TBL_SYS_SPEC_FLD_STATIC_ROUTES:
        p_db_sys_spec->static_routes = p_sys_spec->static_routes;
        break;

    case TBL_SYS_SPEC_FLD_HOST_ROUTES:
        p_db_sys_spec->host_routes = p_sys_spec->host_routes;
        break;

    case TBL_SYS_SPEC_FLD_ECMP_ROUTES:
        p_db_sys_spec->ecmp_routes = p_sys_spec->ecmp_routes;
        break;

    case TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM:
        p_db_sys_spec->ecmp_member_maximum = p_sys_spec->ecmp_member_maximum;
        break;

    case TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES:
        p_db_sys_spec->indirect_v6_routes = p_sys_spec->indirect_v6_routes;
        break;

    case TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES:
        p_db_sys_spec->static_v6_routes = p_sys_spec->static_v6_routes;
        break;

    case TBL_SYS_SPEC_FLD_HOST_V6_ROUTES:
        p_db_sys_spec->host_v6_routes = p_sys_spec->host_v6_routes;
        break;

    case TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES:
        p_db_sys_spec->ecmp_v6_routes = p_sys_spec->ecmp_v6_routes;
        break;

    case TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM:
        p_db_sys_spec->ecmp_v6_member_maximum = p_sys_spec->ecmp_v6_member_maximum;
        break;

    case TBL_SYS_SPEC_FLD_PBR_ENTRIES:
        p_db_sys_spec->pbr_entries = p_sys_spec->pbr_entries;
        break;

    case TBL_SYS_SPEC_FLD_L2MC_ENTRIES:
        p_db_sys_spec->l2mc_entries = p_sys_spec->l2mc_entries;
        break;

    case TBL_SYS_SPEC_FLD_L3MC_ENTRIES:
        p_db_sys_spec->l3mc_entries = p_sys_spec->l3mc_entries;
        break;

    case TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS:
        p_db_sys_spec->l2mc_member_ports = p_sys_spec->l2mc_member_ports;
        break;

    case TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS:
        p_db_sys_spec->l3mc_member_ports = p_sys_spec->l3mc_member_ports;
        break;

    case TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS:
        p_db_sys_spec->vlan_member_ports = p_sys_spec->vlan_member_ports;
        break;

    case TBL_SYS_SPEC_FLD_VOICE_VLAN:
        p_db_sys_spec->voice_vlan = p_sys_spec->voice_vlan;
        break;

    case TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS:
        p_db_sys_spec->flow_entries_ingress = p_sys_spec->flow_entries_ingress;
        break;

    case TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS:
        p_db_sys_spec->flow_entries_egress = p_sys_spec->flow_entries_egress;
        break;

    case TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS:
        p_db_sys_spec->flow6_entries_ingress = p_sys_spec->flow6_entries_ingress;
        break;

    case TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS:
        p_db_sys_spec->flow6_entries_egress = p_sys_spec->flow6_entries_egress;
        break;

    case TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS:
        p_db_sys_spec->cfm_local_and_remote_meps = p_sys_spec->cfm_local_and_remote_meps;
        break;

    case TBL_SYS_SPEC_FLD_G8031_GROUPS:
        p_db_sys_spec->g8031_groups = p_sys_spec->g8031_groups;
        break;

    case TBL_SYS_SPEC_FLD_G8032_RINGS:
        p_db_sys_spec->g8032_rings = p_sys_spec->g8032_rings;
        break;

    case TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING:
        p_db_sys_spec->g8032_member_ports_per_ring = p_sys_spec->g8032_member_ports_per_ring;
        break;

    case TBL_SYS_SPEC_FLD_FTN_ENTRIES:
        p_db_sys_spec->ftn_entries = p_sys_spec->ftn_entries;
        break;

    case TBL_SYS_SPEC_FLD_ILM_ENTRIES:
        p_db_sys_spec->ilm_entries = p_sys_spec->ilm_entries;
        break;

    case TBL_SYS_SPEC_FLD_MPLS_LABLES:
        p_db_sys_spec->mpls_lables = p_sys_spec->mpls_lables;
        break;

    case TBL_SYS_SPEC_FLD_VPWS:
        p_db_sys_spec->vpws = p_sys_spec->vpws;
        break;

    case TBL_SYS_SPEC_FLD_LSP_PE:
        p_db_sys_spec->lsp_pe = p_sys_spec->lsp_pe;
        break;

    case TBL_SYS_SPEC_FLD_LSP_P:
        p_db_sys_spec->lsp_p = p_sys_spec->lsp_p;
        break;

    case TBL_SYS_SPEC_FLD_VPLS_PEER:
        p_db_sys_spec->vpls_peer = p_sys_spec->vpls_peer;
        break;

    case TBL_SYS_SPEC_FLD_VPLS_AC:
        p_db_sys_spec->vpls_ac = p_sys_spec->vpls_ac;
        break;

    case TBL_SYS_SPEC_FLD_VSI:
        p_db_sys_spec->vsi = p_sys_spec->vsi;
        break;

    case TBL_SYS_SPEC_FLD_LSP_OAM:
        p_db_sys_spec->lsp_oam = p_sys_spec->lsp_oam;
        break;

    case TBL_SYS_SPEC_FLD_PW_OAM:
        p_db_sys_spec->pw_oam = p_sys_spec->pw_oam;
        break;

    case TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL:
        p_db_sys_spec->mpls_aps_tunnel = p_sys_spec->mpls_aps_tunnel;
        break;

    case TBL_SYS_SPEC_FLD_CFM_LCK:
        p_db_sys_spec->cfm_lck = p_sys_spec->cfm_lck;
        break;

    case TBL_SYS_SPEC_FLD_IP_TUNNEL:
        p_db_sys_spec->ip_tunnel = p_sys_spec->ip_tunnel;
        break;

    case TBL_SYS_SPEC_FLD_IVI_PEERS:
        p_db_sys_spec->ivi_peers = p_sys_spec->ivi_peers;
        break;

    case TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES:
        p_db_sys_spec->pbr_v6_entries = p_sys_spec->pbr_v6_entries;
        break;

    case TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES:
        p_db_sys_spec->l3mc_v6_entries = p_sys_spec->l3mc_v6_entries;
        break;

    case TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES:
        p_db_sys_spec->l2mc_v6_entries = p_sys_spec->l2mc_v6_entries;
        break;

    case TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES:
        p_db_sys_spec->flow_v6_entries = p_sys_spec->flow_v6_entries;
        break;

    case TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS:
        p_db_sys_spec->ipv6_based_vlan_class = p_sys_spec->ipv6_based_vlan_class;
        break;

    case TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD:
        p_db_sys_spec->ipv6_source_guard = p_sys_spec->ipv6_source_guard;
        break;

    case TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS:
        p_db_sys_spec->l3mc_v6_member_ports = p_sys_spec->l3mc_v6_member_ports;
        break;

    case TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS:
        p_db_sys_spec->l2mc_v6_member_ports = p_sys_spec->l2mc_v6_member_ports;
        break;

    case TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY:
        p_db_sys_spec->vlan_mapping_default_entry = p_sys_spec->vlan_mapping_default_entry;
        break;

    case TBL_SYS_SPEC_FLD_BFD_SESSIONS:
        p_db_sys_spec->bfd_sessions = p_sys_spec->bfd_sessions;
        break;

    case TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS:
        p_db_sys_spec->nvgre_tunnel_id_mappings = p_sys_spec->nvgre_tunnel_id_mappings;
        break;

    case TBL_SYS_SPEC_FLD_NVGRE_PEERS:
        p_db_sys_spec->nvgre_peers = p_sys_spec->nvgre_peers;
        break;

    case TBL_SYS_SPEC_FLD_NVGRE_TUNNELS:
        p_db_sys_spec->nvgre_tunnels = p_sys_spec->nvgre_tunnels;
        break;

    case TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES:
        p_db_sys_spec->openflow_flow_entries = p_sys_spec->openflow_flow_entries;
        break;

    case TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM:
        p_db_sys_spec->policer_profile_num = p_sys_spec->policer_profile_num;
        break;

    case TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM:
        p_db_sys_spec->port_shape_profile_num = p_sys_spec->port_shape_profile_num;
        break;

    case TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM:
        p_db_sys_spec->queue_shape_profile_num = p_sys_spec->queue_shape_profile_num;
        break;

    case TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM:
        p_db_sys_spec->drop_profile_num = p_sys_spec->drop_profile_num;
        break;

    case TBL_SYS_SPEC_FLD_LAG_GROUP_NUM:
        p_db_sys_spec->lag_group_num = p_sys_spec->lag_group_num;
        break;

    case TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM:
        p_db_sys_spec->ospf_area_range_num = p_sys_spec->ospf_area_range_num;
        break;

    case TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM:
        p_db_sys_spec->tap_group_profile_num = p_sys_spec->tap_group_profile_num;
        break;

    case TBL_SYS_SPEC_FLD_TAP_FLOW_NUM:
        p_db_sys_spec->tap_flow_num = p_sys_spec->tap_flow_num;
        break;

    case TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM:
        p_db_sys_spec->tap_flow_entry_num = p_sys_spec->tap_flow_entry_num;
        break;

    case TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM:
        p_db_sys_spec->l2protocol_num = p_sys_spec->l2protocol_num;
        break;

    case TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM:
        p_db_sys_spec->openflow_dynamic_tunnel_num = p_sys_spec->openflow_dynamic_tunnel_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_POOL_NUM:
        p_db_sys_spec->nat_pool_num = p_sys_spec->nat_pool_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_RULE_NUM:
        p_db_sys_spec->nat_rule_num = p_sys_spec->nat_rule_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM:
        p_db_sys_spec->nat_inside_rtif_num = p_sys_spec->nat_inside_rtif_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM:
        p_db_sys_spec->nat_outside_rtif_num = p_sys_spec->nat_outside_rtif_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM:
        p_db_sys_spec->nat_session_upstream_num = p_sys_spec->nat_session_upstream_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM:
        p_db_sys_spec->nat_session_dnstream_num = p_sys_spec->nat_session_dnstream_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM:
        p_db_sys_spec->nat_session_fast_path_entry_num = p_sys_spec->nat_session_fast_path_entry_num;
        break;

    case TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM:
        p_db_sys_spec->nat_session_limit_node_num = p_sys_spec->nat_session_limit_node_num;
        break;

    case TBL_SYS_SPEC_FLD_MAX:
        sal_memcpy(p_db_sys_spec, p_sys_spec, sizeof(tbl_sys_spec_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SYS_SPEC, field_id, p_db_sys_spec);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SYS_SPEC, field_id, p_db_sys_spec);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sys_spec_add_sys_spec(tbl_sys_spec_t *p_sys_spec)
{
    return tbl_sys_spec_add_sys_spec_sync(p_sys_spec, TRUE);
}

int32
tbl_sys_spec_del_sys_spec(tbl_sys_spec_key_t *p_sys_spec_key)
{
    return tbl_sys_spec_del_sys_spec_sync(p_sys_spec_key, TRUE);
}

int32
tbl_sys_spec_set_sys_spec_field(tbl_sys_spec_t *p_sys_spec, tbl_sys_spec_field_id_t field_id)
{
    return tbl_sys_spec_set_sys_spec_field_sync(p_sys_spec, field_id, TRUE);
}

tbl_sys_spec_t*
tbl_sys_spec_get_sys_spec(tbl_sys_spec_key_t *p_sys_spec_key)
{
    tbl_sys_spec_master_t *p_master = _g_p_tbl_sys_spec_master;

    if (GLB_STM_MODE_MAX <= p_sys_spec_key->type)
    {
        return NULL;
    }
    return (p_master->sys_spec_array[p_sys_spec_key->type]);
}

char*
tbl_sys_spec_key_val2str(tbl_sys_spec_t *p_sys_spec, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_sys_spec->key.type);
    return str;
}

int32
tbl_sys_spec_key_str2val(char *str, tbl_sys_spec_t *p_sys_spec)
{
    int32 ret = 0;

    p_sys_spec->key.type = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_sys_spec_key_name_dump(tbl_sys_spec_t *p_sys_spec, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_SPEC);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_sys_spec->key.type);
    return str;
}

char*
tbl_sys_spec_key_value_dump(tbl_sys_spec_t *p_sys_spec, char *str)
{
    sal_sprintf(str, "%u", p_sys_spec->key.type);
    return str;
}

char*
tbl_sys_spec_field_name_dump(tbl_sys_spec_t *p_sys_spec, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_SPEC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SYS_SPEC_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sys_spec_field_value_dump(tbl_sys_spec_t *p_sys_spec, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->key.type);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_UCAST_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ucast_fdb);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_FDB, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->static_fdb);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MSTP_INSTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->mstp_instance);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_INSTANCE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vlan_instance);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_FILTER, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->mac_filter);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->mac_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ipv4_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ipv4_source_guard);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vlan_mapping_entry_applied_port);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DOT1X_MAC, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->dot1x_mac);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->indirect_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->static_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->host_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ecmp_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ecmp_member_maximum);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->indirect_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->static_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->host_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ecmp_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ecmp_v6_member_maximum);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->pbr_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l2mc_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l3mc_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l2mc_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l3mc_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vlan_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VOICE_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->voice_vlan);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->flow_entries_ingress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->flow_entries_egress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->flow6_entries_ingress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->flow6_entries_egress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->cfm_local_and_remote_meps);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8031_GROUPS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->g8031_groups);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_RINGS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->g8032_rings);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->g8032_member_ports_per_ring);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FTN_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ftn_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ILM_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ilm_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_LABLES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->mpls_lables);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPWS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vpws);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_PE, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->lsp_pe);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_P, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->lsp_p);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_PEER, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vpls_peer);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_AC, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vpls_ac);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VSI, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vsi);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_OAM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->lsp_oam);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PW_OAM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->pw_oam);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->mpls_aps_tunnel);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LCK, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->cfm_lck);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IP_TUNNEL, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ip_tunnel);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IVI_PEERS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ivi_peers);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->pbr_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l3mc_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l2mc_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->flow_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ipv6_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ipv6_source_guard);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l3mc_v6_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l2mc_v6_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->vlan_mapping_default_entry);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_BFD_SESSIONS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->bfd_sessions);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nvgre_tunnel_id_mappings);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_PEERS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nvgre_peers);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNELS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nvgre_tunnels);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->openflow_flow_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->policer_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->port_shape_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->queue_shape_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->drop_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LAG_GROUP_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->lag_group_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->ospf_area_range_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->tap_group_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->tap_flow_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->tap_flow_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->l2protocol_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->openflow_dynamic_tunnel_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_POOL_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_pool_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_RULE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_rule_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_inside_rtif_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_outside_rtif_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_session_upstream_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_session_dnstream_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_session_fast_path_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_sys_spec->nat_session_limit_node_num);
    }
    return str;
}

char**
tbl_sys_spec_table_dump(tbl_sys_spec_t *p_sys_spec, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sys_spec_key_name_dump(p_sys_spec, buf));
    for(i=1; i<TBL_SYS_SPEC_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sys_spec_field_name_dump(p_sys_spec, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sys_spec_field_value_dump(p_sys_spec, i, buf));
    }
    return str;
}

int32
tbl_sys_spec_field_value_parser(char *str, int32 field_id, tbl_sys_spec_t *p_sys_spec)
{
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_KEY, field_id))
    {
        int32 ret;
        p_sys_spec->key.type = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_UCAST_FDB, field_id))
    {
        int32 ret;
        p_sys_spec->ucast_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_FDB, field_id))
    {
        int32 ret;
        p_sys_spec->static_fdb = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MSTP_INSTANCE, field_id))
    {
        int32 ret;
        p_sys_spec->mstp_instance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_INSTANCE, field_id))
    {
        int32 ret;
        p_sys_spec->vlan_instance = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_FILTER, field_id))
    {
        int32 ret;
        p_sys_spec->mac_filter = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_sys_spec->mac_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_sys_spec->ipv4_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD, field_id))
    {
        int32 ret;
        p_sys_spec->ipv4_source_guard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT, field_id))
    {
        int32 ret;
        p_sys_spec->vlan_mapping_entry_applied_port = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DOT1X_MAC, field_id))
    {
        int32 ret;
        p_sys_spec->dot1x_mac = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->indirect_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->static_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->host_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->ecmp_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM, field_id))
    {
        int32 ret;
        p_sys_spec->ecmp_member_maximum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->indirect_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->static_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_V6_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->host_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES, field_id))
    {
        int32 ret;
        p_sys_spec->ecmp_v6_routes = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM, field_id))
    {
        int32 ret;
        p_sys_spec->ecmp_v6_member_maximum = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->pbr_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->l2mc_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->l3mc_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS, field_id))
    {
        int32 ret;
        p_sys_spec->l2mc_member_ports = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS, field_id))
    {
        int32 ret;
        p_sys_spec->l3mc_member_ports = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS, field_id))
    {
        int32 ret;
        p_sys_spec->vlan_member_ports = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VOICE_VLAN, field_id))
    {
        int32 ret;
        p_sys_spec->voice_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS, field_id))
    {
        int32 ret;
        p_sys_spec->flow_entries_ingress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS, field_id))
    {
        int32 ret;
        p_sys_spec->flow_entries_egress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS, field_id))
    {
        int32 ret;
        p_sys_spec->flow6_entries_ingress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS, field_id))
    {
        int32 ret;
        p_sys_spec->flow6_entries_egress = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS, field_id))
    {
        int32 ret;
        p_sys_spec->cfm_local_and_remote_meps = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8031_GROUPS, field_id))
    {
        int32 ret;
        p_sys_spec->g8031_groups = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_RINGS, field_id))
    {
        int32 ret;
        p_sys_spec->g8032_rings = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING, field_id))
    {
        int32 ret;
        p_sys_spec->g8032_member_ports_per_ring = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FTN_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->ftn_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ILM_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->ilm_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_LABLES, field_id))
    {
        int32 ret;
        p_sys_spec->mpls_lables = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPWS, field_id))
    {
        int32 ret;
        p_sys_spec->vpws = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_PE, field_id))
    {
        int32 ret;
        p_sys_spec->lsp_pe = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_P, field_id))
    {
        int32 ret;
        p_sys_spec->lsp_p = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_PEER, field_id))
    {
        int32 ret;
        p_sys_spec->vpls_peer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_AC, field_id))
    {
        int32 ret;
        p_sys_spec->vpls_ac = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VSI, field_id))
    {
        int32 ret;
        p_sys_spec->vsi = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_OAM, field_id))
    {
        int32 ret;
        p_sys_spec->lsp_oam = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PW_OAM, field_id))
    {
        int32 ret;
        p_sys_spec->pw_oam = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL, field_id))
    {
        int32 ret;
        p_sys_spec->mpls_aps_tunnel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LCK, field_id))
    {
        int32 ret;
        p_sys_spec->cfm_lck = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IP_TUNNEL, field_id))
    {
        int32 ret;
        p_sys_spec->ip_tunnel = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IVI_PEERS, field_id))
    {
        int32 ret;
        p_sys_spec->ivi_peers = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->pbr_v6_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->l3mc_v6_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->l2mc_v6_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->flow_v6_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS, field_id))
    {
        int32 ret;
        p_sys_spec->ipv6_based_vlan_class = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD, field_id))
    {
        int32 ret;
        p_sys_spec->ipv6_source_guard = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS, field_id))
    {
        int32 ret;
        p_sys_spec->l3mc_v6_member_ports = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS, field_id))
    {
        int32 ret;
        p_sys_spec->l2mc_v6_member_ports = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY, field_id))
    {
        int32 ret;
        p_sys_spec->vlan_mapping_default_entry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_BFD_SESSIONS, field_id))
    {
        int32 ret;
        p_sys_spec->bfd_sessions = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS, field_id))
    {
        int32 ret;
        p_sys_spec->nvgre_tunnel_id_mappings = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_PEERS, field_id))
    {
        int32 ret;
        p_sys_spec->nvgre_peers = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNELS, field_id))
    {
        int32 ret;
        p_sys_spec->nvgre_tunnels = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES, field_id))
    {
        int32 ret;
        p_sys_spec->openflow_flow_entries = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->policer_profile_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->port_shape_profile_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->queue_shape_profile_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->drop_profile_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LAG_GROUP_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->lag_group_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->ospf_area_range_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->tap_group_profile_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->tap_flow_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->tap_flow_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->l2protocol_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->openflow_dynamic_tunnel_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_POOL_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_pool_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_RULE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_rule_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_inside_rtif_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_outside_rtif_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_session_upstream_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_session_dnstream_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_session_fast_path_entry_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM, field_id))
    {
        int32 ret;
        p_sys_spec->nat_session_limit_node_num = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_sys_spec_table_parser(char** array, char* key_value,tbl_sys_spec_t *p_sys_spec)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_SPEC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sys_spec_key_str2val(key_value, p_sys_spec));

    for(i=1; i<TBL_SYS_SPEC_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sys_spec_field_value_parser( array[j++], i, p_sys_spec));
    }

    return PM_E_NONE;
}

int32
tbl_sys_spec_dump_one(tbl_sys_spec_t *p_sys_spec, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_SPEC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SYS_SPEC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_KEY].name,
            p_sys_spec->key.type);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_UCAST_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_UCAST_FDB].name,
            p_sys_spec->ucast_fdb);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_FDB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_STATIC_FDB].name,
            p_sys_spec->static_fdb);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MSTP_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_MSTP_INSTANCE].name,
            p_sys_spec->mstp_instance);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_INSTANCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VLAN_INSTANCE].name,
            p_sys_spec->vlan_instance);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_FILTER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_MAC_FILTER].name,
            p_sys_spec->mac_filter);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS].name,
            p_sys_spec->mac_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS].name,
            p_sys_spec->ipv4_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IPV4_SOURCE_GUARD].name,
            p_sys_spec->ipv4_source_guard);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VLAN_MAPPING_ENTRY_APPLIED_PORT].name,
            p_sys_spec->vlan_mapping_entry_applied_port);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DOT1X_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_DOT1X_MAC].name,
            p_sys_spec->dot1x_mac);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_INDIRECT_ROUTES].name,
            p_sys_spec->indirect_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_STATIC_ROUTES].name,
            p_sys_spec->static_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_HOST_ROUTES].name,
            p_sys_spec->host_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_ECMP_ROUTES].name,
            p_sys_spec->ecmp_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_ECMP_MEMBER_MAXIMUM].name,
            p_sys_spec->ecmp_member_maximum);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_INDIRECT_V6_ROUTES].name,
            p_sys_spec->indirect_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_STATIC_V6_ROUTES].name,
            p_sys_spec->static_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_HOST_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_HOST_V6_ROUTES].name,
            p_sys_spec->host_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_ECMP_V6_ROUTES].name,
            p_sys_spec->ecmp_v6_routes);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_ECMP_V6_MEMBER_MAXIMUM].name,
            p_sys_spec->ecmp_v6_member_maximum);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_PBR_ENTRIES].name,
            p_sys_spec->pbr_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L2MC_ENTRIES].name,
            p_sys_spec->l2mc_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L3MC_ENTRIES].name,
            p_sys_spec->l3mc_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L2MC_MEMBER_PORTS].name,
            p_sys_spec->l2mc_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L3MC_MEMBER_PORTS].name,
            p_sys_spec->l3mc_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VLAN_MEMBER_PORTS].name,
            p_sys_spec->vlan_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VOICE_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VOICE_VLAN].name,
            p_sys_spec->voice_vlan);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS].name,
            p_sys_spec->flow_entries_ingress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS].name,
            p_sys_spec->flow_entries_egress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS].name,
            p_sys_spec->flow6_entries_ingress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS].name,
            p_sys_spec->flow6_entries_egress);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_CFM_LOCAL_AND_REMOTE_MEPS].name,
            p_sys_spec->cfm_local_and_remote_meps);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8031_GROUPS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_G8031_GROUPS].name,
            p_sys_spec->g8031_groups);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_RINGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_G8032_RINGS].name,
            p_sys_spec->g8032_rings);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_G8032_MEMBER_PORTS_PER_RING].name,
            p_sys_spec->g8032_member_ports_per_ring);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FTN_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FTN_ENTRIES].name,
            p_sys_spec->ftn_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_ILM_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_ILM_ENTRIES].name,
            p_sys_spec->ilm_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_LABLES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_MPLS_LABLES].name,
            p_sys_spec->mpls_lables);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPWS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VPWS].name,
            p_sys_spec->vpws);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_PE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_LSP_PE].name,
            p_sys_spec->lsp_pe);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_P, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_LSP_P].name,
            p_sys_spec->lsp_p);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_PEER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VPLS_PEER].name,
            p_sys_spec->vpls_peer);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VPLS_AC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VPLS_AC].name,
            p_sys_spec->vpls_ac);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VSI, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VSI].name,
            p_sys_spec->vsi);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LSP_OAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_LSP_OAM].name,
            p_sys_spec->lsp_oam);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PW_OAM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_PW_OAM].name,
            p_sys_spec->pw_oam);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_MPLS_APS_TUNNEL].name,
            p_sys_spec->mpls_aps_tunnel);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_CFM_LCK, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_CFM_LCK].name,
            p_sys_spec->cfm_lck);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IP_TUNNEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IP_TUNNEL].name,
            p_sys_spec->ip_tunnel);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IVI_PEERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IVI_PEERS].name,
            p_sys_spec->ivi_peers);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_PBR_V6_ENTRIES].name,
            p_sys_spec->pbr_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L3MC_V6_ENTRIES].name,
            p_sys_spec->l3mc_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L2MC_V6_ENTRIES].name,
            p_sys_spec->l2mc_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_FLOW_V6_ENTRIES].name,
            p_sys_spec->flow_v6_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS].name,
            p_sys_spec->ipv6_based_vlan_class);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_IPV6_SOURCE_GUARD].name,
            p_sys_spec->ipv6_source_guard);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L3MC_V6_MEMBER_PORTS].name,
            p_sys_spec->l3mc_v6_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L2MC_V6_MEMBER_PORTS].name,
            p_sys_spec->l2mc_v6_member_ports);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_VLAN_MAPPING_DEFAULT_ENTRY].name,
            p_sys_spec->vlan_mapping_default_entry);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_BFD_SESSIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_BFD_SESSIONS].name,
            p_sys_spec->bfd_sessions);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NVGRE_TUNNEL_ID_MAPPINGS].name,
            p_sys_spec->nvgre_tunnel_id_mappings);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_PEERS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NVGRE_PEERS].name,
            p_sys_spec->nvgre_peers);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NVGRE_TUNNELS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NVGRE_TUNNELS].name,
            p_sys_spec->nvgre_tunnels);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_OPENFLOW_FLOW_ENTRIES].name,
            p_sys_spec->openflow_flow_entries);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_POLICER_PROFILE_NUM].name,
            p_sys_spec->policer_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_PORT_SHAPE_PROFILE_NUM].name,
            p_sys_spec->port_shape_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_QUEUE_SHAPE_PROFILE_NUM].name,
            p_sys_spec->queue_shape_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_DROP_PROFILE_NUM].name,
            p_sys_spec->drop_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_LAG_GROUP_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_LAG_GROUP_NUM].name,
            p_sys_spec->lag_group_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_OSPF_AREA_RANGE_NUM].name,
            p_sys_spec->ospf_area_range_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_TAP_GROUP_PROFILE_NUM].name,
            p_sys_spec->tap_group_profile_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_TAP_FLOW_NUM].name,
            p_sys_spec->tap_flow_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_TAP_FLOW_ENTRY_NUM].name,
            p_sys_spec->tap_flow_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_L2PROTOCOL_NUM].name,
            p_sys_spec->l2protocol_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_OPENFLOW_DYNAMIC_TUNNEL_NUM].name,
            p_sys_spec->openflow_dynamic_tunnel_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_POOL_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_POOL_NUM].name,
            p_sys_spec->nat_pool_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_RULE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_RULE_NUM].name,
            p_sys_spec->nat_rule_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_INSIDE_RTIF_NUM].name,
            p_sys_spec->nat_inside_rtif_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_OUTSIDE_RTIF_NUM].name,
            p_sys_spec->nat_outside_rtif_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_SESSION_UPSTREAM_NUM].name,
            p_sys_spec->nat_session_upstream_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_SESSION_DNSTREAM_NUM].name,
            p_sys_spec->nat_session_dnstream_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_SESSION_FAST_PATH_ENTRY_NUM].name,
            p_sys_spec->nat_session_fast_path_entry_num);
    }
    if (FLD_MATCH(TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_SPEC_FLD_NAT_SESSION_LIMIT_NODE_NUM].name,
            p_sys_spec->nat_session_limit_node_num);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sys_spec_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sys_spec_master_t *p_master = _g_p_tbl_sys_spec_master;
    tbl_sys_spec_t *p_db_sys_spec = NULL;
    uint32 type = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (type = 0; type < GLB_STM_MODE_MAX; type++)
    {
        p_db_sys_spec = p_master->sys_spec_array[type];
        if (NULL == p_db_sys_spec)
        {
            continue;
        }
        rc |= fn(p_db_sys_spec, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sys_spec_master_t*
tbl_sys_spec_get_master()
{
    return _g_p_tbl_sys_spec_master;
}

tbl_sys_spec_master_t*
tbl_sys_spec_init_sys_spec()
{
    _g_p_tbl_sys_spec_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_sys_spec_master_t));
    return _g_p_tbl_sys_spec_master;
}

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

