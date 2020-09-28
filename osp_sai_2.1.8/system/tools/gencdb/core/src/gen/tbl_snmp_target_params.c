
#include "proto.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "cdb_data_cmp.h"

tbl_snmp_target_params_master_t *_g_p_tbl_snmp_target_params_master = NULL;

static uint32
_tbl_snmp_target_params_hash_make(tbl_snmp_target_params_t *p_snmp_target_params)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_snmp_target_params->key;
    for (index = 0; index < sizeof(p_snmp_target_params->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_snmp_target_params_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_snmp_target_params_t *p_snmp_target_params1 = (tbl_snmp_target_params_t*)p_arg1;
    tbl_snmp_target_params_t *p_snmp_target_params2 = (tbl_snmp_target_params_t*)p_arg2;

    if (0 == sal_memcmp(&p_snmp_target_params1->key, &p_snmp_target_params2->key, sizeof(tbl_snmp_target_params_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_snmp_target_params_add_snmp_target_params_sync(tbl_snmp_target_params_t *p_snmp_target_params, uint32 sync)
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    tbl_snmp_target_params_t *p_db_snmp_target_params = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_snmp_target_params_get_snmp_target_params(&p_snmp_target_params->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_snmp_target_params = XCALLOC(MEM_TBL_SNMP_TARGET_PARAMS, sizeof(tbl_snmp_target_params_t));
    if (NULL == p_db_snmp_target_params)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_snmp_target_params, p_snmp_target_params, sizeof(tbl_snmp_target_params_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->snmp_target_params_hash, (void*)p_db_snmp_target_params, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->snmp_target_params_list, p_db_snmp_target_params);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_SNMP_TARGET_PARAMS, p_db_snmp_target_params);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_SNMP_TARGET_PARAMS, p_db_snmp_target_params);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_params_del_snmp_target_params_sync(tbl_snmp_target_params_key_t *p_snmp_target_params_key, uint32 sync)
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    tbl_snmp_target_params_t *p_db_snmp_target_params = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_target_params = tbl_snmp_target_params_get_snmp_target_params(p_snmp_target_params_key);
    if (NULL == p_db_snmp_target_params)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_SNMP_TARGET_PARAMS, p_db_snmp_target_params);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_SNMP_TARGET_PARAMS, p_db_snmp_target_params);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->snmp_target_params_hash, (void*)p_db_snmp_target_params);
    ctclib_slistnode_delete(p_master->snmp_target_params_list, p_db_snmp_target_params);

    /* 4. free db entry */
    XFREE(MEM_TBL_SNMP_TARGET_PARAMS, p_db_snmp_target_params);

    return PM_E_NONE;
}

int32
tbl_snmp_target_params_set_snmp_target_params_field_sync(tbl_snmp_target_params_t *p_snmp_target_params, tbl_snmp_target_params_field_id_t field_id, uint32 sync)
{
    tbl_snmp_target_params_t *p_db_snmp_target_params = NULL;

    /* 1. lookup entry exist */
    p_db_snmp_target_params = tbl_snmp_target_params_get_snmp_target_params(&p_snmp_target_params->key);
    if (NULL == p_db_snmp_target_params)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SNMP_TARGET_PARAMS_FLD_FLAGS:
        p_db_snmp_target_params->flags = p_snmp_target_params->flags;
        break;

    case TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME:
        sal_memcpy(p_db_snmp_target_params->user_name, p_snmp_target_params->user_name, sizeof(p_snmp_target_params->user_name));
        break;

    case TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL:
        p_db_snmp_target_params->usm_level = p_snmp_target_params->usm_level;
        break;

    case TBL_SNMP_TARGET_PARAMS_FLD_MAX:
        sal_memcpy(p_db_snmp_target_params, p_snmp_target_params, sizeof(tbl_snmp_target_params_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SNMP_TARGET_PARAMS, field_id, p_db_snmp_target_params);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SNMP_TARGET_PARAMS, field_id, p_db_snmp_target_params);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_params_add_snmp_target_params(tbl_snmp_target_params_t *p_snmp_target_params)
{
    return tbl_snmp_target_params_add_snmp_target_params_sync(p_snmp_target_params, TRUE);
}

int32
tbl_snmp_target_params_del_snmp_target_params(tbl_snmp_target_params_key_t *p_snmp_target_params_key)
{
    return tbl_snmp_target_params_del_snmp_target_params_sync(p_snmp_target_params_key, TRUE);
}

int32
tbl_snmp_target_params_set_snmp_target_params_field(tbl_snmp_target_params_t *p_snmp_target_params, tbl_snmp_target_params_field_id_t field_id)
{
    return tbl_snmp_target_params_set_snmp_target_params_field_sync(p_snmp_target_params, field_id, TRUE);
}

tbl_snmp_target_params_t*
tbl_snmp_target_params_get_snmp_target_params(tbl_snmp_target_params_key_t *p_snmp_target_params_key)
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    tbl_snmp_target_params_t lkp;

    sal_memcpy(&lkp.key, p_snmp_target_params_key, sizeof(tbl_snmp_target_params_key_t));
    return ctclib_hash_lookup(p_master->snmp_target_params_hash, &lkp);
}

