
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "track.h"
#include "track_api.h"
#include "ipsla.h"
#include "vrrp.h"
#include "vrrp_api.h"
#include "route.h"


uint32
track_debug_is_on(uint32 flag)
{
    tbl_route_debug_t *p_db_glb = NULL;

    p_db_glb = tbl_route_debug_get_route_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->track, flag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
track_static_addrt_cmdstr(tbl_static_route_cfg_t  *p_db_static_route)
{
    int32  rc   = PM_E_NONE;
    char   cmd_str[CMD_BUF_256];
    char   dest[CMD_BUF_32];
    char   gate[CMD_BUF_32];
    addr_ipv4_t     tmp_zero;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(dest, 0, sizeof(dest));
    sal_memset(gate, 0, sizeof(gate));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));

    cdb_addr_ipv4_val2str(dest, CMD_BUF_32, &(p_db_static_route->key.p.u.prefix4));
    cdb_addr_ipv4_val2str(gate, CMD_BUF_32, &(p_db_static_route->key.nh_addr.ipv4));

    if (1 != p_db_static_route->distance)
    {
        if (0 != sal_memcmp(&tmp_zero, &p_db_static_route->key.nh_addr.ipv4, sizeof(tmp_zero)))
        {
            sal_sprintf(cmd_str, "ip route %s/%d %s %d",
                dest, p_db_static_route->key.p.prefixlen,
                gate, p_db_static_route->distance);
        }
        else
        {
            sal_sprintf(cmd_str, "ip route %s/%d null0 %d",
                dest, p_db_static_route->key.p.prefixlen,
                p_db_static_route->distance);

            sal_memset(gate, 0, sizeof(gate));
            sal_strcpy(gate, "null0");
        }
    }
    else
    {
        if (0 != sal_memcmp(&tmp_zero, &p_db_static_route->key.nh_addr.ipv4, sizeof(tmp_zero)))
        {
            sal_sprintf(cmd_str, "ip route %s/%d %s",
                dest, p_db_static_route->key.p.prefixlen, gate);
        }
        else
        {
            sal_sprintf(cmd_str, "ip route %s/%d null0",
                dest, p_db_static_route->key.p.prefixlen);

            sal_memset(gate, 0, sizeof(gate));
            sal_strcpy(gate, "null0");
        }
    }
    rt_vty_config_cmd(cmd_str);

    TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: Set up for ip %s/%d nh %s\n", 
                    dest, p_db_static_route->key.p.prefixlen, gate);
    logid_sys(LOG_TRACK_4_STATIC_RT_UPDATE, "up", dest, p_db_static_route->key.p.prefixlen, gate);

    return rc;
}

int32
track_static_delrt_cmdstr(tbl_static_route_cfg_t  *p_db_static_route)
{
    int32  rc   = PM_E_NONE;
    char   cmd_str[CMD_BUF_256];
    char   dest[CMD_BUF_32];
    char   gate[CMD_BUF_32];
    prefix_t gate_tmp = {0};
    addr_ipv4_t     tmp_zero;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(dest, 0, sizeof(dest));
    sal_memset(gate, 0, sizeof(gate));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));

    cdb_addr_ipv4_val2str(dest, CMD_BUF_32, &(p_db_static_route->key.p.u.prefix4));
    cdb_addr_ipv4_val2str(gate, CMD_BUF_32, &(p_db_static_route->key.nh_addr.ipv4));

    if (0 != sal_memcmp(&tmp_zero, &p_db_static_route->key.nh_addr.ipv4, sizeof(tmp_zero)))
    {
        sal_sprintf(cmd_str, "no ip route %s/%d %s", 
            dest, p_db_static_route->key.p.prefixlen, gate);
    }
    else
    {
        sal_sprintf(cmd_str, "no ip route %s/%d null0", 
            dest, p_db_static_route->key.p.prefixlen);

        sal_memset(gate, 0, sizeof(gate));
        sal_strcpy(gate, "null0");
    }
    rt_vty_config_cmd(cmd_str);

    sal_memset(&gate_tmp, 0, sizeof(gate_tmp));
    gate_tmp.family = p_db_static_route->key.p.family;
    sal_memcpy(&gate_tmp.u.prefix4, &(p_db_static_route->key.nh_addr.ipv4), sizeof(addr_ipv4_t));
    rt_delete_recursive_static_route(&(p_db_static_route->key.p), &gate_tmp);

    TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: Set down for ip %s/%d nh %s\n", 
                    dest, p_db_static_route->key.p.prefixlen, gate);
    logid_sys(LOG_TRACK_4_STATIC_RT_UPDATE, "down", dest, p_db_static_route->key.p.prefixlen, gate);

    return rc;
}

