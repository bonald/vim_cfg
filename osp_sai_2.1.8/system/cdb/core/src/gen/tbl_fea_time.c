
#include "proto.h"
#include "gen/tbl_fea_time_define.h"
#include "gen/tbl_fea_time.h"
#include "cdb_data_cmp.h"

tbl_fea_time_t *_g_p_tbl_fea_time = NULL;

int32
tbl_fea_time_set_fea_time_field_sync(tbl_fea_time_t *p_fea_time, tbl_fea_time_field_id_t field_id, uint32 sync)
{
    tbl_fea_time_t *p_db_fea_time = _g_p_tbl_fea_time;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_FEA_TIME_FLD_SYNC_TYPE:
        p_db_fea_time->sync_type = p_fea_time->sync_type;
        break;

    case TBL_FEA_TIME_FLD_SYNC_COUNT:
        p_db_fea_time->sync_count = p_fea_time->sync_count;
        break;

    case TBL_FEA_TIME_FLD_LAST_SYNC_TIME:
        sal_memcpy(&p_db_fea_time->last_sync_time, &p_fea_time->last_sync_time, sizeof(p_fea_time->last_sync_time));
        break;

    case TBL_FEA_TIME_FLD_MAX:
        sal_memcpy(p_db_fea_time, p_fea_time, sizeof(tbl_fea_time_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_FEA_TIME, field_id, p_db_fea_time);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_FEA_TIME, field_id, p_db_fea_time);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_fea_time_set_fea_time_field(tbl_fea_time_t *p_fea_time, tbl_fea_time_field_id_t field_id)
{
    return tbl_fea_time_set_fea_time_field_sync(p_fea_time, field_id, TRUE);
}

tbl_fea_time_t*
tbl_fea_time_get_fea_time()
{
    return _g_p_tbl_fea_time;
}

int32
tbl_fea_time_key_str2val(char *str, tbl_fea_time_t *p_fea_time)
{
    return PM_E_NONE;
}

char*
tbl_fea_time_key_name_dump(tbl_fea_time_t *p_fea_time, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_TIME);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_fea_time_key_value_dump(tbl_fea_time_t *p_fea_time, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_fea_time_field_name_dump(tbl_fea_time_t *p_fea_time, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_TIME);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_FEA_TIME_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_fea_time_field_value_dump(tbl_fea_time_t *p_fea_time, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_TYPE, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(glb_time_sync_type_strs, GLB_TIME_SYNC_TYPE_MAX, p_fea_time->sync_type));
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_COUNT, field_id))
    {
        sal_sprintf(str, "%u", p_fea_time->sync_count);
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_LAST_SYNC_TIME, field_id))
    {
        cdb_sal_time_val2str(str, MAX_CMD_STR_LEN, &p_fea_time->last_sync_time);
    }
    return str;
}

char**
tbl_fea_time_table_dump(tbl_fea_time_t *p_fea_time, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_fea_time_key_name_dump(p_fea_time, buf));
    for(i=1; i<TBL_FEA_TIME_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_fea_time_field_name_dump(p_fea_time, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_fea_time_field_value_dump(p_fea_time, i, buf));
    }
    return str;
}

int32
tbl_fea_time_field_value_parser(char *str, int32 field_id, tbl_fea_time_t *p_fea_time)
{
    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_TYPE, field_id))
    {
        cdb_enum_str2val(glb_time_sync_type_strs, GLB_TIME_SYNC_TYPE_MAX, str);
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_COUNT, field_id))
    {
        int32 ret;
        p_fea_time->sync_count = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_LAST_SYNC_TIME, field_id))
    {
        cdb_sal_time_str2val(str, &p_fea_time->last_sync_time);
    }
    return PM_E_NONE;
}

int32
tbl_fea_time_table_parser(char** array, char* key_value,tbl_fea_time_t *p_fea_time)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_TIME);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_fea_time_key_str2val(key_value, p_fea_time));

    for(i=1; i<TBL_FEA_TIME_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_fea_time_field_value_parser( array[j++], i, p_fea_time));
    }

    return PM_E_NONE;
}

int32
tbl_fea_time_dump_one(tbl_fea_time_t *p_fea_time, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_FEA_TIME);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_TYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_TIME_FLD_SYNC_TYPE].name, 
            cdb_enum_val2str(glb_time_sync_type_strs, GLB_TIME_SYNC_TYPE_MAX, p_fea_time->sync_type));
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_SYNC_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_FEA_TIME_FLD_SYNC_COUNT].name,
            p_fea_time->sync_count);
    }
    if (FLD_MATCH(TBL_FEA_TIME_FLD_LAST_SYNC_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_FEA_TIME_FLD_LAST_SYNC_TIME].name, 
            cdb_sal_time_val2str(buf, MAX_CMD_STR_LEN, &p_fea_time->last_sync_time));
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_fea_time_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_fea_time_t *p_db_fea_time = _g_p_tbl_fea_time;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_fea_time)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_fea_time, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_fea_time_t*
tbl_fea_time_init_fea_time()
{
    _g_p_tbl_fea_time = XCALLOC(MEM_TBL_FEA_TIME, sizeof(tbl_fea_time_t));
    return _g_p_tbl_fea_time;
}

