
#include "proto.h"

#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"

#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_maid_define.h"
#include "gen/tbl_cfm_maid.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "cfmd.h"
#include "cfm_api.h"
#include "g8032.h"

/* modified by liwh for bug 53818, 2019-09-29 */
uint32
cfm_profile_is_default_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_DEFAULT == type) 
    {
        return TRUE;
    }

    return FALSE;
}
/* liwh end */

int32
_cfm_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node, tbl_cfm_global_t *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;

    sal_memset(&args, 0 ,sizeof(args));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_cfm_global_dump_one(p_db_glb, &args);
            
    sal_fclose(fp);
    
    return PM_E_NONE;
}

int32
cfm_cmd_process_cfm_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_cfm_global_t cfm_glb;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_cfm_global_t *p_db_glb = NULL;
    int32 rc = PM_E_NONE;
    tbl_cfm_md_master_t   *p_db_md_glb = NULL;

    CFG_DBG_FUNC();

    /* modified by liwh for bug 53818, 2019-09-29 */
    if (!cfm_profile_is_default_profile())
    {
        CFG_CONFLICT_RET("No cfm resource, please change profile");
    }
    /* liwh end */
    
    p_node = cdb_get_tbl(TBL_CFM_GLOBAL);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_GLOBAL not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_global_get_cfm_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_GLOBAL not found");
    }

    p_db_md_glb = tbl_cfm_md_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_MD not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&cfm_glb, 0, sizeof(cfm_glb));

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        
        switch (field_id)
        {
        case TBL_CFM_GLOBAL_FLD_CFM_MODE:
            if (p_db_md_glb->cfm_md_list->count)
            {
                CFG_INVALID_PARAM_RET("Should delete all MDs at first");    
            }

            if (p_db_glb->cfm_mode == value)
            {
                return rc;    
            }
            
            cfm_glb.cfm_mode = value;
            rc = tbl_cfm_global_set_cfm_global_field(&cfm_glb, field_id);
            break;
            
        case TBL_CFM_GLOBAL_FLD_CFM_ENABLE:
            if (p_db_glb->cfm_enable == value)
            {
                return rc;    
            }
            
            cfm_glb.cfm_enable = value;
            rc = tbl_cfm_global_set_cfm_global_field(&cfm_glb, field_id);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_global(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_cfm_cmd_add_cfm_md(tbl_cfm_md_master_t   *p_db_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint16                 level = 0;
    uint8                   name_type = GLB_CFM_MD_CHAR_STRING;
    int32                   rc = PM_E_NONE;
    tbl_cfm_md_t       md;
    tbl_cfm_md_t       *p_md;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    /* modified by liwh for bug 53818, 2019-09-29 */
    if (!cfm_profile_is_default_profile())
    {
        CFG_CONFLICT_RET("No cfm resource, please change profile");
    }
    /* liwh end */

    /* modified by liwh for bug 54222, 2019-11-02 */
    /*if(!sal_char_isalpha(*argv[0]))
    {
        CFG_FUNC_ERR_RET("The first character of MD name should be a-z or A-Z");        
    }*/
    /* liwh end */

    /* check md length */
    if ((sal_strlen(argv[0]) < 1)
        || (sal_strlen(argv[0]) > (GLB_CFM_MD_MA_NAME_LENGTH_8021AG - 1)))
    {
        CFG_INVALID_PARAM_RET("MD String length should not exceed 43 characters");
    }

    /* if md is exist, enter md configure mode */
    sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (p_md)
    {   
        if (1 == argc)
        {
            return rc;
        }
        else if (2 == argc)
        {
            PM_CFG_GET_UINT(level, argv[1], para->p_rs);
            if (level == p_md->level)
            {
                return rc;    
            }
        }

        CFG_FUNC_ERR_RET("MD is exist already");    
    }

    /* check md parameters */
    rc = cfm_md_check_valid(argv, argc, para, &level, &name_type, &md);
    if (rc)
    {
        return rc;    
    }

    /* check cfm md number */
    if (p_db_glb->cfm_md_list->count >= GLB_CFM_MD_MAX)
    {
        CFG_FUNC_ERR_RET("Can't add CFM domain: reach max number %d", GLB_CFM_MD_MAX);
    }

    /* reserver md parameters */
    md.level = level;
    md.name_type = name_type;

    /* add md to database */
    rc = tbl_cfm_md_add_cfm_md(&md);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM domain fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

static int32
_cfm_cmd_get_md(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t* p_node, tbl_cfm_md_master_t   *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;
    tbl_cfm_md_t             *p_md = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_cfm_md_t             md;
    int32                         rc = PM_E_NONE;

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_md_list, p_md, listnode)
        {
            tbl_cfm_md_dump_one(p_md, &args);
        }
    }
    else
    {
        sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
        p_md = tbl_cfm_md_get_cfm_md(&md.key);
        if (p_md)
        {
            tbl_cfm_md_dump_one(p_md, &args);
        }
    }
            
    sal_fclose(fp);
    
    return rc;
}

static int32
_cfm_cmd_del_cfm_md(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                   rc = PM_E_NONE;
    tbl_cfm_md_t       md;
    tbl_cfm_md_t       *p_md;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));    
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_INVALID_PARAM_RET("MD not found in the list");
    }

    /* delete all ma for md */
    cfm_api_delete_ma_for_md(p_md);

    /* delete all errors for ma */
    cfm_api_clear_cfm_errors_for_md(p_md);
    
    rc = tbl_cfm_md_del_cfm_md(&md.key);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM domain fail, %s", pm_err_desc(rc));
    }
    
    return rc;
}

int32
cfm_cmd_process_cfm_md(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                  *p_node = NULL;
    tbl_cfm_md_master_t   *p_db_glb = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CFM_MD);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_MD not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_md_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_MD not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _cfm_cmd_add_cfm_md(p_db_glb, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        rc = _cfm_cmd_del_cfm_md(argv, argc, para);
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_md(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_cfm_cmd_add_cfm_ma(tbl_cfm_ma_master_t   *p_db_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint16                 vlan_id = 0;
    uint8                   name_type = GLB_CFM_MA_CHAR_STRING;
    int32                   rc = PM_E_NONE;
    tbl_cfm_md_t       md;
    tbl_cfm_md_t       *p_md = NULL;
    tbl_cfm_ma_t       ma;
    tbl_cfm_ma_t       *p_ma = NULL;
    uint8                   index = 0;
    char                    maid_key[CFM_MAID_MAX_LENGTH];
    uint8                   md_length = 0;
    uint8                   ma_length = 0;
    tbl_cfm_maid_t    maid;
    tbl_cfm_maid_t    *p_maid = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    uint8                   ma_index = 0;
    bool                     ma_exist = FALSE;
    tbl_vlan_key_t     vlan_key;
    tbl_vlan_t            *p_vlan = NULL;

    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&maid_key, 0, CFM_MAID_MAX_LENGTH);
    sal_memset(&maid, 0, sizeof(tbl_cfm_maid_t));
    sal_memset(&vlan_key, 0, sizeof(tbl_vlan_key_t));

    
    /* check cfm md number */
    if (p_db_glb->cfm_ma_list->count >= GLB_CFM_MA_MAX)
    {
        CFG_FUNC_ERR_RET("Can't add CFM service: reach max number %d", GLB_CFM_MA_MAX);
    }

    sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD isn't exist"); 
    }

    rc = cfm_ma_check_valid(argv, argc, para, p_md, &vlan_id, &name_type, &index);
    if (rc)
    {
        return rc;
    }

    /* modified by liwh for bug 54222, 2019-11-02 */
    /* modified by liwh for bug 53670, 2019-09-21 */
    /*if(!sal_char_isalpha(*argv[1]))
    {
        CFG_FUNC_ERR_RET("The first character of MA name should be a-z or A-Z");        
    }*/
    /* liwh end */

    /* check ma exist by key */
    sal_memcpy(ma.key.name, argv[0], sal_strlen(argv[0]));
    sal_memcpy(ma.name, argv[1], sal_strlen(argv[1]));
    ma.key.vlan_id = vlan_id;
    ma.name_type = name_type;
    ma.ccm_vlan_cos = GLB_CFM_PRIO_MAX;
    if ((index >= 4) && (index < argc))
    {
        sal_memcpy(ma.pkt_name, argv[index], sal_strlen(argv[index]));
    }
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (p_ma)
    {   
        CFG_FUNC_ERR_RET("MA exists already"); 
    }

    /* check ma exist by ma name in same domain */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_ma_list, p_ma, listnode)
    {
        if ((0 == sal_strcmp((char*)p_ma->key.name, (char*)ma.key.name))
            && (0 == sal_strcmp(p_ma->name, ma.name)))
        {
            ma_exist = TRUE;
            break;    
        }
    }

    if (ma_exist)
    {
         CFG_FUNC_ERR_RET("MA exists already");    
    }

    /* check level not 0 and vlan_id is 0 */
    if ((0 != p_md->level) && (0 == vlan_id))
    {
        CFG_FUNC_ERR_RET("Link level MA can exist only on level 0 domain");
    }

    /* check level 0 and vlan_id is not 0 */
    if ((0 == p_md->level) && (0 != vlan_id))
    {
        CFG_FUNC_ERR_RET("Link level MA can not been associated with vlan");
    }

    /* check vlan is exist */
    if (vlan_id)
    {
        vlan_key.vid = vlan_id;
        p_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (!p_vlan)
        {
            CFG_FUNC_ERR_RET("VLAN not found on the bridge");
        }
    }

    /* check maid valid */
    maid_key[0] = p_md->name_type;
    if (0 == p_md->pkt_name_length)
    {
        if (GLB_CFM_NO_MD_NAME != p_md->name_type)
        {
            md_length = sal_strlen(p_md->key.name);
            maid_key[1] = md_length;
            sal_memcpy(&maid_key[2], p_md->key.name, md_length);
            ma_index = md_length + 2;
        }
        else
        {
            ma_index = 1;    
        }
    }
    else
    {
        md_length = p_md->pkt_name_length;
        maid_key[1] = md_length;
        sal_memcpy(&maid_key[2], p_md->pkt_name, md_length);
        ma_index = md_length + 2;
    }

    maid_key[ma_index] = ma.name_type;
    if (0 == sal_strlen(ma.pkt_name))
    {
        ma_length = sal_strlen((char*)ma.name); 
        if (md_length + ma_length > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
        {
            CFG_INVALID_PARAM_RET("MAID String length should not exceed %d characters\n", 
                GLB_CFM_MD_MA_NAME_LENGTH_8021AG);        
        }
        
        maid_key[ma_index + 1] = ma_length;
        
        sal_memcpy(&maid_key[ma_index + 2], ma.name, ma_length);
    }
    else
    {
        ma_length = sal_strlen((char*)ma.pkt_name); 
        if (md_length + ma_length > GLB_CFM_MD_MA_NAME_LENGTH_8021AG)
        {
            CFG_INVALID_PARAM_RET("MAID String length should not exceed %d characters\n", 
                GLB_CFM_MD_MA_NAME_LENGTH_8021AG);   
        }

        if (GLB_CFM_MA_ICC_BASED_FORMAT == ma.name_type)
        {
            maid_key[ma_index + 1] = GLB_CFM_MD_MA_NAME_LENGTH_Y1731;
        }
        else
        {
            maid_key[ma_index + 1] = ma_length;
        }
        
        sal_memcpy(&maid_key[ma_index + 2], ma.pkt_name, ma_length);
    }

    /* check different maid */
    sal_memcpy(maid.key.name, maid_key, CFM_MAID_MAX_LENGTH); 
    p_maid = tbl_cfm_maid_get_cfm_maid(&maid.key);
    if (p_maid)
    {
        CFG_FUNC_ERR_RET("Maid(md+ma) exists already");
    }

    /* add ma */
    sal_memcpy(ma.maid, maid_key, CFM_MAID_MAX_LENGTH);
    rc = tbl_cfm_ma_add_cfm_ma(&ma);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM service fail, %s", pm_err_desc(rc));
    }

    /* add maid */
    rc = tbl_cfm_maid_add_cfm_maid(&maid);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM maid fail, %s", pm_err_desc(rc));
    }

    /* count ma number for md */
    p_md->service_number++;

    return rc;
}

