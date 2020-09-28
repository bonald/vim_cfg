
#include "proto.h"
#include "gen/tbl_login_record_define.h"
#include "gen/tbl_login_record.h"
#include "cdb_data_cmp.h"

tbl_login_record_master_t *_g_p_tbl_login_record_master = NULL;

int32
tbl_login_record_add_login_record_sync(tbl_login_record_t *p_login_record, uint32 sync)
{
    tbl_login_record_master_t *p_master = _g_p_tbl_login_record_master;
    tbl_login_record_t *p_db_login_record = NULL;

    /* 1. lookup entry exist */
    if (tbl_login_record_get_login_record(&p_login_record->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_login_record = XCALLOC(MEM_TBL_LOGIN_RECORD, sizeof(tbl_login_record_t));
    if (NULL == p_db_login_record)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_login_record, p_login_record, sizeof(tbl_login_record_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->user_list, p_db_login_record);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LOGIN_RECORD, p_db_login_record);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_LOGIN_RECORD, p_db_login_record);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_login_record_del_login_record_sync(tbl_login_record_key_t *p_login_record_key, uint32 sync)
{
    tbl_login_record_master_t *p_master = _g_p_tbl_login_record_master;
    tbl_login_record_t *p_db_login_record = NULL;

    /* 1. lookup entry exist */
    p_db_login_record = tbl_login_record_get_login_record(p_login_record_key);
    if (NULL == p_db_login_record)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_LOGIN_RECORD, p_db_login_record);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_LOGIN_RECORD, p_db_login_record);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->user_list, p_db_login_record);

    /* 4. free db entry */
    XFREE(MEM_TBL_LOGIN_RECORD, p_db_login_record);

    return PM_E_NONE;
}

int32
tbl_login_record_set_login_record_field_sync(tbl_login_record_t *p_login_record, tbl_login_record_field_id_t field_id, uint32 sync)
{
    tbl_login_record_t *p_db_login_record = NULL;

    /* 1. lookup entry exist */
    p_db_login_record = tbl_login_record_get_login_record(&p_login_record->key);
    if (NULL == p_db_login_record)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_LOGIN_RECORD_FLD_LOCKED:
        p_db_login_record->locked = p_login_record->locked;
        break;

    case TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS:
        p_db_login_record->first_fail_ts = p_login_record->first_fail_ts;
        break;

    case TBL_LOGIN_RECORD_FLD_LOCK_TS:
        p_db_login_record->lock_ts = p_login_record->lock_ts;
        break;

    case TBL_LOGIN_RECORD_FLD_FAIL_COUNT:
        p_db_login_record->fail_count = p_login_record->fail_count;
        break;

    case TBL_LOGIN_RECORD_FLD_INVALID_USER:
        p_db_login_record->invalid_user = p_login_record->invalid_user;
        break;

    case TBL_LOGIN_RECORD_FLD_MAX:
        sal_memcpy(p_db_login_record, p_login_record, sizeof(tbl_login_record_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_LOGIN_RECORD, field_id, p_db_login_record);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_LOGIN_RECORD, field_id, p_db_login_record);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_login_record_add_login_record(tbl_login_record_t *p_login_record)
{
    return tbl_login_record_add_login_record_sync(p_login_record, TRUE);
}

int32
tbl_login_record_del_login_record(tbl_login_record_key_t *p_login_record_key)
{
    return tbl_login_record_del_login_record_sync(p_login_record_key, TRUE);
}

int32
tbl_login_record_set_login_record_field(tbl_login_record_t *p_login_record, tbl_login_record_field_id_t field_id)
{
    return tbl_login_record_set_login_record_field_sync(p_login_record, field_id, TRUE);
}

tbl_login_record_t*
tbl_login_record_get_login_record(tbl_login_record_key_t *p_login_record_key)
{
    tbl_login_record_master_t *p_master = _g_p_tbl_login_record_master;
    tbl_login_record_t lkp;

    sal_memcpy(&lkp.key, p_login_record_key, sizeof(tbl_login_record_key_t));
    return ctclib_slistdata_lookup(p_master->user_list, &lkp);
}

char*
tbl_login_record_key_val2str(tbl_login_record_t *p_login_record, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_login_record->key.name);
    return str;
}

