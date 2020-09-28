
#include "proto.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h"
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_group_define.h"
#include "if_agg.h"
#include "switch.h"
#include "mirror_define.h"
#include "mirror.h"
#include "switch_api.h"
#include "igsp_api.h"

#ifdef CONFIG_MIRROR

int32
mirror_create_session(uint32 sessionid)
{
    tbl_mirror_t mirror;
    tbl_mirror_t *p_mirror = NULL;
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    int rc = PM_E_NONE;

    /* 1. check exist */
    mirror_key.id = sessionid;
    p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
    if (NULL != p_db_mirror)
    {
        return PM_E_NONE;
    }

    /* 2. create new session */
    p_mirror = &mirror;
    sal_memset(p_mirror, 0, sizeof(tbl_mirror_t));
    p_mirror->key.id = sessionid;
    rc = tbl_mirror_add_mirror(p_mirror);

    GLB_BMP_INIT(p_mirror->source_igs_port_bmp);
    GLB_BMP_INIT(p_mirror->source_egs_port_bmp);
    GLB_BMP_INIT(p_mirror->source_igs_vlan_bmp);
    GLB_BMP_INIT(p_mirror->source_egs_vlan_bmp);
    GLB_BMP_INIT(p_mirror->dest_port_bmp);
    return rc;
}

int32
mirror_destroy_session(uint32 sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 source_igs_port_bmp_zero = TRUE;
    uint32 source_egs_port_bmp_zero = TRUE;
    uint32 source_igs_vlan_bmp_zero = TRUE;
    uint32 source_egs_vlan_bmp_zero = TRUE;

    /* 1. check exist */
    mirror_key.id = sessionid;
    p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
    if (NULL == p_db_mirror)
    {
        return PM_E_NONE;
    }

    /* 2. check source mirror has been used */
    GLB_BMP_CHECK_ALL_ZERO(p_db_mirror->source_igs_port_bmp, source_igs_port_bmp_zero);
    GLB_BMP_CHECK_ALL_ZERO(p_db_mirror->source_egs_port_bmp, source_egs_port_bmp_zero);
    GLB_BMP_CHECK_ALL_ZERO(p_db_mirror->source_igs_vlan_bmp, source_igs_vlan_bmp_zero);
    GLB_BMP_CHECK_ALL_ZERO(p_db_mirror->source_egs_vlan_bmp, source_egs_vlan_bmp_zero);

    if (!source_igs_port_bmp_zero || !source_egs_port_bmp_zero
        || !source_igs_vlan_bmp_zero || !source_egs_vlan_bmp_zero)
    {
        return PM_E_NONE;
    }

    /* 3. check dest mirror has been used */
    if (MIRROR_DEST_NONE != p_db_mirror->dest_type)
    {
        return PM_E_NONE;
    }
    
    /* 4. remove mirror session */ 
    mirror_key.id = sessionid;
    return tbl_mirror_del_mirror(&mirror_key);
}

int32
mirror_add_source_interface_to_session(tbl_mirror_t *p_mirror, uint32 direction, tbl_interface_t *p_db_if, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_SET_FLAG(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG);
        GLB_BMP_SET(p_mirror->source_igs_port_bmp, p_db_if->ifindex);
        p_db_if->mirror_igs_id = p_mirror->key.id;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_IGS_ID);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP);
    }

    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_SET_FLAG(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG);
        GLB_BMP_SET(p_mirror->source_egs_port_bmp, p_db_if->ifindex);
        p_db_if->mirror_egs_id = p_mirror->key.id;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_EGS_ID);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP);
    }
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = mirror_add_source_interface_to_session(p_mirror, direction, p_if_mem, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
mirror_del_source_interface_from_session(tbl_mirror_t *p_mirror, uint32 direction, tbl_interface_t *p_db_if, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_UNSET_FLAG(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG);
        GLB_BMP_UNSET(p_mirror->source_igs_port_bmp, p_db_if->ifindex);
        p_db_if->mirror_igs_id = 0;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_IGS_ID);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP);
    }

    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_UNSET_FLAG(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG);
        GLB_BMP_UNSET(p_mirror->source_egs_port_bmp, p_db_if->ifindex);
        p_db_if->mirror_egs_id = 0;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_EGS_ID);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP);
    }
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = mirror_del_source_interface_from_session(p_mirror, direction, p_if_mem, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
mirror_add_source_vlan_to_session(tbl_mirror_t *p_mirror, uint32 direction, vid_t vlanid)
{
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_BMP_SET(p_mirror->source_igs_vlan_bmp, vlanid);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP);
    }

    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_BMP_SET(p_mirror->source_egs_vlan_bmp, vlanid);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP);
    }

    return PM_E_NONE;
}

