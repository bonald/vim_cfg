#include "proto.h"
#include "glb_l2_define.h"
#include "gen/tbl_mirror_define.h" 
#include "gen/tbl_mirror.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "gen/tbl_vlan.h" 
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_sys_global_define.h" 
#include "gen/tbl_sys_global.h"
#include "switch.h"
#include "mirror_define.h"
#include "mirror.h"
#include "swth_sys.h"

#ifdef CONFIG_MIRROR

int32
_mirror_cmd_get_remote_vlan_sessiond_id(vid_t vlanid, uint32 *sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
           continue;     
        }
        
        if (p_db_mirror->dest_vlan == vlanid)
        {
            *sessionid = session;
            return PM_E_EXIST;
        }
    }
    return PM_E_NONE;
}

int32
_mirror_cmd_get_source_vlan_sessiond_id(vid_t vlanid, uint32 *sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vlanid)||
            GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vlanid))
        {
            *sessionid = session;
            return PM_E_EXIST;
        }
    }
    return PM_E_NONE;
}

int32
_mirror_cmd_get_source_igs_vlan_sessiond_id(vid_t vlanid, uint32 *sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vlanid))
        {
            *sessionid = session;
            return PM_E_NONE;
        }
    }
    *sessionid = GLB_MIRROR_MAX_SESSION_ID + 1;
    return PM_E_NONE;
}

int32
_mirror_cmd_get_source_egs_vlan_sessiond_id(vid_t vlanid, uint32 *sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
           continue;     
        }

        if (GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vlanid))
        {
            *sessionid = session;
            return PM_E_NONE;
        }
    }

    *sessionid = GLB_MIRROR_MAX_SESSION_ID + 1;
    return PM_E_NONE;
}

int32
_mirror_cmd_get_multidest_group_sessiond_id(uint32 *sessionid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }
        if (p_db_mirror->dest_group != 0)
        {
            *sessionid = session;
            return PM_E_EXIST;
        }
    }
    return PM_E_NONE;
}

tbl_mirror_t *
_mirror_cmd_get_multidest_session_group_id(uint32 groupid)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 session = 0;

    for (session = GLB_MIRROR_MIN_SESSION_ID; session <= GLB_MIRROR_MAX_SESSION_ID; session++)
    {
         /* 1. check exist */
        mirror_key.id = session;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;     
        }
        if ((p_db_mirror->dest_group != 0) && (p_db_mirror->dest_group != groupid))
        {
            return p_db_mirror;
        }
    }
    return NULL;
}


int32
_mirror_cmd_add_source_interface_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, char *ifname, uint8 direction)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    int rc = 0;

    /*1. check source interface exist, can't be agg member */
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }
    
    /*2. source interface can't be monitor dest port*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)|| 
        GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_REMOTE_FLAG))
    {
        CFG_CONFLICT_RET("This interface is monitor destination");
    }

    /*4. ingress source has been use in other session */
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG))
    {
        if ((p_db_if->mirror_igs_id != p_mirror->key.id) && GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
        {
            rc = PM_E_EXIST;
        }
    }

    /*5. egress source has been use in other session*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    {
        if ((p_db_if->mirror_egs_id != p_mirror->key.id) && GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
        {
            if (PM_E_EXIST == rc)
            {
                CFG_CONFLICT_RET("Both direction of this interface is already monitored in other session");
            }
            else
            {
                CFG_CONFLICT_RET("Egress traffic of this interface is already monitored in other session");
            }
        }
    }

    if (PM_E_EXIST == rc)
    {
        CFG_CONFLICT_RET("Ingress traffic of this interface is already monitored in other session");
    }

    if (0 == direction)
    {
        return PM_E_NONE;
    }

    return mirror_add_source_interface_to_session(p_mirror, direction, p_db_if, TRUE);
}

int32
_mirror_cmd_del_source_interface_from_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, char *ifname, uint8 direction)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;

    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }
    
    if (0 == direction)
    {
        return PM_E_NONE;
    }

    return mirror_del_source_interface_from_session(p_mirror, direction, p_db_if, TRUE);
}

int32
_mirror_cmd_add_source_vlan_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, vid_t vlanid, uint8 direction)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_vlan_key_t key;
    uint32 sessionid = 0;
    int rc = 0;

    /*1. check vlan exist */
    key.vid = vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan is not created");
    }

    /*2. check vlan interface exist */
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_sprintf(if_key.name, "vlan%u", vlanid);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Vlan Interface is not created");
    }

    /*3. check vlan has been used by remote mirror session's dest vlan */
    rc = _mirror_cmd_get_remote_vlan_sessiond_id(vlanid, &sessionid);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("This vlan is already being configured as RSPAN VLAN");
    }

    /*4. check vlan has been used by mirror session's source ingress vlan */
    _mirror_cmd_get_source_igs_vlan_sessiond_id(vlanid, &sessionid);
    if (GLB_MIRROR_MAX_SESSION_ID + 1 != sessionid)
    {
        if ((sessionid != p_mirror->key.id) && (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS)))
        {
            rc = PM_E_EXIST;
        }
        else if (sessionid == p_mirror->key.id)
        {
            GLB_SET_FLAG(direction, GLB_MIRROR_DIR_IGS);
        }
    }

    /*5. check vlan has been usded by mirror session's egress vlan */
    _mirror_cmd_get_source_egs_vlan_sessiond_id(vlanid, &sessionid);
    if (GLB_MIRROR_MAX_SESSION_ID + 1 != sessionid)
    {
        if ((sessionid != p_mirror->key.id) && (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS)))
        {
            if (PM_E_EXIST == rc)
            {
                CFG_CONFLICT_RET("Both direction of this VLAN is already monitored in other session.");
            }
            else
            {
                CFG_CONFLICT_RET("Egress traffic of this VLAN is already monitored in other session");
            }
        }
        else if (sessionid == p_mirror->key.id)
        {
            GLB_SET_FLAG(direction, GLB_MIRROR_DIR_EGS);
        }
    }

    if (PM_E_EXIST == rc)
    {
        CFG_CONFLICT_RET("Ingress traffic of this VLAN is already monitored in other session");
    }

    if (0 == direction)
    {
        return PM_E_NONE;
    }
    
    return mirror_add_source_vlan_to_session(p_mirror, direction, vlanid);
}

