
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "mlag.h"
#include "mlag_api.h"
#include "mlag_fsm.h"
#include "mlag_sock.h"
#include "mlag_encode.h"
#include "errdisable.h"
#include "if_agg.h"
#include "brg_if.h"
#include "hsrv_msg.h"
#ifdef CONFIG_MSTP
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_instance_define.h"
#include "mstp_define.h"
#include "mstp_api.h"
#endif /* !CONFIG_MSTP */

int32
mlag_api_update_protect_enable(uint32 mlag_id, uint32 protect_en);

#define __MLAG_DESC_API__

char*
mlag_msg_type_str(uint8 type)
{
    switch (type)
    {
    case MLAG_MSG_OPEN:
        return "open";
    case MLAG_MSG_KEEPALIVE:
        return "keepalive";
    case MLAG_MSG_FDBSYNC:
        return "fdb-sync";
    case MLAG_MSG_FAILOVER:
        return "fail-over";
    case MLAG_MSG_CONF:
        return "conf";
    case MLAG_MSG_STP:
        return "stp";
    case MLAG_MSG_SYSPRI:
        return "syspri";
    case MLAG_MSG_PEER_FDB:
        return "peer-fdb";
    case MLAG_MSG_DISCONNECT:
        return "disconnect";
    default:
        return "invalid";
    }
}

#define __MLAG_IS_API__

/* check none of local and remote mlag_port is configured */
uint32
mlag_is_peer_conf_none(tbl_mlag_port_t *p_db_port)
{
    if (p_db_port->ifindex || p_db_port->peer_conf)
    {
        return FALSE;
    }
    
    return TRUE;
}

uint32
mlag_is_peer_addr_valid(tbl_mlag_peer_t *p_peer)
{    
    if (AF_INET == p_peer->peer_addr.family)
    {
        return TRUE;
    }
    else if (AF_INET6 == p_peer->peer_addr.family)
    {
        return TRUE;
    }

    return FALSE;
}

/* both peer address and link is ready */
uint32
mlag_is_peer_ready()
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    uint32 address_ready = FALSE;
    uint32 link_ready = FALSE;
    
    if (mlag->peer_link_if)
    {
        link_ready = TRUE;
    }
    
    if (AF_INET == peer->peer_addr.family
     || AF_INET6 == peer->peer_addr.family)
    {
        address_ready = TRUE;
    }

    if (link_ready && address_ready)
    {
        return TRUE;
    }

    return FALSE;
}


#define __MLAG_CMD_USED_API__

static int32
_mlag_init_default()
{
    tbl_lag_global_t* p_lag_glb = tbl_lag_global_get_lag_global();
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_mlag_t mlag;
    tbl_mlag_t *p_mlag = &mlag;
    tbl_mlag_peer_t peer;
    tbl_mlag_peer_t *p_peer = &peer;

    sal_memset(p_mlag, 0, sizeof(tbl_mlag_t));
    sal_memset(p_peer, 0, sizeof(tbl_mlag_peer_t));
    
    sal_memcpy(p_mlag->local_sysid, p_sys_glb->route_mac, MAC_ADDR_LEN);
    sal_memcpy(p_mlag->mlag_sysid, p_sys_glb->route_mac, MAC_ADDR_LEN);
    p_mlag->local_syspri            = p_lag_glb->system_priority;
    p_mlag->mlag_syspri             = p_lag_glb->system_priority;
    p_mlag->reload_delay_auto       = TRUE;
    p_mlag->reload_delay_interval   = MLAG_DEFAULT_RELOAD_DELAY;
    p_mlag->role                    = MLAG_ROLE_MASTER;

    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_LOCAL_SYSID);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_MLAG_SYSID);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_LOCAL_SYSPRI);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_MLAG_SYSPRI);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_RELOAD_DELAY_AUTO);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_ROLE);

    p_peer->holdtime = MLAG_DEFAULT_HOLDTIME;
    p_peer->keepalive = MLAG_DEFAULT_KEEPALIVE;
    p_peer->mpf_state = MPF_STATE_IDLE;
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_HOLDTIME);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_KEEPALIVE);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_MPF_STATE);
    
    return PM_E_NONE;
}

static int32
_mlag_init_peer_default()
{
    tbl_mlag_peer_t peer;
    tbl_mlag_peer_t *p_peer = &peer;

    sal_memset(p_peer, 0, sizeof(tbl_mlag_peer_t));

    p_peer->v_holdtime = p_peer->holdtime;
    p_peer->v_keepalive = p_peer->keepalive;
    p_peer->v_connect = MLAG_DEFAULT_CONNECT_RETRY;
    p_peer->mpf_state = MPF_STATE_IDLE;
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_V_HOLDTIME);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_V_KEEPALIVE);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_V_CONNECT);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_MPF_STATE);
    
    return PM_E_NONE;
}

static int32
_mlag_api_init_local_fsm()
{
    mpn_sock_client_init();

    mpf_process_event(MPF_EVENT_MANUAL_START);
    
    return PM_E_NONE;
}

