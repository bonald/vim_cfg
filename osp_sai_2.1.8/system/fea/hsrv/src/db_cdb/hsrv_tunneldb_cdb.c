#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_group.h"
#include "hsrv_openflow_tunnel.h"
#include "hsrv_msg_openflow.h"
#include "glb_openflow_define.h"
#include "gen/tbl_arp_fib_define.h"
#include "gen/tbl_arp_fib.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "hsrv_tunneldb_cdb.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"

extern int32
_hsrv_openflow_tunnel_lookup_info(uint32 ifindex, hsrv_openflow_tunnel_info_t** pp_info);

extern int32
_hsrv_openflow_tunnel_get_fid(uint32 tun_id, uint16* p_label);

extern int32
hsrv_openflow_map_all_to_mcast_members(hsrv_openflow_action_combo_t* p_action, 
                                          hsrv_of_nh_info_t* p_member_nh_array,
                                          uint32* p_member_cnt);

extern int32
hsrv_tunneldb_cdb_modify_flow_action(hsrv_openflow_flow_t* p_flow);

extern int32
hsrv_openflow_flowdb_lookup_flow(uint32 flow_id, hsrv_openflow_flow_t** pp_flow);

extern int32 
fei_api_openflow_port_enable(fei_openflow_port_enable_t *pst_req);

extern hsrv_openflow_tunnel_master_t* g_pst_openflow_tunnel_master;


void _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_tunnel_nexthop_info)
{
    p_dynamic_tunnel_nexthop_info->tunnel_route_ip = 0;
    p_dynamic_tunnel_nexthop_info->mask_len = 0;
    p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = 0;
    p_dynamic_tunnel_nexthop_info->nexthop_id = 0;
    p_dynamic_tunnel_nexthop_info->tunnel_ifindex = 0;
}


 int32
_hsrv_openflow_dynamic_tunnel_lookup_info(hsrv_openflow_dynamic_tunnel_info_key_t* p_dynamic_tunnel_info_key, hsrv_openflow_dynamic_tunnel_info_t** pp_info)
{
    hsrv_openflow_dynamic_tunnel_info_t lkp_info;
    hsrv_openflow_dynamic_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_dynamic_tunnel_info_t));
    sal_memcpy(&lkp_info.dynamic_tunnel_info_key, p_dynamic_tunnel_info_key, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));

    p_tnl_info = ctclib_hash_lookup(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, &lkp_info);
    if (p_tnl_info)
    {
        *pp_info = p_tnl_info;
        p_tnl_info = NULL;
    }

    return HSRV_E_NONE;
}

int32
_hsrv_tunneldb_cdb_get_dynamic_tunnel_info(hsrv_openflow_dynamic_tunnel_info_key_t* p_dynamic_tunnel_info_key, hsrv_openflow_dynamic_tunnel_info_t** pp_dynamic_tunnel_info )
{
    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tunnel_info = NULL;
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_dynamic_tunnel_lookup_info(p_dynamic_tunnel_info_key, &p_dynamic_tunnel_info));
    if(NULL == p_dynamic_tunnel_info)
    {
        p_dynamic_tunnel_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_info_t));
        sal_memcpy(&p_dynamic_tunnel_info->dynamic_tunnel_info_key, p_dynamic_tunnel_info_key, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
        ctclib_list_init(&p_dynamic_tunnel_info->nexthop_list);
        ctclib_hash_get(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, p_dynamic_tunnel_info, ctclib_hash_alloc_intern);
    }
    *(pp_dynamic_tunnel_info) = p_dynamic_tunnel_info;
    return 0;
}