int32
_mirror_cmd_del_source_vlan_from_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, uint32 vlanid, uint8 direction)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_vlan_key_t vlan_key;

    /*1. check vlan exist */
    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    vlan_key.vid = vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan is not created");
    }

    /*2. check vlan interface exist */
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_sprintf(if_key.name, "vlan%u", vlanid);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Vlan Interface is not created");
    }
    if (0 == direction)
    {
        return PM_E_NONE;
    }
    
    return mirror_del_source_vlan_from_session(p_mirror, direction, vlanid);
}

int32
_mirror_cmd_add_source_cpu_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_db_mirror, uint32 direction)
{
    tbl_mirror_master_t *p_master = tbl_mirror_get_master();
    tbl_mirror_t * p_iter_mirror = NULL;
    uint32 id = 0;

    /* 1. check mirror source cpu to remote vlan*/
    if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
    {
        CFG_CONFLICT_RET("Not support cpu mirror to remote vlan");
    }
    /*2. check mirror source cpu to groupe */
    /*Modified by yejl for bug 54184, 2019-11-04
    if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
    {
        CFG_CONFLICT_RET("Not support cpu mirror to group");
    }
    */
    
    /*check source cpu exist */
    for (id = 0; id < GLB_MIRROR_MAX_SESSION_ID+1; id++)
    {
        p_iter_mirror = p_master->mirror_array[id];
        if (NULL == p_iter_mirror)
        {
            continue;
        }
        /* rx+tx */
        if (p_iter_mirror == p_db_mirror)
        {
            continue;
        }
        
        /* only one session */
        if (p_iter_mirror->cpu_source_dir)
        {
            CFG_CONFLICT_RET("The CPU source has been monitored in session %u", p_iter_mirror->key.id);
        }
    }
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_SET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS);
    }
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_SET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS);
    }
    
    return tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_CPU_SOURCE_DIR);
}

int32
_mirror_cmd_del_source_cpu_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_db_mirror, uint32 direction)
{    
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_IGS))
    {
        GLB_UNSET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS);
    }
    if (GLB_FLAG_ISSET(direction, GLB_MIRROR_DIR_EGS))
    {
        GLB_UNSET_FLAG(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS);
    }
    
    return tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_CPU_SOURCE_DIR);
}

int32
_mirror_cmd_add_dest_to_session(cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, char *ifname)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if     = NULL;
    tbl_interface_t *p_db_old_if = NULL;
    tbl_interface_key_t if_key;
    bool isup = FALSE;

    /*1. check the mirror session is used in other monitor destination */
    if ((MIRROR_DEST_NONE != p_mirror->dest_type) && (MIRROR_DEST_PORT != p_mirror->dest_type))
    {
        CFG_CONFLICT_RET("This monitor session already have monitor destination");
    }

    /*2. check the mirror destination cpu is not update the  monitor destination interface */
    if (mirror_has_source_config(p_mirror))
    {
        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
    }
    
    /*3. check dest interface is exist, must be up, can't be linkagg member port*/
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (swth_sys_is_startup_done())
    {
        isup = tbl_interface_is_up(p_db_if);
        if (!isup)
        {
            CFG_CONFLICT_RET("Interface %s is not UP, can't configure monitor", ifname_ext);
        }
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }

    /*3. dest interface can't be ingress or egress source interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)||
        GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    {
        CFG_CONFLICT_RET("Interface is already monitor source");
    }

    /*4. dest interface can't be other monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }
    
    /*5. dest interface can't be other remote monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_REMOTE_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }

    /* modified by liwh for bug 42729, 2017-03-14 */
    if (p_db_if->erps_enable)
    {
        CFG_CONFLICT_RET("ERPS interface can't be configured as monitor destination");
    }
    /* liwh end */

    if (p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("MLAG peer-link has been configured, can't configure monitor destination");
    }
    
    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("MLAG has been configured, can't configure monitor destination");
    }

     if (p_db_if->current_g8032_ring_count)
    {
        CFG_CONFLICT_RET("G8032 has been configured, can't configure monitor destination");
    }
    
    /*6. check duplicate config */
    if (p_mirror->dest_port == p_db_if->ifindex)
    {
        return PM_E_NONE;
    }
    else
    {
        p_db_old_if = tbl_interface_get_interface_by_ifindex(p_mirror->dest_port);
        if (NULL != p_db_old_if)
        {
            if (mirror_has_source_config(p_mirror))
            {
                CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
            }
            mirror_del_dest_from_session(p_mirror, p_db_old_if);
        }
    }

    return mirror_add_dest_to_session(p_mirror, p_db_if);
}

