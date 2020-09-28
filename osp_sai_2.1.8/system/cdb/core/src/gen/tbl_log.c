
#include "proto.h"
#include "gen/tbl_log_define.h"
#include "gen/tbl_log.h"
#include "gen/tbl_log_global_define.h"
#include "gen/tbl_log_global.h"
#include "cdb_data_cmp.h"

int
tbl_log_to_cds(int log_id, int severity, char *log_str);

tbl_log_master_t *_g_p_tbl_log_master = NULL;

int32
tbl_log_add_log_sync(tbl_log_t *p_log, uint32 sync)
{
    tbl_log_master_t *p_master = _g_p_tbl_log_master;
    tbl_log_t *p_db_log = NULL;

    /* 1. lookup entry exist */
    p_db_log = p_master->array[p_master->curr_index];

    /* 2. alloc a new db entry */
    if (NULL == p_db_log)
    {
        p_db_log = XCALLOC(MEM_TBL_LOG, sizeof(tbl_log_t));
        if (NULL == p_db_log)
        {
            return PM_E_NO_MEMORY;
        }
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_log, p_log, sizeof(tbl_log_t));
    p_db_log->key.sequence_id = p_master->curr_sequence_id;

    /* 4. add to db */
    p_master->array[p_master->curr_index] = p_db_log;
    p_master->curr_sequence_id++;
    p_master->curr_index = (p_master->curr_sequence_id % LOG_ITEM_COUNT);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_LOG, p_db_log);
    }

    return PM_E_NONE;
}

int32
tbl_log_del_log_sync(tbl_log_key_t *p_log_key, uint32 sync)
{
    return PM_E_NONE;
}

int32
tbl_log_set_log_field_sync(tbl_log_t *p_log, tbl_log_field_id_t field_id, uint32 sync)
{
    return PM_E_NONE;
}

int32
tbl_log_add_log(tbl_log_t *p_log)
{
    return tbl_log_add_log_sync(p_log, TRUE);
}

int32
tbl_log_del_log(tbl_log_key_t *p_log_key)
{
    return tbl_log_del_log_sync(p_log_key, TRUE);
}

int32
tbl_log_set_log_field(tbl_log_t *p_log, tbl_log_field_id_t field_id)
{
    return tbl_log_set_log_field_sync(p_log, field_id, TRUE);
}

tbl_log_t*
tbl_log_get_log(tbl_log_key_t *p_log_key)
{
    tbl_log_master_t *p_master = _g_p_tbl_log_master;

    if (LOG_ITEM_COUNT <= p_log_key->sequence_id)
    {
        return NULL;
    }
    return (p_master->array[p_log_key->sequence_id]);
}

char*
tbl_log_key_val2str(tbl_log_t *p_log, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%d", p_log->key.sequence_id);
    return str;
}

int32
tbl_log_key_str2val(char *str, tbl_log_t *p_log)
{
    int32 ret;
    p_log->key.sequence_id = cdb_uint_str2val(str, &ret);
    
    return ret;
}

int32
tbl_log_dump_one(tbl_log_t *p_log, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LOG_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOG_FLD_KEY].name,
            p_log->key.sequence_id);
    }
    if (FLD_MATCH(TBL_LOG_FLD_LOG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOG_FLD_LOG_ID].name,
            p_log->log_id);
    }
    if (FLD_MATCH(TBL_LOG_FLD_SEVERITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_LOG_FLD_SEVERITY].name,
            p_log->severity);
    }
    if (FLD_MATCH(TBL_LOG_FLD_TIMESTAMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOG_FLD_TIMESTAMP].name, 
            cdb_timestamp_val2str(buf, MAX_CMD_STR_LEN, &p_log->timestamp));
    }
    if (FLD_MATCH(TBL_LOG_FLD_DATA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_LOG_FLD_DATA].name,
            p_log->data);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

char* 
tbl_log_key_name_dump(tbl_log_t *p_log, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOG);

    sal_sprintf(str, "%s"KEY_CONNECT_STR"%u", p_node->name, p_log->key.sequence_id);
    return str;
}

char*
tbl_log_key_value_dump(tbl_log_t *p_log, char* str)
{
    sal_sprintf(str, "%u", p_log->key.sequence_id);
    return str;
}

char* 
tbl_log_field_name_dump(tbl_log_t *p_log, int32 field_id, char* str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (field_id >= TBL_LOG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char* 
tbl_log_field_value_dump(tbl_log_t *p_log, int32 field_id, char* str)
{
    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_LOG_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_log->key.sequence_id);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_LOG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_log->log_id);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_SEVERITY, field_id))
    {
        sal_sprintf(str, "%u", p_log->severity);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_TIMESTAMP, field_id))
    {
        sal_sprintf(str, "%s", cdb_timestamp_val2str(buf, MAX_CMD_STR_LEN, &p_log->timestamp));
    }
    else if (FLD_MATCH(TBL_LOG_FLD_DATA, field_id))
    {
        sal_sprintf(str, "%s", p_log->data);   
    }    
    else return NULL;
    
    return str;
}