int32
track_update_static_route(tbl_track_object_t *p_db_object, uint8 track_del)
{
    int32  rc   = PM_E_NONE;
    tbl_static_route_cfg_t  *p_db_static_route = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    if (NULL == p_master)
    {
        return rc;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_static_route, listnode, next)
    {
        if (NULL == p_db_static_route)
        {
            continue;
        }

        if (p_db_static_route->track_id == p_db_object->key.trackobj_id)
        {
            if (track_del)
            {
                if (p_db_static_route->track_status == TRACK_STATE_DOWN)
                {
#if 0
                    track_build_static_addrt_cmdstr(cmd_str, &static_rt_update_cnt, p_db_static_route);
#else
                    track_static_addrt_cmdstr(p_db_static_route);
#endif
                }

                p_db_static_route->track_id = 0;
                PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID));

                p_db_static_route->track_status = TRACK_STATE_UP;
                PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS));

                continue;
            }

            if (p_db_static_route->track_status == p_db_object->state)
            {
                continue;
            }

            if (p_db_object->state == TRACK_STATE_DOWN)
            {
#if 0
                track_build_static_delrt_cmdstr(cmd_str, &static_rt_update_cnt, p_db_static_route);
#else
                track_static_delrt_cmdstr(p_db_static_route);
#endif
                p_db_static_route->track_status = p_db_object->state;
                PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS));
            }
            else
            {
#if 0
                track_build_static_addrt_cmdstr(cmd_str, &static_rt_update_cnt, p_db_static_route);
#else
                track_static_addrt_cmdstr(p_db_static_route);
#endif
                p_db_static_route->track_status = p_db_object->state;
                PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS));
            }
        }
    }

    return rc;
}

int32
track_ipv4_route_unset(tbl_static_route_cfg_key_t *p_rst_cfg_key, cfg_cmd_para_t *para)
{
    int32  rc   = PM_E_NONE;
    tbl_track_object_key_t   track_key;
    tbl_track_object_t      *p_db_object = NULL;
    tbl_static_route_cfg_t  *p_db_static_route = NULL;

    p_db_static_route = tbl_static_route_cfg_get_static_route_cfg(p_rst_cfg_key);
    if (NULL == p_db_static_route)
    {
        CFG_CONFLICT_RET("The static route is not found");
    }

    track_key.trackobj_id = p_db_static_route->track_id;
    p_db_object = tbl_track_object_get_track_object(&track_key);
    if (NULL == p_db_object)
    {
        CFG_PROMPT_RET("Notice : Track object %u is not existed", p_db_static_route->track_id);
    }

    if (p_db_object->static_rt_refcnt > 0)
    {
        p_db_object->static_rt_refcnt--;
        PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT));
    }

    p_db_static_route->track_id = 0;
    PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID));

    p_db_static_route->track_status = TRACK_STATE_UP;
    PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS));

    return rc;
}

