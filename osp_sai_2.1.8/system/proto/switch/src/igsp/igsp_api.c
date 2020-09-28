
#include "proto.h"
#include "switch.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_acl_entry_define.h"
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "igsp_api.h"
#include "igmp_pdu.h"
#include "igsp_pdu.h"
#include "brg_fdb.h"
#include "if_agg.h"
#include "acl.h"

void
igsp_api_timer_querier_fn(void *p_data)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_t *p_db_igsp_if = (tbl_igsp_intf_t*)p_data;
    uint32 query_interval = 0;
    
    p_db_igsp_if->t_querier = NULL;

    if (!p_db_glb->host_join_enable)
    {
        igsp_pdu_tx_general_query(p_db_igsp_if, p_db_igsp_if->querier_max_response_time, 0);
    }

    if (p_db_igsp_if->startup_query_count)
    {
        p_db_igsp_if->startup_query_count--;
        query_interval = p_db_igsp_if->query_interval >> 2;
    }
    else
    {
        query_interval = p_db_igsp_if->query_interval;
    }

    IGSP_DEBUG(IGSP_TIMER, "Timer querier timeout on vlan %u query_interval %u", p_db_igsp_if->key.vid, query_interval);
    
    p_db_igsp_if->t_querier = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_querier_fn, 
            p_db_igsp_if, query_interval);

    return;
}

void
igsp_api_timer_other_querier_fn(void *p_data)
{
    tbl_igsp_intf_t *p_db_igsp_if = (tbl_igsp_intf_t*)p_data;

    IGSP_DEBUG(IGSP_TIMER, "Timer other querier timeout on vlan %u", p_db_igsp_if->key.vid);

    p_db_igsp_if->t_other_querier = NULL;

    p_db_igsp_if->other_querier_address.s_addr = 0;
    if (p_db_igsp_if->querier_enable)
    {
        if (!p_db_igsp_if->querier_operate)
        {
            igsp_api_intf_set_querier_operate(p_db_igsp_if, TRUE, 0);
        }
    }

    return;
}

void
igsp_api_timer_mrouter_expire_fn(void *p_data)
{
    ds_mrouter_port_t *p_db_mr_port = (ds_mrouter_port_t*)p_data;
    tbl_igsp_intf_t *p_db_igsp_if = (tbl_igsp_intf_t*)p_db_mr_port->igsp_if;
    char             ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IFNAME_ETH2FULL(p_db_mr_port->key.name, ifname_ext);
    IGSP_DEBUG(IGSP_TIMER, "Timer mrouter expire timeout on interface %s", ifname_ext);

    p_db_mr_port->expire_timer = NULL;

    igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);

    return;
}

int32
igsp_api_intf_update_querier(tbl_igsp_intf_t *p_db_igsp_if)
{
    uint32 interval = 0;
    
    if (p_db_igsp_if->enable && p_db_igsp_if->querier_enable)
    {
        p_db_igsp_if->startup_query_count = p_db_igsp_if->robustness_var - 1;
        CTC_TASK_STOP_TIMER(p_db_igsp_if->t_querier);
        p_db_igsp_if->t_querier = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_querier_fn, 
                p_db_igsp_if, interval);
    }
    else
    {
        CTC_TASK_STOP_TIMER(p_db_igsp_if->t_querier);
    }
    
    return PM_E_NONE;
}

static int32
_igsp_api_group_add_mcfdb(tbl_igsp_group_t *p_db_grp)
{
    char ip_str[CMD_BUF_16];
    char mac_str[CMD_BUF_16];
    tbl_mcfdb_t macfdb;
    uint32 ifindex = 0;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    sal_memset(&macfdb, 0, sizeof(macfdb));
    macfdb.key.fid = p_db_grp->key.vid;
    IGMP_IPV4_ADDR_TO_MAC(p_db_grp->key.ip.s_addr, macfdb.key.mac);
    ifindex = p_db_grp->key.ifindex;
    sal_memset(ifname, 0, IFNAME_SIZE);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);

    rc = brg_mcfdb_add_mcfdb(&macfdb, ifindex);

    IGSP_DEBUG(IGSP_EVENT, "IGMP Snooping add L2 mcast, IP: %s, MAC: %s, vid: %u, ifname: %s, rc: %d",
        cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_grp->key.ip),
        cdb_mac_addr_val2str(mac_str, CMD_BUF_16, macfdb.key.mac),
        p_db_grp->key.vid,
        IFNAME_ETH2FULL(ifname, ifname_ext),
        rc);

    return rc;
}

static int32
_igsp_api_group_del_mcfdb(tbl_igsp_group_t *p_db_grp)
{
    char ip_str[CMD_BUF_16];
    char mac_str[CMD_BUF_16];
    tbl_mcfdb_t macfdb;
    uint32 ifindex = 0;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 deleted = FALSE;
    int32 rc = PM_E_NONE;
    
    sal_memset(&macfdb, 0, sizeof(macfdb));
    macfdb.key.fid = p_db_grp->key.vid;
    IGMP_IPV4_ADDR_TO_MAC(p_db_grp->key.ip.s_addr, macfdb.key.mac);
    ifindex = p_db_grp->key.ifindex;
    sal_memset(ifname, 0, IFNAME_SIZE);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);

    rc = brg_mcfdb_del_mcfdb(&macfdb, ifindex, &deleted);

    IGSP_DEBUG(IGSP_EVENT, "IGMP Snooping delete L2 mcast, IP: %s, MAC: %s, vid: %u, ifname: %s, rc: %d",
        cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_grp->key.ip),
        cdb_mac_addr_val2str(mac_str, CMD_BUF_16, macfdb.key.mac),
        p_db_grp->key.vid,
        IFNAME_ETH2FULL(ifname, ifname_ext), 
        rc);

    return rc;
}

static uint32
_igsp_api_group_get_active(tbl_igsp_intf_t *p_db_igsp_if)
{
    if (p_db_igsp_if->enable)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static int32
_igsp_api_group_set_active(tbl_igsp_group_t *p_db_grp)
{
    int32 rc = PM_E_NONE;
    p_db_grp->inactive = FALSE;
    tbl_igsp_group_set_igsp_group_field(p_db_grp, TBL_IGSP_GROUP_FLD_INACTIVE);
    
    rc = _igsp_api_group_add_mcfdb(p_db_grp);
    return rc;
}

static int32
_igsp_api_group_set_inactive(tbl_igsp_group_t *p_db_grp)
{
    int32 rc = PM_E_NONE;
    p_db_grp->inactive = TRUE;
    tbl_igsp_group_set_igsp_group_field(p_db_grp, TBL_IGSP_GROUP_FLD_INACTIVE);
    
    rc = _igsp_api_group_del_mcfdb(p_db_grp);
    return rc;
}

static int32
_igsp_api_group_add(tbl_igsp_group_t *p_igsp_grp, tbl_igsp_intf_t *p_db_igsp_if)
{
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_group_t *p_db_grp = NULL;
    int32 rc = PM_E_NONE;
    uint32 active = FALSE;
    char ip_str[CMD_BUF_16];
    char ifname[IFNAME_SIZE] = {0};
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    tbl_interface_get_name_by_ifindex(p_igsp_grp->key.ifindex, ifname, IFNAME_SIZE);
    
    if (p_glb->curr_group_member >= p_glb->max_member_number)
    {
        IGSP_DEBUG(IGSP_EVENT, "group %s vlan %u ifname %s add fail, Total current group number reached limitation %u",
                      cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_igsp_grp->key.ip), p_igsp_grp->key.vid, 
                      IFNAME_ETH2FULL(ifname, ifname_ext), p_glb->max_member_number);
        return PM_E_EXCEED_MAX_NUM;
    }

    if (p_db_igsp_if->curr_group_member >= p_db_igsp_if->max_member_number)
    {
        IGSP_DEBUG(IGSP_EVENT, "group %s ifname %s add fail, VLAN %u current group number reached limitation %u",
                      cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_igsp_grp->key.ip), 
                      IFNAME_ETH2FULL(ifname, ifname_ext), p_igsp_grp->key.vid, p_db_igsp_if->max_member_number);
        return PM_E_EXCEED_MAX_NUM;
    }
    
    p_igsp_grp->uptime = p_igsp_grp->update_time;
    rc = tbl_igsp_group_add_igsp_group(p_igsp_grp);
    if (rc < 0)
    {
        return rc;
    }

    p_db_grp = tbl_igsp_group_get_igsp_group(&p_igsp_grp->key);
    if (NULL == p_db_grp)
    {
        return PM_E_NO_MEMORY;
    }
    
    p_db_igsp_if->curr_group_member++;
    p_glb->curr_group_member++;
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER);

    active = _igsp_api_group_get_active(p_db_igsp_if);
    if (active)
    {
        _igsp_api_group_set_active(p_db_grp);
    }
    else
    {
        p_db_grp->inactive = TRUE;
        tbl_igsp_group_set_igsp_group_field(p_db_grp, TBL_IGSP_GROUP_FLD_INACTIVE);
    }

    return PM_E_NONE;
}

