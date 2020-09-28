#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
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
#include "gen/tbl_mapped_vlan_info_define.h"
#include "gen/tbl_mapped_vlan_info.h"
#include "switch.h"
#include "vlan_mapping.h"
#include "glb_macro.h"
#include "if_mgr.h"
#include "if_agg.h"
#include "brg_if.h"


#define FDB_CLEAR_MODE_NORMAL   0
#define FDB_CLEAR_MODE_UNTAG    1
#define FDB_CLEAR_MODE_GROUP    2


int32
vlanmap_api_process_pdu(tbl_interface_t *p_db_if, vid_t *vid)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_table_t    *p_db_vmt = NULL;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_entry_t    *p_db_vme = NULL;
    tbl_vlan_mapping_entry_key_t vme_key;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    if (NULL == p_db_if)
    {
        return rc;
    }
    /* check vlan mapping configuration */
    if (NULL == p_db_if->brgif)
    {
        return rc;
    }
    if (0 == sal_strlen(p_db_if->brgif->vlan_mapping_table))
    {
        return rc;
    }

    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_strcpy(vmt_key.name, p_db_if->brgif->vlan_mapping_table);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    if (!p_db_vmt)
    {
        return rc;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vme, listnode, next)
    {
        if (!sal_strncmp(p_db_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            if (p_db_vme->key.raw_vid == (uint32)*vid)
            {
                *vid = p_db_vme->mapped_svid;
                return rc;
            }
        }
    }

    sal_memset(&vme_key, 0, sizeof(vme_key));
    sal_strcpy(vme_key.table_name, p_db_if->brgif->vlan_mapping_table);
    vme_key.raw_vid = GLB_VLAN_ALL;
    p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
    if (p_db_vme)
    {
        *vid = p_db_vme->mapped_svid;
    }

    return rc;
}

int32
vlanmap_api_iterate_applied_ports(tbl_vlan_mapping_table_t *p_db_vmt, 
       VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC fn, tbl_iter_args_t *pargs)
{
    int32  rc = PM_E_NONE;
    uint32 applied_ports_bmp_zero    = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;

    GLB_BMP_CHECK_ALL_ZERO(p_db_vmt->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vmt->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vmt->applied_ports, ifindex))
            {
                PM_E_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key);
                if (NULL == p_db_if)
                {
                    return PM_E_NOT_EXIST;
                }
                rc = fn(p_db_if, pargs);
                if (rc)
                {
                    return rc;
                }
            }
        }
        GLB_BMP_ITER_END(p_db_vmt->applied_ports, ifindex);
    }

    return rc;
}

int32
vlanmap_api_check_mapped_vlan_exist(char *vm_table, uint16 vid)
{
    tbl_vlan_mapping_table_t   *p_db_vmt = NULL;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_entry_t   *p_db_vme = NULL;
    tbl_vlan_mapping_entry_key_t vme_key;
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    tbl_mapped_vlan_info_t *p_db_mapped_info = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_strcpy(vmt_key.name, vm_table);

    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    if (!p_db_vmt)
    {
        return FALSE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode, next)
    {
        if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            continue;
        }

        if (p_db_mapped_info->mapped_svid == vid)
        {
            return TRUE;
        }
    }

    if (p_db_vmt->group_id && p_db_vmt->group_mapped_svid == vid)
    {
        return TRUE;
    }

    sal_memset(&vme_key, 0, sizeof(vme_key));
    sal_strcpy(vme_key.table_name, vm_table);
    vme_key.raw_vid = GLB_VLAN_NONE;
    p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
    if (p_db_vme)
    {
        if (p_db_vme->mapped_svid == vid)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
vlanmap_api_clear_mapped_vlan_fdb(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    uint16 *mapped_vid = (uint16 *)pargs->argv[0];
    uint32 *clear_mode = (uint32 *)pargs->argv[1];
    tbl_vlan_mapping_table_t *p_db_vmt = (tbl_vlan_mapping_table_t *)pargs->argv[2];
    tbl_mapped_vlan_info_t   *p_db_mapped_info = NULL;
    tbl_vlan_mapping_entry_t *p_db_vme = NULL;
    tbl_vlan_mapping_entry_key_t vme_key;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    uint32 is_mapped_vlan = FALSE;
    uint32 is_mapped_double = FALSE;
    uint32 is_mapped_group = FALSE;

    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode, next)
    {
        if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            continue;
        }

        if (p_db_mapped_info->mapped_svid == *mapped_vid)
        {
            is_mapped_vlan = TRUE;
            break;
        }
    }

    if (p_db_vmt->group_mapped_svid == *mapped_vid)
    {
        is_mapped_group = TRUE;
    }

    sal_memset(&vme_key, 0, sizeof(vme_key));
    sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
    vme_key.raw_vid = GLB_VLAN_NONE;
    p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
    if (p_db_vme)
    {
        if (p_db_vme->mapped_cvid && p_db_vme->mapped_svid == *mapped_vid)
        {
            is_mapped_double = TRUE;
        }
    }

    if (*clear_mode == FDB_CLEAR_MODE_NORMAL)
    {
        if (!is_mapped_double && !is_mapped_group)
        {
            _brg_if_clear_vlan_process(p_db_if, *mapped_vid);
        }
    }
    else if (*clear_mode == FDB_CLEAR_MODE_UNTAG)
    {
        if (!is_mapped_vlan && !is_mapped_group)
        {
            _brg_if_clear_vlan_process(p_db_if, *mapped_vid);
        }
    }
    else
    {
        if (!is_mapped_vlan && !is_mapped_double)
        {
            _brg_if_clear_vlan_process(p_db_if, *mapped_vid);
        }
    }

    return PM_E_NONE;
}

int32
vlanmap_api_if_delete_vmt(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_table_key_t vm_table_key;
    tbl_vlan_mapping_table_t    *p_db_vm_table = NULL;
    tbl_vlan_mapping_entry_t    *p_db_vm_entry = NULL;
    tbl_egress_mapping_entry_t  *p_db_egress_vm_entry = NULL;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    tbl_egress_mapping_entry_master_t *p_egress_master = tbl_egress_mapping_entry_get_master();
    tbl_brg_global_t*   p_db_brg_global = tbl_brg_global_get_brg_global();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    vid_t vid = 0;
    
    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
    sal_strncpy(vm_table_key.name, p_db_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vm_table = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vm_table)
    {
        return rc;
    }

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_db_vm_entry->key.table_name, p_db_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            --p_db_brg_global->vlan_mapping_entry_cnt;
            PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_egress_master->egress_mapping_entry_list, p_db_egress_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_db_egress_vm_entry->key.table_name, p_db_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            --p_db_brg_global->vlan_mapping_entry_cnt;
            PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
        }
    }

    /* remove fdb when unapply vmt */
    GLB_BMP_ITER_BEGIN(p_db_if->brgif->allowed_vlan.vlan_bmp, vid)
    {
        PM_E_RETURN(_brg_if_clear_vlan_process(p_db_if, vid));
    }
    GLB_BMP_ITER_END(p_db_if->brgif->allowed_vlan.vlan_bmp, vid);

    /* hsrv subscribe brgif->vlan_mapping_table for add/del vlan mapping entry */
    sal_memset(p_db_if->brgif->vlan_mapping_table, 0, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, p_db_if->brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));
    GLB_BMP_UNSET(p_db_vm_table->applied_ports, p_db_if->ifindex);
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vm_table, TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS));

    return rc;
}

