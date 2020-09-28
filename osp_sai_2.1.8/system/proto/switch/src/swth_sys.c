
#include <sys/dir.h>
#include "proto.h"
#include "glb_if_define.h"
#include "glb_ip_define.h"
#include "glb_cpu_traffic_define.h"
#include "glb_tempfile_define.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "brg.h"
#include "sflow.h"
#include "if_mgr.h"
#include "switch_api.h"
#include "qos/qos_mgr.h"
#include "param_check.h"
#include "rsa_derhex.h"
#include "if_mgr.h"
#include "cfg_cmd.h"
#include "glb_message_define.h"
#include "lib_netlink.h"
#include "lib_syslimit.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "swth_sys.h"
#include "ns_port_forwarding.h"
#include "lib_snmp.h"
#include "l2_protocol.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "iptables.h"

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 16:15 */
#include <sched.h>
#include "ncutil.h"
#endif /* !HAVE_NETCONF */

//static mac_addr_t swth_bpdu = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
//static mac_addr_t swth_bpdu_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0};

#define HTTP_CONF_PATH      "/var/web/http.conf"
#define M_RPCAPI_CFGFILE    "/var/rpcapi/http.conf"
#define M_STUNNEL_CFGFILE   CTC_ETC_BASE_DIR"/etc/stunnel/stunnel.conf"
#define  SYS_MEM_DEF_VALUE    1024
#define HTTP_START_TIMEER_MS 5


#define MEMORY_CHECK_INTERVAL_TIME 5
#define MEMORY_CHECK_LOG_PRINT_TIME 180

ctc_task_t *g_p_memcheck_timer = NULL;
ctc_task_t *g_p_memcheck_packet_discard_log_timer = NULL;

ctc_task_t *g_p_http_start_http = NULL;
ctc_task_t *g_p_http_stop_http = NULL;
ctc_task_t *g_p_http_restart_http = NULL;

int32 swth_sys_telnet_startup_done();
extern int32 qos_mgr_init(void);
extern int32 cpu_traffic_init(void);
extern int32 copp_init(void);
extern int32 errdisable_start(switch_master_t *master);
extern int32 _swth_sys_set_rpcapi_stunnel_conf(uint32 ssl_en, uint32 port, uint32 ssl_port);
extern int32 _swth_sys_set_http_port_ns(uint32 old_port, uint32 new_port);


int32
swth_sys_check_telnet_ssh_ace(tbl_ace_config_t *p_ace_config)
{
    int32 rc = PM_E_NONE;

    /* TCP Protocol */
    if (!GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IP_PROTOCOL))
    {
        return PM_E_CFG_CONFLICT;
    }
    if (GLB_ACL_L4_PROTOCOL_TCP != p_ace_config->ip_protocol || 0 != p_ace_config->ip_protocol_mask)
    {
        return PM_E_CFG_CONFLICT;
    }
    /* Source IP */
    if (!GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPSA))
    {
        return PM_E_CFG_CONFLICT;
    }
    /* other field */
    if ((((GLB_IP_ACE_IP_PROTOCOL | GLB_IP_ACE_IPSA) != p_ace_config->key_flags)
        && ((GLB_IP_ACE_IP_PROTOCOL | GLB_IP_ACE_IPSA | GLB_IP_ACE_IPDA) != p_ace_config->key_flags))
        || 0 != p_ace_config->key_flags2)
    {
        return PM_E_CFG_CONFLICT;
    }

    return rc;
}

int32
swth_sys_check_telnet_ssh_acl(tbl_acl_config_t *p_acl_config)
{
    int32 rc = PM_E_NONE;
    tbl_ace_config_master_t *p_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ace_config_t   *p_ace_config = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }
    /* IP Access-list */
    if(GLB_ACL_CONFIG_TYPE_NORMAL != p_acl_config->type_identifying)
    {
        return PM_E_CFG_CONFLICT;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_ace_config, listnode, next)
    {
        if (NULL == p_ace_config)
        {
            continue;
        }
        if (0 != sal_strcmp(p_ace_config->key.acl_name, p_acl_config->key.name))
        {
            continue;
        }
        PM_E_RETURN(swth_sys_check_telnet_ssh_ace(p_ace_config));
    }

    return rc;
}

int32
swth_sys_set_telnet_ssh_acl(tbl_acl_config_t *p_acl_config,
                            uint32 service_type, uint32 is_add)
{
    int32 rc = PM_E_NONE;

    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    if (NULL == p_sys_glb)
    {
        return PM_E_NONE;
    }

    if (is_add)
    {
        if ((GLB_SERVICE_TYPE_TELNET == service_type && sal_strlen(p_sys_glb->telnet_acl))
          ||(GLB_SERVICE_TYPE_SSH == service_type && sal_strlen(p_sys_glb->ssh_acl)))
        {
            ipt_del_service_acl(service_type);
        }

        ipt_add_service_acl(p_acl_config, service_type);

        if (GLB_SERVICE_TYPE_TELNET == service_type)
        {
            sal_strcpy(p_sys_glb->telnet_acl, p_acl_config->key.name);
        }
        else if (GLB_SERVICE_TYPE_SSH == service_type)
        {
            sal_strcpy(p_sys_glb->ssh_acl, p_acl_config->key.name);
        }
    }
    else
    {
        ipt_del_service_acl(service_type);

        if (GLB_SERVICE_TYPE_TELNET == service_type)
        {
            sal_memset(p_sys_glb->telnet_acl, 0, sizeof(p_sys_glb->telnet_acl));
        }
        else if (GLB_SERVICE_TYPE_SSH == service_type)
        {
            sal_memset(p_sys_glb->ssh_acl, 0, sizeof(p_sys_glb->ssh_acl));
        }
    }

    PM_E_RETURN(tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_TELNET_ACL));
    PM_E_RETURN(tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_SSH_ACL));

    return rc;
}

int32
swth_sys_update_ssh_port(uint32 listen_port)
{
    int32 rc = PM_E_NONE;

    ipt_update_service_acl_port(GLB_SERVICE_TYPE_SSH, listen_port);
    
    return rc;
}

/* TODO : hsrv's cpu traffic data is leech on to l2/l3 interface info, so PM need consider order */
int32
swth_sys_pdu_set_port_action(uint32 ifindex, uint32 mode, uint32 set_mode)
{
    return PM_E_NONE;
}

int32
swth_sys_pdu_init()
{
    return PM_E_NONE;
}


int32
swth_sys_check_system_mac(const uint8 *mac)
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    uint32 low16 = 0;
    uint32 low16_min = 0;
    uint32 low16_max = 0;
    
    if (0 == sal_memcmp(mac, p_sys_glb->route_mac, 4))
    {
        low16 = (mac[4] << 8) + mac[5];
        low16_min = (p_sys_glb->route_mac[4] << 8) + p_sys_glb->route_mac[5];
        low16_max = low16_min + p_sys_glb->mac_num - 1;
        if (low16 >= low16_min && low16 <= low16_max)
        {
            return PM_E_INVALID_PARAM;
        }
    }

    return PM_E_NONE;
}

int32
swth_sys_set_route_mac(uint8 *route_mac, uint32 mac_num)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sal_memcpy(sys_glb.route_mac, route_mac, MAC_ADDR_LEN);
    sys_glb.mac_num = mac_num;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_ROUTE_MAC);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MAC_NUM);
    swth_mstp_api_update_system_mac(route_mac);
    return PM_E_NONE;
}

