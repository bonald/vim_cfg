
#include "proto.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "cdb_data_cmp.h"

tbl_syslog_cfg_t *_g_p_tbl_syslog_cfg = NULL;

int32
tbl_syslog_cfg_set_syslog_cfg_field_sync(tbl_syslog_cfg_t *p_syslog_cfg, tbl_syslog_cfg_field_id_t field_id, uint32 sync)
{
    tbl_syslog_cfg_t *p_db_syslog_cfg = _g_p_tbl_syslog_cfg;

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER:
        p_db_syslog_cfg->enable_to_server = p_syslog_cfg->enable_to_server;
        break;

    case TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE:
        p_db_syslog_cfg->enable_to_file = p_syslog_cfg->enable_to_file;
        break;

    case TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP:
        p_db_syslog_cfg->enable_to_trap = p_syslog_cfg->enable_to_trap;
        break;

    case TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG:
        p_db_syslog_cfg->enable_to_diag = p_syslog_cfg->enable_to_diag;
        break;

    case TBL_SYSLOG_CFG_FLD_LOGGING_LINES:
        p_db_syslog_cfg->logging_lines = p_syslog_cfg->logging_lines;
        break;

    case TBL_SYSLOG_CFG_FLD_SERVER_ADDR:
        sal_memcpy(p_db_syslog_cfg->server_addr, p_syslog_cfg->server_addr, sizeof(p_syslog_cfg->server_addr));
        break;

    case TBL_SYSLOG_CFG_FLD_SERVER_FACILITY:
        p_db_syslog_cfg->server_facility = p_syslog_cfg->server_facility;
        break;

    case TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY:
        p_db_syslog_cfg->server_severity = p_syslog_cfg->server_severity;
        break;

    case TBL_SYSLOG_CFG_FLD_TRAP_FACILITY:
        p_db_syslog_cfg->trap_facility = p_syslog_cfg->trap_facility;
        break;

    case TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY:
        p_db_syslog_cfg->trap_severity = p_syslog_cfg->trap_severity;
        break;

    case TBL_SYSLOG_CFG_FLD_TIMESTAMP:
        p_db_syslog_cfg->timestamp = p_syslog_cfg->timestamp;
        break;

    case TBL_SYSLOG_CFG_FLD_FILE_SEVERITY:
        p_db_syslog_cfg->file_severity = p_syslog_cfg->file_severity;
        break;

    case TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY:
        p_db_syslog_cfg->module_severity = p_syslog_cfg->module_severity;
        break;

    case TBL_SYSLOG_CFG_FLD_ENABLE_MERGE:
        p_db_syslog_cfg->enable_merge = p_syslog_cfg->enable_merge;
        break;

    case TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE:
        p_db_syslog_cfg->enable_operate = p_syslog_cfg->enable_operate;
        break;

    case TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT:
        p_db_syslog_cfg->merge_timeout = p_syslog_cfg->merge_timeout;
        break;

    case TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE:
        p_db_syslog_cfg->merge_fifosize = p_syslog_cfg->merge_fifosize;
        break;

    case TBL_SYSLOG_CFG_FLD_MAX:
        sal_memcpy(p_db_syslog_cfg, p_syslog_cfg, sizeof(tbl_syslog_cfg_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_SYSLOG_CFG, field_id, p_db_syslog_cfg);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_SYSLOG_CFG, field_id, p_db_syslog_cfg);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_syslog_cfg_set_syslog_cfg_field(tbl_syslog_cfg_t *p_syslog_cfg, tbl_syslog_cfg_field_id_t field_id)
{
    return tbl_syslog_cfg_set_syslog_cfg_field_sync(p_syslog_cfg, field_id, TRUE);
}

tbl_syslog_cfg_t*
tbl_syslog_cfg_get_syslog_cfg()
{
    return _g_p_tbl_syslog_cfg;
}

int32
tbl_syslog_cfg_key_str2val(char *str, tbl_syslog_cfg_t *p_syslog_cfg)
{
    return PM_E_NONE;
}

char*
tbl_syslog_cfg_key_name_dump(tbl_syslog_cfg_t *p_syslog_cfg, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYSLOG_CFG);
    sal_sprintf(str, "%s"KEY_CONNECT_STR"0",
        p_node->name);
    return str;
}

char*
tbl_syslog_cfg_key_value_dump(tbl_syslog_cfg_t *p_syslog_cfg, char *str)
{

    sal_sprintf(str, "%s", "0");
    return str;
}

