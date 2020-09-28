
#include "proto.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "brg_fdb.h"
#include "swth_sys.h"
#include "switch_api.h"
#include "glb_l2_define.h"
#include "vlan_mapping.h"

mac_addr_t brg_bpdu_mac_address =  {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
mac_addr_t brg_bcast_mac_address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


extern int32
brg_fdb_cmd_process_l2_action(char **argv, int32 argc, cfg_cmd_para_t *para);

static int32
_brg_macfilter_check_spec(cfg_cmd_para_t *para)
{
    tbl_macfilter_master_t *p_master = tbl_macfilter_get_master();
    uint32 max = 0;
    uint32 curr = 0;

    curr = p_master->macflt_list->count;
    max = lib_spec_get_field(TBL_SYS_SPEC_FLD_MAC_FILTER);
    if (curr >= max)
    {
        CFG_CONFLICT_RET("MAC filter reach to max %u entires", max);
    }

    return PM_E_NONE;
}

static int32
_brg_get_ordinary_fdb_number(tbl_fdb_t* p_fdb, tbl_iter_args_t *pargs)
{
    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && !GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        (*((uint32 *)pargs->argv[0])) ++;
    }
    return PM_E_NONE;
}

static int32
_brg_psfdb_and_fdb_check_spec(cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    tbl_psfdb_master_t *p_psfdb_master = tbl_psfdb_get_master();
    
    uint32 max = p_brg_glb->max_static_fdb;
    uint32 security_fdb_num = 0;
    uint32 ordinary_fdb_num = 0;
    uint32 curr_fdb_num = 0;
    
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &ordinary_fdb_num;
    
    tbl_fdb_iterate((TBL_ITER_CB_FUNC)_brg_get_ordinary_fdb_number, &args);

    security_fdb_num =  p_psfdb_master->psfdb_hash->count;
    curr_fdb_num = security_fdb_num + ordinary_fdb_num;
    if (curr_fdb_num >= max)
    {
        CFG_CONFLICT_RET("Static FDB number reach to max %u entires", max);
    }

    return PM_E_NONE;
}

static int32
_brg_mcfdb_check_spec(cfg_cmd_para_t *para)
{
    tbl_mcfdb_master_t *p_master = tbl_mcfdb_get_master();
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    uint32 max = 0;
    uint32 curr = 0;

    curr = p_master->mcfdb_hash->count;
    max = p_brg_glb->max_l2mcast_fdb;
    if (curr >= max)
    {
        CFG_CONFLICT_RET("L2MC FDB reach to max %u entires", max);
    }

    return PM_E_NONE;
}

static int32
_brg_fdb_cmd_check_mac_address(const uint8 *mac, cfg_cmd_para_t *para)
{
    char mac_str[MAC_ADDR_STR_LEN];
    uint8 mac5 = mac[5];
    int32 rc = PM_E_NONE;
    
    if (0 == sal_memcmp(brg_bpdu_mac_address, mac, 5))
    {
        if (0x00 == mac5 || 0x02 == mac5 || 0x10 == mac5 || 0x20 == mac5 || 0x21 == mac5)
        {
            CFG_INVALID_PARAM_RET("Cannot configure special MAC address %s", 
                cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, mac));
        }
    }
#if 0
    /* Deleted by kcao for support IGMP Snooping */
    else if (GLB_IS_MCAST_MAC(mac)&&(!GLB_IS_BCAST_MAC(mac)))
    {
        CFG_INVALID_PARAM_RET("Cannot configure multicast MAC address %s", 
            cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, mac));
    }
