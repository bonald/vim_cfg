
#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_switch_debug_define.h"
#include "gen/tbl_switch_debug.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h"
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_entry_define.h"
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/ds_aclqos_if_define.h"
#include "gen/ds_aclqos_if.h"
#include "gen/tbl_qos_domain_define.h"
#include "gen/tbl_qos_domain.h"
#include "gen/tbl_qos_drop_profile_define.h"
#include "gen/tbl_qos_drop_profile.h"
#include "gen/tbl_qos_global_define.h"
#include "gen/tbl_qos_global.h"
#include "gen/tbl_qos_policer_profile_define.h"
#include "gen/tbl_qos_policer_profile.h"
#include "gen/tbl_qos_queue_shape_profile_define.h"
#include "gen/tbl_qos_queue_shape_profile.h"
#include "gen/tbl_mirror_define.h"
#include "gen/tbl_mirror.h"
#include "gen/tbl_mem_summary_define.h"
#include "gen/tbl_mem_summary.h"
#include "gen/tbl_iptables_prevent_define.h"
#include "gen/tbl_iptables_prevent.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
#include "gen/tbl_tap_group_ingress_define.h"
#include "gen/tbl_tap_group_ingress.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_tap_group_egress_define.h"
#include "gen/tbl_tap_group_egress.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_sys_load_define.h"
#include "gen/tbl_sys_load.h"
#include "gen/tbl_mirror_mac_escape_define.h"
#include "gen/tbl_mirror_mac_escape.h"
#include "gen/tbl_inband_snat_define.h"
#include "gen/tbl_inband_snat.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_hash_field_profile_define.h" 
#include "gen/tbl_hash_field_profile.h" 
#include "gen/tbl_hash_value_profile_define.h" 
#include "gen/tbl_hash_value_profile.h" 
#include "gen/tbl_hash_value_global_define.h" 
#include "gen/tbl_hash_value_global.h" 
#include "gen/tbl_sys_global_define.h" 
#include "gen/tbl_sys_global.h" 
#include "switch.h"
#include "if_mgr.h"
#include "qos/qos_mgr.h"
#include "brg_vlan.h"
#include "brg_fdb.h"
#include "brg.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "mirror.h"
#include "l2_protocol.h"
#include "fei.h"
#include "swth_sys.h"
#include "swth_load.h"
#include "switch_api.h"
#include "openflow_mgr.h"
#include "iptables.h"
#include "ns_port_forwarding.h"
#include "ns_route.h"
#ifdef TAPPRODUCT
#include "tap.h"
#endif
#include "errdisable.h"
#include "acl.h"
#include "acl_policy.h"
#include "ipsour/ip_source.h"
#include "sflow.h"
#include "inband_snat.h"
#include "ctclib_opb.h"
#include "copp.h"
#include "rdb_hiredis.h"
#include "hash_field_profile.h"
#include "hash_value_profile_and_global.h"

#define SYSTEM_PRODUCT_L2PLUS 0
#define SYSTEM_PRODUCT_HYBRID 1
#define SYSTEM_PRODUCT_TAP 2

/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
        sal_printf("CLI utility. Command line shell."
            "The part of the klish project.\n");
        sal_printf("Options:\n");
        sal_printf("\t-v, --version\tPrint version.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
    }
}

int32
system_product_init()
{
    tbl_sys_global_t *p_sys_global = NULL;
    p_sys_global = tbl_sys_global_get_sys_global();
    if (p_sys_global)
    {
#ifdef OFPRODUCT
        p_sys_global->system_product = SYSTEM_PRODUCT_HYBRID;
#else
#ifdef TAPPRODUCT
        p_sys_global->system_product = SYSTEM_PRODUCT_TAP;
#else
        p_sys_global->system_product = SYSTEM_PRODUCT_L2PLUS;
#endif
#endif
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT);
    }
    
    return PM_E_NONE;
}

