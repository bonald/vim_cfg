
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_scl.h"
#include "ctc_sai_port.h"

static ctclib_opf_t *g_pst_ctc_drv_scl_master = NULL;

extern sai_status_t
ctc_sai_for_each_port_object(port_object_cb fn, void* pdata);

int32
ctc_ttp_drv_scl_entry_id_alloc(uint32 *entry_id)
{
    uint32 entry_index = 0;
    entry_index = ctclib_opf_alloc_offset(g_pst_ctc_drv_scl_master, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == entry_index)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *entry_id = entry_index;

    return HSRV_E_NONE;
}

int32
ctc_ttp_drv_scl_entry_id_free(uint32 entry_id)
{
    return ctclib_opf_free_offset(g_pst_ctc_drv_scl_master, 0, 1, entry_id);
}

int32
ctc_ttp_drv_scl_vlan_mapping_add(hsrv_openflow_ttp_flow_t *p_flow)
{   
    int32 rc = 0;
    metadata_mapping_t  *p_metadata = NULL;
    ctc_scl_entry_t scl_entry;
    hsrv_vlan_flow_entry_t  *p_entry =  &p_flow->flow.flow_data.vlan_flow_entry;
    hsrv_openflow_ttp_port_info_t *p_port = NULL;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    p_port = hsrv_openflow_port_get_port_info_by_portid(p_entry->match_criteria.inport);
    if (!p_port)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }

    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_SVLAN;
    scl_entry.key.u.hash_port_svlan_key.gport = p_port->class_id;
    scl_entry.key.u.hash_port_svlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT_CLASS;
    scl_entry.key.u.hash_port_svlan_key.svlan = p_entry->match_criteria.vlan_id;
    scl_entry.key.u.hash_port_svlan_key.dir = CTC_INGRESS;

    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;

    if (p_flow->stats_supported)
    {
        p_stats = hsrv_openflow_get_stats_info(p_flow->stats_id);
        if (p_stats)
        {
            GLB_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_STATS);
            scl_entry.action.u.igs_action.stats_id = p_stats->ctc_stats_id;
        }
    }    
    hsrv_openflow_lookup_metadata_mapping(p_entry->metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    scl_entry.action.u.igs_action.fid = p_metadata->u.fid;
    GLB_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    scl_entry.entry_id = p_flow->entry_id;

    rc = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_SVLAN, &scl_entry);
    if (rc)
    {
        return rc;
    }
    rc = ctc_scl_install_entry(scl_entry.entry_id);

    return rc;
}

int32
ctc_ttp_drv_scl_terminal_mac_flow_add(hsrv_openflow_ttp_flow_t *p_flow)
{
    int32 rc = 0;
    metadata_mapping_t  *p_metadata = NULL;
    ctc_scl_entry_t scl_entry;
    hsrv_termination_mac_flow_entry_t *p_entry = &p_flow->flow.flow_data.termination_mac_flow_entry;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    sal_memset(&scl_entry, 0x0, sizeof(ctc_scl_entry_t));
    
    scl_entry.entry_id = p_flow->entry_id;
    scl_entry.key.type = CTC_SCL_KEY_TCAM_MAC;
    scl_entry.key.u.tcam_mac_key.flag |= CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_DA;
    sal_memcpy(&scl_entry.key.u.tcam_mac_key.mac_da, p_entry->match_criteria.dst_mac, sizeof(mac_addr_t));
    sal_memset(&scl_entry.key.u.tcam_mac_key.mac_da_mask, 0xFF, sizeof(mac_addr_t));

    scl_entry.action.type = CTC_SCL_ACTION_FLOW;
    GLB_SET_FLAG(scl_entry.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_FORCE_ROUTE);
    GLB_SET_FLAG(scl_entry.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_VRFID);
    hsrv_openflow_lookup_metadata_mapping(p_entry->metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    scl_entry.action.u.flow_action.vrf_id = p_metadata->u.vrf_id;
    if (p_flow->stats_supported)
    {
        p_stats = hsrv_openflow_get_stats_info(p_flow->stats_id);
        if (p_stats)
        {
            GLB_SET_FLAG(scl_entry.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_STATS);
            scl_entry.action.u.flow_action.stats_id = p_stats->ctc_stats_id;
        }
    }    
    rc = ctc_scl_add_entry(TTP_GLB_SCL_GROUP, &scl_entry);
    if (rc)
    {
        return rc;
    }
    rc = ctc_scl_install_entry(scl_entry.entry_id);

    return rc;
}

int32
ctc_ttp_drv_scl_entry_remove(hsrv_openflow_ttp_flow_t *p_flow)
{
    int32 rc = 0;
    rc = ctc_scl_uninstall_entry(p_flow->entry_id);
    if (rc)
    {
        return rc;
    }
    rc = ctc_scl_remove_entry(p_flow->entry_id);
    
    return rc;
}

static int32
ctc_ttp_drv_scl_set_default_entry_cb(sai_object_id_t port_id, void* pdata)
{
    ctc_scl_default_action_t dft_action;
    uint32 gport = 0;

    gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
    sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
    
    dft_action.gport = gport;
    dft_action.action.type = CTC_SCL_ACTION_INGRESS;
    GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
    GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_STATS);
    dft_action.action.u.igs_action.stats_id = g_pst_ttp_vlan_mapping_flow_table_miss_stats_id;

    return ctc_scl_set_default_action(&dft_action);
}

static int32
ctc_ttp_drv_global_scl_set_default_entry()
{
    uint32 rc = 0;
    ctc_scl_entry_t entry;
    uint32 entry_id = 0;
    
    sal_memset(&entry, 0, sizeof(ctc_scl_entry_t));
    ctc_ttp_drv_scl_entry_id_alloc(&entry_id);

    entry.key.type = CTC_SCL_KEY_TCAM_MAC;
    entry.priority = 0;
    entry.priority_valid = 1;
    entry.entry_id = entry_id;
    entry.action.type = CTC_SCL_ACTION_FLOW;
    GLB_SET_FLAG(entry.action.u.flow_action.flag, CTC_SCL_FLOW_ACTION_FLAG_DENY_ROUTE);

    rc = ctc_scl_add_entry(TTP_GLB_SCL_GROUP, &entry);
    if (rc)
    {
        return rc;
    }

    return rc = ctc_scl_install_entry(entry.entry_id);
}


int32
ctc_ttp_drv_scl_init()
{ 
    int32 ret = 0;
    g_pst_ctc_drv_scl_master = ctclib_opf_init(1);
    HSRV_OPENFLOW_MEM_CHECK(g_pst_ctc_drv_scl_master);
    ctclib_opf_init_offset(g_pst_ctc_drv_scl_master, 0, CTC_SCL_ENTRY_ID_MIN, CTC_SCL_ENTRY_ID_NUM);

    /* create global scl tcam group */
    ctc_scl_group_info_t group_info;

    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.priority = 0;
    group_info.type = CTC_SCL_GROUP_TYPE_GLOBAL;
    ctc_scl_create_group(TTP_GLB_SCL_GROUP, &group_info);
    ctc_scl_install_group(TTP_GLB_SCL_GROUP, &group_info);

    /* set golbal tcam scl defalut entry ,later may be init in hsrv or ovs*/
    ctc_ttp_drv_global_scl_set_default_entry();

    /* set port-base scl default action */
    ret = ctc_sai_for_each_port_object(ctc_ttp_drv_scl_set_default_entry_cb, NULL);

    return ret;
}


