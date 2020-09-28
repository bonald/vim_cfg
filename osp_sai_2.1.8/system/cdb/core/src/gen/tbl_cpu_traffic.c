
#include "proto.h"
#include "gen/tbl_cpu_traffic_define.h"
#include "gen/tbl_cpu_traffic.h"
#include "cdb_data_cmp.h"

tbl_cpu_traffic_master_t *_g_p_tbl_cpu_traffic_master = NULL;

int32
tbl_cpu_traffic_add_cpu_traffic_sync(tbl_cpu_traffic_t *p_cpu_traffic, uint32 sync)
{
    tbl_cpu_traffic_master_t *p_master = _g_p_tbl_cpu_traffic_master;
    tbl_cpu_traffic_t *p_db_cpu_traffic = NULL;

    /* 1. lookup entry exist */
    if (tbl_cpu_traffic_get_cpu_traffic(&p_cpu_traffic->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_cpu_traffic = XCALLOC(MEM_TBL_CPU_TRAFFIC, sizeof(tbl_cpu_traffic_t));
    if (NULL == p_db_cpu_traffic)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_cpu_traffic, p_cpu_traffic, sizeof(tbl_cpu_traffic_t));

    /* 4. add to db */
    p_master->array[p_cpu_traffic->key.reason_id] = p_db_cpu_traffic;

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_CPU_TRAFFIC, p_db_cpu_traffic);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_CPU_TRAFFIC, p_db_cpu_traffic);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_del_cpu_traffic_sync(tbl_cpu_traffic_key_t *p_cpu_traffic_key, uint32 sync)
{
    tbl_cpu_traffic_master_t *p_master = _g_p_tbl_cpu_traffic_master;
    tbl_cpu_traffic_t *p_db_cpu_traffic = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_traffic = tbl_cpu_traffic_get_cpu_traffic(p_cpu_traffic_key);
    if (NULL == p_db_cpu_traffic)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_CPU_TRAFFIC, p_db_cpu_traffic);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_CPU_TRAFFIC, p_db_cpu_traffic);
        #endif 
    }

    /* 3. delete from db */
    p_master->array[p_cpu_traffic_key->reason_id] = NULL;

    /* 4. free db entry */
    XFREE(MEM_TBL_CPU_TRAFFIC, p_db_cpu_traffic);

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_set_cpu_traffic_field_sync(tbl_cpu_traffic_t *p_cpu_traffic, tbl_cpu_traffic_field_id_t field_id, uint32 sync)
{
    tbl_cpu_traffic_t *p_db_cpu_traffic = NULL;

    /* 1. lookup entry exist */
    p_db_cpu_traffic = tbl_cpu_traffic_get_cpu_traffic(&p_cpu_traffic->key);
    if (NULL == p_db_cpu_traffic)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_TRAFFIC_FLD_RATE:
        p_db_cpu_traffic->rate = p_cpu_traffic->rate;
        break;

    case TBL_CPU_TRAFFIC_FLD_CLASS_ID:
        p_db_cpu_traffic->class_id = p_cpu_traffic->class_id;
        break;

    case TBL_CPU_TRAFFIC_FLD_MAX:
        sal_memcpy(p_db_cpu_traffic, p_cpu_traffic, sizeof(tbl_cpu_traffic_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_TRAFFIC, field_id, p_db_cpu_traffic);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_TRAFFIC, field_id, p_db_cpu_traffic);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_add_cpu_traffic(tbl_cpu_traffic_t *p_cpu_traffic)
{
    return tbl_cpu_traffic_add_cpu_traffic_sync(p_cpu_traffic, TRUE);
}

int32
tbl_cpu_traffic_del_cpu_traffic(tbl_cpu_traffic_key_t *p_cpu_traffic_key)
{
    return tbl_cpu_traffic_del_cpu_traffic_sync(p_cpu_traffic_key, TRUE);
}

int32
tbl_cpu_traffic_set_cpu_traffic_field(tbl_cpu_traffic_t *p_cpu_traffic, tbl_cpu_traffic_field_id_t field_id)
{
    return tbl_cpu_traffic_set_cpu_traffic_field_sync(p_cpu_traffic, field_id, TRUE);
}

tbl_cpu_traffic_t*
tbl_cpu_traffic_get_cpu_traffic(tbl_cpu_traffic_key_t *p_cpu_traffic_key)
{
    tbl_cpu_traffic_master_t *p_master = _g_p_tbl_cpu_traffic_master;

    if (GLB_CPU_TRAFFIC_MAX <= p_cpu_traffic_key->reason_id)
    {
        return NULL;
    }
    return (p_master->array[p_cpu_traffic_key->reason_id]);
}

char*
tbl_cpu_traffic_key_val2str(tbl_cpu_traffic_t *p_cpu_traffic, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_cpu_traffic->key.reason_id);
    return str;
}