static int32
_mlag_api_deinit_local_fsm()
{
    tbl_mlag_peer_t *p_peer = tbl_mlag_peer_get_mlag_peer();

    /* send disconnect to peer */
    mlag_send_disconnect(p_peer);
    sal_task_sleep(100);

    mpn_sock_client_deinit();

    mpf_process_event(MPF_EVENT_MANUAL_STOP);
    
    return PM_E_NONE;
}

static int32
_mlag_api_set_peer_link(tbl_interface_t *p_db_if, uint32 is_set)
{
    tbl_interface_t *p_db_if_mem = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    ds_brgif_t *p_db_brgif_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;

    if (NULL == p_db_if->brgif)
    {
        return PM_E_INVALID_PARAM;
    }
    p_db_brgif = p_db_if->brgif;
    p_db_brgif->mlag_is_peer_link = is_set;
    ds_brgif_set_brgif_field(p_db_if, p_db_brgif, DS_BRGIF_FLD_MLAG_IS_PEER_LINK);
    brg_if_update_oper_mac_learning_en(p_db_if);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_if_mem, listnode)
        {
            p_db_brgif_mem = p_db_if_mem->brgif;
            if (p_db_brgif_mem)
            {
                p_db_brgif_mem->mlag_is_peer_link = is_set;
                ds_brgif_set_brgif_field(p_db_if_mem, p_db_brgif_mem, DS_BRGIF_FLD_MLAG_IS_PEER_LINK);
                brg_if_update_oper_mac_learning_en(p_db_if_mem);
            }
        }
    }

    return PM_E_NONE;
}

static int32
_mlag_api_set_mlag_is_group(tbl_interface_t *p_db_if, uint32 is_group)
{
    ds_brgif_t *p_db_brgif = NULL;
    
    if (NULL == p_db_if->brgif)
    {
        return PM_E_INVALID_PARAM;
    }
    p_db_brgif = p_db_if->brgif;
    p_db_brgif->mlag_is_group = is_group;
    ds_brgif_set_brgif_field(p_db_if, p_db_brgif, DS_BRGIF_FLD_MLAG_IS_GROUP);

    return PM_E_NONE;
}

static int32
_mlag_api_set_mlag_port_block(tbl_interface_t *p_db_if, uint32 is_block)
{
    tbl_interface_t *p_db_if_mem = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    ds_brgif_t *p_db_brgif_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    if (NULL == p_db_if->brgif)
    {
        return PM_E_INVALID_PARAM;
    }
    p_db_brgif = p_db_if->brgif;
    p_db_brgif->mlag_is_port_block = is_block;
    ds_brgif_set_brgif_field(p_db_if, p_db_brgif, DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_if_mem, listnode)
        {
            p_db_brgif_mem = p_db_if_mem->brgif;
            if (p_db_brgif_mem)
            {
                p_db_brgif_mem->mlag_is_port_block = is_block;
                ds_brgif_set_brgif_field(p_db_if_mem, p_db_brgif_mem, DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK);
            }
       }
    }

    return PM_E_NONE;
}

static void
_mlag_api_reload_delay_recover(void* p_data)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;

    p_db_mlag->reload_delay_timer = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            continue;
        }
        if (p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
        {
            continue;
        }
        if (!tbl_interface_is_up(p_db_if) && !errdisable_if_is_errdisable(p_db_if))
        {
            continue;
        }

        errdisable_handle_exception(ERRDIS_MLAG_RELOAD_DELAY, p_db_if, FALSE);
    }

    return;
}

int32
mlag_api_established_reload_recovery()
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    if (!p_db_mlag->reload_delay_auto)
    {
        return PM_E_NONE;
    }

    if (NULL == p_db_mlag->reload_delay_timer)
    {
        return PM_E_NONE;
    }

    /* delete reload-delay timer */
    CTC_TASK_STOP_TIMER(p_db_mlag->reload_delay_timer);

    /* wait 1 seconds for all config is synced */
    ctc_task_add_timer(TASK_PRI_NORMAL, _mlag_api_reload_delay_recover, NULL, 1);

    return PM_E_NONE;
}

int32
mlag_api_set_reload_delay_interval(tbl_mlag_t *p_mlag)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_RELOAD_DELAY_AUTO);

    if (p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }

    /* update reload-delay */
    CTC_TASK_STOP_TIMER(p_db_mlag->reload_delay_timer);
    p_db_mlag->reload_delay_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mlag_api_reload_delay_recover, 
        NULL, p_mlag->reload_delay_interval);

    return PM_E_NONE;
}

static int32
_mlag_api_reload_delay_process(uint32 interval)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    if (p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            continue;
        }
        if (p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
        {
            continue;
        }
        if (GLB_FLAG_ISSET(p_db_if->flags, GLB_IFF_CFG_SHUTDOWN))
        {
            continue;
        }  

        errdisable_handle_exception(ERRDIS_MLAG_RELOAD_DELAY, p_db_if, TRUE);
    }
    
    CTC_TASK_STOP_TIMER(p_db_mlag->reload_delay_timer);
    p_db_mlag->reload_delay_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _mlag_api_reload_delay_recover, 
        NULL, interval);

    return PM_E_NONE;
}

