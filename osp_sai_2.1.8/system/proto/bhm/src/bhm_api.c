
#include "proto.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "gen/tbl_bhm_module_define.h"
#include "gen/tbl_bhm_module.h"
#include "bhm.h"
#ifndef _GLB_UML_SYSTEM_
#include "ctc_watchdog.h"
#endif

#define __BHM_WATCHDOG__

#define PM_ID_NAME_FILE  "/tmp/.pmid"

static int32
_bhm_check_ext_process_running(pm_id_ext_t pm_id)
{
    char cmd[CMD_BUF_64];
    tbl_bhm_module_key_t key;
    tbl_bhm_module_t* p_db_module = NULL;
    const char* pm_name = NULL;
    int32 rc = PM_E_NONE;
    
    key.pm_id = pm_id;
    p_db_module = tbl_bhm_module_get_bhm_module(&key);

    if (NULL == p_db_module)
    {
        return PM_E_NONE;
    }

    sal_memset(cmd, 0, sizeof(cmd));
    pm_name = proto_module_ext_name_desc(pm_id);
    sal_snprintf(cmd, CMD_BUF_64, "pgrep %s 1>/dev/null 2>&1", pm_name);
    rc = sal_system(cmd);
    if (0 == rc)
    {
        p_db_module->death_count = 0;
    }
    
    return PM_E_NONE;
}

static int32
_bhm_add_ext_process_running(pm_id_ext_t pm_id)
{
    char cmd[CMD_BUF_64];
    char line[CMD_BUF_64];
    tbl_bhm_module_t module;
    FILE *fp = NULL;
    int32 rc = 0;
    uint32 pid = 0;
    const char* pm_name = NULL;

    sal_memset(&module, 0, sizeof(module));
    sal_memset(cmd, 0, sizeof(cmd));

    module.key.pm_id = pm_id;
    module.running = FALSE;
    
    pm_name = proto_module_ext_name_desc(pm_id);
    sal_snprintf(cmd, CMD_BUF_64, "pgrep %s > %s", pm_name, PM_ID_NAME_FILE);
    rc = sal_system(cmd);
    if (0 == rc)
    {
        fp = sal_fopen(PM_ID_NAME_FILE, SAL_OPEN_RO);
        if (fp)
        {
            sal_fgets(line, CMD_BUF_64, fp);
            sal_sscanf(line, "%d", &pid);
            sal_fclose(fp);
            fp = NULL;
            module.running = TRUE;
            module.pid = pid;
        }
    }
    sal_unlink(PM_ID_NAME_FILE);
    
    if (module.running)
    {
        tbl_bhm_module_add_bhm_module(&module);
    }
    
    return PM_E_NONE;
}

static void
_bhm_check_monitor_process()
{
    _bhm_check_ext_process_running(PM_ID_EXT_ZEBRA);
    _bhm_check_ext_process_running(PM_ID_EXT_OSPFD);
    _bhm_check_ext_process_running(PM_ID_EXT_BGPD);
    _bhm_check_ext_process_running(PM_ID_EXT_PIMD);
    _bhm_check_ext_process_running(PM_ID_EXT_OVSDB);

    return;
}

void
_bhm_add_monitor_process()
{
    _bhm_add_ext_process_running(PM_ID_EXT_ZEBRA);
    _bhm_add_ext_process_running(PM_ID_EXT_OSPFD);
    _bhm_add_ext_process_running(PM_ID_EXT_BGPD);
    _bhm_add_ext_process_running(PM_ID_EXT_PIMD);
    _bhm_add_ext_process_running(PM_ID_EXT_OVSDB);

    return;
}

int32
bhm_to_switch_shutdown()
{
    lib_master_t* lib_master = lib_get_master();
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = PM_E_NONE;

    if (lib_master->bhm_sock < 0)
    {
        return PM_E_NOT_INITED;
    }

    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_BHM, 0);
    p_req_msg->u_hdr.bhm.src = lib_master->pm_id;
    p_req_msg->u_hdr.bhm.pid = BHM_SHUTDOWN_CODE;

    rc = ctc_sock_send(lib_master->bhm_sock, PM_ID_SWITCH, p_req_msg);

    return rc;
}

