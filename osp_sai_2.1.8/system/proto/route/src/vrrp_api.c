
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_static_rt_cnt_define.h"
#include "gen/tbl_static_rt_cnt.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "sal.h"
#include "sal_types.h"
#include "route.h"
#include "lib_netlink.h"
#include "rt_fea.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_vrrp_vip_define.h"
#include "gen/tbl_vrrp_vip.h"
#include "gen/tbl_vrrp_vmac_define.h"
#include "gen/tbl_vrrp_vmac.h"
#include "vrrp.h" 
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "vrrp.h"
#include "vrrp_api.h"
#include "rt_if.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "arp_api.h"
#include "track.h"
#include "track_api.h"
#include "lib_snmp.h"

bool vrrp_debug_is_on(uint32 flag)
{
    tbl_route_debug_t *p_db_glb = NULL;
    
    p_db_glb = tbl_route_debug_get_route_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->vrrp, flag))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* vrrp_compute_masterdowninterval - computes the vrrp master down interval */
uint32 vrrp_compute_masterdowninterval(tbl_vrrp_session_t *p_db_sess, uint16 advt_interval, uint8 priority) 
{
    uint8   skew_time = 0;
    uint32  master_down_interval = 0;
    
    if(GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
    {
        if (priority > VRRP_SKEW_PRIORITY)
        {    
            skew_time = 0;
        }
        else
        {
            skew_time = 1;
        }
        master_down_interval = 3 * advt_interval + skew_time + p_db_sess->preempt_delay;
    }
    else
    {
        /* advt_int unit is msec, preempt_delay unit is sec. */
        master_down_interval = 3 * advt_interval + p_db_sess->preempt_delay * 1000;  
    }
    
    return master_down_interval;
}

int32 
vrrp_reset_virtual_ip (tbl_vrrp_session_t *p_db_sess)
{
    int32                     rc = PM_E_NONE;
    
    if (p_db_sess)
    {
        sal_memset (&p_db_sess->vip, 0, sizeof(addr_t));
        p_db_sess->vip_status = FALSE;

        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_VIP);
        if (rc < 0)
        {
            VRRP_LOG_ERR("Set vrrp session vip fail");
            return rc;
        }

        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_VIP_STATUS);
        if (rc < 0)
        {
            VRRP_LOG_ERR("Set vrrp session vip status fail");
            return rc;
        }
    }

    return rc;
}

void
vrrp_addr_subnet_match(char *ifname, addr_t *vip, bool *is_owner, bool *is_submatch)
{
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *ifc_tmp = NULL;
    prefix_t            vip_prefix;

    sal_memset(&vip_prefix, 0, sizeof(prefix_t));
    
    if (!ifname || !vip || !is_owner || !is_submatch)
    {
        return;
    }

    sal_memset(&rt_if, 0, sizeof(rt_if));
    sal_memcpy(rt_if.key.name, ifname, sal_strlen(ifname));

    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        return;
    }

    /* check whether vip is ip owner */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
    {
        if (vip->u.prefix4.s_addr == ifc_tmp->key.address.u.prefix4.s_addr)
        {
            if (!GLB_FLAG_ISSET(ifc_tmp->flags, RT_IF_ADDR_VIRTUAL))
            {
                *is_owner = TRUE;
                *is_submatch = TRUE;
                break;
            }
        }
    }

    if (*is_submatch)
    {
        return;    
    }

    /* check whether vip is the same subnet as interface ip */
    vip_prefix.family = vip->family;
    vip_prefix.prefixlen = 32;
    sal_memcpy(&vip_prefix.u, &vip->u, sizeof(addr_u));        
    
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
    {
        if (!GLB_FLAG_ISSET(ifc_tmp->flags, RT_IF_ADDR_VIRTUAL))
        {   
            if (prefix_compare_net(&ifc_tmp->key.address, &vip_prefix))
            {
                 *is_submatch = TRUE;
                 break;
            }
        }
    }

    return;
}

void
vrrp_get_interface_primary_ip(char *ifname, uint32 *primary_ip)
{
    tbl_route_if_t      rt_if;
    tbl_route_if_t      *p_db_rtif  = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *ifc_tmp = NULL;
    
    if (!ifname)
    {
        return;
    }

    sal_memset(&rt_if, 0, sizeof(rt_if));
    sal_memcpy(rt_if.key.name, ifname, sal_strlen(ifname));

    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        return;
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, ifc_tmp, listnode)
    { 
        if (!GLB_FLAG_ISSET(ifc_tmp->flags, RT_IF_ADDR_SECONDARY)
            && !GLB_FLAG_ISSET(ifc_tmp->flags, RT_IF_ADDR_VIRTUAL))
        {
            *primary_ip = ifc_tmp->key.address.u.prefix4.s_addr;
        }
    }

    return;
}

int32
vrrp_shutdown_sess(tbl_vrrp_session_t *p_db_sess)
{
    int32                     rc = PM_E_NONE;
    
    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Event: Virtual-router %d transition to INITIALIZE state", 
        p_db_sess->key.vrid);

    p_db_sess->shutdown_flag = TRUE;
   
    /* If state is Master, must send Advt with priority 0.  Set state first & SendAdvt
       function will send correctly */
    if (VRRP_STATE_MASTER == p_db_sess->state)
    {    
        vrrp_send_advt(p_db_sess);
    }  

    /* Change state & remove IP addrs before leaving multicast group */
    rc = vrrp_fe_state_change(p_db_sess, p_db_sess->state, VRRP_STATE_INIT, p_db_sess->owner);
    if (rc)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Event: State change failed for vrid %d", p_db_sess->key.vrid);
        return PM_E_FAIL;
    }

    p_db_sess->state = VRRP_STATE_INIT;
    p_db_sess->vrrp_uptime = 0;

    rt_if_set_vmac(p_db_sess->ifname, p_db_sess->vmac, FALSE);

    p_db_sess->master_ip_status = FALSE;
    sal_memset (&p_db_sess->master_ip, 0, sizeof(addr_t));
    sal_memset (&p_db_sess->primary_ip, 0, sizeof(addr_t));
  
    p_db_sess->master_down_interval = 0;
    p_db_sess->master_advt_interval = 0;
    p_db_sess->master_priority = 0;

    rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);
    if (rc < 0)
    {
        VRRP_LOG_ERR("Set vrrp session enable fail");
        return rc;
    }

    return rc;
}