int32
mlag_api_create(tbl_mlag_t *p_mlag)
{
    _mlag_init_default();
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_ENABLE);

#ifdef CONFIG_MSTP
    mstp_api_set_mlag_enable(TRUE);
#endif /* !CONFIG_MSTP */

    _mlag_api_reload_delay_process(MLAG_DEFAULT_RELOAD_DELAY);

    return PM_E_NONE;
}

int32
mlag_api_destory()
{
    tbl_mlag_t      mlag;
    tbl_mlag_peer_t peer;
    tbl_mlag_t      *p_mlag = &mlag;
    tbl_mlag_peer_t *p_peer = &peer;
    
#ifdef CONFIG_MSTP
    mstp_api_set_mlag_enable(FALSE);
#endif /* !CONFIG_MSTP */

    /* destory peer */
    mlag_api_destory_peer_link();
    mlag_api_destory_peer();

    sal_memset(p_mlag, 0, sizeof(tbl_mlag_t));

    /* unset values */
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_LOCAL_SYSID);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_MLAG_SYSID);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_LOCAL_SYSPRI);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_MLAG_SYSPRI);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_ROLE);
    tbl_mlag_set_mlag_field(p_mlag, TBL_MLAG_FLD_ENABLE);
    
    p_peer->holdtime = MLAG_DEFAULT_HOLDTIME;
    p_peer->keepalive = MLAG_DEFAULT_KEEPALIVE;
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_HOLDTIME);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_KEEPALIVE);

    return PM_E_NONE;
}

int32
mlag_api_create_peer(tbl_mlag_peer_t *p_peer)
{
    _mlag_init_peer_default();
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_PEER_ADDR);

    mpn_sock_server_init();

    if (mlag_is_peer_ready())
    {
        _mlag_api_init_local_fsm();
    }

    return PM_E_NONE;
}

int32
mlag_api_destory_peer()
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    uint32 old_ready = FALSE;
    uint32 new_ready = FALSE;

    if (!mlag_is_peer_addr_valid(peer))
    {
        return PM_E_NONE;
    }
    old_ready = mlag_is_peer_ready();
    
    sal_memset(&peer->peer_addr, 0, sizeof(peer->peer_addr));
    tbl_mlag_peer_set_mlag_peer_field(peer, TBL_MLAG_PEER_FLD_PEER_ADDR);

    new_ready = mlag_is_peer_ready();
    if (old_ready != new_ready)
    {
        _mlag_api_deinit_local_fsm();
    }
    
    mpn_sock_server_deinit();
    
    /* destory remote peer session */
    mlag_api_clear_counters();

    return PM_E_NONE;
}

static int32
_mlag_api_clear_peer_link_errdisable(tbl_interface_t *p_db_if)
{
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if_mem = NULL;
    
    if (errdisable_if_is_errdisable(p_db_if))
    {
        errdisable_handle_exception(ERRDIS_MLAG_RELOAD_DELAY, p_db_if, FALSE);
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_if_mem, listnode)
        {
            if (errdisable_if_is_errdisable(p_db_if_mem))
            {
                errdisable_handle_exception(ERRDIS_MLAG_RELOAD_DELAY, p_db_if_mem, FALSE);
            }
        }
    }
    
    return PM_E_NONE;
}

int32
mlag_api_create_peer_link(tbl_interface_t *p_db_if)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mlag_port_t *p_db_mlag_port = NULL;

    p_db_mlag->peer_link_if = p_db_if;
    p_db_mlag->peer_link_ifindex = p_db_if->ifindex;
    tbl_mlag_set_mlag_field(p_db_mlag, TBL_MLAG_FLD_PEER_LINK_IFINDEX);

    _mlag_api_clear_peer_link_errdisable(p_db_if);
    _mlag_api_set_peer_link(p_db_if, TRUE);

    if (mlag_is_peer_ready())
    {
        _mlag_api_init_local_fsm();
    }

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_mlag_port, listnode)
    {
        mlag_api_update_mlag_is_group(p_db_mlag_port);
    }
    
    return PM_E_NONE;
}

int32
mlag_api_destory_peer_link()
{
    tbl_interface_t *p_db_if = NULL;
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mlag_port_t *p_db_mlag_port = NULL;
    uint32 old_ready = FALSE;
    uint32 new_ready = FALSE;
    
    if (NULL == p_db_mlag->peer_link_if)
    {
        return PM_E_NONE;
    }
    
    old_ready = mlag_is_peer_ready();

    p_db_if = (tbl_interface_t*)p_db_mlag->peer_link_if;
    _mlag_api_set_peer_link(p_db_if, FALSE);
    
    p_db_mlag->peer_link_if = NULL;
    p_db_mlag->peer_link_ifindex = 0;
    tbl_mlag_set_mlag_field(p_db_mlag, TBL_MLAG_FLD_PEER_LINK_IFINDEX);

    new_ready = mlag_is_peer_ready();
    if (old_ready != new_ready)
    {
        _mlag_api_deinit_local_fsm();
    }

    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_mlag_port, listnode)
    {
        mlag_api_update_mlag_is_group(p_db_mlag_port);
    }

    return PM_E_NONE;
}