static int32
_igsp_api_group_del(tbl_igsp_group_t *p_db_grp)
{
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;

    key.vid = p_db_grp->key.vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (p_db_igsp_if)
    {
        p_db_igsp_if->curr_group_member--;
    }
    p_glb->curr_group_member--;
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_CURR_GROUP_MEMBER);

    if (!p_db_grp->inactive)
    {
        _igsp_api_group_set_inactive(p_db_grp);
    }
    CTC_TASK_STOP_TIMER(p_db_grp->t_retx_query);
    rc = tbl_igsp_group_del_igsp_group(&p_db_grp->key);

    return rc;
}

static int32
_igsp_api_group_del_common(tbl_igsp_group_t *p_db_igsp_grp)
{
    uint32 group_count = 0;
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_igsp_grp)
    {
        return PM_E_NONE;
    }
    group_count = igsp_api_get_group_count(p_db_igsp_grp);
    if (1 == group_count)
    {
        /* if is last group and is not learnt from v3, need send leave report */
        if (FALSE == p_db_igsp_grp->is_v3_leave)
        {
            rc = igsp_pdu_tx_group_record_leave(p_db_igsp_grp);
        }
    }
    rc = _igsp_api_group_del(p_db_igsp_grp);

    return rc;
}

static int32
_igsp_api_group_is_v1_host_exist(tbl_igsp_group_t *p_db_igsp_grp)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;

    if (p_db_igsp_grp->is_v1_host_exist)
    {
        return TRUE;
    }
    
    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if (p_db_group->key.vid != p_db_igsp_grp->key.vid || sal_memcmp(&p_db_group->key.ip, &p_db_igsp_grp->key.ip, sizeof(p_db_group->key.ip)))
            {
                continue;
            }

            if (p_db_group->is_v1_host_exist)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


int32
_igsp_api_intf_set_default(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    
    p_igsp_if->fast_leave = p_db_glb->fast_leave;
    p_igsp_if->report_suppress = p_db_glb->report_suppress;
    p_igsp_if->version = p_db_glb->version;
    p_igsp_if->max_member_number = IGMP_LIMIT_DEF;
    p_igsp_if->query_interval = p_db_glb->query_interval;
    p_igsp_if->robustness_var = p_db_glb->robustness_var;
    p_igsp_if->querier_max_response_time = p_db_glb->querier_max_response_time;
    p_igsp_if->lmqc = p_db_glb->lmqc;
    p_igsp_if->lmqi = p_db_glb->lmqi;
    p_igsp_if->group_member_interval = p_igsp_if->robustness_var * p_igsp_if->query_interval + 
        p_igsp_if->querier_max_response_time;
    p_igsp_if->admin_other_query_interval = 0;
    p_igsp_if->mrouter_aging_interval = IGMP_MROUTER_PORT_TIMEOUT_DEF;
    p_igsp_if->querier_oper_address.s_addr = sal_hton32(INADDR_QUERY_ADDR);
        
    return PM_E_NONE;
}

static int32
_igsp_api_clear_groups_by_port_vlan(tbl_interface_t *p_db_if, vid_t vid)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if ((p_db_group->key.ifindex == p_db_if->ifindex) && (p_db_group->key.vid == vid))
            {
                _igsp_api_group_del_common(p_db_group);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_api_clear_groups_by_port(tbl_interface_t *p_db_if, uint32 all)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if (p_db_group->key.ifindex == p_db_if->ifindex)
            {
                if (all)
                {
                    _igsp_api_group_del_common(p_db_group);
                }
                else
                {
                    igsp_api_group_del_dynamic(p_db_group);
                }
            }
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_api_clear_mrouter_by_port(tbl_interface_t *p_db_if)
{
    vid_t vid = GLB_VLAN_NONE;
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t    *p_db_igsp_if = NULL;
    ds_mrouter_port_t  *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode1 = NULL;

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        if (p_db_igsp_if->mrouter_list.obj_list)
        {
            CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode, listnode1)
            {
                if (NULL == p_db_mrouter_port)
                {
                    continue;
                }

                if (!sal_strcmp(p_db_if->key.name, p_db_mrouter_port->key.name) && IGMP_MROUTE_DYNAMIC == p_db_mrouter_port->type)
                {
                    igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mrouter_port);
                }
            }
        }
    }

    return PM_E_NONE;
}

static int32
_igsp_api_intf_clear_dynamic_mrouter(tbl_igsp_intf_t *p_db_igsp_if)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->mrouter_list.obj_list, p_db_mr_port, node, nnext)
    {
        if (IGMP_MROUTE_DYNAMIC == p_db_mr_port->type)
        {
            igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);
        }
    }
    
    return PM_E_NONE;
}

int32
igsp_api_intf_add_query(ds_query_t *p_query, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address, uint32 max_resp_time)
{
    ds_query_t *p_db_query = NULL;

    p_query->key.group.s_addr = group_address;
    p_db_query = ds_query_get_query(p_db_igsp_if, p_query);
    if (p_db_query)
    {
        p_db_query->max_resp_time = max_resp_time;
        p_db_query->vid = p_db_igsp_if->key.vid;
    }
    else
    {
        p_query->max_resp_time = max_resp_time;
        p_query->vid = p_db_igsp_if->key.vid;
        ds_query_add_query(p_db_igsp_if, p_query);
        p_db_query = ds_query_get_query(p_db_igsp_if, p_query);
        if (NULL == p_db_query)
        {
            return PM_E_NO_MEMORY;
        }
    }

    CTC_TASK_STOP_TIMER(p_db_query->response_timer);
    p_db_query->response_timer = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_max_resp_time_fn, p_db_query, p_db_query->max_resp_time);
    
    return PM_E_NONE;
}

void
igsp_api_max_resp_time_fn(void *p_data)
{
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    ds_query_t *p_db_query = (ds_query_t*)p_data;
    
    p_db_query->response_timer = NULL;

    key.vid = p_db_query->vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return;
    }

    igsp_api_intf_del_query_port(p_db_igsp_if, p_db_query);
    
    return;
}

int32
igsp_api_clear_groups_by_vlan(addr_ipv4_t *group_addr, vid_t vid)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;
    uint32 match = FALSE;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            match = FALSE;
            if (group_addr)
            {
                if (group_addr->s_addr == p_db_group->key.ip.s_addr)
                {
                    if ((GLB_VLAN_NONE == vid) || (p_db_group->key.vid == vid))
                    {
                        match = TRUE;
                    }
                }
            }
            else
            {
                if ((GLB_VLAN_NONE == vid) || (p_db_group->key.vid == vid))
                {
                    match = TRUE;
                }
            }
            
            if (match)
            {
                igsp_api_group_del_dynamic(p_db_group);
            }
        }
    }

    return PM_E_NONE;
}