int32
vrrp_state_to_master(tbl_vrrp_session_t *p_db_sess)
{    
    int32                     rc = PM_E_NONE;
    tbl_interface_t           *p_db_if = NULL;
    tbl_interface_t           intf;
    uint32                    primary_ip = 0;
    char                      ifname_ext[IFNAME_EXT_SIZE];

    VRRP_PTR_CHECK(p_db_sess);

    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Event: Virtual-router %d transition to MASTER state", p_db_sess->key.vrid);
    
    /* get interface status */
    sal_memcpy(intf.key.name, p_db_sess->ifname, IFNAME_SIZE);  

    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);

    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        VRRP_LOG_ERR("Can't find specified interface for ifname %s", ifname_ext);
        return PM_E_FAIL;
    }
    
    if (!tbl_interface_is_running(p_db_if))
    {
        p_db_sess->shutdown_flag = TRUE;
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Warning: Interface %s is down; can't transition to MASTER state", 
            ifname_ext);
        return PM_E_FAIL;
    }
    
    /* get interface primary ip */
    vrrp_get_interface_primary_ip(p_db_if->key.name, &primary_ip);
    p_db_sess->primary_ip.family = AF_INET;
    p_db_sess->primary_ip.u.prefix4.s_addr = primary_ip;

    /* When in Master state, the countdown timer associated with the
      session represents the Advertisement interval timer */
    p_db_sess->timer = p_db_sess->advt_interval;

    if (PM_E_NONE != vrrp_send_advt(p_db_sess))
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_TX, "VRRP SEND[Hello]: Error transmitting VRRP advertisement");
        return PM_E_FAIL;
    }

    /* Perform any layer 2 manipulation necessary to change to Master   */
    rc = vrrp_fe_state_change(p_db_sess, p_db_sess->state, VRRP_STATE_MASTER, p_db_sess->owner);
    if (rc)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Error: State change failed for vrid %d", p_db_sess->key.vrid);
        return PM_E_FAIL;
    }

    /* Set the State variable */
    p_db_sess->state = VRRP_STATE_MASTER;
    p_db_sess->stats_become_master++;

    snmp_vrrp_transition_to_master_trap(&p_db_sess->primary_ip.u.prefix4);

    return rc;
}

int32
vrrp_state_to_backup(tbl_vrrp_session_t *p_db_sess)
{    
    int32                     rc = PM_E_NONE;
    
    VRRP_PTR_CHECK(p_db_sess);
    
    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Event: Virtual-router %d transition to BACKUP state", p_db_sess->key.vrid);
    
    /* When in Backup state, the countdown timer represents the
      Master_Down_Timer, and must be set appropriately */
    p_db_sess->timer = p_db_sess->master_down_interval;

    /* Perform any layer 2 manipulation necessary to change to Backup     */
    rc = vrrp_fe_state_change(p_db_sess, p_db_sess->state, VRRP_STATE_BACKUP, p_db_sess->owner);
    if (rc)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Error: State change failed for vrid %d", p_db_sess->key.vrid);
        return PM_E_FAIL;
    }

    /* Set the state variable */
    p_db_sess->state = VRRP_STATE_BACKUP;

    return rc;
}

int32
vrrp_noshut_sess(tbl_vrrp_session_t *p_db_sess)
{
    int32                     rc = PM_E_NONE;
    uint32                    uptime = 0;
    struct sal_timeval        tv;

    if (p_db_sess->owner)
    {
        p_db_sess->priority = VRRP_DEFAULT_IP_OWNER_PRIORITY;
    }

    p_db_sess->master_down_interval = vrrp_compute_masterdowninterval(p_db_sess, p_db_sess->advt_interval, 
        p_db_sess->priority);
  
    p_db_sess->shutdown_flag = FALSE;

    /* Change to proper state */
    if (p_db_sess->enable)
    {
        if (VRRP_DEFAULT_IP_OWNER_PRIORITY == p_db_sess->priority)
        {
            vrrp_state_to_master(p_db_sess);
        }
        else
        {
            vrrp_state_to_backup(p_db_sess);
        }
    }

    if (VRRP_STATE_INIT != p_db_sess->state)
    {
        ctc_task_update_boottime(&tv);
        uptime = tv.tv_sec * 1000 + + tv.tv_usec / 1000;
        p_db_sess->vrrp_uptime = sal_hton32(uptime);
    }

    if(p_db_sess->trackobj_id != 0)
    {
        vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, p_db_sess->trackobj_status);
    }

    return rc;
}

int32
vrrp_timer(tbl_vrrp_global_t *p_db_vrrp_glb, uint8 time_mode)
{
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    uint8                     update_sess_num = 0;
    int32                     rc = PM_E_NONE;

    VRRP_PTR_CHECK(p_db_vrrp_glb);

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        VRRP_LOG_ERR("Tbl_vrrp_session is not exist");
        return rc;
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        if (GLB_VRRP_SECONDS_MODE == time_mode)
        {
            if (update_sess_num >= (p_db_vrrp_glb->vrrp_session_count - p_db_vrrp_glb->vrrp_msec_session_count))
            {
                break;    
            }
        }
        else
        {
            if (update_sess_num >= p_db_vrrp_glb->vrrp_msec_session_count)
            {
                break;    
            }
        }
        
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if (time_mode != p_db_sess->time_mode)
        {
            continue;
        }

        switch (p_db_sess->state)
        {
        case VRRP_STATE_INIT:
            break;
 
        case VRRP_STATE_BACKUP:
            if(GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
            {
                /* If timer expired --> Master */
                if ((p_db_sess->timer == 0) || (--(p_db_sess->timer)) <= 0)
                {
                    vrrp_state_to_master(p_db_sess);
                }
            }
            else
            {
                if (p_db_sess->timer <= 100)
                {
                    p_db_sess->timer = 0;                  
                }
                else
                {
                    p_db_sess->timer = p_db_sess->timer - 100;   
                }
   
                if (0 == p_db_sess->timer)
                {
                    vrrp_state_to_master(p_db_sess);
                }
            }
            break;
 
        case VRRP_STATE_MASTER:
            if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
            {
                /* If timer expired --> Tx advt */
                if ((--(p_db_sess->timer)) <= 0)
                {
                    vrrp_send_advt(p_db_sess);
                    p_db_sess->timer = p_db_sess->advt_interval;
                }
            }
            else
            {
                if (p_db_sess->timer <= 100)
                {
                    p_db_sess->timer = 0;                  
                }
                else
                {
                    p_db_sess->timer = p_db_sess->timer - 100;
                }
                
                if(p_db_sess->timer <= 0)
                {
                    vrrp_send_advt(p_db_sess);
                    p_db_sess->timer = p_db_sess->advt_interval;
                }
            }
            break;
        }

        update_sess_num++;
    }

    return rc;
}

void vrrp_timer_fn(void *p_data)
{
    tbl_vrrp_global_t *p_db_vrrp_glb = NULL;

    p_db_vrrp_glb = (tbl_vrrp_global_t*)p_data;
    if (!p_db_vrrp_glb)
    {
        VRRP_LOG_ERR("vrrp_timer_fn return null pointer when vrrp timer timeout\n");
        return;
    }

    p_db_vrrp_glb->t_vrrp_timer = NULL;

    vrrp_timer(p_db_vrrp_glb, GLB_VRRP_SECONDS_MODE);

    p_db_vrrp_glb->t_vrrp_timer = ctc_task_add_timer(TASK_PRI_NORMAL, vrrp_timer_fn, 
        p_db_vrrp_glb, GLB_VRRP_ADVT_INT_DFLT);

    return;
}

int32 
vrrp_sys_init_timer(tbl_vrrp_global_t *p_db_vrrp_glb, bool start)
{
    VRRP_PTR_CHECK(p_db_vrrp_glb);

    if (p_db_vrrp_glb->t_vrrp_timer)
    {
        ctc_task_delete_timer(p_db_vrrp_glb->t_vrrp_timer);
        p_db_vrrp_glb->t_vrrp_timer = NULL;
    }    
        
    if (start)
    {
        p_db_vrrp_glb->t_vrrp_timer = ctc_task_add_timer(TASK_PRI_NORMAL, vrrp_timer_fn, 
            p_db_vrrp_glb, GLB_VRRP_ADVT_INT_DFLT);
    }
    
    return PM_E_NONE;
}

