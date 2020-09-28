
#include "proto.h"
#include "gen/tbl_bhm_module_define.h"
#include "gen/tbl_bhm_module.h"
#include "cdb_data_cmp.h"

tbl_bhm_module_master_t *_g_p_tbl_bhm_module_master = NULL;

int32
tbl_bhm_module_add_bhm_module_sync(tbl_bhm_module_t *p_bhm_module, uint32 sync)
{
    tbl_bhm_module_master_t *p_master = _g_p_tbl_bhm_module_master;
    tbl_bhm_module_t *p_db_bhm_module = NULL;

    /* 1. lookup entry exist */
    if (tbl_bhm_module_get_bhm_module(&p_bhm_module->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_bhm_module = XCALLOC(MEM_TBL_BHM_MODULE, sizeof(tbl_bhm_module_t));
    if (NULL == p_db_bhm_module)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_bhm_module, p_bhm_module, sizeof(tbl_bhm_module_t));

    /* 4. add to db */
    p_master->pm_array[p_bhm_module->key.pm_id] = p_db_bhm_module;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_BHM_MODULE, p_db_bhm_module);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_BHM_MODULE, p_db_bhm_module);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bhm_module_del_bhm_module_sync(tbl_bhm_module_key_t *p_bhm_module_key, uint32 sync)
{
    tbl_bhm_module_master_t *p_master = _g_p_tbl_bhm_module_master;
    tbl_bhm_module_t *p_db_bhm_module = NULL;

    /* 1. lookup entry exist */
    p_db_bhm_module = tbl_bhm_module_get_bhm_module(p_bhm_module_key);
    if (NULL == p_db_bhm_module)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_BHM_MODULE, p_db_bhm_module);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_BHM_MODULE, p_db_bhm_module);
        #endif 
    }

    /* 3. delete from db */
    p_master->pm_array[p_bhm_module_key->pm_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_BHM_MODULE, p_db_bhm_module);

    return PM_E_NONE;
}

int32
tbl_bhm_module_set_bhm_module_field_sync(tbl_bhm_module_t *p_bhm_module, tbl_bhm_module_field_id_t field_id, uint32 sync)
{
    tbl_bhm_module_t *p_db_bhm_module = NULL;

    /* 1. lookup entry exist */
    p_db_bhm_module = tbl_bhm_module_get_bhm_module(&p_bhm_module->key);
    if (NULL == p_db_bhm_module)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_BHM_MODULE_FLD_RUNNING:
        p_db_bhm_module->running = p_bhm_module->running;
        break;

    case TBL_BHM_MODULE_FLD_PID:
        p_db_bhm_module->pid = p_bhm_module->pid;
        break;

    case TBL_BHM_MODULE_FLD_DEATH_COUNT:
        p_db_bhm_module->death_count = p_bhm_module->death_count;
        break;

    case TBL_BHM_MODULE_FLD_MAX:
        sal_memcpy(p_db_bhm_module, p_bhm_module, sizeof(tbl_bhm_module_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_BHM_MODULE, field_id, p_db_bhm_module);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_BHM_MODULE, field_id, p_db_bhm_module);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_bhm_module_add_bhm_module(tbl_bhm_module_t *p_bhm_module)
{
    return tbl_bhm_module_add_bhm_module_sync(p_bhm_module, TRUE);
}

int32
tbl_bhm_module_del_bhm_module(tbl_bhm_module_key_t *p_bhm_module_key)
{
    return tbl_bhm_module_del_bhm_module_sync(p_bhm_module_key, TRUE);
}

int32
tbl_bhm_module_set_bhm_module_field(tbl_bhm_module_t *p_bhm_module, tbl_bhm_module_field_id_t field_id)
{
    return tbl_bhm_module_set_bhm_module_field_sync(p_bhm_module, field_id, TRUE);
}

tbl_bhm_module_t*
tbl_bhm_module_get_bhm_module(tbl_bhm_module_key_t *p_bhm_module_key)
{
    tbl_bhm_module_master_t *p_master = _g_p_tbl_bhm_module_master;

    if (PM_ID_EXT_MAX <= p_bhm_module_key->pm_id)
    {
        return NULL;
    }
    return (p_master->pm_array[p_bhm_module_key->pm_id]);
}

char*
tbl_bhm_module_key_val2str(tbl_bhm_module_t *p_bhm_module, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_bhm_module->key.pm_id);
    return str;
}