static int32
_cfm_cmd_del_cfm_ma(tbl_cfm_ma_master_t   *p_db_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_cfm_ma_t       *p_ma = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    int32                    rc = PM_E_NONE;
    tbl_cfm_maid_t     maid;
    tbl_cfm_md_t        md;
    tbl_cfm_md_t        *p_md = NULL;

    sal_memset(&maid, 0, sizeof(tbl_cfm_maid_t));
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD isn't exist"); 
    }
    
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_ma_list, p_ma, listnode)
    {
        if ((0 == sal_strcmp((char*)p_ma->key.name, argv[0]))
            && (0 == sal_strcmp(p_ma->name, argv[1])))
        {
            break;    
        }
    }

    if (!p_ma)
    {
        CFG_INVALID_PARAM_RET("MA not found in the list");
    }

    /* delete all meps for ma */
    cfm_api_delete_meps_for_ma(p_ma);

    /* count ma number for md */
    p_md->service_number--;

    /* del maid */
    sal_memcpy(&maid.key.name, p_ma->maid, CFM_MAID_MAX_LENGTH);
    tbl_cfm_maid_del_cfm_maid(&maid.key);

    /* del ma */
    tbl_cfm_ma_del_cfm_ma(&p_ma->key);
    
    return rc;
}

int32
cfm_cmd_parse_ma_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
{
    cdb_tbl_info_t    *p_tbl_info = p_node->tbl_info;
    uint32            field_id = FLD_ID_ALL;
    uint32            key_id = 0;

    if (with_key)
    {
        key_id = 2;
    }
    
    sal_memset(field, FLD_ID_ALL, sizeof(field_parse_t));
    if (key_id >= argc)
    {
        return PM_E_NONE;
    }
    
    for (field_id = 0; field_id < p_tbl_info->field_num; field_id++)
    {
        if (!sal_strcmp(argv[key_id], p_tbl_info->field[field_id].name))
        {
            field->field_id[0] = field_id;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}

static int32
_cfm_cmd_get_ma(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t* p_node, tbl_cfm_ma_master_t   *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;
    tbl_cfm_ma_t             *p_ma = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_cfm_ma_t             ma;
    int32                         rc = PM_E_NONE;
    uint16                       vlan_id = 0;

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfm_cmd_parse_ma_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_ma_list, p_ma, listnode)
        {
            tbl_cfm_ma_dump_one(p_ma, &args);
        }
    }
    else
    {
        sal_memcpy(ma.key.name, argv[0], sal_strlen(argv[0]));
        /* get vlan */
        if (argc > 1)
        {
            PM_CFG_GET_UINT(vlan_id, argv[1], para->p_rs);
            ma.key.vlan_id = vlan_id;
        }
    
        p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
        if (p_ma)
        {
            tbl_cfm_ma_dump_one(p_ma, &args);
        }
    }
            
    sal_fclose(fp);
    
    return rc;
}

int32
cfm_cmd_process_cfm_ma(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    cdb_node_t                  *p_node = NULL;
    tbl_cfm_ma_master_t   *p_db_glb = NULL;
    tbl_cfm_ma_t               ma;
    uint8                            index = 1;
    uint16                          vlan_id = 0;
    tbl_cfm_ma_t               *p_ma = NULL;
    tbl_cfm_md_t               md;
    tbl_cfm_md_t               *p_md = NULL;
    int32                            field_id = 0;
    cdb_field_t                    *p_field = NULL;
    uint8                             cci_enabled = 0;
    uint8                             ccm_vlan_cos = 0;
    uint8                             error_write_disable = 0;
    
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CFM_MA);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_MA not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_ma_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_MA not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _cfm_cmd_add_cfm_ma(p_db_glb, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        rc = _cfm_cmd_del_cfm_ma(p_db_glb, argv, argc, para);
        break;
        
    case CDB_OPER_SET:
        sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
        sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));

        /* get vlan */
        if (argc > index)
        {
            if (0 == sal_strcmp(argv[index], "vlan"))
            {
                PM_CFG_GET_UINT(vlan_id, argv[index+1], para->p_rs);
                index = index + 2;
            }
        }

        /* check md not exist */
        sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
        p_md = tbl_cfm_md_get_cfm_md(&md.key);
        if (!p_md)
        {   
            CFG_FUNC_ERR_RET("MD not found in the list"); 
        }

        /* check ma not exist */
        sal_memcpy(ma.key.name, argv[0], sal_strlen(argv[0]));
        ma.key.vlan_id = vlan_id;
        p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
        if (!p_ma)
        {
             CFG_FUNC_ERR_RET("MA not found in the list");   
        }

        field_id = cdb_get_field_id(argv[index], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[index]);
        }

        switch (field_id)
        {
        case TBL_CFM_MA_FLD_CCI_ENABLED:
            PM_CFG_GET_UINT(cci_enabled, argv[index+1], para->p_rs);
  
            rc = cfm_set_service_cci_enable(p_ma, cci_enabled);
            break;

        case TBL_CFM_MA_FLD_CCM_VLAN_COS:
            PM_CFG_GET_UINT(ccm_vlan_cos, argv[index+1], para->p_rs);

            if (0 == p_ma->key.vlan_id)
            {
                CFG_FUNC_ERR_RET("Can't set vlan priority because no associated vlan for link oam domain MA");
            }
            
            rc= cfm_set_service_ccm_vlan_cos(p_ma, ccm_vlan_cos);
            break;

        case TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE:
            PM_CFG_GET_UINT(error_write_disable, argv[index+1], para->p_rs);
           
            p_ma->error_write_disable = error_write_disable;
            rc = tbl_cfm_ma_set_cfm_ma_field(p_ma, TBL_CFM_MA_FLD_ERROR_WRITE_DISABLE);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[index]);
            break;
        }

        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_ma(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_cfm_cmd_get_maid(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t* p_node, tbl_cfm_maid_master_t   *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;
    tbl_cfm_maid_t          *p_maid = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_cfm_maid_t          maid;
    int32                         rc = PM_E_NONE;

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&maid, 0, sizeof(tbl_cfm_maid_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfm_cmd_parse_ma_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_maid_list, p_maid, listnode)
        {
            tbl_cfm_maid_dump_one(p_maid, &args);
        }
    }
    else
    {
        sal_memcpy(maid.key.name, argv[0], sal_strlen(argv[0]));
        p_maid = tbl_cfm_maid_get_cfm_maid(&maid.key);
        if (p_maid)
        {
            tbl_cfm_maid_dump_one(p_maid, &args);
        }
    }
            
    sal_fclose(fp);
    
    return rc;
}

