
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_scl.h"
#include "ctc_sai_port.h"

static ctclib_opf_t *g_pst_ctc_drv_scl_master = NULL;
static ctclib_hash_t* g_pst_ctc_drv_scl_vxlan_tunnel_hash; /* vxlan_tunnel_t */

//extern sai_status_t
//ctc_sai_for_each_port_object(port_object_cb fn, void* pdata);

#define ____SCL_DB_FUCTION____

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

static uint32
ctc_ttp_drv_scl_vxlan_tunnel_make(vxlan_tunnel_t* p_vxlan_tunnel)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_vxlan_tunnel->key;
    for (index = 0; index < sizeof(p_vxlan_tunnel->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
ctc_ttp_drv_scl_vxlan_tunnel_compare(void *p_arg1, void *p_arg2)
{
    vxlan_tunnel_t *p_vxlan_tunnel1 = (vxlan_tunnel_t*)p_arg1;
    vxlan_tunnel_t *p_vxlan_tunnel2 = (vxlan_tunnel_t*)p_arg2;

    if (0 == sal_memcmp(&p_vxlan_tunnel1->key, &p_vxlan_tunnel2->key, sizeof(vxlan_tunnel_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
ctc_ttp_drv_scl_lookup_vxlan_tunnel_db(vxlan_tunnel_key_t* key, vxlan_tunnel_t** pp_vxlan_tunnel)
{
    vxlan_tunnel_t lookup_key;

    sal_memcpy(&lookup_key.key, key, sizeof(vxlan_tunnel_key_t));

    *pp_vxlan_tunnel = ctclib_hash_lookup(g_pst_ctc_drv_scl_vxlan_tunnel_hash, &lookup_key);

    return HSRV_E_NONE;
}

int32
ctc_ttp_drv_scl_add_vxlan_tunnel_db(vxlan_tunnel_t* p_vxlan_tunnel)
{
    ctclib_hash_get(g_pst_ctc_drv_scl_vxlan_tunnel_hash, p_vxlan_tunnel, ctclib_hash_alloc_intern);

    return HSRV_E_NONE;
}

int32
ctc_ttp_drv_scl_del_vxlan_tunnel_db(vxlan_tunnel_key_t* key)
{
    vxlan_tunnel_t* p_vxlan_tunnel;
    
    ctc_ttp_drv_scl_lookup_vxlan_tunnel_db(key, &p_vxlan_tunnel);
    if (p_vxlan_tunnel)
    {
        ctclib_hash_release(g_pst_ctc_drv_scl_vxlan_tunnel_hash, p_vxlan_tunnel);
    }
    
    return HSRV_E_NONE;
}

int32
ctc_ttp_drv_scl_vxlan_tunnel_db_init()
{
    g_pst_ctc_drv_scl_vxlan_tunnel_hash = ctclib_hash_create(
        ctc_ttp_drv_scl_vxlan_tunnel_make,
        ctc_ttp_drv_scl_vxlan_tunnel_compare);
    HSRV_OPENFLOW_MEM_CHECK(g_pst_ctc_drv_scl_vxlan_tunnel_hash);

    return HSRV_E_NONE;
}

#define ____SCL_API_FUNCTION____

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

int32
ctc_ttp_drv_scl_vxlan_tunnel_add(hsrv_openflow_ttp_flow_t *p_flow)
{
    int32 ret = 0;
    ctc_overlay_tunnel_param_t  tunnel_param;
    //metadata_mapping_t  *p_metadata = NULL;
    hsrv_ingress_port_flow_entry_t *p_entry = &p_flow->flow.flow_data.ingress_port_flow_entry;
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = NULL;
    hsrv_openflow_ttp_stats_info_t *p_stats = NULL;
    vxlan_tunnel_key_t p_tunnel_key;
    vxlan_tunnel_t *p_vxlan_tunnel = NULL;
    

    sal_memset(&tunnel_param, 0, sizeof(ctc_overlay_tunnel_param_t));
    sal_memset(&p_tunnel_key, 0, sizeof(vxlan_tunnel_key_t));
#if 0
    hsrv_openflow_lookup_metadata_mapping(p_entry->metadata, &p_metadata);
    if (!p_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
#endif
    p_tunnel = hsrv_openflow_tunnel_get_tunnel_info(p_entry->match_criteria.ifindex);
    if (!p_tunnel)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    p_tunnel_key.ipsa = p_tunnel->tunnel_info.remote_ip;
    p_tunnel_key.ipda = p_tunnel->tunnel_info.local_ip;
    p_tunnel_key.vni = p_entry->match_criteria.vni;
    ctc_ttp_drv_scl_lookup_vxlan_tunnel_db(&p_tunnel_key, &p_vxlan_tunnel);
    if (p_vxlan_tunnel)
    {
        //this type of vlxna tunnel decap flow has been add, not need to add again
        p_vxlan_tunnel->ref_cnt ++;
        return 0;
    }

    p_vxlan_tunnel = HSRV_OPENFLOW_FLOW_MALLOC(sizeof(vxlan_tunnel_t));
    HSRV_OPENFLOW_MEM_CHECK(p_vxlan_tunnel);
    sal_memset(p_vxlan_tunnel, 0, sizeof(vxlan_tunnel_t));
    sal_memcpy(&p_vxlan_tunnel->key, &p_tunnel_key, sizeof(vxlan_tunnel_key_t));
    p_vxlan_tunnel->ref_cnt ++;
    ctc_ttp_drv_scl_add_vxlan_tunnel_db(p_vxlan_tunnel);

    tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    tunnel_param.scl_id = 0;
    tunnel_param.ipsa.ipv4 = p_tunnel->tunnel_info.remote_ip;
    tunnel_param.ipda.ipv4 = p_tunnel->tunnel_info.local_ip;
    tunnel_param.src_vn_id = p_entry->match_criteria.vni;
    if (p_flow->stats_supported)
    {
        p_stats = hsrv_openflow_get_stats_info(p_flow->stats_id);
        if (p_stats)
        {
            GLB_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_STATS_EN);
            tunnel_param.stats_id = p_stats->ctc_stats_id;
        }
    }
    GLB_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA);
    tunnel_param.logic_port_type = 1;
    /* metadata low 32 bits  is vni, in metadata_mapping we create vni and fid mapping */
    tunnel_param.action.dst_vn_id = (uint32)p_entry->metadata;
    if (p_entry->openflow_metadata)
    {
        tunnel_param.metadata = p_entry->openflow_metadata;
        GLB_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN);
    }
    
    ret = ctc_overlay_tunnel_add_tunnel(&tunnel_param);
    
    return ret;
}

int32
ctc_ttp_drv_scl_vxlan_tunnel_del(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_overlay_tunnel_param_t  tunnel_param;
    hsrv_ingress_port_flow_entry_t *p_entry = &p_flow->flow.flow_data.ingress_port_flow_entry;
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = NULL;
    vxlan_tunnel_key_t p_tunnel_key;
    vxlan_tunnel_t *p_vxlan_tunnel = NULL;

    sal_memset(&tunnel_param, 0, sizeof(tunnel_param));
    sal_memset(&p_tunnel_key, 0, sizeof(vxlan_tunnel_key_t));
    p_tunnel = hsrv_openflow_tunnel_get_tunnel_info(p_entry->match_criteria.ifindex);
    if (!p_tunnel)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    p_tunnel_key.ipsa = p_tunnel->tunnel_info.remote_ip;
    p_tunnel_key.ipda = p_tunnel->tunnel_info.local_ip;
    p_tunnel_key.vni = p_entry->match_criteria.vni;
    ctc_ttp_drv_scl_lookup_vxlan_tunnel_db(&p_tunnel_key, &p_vxlan_tunnel);
    if (!p_vxlan_tunnel)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    if (p_vxlan_tunnel->ref_cnt > 1)
    {
        p_vxlan_tunnel->ref_cnt --;
        return 0;
    }
    else
    {
        ctc_ttp_drv_scl_del_vxlan_tunnel_db(&p_tunnel_key);
    }

    tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    tunnel_param.scl_id = 0;
    tunnel_param.ipsa.ipv4 = p_tunnel->tunnel_info.remote_ip;
    tunnel_param.ipda.ipv4 = p_tunnel->tunnel_info.local_ip;
    tunnel_param.src_vn_id = p_entry->match_criteria.vni;
    GLB_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA);

    return ctc_overlay_tunnel_remove_tunnel(&tunnel_param);
}

int32
ctc_ttp_drv_scl_set_port_default_entry(uint32 gport)
{    
    ctc_scl_default_action_t dft_action;

    sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
    
    dft_action.gport = gport;
    dft_action.action.type = CTC_SCL_ACTION_INGRESS;
    GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
    GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_STATS);
    dft_action.action.u.igs_action.stats_id = g_pst_ttp_vlan_mapping_flow_table_miss_stats_id;

    return ctc_scl_set_default_action(&dft_action);
}

int32
ctc_ttp_drv_scl_unset_port_default_entry(uint32 gport)
{    
    ctc_scl_default_action_t dft_action;
    
    sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
    dft_action.gport = gport;
    dft_action.action.type = CTC_SCL_ACTION_INGRESS;

    return ctc_scl_set_default_action(&dft_action);
}

static int32
ctc_ttp_drv_global_scl_set_default_entry()
{
    return 0;
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

#define ____SCL_INIT____

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

    /* set global tcam scl defalut entry ,later may be init in hsrv or ovs*/
    ctc_ttp_drv_global_scl_set_default_entry();

    /* set port-base scl default action */

    //ret = ctc_sai_for_each_port_object(ctc_ttp_drv_scl_set_default_entry_cb, NULL);

    /*  default vxlan decap mode: ipsa+ipda+vni */
#if 0
    /* set vxlan tunnel decap mode:ipda+vni */
    ctc_overlay_tunnel_global_cfg_t init_param;
    sal_memset(&init_param, 0, sizeof(ctc_overlay_tunnel_global_cfg_t));
    init_param.vxlan_mode = CTC_OVERLAY_TUNNEL_DECAP_BY_IPDA_VNI;
    ctc_overlay_tunnel_init(&init_param);
#endif

    ctc_ttp_drv_scl_vxlan_tunnel_db_init();

    return ret;
}


