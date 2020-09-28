
#include "proto.h"
#include "lib_tblinc.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "switch.h"
#include "brg_fdb.h"
#include "glb_l2_define.h"
#include "errdisable.h"
#include "gen/tbl_vlan_define.h"
#include "brg_vlan.h"

extern int32 
brg_fdb_cmd_process_l2_action(char **argv, int32 argc, cfg_cmd_para_t *para);

static int32
_brg_mcfdb_del_mcfdb_all_intf(tbl_mcfdb_t *p_mcfdb)
{
    uint32 deleted = FALSE;
    uint32 l2mc_ifindex = 0;

    GLB_BMP_ITER_BEGIN(p_mcfdb->l2mc_port, l2mc_ifindex)
    {
        brg_mcfdb_del_mcfdb(p_mcfdb, l2mc_ifindex, &deleted);
        if (deleted)
        {
            break;
        }
    }
    GLB_BMP_ITER_END(p_mcfdb->l2mc_port, l2mc_ifindex);

    return PM_E_NONE;
}

static int32
_brg_fdb_get_condition_none(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    p_get->node_array[p_get->index] = p_fdb;
    p_get->index++;
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_condition_vlan(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_fdb->key.fid)
    {
        p_get->node_array[p_get->index] = p_fdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_condition_port(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (ifindex == p_fdb->ifindex)
    {
        p_get->node_array[p_get->index] = p_fdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_condition_port_vlan(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_fdb->key.fid && ifindex == p_fdb->ifindex)
    {
        p_get->node_array[p_get->index] = p_fdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_condition_mac(tbl_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint8 *mac = (uint8*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_memcmp(mac, p_fdb->key.mac, MAC_ADDR_LEN))
    {
        p_get->node_array[p_get->index] = p_fdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}


int32
tbl_fdb_iterate_cont(uint32 hash_index, TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fdb_master_t *p_master = tbl_fdb_get_master();
    ctclib_hash_iterate2_cont(p_master->fdb_hash, hash_index, tbl_hash_iter_adpt_fn_cont, fn, pargs);
    return PM_E_NONE;
}

static int32
_brg_psfdb_get_condition_none(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    p_get->node_array[p_get->index] = p_psfdb;
    p_get->index++;
    
    return PM_E_NONE;
}

static int32
_brg_psfdb_get_condition_vlan(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_psfdb->key.fid)
    {
        p_get->node_array[p_get->index] = p_psfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_psfdb_get_condition_port(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (ifindex == p_psfdb->ifindex)
    {
        p_get->node_array[p_get->index] = p_psfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_psfdb_get_condition_port_vlan(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_psfdb->key.fid && ifindex == p_psfdb->ifindex)
    {
        p_get->node_array[p_get->index] = p_psfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_psfdb_get_condition_mac(tbl_psfdb_t *p_psfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint8 *mac = (uint8*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_memcmp(mac, p_psfdb->key.mac, MAC_ADDR_LEN))
    {
        p_get->node_array[p_get->index] = p_psfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

int32
tbl_psfdb_iterate_cont(uint32 hash_index, TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_psfdb_master_t *p_psmaster = tbl_psfdb_get_master();
    ctclib_hash_iterate2_cont(p_psmaster->psfdb_hash, hash_index, tbl_hash_iter_adpt_fn_cont, fn, pargs);
    return PM_E_NONE;
}

static int32
_brg_fdb_flush_static_fdb_condtion_port_vlan(uint32 ifindex, vid_t vid)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_fdb = NULL;
    uint32 i = 0;
    uint32 hash_index = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_port_vlan;
    if (0 == ifindex)
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_none;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_vlan;
        }
    }
    else
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_port;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_port_vlan;
        }
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vid;
    args.argv[3] = &hash_index;
    do 
    {
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_fdb_iterate_cont(hash_index, get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_fdb = get_node.node_array[i];
            if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
            {
                brg_fdb_del_fdb(p_fdb);
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}

int32
brg_fdb_set_flush_fdb(ds_flush_fdb_t *p_flush_fdb)
{
    tbl_l2_action_t *p_l2_action = NULL; 
    ds_flush_fdb_t *p_db_flush_fdb = NULL;
    int32 rc = PM_E_NONE;
    
    p_l2_action = tbl_l2_action_get_l2_action();
    p_db_flush_fdb = ds_flush_fdb_get_flush_fdb(p_l2_action);

    if (NULL == p_db_flush_fdb)
    {
        rc = ds_flush_fdb_add_flush_fdb(p_l2_action, p_flush_fdb);
    }
    rc = ds_flush_fdb_set_flush_fdb_field(p_l2_action, p_flush_fdb, DS_FLUSH_FDB_FLD_MAX);

    return rc;
}

static int32
_brg_fdb_get_l2mc_condition_none(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    p_get->node_array[p_get->index] = p_mcfdb;
    p_get->index++;
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_l2mc_condition_vlan(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_mcfdb->key.fid)
    {
        p_get->node_array[p_get->index] = p_mcfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_l2mc_condition_port(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (GLB_BMP_ISSET(p_mcfdb->l2mc_port, ifindex))
    {
        p_get->node_array[p_get->index] = p_mcfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_l2mc_condition_port_vlan(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint32 ifindex = *(uint32*)pargs->argv[1];
    vid_t vid = *(vid_t*)pargs->argv[2];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (vid == p_mcfdb->key.fid && GLB_BMP_ISSET(p_mcfdb->l2mc_port, ifindex))
    {
        p_get->node_array[p_get->index] = p_mcfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_get_l2mc_condition_mac(tbl_mcfdb_t *p_mcfdb, tbl_iter_args_t *pargs)
{
    cdb_get_node_t *p_get = (cdb_get_node_t*)pargs->argv[0];
    uint8 *mac = (uint8*)pargs->argv[1];

    if (p_get->index >= CDB_GET_COUNT)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_memcmp(mac, p_mcfdb->key.mac, MAC_ADDR_LEN))
    {
        p_get->node_array[p_get->index] = p_mcfdb;
        p_get->index++;
    }
    
    return PM_E_NONE;
}

static int32
_brg_fdb_flush_l2mc_fdb_condtion_port_vlan(uint32 ifindex, vid_t vid)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_mcfdb_t *p_mcfdb = NULL;
    uint32 i = 0;
    uint32 deleted = FALSE;

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_port_vlan;
    if (0 == ifindex)
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_none;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_vlan;
        }
    }
    else
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_port;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_port_vlan;
        }
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vid;
    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_mcfdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_mcfdb = get_node.node_array[i];
            if (GLB_IFINDEX_ALL == ifindex)
            {
                _brg_mcfdb_del_mcfdb_all_intf(p_mcfdb);
            }
            else
            {
                brg_mcfdb_del_mcfdb(p_mcfdb, ifindex, &deleted);
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}

int32
brg_fdb_flush_l2mc_fdb()
{
    return _brg_fdb_flush_l2mc_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, GLB_ALL_VLAN_ID);
}

int32
brg_fdb_flush_l2mc_fdb_vlan(vid_t vid)
{
    return _brg_fdb_flush_l2mc_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, vid);
}

int32
brg_fdb_flush_l2mc_fdb_port(uint32 ifindex)
{
    return _brg_fdb_flush_l2mc_fdb_condtion_port_vlan(ifindex, GLB_ALL_VLAN_ID);
}

int32
brg_fdb_flush_l2mc_fdb_port_vlan(uint32 ifindex, vid_t vid)
{
    return _brg_fdb_flush_l2mc_fdb_condtion_port_vlan(ifindex, vid);
}

int32
brg_fdb_flush_l2mc_fdb_mac(uint8 *mac)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_mcfdb_t *p_mcfdb = NULL;
    uint32 i = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_mac;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = mac;

    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_mcfdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_mcfdb = get_node.node_array[i];
            _brg_mcfdb_del_mcfdb_all_intf(p_mcfdb);
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}

int32
brg_fdb_flush_static_fdb_mac(uint8 *mac)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_fdb = NULL;
    uint32 i = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_mac;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = mac;

    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_fdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_fdb = get_node.node_array[i];
            if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
            {
                brg_fdb_del_fdb(p_fdb);
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}

int32
brg_fdb_flush_static_fdb()
{
    return _brg_fdb_flush_static_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, GLB_ALL_VLAN_ID);
}

int32
brg_fdb_flush_static_fdb_vlan(vid_t vid)
{
    return _brg_fdb_flush_static_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, vid);
}

int32
brg_fdb_flush_static_fdb_port(uint32 ifindex)
{
    return _brg_fdb_flush_static_fdb_condtion_port_vlan(ifindex, GLB_ALL_VLAN_ID);
}

int32
brg_fdb_flush_static_fdb_port_vlan(uint32 ifindex, vid_t vid)
{
    return _brg_fdb_flush_static_fdb_condtion_port_vlan(ifindex, vid);
}

int32
brg_fdb_flush_dynamic_fdb()
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_ALL;

    return brg_fdb_set_flush_fdb(&flush_fdb);
}

int32
brg_fdb_flush_dynamic_fdb_vlan(vid_t vid)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_VLAN;
    flush_fdb.vid = vid;
    return brg_fdb_set_flush_fdb(&flush_fdb);
}

int32
brg_fdb_flush_dynamic_fdb_port(uint32 ifindex)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_PORT;
    flush_fdb.ifindex = ifindex;
    return brg_fdb_set_flush_fdb(&flush_fdb);
}

int32
brg_fdb_flush_dynamic_fdb_port_vlan(uint32 ifindex, vid_t vid)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_PORT_VLAN;
    flush_fdb.ifindex = ifindex;
    flush_fdb.vid = vid;

    if(ifindex == 0)
    {
        flush_fdb.mode = FLUSH_FDB_MODE_VLAN;
    }
        
    return brg_fdb_set_flush_fdb(&flush_fdb);
}

int32
brg_fdb_flush_dynamic_fdb_mac(mac_addr_t mac)
{
    ds_flush_fdb_t flush_fdb;

    sal_memset(&flush_fdb, 0, sizeof(flush_fdb));
    flush_fdb.type = FLUSH_FDB_TYPE_FDB;
    flush_fdb.subtype = FLUSH_FDB_SUBTYPE_DYNAMIC;
    flush_fdb.mode = FLUSH_FDB_MODE_MAC;
    sal_memcpy(flush_fdb.mac, mac, sizeof(mac_addr_t));
        
    return brg_fdb_set_flush_fdb(&flush_fdb);
}

#define BRG_MCFDB_BUILD_ADD_IFINDEX(IFINDEX) (0x10000 | IFINDEX)
#define BRG_MCFDB_BUILD_DEL_IFINDEX(IFINDEX) (IFINDEX)

int32
brg_mcfdb_add_mcfdb(tbl_mcfdb_t *p_mcfdb, uint32 ifindex)
{
    tbl_mcfdb_master_t *p_master = tbl_mcfdb_get_master();
    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    int32 rc = PM_E_NONE;
    uint32 max = 0;
    uint32 curr = 0;
    
    /* 1. check exist */
    p_db_mcfdb = tbl_mcfdb_get_mcfdb(&p_mcfdb->key);
    if (NULL == p_db_mcfdb)
    {
        curr = p_master->mcfdb_hash->count;
        max = p_brg_glb->max_l2mcast_fdb;
        if (curr >= max)
        {
            return PM_E_EXCEED_MAX_NUM;
        }
        PM_E_RETURN(tbl_mcfdb_add_mcfdb(p_mcfdb));
        GLB_BMP_INIT(p_mcfdb->l2mc_port);
    }
    else
    {
        if (GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, ifindex))
        {
            return PM_E_NONE;
        }
        sal_memcpy(p_mcfdb->l2mc_port, p_db_mcfdb->l2mc_port, sizeof(port_bmp_t));
    }

    GLB_BMP_SET(p_mcfdb->l2mc_port, ifindex);
    PM_E_RETURN(tbl_mcfdb_set_mcfdb_field(p_mcfdb, TBL_MCFDB_FLD_L2MC_PORT));
    p_mcfdb->action_ifindex = BRG_MCFDB_BUILD_ADD_IFINDEX(ifindex);
    PM_E_RETURN(tbl_mcfdb_set_mcfdb_field(p_mcfdb, TBL_MCFDB_FLD_ACTION_IFINDEX));

    return rc;
}

/* deleted means the node is deleted, and member bmp loop should be break */
int32
brg_mcfdb_del_mcfdb(tbl_mcfdb_t *p_mcfdb, uint32 ifindex, uint32 *deleted)
{
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    uint32 all_zero = FALSE;
    int32 rc = PM_E_NONE;

    *deleted = FALSE;
    
    /* 1. check exist */
    p_db_mcfdb = tbl_mcfdb_get_mcfdb(&p_mcfdb->key);
    if (NULL == p_db_mcfdb)
    {
        return PM_E_NONE;
    }
    if (!GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, ifindex))
    {
        return PM_E_NOT_EXIST;
    }
    
    sal_memcpy(p_mcfdb->l2mc_port, p_db_mcfdb->l2mc_port, sizeof(port_bmp_t));
    p_mcfdb->action_ifindex = BRG_MCFDB_BUILD_DEL_IFINDEX(ifindex);
    PM_E_RETURN(tbl_mcfdb_set_mcfdb_field(p_mcfdb, TBL_MCFDB_FLD_ACTION_IFINDEX));
    GLB_BMP_UNSET(p_mcfdb->l2mc_port, ifindex);
    PM_E_RETURN(tbl_mcfdb_set_mcfdb_field(p_mcfdb, TBL_MCFDB_FLD_L2MC_PORT));

    GLB_BMP_CHECK_ALL_ZERO(p_db_mcfdb->l2mc_port, all_zero);
    if (all_zero)
    {
        PM_E_RETURN(tbl_mcfdb_del_mcfdb(&p_mcfdb->key));
        *deleted = TRUE;
    }

    return rc;
}

int32
_brg_mcfdb_iter_has_manual_mcfdb(void *obj, tbl_iter_args_t *pargs)
{
    tbl_mcfdb_t *p_db_mcfdb = (tbl_mcfdb_t*)obj;
    vid_t *pvid = (vid_t*)(pargs->argv[0]);
    uint32 *phas_mcfdb = (uint32*)(pargs->argv[1]);
    vid_t vid = 0;
    
    vid = *pvid;
    if (p_db_mcfdb->key.fid == vid)
    {
        if (p_db_mcfdb->manual)
        {
            *phas_mcfdb = TRUE;
            return PM_E_FAIL;
        }
    }
    
    return PM_E_NONE;
    
}

int32
brg_mcfdb_vlan_has_manual_mcfdb(vid_t vid)
{
    tbl_iter_args_t args;
    uint32 has_mcfdb = FALSE;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &vid;
    args.argv[1] = &has_mcfdb;
    tbl_mcfdb_iterate(_brg_mcfdb_iter_has_manual_mcfdb, &args);

    return has_mcfdb;
}

int32
brg_fdb_add_fdb(tbl_fdb_t *p_fdb)
{
    tbl_fdb_t *p_db_fdb = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check exist */
    p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
    
    if (NULL != p_db_fdb)
    {
        if (p_db_fdb->ifindex != p_fdb->ifindex)
        {
            /* 2. update to DB */
            PM_E_RETURN(tbl_fdb_set_fdb_field(p_fdb, TBL_FDB_FLD_IFINDEX));
        }
        else
        {
            /* duplicate configuration */
            return PM_E_NONE;
        }
    }
    else
    {
        /* 2. add to DB */
        PM_E_RETURN(tbl_fdb_add_fdb(p_fdb));
    }

    return rc;
}


int32
brg_fdb_del_fdb(tbl_fdb_t *p_fdb)
{
    tbl_fdb_t *p_db_fdb = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check exist */
    p_db_fdb = tbl_fdb_get_fdb(&p_fdb->key);
    if (NULL == p_db_fdb)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. del from DB */
    PM_E_RETURN(tbl_fdb_del_fdb(&p_db_fdb->key));

    return rc;
}

static int32
brg_fdb_convert_psfdb_to_fdb(tbl_psfdb_t *p_psfdb, tbl_fdb_t* p_fdb)
{
    sal_memset(p_fdb, 0, sizeof(tbl_fdb_t));
    p_fdb->key.fid = p_psfdb->key.fid;
    sal_memcpy(p_fdb->key.mac, p_psfdb->key.mac, MAC_ADDR_LEN);
    p_fdb->flags = p_psfdb->flags;
    p_fdb->ifindex = p_psfdb->ifindex;

    return PM_E_NONE;
}

int32
brg_psfdb_add_psfdb(tbl_psfdb_t *p_psfdb)
{
    tbl_fdb_t fdb;
    tbl_fdb_t *p_fdb = &fdb;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_psfdb_t *p_db_psfdb = NULL; 
    ds_brgif_t* p_brgif = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check exist */
    p_db_psfdb = tbl_psfdb_get_psfdb(&p_psfdb->key);
    if (p_db_psfdb)
    {
        return PM_E_EXIST;
    }

    /* 2. add to DB */
    PM_E_RETURN(tbl_psfdb_add_psfdb(p_psfdb));

    /* 3. add to FDB */
    if(GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(p_psfdb->ifindex);
        if(NULL == p_db_if)
        {
            return PM_E_NOT_EXIST;
        }
        p_brgif = ds_brgif_get_brgif(p_db_if);
        if(NULL == p_brgif)
        {
            return PM_E_NOT_EXIST;
        }
        if(p_brgif->port_security_en)
        {
            p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
            if (p_db_fdb) /*if the mac is exist in fdb table, delete it before*/
            {
                brg_fdb_del_fdb(p_db_fdb);
                p_db_fdb = NULL;
            }
            PM_E_RETURN(brg_fdb_convert_psfdb_to_fdb(p_psfdb, p_fdb));
            PM_E_RETURN(brg_fdb_add_fdb(p_fdb));
        }
        p_db_if->static_security_num++;
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_STATIC_SECURITY_NUM));
    }

    return rc;
}