bool
hsrv_openflow_flow_action_include_dynamic_tunnel_port(hsrv_openflow_flow_t* p_flow)
{
    glb_openflow_flow_action_t* p_action;
    int32 count = 0;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    
    for (count = 0; count < p_flow->flow.actlist.action_count; count ++)
    {
        p_action = p_flow->flow.actlist.actlist + count;
        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == p_action->type)
        {
            if(GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
            {
                /* get tunnel info */
                _hsrv_openflow_tunnel_lookup_info(p_action->value.output.ifindex, &p_tnl_info);
                if(p_tnl_info->tunnel_info.is_dynamic)
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


int32
hsrv_openflow_map_out_action_include_dynamic_tunnel_port(hsrv_openflow_map_output_data_t* p_data, uint32* p_output_num, uint32* p_is_include)
{
    glb_openflow_flow_action_t* p_action;
    int32 count = 0;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;

    for (count = 0; count < p_data->p_actlist->action_count; count ++)
    {
        p_action = p_data->p_actlist->actlist + count;
        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == p_action->type)
        {
            (*p_output_num) ++;
            if(GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
            {
                /* get tunnel info */
                _hsrv_openflow_tunnel_lookup_info(p_action->value.output.ifindex, &p_tnl_info);
                if(p_tnl_info->tunnel_info.is_dynamic)
                {
                    (*p_is_include) ++;
                }
            }
        }
        
    }

    return 0;
}

int32
hsrv_tunneldb_cdb_get_nexthop_by_addr4(addr_ipv4_t *pst_dest, nexthop_key_t* p_nh_key, uint32* p_multicast_cnt, 
                                       uint32* p_mask_len, uint32* is_direct_connection, char* direct_connection_ifname)
{    
    tbl_iproute_node_t *p_db_rt_node = NULL;
    tbl_iproute_node_t rt_node;
    addr_ipv4_t dest_tmp;
    int32 index = 32;

    sal_memset(&rt_node, 0, sizeof(rt_node));
    rt_node.key.p.family = AF_INET;

    *is_direct_connection = 0;
    for(index = 32; index >= 0; index--)
    {
        sal_memcpy(&dest_tmp, pst_dest, sizeof(addr_ipv4_t));
        dest_tmp.s_addr = sal_htonl(dest_tmp.s_addr);
        apply_mask_ipv4_addr(&dest_tmp, index);
        
        rt_node.key.p.prefixlen = index;
        sal_memcpy(&rt_node.key.p.u.prefix4, &dest_tmp, sizeof(addr_ipv4_t));
        p_db_rt_node = tbl_iproute_node_get_iproute_node(&rt_node.key);
        if (p_db_rt_node)
        {
            break;
        }
    }

    if (NULL == p_db_rt_node)
    {
        *p_multicast_cnt = 0;
        return 0;
    }
    *p_mask_len = index;
    /*get nexthop*/
    if (p_db_rt_node->multipath)
    {
        *p_multicast_cnt = p_db_rt_node->nh_group.nh_num;
        sal_memcpy(p_nh_key, p_db_rt_node->nh_group.nh, (*p_multicast_cnt)*sizeof(nexthop_key_t));
    }
    else
    {
        *p_multicast_cnt = 1;
        sal_memcpy(p_nh_key, &p_db_rt_node->nh_key, sizeof(nexthop_key_t));
        if(NH_TYPE_LOCAL == p_db_rt_node->nh_key.type)
        {
            *is_direct_connection = 1;
            sal_strncpy(direct_connection_ifname, p_db_rt_node->nh_key.ifname, GLB_IFNAME_SZ);
        }
    }

    return 0;
}

int32
hsrv_tunneldb_cdb_set_interface_status_change(uint32 old_interface_status, glb_openflow_tunnel_info_t* p_tunnel_info)
{
    tbl_openflow_tunnel_interface_t* p_db_openflow_tunnel_if = NULL;
    tbl_interface_t* p_db_if = NULL;
    tbl_openflow_tunnel_interface_key_t tunnel_if_key;
    if(old_interface_status != p_tunnel_info->interface_status)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(p_tunnel_info->tunnel_ifindex);
        if(p_db_if)
        {
            sal_strncpy(tunnel_if_key.name, p_db_if->key.name, IFNAME_SIZE);
            p_db_openflow_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&tunnel_if_key);
            p_db_openflow_tunnel_if->tunnel_status = p_tunnel_info->interface_status;
            tbl_openflow_tunnel_interface_set_openflow_tunnel_interface_field(p_db_openflow_tunnel_if, TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS);
        }
    }

    return 0;
}

int32 
hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(uint32 remote_ip, glb_openflow_tunnel_info_t* p_tunnel_info)
{
    addr_ipv4_t addr_remote_ip;
    nexthop_key_t nh_key[IP_ECMP_MAX_PATH];
    uint32 multicast_cnt = 0;

    tbl_arp_fib_key_t arp_fib_key;
    tbl_arp_fib_t* p_arp_fib = NULL;

    uint32 index = 0;
    uint32 active_index = 0;

    tbl_interface_t* p_if = NULL; 
    uint32 mask_len = 0;
    uint32 old_interface_status = 0;
    uint32 is_direct_connection = 0;
    glb_if_type_t type = GLB_IF_TYPE_NULL;
    uint32 vlan_id = 0;
    tbl_fea_fdb_key_t  fea_fdb_key;
    tbl_fea_fdb_t* p_fea_fdb = NULL;
    addr_remote_ip.s_addr = remote_ip;
    //addr_remote_ip.s_addr = sal_ntoh32(addr_remote_ip.s_addr);  

    hsrv_tunneldb_cdb_get_nexthop_by_addr4(&addr_remote_ip, nh_key, &multicast_cnt, &mask_len, &is_direct_connection, p_tunnel_info->direct_connection_ifname);
    p_tunnel_info->multipath_cnt = multicast_cnt;
    old_interface_status = p_tunnel_info->interface_status;
    p_tunnel_info->interface_status = 0;
    p_tunnel_info->is_direct_connection = is_direct_connection;
    p_tunnel_info->fdb_status = GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE;
    if(0 == multicast_cnt)
    {
        p_tunnel_info->route_is_ready = 0;
        hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
        return 0;
    }
    else if(1 == multicast_cnt)
    {
        p_tunnel_info->route_is_ready = 1;
        p_tunnel_info->nexthop_ip = nh_key[0].u.ipv4.s_addr;
        p_tunnel_info->route_mask_len = mask_len;
        
        sal_memset(&arp_fib_key, 0x0, sizeof(tbl_arp_fib_key_t));
        if(is_direct_connection)
        {
            arp_fib_key.ip.s_addr = remote_ip;
            p_tunnel_info->nexthop_ip = remote_ip;
            arp_fib_key.ip.s_addr = sal_htonl(arp_fib_key.ip.s_addr);
            p_tunnel_info->nexthop_ip = sal_htonl(p_tunnel_info->nexthop_ip);
        }
        else
        {
            sal_memcpy(&arp_fib_key.ip, &nh_key[0].u.ipv4, sizeof(addr_ipv4_t));
        }
        p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib_key);
        if(NULL == p_arp_fib)
        {
            hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
            p_tunnel_info->arp_is_ready = 0;
            return 0;
        }
        p_if = tbl_interface_get_interface_by_name(p_arp_fib->ifname);
        if(NULL == p_if)
        {
            hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
            p_tunnel_info->arp_is_ready = 0;
            return 0;
        }
        type = tbl_interface_get_type_by_ifindex(p_if->ifindex);
        if(GLB_IF_TYPE_VLAN_IF == type)
        {
            sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
            vlan_id = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
            fea_fdb_key.fid = vlan_id;
            sal_memcpy(fea_fdb_key.mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
            sal_memcpy(p_tunnel_info->nexthop_mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
            p_tunnel_info->vlan_id = vlan_id;
            p_tunnel_info->fdb_status = GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_READY;
            
            p_fea_fdb = tbl_fea_fdb_get_fea_fdb(&fea_fdb_key);
            if(NULL == p_fea_fdb)
            {
                hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
                p_tunnel_info->arp_is_ready = 0;
                return 0;
            }
            p_tunnel_info->arp_is_ready = 1;
            p_tunnel_info->interface_status = 1;
            p_tunnel_info->fdb_status = GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_READY;
            tbl_interface_get_name_by_ifindex(p_fea_fdb->ifindex, p_tunnel_info->bind_port_name, GLB_IFNAME_SZ);
            p_tunnel_info->bond_port_ifindex = p_fea_fdb->ifindex;

        }
        else if((GLB_IF_TYPE_PORT_IF != type) && (GLB_IF_TYPE_LINKAGG_IF != type))
        {
            hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
            p_tunnel_info->arp_is_ready = 0;
            return 0;
        }
        else
        {
            p_tunnel_info->vlan_id = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
            p_tunnel_info->arp_is_ready = 1;
            p_tunnel_info->interface_status = 1;
            sal_strcpy(p_tunnel_info->bind_port_name, p_arp_fib->ifname);
            p_tunnel_info->bond_port_ifindex = tbl_interface_get_ifindex_by_name(p_tunnel_info->bind_port_name);
            sal_memcpy(p_tunnel_info->nexthop_mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
        }
    }
    else
    {
        p_tunnel_info->route_mask_len = mask_len;
        for(index = 0; index < multicast_cnt; index++)
        {
            p_tunnel_info->multipath[index].nexthop_ip = nh_key[index].u.ipv4.s_addr;
            
            sal_memset(&arp_fib_key, 0x0, sizeof(tbl_arp_fib_key_t));
            sal_memcpy(&arp_fib_key.ip, &nh_key[index].u.ipv4, sizeof(addr_ipv4_t));
            p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib_key);
            if(NULL == p_arp_fib)
            {
                p_tunnel_info->multipath[index].arp_is_ready = 0;
                continue;
            }
            p_if = tbl_interface_get_interface_by_name(p_arp_fib->ifname);
            if(NULL == p_if)
            {
                continue;
            }
            type = tbl_interface_get_type_by_ifindex(p_if->ifindex);
            if(GLB_IF_TYPE_VLAN_IF == type)
            {
                sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
                vlan_id = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
                fea_fdb_key.fid = vlan_id;
                sal_memcpy(fea_fdb_key.mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
                sal_memcpy(p_tunnel_info->multipath[index].nexthop_mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
                p_tunnel_info->multipath[index].vlan_id = vlan_id;
                p_tunnel_info->multipath[index].fdb_status = GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_READY;
                p_fea_fdb = tbl_fea_fdb_get_fea_fdb(&fea_fdb_key);
                if(NULL == p_fea_fdb)
                {
                    p_tunnel_info->multipath[index].arp_is_ready = 0;
                    continue;
                }
                p_tunnel_info->multipath[index].arp_is_ready = 1;
                p_tunnel_info->interface_status = 1;
                p_tunnel_info->multipath[index].fdb_status = GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_READY;
                tbl_interface_get_name_by_ifindex(p_fea_fdb->ifindex, p_tunnel_info->multipath[index].bind_port_name, GLB_IFNAME_SZ);
                p_tunnel_info->multipath[index].bond_port_ifindex = p_fea_fdb->ifindex;
                active_index ++;
            }
            else if ((GLB_IF_TYPE_PORT_IF != type) && (GLB_IF_TYPE_LINKAGG_IF != type))
            {
                p_tunnel_info->multipath[index].arp_is_ready = 0;
                continue;
            }
            else
            {
                p_tunnel_info->multipath[index].vlan_id = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
                p_tunnel_info->multipath[index].arp_is_ready = 1;
                p_tunnel_info->interface_status = 1;
                sal_strcpy(p_tunnel_info->multipath[index].bind_port_name, p_arp_fib->ifname);
                p_tunnel_info->multipath[index].bond_port_ifindex = tbl_interface_get_ifindex_by_name(p_tunnel_info->multipath[index].bind_port_name);
                sal_memcpy(p_tunnel_info->multipath[index].nexthop_mac, p_arp_fib->mac_addr, sizeof(mac_addr_t));
                active_index ++;
            }

        }
    }    
    hsrv_tunneldb_cdb_set_interface_status_change(old_interface_status, p_tunnel_info);
    return 0;
}

#if 0
int32 
hsrv_tunneldb_cdb_del_tunnel_info(uint32 remote_ip, glb_openflow_tunnel_info_t* p_tunnel_info)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_info = NULL;
    hsrv_openflow_dynamic_tunnel_info_key_t dynamic_tunnel_info_key;
    uint32 index = 0;

    if(1 == p_tunnel_info->multipath_cnt)
    {
        if(p_tunnel_info->route_is_ready)
        {
            sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
            dynamic_tunnel_info_key.tunnel_route_ip = p_tunnel_info->remote_ip;
            dynamic_tunnel_info_key.mask_len = p_tunnel_info->route_mask_len;
            dynamic_tunnel_info_key.tunnel_nexthop_ip = p_tunnel_info->nexthop_ip;
            _hsrv_openflow_dynamic_tunnel_lookup_info(&dynamic_tunnel_info_key, &p_info);
            if(p_info)
            {
                ctclib_hash_release(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, p_info);
                HSRV_OPENFLOW_TUNNEL_FREE(p_info);
            }
        }
    }
    else if(p_tunnel_info->multipath_cnt > 1)
    {
        for(index = 0; index < p_tunnel_info->multipath_cnt; index ++)
        {
            sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
            dynamic_tunnel_info_key.tunnel_route_ip = p_tunnel_info->remote_ip;
            dynamic_tunnel_info_key.mask_len = p_tunnel_info->route_mask_len;
            dynamic_tunnel_info_key.tunnel_nexthop_ip = p_tunnel_info->multipath[index].nexthop_ip;
            _hsrv_openflow_dynamic_tunnel_lookup_info(&dynamic_tunnel_info_key, &p_info);
            if(p_info)
            {
                ctclib_hash_release(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, p_info);
                HSRV_OPENFLOW_TUNNEL_FREE(p_info);
            }
        }
    }

    return 0;
}
#endif

int32
hsrv_tunneldb_cdb_openflow_preprocess_combo_action(hsrv_openflow_flow_t *p_flow, hsrv_openflow_action_combo_t* p_action_combo)
{
    if (p_flow->flow.key.mac.flags.ipv4_packet)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV4_);
    }
    else if(GLB_OF_FLOW_RULE_TYPE_IPV6 == p_flow->flow.key.rule_type)   /* added by hansf for support ipv6 action */
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, IPV6_);
    }
    else if (p_flow->flow.key.mac.flags.arp_packet)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, ARP_);
    }
    else if (IP_PROTO_UDP == p_flow->flow.key.rule.ipv4_rule.flags.l4_protocol)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, UDP_);
    }
    if (IP_PROTO_TCP == p_flow->flow.key.rule.ipv4_rule.flags.l4_protocol)
    {
        GLB_OFPACT_FLD_SET(p_action_combo->flag, TCP_);
    }
    return 0;
}


