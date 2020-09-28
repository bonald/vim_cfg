
#include "proto.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_syslog_cfg_define.h"
#include "gen/tbl_syslog_cfg.h"
#include "gen/tbl_authen_define.h"
#include "gen/tbl_authen.h"
#include "gen/tbl_rsa_define.h"
#include "gen/tbl_rsa.h"
#include "gen/tbl_user_define.h"
#include "gen/tbl_user.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "gen/tbl_snmp_cfg_define.h"
#include "gen/tbl_snmp_cfg.h"
#include "gen/tbl_snmp_view_define.h"
#include "gen/tbl_snmp_view.h"
#include "gen/tbl_snmp_community_define.h"
#include "gen/tbl_snmp_community.h"
#include "gen/tbl_snmp_trap_define.h"
#include "gen/tbl_snmp_trap.h"
#include "gen/tbl_snmp_inform_define.h"
#include "gen/tbl_snmp_inform.h"
#include "gen/tbl_snmp_context_define.h"
#include "gen/tbl_snmp_context.h"
#include "gen/tbl_snmp_usm_user_define.h"
#include "gen/tbl_snmp_usm_user.h"
#include "gen/tbl_snmp_group_usm_define.h"
#include "gen/tbl_snmp_group_usm.h"
#include "gen/tbl_snmp_access_usm_define.h"
#include "gen/tbl_snmp_access_usm.h"
#include "gen/tbl_snmp_notify_define.h"
#include "gen/tbl_snmp_notify.h"
#include "gen/tbl_snmp_target_addr_define.h"
#include "gen/tbl_snmp_target_addr.h"
#include "gen/tbl_snmp_target_params_define.h"
#include "gen/tbl_snmp_target_params.h"
#include "gen/tbl_rmon_event_define.h"
#include "gen/tbl_rmon_event.h"
#include "gen/tbl_rmon_alarm_define.h"
#include "gen/tbl_rmon_alarm.h"
#include "gen/tbl_console_define.h"
#include "gen/tbl_console.h"
#include "gen/tbl_vty_define.h"
#include "gen/tbl_vty.h"
#include "../../../appcfg/include/app_ntp.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_ace_define.h"
#include "gen/tbl_ntp_server_define.h"
#include "gen/tbl_ntp_key_define.h"
#include "gen/tbl_ntp_server.h"
#include "gen/tbl_ntp_cfg.h"
#include "gen/tbl_ntp_ace.h"
#include "gen/tbl_ntp_key.h"
#include "gen/tbl_cpu_limit_define.h"
#include "gen/tbl_cpu_limit.h"
#include "gen/tbl_iptables_prevent_define.h"
#include "gen/tbl_iptables_prevent.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcrelay.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "gen/tbl_auth_cfg_define.h"
#include "gen/tbl_auth_cfg.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "gen/tbl_enable_define.h"
#include "gen/tbl_enable.h"
#include "gen/tbl_author_define.h"
#include "gen/tbl_author.h"
#include "gen/tbl_account_define.h"
#include "gen/tbl_account.h"
#include "gen/tbl_accountcmd_define.h"
#include "gen/tbl_accountcmd.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "gen/tbl_acl_worm_filter_define.h"
#include "gen/tbl_acl_worm_filter.h"
#include "gen/tbl_tag_summer_clock_define.h"
#include "gen/tbl_tag_summer_clock.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_login_security_glb_define.h"
#include "gen/tbl_login_security_glb.h"
#include "rsa_derhex.h"
#include "lib_fs.h"
#include "ccs_cfg.h"
#include "../../../appcfg/include/app_auth.h"
extern int32
inband_snat_get_if_or_ip(const addr_ipv4_t *ip, uint16_t port, inband_snat_proto_t proto, char *buf);

/**
 * get inband/outbond via ip+port+protocol
 * @return
 *     0: outband, 1: inband, -1: entry not exist
 */
int32
ns_route_is_inband(addr_ipv4_t ip, uint16 port, ns_route_proto_t protocol)
{
    tbl_ns_route_t *p = NULL;
    tbl_ns_route_key_t key;

    sal_memset(&key, 0, sizeof(tbl_ns_route_key_t));
    key.ip.family = AF_INET;
    key.ip.u.prefix4.s_addr = ip.s_addr;
    key.port = port;
    key.proto = protocol;

    p = tbl_ns_route_get_ns_route(&key);
    if (NULL == p)
    {
        return -1;
    }
    return p->is_inband;
}

int32
ns_route_ipv6_is_inband(addr_ipv6_t ipv6_addr, uint16 port, ns_route_proto_t protocol)
{
    tbl_ns_route_t *p = NULL;
    tbl_ns_route_key_t key;

    sal_memset(&key, 0, sizeof(tbl_ns_route_key_t));
    key.ip.family = AF_INET6;
    sal_memcpy(&key.ip.u.prefix6, &ipv6_addr, sizeof(addr_ipv6_t));
    key.port = port;
    key.proto = protocol;

    p = tbl_ns_route_get_ns_route(&key);
    if (NULL == p)
    {
        return -1;
    }
    return p->is_inband;
}

int32
ccs_cfg_build_app_ssh_cfg(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_ssh_cfg_t *p_db_cfg = tbl_ssh_cfg_get_ssh_cfg();
    int i;
    uint32 has_config = FALSE;
   

    if (!p_db_cfg->enable) {
        sal_fprintf(fp, "ip ssh server disable\n");
        has_config = TRUE;
    }
    if (SSH_VERSION_V2 != p_db_cfg->version) {
        sal_fprintf(fp, "ip ssh server version %s\n",
            (SSH_VERSION_V1 == p_db_cfg->version) ? "v1" : "all");
        has_config = TRUE;
    }
    if (120 != p_db_cfg->auth_timeout) {
        sal_fprintf(fp, "ip ssh server authentication-timeout %u\n", p_db_cfg->auth_timeout);
        has_config = TRUE;
    }
    if (6 != p_db_cfg->auth_retry) {
        sal_fprintf(fp, "ip ssh server authentication-retries %u\n", p_db_cfg->auth_retry);
        has_config = TRUE;
    }
    if (22 != p_db_cfg->listen_port) {
        sal_fprintf(fp, "ip ssh server port %u\n", p_db_cfg->listen_port);
        has_config = TRUE;
    }
    if (60 != p_db_cfg->rekey_interval) {
        sal_fprintf(fp, "ip ssh server rekey-interval %u\n", p_db_cfg->rekey_interval);
        has_config = TRUE;
    }

    if ((SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY) != p_db_cfg->auth_type) {
        if ((SSH_AUTH_TYPE_PASSWORD | SSH_AUTH_TYPE_PUBKEY | SSH_AUTH_TYPE_RSA) 
                == p_db_cfg->auth_type) {
            sal_fprintf(fp, "ip ssh server authentication-type all\n");
            has_config = TRUE;
        } else {
            i = 0;
            sal_fprintf(fp, "ip ssh server authentication-type ");
            if (p_db_cfg->auth_type & SSH_AUTH_TYPE_PASSWORD) {
                sal_fprintf(fp, "password");
                i++;
                has_config = TRUE;
            }
            if (p_db_cfg->auth_type & SSH_AUTH_TYPE_PUBKEY) {
                if (!i) {
                    sal_fprintf(fp, "public-key");
                    has_config = TRUE;
                } else {
                    sal_fprintf(fp, " public-key");
                    has_config = TRUE;
                }
                i++;
            }
            if (p_db_cfg->auth_type & SSH_AUTH_TYPE_RSA) {
                if (!i) {
                    sal_fprintf(fp, "rsa");
                    has_config = TRUE;
                } else {
                    sal_fprintf(fp, " rsa");
                    has_config = TRUE;
                }
                i++;
            }
            sal_fprintf(fp, "\n");
        }
    }

    if ('\0' != p_db_cfg->hostkey[0]) {
        sal_fprintf(fp, "ip ssh server host-key rsa key %s\n", p_db_cfg->hostkey);
        has_config = TRUE;
    }

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-25 14:05 */
    if (p_db_cfg->netconf_service) {
        if (830 != p_db_cfg->netconf_port) {
            sal_fprintf(fp, "service netconf enable port %u\n", p_db_cfg->netconf_port);
        } else {
            sal_fprintf(fp, "service netconf enable\n");
        }
        has_config = TRUE;
    }
#endif /* !HAVE_NETCONF */
    
    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

static int
__print_server_addr_item(void *pArg, void *pArg2, int vrf, const char *pszAddr, int nIdx)
{
    int res;
    FILE *fp = (FILE *)pArg;
    addr_ipv4_t ip;
    char buf[IFNAME_SIZE] = "";
#if 0
    fprintf(fp, "logging server address %s%s\n", 
        (MGMT_VRF == vrf) ? "mgmt-if " : "", pszAddr);
#else 
    sal_inet_pton(AF_INET, pszAddr, &ip);
    res = inband_snat_get_if_or_ip(&ip, 514, INBAND_IP_PROTO_UDP, buf);

    fprintf(fp, "logging server address %s%s %s %s\n", 
        (MGMT_VRF == vrf) ? "mgmt-if ": "",
        pszAddr,
        PM_E_NONE == res ? "source-ip" : "",
        PM_E_NONE == res ? buf : "");
#endif
    return 0;
}

int32
ccs_cfg_build_app_syslog_cfg(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_syslog_cfg_t *p_db_syslog = tbl_syslog_cfg_get_syslog_cfg();
    uint32 has_config = FALSE;

    if (500 != p_db_syslog->logging_lines) 
    {
        sal_fprintf(fp, "logging buffer %d\n", 
            p_db_syslog->logging_lines);
        has_config = TRUE;
    }

    if (strcasecmp(cdb_enum_val2str(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX, 
        p_db_syslog->timestamp), "bsd")) 
    {
        sal_fprintf(fp, "logging timestamp %s\n", 
            cdb_enum_val2str(syslog_timestamp_strs, SYSLOG_TIMESTAMP_MAX,
            p_db_syslog->timestamp));
        has_config = TRUE;
    }

    if (!p_db_syslog->enable_to_file) 
    {
        sal_fprintf(fp, "logging file %s\n", p_db_syslog->enable_to_file ? "enable" : "disable");
        has_config = TRUE;
    }

    if (p_db_syslog->enable_operate) 
    {
        sal_fprintf(fp, "logging operate %s\n", p_db_syslog->enable_operate ? "enable" : "disable");
        has_config = TRUE;
    }

    if (strcasecmp(cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
        p_db_syslog->file_severity), "information")) 
    {

        sal_fprintf(fp, "logging level file %s\n", 
           cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
            p_db_syslog->file_severity));
        has_config = TRUE;
    }

    if (strcasecmp(cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
        p_db_syslog->module_severity), "debug")) 
    {        

        sal_fprintf(fp, "logging level module %s\n",
           cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
            p_db_syslog->module_severity));
         has_config = TRUE;
    }

    if (p_db_syslog->enable_to_server) 
    {
        sal_fprintf(fp, "logging server %s\n", p_db_syslog->enable_to_server ? "enable" : "disable");
        has_config = TRUE;
    }

    if (strcasecmp(cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
        p_db_syslog->server_severity), "warning"))
    {

        sal_fprintf(fp, "logging server severity %s\n", 
            cdb_enum_val2str(syslog_severity_strs, SYSLOG_SEVERITY_MAX, 
            p_db_syslog->server_severity));
        has_config = TRUE;
    }

    if (strcasecmp(cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, 
        p_db_syslog->server_facility), "local4")) 
    {
        sal_fprintf(fp, "logging server facility %s\n", 
            cdb_enum_val2str(syslog_facility_strs, SYSLOG_FACILITY_MAX, 
        p_db_syslog->server_facility));
        has_config = TRUE;
    }
    
   
    if (NULL != p_db_syslog->server_addr && '\0' != p_db_syslog->server_addr[0]) 
    {
        logging_server_addr_iter(p_db_syslog->server_addr, __print_server_addr_item, fp, NULL);
        has_config = TRUE;
    }

    if (0 == p_db_syslog->enable_merge) 
    {
        sal_fprintf(fp, "logging merge %s\n", p_db_syslog->enable_merge ? "enable" : "disable");
        has_config = TRUE;
    }

    if (1024 != p_db_syslog->merge_fifosize)
    {
        sal_fprintf(fp, "logging merge fifo-size %d\n", p_db_syslog->merge_fifosize);
        has_config = TRUE;
    }

    if (10 != p_db_syslog->merge_timeout) 
    {
        sal_fprintf(fp, "logging merge timeout %d\n", p_db_syslog->merge_timeout);
        has_config = TRUE;
    }
    
    if (has_config) 
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_auth_cfg(FILE * fp, cfg_cmd_para_t * para)
{
/* modified by liwh for bug 43207, 2017-03-13 
   radius-server XXX will replace aaa-server XXX */
#ifdef TAPPRODUCT
    tbl_auth_cfg_t *p_db = tbl_auth_cfg_get_auth_cfg();
    uint32 has_config = FALSE;

    if (0 != p_db->timeout && GLB_AUTH_DEFAULT_TIMEOUT != p_db->timeout) {
        sal_fprintf(fp, "aaa-server timeout %u\n", p_db->timeout);
        has_config++;
    }

    if (0 != p_db->retries && GLB_AUTH_DEFAULT_RETRIES != p_db->retries) {
        sal_fprintf(fp, "aaa-server retransmit %u\n", p_db->retries);
        has_config++;
    }

    if (0 != p_db->deadtime && GLB_AUTH_DEFAULT_DEADTIME != p_db->deadtime) {
        sal_fprintf(fp, "aaa-server deadtime %u\n", p_db->deadtime);
        has_config++;
    }

    if ('\0' != p_db->secret[0] && strcmp(p_db->secret, "")) {
        sal_fprintf(fp, "aaa-server key %s\n", p_db->secret);
        has_config++;
    }

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }
#endif
/* liwh end */

    return PM_E_NONE;
}