int32
cfm_cmd_process_cfm_maid(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                              rc = PM_E_NONE;
    cdb_node_t                     *p_node = NULL;
    tbl_cfm_maid_master_t   *p_db_glb = NULL;
      
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CFM_MAID);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_MAID not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_maid_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_MAID not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_maid(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
cfm_cmd_show_cfm_domain(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_md_t               md;
    tbl_cfm_md_t               *p_md = NULL;
    FILE                             *fp = NULL;
    tbl_cfm_ma_master_t   *p_db_glb = NULL;
    tbl_cfm_ma_t               *p_ma = NULL;
    ctclib_slistnode_t          *listnode = NULL;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {
        CFG_INVALID_PARAM_RET("MD not found in the list");    
    }

    p_db_glb = tbl_cfm_ma_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_MA not found");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, " Domain Name     : %s\n", p_md->key.name);
    sal_fprintf(fp, " Level           : %d\n", p_md->level);
    sal_fprintf(fp, " Total Services  : %d\n", p_md->service_number);
    sal_fprintf(fp, " Vlan        ServiceID\n");
    sal_fprintf(fp, "--------------------------------------------------------\n");

    /* check ma exist by ma name in same domain */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_ma_list, p_ma, listnode)
    {
        if (0 == sal_strcmp((char*)p_ma->key.name, md.key.name))
        {
            if (p_ma->key.vlan_id)
            {
                sal_fprintf(fp, " %4d%-8s%-16s\n", p_ma->key.vlan_id, "", p_ma->name);
            }
            else
            {
                sal_fprintf(fp, " %2s%-10s%-16s\n", "-", "", p_ma->name);
            }
        }
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
cfm_cmd_show_cfm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                              rc = PM_E_NONE;
    FILE                               *fp = NULL;
     tbl_cfm_global_t            *p_db_glb = tbl_cfm_global_get_cfm_global();
    tbl_cfm_md_master_t      *p_md_master = tbl_cfm_md_get_master();
    tbl_cfm_ma_master_t      *p_ma_master = tbl_cfm_ma_get_master();
    tbl_cfm_lmep_master_t   *p_lmep_master = tbl_cfm_lmep_get_master();
    tbl_cfm_rmep_master_t   *p_rmep_master = tbl_cfm_rmep_get_master();
    
    if (!p_db_glb || !p_md_master || !p_ma_master || !p_lmep_master || !p_rmep_master)
    {
        CFG_FUNC_ERR_RET("Can't get global information");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "-------------  Global Configure  --------------------\n");
    if (p_db_glb->cfm_enable)
    {
        sal_fprintf(fp, "CFM enable                          : Enabled\n");
    }
    else
    {
        sal_fprintf(fp, "CFM enable                          : Disabled\n");
    }

    if (GLB_CFM_MODE_Y1731 == p_db_glb->cfm_mode)
    {
        sal_fprintf(fp, "CFM mode                            : Y1731\n");    
    }
    else
    {
        sal_fprintf(fp, "CFM mode                            : 802.1ag\n");
    }

    sal_fprintf(fp, "ethernet cfm md size                : %d\n", GLB_CFM_MD_MAX);
    sal_fprintf(fp, "ethernet cfm md count               : %d\n", p_md_master->cfm_md_list->count);

    sal_fprintf(fp, "ethernet cfm maid size              : %d\n", GLB_CFM_MA_MAX);
    sal_fprintf(fp, "ethernet cfm maid count             : %d\n", p_ma_master->cfm_ma_list->count);

    sal_fprintf(fp, "ethernet cfm local&remote mep size  : %d\n", GLB_CFM_LMEP_RMEP_MAX);

    sal_fprintf(fp, "ethernet cfm local mep count        : %d\n", p_lmep_master->cfm_lmep_list->count);
    sal_fprintf(fp, "ethernet cfm remote mep count       : %d\n", p_rmep_master->cfm_rmep_list->count);

    sal_fclose(fp);
    fp = NULL;

    return rc;
}

int32
cfm_cmd_show_cfm_cc_config(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_md_master_t  *g_p_master = tbl_cfm_md_get_master();
    tbl_cfm_md_t              *p_md = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    tbl_cfm_ma_master_t  *g_p_master_ma = tbl_cfm_ma_get_master();
    tbl_cfm_ma_t              *p_ma = NULL;
    ctclib_slistnode_t         *listnode_ma = NULL;
    FILE                            *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-43s %-5s %-4s %-10s\n", "Domain Name", "Level", "Vlan", "CC Status");
    sal_fprintf(fp, "------------------------------------------------------------------\n");

    CTCLIB_SLIST_LOOP(g_p_master->cfm_md_list, p_md, listnode)
    {
        CTCLIB_SLIST_LOOP(g_p_master_ma->cfm_ma_list, p_ma, listnode_ma)
        {
            if (0 == sal_strcmp((char*)p_ma->key.name, p_md->key.name))
            {
                if (p_ma->key.vlan_id)
                {
                    sal_fprintf(fp, "%-43s %-5d %-4d %-10s\n", p_ma->key.name, p_md->level, p_ma->key.vlan_id, p_ma->cci_enabled ? "Enable":"Disable");
                }
                else
                {
                    sal_fprintf(fp, "%-43s %-5d %-4s %-10s\n", p_ma->key.name, p_md->level, "-", p_ma->cci_enabled ? "Enable":"Disable");    
                }
            }
        }
    }

    sal_fclose(fp);
    fp = NULL;

    return rc;
}

char* cfm_get_defect_name(uint8 error_reason)
{    
    switch(error_reason)
    {
        case GLB_CFM_RMEP_SRC_MAC_MISMATCH:
            return "Rmep src-mac mismatch";
        case GLB_CFM_SOME_RDI_DEFECT:
            return "Rdi defect";
        case GLB_CFM_SOME_RDI_DEFECT_CLEAR:
            return "Rdi defect clear";    
        case GLB_CFM_SOME_MAC_STATUS_DEFECT:
            return "SomeMACstatusDefect";
        case GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC:
            return "Dloc defect"; 
        case GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC_CLEAR:
            return "Dloc defect clear";
        case GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND:
            return "Rmep not found";
        case GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND_CLEAR:
            return "Rmep not found clear";
        case GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH:
            return "Ccm interval mismatch";                                               
        case GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH_CLEAR:
            return "Ccm interval mismatch clear";
        case GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH:
            return "Maid mismatch";
        case GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH_CLEAR:
            return "Maid mismatch clear";
        case GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM:
            return "Low ccm";
        case GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM_CLEAR:
            return "Low ccm clear"; 
        case GLB_CFM_AIS_DEFECT_AIS_RECEIVE:
            return "Receive AIS"; 
        case GLB_CFM_AIS_DEFECT_AIS_RECEIVE_CLEAR:
            return "No receive AIS in 3.5 period"; 
        case GLB_CFM_RMEP_FIRST_PACKET_RECEIVE:
            return "Receive first packet";
        default:
            return "Unknown reason"; 
    }
    
    return "Unknown reason";    
}

int32
cfm_cmd_show_cfm_errors(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_md_t               md;
    tbl_cfm_md_t               *p_md;
    FILE                            *fp = NULL;
    ds_cfm_error_t            *p_db_cfm_error = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    struct sal_tm               *pst_time = NULL;
    char                               lmep_md_name[GLB_CFM_MD_MA_NAME_LENGTH_Y1731] = {0};
    
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    sal_memcpy(md.key.name, argv[1], sal_strlen(argv[1]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {
        CFG_INVALID_PARAM_RET("MD not found in the list");    
    }

    sal_memcpy(lmep_md_name, p_md->key.name, 6);

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "Domain Vlan MPID RemoteMac      Reason%-23sTime\n", " ");

    CTCLIB_SLIST_LOOP(p_md->cfm_error.obj_list, p_db_cfm_error, listnode)
    {
        pst_time = sal_localtime((sal_time_t *)&p_db_cfm_error->key.time);

        if (p_db_cfm_error->vlan_id)
        {
            sal_fprintf(fp, "%-7s%-5d%-5d%02x%02x.%02x%02x.%02x%02x %-29s%d/%02d/%02d %02d:%02d:%02d\n", 
                lmep_md_name, p_db_cfm_error->vlan_id, p_db_cfm_error->mep_id, p_db_cfm_error->rmep_mac[0], 
                p_db_cfm_error->rmep_mac[1], p_db_cfm_error->rmep_mac[2], p_db_cfm_error->rmep_mac[3],
                p_db_cfm_error->rmep_mac[4], p_db_cfm_error->rmep_mac[5], 
                cfm_get_defect_name(p_db_cfm_error->err_reason),          
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);    
        }
        else
        {
            sal_fprintf(fp, "%-7s%-5s%-5d%02x%02x.%02x%02x.%02x%02x %-29s%d/%02d/%02d %02d:%02d:%02d\n", 
                lmep_md_name, "-", p_db_cfm_error->mep_id, p_db_cfm_error->rmep_mac[0], 
                p_db_cfm_error->rmep_mac[1], p_db_cfm_error->rmep_mac[2], p_db_cfm_error->rmep_mac[3],
                p_db_cfm_error->rmep_mac[4], p_db_cfm_error->rmep_mac[5], 
                cfm_get_defect_name(p_db_cfm_error->err_reason),          
                pst_time->tm_year + 1900, 
                pst_time->tm_mon + 1, 
                pst_time->tm_mday, 
                pst_time->tm_hour, 
                pst_time->tm_min, 
                pst_time->tm_sec);                
        }
    }

    sal_fclose(fp);
    fp = NULL;

    return rc;
}

int32
cfm_cmd_clear_cfm_errors(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_md_t               md;
    tbl_cfm_md_t               *p_md;
    
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));

    sal_memcpy(md.key.name, argv[1], sal_strlen(argv[1]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {
        CFG_INVALID_PARAM_RET("MD not found in the list");    
    }

    cfm_api_clear_cfm_errors_for_md(p_md);

    return rc;
}

static int32
_cfm_check_interface_for_mep(tbl_interface_t   *p_db_if, cfg_cmd_para_t *para)
{
    int32                         rc = PM_E_NONE;

    /* check agg interface */
    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("CFM can't be supported on agg port");    
    }

    /* check agg member */
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        CFG_CONFLICT_RET("CFM can't be supported on agg member port");        
    }

    /* check l3 port */
    if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
         CFG_CONFLICT_RET("CFM can't be supported on routed interface");   
    }

    /* check qinq port */
    if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
    {
         CFG_CONFLICT_RET("CFM can't be supported on QinQ port");   
    }

    /* check vlan class is conflict with cfm */
    if (p_db_if->vlanclass_active_type)
    {
        CFG_CONFLICT_RET("CFM is conflict with vlan class");
    }

    /* check vlan translation is conflict with cfm */
    if (p_db_if->brgif && p_db_if->brgif->vlan_translation_en)
    {
        CFG_CONFLICT_RET("CFM is conflict with vlan translation");
    }

    return rc;
}

int32
cfm_check_lmep_rmep_number(tbl_cfm_lmep_master_t   *p_db_glb_lmep, tbl_cfm_rmep_master_t   *p_db_glb_rmep, cfg_cmd_para_t *para)
{
    int32                              rc = PM_E_NONE;
    tbl_cfm_lmep_master_t   *p_glb_lmep = NULL;
    tbl_cfm_rmep_master_t   *p_glb_rmep = NULL; 

    if (p_db_glb_lmep)
    {
        p_glb_lmep = p_db_glb_lmep;
    }
    else
    {
        p_glb_lmep = tbl_cfm_lmep_get_master();
        if (!p_glb_lmep)
        {
            CFG_CONFLICT_RET("TBL_CFM_LMEP not found");
        }
    }

    if (p_db_glb_rmep)
    {
        p_glb_rmep = p_db_glb_rmep;
    }
    else
    {
        p_glb_rmep = tbl_cfm_rmep_get_master();
        if (!p_glb_rmep)
        {
            CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
        }
    }

    if (p_glb_lmep->cfm_lmep_list->count +  p_glb_rmep->cfm_rmep_list->count >= GLB_CFM_LMEP_RMEP_MAX)
    {
        CFG_CONFLICT_RET("Can't add new mep/rmep because there is no resource");
    }
    
    return rc;
}

