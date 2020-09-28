
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_nexthop.h"

extern int32
hsrv_openflow_nexthop_id_opf_alloc_by_sai(uint32* p_nexthop_id);

extern int32
hsrv_openflow_nexthop_id_opf_release_by_sai(uint32 nexthop_id);

extern int32
hsrv_openflow_mgroup_id_opf_alloc_by_sai(uint32* p_group_id);

extern int32
hsrv_openflow_mgroup_id_opf_free_by_sai(uint32 group_id);

extern uint32
ctc_sai_copp_get_nhid_for_controller(void);

 //static uint32 g_pst_ttp_openflow_controller_nhid = 0;


#define ____RESOURCE_FUNCTION____

int32
ctc_ttp_drv_nexthop_alloc_nhid(uint32 *nhid)
{
    return hsrv_openflow_nexthop_id_opf_alloc_by_sai(nhid);
}

int32
ctc_ttp_drv_nexthop_release_nhid(uint32 nhid)
{
    return hsrv_openflow_nexthop_id_opf_release_by_sai(nhid);
}

int32
ctc_ttp_drv_nexthop_alloc_mc_groupid(uint32 *mc_group_id)
{
    return hsrv_openflow_mgroup_id_opf_alloc_by_sai(mc_group_id);
}

int32
ctc_ttp_drv_nexthop_release_mc_groupid(uint32 mc_group_id)
{
    return hsrv_openflow_mgroup_id_opf_free_by_sai(mc_group_id);
}

#define ____MCAST_FUNCTION____

int32
ctc_ttp_drv_nexthop_add_mcast_nh(uint32 mc_group_id, uint32 mc_nhid, uint32 stats_id)
{
    ctc_mcast_nh_param_group_t nh_param;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;
    
    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    
    nh_param.mc_grp_id = mc_group_id;
    nh_param.opcode = CTC_NH_PARAM_MCAST_NONE;
    /*may be not all mcast group stats_en=1*/
    p_stats = hsrv_openflow_get_stats_info(stats_id);
    if (p_stats)
    {
        nh_param.stats_en = 1;
        nh_param.stats_id = p_stats->ctc_stats_id;
    }

    return ctc_nh_add_mcast(mc_nhid, &nh_param);
}

int32
ctc_ttp_drv_nexthop_del_mcast_nh(uint32 mc_nhid)
{
    return ctc_nh_remove_mcast(mc_nhid);
}

int32
ctc_ttp_drv_nexthop_add_mcast_member(uint32 mc_nhid, uint32 member_nhid)
{
    ctc_mcast_nh_param_group_t nh_param;
    
    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    
    //nh_param.mc_grp_id = mc_nhid;
    nh_param.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_param.mem_info.ref_nhid = member_nhid;
    //nh_param.mem_info.destid = req->memberInfo.gport;

    return ctc_nh_update_mcast(mc_nhid, &nh_param);
}

int32
ctc_ttp_drv_nexthop_del_mcast_member(uint32 mc_nhid, uint32 memeber_nhid)
{
    ctc_mcast_nh_param_group_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_mcast_nh_param_group_t));
    
    nh_param.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    nh_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_param.mem_info.ref_nhid = memeber_nhid;
    //nh_param.mem_info.destid = req->memberInfo.gport;

    return ctc_nh_update_mcast(mc_nhid, &nh_param);
}

#define ____ECMP_FUNCTION____

int32
ctc_ttp_drv_nexthop_add_ecmp_nh(uint32 ecmp_nhid, uint32 stats_id)
{
    ctc_nh_ecmp_nh_param_t nh_param;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    p_stats = hsrv_openflow_get_stats_info(stats_id);
    if (p_stats)
    {
        nh_param.stats_id = p_stats->ctc_stats_id;
    }
    nh_param.failover_en = TRUE;
    /*for first time add emcp gorup*/
    /* can not add an ecmp group with no members */
    /* workaround: add a drop member for ecmp group */
    nh_param.nhid[0] = CTC_NH_RESERVED_NHID_FOR_DROP;
    nh_param.nh_num = 1;
    
    return ctc_nh_add_ecmp(ecmp_nhid, &nh_param);
}

int32
ctc_ttp_drv_nexthop_del_ecmp_nh(uint32 ecmp_nhid)
{
    return ctc_nh_remove_ecmp(ecmp_nhid);
}