int32
mirror_del_source_vlan_from_session(tbl_mirror_t *p_mirror, uint32 direction, vid_t vlanid)
{
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_BMP_UNSET(p_mirror->source_igs_vlan_bmp, vlanid);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP);
    }

    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_BMP_UNSET(p_mirror->source_egs_vlan_bmp, vlanid);
        tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP);
    }
    
    return PM_E_NONE;
}

static int32
_mirror_add_dest_to_session_process(tbl_interface_t *p_db_if)
{
    if (p_db_if->mstp_port)
    {
        swth_mstp_api_del_port(p_db_if);
    }

    /* For bug 47078 by kcao 2018-06-09 clear IGSP group and mrouter-port */
    igsp_api_if_del_cb(p_db_if);
    
    return PM_E_NONE;
}

static int32
_mirror_del_dest_from_session_process(tbl_interface_t *p_db_if)
{
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        swth_mstp_api_add_port(p_db_if);
    }
    return PM_E_NONE;
}

int32
mirror_add_dest_to_session(tbl_mirror_t *p_mirror, tbl_interface_t *p_db_if)
{
    p_db_if->mirror_enable = MIRROR_DEST_FLAG;
    p_db_if->mirror_dest_id = p_mirror->key.id;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);
    
    p_mirror->dest_port    = p_db_if->ifindex;
    p_mirror->dest_type    = MIRROR_DEST_PORT;
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);

    _mirror_add_dest_to_session_process(p_db_if);

    return PM_E_NONE;
}

int32
mirror_del_dest_from_session(tbl_mirror_t *p_mirror, tbl_interface_t *p_db_if)
{
    _mirror_del_dest_from_session_process(p_db_if);
    
    p_db_if->mirror_dest_id = 0;
    p_db_if->mirror_enable = 0;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);

    p_mirror->dest_type    = MIRROR_DEST_NONE;
    p_mirror->dest_port    = 0;
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT);

    return PM_E_NONE; 
}

int32
mirror_add_remote_dest_to_session(tbl_mirror_t *p_mirror, uint32 vlanid, tbl_interface_t *p_db_if)
{
    p_db_if->mirror_enable = MIRROR_DEST_REMOTE_FLAG;
    p_db_if->mirror_dest_id = p_mirror->key.id;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);
    
    p_mirror->dest_port    = p_db_if->ifindex;
    p_mirror->dest_vlan    = vlanid;
    p_mirror->dest_type    = MIRROR_DEST_VLAN;
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_VLAN);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);

    return PM_E_NONE;
}

int32
mirror_del_remote_dest_from_session(tbl_mirror_t *p_mirror, vid_t vlanid, uint32 ifindex)
{
    tbl_interface_t *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    p_db_if->mirror_dest_id = 0;
    p_db_if->mirror_enable = 0;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);
    
    p_mirror->dest_port    = 0;
    p_mirror->dest_vlan    = 0;
    p_mirror->dest_type    = MIRROR_DEST_NONE;
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_VLAN);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT);

    return PM_E_NONE;
}

int32
mirror_add_multi_dest_to_session(tbl_mirror_t *p_mirror, uint32 groupid, tbl_interface_t *p_db_if)    
{
    p_db_if->mirror_enable = MIRROR_DEST_FLAG;
    p_db_if->mirror_dest_id = p_mirror->key.id;

    p_mirror->dest_group   = groupid;
    p_mirror->dest_type    = MIRROR_DEST_GROUP;
    GLB_BMP_SET(p_mirror->dest_port_bmp, p_db_if->ifindex);
    
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_GROUP);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT_BMP);

    _mirror_add_dest_to_session_process(p_db_if);

    return PM_E_NONE;
}

