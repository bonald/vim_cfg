
#include "proto.h"
#include "gen/tbl_sys_load_define.h"
#include "gen/tbl_sys_load.h"
#include "cdb_data_cmp.h"

tbl_sys_load_t *_g_p_tbl_sys_load = NULL;

int32
tbl_sys_load_set_sys_load_field_sync(tbl_sys_load_t *p_sys_load, tbl_sys_load_field_id_t field_id, uint32 sync)
{
    tbl_sys_load_t *p_db_sys_load = _g_p_tbl_sys_load;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SYS_LOAD_FLD_SYS_LOAD_EN:
        p_db_sys_load->sys_load_en = p_sys_load->sys_load_en;
        break;

    case TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN:
        p_db_sys_load->cli_back_pressure_en = p_sys_load->cli_back_pressure_en;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD:
        p_db_sys_load->cdb_queue_notice_threshold = p_sys_load->cdb_queue_notice_threshold;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD:
        p_db_sys_load->cdb_queue_warning_threshold = p_sys_load->cdb_queue_warning_threshold;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH:
        p_db_sys_load->cdb_queue_depth_switch = p_sys_load->cdb_queue_depth_switch;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED:
        p_db_sys_load->cdb_queue_depth_routed = p_sys_load->cdb_queue_depth_routed;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS:
        p_db_sys_load->cdb_queue_depth_cds = p_sys_load->cdb_queue_depth_cds;
        break;

    case TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA:
        p_db_sys_load->cdb_queue_depth_fea = p_sys_load->cdb_queue_depth_fea;
        break;

    case TBL_SYS_LOAD_FLD_MAX:
        sal_memcpy(p_db_sys_load, p_sys_load, sizeof(tbl_sys_load_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SYS_LOAD, field_id, p_db_sys_load);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SYS_LOAD, field_id, p_db_sys_load);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_sys_load_set_sys_load_field(tbl_sys_load_t *p_sys_load, tbl_sys_load_field_id_t field_id)
{
    return tbl_sys_load_set_sys_load_field_sync(p_sys_load, field_id, TRUE);
}

tbl_sys_load_t*
tbl_sys_load_get_sys_load()
{
    return _g_p_tbl_sys_load;
}

int32
tbl_sys_load_key_str2val(char *str, tbl_sys_load_t *p_sys_load)
{
    return PM_E_NONE;
}

char*
tbl_sys_load_key_name_dump(tbl_sys_load_t *p_sys_load, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_LOAD);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_sys_load_key_value_dump(tbl_sys_load_t *p_sys_load, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_sys_load_field_name_dump(tbl_sys_load_t *p_sys_load, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_LOAD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SYS_LOAD_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_sys_load_field_value_dump(tbl_sys_load_t *p_sys_load, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_SYS_LOAD_EN, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->sys_load_en);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cli_back_pressure_en);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_notice_threshold);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_warning_threshold);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_depth_switch);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_depth_routed);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_depth_cds);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA, field_id))
    {
        sal_sprintf(str, "%u", p_sys_load->cdb_queue_depth_fea);
    }
    return str;
}

char**
tbl_sys_load_table_dump(tbl_sys_load_t *p_sys_load, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_sys_load_key_name_dump(p_sys_load, buf));
    for(i=1; i<TBL_SYS_LOAD_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_sys_load_field_name_dump(p_sys_load, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_sys_load_field_value_dump(p_sys_load, i, buf));
    }
    return str;
}

int32
tbl_sys_load_field_value_parser(char *str, int32 field_id, tbl_sys_load_t *p_sys_load)
{
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_SYS_LOAD_EN, field_id))
    {
        int32 ret;
        p_sys_load->sys_load_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN, field_id))
    {
        int32 ret;
        p_sys_load->cli_back_pressure_en = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_notice_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_warning_threshold = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_depth_switch = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_depth_routed = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_depth_cds = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA, field_id))
    {
        int32 ret;
        p_sys_load->cdb_queue_depth_fea = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_sys_load_table_parser(char** array, char* key_value,tbl_sys_load_t *p_sys_load)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_LOAD);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_sys_load_key_str2val(key_value, p_sys_load));

    for(i=1; i<TBL_SYS_LOAD_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_sys_load_field_value_parser( array[j++], i, p_sys_load));
    }

    return PM_E_NONE;
}

int32
tbl_sys_load_dump_one(tbl_sys_load_t *p_sys_load, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYS_LOAD);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SYS_LOAD_FLD_SYS_LOAD_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_SYS_LOAD_EN].name,
            p_sys_load->sys_load_en);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN].name,
            p_sys_load->cli_back_pressure_en);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD].name,
            p_sys_load->cdb_queue_notice_threshold);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD].name,
            p_sys_load->cdb_queue_warning_threshold);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH].name,
            p_sys_load->cdb_queue_depth_switch);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED].name,
            p_sys_load->cdb_queue_depth_routed);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS].name,
            p_sys_load->cdb_queue_depth_cds);
    }
    if (FLD_MATCH(TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA].name,
            p_sys_load->cdb_queue_depth_fea);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_sys_load_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_sys_load_t *p_db_sys_load = _g_p_tbl_sys_load;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_sys_load)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_sys_load, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_sys_load_t*
tbl_sys_load_init_sys_load()
{
    _g_p_tbl_sys_load = XCALLOC(MEM_TBL_SYS_LOAD, sizeof(tbl_sys_load_t));
    return _g_p_tbl_sys_load;
}