int32
tbl_bhm_module_key_str2val(char *str, tbl_bhm_module_t *p_bhm_module)
{
    int32 ret = 0;

    p_bhm_module->key.pm_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_bhm_module_key_name_dump(tbl_bhm_module_t *p_bhm_module, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_MODULE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_bhm_module->key.pm_id);
    return str;
}

char*
tbl_bhm_module_key_value_dump(tbl_bhm_module_t *p_bhm_module, char *str)
{
    sal_sprintf(str, "%u", p_bhm_module->key.pm_id);
    return str;
}

char*
tbl_bhm_module_field_name_dump(tbl_bhm_module_t *p_bhm_module, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_MODULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_BHM_MODULE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_bhm_module_field_value_dump(tbl_bhm_module_t *p_bhm_module, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_module->key.pm_id);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_RUNNING, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_module->running);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_PID, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_module->pid);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_DEATH_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_bhm_module->death_count);
    }
    return str;
}

char**
tbl_bhm_module_table_dump(tbl_bhm_module_t *p_bhm_module, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_bhm_module_key_name_dump(p_bhm_module, buf));
    for(i=1; i<TBL_BHM_MODULE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_bhm_module_field_name_dump(p_bhm_module, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_bhm_module_field_value_dump(p_bhm_module, i, buf));
    }
    return str;
}

int32
tbl_bhm_module_field_value_parser(char *str, int32 field_id, tbl_bhm_module_t *p_bhm_module)
{
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_KEY, field_id))
    {
        int32 ret;
        p_bhm_module->key.pm_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_RUNNING, field_id))
    {
        int32 ret;
        p_bhm_module->running = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_PID, field_id))
    {
        int32 ret;
        p_bhm_module->pid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_DEATH_COUNT, field_id))
    {
        int32 ret;
        p_bhm_module->death_count = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_bhm_module_table_parser(char** array, char* key_value,tbl_bhm_module_t *p_bhm_module)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_MODULE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_bhm_module_key_str2val(key_value, p_bhm_module));

    for(i=1; i<TBL_BHM_MODULE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_bhm_module_field_value_parser( array[j++], i, p_bhm_module));
    }

    return PM_E_NONE;
}

int32
tbl_bhm_module_dump_one(tbl_bhm_module_t *p_bhm_module, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_BHM_MODULE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_BHM_MODULE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_MODULE_FLD_KEY].name,
            p_bhm_module->key.pm_id);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_RUNNING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_MODULE_FLD_RUNNING].name,
            p_bhm_module->running);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_PID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_MODULE_FLD_PID].name,
            p_bhm_module->pid);
    }
    if (FLD_MATCH(TBL_BHM_MODULE_FLD_DEATH_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_BHM_MODULE_FLD_DEATH_COUNT].name,
            p_bhm_module->death_count);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_bhm_module_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_bhm_module_master_t *p_master = _g_p_tbl_bhm_module_master;
    tbl_bhm_module_t *p_db_bhm_module = NULL;
    uint32 pm_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (pm_id = 0; pm_id < PM_ID_EXT_MAX; pm_id++)
    {
        p_db_bhm_module = p_master->pm_array[pm_id];
        if (NULL == p_db_bhm_module)
        {
            continue;
        }
        rc |= fn(p_db_bhm_module, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_bhm_module_master_t*
tbl_bhm_module_get_master()
{
    return _g_p_tbl_bhm_module_master;
}

tbl_bhm_module_master_t*
tbl_bhm_module_init_bhm_module()
{
    _g_p_tbl_bhm_module_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_bhm_module_master_t));
    return _g_p_tbl_bhm_module_master;
}

