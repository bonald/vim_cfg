
#include "proto.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "cdb_data_cmp.h"

tbl_mem_summary_t *_g_p_tbl_mem_summary = NULL;

int32
tbl_mem_summary_set_mem_summary_field_sync(tbl_mem_summary_t *p_mem_info, tbl_mem_summary_field_id_t field_id, uint32 sync)
{
    tbl_mem_summary_t *p_db_mem_info = _g_p_tbl_mem_summary;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_MEM_SUMMARY_FLD_TOTAL:
        p_db_mem_info->total = p_mem_info->total;
        break;

    case TBL_MEM_SUMMARY_FLD_USED:
        p_db_mem_info->used = p_mem_info->used;
        break;

    case TBL_MEM_SUMMARY_FLD_FREE:
        p_db_mem_info->free = p_mem_info->free;
        break;

    case TBL_MEM_SUMMARY_FLD_BUFFER:
        p_db_mem_info->buffer = p_mem_info->buffer;
        break;

    case TBL_MEM_SUMMARY_FLD_CACHED:
        p_db_mem_info->cached = p_mem_info->cached;
        break;

    case TBL_MEM_SUMMARY_FLD_MEM_HIGH:
        p_db_mem_info->mem_high = p_mem_info->mem_high;
        break;

    case TBL_MEM_SUMMARY_FLD_CPU_HIGH:
        p_db_mem_info->cpu_high = p_mem_info->cpu_high;
        break;

    case TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD:
        p_db_mem_info->cpu_high_threshold = p_mem_info->cpu_high_threshold;
        break;

    case TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD:
        p_db_mem_info->cpu_low_threshold = p_mem_info->cpu_low_threshold;
        break;

    case TBL_MEM_SUMMARY_FLD_MAX:
        sal_memcpy(p_db_mem_info, p_mem_info, sizeof(tbl_mem_summary_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_MEM_SUMMARY, field_id, p_db_mem_info);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_MEM_SUMMARY, field_id, p_db_mem_info);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_mem_summary_set_mem_summary_field(tbl_mem_summary_t *p_mem_info, tbl_mem_summary_field_id_t field_id)
{
    return tbl_mem_summary_set_mem_summary_field_sync(p_mem_info, field_id, TRUE);
}

tbl_mem_summary_t*
tbl_mem_summary_get_mem_summary()
{
    return _g_p_tbl_mem_summary;
}

int32
tbl_mem_summary_key_str2val(char *str, tbl_mem_summary_t *p_mem_info)
{
    return PM_E_NONE;
}

char*
tbl_mem_summary_key_name_dump(tbl_mem_summary_t *p_mem_info, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MEM_SUMMARY);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_mem_summary_key_value_dump(tbl_mem_summary_t *p_mem_info, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_mem_summary_field_name_dump(tbl_mem_summary_t *p_mem_info, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_MEM_SUMMARY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_MEM_SUMMARY_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_mem_summary_field_value_dump(tbl_mem_summary_t *p_mem_info, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_TOTAL, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->total);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_USED, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->used);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_FREE, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->free);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_BUFFER, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->buffer);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CACHED, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->cached);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_MEM_HIGH, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->mem_high);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->cpu_high);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->cpu_high_threshold);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_mem_info->cpu_low_threshold);
    }
    return str;
}

char**
tbl_mem_summary_table_dump(tbl_mem_summary_t *p_mem_info, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_mem_summary_key_name_dump(p_mem_info, buf));
    for(i=1; i<TBL_MEM_SUMMARY_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_mem_summary_field_name_dump(p_mem_info, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_mem_summary_field_value_dump(p_mem_info, i, buf));
    }
    return str;
}

int32
tbl_mem_summary_field_value_parser(char *str, int32 field_id, tbl_mem_summary_t *p_mem_info)
{
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_TOTAL, field_id))
    {
        int32 ret;
        p_mem_info->total = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_USED, field_id))
    {
        int32 ret;
        p_mem_info->used = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_FREE, field_id))
    {
        int32 ret;
        p_mem_info->free = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_BUFFER, field_id))
    {
        int32 ret;
        p_mem_info->buffer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CACHED, field_id))
    {
        int32 ret;
        p_mem_info->cached = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_MEM_HIGH, field_id))
    {
        int32 ret;
        p_mem_info->mem_high = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH, field_id))
    {
        int32 ret;
        p_mem_info->cpu_high = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD, field_id))
    {
        int32 ret;
        p_mem_info->cpu_high_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD, field_id))
    {
        int32 ret;
        p_mem_info->cpu_low_threshold = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_mem_summary_table_parser(char** array, char* key_value,tbl_mem_summary_t *p_mem_info)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_MEM_SUMMARY);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_mem_summary_key_str2val(key_value, p_mem_info));

    for(i=1; i<TBL_MEM_SUMMARY_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_mem_summary_field_value_parser( array[j++], i, p_mem_info));
    }

    return PM_E_NONE;
}

int32
tbl_mem_summary_dump_one(tbl_mem_summary_t *p_mem_info, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_MEM_SUMMARY);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_TOTAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_TOTAL].name,
            p_mem_info->total);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_USED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_USED].name,
            p_mem_info->used);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_FREE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_FREE].name,
            p_mem_info->free);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_BUFFER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_BUFFER].name,
            p_mem_info->buffer);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CACHED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_CACHED].name,
            p_mem_info->cached);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_MEM_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_MEM_HIGH].name,
            p_mem_info->mem_high);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_CPU_HIGH].name,
            p_mem_info->cpu_high);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD].name,
            p_mem_info->cpu_high_threshold);
    }
    if (FLD_MATCH(TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD].name,
            p_mem_info->cpu_low_threshold);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_mem_summary_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_mem_summary_t *p_db_mem_info = _g_p_tbl_mem_summary;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_mem_info)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_mem_info, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_mem_summary_t*
tbl_mem_summary_init_mem_summary()
{
    _g_p_tbl_mem_summary = XCALLOC(MEM_TBL_MEM_SUMMARY, sizeof(tbl_mem_summary_t));
    return _g_p_tbl_mem_summary;
}