void vrrp_msec_timer_fn(void *p_data)
{
    tbl_vrrp_global_t *p_db_vrrp_glb = NULL;

    p_db_vrrp_glb = (tbl_vrrp_global_t*)p_data;
    if (!p_db_vrrp_glb)
    {
        VRRP_LOG_ERR("vrrp_msec_timer_fn return null pointer when vrrp msec timer timeout\n");
        return;
    }

    p_db_vrrp_glb->t_vrrp_msec_timer = NULL;

    vrrp_timer(p_db_vrrp_glb, GLB_VRRP_MILLI_SECONDS_MODE);

    p_db_vrrp_glb->t_vrrp_msec_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, vrrp_msec_timer_fn, 
        p_db_vrrp_glb, GLB_VRRP_ADVT_MSEC_MIN);

    return;
}

int32 
vrrp_sys_init_msec_timer(tbl_vrrp_global_t *p_db_vrrp_glb, bool start)
{
    VRRP_PTR_CHECK(p_db_vrrp_glb);

    if (p_db_vrrp_glb->t_vrrp_msec_timer)
    {
        ctc_task_delete_timer(p_db_vrrp_glb->t_vrrp_msec_timer);
        p_db_vrrp_glb->t_vrrp_msec_timer = NULL;
    }    
        
    if (start)
    {        
        p_db_vrrp_glb->t_vrrp_msec_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, vrrp_msec_timer_fn, 
            p_db_vrrp_glb, GLB_VRRP_ADVT_MSEC_MIN);
    }
    
    return PM_E_NONE;
}

uint16
in_checksum(uint16 *ptr, uint8 input)
{
    register int32       sum = 0;
    uint16               oddbyte = 0;
    register uint16      result = 0;
    uint8                *p, *q;
    uint8                nbytes = input;
    
    sum = 0;
    while (nbytes > 1)  
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) 
    {
        p = (uint8 *)&oddbyte;
        q = (uint8 *)ptr;
        *p = *q;
        sum += oddbyte;
    }

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

uint16
vrrp_in6_checksum(vrrp_ip_header_t *p_vrrp_ip_header, uint16 *ptr, uint8 input)
{
    register int32         sum = 0;
    uint16                 oddbyte = 0;
    register uint16        result = 0;
    uint8                  *p = NULL;
    uint8                  *q = NULL;
    uint8                  nbytes = input;
    struct vrrp_in6_pseudo_hdr  in6_ph;
    uint16                 *tmp_p = NULL;

    /* Prepare IPv6 Pseudo-Header */
    sal_memset(&in6_ph, 0, sizeof (struct vrrp_in6_pseudo_hdr));
    sal_memcpy(&in6_ph.in6_src, &p_vrrp_ip_header->src_ip_addr, sizeof(uint32));
    sal_memcpy(&in6_ph.in6_dst, &p_vrrp_ip_header->dest_ip_addr, sizeof(uint32));  
    in6_ph.length = sal_hton32(input);
    in6_ph.nxt_hdr = IPPROTO_VRRP;

    /* First add IPv6 Pseudo-Header */
    sum = 0;
    nbytes = sizeof (struct vrrp_in6_pseudo_hdr);
    tmp_p = (uint16 *) &in6_ph;
    while (nbytes)
    {
        sum += *tmp_p++;
        nbytes -= 2;
    }

    /* add vrrp packet */
    nbytes = input;
    while (nbytes > 1)  
    {
        sum += *ptr++;
        nbytes -= 2;
    }
  
    if (nbytes == 1) 
    {
        p = (uint8 *)&oddbyte;
        q = (uint8 *)ptr;
        *p = *q;
        sum += oddbyte;
    }
  
    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
  
    return result;
}

int32
vrrp_send_advt(tbl_vrrp_session_t *p_db_sess)
{
    uint32                   *size = NULL;
    uint8                    *pnt = NULL;
    tbl_vrrp_global_t        *p_db_vrrp_glb = NULL;
    uint8                    dst_mac[MAC_ADDR_LEN] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x12}; 
    uint16                   w_value = 0;
    uint8                    value = 0;
    int32                    rc = PM_E_NONE;
    uint32                   dw_value = 0;
    struct vrrp_advt_info    *vi = NULL;
    char                     addr[INET_ADDRSTRLEN] = {0};
    vrrp_ip_header_t         *p_vrrp_ip_header = NULL;
    int                      vid = GLB_ALL_VLAN_ID;
    char                     ifname_ext[IFNAME_EXT_SIZE];

    VRRP_PTR_CHECK(p_db_sess);
        
    /* check exist */
    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        VRRP_LOG_ERR("Tbl_vrrp_global not found");
        return PM_E_FAIL;
    }

    sal_memset(p_db_vrrp_glb->buf, 0, GLB_VRRP_PKT_BUF);
    p_db_vrrp_glb->pkt_len = 0;

    pnt = p_db_vrrp_glb->buf;
    size = &(p_db_vrrp_glb->pkt_len);

    VRRP_ENCODE_PUT(dst_mac, MAC_ADDR_LEN);
    VRRP_ENCODE_PUT(p_db_sess->vmac, MAC_ADDR_LEN);

    /* ip type */
    w_value = sal_hton16(0x0800);
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /* ip header version and head length */
    value = 0x45;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /* type of service */
    value = 0;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /*total length*/
    if (GLB_VRRP_VERSION_V3 != p_db_vrrp_glb->vrrp_version)
    {
        w_value = sal_hton16(SIZE_VRRP_ADVT + 20);
    }
    else
    {
        w_value = sal_hton16(SIZE_VRRP_ADVT  - SIZE_VRRP_AUTH + 20);
    }
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /* ip header identification*/
    w_value = 0;
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /* flag and fragment */
    w_value = sal_hton16(0x4000);
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /* time to live, vrrp packets ttl must be 255 */
    value = 0xff;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /* vrrp protocol 112 */
    value = IPPROTO_VRRP;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /*header checksum*/  
    w_value = 0;
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /* source ip address */
    VRRP_ENCODE_PUT(&p_db_sess->primary_ip.u.prefix4.s_addr, sizeof(uint32));

    /* destination ip address */
    dw_value = sal_hton32(VRRP_MCAST_ADDR);
    VRRP_ENCODE_PUT(&dw_value, sizeof(uint32));

    /* VRRP packet */
    /* version & type */
    value = (p_db_vrrp_glb->vrrp_version << 4) | VRRP_ADVT_TYPE;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /* Virtual Rtr ID */
    value = p_db_sess->key.vrid;  
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /* priority */
    if (p_db_sess->shutdown_flag)
    {
        value = 0;
    }
    else
    {
        value = p_db_sess->priority;
    }
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    /* number of vip address */
    value = p_db_sess->num_ip_addrs;
    VRRP_ENCODE_PUT(&value, sizeof(uint8));

    if (GLB_VRRP_VERSION_V3 != p_db_vrrp_glb->vrrp_version)
    {
        /* authentication type */
        value = VRRP_AUTH_NONE;
        VRRP_ENCODE_PUT(&value, sizeof(uint8));
    
        /* vrrp interval */
        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            value = p_db_sess->advt_interval;
        }
        else
        {
            value = GLB_VRRP_ADVT_INT_DFLT;    
        }
        VRRP_ENCODE_PUT(&value, sizeof(uint8));
    }
    else
    {
        /* change advt interval to centiseconds */
        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            w_value = p_db_sess->advt_interval * 100;
        }
        else
        {
            w_value = p_db_sess->advt_interval / 10;    
        }
        w_value = sal_hton16(w_value);
        VRRP_ENCODE_PUT(&w_value, sizeof(uint16));
    }

    /*vrrp checksum*/
    w_value = 0;
    VRRP_ENCODE_PUT(&w_value, sizeof(uint16));

    /*vip address*/
    VRRP_ENCODE_PUT(&p_db_sess->vip.u.prefix4.s_addr, sizeof(uint32));

    /*authentication data*/
    if (GLB_VRRP_VERSION_V3 != p_db_vrrp_glb->vrrp_version)
    {
        dw_value = 0;
        VRRP_ENCODE_PUT(&dw_value, sizeof(uint32));
        VRRP_ENCODE_PUT(&dw_value, sizeof(uint32));
    }

    /*calulate vrrp checksum and ip header checksum*/   
    /*ether header 14 bytes, ip header 20 bytes, then vrrp payload*/
    vi = (struct vrrp_advt_info *)(p_db_vrrp_glb->buf + 14 + 20);
    if (GLB_VRRP_VERSION_V3 != p_db_vrrp_glb->vrrp_version)
    {
        vi->cksum = in_checksum ((uint16*)vi, SIZE_VRRP_ADVT);
    }
    else
    {
        p_vrrp_ip_header = (vrrp_ip_header_t *)(p_db_vrrp_glb->buf + 14);
        vi->cksum = vrrp_in6_checksum(p_vrrp_ip_header, (uint16*)vi, (SIZE_VRRP_ADVT - SIZE_VRRP_AUTH));
    }
    
    if (vrrp_debug_is_on(RTDBG_FLAG_VRRP_PACKET_DETAIL))
    {
        vrrp_packet_dump(vi);
    } 
    
    /*ether header 14 bytes, header check sum is in the 11 bytes of the ip header(10), the checksum data
    is from ip header to vrrp header, that's why 14 and length 20(ip header length)*/
     *((uint16*)(p_db_vrrp_glb->buf + 14 + 10))
     = in_checksum ((uint16 *)(p_db_vrrp_glb->buf + 14), 20);

    if (p_db_vrrp_glb->pkt_len < VRRP_MIN_PKT_LEN)
    {
        sal_memset(((uint8*)p_db_vrrp_glb->buf + p_db_vrrp_glb->pkt_len), 0, (VRRP_MIN_PKT_LEN - p_db_vrrp_glb->pkt_len)); 
        p_db_vrrp_glb->pkt_len = VRRP_MIN_PKT_LEN;    
    }

    if (0 == sal_memcmp(p_db_sess->ifname, "vlan", 4))
    {
        sal_sscanf(p_db_sess->ifname, "vlan%d", &vid);
    }
    
    /* send pkt through vrrp socket to specific interface */
    if (ipc_pkt_tx_vrrp((uint8*)p_db_vrrp_glb->buf, p_db_vrrp_glb->pkt_len, p_db_sess->ifindex, vid))
    {
        VRRP_LOG_ERR("send msg through port %d fail for vrrp %u", 
            p_db_sess->ifindex, p_db_sess->key.vrid);
        return PM_E_SOCK;
    } 

    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext);
    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_TX, "VRRP SEND[Hello]: Advertisement sent for vrid=[%d], virtual-ip=[%s] via"
        " interface %s vid %d", p_db_sess->key.vrid, 
        sal_inet_ntop(AF_INET, &p_db_sess->vip.u.prefix4.s_addr, addr, INET_ADDRSTRLEN), ifname_ext, vid);

    if (p_db_sess->shutdown_flag)
    {
        p_db_sess->stats_priority_zero_pkts_sent++;
    }
 
    return rc;
}