int32
igsp_api_clear_all_groups_by_vlan(vid_t vid)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;
    uint32 match = FALSE;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            match = FALSE;
            if ((GLB_VLAN_NONE == vid) || (p_db_group->key.vid == vid))
            {
                match = TRUE;
            }
            
            if (match)
            {
                _igsp_api_group_del_common(p_db_group);
            }
        }
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_do_add(vid_t vid)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_t igsp_if;
    int32 rc = PM_E_NONE;

    sal_memset(&igsp_if, 0, sizeof(igsp_if));
    igsp_if.key.vid = vid;
    _igsp_api_intf_set_default(&igsp_if);

    PM_E_RETURN(tbl_igsp_intf_add_igsp_intf(&igsp_if));

    if (p_db_glb->enable)
    {
        PM_E_RETURN(igsp_api_intf_enable(&igsp_if));
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_add(vid_t vid)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();

    /* vlan 1 maybe create before init done */
    if (!p_sys_glb->init_done)
    {
        return PM_E_NONE;
    }

    return igsp_api_intf_do_add(vid);
}

int32
igsp_api_intf_del(vid_t vid)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    ds_query_t *p_db_query = NULL;
    ds_join_group_t *p_db_join = NULL;
    int32 rc = PM_E_NONE;

    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }

    if (p_db_igsp_if->enable)
    {
        igsp_api_intf_disable(p_db_igsp_if);
    }
    
    CTC_TASK_STOP_TIMER(p_db_igsp_if->t_querier);
    CTC_TASK_STOP_TIMER(p_db_igsp_if->t_other_querier);

    igsp_api_clear_all_groups_by_vlan(vid);
    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->mrouter_list.obj_list, p_db_mr_port, node, nnext)
    {
        igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);
    }
    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->query_list.obj_list, p_db_query, node, nnext)
    {
        igsp_api_intf_del_query_port(p_db_igsp_if, p_db_query);
    }
    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->join_list.obj_list, p_db_join, node, nnext)
    {
        igsp_api_del_join_group(p_db_igsp_if, p_db_join);
    }

    PM_E_RETURN(tbl_igsp_intf_del_igsp_intf(&key));

    return PM_E_NONE;
}

int32
igsp_api_intf_update_group(tbl_igsp_intf_t *p_db_igsp_if)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    ctclib_hash_backet_t *hb = NULL;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;
    uint32 enable = p_db_igsp_if->enable;

    for (i = 0; i < p_master->group_hash->size; i++)
    {
        for (hb = p_master->group_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_igsp_grp = (tbl_igsp_group_t*)hb->data;
            if (p_db_igsp_grp->key.vid != p_db_igsp_if->key.vid)
            {
                continue;
            }
            if (enable)
            {
                if (IGMP_TYPE_STATIC == p_db_igsp_grp->type)
                {
                    if (p_db_igsp_grp->inactive)
                    {
                        _igsp_api_group_set_active(p_db_igsp_grp);
                    }
                }
            }
            else
            {
                if (IGMP_TYPE_DYNAMIC == p_db_igsp_grp->type)
                {
                    _igsp_api_group_del(p_db_igsp_grp);                    
                }
                else if (IGMP_TYPE_STATIC == p_db_igsp_grp->type)
                {
                    if (!p_db_igsp_grp->inactive)
                    {
                        _igsp_api_group_set_inactive(p_db_igsp_grp);
                    }
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_set_acl_name(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }

    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_ACL_NAME));

    return PM_E_NONE;
}

int32
igsp_api_intf_enable(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint8 old_enable = FALSE;
    int32 rc = PM_E_NONE;

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }

    old_enable = p_db_igsp_if->enable;
    p_igsp_if->enable = TRUE;    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_ENABLE));
    if (old_enable != p_igsp_if->enable)
    {
        PM_E_RETURN(igsp_api_intf_update_group(p_db_igsp_if));
    }
    PM_E_RETURN(igsp_api_update_other_query_interval_group_member_interval(p_db_igsp_if));
    PM_E_RETURN(igsp_api_intf_update_querier(p_db_igsp_if));

    if (p_db_igsp_if->discard_unknown_act != p_db_igsp_if->discard_unknown)
    {
        p_igsp_if->discard_unknown_act = p_db_igsp_if->discard_unknown;
        PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT));
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_disable(tbl_igsp_intf_t *p_igsp_if)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    ds_query_t *p_db_query = NULL;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    uint8 old_enable = FALSE;
    int32 rc = PM_E_NONE;

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    old_enable = p_db_igsp_if->enable;
    p_igsp_if->enable = FALSE;
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_ENABLE));
    if (old_enable != p_igsp_if->enable)
    {
        PM_E_RETURN(igsp_api_intf_update_group(p_db_igsp_if));
    }
    PM_E_RETURN(igsp_api_intf_update_querier(p_db_igsp_if));
    CTC_TASK_STOP_TIMER(p_db_igsp_if->t_querier);
    CTC_TASK_STOP_TIMER(p_db_igsp_if->t_other_querier);

    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->query_list.obj_list, p_db_query, node, nnext)
    {
        igsp_api_intf_del_query_port(p_db_igsp_if, p_db_query);
    }
    _igsp_api_intf_clear_dynamic_mrouter(p_db_igsp_if);

    if (IGMP_UNKNOWN_DISCARD == p_db_igsp_if->discard_unknown_act)
    {
        p_igsp_if->discard_unknown_act = IGMP_UNKNOWN_FLOOD;
        PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT));
    }
    
    return PM_E_NONE;
}

int32
igsp_api_intf_set_version(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_VERSION));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (p_db_igsp_if)
    {
        if (IGMP_VERSION_3 == p_db_igsp_if->version)
        {
            p_igsp_if->report_suppress = FALSE;
        }
        else
        {
            p_igsp_if->report_suppress = TRUE;
        }
        
        if (p_db_igsp_if->report_suppress != p_igsp_if->report_suppress)
        {
            igsp_api_intf_set_report_suppress(p_igsp_if);
        }
    }
    
    return PM_E_NONE;
}

int32
igsp_api_intf_set_fast_leave(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_FAST_LEAVE));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_lmqi(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_LMQI));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_lmqc(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_LMQC));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_max_member_number(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_robustness_var(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    igsp_api_update_other_query_interval_group_member_interval(p_db_igsp_if);
    
    return PM_E_NONE;
}

int32
igsp_api_intf_set_discard_unknown(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN));
    if (p_db_igsp_if->enable)
    {
        p_igsp_if->discard_unknown_act = p_igsp_if->discard_unknown;
        PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT));
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_set_report_suppress(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_REPORT_SUPPRESS));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_querier_operate(tbl_igsp_intf_t *p_igsp_if, uint32 operate, uint32 other_querier_addr)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }
    p_igsp_if->querier_operate = operate;
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_OPERATE));

    if (p_db_igsp_if->querier_enable)
    {
        if (!operate)
        {
            p_db_igsp_if->other_querier_address.s_addr = other_querier_addr;
            CTC_TASK_STOP_TIMER(p_db_igsp_if->t_other_querier);
            p_db_igsp_if->t_other_querier = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_other_querier_fn, 
                    p_db_igsp_if, p_db_igsp_if->other_query_interval);
        }
    }
    else
    {
        p_db_igsp_if->other_querier_address.s_addr = 0;
        CTC_TASK_STOP_TIMER(p_db_igsp_if->t_other_querier);
    }

    PM_E_RETURN(igsp_api_intf_update_querier(p_db_igsp_if));

    return PM_E_NONE;
}

int32
igsp_api_intf_set_querier_enable(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_ENABLE));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }

    /* force querier_operate to querier_enable */
    p_igsp_if->querier_operate = p_igsp_if->querier_enable;
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_OPERATE));
    PM_E_RETURN(igsp_api_intf_update_querier(p_db_igsp_if));
        
    return PM_E_NONE;
}