int32
brg_psfdb_del_psfdb(tbl_psfdb_t *p_psfdb)
{
    tbl_psfdb_t *p_db_psfdb = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check psfdb exist */
    p_db_psfdb = tbl_psfdb_get_psfdb(&p_psfdb->key);
    if (NULL == p_db_psfdb)
    {
        return PM_E_NOT_EXIST;
    }
    
    /* 2. check interface exist */
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_psfdb->ifindex);
    if(NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    /* 3. del psfdb from DB */
    PM_E_RETURN(tbl_psfdb_del_psfdb(&p_db_psfdb->key));
    
    /* 4. set static security num to DB*/
    if(p_db_if->static_security_num)
    {
        p_db_if->static_security_num--;
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_STATIC_SECURITY_NUM));
    }
   
    return rc;
}

int32
brg_fdb_is_static_fdb_on_port(uint32 ifindex)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_fdb = NULL;
    uint32 i = 0;
    vid_t vid = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_l2mc_condition_port;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vid;

    /* Deleted by kcao for bug 47355 2018-06-14, not check mcast FDB same to E series */
#if 0
    /*modified by yejl to fix bug40600, 2016-09-07*/
    sal_memset(&get_node, 0, sizeof(get_node));
    tbl_mcfdb_iterate(get_func, &args);
    for (i = 0; i < get_node.index; i++)
    {
        return TRUE;
    }