static int32
_cfm_cmd_add_cfm_lmep(tbl_cfm_lmep_master_t   *p_db_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                         rc = PM_E_NONE;
    tbl_cfm_md_t             md;
    tbl_cfm_md_t             *p_md = NULL;
    tbl_cfm_ma_t             ma;
    tbl_cfm_ma_t             *p_ma = NULL;
    tbl_cfm_lmep_t           lmep;
    tbl_cfm_lmep_t           *p_lmep = NULL;
    uint16                         vlan_id = 0;
    uint8                           interval = 0;
    uint16                         mep_id = 0;
    tbl_interface_key_t      if_key;
    tbl_interface_t             *p_db_if = NULL;
    uint8                           direction = 0;
    tbl_cfm_rmep_key_t    rmep_key;
    tbl_cfm_rmep_t           *p_rmep = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    uint8                               port_level_num = 0;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));
    sal_memset(&lmep, 0, sizeof(tbl_cfm_lmep_t));
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
    sal_memset(&rmep_key, 0, sizeof(tbl_cfm_rmep_key_t));

    p_db_glb_rmep = tbl_cfm_rmep_get_master();
    if (!p_db_glb_rmep)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }

    rc = cfm_lmep_check_valid(argv, argc, para, &vlan_id, &interval, &direction, &mep_id);
    if (rc)
    {
        return rc;    
    }

    /* lookup interface exist */
    sal_memcpy(if_key.name, argv[1], sal_strlen(argv[1]));    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist!");
    }

    /* check whether local mep or remote mep can be configure on this interface */
    rc = _cfm_check_interface_for_mep(p_db_if, para);
    if (rc)
    {
        return rc;    
    }

    /* check md exist */
    sal_memcpy(md.key.name, argv[3], sal_strlen(argv[3]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD not found in the list"); 
    }

    if ((GLB_CFM_LEVEL_MIN == p_md->level) && (GLB_CFM_UP_MEP == direction))
    {
         CFG_FUNC_ERR_RET("Cannot configure Link Level MEP as UP MEP");    
    }

    /* check only link oam can be configured on access port */
    if ((GLB_CFM_LEVEL_MIN != p_md->level) &&((GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)))
    {
        CFG_FUNC_ERR_RET("Only link oam can be configured on access");    
    }

    /* check ma exist by key */
    sal_memcpy(ma.key.name, argv[3], sal_strlen(argv[3]));
    ma.key.vlan_id = vlan_id;
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (!p_ma)
    {   
        CFG_FUNC_ERR_RET("MA not found in the list"); 
    }
    
    /* check cfm lmep&rmep number */
    rc = cfm_check_lmep_rmep_number(p_db_glb, p_db_glb_rmep, para);
    if (rc)
    {
        return rc;    
    }

    lmep.key.level = p_md->level;
    lmep.key.vlan_id = vlan_id;
    lmep.key.ifindex = p_db_if->ifindex;    
    p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep.key);
    if (p_lmep)
    {   
        CFG_FUNC_ERR_RET("MEP on that port already exists for same vlan and level"); 
    }

    rmep_key.level = p_md->level;
    rmep_key.vlan_id = vlan_id;
    rmep_key.rmep_id = mep_id;
    p_rmep = tbl_cfm_rmep_get_cfm_rmep(&rmep_key);
    if (p_rmep)
    {   
        CFG_FUNC_ERR_RET("Mep id is used by other mep or remote mep with same level and vlan"); 
    }

    /* check same port+vid only support 4 level meps */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
    {
        if ((p_lmep->key.vlan_id == vlan_id) && (p_lmep->key.ifindex == p_db_if->ifindex))
        {
            port_level_num++;

            if (GLB_CFM_MAX_MEP_NUM_ON_CHANNEL == port_level_num)
            {
                CFG_FUNC_ERR_RET("Mep number on the port and vlan channel is full"); 
            }
        }

        /* check mep id is used */
        if ((p_lmep->key.vlan_id == vlan_id) && (p_lmep->key.level == p_md->level)
            && (p_lmep->mep_id == mep_id))
        {
            CFG_FUNC_ERR_RET("Mep id is used by other mep or remote mep with same level and vlan");     
        }

        /* check up and down mep exist at the same time */
        if ((p_lmep->key.vlan_id == vlan_id) && (p_lmep->key.level == p_md->level))
        {
            if (p_lmep->direction != direction)
            {
                CFG_FUNC_ERR_RET("Down mep and up mep can't exist for same level and vlan");
            }
            else  if (GLB_CFM_UP_MEP == direction)  
            {
                CFG_FUNC_ERR_RET("Up mep is exist for same level and vlan");
            }
        }
    }

    /* check vlan is member of interface */
    if (vlan_id && p_db_if->brgif && 
        (!GLB_BMP_ISSET(p_db_if->brgif->allowed_vlan.vlan_bmp, vlan_id)))
    {
        CFG_FUNC_ERR_RET("Interface is not bound to VLAN");
    }

    /* check local mep and remote mep with same level+vlan+port should belong to the same md */
    CTCLIB_SLIST_LOOP(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_rmep->key.level == p_md->level) && (p_rmep->key.vlan_id == vlan_id)
            && (p_rmep->ifindex == p_db_if->ifindex))
        {
            if (sal_strcmp(p_rmep->md_name, p_md->key.name))
            {
                CFG_FUNC_ERR_RET("Local mep and remote mep with same level+vlan+port should belong to the same md");    
            }
        }
    }

    lmep.mep_id = mep_id;
    lmep.cci_interval = interval;
    lmep.direction = direction;
    sal_memcpy(lmep.maid, p_ma->maid, CFM_MAID_MAX_LENGTH);
    sal_memcpy(lmep.md_name, p_md->key.name, CFM_MD_NAME_MAX_LENGTH_8021AG);
    sal_memcpy(lmep.ifname, p_db_if->key.name, IFNAME_SIZE);
    sal_memcpy(lmep.if_mac_addr, p_db_if->mac_addr, sizeof(mac_addr_t));
    rc = tbl_cfm_lmep_add_cfm_lmep(&lmep);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM local mep fail, %s", pm_err_desc(rc));
    }

    /* add lmep number on interface */
    p_db_if->cfm_lmep_num++;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_CFM_LMEP_NUM);

    /* add remote mep to sdk */
    CTCLIB_SLIST_LOOP(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_rmep->key.level == p_md->level) && (p_rmep->key.vlan_id == vlan_id)
            && (p_rmep->ifindex == p_db_if->ifindex))
        {
            p_rmep->local_mep_id = mep_id;
            tbl_cfm_rmep_set_cfm_rmep_field(p_rmep, TBL_CFM_RMEP_FLD_LOCAL_MEP_ID);

            lmep.rmep_num++;
        }
    }

    tbl_cfm_lmep_set_cfm_lmep_field(&lmep, TBL_CFM_LMEP_FLD_RMEP_NUM);

    /* set cfm vlan priority */
    if (p_ma->ccm_vlan_cos)
    {
        lmep.ccm_vlan_cos = p_ma->ccm_vlan_cos;
        tbl_cfm_lmep_set_cfm_lmep_field(&lmep, TBL_CFM_LMEP_FLD_CCM_VLAN_COS);
    }

    /* set cfm cc enable */
    if (p_ma->cci_enabled)
    {
        lmep.cci_enabled = p_ma->cci_enabled;
        tbl_cfm_lmep_set_cfm_lmep_field(&lmep, TBL_CFM_LMEP_FLD_CCI_ENABLED);
    }
    
    return rc;
}

int32
cfm_cmd_parse_lmep_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
{
    cdb_tbl_info_t    *p_tbl_info = p_node->tbl_info;
    uint32            field_id = FLD_ID_ALL;
    uint32            key_id = 0;

    if (with_key)
    {
        key_id = 3;
    }
    
    sal_memset(field, FLD_ID_ALL, sizeof(field_parse_t));
    if (key_id >= argc)
    {
        return PM_E_NONE;
    }
    
    for (field_id = 0; field_id < p_tbl_info->field_num; field_id++)
    {
        if (!sal_strcmp(argv[key_id], p_tbl_info->field[field_id].name))
        {
            field->field_id[0] = field_id;
            return PM_E_NONE;
        }
    }
    
    return PM_E_FAIL;
}

static int32
_cfm_cmd_get_lmep(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t* p_node, tbl_cfm_lmep_master_t   *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;
    tbl_cfm_lmep_t          *p_lmep = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_cfm_lmep_t          lmep;
    int32                         rc = PM_E_NONE;
    tbl_interface_key_t     if_key;
    tbl_interface_t            *p_db_if = NULL;
    tbl_cfm_md_t             md;
    tbl_cfm_md_t             *p_md = NULL;
    uint16                        vlan_id = 0;

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&lmep, 0, sizeof(tbl_cfm_lmep_t));
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfm_cmd_parse_lmep_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
        {
            tbl_cfm_lmep_dump_one(p_lmep, &args);
        }
    }
    else
    {
        sal_memcpy(if_key.name, argv[0], sal_strlen(argv[0]));    
        p_db_if = tbl_interface_get_interface(&if_key);
        if (!p_db_if)
        {
            CFG_FUNC_ERR_RET("Invalid interface or interface does not exist!");
        }

        if (argc > 1)
        {
            /* check md exist */
            sal_memcpy(md.key.name, argv[1], sal_strlen(argv[1]));
            p_md = tbl_cfm_md_get_cfm_md(&md.key);
            if (!p_md)
            {   
                CFG_FUNC_ERR_RET("MD not found in the list"); 
            }
        }

        if (argc > 2)
        {
            PM_CFG_GET_UINT(vlan_id, argv[2], para->p_rs);
        }

        lmep.key.level = p_md->level;
        lmep.key.vlan_id = vlan_id;
        lmep.key.ifindex = p_db_if->ifindex;
        p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep.key);
        if (p_lmep)
        {
            tbl_cfm_lmep_dump_one(p_lmep, &args);
        }
    }
            
    sal_fclose(fp);
    
    return rc;
}