int32
hsrv_openflow_tunneldb_cdb_alloc_tunnel_nh(hsrv_ofnh_tunnel_param_t* p_param, hsrv_of_nh_info_t* p_mem_info, uint32 info_index, uint32 is_ecmp_member)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_tunnel_info_t temp_tnl_info;
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
    sal_memcpy(&temp_tnl_info, p_tnl_info, sizeof(hsrv_openflow_tunnel_info_t));
    if(0 != is_ecmp_member)
    {
        temp_tnl_info.tunnel_info.vlan_id = temp_tnl_info.tunnel_info.multipath[info_index].vlan_id;
        temp_tnl_info.tunnel_info.bond_port_ifindex = temp_tnl_info.tunnel_info.multipath[info_index].bond_port_ifindex;
        sal_strcpy(temp_tnl_info.tunnel_info.bind_port_name, temp_tnl_info.tunnel_info.multipath[info_index].bind_port_name);
        sal_memcpy(temp_tnl_info.tunnel_info.nexthop_mac, temp_tnl_info.tunnel_info.multipath[info_index].nexthop_mac, sizeof(mac_addr_t));
    }
    
    /* get bind port gport */
    bind_ifindex = temp_tnl_info.tunnel_info.bond_port_ifindex;
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
    if (GLB_MAX_VLAN_ID == temp_tnl_info.tunnel_info.vlan_id)
    {
        p_param->tnl.vlan_id = GLB_MIN_VLAN_ID;
    }
    else
    {
        p_param->tnl.vlan_id = temp_tnl_info.tunnel_info.vlan_id;
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
    
    p_param->tnl.tunnel_type = temp_tnl_info.tunnel_info.type;
    p_param->tnl.inner.tunnel_key = p_param->p_action_combo->tunnel_id;
    p_param->tnl.inner.protocol_type = HSRV_OPENFLOW_TUNNEL_PROTOCOL_TYPE_TRANSPARENT_BRIDGING;
    
    p_param->tnl.ip_sa.ipv4 = temp_tnl_info.tunnel_info.local_ip;
    p_param->tnl.ip_da.ipv4 = temp_tnl_info.tunnel_info.remote_ip;
    sal_memcpy(p_param->tnl.mac_da, temp_tnl_info.tunnel_info.nexthop_mac, sizeof(mac_addr_t));
    if (p_param->tnl.inner.tunnel_key)
    {
        if (temp_tnl_info.tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_NVGRE ||
            temp_tnl_info.tunnel_info.type == GLB_OPENFLOW_TUNNEL_TYPE_VXLAN)
        {
            HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_get_fid(p_param->tnl.inner.tunnel_key, &fid));
            p_param->tnl.fid = fid;
        }
    }

    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_ip_tunnel_nh(&p_param->tnl, p_mem_info));

    p_tnl_info = NULL;
  
    return HSRV_E_NONE;
}