int32
bhm_api_sw_watchdog_reactive(tbl_bhm_module_t *p_db_module)
{
    char out_str[CMD_BUF_128];
    tbl_bhm_global_t *p_bhm_glb = tbl_bhm_global_get_bhm_global();
    const char *pm_name = NULL;

    if (p_db_module->key.pm_id < PM_ID_MAX)
    {
        pm_name = proto_module_name_desc(p_db_module->key.pm_id);
    }
    else
    {
        pm_name = proto_module_ext_name_desc(p_db_module->key.pm_id);
    }

    out_str[0] = CMD_ZERO_CHAR;
    switch (p_bhm_glb->reactive)
    {
    case GLB_BHM_RELOAD:
        if (p_db_module->running && !p_bhm_glb->is_reload)
        {
            logid_sys(LOG_BHM_1_REBOOT, pm_name, p_db_module->pid);
            /* added by tongzb for bug 40719 0x03 means BHMDOG reboot, 2016-09-12 */
            system("reboot_record 0x03 1>/dev/null 2>&1 && sync");
            lib_reboot_system_trap();
            lib_reboot_system();
            p_bhm_glb->is_reload = TRUE;
            sal_snprintf(out_str, CMD_BUF_128, "%% Detect process %s (PID %u) is uncontrolled. System will be reset.\n", pm_name, p_db_module->pid);
        }
        else
        {
            logid_sys(LOG_BHM_1_PROMPT, pm_name, p_db_module->pid);
            sal_snprintf(out_str, CMD_BUF_128, "%% Process %s has been uncontrolled. Please manual reboot system.\n", pm_name);
        }
        break;

    case GLB_BHM_SHUTDOWN:
        if (p_db_module->running && !p_bhm_glb->is_shutdown)
        {
            logid_sys(LOG_BHM_1_SHUTDOWN, pm_name, p_db_module->pid);
            bhm_to_switch_shutdown();
            p_bhm_glb->is_shutdown = TRUE;
            sal_snprintf(out_str, CMD_BUF_128, "%% Detect process %s (PID %u) is uncontrolled. All ports will be shutdown. Please manual reboot system.\n", pm_name, p_db_module->pid);
        }
        else
        {
            logid_sys(LOG_BHM_1_PROMPT, pm_name, p_db_module->pid);
            sal_snprintf(out_str, CMD_BUF_128, "%% Process %s has been uncontrolled. Please manual reboot system.\n", pm_name);
        }
        break;

    case GLB_BHM_WARNING:
        if (p_db_module->running && !p_bhm_glb->is_warning)
        {
            logid_sys(LOG_BHM_1_WARNING, pm_name, p_db_module->pid);
            p_bhm_glb->is_warning = TRUE;
            sal_snprintf(out_str, CMD_BUF_128, "%% Detect process %s (PID %u) is uncontrolled. Please manual reboot system.\n", pm_name, p_db_module->pid);
        }
        else
        {
            logid_sys(LOG_BHM_1_PROMPT, pm_name, p_db_module->pid);
            sal_snprintf(out_str, CMD_BUF_128, "%% Process %s has been uncontrolled. Please manual reboot system.\n", pm_name);
        }
        break;
    }
    
    lib_print_to_all_tty(out_str);
    
    p_db_module->running = FALSE;

    return PM_E_NONE;
}

#define BHM_PS_COUNT    20

