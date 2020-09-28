
#include "proto.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_enable.h"
#include "cdb_data_cmp.h"

tbl_enable_master_t *_g_p_tbl_enable_master = NULL;

int32
tbl_enable_add_enable_sync(tbl_enable_t *p_enable, uint32 sync)
{
    tbl_enable_master_t *p_master = _g_p_tbl_enable_master;
    tbl_enable_t *p_db_enable = NULL;

    /* 1. lookup entry exist */
    if (tbl_enable_get_enable(&p_enable->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_enable = XCALLOC(MEM_TBL_ENABLE, sizeof(tbl_enable_t));
    if (NULL == p_db_enable)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_enable, p_enable, sizeof(tbl_enable_t));

    /* 4. add to db */
    p_master->enable_array[p_enable->key.id] = p_db_enable;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ENABLE, p_db_enable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ENABLE, p_db_enable);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_enable_del_enable_sync(tbl_enable_key_t *p_enable_key, uint32 sync)
{
    tbl_enable_master_t *p_master = _g_p_tbl_enable_master;
    tbl_enable_t *p_db_enable = NULL;

    /* 1. lookup entry exist */
    p_db_enable = tbl_enable_get_enable(p_enable_key);
    if (NULL == p_db_enable)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ENABLE, p_db_enable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ENABLE, p_db_enable);
        #endif 
    }

    /* 3. delete from db */
    p_master->enable_array[p_enable_key->id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ENABLE, p_db_enable);

    return PM_E_NONE;
}

int32
tbl_enable_set_enable_field_sync(tbl_enable_t *p_enable, tbl_enable_field_id_t field_id, uint32 sync)
{
    tbl_enable_t *p_db_enable = NULL;

    /* 1. lookup entry exist */
    p_db_enable = tbl_enable_get_enable(&p_enable->key);
    if (NULL == p_db_enable)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ENABLE_FLD_ENC_PASSWD:
        sal_memcpy(p_db_enable->enc_passwd, p_enable->enc_passwd, sizeof(p_enable->enc_passwd));
        break;

    case TBL_ENABLE_FLD_PASSWD:
        sal_memcpy(p_db_enable->passwd, p_enable->passwd, sizeof(p_enable->passwd));
        break;

    case TBL_ENABLE_FLD_MAX:
        sal_memcpy(p_db_enable, p_enable, sizeof(tbl_enable_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ENABLE, field_id, p_db_enable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ENABLE, field_id, p_db_enable);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_enable_add_enable(tbl_enable_t *p_enable)
{
    return tbl_enable_add_enable_sync(p_enable, TRUE);
}

int32
tbl_enable_del_enable(tbl_enable_key_t *p_enable_key)
{
    return tbl_enable_del_enable_sync(p_enable_key, TRUE);
}

int32
tbl_enable_set_enable_field(tbl_enable_t *p_enable, tbl_enable_field_id_t field_id)
{
    return tbl_enable_set_enable_field_sync(p_enable, field_id, TRUE);
}

tbl_enable_t*
tbl_enable_get_enable(tbl_enable_key_t *p_enable_key)
{
    tbl_enable_master_t *p_master = _g_p_tbl_enable_master;

    if (GLB_MAX_PRIVILEAGE_NUM <= p_enable_key->id)
    {
        return NULL;
    }
    return (p_master->enable_array[p_enable_key->id]);
}

char*
tbl_enable_key_val2str(tbl_enable_t *p_enable, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_enable->key.id);
    return str;
}

int32
tbl_enable_key_str2val(char *str, tbl_enable_t *p_enable)
{
    int32 ret = 0;

    p_enable->key.id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_enable_key_name_dump(tbl_enable_t *p_enable, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ENABLE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_enable->key.id);
    return str;
}

char*
tbl_enable_key_value_dump(tbl_enable_t *p_enable, char *str)
{
    sal_sprintf(str, "%u", p_enable->key.id);
    return str;
}

char*
tbl_enable_field_name_dump(tbl_enable_t *p_enable, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ENABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ENABLE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_enable_field_value_dump(tbl_enable_t *p_enable, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ENABLE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_enable->key.id);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_ENC_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_enable->enc_passwd);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_PASSWD, field_id))
    {
        sal_sprintf(str, "%s", p_enable->passwd);
    }
    return str;
}

char**
tbl_enable_table_dump(tbl_enable_t *p_enable, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_enable_key_name_dump(p_enable, buf));
    for(i=1; i<TBL_ENABLE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_enable_field_name_dump(p_enable, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_enable_field_value_dump(p_enable, i, buf));
    }
    return str;
}

int32
tbl_enable_field_value_parser(char *str, int32 field_id, tbl_enable_t *p_enable)
{
    if (FLD_MATCH(TBL_ENABLE_FLD_KEY, field_id))
    {
        int32 ret;
        p_enable->key.id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_ENC_PASSWD, field_id))
    {
        sal_strcpy(p_enable->enc_passwd, str);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_PASSWD, field_id))
    {
        sal_strcpy(p_enable->passwd, str);
    }
    return PM_E_NONE;
}

int32
tbl_enable_table_parser(char** array, char* key_value,tbl_enable_t *p_enable)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ENABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_enable_key_str2val(key_value, p_enable));

    for(i=1; i<TBL_ENABLE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_enable_field_value_parser( array[j++], i, p_enable));
    }

    return PM_E_NONE;
}

int32
tbl_enable_dump_one(tbl_enable_t *p_enable, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ENABLE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ENABLE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ENABLE_FLD_KEY].name,
            p_enable->key.id);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_ENC_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ENABLE_FLD_ENC_PASSWD].name,
            p_enable->enc_passwd);
    }
    if (FLD_MATCH(TBL_ENABLE_FLD_PASSWD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ENABLE_FLD_PASSWD].name,
            p_enable->passwd);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_enable_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_enable_master_t *p_master = _g_p_tbl_enable_master;
    tbl_enable_t *p_db_enable = NULL;
    uint32 id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (id = 0; id < GLB_MAX_PRIVILEAGE_NUM; id++)
    {
        p_db_enable = p_master->enable_array[id];
        if (NULL == p_db_enable)
        {
            continue;
        }
        rc |= fn(p_db_enable, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_enable_master_t*
tbl_enable_get_master()
{
    return _g_p_tbl_enable_master;
}

tbl_enable_master_t*
tbl_enable_init_enable()
{
    _g_p_tbl_enable_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_enable_master_t));
    return _g_p_tbl_enable_master;
}