int32
swth_sys_set_hostname(char *hostname)
{
    char sys_cmd[256];
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sal_strncpy(sys_glb.hostname, hostname, HOSTNAME_SIZE);

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HOSTNAME);

    sal_snprintf(sys_cmd, 256, "hostname %s", hostname);
    system(sys_cmd);

    signal_app("/var/run/syslog-ng.pid", SIGUSR1); /* let syslog-ng reload hostname */
    return PM_E_NONE;
}

int32
swth_sys_set_ipg_global(char *type, char* value)
{
    tbl_sys_global_t sys_glb;
    int32 enable = 0;    
    tbl_sys_global_field_id_t fld_id = TBL_SYS_GLOBAL_FLD_MAX;
    tbl_sys_global_t *p_db_sys_glb = tbl_sys_global_get_sys_global();
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    

    enable = sal_atoi(value);
    if (0 == sal_strcmp(type, "policer"))
    {
        sys_glb.ipg_policing_enable = enable;
        if (p_db_sys_glb->ipg_policing_enable != sys_glb.ipg_policing_enable)
        {
            fld_id = TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE; 
        }
    }
    else if (0 == sal_strcmp(type, "shaping"))
    {
        sys_glb.ipg_shaping_enable = enable;
        if (p_db_sys_glb->ipg_shaping_enable != sys_glb.ipg_shaping_enable)
        {
            fld_id = TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE;     
        }        
    }
    else if (0 == sal_strcmp(type, "storm-control"))
    {
        sys_glb.ipg_storm_control_enable = enable;
        if (p_db_sys_glb->ipg_storm_control_enable != sys_glb.ipg_storm_control_enable)
        { 
            tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE);
        }
    }
    else
    {
        return PM_E_INVALID_PARAM;
    }

    if (TBL_SYS_GLOBAL_FLD_MAX != fld_id)
    {
        tbl_sys_global_set_sys_global_field(&sys_glb, fld_id);
        
    }
    return PM_E_NONE;
}

uint32
swth_sys_is_startup_done()
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    return p_sys_glb->startup_done;
}

extern int32
swth_sys_set_tap_timestamp_ether(mac_addr_t macda, mac_addr_t macsa, uint32 ether_type);

int32
swth_fea_init()
{
    int32 rc = PM_E_NONE;

#ifdef CONFIG_L2
    PM_E_RETURN(brg_fea_init());
#endif /* CONFIG_L2 */

#ifdef TAPPRODUCT
    /* 1. init TAP config */
   // fei_tap_init_config();

    mac_addr_t macda;
    mac_addr_t macsa;
    uint32 ether_type = 0;

    sal_memset(macda, 0, sizeof(mac_addr_t));
    sal_memset(macsa, 0, sizeof(mac_addr_t));
    swth_sys_set_tap_timestamp_ether(macda, macsa, ether_type);
#endif

    return PM_E_NONE;
}

int32
swth_hybrid_agg_init()
{
    char aggname[IFNAME_SIZE] = {0};
    tbl_openflow_interface_t openflow_if;
    uint32 agg_index = 0;
    uint32 index = 0;

    for(agg_index = 1; agg_index < 64; agg_index ++ )
    {
        sal_sprintf(aggname, "agg%d", agg_index);
        sal_memset(&openflow_if, 0x0, sizeof(openflow_if));
        sal_strncpy(openflow_if.key.name, aggname, IFNAME_SIZE);
        openflow_if.obey_vlan_filter = 1;
        openflow_if.bind_tunnel_type = GLB_IF_TYPE_VXLAN_IF;
        openflow_if.ifindex = GLB_AGG_IFINDEX_MIN + agg_index;
        for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
        {
            openflow_if.protected_vlan[index ] = 0;
        }
        tbl_openflow_interface_add_openflow_interface(&openflow_if);
    }

    return PM_E_NONE;
}
int32
swth_sys_set_init_done()
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_sys_load_t* p_sys_load = tbl_sys_load_get_sys_load();
    uint32 fei_error_count = 0;
    tbl_sflow_global_t* p_sflow_glb = tbl_sflow_global_get_sflow_global();
    
    if (p_sys_glb->init_done)
    {
        return PM_E_NONE;
    }

    if_mgr_add_cpu_if();
    swth_module_init();
    swth_fea_init();
    swth_hybrid_agg_init();
    qos_mgr_init();
    cpu_traffic_init();
    copp_init();
    qos_monitor_init();
    l2_protocol_init();
#ifdef TSINGMA
    hash_field_profile_init();
    hash_value_global_init();
#endif

    #ifdef CONFIG_ERRDISABLE
    /*modified by chenc for bug 51765, 2019-04-24*/
    #ifndef TAPPRODUCT
    errdisable_start(NULL);
    #endif
    #endif /* CONFIG_ERRDISABLE */

    /*added by yejl to fix bug43236, 2017-03-14*/
    #ifdef MEMMGR
    /* Initialize memory manager CLIs. */
    swth_sys_set_memory_threshold(0, 0, 0);
    memory_threshold_check_init();
    #endif /* MEMMGR */
    
    p_sys_glb->init_done = TRUE;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_INIT_DONE);

    p_sys_glb->telnet_port = GLB_TELNET_PORT_DEF;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_TELNET_PORT);
    
    p_sflow_glb->counter_interval = SFLOW_COUNTER_DEFAULT_INTERVAL;
    tbl_sflow_global_set_sflow_global_field(p_sflow_glb, TBL_SFLOW_GLOBAL_FLD_COUNTER_INTERVAL);

    /*added by yejl for sflow version 5*/
    p_sflow_glb->version = SFLOW_VERSION5;
    tbl_sflow_global_set_sflow_global_field(p_sflow_glb, TBL_SFLOW_GLOBAL_FLD_VERSION);
    /*ended by yejl*/

    /* initialize max vty numbers */
    p_sys_glb->max_vty = GLB_MAX_VTY_NUM;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_MAX_VTY);

    p_sys_load->cdb_queue_notice_threshold = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_NOTICE;
    p_sys_load->cdb_queue_warning_threshold = SYS_LOAD_QUEUE_DEPTH_THRESHOLD_WARNING;
    p_sys_load->sys_load_en = TRUE;
    p_sys_load->cli_back_pressure_en = TRUE;
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD);
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD);
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_SYS_LOAD_EN);
    tbl_sys_load_set_sys_load_field(p_sys_load, TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN);
    
    // fei_error_count = fei_get_error_count();
    if (fei_error_count)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Switch has %u FEI errors before init done.", fei_error_count);
    }

    return PM_E_NONE;
}

int32
swth_sys_get_reboot_type_from_reboot_info(tbl_sys_global_t* p_sys_glb)
{
    tbl_reboot_info_master_t *p_master = tbl_reboot_info_get_master();
    tbl_reboot_info_t *p_db_last = NULL;

    p_db_last = GETDATA(CTCLIB_SLISTTAIL(p_master->reboot_list));
    if (p_db_last)
    {
        if (sal_strstr(p_db_last->reboot_type, "MANUAL"))
        {
            p_sys_glb->reboot_type = GLB_REBOOT_WARM;
        }
        else if (sal_strstr(p_db_last->reboot_type, "POWER"))
        {
            p_sys_glb->reboot_type = GLB_REBOOT_COLD;
        }
        else
        {
            /* Fix bug 48978 2018-09-12, other means warm */
            p_sys_glb->reboot_type = GLB_REBOOT_WARM;
        }
        tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_REBOOT_TYPE);
    }

    return PM_E_NONE;
}