int32
mlag_api_add_mlag_port(tbl_interface_t *p_db_if, uint32 mlag_id)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    tbl_mlag_port_t port;
    tbl_mlag_port_t *p_db_port = NULL;
    uint32 is_up = FALSE;

    sal_memset(&port, 0, sizeof(port));
    port.key.id = mlag_id;

    p_db_port = tbl_mlag_port_get_mlag_port(&port.key);
    if (NULL == p_db_port)
    {
        tbl_mlag_port_add_mlag_port(&port);
        p_db_port = tbl_mlag_port_get_mlag_port(&port.key);
        if (NULL == p_db_port)
        {
            return PM_E_NO_MEMORY;        
        }
    }

    p_db_if->mlag_id = mlag_id;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MLAG_ID);
    p_db_port->ifindex = p_db_if->ifindex;
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_IFINDEX);
    p_db_port->p_if = p_db_if;
    lacp_set_mlag_id(p_db_if, mlag_id);

    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        is_up = tbl_interface_is_running(p_db_if);
        mlag_send_conf(peer, mlag_id, TRUE, is_up);
    }

    /* Added by kcao for bug 41301 2016-10-25, should set protect enable default status */
    if (tbl_interface_is_running(p_db_if))
    {
        mlag_api_update_protect_enable(mlag_id, FALSE);
    }
    else
    {
        mlag_api_update_protect_enable(mlag_id, TRUE);
    }
    
    /* update mlag_port is_group */
    mlag_api_update_mlag_is_group(p_db_port);
    
    /* update mlag_port port isolation state */
    mlag_api_update_mlag_port_block(p_db_port);

    return PM_E_NONE;
}

int32
mlag_api_del_mlag_port(tbl_interface_t *p_db_if)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    tbl_mlag_port_key_t port_key;
    tbl_mlag_port_t *p_db_port = NULL;
    uint32 is_up = FALSE;

    port_key.id = p_db_if->mlag_id;

    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        is_up = tbl_interface_is_running(p_db_if);
        mlag_send_conf(peer, p_db_if->mlag_id, FALSE, is_up);
    }

    p_db_port = tbl_mlag_port_get_mlag_port(&port_key);
    if (NULL == p_db_port || NULL == p_db_if->brgif)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (p_db_if->brgif->mlag_is_port_block)
    {
        /* clear mlag_port port isolation block state */
        _mlag_api_set_mlag_port_block(p_db_if, FALSE);
    }
    p_db_port->ifindex = GLB_INVALID_IFINDEX;
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_IFINDEX);
    p_db_port->p_if = NULL;

    if (p_db_if->brgif->mlag_is_group)
    {
        _mlag_api_set_mlag_is_group(p_db_if, FALSE);
    }

    if (mlag_is_peer_conf_none(p_db_port))
    {
        /* local - local -> none */
        tbl_mlag_port_del_mlag_port(&p_db_port->key);
    }
    else
    {
        /* both - local -> remote */
    }

    lacp_set_mlag_id(p_db_if, MLAG_ID_INVALID);
    p_db_if->mlag_id = MLAG_ID_INVALID;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MLAG_ID);

    return PM_E_NONE;
}

int32
mlag_api_set_peer_timers(tbl_mlag_peer_t *p_peer)
{
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_SET_TIMERS);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_HOLDTIME);
    tbl_mlag_peer_set_mlag_peer_field(p_peer, TBL_MLAG_PEER_FLD_KEEPALIVE);

    return PM_E_NONE;
}

int32
mlag_api_clear_counters()
{
    tbl_mlag_peer_t *p_peer = tbl_mlag_peer_get_mlag_peer();
    
    p_peer->open_in = 0;
    p_peer->open_out = 0;
    p_peer->keepalive_in = 0;
    p_peer->keepalive_out = 0;
    p_peer->disconnect_in = 0;
    p_peer->disconnect_out = 0;
    p_peer->fdbsync_in = 0;
    p_peer->fdbsync_out = 0;
    p_peer->failover_in = 0;
    p_peer->failover_out = 0;
    p_peer->conf_in = 0;
    p_peer->conf_out = 0;
    p_peer->syspri_in = 0;
    p_peer->syspri_out = 0;
    p_peer->stp_in = 0;
    p_peer->stp_out = 0;
    p_peer->stp_global_in = 0;
    p_peer->stp_global_out = 0;
    p_peer->stp_packet_in = 0;
    p_peer->stp_packet_out = 0;
    p_peer->stp_instance_in = 0;
    p_peer->stp_instance_out = 0;
    p_peer->stp_state_in = 0;
    p_peer->stp_state_out = 0;
    p_peer->peer_fdb_in = 0;
    p_peer->peer_fdb_out = 0;

    return PM_E_NONE;
}

