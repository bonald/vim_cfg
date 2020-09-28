
#include "proto.h"
#include "glb_tempfile_define.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "gen/tbl_bhm_module_define.h"
#include "gen/tbl_bhm_module.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_bootimage_define.h"
#include "gen/tbl_bootimage.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "bhm.h"
#include "lib_image.h"
#include "lib_snmp.h"

static int32
_bhm_cmd_bhm_hw_feed_en(tbl_bhm_global_t *p_db_bhm, uint32 enable, cfg_cmd_para_t *para)
{
    if (p_db_bhm->hw_watchdog_feed_en == FALSE)
    {
        return PM_E_NONE;
    }

    bhm_api_hw_feed_disable();
    
    return PM_E_NONE;
}

static int32
_bhm_cmd_bhm_hw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable, cfg_cmd_para_t *para)
{
    if (p_db_bhm->hw_watchdog_enable == enable)
    {
        return PM_E_NONE;
    }

    bhm_api_set_hw_enable(p_db_bhm, enable);
    
    return PM_E_NONE;
}

static int32
_bhm_cmd_bhm_sw_enable(tbl_bhm_global_t *p_db_bhm, uint32 enable, cfg_cmd_para_t *para)
{
    if (p_db_bhm->sw_watchdog_enable == enable)
    {
        return PM_E_NONE;
    }

    bhm_api_set_sw_enable(p_db_bhm, enable);
    
    return PM_E_NONE;
}

static int32
_bhm_cmd_bhm_sw_reactive(tbl_bhm_global_t *p_db_bhm, uint32 reactive, cfg_cmd_para_t *para)
{
    if (p_db_bhm->reactive == reactive)
    {
        return PM_E_NONE;
    }

    bhm_api_set_sw_reactive(p_db_bhm, reactive);
    
    return PM_E_NONE;
}

