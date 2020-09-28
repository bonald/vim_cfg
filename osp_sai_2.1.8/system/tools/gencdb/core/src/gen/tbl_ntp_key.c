
#include "proto.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_key.h"
#include "cdb_data_cmp.h"

tbl_ntp_key_master_t *_g_p_tbl_ntp_key_master = NULL;

int32
tbl_ntp_key_add_ntp_key_sync(tbl_ntp_key_t *p_ntp_key, uint32 sync)
{
    tbl_ntp_key_master_t *p_master = _g_p_tbl_ntp_key_master;
    tbl_ntp_key_t *p_db_ntp_key = NULL;

    /* 1. lookup entry exist */
    if (tbl_ntp_key_get_ntp_key(&p_ntp_key->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_ntp_key = XCALLOC(MEM_TBL_NTP_KEY, sizeof(tbl_ntp_key_t));
    if (NULL == p_db_ntp_key)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_ntp_key, p_ntp_key, sizeof(tbl_ntp_key_t));

    /* 4. add to db */
    ctclib_slistnode_insert_sort(p_master->ntp_key_list, p_db_ntp_key);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_NTP_KEY, p_db_ntp_key);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_NTP_KEY, p_db_ntp_key);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_key_del_ntp_key_sync(tbl_ntp_key_key_t *p_ntp_key_key, uint32 sync)
{
    tbl_ntp_key_master_t *p_master = _g_p_tbl_ntp_key_master;
    tbl_ntp_key_t *p_db_ntp_key = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_key = tbl_ntp_key_get_ntp_key(p_ntp_key_key);
    if (NULL == p_db_ntp_key)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_NTP_KEY, p_db_ntp_key);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_NTP_KEY, p_db_ntp_key);
        #endif 
    }

    /* 3. delete from db */
    ctclib_slistnode_delete(p_master->ntp_key_list, p_db_ntp_key);

    /* 4. free db entry */
    XFREE(MEM_TBL_NTP_KEY, p_db_ntp_key);

    return PM_E_NONE;
}

int32
tbl_ntp_key_set_ntp_key_field_sync(tbl_ntp_key_t *p_ntp_key, tbl_ntp_key_field_id_t field_id, uint32 sync)
{
    tbl_ntp_key_t *p_db_ntp_key = NULL;

    /* 1. lookup entry exist */
    p_db_ntp_key = tbl_ntp_key_get_ntp_key(&p_ntp_key->key);
    if (NULL == p_db_ntp_key)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_NTP_KEY_FLD_VALUE:
        sal_memcpy(p_db_ntp_key->value, p_ntp_key->value, sizeof(p_ntp_key->value));
        break;

    case TBL_NTP_KEY_FLD_TRUSTED:
        p_db_ntp_key->trusted = p_ntp_key->trusted;
        break;

    case TBL_NTP_KEY_FLD_MAX:
        sal_memcpy(p_db_ntp_key, p_ntp_key, sizeof(tbl_ntp_key_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_NTP_KEY, field_id, p_db_ntp_key);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_NTP_KEY, field_id, p_db_ntp_key);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_ntp_key_add_ntp_key(tbl_ntp_key_t *p_ntp_key)
{
    return tbl_ntp_key_add_ntp_key_sync(p_ntp_key, TRUE);
}

int32
tbl_ntp_key_del_ntp_key(tbl_ntp_key_key_t *p_ntp_key_key)
{
    return tbl_ntp_key_del_ntp_key_sync(p_ntp_key_key, TRUE);
}

int32
tbl_ntp_key_set_ntp_key_field(tbl_ntp_key_t *p_ntp_key, tbl_ntp_key_field_id_t field_id)
{
    return tbl_ntp_key_set_ntp_key_field_sync(p_ntp_key, field_id, TRUE);
}

tbl_ntp_key_t*
tbl_ntp_key_get_ntp_key(tbl_ntp_key_key_t *p_ntp_key_key)
{
    tbl_ntp_key_master_t *p_master = _g_p_tbl_ntp_key_master;
    tbl_ntp_key_t lkp;

    sal_memcpy(&lkp.key, p_ntp_key_key, sizeof(tbl_ntp_key_key_t));
    return ctclib_slistdata_lookup(p_master->ntp_key_list, &lkp);
}

char*
tbl_ntp_key_key_val2str(tbl_ntp_key_t *p_ntp_key, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_ntp_key->key.keyid);
    return str;
}

