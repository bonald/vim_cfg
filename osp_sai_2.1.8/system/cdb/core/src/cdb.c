
#include "proto.h"

#include "gen/cdb_inc_priv.h"
#include "gen/cdb_dir_priv.h"
#include "gen/cdb_tbl_field_priv.h"
#include "gen/cdb_tbl_info_priv.h"
#include "gen/cdb_tbl_priv.h"
#include "gen/cdb_ds_field_priv.h"
#include "gen/cdb_ds_info_priv.h"
#include "gen/cdb_ds_priv.h"
#include "gen/cdb_act_field_priv.h"
#include "gen/cdb_act_info_priv.h"
#include "gen/cdb_act_priv.h"
#include "gen/cdb_bat_priv.h"
#include "cdb_store.h"

#if defined TAPPRODUCT
static cdb_product_type_t g_product_type = CDB_PRODUCT_TAP;
#elif defined OFPRODUCT
static cdb_product_type_t g_product_type = CDB_PRODUCT_OPENFLOW;
#else
static cdb_product_type_t g_product_type = CDB_PRODUCT_L2PLUS;
#endif

static CDB_SUBSCRIBE_TBL_FUNC g_pm_tbl_sub_before_fn[TBL_MAX];
static CDB_SUBSCRIBE_TBL_FUNC g_pm_tbl_sub_after_fn[TBL_MAX];

static CDB_SUBSCRIBE_TBL_REFER_FUNC g_pm_tbl_sub_refer_before_fn[TBL_MAX];
static CDB_SUBSCRIBE_TBL_REFER_FUNC g_pm_tbl_sub_refer_after_fn[TBL_MAX];

static CDB_SUBSCRIBE_TBL_REFER_FUNC g_pm_ds_sub_refer_before_fn[DS_MAX];
static CDB_SUBSCRIBE_TBL_REFER_FUNC g_pm_ds_sub_refer_after_fn[DS_MAX];

#define DB_CDS_ROOT cdb_get_dir(DIR_CDB)

char*
str_tolower(char *str)
{
    char *p = str;
    while (p && CMD_ZERO_CHAR != *p)
    {
        *p = sal_tolower(*p);
        p++;
    }

    return str;
}

char*
str_toupper(char *str)
{
    char *p = str;
    while (p && CMD_ZERO_CHAR != *p)
    {
        *p = sal_toupper(*p);
        p++;
    }

    return str;
}

cdb_node_t*
cdb_get_dir(uint16 id)
{
    if (id >= DIR_MAX)
    {
        return NULL;
    }
    return &g_dir_node_info[id];
}

cdb_node_t*
cdb_get_tbl(uint16 id)
{
    if (id >= TBL_MAX)
    {
        return NULL;
    }
    return &g_tbl_node_info[id];
}

cdb_node_t*
cdb_get_ds(uint16 id)
{
    if (id >= DS_MAX)
    {
        return NULL;
    }
    return &g_ds_node_info[id];
}

cdb_node_t*
cdb_get_act(uint16 id)
{
    if (id >= ACT_MAX)
    {
        return NULL;
    }
    return &g_act_node_info[id];
}

cdb_node_t*
cdb_get_bat(uint16 id)
{
    if (id >= BAT_MAX)
    {
        return NULL;
    }
    return &g_bat_node_info[id];
}

cdb_node_t*
cdb_get_tbl_by_name(char *name)
{
    cdb_node_t *p_node = NULL;
    uint32 i = 0;

    for (i = 0; i < TBL_MAX; i++)
    {
        p_node = &(g_tbl_node_info[i]);
        if (0 == sal_strcmp(name, p_node->name))
        {
            return p_node;
        }

    }
    
    return NULL;
}

cdb_node_t*
cdb_get_ds_by_name(char *name)
{
    cdb_node_t *p_node = NULL;
    uint32 i = 0;

    for (i = 0; i < TBL_MAX; i++)
    {
        p_node = &(g_ds_node_info[i]);
        if (0 == sal_strcmp(name, p_node->name))
        {
            return p_node;
        }

    }
    
    return NULL;
}