void
_swth_sys_send_reboot_trap(void* p_data)
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    /* Added by kcao for bug 39403, get reboot_type from last reboot_info */
    swth_sys_get_reboot_type_from_reboot_info(p_sys_glb);
    snmp_trap_sys_reboot(p_sys_glb->reboot_type);
    
    return;
}

extern int32
if_mgr_set_if_config_pmap();

int32
swth_sys_set_startup_done()
{
    FILE *fp = NULL;
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    int32 rc = PM_E_NONE;

    if (p_sys_glb->startup_done)
    {
        return PM_E_NONE;
    }

    swth_sys_telnet_startup_done();
    
    /* no shutdown interface */
    PM_E_RETURN(if_mgr_set_if_config_up());

    if ((fp = sal_fopen(STARTUP_CFG_DONE_FILE, SAL_OPEN_RW)) != NULL)
    {
        sal_fclose(fp);
    }
    
    ctc_task_add_timer(TASK_PRI_NORMAL, _swth_sys_send_reboot_trap, NULL, 15);

    p_sys_glb->startup_done = TRUE;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_STARTUP_DONE);

#if 0
#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 16:14 */
    ncutil_do_commit(); /* commit all startup config */
    sched_yield(); /* let commit thread running first and then copy config */
    ctc_nc_copy_config_no_verify(NULL, NULL, SR_DS_STARTUP, SR_DS_RUNNING);
    ctc_nc_session_switch_ds(NULL, SR_DS_RUNNING);
#endif /* !HAVE_NETCONF */
#endif

    logid_sys(LOG_SYSTEM_4_STARTUP, "success");
    return PM_E_NONE;
}

int32
swth_sys_set_aaa_new_model(int onoff)
{
    tbl_sys_global_t sys_glb;
    FILE *fp = NULL;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.aaa_new_model = onoff;

    if (onoff)
    {
        fp = sal_fopen("/tmp/aaa_new_model", SAL_OPEN_RW);
        if (NULL != fp)
        {
            sal_fclose(fp);
        }
    }
    else
    {
        remove("/tmp/aaa_new_model");
    }

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL);
    return PM_E_NONE;
}

int32
swth_sys_set_aaa_privilege(int privilege1, int privilege2, int privilege3)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.aaa_privilege1 = privilege1;
    sys_glb.aaa_privilege2 = privilege2;
    sys_glb.aaa_privilege3 = privilege3;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3);
    return PM_E_NONE;
}

int32
swth_sys_set_dhcp_service(int onoff)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.dhcp_service_enable = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE);
    return PM_E_NONE;
}

int32
swth_sys_set_dhcp_relay(int onoff, cfg_cmd_para_t *para)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.dhcp_relay_enable = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE);

/* modified by liwh for bug 51205, 2019-03-28 */
#ifdef OFPRODUCT
    if (onoff)
    {
        /* modified by chenc for bug 51931, 2019-06-19 */
        #ifndef USW
        CFG_PROMPT_RET("Warning: dhcp packets can not match flow table when dhcp client/relay is enabled.");
        #endif
    }
#endif
/* liwh end */

    return PM_E_NONE;
}

int32
swth_sys_set_dhcp_server(int onoff)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.dhcp_server_enable = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE);
    return PM_E_NONE;
}

int32
swth_sys_set_dhcp_snooping(int onoff)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.dhcp_snooping_enable = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE);
    return PM_E_NONE;
}

int32
swth_sys_set_http_service(int onoff)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.http_service_enable = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE);
    return PM_E_NONE;
}

int32
swth_sys_set_max_vty(int vtys, cfg_cmd_para_t *para)
{
    tbl_sys_global_t sys_glb;
	tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    char szBuf[64];
    char szCount[32];
	
    if (NULL == p_sys_glb)
    {
        return PM_E_NOT_INITED;
    }

	if (p_sys_glb->max_vty == vtys) {
		/* unchanged */
		return PM_E_NONE;
	}

    /* check currently max vty number */
    if (sal_cmd_exec(szBuf, 64, "cdbctl read/cdb/app/vty/max") != 0) {
		CFG_FUNC_ERR_RET("Check currently max vty number failed");
        return -1;
    }

    sal_cmd_get_token(szBuf, "max", szCount, 32); /* XXX */
    if (atoi(szCount) > vtys) {
		CFG_CONFLICT_RET("Can't set maximum: vty is in used");
    }

    /* modified by liwh for bug 52351, 2019-06-12 */
    if ((!p_sys_glb->rpcapi_disable) && (0 == vtys))
    {
         CFG_FUNC_ERR_RET("Check currently max vty number failed");   
    }
    /* liwh end */

    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.max_vty = vtys;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MAX_VTY);
    return PM_E_NONE;
}

static int32
app_read_to_buf(const char *filename, void *buf)
{
    FILE *fd;

    fd = sal_fopen(filename, "r");
    if (fd < 0)
        return -1;
    sal_fgets(buf, 1023, fd );
    
    sal_fclose(fd);
    return 0;
}

int32
swth_get_pid_by_name(char* name, int32* ppid)
{
    static DIR *dir;
    struct dirent *entry;
    int n;
    char *comm;
    char status[32];
    char *status_tail;
    char buf[1024];
    int32 pid;
    struct stat sb;

    dir = opendir("/proc");
    do 
    {
        if((entry = readdir(dir)) == NULL) 
        {
            closedir(dir);
            return -1;
        }
        comm = entry->d_name;
        if (!(*comm >= '0' && *comm <= '9'))
            continue;
        pid = sal_atoi(comm);

        status_tail = status + sprintf(status, "/proc/%d", pid);
        if (sal_stat(status, &sb))
            continue;

        sal_strcpy(status_tail, "/stat");
        n = app_read_to_buf(status, buf);
        if(n < 0)
           continue;

        comm = sal_strrchr(buf, ')'); /*split into "PID (cmd" and "<rest>" */
        if(comm == 0 || comm[1] != ' ')
            continue;
        *comm = 0;  /*replace ')' with '\0'*/ 
        comm = sal_strrchr(buf, '(');   /*get comm start point*/
        comm++;     /*skip '('*/

        if (sal_strcmp(comm, name)==0 )
        {
            *ppid = pid;
            return 0;
        }
    }
    while(1);
}

int32
_swth_sys_telnet_enable(uint32 port)
{
    char cmd[CMD_BUF_64];
    
    if (GLB_TELNET_PORT_DEF == port)
    {
#ifdef HAVE_DEBIAN
        sal_system("telnetd -l /bin/start_klish_for_console.sh > /dev/null");
#else
        sal_system("telnetd -l /etc/login.sh > /dev/null");
#endif
    }
    else
    {
        sal_memset(cmd, 0, sizeof(cmd));
#ifdef HAVE_DEBIAN
        sal_snprintf(cmd, CMD_BUF_64, "telnetd -p %u -l /bin/start_klish_for_console.sh > /dev/null", port);
#else
        sal_snprintf(cmd, CMD_BUF_64, "telnetd -p %u -l /etc/login.sh > /dev/null", port);
#endif
        sal_system(cmd);
    }

    return PM_E_NONE;
}