int32
tbl_login_record_key_str2val(char *str, tbl_login_record_t *p_login_record)
{
    int32 ret = 0;

    sal_strcpy(p_login_record->key.name, str);
    return ret;
}

char*
tbl_login_record_key_name_dump(tbl_login_record_t *p_login_record, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_RECORD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_login_record->key.name);
    return str;
}

char*
tbl_login_record_key_value_dump(tbl_login_record_t *p_login_record, char *str)
{
    sal_sprintf(str, "%s", p_login_record->key.name);
    return str;
}

char*
tbl_login_record_field_name_dump(tbl_login_record_t *p_login_record, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_RECORD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_LOGIN_RECORD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_login_record_field_value_dump(tbl_login_record_t *p_login_record, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_login_record->key.name);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCKED, field_id))
    {
        sal_sprintf(str, "%u", p_login_record->locked);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS, field_id))
    {
        sal_sprintf(str, "%u", p_login_record->first_fail_ts);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCK_TS, field_id))
    {
        sal_sprintf(str, "%u", p_login_record->lock_ts);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FAIL_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_login_record->fail_count);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_INVALID_USER, field_id))
    {
        sal_sprintf(str, "%u", p_login_record->invalid_user);
    }
    return str;
}

char**
tbl_login_record_table_dump(tbl_login_record_t *p_login_record, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_login_record_key_name_dump(p_login_record, buf));
    for(i=1; i<TBL_LOGIN_RECORD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_login_record_field_name_dump(p_login_record, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_login_record_field_value_dump(p_login_record, i, buf));
    }
    return str;
}

int32
tbl_login_record_field_value_parser(char *str, int32 field_id, tbl_login_record_t *p_login_record)
{
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_KEY, field_id))
    {
        sal_strcpy(p_login_record->key.name, str);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCKED, field_id))
    {
        int32 ret;
        p_login_record->locked = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS, field_id))
    {
        int32 ret;
        p_login_record->first_fail_ts = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCK_TS, field_id))
    {
        int32 ret;
        p_login_record->lock_ts = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FAIL_COUNT, field_id))
    {
        int32 ret;
        p_login_record->fail_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_INVALID_USER, field_id))
    {
        int32 ret;
        p_login_record->invalid_user = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_login_record_table_parser(char** array, char* key_value,tbl_login_record_t *p_login_record)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_RECORD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_login_record_key_str2val(key_value, p_login_record));

    for(i=1; i<TBL_LOGIN_RECORD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_login_record_field_value_parser( array[j++], i, p_login_record));
    }

    return PM_E_NONE;
}

int32
tbl_login_record_dump_one(tbl_login_record_t *p_login_record, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOGIN_RECORD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_KEY].name,
            p_login_record->key.name);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCKED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_LOCKED].name,
            p_login_record->locked);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_FIRST_FAIL_TS].name,
            p_login_record->first_fail_ts);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_LOCK_TS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_LOCK_TS].name,
            p_login_record->lock_ts);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_FAIL_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_FAIL_COUNT].name,
            p_login_record->fail_count);
    }
    if (FLD_MATCH(TBL_LOGIN_RECORD_FLD_INVALID_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOGIN_RECORD_FLD_INVALID_USER].name,
            p_login_record->invalid_user);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_login_record_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_login_record_master_t *p_master = _g_p_tbl_login_record_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_login_record_t *p_db_login_record = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->user_list, p_db_login_record, listnode, next)
    {
        rc |= fn(p_db_login_record, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_login_record_master_t*
tbl_login_record_get_master()
{
    return _g_p_tbl_login_record_master;
}

tbl_login_record_master_t*
tbl_login_record_init_login_record()
{
    _g_p_tbl_login_record_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_login_record_master_t));
    _g_p_tbl_login_record_master->user_list = ctclib_slist_create(tbl_login_record_cmp, NULL);
    return _g_p_tbl_login_record_master;
}

