
#include "proto.h"
#include "cdbclient.h"

static int32
_cdbctl_show_rawfile(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        /* bug52281 cdbctl crash when act_show '%s' */
        sal_printf("%s", line);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_dispatch(cdb_node_t* p_node, void *p_data, uint32 data_len, char *file_name, cfg_parse_info_t *p_info)
{
    FILE *fp = NULL;
    
    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }

    if (CDB_NODE_TBL == p_node->type)
    {
        switch (p_node->id)
        {
        case TBL_INTERFACE:
            cdbctl_show_interface(p_data, data_len, fp, p_info);
            break;
        case TBL_ACLQOS_IF_STATS:
            cdbctl_show_aclqos_if_stats(p_data, data_len, fp, p_info);
            break;
        case TBL_ACLQOS_IF:
            cdbctl_show_aclqos_if(p_data, data_len, fp, p_info);
            break;
        case TBL_MIRROR:
     //       cdbctl_show_mirror(p_data, data_len, fp, p_info);
            break;

        case TBL_QOS_GLOBAL:
            cdbclt_show_qos_global(fp, p_info);
            break;

        case TBL_QOS_DOMAIN:
            cdbclt_show_qos_domain(p_data, data_len, fp, p_info);
            break;

        case TBL_QOS_MPLS_DOMAIN:
            cdbclt_show_qos_mpls_domain(p_data, data_len, fp, p_info);
            break;

        case TBL_QOS_POLICER_ACTION_PROFILE:
            cdbclt_show_qos_policer_action_profile(p_data, data_len, fp, p_info);
            break;   
            
        case TBL_QOS_POLICER_PROFILE:
            cdbclt_show_qos_policer_profile(p_data, data_len, fp, p_info);
            break;   

        case TBL_QOS_DROP_PROFILE:
            cdbclt_show_qos_drop_profile(fp, p_info);
            break;  

        case TBL_QOS_QUEUE_SHAPE_PROFILE:
            cdbclt_show_qos_queue_shape_profile(fp, p_info);
            break;        

        case TBL_QOS_PORT_SHAPE_PROFILE:
            cdbclt_show_qos_port_shape_profile(fp, p_info);
            break;

        case TBL_TIME_RANGE:
            cdbctl_show_time_range(fp, p_info);
            break;
                   
        case TBL_VLAN:
            cdbctl_show_vlan(fp, p_info);
            break;
        case TBL_PVLAN:
            cdbctl_show_pvlan(fp, p_info);
            break;   
        case TBL_FDB:
            cdbctl_show_fdb(p_data, data_len, fp, p_info);
            break;

        case TBL_MACFILTER:
            cdbctl_show_macfilter(p_data, data_len, fp, p_info);
            break;

        case TBL_BRG_GLOBAL:
            cdbctl_show_brg_global(p_data, data_len, fp, p_info);
            break;

        case TBL_LAG_GLOBAL:
            cdbctl_show_lag_global(p_data, data_len, fp, p_info);
            break;

        case TBL_MSTP_PORT:
            cdbctl_show_mstp_port(p_data, data_len, fp, p_info);
            break;

        case TBL_MSTP_GLOBAL:
            cdbctl_show_mstp_global(p_data, data_len, fp, p_info);
            break;

        case TBL_MSTP_INSTANCE:
            cdbctl_show_mstp_instance(p_data, data_len, fp, p_info);
            break;
    
        case TBL_ROUTE_GLOBAL:
            break;

        case TBL_ROUTE_IF:
            break;

        case TBL_KERNEL_IF:
            //cdbctl_show_ip_interface(p_data, data_len, fp, p_info);
            break;

        case TBL_COPP_CFG:
            cdbclt_show_copp_cfg(fp, p_info);
            break;
        case TBL_CPU_TRAFFIC:
            break;
        case TBL_CPU_TRAFFIC_GROUP:
            cdbclt_show_cpu_traffic_group(fp, p_info);
            break;

        case TBL_IPROUTE_NODE:
            cdbctl_show_iproute_node(p_data, data_len, fp, p_info);
            break;

        case TBL_ARP:
            //cdbctl_show_arp(p_data, data_len, fp, p_info);
            break;
            
        case TBL_ARP_FIB:
            break;
            
        case TBL_SYS_GLOBAL:
            cdbctl_show_sys_global(p_data, data_len, fp, p_info);
            break;

        case TBL_VERSION:
            cdbctl_show_version(p_data, data_len, fp, p_info);
            break;
            
        case TBL_SYS_SPEC:
            //cdbctl_show_sys_spec(p_data, data_len, fp, p_info);
            break;

        case TBL_MANAGE_IF:
            cdbctl_show_manage_if(p_data, data_len, fp, p_info);
            break;

        case TBL_BOOTIMAGE:
            cdbctl_show_bootimage(p_data, data_len, fp, p_info);
            break;

        case TBL_CHASSIS:
            cdbctl_show_chassis(p_data, data_len, fp, p_info);
            break;

        case TBL_REBOOT_INFO:
            cdbctl_show_rebootinfo(p_data, data_len, fp, p_info);
            break;
            
        case TBL_MEM_SUMMARY:
            cdbctl_show_memory_summary(p_data, data_len, fp, p_info);
            break;
            
        case TBL_SWITCH_DEBUG:
            cdbctl_show_switch_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_ROUTE_DEBUG:
            cdbctl_show_route_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_APP_DEBUG:
            cdbctl_show_app_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_OPENFLOW_DEBUG:
            cdbctl_show_openflow_debug(p_data, data_len, fp, p_info);
            break;
            
        case TBL_SSM_DEBUG:
            cdbctl_show_ssm_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_PTP_DEBUG:
            cdbctl_show_ptp_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_QUAGGA_DEBUG:
            cdbctl_show_quagga_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_DHCP_DEBUG:
            cdbctl_show_dhcp_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_LSRV_DEBUG:
            cdbctl_show_lsrv_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_HSRV_DEBUG:
            cdbctl_show_hsrv_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_AUTHD_DEBUG:
            cdbctl_show_auth_debug(p_data, data_len, fp, p_info);
            break;

        case TBL_SSH_CFG:
            cdbctl_show_app_ssh_cfg(fp, p_info);
            break;
        case TBL_SYSLOG_CFG:
            cdbctl_show_app_syslog_cfg(p_data, data_len, fp, p_info);
            break;
        case TBL_USER:
            cdbctl_show_app_user(p_data, data_len, fp, p_info);
            break;
        case TBL_RSA:
            cdbctl_show_app_rsa(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCSRVGRP:
            cdbctl_show_app_dhcsrvgrp(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCRELAY:
            cdbctl_show_app_dhcrelay(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCLIENT:
            cdbctl_show_app_dhclient(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCSNOOPING:
            cdbctl_show_app_dhcsnooping(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCBINDING:
            cdbctl_show_app_dhcbinding(p_data, data_len, fp, p_info);
            break;
        case TBL_AUTHEN:
            cdbctl_show_app_authen(p_data, data_len, fp, p_info);
            break; 
        case TBL_CONSOLE:
            cdbctl_show_app_console(p_data, data_len, fp, p_info);
            break;
        case TBL_VTY:
            cdbctl_show_app_vty(p_data, data_len, fp, p_info);
            break;
        case TBL_LOGIN:
            cdbctl_show_app_login(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_CFG:
            cdbctl_show_app_snmp_cfg(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_VIEW:
            cdbctl_show_app_snmp_view(p_data, data_len, fp, p_info);
            break;
        case TBL_NTP_KEY:
            cdbctl_show_app_ntp_key(p_data, data_len, fp, p_info);
            break;
        case TBL_NTP_CFG:
            cdbctl_show_app_ntp_cfg(p_data, data_len, fp, p_info);
            break;
        case TBL_NTP_ACE:
            cdbctl_show_app_ntp_ace(p_data, data_len, fp, p_info);
            break;
        case TBL_NTP_SERVER:
            cdbctl_show_app_ntp_server(p_data, data_len, fp, p_info);
            break;
        case TBL_STATIC_DNS:
            cdbctl_show_app_static_dns(p_data, data_len, fp, p_info);
            break;    
        case TBL_DYNAMIC_DNS_DOMAIN:
            cdbctl_show_app_dynamic_dns_domain(p_data, data_len, fp, p_info);
            break;  
        case TBL_DYNAMIC_DNS_SERVER:
            cdbctl_show_app_dynamic_dns_server(p_data, data_len, fp, p_info);
            break;  
        case TBL_SNMP_COMMUNITY:
            cdbctl_show_app_snmp_community(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_CONTEXT:
            cdbctl_show_app_snmp_context(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_USM_USER:
            cdbctl_show_app_snmp_usm_user(p_data, data_len, fp, p_info);
            break;
        
        case TBL_SNMP_GROUP_USM:
            cdbctl_show_app_snmp_group_usm(p_data, data_len, fp, p_info);
            break;

        case TBL_SNMP_ACCESS_USM:
            cdbctl_show_app_snmp_access_usm(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_NOTIFY:
            cdbctl_show_app_snmp_notify(p_data, data_len, fp, p_info);
            break;   
        case TBL_SNMP_TARGET_PARAMS:
            cdbctl_show_app_snmp_target_params(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_TARGET_ADDR:
            cdbctl_show_app_snmp_target_addr(p_data, data_len, fp, p_info);
            break;
        case TBL_SNMP_TRAP:
            cdbctl_show_app_snmp_trap(p_data, data_len, fp, p_info);
            break;    
        case TBL_SNMP_INFORM:
            cdbctl_show_app_snmp_inform(p_data, data_len, fp, p_info);
            break;      
        case TBL_CPU_UTILIZATION:
            cdbctl_show_app_cpu_utilization(p_data, data_len, fp, p_info);
            break;    
        case TBL_FIBER:
            cdbctl_show_transceiver(p_data, data_len, fp, p_info);
            break;
        case TBL_IPTABLES_PREVENT:
            cdbctl_show_iptables_prevent(p_data, data_len, fp, p_info);
            break;
        case TBL_ERRDISABLE:
            cdbctl_show_errdisable(p_data, data_len, fp, p_info);
            break;
        case TBL_LOG_GLOBAL:
            cdbctl_show_log_global(p_data, data_len, fp, p_info);
        case TBL_LOG:
            cdbctl_show_log(p_data, data_len, fp, p_info);
            break;
        case TBL_ERPS_DOMAIN:
            break;
        case TBL_OPM_DEBUG:
            cdbctl_show_opm_debug(p_data, data_len, fp, p_info);
            break;
        case TBL_ENABLE:
            cdbctl_show_sys_enable(p_data, data_len, fp, p_info);
        case TBL_AUTHOR:
            cdbctl_show_app_author(p_data, data_len, fp, p_info);
            break;
        case TBL_ACCOUNT:
            cdbctl_show_app_account(p_data, data_len, fp, p_info);
            break;
        case TBL_ACCOUNTCMD:
            cdbctl_show_app_accountcmd(p_data, data_len, fp, p_info);
            break;
        case TBL_DHCPV6_DEBUG:
            cdbctl_show_dhcpv6_debug(p_data, data_len, fp, p_info);
            break;
        case TBL_NAT_POOL:
            cdbctl_show_nat_pool_node(p_data, data_len, fp, p_info);
            break;
        case TBL_NAT_RULE:
            cdbctl_show_nat_rule_node(p_data, data_len, fp, p_info);
            break;
        case TBL_NAT_SESSION:
            cdbctl_show_nat_session_node(p_data, data_len, fp, p_info);
            break;
        case TBL_NAT_SESSION_LIMIT:
            cdbctl_show_nat_session_limit_node(p_data, data_len, fp, p_info);
            break;
        default:
            _cdbctl_show_rawfile(fp, p_info);
        }
    }
    else if (CDB_NODE_ACT == p_node->type)
    {
        switch (p_node->id)
        {
 #if 0
        case ACT_SHOW_PSFDB:
            if(sal_strstr((char*)p_data, "count"))
            {
                cdbctl_show_psfdb_current_port_count(p_data, data_len, fp, p_info);
            }
            else if((sal_strstr((char*)p_data, "all"))||(sal_strstr((char*)p_data, "dynamic"))||(sal_strstr((char*)p_data, "static")))
            {
                cdbctl_show_psfdb(p_data, data_len, fp, p_info);
            }
            else if(sal_strstr((char*)p_data, "interface"))
            {
                cdbctl_show_psfdb_interface(p_data, data_len, fp, p_info);
            }
            else if(sal_strstr((char*)p_data, "status"))
            {
                cdbctl_show_psfdb_status(p_data, data_len, fp, p_info);
            }
            break;
#endif

        case ACT_SHOW_ADD_FAILED_FIB:
            break;

        default:
            _cdbctl_show_rawfile(fp, p_info);
        }
    }

    sal_fclose(fp);
    sal_unlink(file_name);

    return PM_E_NONE;
}

int32
cdbclt_process_cmd_show(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    rc = ipc_ctl_to_ccs_cfg(p_info->oper, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command: %s failed, rc is %d\n", (char*)p_data, rc);
        return rc;
    }
    
    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        rc = _cdbctl_show_dispatch(p_node, p_data, data_len, p_resp_show->path, p_info);
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        rc = p_resp_rslt->rc;
        sal_printf("%% %s \n", p_resp_rslt->desc);
    }
    ctc_msg_free(p_resp_msg);
    
    return rc;
}

