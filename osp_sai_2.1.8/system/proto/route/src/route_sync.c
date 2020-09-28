
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "route.h"
#include "rt_if.h"
#include "arp_api.h"
#include "arpratelimit_api.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "vrrp.h" 
#include "vrrp_api.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "ipsla_api.h"
#include "track_api.h"

extern int32
nd_address_install(tbl_route_if_t *p_db_rtif, void* ifc_v6, uint32 new_mirror_enable);

extern int32
rt_cmd_reset_ospf_process(void);

#ifdef CONFIG_IPV4

static int32
_rt_interface_change_running(tbl_interface_t *p_db_if, uint32 old_running, uint32 new_running)
{
    int32 rc = PM_E_NONE;
    rc |= rt_if_running_change(p_db_if, old_running, new_running);

    return rc;
}

static int32
_rt_interface_change_up(tbl_interface_t *p_db_if, uint32 old_up, uint32 new_up)
{
    int32 rc = PM_E_NONE;
    rc |= rt_if_up_change(p_db_if, old_up, new_up);
    
    return rc;
}

static int32
_rt_interface_sync_lag_before_add(uint32 field_id, tbl_interface_t *p_db_if, tbl_interface_t *p_if, void *p_ds)
{
    tbl_route_if_t  *p_route_if = NULL;    
    int32           rc = PM_E_NONE;

    switch (field_id)
    {
    case 0:
        p_route_if = rt_interface_check_rif_exist(p_db_if);
        if (p_route_if)
        {
            rc = rt_if_clear_if(p_db_if);
        }
        break;
    
    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_rt_interface_sync_before_set(uint32 field_id, tbl_interface_t *p_db_if, tbl_interface_t *p_if, void *p_ds)
{
    uint32 old_running = FALSE;
    uint32 new_running = FALSE;
    uint32 old_up = FALSE;
    uint32 new_up = FALSE;
    int32 rc = PM_E_NONE;
    
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_MODE:
        if (p_db_if->mode != p_if->mode)
        {
            rc = rt_if_set_mode(p_db_if->key.name, p_if->mode);
        }
        break;

    case TBL_INTERFACE_FLD_UP:
        old_up = tbl_interface_is_up(p_db_if);
        new_up = tbl_interface_is_up(p_if);
        if (old_up != new_up)
        {
            rc = _rt_interface_change_up(p_db_if, old_up, new_up);
        }
        break;

    case TBL_INTERFACE_FLD_RUNNING:
        old_running = rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable);
        new_running = rt_if_is_nonmirror_running(p_if->flags, p_db_if->mirror_enable);
        if (old_running != new_running)
        {
            rc = _rt_interface_change_running(p_db_if, old_running, new_running);
        }
        break;

    case TBL_INTERFACE_FLD_MIRROR_ENABLE:
        old_running = rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable);
        new_running = rt_if_is_nonmirror_running(p_db_if->flags, p_if->mirror_enable);
        if (old_running != new_running)
        {
            tbl_route_if_t *p_db_rtif   = NULL;
            
            p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&p_db_if->key);
            if (NULL != p_db_rtif)
            {
                if (new_running)
                {
                    p_db_rtif->set_mirror_enable = FALSE;
                }
                else
                {
                    p_db_rtif->set_mirror_enable = TRUE;
                }
            }
            
            rc = _rt_interface_change_running(p_db_if, old_running, new_running);
        }
        break;

    case TBL_INTERFACE_FLD_MTU:
        if (p_db_if->mtu != p_if->mtu)
        {
            rc = rt_if_set_mtu(p_db_if->key.name, p_if->mtu);
        }
        break;

    case TBL_INTERFACE_FLD_OPER_BANDWIDTH:
        if (p_db_if->oper_bandwidth != p_if->oper_bandwidth)
        {
            rc = rt_if_set_ospf_bandwidth(p_db_if, p_if->oper_bandwidth);
        }
        break;

    case TBL_INTERFACE_FLD_MIRROR_DEST_ID:
        rc = _arp_api_if_set_mirror_update(p_if, p_db_if->key.name);
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE:
        if (p_db_if->arp_numberlimit_enable != p_if->arp_numberlimit_enable)
        {
            rc = _arp_api_if_set_arp_numberlimit_enable(p_db_if, p_if->arp_numberlimit_enable);
        }
        break;

    case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER:
        if (p_db_if->arp_numberlimit_number != p_if->arp_numberlimit_number)
        {
            rc = _arp_api_if_set_arp_numberlimit_number(p_db_if, p_if->arp_numberlimit_number);
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_rt_interface_sync_after_set(uint32 field_id, tbl_interface_t *p_db_if)
{
    uint32 new_running = FALSE;
    int32 rc = PM_E_NONE;
    tbl_route_if_t *p_db_rtif   = NULL;
    ds_connected_v6_t      *p_db_conn  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    
    switch (field_id)
    {
    case TBL_INTERFACE_FLD_RUNNING:
        new_running = rt_if_is_nonmirror_running(p_db_if->flags, p_db_if->mirror_enable);
        /* support vrrp modified by liwh for bug 45215, 2017-09-19 */
        if (new_running)
        {
            vrrp_if_up(p_db_if);
        }
        else
        {
            vrrp_if_down(p_db_if);
        }
        /* liwh end */

        rc = track_quick_update_interface_status();
        break;

    case TBL_INTERFACE_FLD_UP:
        rc = track_quick_update_interface_status();
        break;

    case TBL_INTERFACE_FLD_MIRROR_ENABLE:
        p_db_rtif = tbl_route_if_get_route_if((tbl_route_if_key_t*)&p_db_if->key);
        if (NULL != p_db_rtif)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, listnode)
            {
                if (!p_db_rtif->set_mirror_enable && !p_db_if->mirror_enable)
                {
                    /* set interface running, otherwise can't start nd dad check */
                    p_db_if->flags = p_db_if->flags | (GLB_IFF_UP|GLB_IFF_RUNNING);
         
                    /* start nd dad check and install ipv6 address*/
                    nd_address_install(p_db_rtif, p_db_conn, FALSE);
                }
            }
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}


static int32
_rt_interface_sync_after_add(tbl_interface_t *p_db_if)
{
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    tbl_route_if_t  *p_route_if = NULL;
    uint32          vlanid = 0;
    int32           rc = PM_E_NONE;

    if (NULL != p_db_if && GLB_IF_MODE_L3 == p_db_if->mode)
    {
        p_route_if = rt_interface_check_rif_exist(p_db_if);
        if (NULL == p_route_if)
        {
            rc = rt_if_add_if(p_db_if->key.name);
        }
    }

    /*added by yejl to fix bug44070, 2017-05-19*/
    if (p_db_if->hw_type == GLB_IF_TYPE_VLAN_IF)
    {
        sal_sscanf(p_db_if->key.name, GLB_IFNAME_VLAN_PREFIX"%u", &vlanid);
        sal_memset(&arpinsp_key, 0, sizeof(arpinsp_key));
        arpinsp_key.vlan_id = vlanid;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (p_db_arpinsp)
        {
            if (p_db_arpinsp->enable_state)
            {
                PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_db_arpinsp, TBL_ARPINSP_FLD_ENABLE_STATE));
            }
        }
    }
    /*ended by yejl*/

    return rc;
}