static int32
_cfm_cmd_del_cfm_lmep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                             rc = PM_E_NONE;
    tbl_cfm_md_t                 md;
    tbl_cfm_md_t                 *p_md = NULL;
    tbl_cfm_ma_t                 ma;
    tbl_cfm_ma_t                 *p_ma = NULL;
    tbl_cfm_lmep_t               lmep;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    tbl_interface_key_t         if_key;
    tbl_interface_t                *p_db_if = NULL;
    uint16                            vlan_id = 0;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    uint8                               index = 4;          
    uint8                               direction = 0;
    uint16                             mep_id = 0;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));
    sal_memset(&lmep, 0, sizeof(tbl_cfm_lmep_t));
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));

    p_db_glb_rmep = tbl_cfm_rmep_get_master();
    if (!p_db_glb_rmep)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }

    /* lookup interface exist */
    sal_memcpy(if_key.name, argv[1], sal_strlen(argv[1]));    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist!");
    }

    /* check whether local mep or remote mep can be configure on this interface */
    rc = _cfm_check_interface_for_mep(p_db_if, para);
    if (rc)
    {
        return rc;    
    }

    /* check md exist */
    sal_memcpy(md.key.name, argv[3], sal_strlen(argv[3]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD not found in the list"); 
    }

    /* check ma exist by key */
    sal_memcpy(ma.key.name, argv[3], sal_strlen(argv[3]));

    if (argc >= 6)
    {
        if (0 == sal_strcmp(argv[index], "vlan"))
        {
            PM_CFG_GET_UINT(vlan_id, argv[index + 1], para->p_rs);
            index = index + 2;
        }
    }
    ma.key.vlan_id = vlan_id;
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (!p_ma)
    {   
        CFG_FUNC_ERR_RET("MA not found in the list"); 
    }

    lmep.key.level = p_md->level;
    lmep.key.vlan_id = vlan_id;
    lmep.key.ifindex = p_db_if->ifindex;
    p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep.key);
    if (!p_lmep)
    {   
        CFG_FUNC_ERR_RET("MEP not found in the list"); 
    }

    /* modified by liwh for bug 53637, 2019-09-21 
         when md name isn't same as md_name of local mep, return error */
    if (0 != sal_strcmp(p_lmep->md_name, p_md->key.name))
    {
        CFG_FUNC_ERR_RET("MEP not found in the list");  
    }
    /* liwh end */

    if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "down"))
        {
            direction = GLB_CFM_DOWN_MEP;
        }
        else
        {
            direction = GLB_CFM_UP_MEP;
        }

        if (direction != p_lmep->direction)
        {
            CFG_FUNC_ERR_RET("MEP not found in the list"); 
        }

        index++;        
    }

    if (argc > index)
    {
        PM_CFG_GET_UINT(mep_id, argv[index], para->p_rs);
         if (p_lmep->mep_id != mep_id)
         {
              CFG_FUNC_ERR_RET("MEP not found in the list");   
         }
    }

    /* delete remote from sdk */
    CTCLIB_SLIST_LOOP(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_rmep->key.level == p_md->level) && (p_rmep->key.vlan_id == vlan_id)
            && (p_rmep->ifindex == p_db_if->ifindex))
        {
            p_rmep->local_mep_id = 0;
            tbl_cfm_rmep_set_cfm_rmep_field(p_rmep, TBL_CFM_RMEP_FLD_LOCAL_MEP_ID);
        }
    }

    /* delete g8032 cfm association */
    if (p_lmep->ring_id)
    {
        g8032_delete_association_by_cfm(p_lmep->ring_id);
    }

    rc = tbl_cfm_lmep_del_cfm_lmep(&lmep.key);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del CFM local mep fail, %s", pm_err_desc(rc));
    }

    /* reduce lmep number on interface */
    p_db_if->cfm_lmep_num--;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_CFM_LMEP_NUM);
    
    return rc;
}

int32
cfm_cmd_process_cfm_lmep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                     *p_node = NULL;
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    int32                              rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CFM_LMEP);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_LMEP not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_lmep_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_LMEP not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _cfm_cmd_add_cfm_lmep(p_db_glb, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        rc = _cfm_cmd_del_cfm_lmep(argv, argc, para);
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_lmep(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_cfm_cmd_del_cfm_rmep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                         rc = PM_E_NONE;
    tbl_cfm_md_t             md;
    tbl_cfm_md_t             *p_md = NULL;
    tbl_cfm_ma_t             ma;
    tbl_cfm_ma_t             *p_ma = NULL;
    tbl_cfm_rmep_t           rmep;
    tbl_cfm_rmep_t           *p_rmep = NULL;
    tbl_interface_key_t      if_key;
    tbl_interface_t             *p_db_if = NULL;
    uint16                         vlan_id = 0;
    uint8                           index = 4;
    uint16                         rmep_id = 0;
    tbl_cfm_lmep_key_t     lmep_key;
    tbl_cfm_lmep_t            *p_lmep = NULL;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));
    sal_memset(&rmep, 0, sizeof(tbl_cfm_rmep_t));
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));

    /* lookup interface exist */
    sal_memcpy(if_key.name, argv[1], sal_strlen(argv[1]));    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist!");
    }

    /* check md exist */
    sal_memcpy(md.key.name, argv[3], sal_strlen(argv[3]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD not found in the list"); 
    }

    /* check ma exist by key */
    sal_memcpy(ma.key.name, argv[3], sal_strlen(argv[3]));

     if (argc > index)
    {
        if (0 == sal_strcmp(argv[index], "vlan"))
        {
            PM_CFG_GET_UINT(vlan_id, argv[index+1], para->p_rs);
            index = index + 2;
        }
    }
    ma.key.vlan_id = vlan_id;
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (!p_ma)
    {   
        CFG_FUNC_ERR_RET("MA not found in the list"); 
    }

    if (argc > index)
    {
        PM_CFG_GET_UINT(rmep_id, argv[index], para->p_rs);
    }

    rmep.key.level = p_md->level;
    rmep.key.vlan_id = vlan_id;
    rmep.key.rmep_id = rmep_id;
    p_rmep = tbl_cfm_rmep_get_cfm_rmep(&rmep.key);
    if (!p_rmep)
    {   
        CFG_FUNC_ERR_RET("Remote MEP not found in the list"); 
    }

    /* modified by liwh for bug 53637, 2019-09-21 
        when md name isn't same as md_name of local mep, return error */
    if (0 != sal_strcmp(p_rmep->md_name, p_md->key.name))
    {
        CFG_FUNC_ERR_RET("Remote MEP not found in the list");  
    }
    /* liwh end */

    if (p_rmep->ifindex != p_db_if->ifindex)
    {
        CFG_FUNC_ERR_RET("Remote MEP not found on interface %s", p_db_if->key.name);     
    }

    lmep_key.level = p_md->level;
    lmep_key.vlan_id = vlan_id;
    lmep_key.ifindex = p_db_if->ifindex;    
    p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (p_lmep) 
    {
        /* check g8032 association cfm */
        if (p_lmep->ring_id && (1 == p_lmep->rmep_num))
        {
            CFG_FUNC_ERR_RET("Can't delete remote mep because local mep and remote mep should be point to point when cfm association with g8032");
        }
        
        /* count rmep number for local mep */
        p_lmep->rmep_num--;
    }

    rc = tbl_cfm_rmep_del_cfm_rmep(&rmep.key);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Del CFM remote mep fail, %s", pm_err_desc(rc));
    }

    /* reduce rmep number on interface */
    p_db_if->cfm_rmep_num--;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_CFM_RMEP_NUM);
    
    return rc;
}