int32
mirror_del_multi_dest_from_session(tbl_mirror_t *p_mirror, uint32 groupid, tbl_interface_t *p_db_if)    
{
    _mirror_del_dest_from_session_process(p_db_if);
    
    if (!GLB_BMP_ISSET(p_mirror->dest_port_bmp, p_db_if->ifindex))
    {
        return PM_E_NONE;
    }
    if (mirror_multi_dest_port_is_the_last_one(p_mirror))
    {
        p_db_if->mirror_enable  = 0;
        p_db_if->mirror_dest_id = 0;
        p_mirror->dest_group    = 0;
        p_mirror->dest_type     = MIRROR_DEST_NONE;
        GLB_BMP_UNSET(p_mirror->dest_port_bmp, p_db_if->ifindex);
    }
    else
    {
        p_db_if->mirror_enable  = 0;
        p_db_if->mirror_dest_id = 0;
        p_mirror->dest_group    = groupid;
        p_mirror->dest_type     = MIRROR_DEST_GROUP;
        GLB_BMP_UNSET(p_mirror->dest_port_bmp, p_db_if->ifindex);
    }

    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MIRROR_DEST_ID);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_GROUP);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_TYPE);
    tbl_mirror_set_mirror_field(p_mirror, TBL_MIRROR_FLD_DEST_PORT_BMP);

    return PM_E_NONE;
}

int32
mirror_del_cpu_dest_from_session(tbl_mirror_t *p_db_mirror)
{
    p_db_mirror->dest_type    = MIRROR_DEST_NONE;
    tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_DEST_TYPE);
    
    return PM_E_NONE;
}

int32
mirror_del_port_process(tbl_interface_t *p_db_if)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }
        
        if (p_db_mirror->dest_port == p_db_if->ifindex)
        {
            /*delete source mirror session first if has source config*/
            if (mirror_has_source_config(p_db_mirror))
            {
                mirror_clear_source_session(p_db_mirror);
            }
            
            if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
            {
                mirror_del_dest_from_session(p_db_mirror, p_db_if);
                mirror_destroy_session(session);
            }

            mirror_key.id = session;
            p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
            if (NULL == p_db_mirror)
            {
                continue;     
            }

            if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
            {
                mirror_del_remote_dest_from_session(p_db_mirror, p_db_mirror->dest_vlan, p_db_if->ifindex);
                mirror_destroy_session(session);
            }
        }
        else if (GLB_BMP_ISSET(p_db_mirror->dest_port_bmp, p_db_if->ifindex))
        {
            /*delete source mirror session first if has source config*/
            if (mirror_multi_dest_port_is_the_last_one(p_db_mirror))
            {
                if (mirror_has_source_config(p_db_mirror))
                {
                    mirror_clear_source_session(p_db_mirror);
                }
            }
            mirror_del_multi_dest_from_session(p_db_mirror, p_db_mirror->dest_group, p_db_if);
            mirror_destroy_session(session);
        }

        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }
        
        if (GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, p_db_if->ifindex))
        {
            mirror_del_source_interface_from_session(p_db_mirror, GLB_MIRROR_DIR_IGS, p_db_if, TRUE);
            mirror_destroy_session(session);
        }

        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_egs_port_bmp, p_db_if->ifindex))
        {
            mirror_del_source_interface_from_session(p_db_mirror, GLB_MIRROR_DIR_EGS, p_db_if, TRUE);
            mirror_destroy_session(session);
        }
    }

    return PM_E_NONE;
}

int32
mirror_del_vlan_process(vid_t vlanid, bool is_vlanif)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32   session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (!is_vlanif)
        {
            if (p_db_mirror->dest_vlan == vlanid)
            {
                /*delete source mirror session first if has source config*/
                if (mirror_has_source_config(p_db_mirror))
                {
                    mirror_clear_source_session(p_db_mirror);
                }
                /*delete remote dest vlan*/
                mirror_del_remote_dest_from_session(p_db_mirror, vlanid, p_db_mirror->dest_port);
                mirror_destroy_session(session);
            }
        }

        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vlanid))
        {
            mirror_del_source_vlan_from_session(p_db_mirror, GLB_MIRROR_DIR_IGS, vlanid);
            mirror_destroy_session(session);
        }

        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vlanid))
        {
            mirror_del_source_vlan_from_session(p_db_mirror, GLB_MIRROR_DIR_EGS , vlanid);
            mirror_destroy_session(session);
        }
    }

    return PM_E_NONE;
}