int32
igsp_api_intf_set_query_interval(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERY_INTERVAL));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    igsp_api_update_other_query_interval_group_member_interval(p_db_igsp_if);

    return PM_E_NONE;
}

int32
igsp_api_update_other_query_interval_group_member_interval(tbl_igsp_intf_t *p_db_igsp_if)
{
    if (0 == p_db_igsp_if->admin_other_query_interval)
    {
        p_db_igsp_if->other_query_interval  = p_db_igsp_if->robustness_var * p_db_igsp_if->query_interval + (p_db_igsp_if->querier_max_response_time/2);
    }
    
    p_db_igsp_if->group_member_interval = p_db_igsp_if->robustness_var * p_db_igsp_if->query_interval + (p_db_igsp_if->querier_max_response_time);
    
    return PM_E_NONE;
}

int32
igsp_api_intf_set_admin_other_query_interval(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (p_igsp_if->admin_other_query_interval)
    {
        p_igsp_if->other_query_interval = p_igsp_if->admin_other_query_interval;
        PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL));
    }
    else
    {
        igsp_api_update_other_query_interval_group_member_interval(p_db_igsp_if);
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_set_querier_max_response_time(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME));

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    igsp_api_update_other_query_interval_group_member_interval(p_db_igsp_if);

    return PM_E_NONE;
}

int32
igsp_api_intf_set_querier_config_addr(tbl_igsp_intf_t *p_igsp_if,tbl_igsp_intf_t *p_db_igsp_if)
{
    int interval = 0;
    if (NULL == p_igsp_if)
    {
        return PM_E_NONE;
    }
    p_igsp_if->startup_query_count=p_db_igsp_if->robustness_var;
    tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT);
    CTC_TASK_STOP_TIMER(p_db_igsp_if->t_querier);
    p_db_igsp_if->t_querier = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_querier_fn, 
                p_db_igsp_if, interval);
                
    if (!p_db_igsp_if->querier_operate)
    {
        igsp_api_intf_set_querier_operate(p_db_igsp_if, TRUE, 0);
    }
    
    tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS);
    igsp_api_intf_update_querier_oper_addr(p_igsp_if);
        
    return PM_E_NONE;
}

int32
igsp_api_intf_set_mrouter_aging_interval(tbl_igsp_intf_t *p_igsp_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL));

    return PM_E_NONE;
}

int32
igsp_api_intf_add_static_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_mr_port)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(ds_mrouter_port_add_mrouter_port(p_db_igsp_if, p_mr_port));

    return PM_E_NONE;
}

int32
igsp_api_intf_add_dynamic_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_mr_port)
{
    ds_mrouter_port_t *p_db_mr_port = NULL;
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_db_igsp_if, p_mr_port);
    if (NULL == p_db_mr_port)
    {
        PM_E_RETURN(ds_mrouter_port_add_mrouter_port(p_db_igsp_if, p_mr_port));
        p_db_mr_port = ds_mrouter_port_get_mrouter_port(p_db_igsp_if, p_mr_port);
        if (NULL == p_db_mr_port)
        {
            return PM_E_NO_MEMORY;
        }
        IGSP_DEBUG(IGSP_EVENT, "Interface %s become mrouter port of vlan %u", 
               IFNAME_ETH2FULL(p_mr_port->key.name, ifname_ext), p_db_igsp_if->key.vid);
    }

    if (IGMP_MROUTE_STATIC == p_db_mr_port->type)
    {
        return PM_E_NONE;
    }
    else
    {
        p_db_mr_port->igsp_if = p_db_igsp_if;
        CTC_TASK_STOP_TIMER(p_db_mr_port->expire_timer);
        p_db_mr_port->expire_timer = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_mrouter_expire_fn, 
            p_db_mr_port, p_db_igsp_if->mrouter_aging_interval);
    }

    return PM_E_NONE;
}

int32
igsp_api_intf_del_mrouter_port(tbl_igsp_intf_t *p_db_igsp_if, ds_mrouter_port_t *p_db_mr_port)
{
    int32 rc = PM_E_NONE;
    CTC_TASK_STOP_TIMER(p_db_mr_port->expire_timer);
    PM_E_RETURN(ds_mrouter_port_del_mrouter_port(p_db_igsp_if, p_db_mr_port));

    return PM_E_NONE;
}

int32
igsp_api_intf_del_query_port(tbl_igsp_intf_t *p_db_igsp_if, ds_query_t *p_db_query)
{
    int32 rc = PM_E_NONE;

    CTC_TASK_STOP_TIMER(p_db_query->response_timer);
    PM_E_RETURN(ds_query_del_query(p_db_igsp_if, p_db_query));

    return PM_E_NONE;
}

int32
igsp_api_intf_update_querier_oper_addr(tbl_igsp_intf_t *p_igsp_if)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    addr_ipv4_t new_addr;

    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&p_igsp_if->key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }

    if (p_db_igsp_if->querier_config_address.s_addr)
    {
        new_addr.s_addr = p_db_igsp_if->querier_config_address.s_addr;
    }
    else if (p_db_igsp_if->vlan_if_address.s_addr)
    {
        new_addr.s_addr = p_db_igsp_if->vlan_if_address.s_addr;
    }
    else if (sal_hton32(INADDR_QUERY_ADDR) != p_db_glb->global_src.s_addr)
    {
        new_addr.s_addr = p_db_glb->global_src.s_addr;
    }
    else
    {
        new_addr.s_addr = sal_hton32(INADDR_QUERY_ADDR);
    }

    if (new_addr.s_addr == p_db_igsp_if->querier_oper_address.s_addr)
    {
        return PM_E_NONE;
    }

    p_igsp_if->querier_oper_address = new_addr;
    tbl_igsp_intf_set_igsp_intf_field(p_igsp_if, TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS);

    if (!p_db_igsp_if->querier_operate)
    {
        if (IPV4_ADDR_CMP(&p_db_igsp_if->other_querier_address, &p_db_igsp_if->querier_oper_address) >= 0)
        {
            igsp_api_intf_set_querier_operate(p_db_igsp_if, TRUE, 0);
        }
    }

    return PM_E_NONE;
}

uint32
igsp_api_intf_get_static_mrouter_count(tbl_igsp_intf_t *p_db_igsp_if)
{
    ctclib_slistnode_t *listnode = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    uint32 count = 0;
    
    CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
    {
        if (IGMP_MROUTE_STATIC == p_db_mrouter_port->type)
        {
            count++;
        }
    }

    return count;
}

int32
igsp_api_intf_update_querier_oper_addr_all()
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        igsp_api_intf_update_querier_oper_addr(p_db_igsp_if);
    }

    return PM_E_NONE;
}

int32
igsp_api_get_vlan_erps_enable(vid_t vid)
{
    tbl_vlan_t      *p_db_vlan = NULL;
    tbl_vlan_key_t  vlan_key;

    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (p_db_vlan && p_db_vlan->erps_domain_id)
    {
        return TRUE;
    }

    return FALSE;
}

int32
igsp_api_global_enable()
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_igsp_intf_t igsp_if;
    vid_t vid = 0;
    uint32 has_mcfdb = FALSE;
    
    if (!p_db_glb->enable)
    {
        p_db_glb->enable = TRUE;
        tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ENABLE);
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        
        if (p_db_igsp_if->enable)
        {
            continue;
        }

        has_mcfdb = brg_mcfdb_vlan_has_manual_mcfdb(vid);
        if (has_mcfdb)
        {
            continue;
        }
            
        igsp_if.key.vid = p_db_igsp_if->key.vid;
        igsp_if.enable = p_db_glb->enable;
        igsp_api_intf_enable(&igsp_if);
    }

    return PM_E_NONE;
}