#endif
    else if (0 == sal_memcmp(brg_bcast_mac_address, mac, 6))
    {
        CFG_INVALID_PARAM_RET("Cannot configure broadcast MAC address %s", 
            cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, mac));
    }
     
    rc = swth_sys_check_system_mac(mac);
    if (rc < 0)
    {
        CFG_INVALID_PARAM_RET("Cannot configure system MAC address %s", 
            cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, mac));
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_cmd_add_fdb(tbl_fdb_t *p_fdb, char *ifname, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_interface_key_t if_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_fdb_t *p_db_fdb = NULL;
    ds_brgif_t* p_brgif = NULL;
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    tbl_mcfdb_t mcfdb;
    tbl_macfilter_t* p_db_macfilter = NULL;
    tbl_macfilter_master_t* p_macfilter_master = tbl_macfilter_get_master(); 
    ctclib_slistnode_t *listnode = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 igsp_en = FALSE;
    int32 rc = PM_E_NONE;

    IFNAME_ETH2FULL(ifname, ifname_ext);

    vlan_key.vid = p_fdb->key.fid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u does not exist", p_fdb->key.fid);
    }

    if (p_db_vlan->g8032_ring_id)
    {
        CFG_CONFLICT_RET("Vlan %u has been configured as the control vlan of g8032 ring %d", p_fdb->key.fid, p_db_vlan->g8032_ring_id);
    }

    rc = _brg_fdb_cmd_check_mac_address(p_fdb->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }

    /*added by yejl to fix bug38773*/
    CTCLIB_SLIST_LOOP(p_macfilter_master->macflt_list, p_db_macfilter, listnode)
    {
        if (0 == sal_memcmp(p_db_macfilter->key.mac, p_fdb->key.mac, MAC_ADDR_LEN))
        {
            CFG_CONFLICT_RET("Conflict with blackhole MAC address entry");
        }
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", ifname_ext);
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", ifname_ext);
    }

    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", ifname_ext);
    }

    if (!GLB_BMP_ISSET(p_db_if->brgif->allowed_vlan.vlan_bmp, p_fdb->key.fid))
    {
        CFG_CONFLICT_RET("Interface %s is not belong to vlan %u", ifname_ext, p_fdb->key.fid);
    }

    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Cannot add static FDB on MLAG interface");
    }

    p_brgif = ds_brgif_get_brgif(p_db_if);
    if (NULL == p_brgif)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", ifname_ext);
    }
    if(p_brgif->port_security_en)
    {
        CFG_CONFLICT_RET("Interface %s port security enabled", ifname_ext);
    }

    /* add for bug48422 add check for qinq&vlan translation */
    if (p_brgif->qinq_type == GLB_VLAN_QINQ_TYPE_SELECTIVE || p_brgif->vlan_translation_en)
    {
        if (!sal_strlen(p_brgif->vlan_mapping_table))
        {
            CFG_CONFLICT_RET("Interface %s does not apply vlan mapping table", ifname_ext);
        }
        else
        {
            if (!vlanmap_api_check_mapped_vlan_exist(p_brgif->vlan_mapping_table, vlan_key.vid))
            {
                CFG_CONFLICT_RET("Interface %s does not have this mapped vlan entry in its vlan mapping table", ifname_ext);
            }
        }
    }
    
    p_fdb->ifindex = p_db_if->ifindex;
    GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_STATIC);

    if (GLB_IS_MCAST_MAC(p_fdb->key.mac))
    {
        sal_memcpy(&mcfdb.key, &p_fdb->key, sizeof(fdb_key_t));
        /* 1. check exist */
        p_db_mcfdb = tbl_mcfdb_get_mcfdb(&mcfdb.key);
        if (NULL == p_db_mcfdb)
        {
            /* check resource */
            rc = _brg_mcfdb_check_spec(para);
            if (rc < 0)
            {
                return rc;
            }
        }

        /* 2. check igmp snooping enable */
        igsp_en = swth_api_is_igmp_snooping_en(p_fdb->key.fid);
        if (igsp_en)
        {
            CFG_CONFLICT_RET("IGMP Snooping is enabled on VLAN %u. Can't add static multicast addresses", p_fdb->key.fid);
        }

        mcfdb.manual = TRUE;
        rc = brg_mcfdb_add_mcfdb(&mcfdb, p_db_if->ifindex);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Add L2MC FDB fail, %s", pm_err_desc(rc));
        }
    }
    else
    {
        /* check duplicate FDB */
        p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
        if (NULL != p_db_fdb)
        {
            if (p_db_fdb->ifindex == p_fdb->ifindex)
            {
                CFG_CONFLICT_RET("Duplicate MAC address entry");
            }
            else if(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
            {
                CFG_CONFLICT_RET("Secure MAC address entry exist");
            }
            else if(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_HYBRID_FLOW))
            {
                /* add check with static fdb for bug 52977 */
                CFG_CONFLICT_RET("Hybird extend fdb flow exist");
            }
        }
        else
        {
            /* modified by yejl, check resource: the total number of fdb and security can exceed 1024*/
            rc = _brg_psfdb_and_fdb_check_spec(para);
            if (rc < 0)
            {
                return rc;
            }
        }
            
        rc = brg_fdb_add_fdb(p_fdb);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Add FDB fail, %s", pm_err_desc(rc));
        }
    }

    return rc;
}

