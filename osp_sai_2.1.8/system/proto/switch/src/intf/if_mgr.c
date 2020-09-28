
#include "proto.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"
#include "lib_tblinc.h"
#include "lib_sysctl.h"
#include "ipc.h"
#include "switch.h"
#include "mirror.h"
#include "if_mgr.h"
#include "if_agg.h"
#include "brg_if.h"
#include "brg_fdb.h"
#include "swth_sys.h"
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "switch_api.h"
#include "lib_ioctl.h"
#include "lib_netlink.h"
#include "brg.h"
#include "errdisable.h"
#include "rt_if.h"
#include "glb_const.h"
#include "ctclib_opb.h"
#include "gen/tbl_errdisable_flap_define.h"
#include "gen/tbl_errdisable_flap.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "ipsour/ip_source.h"

#include "fei_openflow_port.h"
#include "gen/tbl_openflow_interface_define.h"
#include "gen/tbl_openflow_interface.h"
#include "gen/tbl_openflow_tunnel_interface_define.h"
#include "gen/tbl_openflow_tunnel_interface.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt_define.h"
#include "gen/tbl_openflow_tunnel_local_ip_cnt.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "sflow.h"
#include "sflow_api_header.h"
#include "lib_snmp.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
ctc_task_t *g_p_stats_rate_scan_time = NULL;
static if_stats_rate_master_info_t if_rate_db_master;

static int32
_if_mgr_running_change_process(tbl_interface_t* p_db_if);

static int32
_if_mgr_set_default_lag_if(tbl_interface_t *p_db_if)
{
    tbl_interface_t intf;
    int32 rc = PM_E_NONE;

    sal_memset(&intf, 0, sizeof(intf));
    sal_memcpy(&intf.key.name, p_db_if->key.name, IFNAME_SIZE);
    intf.ifindex = p_db_if->ifindex;
    
    /* update up/running */
    PM_E_RETURN(if_mgr_set_up(&intf, TRUE, FALSE, TRUE));
    return PM_E_NONE;
}

static int32
_if_mgr_set_default_port_if(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(brg_if_set_port_security_voilation(p_db_if, GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT, TRUE));
    PM_E_RETURN(brg_if_set_port_security_mac_maximum(p_db_if, GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM, TRUE));
    PM_E_RETURN(brg_if_set_port_security_en(p_db_if, FALSE, TRUE));

    /*set default arp inspection*/
    PM_E_RETURN(if_mgr_set_arpinsp_trust_enable(p_db_if, ARPINSP_PORT_UNTRUST, TRUE));

#ifndef TAPPRODUCT
    PM_E_RETURN(brg_if_set_mac_learning_en(p_db_if, TRUE, TRUE));
    PM_E_RETURN(brg_if_set_pvid(p_db_if, GLB_VLAN_DEFAULT_VID, FALSE));
    /* set port's PDU action */
    PM_E_RETURN(swth_sys_pdu_set_port_action(p_db_if->ifindex, GLB_IF_MODE_L2, FALSE));
    PM_E_RETURN(if_mgr_set_default_mtu(p_db_if));

    PM_E_RETURN(swth_mstp_api_add_port(p_db_if));
#endif /* TAPPRODUCT */

#ifdef TAPPRODUCT
    PM_E_RETURN(brg_if_set_mac_learning_en(p_db_if, FALSE, TRUE));
    PM_E_RETURN(if_mgr_set_mtu(p_db_if, GLB_DFT_MAX_FRAME_SIZE_1, TRUE));
    PM_E_RETURN(if_mgr_set_tap_used(p_db_if, TRUE, FALSE));
    p_db_if->vlan_type = GLB_VLAN_PORT_TYPE_TRUNK;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_VLAN_TYPE));
    p_db_if->svlan_tpid = 0x8100;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SVLAN_TPID));
#endif /* TAPPRODUCT */

/* Added by guoxd for bug 53550 2019-10-23, system should jumboframe enable default on interface */
#ifndef TAPPRODUCT
#if defined(DUET2)
    PM_E_RETURN(if_mgr_set_jumboframe_en(p_db_if, 1, TRUE));
#endif
#endif

#ifdef OPENFLOW_V580
    /* Added by weizj for tunnel bug 33473 */
    PM_E_RETURN(_if_mgr_set_default_openflow_enable(p_db_if));
    /* Added by weizj for tunnel bug 33707 */
    PM_E_RETURN(if_mgr_set_jumboframe_en(p_db_if, 1, TRUE));
#endif /* OFPRODUCT */

    return PM_E_NONE;
}

int32
if_mgr_update_oper_bandwidth(tbl_interface_t *p_db_if)
{
    tbl_interface_key_t agg_if_key;
    tbl_interface_t *p_db_if_agg = NULL;
    uint32 bandwidth = 0;
    int32 rc = PM_E_NONE;

    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        PM_E_RETURN(if_agg_aggregator_bw_update(p_db_if));
    }
    else
    {
        /* Added by kcao for bug 37840 2016-06-16, if interface has configured bandwidth, should not update oper_bandwidth */
        if (!p_db_if->bandwidth)
        {
            PM_E_RETURN(if_mgr_get_default_bandwidth(p_db_if, &bandwidth));
            p_db_if->oper_bandwidth = bandwidth;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPER_BANDWIDTH));
            /* Added by kcao for bug 41930 2016-12-19 */
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                sal_memset(&agg_if_key, 0, sizeof(agg_if_key));
                sal_snprintf(agg_if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
                p_db_if_agg = tbl_interface_get_interface(&agg_if_key);
                if (p_db_if_agg)
                {
                    if_agg_aggregator_bw_update(p_db_if_agg);
                }
            }
        }
    }

    swth_mstp_api_update_if_path_cost(p_db_if);

    return rc;
}

int32
if_mgr_set_default_mtu(tbl_interface_t *p_db_if)
{
    uint32 mtu = 0;
    
    switch (p_db_if->hw_type)
    {
    case GLB_IF_TYPE_PORT_IF:
        mtu = GLB_ETHER_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_LINKAGG_IF:
        mtu = GLB_AGG_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_VLAN_IF:
        mtu = GLB_VLAN_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_LOOPBACK_IF:
        mtu = GLB_LOOPBACK_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_TUNNEL_IF:
        mtu = GLB_TUNNEL_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_SUB_IF:
        mtu = GLB_SUB_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_L2GRE_IF:
    case GLB_IF_TYPE_L3GRE_IF:
    case GLB_IF_TYPE_NVGRE_IF:
    case GLB_IF_TYPE_VXLAN_IF:
        mtu = GLB_TUNNEL_IF_DEFAULT_MTU;
        break;
    case GLB_IF_TYPE_CPU_IF:
        mtu = GLB_CPU_IF_DEFAULT_MTU;
        break;
    default:
        mtu = 0;
        break;
    }

    return if_mgr_set_mtu(p_db_if, mtu, TRUE);
}

int32
if_mgr_add_dhclient_ds(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    ds_dhclient_if_t dhclient_if;

    /* init */
    sal_memset(&dhclient_if, 0, sizeof(dhclient_if));
    dhclient_if.requested_flags = M_DHC_REQ_DEFAULT;
    dhclient_if.lease = 1200;
    dhclient_if.status = 0;
    dhclient_if.option = 0;
    sal_memset(dhclient_if.hostname, 0, HOSTNAME_SIZE);

    /* modified by liwh for bug 49365, 2018-12-03 */
    dhclient_if.iana_t1 = DHCPV6_CLIENT_DEFAULT_IANA_T1_TIME;
    dhclient_if.iana_t2 = DHCPV6_CLIENT_DEFAULT_IANA_T2_TIME;
    /* liwh end */

    /* add it */
    PM_E_RETURN(ds_dhclient_if_add_dhclient_if(p_db_if, &dhclient_if));

    return PM_E_NONE;
}

/* modified by liwh for bug 50704, 2019-01-08 */
int32
if_mgr_clear_dhclient_client_id(tbl_interface_t *p_db_delete_if)
{
    int32 rc = PM_E_NONE;
    ds_dhclient_if_t *p_db_dhclient = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {      
        p_db_dhclient = p_db_if->dhclient_if;
        if (p_db_dhclient)
        {
            if ((GLB_DHCLIENT_FLAG_PORT == p_db_dhclient->client_flags)
                && (0 == sal_strcmp(p_db_delete_if->key.name, p_db_dhclient->client_id)))
            {
                p_db_dhclient->client_flags = 0;
                sal_memset(p_db_dhclient->client_id, 0x0, DHCP_NAME_SIZE);
                
                /* 3. update local CDB */
                rc = ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLIENT_FLAGS);
                rc += ds_dhclient_if_set_dhclient_if_field(p_db_if, p_db_dhclient, DS_DHCLIENT_IF_FLD_CLIENT_ID);
            }
        }
    }
    
    return rc;
}
/* liwh end */

int32
if_mgr_del_dhclient_ds(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

     /* modified by liwh for bug 50704, 2019-01-08 */
    if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        || (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type))
    {
        if_mgr_clear_dhclient_client_id(p_db_if);
    }
    /* liwh end */

    PM_E_RETURN(ds_dhclient_if_del_dhclient_if(p_db_if));

    return PM_E_NONE;
}

int32
if_mgr_del_port_if(tbl_interface_t *p_db_if)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_lag = NULL;
    int32 rc = PM_E_NONE;
    tbl_port_stats_rate_key_t port_stats_rate_key;
    tbl_port_stats_rate_t *p_port_stats_rate = NULL;
    if_port_rate_info_t lkp;
    if_port_rate_info_t *p_db_stats_info = NULL;

//    acl_mgr_del_intf_handle(p_db_if);
    
    PM_E_RETURN(ds_brgif_del_brgif(p_db_if));
    PM_E_RETURN(if_mgr_del_dhclient_ds(p_db_if));
    //PM_E_RETURN(fei_if_destroy_port_if(p_db_if->ifindex));
    if (p_db_if->lacp)
    {
        PM_E_RETURN(lacp_del_if_lacp(p_db_if));
    }
    if (p_db_if->lag)
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_db_if->lag->lag_id);
        p_db_if_lag = tbl_interface_get_interface(&if_key);
        PM_E_RETURN(if_mgr_del_if_from_lag(p_db_if_lag, p_db_if));
    }
    CTC_TASK_STOP_TIMER(p_db_if->errdisable_timer);

    lkp.key.ifindex = p_db_if->ifindex;
    p_db_stats_info = ctclib_hash_lookup(if_rate_db_master.if_stats_hash, &lkp);
    if (p_db_stats_info)
    {
        ctclib_hash_release(if_rate_db_master.if_stats_hash, (void*)p_db_stats_info);
        XFREE(MEM_TBL_PORT_STATS_RATE, p_db_stats_info);
    }
    
    port_stats_rate_key.ifindex = p_db_if->ifindex;
    p_port_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
    if (p_port_stats_rate)
    {
        PM_E_RETURN(tbl_port_stats_rate_del_port_stats_rate(&port_stats_rate_key));
    }
    PM_E_RETURN(tbl_interface_del_interface(&p_db_if->key));
    return PM_E_NONE;
}

/* modified by liwh for bug 51233, 2019-03-12 */
int32
if_mgr_set_dhcp_client_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->dhcp_client_enable= enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCP_CLIENT_ENABLE));
    return rc;
}

int32
if_mgr_set_dhcpv6_client_en(tbl_route_if_t *p_db_rtif, uint8 enable)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    p_db_rtif->dhcpv6_client_enable= enable;
    PM_E_RETURN(tbl_route_if_set_route_if_field(p_db_rtif, TBL_ROUTE_IF_FLD_DHCPV6_CLIENT_ENABLE));
    return rc;
}
/*liwh end */

int32
if_mgr_del_vlan_if(tbl_interface_t *p_db_if)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    uint32 vlanid = 0;
    tbl_aclqos_if_key_t aclqos_if_key;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 50219, 2018-12-19 */
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;
    char                sz_fullcmd[GLB_MAX_CMD_BUFSZ] = {0};
    /* liwh end */

    sal_memset(&aclqos_if_key, 0x0, sizeof(tbl_aclqos_if_key_t));
    sal_strcpy(aclqos_if_key.name, p_db_if->key.name);
    sal_sscanf(p_db_if->key.name, "vlan%d", &vlanid);

#ifdef CONFIG_MIRROR
    mirror_del_vlan_process(vlanid, TRUE);
#endif /* CONFIG_MIRROR */

    /* modified by liwh for bug 50219, 2018-12-19 */
    sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
    sal_strncpy(rtif.key.name, p_db_if->key.name, IFNAME_SIZE);
    p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
    if (p_db_rtif)
    {
        if  (p_db_rtif->dhcp_client_enable)
        {
            snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcp_l3if_pre_delete/%s", 
                p_db_if->key.name);
            sal_system(sz_fullcmd);

            if_mgr_set_dhcp_client_en(p_db_rtif, FALSE);
        }

        if (p_db_rtif->dhcpv6_client_enable)
        {
            snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcpv6_l3if_pre_delete/%s", 
                p_db_if->key.name);
            sal_system(sz_fullcmd);

            if_mgr_set_dhcpv6_client_en(p_db_rtif, FALSE);
        }
    }
    /* liwh end */

//    acl_mgr_del_intf_handle(p_db_if);

    PM_E_RETURN(if_mgr_del_dhclient_ds(p_db_if));

    p_sys_glb->curr_vlanif_count--;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT);

    PM_E_RETURN(tbl_interface_del_interface(&p_db_if->key));

    /* added by hansf for qos, tbl_aclqos_if should be deleted when vlan if deleted */
    PM_E_RETURN(tbl_aclqos_if_del_aclqos_if(&aclqos_if_key));
    /* ended by hansf */
    
    return PM_E_NONE;
}

/*add by jqiu for bug 32103 20150304*/
static int32
_if_mgr_set_phyport_default_speed_duplex(tbl_interface_t *p_if)
{
    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type)
    {
        return PM_E_NONE;
    }
    
    if (GLB_PHY_PORT_IS_100G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_FULL;
        p_if->oper_duplex = GLB_DUPLEX_FULL;
    }
    else if (GLB_PHY_PORT_IS_40G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_40G;
        p_if->oper_speed = GLB_SPEED_40G;
        p_if->duplex = GLB_DUPLEX_FULL;
        p_if->oper_duplex = GLB_DUPLEX_FULL;
    }
    else if (GLB_PHY_PORT_IS_10G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_AUTO;
        p_if->oper_duplex = GLB_DUPLEX_AUTO;
    }
    else if (GLB_PHY_PORT_IS_5G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_AUTO;
        p_if->oper_duplex = GLB_DUPLEX_AUTO;
    }
    else if (GLB_PHY_PORT_IS_2_5G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_AUTO;
        p_if->oper_duplex = GLB_DUPLEX_AUTO;
    }
    else if (GLB_PHY_PORT_IS_1G(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_AUTO;
        p_if->oper_duplex = GLB_DUPLEX_AUTO;
    }
    else if (GLB_PHY_PORT_IS_100M(p_if->support_speed_flag))
    {
        p_if->speed = GLB_SPEED_100M;
        p_if->oper_speed = GLB_SPEED_100M;
        p_if->duplex = GLB_DUPLEX_FULL;
        p_if->oper_duplex = GLB_DUPLEX_FULL;
    }
    else
    {
        p_if->speed = GLB_SPEED_AUTO;
        p_if->oper_speed = GLB_SPEED_AUTO;
        p_if->duplex = GLB_DUPLEX_AUTO;
        p_if->oper_duplex = GLB_DUPLEX_AUTO;
    }

    return PM_E_NONE;
}

int32
if_mgr_add_port_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if, cfg_cmd_para_t *para)
{
    mac_addr_t mac;
    uint32 portid = 0;
    uint32 bandwidth = 0;
    uint32 phyport_flag = 0;
    uint32 support_speed_flag = 0;
    ds_brgif_t brgif;
    tbl_interface_t *p_db_if = NULL;
    tbl_port_stats_rate_t port_stats_rate;
    tbl_openflow_interface_t openflow_if;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    int32 rc = PM_E_NONE;
    uint32 index = 0;

    PM_CFG_GET_INTEGER(portid, argv[2], para->p_rs);
    CFG_PARSER_MAC(mac, argv[4], para->p_rs);
    PM_CFG_GET_INTEGER(p_if->phy_type, argv[6], para->p_rs);
    PM_CFG_GET_INTEGER(phyport_flag, argv[8], para->p_rs);
    PM_CFG_GET_INTEGER(support_speed_flag, argv[10], para->p_rs);
    p_if->hw_type = GLB_IF_TYPE_PORT_IF;
    p_if->mode = GLB_IF_MODE_L2;
    p_if->vlan_type = GLB_VLAN_PORT_TYPE_ACCESS;
    sal_memcpy(p_if->mac_addr, mac, sizeof(mac));
    sal_memcpy(p_if->hw_mac_addr, mac, sizeof(mac));
    p_if->crc_error_check_en = TRUE;
    p_if->crc_recalculation_en = TRUE;
    p_if->portid = ((uint64)GLB_IF_SAI_OBJECT_TYPE_PORT<<32) + portid;
    p_if->phyport_flag = phyport_flag;
    p_if->support_speed_flag = support_speed_flag;
    //p_db_if->phy_loopback_mode = GLB_LB_NONE;
    _if_mgr_set_phyport_default_speed_duplex(p_if);
    if_mgr_get_default_bandwidth(p_if, &bandwidth);
    p_if->bandwidth = 0;
    p_if->fec = GLB_FEC_DEFAULT|GLB_FEC_ALL;/* default fec */
    p_if->oper_bandwidth = bandwidth;
    p_if->support_ssm_en = TRUE;
    p_if->svlan_tpid = p_brg_global->vlan_tpid_type1;
    /* added by hansf for flow-statistics default include-interframe */
    p_if->ifg_stats_en = GLB_DEF_IFG_STATS_EN;
    /* added default poe status */
    p_if->poe.enable = DEFAULT_POE_ENALBE;
    p_if->poe.mode = POE_AT;
    p_if->poe.priority = POE_LOW;

    PM_E_RETURN(tbl_interface_add_interface(p_if));

    p_brg_global->tpid_type1_ref_cnt++;
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT));

    sal_memset(&openflow_if, 0x0, sizeof(openflow_if));
    sal_strncpy(openflow_if.key.name, p_if->key.name, IFNAME_SIZE);
    openflow_if.obey_vlan_filter = 1;
    openflow_if.bind_tunnel_type = GLB_IF_TYPE_VXLAN_IF;
    openflow_if.ifindex = p_if->ifindex;
    for(index = 0; index < OPENFLOW_PROTECTED_VLAN; index ++)
    {
        openflow_if.protected_vlan[index ] = 0;
    }
    PM_E_RETURN(tbl_openflow_interface_add_openflow_interface(&openflow_if));
    /* need to check by wangjj for openflow packet, 2017-02-27 */
#ifdef OFPRODUCT
    if_mgr_set_opf_intf_pkt_default(&openflow_if);
    tbl_openflow_interface_set_openflow_interface_field(&openflow_if, TBL_OPENFLOW_INTERFACE_FLD_ACTION);
#endif
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(if_mgr_add_dhclient_ds(p_db_if));

    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.ingress_filter = TRUE;
    brgif.egress_filter = TRUE;
    brgif.tpid_type = GLB_TPID_TYPE_1;
    PM_E_RETURN(ds_brgif_add_brgif(p_db_if, &brgif));
    PM_E_RETURN(_if_mgr_set_default_port_if(p_db_if));
    
    sal_memset(&port_stats_rate, 0, sizeof(port_stats_rate));
    port_stats_rate.key.ifindex = p_if->ifindex;
    port_stats_rate.load_interval = GLB_IF_STATS_LOAD_DEF_INTVAL;
    PM_E_RETURN(tbl_port_stats_rate_add_port_stats_rate(&port_stats_rate));

    if_mgr_set_latency_event_threshold_default(p_if);
        
    return PM_E_NONE;
}

