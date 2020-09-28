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
#include "switch.h"
#include "vlan_mapping.h"
#include "glb_macro.h"
#include "if_mgr.h"


static int32
_vlanmap_cmd_add_evc_entry(tbl_evc_entry_key_t *p_evc_key, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_evc_entry_t    evc_entry;
    tbl_evc_entry_t   *p_db_evc = NULL;

    p_db_evc = tbl_evc_entry_get_evc_entry(p_evc_key);
    if (p_db_evc)
    {
        return rc;
    }

    /* check evc max num */
    if (tbl_evc_entry_get_master()->evc_entry_hash->count >= GLB_VLAN_EVC_MAX_NUM)
    {
        CFG_CONFLICT_RET("Failed to create EVC: the number of EVCs exceeds %d", GLB_VLAN_EVC_MAX_NUM);
    }

    sal_memset(&evc_entry, 0, sizeof(tbl_evc_entry_t));
    sal_strcpy(evc_entry.key.name, p_evc_key->name);
    evc_entry.mapped_svid = 0;
    evc_entry.mapped_cvid = 0;
    evc_entry.evc_type = ETHERNET_EVC_TYPE_NONE;

    PM_E_RETURN(tbl_evc_entry_add_evc_entry(&evc_entry));

    return rc;
}

static int32
_vlanmap_cmd_del_evc_entry(tbl_evc_entry_key_t *p_evc_key, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_evc_entry_t   *p_db_evc = NULL;

    p_db_evc = tbl_evc_entry_get_evc_entry(p_evc_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", p_evc_key->name);
    }
    /* if this evc is in use by mapping table entry, can't delete directly */
    if(p_db_evc->ref_count)
    {
        CFG_CONFLICT_RET("EVC %s is current used", p_evc_key->name);
    }

    PM_E_RETURN(tbl_evc_entry_del_evc_entry(p_evc_key));

    return rc;
}

static int32
_vlanmap_cmd_set_evc_mapped_vid(tbl_evc_entry_t *p_db_evc, int32 mapped_svid, 
        int32 mapped_cvid, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_evc_entry_master_t *p_master = tbl_evc_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_evc_entry_t *p_db_evc_entry = NULL;

    if (mapped_svid < GLB_VLAN_NONE || mapped_svid > GLB_VLAN_ALL 
        || mapped_svid < GLB_VLAN_NONE || mapped_svid > GLB_VLAN_ALL)
    {
        CFG_CONFLICT_RET("Mapped vlan id error");
    }

    if (mapped_svid == p_db_evc->mapped_svid && mapped_cvid == p_db_evc->mapped_cvid)
    {
        return PM_E_NONE;
    }

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    /* check reduplicate configuration of mapped vlan */
    CTCLIB_SLIST_LOOP_DEL(p_master->evc_entry_list, p_db_evc_entry, listnode, next)
    {
        if (mapped_svid == p_db_evc_entry->mapped_svid && mapped_cvid == p_db_evc_entry->mapped_cvid
            && p_db_evc_entry->mapped_svid)
        {
            CFG_CONFLICT_RET("EVC %s have configured same mapped vlan", p_db_evc_entry->key.name);
        }
    }
    /* if this evc is in use by mapping table entry, can't modify directly */
    if(p_db_evc->ref_count)
    {
        CFG_CONFLICT_RET("EVC %s is current used", p_db_evc->key.name);
    }

    if (mapped_svid != p_db_evc->mapped_svid)
    {
        p_db_evc->mapped_svid = mapped_svid;
        PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_MAPPED_SVID));
    }
    if (mapped_cvid != p_db_evc->mapped_cvid)
    {
        p_db_evc->mapped_cvid = mapped_cvid;
        PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_MAPPED_CVID));
    }

    if (!mapped_svid && !mapped_cvid)
    {
        p_db_evc->evc_type = ETHERNET_EVC_TYPE_NONE;
    }
    else if (mapped_cvid)
    {
        p_db_evc->evc_type = ETHERNET_EVC_TYPE_DOUBLE_VLAN;
    }
    else
    {
        p_db_evc->evc_type = ETHERNET_EVC_TYPE_SINGLE_SVLAN;
    }
    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_EVC_TYPE));

    return rc;
}

