
#include "proto.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "cdb_data_cmp.h"

tbl_cpu_utilization_master_t *_g_p_tbl_cpu_utilization_master = NULL;

static uint32
_tbl_cpu_utilization_hash_make(tbl_cpu_utilization_t *p_cpu_utilize)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_cpu_utilize->key;
    for (index = 0; index < sizeof(p_cpu_utilize->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_cpu_utilization_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_cpu_utilization_t *p_cpu_utilize1 = (tbl_cpu_utilization_t*)p_arg1;
    tbl_cpu_utilization_t *p_cpu_utilize2 = (tbl_cpu_utilization_t*)p_arg2;

    if (0 == sal_memcmp(&p_cpu_utilize1->key, &p_cpu_utilize2->key, sizeof(tbl_cpu_utilization_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_cpu_utilization_add_cpu_utilization_sync(tbl_cpu_utilization_t *p_cpu_utilize, uint32 sync)
{
    tbl_cpu_utilization_master_t *p_master = _g_p_tbl_cpu_utilization_master;
    tbl_cpu_utilization_t *p_db_cpu_utilize = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_cpu_utilization_get_cpu_utilization(&p_cpu_utilize->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cpu_utilize = XCALLOC(MEM_TBL_CPU_UTILIZATION, sizeof(tbl_cpu_utilization_t));
    if (NULL == p_db_cpu_utilize)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cpu_utilize, p_cpu_utilize, sizeof(tbl_cpu_utilization_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->cpu_utilize_hash, (void*)p_db_cpu_utilize, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->cpu_utilize_list, p_db_cpu_utilize);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CPU_UTILIZATION, p_db_cpu_utilize);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CPU_UTILIZATION, p_db_cpu_utilize);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_utilization_del_cpu_utilization_sync(tbl_cpu_utilization_key_t *p_cpu_utilize_key, uint32 sync)
{
    tbl_cpu_utilization_master_t *p_master = _g_p_tbl_cpu_utilization_master;
    tbl_cpu_utilization_t *p_db_cpu_utilize = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_utilize = tbl_cpu_utilization_get_cpu_utilization(p_cpu_utilize_key);
    if (NULL == p_db_cpu_utilize)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CPU_UTILIZATION, p_db_cpu_utilize);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CPU_UTILIZATION, p_db_cpu_utilize);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->cpu_utilize_hash, (void*)p_db_cpu_utilize);
    ctclib_slistnode_delete(p_master->cpu_utilize_list, p_db_cpu_utilize);

    /* 4. free db entry */
    XFREE(MEM_TBL_CPU_UTILIZATION, p_db_cpu_utilize);

    return PM_E_NONE;
}

int32
tbl_cpu_utilization_set_cpu_utilization_field_sync(tbl_cpu_utilization_t *p_cpu_utilize, tbl_cpu_utilization_field_id_t field_id, uint32 sync)
{
    tbl_cpu_utilization_t *p_db_cpu_utilize = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_utilize = tbl_cpu_utilization_get_cpu_utilization(&p_cpu_utilize->key);
    if (NULL == p_db_cpu_utilize)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_UTILIZATION_FLD_CPU_USAGE:
        p_db_cpu_utilize->cpu_usage = p_cpu_utilize->cpu_usage;
        break;

    case TBL_CPU_UTILIZATION_FLD_MAX:
        sal_memcpy(p_db_cpu_utilize, p_cpu_utilize, sizeof(tbl_cpu_utilization_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_UTILIZATION, field_id, p_db_cpu_utilize);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_UTILIZATION, field_id, p_db_cpu_utilize);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_utilization_add_cpu_utilization(tbl_cpu_utilization_t *p_cpu_utilize)
{
    return tbl_cpu_utilization_add_cpu_utilization_sync(p_cpu_utilize, TRUE);
}

int32
tbl_cpu_utilization_del_cpu_utilization(tbl_cpu_utilization_key_t *p_cpu_utilize_key)
{
    return tbl_cpu_utilization_del_cpu_utilization_sync(p_cpu_utilize_key, TRUE);
}

int32
tbl_cpu_utilization_set_cpu_utilization_field(tbl_cpu_utilization_t *p_cpu_utilize, tbl_cpu_utilization_field_id_t field_id)
{
    return tbl_cpu_utilization_set_cpu_utilization_field_sync(p_cpu_utilize, field_id, TRUE);
}

tbl_cpu_utilization_t*
tbl_cpu_utilization_get_cpu_utilization(tbl_cpu_utilization_key_t *p_cpu_utilize_key)
{
    tbl_cpu_utilization_master_t *p_master = _g_p_tbl_cpu_utilization_master;
    tbl_cpu_utilization_t lkp;

    sal_memcpy(&lkp.key, p_cpu_utilize_key, sizeof(tbl_cpu_utilization_key_t));
    return ctclib_hash_lookup(p_master->cpu_utilize_hash, &lkp);
}

char*
tbl_cpu_utilization_key_val2str(tbl_cpu_utilization_t *p_cpu_utilize, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_cpu_utilize->key.name);
    return str;
}

int32
tbl_cpu_utilization_key_str2val(char *str, tbl_cpu_utilization_t *p_cpu_utilize)
{
    int32 ret = 0;

    sal_strcpy(p_cpu_utilize->key.name, str);
    return ret;
}

char*
tbl_cpu_utilization_key_name_dump(tbl_cpu_utilization_t *p_cpu_utilize, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_UTILIZATION);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_cpu_utilize->key.name);
    return str;
}