int32
if_mgr_add_vlan_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_interface_t *p_db_if = NULL;
    uint32 bandwidth = 0;
    uint32 index;
    vid_t vid = 0;
    tbl_vlan_t tbl_vlan;
    tbl_vlan_t *p_tbl_vlan;
    int32 rc = PM_E_NONE;
    tbl_aclqos_if_t aclqos_if;
    tbl_aclqos_if_t* p_aclqos_if;
    
    p_if->hw_type = GLB_IF_TYPE_VLAN_IF;
    p_if->phy_type = GLB_PHY_TYPE_UNKNOWN;
    p_if->mode = GLB_IF_MODE_L3;
    p_if->flags = GLB_IFF_UP; /*set vlaninterface admin up when creat vlan if*/
    p_if->admin_up = TRUE;
    p_if->arp_numberlimit_number = ARP_PORT_NUMBERLIMIT_NUMBER;

    sal_memcpy(&p_if->mac_addr, &p_sys_glb->route_mac, sizeof(mac_addr_t));
    if_mgr_get_default_bandwidth(p_if, &bandwidth);
    p_if->bandwidth = 0;
    p_if->oper_bandwidth = bandwidth;
    PM_E_RETURN(tbl_interface_add_interface(p_if));
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    /* added by hansf for duet2 qos, add tbl_aclqos_if when vlan if created  */
    sal_memset(&aclqos_if, 0, sizeof(tbl_aclqos_if_t));
    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(ifname);
    if(!p_aclqos_if)
    {
        sal_strcpy(aclqos_if.key.name, ifname);
        aclqos_if.ifindex = p_db_if->ifindex;
        aclqos_if.port_type = GLB_IF_TYPE_VLAN_IF;
        tbl_aclqos_if_add_aclqos_if(&aclqos_if);
    }
    /* ended by hansf */
    
    //rt_if_add_if(p_if->key.name);
    tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_MODE);
    tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_MAC_ADDR);

    rc = sal_sscanf(ifname, GLB_IFNAME_VLAN_PREFIX"%u", &index);
    vid = index;
    tbl_vlan.key.vid = vid;
    p_tbl_vlan = tbl_vlan_get_vlan(&tbl_vlan.key);
    if(p_tbl_vlan)
    {
        if_mgr_vlan_if_running_update(p_tbl_vlan);
    }

    PM_E_RETURN(if_mgr_set_default_mtu(p_db_if));

    PM_E_RETURN(if_mgr_add_dhclient_ds(p_db_if));

    p_sys_glb->curr_vlanif_count++;
    tbl_sys_global_set_sys_global_field(p_sys_glb, TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT);
    return PM_E_NONE;
}

int32
if_mgr_add_loopback_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 bandwidth = 0;
    int32 rc = PM_E_NONE;
    
    p_if->hw_type = GLB_IF_TYPE_LOOPBACK_IF;
    p_if->phy_type = GLB_PHY_TYPE_UNKNOWN;
    p_if->mode = GLB_IF_MODE_L3;
    p_if->flags = GLB_IFF_RUNNING;

    if_mgr_get_default_bandwidth(p_if, &bandwidth);
    p_if->bandwidth = 0;
    p_if->oper_bandwidth = bandwidth;
    PM_E_RETURN(tbl_interface_add_interface(p_if));
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MODE);
    PM_E_RETURN(if_mgr_set_default_mtu(p_db_if));

    /* set loopback to up */
    PM_E_RETURN(if_mgr_set_up(p_db_if, TRUE, FALSE, TRUE));

    return PM_E_NONE;
}

int32
if_mgr_add_cpu_if()
{
    tbl_interface_t cpu_if;

    sal_memset(&cpu_if, 0, sizeof(cpu_if));
    sal_strcpy(cpu_if.key.name, GLB_IFNAME_CPU_PREFIX);
    cpu_if.ifindex = tbl_interface_get_ifindex_by_name(cpu_if.key.name);
    cpu_if.portid = ((uint64)GLB_IF_SAI_OBJECT_TYPE_COPP<<32) + 1;
    cpu_if.mode = GLB_IF_MODE_OTHER;
    cpu_if.hw_type = GLB_IF_TYPE_CPU_IF;
    cpu_if.phy_type = GLB_PHY_TYPE_UNKNOWN;
    cpu_if.flags = GLB_IFF_UP | GLB_IFF_RUNNING;
    cpu_if.admin_up = TRUE;
    cpu_if.bandwidth = 0;
    cpu_if.oper_bandwidth = 0;
    tbl_interface_add_interface(&cpu_if);

    return PM_E_NONE;
}

static int32
_if_mgr_add_openflow_overlay_if(char *ifname)
{
    int32 rc = PM_E_NONE;
    tbl_openflow_tunnel_interface_t openflow_tunnel_if;
    
    sal_memset(&openflow_tunnel_if, 0x0, sizeof(tbl_openflow_tunnel_interface_t));
    sal_strncpy(openflow_tunnel_if.key.name, ifname, IFNAME_SIZE);
    PM_E_RETURN(tbl_openflow_tunnel_interface_add_openflow_tunnel_interface(&openflow_tunnel_if));
    return PM_E_NONE;
}

static int32
_if_mgr_del_openflow_overlay_if(char *ifname)
{
    int32 rc = PM_E_NONE;
    tbl_openflow_tunnel_interface_t openflow_tunnel_if;
    tbl_openflow_tunnel_interface_t* p_openflow_tunnel_if = NULL;
    tbl_openflow_tunnel_local_ip_cnt_t tunnel_local_ip;
    tbl_openflow_tunnel_local_ip_cnt_t* p_tunnel_local_ip = NULL;
    /* modified by liwh for bug 51184, 2019-03-05 */
    tbl_openflow_interface_t openflow_intf;
    tbl_openflow_interface_t *p_openflow_intf = NULL;
    /* liwh end */
    
    sal_memset(&openflow_tunnel_if, 0x0, sizeof(tbl_openflow_tunnel_interface_t));
    sal_strncpy(openflow_tunnel_if.key.name, ifname, IFNAME_SIZE);
    p_openflow_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_if.key);
    if(p_openflow_tunnel_if)
    {        
        /* modified by liwh for bug 48828, 2018-02-25 */
        sal_memset(&openflow_intf, 0, sizeof(openflow_intf));
        sal_strncpy(openflow_intf.key.name, p_openflow_tunnel_if->bind_port_name, IFNAME_SIZE);
        p_openflow_intf = tbl_openflow_interface_get_openflow_interface(&openflow_intf.key);
        if (p_openflow_intf)
        {
            p_openflow_intf->is_bind_tunnel = FALSE;
            tbl_openflow_interface_set_openflow_interface_field(p_openflow_intf, TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL);
        }
        /* liwh end */
                    
        if(GLB_FLAG_ISSET(p_openflow_tunnel_if->flag, GLB_OPENFLOW_TUNNEL_IFF_SRCIP))
        {
            sal_memset(&tunnel_local_ip, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_t));
            sal_memset(&tunnel_local_ip.key, 0x0, sizeof(tbl_openflow_tunnel_local_ip_cnt_key_t));
            sal_strncpy(tunnel_local_ip.key.local_ip, p_openflow_tunnel_if->source_ip, IPADDR_BUF_SIZE);
            p_tunnel_local_ip = tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
            if(p_tunnel_local_ip)
            {
                p_tunnel_local_ip->local_ip_ref --;
                if(0 == p_tunnel_local_ip->local_ip_ref)
                {
                    tbl_openflow_tunnel_local_ip_cnt_del_openflow_tunnel_local_ip_cnt(&tunnel_local_ip.key);
                }
            }
        }
        PM_E_RETURN(tbl_openflow_tunnel_interface_del_openflow_tunnel_interface(&openflow_tunnel_if.key));
    }

    return rc;
}

static int32
_if_mgr_add_overlay_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if, uint32 hw_type)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 bandwidth = 0;
    int32 rc = PM_E_NONE;
    
    p_if->hw_type = hw_type;
    p_if->phy_type = GLB_PHY_TYPE_UNKNOWN;
    p_if->mode = GLB_IF_MODE_L2;
    p_if->flags = GLB_IFF_UP | GLB_IFF_RUNNING;
    p_if->admin_up = TRUE;
    
    if_mgr_get_default_bandwidth(p_if, &bandwidth);
    p_if->bandwidth = 0;
    p_if->oper_bandwidth = bandwidth;
    PM_E_RETURN(tbl_interface_add_interface(p_if));
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(_if_mgr_add_openflow_overlay_if(ifname))
    PM_E_RETURN(if_mgr_set_default_mtu(p_db_if));

    return PM_E_NONE;
}

int32
if_mgr_add_l2gre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    return _if_mgr_add_overlay_if(ifname, argv, argc, p_if, GLB_IF_TYPE_L2GRE_IF);
}

int32
if_mgr_add_l3gre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    return _if_mgr_add_overlay_if(ifname, argv, argc, p_if, GLB_IF_TYPE_L3GRE_IF);
}

int32
if_mgr_add_nvgre_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    return _if_mgr_add_overlay_if(ifname, argv, argc, p_if, GLB_IF_TYPE_NVGRE_IF);
}

int32
if_mgr_add_vxlan_if(char *ifname, char **argv, int32 argc, tbl_interface_t *p_if)
{
    return _if_mgr_add_overlay_if(ifname, argv, argc, p_if, GLB_IF_TYPE_VXLAN_IF);
}

int32
if_mgr_del_overlay_if(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(_if_mgr_del_openflow_overlay_if(p_db_if->key.name));
    PM_E_RETURN(tbl_interface_del_interface(&p_db_if->key));

    return PM_E_NONE;
}

int32
if_mgr_del_loopback_if(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;

    /* set loopback to down */
    if (tbl_interface_is_up(p_db_if))
    {
        PM_E_RETURN(if_mgr_set_up(p_db_if, FALSE, FALSE, TRUE));
    }
    
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MODE);
    PM_E_RETURN(tbl_interface_del_interface(&p_db_if->key));

    return PM_E_NONE;
}

int32
if_mgr_add_lag_if(char *ifname, tbl_interface_t *p_db_if_mem, ds_lag_t *p_lag)
{
    tbl_interface_t intf;
    tbl_interface_t *p_if = &intf;
    tbl_interface_t *p_db_if = NULL;
#ifdef CONFIG_PVLAN
    tbl_pvlan_t  *p_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_key;
    tbl_pvlan_t p_temp_pvlan;
#endif /* CONFIG_PVLAN */
    int32 rc = PM_E_NONE;
    tbl_port_stats_rate_t port_stats_rate;
    tbl_lag_global_t *p_db_glb = NULL;
#ifdef OFPRODUCT
    tbl_openflow_interface_t openflow_if;
    tbl_openflow_interface_t* p_openflow_if = NULL;
#endif

    sal_memset(&intf, 0, sizeof(intf));
    sal_strncpy(intf.key.name, ifname, IFNAME_SIZE);

    IF_GET_INDEX_BY_NAME(p_if->ifindex, ifname);
    p_if->hw_type = GLB_IF_TYPE_LINKAGG_IF;
    p_if->phy_type = GLB_PHY_TYPE_UNKNOWN;
    p_if->mode = p_db_if_mem->mode;
    if (p_db_if_mem->pvlan_type)
    {
         p_if->pvlan_type = p_db_if_mem->pvlan_type;
         p_if->private_vlan = p_db_if_mem->private_vlan;
         p_if->pvlan_group_id = p_db_if_mem->pvlan_group_id;
         p_if->community_vlan = p_db_if_mem->community_vlan;
    }

    p_if->oper_bandwidth = p_db_if_mem->oper_bandwidth;
    p_if->oper_speed = p_db_if_mem->oper_speed;
    p_if->oper_duplex = p_db_if_mem->oper_duplex;
    p_if->duplex = p_db_if_mem->duplex;

    /* added by hansf for flow-statistics default include-interframe */
    p_if->ifg_stats_en = GLB_DEF_IFG_STATS_EN;

    PM_E_RETURN(tbl_interface_add_interface(p_if));

#ifdef OFPRODUCT
    sal_memset(&openflow_if, 0x0, sizeof(openflow_if));
    sal_strncpy(openflow_if.key.name, p_if->key.name, IFNAME_SIZE);

    p_openflow_if = tbl_openflow_interface_get_openflow_interface(&openflow_if.key);
    /* need to check by wangjj for openflow packet, 2017-02-27 */
    if_mgr_set_opf_intf_pkt_default(p_openflow_if);
    tbl_openflow_interface_set_openflow_interface_field(p_openflow_if, TBL_OPENFLOW_INTERFACE_FLD_ACTION);
#endif

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    logid_sys(LOG_LAG_4_ADD, ifname);
    
#ifdef CONFIG_PVLAN
    if (p_db_if_mem->pvlan_type)
    {
         pvlan_key.primary_vid = p_db_if_mem->private_vlan;
         p_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);

         if(p_pvlan == NULL)
         {
             sal_memset(&p_temp_pvlan, 0, sizeof(tbl_pvlan_t));
             p_temp_pvlan.key.primary_vid = p_db_if_mem->private_vlan;
             PM_E_RETURN(tbl_pvlan_add_pvlan(&p_temp_pvlan));
             p_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
             
         }

         if(GLB_IF_PVLAN_TYPE_COMMUNITY ==  p_if->pvlan_type )
         {
              rc = if_mgr_mapping_community_vlan_to_group(p_if, p_if->community_vlan);
              if(rc < 0)
              {
                  return rc;
              }
         }
         else if(GLB_IF_PVLAN_TYPE_ISOLATED ==  p_if->pvlan_type )
         {
             PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_pvlan, TRUE, p_db_if));
         }
         else if(GLB_IF_PVLAN_TYPE_PROMISCUOUS ==  p_if->pvlan_type )
         {
             PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_pvlan, TRUE, p_db_if));
         }
    }
#endif /* CONFIG_PVLAN */

   /*copy tbl_lag_global_t load balance mode to specific lag */
    p_db_glb = tbl_lag_global_get_lag_global();
    if (NULL == p_db_glb)
    {
        return PM_E_NOT_EXIST;
    }
    p_lag->load_balance_mode =  p_db_glb->load_balance_mode[p_lag->lag_id];
    PM_E_RETURN(ds_lag_add_lag(p_db_if, p_lag));
    PM_E_RETURN(_if_mgr_set_default_lag_if(p_db_if));
    PM_E_RETURN(if_mgr_add_dhclient_ds(p_db_if));

    sal_memset(&port_stats_rate, 0, sizeof(port_stats_rate));
    port_stats_rate.key.ifindex = p_if->ifindex;
    port_stats_rate.load_interval = GLB_IF_STATS_LOAD_DEF_INTVAL;
    PM_E_RETURN(tbl_port_stats_rate_add_port_stats_rate(&port_stats_rate));

    return rc;
}

int32
if_mgr_del_lag_if(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem)
{
#ifdef CONFIG_L2
    vid_t vid = 0;
#endif /* CONFIG_L2 */
#ifdef CONFIG_PVLAN
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_db_key;
#endif /* CONFIG_PVLAN */
    int32 rc = PM_E_NONE;
    tbl_port_stats_rate_key_t port_stats_rate_key;
    tbl_port_stats_rate_t *p_port_stats_rate = NULL;
    if_port_rate_info_t lkp;
    if_port_rate_info_t *p_db_stats_info = NULL;

//    acl_mgr_del_intf_handle(p_db_if_lag);
    swth_if_del_cb(p_db_if_lag);

    if (GLB_IF_MODE_L3 == p_db_if_lag->mode)
    {
        tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_MODE);

        /*sync arp limit to agg*/
        PM_E_RETURN(if_mgr_set_arp_numberlimit_enable(p_db_if_lag, ARP_PORT_NUMBERLIMIT_DISABLE));
        PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if_lag, ARP_PORT_NUMBERLIMIT_NUMBER));
        PM_E_RETURN(if_mgr_set_arp_numberlimit_violate(p_db_if_lag, ARP_PORT_NUMBERLIMIT_PROTECT));
    } 
    
    if (GLB_IF_MODE_L2 == p_db_if_lag->mode)
    {
        /*sync arp inspection to agg*/
        PM_E_RETURN(if_mgr_set_arpinsp_trust_enable(p_db_if_lag, ARPINSP_PORT_UNTRUST, FALSE));

        /*added by yejl for del l2protocol list*/
        if (p_db_if_lag->l2protocol.obj_list->count != 0)
        {
            PM_E_RETURN(if_mgr_del_ds_l2protocol(p_db_if_lag));
        }
    }
  
#ifdef CONFIG_L2
    if (p_db_if_lag->brgif)
    {
        /* 1. update vlan port bmp */
        GLB_BMP_ITER_BEGIN(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid)
        {
            brg_if_set_vlan_port_bmp(p_db_if_lag->ifindex, vid, FALSE);
        }
        GLB_BMP_ITER_END(p_db_if_lag->brgif->allowed_vlan.vlan_bmp, vid);
        brg_if_set_port_security_en(p_db_if_lag, FALSE, FALSE);
        brg_if_set_port_security_mac_maximum(p_db_if_lag, GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM, FALSE);
        brg_if_set_port_security_voilation(p_db_if_lag, GLB_PORT_SECURITY_VIOLATION_MODE_PROTECT, FALSE);
        ds_brgif_del_brgif(p_db_if_lag);
    }
#endif /* CONFIG_L2 */

    if (p_db_if_lag->mstp_port)
    {
        swth_mstp_api_del_port(p_db_if_lag);
    }

#ifdef CONFIG_PVLAN
    if (p_db_if_mem->pvlan_type)
    {
        pvlan_db_key.primary_vid = p_db_if_lag->private_vlan;
        p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_db_key);
        if(p_db_pvlan != NULL)
        {
            if(GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if_lag->pvlan_type)
             {
                 rc = if_mgr_unmapping_community_vlan_to_group(p_db_if_lag, p_db_if_lag->community_vlan);
             }
             else if(GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if_lag->pvlan_type)
             {
                 PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_db_pvlan, FALSE, p_db_if_lag));
             }
             else if(GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_db_if_lag->pvlan_type)
             {
                 PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_db_pvlan, FALSE, p_db_if_lag));
             }
     
             if((p_db_pvlan->community_groups.obj_list->count == 0) 
             && (p_db_pvlan->isloate_ports.obj_list->count == 0)
             && (p_db_pvlan->promiscuous_ports.obj_list->count == 0))
             {
                 PM_E_RETURN(tbl_pvlan_del_pvlan(&pvlan_db_key));
             }
     
         }
    }
#endif /* CONFIG_PVLAN */
    
    CTC_TASK_STOP_TIMER(p_db_if_lag->errdisable_timer);

    lkp.key.ifindex = p_db_if_lag->ifindex;
    p_db_stats_info = ctclib_hash_lookup(if_rate_db_master.if_stats_hash, &lkp);
    if (p_db_stats_info)
    {
        ctclib_hash_release(if_rate_db_master.if_stats_hash, (void*)p_db_stats_info);
        XFREE(MEM_TBL_PORT_STATS_RATE, p_db_stats_info);
    }

    port_stats_rate_key.ifindex = p_db_if_lag->ifindex;
    p_port_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
    if(p_port_stats_rate)
    {
        PM_E_RETURN(tbl_port_stats_rate_del_port_stats_rate(&port_stats_rate_key));
    }

    logid_sys(LOG_LAG_4_DEL, p_db_if_lag->key.name);
    
    PM_E_RETURN(ds_lag_del_lag(p_db_if_lag));
    PM_E_RETURN(if_mgr_del_dhclient_ds(p_db_if_lag));
    PM_E_RETURN(tbl_interface_del_interface(&p_db_if_lag->key));
    
    return rc;
}

int32
if_mgr_add_if_to_lag(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem, ds_lag_t *p_lag)
{
    ds_lag_t *p_db_lag = NULL;
    uint32 is_up = FALSE;
    uint32 is_dynamic = FALSE;
    uint32 proto_index = 0;
    int32 rc = PM_E_NONE;

    if (NULL != p_db_if_mem->lag)
    {
        SAL_ASSERT(0);
    }

    is_dynamic = (GLB_AGG_MODE_STATIC != p_lag->mode) ? TRUE : FALSE;

    /* If aggregator not up, admin down the member port. */
    is_up = tbl_interface_is_up(p_db_if_lag);
    if (!is_up)
    {
        if_mgr_set_up(p_db_if_mem, FALSE, FALSE, TRUE);
    }

#ifdef CONFIG_L2
    PM_E_RETURN(brg_fdb_flush_l2mc_fdb_port(p_db_if_mem->ifindex));
    PM_E_RETURN(brg_fdb_flush_static_fdb_port(p_db_if_mem->ifindex));
    PM_E_RETURN(brg_psfdb_flush_static_fdb_port(p_db_if_mem->ifindex));
#endif /* CONFIG_L2 */

/*added by yejl for Tsingma hash, 2019-06-27*/
#ifdef TSINGMA
    if (p_db_if_mem->hash_value_profile_id != 0)
    {
        PM_E_RETURN(hash_value_profile_unlock(p_db_if_mem->hash_value_profile_id));
        p_db_if_mem->hash_value_profile_id = 0;
        p_db_if_mem->hash_value_profile_dir = GLB_INGRESS;
        p_db_if_mem->hash_value_agg_hash = 0;
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR));
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID));
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_HASH_VALUE_AGG_HASH));
    }