int32
db_init_db_one(cdb_node_t *p_node)
{
    uint32 i = 0;
    cdb_node_t *p_sub = NULL;
    
    ctclib_list_init(&p_node->sub);

    for (i = 0; i < DIR_MAX; i++)
    {
        p_sub = &g_dir_node_info[i];
        if (!p_sub)
        {
            continue;
        }
        if (!p_sub->parent)
        {
            continue;
        }

        if ((p_sub->parent->type == p_node->type) && (p_sub->parent->id == p_node->id))
        {
            ctclib_list_insert_tail(&p_node->sub, &p_sub->node);
        }
    }

    for (i = 0; i < TBL_MAX; i++)
    {
        p_sub = &g_tbl_node_info[i];
        if (!p_sub)
        {
            continue;
        }
        if (!p_sub->parent)
        {
            continue;
        }

        if ((p_sub->parent->type == p_node->type) && (p_sub->parent->id == p_node->id))
        {
            ctclib_list_insert_tail(&p_node->sub, &p_sub->node);
        }
    }

    for (i = 0; i < DS_MAX; i++)
    {
        p_sub = &g_ds_node_info[i];
        if (!p_sub)
        {
            continue;
        }
        if (!p_sub->parent)
        {
            continue;
        }

        if ((p_sub->parent->type == p_node->type) && (p_sub->parent->id == p_node->id))
        {
            ctclib_list_insert_tail(&p_node->sub, &p_sub->node);
        }
    }

    for (i = 0; i < ACT_MAX; i++)
    {
        p_sub = &g_act_node_info[i];
        if (!p_sub)
        {
            continue;
        }
        if (!p_sub->parent)
        {
            continue;
        }

        if ((p_sub->parent->type == p_node->type) && (p_sub->parent->id == p_node->id))
        {
            ctclib_list_insert_tail(&p_node->sub, &p_sub->node);
        }
    }

    for (i = 0; i < BAT_MAX; i++)
    {
        p_sub = &g_bat_node_info[i];
        if (!p_sub)
        {
            continue;
        }
        if (!p_sub->parent)
        {
            continue;
        }

        if ((p_sub->parent->type == p_node->type) && (p_sub->parent->id == p_node->id))
        {
            ctclib_list_insert_tail(&p_node->sub, &p_sub->node);
        }
    }
    
    return PM_E_NONE;
}

int32
cdb_node_parse(const char *path, cfg_parse_info_t *p_info)
{
    uint32          line_strlen = 0;
    const char     *start = NULL;
    char           *pos = NULL;
    char           *line = NULL;
    
    if (!path)
    {
        return PM_E_NONE;
    }

    sal_memset(p_info, 0, sizeof(cfg_parse_info_t));
    if (CMD_DIVISION_CHAR == path[0])
    {
        start = path + 1;
    }
    else
    {
        start = path;
    }
    sal_strncpy(p_info->buf, start, DB_MAX_PATH_LEN);
    line = p_info->buf;
    
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == CMD_DIVISION_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    
    p_info->argv[p_info->argc++] = line;

    pos = cfg_strchr(line, CMD_DIVISION_CHAR);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = cfg_strchr(pos + 1, CMD_DIVISION_CHAR);
    }
    
    return PM_E_NONE;
}

cdb_node_t*
cdb_get_node_by_id(uint16 type, uint16 id)
{
    switch (type)
    {
    case CDB_NODE_DIR:
        return cdb_get_dir(id);
    case CDB_NODE_TBL:
        return cdb_get_tbl(id);
    case CDB_NODE_DS:
        return cdb_get_ds(id);
    case CDB_NODE_ACT:
        return cdb_get_act(id);
    case CDB_NODE_BAT:
        return cdb_get_bat(id);
    default:
        return NULL;
    }
}

cdb_node_t*
cdb_get_node_by_name(const char *path)
{
    cfg_parse_info_t info;
    cdb_node_t *p_node = NULL;
    cdb_node_t *p_sub = NULL;
    ctclib_list_node_t* p_listnode = NULL;
    uint32 depth = 0;
    uint32 match = FALSE;
    uint32 sub_count = 0;
    if (!path)
    {
        return DB_CDS_ROOT;
    }
    cdb_node_parse(path, &info);

    if (!info.argc)
    {
        return NULL;
    }

    p_node = DB_CDS_ROOT;
    if (sal_strcmp(info.argv[0], p_node->name))
    {
        return NULL;
    }
    
    for (depth = 1; depth < info.argc; depth++)
    {
        match = FALSE;
        ctclib_list_for_each(p_listnode, &p_node->sub)
        {
            p_sub = ctclib_container_of(p_listnode, cdb_node_t, node);
            if (!sal_strcmp(info.argv[depth], p_sub->name))
            {
                match = TRUE;
                p_node = p_sub;
                break;
            }
            sub_count++;
        }
        if (!match)
        {
            return NULL;
        }

        if (CDB_NODE_TBL == p_node->type || CDB_NODE_ACT == p_node->type || CDB_NODE_BAT == p_node->type)
        {
            depth++;
            break;
        }
    }

    return p_node;
}