int32
ctc_ttp_drv_nexthop_add_ecmp_member(uint32 ecmp_nhid, uint32 *member_nhid, uint32 member_count)
{
    uint32 member_idx = 0;
    ctc_nh_ecmp_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    /* 1. delete drop nhid, which added when ecmp group create */
    nh_param.upd_type = CTC_NH_ECMP_REMOVE_MEMBER;
    nh_param.nhid[0] = CTC_NH_RESERVED_NHID_FOR_DROP;
    nh_param.nh_num = 1;
    ctc_nh_update_ecmp(ecmp_nhid, &nh_param);

    /* 2. add ecmp member */
    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    nh_param.upd_type = CTC_NH_ECMP_ADD_MEMBER;
    nh_param.nh_num = member_count;

    for (member_idx = 0; member_idx < member_count; member_idx++)
    {
        nh_param.nhid[member_idx] = member_nhid[member_idx];
    }
    
    return ctc_nh_update_ecmp(ecmp_nhid, &nh_param);
}

int32
ctc_ttp_drv_nexthop_del_ecmp_member(uint32 ecmp_nhid, uint32 *member_nhid, uint32 member_count)
{
    uint32 member_idx = 0;
    ctc_nh_ecmp_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));
    
    nh_param.upd_type = CTC_NH_ECMP_REMOVE_MEMBER;
    nh_param.nh_num = member_count;

    for (member_idx = 0; member_idx < member_count; member_idx++)
    {
        nh_param.nhid[member_idx] = member_nhid[member_idx];
    }
    
    return ctc_nh_update_ecmp(ecmp_nhid, &nh_param);
}


#define ____L3_UNICAST_FUNCTION____

int32
ctc_ttp_drv_nexthop_add_ipuc_nh(hsrv_openflow_ttp_group_bucket_entry_t *l3_bucket_entry, 
                                                hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, 
                                                uint32 nhid)
{
    ctc_ip_nh_param_t ip_nh_param;
    
    sal_memset(&ip_nh_param, 0, sizeof(ip_nh_param));
    
    ip_nh_param.oif.gport = l2_bucket_entry->bucket_data.l2_interface.output_port;
    if (l2_bucket_entry->bucket_data.l2_interface.vlan_id)
    {
        ip_nh_param.oif.vid   = l2_bucket_entry->bucket_data.l2_interface.vlan_id;
    }
    ip_nh_param.oif.is_l2_port = TRUE;
    sal_memcpy(ip_nh_param.mac, l3_bucket_entry->bucket_data.l3_unicast.dst_mac, sizeof(mac_addr_t));
    sal_memcpy(ip_nh_param.mac_sa, l3_bucket_entry->bucket_data.l3_unicast.src_mac, sizeof(mac_addr_t));

    return ctc_nh_add_ipuc(nhid, &ip_nh_param);
}

int32
ctc_ttp_drv_nexthop_del_ipuc_nh(uint32 nhid)
{
    return ctc_nh_remove_ipuc(nhid);
}


#define ____L2_INTERFACE_FUNCTION____

int32
ctc_ttp_drv_nexthop_add_vlan_edit_nh(hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, uint32 nhid, uint32 stats_id)
{
    ctc_vlan_edit_nh_param_t    vlan_param;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    sal_memset(&vlan_param, 0, sizeof vlan_param);
    
    vlan_param.gport_or_aps_bridge_id = l2_bucket_entry->bucket_data.l2_interface.output_port;
    if (l2_bucket_entry->bucket_data.l2_interface.pop_vlan_tag)
    {
        vlan_param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
    }
    else if (l2_bucket_entry->bucket_data.l2_interface.push_vlan_tag)
    {
        vlan_param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
        vlan_param.vlan_edit_info.output_svid = l2_bucket_entry->bucket_data.l2_interface.vlan_id;
        vlan_param.vlan_edit_info.edit_flag |= CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
        vlan_param.vlan_edit_info.edit_flag |= CTC_VLAN_EGRESS_EDIT_MAP_SVLAN_COS;
    }
    else if (l2_bucket_entry->bucket_data.l2_interface.vlan_id)
    {
        vlan_param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
        vlan_param.vlan_edit_info.output_svid = l2_bucket_entry->bucket_data.l2_interface.vlan_id;
        vlan_param.vlan_edit_info.edit_flag |= CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    }
    else
    {
        vlan_param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    }

    p_stats = hsrv_openflow_get_stats_info(stats_id);
    if (p_stats)
    {
        vlan_param.vlan_edit_info.stats_id = p_stats->ctc_stats_id;
    }
 
    return ctc_nh_add_xlate(nhid, &vlan_param);
}

