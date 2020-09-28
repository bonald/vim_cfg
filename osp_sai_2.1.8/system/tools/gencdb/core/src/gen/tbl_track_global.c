
#include "proto.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "cdb_data_cmp.h"

tbl_track_global_t *_g_p_tbl_track_global = NULL;

int32
tbl_track_global_set_track_global_field_sync(tbl_track_global_t *p_track_glb, tbl_track_global_field_id_t field_id, uint32 sync)
{
    tbl_track_global_t *p_db_track_glb = _g_p_tbl_track_global;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM:
        p_db_track_glb->track_object_num = p_track_glb->track_object_num;
        break;

    case TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL:
        p_db_track_glb->track_timer_interval = p_track_glb->track_timer_interval;
        break;

    case TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER:
        p_db_track_glb->t_track_timer = p_track_glb->t_track_timer;
        break;

    case TBL_TRACK_GLOBAL_FLD_MAX:
        sal_memcpy(p_db_track_glb, p_track_glb, sizeof(tbl_track_global_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_TRACK_GLOBAL, field_id, p_db_track_glb);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_TRACK_GLOBAL, field_id, p_db_track_glb);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_track_global_set_track_global_field(tbl_track_global_t *p_track_glb, tbl_track_global_field_id_t field_id)
{
    return tbl_track_global_set_track_global_field_sync(p_track_glb, field_id, TRUE);
}

tbl_track_global_t*
tbl_track_global_get_track_global()
{
    return _g_p_tbl_track_global;
}

int32
tbl_track_global_key_str2val(char *str, tbl_track_global_t *p_track_glb)
{
    return PM_E_NONE;
}

char*
tbl_track_global_key_name_dump(tbl_track_global_t *p_track_glb, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_GLOBAL);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_track_global_key_value_dump(tbl_track_global_t *p_track_glb, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_track_global_field_name_dump(tbl_track_global_t *p_track_glb, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_TRACK_GLOBAL_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_track_global_field_value_dump(tbl_track_global_t *p_track_glb, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM, field_id))
    {
        sal_sprintf(str, "%u", p_track_glb->track_object_num);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL, field_id))
    {
        sal_sprintf(str, "%u", p_track_glb->track_timer_interval);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_track_glb->t_track_timer);
    }
    return str;
}

char**
tbl_track_global_table_dump(tbl_track_global_t *p_track_glb, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_track_global_key_name_dump(p_track_glb, buf));
    for(i=1; i<TBL_TRACK_GLOBAL_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_track_global_field_name_dump(p_track_glb, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_track_global_field_value_dump(p_track_glb, i, buf));
    }
    return str;
}

int32
tbl_track_global_field_value_parser(char *str, int32 field_id, tbl_track_global_t *p_track_glb)
{
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM, field_id))
    {
        int32 ret;
        p_track_glb->track_object_num = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL, field_id))
    {
        int32 ret;
        p_track_glb->track_timer_interval = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER, field_id))
    {
        ctc_task_str2val(str, p_track_glb->t_track_timer);
    }
    return PM_E_NONE;
}

int32
tbl_track_global_table_parser(char** array, char* key_value,tbl_track_global_t *p_track_glb)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_GLOBAL);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_track_global_key_str2val(key_value, p_track_glb));

    for(i=1; i<TBL_TRACK_GLOBAL_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_track_global_field_value_parser( array[j++], i, p_track_glb));
    }

    return PM_E_NONE;
}

int32
tbl_track_global_dump_one(tbl_track_global_t *p_track_glb, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_TRACK_GLOBAL);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_GLOBAL_FLD_TRACK_OBJECT_NUM].name,
            p_track_glb->track_object_num);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_TRACK_GLOBAL_FLD_TRACK_TIMER_INTERVAL].name,
            p_track_glb->track_timer_interval);
    }
    if (FLD_MATCH(TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_TRACK_GLOBAL_FLD_T_TRACK_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_track_glb->t_track_timer));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_track_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_track_global_t *p_db_track_glb = _g_p_tbl_track_global;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_track_glb)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_track_glb, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_track_global_t*
tbl_track_global_init_track_global()
{
    _g_p_tbl_track_global = XCALLOC(MEM_TBL_TRACK_GLOBAL, sizeof(tbl_track_global_t));
    return _g_p_tbl_track_global;
}

