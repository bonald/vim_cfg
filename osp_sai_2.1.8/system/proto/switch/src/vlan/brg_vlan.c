
#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_pvlan_define.h"
#include "gen/tbl_pvlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "switch.h"
#include "mirror.h"
#include "if_mgr.h"
#include "brg_if.h"
#include "brg_vlan.h"
#include "brg_fdb.h"
#include "switch_api.h"

//#include "cJSON.h"


static int32
_brg_vlan_clear_vlan_process(vid_t vid)
{
    /* 1. remove multicast FDB */
    brg_fdb_flush_l2mc_fdb_port_vlan(0, vid);
    /* 2. remove static FDB */
    brg_fdb_flush_static_fdb_port_vlan(0, vid);
    /* 3. flush dynamic FDB */
    brg_fdb_flush_dynamic_fdb_port_vlan(0, vid);
    /* 4. flush port security static FDB*/
    brg_psfdb_flush_static_fdb_port_vlan(0, vid);

    return PM_E_NONE;
}

static int32
_brg_vlan_del_dhcsnooping_process(tbl_vlan_t *p_db_vlan)
{
    p_db_vlan->dhcsnooping = 0;
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_DHCSNOOPING);

    return PM_E_NONE;
}

static int32
_brg_vlan_del_port_process(tbl_vlan_t *p_db_vlan, tbl_interface_t *p_db_if, uint32 fdb_flush)
{
    ds_brgif_t brgif;
    ds_brgif_t *p_db_brgif = p_db_if->brgif;
    int32 rc = PM_E_NONE;

    sal_memset(&brgif, 0, sizeof(brgif));    
    switch (p_db_if->vlan_type)
    {
    case GLB_VLAN_PORT_TYPE_ACCESS:
        if (p_db_brgif->pvid == p_db_vlan->key.vid)
        {
            brg_if_set_pvid(p_db_if, GLB_VLAN_DEFAULT_VID, TRUE);    /* iter_member is TODO */
        }
        else
        {
            PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, p_db_vlan->key.vid, FALSE, fdb_flush, TRUE));
            sal_memcpy(&brgif.allowed_vlan, &p_db_brgif->allowed_vlan, sizeof(brgif.allowed_vlan));
            GLB_BMP_UNSET(brgif.allowed_vlan.vlan_bmp, p_db_vlan->key.vid);
            PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, TRUE));
        }
        break;
        
    case GLB_VLAN_PORT_TYPE_TRUNK:
        PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, p_db_vlan->key.vid, FALSE, fdb_flush, TRUE));
        sal_memcpy(&brgif.allowed_vlan, &p_db_brgif->allowed_vlan, sizeof(brgif.allowed_vlan));
        GLB_BMP_UNSET(brgif.allowed_vlan.vlan_bmp, p_db_vlan->key.vid);
        PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, TRUE));
        break;
        
    case GLB_VLAN_PORT_TYPE_QINQ:
        PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, p_db_vlan->key.vid, FALSE, fdb_flush, TRUE));
        sal_memcpy(&brgif.allowed_vlan, &p_db_brgif->allowed_vlan, sizeof(brgif.allowed_vlan));
        GLB_BMP_UNSET(brgif.allowed_vlan.vlan_bmp, p_db_vlan->key.vid);
        PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, TRUE));
        break;
        
    default:
        break;
    }

    return PM_E_NONE;
}

static int32
_brg_vlan_del_process(tbl_vlan_t *p_db_vlan)
{
    tbl_interface_master_t *p_if_master = tbl_interface_get_master();
    tbl_interface_key_t if_key;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

#ifdef CONFIG_MIRROR
    /* 1. clear mirror session  */
    mirror_del_vlan_process(p_db_vlan->key.vid, FALSE);
#endif /* CONFIG_MIRROR */

    _brg_vlan_del_dhcsnooping_process(p_db_vlan);

    /* 2. delete allowed vlan */
    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_db_if)
        {
            continue;
        }
        _brg_vlan_del_port_process(p_db_vlan, p_db_if, FALSE);
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    PM_E_RETURN(_brg_vlan_clear_vlan_process(p_db_vlan->key.vid));

    /* 3. remove trunk pvid */
    CTCLIB_SLIST_LOOP(p_if_master->if_list, p_db_if, listnode)
    {
        p_db_brgif = p_db_if->brgif;
        if (NULL == p_db_brgif)
        {
            continue;
        }
        
        if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            if (p_db_vlan->key.vid == p_db_brgif->pvid)
            {
                brg_if_set_pvid(p_db_if, GLB_VLAN_DEFAULT_VID, TRUE);    /* iter_member is TODO */
            }
        }

        if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
        {
            if (p_db_vlan->key.vid == p_db_brgif->pvid || p_db_vlan->key.vid == p_db_brgif->inner_native_vid)
            {
                brg_if_set_pvid(p_db_if, GLB_VLAN_DEFAULT_VID, TRUE);
            }
        }
    }
    
    /* Added by kcao 2016-11-15, remove vlan should be called after member_port is removed */
    PM_E_RETURN(swth_api_del_vlan_cb(p_db_vlan->key.vid));
    
    /* 4. remove vlan interface */
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_VLAN_PREFIX"%u", p_db_vlan->key.vid);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (p_db_if)
    {
        if_mgr_del_vlan_if(p_db_if);
    }
    
    return rc;
}