int32
igsp_api_global_disable()
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_igsp_intf_t igsp_if;
    vid_t vid = 0;

    if (p_db_glb->enable)
    {
        p_db_glb->enable = FALSE;
        tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ENABLE);
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        
        if (!p_db_igsp_if->enable)
        {
            continue;
        }

        igsp_if.key.vid = p_db_igsp_if->key.vid;
        igsp_if.enable = p_db_glb->enable;
        igsp_api_intf_disable(&igsp_if);
    }

    p_db_glb->tcn_query_current_count = 0;
    CTC_TASK_STOP_TIMER(p_db_glb->t_tcn_query);

    return PM_E_NONE;
}

int32
igsp_api_global_set_fast_leave(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_FAST_LEAVE);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->fast_leave = p_glb->fast_leave;
        igsp_api_intf_set_fast_leave(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_discard_unknown(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_DISCARD_UNKNOWN);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->discard_unknown = p_glb->discard_unknown;
        igsp_api_intf_set_discard_unknown(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_report_suppress(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        if (IGMP_VERSION_3 == p_db_igsp_if->version && p_glb->report_suppress)
        {
            continue;
        }
        p_db_igsp_if->report_suppress = p_glb->report_suppress;
        igsp_api_intf_set_report_suppress(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_host_join(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb)
{
    return tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE);
}

int32
igsp_api_global_set_version(tbl_igsp_global_t *p_glb, tbl_igsp_global_t *p_db_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_VERSION);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->version = p_glb->version;
        igsp_api_intf_set_version(p_db_igsp_if);
    }

    if (IGMP_VERSION_3 == p_glb->version)
    {
        p_glb->report_suppress = FALSE;
        igsp_api_global_set_report_suppress(p_glb);
    }
    else
    {
        p_glb->report_suppress = TRUE;
        igsp_api_global_set_report_suppress(p_glb);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_querier_max_response_time(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->querier_max_response_time = p_glb->querier_max_response_time;
        igsp_api_intf_set_querier_max_response_time(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_query_interval(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->query_interval = p_glb->query_interval;
        igsp_api_intf_set_query_interval(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_lmqc(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_LMQC);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->lmqc = p_glb->lmqc;
        igsp_api_intf_set_lmqc(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_lmqi(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_LMQI);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->lmqi = p_glb->lmqi;
        igsp_api_intf_set_lmqi(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_robustness_var(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        p_db_igsp_if->robustness_var = p_glb->robustness_var;
        igsp_api_intf_set_robustness_var(p_db_igsp_if);
    }
    
    return PM_E_NONE;
}

int32
igsp_api_global_set_max_member_number(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER);

    return PM_E_NONE;
}

int32
igsp_api_global_set_global_src(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->global_src.s_addr = p_glb->global_src.s_addr;
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC);

    igsp_api_intf_update_querier_oper_addr_all();

    return PM_E_NONE;
}

int32
igsp_api_global_set_tcn_enable(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->tcn_enable = p_glb->tcn_enable;
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_ENABLE);

    if (p_db_glb->tcn_enable)
    {

    }
    else
    {
        p_db_glb->tcn_query_current_count = 0;
        CTC_TASK_STOP_TIMER(p_db_glb->t_tcn_query);
    }

    return PM_E_NONE;
}

int32
igsp_api_global_set_tcn_query_count(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->tcn_query_count = p_glb->tcn_query_count;
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT);

    return PM_E_NONE;
}

int32
igsp_api_global_set_tcn_query_interval(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->tcn_query_interval = p_glb->tcn_query_interval;
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL);

    return PM_E_NONE;
}

int32
igsp_api_global_set_tcn_query_max_response_time(tbl_igsp_global_t *p_glb)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->tcn_query_max_response_time = p_glb->tcn_query_max_response_time;
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME);

    return PM_E_NONE;
}

static int32
_igsp_set_default()
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    p_db_glb->enable = TRUE;
    p_db_glb->fast_leave = FALSE;
    p_db_glb->report_suppress = TRUE;
    p_db_glb->version = IGMP_VERSION_DEF;
    p_db_glb->max_member_number = IGMP_LIMIT_DEF;
    p_db_glb->query_interval = IGMP_QUERY_INTERVAL_DEF;
    p_db_glb->robustness_var = IGMP_ROBUSTNESS_VAR_DEF;
    p_db_glb->querier_max_response_time = IGMP_QUERY_RESPONSE_INTERVAL_DEF;
    p_db_glb->lmqc = IGMP_LAST_MEMBER_QUERY_COUNT_DEF;
    p_db_glb->lmqi = IGMP_LAST_MEMBER_QUERY_INTERVAL_DEF;
    p_db_glb->tcn_enable = FALSE;
    p_db_glb->tcn_query_count = IGMP_SNOOPING_TCN_QUERY_CNT;
    p_db_glb->tcn_query_interval = IGMP_SNOOPING_TCN_QUERY_INTERVAL;
    p_db_glb->tcn_query_max_response_time = IGMP_SNOOPING_TCN_QUERY_MAX_RES_TIME;
    p_db_glb->all_hosts.s_addr = sal_hton32(INADDR_ALLHOSTS_GROUP);
    p_db_glb->all_routers.s_addr = sal_hton32(INADDR_ALLRTRS_GROUP);
    p_db_glb->v3_routers.s_addr = sal_hton32(INADDR_V3RTRS_GROUP);
    p_db_glb->global_src.s_addr = sal_hton32(INADDR_QUERY_ADDR);
  
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ENABLE);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_VERSION);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_MAX_MEMBER_NUMBER);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_REPORT_SUPPRESS);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_QUERY_INTERVAL);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ROBUSTNESS_VAR);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_QUERIER_MAX_RESPONSE_TIME);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_LMQC);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_LMQI);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_COUNT);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_INTERVAL);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_TCN_QUERY_MAX_RESPONSE_TIME);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ALL_HOSTS);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_ALL_ROUTERS);
    tbl_igsp_global_set_igsp_global_field(p_db_glb, TBL_IGSP_GLOBAL_FLD_GLOBAL_SRC);

    return PM_E_NONE;
}

int32
igsp_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add)
{
    tbl_igsp_intf_t igsp_if;
    ds_mrouter_port_t mr_port;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;

    if (!is_add)
    {
        /* 1. clear VLAN's igmp snooping mrouter port list */
        igsp_if.key.vid = vid;
        sal_memset(&mr_port, 0, sizeof(mr_port));
        sal_strcpy(mr_port.key.name, p_db_if->key.name);
        p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&igsp_if.key);
        if (NULL == p_db_igsp_if)
        {
            return PM_E_NONE;
        }
        p_db_mrouter_port = ds_mrouter_port_get_mrouter_port(&igsp_if, &mr_port);
        if (p_db_mrouter_port)
        {
            igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mrouter_port);
        }

        /* 2. clear igmp snooping group records of this port + vlan */
        _igsp_api_clear_groups_by_port_vlan(p_db_if, vid);
    }

    return PM_E_NONE;
}

int32
igsp_api_group_add_static(tbl_igsp_group_t *p_igsp_grp, tbl_igsp_intf_t *p_db_igsp_if)
{
    int32 rc = PM_E_NONE;

    p_igsp_grp->type = IGMP_TYPE_STATIC;
    p_igsp_grp->update_time = ctc_time_boottime_sec(NULL);
    rc = _igsp_api_group_add(p_igsp_grp, p_db_igsp_if);
    
    return rc;
}

int32
igsp_api_group_del_static(tbl_igsp_group_t *p_igsp_grp)
{
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    int32 rc = PM_E_NONE;
    
    p_db_igsp_grp = tbl_igsp_group_get_igsp_group(&p_igsp_grp->key);
    if (NULL == p_db_igsp_grp)
    {
        return PM_E_NONE;
    }

    rc = _igsp_api_group_del_common(p_db_igsp_grp);
    
    return rc;
}