int32
track_ipv4_route_set(tbl_static_route_cfg_key_t *p_rst_cfg_key, uint16 track_id, uint16 *is_add, cfg_cmd_para_t *para)
{
    int32  rc   = PM_E_NONE;
    tbl_track_object_key_t   track_key;
    tbl_track_object_t      *p_db_object = NULL;
    tbl_track_object_t      *p_db_oldobject = NULL;
    tbl_static_route_cfg_t  *p_db_static_route = NULL;

    p_db_static_route = tbl_static_route_cfg_get_static_route_cfg(p_rst_cfg_key);
    if (NULL == p_db_static_route)
    {
        CFG_CONFLICT_RET("The static route is not found");
    }

    track_key.trackobj_id = track_id;
    p_db_object = tbl_track_object_get_track_object(&track_key);
    if (NULL == p_db_object)
    {
        CFG_PROMPT_RET("Notice : Track object %u is not existed", track_id);
    }

    if (TRACK_RTR_REACHABILITY != p_db_object->type && TRACK_RTR_STATE != p_db_object->type )
    {
        CFG_PROMPT_RET("Notice : Unsupport this kind of track");
    }

    if (p_db_static_route->track_id != track_id)
    {
        if (p_db_object->static_rt_refcnt >= TRACK_MAX_STATIC_RT_REFCNT)
        {
            CFG_CONFLICT_RET("Can not bind more than 64 static route with single track object %u", track_id);
        }

        p_db_object->static_rt_refcnt++;
        PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT));

        if (0 != p_db_static_route->track_id)
        {
            track_key.trackobj_id = p_db_static_route->track_id;
            p_db_oldobject = tbl_track_object_get_track_object(&track_key);
            if (NULL == p_db_oldobject)
            {
                CFG_PROMPT_RET("Notice : Track object %u is not existed", p_db_static_route->track_id);
            }

            if (p_db_oldobject->static_rt_refcnt > 0)
            {
                p_db_oldobject->static_rt_refcnt--;
                PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_oldobject, TBL_TRACK_OBJECT_FLD_STATIC_RT_REFCNT));
            }
        }
    }

    p_db_static_route->track_id = track_id;
    PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_ID));

    p_db_static_route->track_status = p_db_object->state;
    PM_E_RETURN(tbl_static_route_cfg_set_static_route_cfg_field(p_db_static_route, TBL_STATIC_ROUTE_CFG_FLD_TRACK_STATUS));

    if (p_db_object->state == TRACK_STATE_DOWN)
    {
        *is_add = 0;
    }

    return rc;
}

int32
track_update_vrrp_priority(tbl_track_object_t *p_db_object)
{
    int32  rc   = PM_E_NONE;
    uint32 vrid = 0;
    uint32 tracked_vrrp_bmp_zero = TRUE;

    GLB_BMP_CHECK_ALL_ZERO(p_db_object->tracked_vrid_bmp, tracked_vrrp_bmp_zero);
    if(tracked_vrrp_bmp_zero)
    {
        return rc;
    }

    GLB_BMP_ITER_BEGIN(p_db_object->tracked_vrid_bmp, vrid)
    {
        if (GLB_BMP_ISSET(p_db_object->tracked_vrid_bmp, vrid))
        {
            PM_E_RETURN(vrrp_api_set_vrrp_priority_bytrack(vrid, p_db_object->state));
        }
    }
    GLB_BMP_ITER_END(p_db_object->tracked_vrid_bmp, vrid);

    return rc;
}

int32
track_del_tracked_vrrp(u_int16_t trackobj_id, int32 vrid, cfg_cmd_para_t *para)
{
    tbl_track_object_key_t   key;
    tbl_track_object_t      *p_db_object = NULL;

    key.trackobj_id = trackobj_id;
    p_db_object = tbl_track_object_get_track_object(&key);
    if (!p_db_object)
    {
        CFG_CONFLICT_RET("Track object %u does not exist", trackobj_id);
    }

    if (GLB_BMP_ISSET(p_db_object->tracked_vrid_bmp, vrid))
    {
        GLB_BMP_UNSET(p_db_object->tracked_vrid_bmp, vrid);
    }

    return PM_E_NONE;
}