int32
_mirror_cmd_del_dest_from_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, char *ifname)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;  
    tbl_interface_key_t if_key;

    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);

    if (mirror_has_source_config(p_mirror))
    {
        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
    }
     
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }

    return mirror_del_dest_from_session(p_mirror, p_db_if);
}

int32
_mirror_cmd_add_remote_dest_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, vid_t vlanid, char *ifname)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_key_t if_key;
    uint32 sessionid = 0;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_vlan_key_t key;
    int rc = 0;

    /*1. check the mirror session is used in other monitor destination */
    if ((MIRROR_DEST_NONE != p_mirror->dest_type) && (MIRROR_DEST_VLAN != p_mirror->dest_type))
    {
        CFG_CONFLICT_RET("This monitor session already have monitor destination");
    }
    
    /*2. check vlan is not exist*/
    if (1 == vlanid)
    {
        CFG_CONFLICT_RET("Vlan 1 can't used");
    }
    key.vid = vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan is not created");
    }

    /*3. check dest interface is not exist, can't be linkagg member*/
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }

    /*4. dest interface can't be ingress or egress source interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)||
        GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    {
        CFG_CONFLICT_RET("Interface is already monitor source");
    }

    /*5. dest interface can't be other monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }
    
    /*6. dest interface can't be other remote monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_REMOTE_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }

    /*7. check vlan is used in source vlan*/
    rc = _mirror_cmd_get_source_vlan_sessiond_id(vlanid, &sessionid);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("The vlan is used in source vlan in monitor session %d.", sessionid);
    }

    /*8. check vlan is used by other remote session's dest vlan */
    rc = _mirror_cmd_get_remote_vlan_sessiond_id(vlanid, &sessionid);
    if (rc < 0)
    {
        if (sessionid != p_mirror->key.id)
        {
            CFG_CONFLICT_RET("This vlan is used as remote destination vlan in other session");
        }
    }

    /*9. check dest config */
    if (mirror_has_source_config(p_mirror))
    {
        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
    }
    
    /*10. check duplicate config */
    if ((p_mirror->dest_vlan == vlanid) && (p_mirror->dest_port == p_db_if->ifindex))
    {
        return PM_E_NONE;
    }
    else
    {
        if (mirror_has_source_config(p_mirror))
        {
            CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
        }
        mirror_del_remote_dest_from_session(p_mirror, p_mirror->dest_vlan, p_mirror->dest_port); 
    }

    return mirror_add_remote_dest_to_session(p_mirror, vlanid, p_db_if);
}

int32
_mirror_cmd_del_remote_dest_from_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, vid_t vlanid, char *ifname)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_key_t p_vlan_key;

    sal_memset(&p_vlan_key, 0, sizeof(p_vlan_key));
    p_vlan_key.vid = vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&p_vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("vlan %d doesn't exist", vlanid);
    }
    
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }

    return mirror_del_remote_dest_from_session(p_mirror, vlanid, p_db_if->ifindex);
}

int32
_mirror_cmd_add_multi_dest_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, uint32 groupid, char *ifname)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if     = NULL;
    tbl_interface_key_t if_key;
    tbl_mirror_t *p_db_mirror = NULL;
    uint32 sessionid = 0;
    int32  rc = 0;
    bool   isup = FALSE;

    /*1. check the mirror session is used in other monitor destination */
    if ((MIRROR_DEST_NONE != p_mirror->dest_type) && (MIRROR_DEST_GROUP != p_mirror->dest_type))
    {
        CFG_CONFLICT_RET("This monitor session already have monitor destination");
    }

    /*2. check the mirror session group is used in other monitor session */
    rc = _mirror_cmd_get_multidest_group_sessiond_id(&sessionid);
    if (rc < 0)
    {
        if (sessionid != p_mirror->key.id)
        {
            CFG_CONFLICT_RET("Multi-dest group has already exist, can't set on session %d", p_mirror->key.id);
        }
    }
    
    /*3. mirror session group can be used only once in same monitor session */
    p_db_mirror = _mirror_cmd_get_multidest_session_group_id(groupid);
    if (p_db_mirror)
    {
        if (p_db_mirror->key.id == p_mirror->key.id)
        {
            CFG_CONFLICT_RET("Multi-dest group id has already exist, group id is %d", p_db_mirror->dest_group);
        }
    }
    
    /*4. check dest interface is exist, must be up, can't be linakagg port or linkagg member port*/
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (swth_sys_is_startup_done())
    {
        isup = tbl_interface_is_up(p_db_if);
        if (!isup)
        {
            CFG_CONFLICT_RET("Interface %s is not UP, can't configure monitor", ifname_ext);
        }
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }
    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on link aggregation group");
    }
    
    /*5. dest interface can't be ingress or egress source interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_IGS_FLAG)|| 
        GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_SOURCE_EGS_FLAG))
    {
        CFG_CONFLICT_RET("Interface is already monitor source");
    }

    /*6. dest interface can't be other monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }
    
    /*7. dest interface can't be other remote monitor session's dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_REMOTE_FLAG) &&
        (p_db_if->mirror_dest_id != p_mirror->key.id))
    {
        CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
    }
    
    /*8. check the mirror destination interface is cannot update the  monitor destination cpu  */
    if (mirror_has_source_config(p_mirror))
    {
        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_mirror->key.id);
    }
    
    /*9. check duplicate config */
    if ((p_mirror->dest_group == groupid) && (GLB_BMP_ISSET(p_mirror->dest_port_bmp, p_db_if->ifindex)))
    {
        return PM_E_NONE;
    }
    return mirror_add_multi_dest_to_session(p_mirror, groupid, p_db_if);
}