static int32
_bhm_api_sw_watchdog_timer_process()
{
    tbl_bhm_module_master_t *p_master = tbl_bhm_module_get_master();
    tbl_bhm_module_t *p_db_module = NULL;
    uint32 pm_id = 0;
    static uint32 ps_count = 0;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (pm_id = PM_ID_INVALID+1; pm_id < PM_ID_MAX; pm_id++)
    {
        p_db_module = p_master->pm_array[pm_id];
        if (NULL == p_db_module)
        {
            continue;
        }
        p_db_module->death_count++;
#ifdef OFPRODUCT
        /* hybrid not monitor those processes */
        if ((PM_ID_DHCSNOOPING == pm_id) || (PM_ID_PTP == pm_id))
        {
            continue;
        }
#endif
        if (0 == (p_db_module->death_count % BHM_DEATH_THRESHOLD))
        {
            bhm_api_sw_watchdog_reactive(p_db_module);
        }
    }

    ps_count++;
    if (ps_count >= BHM_PS_COUNT)
    {
        ps_count = 0;
        _bhm_check_monitor_process();
    }
    
    for (pm_id = PM_ID_EXT_BASE+1; pm_id < PM_ID_EXT_MAX; pm_id++)
    {
        p_db_module = p_master->pm_array[pm_id];
        if (NULL == p_db_module)
        {
            continue;
        }
        p_db_module->death_count++;
        if (0 == (p_db_module->death_count % (BHM_DEATH_THRESHOLD)))
        {
            bhm_api_sw_watchdog_reactive(p_db_module);
        }
    }
    
    return PM_E_NONE;
}

static void
_bhm_api_sw_watchdog_timer_func(void *p_data)
{
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();

    p_db_bhm->sw_watchdog_timer = NULL;
    
    _bhm_api_sw_watchdog_timer_process();
    
    p_db_bhm->sw_watchdog_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _bhm_api_sw_watchdog_timer_func, NULL, BHM_KEEPALIVE_INTERVAL);

    return;
}

#define __BHM_API__

int32
bhm_api_hw_feed_disable()
{
    int32 rc = PM_E_NONE;
#ifndef _GLB_UML_SYSTEM_
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();

    p_db_bhm->hw_watchdog_feed_en = FALSE;
    tbl_bhm_global_set_bhm_global_field(p_db_bhm, TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN);
    
    if (p_db_bhm->hw_watchdog_fd < 0)
    {
        return PM_E_NOT_INITED;
    }
    
    rc = ioctl(p_db_bhm->hw_watchdog_fd, CTC_DIS_FEED_WATCHDOG, 0);
    if (rc < 0)
    {
        return PM_E_IOCTL_FAIL;
    }

#endif
    return rc;
}

static int32
_bhm_api_hw_enable(tbl_bhm_global_t *p_db_bhm)
{   
    int32 rc = PM_E_NONE;
#ifndef _GLB_UML_SYSTEM_

    if (p_db_bhm->hw_watchdog_fd < 0)
    {
        return PM_E_NOT_INITED;
    }
    
    rc = ioctl(p_db_bhm->hw_watchdog_fd, CTC_EN_WATCHDOG, 0);
    if (rc < 0)
    {
        return PM_E_IOCTL_FAIL;
    }

#endif
    return rc;
}


static int32
_bhm_api_hw_disable(tbl_bhm_global_t *p_db_bhm)
{   
    int32 rc = PM_E_NONE;
#ifndef _GLB_UML_SYSTEM_

    if (p_db_bhm->hw_watchdog_fd < 0)
    {
        return PM_E_NOT_INITED;
    }
    
    rc = ioctl(p_db_bhm->hw_watchdog_fd, CTC_DIS_WATCHDOG, 0);
    if (rc < 0)
    {
        return PM_E_IOCTL_FAIL;
    }

#endif
    return rc;
}

int32
bhm_api_set_hw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable)
{
    p_db_bhm->hw_watchdog_enable = enable;
    tbl_bhm_global_set_bhm_global_field(p_db_bhm, TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE);

    if (enable)
    {
        _bhm_api_hw_enable(p_db_bhm);
    }
    else
    {
        _bhm_api_hw_disable(p_db_bhm);
    }
    
    return PM_E_NONE;
}

int32
bhm_api_set_sw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable)
{
    CTC_TASK_STOP_TIMER(p_db_bhm->sw_watchdog_timer);

    p_db_bhm->sw_watchdog_enable = enable;
    tbl_bhm_global_set_bhm_global_field(p_db_bhm, TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE);

    if (enable)
    {
        p_db_bhm->sw_watchdog_timer = ctc_task_add_timer(TASK_PRI_NORMAL, _bhm_api_sw_watchdog_timer_func, NULL, BHM_KEEPALIVE_INTERVAL);
    }
    else
    {
        /* nothing */
    }

    return PM_E_NONE;
}

