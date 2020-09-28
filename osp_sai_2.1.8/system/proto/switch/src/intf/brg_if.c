
#include "proto.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "switch.h"
#include "switch_api.h"
#include "if_mgr.h"
#include "brg_if.h"
#include "brg_fdb.h"
#include "vlan_mapping.h"

#ifdef CONFIG_L2

int32
brg_if_set_vlan_port_bmp(uint32 ifindex, vid_t vid, uint32 is_add)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    int32 rc = PM_E_NONE;
    uint64 action =0;

    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (!p_db_vlan)
    {
        return PM_E_NOT_EXIST;
    }

    if (is_add)
    {
        GLB_BMP_SET(p_db_vlan->member_port, ifindex);
        action = GLB_VLAN_OP_ACTION_ADD;
        p_db_vlan->op_ifindex = (action << 32) + ifindex;
    }
    else
    {
        GLB_BMP_UNSET(p_db_vlan->member_port, ifindex);
        action = GLB_VLAN_OP_ACTION_REMOVE;
        p_db_vlan->op_ifindex = (action << 32) + ifindex;
    }
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_MEMBER_PORT);
    tbl_vlan_set_vlan_field(p_db_vlan, TBL_VLAN_FLD_OP_IFINDEX);
    

    PM_E_RETURN(if_mgr_vlan_if_running_update(p_db_vlan));
    return PM_E_NONE;
}

int32
_brg_if_clear_vlan_process(tbl_interface_t *p_db_if, vid_t vid)
{
    /* 1. remove multicast FDB */
    brg_fdb_flush_l2mc_fdb_port_vlan(p_db_if->ifindex, vid);
    /* 2. remove static FDB */
    brg_fdb_flush_static_fdb_port_vlan(p_db_if->ifindex, vid);
    /* 3. flush dynamic FDB */
    brg_fdb_flush_dynamic_fdb_port_vlan(p_db_if->ifindex, vid);
    /* 4. flush port security static FDB*/
    brg_psfdb_flush_static_fdb_port_vlan(p_db_if->ifindex, vid);

    return PM_E_NONE;
}

static int32
_brg_if_add_vlan_for_allowed_all_port_iter(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    vid_t vid = *(vid_t*)pargs->argv[0];
    ds_brgif_t *p_db_brgif = p_db_if->brgif;
    ds_brgif_t brgif;
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_brgif)
    {
        return PM_E_NONE;
    }

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return PM_E_NONE;
    }
    
    if (DS_BRGIF_ALLOWED_VLAN_ACT_ALL != p_db_brgif->allowed_vlan.action)
    {
        return PM_E_NONE;
    }

    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.allowed_vlan.action = p_db_brgif->allowed_vlan.action;
    sal_memcpy(brgif.allowed_vlan.vlan_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t));
    PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, TRUE, TRUE, TRUE));
    GLB_BMP_SET(brgif.allowed_vlan.vlan_bmp, vid);
    PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, TRUE));

    return PM_E_NONE;
}

int32
brg_if_add_vlan_for_allowed_all_port(vid_t vid)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &vid;
    tbl_interface_iterate((TBL_ITER_CB_FUNC)_brg_if_add_vlan_for_allowed_all_port_iter, &args);

    return PM_E_NONE;
}

static int32
_brg_if_access_set_pvid(tbl_interface_t *p_db_if, vid_t pvid)
{
    ds_brgif_t brgif;
    ds_brgif_t *p_db_brgif = NULL;
    int32 rc = PM_E_NONE;

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }

    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.pvid = pvid;
    sal_memcpy(brgif.allowed_vlan.vlan_bmp, p_db_brgif->allowed_vlan.vlan_bmp, sizeof(vlan_bmp_t));

    /* clear old config */
    if (GLB_VLAN_NONE != p_db_brgif->pvid)
    {
        /* notify vlan's port bitmap */
        PM_E_RETURN(brg_if_set_vlan_port_bmp(p_db_if->ifindex, p_db_brgif->pvid, FALSE));
        PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, p_db_brgif->pvid));
        swth_if_set_allowed_vlan_change_cb(p_db_if, p_db_brgif->pvid, FALSE);
        GLB_BMP_UNSET(brgif.allowed_vlan.vlan_bmp, p_db_brgif->pvid);
    }

    /* set new config */
    if (GLB_VLAN_NONE != brgif.pvid)
    {
        /* notify vlan's port bitmap */
        PM_E_RETURN(brg_if_set_vlan_port_bmp(p_db_if->ifindex, brgif.pvid, TRUE));
        swth_if_set_allowed_vlan_change_cb(p_db_if, brgif.pvid, TRUE);
    }
	if (GLB_VLAN_NONE != brgif.pvid)
    {
	    brgif.allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_ADD;
	    GLB_BMP_SET(brgif.allowed_vlan.vlan_bmp, pvid);
	    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_ALLOWED_VLAN));
	    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_PVID));
	}
    return PM_E_NONE;
}