int32
_mirror_cmd_del_multi_dest_from_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_mirror, uint32 groupid, char *ifname)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t *p_db_if     = NULL;
    tbl_interface_key_t if_key;

    if (MIRROR_DEST_GROUP != p_mirror->dest_type)
    {
        CFG_CONFLICT_RET("This monitor session is not group monitor destination");
    }
    
    if (p_mirror->dest_group != groupid)
    {
        CFG_CONFLICT_RET("This multi-dest group is not in the monitor session");
    }
    
    IFNAME_ETH2FULL(ifname, ifname_ext);
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s doesn't exist, can't configure monitor", ifname_ext);
    }
    if (!GLB_BMP_ISSET(p_mirror->dest_port_bmp, p_db_if->ifindex))
    {
        CFG_CONFLICT_RET("Interface %s doesn't belong to the multi-dest port", ifname_ext);
    }   
    
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on member of link aggregation group");
    }
    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        CFG_CONFLICT_RET("Can't monitor traffic on link aggregation group");
    }
    
    return mirror_del_multi_dest_from_session(p_mirror, groupid, p_db_if);
}

int32
_mirror_cmd_add_cpu_dest_to_session(
    cfg_cmd_para_t *para, tbl_mirror_t *p_db_mirror)
{   
    tbl_mirror_t * p_mirror =NULL;    
    uint32 id = 0;
    tbl_mirror_master_t *p_master = tbl_mirror_get_master();
  
    /*1. check the mirror session is used the same monitor destination */
    if ((MIRROR_DEST_NONE != p_db_mirror->dest_type) && (MIRROR_DEST_CPU != p_db_mirror->dest_type))
    {
        CFG_CONFLICT_RET("This monitor session already have monitor destination");
    }
    
    /*2. check the mirror destination interface is cannot update the  monitor destination cpu  */
    if (mirror_has_source_config(p_db_mirror))
    {
        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_db_mirror->key.id);
    }
    
    for (id = 0; id < GLB_MIRROR_MAX_SESSION_ID+1; id++)
    {
        p_mirror = p_master->mirror_array[id];
        if (NULL == p_mirror)
        {
            continue;
        }
        if (p_mirror->key.id == p_db_mirror->key.id)
        {
            continue;               
        }
        if (p_mirror->dest_type == MIRROR_DEST_CPU)
        {
            CFG_CONFLICT_RET("Interface already configured as monitor destination in other monitor session");
        }      
    } 
    p_db_mirror->dest_type    = MIRROR_DEST_CPU;
    tbl_mirror_set_mirror_field(p_db_mirror, TBL_MIRROR_FLD_DEST_TYPE);

    return PM_E_NONE;
}