int32
_cfm_cmd_add_cfm_rmep(tbl_cfm_rmep_master_t   *p_db_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                         rc = PM_E_NONE;
    tbl_cfm_md_t             md;
    tbl_cfm_md_t             *p_md = NULL;
    tbl_cfm_ma_t             ma;
    tbl_cfm_ma_t             *p_ma = NULL;
    tbl_cfm_rmep_t           rmep;
    tbl_cfm_rmep_t           *p_rmep = NULL;
    uint16                         vlan_id = 0;
    uint16                         rmep_id = 0;
    tbl_interface_key_t      if_key;
    tbl_interface_t             *p_db_if = NULL;
    tbl_cfm_lmep_key_t     lmep_key;
    tbl_cfm_lmep_t            *p_lmep = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    tbl_cfm_lmep_master_t   *p_db_glb_lmep = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;

    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));
    sal_memset(&rmep, 0, sizeof(tbl_cfm_rmep_t));
    sal_memset(&if_key, 0, sizeof(tbl_interface_key_t));
    sal_memset(&lmep_key, 0, sizeof(tbl_cfm_lmep_key_t));

    /* check exist */
    p_db_glb_lmep = tbl_cfm_lmep_get_master();
    if (!p_db_glb_lmep)
    {
        CFG_CONFLICT_RET("TBL_CFM_LMEP not found");
    }

    p_db_glb_rmep = tbl_cfm_rmep_get_master();
    if (!p_db_glb_rmep)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }

    rc = cfm_rmep_check_valid(argv, argc, para, &vlan_id, &rmep_id, (uint8*)&rmep.mac_addr);
    if (rc)
    {
        return rc;    
    }

    /* lookup interface exist */
    sal_memcpy(if_key.name, argv[1], sal_strlen(argv[1]));    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        CFG_FUNC_ERR_RET("Invalid interface or interface does not exist!");
    }

    /* check whether local mep or remote mep can be configure on this interface */
    rc = _cfm_check_interface_for_mep(p_db_if, para);
    if (rc)
    {
        return rc;    
    }

    /* check md exist */
    sal_memcpy(md.key.name, argv[3], sal_strlen(argv[3]));
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {   
        CFG_FUNC_ERR_RET("MD not found in the list"); 
    }

    /* check only link oam can be configured on access port */
    if ((GLB_CFM_LEVEL_MIN != p_md->level) &&((GLB_VLAN_PORT_TYPE_ACCESS == p_db_if->vlan_type)))
    {
        CFG_FUNC_ERR_RET("Only link oam can be configured on access");    
    }

    /* check ma exist by key */
    sal_memcpy(ma.key.name, argv[3], sal_strlen(argv[3]));
    ma.key.vlan_id = vlan_id;
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (!p_ma)
    {   
        CFG_FUNC_ERR_RET("MA not found in the list"); 
    }
    
    /* check cfm lmep&rmep number */
    rc = cfm_check_lmep_rmep_number(p_db_glb_lmep, p_db_glb, para);
    if (rc)
    {
        return rc;    
    }

    rmep.key.level = p_md->level;
    rmep.key.vlan_id = vlan_id;
    rmep.key.rmep_id = rmep_id;    
    p_rmep = tbl_cfm_rmep_get_cfm_rmep(&rmep.key);
    if (p_rmep)
    {   
        CFG_FUNC_ERR_RET("RMEP already exist with same level and vlan"); 
    }

    /* check whether mep_id is used by other local mep */
    CTCLIB_SLIST_LOOP(p_db_glb_lmep->cfm_lmep_list, p_lmep, listnode)
    {
        if ((p_md->level == p_lmep->key.level) && (vlan_id == p_lmep->key.vlan_id)
            && (rmep_id == p_lmep->mep_id))
        {
            CFG_FUNC_ERR_RET("Mep id is used by other mep or remote mep with same level and vlan");     
        }
    }

    /* check vlan is member of interface */
    if (vlan_id && p_db_if->brgif && 
        (!GLB_BMP_ISSET(p_db_if->brgif->allowed_vlan.vlan_bmp, vlan_id)))
    {
        CFG_FUNC_ERR_RET("Interface is not bound to VLAN");
    }
    
    lmep_key.level = p_md->level;
    lmep_key.vlan_id = vlan_id;
    lmep_key.ifindex = p_db_if->ifindex;    
    p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (p_lmep) 
    {  
        /* check local mep and remote mep with same level+vlan+port should belong to the same md */
        if (sal_strcmp(p_lmep->md_name, p_md->key.name))
        {
            CFG_FUNC_ERR_RET("Local mep and remote mep with same level+vlan+port should belong to the same md");    
        }
        
        /* check g8032 association cfm */
        if (p_lmep->ring_id && (1 == p_lmep->rmep_num))
        {
            CFG_FUNC_ERR_RET("Can't add remote mep because local mep and remote mep should be point to point when cfm association with g8032");
        }
        
        /* reserver local mep id for remote mep */
        rmep.local_mep_id = p_lmep->mep_id;

        /* count rmep number for local mep */
        p_lmep->rmep_num++;
    }

    /* check local mep and remote mep with same level+vlan+port should belong to the same md */
    CTCLIB_SLIST_LOOP(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_rmep->key.level == p_md->level) && (p_rmep->key.vlan_id == vlan_id)
            && (p_rmep->ifindex == p_db_if->ifindex))
        {
            if (sal_strcmp(p_rmep->md_name, p_md->key.name))
            {
                CFG_FUNC_ERR_RET("Remote meps with same level+vlan+port should belong to the same md");    
            }
        }
    }

    rmep.ifindex = p_db_if->ifindex;
    sal_memcpy(rmep.maid, p_ma->maid, CFM_MAID_MAX_LENGTH);
    sal_memcpy(rmep.md_name, p_md->key.name, CFM_MD_NAME_MAX_LENGTH_8021AG);
    sal_memcpy(rmep.ifname, p_db_if->key.name, IFNAME_SIZE);
    rc = tbl_cfm_rmep_add_cfm_rmep(&rmep);
    if (rc < 0)
    {
        CFG_FUNC_ERR_RET("Add CFM remote mep fail, %s", pm_err_desc(rc));
    }

    /* add lmep number on interface */
    p_db_if->cfm_rmep_num++;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_CFM_RMEP_NUM);

    return rc;
}

static int32
_cfm_cmd_get_rmep(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t* p_node, tbl_cfm_rmep_master_t   *p_db_glb)
{
    FILE                          *fp = NULL;
    tbl_iter_args_t           args;     
    int32                         field_id = 0;
    field_parse_t              field;
    tbl_cfm_rmep_t          *p_rmep = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_cfm_rmep_t          rmep;
    int32                         rc = PM_E_NONE;
    tbl_cfm_md_t             md;
    tbl_cfm_md_t             *p_md = NULL;
    uint16                        vlan_id = 0;
    uint16                        rmep_id = 0;

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&rmep, 0, sizeof(tbl_cfm_rmep_t));

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;
    
    field_id = cfm_cmd_parse_lmep_field(argv, argc, p_node, &field, TRUE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    if (0 == argc)
    {
        CTCLIB_SLIST_LOOP(p_db_glb->cfm_rmep_list, p_rmep, listnode)
        {
            tbl_cfm_rmep_dump_one(p_rmep, &args);
        }
    }
    else
    {
        /* check md exist */
        sal_memcpy(md.key.name, argv[0], sal_strlen(argv[0]));
        p_md = tbl_cfm_md_get_cfm_md(&md.key);
        if (!p_md)
        {   
            CFG_FUNC_ERR_RET("MD not found in the list"); 
        }

        if (argc > 1)
        {
            PM_CFG_GET_UINT(vlan_id, argv[1], para->p_rs);
        }

        if (argc > 2)
        {
            PM_CFG_GET_UINT(rmep_id, argv[2], para->p_rs);
        }

        rmep.key.level = p_md->level;
        rmep.key.vlan_id = vlan_id;
        rmep.key.rmep_id = rmep_id;
        p_rmep = tbl_cfm_rmep_get_cfm_rmep(&rmep.key);
        if (p_rmep)
        {
            tbl_cfm_rmep_dump_one(p_rmep, &args);
        }
    }
            
    sal_fclose(fp);
    
    return rc;
}

int32
cfm_cmd_process_cfm_rmep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                     *p_node = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb = NULL;
    int32                              rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CFM_RMEP);
    if (!p_node)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }
  
    /* check exist */
    p_db_glb = tbl_cfm_rmep_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        rc = _cfm_cmd_add_cfm_rmep(p_db_glb, argv, argc, para);
        break;

    case CDB_OPER_DEL:
        rc = _cfm_cmd_del_cfm_rmep(argv, argc, para);
        break;
        
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = _cfm_cmd_get_rmep(argv, argc, para, p_node, p_db_glb);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32 cfm_show_all_local_mep(char *ifname, char *md_name, cfg_cmd_para_t *para, FILE *fp)
{
    tbl_cfm_lmep_master_t   *p_db_glb_lmep = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t            *listnode = NULL;
    int32                              rc = PM_E_NONE;
    char                               lmep_md_name[GLB_CFM_MD_MA_NAME_LENGTH_Y1731] = {0};
    tbl_cfm_global_t             *p_db_glb = NULL;
    char                               ifname_ext[IFNAME_EXT_SIZE];

    /* check exist */
    p_db_glb = tbl_cfm_global_get_cfm_global();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_GLOBAL not found");
    }
    
    /* check exist */
    p_db_glb_lmep = tbl_cfm_lmep_get_master();
    if (!p_db_glb_lmep)
    {
        CFG_CONFLICT_RET("TBL_CFM_LMEP not found");
    }
    
    sal_fprintf(fp, "######Local MEP:\n");
    sal_fprintf(fp, "Dir-Direction;\n");
    sal_fprintf(fp, "L-Level;\n");
    sal_fprintf(fp, "%4s %-4s %-12s %s %4s %-10s %9s %-14s %-5s %-8s\n", "MPID", "Dir", "DOMAIN", "L", "VLAN",
              "PORT", "CC-Status", "MAC-Address", "RDI", "Interval");
    sal_fprintf(fp, "--------------------------------------------------------------------------------\n");

    CTCLIB_SLIST_LOOP(p_db_glb_lmep->cfm_lmep_list, p_lmep, listnode)
    {
        /* if mep is not located in the interface, don't print the mep */
        if (ifname && sal_strcmp(ifname, p_lmep->ifname))
        {
            continue;    
        }

         /* if mep is not belong to the domain, don't print the mep */
        if (md_name && sal_strcmp(md_name, p_lmep->md_name))
        {
            continue;    
        }
        
        /* only display 12 characters for md name */
        sal_memcpy(lmep_md_name, p_lmep->md_name, GLB_CFM_MD_MA_NAME_LENGTH_Y1731 - 1);

        IFNAME_ETH2FULL(p_lmep->ifname, ifname_ext);

        if (p_lmep->key.vlan_id)
        {
            sal_fprintf(fp, "%-4d %-4s %-12s %d %-4d %-10s %-9s %-14s %-5s %-5s\n",
                p_lmep->mep_id, p_lmep->direction ? "up" : "down", lmep_md_name, 
                p_lmep->key.level, p_lmep->key.vlan_id, 
                ifname_ext, (p_lmep->cci_enabled && p_db_glb->cfm_enable) ? "Enabled":"Disabled", 
                cfm_format_mac_address(p_lmep->if_mac_addr),
                p_lmep->rdi  ? "True":"False",
                cfm_get_cc_interval_desc(p_lmep->cci_interval));     
        }
        else
        {
            sal_fprintf(fp, "%-4d %-4s %-12s %d %-4s %-10s %-9s %-14s %-5s %-5s\n",
                p_lmep->mep_id, p_lmep->direction ? "up" : "down", lmep_md_name, 
                p_lmep->key.level, "-", 
                ifname_ext, (p_lmep->cci_enabled && p_db_glb->cfm_enable) ? "Enabled":"Disabled", 
                cfm_format_mac_address(p_lmep->if_mac_addr),
                p_lmep->rdi  ? "True":"False",
                cfm_get_cc_interval_desc(p_lmep->cci_interval));                 
        }
    }

    sal_fprintf(fp, "\n");

    return rc;
}