static int32
_bhm_cmd_get_bhm_global(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field, uint32 show_type)
{
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();
    FILE *fp = NULL;
    tbl_iter_args_t args;
    const char *bhm_reactive_str[] = {"reload system", "shutdown port", "print warning"};

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (0 == show_type)
    {
        tbl_bhm_global_dump_one(p_db_bhm, &args);
    }
    else if (1 == show_type)
    {
        sal_fprintf(fp, "heart-beat-monitor %s\n", p_db_bhm->sw_watchdog_enable ? "enable" : "disable");
        sal_fprintf(fp, "heart-beat-monitor reactivation: %s\n", cdb_enum_val2str(bhm_reactive_str, GLB_BHM_ACTION_MAX, p_db_bhm->reactive));
    }
    else if (2 == show_type)
    {
        sal_fprintf(fp, "System monitor %s\n", p_db_bhm->hw_watchdog_enable ? "enable" : "disable");
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
bhm_cmd_process_bhm_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 value = 0;
    int32 reactive = 0;
    uint32 show_type = 0;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_BHM_GLOBAL);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_FEED_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            if (FALSE != value)
            {
                CFG_INVALID_PARAM_RET("Only allowed set %s to 0", argv[i]);
            }
            rc = _bhm_cmd_bhm_hw_feed_en(p_db_bhm, value, para);
            break;

        case TBL_BHM_GLOBAL_FLD_HW_WATCHDOG_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            rc = _bhm_cmd_bhm_hw_enable(p_db_bhm, value, para);
            break;

        case TBL_BHM_GLOBAL_FLD_SW_WATCHDOG_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            rc = _bhm_cmd_bhm_sw_enable(p_db_bhm, value, para);
            break;

        case TBL_BHM_GLOBAL_FLD_REACTIVE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            reactive = cdb_enum_str2val(glb_bhm_action_strs, GLB_BHM_ACTION_MAX, argv[i+1]);
            if (reactive < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid reactive %s", argv[i+1]);
            }
            rc = _bhm_cmd_bhm_sw_reactive(p_db_bhm, reactive, para);
            break;
    
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        if (0 == argc)
        {
            show_type = 0;            
        }
        else if ((1 <= argc) && !sal_strcmp(argv[i], "sw"))
        {
            show_type = 1;
        }
        else if ((1 == argc) && !sal_strcmp(argv[i], "hw"))
        {
            show_type = 2;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid input");
        }
        rc = _bhm_cmd_get_bhm_global(para, p_node, &field, show_type);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
bhm_cmd_show_bhm_module(FILE *fp)
{
    tbl_bhm_global_t *p_db_bhm = tbl_bhm_global_get_bhm_global();
    tbl_bhm_module_master_t *p_master = tbl_bhm_module_get_master();
    tbl_bhm_module_t *p_db_bhm_module = NULL;
    uint32 pm_id = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "heart-beat-monitor %s\n", p_db_bhm->sw_watchdog_enable ? "enable" : "disable");
    sal_fprintf(fp, "heart-beat-monitor reactivation: %s\n", cdb_enum_val2str(glb_bhm_action_strs, GLB_BHM_ACTION_MAX, p_db_bhm->reactive));

    sal_fprintf(fp, "is_reload:   %u\n", p_db_bhm->is_reload);
    sal_fprintf(fp, "is_shutdown: %u\n", p_db_bhm->is_shutdown);
    sal_fprintf(fp, "is_warning:  %u\n", p_db_bhm->is_warning);
    
    sal_fprintf(fp, "%-8s %-20s %-8s %-8s %-8s\n", 
        "PM ID", "Description", "Running", "PID", "Death");
    sal_fprintf(fp, "--------+--------------------+--------+--------+--------\n");
    
    for (pm_id = PM_ID_INVALID+1; pm_id < PM_ID_MAX; pm_id++)
    {
        p_db_bhm_module = p_master->pm_array[pm_id];
        if (NULL == p_db_bhm_module)
        {
            sal_fprintf(fp, "%-8u %-20s %-8s %-8s %-8s\n", 
                pm_id, 
                proto_module_name_desc(pm_id), 
                "No", 
                "-",
                "-");
        }
        else
        {
            sal_fprintf(fp, "%-8u %-20s %-8s %-8u %-8u\n", 
                pm_id, 
                proto_module_name_desc(pm_id), 
                p_db_bhm_module->running ? "Yes" : "No", 
                p_db_bhm_module->pid, 
                p_db_bhm_module->death_count);
        }
    }

    for (pm_id = PM_ID_EXT_BASE+1; pm_id < PM_ID_EXT_MAX; pm_id++)
    {
        p_db_bhm_module = p_master->pm_array[pm_id];
        if (NULL == p_db_bhm_module)
        {
            sal_fprintf(fp, "%-8u %-20s %-8s %-8s %-8s\n", 
                pm_id, 
                proto_module_ext_name_desc(pm_id), 
                "No", 
                "-",
                "-");
        }
        else
        {
            sal_fprintf(fp, "%-8u %-20s %-8s %-8u %-8u\n", 
                pm_id, 
                proto_module_ext_name_desc(pm_id), 
                p_db_bhm_module->running ? "Yes" : "No", 
                p_db_bhm_module->pid, 
                p_db_bhm_module->death_count);
        }
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

static int32
_bhm_cmd_get_bhm_module(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field, uint32 show_type)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (0 == show_type)
    {
        tbl_bhm_module_iterate((TBL_ITER_CB_FUNC)tbl_bhm_module_dump_one, &args);
    }
    else if (1 == show_type)
    {
        bhm_cmd_show_bhm_module(fp);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
bhm_cmd_process_bhm_module(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    uint32 show_type = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_BHM_MODULE);
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        if (0 == argc)
        {
            show_type = 0;            
        }
        else if ((1 <= argc) && !sal_strcmp(argv[0], "show"))
        {
            show_type = 1;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid input");
        }
        rc = _bhm_cmd_get_bhm_module(para, p_node, &field, show_type);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
bhm_stop_processes()
{
    tbl_bhm_module_master_t *p_master = tbl_bhm_module_get_master();
    tbl_bhm_module_t *p_db_module = NULL;
    uint32 pm_id = 0;
    int32 rc = PM_E_NONE;
    char cmd_str[CMD_BUF_256];
    char pid_str[CMD_BUF_16];

    sal_system("killall crond 1>/dev/null 2>&1");
    sal_system("killall ntpd 1>/dev/null 2>&1");
    sal_system("killall klish 1>/dev/null 2>&1");
    sal_system("killall zebra 1>/dev/null 2>&1");
    sal_system("killall ospfd 1>/dev/null 2>&1");
    sal_system("killall ctctop 1>/dev/null 2>&1");
    sal_system("killall trap_agent 1>/dev/null 2>&1");
    sal_system("killall python 1>/dev/null 2>&1");
    sal_system("killall sshd 1>/dev/null 2>&1");
    sal_system("killall ovsdb-server 1>/dev/null 2>&1");
    sal_system("killall telnetd 1>/dev/null 2>&1");
    sal_system("killall snmpd 1>/dev/null 2>&1");
    sal_system("killall snmpagent 1>/dev/null 2>&1");
    sal_system("killall ctcmon 1>/dev/null 2>&1");
    
    sal_snprintf(cmd_str, CMD_BUF_256, "kill -9 ");
    
    for (pm_id = 0; pm_id < PM_ID_MAX; pm_id++)
    {
        p_db_module = p_master->pm_array[pm_id];
        if (NULL == p_db_module)
        {
            continue;
        }
        sal_memset(pid_str, 0, sizeof(pid_str));
        if (p_db_module->running)
        {
            sal_snprintf(pid_str, CMD_BUF_16, "%u ", p_db_module->pid);
        }
        sal_strcat(cmd_str, pid_str);
    }
    sal_system(cmd_str);
    return rc;
}

int32
bhm_warmboot_set_warmboot(glb_warmboot_type_t type)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    FILE *fp = NULL;
    sal_time_t curr_time_sec;

    curr_time_sec = ctc_time_boottime_sec(NULL);
    if ((fp = sal_fopen(BHM_WARMBOOT_FILE, "w+")) != NULL)
    {
        sal_fprintf(fp, "type=%u\n", type);
        sal_fprintf(fp, "time=%u\n", (uint32)curr_time_sec);
        sal_fprintf(fp, "count=%u\n", p_sys_glb->warmboot_count+1);
        sal_fclose(fp);
    }
    if (GLB_WARMBOOT_HITLESS == type)
    {
        sal_system("touch "WB_RESTORING_FILE);
    }
    
    return PM_E_NONE;
}

int32
bhm_remove_ns_route()
{
    tbl_ns_route_master_t *p_master = tbl_ns_route_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ns_route_t *p_db_rt = NULL;
    char ip_buf[CMD_BUF_32];
    char cmd[CMD_BUF_128];

    CTCLIB_SLIST_LOOP(p_master->ns_route_list, p_db_rt, listnode)
    {
        sal_inet_ntop(AF_INET, &p_db_rt->key.ip, ip_buf, sizeof(ip_buf));
        sal_sprintf(cmd, "ip route del %s via %s", ip_buf, GLB_NS_ROUTE_MGMT_IP_ADDR);
        sal_system(cmd);
    }

    return PM_E_NONE;
}

void
_bhm_reboot_process(void *p_data)
{
    tbl_bhm_global_t *p_db_glb = tbl_bhm_global_get_bhm_global();
    int32 rc = PM_E_NONE;
    TIMESTAMP_DECLARE();

    log_sys(M_MOD_BHM, E_ERROR, "Start reboot fast");
    sal_system("reboot_record 0x0b 1>/dev/null 2>&1 && sync");

    TIMESTAMP_TS1();
    /* a) kill all protocol processes */
    bhm_stop_processes();

    /* Added by kcao for bug 45433, flush ns_route when fast reboot */
    bhm_remove_ns_route();

    /* remove /tmp/ files */
    sal_system("rm -rf /tmp/* 1>/dev/null 2>&1");
    sal_system("rm -rf /tmp/.* 1>/dev/null 2>&1");

    /* set /tmp/warmboot info */
    bhm_warmboot_set_warmboot(p_db_glb->reboot);
    
    /* b) remove kernel modules */
    sal_system("rmmod ctc_reset 1>/dev/null 2>&1");
    sal_system("rmmod ctc_power 1>/dev/null 2>&1");
    sal_system("rmmod ctc_phy 1>/dev/null 2>&1");
    sal_system("rmmod ctc_gpio 1>/dev/null 2>&1");
    sal_system("rmmod ctc_hw 1>/dev/null 2>&1");
    sal_system("rmmod dal 1>/dev/null 2>&1");
    sal_system("rmmod ctc_watchdog 1>/dev/null 2>&1");

    /* c) disable BHM server socket */
    bhm_sock_deinit();

    /* d) update new ramfs if have */
    lib_image_update_ramfs();

    sal_task_sleep(1000);
    /* e) restart all protocol processes */
#ifdef _GLB_UML_SYSTEM_
    rc = system("/centec_switch/sbin/reboot_fast.sh 1>/dev/null 2>&1");
#else
    rc = system("reboot_fast.sh 1>/dev/null 2>&1");
#endif

    TIMESTAMP_TS2();
    if (rc == 0)
    {
        log_sys(M_MOD_BHM, E_ERROR, "Finish reboot fast, takes %u seconds", (_tv2.tv_sec - _tv1.tv_sec));
    }
    else
    {
        log_sys(M_MOD_BHM, E_ERROR, "Reboot fast failed with return code %d, takes %u seconds", (_tv2.tv_sec - _tv1.tv_sec), rc);
    }

    sal_task_sleep(100);
    exit(0);

    return;
}

int32
bhm_reset_system_do_reboot(uint32 reboot_type, uint32 save)
{
    tbl_bhm_global_t *p_db_glb = tbl_bhm_global_get_bhm_global();
    
    /* 1. send trap */
    snmp_trap_reboot_send(save);

    /* 2. set reboot type, TODO */    
    /* added by tongzb for bug 40719 0x01 means MANUAL reboot, 2016-09-12 */
    system("reboot_record 0x01 1>/dev/null 2>&1 && sync");

    /* 3. do syslog Fix for bug 44823, send trap at first */
    lib_reboot_system_trap();
    
    p_db_glb->reboot = reboot_type;
    tbl_bhm_global_set_bhm_global_field(p_db_glb, TBL_BHM_GLOBAL_FLD_REBOOT);
    /* wait fea finish */
    sleep(2);
        
    if (GLB_WARMBOOT_NORMAL == reboot_type)
    {
        /* reset self */
        lib_reboot_system();
    }
    else if (GLB_WARMBOOT_FAST == reboot_type)
    {
        ctc_task_add_timer(TASK_PRI_HIGH, _bhm_reboot_process, NULL, 1);
    }
    else if (GLB_WARMBOOT_HITLESS == reboot_type)
    {
        sleep(10);
        ctc_task_add_timer(TASK_PRI_HIGH, _bhm_reboot_process, NULL, 1);
    }
    
    return PM_E_NONE;
}

int32
bhm_cmd_check_reboot_fast_condition(cfg_cmd_para_t *para)
{
    tbl_bootimage_t *p_db_boot = tbl_bootimage_get_bootimage();
    char *curr_image = NULL;

    if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
    if (GLB_CHIP_DUET2 == tbl_chip_get_type())
    {
        CFG_UNSUPPORT_PRODUCT_RET();
    }
#ifdef _CTC_X86_64_
    CFG_CONFLICT_RET("Not supported on X64 CPU");
#endif

    curr_image = sal_strstr(p_db_boot->current_image, "flash:/boot/");
    if (curr_image)
    {
        /* if current is flash mode, must uses boot fast to set boot image */
        curr_image += sal_strlen("flash:/boot/");
        if (sal_strlen(curr_image))
        {
            if (0 != sal_strcmp(curr_image, p_db_boot->image))
            {
                if (0 == p_db_boot->fast)
                {
                    CFG_CONFLICT_RET("Current boot image mismatch with next boot image, should uses 'boot fast' to set");
                }
            }
        }
    }
    else 
    {
        curr_image = sal_strstr(p_db_boot->current_image, "TftpFile://");
        if (curr_image)
        {
            /* if current is tftp mode, must should not uses flash mode, for boot mode after reboot fast is confusion */
            if (BOOT_MODE_TFTP != p_db_boot->mode)
            {
                CFG_CONFLICT_RET("The next boot mode is not tftp");
            }    
            curr_image += sal_strlen("TftpFile://");
            curr_image = sal_strstr(curr_image, ":");
            if (curr_image)
            {
                curr_image += sal_strlen(":");
            }
            if (sal_strlen(curr_image))
            {
                if (0 != sal_strcmp(curr_image, p_db_boot->image))
                {
                    /* if current is tftp mode, tftp image name should not changed */
                    CFG_CONFLICT_RET("Current boot image mismatch with next boot image");
                }
            }
        }
        else
        {
            curr_image = sal_strstr(p_db_boot->current_image, "Tftp6File://");
            if (curr_image)
            {
                /* if current is tftp mode, must should not uses flash mode, for boot mode after reboot fast is confusion */
                if (BOOT_MODE_TFTP != p_db_boot->mode)
                {
                    CFG_CONFLICT_RET("The next boot mode is not tftp");
                }    
                curr_image += sal_strlen("Tftp6File://");
                curr_image = sal_strstr(curr_image, "/");
                if (curr_image)
                {
                    curr_image += sal_strlen("/");
                }
                if (sal_strlen(curr_image))
                {
                    if (0 != sal_strcmp(curr_image, p_db_boot->image))
                    {
                        /* if current is tftp mode, tftp image name should not changed */
                        CFG_CONFLICT_RET("Current boot image mismatch with next boot image");
                    }
                }
            }
            else
            {
                CFG_CONFLICT_RET("Get current boot image failed");
            }
        }
    }

    return PM_E_NONE;
}

int32
bhm_cmd_process_reboot(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 reboot_type = 0;
    uint32 check_save = FALSE;
    uint32 save = 1;
    int32 rc = PM_E_NONE;  

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        if (0 == sal_strcmp(argv[0], "normal"))
        {
            check_save = TRUE;
            reboot_type = GLB_WARMBOOT_NORMAL;
        }
        else if (0 == sal_strcmp(argv[0], "fast"))
        {
            check_save = TRUE;
            reboot_type = GLB_WARMBOOT_FAST;
            PM_E_RETURN(bhm_cmd_check_reboot_fast_condition(para));
        }
        else if (0 == sal_strcmp(argv[0], "hitless"))
        {
            check_save = TRUE;
            reboot_type = GLB_WARMBOOT_HITLESS;
        }

        if (check_save)
        {
            if (argc == 2)
            {
                if (0 == sal_strcmp(argv[0], "save"))
                {
                    save = 2;
                }
            }
        }

        bhm_reset_system_do_reboot(reboot_type, save);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