int32 
hsrv_tunneldb_cdb_oper_function_flow_add_single_dynamic_tunnel(hsrv_openflow_flow_t *flow)
{
    glb_openflow_flow_action_t* p_action = NULL;
    int32 count = 0;
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_tunnel_param_t param;
    hsrv_openflow_nexthop_info_t *p_nh_info = &flow->u.dft.var.nh_info;
    hsrv_ofnh_mcast_param_t ecmp_param;
    uint32 ecmp_member_index = 0;

    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tunnel_info = NULL;
    hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_tunnel_nexthop_info = NULL;

    hsrv_openflow_dynamic_tunnel_info_key_t dynamic_tunnel_info_key;

    sal_memset(&action_combo, 0, sizeof(action_combo));
    HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_openflow_preprocess_combo_action(flow, &action_combo));

    for (count = 0; count < flow->flow.actlist.action_count; count ++)
    {
        p_action = flow->flow.actlist.actlist + count;
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_combo_action(p_action, &action_combo));
        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == p_action->type)
        {
            if(GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
            {
                /* get tunnel info */
                HSRV_IF_ERROR_RETURN(_hsrv_openflow_tunnel_lookup_info(p_action->value.output.ifindex, &p_tnl_info));
                if(p_tnl_info->tunnel_info.is_dynamic)
                {
                    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
                    {
                        p_nh_info->main_nh.nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
                        p_nh_info->main_nh.nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
                        p_nh_info->use_mcast = FALSE;
                        p_nh_info->use_ecmp = FALSE;
                        p_nh_info->use_dynamic_tunnel = TRUE;
                        
                        sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
                        dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
                        _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
                        p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
                        _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
                        p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
                        p_dynamic_tunnel_nexthop_info->mask_len = 0;
                        p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = 0;
                        p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->main_nh.nh_offset.nhid;
                        p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_action->value.output.ifindex;
                        ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
                    }
                    else if (1 == p_tnl_info->tunnel_info.multipath_cnt)
                    {
                        if(p_tnl_info->tunnel_info.arp_is_ready)
                        {
                            sal_memset(&param, 0, sizeof(hsrv_ofnh_tunnel_param_t));
                            param.p_action_combo = &action_combo;
                            param.tnl.tunnel_ifindex = p_action->value.output.ifindex;
                            hsrv_openflow_tunneldb_cdb_alloc_tunnel_nh(&param, &p_nh_info->main_nh, 0, 0);
                        }
                        else
                        {
                            p_nh_info->main_nh.nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
                            p_nh_info->main_nh.nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
                        }
                        p_nh_info->use_mcast = FALSE;
                        p_nh_info->use_ecmp = FALSE;
                        p_nh_info->use_dynamic_tunnel = TRUE;
                        sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
                        dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
                        _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
                        p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
                        _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
                        p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
                        p_dynamic_tunnel_nexthop_info->mask_len = p_tnl_info->tunnel_info.route_mask_len;
                        p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = p_tnl_info->tunnel_info.nexthop_ip;
                        p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->main_nh.nh_offset.nhid;
                        p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_action->value.output.ifindex;
                        ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
                    }
                    else
                    {
                        /*alloc ecmp group*/
                        sal_memset(&param, 0x0, sizeof(hsrv_ofnh_mcast_param_t));
                        ecmp_param.stats_en = 0;
                        ecmp_param.no_fwd = FALSE;
                        p_nh_info->main_nh.nh_offset.stats_id = 0;
                        hsrv_openflow_nexthop_alloc_ecmp_group(&ecmp_param, &p_nh_info->main_nh.nh_offset);
                        p_nh_info->use_ecmp = TRUE;
                        p_nh_info->use_dynamic_tunnel = TRUE;
                        p_nh_info->use_mcast = FALSE;

                        for(ecmp_member_index = 0; ecmp_member_index < p_tnl_info->tunnel_info.multipath_cnt; ecmp_member_index ++)
                        {
                            if(p_tnl_info->tunnel_info.multipath[ecmp_member_index].arp_is_ready)
                            {
                                sal_memset(&param, 0, sizeof(hsrv_ofnh_tunnel_param_t));
                                param.p_action_combo = &action_combo;
                                param.tnl.tunnel_ifindex = p_action->value.output.ifindex;
                                HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_cdb_alloc_tunnel_nh(&param, &(p_nh_info->member_nh[ecmp_member_index]), ecmp_member_index, 1));
                                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_ecmp_member(p_nh_info->main_nh.nh_offset.nhid, 0, 1, &(p_nh_info->member_nh[ecmp_member_index].nh_offset.nhid), 0));
                            }
                            else
                            {
                                p_nh_info->member_nh[ecmp_member_index].nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
                                p_nh_info->member_nh[ecmp_member_index].nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
                            }
                            /*save_db*/
                            sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
                            dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
                            _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
                            p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
                            _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
                            p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
                            p_dynamic_tunnel_nexthop_info->mask_len = p_tnl_info->tunnel_info.route_mask_len;
                            p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = p_tnl_info->tunnel_info.multipath[ecmp_member_index].nexthop_ip;
                            p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->member_nh[ecmp_member_index].nh_offset.nhid;
                            p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_action->value.output.ifindex;
                            ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
                        }
                        p_nh_info->member_count = p_tnl_info->tunnel_info.multipath_cnt;
                    }
                }
            }
        }
    }

    return 0;
}

int32
hsrv_tunneldb_cdb_flow_add_multi_tunnel_add(hsrv_ofnh_tunnel_param_t* p_tunnel_param, hsrv_of_nh_info_t* p_nh_info, hsrv_openflow_flow_t *flow)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = NULL;
    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tunnel_info = NULL;
    hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_tunnel_nexthop_info = NULL;

    hsrv_openflow_dynamic_tunnel_info_key_t dynamic_tunnel_info_key;
    uint32 index = 0;
    bool ecmp_member_active = FALSE;
    
    _hsrv_openflow_tunnel_lookup_info(p_tunnel_param->tnl.tunnel_ifindex , &p_tnl_info);
    if(p_tnl_info->tunnel_info.is_dynamic)
    {
        if(p_tnl_info->tunnel_info.multipath_cnt > 0)
        {
            if(p_tnl_info->tunnel_info.multipath_cnt == 1)
            {
                if(p_tnl_info->tunnel_info.arp_is_ready)
                {
                    hsrv_openflow_tunneldb_cdb_alloc_tunnel_nh(p_tunnel_param, p_nh_info, 0, 0);
                }
                else
                {
                    p_nh_info->nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
                    p_nh_info->nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
                }
                sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
                dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
                _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
                p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
                _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
                p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
                p_dynamic_tunnel_nexthop_info->mask_len = p_tnl_info->tunnel_info.route_mask_len;
                p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = p_tnl_info->tunnel_info.nexthop_ip;
                p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->nh_offset.nhid;
                p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_tunnel_param->tnl.tunnel_ifindex;
                ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
            }
            else
            {
                for(index = 0; index < p_tnl_info->tunnel_info.multipath_cnt; index ++)
                {
                    if(p_tnl_info->tunnel_info.multipath[index].arp_is_ready)
                    {
                        hsrv_openflow_tunneldb_cdb_alloc_tunnel_nh(p_tunnel_param, p_nh_info, index, 1);
                        ecmp_member_active = TRUE;
                        break;
                    }
                }
                if(!ecmp_member_active)
                {
                    p_nh_info->nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
                    p_nh_info->nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
                }
                sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
                dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
                _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
                p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
                _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
                p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
                p_dynamic_tunnel_nexthop_info->mask_len = p_tnl_info->tunnel_info.route_mask_len;
                p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = p_tnl_info->tunnel_info.nexthop_ip;
                p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->nh_offset.nhid;
                p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_tunnel_param->tnl.tunnel_ifindex;
                ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
            }
            
        }
        else
        {
            p_nh_info->nh_offset.nh_type = HSRV_OF_NH_TYPE_MAX;
            p_nh_info->nh_offset.nhid = HSRV_OF_NH_RESV_NHID_FOR_DROP;
    
            sal_memset(&dynamic_tunnel_info_key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
            dynamic_tunnel_info_key.flow_id = flow->flow.flow_id;
            _hsrv_tunneldb_cdb_get_dynamic_tunnel_info(&dynamic_tunnel_info_key, &p_dynamic_tunnel_info);
            p_dynamic_tunnel_nexthop_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_dynamic_tunnel_nexthop_list_t));
            _hsrv_openflow_dynamic_tunnel_nexthop_info_list_init(p_dynamic_tunnel_nexthop_info);
            p_dynamic_tunnel_nexthop_info->tunnel_route_ip = p_tnl_info->tunnel_info.remote_ip;
            p_dynamic_tunnel_nexthop_info->mask_len = 0;
            p_dynamic_tunnel_nexthop_info->tunnel_nexthop_ip = 0;
            p_dynamic_tunnel_nexthop_info->nexthop_id = p_nh_info->nh_offset.nhid;
            p_dynamic_tunnel_nexthop_info->tunnel_ifindex = p_tunnel_param->tnl.tunnel_ifindex;
            ctclib_list_insert_tail(&p_dynamic_tunnel_info->nexthop_list, &p_dynamic_tunnel_nexthop_info->node);
        }
    }
    else
    {
        hsrv_openflow_tunnel_alloc_tunnel_nh(p_tunnel_param, p_nh_info);
    }
    return 0;
}