int32
cdb_get_field_id(char *field_name, cdb_node_t* p_node, cdb_field_t **pp_field)
{
    cdb_tbl_info_t *p_tbl_info = NULL;
    int32 fld_id = -1;

    if (NULL == field_name)
    {
        return PM_E_FAIL;
    }
    
    p_tbl_info = p_node->tbl_info;
    for (fld_id = 0; fld_id < p_tbl_info->field_num; fld_id++)
    {
        if (!sal_strcmp(field_name, p_tbl_info->field[fld_id].name))
        {
            *pp_field = &(p_tbl_info->field[fld_id]);
            return fld_id;
        }
    }
    return PM_E_FAIL;
}

const char*
cdb_node_type_str(uint32 type)
{
    switch(type)
    {
    case CDB_NODE_DIR:
        return "dir";

    case CDB_NODE_TBL:
        return "table";

    case CDB_NODE_DS:
        return "data-struct";

    case CDB_NODE_ACT:
        return "action";
        
    case CDB_NODE_BAT:
        return "batch";

    default :
        return "invalid";
    }
}

const char*
cdb_product_type_str(uint32 type)
{
    switch(type)
    {
    case CDB_PRODUCT_ALL:
        return "all";

    case CDB_PRODUCT_L2PLUS:
        return "L2+";

    case CDB_PRODUCT_OPENFLOW:
        return "openflow";

    case CDB_PRODUCT_TAP:
        return "tap";

    default :
        return "invalid";
    }
}

static int32
_cdb_dump_dir_node(const char *path, FILE *fp)
{
    cdb_node_t *p_node = NULL;
    cdb_node_t *p_sub = NULL;
    ctclib_list_node_t* p_listnode = NULL;
    uint32 sub_count = 0;
    int32 rc = PM_E_NONE;
    
    p_node = cdb_get_node_by_name(path);
    if (!p_node)
    {
        sal_fprintf(fp, "Invalid db path: %s\n", path);
        return PM_E_NONE;
    }

    PM_E_RETURN(cdb_check_product_out(p_node, fp));

    sal_fprintf(fp, "ID          : %d\n", p_node->id);
    sal_fprintf(fp, "Type        : %s\n", cdb_node_type_str(p_node->type));
    sal_fprintf(fp, "Name        : %s\n", p_node->name);
    sal_fprintf(fp, "Full Name   : %s\n", p_node->full_name);

    sal_fprintf(fp, "\n");

    if (!ctclib_list_empty(&p_node->sub))
    {
        sal_fprintf(fp, "### Sub ###\n");
        ctclib_list_for_each(p_listnode, &p_node->sub)
        {
            p_sub = ctclib_container_of(p_listnode, cdb_node_t, node);
            rc = cdb_check_product(p_sub);
            if (rc < 0)
            {
                continue;
            }
            sal_fprintf(fp, "  %-4d      : %-22s %s\n", sub_count, p_sub->name, cdb_node_type_str(p_sub->type));
            sub_count++;
        }
    }
    return PM_E_NONE;
}

int32
cdb_dump_dir_node(const char *path, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    _cdb_dump_dir_node(path, fp);
    sal_fclose(fp);

    return PM_E_NONE;
}


cdb_field_t*
cdb_get_field(cdb_node_t* p_node, const char *field_name, uint32 *p_field_id)
{
    cdb_field_t *p_field = NULL;
    cdb_tbl_info_t *tbl_info = NULL;
    uint32 i = 0;

    tbl_info = p_node->tbl_info;
    if (NULL == tbl_info)
    {
        return NULL;
    }
    
    for (i = 0; i < tbl_info->field_num; i++)
    {
        if (!sal_strcmp(field_name, tbl_info->field[i].name))
        {
            p_field = &(tbl_info->field[i]);
            break;
        }
    }

    *p_field_id = i;
    return p_field;
}