int32
_swth_sys_telnet_disable()
{
    int32 pid = -1;

    swth_get_pid_by_name("telnetd", &pid);
    if (pid == -1)
    {
        return PM_E_NONE;
    }
    
    /*kill inband telnetd*/
    kill(pid, SIGKILL);

    return PM_E_NONE;
}

int32
swth_sys_set_telnet_disable(uint32 disable, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (disable == p_db_glb->telnet_disable)
    {
        return PM_E_NONE;
    }

    if (disable)
    {
        _swth_sys_telnet_disable();
    }
    else
    {
        _swth_sys_telnet_enable(p_db_glb->telnet_port);
    }
    
    p_db_glb->telnet_disable = disable;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_TELNET_DISABLE);
    
    return PM_E_NONE;
}

int32
_swth_sys_set_telnet_port(uint32 old_port, uint32 new_port)
{
    tbl_ns_port_forwarding_t entry;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    sal_memset(&entry, 0, sizeof(entry));
    
    /* 1. delete old NS port forwarding */
    if (GLB_TELNET_PORT_DEF != old_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = old_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_DEL, &entry);
    }
    
    /* 2. add new NS port forwarding */
    if (GLB_TELNET_PORT_DEF != new_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = new_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_ADD, &entry);
    }

    /* 3. update service acl iptables chain */
    ipt_update_service_acl_port(GLB_SERVICE_TYPE_TELNET, new_port);

    if (!p_db_glb->telnet_disable)
    {
        _swth_sys_telnet_disable();
        _swth_sys_telnet_enable(new_port);
    }
    
    return PM_E_NONE;
}

int32
swth_sys_set_telnet_port(uint32 port)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    uint32 old_port = p_db_glb->telnet_port;
        
    if (port == p_db_glb->telnet_port)
    {
        return PM_E_NONE;
    }

    if (p_db_glb->startup_done)
    {
        _swth_sys_set_telnet_port(old_port, port);
    }

    p_db_glb->telnet_port = port;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_TELNET_PORT);
    
    return PM_E_NONE;
}

int32
swth_sys_telnet_startup_done()
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (GLB_TELNET_PORT_DEF != p_db_glb->telnet_port)
    {
        _swth_sys_set_telnet_port(GLB_TELNET_PORT_DEF, p_db_glb->telnet_port);
    }
    else
    {
        if (!p_db_glb->telnet_disable)
        {
            _swth_sys_telnet_enable(p_db_glb->telnet_port);
        }
    }

    /* update service acl iptables chain after startup */
    ipt_update_service_acl_port(GLB_SERVICE_TYPE_TELNET, p_db_glb->telnet_port);

    return PM_E_NONE;
}

void swth_sys_start_rpcapi(void *pArg)
{
    char szCmd[512];

    CTC_TASK_STOP_TIMER(g_p_http_start_http);

#ifdef HAVE_DEBIAN
    snprintf(szCmd, 512,  "python /var/rpcapi/run_server.py start");
#else
    snprintf(szCmd, 512,  "export PYTHONHOME=/usr; python /var/rpcapi/run_server.py start");
#endif
    system(szCmd);

    return;
}

void swth_sys_stop_rpcapi(void *pArg)
{
    char szCmd[512];

    CTC_TASK_STOP_TIMER(g_p_http_start_http);

#ifdef HAVE_DEBIAN
    snprintf(szCmd, 512, 
        "python /var/rpcapi/run_server.py stop");
#else
    snprintf(szCmd, 512, 
        "export PYTHONHOME=/usr; python /var/rpcapi/run_server.py stop");
#endif
    system(szCmd);

    return;
}

int32
app_webui_en_file(uint32 en)
{
    FILE *fp = NULL;

    if (en)
    {
        fp = sal_fopen(GLB_WEBUI_EN_FILE, "w+");
        sal_fclose(fp);
    }
    else
    {
        sal_unlink(GLB_WEBUI_EN_FILE);
    }

    return 0;
}

void swth_sys_start_http(void *pArg)
{
    char szCmd[512];

    CTC_TASK_STOP_TIMER(g_p_http_start_http);

#ifdef HAVE_DEBIAN
    snprintf(szCmd, 512,  "python /var/web/run_server.py start");
#else
    snprintf(szCmd, 512,  "export PYTHONHOME=/usr; python /var/web/run_server.py start");
#endif
    system(szCmd);
    app_webui_en_file(TRUE);

    return;
}

void swth_sys_stop_http(void *pArg)
{
    char szCmd[512];

    CTC_TASK_STOP_TIMER(g_p_http_start_http);

#ifdef HAVE_DEBIAN
    snprintf(szCmd, 512, 
        "python /var/web/run_server.py stop");
#else
    snprintf(szCmd, 512, 
        "export PYTHONHOME=/usr; python /var/web/run_server.py stop");
#endif
    system(szCmd);
    app_webui_en_file(FALSE);
    
    return;
}

void swth_sys_restart_http(void *pArg)
{
    char szCmd[512];

    CTC_TASK_STOP_TIMER(g_p_http_restart_http);

#ifdef HAVE_DEBIAN
    snprintf(szCmd, 512, 
        "python /var/web/run_server.py restart");
#else
    snprintf(szCmd, 512, 
        "export PYTHONHOME=/usr; python /var/web/run_server.py restart");
#endif
    system(szCmd);
    app_webui_en_file(TRUE);

    return;
}

void swth_sys_set_http_conf(uint32 port,uint32 disable)
{
    FILE *fp = NULL;

    fp = sal_fopen(HTTP_CONF_PATH, "w+");
    if(!fp)
    {
        return;
    }
    
    /* write file */
    sal_fprintf(fp, "port=%d\n", port);
    sal_fprintf(fp, "disable=%d\n", disable);
    sal_fprintf(fp, "ssl_en=0\n");

    sal_fclose(fp);
}

int32
swth_sys_set_https_conf(uint32 disable, uint32 port)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if ((fp = sal_fopen(HTTP_CONF_PATH, "w+")) == NULL)
    {
        goto err_out;
    }

    /* output default configuration */
    sal_fprintf(fp, "port=%u\n", port);
    sal_fprintf(fp, "disable=%u\n", disable);
    sal_fprintf(fp, "ssl_en=1\n");
    
err_out:
    if (NULL != fp)
    {
        sal_fclose(fp);
    }
    
    return rc;
}