static int32
tbl_auth_server_cfg_output(tbl_auth_server_t *p_auth, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    u_int32_t *pnCnt = (u_int32_t *)pargs->argv[1];
    char ipbuf[256];
    int32 is_inband = 0;
    char buf[IFNAME_SIZE] = "";
    int res = PM_E_NOT_EXIST;
    tbl_dot1x_radius_t          *p_db_dot1x_radius = NULL;
    tbl_dot1x_radius_t          dot1x_radius;
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    sal_memcpy(&dot1x_radius.key.addr.u, &p_auth->key.addr.u, sizeof(addr_u));
    dot1x_radius.key.addr.family = p_auth->key.addr.family;
    cdb_addr_val2str(ipbuf, 256, &p_auth->key.addr);
    dot1x_radius.key.sin_port = (uint16)p_auth->key.sin_port;
    
    
#if 1 /* address optimization by wangqj */
    if (AUTH_SERVER_TYPE_RADIUS == p_auth->key.type)
    {
        p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
        if (NULL == p_db_dot1x_radius)
        {
            return PM_E_NONE;
        }
        if (AF_INET == p_auth->key.addr.family)
        {
            res = inband_snat_get_if_or_ip(&p_auth->key.addr.u.prefix4, p_auth->key.sin_port, INBAND_IP_PROTO_UDP, buf);
            is_inband = ns_route_is_inband(p_auth->key.addr.u.prefix4, p_auth->key.sin_port, NS_ROUTE_UDP);
        }
        else
        {
            is_inband = ns_route_ipv6_is_inband(p_auth->key.addr.u.prefix6, p_auth->key.sin_port, NS_ROUTE_UDP);
        }
        if (PM_E_NONE == res)
        {
            sal_fprintf(fp, "radius-server host %s%s source-ip %s", (is_inband == 1) ? "" : 
                "mgmt-if ",
                ipbuf, buf);
        }
        else
        {
            sal_fprintf(fp, "radius-server host %s%s", (is_inband == 1) ? "" : 
                "mgmt-if ",
                ipbuf);
        }
    }
   
#else
    if (AUTH_SERVER_TYPE_RADIUS == p_auth->key.type)
    {
        sal_fprintf(fp, "radius-server host %s", ipbuf);
    }
    else
    {
        sal_fprintf(fp, "tacacs-server host %s", ipbuf);
    }
#endif
    /*modified by wus for bug52242 in 2019-10-28*/
    if (AUTH_SERVER_TYPE_RADIUS == p_auth->key.type 
            && GLB_AUTH_RADIUS_DEFAULT_PORT != p_auth->port) {
        sal_fprintf(fp, " auth-port %u", p_auth->port);
    } 

    if (AUTH_SERVER_TYPE_RADIUS == p_auth->key.type)
    {
        if (!GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_TIMEOUT))
        {
            if (0 != p_auth->timeout && GLB_AUTH_DEFAULT_TIMEOUT != p_auth->timeout) {
                sal_fprintf(fp, " timeout %u", p_auth->timeout);
            }
        }
        if (!GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_RETRANSMIT))
        {
            if (0 != p_auth->retries && GLB_AUTH_DEFAULT_RETRIES != p_auth->retries) {
                sal_fprintf(fp, " retransmit %u", p_auth->retries);
            }
        }
        if (!GLB_FLAG_ISSET(p_db_dot1x_radius->radius_config, AUTH_RADIUS_CONFIG_DEFAULT_KEY))
        {
            if ('\0' != p_auth->secret[0] && strcmp(p_auth->secret, "")) {
                sal_fprintf(fp, " key %s", p_auth->secret);
            } else if ('\0' != p_auth->enc_secret[0] && strcmp(p_auth->enc_secret, "")) {
                sal_fprintf(fp, " key 8 %s", p_auth->enc_secret);
            }
        }
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "!\n");
       
    }
    *pnCnt += 1;
    return PM_E_NONE;
}

static int32
tbl_auth_server_tacacs_cfg_output(tbl_auth_server_t *p_auth, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    u_int32_t *pnCnt = (u_int32_t *)pargs->argv[1];
    char ipbuf[256];
    int32 is_inband = 0;
    char buf[IFNAME_SIZE] = "";
    int res = PM_E_NOT_EXIST;
    cdb_addr_val2str(ipbuf, 256, &p_auth->key.addr);
    
#if 1 /* address optimization by wangqj */
    if (AUTH_SERVER_TYPE_TACPLUS == p_auth->key.type)
    {

        if (AF_INET == p_auth->key.addr.family)
        {
            res = inband_snat_get_if_or_ip(&p_auth->key.addr.u.prefix4, p_auth->key.sin_port, INBAND_IP_PROTO_TCP, buf);
            is_inband = ns_route_is_inband(p_auth->key.addr.u.prefix4, p_auth->key.sin_port, NS_ROUTE_TCP);
        }
        else
        {
            is_inband = ns_route_ipv6_is_inband(p_auth->key.addr.u.prefix6, p_auth->key.sin_port, NS_ROUTE_TCP);
        }
        
        if (PM_E_NONE == res)
        {
            sal_fprintf(fp, "tacacs-server host %s%s source-ip %s", (is_inband == 1) ? "" : 
                "mgmt-if ",
                ipbuf, buf);
        }
        else
        {
            sal_fprintf(fp, "tacacs-server host %s%s", (is_inband == 1) ? "" : 
                "mgmt-if ",
                ipbuf);
        }
    }
#else
    if (AUTH_SERVER_TYPE_RADIUS == p_auth->key.type)
    {
        sal_fprintf(fp, "radius-server host %s", ipbuf);
    }
    else
    {
        sal_fprintf(fp, "tacacs-server host %s", ipbuf);
    }
#endif
    if (AUTH_SERVER_TYPE_TACPLUS == p_auth->key.type 
            && GLB_AUTH_TACPLUS_DEFAULT_PORT != p_auth->port) {
        sal_fprintf(fp, " auth-port %u", p_auth->port);
    }

    if (AUTH_SERVER_TYPE_TACPLUS == p_auth->key.type)
    {
        if (0 != p_auth->retries && GLB_AUTH_DEFAULT_RETRIES != p_auth->retries) {
            sal_fprintf(fp, " retransmit %u", p_auth->retries);
        } 
    
        if (0 != p_auth->timeout && GLB_AUTH_DEFAULT_TIMEOUT != p_auth->timeout) {
            sal_fprintf(fp, " timeout %u", p_auth->timeout);
        }
            if ('\0' != p_auth->secret[0] && strcmp(p_auth->secret, "")) {
                sal_fprintf(fp, " key %s", p_auth->secret);
            } else if ('\0' != p_auth->enc_secret[0] && strcmp(p_auth->enc_secret, "")) {
                sal_fprintf(fp, " key 8 %s", p_auth->enc_secret);
            }
            sal_fprintf(fp, "\n");
            sal_fprintf(fp, "!\n");
            
    }
    
    
    
    
    
    *pnCnt += 1;
    return PM_E_NONE;
}