static int32
_vlanmap_cmd_get_ecv_entry(tbl_evc_entry_key_t *p_evc_key, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_evc_entry_key_t     key;
    tbl_evc_entry_t        *p_db_entry = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!sal_strlen(p_evc_key->name))
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_evc_entry_dump_one, &args);
    }
    else
    {
        sal_strcpy(key.name, p_evc_key->name);
        p_db_entry = tbl_evc_entry_get_evc_entry(&key);
        if (!p_db_entry)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("evc entry %s does not exist", p_evc_key->name);
        }

        tbl_evc_entry_dump_one(p_db_entry, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
vlanmap_cmd_process_evc_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    field_parse_t          field;
    int32                  i        = 0;
    tbl_evc_entry_key_t    evc_key;
    tbl_evc_entry_t       *p_db_evc  = NULL;
    int32                  mapped_svid = 0;
    int32                  mapped_cvid = 0;

    CFG_DBG_FUNC();

    sal_memset(&evc_key, 0, sizeof(tbl_evc_entry_key_t));

    p_node = cdb_get_tbl(TBL_EVC_ENTRY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        if (sal_strlen(argv[i]) > GLB_VLAN_EVC_NAME_LEN)
        {
            CFG_INVALID_PARAM_RET("Configuration evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
        }
        if (check_name_character_and_len(argv[i], GLB_VLAN_EVC_NAME_LEN) != 0)
        {
            CFG_CONFLICT_RET("The first character of evc name should be 0-9A-Za-z, character only can be 0-9A-Za-z.-_");
        }

        sal_strcpy(evc_key.name, argv[0]);

        rc = _vlanmap_cmd_add_evc_entry(&evc_key, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        if (sal_strlen(argv[i]) > GLB_VLAN_EVC_NAME_LEN)
        {
            CFG_INVALID_PARAM_RET("Configuration evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
        }
        
        sal_strcpy(evc_key.name, argv[i]);

        rc = _vlanmap_cmd_del_evc_entry(&evc_key, para);
        break;

    case CDB_OPER_SET:
        sal_strcpy(evc_key.name, argv[i]);
        p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
        if (!p_db_evc)
        {
            CFG_CONFLICT_RET("EVC not exist");
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_EVC_ENTRY_FLD_MAPPED_SVID:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            PM_CFG_GET_UINT(mapped_svid, argv[i+1], para->p_rs);
            PM_CFG_GET_UINT(mapped_cvid, argv[i+3], para->p_rs);

            rc = _vlanmap_cmd_set_evc_mapped_vid(p_db_evc, mapped_svid, mapped_cvid, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            sal_strncpy(evc_key.name, argv[0], GLB_VLAN_EVC_NAME_LEN);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vlanmap_cmd_get_ecv_entry(&evc_key, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
    
    return PM_E_NONE;
}


static int32
_vlanmap_cmd_add_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key, char *vlan_list, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_vlan_mapping_table_master_t *p_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    /* check in use */
    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_table_list, p_db_vlan_mapping_table, listnode, next)
    {
        if (p_db_vlan_mapping_table->group_id == p_vlan_group_key->group_id)
        {
            CFG_CONFLICT_RET("Group %d is used", p_vlan_group_key->group_id);
        }
    }

    PM_E_RETURN(vlanmap_api_add_vlan_group(p_vlan_group_key, vlan_list, para));

    return rc;
}

static int32
_vlanmap_cmd_del_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_vlan_group_t  *p_db_vlan_group = NULL;
    tbl_vlan_mapping_table_master_t *p_master = tbl_vlan_mapping_table_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_vlan_mapping_table_t *p_db_vlan_mapping_table = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    /* check in use */
    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_table_list, p_db_vlan_mapping_table, listnode, next)
    {
        if (p_db_vlan_mapping_table->group_id == p_vlan_group_key->group_id)
        {
            CFG_CONFLICT_RET("Group %d is used", p_vlan_group_key->group_id);
        }
    }

    p_db_vlan_group = tbl_vlan_group_get_vlan_group(p_vlan_group_key);
    if (p_db_vlan_group)
    {
        PM_E_RETURN(tbl_vlan_group_del_vlan_group(p_vlan_group_key));
    }

    return rc;
}

static int32
_vlanmap_cmd_get_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key, cfg_cmd_para_t *para, 
        cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_vlan_group_key_t    key;
    tbl_vlan_group_t       *p_db_vg = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!p_vlan_group_key->group_id)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vlan_group_dump_one, &args);
    }
    else
    {
        key.group_id = p_vlan_group_key->group_id;
        p_db_vg = tbl_vlan_group_get_vlan_group(&key);
        if (!p_db_vg)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("vlan group %d does not exist", p_vlan_group_key->group_id);
        }

        tbl_vlan_group_dump_one(p_db_vg, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}