static int32
_brg_if_trunk_set_pvid(tbl_interface_t *p_db_if, vid_t pvid)
{
    ds_brgif_t brgif;
    ds_brgif_t *p_db_brgif = NULL;
    int32 rc = PM_E_NONE;

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.pvid = pvid;

    if (GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, pvid))
    {
        GLB_BMP_UNSET(p_db_brgif->untagged_vlan.vlan_bmp, pvid);
        PM_E_RETURN(brg_if_set_untagged_vlan_db(p_db_if, p_db_brgif, TRUE));
    }

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_PVID));
    return PM_E_NONE;
}

static int32
_brg_if_dot1q_set_pvid(tbl_interface_t *p_db_if, vid_t pvid)
{
    ds_brgif_t brgif;
    ds_brgif_t *p_db_brgif = NULL;
    int32 rc = PM_E_NONE;

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));

    if (GLB_VLAN_NONE != p_db_brgif->inner_native_vid)
    {
        brgif.inner_native_vid = GLB_VLAN_NONE;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_INNER_NATIVE_VID));
    }

    brgif.pvid = pvid;
    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_PVID));

    return PM_E_NONE;
}

static int32
_brg_if_clear_if_vm_table(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    tbl_vlan_mapping_table_key_t vm_table_key;
    tbl_vlan_mapping_table_t    *p_db_vm_table = NULL;
    tbl_vlan_group_key_t vlan_group_key;
    tbl_vlan_group_t    *p_db_vlan_group = NULL;
    tbl_brg_global_t*    p_db_brg_global = tbl_brg_global_get_brg_global();

    if (sal_strlen(p_db_if->brgif->vlan_mapping_table))
    {
        sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
        sal_strncpy(vm_table_key.name, p_db_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vm_table = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
        if (!p_db_vm_table)
        {
            return rc;
        }
        
        vlanmap_api_if_delete_vmt(p_db_if, NULL);
        if (p_db_vm_table->group_id)
        {
            sal_memset(&vlan_group_key, 0, sizeof(tbl_vlan_group_key_t));
            vlan_group_key.group_id = p_db_vm_table->group_id;
            p_db_vlan_group = tbl_vlan_group_get_vlan_group(&vlan_group_key);
            if (!p_db_vlan_group)
            {
                return rc;
            }

            uint32 group_valid_count = 0;
            group_valid_count = vlanmap_api_get_valid_group(p_db_vlan_group);
            p_db_brg_global->vlan_mapping_entry_cnt -= group_valid_count;
            PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
        }
    }

    return PM_E_NONE;
}

int32
brg_if_set_pvid(tbl_interface_t *p_if, vid_t pvid, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    vid_t old_pvid = 0;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    old_pvid = p_db_if->brgif->pvid;
    
    /* 1. process */
    switch (p_db_if->vlan_type)
    {
    case GLB_VLAN_PORT_TYPE_ACCESS:
        PM_E_RETURN(_brg_if_access_set_pvid(p_db_if, pvid));
        break;

    case GLB_VLAN_PORT_TYPE_TRUNK:
        PM_E_RETURN(_brg_if_trunk_set_pvid(p_db_if, pvid));
        break;

    case GLB_VLAN_PORT_TYPE_QINQ:
        PM_E_RETURN(_brg_if_dot1q_set_pvid(p_db_if, pvid));
        break;

    default:
        SAL_ASSERT(0);
    }

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_pvid(p_if_mem, pvid, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
        /* yejl for bug 35481, flush agg FDB should be after memports are deleted */
        if (GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)
        {
            PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, old_pvid));
        }
    }

    return rc;
}