int32 cfm_show_all_remote_mep(char *ifname, char *md_name, cfg_cmd_para_t *para, FILE *fp)
{
    tbl_cfm_rmep_master_t   *p_db_glb = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    int32                               rc = PM_E_NONE;
    
    /* check exist */
    p_db_glb = tbl_cfm_rmep_get_master();
    if (!p_db_glb)
    {
        CFG_CONFLICT_RET("TBL_CFM_RMEP not found");
    }

    sal_fprintf(fp, "######Remote MEP:\n");
    sal_fprintf(fp, "%-5s %-5s %-4s %-14s %-5s %-10s %-5s\n", "MPID", "LEVEL", "VLAN", "Remote Mac", "RDI",
              "FLAGS", "STATE");
    sal_fprintf(fp, "---------------------------------------------------------\n");
  
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_rmep_list, p_rmep, listnode)
    {
         /* if mep is not located in the interface, don't print the mep */
        if (ifname && sal_strcmp(ifname, p_rmep->ifname))
        {
            continue;    
        }

         /* if mep is not belong to the domain, don't print the mep */
        if (md_name && sal_strcmp(md_name, p_rmep->md_name))
        {
            continue;    
        }

        if (p_rmep->key.vlan_id)
        {
            sal_fprintf(fp, "%-5d %-5d %-4d %-14s %-5s %-10s %-5s\n",
                p_rmep->key.rmep_id, p_rmep->key.level, p_rmep->key.vlan_id,
                cfm_format_mac_address(p_rmep->mac_addr),
                p_rmep->rmep_last_rdi  ? "True":"False", "Mac_config", 
                p_rmep->first_pkt_rx ? (p_rmep->d_loc ? "Down":"Up") : "Initial");
        }
        else
        {
            sal_fprintf(fp, "%-5d %-5d %-4s %-14s %-5s %-10s %-5s\n",
                p_rmep->key.rmep_id, p_rmep->key.level, "-",
                cfm_format_mac_address(p_rmep->mac_addr),
                p_rmep->rmep_last_rdi  ? "True":"False", "Mac_config", 
                p_rmep->first_pkt_rx ? (p_rmep->d_loc ? "Down":"Up") : "Initial");               
        }
    }
    
    return rc;
}
int32
cfm_cmd_show_cfm_mep(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                           rc = PM_E_NONE;
    FILE                            *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (0 == argc)
    {
        cfm_show_all_local_mep(NULL, NULL, para, fp);
        cfm_show_all_remote_mep(NULL, NULL, para, fp);
    }
    else if ((0 == sal_strcmp(argv[0], "local")))
    {
        if (3 == argc)
        {
            if (0 == sal_strcmp(argv[1], "interface"))
            {
                cfm_show_all_local_mep(argv[2], NULL, para, fp);
            }
            else if (0 == sal_strcmp(argv[1], "domain"))
            {
                cfm_show_all_local_mep(NULL, argv[2], para, fp);
            }
        }
    }
    else if ((0 == sal_strcmp(argv[0], "remote")))
    {
        if (3 == argc)
        {
            if (0 == sal_strcmp(argv[1], "interface"))
            {
                cfm_show_all_remote_mep(argv[2], NULL, para, fp);
            }
            else if (0 == sal_strcmp(argv[1], "domain"))
            {
                cfm_show_all_remote_mep(NULL, argv[2], para, fp);
            }
        }
    }

    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
cfm_clear_rdi(tbl_cfm_lmep_t   *p_db_cfm_lmep)
{
    tbl_cfm_rmep_master_t   *p_db_glb = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    int32                               rc = PM_E_NONE;
    
    /* check exist */
    p_db_glb = tbl_cfm_rmep_get_master();
    if (!p_db_glb)
    {
        CFM_LOG_ERR("TBL_CFM_RMEP not found");
    }

    if (!p_db_cfm_lmep->rdi)
    {
        return PM_E_NONE;
    }

    if (p_db_cfm_lmep->d_unexp_mep ||p_db_cfm_lmep->d_mismerge || p_db_cfm_lmep->d_meg_lvl)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_db_glb->cfm_rmep_list, p_rmep, listnode)
    {
        if ((p_db_cfm_lmep->key.ifindex == p_rmep->ifindex)
            && (p_db_cfm_lmep->key.vlan_id == p_rmep->key.vlan_id)
            && (p_db_cfm_lmep->key.level == p_rmep->key.level))
        {
            if (p_rmep->d_loc || p_rmep->d_unexp_period)
            {
                return PM_E_NONE;    
            }
        }
    }

    p_db_cfm_lmep->rdi = FALSE;
    tbl_cfm_lmep_set_cfm_lmep_field(p_db_cfm_lmep, TBL_CFM_LMEP_FLD_RDI);
    
    return rc;
}

int32
cfm_set_error(uint8 error_reason, tbl_cfm_lmep_t   *p_db_cfm_lmep, tbl_cfm_rmep_t   *p_db_cfm_rmep)
{
    int32                           rc = PM_E_NONE;
    ds_cfm_error_t            cfm_error;
    tbl_cfm_md_t               md;
    tbl_cfm_md_t               *p_md = NULL;
    ds_cfm_error_t             *p_db_cfm_error = NULL;
    ctclib_slistnode_t          *listnode = NULL;
    ctclib_slistnode_t          *listnode_next = NULL;
    tbl_cfm_ma_t               ma;
    tbl_cfm_ma_t               *p_ma = NULL;
    static  int32                  seq = 0;

    sal_memset(&cfm_error, 0, sizeof(ds_cfm_error_t));
    sal_memset(&md, 0, sizeof(tbl_cfm_md_t));
    sal_memset(&ma, 0, sizeof(tbl_cfm_ma_t));

    if (!p_db_cfm_rmep)
    {
        CFM_LOG_INFO("CFM: MD level %d, vlan id %d, mep id %d, report error: %s",
            p_db_cfm_lmep->key.level, p_db_cfm_lmep->key.vlan_id, p_db_cfm_lmep->mep_id,
            cfm_get_defect_name(error_reason));
    }
    else
    {
        CFM_LOG_INFO("CFM: MD level %d, vlan id %d, mep id %d, rmep mac: %02x%02x.%02x%02x.%02x%02x, report error: %s",
            p_db_cfm_lmep->key.level, p_db_cfm_lmep->key.vlan_id, p_db_cfm_lmep->mep_id,
            p_db_cfm_rmep->mac_addr[0], p_db_cfm_rmep->mac_addr[1], p_db_cfm_rmep->mac_addr[2], 
            p_db_cfm_rmep->mac_addr[3], p_db_cfm_rmep->mac_addr[4], p_db_cfm_rmep->mac_addr[5], 
            cfm_get_defect_name(error_reason));
    }

    sal_memcpy(&md.key, p_db_cfm_lmep->md_name, CFM_MD_NAME_MAX_LENGTH_8021AG);
    p_md = tbl_cfm_md_get_cfm_md(&md.key);
    if (!p_md)
    {
        CFM_LOG_ERR("%s %d md %s not exist\n", __func__, __LINE__, md.key.name);
        return PM_E_NOT_EXIST;
    }

    ma.key.vlan_id = p_db_cfm_lmep->key.vlan_id;
    sal_memcpy(ma.key.name, p_db_cfm_lmep->md_name, sal_strlen(p_db_cfm_lmep->md_name));
    p_ma = tbl_cfm_ma_get_cfm_ma(&ma.key);
    if (!p_ma)
    {   
        CFM_LOG_ERR("%s %d ma key %s %d not exist\n", __func__, __LINE__, ma.key.name, ma.key.vlan_id);
        return PM_E_NOT_EXIST;
    }

    if (!p_ma->error_write_disable)
    {
        if (p_md->cfm_error.obj_list  && (p_md->cfm_error.obj_list->count >= CFM_ERROR_INDEX_MAX))
        {
            /* delete first error */
            CTCLIB_SLIST_LOOP_DEL(p_md->cfm_error.obj_list, p_db_cfm_error, listnode, listnode_next)
            {
                ds_cfm_error_del_cfm_error(p_md, p_db_cfm_error);
                break;
            }    
        }
        
        time((sal_time_t *)&cfm_error.key.time);
    
        cfm_error.vlan_id = p_db_cfm_lmep->key.vlan_id;
        sal_memcpy(cfm_error.md_name, p_db_cfm_lmep->md_name, CFM_MD_NAME_MAX_LENGTH_8021AG);
        cfm_error.level = p_db_cfm_lmep->key.level;
        cfm_error.mep_id = p_db_cfm_lmep->mep_id;
    
        if (p_db_cfm_rmep)   
        {
            sal_memcpy(cfm_error.rmep_mac, p_db_cfm_rmep->mac_addr, GLB_ETH_ADDR_LEN);
        }
        cfm_error.err_reason = error_reason;
    
        rc= ds_cfm_error_add_cfm_error(&md, &cfm_error);
        if (rc)
        {
            seq++;
            cfm_error.key.seq = seq;
            rc= ds_cfm_error_add_cfm_error(&md, &cfm_error);
            if (rc)
            {
                CFM_LOG_ERR("CFM: MD level %d add error fail", md.level);    
            }
        }
        else
        {
            seq = 0;    
        }
    }

    /* clear rdi */
    if ((GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC_CLEAR == error_reason)
        || (GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND_CLEAR == error_reason)
        || (GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH_CLEAR == error_reason)
        || (GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH_CLEAR == error_reason)
        || (GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM_CLEAR == error_reason))
    {
        cfm_clear_rdi(p_db_cfm_lmep);
    }
    
    return rc;
}

