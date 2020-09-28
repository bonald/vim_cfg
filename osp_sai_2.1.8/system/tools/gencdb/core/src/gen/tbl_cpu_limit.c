
#include "proto.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "cdb_data_cmp.h"

tbl_cpu_limit_master_t *_g_p_tbl_cpu_limit_master = NULL;

int32
tbl_cpu_limit_add_cpu_limit_sync(tbl_cpu_limit_t *p_cpu_limit, uint32 sync)
{
    tbl_cpu_limit_master_t *p_master = _g_p_tbl_cpu_limit_master;
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;

    /* 1. lookup entry exist */
    if (tbl_cpu_limit_get_cpu_limit(&p_cpu_limit->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cpu_limit = XCALLOC(MEM_TBL_CPU_LIMIT, sizeof(tbl_cpu_limit_t));
    if (NULL == p_db_cpu_limit)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cpu_limit, p_cpu_limit, sizeof(tbl_cpu_limit_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->cpu_limit_list, p_db_cpu_limit);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CPU_LIMIT, p_db_cpu_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CPU_LIMIT, p_db_cpu_limit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_limit_del_cpu_limit_sync(tbl_cpu_limit_key_t *p_cpu_limit_key, uint32 sync)
{
    tbl_cpu_limit_master_t *p_master = _g_p_tbl_cpu_limit_master;
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_limit = tbl_cpu_limit_get_cpu_limit(p_cpu_limit_key);
    if (NULL == p_db_cpu_limit)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CPU_LIMIT, p_db_cpu_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CPU_LIMIT, p_db_cpu_limit);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->cpu_limit_list, p_db_cpu_limit);

    /* 4. free db entry */
    XFREE(MEM_TBL_CPU_LIMIT, p_db_cpu_limit);

    return PM_E_NONE;
}

int32
tbl_cpu_limit_set_cpu_limit_field_sync(tbl_cpu_limit_t *p_cpu_limit, tbl_cpu_limit_field_id_t field_id, uint32 sync)
{
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_limit = tbl_cpu_limit_get_cpu_limit(&p_cpu_limit->key);
    if (NULL == p_db_cpu_limit)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_LIMIT_FLD_PERCENT:
        p_db_cpu_limit->percent = p_cpu_limit->percent;
        break;

    case TBL_CPU_LIMIT_FLD_MAX:
        sal_memcpy(p_db_cpu_limit, p_cpu_limit, sizeof(tbl_cpu_limit_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_LIMIT, field_id, p_db_cpu_limit);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_LIMIT, field_id, p_db_cpu_limit);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_limit_add_cpu_limit(tbl_cpu_limit_t *p_cpu_limit)
{
    return tbl_cpu_limit_add_cpu_limit_sync(p_cpu_limit, TRUE);
}

int32
tbl_cpu_limit_del_cpu_limit(tbl_cpu_limit_key_t *p_cpu_limit_key)
{
    return tbl_cpu_limit_del_cpu_limit_sync(p_cpu_limit_key, TRUE);
}

int32
tbl_cpu_limit_set_cpu_limit_field(tbl_cpu_limit_t *p_cpu_limit, tbl_cpu_limit_field_id_t field_id)
{
    return tbl_cpu_limit_set_cpu_limit_field_sync(p_cpu_limit, field_id, TRUE);
}

tbl_cpu_limit_t*
tbl_cpu_limit_get_cpu_limit(tbl_cpu_limit_key_t *p_cpu_limit_key)
{
    tbl_cpu_limit_master_t *p_master = _g_p_tbl_cpu_limit_master;
    tbl_cpu_limit_t lkp;

    sal_memcpy(&lkp.key, p_cpu_limit_key, sizeof(tbl_cpu_limit_key_t));
    return ctclib_slistdata_lookup(p_master->cpu_limit_list, &lkp);
}

char*
tbl_cpu_limit_key_val2str(tbl_cpu_limit_t *p_cpu_limit, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_cpu_limit->key.name);
    return str;
}

int32
tbl_cpu_limit_key_str2val(char *str, tbl_cpu_limit_t *p_cpu_limit)
{
    int32 ret = 0;

    sal_strcpy(p_cpu_limit->key.name, str);
    return ret;
}

char*
tbl_cpu_limit_key_name_dump(tbl_cpu_limit_t *p_cpu_limit, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_LIMIT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_cpu_limit->key.name);
    return str;
}

char*
tbl_cpu_limit_key_value_dump(tbl_cpu_limit_t *p_cpu_limit, char *str)
{
    sal_sprintf(str, "%s", p_cpu_limit->key.name);
    return str;
}

char*
tbl_cpu_limit_field_name_dump(tbl_cpu_limit_t *p_cpu_limit, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_LIMIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_LIMIT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_limit_field_value_dump(tbl_cpu_limit_t *p_cpu_limit, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_cpu_limit->key.name);
    }
    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_PERCENT, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_limit->percent);
    }
    return str;
}

char**
tbl_cpu_limit_table_dump(tbl_cpu_limit_t *p_cpu_limit, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_limit_key_name_dump(p_cpu_limit, buf));
    for(i=1; i<TBL_CPU_LIMIT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_limit_field_name_dump(p_cpu_limit, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_limit_field_value_dump(p_cpu_limit, i, buf));
    }
    return str;
}

int32
tbl_cpu_limit_field_value_parser(char *str, int32 field_id, tbl_cpu_limit_t *p_cpu_limit)
{
    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_KEY, field_id))
    {
        sal_strcpy(p_cpu_limit->key.name, str);
    }
    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_PERCENT, field_id))
    {
        int32 ret;
        p_cpu_limit->percent = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_limit_table_parser(char** array, char* key_value,tbl_cpu_limit_t *p_cpu_limit)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_LIMIT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_limit_key_str2val(key_value, p_cpu_limit));

    for(i=1; i<TBL_CPU_LIMIT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_limit_field_value_parser( array[j++], i, p_cpu_limit));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_limit_dump_one(tbl_cpu_limit_t *p_cpu_limit, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_LIMIT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_LIMIT_FLD_KEY].name,
            p_cpu_limit->key.name);
    }
    if (FLD_MATCH(TBL_CPU_LIMIT_FLD_PERCENT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_LIMIT_FLD_PERCENT].name,
            p_cpu_limit->percent);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_limit_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_limit_master_t *p_master = _g_p_tbl_cpu_limit_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cpu_limit_list, p_db_cpu_limit, listnode, next)
    {
        rc |= fn(p_db_cpu_limit, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_limit_master_t*
tbl_cpu_limit_get_master()
{
    return _g_p_tbl_cpu_limit_master;
}

tbl_cpu_limit_master_t*
tbl_cpu_limit_init_cpu_limit()
{
    _g_p_tbl_cpu_limit_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cpu_limit_master_t));
    _g_p_tbl_cpu_limit_master->cpu_limit_list = ctclib_slist_create(tbl_cpu_limit_cmp, NULL);
    return _g_p_tbl_cpu_limit_master;
}

