
#include "proto.h"
#include "genlog.h"
#include "glb_tempfile_define.h"

#ifdef _GLB_UML_SYSTEM_
#define M_SNMPD_PROG        "/centec_switch/sbin/snmpd"
#define M_SNMPD_CFGFILE     "/centec_switch/etc/snmpd.conf"
#else
#define M_SNMPD_PROG        "snmpd"
#define M_SNMPD_CFGFILE     CTC_ETC_BASE_DIR"/etc/snmpd.conf"
#endif

typedef struct
{
    ctc_task_master_t  *task_mst;
    uint32 noaction;
} ctcmon_master_t;

ctcmon_master_t* _g_ctcmon_master = NULL;

#define SNMP_RUNNING_POLLING_WAIT           60
#define SNMP_RUNNING_POLLING_INTERVAL       10
#define SNMP_RUNNING_POLLING_THRESHOLD      3

int32
ctcmon_file_exist(char *filename)
{
    struct stat stat_buf;
    
    if (!stat(filename, &stat_buf))
    {
        return 1;
    }
    return 0;
}

int32
_ctcmon_snmp_running_monitor()
{
    int32 rc = 0;
    uint32 agent_norun = 0;
    uint32 snmpd_norun = 0;
    static uint32 norun_count = 0;
    char szCmd[512];

    if (ctcmon_file_exist(GLB_SNMP_EN_FILE))
    {
#ifdef CONFIG_SNMPAGT
        rc = sal_system("ps -fe | grep snmpagent | grep -v grep 2>&1 > /dev/null");
#else
        rc = sal_system("ps -fe --cols 120 | grep snmp_agent_main.py | grep -v grep 2>&1 > /dev/null");
#endif
        agent_norun = (0 != rc) ? TRUE : FALSE;

        rc = sal_system("ps -fe | grep snmpd | grep -v grep 2>&1 > /dev/null");
        snmpd_norun = (0 != rc) ? TRUE : FALSE;

        if (agent_norun || snmpd_norun)
        {
            norun_count++;
        }
        else
        {
            norun_count = 0;
        }
        
        if (norun_count >= SNMP_RUNNING_POLLING_THRESHOLD)
        {
            if (snmpd_norun)
            {
#ifdef CONFIG_SNMPAGT
                /* need to kill old snmpagent */
                sal_system("killall snmpagent 2>&1 > /dev/null");
#endif
                log_sys(M_MOD_APP, E_ERROR, "Detect snmpd not running, do restart it");
                CTCLIB_PRINT_CON("Detect snmpd not running, do restart it");
                sal_snprintf(szCmd, 512, "%s -c %s", M_SNMPD_PROG, M_SNMPD_CFGFILE);
                sal_system(szCmd);
                sal_task_sleep(1000);
                log_sys(M_MOD_APP, E_ERROR, "Finish restart snmpd");
                CTCLIB_PRINT_CON("Finish restart snmpd");
#ifdef CONFIG_SNMPAGT
                /* restart snmpagent */
                sal_system("snmpagent &");
#endif
            }
            if (agent_norun)
            {
                log_sys(M_MOD_APP, E_ERROR, "Detect agent not running, do restart it");
                CTCLIB_PRINT_CON("Detect agent not running, do restart it");
#ifdef CONFIG_SNMPAGT
                sal_snprintf(szCmd, 512, "snmpagent &");
#else
                sal_snprintf(szCmd, 512, "snmp_agent_main.py -d");
#endif
                sal_system(szCmd);
                sal_task_sleep(1000);
                log_sys(M_MOD_APP, E_ERROR, "Finish restart snmp agent");
                CTCLIB_PRINT_CON("Finish restart snmp agent");
            }
            norun_count = 0;
        }
    }
    else
    {
        norun_count = 0;
    }

    return PM_E_NONE;
}

void
ctcmon_snmp_polling_timer_fn(void *p_data)
{
    _ctcmon_snmp_running_monitor();

    ctc_task_add_timer(TASK_PRI_NORMAL, ctcmon_snmp_polling_timer_fn, NULL, SNMP_RUNNING_POLLING_INTERVAL);
    
    return;
}

int32
_ctcmon_web_running_monitor()
{
    int32 rc = 0;
    uint32 web_norun = 0;
    static uint32 norun_count = 0;
    char szCmd[512];

    if (ctcmon_file_exist(GLB_WEBUI_EN_FILE))
    {
        rc = sal_system("ps -fe --cols 120 | grep index.py | grep -v grep 2>&1 > /dev/null");
        web_norun = (0 != rc) ? TRUE : FALSE;
        
        if (web_norun)
        {
            norun_count++;
        }
        else
        {
            norun_count = 0;
        }
        
        if (norun_count >= SNMP_RUNNING_POLLING_THRESHOLD)
        {
#ifdef HAVE_DEBIAN
            snprintf(szCmd, 512, "python /var/web/run_server.py start");
#else
            snprintf(szCmd, 512, "export PYTHONHOME=/usr; python /var/web/run_server.py start");
#endif
            sal_system(szCmd);
            sal_task_sleep(1000);
            log_sys(M_MOD_APP, E_ERROR, "HTTP routine monitor");
            norun_count = 0;
        }
    }
    else
    {
        norun_count = 0;
    }

    return PM_E_NONE;
}

void
ctcmon_web_polling_timer_fn(void *p_data)
{
    _ctcmon_web_running_monitor();

    ctc_task_add_timer(TASK_PRI_NORMAL, ctcmon_web_polling_timer_fn, NULL, SNMP_RUNNING_POLLING_INTERVAL);
    
    return;
}

int32
ctcmon_init(uint32 noaction)
{
    memory_init(0);
    
    _g_ctcmon_master = (ctcmon_master_t*)XCALLOC(MEM_PM_LIB_MASTER, sizeof(ctcmon_master_t));
    _g_ctcmon_master->task_mst = ctc_task_init();
    _g_ctcmon_master->noaction = noaction;
    
    ctc_task_add_timer(TASK_PRI_NORMAL, ctcmon_snmp_polling_timer_fn, NULL, SNMP_RUNNING_POLLING_WAIT);
    /*modified by yangl for bug 54043 2019-10-21*/
    /*because the http service is default disabled on hybrid product*/
#ifndef OFPRODUCT
    ctc_task_add_timer(TASK_PRI_NORMAL, ctcmon_web_polling_timer_fn, NULL, SNMP_RUNNING_POLLING_WAIT);
#endif
    /*yangl end*/
    ctc_task_main();

    return PM_E_NONE;
}