int32
vlanmap_api_if_enable_vlantrans(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    if (p_db_if->brgif->vlan_translation_en && GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return PM_E_NONE;
}

int32
vlanmap_api_mod_applied_entry_one(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    uint32 *entry_num = pargs->argv[0];
    uint32 *mod_num   = pargs->argv[1];

    *mod_num += *entry_num;

    return PM_E_NONE;
}

int32
vlanmap_api_mod_applied_entry(tbl_vlan_mapping_table_t *p_db_vmt, uint32 entry_num, uint32 is_add)
{
    int32  rc = PM_E_NONE;
    uint32 mod_num = 0;
    tbl_iter_args_t    args;
    tbl_brg_global_t*  p_db_brg_global = tbl_brg_global_get_brg_global();

    args.argv[0] = &entry_num;
    args.argv[1] = &mod_num;

    vlanmap_api_iterate_applied_ports(p_db_vmt, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_mod_applied_entry_one, &args);

    if (is_add)
    {
        if (p_db_brg_global->vlan_mapping_entry_cnt + mod_num > GLB_VLAN_MAPPING_ENTRY_MAX)
        {
            return PM_E_EXCEED_MAX_NUM;
        }
        p_db_brg_global->vlan_mapping_entry_cnt += mod_num;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
    }
    else
    {
        if (p_db_brg_global->vlan_mapping_entry_cnt >= mod_num)
        {
            p_db_brg_global->vlan_mapping_entry_cnt -= mod_num;
            PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_db_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT));
        }
    }

    return rc;
}

int32
vlanmap_api_vmt_check_vlantrans(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next1 = NULL;
    tbl_vlan_mapping_entry_t *p_db_vm_entry = NULL;
    tbl_vlan_mapping_entry_t *p_db_vm_entry1 = NULL;

    if (!p_db_vmt)
    {
        return PM_E_NONE;
    }

    if (p_db_vmt->group_id)
    {
        CFG_CONFLICT_RET("Can't apply VLAN mapping table, raw vlan group exist");
    }

    if (p_db_vmt->egress_entry_num)
    {
        CFG_CONFLICT_RET("Can't apply VLAN mapping table, egress mapping entry exist");
    }

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_db_vm_entry->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            if (p_db_vm_entry->mapped_cvid || GLB_VLAN_NONE == p_db_vm_entry->key.raw_vid)
            {
                CFG_CONFLICT_RET("Can't apply VLAN mapping table, untagged raw vlan entry exist");
            }

            if (GLB_VLAN_ALL == p_db_vm_entry->key.raw_vid)
            {
                CFG_CONFLICT_RET("Can't apply VLAN mapping table, out-of-range raw vlan entry exist");
            }

            CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry1, listnode1, next1)
            {
                if (!sal_strncmp(p_db_vm_entry1->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
                    && p_db_vm_entry1->key.raw_vid != p_db_vm_entry->key.raw_vid)
                {
                    if (p_db_vm_entry1->mapped_cvid)
                    {
                        CFG_CONFLICT_RET("Can't apply VLAN mapping table, untagged raw vlan entry exist");
                    }
                    if (p_db_vm_entry1->mapped_svid == p_db_vm_entry->mapped_svid)
                    {
                        CFG_CONFLICT_RET("Can't apply VLAN mapping table, multi-to-one mapping entry exist");
                    }
                }
            }
        }
    }

    return rc;
}

int32
vlanmap_api_mapped_double_check(tbl_vlan_mapping_table_t *p_db_vmt, tbl_vlan_mapping_entry_t *p_vm_entry, 
        tbl_vlan_group_t *p_vlan_group, uint32 group_svid, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_vlan_mapping_entry_t *p_db_vme = NULL;
    tbl_vlan_mapping_entry_key_t      vme_key;
    tbl_vlan_group_t *p_db_vg = NULL;
    tbl_vlan_group_key_t      vg_key;
    int32  i = 0;

    /* for bug 48375 no need to check egress mapping entry */
#if 0
    tbl_egress_mapping_entry_master_t *p_egress_master = tbl_egress_mapping_entry_get_master();
    tbl_egress_mapping_entry_t *p_db_egress_vme = NULL;
    CTCLIB_SLIST_LOOP_DEL(p_egress_master->egress_mapping_entry_list, p_db_egress_vme, listnode, next)
    {
        if (sal_strncmp(p_db_egress_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            continue;
        }

        if (p_db_egress_vme->key.raw_svid == p_db_evc->mapped_svid 
            && p_db_egress_vme->key.raw_cvid != p_db_evc->mapped_cvid)
        {
            CFG_CONFLICT_RET("Can't add mapping entry, egress mapping entry which have the same raw svid exist");
        }
    }
#endif

    if (p_vm_entry)
    {
        /* check for untag->c+s, conflict with same c->c+s */
        if (p_vm_entry->mapped_cvid)
        {
            if (p_db_vmt->group_id)
            {
                sal_memset(&vg_key, 0, sizeof(vg_key));
                vg_key.group_id = p_db_vmt->group_id;
                p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
                if (!p_db_vg)
                {
                    return rc;
                }
                if (p_vm_entry->mapped_svid == p_db_vmt->group_mapped_svid)
                {
                    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
                    {
                        if (p_db_vg->valid[i])
                        {
                            if (p_vm_entry->mapped_cvid >= p_db_vg->min_vid[i] && p_vm_entry->mapped_cvid <= p_db_vg->max_vid[i])
                            {
                                CFG_CONFLICT_RET("Can't add mapping entry, same cvlan->svlan mapping entry exist in vlan group");
                                break;
                            }
                        }
                    }
                }
            }

            sal_memset(&vme_key, 0, sizeof(vme_key));
            sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
            vme_key.raw_vid = p_vm_entry->mapped_cvid;
            p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
            if (p_db_vme)
            {
                if (p_db_vme->mapped_svid == p_vm_entry->mapped_svid)
                {
                    CFG_CONFLICT_RET("Can't add mapping entry, same cvlan->svlan mapping entry exist");
                }
            }

            sal_memset(&vme_key, 0, sizeof(vme_key));
            sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
            vme_key.raw_vid = GLB_VLAN_ALL;
            p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
            if (p_db_vme)
            {
                if (p_db_vme->mapped_svid == p_vm_entry->mapped_svid)
                {
                    CFG_CONFLICT_RET("Can't add mapping entry, out-of-range->same svlan mapping entry exist");
                }
            }

        }
        else
        {
            /* check for out-of-range -> svlan, conflict with untag->c+s */
            if (p_vm_entry->key.raw_vid == GLB_VLAN_ALL)
            {
                sal_memset(&vme_key, 0, sizeof(vme_key));
                sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
                vme_key.raw_vid = GLB_VLAN_NONE;
                p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
                if (p_db_vme)
                {
                    if (p_db_vme->mapped_svid == p_vm_entry->mapped_svid && p_db_vme->mapped_cvid)
                    {
                        CFG_CONFLICT_RET("Can't add mapping entry, untag->cvlan+(same)svlan mapping entry exist");
                    }
                }
            }
            /* check for normal cvlan -> svlan, conflict with untag->c+s */
            else
            {
                sal_memset(&vme_key, 0, sizeof(vme_key));
                sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
                vme_key.raw_vid = GLB_VLAN_NONE;
                p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
                if (p_db_vme)
                {
                    if (p_db_vme->mapped_svid == p_vm_entry->mapped_svid && p_db_vme->mapped_cvid == p_vm_entry->key.raw_vid)
                    {
                        CFG_CONFLICT_RET("Can't add mapping entry, untag->(same with raw-vlan)cvlan+(same)svlan mapping entry exist");
                    }
                }
            }
        }
    }

    /* check for vlan group -> svlan, conflict with untag->c+s */
    if (p_vlan_group)
    {
        sal_memset(&vme_key, 0, sizeof(vme_key));
        sal_strcpy(vme_key.table_name, p_db_vmt->key.name);
        vme_key.raw_vid = GLB_VLAN_NONE;
        p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vme_key);
        if (p_db_vme)
        {
            if (p_db_vme->mapped_svid == group_svid && p_db_vme->mapped_cvid)
            {
                for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
                {
                    if (p_vlan_group->valid[i])
                    {
                        if (p_db_vme->mapped_cvid >= p_vlan_group->min_vid[i] && p_db_vme->mapped_cvid <= p_vlan_group->max_vid[i])
                        {
                            CFG_CONFLICT_RET("Can't add vlan group entry, untag->(same with one of raw-vlan in group)cvlan+(same)svlan mapping entry exist");
                            break;
                        }
                    }
                }
            }
        }
    }

    return rc;
}

