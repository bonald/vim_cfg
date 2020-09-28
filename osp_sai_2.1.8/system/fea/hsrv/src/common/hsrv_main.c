
#include "hsrv_inc.h"
#include "glb_l2_define.h"
#include "hsrv_mirror.h"
#include "hsrv_qos.h"
#include "hsrv_port_stats.h"
#include "hsrv_aclqos_if.h"
#include "hsrv_cpu_traffic.h"
#include "hsrv_sys_action.h"
#include "hsrv_router.h"
#include "hsrv_route.h"
#include "hsrv_stp.h"
#include "hsrv_isolation.h"
#include "hsrv_port_security.h"
#include "hsrv_ip_binding.h"
#include "hsrv_mlag.h"
#include "hsrv_brgif.h"
#include "hsrv_mcfdb.h"
#include "hsrv_macfilter.h"
#include "hsrv_iptables_prevent.h"
#include "hsrv_lldp.h"
#include "hsrv_dot1x.h"
#include "hsrv_stats.h"
#include "hsrv_ptp.h"
#include "hsrv_copp.h"
#include "hsrv_hostif.h"
#include "hsrv_time.h"
#include "hsrv_hash.h"
#include "hsrv_l2protocol.h"
#include "hsrv_openflow.h"
#include "hsrv_g8032.h"
#include "hsrv_vrrp.h"
#include "hsrv_mrt.h"
#include "hsrv_nat.h"
#include "sai.h"
#include "ctc_sai_common.h"
#ifdef TAPPRODUCT
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "hsrv_tap.h"
#endif /* !TAPPRODUCT */
#include "gen/tbl_fea_global_define.h"
#include "gen/tbl_fea_global.h"
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include "sdb.h"
#include "hsrv_sdb.h"
#include "hsrv_wb.h"
#include "hsrv_mirror.h"
#include "rdb_hiredis.h"
#include "hsrv_hash_field_and_value.h"
#include "hsrv_cfm.h"

void
lsrv_card_init(void* data);

int32
lsrv_debug_start();

int32
lsrv_synce_start();

extern int32
hsrv_openflow_set_vxlan_destport(uint32 is_set, uint32 destport);

extern int32
hsrv_cmd_act_cpu_mirror(char **argv, int32 argc, cfg_cmd_para_t *para);


#define HSRV_CARD_INIT_TIMER 0

/***************************************************************************************************
 * Name         : _hsrv_modules_start
 * Purpose      : start hal server all modules, one module start will not depend on other modules, eg: msg callbacks register
 * Input        : N/A
 * Output       : N/A
 * Return       : HSRV_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hsrv_modules_start(hsrv_master_t* master)
{
    /*1. Start hal server message module*/
    HSRV_IF_ERROR_RETURN(hsrv_message_module_start(&(master->p_messge_info)));
    HSRV_IF_ERROR_RETURN(hsrv_brgif_start());
    HSRV_IF_ERROR_RETURN(hsrv_debug_start());
    HSRV_IF_ERROR_RETURN(hsrv_fdb_start()); 
    HSRV_IF_ERROR_RETURN(hsrv_mcfdb_start()); 
    HSRV_IF_ERROR_RETURN(hsrv_mrt_start());
    HSRV_IF_ERROR_RETURN(hsrv_macfilter_start()); 
    HSRV_IF_ERROR_RETURN(hsrv_stp_start());
    HSRV_IF_ERROR_RETURN(hsrv_stats_module_start());
    return HSRV_E_NONE;
}

#define GLB_HSRV_DONE_FILE       "/tmp/glb_hsrv_done"