char** 
tbl_log_table_dump(tbl_log_t *p_log, char** str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s", tbl_log_key_name_dump(p_log, buf));
    for(i=1; i<TBL_LOG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s", tbl_log_field_name_dump(p_log, i, buf));
        sal_sprintf(str[j++], "%s", tbl_log_field_value_dump(p_log, i, buf));
    }
    
    return str;
}

int32 
tbl_log_field_value_parser(char* str, int32 field_id, tbl_log_t *p_log)
{
    int32 ret = 0;

    if (FLD_MATCH(TBL_LOG_FLD_KEY, field_id))
    {
        p_log->key.sequence_id = cdb_int_str2val(str, &ret);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_LOG_ID, field_id))
    {
        p_log->log_id = cdb_int_str2val(str, &ret);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_SEVERITY, field_id))
    {
        p_log->severity = cdb_int_str2val(str, &ret);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_TIMESTAMP, field_id))
    {
        cdb_timestamp_str2val(str, &p_log->timestamp);
    }
    else if (FLD_MATCH(TBL_LOG_FLD_DATA, field_id))
    {
        sal_strcpy(p_log->data, str);   
    }    
    else return PM_E_INVALID_PARAM;

    return ret;
}

int32 
tbl_log_table_parser(char** array, char* key_value, tbl_log_t *p_log)
{
    int32 rc;
    int32 i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_LOG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_log_key_str2val(key_value, p_log));

    for(i=1; i<TBL_LOG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
            return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_log_field_value_parser( array[j++], i, p_log));
    }
    
    return PM_E_NONE;
}

int32
tbl_log_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_log_master_t *p_master = _g_p_tbl_log_master;
    tbl_log_t *p_db_log = NULL;
    uint32 start_index = 0;
    uint32 log_count = 0;
    uint32 index = 0;
    uint32 i = 0;
    int32 rc = PM_E_NONE;

    if (p_master->curr_sequence_id >= LOG_ITEM_COUNT)
    {
        start_index = p_master->curr_index;
        log_count = LOG_ITEM_COUNT;
    }
    else
    {
        start_index = 0;
        log_count = p_master->curr_sequence_id;
    }

    index = start_index;
    for (i = 0; i < log_count; i++)
    {
        p_db_log = p_master->array[index];
        if (NULL == p_db_log)
        {
            continue;
        }
        rc |= fn(p_db_log, pargs);
        
        index++;
        if (index >= LOG_ITEM_COUNT)
        {
            index = index - LOG_ITEM_COUNT;
        }
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_log_master_t*
tbl_log_get_master()
{
    return _g_p_tbl_log_master;
}

tbl_log_master_t*
tbl_log_init_log()
{
    logid_register_cb_func(tbl_log_to_cds);
    
    _g_p_tbl_log_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_log_master_t));
    return _g_p_tbl_log_master;
}

/* not write local CDB, but just sent to CDS
 * CDS should not use this func 
 */

int
tbl_log_format_log(int log_id, int severity, char *log_str, tbl_log_t *log)
{
    char *p = NULL;
    uint32 str_len = 0;
    
    p = sal_strchr(log_str, '>');
    if (NULL == p)
    {
        return PM_E_INVALID_PARAM;
    }
    p++;

    str_len = sal_strlen(p);
    if (p[str_len-1] == '\n')
    {
        p[str_len-1] = CMD_ZERO_CHAR;
        str_len--;
    }

    if (str_len >= LOG_DATA_LEN)
    {
        return PM_E_INVALID_PARAM;
    }
    
    log->key.sequence_id = 0;
    log->log_id = log_id;
    log->severity = severity;
    ctc_task_boottime(&log->timestamp);
    sal_strncpy(log->data, p, LOG_DATA_LEN);
    
    return PM_E_NONE;
}

/* for CDS add log */
int
tbl_log_add(int log_id, int severity, char *log_str)
{
    tbl_log_global_t *p_glb = tbl_log_global_get_log_global();
    tbl_log_t log;
    int32 rc = PM_E_NONE;

    if (!GLB_BMP_ISSET(p_glb->log_to_cdb, log_id))
    {
        return PM_E_NONE;
    }

    rc = tbl_log_format_log(log_id, severity, log_str, &log);
    if (rc < 0)
    {
        return rc;
    }
    
    tbl_log_add_log(&log);

    return PM_E_NONE;
}

int
tbl_log_to_cds(int log_id, int severity, char *log_str)
{
    tbl_log_global_t *p_glb = tbl_log_global_get_log_global();
    tbl_log_t log;
    int32 rc = PM_E_NONE;

    if (!GLB_BMP_ISSET(p_glb->log_to_cdb, log_id))
    {
        return PM_E_NONE;
    }

    rc = tbl_log_format_log(log_id, severity, log_str, &log);
    if (rc < 0)
    {
        return rc;
    }
    
    cdb_sync_tbl_add(TBL_LOG, &log);

    return PM_E_NONE;
}