int32
ctc_ttp_drv_nexthop_del_vlan_edit_nh(uint32 nhid)
{
    return ctc_nh_remove_xlate(nhid);
}

uint32
ctc_ttp_drv_nexthop_get_controller_nh(void)
{
    //return g_pst_ttp_openflow_controller_nhid;
    return ctc_sai_copp_get_nhid_for_controller();
}

int32
ctc_ttp_drv_nexthop_add_flex_cpu_nh(uint32 nhid)
{
    ctc_misc_nh_param_t nh_param;

    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));

    nh_param.type = CTC_MISC_NH_TYPE_TO_CPU;
    nh_param.misc_param.cpu_reason.cpu_reason_id = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1;
    return ctc_nh_add_misc(nhid, &nh_param);
}

#define ____TUNNEL_NEXTHOP_FUNCTION____

int32
ctc_ttp_drv_nexthop_add_vxlan_tunnel_bridge_nh(hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, uint32 nhid, uint32 stats_id)
{
    ctc_ip_tunnel_nh_param_t vxlan_bridge_nexthop_param;
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = NULL;
    hsrv_openflow_ttp_stats_info_t *p_stats = NULL;
    uint16 gport;

    HSRV_OPENFLOW_MEM_CHECK(l2_bucket_entry);

    sal_memset(&vxlan_bridge_nexthop_param, 0, sizeof(vxlan_bridge_nexthop_param));

    p_tunnel = hsrv_openflow_tunnel_get_tunnel_info(l2_bucket_entry->bucket_data.l2_interface.ifindex);
    if (!p_tunnel)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tunnel->tunnel_info.bond_port_ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(gport, vxlan_bridge_nexthop_param.mac_sa));
    sal_memcpy(vxlan_bridge_nexthop_param.mac, p_tunnel->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
    vxlan_bridge_nexthop_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
    //vxlan_bridge_nexthop_param.tunnel_info.flag = CTC_IP_NH_TUNNEL_FLAG_MAP_TTL;
    vxlan_bridge_nexthop_param.tunnel_info.ttl = 64;
    vxlan_bridge_nexthop_param.tunnel_info.dscp_select = CTC_NH_DSCP_SELECT_MAP;
    vxlan_bridge_nexthop_param.tunnel_info.ip_sa.ipv4 = p_tunnel->tunnel_info.local_ip;
    vxlan_bridge_nexthop_param.tunnel_info.ip_da.ipv4 = p_tunnel->tunnel_info.remote_ip;
    vxlan_bridge_nexthop_param.tunnel_info.inner_packet_type = PKT_TYPE_ETH;
    vxlan_bridge_nexthop_param.tunnel_info.vn_id = l2_bucket_entry->bucket_data.l2_interface.vni;

    //stats_id is hsrv layer stats_id
    p_stats = hsrv_openflow_get_stats_info(stats_id);
    if (p_stats)
    {
        vxlan_bridge_nexthop_param.tunnel_info.stats_id = p_stats->ctc_stats_id;
    }
    
    if (l2_bucket_entry->bucket_data.l2_interface.pop_vlan_tag)
    {
        vxlan_bridge_nexthop_param.tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN;
    }
    else if (l2_bucket_entry->bucket_data.l2_interface.vlan_id)
    {
        vxlan_bridge_nexthop_param.oif.vid |= l2_bucket_entry->bucket_data.l2_interface.vlan_id;
    }

    vxlan_bridge_nexthop_param.oif.gport = gport;
    vxlan_bridge_nexthop_param.oif.is_l2_port = 1;

    return ctc_nh_add_ip_tunnel(nhid, &vxlan_bridge_nexthop_param);
}

int32
ctc_ttp_drv_nexthop_del_vxlan_tunnel_bridge_nh(uint32 nhid)
{
    return ctc_nh_remove_ip_tunnel(nhid);
}

