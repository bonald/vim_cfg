
#include "proto.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "switch.h"
#include "switch_api.h"
#include "mstp_define.h"
#include "mstp_api.h"
#include "mstp.h"
#include "if_mgr.h"
#include "lldp.h"
#include "igsp_api.h"
#include "brg.h"
#include "vlanclass.h"
#include "vlan_mapping.h"
#include "acl_policy.h"
#ifdef CONFIG_MLAG
#include "mlag.h"
#include "mlag_api.h"
#endif /* CONFIG_MLAG */

int32
swth_api_add_vlan_cb(vid_t vid)
{
#ifdef CONFIG_MSTP
    mstp_api_instance_add_vlan(vid);
#endif
#ifdef CONFIG_IGSP
    igsp_api_intf_add(vid);
#endif
    return PM_E_NONE;
}

int32
swth_api_del_vlan_cb(vid_t vid)
{
#ifdef CONFIG_MSTP
    mstp_api_instance_del_vlan(vid);
    mstp_api_of_del_vlan(vid);
#endif
#ifdef CONFIG_IGSP
    igsp_api_intf_del(vid);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_add_port(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MSTP
    mstp_api_add_port(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_del_port(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MSTP
    mstp_api_del_port(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_update_if_path_cost(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MSTP
    mstp_api_update_if_path_cost(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_update_if_running(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MSTP
    mstp_api_update_if_running(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_update_if_p2p_mac(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MSTP
    mstp_api_update_if_p2p_mac(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_mstp_api_update_system_mac(uint8 *system_mac)
{
#ifdef CONFIG_MSTP
    mstp_api_update_system_mac(system_mac);
#endif
    return PM_E_NONE;
}

int32
swth_module_init()
{
#ifdef CONFIG_LLDP
# ifndef TAPPRODUCT
    lldp_init();
# endif
#endif /* CONFIG_LLDP */

#ifdef CONFIG_IGSP
    igsp_init();
#endif /* CONFIG_IGSP */

    return PM_E_NONE;
}

int32
swth_module_start(switch_master_t *master)
{
#ifdef CONFIG_MSTP
    mstp_start(master);
#endif /* CONFIG_MSTP */

#ifdef CONFIG_LLDP
    lldp_start(master);
#endif /* CONFIG_LLDP */

#ifdef CONFIG_MLAG
    mlag_start(master);
#endif /* CONFIG_MLAG */

#ifdef CONFIG_IGSP
    igsp_start(master);
#endif /* CONFIG_IGSP */

#ifdef CONFIG_VLANCLASS
    vlanclass_start(master);
#endif /* CONFIG_VLANCLASS */

#ifdef CONFIG_VLANMAPPING
    vlan_mapping_start(master);
#endif

    return PM_E_NONE;
}

int32
switch_is_mlag_peer_link(uint32 ifindex)
{
#ifdef CONFIG_MLAG
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    if (p_db_mlag->peer_link_ifindex == ifindex)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else
    return FALSE;
#endif /* !CONFIG_MLAG */
}

int32
swth_api_igsp_mroute_del_port(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_IGSP
#ifdef OFPRODUCT
    igsp_api_if_del_cb(p_db_if);
#endif
#endif
    return PM_E_NONE;
}


int32
swth_if_del_cb(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MLAG
    mlag_api_if_del_cb(p_db_if);
#endif
#ifdef CONFIG_IGSP
    igsp_api_if_del_cb(p_db_if);
#endif
    acl_api_if_del_cb(p_db_if);

    if_mgr_isolate_group_process(p_db_if, GLB_PORT_ISOLATE_GROUP_INVALID);

    return PM_E_NONE;
}

int32
swth_if_up_cb(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MLAG
    mlag_api_if_up_cb(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_if_down_cb(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_MLAG
    mlag_api_if_down_cb(p_db_if);
#endif
#ifdef CONFIG_IGSP
    igsp_api_if_down_cb(p_db_if);
#endif
    return PM_E_NONE;
}

int32
swth_if_set_allowed_vlan_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add)
{
#ifdef CONFIG_MSTP
    mstp_api_allowed_vlan_change_change_cb(p_db_if, vid, is_add);
#endif /* !CONFIG_MLAG */

#ifdef CONFIG_MLAG
    mlag_api_allowed_vlan_change_change_cb(p_db_if, vid, is_add);
#endif /* !CONFIG_MLAG */

#ifdef CONFIG_IGSP
    igsp_api_allowed_vlan_change_change_cb(p_db_if, vid, is_add);
#endif /* !CONFIG_IGSP */
    return PM_E_NONE;
}

int32
swth_if_add_to_agg_cb(tbl_interface_t *p_db_if)
{
#ifdef CONFIG_IGSP
    igsp_api_if_add_to_agg_cb(p_db_if);
#endif
   /*removed by yejl to fix bug42708, 2016-02-07*/
//    if_mgr_isolate_group_process(p_db_if, GLB_PORT_ISOLATE_GROUP_INVALID);

    return PM_E_NONE;
}

int32
swth_set_lacp_priority(uint16 priority)
{
#ifdef CONFIG_MLAG
    mlag_api_set_lacp_system_priority_cb(priority);
#endif
    return PM_E_NONE;
}

int32
swth_vlan_disable_cb(tbl_vlan_t *p_db_vlan)
{
#ifdef CONFIG_IGSP
    igsp_api_vlan_disable_cb(p_db_vlan);
#endif
    return PM_E_NONE;
}

uint32
swth_api_is_igmp_snooping_en(vid_t vid)
{
#ifdef CONFIG_IGSP
    return igsp_api_is_igmp_snooping_en(vid);
#else
    return FALSE;
#endif
}

int32
swth_api_mstp_topology_change_cb(uint32 new_topology_change_detected, uint32 ageing_time)
{
    if (new_topology_change_detected)
    {
        /* TC start, change FDB ageing time */
        brg_global_set_oper_aging_time(ageing_time);
    }
    else
    {
        /* TC stop, resume FDB ageing time */
        brg_global_set_oper_aging_time(BRG_INVALID_AGEING_TIME);
    }

#ifdef CONFIG_IGSP
    igsp_api_mstp_topology_change_cb(new_topology_change_detected);
#endif

    return PM_E_NONE;
}

int32
swth_api_get_mlag_remain_sec()
{
#ifdef CONFIG_MLAG
    return mlag_api_get_reload_delay_remain_sec();
#else
    return 0;
#endif
}

int32
swth_api_is_mlag_peer_link(uint32 ifindex)
{
#ifdef CONFIG_MLAG
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    if (p_db_mlag->peer_link_ifindex == ifindex)
    {
        return TRUE;
    }
    return FALSE;
#else
    return FALSE;
#endif
}