char*
tbl_syslog_cfg_field_name_dump(tbl_syslog_cfg_t *p_syslog_cfg, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_SYSLOG_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_SYSLOG_CFG_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_syslog_cfg_field_value_dump(tbl_syslog_cfg_t *p_syslog_cfg, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_to_server);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_to_file);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_to_trap);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_to_diag);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_LOGGING_LINES, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->logging_lines);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_ADDR, field_id))
    {
        sal_sprintf(str, "%s", p_syslog_cfg->server_addr);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_FACILITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, p_syslog_cfg->server_facility));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->server_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_FACILITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, p_syslog_cfg->trap_facility));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->trap_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TIMESTAMP, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, p_syslog_cfg->timestamp));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_FILE_SEVERITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->file_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY, field_id))
    {
        sal_sprintf(str, "%s", cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->module_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_MERGE, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_merge);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->enable_operate);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->merge_timeout);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE, field_id))
    {
        sal_sprintf(str, "%u", p_syslog_cfg->merge_fifosize);
    }
    return str;
}

char**
tbl_syslog_cfg_table_dump(tbl_syslog_cfg_t *p_syslog_cfg, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_syslog_cfg_key_name_dump(p_syslog_cfg, buf));
    for(i=1; i<TBL_SYSLOG_CFG_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_syslog_cfg_field_name_dump(p_syslog_cfg, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_syslog_cfg_field_value_dump(p_syslog_cfg, i, buf));
    }
    return str;
}

int32
tbl_syslog_cfg_field_value_parser(char *str, int32 field_id, tbl_syslog_cfg_t *p_syslog_cfg)
{
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_to_server = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_to_file = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_to_trap = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_to_diag = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_LOGGING_LINES, field_id))
    {
        int32 ret;
        p_syslog_cfg->logging_lines = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_ADDR, field_id))
    {
        sal_strcpy(p_syslog_cfg->server_addr, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_FACILITY, field_id))
    {
        cdb_enum_str2val(syslog_facility_strs, SYSLOG_FACILITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY, field_id))
    {
        cdb_enum_str2val(syslog_severity_strs, SYSLOG_SEVERITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_FACILITY, field_id))
    {
        cdb_enum_str2val(syslog_facility_strs, SYSLOG_FACILITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY, field_id))
    {
        cdb_enum_str2val(syslog_severity_strs, SYSLOG_SEVERITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TIMESTAMP, field_id))
    {
        cdb_enum_str2val(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_FILE_SEVERITY, field_id))
    {
        cdb_enum_str2val(syslog_severity_strs, SYSLOG_SEVERITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY, field_id))
    {
        cdb_enum_str2val(syslog_severity_strs, SYSLOG_SEVERITY_MAX, str);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_MERGE, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_merge = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE, field_id))
    {
        int32 ret;
        p_syslog_cfg->enable_operate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT, field_id))
    {
        int32 ret;
        p_syslog_cfg->merge_timeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE, field_id))
    {
        int32 ret;
        p_syslog_cfg->merge_fifosize = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_syslog_cfg_table_parser(char** array, char* key_value,tbl_syslog_cfg_t *p_syslog_cfg)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYSLOG_CFG);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_syslog_cfg_key_str2val(key_value, p_syslog_cfg));

    for(i=1; i<TBL_SYSLOG_CFG_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_syslog_cfg_field_value_parser( array[j++], i, p_syslog_cfg));
    }

    return PM_E_NONE;
}

int32
tbl_syslog_cfg_dump_one(tbl_syslog_cfg_t *p_syslog_cfg, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_SYSLOG_CFG);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER].name,
            p_syslog_cfg->enable_to_server);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE].name,
            p_syslog_cfg->enable_to_file);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP].name,
            p_syslog_cfg->enable_to_trap);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG].name,
            p_syslog_cfg->enable_to_diag);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_LOGGING_LINES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_LOGGING_LINES].name,
            p_syslog_cfg->logging_lines);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_ADDR, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_SERVER_ADDR].name,
            p_syslog_cfg->server_addr);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_FACILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_SERVER_FACILITY].name, 
            cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, p_syslog_cfg->server_facility));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY].name, 
            cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->server_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_FACILITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_TRAP_FACILITY].name, 
            cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, p_syslog_cfg->trap_facility));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY].name, 
            cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->trap_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_TIMESTAMP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_TIMESTAMP].name, 
            cdb_enum_val2str(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, p_syslog_cfg->timestamp));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_FILE_SEVERITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_FILE_SEVERITY].name, 
            cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->file_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY].name, 
            cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, p_syslog_cfg->module_severity));
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_MERGE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_MERGE].name,
            p_syslog_cfg->enable_merge);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE].name,
            p_syslog_cfg->enable_operate);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT].name,
            p_syslog_cfg->merge_timeout);
    }
    if (FLD_MATCH(TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE].name,
            p_syslog_cfg->merge_fifosize);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_syslog_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_syslog_cfg_t *p_db_syslog_cfg = _g_p_tbl_syslog_cfg;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_syslog_cfg)
    {
        return PM_E_NONE;
    }

    rc = fn(p_db_syslog_cfg, pargs);
    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_syslog_cfg_t*
tbl_syslog_cfg_init_syslog_cfg()
{
    _g_p_tbl_syslog_cfg = XCALLOC(MEM_TBL_SYSLOG_CFG, sizeof(tbl_syslog_cfg_t));
    return _g_p_tbl_syslog_cfg;
}