int32 
hsrv_tunneldb_cdb_oper_function_flow_add_multi_dynamic_tunnel(hsrv_openflow_flow_t *flow)
{
    glb_openflow_flow_action_t* p_action = NULL;
    int32 count = 0;
    hsrv_openflow_action_combo_t action_combo;
    hsrv_ofnh_tunnel_param_t tunnel_param;
    hsrv_ofnh_flex_param_t flex_param;
    hsrv_ofnh_mcast_param_t mcast_param;
    uint32 mcast_member_index = 0;
    //bool output_to_controller = FALSE;
    //bool output_to_local = FALSE;
    bool is_in_port = FALSE;
    bool port_check_discard = FALSE;
    uint32 ifindex = 0;
    uint32 ofport = 0;
    bool is_truncate = FALSE;
    hsrv_openflow_nexthop_info_t *p_nh_info = &flow->u.dft.var.nh_info;

    sal_memset(&action_combo, 0, sizeof(action_combo));
    hsrv_tunneldb_cdb_openflow_preprocess_combo_action(flow, &action_combo);

        /*alloc mcast group*/
    sal_memset(&mcast_param, 0x0, sizeof(hsrv_ofnh_mcast_param_t));
    mcast_param.stats_en = 0;
    mcast_param.no_fwd = FALSE;
    p_nh_info->main_nh.nh_offset.stats_id = 0;
    hsrv_openflow_nexthop_alloc_mcast_group(&mcast_param, &p_nh_info->main_nh.nh_offset);
    p_nh_info->use_mcast = TRUE;
    for (count = 0; count < flow->flow.actlist.action_count; count ++)
    {
        p_action = flow->flow.actlist.actlist + count;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_map_combo_action(p_action, &action_combo));

        if (GLB_OPENFLOW_ACTION_TYPE_SET_TRUNCATE == p_action->type)
        {
            is_truncate = TRUE;
        }
        
        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT != p_action->type)
        {
            continue;
        }

        if (is_truncate)
        {
            is_truncate = FALSE;
            continue;
        }
        
        ifindex = p_action->value.output.ifindex;
        ofport = p_action->value.output.ofport;
        is_in_port = FALSE;
        
        if (GLB_OF_PORT_TYPE_PHYSICAL == p_action->value.output.type)
        {    
            flex_param.p_action_combo = &action_combo;
            if (p_action->value.output.tid)
            {
                action_combo.output_linkagg_gport = CTC_MAP_TID_TO_GPORT(p_action->value.output.tid);
            }
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&flex_param, &(p_nh_info->member_nh[mcast_member_index])));
            if(!check_feature_support(CONFIG_FEATURE_HYBRID_FORCE_BACK))
            {
                if(flow->flow.key.in_port.ofport== ofport)
                {
                    is_in_port = TRUE;
                }
            }
        }
        else if (GLB_OF_PORT_TYPE_TUNNEL == p_action->value.output.type)
        {
            sal_memset(&tunnel_param, 0, sizeof(hsrv_ofnh_tunnel_param_t));
            tunnel_param.p_action_combo = &action_combo;
            tunnel_param.tnl.tunnel_ifindex = ifindex;
            HSRV_IF_ERROR_RETURN(hsrv_tunneldb_cdb_flow_add_multi_tunnel_add(&tunnel_param, &(p_nh_info->member_nh[mcast_member_index]), flow));
            if(p_nh_info->member_nh[mcast_member_index].nh_offset.nhid == HSRV_OF_NH_RESV_NHID_FOR_DROP)
            {
                continue;
            }
        }
        else if (GLB_OF_PORT_TYPE_BOND == p_action->value.output.type)
        {
            flex_param.p_action_combo = &action_combo;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_get_gport_by_ifindex(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&flex_param, &(p_nh_info->member_nh[mcast_member_index])));
        }
        else if (GLB_OF_PORT_TYPE_AGG == p_action->value.output.type)
        {
            flex_param.p_action_combo = &action_combo;
            HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_get_gport_by_ifindex(ifindex, &action_combo.output_gport));
            HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&flex_param, &(p_nh_info->member_nh[mcast_member_index])));
        }
        else if (GLB_OFPP_CONTROLLER == ofport)
        {
            //output_to_controller = TRUE;
            continue;
        }
        else if (GLB_OFPP_LOCAL == ofport)
        {
            //output_to_local = TRUE;
            continue;
        }
        else if(GLB_OFPP_IN_PORT == ofport)
        {
            if(check_feature_support(CONFIG_FEATURE_HYBRID_FORCE_BACK))
            {
                is_in_port = TRUE;
                
                flex_param.p_action_combo = &action_combo;
                /* Use OF spec IN_PORT to differentiate nexthop, this gport will not collide
                   with any physical/aggregation/cpu port because its bigger value.
                   In hagt we will translate it to cpu port to make SDK happy, however the
                   cpu port is actually not used.
                   . */
                action_combo.output_gport = GLB_OFPP_IN_PORT;
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&flex_param, &(p_nh_info->member_nh[mcast_member_index])));
            }
            else
            {
                is_in_port = TRUE;
                
                flex_param.p_action_combo = &action_combo;
                hsrv_ifdb_get_portid(flow->flow.key.in_port.ifindex, &action_combo.output_gport);
                HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_flex_nh(&flex_param, &(p_nh_info->member_nh[mcast_member_index])));
            }
        }
        else if(GLB_OFPP_ALL == ofport)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_map_all_to_mcast_members(
                &action_combo,
                p_nh_info->member_nh,
                &mcast_member_index));
            continue;
        }
        /* Added by weizj for tunnel bug 33724
         * For output to physcial port, if user set drop_pkt_to_ingress_port
         * to true, we should enable the check in the ASIC.
         * Modifed by weizj for bug 35135 */
        if (NULL == flow)
        {
            if (hsrv_openflow_group_get_drop_ingress())
            {
                port_check_discard = TRUE;
            }
        }
        else
        {
            if (hsrv_openflow_flowdb_get_drop_ingress())
            {
                port_check_discard = TRUE;
            }
        }

        p_nh_info->member_nh[mcast_member_index].nh_offset.port_check_discard = port_check_discard;
        p_nh_info->member_nh[mcast_member_index].nh_offset.redirect_to_ingress_port = is_in_port;
        p_nh_info->member_nh[mcast_member_index].gport = action_combo.output_gport;
        p_nh_info->member_nh[mcast_member_index].tid = p_action->value.output.tid;
        p_nh_info->member_nh[mcast_member_index].ifindex = ifindex;

        hsrv_ofnh_mcast_member_param_t member_param;
        hsrv_of_nh_info_t* p_member = NULL;

        sal_memset(&member_param, 0x0, sizeof(hsrv_ofnh_mcast_member_param_t));
        p_member = &p_nh_info->member_nh[mcast_member_index];
        member_param.group_nhid = p_nh_info->main_nh.nh_offset.nhid;
        member_param.op_type = HSRV_OFNH_MCAST_OP_ADD_MEMBER;
        member_param.mem_gport = p_member->gport;
        member_param.mem_nhid = p_member->nh_offset.nhid;
        member_param.port_check_discard = p_member->nh_offset.port_check_discard;
        member_param.redirect_to_ingress_port = p_member->nh_offset.redirect_to_ingress_port;
        
        HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_add_mcast_member(&member_param));
        mcast_member_index ++;
        p_nh_info->member_count ++;

    }

    return 0;
}