int32
swth_sys_set_https_disable(uint32 disable, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t    *p_db_ssh = tbl_ssh_cfg_get_ssh_cfg();

    if (!p_db_glb->http_disable)
    {
        CFG_INVALID_PARAM_RET("Please disable HTTP service at first");
    }
    
    if (!disable)
    {
        if (!p_db_glb->rpcapi_disable)
        {
            if (p_db_glb->https_port == p_db_glb->rpcapi_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by RPCAPI service", p_db_glb->rpcapi_port);
            }

            if (p_db_glb->rpcapi_ssl_en)
            {
                CFG_CONFLICT_RET("The RPCAPI SSL service has been started");
            }
        }

        if (!p_db_glb->telnet_disable)
        {
            if (p_db_glb->http_port == p_db_glb->telnet_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by telnet service", p_db_glb->http_port);
            }
        }

        if (p_db_ssh->enable)
        {
            if (p_db_glb->http_port == p_db_ssh->listen_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by ssh service", p_db_glb->http_port);
            }
        }
    }
    
    if (disable == p_db_glb->https_disable)
    {
        return PM_E_NONE;
    }
    
    p_db_glb->https_disable = disable;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE);

    if (disable)
    {
        _swth_sys_set_rpcapi_stunnel_conf(FALSE, GLB_HTTP_PORT_INTERNAL, GLB_HTTPS_PORT_DEF);
        swth_sys_set_https_conf(FALSE, GLB_HTTP_PORT_INTERNAL);
        sal_system("killall stunnel 1>/dev/null 2>&1");
        g_p_http_stop_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_stop_http, NULL, HTTP_START_TIMEER_MS);
    }
    else
    {
        _swth_sys_set_rpcapi_stunnel_conf(TRUE, GLB_HTTP_PORT_INTERNAL, p_db_glb->https_port);
        swth_sys_set_https_conf(disable, GLB_HTTP_PORT_INTERNAL);
        sal_system("stunnel");
        g_p_http_start_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_start_http, NULL, HTTP_START_TIMEER_MS);
    }

    return PM_E_NONE;
}

int32
swth_sys_set_https_port(uint32 port)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    
    if (port == p_db_glb->https_port)
    {
        return PM_E_NONE;
    }
    
    _swth_sys_set_http_port_ns(p_db_glb->https_port, port);
    
    p_db_glb->https_port = port;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTPS_PORT);

    return PM_E_NONE;
}

int
swth_sys_check_vty_resource_ok()
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_vty_master_t *p_master;
    tbl_vty_t *p_db_vty;
    uint32 id;
    uint32 count = 0;

    /* for vty */
    p_master = tbl_vty_get_master();
    if(NULL == p_master)
    {
        return PM_E_INVALID_PARAM;
    }
    
    for (id = 0; id < GLB_MAX_VTY_NUM; id++) {
        p_db_vty = p_master->vty_array[id];
        if (NULL == p_db_vty) {
            continue;
        }
        if (p_db_vty->inuse) {
            count ++;
        }
    }

    if(p_sys_glb && (count >= p_sys_glb->max_vty))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int32
swth_sys_set_http_disable(uint32 disable, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t    *p_db_ssh = tbl_ssh_cfg_get_ssh_cfg();

    if (!disable)
    {
        if (!p_db_glb->rpcapi_disable)
        {
            if (p_db_glb->http_port == p_db_glb->rpcapi_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by RPCAPI service", p_db_glb->http_port);
            }
            if (p_db_glb->http_port == p_db_glb->rpcapi_ssl_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by RPCAPI SSL service", p_db_glb->http_port);
            }
        }

        if (!p_db_glb->telnet_disable)
        {
            if (p_db_glb->http_port == p_db_glb->telnet_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by telnet service", p_db_glb->http_port);
            }
        }

        if (p_db_ssh->enable)
        {
            if (p_db_glb->http_port == p_db_ssh->listen_port)
            {
                CFG_CONFLICT_RET("The port %u has been occupied by ssh service", p_db_glb->http_port);
            }
        }
    }
    
    if (!p_db_glb->https_disable)
    {
        CFG_INVALID_PARAM_RET("Please disable HTTPS service at first");
    }
    
    if (disable == p_db_glb->http_disable)
    {
        return PM_E_NONE;
    }
    
    p_db_glb->http_disable = disable;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTP_DISABLE);

    swth_sys_set_http_conf(p_db_glb->http_port,disable);

    if(disable)
    {
        g_p_http_stop_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_stop_http, NULL, HTTP_START_TIMEER_MS);
    }
    else
    {
        g_p_http_start_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_start_http, NULL, HTTP_START_TIMEER_MS);
    }

    return PM_E_NONE;
}

int32
_swth_sys_set_rpcapi_stunnel_conf(uint32 ssl_en, uint32 port, uint32 ssl_port)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if ((fp = sal_fopen(M_STUNNEL_CFGFILE, "w+")) == NULL)
    {
        goto err_out;
    }
    
    /* output default configuration */
    sal_fprintf(fp, "debug = 2 \n");
    sal_fprintf(fp, ";output = /switch/etc/stunnel/stunnel.log \n");
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "[https]\n");
    sal_fprintf(fp, "accept = :::%u \n", ssl_port);
    sal_fprintf(fp, "connect = ::1:%u \n", GLB_HTTP_PORT_INTERNAL);
    sal_fprintf(fp, "cert = /switch/etc/stunnel/stunnel.pem \n");
    
err_out:
    if (NULL != fp)
    {
        sal_fclose(fp);
    }
    
    return rc;
}

int32
_swth_sys_set_rpcapi_http_conf(uint32 rpcapi_en, uint32 port, uint32 ssl_en, uint32 auth_mode)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if ((fp = sal_fopen(M_RPCAPI_CFGFILE, "w+")) == NULL)
    {
        goto err_out;
    }

    /* output default configuration */
    sal_fprintf(fp, "port=%u\n", port);
    sal_fprintf(fp, "disable=%u\n", !rpcapi_en);
    sal_fprintf(fp, "ssl_en=%u\n", ssl_en);
    sal_fprintf(fp, "auth_mode=%u\n", auth_mode);
    
err_out:
    if (NULL != fp)
    {
        sal_fclose(fp);
    }
    
    return rc;
}

int32
_swth_sys_set_http_port_ns(uint32 old_port, uint32 new_port)
{
    tbl_ns_port_forwarding_t entry;

    sal_memset(&entry, 0, sizeof(entry));
    
    /* 1. delete old NS port forwarding */
    if (GLB_HTTP_PORT_DEF != old_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = old_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_DEL, &entry);
    }
    
    /* 2. add new NS port forwarding */
    if (GLB_HTTP_PORT_DEF != new_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = new_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_ADD, &entry);
    }
    
    return PM_E_NONE;
}

int32
_swth_sys_set_https_port_ns(uint32 old_port, uint32 new_port)
{
    tbl_ns_port_forwarding_t entry;

    sal_memset(&entry, 0, sizeof(entry));
    
    /* 1. delete old NS port forwarding */
    if (GLB_HTTPS_PORT_DEF != old_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = old_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_DEL, &entry);
    }
    
    /* 2. add new NS port forwarding */
    if (GLB_HTTPS_PORT_DEF != new_port)
    {
        entry.key.type = NS_PORT_FORWARDING_LOCAL;
        entry.key.proto = NS_PORT_FORWARDING_TCP;
        entry.key.port = new_port;
        entry.key.pad = 0;
        entry.key.mapped_addr.s_addr = 0;
        entry.ip.s_addr = 0;
        ns_port_forwarding_op_entry(CDB_OPER_ADD, &entry);
    }
    
    return PM_E_NONE;
}