static int32
_rt_interface_sync_before_del(tbl_interface_t *p_db_if)
{
    tbl_arpinsp_t *p_db_arpinsp = NULL;
    tbl_arpinsp_key_t arpinsp_key;
    tbl_route_if_t  *p_route_if = NULL; 
    uint32          vlanid = 0;
    int32           rc = PM_E_NONE;
    
    if (NULL != p_db_if && GLB_IF_MODE_L3 == p_db_if->mode)
    {
        p_route_if = rt_interface_check_rif_exist(p_db_if);
        if (NULL != p_route_if)        
        {
            /*delete ipv6 static route by if*/
            static_delete_by_if (AFI_IP6, p_db_if->key.name);
            rc = rt_if_del_if(p_db_if->key.name);
        }
    }
    
    /*added by yejl to fix bug44070, 2017-05-19*/
    if (p_db_if->hw_type == GLB_IF_TYPE_VLAN_IF)
    {
        sal_sscanf(p_db_if->key.name, GLB_IFNAME_VLAN_PREFIX"%u", &vlanid);
        sal_memset(&arpinsp_key, 0, sizeof(arpinsp_key));
        arpinsp_key.vlan_id = vlanid;
        p_db_arpinsp = tbl_arpinsp_get_arpinsp(&arpinsp_key);
        if (p_db_arpinsp)
        {
            if (p_db_arpinsp->enable_state)
            {
                PM_E_RETURN(tbl_arpinsp_set_arpinsp_field(p_db_arpinsp, TBL_ARPINSP_FLD_ENABLE_STATE));
            }
        }
    }
    /*ended by yejl*/
    
    return rc;
}

