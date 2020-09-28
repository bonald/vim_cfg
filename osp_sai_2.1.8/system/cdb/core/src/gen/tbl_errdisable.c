
#include "proto.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "cdb_data_cmp.h"

tbl_errdisable_master_t *_g_p_tbl_errdisable_master = NULL;

int32
tbl_errdisable_add_errdisable_sync(tbl_errdisable_t *p_errdisable, uint32 sync)
{
    tbl_errdisable_master_t *p_master = _g_p_tbl_errdisable_master;
    tbl_errdisable_t *p_db_errdisable = NULL;

    /* 1. lookup entry exist */
    if (tbl_errdisable_get_errdisable(&p_errdisable->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_errdisable = XCALLOC(MEM_TBL_ERRDISABLE, sizeof(tbl_errdisable_t));
    if (NULL == p_db_errdisable)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_errdisable, p_errdisable, sizeof(tbl_errdisable_t));

    /* 4. add to db */
    p_master->errdisable_array[p_errdisable->key.reason] = p_db_errdisable;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ERRDISABLE, p_db_errdisable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ERRDISABLE, p_db_errdisable);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_del_errdisable_sync(tbl_errdisable_key_t *p_errdisable_key, uint32 sync)
{
    tbl_errdisable_master_t *p_master = _g_p_tbl_errdisable_master;
    tbl_errdisable_t *p_db_errdisable = NULL;

    /* 1. lookup entry exist */
    p_db_errdisable = tbl_errdisable_get_errdisable(p_errdisable_key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ERRDISABLE, p_db_errdisable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ERRDISABLE, p_db_errdisable);
        #endif 
    }

    /* 3. delete from db */
    p_master->errdisable_array[p_errdisable_key->reason] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ERRDISABLE, p_db_errdisable);

    return PM_E_NONE;
}

int32
tbl_errdisable_set_errdisable_field_sync(tbl_errdisable_t *p_errdisable, tbl_errdisable_field_id_t field_id, uint32 sync)
{
    tbl_errdisable_t *p_db_errdisable = NULL;

    /* 1. lookup entry exist */
    p_db_errdisable = tbl_errdisable_get_errdisable(&p_errdisable->key);
    if (NULL == p_db_errdisable)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ERRDISABLE_FLD_ERRDISABLE_EN:
        p_db_errdisable->errdisable_en = p_errdisable->errdisable_en;
        break;

    case TBL_ERRDISABLE_FLD_RECOVERY_EN:
        p_db_errdisable->recovery_en = p_errdisable->recovery_en;
        break;

    case TBL_ERRDISABLE_FLD_MAX:
        sal_memcpy(p_db_errdisable, p_errdisable, sizeof(tbl_errdisable_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ERRDISABLE, field_id, p_db_errdisable);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ERRDISABLE, field_id, p_db_errdisable);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_add_errdisable(tbl_errdisable_t *p_errdisable)
{
    return tbl_errdisable_add_errdisable_sync(p_errdisable, TRUE);
}

int32
tbl_errdisable_del_errdisable(tbl_errdisable_key_t *p_errdisable_key)
{
    return tbl_errdisable_del_errdisable_sync(p_errdisable_key, TRUE);
}

int32
tbl_errdisable_set_errdisable_field(tbl_errdisable_t *p_errdisable, tbl_errdisable_field_id_t field_id)
{
    return tbl_errdisable_set_errdisable_field_sync(p_errdisable, field_id, TRUE);
}

tbl_errdisable_t*
tbl_errdisable_get_errdisable(tbl_errdisable_key_t *p_errdisable_key)
{
    tbl_errdisable_master_t *p_master = _g_p_tbl_errdisable_master;

    if (ERRDIS_RSN_MAX <= p_errdisable_key->reason)
    {
        return NULL;
    }
    return (p_master->errdisable_array[p_errdisable_key->reason]);
}

char*
tbl_errdisable_key_val2str(tbl_errdisable_t *p_errdisable, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%s", cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_errdisable->key.reason));
    return str;
}

int32
tbl_errdisable_key_str2val(char *str, tbl_errdisable_t *p_errdisable)
{
    int32 ret = 0;
    ret =  cdb_enum_str2val(errdis_rsn_strs, ERRDIS_RSN_MAX, str);
    return ret;
}

char*
tbl_errdisable_key_name_dump(tbl_errdisable_t *p_errdisable, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
        "%s", p_node->name, cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_errdisable->key.reason));
    return str;
}

char*
tbl_errdisable_key_value_dump(tbl_errdisable_t *p_errdisable, char *str)
{
    sal_sprintf(str, "%s", cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_errdisable->key.reason));
    return str;
}

char*
tbl_errdisable_field_name_dump(tbl_errdisable_t *p_errdisable, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ERRDISABLE_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_errdisable_field_value_dump(tbl_errdisable_t *p_errdisable, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_errdisable->key.reason));
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_ERRDISABLE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable->errdisable_en);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_RECOVERY_EN, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable->recovery_en);
    }
    return str;
}

char**
tbl_errdisable_table_dump(tbl_errdisable_t *p_errdisable, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_errdisable_key_name_dump(p_errdisable, buf));
    for(i=1; i<TBL_ERRDISABLE_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_errdisable_field_name_dump(p_errdisable, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_errdisable_field_value_dump(p_errdisable, i, buf));
    }
    return str;
}

int32
tbl_errdisable_field_value_parser(char *str, int32 field_id, tbl_errdisable_t *p_errdisable)
{
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_KEY, field_id))
    {
        cdb_enum_str2val(errdis_rsn_strs, ERRDIS_RSN_MAX, str);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_ERRDISABLE_EN, field_id))
    {
        int32 ret;
        p_errdisable->errdisable_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_RECOVERY_EN, field_id))
    {
        int32 ret;
        p_errdisable->recovery_en = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_errdisable_table_parser(char** array, char* key_value,tbl_errdisable_t *p_errdisable)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_errdisable_key_str2val(key_value, p_errdisable));

    for(i=1; i<TBL_ERRDISABLE_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_errdisable_field_value_parser( array[j++], i, p_errdisable));
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_dump_one(tbl_errdisable_t *p_errdisable, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ERRDISABLE_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_ERRDISABLE_FLD_KEY].name, 
            cdb_enum_val2str(errdis_rsn_strs, ERRDIS_RSN_MAX, p_errdisable->key.reason));
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_ERRDISABLE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLD_ERRDISABLE_EN].name,
            p_errdisable->errdisable_en);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLD_RECOVERY_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLD_RECOVERY_EN].name,
            p_errdisable->recovery_en);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_errdisable_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_errdisable_master_t *p_master = _g_p_tbl_errdisable_master;
    tbl_errdisable_t *p_db_errdisable = NULL;
    uint32 reason = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (reason = 0; reason < ERRDIS_RSN_MAX; reason++)
    {
        p_db_errdisable = p_master->errdisable_array[reason];
        if (NULL == p_db_errdisable)
        {
            continue;
        }
        rc |= fn(p_db_errdisable, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_errdisable_master_t*
tbl_errdisable_get_master()
{
    return _g_p_tbl_errdisable_master;
}

tbl_errdisable_master_t*
tbl_errdisable_init_errdisable()
{
    _g_p_tbl_errdisable_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_errdisable_master_t));
    return _g_p_tbl_errdisable_master;
}