int32
_swth_sys_set_rpcapi_ssl_en(tbl_sys_global_t *p_db_glb, uint32 ssl_en, uint32 port, uint32 ssl_port)
{
    if (!ssl_en)
    {
        ssl_port = GLB_HTTPS_PORT_DEF;
    }
    /* remove for bug 53600 restart rpc with saved port configuration
    if (p_db_glb->rpcapi_port != port) */
    {
        _swth_sys_set_http_port_ns(p_db_glb->rpcapi_port, port);
    }
    /* remove for bug 53600 restart rpc with saved port configuration
    if (p_db_glb->rpcapi_ssl_port != ssl_port) */
    {
        _swth_sys_set_https_port_ns(p_db_glb->rpcapi_ssl_port, ssl_port);
    }
    _swth_sys_set_rpcapi_stunnel_conf(ssl_en, port, ssl_port);
    if (ssl_en)
    {
        _swth_sys_set_rpcapi_http_conf(TRUE, GLB_HTTP_PORT_INTERNAL, ssl_en, p_db_glb->rpcapi_auth_mode);
    }
    else
    {
        _swth_sys_set_rpcapi_http_conf(TRUE, port, ssl_en, p_db_glb->rpcapi_auth_mode);
    }
    if (ssl_en)
    {
        sal_system("stunnel");
    }
    
    return PM_E_NONE;
}

int32
_swth_sys_set_rpcapi_ssl_dis(tbl_sys_global_t *p_db_glb)
{
    if (p_db_glb->rpcapi_port != GLB_HTTP_PORT_DEF)
    {
        _swth_sys_set_http_port_ns(p_db_glb->rpcapi_port, GLB_HTTP_PORT_DEF);
    }

    if (!p_db_glb->rpcapi_ssl_en)
    {
        return PM_E_NONE;
    }
        
    if (p_db_glb->rpcapi_ssl_port != GLB_HTTPS_PORT_DEF)
    {
        _swth_sys_set_https_port_ns(p_db_glb->rpcapi_ssl_port, GLB_HTTPS_PORT_DEF);
    }
    _swth_sys_set_rpcapi_stunnel_conf(FALSE, GLB_HTTP_PORT_INTERNAL, GLB_HTTPS_PORT_DEF);
    _swth_sys_set_rpcapi_http_conf(FALSE, GLB_HTTP_PORT_INTERNAL, FALSE, p_db_glb->rpcapi_auth_mode);
    sal_system("killall stunnel 1>/dev/null 2>&1");
    
    return PM_E_NONE;
}

int32
_swth_sys_set_rpcapi_en(tbl_sys_global_t *p_db_glb, uint32 ssl_en, uint32 port, uint32 ssl_port)
{
    _swth_sys_set_rpcapi_ssl_en(p_db_glb, ssl_en, port, ssl_port);
    g_p_http_start_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_start_rpcapi, NULL, HTTP_START_TIMEER_MS);

    p_db_glb->rpcapi_disable = FALSE;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE);
    p_db_glb->rpcapi_port = port;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_PORT);
    p_db_glb->rpcapi_ssl_en = ssl_en;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN);
    p_db_glb->rpcapi_ssl_port = ssl_port;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT);
    
    return PM_E_NONE;
}

int32
_swth_sys_set_rpcapi_dis(tbl_sys_global_t *p_db_glb)
{
    _swth_sys_set_rpcapi_ssl_dis(p_db_glb);
    g_p_http_start_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_stop_rpcapi, NULL, HTTP_START_TIMEER_MS);

    p_db_glb->rpcapi_disable = TRUE;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE);
    /* p_db_glb->rpcapi_port = 0;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_PORT);
    p_db_glb->rpcapi_ssl_en = FALSE;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN);
    p_db_glb->rpcapi_ssl_port = GLB_HTTPS_PORT_DEF;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT) */;

    sal_unlink(RPCAPI_COUNTER_FILE);

    return PM_E_NONE;
}

int32
swth_sys_set_rpcapi_disable(uint32 disable, uint32 ssl_en, uint32 port, uint32 ssl_port, cfg_cmd_para_t *para)
{
    tbl_sys_global_t  *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_user_master_t *p_master = tbl_user_get_master();
    int32 rc = PM_E_NONE;

    if (disable == p_db_glb->rpcapi_disable)
    {
        if (!disable)
        {
            /* not allow update */
            CFG_INVALID_PARAM_RET("Please disable RPC-API service at first");
        }
        return PM_E_NONE;
    }

    if (!disable)
    {
        /* support rpc service restart */
        if (0 == port)
        {
            port = p_db_glb->rpcapi_port;
            ssl_en   = p_db_glb->rpcapi_ssl_en;
            ssl_port = p_db_glb->rpcapi_ssl_port;
        }

        if (port == ssl_port)
        {
            /* not allow update */
            CFG_INVALID_PARAM_RET("RPC-API port must not equal to SSL port");
        }

        if (port == p_db_glb->https_port)
        {
            CFG_CONFLICT_RET("The port %u has been occupied by HTTPS service", port);
        }

        if (ssl_en)
        {
            if (!p_db_glb->https_disable)
            {
                CFG_CONFLICT_RET("The HTTPS service has been started");
            }
        }

        if (0 == p_master->user_list->count && GLB_RPCAPI_AUTH_BASIC == p_db_glb->rpcapi_auth_mode)
        {
            CFG_CONFLICT_RET("Can not enable RPCAPI with basic auth mode while no username exist in system");
        }
    }
    
    if (!ssl_en)
    {
        if (!p_db_glb->http_disable)
        {
            PM_E_RETURN(lib_cmd_check_port(port, LIB_SERVICE_PORT_RPCAPI, para));
        }
    }

    if (ssl_en)
    {
        PM_E_RETURN(lib_cmd_check_port(ssl_port, LIB_SERVICE_PORT_RPCAPI_SSL, para));
    }
    
    if (!disable)
    {
        if (FALSE == swth_sys_check_vty_resource_ok())
        {
            CFG_INVALID_PARAM_RET("Too many users");
        }
    }

    p_db_glb->oper_rpcapi_auth_mode = p_db_glb->rpcapi_auth_mode;
    if (disable)
    {
        return _swth_sys_set_rpcapi_dis(p_db_glb);
    }
    else
    {
        return _swth_sys_set_rpcapi_en(p_db_glb, ssl_en, port, ssl_port);
    }
    
    return PM_E_NONE;
}

int32
swth_sys_set_rpcapi_auth_mode(uint32 auth_mode, cfg_cmd_para_t *para)
{
    tbl_sys_global_t  *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_user_master_t *p_master = tbl_user_get_master();

    if (auth_mode == p_db_glb->rpcapi_auth_mode)
    {
        return PM_E_NONE;
    }

    if (0 == p_master->user_list->count && GLB_RPCAPI_AUTH_BASIC == auth_mode)
    {
        CFG_CONFLICT_RET("Can not set RPCAPI basic auth mode while no username exist in system");
    }

    p_db_glb->rpcapi_auth_mode = auth_mode;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE);

    _swth_sys_set_rpcapi_http_conf(!p_db_glb->rpcapi_disable, p_db_glb->rpcapi_port, p_db_glb->rpcapi_ssl_en, p_db_glb->rpcapi_auth_mode);
    if (!p_db_glb->rpcapi_disable)
    {
        CFG_PROMPT_RET("This action will take effect after you restart RPC API service");
    }
    else
    {
        p_db_glb->oper_rpcapi_auth_mode = p_db_glb->rpcapi_auth_mode;
    }
    
    return PM_E_NONE;
}