static int
vrrp_handle_advt(tbl_vrrp_session_t *p_db_sess,  /*the recvd interface session*/
                    struct vrrp_advt_info *advt, /*the advertisement packet information*/
                    addr_t src_ip, uint16 advt_interval) /*the advertisement packet source primary ip address*/
{
    int32                    rc = PM_E_NONE;
    
    VRRP_PTR_CHECK(p_db_sess);
    VRRP_PTR_CHECK(advt);

    switch (p_db_sess->state) 
    {
    case VRRP_STATE_INIT: 
        /* Drop packet */
        break;

    case VRRP_STATE_BACKUP:
        /* Check msg priority */
        if (0 == advt->priority) 
        {       
            p_db_sess->timer = p_db_sess->skew_time;
            break;
        }   

        /* learn master information */    
        p_db_sess->master_ip_status = TRUE;    
        sal_memcpy(&p_db_sess->master_ip, &src_ip, sizeof(addr_t));            
        p_db_sess->master_priority = advt->priority;            
        p_db_sess->master_advt_interval = advt_interval;   
    
        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            if (p_db_sess->learn_master) 
            { 
                p_db_sess->master_down_interval = vrrp_compute_masterdowninterval(p_db_sess, advt_interval, 
                    advt->priority);
            }       
        }
     
        /* Check Preempt */
        if (!p_db_sess->preempt_mode)
        { 
            p_db_sess->timer = p_db_sess->master_down_interval;
            break;
        }

        if (advt->priority >= p_db_sess->priority)
        {
            p_db_sess->timer = p_db_sess->master_down_interval;  
            break;
        }
        break;
    
    case VRRP_STATE_MASTER:

        /* Check msg priority */
        if (0 == advt->priority)
        {       
            vrrp_send_advt(p_db_sess);
            break;
        }    

        if ((advt->priority > p_db_sess->priority)
            || ((advt->priority == p_db_sess->priority) 
            && (src_ip.u.prefix4.s_addr > p_db_sess->primary_ip.u.prefix4.s_addr)))
        {
            p_db_sess->master_ip_status = TRUE;    
            sal_memcpy(&p_db_sess->master_ip, &src_ip, sizeof(addr_t));             
            p_db_sess->master_priority = advt->priority;            
            p_db_sess->master_advt_interval = advt_interval;    
    
            if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
            {
                if (p_db_sess->learn_master)
                { 
                    p_db_sess->master_down_interval = vrrp_compute_masterdowninterval(p_db_sess, advt_interval,
                        advt->priority);
                }
            }

            vrrp_state_to_backup(p_db_sess);
            break;
        }
        break;
    }

    return rc;
}

void
vrrp_packet_dump(struct vrrp_advt_info *p_vrrp_advt)
{
    char     addr[INET_ADDRSTRLEN] = {0};
    uint8    ver_type = 0;
    uint16   advt_interval = 0;
    
    if (!p_vrrp_advt)
    {
        return;    
    }

    VRRP_LOG_PRINT("------------------------------------------");
    ver_type = p_vrrp_advt->ver_type >> 4;
    VRRP_LOG_PRINT("Version %d", ver_type);
    
    VRRP_LOG_PRINT("Type %d", (p_vrrp_advt->ver_type & 0xf));
    
    VRRP_LOG_PRINT("Priority %d", p_vrrp_advt->priority);
    
    VRRP_LOG_PRINT("Count IP address %d", p_vrrp_advt->num_ip_addrs);

    if (GLB_VRRP_VERSION_V3 != ver_type)
    {
        VRRP_LOG_PRINT("Auth Type %d", p_vrrp_advt->auth_type);
        VRRP_LOG_PRINT("Adver Int %d", p_vrrp_advt->advt_interval);
    }
    else
    {
        advt_interval = p_vrrp_advt->auth_type;
        advt_interval = (advt_interval << 8) + p_vrrp_advt->advt_interval;
        VRRP_LOG_PRINT("Adver Int %d", advt_interval);
    }
    
    VRRP_LOG_PRINT("Checksum 0x%x", p_vrrp_advt->cksum);
    
    VRRP_LOG_PRINT("IP Address1 %s", sal_inet_ntop(AF_INET, &p_vrrp_advt->ip_addr, addr, INET_ADDRSTRLEN));

    if (GLB_VRRP_VERSION_V3 != p_vrrp_advt->ver_type)
    {
        VRRP_LOG_PRINT("Authentication Data1 %x", p_vrrp_advt->auth_data[0]);

        VRRP_LOG_PRINT("Authentication Data2 %x", p_vrrp_advt->auth_data[1]);
    }
    
    return;
}