int32
mlag_api_update_protect_enable(uint32 mlag_id, uint32 protect_en)
{
    tbl_mlag_port_key_t key;
    uint32 old_protect_en = FALSE;
    tbl_interface_t *p_db_if = NULL;
    tbl_mlag_port_t *p_db_mlag = NULL;

    
    key.id = mlag_id;
    p_db_mlag = tbl_mlag_port_get_mlag_port(&key);
    if (NULL == p_db_mlag)
    {
        return PM_E_NOT_EXIST;
    }
    p_db_if = (tbl_interface_t*)p_db_mlag->p_if;
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    old_protect_en = p_db_mlag->protect_en;

    if (old_protect_en == protect_en)
    {
        return PM_E_NONE;
    }

    p_db_mlag->protect_en = protect_en;
    tbl_mlag_port_set_mlag_port_field(p_db_mlag, TBL_MLAG_PORT_FLD_PROTECT_EN);

    return PM_E_NONE;
}

#define __MLAG_FSM_PACKET_USED_API__

int32
mlag_api_update_mlag_is_group(tbl_mlag_port_t *p_db_port)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_interface_t *p_db_if = NULL;
    uint32 old_is_group = FALSE;
    uint32 new_is_group = FALSE;

    p_db_if = (tbl_interface_t*)p_db_port->p_if;
    if (NULL == p_db_if || NULL == p_db_if->brgif)
    {
        return PM_E_NOT_EXIST;
    }

    old_is_group = p_db_if->brgif->mlag_is_group;

    if (p_db_mlag->peer_link_if && p_db_if->mlag_id)
    {
        new_is_group = TRUE;
    }
    else
    {
        new_is_group = FALSE;
    }

    if (new_is_group != old_is_group)
    {
        _mlag_api_set_mlag_is_group(p_db_if, new_is_group);
    }
    
    return PM_E_NONE;
}

int32
mlag_api_update_mlag_port_block(tbl_mlag_port_t *p_db_port)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 old_block = FALSE;
    uint32 new_block = FALSE;

    p_db_if = (tbl_interface_t*)p_db_port->p_if;
    if (NULL == p_db_if || NULL == p_db_if->brgif)
    {
        return PM_E_NOT_EXIST;
    }

    old_block = p_db_if->brgif->mlag_is_port_block;

    if (p_db_port->peer_conf && p_db_port->peer_if_up)
    {
        new_block = TRUE;
    }
    else
    {
        new_block = FALSE;
    }

    if (new_block != old_block)
    {
        _mlag_api_set_mlag_port_block(p_db_if, new_block);
    }
    
    return PM_E_NONE;
}

int32
mlag_api_sync_peer_conf_all_ports(tbl_mlag_peer_t *peer)
{
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mlag_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 is_up = FALSE;

    CTCLIB_SLIST_LOOP_DEL(p_master->port_list, p_db_port, listnode, next)
    {
        if (p_db_port->p_if)
        {
            p_db_if = (tbl_interface_t*)p_db_port->p_if;
            is_up = tbl_interface_is_running(p_db_if);
            mlag_send_conf(peer, p_db_port->key.id, TRUE, is_up);
        }
    }
    
    return PM_E_NONE;
}

int32
mlag_api_clear_peer_conf()
{
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_mlag_port_t *p_db_port = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->port_list, p_db_port, listnode, next)
    {
        mlag_api_port_peer_unconf(p_db_port->key.id);
    }

    return PM_E_NONE;
}

/*bug 30229. huangxt, 2014-11-04*/
int32
mlag_api_flush_fdb_sync_in()
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();

    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_FLUSH_FDB_MI);

    return PM_E_NONE;
}

/* used for holdtime timeout */
int32
mlag_api_restart_socket(tbl_mlag_peer_t *peer)
{
    mpn_sock_server_deinit();
    mpn_sock_server_init();

    if (mlag_is_peer_ready())
    {
        mpn_sock_client_deinit();
        mpn_sock_client_init();
        mpn_sock_client_connect();
    }

    return PM_E_NONE;
}

int32
mlag_api_peer_session_close(tbl_mlag_peer_t *peer)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    
    /* clear mlag */
    mlag_api_peer_fdb_flush();

    sal_memset(mlag->peer_routemac, 0, MAC_ADDR_LEN);
    sal_memset(mlag->remote_sysid, 0, MAC_ADDR_LEN);
    mlag->remote_syspri = 0;
    mlag->role = MLAG_ROLE_MASTER;
    
    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_PEER_ROUTEMAC);
    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_REMOTE_SYSID);
    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_REMOTE_SYSPRI);
    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_ROLE);

    /* clear mlag peer */
    peer->uptime = 0;
    peer->v_auto_start = 0;
    peer->v_connect = MLAG_DEFAULT_CONNECT_RETRY;
    peer->v_keepalive = peer->keepalive;
    peer->v_holdtime = peer->holdtime;    
    
    CTC_TASK_STOP_TIMER(peer->t_auto_start);
    CTC_TASK_STOP_TIMER(peer->t_holdtime);
    CTC_TASK_STOP_TIMER(peer->t_keepalive);

    mlag_api_role_select();
    mlag_api_syspri_select();

    if (mlag_is_peer_ready())
    {
        /* local peer config not removed, but remote peer connect fail */
        mpf_change_state(peer, MPF_STATE_ACTIVE);
    }
    else
    {
        /* local peer config is removed */
        mpf_change_state(peer, MPF_STATE_IDLE);
        if (!ctc_sock_is_connecting(peer->client_sock))
        {
            /* client sock connect is stop, reconnect */
            mpn_sock_client_connect();
        }
    }

    return PM_E_NONE;
}