int32
brg_vlan_add_vlan(vid_t vid)
{
    tbl_vlan_key_t key;
    tbl_vlan_t     vlan;
    tbl_vlan_t *p_db_vlan = NULL;
    int32 rc = PM_E_NONE;

    key.vid = vid;
    /* 1. check exist */
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (p_db_vlan)
    {
        return PM_E_NONE;
    }

    sal_memset(&vlan, 0, sizeof(vlan));
    vlan.key.vid = vid;
    if (GLB_DEF_VLAN_ID == vid)
    {
        sal_snprintf(vlan.name, VLANNAME_SIZE, "default");
    }
    else
    {
        sal_snprintf(vlan.name, VLANNAME_SIZE, "VLAN%04d", vid);
    }
    vlan.enable = TRUE;
    vlan.instance = 0;
    
    /* 2. add to DB */
    PM_E_RETURN(tbl_vlan_add_vlan(&vlan));

    /* 4. set vlan learning enable */
    vlan.mac_learning_en = TRUE;
    PM_E_RETURN(brg_vlan_set_mac_learning_en(&vlan));

    /* 5. set mac-limit default action*/
    vlan.security_action = GLB_VLAN_SECURITY_ACTION_FORWARD;
    PM_E_RETURN(brg_vlan_set_security_action(&vlan, FALSE));

    /* 6. set mac-limit maximum mac count */
    vlan.max_mac_count = GLB_VLAN_SEC_SPEC_MAX_MAC_COUNT;
    PM_E_RETURN(brg_vlan_set_max_mac_count(&vlan, FALSE));
    
    /* 7. add allowed all ports */
    PM_E_RETURN(brg_if_add_vlan_for_allowed_all_port(vid));

    /* 8. add vlan to instance 0 */
    PM_E_RETURN(swth_api_add_vlan_cb(vid));
        
    return rc;
}

int32
brg_vlan_security_log(vid_t vid, mac_addr_t src_mac, mac_addr_t dest_mac, char *if_name)
{
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    int32 rc = PM_E_NONE;

    key.vid = vid;
    
    /* 1. check exist */
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (!p_db_vlan)
    {
        return PM_E_NONE;
    }
    
    VLAN_SEC_LOG("Vlan Security action     : Warn\n");
    
    VLAN_SEC_LOG("Packet from vlan         : %d\n", vid);
    VLAN_SEC_LOG("Packet source MAC address: %02x.%02x.%02x.%02x.%02x.%02x\n",
                 src_mac[0], src_mac[1], src_mac[2],
                 src_mac[3], src_mac[4], src_mac[5]);
    VLAN_SEC_LOG("Packet dest MAC address  : %02x.%02x.%02x.%02x.%02x.%02x\n", 
                 dest_mac[0], dest_mac[1], dest_mac[2],
                 dest_mac[3], dest_mac[4], dest_mac[5]);
    VLAN_SEC_LOG("Packet from interface    : %s\n", if_name);

    return rc;
}

int32
brg_vlan_del_vlan(vid_t vid)
{
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_pvlan_t *p_db_pvlan = NULL;
    tbl_vlan_key_t key;
    tbl_pvlan_key_t pvlan_key;
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    
    key.vid = vid;
    pvlan_key.primary_vid = vid;
    /* 1. check exist */
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (!p_db_vlan)
    {
        return PM_E_NONE;
    }
    p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
    
    if ((!p_db_pvlan) && (!p_db_vlan))
    {
        return PM_E_NONE;
    }
    if (p_db_pvlan)
    {
        sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
        dump_args.argv[0] = &vid;
        PM_E_RETURN(tbl_interface_iterate((TBL_ITER_CB_FUNC)if_mgr_init_pvlan_handle, &dump_args));
    }
    
    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    dump_args.argv[0] = &vid;
    PM_E_RETURN(tbl_interface_iterate((TBL_ITER_CB_FUNC)if_mgr_init_trunk_untag_handle, &dump_args));
   
    PM_E_RETURN(_brg_vlan_del_process(p_db_vlan));
    
    /* 2. delete from DB */
    PM_E_RETURN(tbl_vlan_del_vlan(&key));
    
    return rc;
}