int32
igsp_api_add_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_group)
{
    ds_join_group_t *p_db_group = NULL;
    int32 rc = PM_E_NONE;
    
    p_group->type = IGMP_TYPE_STATIC;
    p_group->vid = p_db_igsp_if->key.vid;
        
    rc = ds_join_group_add_join_group(p_db_igsp_if, p_group);

    /* send join report to mrouter */
    p_db_group = ds_join_group_get_join_group(p_db_igsp_if, p_group);
    if (NULL == p_db_group)
    {
        return PM_E_NONE;
    }
    
    igsp_pdu_host_tx_report(p_db_igsp_if, p_db_group, TRUE);
    
    return rc;
}

int32
igsp_api_del_join_group(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_db_group)
{
    int32 rc = PM_E_NONE;
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();

    /* send leave report to mrouter */
    igsp_pdu_host_tx_report(p_db_igsp_if, p_db_group, FALSE);

    PM_E_RETURN(ds_join_group_del_join_group(p_db_igsp_if, p_db_group));

    p_glb->host_joined_num--;
    PM_E_RETURN(tbl_igsp_global_set_igsp_global_field(p_glb, TBL_IGSP_GLOBAL_FLD_HOST_JOINED_NUM));

    return rc;
}

static int32
_igsp_api_get_vid_group_entry_iter(tbl_igsp_group_t *p_group, tbl_iter_args_t *pargs)
{
    vid_t  *p_vid = (vid_t*)pargs->argv[0];
    uint32 *p_group_address = (uint32*)pargs->argv[1];
    tbl_igsp_group_t **pp_vid_group_entry = (tbl_igsp_group_t**)pargs->argv[2];

    if ((p_group->key.vid == *p_vid) && (p_group->key.ip.s_addr == *p_group_address))
    {
        *pp_vid_group_entry = p_group;
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

static tbl_igsp_group_t*
_igsp_api_get_vid_group_entry(vid_t vid, uint32 group_address)
{
    tbl_iter_args_t args;
    tbl_igsp_group_t *p_vid_group_entry = NULL;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &vid;
    args.argv[1] = &group_address;
    args.argv[2] = &p_vid_group_entry;
    
    tbl_igsp_group_iterate((TBL_ITER_CB_FUNC)_igsp_api_get_vid_group_entry_iter, &args);

    return p_vid_group_entry;
}
    
static int32
_igsp_api_do_sched_report(tbl_interface_t *p_src_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    uint32 group_address, uint32 is_v3, const ctc_msg_t *p_msg)
{
    tbl_igsp_global_t *p_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_group_t *p_vid_group_entry = NULL;
    uint32 need_forward = FALSE;
    ds_query_t query;
    ds_query_t *p_db_query = NULL;
    
    if (is_v3)
    {
        need_forward = FALSE;
    }
    else
    {
        p_vid_group_entry = _igsp_api_get_vid_group_entry(p_db_igsp_if->key.vid, group_address);
        if (NULL == p_vid_group_entry)
        {
            if (p_glb->curr_group_member >= p_glb->max_member_number)
            {
                need_forward = FALSE;
            }
            else if (p_db_igsp_if->curr_group_member >= p_db_igsp_if->max_member_number)
            {
                need_forward = FALSE;
            }
            else
            {
                /* new entry, need forward */
                need_forward = TRUE;
            }
        }
        else
        {
            if (!p_db_igsp_if->report_suppress)
            {
                /* Added by kcao for bug 38772, if existed entry is dynamic, need forward to mrouter-port, if existed entry is static, need not */
                if (IGMP_TYPE_DYNAMIC == p_vid_group_entry->type)
                {
                    need_forward = TRUE;
                }
                else
                {
                    /* Added by kcao for bug 38941, if same interface, not forward */
                    if (p_vid_group_entry->key.ifindex == p_src_db_if->ifindex)
                    {
                        need_forward = FALSE;
                    }
                    else
                    {
                        need_forward = TRUE;
                    }
                }
            }
            else
            {
                query.key.group.s_addr = group_address;
                p_db_query = ds_query_get_query(p_db_igsp_if, &query);
                if ((p_db_query))
                {
                    need_forward = TRUE;
                    igsp_api_intf_del_query_port(p_db_igsp_if, p_db_query);/*modify by yangl for bug 47294 2018-11-28*/
                }
                
            }
        }
    }

    if (need_forward)
    {
        /* remove query group entry */
        igsp_pdu_forward_report(p_src_db_if, p_db_igsp_if, p_msg);
    }

    return PM_E_NONE;
}

int32
igsp_api_group_add_dynamic(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    uint32 group_address, uint32 src_address, uint32 is_v1, uint32 is_v3, const ctc_msg_t *p_msg)
{
    tbl_igsp_group_t igsp_grp;
    tbl_igsp_group_t *p_igsp_grp = &igsp_grp;
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    /* mirror dest port should not learning */
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        return PM_E_NONE;
    }
    
    sal_memset(&igsp_grp, 0, sizeof(igsp_grp));
    p_igsp_grp->key.vid = p_db_igsp_if->key.vid;
    p_igsp_grp->key.ifindex = p_db_if->ifindex;
    p_igsp_grp->key.ip.s_addr = group_address;
    p_igsp_grp->last_reporter_address.s_addr = src_address;

    p_igsp_grp->type = IGMP_TYPE_DYNAMIC;
    p_igsp_grp->update_time = ctc_time_boottime_sec(NULL);
    p_igsp_grp->liveness = p_db_igsp_if->group_member_interval;
    if (is_v1)
    {
        p_igsp_grp->is_v1_host_exist = TRUE;
        p_igsp_grp->v1_update_time = ctc_time_boottime_sec(NULL);

        IGSP_DEBUG(IGSP_TIMER, "group %s vlan %u ifname %s, update version-1 host present timer expiry in %u",
            sal_inet_ntop(AF_INET, (void*) (&group_address), ip_str, CMD_BUF_16), p_igsp_grp->key.vid, 
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_igsp_grp->liveness);
    }

    /* should judge is_new before add group to CDB */
    _igsp_api_do_sched_report(p_db_if, p_db_igsp_if, group_address, is_v3, p_msg);
    
    p_db_igsp_grp = tbl_igsp_group_get_igsp_group(&p_igsp_grp->key);
    if (NULL == p_db_igsp_grp)
    {
        _igsp_api_group_add(p_igsp_grp, p_db_igsp_if);
    } 
    else
    {
        if (IGMP_TYPE_DYNAMIC == p_db_igsp_grp->type)
        {
            if (p_db_igsp_grp->update_time != p_igsp_grp->update_time)
            {
                p_db_igsp_grp->update_time = p_igsp_grp->update_time;
            }
            if (p_db_igsp_grp->liveness != p_igsp_grp->liveness)
            {
                p_db_igsp_grp->liveness = p_igsp_grp->liveness;
            }
            if (p_db_igsp_grp->last_reporter_address.s_addr != p_igsp_grp->last_reporter_address.s_addr)
            {
                p_db_igsp_grp->last_reporter_address.s_addr = p_igsp_grp->last_reporter_address.s_addr;
            }
            if (p_igsp_grp->is_v1_host_exist)
            {
                p_db_igsp_grp->is_v1_host_exist = TRUE;
                if (p_db_igsp_grp->v1_update_time != p_igsp_grp->v1_update_time)
                {
                    p_db_igsp_grp->v1_update_time = p_igsp_grp->v1_update_time;
                }
            }

            if (CTC_TASK_IS_RUNNING(p_db_igsp_grp->t_retx_query))
            {
                CTC_TASK_STOP_TIMER(p_db_igsp_grp->t_retx_query);
            }
            p_db_igsp_grp->has_rx_leave = FALSE;
        }
    }
    
    return PM_E_NONE;
}

uint32
igsp_api_get_group_count(tbl_igsp_group_t *p_lookup_grp)
{
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_t *hash = p_master->group_hash;
    tbl_igsp_group_t *p_db_group = NULL;
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;
    uint32 count = 0;

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_group = hb->data;
            if ((p_lookup_grp->key.vid == p_db_group->key.vid)
             && (p_lookup_grp->key.ip.s_addr == p_db_group->key.ip.s_addr ))
            {
                count++;
            }
        }
    }

    return count;
}