static int32
_rt_interface_sync_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t *p_if = NULL;
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            if (DS_LAG == p_ds_node->id)
            {
                p_if_key = (tbl_interface_key_t*)p_tbl;
                p_if = (tbl_interface_t*)p_tbl;
                p_db_if = tbl_interface_get_interface(p_if_key);
                if (NULL == p_db_if)
                {
                    SAL_ASSERT(0);
                }
                _rt_interface_sync_lag_before_add(field_id, p_db_if, p_if, p_ds);
            }
        }
        break;

    case CDB_OPER_SET:
        /* Added by kcao for bug 31492, if TBL_INTERFACE.DS_XXX_FLD_XXX is changed, ignore */
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = (tbl_interface_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            /* Modified by kcao 2016-09-29 for bug 40960, Interface maybe set from both switch and fea process */
            return PM_E_NOT_EXIST;
        }
        rc = _rt_interface_sync_before_set(field_id, p_db_if, p_if, p_ds);
        break;

    case CDB_OPER_DEL:
        if (p_ds_node)
        {
            /* delete sub DS */
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = (tbl_interface_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            SAL_ASSERT(0);
        }
        rc = _rt_interface_sync_before_del(p_db_if);
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

static int32
_rt_interface_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t *p_if = NULL;
    int32 rc = PM_E_NONE;
    tbl_route_if_t *p_db_rtif = NULL;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        if (p_ds_node)
        {
            if (DS_LAG == p_ds_node->id)
            {
                p_if_key = (tbl_interface_key_t*)p_tbl;
                p_if = (tbl_interface_t*)p_tbl;
                p_db_if = tbl_interface_get_interface(p_if_key);
                if (NULL == p_db_if)
                {
                    SAL_ASSERT(0);
                }
                p_db_rtif = tbl_route_if_get_route_if_by_name(p_if_key->name);
                if (NULL == p_db_rtif)
                {
                    return PM_E_FAIL;
                }

                arpratelimit_init_para_for_add_if(p_db_rtif);
            }
            
            /* add sub DS */
            break;
        }

        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = (tbl_interface_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            SAL_ASSERT(0);
        }
        
        rc = _rt_interface_sync_after_add(p_if);
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = (tbl_interface_t*)p_tbl;
        p_db_if = tbl_interface_get_interface(p_if_key);
        if (NULL == p_db_if)
        {
            return PM_E_NOT_EXIST;
        }
        
        rc = _rt_interface_sync_after_set(field_id, p_db_if);
        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        if (NULL != p_if_key)
        {
            p_db_if = tbl_interface_get_interface(p_if_key);

            if (NULL == p_db_if)
            {
                return PM_E_NOT_EXIST;
            }
        }

        ipsla_update_src_intf_delete(p_db_if);

        track_quick_update_interface_status();
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

int32
_rt_ds_lag_refer_sync_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds,
    cdb_sync_refer_t *p_refer)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_db_lag = NULL;
    tbl_interface_key_t *p_refer_tbl = NULL;
    uint32 add = FALSE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    p_refer_tbl = (tbl_interface_key_t*)p_refer->object;
    p_db_if = tbl_interface_get_interface(p_if_key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }
    
    p_db_lag = p_db_if->lag;

    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;

    log_sys(M_MOD_LIB, E_ERROR, "before callback %s %s %s current has %u members", p_if_key->name, add ? "add" : "del", 
        p_refer_tbl->name, p_db_lag->member_ports.obj_list->count);
    return PM_E_NONE;
}

int32
_rt_ds_lag_refer_sync_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds,
    cdb_sync_refer_t *p_refer)
{
    tbl_interface_key_t *p_if_key = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_db_lag = NULL;
    tbl_interface_key_t *p_refer_tbl = NULL;
    uint32 add = FALSE;

    p_if_key = (tbl_interface_key_t*)p_tbl;
    p_refer_tbl = (tbl_interface_key_t*)p_refer->object;
    p_db_if = tbl_interface_get_interface(p_if_key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }
    
    p_db_lag = p_db_if->lag;

    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;
    
    log_sys(M_MOD_LIB, E_ERROR, "after callback %s %s %s current has %u members", p_if_key->name, add ? "add" : "del", 
        p_refer_tbl->name, p_db_lag->member_ports.obj_list->count);
    
    return PM_E_NONE;
}

