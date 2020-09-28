
#include <assert.h>
#include <string.h>

#include "proto.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "appcfg.h"
#include "app_syslog.h"
#include "lib_fs.h"
#include "cfg_cmd.h"
#include "logging.h"

#define M_LOGFILE     "/mnt/flash/syslog "
extern int logging_clear_buf();
extern int _app_syslogd_restart(void);
extern int _app_syslogd_generate_cfg(char *pszErrMsg, size_t nBufLen, E_ActionType action);

static int32
_app_syslog_cmd_get_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    S_LogStatistics *pLogStats = logging_get_stat();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "/total"CMD_EQUAL_STR"%llu", pLogStats->ullTotalLogCnts);
    sal_fprintf(fp, "/dropped"CMD_EQUAL_STR"%llu", pLogStats->ullDropLogCnts);
    sal_fprintf(fp, "/current"CMD_EQUAL_STR"%lu", pLogStats->ulCurLogCnts);
    sal_fclose(fp);
    return PM_E_NONE;
}

static int
_app_show_log_item(void *pArg, const char *pszMsg)
{
    FILE *fp = (FILE *)pArg;
    write(fileno(fp), pszMsg, strlen(pszMsg));
    return 0;
}

static int32
_app_syslog_cmd_get_logs(cfg_cmd_para_t *para, int nLines)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    logging_show_records(nLines, _app_show_log_item, fp);
    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_app_syslog_cmd_get(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_syslog_cfg_iterate((TBL_ITER_CB_FUNC)tbl_syslog_cfg_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_syslog_cmd_set_enable_server(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_to_server == p_cfg->enable_to_server)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_server(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_enable_merge(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_merge == p_cfg->enable_merge)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_merge(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_merge_timeout(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->merge_timeout == p_cfg->merge_timeout)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_merge_timeout(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_merge_fifosize(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->merge_fifosize == p_cfg->merge_fifosize)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_merge_fifosize(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_enable_operate(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_operate == p_cfg->enable_operate)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_operate(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_enable_file(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_to_file == p_cfg->enable_to_file)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_file(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_enable_trap(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_to_trap == p_cfg->enable_to_trap)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_trap(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_enable_diag(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->enable_to_diag == p_cfg->enable_to_diag)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_enable_diag(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_bufsz(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (p_db_cfg->logging_lines < LOGGING_BUF_LINENUM_MIN 
            || p_db_cfg->logging_lines > LOGGING_BUF_LINENUM_MAX) {
        return PM_E_CFG_INVALID_PARAM;
    }

    /* 2. check duplicate config */
    if (p_db_cfg->logging_lines == p_cfg->logging_lines)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(app_syslog_set_bufsz(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_server_addr(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (strstr(p_db_cfg->server_addr, p_cfg->server_addr))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_server_addr(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_server_facility(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if ((p_db_cfg->server_facility > SYSLOG_FACILITY_FTP 
                && p_db_cfg->server_facility < SYSLOG_FACILITY_LOCAL0) 
            || p_db_cfg->server_facility > SYSLOG_FACILITY_LOCAL7) {
        return PM_E_CFG_INVALID_PARAM;
    }

    /* 2. check duplicate config */
    if (p_db_cfg->server_facility == p_cfg->server_facility)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_server_facility(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_server_severity(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_cfg->server_severity > SYSLOG_SEVERITY_DEBUG) {
        return PM_E_CFG_INVALID_PARAM;
    }    

    /* 2. check duplicate config */
    if (p_db_cfg->server_severity == p_cfg->server_severity)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_server_severity(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_trap_facility(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->trap_facility == p_cfg->trap_facility)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_trap_facility(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_trap_severity(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (p_db_cfg->trap_severity == p_cfg->trap_severity)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_trap_severity(p_cfg));
    return rc;
}


static int32
_app_syslog_cmd_set_timestamp(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_cfg->timestamp > SYSLOG_TIMESTAMP_RFULL) {
        return PM_E_CFG_INVALID_PARAM;
    }

    /* 2. check duplicate config */
    if (p_db_cfg->timestamp == p_cfg->timestamp)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_timestamp(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_file_severity(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_cfg->file_severity > SYSLOG_SEVERITY_DEBUG) {
        return PM_E_CFG_INVALID_PARAM;
    }

    /* 2. check duplicate config */
    if (p_db_cfg->file_severity == p_cfg->file_severity)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_file_severity(p_cfg));
    return rc;
}

static int32
_app_syslog_cmd_set_module_severity(tbl_syslog_cfg_t *p_cfg, tbl_syslog_cfg_t *p_db_cfg, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (p_db_cfg->module_severity > SYSLOG_SEVERITY_DEBUG) {
        return PM_E_CFG_INVALID_PARAM;
    }

    /* 2. check duplicate config */
    if (p_db_cfg->module_severity == p_cfg->module_severity)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(app_syslog_set_module_severity(p_cfg));
    return rc;
}

static int
_app_syslog_check_log_lines(char *pszLine, int *pnLines)
{
    int nlines;
    
    if (!strcasecmp(pszLine, "-")) {
        *pnLines = -LOGGING_BUF_SHOWNUM_DEFAULT;
    } else if (!strcasecmp(pszLine, "+")) {
        *pnLines = LOGGING_BUF_SHOWNUM_DEFAULT;
    } else {
        int nLen = strlen(pszLine);
        if ((strspn(pszLine, "0123456789+-") != nLen)
            || (nLen > 1 && (strspn(pszLine + 1, "0123456789") != (nLen - 1))))
        {
            return -1;
        }
        
        nlines = atoi(pszLine);
        if (nlines > LOGGING_BUF_SHOWNUM_MAX || nlines < LOGGING_BUF_SHOWNUM_MIN) {
            return -1;
        }
        *pnLines = nlines;
    }
    return 0;
}

int32
app_syslog_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 field_id = TBL_SYSLOG_CFG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    int32 value = 0;
    field_parse_t field;
    tbl_syslog_cfg_t cfg;
    tbl_syslog_cfg_t *p_db_cfg = tbl_syslog_cfg_get_syslog_cfg();
    //uint32 parse_field = TRUE;
    int32 rc = PM_E_NONE;
    gensock_t gsock;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SYSLOG_CFG);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        if (!sal_strcmp(argv[0], "clearbuf") && 1 == argc) {
            logging_clear_buf();
        } else if (!sal_strcmp(argv[0], "server_addr") && 3 == argc) {
            if ((strspn(argv[1], "0123456789") != strlen(argv[1]))
                    || atoi(argv[1]) > 64) {
                CFG_INVALID_PARAM_RET("Invalid vrf id: %s", argv[1]);
            }
            if (str2gensock(&gsock,  argv[2]) != 0) {
                CFG_INVALID_PARAM_RET("Invalid server address: %s", argv[2]);
            }
            value = atoi(argv[1]);
            if (logging_server_del_server_address(p_db_cfg, value, argv[2], 
                    para->p_rs->desc, MAX_CMD_RET_LEN) != 0) {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return PM_E_CFG_INVALID_PARAM;
            }
            rc = app_syslog_set_server_addr(p_db_cfg);
        } else {
            CFG_INVALID_PARAM_RET("Invalid delete command: argc %d", argc);
        }
        break;

    case CDB_OPER_SET:
        sal_memset(&cfg, 0, sizeof(cfg));
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        if (TBL_SYSLOG_CFG_FLD_SERVER_ADDR == field_id) {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        } else {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        }
        switch ((tbl_syslog_cfg_field_id_t)field_id)
        {     
        case TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT:
            value = atoi(argv[i+1]);
            if (strlen(argv[i+1]) != strspn(argv[i+1], "0123456789")
                    || value < (LOGGING_MERGE_MIN_TIMEOUT/1000)
                    || value > (LOGGING_MERGE_MAX_TIMEOUT/1000)) {
                CFG_INVALID_PARAM_RET("Invalid merge timeout value: %s", argv[i+1]);
            }
            cfg.merge_timeout = value;
            rc = _app_syslog_cmd_set_merge_timeout(&cfg, p_db_cfg, para);
            break;
        case TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE:
            value = atoi(argv[i+1]);
            if (strlen(argv[i+1]) != strspn(argv[i+1], "0123456789")
                    || value < LOGGING_MERGE_MIN_FIFO_SIZE 
                    || value > LOGGING_MERGE_MAX_FIFO_SIZE) {
                CFG_INVALID_PARAM_RET("Invalid merge fifo-size value: %s", argv[i+1]);
            }
            cfg.merge_fifosize = value;
            rc = _app_syslog_cmd_set_merge_fifosize(&cfg, p_db_cfg, para);
            break;
        case TBL_SYSLOG_CFG_FLD_ENABLE_MERGE:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_merge = value;
            rc = _app_syslog_cmd_set_enable_merge(&cfg, p_db_cfg, para);
            break;
        case TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_to_server = value;
            rc = _app_syslog_cmd_set_enable_server(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_to_file = value;
            rc = _app_syslog_cmd_set_enable_file(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_operate = value;
            rc = _app_syslog_cmd_set_enable_operate(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_to_trap = value;
            rc = _app_syslog_cmd_set_enable_trap(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG:
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            cfg.enable_to_diag = value;
            rc = _app_syslog_cmd_set_enable_diag(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_LOGGING_LINES:
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            cfg.logging_lines = value;
            rc = _app_syslog_cmd_set_bufsz(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_SERVER_ADDR:
            if (3 != argc) {
                CFG_INVALID_PARAM_RET("Invalid server address parameters: format vrfid/ipaddr");
            }
            if ((strspn(argv[i+1], "0123456789") != strlen(argv[i+1]))
                    || atoi(argv[i+1]) > 64) {
                CFG_INVALID_PARAM_RET("Invalid vrf id: %s", argv[i+1]);
            }
            if (str2gensock(&gsock,  argv[i+2]) != 0) {
                CFG_INVALID_PARAM_RET("Invalid server address: %s", argv[i+2]);
            }
            if ('\0' != p_db_cfg->server_addr[0]) {
                snprintf(cfg.server_addr, M_SRV_ADDR_BUF_SZ, "%s", p_db_cfg->server_addr);
            }
            value = atoi(argv[i+1]);
            
            if (logging_server_add_server_address(&cfg, value, argv[i+2], 
                    para->p_rs->desc, MAX_CMD_RET_LEN) != 0) {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return PM_E_CFG_INVALID_PARAM;
            }
            rc = _app_syslog_cmd_set_server_addr(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_SERVER_FACILITY:
            if (strspn(argv[i+1], "0123456789") == strlen(argv[i+1])) {
                value = atoi(argv[i+1]);
                if ((value > SYSLOG_FACILITY_FTP && value < SYSLOG_FACILITY_LOCAL0) 
                        || value > SYSLOG_FACILITY_LOCAL7) {
                    CFG_INVALID_PARAM_RET("Invalid server facility: %s", argv[i+1]);
                }
            } else {
                PM_CFG_GET_ENUM(value, syslog_facility_strs, SYSLOG_FACILITY_MAX, argv[i+1], para->p_rs);
            }
            cfg.server_facility = value;
            rc = _app_syslog_cmd_set_server_facility(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY:
            PM_CFG_GET_ENUM(value, syslog_severity_strs, SYSLOG_SEVERITY_MAX, argv[i+1], para->p_rs);
            cfg.server_severity = value;
            rc = _app_syslog_cmd_set_server_severity(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_TRAP_FACILITY:
            PM_CFG_GET_ENUM(value, syslog_facility_strs, SYSLOG_FACILITY_MAX, argv[i+1], para->p_rs);
            cfg.trap_facility = value;
            rc = _app_syslog_cmd_set_trap_facility(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY:
            PM_CFG_GET_ENUM(value, syslog_severity_strs, SYSLOG_SEVERITY_MAX, argv[i+1], para->p_rs);
            cfg.trap_severity = value;
            rc = _app_syslog_cmd_set_trap_severity(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_TIMESTAMP:
            PM_CFG_GET_ENUM(value, syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, argv[i+1], para->p_rs);
            cfg.timestamp = value;
            rc = _app_syslog_cmd_set_timestamp(&cfg, p_db_cfg, para);
            break;

        case TBL_SYSLOG_CFG_FLD_FILE_SEVERITY:
            PM_CFG_GET_ENUM(value, syslog_severity_strs, SYSLOG_SEVERITY_MAX, argv[i+1], para->p_rs);
            cfg.file_severity = value;
            rc = _app_syslog_cmd_set_file_severity(&cfg, p_db_cfg, para);
            //rc += app_syslog_set_quagga_log_file_severity(argv[i+1]);
            break;

        case TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY:
            PM_CFG_GET_ENUM(value, syslog_severity_strs, SYSLOG_SEVERITY_MAX, argv[i+1], para->p_rs);
            cfg.module_severity = value;
            rc = _app_syslog_cmd_set_module_severity(&cfg, p_db_cfg, para);
            //rc += app_syslog_set_quagga_log_module_severity(argv[i+1]);
            break;
            
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && !strcasecmp(argv[0], "statistics")) {
            rc = _app_syslog_cmd_get_statistics(para);
        } else if (2 == argc && !strcasecmp(argv[0], "buffer")
                && strspn(argv[1], "0123456789-+") == strlen(argv[1])) {
            if (_app_syslog_check_log_lines(argv[1], &value) != 0) {
                CFG_INVALID_PARAM_RET("Invalid parameter %s", argv[1]);
            }
            rc = _app_syslog_cmd_get_logs(para, value);
        } else {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }

            rc = _app_syslog_cmd_get(para, p_node, &field);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
app_syslog_terminal_monitor(int onoff, char *pszTty)
{
    if (0 == logging_terminal_monitor(onoff, pszTty)) {
        char szErrMsg[256];

        _app_syslogd_generate_cfg(szErrMsg, 256, E_ACTION_RUNNING);
        _app_syslogd_restart();
        return 0;
    }
    
    return -1;
}

int32
app_syslog_act_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char szTty[PATH_MAX];
    int i;
    int len;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (argc <= 2
                || (strcasecmp(argv[0], "on") && strcasecmp(argv[0], "off"))
                || strcasecmp(argv[1], "dev")) {
            CFG_INVALID_PARAM_RET("Invalid parameter");
        }
        len = 0;
        for (i = 1; i < argc; i++) {
            len += snprintf(szTty + len, PATH_MAX - len, "/%s", argv[i]);
        }

        if (!lib_fs_is_chardev(szTty)) {
            CFG_INVALID_PARAM_RET("Invalid tty name %s", szTty);
        }

        app_syslog_terminal_monitor((!strcasecmp(argv[0], "on")) ? 1 : 0, szTty);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