int32
mlag_api_sync_peer_fdb(tbl_mlag_peer_t *peer)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    vid_t vid = 0;
    vid_t vlan_array[256];
    uint16 cnt = 0;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
            if (tbl_interface_is_running(p_db_if))
            {
                vid = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
                vlan_array[cnt++] = vid;
            }
        }
    }

    if (cnt)
    {
        rc = mlag_send_peer_fdb(peer, MLAG_MSG_PEER_FDB_OP_SYNC, vlan_array, cnt);
    }
    
    return rc;
}

int32
mlag_api_peer_fdb_flush()
{
    tbl_mlag_t tmp_mlag;
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();

    GLB_BMP_INIT(tmp_mlag.peer_vlanif_bmp);
    GLB_BMP_INIT(tmp_mlag.peer_fdb_bmp);

    if (0 != sal_memcmp(tmp_mlag.peer_vlanif_bmp, mlag->peer_vlanif_bmp, sizeof(vlan_bmp_t)))
    {
        tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_VLANIF_BMP);
    }

    if (0 != sal_memcmp(tmp_mlag.peer_fdb_bmp, mlag->peer_fdb_bmp, sizeof(vlan_bmp_t)))
    {
        tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
    }

    return PM_E_NONE;
}

int32
mlag_api_role_select()
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();

    if (sal_memcmp(mlag->local_sysid, mlag->remote_sysid, MAC_ADDR_LEN) >= 0)
    {
        mlag->role = MLAG_ROLE_MASTER;
        tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_ROLE);
        sal_memcpy(mlag->mlag_sysid, &mlag->local_sysid, MAC_ADDR_LEN);
    }
    else
    {
        mlag->role = MLAG_ROLE_SLAVE;
        tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_ROLE);
        sal_memcpy(&mlag->mlag_sysid, &mlag->remote_sysid, MAC_ADDR_LEN);
    }

#ifdef CONFIG_MSTP
    mstp_api_set_mlag_role(mlag->role);
#endif /* !CONFIG_MSTP */
    lacp_set_mlag_sysid(mlag->mlag_sysid);

#ifdef MLAG_TODO
    /*sync port state*/
    if (MLAG_ROLE_MASTER == mlag->role)
    {
        MLAG_TIMER_ON(nzg, mlag->t_stp_sync, mlag, mlag_stp_sync_state, MLAG_STP_SYNC_TIMER);
    }
#endif /* !MLAG_TODO */

    return PM_E_NONE;
}

int32
mlag_api_syspri_select()
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();

    if (MLAG_ROLE_SLAVE == mlag->role)
    {
        mlag->mlag_syspri = mlag->remote_syspri;
    }
    else
    {
        mlag->mlag_syspri = mlag->local_syspri;
    }

    lacp_set_mlag_syspri(mlag->mlag_syspri);
    
    return PM_E_NONE;
}

int32
mlag_api_port_peer_conf(uint32 mlag_id, uint8 is_up)
{
    tbl_mlag_port_t port;
    tbl_mlag_port_t *p_db_port = NULL;

    sal_memset(&port, 0, sizeof(port));
    port.key.id = mlag_id;
    p_db_port = tbl_mlag_port_get_mlag_port(&port.key);
    if (NULL == p_db_port)
    {
        tbl_mlag_port_add_mlag_port(&port);
        p_db_port = tbl_mlag_port_get_mlag_port(&port.key);
        if (NULL == p_db_port)
        {
            return PM_E_NO_MEMORY;        
        }
    }

    p_db_port->peer_conf = TRUE;
    p_db_port->peer_if_up = is_up;
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_PEER_CONF);
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_PEER_IF_UP);

    /* update mlag_port port isolation state */
    mlag_api_update_mlag_port_block(p_db_port);

    return PM_E_NONE;
}

int32
mlag_api_port_peer_unconf(uint32 mlag_id)
{
    tbl_mlag_port_t port;
    tbl_mlag_port_t *p_db_port = NULL;

    sal_memset(&port, 0, sizeof(port));
    port.key.id = mlag_id;
    p_db_port = tbl_mlag_port_get_mlag_port(&port.key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }
    
    p_db_port->peer_conf = FALSE;
    p_db_port->peer_if_up = FALSE;
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_PEER_CONF);
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_PEER_IF_UP);
        
    /* update mlag_port port isolation state */
    mlag_api_update_mlag_port_block(p_db_port);

    if (mlag_is_peer_conf_none(p_db_port))
    {
        /* remote - remote -> none */
        tbl_mlag_port_del_mlag_port(&p_db_port->key);
    }
    else
    {
        /* both - remote -> local */
    }
    
    return PM_E_NONE;
}