const char*
cdb_ds_val2str(uint16 ds_type)
{
    cdb_node_t *p_node = cdb_get_ds(ds_type);
    if (NULL == p_node)
    {
        return "Invalid";
    }
    return p_node->name;
}

static int32
_cdb_dump_tbl_node(cdb_node_t* p_node, char *param, uint32 param_len, FILE *fp)
{
    cfg_parse_info_t parse_info;
    cdb_node_t *p_sub_node = NULL;
    cdb_field_t* p_field = NULL;
    cdb_tbl_info_t *tbl_info = NULL;
    uint32 field_id = 0;
    uint32 sub_param_len = 0;
    uint32 i = 0;
    int32 rc = PM_E_NONE;

    PM_E_RETURN(cdb_check_product_out(p_node, fp));

    tbl_info = p_node->tbl_info;
    if (0 == param_len)
    {
        sal_fprintf(fp, "ID          : %d\n", p_node->id);
        sal_fprintf(fp, "Name        : %s\n", p_node->name);
        sal_fprintf(fp, "Full Name   : %s\n", p_node->full_name);
        sal_fprintf(fp, "Type        : %s\n", cdb_node_type_str(p_node->type));
        sal_fprintf(fp, "\n");
        
        sal_fprintf(fp, "### data-struct ###\n");
        sal_fprintf(fp, "Key Size    : %u\n", tbl_info->key_size);
        sal_fprintf(fp, "Data Size   : %u\n", tbl_info->data_size);
        sal_fprintf(fp, "Field Num   : %u\n", tbl_info->field_num);
        sal_fprintf(fp, "%-8s %-32s %-10s %-15s %-10s\n", "Field ID", "Field Name", "DT Type", "DS Type", "Access");
        sal_fprintf(fp, "------------------------------------------------------------------------------\n");
        for (i = 0; i < tbl_info->field_num; i++)
        {
            p_field = &tbl_info->field[i];
            sal_fprintf(fp, "%-8u %-32s %-10s %-15s %-10s\n",
                i,
                p_field->name,
                cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type),
                cdb_ds_val2str(p_field->ds_type),
                cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));
        }
    }
    else
    {
        rc = cdb_node_parse(param, &parse_info);
        if (0 == parse_info.argc)
        {
            sal_fprintf(fp, "Invalid path %s/%s\n", p_node->full_name, param);
            return PM_E_NONE;
        }

        p_field = cdb_get_field(p_node, parse_info.argv[0], &field_id);
        if (NULL == p_field)
        {
            sal_fprintf(fp, "Invalid path %s/%s\n", p_node->full_name, param);
            sal_fprintf(fp, "Options include:\n");
            for (i = 0; i < tbl_info->field_num; i++)
            {
                p_field = &tbl_info->field[i];
                sal_fprintf(fp, "  %s\n", p_field->name);
            }
            return PM_E_NONE;
        }

        sal_fprintf(fp, "Field ID    : %u\n", field_id);
        sal_fprintf(fp, "Field Name  : %s\n", p_field->name);
        sal_fprintf(fp, "Full Name   : %s/%s\n", p_node->full_name, p_field->name);
        sal_fprintf(fp, "DT Type     : %s\n", cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type));
        sal_fprintf(fp, "DS Type     : %s\n", cdb_ds_val2str(p_field->ds_type));
        sal_fprintf(fp, "Access      : %s\n", cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));

        switch (p_field->dt_type)
        {
        case CDB_DT_DS:
            sal_fprintf(fp, "\n### Sub DS info ###\n");
            p_sub_node = cdb_get_ds(p_field->ds_type);
            if (p_sub_node)
            {
                if (parse_info.argv[1])
                {
                    sub_param_len = sal_strlen(parse_info.argv[1]);
                }
                _cdb_dump_tbl_node(p_sub_node, parse_info.argv[1], sub_param_len, fp);
            }
            else
            {
                sal_fprintf(fp, "Not found DS %u\n", p_field->ds_type);
            }
            break;
            
        case CDB_DT_REFERLIST:
            sal_fprintf(fp, "\n### Sub Referlist info ###\n");
            p_sub_node = cdb_get_ds(p_field->ds_type);
            if (p_sub_node)
            {
                if (parse_info.argv[1])
                {
                    sub_param_len = sal_strlen(parse_info.argv[1]);
                }
                _cdb_dump_tbl_node(p_sub_node, parse_info.argv[1], sub_param_len, fp);
            }
            else
            {
                sal_fprintf(fp, "Referlist node is not DS\n");
            }
            break;
            
        default:
            break;
        }
    }
    
    return PM_E_NONE;
}

