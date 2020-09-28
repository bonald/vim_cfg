
#include "proto.h"
#include "lib_tblinc.h"

static int32
_cfg_memory_show(cfg_show_t *p_resp_show, cfg_cmd_para_t *para)
{
    lib_master_t *lib_master = lib_get_master();
    FILE *fp = NULL;

    sal_memset(p_resp_show, 0, sizeof(cfg_show_t));
    p_resp_show->banner = TRUE;
    CFG_BUILD_SHOW_PATH(p_resp_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(p_resp_show->path, "w+");

    sal_fprintf(fp, "%s Memory Information:\n", proto_module_desc(lib_master->pm_id));
    memory_dump(fp);
    sal_fprintf(fp, "\n");
    
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_cfg_socket_show(cfg_show_t *p_resp_show, cfg_cmd_para_t *para, uint32 detail)
{
    lib_master_t *lib_master = lib_get_master();
    FILE *fp = NULL;

    sal_memset(p_resp_show, 0, sizeof(cfg_show_t));
    p_resp_show->banner = TRUE;
    CFG_BUILD_SHOW_PATH(p_resp_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(p_resp_show->path, "w+");
    
    /* write file */
    sal_fprintf(fp, "%s Socket Information:\n", proto_module_desc(lib_master->pm_id));
    ctc_sock_dump(fp, detail);
    sal_fprintf(fp, "\n");
    
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
lib_cmd_process_memory(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = _cfg_memory_show(para->p_show, para);
        break;
        
    default:
        rc = PM_E_INVALID_PARAM;
        break;
    }
    
    return rc;
}

int32
lib_cmd_process_socket(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 detail = FALSE;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 3)
        {
            detail = TRUE;
        }
        rc = _cfg_socket_show(para->p_show, para, detail);
        break;
        
    default:
        rc = PM_E_INVALID_PARAM;
        break;
    }
    
    return rc;
}

int32
lib_cmd_process_show_com(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (argc < 1)
    {
        CFG_CONFLICT_RET("Invalid parameter count %u for show_com", argc);
    }

    if (0 == sal_strcmp(argv[1], "memory"))
    {
        lib_cmd_process_memory(argv, argc, para);
    }
    else if (0 == sal_strcmp(argv[1], "socket"))
    {
        lib_cmd_process_socket(argv, argc, para);
    }
    else
    {
        CFG_CONFLICT_RET("Invalid parameter %s for show_com", argv[0]);
    }

    return rc;
}

int32
lib_cmd_process_set_socket(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    ctc_sock_t *p_sock = NULL;
    int32 sock = 0;
    uint32 is_rx = FALSE;
    uint32 size = 0;
    int32 rc = PM_E_NONE;

    if (argc != 4)
    {
        CFG_CONFLICT_RET("Invalid parameter count %u", argc);
    }

    PM_CFG_GET_UINT(sock, argv[1], para->p_rs);
    if (0 == sal_strcmp(argv[2], "rx"))
    {
        is_rx = TRUE;
    }
    PM_CFG_GET_UINT(size, argv[3], para->p_rs);

    p_sock = ctc_sock_get_sock(sock);
    if (NULL == p_sock)
    {
        CFG_CONFLICT_RET("Socket %d not exist", sock);
    }
    rc = ctc_sock_set_buf(sock, is_rx, size);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Set Sock %d buffer size fail", sock);
    }

    return rc;
}

int32
lib_cmd_dump_cdb_all(tbl_iter_args_t *p_args)
{
    uint32 tbl_id = 0;
    cdb_node_t* p_node = NULL;
    FILE *fp = p_args->argv[1];
    
    for (tbl_id = 0; tbl_id < TBL_MAX; tbl_id++)
    {
        p_node = cdb_get_tbl(tbl_id);
        if (p_node && p_node->inited)
        {
            sal_fprintf(fp, "##### TBL %s(%u) #####\n", p_node->name, p_node->id);
            p_node->tbl_info->ops.iter_fn(p_node->tbl_info->ops.dump_fn, p_args);
        }
    }

    return PM_E_NONE;
}