int32
_mirror_cmd_read_session(uint32 sessionid, cfg_cmd_para_t *para)
{
    tbl_mirror_key_t mirror_key;
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    /* 1. check exist */
    mirror_key.id = sessionid;
    p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
    if (NULL == p_db_mirror)
    {
        sal_fclose(fp);
        return PM_E_NONE;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    /* 2. dump mirror */
    tbl_mirror_dump_one(p_db_mirror, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
_mirror_cmd_read_session_all(cfg_cmd_para_t *para)
{
    tbl_mirror_key_t mirror_key;
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_iter_args_t args;
    uint32 sessionid = 0;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    for (sessionid = GLB_MIRROR_MIN_SESSION_ID; sessionid <= GLB_MIRROR_MAX_SESSION_ID; sessionid++)
    {
        /* 1. check exist */
        mirror_key.id = sessionid;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            continue;
        }

        sal_memset(&args, 0 ,sizeof(args));
        p_field = &field;
        sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
        args.argv[0] = p_field;
        args.argv[1] = fp;

        /* 2. dump mirror */
        tbl_mirror_dump_one(p_db_mirror, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

/*added by yejl to fix bug41995, 2016-12-06*/
int32
_mirror_cmd_show_monitor_session_one(tbl_mirror_t *p_db_mirror,  FILE *fp)
{
    char if_str[IFNAME_SIZE+1];
    char ifname_ext[IFNAME_SIZE+1];
    tbl_interface_t* p_db_if = NULL;
    uint32 ifindex = 0;
    uint32 vid = 0;
    uint32 i = 0;
    int32 rc = PM_E_NONE;
    
    sal_fprintf(fp, "%-17s: %d\n", "Session", p_db_mirror->key.id);

    if (MIRROR_DEST_NONE == p_db_mirror->dest_type)
    {
        sal_fprintf(fp, "%-17s: %s\n", "Status", "Invalid");
    }
    else 
    {
        sal_fprintf(fp, "%-17s: %s\n", "Status", "valid");
        if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
        {
            sal_fprintf(fp,"%-17s: %s\n", "Type", "Local Session");
        }
        else if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
        {
            sal_fprintf(fp,"%-17s: %s\n", "Type", "Group Session");
        }
        else if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
        {
            sal_fprintf(fp,"%-17s: %s\n", "Type", "Remote Session");
        }
        else if (MIRROR_DEST_CPU == p_db_mirror->dest_type)
        {
            sal_fprintf(fp,"%-17s: %s\n", "Type", "CPU Session");
        }
    }
    
    sal_fprintf(fp,"%-17s: %s\n", "Source Ports", "");
   
    if (p_db_mirror->cpu_source_dir == GLB_MIRROR_DIR_IGS)
    {
        sal_fprintf(fp,"%-17s: %s", "Receive Only", "CPU ");
    }
    else
    {
        sal_fprintf(fp,"%-17s: %s", "Receive Only", "");
    }   
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_port_bmp, ifindex)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_egs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp,"%s ", ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp,"%s ", ifname_ext);
            }
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_igs_port_bmp, ifindex);
    
    if (p_db_mirror->cpu_source_dir == GLB_MIRROR_DIR_EGS)
    {
        sal_fprintf(fp,"%-17s: %s", "Transmit Only", "CPU ");
    }
    else
    {
        sal_fprintf(fp,"%-17s: %s", "Transmit Only", "");
    }
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, ifindex)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp, "%s ", ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp, "%s ", ifname_ext);
            }
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, ifindex);

    if (p_db_mirror->cpu_source_dir == GLB_MIRROR_DIR_BOTH)
    {
        sal_fprintf(fp,"%-17s: %s", "Both", "CPU ");
    }
    else
    {    
        sal_fprintf(fp,"%-17s: %s", "Both", "");
    }    
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_port_bmp, ifindex)
    {
        if (GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex))
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return PM_E_INVALID_PARAM;
            }
            if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (NULL == p_db_if->lag && NULL == p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp, "%s ", ifname_ext);
            }
            else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type && (NULL != p_db_if->lag || NULL != p_db_if->lacp))
            {
                IFNAME_ETH2SHORT(if_str, ifname_ext);
                if(i && (i % 5) == 0)
                {
                    sal_fprintf(fp, "\n");
                    sal_fprintf(fp, "%-17s  ", "");
                }
                i++;
                sal_fprintf(fp, "%s ", ifname_ext);
            }
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_egs_port_bmp, ifindex);

    sal_fprintf(fp,"%-17s: %s", "Source Vlans", "");
    sal_fprintf(fp,"\n");
    sal_fprintf(fp,"%-17s: %s", "Receive Only", "");
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_igs_vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vid))
        {
            if(i && (i % 8) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-17s  ", "");
            }
            i++;
            sal_fprintf(fp,"%-5d", vid);
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_igs_vlan_bmp, vid);

    sal_fprintf(fp,"%-17s: %s", "Transmit Only", "");
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vid))
        {
            if(i && (i % 8) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-17s  ", "");
            }
            i++;
            sal_fprintf(fp,"%-5d", vid);
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vid);

    sal_fprintf(fp,"%-17s: %s", "Both", "");
    i = 0;
    GLB_BMP_ITER_BEGIN(p_db_mirror->source_egs_vlan_bmp, vid)
    {
        if (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vid))
        {
            if(i && (i % 8) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-17s  ", "");
            }
            i++;
            sal_fprintf(fp,"%-5d", vid);
        }
    }
    sal_fprintf(fp,"\n");
    GLB_BMP_ITER_END(p_db_mirror->source_egs_vlan_bmp, vid);

    i = 0;
    if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
    {
        rc = tbl_interface_get_name_by_ifindex(p_db_mirror->dest_port, if_str, IFNAME_SIZE);
        if (rc)
        {
            return rc;
        }
        IFNAME_ETH2SHORT(if_str, ifname_ext);
        sal_fprintf(fp,"%-17s: %s\n", "Destination Port", ifname_ext);
    }
    else if (MIRROR_DEST_VLAN == p_db_mirror->dest_type)
    {
        rc = tbl_interface_get_name_by_ifindex(p_db_mirror->dest_port, if_str, IFNAME_SIZE);
        if (rc)
        {
             return rc;
        }
        IFNAME_ETH2SHORT(if_str, ifname_ext);
        sal_fprintf(fp,"%-17s: %s\n", "Destination Port", ifname_ext);
        sal_fprintf(fp,"%-17s: %d\n", "Destination remote VLAN ", p_db_mirror->dest_vlan);
    }
    else if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
    {
        sal_fprintf(fp,"%-17s: %s", "Destination Port", "");
        GLB_BMP_ITER_BEGIN(p_db_mirror->dest_port_bmp, ifindex)
        {
            rc = tbl_interface_get_name_by_ifindex(ifindex, if_str, IFNAME_SIZE);
            if (rc)
            {
                continue;
            }
            IFNAME_ETH2SHORT(if_str, ifname_ext);
            if(i && (i % 5) == 0)
            {
                sal_fprintf(fp, "\n");
                sal_fprintf(fp, "%-17s  ", "");
            }
            i++;            
            sal_fprintf(fp,"%s ", ifname_ext);
        }
        GLB_BMP_ITER_END(p_db_mirror->dest_port_bmp, ifindex);
        sal_fprintf(fp,"\n");
    }
    else if (MIRROR_DEST_CPU == p_db_mirror->dest_type)
    {
        sal_fprintf(fp,"%-17s: %s\n", "Destination Port", "CPU ");
    }
    else
    {
        return PM_E_NOT_SUPPORT;
    }
    sal_fprintf(fp,"\n");
    return PM_E_NONE;
}