int32
hsrv_cmd_process_fea(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_SHOW:
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_fea_cpu_packets_clear_stats(tbl_cpu_packets_t *cpu_stats)
{
    sal_memset(cpu_stats, 0, sizeof(tbl_cpu_packets_t));
    
    return HSRV_E_NONE;
}

int32
fea_cpu_packets_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_cpu_packets_t *cpu_stats = tbl_cpu_packets_get_cpu_packets();
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    switch (para->oper)
    {
    case CDB_OPER_SET:
        _fea_cpu_packets_clear_stats(cpu_stats);
        break;

    case CDB_OPER_GET:
        sal_fprintf(fp, "CPU Packet Stats:\n\n");
        sal_fprintf(fp, "%-20s %-15s %-15s %-15s\n", "Type", "Rx", "Tx", "Total");
        sal_fprintf(fp, "--------------------+---------------+---------------+---------------\n");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "BPDU", cpu_stats->rx_bpdu, cpu_stats->tx_bpdu, cpu_stats->rx_bpdu+cpu_stats->tx_bpdu);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "SLOW PROTO", cpu_stats->rx_slow_proto, cpu_stats->tx_slow_proto, cpu_stats->rx_slow_proto+cpu_stats->tx_slow_proto);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "EAPOL", cpu_stats->rx_eapol, cpu_stats->tx_eapol, cpu_stats->rx_eapol+cpu_stats->tx_eapol);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "LLDP", cpu_stats->rx_lldp, cpu_stats->tx_lldp, cpu_stats->rx_lldp+cpu_stats->tx_lldp);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "ERPS", cpu_stats->rx_erps, cpu_stats->tx_erps, cpu_stats->rx_erps+cpu_stats->tx_erps);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "ARP", cpu_stats->rx_arp, cpu_stats->tx_arp, cpu_stats->rx_arp+cpu_stats->tx_arp);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "DHCP", cpu_stats->rx_dhcp, cpu_stats->tx_dhcp, cpu_stats->rx_dhcp+cpu_stats->tx_dhcp);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "IGMP", cpu_stats->rx_igmp, cpu_stats->tx_igmp, cpu_stats->rx_igmp+cpu_stats->tx_igmp);        
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "PTP", cpu_stats->rx_ptp, cpu_stats->tx_ptp, cpu_stats->rx_ptp + cpu_stats->tx_ptp);

        #ifdef OFPRODUCT
        #ifdef CONFIG_TPOAM
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "TPOAM", cpu_stats->rx_tpoam, cpu_stats->tx_tpoam, cpu_stats->rx_tpoam + cpu_stats->tx_tpoam);
        #endif
        #endif
        /*modified by yejl for bug 52079, 2019-05-30*/
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "L2PRO-PROTOCOL-MAC", cpu_stats->rx_l2pro_protocol_mac, cpu_stats->tx_l2pro_protocol_mac, cpu_stats->rx_l2pro_protocol_mac+cpu_stats->tx_l2pro_protocol_mac);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "L2PRO-GROUP-MAC", cpu_stats->rx_l2pro_group_mac, cpu_stats->tx_l2pro_group_mac, cpu_stats->rx_l2pro_group_mac+cpu_stats->tx_l2pro_group_mac);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "CPU-MIRROR", cpu_stats->rx_cpu_mirror, "-", "-");
        /*ended by yejl*/
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "RIP", cpu_stats->rx_rip, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "OSPF", cpu_stats->rx_ospf, "-", "-");
        /* sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "BGP", cpu_stats->rx_bgp, "-", "-"); removed for bug 43986 */
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "MACDA", cpu_stats->rx_macda, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "IPDA", cpu_stats->rx_ipda, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "MAC LIMIT", cpu_stats->rx_mac_limit, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "MAC MISMATCH", cpu_stats->rx_mac_mismatch, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "L3COPY CPU", cpu_stats->rx_l3copy_cpu, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "TTL ERROR", cpu_stats->rx_ttl_error, "-", "-");
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "MTU ERROR", cpu_stats->rx_mtu_fail, "-", "-");
        #ifdef OFPRODUCT
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15s %-15s\n", "PACKET IN", cpu_stats->rx_packetin, "-", "-");
        #endif
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "ND", cpu_stats->rx_nd, cpu_stats->tx_nd, cpu_stats->rx_nd+cpu_stats->tx_nd);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "RAPS", cpu_stats->rx_g8032, cpu_stats->tx_g8032, cpu_stats->rx_g8032 + cpu_stats->tx_g8032);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "Other", cpu_stats->rx_other, cpu_stats->tx_other, cpu_stats->rx_other+cpu_stats->tx_other);
        sal_fprintf(fp, "%-20s %-15"PRIu64" %-15"PRIu64" %-15"PRIu64"\n", "Total", cpu_stats->rx, cpu_stats->tx, cpu_stats->total);
        break;

    default:
        break;
    }

    sal_fclose(fp);
    return HSRV_E_NONE;
}