#endif

    p_db_lag = p_db_if_lag->lag;
    PM_E_RETURN(ds_lag_set_member_ports(p_db_if_lag, p_db_lag, TRUE, p_db_if_mem));
    PM_E_RETURN(ds_lag_add_lag(p_db_if_mem, p_lag));    // should uses p_lag for active mode is different
    
    //PM_E_RETURN(fei_agg_add_member_port(p_db_if_lag->ifindex, p_db_if_mem->ifindex));

    PM_E_RETURN(if_mgr_lag_if_running_update(p_db_if_lag));

    if (is_dynamic)
    {
        PM_E_RETURN(lacp_add_if_lacp(p_db_if_lag, p_db_if_mem));
    }
    else
    {
        PM_E_RETURN(if_agg_attach_mux_to_aggregator(p_db_if_lag, p_db_if_mem));
    }
    
    #ifdef CONFIG_L2
    /* Added by kcao 2014-10-29 for bug 30429, should flush FDB after hsrv's mem_port is set IF_PORT_FLAG_AGGREGATED */
    PM_E_RETURN(brg_fdb_flush_dynamic_fdb_port(p_db_if_mem->ifindex));

    /*Added by yejl for l2protocol*/
    if (GLB_IF_MODE_L2 == p_db_if_mem->mode)
    {
        if (p_db_if_mem->l2pro_uplink)
        {
            p_db_if_mem->l2pro_uplink = FALSE;
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_L2PRO_UPLINK));
        }
        if (p_db_if_mem->l2protocol.obj_list->count != 0)
        {
            PM_E_RETURN(if_mgr_del_ds_l2protocol(p_db_if_mem));
            for (proto_index = 0; proto_index < GLB_L2_PROTO_INDEX_MAX; proto_index++)
            {
                p_db_if_mem->l2pro_if_action_vlan.l2action[proto_index] = GLB_L2_PROTO_PEER; 
                p_db_if_mem->l2pro_if_action_vlan.l2vlan[proto_index] = GLB_VLAN_NONE; 
            }
            PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
        }
    }
    #endif /* CONFIG_L2 */

    /* added by hansf for set lag member ifg_stats_en attribute inherit from lag interface */
    p_db_if_mem->ifg_stats_en = p_db_if_lag->ifg_stats_en;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_IFG_STATS_EN));
    
    return rc;
}

int32
if_mgr_del_if_from_lag(tbl_interface_t *p_db_if_lag, tbl_interface_t *p_db_if_mem)
{
    ds_lag_t *p_db_lag = NULL;
    uint32 is_dynamic = FALSE;
    int32 rc = PM_E_NONE;

    if (NULL == p_db_if_mem->lag)
    {
        SAL_ASSERT(0);
    }
#ifdef TSINGMA
    if (p_db_if_lag->hash_value_profile_id)
    {
        p_db_if_mem->hash_value_profile_id = 0;
        p_db_if_mem->hash_value_profile_dir = GLB_INGRESS;
        PM_E_RETURN(if_mgr_set_hash_value_profile_select(p_db_if_mem));
    }
#endif


#ifdef CONFIG_ERRDISABLE
    PM_E_RETURN(errdisable_lacp_inherit_parent(p_db_if_lag, p_db_if_mem));
#endif /* CONFIG_ERRDISABLE */

    is_dynamic = (GLB_AGG_MODE_STATIC != p_db_if_mem->lag->mode) ? TRUE : FALSE;
    /* Modified by kcao for bug 30868, 2015-01-28, lacp should also remove from Chip */
    PM_E_RETURN(if_agg_detach_mux_from_aggregator(p_db_if_lag, p_db_if_mem));
    if (is_dynamic)
    {
        PM_E_RETURN(lacp_del_if_lacp(p_db_if_mem));
    }
    
    p_db_lag = p_db_if_lag->lag;
    //PM_E_RETURN(fei_agg_del_member_port(p_db_if_lag->ifindex, p_db_if_mem->ifindex));
    PM_E_RETURN(ds_lag_del_lag(p_db_if_mem));
    PM_E_RETURN(ds_lag_set_member_ports(p_db_if_lag, p_db_lag, FALSE, p_db_if_mem));
    PM_E_RETURN(if_mgr_lag_if_running_update(p_db_if_lag));
    if(p_db_if_lag->bandwidth_may_change)
    {
        p_db_if_lag->bandwidth_may_change = 0;
    }
    else
    {
        p_db_if_lag->bandwidth_may_change = 1;
    }
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_lag, TBL_INTERFACE_FLD_BANDWIDTH_MAY_CHANGE));

    /* added by hansf for restore interface ifg_stats_en to default */
    p_db_if_mem->ifg_stats_en = GLB_DEF_IFG_STATS_EN;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if_mem, TBL_INTERFACE_FLD_IFG_STATS_EN));
    
    return rc;
}

int32
if_mgr_set_desc(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_DESC));
    return rc;
}

int32
if_mgr_set_svlan_tpid(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_SVLAN_TPID));
    return rc;
}

int32
if_mgr_set_dot1q_ethertype(tbl_interface_t *p_db_if, uint32 tpid_value, 
        cfg_cmd_para_t *para, uint32 iter_member)
{
    ds_lag_t   *p_lag = NULL;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    char    ifname_ext[IFNAME_EXT_SIZE];
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        p_db_if->svlan_tpid = tpid_value;
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SVLAN_TPID));
        return PM_E_NONE;
    }

    /* the svlan tpid before set must be one of vlan_tpid_type1 and vlan_tpid_type2 */
    if (p_db_if->svlan_tpid == p_brg_global->vlan_tpid_type1)
    {
        if (tpid_value == p_brg_global->vlan_tpid_type2)
        {
            if (p_brg_global->tpid_type1_ref_cnt > 0)
            {
                p_brg_global->tpid_type1_ref_cnt--;
            }
            p_brg_global->tpid_type2_ref_cnt++;
            p_db_if->svlan_tpid = tpid_value;
        }
        /* tpid_value neither equal to vlan_tpid_type1 nor vlan_tpid_type2 */
        else
        {
            if (p_brg_global->tpid_type1_ref_cnt == 1)
            {
                p_brg_global->vlan_tpid_type1 = tpid_value;
                p_db_if->svlan_tpid = tpid_value;
                PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1));
            }
            else if (!p_brg_global->tpid_type2_ref_cnt)
            {
                if (p_brg_global->tpid_type1_ref_cnt > 0)
                {
                    p_brg_global->tpid_type1_ref_cnt--;
                }
                p_brg_global->tpid_type2_ref_cnt++;
                p_brg_global->vlan_tpid_type2 = tpid_value;
                PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2));
                p_db_if->svlan_tpid = tpid_value;
            }
            else
            {
                CFG_CONFLICT_RET("Can not set ethernet-type on port %s, because different ethernet-type 0x%04x and 0x%04x have been set on other port(s)",
                    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_brg_global->vlan_tpid_type1, p_brg_global->vlan_tpid_type2);
            }
        }
    }
    else if (p_db_if->svlan_tpid == p_brg_global->vlan_tpid_type2)
    {
        if (tpid_value == p_brg_global->vlan_tpid_type1)
        {
            if (p_brg_global->tpid_type2_ref_cnt > 0)
            {
                p_brg_global->tpid_type2_ref_cnt--;
            }
            p_brg_global->tpid_type1_ref_cnt++;
            p_db_if->svlan_tpid = tpid_value;
        }
        /* tpid_value neither equal to vlan_tpid_type1 nor vlan_tpid_type2 */
        else
        {
            if (p_brg_global->tpid_type2_ref_cnt == 1)
            {
                p_brg_global->vlan_tpid_type2 = tpid_value;
                p_db_if->svlan_tpid = tpid_value;
                PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2));
            }
            else if (!p_brg_global->tpid_type1_ref_cnt)
            {
                if (p_brg_global->tpid_type2_ref_cnt > 0)
                {
                    p_brg_global->tpid_type2_ref_cnt--;
                }
                p_brg_global->tpid_type1_ref_cnt++;
                p_brg_global->vlan_tpid_type1 = tpid_value;
                PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1));
                p_db_if->svlan_tpid = tpid_value;
            }
            else
            {
                CFG_CONFLICT_RET("Can not set ethernet-type on port %s, because different ethernet-type 0x%04x and 0x%04x have been set on other port(s)",
                    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_brg_global->vlan_tpid_type1, p_brg_global->vlan_tpid_type2);
            }
        }
    }

    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT));
    PM_E_RETURN(tbl_brg_global_set_brg_global_field(p_brg_global, TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT));
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SVLAN_TPID));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_dot1q_ethertype(p_if_mem, tpid_value, para, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

int32
if_mgr_set_speed(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* Deleted by liuht for bug43905, oper_speed should be updated by fea with real speed */
    /* 1. process */
    //p_if->oper_speed = p_if->speed;
        
    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_SPEED));
    //PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_SPEED));

    PM_E_RETURN(if_mgr_update_oper_bandwidth(p_db_if));        
    return rc;
}

int32
if_mgr_set_duplex(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* Deleted by liuht for bug43905, oper_duplex should be updated by fea with real duplex */
    /* 1. process */
    //p_if->oper_duplex = p_if->duplex;
    
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_DUPLEX));
    //PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_DUPLEX));
    return rc;
}

int32
if_mgr_set_loopback(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    p_if->oper_loopback = p_if->phy_loopback_mode;
    
    /* 2. update local CDB */    
    /* for bug 45108, change the order of set oper,TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX need to be first */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_INTERNAL_LP_IFINDEX));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PHY_LOOPBACK_MODE));
    //PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PHYPORT_LOOPBACK));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_LOOPBACK));

    return rc;
}

int32
if_mgr_set_metadata(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_METADATA_ENABLE));

    return rc;
}

int32
if_mgr_set_media(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
        
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_MEDIA));

    return rc;
}

int32
if_mgr_set_unidir(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    p_if->oper_unidir = p_if->unidir;
    
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_UNIDIR));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_UNIDIR));
    return rc;
}

int32
if_mgr_set_fec(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    p_if->oper_fec = p_if->fec;
    
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_FEC));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_FEC));
    return rc;
}

int32
if_mgr_set_poe(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    
    /* 2. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_POE));
    return rc;
}

int32
if_mgr_set_reflective(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. update local CDB */    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_REFLECTIVE_BRIDGE_EN));
    return rc;
}

int32
if_mgr_set_flowcontrol_send(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_SEND))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND);
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_FLOWCONTROL_SEND));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND));
    
    return rc;
}

int32
if_mgr_set_flowcontrol_receive(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    if (GLB_FLAG_ISSET(p_if->flowcontrol, GLB_IF_FLOW_CONTROL_RECEIVE))
    {
        GLB_SET_FLAG(p_if->flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE);
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_FLOWCONTROL_RECEIVE));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE));
    
    return rc;
}

int32
if_mgr_set_pfc_en(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *p_db_if = NULL;
    
    /*lookup the interface exist*/
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    /* 2. process */ 
    p_if->pfc_oper =  p_if->pfc_admin;
    
    /* 3. update local CDB */ 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PFC_ADMIN));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PFC_OPER));
    
    return rc;
}

int32
if_mgr_set_pfc_priority(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
     /*lookup the interface exist*/
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PFC_PRIORITY));
    
    return rc;
}

int32
if_mgr_set_jumboframe_en(tbl_interface_t *p_if, uint32 jumboframe_en, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* 2. update local CDB */
    p_if->jumboframe_en = jumboframe_en;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_JUMBOFRAME_EN));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_jumboframe_en(p_if_mem, jumboframe_en, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}

static int32
_if_mgr_isolate_group_check_add(tbl_interface_t *p_db_if, uint32 group_id)
{
    tbl_isolation_t iso;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&iso, 0, sizeof(iso));
    iso.key.isolation_id = group_id;
    p_db_iso = tbl_isolation_get_isolation(&iso.key);
    if (NULL == p_db_iso)
    {
        PM_E_RETURN(tbl_isolation_add_isolation(&iso));
    }

    PM_E_RETURN(tbl_isolation_set_ports(&iso, TRUE, p_db_if));

    return PM_E_NONE;
}

static int32
_if_mgr_isolate_group_check_del(tbl_interface_t *p_db_if, uint32 group_id)
{
    tbl_isolation_t iso;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&iso, 0, sizeof(iso));
    iso.key.isolation_id = group_id;
    p_db_iso = tbl_isolation_get_isolation(&iso.key);
    if (NULL == p_db_iso)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(tbl_isolation_set_ports(&iso, FALSE, p_db_if));

    return PM_E_NONE;
}

static int32
_if_mgr_isolate_group_check_del_group(uint32 group_id)
{
    tbl_isolation_t iso;
    tbl_isolation_t *p_db_iso = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&iso, 0, sizeof(iso));
    iso.key.isolation_id = group_id;
    p_db_iso = tbl_isolation_get_isolation(&iso.key);
    if (NULL == p_db_iso)
    {
        return PM_E_FAIL;
    }

    /* if new_group_id is 0, should delete after all ports removed */
    if (CTCLIB_SLIST_ISEMPTY(p_db_iso->ports.obj_list))
    {
        PM_E_RETURN(tbl_isolation_del_isolation(&iso.key));
    }

    return PM_E_NONE;
}

int32
if_mgr_isolate_group_process(tbl_interface_t *p_db_if, uint32 new_group_id)
{
    uint32 old_group_id = 0;
    
    old_group_id = p_db_if->isolate_group_id;

    if (old_group_id == new_group_id)
    {
        return PM_E_NONE;
    }

    if (old_group_id)
    {
        _if_mgr_isolate_group_check_del(p_db_if, old_group_id);
    }
    
    if (new_group_id)
    {
        _if_mgr_isolate_group_check_add(p_db_if, new_group_id);
    }

    return PM_E_NONE;
}

int32
if_mgr_set_isolate_group_id(tbl_interface_t *p_if, uint32 isolate_group_id, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    uint32 old_isolate_group_id = 0;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    old_isolate_group_id = p_db_if->isolate_group_id;
    
    /* 2. process */
    if_mgr_isolate_group_process(p_db_if, isolate_group_id);

    /* 3. update local CDB */
    p_if->isolate_group_id = isolate_group_id;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ISOLATE_GROUP_ID));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_isolate_group_id(p_if_mem, isolate_group_id, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    /* 4. remove old group */
    if (old_isolate_group_id)
    {
        _if_mgr_isolate_group_check_del_group(old_isolate_group_id);
    }

    return rc;
}

#ifdef CONFIG_PVLAN

int32
if_mgr_init_pvlan_handle(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    int32 rc = PM_E_NONE;
    vid_t private_vlan = *(vid_t*)pargs->argv[0];
    uint32 pvlan_type = 0;
    
    if (p_db_if->private_vlan == private_vlan)
    {
        if((p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_ISOLATED) ||(p_db_if->pvlan_type == GLB_IF_PVLAN_TYPE_COMMUNITY))
        {
             pvlan_type = GLB_IF_PVLAN_TYPE_ISOLATED;
        }
        else
        {
             pvlan_type = GLB_IF_PVLAN_TYPE_PROMISCUOUS; 
        } 
     
        rc = if_mgr_set_pvlan_mode(p_db_if, pvlan_type, TRUE);
    }      
    return rc;
}

int32
if_mgr_init_trunk_untag_handle(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    int32 rc = PM_E_NONE;
    vid_t vid = *(vid_t*)pargs->argv[0];
    ds_brgif_t *p_db_brgif = p_db_if->brgif;   

    if (NULL == p_db_brgif)
    {
        return PM_E_NONE;
    }

    if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
    {
        return PM_E_NONE;
    }

    if (!GLB_BMP_ISSET(p_db_brgif->untagged_vlan.vlan_bmp, vid))
    {
         return PM_E_NONE;       
    }

    GLB_BMP_UNSET(p_db_brgif->untagged_vlan.vlan_bmp, vid);
    PM_E_RETURN(brg_if_set_untagged_vlan_db(p_db_if, p_db_brgif, TRUE));
    
    return rc;
}

int32
if_mgr_set_pvlan_primary_vlan(tbl_interface_t *p_if, vid_t primary_vlan)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t* p_db_vlan = NULL;
    int32 rc = PM_E_NONE;
    
    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NO_MEMORY;
    }
    
    /* 2. lookup vlan exist */
    vlan_key.vid = primary_vlan;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. clear old properties */
    PM_E_RETURN(brg_if_set_vlan_type_clear(p_db_if));
    
    /* 4. update local CDB */ 
    p_if->vlan_type = GLB_VLAN_PORT_TYPE_ACCESS;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLAN_TYPE));
    
    /* 5. set primarity vlan */
    PM_E_RETURN(brg_if_set_pvid(p_db_if, primary_vlan, FALSE));
    
    return PM_E_NONE;
}

int32
if_mgr_set_pvlan_secodary_vlan(tbl_interface_t *p_if, uint32 pvlan_type, vid_t primary_vlan, vid_t comunity_vlan)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_t  *p_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_db_key;
    tbl_pvlan_key_t  pvlan_key;
    tbl_pvlan_t  p_temp_pvlan;
    int32 rc = PM_E_NONE;
    
    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* 2. config check, if the private_vlan is exist, delete the configuration first*/
    sal_memset(&pvlan_db_key,0,sizeof(tbl_pvlan_key_t));
    pvlan_db_key.primary_vid = p_db_if->private_vlan;
    p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_db_key);
    if (NULL != p_db_pvlan)
    {
        if (GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
        {
            rc = if_mgr_unmapping_community_vlan_to_group(p_db_if, p_db_if->community_vlan);
        }
        else if (GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if->pvlan_type)
        {
            PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_db_pvlan, FALSE, p_db_if));
        }
        else if (GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_db_if->pvlan_type)
        {
            PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_db_pvlan, FALSE, p_db_if));
        }
        
        if((p_db_pvlan->community_groups.obj_list->count == 0) 
        && (p_db_pvlan->isloate_ports.obj_list->count == 0)
        && (p_db_pvlan->promiscuous_ports.obj_list->count == 0))
        {
            PM_E_RETURN(tbl_pvlan_del_pvlan(&pvlan_db_key));
        }  
    }

    /* 3. if the pvlan_type is not none, get the private vlan*/
    if (pvlan_type)
    {
        p_if->pvlan_type = pvlan_type;
        p_if->private_vlan = primary_vlan;
        p_if->community_vlan = comunity_vlan;
        p_if->pvlan_group_id = rc;
        
        sal_memset(&pvlan_key,0,sizeof(tbl_pvlan_key_t));
        pvlan_key.primary_vid = primary_vlan;
        p_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
        if (p_pvlan == NULL)
        {
            sal_memset(&p_temp_pvlan, 0, sizeof(tbl_pvlan_t));
            p_temp_pvlan.key.primary_vid = primary_vlan;
            PM_E_RETURN(tbl_pvlan_add_pvlan(&p_temp_pvlan));
            p_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
        }
    }
    else
    {
        p_if->pvlan_type = GLB_IF_PVLAN_TYPE_NONE;
        p_if->private_vlan = 0;
        p_if->community_vlan = 0;
        p_if->pvlan_group_id = 0;
    }

    /* 4. map secondary vlan to group*/
    if (GLB_IF_PVLAN_TYPE_COMMUNITY == pvlan_type)
    {
         rc = if_mgr_mapping_community_vlan_to_group(p_if, comunity_vlan);
         if(rc < 0)
         {
             return rc;
         }
         p_if->pvlan_group_id = rc;
    }
    else if (GLB_IF_PVLAN_TYPE_ISOLATED == pvlan_type)
    {
        PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_pvlan, TRUE, p_db_if));
    }
    else if (GLB_IF_PVLAN_TYPE_PROMISCUOUS == pvlan_type)
    {
        PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_pvlan, TRUE, p_db_if));
    }

    /* 5. update local CDB */  
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_TYPE));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PRIVATE_VLAN));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_COMMUNITY_VLAN));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_GROUP_ID));

    return rc;
}


int32
if_mgr_set_pvlan_mode(tbl_interface_t *p_if, uint32 pvlan_type, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    /*set primary vlan and secondary vlan*/
    PM_E_RETURN(if_mgr_set_pvlan_primary_vlan(p_if, GLB_VLAN_DEFAULT_VID));
    PM_E_RETURN(if_mgr_set_pvlan_secodary_vlan(p_if, pvlan_type, GLB_VLAN_DEFAULT_VID, 0));

    /*judge agg port*/
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_pvlan_mode(p_if_mem, pvlan_type, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
if_mgr_set_pvlan(tbl_interface_t *p_if, uint32 pvlan_type, vid_t primary_vlan, vid_t community_vlan, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ds_lag_t *p_lag = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /*set primary vlan and secondary vlan*/
    PM_E_RETURN(if_mgr_set_pvlan_primary_vlan(p_if, primary_vlan));
    PM_E_RETURN(if_mgr_set_pvlan_secodary_vlan(p_if, pvlan_type, primary_vlan, community_vlan));

    /*judge agg port*/
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_pvlan(p_if_mem, pvlan_type, primary_vlan, community_vlan, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
if_mgr_mapping_community_vlan_to_group(tbl_interface_t *p_if, vid_t community_vlan)
{
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_t  p_pvlan;
    tbl_pvlan_key_t  pvlan_key;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    
    pvlan_key.primary_vid = p_if->private_vlan;
    p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
    if(p_db_pvlan == NULL)
    {
        sal_memset(&p_pvlan, 0, sizeof(tbl_pvlan_t));
        p_pvlan.key.primary_vid = p_if->private_vlan;
        PM_E_RETURN(tbl_pvlan_add_pvlan(&p_pvlan));
        rc = brg_alloc_pvlan_group_id(&p_pvlan, p_if, community_vlan);
      
    }
    else
    {
        rc = brg_alloc_pvlan_group_id(p_db_pvlan, p_if, community_vlan);
    }
    return rc;
}

int32
if_mgr_unmapping_community_vlan_to_group(tbl_interface_t *p_db_if, vid_t community_vlan)
{
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_key;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    pvlan_key.primary_vid = p_db_if->private_vlan;
    p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_key);
    if(p_db_pvlan != NULL)
    {
        rc = brg_release_pvlan_group_id(p_db_pvlan, p_db_if, community_vlan);
    }
    return rc;
}
#endif /* CONFIG_PVLAN */

#ifdef CONFIG_VLANCLASS
int32
if_mgr_set_vlanclass_add_group_to_if(tbl_interface_t *p_if, uint32 vclass_group_id, uint32 vclass_active_type)
{
    tbl_interface_t *p_db_if = NULL;      
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */    
    p_db_if = tbl_interface_get_interface(&p_if->key);    
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /*delete the vlanclass configuration of the interface*/ 
    if (p_db_if->vlanclass_active_type)
    {
        PM_E_RETURN(if_mgr_set_vlanclass_del_group_from_if(p_db_if));
    }

    /*update the DB*/    
    p_if->vlanclass_group_id = vclass_group_id;    
    p_if->vlanclass_active_type = vclass_active_type;    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID));

    return rc;
}

int32
if_mgr_set_vlanclass_del_group_from_if(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;       
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */    
    p_db_if = tbl_interface_get_interface(&p_if->key);    
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /*2. set to DB*/    
    p_if->vlanclass_group_id = 0;    
    p_if->vlanclass_active_type = 0;    
    p_if->vlanclass_default_action = 0;    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_ACTIVE_TYPE)); 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_GROUP_ID)); 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION));

    return rc;
}