char*
tbl_snmp_target_params_key_val2str(tbl_snmp_target_params_t *p_snmp_target_params, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_snmp_target_params->key.params_name);
    return str;
}

int32
tbl_snmp_target_params_key_str2val(char *str, tbl_snmp_target_params_t *p_snmp_target_params)
{
    int32 ret = 0;

    sal_strcpy(p_snmp_target_params->key.params_name, str);
    return ret;
}

char*
tbl_snmp_target_params_key_name_dump(tbl_snmp_target_params_t *p_snmp_target_params, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_PARAMS);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_snmp_target_params->key.params_name);
    return str;
}

char*
tbl_snmp_target_params_key_value_dump(tbl_snmp_target_params_t *p_snmp_target_params, char *str)
{
    sal_sprintf(str, "%s", p_snmp_target_params->key.params_name);
    return str;
}

char*
tbl_snmp_target_params_field_name_dump(tbl_snmp_target_params_t *p_snmp_target_params, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_PARAMS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SNMP_TARGET_PARAMS_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_snmp_target_params_field_value_dump(tbl_snmp_target_params_t *p_snmp_target_params, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_target_params->key.params_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_FLAGS, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_params->flags);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_snmp_target_params->user_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL, field_id))
    {
        sal_sprintf(str, "%u", p_snmp_target_params->usm_level);
    }
    return str;
}

char**
tbl_snmp_target_params_table_dump(tbl_snmp_target_params_t *p_snmp_target_params, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_snmp_target_params_key_name_dump(p_snmp_target_params, buf));
    for(i=1; i<TBL_SNMP_TARGET_PARAMS_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_snmp_target_params_field_name_dump(p_snmp_target_params, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_snmp_target_params_field_value_dump(p_snmp_target_params, i, buf));
    }
    return str;
}

int32
tbl_snmp_target_params_field_value_parser(char *str, int32 field_id, tbl_snmp_target_params_t *p_snmp_target_params)
{
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_KEY, field_id))
    {
        sal_strcpy(p_snmp_target_params->key.params_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_FLAGS, field_id))
    {
        int32 ret;
        p_snmp_target_params->flags = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME, field_id))
    {
        sal_strcpy(p_snmp_target_params->user_name, str);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL, field_id))
    {
        int32 ret;
        p_snmp_target_params->usm_level = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_snmp_target_params_table_parser(char** array, char* key_value,tbl_snmp_target_params_t *p_snmp_target_params)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_PARAMS);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_snmp_target_params_key_str2val(key_value, p_snmp_target_params));

    for(i=1; i<TBL_SNMP_TARGET_PARAMS_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_snmp_target_params_field_value_parser( array[j++], i, p_snmp_target_params));
    }

    return PM_E_NONE;
}

int32
tbl_snmp_target_params_dump_one(tbl_snmp_target_params_t *p_snmp_target_params, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SNMP_TARGET_PARAMS);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_PARAMS_FLD_KEY].name,
            p_snmp_target_params->key.params_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_FLAGS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_PARAMS_FLD_FLAGS].name,
            p_snmp_target_params->flags);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SNMP_TARGET_PARAMS_FLD_USER_NAME].name,
            p_snmp_target_params->user_name);
    }
    if (FLD_MATCH(TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SNMP_TARGET_PARAMS_FLD_USM_LEVEL].name,
            p_snmp_target_params->usm_level);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_snmp_target_params_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_target_params_t *p_db_snmp_target_params = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_target_params_list, p_db_snmp_target_params, listnode, next)
    {
        rc |= fn(p_db_snmp_target_params, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_snmp_target_params_master_t*
tbl_snmp_target_params_get_master()
{
    return _g_p_tbl_snmp_target_params_master;
}

tbl_snmp_target_params_master_t*
tbl_snmp_target_params_init_snmp_target_params()
{
    _g_p_tbl_snmp_target_params_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_snmp_target_params_master_t));
    _g_p_tbl_snmp_target_params_master->snmp_target_params_hash = ctclib_hash_create(_tbl_snmp_target_params_hash_make, _tbl_snmp_target_params_hash_cmp);
    _g_p_tbl_snmp_target_params_master->snmp_target_params_list = ctclib_slist_create(tbl_snmp_target_params_cmp, NULL);
    return _g_p_tbl_snmp_target_params_master;
}

int32
tbl_snmp_target_params_num_get()
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_target_params_list->count;
    return count;
}