int32
hsrv_tunneldb_cdb_oper_function_flow_unmap_action(hsrv_openflow_flow_t *p_flow_info)
{
   hsrv_openflow_dynamic_tunnel_info_key_t key;
   hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tunnel_info = NULL;
   ctclib_list_node_t* p_node = NULL;
   ctclib_list_node_t *p_next = NULL;
   hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_tunnel_nexthop_info = NULL; 
    
    sal_memset(&key, 0x0, sizeof(hsrv_openflow_dynamic_tunnel_info_key_t));
    key.flow_id = p_flow_info->flow.flow_id;
    _hsrv_openflow_dynamic_tunnel_lookup_info(&key, &p_dynamic_tunnel_info);
    if(p_dynamic_tunnel_info)
    {
        ctclib_list_for_each_safe(p_node, p_next, &p_dynamic_tunnel_info->nexthop_list)
        {
            p_dynamic_tunnel_nexthop_info = ctclib_container_of(p_node, hsrv_openflow_dynamic_tunnel_nexthop_list_t, node);
            if(p_dynamic_tunnel_nexthop_info)
            {
                ctclib_list_delete(&p_dynamic_tunnel_info->nexthop_list, p_node);
                HSRV_OPENFLOW_TUNNEL_FREE(p_dynamic_tunnel_nexthop_info);
            }
        }
    }
    else
    {
        return 0;
    }
    if(ctclib_list_empty(&p_dynamic_tunnel_info->nexthop_list))
    {
         ctclib_hash_release(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, p_dynamic_tunnel_info);
         HSRV_OPENFLOW_TUNNEL_FREE(p_dynamic_tunnel_info);
    }

    return 0;
}



static void
hsrv_tunneldb_cdb_arp_oper_flow_modify_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tnl = (hsrv_openflow_dynamic_tunnel_info_t*)phb->data;
    tbl_arp_fib_t* p_data = user_data;
    ctclib_list_node_t* p_node = NULL;
    ctclib_list_node_t* p_next = NULL;
    hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_nexthop = NULL;

    hsrv_openflow_flow_t* p_flow = NULL;
    uint32 need_update = 0;

    ctclib_list_for_each_safe(p_node, p_next, &(p_dynamic_tnl->nexthop_list))
    {
        p_dynamic_nexthop = ctclib_container_of(p_node, hsrv_openflow_dynamic_tunnel_nexthop_list_t, node);
        if(p_dynamic_nexthop->tunnel_nexthop_ip == p_data->key.ip.s_addr)
        {
            need_update = 1;
            break;
        }
    }
    if(need_update)
    {
        hsrv_openflow_flowdb_lookup_flow(p_dynamic_tnl->dynamic_tunnel_info_key.flow_id, &p_flow);
        if(p_flow)
        {
            hsrv_tunneldb_cdb_modify_flow_action(p_flow);
        }
    }
}

static void
hsrv_tunneldb_cdb_arp_oper_add_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_arp_fib_t* p_arp_fib = user_data;
    uint32 index = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }

    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        return;
    }
    else if(1 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        if(p_tnl_info->tunnel_info.nexthop_ip == p_arp_fib->key.ip.s_addr)
        {
            hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
        }
    }
    else
    {
        for(index = 0; index < p_tnl_info->tunnel_info.multipath_cnt; index ++)
        {
            if(p_tnl_info->tunnel_info.multipath[index].nexthop_ip == p_arp_fib->key.ip.s_addr)
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
                break;
            }
        }
    }
}

static void
hsrv_tunneldb_cdb_arp_oper_del_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_arp_fib_t* p_arp_fib = user_data;
    uint32 index = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }

    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        return;
    }
    else if(1 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        if(p_tnl_info->tunnel_info.nexthop_ip == p_arp_fib->key.ip.s_addr)
        {
            hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
        }
    }
    else
    {
        for(index = 0; index < p_tnl_info->tunnel_info.multipath_cnt; index ++)
        {
            if(p_tnl_info->tunnel_info.multipath[index].nexthop_ip == p_arp_fib->key.ip.s_addr)
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
        }
    }
}


int32
hsrv_tunneldb_cdb_arp_oper_add(tbl_arp_fib_t *p_arp_fib)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_arp_oper_add_tunnel_port_iterate, p_arp_fib);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_arp_oper_flow_modify_iterate, p_arp_fib);

    return 0;
}

int32
hsrv_tunneldb_cdb_arp_oper_del(tbl_arp_fib_t *p_arp_fib)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_arp_oper_del_tunnel_port_iterate, p_arp_fib);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_arp_oper_flow_modify_iterate, p_arp_fib);

    return 0;
}


int32
hsrv_tunneldb_cdb_arp_oper_function(hsrv_tunnel_cdb_arp_oper_t op, tbl_arp_fib_t *p_arp_fib)
{
    if(HSRV_TUNNEL_CDB_ARP_ADD == op)
    {
        hsrv_tunneldb_cdb_arp_oper_add(p_arp_fib);
    }
    else
    {
        hsrv_tunneldb_cdb_arp_oper_del(p_arp_fib);
    }
    return 0;
}

int32
hsrv_tunneldb_cdb_arp_oper_add_function(tbl_arp_fib_t * p_arp_fib)
{
    hsrv_tunneldb_cdb_arp_oper_function(HSRV_TUNNEL_CDB_ARP_ADD, p_arp_fib);
    return 0;
}

int32
hsrv_tunneldb_cdb_arp_oper_del_function(tbl_arp_fib_t * p_arp_fib)
{
    hsrv_tunneldb_cdb_arp_oper_function(HSRV_TUNNEL_CDB_ARP_DEL, p_arp_fib);
    return 0;
}

int32
hsrv_tunneldb_cdb_get_route_mask_by_remote_ip(uint32 remote_ip, tbl_iproute_node_t* p_rt_node, uint32* is_match)
{   
    addr_ipv4_t tunnel_dest_tmp;
    addr_ipv4_t route_dest_tmp;


    sal_memset(&tunnel_dest_tmp, 0x0, sizeof(addr_ipv4_t));
    tunnel_dest_tmp.s_addr = remote_ip;
    tunnel_dest_tmp.s_addr = sal_htonl(tunnel_dest_tmp.s_addr);
    apply_mask_ipv4_addr(&tunnel_dest_tmp, p_rt_node->key.p.prefixlen);

    sal_memset(&route_dest_tmp, 0x0, sizeof(addr_ipv4_t));
    route_dest_tmp.s_addr = p_rt_node->key.p.u.prefix4.s_addr;
    apply_mask_ipv4_addr(&route_dest_tmp, p_rt_node->key.p.prefixlen);
        
    if(0 == sal_memcmp(&tunnel_dest_tmp, &route_dest_tmp, sizeof(addr_ipv4_t)))
    {
        *is_match = 1;
        return 0;
    }

    *is_match = 0;
    return 0;
}


static void
hsrv_tunneldb_cdb_route_oper_add_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_iproute_node_t* p_rt_node = user_data;
    uint32 is_match = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }

    hsrv_tunneldb_cdb_get_route_mask_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, p_rt_node, &is_match);
    if(is_match)
    {
        if(p_tnl_info->tunnel_info.route_is_ready)
        {
            if(p_tnl_info->tunnel_info.route_mask_len <= p_rt_node->key.p.prefixlen)
            {
               hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
        }
        else
        {
            hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
        }
    }
}