static int32
 _mirror_cmd_show_monitor_session(tbl_mirror_t *p_db_mirror,  cfg_cmd_para_t *para)
{   
    tbl_mirror_key_t mirror_key;
    FILE *fp = NULL;
    int id = 0;
    uint32 count = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_mirror)
    {
        _mirror_cmd_show_monitor_session_one(p_db_mirror, fp);
    }
    else
    {
        for (id = 1; id <= GLB_MIRROR_MAX_SESSION_ID; id++)
        {
           sal_memset(&mirror_key, 0, sizeof(tbl_mirror_key_t));
           mirror_key.id = id;
           p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
           if (NULL == p_db_mirror)
           {
               continue;
           }
           _mirror_cmd_show_monitor_session_one(p_db_mirror, fp);
           count++;
        }
        if (0 == count)
        {
            /*modified by yejl for bug 52439, 2019-05-30*/
            sal_fprintf(fp,"%%There is no mirror session configured in the system");
            sal_fclose(fp);
            fp = NULL;
            return PM_E_NONE;
        }
    }  
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32 
mirror_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    uint32 sessionid = 0;
    int rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
   
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            _mirror_cmd_show_monitor_session(NULL, para);
        }
        else if(1 == argc)
        {
            PM_CFG_GET_UINT(sessionid, argv[0], para->p_rs);
            sal_memset(&mirror_key, 0, sizeof(mirror_key));
            mirror_key.id = sessionid;
            p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
            if (NULL == p_db_mirror)
            {
                CFG_CONFLICT_RET("Mirror session %u is not configured in the system", sessionid);
            }
            _mirror_cmd_show_monitor_session(p_db_mirror, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break; 
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;   
    }

    return PM_E_NONE;
}

/* support sflow by liwh for bug 43981, 2017-06-05 */
bool
mirror_discard_sflow_packet(uint32 sessionid)
{
    tbl_mirror_t       *p_db_mirror = NULL;
    tbl_mirror_key_t   mirror_key;
    
    sal_memset(&mirror_key, 0, sizeof(mirror_key));

    mirror_key.id = sessionid;
    p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
    if (NULL == p_db_mirror)
    {
        return FALSE;    
    }

    if (p_db_mirror->dest_vlan)
    {
        return FALSE;    
    }

    return TRUE;
}
/* liwh end */