int32
if_mgr_set_vlanclass_default_action(tbl_interface_t *p_if, uint32 default_action)
{
    tbl_interface_t *p_db_if = NULL;       
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */    
    p_db_if = tbl_interface_get_interface(&p_if->key);    
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /*2. set to DB*/    
    p_if->vlanclass_default_action = default_action;    
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLANCLASS_DEFAULT_ACTION));
    
    return rc;
}
#endif /* CONFIG_VLANCLASS */

#ifdef CONFIG_L2

static int32
_if_mgr_unset_mode_l3(tbl_interface_t *p_db_if)
{   
    int32 rc = 0;
    if (p_db_if->arp_numberlimit_enable)
    {
        PM_E_RETURN(if_mgr_set_arp_numberlimit_enable(p_db_if, ARP_PORT_NUMBERLIMIT_DISABLE));
    }
    if (p_db_if->arp_numberlimit_number)
    {
        PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if, 0));
    }
    if (ARP_PORT_NUMBERLIMIT_PROTECT != p_db_if->arp_numberlimit_violate)
    {
        PM_E_RETURN(if_mgr_set_arp_numberlimit_violate(p_db_if, ARP_PORT_NUMBERLIMIT_PROTECT));
    }

    return PM_E_NONE;
}

static int32
_if_mgr_set_mode_l3(tbl_interface_t *p_db_if)
{
    int32 rc = 0;
    if_mgr_set_default_mtu(p_db_if);

    PM_E_RETURN(if_mgr_set_arp_numberlimit_number(p_db_if, ARP_PORT_NUMBERLIMIT_NUMBER));

    return PM_E_NONE;
}

static int32
_if_mgr_unset_mode_l2(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    
    PM_E_RETURN(brg_if_set_vlan_type_clear(p_db_if));
    if (NULL != p_db_if->storm_control )
    {
        if ( p_db_if->storm_control->bcast_mode != GLB_RATELIMIT_MODE_NONE)
        {
            PM_E_RETURN(if_mgr_set_storm_control(p_db_if, 0, GLB_RATELIMIT_BROADCAST, GLB_RATELIMIT_MODE_NONE));
        }
        if (p_db_if->storm_control->mcast_mode != GLB_RATELIMIT_MODE_NONE)
        {
            PM_E_RETURN(if_mgr_set_storm_control(p_db_if, 0, GLB_RATELIMIT_MULTICAST, GLB_RATELIMIT_MODE_NONE));
        }
        if (p_db_if->storm_control->ucast_mode != GLB_RATELIMIT_MODE_NONE)
        {
            PM_E_RETURN(if_mgr_set_storm_control(p_db_if, 0, GLB_RATELIMIT_UNICAST, GLB_RATELIMIT_MODE_NONE));
        }
    }
    if (p_db_if->arpinsp_trust_enable)
    {
        PM_E_RETURN(if_mgr_set_arpinsp_trust_enable(p_db_if, FALSE, TRUE));
    }
    if (p_db_if->brgif->dot1q_tag_native_en)
    {
        PM_E_RETURN(brg_if_set_dot1q_tag_native_en(p_db_if, FALSE, TRUE));
    }
    PM_E_RETURN(brg_if_clear_port_security(p_db_if));
    PM_E_RETURN(ds_brgif_del_brgif(p_db_if));
    PM_E_RETURN(swth_mstp_api_del_port(p_db_if));
    /* add for bug 52547 remove mroute_if when change to routed port */
    PM_E_RETURN(swth_api_igsp_mroute_del_port(p_db_if));
    return PM_E_NONE;
}

static int32
_if_mgr_set_mode_l2(tbl_interface_t *p_db_if)
{
    tbl_interface_t intf;
    ds_brgif_t brgif;
    int32 rc = PM_E_NONE;

    sal_memcpy(&intf.key, &p_db_if->key, sizeof(intf.key));
    intf.ifindex = p_db_if->ifindex;
    sal_memset(&brgif, 0, sizeof(brgif));
    brgif.ingress_filter = TRUE;
    brgif.egress_filter = TRUE;
    brgif.tpid_type = GLB_TPID_TYPE_1;

    /* set port's PDU action */
    PM_E_RETURN(swth_sys_pdu_set_port_action(p_db_if->ifindex, GLB_IF_MODE_L2, TRUE));    
    PM_E_RETURN(ds_brgif_add_brgif(p_db_if, &brgif));
    PM_E_RETURN(if_mgr_set_vlan_type(&intf, GLB_VLAN_PORT_TYPE_ACCESS, TRUE));
    PM_E_RETURN(brg_if_set_mac_learning_en(p_db_if, TRUE, TRUE));
    PM_E_RETURN(brg_if_set_port_security_mac_maximum(p_db_if, GLB_PORT_SECURITY_MAC_DEFAULT_MAXIMUM, TRUE));
    PM_E_RETURN(if_mgr_set_default_mtu(p_db_if));
    PM_E_RETURN(swth_mstp_api_add_port(p_db_if));

    return PM_E_NONE;
}

static int32
_if_mgr_set_switch_port(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    PM_E_RETURN(_if_mgr_unset_mode_l3(p_db_if));
    PM_E_RETURN(_if_mgr_set_mode_l2(p_db_if));
    
    return PM_E_NONE;
}

static int32
_if_mgr_set_routed_port(tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    PM_E_RETURN(_if_mgr_unset_mode_l2(p_db_if));
    PM_E_RETURN(_if_mgr_set_mode_l3(p_db_if));

    return PM_E_NONE;
}

int32
if_mgr_set_mode(tbl_interface_t *p_if, uint32 mode, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t  *p_lag = NULL;
    ds_brgif_t brgif;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();

    /* 1. process */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* delete vlan mapping configuration */
    if (GLB_IF_MODE_L3 == mode)
    {
        if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
        {
            PM_E_RETURN(brg_if_dot1q_clear_vlan_mapping(p_db_if, FALSE));
            brgif.qinq_type = GLB_VLAN_QINQ_TYPE_BASIC;
            PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_QINQ_TYPE));
        }
        else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            PM_E_RETURN(brg_if_set_vlan_translation_en(p_db_if, FALSE, FALSE));
        }
    }

    p_if->mode = mode;
    if ((GLB_IF_MODE_L3 == p_db_if->mode) && (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type))
    {
        if (GLB_LB_PHY_LOOPBACK == p_db_if->phy_loopback_mode)
        {
            p_if->phy_loopback_mode = GLB_LB_NONE;
            if_mgr_set_loopback(p_if);
        }
    }

    if (GLB_IF_MODE_L2 == p_if->mode)
    {
        sal_memcpy(&p_if->mac_addr, &p_db_if->hw_mac_addr, sizeof(mac_addr_t));
        /*when change port mode to switchport, set linux mac address to l2 addr*/
        //lib_ioctl_if_set_hw_addr(p_if->key.name, p_if->mac_addr, MAC_ADDR_LEN);
        PM_E_RETURN(_if_mgr_set_switch_port(p_db_if));
    }
    else
    {
        sal_memcpy(&p_if->mac_addr, &p_sys_glb->route_mac, sizeof(mac_addr_t));
        PM_E_RETURN(_if_mgr_set_routed_port(p_db_if));
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_MODE));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_MAC_ADDR));
    
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_mode(p_if_mem, mode, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
if_mgr_set_vlan_type(tbl_interface_t *p_if, uint32 vlan_type, uint32 iter_member)
{
    ds_brgif_t brgif;
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    tbl_pvlan_t  *p_db_pvlan = NULL;
    tbl_pvlan_key_t  pvlan_db_key;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;    
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    /* clear pvlan */
    if (p_db_if->pvlan_type != GLB_IF_PVLAN_TYPE_NONE)
    {
        pvlan_db_key.primary_vid = p_db_if->private_vlan;
        p_db_pvlan = tbl_pvlan_get_pvlan(&pvlan_db_key);
        if(p_db_pvlan != NULL)
        {
            if(GLB_IF_PVLAN_TYPE_COMMUNITY == p_db_if->pvlan_type)
            {
                rc = if_mgr_unmapping_community_vlan_to_group(p_db_if, p_db_if->community_vlan);
            }
            else if(GLB_IF_PVLAN_TYPE_ISOLATED == p_db_if->pvlan_type)
            {
                PM_E_RETURN(tbl_pvlan_set_isloate_ports(p_db_pvlan, FALSE, p_db_if));
            }
            else if(GLB_IF_PVLAN_TYPE_PROMISCUOUS == p_db_if->pvlan_type)
            {
                PM_E_RETURN(tbl_pvlan_set_promiscuous_ports(p_db_pvlan, FALSE, p_db_if));
            }

            if((p_db_pvlan->community_groups.obj_list->count == 0) 
            && (p_db_pvlan->isloate_ports.obj_list->count == 0)
            && (p_db_pvlan->promiscuous_ports.obj_list->count == 0))
            {
                PM_E_RETURN(tbl_pvlan_del_pvlan(&pvlan_db_key));
            }

        }
        p_if->pvlan_type = GLB_IF_PVLAN_TYPE_NONE;
        p_if->private_vlan = 0;
        p_if->community_vlan = 0;
        p_if->pvlan_group_id = 0;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_TYPE));
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PRIVATE_VLAN));
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_COMMUNITY_VLAN));
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_PVLAN_GROUP_ID));
    }
    
    /* clear old properties */
    PM_E_RETURN(brg_if_set_vlan_type_clear(p_db_if));
    p_if->vlan_type = vlan_type;
    brgif.pvid = GLB_VLAN_DEFAULT_VID;

    /* 2. update local CDB */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_VLAN_TYPE));

    /* 3. set default properties */    
    switch (vlan_type)
    {
    case GLB_VLAN_PORT_TYPE_ACCESS:
        PM_E_RETURN(brg_if_set_pvid(p_if, GLB_VLAN_DEFAULT_VID, FALSE));
        break;
    case GLB_VLAN_PORT_TYPE_TRUNK:
        PM_E_RETURN(brg_if_set_pvid(p_if, GLB_VLAN_DEFAULT_VID, FALSE));
        brgif.allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_ADD;
        GLB_BMP_INIT(brgif.allowed_vlan.vlan_bmp);
        GLB_BMP_SET(brgif.allowed_vlan.vlan_bmp, GLB_VLAN_DEFAULT_VID);
        PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, GLB_VLAN_DEFAULT_VID, TRUE, TRUE, FALSE));
        PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, FALSE));
        /* change vlan type from dot1q to trunk need sync ethertype once to conifgure in ASIC */
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SVLAN_TPID));
        break;
    case GLB_VLAN_PORT_TYPE_QINQ:
        PM_E_RETURN(brg_if_set_pvid(p_if, GLB_VLAN_DEFAULT_VID, FALSE));
        brgif.allowed_vlan.action = DS_BRGIF_ALLOWED_VLAN_ACT_ADD;
        GLB_BMP_INIT(brgif.allowed_vlan.vlan_bmp);
        GLB_BMP_SET(brgif.allowed_vlan.vlan_bmp, GLB_VLAN_DEFAULT_VID);
        PM_E_RETURN(brg_if_set_allowed_vlan_process(p_db_if, GLB_VLAN_DEFAULT_VID, TRUE, TRUE, FALSE));
        PM_E_RETURN(brg_if_set_allowed_vlan_db(p_db_if, &brgif, FALSE));
        brgif.qinq_type = GLB_VLAN_QINQ_TYPE_BASIC;
        PM_E_RETURN(ds_brgif_set_brgif_field(p_db_if, &brgif, DS_BRGIF_FLD_QINQ_TYPE));
        break;
    default:
        return PM_E_INVALID_PARAM;
    }

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_vlan_type(p_if_mem, vlan_type, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;
}
#endif /* CONFIG_L2 */

static int32
_if_mgr_set_up_process(tbl_interface_t* p_db_if)
{
    uint32 vid = 0;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    int32 rc = PM_E_NONE;

    if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        rc = sal_sscanf(p_db_if->key.name, GLB_IFNAME_VLAN_PREFIX"%u", &vid);
        if (rc < 0)
        {
            return PM_E_FAIL;
        }
        
        vlan_key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            return PM_E_NONE;
        }
        PM_E_RETURN(if_mgr_vlan_if_running_update(p_db_vlan));
    }
    else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_lag_if_running_update(p_db_if));
    }
    else if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_lag_if_running_update(p_db_if));
        PM_E_RETURN(_if_mgr_running_change_process(p_db_if));
    }

    return PM_E_NONE;
}

int32
if_mgr_vlan_if_running_update_check(tbl_vlan_t *p_db_vlan)
{
    tbl_interface_t intf;
    tbl_interface_t *p_vlan_if = NULL;
    tbl_interface_t *p_mem_if = NULL;
    int32 ifindex = 0;
    int32 rc     = PM_E_NONE;
    int32 count_up = 0;

    sal_memset(&intf, 0, sizeof(intf));
    sal_snprintf(intf.key.name, IFNAME_SIZE, GLB_IFNAME_VLAN_PREFIX"%u", p_db_vlan->key.vid);

    p_vlan_if = tbl_interface_get_interface(&intf.key);
    if (NULL == p_vlan_if)
    {
        return PM_E_NONE;
    }
    
    if (p_db_vlan->enable && tbl_interface_is_up(p_vlan_if))
    {
        GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
        {
            p_mem_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_mem_if)
            {
                continue;
            }
            if (tbl_interface_is_up(p_mem_if))
            {
                count_up ++;
                continue;
            }
        }
        GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);
            
        if (count_up)
        {
            rc = if_up_sync_when_process(p_mem_if, TRUE);
            if (rc)
            {
                return rc;
            }
        }
        else
        {
            rc = if_up_sync_when_process(p_mem_if, FALSE);
            if (rc)
            {
                return rc;
            }
        }
    }

    return rc;
}

int32
if_up_sync_when_process(tbl_interface_t* p_if, uint32 enable)
{
    tbl_interface_t     *p_db_if        = NULL;
    tbl_route_if_t      *p_rt_if        = NULL;
    tbl_route_if_key_t  rtif_key;
    char                cmd_str[128];
    //uint32              new_running     = FALSE;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    //new_running = GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING);    

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_memcpy(rtif_key.name, p_db_if->key.name, sizeof(rtif_key.name));
    p_rt_if = tbl_route_if_get_route_if(&rtif_key);    
    if (NULL == p_rt_if)
    {
        return PM_E_NOT_EXIST;
    }

    return PM_E_NONE;
}

int32
if_mgr_set_up_agg_loop(tbl_interface_t* p_if, uint32 enable, uint32 iter_member, uint32 admin)
{
    tbl_interface_t *p_db_if    = NULL;
    int32           rc          = PM_E_NONE;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

#ifdef CONFIG_ERRDISABLE
    if (errdisable_if_is_errdisable(p_db_if))
    {
        if (CTC_TASK_IS_RUNNING(p_db_if->errdisable_timer))
        {
            CTC_TASK_STOP_TIMER (p_db_if->errdisable_timer);
        }
        p_db_if->errdisable_reason = ERRDIS_NO_ERRDIS;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
    }
#endif /* CONFIG_ERRDISABLE */
  
    /*  update local CDB */
    if (enable)
    {
        /*modified by chenc for bug 52153, 2019-06-03*/
        if (p_db_if->admin_up || (!p_db_if->admin_up && admin))
        /*chenc end*/
        {
            GLB_SET_FLAG(p_if->flags, GLB_IFF_UP);
        }
    }
    else
    {
        GLB_UNSET_FLAG(p_if->flags, GLB_IFF_UP);
    }

    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_UP));
    if (admin)
    {
        p_if->admin_up = (enable) ? TRUE : FALSE;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ADMIN_UP));
    }    
    
    /* dete for changing from running set to normal */
    //rc = if_up_sync_when_process(p_db_if, enable);

    PM_E_RETURN(_if_mgr_set_up_process(p_db_if));

    if (p_db_if->lacp)
    {
        rc = lacp_if_change_process(p_db_if);
    }
     
    return rc;
}

int32
if_mgr_set_up_agg_mem_loop(tbl_interface_t* p_if, uint32 enable, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

#ifdef CONFIG_ERRDISABLE
    if (errdisable_if_is_errdisable(p_db_if))
    {
        if (CTC_TASK_IS_RUNNING(p_db_if->errdisable_timer))
        {
            CTC_TASK_STOP_TIMER (p_db_if->errdisable_timer);
        }
        p_db_if->errdisable_reason = ERRDIS_NO_ERRDIS;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
    }
#endif /* CONFIG_ERRDISABLE */

    rc = if_up_sync_when_process(p_db_if, enable);

    PM_E_RETURN(_if_mgr_set_up_process(p_db_if));

    if (p_db_if->lacp)
    {
        rc = lacp_if_change_process(p_db_if);
    }

    return rc;
}

int32
if_mgr_set_up(tbl_interface_t* p_if, uint32 enable, uint32 iter_member, uint32 admin)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_interface_t     *p_db_if        = NULL;
    ds_lag_t            *p_lag          = NULL;
    ctclib_slistnode_t  *listnode       = NULL;
    tbl_interface_t     *p_if_mem       = NULL;  
    tbl_interface_t     *pst_if_mem     = NULL;   
    tbl_interface_t     *pst_if_lag     = NULL;
    tbl_interface_key_t if_key;
    uint32              agg_if_up = 0;
    uint32              lag_is_up_ck = 0;       
    uint32              agg_mem_is_up_ck = 0;        
    uint32              lag_is_up = 0;        
    int32               memrc = PM_E_NONE;
    int32               rc = PM_E_NONE;
    /* modified by liwh for bug 50646, 2019-01-02 */
    tbl_route_if_t               *p_db_rtif = NULL;
    tbl_route_if_t                rtif;
        char                sz_fullcmd[GLB_MAX_CMD_BUFSZ] = {0};
    /* liwh end */

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* modified by liwh for bug 50646, 2019-01-02 */
    sal_memset(&rtif, 0, sizeof(tbl_route_if_t));
    sal_strncpy(rtif.key.name, p_db_if->key.name, IFNAME_SIZE);

    if (!enable)
    {
        p_db_rtif = tbl_route_if_get_route_if(&rtif.key);
        if (p_db_rtif)
        {
            if  (p_db_rtif->dhcp_client_enable)
            {
                snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcp_l3if_pre_shutdown/%s", p_db_if->key.name);
                sal_system(sz_fullcmd);
            }        
    
            if (p_db_rtif->dhcpv6_client_enable)
            {
                snprintf(sz_fullcmd, GLB_MAX_CMD_BUFSZ, "cdbctl update/cdb/dhcp/dhcpv6_l3if_pre_shutdown/%s", p_db_if->key.name);
                sal_system(sz_fullcmd);
            }
        }
    }
    /* liwh end */

#ifdef CONFIG_ERRDISABLE
    if (errdisable_if_is_errdisable(p_db_if))
    {
        if (CTC_TASK_IS_RUNNING(p_db_if->errdisable_timer))
        {
            CTC_TASK_STOP_TIMER (p_db_if->errdisable_timer);
        }
        p_db_if->errdisable_reason = ERRDIS_NO_ERRDIS;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ERRDISABLE_REASON);
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        logid_sys(LOG_INTERFACE_4_ERRDISABLE_EXIT, ifname_ext, "administer");
    }
