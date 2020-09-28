
#include "proto.h"
#include "appcfg.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_cpu_utilization_define.h"
#include "gen/tbl_cpu_utilization.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_login_define.h"
#include "gen/tbl_login.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_rmon_ether_stats_define.h"
#include "gen/tbl_rmon_ether_stats.h"
#include "gen/tbl_rmon_history_define.h"
#include "gen/tbl_rmon_history.h"
#include "gen/tbl_rmon_stats_history_define.h"
#include "gen/tbl_rmon_stats_history.h"
#include "gen/tbl_port_stats_define.h"
#include "gen/tbl_port_stats.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_app_debug.h"
#include "app_cem.h"
#include "app_dns.h"
#include "app_ssh.h"
#include "app_syslog.h"
#include "app_auth.h"
#include "app_ntp.h"
#include "app_snmp.h"
#include "app_rmon.h"
#include "app_clock.h"
#include "app_cpu_mem.h"

extern int rsa_init_crypto();
extern int login_session_del_all_web_session();

int app_is_startup_done()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->startup_done) {
        return 1;
    }
    return 0;
}

int app_is_aaa_new_model()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->aaa_new_model) {
        return 1;
    }
    return 0;
}

int app_is_password_encryption()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->service_password_encryption) {
        return 1;
    }
    return 0;
}

int app_get_max_vty()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
	
    if (pGlobal && pGlobal->startup_done) {
        /* check start up done for load start up config */
        return pGlobal->max_vty;
    }
    return GLB_MAX_VTY_NUM;
}


/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
        sal_printf("CLI utility. Command line shell."
            "The part of the klish project.\n");
        sal_printf("Options:\n");
        sal_printf("\t-v, --version\tPrint version.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
    }
}

int32
appcfg_cpu_init()
{
    tbl_mem_summary_t* p_mem_info = NULL;
    p_mem_info = tbl_mem_summary_get_mem_summary();
    p_mem_info->cpu_low_threshold = APPCFG_CPU_DEFULT_LOW_THRESHOLD;
    tbl_mem_summary_set_mem_summary_field(p_mem_info,TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD);
    p_mem_info->cpu_high_threshold = APPCFG_CPU_DEFULT_HIGH_THRESHOLD;
    tbl_mem_summary_set_mem_summary_field(p_mem_info,TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD);
    ctc_task_add_timer(TASK_PRI_NORMAL, app_cpu_mem_polling_timer_fn, NULL, CPU_MEM_POLLING_INTERVAL);
    ctc_task_add_timer(TASK_PRI_NORMAL, app_cpu_usage_polling_timer_fn, NULL, CPU_USAGE_POLLING_INTERVAL);
    return PM_E_NONE;
}

int32
appcfg_sock_init()
{
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(10);

    ipc_connect_snmp_trap();
    
    return PM_E_NONE;
}

extern int32
app_clock_proc_init_done();

static int32
appcfg_process_sys_global(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *pGlobal = (tbl_sys_global_t *)p_tbl;

    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_SYS_GLOBAL_FLD_INIT_DONE == field_id) {
            appcfg_cpu_init();
            app_clock_proc_init_done();
        } else if (TBL_SYS_GLOBAL_FLD_STARTUP_DONE == field_id) {

            app_ssh_proc_startup_done();
            app_syslog_proc_startup_done();
            app_snmp_proc_startup_done();
            app_clock_proc_startup_done();
        } else if (TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION == field_id) {
            app_auth_password_encryption(pGlobal->service_password_encryption);
        } else if (TBL_SYS_GLOBAL_FLD_HTTP_DISABLE == field_id) {
            if (pGlobal->http_disable)
            {
                login_session_del_all_web_session();
            }
        } 

        break;

    case CDB_OPER_GET:

        break;

    default:
        break;
    }
    
    return rc;

}

