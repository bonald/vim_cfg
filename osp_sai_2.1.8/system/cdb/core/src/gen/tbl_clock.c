
#include "proto.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "cdb_data_cmp.h"

tbl_clock_t *_g_p_tbl_clock = NULL;

int32
tbl_clock_set_clock_field_sync(tbl_clock_t *p_clk, tbl_clock_field_id_t field_id, uint32 sync)
{
    tbl_clock_t *p_db_clk = _g_p_tbl_clock;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_CLOCK_FLD_TIMEZONE_POSITIVE:
        p_db_clk->timezone_positive = p_clk->timezone_positive;
        break;

    case TBL_CLOCK_FLD_TIMEZONE_HOUR:
        p_db_clk->timezone_hour = p_clk->timezone_hour;
        break;

    case TBL_CLOCK_FLD_TIMEZONE_MINUTE:
        p_db_clk->timezone_minute = p_clk->timezone_minute;
        break;

    case TBL_CLOCK_FLD_TIMEZONE_SECOND:
        p_db_clk->timezone_second = p_clk->timezone_second;
        break;

    case TBL_CLOCK_FLD_TIMEZONE_NAME:
        sal_memcpy(p_db_clk->timezone_name, p_clk->timezone_name, sizeof(p_clk->timezone_name));
        break;

    case TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE:
        p_db_clk->cp_timezone_positive = p_clk->cp_timezone_positive;
        break;

    case TBL_CLOCK_FLD_CP_TIMEZONE_HOUR:
        p_db_clk->cp_timezone_hour = p_clk->cp_timezone_hour;
        break;

    case TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE:
        p_db_clk->cp_timezone_minute = p_clk->cp_timezone_minute;
        break;

    case TBL_CLOCK_FLD_CP_TIMEZONE_SECOND:
        p_db_clk->cp_timezone_second = p_clk->cp_timezone_second;
        break;

    case TBL_CLOCK_FLD_CP_TIMEZONE_NAME:
        sal_memcpy(p_db_clk->cp_timezone_name, p_clk->cp_timezone_name, sizeof(p_clk->cp_timezone_name));
        break;

    case TBL_CLOCK_FLD_MAX:
        sal_memcpy(p_db_clk, p_clk, sizeof(tbl_clock_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_CLOCK, field_id, p_db_clk);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_CLOCK, field_id, p_db_clk);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_clock_set_clock_field(tbl_clock_t *p_clk, tbl_clock_field_id_t field_id)
{
    return tbl_clock_set_clock_field_sync(p_clk, field_id, TRUE);
}

tbl_clock_t*
tbl_clock_get_clock()
{
    return _g_p_tbl_clock;
}

int32
tbl_clock_key_str2val(char *str, tbl_clock_t *p_clk)
{
    return PM_E_NONE;
}

char*
tbl_clock_key_name_dump(tbl_clock_t *p_clk, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLOCK);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_clock_key_value_dump(tbl_clock_t *p_clk, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_clock_field_name_dump(tbl_clock_t *p_clk, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_CLOCK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_CLOCK_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_clock_field_value_dump(tbl_clock_t *p_clk, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_POSITIVE, field_id))
    {
        sal_sprintf(str, "%u", p_clk->timezone_positive);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_HOUR, field_id))
    {
        sal_sprintf(str, "%u", p_clk->timezone_hour);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_MINUTE, field_id))
    {
        sal_sprintf(str, "%u", p_clk->timezone_minute);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_SECOND, field_id))
    {
        sal_sprintf(str, "%u", p_clk->timezone_second);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_clk->timezone_name);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE, field_id))
    {
        sal_sprintf(str, "%u", p_clk->cp_timezone_positive);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_HOUR, field_id))
    {
        sal_sprintf(str, "%u", p_clk->cp_timezone_hour);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE, field_id))
    {
        sal_sprintf(str, "%u", p_clk->cp_timezone_minute);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_SECOND, field_id))
    {
        sal_sprintf(str, "%u", p_clk->cp_timezone_second);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_clk->cp_timezone_name);
    }
    return str;
}

char**
tbl_clock_table_dump(tbl_clock_t *p_clk, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_clock_key_name_dump(p_clk, buf));
    for(i=1; i<TBL_CLOCK_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_clock_field_name_dump(p_clk, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_clock_field_value_dump(p_clk, i, buf));
    }
    return str;
}

int32
tbl_clock_field_value_parser(char *str, int32 field_id, tbl_clock_t *p_clk)
{
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_POSITIVE, field_id))
    {
        int32 ret;
        p_clk->timezone_positive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_HOUR, field_id))
    {
        int32 ret;
        p_clk->timezone_hour = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_MINUTE, field_id))
    {
        int32 ret;
        p_clk->timezone_minute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_SECOND, field_id))
    {
        int32 ret;
        p_clk->timezone_second = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_NAME, field_id))
    {
        sal_strcpy(p_clk->timezone_name, str);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE, field_id))
    {
        int32 ret;
        p_clk->cp_timezone_positive = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_HOUR, field_id))
    {
        int32 ret;
        p_clk->cp_timezone_hour = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE, field_id))
    {
        int32 ret;
        p_clk->cp_timezone_minute = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_SECOND, field_id))
    {
        int32 ret;
        p_clk->cp_timezone_second = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_NAME, field_id))
    {
        sal_strcpy(p_clk->cp_timezone_name, str);
    }
    return PM_E_NONE;
}

int32
tbl_clock_table_parser(char** array, char* key_value,tbl_clock_t *p_clk)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLOCK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_clock_key_str2val(key_value, p_clk));

    for(i=1; i<TBL_CLOCK_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_clock_field_value_parser( array[j++], i, p_clk));
    }

    return PM_E_NONE;
}

int32
tbl_clock_dump_one(tbl_clock_t *p_clk, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_CLOCK);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_POSITIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_TIMEZONE_POSITIVE].name,
            p_clk->timezone_positive);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_HOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_TIMEZONE_HOUR].name,
            p_clk->timezone_hour);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_MINUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_TIMEZONE_MINUTE].name,
            p_clk->timezone_minute);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_SECOND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_TIMEZONE_SECOND].name,
            p_clk->timezone_second);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_TIMEZONE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CLOCK_FLD_TIMEZONE_NAME].name,
            p_clk->timezone_name);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE].name,
            p_clk->cp_timezone_positive);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_HOUR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_CP_TIMEZONE_HOUR].name,
            p_clk->cp_timezone_hour);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE].name,
            p_clk->cp_timezone_minute);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_SECOND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_CLOCK_FLD_CP_TIMEZONE_SECOND].name,
            p_clk->cp_timezone_second);
    }
    if (FLD_MATCH(TBL_CLOCK_FLD_CP_TIMEZONE_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_CLOCK_FLD_CP_TIMEZONE_NAME].name,
            p_clk->cp_timezone_name);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_clock_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_clock_t *p_db_clk = _g_p_tbl_clock;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_clk)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_clk, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_clock_t*
tbl_clock_init_clock()
{
    _g_p_tbl_clock = XCALLOC(MEM_TBL_CLOCK, sizeof(tbl_clock_t));
    return _g_p_tbl_clock;
}