int32
ccs_cfg_build_auth_server(FILE * fp, cfg_cmd_para_t * para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;
    tbl_dot1x_global_t *p_db_glb = NULL;
    
    /* check exist */
    p_db_glb = tbl_dot1x_global_get_dot1x_global();

    if (!p_db_glb)
    {
        return PM_E_NONE;
    }
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
    
    tbl_auth_server_iterate((TBL_ITER_CB_FUNC)tbl_auth_server_cfg_output, &args);
    if (p_db_glb->radius_default_shared_secret_valid)
    {
        if (p_db_glb->radius_default_shared_secret[0] != '\0')
        {
            sal_fprintf(fp, "radius-server key %s\n", p_db_glb->radius_default_shared_secret);
        }
        else
        {
            sal_fprintf(fp, "radius-server key 8 %s\n", p_db_glb->radius_default_shared_enc_secret);            
        }
        
        sal_fprintf(fp, "!\n");
    }

    if (RADIUS_TIMEOUT_DEFAULT != p_db_glb->radius_default_timeout)
    {
        sal_fprintf(fp, "radius-server timeout %d\n", p_db_glb->radius_default_timeout);
        sal_fprintf(fp, "!\n");
    }

    if (RADIUS_RETRANSMIT_DEFAULT != p_db_glb->radius_default_retry)
    {
        sal_fprintf(fp, "radius-server retransmit %d\n", p_db_glb->radius_default_retry);
        sal_fprintf(fp, "!\n");
    }

    if (RADIUS_DEADTIME_DEFAULT != p_db_glb->radius_default_deadtime)
    {
        sal_fprintf(fp, "radius-server deadtime %d\n", p_db_glb->radius_default_deadtime);
        sal_fprintf(fp, "!\n");
    }
    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_tacacs_server(FILE * fp, cfg_cmd_para_t * para)
{
    uint32 has_config = FALSE;
    tbl_iter_args_t args;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;
    args.argv[1] = &has_config;
    tbl_auth_server_iterate((TBL_ITER_CB_FUNC)tbl_auth_server_tacacs_cfg_output, &args);

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int ccs_get_rsa_key(const char *pszEncodedKeyStr, const char *pszPasswd, 
    int bPrivate, RSA **ppRsa, char *pszErr, size_t err_bufsz)
{
    int ret = -1;
    char *pszKey = NULL;

    pszKey = malloc(M_RSA_KEYSTR_BUFSZ);
    if (NULL == pszKey) {
        snprintf(pszErr, err_bufsz, "out of memory");
        return -1;
    }
    if (sal_b64_pton(pszEncodedKeyStr, (unsigned char *)pszKey, M_RSA_KEYSTR_BUFSZ) == -1) {
        snprintf(pszErr, err_bufsz, "Decoding RSA key string failed");
        goto err_out;
    }
    if (validate_rsa_key_pem(pszPasswd, pszKey, sal_strlen(pszKey), 
            bPrivate, ppRsa, pszErr, err_bufsz) != 0) {
        snprintf(pszErr, err_bufsz, "Invalid PEM RSA key string");
        goto err_out;
    }

    ret = 0;
    
err_out:
    if (pszKey) {
        free(pszKey);
    }
    return ret;
}

int32
ccs_cfg_build_app_rsa(FILE * fp, cfg_cmd_para_t * para)
{
    tbl_rsa_master_t *p_master = tbl_rsa_get_master();
    tbl_rsa_t *p_db_rsa = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char szErr[256];
    char szPasswd[PASSWD_SIZE];
    char *pszPasswd = NULL;
    char *pszKey;
    int nValue = 0;
    RSA *pRsa = NULL;
    char buf[MAX_CMD_STR_LEN];
    uint32 has_config = FALSE;
    
    pszKey = malloc(M_RSA_KEYSTR_BUFSZ);
    if (NULL == pszKey) {
        sal_fprintf(stderr, "%% cdbctl_buildcfg_build_app_rsa: Out of memory\r\n");
        return -1;
    }

    CTCLIB_SLIST_LOOP(p_master->rsa_list, p_db_rsa, listnode)
    {
        if (NULL != pRsa) {
            RSA_free(pRsa);
            pRsa = NULL;
        }
        
        if('\0' == p_db_rsa->pem.buf[0])
        {
            continue;
        }
        
        sal_fprintf(fp, "rsa key %s\n", p_db_rsa->key.name);
        sal_fprintf(fp, "  key type %s\n", cdb_enum_val2str(key_type_strs, KEY_TYPE_MAX, p_db_rsa->type));
        sal_fprintf(fp, "  key format der\n");

        if (sal_strcmp(p_db_rsa->enc_passwd,"")) {
            sal_fprintf(fp, "  key password 8 %s\n", p_db_rsa->enc_passwd);
            szPasswd[0] = '\0';
            do_decrypt(M_DEFAULT_PW_STR, p_db_rsa->enc_passwd, 
                sal_strlen(p_db_rsa->enc_passwd), szPasswd, PASSWD_SIZE);
            pszPasswd = szPasswd;
        }

        if (sal_strcmp(p_db_rsa->passwd,"")) {
            sal_fprintf(fp, "  key password %s\n", p_db_rsa->passwd);
            pszPasswd = p_db_rsa->passwd;
        }

        if ('\0' != p_db_rsa->pem.buf[0]) {
            if (ccs_get_rsa_key(cdb_rsa_keystr_val2str(buf, MAX_CMD_STR_LEN, &p_db_rsa->pem), pszPasswd, 
                    !strcasecmp(cdb_enum_val2str(key_type_strs, KEY_TYPE_MAX, p_db_rsa->type), "private"),
                    &pRsa, szErr, 256) != 0) {
                /* error */
                sal_fprintf(stderr, "%% Get RSA key %s failed: %s\n",
                        p_db_rsa->key.name, szErr);
                continue;
            }
            /* get der string for key */
            if (rsa_convert_key_der2str(pRsa, 0, pszKey, M_RSA_KEYSTR_BUFSZ, &nValue) != 0)
            {
                /* error */
                sal_fprintf(stderr, "%% Can't get DER string for RSA key %s\n",
                        p_db_rsa->key.name);
                continue;
            }
            pszKey[nValue] = '\0';
            
            sal_fprintf(fp, "%s\n", pszKey);
            sal_fprintf(fp, "  key string end\n"); 
        } else {
            sal_fprintf(fp, "  exit\n");
        }
         has_config = TRUE;
    }

    if (has_config) {
        sal_fprintf(fp, "!\n");
    }

    if (NULL != pszKey) {
        free(pszKey);
    }

    if (NULL != pRsa) {
        RSA_free(pRsa);
    }
    return PM_E_NONE;
}
int32 _ccs_cfg_get_user_service_type(int8 service_type, char * user_service)
{
    if (NULL == user_service)
    {
        return PM_E_NONE;
    }
    switch (service_type)
    {
        case 0:
            sal_strcpy(user_service, "none");
            break;
        case 0x01:
            sal_strcpy(user_service, "telnet");
            break;
        case 0x02:
            sal_strcpy(user_service, "ssh");
            break;
        case 0x03:
            sal_strcpy(user_service, "telnet ssh");
            break;
        case 0x04:
            sal_strcpy(user_service, "web");
            break;
        case 0x05:
            sal_strcpy(user_service, "telnet web");
            break;
        case 0x06:
            sal_strcpy(user_service, "ssh web");
            break;
        case 0x07:
            sal_strcpy(user_service, "telnet ssh web");
            break;
        case 0x08:
            sal_strcpy(user_service, "rpc-api");
            break;
        case 0x09:
            sal_strcpy(user_service, "telnet rpc-api");
            break;
        case 0x0a:
            sal_strcpy(user_service, "ssh rpc-api");
            break;
        case 0x0b:
            sal_strcpy(user_service, "telnet ssh rpc-api");
            break;  
        case 0x0c:
            sal_strcpy(user_service, "web rpc-api");
            break;
        case 0x0d:
            sal_strcpy(user_service, "telnet web rpc-api");
            break;
        case 0x0e:
            sal_strcpy(user_service, "ssh web rpc-api");
            break;
        case 0x0f:
            sal_strcpy(user_service, "all");
            break;
        default:
            break;
            
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_user(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_user_master_t *p_master = tbl_user_get_master();
    tbl_user_t *p_db_user = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char service_type[MAX_CMD_STR_LEN];

    CTCLIB_SLIST_LOOP(p_master->user_list, p_db_user, listnode)
    {

        sal_fprintf(fp, "username %s", p_db_user->key.name);

        if (1 != p_db_user->privilege) {
            sal_fprintf(fp, " privilege %d", p_db_user->privilege);
        }
        
        if (sal_strcmp(p_db_user->enc_passwd,"")) {
            sal_fprintf(fp, " password 8 %s", p_db_user->enc_passwd);
        }
        
        if (sal_strcmp(p_db_user->passwd,"")) {
            sal_fprintf(fp, " password %s", p_db_user->passwd);
        }
        sal_fprintf(fp, "\n");
        
        if (sal_strcmp(p_db_user->rsakey,"")) {
            sal_fprintf(fp, "username %s assign rsa key %s\n",
                    p_db_user->key.name, p_db_user->rsakey);
        }
        has_config = TRUE;
    }
    if (has_config) {
        sal_fprintf(fp, "!\n");
    } 
    CTCLIB_SLIST_LOOP(p_master->user_list, p_db_user, listnode)
    {
        sal_memset(service_type, 0, sizeof(service_type));
        _ccs_cfg_get_user_service_type(p_db_user->service_type, service_type);
        if (0x0f != p_db_user->service_type) {
            sal_fprintf(fp, "username %s service-type %s\n",
                    p_db_user->key.name, service_type);
        }
        has_config = TRUE;
    }
    if (has_config) {
        sal_fprintf(fp, "!\n");
    } 

    /* bug53609 move service rpc cfg here for auth-mode cfg */
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    char rpcapi_str[CMD_BUF_64];
    char tmp_str[CMD_BUF_64];
    has_config = FALSE;

    if (p_db_glb->rpcapi_auth_mode != GLB_RPCAPI_AUTH_NONE)
    {
        sal_fprintf(fp, "service rpc-api auth-mode basic\n");
        has_config = TRUE;
    }
    if (!p_db_glb->rpcapi_disable)
    {
        sal_memset(rpcapi_str, 0, sizeof(rpcapi_str));
        sal_memset(tmp_str, 0, sizeof(tmp_str));
        if (p_db_glb->rpcapi_port != GLB_HTTP_PORT_DEF)
        {
            sal_snprintf(tmp_str, CMD_BUF_64, " port %u", p_db_glb->rpcapi_port);
            sal_strcat(rpcapi_str, tmp_str);
        }
        if (p_db_glb->rpcapi_ssl_en)
        {
            if (p_db_glb->rpcapi_ssl_port != GLB_HTTPS_PORT_DEF)
            {
                sal_snprintf(tmp_str, CMD_BUF_64, " ssl ssl-port %u", p_db_glb->rpcapi_ssl_port);
            }
            else
            {
                sal_snprintf(tmp_str, CMD_BUF_64, " ssl");
            }
            sal_strcat(rpcapi_str, tmp_str);
        }
        sal_fprintf(fp, "service rpc-api enable%s\n", rpcapi_str);
        has_config = TRUE;
    }

    if (has_config) {
        sal_fprintf(fp, "!\n");
    } 

    return PM_E_NONE;
}
int32
ccs_cfg_build_app_login_security(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_login_security_glb_t *p_db_login_security = NULL;
    uint32 has_config = FALSE;
    p_db_login_security = tbl_login_security_glb_get_login_security_glb();
    if (!p_db_login_security->enable)
    {
        sal_fprintf(fp, "no login-security enable\n");
        has_config = TRUE;
    }
    if (p_db_login_security->max_fail_num != APP_LOGIN_DEFAULT_MAX_FAIL_NUM || p_db_login_security->fail_period != APP_LOGIN_DEFAULT_FAIL_PERIOD)
    {
        sal_fprintf(fp, "login-security max-fail-num %d %d\n", p_db_login_security->max_fail_num, p_db_login_security->fail_period);
        has_config = TRUE;
    }
    if (p_db_login_security->lock_duration != APP_LOGIN_DEFAULT_LOCKED_DURATION)
    {
        sal_fprintf(fp, "login-security lock-duration %d\n", p_db_login_security->lock_duration);
        has_config = TRUE;
    }
    if (has_config) {
        sal_fprintf(fp, "!\n");
    } 
    return PM_E_NONE;
}
int32
ccs_cfg_build_app_snmp_cfg(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_cfg_t *p_db_cfg = tbl_snmp_cfg_get_snmp_cfg();
    uint32 has_config = FALSE;
    char str[10]={0};
    if (p_db_cfg->server_enable) {
        sal_fprintf(fp, "snmp-server enable\n");
        has_config = TRUE;
    }

    if (sal_strcmp(p_db_cfg->contact.info, "")){
        sal_fprintf(fp, "snmp-server system-contact %s\n", 
                    p_db_cfg->contact.info);
        has_config = TRUE;
    }
    if (sal_strcmp(p_db_cfg->location.info, "")) {
        sal_fprintf(fp, "snmp-server system-location %s\n", 
                    p_db_cfg->location.info);
        has_config = TRUE;
    }

    if (sal_strcmp(p_db_cfg->engineid,"")
        && 0 != sal_strcmp(p_db_cfg->engineid, "30383038303830383038")) {
        sal_fprintf(fp, "snmp-server engineID %s\n", p_db_cfg->engineid);
        has_config = TRUE;
    }
    
    sal_strcpy(str,cdb_enum_val2str(snmp_version_strs, SNMP_VERSION_MAX, p_db_cfg->version));
    if (strcasecmp(str, "all")) {
        sal_fprintf(fp, "snmp-server version %s\n", str);
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_COLDSTART)) {
        sal_fprintf(fp, "snmp-server trap enable coldstart\n");
        has_config = TRUE;
    }
 
    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_WARMSTART)) {
        sal_fprintf(fp,"snmp-server trap enable warmstart\n");
        has_config = TRUE;
    }

    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_LINKDOWN)) {
        sal_fprintf(fp, "snmp-server trap enable linkdown\n");
        has_config = TRUE;
    }
    
    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_LINKUP)) {
        sal_fprintf(fp, "snmp-server trap enable linkup\n");
        has_config = TRUE;
    }
    
    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_SYSTEM)) {
        sal_fprintf(fp, "snmp-server trap enable system\n");
        has_config = TRUE;
    }

    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_VRRP)) {
        sal_fprintf(fp, "snmp-server trap enable vrrp\n");
        has_config = TRUE;
    }

    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_PTP_MASTER_CHANGE)) {
        sal_fprintf(fp, "snmp-server trap enable ptp master-change\n");
        has_config = TRUE;
    }

    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_PTP_STATE_CHANGE)) {
        sal_fprintf(fp, "snmp-server trap enable ptp state-change\n");
        has_config = TRUE;
    }

    if (GLB_FLAG_ISSET(p_db_cfg->trap_enable, GLB_TRAP_TYPE_PTP_ACCURACY_CHANGE)) {
        sal_fprintf(fp, "snmp-server trap enable ptp accuracy-change\n");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}    