int32
vlanmap_api_mapped_evc_check(tbl_vlan_mapping_table_t *p_db_vmt, 
        uint16 raw_svid, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    tbl_mapped_vlan_info_t *p_db_mapped_info = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode, next)
    {
        if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            continue;
        }

        if (p_db_mapped_info->mapped_svid == raw_svid)
        {
            CFG_CONFLICT_RET("Can't add egress mapping entry, mapping entry which have the same mapped svid exist");
        }
    }

    if (p_db_vmt->group_id && p_db_vmt->group_mapped_svid == raw_svid)
    {
        CFG_CONFLICT_RET("Can't add egress mapping entry, mapping entry which have the same mapped svid exist");
    }

    return rc;
}

int32
vlanmap_api_isin_vlan_group(uint32 group_id, uint32 vid)
{
    tbl_vlan_group_key_t vg_key;
    tbl_vlan_group_t    *p_db_vg = NULL;
    uint32  i = 0;

    sal_memset(&vg_key, 0, sizeof(vg_key));
    vg_key.group_id = group_id;
    p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
    if (!p_db_vg)
    {
        return FALSE;
    }
    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vg->valid[i])
        {
            if ((vid >= p_db_vg->min_vid[i]) && (vid <= p_db_vg->max_vid[i]))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int32
vlanmap_api_vmt_add_vlan_group(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_table_t *p_vm_table, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_evc_entry_key_t  evc_key;
    tbl_evc_entry_t     *p_db_evc = NULL;
    tbl_vlan_group_key_t vlan_group_key;
    tbl_vlan_group_t    *p_db_vlan_group = NULL;
    tbl_vlan_mapping_entry_key_t vm_entry_key;
    tbl_vlan_mapping_entry_t    *p_db_vm_entry = NULL;
    uint32 group_valid_count = 0;
    int32  i  = 0;
    uint16 raw_vid = 0;

    sal_memset(&evc_key, 0, sizeof(tbl_evc_entry_key_t));
    sal_strncpy(evc_key.name, p_vm_table->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
    p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", p_vm_table->group_evc_entry_name);
    }
    if (!p_db_evc->mapped_svid)
    {
        CFG_CONFLICT_RET("EVC's mapped-vlan isn't configured");
    }
    if (p_db_evc->mapped_cvid)
    {
        CFG_CONFLICT_RET("Only untagged packet can be added two tags");
    }
    if (vlanmap_api_iterate_applied_ports(p_db_vmt, 
        (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_if_enable_vlantrans, NULL))
    {
        CFG_CONFLICT_RET("VLAN translation have enabled, only one-to-one mapping entry can be added");
    }

    sal_memset(&vlan_group_key, 0, sizeof(tbl_vlan_group_key_t));
    vlan_group_key.group_id = p_vm_table->group_id;
    p_db_vlan_group = tbl_vlan_group_get_vlan_group(&vlan_group_key);
    if (!p_db_vlan_group)
    {
        CFG_CONFLICT_RET("The raw vlan group is not created");
    }

    if (p_db_vmt->group_id == p_vm_table->group_id && !sal_strcmp(p_db_vmt->group_evc_entry_name, p_vm_table->group_evc_entry_name))
    {
        return rc;
    }

    if (p_db_vmt->group_id == p_vm_table->group_id)
    {
        CFG_CONFLICT_RET("VLAN mapping entry exist for raw vlan group %d", p_db_vmt->group_id);
    }

    if (p_db_vmt->group_id)
    {
        CFG_CONFLICT_RET("Vlan mapping table only support to bind with single raw vlan group");
    }

    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vlan_group->valid[i])
        {
            for (raw_vid = p_db_vlan_group->min_vid[i]; raw_vid <= p_db_vlan_group->max_vid[i]; raw_vid++)
            {
                sal_memset(&vm_entry_key, 0, sizeof(tbl_vlan_mapping_entry_key_t));
                vm_entry_key.raw_vid = raw_vid;
                sal_strncpy(vm_entry_key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
                p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
                if (p_db_vm_entry)
                {
                    CFG_CONFLICT_RET("VLAN mapping entry exist for raw vlan %d", raw_vid);
                }
            }
        }
    }

    /* check for untag->c+s and c->c+s */
    PM_E_RETURN(vlanmap_api_mapped_double_check(p_db_vmt, NULL, p_db_vlan_group, p_db_evc->mapped_svid, para));

    group_valid_count = vlanmap_api_get_valid_group(p_db_vlan_group);
    if (vlanmap_api_mod_applied_entry(p_db_vmt, group_valid_count, TRUE))
    {
        CFG_CONFLICT_RET("VLAN mapping entry exceed max number %d", GLB_VLAN_MAPPING_ENTRY_MAX);
    }

    p_db_vmt->group_id = p_vm_table->group_id;
    sal_strncpy(p_db_vmt->group_evc_entry_name, p_vm_table->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
    p_db_vmt->group_mapped_svid = p_db_evc->mapped_svid;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME));
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID));
    /* hsrv subscibe group changed event, so modify group id after evc entry */
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID));

    p_db_vmt->entry_num += group_valid_count;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM));
    ++p_db_evc->ref_count;
    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));

    return rc;
}

int32
vlanmap_api_vmt_del_vlan_group(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para)
{
    int32         rc  = PM_E_NONE;
    tbl_vlan_group_key_t vlan_group_key;
    tbl_vlan_group_t    *p_db_vlan_group = NULL;
    tbl_evc_entry_key_t  evc_key;
    tbl_evc_entry_t     *p_db_evc = NULL;
    tbl_iter_args_t      args;
    uint32 fdb_clear_mode = FDB_CLEAR_MODE_GROUP;

    sal_memset(&evc_key, 0, sizeof(tbl_evc_entry_key_t));
    sal_strncpy(evc_key.name, p_db_vmt->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
    p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", p_db_vmt->group_evc_entry_name);
    }

    sal_memset(&vlan_group_key, 0, sizeof(tbl_vlan_group_key_t));
    vlan_group_key.group_id = p_db_vmt->group_id;
    p_db_vlan_group = tbl_vlan_group_get_vlan_group(&vlan_group_key);
    if (!p_db_vlan_group)
    {
        CFG_CONFLICT_RET("The raw vlan group is not created");
    }

    uint32 group_valid_count = 0;
    group_valid_count = vlanmap_api_get_valid_group(p_db_vlan_group);
    vlanmap_api_mod_applied_entry(p_db_vmt, group_valid_count, FALSE);

    /* clear fdb when delete vme */
    args.argv[0] = (uint16 *) &(p_db_vmt->group_mapped_svid);
    args.argv[1] = &fdb_clear_mode;
    args.argv[2] = p_db_vmt;
    vlanmap_api_iterate_applied_ports(p_db_vmt, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_clear_mapped_vlan_fdb, &args);

    p_db_vmt->group_id = 0;
    sal_memset(p_db_vmt->group_evc_entry_name, 0, GLB_VLAN_EVC_NAME_LEN);
    p_db_vmt->group_mapped_svid = 0;

    /* hsrv subscibe group del event still need mapped svid, so modify group id first */
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID));
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_EVC_ENTRY_NAME));
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_GROUP_MAPPED_SVID));

    p_db_vmt->entry_num -= group_valid_count;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM));

    --p_db_evc->ref_count;
    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));

    return rc;
}