int32
ctc_ttp_drv_nexthop_add_vxlan_tunnel_route_nh(hsrv_openflow_ttp_group_bucket_entry_t *l3_bucket_entry, 
                                                hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, 
                                                uint32 nhid, 
                                                uint32 stats_id)
{
    ctc_ip_tunnel_nh_param_t vxlan_bridge_nexthop_param;
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = NULL;
    hsrv_openflow_ttp_stats_info_t *p_stats = NULL;
    uint16 gport;

    HSRV_OPENFLOW_MEM_CHECK(l2_bucket_entry);

    sal_memset(&vxlan_bridge_nexthop_param, 0, sizeof(vxlan_bridge_nexthop_param));

    p_tunnel = hsrv_openflow_tunnel_get_tunnel_info(l2_bucket_entry->bucket_data.l2_interface.ifindex);
    if (!p_tunnel)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tunnel->tunnel_info.bond_port_ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_get_port_mac_by_gport(gport, vxlan_bridge_nexthop_param.mac_sa));
    sal_memcpy(vxlan_bridge_nexthop_param.mac, p_tunnel->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
    vxlan_bridge_nexthop_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
    vxlan_bridge_nexthop_param.tunnel_info.flag = CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI | CTC_IP_NH_TUNNEL_FLAG_DONT_FRAG | CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA;
    vxlan_bridge_nexthop_param.tunnel_info.ttl = 64;
    vxlan_bridge_nexthop_param.tunnel_info.dscp_select = CTC_NH_DSCP_SELECT_MAP;
    vxlan_bridge_nexthop_param.tunnel_info.ip_sa.ipv4 = p_tunnel->tunnel_info.local_ip;
    vxlan_bridge_nexthop_param.tunnel_info.ip_da.ipv4 = p_tunnel->tunnel_info.remote_ip;
    vxlan_bridge_nexthop_param.tunnel_info.inner_packet_type = PKT_TYPE_ETH;
    vxlan_bridge_nexthop_param.tunnel_info.vn_id = l2_bucket_entry->bucket_data.l2_interface.vni;

    sal_memcpy(vxlan_bridge_nexthop_param.tunnel_info.inner_macda, l3_bucket_entry->bucket_data.l3_unicast.dst_mac, sizeof(mac_addr_t));
    sal_memcpy(vxlan_bridge_nexthop_param.tunnel_info.inner_macsa, l3_bucket_entry->bucket_data.l3_unicast.src_mac, sizeof(mac_addr_t));

    //stats_id is hsrv layer stats_id
    p_stats = hsrv_openflow_get_stats_info(stats_id);
    if (p_stats)
    {
        vxlan_bridge_nexthop_param.tunnel_info.stats_id = p_stats->ctc_stats_id;
    }
    
    if (l2_bucket_entry->bucket_data.l2_interface.pop_vlan_tag)
    {
        vxlan_bridge_nexthop_param.tunnel_info.flag |= CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN;
    }
    else if (l2_bucket_entry->bucket_data.l2_interface.vlan_id)
    {
        vxlan_bridge_nexthop_param.oif.vid |= l2_bucket_entry->bucket_data.l2_interface.vlan_id;
    }

    vxlan_bridge_nexthop_param.oif.gport = gport;
    vxlan_bridge_nexthop_param.oif.is_l2_port = 1;

    return ctc_nh_add_ip_tunnel(nhid, &vxlan_bridge_nexthop_param);
}

int32
ctc_ttp_drv_nexthop_del_vxlan_tunnel_route_nh(uint32 nhid)
{
    return ctc_nh_remove_ip_tunnel(nhid);
}

int32
ctc_ttp_drv_nexthop_overlay_tunnel_set_fid(uint32 vni, uint16 fid)
{
    return ctc_overlay_tunnel_set_fid(vni, fid);
}

int32
ctc_ttp_drv_nexthop_overlay_tunnel_del_fid(uint32 vni)
{
    return ctc_overlay_tunnel_set_fid(vni, 0);
}

#define ____INIT____

int32
ctc_ttp_drv_nexthop_init(void)
{
    //ctc_ttp_drv_nexthop_alloc_nhid(&g_pst_ttp_openflow_controller_nhid);
    //return ctc_ttp_drv_nexthop_add_flex_cpu_nh(g_pst_ttp_openflow_controller_nhid);
    return 0;
}