int32
fea_global_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_iter_args_t args;
    tbl_fea_global_t *p_fea_glb = tbl_fea_global_get_fea_global();
    uint32 port_stats_ms = 0;
    int32 rc = 0;
    FILE *fp = NULL;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        if (0 == sal_strcmp(argv[0], "port_stats_ms"))
        {
            PM_CFG_GET_UINT(port_stats_ms, argv[1], para->p_rs);
            p_fea_glb->port_stats_ms = port_stats_ms;
        }
        else
        {
            CFG_CONFLICT_RET("Invalid field name");
        }
        break;

    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        sal_memset(&field, FLD_ID_ALL, sizeof(field));
        args.argv[0] = &field;
        args.argv[1] = fp;
        tbl_fea_global_dump_one(p_fea_glb, &args);
        sal_fclose(fp);
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_modeule_openflow_init(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_module_start());
    hsrv_system_spec_info_t p_system_info;
    sal_memset(&p_system_info, 0x0, sizeof(hsrv_system_spec_info_t));
    p_system_info.res_info.openflow_linkagg_interface_max_tid = 17;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_module_init(&p_system_info));

    return HSRV_E_NONE;
}

#ifndef TTPPRODUCT
int32
hsrv_openflow_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_openflow_t *p_openflow = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_openflow = (tbl_openflow_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        if (TBL_OPENFLOW_FLD_VXLAN_DESTPORT_IS_SET == field_id)
        {
            hsrv_openflow_set_vxlan_destport(p_openflow->vxlan_destport_is_set, p_openflow->vxlan_destport);
        }
        else if (TBL_OPENFLOW_FLD_STPID == field_id)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_set_stpid(p_openflow->stpid));
        }
        else if (TBL_OPENFLOW_FLD_CTPID == field_id)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_set_ctpid(p_openflow->ctpid));
        }
        else if (TBL_OPENFLOW_FLD_FDB_FLOW_VALID == field_id)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_set_fdb_flow_en());
        }
        else if (TBL_OPENFLOW_FLD_FFGROUP_SWITCHBACK_EN == field_id)
        {
            HSRV_IF_ERROR_RETURN(hsrv_openflow_set_ffgroup_switchback_en(p_openflow->ffgroup_switchback_en));
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}
#endif

