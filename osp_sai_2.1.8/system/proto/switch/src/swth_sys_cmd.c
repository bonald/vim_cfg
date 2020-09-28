
#include "proto.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_log_global_define.h"
#include "gen/tbl_log_global.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "switch.h"
#include "swth_sys.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "brg_fdb.h"
#include "lib_fs.h"
#include "glb_tempfile_define.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#ifdef TAPPRODUCT
#include "tap.h"
#endif
#include "cfg_cmd.h"

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 16:15 */
#include "ncutil.h"
#endif /* !HAVE_NETCONF */

extern int32 
acl_process_clock_change(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32
swth_sys_check_telnet_ssh_ace(tbl_ace_config_t *p_ace_config);

extern int32
swth_sys_check_telnet_ssh_acl(tbl_acl_config_t *p_acl_config);

extern int32
swth_sys_set_telnet_ssh_acl(tbl_acl_config_t *p_acl_config,
                            uint32 service_type, uint32 is_add);

extern int32
swth_sys_update_ssh_port(uint32 listen_port);


uint32 route_mac_inited = FALSE;

static int32
_swth_cmd_get_sys_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_sys_global_t* p_sys_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_sys_glb = tbl_sys_global_get_sys_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_sys_global_dump_one(p_sys_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
swth_cmd_set_telnet_port(uint32 port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(lib_cmd_check_port(port, LIB_SERVICE_PORT_TELNET, para));
    
    return swth_sys_set_telnet_port(port);
}

int32
swth_cmd_set_telnet_ssh_acl(cfg_cmd_para_t *para, tbl_acl_config_t *p_acl_config, 
                            uint32 service_type, uint32 is_add)
{
    int32 rc = PM_E_NONE;
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();

    if (NULL == p_sys_glb)
    {
        return PM_E_NONE;
    }

    if (is_add)
    {
        /* check duplicated */
        if (GLB_SERVICE_TYPE_TELNET == service_type 
            && 0 == sal_strcmp(p_sys_glb->telnet_acl, p_acl_config->key.name))
        {
            return rc;
        }
        if (GLB_SERVICE_TYPE_SSH == service_type 
            && 0 == sal_strcmp(p_sys_glb->ssh_acl, p_acl_config->key.name))
        {
            return rc;
        }
        /* ace valid */
        if (0 != swth_sys_check_telnet_ssh_acl(p_acl_config))
        {
            CFG_CONTINUE_RET("Failed to attach ACL: ACL should be ip, ACE should specify protocol TCP and source IP with sequential mask, destination IP is optional");
        }
    }
    else
    {
        /* check duplicated */
        if (GLB_SERVICE_TYPE_TELNET == service_type && 0 == sal_strlen(p_sys_glb->telnet_acl))
        {
            return rc;
        }
        if (GLB_SERVICE_TYPE_SSH == service_type && 0 == sal_strlen(p_sys_glb->ssh_acl))
        {
            return rc;
        }
    }

    swth_sys_set_telnet_ssh_acl(p_acl_config, service_type, is_add);

    return rc;
}

int32
swth_cmd_set_service_acl(char* acl_name, cfg_cmd_para_t *para, uint32 service_type)
{
    int32 rc = PM_E_NONE;
    uint32 is_add = TRUE;
    tbl_acl_config_t    *p_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();

    if (NULL == p_sys_glb)
    {
        return PM_E_NONE;
    }

    if (0 == sal_strlen(acl_name))
    {
        is_add = FALSE;
    }
    else
    {
        /* acl name valid */
        if (check_name_character_and_len(acl_name, ACL_NAME_SIZE) != 0)
        {
            CFG_CONFLICT_RET("No access-list: %s", acl_name);
        }
        /* acl config exist */
        sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));
        sal_strcpy(acl_config_key.name, acl_name);
        p_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
        if (NULL == p_acl_config)
        {
            CFG_CONFLICT_RET("No access-list: %s", acl_name);
        }
    }

    if (GLB_SERVICE_TYPE_TELNET == service_type || GLB_SERVICE_TYPE_SSH == service_type)
    {
        swth_cmd_set_telnet_ssh_acl(para, p_acl_config, service_type, is_add);
    }

    return rc;

}