#endif

    get_func = (TBL_ITER_CB_FUNC)_brg_fdb_get_condition_port;
    
    sal_memset(&get_node, 0, sizeof(get_node));
    tbl_fdb_iterate(get_func, &args);
    for (i = 0; i < get_node.index; i++)
    {
        p_fdb = get_node.node_array[i];
        if(!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


static int32
_brg_psfdb_flush_static_fdb_condtion_port_vlan(uint32 ifindex, vid_t vid)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_psfdb_t *p_psfdb = NULL;
    uint32 i = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port_vlan;
    if (0 == ifindex)
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_none;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_vlan;
        }
    }
    else
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port_vlan;
        }
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vid;
    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_psfdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_psfdb = get_node.node_array[i];
            if(GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
            {
                p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
                if (p_db_fdb)
                {
                    brg_fdb_del_fdb(p_db_fdb);
                    brg_psfdb_del_psfdb(p_psfdb);
                }
                else
                {
                    brg_psfdb_del_psfdb(p_psfdb);    
                }
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}


static int32
_brg_psfdb_flush_static_fdb_condtion_mac_port_vlan(uint8 *mac,
        uint32 ifindex, vid_t vid)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_psfdb_t *p_psfdb = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 i = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if(NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port_vlan;
    if (0 == ifindex)
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_none;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_vlan;
        }
    }
    else
    {
        if (GLB_ALL_VLAN_ID == vid)
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port;
        }
        else
        {
            get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port_vlan;
        }
    }

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;
    args.argv[2] = &vid;
    
    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_psfdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_psfdb = get_node.node_array[i];
            if ((GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))&&
                (sal_memcmp(mac, &p_psfdb->key.mac, sizeof(mac_addr_t)))) 
            {
                p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
                if (p_db_fdb)
                {
                    brg_fdb_del_fdb(p_db_fdb);
                    brg_psfdb_del_psfdb(p_psfdb);
                }
                else
                {
                    brg_psfdb_del_psfdb(p_psfdb);
                }
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}


