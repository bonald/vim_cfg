
#include "proto.h"
#include "gen/tbl_cpu_mirror_global_define.h"
#include "gen/tbl_cpu_mirror_global.h"
#include "cdb_data_cmp.h"

tbl_cpu_mirror_global_t *_g_p_tbl_cpu_mirror_global = NULL;

int32
tbl_cpu_mirror_global_set_cpu_mirror_global_field_sync(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_cpu_mirror_global_field_id_t field_id, uint32 sync)
{
    tbl_cpu_mirror_global_t *p_db_cpu_mirror = _g_p_tbl_cpu_mirror_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY:
        p_db_cpu_mirror->strategy = p_cpu_mirror->strategy;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE:
        p_db_cpu_mirror->buf_size = p_cpu_mirror->buf_size;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST:
        p_db_cpu_mirror->buf_list = p_cpu_mirror->buf_list;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE:
        p_db_cpu_mirror->write_file = p_cpu_mirror->write_file;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD:
        p_db_cpu_mirror->flash_size_threshold = p_cpu_mirror->flash_size_threshold;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX:
        p_db_cpu_mirror->buf_mutex = p_cpu_mirror->buf_mutex;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP:
        p_db_cpu_mirror->capture_fp = p_cpu_mirror->capture_fp;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER:
        p_db_cpu_mirror->capture_timer = p_cpu_mirror->capture_timer;
        break;

    case TBL_CPU_MIRROR_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_cpu_mirror, p_cpu_mirror, sizeof(tbl_cpu_mirror_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CPU_MIRROR_GLOBAL, field_id, p_db_cpu_mirror);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CPU_MIRROR_GLOBAL, field_id, p_db_cpu_mirror);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_cpu_mirror_global_set_cpu_mirror_global_field(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_cpu_mirror_global_field_id_t field_id)
{
    return tbl_cpu_mirror_global_set_cpu_mirror_global_field_sync(p_cpu_mirror, field_id, TRUE);
}

tbl_cpu_mirror_global_t*
tbl_cpu_mirror_global_get_cpu_mirror_global()
{
    return _g_p_tbl_cpu_mirror_global;
}

int32
tbl_cpu_mirror_global_key_str2val(char *str, tbl_cpu_mirror_global_t *p_cpu_mirror)
{
    return PM_E_NONE;
}

char*
tbl_cpu_mirror_global_key_name_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MIRROR_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_cpu_mirror_global_key_value_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_cpu_mirror_global_field_name_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MIRROR_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CPU_MIRROR_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_cpu_mirror_global_field_value_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_mirror->strategy);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_mirror->buf_size);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE, field_id))
    {
        sal_sprintf(str, "%u", p_cpu_mirror->write_file);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%"PRIu64, p_cpu_mirror->flash_size_threshold);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_cpu_mirror->capture_timer);
    }
    return str;
}

char**
tbl_cpu_mirror_global_table_dump(tbl_cpu_mirror_global_t *p_cpu_mirror, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_cpu_mirror_global_key_name_dump(p_cpu_mirror, buf));
    for(i=1; i<TBL_CPU_MIRROR_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_cpu_mirror_global_field_name_dump(p_cpu_mirror, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_cpu_mirror_global_field_value_dump(p_cpu_mirror, i, buf));
    }
    return str;
}

int32
tbl_cpu_mirror_global_field_value_parser(char *str, int32 field_id, tbl_cpu_mirror_global_t *p_cpu_mirror)
{
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY, field_id))
    {
        int32 ret;
        p_cpu_mirror->strategy = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE, field_id))
    {
        int32 ret;
        p_cpu_mirror->buf_size = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE, field_id))
    {
        int32 ret;
        p_cpu_mirror->write_file = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD, field_id))
    {
        int32 ret;
        p_cpu_mirror->flash_size_threshold = cdb_uint64_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP, field_id))
    {
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER, field_id))
    {
        ctc_task_str2val(str, p_cpu_mirror->capture_timer);
    }
    return PM_E_NONE;
}

int32
tbl_cpu_mirror_global_table_parser(char** array, char* key_value,tbl_cpu_mirror_global_t *p_cpu_mirror)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MIRROR_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_cpu_mirror_global_key_str2val(key_value, p_cpu_mirror));

    for(i=1; i<TBL_CPU_MIRROR_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_cpu_mirror_global_field_value_parser( array[j++], i, p_cpu_mirror));
    }

    return PM_E_NONE;
}

int32
tbl_cpu_mirror_global_dump_one(tbl_cpu_mirror_global_t *p_cpu_mirror, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CPU_MIRROR_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY].name,
            p_cpu_mirror->strategy);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE].name,
            p_cpu_mirror->buf_size);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST].name);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE].name,
            p_cpu_mirror->write_file);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD].name,
            p_cpu_mirror->flash_size_threshold);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX].name);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP].name);
    }
    if (FLD_MATCH(TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_cpu_mirror->capture_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_cpu_mirror_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_cpu_mirror_global_t *p_db_cpu_mirror = _g_p_tbl_cpu_mirror_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_cpu_mirror)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_cpu_mirror, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_cpu_mirror_global_t*
tbl_cpu_mirror_global_init_cpu_mirror_global()
{
    _g_p_tbl_cpu_mirror_global = XCALLOC(MEM_TBL_CPU_MIRROR_GLOBAL, sizeof(tbl_cpu_mirror_global_t));
    return _g_p_tbl_cpu_mirror_global;
}