int32
track_add_tracked_vrrp(u_int16_t trackobj_id, int32 vrid, cfg_cmd_para_t *para)
{
    tbl_track_object_key_t   key;
    tbl_track_object_t      *p_db_object = NULL;

    key.trackobj_id = trackobj_id;
    p_db_object = tbl_track_object_get_track_object(&key);
    if (!p_db_object)
    {
        CFG_CONFLICT_RET("Track object %u does not exist", trackobj_id);
    }

    if (!GLB_BMP_ISSET(p_db_object->tracked_vrid_bmp, vrid))
    {
        GLB_BMP_SET(p_db_object->tracked_vrid_bmp, vrid);
    }

    vrrp_api_set_vrrp_priority_bytrack(vrid, p_db_object->state);
    return PM_E_NONE;
}

int32
track_delay_up_process(tbl_track_object_t  *p_db_object, u_int16_t  track_timer_interval)
{
    if (p_db_object->delay_up != 0)
    {
        if (p_db_object->running_delay_up == 0)
        {
            p_db_object->running_delay_up = p_db_object->delay_up;
        }
        else if (p_db_object->running_delay_up < track_timer_interval)
        {
              p_db_object->running_delay_up = 0;
              return  TRACK_DELAY_UP_END;
        }
        else
        {
            p_db_object->running_delay_up = p_db_object->running_delay_up - track_timer_interval;
        }

        if (p_db_object->running_delay_up != 0)
            return TRACK_DELAY_UP_CONTINUE;
    }

    return TRACK_DELAY_UP_END;
}

int32
track_delay_down_process(tbl_track_object_t  *p_db_object, u_int16_t  track_timer_interval)
{
    if (p_db_object->delay_down!=0)
    {
        if (p_db_object->running_delay_down == 0)
        {
            p_db_object->running_delay_down = p_db_object->delay_down;
        }
        else if (p_db_object->running_delay_down < track_timer_interval)
        {
              p_db_object->running_delay_down = 0;
              return  TRACK_DELAY_DOWN_END;
        }
        else
        {
            p_db_object->running_delay_down = p_db_object->running_delay_down - track_timer_interval;
        }

        if (p_db_object->running_delay_down != 0)
            return TRACK_DELAY_DOWN_CONTINUE;
    }

    return TRACK_DELAY_DOWN_END;
}

int32
track_update_interface_status(tbl_track_object_t  *p_db_object, int32 if_state)
{
    int32                  rc = PM_E_NONE;
    tbl_track_global_t    *p_db_track_glb = NULL;
    tbl_interface_t        intf;
    tbl_interface_t       *p_db_if = NULL;

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        return rc;
    }

    if (!p_db_object->track_if)
    {
        return rc;
    }

    sal_memset(&intf, 0, sizeof(tbl_interface_t));
    sal_strncpy(intf.key.name, p_db_object->track_if, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&intf.key);

    if ((!p_db_if) || (if_state == TRACK_IF_STATE_DELETE))
    {
        TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: Interface %s is not exist, track %d is deleted\n",
                p_db_object->track_if, p_db_object->key.trackobj_id);

        rc = track_delete_object(p_db_track_glb, p_db_object);
        return rc;
    }

    if ((tbl_interface_is_running(p_db_if)) && (p_db_object->state == TRACK_STATE_DOWN))
    {
        /* delay up */
        if (TRACK_DELAY_UP_CONTINUE == track_delay_up_process(p_db_object, p_db_track_glb->track_timer_interval))
        {
            return rc;
        }
        /* update object status */
        p_db_object->state = TRACK_STATE_UP;
        TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to up for track %d\n", p_db_object->key.trackobj_id);
        logid_sys(LOG_TRACK_4_TRACK_EVENT, "up", p_db_object->key.trackobj_id);

        PM_E_RETURN(track_update_vrrp_priority(p_db_object));
    }
    else if ((!tbl_interface_is_running(p_db_if)) && (p_db_object->state == TRACK_STATE_UP))
    {
        /* delay down */
        if(TRACK_DELAY_DOWN_CONTINUE == track_delay_down_process(p_db_object, p_db_track_glb->track_timer_interval))
        {
            return rc;
        }

        p_db_object->state = TRACK_STATE_DOWN;
        TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to down for track %d\n", p_db_object->key.trackobj_id);
        logid_sys(LOG_TRACK_4_TRACK_EVENT, "down", p_db_object->key.trackobj_id);

        PM_E_RETURN(track_update_vrrp_priority(p_db_object));
    }

    if (p_db_object->running_delay_down != 0)
    {
        p_db_object->running_delay_down = 0;
    }

    if (p_db_object->running_delay_up != 0)
    {
        p_db_object->running_delay_up = 0;
    }

    return rc;
}

