
#include "proto.h"
#include "gen/tbl_tag_summer_clock_define.h"
#include "gen/tbl_tag_summer_clock.h"
#include "cdb_data_cmp.h"

tbl_tag_summer_clock_t *_g_p_tbl_tag_summer_clock = NULL;

int32
tbl_tag_summer_clock_set_tag_summer_clock_field_sync(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_tag_summer_clock_field_id_t field_id, uint32 sync)
{
    tbl_tag_summer_clock_t *p_db_tag_summer_clk = _g_p_tbl_tag_summer_clock;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TAG_SUMMER_CLOCK_FLD_TMSTART:
        sal_memcpy(&p_db_tag_summer_clk->tmstart, &p_tag_summer_clk->tmstart, sizeof(p_tag_summer_clk->tmstart));
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_TMEND:
        sal_memcpy(&p_db_tag_summer_clk->tmend, &p_tag_summer_clk->tmend, sizeof(p_tag_summer_clk->tmend));
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET:
        p_db_tag_summer_clk->iDstOffset = p_tag_summer_clk->iDstOffset;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING:
        p_db_tag_summer_clk->is_recurring = p_tag_summer_clk->is_recurring;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY:
        p_db_tag_summer_clk->nStartFebDay = p_tag_summer_clk->nStartFebDay;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY:
        p_db_tag_summer_clk->nEndFebDay = p_tag_summer_clk->nEndFebDay;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME:
        sal_memcpy(p_db_tag_summer_clk->strZoneName, p_tag_summer_clk->strZoneName, sizeof(p_tag_summer_clk->strZoneName));
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM:
        p_db_tag_summer_clk->is_wday_tm = p_tag_summer_clk->is_wday_tm;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START:
        sal_memcpy(&p_db_tag_summer_clk->wday_tm_start, &p_tag_summer_clk->wday_tm_start, sizeof(p_tag_summer_clk->wday_tm_start));
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END:
        sal_memcpy(&p_db_tag_summer_clk->wday_tm_end, &p_tag_summer_clk->wday_tm_end, sizeof(p_tag_summer_clk->wday_tm_end));
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER:
        p_db_tag_summer_clk->t_summer_time_timer = p_tag_summer_clk->t_summer_time_timer;
        break;

    case TBL_TAG_SUMMER_CLOCK_FLD_MAX:
        sal_memcpy(p_db_tag_summer_clk, p_tag_summer_clk, sizeof(tbl_tag_summer_clock_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TAG_SUMMER_CLOCK, field_id, p_db_tag_summer_clk);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TAG_SUMMER_CLOCK, field_id, p_db_tag_summer_clk);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_tag_summer_clock_set_tag_summer_clock_field(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_tag_summer_clock_field_id_t field_id)
{
    return tbl_tag_summer_clock_set_tag_summer_clock_field_sync(p_tag_summer_clk, field_id, TRUE);
}

tbl_tag_summer_clock_t*
tbl_tag_summer_clock_get_tag_summer_clock()
{
    return _g_p_tbl_tag_summer_clock;
}

int32
tbl_tag_summer_clock_key_str2val(char *str, tbl_tag_summer_clock_t *p_tag_summer_clk)
{
    return PM_E_NONE;
}

char*
tbl_tag_summer_clock_key_name_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAG_SUMMER_CLOCK);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_tag_summer_clock_key_value_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_tag_summer_clock_field_name_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TAG_SUMMER_CLOCK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TAG_SUMMER_CLOCK_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_tag_summer_clock_field_value_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMSTART, field_id))
    {
        cdb_summer_time_val2str(str, MAX_CMD_STR_LEN, &p_tag_summer_clk->tmstart);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMEND, field_id))
    {
        cdb_summer_time_val2str(str, MAX_CMD_STR_LEN, &p_tag_summer_clk->tmend);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET, field_id))
    {
        sal_sprintf(str, "%d", p_tag_summer_clk->iDstOffset);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING, field_id))
    {
        sal_sprintf(str, "%d", p_tag_summer_clk->is_recurring);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY, field_id))
    {
        sal_sprintf(str, "%d", p_tag_summer_clk->nStartFebDay);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY, field_id))
    {
        sal_sprintf(str, "%d", p_tag_summer_clk->nEndFebDay);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME, field_id))
    {
        sal_sprintf(str, "%s", p_tag_summer_clk->strZoneName);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM, field_id))
    {
        sal_sprintf(str, "%d", p_tag_summer_clk->is_wday_tm);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START, field_id))
    {
        cdb_which_summer_time_val2str(str, MAX_CMD_STR_LEN, &p_tag_summer_clk->wday_tm_start);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END, field_id))
    {
        cdb_which_summer_time_val2str(str, MAX_CMD_STR_LEN, &p_tag_summer_clk->wday_tm_end);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_tag_summer_clk->t_summer_time_timer);
    }
    return str;
}