int32
brg_if_set_inner_native_vlan(tbl_interface_t *p_if, vid_t inner_native_vid, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.inner_native_vid = inner_native_vid;

    if (inner_native_vid && GLB_VLAN_NONE != p_db_brgif->pvid)
    {
        brgif.pvid = GLB_VLAN_NONE;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_PVID));
    }

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_INNER_NATIVE_VID));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_inner_native_vlan(p_if_mem, inner_native_vid, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_qinq_type(tbl_interface_t *p_if, uint8 qinq_type, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }

    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.qinq_type = qinq_type;

    if (GLB_VLAN_QINQ_TYPE_SELECTIVE != qinq_type)
    {
        _brg_if_clear_if_vm_table(p_db_if);
        if (p_db_brgif->qinq_default_fwd)
        {
            brgif.qinq_default_fwd = FALSE;
            PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_QINQ_DEFAULT_FWD));
        }
    }

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_QINQ_TYPE));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_qinq_type(p_if_mem, qinq_type, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_vlan_translation_en(tbl_interface_t *p_if, uint8 vlan_translation_en, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    vid_t vid = 0;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }

    /* modified by liwh for bug 51544, 2019-04-09 */
    if (p_db_brgif->vlan_translation_en == vlan_translation_en)
    {
        return PM_E_NONE;    
    }
    /* liwh end */
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.vlan_translation_en = vlan_translation_en;

    if (!vlan_translation_en)
    {
        _brg_if_clear_if_vm_table(p_db_if);
        if (p_db_brgif->vlan_trans_default_fwd)
        {
            brgif.vlan_trans_default_fwd = FALSE;
            PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD));
        }
    }

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_VLAN_TRANSLATION_EN));

    /* flush fdb when set vlan translation enable  */
    GLB_BMP_ITER_BEGIN(p_db_if->brgif->allowed_vlan.vlan_bmp, vid)
    {
        PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, vid));
    }
    GLB_BMP_ITER_END(p_db_if->brgif->allowed_vlan.vlan_bmp, vid);

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_vlan_translation_en(p_if_mem, vlan_translation_en, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_apply_vlanmap_table(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, 
        uint32 is_add, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    ds_brgif_t *p_db_brgif = NULL;
    tbl_vlan_mapping_table_t      *p_db_vmt = NULL;
    tbl_vlan_mapping_table_key_t   vmt_key;
    tbl_brg_global_t*  p_db_brg_global = tbl_brg_global_get_brg_global();

    vid_t vid = 0;

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }

    if (is_add)
    {
        if (!sal_strcmp(p_db_if->brgif->vlan_mapping_table, p_brgif->vlan_mapping_table))
        {
            return PM_E_FAIL;
        }
        else if (sal_strlen(p_db_if->brgif->vlan_mapping_table))
        {
            CFG_CONFLICT_RET("VLAN mapping table exist on this port");
        }

        sal_memset(&vmt_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
        sal_strncpy(vmt_key.name, p_brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);

        p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
        if (!p_db_vmt)
        {
            CFG_CONFLICT_RET("VLAN mapping table %s does not exist", p_brgif->vlan_mapping_table);
        }

        if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            PM_E_RETURN(vlanmap_api_vmt_check_vlantrans(p_db_vmt, para));
        }

        if (p_db_brg_global->vlan_mapping_entry_cnt + p_db_vmt->entry_num + p_db_vmt->egress_entry_num > GLB_VLAN_MAPPING_ENTRY_MAX)
        {
            CFG_CONFLICT_RET("VLAN mapping entry exceed max number %d", GLB_VLAN_MAPPING_ENTRY_MAX);
        }

        GLB_BMP_SET(p_db_vmt->applied_ports, p_db_if->ifindex);
        PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS));

        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));

        p_db_brg_global->vlan_mapping_entry_cnt += (p_db_vmt->entry_num + p_db_vmt->egress_entry_num);
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
    }
    else
    {
        if (!sal_strlen(p_db_if->brgif->vlan_mapping_table))
        {
            return PM_E_NONE;
        }
        else
        {
            sal_memset(&vmt_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
            sal_strncpy(vmt_key.name, p_db_brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
            if (!p_db_vmt)
            {
                CFG_CONFLICT_RET("VLAN mapping table %s does not exist", p_db_brgif->vlan_mapping_table);
            }
            
            GLB_BMP_UNSET(p_db_vmt->applied_ports, p_db_if->ifindex);
            PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS));

            PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));

            p_db_brg_global->vlan_mapping_entry_cnt -= (p_db_vmt->entry_num + p_db_vmt->egress_entry_num);
            PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
        }
        /* for bug48422 remove fdb when unapply vmt */
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, vid));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
    }

    return rc;
}