int32
cdb_check_product(cdb_node_t* p_node)
{
    if (CDB_PRODUCT_ALL == g_product_type)
    {
        return PM_E_NONE;
    }
    else
    {
        if (CDB_PRODUCT_ALL == p_node->product
         || g_product_type == p_node->product)
        {
            return PM_E_NONE;
        }
    }
    
    return PM_E_NOT_SUPPORT;
}

int32
cdb_check_product_out(cdb_node_t* p_node, FILE *fp)
{
    int32 rc = PM_E_NONE;
    
    rc = cdb_check_product(p_node);
    if (rc < 0)
    {
        sal_fprintf(fp, "CDB %s is not supported in the product\n", p_node->full_name);
    }

    return rc;
}

int32
cdb_dump_tbl_node(cdb_node_t* p_node, char *param, uint32 param_len, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    _cdb_dump_tbl_node(p_node, param, param_len, fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
call_show_func(cfg_cmd_para_t *para, void (*func)(), void* args)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (args)
    {
        func(fp, args);
    }
    else
    {
        func(fp);
    }

    sal_fclose(fp);
    return rc;
}

int32
cdb_init(pm_id_t pm_id)
{
    uint32 i = 0;
    cdb_node_t *p_node = NULL;
   
    for (i = 0; i < DIR_MAX; i++)
    {
        p_node = &g_dir_node_info[i];
        if (!p_node)
        {
            continue;
        }
        db_init_db_one(p_node);
    }

    for (i = 0; i < TBL_MAX; i++)
    {
        p_node = &g_tbl_node_info[i];
        if (!p_node)
        {
            continue;
        }
        db_init_db_one(p_node);
    }

    for (i = 0; i < DS_MAX; i++)
    {
        p_node = &g_ds_node_info[i];
        if (!p_node)
        {
            continue;
        }
        db_init_db_one(p_node);
    }

    for (i = 0; i < ACT_MAX; i++)
    {
        p_node = &g_act_node_info[i];
        if (!p_node)
        {
            continue;
        }
        db_init_db_one(p_node);
    }

    for (i = 0; i < BAT_MAX; i++)
    {
        p_node = &g_bat_node_info[i];
        if (!p_node)
        {
            continue;
        }
        db_init_db_one(p_node);
    }

    cdb_tbl_pm_init(pm_id);
    cdb_sync_publish_init();
    return PM_E_NONE;
}

int32
cdb_register_cfg_tbl_cb(uint16 tbl_id, CMD_FUNC cb)
{
    cdb_node_t* p_node = NULL;
    
    p_node = cdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return PM_E_FAIL;
    }

    p_node->tbl_info->ops.cmd_fn = cb;
    
    return PM_E_NONE;
}

int32
cdb_register_cfg_act_cb(uint16 act_id, CMD_FUNC cb)
{
    cdb_node_t* p_node = NULL;
    
    p_node = cdb_get_act(act_id);
    if (NULL == p_node)
    {
        return PM_E_FAIL;
    }

    p_node->tbl_info->ops.cmd_fn = cb;
    
    return PM_E_NONE;
}

int32
tbl_hash_iter_adpt_fn(ctclib_hash_backet_t* phb, void *parg1, void *p_arg2)
{
    int32 rc = PM_E_NONE;
    
    TBL_ITER_CB_FUNC fn = (TBL_ITER_CB_FUNC)parg1;
    tbl_iter_args_t *pargs = (tbl_iter_args_t *)p_arg2;

    rc = fn(phb->data, pargs);
    return rc;
}

