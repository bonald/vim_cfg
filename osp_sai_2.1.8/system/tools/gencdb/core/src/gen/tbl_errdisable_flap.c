
#include "proto.h"
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"
#include "cdb_data_cmp.h"

tbl_errdisable_flap_master_t *_g_p_tbl_errdisable_flap_master = NULL;

int32
tbl_errdisable_flap_add_errdisable_flap_sync(tbl_errdisable_flap_t *p_errdisable_flap, uint32 sync)
{
    tbl_errdisable_flap_master_t *p_master = _g_p_tbl_errdisable_flap_master;
    tbl_errdisable_flap_t *p_db_errdisable_flap = NULL;

    /* 1. lookup entry exist */
    if (tbl_errdisable_flap_get_errdisable_flap(&p_errdisable_flap->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_errdisable_flap = XCALLOC(MEM_TBL_ERRDISABLE_FLAP, sizeof(tbl_errdisable_flap_t));
    if (NULL == p_db_errdisable_flap)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_errdisable_flap, p_errdisable_flap, sizeof(tbl_errdisable_flap_t));

    /* 4. add to db */
    p_master->errdisable_flap_array[p_errdisable_flap->key.flap_rsn] = p_db_errdisable_flap;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_ERRDISABLE_FLAP, p_db_errdisable_flap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_ERRDISABLE_FLAP, p_db_errdisable_flap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_flap_del_errdisable_flap_sync(tbl_errdisable_flap_key_t *p_errdisable_flap_key, uint32 sync)
{
    tbl_errdisable_flap_master_t *p_master = _g_p_tbl_errdisable_flap_master;
    tbl_errdisable_flap_t *p_db_errdisable_flap = NULL;

    /* 1. lookup entry exist */
    p_db_errdisable_flap = tbl_errdisable_flap_get_errdisable_flap(p_errdisable_flap_key);
    if (NULL == p_db_errdisable_flap)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_ERRDISABLE_FLAP, p_db_errdisable_flap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_ERRDISABLE_FLAP, p_db_errdisable_flap);
        #endif 
    }

    /* 3. delete from db */
    p_master->errdisable_flap_array[p_errdisable_flap_key->flap_rsn] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_ERRDISABLE_FLAP, p_db_errdisable_flap);

    return PM_E_NONE;
}

int32
tbl_errdisable_flap_set_errdisable_flap_field_sync(tbl_errdisable_flap_t *p_errdisable_flap, tbl_errdisable_flap_field_id_t field_id, uint32 sync)
{
    tbl_errdisable_flap_t *p_db_errdisable_flap = NULL;

    /* 1. lookup entry exist */
    p_db_errdisable_flap = tbl_errdisable_flap_get_errdisable_flap(&p_errdisable_flap->key);
    if (NULL == p_db_errdisable_flap)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN:
        p_db_errdisable_flap->errdisable_rsn = p_errdisable_flap->errdisable_rsn;
        break;

    case TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD:
        p_db_errdisable_flap->flap_cnt_threshold = p_errdisable_flap->flap_cnt_threshold;
        break;

    case TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD:
        p_db_errdisable_flap->flap_time_threshold = p_errdisable_flap->flap_time_threshold;
        break;

    case TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT:
        p_db_errdisable_flap->flap_time_cnt = p_errdisable_flap->flap_time_cnt;
        break;

    case TBL_ERRDISABLE_FLAP_FLD_MAX:
        sal_memcpy(p_db_errdisable_flap, p_errdisable_flap, sizeof(tbl_errdisable_flap_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_ERRDISABLE_FLAP, field_id, p_db_errdisable_flap);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_ERRDISABLE_FLAP, field_id, p_db_errdisable_flap);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_flap_add_errdisable_flap(tbl_errdisable_flap_t *p_errdisable_flap)
{
    return tbl_errdisable_flap_add_errdisable_flap_sync(p_errdisable_flap, TRUE);
}

int32
tbl_errdisable_flap_del_errdisable_flap(tbl_errdisable_flap_key_t *p_errdisable_flap_key)
{
    return tbl_errdisable_flap_del_errdisable_flap_sync(p_errdisable_flap_key, TRUE);
}

int32
tbl_errdisable_flap_set_errdisable_flap_field(tbl_errdisable_flap_t *p_errdisable_flap, tbl_errdisable_flap_field_id_t field_id)
{
    return tbl_errdisable_flap_set_errdisable_flap_field_sync(p_errdisable_flap, field_id, TRUE);
}

tbl_errdisable_flap_t*
tbl_errdisable_flap_get_errdisable_flap(tbl_errdisable_flap_key_t *p_errdisable_flap_key)
{
    tbl_errdisable_flap_master_t *p_master = _g_p_tbl_errdisable_flap_master;

    if (ERRDIS_FLAP_REASON_MAX <= p_errdisable_flap_key->flap_rsn)
    {
        return NULL;
    }
    return (p_master->errdisable_flap_array[p_errdisable_flap_key->flap_rsn]);
}

char*
tbl_errdisable_flap_key_val2str(tbl_errdisable_flap_t *p_errdisable_flap, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_errdisable_flap->key.flap_rsn);
    return str;
}