#endif /* CONFIG_ERRDISABLE */

    /* 1. process */
    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        lag_is_up_ck = GLB_FLAG_ISSET(p_db_if->flags, GLB_IFF_UP);
        
        /* For clear: enable is true: config no shutdown; enable is false: config shutdown */
        if (enable)
        {
            /* if agg intf is not admin down, and enable is true, just update agg intf excluding agg member */
            /* if agg intf is admin down, and enable is true, need update agg intf and agg member */            
            if (lag_is_up_ck)
            {
                memrc = if_mgr_set_up_agg_loop(p_db_if, enable, FALSE, admin);
            }
            else
            {
                CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
                {
                    memrc = if_mgr_set_up_agg_loop(p_if_mem, enable, FALSE, admin);
                }
            }
        }
        else
        {
            CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
            {
                agg_mem_is_up_ck = GLB_FLAG_ISSET(p_if_mem->flags, GLB_IFF_UP);
                if (enable == agg_mem_is_up_ck)
                {
                    continue;
                }
                
                memrc = if_mgr_set_up_agg_loop(p_if_mem, enable, FALSE, admin);
            }
        }
    }
    (void)memrc;

    /* 2. update local CDB */
    if (enable)
    {
        GLB_SET_FLAG(p_if->flags, GLB_IFF_UP);
    }
    else
    {
        GLB_UNSET_FLAG(p_if->flags, GLB_IFF_UP);
    }
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_UP));

    if (admin)
    {
        p_if->admin_up = (enable) ? TRUE : FALSE;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ADMIN_UP));
    }

    
    sal_memset(&if_key, 0, sizeof(if_key));

    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
        pst_if_lag = tbl_interface_get_interface(&if_key);
        if (NULL == pst_if_lag)
        {
            SAL_ASSERT(0);
        }
        
        p_lag = pst_if_lag->lag;
        lag_is_up = GLB_FLAG_ISSET(pst_if_lag->flags, GLB_IFF_UP | GLB_IFF_RUNNING);
        if (NULL != p_lag)
        {
            CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, pst_if_mem, listnode)
            {
                if (GLB_FLAG_ISSET(pst_if_mem->flags, GLB_IFF_UP))
                    agg_if_up++;
            }
        }

        /* For clear: enable is true: agg mem config no shutdown; enable is false: agg mem config shutdown */
        if (enable == lag_is_up)
        {
            goto out; 
        }

        /* if agg has more than one UP, and enable is true, just pass */
        /* if agg not have more than one UP, and enable is true, need to update agg intf */
        
        /* if enable is false, and agg has mem intf UP, just pass */
        /* if enable is false, and agg not have mem intf UP , need update agg intf */
        if (enable)
        {
            if (1 < agg_if_up)
            {
                goto out;
            }
            else
            {
                rc = if_mgr_set_up_agg_mem_loop(pst_if_lag, enable, TRUE);
            }
        }
        else
        {
            if (0 < agg_if_up)
            {
                goto out;
            }
            else
            {
                rc = if_mgr_set_up_agg_mem_loop(pst_if_lag, enable, TRUE);
            }
        }
    }    

out:
    PM_E_RETURN(_if_mgr_set_up_process(p_db_if));

    if (p_db_if->lacp)
    {
        rc = lacp_if_change_process(p_db_if);
    }
    return rc;
}

int32
if_mgr_set_mtu(tbl_interface_t *p_db_if, uint32 mtu, uint32 iter_member)
{
    ds_lag_t        *p_lag = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 2. update local CDB */
    p_db_if->mtu = mtu;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_MTU));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_mtu(p_if_mem, mtu, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
if_mgr_set_tap_used(tbl_interface_t *p_db_if, uint32 used, uint32 iter_member)
{
    ds_lag_t        *p_lag = NULL;
    tbl_interface_t *p_if_mem = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 2. update local CDB */
    p_db_if->tap_used = used;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_TAP_USED));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_tap_used(p_if_mem, used, iter_member);
            if (memrc)
            {
                return memrc;
            }
        }
    }
    
    return rc;
}

int32
if_mgr_set_bandwidth(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* update config bandwidth */
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_BANDWIDTH));

    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    if (p_if->bandwidth)
    {
        /* oper bandwidth is equal to config */
        p_if->oper_bandwidth = p_if->bandwidth;
        PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_OPER_BANDWIDTH));
        swth_mstp_api_update_if_path_cost(p_db_if);
    }
    else
    {
        /* oper bandwidth calculated */
        PM_E_RETURN(if_mgr_update_oper_bandwidth(p_db_if));
    }

    return rc;
}

int32
if_mgr_flap_check(tbl_interface_t * p_if)
{
    tbl_interface_t *p_db_if  = NULL;
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    errdis_flap_rsn_t flap_rsn = ERRDIS_FLAP_REASON_LINK;
    errdis_rsn_t reason = ERRDIS_LINK_FLAP;
    tbl_errdisable_flap_t *p_db_errdis_flap = NULL;
    tbl_errdisable_flap_key_t errdis_flap_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    bool errdis_en = FALSE;

    if (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF && p_db_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
    {
        return PM_E_NONE;
    }
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return PM_E_NONE;
    }
  
    errdis_flap_key.flap_rsn = flap_rsn;
    p_db_errdis_flap = tbl_errdisable_flap_get_errdisable_flap(&errdis_flap_key);
    if (!p_db_errdis_flap)
    {
        return PM_E_NONE;
    }
    errdis_en = errdisable_is_detect_en(reason);
    p_db_if->link_flap_cnt++;

    if (p_db_if->link_flap_cnt > p_db_errdis_flap->flap_cnt_threshold)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Interface %s link-flap %d times in %d seconds with %s", 
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->link_flap_cnt, p_db_errdis_flap->flap_time_cnt,
            errdis_en ? "errdisable":"warning");
        if (errdis_en)
        {
            errdisable_handle_exception(reason, p_db_if, TRUE);
        }
        p_db_if->link_flap_cnt = 0;
    }
    
    return PM_E_NONE;
}

static int32
_if_mgr_running_change_process(tbl_interface_t* p_db_if)
{
    tbl_interface_key_t if_key;
    ds_lag_t   *p_lag = NULL;
#ifdef CONFIG_L2
    tbl_vlan_key_t vlan_key;
    vid_t vid = 0;
    ds_brgif_t *p_db_brgif = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
#endif /* CONFIG_L2 */
    tbl_interface_t* p_db_if_lag = NULL;
    bool enable = FALSE;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_mirror_t* p_db_mirror = NULL;
    tbl_mirror_key_t mirror_key;
    int32 rc = PM_E_NONE;
    
    enable = (tbl_interface_is_running(p_db_if)) ? TRUE : FALSE;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    logid_sys(LOG_INTERFACE_4_UPDOWN, ifname_ext, enable ? "up" : "down");
    snmp_trap_if_running_change(p_db_if, enable);

    if (enable)
    {
        swth_if_up_cb(p_db_if);
        if_mgr_flap_check(p_db_if);
    }
    else
    {
        swth_if_down_cb(p_db_if);
    }
    
#ifdef CONFIG_L2
    if (!enable)
    {
        /* if down, flush dynamic fdb */
        if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            /* modified by liwh for bug 37200, 2016-08-25 */
            && !p_db_if->erps_enable)
            /* liwh end */
        {
            brg_fdb_flush_dynamic_fdb_port(p_db_if->ifindex);
        }
    }

    /* if switch port, should update vlan interface running */
    p_db_brgif = p_db_if->brgif;
    if (p_db_brgif)
    {
        GLB_BMP_ITER_BEGIN(p_db_brgif->allowed_vlan.vlan_bmp, vid)
        {
            vlan_key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (NULL == p_db_vlan)
            {
                continue;
            }
            PM_E_RETURN(if_mgr_vlan_if_running_update(p_db_vlan));
        }
        GLB_BMP_ITER_END(p_db_brgif->allowed_vlan.vlan_bmp, vid);
    }
#endif /* CONFIG_L2 */

    p_lag = p_db_if->lag;
    /* if has lag and is port if, should update agg interface running */    
    if (p_lag && GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        if (!enable)
        {
            /* if member port is down, clear mismatch */
            p_lag->mismatch = FALSE;
        }
        if (p_lag->lag_id)
        {
            sal_memset(&if_key, 0, sizeof(if_key));
            sal_snprintf(if_key.name, IFNAME_SIZE, GLB_IFNAME_AGG_PREFIX"%u", p_lag->lag_id);
            p_db_if_lag = tbl_interface_get_interface(&if_key);
            if (NULL == p_db_if_lag)
            {
                return PM_E_NOT_EXIST;
            }

            if (GLB_AGG_MODE_STATIC == p_lag->mode)
            {
                PM_E_RETURN(if_agg_static_agg_mux_aggregator_update(p_db_if));
            }
            /*modified by yejl to fix bug43991, 2017-05-13*/
            if (enable && p_db_if_lag->mirror_enable)
            {
                sal_memset(&mirror_key, 0, sizeof(mirror_key));
                mirror_key.id = p_db_if_lag->mirror_dest_id;
                p_db_mirror = tbl_mirror_get_mirror(&mirror_key);
                if (p_db_mirror)
                {
                    mirror_add_dest_to_session(p_db_mirror, p_db_if_lag);
                }
            }
            /*end*/
            PM_E_RETURN(if_mgr_lag_if_running_update(p_db_if_lag));
        }
    }
    
    swth_mstp_api_update_if_running(p_db_if);

    _if_active_backup_agg_oper(NULL, p_db_if, IF_AGG_ACTIVE_BACKUP_OP_MEMBER_LINK_DOWN, 0, NULL);
    return PM_E_NONE;
}

uint32
if_mgr_phy_type_map(tbl_interface_t *p_if, uint32 phy_type)
{
    uint32 mask = 0;
    uint32 pure_phy_type = 0;

    /* phy_type not store flags */
    // mask = GLB_PORT_PHY_TYPE_MASK & phy_type;

    pure_phy_type = phy_type & ~GLB_PORT_PHY_TYPE_MASK;
    switch (pure_phy_type)
    {
        case GLB_PHY_TYPE_UNKNOWN:
            return GLB_PHY_TYPE_UNKNOWN;
        case GLB_PHY_TYPE_1000BASE_T:
        case GLB_PHY_TYPE_10BASE_T:
        case GLB_PHY_TYPE_BASE_PX:
        case GLB_PHY_TYPE_BASE_BX10:
        case GLB_PHY_TYPE_100BASE_LX:
        case GLB_PHY_TYPE_100BASE_TX:
        case GLB_PHY_TYPE_100BASE_FX:
        case GLB_PHY_TYPE_1000BASE_T_SFP:   
        case GLB_PHY_TYPE_1000BASE_CX:
        case GLB_PHY_TYPE_1000BASE_LX:
        case GLB_PHY_TYPE_1000BASE_SX:
        case GLB_PHY_TYPE_10GBASE_ER:
        case GLB_PHY_TYPE_10GBASE_LRM:
        case GLB_PHY_TYPE_10GBASE_LR:
        case GLB_PHY_TYPE_10GBASE_SR:
        case GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER:
        case GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER:
        case GLB_PHY_TYPE_40GBASE_CR4:
        case GLB_PHY_TYPE_40GBASE_SR4:
        case GLB_PHY_TYPE_40GBASE_LR4:
        case GLB_PHY_TYPE_40GBASE_XLPPI:
        /* Added by liuht for bug 33761, 2015-06-05 */
        /* external phy type */
        case GLB_PHY_TYPE_100G_AOC:
        case GLB_PHY_TYPE_100GBASE_SR4:
        case GLB_PHY_TYPE_100GBASE_LR4:
        case GLB_PHY_TYPE_100GBASE_ER4:
        case GLB_PHY_TYPE_100GBASE_SR10:
        case GLB_PHY_TYPE_100G_CWDM4_MSA_FEC:
        case GLB_PHY_TYPE_100G_PSM4_SMF:
        case GLB_PHY_TYPE_100G_ACC:
        case GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC:
        case GLB_PHY_TYPE_100GBASE_CR4:
        case GLB_PHY_TYPE_40GBASE_ER4:
        case GLB_PHY_TYPE_4X10GBASE_SR:
        case GLB_PHY_TYPE_40G_PSM4_SMF:
        case GLB_PHY_TYPE_2G5BASE_T:
        case GLB_PHY_TYPE_5GBASE_T:
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L:
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H:    
        case GLB_PHY_TYPE_25GBASE_SR:
        case GLB_PHY_TYPE_25GBASE_LR:
        case GLB_PHY_TYPE_25GBASE_ER:
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L:
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H:
        case GLB_PHY_TYPE_25GBASE_CR_L:
        case GLB_PHY_TYPE_25GBASE_CR_S:    
        case GLB_PHY_TYPE_25GBASE_CR_N:
        case GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE: 
            return mask | pure_phy_type;
    }
    
    return mask | GLB_PHY_TYPE_UNKNOWN;
}


int32
if_mgr_update_phy_state(tbl_interface_t *p_db_if, fei_if_phy_state_notify_t *p_phy)
{
    uint32 new_phy_type = 0;

    new_phy_type = if_mgr_phy_type_map(p_db_if, p_phy->media);
    if (p_db_if->phy_type == new_phy_type)
    {
        return PM_E_NONE;
    }

    /* Deleted by kcao for bug 43790 2017-04-25 */
#if 0    
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    logid_sys(LOG_INTERFACE_6_PHYCHANGE, ifname_ext, 
        cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_db_if->phy_type),
        cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, new_phy_type));
#endif

    p_db_if->phy_type = new_phy_type;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_TYPE);

    return PM_E_NONE;
}

static int32
_if_mgr_storm_control_speed_change(tbl_interface_t* p_db_if, uint8 speed)
{
    return PM_E_NONE;
}

static int32
_if_mgr_link_oper_speed_upate_handle(tbl_interface_t* p_db_if, uint8 speed)
{
    int32 rc = PM_E_NONE;
    
    if (p_db_if->oper_speed == speed)
    {
        return rc;
    }
    _if_mgr_storm_control_speed_change(p_db_if, speed);

    return rc;
}

int32
if_mgr_rx_link_notify(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t intf;
    tbl_interface_t* p_db_if = NULL;
    fei_if_phy_state_notify_t *p_phy = NULL;
    p_phy = (fei_if_phy_state_notify_t*)p_msg->data;
    uint32 old_link_up = FALSE;
    uint32 new_link_up = FALSE;
    uint32 old_duplex = 0;
    uint32 new_duplex = 0;
    int32 rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_phy->ifindex);
    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    sal_memset(&intf, 0, sizeof(intf));
    sal_memcpy(intf.key.name, p_db_if->key.name, IFNAME_SIZE);
    intf.ifindex = p_db_if->ifindex;

    intf.oper_duplex = p_phy->duplex;
    intf.oper_speed = p_phy->speed;

    PM_E_RETURN(_if_mgr_link_oper_speed_upate_handle(p_db_if, p_phy->speed));
    
    old_link_up = (p_db_if->flags & GLB_IFF_RUNNING) ? TRUE : FALSE; // should not use tbl_interface_is_running(p_if);
    new_link_up = (p_phy->link_up) ? TRUE : FALSE;
    old_duplex = p_db_if->oper_duplex;
    new_duplex = (p_phy->duplex) ? TRUE : FALSE;

    GLB_SET_FLAG_COND(new_link_up, intf.flags, GLB_IFF_RUNNING);

    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_OPER_SPEED));
    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_OPER_DUPLEX));
    if (p_phy->flowctrl_send)
    {
        GLB_SET_FLAG(intf.flowcontrol, GLB_IF_FLOW_OPER_CONTROL_SEND);
    }
    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_FLOWCONTROL_OPER_SEND));

    if (p_phy->flowctrl_recv)
    {
        GLB_SET_FLAG(intf.flowcontrol, GLB_IF_FLOW_OPER_CONTROL_RECEIVE);
    }
    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_FLOWCONTROL_OPER_RECEIVE));
        
    if (old_link_up != new_link_up)
    {
        if (!new_link_up)
        {
            /* if down, flush dynamic fdb */
            if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
                /* modified by liwh for bug 37200, 2016-08-25 */
                && !p_db_if->erps_enable)
                /* liwh end */
            {
                brg_fdb_flush_dynamic_fdb_port(p_db_if->ifindex);
            }
        }
        PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_RUNNING));
        PM_E_RETURN(_if_mgr_running_change_process(p_db_if));
        if (p_db_if->lacp)
        {
            rc = lacp_if_change_process(p_db_if);
        }

        PM_E_RETURN(if_mgr_update_oper_bandwidth(p_db_if));        
    }
    if (old_duplex != new_duplex)
    {
        PM_E_RETURN(swth_mstp_api_update_if_p2p_mac(p_db_if));
    }
    PM_E_RETURN(if_mgr_update_phy_state(p_db_if, p_phy));

    return PM_E_NONE;
}

bool
if_mgr_del_if_openflow_en_check(char *ifname)
{
    tbl_openflow_tunnel_interface_key_t openflow_tunnel_if_key;
    tbl_openflow_tunnel_interface_t *p_db_openflow_tunnel_if = NULL;
    sal_strncpy(openflow_tunnel_if_key.name, ifname, IFNAME_SIZE);

    p_db_openflow_tunnel_if = tbl_openflow_tunnel_interface_get_openflow_tunnel_interface(&openflow_tunnel_if_key);
    if(p_db_openflow_tunnel_if)
    {
        if(p_db_openflow_tunnel_if->openflow_enable)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
if_mgr_del_if(char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0 ,sizeof(if_key));
    sal_strncpy(if_key.name, ifname, IFNAME_SIZE);
    /* 1. get ifindex from ifname */

    /* 2. check exist */
    p_db_if = tbl_interface_get_interface(&if_key);
    if (!p_db_if)
    {
        return PM_E_NOT_EXIST;
    }

    swth_if_del_cb(p_db_if);

    if((GLB_IF_TYPE_L2GRE_IF == p_db_if->hw_type)
        ||(GLB_IF_TYPE_L3GRE_IF == p_db_if->hw_type)
        ||(GLB_IF_TYPE_NVGRE_IF == p_db_if->hw_type)
        ||(GLB_IF_TYPE_VXLAN_IF == p_db_if->hw_type))
    {
        if(if_mgr_del_if_openflow_en_check(ifname))
        {
            CFG_CONFLICT_RET("This interface is enable openflow, please disable it first");
        }
    }

    /* added by hansf for netconf */
    if (CDB_CFG_NC_CHECK == para->cfg)
    {
        return PM_E_NONE;
    }
    /* ended by hansf */

    /* 3. delete different interface (port/agg/vlan/tunnel/...) */
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_port_if(p_db_if));
    }
    else if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_vlan_if(p_db_if));
    }
    else if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_loopback_if(p_db_if));
    }
    else if (GLB_IF_TYPE_L2GRE_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_overlay_if(p_db_if));
    }
    else if (GLB_IF_TYPE_L3GRE_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_overlay_if(p_db_if));
    }
    else if (GLB_IF_TYPE_NVGRE_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_overlay_if(p_db_if));
    }
    else if (GLB_IF_TYPE_VXLAN_IF == p_db_if->hw_type)
    {
        PM_E_RETURN(if_mgr_del_overlay_if(p_db_if));
    }
    
    return rc;
}

int32
if_mgr_set_if_config_up()
{
    tbl_interface_master_t *p_if_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t intf;
    uint32 shutdown = FALSE;

    /* set if up */
    sal_memset(&intf, 0, sizeof(intf));
    
    CTCLIB_SLIST_LOOP(p_if_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }

        shutdown = (p_db_if->flags & GLB_IFF_CFG_SHUTDOWN);
        if (!shutdown)
        {
            sal_memcpy(intf.key.name, p_db_if->key.name, IFNAME_SIZE);
            intf.ifindex = p_db_if->ifindex;
            /* for MLAG reload-delay */
            if (errdisable_if_is_errdisable(p_db_if))
            {
                p_db_if->admin_up = TRUE;
                tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ADMIN_UP);
            }
            else
            {
                if_mgr_set_up(&intf, TRUE, FALSE, TRUE);
            }
        }
        else
        {
            GLB_UNSET_FLAG(p_db_if->flags, GLB_IFF_CFG_SHUTDOWN);
        }
    }

    return PM_E_NONE;
}

int32
if_mgr_get_default_bandwidth(tbl_interface_t *p_if, uint32 *p_bw)
{
    if (NULL == p_if)
    {
        return PM_E_INVALID_PARAM;
    }
    
    switch(p_if->hw_type)
    {
    case GLB_IF_TYPE_LOOPBACK_IF:
        *p_bw = IF_LO_DEFAULT_BW;
        break;

    case GLB_IF_TYPE_PORT_IF:
        break;

    case GLB_IF_TYPE_LINKAGG_IF:
        break;
        
    case GLB_IF_TYPE_VLAN_IF:
        *p_bw = IF_VLAN_DEFAULT_BW;
        break;

    case GLB_IF_TYPE_TUNNEL_IF:
        *p_bw = IF_TUNNEL_DEFAULT_BW;
        break;

    case GLB_IF_TYPE_L2GRE_IF:
    case GLB_IF_TYPE_L3GRE_IF:
    case GLB_IF_TYPE_NVGRE_IF:
    case GLB_IF_TYPE_VXLAN_IF:
        *p_bw = IF_TUNNEL_DEFAULT_BW;
        break;

    default:
        *p_bw = 0;
        return PM_E_INVALID_PARAM;
    }

    /*Physical port's bw is depend on the speed*/
    if (GLB_IF_TYPE_PORT_IF == p_if->hw_type)
    {
        switch (p_if->oper_speed)
        {
        case GLB_SPEED_10M:
            *p_bw = 10000;
            break;
            
        case GLB_SPEED_100M:
            *p_bw = 100000;
            break;

        case GLB_SPEED_1G:
            *p_bw = 1000000;
            break;
            
        case GLB_SPEED_10G:
            *p_bw = 10000000;
            break;

        case GLB_SPEED_5G:
            *p_bw = 5000000;
            break;

        case GLB_SPEED_2_5G:
            *p_bw = 2500000;
            break;
            
        case GLB_SPEED_40G:
            *p_bw = 40000000;
            break;

        case GLB_SPEED_50G:
            *p_bw = 50000000;
            break;

        case GLB_SPEED_25G:
            *p_bw = 25000000;
            break;            
            
        case GLB_SPEED_100G:
            *p_bw = 100000000;
            break;

        default:
            *p_bw = 1000000;
            return PM_E_NONE;
        }
    }

    return PM_E_NONE;
}