int32
sevice_http_stop()
{
    tbl_sys_global_t *p_sys_global = NULL;
    char szCmd[512];
    
    p_sys_global = tbl_sys_global_get_sys_global();
    if (p_sys_global)
    {
        p_sys_global->http_disable = 1;
        tbl_sys_global_set_sys_global_field(p_sys_global, TBL_SYS_GLOBAL_FLD_HTTP_DISABLE);

        swth_sys_set_http_conf(p_sys_global->http_port,p_sys_global->http_disable);
        sleep(5);
#ifdef HAVE_DEBIAN
        snprintf(szCmd, 512, "python /var/web/run_server.py stop");
#else
        snprintf(szCmd, 512, "export PYTHONHOME=/usr; python /var/web/run_server.py stop");
#endif
        system(szCmd);
        app_webui_en_file(FALSE);

    }
    return PM_E_NONE;
}

int32
switch_sock_init(switch_master_t *master)
{
    ipc_connect_bhm();
    
    /* client to CDS */
    ipc_connect_ccs_cds(10);
    
    /* client to FEA */
    fei_init(lib_get_master(), NULL, TRUE, 0);

    /* client to Packet */
    ipc_pkt_init(0);

    /* client to trap agent */
    ipc_connect_snmp_trap();

    return PM_E_NONE;
}

static int32
_switch_cmd_get_switch_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_switch_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_switch_debug_get_switch_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_switch_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#define DBG_SWTH_TYPE_FIELD     0
#define DBG_SWTH_TYPE_ALL       1

#define SWTH_DEBUG_RPCAPI_FILE  "/tmp/rpcapi_debug"