int32
track_update_rtr_status(tbl_track_object_t  *p_db_object)
{
    int32                    rc = PM_E_NONE;
    tbl_track_global_t      *p_db_track_glb = NULL;
    tbl_ipsla_entry_key_t    ipsla_entry;
    tbl_ipsla_entry_t       *p_db_entry  = NULL;

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        return rc;
    }

    /* lookup ipsla entry */
    sal_memset(&ipsla_entry, 0, sizeof(tbl_ipsla_entry_key_t));
    ipsla_entry.entry_id = p_db_object->rtr_entryid;

    p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&ipsla_entry);
    if (!p_db_entry)
    {
        TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: Ip sla entry %d is not exist, track %d is deleted\n",
                ipsla_entry.entry_id, p_db_object->key.trackobj_id);

        track_update_static_route(p_db_object, 1);
        rc = track_delete_object(p_db_track_glb, p_db_object);
        return rc;
    }

    if (p_db_object->type == TRACK_RTR_REACHABILITY)
    {
        if ((p_db_object->state == TRACK_STATE_DOWN)
           &&((p_db_entry->returncode == ECHO_OK)||(p_db_entry->returncode == ECHO_OVERTHRESHOLD)))
        {
            /* delay up */
            if(TRACK_DELAY_UP_CONTINUE == track_delay_up_process(p_db_object, p_db_track_glb->track_timer_interval))
            {
                return rc;
            }

            /* update object status */
            p_db_object->state = TRACK_STATE_UP;
            TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to up for track %d\n", p_db_object->key.trackobj_id);
            logid_sys(LOG_TRACK_4_TRACK_EVENT, "up", p_db_object->key.trackobj_id);

            PM_E_RETURN(track_update_vrrp_priority(p_db_object));
            track_update_static_route(p_db_object, 0);
        }
        else if ((p_db_object->state == TRACK_STATE_UP)
           &&((p_db_entry->returncode != ECHO_OK)&&(p_db_entry->returncode != ECHO_OVERTHRESHOLD)))
        {
            /* delay down */
            if(TRACK_DELAY_DOWN_CONTINUE == track_delay_down_process(p_db_object, p_db_track_glb->track_timer_interval))
            {
                return rc;
            }

            p_db_object->state = TRACK_STATE_DOWN;
            TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to down for track %d\n", p_db_object->key.trackobj_id);
            logid_sys(LOG_TRACK_4_TRACK_EVENT, "down", p_db_object->key.trackobj_id);

            PM_E_RETURN(track_update_vrrp_priority(p_db_object));
            track_update_static_route(p_db_object, 0);
        }
    }
    else if(p_db_object->type == TRACK_RTR_STATE)
    {
        if ((p_db_object->state == TRACK_STATE_DOWN)&&(p_db_entry->returncode == ECHO_OK))
        {
            /* delay up */
            if (TRACK_DELAY_UP_CONTINUE == track_delay_up_process(p_db_object, p_db_track_glb->track_timer_interval))
            {
                return rc;
            }

            /* update object status */
            p_db_object->state = TRACK_STATE_UP;
            TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to up for track %d\n", p_db_object->key.trackobj_id);
            logid_sys(LOG_TRACK_4_TRACK_EVENT, "up", p_db_object->key.trackobj_id);

            PM_E_RETURN(track_update_vrrp_priority(p_db_object));
            track_update_static_route(p_db_object, 0);
        }
        else if ((p_db_object->state == TRACK_STATE_UP)&&(p_db_entry->returncode != ECHO_OK))
        {
            /* delay down */
            if(TRACK_DELAY_DOWN_CONTINUE == track_delay_down_process(p_db_object, p_db_track_glb->track_timer_interval))
            {
                return rc;
            }

            p_db_object->state = TRACK_STATE_DOWN;
            TRACK_LOG_DEBUG(RTDBG_FLAG_TRACK_EVENT, "TRACK Event: State change to down for track %d\n", p_db_object->key.trackobj_id);
            logid_sys(LOG_TRACK_4_TRACK_EVENT, "down", p_db_object->key.trackobj_id);

            PM_E_RETURN(track_update_vrrp_priority(p_db_object));
            track_update_static_route(p_db_object, 0);
        }
    }

    if (p_db_object->running_delay_down!=0)
        p_db_object->running_delay_down =0;

    if (p_db_object->running_delay_up!=0)
        p_db_object->running_delay_up =0;

    return rc;
}