int32
vrrp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    int32                    rc = PM_E_NONE;
    uint32                   rcv_ifindex = 0;
    uint8                    *p_rcv_buf = NULL;
    tbl_interface_t          *p_db_if = NULL;
    vrrp_message_header_t    *p_vrrp_header = NULL;  
    vrrp_ip_header_t         *p_vrrp_ip_header = NULL;
    struct vrrp_advt_info    *p_vrrp_advt = NULL;
    tbl_vrrp_session_t       sess;
    tbl_vrrp_session_t       *p_db_sess = NULL;
    bool                     is_owner = FALSE;
    bool                     is_submatch = FALSE;
    uint16                   length = 0;
    uint16                   cksum = 0;
    addr_t                   src_ip;
    char                     addr[INET_ADDRSTRLEN] = {0};
    uint8                    ver_type = 0;
    uint16                   advt_interval = 0;
    uint32                   auth_failure_type = 0;
    uint16                   pkt_vid = GLB_ALL_VLAN_ID;
    int                      vid = GLB_ALL_VLAN_ID;
    bool                     match_flag = FALSE;
    uint8                    index = 0;
    uint32                   *ip_addr = NULL;
    tbl_vrrp_global_t        *p_db_vrrp_glb = NULL;
    char                      ifname_ext[IFNAME_EXT_SIZE];
    
    sal_memset(&src_ip, 0, sizeof(addr_t));
    sal_memset(&sess, 0, sizeof(tbl_vrrp_session_t));
    
    p_rcv_buf = (uint8*)p_msg->data;
    rcv_ifindex = p_msg->u_hdr.pkt.ifindex;

    if (p_msg->data_len < VRRP_PKT_MIN_LEN)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP Error: recvfrom packet short");
        return PM_E_VRRP_BAD_PACKET;
    }
   
    p_db_if = tbl_interface_get_interface_by_ifindex(rcv_ifindex);
    if (!p_db_if)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "Can't find specified interface %d", rcv_ifindex);
        return PM_E_FAIL;
    }

    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Received [%d] bytes via interface %s.", 
            p_msg->data_len, ifname_ext);
    
    p_vrrp_header = (vrrp_message_header_t*)p_rcv_buf;

    /* there is no cli to configure tpid, therefore use GLB_DEFAULT_TPID */
    p_vrrp_header->eth_type = sal_ntoh16(p_vrrp_header->eth_type);

    if (GLB_DEFAULT_TPID == p_vrrp_header->eth_type)
    {
        pkt_vid = *(uint16*)(p_rcv_buf + ETH_HEADER_LEN);
        pkt_vid = sal_hton16(pkt_vid);
            
        p_rcv_buf = p_rcv_buf + ETH_HEADER_LEN + VLAN_TAG_LEN;
        length = p_msg->data_len - ETH_HEADER_LEN - VLAN_TAG_LEN;
    }
    else if (ETH_P_IPV4 == p_vrrp_header->eth_type)
    {
        p_rcv_buf = p_rcv_buf + ETH_HEADER_LEN;
        length = p_msg->data_len - ETH_HEADER_LEN;
    }
    else
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]:Dropped - Invalid ethernet type[%x]!", p_vrrp_header->eth_type);
        return PM_E_VRRP_BAD_PACKET;
    }

    p_vrrp_ip_header = (vrrp_ip_header_t*)p_rcv_buf;
    
    /*check the protocol, must be vrrp packets*/
    if (IPPROTO_VRRP != p_vrrp_ip_header->protocol)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid protocol type[%d]!", 
            p_vrrp_ip_header->protocol);  
        return rc;
    }

    p_rcv_buf = p_rcv_buf + sizeof(vrrp_ip_header_t);
    p_vrrp_advt = (struct vrrp_advt_info*)p_rcv_buf;

    if (vrrp_debug_is_on(RTDBG_FLAG_VRRP_PACKET_DETAIL))
    {
        vrrp_packet_dump(p_vrrp_advt);
    }

    /* if vrrp session is not exist, return */
    sess.key.vrid = p_vrrp_advt->vrid;
    p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess.key);
    if (!p_db_sess)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid vrid (recved vrid=%d)!", 
            p_vrrp_advt->vrid);
        return PM_E_NONE;
    }
    else
    {
        if (VRRP_STATE_INIT == p_db_sess->state)
        {
            VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Session %d in Init state!", 
                p_vrrp_advt->vrid);
            return PM_E_NONE;    
        }
    }

    /* check src ip */
    src_ip.u.prefix4.s_addr = p_vrrp_ip_header->src_ip_addr;
    src_ip.family = AF_INET;
    vrrp_addr_subnet_match(p_db_sess->ifname, &src_ip, &is_owner, &is_submatch);
    if (is_owner)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Duplicate IP address detected!");        
        return PM_E_NONE;
    }   

    p_db_sess->stats_advt_rcvd++;
    
    /* check the local router is not ip address owner*/
    if (p_db_sess->owner || (VRRP_DEFAULT_IP_OWNER_PRIORITY == p_db_sess->priority))
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Session %d is IP address owner.", 
            p_db_sess->key.vrid);
        return PM_E_NONE;
    }

    /* validate the IP TTL - MUST be 255 */
    if (VRRP_IP_TTL != p_vrrp_ip_header->ttl) 
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid IP TTL (recved TTL %d)", 
            p_vrrp_ip_header->ttl);
        p_db_sess->stats_ip_ttl_errors++;
        return PM_E_VRRP_BAD_PACKET;
    }

    /* validate packet length */
    if ((length - sizeof(vrrp_ip_header_t)) < SIZE_VRRP_ADVT)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: VRRP RECV[Hello]:Dropped - Too short packet!");      
        p_db_sess->stats_pkt_len_errors++;
        return PM_E_VRRP_BAD_PACKET;
    }

    if (0 == p_vrrp_advt->priority)
    {
        p_db_sess->stats_priority_zero_pkts_rcvd++;    
    }

    /* validate VRRP version */
    ver_type = p_vrrp_advt->ver_type >> 4;
    if ((GLB_VRRP_VERSION_V2 != ver_type) && (GLB_VRRP_VERSION_V3 != ver_type))
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid version (recved version %d)", 
            (p_vrrp_advt->ver_type >> 4));
        return PM_E_VRRP_BAD_PACKET;
    }
    
    p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    if (!p_db_vrrp_glb)
    {
        VRRP_LOG_ERR("Tbl_vrrp_global not found");
    }

    if (p_db_vrrp_glb->vrrp_version != ver_type)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - mismatch version (recved version %d)", 
            ver_type);
        return PM_E_VRRP_BAD_PACKET;
    }

    /* validate VRRP type field. */
    if ((p_vrrp_advt->ver_type & 0x0f) != VRRP_ADVT_TYPE)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid type (recved type %d)", 
            (p_vrrp_advt->ver_type & 0x0f));
  
        p_db_sess->stats_invalid_type_pkts_rcvd++;
        return PM_E_VRRP_BAD_PACKET;
    }

    /* validate VRRP checksum */
    cksum = p_vrrp_advt->cksum;
    p_vrrp_advt->cksum = 0;
    
    if (GLB_VRRP_VERSION_V2 == ver_type)
    {
        p_vrrp_advt->cksum = in_checksum((uint16 *)p_vrrp_advt, SIZE_VRRP_ADVT);
    }
    else
    {
        p_vrrp_advt->cksum = vrrp_in6_checksum(p_vrrp_ip_header, (uint16 *)p_vrrp_advt, (SIZE_VRRP_ADVT - SIZE_VRRP_AUTH));   
    }
    
    if (p_vrrp_advt->cksum != cksum)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid checksum (recved checksum 0x%x)", 
            cksum);
        return PM_E_VRRP_BAD_PACKET;
    }

    if (GLB_VRRP_VERSION_V2 == ver_type)
    {
        if (p_vrrp_advt->auth_type > VRRP_AUTH_MD5)
        {
            VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Invalid Authentication type (recved"
                " auth type %d )", 
                p_vrrp_advt->auth_type);

            p_db_sess->stats_invalid_auth_type++;
    
            auth_failure_type = AUTH_TYPE_INVALID;
            snmp_vrrp_failure_trap((struct sal_in4_addr *)(&p_vrrp_advt->ip_addr), auth_failure_type);
            
            return PM_E_VRRP_BAD_PACKET;
        }  
        
        if (p_vrrp_advt->auth_type != 0)
        {
            VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Authentication type mismatch "
                "(session type %s)", ((VRRP_AUTH_NONE == p_vrrp_advt->auth_type) ? "NONE" : "Simple Text"));

            p_db_sess->stats_auth_type_mismatch++;
            
            auth_failure_type = AUTH_TYPE_MISMATCH;
            snmp_vrrp_failure_trap((struct sal_in4_addr *)(&p_vrrp_advt->ip_addr), auth_failure_type);

            return PM_E_VRRP_BAD_PACKET;
        }
    }

    /* validate vrid/interface combination */
    if ((p_db_sess->ifindex != p_db_if->ifindex)
        && sal_memcmp(p_db_sess->ifname, "vlan", 4))
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Received in wrong interface "
            "(recved src ip %s)", 
            sal_inet_ntop(AF_INET, &p_vrrp_ip_header->src_ip_addr, addr, INET_ADDRSTRLEN));  
        return PM_E_VRRP_BAD_PACKET;
    }


    if (0 == sal_memcmp(p_db_sess->ifname, "vlan", 4))
    {
        sal_sscanf(p_db_sess->ifname, "vlan%d", &vid);
    }

    if ((GLB_ALL_VLAN_ID != pkt_vid) && (vid != (pkt_vid & 0xfff)))
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Received in wrong vlan "
            "(recved src ip %s vlan id %d)", 
            sal_inet_ntop(AF_INET, &p_vrrp_ip_header->src_ip_addr, addr, INET_ADDRSTRLEN), pkt_vid);  
        return PM_E_VRRP_BAD_PACKET;    
    }

    ip_addr = &p_vrrp_advt->ip_addr;
    for (index = 0; index < p_vrrp_advt->num_ip_addrs; index++)
    {        
        ip_addr = ip_addr + index;        
        if (*ip_addr == p_db_sess->vip.u.prefix4.s_addr)
        {
            match_flag = TRUE;
            break;
        }        
    } 
    
    /* check the virtual IP address. */
    if (!match_flag)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Bad virtual IP address (session vip %s)", 
            sal_inet_ntop(AF_INET, &p_db_sess->vip.u.prefix4.s_addr, addr, INET_ADDRSTRLEN));
  
        p_db_sess->stats_addr_list_errors++;
        return PM_E_VRRP_BAD_PACKET;
    }

    /* when learn master mode is set, vrrp will not check master advt and backup advt */
    if (GLB_VRRP_VERSION_V2 == ver_type)
    {
        advt_interval = p_vrrp_advt->advt_interval;          
    }
    else
    {  
        advt_interval = p_vrrp_advt->auth_type;
        advt_interval = (advt_interval << 8) + p_vrrp_advt->advt_interval;

        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            advt_interval = advt_interval / 100;
        }
    }

    if (!p_db_sess->learn_master)
    {
        if (GLB_VRRP_SECONDS_MODE == p_db_sess->time_mode)
        {
            /* validate advertisement interval */           
            if (advt_interval != p_db_sess->advt_interval)
            {
                VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Advert Interval mismatch"
                    " (session interval %d second)", p_db_sess->advt_interval);
  
                p_db_sess->stats_advt_interval_errors++;
                return PM_E_VRRP_BAD_PACKET;
            }
        }
        else
        {
            /* for version v3, check advt interval mismatch,
               for version v2, can't check advt interval */
            if (GLB_VRRP_VERSION_V3 == ver_type)
            {
                /* validate advertisement interval 
                   advt_interval unit is centiseconds, 
                   p_db_sess->advt_interval unit is milli-second */           
                if (advt_interval != (p_db_sess->advt_interval / 10))
                {
                    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Dropped - Advert Interval mismatch"
                        " (session interval %d mili-second)", p_db_sess->advt_interval);
      
                    p_db_sess->stats_advt_interval_errors++;
                    return PM_E_VRRP_BAD_PACKET;
                }    
            }
        }
    }

    VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "VRRP RECV[Hello]: Advertisement received for vrid=[%d], virtual-ip=[%s]"
        " from source %s is valid", 
        p_db_sess->key.vrid, 
        sal_inet_ntop(AF_INET, &p_db_sess->vip.u.prefix4.s_addr, addr, INET_ADDRSTRLEN), 
        sal_inet_ntop(AF_INET, &p_vrrp_ip_header->src_ip_addr, addr, INET_ADDRSTRLEN));

    /* If all of these checks pass, call vrrp_handle_advt to take action */
    rc = vrrp_handle_advt(p_db_sess, p_vrrp_advt, src_ip, advt_interval);

    return rc;
}