static void
hsrv_tunneldb_cdb_route_oper_del_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_iproute_node_t* p_rt_node = user_data;
    uint32 is_match = 0;
    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }
    hsrv_tunneldb_cdb_get_route_mask_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, p_rt_node, &is_match);
    if(is_match)
    {
        if(p_tnl_info->tunnel_info.route_is_ready)
        {
            if(p_tnl_info->tunnel_info.route_mask_len == p_rt_node->key.p.prefixlen)
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
        }
    }
}

static void
hsrv_tunneldb_cdb_route_oper_flow_modify_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tnl = (hsrv_openflow_dynamic_tunnel_info_t*)phb->data;
    tbl_iproute_node_t* p_rt_node = user_data;
    ctclib_list_node_t* p_node = NULL;
    ctclib_list_node_t *p_next = NULL;
    hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_nexthop = NULL;
    uint32 is_match = 0;
    uint32 need_update = 0;

    hsrv_openflow_flow_t* p_flow = NULL;

    ctclib_list_for_each_safe(p_node, p_next, &(p_dynamic_tnl->nexthop_list))
    {
        p_dynamic_nexthop = ctclib_container_of(p_node, hsrv_openflow_dynamic_tunnel_nexthop_list_t, node);
        hsrv_tunneldb_cdb_get_route_mask_by_remote_ip(p_dynamic_nexthop->tunnel_route_ip, p_rt_node, &is_match);
        if(is_match&&(p_dynamic_nexthop->mask_len <= p_rt_node->key.p.prefixlen))
        {
            need_update = 1;
            break;
        }
    }

    if(need_update)
    {
        hsrv_openflow_flowdb_lookup_flow(p_dynamic_tnl->dynamic_tunnel_info_key.flow_id, &p_flow);
        if(p_flow)
        {
            hsrv_tunneldb_cdb_modify_flow_action(p_flow);
        }
    }
    
}


int32
hsrv_tunneldb_cdb_route_oper_add(tbl_iproute_node_t* p_rt_node)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_route_oper_add_tunnel_port_iterate, p_rt_node);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_route_oper_flow_modify_iterate, p_rt_node);

    return 0;
}

int32
hsrv_tunneldb_cdb_route_oper_del(tbl_iproute_node_t* p_rt_node)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_route_oper_del_tunnel_port_iterate, p_rt_node);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_route_oper_flow_modify_iterate, p_rt_node);

    return 0;
}

int32
hsrv_tunneldb_cdb_route_oper_function(hsrv_tunnel_cdb_route_oper_t op, tbl_iproute_node_t* p_rt_node)
{      
    if(HSRV_TUNNEL_CDB_ROUTE_ADD == op)
    {
        hsrv_tunneldb_cdb_route_oper_add(p_rt_node);
    }
    else if(HSRV_TUNNEL_CDB_ROUTE_DEL == op)
    {
        hsrv_tunneldb_cdb_route_oper_del(p_rt_node);
    }

    return 0;
}

int32
hsrv_tunneldb_cdb_route_oper_add_function(tbl_iproute_node_t* p_rt_node)
{
    hsrv_tunneldb_cdb_route_oper_function(HSRV_TUNNEL_CDB_ROUTE_ADD, p_rt_node);
    return 0;
}

int32
hsrv_tunneldb_cdb_route_oper_del_function(tbl_iproute_node_t* p_rt_node)
{
    hsrv_tunneldb_cdb_route_oper_function(HSRV_TUNNEL_CDB_ROUTE_DEL, p_rt_node);
    return 0;
}


static void
hsrv_tunneldb_cdb_fdb_oper_add_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_fea_fdb_t* p_fdb = user_data;
    tbl_fea_fdb_key_t    fea_fdb_key;
    uint32 member_index = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }
    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        return;
    }
    else if(1 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_READY == p_tnl_info->tunnel_info.fdb_status)
        {
            sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
            fea_fdb_key.fid = p_tnl_info->tunnel_info.vlan_id;
            sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
            if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
            return;
        }
    }
    else
    {
        for(member_index = 0; member_index < p_tnl_info->tunnel_info.multipath_cnt; member_index++)
        {
            if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_READY == p_tnl_info->tunnel_info.multipath[member_index].fdb_status)
            {
                sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
                fea_fdb_key.fid = p_tnl_info->tunnel_info.multipath[member_index].vlan_id;
                sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.multipath[member_index].nexthop_mac, sizeof(mac_addr_t));
                if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
                {
                    hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
                }
                continue;
            }
        }
    }
    return;
}

static void
hsrv_tunneldb_cdb_fdb_oper_del_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_fea_fdb_t* p_fdb = user_data;
    tbl_fea_fdb_key_t    fea_fdb_key;
    uint32 member_index = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }
    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        return;
    }
    else if(1 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE != p_tnl_info->tunnel_info.fdb_status)
        {
            sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
            fea_fdb_key.fid = p_tnl_info->tunnel_info.vlan_id;
            sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
            if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
            return;
        }
    }
    else
    {
        for(member_index = 0; member_index < p_tnl_info->tunnel_info.multipath_cnt; member_index++)
        {
            if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE != p_tnl_info->tunnel_info.multipath[member_index].fdb_status)
            {
                sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
                fea_fdb_key.fid = p_tnl_info->tunnel_info.multipath[member_index].vlan_id;
                sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.multipath[member_index].nexthop_mac, sizeof(mac_addr_t));
                if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
                {
                    hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
                }
                continue;
            }
        }
    }
    return;
}

static void
hsrv_tunneldb_cdb_fdb_oper_update_tunnel_port_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_tunnel_info_t* p_tnl_info = (hsrv_openflow_tunnel_info_t*)phb->data;
    tbl_fea_fdb_t* p_fdb = user_data;
    tbl_fea_fdb_key_t    fea_fdb_key;
    uint32 member_index = 0;

    if(!p_tnl_info->tunnel_info.is_dynamic)
    {
        return;
    }
    if(0 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        return;
    }
    else if(1 == p_tnl_info->tunnel_info.multipath_cnt)
    {
        if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE != p_tnl_info->tunnel_info.fdb_status)
        {
            sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
            fea_fdb_key.fid = p_tnl_info->tunnel_info.vlan_id;
            sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.nexthop_mac, sizeof(mac_addr_t));
            if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
            {
                hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
            }
            return;
        }
    }
    else
    {
        for(member_index = 0; member_index < p_tnl_info->tunnel_info.multipath_cnt; member_index++)
        {
            if(GLB_OPENFLOW_TUNNEL_INFO_FDB_STATUS_NOT_CARE != p_tnl_info->tunnel_info.multipath[member_index].fdb_status)
            {
                sal_memset(&fea_fdb_key, 0x0, sizeof(tbl_fea_fdb_key_t));
                fea_fdb_key.fid = p_tnl_info->tunnel_info.multipath[member_index].vlan_id;
                sal_memcpy(fea_fdb_key.mac, p_tnl_info->tunnel_info.multipath[member_index].nexthop_mac, sizeof(mac_addr_t));
                if(0 == sal_memcmp(&fea_fdb_key, &(p_fdb->key), sizeof(tbl_fea_fdb_key_t)))
                {
                    hsrv_tunneldb_cdb_get_tunnel_info_by_remote_ip(p_tnl_info->tunnel_info.remote_ip, &p_tnl_info->tunnel_info);
                }
                continue;
            }
        }
    }
    return;
}