int32
ccs_cfg_build_app_snmp_view(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_view_master_t *p_master = tbl_snmp_view_get_master();
    tbl_snmp_view_t *p_db_view = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char str[10]={0};
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_view_list, p_db_view, listnode)
    {
         sal_strcpy(str,cdb_enum_val2str(snmp_view_class_strs, SNMP_VIEW_CLASS_MAX, p_db_view->type));
         sal_fprintf(fp, "snmp-server view %s %s %s",
         p_db_view->key.view.name,
         str,
         p_db_view->key.view.subtree);
         if(sal_strcmp(p_db_view->mask, ""))
         {        
             sal_fprintf(fp, " mask %s\n", p_db_view->mask);
         } 
         else 
         {
             sal_fprintf(fp, "\n");
         }

        has_config = TRUE;
    }  
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_community(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_community_master_t *p_master = tbl_snmp_community_get_master();
    tbl_snmp_community_t *p_db_comm = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_community_list, p_db_comm, listnode)
    {
        sal_fprintf(fp, "snmp-server community %s %s",
            p_db_comm->key.name,
            cdb_enum_val2str(snmp_community_strs, SNMP_COMMUNITY_MAX, p_db_comm->type));
        if (sal_strcmp(p_db_comm->view, "")) 
        {
            /* with view */
            sal_fprintf(fp, " view %s\n", p_db_comm->view);
        }
        else 
        {
            sal_fprintf(fp, "\n");
        }
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_context(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_context_master_t *p_master = tbl_snmp_context_get_master();
    tbl_snmp_context_t *p_db_context = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_context_list, p_db_context, listnode)
    {
        sal_fprintf(fp, "snmp-server context %s\n",
            p_db_context->key.context_name);
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_usm_user(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_usm_user_master_t *p_master = tbl_snmp_usm_user_get_master();
    tbl_snmp_usm_user_t *p_db_user = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_usm_user_list, p_db_user, listnode)
    {
        sal_fprintf(fp, "snmp-server usm-user %s", p_db_user->key.usm_user_name);
        if (sal_strcmp(p_db_user->remo_engid, "")) 
        {
            /* with remote enginid */
            sal_fprintf(fp, " remote %s", p_db_user->remo_engid);
        }
        if (0 != p_db_user->auth_type)
        {
            sal_fprintf(fp, " authentication %s", 
                (SNMP_AUTHENTICATION_MD5 == p_db_user->auth_type)? "md5":"sha");
            
            if ('\0' != p_db_user->auth_pwd[0]) 
            {
                sal_fprintf(fp, " %s", p_db_user->auth_pwd);
            }
            else if ('\0' == p_db_user->auth_pwd[0] && '\0' != p_db_user->enc_auth_pwd[0])
            {
                sal_fprintf(fp, " 8 %s", p_db_user->enc_auth_pwd);
            }

            if (p_db_user->priv_type)
            {
                sal_fprintf(fp, " privacy %s", 
                    (SNMP_PRIVACY_AES == p_db_user->priv_type)? "aes":"des");
                if ('\0' != p_db_user->priv_pwd[0]) 
                {
                    sal_fprintf(fp, " %s", p_db_user->priv_pwd);
                }
                else if ('\0' == p_db_user->priv_pwd[0] && '\0' != p_db_user->enc_priv_pwd[0])
                {
                    sal_fprintf(fp, " 8 %s", p_db_user->enc_priv_pwd);
                }
            }
        }
        sal_fprintf(fp, "\n");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_group_usm(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_group_usm_master_t *p_master = tbl_snmp_group_usm_get_master();
    tbl_snmp_group_usm_t *p_db_group = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_group_usm_list, p_db_group, listnode)
    {
        sal_fprintf(fp, "snmp-server group %s user %s security-model usm\n",
            p_db_group->key.group_usm.grp_name, p_db_group->key.group_usm.security_name);
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_access_usm(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_access_usm_master_t *p_master = tbl_snmp_access_usm_get_master();
    tbl_snmp_access_usm_t *p_db_access = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_access_usm_list, p_db_access, listnode)
    {
        sal_fprintf(fp, "snmp-server access %s security-model usm",
            p_db_access->key.usm_access_name);

        if (SNMP_LEVEL_NOAUTH == p_db_access->usm_level)
        {
            sal_fprintf(fp, " noauth");
        }
        else if (SNMP_LEVEL_AUTHNOPRIV == p_db_access->usm_level)
        {
            sal_fprintf(fp, " auth");
        }
        else if (SNMP_LEVEL_AUTHPRIV == p_db_access->usm_level)
        {
            sal_fprintf(fp, " priv");
        }

        if ('\0' != p_db_access->read_view[0] && 0 != sal_strcmp(p_db_access->read_view, "_all_"))
        {
            sal_fprintf(fp, " read %s", p_db_access->read_view);
        }

        if ('\0' != p_db_access->write_view[0] && 0 != sal_strcmp(p_db_access->write_view, "none"))
        {
            sal_fprintf(fp, " write %s", p_db_access->write_view);
        }

        if ('\0' != p_db_access->notify_view[0] && 0 != sal_strcmp(p_db_access->notify_view, "none"))
        {
            sal_fprintf(fp, " notify %s", p_db_access->notify_view);
        }

        if ('\0' != p_db_access->context_name[0] && 0 != sal_strcmp(p_db_access->context_name, "\"\""))
        {
            sal_fprintf(fp, " context %s", p_db_access->context_name);
            if (p_db_access->isprefix)
            {
                sal_fprintf(fp, " prefix");
            }
        }
        sal_fprintf(fp, "\n");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_notify(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_notify_master_t *p_master = tbl_snmp_notify_get_master();
    tbl_snmp_notify_t *p_db_notify = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_notify_list, p_db_notify, listnode)
    {
        sal_fprintf(fp, "snmp-server notify %s tag %s %s\n",
            p_db_notify->key.notify_name, p_db_notify->tag_name,
            p_db_notify->notify_type ? "inform":"");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_target_addr(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_target_addr_master_t *p_master = tbl_snmp_target_addr_get_master();
    tbl_snmp_target_addr_t *p_db_tarAddr = NULL;
    ctclib_slistnode_t *listnode = NULL;
    char szIpBuf[256] = {0};
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_target_addr_list, p_db_tarAddr, listnode)
    {
        sal_fprintf(fp, "snmp-server target-address %s param %s",
            p_db_tarAddr->key.taraddr_name, p_db_tarAddr->para_name);
        if (p_db_tarAddr->mgmt_if)
        {
            sal_fprintf(fp, " mgmt-if");
        }
        sal_fprintf(fp, " %s", cdb_addr_val2str(szIpBuf, 256, &p_db_tarAddr->ip_addr));
        
        if (SNMP_DEFAULT_UDPPORT != p_db_tarAddr->upd_port)
        {
            sal_fprintf(fp, " udpport %d",p_db_tarAddr->upd_port);
        }

        if (SNMP_DEFAULT_TIMEOUT_CTC != p_db_tarAddr->timeout)
        {
            sal_fprintf(fp, " timeout %d",p_db_tarAddr->timeout);
        }

        if (SNMP_DEFAULT_RETRIES_CTC != p_db_tarAddr->retries)
        {
            sal_fprintf(fp, " retries %d",p_db_tarAddr->retries);
        }
        sal_fprintf(fp, " taglist %s\n", p_db_tarAddr->tag_list);
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_target_params(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_target_params_master_t *p_master = tbl_snmp_target_params_get_master();
    tbl_snmp_target_params_t *p_db_tarParams = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_target_params_list, p_db_tarParams, listnode)
    {
        sal_fprintf(fp, "snmp-server target-params %s user %s security-model v3 message-processing v3",
            p_db_tarParams->key.params_name, p_db_tarParams->user_name);
        if (SNMP_LEVEL_NOAUTH == p_db_tarParams->usm_level)
        {
            sal_fprintf(fp, " noauth\n");
        }
        else if (SNMP_LEVEL_AUTHNOPRIV == p_db_tarParams->usm_level)
        {
            sal_fprintf(fp, " auth\n");
        }
        else if (SNMP_LEVEL_AUTHPRIV == p_db_tarParams->usm_level)
        {
            sal_fprintf(fp, " priv\n");
        }
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_trap(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_trap_master_t *p_master = tbl_snmp_trap_get_master();
    tbl_snmp_trap_t *p_db_trap = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char szIpBuf[47] = {0};
    char buf[IFNAME_SIZE] = "";
    int res;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_trap_list, p_db_trap, listnode)
    {
#if 1 /* address optimization by wangqj */
        res = inband_snat_get_if_or_ip(&p_db_trap->key.trap.ip_addr.u.prefix4, p_db_trap->key.trap.udpport, INBAND_IP_PROTO_UDP, buf);

        if (p_db_trap->key.trap.udpport != 162)
        {
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "snmp-server trap target-address %s%s source-ip %s community %s udpport %d\n",
                   p_db_trap->is_inband ? "" : 
                   "mgmt-if ",
                   cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
                   buf,
                   p_db_trap->key.trap.community, p_db_trap->key.trap.udpport);
            }
            else
            {
                sal_fprintf(fp, "snmp-server trap target-address %s%s community %s udpport %d\n",
                   p_db_trap->is_inband ? "" : 
                    "mgmt-if ",
                   cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
                   p_db_trap->key.trap.community, p_db_trap->key.trap.udpport);
            }
        }        
        else
        {                    
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "snmp-server trap target-address %s%s source-ip %s community %s\n",
                  p_db_trap->is_inband ? "" : 
                  "mgmt-if ",
                  cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
                  buf,
                  p_db_trap->key.trap.community);
            }
            else
            {
                sal_fprintf(fp, "snmp-server trap target-address %s%s community %s\n",
                  p_db_trap->is_inband ? "" : 
                  "mgmt-if ",
                  cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
                  p_db_trap->key.trap.community);
            }
        }
#else
        if (p_db_trap->key.trap.udpport != 162)
        {
             sal_fprintf(fp, "snmp-server trap target-address %s community %s udpport %d\n",
                cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
                p_db_trap->key.trap.community, p_db_trap->key.trap.udpport);
        }        
        else
        {                    
             sal_fprintf(fp, "snmp-server trap target-address %s community %s\n",
               cdb_addr_val2str(szIpBuf, 47, &p_db_trap->key.trap.ip_addr), 
               p_db_trap->key.trap.community);
        }
#endif
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_snmp_inform(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_snmp_inform_master_t *p_master = tbl_snmp_inform_get_master();
    tbl_snmp_inform_t *p_db_inform = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char szIpBuf[47] = {0};
    char buf[IFNAME_SIZE] = "";
    int res;
         
    CTCLIB_SLIST_LOOP(p_master->snmp_inform_list, p_db_inform, listnode)
    {
#if 1 /* address optimization by wangqj */
        res = inband_snat_get_if_or_ip(&p_db_inform->key.inform.ip_addr.u.prefix4, p_db_inform->key.inform.udpport, INBAND_IP_PROTO_UDP, buf);

        if (p_db_inform->key.inform.udpport != 162)
        {
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "snmp-server inform target-address %s%s source-ip %s community %s udpport %d\n",
                   p_db_inform->is_inband ? "" : "mgmt-if ",
                   cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
                   buf,
                   p_db_inform->key.inform.community, p_db_inform->key.inform.udpport);
            }
            else
            {
                sal_fprintf(fp, "snmp-server inform target-address %s%s community %s udpport %d\n",
                   p_db_inform->is_inband ? "" : "mgmt-if ",
                   cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
                   p_db_inform->key.inform.community, p_db_inform->key.inform.udpport);
            }
        }        
        else
        {                    
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "snmp-server inform target-address %s%s source-ip %s community %s\n",
                  p_db_inform->is_inband ? "" : "mgmt-if ",
                  cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
                  buf,
                  p_db_inform->key.inform.community);
            }
            else
            {
                sal_fprintf(fp, "snmp-server inform target-address %s%s community %s\n",
                  p_db_inform->is_inband ? "" : "mgmt-if ",
                  cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
                  p_db_inform->key.inform.community);
            }
        }
#else
        if (p_db_inform->key.inform.udpport != 162)
        {
             sal_fprintf(fp, "snmp-server inform target-address %s community %s udpport %d\n",
                cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
                p_db_inform->key.inform.community, p_db_inform->key.inform.udpport);
        }        
        else
        {                    
             sal_fprintf(fp, "snmp-server inform target-address %s community %s\n",
               cdb_addr_val2str(szIpBuf, 47, &p_db_inform->key.inform.ip_addr), 
               p_db_inform->key.inform.community);
        }
#endif
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_rmon_event(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_rmon_event_master_t *p_master = tbl_rmon_event_get_master();
    tbl_rmon_event_t *p_db_event = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->rmon_event_list, p_db_event, listnode)
    {
        sal_fprintf(fp, "rmon event %d", p_db_event->key.index);
        if (p_db_event->type == RMON_EVENT_LOG || p_db_event->type == RMON_EVENT_LOG_TRAP)
        {
            sal_fprintf(fp, " log");
        }
        if (p_db_event->type == RMON_EVENT_TRAP || p_db_event->type == RMON_EVENT_LOG_TRAP)
        {
            sal_fprintf(fp, " trap %s", p_db_event->community);
        }
        if (sal_strcmp(p_db_event->desc, RMON_DEFAULT_NAME))
        {
            sal_fprintf(fp, " description %s", p_db_event->desc);
        }
        if (sal_strcmp(p_db_event->owner, RMON_DEFAULT_NAME))
        {
            sal_fprintf(fp, " owner %s", p_db_event->owner);
        }
        sal_fprintf(fp, "\n");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_rmon_alarm(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_rmon_alarm_master_t *p_master = tbl_rmon_alarm_get_master();
    tbl_rmon_alarm_t *p_db_alarm = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
         
    CTCLIB_SLIST_LOOP(p_master->rmon_alarm_list, p_db_alarm, listnode)
    {
        sal_fprintf(fp, "rmon alarm %d %s interval %d", p_db_alarm->key.index, p_db_alarm->variable_word, p_db_alarm->interval);
        
        if (p_db_alarm->sample_type == RMON_ALARM_ABS)
        {
            sal_fprintf(fp, " absolute");
        }
        else
        {
            sal_fprintf(fp, " delta");
        }

        sal_fprintf(fp, " rising-threshold %d", p_db_alarm->rising_threshold);
        if (p_db_alarm->rising_event_index)
        {
            sal_fprintf(fp, " rising-event %d", p_db_alarm->rising_event_index);
        }

        sal_fprintf(fp, " falling-threshold %d", p_db_alarm->falling_threshold);
        if (p_db_alarm->falling_event_index)
        {
            sal_fprintf(fp, " falling-event %d", p_db_alarm->falling_event_index);
        }

        if (sal_strcmp(p_db_alarm->owner, RMON_DEFAULT_NAME))
        {
            sal_fprintf(fp, " owner %s", p_db_alarm->owner);
        }
        sal_fprintf(fp, "\n");
        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_static_dns(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_static_dns_master_t *p_master = tbl_static_dns_get_master();
    tbl_static_dns_t *p_db_dns = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char szIpBuf[47] = {0};

    CTCLIB_SLIST_LOOP(p_master->static_list, p_db_dns, listnode)
    {
        cdb_addr_val2str(szIpBuf, 47, &p_db_dns->ip_addr);
        sal_fprintf(fp, "ip host %s %s\n", p_db_dns->key.name, szIpBuf);
        has_config = TRUE;
    }  
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_dynamic_dns_domain(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_dynamic_dns_domain_master_t *p_master = tbl_dynamic_dns_domain_get_master();
    tbl_dynamic_dns_domain_t *p_db_dns = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;

    CTCLIB_SLIST_LOOP(p_master->dynamic_domain_list, p_db_dns, listnode)
    {
        sal_fprintf(fp, "dns domain %s\n",p_db_dns->key.name);
        has_config = TRUE;
    }  
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_dynamic_dns_server(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_dynamic_dns_server_master_t *p_master = tbl_dynamic_dns_server_get_master();
    tbl_dynamic_dns_server_t *p_db_dns = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char szIpBuf[47] = {0};       

    CTCLIB_SLIST_LOOP(p_master->dynamic_server_list, p_db_dns, listnode)
    {
        cdb_addr_val2str(szIpBuf, 47, &p_db_dns->key.ip_addr);
        sal_fprintf(fp, "dns server %s\n",szIpBuf);
        has_config = TRUE;
    }  
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32
ccs_cfg_build_app_authen(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_authen_master_t *p_master = tbl_authen_get_master();
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_authen_t *p_db_auth = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    char tmp[LINE_AUTH_METHOD_LINE_SIZE];
    char *p;

    if (!p_db_glb->aaa_new_model)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->authen_list, p_db_auth, listnode)
    {
        snprintf(tmp, LINE_AUTH_METHOD_LINE_SIZE, "%s", p_db_auth->methods);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }

        sal_fprintf(fp, "aaa authentication login %s %s\n", 
                    p_db_auth->key.name, tmp);
        has_config = TRUE;
    } 
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_author(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_author_master_t  *p_master = tbl_author_get_master();
    tbl_sys_global_t     *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_author_t         *p_db_author = NULL;
    ctclib_slistnode_t   *listnode = NULL;
    uint32               has_config = FALSE;
    char                 tmp[LINE_AUTH_METHOD_LINE_SIZE];
    char                 *p;

    if (!p_db_glb->aaa_new_model)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->author_list, p_db_author, listnode)
    {
        snprintf(tmp, LINE_AUTH_METHOD_LINE_SIZE, "%s", p_db_author->methods);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }

        sal_fprintf(fp, "aaa authorization exec %s %s\n", 
                    p_db_author->key.name, tmp);
        has_config = TRUE;
    } 
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_account(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_account_master_t  *p_master = tbl_account_get_master();
    tbl_sys_global_t      *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_account_t         *p_db_account = NULL;
    ctclib_slistnode_t    *listnode = NULL;
    uint32                has_config = FALSE;
    char                  tmp[LINE_AUTH_METHOD_LINE_SIZE];
    char                  *p;

    if (!p_db_glb->aaa_new_model)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->account_list, p_db_account, listnode)
    {
        sal_fprintf(fp, "aaa accounting exec %s ", 
                    p_db_account->key.name);

        if (p_db_account->start_stop)
        {
            sal_fprintf(fp, "start-stop ");    
        }
        else if (p_db_account->stop_only)
        {
            sal_fprintf(fp, "stop-only "); 
        }
        
        snprintf(tmp, LINE_AUTH_METHOD_LINE_SIZE, "%s", p_db_account->methods);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        
        sal_fprintf(fp, "%s\n", tmp);
        has_config = TRUE;
    } 
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_app_accountcmd(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_accountcmd_master_t  *p_master = tbl_accountcmd_get_master();
    tbl_sys_global_t         *p_db_glb = tbl_sys_global_get_sys_global();
    tbl_accountcmd_t         *p_db_accountcmd = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    uint32                   has_config = FALSE;
    char                     tmp[LINE_AUTH_METHOD_LINE_SIZE];
    char                     *p;

    if (!p_db_glb->aaa_new_model)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->accountcmd_list, p_db_accountcmd, listnode)
    {
        sal_fprintf(fp, "aaa accounting commands %s ", 
                    p_db_accountcmd->key.name);
        
        snprintf(tmp, LINE_AUTH_METHOD_LINE_SIZE, "%s", p_db_accountcmd->methods);
        while (NULL != (p = strchr(tmp, ','))) {
            *p = ' ';
        }
        
        sal_fprintf(fp, "%s\n", tmp);
        has_config = TRUE;
    } 
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}


int32 
_ccs_cfg_build_app_vty(FILE* fp, tbl_vty_t *tbl_vty, uint32 start, uint32 end)
{
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();

    if (start == end)
    {
        sal_fprintf(fp, "line vty %u\n", start);
    }
    else
    {
        sal_fprintf(fp, "line vty %u %u\n", start, end);
    }

    if (!(M_EXEC_TIMEOUT_MIN_DEFAULT == tbl_vty->timeout_min && 0 == tbl_vty->timeout_sec))
    {
        sal_fprintf(fp, " exec-timeout %u %u\n", tbl_vty->timeout_min, tbl_vty->timeout_sec);
    }

    if (1 != tbl_vty->privilege) 
    {
        sal_fprintf(fp, " privilege level %u\n", tbl_vty->privilege);
    }

    if (0 != sal_strlen(tbl_vty->passwd)) 
    {
        sal_fprintf(fp, " line-password %s\n", tbl_vty->passwd);
    } 
    else 
    {
        if (0 != sal_strlen(tbl_vty->enc_passwd))
        {
            sal_fprintf(fp, " line-password 8 %s\n", tbl_vty->enc_passwd);
        }
        else 
        {
            sal_fprintf(fp, " "CMD_NO_STR" line-password\n");
        }
    }

    if (0 != sal_strlen(tbl_vty->ipv4_acl)) 
    {
        sal_fprintf(fp, " ip access-class %s in\n", tbl_vty->ipv4_acl);
    } 
    if (0 != sal_strlen(tbl_vty->ipv6_acl)) 
    {
        sal_fprintf(fp, " ipv6 access-class %s in\n", tbl_vty->ipv6_acl);
    } 
    
    if (0 != sal_strlen(tbl_vty->auth_method) && 0 != sal_strcasecmp(tbl_vty->auth_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, " login authentication %s\n", tbl_vty->auth_method);
    }
    else if (0 != sal_strlen(tbl_vty->auth_method) && 0 == sal_strcasecmp(tbl_vty->auth_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        /*do nothing*/
    }
    else if (LOGIN_METHOD_NOLOGIN == tbl_vty->login)
    {
        sal_fprintf(fp, " "CMD_NO_STR" login\n");
    }
    else if (LOGIN_METHOD_LOCAL == tbl_vty->login)
    {
        sal_fprintf(fp, " login local\n");
    }
    else if (LOGIN_METHOD_LOGIN == tbl_vty->login)
    {
        sal_fprintf(fp, " login\n");
    }

    if (0 != sal_strlen(tbl_vty->author_method) && 0 != sal_strcasecmp(tbl_vty->author_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, " authorization exec %s\n", tbl_vty->author_method);
    }

    if (0 != sal_strlen(tbl_vty->account_method) && 0 != sal_strcasecmp(tbl_vty->account_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, " accounting exec %s\n", tbl_vty->account_method);
    }   

    if (0 != sal_strlen(tbl_vty->accountcmd_method) && 0 != sal_strcasecmp(tbl_vty->accountcmd_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, " accounting commands %s\n", tbl_vty->accountcmd_method);
    }   

    return PM_E_NONE;
}

int32
_ccs_cfg_is_app_vty_same_config(tbl_vty_t *a, tbl_vty_t *b)
{
    if (a->timeout_min != b->timeout_min)
    {
        return FALSE;
    }
    if (a->timeout_sec != b->timeout_sec)
    {
        return FALSE;
    }
    if (a->login != b->login)
    {
        return FALSE;
    }
    if (a->privilege != b->privilege)
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->protocol, b->protocol))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->enc_passwd, b->enc_passwd))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->passwd, b->passwd))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->auth_method, b->auth_method))
    {
        return FALSE;
    }
    /*added by yejl to fix bug41201, 2016-12-15*/
    if (0 != sal_strcmp(a->author_method, b->author_method))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->account_method, b->account_method))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->accountcmd_method, b->accountcmd_method))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->ipv4_acl, b->ipv4_acl))
    {
        return FALSE;
    }
    if (0 != sal_strcmp(a->ipv6_acl, b->ipv6_acl))
    {
        return FALSE;
    }
    /*end by yejl*/
    return TRUE;
}