int32
vlanmap_api_vmt_add_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_entry_t *p_db_vm_entry = NULL;
    tbl_mapped_vlan_info_t    mapped_info;
    tbl_mapped_vlan_info_t   *p_db_mapped_info = NULL;

    if (vlanmap_api_iterate_applied_ports(p_db_vmt, 
        (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_if_enable_vlantrans, NULL))
    {
        if (GLB_VLAN_NONE == p_vm_entry->key.raw_vid || GLB_VLAN_ALL == p_vm_entry->key.raw_vid)
        {
            CFG_CONFLICT_RET("VLAN mapping table have applied to vlan translation, can't add untagged raw vlan or out-of-range entry");
        }

        if (p_vm_entry->mapped_cvid)
        {
            CFG_CONFLICT_RET("VLAN mapping table have applied to vlan translation, can't apply mapped cvid EVC");
        }

        CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
        {
            if (!sal_strncmp(p_db_vm_entry->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                if (p_db_vm_entry->mapped_svid ==  p_vm_entry->mapped_svid)
                {
                    CFG_CONFLICT_RET("VLAN mapping table have applied to vlan translation, can't apply multi-to-one mapping entry");
                }
            }
        }
    }

    if (GLB_VLAN_NONE != p_vm_entry->key.raw_vid && GLB_VLAN_NONE != p_vm_entry->mapped_cvid)
    {
        CFG_CONFLICT_RET("Only untagged packet can be added two tags");
    }

    /* check for untag->c+s and c->c+s */
    PM_E_RETURN(vlanmap_api_mapped_double_check(p_db_vmt, p_vm_entry, NULL, 0, para));

    if (vlanmap_api_mod_applied_entry(p_db_vmt, 1, TRUE))
    {
        CFG_CONFLICT_RET("VLAN mapping entry exceed max number %d", GLB_VLAN_MAPPING_ENTRY_MAX);
    }
    PM_E_RETURN(tbl_vlan_mapping_entry_add_vlan_mapping_entry(p_vm_entry));

    if (GLB_VLAN_NONE == p_vm_entry->mapped_cvid)
    {
        sal_memset(&mapped_info, 0, sizeof(mapped_info));
        sal_strncpy(mapped_info.key.evc_name, p_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
        sal_strncpy(mapped_info.key.table_name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_mapped_info = tbl_mapped_vlan_info_get_mapped_vlan_info(&mapped_info.key);
        if (!p_db_mapped_info)
        {
            GLB_BMP_INIT(mapped_info.raw_vid_bmp);
            GLB_BMP_SET(mapped_info.raw_vid_bmp, p_vm_entry->key.raw_vid);
            mapped_info.mapped_svid = p_vm_entry->mapped_svid;
            mapped_info.mapped_cvid = p_vm_entry->mapped_cvid;
            PM_E_RETURN(tbl_mapped_vlan_info_add_mapped_vlan_info(&mapped_info));
        }
        else
        {
            GLB_BMP_SET(p_db_mapped_info->raw_vid_bmp, p_vm_entry->key.raw_vid);
            PM_E_RETURN(tbl_mapped_vlan_info_set_mapped_vlan_info_field(p_db_mapped_info, TBL_MAPPED_VLAN_INFO_FLD_RAW_VID_BMP));
        }
    }

    p_db_vmt->entry_num++;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM));

    return rc;
}

int32
vlanmap_api_vmt_del_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_mapped_vlan_info_t    mapped_info;
    tbl_mapped_vlan_info_t   *p_db_mapped_info = NULL;
    tbl_vlan_mapping_entry_key_t    vm_entry_key;
    tbl_vlan_mapping_entry_t       *p_db_vme = NULL;
    uint32 bmp_zero    = TRUE;
    tbl_iter_args_t    args;
    uint32 fdb_clear_mode = FDB_CLEAR_MODE_NORMAL;

    vlanmap_api_mod_applied_entry(p_db_vmt, 1, FALSE);

    sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
    vm_entry_key.raw_vid = p_vm_entry->key.raw_vid;
    sal_strncpy(vm_entry_key.table_name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
    if (!p_db_vme)
    {
        return rc;
    }

    if (GLB_VLAN_NONE == p_db_vme->mapped_cvid)
    {
        sal_memset(&mapped_info, 0, sizeof(mapped_info));
        sal_strncpy(mapped_info.key.evc_name, p_db_vme->evc_entry, GLB_VLAN_EVC_NAME_LEN);
        sal_strncpy(mapped_info.key.table_name, p_db_vme->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_mapped_info = tbl_mapped_vlan_info_get_mapped_vlan_info(&mapped_info.key);
        if (p_db_mapped_info)
        {
            if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, p_db_vme->key.raw_vid))
            {
                GLB_BMP_UNSET(p_db_mapped_info->raw_vid_bmp, p_db_vme->key.raw_vid);
                PM_E_RETURN(tbl_mapped_vlan_info_set_mapped_vlan_info_field(p_db_mapped_info, TBL_MAPPED_VLAN_INFO_FLD_RAW_VID_BMP));
            }

            GLB_BMP_CHECK_ALL_ZERO(p_db_mapped_info->raw_vid_bmp, bmp_zero);
            if (bmp_zero)
            {
                /* clear fdb when delete vme */
                args.argv[0] = (uint16 *) &(p_db_mapped_info->mapped_svid);
                args.argv[1] = &fdb_clear_mode;
                args.argv[2] = p_db_vmt;
                vlanmap_api_iterate_applied_ports(p_db_vmt, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_clear_mapped_vlan_fdb, &args);

                PM_E_RETURN(tbl_mapped_vlan_info_del_mapped_vlan_info(&mapped_info.key));
            }
        }
    }
    else
    {
        /* clear fdb when delete vme */
        fdb_clear_mode = FDB_CLEAR_MODE_UNTAG;
        args.argv[0] = (uint16 *) &(p_db_vme->mapped_svid);
        args.argv[1] = &fdb_clear_mode;
        args.argv[2] = p_db_vmt;
        vlanmap_api_iterate_applied_ports(p_db_vmt, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_clear_mapped_vlan_fdb, &args);
    }
    

    PM_E_RETURN(tbl_vlan_mapping_entry_del_vlan_mapping_entry(&p_vm_entry->key));

    p_db_vmt->entry_num--;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_ENTRY_NUM));

    return rc;
}

int32
vlanmap_api_vmt_add_egress_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
        tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;

    if (vlanmap_api_iterate_applied_ports(p_db_vmt, 
        (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_if_enable_vlantrans, NULL))
    {
        CFG_CONFLICT_RET("VLAN mapping table have applied to vlan translation, can't add egress mapping entry");
    }

    if (vlanmap_api_mod_applied_entry(p_db_vmt, 1, TRUE))
    {
        CFG_CONFLICT_RET("VLAN mapping entry exceed max number %d", GLB_VLAN_MAPPING_ENTRY_MAX);
    }
    PM_E_RETURN(tbl_egress_mapping_entry_add_egress_mapping_entry(p_vm_entry));

    p_db_vmt->egress_entry_num++;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM));

    return rc;
}

int32
vlanmap_api_vmt_del_egress_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
        tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;

    vlanmap_api_mod_applied_entry(p_db_vmt, 1, FALSE);
    PM_E_RETURN(tbl_egress_mapping_entry_del_egress_mapping_entry(&p_vm_entry->key));

    p_db_vmt->egress_entry_num--;
    PM_E_RETURN(tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt, TBL_VLAN_MAPPING_TABLE_FLD_EGRESS_ENTRY_NUM));

    return rc;
}

int32
vlanmap_api_get_valid_group(tbl_vlan_group_t *p_db_vlan_group)
{
    uint32 group_valid_count = 0;
    uint32 i = 0;

    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vlan_group->valid[i])
        {
            group_valid_count++;
        }
    }

    return group_valid_count;
}