static int32
_brg_fdb_cmd_del_fdb(tbl_fdb_t *p_fdb, char *ifname, cfg_cmd_para_t *para)
{
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    tbl_mcfdb_t mcfdb;
    char db_ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    uint32 deleted = FALSE;
    int32 rc = PM_E_NONE;

    rc = _brg_fdb_cmd_check_mac_address(p_fdb->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }

    if (GLB_IS_MCAST_MAC(p_fdb->key.mac))
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
        }

        sal_memcpy(&mcfdb.key, &p_fdb->key, sizeof(fdb_key_t));
        p_db_mcfdb = tbl_mcfdb_get_mcfdb(&mcfdb.key);
        if (NULL == p_db_mcfdb)
        {
            CFG_CONFLICT_RET("L2MC FDB does not exist");
        }
        if (!p_db_mcfdb->manual)
        {
            CFG_CONFLICT_RET("L2MC FDB is learnt from IGMP Snooping");
        }
        if (!GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, p_db_if->ifindex))
        {
            CFG_CONFLICT_RET("L2MC FDB does not exist at interface %s", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        
        rc = brg_mcfdb_del_mcfdb(&mcfdb, p_db_if->ifindex, &deleted);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Delete L2MC FDB fail, %s", pm_err_desc(rc));
        }
    }
    else
    {
        p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
        if (NULL == p_db_fdb)
        {
            CFG_CONFLICT_RET("FDB entry does not exist");
        }

        sal_memset(db_ifname, 0, sizeof(db_ifname));
        rc = tbl_interface_get_name_by_ifindex(p_db_fdb->ifindex, db_ifname, IFNAME_SIZE);
        if (0 != sal_strncmp(db_ifname, ifname, IFNAME_SIZE))
        {
            CFG_CONFLICT_RET("FDB is exist at interface %s", IFNAME_ETH2FULL(db_ifname, ifname_ext));
        }
        if(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            CFG_CONFLICT_RET("FDB is is Secure FDB");
        }
        
        rc = brg_fdb_del_fdb(p_fdb);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Delete FDB fail, %s", pm_err_desc(rc));
        }
    }
    
    return rc;
}

/* Added by kcao for bug 30677, sort running-config with vlan order */
static int32
_tbl_fdb_sort_by_vlan(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    ctclib_slist_t **pp_list = pargs->argv[0];
    ctclib_slist_t *p_list = NULL;

    p_list = pp_list[p_fdb->key.fid];
    if (p_list)
    {
        ctclib_slistnode_insert(p_list, p_fdb);
    }

    return PM_E_NONE;
}

static int32
_brg_fdb_cmd_get_fdb(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_fdb_t *p_fdb = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slist_t *vlan_list[GLB_VLAN_MAX+1];
    FILE *fp = NULL;
    vid_t vid = 0;
    tbl_iter_args_t args;

    /* 1. init 4k slist */
    sal_memset(vlan_list, 0, sizeof(vlan_list));
    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        vlan_list[vid] = ctclib_slist_create(NULL, NULL);
    }

    /* 2. sort by vlan */
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = vlan_list;
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)_tbl_fdb_sort_by_vlan, &args);

    /* 3. get by vlan */    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        CTCLIB_SLIST_LOOP(vlan_list[vid], p_fdb, listnode)
        {
            tbl_fdb_dump_one(p_fdb, &args);
        }
    }    
    sal_fclose(fp);

    /* 4. deinit 4k slist */
    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        ctclib_slist_delete(vlan_list[vid]);
    }

    return PM_E_NONE;
}

static int32
_brg_psfdb_cmd_get_psfdb(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_psfdb_t *p_psfdb = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slist_t *vlan_list[GLB_VLAN_MAX+1];
    FILE *fp = NULL;
    vid_t vid = 0;
    tbl_iter_args_t args;

    /* 1. init 4k slist */
    sal_memset(vlan_list, 0, sizeof(vlan_list));
    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        vlan_list[vid] = ctclib_slist_create(NULL, NULL);
    }

    /* 2. sort by vlan */
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = vlan_list;
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)_tbl_fdb_sort_by_vlan, &args);

    /* 3. get by vlan */    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        CTCLIB_SLIST_LOOP(vlan_list[vid], p_psfdb, listnode)
        {
            tbl_psfdb_dump_one(p_psfdb, &args);
        }
    }    
    sal_fclose(fp);

    /* 4. deinit 4k slist */
    for (vid = 0; vid <= GLB_VLAN_MAX; vid++)
    {
        ctclib_slist_delete(vlan_list[vid]);
    }

    return PM_E_NONE;
}