int32
vlanmap_cmd_process_vlan_group(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    field_parse_t          field;
    int32                  value    = 0;
    char                   vlan_list[MAX_CMD_STR_LEN];
    tbl_vlan_group_key_t   vlan_group_key;

    CFG_DBG_FUNC();

    sal_memset(&vlan_group_key, 0, sizeof(tbl_vlan_group_key_t));

    p_node = cdb_get_tbl(TBL_VLAN_GROUP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        PM_CFG_GET_UINT(value, argv[0], para->p_rs);
        if (value < 0 || value > GLB_VLAN_RANGE_GROUP_NUM)
        {
            CFG_INVALID_PARAM_RET("Invalid group id %d", value);
        }
        vlan_group_key.group_id = value;
        sal_memset(vlan_list, 0, MAX_CMD_STR_LEN);
        sal_strcpy(vlan_list, argv[2]);

        rc = _vlanmap_cmd_add_vlan_group(&vlan_group_key, vlan_list, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        PM_CFG_GET_UINT(value, argv[0], para->p_rs);
        if (value < 0 || value > GLB_VLAN_RANGE_GROUP_NUM)
        {
            CFG_INVALID_PARAM_RET("Invalid group id %d", value);
        }
        vlan_group_key.group_id = value;

        rc = _vlanmap_cmd_del_vlan_group(&vlan_group_key, para);
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(value, argv[0], para->p_rs);
            if (value < 0 || value > GLB_VLAN_RANGE_GROUP_NUM)
            {
                CFG_INVALID_PARAM_RET("Invalid group id %d", value);
            }
            vlan_group_key.group_id = value;
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vlanmap_cmd_get_vlan_group(&vlan_group_key, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
    
    return PM_E_NONE;
}


static int32
_vlanmap_cmd_add_vm_entry(tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_entry_t  *p_db_vm_entry = NULL;
    tbl_evc_entry_key_t        evc_entry_key;
    tbl_evc_entry_t           *p_db_evc = NULL;
    tbl_vlan_mapping_table_key_t  vm_table_key;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;

    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
    sal_strncpy(vm_table_key.name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("Vlan mapping table %s isn't exist", p_vm_entry->key.table_name);
    }

    /* check EVC existence */
    sal_memset(&evc_entry_key, 0, sizeof(tbl_evc_entry_key_t));
    sal_strncpy(evc_entry_key.name, p_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
    p_db_evc = tbl_evc_entry_get_evc_entry(&evc_entry_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", p_vm_entry->evc_entry);
    }
    if (!p_db_evc->mapped_svid)
    {
        CFG_CONFLICT_RET("Mapped-vlan of EVC %s isn't configured", p_vm_entry->evc_entry);
    }
    p_vm_entry->mapped_svid = p_db_evc->mapped_svid;
    p_vm_entry->mapped_cvid = p_db_evc->mapped_cvid;

    /* check VME existence */
    p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&p_vm_entry->key);
    if (p_db_vm_entry)
    {
        if (!sal_strncmp(p_db_vm_entry->evc_entry, p_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN))
        {
            return PM_E_NONE;
        }
        else
        {
            CFG_VLAN_MAPPING_ENTRY_EXIST_RET(p_vm_entry->key.raw_vid);
        }
    }

    /* check raw-vid in used group range */
    if (p_db_vmt->group_id)
    {
        if (vlanmap_api_isin_vlan_group(p_db_vmt->group_id, p_vm_entry->key.raw_vid))
        {
            CFG_VLAN_MAPPING_ENTRY_EXIST_RET(p_vm_entry->key.raw_vid);
        }
    }

    PM_E_RETURN(vlanmap_api_vmt_add_vm_entry(p_db_vmt, p_vm_entry, para));

    ++p_db_evc->ref_count;
    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));

    return rc;
}

static int32
_vlanmap_cmd_del_vm_entry(tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_entry_t  *p_db_vm_entry = NULL;
    tbl_evc_entry_key_t        evc_entry_key;
    tbl_evc_entry_t           *p_db_evc = NULL;
    tbl_vlan_mapping_table_key_t  vm_table_key;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;

    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
    sal_strncpy(vm_table_key.name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("Vlan mapping table %s isn't exist", p_vm_entry->key.table_name);
    }

    p_db_vm_entry = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&p_vm_entry->key);
    if (!p_db_vm_entry)
    {
        CFG_VLAN_MAPPING_ENTRY_NOTEXIST_RET(p_vm_entry->key.raw_vid);
    }

    sal_memset(&evc_entry_key, 0, sizeof(tbl_evc_entry_key_t));
    sal_strncpy(evc_entry_key.name, p_db_vm_entry->evc_entry, GLB_VLAN_EVC_NAME_LEN);
    p_db_evc = tbl_evc_entry_get_evc_entry(&evc_entry_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", p_vm_entry->evc_entry);
    }
    
    PM_E_RETURN(vlanmap_api_vmt_del_vm_entry(p_db_vmt, p_vm_entry, para));

    --p_db_evc->ref_count;
    PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));

    return rc;
}