int32
vrrp_add_vip_entry(tbl_vrrp_session_t  *p_db_sess, tbl_interface_t  *p_db_if)
{
    int32 rc = PM_E_NONE;
    tbl_vrrp_vip_t     vip_entry;

    VRRP_PTR_CHECK(p_db_sess);
    VRRP_PTR_CHECK(p_db_if);

    sal_memset(&vip_entry, 0, sizeof(tbl_vrrp_vip_t));

    sal_memcpy(&vip_entry.key, &p_db_sess->vip, sizeof(addr_t));
    sal_memcpy(&vip_entry.vmac, &p_db_sess->vmac, GLB_ETH_ADDR_LEN);
    sal_memcpy(&vip_entry.interface_mac, p_db_if->mac_addr, GLB_ETH_ADDR_LEN);

    rc = tbl_vrrp_vip_add_vrrp_vip(&vip_entry);

    return rc;
}

int32
vrrp_del_vip_entry(tbl_vrrp_session_t  *p_db_sess)
{
    int32 rc = PM_E_NONE;
    tbl_vrrp_vip_t     vip_entry;

    VRRP_PTR_CHECK(p_db_sess);
    
    sal_memset(&vip_entry, 0, sizeof(tbl_vrrp_vip_t));

    sal_memcpy(&vip_entry.key, &p_db_sess->vip, sizeof(addr_t));
    
    rc = tbl_vrrp_vip_del_vrrp_vip(&vip_entry.key);

    return rc;
}