int32
if_mgr_get_if_stats(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) &&
            (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        return PM_E_NONE;
    }
       
    return PM_E_NONE;
}

int32
if_mgr_clear_if_stats(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs)
{
    int32 rc = PM_E_NONE;
    
    if ((GLB_IF_TYPE_PORT_IF != p_db_if->hw_type) &&
            (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type))
    {
        return PM_E_NONE;
    }
   
    return rc;
}

static int32
_if_mgr_agg_add_member_check_pvlan_cb(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    cfg_cmd_para_t *para = (cfg_cmd_para_t*)p_param;
#if 0
    if (GLB_IF_FLOW_CONTROL_OFF != p_db_mem_if->flowcontrol)
    {
        CFG_CONFLICT_RET("Interface %s is already enabled flow control", p_db_mem_if->key.name);
    }
#endif
    if (p_db_agg_if != NULL)
    {
        if ((p_db_agg_if->pvlan_type != p_db_mem_if->pvlan_type) 
        || (p_db_agg_if->private_vlan != p_db_mem_if->private_vlan) 
        || (p_db_agg_if->community_vlan != p_db_mem_if->community_vlan)
        || (p_db_agg_if->pvlan_group_id != p_db_mem_if->pvlan_group_id))
        {
            CFG_CONFLICT_RET(" Private vlan configuartion does not match.");
        }
    }
    return PM_E_NONE;
}

static int32
_if_mgr_set_agg_add_member_check_func()
{
    if_agg_add_member_check_set_cb(IF_AGG_CB_MODULE_PVLAN, _if_mgr_agg_add_member_check_pvlan_cb);
    return PM_E_NONE;
}

int32
if_mgr_sflow_pkt_tx(uint8 *p_pkt, uint32 pkt_len)
{
    return ipc_pkt_tx_sflow(p_pkt, pkt_len, 0, GLB_ALL_VLAN_ID);
}

int32
if_mgr_set_opf_intf_pkt_default(tbl_openflow_interface_t *p_opf_intf)
{
    p_opf_intf->action[GLB_OPF_PKT_TYPE_STP] = GLB_OPF_PKT_ACTION_FORWARD;
    p_opf_intf->action[GLB_OPF_PKT_TYPE_LACP] = GLB_OPF_PKT_ACTION_SYSTEM;
    p_opf_intf->action[GLB_OPF_PKT_TYPE_LLDP] = GLB_OPF_PKT_ACTION_FORWARD;

    return PM_E_NONE;
}

int32
if_mgr_packet_type(char *pkt_type)
{
    int32 packet_type = 0;

    if (0 == sal_strcmp(pkt_type, "stp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_STP;
    }
    if (0 == sal_strcmp(pkt_type, "lacp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_LACP;
    }
    if (0 == sal_strcmp(pkt_type, "lldp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_LLDP;
    }
    if (0 == sal_strcmp(pkt_type, "igmp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_IGMP;
    }
    if (0 == sal_strcmp(pkt_type, "erps"))
    {
        packet_type = GLB_OPF_PKT_TYPE_ERPS;
    }
    if (0 == sal_strcmp(pkt_type, "dot1x"))
    {
        packet_type = GLB_OPF_PKT_TYPE_DOT1X;
    }
    if (0 == sal_strcmp(pkt_type, "arp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_ARP;
    }
    if (0 == sal_strcmp(pkt_type, "dhcp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_DHCP;
    }
    if (0 == sal_strcmp(pkt_type, "ospf"))
    {
        packet_type = GLB_OPF_PKT_TYPE_OSPF;
    }
    if (0 == sal_strcmp(pkt_type, "pim"))
    {
        packet_type = GLB_OPF_PKT_TYPE_PIM;
    }
    if (0 == sal_strcmp(pkt_type, "vrrp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_VRRP;
    }
    if (0 == sal_strcmp(pkt_type, "bgp"))
    {
        packet_type = GLB_OPF_PKT_TYPE_BGP;
    }

    return packet_type;
}

int32
if_mgr_packet_action(char *pkt_act)
{
    int32 packet_act = 0;

    if (0 == sal_strcmp(pkt_act, "forward"))
    {
        packet_act = GLB_OPF_PKT_ACTION_FORWARD;
    }
    if (0 == sal_strcmp(pkt_act, "system"))
    {
        packet_act = GLB_OPF_PKT_ACTION_SYSTEM;
    }
    if (0 == sal_strcmp(pkt_act, "ovs-local"))
    {
        packet_act = GLB_OPF_PKT_ACTION_OVS;
    }

    return packet_act;
}

static int32
_sflow_join_agg_create(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
#ifdef CONFIG_SFLOW
    /* 1. clear member interface sflow configuration */
    if (p_db_mem_if)
    {
        /* disable sflow counter sampling*/
        if_mgr_set_sflow_counter_enable(p_db_mem_if, FALSE);

        /* disable sflow flow sampling */
        if_mgr_set_sflow_flow_enable(p_db_mem_if, FALSE, TRUE, TRUE);

        /* set sflow rate to default value */
        if_mgr_set_sflow_rate(p_db_mem_if, GLB_SFLOW_DEFAULT_RATE);
    }
#endif /* CONFIG_SFLOW */

    return PM_E_NONE;
}

static int32
_sflow_join_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
#ifdef CONFIG_SFLOW
    /* 1. clear member interface sflow configuration */
    if (p_db_mem_if)
    {
        /* disable sflow counter sampling*/
        if_mgr_set_sflow_counter_enable(p_db_mem_if, FALSE);

        /* disable sflow flow sampling */
        if_mgr_set_sflow_flow_enable(p_db_mem_if, FALSE, TRUE, TRUE);

        /* set sflow rate to default value */
        if_mgr_set_sflow_rate(p_db_mem_if, GLB_SFLOW_DEFAULT_RATE);

        if (p_db_agg_if)
        {
            if (GLB_SFLOW_DEFAULT_RATE!= p_db_agg_if->sflow_rate)
            {
                if_mgr_set_sflow_rate(p_db_mem_if,p_db_agg_if->sflow_rate);
            }
                
            if (p_db_agg_if->sflow_flow_ingress)
            {
                if_mgr_set_sflow_flow_enable(p_db_mem_if, TRUE, TRUE, FALSE);
            }

            if (p_db_agg_if->sflow_flow_egress)
            {
                if_mgr_set_sflow_flow_enable(p_db_mem_if, TRUE, FALSE, TRUE);  
            }
        }
    }
#endif /* CONFIG_SFLOW */

    return PM_E_NONE;
}

static int32
_sflow_leave_agg_update(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
#ifdef CONFIG_SFLOW
    /* 1. clear member interface sflow configuration */
    if (p_db_mem_if)
    {
        /* disable sflow counter sampling*/
        if_mgr_set_sflow_counter_enable(p_db_mem_if, FALSE);

        /* disable sflow flow sampling */
        if_mgr_set_sflow_flow_enable(p_db_mem_if, FALSE, TRUE, TRUE);  

        /* set sflow rate to default value */
        if_mgr_set_sflow_rate(p_db_mem_if, GLB_SFLOW_DEFAULT_RATE);
    }
#endif /* CONFIG_SFLOW */

    return PM_E_NONE;
}

static int32
_sflow_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
#ifdef CONFIG_SFLOW
    if (p_db_agg_if)
    {
        /* disable sflow counter sampling*/
        if_mgr_set_sflow_counter_enable(p_db_agg_if, FALSE);
    }
#endif /* CONFIG_SFLOW */

    return PM_E_NONE;
}

/* delete meps before configure qinq interface modified by liwh for bug 53010, 2019-07-22 */
int32
if_mgr_clear_interface_cfm_meps( tbl_interface_t *p_db_if)
{
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    ctclib_slistnode_t             *listnode_next = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;

    if (p_db_if->cfm_rmep_num)
    {
        p_db_glb_rmep = tbl_cfm_rmep_get_master();
        if (p_db_glb_rmep)
        {
            CTCLIB_SLIST_LOOP_DEL(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode, listnode_next)
            {
                if (p_rmep->ifindex == p_db_if->ifindex)
                {
                    tbl_cfm_rmep_del_cfm_rmep(&p_rmep->key);
                    p_db_if->cfm_rmep_num--;
                }
            }
        }
    }
    
    if (p_db_if->cfm_lmep_num)
    {
        p_db_glb = tbl_cfm_lmep_get_master();
        if (p_db_glb)
        {
            CTCLIB_SLIST_LOOP_DEL(p_db_glb->cfm_lmep_list, p_lmep, listnode, listnode_next)
            {
                if (p_lmep->key.ifindex == p_db_if->ifindex)
                {
                    tbl_cfm_lmep_del_cfm_lmep(&p_lmep->key);
                    p_db_if->cfm_lmep_num--;
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int32
if_mgr_clear_cfm_meps_by_interface_vlan( tbl_interface_t *p_db_if, vid_t vid)
{
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    ctclib_slistnode_t             *listnode_next = NULL;
    tbl_cfm_rmep_master_t   *p_db_glb_rmep = NULL;
    tbl_cfm_rmep_t               *p_rmep = NULL;

    if (p_db_if->cfm_rmep_num)
    {
        p_db_glb_rmep = tbl_cfm_rmep_get_master();
        if (p_db_glb_rmep)
        {
            CTCLIB_SLIST_LOOP_DEL(p_db_glb_rmep->cfm_rmep_list, p_rmep, listnode, listnode_next)
            {
                if ((p_rmep->ifindex == p_db_if->ifindex) && (p_rmep->key.vlan_id == vid))
                {
                    tbl_cfm_rmep_del_cfm_rmep(&p_rmep->key);
                    p_db_if->cfm_rmep_num--;
                }
            }
        }
    }
    
    if (p_db_if->cfm_lmep_num)
    {
        p_db_glb = tbl_cfm_lmep_get_master();
        if (p_db_glb)
        {
            CTCLIB_SLIST_LOOP_DEL(p_db_glb->cfm_lmep_list, p_lmep, listnode, listnode_next)
            {
                if ((p_lmep->key.ifindex == p_db_if->ifindex) && (p_lmep->key.vlan_id == vid))
                {
                    tbl_cfm_lmep_del_cfm_lmep(&p_lmep->key);
                    p_db_if->cfm_lmep_num--;
                }
            }
        }
    }
    
    return PM_E_NONE;
}
/* liwh end */

/* modified by liwh for bug 53010, 2019-07-22 */
static int32
_cfm_check_before_add_agg_member(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if_mgr_clear_interface_cfm_meps(p_db_mem_if);

    return PM_E_NONE;
}
/* liwh end */

static int32
_dhcp_snooping_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if (NULL != p_db_agg_if) {
        ctclib_slistnode_t *lnode = NULL;
        ctclib_slistnode_t *nnext = NULL;
        ds_circuit_id_t *pItem;
        
        CTCLIB_SLIST_LOOP_DEL(p_db_agg_if->circuit_id.obj_list, pItem, lnode, nnext) {
            ds_circuit_id_del_circuit_id(p_db_agg_if, pItem);
        }
    }

    return PM_E_NONE;
}

int32
if_mgr_link_notify_process(tbl_interface_t* p_db_if, glb_port_oper_status_t port_status)
{
    uint32 old_link_up = FALSE;
    uint32 new_link_up = FALSE;
    int32 rc = PM_E_NONE;

    if (!p_db_if)
    {
        return PM_E_FAIL;
    }

    old_link_up = (p_db_if->flags & GLB_IFF_RUNNING) ? TRUE : FALSE; // should not use tbl_interface_is_running(p_if);
    new_link_up = (port_status == GLB_PORT_OPER_STATUS_UP) ? TRUE : FALSE;

    GLB_SET_FLAG_COND(new_link_up, p_db_if->flags, GLB_IFF_RUNNING);
        
    if (old_link_up != new_link_up)
    {
        if (!new_link_up)
        {
            /* if down, flush dynamic fdb */
            if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type || GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
                /* modified by liwh for bug 37200, 2016-08-25 */
                && !p_db_if->erps_enable)
                /* liwh end */
            {
                brg_fdb_flush_dynamic_fdb_port(p_db_if->ifindex);
            }
        }
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_RUNNING));
        PM_E_RETURN(_if_mgr_running_change_process(p_db_if));
        if (p_db_if->lacp)
        {
            rc = lacp_if_change_process(p_db_if);
        }
        PM_E_RETURN(if_mgr_update_oper_bandwidth(p_db_if)); 
    }

	if(new_link_up)
	{
		/*added by yejl to fix bug 44000, 2017-05-12, get the new oper_speed and set the storm control again*/
	    if (p_db_if->storm_control)
	    {
	        PM_E_RETURN(if_mgr_update_storm_control_level(p_db_if, p_db_if->storm_control));
	    }
	}
		
#if 0
    if (old_duplex != new_duplex)
    {
        PM_E_RETURN(swth_mstp_api_update_if_p2p_mac(p_db_if));
    }
  
    PM_E_RETURN(if_mgr_update_phy_state(p_db_if, p_phy));
#endif  
    return PM_E_NONE;
}

int32 
if_mgr_interface_fdb_loop_cb(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    p_if = (tbl_interface_t*)p_msg->data;
    if (NULL == p_if)
    {
        return PM_E_NONE;
    }
    p_db_if = tbl_interface_get_interface(&(p_if->key));
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }
    
    if (errdisable_is_detect_en(ERRDIS_FDB_LOOP))
    {
        errdisable_handle_exception(ERRDIS_FDB_LOOP, p_db_if, TRUE);
    }
    
    return PM_E_NONE;
}

int32
if_mgr_arp_numberlimit_occur_process(tbl_interface_t* p_db_if)
{
    if (p_db_if == NULL)
    {
        return PM_E_NONE;
    }
    if (errdisable_is_detect_en(ERRDIS_ARP_NUMBERLIMIT))
    {
        errdisable_handle_exception(ERRDIS_ARP_NUMBERLIMIT, p_db_if, TRUE);
    }
    return PM_E_NONE;    
}

/*arp rate limit up to max, set action to errdisable, this function will be called*/
int32
if_mgr_arp_rate_limit_errdisable_occur_process(tbl_interface_t* p_db_if)
{
    if (p_db_if == NULL)
    {
        return PM_E_NONE;
    }
    if (errdisable_is_detect_en(ERRDIS_ARP_RATE_LIMIT))
    {
        errdisable_handle_exception(ERRDIS_ARP_RATE_LIMIT, p_db_if, TRUE);
    }
    return PM_E_NONE;
}

int32
if_mgr_openflow_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    return PM_E_NONE;
}

int32
if_mgr_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_t* p_if = NULL;
    tbl_interface_t* p_db_if = NULL; 
    int32 rc = PM_E_NONE;

    if(p_ds_node)
    {
        return PM_E_NONE;
    }
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_if = (tbl_interface_t*)p_tbl;
        if(NULL == p_if)
        {
            return PM_E_INVALID_PARAM;
        }
        p_db_if = tbl_interface_get_interface(&p_if->key);
        if(NULL == p_db_if)
        {
            return PM_E_NOT_EXIST;
        }        
        
        switch(field_id)
        {
            case TBL_INTERFACE_FLD_PHY_STATUS:
                PM_E_RETURN(if_mgr_link_notify_process(p_db_if, p_if->phy_status));
                break;
            case TBL_INTERFACE_FLD_FDB_LOOP_OCCUR:
                break;
            case TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR:
                PM_E_RETURN(if_mgr_arp_numberlimit_occur_process(p_db_if));
                break;
            case TBL_INTERFACE_FLD_ARP_RATE_LIMIT_ERRDISABLE_OCCUR:
                PM_E_RETURN(if_mgr_arp_rate_limit_errdisable_occur_process(p_db_if));
                break;
        }
        break;

    default:
        return PM_E_NONE;
    }

    return PM_E_NONE;
}

static uint32
_if_stats_rate_hash_make(if_port_rate_info_t *p_stats_rate_info)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_stats_rate_info->key;
    for (index = 0; index < sizeof(p_stats_rate_info->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_if_stats_rate_hash_cmp(void *p_arg1, void *p_arg2)
{
    if_port_rate_info_t *p_stats_info_port1 = (if_port_rate_info_t*)p_arg1;
    if_port_rate_info_t *p_stats_info_port2 = (if_port_rate_info_t*)p_arg2;

    if (0 == sal_memcmp(&p_stats_info_port1->key, &p_stats_info_port2->key, sizeof(tbl_port_stats_rate_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

void
_if_process_bandwidth_in_use(tbl_port_stats_rate_t *p_db_stats_rate)
{
    uint32  speed = 0;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t intf;
    tbl_interface_t *p_db_if = NULL;

    tbl_interface_get_name_by_ifindex(p_db_stats_rate->key.ifindex, ifname, IFNAME_SIZE);
    sal_memset(&intf, 0, sizeof(intf));
    sal_strncpy(intf.name, ifname, IFNAME_SIZE);

    p_db_if = tbl_interface_get_interface(&intf);
    if (!p_db_if)
    {
        return;
    }
    if_mgr_get_default_bandwidth(p_db_if, &speed);

    if (speed == 0)
    {
        p_db_stats_rate->input_rate = 0;
        p_db_stats_rate->output_rate = 0;
        return;
    }

    p_db_stats_rate->input_rate = (p_db_stats_rate->octets_rcv_rate 
                + p_db_stats_rate->pkts_rcv_rate * IF_DEFAULT_IPG_SIZE) / (speed * 10);
    if (p_db_stats_rate->input_rate > 100)
    {
        p_db_stats_rate->input_rate = 100;
    }
    p_db_stats_rate->output_rate = (p_db_stats_rate->octets_send_rate
                + p_db_stats_rate->pkts_send_rate * IF_DEFAULT_IPG_SIZE) / (speed * 10);
    if (p_db_stats_rate->output_rate > 100)
    {
        p_db_stats_rate->output_rate = 100;
    }
    tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate, TBL_PORT_STATS_RATE_FLD_INPUT_RATE);
    tbl_port_stats_rate_set_port_stats_rate_field(p_db_stats_rate, TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE);

    if (LOG_THRESHOLD_DISABLE != p_db_stats_rate->input_rate_state)
    {
        if (p_db_stats_rate->input_rate >= p_db_stats_rate->input_rate_threshold)
        {
            if (LOG_THRESHOLD_OVER != p_db_stats_rate->input_rate_state)
            {
                p_db_stats_rate->input_rate_state = LOG_THRESHOLD_OVER;
                logid_sys(LOG_INTERFACE_4_BANDWIDTH_OVERFLOW, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext),
                          "input", p_db_stats_rate->input_rate_threshold);
            }
        }
        else if (p_db_stats_rate->input_rate < p_db_stats_rate->input_rate_resume)
        {
            if (LOG_THRESHOLD_OVER == p_db_stats_rate->input_rate_state)
            {
                p_db_stats_rate->input_rate_state = LOG_THRESHOLD_RESUME;
                logid_sys(LOG_INTERFACE_4_BANDWIDTH_RESUME, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext),
                          "input", p_db_stats_rate->input_rate_resume);
            }
        }
    }

    if (LOG_THRESHOLD_DISABLE != p_db_stats_rate->output_rate_state)
    {
        if (p_db_stats_rate->output_rate >= p_db_stats_rate->output_rate_threshold)
        {
            if (LOG_THRESHOLD_OVER != p_db_stats_rate->output_rate_state)
            {
                p_db_stats_rate->output_rate_state = LOG_THRESHOLD_OVER;
                logid_sys(LOG_INTERFACE_4_BANDWIDTH_OVERFLOW, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext),
                          "output", p_db_stats_rate->output_rate_threshold);
            }
        }
        else if (p_db_stats_rate->output_rate < p_db_stats_rate->output_rate_resume)
        {
            if (LOG_THRESHOLD_OVER == p_db_stats_rate->output_rate_state)
            {
                p_db_stats_rate->output_rate_state = LOG_THRESHOLD_RESUME;
                logid_sys(LOG_INTERFACE_4_BANDWIDTH_RESUME, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext),
                          "output", p_db_stats_rate->output_rate_resume);
            }
        }
    }

    return;
}

void
if_port_stats_rate_scan_timeout(void *parg)
{
    ctclib_slistnode_t *listnode = NULL;  
    tbl_port_stats_rate_master_t *p_master = tbl_port_stats_rate_get_master();
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;
    if_port_rate_info_t *p_db_stats_info = NULL;
    if_port_rate_info_t lkp;
    tbl_port_stats_t* p_port_stats = NULL;
    tbl_port_stats_key_t port_stats_key;
    bool need_store;
    void *p_ret = NULL;
    int interval;
    uint8 tmp_index;

    g_p_stats_rate_scan_time = NULL;

    CTCLIB_SLIST_LOOP(p_master->port_stats_rate_list, p_db_stats_rate, listnode)
    {
        lkp.key.ifindex = p_db_stats_rate->key.ifindex;
        p_db_stats_info = ctclib_hash_lookup(if_rate_db_master.if_stats_hash, &lkp);
        if (NULL == p_db_stats_info)
        {
            p_db_stats_info = XCALLOC(MEM_TBL_PORT_STATS_RATE , sizeof(if_port_rate_info_t));
            if (NULL == p_db_stats_info)
            {
                continue;
            }

            p_db_stats_info->key.ifindex = p_db_stats_rate->key.ifindex;
            p_db_stats_info->start_time = ctc_time_boottime_sec(NULL);
            p_ret = ctclib_hash_get(if_rate_db_master.if_stats_hash, (void*)p_db_stats_info, ctclib_hash_alloc_intern);
            if (NULL == p_ret)
            {
                continue;
            }  
        }

        p_db_stats_info->store_poll_cnt++;

        port_stats_key.ifindex = p_db_stats_rate->key.ifindex;
        p_port_stats = tbl_port_stats_get_port_stats(&port_stats_key);
        if (NULL == p_port_stats)
        {
            continue;
        }

        need_store = (0 == (p_db_stats_info->store_poll_cnt*GLB_PORT_STATS_INTERVAL) % (p_db_stats_rate->load_interval / 5));
        if (need_store)
        {
            p_db_stats_info->octets_rcv[p_db_stats_info->index] = p_port_stats->octets_rcv;
            p_db_stats_info->pkts_rcv[p_db_stats_info->index] = p_port_stats->pkts_rcv;
            p_db_stats_info->octets_send[p_db_stats_info->index] = p_port_stats->octets_send;
            p_db_stats_info->pkts_send[p_db_stats_info->index] = p_port_stats->pkts_send;
            p_db_stats_info->index = (1 + p_db_stats_info->index) % GLB_IF_STATS_STORE_CNT;
            p_db_stats_info->store_time = ctc_time_boottime_sec(NULL);
        }

        if (ctc_time_boottime_sec(NULL) - p_db_stats_info->start_time > p_db_stats_rate->load_interval)
        {
            interval = ctc_time_boottime_sec(NULL) - p_db_stats_info->store_time + p_db_stats_rate->load_interval;
            tmp_index = p_db_stats_info->index;
        }
        else
        {
            interval = p_db_stats_rate->load_interval;
            tmp_index = 0;
        }

        if (0 == interval)
        {
            continue;
        }

        p_db_stats_rate->octets_rcv_rate = (p_port_stats->octets_rcv - 
                p_db_stats_info->octets_rcv[tmp_index])*8/interval;

        p_db_stats_rate->pkts_rcv_rate = (p_port_stats->pkts_rcv -
            p_db_stats_info->pkts_rcv[tmp_index])/interval;

        p_db_stats_rate->octets_send_rate = (p_port_stats->octets_send -
            p_db_stats_info->octets_send[tmp_index])*8/interval;

        p_db_stats_rate->pkts_send_rate = (p_port_stats->pkts_send -
            p_db_stats_info->pkts_send[tmp_index])/interval;

        _if_process_bandwidth_in_use(p_db_stats_rate);
    }

    g_p_stats_rate_scan_time = ctc_task_add_timer(TASK_PRI_NORMAL,
            if_port_stats_rate_scan_timeout, NULL, GLB_PORT_STATS_INTERVAL);

    return;
}

void
_if_clear_port_stats_rate(uint32 ifindex)
{
    if_port_rate_info_t *p_db_stats_info = NULL;
    if_port_rate_info_t lkp_db_info;
    tbl_port_stats_rate_t *p_stats_rate = NULL;
    tbl_port_stats_rate_key_t lkp_stats_rate;

    lkp_db_info.key.ifindex = ifindex;
    p_db_stats_info = ctclib_hash_lookup(if_rate_db_master.if_stats_hash, &lkp_db_info);
    if (p_db_stats_info)
    {
        sal_memset(p_db_stats_info, 0, sizeof(if_port_rate_info_t));
        p_db_stats_info->key.ifindex = ifindex;
        p_db_stats_info->start_time = ctc_time_boottime_sec(NULL);
    }

    lkp_stats_rate.ifindex = ifindex;
    p_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&lkp_stats_rate);
    if (p_stats_rate)
    {
        p_stats_rate->octets_rcv_rate = 0;
        p_stats_rate->octets_send_rate = 0;
        p_stats_rate->pkts_rcv_rate = 0;
        p_stats_rate->pkts_send_rate = 0;
    }
    
    return;
}

int32
if_clear_port_stats_rate(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    ctclib_slistnode_t *listnode = NULL;  
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    tbl_port_stats_t *p_db_port_stats = NULL;
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (argc)
        {
            ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
            if (0 == sal_memcmp(argv[0], GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN)
                ||0 == sal_memcmp(argv[0], GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
            {
                _if_clear_port_stats_rate(ifindex);
            }
        }
        else
        {
            CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_db_port_stats, listnode)
            {
                _if_clear_port_stats_rate(p_db_port_stats->key.ifindex);
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_if_mgr_port_stats_dump_all(tbl_port_stats_rate_t *p_stats_rate,  cfg_cmd_para_t *para)
{
    tbl_port_stats_rate_master_t *p_master = tbl_port_stats_rate_get_master();
    tbl_iter_args_t args;
    FILE *fp = NULL;
    field_parse_t field;
    ctclib_slistnode_t *listnode = NULL;
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    if (NULL == p_stats_rate)
    {
        CTCLIB_SLIST_LOOP(p_master->port_stats_rate_list, p_db_stats_rate, listnode)
        {
            tbl_port_stats_rate_dump_one(p_db_stats_rate, &args);
        }
    }
    else
    {
        tbl_port_stats_rate_dump_one(p_stats_rate, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
if_mgr_cmd_process_stats_rate(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;
    tbl_port_stats_rate_t port_stats_rate;
    cdb_field_t  *p_field = NULL;
    cdb_node_t   *p_node  = NULL;
    uint32 ifindex = 0;
    uint32  if_type = 0;
    int32   field_id = 0;
    uint16  load_interval = GLB_IF_STATS_LOAD_DEF_INTVAL;
    uint8   threshold  = 0;
    uint8   resumerate = 0;
    uint32  discard_threshold = 0;
    uint32  discard_interval  = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_PORT_STATS_RATE);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
        if_type = tbl_interface_get_type_by_ifindex(ifindex);
        if (GLB_IF_TYPE_PORT_IF != if_type)
        {
            CFG_CONFLICT_RET("Unsupport for port %s", argv[0]);
        }

        port_stats_rate.key.ifindex = ifindex;
        p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate.key);
        if (NULL == p_db_stats_rate)
        {
            CFG_FUNC_ERR_RET("Port stats rate ifindex %d not exist", ifindex);
        }

        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_PORT_STATS_RATE_FLD_INPUT_RATE_THRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            PM_CFG_GET_UINT(threshold, argv[2], para->p_rs);
            PM_CFG_GET_UINT(resumerate, argv[4], para->p_rs);

            rc = _if_mgr_cmd_set_input_rate_threshold(p_db_stats_rate, threshold, resumerate, para);
            break;

        case TBL_PORT_STATS_RATE_FLD_OUTPUT_RATE_THRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            PM_CFG_GET_UINT(threshold, argv[2], para->p_rs);
            PM_CFG_GET_UINT(resumerate, argv[4], para->p_rs);

            rc = _if_mgr_cmd_set_output_rate_threshold(p_db_stats_rate, threshold, resumerate, para);
            break;

        case TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_THRESHOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 5, para->p_rs);
            PM_CFG_GET_UINT(discard_threshold, argv[2], para->p_rs);
            PM_CFG_GET_UINT(discard_interval, argv[4], para->p_rs);

            rc = _if_mgr_cmd_set_output_discard_threshold(p_db_stats_rate, discard_threshold, discard_interval, para);
            break;

        case TBL_PORT_STATS_RATE_FLD_LOAD_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(load_interval, argv[2], para->p_rs);

            rc = _if_mgr_cmd_set_load_interval(p_db_stats_rate, load_interval, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
            break;
        }

        break;

    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            if (sal_isdigit(argv[0][0]))
            {
                PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
            }
            else
            {
                ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
            }
            port_stats_rate.key.ifindex = ifindex;
            p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate.key);
            if (NULL == p_db_stats_rate)
            {
                CFG_FUNC_ERR_RET("Port stats rate ifindex %d not exist\n", ifindex);
            }            
        }
        _if_mgr_port_stats_dump_all(p_db_stats_rate, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_if_mgr_mib_stats_dump_one(tbl_port_stats_t *p_db_stats, tbl_port_stats_rate_t *p_db_rate, uint32 only_rate, FILE *fp)
{
    uint64 zero64 = 0;
    tbl_interface_t *p_db_if = NULL;
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_stats->key.ifindex);
    if (!p_db_if)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u",   "ifindex",      p_db_stats->key.ifindex);

    if (!only_rate)
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_uc",    p_db_stats->uc_pkts_rcv);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_mc",    p_db_stats->mc_pkts_rcv);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_bc",    p_db_stats->brdc_pkts_rcv);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_uc",    p_db_stats->uc_pkts_send);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_mc",    p_db_stats->mc_pkts_send);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_bc",    p_db_stats->brdc_pkts_send);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_err",   p_db_stats->mac_transmit_err);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_bad",   p_db_stats->bad_pkts_rcv);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_crc",   p_db_stats->bad_crc);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_rx",
            p_db_if->ifg_stats_en ? p_db_stats->octets_rcv + p_db_stats->pkts_rcv*GLB_DEF_IPG_LEN : p_db_stats->octets_rcv);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_tx",
            p_db_if->ifg_stats_en ? p_db_stats->octets_send + p_db_stats->pkts_send*GLB_DEF_IPG_LEN : p_db_stats->octets_send);
    }
    /* For bug 46189, port_stats & port_stats_rate not create at same PM/time, need to check pointer */
    if (p_db_rate)
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_rate",  p_db_rate->pkts_rcv_rate);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_rate",  p_db_rate->pkts_send_rate);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_rx_rate",
            p_db_if->ifg_stats_en ? p_db_rate->octets_rcv_rate + p_db_rate->pkts_rcv_rate*GLB_DEF_IPG_LEN*8 : p_db_rate->octets_rcv_rate);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_tx_rate",
            p_db_if->ifg_stats_en ? p_db_rate->octets_send_rate + p_db_rate->pkts_send_rate*GLB_DEF_IPG_LEN*8 : p_db_rate->octets_send_rate);
    }
    else
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_rx_rate",  zero64);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "pkt_tx_rate",  zero64);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_rx_rate",  zero64);
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%"PRIu64, "oct_tx_rate",  zero64);
    }

    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