int32
track_update_object_status(tbl_track_object_t  *p_db_object)
{
    int32  rc = PM_E_NONE;

    if (p_db_object->type == TRACK_INTERFACE_LINKSTATE)
    {
        PM_E_RETURN(track_update_interface_status(p_db_object, TRACK_IF_STATE_UNKNOWN));
    }
    else if ((p_db_object->type == TRACK_RTR_REACHABILITY)
          || (p_db_object->type == TRACK_RTR_STATE))
    {
        PM_E_RETURN(track_update_rtr_status(p_db_object));
    }

    return rc;
}

int32
track_quick_update_interface_status()
{
    int32  rc = PM_E_NONE;
    tbl_track_global_t     *p_db_track_glb = NULL;

    p_db_track_glb = tbl_track_global_get_track_global();
    if (!p_db_track_glb)
    {
        return PM_E_NOT_EXIST;
    }

    track_timer(p_db_track_glb);

    return rc;
}

void
track_timer(tbl_track_global_t *p_db_track_glb)
{
    uint32     object_id      = 0;
    tbl_track_object_t *p_db_object = NULL;
    tbl_track_object_master_t *p_master  = NULL;
    uint16     track_object_num = p_db_track_glb->track_object_num;

    p_master = tbl_track_object_get_master();
    if (!p_master)
    {
        return;
    }

    /* Loop through all objects */
    for (object_id = 1; object_id <= GLB_TRACK_OBJECT_MAX && track_object_num > 0; object_id++)
    {
        p_db_object = p_master->trackobj_array[object_id];
        if (!p_db_object)
        {
            continue;
        }
        else
        {
            track_object_num--;
            track_update_object_status(p_db_object);
        }
    }

    return;
}

void
track_sys_timer_update(void *p_data)
{
    tbl_track_global_t *p_db_track_glb = (tbl_track_global_t*)p_data;

    if (!p_db_track_glb)
    {
        return;
    }

    p_db_track_glb->t_track_timer = NULL;

    track_timer(p_db_track_glb);

    p_db_track_glb->t_track_timer = CTC_TASK_ADD_TIME(track_sys_timer_update, p_db_track_glb, 1);

    return;
}

int32
track_sys_init_timer(tbl_track_global_t *p_db_track_glb, uint32 start)
{
    if (NULL == p_db_track_glb)
    {
        return PM_E_INVALID_PARAM;
    }

    if (p_db_track_glb->t_track_timer)
    {
        CTC_TASK_DEL_TIME(p_db_track_glb->t_track_timer);
    }

    if (start)
    {
        p_db_track_glb->t_track_timer = CTC_TASK_ADD_TIME(track_sys_timer_update,
                        p_db_track_glb, p_db_track_glb->track_timer_interval);
    }

    return PM_E_NONE;
}