int32
mlag_api_peer_failover(uint32 mlag_id, uint8 is_recover)
{
    tbl_mlag_port_key_t key;
    tbl_mlag_port_t *p_db_port = NULL;

    key.id = mlag_id;
    p_db_port = tbl_mlag_port_get_mlag_port(&key);
    if (NULL == p_db_port)
    {
        return PM_E_NOT_EXIST;
    }

    p_db_port->peer_if_up = is_recover;
    tbl_mlag_port_set_mlag_port_field(p_db_port, TBL_MLAG_PORT_FLD_PEER_IF_UP);

    /* update mlag_port port isolation state */
    mlag_api_update_mlag_port_block(p_db_port);

    return PM_E_NONE;
}

int32
mlag_api_get_reload_delay_remain_sec()
{
    int32 remain_sec = 0;
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    if (p_db_mlag->reload_delay_timer)
    {
        remain_sec = ctc_timer_get_remain_sec(p_db_mlag->reload_delay_timer);
    }
    return remain_sec;
}

#define __MLAG_SWITCH_API_CALLBACK__

/* port join agg will effect ingress interface */
static int32
_mlag_agg_add_member_cb(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if (p_db_agg_if->brgif && p_db_mem_if->brgif)
    {
        if (p_db_agg_if->brgif->mlag_is_peer_link)
        {
            p_db_mem_if->brgif->mlag_is_peer_link = p_db_agg_if->brgif->mlag_is_peer_link;
            ds_brgif_set_brgif_field(p_db_mem_if, p_db_mem_if->brgif, DS_BRGIF_FLD_MLAG_IS_PEER_LINK);
            brg_if_update_oper_mac_learning_en(p_db_mem_if);
        }

        if (p_db_agg_if->brgif->mlag_is_port_block)
        {
            p_db_mem_if->brgif->mlag_is_port_block = p_db_agg_if->brgif->mlag_is_port_block;
            ds_brgif_set_brgif_field(p_db_mem_if, p_db_mem_if->brgif, DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK);
        }
    }

    return PM_E_NONE;
}

/* port join agg will effect ingress interface */
static int32
_mlag_agg_del_member_cb(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if (p_db_agg_if->brgif && p_db_mem_if->brgif)
    {
        if (p_db_agg_if->brgif->mlag_is_peer_link)
        {
            p_db_mem_if->brgif->mlag_is_peer_link = FALSE;
            ds_brgif_set_brgif_field(p_db_mem_if, p_db_mem_if->brgif, DS_BRGIF_FLD_MLAG_IS_PEER_LINK);
            brg_if_update_oper_mac_learning_en(p_db_mem_if);
        }

        if (p_db_agg_if->brgif->mlag_is_port_block)
        {
            p_db_mem_if->brgif->mlag_is_port_block = FALSE;
            ds_brgif_set_brgif_field(p_db_mem_if, p_db_mem_if->brgif, DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK);
        }    
    }
    
    return PM_E_NONE;
}

static int32
_mlag_api_vlanif_up_cb(tbl_interface_t *p_db_if)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    vid_t vid_array[1];
    int32 rc = PM_E_NONE;

    if (GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    vid_array[0] = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
    rc = mlag_send_peer_fdb(peer, MLAG_MSG_PEER_FDB_OP_ADD, vid_array, 1);

    return rc;
}

static int32
_mlag_api_vlanif_down_cb(tbl_interface_t *p_db_if)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    vid_t vid_array[1];
    int32 rc = PM_E_NONE;

    if (GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    vid_array[0] = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
    rc = mlag_send_peer_fdb(peer, MLAG_MSG_PEER_FDB_OP_DEL, vid_array, 1);

    return rc;
}

static int32
_mlag_api_l2if_up_cb(tbl_interface_t *p_db_if)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    mlag_api_update_protect_enable(p_db_if->mlag_id, FALSE);

    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        mlag_send_failover(peer, p_db_if->mlag_id, TRUE);
    }

    return PM_E_NONE;
}

static int32
_mlag_api_l2if_down_cb(tbl_interface_t *p_db_if)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    mlag_api_update_protect_enable(p_db_if->mlag_id, TRUE);

    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        mlag_send_failover(peer, p_db_if->mlag_id, FALSE);
    }

    return PM_E_NONE;
}

int32
mlag_api_if_del_cb(tbl_interface_t *p_db_if)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();

    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        _mlag_api_vlanif_down_cb(p_db_if);
    }

    if (p_db_if->mlag_id)
    {
        mlag_api_del_mlag_port(p_db_if);
    }
    
    /* Modified by kcao for bug 40939 */
    if (mlag->peer_link_if == p_db_if)
    {
        mlag_api_destory_peer_link();
    }
    
    return PM_E_NONE;
}