static int32
_if_mgr_mib_stats_dump_count(cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    FILE *fp = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_port_stats_t *p_iter_stats = NULL;
    tbl_port_stats_rate_t *p_iter_rate = NULL;
    tbl_port_stats_rate_key_t rate_key;
    uint32 read_number = 0;
    uint32 current_index = 0;
    uint32 start_start_index = 0;
    static uint32 start_index = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    start_start_index = start_index;
    CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_iter_stats, listnode)
    {
        current_index++;
        if (current_index <= start_index)
        {
            continue;
        }
        rate_key.ifindex = p_iter_stats->key.ifindex;
        p_iter_rate = tbl_port_stats_rate_get_port_stats_rate(&rate_key); 
        _if_mgr_mib_stats_dump_one(p_iter_stats, p_iter_rate, FALSE, fp);
        start_index++;
        read_number++;
        if (read_number >= p_sys_glb->mib_port_stats_read_num)
        {
            break;
        }
    }

    /* if not full, set start_index = 0 */
    if (read_number < p_sys_glb->mib_port_stats_read_num)
    {
        start_index = 0;
        /* not from the 0 position, need to do secondary loop */
        if (0 != start_start_index)
        {
            CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_iter_stats, listnode)
            {
                current_index++;
                rate_key.ifindex = p_iter_stats->key.ifindex;
                p_iter_rate = tbl_port_stats_rate_get_port_stats_rate(&rate_key); 
                _if_mgr_mib_stats_dump_one(p_iter_stats, p_iter_rate, FALSE, fp);
                start_index++;
                read_number++;
                if (read_number >= p_sys_glb->mib_port_stats_read_num)
                {
                    break;
                }
            }
            /* after secondary loop, set start_index to 0 */
            if (read_number < p_sys_glb->mib_port_stats_read_num)
            {
                start_index = 0;
            }
        }
    }
        
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_if_mgr_mib_stats_dump_all(tbl_port_stats_t *p_db_stats, tbl_port_stats_rate_t *p_db_rate, uint32 only_rate, cfg_cmd_para_t *para)
{
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    FILE *fp = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_port_stats_t *p_iter_stats = NULL;
    tbl_port_stats_rate_t *p_iter_rate = NULL;
    tbl_port_stats_rate_key_t rate_key;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (NULL == p_db_stats)
    {
        CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_iter_stats, listnode)
        {
            rate_key.ifindex = p_iter_stats->key.ifindex;
            p_iter_rate = tbl_port_stats_rate_get_port_stats_rate(&rate_key); 
            _if_mgr_mib_stats_dump_one(p_iter_stats, p_iter_rate, only_rate, fp);
        }
    }
    else
    {
        _if_mgr_mib_stats_dump_one(p_db_stats, p_db_rate, only_rate, fp);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
if_mgr_cmd_process_mib_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_port_stats_t *p_db_stats = NULL;
    tbl_port_stats_key_t key;
    tbl_port_stats_rate_t *p_db_rate = NULL;
    tbl_port_stats_rate_key_t rate_key;
    uint32 ifindex = 0;
    uint32 only_rate = FALSE;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            if (0 == sal_strcmp("all", argv[0]))
            {
            }
            else if  (0 == sal_strcmp("rate", argv[0]))
            {
                only_rate = TRUE;
            }
            else
            {
                PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
                key.ifindex = ifindex;
                rate_key.ifindex = ifindex;
                p_db_stats = tbl_port_stats_get_port_stats(&key);
                p_db_rate = tbl_port_stats_rate_get_port_stats_rate(&rate_key);            
                if (NULL == p_db_stats || NULL == p_db_rate)
                {
                    CFG_FUNC_ERR_RET("Port stats ifindex %u not exist\n", ifindex);
                }
            }
            _if_mgr_mib_stats_dump_all(p_db_stats, p_db_rate, only_rate, para);
        }
        else
        {
            _if_mgr_mib_stats_dump_count(para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
if_mgr_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_INTERFACE, if_mgr_cmd_process_interface);
    //cdb_register_cfg_tbl_cb(TBL_ROUTE_IF, rt_if_cmd_process_route_if);
    cdb_register_cfg_tbl_cb(TBL_ACLQOS_IF, if_mgr_cmd_process_aclqos_if);
    cdb_register_cfg_tbl_cb(TBL_ISOLATION, if_mgr_cmd_process_isolation);
    ipc_register_hal_rx_fn(SWITCH_FEI_PHY_NOTIFY, if_mgr_rx_link_notify);
    cdb_register_cfg_act_cb(ACT_SHOW_INTERFACE, if_mgr_cmd_process_interface_show);
    cdb_register_cfg_act_cb(ACT_SHOW_ISOLATION, if_mgr_cmd_process_isolation_show);
    cdb_register_cfg_act_cb(ACT_SHOW_PVLAN, if_mgr_cmd_process_pvlan_show);
    cdb_register_cfg_act_cb(ACT_SHOW_PFC, if_mgr_cmd_process_pfc_show);

    cdb_register_cfg_tbl_cb(TBL_IPSG_FIB, ipsg_cmd_process_ip_source_guard);
    cdb_register_cfg_tbl_cb(TBL_IPSG_S_IP, ipsg_cmd_process_ipsg_ip);
    cdb_register_cfg_tbl_cb(TBL_IPSG_S_MAC, ipsg_cmd_process_ipsg_mac);

    cdb_register_cfg_act_cb(ACT_SHOW_IPSG, ipsg_cmd_process_ipsg_dhcs);

    cdb_register_cfg_tbl_cb(TBL_PORT_STATS_RATE, if_mgr_cmd_process_stats_rate);
    cdb_register_cfg_act_cb(ACT_CLEAR_IF_STATS_RATE, if_clear_port_stats_rate);
    cdb_register_cfg_act_cb(ACT_SHOW_BANDWIDTH_USAGE, if_mgr_cmd_show_bandwidth_usage);
    cdb_register_cfg_act_cb(ACT_SHOW_MIB_STATS, if_mgr_cmd_process_mib_stats);

    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_INTERFACE, if_mgr_cmd_process_openflow_interface);
    cdb_register_cfg_tbl_cb(TBL_OPENFLOW_TUNNEL_INTERFACE, if_mgr_cmd_process_openflow_tunnel_interface);
    
    cdb_pm_subscribe_tbl(TBL_INTERFACE, if_mgr_interface_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_OPENFLOW_INTERFACE, NULL, if_mgr_openflow_interface_sync);

    if_agg_create_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_SFLOW, _sflow_join_agg_create);
    if_agg_add_member_sync_prop_set_cb(IF_AGG_CB_MODULE_SFLOW, _sflow_join_agg_update);
    if_agg_del_member_sync_prop_set_cb(IF_AGG_CB_MODULE_SFLOW, _sflow_leave_agg_update);
    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_SFLOW, _sflow_agg_destroy);

    if_agg_add_member_check_set_cb(IF_AGG_CB_MODULE_IF_MTU, rt_if_check_before_add_agg_member);
    if_agg_attach_member_sync_prop_set_cb(IF_AGG_CB_MODULE_IF_MTU, rt_if_after_add_agg_member);
    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_IF_MTU, rt_if_before_destroy_agg);

    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_DHCP, _dhcp_snooping_agg_destroy);

    /* modified by liwh for bug 53010, 2019-07-22 */
    if_agg_add_member_check_set_cb(IF_AGG_CB_MODULE_CFM, _cfm_check_before_add_agg_member);
    /* liwh end */
    
    _if_mgr_set_agg_add_member_check_func();

    ipc_register_hal_rx_fn(SWITCH_FEI_FDB_LOOP_SYNC, if_mgr_interface_fdb_loop_cb);

    if_rate_db_master.if_stats_hash = ctclib_hash_create(_if_stats_rate_hash_make, _if_stats_rate_hash_cmp);
    g_p_stats_rate_scan_time = ctc_task_add_timer(TASK_PRI_NORMAL,
            if_port_stats_rate_scan_timeout, NULL, GLB_PORT_STATS_INTERVAL);

    return PM_E_NONE;
}

int32
if_mgr_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_INTERFACE, NULL);
    cdb_register_cfg_tbl_cb(TBL_ACLQOS_IF, NULL);
    ipc_register_hal_rx_fn(SWITCH_FEI_PHY_NOTIFY, NULL); 
    ipc_register_hal_rx_fn(SWITCH_FEI_FDB_LOOP_SYNC, NULL); 
    return PM_E_NONE;
}

int32
if_mgr_interface_running_update_by_syscmd(tbl_interface_t* p_if)
{
    int32   rc = PM_E_NONE;
    uint32  new_running = FALSE;

    new_running = GLB_FLAG_ISSET(p_if->flags, GLB_IFF_RUNNING);
    rc = if_up_sync_when_process(p_if, new_running);

    return rc;
}

int32
if_mgr_lag_if_running_update(tbl_interface_t *p_db_if_lag)
{
    tbl_interface_t intf;
    ds_lag_t *p_lag = NULL;
    uint32 old_running = FALSE;
    uint32 new_running = FALSE;
    int32 rc = PM_E_NONE;

    if (GLB_IF_TYPE_LINKAGG_IF != p_db_if_lag->hw_type)
    {
        return PM_E_NONE;
    }

    sal_memset(&intf, 0, sizeof(intf));
    sal_memcpy(intf.key.name, p_db_if_lag->key.name, IFNAME_SIZE);
    intf.ifindex = p_db_if_lag->ifindex;

    old_running = GLB_FLAG_ISSET(p_db_if_lag->flags, GLB_IFF_RUNNING);
    if (tbl_interface_is_up(p_db_if_lag))
    {
        p_lag = p_db_if_lag->lag;
        if (p_lag->bundle_ports_count > 0)
        {
            new_running = TRUE;
        }
    }

    if (!new_running)
    {
        new_running = if_agg_if_running_update_mismatch(p_db_if_lag);
    }

    if (new_running)
    {
        GLB_SET_FLAG(intf.flags, GLB_IFF_RUNNING);
    }

    if (old_running == new_running)
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_RUNNING));
    rc = if_mgr_interface_running_update_by_syscmd(&intf);

    PM_E_RETURN(_if_mgr_running_change_process(p_db_if_lag));

    return PM_E_NONE;
}