int32
ccs_cfg_build_app_vty(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_db_sysg = tbl_sys_global_get_sys_global();

    tbl_vty_master_t *vty_master = NULL;
    uint32 start = 0;
    uint32 end = 0;
    uint32 i = 0;

    if (p_db_sysg && GLB_MAX_VTY_NUM != p_db_sysg->max_vty) {
        sal_fprintf(fp, "line vty maximum %d\n", p_db_sysg->max_vty);
    }

    vty_master = tbl_vty_get_master();
    for (i = 1; i < GLB_MAX_VTY_NUM; i++)
    {
        if (_ccs_cfg_is_app_vty_same_config(vty_master->vty_array[start], vty_master->vty_array[i]))
        {
            continue;
        }
        end = i - 1;
        _ccs_cfg_build_app_vty(fp, vty_master->vty_array[start], start, end);
        start = i;
    }

    end = i - 1;
    _ccs_cfg_build_app_vty(fp, vty_master->vty_array[start], start, end);

    return PM_E_NONE;
}

int32
ccs_cfg_build_app_console(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_console_t *app_console = NULL;
    tbl_sys_global_t *p_db_glb = tbl_sys_global_get_sys_global();
    
    app_console = tbl_console_get_console();
   
    sal_fprintf(fp, "line console 0\n");

    if (GLB_CONSOLE_RATE_DEFAULT != app_console->baudrate)
    {
        sal_fprintf(fp, " speed %d\n", app_console->baudrate);
    }

    if (PARITY_ODD == app_console->parity)
    {
        sal_fprintf(fp, " parity odd\n");
    }
    else if (PARITY_EVEN == app_console->parity)
    {
        sal_fprintf(fp, " parity even\n");
    }

    if (8 != app_console->databits)
    {
        sal_fprintf(fp, " databits %d\n", app_console->databits);
    }

    if (2 == app_console->stopbits)
    {
        sal_fprintf(fp, " stopbits 2\n");
    }

    if (!(M_EXEC_TIMEOUT_MIN_DEFAULT == app_console->timeout_min && (0 == app_console->timeout_sec)))
    {
        sal_fprintf(fp, " exec-timeout %u %u\n", app_console->timeout_min, app_console->timeout_sec);
    }
        
    if (1 != app_console->privilege)
    {
        sal_fprintf(fp, " privilege level %u\n", app_console->privilege);
    }

    if (0 != sal_strlen(app_console->passwd))
    {
        sal_fprintf(fp, " line-password %s\n", app_console->passwd);
    }
    else
    {
        if (0 != sal_strlen(app_console->enc_passwd))
        {
            sal_fprintf(fp, " line-password 8 %s\n", app_console->enc_passwd);
        }
        else
        {
            sal_fprintf(fp, " "CMD_NO_STR" line-password\n");
        }
    }

    if (0 != sal_strlen(app_console->auth_method) && 0 != sal_strcasecmp(app_console->auth_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        sal_fprintf(fp, " login authentication %s\n", app_console->auth_method);
    }
    else if (0 != sal_strlen(app_console->auth_method) && 0 == sal_strcasecmp(app_console->auth_method, "default")
        && TRUE == p_db_glb->aaa_new_model)
    {
        /*do nothing*/
    }
    else if (LOGIN_METHOD_NOLOGIN == app_console->login)
    {
        sal_fprintf(fp, " "CMD_NO_STR" login\n");
    }
    else if (LOGIN_METHOD_LOCAL == app_console->login)
    {
        sal_fprintf(fp, " login local\n");
    }
    else if (LOGIN_METHOD_LOGIN == app_console->login)
    {
        sal_fprintf(fp, " login\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_app_ntp_cfg(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_ntp_cfg_t *tbl_ntp_cfg = tbl_ntp_cfg_get_ntp_cfg();
    uint32 has_config = FALSE;


    if (tbl_ntp_cfg->brefclock && tbl_ntp_cfg->refclock_stratum != 0) 
    {
        sal_fprintf(fp, "ntp refclock stratum %d\n", tbl_ntp_cfg->refclock_stratum);
        has_config = TRUE;
    }

    if (tbl_ntp_cfg->bcastDelay != 0) 
    {
        sal_fprintf(fp, "ntp broadcastdelay %d\n", tbl_ntp_cfg->bcastDelay);
        has_config = TRUE;
    }

    if (tbl_ntp_cfg->minimumDistance != 0) 
    {
        sal_fprintf(fp, "ntp minimum-distance %d\n", tbl_ntp_cfg->minimumDistance);
        has_config = TRUE;
    }

    if (tbl_ntp_cfg->authentication) 
    {
        sal_fprintf(fp, "ntp authentication enable\n");
        has_config = TRUE;
    }

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}    

int32
ccs_cfg_build_app_ntp_key(FILE* fp, cfg_cmd_para_t *para)
{
    ctclib_slistnode_t * listnode = NULL; 
    tbl_ntp_key_t *key = NULL;
    tbl_ntp_key_master_t *key_master = tbl_ntp_key_get_master();
    uint32 has_config = FALSE;

    CTCLIB_SLIST_LOOP(key_master->ntp_key_list, key, listnode)
    {
        sal_fprintf(fp, "ntp key %d %s\n", key->key.keyid, key->value);
        if(key->trusted)
        {
            sal_fprintf(fp, "ntp trustedkey %d\n", key->key.keyid);
        }
        has_config = TRUE;
    }

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}   

int32
ccs_cfg_build_app_ntp_server(FILE* fp, cfg_cmd_para_t *para)
{
    ctclib_slistnode_t * listnode = NULL; 
    tbl_ntp_server_t *server = NULL;
    tbl_ntp_server_master_t *server_master = tbl_ntp_server_get_master();
    uint32 has_config = FALSE;
    addr_ipv4_t ip;
    char buf[IFNAME_SIZE] = "";
    int res;


    CTCLIB_SLIST_LOOP(server_master->ntp_server_list, server, listnode)
    {
        if (server->type == M_NTP_TYPE_UCAST_SERVER)
        {
            /* modify by huxd for bugid 54133. 2019-10-26 */
            sal_memset(buf, 0, IFNAME_SIZE);
            sal_memset(&ip, 0, sizeof(addr_ipv4_t));
            /* end huxd */
            sal_inet_pton(AF_INET, server->key.host, &ip);
            res = inband_snat_get_if_or_ip(&ip, 123, INBAND_IP_PROTO_UDP, buf);

#if 1 /* address optimization by wangqj */
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "ntp server %s%s source-ip %s",server->is_inband ? "" : 
                "mgmt-if ",
                    server->key.host, buf);
            }
            else
            {
                sal_fprintf(fp, "ntp server %s%s",server->is_inband ? "" : 
                "mgmt-if ",
                    server->key.host);
            }
#else
            sal_fprintf(fp, "ntp server %s", server->key.host);
#endif
            if(server->keyid != 0)
            {
                sal_fprintf(fp, " key %d ", server->keyid);
            }
            if(server->version != 0)
            {
                sal_fprintf(fp, " version %d ", server->version);
            }
            if(server->prefer != 0)
            {
                sal_fprintf(fp, " prefer");
            }
            sal_fprintf(fp, "\n");
            
            has_config = TRUE;
        }
    }

    CTCLIB_SLIST_LOOP(server_master->ntp_server_list, server, listnode)
    {
        if (server->type == M_NTP_TYPE_UCAST_CLIENT)
        {
            /* modify by huxd for bugid 54133. 2019-10-26 */
            sal_memset(buf, 0, IFNAME_SIZE);
            sal_memset(&ip, 0, sizeof(addr_ipv4_t));
            /* end huxd */
            sal_inet_pton(AF_INET, server->key.host, &ip);
            res = inband_snat_get_if_or_ip(&ip, 123, INBAND_IP_PROTO_UDP, buf);

#if 1 /* address optimization by wangqj */
            if (PM_E_NONE == res)
            {
                sal_fprintf(fp, "ntp peer %s%s source-ip %s",server->is_inband ? "" : 
                "mgmt-if ",
                    server->key.host, buf);
            }
            else
            {
                sal_fprintf(fp, "ntp peer %s%s",server->is_inband ? "" : 
                "mgmt-if ",
                    server->key.host);
            }
#else
            sal_fprintf(fp, "ntp peer %s", server->key.host);
#endif
            if(server->keyid != 0)
            {
                sal_fprintf(fp, " key %d", server->keyid);
            }
            if(server->version != 0)
            {
                sal_fprintf(fp, " version %d", server->version);
            }
            if(server->prefer != 0)
            {
                sal_fprintf(fp, " prefer");
            }
            sal_fprintf(fp, "\n");

            has_config = TRUE;
        }
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}   

int32
ccs_cfg_build_app_ntp_ace(FILE* fp, cfg_cmd_para_t *para)
{
    ctclib_slistnode_t * listnode = NULL; 
    tbl_ntp_ace_t* ace = NULL;
    tbl_ntp_ace_master_t *ace_master = tbl_ntp_ace_get_master();
    uint32 has_config = FALSE;
    uint32 i = 0;

    static char * option_name[] =
    {
        "version",
        "kod",
        "ignore",
        "noquery",
        "nomodify",
        "notrap",
        "lowpriotrap",
        "noserve",
        "nopeer",
        "notrust",
        "limited",
        "ntpport"
    };


    CTCLIB_SLIST_LOOP(ace_master->ntp_ace_list, ace, listnode)
    {
        sal_fprintf(fp, "ntp ace %s", ace->key.host);
        sal_fprintf(fp, " mask %d", ace->masklen);

        if (ace->option == 0)
        {
            sal_fprintf(fp, " none");
        }
        else
        {
            for (i=0; i<13; i++) /*note 13 is BITOFFSET_END*/
            {
                if(((ace->option >>i )&0x01))
                {
                    sal_fprintf(fp, " %s", option_name[i]);
                }
            }
        }

        sal_fprintf(fp, "\n");
                
        has_config = TRUE;
    }

    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}   

int32
ccs_cfg_build_app_cpu_limit(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_cpu_limit_master_t *p_master = tbl_cpu_limit_get_master();
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    tbl_cpu_limit_t *p_db_cpu_limit = NULL;

    CTCLIB_SLIST_LOOP(p_master->cpu_limit_list, p_db_cpu_limit, listnode)
    {
        sal_fprintf(fp, "cpu-limit %s percent %u\n",
            p_db_cpu_limit->key.name, p_db_cpu_limit->percent);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_app_iptables_prevent(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_iptables_prevent_t *p_db_iptables = tbl_iptables_prevent_get_iptables_prevent();
    uint32 has_config = FALSE;
  
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
    {
        sal_fprintf(fp, "ip intercept fraggle\n");
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_ICMP))
    {
        sal_fprintf(fp, "ip intercept icmp maxcount %u\n", p_db_iptables->icmp_rate_limit);
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_IPEQ))
    {
        sal_fprintf(fp, "ip intercept ipeq\n");
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_MACEQ))
    {
        sal_fprintf(fp, "ip intercept maceq\n");
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT))
    {
        sal_fprintf(fp, "ip intercept small-packet length %u\n", p_db_iptables->small_pkt_length);
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_SMURF))
    {
        sal_fprintf(fp, "ip intercept smurf\n");
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_TCP))
    {
        sal_fprintf(fp, "ip intercept tcp maxcount %u\n", p_db_iptables->tcp_rate_limit);
        has_config = TRUE;
    }
    if (GLB_FLAG_ISSET(p_db_iptables->flags, GLB_IPTABLES_CFG_FLAG_UDP))
    {
        sal_fprintf(fp, "ip intercept udp maxcount %u\n", p_db_iptables->udp_rate_limit);
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_app_worm_filter(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_acl_worm_filter_master_t *p_master = tbl_acl_worm_filter_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_acl_worm_filter_t *p_db_worm = NULL;
    uint32 has_config = FALSE;
  
    CTCLIB_SLIST_LOOP(p_master->acl_worm_filter_list, p_db_worm, listnode)
    {
        if (p_db_worm->isrange)
        {
            sal_fprintf(fp, "worm-filter %s %u to %u %s\n",
                p_db_worm->istcp ? "tcp" : "udp", p_db_worm->dstport, p_db_worm->isrange, p_db_worm->key.worm_filter_name);
        }
        else
        {
            sal_fprintf(fp, "worm-filter %s %u %s\n",
                p_db_worm->istcp ? "tcp" : "udp", p_db_worm->dstport, p_db_worm->key.worm_filter_name);
        }
        has_config = TRUE;
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_dhcsrvgrp(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_dhcsrvgrp_master_t *p_master = tbl_dhcsrvgrp_get_master();
    tbl_dhcsrvgrp_t *p_db_srvgrp = NULL;
    uint32 id = 0;
    uint32 has_config = FALSE;
    char   addrs[DHCP_SRV_ADDRS_BUF_SIZE];
    uint32 index = 0;

    for (id = 0; id < GLB_MAX_DHCSRVGRP_NUM; id++)
    {
        p_db_srvgrp = p_master->dhcsrvgrp_array[id];

        if (NULL == p_db_srvgrp || 0 == sal_strlen(p_db_srvgrp->addrs))
        {
            continue;
        }

        /* use blank instead ";", and print new string */
        sal_strncpy(addrs, p_db_srvgrp->addrs, DHCP_SRV_ADDRS_BUF_SIZE);
        for (index = 0; index < DHCP_SRV_ADDRS_BUF_SIZE; index++)
        {
            if ('\0' == addrs[index])
            {
                break;
            }
            
            if (';' == addrs[index])
            {
                addrs[index]=' ';
            }
        }
        sal_fprintf(fp, "dhcp-server %u %s\n", (id + 1), addrs);
        has_config = TRUE;
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_dhcrelay(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t    *p_glb      = tbl_sys_global_get_sys_global();
    tbl_dhcrelay_t *p_relay = tbl_dhcrelay_get_dhcrelay();
    uint32 has_config = FALSE;

    if (p_relay->add_agent_options)
    {
        sal_fprintf(fp, "dhcp relay information option\n");
        has_config = TRUE;
    }

    if (FALSE == p_relay->drop_agent_mismatch)
    {
        sal_fprintf(fp, CMD_NO_STR" dhcp relay information check\n");
        has_config = TRUE;
    }
    
    switch (p_relay->agent_proc_option)
    {
    case AGENT_PROC_OPTION_APPEND:
        sal_fprintf(fp, "dhcp relay information policy append\n");
        break;
    case AGENT_PROC_OPTION_DISCARD:
        sal_fprintf(fp, "dhcp relay information policy drop\n");
        break;
    case AGENT_PROC_OPTION_REPLACE:
        sal_fprintf(fp, "dhcp relay information policy replace\n");
        break;
    default:
        break;
    }

    if (p_relay->trust_all)
    {
        sal_fprintf(fp, "dhcp relay information trust-all\n");
        has_config = TRUE;
    }
    
    if (p_glb->dhcp_relay_enable)
    {
        sal_fprintf(fp, "dhcp relay\n");
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_dhcsnooping(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_sys_global_t    *p_glb      = tbl_sys_global_get_sys_global();
    tbl_dhcsnooping_t   *p_snoop    = tbl_dhcsnooping_get_dhcsnooping();
    char    buf[MAX_CMD_STR_LEN];
    uint32  has_config = FALSE;
    buf[0] = CMD_ZERO_CHAR;

    if (p_snoop->hostname_as_remote_id)
    {
        sal_fprintf(fp, "dhcp snooping information option format remote-id hostname\n");
        has_config = TRUE;
    }
    else
    {
        if (sal_strlen(p_snoop->remote_id_string))
        {
            sal_fprintf(fp, "dhcp snooping information option format remote-id string %s\n",
                p_snoop->remote_id_string);
            has_config = TRUE;
        }
    }
    
    if (FALSE == p_snoop->verify_mac_address)
    {
        sal_fprintf(fp, CMD_NO_STR" dhcp snooping verify mac-address\n");
        has_config = TRUE;
    }
    
    if (p_snoop->add_agent_options)
    {
        sal_fprintf(fp, "dhcp snooping information option\n");
        has_config = TRUE;
    }

    if (p_snoop->allow_untrusted)
    {
        sal_fprintf(fp, "dhcp snooping information option allow-untrusted\n");
        has_config = TRUE;
    }
    
    cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_snoop->vlans, sizeof(p_snoop->vlans)/4);
    if (sal_strlen(buf))
    {
        ccs_show_vlan_range(fp, "dhcp snooping vlan", buf);
        has_config = TRUE;
    }
    
    if (p_glb->dhcp_snooping_enable)
    {
        sal_fprintf(fp, "dhcp snooping\n");
        has_config = TRUE;
    }

    if (DHCSNOOPING_SAVE_BINDING_INTERVAL_DEFAULT != p_snoop->database_save_interval)
    {
        sal_fprintf(fp, "dhcp snooping database auto-save interval %u\n",
            p_snoop->database_save_interval);
        has_config = TRUE;
    }
    
    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}

int32
ccs_cfg_build_dhclient(FILE* fp, cfg_cmd_para_t *para)
{
    //tbl_sys_global_t    *p_glb      = tbl_sys_global_get_sys_global();
    tbl_dhclient_t *p_client = tbl_dhclient_get_dhclient();
    uint32 has_config = FALSE;

    if (DHCLIENT_DEFAULT_ROUTER_DISTANCE != p_client->distance)
    {
        sal_fprintf(fp, "dhcp client default-router distance %d\n", p_client->distance);
        has_config = TRUE;
    }

    if (DHCLIENT_DEFAULT_FLAGS != p_client->broadcast)
    {
        sal_fprintf(fp, CMD_NO_STR" dhcp client broadcast-flag\n");
        has_config = TRUE;
    }

    if (has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}
int32
ccs_cfg_build_app_cem(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_cem_master_t *p_master = tbl_cem_get_master();
    tbl_cem_t *p_db_cem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 has_config = FALSE;
    unsigned char buf[CEM_ADDRESS_SIZE];
         
    CTCLIB_SLIST_LOOP(p_master->cem_list, p_db_cem, listnode)
    {
        sal_fprintf(fp, "event manager %s\n",p_db_cem->key.name);
        
        if(sal_strcmp(p_db_cem->event,""))
        {
            sal_fprintf(fp, INTEND_1_STR"event %s threshold %d\n",p_db_cem->event,p_db_cem->threshold);
        }        
        if(0 != p_db_cem->snmptrap)
        {
            sal_fprintf(fp, INTEND_1_STR"action snmp-trap\n");
        }
        if(sal_strcmp(p_db_cem->loadpath,""))
        {
            sal_b64_pton(p_db_cem->loadpath, buf, COMM_NAME_SIZE); 
            sal_fprintf(fp, INTEND_1_STR"action load-config %s\n",buf);
        }
        if(sal_strcmp(p_db_cem->mail.from,""))
        {
            sal_b64_pton(p_db_cem->mail.subject, buf, CEM_SUBJECT_SIZE);
            sal_fprintf(fp, INTEND_1_STR"action mail from %s to %s subject \"%s\"",
                p_db_cem->mail.from,p_db_cem->mail.to,buf);

            if(sal_strcmp(p_db_cem->mail.cc,""))
            {
                sal_fprintf(fp, " cc %s",p_db_cem->mail.cc);
            }
            if(sal_strcmp(p_db_cem->mail.body,""))
            {   
                sal_b64_pton(p_db_cem->mail.body, buf, CEM_BODY_SIZE);
                sal_fprintf(fp, " body \"%s\"",buf);
            }
            if(sal_strcmp(p_db_cem->mail.server,""))
            {
                sal_fprintf(fp, " server %s", p_db_cem->mail.server);
            }
            if(sal_strcmp(p_db_cem->mail.attach,""))
            {
                sal_b64_pton(p_db_cem->mail.attach, buf, CEM_ADDRESS_SIZE);
                sal_fprintf(fp, " attach %s",buf);
            }
            sal_fprintf(fp,"\n");
        }

        has_config = TRUE;
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }
    return PM_E_NONE;
}

int32
ccs_cfg_build_sys_enable(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_enable_master_t *enable_master = NULL;
    tbl_enable_t *pEnable = NULL;
    int i = 0;
    int has_config = 0;

    enable_master = tbl_enable_get_master();
    for (i = 0; i < GLB_MAX_PRIVILEAGE_NUM; i++) {
        pEnable = enable_master->enable_array[i];
        if ('\0' != pEnable->passwd[0] 
                && sal_strcasecmp(pEnable->passwd, "")) {
            has_config++;
            if (i == (GLB_MAX_PRIVILEAGE_NUM - 1)) {
                sal_fprintf(fp, "enable password %s\n", pEnable->passwd);
            } else {
                sal_fprintf(fp, "enable password privilege %d %s\n", 
                        (i+1), pEnable->passwd);
            }

        } else if ('\0' != pEnable->enc_passwd[0] 
                && strcasecmp(pEnable->enc_passwd, "")) {
            has_config++;
            if (i == (GLB_MAX_PRIVILEAGE_NUM - 1)) {
                sal_fprintf(fp, "enable password 8 %s\n", pEnable->enc_passwd);
            } else {
                sal_fprintf(fp, "enable password privilege %d 8 %s\n", 
                        (i+1), pEnable->enc_passwd);
            }

        }
    }
    
    if(has_config)
    {
        sal_fprintf(fp, "!\n");
    }

    return PM_E_NONE;
}

int32
ccs_cfg_build_app_clock(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    char *direction = NULL;

    direction = (p_db_clk->timezone_positive) ? "add" : "minus";
    if (sal_strlen(p_db_clk->timezone_name))
    {
        sal_fprintf(fp, "clock set timezone %s %s %u %u %u\n", p_db_clk->timezone_name, direction, 
            p_db_clk->timezone_hour, p_db_clk->timezone_minute, p_db_clk->timezone_second);
        sal_fprintf(fp, "!\n");
    }
    
    return PM_E_NONE;
}
int32
ccs_cfg_build_app_summer_clock(FILE* fp, cfg_cmd_para_t *para)
{
    tbl_tag_summer_clock_t *p_summer_db_clk =  NULL;
    p_summer_db_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    if (!sal_strlen(p_summer_db_clk->strZoneName))
    {
        return PM_E_NONE;
    }
    if (!p_summer_db_clk->is_recurring)
    {
        sal_fprintf(fp,"clock set summer-time %s date %02u %u %u %02u:%02u:%02u %02u %u %u %02u:%02u:%02u %u\n",
                    p_summer_db_clk->strZoneName,p_summer_db_clk->tmstart.tm_mon,p_summer_db_clk->tmstart.tm_mday,
                    p_summer_db_clk->tmstart.tm_year,p_summer_db_clk->tmstart.tm_hour,p_summer_db_clk->tmstart.tm_min,
                    p_summer_db_clk->tmstart.tm_sec,p_summer_db_clk->tmend.tm_mon,p_summer_db_clk->tmend.tm_mday,
                    p_summer_db_clk->tmend.tm_year,p_summer_db_clk->tmend.tm_hour,p_summer_db_clk->tmend.tm_min,
                    p_summer_db_clk->tmend.tm_sec,p_summer_db_clk->iDstOffset);
        sal_fprintf(fp, "!\n");
        return PM_E_NONE;
    }
    if (p_summer_db_clk->is_recurring&&p_summer_db_clk->is_wday_tm)
    {
         char *weekday[7] = {"sunday","monday","tuesday","wednesday","thursday","friday","saturday"};
         char *order[5] = {"first","second","third","fourth","last"};
         sal_fprintf(fp,"clock set summer-time %s recurring %s %s %02u %02u:%02u:%02u %s %s %02u %02u:%02u:%02u %u\n",p_summer_db_clk->strZoneName,
                    order[p_summer_db_clk->wday_tm_start.wday_tm_which-1],weekday[p_summer_db_clk->wday_tm_start.wday_tm_wday],
                    p_summer_db_clk->wday_tm_start.wday_tm_mon,p_summer_db_clk->wday_tm_start.wday_tm_hour,
                    p_summer_db_clk->wday_tm_start.wday_tm_min,p_summer_db_clk->wday_tm_start.wday_tm_sec,
                    order[p_summer_db_clk->wday_tm_end.wday_tm_which-1],weekday[p_summer_db_clk->wday_tm_end.wday_tm_wday],
                    p_summer_db_clk->wday_tm_end.wday_tm_mon,p_summer_db_clk->wday_tm_end.wday_tm_hour,
                    p_summer_db_clk->wday_tm_end.wday_tm_min,p_summer_db_clk->wday_tm_end.wday_tm_sec,p_summer_db_clk->iDstOffset);
         sal_fprintf(fp, "!\n");
         return PM_E_NONE;
    }
    if (p_summer_db_clk->is_recurring&&(!p_summer_db_clk->is_wday_tm))
    {
        sal_fprintf(fp,"clock set summer-time %s recurring %02u %u %02u:%02u:%02u %02u %u %02u:%02u:%02u %u\n",
                    p_summer_db_clk->strZoneName,p_summer_db_clk->tmstart.tm_mon,p_summer_db_clk->tmstart.tm_mday,
                    p_summer_db_clk->tmstart.tm_hour,p_summer_db_clk->tmstart.tm_min,p_summer_db_clk->tmstart.tm_sec,
                    p_summer_db_clk->tmend.tm_mon,p_summer_db_clk->tmend.tm_mday,p_summer_db_clk->tmend.tm_hour,p_summer_db_clk->tmend.tm_min,
                    p_summer_db_clk->tmend.tm_sec,p_summer_db_clk->iDstOffset);
         sal_fprintf(fp, "!\n");
         return PM_E_NONE;
    }
    return PM_E_NONE;
}