int32 
_rt_set_quagga_log_module_severity(const char* level)
{
#ifndef TAPPRODUCT
    char szCmd[512];

    snprintf(szCmd, 512, "log syslog %s", level);
    rt_vty_config_cmd(szCmd);
#endif

    return PM_E_NONE;
}

static int32
_rt_syslog_cfg_sync_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_syslog_cfg_t *p_log_cfg = NULL; 
    const char* severity_str = NULL;
    int32 rc = PM_E_NONE;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_log_cfg = (tbl_syslog_cfg_t*)p_tbl;
        switch (field_id)
        {
        case TBL_SYSLOG_CFG_FLD_FILE_SEVERITY:
            break;

        case TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY:
            /* bug50853 frr_syslog_severity_strs instead syslog_severity_strs */
            severity_str = cdb_enum_val2str(frr_syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_log_cfg->module_severity);
            if (severity_str)
            {
                rc = _rt_set_quagga_log_module_severity(severity_str);
            }
            break;

        default:
            break;
        }
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

static int32
_rt_proc_startup_done()
{
    _rt_set_quagga_log_module_severity("debug");
    rt_cmd_reset_ospf_process();
    rt_cmd_set_vrrp_process();

    if (TRUE == rt_api_is_ipv6_profile())
    {
        route_set_ipv6_enable(TRUE);
    }

    return PM_E_NONE;
}

static int32
_rt_proc_init_done()
{
    tbl_route_global_t  *p_db_rt_glb      = tbl_route_global_get_route_global();
    
    if (NULL != p_db_rt_glb)
    {
        p_db_rt_glb->nd_hop_limit = GLB_ND_DEFAULT_IPV6_HOP_LIMIT;
        tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT);
    }

    rt_vty_init();
    
    return PM_E_NONE;
}

static int32
_rt_process_sys_global_cb(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id)
        {
            _rt_proc_startup_done();
        }
        else if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id)
        {
            _rt_proc_init_done();
        }
        break;

    default:
        break;
    }
    
    return rc;

}


static int32
_arp_process_sync_ptp_port_before_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_ptp_port_t *p_ptp_port = NULL;
    tbl_arp_t       *ae_p = NULL;
    char buf[CMD_BUF_256];
    tbl_ptp_port_t *p_db_ptp_port = NULL;
    