int32
igsp_api_group_del_dynamic(tbl_igsp_group_t *p_db_igsp_grp)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_igsp_grp)
    {
        return PM_E_NONE;
    }
    if (IGMP_TYPE_STATIC == p_db_igsp_grp->type)
    {
        return PM_E_NONE;
    }

    rc = _igsp_api_group_del_common(p_db_igsp_grp);

    return rc;
}

void
igsp_api_group_retx_query_fn(void* p_data)
{
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_igsp_group_t *p_db_igsp_grp = (tbl_igsp_group_t *)p_data;
    char ip_str[CMD_BUF_16];
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];

    p_db_igsp_grp->t_retx_query = NULL;

    key.vid = p_db_igsp_grp->key.vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return;
    }
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_igsp_grp->key.ifindex);
    if (NULL == p_db_if)
    {
        return;
    }

    sal_memset(ifname, 0, IFNAME_SIZE);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    tbl_interface_get_name_by_ifindex(p_db_igsp_grp->key.ifindex, ifname, IFNAME_SIZE);
    p_db_igsp_grp->retx_group_lmqc--;
    IGSP_DEBUG(IGSP_TIMER, "group %s vlan %u ifname %s retx timeout, liveness %u, retx_group_lmqc %u", 
        cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_igsp_grp->key.ip), p_db_igsp_grp->key.vid, 
        IFNAME_ETH2FULL(ifname, ifname_ext), p_db_igsp_grp->liveness, p_db_igsp_grp->retx_group_lmqc);

    if (p_db_igsp_grp->retx_group_lmqc)
    {
        igsp_pdu_tx_group_query(p_db_if, p_db_igsp_if, p_db_igsp_grp->key.ip.s_addr);
        p_db_igsp_grp->t_retx_query = ctc_task_add_timer_msec(TASK_PRI_NORMAL, igsp_api_group_retx_query_fn, 
                p_db_igsp_grp, p_db_igsp_if->lmqi);
    }
    
    return;
}

int32
igsp_api_group_rx_leave(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address, uint32 is_v3_leave)
{
    tbl_igsp_group_key_t igsp_grp_key;
    tbl_igsp_group_t *p_db_igsp_grp = NULL;
    uint32 lmqt = 0;
    char ip_str[CMD_BUF_16];
    
    sal_memset(&igsp_grp_key, 0, sizeof(igsp_grp_key));
    igsp_grp_key.vid = p_db_igsp_if->key.vid;
    igsp_grp_key.ifindex = p_db_if->ifindex;
    igsp_grp_key.ip.s_addr = group_address;

    p_db_igsp_grp = tbl_igsp_group_get_igsp_group(&igsp_grp_key);
    if (NULL == p_db_igsp_grp)
    {
        return PM_E_NONE;
    }

    if (_igsp_api_group_is_v1_host_exist(p_db_igsp_grp))
    {
        IGSP_DEBUG(IGSP_EVENT, "group %s vlan %u, ignore igmp leave message while version-1 host present", 
          sal_inet_ntop(AF_INET, (void*)(&(p_db_igsp_grp->key.ip.s_addr)), ip_str, CMD_BUF_16), p_db_igsp_grp->key.vid);
        return PM_E_NONE;
    }

    p_db_igsp_grp->is_v3_leave = is_v3_leave;
    if (p_db_igsp_if->fast_leave)
    {
        /* fast-leave */
        igsp_api_group_del_dynamic(p_db_igsp_grp);
    }
    else
    {
        p_db_igsp_grp->retx_group_lmqc = p_db_igsp_if->lmqc;
        if (!p_db_igsp_grp->has_rx_leave)
        {
            if (p_db_igsp_grp->retx_group_lmqc)
            {
                igsp_pdu_tx_group_query(p_db_if, p_db_igsp_if, group_address);
                CTC_TASK_STOP_TIMER(p_db_igsp_grp->t_retx_query);
                p_db_igsp_grp->t_retx_query = ctc_task_add_timer_msec(TASK_PRI_NORMAL, igsp_api_group_retx_query_fn, 
                        p_db_igsp_grp, p_db_igsp_if->lmqi);
            }
            lmqt = (p_db_igsp_if->lmqi / IGMP_ONE_SEC_MILLISECOND) * p_db_igsp_if->lmqc;
            p_db_igsp_grp->update_time = ctc_time_boottime_sec(NULL);
            p_db_igsp_grp->liveness = lmqt;
            p_db_igsp_grp->has_rx_leave = TRUE;
        }
    }

    return PM_E_NONE;
}

void
igsp_group_expire_time_cb(void* p_data)
{
    tbl_igsp_global_t *p_db_glb = (tbl_igsp_global_t*)p_data;
    ctclib_hash_backet_t *hb = NULL;
    ctclib_hash_backet_t *next = NULL;
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    tbl_igsp_group_t *p_db_grp = NULL;
    ctclib_hash_t *hash = p_master->group_hash;
    sal_time_t now;
    int32 delta = 0;
    int32 delta_v1_host = 0;
    uint32 i = 0;
    char ip_str[CMD_BUF_16];
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    
    p_db_glb->t_group_expire = NULL;

    now = ctc_time_boottime_sec(NULL);

    for (i = 0; i < hash->size; i++)
    {
        for (hb = hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_grp = (tbl_igsp_group_t*)hb->data;
            if (IGMP_TYPE_STATIC == p_db_grp->type)
            {
                continue;
            }

            sal_memset(ifname, 0, IFNAME_SIZE);
            tbl_interface_get_name_by_ifindex(p_db_grp->key.ifindex, ifname, IFNAME_SIZE);
            if (p_db_grp->is_v1_host_exist)
            {
                delta_v1_host = now - p_db_grp->v1_update_time;
                if (delta_v1_host >= p_db_grp->liveness)
                {
                    p_db_grp->is_v1_host_exist = FALSE;
                    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
                    IGSP_DEBUG(IGSP_TIMER, "group %s vlan %u ifname %s, version-1 host present timer expired",
                      cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_grp->key.ip), p_db_grp->key.vid, 
                                            IFNAME_ETH2FULL(ifname, ifname_ext));
                }
            }

            delta = now - p_db_grp->update_time;
            if (delta < p_db_grp->liveness)
            {
                continue;
            }
            sal_memset(ip_str, 0, CMD_BUF_16);
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_TIMER, "group %s vlan %u ifname %s is expired for liveness %u", 
                sal_inet_ntop(AF_INET, (void*)(&(p_db_grp->key.ip.s_addr)), ip_str, CMD_BUF_16), 
                p_db_grp->key.vid, IFNAME_ETH2FULL(ifname, ifname_ext), p_db_grp->liveness);
            igsp_api_group_del_dynamic(p_db_grp);
        }
    }
    
    p_db_glb->t_group_expire = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_group_expire_time_cb,
        p_db_glb, IGMP_GROUP_EXPIRE_INTERVAL);

    return;
}

int32
igsp_api_intf_set_vlan_if_addr(vid_t vid, addr_ipv4_t *addr)
{
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;

    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_igsp_if->vlan_if_address.s_addr = addr->s_addr;
    tbl_igsp_intf_set_igsp_intf_field(p_db_igsp_if, TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS);

    igsp_api_intf_update_querier_oper_addr(p_db_igsp_if);

    return PM_E_NONE;
}