int32
lib_cmd_process_dump_cdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    void *p_master = NULL;
    FILE *fp = NULL;
    tbl_iter_args_t args;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    
    if (argc < 1)
    {
        CFG_CONFLICT_RET("Invalid parameter count %u for dump_cdb", argc);
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field));
    args.argv[0] = &field;
    args.argv[1] = fp;
 
    if (0 == sal_strcmp(argv[1], "interface"))
    {
        p_master = tbl_interface_get_master();
        if (p_master)
        {
            tbl_interface_iterate((TBL_ITER_CB_FUNC)tbl_interface_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "vlan"))
    {
        p_master = tbl_vlan_get_master();
        if (p_master)
        {
            tbl_vlan_iterate((TBL_ITER_CB_FUNC)tbl_vlan_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "fdb"))
    {
        p_master = tbl_fdb_get_master();
        if (p_master)
        {
            tbl_fdb_iterate((TBL_ITER_CB_FUNC)tbl_fdb_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "mcfdb"))
    {
        p_master = tbl_mcfdb_get_master();
        if (p_master)
        {
            tbl_mcfdb_iterate((TBL_ITER_CB_FUNC)tbl_mcfdb_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "macfilter"))
    {
        p_master = tbl_macfilter_get_master();
        if (p_master)
        {
            tbl_macfilter_iterate((TBL_ITER_CB_FUNC)tbl_macfilter_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "brg_global"))
    {
        p_master = tbl_brg_global_get_brg_global();
        if (p_master)
        {
            tbl_brg_global_iterate((TBL_ITER_CB_FUNC)tbl_brg_global_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "lag_global"))
    {
        p_master = tbl_lag_global_get_lag_global();
        if (p_master)
        {
            tbl_lag_global_iterate((TBL_ITER_CB_FUNC)tbl_lag_global_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "route_if"))
    {
        p_master = tbl_route_if_get_master();
        if (p_master)
        {
            tbl_route_if_iterate((TBL_ITER_CB_FUNC)tbl_route_if_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "time_range"))
    {
        p_master = tbl_time_range_get_master();
        if (p_master)
        {
        tbl_time_range_iterate((TBL_ITER_CB_FUNC)tbl_time_range_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "sys_global"))
    {
        p_master = tbl_sys_global_get_sys_global();
        if (p_master)
        {
            tbl_sys_global_iterate((TBL_ITER_CB_FUNC)tbl_sys_global_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "sys_spec"))
    {
        p_master = tbl_sys_spec_get_master();
        if (p_master)
        {
            tbl_sys_spec_iterate((TBL_ITER_CB_FUNC)tbl_sys_spec_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "card"))
    {
        p_master = tbl_sys_spec_get_master();
        if (p_master)
        {
            tbl_card_iterate((TBL_ITER_CB_FUNC)tbl_card_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "version"))
    {
        p_master = tbl_version_get_version();
        if (p_master)
        {
            tbl_version_iterate((TBL_ITER_CB_FUNC)tbl_version_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "manage_if"))
    {
        p_master = tbl_manage_if_get_manage_if();
        if (p_master)
        {
            tbl_manage_if_iterate((TBL_ITER_CB_FUNC)tbl_manage_if_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "bootimage"))
    {
        p_master = tbl_bootimage_get_bootimage();
        if (p_master)
        {
            tbl_bootimage_iterate((TBL_ITER_CB_FUNC)tbl_bootimage_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "fan"))
    {
        p_master = tbl_fan_get_master();
        if (p_master)
        {
            tbl_fan_iterate((TBL_ITER_CB_FUNC)tbl_fan_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "psu"))
    {
        p_master = tbl_psu_get_master();
        if (p_master)
        {
            tbl_psu_iterate((TBL_ITER_CB_FUNC)tbl_psu_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "sensor"))
    {
        p_master = tbl_sensor_get_master();
        if (p_master)
        {
            tbl_sensor_iterate((TBL_ITER_CB_FUNC)tbl_sensor_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "switch_debug"))
    {
        p_master = tbl_switch_debug_get_switch_debug();
        if (p_master)
        {
            tbl_switch_debug_iterate((TBL_ITER_CB_FUNC)tbl_switch_debug_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "chassis"))
    {
        p_master = tbl_chassis_get_chassis();
        if (p_master)
        {
            tbl_chassis_iterate((TBL_ITER_CB_FUNC)tbl_chassis_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "hsrv_debug"))
    {
        p_master = tbl_hsrv_debug_get_hsrv_debug();
        if (p_master)
        {
            tbl_hsrv_debug_iterate((TBL_ITER_CB_FUNC)tbl_hsrv_debug_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "lsrv_debug"))
    {
        p_master = tbl_lsrv_debug_get_lsrv_debug();
        if (p_master)
        {
            tbl_lsrv_debug_iterate((TBL_ITER_CB_FUNC)tbl_lsrv_debug_dump_one, &args);
        }
    }
    else if (0 == sal_strcmp(argv[1], "all"))
    {
        lib_cmd_dump_cdb_all(&args);
    }
    else
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid parameter %s for dump_cdb", argv[0]);
    }
    
    sal_fclose(fp);

    return rc;
}

int32
lib_cmd_process_show_log(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
    field_parse_t field;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field));
    args.argv[0] = &field;
    args.argv[1] = fp;

    tbl_log_iterate((TBL_ITER_CB_FUNC)tbl_log_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32 
lib_cmd_process_cli_return(cfg_cmd_para_t *para, int32 rc)
{
    switch(rc)
    {
        case PM_E_INVALID_PARAM:
            CFG_INVALID_PARAM_RET("Invalid parameters");
            break;
        case PM_E_NOT_EXIST:
            CFG_INVALID_PARAM_RET("Entry not exist");
            break;
        case PM_E_EXIST:
            CFG_INVALID_PARAM_RET("Entry already exist");
            break;
        case PM_E_NOT_INITED:
            CFG_INVALID_PARAM_RET("This Module not init");
            break;
        case PM_E_INIT_FAIL:
            CFG_INVALID_PARAM_RET("This Module init fail");
            break;   
        case PM_E_NOT_SUPPORT:
            CFG_INVALID_PARAM_RET("This function not support");
            break;             
        case PM_E_EXCEED_MAX_NUM:
            CFG_INVALID_PARAM_RET("Entry number is more than max number");
            break;  
        case PM_E_NO_MEMORY:
            CFG_INVALID_PARAM_RET("System has no enough memory");
            break;  
        case PM_E_NOT_PERMIT:
            CFG_INVALID_PARAM_RET("This action is not permit");
            break;    
    }

    return PM_E_NONE;

}

void
_lib_cmd_set_timezone(tbl_clock_t *p_clk)
{
    char strZone[CMD_BUF_256];
    int32 rc = PM_E_NONE;
    
    sal_memset(strZone, 0, sizeof(strZone));

    /* If timezone name length is 0, means the timezone have been cleared. */
    if (0 != strlen(p_clk->timezone_name))
    {
        if (p_clk->timezone_positive) 
        {
            sal_sprintf(strZone, "%s-%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
        else
        {
            /*negative time zone from UTC*/
            sal_sprintf(strZone, "%s+%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
    }
    else
    {
        sal_sprintf(strZone, "%s", "UTC");         
    }  

    rc = setenv("TZ", strZone, 1);    
    if (rc < 0)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Put TZ env failed! in %s.%s", __FILE__, __LINE__);
    }
    
    tzset();
}

static int32
_lib_cmd_process_clock_timezone(cdb_node_t* p_tbl_node, 
                        cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    int32 rc = PM_E_NONE;
    tbl_clock_t *p_clk;
    
    p_clk = (tbl_clock_t*)p_tbl;
    
    CFG_DBG_FUNC();

    switch (oper)
    {
    case CDB_OPER_SET:
        /* When timezone name notified, we can assure that the timezone offset have been synced. */
        if (p_clk && (TBL_CLOCK_FLD_TIMEZONE_NAME == field_id))
        {
            _lib_cmd_set_timezone(p_clk);
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
lib_cmd_check_port(uint32 port, uint32 type, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t *p_db_ssh_cfg = tbl_ssh_cfg_get_ssh_cfg();

    if (LIB_SERVICE_PORT_TELNET != type)
    {
        if (p_db_glb->telnet_port == port)
        {
            CFG_CONFLICT_RET("The port %u has been occupied by telnet service", port);
        }
    }
    
    if (LIB_SERVICE_PORT_SSH != type)
    {
        if (p_db_ssh_cfg->listen_port == port)
        {
            if (0 != port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by SSH service", port);
            }
        }
    }
    
    if (LIB_SERVICE_PORT_NETCONF != type)
    {
        if (port && (p_db_ssh_cfg->netconf_port == port))
        {
            CFG_CONFLICT_RET("The port %u has been occupied by NETCONF SSH service", port);
        }
    }

    if (LIB_SERVICE_PORT_HTTP != type)
    {
        if (p_db_glb->http_port == port)
        {
            CFG_CONFLICT_RET("The port %u has been occupied by HTTP service", port);
        }
    }

    if (LIB_SERVICE_PORT_HTTPS != type)
    {
        /* CNOS and hybrid should not allow enable HTTPS and RPCAPI at the same time */
        if (LIB_SERVICE_PORT_RPCAPI_SSL != type)
        {
            if (p_db_glb->https_port == port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by HTTPS service", port);
            }
        }
    }
    
    if (LIB_SERVICE_PORT_RPCAPI != type)
    {
        if (p_db_glb->rpcapi_port == port)
        {
            CFG_CONFLICT_RET("The port %u has been occupied by RPC-API service", port);
        }
    }

    if (LIB_SERVICE_PORT_RPCAPI_SSL != type)
    {
        /* not check port 0 */
        if (port)
        {
            if (LIB_SERVICE_PORT_HTTPS != type && LIB_SERVICE_PORT_RPCAPI != type)
            {
                if (p_db_glb->rpcapi_ssl_port == port)
                {
                    CFG_CONFLICT_RET("The port %u has been occupied by RPC-API SSL service", port);
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int
lib_check_ipv6_addr(addr_t *addr)
{
    if (IN6_IS_ADDR_MULTICAST (&addr->u.prefix6)
        || IN6_IS_ADDR_LOOPBACK (&addr->u.prefix6)
        || IN6_IS_ADDR_UNSPECIFIED (&addr->u.prefix6))
    {
        return -1;
    }

    return 0;
}

int
lib_check_ipv6_prefix(prefix_t *p)
{
    if (p->prefixlen == IPV6_MAX_PREFIXLEN)
    {
        return -1;
    }

    if (0 == p->prefixlen)
    {
        return -1;
    }
    
    if (IN6_IS_ADDR_MULTICAST (&p->u.prefix6)
        || IN6_IS_ADDR_LOOPBACK (&p->u.prefix6)
        || IN6_IS_ADDR_UNSPECIFIED (&p->u.prefix6))
    {
        return -1;
    }

    return 0;
}

int32
lib_cmd_init(pm_id_t pm_id)
{
    cdb_register_cfg_act_cb(ACT_SHOW_COMMON, lib_cmd_process_show_com);
    cdb_register_cfg_act_cb(ACT_SET_SOCKET, lib_cmd_process_set_socket);
    cdb_register_cfg_act_cb(ACT_DUMP_CDB, lib_cmd_process_dump_cdb);
    cdb_register_cfg_tbl_cb(TBL_LOG, lib_cmd_process_show_log);

    /* APPCFG will handle clock change by itself because it will notify other applications
       which does not call lib_init. (i.e: syslog-ng, ntpd. )*/
    if (PM_ID_APPCFG != pm_id)
    {
        cdb_pm_subscribe_tbl(TBL_CLOCK, NULL, _lib_cmd_process_clock_timezone);
    }
    
    return PM_E_NONE;
}