int32
vlanmap_api_del_vm_table(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para)
{
    int32         rc  = PM_E_NONE;
    tbl_vlan_mapping_entry_t    *p_db_vm_entry = NULL;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    tbl_egress_mapping_entry_t    *p_egress_vm_entry = NULL;
    tbl_egress_mapping_entry_master_t *p_egress_master = tbl_egress_mapping_entry_get_master();
    tbl_evc_entry_key_t  evc_key;
    tbl_evc_entry_t     *p_db_evc = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
    tbl_mapped_vlan_info_t    mapped_info;
    tbl_mapped_vlan_info_t   *p_db_mapped_info = NULL;
    uint32 bmp_zero    = TRUE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    if (p_db_vmt->group_id)
    {
        vlanmap_api_vmt_del_vlan_group(p_db_vmt, para);
    }

    vlanmap_api_iterate_applied_ports(p_db_vmt, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_if_delete_vmt, NULL);

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_db_vm_entry->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            sal_memset(&evc_key, 0, sizeof(tbl_evc_entry_key_t));
            sal_strncpy(evc_key.name, p_db_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
            p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
            if (!p_db_evc)
            {
                CFG_CONFLICT_RET("EVC %s isn't exist", p_db_vmt->group_evc_entry_name);
            }
            --p_db_evc->ref_count;
            PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));

            sal_memset(&mapped_info, 0, sizeof(mapped_info));
            sal_strncpy(mapped_info.key.evc_name, p_db_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
            sal_strncpy(mapped_info.key.table_name, p_db_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_mapped_info = tbl_mapped_vlan_info_get_mapped_vlan_info(&mapped_info.key);
            if (p_db_mapped_info)
            {
                if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, p_db_vm_entry->key.raw_vid))
                {
                    GLB_BMP_UNSET(p_db_mapped_info->raw_vid_bmp, p_db_vm_entry->key.raw_vid);
                    PM_E_RETURN(tbl_mapped_vlan_info_set_mapped_vlan_info_field(p_db_mapped_info, TBL_MAPPED_VLAN_INFO_FLD_RAW_VID_BMP));
                }

                GLB_BMP_CHECK_ALL_ZERO(p_db_mapped_info->raw_vid_bmp, bmp_zero);
                if (bmp_zero)
                {
                    PM_E_RETURN(tbl_mapped_vlan_info_del_mapped_vlan_info(&mapped_info.key));
                }
            }

            PM_E_RETURN(tbl_vlan_mapping_entry_del_vlan_mapping_entry(&p_db_vm_entry->key));
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_egress_master->egress_mapping_entry_list, p_egress_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_egress_vm_entry->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            PM_E_RETURN(tbl_egress_mapping_entry_del_egress_mapping_entry(&p_egress_vm_entry->key));
        }
    }

    PM_E_RETURN(tbl_vlan_mapping_table_del_vlan_mapping_table(&p_db_vmt->key));

    return rc;
}

int32
vlanmap_api_add_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key, char *vlan_list, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_vlan_group_t   vlan_group;
    tbl_vlan_group_t  *p_db_vlan_group = NULL;
    vlan_bmp_t         vlan_bmp;
    vid_t              vid = 0;
    uint32             range_cnt = 0;

    sal_memset(&vlan_group, 0, sizeof(tbl_vlan_group_t));
    vlan_group.key.group_id = p_vlan_group_key->group_id;

    GLB_BMP_INIT(vlan_bmp);
    rc = cdb_bitmap_str2val(vlan_list, GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
    if (rc < 0)
    {
        para->p_rs->rc = rc;
        return rc;
    }
    GLB_BMP_ITER_BEGIN(vlan_bmp, vid)
    {
        if (GLB_VLAN_RANGE_ENTRY_NUM == range_cnt)
        {
            CFG_CONFLICT_RET("vlan group can be consisted of %d range maximal", GLB_VLAN_RANGE_ENTRY_NUM);
        }

        if (!vlan_group.min_vid[range_cnt])
        {
            vlan_group.min_vid[range_cnt] = vid;
            vlan_group.max_vid[range_cnt] = vid;
            vlan_group.valid[range_cnt] = TRUE;
            continue;
        }

        if (vid == (vlan_group.max_vid[range_cnt] + 1))
        {
            vlan_group.max_vid[range_cnt] = vid;
            continue;
        }

        if (vlan_group.min_vid[range_cnt] == vlan_group.max_vid[range_cnt])
        {
            CFG_CONFLICT_RET("Can't add single vlan %d to group", vlan_group.min_vid[range_cnt]);
        }

        range_cnt++;
        vlan_group.min_vid[range_cnt] = vid;
        vlan_group.max_vid[range_cnt] = vid;
        vlan_group.valid[range_cnt] = TRUE;
    }
    GLB_BMP_ITER_END(vlan_bmp, vid);
    if (vlan_group.min_vid[range_cnt] == vlan_group.max_vid[range_cnt])
    {
        CFG_CONFLICT_RET("Can't add single vlan %d to group", vlan_group.min_vid[range_cnt]);
    }

    p_db_vlan_group = tbl_vlan_group_get_vlan_group(p_vlan_group_key);
    if (p_db_vlan_group)
    {
         PM_E_RETURN(tbl_vlan_group_del_vlan_group(p_vlan_group_key));
    }

    PM_E_RETURN(tbl_vlan_group_add_vlan_group(&vlan_group));

    return rc;
}


char *
vlanmap_api_show_vlan_group(uint32 group_id, char *group_str)
{
    tbl_vlan_group_key_t vg_key;
    tbl_vlan_group_t    *p_db_vg = NULL;
    uint32  i = 0;

    sal_memset(&vg_key, 0, sizeof(vg_key));
    vg_key.group_id = group_id;
    p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
    if (!p_db_vg)
    {
        return group_str;
    }
    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vg->valid[i])
        {
            sal_sprintf(group_str + sal_strlen(group_str), "%d-%d,", p_db_vg->min_vid[i], p_db_vg->max_vid[i]);
        }
    }
    if (sal_strlen(group_str))
    {
        group_str[sal_strlen(group_str) - 1] = '\0';
    }

    return group_str;
}


int32
vlanmap_api_show_applied_if(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    char   *applied_str = pargs->argv[0];
    uint32 *first_line  = pargs->argv[1] ;

    char ifname_ext[IFNAME_SIZE] = {0};

    if (!(*first_line))
    {
        sal_sprintf(applied_str + sal_strlen(applied_str), "%-16s ", "");
    }

    sal_sprintf(applied_str + sal_strlen(applied_str), "%-16s ", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));

    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
        sal_sprintf(applied_str + sal_strlen(applied_str), "%s", "802.1Q-Tunnel\n");
    }
    else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        sal_sprintf(applied_str + sal_strlen(applied_str), "%s", "VLAN Translation\n");
    }
    else
    {
        sal_sprintf(applied_str + sal_strlen(applied_str), "%s", "<N/A>\n");
    }

    if (*first_line)
    {
        *first_line = FALSE;
    }

    return PM_E_NONE;
}