int32
brg_psfdb_flush_static_fdb_mac(uint8 *mac)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_psfdb_t *p_psfdb = NULL;
    uint32 i = 0;

    get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_mac;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = mac;

    do 
    {    
        sal_memset(&get_node, 0, sizeof(get_node));
        tbl_psfdb_iterate(get_func, &args);
        for (i = 0; i < get_node.index; i++)
        {
            p_psfdb = get_node.node_array[i];
            if (GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
            {
                p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
                if (p_db_fdb)
                {
                    brg_fdb_del_fdb(p_db_fdb);
                    brg_psfdb_del_psfdb(p_psfdb);
                }
                else
                {
                    brg_psfdb_del_psfdb(p_psfdb);
                }
            }
        }
    } while (CDB_GET_COUNT == get_node.index);
    
    return PM_E_NONE;
}

int32
brg_psfdb_add_psfdb_to_fea(uint32 ifindex)
{
    tbl_fdb_t fdb;
    tbl_fdb_t *p_fdb = &fdb;
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_psfdb_t *p_psfdb = NULL;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 i = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if(NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;

    sal_memset(&get_node, 0, sizeof(get_node));
    tbl_psfdb_iterate(get_func, &args);
    /*modified by yejl to fix bug40600, 2016-09-07*/
    for (i = 0; i < get_node.index; i++)
    {
        p_psfdb = get_node.node_array[i];
        if (GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
            if (p_db_fdb)
            {
                brg_fdb_del_fdb(p_db_fdb);
                p_db_fdb = NULL;
            }
            brg_fdb_convert_psfdb_to_fdb(p_psfdb, p_fdb);
            brg_fdb_add_fdb(p_fdb);
        }
    }
    
    return PM_E_NONE;
}


int32
brg_psfdb_del_psfdb_form_fea(uint32 ifindex)
{
    TBL_ITER_CB_FUNC get_func;
    cdb_get_node_t  get_node;
    tbl_iter_args_t args;
    tbl_psfdb_t *p_psfdb = NULL;
    tbl_fdb_t *p_db_fdb = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 i = 0;

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if(NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    
    get_func = (TBL_ITER_CB_FUNC)_brg_psfdb_get_condition_port;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &get_node;
    args.argv[1] = &ifindex;

    sal_memset(&get_node, 0, sizeof(get_node));
    tbl_psfdb_iterate(get_func, &args);
    /*modified by yejl to fix bug40600, 2016-09-07*/
    for (i = 0; i < get_node.index; i++)
    {
        p_psfdb = get_node.node_array[i];
        if (GLB_FLAG_ISSET(p_psfdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            p_db_fdb = tbl_fdb_get_fdb(&p_psfdb->key);
            if (p_db_fdb)
            {
                brg_fdb_del_fdb(p_db_fdb);
                p_db_fdb = NULL;
            }
        }
    }
    
    return PM_E_NONE;
}


int32
brg_psfdb_flush_static_fdb_mac_vlan(uint8 *mac, uint16 vid)
{
    return _brg_psfdb_flush_static_fdb_condtion_mac_port_vlan(mac,GLB_IFINDEX_ALL, vid);
}

int32
brg_psfdb_flush_static_fdb_mac_port(uint8 *mac, uint32 ifindex)
{
    return _brg_psfdb_flush_static_fdb_condtion_mac_port_vlan(mac,ifindex, GLB_ALL_VLAN_ID);
}

int32
brg_psfdb_flush_static_fdb_mac_vlan_port(uint8 *mac, uint32 ifindex, uint16 vid)
{
    return _brg_psfdb_flush_static_fdb_condtion_mac_port_vlan(mac,ifindex, vid);
}

int32
brg_psfdb_flush_static_fdb_vlan(vid_t vid)
{
    return _brg_psfdb_flush_static_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, vid);
}


int32
brg_psfdb_flush_static_fdb_port(uint32 ifindex)
{
    return _brg_psfdb_flush_static_fdb_condtion_port_vlan(ifindex, GLB_ALL_VLAN_ID);
}

int32
brg_psfdb_flush_static_fdb_port_vlan(uint32 ifindex, vid_t vid)
{
    return _brg_psfdb_flush_static_fdb_condtion_port_vlan(ifindex, vid);
}

int32
brg_psfdb_flush_static_fdb()
{
    return _brg_psfdb_flush_static_fdb_condtion_port_vlan(GLB_IFINDEX_ALL, GLB_ALL_VLAN_ID);
}

int32
brg_macfilter_add_macfilter(tbl_macfilter_t *p_macflt)
{
    tbl_macfilter_t *p_db_macflt = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check exist */
    p_db_macflt = tbl_macfilter_get_macfilter(&p_macflt->key);
    if (NULL != p_db_macflt)
    {
        return PM_E_NONE;
    }

    /* 2. add to DB */
    PM_E_RETURN(tbl_macfilter_add_macfilter(p_macflt));

    return rc;
}

int32
brg_macfilter_del_macfilter(tbl_macfilter_t *p_macflt)
{
    tbl_macfilter_t *p_db_macflt = NULL;
    int32 rc = PM_E_NONE;

    /* 1. check exist */
    p_db_macflt = tbl_macfilter_get_macfilter(&p_macflt->key);
    if (NULL == p_db_macflt)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. del from DB */
    PM_E_RETURN(tbl_macfilter_del_macfilter(&p_macflt->key));

    return rc;
}

int32
brg_macfilter_flush_macfilter()
{
    tbl_macfilter_master_t* p_master = tbl_macfilter_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;
    tbl_macfilter_t *p_db_macflt = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->macflt_list, p_db_macflt, listnode, nextnode)
    {
        brg_macfilter_del_macfilter(p_db_macflt);
    }

    return PM_E_NONE;
}


int32
brg_psfdb_pdu_decode_mac(uint8 *p_buf, uint32 len, mac_addr_t smac, mac_addr_t dmac)
{
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    
    size = &len_left;
    ppnt = &p_buf;

    MSG_DECODE_GETC(dmac[0]);
    MSG_DECODE_GETC(dmac[1]);
    MSG_DECODE_GETC(dmac[2]);
    MSG_DECODE_GETC(dmac[3]);
    MSG_DECODE_GETC(dmac[4]);
    MSG_DECODE_GETC(dmac[5]);

    MSG_DECODE_GETC(smac[0]);
    MSG_DECODE_GETC(smac[1]);
    MSG_DECODE_GETC(smac[2]);
    MSG_DECODE_GETC(smac[3]);
    MSG_DECODE_GETC(smac[4]);
    MSG_DECODE_GETC(smac[5]);
    
    return PM_E_NONE;
}


int32
brg_psfdb_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_brgif = NULL;
    char ifname_ext[IFNAME_EXT_SIZE]; 
    uint32 ifindex = 0;
    mac_addr_t src_mac;
    mac_addr_t dest_mac;

    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    /*fix bug 40604, 2016-09-06*/
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    if (!p_db_if || !p_db_if->brgif)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Port-security feature is not supported on this interface %s\n", ifname_ext);        
        return PM_E_FAIL;
    }

    if (GLB_PORT_SECURITY_VIO_TYPE_VLAN_MACLIMIT_DISCARD == p_msg->u_hdr.pkt.sub_type)
    {
        brg_psfdb_pdu_decode_mac(p_msg->data, p_msg->data_len, src_mac, dest_mac);
        brg_vlan_security_log(p_msg->u_hdr.pkt.vid, src_mac, dest_mac, p_db_if->desc);
        return PM_E_NONE;
    }

    p_brgif = p_db_if->brgif;

    if ((GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_brgif->violate_mode)
        || (GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_brgif->violate_mode))
    {
        if (GLB_PORT_SECURITY_VIOLATION_MODE_RESTRICT == p_brgif->violate_mode)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Port Security Violation  : restrict mode\n");
        }
        else
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Port Security Violation  : shutdown mode\n");
        }

        if (GLB_PORT_SECURITY_VIO_TYPE_MACSA_MISMATCH == p_msg->u_hdr.pkt.sub_type)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Port Security Violation Reason: mac sa mismatch\n");
        }
        else if (GLB_PORT_SECURITY_VIO_TYPE_PORT_MACLIMIT_DISCARD == p_msg->u_hdr.pkt.sub_type)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "Port Security Violation Reason: mac limit discard\n");
        }

        brg_psfdb_pdu_decode_mac(p_msg->data, p_msg->data_len, src_mac, dest_mac);
        
        
        log_sys(M_MOD_INTERNAL, E_ERROR, "Packet from interface    : %s\n", ifname_ext);

        log_sys(M_MOD_INTERNAL, E_ERROR, "Packet source MAC address: %02x.%02x.%02x.%02x.%02x.%02x\n",
            src_mac[0], src_mac[1], src_mac[2],
            src_mac[3], src_mac[4], src_mac[5]);
        log_sys(M_MOD_INTERNAL, E_ERROR, "Packet dest MAC address  : %02x.%02x.%02x.%02x.%02x.%02x\n",
            dest_mac[0], dest_mac[1], dest_mac[2],
            dest_mac[3], dest_mac[4], dest_mac[5]);

        log_sys(M_MOD_INTERNAL, E_ERROR, "Packet from VLAN         : %d\n", p_msg->u_hdr.pkt.vid);
    }