int32
hsrv_module_init()
{
    cdb_pm_subscribe_tbl(TBL_BRG_GLOBAL, hsrv_switch_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_BHM_GLOBAL, hsrv_switch_bhm_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_LAG_GLOBAL, hsrv_lag_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_ECMP_GLOBAL, hsrv_ecmp_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_SYS_GLOBAL, hsrv_sys_global_sync, NULL);

    /*LLDP*/
    cdb_pm_subscribe_tbl(TBL_LLDP_GLOBAL, hsrv_lldp_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_LLDP_IF, hsrv_lldp_if_sync, NULL);

    /*L2 protocol*/
    cdb_pm_subscribe_tbl(TBL_L2_PROTOCOL, hsrv_l2protocol_sync, hsrv_l2protocol_sync_after);
    
    cdb_pm_subscribe_tbl(TBL_VLAN, hsrv_vlan_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_VLAN_GROUP, hsrv_vlan_group_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_VLAN_MAPPING_TABLE, hsrv_vlanmap_table_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_VLAN_MAPPING_ENTRY, hsrv_vlanmap_entry_sync, hsrv_vlanmap_entry_sync_after);
    cdb_pm_subscribe_tbl(TBL_EGRESS_MAPPING_ENTRY, hsrv_vlanmap_egress_entry_sync, hsrv_vlanmap_egress_entry_sync_after);
    cdb_pm_subscribe_tbl(TBL_IGSP_GLOBAL, hsrv_igsp_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_IGSP_INTF, hsrv_igsp_intf_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MIRROR, hsrv_mirror_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MIRROR_MAC_ESCAPE, hsrv_mirror_mac_escape_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_FDB, hsrv_fdb_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MCFDB, hsrv_mcfdb_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MACFILTER, hsrv_macfilter_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_L2_ACTION, hsrv_l2_action_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_G8032_RING, hsrv_g8032_ring_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_INTERFACE, hsrv_interface_sync, hsrv_interface_sync_after);
    cdb_pm_subscribe_tbl(TBL_PORT, hsrv_port_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_MRT, hsrv_mrt_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_IPMC_INTF, hsrv_ipmc_intf_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_OPENFLOW_INTERFACE, hsrv_openflow_interface_sync, NULL);
#ifndef TTPPRODUCT
    cdb_pm_subscribe_tbl(TBL_OPENFLOW, hsrv_openflow_sync, NULL);
#endif
    cdb_pm_subscribe_tbl(TBL_ROUTE_IF, hsrv_router_interface_sync, hsrv_router_interface_after_sync);
    cdb_pm_subscribe_tbl(TBL_ARP_FIB, hsrv_neighbor_sync, hsrv_neighbor_sync_after);

    /* support nd modified by liwh for bug 47547, 2018-07-06 */
    cdb_pm_subscribe_tbl(TBL_ND_FIB, hsrv_ipv6_neighbor_sync, NULL);
    /* liwh end */
    
#ifndef TAPPRODUCT    
    cdb_pm_subscribe_tbl(TBL_MSTP_PORT, hsrv_mstp_port_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MSTI_PORT, hsrv_msti_port_sync, NULL);
#endif
    cdb_pm_subscribe_tbl(TBL_MSTP_INSTANCE, hsrv_mstp_instance_sync, hsrv_mstp_instance_sync_after);    
    cdb_pm_subscribe_tbl(TBL_IPSG_FIB, hsrv_ip_source_guard_sync, NULL);

//    cdb_pm_subscribe_tbl(TBL_VLANCLASS_RULE, hsrv_lldp_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_VLANCLASS_GROUP, hsrv_vlanclass_sync, hsrv_vlanclass_sync_after);
    
    //cdb_pm_subscribe_tbl(TBL_PORT_STATS, hsrv_port_stats_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_QOS_DOMAIN, hsrv_qos_domain_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_MPLS_DOMAIN, hsrv_qos_mpls_domain_synce, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_POLICER_ACTION_PROFILE, hsrv_qos_policer_action_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_POLICER_PROFILE, hsrv_qos_policer_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_DROP_PROFILE, hsrv_qos_drop_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_QUEUE_SHAPE_PROFILE, hsrv_qos_scheduler_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_PORT_SHAPE_PROFILE, hsrv_qos_port_shape_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_ACLQOS_IF, hsrv_aclqos_if_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_QUEUE_SMART_BUFFER, hsrv_qos_queue_smart_buffer, NULL);
    cdb_pm_subscribe_tbl(TBL_QOS_GLOBAL, hsrv_qos_global_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_NEXTHOP, hsrv_router_nexthop_sync, NULL);
//#ifndef LDB_SYNC_TO_RDB
    cdb_pm_subscribe_tbl(TBL_IPROUTE_NODE, hsrv_route_rt_sync, hsrv_route_rt_after_sync);
//#else
    //cdb_sync_subscribe_channel(TBL_IPROUTE_NODE, hsrv_route_rt_sync, NULL);
//#endif

    cdb_pm_subscribe_tbl(TBL_NEXTHOP_GROUP, hsrv_router_nexthop_group_sync, hsrv_router_nexthop_group_sync_after);
    cdb_pm_subscribe_tbl(TBL_CPU_TRAFFIC, hsrv_cpu_traffic_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_CPU_TRAFFIC_GROUP, hsrv_cpu_traffic_group_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_ISOLATION, hsrv_isolation_sync, hsrv_isolation_sync_after);
    
    cdb_register_cfg_tbl_cb(TBL_ACLQOS_IF_STATS, hsrv_aclqos_if_process_aclqos_if_stats);
    cdb_register_cfg_act_cb(ACT_SHOW_STM, hsrv_cmd_show_stm);
    cdb_register_cfg_act_cb(ACT_CPU_MIRROR, hsrv_cmd_act_cpu_mirror);
    cdb_register_cfg_tbl_cb(TBL_FEA_NEXTHOP, fea_nexthop_cmd_process);
    
    cdb_register_cfg_tbl_cb(TBL_CPU_PACKETS, fea_cpu_packets_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_FEA_GLOBAL,  fea_global_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_RIF, rif_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_VLAN_STATS, hsrv_vlan_stats_cmd_process);

    cdb_pm_subscribe_tbl(TBL_MLAG, hsrv_mlag_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MLAG_PEER, hsrv_mlag_peer_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_MLAG_PORT, hsrv_mlag_port_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_ERPS_RING, hsrv_erps_ring_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_IPTABLES_PREVENT, hsrv_iptables_prevent_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_DOT1X_PORT, hsrv_dot1x_port_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_DOT1X_MAC, hsrv_dot1x_mac_sync, NULL);

    cdb_pm_subscribe_tbl(TBL_VRRP_VIP, hsrv_vrrp_sync, NULL);

    /* begin added by taocy for NAT 20190603*/
    cdb_pm_subscribe_tbl(TBL_NAT_RULE, hsrv_nat_rule_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_NAT_SESSION, hsrv_nat_session_sync, NULL);
    /* end added by taocy for NAT 20190603*/

    hsrv_interface_start();
    hsrv_acl_start();
    hsrv_lag_start();
    hsrv_port_stats_init();  
    hsrv_port_security_start();
    hsrv_neighbor_fdb_age_cb_init();

    /* modified by liwh for bug 48486, 2018-08-14 */
    hsrv_neighbor_nd_cb_init();
    /* liwh end */

    hsrv_dot1x_init();

#ifdef TAPPRODUCT
    hsrv_tap_start();
#endif
    hsrv_time_start();
    hsrv_ptp_start();
    
    hsrv_hostif_start();
    hsrv_copp_start();
    hsrv_mirror_start();
#ifdef TSINGMA
    hsrv_hash_field_and_value_start();/*Tsingma hash*/
#endif    
    hsrv_nat_start(); /* added by taocy for NAT 20190603*/
    hsrv_cfm_start(); /* modified by liwh for bug 53010, 2019-07-22 */
    

    return PM_E_NONE;
}