int32
swth_sys_set_http_port(uint32 port)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (port == p_db_glb->http_port)
    {
        return PM_E_NONE;
    }
    
    _swth_sys_set_http_port_ns(p_db_glb->http_port, port);
    
    p_db_glb->http_port = port;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTP_PORT);
    swth_sys_set_http_conf(port,p_db_glb->http_disable);

    if(!p_db_glb->http_disable)
    {
        g_p_http_restart_http = ctc_task_add_timer_msec(TASK_PRI_NORMAL, swth_sys_restart_http, NULL, HTTP_START_TIMEER_MS);
    }

    return PM_E_NONE;
}

int32
swth_sys_set_http_timeout(uint32 interval)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    p_db_glb->http_timeout = interval * 60;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT);
    
    return PM_E_NONE;
}

int32
swth_sys_set_cut_through_enable(uint32 cut_through_enable)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (cut_through_enable == p_db_glb->cut_through_enable)
    {
        return PM_E_NONE;
    }

    p_db_glb->cut_through_enable = cut_through_enable;
    tbl_sys_global_set_sys_global_field(p_db_glb, TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE);

    if (syscmd_file_exist(STARTUP_CFG_DONE_FILE))
    {
        return PM_E_EXIST;
    }
    return PM_E_NONE;
}


int32
swth_sys_set_http_image_name(char *pszImage)
{
    tbl_sys_global_t sys_glb;
    
    if (NULL == pszImage) {
        memset(sys_glb.http_image_name, 0x00, sizeof(char) * HTTP_IMAGE_NAME_SIZE);
    } else {
        sal_snprintf(sys_glb.http_image_name, HTTP_IMAGE_NAME_SIZE, "%s", pszImage);
    }

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME);
    return PM_E_NONE;
}

int32
swth_sys_set_service_password_encryption(int onoff)
{
    tbl_sys_global_t sys_glb;
    
    sal_memset(&sys_glb, 0, sizeof(sys_glb));    
    sys_glb.service_password_encryption = onoff;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION);
    return PM_E_NONE;
}

int32
swth_sys_set_tap_timestamp_ether(mac_addr_t macda, mac_addr_t macsa, uint32 ether_type)
{
    tbl_sys_global_t sys_glb;

    //fei_tap_timestamp_ether_type(macda, macsa, ether_type);

    sal_memset(&sys_glb, 0, sizeof(tbl_sys_global_t));
    sal_memcpy(sys_glb.tap_ts_macda, macda, sizeof(mac_addr_t));
    sal_memcpy(sys_glb.tap_ts_macsa, macsa, sizeof(mac_addr_t));
    sys_glb.ether_type = ether_type;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_ETHER_TYPE);
    return PM_E_NONE;
}

int32
swth_sys_set_truncation_length(uint32 length)
{
    tbl_sys_global_t sys_glb;

   // fei_tap_truncation_len(length);
    sys_glb.trunction_length = length;
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH);
    return PM_E_NONE;
}

int32
swth_sys_set_ptf_port(uint32 ptf_port)
{
    tbl_sys_global_t sys_glb;

    sys_glb.ptf_port = ptf_port;
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_PTF_PORT);
    return PM_E_NONE;
}

char*
swth_sys_get_memchkstate_val2str(uint32 memchk_state)
{
    switch (memchk_state)
    {
    case SYSLIMIT_MEMCHECK_STATE_NORMAL:
        return "normal";
    case SYSLIMIT_MEMCHECK_STATE_WARNING:
        return "warning";
    case SYSLIMIT_MEMCHECK_STATE_ALERT:
        return "alert";
    case SYSLIMIT_MEMCHECK_STATE_CRITICAL:
        return "critical";
    default:
        return "invalid";
    }
}

int32
swth_sys_get_reserve_and_total_memory(uint32 *sys_kernel_reserve, uint32 *sys_total)
{
    int mem_val = 0; 
    int commit_limit = 0;
    int total = 0;
    FILE *pFile = NULL;
    char strLine[CMD_BUF_64];
    char strName[CMD_BUF_64];  

    *sys_kernel_reserve = 0;
    *sys_total = SYS_MEM_DEF_VALUE;
    
    /*read the file and get the Memtotal and kernal reserve*/
    pFile = sal_fopen(MEM_INFO_PATH, "r");
    if ((NULL == pFile) || (sal_feof(pFile)))
    {
        return PM_E_NONE;
    }
    sal_memset(strLine, 0, sizeof(strLine));
    sal_memset(strName, 0, sizeof(strName));
    while (sal_fgets(strLine, CMD_BUF_64, pFile))
    {
        sal_sscanf(strLine, "%s%d", strName, &mem_val);
        if (!sal_strncmp ("MemTotal:", strName, 9))
        {
            total = mem_val / 1024;
        }
        else if (!sal_strncmp ("CommitLimit:", strName, 12))
        {
            commit_limit = mem_val / 1024;
        }
    }
    sal_fclose(pFile);

    if (!total || !commit_limit)
    {
        return PM_E_NONE;
    }
    if (total < commit_limit)
    {
        return PM_E_NONE;
    }
    *sys_total = total;
    *sys_kernel_reserve = total - commit_limit;
    
    return PM_E_NONE;
}