static int32
_brg_fdb_cmd_get_mcfdb(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_mcfdb_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_brg_psfdb_cmd_add_fdb(tbl_psfdb_t *p_psfdb, char *ifname, cfg_cmd_para_t *para)
{
    tbl_vlan_key_t vlan_key;
    tbl_interface_key_t if_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_psfdb_t * p_db_psfdb = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;
    
    vlan_key.vid = p_psfdb->key.fid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        CFG_CONFLICT_RET("Vlan %u does not exist", p_psfdb->key.fid);
    }

    rc = _brg_fdb_cmd_check_mac_address(p_psfdb->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);
    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", ifname_ext);
    }

    IF_MGR_CHECK_LAG_MEMBER(p_db_if);
    
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", ifname_ext);
    }

    if (GLB_IF_MODE_L2 != p_db_if->mode)
    {
        CFG_CONFLICT_RET("Interface %s is not switch port", ifname_ext);
    }

    if (!GLB_BMP_ISSET(p_db_if->brgif->allowed_vlan.vlan_bmp, p_psfdb->key.fid))
    {
        CFG_CONFLICT_RET("Interface %s is not belong to vlan %u", ifname_ext, p_psfdb->key.fid);
    }

    if (p_db_if->brgif->vlan_translation_en && !sal_strlen(p_db_if->brgif->vlan_mapping_table))
    {
        CFG_CONFLICT_RET("Interface %s does not apply vlan mapping table", ifname_ext);
    }

    if (GLB_IS_MCAST_MAC(p_psfdb->key.mac))
    {
        CFG_FUNC_ERR_RET("Can't confiugre multicast FDB for security port entry");
    }
    if (GLB_IS_BCAST_MAC(p_psfdb->key.mac))
    {
        CFG_FUNC_ERR_RET("Can't confiugre broadcast FDB for security port entry");
    }
    
    if (p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Cannot add static security MAC on MLAG interface");
    }
    
    /*GB*/
    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        if (p_db_if->static_security_num >= p_db_if->brgif->max_mac)
        {
            CFG_CONFLICT_RET("The configured port security address number of interface %s reach the maximum", ifname_ext);
        }
    }
    
    p_psfdb->ifindex = p_db_if->ifindex;
    GLB_SET_FLAG(p_psfdb->flags, GLB_FDB_FLAG_STATIC);
    GLB_SET_FLAG(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
    
    /* check duplicate FDB */

    p_db_psfdb = tbl_psfdb_get_psfdb(&p_psfdb->key);
    if (NULL != p_db_psfdb)
    {
        if ((GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))&&(p_db_psfdb->ifindex == p_psfdb->ifindex))
        {
            CFG_CONFLICT_RET("Duplicate MAC address entry");
        }
        if(GLB_FLAG_ISSET(p_db_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            CFG_CONFLICT_RET("Secure MAC address entry exist");
        }
        brg_psfdb_del_psfdb(p_db_psfdb);
        p_db_psfdb = NULL;
    }
    else
    {
        /* modified by yejl, check resource: the total number of fdb and security can exceed 1024*/
        rc = _brg_psfdb_and_fdb_check_spec(para);
        if (rc < 0)
        {
            return rc;
        }
    }
    
    rc = brg_psfdb_add_psfdb(p_psfdb);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add PSFDB fail, %s", pm_err_desc(rc));
    }
  
    return rc;
}


static int32
_brg_psfdb_cmd_del_fdb(tbl_psfdb_t *p_psfdb, char *ifname, cfg_cmd_para_t *para)
{
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_psfdb_t *p_db_psfdb = NULL;
    tbl_interface_t *p_db_if = NULL;
    char db_ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    int32 rc = PM_E_NONE;
    
    rc = _brg_fdb_cmd_check_mac_address(p_psfdb->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }

    p_db_psfdb = tbl_psfdb_get_psfdb(&p_psfdb->key);
    if (NULL == p_db_psfdb)
    {
        CFG_CONFLICT_RET("Port security FDB entry does not exist");
    }

    sal_memset(db_ifname, 0, sizeof(db_ifname));
    rc = tbl_interface_get_name_by_ifindex(p_db_psfdb->ifindex, db_ifname, IFNAME_SIZE);
    if (0 != sal_strncmp(db_ifname, ifname, IFNAME_SIZE))
    {
        CFG_CONFLICT_RET("FDB is exist at interface %s", IFNAME_ETH2FULL(db_ifname, ifname_ext));
    }

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s does not exist", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    
    p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
    if (p_db_fdb)
    {
        rc = brg_fdb_del_fdb(p_db_fdb);
        if (rc < 0)
        {
            CFG_FUNC_ERR_RET("Delete static FDB fail, %s", pm_err_desc(rc));
        }
    }
    rc = brg_psfdb_del_psfdb(p_psfdb);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Delete static port security FDB fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_brg_macfilter_cmd_add_macfilter(tbl_macfilter_t *p_macflt, cfg_cmd_para_t *para)
{
    tbl_macfilter_t *p_db_macflt = NULL;
    tbl_fdb_t* p_db_fdb = NULL;
    tbl_fdb_master_t* p_master = tbl_fdb_get_master();
    ctclib_hash_backet_t *hb_next = NULL;
    ctclib_hash_backet_t *pst_hb = NULL;
    int32 index = 0;
    int32 rc = PM_E_NONE;

    if (GLB_IS_MCAST_MAC(p_macflt->key.mac))
    {
        CFG_CONFLICT_RET("Cannot configure multicast MAC filter entry");
    }

    rc = _brg_fdb_cmd_check_mac_address(p_macflt->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }

    rc = _brg_macfilter_check_spec(para);
    if (rc < 0)
    {
        return rc;
    }

    /*added by yejl to fix bug38773*/
    for (index = 0; index < p_master->fdb_hash->size; index++)
    {
        for (pst_hb = p_master->fdb_hash->index[index]; pst_hb; pst_hb = hb_next)
        {
            hb_next = pst_hb->next;
            if (!pst_hb->data)
            {
                continue;
            }
            p_db_fdb = (tbl_fdb_t*)pst_hb->data;
            if (0 == sal_memcmp(p_db_fdb->key.mac, p_macflt->key.mac, MAC_ADDR_LEN))
            {
                if(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_HYBRID_FLOW))
                {
                    CFG_CONFLICT_RET("Conflict with Hybird extend fdb flow");
                }
                else
                {
                    CFG_CONFLICT_RET("Conflict with static MAC address entry");
                }
            }
        }
    }

    /* check duplicate FDB */
    p_db_macflt = tbl_macfilter_get_macfilter(&p_macflt->key);
    if (NULL != p_db_macflt)
    {
        CFG_CONFLICT_RET("Duplicate MAC filter entry");
    }

    rc = brg_macfilter_add_macfilter(p_macflt);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add MAC filter fail, %s", pm_err_desc(rc));
    }

    return rc;
}