int _is_startup_done()
{
    tbl_sys_global_t* pGlobal = tbl_sys_global_get_sys_global();
    if (pGlobal && pGlobal->startup_done) {
        return 1;
    }
    return 0;
}

int32
swth_cmd_set_http_port(uint32 port, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;

    PM_E_RETURN(lib_cmd_check_port(port, LIB_SERVICE_PORT_HTTP, para));

    if (_is_startup_done())
    {
        if (!p_db_glb->http_disable)
        {
            CFG_CONFLICT_RET("Please disable HTTP service at first");
        }
    }
    
    return swth_sys_set_http_port(port);
}
int32
swth_cmd_set_http_timeout(uint32 interval, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (_is_startup_done())
    {
        if (!p_db_glb->http_disable)
        {
            CFG_CONFLICT_RET("Please disable HTTP service at first");
        }
    }
    return swth_sys_set_http_timeout(interval);
}
int32
swth_cmd_set_https_port(uint32 port, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(lib_cmd_check_port(port, LIB_SERVICE_PORT_HTTPS, para));
    
    if (!p_db_glb->https_disable)
    {
        CFG_CONFLICT_RET("Please disable HTTPS service at first");
    }
    
    return swth_sys_set_https_port(port);
}

int32
swth_cmd_set_hostname(char *hostname, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. check */
    rc = cdb_check_name(hostname, HOSTNAME_SIZE, para->p_rs->desc);
    if (rc < 0)
    {
        para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
        return rc;
    }
    if (CDB_CFG_NC_CHECK == para->cfg)
    {
        return PM_E_NONE;
    }

    /* 2. config */
    rc = swth_sys_set_hostname(hostname);

#ifdef HAVE_NETCONF
    /* 3. commit to NETCONF */
    if (CDB_CFG_CLI == para->cfg)
    {
        if (PM_E_NONE == rc)
        {
            if (ctc_nc_set_item_str(NULL, "/ietf-system:system/hostname", hostname, SR_EDIT_DEFAULT) == SR_ERR_OK) 
            {
                ctc_nc_commit_no_verify(NULL);
            }
        }
    }
#endif /* !HAVE_NETCONF */

    return rc;
}

int32
_swth_cmd_log_ntp_time_update()
{
    char strTime[CMD_BUF_256];
    sal_time_t date;

    strTime[0] = CMD_ZERO_CHAR;
    date = sal_time(NULL);
    sal_time_strf(strTime, CMD_BUF_256, "%T %Z %a %b %d %Y", sal_localtime(&date));
    
    logid_sys(LOG_SYSTEM_4_TIMEUPDATE, strTime);

    return PM_E_NONE;
}