char**
tbl_tag_summer_clock_table_dump(tbl_tag_summer_clock_t *p_tag_summer_clk, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_tag_summer_clock_key_name_dump(p_tag_summer_clk, buf));
    for(i=1; i<TBL_TAG_SUMMER_CLOCK_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_tag_summer_clock_field_name_dump(p_tag_summer_clk, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_tag_summer_clock_field_value_dump(p_tag_summer_clk, i, buf));
    }
    return str;
}

int32
tbl_tag_summer_clock_field_value_parser(char *str, int32 field_id, tbl_tag_summer_clock_t *p_tag_summer_clk)
{
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMSTART, field_id))
    {
        cdb_summer_time_str2val(str, &p_tag_summer_clk->tmstart);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMEND, field_id))
    {
        cdb_summer_time_str2val(str, &p_tag_summer_clk->tmend);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME, field_id))
    {
        sal_strcpy(p_tag_summer_clk->strZoneName, str);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM, field_id))
    {
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START, field_id))
    {
        cdb_which_summer_time_str2val(str, &p_tag_summer_clk->wday_tm_start);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END, field_id))
    {
        cdb_which_summer_time_str2val(str, &p_tag_summer_clk->wday_tm_end);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER, field_id))
    {
        ctc_task_str2val(str, p_tag_summer_clk->t_summer_time_timer);
    }
    return PM_E_NONE;
}

int32
tbl_tag_summer_clock_table_parser(char** array, char* key_value,tbl_tag_summer_clock_t *p_tag_summer_clk)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAG_SUMMER_CLOCK);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_tag_summer_clock_key_str2val(key_value, p_tag_summer_clk));

    for(i=1; i<TBL_TAG_SUMMER_CLOCK_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_tag_summer_clock_field_value_parser( array[j++], i, p_tag_summer_clk));
    }

    return PM_E_NONE;
}

int32
tbl_tag_summer_clock_dump_one(tbl_tag_summer_clock_t *p_tag_summer_clk, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TAG_SUMMER_CLOCK);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMSTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_TMSTART].name, 
            cdb_summer_time_val2str(buf, MAX_CMD_STR_LEN, &p_tag_summer_clk->tmstart));
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_TMEND, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_TMEND].name, 
            cdb_summer_time_val2str(buf, MAX_CMD_STR_LEN, &p_tag_summer_clk->tmend));
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET].name,
            p_tag_summer_clk->iDstOffset);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING].name,
            p_tag_summer_clk->is_recurring);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY].name,
            p_tag_summer_clk->nStartFebDay);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY].name,
            p_tag_summer_clk->nEndFebDay);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME].name,
            p_tag_summer_clk->strZoneName);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%d", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM].name,
            p_tag_summer_clk->is_wday_tm);
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START].name, 
            cdb_which_summer_time_val2str(buf, MAX_CMD_STR_LEN, &p_tag_summer_clk->wday_tm_start));
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END].name, 
            cdb_which_summer_time_val2str(buf, MAX_CMD_STR_LEN, &p_tag_summer_clk->wday_tm_end));
    }
    if (FLD_MATCH(TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_tag_summer_clk->t_summer_time_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_tag_summer_clock_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_tag_summer_clock_t *p_db_tag_summer_clk = _g_p_tbl_tag_summer_clock;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_tag_summer_clk)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_tag_summer_clk, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_tag_summer_clock_t*
tbl_tag_summer_clock_init_tag_summer_clock()
{
    _g_p_tbl_tag_summer_clock = XCALLOC(MEM_TBL_TAG_SUMMER_CLOCK, sizeof(tbl_tag_summer_clock_t));
    return _g_p_tbl_tag_summer_clock;
}