int32
appcfg_one_minute_reboot_alarm_notify()
{
    tbl_login_master_t *p_master = tbl_login_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_login_t *p_db_login = NULL;
    int fd = -1;
    const char *fn_name = "\n System will reboot by reboot schedule in 1 minute. ";
    char tty_fname[CMD_BUF_64];
    char *p = NULL;

    CTCLIB_SLIST_LOOP(p_master->login_list, p_db_login, listnode)
    {
        sal_snprintf(tty_fname, CMD_BUF_64, "/dev/%s", p_db_login->key.name);
        p = sal_strchr(tty_fname, '-');
        if (p)
        {
            p[0] = '/';
        }
        fd = sal_open(tty_fname, O_WRONLY);
        if (fd < 0)
        {
            continue;
        }
        sal_write(fd, fn_name, sal_strlen(fn_name));
        sal_close(fd);
    }

    return PM_E_NONE;
}

static int32
appcfg_process_chassis(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    switch (oper)
    {
    case CDB_OPER_SET:
        if (TBL_CHASSIS_FLD_ONE_MINUTE_REBOOT_ALARM == field_id) 
        {
            appcfg_one_minute_reboot_alarm_notify();
        }
        break;

    default:
        break;
    }
    
    return PM_E_NONE;
}

static int32
appcfg_process_port_stats(cdb_node_t* p_tbl_node, 
    cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_port_stats_key_t *p_port_stats_key = NULL;
    tbl_port_stats_t *p_port_stats = NULL;
    tbl_interface_t  *p_if = NULL;

    p_port_stats_key = (tbl_port_stats_key_t*)p_tbl;
    p_if = tbl_interface_get_interface_by_ifindex(p_port_stats_key->ifindex);
    if (NULL == p_if)
    {
        return PM_E_NONE;
    }
    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return PM_E_NONE;
    }

    p_port_stats = tbl_port_stats_get_port_stats(p_port_stats_key);

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_SET:
        rmon_api_update_ether_stats(p_if, p_port_stats, field_id);
        break;
    case CDB_OPER_GET:
        break;
    case CDB_OPER_DEL:
        break;
    default :
        break;
    }

    return rc;
}

int32
app_cmd_get_app_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_app_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_app_debug_get_app_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_app_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

/*
        config_type: 0 - debug level  -- 0 err, 1 warning, 2 dbg
                          1 - netconf rpc enable
                          2 - netconf rpc statistic
        confg format: config_type=2,config_val=1
      */
int32 app_netconf_process_debug(tbl_app_debug_field_id_t field, int32 value, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char *pipe_name = "/tmp/netconf_debug";
    char debug_format[] = "config_type=%d,config_val=%d";
    char debug_config[32] = {0};
    int pipe_fd, res, len;
    int config_type, config_val;
    uint8 is_uml = 0;
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();

    #ifdef _GLB_UML_SYSTEM_        
        is_uml = 1;
    #endif

    if (field == TBL_APP_DEBUG_FLD_NETCONF_RPC)
    {
        config_type = 1;
        config_val = value;
    }
    else if (field == TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR)
    {
        config_type = 2;
        config_val = value;
    }
    else if (field == TBL_APP_DEBUG_FLD_NETCONF_DEBUG)
    {
        config_type = 0;
        config_val = value ? 2 : 0;
    }
    else
    {
        return rc;
    }

    /* check if netconf enable */
    if (!p_db_cfg->netconf_service && !is_uml)
    {
        CFG_CONFLICT_RET("Please enable netconf service firstly.");
    }
    /* open pipe file, we use nonblock mode, because we don't expect block in here if there is no read side */
    pipe_fd = open(pipe_name, O_WRONLY | O_NONBLOCK);
    if (pipe_fd < 0)
    {
        CFG_FUNC_ERR_RET("Appcfg open pipe file %s fail.", pipe_name);
    }
    /* write config buffer */
    len = sprintf(debug_config, debug_format, config_type , config_val);
    res = write(pipe_fd, debug_config, len);
    if (res != len)
    {
        close(pipe_fd);
        CFG_FUNC_ERR_RET("Appcfg write pipe file %s fail.", pipe_name);
    }
    
    return rc;
}