static int32
_switch_cmd_update_rpcapi_debug()
{
    tbl_switch_debug_t *p_dbg = tbl_switch_debug_get_switch_debug();
    FILE *fp = NULL;

    fp = sal_fopen(SWTH_DEBUG_RPCAPI_FILE, "w+");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "%u", p_dbg->rpcapi);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_switch_cmd_process_switch_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_switch_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_switch_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    uint32 multi_type = DBG_SWTH_TYPE_FIELD;
    int32 rc = PM_E_NONE;
    uint32 is_rpcapi_debug = FALSE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_SWITCH_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_switch_debug_get_switch_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("SWITCH_DEBUG not found");
        }
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_SWTH_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }

        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_SWTH_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_SWITCH_DEBUG_FLD_STP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PACKET_TX);
                break;
            case TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PACKET_RX);
                break;
            case TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PROTO_CIST);
                break;
            case TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PROTO_MSTI);
                break;
            case TBL_SWITCH_DEBUG_FLD_STP_TIMER:
                GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_TIMER);
                break;
            case TBL_SWITCH_DEBUG_FLD_MLAG_EVENT:
                GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_MLAG_PACKET:
                GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_PACKET);
                break;
            case TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_PROTO);
                break;
            case TBL_SWITCH_DEBUG_FLD_MLAG_TIMER:
                GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_TIMER);
                break;
            case TBL_SWITCH_DEBUG_FLD_LACP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_LACP_PACKET:
                GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_PACKET);
                break;
            case TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_PROTO);
                break;
            case TBL_SWITCH_DEBUG_FLD_LACP_TIMER:
                GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_TIMER);
                break;
            case TBL_SWITCH_DEBUG_FLD_IGSP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX:
                GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PACKET_TX);
                break;
            case TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX:
                GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PACKET_RX);
                break;
            case TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PROTO);
                break;
            case TBL_SWITCH_DEBUG_FLD_IGSP_TIMER:
                GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_TIMER);
                break;
            case TBL_SWITCH_DEBUG_FLD_IPSG_EVENT:
                GLB_SET_FLAG_COND(value, dbg.ipsg, SWTHDBG_FLAG_IPSG_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_IPSG_ERROR:
                GLB_SET_FLAG_COND(value, dbg.ipsg, SWTHDBG_FLAG_IPSG_ERROR);
                break;
            case TBL_SWITCH_DEBUG_FLD_LLDP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_EVENT);
                break;
            case TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX:
                GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PACKET_TX);
                break;
            case TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX:
                GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PACKET_RX);
                break;
            case TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PROTO);
                break;
            case TBL_SWITCH_DEBUG_FLD_LLDP_TIMER:
                GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_TIMER);
                break;
            case TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST:
                GLB_SET_FLAG_COND(value, dbg.rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST);
                is_rpcapi_debug = TRUE;
                break;
            case TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND:
                GLB_SET_FLAG_COND(value, dbg.rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND);
                is_rpcapi_debug = TRUE;
                break;

            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, field_id);
        }
        else if (DBG_SWTH_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_EVENT);
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PACKET_TX);
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PACKET_RX);
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PROTO_CIST);
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_PROTO_MSTI);
            GLB_SET_FLAG_COND(value, dbg.stp, SWTHDBG_FLAG_STP_TIMER);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_STP_TIMER);

            GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_EVENT);
            GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_PACKET);
            GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_PROTO);
            GLB_SET_FLAG_COND(value, dbg.mlag, SWTHDBG_FLAG_MLAG_TIMER);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_MLAG_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_MLAG_PACKET);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_MLAG_TIMER);

            GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_EVENT);
            GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_PACKET);
            GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_PROTO);
            GLB_SET_FLAG_COND(value, dbg.lacp, SWTHDBG_FLAG_LACP_TIMER);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LACP_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LACP_PACKET);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LACP_TIMER);

            GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_EVENT);
            GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PACKET_TX);
            GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PACKET_RX);
            GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_PROTO);
            GLB_SET_FLAG_COND(value, dbg.igsp, SWTHDBG_FLAG_IGSP_TIMER);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IGSP_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IGSP_TIMER);

            GLB_SET_FLAG_COND(value, dbg.ipsg, SWTHDBG_FLAG_IPSG_EVENT);
            GLB_SET_FLAG_COND(value, dbg.ipsg, SWTHDBG_FLAG_IPSG_ERROR);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IPSG_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_IPSG_ERROR);

            GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_EVENT);
            GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PACKET_TX);
            GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PACKET_RX);
            GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_PROTO);
            GLB_SET_FLAG_COND(value, dbg.lldp, SWTHDBG_FLAG_LLDP_TIMER);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LLDP_EVENT);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_LLDP_TIMER);
            
            GLB_SET_FLAG_COND(value, dbg.rpcapi, SWTHDBG_FLAG_RPCAPI_REQUEST);
            GLB_SET_FLAG_COND(value, dbg.rpcapi, SWTHDBG_FLAG_RPCAPI_COMMAND);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST);
            rc = tbl_switch_debug_set_switch_debug_field(&dbg, TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND);
            is_rpcapi_debug = TRUE;
        }
        if (is_rpcapi_debug)
        {
            _switch_cmd_update_rpcapi_debug();
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _switch_cmd_get_switch_debug(para, p_node, &field);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid path");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
switch_process_msg(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    if (MSG_OPER_BHM == p_msg->hdr.operation)
    {
        if ((PM_ID_BHM == p_msg->u_hdr.bhm.src) && (BHM_SHUTDOWN_CODE == p_msg->u_hdr.bhm.pid))
        {
            if_mgr_set_all_if_down();
        }
    }
    else
    {
        return ipc_pm_process_msg(sock, sess_fd, p_msg);
    }
    
    return PM_E_NONE;
}

int32
switch_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_SWITCH);
    lib_master->task_mst = ctc_task_init();
    switch_master_t *master = (switch_master_t*)XCALLOC(MEM_PM_SWITCH_MASTER, sizeof(switch_master_t));

    lib_master->proto_mst = master;
    switch_sock_init(master);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ctclib_opb_init();

    if_mgr_start(master);
    brg_start(master);
#if defined OFPRODUCT && !defined _GLB_UML_SYSTEM_
    sevice_http_stop();
#endif
    