#ifdef CONFIG_ERRDISABLE
    /* set error disable */
    if (GLB_PORT_SECURITY_VIOLATION_MODE_SHUTDOWN == p_brgif->violate_mode)
    {
        errdisable_handle_exception(ERRDIS_PORT_SECURITY_VIOLATION, p_db_if, TRUE);
    }
#endif /* CONFIG_ERRDISABLE */

    return PM_E_NONE;
}


int32
brg_fdb_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_FDB, brg_fdb_cmd_process_fdb);
    cdb_register_cfg_tbl_cb(TBL_MCFDB, brg_fdb_cmd_process_fdb);
    cdb_register_cfg_tbl_cb(TBL_MACFILTER, brg_fdb_cmd_process_macfilter);
    cdb_register_cfg_tbl_cb(TBL_PSFDB, brg_psfdb_cmd_process_psfdb);

    cdb_register_cfg_tbl_cb(TBL_L2_ACTION, brg_fdb_cmd_process_l2_action);

    cdb_register_cfg_act_cb(ACT_FLUSH_FDB, brg_fdb_cmd_process_flushfdb);
    cdb_register_cfg_act_cb(ACT_FLUSH_PSFDB, brg_psfdb_cmd_process_flushfdb);
    cdb_register_cfg_act_cb(ACT_FLUSH_MACFILTER, brg_fdb_cmd_process_flush_macfilter);

    ipc_register_pkt_rx_fn(GLB_PKT_MAC_SEC, brg_psfdb_pdu_rx);

    return PM_E_NONE;
}

int32
brg_fdb_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_FDB, NULL);
    cdb_register_cfg_act_cb(ACT_FLUSH_FDB, NULL);
    cdb_register_cfg_act_cb(ACT_FLUSH_MACFILTER, NULL);
    return PM_E_NONE;
}