//    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        
        p_ptp_port = (tbl_ptp_port_t*)p_tbl;
        switch (field_id)
        {
        case TBL_PTP_PORT_FLD_PORT_ADDR:
            p_db_ptp_port = tbl_ptp_port_get_ptp_port(&(p_ptp_port->key));
            if (NULL == p_db_ptp_port)
            {
                return PM_E_FAIL;
            }
            
            if (p_ptp_port->port_addr.dest_ip_addr.s_addr == 0 || p_ptp_port->port_addr.protocol == PTP_NET_PROTO_IEEE_802_3 || 
                sal_memcmp(&(p_ptp_port->port_addr.field.ip_addr), &(p_db_ptp_port->port_addr.field.ip_addr), sizeof(addr_ipv4_t)) || 
                sal_memcmp(&(p_ptp_port->port_addr.dest_ip_addr), &(p_db_ptp_port->port_addr.dest_ip_addr), sizeof(addr_ipv4_t)))
            {
                /*delete arp*/
                ae_p = arp_db_entry_lookup(&(p_db_ptp_port->port_addr.dest_ip_addr));
                if (ae_p == NULL)
                {
                    return PM_E_NONE;
                }
                if (ae_p->route_ref)
                {
                    ae_p->route_ref--;
                    log_sys(M_MOD_ARP, E_INFORMATIONAL, "Delete arp ip %s, ref %d, by ptp port addr sync\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &(p_db_ptp_port->port_addr.dest_ip_addr)), ae_p->route_ref);
#ifdef TODO     /* modified by huangxt for bug 14539, 2011-04-02 */
                    if (ae_p->route_ref == 0 && FALSE == ae_p->ae_bfd_ref)
#else
                    if (ae_p->route_ref == 0)
#endif /*_CENTEC_*/
                    {
                        if (ae_p->status == AS_INCOMPLETE)
                        {
                            ae_p->status = AS_INIT;
                            
                            /* Modified by Jeffrey for bug 4391 use boot time as update time */
                            /* ae_p->ae_uptime = pal_time_current(NULL); */
                            ae_p->uptime = ctc_time_boottime_sec(NULL);
                            arp_sm(ae_p, AE_DELETE_ENTRY);
                        }
                    }
                }
            }
            #if 0
            else
            {
                /*add arp*/
                ae_p = arp_db_entry_lookup(&(p_ptp_port->port_addr.dest_ip_addr));
                if (ae_p != NULL)
                {
                    ae_p->route_ref++; /*modified by ychen for bug 7351 for invalid read*/
                }
                else
                {
                    p_db_if = tbl_interface_get_interface_by_ifindex(p_ptp_port->ifindex);
                    rc = arp_db_entry_create(p_db_if, &(p_ptp_port->port_addr.dest_ip_addr), NULL, ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
                    if (rc < 0)
                    {
                        return PM_E_NONE;
                    }
                    
                    ae_p->route_ref++;
                    ae_p->status = AS_INCOMPLETE;
                    arp_sm(ae_p, AE_RETRY_EXP);
                }
            }
            #endif
            break;
        default:
            return PM_E_INVALID_PARAM;
        }
        break;

    case CDB_OPER_DEL:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

static int32
_arp_process_sync_ptp_port_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_ptp_port_t *p_ptp_port = NULL;
    tbl_arp_t       *ae_p = NULL;
    char buf[CMD_BUF_256];
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        
        p_ptp_port = (tbl_ptp_port_t*)p_tbl;
        switch (field_id)
        {
        case TBL_PTP_PORT_FLD_PORT_ADDR:
            if (p_ptp_port->port_addr.dest_ip_addr.s_addr != 0 && p_ptp_port->port_addr.protocol != PTP_NET_PROTO_IEEE_802_3)
            {
                /*add arp*/
                ae_p = arp_db_entry_lookup(&(p_ptp_port->port_addr.dest_ip_addr));
                if (ae_p != NULL)
                {
                    ae_p->route_ref++; /*modified by ychen for bug 7351 for invalid read*/
                }
                else
                {
                    p_db_if = tbl_interface_get_interface_by_ifindex(p_ptp_port->ifindex);
                    rc = arp_db_entry_create(p_db_if, &(p_ptp_port->port_addr.dest_ip_addr), NULL, ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
                    if (rc < 0)
                    {
                        return PM_E_NONE;
                    }
                    
                    ae_p->route_ref++;
                    ae_p->status = AS_INCOMPLETE;
                    arp_sm(ae_p, AE_RETRY_EXP);
                }
                log_sys(M_MOD_ARP, E_INFORMATIONAL, "Add arp ip %s, ref %d, by ptp port addr sync\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &(p_ptp_port->port_addr.dest_ip_addr)), ae_p->route_ref);
            }
            break;
        default:
            return PM_E_INVALID_PARAM;
        }
        break;

    case CDB_OPER_DEL:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}

int32
rt_sync_init()
{
    cdb_pm_subscribe_tbl(TBL_INTERFACE, _rt_interface_sync_before_cb, _rt_interface_sync_after_cb);
    cdb_pm_subscribe_tbl(TBL_SYSLOG_CFG, _rt_syslog_cfg_sync_cb, NULL);
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, _rt_process_sys_global_cb, NULL);
    cdb_pm_subscribe_tbl(TBL_PTP_PORT, _arp_process_sync_ptp_port_before_cb, _arp_process_sync_ptp_port_after_cb);

    tbl_route_global_t  *p_db_rt_glb      = tbl_route_global_get_route_global();
    if (NULL != p_db_rt_glb)
    {
        if (!p_db_rt_glb->gratuitous_arp_learn_en)
        {
            p_db_rt_glb->gratuitous_arp_learn_en = 1;
            tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN);
        }

        #ifdef GREATBELT
            p_db_rt_glb->ip_redirects_global_en = 0;
        #else
            p_db_rt_glb->ip_redirects_global_en = 1;
        #endif
        tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN);
    }

    return PM_E_NONE;
}

#endif /* CONFIG_IPV4 */

