
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#include "proto.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "appcfg.h"
#include "app_syslog.h"
#include "logging.h"

extern int logging_process_cfgfile(tbl_syslog_cfg_t *pCfg, E_ActionType eActionType);
extern int app_is_startup_done();
static int g_can_restart = 0;

int
_app_syslogd_restart(void)
{    
    char szCmd[256];

    snprintf(szCmd, 256, "kill -%d `cat /var/run/syslog-ng.pid`", SIGUSR2);
    return system(szCmd);
}

int
_app_syslogd_generate_cfg(char *pszErrMsg, size_t nBufLen, E_ActionType action)
{
    tbl_syslog_cfg_t *pCfg = tbl_syslog_cfg_get_syslog_cfg();
    FILE *fp = NULL;
    int ret = -1;

    assert(NULL != pCfg);
    if ((fp = fopen(LOGGING_CFGFILE_DEFAULT, "w")) == NULL) {
        if (pszErrMsg) {
            snprintf(pszErrMsg, nBufLen, 
                "Can't open syslogd configuration file: %s",
                strerror(errno));
        }
        goto err_out;
    }

    ret = logging_process_cfgfile(pCfg, action);
    
err_out:
    if (NULL != fp) {
        fclose(fp);
    }
    return ret;
}

static int
_app_syslog_set(tbl_syslog_cfg_field_id_t cfgid, tbl_syslog_cfg_t *p_cfg, E_ActionType action)
{
    int32 rc = PM_E_NONE;
    char szErrMsg[256];
        
    PM_E_RETURN(tbl_syslog_cfg_set_syslog_cfg_field(p_cfg, cfgid));
    if (g_can_restart) {
        rc = _app_syslogd_generate_cfg(szErrMsg, 256, action);
        rc = _app_syslogd_restart();
    }
    return rc;
}

int32
app_syslog_set_enable_merge(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_MERGE, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_merge_timeout(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_merge_fifosize(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_enable_server(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_enable_file(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_enable_operate(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_OPERATE, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_enable_trap(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_enable_diag(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG, p_cfg, 
        p_cfg->enable_to_diag ? E_ACTION_DIAG : E_ACTION_DIAG_SHUTDOWN);
}

int32
app_syslog_set_bufsz(tbl_syslog_cfg_t *p_cfg)
{
    logging_set_bufsz(p_cfg->logging_lines);
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_LOGGING_LINES, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_server_addr(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_SERVER_ADDR, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_server_facility(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_SERVER_FACILITY, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_server_severity(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_trap_facility(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_TRAP_FACILITY, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_trap_severity(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_timestamp(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_TIMESTAMP, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_file_severity(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_FILE_SEVERITY, p_cfg, E_ACTION_RUNNING);
}

int32
app_syslog_set_module_severity(tbl_syslog_cfg_t *p_cfg)
{
    return _app_syslog_set(TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY, p_cfg, E_ACTION_RUNNING);
}

extern int logging_module_init();
int32
app_syslog_init()
{   
    tbl_syslog_cfg_t stCfg;
    tbl_syslog_cfg_t *pCfg = &stCfg;
    
    memset(pCfg, 0x0, sizeof(tbl_syslog_cfg_t));
    pCfg->enable_to_server = 0;
    pCfg->enable_to_file = 1;
    pCfg->enable_to_trap = 1;
    pCfg->enable_to_diag = 1;
    pCfg->logging_lines = LOGGING_BUF_LINENUM_DEFAULT;
    pCfg->enable_merge = 1;
    pCfg->merge_fifosize = LOGGING_MERGE_DEFAULT_FIFO_SIZE;
    pCfg->merge_timeout = LOGGING_MERGE_DEFAULT_TIMEOUT / 1000;

    pCfg->file_severity = LOGGING_FILE_SEVERITY_DEFAULT;
    pCfg->module_severity = LOGGING_MOD_SEVERITY_DEFAULT;
    pCfg->server_severity = LOGGING_SRV_SEVERITY_DEFAULT; 
    pCfg->server_facility = SYSLOG_FACILITY_LOCAL4;
    pCfg->timestamp = SYSLOG_TIMESTAMP_BSD;
    pCfg->trap_facility = SYSLOG_FACILITY_LOCAL6;
    pCfg->trap_severity = LOG_ALERT;

    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_ENABLE_TO_SERVER);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_ENABLE_TO_FILE);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_ENABLE_TO_TRAP);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_ENABLE_TO_DIAG);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_LOGGING_LINES);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_ENABLE_MERGE);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_MERGE_TIMEOUT);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_MERGE_FIFOSIZE);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_FILE_SEVERITY);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_MODULE_SEVERITY);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_SERVER_SEVERITY);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_SERVER_FACILITY);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_TIMESTAMP);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_TRAP_FACILITY);
    tbl_syslog_cfg_set_syslog_cfg_field(pCfg, TBL_SYSLOG_CFG_FLD_TRAP_SEVERITY);
    logging_module_init();
    return PM_E_NONE;
}

extern int redirect_kernel_log(void);

int32
app_syslog_proc_startup_done()
{
    char szErrMsg[256];

    redirect_kernel_log();
    g_can_restart = 1;   
    _app_syslogd_generate_cfg(szErrMsg, 256, E_ACTION_RUNNING);
    _app_syslogd_restart(); 

    return PM_E_NONE;
}