int32
mirror_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_mirror_t *p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    tbl_interface_t *p_db_if = NULL;
    uint32 sessionid = 0;
    uint32 direction = 0;
    vid_t  vlanid = 0;
    uint32 groupid = 0;
    uint32 ifindex = 0;
    uint32 create_mirror = FALSE;
    int rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_GET_UINT(sessionid, argv[0], para->p_rs);

        mirror_key.id = sessionid;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            /*when creat mirror session ,should creat destination mirror session first*/
            if (0 == sal_strcmp(argv[1], "source"))
            {
                CFG_CONFLICT_RET("Should creat monitor session %u destination at first", sessionid);
            }
            /*creat mirror session*/
            rc = mirror_create_session(sessionid);
            if (PM_E_NONE != rc)
            {
                return rc;
            }
            create_mirror = TRUE;

            /*p_db_mirror is NULL or not*/
            p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
            if (NULL == p_db_mirror)
            {
                return PM_E_NOT_EXIST;
            }
        }

        if (0 == sal_strcmp(argv[1], "source")) /*set mirror source session*/
        {
            /*the direction of mirror source session*/
            PM_CFG_CHECK_MIN_PARAMS(argc, 5, para->p_rs);
            if (0 == sal_strcmp(argv[4], "both"))
            {
                direction = GLB_MIRROR_DIR_BOTH;
            }
            else if (0 == sal_strcmp(argv[4], "tx"))
            {
                direction = GLB_MIRROR_DIR_EGS;
            }
            else if (0 == sal_strcmp(argv[4], "rx"))
            {
                direction = GLB_MIRROR_DIR_IGS;
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }

            if (0 == sal_strcmp(argv[2], "interface")) /*add the mirror source interface*/
            {
                rc = _mirror_cmd_add_source_interface_to_session(para, p_db_mirror, argv[3], direction);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
            }
            else if (0 == sal_strcmp(argv[2], "vlan")) /*add the mirror source vlan*/
            {
                PM_CFG_GET_UINT(vlanid, argv[3], para->p_rs);
                rc = _mirror_cmd_add_source_vlan_to_session(para, p_db_mirror, vlanid, direction);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
            }
            else if (0 == sal_strcmp(argv[2], "cpu")) /*add the mirror source cpu*/
            {                  
                rc = _mirror_cmd_add_source_cpu_to_session(para, p_db_mirror, direction);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }
        }
        else  /*set mirror destination session*/
        {
            if (0 == sal_strcmp(argv[2], "interface")) /*set local mirror destination*/
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 4, para->p_rs);
                rc = _mirror_cmd_add_dest_to_session(para, p_db_mirror, argv[3]);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
                if (p_sys_glb->startup_done)
                {
                    CFG_PROMPT_RET("Notice: spanning-tree configuration of this interface is removed");
                }
            }
            else if (0 == sal_strcmp(argv[2], "remote")) /*set remote mirror destination*/
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 5, para->p_rs);
                PM_CFG_GET_UINT(vlanid, argv[3], para->p_rs);
                rc = _mirror_cmd_add_remote_dest_to_session(para, p_db_mirror, vlanid, argv[4]);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
            }
            else if (0 == sal_strcmp(argv[2], "group")) /*set multi mirror destination*/
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 5, para->p_rs);
                PM_CFG_GET_UINT(groupid, argv[3], para->p_rs);
                rc = _mirror_cmd_add_multi_dest_to_session(para, p_db_mirror, groupid, argv[4]);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
                if (p_sys_glb->startup_done)
                {
                    CFG_PROMPT_RET("Notice: spanning-tree configuration of this interface is removed");
                }
            }
            else if (0 == sal_strcmp(argv[2], "cpu"))
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                rc = _mirror_cmd_add_cpu_dest_to_session(para, p_db_mirror);
                if (rc)
                {
                    if (create_mirror)
                    {
                        mirror_destroy_session(sessionid);
                    }
                    return rc;
                }
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }
        }
        break;

    case CDB_OPER_SET:
        
            break;

    case CDB_OPER_DEL:
        PM_CFG_GET_UINT(sessionid, argv[0], para->p_rs);

        /*if mirror session is not exist, return*/
        mirror_key.id = sessionid;
        p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
        if (NULL == p_db_mirror)
        {
            CFG_CONFLICT_RET("Mirror session %u is not exist", sessionid);
        }

        /*delete all information of one mirror session, include source and dest*/
        if (1 == argc)
        {
            mirror_clear_session(p_db_mirror);    
            mirror_destroy_session(sessionid);
            return PM_E_NONE;
        }

        /*delete mirror source session*/
        if (0 == sal_strcmp(argv[1], "source"))
        {
            /*the direction of mirror source session*/
            PM_CFG_CHECK_MIN_PARAMS(argc, 5, para->p_rs);
            if (0 == sal_strcmp(argv[4], "both"))
            {
                direction = GLB_MIRROR_DIR_BOTH;
            }
            else  if (0 == sal_strcmp(argv[4], "tx"))
            {
                direction = GLB_MIRROR_DIR_EGS;
            }
            else if (0 == sal_strcmp(argv[4], "rx"))
            {
                direction = GLB_MIRROR_DIR_IGS;
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }
            
            if (0 == sal_strcmp(argv[2], "interface")) /*delete the mirror source interface*/
            {
                _mirror_cmd_del_source_interface_from_session(para, p_db_mirror, argv[3], direction);
            }
            else if (0 == sal_strcmp(argv[2], "vlan")) /*delete the mirror source vlan*/
            {
                PM_CFG_GET_UINT(vlanid, argv[3], para->p_rs);
                _mirror_cmd_del_source_vlan_from_session(para, p_db_mirror, vlanid, direction);
            }
            else if (0 == sal_strcmp(argv[2], "cpu")) /*delete the mirror source cpu*/
            {            
                _mirror_cmd_del_source_cpu_to_session(para, p_db_mirror, direction);
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }
        }
        else
        {
            /*when delete the mirror session, should delete the souce mirror session first*/
            if (argc == 3)
            {
                if (mirror_has_source_config(p_db_mirror))
                {
                    CFG_CONFLICT_RET("Monitor session %u destination still has source information, cannot delete", sessionid);
                }
            }
            else if (argc == 5)
            {
                if (mirror_multi_dest_port_is_the_last_one(p_db_mirror))
                {
                    if (mirror_has_source_config(p_db_mirror))
                    {
                        CFG_CONFLICT_RET("Monitor session %u destination still has source information, cannot delete", sessionid);
                    }
                }
            }
            else
            {
                return PM_E_INVALID_PARAM;
            }
            
            if (0 == sal_strcmp(argv[2], "interface")) /*delete the local mirror destination*/
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
                {
                    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_mirror->dest_port);
                    if (NULL == p_db_if)
                    {
                        return PM_E_NOT_EXIST;
                    }
                    _mirror_cmd_del_dest_from_session(para, p_db_mirror, p_db_if->key.name);
                }
                else if (MIRROR_DEST_CPU == p_db_mirror->dest_type) /*delete the cpu mirror destination*/
                {
                    if (mirror_has_source_config(p_db_mirror))
                    {
                        CFG_CONFLICT_RET("Monitor session %u still has source information, cannot update", p_db_mirror->key.id);
                    }
                    mirror_del_cpu_dest_from_session(p_db_mirror);
                }
       
            }
            else if (0 == sal_strcmp(argv[2], "remote")) /*delete the remote mirror destination*/
            {
                p_db_if = tbl_interface_get_interface_by_ifindex(p_db_mirror->dest_port);
                if (NULL == p_db_if)
                {
                    return PM_E_NOT_EXIST;
                }
                PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                _mirror_cmd_del_remote_dest_from_session(para, p_db_mirror, p_db_mirror->dest_vlan, p_db_if->key.name);
            }
            else if (0 == sal_strcmp(argv[2], "group")) /*delete the multi mirror destination*/
            {
                PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
                /*delete the all multi mirror destination port*/
                if (argc == 3)
                {
                    groupid = p_db_mirror->dest_group;
                    if (0 == groupid)
                    {
                        return PM_E_NOT_EXIST;
                    }
                    GLB_BMP_ITER_BEGIN(p_db_mirror->dest_port_bmp, ifindex)
                    {
                        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
                        if (NULL == p_db_if)
                        {
                            continue;
                        }
                        _mirror_cmd_del_multi_dest_from_session(para, p_db_mirror, groupid, p_db_if->key.name);
                    }
                    GLB_BMP_ITER_END(p_db_mirror->dest_port_bmp, ifindex);
                }
                /*delete the specific multi mirror destination port*/
                else if (argc == 5)
                {
                    PM_CFG_GET_UINT(groupid, argv[3], para->p_rs);
                    _mirror_cmd_del_multi_dest_from_session(para, p_db_mirror, groupid, argv[4]);
                }
                else
                {
                    return PM_E_INVALID_PARAM;
                }
            }
            else
            {
                return PM_E_NOT_SUPPORT;
            }
        }

        return mirror_destroy_session(sessionid);
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(sessionid, argv[0], para->p_rs);
            rc = _mirror_cmd_read_session(sessionid, para);
        }
        else
        {
            rc = _mirror_cmd_read_session_all(para);
        }
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
_mirror_mac_escape_add_mac_escape(tbl_mirror_mac_escape_t *p_mac_escape, cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    tbl_mirror_mac_escape_t *p_db_mac_escape = NULL;
    int32 rc = PM_E_NONE;

    /*1. mac escape num check */
    if (p_brg_global->max_mirror_mac_escape >= GLB_MIRROR_MAC_ESCAPE_MAX)
    {
        CFG_CONFLICT_RET("Mac escape entries overflow");
    } 
    /*2. duplicate check*/
    p_db_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(&p_mac_escape->key);
    if (p_db_mac_escape)
    {
        CFG_CONFLICT_RET("Mac escape entry already exist");
    }
    /*2. call API*/
    rc = mirror_mac_escape_add_mac_escape(p_mac_escape);
    return rc;
}