int32
swth_cmd_process_sys_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_sys = tbl_sys_global_get_sys_global();
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    mac_addr_t route_mac;
    uint32 mac_num = 0;
    int32 rc = PM_E_NONE;
    uint32 aaa_privilege1 = 0;
    uint32 aaa_privilege2 = 0;
    uint32 aaa_privilege3 = 0;
    uint32 mem_threshold1 = 0;
    uint32 mem_threshold2 = 0;
    uint32 mem_threshold3 = 0;
    

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SYS_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == sal_strcmp("hostname", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = swth_cmd_set_hostname(argv[1], para);
        }
        else if (0 == sal_strcmp("route_mac", argv[0]))
        {
            if (route_mac_inited)
            {
                CFG_INVALID_PARAM_RET("route MAC has been inited");
                return PM_E_NONE;
            }
            route_mac_inited = TRUE;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
            CFG_PARSER_MAC(route_mac, argv[1], para->p_rs);
            PM_CFG_GET_UINT(mac_num, argv[3], para->p_rs);
            rc = swth_sys_set_route_mac(route_mac, mac_num);
            swth_sys_pdu_init();
            lacp_init_default_config(route_mac);
        }
        else if (0 == sal_strcmp("init_done", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(mac_num, argv[1], para->p_rs);
            if (0 == mac_num)
            {
                CFG_INVALID_PARAM_RET("init_done should only set to 1");
            }
            rc = swth_sys_set_init_done();
        }
        else if (0 == sal_strcmp("startup_done", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(mac_num, argv[1], para->p_rs);
            if (0 == mac_num)
            {
                CFG_INVALID_PARAM_RET("startup_done should only set to 1");
            }
            rc = swth_sys_set_startup_done();
        }
        else if (0 == sal_strcmp("ipg_global", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            rc = swth_sys_set_ipg_global(argv[1], argv[2]);
        }        
        else if (0 == sal_strcasecmp("aaa_new_model", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid aaa_new_model value %s", argv[1]);
            }
            rc = swth_sys_set_aaa_new_model(atoi(argv[1]));

            /*added by yejl to fix bug41404, 2016-12-08.*/
            /*The aaa privilege mapping should keep unchanged after disable and enable aaa new-model*/
            if (atoi(argv[1]))
            {
                if (p_db_sys->aaa_privilege3 == 0 && p_db_sys->aaa_privilege2 == 0 && p_db_sys->aaa_privilege1 == 0)
                {
                    swth_sys_set_aaa_privilege(CISCO_PRIVILEGE_MIN, CISCO_PRIVILEGE_TWO, CISCO_PRIVILEGE_THREE);
                }
            }
        } 
        else if (0 == sal_strcasecmp("aaa_privilege1", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
            
            if (!p_db_sys->aaa_new_model) 
            {
                CFG_INVALID_PARAM_RET("Please enable aaa first!");
            }
            /*added by yejl to fix bug 41140*/
            if ((0 == sal_strcmp("-", argv[1])) || (0 == sal_strcmp("-", argv[3])) || (0 == sal_strcmp("-", argv[5])))
            {
                CFG_INVALID_PARAM_RET("Illegal parameter");
            }
            aaa_privilege1 = atoi(argv[1]);
            aaa_privilege2 = atoi(argv[3]);
            aaa_privilege3 = atoi(argv[5]);

            /*added by yejl to fix bug41143, 2016-12-12*/
            if (aaa_privilege2 <= aaa_privilege1) 
            {
                CFG_INVALID_PARAM_RET("Invalid value: max privilege 2 should be greater than max privilege 1");
            }
            if (aaa_privilege3 <= aaa_privilege2) 
            {
                CFG_INVALID_PARAM_RET("Invalid value: max privilege 3 should be greater than max privilege 2");
            }
            
            rc = swth_sys_set_aaa_privilege(aaa_privilege1, aaa_privilege2, aaa_privilege3);
        } 
        else if (0 == sal_strcasecmp("service_password_encryption", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid service_password_encryption value %s", argv[1]);
            }
            rc = swth_sys_set_service_password_encryption(atoi(argv[1]));
        } 
        else if (0 == sal_strcasecmp("dhcp_service_enable", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid dhcp_service_enable value %s", argv[1]);
            }
            rc = swth_sys_set_dhcp_service(atoi(argv[1]));
        } 
        else if (0 == sal_strcasecmp("dhcp_relay_enable", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid dhcp_relay_enable value %s", argv[1]);
            }
            rc = swth_sys_set_dhcp_relay(atoi(argv[1]), para);
        } 
        else if (0 == sal_strcasecmp("dhcp_snooping_enable", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid dhcp_snooping_enable value %s", argv[1]);
            }
            rc = swth_sys_set_dhcp_snooping(atoi(argv[1]));
        }
        else if (0 == sal_strcasecmp("http_service_enable", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid http_service_enable value %s", argv[1]);
            }
            rc = swth_sys_set_http_service(atoi(argv[1]));
        }
        else if (0 == sal_strcasecmp("http_image_name", argv[0]))
        {
            char szFilename[PATH_MAX];
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            snprintf(szFilename, PATH_MAX, "%s/%s", FULL_NAME_FLASH, argv[1]);
            if (lib_fs_is_file(szFilename) == FALSE) {
                CFG_INVALID_PARAM_RET("Invalid http image file name %s", argv[1]);
            }
            rc = swth_sys_set_http_image_name(argv[1]);
        }
        else if (0 == sal_strcasecmp("tap_ts_macda", argv[0]))
        {
            tbl_tap_group_master_t* tap_group_master = NULL;
 
            mac_addr_t macda;
            mac_addr_t macsa;
            uint32 ether_type = 0;

            if(!check_feature_support(CONFIG_FEATURE_TAP_TIMESTAMP))
            {
                return lib_cmd_process_cli_return(para,PM_E_NOT_SUPPORT);
            }

            CFG_PARSER_MAC(macda, argv[1], para->p_rs);
            CFG_PARSER_MAC(macsa, argv[2], para->p_rs);
            PM_CFG_GET_UINT_RANGE(ether_type,0, 0xFFFF, argv[3], para->p_rs);

            tap_group_master = tbl_tap_group_get_master();
            if (NULL != tap_group_master)
            {
                if (0 != tap_group_master->tap_group_hash->count)
                {
                    CFG_INVALID_PARAM_RET("Tap timestamp parameter should changed before configure any TAP group");
                }
            }

            swth_sys_set_tap_timestamp_ether(macda, macsa, ether_type);
        }
        else if (0 == sal_strcasecmp("trunction_length", argv[0]))
        {
            uint32 length = 0;

            if(!check_feature_support(CONFIG_FEATURE_TAP_TRUNCTION))
            {
                return lib_cmd_process_cli_return(para,PM_E_NOT_SUPPORT);
            }

            PM_CFG_GET_UINT(length, argv[1], para->p_rs);
            swth_sys_set_truncation_length(length);
        }
        else if (0 == sal_strcasecmp("ptf_port", argv[0]))
        {
            uint32 ptf_port = 0;
            PM_CFG_GET_UINT(ptf_port, argv[1], para->p_rs);
            if (p_db_sys->ptf_port)
            {
                if (ptf_port)
                {
                    CFG_CONFLICT_RET("PTF has been enabled on port %u", p_db_sys->ptf_port);
                }
            }
            swth_sys_set_ptf_port(ptf_port);
        }
        else if (0 == sal_strcasecmp("max_vty", argv[0]))
        {
            uint32 vtys = 0;
            /*added by yejl to fix bug 41140*/
            if (0 == sal_strcmp("-", argv[1]))
            {
                CFG_INVALID_PARAM_RET("Illegal parameter");
            }
            PM_CFG_GET_UINT(vtys, argv[1], para->p_rs);
            if (vtys < 0 || vtys > GLB_MAX_VTY_NUM)
            {
                CFG_INVALID_PARAM_RET("Invalid maximum vty number");
            }
            swth_sys_set_max_vty(vtys, para);
        }
        else if (0 == sal_strcasecmp("telnet_disable", argv[0]))
        {
            uint32 disable = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(disable, argv[1], para->p_rs);
            swth_sys_set_telnet_disable(disable, para);
        }
        else if (0 == sal_strcasecmp("telnet_port", argv[0]))
        {
            uint32 port = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(port, argv[1], para->p_rs);
            swth_cmd_set_telnet_port(port, para);
        }
        else if (0 == sal_strcasecmp("telnet_acl", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            swth_cmd_set_service_acl(argv[1], para, GLB_SERVICE_TYPE_TELNET);
        }
        else if (0 == sal_strcasecmp("ssh_acl", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            swth_cmd_set_service_acl(argv[1], para, GLB_SERVICE_TYPE_SSH);
        }
        else if (0 == sal_strcasecmp("http_disable", argv[0]))
        {
            uint32 disable = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(disable, argv[1], para->p_rs);
            rc = swth_sys_set_http_disable(disable, para);
            return rc;
        }
        else if (0 == sal_strcasecmp("http_port", argv[0]))
        {
            uint32 port = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(port, argv[1], para->p_rs);
            swth_cmd_set_http_port(port, para);
        }
        else if (0 == sal_strcasecmp("http_timeout", argv[0]))
        {
            uint32 interval = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(interval, argv[1], para->p_rs);
            swth_cmd_set_http_timeout(interval, para);
        }
        else if (0 == sal_strcasecmp("https_disable", argv[0]))
        {
            uint32 disable = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(disable, argv[1], para->p_rs);
            rc = swth_sys_set_https_disable(disable, para);
            return rc;
        }
        else if (0 == sal_strcasecmp("https_port", argv[0]))
        {
            uint32 port = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(port, argv[1], para->p_rs);
            swth_cmd_set_https_port(port, para);
        }
        else if (0 == sal_strcasecmp("rpcapi_disable", argv[0]))
        {
            uint32 disable = FALSE;
            uint32 ssl_en = FALSE;
            uint32 port = 0;
            uint32 ssl_port = 0;
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 4, 6, para->p_rs);
            PM_CFG_GET_BOOL(disable, argv[1], para->p_rs);
            PM_CFG_GET_UINT(port, argv[3], para->p_rs);
            if (6 == argc)
            {
                ssl_en = TRUE;
                PM_CFG_GET_UINT(ssl_port, argv[5], para->p_rs);
            }
            rc = swth_sys_set_rpcapi_disable(disable, ssl_en, port, ssl_port, para);
            return rc;
        }
        else if (0 == sal_strcasecmp("rpcapi_auth_mode", argv[0]))
        {
            uint32 auth_mode = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(auth_mode, argv[1], para->p_rs);
            rc = swth_sys_set_rpcapi_auth_mode(auth_mode, para);
            return rc;
        }
        else if (0 == sal_strcasecmp("cut_through_enable", argv[0]))
        {
            uint32 cut_through_enable = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
            {
                CFG_UNSUPPORT_PRODUCT_RET();
            }

            PM_CFG_GET_UINT(cut_through_enable, argv[1], para->p_rs);
            rc = swth_sys_set_cut_through_enable(cut_through_enable);
            if (PM_E_EXIST == rc)
            {
                CFG_PROMPT_RET("Notice: Configuration about cutting through forwarding mode has been stored, but cannot take effect until the next reload");
            }
        }
        else if (0 == sal_strcasecmp("memory_threshold1", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);
            mem_threshold1 = atoi(argv[1]);
            mem_threshold2 = atoi(argv[3]);
            mem_threshold3 = atoi(argv[5]);
            
            if (mem_threshold1 > mem_threshold2)
            {
                CFG_INVALID_PARAM_RET("threshold1 should be less than threshold2");
            }
            if (mem_threshold2 > mem_threshold3)
            {
                CFG_INVALID_PARAM_RET("threshold2 should be less than threshold3");
            }
            rc = swth_sys_set_memory_threshold(mem_threshold1, mem_threshold2, mem_threshold3);
        }
        else if (0 == sal_strcasecmp("mib_port_stats_read_num", argv[0]))
        {
            uint32 read_num = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(read_num, argv[1], para->p_rs);
            p_db_sys->mib_port_stats_read_num = read_num;
            tbl_sys_global_set_sys_global_field(p_db_sys, TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM);
        }
        else if (0 == sal_strcasecmp("mib_flow_stats_read_num", argv[0]))
        {
            uint32 read_num = 0;
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(read_num, argv[1], para->p_rs);
            p_db_sys->mib_flow_stats_read_num = read_num;
            tbl_sys_global_set_sys_global_field(p_db_sys, TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM);
        }
		else if (0 == sal_strcasecmp("system_time_update", argv[0]))
        {
            acl_process_clock_change(NULL, 0, NULL);
            _swth_cmd_log_ntp_time_update();
        }
        else if (0 == sal_strcasecmp("dhcp_server_enable", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (strcasecmp(argv[1], "0") && strcasecmp(argv[1], "1")) {
                CFG_INVALID_PARAM_RET("Invalid dhcp_server_enable value %s", argv[1]);
            }
            rc = swth_sys_set_dhcp_server(atoi(argv[1]));
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter for sys_global %s", argv[0]);
        }
        break;

    case CDB_OPER_GET:
        if ((1 == argc) && (!sal_strcmp(argv[0], "routemac") 
                            || !sal_strcmp(argv[0], "systemmac")
                            || !sal_strcmp(argv[0], "services")
                            || !sal_strcmp(argv[0], "services_rpcapi")
                            || !sal_strcmp(argv[0], "sflow")
                            || !sal_strcmp(argv[0], "aaa_new_model")
                            || !sal_strcmp(argv[0], "http")
                            || !sal_strcmp(argv[0], "aaa_privilege1")))
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        else
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        rc = _swth_cmd_get_sys_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;

}

static int32
_swth_cmd_get_log_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_log_global_t* p_glb = tbl_log_global_get_log_global();
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_log_global_dump_one(p_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_swth_sys_cmd_set_log_to_cdb(uint32 log_id, uint32 enable, cfg_cmd_para_t *para)
{
    tbl_log_global_t* p_db_log_glb = tbl_log_global_get_log_global();
    uint32 db_enable = FALSE;

    /* 1. config check */
    db_enable = GLB_BMP_ISSET(p_db_log_glb->log_to_cdb, log_id) ? TRUE : FALSE;
    if (db_enable == enable)
    {
        return PM_E_NONE;
    }
    
    if (enable)
    {
        GLB_BMP_SET(p_db_log_glb->log_to_cdb, log_id);
    }
    else
    {
        GLB_BMP_UNSET(p_db_log_glb->log_to_cdb, log_id);
    }
    tbl_log_global_set_log_global_field(p_db_log_glb, TBL_LOG_GLOBAL_FLD_LOG_TO_CDB);
    
    return PM_E_NONE;
}

int32
swth_cmd_process_log_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    logid_bmp_t bmp;
    field_parse_t field;
    uint32 log_id = 0;
    uint32 value = 0;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_LOG_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        if (0 == sal_strcmp(argv[0], "log_to_cdb"))
        {
            GLB_BMP_INIT(bmp);
            rc = cdb_bitmap_str2val(argv[1], 0, LOG_ID_MAX, bmp, GLB_PORT_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
            PM_CFG_GET_BOOL(value, argv[2], para->p_rs);
            GLB_BMP_ITER_BEGIN(bmp, log_id)
            {
                _swth_sys_cmd_set_log_to_cdb(log_id, value, para);
            }
            GLB_BMP_ITER_END(bmp, log_id);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid field ID %s", argv[0]);
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _swth_cmd_get_log_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
swth_cmd_show_resource(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc != 1)
        {
            CFG_INVALID_PARAM_RET("Invalid argc count %u", argc);
        }
        
        if (!sal_strcmp(argv[0], "fdb"))
        {
        }
        else if (!sal_strcmp(argv[0], "tap"))
        {
#if 0/* by xuwj */        
            tap_cmd_show_resource_tap(para);
#endif
        }
        else 
        {
            CFG_INVALID_PARAM_RET("Invalid argv %s", argv[0]);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32 
swth_show_memory_threshold(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    uint32 sys_total = 0;
    uint32 sys_kernel_reserve = 0;
    
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    swth_sys_get_reserve_and_total_memory(&sys_kernel_reserve, &sys_total);
    sal_fprintf(fp, "System reserve : %d M Bytes\n", sys_kernel_reserve);
    sal_fprintf(fp, "Threshold1     : %d M Bytes\n", p_db_glb->memory_threshold1);
    sal_fprintf(fp, "Threshold2     : %d M Bytes\n", p_db_glb->memory_threshold2);
    sal_fprintf(fp, "Threshold3     : %d M Bytes\n", p_db_glb->memory_threshold3);
    sal_fprintf(fp, "The current system memory state is %s.\n", swth_sys_get_memchkstate_val2str(p_db_glb->system_memchk_state));
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
swth_cmd_show_memory_threshold(char **argv, int32 argc, cfg_cmd_para_t *para)
{   
    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            swth_show_memory_threshold(para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

static int32 
swth_show_monitor_latency(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;   
    tbl_interface_t *p_db_if = NULL;
    tbl_qos_global_t *p_db_glb = NULL;
    ctclib_slistnode_t *listnode = NULL;
    bool interface_config = FALSE;
    char ifname_ext[GLB_IFNAME_SZ];
    bool latency_event_en = FALSE;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
 
    p_db_glb = tbl_qos_global_get_qos_global();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF)
       {
            if (interface_config == FALSE)
           {
               sal_fprintf(fp, "Monitor latency active range(ns):  \n");
               sal_fprintf(fp, "-----------------------+------------------------\n");
               sal_fprintf(fp, "Range1: %4d - %4d     Range2: %4d - %4d\n",  p_db_glb->latency_range[0],  p_db_glb->latency_range[1],  
                                                                                                         p_db_glb->latency_range[1],  p_db_glb->latency_range[2]);

               sal_fprintf(fp, "Range3: %4d - %4d     Range4: %4d - %4d\n",  p_db_glb->latency_range[2],  p_db_glb->latency_range[3],
                                                                                                         p_db_glb->latency_range[3],  p_db_glb->latency_range[4]);

               sal_fprintf(fp, "Range5: %4d - %4d     Range6: %4d - %4d\n",  p_db_glb->latency_range[4],  p_db_glb->latency_range[5],  
                                                                                                         p_db_glb->latency_range[5],  p_db_glb->latency_range[6]);
                                                                                                                                          
               sal_fprintf(fp, "Range7: %4d - %4d     Range8: %4d - more\n",  p_db_glb->latency_range[6],  p_db_glb->latency_range[7],
                                                                                                         p_db_glb->latency_range[7]);
                
               sal_fprintf(fp, "\n");
               sal_fprintf(fp, "%-12s%-12s%-12s%-12s\n", "Port", "Event", "Thrd_high", "Thrd_low");
               sal_fprintf(fp, "-----------+-----------+-----------+------------\n");
               interface_config = TRUE;
           }
        
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);     
             if (p_db_if->latency_event_enable)
             {
                    latency_event_en = TRUE;
             }
             else
             {
                   latency_event_en = FALSE;
             }
             sal_fprintf(fp, "%-12s%-12s%-12u%-12u\n", ifname_ext, (latency_event_en? "enable":"disable"), p_db_if->latency_event_high, p_db_if->latency_event_low);
        }
    }

    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

static int32 
swth_show_monitor_buffer(cfg_cmd_para_t *para)
{   
    FILE *fp = NULL;   
    tbl_qos_global_t *p_db_glb = NULL;
    bool latency_buffer_en = FALSE;

    p_db_glb = tbl_qos_global_get_qos_global();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_glb->buffer_event_enable)
    {
        latency_buffer_en = TRUE;
    }
    else
    {
        latency_buffer_en = FALSE;
    }
    
    sal_fprintf(fp, "Monitor buffer event : %s\n", (latency_buffer_en?"enable":"disable"));
    sal_fprintf(fp, "Monitor buffer event threshold high (buffer cell count) : %u\n", p_db_glb->buffer_event_high);
    sal_fprintf(fp, "Monitor buffer event threshold low  (buffer cell count) : %u\n", p_db_glb->buffer_event_low);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
swth_cmd_show_monitor(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    { 
    case CDB_OPER_GET:
        if (0 == sal_strcmp(argv[0], "latency"))
        {
            swth_show_monitor_latency(para);
        }
        else if (0 == sal_strcmp(argv[0], "buffer"))
        {
            swth_show_monitor_buffer(para);
        }
        
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
    
    return PM_E_NONE;
}


int32
swth_cmd_ssh_cfg_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, 
                         uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_ssh_cfg_t* p_ssh_global = NULL;

    if(p_ds_node)
    {
        return PM_E_NONE;
    }

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_ssh_global = (tbl_ssh_cfg_t*)p_tbl;
        if (TBL_SSH_CFG_FLD_LISTEN_PORT == field_id)
        {
           swth_sys_update_ssh_port(p_ssh_global->listen_port);
        }

        break;

    default:
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

int32
swth_cmd_clear_rpcapi_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *counter_fp = NULL;
    counter_fp = sal_fopen(RPCAPI_COUNTER_FILE, "w+");
    if (counter_fp)
    {
        sal_fprintf(counter_fp, "0 0");
        sal_fclose(counter_fp);
    }

    return PM_E_NONE;
}