int32
tbl_hash_iter_adpt_fn_cont(ctclib_hash_backet_t* phb, int32 cur_index, void *parg1, void *p_arg2)
{
    uint32 *p_hash_index = NULL;
    int32 rc = PM_E_NONE;
    
    TBL_ITER_CB_FUNC fn = (TBL_ITER_CB_FUNC)parg1;
    tbl_iter_args_t *pargs = (tbl_iter_args_t *)p_arg2;

    rc = fn(phb->data, pargs);
    p_hash_index = (uint32*)pargs->argv[3];
    *p_hash_index = cur_index;

    return rc;
}

int32
cfg_cmd_parse_field(char **argv, int32 argc, cdb_node_t* p_node, field_parse_t *field, uint32 with_key)
{
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    uint32 field_id = FLD_ID_ALL;
    uint32 key_id = 0;

    if (with_key)
    {
        key_id = 1;
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

int32
cdb_reference_list_add_object(cdb_reference_list_t *ref_list, void *p_object)
{
    ctclib_slistnode_t *listnode = NULL;

    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return PM_E_NOT_EXIST;
    }
    
    listnode = ctclib_slistnode_insert_sort(ref_list->obj_list, p_object);
    if (NULL == listnode)
    {
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

int32
cdb_reference_list_del_object(cdb_reference_list_t *ref_list, void *p_object)
{
    int32 rc = PM_E_NONE;

    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return PM_E_NOT_EXIST;
    }
    
    rc = ctclib_slistnode_delete(ref_list->obj_list, p_object);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}

void*
cdb_reference_list_lookup_object(cdb_reference_list_t *ref_list, void *p_object)
{
    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return NULL;
    }
    
    return ctclib_slistdata_lookup(ref_list->obj_list, p_object);
}

int32
cdb_reference_list_init(cdb_reference_list_t *ref_list, uint16 type, uint16 id, CTCLIB_SLIST_CMP_FN cmp)
{
    ref_list->type = type;
    ref_list->id = id;
    ref_list->obj_list = ctclib_slist_create(cmp, NULL);
    if (NULL == ref_list->obj_list)
    {
        return PM_E_NO_MEMORY;
    }
    return PM_E_NONE;
}

int32
cdb_reference_list_deinit(cdb_reference_list_t *ref_list)
{
    ref_list->type = 0;
    ref_list->id = 0;
    if (!CTCLIB_SLIST_ISEMPTY(ref_list->obj_list))
    {
        SAL_ASSERT(0);
    }
    ctclib_slist_delete(ref_list->obj_list);
    ref_list->obj_list = NULL;
    return PM_E_NONE;
}

int32
cdb_reference_init(cdb_reference_t *p_ref, uint16 id)
{
    p_ref->type = CDB_NODE_TBL;
    p_ref->id = id;
    return PM_E_NONE;
}

int32
cdb_reference_deinit(cdb_reference_t *p_ref)
{
    p_ref->type = 0;
    p_ref->id = 0;
    return PM_E_NONE;
}

int32
cdb_pm_subscribe_tbl(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_FUNC before_fn, CDB_SUBSCRIBE_TBL_FUNC after_fn)
{
    if (tbl_id >= TBL_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    g_pm_tbl_sub_before_fn[tbl_id] = before_fn;
    g_pm_tbl_sub_after_fn[tbl_id] = after_fn;
    
    return PM_E_NONE;
}

int32
cdb_pm_subscribe_tbl_refer(cdb_tbl_t tbl_id, CDB_SUBSCRIBE_TBL_REFER_FUNC before_fn, CDB_SUBSCRIBE_TBL_REFER_FUNC after_fn)
{
    if (tbl_id >= TBL_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    g_pm_tbl_sub_refer_before_fn[tbl_id] = before_fn;
    g_pm_tbl_sub_refer_after_fn[tbl_id] = after_fn;
    
    return PM_E_NONE;
}

int32
cdb_pm_subscribe_ds_refer(cdb_ds_t ds_id, CDB_SUBSCRIBE_TBL_REFER_FUNC before_fn, CDB_SUBSCRIBE_TBL_REFER_FUNC after_fn)
{
    if (ds_id >= DS_MAX)
    {
        return PM_E_INVALID_PARAM;
    }

    g_pm_ds_sub_refer_before_fn[ds_id] = before_fn;
    g_pm_ds_sub_refer_after_fn[ds_id] = after_fn;
    
    return PM_E_NONE;
}

int32
cdb_store_data(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node, ctc_msg_t *p_msg)
{
    void *p_left = NULL;
    cdb_sync_refer_t *p_refer = NULL;
    void *p_tbl = NULL;
    void *p_ds = NULL;
    uint32 oper = 0;
    uint32 field_id = 0;
    uint32 ds2_data_size = 0;
    int32 rc = PM_E_NONE;

    /* 1. build and check */
    oper =  p_msg->u_hdr.data.oper;
    field_id = p_msg->u_hdr.data.fields[0];

    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        if (p_ds_node)
        {
            if (p_ds2_node)
            {
                ds2_data_size = p_ds2_node->tbl_info->key_size;
            }
            p_tbl = p_msg->data;
            p_ds = p_msg->data + p_tbl_node->tbl_info->key_size;
            p_left = p_msg->data + p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->key_size + ds2_data_size;
            p_refer = p_left;
        }
        else
        {
            p_tbl = p_msg->data;
            p_left = p_msg->data + p_tbl_node->tbl_info->key_size;
            p_refer = p_left;
        }
    }
    else
    {
        if (p_ds_node)
        {
            if (p_ds2_node)
            {
                SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size + p_ds2_node->tbl_info->data_size == p_msg->data_len);
            }
            else
            {
                SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size == p_msg->data_len);
            }
            p_tbl = p_msg->data;
            p_ds = p_msg->data + p_tbl_node->tbl_info->key_size;
        }
        else
        {
            p_tbl = p_msg->data;
        }
    }

    /* 2. before store sync process */
    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        if (p_ds_node)
        {
            if (g_pm_ds_sub_refer_before_fn[p_ds_node->id])
            {
                g_pm_ds_sub_refer_before_fn[p_ds_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds, p_refer);
            }
        }
        else
        {
            if (g_pm_tbl_sub_refer_before_fn[p_tbl_node->id])
            {
                g_pm_tbl_sub_refer_before_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds, p_refer);
            }
        }
    }
    else
    {
        if (g_pm_tbl_sub_before_fn[p_tbl_node->id])
        {
            g_pm_tbl_sub_before_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds);
        }
    }
    
    /* 3. store CDB */
    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        if (p_ds_node)
        {
            rc = cdb_store_ds_refer(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                p_ds_node, p_ds2_node, p_msg->data, p_msg->data_len);
        }
        else
        {
            rc = cdb_store_tbl_refer(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                p_msg->data, p_msg->data_len);
        }    
    }
    else
    {
        if (p_ds_node)
        {
            rc = cdb_store_ds(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, 
                    p_ds_node, p_ds2_node, p_msg->data, p_msg->data_len);
        }
        else
        {
            rc = cdb_store_tbl(p_msg->u_hdr.data.oper, p_msg->u_hdr.data.fields, p_tbl_node, p_msg->data, p_msg->data_len);
        }
    }

    if (rc < 0)
    {
        return rc;
    }

    if (CDB_OPER_SYNC_REFER == p_msg->u_hdr.data.oper)
    {
        if (p_ds_node)
        {
            if (g_pm_ds_sub_refer_after_fn[p_ds_node->id])
            {
                g_pm_ds_sub_refer_after_fn[p_ds_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds, p_refer);
            }
        }
        else
        {
            if (g_pm_tbl_sub_refer_after_fn[p_tbl_node->id])
            {
                g_pm_tbl_sub_refer_after_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds, p_refer);
            }
        }
    }
    else
    {
        /* 4. after store sync process */
        if (g_pm_tbl_sub_after_fn[p_tbl_node->id])
        {
            g_pm_tbl_sub_after_fn[p_tbl_node->id](p_tbl_node, p_ds_node, oper, field_id, p_tbl, p_ds);
        }
    }
    
    return PM_E_NONE;
}

int32
cdb_build_refer_tbl(uint32 tbl_id, uint32 add, cdb_sync_refer_hdr_t *p_refer)
{
    cdb_node_t* p_node_tbl = cdb_get_tbl(tbl_id);
    p_refer->type = CDB_REFER_TBL;
    p_refer->id   = tbl_id;
    p_refer->oper = (add) ? CDB_REFER_OPER_ADD : CDB_REFER_OPER_DEL;
    p_refer->size = p_node_tbl->tbl_info->key_size;

    return PM_E_NONE;
}

int32
cdb_get_product_type()
{
    return g_product_type;
}
