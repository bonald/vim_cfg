
#include "proto.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "cdb_data_cmp.h"

tbl_account_master_t *_g_p_tbl_account_master = NULL;

int32
tbl_account_add_account_sync(tbl_account_t *p_account, uint32 sync)
{
    tbl_account_master_t *p_master = _g_p_tbl_account_master;
    tbl_account_t *p_db_account = NULL;

    /* 1. lookup entry exist */
    if (tbl_account_get_account(&p_account->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_account = XCALLOC(MEM_TBL_ACCOUNT, sizeof(tbl_account_t));
    if (NULL == p_db_account)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_account, p_account, sizeof(tbl_account_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->account_list, p_db_account);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ACCOUNT, p_db_account);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ACCOUNT, p_db_account);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_account_del_account_sync(tbl_account_key_t *p_account_key, uint32 sync)
{
    tbl_account_master_t *p_master = _g_p_tbl_account_master;
    tbl_account_t *p_db_account = NULL;

    /* 1. lookup entry exist */
    p_db_account = tbl_account_get_account(p_account_key);
    if (NULL == p_db_account)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ACCOUNT, p_db_account);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ACCOUNT, p_db_account);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->account_list, p_db_account);

    /* 4. free db entry */
    XFREE(MEM_TBL_ACCOUNT, p_db_account);

    return PM_E_NONE;
}

int32
tbl_account_set_account_field_sync(tbl_account_t *p_account, tbl_account_field_id_t field_id, uint32 sync)
{
    tbl_account_t *p_db_account = NULL;

    /* 1. lookup entry exist */
    p_db_account = tbl_account_get_account(&p_account->key);
    if (NULL == p_db_account)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ACCOUNT_FLD_START_STOP:
        p_db_account->start_stop = p_account->start_stop;
        break;

    case TBL_ACCOUNT_FLD_STOP_ONLY:
        p_db_account->stop_only = p_account->stop_only;
        break;

    case TBL_ACCOUNT_FLD_METHODS:
        sal_memcpy(p_db_account->methods, p_account->methods, sizeof(p_account->methods));
        break;

    case TBL_ACCOUNT_FLD_MAX:
        sal_memcpy(p_db_account, p_account, sizeof(tbl_account_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ACCOUNT, field_id, p_db_account);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ACCOUNT, field_id, p_db_account);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_account_add_account(tbl_account_t *p_account)
{
    return tbl_account_add_account_sync(p_account, TRUE);
}

int32
tbl_account_del_account(tbl_account_key_t *p_account_key)
{
    return tbl_account_del_account_sync(p_account_key, TRUE);
}

int32
tbl_account_set_account_field(tbl_account_t *p_account, tbl_account_field_id_t field_id)
{
    return tbl_account_set_account_field_sync(p_account, field_id, TRUE);
}

tbl_account_t*
tbl_account_get_account(tbl_account_key_t *p_account_key)
{
    tbl_account_master_t *p_master = _g_p_tbl_account_master;
    tbl_account_t lkp;

    sal_memcpy(&lkp.key, p_account_key, sizeof(tbl_account_key_t));
    return ctclib_slistdata_lookup(p_master->account_list, &lkp);
}

char*
tbl_account_key_val2str(tbl_account_t *p_account, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", p_account->key.name);
    return str;
}

int32
tbl_account_key_str2val(char *str, tbl_account_t *p_account)
{
    int32 ret = 0;

    sal_strcpy(p_account->key.name, str);
    return ret;
}

char*
tbl_account_key_name_dump(tbl_account_t *p_account, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"%s", p_node->name, 
        p_account->key.name);
    return str;
}

char*
tbl_account_key_value_dump(tbl_account_t *p_account, char *str)
{
    sal_sprintf(str, "%s", p_account->key.name);
    return str;
}

char*
tbl_account_field_name_dump(tbl_account_t *p_account, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ACCOUNT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_account_field_value_dump(tbl_account_t *p_account, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ACCOUNT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", p_account->key.name);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_START_STOP, field_id))
    {
        sal_sprintf(str, "%u", p_account->start_stop);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_STOP_ONLY, field_id))
    {
        sal_sprintf(str, "%u", p_account->stop_only);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_METHODS, field_id))
    {
        sal_sprintf(str, "%s", p_account->methods);
    }
    return str;
}

char**
tbl_account_table_dump(tbl_account_t *p_account, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_account_key_name_dump(p_account, buf));
    for(i=1; i<TBL_ACCOUNT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_account_field_name_dump(p_account, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_account_field_value_dump(p_account, i, buf));
    }
    return str;
}

int32
tbl_account_field_value_parser(char *str, int32 field_id, tbl_account_t *p_account)
{
    if (FLD_MATCH(TBL_ACCOUNT_FLD_KEY, field_id))
    {
        sal_strcpy(p_account->key.name, str);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_START_STOP, field_id))
    {
        int32 ret;
        p_account->start_stop = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_STOP_ONLY, field_id))
    {
        int32 ret;
        p_account->stop_only = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_METHODS, field_id))
    {
        sal_strcpy(p_account->methods, str);
    }
    return PM_E_NONE;
}

int32
tbl_account_table_parser(char** array, char* key_value,tbl_account_t *p_account)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_account_key_str2val(key_value, p_account));

    for(i=1; i<TBL_ACCOUNT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_account_field_value_parser( array[j++], i, p_account));
    }

    return PM_E_NONE;
}

int32
tbl_account_dump_one(tbl_account_t *p_account, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ACCOUNT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ACCOUNT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACCOUNT_FLD_KEY].name,
            p_account->key.name);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_START_STOP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACCOUNT_FLD_START_STOP].name,
            p_account->start_stop);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_STOP_ONLY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ACCOUNT_FLD_STOP_ONLY].name,
            p_account->stop_only);
    }
    if (FLD_MATCH(TBL_ACCOUNT_FLD_METHODS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ACCOUNT_FLD_METHODS].name,
            p_account->methods);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_account_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_account_master_t *p_master = _g_p_tbl_account_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_account_t *p_db_account = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->account_list, p_db_account, listnode, next)
    {
        rc |= fn(p_db_account, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_account_master_t*
tbl_account_get_master()
{
    return _g_p_tbl_account_master;
}

tbl_account_master_t*
tbl_account_init_account()
{
    _g_p_tbl_account_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_account_master_t));
    _g_p_tbl_account_master->account_list = ctclib_slist_create(tbl_account_cmp, NULL);
    return _g_p_tbl_account_master;
}