int32
track_delete_object(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object)
{
    int32  rc = PM_E_NONE;
    uint32 vrid = 0;
    tbl_vrrp_session_t  *p_db_sess = NULL;
    tbl_vrrp_session_key_t sess;

    track_update_static_route(p_db_object, 1);

    GLB_BMP_ITER_BEGIN(p_db_object->tracked_vrid_bmp, vrid)
    {
        if (GLB_BMP_ISSET(p_db_object->tracked_vrid_bmp, vrid))
        {
            sal_memset(&sess, 0, sizeof(sess));
            sess.vrid = vrid;
            p_db_sess = tbl_vrrp_session_get_vrrp_session(&sess);
            if (p_db_sess)
            {
                PM_E_RETURN(vrrp_disable_track_object(p_db_sess));
            }
        }
    }
    GLB_BMP_ITER_END(p_db_object->tracked_vrid_bmp, vrid);

    PM_E_RETURN(tbl_track_object_del_track_object(&p_db_object->key));

    p_db_track_glb->track_object_num--;
    PM_E_RETURN(tbl_track_global_set_track_global_field(p_db_track_glb, TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM));

    if (0 == p_db_track_glb->track_object_num)
    {
        /* stop track timer */
        track_sys_init_timer(p_db_track_glb, FALSE);
    }
    return rc;
}

int32
track_create_object_intf(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object,
        char *track_intf, uint32 object_id, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    tbl_track_object_t     object;
    tbl_interface_t        intf;
    tbl_interface_t       *p_db_if = NULL;

    sal_memset(&intf, 0, sizeof(tbl_interface_t));
    sal_strncpy(intf.key.name, track_intf, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&intf.key);
    if (!p_db_if)
    {
        CFG_CONFLICT_RET("Not found interface %s", track_intf);
    }

    if (!p_db_object)
    {
        object.key.trackobj_id = object_id;
        object.type = TRACK_INTERFACE_LINKSTATE;
        object.rtr_entryid = 0;
        object.delay_up    = 0;
        object.running_delay_up   = 0;
        object.delay_down  = 0;
        object.running_delay_down = 0;
        sal_memset(object.track_if, 0, IFNAME_SIZE);
        GLB_BMP_INIT(object.tracked_vrid_bmp);
        object.static_rt_refcnt = 0;

        sal_strncpy(object.track_if, track_intf, IFNAME_SIZE);

        if (tbl_interface_is_running(p_db_if))
        {
            object.state = TRACK_STATE_UP;
        }
        else
        {
            object.state = TRACK_STATE_DOWN;
        }

        PM_E_RETURN(tbl_track_object_add_track_object(&object));

        track_update_vrrp_priority(&object);

        if (0 == p_db_track_glb->track_object_num && !p_db_track_glb->t_track_timer)
        {
            p_db_track_glb->track_timer_interval = TRACK_TIMER_INTERVAL_DEFAULT;
            track_sys_init_timer(p_db_track_glb, TRUE);
        }

        p_db_track_glb->track_object_num++;
        PM_E_RETURN(tbl_track_global_set_track_global_field(p_db_track_glb, TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM));
    }
    else
    {
        if (p_db_object->type != TRACK_INTERFACE_LINKSTATE)
        {
            CFG_CONFLICT_RET("Cannot change tracked object - delete old config first");
        }

        sal_strncpy(p_db_object->track_if, track_intf, IFNAME_SIZE);
        PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_TRACK_IF));
        if (tbl_interface_is_running(p_db_if))
        {
            p_db_object->state = TRACK_STATE_UP;
        }
        else
        {
            p_db_object->state = TRACK_STATE_DOWN;
        }
        track_update_vrrp_priority(p_db_object);
    }

    return rc;
}