static void
hsrv_tunneldb_cdb_fdb_oper_flow_modify_iterate(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_dynamic_tunnel_info_t* p_dynamic_tnl = (hsrv_openflow_dynamic_tunnel_info_t*)phb->data;
    tbl_fea_fdb_t* p_fdb = user_data;
    ctclib_list_node_t* p_node = NULL;
    ctclib_list_node_t* p_next = NULL;
    hsrv_openflow_dynamic_tunnel_nexthop_list_t* p_dynamic_nexthop = NULL;

    hsrv_openflow_flow_t* p_flow = NULL;

    tbl_arp_fib_key_t arp_fib_key;
    tbl_arp_fib_t* p_arp_fib = NULL;

    tbl_interface_t* p_if = NULL;
    glb_if_type_t type = GLB_IF_TYPE_NULL;
    uint32 vlan_id = 0;
    uint32 need_update = 0;


    ctclib_list_for_each_safe(p_node, p_next, &(p_dynamic_tnl->nexthop_list))
    {
        p_dynamic_nexthop = ctclib_container_of(p_node, hsrv_openflow_dynamic_tunnel_nexthop_list_t, node);
        sal_memset(&arp_fib_key, 0x0, sizeof(tbl_arp_fib_key_t));
        arp_fib_key.ip.s_addr = p_dynamic_nexthop->tunnel_nexthop_ip;
        p_arp_fib = tbl_arp_fib_get_arp_fib(&arp_fib_key);
        if(NULL == p_arp_fib)
        {
            continue;
        }
        p_if = tbl_interface_get_interface_by_name(p_arp_fib->ifname);
        if(NULL == p_if)
        {
            continue;
        }
        type = tbl_interface_get_type_by_ifindex(p_if->ifindex);
        if(GLB_IF_TYPE_VLAN_IF == type)
        {
            vlan_id = tbl_interface_get_vid_by_ifindex(p_if->ifindex);
            if((0 == sal_memcmp(p_arp_fib->mac_addr, p_fdb->key.mac, sizeof(mac_addr_t))) &&
                (vlan_id == p_fdb->key.fid))
            {
                need_update = 1;
                break;
            }
        }

    }
    if(need_update)
    {
        hsrv_openflow_flowdb_lookup_flow(p_dynamic_tnl->dynamic_tunnel_info_key.flow_id, &p_flow);
        if(p_flow)
        {
            hsrv_tunneldb_cdb_modify_flow_action(p_flow);
        }
    }
    
    return;
}

int32
hsrv_tunneldb_cdb_fdb_oper_add(tbl_fea_fdb_t *p_fdb)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_add_tunnel_port_iterate, p_fdb);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_flow_modify_iterate, p_fdb);

    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_del(tbl_fea_fdb_t *p_fdb)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_del_tunnel_port_iterate, p_fdb);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_flow_modify_iterate, p_fdb);

    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_update(tbl_fea_fdb_t *p_fdb)
{
    ctclib_hash_iterate(HSRV_OPENFLOW_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_update_tunnel_port_iterate, p_fdb);
    ctclib_hash_iterate(HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH, hsrv_tunneldb_cdb_fdb_oper_flow_modify_iterate, p_fdb);

    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_function(hsrv_tunnel_cdb_fdb_oper_t op , tbl_fea_fdb_t *p_fdb)
{
    if(HSRV_TUNNEL_CDB_FDB_ADD == op)
    {
        hsrv_tunneldb_cdb_fdb_oper_add(p_fdb);
    }
    else if(HSRV_TUNNEL_CDB_FDB_DEL == op)
    {
        hsrv_tunneldb_cdb_fdb_oper_del(p_fdb);
    }
    else if(HSRV_TUNNEL_CDB_FDB_UPDATE == op)
    {
        hsrv_tunneldb_cdb_fdb_oper_update(p_fdb);
    }

    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_add_function(tbl_fea_fdb_t *p_fdb)
{
    hsrv_tunneldb_cdb_fdb_oper_function(HSRV_TUNNEL_CDB_FDB_ADD, p_fdb);
    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_del_function(tbl_fea_fdb_t *p_fdb)
{
    hsrv_tunneldb_cdb_fdb_oper_function(HSRV_TUNNEL_CDB_FDB_DEL, p_fdb);
    return 0;
}

int32
hsrv_tunneldb_cdb_fdb_oper_update_function(tbl_fea_fdb_t *p_fdb)
{
    hsrv_tunneldb_cdb_fdb_oper_function(HSRV_TUNNEL_CDB_FDB_UPDATE, p_fdb);
    return 0;
}

int32 
hsrv_tunneldb_cdb_oper_function(hsrv_tunnel_cdb_oper_t op, hsrv_openflow_flow_t *flow)
{
    if(HSRV_TUNNEL_CDB_FLOW_ADD_SINGLE_DYNAMIC_TUNNEL == op)
    {
        hsrv_tunneldb_cdb_oper_function_flow_add_single_dynamic_tunnel(flow);
    }
    else if(HSRV_TUNNEL_CDB_FLOW_ADD_MULTI_DYNAMIC_TUNNEL == op)
    {
        hsrv_tunneldb_cdb_oper_function_flow_add_multi_dynamic_tunnel(flow);
    }
    else if(HSRV_TUNNEL_CDB_FLOW_DEL_UNMAP_ACTION == op)
    {
        hsrv_tunneldb_cdb_oper_function_flow_unmap_action(flow);
    }

    return 0;
}

int32
hsrv_interface_set_openflow_en(tbl_interface_t* p_if)
{
    fei_openflow_port_enable_t req;
    
    if((p_if->hw_type == GLB_IF_TYPE_PORT_IF) || (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF))
    {
        sal_memset(&req, 0x0, sizeof(fei_openflow_port_enable_t));
        req.enable = p_if->openflow_en;
        req.ifindex = p_if->ifindex;
        req.link_up = tbl_interface_is_running(p_if);
        fei_api_openflow_port_enable(&req);
    }

    return 0;
}

int32
hsrv_interface_set_openflow_operation_agg(tbl_interface_t* p_if)
{
    fei_openflow_port_operation_agg_t req;
    tbl_openflow_interface_key_t openflow_if_key;
    tbl_openflow_interface_t* p_openflow_if = NULL;

    if((p_if->hw_type == GLB_IF_TYPE_PORT_IF) || (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF))
    {
        sal_strncpy(openflow_if_key.name, p_if->key.name, IFNAME_SIZE);
        p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if_key);
        if(p_openflow_if)
        {
            sal_memset(&req, 0x0, sizeof(fei_openflow_port_operation_agg_t));
            req.enable = p_if->openflow_operation_agg;
            req.ifindex = p_if->openflow_operation_agg_mem_ifindex;
            req.vlan_filter = p_openflow_if->obey_vlan_filter;
            req.ingress_add_native_vlan = p_openflow_if->ingress_add_native_vlan_enable;
            req.tunnel_mpls_disable = p_openflow_if->tunnel_mpls_disable;
            if(GLB_IF_TYPE_NVGRE_IF == p_openflow_if->bind_tunnel_type)
            {
                req.type = GLB_OPENFLOW_TUNNEL_TYPE_NVGRE;
            }
            else if(GLB_IF_TYPE_VXLAN_IF == p_openflow_if->bind_tunnel_type)
            {
                req.type = GLB_OPENFLOW_TUNNEL_TYPE_VXLAN;
            }
            else if(GLB_IF_TYPE_L2GRE_IF == p_openflow_if->bind_tunnel_type)
            {
                req.type = GLB_OPENFLOW_TUNNEL_TYPE_L2GRE;
            }
            fei_api_openflow_port_operation_agg(&req);
        }
    }

    return 0;
}