#define DBG_APP_TYPE_FIELD       0
#define DBG_APP_TYPE_ALL         1

int32
app_cmd_process_app_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_app_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_app_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    uint32 multi_type = DBG_APP_TYPE_FIELD;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_APP_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_app_debug_get_app_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("APP_DEBUG not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_APP_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }

        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_APP_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_APP_DEBUG_FLD_SNMP_AGENT:
                GLB_SET_FLAG_COND(value, dbg.snmp, APPDBG_FLAG_SNMP_AGENT);
                break;
            case TBL_APP_DEBUG_FLD_RMON_EVENT:
                GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_EVENT);
                break;
            case TBL_APP_DEBUG_FLD_RMON_ALARM:
                GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_ALARM);
                break;
            case TBL_APP_DEBUG_FLD_RMON_STATS:
                GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_STATS);
                break;
            case TBL_APP_DEBUG_FLD_RMON_HISTORY:
                GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_HISTORY);
                break;
            case TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL:
                GLB_SET_FLAG_COND(value, dbg.ntp, APPDBG_FLAG_NTP_DEBUG_LEVEL);
                break;
            case TBL_APP_DEBUG_FLD_NETCONF_RPC:
                GLB_SET_FLAG_COND(value, dbg.netconf, APPDBG_FLAG_NETCONF_RPC);
                PM_E_RETURN(app_netconf_process_debug(TBL_APP_DEBUG_FLD_NETCONF_RPC, value, para));
                break;
            case TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR:
                GLB_SET_FLAG_COND(value, dbg.netconf, APPDBG_FLAG_NETCONF_RPC_CLEAR);
                PM_E_RETURN(app_netconf_process_debug(TBL_APP_DEBUG_FLD_NETCONF_RPC_CLEAR, value, para));
                break;
            case TBL_APP_DEBUG_FLD_NETCONF_DEBUG:
                GLB_SET_FLAG_COND(value, dbg.netconf, APPDBG_FLAG_NETCONF_DEBUG);
                PM_E_RETURN(app_netconf_process_debug(TBL_APP_DEBUG_FLD_NETCONF_DEBUG, value, para));
                break;
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_app_debug_set_app_debug_field(&dbg, field_id);
        }
        else if (DBG_APP_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.snmp, APPDBG_FLAG_SNMP_AGENT);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_SNMP_AGENT);

            GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_EVENT);
            GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_ALARM);
            GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_STATS);
            GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_RMON_HISTORY);
            GLB_SET_FLAG_COND(value, dbg.rmon, APPDBG_FLAG_NTP_DEBUG_LEVEL);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_RMON_EVENT);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_RMON_ALARM);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_RMON_STATS);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_RMON_HISTORY);
            rc = tbl_app_debug_set_app_debug_field(&dbg, TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL);
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = app_cmd_get_app_debug(para, p_node, &field);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid path");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
appcfg_module_init()
{
    cdb_register_cfg_tbl_cb(TBL_APP_DEBUG, app_cmd_process_app_debug);
    cdb_register_cfg_tbl_cb(TBL_SSH_CFG, app_ssh_cmd_process_ssh);
    cdb_register_cfg_tbl_cb(TBL_SYSLOG_CFG, app_syslog_cmd_process);
    cdb_register_cfg_act_cb(ACT_TERMINAL_MONITOR, app_syslog_act_process);    
    cdb_register_cfg_act_cb(ACT_WEB_LOGIN, app_web_login_act_process);
    cdb_register_cfg_act_cb(ACT_SEND_TRAP, app_send_trap_act_process);
    cdb_register_cfg_tbl_cb(TBL_NTP_CFG, app_ntp_normal_cfg_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NTP_KEY, app_ntp_trustkey_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NTP_SERVER, app_ntp_server_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NTP_IF, app_ntp_interface_cfg_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NTP_ACE, app_ntp_ace_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_CFG, app_snmp_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_VIEW, app_snmp_view_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_COMMUNITY, app_snmp_community_cmd_process);
    
    cdb_register_cfg_tbl_cb(TBL_SNMP_CONTEXT, app_snmp_context_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_USM_USER, app_snmp_usm_user_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_GROUP_USM, app_snmp_group_usm_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_ACCESS_USM, app_snmp_access_usm_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_NOTIFY, app_snmp_notify_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_TARGET_ADDR, app_snmp_target_addr_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_TARGET_PARAMS, app_snmp_target_params_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_USER, app_user_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_LOGIN_SECURITY_GLB, app_login_security_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_LOGIN_RECORD, app_login_record_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CONSOLE, app_console_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_VTY, app_vty_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ENABLE, app_enable_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_AUTHEN, app_authen_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_AUTHOR, app_author_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACCOUNT, app_account_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACCOUNTCMD, app_accountcmd_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_LOGIN, app_login_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_RSA, app_rsa_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_STATIC_DNS, app_static_dns_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_DYNAMIC_DNS_DOMAIN, app_dynamic_dns_domain_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_DYNAMIC_DNS_SERVER, app_dynamic_dns_server_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_TRAP, app_snmp_trap_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_SNMP_INFORM, app_snmp_inform_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CEM, app_event_manager_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_CPU_UTILIZATION, app_cmd_process_cpu_utilization);
    cdb_register_cfg_tbl_cb(TBL_CPU_LIMIT, app_cmd_process_cpu_limit);
    cdb_register_cfg_tbl_cb(TBL_MEM_SUMMARY, app_cmd_process_mem_summary);
    cdb_register_cfg_act_cb(ACT_SHOW_CPU_MEM_HISTORY, app_act_show_cpu_mem_history);
    cdb_register_cfg_act_cb(ACT_SHOW_LOGIN_SECURITY, app_act_show_login_security);
    cdb_register_cfg_act_cb(ACT_CHANGE_USERNAME, app_change_username_act_process);
    cdb_register_cfg_act_cb(ACT_GET_AUTH, app_user_cmd_get_auth_process);
    cdb_register_cfg_act_cb(ACT_LINE_ACL_CHECK, app_act_line_acl_check);
        
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, NULL, appcfg_process_sys_global);
    cdb_pm_subscribe_tbl(TBL_CHASSIS, NULL, appcfg_process_chassis);

    /* store data into rmon tbl & sub clear counters */
    cdb_pm_subscribe_tbl(TBL_PORT_STATS, NULL, appcfg_process_port_stats);
    app_rmon_start();

    app_ssh_init();
    app_syslog_init();
    app_auth_init();
    app_snmp_init();
    app_ntp_init();
    //app_cem_init();
    app_clock_init();
    ctc_task_add_timer(TASK_PRI_NORMAL, app_login_polling_timer_fn, NULL, CPU_LOGIN_POLLING_INTERVAL);
    
    return PM_E_NONE;
}

int32
appcfg_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_APPCFG);
    lib_master->task_mst = ctc_task_init();
    lib_master->proto_mst = NULL;
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    appcfg_sock_init();
    appcfg_module_init();
    ctc_task_main();
   
    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "d:h:v";

    rsa_init_crypto();
/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
 { "daemon",      no_argument,       NULL, 'd'},
 { "help",        no_argument,       NULL, 'h'},
 { "version",     no_argument,       NULL, 'v'},
 { 0 }
};
#endif /* HAVE_GETOPT_H */

  
 
    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {   
         int32 opt;
   
#ifdef HAVE_GETOPT_H
         opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
         opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
   
         if (EOF == opt)
         {   
             break;
         }
     
         switch (opt)
         {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
             usage(FALSE, progname);
             break;
         default:
             usage(TRUE, progname);
             break;
         }
    }
 
    appcfg_init(daemon_mode);
    exit(0);
}