int32
_mirror_mac_escape_del_mac_escape(tbl_mirror_mac_escape_t *p_db_mac_escape, cfg_cmd_para_t *para)
{
    tbl_mirror_mac_escape_master_t* p_master = tbl_mirror_mac_escape_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;
    int32 rc = PM_E_NONE;
    if (p_db_mac_escape)
    {
        rc = mirror_mac_escape_del_mac_escape(p_db_mac_escape);
    }
    else
    {
        if (p_master->mirror_mac_escape_list->count == 0)
        {
            CFG_INVALID_PARAM_RET("Mac escape entry not exist");
        }
        CTCLIB_SLIST_LOOP_DEL(p_master->mirror_mac_escape_list, p_db_mac_escape, listnode, nextnode)
        {
            rc = mirror_mac_escape_del_mac_escape(p_db_mac_escape);
        }
    }
    return rc;
}

int32
_mirror_mac_escape_get_mac_escape(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_mirror_mac_escape_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mirror_mac_escape_show_mac_escape_banner(FILE *fp, uint32 count)
{
    sal_fprintf(fp,"-------------------------------------------------------------\n");
    sal_fprintf(fp,"            monitor rspan mac escape database\n");
    sal_fprintf(fp,"-------------------------------------------------------------\n");
    sal_fprintf(fp," count   : %d\n", count);
    sal_fprintf(fp,"-------------------------------------------------------------\n");
    return PM_E_NONE;
}
    
int32
_mirror_mac_escape_cmd_show_mac_escape(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_mirror_mac_escape_t *p_db_mac_escape = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mirror_mac_escape_master_t* p_master = tbl_mirror_mac_escape_get_master();
    uint32 count = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    count = p_master->mirror_mac_escape_list->count;
    _mirror_mac_escape_show_mac_escape_banner(fp, count);
        
    CTCLIB_SLIST_LOOP(p_master->mirror_mac_escape_list, p_db_mac_escape, listnode)
    {
         sal_fprintf(fp," Mac     : %.04hX.%.04hX.%.04hX\n",
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mac)[0]),
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mac)[1]), 
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mac)[2]));
         sal_fprintf(fp," Mask    : %.04hX.%.04hX.%.04hX\n",
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mask)[0]),
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mask)[1]), 
            sal_ntoh16(((uint16*)p_db_mac_escape->key.mask)[2]));
    }
    if (count)
    {
        sal_fprintf(fp,"-------------------------------------------------------------\n");
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
mirror_mac_escape_cmd_process_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
   
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 0, para->p_rs);
        if (0 == argc)
        {
            rc = _mirror_mac_escape_cmd_show_mac_escape(para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break; 
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;   
    }

    return rc;
}

int32
mirror_mac_escape_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mirror_mac_escape_t p_mac_escape;
    tbl_mirror_mac_escape_t *p_db_mac_escape = NULL;
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_MIRROR_MAC_ESCAPE);

    sal_memset(&p_mac_escape, 0, sizeof(tbl_mirror_mac_escape_t));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        rc = cdb_mac_str2val(p_mac_escape.key.mac, argv[0]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[0]);
        }
        rc = cdb_mac_str2val(p_mac_escape.key.mask, argv[1]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Unable to translate MASK address %s", argv[1]);
        }
        rc = _mirror_mac_escape_add_mac_escape(&p_mac_escape, para);
       break;
    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 2, para->p_rs);
        if (argc == 0)
        {
            rc = _mirror_mac_escape_del_mac_escape(NULL, para);
        }
        else if (argc == 2)
        {
            rc = cdb_mac_str2val(p_mac_escape.key.mac, argv[0]);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[0]);
            }
            rc = cdb_mac_str2val(p_mac_escape.key.mask, argv[1]);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Unable to translate MASK address %s", argv[1]);
            }
            p_db_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(&p_mac_escape.key);
            if (NULL == p_db_mac_escape)
            {
                CFG_INVALID_PARAM_RET("Mac escape entry not exist");
            }
            rc = _mirror_mac_escape_del_mac_escape(p_db_mac_escape, para);
        }
        else
        {
            return PM_E_INVALID_PARAM;
        }
       break; 
       
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _mirror_mac_escape_get_mac_escape(para, p_node, &field);
        break;
       
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;  
    }
    return rc;
}

#endif /* CONFIG_MIRROR */