int32
cfm_set_rmep_error(tbl_cfm_rmep_t   *p_cfm_rmep, uint32 field_id)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_rmep_t           *p_db_cfm_rmep = NULL;
    tbl_cfm_lmep_key_t     lmep_key;
    tbl_cfm_lmep_t           *p_db_cfm_lmep = NULL;

    sal_memset(&lmep_key, 0, sizeof(tbl_cfm_lmep_key_t));

    /* find remote mep */
    p_db_cfm_rmep = tbl_cfm_rmep_get_cfm_rmep(&p_cfm_rmep->key);
    if (!p_db_cfm_rmep)
    {   
        return PM_E_NOT_EXIST; 
    }

    /* find local mep */
    lmep_key.ifindex = p_db_cfm_rmep->ifindex;
    lmep_key.vlan_id = p_db_cfm_rmep->key.vlan_id;
    lmep_key.level = p_db_cfm_rmep->key.level;
    
    p_db_cfm_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (!p_db_cfm_lmep)
    {   
        return PM_E_NOT_EXIST; 
    }
    
    switch (field_id)
    {
    case TBL_CFM_RMEP_FLD_FIRST_PKT_RX:
        if (p_cfm_rmep->first_pkt_rx != p_db_cfm_rmep->first_pkt_rx)
        {
            if (p_cfm_rmep->first_pkt_rx)
            {
                if (p_db_cfm_lmep->ring_id)
                {
                    g8032_ring_switch_trigged_by_cfm(p_db_cfm_lmep->ring_id, lmep_key.ifindex, TRUE);
                }
            }
            
            p_db_cfm_rmep->first_pkt_rx = p_cfm_rmep->first_pkt_rx;
            cfm_set_error(GLB_CFM_RMEP_FIRST_PACKET_RECEIVE, p_db_cfm_lmep, p_db_cfm_rmep);
        }
        break;

    case TBL_CFM_RMEP_FLD_D_LOC:
        if (p_cfm_rmep->d_loc != p_db_cfm_rmep->d_loc)
        {
            p_db_cfm_rmep->d_loc = p_cfm_rmep->d_loc;

            if (p_cfm_rmep->d_loc)
            {
                if (p_db_cfm_lmep->ring_id)
                {
                    g8032_ring_switch_trigged_by_cfm(p_db_cfm_lmep->ring_id, lmep_key.ifindex, FALSE);
                }
                
                cfm_set_error(GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC, p_db_cfm_lmep, p_db_cfm_rmep);
                p_db_cfm_lmep->rdi = TRUE;
            }
            else
            {
                if (p_db_cfm_lmep->ring_id)
                {
                    g8032_ring_switch_trigged_by_cfm(p_db_cfm_lmep->ring_id, lmep_key.ifindex, TRUE);
                }
                
                cfm_set_error(GLB_CFM_SOME_RMEP_CCM_DEFECT_DLOC_CLEAR, p_db_cfm_lmep, p_db_cfm_rmep);
            }
        }
        break;

    case TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD:
         if (p_cfm_rmep->d_unexp_period != p_db_cfm_rmep->d_unexp_period)
        {
            p_db_cfm_rmep->d_unexp_period = p_cfm_rmep->d_unexp_period;

            if (p_cfm_rmep->d_unexp_period)
            {
                cfm_set_error(GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH, p_db_cfm_lmep, p_db_cfm_rmep);
                p_db_cfm_lmep->rdi = TRUE;
            }
            else
            {
                cfm_set_error(GLB_CFM_ERROR_CCM_DEFECT_CCM_INTERVAL_MISMATCH_CLEAR, p_db_cfm_lmep, p_db_cfm_rmep);
            }
        }
        break;

    case TBL_CFM_RMEP_FLD_RMEP_LAST_RDI:
         if (p_cfm_rmep->rmep_last_rdi != p_db_cfm_rmep->rmep_last_rdi)
        {
            CFM_LOG_DEBUG(OPMDBG_FLAG_CFM_EVENT, "[EVENT]: set rdi %d by chip for rmep mepid %d level %d vid %d",
                p_cfm_rmep->rmep_last_rdi, p_cfm_rmep->key.rmep_id, p_cfm_rmep->key.level, p_cfm_rmep->key.vlan_id);
        }
        break;

    case TBL_CFM_RMEP_FLD_MA_SA_MISMATCH:
         if (p_cfm_rmep->ma_sa_mismatch != p_db_cfm_rmep->ma_sa_mismatch)
        {
            p_db_cfm_rmep->ma_sa_mismatch = p_cfm_rmep->ma_sa_mismatch;

            if (p_cfm_rmep->ma_sa_mismatch)
            {
                cfm_set_error(GLB_CFM_RMEP_SRC_MAC_MISMATCH, p_db_cfm_lmep, p_db_cfm_rmep);
            }
        }
        break;

    case TBL_CFM_RMEP_FLD_MAC_ADDR:
        if (sal_memcmp(p_db_cfm_rmep->mac_addr, p_cfm_rmep->mac_addr, GLB_ETH_ADDR_LEN))
        {        
            CFM_LOG_INFO("CFM: remote mep (MD level %d, vlan id %d, rmep id %d) update mac to: %02x%02x.%02x%02x.%02x%02x",
                p_cfm_rmep->key.level, p_cfm_rmep->key.vlan_id, p_cfm_rmep->key.rmep_id,
                p_cfm_rmep->mac_addr[0], p_cfm_rmep->mac_addr[1], p_cfm_rmep->mac_addr[2], 
                p_cfm_rmep->mac_addr[3], p_cfm_rmep->mac_addr[4], p_cfm_rmep->mac_addr[5]);        
        }
        break;
      
    default:
        break;
    }

    return rc;
}

int32
cfm_cmd_pm_process_cfm_rmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;
    tbl_cfm_rmep_t           *p_cfm_rmep = NULL;

    CFG_DBG_FUNC();

    if (!p_tbl)
    {
        return PM_E_INVALID_PARAM;   
    }

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cfm_rmep = (tbl_cfm_rmep_t*)p_tbl;
        rc = cfm_set_rmep_error(p_cfm_rmep, field_id);
        break;

    default:
        break;
    }
    
    return rc;
}

int32
cfm_set_lmep_error(tbl_cfm_lmep_t   *p_cfm_lmep, uint32 field_id)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_lmep_t           *p_db_cfm_lmep = NULL;

    /* find local mep */
    p_db_cfm_lmep = tbl_cfm_lmep_get_cfm_lmep(&p_cfm_lmep->key);
    if (!p_db_cfm_lmep)
    {   
        return PM_E_NOT_EXIST; 
    }
    
    switch (field_id)
    {
    case TBL_CFM_LMEP_FLD_D_UNEXP_MEP:
        if (p_cfm_lmep->d_unexp_mep != p_db_cfm_lmep->d_unexp_mep)
        {
            p_db_cfm_lmep->d_unexp_mep = p_cfm_lmep->d_unexp_mep;

            if (p_cfm_lmep->d_unexp_mep)
            {
                cfm_set_error(GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND, p_db_cfm_lmep, NULL);
                p_db_cfm_lmep->rdi = TRUE;
            }
            else
            {
                cfm_set_error(GLB_CFM_ERROR_CCM_DEFECT_RMEP_NOT_FOUND_CLEAR, p_db_cfm_lmep, NULL);    
            }
        }
        break;

    case TBL_CFM_LMEP_FLD_D_MISMERGE:
        if (p_cfm_lmep->d_mismerge != p_db_cfm_lmep->d_mismerge)
        {
            p_db_cfm_lmep->d_mismerge = p_cfm_lmep->d_mismerge;
 
            if (p_cfm_lmep->d_mismerge)
            {
                cfm_set_error(GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH, p_db_cfm_lmep, NULL);
                p_db_cfm_lmep->rdi = TRUE;
            }
            else
            {
                cfm_set_error(GLB_CFM_XCON_CCM_DEFECT_CCM_MAID_MISMATCH_CLEAR, p_db_cfm_lmep, NULL);    
            }
        }
        break;

    case TBL_CFM_LMEP_FLD_D_MEG_LVL:
        if (p_cfm_lmep->d_meg_lvl != p_db_cfm_lmep->d_meg_lvl)
        {
            p_db_cfm_lmep->d_meg_lvl = p_cfm_lmep->d_meg_lvl;

            if (p_cfm_lmep->d_meg_lvl)
            {
                cfm_set_error(GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM, p_db_cfm_lmep, NULL);
                p_db_cfm_lmep->rdi = TRUE;
            }
            else
            {
                cfm_set_error(GLB_CFM_XCON_CCM_DEFECT_CCM_LOW_CCM_CLEAR, p_db_cfm_lmep, NULL);    
            }
        }
        break;

    case TBL_CFM_LMEP_FLD_RDI:
         if (p_cfm_lmep->rdi != p_db_cfm_lmep->rdi)
        {
            CFM_LOG_DEBUG(OPMDBG_FLAG_CFM_EVENT, "[EVENT]: set rdi %d by chip for mep mepid %d level %d vid %d when report error",
                p_cfm_lmep->rdi, p_cfm_lmep->mep_id, p_cfm_lmep->key.level, p_cfm_lmep->key.vlan_id);
        }
        break;

    default:
        break;
    }

    return rc;
}

int32
cfm_api_delete_lmep(tbl_cfm_lmep_t    *p_cfm_lmep)
{
    tbl_cfm_lmep_t               *p_db_lmep = NULL;
    
    p_db_lmep = tbl_cfm_lmep_get_cfm_lmep(&p_cfm_lmep->key);
    if (p_db_lmep)
    {
         /* delete g8032 cfm association */
         if (p_db_lmep->ring_id)
         {
             g8032_delete_association_by_cfm(p_db_lmep->ring_id);
         }
    }

    return PM_E_NONE;
}

int32
cfm_cmd_pm_process_cfm_lmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;
    tbl_cfm_lmep_t           *p_cfm_lmep = NULL;

    CFG_DBG_FUNC();

    if (!p_tbl)
    {
        return PM_E_INVALID_PARAM;   
    }

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        rc = cfm_set_lmep_error(p_cfm_lmep, field_id);
        break;

    case CDB_OPER_DEL:
        p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        rc = cfm_api_delete_lmep(p_cfm_lmep);
        break;

    default:
        break;
    }
    
    return rc;
}

int32
cfm_vlan_delete_vlan(vid_t    vid)
{
    int32                   rc = PM_E_NONE;
    tbl_vlan_key_t     vlan_key;
    tbl_vlan_t            *p_db_vlan = NULL;

    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (!p_db_vlan)
    {
        CFM_LOG_DEBUG(OPMDBG_FLAG_CFM_EVENT, "VLAN not found on the bridge");
        return PM_E_NOT_EXIST;
    }

    cfm_api_delete_ma_for_vlan(vid);

    return rc;
}

int32
cfm_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_key_t           *p_vlan_key = NULL;
    int32                          rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
        
    case CDB_OPER_DEL:
        p_vlan_key = (tbl_vlan_key_t*)p_tbl;
        rc = cfm_vlan_delete_vlan(p_vlan_key->vid);
        break;

    case CDB_OPER_SET:
 
        break;

    default:
        break;
    }

    return rc;
}

