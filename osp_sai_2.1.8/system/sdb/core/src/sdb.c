
#include "sdb.h"

#include "gen/sdb_inc_priv.h"
#include "gen/sdb_dir_priv.h"
#include "gen/sdb_tbl_field_priv.h"
#include "gen/sdb_tbl_info_priv.h"
#include "gen/sdb_tbl_priv.h"

#define SDB_CDS_ROOT sdb_get_dir(SDIR_SDB)

int32
sdb_init_db_one(sdb_node_t *p_node)
{
    uint32 i = 0;
    sdb_node_t *p_sub = NULL;
    
    ctclib_list_init(&p_node->sub);

    for (i = 0; i < SDIR_MAX; i++)
    {
        p_sub = &g_sdb_dir_node_info[i];
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

    for (i = 0; i < STBL_MAX; i++)
    {
        p_sub = &g_sdb_tbl_node_info[i];
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
    
    return SDB_E_NONE;
}

int32
sdb_init(uint32 pm_id)
{
    uint32 i = 0;
    sdb_node_t *p_node = NULL;
   
    for (i = 0; i < SDIR_MAX; i++)
    {
        p_node = &g_sdb_dir_node_info[i];
        if (!p_node)
        {
            continue;
        }
        sdb_init_db_one(p_node);
    }

    for (i = 0; i < STBL_MAX; i++)
    {
        p_node = &g_sdb_tbl_node_info[i];
        if (!p_node)
        {
            continue;
        }
        sdb_init_db_one(p_node);
    }
    
    sdb_tbl_pm_init(pm_id);
    return SDB_E_NONE;
}

sdb_node_t*
sdb_get_dir(uint16 id)
{
    if (id >= SDIR_MAX)
    {
        return NULL;
    }
    return &g_sdb_dir_node_info[id];
}

sdb_node_t*
sdb_get_tbl(uint16 id)
{
    if (id >= STBL_MAX)
    {
        return NULL;
    }
    return &g_sdb_tbl_node_info[id];
}

sdb_node_t*
sdb_get_tbl_by_name(char *name)
{
    sdb_node_t *p_node = NULL;
    uint32 i = 0;

    for (i = 0; i < STBL_MAX; i++)
    {
        p_node = &(g_sdb_tbl_node_info[i]);
        if (0 == sal_strcmp(name, p_node->name))
        {
            return p_node;
        }

    }
    
    return NULL;
}

const char*
sdb_node_type_str(uint32 type)
{
    switch(type)
    {
    case SDB_NODE_DIR:
        return "dir";

    case SDB_NODE_TBL:
        return "table";

    default :
        return "invalid";
    }
}

int32
sdb_get_field_id(char *field_name, sdb_node_t* p_node, sdb_field_t **pp_field)
{
    sdb_tbl_info_t *p_tbl_info = NULL;
    int32 fld_id = -1;

    if (NULL == field_name)
    {
        return SDB_E_FAIL;
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
    return SDB_E_FAIL;
}

sdb_node_t*
sdb_get_node_by_id(uint16 type, uint16 id)
{
    switch (type)
    {
    case SDB_NODE_DIR:
        return sdb_get_dir(id);
    case SDB_NODE_TBL:
        return sdb_get_tbl(id);
    default:
        return NULL;
    }
}

/* bypass "" sub string */
char*
sdb_cfg_strchr(char *s, char c)
{
    char *p = s;
    uint32 quotation_count = 0;

    while (CMD_ZERO_CHAR != *p) 
    { 
        if (CMD_QUOTATION_CHAR == *p)
        {
            quotation_count++;
        }

        /* bypass '/' in the pair of " " */
        if (0 == (quotation_count % 2))
        {
            if (c == *p)  
            {
                return p;
            }
        }
        p++;
    }

    return NULL;
}

#define SDB_MAX_DEPTH            256
#define SDB_MAX_PATH_LEN         1024

typedef struct
{
    char       *path;
    char       *argv[SDB_MAX_DEPTH];
    uint32      argc;
    char        buf[SDB_MAX_PATH_LEN];   /* buffer for command need append or changed */
} sdb_cfg_parse_info_t;

int32
sdb_node_parse(const char *path, sdb_cfg_parse_info_t *p_info)
{
    uint32          line_strlen = 0;
    const char     *start = NULL;
    char           *pos = NULL;
    char           *line = NULL;
    
    if (!path)
    {
        return SDB_E_NONE;
    }

    sal_memset(p_info, 0, sizeof(sdb_cfg_parse_info_t));
    if (CMD_DIVISION_CHAR == path[0])
    {
        start = path + 1;
    }
    else
    {
        start = path;
    }
    sal_strncpy(p_info->buf, start, SDB_MAX_PATH_LEN);
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

    pos = sdb_cfg_strchr(line, CMD_DIVISION_CHAR);
    if (!pos)
    {
        return SDB_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = sdb_cfg_strchr(pos + 1, CMD_DIVISION_CHAR);
    }
    return SDB_E_NONE;
}

sdb_node_t*
sdb_get_node_by_name(const char *path)
{
    sdb_cfg_parse_info_t info;
    sdb_node_t *p_node = NULL;
    sdb_node_t *p_sub = NULL;
    ctclib_list_node_t* p_listnode = NULL;
    uint32 depth = 0;
    uint32 match = FALSE;
    uint32 sub_count = 0;
    if (!path)
    {
        return SDB_CDS_ROOT;
    }
    sdb_node_parse(path, &info);

    if (!info.argc)
    {
        return NULL;
    }

    p_node = SDB_CDS_ROOT;
    if (sal_strcmp(info.argv[0], p_node->name))
    {
        return NULL;
    }
    
    for (depth = 1; depth < info.argc; depth++)
    {
        match = FALSE;
        ctclib_list_for_each(p_listnode, &p_node->sub)
        {
            p_sub = ctclib_container_of(p_listnode, sdb_node_t, node);
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

        if (SDB_NODE_TBL == p_node->type || SDB_NODE_ACT == p_node->type)
        {
            depth++;
            break;
        }
    }

    return p_node;
}

int32
sdb_register_cfg_tbl_cb(uint16 tbl_id, SCMD_FUNC cb)
{
    sdb_node_t* p_node = NULL;
    
    p_node = sdb_get_tbl(tbl_id);
    if (NULL == p_node)
    {
        return SDB_E_FAIL;
    }

    p_node->tbl_info->ops.cmd_fn = cb;
    
    return SDB_E_NONE;
}

int32
stbl_hash_iter_adpt_fn(ctclib_hash_backet_t* phb, void *parg1, void *p_arg2)
{
    int32 rc = SDB_E_NONE;
    
    STBL_ITER_CB_FUNC fn = (STBL_ITER_CB_FUNC)parg1;
    stbl_iter_args_t *pargs = (stbl_iter_args_t *)p_arg2;

    rc = fn(phb->data, pargs);
    return rc;
}

const char*
sdb_enum_val2str(const char **array, uint32 array_size, uint32 value)
{
    if (value >= array_size)
    {
        return "invalid";
    }
    return array[value];
}