void
swth_sys_get_memorycheck_states(void *p_argv)
{
    int32 mem_val = 0; 
    int32 memfree = 0;
    int32 total = 0;
    FILE *pFile = NULL;
    char strLine[CMD_BUF_64];
    char strName[CMD_BUF_64];  
    tbl_sys_global_t *sys_db_glb = NULL;
    tbl_sys_global_t sys_glb;
    uint32 threshold1 = 0;
    uint32 threshold2 = 0;
    uint32 threshold3 = 0;

    sal_memset(&sys_glb, 0, sizeof(tbl_sys_global_t));
    sal_memset(strLine, 0, sizeof(strLine));
    sal_memset(strName, 0, sizeof(strName));

    /*get the system global table*/
    sys_db_glb = tbl_sys_global_get_sys_global();
    if (sys_db_glb == NULL)
    {
        return;
    }

    /*read the file and get the system free memory*/
    pFile = sal_fopen(MEM_INFO_PATH, "r");
    if ((NULL == pFile) || (sal_feof(pFile)))
    {
        return;
    }
    while (sal_fgets(strLine, CMD_BUF_64, pFile))
    {
        sal_sscanf(strLine, "%s%d", strName, &mem_val);
        if (!sal_strncmp ("MemTotal:", strName, 9))
        {
            total = mem_val / 1024;
        }
        else if (!sal_strncmp ("MemFree:", strName, 8))
        {
            memfree = mem_val /1024;
        }
#if (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
        else if (!sal_strncmp ("CmaTotal:", strName, 9))
        {
            memfree -= mem_val / 1024;
        }
#endif
    }
    sal_fclose(pFile);
    pFile = NULL;

    if (total < 0 || memfree < 0)
    {
        return;
    }
    if (total < memfree)
    {
        return;
    }
    /*end*/
    
    /*compare the memfree with thresholdX, the thresholdX is memory_thresholdX + memory_threshold1*/
    /*added by yejl to fix bug43231, 2017-03-14*/
    threshold1 = sys_db_glb->memory_threshold1 + sys_db_glb->memory_threshold1;
    threshold2 = sys_db_glb->memory_threshold2 + sys_db_glb->memory_threshold1;
    threshold3 = sys_db_glb->memory_threshold3 + sys_db_glb->memory_threshold1;

    if (memfree > threshold3)
    {
        sys_glb.system_memchk_state = SYSLIMIT_MEMCHECK_STATE_NORMAL;
        if (SYSLIMIT_MEMCHECK_STATE_NORMAL != sys_db_glb->system_memchk_state)
        {
            logid_sys(LOG_SYSTEM_4_MEM_NORMAL, swth_sys_get_memchkstate_val2str(sys_db_glb->system_memchk_state));
            tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE);
        }
    }
    else if ((memfree <= threshold3) && (memfree > threshold2))
    {
        sys_glb.system_memchk_state = SYSLIMIT_MEMCHECK_STATE_WARNING;
        if (SYSLIMIT_MEMCHECK_STATE_WARNING != sys_db_glb->system_memchk_state)
        {
            logid_sys(LOG_SYSTEM_3_MEM_WARNING, swth_sys_get_memchkstate_val2str(sys_db_glb->system_memchk_state));
            tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE);
        }
    }
    else if ((memfree <= threshold2) && (memfree > threshold1))
    {
        sys_glb.system_memchk_state = SYSLIMIT_MEMCHECK_STATE_ALERT;
        if (SYSLIMIT_MEMCHECK_STATE_ALERT != sys_db_glb->system_memchk_state)
        {
            logid_sys(LOG_SYSTEM_1_MEM_ALERT, swth_sys_get_memchkstate_val2str(sys_db_glb->system_memchk_state));
            tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE);
        }
    }
    else if (memfree <= threshold1)
    {
        sys_glb.system_memchk_state = SYSLIMIT_MEMCHECK_STATE_CRITICAL;
        if (SYSLIMIT_MEMCHECK_STATE_CRITICAL != sys_db_glb->system_memchk_state)
        {
            logid_sys(LOG_SYSTEM_0_MEM_CRITICAL, swth_sys_get_memchkstate_val2str(sys_db_glb->system_memchk_state));
            tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE);
        }
    }
    else
    {
        return;
    } 
    /*end*/
    
    /*write the system memory check state in the file*/
    pFile = sal_fopen(GLB_MEMCHK_STATE_FILE, "w+");
    sal_fprintf(pFile, "%d", sys_glb.system_memchk_state);
    sal_fclose(pFile);
    pFile = NULL;
    /*end*/
    
    g_p_memcheck_timer = CTC_TASK_ADD_TIME(swth_sys_get_memorycheck_states, NULL, MEMORY_CHECK_INTERVAL_TIME);
    
    return;
}

static void
swth_sys_memchk_packet_discard_log(void *p_argv)
{
    tbl_sys_global_t *sys_db_glb = NULL;
    
    sys_db_glb = tbl_sys_global_get_sys_global();
    if (sys_db_glb == NULL)
    {
        return;
    }
    if (sys_db_glb->system_memchk_state >= SYSLIMIT_MEMCHECK_STATE_WARNING)
    {
        /*modified by yejl to fix bug43266, 2017-03-14*/
        logid_sys(LOG_SYSTEM_3_MEM_PACKETDISCARD, "lacp, igmp, arp, dhcp, lldp, erps, mlag");
    }
    
    g_p_memcheck_packet_discard_log_timer = CTC_TASK_ADD_TIME(swth_sys_memchk_packet_discard_log, NULL, MEMORY_CHECK_LOG_PRINT_TIME);
    
    return;
}


int32
swth_sys_set_memory_threshold(uint32 mem_threshold1, uint32 mem_threshold2, uint32 mem_threshold3)
{
    uint32 sys_total = 0;
    uint32 sys_kernel_reserve = 0;
    tbl_sys_global_t sys_glb;

    sal_memset(&sys_glb, 0, sizeof(tbl_sys_global_t));

    swth_sys_get_reserve_and_total_memory(&sys_kernel_reserve, &sys_total);
    /* reserve too much memory, ignore it */
    if (sys_kernel_reserve > 100)
    {
        log_sys(M_MOD_LIB, E_WARNING, "System reserve too much memory %d", sys_kernel_reserve);
        sys_kernel_reserve = 0;
    }

    //thresh is unset
    if ((0 == mem_threshold1) && (0 == mem_threshold2) && (0 == mem_threshold3))
    {
        if (sys_total > SYS_MEM_DEF_VALUE)
        {
            mem_threshold1 = SYS_MEM_DEF_VALUE * SYSTEM_MEM_DEFAULT_PECENT_THRESH1/100;
            mem_threshold2 = SYS_MEM_DEF_VALUE * SYSTEM_MEM_DEFAULT_PECENT_THRESH2/100;
            mem_threshold3 = SYS_MEM_DEF_VALUE * SYSTEM_MEM_DEFAULT_PECENT_THRESH3/100;
        }
        else if (sys_total > (SYS_MEM_DEF_VALUE/4))
        {
            /*4%;6%;8%*/
            mem_threshold1 = sys_total * SYSTEM_MEM_DEFAULT_PECENT_THRESH1/100;
            mem_threshold2 = sys_total * SYSTEM_MEM_DEFAULT_PECENT_THRESH2/100;
            mem_threshold3 = sys_total * SYSTEM_MEM_DEFAULT_PECENT_THRESH3/100;
        }
        else
        {
            /*1%;2%;3%*/
            mem_threshold1 = sys_total * 1/200;
            mem_threshold2 = sys_total * 2/200;
            mem_threshold3 = sys_total * 3/200;
        }
    }

    sys_glb.memory_threshold1 = mem_threshold1 + sys_kernel_reserve;
    sys_glb.memory_threshold2 = mem_threshold2 + sys_kernel_reserve;
    sys_glb.memory_threshold3 = mem_threshold3 + sys_kernel_reserve;

    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2);
    tbl_sys_global_set_sys_global_field(&sys_glb, TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3);

    return PM_E_NONE;
}

int32
memory_threshold_check_init()
{
    /*if the g_p_memcheck_timer is not NULL, stop the timer firstly and the set the new timer*/
    if (CTC_TASK_IS_RUNNING(g_p_memcheck_timer))
    {
        CTC_TASK_STOP_TIMER(g_p_memcheck_timer);
    }
    g_p_memcheck_timer = CTC_TASK_ADD_TIME(swth_sys_get_memorycheck_states, NULL, MEMORY_CHECK_INTERVAL_TIME);
    /*end*/

    /*if the g_p_memcheck_packet_discard_log_timer is not NULL, stop the timer firstly and the set the new timer*/
    if (CTC_TASK_IS_RUNNING(g_p_memcheck_packet_discard_log_timer))
    {
        CTC_TASK_STOP_TIMER(g_p_memcheck_packet_discard_log_timer);
    }
    g_p_memcheck_packet_discard_log_timer = CTC_TASK_ADD_TIME(swth_sys_memchk_packet_discard_log, NULL, MEMORY_CHECK_LOG_PRINT_TIME);
    /*end*/
    
    return PM_E_NONE;
}