int32
brg_if_set_qinq_default_fwd(tbl_interface_t *p_if, uint8 qinq_default_fwd, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.qinq_default_fwd = qinq_default_fwd;

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_QINQ_DEFAULT_FWD));

    PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, p_db_if->brgif->pvid));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_qinq_default_fwd(p_if_mem, qinq_default_fwd, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_vlan_trans_default_fwd(tbl_interface_t *p_if, uint8 vlan_trans_default_fwd, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.vlan_trans_default_fwd = vlan_trans_default_fwd;

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD));

    PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, p_db_if->brgif->pvid));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_vlan_trans_default_fwd(p_if_mem, vlan_trans_default_fwd, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_dot1q_tag_native_en(tbl_interface_t *p_if, uint8 dot1q_tag_native_en, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t   *p_lag = NULL;
    ds_brgif_t  brgif;
    ds_brgif_t *p_db_brgif = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    p_db_brgif = p_db_if->brgif;
    if (NULL == p_db_brgif)
    {
        return PM_E_FAIL;
    }
    
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.dot1q_tag_native_en = dot1q_tag_native_en;

    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_dot1q_tag_native_en(p_if_mem, dot1q_tag_native_en, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_tpid_type(tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, p_brgif, DS_BRGIF_FLD_TPID_TYPE));
    return rc;
}

int32
brg_if_set_allowed_vlan_db(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_brgif, DS_BRGIF_FLD_ALLOWED_VLAN));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_allowed_vlan_db(p_if_mem, p_brgif, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_untagged_vlan_db(tbl_interface_t *p_db_if, ds_brgif_t *p_brgif, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_brgif, DS_BRGIF_FLD_UNTAGGED_VLAN));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_untagged_vlan_db(p_if_mem, p_brgif, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_ingress_filter(tbl_interface_t *p_if, ds_brgif_t *p_brgif, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, p_brgif, DS_BRGIF_FLD_INGRESS_FILTER));

    if ((INTERFACE_IS_LAG_GROUP(p_if)) && (iter_member == TRUE))
    {
        p_lag = p_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_ingress_filter(p_if_mem, p_brgif, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
brg_if_update_oper_mac_learning_en(tbl_interface_t *p_db_if)
{
    ds_brgif_t *p_db_brgif = NULL;
    
    if (NULL == p_db_if->brgif)
    {
        return PM_E_INVALID_PARAM;
    }
    p_db_brgif = p_db_if->brgif;

    /* MLAG peer link should disable MAC learning */
    if (p_db_brgif->mac_learning_en && !p_db_brgif->mlag_is_peer_link)
    {
        p_db_brgif->oper_mac_learning_en = TRUE;
    }
    else
    {
        p_db_brgif->oper_mac_learning_en = FALSE;
    }
    ds_brgif_set_brgif_field(p_db_if, p_db_brgif, DS_BRGIF_FLD_OPER_MAC_LEARNING_EN);

    return PM_E_NONE;
}

int32
brg_if_set_mac_learning_en(tbl_interface_t *p_if, uint32 enable, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ds_brgif_t brgif;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* update local CDB */
    brgif.mac_learning_en = enable;
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, &brgif, DS_BRGIF_FLD_MAC_LEARNING_EN));

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    rc = brg_if_update_oper_mac_learning_en(p_db_if);

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_mac_learning_en(p_if_mem, enable, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
brg_if_set_port_security_en(tbl_interface_t *p_if, uint32 enable, uint32 iter_member)
{
    ds_brgif_t *brgif = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    brgif = p_db_if->brgif;
    if (NULL == brgif)
    {
        return PM_E_FAIL;
    }
    /* 2. update local CDB */
    brgif->port_security_en = enable;
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, brgif, DS_BRGIF_FLD_PORT_SECURITY_EN));

    
    if(enable)
    {
        brg_psfdb_add_psfdb_to_fea(p_if->ifindex);
    }
    else
    {
        brg_psfdb_del_psfdb_form_fea(p_if->ifindex);   
    }

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_port_security_en(p_if_mem, enable, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
brg_if_set_port_security_mac_maximum(tbl_interface_t *p_if, uint32 num, uint32 iter_member)
{
    ds_brgif_t *brgif = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    brgif = p_db_if->brgif;
    if (NULL == brgif)
    {
        return PM_E_FAIL;
    }
    
    /*2. update local CDB */
    brgif->max_mac = num;
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, brgif, DS_BRGIF_FLD_MAX_MAC));
    
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_port_security_mac_maximum(p_if_mem, num, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
brg_if_set_port_security_voilation(tbl_interface_t *p_if, int32 mode, uint32 iter_member)
{
    ds_brgif_t *brgif = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    brgif = p_db_if->brgif;
    if (NULL == brgif)
    {
        return PM_E_FAIL;
    }
    
    /*2. update local CDB */
    brgif->violate_mode = mode;
    PM_E_RETURN(ds_brgif_set_brgif_field(p_if, brgif, DS_BRGIF_FLD_VIOLATE_MODE));
    
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_port_security_voilation(p_if_mem, mode, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    return rc;
}

int32
brg_if_clear_port_security(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    if(NULL == p_db_if->brgif)
    {
        return PM_E_NONE;
    }

    if (p_db_if->brgif->port_security_en)
    {
        brg_if_set_port_security_en(p_db_if, FALSE, TRUE);
    }
    if (GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM != p_db_if->brgif->max_mac)
    {
        /* Added by kcao for bug 31192, only when security_en, set to HAL */
        PM_E_RETURN(brg_if_set_port_security_mac_maximum(p_db_if, GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM, TRUE));
    }
    if (GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT != p_db_if->brgif->violate_mode)
    {
        PM_E_RETURN(brg_if_set_port_security_voilation(p_db_if, GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT, TRUE));
    }
    return rc;
}

int32
brg_if_set_allowed_vlan_process(tbl_interface_t *p_db_if, vid_t vid, uint32 is_add, uint32 fdb_flush, uint32 iter_member)
{
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    if (is_add)
    {
        PM_E_RETURN(brg_if_set_vlan_port_bmp(p_db_if->ifindex, vid, TRUE));
    }
    else
    {
        /* modified by liwh for bug 53010, 2019-07-25 */
        if_mgr_clear_cfm_meps_by_interface_vlan(p_db_if, vid);
        /* liwh end */

        PM_E_RETURN(brg_if_set_vlan_port_bmp(p_db_if->ifindex, vid, FALSE));
        /* kcao for bug 29726, flush FDB should be after vlan is remove from port */
        if(fdb_flush)
        {
            PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, vid));
        }
    }
    swth_if_set_allowed_vlan_change_cb(p_db_if, vid, is_add);

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_set_allowed_vlan_process(p_if_mem, vid, is_add, fdb_flush, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
        if (!is_add)
        {
            /* yejl for bug 35481, flush agg FDB should be after memports are deleted */
            PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, vid));
        }
    }

    return rc;
}