int32
if_mgr_vlan_if_running_update(tbl_vlan_t *p_db_vlan)
{
    tbl_interface_t intf;
    tbl_interface_t *p_vlan_if = NULL;
    tbl_interface_t *p_mem_if = NULL;
    uint32 old_running = FALSE;
    uint32 new_running = FALSE;
    int32 ifindex = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&intf, 0, sizeof(intf));
    sal_snprintf(intf.key.name, IFNAME_SIZE, GLB_IFNAME_VLAN_PREFIX"%u", p_db_vlan->key.vid);

    p_vlan_if = tbl_interface_get_interface(&intf.key);
    if (NULL == p_vlan_if)
    {
        return PM_E_NONE;
    }
    old_running = GLB_FLAG_ISSET(p_vlan_if->flags, GLB_IFF_RUNNING);
    
    if (p_db_vlan->enable && tbl_interface_is_up(p_vlan_if))
    {
        GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
        {
            p_mem_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_mem_if)
            {
                continue;
            }

            //if (GLB_BMP_ISSET(p_mem_if->brgif->allowed_vlan.vlan_bmp, p_db_vlan->key.vid)
            //        && tbl_interface_is_running(p_mem_if)) /* TODO : LAG memberport */
            if (tbl_interface_is_running(p_mem_if))
            {
                new_running = TRUE;
                break;
            }
        }
        GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);
    }

    if (new_running)
    {
        GLB_SET_FLAG(intf.flags, GLB_IFF_RUNNING);
    }

    if (old_running == new_running)
    {
        rc = if_mgr_interface_running_update_by_syscmd(&intf);        
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_interface_set_interface_field(&intf, TBL_INTERFACE_FLD_RUNNING));
    rc = if_mgr_interface_running_update_by_syscmd(&intf);
    
    PM_E_RETURN(_if_mgr_running_change_process(p_vlan_if));

    return PM_E_NONE;
}

int32
if_mgr_get_vlan_if_count()
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    return p_sys_glb->curr_vlanif_count;
}

int32
if_mgr_get_loopback_count()
{
    uint32 count = 0;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) && 
            (GLB_LB_NONE != p_db_if->phy_loopback_mode) && 
            (GLB_LB_PHY_FAR != p_db_if->phy_loopback_mode) &&
            (GLB_LB_PHY_LOOPBACK != p_db_if->phy_loopback_mode))
        {
            count++;
        }
    }
    return count;
}
   
uint32
if_mgr_get_max_frame_size(tbl_interface_t *p_db_if)
{
    tbl_sys_global_t* p_sys_glb = tbl_sys_global_get_sys_global();
    
    return (p_db_if->jumboframe_en) ? p_sys_glb->jumbo_frame_size : p_sys_glb->max_frame_size;
}

uint32
if_mgr_get_link_speed(uint8 speed)
{
  switch (speed)
    {
      case GLB_SPEED_AUTO:
        return 0;
      case GLB_SPEED_10M:
        return 10000;
      case GLB_SPEED_100M:
        return 100000;
      case GLB_SPEED_1G:
        return 1000000;
      case GLB_SPEED_2_5G:
        return 2500000;
      case GLB_SPEED_5G:
        return 5000000;
      case GLB_SPEED_10G:
        return 10000000;
      case GLB_SPEED_40G:
        return 40000000;
      case GLB_SPEED_50G:
        return 50000000;
      case GLB_SPEED_25G:
        return 25000000;        
      case GLB_SPEED_100G:
        return 100000000; 
      default:
        return 0;
    }
}

int32
if_mgr_set_storm_control(tbl_interface_t *p_db_if, uint32 rate, uint32 type, uint32 mode)
{
    int32 rc = PM_E_NONE;
    ds_storm_control_t storm_ctl;
	ds_storm_control_t *pdb_storm_ctl = NULL;
    
    if (p_db_if->mode != GLB_IF_MODE_L2)
    {
        return PM_E_NONE; 
    }

    if (p_db_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
    {
        return PM_E_NONE; ; 
    }    

    sal_memset(&storm_ctl, 0, sizeof(storm_ctl));

	pdb_storm_ctl = ds_storm_control_get_storm_control(p_db_if);
	if(pdb_storm_ctl)
	{
		sal_memcpy(&storm_ctl, pdb_storm_ctl, sizeof(storm_ctl));
	}
    
    switch (mode)
    {
    case GLB_RATELIMIT_MODE_NONE: /* disable ratelimit */
        rate = GLB_RATELIMIT_UNIT_BYTE;
	case GLB_RATELIMIT_MODE_PPS:
	case GLB_RATELIMIT_MODE_LEVEL:
        switch (type)
        {
        case GLB_RATELIMIT_UNICAST:              
            storm_ctl.ucast_mode = mode;
            storm_ctl.ucast_rate = rate;
            break;
            
        case GLB_RATELIMIT_BROADCAST:               
            storm_ctl.bcast_mode = mode;
            storm_ctl.bcast_rate = rate;
            break;
            
        case GLB_RATELIMIT_MULTICAST:             
            storm_ctl.mcast_mode = mode;
            storm_ctl.mcast_rate = rate;
            break;
        }    
        break;

    default:
        break;
    }

    if (!p_db_if->storm_control && GLB_RATELIMIT_MODE_NONE != mode)
    {
        rc = ds_storm_control_add_storm_control(p_db_if, &storm_ctl);
    }
    else if(p_db_if->storm_control)
    {
    	if(pdb_storm_ctl->ucast_mode != storm_ctl.ucast_mode)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_UCAST_MODE));

			if(pdb_storm_ctl->ucast_rate != storm_ctl.ucast_rate)
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_UCAST_RATE));
			}
			else
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, pdb_storm_ctl, DS_STORM_CONTROL_FLD_UCAST_RATE));
			}
		}
		else if(pdb_storm_ctl->ucast_rate != storm_ctl.ucast_rate)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_UCAST_RATE));
		}

		if(pdb_storm_ctl->mcast_mode != storm_ctl.mcast_mode)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_MCAST_MODE));

			if(pdb_storm_ctl->mcast_rate != storm_ctl.mcast_rate)
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_MCAST_RATE));
			}
			else
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, pdb_storm_ctl, DS_STORM_CONTROL_FLD_MCAST_RATE));
			}
		}
		else if(pdb_storm_ctl->mcast_rate != storm_ctl.mcast_rate)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_MCAST_RATE));
		}

		if(pdb_storm_ctl->bcast_mode != storm_ctl.bcast_mode)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_BCAST_MODE));

			if(pdb_storm_ctl->bcast_rate != storm_ctl.bcast_rate)
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_BCAST_RATE));
			}
			else
			{
				PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, pdb_storm_ctl, DS_STORM_CONTROL_FLD_BCAST_RATE));
			}
		}
		else if(pdb_storm_ctl->bcast_rate != storm_ctl.bcast_rate)
    	{
			PM_E_RETURN(ds_storm_control_set_storm_control_field(
									p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_BCAST_RATE));
		}
    }
    
    return PM_E_NONE;
}

int32
if_mgr_set_all_if_down()
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if ((GLB_IF_TYPE_PORT_IF == p_db_if->hw_type) || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type))
        {
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                continue;
            }

            if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
            {
                continue;
            }

            if (tbl_interface_is_up(p_db_if))
            {
                if_mgr_set_up(p_db_if, FALSE, TRUE, FALSE);
            }
        }
    }

    return PM_E_NONE;
}

int32
if_mgr_update_storm_control_level(tbl_interface_t *p_db_if, ds_storm_control_t *p_storm_ctl)
{
    int32 rc = PM_E_NONE;
	ds_storm_control_t storm_ctl = {0};

    if(GLB_RATELIMIT_MODE_LEVEL == p_storm_ctl->ucast_mode)
    {
    	storm_ctl.ucast_rate = p_storm_ctl->ucast_rate;
		PM_E_RETURN(ds_storm_control_set_storm_control_field(p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_UCAST_RATE));
    }

    if(GLB_RATELIMIT_MODE_LEVEL == p_storm_ctl->bcast_mode)
    {
    	storm_ctl.bcast_rate = p_storm_ctl->bcast_rate;
		PM_E_RETURN(ds_storm_control_set_storm_control_field(p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_BCAST_RATE));
    }

    if(GLB_RATELIMIT_MODE_LEVEL == p_storm_ctl->mcast_mode)
    {
    	storm_ctl.mcast_rate = p_storm_ctl->mcast_rate;
		PM_E_RETURN(ds_storm_control_set_storm_control_field(p_db_if, &storm_ctl, DS_STORM_CONTROL_FLD_MCAST_RATE));
    }

    return PM_E_NONE;
}

#ifdef CONFIG_SFLOW
int32
if_mgr_set_sflow_counter_enable(tbl_interface_t *p_db_if, uint32 sflow_counter_en)
{
    /* check ptr */
    if (!p_db_if)
    {
        return PM_E_INVALID_PARAM;    
    }

    /* check value isn't change */
    if (sflow_counter_en == p_db_if->sflow_counter_en)
    {
        return PM_E_NONE;
    }

    sflow_api_counter_sampling_enable(p_db_if, sflow_counter_en);
    
    p_db_if->sflow_counter_en = sflow_counter_en;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SFLOW_COUNTER_EN);

    return PM_E_NONE;
}

int32
if_mgr_set_sflow_flow_enable(tbl_interface_t *p_db_if, uint32 flow_en, uint32 ingress, uint32 egress)
{
    ds_lag_t                  *p_lag = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_interface_t           *p_if_mem = NULL;    
    
    /* check ptr */
    if (!p_db_if)
    {
        return PM_E_INVALID_PARAM;    
    }

    /* check value isn't change */
    if (ingress && egress)
    {
        if ((flow_en == p_db_if->sflow_flow_ingress)
            && (flow_en == p_db_if->sflow_flow_egress))
        {
            return PM_E_NONE;    
        }
    }
    else if (ingress)
    {
        if (flow_en == p_db_if->sflow_flow_ingress)
        {
            return PM_E_NONE;    
        }
    }
    else if (egress)
    {
        if (flow_en == p_db_if->sflow_flow_egress)
        {
            return PM_E_NONE;    
        }
    }

    if (flow_en && !p_db_if->sflow_flow_ingress && !p_db_if->sflow_flow_egress)
    {
        p_db_if->sflow_sequence = 0;
        p_db_if->sflow_sample_pool = 0;
    }
    
    if (flow_en && (0 == p_db_if->sflow_rate))
    {
        p_db_if->sflow_rate = GLB_SFLOW_DEFAULT_RATE;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SFLOW_RATE);
    }
    
    if (ingress)
    {
        p_db_if->sflow_flow_ingress = flow_en;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS);
    }
    
    if (egress)
    {
        p_db_if->sflow_flow_egress = flow_en;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS);
    }

    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        if (!p_db_if->lag)
        {
            return PM_E_NONE; 
        }
        
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            if (flow_en && (0 == p_if_mem->sflow_rate))
            {
                p_if_mem->sflow_rate = GLB_SFLOW_DEFAULT_RATE;
                tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_SFLOW_RATE);
            }
        
            if (ingress)
            {
                p_if_mem->sflow_flow_ingress = flow_en;
                tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_SFLOW_FLOW_INGRESS);
            }
        
            if (egress)
            {
                p_if_mem->sflow_flow_egress = flow_en;
                tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_SFLOW_FLOW_EGRESS);
            }
        }
    }
    
    return PM_E_NONE;
}

int32
if_mgr_sflow_if_map_rate(uint32 from, uint8 *out)
{
    uint32    i = 0;
    
    if ((from > GLB_SFLOW_MAX_RATE) || (from == 0))
    {
        return PM_E_INVALID_PARAM;
    }

    /* from should be a power of 2 */
    if (0 != ((from - 1) & from))
    {
        return PM_E_INVALID_PARAM;
    }

    for (i = 0; from; i++)
    {
        from = from >> 1;
    }
    
    *out = GLB_ACL_LOG_BIT_LEN - (uint8)(i -1);
    return PM_E_NONE;
}

int32
if_mgr_set_sflow_rate(tbl_interface_t *p_db_if, uint32 weight)
{
    ds_lag_t                  *p_lag = NULL;
    ctclib_slistnode_t        *listnode = NULL;
    tbl_interface_t           *p_if_mem = NULL;

    /* check ptr */
    if (!p_db_if)
    {
        return PM_E_INVALID_PARAM;    
    }

    /* check value isn't change */
    if (weight == p_db_if->sflow_rate)
    {
        return PM_E_NONE;
    }
    
    p_db_if->sflow_rate = weight;
    tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_SFLOW_RATE);

    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        if (!p_db_if->lag)
        {
            return PM_E_NONE; 
        }
        
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_if_mem->sflow_rate = weight;
            tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_SFLOW_RATE);
        }
    }
 
    return PM_E_NONE;
}
#endif /* CONFIG_SFLOW */

int32
if_mgr_check_packet_tx_condition(tbl_interface_t *p_db_if, glb_pkt_type_t type)
{
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    uint32 is_running = FALSE;

    /* 1. check running */
    is_running = tbl_interface_is_running(p_db_if);
    if (!is_running)
    {
        return PM_E_FAIL;
    }

    /* 2. check LAG member */
    if (GLB_PKT_LACP != type)
    {
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            return PM_E_FAIL;
        }
    }

    /* 3. check STP status */
    if (GLB_PKT_BPDU != type)
    {
        p_db_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t*)(&p_db_if->key));
        if (p_db_mstp_port)
        {
            if (p_db_mstp_port->portEnabled)
            {
                if (GLB_BR_PORT_STATE_DISABLED != p_db_mstp_port->fea_state
                 && GLB_BR_PORT_STATE_FORWARDING != p_db_mstp_port->fea_state)
                {
                    return PM_E_FAIL;
                }
            }
        }
    }
    
    /* 4. check dot1.X status */

    /* 5. IGMP snooping check */
    if (GLB_PKT_IGMP == type)
    {
        if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
        {
            return PM_E_FAIL;
        }
    }

    /* others ... */
    
    return PM_E_NONE;
}

#ifdef CONFIG_OPENFLOW
int32
if_mgr_set_openflow_enable(tbl_interface_t *p_db_if)
{
    ds_openflow_if_t of_if;
    ds_openflow_if_t *p_db_of_if = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&of_if, 0, sizeof(of_if));
    of_if.enable = TRUE;

    p_db_of_if = ds_openflow_if_get_openflow_if(p_db_if);
    if (p_db_of_if)
    {
        return PM_E_NONE;
    }
    
    PM_E_RETURN(ds_openflow_if_add_openflow_if(p_db_if, &of_if));

    if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF)
    {
        p_db_if->openflow_en = 1;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPENFLOW_EN);
    }

    return PM_E_NONE;
}

int32
if_mgr_set_openflow_disable(tbl_interface_t *p_db_if)
{
    ds_openflow_if_t *p_db_of_if = NULL; 
    int32 rc = PM_E_NONE;

    p_db_of_if = ds_openflow_if_get_openflow_if(p_db_if);
    if (NULL == p_db_of_if)
    {
        return PM_E_NONE;
    }
    
    PM_E_RETURN(ds_openflow_if_del_openflow_if(p_db_if));

    if (p_db_if->hw_type == GLB_IF_TYPE_PORT_IF)
    {
        p_db_if->openflow_en = 0;
        tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_OPENFLOW_EN);
    }
    
    return PM_E_NONE;
}
#endif /* CONFIG_OPENFLOW */

int32
if_mgr_set_arpinsp_trust_enable(tbl_interface_t *p_if, uint32 trust_enable, uint32 iter_member)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lag_t *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_if_mem = NULL;
    int32 memrc = PM_E_NONE;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    /*2. set to DB*/ 
    p_if->arpinsp_trust_enable = trust_enable;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ARPINSP_TRUST_ENABLE));

    if ((INTERFACE_IS_LAG_GROUP(p_db_if)) && (iter_member == TRUE))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            memrc = if_mgr_set_arpinsp_trust_enable(p_if_mem, trust_enable, FALSE);
            if (memrc)
            {
                return memrc;
            }
        }
    }

    return rc;  
}

/*modified by yejl to fix bug44736, 2017-07-24*/
int32 
if_mgr_set_arp_numberlimit_enable(tbl_interface_t *p_if, uint32 numberlimit_enable)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    /*2. set to DB*/ 
    p_if->arp_numberlimit_enable = numberlimit_enable; 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_ENABLE));

    return rc;
}

int32 
if_mgr_set_arp_numberlimit_number(tbl_interface_t *p_if, uint32 numberlimit_number)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    /*2. set to DB*/ 
    p_if->arp_numberlimit_number = numberlimit_number; 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_NUMBER));

    return rc;
}

int32 
if_mgr_set_arp_numberlimit_violate(tbl_interface_t *p_if, uint32 numberlimit_violate)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. lookup interface exist */
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    /*2. set to DB*/ 
    p_if->arp_numberlimit_violate = numberlimit_violate; 
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_VIOLATE));

    return rc;
}
/*ended by yejl*/

int32
if_mgr_set_latency_event_enable(tbl_interface_t *p_if, uint32 enable)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. feature process */ 
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */   
    p_if->latency_event_enable = enable;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_LATENCY_EVENT_ENABLE));

    return rc;
}

int32
if_mgr_set_latency_event_threshold(tbl_interface_t *p_if, uint32 low_thrd , uint32 high_thrd)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. feature process */ 
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* 2. update local CDB */   
    p_if->latency_event_low = low_thrd;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_LATENCY_EVENT_LOW));

    p_if->latency_event_high = high_thrd;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH));

    return rc;
}

int32
if_mgr_set_latency_event_threshold_default(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;
    p_if->latency_event_low = LATENCY_EVENT_LOW_THRESHOLD_DEFAULT;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_LATENCY_EVENT_LOW));
    p_if->latency_event_high = LATENCY_EVENT_HIGH_THRESHOLD_DEFAULT;
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_LATENCY_EVENT_HIGH));

    return rc;
}

int32
if_mgr_set_l2protocol_uplink(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. feature process */ 
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_L2PRO_UPLINK));
    return rc;
}

int32
if_mgr_set_l2protocol_vlan(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    ds_l2protocol_t* p_db_dsl2pro = NULL;
    p_db_dsl2pro = ds_l2protocol_get_l2protocol(p_db_if, p_dsl2proto);
    if (NULL == p_db_dsl2pro)
    {
        return PM_E_NONE;
    }
    sal_memset(p_dsl2proto->l2pro_evc_name, 0, sizeof(p_dsl2proto->l2pro_evc_name));
    PM_E_RETURN(ds_l2protocol_set_l2protocol_field(p_db_if, p_dsl2proto, DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME));
    PM_E_RETURN(ds_l2protocol_set_l2protocol_field(p_db_if, p_dsl2proto, DS_L2PROTOCOL_FLD_L2PRO_VLAN));
    return rc;
}

int32
if_mgr_set_l2protocol_action(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    ds_l2protocol_t* p_db_dsl2pro = NULL;
    p_db_dsl2pro = ds_l2protocol_get_l2protocol(p_db_if, p_dsl2proto);
    if (NULL == p_db_dsl2pro)
    {
        return PM_E_NONE;
    }
    PM_E_RETURN(ds_l2protocol_set_l2protocol_field(p_db_if, p_dsl2proto, DS_L2PROTOCOL_FLD_L2PRO_ACTION));
    return rc;
}

int32
if_mgr_set_l2protocol_evc(ds_l2protocol_t *p_dsl2proto, tbl_interface_t *p_db_if)
{
    int32 rc = PM_E_NONE;
    ds_l2protocol_t* p_db_dsl2pro = NULL;
    p_db_dsl2pro = ds_l2protocol_get_l2protocol(p_db_if, p_dsl2proto);
    if (NULL == p_db_dsl2pro)
    {
        return PM_E_NONE;
    }
    p_dsl2proto->l2pro_vlan = 0;
    PM_E_RETURN(ds_l2protocol_set_l2protocol_field(p_db_if, p_dsl2proto, DS_L2PROTOCOL_FLD_L2PRO_VLAN));
    PM_E_RETURN(ds_l2protocol_set_l2protocol_field(p_db_if, p_dsl2proto, DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME));
    return rc;
}

int32
if_mgr_del_ds_l2protocol(tbl_interface_t *p_if)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t* p_db_if = NULL;
    ds_l2protocol_t* p_ds_l2protocol = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    CTCLIB_SLIST_LOOP_DEL(p_db_if->l2protocol.obj_list, p_ds_l2protocol, listnode, next)
    {
        if (p_ds_l2protocol)
        {
            PM_E_RETURN(ds_l2protocol_del_l2protocol(p_db_if, p_ds_l2protocol));
        }
    }
    return rc;
}

int32
if_mgr_set_flow_statistics(tbl_interface_t *p_db_if, uint8 enable)
{
    ds_lag_t   *p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    int32 rc = PM_E_NONE;

    p_db_if->ifg_stats_en = enable;
    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_IFG_STATS_EN));

    if (INTERFACE_IS_LAG_GROUP(p_db_if))
    {
        p_lag = p_db_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_if_mem->ifg_stats_en = enable;
            PM_E_RETURN(tbl_interface_set_interface_field(p_if_mem, TBL_INTERFACE_FLD_IFG_STATS_EN));
        }
    }

    return rc;

}

int32
if_mgr_set_hash_value_profile_select(tbl_interface_t *p_if)
{
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    /* 1. feature process */ 
    p_db_if = tbl_interface_get_interface(&p_if->key);
    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_DIR));
    PM_E_RETURN(tbl_interface_set_interface_field(p_if, TBL_INTERFACE_FLD_HASH_VALUE_PROFILE_ID));
    return rc;
}