int32
tbl_ntp_key_key_str2val(char *str, tbl_ntp_key_t *p_ntp_key)
{
    int32 ret = 0;

    p_ntp_key->key.keyid = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_ntp_key_key_name_dump(tbl_ntp_key_t *p_ntp_key, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_KEY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_ntp_key->key.keyid);
    return str;
}

char*
tbl_ntp_key_key_value_dump(tbl_ntp_key_t *p_ntp_key, char *str)
{
    sal_sprintf(str, "%u", p_ntp_key->key.keyid);
    return str;
}

char*
tbl_ntp_key_field_name_dump(tbl_ntp_key_t *p_ntp_key, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_KEY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_NTP_KEY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_ntp_key_field_value_dump(tbl_ntp_key_t *p_ntp_key, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_NTP_KEY_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_key->key.keyid);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_VALUE, field_id))
    {
        sal_sprintf(str, "%s", p_ntp_key->value);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_TRUSTED, field_id))
    {
        sal_sprintf(str, "%u", p_ntp_key->trusted);
    }
    return str;
}

char**
tbl_ntp_key_table_dump(tbl_ntp_key_t *p_ntp_key, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_ntp_key_key_name_dump(p_ntp_key, buf));
    for(i=1; i<TBL_NTP_KEY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_ntp_key_field_name_dump(p_ntp_key, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_ntp_key_field_value_dump(p_ntp_key, i, buf));
    }
    return str;
}

int32
tbl_ntp_key_field_value_parser(char *str, int32 field_id, tbl_ntp_key_t *p_ntp_key)
{
    if (FLD_MATCH(TBL_NTP_KEY_FLD_KEY, field_id))
    {
        int32 ret;
        p_ntp_key->key.keyid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_VALUE, field_id))
    {
        sal_strcpy(p_ntp_key->value, str);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_TRUSTED, field_id))
    {
        int32 ret;
        p_ntp_key->trusted = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_ntp_key_table_parser(char** array, char* key_value,tbl_ntp_key_t *p_ntp_key)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_KEY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_ntp_key_key_str2val(key_value, p_ntp_key));

    for(i=1; i<TBL_NTP_KEY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_ntp_key_field_value_parser( array[j++], i, p_ntp_key));
    }

    return PM_E_NONE;
}

int32
tbl_ntp_key_dump_one(tbl_ntp_key_t *p_ntp_key, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_NTP_KEY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_NTP_KEY_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_KEY_FLD_KEY].name,
            p_ntp_key->key.keyid);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_VALUE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_NTP_KEY_FLD_VALUE].name,
            p_ntp_key->value);
    }
    if (FLD_MATCH(TBL_NTP_KEY_FLD_TRUSTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_NTP_KEY_FLD_TRUSTED].name,
            p_ntp_key->trusted);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_ntp_key_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_ntp_key_master_t *p_master = _g_p_tbl_ntp_key_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ntp_key_t *p_db_ntp_key = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ntp_key_list, p_db_ntp_key, listnode, next)
    {
        rc |= fn(p_db_ntp_key, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_ntp_key_master_t*
tbl_ntp_key_get_master()
{
    return _g_p_tbl_ntp_key_master;
}

tbl_ntp_key_master_t*
tbl_ntp_key_init_ntp_key()
{
    _g_p_tbl_ntp_key_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_ntp_key_master_t));
    _g_p_tbl_ntp_key_master->ntp_key_list = ctclib_slist_create(tbl_ntp_key_cmp, NULL);
    return _g_p_tbl_ntp_key_master;
}