int32
vlanmap_api_show_vmt_applied_if(tbl_vlan_mapping_table_t *p_db_vmt, FILE *fp, cfg_cmd_para_t *para)
{
    int32        rc   = PM_E_NONE;
    tbl_vlan_mapping_table_master_t *p_vmt_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vm_table = NULL;
    uint32  vmt_fount   = FALSE;
    uint32  applied_ports_bmp_zero    = TRUE;
    uint32  first_line = TRUE;
    char    applied_str[5000];
    tbl_iter_args_t   args;
    
    CTCLIB_SLIST_LOOP_DEL(p_vmt_master->vlan_mapping_table_list, p_db_vm_table, listnode, next)
    {
        if (p_db_vmt)
        {
            if (sal_strncmp(p_db_vm_table->key.name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                continue;
            }
        }

        if (!vmt_fount)
        {
            sal_fprintf(fp, VLANMAP_TABLE_SHOW_APPLIED_IF_HDR_STR);
            sal_fprintf(fp, VLANMAP_TABLE_SHOW_APPLIED_IF_HDR_FMT);
            vmt_fount = TRUE;
        }

        sal_fprintf(fp, "%-16s ", p_db_vm_table->key.name);
        GLB_BMP_CHECK_ALL_ZERO(p_db_vm_table->applied_ports, applied_ports_bmp_zero);
        if (applied_ports_bmp_zero)
        {
            sal_fprintf(fp, "<No Interface>\n");
            continue;
        }

        sal_memset(applied_str, 0, sizeof(applied_str));
        args.argv[0] = applied_str;
        args.argv[1] = &first_line;
        vlanmap_api_iterate_applied_ports(p_db_vm_table, (VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)vlanmap_api_show_applied_if, &args);
        sal_fprintf(fp, applied_str);
        first_line = TRUE;
    }
    if (!vmt_fount)
    {
        sal_fprintf(fp, "VLAN mapping table %s not found", p_db_vmt ? p_db_vmt->key.name : "\b");
    }

    return rc;
}

int32
vlanmap_api_show_vmt(tbl_vlan_mapping_table_t *p_db_vmt, char *evc_name, uint32 raw_vid, 
        FILE *fp, cfg_cmd_para_t *para)
{
    int32        rc   = PM_E_NONE;
    tbl_vlan_mapping_table_master_t *p_vmt_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vm_table = NULL;
    tbl_vlan_mapping_entry_master_t *p_vme_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next1 = NULL;
    tbl_vlan_mapping_entry_key_t vm_entry_key;
    tbl_vlan_mapping_entry_t *p_db_vm_entry = NULL;
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    ctclib_slistnode_t *listnode2 = NULL;
    ctclib_slistnode_t *next2 = NULL;
    tbl_mapped_vlan_info_t *p_db_mapped_info = NULL;
    tbl_evc_entry_key_t evc_key;
    tbl_evc_entry_t    *p_db_evc = NULL;
    uint32  entry_exist = FALSE;
    uint32  first_line  = TRUE;
    uint32  vmt_fount   = FALSE;
    char    group_str[100] = {0};
    char    double_mapped_str[12] = {0};
    uint32  all_zero = TRUE;
    char    buf[MAX_CMD_STR_LEN];
    vid_t   raw_vlan;
    vlan_bmp_t bmp;

    if (!p_vmt_master || !p_vmt_master)
    {
        CFG_CONFLICT_RET("Can't find vlan mapping table");
    }

    CTCLIB_SLIST_LOOP_DEL(p_vmt_master->vlan_mapping_table_list, p_db_vm_table, listnode, next)
    {
        if (p_db_vmt)
        {
            if (sal_strncmp(p_db_vm_table->key.name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                continue;
            }
        }

        if (raw_vid != 0xffff)
        {
            CTCLIB_SLIST_LOOP_DEL(p_vme_master->vlan_mapping_entry_list, p_db_vm_entry, listnode1, next1)
            {
                if (sal_strncmp(p_db_vm_entry->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX) 
                    || p_db_vm_entry->key.raw_vid != raw_vid)
                {
                    continue;
                }
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                if (!p_db_vm_entry->mapped_cvid)
                {
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT,
                        p_db_vm_entry->evc_entry, p_db_vm_entry->mapped_svid);
                }
                else
                {
                    sal_sprintf(double_mapped_str, "%d(%d)", p_db_vm_entry->mapped_svid, p_db_vm_entry->mapped_cvid);
                    sal_fprintf(fp, "%-16s %-12s ", p_db_vm_entry->evc_entry, double_mapped_str);
                }

                /* only have one entry related to specific raw vid in a vmt */
                VLANMAP_TABLE_SHOW_RAW_VID_FTM(fp, raw_vid);
                entry_exist = TRUE;
            }
            if (!entry_exist)
            {
                if (p_db_vmt->group_id)
                {
                    if (vlanmap_api_isin_vlan_group(p_db_vmt->group_id, raw_vid))
                    {
                        sal_memset(&evc_key, 0, sizeof(evc_key));
                        sal_strncpy(evc_key.name, p_db_vmt->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
                        p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
                        if (!p_db_evc)
                        {
                            return rc;
                        }
                        sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                        sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
                        VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                        sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT,
                                p_db_vmt->group_evc_entry_name, p_db_evc->mapped_svid);
                        sal_fprintf(fp, "%d in group %d\n", raw_vid, p_db_vmt->group_id);
                        return rc;
                    }
                }
                CFG_VLAN_MAPPING_ENTRY_NOTEXIST_RET(raw_vid);
            }
            return rc;
        }
        else if (sal_strlen(evc_name))
        {
            sal_memset(&evc_key, 0, sizeof(evc_key));
            sal_strncpy(evc_key.name, evc_name, GLB_VLAN_EVC_NAME_LEN);
            p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
            if (!p_db_evc)
            {
                CFG_CONFLICT_RET("EVC:%s not exist", evc_name);
                return rc;
            }
            CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode2, next2)
            {
                if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
                    || sal_strncmp(p_db_mapped_info->key.evc_name, evc_name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
                {
                    continue;
                }
                if (!entry_exist)
                {
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
                }
                
                GLB_BMP_INIT(bmp);
                GLB_BMP_ITER_BEGIN(p_db_mapped_info->raw_vid_bmp, raw_vlan)
                {
                    if (GLB_VLAN_NONE != raw_vlan && GLB_VLAN_ALL != raw_vlan)
                    {
                        GLB_BMP_SET(bmp, raw_vlan);
                    }
                }
                GLB_BMP_ITER_END(p_db_mapped_info->raw_vid_bmp, raw_vlan);

                if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, GLB_VLAN_NONE))
                {
                    VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                    if (!p_db_evc->mapped_cvid)
                    {
                        sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, evc_name, p_db_evc->mapped_svid);
                    }
                    else
                    {
                        sal_sprintf(double_mapped_str, "%d(%d)", p_db_evc->mapped_svid, p_db_evc->mapped_cvid);
                        sal_fprintf(fp, "%-16s %-12s ", evc_name, double_mapped_str);
                    }
                    VLANMAP_TABLE_SHOW_RAW_VID_FTM(fp, GLB_VLAN_NONE);
                    first_line = FALSE;
                }
                if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, GLB_VLAN_ALL))
                {
                    VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, evc_name, p_db_evc->mapped_svid);
                    VLANMAP_TABLE_SHOW_RAW_VID_FTM(fp, GLB_VLAN_ALL);
                    first_line = FALSE;
                }
                
                GLB_BMP_CHECK_ALL_ZERO(bmp, all_zero);
                if (!all_zero)
                {
                    sal_memset(buf, 0, sizeof(buf));
                    VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, evc_name, p_db_evc->mapped_svid);
                    sal_fprintf(fp, "%s\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, bmp, GLB_VLAN_BMP_WORD_MAX));
                }

                entry_exist = TRUE;
                first_line = FALSE;
            }

            sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
            vm_entry_key.raw_vid = GLB_VLAN_NONE;
            sal_strncpy(vm_entry_key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
            if (GLB_VLAN_NONE != p_db_evc->mapped_cvid && p_db_vm_entry 
             && !sal_strncmp(p_db_vm_entry->evc_entry, evc_name, GLB_VLAN_EVC_NAME_LEN))
            {
                if (!entry_exist)
                {
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
                }

                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                sal_sprintf(double_mapped_str, "%d(%d)", p_db_evc->mapped_svid, p_db_evc->mapped_cvid);
                sal_fprintf(fp, "%-16s %-12s ", evc_name, double_mapped_str);
                sal_fprintf(fp, "untagged\n");
                entry_exist = TRUE;
                first_line = FALSE;
            }

            if (p_db_vmt->group_id && !sal_strncmp(p_db_vmt->group_evc_entry_name, evc_name, GLB_VLAN_EVC_NAME_LEN))
            {
                sal_memset(&evc_key, 0, sizeof(evc_key));
                sal_strncpy(evc_key.name, evc_name, GLB_VLAN_EVC_NAME_LEN);
                p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
                if (!p_db_evc)
                {
                    return rc;
                }
                if (!entry_exist)
                {
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
                }
                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vmt);
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT,
                        p_db_evc->key.name, p_db_evc->mapped_svid);
                sal_fprintf(fp, "%s\n", vlanmap_api_show_vlan_group(p_db_vmt->group_id, group_str));
                entry_exist = TRUE;
            }

            if (!entry_exist)
            {
                CFG_CONFLICT_RET("Entry related to EVC : %s does not exist in vlan mapping table %s",
                                  evc_name, p_db_vmt->key.name);
            }
            return rc;
        }
        else
        {
            if (!entry_exist && !vmt_fount)
            {
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE);
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_ALL_TITLE_FTM);
            }
            CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_db_mapped_info, listnode2, next2)
            {
                if (sal_strncmp(p_db_mapped_info->key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
                {
                    continue;
                }

                sal_memset(&evc_key, 0, sizeof(evc_key));
                sal_strncpy(evc_key.name, p_db_mapped_info->key.evc_name, GLB_VLAN_EVC_NAME_LEN);
                p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
                if (!p_db_evc)
                {
                    sal_fprintf(fp, "EVC:%s not exist", evc_name);
                    return rc;
                }
                
                GLB_BMP_INIT(bmp);
                GLB_BMP_ITER_BEGIN(p_db_mapped_info->raw_vid_bmp, raw_vlan)
                {
                    if (GLB_VLAN_NONE != raw_vlan && GLB_VLAN_ALL != raw_vlan)
                    {
                        GLB_BMP_SET(bmp, raw_vlan);
                    }
                }
                GLB_BMP_ITER_END(p_db_mapped_info->raw_vid_bmp, raw_vlan);

                if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, GLB_VLAN_NONE))
                {
                    VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vm_table);
                    if (!p_db_evc->mapped_cvid)
                    {
                        sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, p_db_mapped_info->key.evc_name, p_db_evc->mapped_svid);
                    }
                    else
                    {
                        sal_sprintf(double_mapped_str, "%d(%d)", p_db_evc->mapped_svid, p_db_evc->mapped_cvid);
                        sal_fprintf(fp, "%-16s %-12s ", p_db_mapped_info->key.evc_name, double_mapped_str);
                    }
                    VLANMAP_TABLE_SHOW_RAW_VID_FTM(fp, GLB_VLAN_NONE);
                    first_line = FALSE;
                }

                /* move out-of-range entry to end of lines */
                /* if (GLB_BMP_ISSET(p_db_mapped_info->raw_vid_bmp, GLB_VLAN_ALL)) */
                
                GLB_BMP_CHECK_ALL_ZERO(bmp, all_zero);
                if (!all_zero)
                {
                    sal_memset(buf, 0, sizeof(buf));
                    VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vm_table);
                    sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, p_db_mapped_info->key.evc_name, p_db_evc->mapped_svid);
                    sal_fprintf(fp, "%s\n", cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, bmp, GLB_VLAN_BMP_WORD_MAX));
                }

                entry_exist = TRUE;
                first_line = FALSE;
            }

            sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
            vm_entry_key.raw_vid = GLB_VLAN_NONE;
            sal_strncpy(vm_entry_key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
            if (p_db_vm_entry && GLB_VLAN_NONE != p_db_vm_entry->mapped_cvid )
            {
                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vm_table);
                sal_sprintf(double_mapped_str, "%d(%d)", p_db_vm_entry->mapped_svid, p_db_vm_entry->mapped_cvid);
                sal_fprintf(fp, "%-16s %-12s ", p_db_vm_entry->evc_entry, double_mapped_str);
                sal_fprintf(fp, "untagged\n");
                entry_exist = TRUE;
                first_line = FALSE;
            }

            if (p_db_vm_table->group_id)
            {
                sal_memset(&evc_key, 0, sizeof(evc_key));
                sal_strncpy(evc_key.name, p_db_vm_table->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
                p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
                if (!p_db_evc)
                {
                    return rc;
                }

                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vm_table);
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT,
                        p_db_evc->key.name, p_db_evc->mapped_svid);
                sal_memset(group_str, 0, sizeof(group_str));
                sal_fprintf(fp, "%s\n", vlanmap_api_show_vlan_group(p_db_vm_table->group_id, group_str));
                entry_exist = TRUE;
            }

            /* move out-of-range entry to end of lines */
            sal_memset(&vm_entry_key, 0, sizeof(vm_entry_key));
            vm_entry_key.raw_vid = GLB_VLAN_ALL;
            sal_strncpy(vm_entry_key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&vm_entry_key);
            if (p_db_vm_entry)
            {
                sal_memset(&evc_key, 0, sizeof(evc_key));
                sal_strncpy(evc_key.name, p_db_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
                p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
                if (!p_db_evc)
                {
                    return rc;
                }

                VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(first_line, fp, p_db_vm_table);
                sal_fprintf(fp, VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT, p_db_evc->key.name, p_db_evc->mapped_svid);
                VLANMAP_TABLE_SHOW_RAW_VID_FTM(fp, GLB_VLAN_ALL);
                first_line = FALSE;
                entry_exist = TRUE;
            }

            if (!entry_exist)
            {
                sal_fprintf(fp, "%-16s ", p_db_vm_table->key.name);
                sal_fprintf(fp, "<mapping entry not exist>\n");
            }

            first_line  = TRUE;
            entry_exist = FALSE;
            vmt_fount   = TRUE;
        }
    }
    if (!vmt_fount)
    {
        CFG_CONFLICT_RET("VLAN mapping table %s not found", p_db_vmt ? p_db_vmt->key.name : "\b");
    }

    return rc;
}