int32
brg_vlan_set_name(tbl_vlan_t *p_vlan)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    /* no need to process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_NAME));
    return rc;
}

int32
brg_vlan_set_enable(tbl_vlan_t *p_vlan)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    int32 rc = PM_E_NONE;

    vlan_key.vid = p_vlan->key.vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_NOT_EXIST;
    }
    

    /* Added by kcao for 29836, flush FDB should be after disable vlan */
    if (!p_vlan->enable)
    {
        PM_E_RETURN(brg_fdb_flush_dynamic_fdb_vlan(p_vlan->key.vid));
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_ENABLE));

    if (!p_db_vlan->enable)
    {
        swth_vlan_disable_cb(p_db_vlan);
    }
    
    PM_E_RETURN(if_mgr_vlan_if_running_update(p_db_vlan));
    return rc;
}

int32
brg_vlan_set_mac_learning_en(tbl_vlan_t *p_vlan)
{
    int32 rc = PM_E_NONE;

    /* update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_MAC_LEARNING_EN));
    
    return rc;
}

int32
brg_vlan_set_security_action(tbl_vlan_t *p_vlan, bool notify)
{
    int32 rc = PM_E_NONE;
    int32 rt_val = PM_E_NONE;

    if (notify)
    {
        if (rt_val)
        {
            return rt_val;
        }
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_SECURITY_ACTION));
    
    return rc;
}

int32
brg_vlan_set_max_mac_count(tbl_vlan_t *p_vlan, bool notify)
{
    int32 rc = PM_E_NONE;
    int32 rt_val = PM_E_NONE;

    /* 1. process */
    if (notify)
    {
        if (rt_val)
        {
            return rt_val;
        }
    }
    
    /* 2. update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_MAX_MAC_COUNT));
    
    return rc;
}

int32
brg_vlan_stats_operation(tbl_vlan_t *p_vlan)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *p_sys_global = NULL;

    /* set vlan stats cnt */
    p_sys_global = tbl_sys_global_get_sys_global();
    if (1 == p_vlan->stats_en)
    {
        p_sys_global->vlan_stats_cnt++;
    }
    else
    {
        p_sys_global->vlan_stats_cnt--;
    }
    /*modify by zhaoqd for bug 52835 2019-07-29*/
    tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT);
    
    /* update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_vlan, TBL_VLAN_FLD_STATS_EN));
    
    return rc;
}
int32
brg_vlan_get_cur_mac_count(tbl_vlan_t *p_vlan)
{
   // int32 rc = PM_E_NONE;

    /* 1. process */
    //PM_E_RETURN(fei_vlan_security_get_cur_mac_num(p_vlan->key.vid, &p_vlan->cur_mac_count));

    return 0;
}

int32
brg_vlan_set_arpinsp_enable_callback(tbl_vlan_t* p_db_vlan, bool arpinsp_enable)
{
    int32 rc = 0;
    if (NULL == p_db_vlan)
    {
        return PM_E_NONE;
    }
    p_db_vlan->arpsnooping = arpinsp_enable;
    /* update local CDB */
    PM_E_RETURN(tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_ARPSNOOPING));
    return PM_E_NONE;
}

int32
brg_vlan_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_VLAN, brg_vlan_cmd_process_vlan);
    cdb_register_cfg_tbl_cb(TBL_PVLAN, brg_pvlan_cmd_process_pvlan);
    cdb_register_cfg_act_cb(ACT_SHOW_VLAN, brg_vlan_cmd_process_show_vlan);
    cdb_register_cfg_act_cb(ACT_SHOW_DOT1Q_ETHERTYPE, brg_vlan_cmd_process_show_dot1q_ethertype);
    cdb_pm_subscribe_tbl(TBL_ARPINSP, brg_vlan_cmd_arpinsp_enable_sync, NULL);
    return PM_E_NONE;
}

int32
brg_vlan_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_VLAN, NULL);
    cdb_register_cfg_tbl_cb(TBL_PVLAN, NULL);
    return PM_E_NONE;
}


