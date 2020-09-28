
#include "proto.h"
#include "gen/tbl_cpu_mem_history_define.h"
#include "gen/tbl_cpu_mem_history.h"
#include "cdb_data_cmp.h"

tbl_cpu_mem_history_t *_g_p_tbl_cpu_mem_history = NULL;

int32
tbl_cpu_mem_history_set_cpu_mem_history_field_sync(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_cpu_mem_history_field_id_t field_id, uint32 sync)
{
    tbl_cpu_mem_history_t *p_db_cpu_mem_history = _g_p_tbl_cpu_mem_history;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX:
        p_db_cpu_mem_history->polling_index = p_cpu_mem_history->polling_index;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX:
        p_db_cpu_mem_history->saving_index = p_cpu_mem_history->saving_index;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_HOUR_START:
        p_db_cpu_mem_history->hour_start = p_cpu_mem_history->hour_start;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_DAY_START:
        p_db_cpu_mem_history->day_start = p_cpu_mem_history->day_start;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE:
        p_db_cpu_mem_history->hour_last_save = p_cpu_mem_history->hour_last_save;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE:
        p_db_cpu_mem_history->day_last_save = p_cpu_mem_history->day_last_save;
        break;

    case TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE:
        sal_memcpy(p_db_cpu_mem_history->cpu_polling_save, p_cpu_mem_history->cpu_polling_save, sizeof(p_cpu_mem_history->cpu_polling_save));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR:
        sal_memcpy(p_db_cpu_mem_history->cpu_history_hour, p_cpu_mem_history->cpu_history_hour, sizeof(p_cpu_mem_history->cpu_history_hour));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY:
        sal_memcpy(p_db_cpu_mem_history->cpu_history_day, p_cpu_mem_history->cpu_history_day, sizeof(p_cpu_mem_history->cpu_history_day));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE:
        sal_memcpy(p_db_cpu_mem_history->mem_polling_save, p_cpu_mem_history->mem_polling_save, sizeof(p_cpu_mem_history->mem_polling_save));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR:
        sal_memcpy(p_db_cpu_mem_history->mem_history_hour, p_cpu_mem_history->mem_history_hour, sizeof(p_cpu_mem_history->mem_history_hour));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY:
        sal_memcpy(p_db_cpu_mem_history->mem_history_day, p_cpu_mem_history->mem_history_day, sizeof(p_cpu_mem_history->mem_history_day));
        break;

    case TBL_CPU_MEM_HISTORY_FLD_MAX:
        sal_memcpy(p_db_cpu_mem_history, p_cpu_mem_history, sizeof(tbl_cpu_mem_history_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_MEM_HISTORY, field_id, p_db_cpu_mem_history);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_MEM_HISTORY, field_id, p_db_cpu_mem_history);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_mem_history_set_cpu_mem_history_field(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_cpu_mem_history_field_id_t field_id)
{
    return tbl_cpu_mem_history_set_cpu_mem_history_field_sync(p_cpu_mem_history, field_id, TRUE);
}

tbl_cpu_mem_history_t*
tbl_cpu_mem_history_get_cpu_mem_history()
{
    return _g_p_tbl_cpu_mem_history;
}

int32
tbl_cpu_mem_history_key_str2val(char *str, tbl_cpu_mem_history_t *p_cpu_mem_history)
{
    return PM_E_NONE;
}

char*
tbl_cpu_mem_history_key_name_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MEM_HISTORY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_cpu_mem_history_key_value_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_cpu_mem_history_field_name_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MEM_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_MEM_HISTORY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_mem_history_field_value_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_mem_history->polling_index);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_mem_history->saving_index);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_START, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_mem_history->hour_start);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_START, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_mem_history->day_start);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_mem_history->hour_last_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_mem_history->day_last_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_polling_save, sizeof(p_cpu_mem_history->cpu_polling_save));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_history_hour, sizeof(p_cpu_mem_history->cpu_history_hour));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_history_day, sizeof(p_cpu_mem_history->cpu_history_day));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_polling_save, sizeof(p_cpu_mem_history->mem_polling_save));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_history_hour, sizeof(p_cpu_mem_history->mem_history_hour));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_history_day, sizeof(p_cpu_mem_history->mem_history_day));
    }
    return str;
}

char**
tbl_cpu_mem_history_table_dump(tbl_cpu_mem_history_t *p_cpu_mem_history, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_mem_history_key_name_dump(p_cpu_mem_history, buf));
    for(i=1; i<TBL_CPU_MEM_HISTORY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_mem_history_field_name_dump(p_cpu_mem_history, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_mem_history_field_value_dump(p_cpu_mem_history, i, buf));
    }
    return str;
}

int32
tbl_cpu_mem_history_field_value_parser(char *str, int32 field_id, tbl_cpu_mem_history_t *p_cpu_mem_history)
{
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX, field_id))
    {
        int32 ret;
        p_cpu_mem_history->polling_index = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX, field_id))
    {
        int32 ret;
        p_cpu_mem_history->saving_index = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_START, field_id))
    {
        int32 ret;
        p_cpu_mem_history->hour_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_START, field_id))
    {
        int32 ret;
        p_cpu_mem_history->day_start = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE, field_id))
    {
        int32 ret;
        p_cpu_mem_history->hour_last_save = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE, field_id))
    {
        int32 ret;
        p_cpu_mem_history->day_last_save = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->cpu_polling_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->cpu_history_hour);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->cpu_history_day);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->mem_polling_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->mem_history_hour);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY, field_id))
    {
        cdb_uint8_array_str2val(str, p_cpu_mem_history->mem_history_day);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_mem_history_table_parser(char** array, char* key_value,tbl_cpu_mem_history_t *p_cpu_mem_history)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MEM_HISTORY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_mem_history_key_str2val(key_value, p_cpu_mem_history));

    for(i=1; i<TBL_CPU_MEM_HISTORY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_mem_history_field_value_parser( array[j++], i, p_cpu_mem_history));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_mem_history_dump_one(tbl_cpu_mem_history_t *p_cpu_mem_history, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MEM_HISTORY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX].name,
            p_cpu_mem_history->polling_index);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX].name,
            p_cpu_mem_history->saving_index);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_HOUR_START].name,
            p_cpu_mem_history->hour_start);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_DAY_START].name,
            p_cpu_mem_history->day_start);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE].name,
            p_cpu_mem_history->hour_last_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE].name,
            p_cpu_mem_history->day_last_save);
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_polling_save, sizeof(p_cpu_mem_history->cpu_polling_save)));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_history_hour, sizeof(p_cpu_mem_history->cpu_history_hour)));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->cpu_history_day, sizeof(p_cpu_mem_history->cpu_history_day)));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_polling_save, sizeof(p_cpu_mem_history->mem_polling_save)));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_history_hour, sizeof(p_cpu_mem_history->mem_history_hour)));
    }
    if (FLD_MATCH(TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mem_history->mem_history_day, sizeof(p_cpu_mem_history->mem_history_day)));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_mem_history_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_mem_history_t *p_db_cpu_mem_history = _g_p_tbl_cpu_mem_history;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_cpu_mem_history)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_cpu_mem_history, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_mem_history_t*
tbl_cpu_mem_history_init_cpu_mem_history()
{
    _g_p_tbl_cpu_mem_history = XCALLOC(MEM_TBL_CPU_MEM_HISTORY, sizeof(tbl_cpu_mem_history_t));
    return _g_p_tbl_cpu_mem_history;
}