int32
vlanmap_api_show_evc_one(tbl_evc_entry_t *p_db_evc, FILE *fp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 usedinvmt  = 0;
    int32 used = 0;

    tbl_vlan_mapping_table_master_t *p_vmt_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vmt = NULL;
    tbl_vlan_mapping_entry_master_t *p_vme_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next1 = NULL;
    tbl_vlan_mapping_entry_t *p_db_vme = NULL;

    sal_fprintf(fp, "EVC %s:\n", p_db_evc->key.name);
    if (p_db_evc->mapped_svid)
    {
        sal_fprintf(fp, " EVC Dot1Q SVLANID                : %d\n", p_db_evc->mapped_svid);
    }
    else
    {
        sal_fprintf(fp, " EVC Dot1Q SVLANID                : N/A\n");
    }

    if (p_db_evc->mapped_cvid)
    {
        sal_fprintf(fp, " EVC Dot1Q CVLANID                : %d\n", p_db_evc->mapped_cvid);
    }
    else
    {
        sal_fprintf(fp, " EVC Dot1Q CVLANID                : N/A\n");
    }

    sal_fprintf(fp, " Number of using this EVC         : %d\n", p_db_evc->ref_count);

    sal_fprintf(fp, " Used in VLAN Mapping table       :");
    CTCLIB_SLIST_LOOP_DEL(p_vmt_master->vlan_mapping_table_list, p_db_vmt, listnode, next)
    {
        usedinvmt = FALSE;
        CTCLIB_SLIST_LOOP_DEL(p_vme_master->vlan_mapping_entry_list, p_db_vme, listnode1, next1)
        {
            if (sal_strncmp(p_db_vmt->key.name, p_db_vme->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
            {
                continue;
            }

            if (!strncmp(p_db_vme->evc_entry, p_db_evc->key.name, GLB_VLAN_EVC_NAME_LEN))
            {
                sal_fprintf(fp, " %s", p_db_vmt->key.name);
                usedinvmt = TRUE;
                used = TRUE;
                break;
            }
        }

        if (!usedinvmt)
        {
            if (p_db_vmt->group_id && !strncmp(p_db_vmt->group_evc_entry_name, p_db_evc->key.name, GLB_VLAN_EVC_NAME_LEN))
            {
                sal_fprintf(fp, " %s", p_db_vmt->key.name);
                used = TRUE;
            }
        }
    }
    if (!used)
    {
        sal_fprintf(fp, " N/A\n");
    }
    else
    {
        sal_fprintf(fp, "\n");
    }

    return rc;
}

static int32
_vlan_mapping_join_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    int rc = PM_E_NONE;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();

    if (GLB_IF_MODE_L2 != p_db_mem_if->mode || GLB_IF_MODE_L2 != p_db_agg_if->mode)
    {
        return rc;
    }

    if (sal_strlen(p_db_mem_if->brgif->vlan_mapping_table))
    {
        vlanmap_api_if_delete_vmt(p_db_mem_if, NULL);
    }

    if (p_db_agg_if->brgif->qinq_type != p_db_mem_if->brgif->qinq_type)
    {
        brg_if_set_qinq_type(p_db_mem_if, p_db_agg_if->brgif->qinq_type, FALSE);
    }

    if (p_db_agg_if->brgif->vlan_translation_en != p_db_mem_if->brgif->vlan_translation_en)
    {
        brg_if_set_vlan_translation_en(p_db_mem_if, p_db_agg_if->brgif->vlan_translation_en, FALSE);
    }

    if (p_db_agg_if->brgif->qinq_default_fwd != p_db_mem_if->brgif->qinq_default_fwd)
    {
        brg_if_set_qinq_default_fwd(p_db_mem_if, p_db_agg_if->brgif->qinq_default_fwd, FALSE);
    }

    if (p_db_agg_if->brgif->vlan_trans_default_fwd != p_db_mem_if->brgif->vlan_trans_default_fwd)
    {
        brg_if_set_vlan_trans_default_fwd(p_db_mem_if, p_db_agg_if->brgif->vlan_trans_default_fwd, FALSE);
    }

    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_mem_if->vlan_type 
        && GLB_VLAN_QINQ_TYPE_BASIC == p_db_mem_if->brgif->qinq_type)
    {
        /* remove vlan mapping default entry for basic qinq agg member will be process in hsrv */
    }
    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_mem_if->vlan_type && sal_strlen(p_db_agg_if->brgif->vlan_mapping_table))
    {
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_agg_if, p_db_agg_if->brgif, DS_BRGIF_FLD_VLAN_MAPPING_TABLE));
    }

    if (p_db_mem_if->svlan_tpid == p_brg_global->vlan_tpid_type1)
    {
        p_brg_global->tpid_type1_ref_cnt--;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1));
    }
    else if (p_db_mem_if->svlan_tpid == p_brg_global->vlan_tpid_type2)
    {
        p_brg_global->tpid_type2_ref_cnt--;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2));
    }

    return rc;
}