#ifdef TAPPRODUCT
    /* TAP Product */
    lacp_start(master);
//    acl_mgr_start(master);
    
#else
    /* L2+ and OF Product */
    ipsour_api_init();
#ifdef CONFIG_L2
    brg_vlan_start(master);
    brg_fdb_start(master);
    l2_protocol_start(master);
#endif /* CONFIG_L2 */
#ifdef CONFIG_ERRDISABLE
    //errdisable_start(master);
#endif /* CONFIG_ERRDISABLE */
    swth_module_start(master);
#ifdef CONFIG_MIRROR
    mirror_start(master);
#endif /* CONFIG_MIRROR */
    lacp_start(master);
//    acl_mgr_start(master);
    qos_mgr_start(master);
    openflow_mgr_start(master);
    
#endif /* !TAPPRODUCT */
    acl_start(master);
    acl_policy_start(master);
    
#ifdef TSINGMA
    hash_field_profile_start(master);
    hash_value_profile_and_global_start(master);
#endif

#ifdef CONFIG_SFLOW
    sflow_start(master);
#endif

#ifdef TAPPRODUCT
    tap_start(master);
#endif
    cpu_traffic_mgr_start(master);
    ipt_prevent_start(master);
    ipt_service_acl_start(master);
    ns_port_forwarding_start(master);
    ns_route_start(master);
    inband_snat_start(master);
    lib_load_init();

    copp_start(master);
    _switch_cmd_update_rpcapi_debug();
    system_product_init();

    ctc_sock_register_rx_fn(switch_process_msg);
    
    cdb_register_cfg_tbl_cb(TBL_SWITCH_DEBUG, _switch_cmd_process_switch_debug);
    cdb_register_cfg_tbl_cb(TBL_SYS_GLOBAL, swth_cmd_process_sys_global);
    cdb_register_cfg_tbl_cb(TBL_LOG_GLOBAL, swth_cmd_process_log_global);
    cdb_register_cfg_tbl_cb(TBL_SYS_LOAD, swth_cmd_process_load);
    
    cdb_register_cfg_act_cb(ACT_SHOW_RESOURCE, swth_cmd_show_resource);
    cdb_register_cfg_act_cb(ACT_SHOW_MEMORY_THRESHOLD, swth_cmd_show_memory_threshold);
    cdb_register_cfg_act_cb(ACT_SHOW_MONITOR, swth_cmd_show_monitor);
    cdb_register_cfg_act_cb(ACT_CLEAR_RPCAPI_COUNTER, swth_cmd_clear_rpcapi_counter);

    cdb_pm_subscribe_tbl(TBL_SSH_CFG, NULL, swth_cmd_ssh_cfg_sync);

#ifdef HAVE_NETCONF /* Modified by Alexander 2018-04-24 17:01 */
    extern int swth_nc_init(void);
    if (swth_nc_init() != 0) {
        return PM_E_INIT_FAIL;
    }
#endif /* !HAVE_NETCONF */

    ctc_task_main();
   
    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint16 daemon_mode = 0;
    static const char *shortopts = "d:h:v";
 
/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
 { "daemon",      no_argument,       NULL, 'd'},
 { "help",        no_argument,       NULL, 'h'},
 { "version",     no_argument,       NULL, 'v'},
 { 0 }
};
#endif /* HAVE_GETOPT_H */

    /* process SIGINT for telnet child process support CTRL+C */
    signal(SIGINT, SIG_DFL);
 
    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {   
         int32 opt;
   
#ifdef HAVE_GETOPT_H
         opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
         opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
   
         if (EOF == opt)
         {   
             break;
         }
     
         switch (opt)
         {
         case 'd':
             daemon_mode = TRUE;
             break;
         case 'v':
             lib_print_version(progname);
             exit(0);
             break;
         case 'h':
             usage(FALSE, progname);
             break;
         default:
             usage(TRUE, progname);
             break;
         }
    }
 
    switch_init(daemon_mode);
    exit(0);
}