int32
track_create_object_rtr(tbl_track_global_t *p_db_track_glb, tbl_track_object_t *p_db_object,
        char *rtr_type, uint32 rtr_entry_id, uint32 object_id, cfg_cmd_para_t *para)
{
    int32                     rc = PM_E_NONE;
    uint32         rtr_type_code = TRACK_UNCONFIG;
    tbl_track_object_t        object;
    tbl_ipsla_entry_key_t     ipsla_entry;
    tbl_ipsla_entry_t        *p_db_entry  = NULL;

    sal_memset(&ipsla_entry, 0, sizeof(tbl_ipsla_entry_key_t));
    ipsla_entry.entry_id = rtr_entry_id;

    p_db_entry = tbl_ipsla_entry_get_ipsla_entry(&ipsla_entry);
    if (!p_db_entry)
    {
        CFG_CONFLICT_RET("IP sla entry not exist");
    }

    if (!sal_strcmp(rtr_type, "reachability"))
    {
        rtr_type_code = TRACK_RTR_REACHABILITY;
    }
    else if (!sal_strcmp(rtr_type, "state"))
    {
        rtr_type_code = TRACK_RTR_STATE;
    }

    if (!p_db_object)
    {
        object.key.trackobj_id = object_id;
        object.type = rtr_type_code;
        object.rtr_entryid = rtr_entry_id;
        object.delay_up   = 0;
        object.running_delay_up   = 0;
        object.delay_down = 0;
        object.running_delay_down = 0;
        sal_memset(object.track_if, 0, IFNAME_SIZE);
        GLB_BMP_INIT(object.tracked_vrid_bmp);
        object.static_rt_refcnt = 0;

        if ((object.type == TRACK_RTR_REACHABILITY)
            && ((p_db_entry->returncode == ECHO_OK) || (p_db_entry->returncode == ECHO_OVERTHRESHOLD)))
        {
            object.state = TRACK_STATE_UP;
        }
        else if ((object.type == TRACK_RTR_STATE) && (p_db_entry->returncode == ECHO_OK))
        {
            object.state = TRACK_STATE_UP;
        }
        else
        {
            object.state = TRACK_STATE_DOWN;
        }

        PM_E_RETURN(tbl_track_object_add_track_object(&object));

        track_update_vrrp_priority(&object);

        if (0 == p_db_track_glb->track_object_num && !p_db_track_glb->t_track_timer)
        {
            p_db_track_glb->track_timer_interval = TRACK_TIMER_INTERVAL_DEFAULT;
            track_sys_init_timer(p_db_track_glb, TRUE);
        }

        p_db_track_glb->track_object_num++;
        PM_E_RETURN(tbl_track_global_set_track_global_field(p_db_track_glb, TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM));
    }
    else
    {
        if (p_db_object->type != TRACK_RTR_REACHABILITY && p_db_object->type != TRACK_RTR_STATE)
        {
            CFG_CONFLICT_RET("Cannot change tracked object - delete old config first");
        }

        p_db_object->type = rtr_type_code;
        PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_TYPE));

        p_db_object->rtr_entryid = rtr_entry_id;
        PM_E_RETURN(tbl_track_object_set_track_object_field(p_db_object, TBL_TRACK_OBJECT_FLD_RTR_ENTRYID));

        if ((p_db_object->type == TRACK_RTR_REACHABILITY)
            && ((p_db_entry->returncode == ECHO_OK) || (p_db_entry->returncode == ECHO_OVERTHRESHOLD)))
        {
            p_db_object->state = TRACK_STATE_UP;
        }
        else if ((p_db_object->type == TRACK_RTR_STATE) && (p_db_entry->returncode == ECHO_OK))
        {
            p_db_object->state = TRACK_STATE_UP;
        }
        else
        {
            p_db_object->state = TRACK_STATE_DOWN;
        }

        track_update_vrrp_priority(&object);
        track_update_static_route(p_db_object, 0);
    }

    return rc;
}

int32
track_start()
{
    cdb_register_cfg_tbl_cb(TBL_TRACK_GLOBAL, track_cmd_process_global);
    cdb_register_cfg_tbl_cb(TBL_TRACK_OBJECT, track_cmd_process_object);

    cdb_register_cfg_act_cb(ACT_SHOW_TRACK, track_cmd_show_track);
    return PM_E_NONE;
}