static int32
_vlan_mapping_leave_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    int rc = PM_E_NONE;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();

    if (GLB_IF_MODE_L2 != p_db_mem_if->mode || GLB_IF_MODE_L2 != p_db_agg_if->mode)
    {
        return rc;
    }

    if (sal_strlen(p_db_agg_if->brgif->vlan_mapping_table))
    {
        brg_if_apply_vlanmap_table(p_db_mem_if, p_db_agg_if->brgif, TRUE, NULL);
    }

    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_mem_if->vlan_type 
        && GLB_VLAN_QINQ_TYPE_BASIC == p_db_mem_if->brgif->qinq_type)
    {
        /* add vlan mapping default entry for basic qinq agg member will be process in hsrv */
    }

    if (p_db_mem_if->svlan_tpid == p_brg_global->vlan_tpid_type1)
    {
        p_brg_global->tpid_type1_ref_cnt++;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1));
    }
    else if (p_db_mem_if->svlan_tpid == p_brg_global->vlan_tpid_type2)
    {
        p_brg_global->tpid_type2_ref_cnt++;
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2));
    }

    return rc;
}

static int32
_vlan_mapping_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    int rc = PM_E_NONE;
    tbl_vlan_mapping_table_t *p_db_vmt = NULL;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    
    if (p_db_agg_if->svlan_tpid == p_brg_global->vlan_tpid_type1)
    {
        if (p_brg_global->tpid_type1_ref_cnt > 0)
        {
            p_brg_global->tpid_type1_ref_cnt--;
        }
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT));
    }
    else
    {
        if (p_brg_global->tpid_type2_ref_cnt > 0)
        {
            p_brg_global->tpid_type2_ref_cnt--;
        }
        PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT));
    }

    if (GLB_IF_MODE_L2 == p_db_agg_if->mode)
    {
        PM_E_RETURN(brg_if_set_vlan_translation_en(p_db_agg_if, FALSE, FALSE));
    }
    
    if((NULL != p_db_agg_if->brgif) && (NULL != p_db_agg_if->brgif->vlan_mapping_table))
    {
        sal_memset(&vmt_key, 0, sizeof(vmt_key));
        sal_strncpy(vmt_key.name, p_db_agg_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
        
        if(NULL != p_db_vmt)
        {
            GLB_BMP_UNSET(p_db_vmt->applied_ports, p_db_agg_if->ifindex);
            tbl_vlan_mapping_table_set_vlan_mapping_table_field(p_db_vmt,TBL_VLAN_MAPPING_TABLE_FLD_APPLIED_PORTS);
        }
    }

    return rc ;
}

int32
vlan_mapping_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_EVC_ENTRY, vlanmap_cmd_process_evc_entry);
    cdb_register_cfg_tbl_cb(TBL_VLAN_GROUP, vlanmap_cmd_process_vlan_group);
    cdb_register_cfg_tbl_cb(TBL_VLAN_MAPPING_ENTRY, vlanmap_cmd_process_vlanmap_entry);
    cdb_register_cfg_tbl_cb(TBL_EGRESS_MAPPING_ENTRY, vlanmap_cmd_process_egress_vlanmap_entry);
    cdb_register_cfg_tbl_cb(TBL_VLAN_MAPPING_TABLE, vlanmap_cmd_process_vlanmap_table);
    cdb_register_cfg_act_cb(ACT_SHOW_VLAN_MAPPING_TABLE, vlanmap_cmd_show_vmt);
    cdb_register_cfg_act_cb(ACT_SHOW_EVC_ENTRY, vlanmap_cmd_show_evc);

    if_agg_add_member_sync_prop_set_cb(IF_AGG_CB_MODULE_VLAN_MAPPING, _vlan_mapping_join_agg_update);
    if_agg_del_member_sync_prop_set_cb(IF_AGG_CB_MODULE_VLAN_MAPPING, _vlan_mapping_leave_agg_update);
    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_VLAN_MAPPING, _vlan_mapping_agg_destroy);

    return PM_E_NONE;
}

int32
vlan_mapping_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_VLAN_MAPPING_TABLE, NULL);
    cdb_register_cfg_tbl_cb(TBL_VLAN_MAPPING_ENTRY, NULL);
    cdb_register_cfg_tbl_cb(TBL_VLAN_GROUP, NULL);
    cdb_register_cfg_tbl_cb(TBL_EVC_ENTRY, NULL);
    
    return PM_E_NONE;
}