static int32
_brg_macfilter_cmd_del_macfilter(tbl_macfilter_t *p_macflt, cfg_cmd_para_t *para)
{
    tbl_macfilter_t *p_db_macflt = NULL;
    int32 rc = PM_E_NONE;

    if (GLB_IS_MCAST_MAC(p_macflt->key.mac))
    {
        CFG_CONFLICT_RET("Cannot configure multicast MAC filter entry");
    }

    rc = _brg_fdb_cmd_check_mac_address(p_macflt->key.mac, para);
    if (rc < 0)
    {
        return rc;
    }

    /* check duplicate FDB */
    p_db_macflt = tbl_macfilter_get_macfilter(&p_macflt->key);
    if (NULL == p_db_macflt)
    {
        CFG_CONFLICT_RET("MAC filter entry does not exist");
    }

    rc = brg_macfilter_del_macfilter(p_macflt);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add MAC filter fail, %s", pm_err_desc(rc));
    }

    return rc;
}

static int32
_brg_macfilter_cmd_get_macfilter(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_macfilter_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
brg_fdb_cmd_process_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_fdb_t fdb;
    char *ifname = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(para->p_msg->u_hdr.cfg.id);

    sal_memset(&fdb, 0, sizeof(fdb));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        rc = cdb_fdb_key_str2val(&fdb.key, argv[0], para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            return rc;
        }
        ifname = argv[2];
        rc = _brg_fdb_cmd_add_fdb(&fdb, ifname, para);
        
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        rc = cdb_fdb_key_str2val(&fdb.key, argv[0], para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            return rc;
        }
        ifname = argv[2];
        rc = _brg_fdb_cmd_del_fdb(&fdb, ifname, para);
        
        break;

    case CDB_OPER_SET:
        CFG_INVALID_PARAM_RET("Not support to set FDB");
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        if (TBL_FDB == p_node->id)
        {
            rc = _brg_fdb_cmd_get_fdb(para, p_node, &field);
        }
        else
        {
            rc = _brg_fdb_cmd_get_mcfdb(para, p_node, &field);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
brg_fdb_cmd_process_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    char mac_str[64];
    tbl_macfilter_t macflt;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MACFILTER);

    sal_memset(&macflt, 0, sizeof(macflt));
    sal_memset(mac_str, 0, sizeof(mac_str));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        
        /*added by yejl to fix bug38776*/
        rc = cdb_mac_str2val(macflt.key.mac, argv[0]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", mac_str);
        }
        rc = _brg_macfilter_cmd_add_macfilter(&macflt, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        
        /*added by yejl to fix bug38776*/
        rc = cdb_mac_str2val(macflt.key.mac, argv[0]);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", mac_str);
        }
        rc = _brg_macfilter_cmd_del_macfilter(&macflt, para);
        break;

    case CDB_OPER_SET:
        CFG_INVALID_PARAM_RET("Not support to set MAC filter");
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _brg_macfilter_cmd_get_macfilter(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
brg_psfdb_cmd_process_psfdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_psfdb_t ps_fdb;
    char *ifname = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(para->p_msg->u_hdr.cfg.id);

    sal_memset(&ps_fdb, 0, sizeof(ps_fdb));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        rc = cdb_fdb_key_str2val(&ps_fdb.key, argv[0], para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            return rc;
        }
        ifname = argv[2];
        rc = _brg_psfdb_cmd_add_fdb(&ps_fdb, ifname, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        rc = cdb_fdb_key_str2val(&ps_fdb.key, argv[0], para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = rc;
            return rc;
        }
        ifname = argv[2];
        rc = _brg_psfdb_cmd_del_fdb(&ps_fdb, ifname, para);
        break;

    case CDB_OPER_SET:
        CFG_INVALID_PARAM_RET("Not support to set FDB");
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        if (TBL_PSFDB == p_node->id)
        {
            rc = _brg_psfdb_cmd_get_psfdb(para, p_node, &field);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


static int32
_brg_fdb_cmd_process_flush_dynamic_fdb(glb_l2_fdb_flush_type_t type, uint32 value, uint8 *mac)
{
    uint32 ifindex = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;
    switch (type)
    {
    case GLB_L2_FDB_FLUSH_ALL_FDB:
        rc = brg_fdb_flush_dynamic_fdb();
        break;
    case GLB_L2_FDB_FLUSH_BY_VID:
        vid = value;
        rc = brg_fdb_flush_dynamic_fdb_vlan(vid);
        break;
    case GLB_L2_FDB_FLUSH_BY_PORT:
        ifindex = value;
        rc = brg_fdb_flush_dynamic_fdb_port(ifindex);
        break;
    case GLB_L2_FDB_FLUSH_BY_MAC:
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_brg_fdb_cmd_process_flush_mcfdb(glb_l2_fdb_flush_type_t type, uint32 value, uint8 *mac)
{
    uint32 ifindex = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    switch (type)
    {
    case GLB_L2_FDB_FLUSH_ALL_FDB:
        rc = brg_fdb_flush_l2mc_fdb();
        break;
    case GLB_L2_FDB_FLUSH_BY_VID:
        vid = value;
        rc = brg_fdb_flush_l2mc_fdb_vlan(vid);
        break;
    case GLB_L2_FDB_FLUSH_BY_PORT:
        ifindex = value;
        rc = brg_fdb_flush_l2mc_fdb_port(ifindex);
        break;
    case GLB_L2_FDB_FLUSH_BY_MAC:
        rc = brg_fdb_flush_l2mc_fdb_mac(mac);
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_brg_fdb_cmd_process_flush_static_fdb(glb_l2_fdb_flush_type_t type, uint32 value, uint8 *mac)
{
    uint32 ifindex = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;
    
    switch (type)
    {
    case GLB_L2_FDB_FLUSH_ALL_FDB:
        rc = brg_fdb_flush_static_fdb();
        break;
    case GLB_L2_FDB_FLUSH_BY_VID:
        vid = value;
        rc = brg_fdb_flush_static_fdb_vlan(vid);
        break;
    case GLB_L2_FDB_FLUSH_BY_PORT:
        ifindex = value;
        rc = brg_fdb_flush_static_fdb_port(ifindex);
        break;
    case GLB_L2_FDB_FLUSH_BY_MAC:
        rc = brg_fdb_flush_static_fdb_mac(mac);
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

static int32
_brg_fdb_cmd_set_flush_fdb(ds_flush_fdb_t *p_flush_fdb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    if (FLUSH_FDB_TYPE_FDB == p_flush_fdb->type)
    {
        if (FLUSH_FDB_SUBTYPE_STATIC == p_flush_fdb->subtype)
        {
            switch(p_flush_fdb->mode)
            {
            case FLUSH_FDB_MODE_ALL:
                rc = brg_fdb_flush_static_fdb();
                break;
            case FLUSH_FDB_MODE_PORT:
                rc = brg_fdb_flush_static_fdb_port(p_flush_fdb->ifindex);
                break;
            case FLUSH_FDB_MODE_VLAN:
                rc = brg_fdb_flush_static_fdb_vlan(p_flush_fdb->vid);
                break;
            case FLUSH_FDB_MODE_MAC:
                rc = brg_fdb_flush_static_fdb_mac(p_flush_fdb->mac);
                break;
            default:
                return PM_E_INVALID_PARAM;
            }
           return brg_fdb_set_flush_fdb(p_flush_fdb); 
        }
        else if (FLUSH_FDB_SUBTYPE_DYNAMIC == p_flush_fdb->subtype)
        {
            switch(p_flush_fdb->mode)
            {
            case FLUSH_FDB_MODE_ALL:
                rc = brg_fdb_flush_dynamic_fdb();
                break;
            case FLUSH_FDB_MODE_PORT:
                rc = brg_fdb_flush_dynamic_fdb_port(p_flush_fdb->ifindex);
                break;
            case FLUSH_FDB_MODE_VLAN:
                rc = brg_fdb_flush_dynamic_fdb_vlan(p_flush_fdb->vid);
                break;
            case FLUSH_FDB_MODE_PORT_VLAN:
                rc = brg_fdb_flush_dynamic_fdb_port_vlan(p_flush_fdb->ifindex,p_flush_fdb->vid);
                break;
            case FLUSH_FDB_MODE_MAC:
                rc = brg_fdb_flush_dynamic_fdb_mac(p_flush_fdb->mac);
                break;
            default:
                return PM_E_INVALID_PARAM;
            }
        }
        else if (FLUSH_FDB_SUBTYPE_MULTICAST == p_flush_fdb->subtype)
        {
           CFG_INVALID_PARAM_RET("Not support multicast");
        }
    }
    else if (FLUSH_FDB_TYPE_PSFDB == p_flush_fdb->type)
    {
        if (FLUSH_FDB_SUBTYPE_STATIC == p_flush_fdb->subtype)
        {
            switch(p_flush_fdb->mode)
            {
            case FLUSH_FDB_MODE_ALL:
                rc = brg_psfdb_flush_static_fdb();
                break;
            case FLUSH_FDB_MODE_PORT:
                rc = brg_psfdb_flush_static_fdb_port(p_flush_fdb->ifindex);
                break;
            case FLUSH_FDB_MODE_VLAN:
                rc = brg_psfdb_flush_static_fdb_vlan(p_flush_fdb->vid);
                break;
            case FLUSH_FDB_MODE_MAC:
                rc = brg_psfdb_flush_static_fdb_mac(p_flush_fdb->mac);
                break;
            default:
                return PM_E_INVALID_PARAM;
            }
           return brg_fdb_set_flush_fdb(p_flush_fdb); 
        }
        else 
        {
           CFG_INVALID_PARAM_RET("Not support dynamic and multicast");
        }
    }
    else if (FLUSH_FDB_TYPE_MACFILTER == p_flush_fdb->type)
    {
        return brg_macfilter_flush_macfilter();
    }
    return rc;
}

int32
brg_fdb_cmd_process_l2_action(char **argv, int32 argc, cfg_cmd_para_t *para)
{ 
    int32 ifindex = 0;
    vid_t vid = 0;
    mac_addr_t mac;
    ds_flush_fdb_t flush_fdb;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        if (0 == sal_strcmp("flush_fdb", argv[0]))
        {
            if (3 == argc)
            {
                flush_fdb.mode = FLUSH_FDB_MODE_ALL;
            }
            else if (5 == argc)
            {
                if (0 == sal_strcmp(argv[3], "interface"))
                {
                    ifindex = tbl_interface_get_ifindex_by_name(argv[4]);
                    if (ifindex < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[4]);
                    }
                    flush_fdb.ifindex = ifindex;
                    flush_fdb.mode = FLUSH_FDB_MODE_PORT;
                }
                else if (0 == sal_strcmp(argv[3], "vlan"))
                {
                    PM_CFG_GET_UINT(vid, argv[4], para->p_rs);
                    flush_fdb.vid = vid;
                    flush_fdb.mode = FLUSH_FDB_MODE_VLAN;
                }
                else if (0 == sal_strcmp(argv[3], "address"))
                {
                    sal_memset(mac, 0, sizeof(mac));
                    rc = cdb_mac_str2val(mac, argv[4]);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[4]);
                    }
                    sal_memcpy(flush_fdb.mac, mac, sizeof(mac_addr_t));
                    flush_fdb.mode = FLUSH_FDB_MODE_MAC;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid type %s", argv[3]);
                }
            }
            if (0 == sal_strcmp(argv[1], "fdb"))
            {
                flush_fdb.type = FLUSH_FDB_TYPE_FDB;
            }
            else if (0 == sal_strcmp(argv[1], "psfdb"))
            {
                flush_fdb.type = FLUSH_FDB_TYPE_PSFDB;
            }
            else if (0 == sal_strcmp(argv[1], "macfilter"))
            {
                flush_fdb.type = FLUSH_FDB_TYPE_MACFILTER;
                rc = _brg_fdb_cmd_set_flush_fdb(&flush_fdb,para);
                return rc;
            } 
            
            if (0 == sal_strcmp(argv[2], "static"))
            {
                flush_fdb.subtype = FLUSH_FDB_SUBTYPE_STATIC;
            }
            else if (0 == sal_strcmp(argv[2], "dynamic"))
            {
                 flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
            }
            else if (0 == sal_strcmp(argv[2], "multicast"))
            {
                flush_fdb.subtype = FLUSH_FDB_SUBTYPE_MULTICAST;
            }
            rc = _brg_fdb_cmd_set_flush_fdb(&flush_fdb,para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        break;

    case CDB_OPER_GET:
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
brg_fdb_cmd_process_flushfdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    glb_l2_fdb_flush_type_t type = GLB_L2_FDB_FLUSH_ALL_FDB;
    int32 ifindex = 0;
    vid_t vid = 0;
    uint32 value = 0;
    mac_addr_t mac;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }

        if (1 == argc)
        {
            type = GLB_L2_FDB_FLUSH_ALL_FDB;
        }
        else if (3 == argc)
        {
            if (0 == sal_strcmp(argv[1], "interface"))
            {
                ifindex = tbl_interface_get_ifindex_by_name(argv[2]);
                if (ifindex < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[2]);
                }
                value = ifindex;
                type = GLB_L2_FDB_FLUSH_BY_PORT;
            }
            else if (0 == sal_strcmp(argv[1], "vlan"))
            {
                PM_CFG_GET_UINT(vid, argv[2], para->p_rs);
                value = vid;
                type = GLB_L2_FDB_FLUSH_BY_VID;
            }
            else if (0 == sal_strcmp(argv[1], "address"))
            {
                sal_memset(mac, 0, sizeof(mac));
                rc = cdb_mac_str2val(mac, argv[2]);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[2]);
                }
                type = GLB_L2_FDB_FLUSH_BY_MAC;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid type %s", argv[1]);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }
        
        if (0 == sal_strcmp(argv[0], "static"))
        {
            _brg_fdb_cmd_process_flush_static_fdb(type, value, mac);
        }
        else if (0 == sal_strcmp(argv[0], "dynamic"))
        {
            _brg_fdb_cmd_process_flush_dynamic_fdb(type, value, mac);
        }
        else if (0 == sal_strcmp(argv[0], "multicast"))
        {
            _brg_fdb_cmd_process_flush_mcfdb(type, value, mac);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

int32
brg_fdb_cmd_process_flush_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    return brg_macfilter_flush_macfilter();
}


static int32
_brg_psfdb_cmd_process_flush_static_fdb(glb_l2_fdb_flush_type_t type, uint32 value, uint8 *mac)
{
    uint32 ifindex = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    switch (type)
    {
    case GLB_L2_FDB_FLUSH_ALL_FDB:
        rc = brg_psfdb_flush_static_fdb();
        break;
    case GLB_L2_FDB_FLUSH_BY_VID:
        vid = value;
        rc = brg_psfdb_flush_static_fdb_vlan(vid);
        break;
    case GLB_L2_FDB_FLUSH_BY_PORT:
        ifindex = value;
        rc = brg_psfdb_flush_static_fdb_port(ifindex);
        break;
    case GLB_L2_FDB_FLUSH_BY_MAC:
        rc = brg_psfdb_flush_static_fdb_mac(mac);
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    return rc;
}

int32
brg_psfdb_cmd_process_flushfdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    glb_l2_fdb_flush_type_t type = GLB_L2_FDB_FLUSH_ALL_FDB;
    int32 ifindex = 0;
    vid_t vid = 0;
    uint32 value = 0;
    mac_addr_t mac;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }

        if (1 == argc)
        {
            type = GLB_L2_FDB_FLUSH_ALL_FDB;
        }
        else if (3 == argc)
        {
            if (0 == sal_strcmp(argv[1], "interface"))
            {
                ifindex = tbl_interface_get_ifindex_by_name(argv[2]);
                if (ifindex < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[2]);
                }
                value = ifindex;
                type = GLB_L2_FDB_FLUSH_BY_PORT;
            }
            else if (0 == sal_strcmp(argv[1], "vlan"))
            {
                PM_CFG_GET_UINT(vid, argv[2], para->p_rs);
                value = vid;
                type = GLB_L2_FDB_FLUSH_BY_VID;
            }
            else if (0 == sal_strcmp(argv[1], "address"))
            {
                sal_memset(mac, 0, sizeof(mac));
                rc = cdb_mac_str2val(mac, argv[2]);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[2]);
                }
                type = GLB_L2_FDB_FLUSH_BY_MAC;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid type %s", argv[1]);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }
        
        _brg_psfdb_cmd_process_flush_static_fdb(type, value, mac);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