int32
bhm_api_set_sw_reactive(tbl_bhm_global_t *p_db_bhm, uint32 reactive)
{

    p_db_bhm->reactive = reactive;
    tbl_bhm_global_set_bhm_global_field(p_db_bhm, TBL_BHM_GLOBAL_FLD_REACTIVE);

    return PM_E_NONE;
}

#define __BHM_MESSAGE__

static int32
_bhm_process_bhm_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_bhm_module_t bhm_module;
    tbl_bhm_module_t *p_db_module = NULL;

    sal_memset(&bhm_module, 0, sizeof(bhm_module));

    bhm_module.key.pm_id = p_msg->u_hdr.bhm.src;
    if (bhm_module.key.pm_id >= PM_ID_MAX)
    {
        return PM_E_FAIL;
    }

    p_db_module = tbl_bhm_module_get_bhm_module(&bhm_module.key);
    if (p_db_module)
    {
        p_db_module->pid = p_msg->u_hdr.bhm.pid;
        p_db_module->death_count = 0;
    }
    else
    {
        bhm_module.running = TRUE;
        bhm_module.pid = p_msg->u_hdr.bhm.pid;
        bhm_module.death_count = 0;
        tbl_bhm_module_add_bhm_module(&bhm_module);
    }

    return PM_E_NONE;
}

int32
bhm_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    if (MSG_OPER_BHM == p_msg->hdr.operation)
    {
        _bhm_process_bhm_msg(sock, sess_fd, p_msg);
    }
    else
    {
        return ipc_pm_process_msg(sock, sess_fd, p_msg);
    }
    return PM_E_NONE;
}

#define __BHM_INIT__

int32
bhm_api_set_hw_init(tbl_bhm_global_t *p_db_bhm)
{
#ifndef _GLB_UML_SYSTEM_
    int32 wd_fd = -1;

    p_db_bhm->hw_watchdog_feed_en = TRUE;
    tbl_bhm_global_set_bhm_global_field(p_db_bhm, TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN);

    p_db_bhm->hw_watchdog_fd = -1;
    wd_fd = sal_open(CTC_WD_DEV_NAME, O_RDWR|O_SYNC);
    if (wd_fd < 0)
    {
        log_sys(M_MOD_BHM, E_ERROR, "opening /dev/ctc_watchdog char device failed.");
        return PM_E_NONE;
    }
    p_db_bhm->hw_watchdog_fd = wd_fd;
    /* for bug 53319, use hardware watchdog */
    bhm_api_set_hw_enable(p_db_bhm, TRUE);
#endif
    return PM_E_NONE;
}

static int32
_bhm_set_default()
{
#ifndef TAPPRODUCT
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();

    bhm_api_set_sw_reactive(p_db_bhm, GLB_BHM_RELOAD);
    bhm_api_set_sw_enable(p_db_bhm, TRUE);
    bhm_api_set_hw_init(p_db_bhm);
    /* wait 30s for all PMs start */
    ctc_task_add_timer(TASK_PRI_NORMAL, _bhm_add_monitor_process, NULL, 30);
#endif /* !TAPPRODUCT */
    return PM_E_NONE;
}

int32
bhm_module_init()
{
    cdb_register_cfg_tbl_cb(TBL_BHM_GLOBAL, bhm_cmd_process_bhm_global);
    cdb_register_cfg_tbl_cb(TBL_BHM_MODULE, bhm_cmd_process_bhm_module);
    cdb_register_cfg_act_cb(ACT_REBOOT,     bhm_cmd_process_reboot);
    
    _bhm_set_default();
    ctc_sock_register_rx_fn(bhm_process_msg);

    return PM_E_NONE;
}

int32
bhm_sock_deinit()
{
    lib_master_t *master = lib_get_master();
    
    ctc_sock_close(master->bhm_sock);
    master->bhm_sock = INVALID_SOCK_ID;

    return PM_E_NONE;
}