int32
mirror_clear_source_session(tbl_mirror_t *p_db_mirror)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 port = 0;
    vid_t vlanid = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_port_bmp, port)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(port);
        if (NULL == p_db_if)
        {
            continue;
        }
        mirror_del_source_interface_from_session(p_db_mirror, GLB_MIRROR_DIR_IGS, p_db_if, TRUE);
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_port_bmp, port);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, port)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(port);
        if (NULL == p_db_if)
        {
            continue;
        }

        mirror_del_source_interface_from_session(p_db_mirror, GLB_MIRROR_DIR_EGS, p_db_if, TRUE);
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, port);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_vlan_bmp, vlanid)
    {
        mirror_del_source_vlan_from_session(p_db_mirror, GLB_MIRROR_DIR_IGS, vlanid);
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_vlan_bmp, vlanid);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vlanid)
    {
        mirror_del_source_vlan_from_session(p_db_mirror, GLB_MIRROR_DIR_EGS, vlanid);
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vlanid);

    if (GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS))
    {
        GLB_UNSET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS);
        tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_CPU_SOURCE_DIR);
    }
    if (GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS))
    {
        GLB_UNSET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS);
        tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_CPU_SOURCE_DIR);
    }
    
    return PM_E_NONE;
}

int32
mirror_clear_session(tbl_mirror_t *p_db_mirror)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 port = 0;
    
    /*1. clear source */
    mirror_clear_source_session(p_db_mirror);

    /*2. clear dest */
    if (MIRROR_DEST_PORT == p_db_mirror->dest_type) 
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(p_db_mirror->dest_port);
        if (NULL != p_db_if)
        {
            mirror_del_dest_from_session(p_db_mirror, p_db_if);
        }
    }
    else if (MIRROR_DEST_CPU == p_db_mirror->dest_type)
    {
        sal_system("cdbctl update/cdb/l2/cpu_mirror_global/write_file/stop");
        mirror_del_cpu_dest_from_session(p_db_mirror);
    }
    else if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
    {
        mirror_del_remote_dest_from_session(p_db_mirror, p_db_mirror->dest_vlan, p_db_mirror->dest_port);
    }
    else if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
    {
        GLB_BMP_ITER_BEGIN(p_db_mirror->dest_port_bmp, port)
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(port);
            if (NULL == p_db_if)
            {
                continue;
            }
            mirror_del_multi_dest_from_session(p_db_mirror, p_db_mirror->dest_group, p_db_if);
        }
        GLB_BMP_ITER_END(p_db_mirror->dest_port_bmp, port);
    }
    else
    {
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}

int32
mirror_mac_escape_add_mac_escape(tbl_mirror_mac_escape_t *p_mac_escape)
{
    int32 rc = PM_E_NONE;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();

    /* 1. add to DB */
    PM_E_RETURN(tbl_mirror_mac_escape_add_mirror_mac_escape(p_mac_escape));

    /* 2. set the mirror mac escape num */
    p_brg_global->max_mirror_mac_escape++;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE));

    return rc;
}

int32
mirror_mac_escape_del_mac_escape(tbl_mirror_mac_escape_t *p_mac_escape)
{
    int32 rc = PM_E_NONE;
    tbl_mirror_mac_escape_t* p_db_mac_escape = NULL;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    
    /* 1. check exist */
    p_db_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(&p_mac_escape->key);
    if (NULL == p_db_mac_escape)
    {
        return PM_E_NOT_EXIST;
    }
    
    /* 2. del from DB */
    PM_E_RETURN(tbl_mirror_mac_escape_del_mirror_mac_escape(&p_db_mac_escape->key));

    /* 3. set the mirror mac escape num */
    p_brg_global->max_mirror_mac_escape--;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE));
    
    return rc;
}

static int32
_mirror_join_agg_check(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    cfg_cmd_para_t *para = (cfg_cmd_para_t*)p_param;

    if (NULL == p_db_mem_if)
    {
        return PM_E_NONE;
    }

    if ((GLB_FLAG_ISSET(p_db_mem_if->mirror_enable, MIRROR_DEST_FLAG)) ||
        (GLB_FLAG_ISSET(p_db_mem_if->mirror_enable, MIRROR_DEST_REMOTE_FLAG)))
    {
        CFG_CONFLICT_RET("This port is already mirror destination port, can't add to aggregator");
    }

    if ((GLB_FLAG_ISSET(p_db_mem_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)) ||
        (GLB_FLAG_ISSET(p_db_mem_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG)))
    {
        CFG_CONFLICT_RET("This port is already mirror source port, can't add to aggregator");
    }

    return PM_E_NONE;
}