char*
tbl_cpu_utilization_key_value_dump(tbl_cpu_utilization_t *p_cpu_utilize, char *str)
{
    sal_sprintf(str, "%s", p_cpu_utilize->key.name);
    return str;
}

char*
tbl_cpu_utilization_field_name_dump(tbl_cpu_utilization_t *p_cpu_utilize, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_UTILIZATION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_UTILIZATION_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_utilization_field_value_dump(tbl_cpu_utilization_t *p_cpu_utilize, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_cpu_utilize->key.name);
    }
    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_CPU_USAGE, field_id))
    {
        sal_sprintf(str, "%f",
            p_cpu_utilize->cpu_usage);
    }
    return str;
}

char**
tbl_cpu_utilization_table_dump(tbl_cpu_utilization_t *p_cpu_utilize, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_utilization_key_name_dump(p_cpu_utilize, buf));
    for(i=1; i<TBL_CPU_UTILIZATION_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_utilization_field_name_dump(p_cpu_utilize, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_utilization_field_value_dump(p_cpu_utilize, i, buf));
    }
    return str;
}

int32
tbl_cpu_utilization_field_value_parser(char *str, int32 field_id, tbl_cpu_utilization_t *p_cpu_utilize)
{
    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_KEY, field_id))
    {
        sal_strcpy(p_cpu_utilize->key.name, str);
    }
    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_CPU_USAGE, field_id))
    {
        int32 ret;
        p_cpu_utilize->cpu_usage = cdb_double_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_utilization_table_parser(char** array, char* key_value,tbl_cpu_utilization_t *p_cpu_utilize)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_UTILIZATION);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_utilization_key_str2val(key_value, p_cpu_utilize));

    for(i=1; i<TBL_CPU_UTILIZATION_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_utilization_field_value_parser( array[j++], i, p_cpu_utilize));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_utilization_dump_one(tbl_cpu_utilization_t *p_cpu_utilize, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_UTILIZATION);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_UTILIZATION_FLD_KEY].name,
            p_cpu_utilize->key.name);
    }
    if (FLD_MATCH(TBL_CPU_UTILIZATION_FLD_CPU_USAGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%f", p_tbl_info->field[TBL_CPU_UTILIZATION_FLD_CPU_USAGE].name,
            p_cpu_utilize->cpu_usage);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_utilization_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_utilization_master_t *p_master = _g_p_tbl_cpu_utilization_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_cpu_utilization_t *p_db_cpu_utilize = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->cpu_utilize_list, p_db_cpu_utilize, listnode, next)
    {
        rc |= fn(p_db_cpu_utilize, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_utilization_master_t*
tbl_cpu_utilization_get_master()
{
    return _g_p_tbl_cpu_utilization_master;
}

tbl_cpu_utilization_master_t*
tbl_cpu_utilization_init_cpu_utilization()
{
    _g_p_tbl_cpu_utilization_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cpu_utilization_master_t));
    _g_p_tbl_cpu_utilization_master->cpu_utilize_hash = ctclib_hash_create(_tbl_cpu_utilization_hash_make, _tbl_cpu_utilization_hash_cmp);
    _g_p_tbl_cpu_utilization_master->cpu_utilize_list = ctclib_slist_create(tbl_cpu_utilization_cmp, NULL);
    return _g_p_tbl_cpu_utilization_master;
}