int32
brg_if_trunk_clear_vlan_translation(tbl_interface_t *p_db_if, uint32 enable, uint32 iter_member)
{
    int32 rc = PM_E_NONE;
    int32 memrc = PM_E_NONE;
    ds_lag_t  *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;

    if (enable == p_db_if->brgif->vlan_translation_en)
    {
        return rc;
    }

    if (!p_db_if->brgif->vlan_trans_default_fwd)
    {
        p_db_if->brgif->vlan_translation_en = TRUE;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD));
        p_db_if->brgif->vlan_translation_en = FALSE;
    }

    if(GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        p_db_if->brgif->vlan_translation_en = enable;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_VLAN_TRANSLATION_EN));
    }

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_trunk_clear_vlan_translation(p_if_mem, enable, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_dot1q_clear_vlan_mapping(tbl_interface_t *p_db_if, uint32 iter_member)
{
    int32 rc = PM_E_NONE;
    int32 memrc = PM_E_NONE;
    ds_lag_t  *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;

    PM_E_RETURN(_brg_if_clear_if_vm_table(p_db_if));

    if (sal_strlen(p_db_if->brgif->vlan_mapping_table))
    {
        sal_memset(p_db_if->brgif->vlan_mapping_table, 0, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));
    }

    if (p_db_if->brgif->qinq_default_fwd)
    {
        p_db_if->brgif->qinq_default_fwd = FALSE;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_QINQ_DEFAULT_FWD));
    }

    if(GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type && GLB_VLAN_QINQ_TYPE_SELECTIVE == p_db_if->brgif->qinq_type)
    {
        p_db_if->brgif->qinq_type = GLB_VLAN_QINQ_TYPE_BASIC;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_QINQ_TYPE));
    }

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = brg_if_dot1q_clear_vlan_mapping(p_if_mem, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
brg_if_set_vlan_type_clear(tbl_interface_t *p_db_if)
{
    ds_brgif_t brgif;
    ds_brgif_t *p_db_brgif = NULL;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    p_db_brgif = (ds_brgif_t*)p_db_if->brgif;

    switch (p_db_if->vlan_type)
    {
    case GLB_VLAN_PORT_TYPE_ACCESS:
        /* clear pvid */
        PM_E_RETURN(_brg_if_access_set_pvid(p_db_if, GLB_VLAN_NONE));
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, FALSE, TRUE, FALSE));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
        break;

    case GLB_VLAN_PORT_TYPE_TRUNK:
        PM_E_RETURN(brg_if_trunk_clear_vlan_translation(p_db_if, FALSE, FALSE));
        PM_E_RETURN(_brg_if_clear_if_vm_table(p_db_if));
        PM_E_RETURN(_brg_if_trunk_set_pvid(p_db_if, GLB_VLAN_NONE));
        brgif.ingress_filter = TRUE;
        PM_E_RETURN(brg_if_set_ingress_filter(p_db_if, &brgif, FALSE));
        if (p_db_if->brgif->dot1q_tag_native_en)
        {
            PM_E_RETURN(brg_if_set_dot1q_tag_native_en(p_db_if, FALSE, TRUE));
        }
        brgif.allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_NONE;
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, FALSE, TRUE, FALSE));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
        GLB_BMP_INIT(p_db_brgif->allowed_vlan.vlan_bmp);
        GLB_BMP_INIT(brgif.untagged_vlan.vlan_bmp);
        PM_E_RETURN(brg_if_set_untagged_vlan_db(p_db_if, &brgif, FALSE));
        break;

    case GLB_VLAN_PORT_TYPE_QINQ:
        PM_E_RETURN(brg_if_dot1q_clear_vlan_mapping(p_db_if, FALSE));
        PM_E_RETURN(_brg_if_dot1q_set_pvid(p_db_if, GLB_VLAN_NONE));
        /* set trans disable, fea sub to set vlan mapping to type none, no need to set basic qinq */
        PM_E_RETURN(brg_if_set_vlan_translation_en(p_db_if, FALSE, FALSE));
        brgif.ingress_filter = TRUE;
        PM_E_RETURN(brg_if_set_ingress_filter(p_db_if, &brgif, FALSE));
        brgif.allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_NONE;
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, vid, FALSE, TRUE, FALSE));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
        GLB_BMP_INIT(p_db_brgif->allowed_vlan.vlan_bmp);
        GLB_BMP_INIT(brgif.allowed_vlan.vlan_bmp);
        PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, FALSE));
        break;

    case GLB_VLAN_PORT_TYPE_MAX:
        return PM_E_NONE;

    default:
        return PM_E_INVALID_PARAM;
    }
    
    p_db_if->vlan_type = GLB_VLAN_PORT_TYPE_MAX;
    
    return PM_E_NONE;
}

#endif /* CONFIG_L2 */