int32
tbl_errdisable_flap_key_str2val(char *str, tbl_errdisable_flap_t *p_errdisable_flap)
{
    int32 ret = 0;

    p_errdisable_flap->key.flap_rsn = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_errdisable_flap_key_name_dump(tbl_errdisable_flap_t *p_errdisable_flap, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE_FLAP);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_errdisable_flap->key.flap_rsn);
    return str;
}

char*
tbl_errdisable_flap_key_value_dump(tbl_errdisable_flap_t *p_errdisable_flap, char *str)
{
    sal_sprintf(str, "%u", p_errdisable_flap->key.flap_rsn);
    return str;
}

char*
tbl_errdisable_flap_field_name_dump(tbl_errdisable_flap_t *p_errdisable_flap, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE_FLAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_ERRDISABLE_FLAP_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_errdisable_flap_field_value_dump(tbl_errdisable_flap_t *p_errdisable_flap, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable_flap->key.flap_rsn);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable_flap->errdisable_rsn);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable_flap->flap_cnt_threshold);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable_flap->flap_time_threshold);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT, field_id))
    {
        sal_sprintf(str, "%u", p_errdisable_flap->flap_time_cnt);
    }
    return str;
}

char**
tbl_errdisable_flap_table_dump(tbl_errdisable_flap_t *p_errdisable_flap, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_errdisable_flap_key_name_dump(p_errdisable_flap, buf));
    for(i=1; i<TBL_ERRDISABLE_FLAP_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_errdisable_flap_field_name_dump(p_errdisable_flap, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_errdisable_flap_field_value_dump(p_errdisable_flap, i, buf));
    }
    return str;
}

int32
tbl_errdisable_flap_field_value_parser(char *str, int32 field_id, tbl_errdisable_flap_t *p_errdisable_flap)
{
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_KEY, field_id))
    {
        int32 ret;
        p_errdisable_flap->key.flap_rsn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN, field_id))
    {
        int32 ret;
        p_errdisable_flap->errdisable_rsn = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD, field_id))
    {
        int32 ret;
        p_errdisable_flap->flap_cnt_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD, field_id))
    {
        int32 ret;
        p_errdisable_flap->flap_time_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT, field_id))
    {
        int32 ret;
        p_errdisable_flap->flap_time_cnt = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_errdisable_flap_table_parser(char** array, char* key_value,tbl_errdisable_flap_t *p_errdisable_flap)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE_FLAP);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_errdisable_flap_key_str2val(key_value, p_errdisable_flap));

    for(i=1; i<TBL_ERRDISABLE_FLAP_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_errdisable_flap_field_value_parser( array[j++], i, p_errdisable_flap));
    }

    return PM_E_NONE;
}

int32
tbl_errdisable_flap_dump_one(tbl_errdisable_flap_t *p_errdisable_flap, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_ERRDISABLE_FLAP);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLAP_FLD_KEY].name,
            p_errdisable_flap->key.flap_rsn);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLAP_FLD_ERRDISABLE_RSN].name,
            p_errdisable_flap->errdisable_rsn);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLAP_FLD_FLAP_CNT_THRESHOLD].name,
            p_errdisable_flap->flap_cnt_threshold);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_THRESHOLD].name,
            p_errdisable_flap->flap_time_threshold);
    }
    if (FLD_MATCH(TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_ERRDISABLE_FLAP_FLD_FLAP_TIME_CNT].name,
            p_errdisable_flap->flap_time_cnt);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_errdisable_flap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_errdisable_flap_master_t *p_master = _g_p_tbl_errdisable_flap_master;
    tbl_errdisable_flap_t *p_db_errdisable_flap = NULL;
    uint32 flap_rsn = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (flap_rsn = 0; flap_rsn < ERRDIS_FLAP_REASON_MAX; flap_rsn++)
    {
        p_db_errdisable_flap = p_master->errdisable_flap_array[flap_rsn];
        if (NULL == p_db_errdisable_flap)
        {
            continue;
        }
        rc |= fn(p_db_errdisable_flap, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_errdisable_flap_master_t*
tbl_errdisable_flap_get_master()
{
    return _g_p_tbl_errdisable_flap_master;
}

tbl_errdisable_flap_master_t*
tbl_errdisable_flap_init_errdisable_flap()
{
    _g_p_tbl_errdisable_flap_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_errdisable_flap_master_t));
    return _g_p_tbl_errdisable_flap_master;
}