int32
igsp_process_route_if_ds_connected_msg(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_route_if_t *p_rt_if = NULL;
    ds_connected_t *p_conn = NULL;
    uint32 value = 0;
    vid_t vid = 0;
    addr_ipv4_t addr;

    p_rt_if  = (tbl_route_if_t *)p_tbl;
    p_conn  = (ds_connected_t *)p_ds;

    if (0 != sal_strncmp(p_rt_if->key.name, GLB_IFNAME_VLAN_PREFIX, 4))
    {
        return PM_E_NONE;
    }

    sal_sscanf(p_rt_if->key.name, GLB_IFNAME_VLAN_PREFIX"%u", &value);
    vid = value;
    switch (oper) 
    {
    case CDB_OPER_ADD:
        addr.s_addr = p_conn->key.address.u.prefix4.s_addr;
        igsp_api_intf_set_vlan_if_addr(vid, &addr);
        break;
    case CDB_OPER_DEL:
        addr.s_addr = 0;
        igsp_api_intf_set_vlan_if_addr(vid, &addr);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

int32
igsp_process_route_if_msg(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_route_if_t *p_rt_if = NULL;
    //int32 rc = PM_E_NONE;

    p_rt_if  = (tbl_route_if_t *)p_tbl;
    (void)p_rt_if;
    
    switch (oper) 
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            if (DS_CONNECTED == p_ds_node->id)
            {
                return igsp_process_route_if_ds_connected_msg(p_tbl_node, 
                    p_ds_node, oper, field_id, p_tbl, p_ds);
            }
        }
        break;

    case CDB_OPER_SET:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    
    return PM_E_NONE;
}

int32
igsp_api_if_down_cb(tbl_interface_t *p_db_if)
{
    _igsp_api_clear_groups_by_port(p_db_if, FALSE);
    _igsp_api_clear_mrouter_by_port(p_db_if);

    return PM_E_NONE;
}

int32
igsp_api_if_del_cb(tbl_interface_t *p_db_if)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nn = NULL;
    uint32 vid = 0;
    int32 rc = PM_E_NONE;

    _igsp_api_clear_groups_by_port(p_db_if, TRUE);

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->mrouter_list.obj_list, p_db_mr_port, listnode, nn)
        {
            if (0 == sal_strcmp(p_db_mr_port->key.name, p_db_if->key.name))
            {
                igsp_api_intf_del_mrouter_port(p_db_igsp_if, p_db_mr_port);
            }
        }
    }

    return rc;
}

int32
igsp_api_if_add_to_agg_cb(tbl_interface_t *p_db_if)
{
    return igsp_api_if_del_cb(p_db_if);
}
    
int32
igsp_api_vlan_disable_cb(tbl_vlan_t *p_db_vlan)
{
    igsp_api_clear_groups_by_vlan(NULL, p_db_vlan->key.vid);

    return PM_E_NONE;
}

uint32
igsp_api_is_igmp_snooping_en(vid_t vid)
{
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    
    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return FALSE;
    }
    
    return p_db_igsp_if->enable;
}

void
igsp_api_timer_tcn_query_fn(void* p_data)
{
    tbl_igsp_intf_master_t *p_master = tbl_igsp_intf_get_master();
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    vid_t vid = 0;
    
    p_db_glb->t_tcn_query = NULL;

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_igsp_if = p_master->igsp_array[vid];
        if (NULL == p_db_igsp_if)
        {
            continue;
        }
        if (p_db_igsp_if->enable)
        {
            igsp_pdu_tx_general_query(p_db_igsp_if, p_db_glb->tcn_query_max_response_time, 1);
        }
    }
    
    p_db_glb->tcn_query_current_count++;

    IGSP_DEBUG(IGSP_TIMER, "TCN query timeout current_count %u, query_count %u", 
        p_db_glb->tcn_query_current_count, p_db_glb->tcn_query_count);
    
    if (p_db_glb->tcn_query_current_count >= p_db_glb->tcn_query_count)
    {
        p_db_glb->tcn_query_current_count = 0;
    }
    else
    {
        p_db_glb->t_tcn_query = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_tcn_query_fn,
            NULL, p_db_glb->tcn_query_interval);
    }

    return;
}

int32
igsp_api_mstp_topology_change_cb(uint32 new_topology_change_detected)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();

    if (new_topology_change_detected)
    {
        if (p_db_glb->enable && p_db_glb->tcn_enable)
        {
            CTC_TASK_STOP_TIMER(p_db_glb->t_tcn_query);
            p_db_glb->tcn_query_current_count = 0;
            p_db_glb->t_tcn_query = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_tcn_query_fn, 
                NULL, 0);
        }
    }
    
    return PM_E_NONE;
}

int32
igsp_api_intf_acl_prefix_check(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 address, uint32 src_address)
{
    prefix_t group_addr;
    prefix_t src_addr;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char srcip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(&group_addr, 0, sizeof(group_addr));
    group_addr.family = AF_INET;
    group_addr.prefixlen = IPV4_MAX_BITLEN;
    group_addr.u.prefix4.s_addr = address;
    
    sal_memset(&src_addr, 0, sizeof(src_addr));
    src_addr.family = AF_INET;
    src_addr.prefixlen = IPV4_MAX_BITLEN;
    src_addr.u.prefix4.s_addr = src_address;
    
    rc = acl_prefix_check(p_db_igsp_if->acl_name, &group_addr, &src_addr);
    if (rc == PM_E_FAIL)
    {
        sal_memset(ip_str, 0, CMD_BUF_16);
        sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
        IGSP_DEBUG(IGSP_PACKET_RX, "Interface %s IGMP snooping vlan %u group %s source %s is filter by access-list",
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid, 
            sal_inet_ntop(AF_INET, (void*)(&address), ip_str, CMD_BUF_16), sal_inet_ntop(AF_INET, (void*)(&src_address), srcip_str, CMD_BUF_16));
    }
    else if (rc == PM_E_INVALID_PARAM)
    {
        IGSP_DEBUG(IGSP_PACKET_RX, "Interface %s IGMP snooping vlan %u group %s source %s can't filter by non-ipv4 access-list %s",
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid, 
            sal_inet_ntop(AF_INET, (void*)(&address), ip_str, CMD_BUF_16), sal_inet_ntop(AF_INET, (void*)(&src_address), srcip_str, CMD_BUF_16), 
            p_db_igsp_if->acl_name);
    }

    return rc;
}

int32
igsp_init()
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_vlan_master_t *p_master = tbl_vlan_get_master();
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;
    
    _igsp_set_default();

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        igsp_api_intf_do_add(p_db_vlan->key.vid);
    }

    p_db_glb->t_group_expire = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_group_expire_time_cb,
        p_db_glb, IGMP_GROUP_EXPIRE_INTERVAL);

    return PM_E_NONE;
}

int32
igsp_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_IGSP_GLOBAL, igsp_cmd_process_global);
    cdb_register_cfg_tbl_cb(TBL_IGSP_INTF, igsp_cmd_process_intf);
    cdb_register_cfg_tbl_cb(TBL_IGSP_GROUP, igsp_cmd_process_group);
    cdb_register_cfg_act_cb(ACT_SHOW_IGSP, igsp_cmd_process_act_show);
    cdb_register_cfg_act_cb(ACT_CLEAR_IGSP, igsp_cmd_process_act_clear);
    
    cdb_pm_subscribe_tbl(TBL_ROUTE_IF, igsp_process_route_if_msg, NULL);
    
    ipc_register_pkt_rx_fn(GLB_PKT_IGMP, igsp_pdu_rx);
    ipc_register_pkt_rx_fn(GLB_PKT_PIM, igsp_pdu_rx_pim);
    return PM_E_NONE;
}

int32
igsp_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_IGSP_GLOBAL, NULL);
    cdb_register_cfg_tbl_cb(TBL_IGSP_INTF, NULL);
    cdb_register_cfg_tbl_cb(TBL_IGSP_GROUP, NULL);
    cdb_register_cfg_act_cb(ACT_SHOW_IGSP, NULL);
    cdb_register_cfg_act_cb(ACT_CLEAR_IGSP, NULL);

    ipc_register_pkt_rx_fn(GLB_PKT_IGMP, NULL);
    ipc_register_pkt_rx_fn(GLB_PKT_PIM, NULL);
    return PM_E_NONE;
}