static int32
_mirror_join_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    int32 rc = PM_E_NONE;

    sal_memset(&mirror_key, 0 ,sizeof(tbl_mirror_key_t));
    if((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return PM_E_NONE;
    }
    
    if ((!GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)) &&
        (!GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG)))
    {
        return PM_E_NONE;
    }
    
    if (GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG))
    {
        mirror_key.id = p_db_agg_if->mirror_igs_id;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            return PM_E_NOT_EXIST;
        }
        GLB_BMP_SET(p_db_mirror->source_igs_port_bmp, p_db_mem_if->ifindex);
        p_db_mem_if->mirror_igs_id = p_db_mirror->key.id;
        tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_IGS_ID);
        rc |= tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP);
    }

    if (GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    {
        mirror_key.id = p_db_agg_if->mirror_egs_id;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            return PM_E_NOT_EXIST;
        }
        GLB_BMP_SET(p_db_mirror->source_egs_port_bmp, p_db_mem_if->ifindex);
        p_db_mem_if->mirror_egs_id = p_db_mirror->key.id;
        tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_EGS_ID);
        rc |= tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP);
    }
    
    p_db_mem_if->mirror_enable = p_db_agg_if->mirror_enable;
    tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    return rc;
}

static int32
_mirror_leave_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    int32 rc = PM_E_NONE;

    sal_memset(&mirror_key, 0 ,sizeof(tbl_mirror_key_t));
    
    if((NULL == p_db_agg_if) || (NULL == p_db_mem_if))
    {
        return PM_E_NONE;
    }

    if ((!GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)) &&
        (!GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG)))
    {
        return PM_E_NONE;
    }
    if (GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG))
    {
        mirror_key.id = p_db_agg_if->mirror_igs_id;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            return PM_E_NOT_EXIST;
        }
        GLB_BMP_UNSET(p_db_mirror->source_igs_port_bmp, p_db_mem_if->ifindex);
        p_db_mem_if->mirror_igs_id = 0;
        tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_IGS_ID);
        rc |= tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP);
    }

    if (GLB_FLAG_ISSET(p_db_agg_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    { 
        mirror_key.id = p_db_agg_if->mirror_egs_id;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            return PM_E_NOT_EXIST;
        }
        GLB_BMP_UNSET(p_db_mirror->source_egs_port_bmp, p_db_mem_if->ifindex);
        p_db_mem_if->mirror_egs_id = 0;
        tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_EGS_ID);
        rc |= tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP);
    }
    
    p_db_mem_if->mirror_enable = 0;
    tbl_interface_set_interface_field(p_db_mem_if, TBL_INTERFACE_FLD_MIRROR_ENABLE);
    return rc;
}

static int32
_mirror_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    mirror_del_port_process(p_db_agg_if);
    return PM_E_NONE;
}

int32
mirror_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_MIRROR, mirror_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_MIRROR_MAC_ESCAPE, mirror_mac_escape_cmd_process);
    cdb_register_cfg_act_cb(ACT_SHOW_MIRROR, mirror_cmd_process_show);
    cdb_register_cfg_act_cb(ACT_SHOW_MIRROR_MAC_ESCAPE, mirror_mac_escape_cmd_process_show);
    if_agg_add_member_check_set_cb(IF_AGG_CB_MODULE_MIRROR, _mirror_join_agg_check);
    if_agg_add_member_sync_prop_set_cb(IF_AGG_CB_MODULE_MIRROR, _mirror_join_agg_update);
    if_agg_del_member_sync_prop_set_cb(IF_AGG_CB_MODULE_MIRROR, _mirror_leave_agg_update);
    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_MIRROR, _mirror_agg_destroy);
    return PM_E_NONE;
}

int32
mirror_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_MIRROR, NULL);
    cdb_register_cfg_tbl_cb(TBL_MIRROR_MAC_ESCAPE, NULL);
    return PM_E_NONE;
}

int32
mirror_has_source_config(tbl_mirror_t *p_db_mirror)
{
    uint32 ifindex = 0;
    vid_t   vid = 0;

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_port_bmp, ifindex)
    {
        return TRUE;
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_port_bmp, ifindex);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, ifindex)
    {
        return TRUE;    
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, ifindex);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_vlan_bmp, vid)
    {
        return TRUE;    
    }
    GLB_BMP_ITER_END(p_db_mirror->source_igs_vlan_bmp, vid);

    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vid)
    {
        return TRUE;    
    }
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vid);

    if (p_db_mirror->cpu_source_dir)
    {
        return TRUE; 
    }
    
    return FALSE;
}

int32
mirror_multi_dest_port_is_the_last_one(tbl_mirror_t *p_db_mirror)
{
    uint32 ifindex = 0;
    uint32 count = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->dest_port_bmp, ifindex)
    {
        count++;
    }
    GLB_BMP_ITER_END(p_db_mirror->dest_port_bmp, ifindex);
    if (1 == count)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif /* CONFIG_MIRROR */