int32
tbl_cpu_traffic_key_str2val(char *str, tbl_cpu_traffic_t *p_cpu_traffic)
{
    int32 ret = 0;

    p_cpu_traffic->key.reason_id = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_cpu_traffic_key_name_dump(tbl_cpu_traffic_t *p_cpu_traffic, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_cpu_traffic->key.reason_id);
    return str;
}

char*
tbl_cpu_traffic_key_value_dump(tbl_cpu_traffic_t *p_cpu_traffic, char *str)
{
    sal_sprintf(str, "%u", p_cpu_traffic->key.reason_id);
    return str;
}

char*
tbl_cpu_traffic_field_name_dump(tbl_cpu_traffic_t *p_cpu_traffic, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_TRAFFIC_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_traffic_field_value_dump(tbl_cpu_traffic_t *p_cpu_traffic, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_traffic->key.reason_id);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_RATE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_traffic->rate);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_CLASS_ID, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_traffic->class_id);
    }
    return str;
}

char**
tbl_cpu_traffic_table_dump(tbl_cpu_traffic_t *p_cpu_traffic, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_traffic_key_name_dump(p_cpu_traffic, buf));
    for(i=1; i<TBL_CPU_TRAFFIC_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_traffic_field_name_dump(p_cpu_traffic, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_traffic_field_value_dump(p_cpu_traffic, i, buf));
    }
    return str;
}

int32
tbl_cpu_traffic_field_value_parser(char *str, int32 field_id, tbl_cpu_traffic_t *p_cpu_traffic)
{
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_KEY, field_id))
    {
        int32 ret;
        p_cpu_traffic->key.reason_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_RATE, field_id))
    {
        int32 ret;
        p_cpu_traffic->rate = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_CLASS_ID, field_id))
    {
        int32 ret;
        p_cpu_traffic->class_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_traffic_table_parser(char** array, char* key_value,tbl_cpu_traffic_t *p_cpu_traffic)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_traffic_key_str2val(key_value, p_cpu_traffic));

    for(i=1; i<TBL_CPU_TRAFFIC_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_traffic_field_value_parser( array[j++], i, p_cpu_traffic));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_dump_one(tbl_cpu_traffic_t *p_cpu_traffic, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_TRAFFIC);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_TRAFFIC_FLD_KEY].name,
            p_cpu_traffic->key.reason_id);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_RATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_TRAFFIC_FLD_RATE].name,
            p_cpu_traffic->rate);
    }
    if (FLD_MATCH(TBL_CPU_TRAFFIC_FLD_CLASS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_TRAFFIC_FLD_CLASS_ID].name,
            p_cpu_traffic->class_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_traffic_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_traffic_master_t *p_master = _g_p_tbl_cpu_traffic_master;
    tbl_cpu_traffic_t *p_db_cpu_traffic = NULL;
    uint32 reason_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (reason_id = 0; reason_id < GLB_CPU_TRAFFIC_MAX; reason_id++)
    {
        p_db_cpu_traffic = p_master->array[reason_id];
        if (NULL == p_db_cpu_traffic)
        {
            continue;
        }
        rc |= fn(p_db_cpu_traffic, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_traffic_master_t*
tbl_cpu_traffic_get_master()
{
    return _g_p_tbl_cpu_traffic_master;
}

tbl_cpu_traffic_master_t*
tbl_cpu_traffic_init_cpu_traffic()
{
    _g_p_tbl_cpu_traffic_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_cpu_traffic_master_t));
    return _g_p_tbl_cpu_traffic_master;
}