static int32
_vlanmap_cmd_del_evc_vm_entry(char *table_name, char *evc_name, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_vlan_mapping_entry_t  *p_db_vm_entry = NULL;
    tbl_evc_entry_key_t        evc_entry_key;
    tbl_evc_entry_t           *p_db_evc = NULL;
    tbl_vlan_mapping_table_key_t  vm_table_key;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
    sal_strncpy(vm_table_key.name, table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("Vlan mapping table %s isn't exist", table_name);
    }

    sal_memset(&evc_entry_key, 0, sizeof(tbl_evc_entry_key_t));
    sal_strncpy(evc_entry_key.name, evc_name, GLB_VLAN_EVC_NAME_LEN);
    p_db_evc = tbl_evc_entry_get_evc_entry(&evc_entry_key);
    if (!p_db_evc)
    {
        CFG_CONFLICT_RET("EVC %s isn't exist", evc_name);
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vm_entry, listnode, next)
    {
        if (!sal_strncmp(p_db_vm_entry->key.table_name, table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
         && !sal_strncmp(p_db_vm_entry->evc_entry, evc_name, GLB_VLAN_EVC_NAME_LEN))
        {
            PM_E_RETURN(vlanmap_api_vmt_del_vm_entry(p_db_vmt, p_db_vm_entry, para));

            --p_db_evc->ref_count;
            PM_E_RETURN(tbl_evc_entry_set_evc_entry_field(p_db_evc, TBL_EVC_ENTRY_FLD_REF_COUNT));
        }
    }

    if (!sal_strncmp(p_db_vmt->group_evc_entry_name, evc_name, GLB_VLAN_EVC_NAME_LEN))
    {
        PM_E_RETURN(vlanmap_api_vmt_del_vlan_group(p_db_vmt, para));
    }

    return rc;
}

static int32
_vlanmap_cmd_get_vm_entry(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vlan_mapping_entry_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
vlanmap_cmd_process_vlanmap_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    field_parse_t          field;
    vlan_bmp_t             vlan_bmp;
    vid_t                  vid = 0;
    tbl_vlan_mapping_entry_t  vm_entry;

    CFG_DBG_FUNC();

    sal_memset(&vm_entry, 0, sizeof(vm_entry));

    p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        sal_strncpy(vm_entry.key.table_name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        if (!sal_strcmp(argv[1], "raw_vid"))
        {
            if (sal_strlen(argv[4]) > GLB_VLAN_EVC_NAME_LEN)
            {
                CFG_INVALID_PARAM_RET("Configuration evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
            }
            sal_strncpy(vm_entry.evc_entry, argv[4], GLB_VLAN_EVC_NAME_LEN);

            if (!sal_strcmp(argv[2], "untagged"))
            {
                vm_entry.key.raw_vid = GLB_VLAN_NONE;
                rc = _vlanmap_cmd_add_vm_entry(&vm_entry, para);
            }
            else if (!sal_strcmp(argv[2], "out-of-range"))
            {
                vm_entry.key.raw_vid = GLB_VLAN_ALL;
                rc = _vlanmap_cmd_add_vm_entry(&vm_entry, para);
            }
            else
            {
                GLB_BMP_INIT(vlan_bmp);
                rc = cdb_bitmap_str2val(argv[2], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
                if (rc < 0)
                {
                    para->p_rs->rc = rc;
                    return rc;
                }
                GLB_BMP_ITER_BEGIN(vlan_bmp, vid)
                {
                    vm_entry.key.raw_vid = vid;
                    PM_E_RETURN(_vlanmap_cmd_add_vm_entry(&vm_entry, para));
                }
                GLB_BMP_ITER_END(vlan_bmp, vid);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid param");
        }
 
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        sal_strncpy(vm_entry.key.table_name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        if (!sal_strcmp(argv[1], "raw_vid"))
        {
            if (!sal_strcmp(argv[2], "untagged"))
            {
                vm_entry.key.raw_vid = GLB_VLAN_NONE;
                rc = _vlanmap_cmd_del_vm_entry(&vm_entry, para);
            }
            else if (!sal_strcmp(argv[2], "out-of-range"))
            {
                vm_entry.key.raw_vid = GLB_VLAN_ALL;
                rc = _vlanmap_cmd_del_vm_entry(&vm_entry, para);
            }
            else
            {
                GLB_BMP_INIT(vlan_bmp);
                rc = cdb_bitmap_str2val(argv[2], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
                if (rc < 0)
                {
                    para->p_rs->rc = rc;
                    return rc;
                }
                GLB_BMP_ITER_BEGIN(vlan_bmp, vid)
                {
                    vm_entry.key.raw_vid = vid;
                    rc = _vlanmap_cmd_del_vm_entry(&vm_entry, para);
                }
                GLB_BMP_ITER_END(vlan_bmp, vid);
            }
        }
        else if (!sal_strcmp(argv[1], "evc_entry"))
        {
            if (sal_strlen(argv[2]) > GLB_VLAN_EVC_NAME_LEN)
            {
                CFG_INVALID_PARAM_RET("Configuration evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
            }

            _vlanmap_cmd_del_evc_vm_entry(argv[0], argv[2], para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid param");
        }

        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vlanmap_cmd_get_vm_entry(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_vlanmap_cmd_get_egress_vm_entry(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_egress_mapping_entry_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_vlanmap_cmd_add_egress_vm_entry(tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_egress_mapping_entry_t   *p_db_vm_entry = NULL;
    tbl_vlan_mapping_table_key_t  vm_table_key;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;

    sal_memset(&vm_table_key, 0, sizeof(vm_table_key));
    sal_strncpy(vm_table_key.name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("Vlan mapping table %s isn't exist", p_vm_entry->key.table_name);
    }

    /* check egress vlan mapping entry existence */
    p_db_vm_entry = tbl_egress_mapping_entry_get_egress_mapping_entry(&p_vm_entry->key);
    if (p_db_vm_entry)
    {
        if (p_db_vm_entry->mapped_vid == p_vm_entry->mapped_vid)
        {
            return PM_E_NONE;
        }
        else
        {
            CFG_CONFLICT_RET("Egress vlan mapping entry exist for inner vlan %d, outer vlan %d", 
                              p_vm_entry->key.raw_cvid, p_vm_entry->key.raw_svid);
        }
    }

    /* not check mapped vlan for egress entry */
    //PM_E_RETURN(vlanmap_api_mapped_evc_check(p_db_vmt, p_vm_entry->key.raw_svid, para));

    PM_E_RETURN(vlanmap_api_vmt_add_egress_vm_entry(p_db_vmt, p_vm_entry, para));

    return rc;
}

static int32
_vlanmap_cmd_del_egress_vm_entry(tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para)
{
    int32  rc = PM_E_NONE;
    tbl_egress_mapping_entry_t *p_db_vm_entry = NULL;
    tbl_vlan_mapping_table_key_t  vm_table_key;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;

    sal_memset(&vm_table_key, 0, sizeof(vm_table_key));
    sal_strncpy(vm_table_key.name, p_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("Vlan mapping table %s isn't exist", p_vm_entry->key.table_name);
    }

    p_db_vm_entry = tbl_egress_mapping_entry_get_egress_mapping_entry(&p_vm_entry->key);
    if (!p_db_vm_entry)
    {
        CFG_CONFLICT_RET("Egress vlan mapping entry not exist for inner vlan %d, outer vlan %d", 
                              p_vm_entry->key.raw_cvid, p_vm_entry->key.raw_svid);
    }

    PM_E_RETURN(vlanmap_api_vmt_del_egress_vm_entry(p_db_vmt, p_vm_entry, para));

    return rc;
}

int32
vlanmap_cmd_process_egress_vlanmap_entry(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    field_parse_t          field;
    tbl_egress_mapping_entry_t  vm_entry;
    int32 i = 0;
    int32 raw_cvid = 0;
    int32 raw_svid = 0;
    int32 egress_mapped_vid = 0;

    CFG_DBG_FUNC();

    sal_memset(&vm_entry, 0, sizeof(vm_entry));

    p_node = cdb_get_tbl(TBL_VLAN_MAPPING_ENTRY);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        sal_strncpy(vm_entry.key.table_name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        if (!sal_strcmp(argv[1], "egress"))
        {
            for (i = 0; i < sal_strlen(argv[2]); ++i)
            {
                if (!sal_isdigit(argv[2][i]))
                {
                    CFG_INVALID_PARAM_RET("Invalid Inner vlanid value");
                }
            }

            PM_CFG_GET_UINT(raw_cvid, argv[2], para->p_rs);
            PM_CFG_GET_UINT(raw_svid, argv[3], para->p_rs);
            PM_CFG_GET_UINT(egress_mapped_vid, argv[4], para->p_rs);

            if (GLB_VLAN_NONE > raw_cvid || GLB_VLAN_MAX < raw_cvid)
            {
                CFG_INVALID_PARAM_RET("Invalid Inner vlanid value");
            }
            if (GLB_VLAN_DEFAULT_VID > raw_svid || GLB_VLAN_MAX < raw_svid)
            {
                CFG_INVALID_PARAM_RET("Invalid Outer vlanid value");
            }
            if (GLB_VLAN_NONE > egress_mapped_vid || GLB_VLAN_MAX < egress_mapped_vid)
            {
                CFG_INVALID_PARAM_RET("Invalid Mapped vlanid value");
            }

            vm_entry.key.raw_cvid = raw_cvid;
            vm_entry.key.raw_svid = raw_svid;
            vm_entry.mapped_vid = egress_mapped_vid;
            PM_E_RETURN(_vlanmap_cmd_add_egress_vm_entry(&vm_entry, para));
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid param");
        }
 
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        sal_strncpy(vm_entry.key.table_name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        if (!sal_strcmp(argv[1], "egress"))
        {
            for (i = 0; i < sal_strlen(argv[2]); ++i)
            {
                if (!sal_isdigit(argv[2][i]))
                {
                    CFG_INVALID_PARAM_RET("Invalid Inner vlanid value");
                }
            }

            PM_CFG_GET_UINT(raw_cvid, argv[2], para->p_rs);
            PM_CFG_GET_UINT(raw_svid, argv[3], para->p_rs);

            if (GLB_VLAN_NONE > raw_cvid || GLB_VLAN_MAX < raw_cvid)
            {
                CFG_INVALID_PARAM_RET("Invalid Inner vlanid value");
            }
            if (GLB_VLAN_DEFAULT_VID > raw_svid || GLB_VLAN_MAX < raw_svid)
            {
                CFG_INVALID_PARAM_RET("Invalid Outer vlanid value");
            }

            vm_entry.key.raw_cvid = raw_cvid;
            vm_entry.key.raw_svid = raw_svid;
            PM_E_RETURN(_vlanmap_cmd_del_egress_vm_entry(&vm_entry, para));
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid param");
        }

        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vlanmap_cmd_get_egress_vm_entry(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}



static int32
_vlanmap_cmd_add_vm_table(tbl_vlan_mapping_table_key_t *p_vmt_key, cfg_cmd_para_t *para)
{
    int32              rc       = PM_E_NONE;
    tbl_vlan_mapping_table_t   vm_table;
    tbl_vlan_mapping_table_t  *p_db_vmt = NULL;

    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(p_vmt_key);
    if (p_db_vmt)
    {
        return rc;
    }

    /* check maximum VMT num */
    if (tbl_vlan_mapping_table_get_master()->vlan_mapping_table_hash->count >= GLB_VLAN_MAX_VLAN_MAP_TABLE)
    {
        CFG_CONFLICT_RET("Failed to create vlan mapping table: the number of vlan mapping tables exceeds %d", GLB_VLAN_MAX_VLAN_MAP_TABLE);
    }

    sal_memset(&vm_table, 0, sizeof(tbl_vlan_mapping_table_t));
    sal_strcpy(vm_table.key.name, p_vmt_key->name);
    vm_table.group_id = 0;
    GLB_BMP_INIT(vm_table.applied_ports);

    PM_E_RETURN(tbl_vlan_mapping_table_add_vlan_mapping_table(&vm_table));

    return rc;
}

static int32
_vlanmap_cmd_del_vm_table(tbl_vlan_mapping_table_key_t *p_vmt_key, cfg_cmd_para_t *para)
{
    int32              rc         = PM_E_NONE;
    tbl_vlan_mapping_table_t *p_db_vmt = NULL;

    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(p_vmt_key);
    if (!p_db_vmt)
    {
        CFG_CONFLICT_RET("vlan mapping table %s isn't exist", p_vmt_key->name);
    }

    PM_E_RETURN(vlanmap_api_del_vm_table(p_db_vmt, para));

    return rc;
}

static int32
_vlanmap_cmd_set_vmt_group_id(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_table_t *p_vm_table, cfg_cmd_para_t *para)
{
    int32   rc  = PM_E_NONE;

    if (p_vm_table->group_id)
    {
        /* call api */
        PM_E_RETURN(vlanmap_api_vmt_add_vlan_group(p_db_vmt, p_vm_table, para));
    }
    else
    {
        if (!p_db_vmt->group_id)
        {
            CFG_CONFLICT_RET("VLAN mapping entry not exist for raw vlan group");
            return rc;
        }
        /* call api */
        PM_E_RETURN(vlanmap_api_vmt_del_vlan_group(p_db_vmt, para));
    }

    return rc;
}

static int32
_vlanmap_cmd_get_vm_table(tbl_vlan_mapping_table_key_t *p_vmt_key, 
      cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;
    tbl_vlan_mapping_table_key_t     key;
    tbl_vlan_mapping_table_t        *p_db_entry = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (!sal_strlen(p_vmt_key->name))
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_vlan_mapping_table_dump_one, &args);
    }
    else
    {
        sal_strncpy(key.name, p_vmt_key->name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
        p_db_entry = tbl_vlan_mapping_table_get_vlan_mapping_table(&key);
        if (!p_db_entry)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("vlan mapping table %s does not exist", p_vmt_key->name);
        }

        tbl_vlan_mapping_table_dump_one(p_db_entry, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
vlanmap_cmd_process_vlanmap_table(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    field_parse_t          field;
    int32                  value    = 0;
    int32                  i        = 0;
    tbl_vlan_mapping_table_key_t    vm_table_key;
    tbl_vlan_mapping_table_t       *p_db_vmt  = NULL;
    tbl_vlan_mapping_table_t        vm_table;

    CFG_DBG_FUNC();

    sal_memset(&vm_table_key, 0, sizeof(tbl_vlan_mapping_table_key_t));

    p_node = cdb_get_tbl(TBL_VLAN_MAPPING_TABLE);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        if (check_name_character_and_len(argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX) != 0)
        {
            CFG_CONFLICT_RET("The first character of table name should be 0-9A-Za-z, character only can be 0-9A-Za-z.-_");
        }
        sal_strncpy(vm_table_key.name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        rc = _vlanmap_cmd_add_vm_table(&vm_table_key, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        if (sal_strlen(argv[0]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
        {
            CFG_INVALID_PARAM_RET("Configuration table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }
        if (check_name_character_and_len(argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX) != 0)
        {
            CFG_CONFLICT_RET("The first character of table name should be 0-9A-Za-z, character only can be 0-9A-Za-z.-_");
        }
        sal_strncpy(vm_table_key.name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        rc = _vlanmap_cmd_del_vm_table(&vm_table_key, para);
        break;

    case CDB_OPER_SET:
        sal_strncpy(vm_table_key.name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);

        p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vm_table_key);
        if (!p_db_vmt)
        {
            CFG_CONFLICT_RET("VMT not exist");
        }

        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        sal_memset(&vm_table, 0, sizeof(tbl_vlan_mapping_table_t));
        switch (field_id)
        {
        case TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID:
            PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            vm_table.group_id = value;
            if (3 == argc && 0 != value)
            {
                CFG_INVALID_PARAM_RET("Invalid group param")
            }
            else if (5 == argc)
            {
                if (!sal_strlen(argv[i+3]) || sal_strlen(argv[i+3]) > GLB_VLAN_EVC_NAME_LEN)
                {
                    CFG_INVALID_PARAM_RET("Invalid group evc entry param")
                }
                sal_strncpy(vm_table.group_evc_entry_name, argv[i+3], GLB_VLAN_EVC_NAME_LEN);
            }
            
            rc = _vlanmap_cmd_set_vmt_group_id(p_db_vmt, &vm_table, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        if (1 == argc)
        {
            sal_strncpy(vm_table_key.name, argv[0], GLB_VLAN_MAPPING_TAB_NAME_MAX);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _vlanmap_cmd_get_vm_table(&vm_table_key, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
vlanmap_cmd_show_vmt(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32  rc   = PM_E_NONE;
    FILE  *fp   = NULL;
    char   evc_name[GLB_VLAN_EVC_NAME_LEN+1] = {0};
    uint32 raw_vid = 0xffff;
    tbl_vlan_mapping_table_key_t  vmt_key;
    tbl_vlan_mapping_table_t     *p_db_vmt;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (!sal_strcmp(argv[0], "table_name"))
    {
        if (1 == argc)
        {
            PM_E_RETURN(vlanmap_api_show_vmt(NULL, evc_name, raw_vid, fp, para));
        }
        else
        {
            if (sal_strlen(argv[1]) > GLB_VLAN_MAPPING_TAB_NAME_MAX)
            {
                CFG_INVALID_PARAM_RET("Vlan mapping table name should not be more than %d characters", GLB_VLAN_MAPPING_TAB_NAME_MAX);
            }

            sal_memset(&vmt_key, 0, sizeof(tbl_vlan_mapping_table_key_t));
            sal_strncpy(vmt_key.name, argv[1], GLB_VLAN_MAPPING_TAB_NAME_MAX);
            p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
            if (!p_db_vmt)
            {
                CFG_CONFLICT_RET("vlan mapping table %s does not exist", argv[1]);
            }

            if (2 == argc)
            {
                PM_E_RETURN(vlanmap_api_show_vmt(p_db_vmt, evc_name, raw_vid, fp, para));
            }
            else
            {
                if (!sal_strcmp(argv[2], "applied-interface"))
                {
                    PM_E_RETURN(vlanmap_api_show_vmt_applied_if(p_db_vmt, fp, para));
                }
                else if (!sal_strcmp(argv[2], "evc"))
                {
                    if (sal_strlen(argv[3]) > GLB_VLAN_EVC_NAME_LEN)
                    {
                        CFG_INVALID_PARAM_RET("Evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
                    }
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    sal_strncpy(evc_name, argv[3], GLB_VLAN_EVC_NAME_LEN);
                    PM_E_RETURN(vlanmap_api_show_vmt(p_db_vmt, evc_name, raw_vid, fp, para));
                }
                else if (!sal_strcmp(argv[2], "raw-vlan"))
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
                    PM_CFG_GET_UINT(raw_vid, argv[3], para->p_rs);
                    if (raw_vid < GLB_VLAN_NONE || raw_vid > GLB_VLAN_ALL)
                    {
                        CFG_CONFLICT_RET("invalid raw vlan id %d", raw_vid);
                    }
                    PM_E_RETURN(vlanmap_api_show_vmt(p_db_vmt, evc_name, raw_vid, fp, para));
                }
            }
        }
    }
    else if (!sal_strcmp(argv[0], "applied-interface"))
    {
        PM_E_RETURN(vlanmap_api_show_vmt_applied_if(NULL, fp, para));
    }
    else
    {
        CFG_CONFLICT_RET("Wrong vm table show mode");
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

int32
vlanmap_cmd_show_evc(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32         rc        = PM_E_NONE;
    FILE         *fp        = NULL;
    tbl_evc_entry_master_t *p_evc_master = tbl_evc_entry_get_master();
    ctclib_slistnode_t     *listnode = NULL;
    ctclib_slistnode_t     *next = NULL;
    tbl_evc_entry_key_t     evc_key;
    tbl_evc_entry_t        *p_db_evc = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP_DEL(p_evc_master->evc_entry_list, p_db_evc, listnode, next)
        {
            vlanmap_api_show_evc_one(p_db_evc, fp, para);
        }
    }
    else if (1 == argc)
    {
        if (sal_strlen(argv[0]) > GLB_VLAN_EVC_NAME_LEN)
        {
            CFG_INVALID_PARAM_RET("Configuration evc name should not be more than %d characters", GLB_VLAN_EVC_NAME_LEN);
        }

        sal_memset(&evc_key, 0, sizeof(evc_key));
        sal_strncpy(evc_key.name, argv[0], GLB_VLAN_EVC_NAME_LEN);
        p_db_evc = tbl_evc_entry_get_evc_entry(&evc_key);
        if (!p_db_evc)
        {
            CFG_CONFLICT_RET("EVC %s not exist", argv[0]);
        }
        vlanmap_api_show_evc_one(p_db_evc, fp, para);
    }
    else
    {
        CFG_INVALID_PARAM_RET("Wrong param");
    }

    sal_fclose(fp);
    fp = NULL;
    return rc;
}