int32
hsrv_start(int32 daemon_mode)
{
    lib_master_t *lib_master = lib_init(PM_ID_FEA);
    hsrv_master_t *master = NULL;
    // SAL_FILE *fp = NULL;

    lib_master->task_mst = ctc_task_init();
    
    master = HSRV_CALLOC(MEM_FEA_TEMP, sizeof(hsrv_master_t));
    if (NULL == master)
    {
        return PM_E_NO_MEMORY;
    }
    lib_master->proto_mst = master;
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    sdb_init(PM_ID_FEA);
    hsrv_sdb_init();
    hsrv_wb_init();
    
    /*1. Start hal server modules*/
    HSRV_IF_ERROR_RETURN(_hsrv_modules_start(master));

#if 0
    /*3. Create hal server start done file, in order to notify chsm*/
    if ((fp = sal_fopen(GLB_HSRV_DONE_FILE, SAL_OPEN_RW)) != NULL)
    {
        sal_fclose(fp);
    }
#endif
    hsrv_module_init();
    lib_load_init();
    
    
    /*For center forward system, just call LSRV function to init lsrv/lcm/sdk*/
    /*For distribute system, these moudles will be start on linecard*/
    ctc_task_add_timer(TASK_PRI_NORMAL, lsrv_card_init, 
            NULL, HSRV_CARD_INIT_TIMER);
    
    HSRV_IF_ERROR_RETURN(lsrv_synce_start());
    HSRV_IF_ERROR_RETURN(lsrv_debug_start());

#if defined(OFDPAPRODUCT)
extern void indigo_start(void *arg);

    ctc_task_add_timer(TASK_PRI_LOW, indigo_start, 
            NULL, HSRV_CARD_INIT_TIMER);
#endif

    ctc_task_main();


    return PM_E_NONE;
}

void
usage(void)
{
    sal_printf("./hsrv -d -h\n");
    sal_printf("    -d, --daemon       Runs in daemon mode\n");
    sal_printf("    -h print this help message\n");
    return;
}

int32
main(int32 argc, char **argv)
{
    int32 opt;
    int32 daemon_mode = 0;

    while ((opt = getopt(argc, argv, "dh")) != -1)
    {
        switch (opt)
        {
        case 'd':
            daemon_mode = 1;
            break;
        case 'h':
        default:
            usage();
            exit(1);
            break;
        }
    }

    hsrv_start(daemon_mode);
    exit(0);
}