int32
vrrp_fe_state_change(tbl_vrrp_session_t  *p_db_sess, 
                        vrrp_state_t curr_state, 
                        vrrp_state_t new_state, 
                        uint8 owner)
{
    tbl_interface_t          *p_db_if = NULL;
    int32                    rc = PM_E_NONE;
    ds_connected_t           conn;
    mac_addr_t               vmac;
    uint32                   is_running = 0;   
    tbl_route_if_t           rt_if;
    tbl_route_if_t           *p_db_rtif  = NULL;
    char                     state[VRRP_STATE_MASTER][VRRP_STATE_MAX_STR_LEN + 1] ={"init", "backup", "master"};
    tbl_vrrp_vmac_t          vrrp_vmac;
    char   ifname_ext[IFNAME_EXT_SIZE];

    VRRP_PTR_CHECK(p_db_sess);

    sal_memset(&conn, 0, sizeof(ds_connected_t));
    sal_memset(&vmac, 0, sizeof(mac_addr_t));
    sal_memset(&rt_if, 0, sizeof(tbl_route_if_t));
    sal_memset(&vrrp_vmac, 0, sizeof(tbl_vrrp_vmac_t));

    logid_sys(LOG_VRRP_4_VRRP_STATE, p_db_sess->key.vrid, IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext),
        state[curr_state - 1], state[new_state - 1]);

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_sess->ifindex);
    if (!p_db_if)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "Can't find specified interface %d", p_db_sess->ifindex);
        return PM_E_FAIL;
    }

    sal_memcpy(rt_if.key.name, p_db_sess->ifname, IFNAME_SIZE);         
    p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
    if (NULL == p_db_rtif)
    {
        VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_PACKET_RX, "Can't find route interface %d", p_db_sess->ifindex);
        return PM_E_FAIL;
    }
    is_running = tbl_interface_is_running(p_db_if);  

    /*set session config to default when disable session*/
    if (new_state == VRRP_STATE_INIT)
    {
        p_db_sess->priority = p_db_sess->config_priority;
    }

    switch (curr_state) {
    case VRRP_STATE_INIT:
    case VRRP_STATE_BACKUP:
        if (new_state == VRRP_STATE_MASTER)
        {
            /* add vmac */  
            rc = rt_if_set_vmac(p_db_if->key.name, p_db_sess->vmac, TRUE);
            if (rc)
            {
                VRRP_LOG_ERR("Add vmac for vrrp session %d interface %s failure", p_db_sess->key.vrid,
                    IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext));
            }

            /* add vmac to vmac table */
            vrrp_vmac.ifindex = p_db_sess->ifindex;
            sal_memcpy(vrrp_vmac.key.mac, p_db_sess->vmac, GLB_ETH_ADDR_LEN);
            tbl_vrrp_vmac_add_vrrp_vmac(&vrrp_vmac);

            /* add vip & arp, send */            
            conn.key.address.family = AF_INET;
            conn.key.address.prefixlen = 32;
            sal_memcpy(&conn.key.address.u.prefix4, &p_db_sess->vip.u.prefix4, sizeof(addr_ipv4_t));
            sal_memcpy(&conn.destination, &conn.key.address, sizeof(prefix_t));
            conn.flags = RT_IF_ADDR_VIRTUAL;
             
            if (!p_db_sess->owner)
            {
                 rt_if_add_vip_and_arp(p_db_if->key.name, p_db_sess->vmac, &conn);
            }
            else
            {
                arp_api_connected_add(p_db_if, p_db_rtif, &conn, 1, is_running, p_db_sess->vmac);
            }
            
            /* add vip table */  
            vrrp_add_vip_entry(p_db_sess, p_db_if);
        }
        break;
    
    case VRRP_STATE_MASTER:
       
        /* delete vip & arp, send */
        conn.key.address.family = AF_INET;
        conn.key.address.prefixlen = 32;
        sal_memcpy(&conn.key.address.u.prefix4, &p_db_sess->vip.u.prefix4, sizeof(addr_ipv4_t));
        sal_memcpy(&conn.destination, &conn.key.address, sizeof(prefix_t));
        conn.flags = RT_IF_ADDR_VIRTUAL;
       
        if (!p_db_sess->owner)
        {
            rt_if_del_vip_and_arp(p_db_if->key.name, &conn);
        }
        else
        {
            arp_api_connected_add(p_db_if, p_db_rtif, &conn, 1, is_running, vmac);    
        }

        /* delete vip table */
        vrrp_del_vip_entry(p_db_sess);

        /* delete vmac */  
        rc = rt_if_set_vmac(p_db_if->key.name, p_db_sess->vmac, FALSE);
        if (rc)
        {
            VRRP_LOG_ERR("Delete vmac for vrrp session %d interface %s failure", p_db_sess->key.vrid,
                IFNAME_ETH2FULL(p_db_sess->ifname, ifname_ext));
        }
        
        /* delete vmac from vmac table */
        sal_memcpy(vrrp_vmac.key.mac, p_db_sess->vmac, GLB_ETH_ADDR_LEN);
        tbl_vrrp_vmac_del_vrrp_vmac(&vrrp_vmac.key);
        break;

    default:
        return PM_E_FAIL;
        break;
    }

    return PM_E_NONE;
}

/*******************************************************************************
 * Name:    vrrp_get_master_router_mac
 * Purpose: when vrrp is enabled in this interface and is master and vmac is enabled, send vmac with arp response, 
          otherwise with interface mac.
 * Input:  
 *   ifp : interface pointer
 *   addr: pinging ip address 
 * Output: 
     vmac: virtual mac in vrrp router
 * Return: 
 *   GLB_VRRP_MAC_INTERFACE: error or use interface mac when do arp response
 *   GLB_VRRP_MAC_INVALID:    should not do arp response
 *   GLB_VRRP_MAC_VALID:      master router should response arp request with vmac
 * Note: N/A
 ******************************************************************************/
int32 
vrrp_get_master_router_mac(tbl_interface_t *p_db_if, uint32 *addr, uint8 *vmac)
{
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    
    if (!p_db_if || !addr || !vmac)
    {
        return GLB_VRRP_MAC_INTERFACE;   
    }

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        return GLB_VRRP_MAC_INTERFACE;   
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if ((p_db_sess->ifindex == p_db_if->ifindex)
            && (p_db_sess->vip.u.prefix4.s_addr == *addr))
        {
            break;    
        }
    }

    if ((vrid > GLB_VRRP_MAX_VRID) || !p_db_sess)
    {
        return GLB_VRRP_MAC_INTERFACE;
    }

    /*backup router should not response a arp request, and if it is backup router, it can't be session owner
    that's means backup router virtual ip will always not be the interface ip*/
    switch(p_db_sess->state)
    {
    case VRRP_STATE_MASTER:
        sal_memcpy(vmac, p_db_sess->vmac, GLB_ETH_ADDR_LEN);                
        return GLB_VRRP_MAC_VALID;   /*mater router should response arp request*/
    case VRRP_STATE_BACKUP:  
        return GLB_VRRP_MAC_INVALID; /*backup router and init state router should not response arp request*/
    case VRRP_STATE_INIT:            
        if (p_db_sess->owner)
        {
            return GLB_VRRP_MAC_INTERFACE;
        }
        else
        {
            return GLB_VRRP_MAC_INVALID;
        }
    default:
        return GLB_VRRP_MAC_INTERFACE;
    }

    return GLB_VRRP_MAC_INTERFACE;
}