int32
mlag_api_if_up_cb(tbl_interface_t *p_db_if)
{
    /* need not mlag peer ready */
    if (GLB_IF_MODE_L2 == p_db_if->mode && GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (MLAG_ID_INVALID != p_db_if->mlag_id)
        {
            _mlag_api_l2if_up_cb(p_db_if);
        }
    }
    
    if (!mlag_is_peer_ready())
    {
        return PM_E_NONE;
    }
    
    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        _mlag_api_vlanif_up_cb(p_db_if);
    }

    return PM_E_NONE;
}

int32
mlag_api_if_down_cb(tbl_interface_t *p_db_if)
{
    /* need not mlag peer ready */
    if (GLB_IF_MODE_L2 == p_db_if->mode && GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (MLAG_ID_INVALID != p_db_if->mlag_id)
        {
            _mlag_api_l2if_down_cb(p_db_if);
        }
    }
    
    if (!mlag_is_peer_ready())
    {
        return PM_E_NONE;
    }
    
    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        _mlag_api_vlanif_down_cb(p_db_if);
    }

    return PM_E_NONE;
}

int32
mlag_api_set_lacp_system_priority_cb(uint16 priority)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();

    mlag->local_syspri = priority;

    if (MLAG_ROLE_MASTER == mlag->role)
    {
        mlag->mlag_syspri = priority;
        lacp_set_mlag_syspri(priority);
    }
    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        mlag_send_syspri(peer);
    }

    return PM_E_NONE;
}

int32
mlag_api_allowed_vlan_change_change_cb(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();

    if (p_db_mlag->peer_link_ifindex != p_db_if->ifindex)
    {
        return PM_E_NONE;
    }

    if (is_add)
    {
        if (GLB_BMP_ISSET(p_db_mlag->peer_vlanif_bmp, vid) && !GLB_BMP_ISSET(p_db_mlag->peer_fdb_bmp, vid))
        {
            GLB_BMP_SET(p_db_mlag->peer_fdb_bmp, vid);
            tbl_mlag_set_mlag_field(p_db_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
        }
    }
    else
    {
        if (GLB_BMP_ISSET(p_db_mlag->peer_vlanif_bmp, vid) && GLB_BMP_ISSET(p_db_mlag->peer_fdb_bmp, vid))
        {
            GLB_BMP_UNSET(p_db_mlag->peer_fdb_bmp, vid);
            tbl_mlag_set_mlag_field(p_db_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
        }
    }

    return PM_E_NONE;
}

#define __MLAG_INIT_DEINIT__

int32
mlag_api_rx_fdbsync_cb(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    glb_mlag_fdb_sync_t *sync = NULL;

    sync = (glb_mlag_fdb_sync_t*)p_msg->data;
    if (MPF_STATE_ESTABLISHED == peer->mpf_state)
    {
        mlag_send_fdbsync(peer, sync);
    }
    
    return PM_E_NONE;
}

int32
mlag_start(switch_master_t *master)
{
#if 0
    nsm_mlag_fdb_sync_pending_list_init();
    
    /* register netlink callback */
    nsm_set_hal_callback(NSM_MLAG_FDB_SYNC, mlag_fdb_sync_cb);

    /*bug 30117. huangxt, 2014-11-03.*/
    nsm_lacp_add_member_sync_prop_set_cb(NSM_LACP_CB_MODULE_MLAG, nsm_mlag_add_agg_member);
    nsm_lacp_del_member_sync_prop_set_cb(NSM_LACP_CB_MODULE_MLAG, nsm_mlag_del_agg_member);
#endif
    if_agg_add_member_sync_prop_set_cb(IF_AGG_CB_MODULE_MLAG,  _mlag_agg_add_member_cb);
    if_agg_del_member_sync_prop_set_cb(IF_AGG_CB_MODULE_MLAG,  _mlag_agg_del_member_cb);
    
    cdb_register_cfg_tbl_cb(TBL_MLAG,       mlag_cmd_process_mlag);
    cdb_register_cfg_tbl_cb(TBL_MLAG_PEER,  mlag_cmd_process_mlag_peer);
    cdb_register_cfg_tbl_cb(TBL_MLAG_PORT,  mlag_cmd_process_mlag_port);
    cdb_register_cfg_act_cb(ACT_SHOW_MLAG,  mlag_cmd_process_show_mlag);
    cdb_register_cfg_act_cb(ACT_CLEAR_MLAG_COUNTERS,  mlag_cmd_process_clear_counters);

    ipc_register_hal_rx_fn(SWITCH_FEI_MLAG_FDB_SYNC, mlag_api_rx_fdbsync_cb);

    return PM_E_NONE;
}

int32
mlag_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_MLAG,       NULL);
    cdb_register_cfg_tbl_cb(TBL_MLAG_PEER,  NULL);
    cdb_register_cfg_tbl_cb(TBL_MLAG_PORT,  NULL);
    cdb_register_cfg_act_cb(ACT_SHOW_MLAG,  NULL);
    cdb_register_cfg_act_cb(ACT_CLEAR_MLAG_COUNTERS,  NULL);

    ipc_register_hal_rx_fn(SWITCH_FEI_MLAG_FDB_SYNC, NULL);

    return PM_E_NONE;
}