int32
vrrp_if_up(tbl_interface_t *p_db_if)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_route_if_t            *p_db_rtif = NULL;
    tbl_route_if_key_t        rtif_key;
    
    VRRP_PTR_CHECK(p_db_if);

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
    if (!p_db_rtif)
    {
        return PM_E_NONE;       
    }

    if (0 == p_db_rtif->vrrp_session_num)
    {
        return PM_E_NONE;    
    }

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        VRRP_LOG_ERR("Tbl_vrrp_session is not exist");    
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if (!p_db_sess->shutdown_flag)
        {
            continue;
        }
        
        if (p_db_sess->ifindex == p_db_if->ifindex)
        {
            if (p_db_sess->enable)
            {
                vrrp_noshut_sess(p_db_sess);   
            }
        }
    }
    
    return rc; 
}

int32
vrrp_if_down(tbl_interface_t *p_db_if)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    tbl_route_if_t            *p_db_rtif = NULL;
    tbl_route_if_key_t        rtif_key;
    
    VRRP_PTR_CHECK(p_db_if);

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
    if (!p_db_rtif)
    {
        return PM_E_NONE;       
    }

    if (0 == p_db_rtif->vrrp_session_num)
    {
        return PM_E_NONE;    
    }

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        VRRP_LOG_ERR("Tbl_vrrp_session is not exist");    
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if (p_db_sess->shutdown_flag)
        {
            continue;
        }

        if (p_db_sess->ifindex == p_db_if->ifindex)
        {
            if (p_db_sess->enable)
            {
                vrrp_shutdown_sess(p_db_sess);   
            }
        }
    }
    
    return rc; 
}

int32
vrrp_ip_address_interface_del(tbl_route_if_t *p_db_rtif)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    prefix_t                  vip_prefix;

    VRRP_PTR_CHECK(p_db_rtif);

    sal_memset(&vip_prefix, 0, sizeof(prefix_t));

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        VRRP_LOG_ERR("Tbl_vrrp_session is not exist");    
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if (p_db_sess->ifindex != p_db_rtif->ifindex)
        {
            continue;
        }
        
        if (p_db_sess->state != VRRP_STATE_INIT)
        {
            VRRP_LOG_DEBUG(RTDBG_FLAG_VRRP_EVENTS, "VRRP Event: primary IP address deleted; shutdown vrid %d", 
                p_db_sess->key.vrid);

            vrrp_shutdown_sess (p_db_sess);                     
        }

        if (p_db_sess->owner)
        {
            p_db_sess->owner = FALSE;   
        }

        p_db_sess->admin_state = VRRP_ADMIN_STATE_DOWN;
        vrrp_reset_virtual_ip(p_db_sess);

        p_db_sess->enable = FALSE;
        tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_ENABLE);

        p_db_sess->ifindex = GLB_INVALID_IFINDEX;
        sal_memset(p_db_sess->ifname, 0, IFNAME_SIZE);
    
        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_IFNAME);
        if (rc < 0)
        {
            VRRP_LOG_ERR("clear vrrp session ifname fail");
        }
    
        rc = tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_IFINDEX);
        if (rc < 0)
        {
            VRRP_LOG_ERR("clear vrrp session ifindex fail");
        }    
    }
    
    return rc;
}

uint32
vrrp_ip_address_check(tbl_route_if_t *p_db_rtif)
{
    tbl_vrrp_session_master_t *p_master = NULL;
    uint32                    vrid = 0;
    tbl_vrrp_session_t        *p_db_sess = NULL;
    prefix_t                  vip_prefix;

    VRRP_PTR_CHECK(p_db_rtif);

    sal_memset(&vip_prefix, 0, sizeof(prefix_t));

    p_master = tbl_vrrp_session_get_master();
    if (!p_master)
    {
        VRRP_LOG_ERR("Tbl_vrrp_session is not exist");    
    }

    for (vrid = 1; vrid <= GLB_VRRP_MAX_VRID; vrid++)
    {
        p_db_sess = p_master->session_array[vrid];
        if (!p_db_sess)
        {
            continue;
        }

        if (p_db_sess->ifindex != p_db_rtif->ifindex)
        {
            continue;
        }

        if (p_db_sess->vip_status)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

uint8
vrrp_adjust_priority (tbl_vrrp_session_t   *p_db_sess)
{
    /* Prio is not yet set.  */
    if (p_db_sess->priority < 0)
        return p_db_sess->priority;

    if (p_db_sess->config_priority == VRRP_UNSET)
    {
        if (p_db_sess->owner)
        {
            return VRRP_DEFAULT_IP_OWNER_PRIORITY;
        }
        else
        {
            return (GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY >= p_db_sess->priority_delta
                  ? GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY - p_db_sess->priority_delta : 0);
        }
    }
    else
    {
        if (p_db_sess->owner)
        {
            return VRRP_DEFAULT_IP_OWNER_PRIORITY;
        }
        else
        {
            return (p_db_sess->config_priority >= p_db_sess->priority_delta
                  ? p_db_sess->config_priority - p_db_sess->priority_delta : 0);
        }
    }
    return p_db_sess->priority;
}

int32
vrrp_api_set_vrrp_priority_bytrack(u_int16_t vrid, uint8 track_state)
{
    int32                     rc = PM_E_NONE;
    tbl_vrrp_session_key_t    sess_vrid;
    tbl_vrrp_session_t       *p_db_sess = NULL;

    sess_vrid.vrid = vrid;
    p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess_vrid);
    if (!p_db_sess)
    {
        return PM_E_NONE;
    }

    p_db_sess->trackobj_status = track_state;

    if(track_state == TRACK_STATE_UP)
    {
        /* Revert back configured priority. */
        if (p_db_sess->config_priority == VRRP_UNSET)
        {
            if (p_db_sess->owner)
            {
                p_db_sess->priority = VRRP_DEFAULT_IP_OWNER_PRIORITY;
            }
            else
            {
                p_db_sess->priority = GLB_VRRP_DEFAULT_NON_IP_OWNER_PRIORITY;
            }
        }
        else
        {
            if (p_db_sess->owner)
            {
                p_db_sess->priority= VRRP_DEFAULT_IP_OWNER_PRIORITY;
            }
            else
            {
                p_db_sess->priority = p_db_sess->config_priority;
            }
        }
    }
    else
    {
        /* Adjust priority. */
        p_db_sess->priority = vrrp_adjust_priority(p_db_sess);
    }

    return rc;
}

int32
vrrp_disable_track_object(tbl_vrrp_session_t  *p_db_sess)
{
    int32      rc = PM_E_NONE;
    p_db_sess->trackobj_id     = 0;
    p_db_sess->priority_delta  = VRRP_DEFAULT_DELTA_PRIORITY;
    p_db_sess->trackobj_status = 0;

    PM_E_RETURN(tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_TRACKOBJ_ID));
    PM_E_RETURN(tbl_vrrp_session_set_vrrp_session_field(p_db_sess, TBL_VRRP_SESSION_FLD_PRIORITY_DELTA));

    vrrp_api_set_vrrp_priority_bytrack(p_db_sess->key.vrid, TRACK_STATE_UP);

    return rc;
}


